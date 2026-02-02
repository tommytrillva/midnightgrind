// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPoliceSubsystem.cpp
 * @brief Implementation of Police and Wanted System World Subsystem.
 *
 * Manages police AI behavior, pursuit mechanics, heat level escalation,
 * cooldown/escape mechanics, and bust consequences. Handles coordinated
 * police tactics including roadblocks, spike strips, and helicopter support.
 *
 * Heat Level System (per GDD Section 4.4):
 * - Level 0 (CLEAN): No attention
 * - Level 1 (NOTICED): Occasional patrols
 * - Level 2 (WANTED): Active searching
 * - Level 3 (PURSUIT): Aggressive tactics
 * - Level 4 (MANHUNT): Roadblocks, spikes
 * - Level 5 (MAXIMUM): Full response + helicopter
 *
 * @see UMGPoliceSubsystem
 */

#include "Police/MGPoliceSubsystem.h"
#include "Police/MGPoliceUnit.h"
#include "Police/MGPoliceRoadblock.h"
#include "Police/MGSpikeStrip.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// ==========================================
// SUBSYSTEM LIFECYCLE
// ==========================================

void UMGPoliceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentHeatLevel = EMGHeatLevel::None;
	CurrentHeatPoints = 0;
	bInPursuit = false;
	bInCooldown = false;
	bGettingBusted = false;
	bHelicopterActive = false;
	CurrentTactic = EMGPoliceTactic::StandardPursuit;
}

void UMGPoliceSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	DespawnAllUnits();

	Super::Deinitialize();
}

void UMGPoliceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Start update timer at 10Hz for performance
	TWeakObjectPtr<UMGPoliceSubsystem> WeakThis(this);
	InWorld.GetTimerManager().SetTimer(
		UpdateTimerHandle,
		[WeakThis]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			if (WeakThis->bPoliceEnabled)
			{
				const float MGDeltaTime = 0.1f;
				WeakThis->UpdatePursuit(DeltaTime);
				WeakThis->UpdateCooldown(DeltaTime);
				WeakThis->UpdatePoliceAI(DeltaTime);
				WeakThis->UpdateBustedState(DeltaTime);
				WeakThis->CheckCooldownZones();
				WeakThis->UpdatePursuitStats(DeltaTime);

				// Periodic tactic evaluation
				WeakThis->TimeSinceTacticEvaluation += DeltaTime;
				if (WeakThis->TimeSinceTacticEvaluation >= WeakThis->TacticEvaluationInterval)
				{
					WeakThis->EvaluateTactics();
					WeakThis->TimeSinceTacticEvaluation = 0.0f;
				}

				// Heat decay when not in pursuit
				if (!WeakThis->bInPursuit && WeakThis->CurrentHeatPoints > 0)
				{
					WeakThis->UpdateHeatDecay(DeltaTime);
				}
			}
		},
		0.1f,
		true
	);
}

// ==========================================
// HEAT MANAGEMENT
// ==========================================

void UMGPoliceSubsystem::AddHeat(int32 Amount, EMGViolationType Reason)
{
	if (!bPoliceEnabled)
	{
		return;
	}

	// Apply race multiplier if racing
	if (bStreetRaceActive)
	{
		Amount = FMath::RoundToInt(Amount * RaceHeatMultiplier);
	}

	// Apply notoriety bonus (criminals get noticed faster)
	Amount = FMath::RoundToInt(Amount * (1.0f + CriminalRecord.NotorietyLevel * 0.5f));

	CurrentHeatPoints = FMath::Clamp(CurrentHeatPoints + Amount, 0, MaxHeatPoints);

	EMGHeatLevel OldLevel = CurrentHeatLevel;
	EMGHeatLevel NewLevel = CalculateHeatLevel();

	if (NewLevel != OldLevel)
	{
		SetHeatLevel(NewLevel);
	}

	// Track highest heat in current pursuit
	if (static_cast<uint8>(NewLevel) > static_cast<uint8>(CurrentPursuitStats.PeakHeatLevel))
	{
		CurrentPursuitStats.PeakHeatLevel = NewLevel;
	}

	// Track in criminal record
	if (static_cast<uint8>(NewLevel) > static_cast<uint8>(CriminalRecord.HighestHeatReached))
	{
		CriminalRecord.HighestHeatReached = NewLevel;
	}

	// Update violation count in record
	if (CriminalRecord.ViolationCounts.Contains(Reason))
	{
		CriminalRecord.ViolationCounts[Reason]++;
	}
	else
	{
		CriminalRecord.ViolationCounts.Add(Reason, 1);
	}

	// Start pursuit if we hit Level 1+ and not already in pursuit
	if (NewLevel >= EMGHeatLevel::Level1 && !bInPursuit && !bInCooldown)
	{
		StartPursuit();
	}
}

void UMGPoliceSubsystem::SetHeatLevel(EMGHeatLevel NewLevel)
{
	EMGHeatLevel OldLevel = CurrentHeatLevel;
	CurrentHeatLevel = NewLevel;

	if (OldLevel != NewLevel)
	{
		OnHeatLevelChanged.Broadcast(OldLevel, NewLevel);

		// Adjust police presence for new level
		SpawnUnitsForHeatLevel();

		// Deploy special units at high heat levels
		if (NewLevel >= EMGHeatLevel::Level4 && OldLevel < EMGHeatLevel::Level4)
		{
			// Request roadblock tactic
			if (IsTacticAvailable(EMGPoliceTactic::RoadblockAhead))
			{
				RequestTactic(EMGPoliceTactic::RoadblockAhead);
			}
		}

		if (NewLevel >= EMGHeatLevel::Level5 && !bHelicopterActive)
		{
			DeployHelicopter();
		}

		UE_LOG(LogTemp, Log, TEXT("Police: Heat level changed from %d to %d"),
			static_cast<int32>(OldLevel), static_cast<int32>(NewLevel));
	}
}

float UMGPoliceSubsystem::GetHeatDecayProgress() const
{
	if (!bInCooldown || CooldownDuration <= 0.0f)
	{
		return 0.0f;
	}
	return CooldownTimer / CooldownDuration;
}

void UMGPoliceSubsystem::ClearHeat()
{
	EMGHeatLevel OldLevel = CurrentHeatLevel;
	CurrentHeatPoints = 0;
	CurrentHeatLevel = EMGHeatLevel::None;

	if (OldLevel != EMGHeatLevel::None)
	{
		OnHeatLevelChanged.Broadcast(OldLevel, EMGHeatLevel::None);
	}

	bInPursuit = false;
	bInCooldown = false;
	bGettingBusted = false;
	BustedProgress = 0.0f;
	bHelicopterActive = false;
	HelicopterUnitID = -1;
	CurrentTactic = EMGPoliceTactic::StandardPursuit;

	DespawnAllUnits();

	// Clear active roadblocks and spike strips
	for (auto& Roadblock : ActiveRoadblocks)
	{
		if (Roadblock.IsValid())
		{
			Roadblock->Destroy();
		}
	}
	ActiveRoadblocks.Empty();

	for (auto& SpikeStrip : ActiveSpikeStrips)
	{
		if (SpikeStrip.IsValid())
		{
			SpikeStrip->Destroy();
		}
	}
	ActiveSpikeStrips.Empty();
}

int32 UMGPoliceSubsystem::GetHeatPointsForLevel(EMGHeatLevel Level) const
{
	switch (Level)
	{
		case EMGHeatLevel::None: return 0;
		case EMGHeatLevel::Level1: return HeatLevel1Threshold;
		case EMGHeatLevel::Level2: return HeatLevel2Threshold;
		case EMGHeatLevel::Level3: return HeatLevel3Threshold;
		case EMGHeatLevel::Level4: return HeatLevel4Threshold;
		case EMGHeatLevel::Level5: return HeatLevel5Threshold;
		default: return 0;
	}
}

float UMGPoliceSubsystem::GetHeatPercentage() const
{
	return FMath::Clamp(static_cast<float>(CurrentHeatPoints) / static_cast<float>(MaxHeatPoints), 0.0f, 1.0f);
}

float UMGPoliceSubsystem::GetHeatLevelProgress() const
{
	int32 CurrentThreshold = GetHeatPointsForLevel(CurrentHeatLevel);
	int32 NextThreshold = 0;

	switch (CurrentHeatLevel)
	{
		case EMGHeatLevel::None: NextThreshold = HeatLevel1Threshold; break;
		case EMGHeatLevel::Level1: NextThreshold = HeatLevel2Threshold; break;
		case EMGHeatLevel::Level2: NextThreshold = HeatLevel3Threshold; break;
		case EMGHeatLevel::Level3: NextThreshold = HeatLevel4Threshold; break;
		case EMGHeatLevel::Level4: NextThreshold = HeatLevel5Threshold; break;
		case EMGHeatLevel::Level5: return 1.0f; // Max level
		default: return 0.0f;
	}

	int32 PointsInLevel = CurrentHeatPoints - CurrentThreshold;
	int32 LevelRange = NextThreshold - CurrentThreshold;

	if (LevelRange <= 0) return 1.0f;

	return FMath::Clamp(static_cast<float>(PointsInLevel) / static_cast<float>(LevelRange), 0.0f, 1.0f);
}

EMGHeatLevel UMGPoliceSubsystem::CalculateHeatLevel() const
{
	if (CurrentHeatPoints >= HeatLevel5Threshold) return EMGHeatLevel::Level5;
	if (CurrentHeatPoints >= HeatLevel4Threshold) return EMGHeatLevel::Level4;
	if (CurrentHeatPoints >= HeatLevel3Threshold) return EMGHeatLevel::Level3;
	if (CurrentHeatPoints >= HeatLevel2Threshold) return EMGHeatLevel::Level2;
	if (CurrentHeatPoints >= HeatLevel1Threshold) return EMGHeatLevel::Level1;
	return EMGHeatLevel::None;
}

// ==========================================
// PURSUIT STATE
// ==========================================

float UMGPoliceSubsystem::GetCooldownProgress() const
{
	if (!bInCooldown || CooldownDuration <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(CooldownTimer / CooldownDuration, 0.0f, 1.0f);
}

float UMGPoliceSubsystem::GetCooldownTimeRemaining() const
{
	if (!bInCooldown)
	{
		return 0.0f;
	}
	return FMath::Max(0.0f, CooldownDuration - CooldownTimer);
}

void UMGPoliceSubsystem::StartPursuit()
{
	if (bInPursuit)
	{
		return;
	}

	bInPursuit = true;
	bInCooldown = false;
	CooldownTimer = 0.0f;

	// Reset pursuit stats
	CurrentPursuitStats = FMGPursuitStats();
	CurrentPursuitStats.PeakHeatLevel = CurrentHeatLevel;

	OnPursuitStarted.Broadcast(CurrentHeatLevel);

	// Spawn initial units
	SpawnUnitsForHeatLevel();

	UE_LOG(LogTemp, Log, TEXT("Police: Pursuit started at heat level %d"), static_cast<int32>(CurrentHeatLevel));
}

void UMGPoliceSubsystem::EndPursuit(EMGPursuitOutcome Outcome)
{
	if (!bInPursuit)
	{
		return;
	}

	bInPursuit = false;
	bGettingBusted = false;
	BustedProgress = 0.0f;
	bInCooldown = false;

	// Update criminal record based on outcome
	UpdateCriminalRecord(Outcome);

	if (Outcome == EMGPursuitOutcome::Escaped)
	{
		CurrentPursuitStats.BountyEarned = CalculateBountyReward();
		OnPlayerEscaped.Broadcast(CurrentPursuitStats);
		UE_LOG(LogTemp, Log, TEXT("Police: Player escaped! Bounty earned: %lld"), CurrentPursuitStats.BountyEarned);
	}
	else if (Outcome == EMGPursuitOutcome::Busted)
	{
		FMGBustConsequences Consequences = CalculateBustConsequences();
		OnPlayerBusted.Broadcast(Consequences);
		UE_LOG(LogTemp, Log, TEXT("Police: Player busted! Fine: %lld, REP lost: %d"),
			Consequences.FineAmount, Consequences.REPLost);
	}

	OnPursuitEnded.Broadcast(Outcome, CurrentPursuitStats);

	// Clear heat after pursuit ends
	ClearHeat();
}

void UMGPoliceSubsystem::StartCooldown()
{
	if (!bInPursuit || bInCooldown)
	{
		return;
	}

	bInCooldown = true;
	CooldownTimer = 0.0f;

	// Calculate cooldown duration based on heat level
	// Higher heat = longer cooldown required
	switch (CurrentHeatLevel)
	{
		case EMGHeatLevel::Level1: CooldownDuration = 15.0f; break;
		case EMGHeatLevel::Level2: CooldownDuration = 25.0f; break;
		case EMGHeatLevel::Level3: CooldownDuration = 40.0f; break;
		case EMGHeatLevel::Level4: CooldownDuration = 60.0f; break;
		case EMGHeatLevel::Level5: CooldownDuration = 90.0f; break;
		default: CooldownDuration = 10.0f; break;
	}

	OnCooldownStarted.Broadcast(CooldownDuration);
	UE_LOG(LogTemp, Log, TEXT("Police: Cooldown started, duration: %.1f seconds"), CooldownDuration);
}

void UMGPoliceSubsystem::InterruptCooldown()
{
	if (!bInCooldown)
	{
		return;
	}

	bInCooldown = false;
	CooldownTimer = 0.0f;
	CurrentPursuitStats.CooldownsInterrupted++;

	UE_LOG(LogTemp, Log, TEXT("Police: Cooldown interrupted! Player spotted again."));
}

// ==========================================
// VIOLATIONS
// ==========================================

void UMGPoliceSubsystem::ReportViolation(EMGViolationType Type, FVector Location)
{
	ReportViolationWithWitness(Type, Location, true, -1);
}

void UMGPoliceSubsystem::ReportViolationWithWitness(EMGViolationType Type, FVector Location, bool bWasWitnessed, int32 WitnessUnitID)
{
	int32 HeatGained = GetHeatForViolation(Type);
	int64 Fine = GetFineForViolation(Type);

	// Unwitnessed violations have reduced impact
	if (!bWasWitnessed)
	{
		HeatGained = FMath::RoundToInt(HeatGained * 0.5f);
		Fine = FMath::RoundToInt(Fine * 0.5f);
	}

	FMGViolationRecord Record;
	Record.Type = Type;
	Record.Timestamp = FDateTime::Now();
	Record.Location = Location;
	Record.HeatGained = HeatGained;
	Record.FineAmount = Fine;
	Record.bWasWitnessed = bWasWitnessed;
	Record.WitnessUnitID = WitnessUnitID;

	CurrentPursuitStats.Violations.Add(Record);
	CurrentPursuitStats.TotalFines += Fine;

	OnViolationCommitted.Broadcast(Record);

	AddHeat(HeatGained, Type);
}

int32 UMGPoliceSubsystem::GetHeatForViolation(EMGViolationType Type) const
{
	// Heat values tuned for progression from minor to severe
	switch (Type)
	{
		case EMGViolationType::Speeding:             return 20;
		case EMGViolationType::Reckless:             return 40;
		case EMGViolationType::RunRedLight:          return 25;
		case EMGViolationType::HitCivilian:          return 75;
		case EMGViolationType::HitPolice:            return 150;
		case EMGViolationType::EvadePursuit:         return 100;
		case EMGViolationType::StreetRacing:         return 200;
		case EMGViolationType::PropertyDamage:       return 35;
		case EMGViolationType::WrongWay:             return 30;
		case EMGViolationType::Nitrous:              return 15;
		case EMGViolationType::RoadblockBreach:      return 125;
		case EMGViolationType::PoliceVehicleDestroyed: return 200;
		case EMGViolationType::NearMissPolice:       return 50;
		case EMGViolationType::SpikeStripEvaded:     return 75;
		default: return 10;
	}
}

int64 UMGPoliceSubsystem::GetFineForViolation(EMGViolationType Type) const
{
	// Fines in dollars
	switch (Type)
	{
		case EMGViolationType::Speeding:             return 350;
		case EMGViolationType::Reckless:             return 800;
		case EMGViolationType::RunRedLight:          return 200;
		case EMGViolationType::HitCivilian:          return 2000;
		case EMGViolationType::HitPolice:            return 5000;
		case EMGViolationType::EvadePursuit:         return 3000;
		case EMGViolationType::StreetRacing:         return 2500;
		case EMGViolationType::PropertyDamage:       return 1000;
		case EMGViolationType::WrongWay:             return 400;
		case EMGViolationType::Nitrous:              return 250;
		case EMGViolationType::RoadblockBreach:      return 3500;
		case EMGViolationType::PoliceVehicleDestroyed: return 7500;
		case EMGViolationType::NearMissPolice:       return 500;
		case EMGViolationType::SpikeStripEvaded:     return 1500;
		default: return 100;
	}
}

// ==========================================
// POLICE UNITS
// ==========================================

int32 UMGPoliceSubsystem::SpawnPoliceUnit(EMGPoliceUnitType UnitType, FVector SpawnLocation)
{
	// Check max units for current heat level
	int32 MaxUnits = GetMaxUnitsForHeatLevel(CurrentHeatLevel);
	if (GetActiveUnitCount() >= MaxUnits)
	{
		return -1;
	}

	FMGPoliceUnitState NewUnit;
	NewUnit.UnitID = NextUnitID++;
	NewUnit.UnitType = UnitType;
	NewUnit.Behavior = EMGPoliceBehavior::Alerted;
	NewUnit.Health = 100.0f;
	NewUnit.AggressionLevel = AggressionMultiplier * (1.0f + CriminalRecord.NotorietyLevel * 0.3f);

	// Spawn the police vehicle actor
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Get spawn rotation facing the player
		FRotator SpawnRotation = FRotator::ZeroRotator;
		if (PlayerVehicle.IsValid())
		{
			FVector ToPlayer = PlayerVehicle->GetActorLocation() - SpawnLocation;
			ToPlayer.Z = 0.0f;
			if (!ToPlayer.IsNearlyZero())
			{
				SpawnRotation = ToPlayer.Rotation();
			}
		}

		AMGPoliceUnit* SpawnedUnit = World->SpawnActor<AMGPoliceUnit>(
			AMGPoliceUnit::StaticClass(),
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (SpawnedUnit)
		{
			SpawnedUnit->InitializeUnit(NewUnit.UnitID, EMGPoliceState::Alerted);

			// Set pursuit target to player
			if (PlayerVehicle.IsValid())
			{
				SpawnedUnit->SetPursuitTarget(PlayerVehicle.Get());
				SpawnedUnit->StartPursuit();
			}

			NewUnit.UnitActor = SpawnedUnit;
		}
	}

	ActiveUnits.Add(NewUnit);
	OnPoliceUnitSpawned.Broadcast(NewUnit.UnitID, UnitType);

	UE_LOG(LogTemp, Log, TEXT("Police: Unit %d spawned (Type: %d) at %s"),
		NewUnit.UnitID, static_cast<int32>(UnitType), *SpawnLocation.ToString());

	return NewUnit.UnitID;
}

void UMGPoliceSubsystem::DespawnPoliceUnit(int32 UnitID)
{
	for (int32 i = ActiveUnits.Num() - 1; i >= 0; --i)
	{
		if (ActiveUnits[i].UnitID == UnitID)
		{
			if (ActiveUnits[i].UnitActor.IsValid())
			{
				ActiveUnits[i].UnitActor->Destroy();
			}
			ActiveUnits.RemoveAt(i);
			break;
		}
	}
}

void UMGPoliceSubsystem::DespawnAllUnits()
{
	for (FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.UnitActor.IsValid())
		{
			Unit.UnitActor->Destroy();
		}
	}
	ActiveUnits.Empty();
}

int32 UMGPoliceSubsystem::GetActiveUnitCount() const
{
	return ActiveUnits.Num();
}

int32 UMGPoliceSubsystem::GetMaxUnitsForHeatLevel(EMGHeatLevel Level) const
{
	switch (Level)
	{
		case EMGHeatLevel::None:   return 0;
		case EMGHeatLevel::Level1: return 2;
		case EMGHeatLevel::Level2: return 4;
		case EMGHeatLevel::Level3: return 6;
		case EMGHeatLevel::Level4: return 8;
		case EMGHeatLevel::Level5: return 12;
		default: return 0;
	}
}

TArray<FMGPoliceUnitState> UMGPoliceSubsystem::GetUnitsWithVisual() const
{
	TArray<FMGPoliceUnitState> UnitsWithVisual;
	for (const FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.bHasVisualOnPlayer && Unit.Behavior != EMGPoliceBehavior::Disabled)
		{
			UnitsWithVisual.Add(Unit);
		}
	}
	return UnitsWithVisual;
}

FMGPoliceUnitState UMGPoliceSubsystem::GetNearestUnit() const
{
	FMGPoliceUnitState NearestUnit;
	float NearestDistance = MAX_FLT;

	for (const FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.Behavior != EMGPoliceBehavior::Disabled && Unit.DistanceToPlayer < NearestDistance)
		{
			NearestDistance = Unit.DistanceToPlayer;
			NearestUnit = Unit;
		}
	}

	return NearestUnit;
}

void UMGPoliceSubsystem::DisableUnit(int32 UnitID)
{
	for (FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.UnitID == UnitID)
		{
			Unit.Behavior = EMGPoliceBehavior::Disabled;
			Unit.Health = 0.0f;

			CurrentPursuitStats.CopsDisabled++;
			CriminalRecord.TotalCopsDisabled++;

			// Add violation for destroying police vehicle
			ReportViolation(EMGViolationType::PoliceVehicleDestroyed,
				Unit.UnitActor.IsValid() ? Unit.UnitActor->GetActorLocation() : FVector::ZeroVector);

			OnPoliceUnitDisabled.Broadcast(UnitID);
			break;
		}
	}
}

void UMGPoliceSubsystem::SpawnRoadblock(FVector Location, FVector Direction)
{
	// Only at heat level 4+
	if (CurrentHeatLevel < EMGHeatLevel::Level4)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AMGPoliceRoadblock* Roadblock = World->SpawnActor<AMGPoliceRoadblock>(
		AMGPoliceRoadblock::StaticClass(),
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Roadblock)
	{
		Roadblock->Initialize(NextRoadblockID++, Direction);
		Roadblock->SetNumVehicles(FMath::RandRange(2, 3));

		ActiveRoadblocks.Add(Roadblock);
		OnRoadblockSpawned.Broadcast(Location);

		UE_LOG(LogTemp, Log, TEXT("Police: Roadblock spawned at %s"), *Location.ToString());
	}

	// Also spawn police units at the roadblock
	SpawnPoliceUnit(EMGPoliceUnitType::Roadblock, Location);
}

void UMGPoliceSubsystem::DeploySpikeStrip(FVector Location, FVector Direction)
{
	// Only at heat level 3+
	if (CurrentHeatLevel < EMGHeatLevel::Level3)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AMGSpikeStrip* SpikeStrip = World->SpawnActor<AMGSpikeStrip>(
		AMGSpikeStrip::StaticClass(),
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (SpikeStrip)
	{
		SpikeStrip->Initialize(NextSpikeStripID++, Direction);
		SpikeStrip->SetLength(FMath::RandRange(400.0f, 800.0f));
		SpikeStrip->Deploy();

		ActiveSpikeStrips.Add(SpikeStrip);
		OnSpikeStripDeployed.Broadcast(Location);

		UE_LOG(LogTemp, Log, TEXT("Police: Spike strip deployed at %s"), *Location.ToString());
	}
}

void UMGPoliceSubsystem::DeployHelicopter()
{
	if (bHelicopterActive || CurrentHeatLevel < EMGHeatLevel::Level5)
	{
		return;
	}

	// Spawn helicopter above player
	FVector HeliSpawnLocation = PlayerLastKnownPosition + FVector(0, 0, 50000.0f); // 500m up
	HelicopterUnitID = SpawnPoliceUnit(EMGPoliceUnitType::Helicopter, HeliSpawnLocation);

	if (HelicopterUnitID != -1)
	{
		bHelicopterActive = true;
		CurrentPursuitStats.bHelicopterDeployed = true;
		OnHelicopterDeployed.Broadcast();

		UE_LOG(LogTemp, Log, TEXT("Police: Helicopter deployed!"));
	}
}

// ==========================================
// TACTICS
// ==========================================

void UMGPoliceSubsystem::RequestTactic(EMGPoliceTactic Tactic)
{
	if (!IsTacticAvailable(Tactic))
	{
		return;
	}

	if (CurrentTactic != Tactic)
	{
		CurrentTactic = Tactic;
		OnPoliceTacticChanged.Broadcast(Tactic);

		// Assign tactic to units
		for (FMGPoliceUnitState& Unit : ActiveUnits)
		{
			if (Unit.Behavior != EMGPoliceBehavior::Disabled)
			{
				Unit.AssignedTactic = Tactic;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Police: Tactic changed to %d"), static_cast<int32>(Tactic));
	}
}

bool UMGPoliceSubsystem::IsTacticAvailable(EMGPoliceTactic Tactic) const
{
	switch (Tactic)
	{
		case EMGPoliceTactic::StandardPursuit:
			return true;

		case EMGPoliceTactic::BoxingManeuver:
			return CurrentHeatLevel >= EMGHeatLevel::Level2 && GetActiveUnitCount() >= 3;

		case EMGPoliceTactic::RoadblockAhead:
			return CurrentHeatLevel >= EMGHeatLevel::Level4;

		case EMGPoliceTactic::SpikeStripTrap:
			return CurrentHeatLevel >= EMGHeatLevel::Level3;

		case EMGPoliceTactic::AerialSupport:
			return CurrentHeatLevel >= EMGHeatLevel::Level5 && bHelicopterActive;

		case EMGPoliceTactic::RollingRoadblock:
			return CurrentHeatLevel >= EMGHeatLevel::Level4 && GetActiveUnitCount() >= 4;

		case EMGPoliceTactic::Funneling:
			return CurrentHeatLevel >= EMGHeatLevel::Level3 && GetActiveUnitCount() >= 4;

		default:
			return false;
	}
}

// ==========================================
// BUSTED MECHANICS
// ==========================================

void UMGPoliceSubsystem::PlayerBusted()
{
	if (!bInPursuit)
	{
		return;
	}

	SetHeatLevel(EMGHeatLevel::Busted);
	EndPursuit(EMGPursuitOutcome::Busted);
}

void UMGPoliceSubsystem::CancelBusted()
{
	if (!bGettingBusted)
	{
		return;
	}

	bGettingBusted = false;
	BustedProgress = 0.0f;
	BustedTimer = 0.0f;

	// Add extra heat for escaping during bust
	AddHeat(50, EMGViolationType::EvadePursuit);

	UE_LOG(LogTemp, Log, TEXT("Police: Bust cancelled - player escaped!"));
}

int64 UMGPoliceSubsystem::CalculateBustPenalty() const
{
	// Base fine plus all violations
	int64 BaseFine = 5000;

	// Multiplier based on heat level (per GDD: 5-15% of car value)
	float Multiplier = 1.0f;
	switch (CurrentHeatLevel)
	{
		case EMGHeatLevel::Level1: Multiplier = 1.0f; break;
		case EMGHeatLevel::Level2: Multiplier = 1.5f; break;
		case EMGHeatLevel::Level3: Multiplier = 2.0f; break;
		case EMGHeatLevel::Level4: Multiplier = 3.0f; break;
		case EMGHeatLevel::Level5: Multiplier = 5.0f; break;
		default: Multiplier = 1.0f; break;
	}

	// Calculate vehicle-based fine (5-15% of value per GDD)
	float VehicleFinePercent = FMath::GetMappedRangeValueClamped(
		FVector2D(static_cast<float>(EMGHeatLevel::Level1), static_cast<float>(EMGHeatLevel::Level5)),
		FVector2D(0.05f, 0.15f),
		static_cast<float>(CurrentHeatLevel)
	);
	int64 VehicleFine = static_cast<int64>(PlayerVehicleValue * VehicleFinePercent);

	return static_cast<int64>((BaseFine + CurrentPursuitStats.TotalFines + VehicleFine) * Multiplier);
}

FMGBustConsequences UMGPoliceSubsystem::CalculateBustConsequences() const
{
	FMGBustConsequences Consequences;

	Consequences.bVehicleImpounded = true;
	Consequences.FineAmount = CalculateBustPenalty();

	// REP loss based on heat level (per GDD: -200 to -1000)
	switch (CurrentHeatLevel)
	{
		case EMGHeatLevel::Level1: Consequences.REPLost = 200; break;
		case EMGHeatLevel::Level2: Consequences.REPLost = 350; break;
		case EMGHeatLevel::Level3: Consequences.REPLost = 500; break;
		case EMGHeatLevel::Level4: Consequences.REPLost = 750; break;
		case EMGHeatLevel::Level5: Consequences.REPLost = 1000; break;
		default: Consequences.REPLost = 200; break;
	}

	Consequences.ImpoundRetrievalCost = CalculateImpoundCost(PlayerVehicleValue);
	Consequences.DaysUntilAuction = 7;
	Consequences.ImpoundTime = FDateTime::Now();
	Consequences.ImpoundedVehicleID = PlayerVehicleID;
	Consequences.bCriminalRecordUpdated = true;

	return Consequences;
}

// ==========================================
// COOLDOWN ZONES
// ==========================================

void UMGPoliceSubsystem::RegisterCooldownZone(const FMGCooldownZone& Zone)
{
	// Remove existing zone with same ID
	UnregisterCooldownZone(Zone.ZoneID);
	CooldownZones.Add(Zone);

	UE_LOG(LogTemp, Log, TEXT("Police: Cooldown zone '%s' registered at %s"),
		*Zone.ZoneName, *Zone.Location.ToString());
}

void UMGPoliceSubsystem::UnregisterCooldownZone(FName ZoneID)
{
	for (int32 i = CooldownZones.Num() - 1; i >= 0; --i)
	{
		if (CooldownZones[i].ZoneID == ZoneID)
		{
			CooldownZones.RemoveAt(i);
			break;
		}
	}
}

bool UMGPoliceSubsystem::GetNearestCooldownZone(FMGCooldownZone& OutZone, float& OutDistance) const
{
	if (!PlayerVehicle.IsValid() || CooldownZones.Num() == 0)
	{
		return false;
	}

	FVector PlayerLocation = PlayerVehicle->GetActorLocation();
	float NearestDistance = MAX_FLT;
	int32 NearestIndex = -1;

	for (int32 i = 0; i < CooldownZones.Num(); ++i)
	{
		const FMGCooldownZone& Zone = CooldownZones[i];
		if (!Zone.bIsUnlocked) continue;
		if (static_cast<uint8>(CurrentHeatLevel) > static_cast<uint8>(Zone.MaxEffectiveHeatLevel)) continue;

		float Distance = FVector::Dist(PlayerLocation, Zone.Location);
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestIndex = i;
		}
	}

	if (NearestIndex >= 0)
	{
		OutZone = CooldownZones[NearestIndex];
		OutDistance = NearestDistance;
		return true;
	}

	return false;
}

// ==========================================
// IMPOUND SYSTEM
// ==========================================

void UMGPoliceSubsystem::ImpoundVehicle(FGuid VehicleID, int64 VehicleValue, const FString& DisplayName)
{
	FMGImpoundedVehicle ImpoundData;
	ImpoundData.VehicleID = VehicleID;
	ImpoundData.ImpoundTime = FDateTime::Now();
	ImpoundData.BaseRetrievalCost = CalculateImpoundCost(VehicleValue);
	ImpoundData.DailyStorageFee = 500;
	ImpoundData.DaysUntilAuction = 7;
	ImpoundData.VehicleDisplayName = DisplayName;
	ImpoundData.VehicleValue = VehicleValue;

	ImpoundedVehicles.Add(ImpoundData);
	OnVehicleImpounded.Broadcast(ImpoundData);

	UE_LOG(LogTemp, Log, TEXT("Police: Vehicle '%s' impounded. Retrieval cost: %lld"),
		*DisplayName, ImpoundData.BaseRetrievalCost);
}

int64 UMGPoliceSubsystem::GetVehicleRetrievalCost(FGuid VehicleID) const
{
	for (const FMGImpoundedVehicle& Vehicle : ImpoundedVehicles)
	{
		if (Vehicle.VehicleID == VehicleID)
		{
			// Base cost + daily storage fees
			FTimespan TimeSinceImpound = FDateTime::Now() - Vehicle.ImpoundTime;
			int32 DaysImpounded = FMath::Max(1, static_cast<int32>(TimeSinceImpound.GetTotalDays()));
			return Vehicle.BaseRetrievalCost + (Vehicle.DailyStorageFee * DaysImpounded);
		}
	}
	return 0;
}

bool UMGPoliceSubsystem::RetrieveVehicle(FGuid VehicleID)
{
	for (int32 i = ImpoundedVehicles.Num() - 1; i >= 0; --i)
	{
		if (ImpoundedVehicles[i].VehicleID == VehicleID)
		{
			ImpoundedVehicles.RemoveAt(i);
			OnVehicleRetrieved.Broadcast(VehicleID);
			return true;
		}
	}
	return false;
}

bool UMGPoliceSubsystem::IsVehicleImpounded(FGuid VehicleID) const
{
	for (const FMGImpoundedVehicle& Vehicle : ImpoundedVehicles)
	{
		if (Vehicle.VehicleID == VehicleID)
		{
			return true;
		}
	}
	return false;
}

void UMGPoliceSubsystem::ProcessExpiredImpounds()
{
	FDateTime Now = FDateTime::Now();

	for (int32 i = ImpoundedVehicles.Num() - 1; i >= 0; --i)
	{
		FTimespan TimeSinceImpound = Now - ImpoundedVehicles[i].ImpoundTime;
		if (TimeSinceImpound.GetTotalDays() >= ImpoundedVehicles[i].DaysUntilAuction)
		{
			// Vehicle is auctioned off (lost forever)
			UE_LOG(LogTemp, Warning, TEXT("Police: Vehicle '%s' was auctioned after %d days in impound!"),
				*ImpoundedVehicles[i].VehicleDisplayName, ImpoundedVehicles[i].DaysUntilAuction);
			ImpoundedVehicles.RemoveAt(i);
		}
	}
}

// ==========================================
// PLAYER STATE
// ==========================================

void UMGPoliceSubsystem::SetPlayerVehicle(AMGVehiclePawn* Vehicle)
{
	PlayerVehicle = Vehicle;
	if (Vehicle)
	{
		PlayerLastKnownPosition = Vehicle->GetActorLocation();
	}
}

bool UMGPoliceSubsystem::CanPoliceCurrentlySeePlayer() const
{
	for (const FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.bHasVisualOnPlayer && Unit.Behavior != EMGPoliceBehavior::Disabled)
		{
			return true;
		}
	}
	return false;
}

float UMGPoliceSubsystem::GetDistanceToNearestPursuer() const
{
	float NearestDistance = -1.0f;

	for (const FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.Behavior != EMGPoliceBehavior::Disabled &&
			(Unit.Behavior == EMGPoliceBehavior::Pursuing ||
			 Unit.Behavior == EMGPoliceBehavior::Ramming ||
			 Unit.Behavior == EMGPoliceBehavior::Boxing ||
			 Unit.Behavior == EMGPoliceBehavior::PITManeuver))
		{
			if (NearestDistance < 0.0f || Unit.DistanceToPlayer < NearestDistance)
			{
				NearestDistance = Unit.DistanceToPlayer;
			}
		}
	}

	return NearestDistance;
}

// ==========================================
// RACING INTEGRATION
// ==========================================

void UMGPoliceSubsystem::OnStreetRaceStarted(FVector RaceLocation)
{
	bStreetRaceActive = true;
	RaceStartLocation = RaceLocation;

	// Street racing is a violation
	ReportViolation(EMGViolationType::StreetRacing, RaceLocation);

	UE_LOG(LogTemp, Log, TEXT("Police: Street race detected at %s! Heat multiplier active."),
		*RaceLocation.ToString());
}

void UMGPoliceSubsystem::OnStreetRaceEnded()
{
	bStreetRaceActive = false;
	RaceStartLocation = FVector::ZeroVector;

	UE_LOG(LogTemp, Log, TEXT("Police: Street race ended. Normal heat accumulation resumed."));
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGPoliceSubsystem::SetPoliceEnabled(bool bEnabled)
{
	bPoliceEnabled = bEnabled;

	if (!bEnabled)
	{
		ClearHeat();
	}
}

void UMGPoliceSubsystem::SetCooldownDuration(float BaseDuration)
{
	CooldownDuration = FMath::Max(5.0f, BaseDuration);
}

void UMGPoliceSubsystem::SetAggressionMultiplier(float Multiplier)
{
	AggressionMultiplier = FMath::Clamp(Multiplier, 0.5f, 3.0f);

	// Update existing units
	for (FMGPoliceUnitState& Unit : ActiveUnits)
	{
		Unit.AggressionLevel = AggressionMultiplier * (1.0f + CriminalRecord.NotorietyLevel * 0.3f);
	}
}

// ==========================================
// STATS
// ==========================================

int32 UMGPoliceSubsystem::GetHighestHeatLevelReached() const
{
	return static_cast<int32>(CriminalRecord.HighestHeatReached);
}

// ==========================================
// UPDATE FUNCTIONS
// ==========================================

void UMGPoliceSubsystem::UpdatePursuit(float MGDeltaTime)
{
	if (!bInPursuit)
	{
		return;
	}

	// Update pursuit duration
	CurrentPursuitStats.Duration += DeltaTime;
	CriminalRecord.TotalPursuitTime += DeltaTime;

	// Check if player is still being seen
	if (!CanPoliceCurrentlySeePlayer())
	{
		TimeSincePlayerSeen += DeltaTime;

		// Start cooldown if lost for enough time (varies by heat level)
		float LostTimeThreshold = 3.0f + (static_cast<float>(CurrentHeatLevel) * 0.5f);
		if (TimeSincePlayerSeen >= LostTimeThreshold && !bInCooldown)
		{
			StartCooldown();
		}
	}
	else
	{
		TimeSincePlayerSeen = 0.0f;

		// Cancel cooldown if seen again
		if (bInCooldown)
		{
			InterruptCooldown();
		}
	}

	// Update player position tracking
	if (PlayerVehicle.IsValid())
	{
		FVector NewPosition = PlayerVehicle->GetActorLocation();

		// Track distance traveled during pursuit
		float DistanceTraveled = FVector::Dist(PlayerLastKnownPosition, NewPosition);
		CurrentPursuitStats.TotalDistance += DistanceTraveled / 100.0f; // Convert to meters

		// Track top speed
		float CurrentSpeed = PlayerVehicle->GetRuntimeState().SpeedMPH;
		if (CurrentSpeed > CurrentPursuitStats.TopSpeed)
		{
			CurrentPursuitStats.TopSpeed = CurrentSpeed;
		}

		PlayerLastKnownPosition = NewPosition;
	}

	// Spawn reinforcements periodically
	TimeSinceLastSpawn += DeltaTime;
	if (TimeSinceLastSpawn >= UnitSpawnInterval)
	{
		SpawnUnitsForHeatLevel();
		TimeSinceLastSpawn = 0.0f;
	}
}

void UMGPoliceSubsystem::UpdateCooldown(float MGDeltaTime)
{
	if (!bInCooldown)
	{
		return;
	}

	// Apply cooldown zone multiplier if in zone
	float Multiplier = bInCooldownZone ? CurrentCooldownZone.CooldownMultiplier : 1.0f;

	CooldownTimer += DeltaTime * Multiplier;

	// Check if cooldown complete
	if (CooldownTimer >= CooldownDuration)
	{
		CurrentPursuitStats.BountyEarned = CalculateBountyReward();
		OnCooldownComplete.Broadcast();
		EndPursuit(EMGPursuitOutcome::Escaped);
	}
}

void UMGPoliceSubsystem::UpdatePoliceAI(float MGDeltaTime)
{
	if (!bInPursuit)
	{
		return;
	}

	// Update each unit
	for (FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.Behavior == EMGPoliceBehavior::Disabled)
		{
			continue;
		}

		Unit.TimeInPursuit += DeltaTime;

		// Update distance to player
		if (PlayerVehicle.IsValid() && Unit.UnitActor.IsValid())
		{
			Unit.DistanceToPlayer = FVector::Dist(
				Unit.UnitActor->GetActorLocation(),
				PlayerVehicle->GetActorLocation()
			);

			// Simple line of sight check based on distance
			// Real implementation would use raycasting
			Unit.bHasVisualOnPlayer = Unit.DistanceToPlayer < 50000.0f; // 500m

			if (Unit.bHasVisualOnPlayer)
			{
				Unit.LastKnownPlayerPosition = PlayerVehicle->GetActorLocation();
				Unit.TimeSinceSawPlayer = 0.0f;
			}
			else
			{
				Unit.TimeSinceSawPlayer += DeltaTime;
			}
		}

		// Behavior state machine
		switch (Unit.Behavior)
		{
			case EMGPoliceBehavior::Alerted:
				if (Unit.bHasVisualOnPlayer)
				{
					Unit.Behavior = EMGPoliceBehavior::Pursuing;
				}
				break;

			case EMGPoliceBehavior::Pursuing:
				// Escalate to ramming at higher heat levels when close
				if (CurrentHeatLevel >= EMGHeatLevel::Level3 &&
					Unit.DistanceToPlayer < 5000.0f && // 50m
					Unit.bHasVisualOnPlayer)
				{
					// Randomly choose between ramming and PIT based on aggression
					if (FMath::FRand() < Unit.AggressionLevel * 0.3f)
					{
						Unit.Behavior = EMGPoliceBehavior::Ramming;
					}
					else if (CurrentHeatLevel >= EMGHeatLevel::Level4)
					{
						Unit.Behavior = EMGPoliceBehavior::PITManeuver;
					}
				}
				// Execute boxing tactic if assigned
				else if (Unit.AssignedTactic == EMGPoliceTactic::BoxingManeuver &&
						 GetActiveUnitCount() >= 3)
				{
					Unit.Behavior = EMGPoliceBehavior::Boxing;
				}
				break;

			case EMGPoliceBehavior::Ramming:
			case EMGPoliceBehavior::PITManeuver:
				// Return to pursuing if lost visual or distance increased
				if (!Unit.bHasVisualOnPlayer || Unit.DistanceToPlayer > 10000.0f)
				{
					Unit.Behavior = EMGPoliceBehavior::Pursuing;
				}
				break;

			case EMGPoliceBehavior::Boxing:
				// Continue boxing until player escapes or tactic changes
				if (Unit.AssignedTactic != EMGPoliceTactic::BoxingManeuver)
				{
					Unit.Behavior = EMGPoliceBehavior::Pursuing;
				}
				break;

			default:
				break;
		}
	}
}

void UMGPoliceSubsystem::UpdateBustedState(float MGDeltaTime)
{
	if (!bInPursuit || !PlayerVehicle.IsValid())
	{
		return;
	}

	// Check if player is stopped and cops are nearby
	float PlayerSpeed = PlayerVehicle->GetRuntimeState().SpeedMPH;
	bool bPlayerStopped = PlayerSpeed < 5.0f;
	bool bCopsNearby = false;

	for (const FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.Behavior != EMGPoliceBehavior::Disabled && Unit.DistanceToPlayer < 2000.0f) // 20m
		{
			bCopsNearby = true;
			break;
		}
	}

	if (bPlayerStopped && bCopsNearby)
	{
		if (!bGettingBusted)
		{
			bGettingBusted = true;
			BustedTimer = 0.0f;
			UE_LOG(LogTemp, Log, TEXT("Police: Bust in progress..."));
		}

		BustedTimer += DeltaTime;

		// Bust duration varies by heat level (higher = faster bust)
		float AdjustedBustDuration = BustedDuration / (1.0f + static_cast<float>(CurrentHeatLevel) * 0.2f);
		BustedProgress = FMath::Clamp(BustedTimer / AdjustedBustDuration, 0.0f, 1.0f);

		OnBustProgressUpdated.Broadcast(BustedProgress);

		if (BustedProgress >= 1.0f)
		{
			PlayerBusted();
		}
	}
	else if (bGettingBusted)
	{
		// Player escaped during bust
		CancelBusted();
	}
}

void UMGPoliceSubsystem::CheckCooldownZones()
{
	if (!PlayerVehicle.IsValid())
	{
		return;
	}

	FVector PlayerLocation = PlayerVehicle->GetActorLocation();
	bool bWasInZone = bInCooldownZone;

	bInCooldownZone = false;

	for (const FMGCooldownZone& Zone : CooldownZones)
	{
		if (!Zone.bIsUnlocked) continue;

		// Check if zone is effective at current heat level
		if (static_cast<uint8>(CurrentHeatLevel) > static_cast<uint8>(Zone.MaxEffectiveHeatLevel))
		{
			continue;
		}

		float Distance = FVector::Dist(PlayerLocation, Zone.Location);
		if (Distance <= Zone.Radius)
		{
			bInCooldownZone = true;
			CurrentCooldownZone = Zone;

			if (!bWasInZone)
			{
				OnEnteredCooldownZone.Broadcast(Zone);
				UE_LOG(LogTemp, Log, TEXT("Police: Entered cooldown zone '%s'"), *Zone.ZoneName);
			}
			break;
		}
	}

	if (bWasInZone && !bInCooldownZone)
	{
		OnExitedCooldownZone.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("Police: Exited cooldown zone"));
	}
}

void UMGPoliceSubsystem::SpawnUnitsForHeatLevel()
{
	int32 DesiredUnits = GetMaxUnitsForHeatLevel(CurrentHeatLevel);
	int32 CurrentUnits = GetActiveUnitCount();

	// Get available unit types for this heat level
	TArray<EMGPoliceUnitType> AvailableTypes = GetAvailableUnitTypes(CurrentHeatLevel);

	// Spawn more units if needed
	while (CurrentUnits < DesiredUnits && AvailableTypes.Num() > 0)
	{
		// Weighted random selection favoring appropriate unit types
		EMGPoliceUnitType UnitType = AvailableTypes[FMath::RandRange(0, AvailableTypes.Num() - 1)];

		// Find optimal spawn location
		FVector SpawnLocation = FindOptimalSpawnLocation();

		if (SpawnPoliceUnit(UnitType, SpawnLocation) != -1)
		{
			CurrentUnits++;
		}
		else
		{
			break; // Spawn failed, don't keep trying
		}
	}

	// Deploy helicopter at heat level 5
	if (CurrentHeatLevel == EMGHeatLevel::Level5 && !bHelicopterActive)
	{
		DeployHelicopter();
	}
}

void UMGPoliceSubsystem::UpdatePursuitStats(float MGDeltaTime)
{
	// Update longest pursuit record
	if (CurrentPursuitStats.Duration > CriminalRecord.LongestPursuitSurvived)
	{
		CriminalRecord.LongestPursuitSurvived = CurrentPursuitStats.Duration;
	}
}

void UMGPoliceSubsystem::EvaluateTactics()
{
	if (!bInPursuit)
	{
		return;
	}

	// Evaluate best tactic based on current situation
	EMGPoliceTactic NewTactic = EMGPoliceTactic::StandardPursuit;

	int32 ActiveUnitCount = GetActiveUnitCount();
	int32 UnitsWithVisual = GetUnitsWithVisual().Num();

	// High heat with many units - try boxing
	if (CurrentHeatLevel >= EMGHeatLevel::Level3 &&
		ActiveUnitCount >= 4 &&
		UnitsWithVisual >= 2)
	{
		if (IsTacticAvailable(EMGPoliceTactic::BoxingManeuver))
		{
			NewTactic = EMGPoliceTactic::BoxingManeuver;
		}
	}

	// Very high heat - roadblocks
	if (CurrentHeatLevel >= EMGHeatLevel::Level4)
	{
		// Randomly decide to set up roadblock
		if (FMath::FRand() < 0.3f && IsTacticAvailable(EMGPoliceTactic::RoadblockAhead))
		{
			NewTactic = EMGPoliceTactic::RoadblockAhead;

			// Actually spawn a roadblock
			if (PlayerVehicle.IsValid())
			{
				FVector PlayerForward = PlayerVehicle->GetActorForwardVector();
				FVector RoadblockLocation = PlayerLastKnownPosition + PlayerForward * 30000.0f; // 300m ahead
				SpawnRoadblock(RoadblockLocation, PlayerForward);
			}
		}
	}

	// Deploy spike strips at level 3+
	if (CurrentHeatLevel >= EMGHeatLevel::Level3 && FMath::FRand() < 0.2f)
	{
		if (PlayerVehicle.IsValid())
		{
			FVector PlayerForward = PlayerVehicle->GetActorForwardVector();
			FVector SpikeLocation = PlayerLastKnownPosition + PlayerForward * 20000.0f; // 200m ahead
			DeploySpikeStrip(SpikeLocation, PlayerForward);
		}
	}

	// Aerial support at max heat
	if (CurrentHeatLevel == EMGHeatLevel::Level5 && bHelicopterActive)
	{
		NewTactic = EMGPoliceTactic::AerialSupport;
	}

	if (NewTactic != CurrentTactic)
	{
		RequestTactic(NewTactic);
	}
}

void UMGPoliceSubsystem::UpdateHeatDecay(float MGDeltaTime)
{
	// Heat decays slowly when not in pursuit
	float DecayAmount = BaseHeatDecayRate * DeltaTime;

	CurrentHeatPoints = FMath::Max(0, CurrentHeatPoints - FMath::RoundToInt(DecayAmount));

	// Update heat level if threshold crossed
	EMGHeatLevel NewLevel = CalculateHeatLevel();
	if (NewLevel != CurrentHeatLevel)
	{
		SetHeatLevel(NewLevel);
	}
}

int64 UMGPoliceSubsystem::CalculateBountyReward() const
{
	// Bounty increases with heat level and pursuit duration
	int64 BaseBounty = 0;
	switch (CurrentHeatLevel)
	{
		case EMGHeatLevel::Level1: BaseBounty = 500; break;
		case EMGHeatLevel::Level2: BaseBounty = 1500; break;
		case EMGHeatLevel::Level3: BaseBounty = 3500; break;
		case EMGHeatLevel::Level4: BaseBounty = 7500; break;
		case EMGHeatLevel::Level5: BaseBounty = 15000; break;
		default: BaseBounty = 250; break;
	}

	// Duration bonus (more time = more bounty)
	float DurationMultiplier = 1.0f + (CurrentPursuitStats.Duration / 60.0f) * 0.25f; // +25% per minute

	// Bonus for cops disabled
	int64 DisabledBonus = CurrentPursuitStats.CopsDisabled * 500;

	// Bonus for roadblocks evaded
	int64 RoadblockBonus = CurrentPursuitStats.RoadblocksEvaded * 250;

	// Bonus for evading helicopter
	int64 HeliBonus = CurrentPursuitStats.bHelicopterDeployed ? 2500 : 0;

	return static_cast<int64>(BaseBounty * DurationMultiplier) + DisabledBonus + RoadblockBonus + HeliBonus;
}

void UMGPoliceSubsystem::UpdateCriminalRecord(EMGPursuitOutcome Outcome)
{
	if (Outcome == EMGPursuitOutcome::Escaped)
	{
		CriminalRecord.TotalEscapes++;
		CriminalRecord.TotalBountyEarned += CurrentPursuitStats.BountyEarned;
		CriminalRecord.TotalRoadblocksEvaded += CurrentPursuitStats.RoadblocksEvaded;

		// Escaping increases notoriety
		CriminalRecord.NotorietyLevel = FMath::Clamp(
			CriminalRecord.NotorietyLevel + 0.05f * static_cast<float>(CurrentHeatLevel),
			0.0f, 1.0f
		);
	}
	else if (Outcome == EMGPursuitOutcome::Busted)
	{
		CriminalRecord.TotalBusts++;
		CriminalRecord.TotalFinesPaid += CalculateBustPenalty();

		// Getting busted slightly reduces notoriety (paid debt to society)
		CriminalRecord.NotorietyLevel = FMath::Max(0.0f, CriminalRecord.NotorietyLevel - 0.1f);
	}

	CriminalRecord.TotalCopsDisabled += CurrentPursuitStats.CopsDisabled;
}

int64 UMGPoliceSubsystem::CalculateImpoundCost(int64 VehicleValue) const
{
	// Base impound fee + percentage of vehicle value
	int64 BaseFee = 1000;
	int64 ValuePercentage = static_cast<int64>(VehicleValue * 0.05f); // 5% of value

	return BaseFee + ValuePercentage;
}

TArray<EMGPoliceUnitType> UMGPoliceSubsystem::GetAvailableUnitTypes(EMGHeatLevel Level) const
{
	TArray<EMGPoliceUnitType> Types;

	// Patrol cars always available
	Types.Add(EMGPoliceUnitType::Patrol);

	if (Level >= EMGHeatLevel::Level2)
	{
		Types.Add(EMGPoliceUnitType::Interceptor);
	}

	if (Level >= EMGHeatLevel::Level3)
	{
		Types.Add(EMGPoliceUnitType::SUV);
		Types.Add(EMGPoliceUnitType::Undercover);
	}

	if (Level >= EMGHeatLevel::Level4)
	{
		Types.Add(EMGPoliceUnitType::Roadblock);
		Types.Add(EMGPoliceUnitType::SpikeStrip);
	}

	if (Level >= EMGHeatLevel::Level5)
	{
		Types.Add(EMGPoliceUnitType::Rhino);
		// Helicopter handled separately
	}

	return Types;
}

FVector UMGPoliceSubsystem::FindOptimalSpawnLocation() const
{
	FVector SpawnLocation = PlayerLastKnownPosition;

	if (PlayerVehicle.IsValid())
	{
		FVector PlayerForward = PlayerVehicle->GetActorForwardVector();
		FVector PlayerRight = PlayerVehicle->GetActorRightVector();

		// Primarily spawn behind the player
		float BehindDistance = FMath::RandRange(8000.0f, 15000.0f); // 80-150m behind
		float SideOffset = FMath::RandRange(-5000.0f, 5000.0f); // Up to 50m to either side

		SpawnLocation = PlayerLastKnownPosition - PlayerForward * BehindDistance + PlayerRight * SideOffset;

		// Occasionally spawn from sides or ahead at higher heat levels
		if (CurrentHeatLevel >= EMGHeatLevel::Level3 && FMath::FRand() < 0.3f)
		{
			// Spawn from the side
			float SideDirection = FMath::RandBool() ? 1.0f : -1.0f;
			SpawnLocation = PlayerLastKnownPosition + PlayerRight * SideDirection * FMath::RandRange(5000.0f, 10000.0f);
		}
	}

	return SpawnLocation;
}

bool UMGPoliceSubsystem::ShouldChangeTactic() const
{
	// Logic to determine if current tactic is working
	// Could analyze pursuit progress, player behavior, etc.
	return TimeSinceTacticEvaluation >= TacticEvaluationInterval;
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Police/MGPoliceSubsystem.h"
#include "Police/MGPoliceUnit.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UMGPoliceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentHeatLevel = EMGHeatLevel::None;
	CurrentHeatPoints = 0;
	bInPursuit = false;
	bInCooldown = false;
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

	// Start update timer
	InWorld.GetTimerManager().SetTimer(
		UpdateTimerHandle,
		[this]()
		{
			if (bPoliceEnabled)
			{
				const float DeltaTime = 0.1f;
				UpdatePursuit(DeltaTime);
				UpdateCooldown(DeltaTime);
				UpdatePoliceAI(DeltaTime);
				UpdateBustedState(DeltaTime);
				CheckCooldownZones();
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

	CurrentHeatPoints += Amount;

	EMGHeatLevel OldLevel = CurrentHeatLevel;
	EMGHeatLevel NewLevel = CalculateHeatLevel();

	if (NewLevel != OldLevel)
	{
		SetHeatLevel(NewLevel);
	}

	// Track highest heat
	if (static_cast<uint8>(NewLevel) > static_cast<uint8>(HighestHeatReached))
	{
		HighestHeatReached = NewLevel;
	}

	// Start pursuit if we hit Level 1+
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

		// Adjust police presence
		SpawnUnitsForHeatLevel();
	}
}

float UMGPoliceSubsystem::GetHeatDecayProgress() const
{
	if (!bInCooldown)
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

	DespawnAllUnits();
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

EMGHeatLevel UMGPoliceSubsystem::CalculateHeatLevel() const
{
	if (CurrentHeatPoints >= HeatLevel5Threshold)
	{
		return EMGHeatLevel::Level5;
	}
	if (CurrentHeatPoints >= HeatLevel4Threshold)
	{
		return EMGHeatLevel::Level4;
	}
	if (CurrentHeatPoints >= HeatLevel3Threshold)
	{
		return EMGHeatLevel::Level3;
	}
	if (CurrentHeatPoints >= HeatLevel2Threshold)
	{
		return EMGHeatLevel::Level2;
	}
	if (CurrentHeatPoints >= HeatLevel1Threshold)
	{
		return EMGHeatLevel::Level1;
	}
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

	OnPursuitStarted.Broadcast(CurrentHeatLevel);

	// Spawn initial units
	SpawnUnitsForHeatLevel();
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

	// Track stats
	if (CurrentPursuitStats.Duration > LongestPursuitTime)
	{
		LongestPursuitTime = CurrentPursuitStats.Duration;
	}

	if (Outcome == EMGPursuitOutcome::Escaped)
	{
		TotalEscapes++;
		TotalBountyEarned += CurrentPursuitStats.BountyEarned;
		OnPlayerEscaped.Broadcast(CurrentPursuitStats);
	}
	else if (Outcome == EMGPursuitOutcome::Busted)
	{
		TotalBusts++;
		TotalFinesPaid += CalculateBustPenalty();
		OnPlayerBusted.Broadcast(CurrentPursuitStats);
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
	float BaseCooldown = CooldownDuration;
	switch (CurrentHeatLevel)
	{
		case EMGHeatLevel::Level1: BaseCooldown = 20.0f; break;
		case EMGHeatLevel::Level2: BaseCooldown = 30.0f; break;
		case EMGHeatLevel::Level3: BaseCooldown = 45.0f; break;
		case EMGHeatLevel::Level4: BaseCooldown = 60.0f; break;
		case EMGHeatLevel::Level5: BaseCooldown = 90.0f; break;
		default: BaseCooldown = 15.0f; break;
	}

	CooldownDuration = BaseCooldown;

	OnCooldownStarted.Broadcast(CooldownDuration);
}

// ==========================================
// VIOLATIONS
// ==========================================

void UMGPoliceSubsystem::ReportViolation(EMGViolationType Type, FVector Location)
{
	int32 HeatGained = GetHeatForViolation(Type);
	int64 Fine = GetFineForViolation(Type);

	FMGViolationRecord Record;
	Record.Type = Type;
	Record.Timestamp = FDateTime::Now();
	Record.Location = Location;
	Record.HeatGained = HeatGained;
	Record.FineAmount = Fine;

	CurrentPursuitStats.Violations.Add(Record);
	CurrentPursuitStats.TotalFines += Fine;

	OnViolationCommitted.Broadcast(Record);

	AddHeat(HeatGained, Type);
}

int32 UMGPoliceSubsystem::GetHeatForViolation(EMGViolationType Type) const
{
	switch (Type)
	{
		case EMGViolationType::Speeding: return 25;
		case EMGViolationType::Reckless: return 50;
		case EMGViolationType::RunRedLight: return 30;
		case EMGViolationType::HitCivilian: return 75;
		case EMGViolationType::HitPolice: return 150;
		case EMGViolationType::EvadePursuit: return 100;
		case EMGViolationType::StreetRacing: return 200;
		case EMGViolationType::PropertyDamage: return 40;
		case EMGViolationType::WrongWay: return 35;
		case EMGViolationType::Nitrous: return 20;
		default: return 10;
	}
}

int64 UMGPoliceSubsystem::GetFineForViolation(EMGViolationType Type) const
{
	switch (Type)
	{
		case EMGViolationType::Speeding: return 500;
		case EMGViolationType::Reckless: return 1000;
		case EMGViolationType::RunRedLight: return 250;
		case EMGViolationType::HitCivilian: return 2500;
		case EMGViolationType::HitPolice: return 5000;
		case EMGViolationType::EvadePursuit: return 3000;
		case EMGViolationType::StreetRacing: return 2000;
		case EMGViolationType::PropertyDamage: return 1500;
		case EMGViolationType::WrongWay: return 500;
		case EMGViolationType::Nitrous: return 300;
		default: return 100;
	}
}

// ==========================================
// POLICE UNITS
// ==========================================

int32 UMGPoliceSubsystem::SpawnPoliceUnit(EMGPoliceUnitType UnitType, FVector SpawnLocation)
{
	// Don't spawn if at max for current heat level
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

	// Spawn the police vehicle actor
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// Get spawn rotation facing the player
		FRotator SpawnRotation = FRotator::ZeroRotator;
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			FVector ToPlayer = PlayerPawn->GetActorLocation() - SpawnLocation;
			ToPlayer.Z = 0.0f;
			SpawnRotation = ToPlayer.Rotation();
		}

		AMGPoliceUnit* SpawnedUnit = World->SpawnActor<AMGPoliceUnit>(
			AMGPoliceUnit::StaticClass(),
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		if (SpawnedUnit)
		{
			// Initialize the unit
			SpawnedUnit->InitializeUnit(NewUnit.UnitID, EMGPoliceState::Alerted);

			// Set pursuit target to player
			if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
			{
				SpawnedUnit->SetPursuitTarget(PlayerPawn);
				SpawnedUnit->StartPursuit();
			}

			NewUnit.UnitActor = SpawnedUnit;
		}
	}

	ActiveUnits.Add(NewUnit);

	OnPoliceUnitSpawned.Broadcast(NewUnit.UnitID, UnitType);

	return NewUnit.UnitID;
}

void UMGPoliceSubsystem::DespawnPoliceUnit(int32 UnitID)
{
	for (int32 i = ActiveUnits.Num() - 1; i >= 0; --i)
	{
		if (ActiveUnits[i].UnitID == UnitID)
		{
			// Destroy the actor
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
		case EMGHeatLevel::None: return 0;
		case EMGHeatLevel::Level1: return 2;
		case EMGHeatLevel::Level2: return 4;
		case EMGHeatLevel::Level3: return 6;
		case EMGHeatLevel::Level4: return 8;
		case EMGHeatLevel::Level5: return 12;
		default: return 0;
	}
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

	// TODO: Spawn roadblock actors
	SpawnPoliceUnit(EMGPoliceUnitType::Roadblock, Location);
}

void UMGPoliceSubsystem::DeploySpikeStrip(FVector Location, FVector Direction)
{
	// Only at heat level 3+
	if (CurrentHeatLevel < EMGHeatLevel::Level3)
	{
		return;
	}

	// TODO: Spawn spike strip actor
	SpawnPoliceUnit(EMGPoliceUnitType::SpikeStrip, Location);
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

	// Set heat level to busted
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
}

int64 UMGPoliceSubsystem::CalculateBustPenalty() const
{
	// Base fine plus all violations
	int64 BaseFine = 5000;

	// Multiplier based on heat level
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

	return static_cast<int64>((BaseFine + CurrentPursuitStats.TotalFines) * Multiplier);
}

// ==========================================
// COOLDOWN ZONES
// ==========================================

void UMGPoliceSubsystem::RegisterCooldownZone(const FMGCooldownZone& Zone)
{
	// Remove existing zone with same ID
	UnregisterCooldownZone(Zone.ZoneID);

	CooldownZones.Add(Zone);
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

// ==========================================
// PLAYER STATE
// ==========================================

void UMGPoliceSubsystem::SetPlayerVehicle(AMGVehiclePawn* Vehicle)
{
	PlayerVehicle = Vehicle;
}

bool UMGPoliceSubsystem::CanPoliceCurrentlySeePlayer() const
{
	// Check if any unit has visual
	for (const FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.bHasVisualOnPlayer && Unit.Behavior != EMGPoliceBehavior::Disabled)
		{
			return true;
		}
	}
	return false;
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

// ==========================================
// STATS
// ==========================================

int32 UMGPoliceSubsystem::GetHighestHeatLevelReached() const
{
	return static_cast<int32>(HighestHeatReached);
}

// ==========================================
// UPDATE FUNCTIONS
// ==========================================

void UMGPoliceSubsystem::UpdatePursuit(float DeltaTime)
{
	if (!bInPursuit)
	{
		return;
	}

	// Update pursuit duration
	CurrentPursuitStats.Duration += DeltaTime;

	// Check if player is still being seen
	if (!CanPoliceCurrentlySeePlayer())
	{
		TimeSincePlayerSeen += DeltaTime;

		// Start cooldown if lost for enough time
		if (TimeSincePlayerSeen >= 5.0f && !bInCooldown)
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
			bInCooldown = false;
			CooldownTimer = 0.0f;
		}
	}

	// Update player position tracking
	if (PlayerVehicle.IsValid())
	{
		PlayerLastKnownPosition = PlayerVehicle->GetActorLocation();

		// TODO: Track top speed, distance traveled, etc.
	}
}

void UMGPoliceSubsystem::UpdateCooldown(float DeltaTime)
{
	if (!bInCooldown)
	{
		return;
	}

	// Apply cooldown zone multiplier
	float Multiplier = bInCooldownZone ? CurrentCooldownZone.CooldownMultiplier : 1.0f;

	CooldownTimer += DeltaTime * Multiplier;

	// Check if cooldown complete
	if (CooldownTimer >= CooldownDuration)
	{
		// Calculate bounty based on heat level
		int64 Bounty = 0;
		switch (CurrentHeatLevel)
		{
			case EMGHeatLevel::Level1: Bounty = 1000; break;
			case EMGHeatLevel::Level2: Bounty = 2500; break;
			case EMGHeatLevel::Level3: Bounty = 5000; break;
			case EMGHeatLevel::Level4: Bounty = 10000; break;
			case EMGHeatLevel::Level5: Bounty = 25000; break;
			default: Bounty = 500; break;
		}

		CurrentPursuitStats.BountyEarned = Bounty;

		OnCooldownComplete.Broadcast();
		EndPursuit(EMGPursuitOutcome::Escaped);
	}
}

void UMGPoliceSubsystem::UpdatePoliceAI(float DeltaTime)
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

		// Update distance to player
		if (PlayerVehicle.IsValid())
		{
			Unit.DistanceToPlayer = FVector::Dist(
				Unit.UnitActor.IsValid() ? Unit.UnitActor->GetActorLocation() : FVector::ZeroVector,
				PlayerVehicle->GetActorLocation()
			);

			// Simple line of sight check (would need proper implementation)
			Unit.bHasVisualOnPlayer = Unit.DistanceToPlayer < 500.0f * 100.0f; // 500m

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
				// Escalate to ramming at higher heat levels
				if (CurrentHeatLevel >= EMGHeatLevel::Level3 && Unit.DistanceToPlayer < 50.0f * 100.0f)
				{
					Unit.Behavior = EMGPoliceBehavior::Ramming;
				}
				break;

			case EMGPoliceBehavior::Ramming:
				// If lost visual, go back to pursuing
				if (!Unit.bHasVisualOnPlayer)
				{
					Unit.Behavior = EMGPoliceBehavior::Pursuing;
				}
				break;

			default:
				break;
		}
	}
}

void UMGPoliceSubsystem::UpdateBustedState(float DeltaTime)
{
	if (!bInPursuit || !PlayerVehicle.IsValid())
	{
		return;
	}

	// Check if player is stopped and surrounded
	// TODO: Get actual vehicle speed
	float PlayerSpeed = 0.0f; // PlayerVehicle->GetVehicleSpeed();

	bool bPlayerStopped = PlayerSpeed < 5.0f;
	bool bCopsNearby = false;

	for (const FMGPoliceUnitState& Unit : ActiveUnits)
	{
		if (Unit.Behavior != EMGPoliceBehavior::Disabled && Unit.DistanceToPlayer < 20.0f * 100.0f)
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
		}

		BustedTimer += DeltaTime;
		BustedProgress = FMath::Clamp(BustedTimer / BustedDuration, 0.0f, 1.0f);

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
		float Distance = FVector::Dist(PlayerLocation, Zone.Location);
		if (Distance <= Zone.Radius)
		{
			bInCooldownZone = true;
			CurrentCooldownZone = Zone;

			if (!bWasInZone)
			{
				OnEnteredCooldownZone.Broadcast(Zone);
			}
			break;
		}
	}
}

void UMGPoliceSubsystem::SpawnUnitsForHeatLevel()
{
	int32 DesiredUnits = GetMaxUnitsForHeatLevel(CurrentHeatLevel);
	int32 CurrentUnits = GetActiveUnitCount();

	// Spawn more units if needed
	while (CurrentUnits < DesiredUnits)
	{
		// Determine unit type based on heat level
		EMGPoliceUnitType UnitType = EMGPoliceUnitType::Patrol;

		if (CurrentHeatLevel >= EMGHeatLevel::Level4)
		{
			// Mix of interceptors and SUVs at high heat
			UnitType = (FMath::RandBool()) ? EMGPoliceUnitType::Interceptor : EMGPoliceUnitType::SUV;
		}
		else if (CurrentHeatLevel >= EMGHeatLevel::Level2)
		{
			// Interceptors join at level 2
			UnitType = (FMath::RandRange(0, 2) == 0) ? EMGPoliceUnitType::Interceptor : EMGPoliceUnitType::Patrol;
		}

		// TODO: Calculate proper spawn location behind/around player
		FVector SpawnLocation = PlayerLastKnownPosition + FVector(FMath::RandRange(-1000.0f, 1000.0f) * 100.0f, FMath::RandRange(-1000.0f, 1000.0f) * 100.0f, 0.0f);

		SpawnPoliceUnit(UnitType, SpawnLocation);
		CurrentUnits++;
	}

	// Add helicopter at heat level 5
	if (CurrentHeatLevel == EMGHeatLevel::Level5)
	{
		bool bHasHeli = false;
		for (const FMGPoliceUnitState& Unit : ActiveUnits)
		{
			if (Unit.UnitType == EMGPoliceUnitType::Helicopter)
			{
				bHasHeli = true;
				break;
			}
		}

		if (!bHasHeli)
		{
			SpawnPoliceUnit(EMGPoliceUnitType::Helicopter, PlayerLastKnownPosition + FVector(0, 0, 500.0f * 100.0f));
		}
	}
}

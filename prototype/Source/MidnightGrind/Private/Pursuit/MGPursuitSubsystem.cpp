// Copyright Epic Games, Inc. All Rights Reserved.

#include "Pursuit/MGPursuitSubsystem.h"
#include "TimerManager.h"

void UMGPursuitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UnitCounter = 0;

	// Set up default pursuit config
	PursuitConfig.BaseEscapeTime = 15.0f;
	PursuitConfig.BustedThreshold = 100.0f;
	PursuitConfig.BustedFillRate = 25.0f;
	PursuitConfig.EscapeFillRate = 10.0f;
	PursuitConfig.EscapeDrainRate = 5.0f;
	PursuitConfig.MinDistanceForEscape = 5000.0f;
	PursuitConfig.CooldownDuration = 10.0f;
	PursuitConfig.VisualRange = 15000.0f;

	// Max units per intensity
	PursuitConfig.MaxUnitsPerIntensity.Add(EMGPursuitIntensity::Low, 2);
	PursuitConfig.MaxUnitsPerIntensity.Add(EMGPursuitIntensity::Medium, 4);
	PursuitConfig.MaxUnitsPerIntensity.Add(EMGPursuitIntensity::High, 6);
	PursuitConfig.MaxUnitsPerIntensity.Add(EMGPursuitIntensity::Extreme, 8);
	PursuitConfig.MaxUnitsPerIntensity.Add(EMGPursuitIntensity::Maximum, 10);

	// Intensity thresholds
	PursuitConfig.IntensityUpgradeThresholds.Add(EMGPursuitIntensity::Low, 30.0f);
	PursuitConfig.IntensityUpgradeThresholds.Add(EMGPursuitIntensity::Medium, 60.0f);
	PursuitConfig.IntensityUpgradeThresholds.Add(EMGPursuitIntensity::High, 120.0f);
	PursuitConfig.IntensityUpgradeThresholds.Add(EMGPursuitIntensity::Extreme, 180.0f);

	// Default scoring
	PursuitScoring.BaseEscapeBonus = 1000;
	PursuitScoring.PerUnitDisabledBonus = 500;
	PursuitScoring.PerRoadblockEvadedBonus = 750;
	PursuitScoring.PerSpikeStripEvadedBonus = 300;
	PursuitScoring.DurationMultiplierPerMinute = 0.25f;
	PursuitScoring.IntensityMultiplier = 1.5f;
	PursuitScoring.CleanEscapeBonus = 2000;
	PursuitScoring.NearMissBonus = 100;
	PursuitScoring.HelicopterEvadeBonus = 1500;

	// Initialize session stats
	SessionStats = FMGPursuitSessionStats();

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PursuitTickTimer,
			[this]()
			{
				TickPursuit(0.033f);
			},
			0.033f,
			true
		);
	}

	LoadPursuitData();
}

void UMGPursuitSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PursuitTickTimer);
	}

	SavePursuitData();
	Super::Deinitialize();
}

// ============================================================================
// Pursuit Control
// ============================================================================

void UMGPursuitSubsystem::StartPursuit(const FString& PlayerId, EMGPursuitIntensity InitialIntensity)
{
	if (IsPursuitActive(PlayerId))
	{
		return;
	}

	FMGPursuitStatus Status;
	Status.PlayerId = PlayerId;
	Status.State = EMGPursuitState::Searching;
	Status.Intensity = InitialIntensity;
	Status.PursuitDuration = 0.0f;
	Status.EscapeMeter = 0.0f;
	Status.BustedMeter = 0.0f;
	Status.InfractionMultiplier = 1.0f;

	ActivePursuits.Add(PlayerId, Status);

	// Initialize event list
	PursuitEvents.Add(PlayerId, TArray<FMGPursuitEvent>());

	// Update stats
	SessionStats.TotalPursuitsStarted++;

	OnPursuitStarted.Broadcast(PlayerId, InitialIntensity);
	RecordEvent(PlayerId, EMGPursuitEventType::Spotted, TEXT("Pursuit initiated"), FVector::ZeroVector);

	// Spawn initial units
	SpawnBackup(PlayerId);
}

void UMGPursuitSubsystem::EndPursuit(const FString& PlayerId, bool bEscaped)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	int32 FinalBounty = 0;

	if (bEscaped)
	{
		FinalBounty = CalculateEscapeScore(PlayerId);
		Status->State = EMGPursuitState::Escaped;
		SessionStats.TotalEscapes++;

		// Update longest pursuit
		if (Status->PursuitDuration > SessionStats.LongestPursuitDuration)
		{
			SessionStats.LongestPursuitDuration = Status->PursuitDuration;
		}

		// Add bounty earned
		SessionStats.TotalBountyEarned += FinalBounty;
	}
	else
	{
		FinalBounty = CalculateBustedPenalty(PlayerId);
		Status->State = EMGPursuitState::Busted;
		SessionStats.TotalBusted++;
		SessionStats.TotalBountyLost += Status->CurrentBounty;
	}

	OnPursuitEnded.Broadcast(PlayerId, bEscaped, FinalBounty);

	// Remove pursuit data
	ActivePursuits.Remove(PlayerId);
	ActiveRoadblocks.Remove(PlayerId);
	PursuitEvents.Remove(PlayerId);
}

void UMGPursuitSubsystem::SetPursuitIntensity(const FString& PlayerId, EMGPursuitIntensity Intensity)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	EMGPursuitIntensity OldIntensity = Status->Intensity;
	if (OldIntensity == Intensity)
	{
		return;
	}

	Status->Intensity = Intensity;

	// Update highest intensity
	if (Intensity > SessionStats.HighestIntensity)
	{
		SessionStats.HighestIntensity = Intensity;
	}

	OnPursuitIntensityChanged.Broadcast(PlayerId, OldIntensity, Intensity);
	RecordEvent(PlayerId, EMGPursuitEventType::IntensityIncreased,
	            FString::Printf(TEXT("Intensity increased to %d"), static_cast<int32>(Intensity)),
	            FVector::ZeroVector);

	// Spawn additional units for new intensity
	SpawnBackup(PlayerId);
}

void UMGPursuitSubsystem::IncreaseIntensity(const FString& PlayerId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	switch (Status->Intensity)
	{
		case EMGPursuitIntensity::Low:
			SetPursuitIntensity(PlayerId, EMGPursuitIntensity::Medium);
			break;
		case EMGPursuitIntensity::Medium:
			SetPursuitIntensity(PlayerId, EMGPursuitIntensity::High);
			break;
		case EMGPursuitIntensity::High:
			SetPursuitIntensity(PlayerId, EMGPursuitIntensity::Extreme);
			break;
		case EMGPursuitIntensity::Extreme:
			SetPursuitIntensity(PlayerId, EMGPursuitIntensity::Maximum);
			break;
		default:
			break;
	}
}

bool UMGPursuitSubsystem::IsPursuitActive(const FString& PlayerId) const
{
	const FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return false;
	}

	return Status->State != EMGPursuitState::Inactive &&
	       Status->State != EMGPursuitState::Escaped &&
	       Status->State != EMGPursuitState::Busted;
}

bool UMGPursuitSubsystem::IsInCooldown(const FString& PlayerId) const
{
	const FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	return Status && Status->State == EMGPursuitState::Cooldown;
}

// ============================================================================
// Status
// ============================================================================

FMGPursuitStatus UMGPursuitSubsystem::GetPursuitStatus(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		return *Found;
	}
	return FMGPursuitStatus();
}

EMGPursuitState UMGPursuitSubsystem::GetPursuitState(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		return Found->State;
	}
	return EMGPursuitState::Inactive;
}

EMGPursuitIntensity UMGPursuitSubsystem::GetPursuitIntensity(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		return Found->Intensity;
	}
	return EMGPursuitIntensity::Low;
}

float UMGPursuitSubsystem::GetEscapeMeter(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		return Found->EscapeMeter;
	}
	return 0.0f;
}

float UMGPursuitSubsystem::GetBustedMeter(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		return Found->BustedMeter;
	}
	return 0.0f;
}

int32 UMGPursuitSubsystem::GetBounty(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		return Found->CurrentBounty;
	}
	return 0;
}

float UMGPursuitSubsystem::GetCooldownRemaining(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		return Found->CooldownRemaining;
	}
	return 0.0f;
}

// ============================================================================
// Units
// ============================================================================

void UMGPursuitSubsystem::SpawnPursuitUnit(const FString& PlayerId, EMGPursuitRole Role, FVector SpawnLocation)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	// Check max units
	int32* MaxUnits = PursuitConfig.MaxUnitsPerIntensity.Find(Status->Intensity);
	if (MaxUnits && Status->ActiveUnits.Num() >= *MaxUnits)
	{
		return;
	}

	FMGPursuitUnit Unit;
	Unit.UnitId = GenerateUnitId();
	Unit.Role = Role;
	Unit.Location = SpawnLocation;
	Unit.TargetPlayerId = PlayerId;
	Unit.Health = 100.0f;
	Unit.MaxHealth = 100.0f;

	// Set tactic based on role
	switch (Role)
	{
		case EMGPursuitRole::Pursuer:
			Unit.CurrentTactic = EMGPursuitTactic::Follow;
			break;
		case EMGPursuitRole::Interceptor:
			Unit.CurrentTactic = EMGPursuitTactic::PitManeuver;
			break;
		case EMGPursuitRole::RoadBlock:
			Unit.CurrentTactic = EMGPursuitTactic::Roadblock;
			break;
		case EMGPursuitRole::Helicopter:
			Unit.CurrentTactic = EMGPursuitTactic::Helicopter;
			break;
		default:
			Unit.CurrentTactic = EMGPursuitTactic::Follow;
			break;
	}

	Status->ActiveUnits.Add(Unit);
	Status->TotalUnitsEngaged++;

	// Update most units engaged
	if (Status->ActiveUnits.Num() > SessionStats.MostUnitsEngagedAtOnce)
	{
		SessionStats.MostUnitsEngagedAtOnce = Status->ActiveUnits.Num();
	}

	OnUnitEngaged.Broadcast(PlayerId, Unit);
}

void UMGPursuitSubsystem::RemovePursuitUnit(const FString& PlayerId, const FString& UnitId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	for (int32 i = Status->ActiveUnits.Num() - 1; i >= 0; i--)
	{
		if (Status->ActiveUnits[i].UnitId == UnitId)
		{
			Status->ActiveUnits.RemoveAt(i);
			return;
		}
	}
}

void UMGPursuitSubsystem::DisableUnit(const FString& PlayerId, const FString& UnitId, float Damage)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	for (FMGPursuitUnit& Unit : Status->ActiveUnits)
	{
		if (Unit.UnitId == UnitId && !Unit.bIsDisabled)
		{
			Unit.Health = FMath::Max(0.0f, Unit.Health - Damage);

			if (Unit.Health <= 0.0f)
			{
				Unit.bIsDisabled = true;
				Status->UnitsDisabled++;
				SessionStats.TotalUnitsDisabled++;

				OnUnitDisabled.Broadcast(PlayerId, Unit);

				// Add bounty for takedown
				AddBounty(PlayerId, 250, TEXT("Unit disabled"));
			}
			return;
		}
	}
}

TArray<FMGPursuitUnit> UMGPursuitSubsystem::GetActiveUnits(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		TArray<FMGPursuitUnit> ActiveOnly;
		for (const FMGPursuitUnit& Unit : Found->ActiveUnits)
		{
			if (!Unit.bIsDisabled)
			{
				ActiveOnly.Add(Unit);
			}
		}
		return ActiveOnly;
	}
	return TArray<FMGPursuitUnit>();
}

int32 UMGPursuitSubsystem::GetActiveUnitCount(const FString& PlayerId) const
{
	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		int32 Count = 0;
		for (const FMGPursuitUnit& Unit : Found->ActiveUnits)
		{
			if (!Unit.bIsDisabled)
			{
				Count++;
			}
		}
		return Count;
	}
	return 0;
}

FMGPursuitUnit UMGPursuitSubsystem::GetClosestUnit(const FString& PlayerId, FVector Location) const
{
	FMGPursuitUnit ClosestUnit;
	float ClosestDistance = FLT_MAX;

	if (const FMGPursuitStatus* Found = ActivePursuits.Find(PlayerId))
	{
		for (const FMGPursuitUnit& Unit : Found->ActiveUnits)
		{
			if (Unit.bIsDisabled)
			{
				continue;
			}

			float Distance = FVector::Dist(Location, Unit.Location);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestUnit = Unit;
			}
		}
	}

	ClosestUnit.DistanceToTarget = ClosestDistance;
	return ClosestUnit;
}

// ============================================================================
// Tactics
// ============================================================================

void UMGPursuitSubsystem::DeployRoadblock(const FString& PlayerId, const FMGRoadblock& Roadblock)
{
	FMGRoadblock NewRoadblock = Roadblock;
	if (NewRoadblock.RoadblockId.IsEmpty())
	{
		NewRoadblock.RoadblockId = FString::Printf(TEXT("RB_%d"), ++UnitCounter);
	}
	NewRoadblock.bIsActive = true;

	TArray<FMGRoadblock>& Roadblocks = ActiveRoadblocks.FindOrAdd(PlayerId);
	Roadblocks.Add(NewRoadblock);

	OnRoadblockDeployed.Broadcast(PlayerId, NewRoadblock);
	RecordEvent(PlayerId, EMGPursuitEventType::RoadblockDeployed, TEXT("Roadblock deployed"), NewRoadblock.Location);
}

void UMGPursuitSubsystem::EvadeRoadblock(const FString& PlayerId, const FString& RoadblockId)
{
	TArray<FMGRoadblock>* Roadblocks = ActiveRoadblocks.Find(PlayerId);
	if (!Roadblocks)
	{
		return;
	}

	for (FMGRoadblock& Roadblock : *Roadblocks)
	{
		if (Roadblock.RoadblockId == RoadblockId && !Roadblock.bHasBeenEvaded)
		{
			Roadblock.bHasBeenEvaded = true;

			FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
			if (Status)
			{
				Status->RoadblocksEvaded++;
				SessionStats.TotalRoadblocksEvaded++;
				AddBounty(PlayerId, 500, TEXT("Roadblock evaded"));
			}

			OnRoadblockEvaded.Broadcast(PlayerId, RoadblockId);
			return;
		}
	}
}

void UMGPursuitSubsystem::HitRoadblock(const FString& PlayerId, const FString& RoadblockId, float Damage)
{
	TArray<FMGRoadblock>* Roadblocks = ActiveRoadblocks.Find(PlayerId);
	if (!Roadblocks)
	{
		return;
	}

	for (FMGRoadblock& Roadblock : *Roadblocks)
	{
		if (Roadblock.RoadblockId == RoadblockId)
		{
			Roadblock.VehiclesDamaged++;
			// Apply damage to player vehicle would be handled externally
			return;
		}
	}
}

void UMGPursuitSubsystem::CallHelicopter(const FString& PlayerId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status || Status->bHelicopterActive)
	{
		return;
	}

	Status->bHelicopterActive = true;

	// Spawn helicopter unit
	SpawnPursuitUnit(PlayerId, EMGPursuitRole::Helicopter, FVector(0, 0, 5000));

	OnHelicopterCalled.Broadcast(PlayerId);
	RecordEvent(PlayerId, EMGPursuitEventType::HelicopterCalled, TEXT("Helicopter called in"), FVector::ZeroVector);
}

void UMGPursuitSubsystem::EvadeHelicopter(const FString& PlayerId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status || !Status->bHelicopterActive)
	{
		return;
	}

	Status->bHelicopterActive = false;

	// Remove helicopter unit
	for (int32 i = Status->ActiveUnits.Num() - 1; i >= 0; i--)
	{
		if (Status->ActiveUnits[i].Role == EMGPursuitRole::Helicopter)
		{
			Status->ActiveUnits.RemoveAt(i);
		}
	}

	AddBounty(PlayerId, PursuitScoring.HelicopterEvadeBonus, TEXT("Helicopter evaded"));

	OnHelicopterEvaded.Broadcast(PlayerId);
}

void UMGPursuitSubsystem::HitSpikeStrip(const FString& PlayerId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	// Apply spike strip effect would be handled externally
	RecordEvent(PlayerId, EMGPursuitEventType::SpikeStripDeployed, TEXT("Hit spike strip"), FVector::ZeroVector);
}

TArray<FMGRoadblock> UMGPursuitSubsystem::GetActiveRoadblocks(const FString& PlayerId) const
{
	if (const TArray<FMGRoadblock>* Found = ActiveRoadblocks.Find(PlayerId))
	{
		TArray<FMGRoadblock> Active;
		for (const FMGRoadblock& Roadblock : *Found)
		{
			if (Roadblock.bIsActive && !Roadblock.bHasBeenEvaded)
			{
				Active.Add(Roadblock);
			}
		}
		return Active;
	}
	return TArray<FMGRoadblock>();
}

// ============================================================================
// Bounty
// ============================================================================

void UMGPursuitSubsystem::AddBounty(const FString& PlayerId, int32 Amount, const FString& Reason)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	int32 AdjustedAmount = FMath::RoundToInt(Amount * Status->InfractionMultiplier);
	Status->CurrentBounty += AdjustedAmount;

	// Update highest bounty
	if (Status->CurrentBounty > SessionStats.HighestBounty)
	{
		SessionStats.HighestBounty = Status->CurrentBounty;
	}

	OnBountyChanged.Broadcast(PlayerId, Status->CurrentBounty);
}

void UMGPursuitSubsystem::ResetBounty(const FString& PlayerId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (Status)
	{
		Status->CurrentBounty = 0;
		OnBountyChanged.Broadcast(PlayerId, 0);
	}
}

int32 UMGPursuitSubsystem::CollectBounty(const FString& PlayerId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return 0;
	}

	int32 Bounty = Status->CurrentBounty;
	Status->CurrentBounty = 0;
	return Bounty;
}

// ============================================================================
// Update
// ============================================================================

void UMGPursuitSubsystem::UpdatePursuit(const FString& PlayerId, FVector PlayerLocation, FVector PlayerVelocity, float DeltaTime)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	// Skip if not in active pursuit
	if (Status->State == EMGPursuitState::Escaped ||
	    Status->State == EMGPursuitState::Busted ||
	    Status->State == EMGPursuitState::Inactive)
	{
		return;
	}

	// Handle cooldown
	if (Status->State == EMGPursuitState::Cooldown)
	{
		Status->CooldownRemaining -= DeltaTime;
		if (Status->CooldownRemaining <= 0.0f)
		{
			EndPursuit(PlayerId, true);
		}
		return;
	}

	// Update duration
	Status->PursuitDuration += DeltaTime;

	// Check for intensity upgrade
	CheckIntensityUpgrade(PlayerId);

	// Update unit distances and visual status
	bool bAnyUnitHasVisual = false;
	float ClosestDistance = FLT_MAX;

	for (FMGPursuitUnit& Unit : Status->ActiveUnits)
	{
		if (Unit.bIsDisabled)
		{
			continue;
		}

		Unit.DistanceToTarget = FVector::Dist(PlayerLocation, Unit.Location);
		Unit.bHasVisual = Unit.DistanceToTarget < PursuitConfig.VisualRange;
		Unit.TimeEngaged += DeltaTime;

		if (Unit.bHasVisual)
		{
			bAnyUnitHasVisual = true;
		}

		if (Unit.DistanceToTarget < ClosestDistance)
		{
			ClosestDistance = Unit.DistanceToTarget;
		}
	}

	// Update pursuit state based on visual
	if (bAnyUnitHasVisual)
	{
		if (Status->State != EMGPursuitState::PursuitActive)
		{
			SetPursuitState(PlayerId, EMGPursuitState::PursuitActive);
		}
	}
	else
	{
		if (Status->State != EMGPursuitState::Cooldown)
		{
			SetPursuitState(PlayerId, EMGPursuitState::Searching);
		}
	}

	// Update escape and busted progress
	UpdateEscapeProgress(PlayerId, PlayerLocation, DeltaTime);
	UpdateBustedProgress(PlayerId, DeltaTime);
}

void UMGPursuitSubsystem::UpdateUnitAI(const FString& PlayerId, float DeltaTime)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	// AI would be handled by game code - this is placeholder
	for (FMGPursuitUnit& Unit : Status->ActiveUnits)
	{
		if (Unit.bIsDisabled)
		{
			continue;
		}

		// Update unit behavior based on tactic
		// This would interface with actual AI systems
	}
}

// ============================================================================
// Configuration
// ============================================================================

void UMGPursuitSubsystem::SetPursuitConfig(const FMGPursuitConfig& Config)
{
	PursuitConfig = Config;
}

FMGPursuitConfig UMGPursuitSubsystem::GetPursuitConfig() const
{
	return PursuitConfig;
}

void UMGPursuitSubsystem::SetPursuitScoring(const FMGPursuitScoring& Scoring)
{
	PursuitScoring = Scoring;
}

FMGPursuitScoring UMGPursuitSubsystem::GetPursuitScoring() const
{
	return PursuitScoring;
}

// ============================================================================
// Stats
// ============================================================================

FMGPursuitSessionStats UMGPursuitSubsystem::GetSessionStats() const
{
	return SessionStats;
}

void UMGPursuitSubsystem::ResetSessionStats()
{
	SessionStats = FMGPursuitSessionStats();
}

// ============================================================================
// Scoring
// ============================================================================

int32 UMGPursuitSubsystem::CalculateEscapeScore(const FString& PlayerId) const
{
	const FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return 0;
	}

	int32 Score = PursuitScoring.BaseEscapeBonus;

	// Units disabled bonus
	Score += Status->UnitsDisabled * PursuitScoring.PerUnitDisabledBonus;

	// Roadblocks evaded bonus
	Score += Status->RoadblocksEvaded * PursuitScoring.PerRoadblockEvadedBonus;

	// Spike strips evaded bonus
	Score += Status->SpikeStripsEvaded * PursuitScoring.PerSpikeStripEvadedBonus;

	// Duration multiplier
	float DurationMinutes = Status->PursuitDuration / 60.0f;
	float DurationMultiplier = 1.0f + (DurationMinutes * PursuitScoring.DurationMultiplierPerMinute);
	Score = FMath::RoundToInt(Score * DurationMultiplier);

	// Intensity multiplier
	float IntensityMult = 1.0f;
	switch (Status->Intensity)
	{
		case EMGPursuitIntensity::Medium:
			IntensityMult = PursuitScoring.IntensityMultiplier;
			break;
		case EMGPursuitIntensity::High:
			IntensityMult = PursuitScoring.IntensityMultiplier * 1.5f;
			break;
		case EMGPursuitIntensity::Extreme:
			IntensityMult = PursuitScoring.IntensityMultiplier * 2.0f;
			break;
		case EMGPursuitIntensity::Maximum:
			IntensityMult = PursuitScoring.IntensityMultiplier * 3.0f;
			break;
		default:
			break;
	}
	Score = FMath::RoundToInt(Score * IntensityMult);

	// Add bounty
	Score += Status->CurrentBounty;

	return Score;
}

int32 UMGPursuitSubsystem::CalculateBustedPenalty(const FString& PlayerId) const
{
	const FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return 0;
	}

	// Lose all bounty plus additional penalty based on intensity
	int32 Penalty = Status->CurrentBounty;

	switch (Status->Intensity)
	{
		case EMGPursuitIntensity::Low:
			Penalty += 500;
			break;
		case EMGPursuitIntensity::Medium:
			Penalty += 1000;
			break;
		case EMGPursuitIntensity::High:
			Penalty += 2500;
			break;
		case EMGPursuitIntensity::Extreme:
			Penalty += 5000;
			break;
		case EMGPursuitIntensity::Maximum:
			Penalty += 10000;
			break;
	}

	return Penalty;
}

// ============================================================================
// Events
// ============================================================================

TArray<FMGPursuitEvent> UMGPursuitSubsystem::GetPursuitEvents(const FString& PlayerId) const
{
	if (const TArray<FMGPursuitEvent>* Found = PursuitEvents.Find(PlayerId))
	{
		return *Found;
	}
	return TArray<FMGPursuitEvent>();
}

// ============================================================================
// Save/Load
// ============================================================================

void UMGPursuitSubsystem::SavePursuitData()
{
	// Placeholder - would serialize to save game
}

void UMGPursuitSubsystem::LoadPursuitData()
{
	// Placeholder - would deserialize from save game
}

// ============================================================================
// Protected Methods
// ============================================================================

void UMGPursuitSubsystem::TickPursuit(float DeltaTime)
{
	// Update all active pursuits
	for (auto& Pair : ActivePursuits)
	{
		FMGPursuitStatus& Status = Pair.Value;

		if (Status.State == EMGPursuitState::Inactive ||
		    Status.State == EMGPursuitState::Escaped ||
		    Status.State == EMGPursuitState::Busted)
		{
			continue;
		}

		// Update cooldown
		if (Status.State == EMGPursuitState::Cooldown)
		{
			Status.CooldownRemaining -= DeltaTime;
			if (Status.CooldownRemaining <= 0.0f)
			{
				EndPursuit(Pair.Key, true);
			}
		}
	}
}

void UMGPursuitSubsystem::UpdateEscapeProgress(const FString& PlayerId, FVector PlayerLocation, float DeltaTime)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	// Check if any unit has visual
	bool bAnyUnitHasVisual = false;
	for (const FMGPursuitUnit& Unit : Status->ActiveUnits)
	{
		if (!Unit.bIsDisabled && Unit.bHasVisual)
		{
			bAnyUnitHasVisual = true;
			break;
		}
	}

	float OldEscapeMeter = Status->EscapeMeter;

	if (!bAnyUnitHasVisual)
	{
		// Fill escape meter when out of sight
		Status->EscapeMeter += PursuitConfig.EscapeFillRate * DeltaTime;

		if (Status->EscapeMeter >= 100.0f)
		{
			// Start cooldown
			Status->EscapeMeter = 100.0f;
			Status->CooldownRemaining = PursuitConfig.CooldownDuration;
			SetPursuitState(PlayerId, EMGPursuitState::Cooldown);

			RecordEvent(PlayerId, EMGPursuitEventType::CooldownStarted, TEXT("Cooldown started"), PlayerLocation);
			OnCooldownStarted.Broadcast(PlayerId, PursuitConfig.CooldownDuration);
		}
	}
	else
	{
		// Drain escape meter when in sight
		Status->EscapeMeter -= PursuitConfig.EscapeDrainRate * DeltaTime;
		Status->EscapeMeter = FMath::Max(0.0f, Status->EscapeMeter);
	}

	if (FMath::Abs(Status->EscapeMeter - OldEscapeMeter) > 0.1f)
	{
		OnEscapeMeterChanged.Broadcast(PlayerId, Status->EscapeMeter);
	}
}

void UMGPursuitSubsystem::UpdateBustedProgress(const FString& PlayerId, float DeltaTime)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	// Check if player is stopped and units are close
	bool bShouldFillBusted = false;
	for (const FMGPursuitUnit& Unit : Status->ActiveUnits)
	{
		if (!Unit.bIsDisabled && Unit.DistanceToTarget < 500.0f) // Very close
		{
			bShouldFillBusted = true;
			break;
		}
	}

	float OldBustedMeter = Status->BustedMeter;

	if (bShouldFillBusted)
	{
		Status->BustedMeter += PursuitConfig.BustedFillRate * DeltaTime;

		if (Status->BustedMeter >= PursuitConfig.BustedThreshold)
		{
			// Busted!
			Status->BustedMeter = PursuitConfig.BustedThreshold;
			EndPursuit(PlayerId, false);
			return;
		}
	}
	else
	{
		// Slowly drain busted meter when not boxed in
		Status->BustedMeter -= (PursuitConfig.BustedFillRate * 0.5f) * DeltaTime;
		Status->BustedMeter = FMath::Max(0.0f, Status->BustedMeter);
	}

	if (FMath::Abs(Status->BustedMeter - OldBustedMeter) > 0.1f)
	{
		OnBustedMeterChanged.Broadcast(PlayerId, Status->BustedMeter);
	}
}

void UMGPursuitSubsystem::CheckIntensityUpgrade(const FString& PlayerId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	// Check if duration exceeds threshold for next intensity
	float* Threshold = PursuitConfig.IntensityUpgradeThresholds.Find(Status->Intensity);
	if (Threshold && Status->PursuitDuration >= *Threshold)
	{
		IncreaseIntensity(PlayerId);
	}
}

void UMGPursuitSubsystem::SpawnBackup(const FString& PlayerId)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status)
	{
		return;
	}

	int32* MaxUnits = PursuitConfig.MaxUnitsPerIntensity.Find(Status->Intensity);
	if (!MaxUnits)
	{
		return;
	}

	int32 CurrentActiveUnits = GetActiveUnitCount(PlayerId);
	int32 UnitsToSpawn = *MaxUnits - CurrentActiveUnits;

	for (int32 i = 0; i < UnitsToSpawn; i++)
	{
		EMGPursuitRole Role = EMGPursuitRole::Pursuer;

		// Add interceptors at higher intensities
		if (Status->Intensity >= EMGPursuitIntensity::High && i % 3 == 0)
		{
			Role = EMGPursuitRole::Interceptor;
		}

		// Spawn at offset from player (would need player location in real implementation)
		SpawnPursuitUnit(PlayerId, Role, FVector::ZeroVector);
	}

	RecordEvent(PlayerId, EMGPursuitEventType::BackupCalled,
	            FString::Printf(TEXT("%d backup units called"), UnitsToSpawn),
	            FVector::ZeroVector);
}

void UMGPursuitSubsystem::SetPursuitState(const FString& PlayerId, EMGPursuitState NewState)
{
	FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (!Status || Status->State == NewState)
	{
		return;
	}

	Status->State = NewState;
	OnPursuitStateChanged.Broadcast(PlayerId, NewState);
}

void UMGPursuitSubsystem::RecordEvent(const FString& PlayerId, EMGPursuitEventType Type, const FString& Description, FVector Location)
{
	TArray<FMGPursuitEvent>& Events = PursuitEvents.FindOrAdd(PlayerId);

	FMGPursuitEvent Event;
	Event.Type = Type;
	Event.Description = Description;
	Event.Location = Location;

	const FMGPursuitStatus* Status = ActivePursuits.Find(PlayerId);
	if (Status)
	{
		Event.Timestamp = Status->PursuitDuration;
	}

	Events.Add(Event);

	OnPursuitEvent.Broadcast(PlayerId, Event);
}

FString UMGPursuitSubsystem::GenerateUnitId() const
{
	return FString::Printf(TEXT("UNIT_%d_%lld"), ++const_cast<UMGPursuitSubsystem*>(this)->UnitCounter,
	                       FDateTime::Now().GetTicks());
}

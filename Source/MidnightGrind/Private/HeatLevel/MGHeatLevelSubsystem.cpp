// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGHeatLevelSubsystem.cpp
 * @brief Implementation of the heat/wanted level management system.
 *
 * @see MGHeatLevelSubsystem.h for full documentation
 */

#include "HeatLevel/MGHeatLevelSubsystem.h"
#include "Save/MG_SAVE_ManagerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UMGHeatLevelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultConfigs();

	// Initialize pursuit status
	PursuitStatus.MaxHeat = 1000;

	// Initialize bounty config
	BountyConfig.BaseBountyPerSecond = 10.0f;
	BountyConfig.HeatLevelMultiplier = 1.5f;
	BountyConfig.UnitDisabledBonus = 500;
	BountyConfig.RoadblockBonus = 250;
	BountyConfig.HelicopterEvadeBonus = 1000;
	BountyConfig.EvadeMultiplier = 1.0f;
	BountyConfig.BustedPenaltyPercent = 0.5f;

	LoadHeatData();
}

void UMGHeatLevelSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PursuitTickTimer);
	}

	SaveHeatData();
	Super::Deinitialize();
}

void UMGHeatLevelSubsystem::InitializeDefaultConfigs()
{
	// Heat source configs
	{
		FMGHeatSourceConfig Speeding;
		Speeding.Source = EMGHeatSource::Speeding;
		Speeding.BaseHeatGain = 5;
		Speeding.BaseCostPenalty = 50;
		Speeding.CooldownTime = 3.0f;
		Speeding.bRequiresWitness = true;
		Speeding.bStackable = true;
		Speeding.StackMultiplier = 1.2f;
		Speeding.MaxStacks = 10;
		HeatSourceConfigs.Add(EMGHeatSource::Speeding, Speeding);
	}
	{
		FMGHeatSourceConfig Reckless;
		Reckless.Source = EMGHeatSource::Reckless;
		Reckless.BaseHeatGain = 15;
		Reckless.BaseCostPenalty = 150;
		Reckless.CooldownTime = 5.0f;
		Reckless.bRequiresWitness = true;
		Reckless.bStackable = true;
		Reckless.StackMultiplier = 1.5f;
		Reckless.MaxStacks = 5;
		HeatSourceConfigs.Add(EMGHeatSource::Reckless, Reckless);
	}
	{
		FMGHeatSourceConfig PropertyDamage;
		PropertyDamage.Source = EMGHeatSource::PropertyDamage;
		PropertyDamage.BaseHeatGain = 25;
		PropertyDamage.BaseCostPenalty = 500;
		PropertyDamage.CooldownTime = 2.0f;
		PropertyDamage.bRequiresWitness = false;
		PropertyDamage.bStackable = true;
		PropertyDamage.StackMultiplier = 1.25f;
		PropertyDamage.MaxStacks = 20;
		HeatSourceConfigs.Add(EMGHeatSource::PropertyDamage, PropertyDamage);
	}
	{
		FMGHeatSourceConfig Collision;
		Collision.Source = EMGHeatSource::Collision;
		Collision.BaseHeatGain = 50;
		Collision.BaseCostPenalty = 1000;
		Collision.CooldownTime = 1.0f;
		Collision.bRequiresWitness = false;
		Collision.bStackable = true;
		Collision.StackMultiplier = 2.0f;
		Collision.MaxStacks = 10;
		HeatSourceConfigs.Add(EMGHeatSource::Collision, Collision);
	}
	{
		FMGHeatSourceConfig Evading;
		Evading.Source = EMGHeatSource::Evading;
		Evading.BaseHeatGain = 10;
		Evading.BaseCostPenalty = 100;
		Evading.CooldownTime = 10.0f;
		Evading.bRequiresWitness = false;
		Evading.bStackable = false;
		HeatSourceConfigs.Add(EMGHeatSource::Evading, Evading);
	}
	{
		FMGHeatSourceConfig RoadBlock;
		RoadBlock.Source = EMGHeatSource::RoadBlock;
		RoadBlock.BaseHeatGain = 75;
		RoadBlock.BaseCostPenalty = 2500;
		RoadBlock.CooldownTime = 0.0f;
		RoadBlock.bRequiresWitness = false;
		RoadBlock.bStackable = true;
		RoadBlock.StackMultiplier = 1.5f;
		RoadBlock.MaxStacks = 5;
		HeatSourceConfigs.Add(EMGHeatSource::RoadBlock, RoadBlock);
	}

	// Heat level configs
	{
		FMGHeatLevelConfig Level1;
		Level1.Level = EMGHeatLevel::Level1;
		Level1.HeatThreshold = 50;
		Level1.MaxUnits = 2;
		Level1.AvailableUnits = { EMGPoliceUnitType::Patrol };
		Level1.AggressionMultiplier = 1.0f;
		Level1.SpawnRate = 20.0f;
		Level1.CooldownTime = 20.0f;
		Level1.bRoadblocksEnabled = false;
		Level1.bSpikeStripsEnabled = false;
		Level1.bHelicopterEnabled = false;
		Level1.BustTimeMultiplier = 1.0f;
		Level1.HeatColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
		HeatLevelConfigs.Add(EMGHeatLevel::Level1, Level1);
	}
	{
		FMGHeatLevelConfig Level2;
		Level2.Level = EMGHeatLevel::Level2;
		Level2.HeatThreshold = 150;
		Level2.MaxUnits = 4;
		Level2.AvailableUnits = { EMGPoliceUnitType::Patrol, EMGPoliceUnitType::SUV };
		Level2.AggressionMultiplier = 1.25f;
		Level2.SpawnRate = 15.0f;
		Level2.CooldownTime = 30.0f;
		Level2.bRoadblocksEnabled = false;
		Level2.bSpikeStripsEnabled = false;
		Level2.bHelicopterEnabled = false;
		Level2.BustTimeMultiplier = 0.9f;
		Level2.HeatColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
		HeatLevelConfigs.Add(EMGHeatLevel::Level2, Level2);
	}
	{
		FMGHeatLevelConfig Level3;
		Level3.Level = EMGHeatLevel::Level3;
		Level3.HeatThreshold = 300;
		Level3.MaxUnits = 6;
		Level3.AvailableUnits = { EMGPoliceUnitType::Patrol, EMGPoliceUnitType::SUV, EMGPoliceUnitType::Interceptor };
		Level3.AggressionMultiplier = 1.5f;
		Level3.SpawnRate = 12.0f;
		Level3.CooldownTime = 45.0f;
		Level3.bRoadblocksEnabled = true;
		Level3.bSpikeStripsEnabled = false;
		Level3.bHelicopterEnabled = false;
		Level3.BustTimeMultiplier = 0.8f;
		Level3.HeatColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
		HeatLevelConfigs.Add(EMGHeatLevel::Level3, Level3);
	}
	{
		FMGHeatLevelConfig Level4;
		Level4.Level = EMGHeatLevel::Level4;
		Level4.HeatThreshold = 500;
		Level4.MaxUnits = 8;
		Level4.AvailableUnits = { EMGPoliceUnitType::SUV, EMGPoliceUnitType::Interceptor, EMGPoliceUnitType::Muscle, EMGPoliceUnitType::Undercover };
		Level4.AggressionMultiplier = 1.75f;
		Level4.SpawnRate = 10.0f;
		Level4.CooldownTime = 60.0f;
		Level4.bRoadblocksEnabled = true;
		Level4.bSpikeStripsEnabled = true;
		Level4.bHelicopterEnabled = false;
		Level4.BustTimeMultiplier = 0.7f;
		Level4.HeatColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
		HeatLevelConfigs.Add(EMGHeatLevel::Level4, Level4);
	}
	{
		FMGHeatLevelConfig Level5;
		Level5.Level = EMGHeatLevel::Level5;
		Level5.HeatThreshold = 750;
		Level5.MaxUnits = 10;
		Level5.AvailableUnits = { EMGPoliceUnitType::Interceptor, EMGPoliceUnitType::Muscle, EMGPoliceUnitType::Supercar, EMGPoliceUnitType::SWAT };
		Level5.AggressionMultiplier = 2.0f;
		Level5.SpawnRate = 8.0f;
		Level5.CooldownTime = 90.0f;
		Level5.bRoadblocksEnabled = true;
		Level5.bSpikeStripsEnabled = true;
		Level5.bHelicopterEnabled = true;
		Level5.BustTimeMultiplier = 0.5f;
		Level5.HeatColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
		HeatLevelConfigs.Add(EMGHeatLevel::Level5, Level5);
	}
	{
		FMGHeatLevelConfig MaxHeat;
		MaxHeat.Level = EMGHeatLevel::MaxHeat;
		MaxHeat.HeatThreshold = 900;
		MaxHeat.MaxUnits = 12;
		MaxHeat.AvailableUnits = { EMGPoliceUnitType::Supercar, EMGPoliceUnitType::SWAT, EMGPoliceUnitType::Rhino };
		MaxHeat.AggressionMultiplier = 2.5f;
		MaxHeat.SpawnRate = 5.0f;
		MaxHeat.CooldownTime = 120.0f;
		MaxHeat.bRoadblocksEnabled = true;
		MaxHeat.bSpikeStripsEnabled = true;
		MaxHeat.bHelicopterEnabled = true;
		MaxHeat.BustTimeMultiplier = 0.25f;
		MaxHeat.HeatColor = FLinearColor(0.5f, 0.0f, 0.5f, 1.0f);
		HeatLevelConfigs.Add(EMGHeatLevel::MaxHeat, MaxHeat);
	}
}

void UMGHeatLevelSubsystem::AddHeat(EMGHeatSource Source, FVector Location, bool bWasWitnessed, const FString& WitnessId)
{
	if (!bSessionActive)
	{
		return;
	}

	FMGHeatSourceConfig Config = GetHeatSourceConfig(Source);

	// Check witness requirement
	if (Config.bRequiresWitness && !bWasWitnessed)
	{
		return;
	}

	// Check cooldown
	if (float* Cooldown = InfractionCooldowns.Find(Source))
	{
		if (*Cooldown > 0.0f && !Config.bStackable)
		{
			return;
		}
	}

	// Calculate heat with stacking
	int32 HeatGain = CalculateInfractionHeat(Source);
	int32 CostPenalty = Config.BaseCostPenalty;

	// Update stacks
	if (Config.bStackable)
	{
		int32& Stacks = InfractionStacks.FindOrAdd(Source);
		Stacks = FMath::Min(Stacks + 1, Config.MaxStacks);
	}

	// Set cooldown
	InfractionCooldowns.Add(Source, Config.CooldownTime);

	// Create infraction record
	FMGHeatInfraction Infraction;
	Infraction.InfractionId = FGuid::NewGuid().ToString();
	Infraction.Source = Source;
	Infraction.HeatGained = HeatGain;
	Infraction.CostPenalty = CostPenalty;
	Infraction.Location = Location;
	Infraction.Timestamp = FDateTime::Now();
	Infraction.bWasWitnessed = bWasWitnessed;
	Infraction.WitnessUnitId = WitnessId;

	// Apply heat
	PursuitStatus.CurrentHeat = FMath::Min(PursuitStatus.CurrentHeat + HeatGain, PursuitStatus.MaxHeat);
	PursuitStatus.TotalInfractions++;
	PursuitStatus.AccumulatedCost += CostPenalty;

	// Update session stats
	SessionStats.TotalInfractions++;
	SessionStats.TotalCostAccumulated += CostPenalty;
	int32& TypeCount = SessionStats.InfractionsByType.FindOrAdd(Source);
	TypeCount++;

	// Update heat level
	UpdateHeatLevel();

	OnInfractionCommitted.Broadcast(Infraction, PursuitStatus.CurrentHeat);

	// Start pursuit if not already
	if (PursuitStatus.State == EMGPursuitState::None && PursuitStatus.CurrentHeatLevel != EMGHeatLevel::None)
	{
		StartPursuit();
	}
}

void UMGHeatLevelSubsystem::RemoveHeat(int32 Amount)
{
	PursuitStatus.CurrentHeat = FMath::Max(0, PursuitStatus.CurrentHeat - Amount);
	UpdateHeatLevel();
}

void UMGHeatLevelSubsystem::ClearAllHeat()
{
	PursuitStatus.CurrentHeat = 0;
	PursuitStatus.CurrentHeatLevel = EMGHeatLevel::None;
	PursuitStatus.State = EMGPursuitState::None;
	InfractionStacks.Empty();
	CurrentBounty = 0;
}

int32 UMGHeatLevelSubsystem::GetCurrentHeat() const
{
	return PursuitStatus.CurrentHeat;
}

EMGHeatLevel UMGHeatLevelSubsystem::GetCurrentHeatLevel() const
{
	return PursuitStatus.CurrentHeatLevel;
}

float UMGHeatLevelSubsystem::GetHeatPercent() const
{
	return PursuitStatus.MaxHeat > 0 ? (static_cast<float>(PursuitStatus.CurrentHeat) / static_cast<float>(PursuitStatus.MaxHeat)) * 100.0f : 0.0f;
}

float UMGHeatLevelSubsystem::GetHeatLevelProgress() const
{
	// Progress within current heat level
	FMGHeatLevelConfig CurrentConfig = GetHeatLevelConfig(PursuitStatus.CurrentHeatLevel);

	EMGHeatLevel NextLevel = static_cast<EMGHeatLevel>(static_cast<int32>(PursuitStatus.CurrentHeatLevel) + 1);
	if (NextLevel > EMGHeatLevel::MaxHeat)
	{
		return 100.0f;
	}

	FMGHeatLevelConfig NextConfig = GetHeatLevelConfig(NextLevel);

	int32 LevelRange = NextConfig.HeatThreshold - CurrentConfig.HeatThreshold;
	int32 CurrentProgress = PursuitStatus.CurrentHeat - CurrentConfig.HeatThreshold;

	return LevelRange > 0 ? (static_cast<float>(CurrentProgress) / static_cast<float>(LevelRange)) * 100.0f : 0.0f;
}

FMGPursuitStatus UMGHeatLevelSubsystem::GetPursuitStatus() const
{
	return PursuitStatus;
}

EMGPursuitState UMGHeatLevelSubsystem::GetPursuitState() const
{
	return PursuitStatus.State;
}

bool UMGHeatLevelSubsystem::IsInPursuit() const
{
	return PursuitStatus.State == EMGPursuitState::Pursuit || PursuitStatus.State == EMGPursuitState::Spotted;
}

bool UMGHeatLevelSubsystem::IsEvading() const
{
	return PursuitStatus.State == EMGPursuitState::Escaping || PursuitStatus.State == EMGPursuitState::Cooldown;
}

float UMGHeatLevelSubsystem::GetPursuitDuration() const
{
	return PursuitStatus.PursuitDuration;
}

void UMGHeatLevelSubsystem::StartPursuit()
{
	if (PursuitStatus.State == EMGPursuitState::Pursuit)
	{
		return;
	}

	EMGPursuitState OldState = PursuitStatus.State;
	PursuitStatus.State = EMGPursuitState::Pursuit;
	PursuitStatus.PursuitDuration = 0.0f;
	PursuitStatus.BustProgress = 0.0f;

	SessionStats.TotalPursuits++;

	// Start pursuit tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PursuitTickTimer, FTimerDelegate::CreateUObject(this, &UMGHeatLevelSubsystem::TickPursuit, 0.1f), 0.1f, true);
	}

	OnPursuitStateChanged.Broadcast(OldState, EMGPursuitState::Pursuit);
}

void UMGHeatLevelSubsystem::StartEscaping()
{
	if (PursuitStatus.State != EMGPursuitState::Pursuit)
	{
		return;
	}

	if (!AnyUnitHasVisual())
	{
		EMGPursuitState OldState = PursuitStatus.State;
		PursuitStatus.State = EMGPursuitState::Escaping;
		OnPursuitStateChanged.Broadcast(OldState, EMGPursuitState::Escaping);

		// Start cooldown after a delay
		StartCooldown();
	}
}

void UMGHeatLevelSubsystem::UpdateBustProgress(float DeltaProgress)
{
	if (PursuitStatus.State != EMGPursuitState::Pursuit)
	{
		return;
	}

	FMGHeatLevelConfig Config = GetHeatLevelConfig(PursuitStatus.CurrentHeatLevel);
	float AdjustedProgress = DeltaProgress / Config.BustTimeMultiplier;

	PursuitStatus.BustProgress = FMath::Clamp(PursuitStatus.BustProgress + AdjustedProgress, 0.0f, 100.0f);
	OnBustProgressUpdate.Broadcast(PursuitStatus.BustProgress);

	if (PursuitStatus.BustProgress >= 100.0f)
	{
		TriggerBust();
	}
}

void UMGHeatLevelSubsystem::RegisterPoliceUnit(const FMGActivePoliceUnit& Unit)
{
	ActiveUnits.Add(Unit.UnitId, Unit);
	PursuitStatus.ActiveUnits = ActiveUnits.Num();
	OnPoliceUnitSpawned.Broadcast(Unit.UnitId, Unit.UnitType);
}

void UMGHeatLevelSubsystem::UpdatePoliceUnit(const FString& UnitId, FVector Location, bool bHasVisual, bool bIsInPursuit)
{
	if (FMGActivePoliceUnit* Unit = ActiveUnits.Find(UnitId))
	{
		Unit->Location = Location;
		Unit->bHasVisual = bHasVisual;
		Unit->bIsInPursuit = bIsInPursuit;

		if (bIsInPursuit)
		{
			Unit->TimeInPursuit += 0.1f;
		}
	}
}

void UMGHeatLevelSubsystem::DisablePoliceUnit(const FString& UnitId)
{
	if (FMGActivePoliceUnit* Unit = ActiveUnits.Find(UnitId))
	{
		if (!Unit->bIsDisabled)
		{
			Unit->bIsDisabled = true;
			PursuitStatus.UnitsDisabled++;
			SessionStats.TotalUnitsDisabled++;

			CurrentBounty += BountyConfig.UnitDisabledBonus;
			OnPoliceUnitDisabled.Broadcast(UnitId, BountyConfig.UnitDisabledBonus);
		}
	}
}

void UMGHeatLevelSubsystem::RemovePoliceUnit(const FString& UnitId)
{
	ActiveUnits.Remove(UnitId);
	PursuitStatus.ActiveUnits = ActiveUnits.Num();
}

TArray<FMGActivePoliceUnit> UMGHeatLevelSubsystem::GetActiveUnits() const
{
	TArray<FMGActivePoliceUnit> Result;
	for (const auto& UnitPair : ActiveUnits)
	{
		if (!UnitPair.Value.bIsDisabled)
		{
			Result.Add(UnitPair.Value);
		}
	}
	return Result;
}

int32 UMGHeatLevelSubsystem::GetActiveUnitCount() const
{
	int32 Count = 0;
	for (const auto& UnitPair : ActiveUnits)
	{
		if (!UnitPair.Value.bIsDisabled)
		{
			Count++;
		}
	}
	return Count;
}

FMGActivePoliceUnit UMGHeatLevelSubsystem::GetNearestUnit() const
{
	FMGActivePoliceUnit Nearest;
	float NearestDist = FLT_MAX;

	for (const auto& UnitPair : ActiveUnits)
	{
		if (!UnitPair.Value.bIsDisabled && UnitPair.Value.DistanceToPlayer < NearestDist)
		{
			NearestDist = UnitPair.Value.DistanceToPlayer;
			Nearest = UnitPair.Value;
		}
	}

	return Nearest;
}

bool UMGHeatLevelSubsystem::AnyUnitHasVisual() const
{
	for (const auto& UnitPair : ActiveUnits)
	{
		if (!UnitPair.Value.bIsDisabled && UnitPair.Value.bHasVisual)
		{
			return true;
		}
	}
	return false;
}

void UMGHeatLevelSubsystem::StartCooldown()
{
	if (PursuitStatus.State == EMGPursuitState::Cooldown)
	{
		return;
	}

	EMGPursuitState OldState = PursuitStatus.State;
	PursuitStatus.State = EMGPursuitState::Cooldown;

	FMGHeatLevelConfig Config = GetHeatLevelConfig(PursuitStatus.CurrentHeatLevel);
	PursuitStatus.CooldownRemaining = Config.CooldownTime;
	CooldownTotal = Config.CooldownTime;

	OnPursuitStateChanged.Broadcast(OldState, EMGPursuitState::Cooldown);
	OnCooldownStarted.Broadcast(PursuitStatus.CooldownRemaining);
}

void UMGHeatLevelSubsystem::EnterCooldownSpot(const FString& SpotId)
{
	if (const FMGCooldownSpot* Spot = CooldownSpots.Find(SpotId))
	{
		if (Spot->bIsUnlocked && static_cast<int32>(PursuitStatus.CurrentHeatLevel) <= static_cast<int32>(Spot->MaxEffectiveHeat))
		{
			CurrentCooldownSpotId = SpotId;
		}
	}
}

void UMGHeatLevelSubsystem::ExitCooldownSpot()
{
	CurrentCooldownSpotId.Empty();
}

float UMGHeatLevelSubsystem::GetCooldownRemaining() const
{
	return PursuitStatus.CooldownRemaining;
}

float UMGHeatLevelSubsystem::GetCooldownProgress() const
{
	return CooldownTotal > 0.0f ? ((CooldownTotal - PursuitStatus.CooldownRemaining) / CooldownTotal) * 100.0f : 0.0f;
}

bool UMGHeatLevelSubsystem::IsInCooldownSpot() const
{
	return !CurrentCooldownSpotId.IsEmpty();
}

void UMGHeatLevelSubsystem::RegisterCooldownSpot(const FMGCooldownSpot& Spot)
{
	if (!Spot.SpotId.IsEmpty())
	{
		CooldownSpots.Add(Spot.SpotId, Spot);
	}
}

FMGCooldownSpot UMGHeatLevelSubsystem::GetCooldownSpot(const FString& SpotId) const
{
	if (const FMGCooldownSpot* Spot = CooldownSpots.Find(SpotId))
	{
		return *Spot;
	}
	return FMGCooldownSpot();
}

TArray<FMGCooldownSpot> UMGHeatLevelSubsystem::GetAllCooldownSpots() const
{
	TArray<FMGCooldownSpot> Result;
	CooldownSpots.GenerateValueArray(Result);
	return Result;
}

FMGCooldownSpot UMGHeatLevelSubsystem::GetNearestCooldownSpot(FVector Location) const
{
	FMGCooldownSpot Nearest;
	float NearestDist = FLT_MAX;

	for (const auto& SpotPair : CooldownSpots)
	{
		if (SpotPair.Value.bIsUnlocked)
		{
			float Dist = FVector::Dist(Location, SpotPair.Value.Location);
			if (Dist < NearestDist)
			{
				NearestDist = Dist;
				Nearest = SpotPair.Value;
			}
		}
	}

	return Nearest;
}

void UMGHeatLevelSubsystem::SetHeatSourceConfig(EMGHeatSource Source, const FMGHeatSourceConfig& Config)
{
	HeatSourceConfigs.Add(Source, Config);
}

FMGHeatSourceConfig UMGHeatLevelSubsystem::GetHeatSourceConfig(EMGHeatSource Source) const
{
	if (const FMGHeatSourceConfig* Config = HeatSourceConfigs.Find(Source))
	{
		return *Config;
	}
	return FMGHeatSourceConfig();
}

void UMGHeatLevelSubsystem::SetHeatLevelConfig(EMGHeatLevel Level, const FMGHeatLevelConfig& Config)
{
	HeatLevelConfigs.Add(Level, Config);
}

FMGHeatLevelConfig UMGHeatLevelSubsystem::GetHeatLevelConfig(EMGHeatLevel Level) const
{
	if (const FMGHeatLevelConfig* Config = HeatLevelConfigs.Find(Level))
	{
		return *Config;
	}
	return FMGHeatLevelConfig();
}

void UMGHeatLevelSubsystem::SetBountyConfig(const FMGBountyConfig& Config)
{
	BountyConfig = Config;
}

FMGBountyConfig UMGHeatLevelSubsystem::GetBountyConfig() const
{
	return BountyConfig;
}

int32 UMGHeatLevelSubsystem::GetCurrentBounty() const
{
	return CurrentBounty;
}

int32 UMGHeatLevelSubsystem::CalculateEvadeBounty() const
{
	return FMath::RoundToInt(CurrentBounty * BountyConfig.EvadeMultiplier);
}

void UMGHeatLevelSubsystem::NotifyRoadblockEvaded()
{
	PursuitStatus.RoadblocksEvaded++;
	SessionStats.TotalRoadblocksEvaded++;
	CurrentBounty += BountyConfig.RoadblockBonus;
}

void UMGHeatLevelSubsystem::NotifyHelicopterEvaded()
{
	CurrentBounty += BountyConfig.HelicopterEvadeBonus;
}

void UMGHeatLevelSubsystem::DeployHelicopter()
{
	FMGHeatLevelConfig Config = GetHeatLevelConfig(PursuitStatus.CurrentHeatLevel);
	if (Config.bHelicopterEnabled && !PursuitStatus.bHelicopterActive)
	{
		PursuitStatus.bHelicopterActive = true;
		OnHelicopterDeployed.Broadcast();
	}
}

void UMGHeatLevelSubsystem::SpawnRoadblock(FVector Location)
{
	OnRoadblockSpawned.Broadcast(Location);
}

void UMGHeatLevelSubsystem::SpawnSpikeStrip(FVector Location)
{
	// Handled by gameplay
}

void UMGHeatLevelSubsystem::TriggerBust()
{
	EMGPursuitState OldState = PursuitStatus.State;
	PursuitStatus.State = EMGPursuitState::Busted;

	SessionStats.TimesBusted++;

	int32 BustCost = GetBustCost();
	int32 LostBounty = FMath::RoundToInt(CurrentBounty * BountyConfig.BustedPenaltyPercent);

	OnPlayerBusted.Broadcast(BustCost, PursuitStatus.PursuitDuration);
	OnPursuitStateChanged.Broadcast(OldState, EMGPursuitState::Busted);

	// Stop pursuit timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PursuitTickTimer);
	}

	ClearAllHeat();
	ActiveUnits.Empty();
}

float UMGHeatLevelSubsystem::GetBustProgress() const
{
	return PursuitStatus.BustProgress;
}

int32 UMGHeatLevelSubsystem::GetBustCost() const
{
	return PursuitStatus.AccumulatedCost;
}

void UMGHeatLevelSubsystem::StartSession()
{
	bSessionActive = true;
	SessionStats = FMGHeatSessionStats();
	ClearAllHeat();
	ActiveUnits.Empty();
}

void UMGHeatLevelSubsystem::EndSession()
{
	bSessionActive = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PursuitTickTimer);
	}

	SaveHeatData();
}

bool UMGHeatLevelSubsystem::IsSessionActive() const
{
	return bSessionActive;
}

FMGHeatSessionStats UMGHeatLevelSubsystem::GetSessionStats() const
{
	return SessionStats;
}

FText UMGHeatLevelSubsystem::GetHeatLevelDisplayName(EMGHeatLevel Level) const
{
	switch (Level)
	{
		case EMGHeatLevel::None: return FText::FromString(TEXT("No Heat"));
		case EMGHeatLevel::Level1: return FText::FromString(TEXT("Heat Level 1"));
		case EMGHeatLevel::Level2: return FText::FromString(TEXT("Heat Level 2"));
		case EMGHeatLevel::Level3: return FText::FromString(TEXT("Heat Level 3"));
		case EMGHeatLevel::Level4: return FText::FromString(TEXT("Heat Level 4"));
		case EMGHeatLevel::Level5: return FText::FromString(TEXT("Heat Level 5"));
		case EMGHeatLevel::MaxHeat: return FText::FromString(TEXT("MAXIMUM HEAT"));
		default: return FText::FromString(TEXT("Unknown"));
	}
}

FLinearColor UMGHeatLevelSubsystem::GetHeatLevelColor(EMGHeatLevel Level) const
{
	FMGHeatLevelConfig Config = GetHeatLevelConfig(Level);
	return Config.HeatColor;
}

int32 UMGHeatLevelSubsystem::GetHeatLevelStars(EMGHeatLevel Level) const
{
	return static_cast<int32>(Level);
}

void UMGHeatLevelSubsystem::SaveHeatData()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UMGSaveManagerSubsystem* SaveManager = GameInstance->GetSubsystem<UMGSaveManagerSubsystem>();
	if (!SaveManager)
	{
		return;
	}

	UMGSaveGame* SaveGame = SaveManager->GetSaveDataMutable();
	if (!SaveGame)
	{
		return;
	}

	// Save lifetime stats
	SaveGame->HeatLevelData.TotalPursuitsEscaped = LifetimeStats.TotalPursuitsEvaded;
	SaveGame->HeatLevelData.TotalPursuitsBusted = LifetimeStats.TotalPursuitsBusted;
	SaveGame->HeatLevelData.MaxHeatLevelReached = static_cast<int32>(LifetimeStats.HighestHeatLevel);
	SaveGame->HeatLevelData.LongestPursuitDuration = LifetimeStats.LongestPursuitTime;
	SaveGame->HeatLevelData.TotalFinesPaid = LifetimeStats.TotalCostPaid;
	SaveGame->HeatLevelData.TotalBountyEarned = LifetimeStats.TotalBountyEarned;
	SaveGame->HeatLevelData.CopsDisabledTotal = LifetimeStats.TotalUnitsDisabled;
	SaveGame->HeatLevelData.RoadblocksEvadedTotal = LifetimeStats.RoadblocksEvaded;
	SaveGame->HeatLevelData.SpikeStripsEvadedTotal = LifetimeStats.SpikeStripsEvaded;
}

void UMGHeatLevelSubsystem::LoadHeatData()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UMGSaveManagerSubsystem* SaveManager = GameInstance->GetSubsystem<UMGSaveManagerSubsystem>();
	if (!SaveManager)
	{
		return;
	}

	const UMGSaveGame* SaveGame = SaveManager->GetCurrentSaveData();
	if (!SaveGame)
	{
		return;
	}

	// Load lifetime stats
	LifetimeStats.TotalPursuitsEvaded = SaveGame->HeatLevelData.TotalPursuitsEscaped;
	LifetimeStats.TotalPursuitsBusted = SaveGame->HeatLevelData.TotalPursuitsBusted;
	LifetimeStats.HighestHeatLevel = static_cast<EMGHeatLevel>(SaveGame->HeatLevelData.MaxHeatLevelReached);
	LifetimeStats.LongestPursuitTime = SaveGame->HeatLevelData.LongestPursuitDuration;
	LifetimeStats.TotalCostPaid = SaveGame->HeatLevelData.TotalFinesPaid;
	LifetimeStats.TotalBountyEarned = SaveGame->HeatLevelData.TotalBountyEarned;
	LifetimeStats.TotalUnitsDisabled = SaveGame->HeatLevelData.CopsDisabledTotal;
	LifetimeStats.RoadblocksEvaded = SaveGame->HeatLevelData.RoadblocksEvadedTotal;
	LifetimeStats.SpikeStripsEvaded = SaveGame->HeatLevelData.SpikeStripsEvadedTotal;
}

void UMGHeatLevelSubsystem::UpdateHeatLevel()
{
	EMGHeatLevel OldLevel = PursuitStatus.CurrentHeatLevel;
	EMGHeatLevel NewLevel = EMGHeatLevel::None;

	// Find appropriate level
	for (int32 i = static_cast<int32>(EMGHeatLevel::MaxHeat); i >= static_cast<int32>(EMGHeatLevel::Level1); --i)
	{
		EMGHeatLevel Level = static_cast<EMGHeatLevel>(i);
		FMGHeatLevelConfig Config = GetHeatLevelConfig(Level);
		if (PursuitStatus.CurrentHeat >= Config.HeatThreshold)
		{
			NewLevel = Level;
			break;
		}
	}

	PursuitStatus.CurrentHeatLevel = NewLevel;

	if (OldLevel != NewLevel)
	{
		int32 NewLevelInt = static_cast<int32>(NewLevel);
		if (NewLevelInt > SessionStats.HighestHeatLevel)
		{
			SessionStats.HighestHeatLevel = NewLevelInt;
		}

		OnHeatLevelChanged.Broadcast(OldLevel, NewLevel);
	}
}

void UMGHeatLevelSubsystem::TickPursuit(float MGDeltaTime)
{
	if (PursuitStatus.State == EMGPursuitState::Pursuit || PursuitStatus.State == EMGPursuitState::Escaping)
	{
		PursuitStatus.PursuitDuration += DeltaTime;

		if (PursuitStatus.PursuitDuration > SessionStats.LongestPursuit)
		{
			SessionStats.LongestPursuit = PursuitStatus.PursuitDuration;
		}

		// Tick bounty
		TickBounty(DeltaTime);

		// Check for escape
		if (PursuitStatus.State == EMGPursuitState::Pursuit && !AnyUnitHasVisual())
		{
			StartEscaping();
		}
		else if (PursuitStatus.State == EMGPursuitState::Escaping && AnyUnitHasVisual())
		{
			// Back to pursuit
			EMGPursuitState OldState = PursuitStatus.State;
			PursuitStatus.State = EMGPursuitState::Pursuit;
			OnPursuitStateChanged.Broadcast(OldState, EMGPursuitState::Pursuit);
		}
	}

	if (PursuitStatus.State == EMGPursuitState::Cooldown)
	{
		TickCooldown(DeltaTime);
	}

	// Decay infraction cooldowns
	TArray<EMGHeatSource> KeysToRemove;
	for (auto& CooldownPair : InfractionCooldowns)
	{
		CooldownPair.Value -= DeltaTime;
		if (CooldownPair.Value <= 0.0f)
		{
			KeysToRemove.Add(CooldownPair.Key);
		}
	}
	for (EMGHeatSource Key : KeysToRemove)
	{
		InfractionCooldowns.Remove(Key);
		InfractionStacks.Remove(Key);
	}
}

void UMGHeatLevelSubsystem::TickCooldown(float MGDeltaTime)
{
	float CooldownRate = 1.0f;

	// Apply cooldown spot multiplier
	if (!CurrentCooldownSpotId.IsEmpty())
	{
		if (const FMGCooldownSpot* Spot = CooldownSpots.Find(CurrentCooldownSpotId))
		{
			CooldownRate = Spot->CooldownMultiplier;
		}
	}

	PursuitStatus.CooldownRemaining -= DeltaTime * CooldownRate;

	if (PursuitStatus.CooldownRemaining <= 0.0f)
	{
		CompleteCooldown();
	}
}

void UMGHeatLevelSubsystem::TickBounty(float MGDeltaTime)
{
	float BountyRate = BountyConfig.BaseBountyPerSecond;

	// Heat level multiplier
	int32 HeatLevelInt = static_cast<int32>(PursuitStatus.CurrentHeatLevel);
	BountyRate *= FMath::Pow(BountyConfig.HeatLevelMultiplier, HeatLevelInt);

	CurrentBounty += FMath::RoundToInt(BountyRate * DeltaTime);
}

void UMGHeatLevelSubsystem::CompleteCooldown()
{
	EMGPursuitState OldState = PursuitStatus.State;
	PursuitStatus.State = EMGPursuitState::Evaded;

	SessionStats.PursuitsEvaded++;

	int32 EarnedBounty = CalculateEvadeBounty();
	SessionStats.TotalBountyEarned += EarnedBounty;

	OnPursuitEvaded.Broadcast(PursuitStatus.PursuitDuration, EarnedBounty);
	OnPursuitStateChanged.Broadcast(OldState, EMGPursuitState::Evaded);
	OnCooldownComplete.Broadcast();

	// Stop pursuit timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PursuitTickTimer);
	}

	// Clear pursuit state
	ClearAllHeat();
	ActiveUnits.Empty();
}

int32 UMGHeatLevelSubsystem::CalculateInfractionHeat(EMGHeatSource Source) const
{
	FMGHeatSourceConfig Config = GetHeatSourceConfig(Source);
	int32 Heat = Config.BaseHeatGain;

	// Apply stack multiplier
	if (Config.bStackable)
	{
		if (const int32* Stacks = InfractionStacks.Find(Source))
		{
			Heat = FMath::RoundToInt(Heat * FMath::Pow(Config.StackMultiplier, *Stacks));
		}
	}

	return Heat;
}

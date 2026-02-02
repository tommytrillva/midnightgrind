// Copyright Midnight Grind. All Rights Reserved.

#include "NitroBoost/MGNitroBoostSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGNitroBoostSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultConfig();

	NitroState.CurrentAmount = NitroConfig.MaxCapacity;
	NitroState.MaxAmount = NitroConfig.MaxCapacity;
	NitroState.State = EMGBoostState::Ready;
	NitroState.BoostLevel = 1;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			NitroTickHandle,
			this,
			&UMGNitroBoostSubsystem::OnNitroTick,
			0.016f,
			true
		);
	}
}

void UMGNitroBoostSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NitroTickHandle);
	}

	Super::Deinitialize();
}

bool UMGNitroBoostSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

bool UMGNitroBoostSubsystem::ActivateBoost()
{
	if (!CanActivateBoost())
	{
		return false;
	}

	// Check for perfect boost
	if (bPerfectBoostWindowActive)
	{
		NitroState.bIsPerfectBoost = true;
		OnPerfectBoostAchieved.Broadcast(PerfectBoostBonusPower);
	}
	else
	{
		NitroState.bIsPerfectBoost = false;
	}

	NitroState.State = EMGBoostState::Active;
	NitroState.ActiveTime = 0.0f;

	OnNitroActivated.Broadcast(NitroState.BoostLevel);

	return true;
}

void UMGNitroBoostSubsystem::DeactivateBoost()
{
	if (NitroState.State != EMGBoostState::Active)
	{
		return;
	}

	NitroState.State = EMGBoostState::Cooldown;
	NitroState.CooldownRemaining = NitroConfig.CooldownTime;
	NitroState.bIsPerfectBoost = false;

	OnNitroDeactivated.Broadcast();
}

bool UMGNitroBoostSubsystem::CanActivateBoost() const
{
	if (NitroState.State == EMGBoostState::Active ||
		NitroState.State == EMGBoostState::Cooldown ||
		NitroState.State == EMGBoostState::Overheated)
	{
		return false;
	}

	if (NitroState.CurrentAmount < NitroConfig.MinActivationAmount)
	{
		return false;
	}

	return true;
}

float UMGNitroBoostSubsystem::GetCurrentBoostMultiplier() const
{
	if (NitroState.State != EMGBoostState::Active)
	{
		return 1.0f;
	}

	return CalculateEffectiveMultiplier();
}

float UMGNitroBoostSubsystem::GetNitroPercent() const
{
	if (NitroState.MaxAmount <= 0.0f)
	{
		return 0.0f;
	}

	return (NitroState.CurrentAmount / NitroState.MaxAmount) * 100.0f;
}

void UMGNitroBoostSubsystem::AddNitroCharge(float Amount, EMGNitroChargeMethod Method)
{
	// Apply charge source multipliers
	for (const FMGNitroChargeSource& Source : ChargeSources)
	{
		if (Source.Method == Method && Source.bEnabled)
		{
			Amount *= Source.ChargeMultiplier;
			break;
		}
	}

	float OldAmount = NitroState.CurrentAmount;
	NitroState.CurrentAmount = FMath::Clamp(NitroState.CurrentAmount + Amount, 0.0f, NitroState.MaxAmount);

	if (!FMath::IsNearlyEqual(OldAmount, NitroState.CurrentAmount))
	{
		OnNitroAmountChanged.Broadcast(NitroState.CurrentAmount, NitroState.MaxAmount);
		OnNitroCharged.Broadcast(Method, Amount);
	}

	UpdateBoostState();
}

void UMGNitroBoostSubsystem::SetNitroAmount(float Amount)
{
	float OldAmount = NitroState.CurrentAmount;
	NitroState.CurrentAmount = FMath::Clamp(Amount, 0.0f, NitroState.MaxAmount);

	if (!FMath::IsNearlyEqual(OldAmount, NitroState.CurrentAmount))
	{
		OnNitroAmountChanged.Broadcast(NitroState.CurrentAmount, NitroState.MaxAmount);
	}

	UpdateBoostState();
}

void UMGNitroBoostSubsystem::RefillNitro()
{
	SetNitroAmount(NitroState.MaxAmount);
}

void UMGNitroBoostSubsystem::DrainNitro(float Amount)
{
	float OldAmount = NitroState.CurrentAmount;
	NitroState.CurrentAmount = FMath::Max(0.0f, NitroState.CurrentAmount - Amount);

	if (!FMath::IsNearlyEqual(OldAmount, NitroState.CurrentAmount))
	{
		OnNitroAmountChanged.Broadcast(NitroState.CurrentAmount, NitroState.MaxAmount);
	}

	if (NitroState.CurrentAmount <= 0.0f)
	{
		if (NitroState.State == EMGBoostState::Active)
		{
			DeactivateBoost();
		}
		OnNitroDepleted.Broadcast();
	}
}

void UMGNitroBoostSubsystem::RegisterChargeSource(const FMGNitroChargeSource& Source)
{
	for (FMGNitroChargeSource& Existing : ChargeSources)
	{
		if (Existing.Method == Source.Method)
		{
			Existing = Source;
			return;
		}
	}

	ChargeSources.Add(Source);
}

void UMGNitroBoostSubsystem::SetNitroConfig(const FMGNitroConfig& Config)
{
	NitroConfig = Config;
	NitroState.MaxAmount = Config.MaxCapacity;

	if (NitroState.CurrentAmount > NitroState.MaxAmount)
	{
		NitroState.CurrentAmount = NitroState.MaxAmount;
	}
}

void UMGNitroBoostSubsystem::SetNitroType(EMGNitroType Type)
{
	NitroConfig.NitroType = Type;

	// Adjust properties based on nitro type
	switch (Type)
	{
	case EMGNitroType::Standard:
		NitroConfig.BoostMultiplier = 1.5f;
		NitroConfig.ConsumptionRate = 25.0f;
		break;

	case EMGNitroType::Supercharged:
		NitroConfig.BoostMultiplier = 1.75f;
		NitroConfig.ConsumptionRate = 35.0f;
		break;

	case EMGNitroType::Cryogenic:
		NitroConfig.BoostMultiplier = 1.6f;
		NitroConfig.ConsumptionRate = 20.0f;
		NitroConfig.bCanOverheat = false;
		break;

	case EMGNitroType::Electric:
		NitroConfig.BoostMultiplier = 1.4f;
		NitroConfig.ConsumptionRate = 15.0f;
		NitroConfig.RechargeRate = 15.0f;
		break;

	case EMGNitroType::Plasma:
		NitroConfig.BoostMultiplier = 2.0f;
		NitroConfig.ConsumptionRate = 50.0f;
		NitroConfig.bCanOverheat = true;
		break;

	case EMGNitroType::Experimental:
		NitroConfig.BoostMultiplier = 2.5f;
		NitroConfig.ConsumptionRate = 60.0f;
		NitroConfig.bCanOverheat = true;
		NitroConfig.OverheatThreshold = 60.0f;
		break;
	}
}

void UMGNitroBoostSubsystem::SetBoostLevel(int32 Level)
{
	NitroState.BoostLevel = FMath::Clamp(Level, 1, MaxBoostLevel);
}

bool UMGNitroBoostSubsystem::CanUpgradeBoostLevel() const
{
	if (NitroState.BoostLevel >= MaxBoostLevel)
	{
		return false;
	}

	// Require full nitro to upgrade level
	return NitroState.CurrentAmount >= NitroState.MaxAmount * 0.9f;
}

void UMGNitroBoostSubsystem::TriggerPerfectBoostWindow()
{
	bPerfectBoostWindowActive = true;
	PerfectBoostWindowTimer = PerfectBoostWindowDuration;
}

float UMGNitroBoostSubsystem::GetPerfectBoostWindowRemaining() const
{
	return bPerfectBoostWindowActive ? PerfectBoostWindowTimer : 0.0f;
}

void UMGNitroBoostSubsystem::ApplyUpgrade(const FMGNitroUpgrade& Upgrade)
{
	InstalledUpgrades.Add(Upgrade);

	// Apply bonuses
	NitroState.MaxAmount += Upgrade.CapacityBonus;
	NitroConfig.MaxCapacity += Upgrade.CapacityBonus;
	NitroConfig.RechargeRate += Upgrade.RechargeBonus;
	NitroConfig.BoostMultiplier += Upgrade.PowerBonus;

	if (Upgrade.EfficiencyBonus > 0.0f)
	{
		NitroConfig.ConsumptionRate *= (1.0f - Upgrade.EfficiencyBonus);
	}
}

void UMGNitroBoostSubsystem::RegisterBoostZone(const FMGBoostZone& Zone)
{
	BoostZones.Add(Zone.ZoneID, Zone);
}

void UMGNitroBoostSubsystem::UnregisterBoostZone(FName ZoneID)
{
	BoostZones.Remove(ZoneID);

	if (ActiveBoostZone.ZoneID == ZoneID)
	{
		ActiveBoostZone = FMGBoostZone();
	}
}

void UMGNitroBoostSubsystem::RegisterPickup(const FMGNitroPickup& Pickup)
{
	Pickups.Add(Pickup.PickupID, Pickup);
}

void UMGNitroBoostSubsystem::CollectPickup(FName PickupID)
{
	if (FMGNitroPickup* Pickup = Pickups.Find(PickupID))
	{
		if (Pickup->bIsAvailable)
		{
			AddNitroCharge(Pickup->ChargeAmount, EMGNitroChargeMethod::Pickup);
			Pickup->bIsAvailable = false;
		}
	}
}

TArray<FMGNitroPickup> UMGNitroBoostSubsystem::GetActivePickups() const
{
	TArray<FMGNitroPickup> ActivePickups;
	for (const auto& Pair : Pickups)
	{
		if (Pair.Value.bIsAvailable)
		{
			ActivePickups.Add(Pair.Value);
		}
	}
	return ActivePickups;
}

void UMGNitroBoostSubsystem::UpdateVehicleLocation(FVector Location)
{
	CurrentVehicleLocation = Location;
}

void UMGNitroBoostSubsystem::OnNitroTick()
{
	float DeltaTime = 0.016f;

	// Update perfect boost window
	if (bPerfectBoostWindowActive)
	{
		PerfectBoostWindowTimer -= DeltaTime;
		if (PerfectBoostWindowTimer <= 0.0f)
		{
			bPerfectBoostWindowActive = false;
		}
	}

	// Update cooldown
	if (NitroState.State == EMGBoostState::Cooldown)
	{
		NitroState.CooldownRemaining -= DeltaTime;
		if (NitroState.CooldownRemaining <= 0.0f)
		{
			NitroState.State = (NitroState.CurrentAmount >= NitroConfig.MinActivationAmount) ?
				EMGBoostState::Ready : EMGBoostState::Charging;
		}
	}

	// Update overheat cooldown
	if (NitroState.State == EMGBoostState::Overheated)
	{
		NitroState.HeatLevel -= DeltaTime * (100.0f / NitroConfig.OverheatCooldownTime);
		if (NitroState.HeatLevel <= 0.0f)
		{
			NitroState.HeatLevel = 0.0f;
			NitroState.State = EMGBoostState::Ready;
		}
	}

	// Active boost consumption
	if (NitroState.State == EMGBoostState::Active)
	{
		NitroState.ActiveTime += DeltaTime;

		// Consume nitro based on boost level
		float Consumption = NitroConfig.ConsumptionRate * NitroState.BoostLevel * DeltaTime;
		DrainNitro(Consumption);

		// Update heat
		if (NitroConfig.bCanOverheat)
		{
			UpdateHeat();
		}

		// Check if we should stop boost
		if (NitroState.CurrentAmount <= 0.0f ||
			(NitroConfig.bCanOverheat && NitroState.HeatLevel >= 100.0f))
		{
			DeactivateBoost();
		}
	}

	// Passive recharging
	UpdateCharging();

	// Check boost zones
	CheckBoostZones();

	// Update pickups
	UpdatePickups();
}

void UMGNitroBoostSubsystem::UpdateBoostState()
{
	if (NitroState.State == EMGBoostState::Active ||
		NitroState.State == EMGBoostState::Cooldown ||
		NitroState.State == EMGBoostState::Overheated)
	{
		return;
	}

	if (NitroState.CurrentAmount >= NitroConfig.MinActivationAmount)
	{
		NitroState.State = EMGBoostState::Ready;
	}
	else
	{
		NitroState.State = EMGBoostState::Charging;
	}
}

void UMGNitroBoostSubsystem::UpdateCharging()
{
	if (NitroState.State == EMGBoostState::Active ||
		NitroState.State == EMGBoostState::Overheated)
	{
		return;
	}

	if (NitroState.CurrentAmount < NitroState.MaxAmount)
	{
		float ChargeAmount = NitroConfig.RechargeRate * 0.016f;
		AddNitroCharge(ChargeAmount, EMGNitroChargeMethod::Time);
	}
}

void UMGNitroBoostSubsystem::UpdateHeat()
{
	if (!NitroConfig.bCanOverheat)
	{
		return;
	}

	if (NitroState.State == EMGBoostState::Active)
	{
		// Heat increases based on boost level
		float HeatIncrease = 10.0f * NitroState.BoostLevel * 0.016f;
		NitroState.HeatLevel = FMath::Min(100.0f, NitroState.HeatLevel + HeatIncrease);

		if (NitroState.HeatLevel >= NitroConfig.OverheatThreshold)
		{
			NitroState.State = EMGBoostState::Overheated;
			OnNitroOverheat.Broadcast();
		}
	}
	else
	{
		// Cool down when not boosting
		float HeatDecrease = 20.0f * 0.016f;
		NitroState.HeatLevel = FMath::Max(0.0f, NitroState.HeatLevel - HeatDecrease);
	}
}

void UMGNitroBoostSubsystem::CheckBoostZones()
{
	FMGBoostZone PreviousZone = ActiveBoostZone;
	ActiveBoostZone = FMGBoostZone();

	for (const auto& Pair : BoostZones)
	{
		const FMGBoostZone& Zone = Pair.Value;
		float Distance = FVector::Dist(CurrentVehicleLocation, Zone.Location);

		if (Distance <= Zone.Radius)
		{
			ActiveBoostZone = Zone;

			if (Zone.bInstantRefill)
			{
				RefillNitro();
			}
			break;
		}
	}
}

void UMGNitroBoostSubsystem::UpdatePickups()
{
	// Handle pickup respawns
	for (auto& Pair : Pickups)
	{
		FMGNitroPickup& Pickup = Pair.Value;
		if (!Pickup.bIsAvailable)
		{
			// Simple respawn timer simulation
			static TMap<FName, float> RespawnTimers;
			float& Timer = RespawnTimers.FindOrAdd(Pickup.PickupID);
			Timer += 0.016f;

			if (Timer >= Pickup.RespawnTime)
			{
				Pickup.bIsAvailable = true;
				Timer = 0.0f;
			}
		}
	}
}

void UMGNitroBoostSubsystem::InitializeDefaultConfig()
{
	NitroConfig.NitroType = EMGNitroType::Standard;
	NitroConfig.MaxCapacity = 100.0f;
	NitroConfig.ConsumptionRate = 25.0f;
	NitroConfig.RechargeRate = 10.0f;
	NitroConfig.BoostMultiplier = 1.5f;
	NitroConfig.MinActivationAmount = 10.0f;
	NitroConfig.CooldownTime = 0.5f;
	NitroConfig.bAllowPartialBoost = true;
	NitroConfig.bCanOverheat = false;
	NitroConfig.OverheatThreshold = 80.0f;
	NitroConfig.OverheatCooldownTime = 3.0f;

	// Register default charge sources
	FMGNitroChargeSource TimeCharge;
	TimeCharge.Method = EMGNitroChargeMethod::Time;
	TimeCharge.ChargeAmount = 1.0f;
	TimeCharge.ChargeMultiplier = 1.0f;
	ChargeSources.Add(TimeCharge);

	FMGNitroChargeSource DriftCharge;
	DriftCharge.Method = EMGNitroChargeMethod::Drifting;
	DriftCharge.ChargeAmount = 5.0f;
	DriftCharge.ChargeMultiplier = 1.0f;
	ChargeSources.Add(DriftCharge);

	FMGNitroChargeSource NearMissCharge;
	NearMissCharge.Method = EMGNitroChargeMethod::NearMiss;
	NearMissCharge.ChargeAmount = 10.0f;
	NearMissCharge.ChargeMultiplier = 1.0f;
	ChargeSources.Add(NearMissCharge);

	FMGNitroChargeSource DraftCharge;
	DraftCharge.Method = EMGNitroChargeMethod::Drafting;
	DraftCharge.ChargeAmount = 2.0f;
	DraftCharge.ChargeMultiplier = 1.0f;
	ChargeSources.Add(DraftCharge);

	// Initialize default upgrades
	FMGNitroUpgrade CapacityUpgrade;
	CapacityUpgrade.UpgradeID = FName(TEXT("Capacity1"));
	CapacityUpgrade.DisplayName = FText::FromString(TEXT("Larger Tank"));
	CapacityUpgrade.CapacityBonus = 25.0f;
	CapacityUpgrade.UnlockLevel = 5;
	CapacityUpgrade.Cost = 5000;
	AvailableUpgrades.Add(CapacityUpgrade);

	FMGNitroUpgrade RechargeUpgrade;
	RechargeUpgrade.UpgradeID = FName(TEXT("Recharge1"));
	RechargeUpgrade.DisplayName = FText::FromString(TEXT("Quick Recharge"));
	RechargeUpgrade.RechargeBonus = 5.0f;
	RechargeUpgrade.UnlockLevel = 10;
	RechargeUpgrade.Cost = 7500;
	AvailableUpgrades.Add(RechargeUpgrade);

	FMGNitroUpgrade PowerUpgrade;
	PowerUpgrade.UpgradeID = FName(TEXT("Power1"));
	PowerUpgrade.DisplayName = FText::FromString(TEXT("Power Boost"));
	PowerUpgrade.PowerBonus = 0.1f;
	PowerUpgrade.UnlockLevel = 15;
	PowerUpgrade.Cost = 10000;
	AvailableUpgrades.Add(PowerUpgrade);

	FMGNitroUpgrade EfficiencyUpgrade;
	EfficiencyUpgrade.UpgradeID = FName(TEXT("Efficiency1"));
	EfficiencyUpgrade.DisplayName = FText::FromString(TEXT("Fuel Efficiency"));
	EfficiencyUpgrade.EfficiencyBonus = 0.15f;
	EfficiencyUpgrade.UnlockLevel = 20;
	EfficiencyUpgrade.Cost = 12500;
	AvailableUpgrades.Add(EfficiencyUpgrade);
}

float UMGNitroBoostSubsystem::CalculateEffectiveMultiplier() const
{
	float Multiplier = NitroConfig.BoostMultiplier;

	// Apply boost level scaling
	Multiplier += (NitroState.BoostLevel - 1) * 0.15f;

	// Apply perfect boost bonus
	if (NitroState.bIsPerfectBoost)
	{
		Multiplier += PerfectBoostBonusPower;
	}

	// Apply boost zone multiplier
	if (ActiveBoostZone.ZoneID != NAME_None)
	{
		Multiplier *= ActiveBoostZone.BoostMultiplier;
	}

	return Multiplier;
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleUpgradeSystem.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"

UMGVehicleUpgradeSystem::UMGVehicleUpgradeSystem()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default tier costs (scale exponentially)
	TierCosts.Add(EMGUpgradeTier::Stock, 0);
	TierCosts.Add(EMGUpgradeTier::Street, 1000);
	TierCosts.Add(EMGUpgradeTier::Sport, 3000);
	TierCosts.Add(EMGUpgradeTier::Race, 7000);
	TierCosts.Add(EMGUpgradeTier::Pro, 15000);

	// Performance multipliers per tier
	TierMultipliers.Add(EMGUpgradeTier::Stock, 1.0f);
	TierMultipliers.Add(EMGUpgradeTier::Street, 1.15f);
	TierMultipliers.Add(EMGUpgradeTier::Sport, 1.35f);
	TierMultipliers.Add(EMGUpgradeTier::Race, 1.60f);
	TierMultipliers.Add(EMGUpgradeTier::Pro, 2.0f);
}

void UMGVehicleUpgradeSystem::BeginPlay()
{
	Super::BeginPlay();

	InitializeUpgrades();
	UE_LOG(LogTemp, Log, TEXT("[VehicleUpgrade] Upgrade system initialized for %s"), 
		*GetOwner()->GetName());
}

// ============================================
// UPGRADE MANAGEMENT
// ============================================

bool UMGVehicleUpgradeSystem::PurchaseUpgrade(EMGUpgradeCategory Category, int32 PlayerCash)
{
	if (!CurrentUpgrades.Contains(Category))
	{
		UE_LOG(LogTemp, Error, TEXT("[VehicleUpgrade] Invalid category"));
		return false;
	}

	FMGVehicleUpgrade& Upgrade = CurrentUpgrades[Category];

	// Check if already maxed
	if (IsMaxTier(Category))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VehicleUpgrade] %s already at max tier"), 
			*UEnum::GetValueAsString(Category));
		return false;
	}

	// Check if player can afford
	int32 Cost = GetNextTierCost(Category);
	if (PlayerCash < Cost)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VehicleUpgrade] Cannot afford %s upgrade ($%d needed, $%d available)"),
			*UEnum::GetValueAsString(Category), Cost, PlayerCash);
		return false;
	}

	// Upgrade to next tier
	int32 CurrentTierInt = static_cast<int32>(Upgrade.Tier);
	Upgrade.Tier = static_cast<EMGUpgradeTier>(CurrentTierInt + 1);
	Upgrade.PerformanceMultiplier = CalculateTierMultiplier(Upgrade.Tier);
	Upgrade.NextTierCost = CalculateNextTierCost(Upgrade.Tier);

	// Apply effects
	ApplyUpgradeEffects(Category);

	UE_LOG(LogTemp, Warning, TEXT("[VehicleUpgrade] UPGRADED %s to %s! (x%.2f performance)"),
		*UEnum::GetValueAsString(Category),
		*UEnum::GetValueAsString(Upgrade.Tier),
		Upgrade.PerformanceMultiplier);

	return true;
}

EMGUpgradeTier UMGVehicleUpgradeSystem::GetUpgradeTier(EMGUpgradeCategory Category) const
{
	if (CurrentUpgrades.Contains(Category))
	{
		return CurrentUpgrades[Category].Tier;
	}
	return EMGUpgradeTier::Stock;
}

int32 UMGVehicleUpgradeSystem::GetNextTierCost(EMGUpgradeCategory Category) const
{
	if (!CurrentUpgrades.Contains(Category))
	{
		return 0;
	}

	const FMGVehicleUpgrade& Upgrade = CurrentUpgrades[Category];
	if (IsMaxTier(Category))
	{
		return 0; // Already maxed
	}

	return CalculateNextTierCost(Upgrade.Tier);
}

bool UMGVehicleUpgradeSystem::CanAffordUpgrade(EMGUpgradeCategory Category, int32 PlayerCash) const
{
	if (IsMaxTier(Category))
	{
		return false;
	}

	int32 Cost = GetNextTierCost(Category);
	return PlayerCash >= Cost;
}

bool UMGVehicleUpgradeSystem::IsMaxTier(EMGUpgradeCategory Category) const
{
	if (!CurrentUpgrades.Contains(Category))
	{
		return false;
	}

	return CurrentUpgrades[Category].Tier == EMGUpgradeTier::Pro;
}

float UMGVehicleUpgradeSystem::GetOverallPerformanceRating() const
{
	float TotalMultiplier = 0.0f;
	int32 NumCategories = 0;

	// Average all category multipliers
	for (const auto& Pair : CurrentUpgrades)
	{
		TotalMultiplier += Pair.Value.PerformanceMultiplier;
		NumCategories++;
	}

	if (NumCategories == 0)
	{
		return 0.0f;
	}

	float AverageMultiplier = TotalMultiplier / NumCategories;

	// Convert to 0-100 scale (1.0 = 0, 2.0 = 100)
	return (AverageMultiplier - 1.0f) * 100.0f;
}

void UMGVehicleUpgradeSystem::ResetToStock()
{
	UE_LOG(LogTemp, Log, TEXT("[VehicleUpgrade] Resetting all upgrades to stock"));

	for (auto& Pair : CurrentUpgrades)
	{
		Pair.Value.Tier = EMGUpgradeTier::Stock;
		Pair.Value.PerformanceMultiplier = 1.0f;
		Pair.Value.NextTierCost = CalculateNextTierCost(EMGUpgradeTier::Stock);
		ApplyUpgradeEffects(Pair.Key);
	}
}

// ============================================
// UPGRADE DATA
// ============================================

FMGVehicleUpgrade UMGVehicleUpgradeSystem::GetUpgradeData(EMGUpgradeCategory Category) const
{
	if (CurrentUpgrades.Contains(Category))
	{
		return CurrentUpgrades[Category];
	}
	return FMGVehicleUpgrade();
}

TArray<FMGVehicleUpgrade> UMGVehicleUpgradeSystem::GetAllUpgrades() const
{
	TArray<FMGVehicleUpgrade> AllUpgrades;
	for (const auto& Pair : CurrentUpgrades)
	{
		AllUpgrades.Add(Pair.Value);
	}
	return AllUpgrades;
}

// ============================================
// INTERNAL
// ============================================

void UMGVehicleUpgradeSystem::InitializeUpgrades()
{
	// Initialize all upgrade categories to stock
	TArray<EMGUpgradeCategory> Categories = {
		EMGUpgradeCategory::Engine,
		EMGUpgradeCategory::Handling,
		EMGUpgradeCategory::Transmission,
		EMGUpgradeCategory::Weight,
		EMGUpgradeCategory::Nitrous
	};

	for (EMGUpgradeCategory Category : Categories)
	{
		FMGVehicleUpgrade Upgrade;
		Upgrade.Category = Category;
		Upgrade.Tier = EMGUpgradeTier::Stock;
		Upgrade.PerformanceMultiplier = 1.0f;
		Upgrade.NextTierCost = CalculateNextTierCost(EMGUpgradeTier::Stock);

		CurrentUpgrades.Add(Category, Upgrade);
	}
}

void UMGVehicleUpgradeSystem::ApplyUpgradeEffects(EMGUpgradeCategory Category)
{
	AMGVehiclePawn* Vehicle = GetOwnerVehicle();
	if (!Vehicle)
	{
		UE_LOG(LogTemp, Error, TEXT("[VehicleUpgrade] No owner vehicle found"));
		return;
	}

	UMG_VHCL_MovementComponent* Movement = Vehicle->FindComponentByClass<UMG_VHCL_MovementComponent>();
	if (!Movement)
	{
		UE_LOG(LogTemp, Error, TEXT("[VehicleUpgrade] No movement component found"));
		return;
	}

	const FMGVehicleUpgrade& Upgrade = CurrentUpgrades[Category];
	float Multiplier = Upgrade.PerformanceMultiplier;

	// Apply category-specific effects
	switch (Category)
	{
	case EMGUpgradeCategory::Engine:
		// Increase engine power and top speed
		Movement->SetEnginePowerMultiplier(Multiplier);
		UE_LOG(LogTemp, Log, TEXT("[VehicleUpgrade] Engine power: x%.2f"), Multiplier);
		break;

	case EMGUpgradeCategory::Handling:
		// Improve grip and steering response
		Movement->SetHandlingMultiplier(Multiplier);
		UE_LOG(LogTemp, Log, TEXT("[VehicleUpgrade] Handling: x%.2f"), Multiplier);
		break;

	case EMGUpgradeCategory::Transmission:
		// Faster acceleration
		Movement->SetAccelerationMultiplier(Multiplier);
		UE_LOG(LogTemp, Log, TEXT("[VehicleUpgrade] Acceleration: x%.2f"), Multiplier);
		break;

	case EMGUpgradeCategory::Weight:
		// Reduce effective mass (improves everything slightly)
		float WeightReduction = 1.0f / Multiplier; // Inverse for weight
		Movement->SetMassMultiplier(WeightReduction);
		UE_LOG(LogTemp, Log, TEXT("[VehicleUpgrade] Weight reduction: %.2f%%"), (1.0f - WeightReduction) * 100.0f);
		break;

	case EMGUpgradeCategory::Nitrous:
		// Increase boost capacity
		Movement->SetBoostCapacityMultiplier(Multiplier);
		UE_LOG(LogTemp, Log, TEXT("[VehicleUpgrade] Boost capacity: x%.2f"), Multiplier);
		break;
	}
}

AMGVehiclePawn* UMGVehicleUpgradeSystem::GetOwnerVehicle() const
{
	return Cast<AMGVehiclePawn>(GetOwner());
}

int32 UMGVehicleUpgradeSystem::CalculateNextTierCost(EMGUpgradeTier CurrentTier) const
{
	// Get cost of next tier
	int32 NextTierInt = static_cast<int32>(CurrentTier) + 1;
	if (NextTierInt > static_cast<int32>(EMGUpgradeTier::Pro))
	{
		return 0; // Already at max
	}

	EMGUpgradeTier NextTier = static_cast<EMGUpgradeTier>(NextTierInt);
	if (TierCosts.Contains(NextTier))
	{
		return TierCosts[NextTier];
	}

	return 0;
}

float UMGVehicleUpgradeSystem::CalculateTierMultiplier(EMGUpgradeTier Tier) const
{
	if (TierMultipliers.Contains(Tier))
	{
		return TierMultipliers[Tier];
	}
	return 1.0f;
}

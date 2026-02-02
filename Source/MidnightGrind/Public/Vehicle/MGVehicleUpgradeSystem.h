// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGVehicleUpgradeSystem.generated.h"

class AMGVehiclePawn;

/**
 * Simple Vehicle Upgrade System
 * 
 * Provides straightforward car progression:
 * - Engine upgrades (more power)
 * - Handling upgrades (better cornering)
 * - Transmission upgrades (faster acceleration)
 * - Weight reduction (better performance overall)
 * - Nitrous/Boost capacity
 * 
 * Design: Keep it simple and satisfying
 * - 5 tiers per category (Stock -> Pro)
 * - Clear performance improvements
 * - Reasonable costs that scale with tier
 * - Feel the difference immediately
 */

/** Upgrade category */
UENUM(BlueprintType)
enum class EMGUpgradeCategory : uint8
{
	Engine      UMETA(DisplayName = "Engine"),           // More power, higher top speed
	Handling    UMETA(DisplayName = "Handling"),         // Better grip, sharper steering
	Transmission UMETA(DisplayName = "Transmission"),    // Faster acceleration, quicker shifts
	Weight      UMETA(DisplayName = "Weight Reduction"), // Better overall performance
	Nitrous     UMETA(DisplayName = "Nitrous System")    // Bigger boost capacity
};

/** Upgrade tier */
UENUM(BlueprintType)
enum class EMGUpgradeTier : uint8
{
	Stock   UMETA(DisplayName = "Stock"),       // Base vehicle
	Street  UMETA(DisplayName = "Street"),      // Tier 1 upgrades
	Sport   UMETA(DisplayName = "Sport"),       // Tier 2 upgrades
	Race    UMETA(DisplayName = "Race"),        // Tier 3 upgrades
	Pro     UMETA(DisplayName = "Pro")          // Tier 4 upgrades (max)
};

/** Single upgrade applied to vehicle */
USTRUCT(BlueprintType)
struct FMGVehicleUpgrade
{
	GENERATED_BODY()

	/** Upgrade category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	EMGUpgradeCategory Category;

	/** Current tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	EMGUpgradeTier Tier;

	/** Cost to upgrade to next tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	int32 NextTierCost;

	/** Performance multiplier (1.0 = stock, 2.0 = double) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	float PerformanceMultiplier;

	FMGVehicleUpgrade()
		: Category(EMGUpgradeCategory::Engine)
		, Tier(EMGUpgradeTier::Stock)
		, NextTierCost(1000)
		, PerformanceMultiplier(1.0f)
	{}
};

/** Upgrade cost and stats for a specific tier */
USTRUCT(BlueprintType)
struct FMGUpgradeTierData
{
	GENERATED_BODY()

	/** Tier name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	FText TierName;

	/** Cost to purchase this tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	int32 Cost;

	/** Performance multiplier at this tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	float PerformanceMultiplier;

	/** Visual description of improvements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	FText Description;

	FMGUpgradeTierData()
		: TierName(FText::FromString(TEXT("Stock")))
		, Cost(0)
		, PerformanceMultiplier(1.0f)
		, Description(FText::FromString(TEXT("Factory stock")))
	{}
};

/**
 * Vehicle Upgrade Component
 * 
 * Attach to vehicle to enable upgrades.
 * Manages upgrade state and applies performance modifications.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGVehicleUpgradeSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGVehicleUpgradeSystem();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

	// ============================================
	// UPGRADE MANAGEMENT
	// ============================================

	/** Purchase upgrade for category (returns true if successful) */
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	bool PurchaseUpgrade(EMGUpgradeCategory Category, int32 PlayerCash);

	/** Get current tier for category */
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	EMGUpgradeTier GetUpgradeTier(EMGUpgradeCategory Category) const;

	/** Get cost of next tier for category */
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	int32 GetNextTierCost(EMGUpgradeCategory Category) const;

	/** Can player afford next tier? */
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	bool CanAffordUpgrade(EMGUpgradeCategory Category, int32 PlayerCash) const;

	/** Is category at max tier? */
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	bool IsMaxTier(EMGUpgradeCategory Category) const;

	/** Get total performance rating (0-100) */
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	float GetOverallPerformanceRating() const;

	/** Reset all upgrades to stock */
	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	void ResetToStock();

	// ============================================
	// UPGRADE DATA
	// ============================================

	/** Get upgrade data for category */
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	FMGVehicleUpgrade GetUpgradeData(EMGUpgradeCategory Category) const;

	/** Get all upgrades */
	UFUNCTION(BlueprintPure, Category = "Upgrades")
	TArray<FMGVehicleUpgrade> GetAllUpgrades() const;

protected:
	// ============================================
	// UPGRADE STATE
	// ============================================

	/** Current upgrades applied to vehicle */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Upgrades")
	TMap<EMGUpgradeCategory, FMGVehicleUpgrade> CurrentUpgrades;

	// ============================================
	// UPGRADE CONFIGURATION
	// ============================================

	/** Upgrade costs per tier (configurable in Blueprint) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TMap<EMGUpgradeTier, int32> TierCosts;

	/** Performance multipliers per tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TMap<EMGUpgradeTier, float> TierMultipliers;

	// ============================================
	// INTERNAL
	// ============================================

	/** Initialize upgrade state */
	void InitializeUpgrades();

	/** Apply upgrade effects to vehicle */
	void ApplyUpgradeEffects(EMGUpgradeCategory Category);

	/** Get owner vehicle */
	AMGVehiclePawn* GetOwnerVehicle() const;

	/** Calculate cost for next tier */
	int32 CalculateNextTierCost(EMGUpgradeTier CurrentTier) const;

	/** Calculate multiplier for tier */
	float CalculateTierMultiplier(EMGUpgradeTier Tier) const;
};

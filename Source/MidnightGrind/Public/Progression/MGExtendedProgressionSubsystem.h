// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGExtendedProgressionSubsystem.h
 * @brief Extended progression manager for 150-200h single-player experience
 * 
 * This subsystem builds upon MGPlayerProgression to add:
 * - Car tier progression and ownership tracking
 * - Location/district unlock management
 * - Housing progression system
 * - Deep customization tracking
 * - Enhanced prerequisite checking with AND/OR logic
 * - Milestone and "wow moment" tracking
 * - Anti-frustration catch-up systems
 * 
 * @see MGProgressionTypes.h for data structures
 * @see MGPlayerProgression.h for base progression (XP, Rep, Stats)
 * @see Progression-Systems-Design.md for complete design documentation
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Progression/MGProgressionTypes.h"
#include "Engine/DataTable.h"
#include "MGExtendedProgressionSubsystem.generated.h"

class UMGPlayerProgression;

// =============================================================================
// DELEGATES - Extended Events
// =============================================================================

/** Broadcast when player purchases/unlocks a new car */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCarAcquired, FName, CarID, EMGCarTier, Tier);

/** Broadcast when a new location is unlocked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLocationUnlocked, FName, LocationID, EMGDistrict, District);

/** Broadcast when player upgrades housing */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHousingUpgraded, EMGHousingTier, NewTier, EMGHousingTier, OldTier);

/** Broadcast when a milestone is completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMilestoneCompleted, const FMGMilestone&, Milestone);

/** Broadcast when customization item is unlocked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomizationUnlocked, EMGCustomizationType, Category, FName, ItemID);

/** Broadcast when player money changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoneyChanged, int64, NewAmount, int64, Delta);

/**
 * @class UMGExtendedProgressionSubsystem
 * @brief Game Instance Subsystem for extended progression systems
 * 
 * Manages the deep progression layers that support 150-200 hours of gameplay:
 * - **Car Progression**: 52 cars across 5 tiers with usage-based unlocks
 * - **Location System**: 6 districts with 24 sub-locations
 * - **Housing**: 4 tiers from Garage to Penthouse
 * - **Customization**: 400+ items across 7 categories
 * - **Milestones**: Major "wow moments" throughout the experience
 * 
 * ## Usage Example
 * @code
 * auto* ProgSys = GetGameInstance()->GetSubsystem<UMGExtendedProgressionSubsystem>();
 * 
 * // Check if player can unlock a car
 * FText Reason;
 * if (ProgSys->CanUnlockCar(TEXT("Car_TunerNSX"), Reason))
 * {
 *     ProgSys->UnlockCar(TEXT("Car_TunerNSX"));
 * }
 * 
 * // Track car usage for per-car progression
 * ProgSys->AddCarUsage(TEXT("Car_StarterCivic"), 10.5f); // 10.5 km driven
 * 
 * // Check housing upgrade availability
 * if (ProgSys->CanUpgradeHousing(EMGHousingTier::Apartment, Reason))
 * {
 *     ProgSys->UpgradeHousing(EMGHousingTier::Apartment);
 * }
 * @endcode
 * 
 * @note This subsystem works alongside MGPlayerProgression for complete progression tracking
 */
UCLASS()
class MIDNIGHTGRIND_API UMGExtendedProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ==========================================
	// MONEY SYSTEM
	// ==========================================

	/** Get current player money */
	UFUNCTION(BlueprintPure, Category = "Progression|Money")
	int64 GetMoney() const { return PlayerMoney; }

	/** Add money to player (can be negative for spending) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Money")
	void AddMoney(int64 Amount, bool bNotify = true);

	/** Check if player can afford something */
	UFUNCTION(BlueprintPure, Category = "Progression|Money")
	bool CanAfford(int64 Cost) const { return PlayerMoney >= Cost; }

	/** Spend money (returns true if successful) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Money")
	bool SpendMoney(int64 Amount);

	// ==========================================
	// CAR PROGRESSION
	// ==========================================

	/** Check if player owns a specific car */
	UFUNCTION(BlueprintPure, Category = "Progression|Cars")
	bool OwnsC Car(FName CarID) const;

	/** Check if player owns any car in a specific tier */
	UFUNCTION(BlueprintPure, Category = "Progression|Cars")
	bool OwnsCarInTier(EMGCarTier Tier) const;

	/** Get all owned cars */
	UFUNCTION(BlueprintPure, Category = "Progression|Cars")
	TArray<FMGOwnedCar> GetOwnedCars() const { return OwnedCars; }

	/** Get owned cars filtered by tier */
	UFUNCTION(BlueprintCallable, Category = "Progression|Cars")
	TArray<FMGOwnedCar> GetOwnedCarsByTier(EMGCarTier Tier) const;

	/** Get specific owned car data */
	UFUNCTION(BlueprintCallable, Category = "Progression|Cars")
	bool GetOwnedCarData(FName CarID, FMGOwnedCar& OutCarData) const;

	/** Check if car can be unlocked (returns reason if not) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Cars")
	bool CanUnlockCar(FName CarID, FText& OutReason) const;

	/** Unlock/purchase a car (handles cost and prerequisites) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Cars")
	bool UnlockCar(FName CarID);

	/** Add usage statistics for a car (triggers per-car progression) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Cars")
	void AddCarUsage(FName CarID, float DistanceKm, bool bRaceWon = false);

	/** Check if car performance upgrade is available */
	UFUNCTION(BlueprintCallable, Category = "Progression|Cars")
	bool CanUpgradeCarPerformance(FName CarID, int32& OutCurrentStage, int32& OutNextStage) const;

	/** Upgrade car performance stage */
	UFUNCTION(BlueprintCallable, Category = "Progression|Cars")
	bool UpgradeCarPerformance(FName CarID);

	/** Get highest car tier owned by player */
	UFUNCTION(BlueprintPure, Category = "Progression|Cars")
	EMGCarTier GetHighestOwnedTier() const;

	// ==========================================
	// LOCATION PROGRESSION
	// ==========================================

	/** Check if location is unlocked */
	UFUNCTION(BlueprintPure, Category = "Progression|Locations")
	bool IsLocationUnlocked(FName LocationID) const;

	/** Check if entire district is unlocked */
	UFUNCTION(BlueprintPure, Category = "Progression|Locations")
	bool IsDistrictUnlocked(EMGDistrict District) const;

	/** Get all unlocked locations */
	UFUNCTION(BlueprintPure, Category = "Progression|Locations")
	TArray<FMGUnlockedLocation> GetUnlockedLocations() const { return UnlockedLocations; }

	/** Get unlocked locations in a specific district */
	UFUNCTION(BlueprintCallable, Category = "Progression|Locations")
	TArray<FMGUnlockedLocation> GetUnlockedLocationsInDistrict(EMGDistrict District) const;

	/** Check if location can be unlocked */
	UFUNCTION(BlueprintCallable, Category = "Progression|Locations")
	bool CanUnlockLocation(FName LocationID, FText& OutReason) const;

	/** Unlock a location */
	UFUNCTION(BlueprintCallable, Category = "Progression|Locations")
	bool UnlockLocation(FName LocationID);

	/** Record race completion at location (for tracking) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Locations")
	void RecordLocationRaceCompletion(FName LocationID);

	// ==========================================
	// HOUSING PROGRESSION
	// ==========================================

	/** Get current housing tier */
	UFUNCTION(BlueprintPure, Category = "Progression|Housing")
	EMGHousingTier GetCurrentHousing() const { return HousingData.CurrentTier; }

	/** Get maximum car display capacity for current housing */
	UFUNCTION(BlueprintPure, Category = "Progression|Housing")
	int32 GetMaxCarDisplay() const { return HousingData.MaxDisplayCapacity; }

	/** Check if housing upgrade is available */
	UFUNCTION(BlueprintCallable, Category = "Progression|Housing")
	bool CanUpgradeHousing(EMGHousingTier TargetTier, FText& OutReason) const;

	/** Upgrade to new housing tier */
	UFUNCTION(BlueprintCallable, Category = "Progression|Housing")
	bool UpgradeHousing(EMGHousingTier TargetTier);

	/** Check if housing cosmetic is unlocked */
	UFUNCTION(BlueprintPure, Category = "Progression|Housing")
	bool IsHousingCosmeticUnlocked(FName CosmeticID) const;

	/** Unlock housing cosmetic item */
	UFUNCTION(BlueprintCallable, Category = "Progression|Housing")
	bool UnlockHousingCosmetic(FName CosmeticID);

	// ==========================================
	// CUSTOMIZATION PROGRESSION
	// ==========================================

	/** Check if customization item is unlocked */
	UFUNCTION(BlueprintPure, Category = "Progression|Customization")
	bool IsCustomizationUnlocked(FName ItemID, EMGCustomizationType Category) const;

	/** Get all unlocked items in a category */
	UFUNCTION(BlueprintCallable, Category = "Progression|Customization")
	TArray<FName> GetUnlockedCustomization(EMGCustomizationType Category) const;

	/** Get count of unlocked items in category */
	UFUNCTION(BlueprintPure, Category = "Progression|Customization")
	int32 GetCustomizationUnlockCount(EMGCustomizationType Category) const;

	/** Check if customization can be unlocked */
	UFUNCTION(BlueprintCallable, Category = "Progression|Customization")
	bool CanUnlockCustomization(FName ItemID, FText& OutReason) const;

	/** Unlock customization item */
	UFUNCTION(BlueprintCallable, Category = "Progression|Customization")
	bool UnlockCustomization(FName ItemID, EMGCustomizationType Category);

	// ==========================================
	// MILESTONE SYSTEM
	// ==========================================

	/** Get all completed milestones */
	UFUNCTION(BlueprintPure, Category = "Progression|Milestones")
	TArray<FMGMilestone> GetCompletedMilestones() const { return CompletedMilestones; }

	/** Get next uncompleted milestone */
	UFUNCTION(BlueprintCallable, Category = "Progression|Milestones")
	bool GetNextMilestone(FMGMilestone& OutMilestone) const;

	/** Check if milestone is completed */
	UFUNCTION(BlueprintPure, Category = "Progression|Milestones")
	bool IsMilestoneCompleted(FName MilestoneID) const;

	/** Check and grant newly available milestones */
	UFUNCTION(BlueprintCallable, Category = "Progression|Milestones")
	TArray<FMGMilestone> CheckAndGrantMilestones();

	// ==========================================
	// PREREQUISITE SYSTEM
	// ==========================================

	/** Check if a single prerequisite is met */
	UFUNCTION(BlueprintCallable, Category = "Progression|Requirements")
	bool CheckPrerequisite(const FMGPrerequisite& Prereq) const;

	/** Check if unlock requirements are met (returns failure reason if not) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Requirements")
	bool CheckUnlockRequirements(const FMGUnlockRequirement& Requirements, FText& OutReason) const;

	/** Get human-readable description of requirements */
	UFUNCTION(BlueprintCallable, Category = "Progression|Requirements")
	FText GetRequirementDescription(const FMGUnlockRequirement& Requirements) const;

	// ==========================================
	// PROGRESSION QUERIES
	// ==========================================

	/** Get all available car unlocks (requirements met, not yet owned) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Query")
	TArray<FMGCarUnlockData> GetAvailableCarUnlocks() const;

	/** Get all available location unlocks */
	UFUNCTION(BlueprintCallable, Category = "Progression|Query")
	TArray<FMGLocationUnlockData> GetAvailableLocationUnlocks() const;

	/** Get recommended next unlock (for UI guidance) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Query")
	FString GetRecommendedNextUnlock() const;

	/** Check if player is in a "dead zone" (no progress in X time) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Query")
	bool IsInDeadZone(float& OutHoursSinceLastUnlock) const;

	// ==========================================
	// ANTI-FRUSTRATION SYSTEMS
	// ==========================================

	/** Check if player needs catch-up bonus (behind expected curve) */
	UFUNCTION(BlueprintCallable, Category = "Progression|CatchUp")
	bool ShouldApplyCatchUpBonus(float& OutBonusMultiplier) const;

	/** Apply catch-up bonus (hidden from player, automatic) */
	UFUNCTION(BlueprintCallable, Category = "Progression|CatchUp")
	void ApplyCatchUpBonus();

	/** Get difficulty adjustment (if player is struggling) */
	UFUNCTION(BlueprintCallable, Category = "Progression|CatchUp")
	float GetDifficultyAdjustment() const;

	// ==========================================
	// DATA TABLE LOADING
	// ==========================================

	/** Load all progression data from data tables */
	UFUNCTION(BlueprintCallable, Category = "Progression|Data")
	void LoadProgressionData();

	/** Set data table references (called by game mode or blueprint) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Data")
	void SetDataTables(
		UDataTable* Cars,
		UDataTable* Locations,
		UDataTable* Housing,
		UDataTable* Customizations,
		UDataTable* Milestones);

	// ==========================================
	// DEBUG / DEVELOPMENT
	// ==========================================

#if WITH_EDITOR
	/** Unlock everything for testing */
	UFUNCTION(BlueprintCallable, Category = "Progression|Debug")
	void Debug_UnlockAll();

	/** Reset all progression */
	UFUNCTION(BlueprintCallable, Category = "Progression|Debug")
	void Debug_ResetProgression();

	/** Set player progression to specific hour mark (for testing) */
	UFUNCTION(BlueprintCallable, Category = "Progression|Debug")
	void Debug_SetProgressionHour(int32 TargetHour);
#endif

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnCarAcquired OnCarAcquired;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnLocationUnlocked OnLocationUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnHousingUpgraded OnHousingUpgraded;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnMilestoneCompleted OnMilestoneCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnCustomizationUnlocked OnCustomizationUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnMoneyChanged OnMoneyChanged;

protected:
	// ==========================================
	// INTERNAL HELPERS
	// ==========================================

	/** Get the base progression subsystem */
	UMGPlayerProgression* GetBaseProgression() const;

	/** Load car unlock data from data table */
	void LoadCarUnlocks();

	/** Load location unlock data from data table */
	void LoadLocationUnlocks();

	/** Load housing unlock data from data table */
	void LoadHousingUnlocks();

	/** Load customization unlock data from data table */
	void LoadCustomizationUnlocks();

	/** Load milestone data from data table */
	void LoadMilestones();

	/** Find owned car entry (mutable) */
	FMGOwnedCar* FindOwnedCar(FName CarID);

	/** Find unlocked location entry (mutable) */
	FMGUnlockedLocation* FindUnlockedLocation(FName LocationID);

	/** Get or create customization progress for category */
	FMGCustomizationProgress& GetOrCreateCustomizationProgress(EMGCustomizationType Category);

	/** Calculate expected progression at current play time */
	float GetExpectedProgressionCurve() const;

	// ==========================================
	// SAVED DATA
	// ==========================================

	/** Player money */
	UPROPERTY(SaveGame)
	int64 PlayerMoney = 0;

	/** Owned cars with usage tracking */
	UPROPERTY(SaveGame)
	TArray<FMGOwnedCar> OwnedCars;

	/** Unlocked locations */
	UPROPERTY(SaveGame)
	TArray<FMGUnlockedLocation> UnlockedLocations;

	/** Housing progression data */
	UPROPERTY(SaveGame)
	FMGHousingData HousingData;

	/** Customization unlocks per category */
	UPROPERTY(SaveGame)
	TArray<FMGCustomizationProgress> CustomizationProgress;

	/** Completed milestones */
	UPROPERTY(SaveGame)
	TArray<FMGMilestone> CompletedMilestones;

	/** Timestamp of last significant unlock (for dead zone detection) */
	UPROPERTY(SaveGame)
	FDateTime LastUnlockTime;

	/** Consecutive race losses (for difficulty adjustment) */
	UPROPERTY(SaveGame)
	int32 ConsecutiveLosses = 0;

	// ==========================================
	// RUNTIME DATA (Loaded from Data Tables)
	// ==========================================

	/** Car unlock definitions */
	UPROPERTY()
	TMap<FName, FMGCarUnlockData> CarUnlockDatabase;

	/** Location unlock definitions */
	UPROPERTY()
	TMap<FName, FMGLocationUnlockData> LocationUnlockDatabase;

	/** Housing tier definitions */
	UPROPERTY()
	TMap<EMGHousingTier, FMGHousingUnlockData> HousingUnlockDatabase;

	/** Customization unlock definitions */
	UPROPERTY()
	TMap<FName, FMGCustomizationUnlockData> CustomizationUnlockDatabase;

	/** Milestone definitions */
	UPROPERTY()
	TMap<FName, FMGMilestoneData> MilestoneDatabase;

	// ==========================================
	// DATA TABLE REFERENCES
	// ==========================================

	UPROPERTY()
	UDataTable* CarUnlockTable = nullptr;

	UPROPERTY()
	UDataTable* LocationUnlockTable = nullptr;

	UPROPERTY()
	UDataTable* HousingUnlockTable = nullptr;

	UPROPERTY()
	UDataTable* CustomizationUnlockTable = nullptr;

	UPROPERTY()
	UDataTable* MilestoneTable = nullptr;
};

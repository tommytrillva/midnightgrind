// Copyright Midnight Grind. All Rights Reserved.
// Iteration 82: Parts Catalog Subsystem - Runtime lookups for parts data

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Catalog/MGCatalogTypes.h"
#include "MGPartsCatalogSubsystem.generated.h"

class UDataTable;

/**
 * Simplified part info for quick lookups
 */
USTRUCT(BlueprintType)
struct FMGPartPricingInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	int32 BasePrice = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	int32 LaborCost = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	float InstallTime = 60.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	bool bIsValid = false;
};

/**
 * Part specialization info for mechanic system
 */
USTRUCT(BlueprintType)
struct FMGPartSpecializationInfo
{
	GENERATED_BODY()

	/** Part category determines mechanic specialization required */
	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	EMGPartCategory Category = EMGPartCategory::Engine;

	/** Sub-category for more specific matching */
	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	FString SubCategory;

	/** Required skill level (1-10) */
	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	int32 RequiredSkillLevel = 1;

	/** Install time in minutes */
	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	float InstallTime = 60.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	bool bIsValid = false;
};

/**
 * Parts Catalog Subsystem
 *
 * Provides runtime access to parts catalog data from DataTables.
 * Used by mechanic system for specialization lookups, by shops for
 * pricing, and by customization systems for compatibility checking.
 *
 * DataTable Setup:
 * - Create DataTable asset with row struct: FMGPartCatalogRow
 * - Set PartsCatalogTable property to reference the DataTable
 * - Rows use PartID as the row name (FName key)
 *
 * Usage:
 * @code
 * UMGPartsCatalogSubsystem* Catalog = GetGameInstance()->GetSubsystem<UMGPartsCatalogSubsystem>();
 * FMGPartSpecializationInfo Spec = Catalog->GetPartSpecialization(FName("TURBO_SMALL"));
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPartsCatalogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== Pricing Lookups ==========

	/**
	 * Get part pricing information
	 * @param PartID Part identifier (e.g. "TURBO_SMALL")
	 * @return Pricing info struct with bIsValid set to false if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Pricing")
	FMGPartPricingInfo GetPartPricing(FName PartID) const;

	/**
	 * Get base price for a part
	 * @param PartID Part identifier
	 * @return Base price, or -1 if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Pricing")
	int32 GetPartBasePrice(FName PartID) const;

	/**
	 * Get total cost for part installation (price + labor)
	 * @param PartID Part identifier
	 * @return Total installation cost, or -1 if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Pricing")
	int32 GetPartTotalCost(FName PartID) const;

	/**
	 * Calculate part price with vehicle-specific multiplier
	 * @param PartID Part identifier
	 * @param VehiclePriceMultiplier Vehicle's parts price multiplier
	 * @return Adjusted price, or -1 if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Pricing")
	int32 GetAdjustedPartPrice(FName PartID, float VehiclePriceMultiplier) const;

	// ========== Specialization Lookups (For Mechanic System) ==========

	/**
	 * Get part specialization info for mechanic matching
	 * Resolves TODO: Determine specialization needed from part type
	 * @param PartID Part identifier
	 * @return Specialization info struct
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Mechanic")
	FMGPartSpecializationInfo GetPartSpecialization(FName PartID) const;

	/**
	 * Get the category for a part (Engine, Drivetrain, etc.)
	 * Used for mechanic specialization matching
	 * @param PartID Part identifier
	 * @return Part category enum value
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Mechanic")
	EMGPartCategory GetPartCategory(FName PartID) const;

	/**
	 * Get the sub-category string for a part
	 * @param PartID Part identifier
	 * @return Sub-category string (e.g. "Intake", "Exhaust")
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Mechanic")
	FString GetPartSubCategory(FName PartID) const;

	/**
	 * Check if a mechanic specialization matches a part category
	 * Resolves TODO: Check if part matches specialization for bonus
	 * @param MechanicSpecialization Mechanic's specialization category
	 * @param PartID Part to check
	 * @return True if mechanic specializes in this part type
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Mechanic")
	bool DoesSpecializationMatchPart(EMGPartCategory MechanicSpecialization, FName PartID) const;

	/**
	 * Get install time for a part
	 * Resolves TODO: Look up from parts catalog (install time)
	 * @param PartID Part identifier
	 * @return Install time in minutes, or default if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Mechanic")
	float GetPartInstallTime(FName PartID) const;

	/**
	 * Get required skill level for part installation
	 * Resolves TODO: Look up from parts catalog (skill level)
	 * @param PartID Part identifier
	 * @return Required skill level (1-10), or 1 if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Mechanic")
	int32 GetPartRequiredSkillLevel(FName PartID) const;

	// ========== Data Lookups ==========

	/**
	 * Get full part catalog data
	 * @param PartID Part identifier
	 * @param OutData Output parameter for part data
	 * @return True if part found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog")
	bool GetPartData(FName PartID, FMGPartCatalogRow& OutData) const;

	/**
	 * Check if a part exists in the catalog
	 * @param PartID Part identifier
	 * @return True if part is cataloged
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog")
	bool PartExists(FName PartID) const;

	/**
	 * Get display name for a part
	 * @param PartID Part identifier
	 * @return Display name, or "Unknown Part" if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog")
	FText GetPartDisplayName(FName PartID) const;

	/**
	 * Get part tier
	 * @param PartID Part identifier
	 * @return Part tier enum value
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog")
	EMGPartTier GetPartTier(FName PartID) const;

	// ========== Compatibility Checks ==========

	/**
	 * Check if a part is compatible with a vehicle
	 * @param PartID Part identifier
	 * @param VehicleID Vehicle identifier
	 * @return True if compatible (or universal part)
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Compatibility")
	bool IsPartCompatibleWithVehicle(FName PartID, FName VehicleID) const;

	/**
	 * Get all parts compatible with a specific vehicle
	 * @param VehicleID Vehicle identifier
	 * @return Array of compatible parts
	 */
	UFUNCTION(BlueprintCallable, Category = "Parts Catalog|Compatibility")
	TArray<FMGPartCatalogRow> GetPartsForVehicle(FName VehicleID) const;

	/**
	 * Check if required parts are installed
	 * @param PartID Part to install
	 * @param InstalledParts Array of currently installed part IDs
	 * @return True if all prerequisites are met
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Compatibility")
	bool ArePrerequisitesMet(FName PartID, const TArray<FName>& InstalledParts) const;

	/**
	 * Check if a part conflicts with installed parts
	 * @param PartID Part to install
	 * @param InstalledParts Array of currently installed part IDs
	 * @return True if there's a conflict
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Compatibility")
	bool HasConflictingParts(FName PartID, const TArray<FName>& InstalledParts) const;

	// ========== Filtering ==========

	/**
	 * Get all parts in a specific category
	 * @param Category Part category (Engine, Drivetrain, etc.)
	 * @return Array of matching parts
	 */
	UFUNCTION(BlueprintCallable, Category = "Parts Catalog|Filtering")
	TArray<FMGPartCatalogRow> GetPartsByCategory(EMGPartCategory Category) const;

	/**
	 * Get all parts of a specific tier
	 * @param Tier Part tier (Stock, Street, Race, etc.)
	 * @return Array of matching parts
	 */
	UFUNCTION(BlueprintCallable, Category = "Parts Catalog|Filtering")
	TArray<FMGPartCatalogRow> GetPartsByTier(EMGPartTier Tier) const;

	/**
	 * Get all parts within a price range
	 * @param MinPrice Minimum base price
	 * @param MaxPrice Maximum base price
	 * @return Array of matching parts
	 */
	UFUNCTION(BlueprintCallable, Category = "Parts Catalog|Filtering")
	TArray<FMGPartCatalogRow> GetPartsInPriceRange(int32 MinPrice, int32 MaxPrice) const;

	/**
	 * Get all part IDs in the catalog
	 * @return Array of all cataloged part IDs
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog")
	TArray<FName> GetAllPartIDs() const;

	/**
	 * Get count of parts in catalog
	 * @return Number of cataloged parts
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog")
	int32 GetPartCount() const;

	// ========== Performance Stats ==========

	/**
	 * Get performance impact of a part
	 * @param PartID Part identifier
	 * @param OutPowerBonus Output: HP gain
	 * @param OutTorqueBonus Output: Torque gain
	 * @param OutWeightChange Output: Weight change (negative = lighter)
	 * @param OutPIChange Output: Performance Index change
	 * @return True if part found
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog|Performance")
	bool GetPartPerformanceStats(FName PartID, int32& OutPowerBonus, int32& OutTorqueBonus,
		int32& OutWeightChange, int32& OutPIChange) const;

	// ========== Validation ==========

	/**
	 * Check if the catalog is properly loaded
	 * @return True if DataTable is valid and loaded
	 */
	UFUNCTION(BlueprintPure, Category = "Parts Catalog")
	bool IsCatalogLoaded() const;

	/**
	 * Force reload of catalog data (for editor/debug use)
	 */
	UFUNCTION(BlueprintCallable, Category = "Parts Catalog")
	void ReloadCatalog();

protected:
	/** Build internal lookup cache from DataTable */
	void BuildCache();

	/** Clear internal caches */
	void ClearCache();

	// ========== Configuration ==========

	/** Reference to parts catalog DataTable asset */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> PartsCatalogTableRef;

	/** Loaded DataTable pointer */
	UPROPERTY(Transient)
	UDataTable* PartsCatalogTable = nullptr;

	// ========== Cached Data ==========

	/** Cached part data for fast lookups */
	UPROPERTY(Transient)
	TMap<FName, FMGPartCatalogRow> PartCache;

	/** Cache of parts by category for fast filtering */
	TMap<EMGPartCategory, TArray<FName>> PartsByCategory;

	/** Cache of parts by vehicle for fast compatibility checks */
	TMap<FName, TArray<FName>> PartsByVehicle;

	/** Flag indicating if cache is built */
	bool bCacheBuilt = false;

private:
	/** Default pricing info returned when part not found */
	static const FMGPartPricingInfo InvalidPricingInfo;

	/** Default specialization info returned when part not found */
	static const FMGPartSpecializationInfo InvalidSpecializationInfo;
};

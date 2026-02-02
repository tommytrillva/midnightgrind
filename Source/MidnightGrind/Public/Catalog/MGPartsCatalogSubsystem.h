// Copyright Midnight Grind. All Rights Reserved.


#pragma once
// Iteration 82: Parts Catalog Subsystem - Runtime lookups for parts data

/**
 * =============================================================================
 * MGPartsCatalogSubsystem.h - Parts Data Access Layer
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This subsystem provides centralized access to the aftermarket parts catalog.
 * It's the "parts database" that other systems query to get pricing, specs,
 * compatibility info, and mechanic requirements for any part in the game.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. PARTS VS. VEHICLES:
 *    Vehicles are the "canvas", parts are the "brushstrokes".
 *    This subsystem handles the parts data, while MGVehicleCatalogSubsystem
 *    handles vehicle data. They work together for compatibility checking.
 *
 * 2. MECHANIC SYSTEM INTEGRATION:
 *    Parts have skill requirements and install times. The mechanic system
 *    uses GetPartSpecialization() to determine:
 *    - Which mechanic NPCs can install a part (by specialization match)
 *    - Install time (affected by mechanic skill level)
 *    - Labor cost calculations
 *
 *    Example: A "Race Turbo" might require skill level 7, take 180 minutes
 *    to install, and match the "Engine" specialization.
 *
 * 3. COMPATIBILITY SYSTEM:
 *    Parts can be universal (fit any car) or vehicle-specific.
 *    - IsPartCompatibleWithVehicle() checks basic compatibility
 *    - ArePrerequisitesMet() checks if required parts are installed
 *    - HasConflictingParts() checks for incompatible combinations
 *
 *    For ADVANCED dependency logic (warnings, failure risk), see
 *    MGPartDependencySubsystem instead.
 *
 * 4. PRICING WITH MULTIPLIERS:
 *    Parts have a base price, but actual cost varies by vehicle.
 *    A turbo might cost $5,000 base, but on an exotic car with a
 *    2.5x parts multiplier, it costs $12,500.
 *
 *    GetAdjustedPartPrice() handles this calculation.
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 *
 *    [DataTable Asset]
 *           |
 *           v
 *    [MGPartsCatalogSubsystem] <--- Cache for fast lookups
 *           |
 *           +---> [Parts Shop UI] - Shows available upgrades and prices
 *           |
 *           +---> [Mechanic System] - Determines install requirements
 *           |
 *           +---> [Tuning Menu] - Displays part effects on vehicle
 *           |
 *           +---> [MGPartDependencySubsystem] - Advanced compatibility checks
 *           |
 *           +---> [Performance Calculator] - Sums part bonuses for PI
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * // Get subsystem
 * UMGPartsCatalogSubsystem* Parts = GetGameInstance()->GetSubsystem<UMGPartsCatalogSubsystem>();
 *
 * // Check if part fits vehicle before showing in shop
 * if (Parts->IsPartCompatibleWithVehicle(PartID, CurrentVehicleID))
 * {
 *     // Calculate price with vehicle multiplier
 *     int32 Price = Parts->GetAdjustedPartPrice(PartID, VehiclePriceMultiplier);
 * }
 *
 * // Get mechanic info for install dialog
 * FMGPartSpecializationInfo Spec = Parts->GetPartSpecialization(PartID);
 * float InstallMinutes = Spec.InstallTime;
 * EMGPartCategory RequiredSpec = Spec.Category;
 *
 * RELATED SYSTEMS:
 * ----------------
 * - MGVehicleCatalogSubsystem: Vehicle master data (used for price multipliers)
 * - MGPartDependencySubsystem: Advanced dependency rules and failure simulation
 * - MGMechanicSubsystem: NPC mechanics who install parts
 * - MGInventorySubsystem: Tracks which parts the player owns/has installed
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Catalog/MGCatalogTypes.h"
#include "MGPartsCatalogSubsystem.generated.h"

// Forward declaration
class UDataTable;

// =============================================================================
// LIGHTWEIGHT RETURN STRUCTS
// These are simplified versions of catalog data for specific use cases
// =============================================================================

/**
 * Simplified part pricing info for quick lookups.
 *
 * Used when a system only needs pricing data, not full part details.
 * This is returned by GetPartPricing() and contains just the essential
 * cost information.
 *
 * TOTAL COST CALCULATION:
 * The full cost of installing a part is:
 *   TotalCost = (BasePrice * VehiclePriceMultiplier) + LaborCost
 *
 * Where VehiclePriceMultiplier comes from the vehicle catalog.
 *
 * The bIsValid flag MUST be checked before using other values - it indicates
 * whether the part lookup succeeded.
 */
USTRUCT(BlueprintType)
struct FMGPartPricingInfo
{
	GENERATED_BODY()

	/** Part cost before vehicle multipliers */
	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	int32 BasePrice = 0;

	/** Mechanic labor cost (flat fee, not multiplied) */
	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	int32 LaborCost = 0;

	/** Installation time in minutes (affects mechanic availability) */
	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	float InstallTime = 60.0f;

	/** Did the lookup succeed? Always check this first! */
	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	bool bIsValid = false;
};

/**
 * Part specialization info for the mechanic system.
 *
 * This struct contains everything the mechanic system needs to know
 * about installing a specific part. It's used to:
 *
 * 1. MATCH MECHANICS TO PARTS:
 *    Mechanics have specializations (Engine, Suspension, etc.)
 *    A part's Category must match a mechanic's specialization for
 *    them to install it (or they take longer/charge more).
 *
 * 2. CHECK SKILL REQUIREMENTS:
 *    RequiredSkillLevel (1-10) determines if a mechanic CAN install
 *    the part at all. A level 3 mechanic can't install a level 7 part.
 *
 * 3. SCHEDULE WORK:
 *    InstallTime determines how long the mechanic is "busy" after
 *    starting the job. This affects when the player can pick up their car.
 *
 * SKILL LEVEL GUIDELINES:
 * - 1-2: Basic parts (filters, spark plugs)
 * - 3-4: Bolt-on upgrades (exhaust, intake)
 * - 5-6: Performance parts (turbo kits, coilovers)
 * - 7-8: Advanced builds (engine internals, ECU tuning)
 * - 9-10: Expert only (engine swaps, custom fabrication)
 */
USTRUCT(BlueprintType)
struct FMGPartSpecializationInfo
{
	GENERATED_BODY()

	/** Part category - determines which mechanic specialization is needed */
	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	EMGPartCategory Category = EMGPartCategory::Engine;

	/** Sub-category for more specific matching (e.g., "Intake", "Turbo") */
	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	FString SubCategory;

	/** Minimum mechanic skill level required (1=novice, 10=master) */
	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	int32 RequiredSkillLevel = 1;

	/** How long installation takes in minutes */
	UPROPERTY(BlueprintReadOnly, Category = "Specialization")
	float InstallTime = 60.0f;

	/** Did the lookup succeed? Always check this first! */
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
	// ========== Internal Cache Management ==========

	/**
	 * Build internal lookup caches from DataTable.
	 *
	 * This method creates three caches for different access patterns:
	 * 1. PartCache: Direct lookup by PartID
	 * 2. PartsByCategory: Fast filtering by category
	 * 3. PartsByVehicle: Fast compatibility lookups
	 *
	 * Called during Initialize() and ReloadCatalog().
	 */
	void BuildCache();

	/** Clear all internal caches and reset state */
	void ClearCache();

	// ========== Configuration ==========

	/**
	 * Reference to parts catalog DataTable asset.
	 *
	 * This is a soft reference - the DataTable is loaded on-demand
	 * rather than at subsystem creation. Set this in the editor or config.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> PartsCatalogTableRef;

	/** Loaded DataTable pointer (runtime only, not saved) */
	UPROPERTY(Transient)
	UDataTable* PartsCatalogTable = nullptr;

	// ========== Cached Data ==========
	// Multiple caches optimized for different query patterns

	/**
	 * Primary cache: Part data indexed by PartID.
	 *
	 * O(1) lookup when you know the exact part you want.
	 * Example: PartCache["TURBO_BIG"] returns the full part row.
	 */
	UPROPERTY(Transient)
	TMap<FName, FMGPartCatalogRow> PartCache;

	/**
	 * Secondary cache: Parts grouped by category.
	 *
	 * O(1) to get all parts in a category (then iterate the array).
	 * Used by shop UI for tab filtering and mechanic specialization checks.
	 * Example: PartsByCategory[EMGPartCategory::Engine] returns all engine parts.
	 */
	TMap<EMGPartCategory, TArray<FName>> PartsByCategory;

	/**
	 * Tertiary cache: Vehicle-specific parts indexed by VehicleID.
	 *
	 * For fast "what parts fit this car?" queries.
	 * Only contains vehicle-specific parts; universal parts are not in this cache.
	 * Example: PartsByVehicle["KAZE_CIVIC"] returns Civic-specific parts only.
	 */
	TMap<FName, TArray<FName>> PartsByVehicle;

	/** Flag indicating if caches are populated and valid */
	bool bCacheBuilt = false;

private:
	/**
	 * Default pricing info returned when part not found.
	 * Static to avoid allocating new structs for failed lookups.
	 */
	static const FMGPartPricingInfo InvalidPricingInfo;

	/**
	 * Default specialization info returned when part not found.
	 * Static to avoid allocating new structs for failed lookups.
	 */
	static const FMGPartSpecializationInfo InvalidSpecializationInfo;
};

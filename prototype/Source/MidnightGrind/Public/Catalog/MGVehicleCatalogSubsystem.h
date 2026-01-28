// Copyright Midnight Grind. All Rights Reserved.
// Iteration 81: Vehicle Catalog Subsystem - Runtime lookups for vehicle data

/**
 * =============================================================================
 * MGVehicleCatalogSubsystem.h - Vehicle Data Access Layer
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This subsystem provides centralized, efficient access to vehicle catalog data
 * at runtime. It acts as the "database" for all vehicle information in the game,
 * loading data from DataTable assets and caching it for fast lookups.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. GAME INSTANCE SUBSYSTEMS:
 *    In Unreal Engine, subsystems are singleton-like objects that live for the
 *    duration of their "outer" object. A GameInstanceSubsystem exists for the
 *    entire game session (from launch to quit), persisting across level loads.
 *
 *    Why use a subsystem instead of a static class or singleton?
 *    - Automatic lifecycle management (Unreal creates/destroys it)
 *    - Easy access from anywhere: GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>()
 *    - Blueprint-accessible without extra work
 *    - Proper integration with Unreal's garbage collection
 *
 * 2. CATALOG VS. INVENTORY:
 *    The CATALOG contains all possible vehicles (master data).
 *    The INVENTORY contains vehicles the player actually owns (instance data).
 *
 *    Catalog: "A Nissan Skyline R34 costs $80,000 and has 280HP stock"
 *    Inventory: "Player owns 2 Skylines, one is blue with 500HP mods"
 *
 * 3. SOFT OBJECT REFERENCES:
 *    VehicleCatalogTableRef uses TSoftObjectPtr which allows referencing assets
 *    without loading them immediately. The DataTable only loads when needed.
 *
 * 4. CACHING:
 *    After loading the DataTable, data is cached in VehicleCache (TMap) for O(1)
 *    lookups by VehicleID. This avoids repeated DataTable searches.
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 *
 *    [DataTable Asset]
 *           |
 *           v
 *    [MGVehicleCatalogSubsystem] <--- Cache for fast lookups
 *           |
 *           +---> [Shop/Dealership] - Shows available vehicles and prices
 *           |
 *           +---> [Garage UI] - Displays vehicle specs and max potential
 *           |
 *           +---> [Race System] - Gets vehicle class for matchmaking
 *           |
 *           +---> [Economy System] - Calculates purchase/sell prices
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * // Get subsystem reference (cached for reuse is best)
 * UMGVehicleCatalogSubsystem* Catalog = GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>();
 *
 * // Check if vehicle exists before using
 * if (Catalog->VehicleExists(VehicleID))
 * {
 *     FMGVehiclePricingInfo Pricing = Catalog->GetVehiclePricing(VehicleID);
 *     // Use pricing data...
 * }
 *
 * // Get vehicles for shop display
 * TArray<FMGVehicleCatalogRow> JDMCars = Catalog->GetVehiclesByCategory(EMGVehicleCategory::JDM);
 *
 * RELATED SYSTEMS:
 * ----------------
 * - MGPartsCatalogSubsystem: Same pattern but for parts data
 * - MGInventorySubsystem: Tracks which vehicles the player owns
 * - MGShopSubsystem: Uses this catalog for purchase workflows
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Catalog/MGCatalogTypes.h"
#include "MGVehicleCatalogSubsystem.generated.h"

// Forward declaration - avoids including the full DataTable header here
class UDataTable;

/**
 * Vehicle Catalog Subsystem
 *
 * Provides runtime access to vehicle catalog data from DataTables.
 * Used by economy systems for pricing lookups, by shops for vehicle
 * display, and by various subsystems for vehicle metadata.
 *
 * DataTable Setup:
 * - Create DataTable asset with row struct: FMGVehicleCatalogRow
 * - Set VehicleCatalogTable property to reference the DataTable
 * - Rows use VehicleID as the row name (FName key)
 *
 * Usage:
 * @code
 * UMGVehicleCatalogSubsystem* Catalog = GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>();
 * FMGVehiclePricingInfo Pricing = Catalog->GetVehiclePricing(FName("KAZE_CIVIC"));
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleCatalogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== Pricing Lookups ==========

	/**
	 * Get vehicle pricing information for economy calculations
	 * @param VehicleID Vehicle identifier (e.g. "KAZE_CIVIC")
	 * @return Pricing info struct with bIsValid set to false if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog|Pricing")
	FMGVehiclePricingInfo GetVehiclePricing(FName VehicleID) const;

	/**
	 * Get estimated market value for a vehicle based on condition
	 * @param VehicleID Vehicle identifier
	 * @param Condition Vehicle condition (0.0 - 1.0)
	 * @return Estimated street value adjusted for condition
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog|Pricing")
	int32 GetEstimatedValue(FName VehicleID, float Condition = 1.0f) const;

	/**
	 * Get base purchase price for a vehicle model
	 * @param VehicleID Vehicle identifier
	 * @return Base MSRP price, or -1 if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog|Pricing")
	int32 GetBasePurchasePrice(FName VehicleID) const;

	// ========== Data Lookups ==========

	/**
	 * Get full vehicle catalog data
	 * @param VehicleID Vehicle identifier
	 * @param OutData Output parameter for vehicle data
	 * @return True if vehicle found
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	bool GetVehicleData(FName VehicleID, FMGVehicleCatalogRow& OutData) const;

	/**
	 * Check if a vehicle exists in the catalog
	 * @param VehicleID Vehicle identifier
	 * @return True if vehicle is cataloged
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	bool VehicleExists(FName VehicleID) const;

	/**
	 * Get display name for a vehicle
	 * @param VehicleID Vehicle identifier
	 * @return Display name, or "Unknown" if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	FText GetVehicleDisplayName(FName VehicleID) const;

	// ========== Filtering ==========

	/**
	 * Get all vehicles in a specific performance class
	 * @param PerformanceClass Target class (D, C, B, A, S, X)
	 * @return Array of matching vehicle rows
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Catalog|Filtering")
	TArray<FMGVehicleCatalogRow> GetVehiclesByClass(EMGPerformanceClass PerformanceClass) const;

	/**
	 * Get all vehicles in a specific category
	 * @param Category Vehicle category (JDM, American, etc.)
	 * @return Array of matching vehicle rows
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Catalog|Filtering")
	TArray<FMGVehicleCatalogRow> GetVehiclesByCategory(EMGVehicleCategory Category) const;

	/**
	 * Get all vehicles within a price range
	 * @param MinPrice Minimum base purchase price
	 * @param MaxPrice Maximum base purchase price
	 * @return Array of matching vehicle rows
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Catalog|Filtering")
	TArray<FMGVehicleCatalogRow> GetVehiclesInPriceRange(int32 MinPrice, int32 MaxPrice) const;

	/**
	 * Get all vehicles matching a tag
	 * @param Tag Tag to search for (e.g. "JDM", "VTEC", "Starter")
	 * @return Array of matching vehicle rows
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Catalog|Filtering")
	TArray<FMGVehicleCatalogRow> GetVehiclesByTag(const FString& Tag) const;

	/**
	 * Get all vehicle IDs in the catalog
	 * @return Array of all cataloged vehicle IDs
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	TArray<FName> GetAllVehicleIDs() const;

	/**
	 * Get count of vehicles in catalog
	 * @return Number of cataloged vehicles
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	int32 GetVehicleCount() const;

	// ========== Validation ==========

	/**
	 * Check if the catalog is properly loaded
	 * @return True if DataTable is valid and loaded
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	bool IsCatalogLoaded() const;

	/**
	 * Force reload of catalog data (for editor/debug use)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Catalog")
	void ReloadCatalog();

protected:
	// ========== Internal Cache Management ==========
	// These methods manage the internal data cache for performance

	/**
	 * Build internal lookup cache from DataTable.
	 *
	 * Called during Initialize() and ReloadCatalog(). Iterates through
	 * all DataTable rows and populates VehicleCache for O(1) lookups.
	 *
	 * PERFORMANCE NOTE: Building the cache is O(n) where n = vehicle count.
	 * After building, all lookups are O(1). This tradeoff is ideal for
	 * read-heavy workloads (which game catalogs are).
	 */
	void BuildCache();

	/**
	 * Clear internal caches.
	 *
	 * Called before rebuilding cache and during Deinitialize().
	 * Releases memory and resets bCacheBuilt flag.
	 */
	void ClearCache();

	// ========== Configuration ==========
	// Designer-configured properties set in editor/config

	/**
	 * Reference to vehicle catalog DataTable asset.
	 *
	 * TSoftObjectPtr means the asset path is stored, but the actual
	 * DataTable isn't loaded until we call LoadSynchronous() or similar.
	 * This prevents loading all vehicle data at startup if not needed.
	 *
	 * Set this in the subsystem's default object or via config.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> VehicleCatalogTableRef;

	/**
	 * Loaded DataTable pointer (runtime).
	 *
	 * UPROPERTY(Transient) means this won't be saved/serialized.
	 * It's populated at runtime from VehicleCatalogTableRef.
	 */
	UPROPERTY(Transient)
	UDataTable* VehicleCatalogTable = nullptr;

	// ========== Cached Data ==========
	// Runtime caches populated from DataTable for fast access

	/**
	 * Cached vehicle data for fast lookups.
	 *
	 * TMap provides O(1) average-case lookup by FName key.
	 * Key: VehicleID (e.g., "KAZE_CIVIC")
	 * Value: Complete vehicle row data
	 *
	 * This cache duplicates DataTable data in a more accessible format.
	 */
	UPROPERTY(Transient)
	TMap<FName, FMGVehicleCatalogRow> VehicleCache;

	/**
	 * Flag indicating if cache is built.
	 *
	 * Used to avoid redundant cache builds and to detect uninitialized state.
	 * Public methods check this and may trigger lazy initialization.
	 */
	bool bCacheBuilt = false;

private:
	/**
	 * Default pricing info returned when vehicle not found.
	 *
	 * Static const avoids creating a new struct for each failed lookup.
	 * Callers check bIsValid field to detect lookup failure.
	 */
	static const FMGVehiclePricingInfo InvalidPricingInfo;
};

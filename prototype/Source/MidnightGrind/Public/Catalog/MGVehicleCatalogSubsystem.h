// Copyright Midnight Grind. All Rights Reserved.
// Iteration 81: Vehicle Catalog Subsystem - Runtime lookups for vehicle data

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Catalog/MGCatalogTypes.h"
#include "MGVehicleCatalogSubsystem.generated.h"

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
	/** Build internal lookup cache from DataTable */
	void BuildCache();

	/** Clear internal caches */
	void ClearCache();

	// ========== Configuration ==========

	/** Reference to vehicle catalog DataTable asset */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> VehicleCatalogTableRef;

	/** Loaded DataTable pointer */
	UPROPERTY(Transient)
	UDataTable* VehicleCatalogTable = nullptr;

	// ========== Cached Data ==========

	/** Cached vehicle data for fast lookups */
	UPROPERTY(Transient)
	TMap<FName, FMGVehicleCatalogRow> VehicleCache;

	/** Flag indicating if cache is built */
	bool bCacheBuilt = false;

private:
	/** Default pricing info returned when vehicle not found */
	static const FMGVehiclePricingInfo InvalidPricingInfo;
};

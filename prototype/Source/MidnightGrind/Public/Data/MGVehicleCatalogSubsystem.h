// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "MGVehicleCatalogSubsystem.generated.h"

/**
 * Vehicle pricing information
 * Extracted from catalog for easy access
 */
USTRUCT(BlueprintType)
struct FMGVehiclePricingInfo
{
	GENERATED_BODY()

	/** Base purchase price (MSRP) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BasePurchasePrice = 25000;

	/** Street value (used condition) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StreetValue = 30000;

	/** Legendary value (pristine/rare) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 LegendaryValue = 60000;

	/** Maintenance cost multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaintenanceCostMultiplier = 1.0f;

	/** Parts price multiplier for this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PartsPriceMultiplier = 1.0f;

	/** Insurance class (affects insurance costs) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString InsuranceClass = "C";
};

/**
 * Vehicle performance information
 */
USTRUCT(BlueprintType)
struct FMGVehiclePerformanceInfo
{
	GENERATED_BODY()

	/** Base Performance Index */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BasePI = 420;

	/** Performance class (D, C, B, A, S, X) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString PerformanceClass = "C";

	/** Maximum PI potential with upgrades */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxPIPotential = 750;

	/** Base horsepower */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BaseHorsepower = 200;

	/** Base torque (lb-ft) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BaseTorque = 150;

	/** Base weight (lbs) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BaseWeight = 2800;

	/** Drivetrain type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Drivetrain = "RWD";
};

/**
 * DataTable row for vehicle catalog
 * Matches JSON schema from Content/Data/Vehicles/*.json
 */
USTRUCT(BlueprintType)
struct FMGVehicleCatalogRow : public FTableRowBase
{
	GENERATED_BODY()

	// ==========================================
	// IDENTITY
	// ==========================================

	/** Unique vehicle identifier (e.g. "KAZE_CIVIC") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString VehicleID;

	/** Display name (e.g. "Kaze Civic") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString DisplayName;

	/** Manufacturer name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Manufacturer;

	/** Model year */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	int32 Year = 1999;

	/** Vehicle class/category (JDM, Muscle, Euro, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Category;

	/** Vehicle country of origin */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Country;

	/** Description text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Description;

	// ==========================================
	// ECONOMY
	// ==========================================

	/** Pricing information */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	FMGVehiclePricingInfo Pricing;

	// ==========================================
	// PERFORMANCE
	// ==========================================

	/** Performance information */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	FMGVehiclePerformanceInfo Performance;

	// ==========================================
	// REQUIREMENTS
	// ==========================================

	/** Required reputation tier to purchase */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	FString RequiredREPTier = "UNKNOWN";

	/** Required player level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	int32 RequiredLevel = 1;

	/** Special unlock conditions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	TArray<FString> SpecialConditions;

	// ==========================================
	// TAGS
	// ==========================================

	/** Search/filter tags */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	TArray<FString> Tags;
};

/**
 * Vehicle Catalog Subsystem
 *
 * Provides access to vehicle catalog data loaded from DataTables.
 * Used by economy systems to lookup vehicle pricing, specs, and metadata.
 *
 * Data Source: Content/Data/Vehicles/*.json -> Imported to DataTable
 *
 * Usage:
 *   if (UMGVehicleCatalogSubsystem* Catalog = GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>())
 *   {
 *       FMGVehiclePricingInfo Pricing = Catalog->GetVehiclePricing(FName("KAZE_CIVIC"));
 *       int32 Price = Pricing.BasePurchasePrice;
 *   }
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleCatalogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// SUBSYSTEM LIFECYCLE
	// ==========================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// PRICING LOOKUPS
	// ==========================================

	/**
	 * Get vehicle pricing information
	 * @param VehicleID Vehicle identifier (e.g. "KAZE_CIVIC")
	 * @return Pricing info, or default values if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	FMGVehiclePricingInfo GetVehiclePricing(FName VehicleID) const;

	/**
	 * Get base purchase price
	 * @param VehicleID Vehicle identifier
	 * @return Base MSRP price, or 0 if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	int32 GetBasePurchasePrice(FName VehicleID) const;

	/**
	 * Get street value (used condition price)
	 * @param VehicleID Vehicle identifier
	 * @return Street value, or 0 if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	int32 GetStreetValue(FName VehicleID) const;

	// ==========================================
	// PERFORMANCE LOOKUPS
	// ==========================================

	/**
	 * Get vehicle performance information
	 * @param VehicleID Vehicle identifier
	 * @return Performance info, or defaults if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	FMGVehiclePerformanceInfo GetVehiclePerformance(FName VehicleID) const;

	/**
	 * Get performance class (D, C, B, A, S, X)
	 * @param VehicleID Vehicle identifier
	 * @return Performance class string
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	FString GetPerformanceClass(FName VehicleID) const;

	// ==========================================
	// CATALOG QUERIES
	// ==========================================

	/**
	 * Get full vehicle catalog entry
	 * @param VehicleID Vehicle identifier
	 * @param OutData Output catalog row data
	 * @return True if vehicle found in catalog
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	bool GetVehicleData(FName VehicleID, FMGVehicleCatalogRow& OutData) const;

	/**
	 * Get all vehicles in a performance class
	 * @param PerformanceClass Class letter (D, C, B, A, S, X)
	 * @return Array of vehicles in that class
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	TArray<FMGVehicleCatalogRow> GetVehiclesByClass(const FString& PerformanceClass) const;

	/**
	 * Get all vehicles with a specific tag
	 * @param Tag Tag to search for (e.g. "JDM", "Muscle", "Starter")
	 * @return Array of matching vehicles
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	TArray<FMGVehicleCatalogRow> GetVehiclesByTag(const FString& Tag) const;

	/**
	 * Check if vehicle exists in catalog
	 * @param VehicleID Vehicle identifier
	 * @return True if vehicle is in catalog
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	bool IsVehicleInCatalog(FName VehicleID) const;

	/**
	 * Get all vehicle IDs in catalog
	 * @return Array of all vehicle identifiers
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
	TArray<FName> GetAllVehicleIDs() const;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * Reference to vehicle catalog DataTable
	 * Set this in DefaultEngine.ini or Blueprint
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSoftObjectPtr<UDataTable> VehicleCatalogTable;

	// ==========================================
	// INTERNAL DATA
	// ==========================================

	/** Cached catalog rows for fast lookup */
	TMap<FName, FMGVehicleCatalogRow*> VehicleCache;

	/** Loaded DataTable reference */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> LoadedCatalogTable;

	// ==========================================
	// INTERNAL HELPERS
	// ==========================================

	/** Build cache from DataTable */
	void BuildCache();

	/** Get catalog row pointer (internal use) */
	const FMGVehicleCatalogRow* GetCatalogRow(FName VehicleID) const;
};

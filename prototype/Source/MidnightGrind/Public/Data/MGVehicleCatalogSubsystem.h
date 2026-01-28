// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehicleCatalogSubsystem.h
 * @brief Vehicle Catalog Data Access System - Central Repository for Vehicle Information
 *
 * @section overview Overview for New Developers
 *
 * This file defines the Vehicle Catalog Subsystem, which serves as the central
 * database for all vehicle information in Midnight Grind. Think of it as the
 * "vehicle encyclopedia" that other systems query to get vehicle data.
 *
 * The catalog contains:
 * - Vehicle identity (names, manufacturers, model years)
 * - Pricing information (purchase prices, street values, insurance classes)
 * - Performance specifications (horsepower, torque, weight, PI ratings)
 * - Unlock requirements (reputation tier, player level, special conditions)
 *
 * @section why Why Does This System Exist?
 *
 * In a racing game, many different systems need vehicle data:
 * - Dealership: "How much does this car cost?"
 * - Garage: "What are this car's base stats?"
 * - Insurance: "What insurance class is this vehicle?"
 * - Pink Slips: "What's this car worth for betting?"
 * - Matchmaking: "What performance class is this vehicle?"
 *
 * Instead of duplicating this data everywhere, we centralize it in DataTables
 * and provide this subsystem as a clean interface to access it.
 *
 * @section concepts Key Concepts
 *
 * 1. DATA TABLES:
 *    - Vehicle data is stored in Unreal Engine DataTables
 *    - DataTables can be edited in the Unreal Editor or imported from JSON
 *    - Data path: Content/Data/Vehicles/*.json
 *    - The subsystem loads and caches this data for fast access
 *
 * 2. VEHICLE PRICING (FMGVehiclePricingInfo):
 *    - BasePurchasePrice: MSRP - the "new" price at dealerships
 *    - StreetValue: Used condition price - what you'd pay/receive in player trades
 *    - LegendaryValue: Pristine/rare condition - collectors pay premium
 *    - MaintenanceCostMultiplier: Affects repair costs (luxury cars cost more)
 *    - PartsPriceMultiplier: Affects upgrade costs (exotic parts are pricier)
 *    - InsuranceClass: A-F rating affecting insurance premiums
 *
 * 3. VEHICLE PERFORMANCE (FMGVehiclePerformanceInfo):
 *    - BasePI: Performance Index - single number rating (like Forza's PI)
 *    - PerformanceClass: Letter grade (D, C, B, A, S, X) derived from PI
 *    - MaxPIPotential: Highest PI achievable with full upgrades
 *    - BaseHorsepower/Torque/Weight: Raw specs for physics calculations
 *    - Drivetrain: FWD, RWD, or AWD
 *
 * 4. VEHICLE CATALOG ROW (FMGVehicleCatalogRow):
 *    The complete vehicle entry containing:
 *    - Identity: VehicleID, DisplayName, Manufacturer, Year, Category
 *    - Economy: Pricing struct with all price data
 *    - Performance: Performance struct with all specs
 *    - Requirements: What player needs to purchase (level, reputation)
 *    - Tags: Search/filter metadata (JDM, Muscle, Starter, etc.)
 *
 * 5. PERFORMANCE CLASSES:
 *    - D Class: PI 100-399 (beginner cars)
 *    - C Class: PI 400-499 (budget sports cars)
 *    - B Class: PI 500-599 (sports cars)
 *    - A Class: PI 600-699 (high-end sports)
 *    - S Class: PI 700-799 (supercars)
 *    - X Class: PI 800-999 (hypercars)
 *
 * 6. CACHING:
 *    - On Initialize(), the subsystem builds a cache from the DataTable
 *    - Lookups are O(1) via TMap instead of scanning the table each time
 *    - Cache is rebuilt if DataTable is modified
 *
 * @section usage Usage Examples
 *
 * @code
 * // Get the subsystem
 * UMGVehicleCatalogSubsystem* Catalog = GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>();
 *
 * // Example 1: Get vehicle purchase price for dealership
 * int32 Price = Catalog->GetBasePurchasePrice(FName("KAZE_CIVIC"));
 * // Returns: 15000 (or whatever is configured)
 *
 * // Example 2: Get full pricing info for insurance calculations
 * FMGVehiclePricingInfo Pricing = Catalog->GetVehiclePricing(FName("RYUSEI_SKYLINE"));
 * int32 InsurableValue = Pricing.StreetValue;
 * FString InsuranceClass = Pricing.InsuranceClass; // "B"
 *
 * // Example 3: Get performance class for matchmaking
 * FString PerfClass = Catalog->GetPerformanceClass(FName("FENRIR_M3"));
 * // Returns: "A" (for A-class cars)
 *
 * // Example 4: Get all JDM cars for a filter menu
 * TArray<FMGVehicleCatalogRow> JDMCars = Catalog->GetVehiclesByTag("JDM");
 * for (const FMGVehicleCatalogRow& Car : JDMCars)
 * {
 *     UE_LOG(LogTemp, Log, TEXT("JDM Car: %s"), *Car.DisplayName);
 * }
 *
 * // Example 5: Get all B-class vehicles for a restricted race
 * TArray<FMGVehicleCatalogRow> BClassCars = Catalog->GetVehiclesByClass("B");
 *
 * // Example 6: Check if a vehicle exists before using it
 * if (Catalog->IsVehicleInCatalog(FName("CUSTOM_VEHICLE")))
 * {
 *     // Safe to proceed
 * }
 *
 * // Example 7: Get full vehicle data for detailed display
 * FMGVehicleCatalogRow VehicleData;
 * if (Catalog->GetVehicleData(FName("HAYABUSA_RX7"), VehicleData))
 * {
 *     UE_LOG(LogTemp, Log, TEXT("Vehicle: %s by %s (%d)"),
 *         *VehicleData.DisplayName,
 *         *VehicleData.Manufacturer,
 *         VehicleData.Year);
 *     UE_LOG(LogTemp, Log, TEXT("  Base PI: %d, Max PI: %d"),
 *         VehicleData.Performance.BasePI,
 *         VehicleData.Performance.MaxPIPotential);
 * }
 * @endcode
 *
 * @section blueprint Blueprint Usage
 *
 * All functions are marked BlueprintPure and can be called from Blueprints:
 * 1. Get a reference to the subsystem using "Get Game Instance Subsystem"
 * 2. Select UMGVehicleCatalogSubsystem as the class
 * 3. Call any of the lookup functions
 *
 * @section setup Setup Instructions
 *
 * 1. Create vehicle data JSON files in Content/Data/Vehicles/
 * 2. Import JSON files as DataTable assets with FMGVehicleCatalogRow row type
 * 3. Set VehicleCatalogTable reference in DefaultEngine.ini or Blueprint
 * 4. The subsystem will automatically load on game start
 *
 * @section related Related Files
 *
 * - MGVehicleCatalogSubsystem.cpp - Implementation
 * - MGGarageSubsystem.h - Uses catalog for vehicle management
 * - MGDealershipSubsystem.h - Uses catalog for vehicle purchases
 * - MGInsuranceSubsystem.h - Uses catalog for insurance calculations
 * - MGPinkSlipSubsystem.h - Uses catalog for vehicle valuations
 * - Content/Data/Vehicles/*.json - Source vehicle data
 *
 * @see FMGVehicleCatalogRow for the complete vehicle data structure
 * @see FMGVehiclePricingInfo for pricing breakdown
 * @see FMGVehiclePerformanceInfo for performance specifications
 */

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

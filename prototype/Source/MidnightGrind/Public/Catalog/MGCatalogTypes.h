// Copyright Midnight Grind. All Rights Reserved.
// Iteration 81: Vehicle and Parts Catalog DataTable row definitions

/**
 * =============================================================================
 * MGCatalogTypes.h - Core Data Types for Vehicle and Parts Catalog System
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the foundational data structures (structs) and enumerations
 * (enums) used throughout the Midnight Grind vehicle customization and upgrade
 * system. Think of this as the "dictionary" that defines how vehicle and parts
 * data is organized and stored.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. DATA TABLES:
 *    Unreal Engine DataTables are spreadsheet-like assets that store rows of
 *    structured data. Each row follows a "row struct" definition. The structs
 *    in this file (FMGVehicleCatalogRow, FMGPartCatalogRow) define what columns
 *    exist in our vehicle and parts DataTables.
 *
 *    Example: A DataTable using FMGVehicleCatalogRow might have rows like:
 *    | VehicleID    | DisplayName      | Year | Category | BaseStats.Power |
 *    |--------------|------------------|------|----------|-----------------|
 *    | KAZE_CIVIC   | "Kaze Civic"     | 1999 | JDM      | 160             |
 *    | VENOM_CAMARO | "Venom Camaro"   | 2020 | American | 455             |
 *
 * 2. ENUMERATIONS (UENUM):
 *    Enums define a fixed set of named options. They're type-safe and show up
 *    nicely in the Unreal Editor as dropdown menus.
 *
 *    Example: EMGPerformanceClass has values D, C, B, A, S, X - used to
 *    categorize vehicles by their power level for matchmaking and race classes.
 *
 * 3. STRUCTURES (USTRUCT):
 *    Structs group related data together. They can be embedded within other
 *    structs to create organized, hierarchical data. All structs here use
 *    GENERATED_BODY() which enables Unreal's reflection system.
 *
 *    Example: FMGVehicleCatalogRow contains FMGVehicleBaseStats, which itself
 *    contains Power, Torque, Weight, etc. This keeps data organized.
 *
 * 4. UPROPERTY SPECIFIERS:
 *    - EditAnywhere: Value can be changed in the editor
 *    - BlueprintReadOnly: Blueprints can read but not modify the value
 *    - Category: Groups properties in the editor's Details panel
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 *
 *    [JSON Files] --> [DataTable Assets] --> [Catalog Subsystems] --> [Game Logic]
 *         ^                   ^                       ^
 *         |                   |                       |
 *    Designer edits      Uses these types        Provides runtime
 *    vehicle specs       as row format           lookups for pricing,
 *                                                specs, compatibility
 *
 * The types defined here are used by:
 * - MGVehicleCatalogSubsystem: Looks up vehicle pricing and specifications
 * - MGPartsCatalogSubsystem: Looks up part pricing and mechanic requirements
 * - MGInventorySubsystem: Stores owned vehicles/parts
 * - Shop/Dealership UI: Displays vehicle and part information to players
 *
 * FILE SECTIONS:
 * --------------
 * 1. Vehicle Enums: Performance classes, categories, drivetrains
 * 2. Vehicle Structs: Base stats, economy, performance index, unlock requirements
 * 3. Vehicle Catalog Row: Complete vehicle definition for DataTable
 * 4. Part Enums: Tiers and categories
 * 5. Part Catalog Row: Complete part definition for DataTable
 * 6. Simplified Info Structs: Lightweight structs for quick lookups
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MGCatalogTypes.generated.h"

// =============================================================================
// VEHICLE ENUMERATIONS
// These enums categorize vehicles by their capabilities and origins
// =============================================================================

/**
 * Performance class tiers for vehicles.
 *
 * Performance classes are used for:
 * - Race matchmaking: Players compete against vehicles in the same class
 * - Race restrictions: Some events only allow certain classes
 * - Progression gating: Higher classes unlock as players advance
 * - Upgrade targets: Players can upgrade a D-class car into S-class
 *
 * The classes roughly correspond to real-world vehicle tiers:
 * - D: Economy cars, compacts (Honda Fit, Toyota Corolla)
 * - C: Sport compacts, muscle car base models (Civic Si, Mustang V6)
 * - B: Hot hatches, mid-tier sports cars (Golf GTI, Miata)
 * - A: Performance variants, Euro sports (BMW M3, Porsche Cayman)
 * - S: Supercars, heavily modified builds (GT-R, 911 Turbo)
 * - X: Hypercars, record-breaking builds (LaFerrari, 2000hp builds)
 */
UENUM(BlueprintType)
enum class EMGPerformanceClass : uint8
{
	D UMETA(DisplayName = "D - Entry Level"),
	C UMETA(DisplayName = "C - Street"),
	B UMETA(DisplayName = "B - Sport"),
	A UMETA(DisplayName = "A - Performance"),
	S UMETA(DisplayName = "S - Super"),
	X UMETA(DisplayName = "X - Hyper")
};

/**
 * Vehicle category for filtering and organization
 */
UENUM(BlueprintType)
enum class EMGVehicleCategory : uint8
{
	JDM,
	American,
	European,
	Korean,
	Exotic
};

/**
 * Drivetrain configuration
 */
UENUM(BlueprintType)
enum class EMGDrivetrain : uint8
{
	FWD UMETA(DisplayName = "Front-Wheel Drive"),
	RWD UMETA(DisplayName = "Rear-Wheel Drive"),
	AWD UMETA(DisplayName = "All-Wheel Drive")
};

/**
 * Base vehicle stats from catalog
 * Embedded struct for vehicle specifications
 */
USTRUCT(BlueprintType)
struct FMGVehicleBaseStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Power = 200;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Torque = 200;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Weight = 2800;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 WeightDistributionFront = 55;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	EMGDrivetrain Drivetrain = EMGDrivetrain::RWD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Displacement = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Redline = 7000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float TopSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float Acceleration0to60 = 6.0f;
};

/**
 * Vehicle economy data from catalog
 * Embedded struct for pricing information
 */
USTRUCT(BlueprintType)
struct FMGVehicleEconomy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 BasePurchasePrice = 25000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 StreetValue = 30000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 LegendaryValue = 60000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float MaintenanceCostMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float PartsPriceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	FString InsuranceClass = TEXT("C");
};

/**
 * Vehicle performance index data
 */
USTRUCT(BlueprintType)
struct FMGVehiclePerformanceIndex
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Base = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 MaxPotential = 800;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	EMGPerformanceClass Class = EMGPerformanceClass::C;
};

/**
 * Vehicle unlock requirements
 */
USTRUCT(BlueprintType)
struct FMGVehicleUnlockRequirements
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlocks")
	FString RequiredREPTier = TEXT("UNKNOWN");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlocks")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlocks")
	TArray<FString> SpecialConditions;
};

/**
 * DataTable row for vehicle catalog
 * Matches JSON schema from Content/Data/Vehicles/*.json
 * Used for runtime lookups of vehicle pricing, specs, and metadata
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGVehicleCatalogRow : public FTableRowBase
{
	GENERATED_BODY()

	// === Identity ===

	/** Unique vehicle identifier (e.g. "KAZE_CIVIC") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName VehicleID;

	/** Display name shown to players */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Vehicle manufacturer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Manufacturer;

	/** Model year */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	int32 Year = 1999;

	/** Country of origin */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Country;

	/** Vehicle category for filtering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGVehicleCategory Category = EMGVehicleCategory::JDM;

	/** Vehicle description text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	// === Performance ===

	/** Base vehicle performance statistics */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	FMGVehicleBaseStats BaseStats;

	/** Performance index information */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	FMGVehiclePerformanceIndex PerformanceIndex;

	// === Economy ===

	/** Vehicle economy and pricing data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	FMGVehicleEconomy Economy;

	// === Unlocks ===

	/** Requirements to unlock/purchase this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlocks")
	FMGVehicleUnlockRequirements Unlocks;

	// === Max Build Stats ===

	/** Maximum power achievable with full upgrades */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaxBuild")
	int32 MaxPower = 500;

	/** Maximum torque achievable with full upgrades */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaxBuild")
	int32 MaxTorque = 400;

	/** Minimum weight achievable with weight reduction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaxBuild")
	int32 MinWeight = 2400;

	/** Maximum PI achievable with optimal build */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaxBuild")
	int32 MaxPI = 800;

	// === Tags ===

	/** Searchable tags for filtering (JDM, FWD, VTEC, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	TArray<FString> Tags;

	// === Asset References ===

	/** Engine audio profile name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	FName EngineAudioProfile;

	/** Soft reference to vehicle blueprint class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TSoftClassPtr<AActor> VehicleBlueprintClass;
};

/**
 * Part upgrade tier levels
 */
UENUM(BlueprintType)
enum class EMGPartTier : uint8
{
	Stock UMETA(DisplayName = "Stock"),
	Street UMETA(DisplayName = "Street"),
	Sport UMETA(DisplayName = "Sport"),
	Race UMETA(DisplayName = "Race"),
	Elite UMETA(DisplayName = "Elite"),
	Legendary UMETA(DisplayName = "Legendary")
};

/**
 * Part category for organization and mechanic specialization
 */
UENUM(BlueprintType)
enum class EMGPartCategory : uint8
{
	Engine,
	Drivetrain,
	Suspension,
	Brakes,
	Wheels,
	Tires,
	Aero,
	Body,
	Interior,
	Nitrous,
	Electronics
};

/**
 * DataTable row for parts catalog
 * Used for pricing lookups and mechanic specialization
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGPartCatalogRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique part identifier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName PartID;

	/** Display name shown to players */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Part description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	/** Part category for filtering and specialization */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EMGPartCategory Category = EMGPartCategory::Engine;

	/** Sub-category within main category (e.g. "Intake", "Exhaust") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	FString SubCategory;

	/** Part upgrade tier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EMGPartTier Tier = EMGPartTier::Street;

	/** Base price before vehicle multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 BasePrice = 1000;

	/** Labor cost for installation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 LaborCost = 100;

	/** Installation time in minutes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float InstallTime = 60.0f;

	/** Required mechanic skill level (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	int32 RequiredSkillLevel = 1;

	/** Compatible vehicle IDs (empty = universal) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> CompatibleVehicles;

	/** Required parts that must be installed first */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> RequiredParts;

	/** Parts that conflict with this one */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> ConflictingParts;

	/** Performance stat modifiers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	int32 PowerBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	int32 TorqueBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	int32 WeightChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	int32 PIChange = 0;
};

/**
 * Simplified pricing info for quick lookups
 */
USTRUCT(BlueprintType)
struct FMGVehiclePricingInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	int32 BasePurchasePrice = 25000;

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	int32 StreetValue = 30000;

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	int32 LegendaryValue = 60000;

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	float MaintenanceCostMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	float PartsPriceMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Pricing")
	bool bIsValid = false;
};

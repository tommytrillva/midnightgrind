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
 * Vehicle category for filtering and organization.
 *
 * Categories group vehicles by their cultural/regional origin, which affects:
 * - Shop organization: Dealerships may specialize in certain categories
 * - Event eligibility: "JDM Night" events only allow JDM cars
 * - Part compatibility: Some aftermarket parts are category-specific
 * - Community culture: Players often identify with a category
 *
 * JDM = "Japanese Domestic Market" - cars originally sold in Japan
 * (Nissan Skyline, Toyota Supra, Honda NSX, Mazda RX-7)
 */
UENUM(BlueprintType)
enum class EMGVehicleCategory : uint8
{
	JDM,        // Japanese imports (Skyline, Supra, RX-7, NSX)
	American,   // US muscle and sports (Mustang, Camaro, Corvette)
	European,   // Euro performance (BMW M, Mercedes AMG, Porsche)
	Korean,     // Korean tuners (Hyundai N, Kia Stinger)
	Exotic      // Rare supercars (Ferrari, Lamborghini, McLaren)
};

/**
 * Drivetrain configuration - which wheels receive engine power.
 *
 * Drivetrain significantly affects vehicle handling characteristics:
 *
 * FWD (Front-Wheel Drive):
 * - Engine power goes to front wheels
 * - Tends to understeer (front pushes wide in corners)
 * - Better traction in rain due to weight over driven wheels
 * - Common in economy cars and hot hatches (Civic, Golf GTI)
 *
 * RWD (Rear-Wheel Drive):
 * - Engine power goes to rear wheels
 * - Can oversteer (rear slides out in corners) - enables drifting
 * - Classic sports car layout (Supra, Corvette, 911)
 * - Preferred by enthusiasts for balanced handling
 *
 * AWD (All-Wheel Drive):
 * - Power distributed to all four wheels
 * - Maximum traction for launches and wet conditions
 * - Heavier due to additional drivetrain components
 * - Examples: GT-R, Evo, STI, Audi Quattro
 *
 * Drivetrain can sometimes be SWAPPED during gameplay (e.g., RWD to AWD
 * conversion) as a major upgrade path.
 */
UENUM(BlueprintType)
enum class EMGDrivetrain : uint8
{
	FWD UMETA(DisplayName = "Front-Wheel Drive"),
	RWD UMETA(DisplayName = "Rear-Wheel Drive"),
	AWD UMETA(DisplayName = "All-Wheel Drive")
};

// =============================================================================
// VEHICLE STRUCTS
// These structs organize vehicle data into logical groups
// =============================================================================

/**
 * Base vehicle stats from catalog - the factory specifications.
 *
 * These are the STOCK (unmodified) specifications for a vehicle. When a
 * player installs upgrades, these values are used as the starting point
 * and modified by the parts system.
 *
 * TERMINOLOGY FOR NEW DEVELOPERS:
 *
 * Power (HP/Horsepower):
 *   Measures the engine's ability to do work. Higher = faster acceleration
 *   at high speeds. Typical range: 100 HP (economy) to 700+ HP (supercar)
 *
 * Torque (lb-ft):
 *   Measures rotational force. Higher = faster acceleration from standstill
 *   and better response. Diesel engines have high torque, sports cars balance both.
 *
 * Weight (lbs):
 *   Vehicle curb weight in pounds. Lighter = better acceleration and handling.
 *   Weight reduction is a key tuning strategy.
 *
 * Weight Distribution:
 *   Percentage of weight on front wheels. 50/50 is ideal for balanced handling.
 *   Front-engine cars are typically 55-60% front, mid-engine cars are ~45% front.
 *
 * Displacement (cc):
 *   Engine size in cubic centimeters. Larger engines typically make more power
 *   but are heavier. 2000cc = 2.0L engine.
 *
 * Redline (RPM):
 *   Maximum safe engine speed. Higher-revving engines (8000+ RPM) feel sportier.
 *   Rotary engines can rev to 9000+, diesel engines may only reach 4500.
 *
 * This struct is EMBEDDED in FMGVehicleCatalogRow, meaning each vehicle row
 * contains one of these stat blocks.
 */
USTRUCT(BlueprintType)
struct FMGVehicleBaseStats
{
	GENERATED_BODY()

	/** Engine horsepower (HP) - affects top-end acceleration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Power = 200;

	/** Engine torque in lb-ft - affects low-end acceleration and response */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Torque = 200;

	/** Curb weight in pounds - lighter = faster and more agile */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Weight = 2800;

	/** Percentage of weight over front wheels (50 = perfect balance) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 WeightDistributionFront = 55;

	/** Which wheels receive power (FWD/RWD/AWD) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	EMGDrivetrain Drivetrain = EMGDrivetrain::RWD;

	/** Engine displacement in cc (2000 = 2.0L) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Displacement = 2000;

	/** Maximum engine RPM before damage/rev limiter */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Redline = 7000;

	/** Maximum speed in MPH (stock, without modifications) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float TopSpeed = 150.0f;

	/** 0-60 MPH time in seconds (lower = faster) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float Acceleration0to60 = 6.0f;
};

/**
 * Vehicle economy data from catalog - pricing and cost multipliers.
 *
 * This struct controls the financial aspects of owning a vehicle.
 * Exotic cars cost more to buy, maintain, and upgrade than economy cars.
 *
 * ECONOMY SYSTEM OVERVIEW:
 *
 * BasePurchasePrice:
 *   What the player pays to buy this vehicle new from a dealership.
 *   This is the "MSRP" equivalent.
 *
 * StreetValue:
 *   What the vehicle sells for when in good condition with no mods.
 *   Used by the sell/trade system. Often lower than purchase price
 *   (depreciation) but can be higher for rare vehicles.
 *
 * LegendaryValue:
 *   Maximum potential value when fully upgraded and in perfect condition.
 *   Players can invest in their vehicles to increase their worth.
 *
 * MaintenanceCostMultiplier:
 *   Exotic cars have higher multipliers (2.0-3.0) meaning repairs,
 *   oil changes, and race damage cost more to fix. Economy cars ~1.0.
 *
 * PartsPriceMultiplier:
 *   How much more/less parts cost for this specific vehicle.
 *   European exotics might have 2.5x parts costs, while common
 *   Japanese cars have 0.8x (cheap, readily available parts).
 *
 * InsuranceClass:
 *   Determines insurance premium costs. A = highest, D = lowest.
 *   Affects ongoing ownership costs for the player.
 */
USTRUCT(BlueprintType)
struct FMGVehicleEconomy
{
	GENERATED_BODY()

	/** Dealership purchase price (new vehicle cost) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 BasePurchasePrice = 25000;

	/** Base resale value (stock, good condition) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 StreetValue = 30000;

	/** Maximum value when fully built and pristine */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 LegendaryValue = 60000;

	/** Multiplier for repair/maintenance costs (1.0 = normal, 2.0 = double) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float MaintenanceCostMultiplier = 1.0f;

	/** Multiplier for aftermarket parts prices (1.0 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float PartsPriceMultiplier = 1.0f;

	/** Insurance tier (A=expensive, D=cheap) affects ongoing costs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	FString InsuranceClass = TEXT("C");
};

/**
 * Vehicle performance index data - used for matchmaking and restrictions.
 *
 * PERFORMANCE INDEX (PI) EXPLAINED:
 *
 * The Performance Index is a single number (100-999) that represents a
 * vehicle's overall capability. Think of it like a "power level" that
 * combines HP, weight, handling, and other factors into one score.
 *
 * PI is used for:
 * - Fair matchmaking: Players race against similar PI vehicles
 * - Class restrictions: "PI 500 max" events create balanced competition
 * - Build guidance: Shows players how upgrades affect competitiveness
 *
 * The formula considers:
 * - Power-to-weight ratio (acceleration potential)
 * - Handling characteristics (cornering speed)
 * - Top speed (straight-line performance)
 * - Tire grip and braking capability
 *
 * Base PI is the stock vehicle's rating. MaxPotential shows how high
 * the PI can go with maximum upgrades (helps players plan builds).
 *
 * Class is derived from PI ranges:
 * - D: 100-399  (starter cars)
 * - C: 400-499  (street cars)
 * - B: 500-599  (sport cars)
 * - A: 600-699  (performance cars)
 * - S: 700-799  (supercars)
 * - X: 800-999  (hypercars, unlimited builds)
 */
USTRUCT(BlueprintType)
struct FMGVehiclePerformanceIndex
{
	GENERATED_BODY()

	/** Stock PI rating before any modifications */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 Base = 500;

	/** Maximum achievable PI with best possible upgrades */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 MaxPotential = 800;

	/** Performance class tier based on stock PI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	EMGPerformanceClass Class = EMGPerformanceClass::C;
};

/**
 * Vehicle unlock requirements - what the player needs to purchase this vehicle.
 *
 * PROGRESSION GATING:
 *
 * Not all vehicles are available from the start. Players must earn access
 * to better cars through progression. This creates goals and rewards.
 *
 * RequiredREPTier (Reputation Tier):
 *   REP is the player's street racing reputation. Higher REP unlocks access
 *   to exclusive dealerships and legendary vehicles. Tiers might be:
 *   UNKNOWN, ROOKIE, STREET, UNDERGROUND, LEGEND, ICON
 *
 * RequiredLevel:
 *   Player account level required. Separate from REP, this tracks overall
 *   playtime and experience. Level 1 = new player, Level 50+ = veteran.
 *
 * SpecialConditions:
 *   Additional requirements like:
 *   - "Complete Tokyo Drift storyline"
 *   - "Win 10 races in JDM vehicles"
 *   - "Own 5 different S-class cars"
 *   - "Seasonal event exclusive" (limited time availability)
 *
 * The shop/dealership UI checks these requirements and shows locked
 * vehicles with their unlock conditions.
 */
USTRUCT(BlueprintType)
struct FMGVehicleUnlockRequirements
{
	GENERATED_BODY()

	/** Reputation tier needed (ROOKIE, STREET, UNDERGROUND, LEGEND, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlocks")
	FString RequiredREPTier = TEXT("UNKNOWN");

	/** Minimum player level required to purchase */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlocks")
	int32 RequiredLevel = 1;

	/** Additional unlock conditions (story completion, achievements, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlocks")
	TArray<FString> SpecialConditions;
};

// =============================================================================
// VEHICLE CATALOG ROW
// This is the main struct that defines a complete vehicle in the game
// =============================================================================

/**
 * DataTable row for vehicle catalog - the complete vehicle definition.
 *
 * This struct inherits from FTableRowBase, which means it can be used as
 * the row type for Unreal Engine DataTable assets. Each row in the
 * DataTable represents one vehicle in the game.
 *
 * DATA PIPELINE:
 *
 * 1. Designers create/edit vehicle data in JSON files:
 *    Content/Data/Vehicles/JDM/KAZE_CIVIC.json
 *
 * 2. An import process converts JSON to DataTable rows using this struct
 *
 * 3. At runtime, MGVehicleCatalogSubsystem loads the DataTable and provides
 *    fast lookups by VehicleID
 *
 * 4. Game systems (shops, garage, races) query the subsystem to get vehicle info
 *
 * WHY USE DATATABLES?
 * - Designer-friendly: Non-programmers can edit in Unreal Editor
 * - Fast lookups: O(1) access by row name (VehicleID)
 * - Type-safe: Compiler catches mistakes in struct definitions
 * - Localization: FText fields support automatic translation
 *
 * EXTENDING THE CATALOG:
 * To add a new vehicle, create a new row in the DataTable with a unique
 * VehicleID. The subsystem will automatically include it in lookups.
 *
 * MIDNIGHTGRIND_API macro makes this struct accessible from other modules.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGVehicleCatalogRow : public FTableRowBase
{
	GENERATED_BODY()

	// === Identity ===
	// Basic information that identifies and describes the vehicle

	/**
	 * Unique vehicle identifier used as the DataTable row key.
	 * Convention: MANUFACTURER_MODEL (e.g., "KAZE_CIVIC", "VENOM_MUSTANG")
	 * This ID is used everywhere in code to reference this vehicle.
	 */
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
	// These define the upgrade ceiling - how far this vehicle can be pushed

	/**
	 * Maximum power achievable with full upgrades.
	 * This helps players understand a vehicle's tuning potential.
	 * A Civic might max at 500HP, but a Supra might reach 1000HP.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaxBuild")
	int32 MaxPower = 500;

	/** Maximum torque achievable with full upgrades */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaxBuild")
	int32 MaxTorque = 400;

	/**
	 * Minimum weight achievable with weight reduction mods.
	 * Carbon fiber parts, stripped interior, etc. can reduce weight
	 * but there's a floor based on the chassis.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaxBuild")
	int32 MinWeight = 2400;

	/** Maximum Performance Index achievable with optimal build */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MaxBuild")
	int32 MaxPI = 800;

	// === Tags ===
	// Flexible tagging system for filtering and categorization

	/**
	 * Searchable tags for filtering.
	 * Examples: "JDM", "FWD", "VTEC", "Turbo", "V8", "Classic", "Starter"
	 * The shop UI uses these for filter buttons.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	TArray<FString> Tags;

	// === Asset References ===
	// Links to actual Unreal assets (meshes, sounds, blueprints)

	/**
	 * Engine audio profile name - maps to audio system presets.
	 * Examples: "I4_VTEC", "V8_MUSCLE", "FLAT6_BOXER"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	FName EngineAudioProfile;

	/**
	 * Soft reference to the vehicle Blueprint class.
	 * "Soft" means it won't load until actually needed, saving memory.
	 * The actual vehicle Actor is spawned from this Blueprint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TSoftClassPtr<AActor> VehicleBlueprintClass;
};

// =============================================================================
// PARTS ENUMERATIONS
// These enums categorize aftermarket parts
// =============================================================================

/**
 * Part upgrade tier levels - quality/performance grades for parts.
 *
 * The tier system provides a clear upgrade path for players:
 *
 * Stock: Factory original parts (baseline, no cost to "install")
 *
 * Street: Entry-level aftermarket (~10-15% improvement)
 *   Examples: K&N air filter, cat-back exhaust, ECU tune
 *   Affordable, good for beginners
 *
 * Sport: Mid-tier performance (~20-30% improvement)
 *   Examples: Cold air intake, headers, bigger injectors
 *   Noticeable gains, reasonable price
 *
 * Race: Serious upgrades (~40-60% improvement)
 *   Examples: Turbo kits, forged internals, race suspension
 *   Expensive, may require supporting mods
 *
 * Elite: Top-tier parts (~70-90% improvement)
 *   Examples: Built engine, sequential gearbox, aero package
 *   Very expensive, professional-grade
 *
 * Legendary: Best possible parts (maximum performance)
 *   Examples: Billet everything, one-off custom parts
 *   Extremely rare and expensive
 *
 * Higher tiers often REQUIRE supporting mods (see MGPartDependencySubsystem).
 * For example, a Race turbo needs upgraded fuel system and cooling.
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
 * Part category for organization and mechanic specialization.
 *
 * Categories serve multiple purposes:
 *
 * 1. SHOP ORGANIZATION:
 *    Parts are grouped by category in the UI (tabs/filters)
 *
 * 2. MECHANIC SYSTEM:
 *    Each mechanic NPC specializes in certain categories.
 *    An engine specialist installs Engine parts faster/cheaper.
 *    See MGMechanicSubsystem for how this is used.
 *
 * 3. DEPENDENCY CHECKING:
 *    Some parts require others in the same or related category.
 *    A big turbo might require upgraded Engine parts (fuel system).
 *
 * CATEGORY BREAKDOWN:
 *
 * Engine: Anything that makes more power
 *   - Intake, exhaust, turbo/supercharger, internals, fuel system
 *
 * Drivetrain: Power delivery to wheels
 *   - Clutch, transmission, differential, axles, driveshaft
 *
 * Suspension: Handling and ride
 *   - Springs, dampers, sway bars, alignment parts, coilovers
 *
 * Brakes: Stopping power
 *   - Rotors, calipers, pads, lines, master cylinder
 *
 * Wheels: The actual wheel/rim
 * Tires: Rubber that contacts the road
 *
 * Aero: Aerodynamic parts
 *   - Spoilers, splitters, diffusers, canards
 *
 * Body: Exterior panels and kits
 *   - Bumpers, fenders, hoods, body kits
 *
 * Interior: Inside the cabin
 *   - Seats, roll cage, gauges, steering wheel
 *
 * Nitrous: N2O injection systems
 * Electronics: ECU, gauges, wiring
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

// =============================================================================
// PARTS CATALOG ROW
// This defines a complete part/upgrade in the game
// =============================================================================

/**
 * DataTable row for parts catalog - complete part definition.
 *
 * Similar to FMGVehicleCatalogRow, this defines all data for a single
 * aftermarket part. Parts can be universal (fit any car) or vehicle-specific.
 *
 * PARTS SYSTEM OVERVIEW:
 *
 * Parts are the core of the tuning experience. Players buy and install
 * parts to improve their vehicles. Each part has:
 *
 * - Pricing (base cost + labor for installation)
 * - Effects (power/torque/weight changes)
 * - Requirements (mechanic skill, prerequisite parts)
 * - Compatibility (which vehicles can use this part)
 *
 * COMPATIBILITY SYSTEM:
 *
 * CompatibleVehicles: If empty, part fits all vehicles (universal)
 * If populated, part only works on listed vehicles.
 *
 * RequiredParts: Must have these installed first (dependency chain)
 * Example: "TURBO_BIG" requires "INTERCOOLER_FMIC" and "FUEL_UPGRADED"
 *
 * ConflictingParts: Cannot be installed alongside these
 * Example: "SUPERCHARGER" conflicts with "TURBO_*"
 *
 * See MGPartDependencySubsystem for advanced dependency logic with
 * warnings and failure risk calculations.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGPartCatalogRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * Unique part identifier (DataTable row key).
	 * Convention: CATEGORY_TYPE_TIER (e.g., "TURBO_SMALL", "EXHAUST_CATBACK_SPORT")
	 */
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

// =============================================================================
// SIMPLIFIED INFO STRUCTS
// Lightweight structs for passing commonly-needed data
// =============================================================================

/**
 * Simplified pricing info for quick lookups.
 *
 * This is a subset of FMGVehicleEconomy, designed for fast passing
 * between systems that only need pricing data (not full vehicle info).
 *
 * WHY HAVE BOTH THIS AND FMGVehicleEconomy?
 *
 * FMGVehicleEconomy is embedded in the full catalog row (rich data).
 * FMGVehiclePricingInfo is returned by lookup functions (lightweight).
 *
 * This pattern (detailed storage struct vs. lightweight return struct)
 * is common when you want to:
 * - Avoid copying large amounts of data
 * - Hide internal implementation details
 * - Provide a stable API even if internal storage changes
 *
 * The bIsValid flag indicates whether the lookup succeeded. Always
 * check this before using the other values!
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

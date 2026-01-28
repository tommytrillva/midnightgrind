// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file MGVehicleClassSubsystem.h
 * @brief Vehicle Classification and Performance Index (PI) System
 * @author Midnight Grind Team
 * @version 1.0
 *
 * @section overview Overview
 * ============================================================================
 * MGVehicleClassSubsystem.h
 * Midnight Grind - Vehicle Classification and Performance Index System
 * ============================================================================
 *
 * This subsystem is responsible for categorizing vehicles by their performance
 * capabilities. Think of it like the "rating system" you see in racing games
 * like Forza Horizon or Need for Speed.
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection pi What is a Performance Index (PI)?
 * Performance Index is a single number (typically 100-999) that represents
 * how fast/capable a vehicle is overall. It's calculated from individual stats:
 * - Speed (top speed capability)
 * - Acceleration (0-60 time, torque)
 * - Handling (cornering grip, steering response)
 * - Braking (stopping power)
 * - Nitro (boost effectiveness)
 * - Durability (crash resistance)
 *
 * @subsection tiers What are Class Tiers?
 * Vehicles are grouped into tiers based on their PI:
 *   D Class (lowest) -> C -> B -> A -> S -> S+ -> Hyper -> Legend (highest)
 *
 * This allows fair matchmaking - a D-class Honda Civic won't race against
 * an S-class Lamborghini in competitive events.
 *
 * PI Ranges:
 * - D: Entry-level vehicles (PI ~100-199) - economy cars, project starters
 * - C: Street vehicles (PI ~200-299) - hot hatches, tuner bases
 * - B: Sport vehicles (PI ~300-399) - sports cars, muscle cars
 * - A: Super Sport (PI ~400-499) - high-end sports, tuned vehicles
 * - S: Supercar (PI ~500-599) - exotic supercars
 * - S+: Hypercar (PI ~600-699) - limited production hypercars
 * - Hyper: Ultimate (PI ~700-799) - extreme builds
 * - Legend: Legendary (PI 800+) - fully maxed builds
 *
 * @subsection classification 1. Vehicle Classification (FMGVehicleClassification)
 * Complete data about a vehicle: name, manufacturer, year, body type,
 * drivetrain, era, and current performance stats.
 *
 * @subsection profile 2. Performance Profile (FMGVehiclePerformanceProfile)
 * Detailed performance stats including real-world metrics like
 * horsepower, torque, weight, and power-to-weight ratio.
 *
 * @subsection restrictions 3. Class Restrictions (FMGClassRestriction)
 * Rules that define which vehicles can enter specific events.
 * Example: "JDM Legends" event might require Japanese manufacturers
 * and vehicles from the Retro era (1980-1999).
 *
 * @subsection weights 4. PI Weights (FMGPIWeights)
 * Different weight presets for calculating PI. A "Drift Tuned" preset
 * might value handling more than top speed.
 *
 * @section usage Usage Examples
 *
 * @code
 * // Get the subsystem from GameInstance
 * UMGVehicleClassSubsystem* VehicleClass = GetGameInstance()->GetSubsystem<UMGVehicleClassSubsystem>();
 *
 * // === REGISTERING VEHICLES ===
 * FMGVehicleClassification MyVehicle;
 * MyVehicle.VehicleId = "nissan_skyline_r34";
 * MyVehicle.VehicleName = FText::FromString("Nissan Skyline GT-R");
 * MyVehicle.Manufacturer = FText::FromString("Nissan");
 * MyVehicle.Year = 1999;
 * MyVehicle.BodyType = EMGVehicleBodyType::Coupe;
 * MyVehicle.Drivetrain = EMGDrivetrainType::AWD;
 * MyVehicle.Era = EMGVehicleEra::Retro;
 * MyVehicle.CountryOfOrigin = "Japan";
 * VehicleClass->RegisterVehicle(MyVehicle);
 *
 * // === QUERYING VEHICLES ===
 * // Get vehicle classification
 * FMGVehicleClassification Classification = VehicleClass->GetVehicleClassification("nissan_skyline_r34");
 *
 * // Get vehicle's current class tier
 * EMGVehicleClassTier Tier = VehicleClass->GetVehicleClassTier("nissan_skyline_r34");
 *
 * // Get all A-class vehicles
 * TArray<FMGVehicleClassification> AClassCars = VehicleClass->GetVehiclesByClass(EMGVehicleClassTier::A);
 *
 * // Get all Japanese cars
 * TArray<FMGVehicleClassification> JDMCars = VehicleClass->GetVehiclesByCountry("Japan");
 *
 * // Get all AWD vehicles
 * TArray<FMGVehicleClassification> AWDCars = VehicleClass->GetVehiclesByDrivetrain(EMGDrivetrainType::AWD);
 *
 * // === PERFORMANCE INDEX ===
 * // Get a vehicle's PI
 * int32 PI = VehicleClass->GetVehiclePI("nissan_skyline_r34");
 *
 * // Calculate PI from a performance profile
 * FMGPIWeights Weights = VehicleClass->GetPIWeightsForPreset(EMGPIWeightPreset::Balanced);
 * int32 CalculatedPI = VehicleClass->CalculatePI(PerformanceProfile, Weights);
 *
 * // Check how much PI headroom exists before hitting next class
 * int32 Headroom = VehicleClass->GetPIHeadroomForClass("nissan_skyline_r34");
 *
 * // Preview upgrade impact on PI
 * FMGUpgradePIImpact Impact = VehicleClass->CalculateUpgradeImpact("nissan_skyline_r34", "turbo_stage3");
 * if (Impact.bMayChangeClass)
 * {
 *     ShowWarningUI("This upgrade will change your vehicle class!");
 * }
 *
 * // === CLASS RESTRICTIONS ===
 * // Check if a vehicle can enter an event
 * if (VehicleClass->DoesVehicleMeetRestriction("nissan_skyline_r34", "JDM_Legends"))
 * {
 *     // Vehicle is eligible!
 * }
 *
 * // Find out why a vehicle doesn't meet restrictions
 * TArray<FString> Violations = VehicleClass->GetViolatedRestrictions("nissan_skyline_r34", "Muscle_Only");
 * // Might return: ["Body type 'Coupe' not in allowed types: Muscle"]
 *
 * // Get all eligible vehicles for an event
 * TArray<FMGVehicleClassification> EligibleCars = VehicleClass->GetEligibleVehicles("JDM_Legends");
 *
 * // === COMPARISON ===
 * // Compare two vehicles
 * FMGVehicleComparison Comparison = VehicleClass->CompareVehicles("nissan_skyline_r34", "mazda_rx7_fd");
 *
 * // === STATISTICS ===
 * // Get vehicle counts and averages
 * int32 TotalVehicles = VehicleClass->GetTotalVehicleCount();
 * int32 AClassCount = VehicleClass->GetVehicleCountInClass(EMGVehicleClassTier::A);
 * float AvgPI = VehicleClass->GetAveragePIInClass(EMGVehicleClassTier::A);
 *
 * // === EVENT LISTENERS ===
 * VehicleClass->OnVehicleClassChanged.AddDynamic(this, &UMyClass::HandleClassChange);
 * VehicleClass->OnVehiclePIChanged.AddDynamic(this, &UMyClass::HandlePIChange);
 * @endcode
 *
 * @section persistence Persistence
 * This is a GameInstanceSubsystem, meaning it persists across level loads.
 * Vehicle data can be saved/loaded using SaveVehicleClassData() and LoadVehicleClassData().
 *
 * @section delegates Available Delegates
 * - OnVehicleClassChanged: Fires when upgrades push a vehicle into a new class tier
 * - OnVehiclePIChanged: Fires when a vehicle's PI changes (from upgrades/tuning)
 * - OnVehicleRegistered: Fires when a new vehicle is added to the system
 *
 * @see UMGGarageSubsystem For managing the player's vehicle collection
 * @see UMGUpgradeSubsystem For applying upgrades that affect PI
 * @see UMGRaceModeSubsystem Uses class restrictions for race matchmaking
 * ============================================================================
 */

/**
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file defines the Vehicle Class Subsystem, which is responsible for
 * categorizing vehicles by their performance capabilities. Think of it like
 * the "rating system" you see in racing games like Forza Horizon or Need for Speed.
 *
 * WHAT IS A PERFORMANCE INDEX (PI)?
 * ---------------------------------
 * Performance Index is a single number (typically 100-999) that represents
 * how fast/capable a vehicle is overall. It's calculated from individual stats:
 *   - Speed (top speed capability)
 *   - Acceleration (0-60 time, torque)
 *   - Handling (cornering grip, steering response)
 *   - Braking (stopping power)
 *   - Nitro (boost effectiveness)
 *   - Durability (crash resistance)
 *
 * WHAT ARE CLASS TIERS?
 * ---------------------
 * Vehicles are grouped into tiers based on their PI:
 *   D Class (lowest) -> C -> B -> A -> S -> S+ -> Hyper -> Legend (highest)
 *
 * This allows fair matchmaking - a D-class Honda Civic won't race against
 * an S-class Lamborghini in competitive events.
 *
 * KEY CONCEPTS:
 * -------------
 * 1. VEHICLE CLASSIFICATION (FMGVehicleClassification)
 *    Complete data about a vehicle: name, manufacturer, year, body type,
 *    drivetrain, era, and current performance stats.
 *
 * 2. PERFORMANCE PROFILE (FMGVehiclePerformanceProfile)
 *    Detailed performance stats including real-world metrics like
 *    horsepower, torque, weight, and power-to-weight ratio.
 *
 * 3. CLASS RESTRICTIONS (FMGClassRestriction)
 *    Rules that define which vehicles can enter specific events.
 *    Example: "JDM Legends" event might require Japanese manufacturers
 *    and vehicles from the Retro era (1980-1999).
 *
 * 4. PI WEIGHTS (FMGPIWeights)
 *    Different weight presets for calculating PI. A "Drift Tuned" preset
 *    might value handling more than top speed.
 *
 * HOW TO USE THIS SUBSYSTEM:
 * --------------------------
 * @code
 * // Get the subsystem from GameInstance
 * UMGVehicleClassSubsystem* VehicleClass = GetGameInstance()->GetSubsystem<UMGVehicleClassSubsystem>();
 *
 * // Register a new vehicle
 * FMGVehicleClassification MyVehicle;
 * MyVehicle.VehicleId = "nissan_skyline_r34";
 * MyVehicle.VehicleName = FText::FromString("Nissan Skyline GT-R");
 * MyVehicle.Manufacturer = FText::FromString("Nissan");
 * MyVehicle.Year = 1999;
 * MyVehicle.BodyType = EMGVehicleBodyType::Coupe;
 * MyVehicle.Drivetrain = EMGDrivetrainType::AWD;
 * MyVehicle.Era = EMGVehicleEra::Retro;
 * VehicleClass->RegisterVehicle(MyVehicle);
 *
 * // Check if a vehicle can enter an event
 * if (VehicleClass->DoesVehicleMeetRestriction("nissan_skyline_r34", "JDM_Legends"))
 * {
 *     // Vehicle is eligible!
 * }
 *
 * // Get all A-class vehicles
 * TArray<FMGVehicleClassification> AClassCars = VehicleClass->GetVehiclesByClass(EMGVehicleClassTier::A);
 * @endcode
 *
 * PERSISTENCE:
 * ------------
 * This is a GameInstanceSubsystem, meaning it persists across level loads.
 * Vehicle data can be saved/loaded using SaveVehicleClassData() and LoadVehicleClassData().
 *
 * EVENTS TO LISTEN FOR:
 * ---------------------
 * - OnVehicleClassChanged: Fires when upgrades push a vehicle into a new class tier
 * - OnVehiclePIChanged: Fires when a vehicle's PI changes (from upgrades/tuning)
 * - OnVehicleRegistered: Fires when a new vehicle is added to the system
 *
 * @see UMGGarageSubsystem - For managing the player's vehicle collection
 * @see UMGUpgradeSubsystem - For applying upgrades that affect PI
 * @see UMGRaceModeSubsystem - Uses class restrictions for race matchmaking
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGVehicleClassSubsystem.generated.h"

/**
 * Vehicle class tier - Groups vehicles by overall performance capability
 *
 * Classes are determined by Performance Index (PI) ranges:
 * - D: Entry-level vehicles (PI ~100-199) - economy cars, project starters
 * - C: Street vehicles (PI ~200-299) - hot hatches, tuner bases
 * - B: Sport vehicles (PI ~300-399) - sports cars, muscle cars
 * - A: Super Sport (PI ~400-499) - high-end sports, tuned vehicles
 * - S: Supercar (PI ~500-599) - exotic supercars
 * - S+: Hypercar (PI ~600-699) - limited production hypercars
 * - Hyper: Ultimate (PI ~700-799) - extreme builds
 * - Legend: Legendary (PI 800+) - fully maxed builds
 * - Custom: User-defined class for special events
 */
UENUM(BlueprintType)
enum class EMGVehicleClassTier : uint8
{
	/** Entry level: Economy cars, project starters */
	D			UMETA(DisplayName = "D Class"),
	/** Street level: Hot hatches, beginner tuner cars */
	C			UMETA(DisplayName = "C Class"),
	/** Sport level: Sports cars, muscle cars */
	B			UMETA(DisplayName = "B Class"),
	/** Super Sport: High-end sports, well-tuned vehicles */
	A			UMETA(DisplayName = "A Class"),
	/** Supercar: Exotic supercars, heavily modified sports */
	S			UMETA(DisplayName = "S Class"),
	/** Hypercar: Limited production hypercars */
	SPlus		UMETA(DisplayName = "S+ Class"),
	/** Ultimate: Extreme performance builds */
	Hyper		UMETA(DisplayName = "Hyper Class"),
	/** Legendary: Maximum performance, fully maxed builds */
	Legend		UMETA(DisplayName = "Legend Class"),
	/** User-defined class for special events */
	Custom		UMETA(DisplayName = "Custom Class")
};

/**
 * Vehicle body type - The physical form factor of the vehicle
 *
 * Body type affects handling characteristics and can be used for
 * event restrictions (e.g., "Muscle Car Showdown" event).
 * Different body types have different default physics behaviors.
 */
UENUM(BlueprintType)
enum class EMGVehicleBodyType : uint8
{
	/** Small, economical cars (Honda Fit, VW Golf) */
	Compact		UMETA(DisplayName = "Compact"),
	/** Two-door sports styling (Nissan 370Z, BMW M4) */
	Coupe		UMETA(DisplayName = "Coupe"),
	/** Four-door practical performance (BMW M3, Subaru WRX) */
	Sedan		UMETA(DisplayName = "Sedan"),
	/** Sporty compact with rear hatch (VW GTI, Honda Civic Type R) */
	Hatchback	UMETA(DisplayName = "Hatchback"),
	/** Purpose-built sports cars (Porsche Cayman, Lotus Elise) */
	Sports		UMETA(DisplayName = "Sports Car"),
	/** American muscle (Mustang, Camaro, Challenger) */
	Muscle		UMETA(DisplayName = "Muscle Car"),
	/** High-end exotic performance (Ferrari, Lamborghini) */
	Supercar	UMETA(DisplayName = "Supercar"),
	/** Ultimate performance machines (Bugatti, Koenigsegg) */
	Hypercar	UMETA(DisplayName = "Hypercar"),
	/** Sport utility vehicles (BMW X5M, Porsche Cayenne) */
	SUV			UMETA(DisplayName = "SUV"),
	/** Performance trucks (Ford F-150 Raptor, RAM TRX) */
	Truck		UMETA(DisplayName = "Truck"),
	/** Estate/station wagon (Audi RS6 Avant, Mercedes E63 Wagon) */
	Wagon		UMETA(DisplayName = "Wagon"),
	/** Open-top sports (Mazda MX-5, Porsche Boxster) */
	Roadster	UMETA(DisplayName = "Roadster"),
	/** Japanese micro cars (Honda S660, Suzuki Cappuccino) */
	Kei			UMETA(DisplayName = "Kei Car"),
	/** Performance vans (VW Transporter, Ford Transit ST) */
	Van			UMETA(DisplayName = "Van"),
	/** Vintage/retro vehicles (60s-80s icons) */
	Classic		UMETA(DisplayName = "Classic"),
	/** Rare/exotic collector vehicles */
	Exotic		UMETA(DisplayName = "Exotic")
};

/**
 * Vehicle drivetrain type - Which wheels receive power
 *
 * Drivetrain fundamentally affects handling characteristics:
 * - FWD: Tends to understeer, stable but less responsive
 * - RWD: Can oversteer, better for drifting, more challenging
 * - AWD: Balanced, excellent traction, stable at high speeds
 *
 * Some events may restrict drivetrain (e.g., "RWD Drift Challenge")
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
	/** Front-Wheel Drive: Engine in front, power to front wheels. Understeer tendency. (Honda Civic) */
	FWD			UMETA(DisplayName = "Front-Wheel Drive"),
	/** Rear-Wheel Drive: Engine in front, power to rear wheels. Classic sports car layout. (BMW M3) */
	RWD			UMETA(DisplayName = "Rear-Wheel Drive"),
	/** All-Wheel Drive: Power to all wheels. Best traction. (Nissan GT-R, Audi RS) */
	AWD			UMETA(DisplayName = "All-Wheel Drive"),
	/** Mid-Engine RWD: Engine behind driver, rear drive. Balanced weight. (Ferrari, Lamborghini) */
	MR			UMETA(DisplayName = "Mid-Engine RWD"),
	/** Rear-Engine RWD: Engine at rear, rear drive. Unique handling. (Porsche 911) */
	RR			UMETA(DisplayName = "Rear-Engine RWD"),
	/** Full-Time 4WD: Permanent 4-wheel drive with center differential. (Rally vehicles) */
	F4WD		UMETA(DisplayName = "Full-Time 4WD")
};

/**
 * Vehicle era/generation
 */
UENUM(BlueprintType)
enum class EMGVehicleEra : uint8
{
	Classic		UMETA(DisplayName = "Classic (Pre-1980)"),
	Retro		UMETA(DisplayName = "Retro (1980-1999)"),
	Modern		UMETA(DisplayName = "Modern (2000-2015)"),
	Current		UMETA(DisplayName = "Current (2015+)"),
	Future		UMETA(DisplayName = "Future Concept")
};

/**
 * Performance stat category
 */
UENUM(BlueprintType)
enum class EMGPerformanceStat : uint8
{
	Speed		UMETA(DisplayName = "Top Speed"),
	Acceleration UMETA(DisplayName = "Acceleration"),
	Handling	UMETA(DisplayName = "Handling"),
	Braking		UMETA(DisplayName = "Braking"),
	Nitro		UMETA(DisplayName = "Nitro"),
	Durability	UMETA(DisplayName = "Durability")
};

/**
 * PI calculation weight preset
 */
UENUM(BlueprintType)
enum class EMGPIWeightPreset : uint8
{
	Balanced	UMETA(DisplayName = "Balanced"),
	SpeedFocus	UMETA(DisplayName = "Speed Focus"),
	AccelFocus	UMETA(DisplayName = "Acceleration Focus"),
	HandlingFocus UMETA(DisplayName = "Handling Focus"),
	DriftTuned	UMETA(DisplayName = "Drift Tuned"),
	DragTuned	UMETA(DisplayName = "Drag Tuned"),
	Custom		UMETA(DisplayName = "Custom Weights")
};

/**
 * Individual performance stat value
 *
 * Tracks a single performance stat (Speed, Acceleration, etc.) with:
 * - Base value: The vehicle's stock stat
 * - Modified value: Current stat after upgrades and tuning
 * - Bonuses: Separate tracking of what upgrades/tuning contributed
 *
 * The final stat shown to player = BaseValue + UpgradeBonus + TuneBonus
 * This allows seeing the impact of each modification source.
 */
USTRUCT(BlueprintType)
struct FMGPerformanceStatValue
{
	GENERATED_BODY()

	/** Which stat this value represents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPerformanceStat Stat = EMGPerformanceStat::Speed;

	/** Stock/base value of this stat (before any modifications) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseValue = 0.0f;

	/** Current effective value (BaseValue + UpgradeBonus + TuneBonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ModifiedValue = 0.0f;

	/** Maximum possible value for this stat (typically 10.0 for display) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 10.0f;

	/** Bonus from installed upgrade parts (engine, suspension, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UpgradeBonus = 0.0f;

	/** Bonus from tuning adjustments (gear ratios, alignment, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TuneBonus = 0.0f;
};

/**
 * Vehicle performance profile
 */
USTRUCT(BlueprintType)
struct FMGVehiclePerformanceProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Handling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Braking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Nitro;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Durability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedKMH = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZeroToSixtyTime = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QuarterMileTime = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerHP = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueNM = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeightKG = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerToWeightRatio = 0.0f;
};

/**
 * Class tier definition with PI range
 */
USTRUCT(BlueprintType)
struct FMGClassTierDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleClassTier Tier = EMGVehicleClassTier::D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPI = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPI = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ClassColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ClassIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedEventTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseRewardMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DifficultyMultiplier = 1.0f;
};

/**
 * Vehicle classification data
 */
USTRUCT(BlueprintType)
struct FMGVehicleClassification
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VehicleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Manufacturer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Year = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleClassTier ClassTier = EMGVehicleClassTier::D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleBodyType BodyType = EMGVehicleBodyType::Coupe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDrivetrainType Drivetrain = EMGDrivetrainType::RWD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleEra Era = EMGVehicleEra::Modern;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePI = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPI = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPI = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGVehiclePerformanceProfile PerformanceProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SpecialTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CountryOfOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRare = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLegendary = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlockable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePurchasePrice = 0;
};

/**
 * PI calculation weights
 */
USTRUCT(BlueprintType)
struct FMGPIWeights
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HandlingWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingWeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroWeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurabilityWeight = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerToWeightFactor = 1.5f;
};

/**
 * Class restriction for events - Defines vehicle eligibility rules
 *
 * Used to create themed events with specific vehicle requirements.
 *
 * Examples of restrictions:
 * - "JDM Legends": AllowedCountries = {"Japan"}, AllowedEras = {Retro}
 * - "Muscle Mayhem": AllowedBodyTypes = {Muscle}, AllowedCountries = {"USA"}
 * - "B-Class Circuit": AllowedTiers = {B}, MaxPI = 399
 * - "Stock Showdown": bRequireStock = true (no upgrades allowed)
 * - "Drift King": RequiredTags = {"DriftSpecial"}, AllowedDrivetrains = {RWD}
 *
 * Empty arrays mean "no restriction" for that property.
 * A vehicle must meet ALL specified criteria to be eligible.
 */
USTRUCT(BlueprintType)
struct FMGClassRestriction
{
	GENERATED_BODY()

	/** Unique identifier for this restriction set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RestrictionId;

	/** Display name shown to player (e.g., "JDM Legends Requirement") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RestrictionName;

	/** Allowed class tiers. Empty = all tiers allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGVehicleClassTier> AllowedTiers;

	/** Minimum Performance Index required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPI = 0;

	/** Maximum Performance Index allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPI = 999;

	/** Allowed body types. Empty = all body types allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGVehicleBodyType> AllowedBodyTypes;

	/** Allowed drivetrain types. Empty = all drivetrains allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGDrivetrainType> AllowedDrivetrains;

	/** Allowed vehicle eras. Empty = all eras allowed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGVehicleEra> AllowedEras;

	/** Allowed countries of origin (e.g., "Japan", "Germany"). Empty = all. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedCountries;

	/** Allowed manufacturers (e.g., "Nissan", "BMW"). Empty = all. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedManufacturers;

	/** Vehicle must have ALL of these tags to be eligible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RequiredTags;

	/** Specific vehicle IDs that are banned from this event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ExcludedVehicleIds;

	/** If true, vehicle must be completely stock (no upgrades) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireStock = false;

	/** If false, upgraded vehicles are not allowed (even if otherwise eligible) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowUpgrades = true;
};

/**
 * Upgrade impact on PI
 */
USTRUCT(BlueprintType)
struct FMGUpgradePIImpact
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UpgradeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PIChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPerformanceStat, float> StatChanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMayChangeClass = false;
};

/**
 * Vehicle comparison result
 */
USTRUCT(BlueprintType)
struct FMGVehicleComparison
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PIDifference = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPerformanceStat, float> StatDifferences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RecommendedChoice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComparisonSummary;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVehicleClassChanged, const FString&, VehicleId, EMGVehicleClassTier, OldClass, EMGVehicleClassTier, NewClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVehiclePIChanged, const FString&, VehicleId, int32, OldPI, int32, NewPI);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleRegistered, const FString&, VehicleId, EMGVehicleClassTier, ClassTier);

/**
 * Vehicle Class Subsystem
 * Manages vehicle classification, performance index, and class restrictions
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleClassSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "VehicleClass|Events")
	FOnVehicleClassChanged OnVehicleClassChanged;

	UPROPERTY(BlueprintAssignable, Category = "VehicleClass|Events")
	FOnVehiclePIChanged OnVehiclePIChanged;

	UPROPERTY(BlueprintAssignable, Category = "VehicleClass|Events")
	FOnVehicleRegistered OnVehicleRegistered;

	// Vehicle Registration
	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	bool RegisterVehicle(const FMGVehicleClassification& Classification);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	bool UnregisterVehicle(const FString& VehicleId);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	FMGVehicleClassification GetVehicleClassification(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	bool IsVehicleRegistered(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	TArray<FMGVehicleClassification> GetAllVehicles() const;

	// Class Tier Management
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Tier")
	bool RegisterClassTier(const FMGClassTierDefinition& TierDef);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Tier")
	FMGClassTierDefinition GetClassTierDefinition(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Tier")
	EMGVehicleClassTier GetVehicleClassTier(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Tier")
	EMGVehicleClassTier CalculateClassTierFromPI(int32 PI) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Tier")
	FLinearColor GetClassColor(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Tier")
	FText GetClassDisplayName(EMGVehicleClassTier Tier) const;

	// Performance Index
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	int32 GetVehiclePI(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	int32 CalculatePI(const FMGVehiclePerformanceProfile& Profile, const FMGPIWeights& Weights) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	int32 CalculatePIWithPreset(const FMGVehiclePerformanceProfile& Profile, EMGPIWeightPreset Preset) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	bool UpdateVehiclePI(const FString& VehicleId, int32 NewPI);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	FMGPIWeights GetPIWeightsForPreset(EMGPIWeightPreset Preset) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	FMGUpgradePIImpact CalculateUpgradeImpact(const FString& VehicleId, const FString& UpgradeId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	int32 GetPIHeadroomForClass(const FString& VehicleId) const;

	// Performance Stats
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Stats")
	FMGVehiclePerformanceProfile GetPerformanceProfile(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Stats")
	bool UpdatePerformanceProfile(const FString& VehicleId, const FMGVehiclePerformanceProfile& Profile);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Stats")
	float GetPerformanceStat(const FString& VehicleId, EMGPerformanceStat Stat) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Stats")
	FMGVehicleComparison CompareVehicles(const FString& VehicleIdA, const FString& VehicleIdB) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	float CalculatePowerToWeightRatio(float PowerHP, float WeightKG) const;

	// Class Restrictions
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	bool RegisterRestriction(const FMGClassRestriction& Restriction);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	FMGClassRestriction GetRestriction(const FString& RestrictionId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	bool DoesVehicleMeetRestriction(const FString& VehicleId, const FString& RestrictionId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	TArray<FString> GetViolatedRestrictions(const FString& VehicleId, const FString& RestrictionId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	TArray<FMGVehicleClassification> GetEligibleVehicles(const FString& RestrictionId) const;

	// Queries
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByClass(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByBodyType(EMGVehicleBodyType BodyType) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByDrivetrain(EMGDrivetrainType Drivetrain) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByEra(EMGVehicleEra Era) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByPIRange(int32 MinPI, int32 MaxPI) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByManufacturer(const FString& Manufacturer) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByCountry(const FString& Country) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FString> GetAllManufacturers() const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FString> GetAllCountries() const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	int32 GetTotalVehicleCount() const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	int32 GetVehicleCountInClass(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	float GetAveragePIInClass(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	FMGVehicleClassification GetHighestPIVehicle() const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	FMGVehicleClassification GetLowestPIVehicle() const;

	// Utility
	UFUNCTION(BlueprintPure, Category = "VehicleClass|Utility")
	FText GetBodyTypeDisplayName(EMGVehicleBodyType BodyType) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Utility")
	FText GetDrivetrainDisplayName(EMGDrivetrainType Drivetrain) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Utility")
	FText GetEraDisplayName(EMGVehicleEra Era) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Persistence")
	void SaveVehicleClassData();

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Persistence")
	void LoadVehicleClassData();

protected:
	void InitializeDefaultClassTiers();
	void RecalculateVehicleClass(const FString& VehicleId);

private:
	UPROPERTY()
	TMap<FString, FMGVehicleClassification> RegisteredVehicles;

	UPROPERTY()
	TMap<EMGVehicleClassTier, FMGClassTierDefinition> ClassTierDefinitions;

	UPROPERTY()
	TMap<FString, FMGClassRestriction> RegisteredRestrictions;

	UPROPERTY()
	TMap<EMGPIWeightPreset, FMGPIWeights> PIWeightPresets;
};

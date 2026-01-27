// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehicleContentAssets.h
 * @brief Vehicle configuration data assets for Midnight Grind racing game.
 *
 * This file contains data asset definitions for configuring vehicles, including:
 * - Vehicle identity (name, manufacturer, class, body style)
 * - Visual assets (meshes, materials, paint options, customization)
 * - Engine specifications (power, torque, RPM ranges)
 * - Transmission settings (gear ratios, shift times)
 * - Handling characteristics (grip, drift, steering)
 * - Performance stats for UI display
 * - NOS/nitrous boost configuration
 * - Audio and VFX references
 * - Unlock requirements for progression
 *
 * @section usage_vehicle Usage
 * Create vehicle data assets in the Unreal Editor:
 * Right-click in Content Browser > Miscellaneous > Data Asset > MGVehicleDataAsset
 *
 * Each driveable vehicle in the game requires a corresponding UMGVehicleDataAsset.
 * Use UMGVehicleCollectionAsset to group vehicles by manufacturer or theme.
 *
 * @section vehicle_classes Vehicle Class System
 * Vehicles are categorized into classes (D through S+) based on performance:
 * - D_Class: Starter vehicles, balanced and forgiving
 * - C_Class: Improved performance, good for learning
 * - B_Class: Sports cars, requires skill
 * - A_Class: Supercars, high performance
 * - S_Class: Hypercars, elite performance
 * - S_Plus: Legendary vehicles, maximum performance
 *
 * @see UMGVehicleDataAsset
 * @see UMGVehicleCollectionAsset
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGVehicleContentAssets.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UMaterialInterface;
class USoundBase;
class UNiagaraSystem;
class UAnimMontage;

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * Vehicle class/category
 *
 * Performance tier classification for matchmaking and race balancing.
 * Higher classes have better performance but may be restricted in
 * certain race modes or require higher player levels to unlock.
 */
UENUM(BlueprintType)
enum class EMGVehicleClass : uint8
{
	/** Starter vehicles - balanced */
	D_Class,
	/** Improved performance */
	C_Class,
	/** Sports cars */
	B_Class,
	/** Supercars */
	A_Class,
	/** Hypercars */
	S_Class,
	/** Legendary vehicles */
	S_Plus
};

/**
 * Vehicle drivetrain type
 *
 * Determines which wheels receive engine power and affects
 * handling characteristics, especially during acceleration and cornering.
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
	FWD,  ///< Front-wheel drive - power to front wheels, tends to understeer
	RWD,  ///< Rear-wheel drive - power to rear wheels, can oversteer/drift
	AWD   ///< All-wheel drive - power to all wheels, best traction
};

/**
 * Vehicle body style
 *
 * Visual category for the vehicle's body type.
 * Affects garage organization and may influence physics (drag, weight distribution).
 */
UENUM(BlueprintType)
enum class EMGBodyStyle : uint8
{
	Coupe,      ///< Two-door sports coupe
	Sedan,      ///< Four-door sedan
	Hatchback,  ///< Compact hatchback
	SUV,        ///< Sport utility vehicle
	Truck,      ///< Pickup truck
	Muscle,     ///< American muscle car
	Sports,     ///< European sports car
	Supercar,   ///< High-end supercar
	Classic,    ///< Vintage/classic car
	JDM         ///< Japanese domestic market import
};

// ============================================================================
// STRUCTURE DEFINITIONS - ENGINE & TRANSMISSION
// ============================================================================

/**
 * Engine specification
 *
 * Defines the powertrain characteristics of a vehicle's engine.
 * These values feed into the physics simulation for realistic
 * acceleration curves and engine behavior.
 */
USTRUCT(BlueprintType)
struct FMGEngineSpec
{
	GENERATED_BODY()

	/// Descriptive engine name for UI (e.g., "2.0L Turbocharged I4", "5.2L V10")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EngineName;

	/// Engine displacement in liters (affects sound and character)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Displacement = 2.0f;

	/// Number of cylinders (4, 6, 8, 10, 12, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cylinders = 4;

	/// True if engine has turbocharger (affects boost lag and sound)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTurbocharged = false;

	/// True if engine has supercharger (instant boost, distinct sound)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSupercharged = false;

	/// Maximum horsepower output at peak RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Horsepower = 200.0f;

	/// Maximum torque in Newton-meters (affects low-end acceleration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Torque = 250.0f;

	/// Maximum safe engine RPM (rev limiter kicks in at this point)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RedlineRPM = 7000.0f;

	/// Engine RPM when stationary with no throttle input
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdleRPM = 800.0f;

	/// RPM where optimal power begins (start of power band)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerBandStart = 4000.0f;

	/// RPM where peak power is produced (optimal shift point)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerBandPeak = 6000.0f;
};

/**
 * Transmission specification
 *
 * Defines gear ratios and shifting behavior for the vehicle.
 * Gear ratios determine speed vs acceleration trade-off at each gear.
 */
USTRUCT(BlueprintType)
struct FMGTransmissionSpec
{
	GENERATED_BODY()

	/// Total number of forward gears (typically 5-8 for sports cars)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GearCount = 6;

	/// Gear ratio for each forward gear (index 0 = 1st gear)
	/// Higher ratios = more torque multiplication but lower top speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> GearRatios;

	/// Final drive ratio (affects all gears, higher = more acceleration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalDriveRatio = 3.5f;

	/// Time in seconds to complete a gear shift (affects acceleration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShiftTime = 0.2f;

	/// True for automatic transmission, false for manual/sequential
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomatic = true;

	FMGTransmissionSpec()
	{
		// Default 6-speed ratios typical of a sports car
		GearRatios = { 3.5f, 2.2f, 1.5f, 1.1f, 0.9f, 0.75f };
	}
};

// ============================================================================
// STRUCTURE DEFINITIONS - HANDLING & PERFORMANCE
// ============================================================================

/**
 * Handling characteristics
 *
 * Defines how the vehicle responds to player input and behaves
 * during cornering, acceleration, and drifting. These values
 * are tuned per-vehicle for distinct driving feel.
 */
USTRUCT(BlueprintType)
struct FMGHandlingSpec
{
	GENERATED_BODY()

	/// Drivetrain configuration (affects power delivery and handling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDrivetrainType Drivetrain = EMGDrivetrainType::RWD;

	/// Steering response multiplier (0.5 = sluggish, 2.0 = twitchy)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float SteeringSensitivity = 1.0f;

	/// How quickly steering input reaches maximum angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringSpeed = 5.0f;

	/// Maximum wheel turn angle in degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSteerAngle = 35.0f;

	/// Tire grip multiplier (0.5 = ice, 2.0 = racing slicks)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float GripMultiplier = 1.0f;

	/// How easily the vehicle enters a drift (0 = hard, 1 = easy)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DriftPropensity = 0.5f;

	/// How stable the vehicle is while maintaining a drift angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftStability = 0.7f;

	/// Aerodynamic downforce multiplier (affects high-speed grip)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceCoefficient = 0.5f;

	/// Front/rear weight bias (0 = all rear, 0.5 = balanced, 1 = all front)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WeightDistribution = 0.5f;
};

/**
 * Performance stats (normalized 0-100)
 *
 * Simplified performance ratings for UI display and comparison.
 * These values are shown in the garage and vehicle selection screens
 * to help players compare vehicles at a glance.
 */
USTRUCT(BlueprintType)
struct FMGPerformanceStats
{
	GENERATED_BODY()

	/// Maximum speed rating (higher = faster top speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 TopSpeed = 50;

	/// Acceleration rating (higher = quicker 0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Acceleration = 50;

	/// Cornering/handling rating (higher = better grip)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Handling = 50;

	/// Braking power rating (higher = shorter stopping distance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Braking = 50;

	/// Drift capability rating (higher = easier to drift)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Drift = 50;

	/// Nitrous boost effectiveness rating
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Nitro = 50;

	/**
	 * Calculates the average of all performance stats.
	 * @return Overall performance rating (0-100)
	 */
	int32 GetOverallRating() const
	{
		return (TopSpeed + Acceleration + Handling + Braking + Drift + Nitro) / 6;
	}
};

// ============================================================================
// STRUCTURE DEFINITIONS - CUSTOMIZATION
// ============================================================================

/**
 * Visual customization slot
 *
 * Defines a customizable part attachment point on the vehicle.
 * Examples: spoiler, hood, bumper, side skirts, wheels.
 */
USTRUCT(BlueprintType)
struct FMGCustomizationSlot
{
	GENERATED_BODY()

	/// Unique identifier for this slot (e.g., "Spoiler", "FrontBumper")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotID;

	/// Localized name shown in customization UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Skeletal mesh socket or bone name for attachment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachmentSocket;

	/// Default mesh when no customization is applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* DefaultMesh;

	/// All available mesh options for this slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UStaticMesh*> AvailableMeshes;
};

/**
 * Paint/livery option
 *
 * Defines a paint finish or color option for vehicle customization.
 * Includes material properties for realistic paint rendering.
 */
USTRUCT(BlueprintType)
struct FMGPaintOption
{
	GENERATED_BODY()

	/// Unique identifier for this paint option
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PaintID;

	/// Localized name shown in paint shop (e.g., "Midnight Blue")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Primary paint color (RGB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BaseColor = FLinearColor::White;

	/// Metallic intensity (0 = matte, 1 = full metallic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Metallic = 0.5f;

	/// Surface roughness (0 = mirror shine, 1 = matte)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Roughness = 0.3f;

	/// Clearcoat layer intensity (0 = none, 1 = full gloss)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Clearcoat = 0.8f;

	/// True if this is a premium/special paint requiring unlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPremium = false;

	/// In-game currency cost to purchase this paint option
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockPrice = 0;
};

/**
 * Vehicle unlock requirements
 *
 * Defines the conditions a player must meet to unlock/purchase a vehicle.
 * Supports multiple unlock paths: level, reputation, achievements, or ownership.
 */
USTRUCT(BlueprintType)
struct FMGVehicleUnlockRequirements
{
	GENERATED_BODY()

	/// True if vehicle is available from game start without unlocking
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlockedByDefault = false;

	/// Minimum player level required to see/purchase this vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/// Minimum reputation points needed (from race performance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredReputation = 0;

	/// In-game currency cost to purchase after meeting requirements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PurchasePrice = 10000;

	/// Achievement ID that must be completed to unlock (empty = none required)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredAchievement;

	/// Vehicle ID that must be owned first (e.g., must own base model for upgrade)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicle;
};

// ============================================================================
// DATA ASSET CLASSES
// ============================================================================

/**
 * Vehicle Data Asset
 *
 * Complete definition of a driveable vehicle in the game.
 * Contains all visual, audio, physics, and gameplay data needed
 * to spawn and drive a vehicle.
 *
 * @section creating_vehicle Creating a Vehicle Asset
 * 1. Right-click in Content Browser
 * 2. Select Miscellaneous > Data Asset
 * 3. Choose MGVehicleDataAsset
 * 4. Configure all required properties
 *
 * @section required_vehicle Required Configuration
 * - VehicleID: Unique identifier (must be unique across all vehicles)
 * - VehicleMesh: Skeletal mesh with proper bone hierarchy
 * - Engine, Transmission, Handling: Physics configuration
 * - Stats: UI display values (should match actual physics)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// IDENTITY
	// ==========================================
	// Vehicle identification and categorization info.

	/// Unique identifier for this vehicle (e.g., "VEH_Nissan_GTR_R35")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName VehicleID;

	/// Localized display name (e.g., "GT-R R35")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText VehicleName;

	/// Vehicle manufacturer name (e.g., "Nissan")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Manufacturer;

	/// Model year of the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	int32 Year = 2024;

	/// Performance class for matchmaking and progression
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGVehicleClass VehicleClass = EMGVehicleClass::D_Class;

	/// Visual body type category
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGBodyStyle BodyStyle = EMGBodyStyle::Coupe;

	/// Extended description for garage/dealer screens
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	// ==========================================
	// VISUALS
	// ==========================================
	// Mesh and material assets for rendering the vehicle.

	/// Main skeletal mesh for the vehicle (includes suspension/wheel bones)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	USkeletalMesh* VehicleMesh;

	/// Small preview image for selection UI (recommended: 256x256)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	UTexture2D* Thumbnail;

	/// High-detail static mesh for showroom display
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	UStaticMesh* ShowroomMesh;

	/// Default body material (paint system modifies parameters on this)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	UMaterialInterface* BaseMaterial;

	/// Available paint finishes for this vehicle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TArray<FMGPaintOption> PaintOptions;

	/// Body part customization attachment points
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TArray<FMGCustomizationSlot> CustomizationSlots;

	// ==========================================
	// SPECS
	// ==========================================
	// Physics and simulation parameters.

	/// Engine power and RPM configuration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGEngineSpec Engine;

	/// Gear ratios and shifting configuration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGTransmissionSpec Transmission;

	/// Steering, grip, and drift parameters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGHandlingSpec Handling;

	/// Simplified stats for UI display (0-100 scale)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGPerformanceStats Stats;

	/// Vehicle curb weight in kilograms (affects acceleration and handling)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	float Mass = 1400.0f;

	/// Maximum speed in kilometers per hour
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	float TopSpeedKPH = 250.0f;

	/// 0-100 KPH acceleration time in seconds (for display/comparison)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	float ZeroToHundredTime = 5.0f;

	// ==========================================
	// NOS
	// ==========================================
	// Nitrous oxide boost system configuration.

	/// Maximum NOS capacity (units consumed during boost)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NOS")
	float NOSCapacity = 100.0f;

	/// Speed/power multiplier when NOS is active (1.0 = no boost)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NOS")
	float NOSPowerBoost = 1.5f;

	/// NOS units consumed per second while boosting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NOS")
	float NOSConsumptionRate = 20.0f;

	/// NOS units regenerated per second (from drift, near-misses, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NOS")
	float NOSRechargeRate = 5.0f;

	// ==========================================
	// AUDIO
	// ==========================================
	// Sound assets for engine and vehicle effects.

	/// Primary looping engine sound (pitch varies with RPM)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* EngineSound;

	/// One-shot sound when starting the engine
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* StartupSound;

	/// Turbo whistle/blow-off valve sound (if turbocharged)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* TurboSound;

	/// Exhaust backfire/pop sound on deceleration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* BackfireSound;

	// ==========================================
	// VFX
	// ==========================================
	// Visual effect assets for exhaust, tires, and boost.

	/// Exhaust flame/smoke particle system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* ExhaustEffect;

	/// Tire smoke/burnout particle system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* TireSmokeEffect;

	/// NOS activation flame effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* NOSEffect;

	// ==========================================
	// UNLOCK
	// ==========================================
	// Progression system requirements.

	/// Requirements to unlock/purchase this vehicle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	FMGVehicleUnlockRequirements UnlockRequirements;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * Returns a formatted string with key vehicle specifications.
	 * Used for garage and dealer UI display.
	 * @return Formatted text with HP, torque, and drivetrain info
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	FText GetFormattedSpecs() const;

	/**
	 * Returns the display name for this vehicle's class.
	 * @return Localized class name (e.g., "S-Class", "A-Class")
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	FText GetClassDisplayName() const;
};

/**
 * Vehicle Collection Asset
 *
 * Groups vehicles together for manufacturer collections, themed sets,
 * or progression milestones. Completing a collection may grant bonuses.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleCollectionAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/// Unique identifier for this collection (e.g., "COLL_JDM_Legends")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FName CollectionID;

	/// Localized display name (e.g., "JDM Legends Collection")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FText CollectionName;

	/// Description of the collection theme
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FText Description;

	/// Vehicles included in this collection (soft references for async loading)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	TArray<TSoftObjectPtr<UMGVehicleDataAsset>> Vehicles;

	/// Preview image for collection display in garage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	UTexture2D* Thumbnail;

	/// In-game currency bonus awarded for owning all vehicles in collection
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	int32 CompletionBonus = 0;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

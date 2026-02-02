// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTireSubsystem.h
 * @brief Tire Simulation and Management Subsystem for Midnight Grind Racing
 *
 * This subsystem provides comprehensive tire physics simulation and management for all
 * vehicles in the game. It handles tire wear, temperature, grip calculations, compound
 * characteristics, and tire strategy for races.
 *
 * =============================================================================
 * OVERVIEW FOR ENTRY-LEVEL DEVELOPERS
 * =============================================================================
 *
 * Tires are one of the most important aspects of racing simulation. This subsystem
 * simulates how real tires behave:
 *
 * 1. TIRE WEAR: As you drive, tires lose rubber and grip decreases. Aggressive
 *    driving (hard braking, fast cornering) wears tires faster.
 *
 * 2. TIRE TEMPERATURE: Tires need to be at optimal temperature for best grip.
 *    Cold tires = less grip (slippery). Overheated tires = also less grip and
 *    faster wear. The "optimal window" is the sweet spot.
 *
 * 3. TIRE COMPOUNDS: Different rubber mixtures with different characteristics:
 *    - Soft compounds: More grip but wear faster (good for qualifying)
 *    - Hard compounds: Less grip but last longer (good for long stints)
 *    - Wet compounds: Special rubber for rain conditions
 *
 * 4. TIRE GRIP: The amount of friction between tire and track. Affected by:
 *    - Compound type (softer = more grip)
 *    - Temperature (optimal range = best grip)
 *    - Wear level (new tires = more grip)
 *    - Track surface (asphalt vs dirt vs wet)
 *
 * Key Racing Terms:
 * - "Stint": Period between pit stops on one set of tires
 * - "Compound": The type of rubber mixture (Soft, Medium, Hard, etc.)
 * - "Grip": How much traction the tire provides
 * - "Lockup": When brakes lock and tire slides (causes flat spots and wear)
 * - "Wheelspin": When tires spin faster than the car is moving (traction loss)
 * - "Slip Ratio": Difference between wheel speed and car speed
 * - "Slip Angle": Angle between where the tire points and where it's going
 *
 * =============================================================================
 * HOW IT FITS INTO THE GAME ARCHITECTURE
 * =============================================================================
 *
 * This is a UGameInstanceSubsystem, meaning:
 * - One instance exists for the entire game session
 * - Persists across level loads (keeps tire data when changing tracks)
 * - Access via: GetGameInstance()->GetSubsystem<UMGTireSubsystem>()
 *
 * Integration Points:
 * - MGPitStopSubsystem: Tire changes during pit stops
 * - MG_FUEL_Subsystem: Tire wear affects fuel consumption slightly
 * - Vehicle Physics: Grip values fed to wheel components for handling
 * - AI System: AI uses tire data for pit stop strategy decisions
 * - HUD/UI: Displays tire temperatures, wear levels, and compound info
 *
 * Typical Usage Flow:
 * 1. RegisterVehicle() when a car joins the race
 * 2. UpdateTireState() called every frame with physics data
 * 3. GetTireGrip() returns current grip for physics calculations
 * 4. ChangeTires() called during pit stops to install fresh tires
 *
 * @see UMGPitStopSubsystem for tire change operations during pit stops
 * @see UMGFuelSubsystem for how tire wear affects fuel consumption
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTireSubsystem.generated.h"

//=============================================================================
// TIRE COMPOUND TYPES
//=============================================================================

/**
 * @brief Available tire compound types with different performance characteristics
 *
 * Each compound is designed for specific conditions and driving styles.
 * Choosing the right compound is crucial for race strategy.
 *
 * Racing Compound Hierarchy (grip vs durability):
 * - UltraSoft: Maximum grip, shortest life (~10 laps)
 * - Soft: High grip, short life (~15 laps)
 * - Medium: Balanced (~25 laps)
 * - Hard: Lower grip, longest life (~40 laps)
 *
 * Special Compounds:
 * - Intermediate: Light rain (standing water in patches)
 * - FullWet: Heavy rain (lots of standing water)
 * - Others: Specialized for specific game modes
 */
UENUM(BlueprintType)
enum class EMGTireCompoundType : uint8
{
	UltraSoft,     ///< Maximum grip, wears very quickly. Best for qualifying or short stints.
	Soft,          ///< High grip with short lifespan. Good for aggressive strategies.
	Medium,        ///< Balanced grip and durability. The "safe" choice for most conditions.
	Hard,          ///< Lower grip but lasts longest. Good for long stints or fuel saving.
	Intermediate,  ///< Light rain compound. Grooves channel water away on damp tracks.
	FullWet,       ///< Heavy rain compound. Deep grooves for standing water.
	AllSeason,     ///< General purpose tire for free roam mode. Decent in all conditions.
	DriftCompound, ///< Special compound designed for controlled sliding in drift events.
	OffRoad,       ///< Aggressive tread pattern for dirt and gravel surfaces.
	Rally,         ///< Mixed surface compound for rally stages (tarmac/gravel mix).
	Slick,         ///< Pure racing compound with no tread. Dry conditions only.
	Vintage        ///< Period-correct tires for classic cars. Less grip than modern tires.
};

//=============================================================================
// TIRE POSITION ENUMERATION
//=============================================================================

/**
 * @brief Identifies which corner of the vehicle a tire is on
 *
 * Used throughout the system to reference specific tires. Each tire can have
 * different wear, temperature, and grip based on the car's setup and driving style.
 *
 * Front tires typically handle more braking wear and steering forces.
 * Rear tires handle more acceleration wear (on RWD cars).
 */
UENUM(BlueprintType)
enum class EMGTirePosition : uint8
{
	FrontLeft,   ///< Driver's side front tire (in left-hand drive countries)
	FrontRight,  ///< Passenger's side front tire
	RearLeft,    ///< Driver's side rear tire
	RearRight    ///< Passenger's side rear tire
};

//=============================================================================
// TIRE CONDITION ENUMERATION
//=============================================================================

/**
 * @brief Overall health state of a tire
 *
 * This is a simplified view of tire status for UI display and AI decision making.
 * Derived from wear level and damage state.
 *
 * Condition affects both grip and risk:
 * - Optimal/Good: Safe to drive, full performance
 * - Worn: Reduced grip, consider pitting soon
 * - Critical: Significant grip loss, pit immediately or risk failure
 * - Punctured/Blown: Immediate pit stop required
 */
UENUM(BlueprintType)
enum class EMGTireCondition : uint8
{
	Optimal,   ///< Like-new condition (100-80% wear remaining). Full performance.
	Good,      ///< Some wear but still performing well (80-50% remaining).
	Worn,      ///< Noticeable grip loss (50-25% remaining). Start planning pit stop.
	Critical,  ///< Severely worn (<25% remaining). Pit immediately or risk failure.
	Punctured, ///< Slow leak - can limp to pits but reduced speed. Must change.
	Blown      ///< Complete tire failure. Vehicle difficult/impossible to control.
};

//=============================================================================
// TIRE WEAR PATTERN ENUMERATION
//=============================================================================

/**
 * @brief How wear is distributed across the tire surface
 *
 * Wear patterns indicate setup problems or driving style issues:
 * - Even wear: Good setup, balanced driving
 * - Edge wear: Too much camber (inside) or not enough (outside)
 * - Center wear: Over-inflation
 * - Flat spots: Caused by wheel lockups during braking
 *
 * AI and setup systems can use this to suggest adjustments.
 */
UENUM(BlueprintType)
enum class EMGTireWearPattern : uint8
{
	Even,        ///< Uniform wear across tread. Ideal - indicates good setup.
	InsideEdge,  ///< Heavy wear on inside edge. Too much negative camber.
	OutsideEdge, ///< Heavy wear on outside edge. Not enough negative camber or too much toe-out.
	Center,      ///< Heavy wear in center. Tire is over-inflated.
	Flat,        ///< Flat spot from wheel lockup. Causes vibration at speed.
	Cupping      ///< Scalloped wear pattern. Indicates suspension or alignment issues.
};

//=============================================================================
// TRACK SURFACE ENUMERATION
//=============================================================================

/**
 * @brief Type of surface the tire is currently in contact with
 *
 * Different surfaces dramatically affect grip and wear:
 * - Asphalt: Standard racing surface, best grip for slick tires
 * - Wet: Reduced grip, requires rain tires for best performance
 * - Off-track surfaces: Very low grip, high wear, slow vehicle down
 *
 * The subsystem uses this to modify grip calculations in real-time.
 */
UENUM(BlueprintType)
enum class EMGTrackSurface : uint8
{
	Asphalt,  ///< Standard racing surface. Best grip for racing compounds.
	Concrete, ///< Harder surface, slightly less grip than asphalt but durable.
	Gravel,   ///< Loose surface. Very low grip for road tires, needs off-road compound.
	Dirt,     ///< Compact earth. Variable grip depending on moisture.
	Grass,    ///< Very low grip. Causes rapid speed loss and potential spin.
	Sand,     ///< Extremely low grip. Can get stuck if too slow.
	Snow,     ///< Requires winter/studded tires for any grip.
	Ice,      ///< Minimal grip even with studded tires. Careful inputs required.
	Wet,      ///< Water on track surface. Grip depends heavily on tire compound.
	Puddle,   ///< Standing water. Risk of aquaplaning (loss of all grip).
	Oil       ///< Oil spill on track. Extremely slippery, very dangerous.
};

//=============================================================================
// TIRE COMPOUND DATA STRUCTURE
//=============================================================================

/**
 * @brief Configuration data defining a tire compound's characteristics
 *
 * Each tire compound (Soft, Medium, Hard, etc.) has unique properties that
 * determine its performance. This struct holds all the parameters that define
 * how a compound behaves.
 *
 * Key Concepts:
 * - BaseGrip: Starting grip level (1.0 = normal, higher = more grip)
 * - Temperature Window: Each compound works best in a specific temp range
 * - WearRate: How quickly the tire degrades (higher = faster wear)
 * - WetPerformance: How well the compound handles wet conditions
 *
 * Example: A Soft compound might have:
 * - BaseGrip = 1.2 (20% more grip than baseline)
 * - WearRate = 1.5 (wears 50% faster)
 * - OptimalTempMin/Max = 85-105C (narrower operating window)
 *
 * Configure these via data assets for easy balancing without code changes.
 */
USTRUCT(BlueprintType)
struct FMGTireCompoundData
{
	GENERATED_BODY()

	/// The enum type this configuration defines
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCompoundType CompoundType = EMGTireCompoundType::Medium;

	/// Unique identifier for this compound configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompoundID;

	/// Localized name shown in UI (e.g., "Soft", "Medium", "Hard")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Color used in HUD tire displays (e.g., Red=Soft, Yellow=Medium, White=Hard)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CompoundColor = FLinearColor::White;

	/// Base grip multiplier when tire is at optimal temperature and new (1.0 = baseline)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseGrip = 1.0f;

	/// Temperature (Celsius) where this compound provides maximum grip
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakGripTemperature = 90.0f;

	/// Minimum temperature for optimal grip window (below this = reduced grip)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalTempMin = 70.0f;

	/// Maximum temperature for optimal grip window (above this = overheating)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalTempMax = 110.0f;

	/// How quickly this compound wears (1.0 = normal, 1.5 = 50% faster wear)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WearRate = 1.0f;

	/// How quickly temperature rises when driving hard (1.0 = normal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatUpRate = 1.0f;

	/// How quickly temperature drops when driving conservatively (1.0 = normal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolDownRate = 1.0f;

	/// Grip multiplier in wet conditions (0.5 = 50% grip, 1.0 = full grip on wet tires)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WetPerformance = 0.5f;

	/// Overall durability - affects puncture resistance and wear consistency
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurabilityFactor = 1.0f;

	/// Lateral (sideways/cornering) grip modifier - affects turn-in and mid-corner grip
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralGripMod = 1.0f;

	/// Longitudinal (forward/backward) grip modifier - affects acceleration and braking
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongitudinalGripMod = 1.0f;

	/// Typical number of laps this compound lasts at average pace (for strategy planning)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExpectedLaps = 20;

	/// True if this compound works in all weather (AllSeason, Rally types)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllWeather = false;

	/// True if this compound has metal studs for ice/snow grip
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStudded = false;
};

//=============================================================================
// TIRE STATE STRUCTURE
//=============================================================================

/**
 * @brief Real-time state data for a single tire
 *
 * This struct contains all current values for one tire on a vehicle.
 * Updated every frame based on driving inputs and conditions.
 *
 * Key Values Explained:
 *
 * WEAR (0.0 to 1.0):
 * - 1.0 = brand new tire, full grip
 * - 0.5 = 50% worn, reduced grip
 * - 0.0 = completely worn (tire failure imminent)
 *
 * TEMPERATURE (Celsius):
 * - Cold (<70C): Less grip, tires need warming up
 * - Optimal (70-110C): Best grip range
 * - Hot (>110C): Overheating, increased wear, grip starts dropping
 *
 * GRIP (0.0 to 1.5+):
 * - Calculated from wear, temperature, compound, and surface
 * - 1.0 = normal grip, higher = more grip
 * - Fed to vehicle physics for handling calculations
 *
 * SLIP VALUES:
 * - SlipRatio: Difference between wheel rotation speed and car speed
 *   - 0.0 = no slip (rolling normally)
 *   - Positive = wheelspin (accelerating)
 *   - Negative = wheel lockup (braking)
 * - SlipAngle: Angle between tire direction and travel direction
 *   - Small angle = good cornering grip
 *   - Large angle = sliding/drifting
 */
USTRUCT(BlueprintType)
struct FMGTireState
{
	GENERATED_BODY()

	/// Which corner of the vehicle this tire is on
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTirePosition Position = EMGTirePosition::FrontLeft;

	/// Current compound installed on this tire
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCompoundType Compound = EMGTireCompoundType::Medium;

	/// Overall condition category (for UI/AI decisions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCondition Condition = EMGTireCondition::Optimal;

	/// How wear is distributed across the tire surface (indicates setup issues)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireWearPattern WearPattern = EMGTireWearPattern::Even;

	/// Remaining tire life (1.0 = new, 0.0 = worn out)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WearLevel = 1.0f;

	/// Current overall tire temperature in Celsius (average of surface and core)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 25.0f;

	/// Outer rubber surface temperature - changes quickly with driving
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceTemperature = 25.0f;

	/// Internal tire temperature - changes slowly, retains heat longer
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoreTemperature = 25.0f;

	/// Tire pressure in bar (affects grip and wear pattern)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pressure = 2.0f;

	/// Current overall grip coefficient (combined from all factors)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentGrip = 1.0f;

	/// Grip available for cornering/turning forces
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralGrip = 1.0f;

	/// Grip available for acceleration/braking forces
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongitudinalGrip = 1.0f;

	/// Current slip ratio - wheel speed vs vehicle speed (-1 to +1 typical range)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipRatio = 0.0f;

	/// Current slip angle in degrees - tire direction vs travel direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipAngle = 0.0f;

	/// Vertical load force on this tire in Newtons (weight + aero + transfer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoadForce = 0.0f;

	/// True if wheel is locked under braking (no rotation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLocked = false;

	/// True if wheel is spinning faster than ground speed (wheelspin)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpinning = false;

	/// True if tire has a puncture or blowout
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlat = false;

	/// Number of laps completed on this set of tires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapsOnTire = 0.0f;

	/// Total distance driven on this tire in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceOnTire = 0.0f;
};

//=============================================================================
// VEHICLE TIRE SET STRUCTURE
//=============================================================================

/**
 * @brief Complete tire state for all four wheels of a vehicle
 *
 * Groups all tire data for a single vehicle together for convenient access.
 * Includes computed averages useful for quick UI display and strategy decisions.
 *
 * Mixed Compounds:
 * Some race series allow different compounds front vs rear (e.g., Medium front,
 * Hard rear). The bMixedCompounds flag tracks this for rule checking.
 *
 * Note: In real motorsport, you typically cannot mix compounds left-to-right
 * (both fronts must match, both rears must match).
 */
USTRUCT(BlueprintType)
struct FMGVehicleTireSet
{
	GENERATED_BODY()

	/// Vehicle these tires belong to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Front left tire state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireState FrontLeft;

	/// Front right tire state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireState FrontRight;

	/// Rear left tire state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireState RearLeft;

	/// Rear right tire state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireState RearRight;

	/// Average wear across all four tires (1.0 = new, 0.0 = worn out)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageWear = 1.0f;

	/// Average temperature across all four tires in Celsius
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageTemperature = 25.0f;

	/// Average grip coefficient across all four tires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageGrip = 1.0f;

	/// True if front and rear have different compound types
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMixedCompounds = false;
};

//=============================================================================
// TIRE WEAR FACTORS STRUCTURE
//=============================================================================

/**
 * @brief Multipliers controlling how different actions cause tire wear
 *
 * These factors determine how quickly tires degrade based on driving style.
 * Aggressive driving (heavy braking, fast cornering, wheelspin) wears tires
 * faster than smooth, conservative driving.
 *
 * Useful for:
 * - Game balance tuning (make tires last longer/shorter)
 * - Difficulty settings (easier = lower wear rates)
 * - Track-specific adjustments (rough tracks = more wear)
 *
 * Each factor multiplies a base wear rate. 1.0 = normal, 2.0 = double wear.
 */
USTRUCT(BlueprintType)
struct FMGTireWearFactors
{
	GENERATED_BODY()

	/// Wear from accelerating hard (affects drive wheels primarily)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationWear = 1.0f;

	/// Wear from heavy braking (affects all tires, especially fronts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingWear = 1.0f;

	/// Wear from cornering forces (lateral load on tires during turns)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorneringWear = 1.0f;

	/// Wear from tire slip (sliding/drifting causes rapid wear)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipWear = 2.0f;

	/// Wear from wheel lockup under braking (creates flat spots)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockupWear = 3.0f;

	/// Wear from tire temperature (overheating damages rubber)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TemperatureWear = 1.0f;

	/// Wear from track surface type (gravel, kerbs cause more wear)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceWear = 1.0f;

	/// Wear from vertical load (heavy cars or high downforce = more wear)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoadWear = 1.0f;
};

//=============================================================================
// TIRE TEMPERATURE ZONE STRUCTURE
//=============================================================================

/**
 * @brief Temperature readings across the tire tread width
 *
 * Real tires don't heat uniformly - they have three temperature zones:
 * - Inner edge: Heats more during cornering (weight shifts to this side)
 * - Middle: General running temperature
 * - Outer edge: Can overheat with too much camber
 *
 * The temperature spread indicates setup quality:
 * - Low spread (even temps): Good setup, balanced wear
 * - High spread: Setup issues, uneven wear will occur
 *
 * Pro tip: If inner temps are much higher than outer, consider reducing
 * camber. If outer is hotter, increase camber or check toe settings.
 */
USTRUCT(BlueprintType)
struct FMGTireTemperatureZone
{
	GENERATED_BODY()

	/// Temperature of inner tread edge (Celsius) - wheel side
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InnerTemp = 25.0f;

	/// Temperature of center tread area (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MiddleTemp = 25.0f;

	/// Temperature of outer tread edge (Celsius) - body side
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OuterTemp = 25.0f;

	/// Average across all three zones (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageTemp = 25.0f;

	/// Difference between hottest and coolest zone (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TempSpread = 0.0f;

	/// True if any zone exceeds safe temperature limits
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverheating = false;

	/// True if tire is below optimal temperature window (cold tires = less grip)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUndercooled = false;
};

//=============================================================================
// TIRE TELEMETRY STRUCTURE
//=============================================================================

/**
 * @brief Historical tire data for analysis and feedback
 *
 * Tracks tire performance over time to help players and AI understand
 * driving patterns and improve technique.
 *
 * Uses:
 * - Post-race analysis (see where you abused tires most)
 * - AI learning (identify corners/sections causing most wear)
 * - Achievement tracking (e.g., "Complete race with no lockups")
 * - Setup feedback (high peak temps = setup or driving issue)
 */
USTRUCT(BlueprintType)
struct FMGTireTelemetry
{
	GENERATED_BODY()

	/// Vehicle this telemetry belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Periodic snapshots of tire state (for graphs/playback)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTireState> TireHistory;

	/// Highest temperature reached by front left tire this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakFrontLeftTemp = 0.0f;

	/// Highest temperature reached by front right tire this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakFrontRightTemp = 0.0f;

	/// Highest temperature reached by rear left tire this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakRearLeftTemp = 0.0f;

	/// Highest temperature reached by rear right tire this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakRearRightTemp = 0.0f;

	/// Number of wheel lockup events (brake too hard = flat spots)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Lockups = 0;

	/// Number of wheelspin events (accelerate too hard = lost traction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wheelspin = 0;

	/// Total distance traveled while tires were slipping (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalSlipDistance = 0.0f;
};

//=============================================================================
// TIRE SETTINGS STRUCTURE
//=============================================================================

/**
 * @brief Global configuration for tire simulation behavior
 *
 * Controls the realism level and difficulty of tire simulation.
 * These settings can be adjusted for different game modes:
 * - Arcade: Disable wear/temperature, increase grip
 * - Simulation: Full wear, temperature, puncture simulation
 * - Custom: Player-defined balance
 *
 * Also controls environmental conditions that affect all vehicles.
 */
USTRUCT(BlueprintType)
struct FMGTireSettings
{
	GENERATED_BODY()

	/// Master multiplier for all tire wear (0.5 = half wear rate)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalWearMultiplier = 1.0f;

	/// Master multiplier for all tire grip (1.2 = 20% more grip everywhere)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalGripMultiplier = 1.0f;

	/// Speed of temperature changes (2.0 = temps change twice as fast)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TemperatureSimSpeed = 1.0f;

	/// Air temperature in Celsius (affects tire cooling rate)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AmbientTemperature = 25.0f;

	/// Track surface temperature in Celsius (affects tire heating)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackTemperature = 35.0f;

	/// Enable tire pressure simulation (affects grip and wear pattern)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulatePressure = true;

	/// Enable tire temperature simulation (cold/hot tires affect grip)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateTemperature = true;

	/// Enable tire wear simulation (tires degrade over time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateWear = true;

	/// Allow random tire punctures from debris/wear
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPunctures = true;

	/// Base probability of puncture per update (very small number)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PunctureChance = 0.001f;

	/// Show temperature colors on tire HUD elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisualizeTemperature = true;

	/// Show wear level on tire HUD elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisualizeWear = true;
};

//=============================================================================
// DELEGATE DECLARATIONS
//=============================================================================

/**
 * Event Delegates allow other systems to react to tire events.
 *
 * HOW DELEGATES WORK (for entry-level developers):
 * Delegates are like "event hooks" - other code can subscribe to be notified
 * when something happens. For example, the HUD can subscribe to OnTireWearUpdated
 * to update the tire wear display whenever wear changes.
 *
 * Usage example:
 *   TireSubsystem->OnTirePunctured.AddDynamic(this, &AMyClass::HandlePuncture);
 *
 * Then your HandlePuncture function is called whenever a tire gets punctured.
 */

/// Fired when any tire's wear level changes significantly
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTireWearUpdated, FName, VehicleID, EMGTirePosition, Position, float, NewWear);

/// Fired when a tire's condition category changes (e.g., Good -> Worn)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTireConditionChanged, FName, VehicleID, EMGTirePosition, Position, EMGTireCondition, NewCondition);

/// Fired when a tire exceeds safe temperature limits
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTireTemperatureWarning, FName, VehicleID, EMGTirePosition, Position, float, Temperature);

/// Fired when a tire gets punctured - vehicle should pit immediately
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTirePunctured, FName, VehicleID, EMGTirePosition, Position);

/// Fired when a wheel locks up under braking (creates flat spot)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTireLockup, FName, VehicleID, EMGTirePosition, Position);

/// Fired when a wheel spins excessively during acceleration
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTireWheelspin, FName, VehicleID, EMGTirePosition, Position);

/// Fired when tires are changed (during pit stop)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTiresChanged, FName, VehicleID, EMGTireCompoundType, NewCompound);

/// Fired when grip level changes significantly (useful for AI/HUD updates)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGripChanged, FName, VehicleID, EMGTirePosition, Position, float, NewGrip);

//=============================================================================
// MAIN TIRE SUBSYSTEM CLASS
//=============================================================================

/**
 * @brief Game Instance Subsystem for tire simulation and management
 *
 * UMGTireSubsystem is the central authority for all tire-related operations
 * in Midnight Grind. It simulates realistic tire behavior including wear,
 * temperature, grip, and compound characteristics.
 *
 * =============================================================================
 * FOR ENTRY-LEVEL DEVELOPERS
 * =============================================================================
 *
 * This class inherits from UGameInstanceSubsystem, which means:
 * - Unreal creates ONE instance automatically when the game starts
 * - It persists across level loads (tire data survives track changes)
 * - Access it via: GetGameInstance()->GetSubsystem<UMGTireSubsystem>()
 *
 * TYPICAL WORKFLOW:
 * 1. When a car joins a race, call RegisterVehicle(CarID) to start tracking
 * 2. Every frame (or physics tick), call UpdateTireState() with driving data
 * 3. Query GetTireGrip() to get grip values for physics calculations
 * 4. During pit stops, call ChangeTires() to install fresh rubber
 * 5. When car leaves race, call UnregisterVehicle() to cleanup
 *
 * BLUEPRINT USAGE:
 * Most functions are marked BlueprintCallable or BlueprintPure, so you can:
 * - Call functions from Blueprint event graphs
 * - Query tire data for UI widgets
 * - Respond to tire events via delegate bindings
 *
 * CODE EXAMPLE:
 * @code
 * // Get the tire subsystem
 * UMGTireSubsystem* TireSys = GetGameInstance()->GetSubsystem<UMGTireSubsystem>();
 *
 * // Check if we should pit for new tires
 * float AvgWear = TireSys->GetAverageWear(VehicleID);
 * if (AvgWear < 0.3f) // Less than 30% tire remaining
 * {
 *     // Recommend pit stop
 *     ShowPitStopRecommendation();
 * }
 *
 * // Get grip for physics
 * float Grip = TireSys->GetTireGrip(VehicleID, EMGTirePosition::FrontLeft);
 * ApplyGripToWheel(Grip);
 * @endcode
 *
 * @see UMGPitStopSubsystem for tire change during pit stops
 * @see UMGFuelSubsystem for fuel management integration
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTireSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//=========================================================================
	// SUBSYSTEM LIFECYCLE
	//=========================================================================

	/** Called when game instance is created - initializes default tire compounds */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called when game instance is destroyed - cleanup */
	virtual void Deinitialize() override;

	/** Determines if this subsystem should be created - returns true for game builds */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//=========================================================================
	// VEHICLE REGISTRATION
	//=========================================================================
	// These functions manage which vehicles are being tracked by the tire system.
	// You must register a vehicle before querying or updating its tire data.

	/** Register a vehicle for tire tracking - call when car joins race */
	UFUNCTION(BlueprintCallable, Category = "Tire|State")
	void RegisterVehicle(FName VehicleID);

	/** Remove a vehicle from tire tracking - call when car leaves race */
	UFUNCTION(BlueprintCallable, Category = "Tire|State")
	void UnregisterVehicle(FName VehicleID);

	//=========================================================================
	// TIRE STATE QUERIES
	//=========================================================================
	// These "Pure" functions query current tire state without modifying anything.
	// Safe to call frequently (every frame) for UI updates or physics.

	/** Get complete state data for one tire */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	FMGTireState GetTireState(FName VehicleID, EMGTirePosition Position) const;

	/** Get all four tires' state for a vehicle */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	FMGVehicleTireSet GetVehicleTires(FName VehicleID) const;

	/** Get wear level for one tire (1.0 = new, 0.0 = worn out) */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetTireWear(FName VehicleID, EMGTirePosition Position) const;

	/** Get temperature for one tire in Celsius */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetTireTemperature(FName VehicleID, EMGTirePosition Position) const;

	/** Get current grip coefficient for one tire (feed to physics) */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetTireGrip(FName VehicleID, EMGTirePosition Position) const;

	/** Get tire pressure in bar */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetTirePressure(FName VehicleID, EMGTirePosition Position) const;

	/** Get overall condition category for one tire */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	EMGTireCondition GetTireCondition(FName VehicleID, EMGTirePosition Position) const;

	/** Get average wear across all four tires (quick strategy check) */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetAverageWear(FName VehicleID) const;

	/** Get average grip across all four tires */
	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetAverageGrip(FName VehicleID) const;

	//=========================================================================
	// TIRE STATE UPDATES
	//=========================================================================
	// These functions modify tire state based on driving inputs.
	// Call from physics simulation or vehicle tick.

	/**
	 * Main update function - call every physics tick with current driving data.
	 * @param VehicleID The vehicle to update
	 * @param Position Which tire to update
	 * @param SlipRatio Wheel speed vs vehicle speed difference (-1 to +1)
	 * @param SlipAngle Angle between tire heading and travel direction (degrees)
	 * @param Load Vertical force on tire in Newtons
	 * @param Speed Vehicle speed in m/s
	 */
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void UpdateTireState(FName VehicleID, EMGTirePosition Position, float SlipRatio, float SlipAngle, float Load, float Speed);

	/** Manually apply wear to a tire (e.g., from debris hit) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ApplyWear(FName VehicleID, EMGTirePosition Position, float WearAmount);

	/** Add heat to a tire (e.g., from hard braking) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ApplyHeat(FName VehicleID, EMGTirePosition Position, float HeatAmount);

	/** Cool a tire down (e.g., from straight-line cruising) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void CoolTire(FName VehicleID, EMGTirePosition Position, float CoolAmount);

	/** Set tire pressure manually (pit stop adjustment) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void SetTirePressure(FName VehicleID, EMGTirePosition Position, float Pressure);

	/** Report a wheel lockup event - causes flat spot and wear */
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ReportLockup(FName VehicleID, EMGTirePosition Position);

	/** Report excessive wheelspin - causes temperature spike and wear */
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ReportWheelspin(FName VehicleID, EMGTirePosition Position);

	/** Report what surface the tire is on - affects grip and wear calculations */
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ReportSurfaceContact(FName VehicleID, EMGTirePosition Position, EMGTrackSurface Surface);

	//=========================================================================
	// TIRE CHANGE OPERATIONS
	//=========================================================================
	// Functions for changing tires during pit stops.
	// These reset wear and temperature to fresh tire values.

	/** Change all four tires to a new compound (full tire change) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void ChangeTires(FName VehicleID, EMGTireCompoundType NewCompound);

	/** Change just one tire (e.g., puncture repair with spare) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void ChangeSingleTire(FName VehicleID, EMGTirePosition Position, EMGTireCompoundType NewCompound);

	/** Change front tires only (front axle change) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void ChangeFrontTires(FName VehicleID, EMGTireCompoundType NewCompound);

	/** Change rear tires only (rear axle change) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void ChangeRearTires(FName VehicleID, EMGTireCompoundType NewCompound);

	/** Repair a punctured tire - restores it to usable state but with existing wear */
	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void PunctureRepair(FName VehicleID, EMGTirePosition Position);

	//=========================================================================
	// COMPOUND INFORMATION
	//=========================================================================
	// Query and configure tire compound data.
	// Compounds can be customized via data assets or at runtime.

	/** Register/update a tire compound configuration */
	UFUNCTION(BlueprintCallable, Category = "Tire|Compound")
	void RegisterCompound(const FMGTireCompoundData& CompoundData);

	/** Get configuration data for a specific compound type */
	UFUNCTION(BlueprintPure, Category = "Tire|Compound")
	FMGTireCompoundData GetCompoundData(EMGTireCompoundType CompoundType) const;

	/** Get all registered compound configurations */
	UFUNCTION(BlueprintPure, Category = "Tire|Compound")
	TArray<FMGTireCompoundData> GetAllCompounds() const;

	/** Get AI/strategy recommendation for best compound based on conditions */
	UFUNCTION(BlueprintPure, Category = "Tire|Compound")
	EMGTireCompoundType GetRecommendedCompound(float TrackTemp, bool bWet) const;

	/** Get expected lap count for a compound at average pace */
	UFUNCTION(BlueprintPure, Category = "Tire|Compound")
	int32 GetExpectedTireLaps(EMGTireCompoundType CompoundType) const;

	//=========================================================================
	// GRIP CALCULATIONS
	//=========================================================================
	// Low-level grip calculation functions.
	// Useful for understanding how grip is computed or for custom physics.

	/** Calculate total grip from tire state and compound data */
	UFUNCTION(BlueprintPure, Category = "Tire|Grip")
	float CalculateGrip(const FMGTireState& TireState, const FMGTireCompoundData& Compound) const;

	/** Get grip multiplier based on tire temperature vs optimal range */
	UFUNCTION(BlueprintPure, Category = "Tire|Grip")
	float GetGripFromTemperature(float Temperature, const FMGTireCompoundData& Compound) const;

	/** Get grip multiplier based on wear level (worn tires = less grip) */
	UFUNCTION(BlueprintPure, Category = "Tire|Grip")
	float GetGripFromWear(float WearLevel) const;

	/** Get grip multiplier for a surface type with a given compound */
	UFUNCTION(BlueprintPure, Category = "Tire|Grip")
	float GetSurfaceGripMultiplier(EMGTrackSurface Surface, EMGTireCompoundType Compound) const;

	// =========================================================================
	// SURFACE INTEGRATION (Added in Iteration 95)
	// =========================================================================
	// Functions that connect track surface detection to tire physics.
	// Uses the MGTrackSubsystem's GetSurfaceAtPosition for real-time surface detection.

	/**
	 * @brief Calculate complete tire grip at a world position including surface modifier
	 *
	 * This function combines:
	 * - Base tire grip from compound, temperature, and wear
	 * - Surface grip modifier from track surface detection
	 *
	 * @param VehicleID The vehicle to calculate grip for
	 * @param Position Which tire position (FL, FR, RL, RR)
	 * @param WorldPosition The world location of the tire contact patch
	 * @return Complete grip value including surface modifier
	 */
	UFUNCTION(BlueprintPure, Category = "Tire|Grip|Surface")
	float CalculateGripAtPosition(FName VehicleID, EMGTirePosition Position, FVector WorldPosition) const;

	/**
	 * @brief Get the current surface type at a world position via track subsystem
	 *
	 * Delegates to UMGTrackSubsystem::GetSurfaceAtPosition for surface detection.
	 * Returns Asphalt as default if track subsystem is unavailable.
	 *
	 * @param WorldPosition Position to query
	 * @return Surface type at that position
	 */
	UFUNCTION(BlueprintPure, Category = "Tire|Grip|Surface")
	EMGTrackSurface GetSurfaceTypeAtPosition(FVector WorldPosition) const;

	/**
	 * @brief Update all tire grips for a vehicle based on current surface
	 *
	 * Call this each frame with tire contact positions to update surface-based grip.
	 * Automatically handles surface transitions (e.g., going from asphalt to grass).
	 *
	 * @param VehicleID The vehicle to update
	 * @param FLPosition Front Left tire world position
	 * @param FRPosition Front Right tire world position
	 * @param RLPosition Rear Left tire world position
	 * @param RRPosition Rear Right tire world position
	 */
	UFUNCTION(BlueprintCallable, Category = "Tire|Grip|Surface")
	void UpdateSurfaceGrip(FName VehicleID, FVector FLPosition, FVector FRPosition, FVector RLPosition, FVector RRPosition);

	//=========================================================================
	// TEMPERATURE MONITORING
	//=========================================================================
	// Functions for detailed temperature analysis.
	// Used for setup optimization and temperature-aware driving.

	/** Get detailed temperature readings across tire width (inner/middle/outer) */
	UFUNCTION(BlueprintPure, Category = "Tire|Temperature")
	FMGTireTemperatureZone GetTireTemperatureZones(FName VehicleID, EMGTirePosition Position) const;

	/** Check if tire is overheating (causes increased wear and grip loss) */
	UFUNCTION(BlueprintPure, Category = "Tire|Temperature")
	bool IsTireOverheating(FName VehicleID, EMGTirePosition Position) const;

	/** Check if tire is too cold (reduced grip until warmed up) */
	UFUNCTION(BlueprintPure, Category = "Tire|Temperature")
	bool IsTireCold(FName VehicleID, EMGTirePosition Position) const;

	/** Check if tire is in optimal temperature window (best grip) */
	UFUNCTION(BlueprintPure, Category = "Tire|Temperature")
	bool IsTireInOptimalWindow(FName VehicleID, EMGTirePosition Position) const;

	//=========================================================================
	// TELEMETRY DATA
	//=========================================================================
	// Historical tire data for analysis and feedback.

	/** Get complete telemetry history for a vehicle */
	UFUNCTION(BlueprintPure, Category = "Tire|Telemetry")
	FMGTireTelemetry GetTireTelemetry(FName VehicleID) const;

	/** Take a telemetry snapshot (call periodically or at key moments) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Telemetry")
	void RecordTelemetry(FName VehicleID);

	/** Clear telemetry history (e.g., at session start) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Telemetry")
	void ClearTelemetry(FName VehicleID);

	//=========================================================================
	// SETTINGS MANAGEMENT
	//=========================================================================
	// Configure global tire simulation behavior.

	/** Update global tire simulation settings */
	UFUNCTION(BlueprintCallable, Category = "Tire|Settings")
	void SetTireSettings(const FMGTireSettings& NewSettings);

	/** Get current global tire settings */
	UFUNCTION(BlueprintPure, Category = "Tire|Settings")
	FMGTireSettings GetTireSettings() const { return Settings; }

	/** Update wear factor multipliers */
	UFUNCTION(BlueprintCallable, Category = "Tire|Settings")
	void SetWearFactors(const FMGTireWearFactors& Factors);

	/** Get current wear factor configuration */
	UFUNCTION(BlueprintPure, Category = "Tire|Settings")
	FMGTireWearFactors GetWearFactors() const { return WearFactors; }

	/** Set track surface temperature (affects tire heating) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Settings")
	void SetTrackTemperature(float Temperature);

	/** Set air temperature (affects tire cooling) */
	UFUNCTION(BlueprintCallable, Category = "Tire|Settings")
	void SetAmbientTemperature(float Temperature);

	//=========================================================================
	// STRATEGY PREDICTION
	//=========================================================================
	// AI/strategy helper functions for pit stop planning.

	/** Predict how many laps current tires can last */
	UFUNCTION(BlueprintPure, Category = "Tire|Prediction")
	int32 PredictTireLapsRemaining(FName VehicleID) const;

	/** Predict wear level after driving N more laps */
	UFUNCTION(BlueprintPure, Category = "Tire|Prediction")
	float PredictWearAfterLaps(FName VehicleID, int32 Laps) const;

	/** Get strategy recommendation: should we pit for tires? */
	UFUNCTION(BlueprintPure, Category = "Tire|Prediction")
	bool ShouldChangeTires(FName VehicleID, int32 RemainingLaps) const;

	//=========================================================================
	// EVENT DELEGATES
	//=========================================================================
	// Subscribe to these to react to tire events in your code/Blueprints.
	// Example: TireSubsystem->OnTirePunctured.AddDynamic(this, &MyClass::OnPuncture);

	/** Broadcast when tire wear changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireWearUpdated OnTireWearUpdated;

	/** Broadcast when tire condition category changes (Good -> Worn, etc.) */
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireConditionChanged OnTireConditionChanged;

	/** Broadcast when tire exceeds safe temperature */
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireTemperatureWarning OnTireTemperatureWarning;

	/** Broadcast when a tire is punctured */
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTirePunctured OnTirePunctured;

	/** Broadcast when wheel locks up under braking */
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireLockup OnTireLockup;

	/** Broadcast when excessive wheelspin occurs */
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireWheelspin OnTireWheelspin;

	/** Broadcast when tires are changed */
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTiresChanged OnTiresChanged;

	/** Broadcast when grip level changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnGripChanged OnGripChanged;

protected:
	//=========================================================================
	// INTERNAL IMPLEMENTATION
	//=========================================================================
	// These functions are called internally and should not be called directly.
	// They're protected so derived classes could override if needed.

	/** Timer callback - called periodically to update all tire state */
	void OnTireTick();

	/** Update all registered vehicles' tire states */
	void UpdateAllTires(float DeltaTime);

	/** Update temperature for a single tire based on driving */
	void UpdateTireTemperature(FMGTireState& Tire, const FMGTireCompoundData& Compound, float DeltaTime);

	/** Update wear for a single tire based on driving */
	void UpdateTireWear(FMGTireState& Tire, const FMGTireCompoundData& Compound, float DeltaTime);

	/** Recalculate grip values for a tire after state changes */
	void UpdateTireGrip(FMGTireState& Tire, const FMGTireCompoundData& Compound);

	/** Random puncture check based on wear and settings */
	void CheckForPuncture(FName VehicleID, FMGTireState& Tire);

	/** Determine condition category from wear level */
	EMGTireCondition CalculateCondition(float WearLevel, bool bFlat) const;

	/** Create default compound configurations at startup */
	void InitializeDefaultCompounds();

	/** Persist tire data to save game (for career mode) */
	void SaveTireData();

	/** Load tire data from save game */
	void LoadTireData();

	//=========================================================================
	// INTERNAL DATA STORAGE
	//=========================================================================

	/// Current tire state for all registered vehicles (keyed by VehicleID)
	UPROPERTY()
	TMap<FName, FMGVehicleTireSet> VehicleTires;

	/// Configuration data for all compound types
	UPROPERTY()
	TMap<EMGTireCompoundType, FMGTireCompoundData> CompoundDatabase;

	/// Telemetry history for all vehicles
	UPROPERTY()
	TMap<FName, FMGTireTelemetry> VehicleTelemetry;

	/// Global tire simulation settings
	UPROPERTY()
	FMGTireSettings Settings;

	/// Wear multiplier configuration
	UPROPERTY()
	FMGTireWearFactors WearFactors;

	/// Timer handle for periodic tire updates
	FTimerHandle TireTickHandle;
};

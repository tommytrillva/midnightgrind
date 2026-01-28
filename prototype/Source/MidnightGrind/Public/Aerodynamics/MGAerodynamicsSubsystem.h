// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * =============================================================================
 * MGAerodynamicsSubsystem.h
 * Advanced Vehicle Aerodynamics System for Midnight Grind Racing Game
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem simulates realistic vehicle aerodynamics, managing how air
 * interacts with vehicles during racing. It calculates forces that affect
 * vehicle behavior based on real physics principles.
 *
 * KEY AERODYNAMIC CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------------------
 *
 * 1. DRAG (Air Resistance):
 *    - Force that opposes a vehicle's motion through air
 *    - Increases with the SQUARE of speed (2x speed = 4x drag!)
 *    - Limits top speed - at some point, drag equals engine power
 *    - Measured by "drag coefficient" (Cd) - lower = more aerodynamic
 *    - Formula: Drag = 0.5 * AirDensity * Speed^2 * DragCoefficient * FrontalArea
 *
 * 2. DOWNFORCE:
 *    - Aerodynamic force that pushes the car DOWN onto the track
 *    - Created by wings, spoilers, and body shape
 *    - More downforce = more grip = faster cornering
 *    - BUT: More downforce usually means more drag (trade-off!)
 *    - Formula similar to drag but pushes vertically
 *
 * 3. LIFT (usually unwanted):
 *    - Force that can make a car "lighter" at high speeds
 *    - Can cause instability or loss of grip
 *    - Race cars are designed to have NEGATIVE lift (= downforce)
 *
 * 4. SLIPSTREAM/DRAFTING (also handled here):
 *    - Following behind another car reduces YOUR drag
 *    - The lead car "punches a hole" in the air
 *    - Can gain 5-15% speed advantage when drafting
 *    - Used strategically for overtaking ("slingshot" move)
 *
 * 5. WIND EFFECTS:
 *    - Headwind: Slows you down (increases effective drag)
 *    - Tailwind: Speeds you up (decreases effective drag)
 *    - Crosswind: Can push car sideways, affects stability
 *    - Gusts/Turbulence: Unpredictable variations
 *
 * REAL-WORLD EXAMPLES:
 * -------------------
 * - Formula 1 cars: High downforce (3-5x car weight at top speed!)
 * - Le Mans cars: Balanced for high speed straights + fast corners
 * - NASCAR: Low drag for top speed, draft-heavy racing
 * - Street cars: Prioritize low drag for fuel efficiency
 *
 * THE DRAG vs DOWNFORCE TRADE-OFF:
 * --------------------------------
 * This is the fundamental challenge in racing aerodynamics:
 *
 *   HIGH DOWNFORCE SETUP:
 *   + Faster in corners (more grip)
 *   + Better braking stability
 *   - Slower top speed
 *   - Uses more fuel
 *   Best for: Twisty tracks with lots of corners
 *
 *   LOW DRAG SETUP:
 *   + Higher top speed
 *   + Better fuel efficiency
 *   - Slower in corners
 *   - Less stable under braking
 *   Best for: Tracks with long straights
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 * - This is a UGameInstanceSubsystem (persists across levels/maps)
 * - Works alongside MGSlipstreamSubsystem for drafting mechanics
 * - Vehicles register their aero profiles on spawn
 * - Every frame, forces are calculated and applied to vehicle physics
 * - Spoilers/wings can be adjusted for different setups
 * - Wind zones can be placed in levels for varied conditions
 *
 * USAGE EXAMPLE:
 * --------------
 *   // Get the subsystem (available globally via game instance)
 *   UMGAerodynamicsSubsystem* Aero = GetGameInstance()->GetSubsystem<UMGAerodynamicsSubsystem>();
 *
 *   // Register a vehicle with an aero profile
 *   Aero->RegisterVehicle("Player1_Car", "HighDownforce_Profile");
 *
 *   // Each frame, update and get forces
 *   Aero->UpdateVehicleAero("Player1_Car", Position, Velocity, DeltaTime);
 *   FVector AeroForce = Aero->CalculateTotalAeroForce("Player1_Car", Velocity);
 *
 *   // Apply force to physics (in your vehicle pawn)
 *   VehicleMesh->AddForce(AeroForce);
 *
 * @see UMGSlipstreamSubsystem - Dedicated drafting/slipstream system
 * @see UMGNitroBoostSubsystem - Nitro boost that interacts with aero
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAerodynamicsSubsystem.generated.h"

//=============================================================================
// ENUMERATIONS
// These define the discrete types and states used by the aero system
//=============================================================================

/**
 * Predefined aerodynamic profile types.
 *
 * Each profile represents a different philosophy for balancing
 * drag vs downforce. Think of these like "presets" that players
 * can choose based on track characteristics.
 *
 * Designers can create custom profiles, but these provide
 * standard archetypes that players understand.
 */
UENUM(BlueprintType)
enum class EMGAeroProfile : uint8
{
	Standard			UMETA(DisplayName = "Standard"),       ///< Jack-of-all-trades, master of none
	LowDrag				UMETA(DisplayName = "Low Drag"),       ///< Maximum top speed, minimal downforce
	HighDownforce		UMETA(DisplayName = "High Downforce"), ///< Maximum grip, sacrifices top speed
	Balanced			UMETA(DisplayName = "Balanced"),       ///< Good compromise for mixed tracks
	SpeedFocused		UMETA(DisplayName = "Speed Focused"),  ///< Prioritizes straight-line speed
	GripFocused			UMETA(DisplayName = "Grip Focused"),   ///< Prioritizes cornering grip
	DriftOptimized		UMETA(DisplayName = "Drift Optimized"),///< Tuned for controlled sliding
	TopSpeed			UMETA(DisplayName = "Top Speed")       ///< Absolute minimum drag for max speed
};

/**
 * Current state of slipstream/drafting for a vehicle.
 *
 * This tracks the progression of a drafting maneuver:
 * None -> Entering -> Active -> Optimal -> (Slingshot or Exiting)
 *
 * The state machine helps with:
 * - Smooth transitions for visual effects
 * - Proper timing for slingshot activation
 * - UI feedback showing drafting progress
 */
UENUM(BlueprintType)
enum class EMGSlipstreamState : uint8
{
	None				UMETA(DisplayName = "None"),     ///< Not in any slipstream
	Entering			UMETA(DisplayName = "Entering"), ///< Just entered, effect ramping up
	Active				UMETA(DisplayName = "Active"),   ///< Fully in slipstream, receiving benefit
	Optimal				UMETA(DisplayName = "Optimal"),  ///< Perfect position, maximum benefit
	Exiting				UMETA(DisplayName = "Exiting"),  ///< Leaving slipstream, effect fading
	Slingshot			UMETA(DisplayName = "Slingshot") ///< Executing slingshot overtake maneuver
};

/**
 * Type of wind effect currently affecting a vehicle.
 *
 * Wind adds another layer of realism and gameplay variety:
 * - Headwind: Coming from the front, increases drag, reduces top speed
 * - Tailwind: Coming from behind, decreases effective drag, boosts speed
 * - Crosswind: From the side, pushes car laterally, affects handling
 * - Gust: Sudden burst of wind, temporary effect
 * - Turbulence: Unpredictable wind variations
 *
 * Different tracks/weather can have different wind conditions,
 * adding strategic depth (e.g., timing overtakes for tailwind zones).
 */
UENUM(BlueprintType)
enum class EMGWindEffect : uint8
{
	None				UMETA(DisplayName = "None"),       ///< No significant wind
	Headwind			UMETA(DisplayName = "Headwind"),   ///< Wind opposing motion
	Tailwind			UMETA(DisplayName = "Tailwind"),   ///< Wind assisting motion
	Crosswind			UMETA(DisplayName = "Crosswind"),  ///< Wind from the side
	Gust				UMETA(DisplayName = "Gust"),       ///< Sudden wind burst
	Turbulence			UMETA(DisplayName = "Turbulence")  ///< Chaotic wind variations
};

/**
 * Types of body kits that affect aerodynamics.
 *
 * Body kits are visual AND functional modifications that change
 * a car's aerodynamic properties. Each has different characteristics:
 *
 * - Stock: Factory original, baseline performance
 * - StreetRacer: Subtle improvements, still street-legal look
 * - TrackDay: More aggressive, meant for weekend track use
 * - TimeAttack: Maximum downforce, all-out track attack
 * - DriftSpec: Designed for drift angles, manages airflow differently
 * - WidebodyKit: Wider body allows larger tires + different aero
 * - Canards: Small fins on front bumper, fine-tune front downforce
 * - GTWing: Large rear wing, significant downforce + drag
 * - Custom: Player-designed, mixed characteristics
 *
 * These affect both the visual appearance and the aero coefficients.
 */
UENUM(BlueprintType)
enum class EMGBodyKitType : uint8
{
	Stock				UMETA(DisplayName = "Stock"),        ///< Factory original
	StreetRacer			UMETA(DisplayName = "Street Racer"), ///< Mild aero improvements
	TrackDay			UMETA(DisplayName = "Track Day"),    ///< Track-focused upgrades
	TimeAttack			UMETA(DisplayName = "Time Attack"),  ///< Maximum downforce setup
	DriftSpec			UMETA(DisplayName = "Drift Spec"),   ///< Drift-optimized aero
	WidebodyKit			UMETA(DisplayName = "Widebody Kit"), ///< Wider body, wider tires
	Canards				UMETA(DisplayName = "Canards"),      ///< Front aero fins
	GTWing				UMETA(DisplayName = "GT Wing"),      ///< Large rear wing
	Custom				UMETA(DisplayName = "Custom")        ///< Player customized
};

//=============================================================================
// DATA STRUCTURES - RUNTIME STATE
// These structs hold current aerodynamic state calculated each frame
//=============================================================================

/**
 * Current aerodynamic state for a vehicle.
 *
 * This struct is recalculated every frame and contains all the forces
 * and effects currently acting on the vehicle. Use this for:
 * - Applying forces to vehicle physics
 * - Displaying aero telemetry on HUD
 * - Debugging aerodynamic behavior
 *
 * THE PHYSICS BEHIND THE VALUES:
 * All forces are calculated using the standard aerodynamic equation:
 *   Force = 0.5 * AirDensity * Speed^2 * Coefficient * Area
 *
 * This is why speed is so important - doubling speed quadruples the forces!
 */
USTRUCT(BlueprintType)
struct FMGVehicleAeroState
{
	GENERATED_BODY()

	/// Unique identifier for this vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	/// Current speed in game units per second.
	/// All aero forces scale with speed squared.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	/// Current drag force in Newtons (opposing motion).
	/// Apply this as a force opposite to velocity direction.
	/// Higher = more air resistance = lower top speed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragForce = 0.0f;

	/// Current lift force (usually negative = downforce).
	/// Positive lift is bad - makes car "lighter" and less stable.
	/// Race cars have negative lift (downforce).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LiftForce = 0.0f;

	/// Total downforce pushing car onto track (Newtons).
	/// More downforce = more grip = faster cornering.
	/// This is the sum of front and rear downforce.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceTotal = 0.0f;

	/// Downforce on front axle specifically.
	/// Affects front tire grip and steering response.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceFront = 0.0f;

	/// Downforce on rear axle specifically.
	/// Affects rear tire grip and stability under acceleration.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceRear = 0.0f;

	/// Current air density at vehicle location (kg/m^3).
	/// Standard sea level = 1.225 kg/m^3.
	/// Affected by altitude, temperature, humidity.
	/// Higher density = more aero forces (both drag AND downforce).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirDensity = 1.225f;

	/// Effective drag coefficient after all modifiers.
	/// Includes slipstream reduction, upgrades, etc.
	/// Typical values: 0.25 (sleek sports car) to 0.40 (boxy car).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectiveDragCoefficient = 0.3f;

	/// Effective downforce coefficient after all modifiers.
	/// Higher values mean more downforce for a given speed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectiveDownforceCoefficient = 0.5f;

	/// Current slipstream/drafting state.
	/// @see EMGSlipstreamState for state descriptions.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSlipstreamState SlipstreamState = EMGSlipstreamState::None;

	/// Speed bonus from slipstream (0.0 to ~0.15 typically).
	/// Add this to your speed calculation: FinalSpeed = BaseSpeed * (1 + SlipstreamBonus)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipstreamBonus = 0.0f;

	/// Slingshot charge level (0.0 to 1.0).
	/// When this reaches 1.0, slingshot can be activated.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipstreamCharge = 0.0f;

	/// What type of wind is currently affecting this vehicle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWindEffect CurrentWindEffect = EMGWindEffect::None;

	/// Wind force vector being applied.
	/// Add this directly to your vehicle's forces.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WindForce = FVector::ZeroVector;

	/// Multiplier for tire grip based on current downforce.
	/// 1.0 = normal grip, >1.0 = enhanced grip from downforce.
	/// Apply to your tire friction calculations.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GripMultiplier = 1.0f;

	/// Multiplier for top speed based on current drag.
	/// 1.0 = normal top speed, <1.0 = reduced due to drag.
	/// Apply to your speed limiter calculations.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedMultiplier = 1.0f;
};

//=============================================================================
// DATA STRUCTURES - CONFIGURATION
// These structs define vehicle aerodynamic properties
//=============================================================================

/**
 * Definition of an aerodynamic profile.
 *
 * Profiles are "presets" that define a vehicle's aerodynamic characteristics.
 * Designers create profiles, and vehicles are assigned profiles.
 * Multiple vehicles can share the same profile.
 *
 * UNDERSTANDING THE COEFFICIENTS:
 * - DragCoefficient (Cd): How "slippery" the car is through air
 *   Real-world examples: Tesla Model S = 0.24, Hummer H2 = 0.57
 * - LiftCoefficient (Cl): Tendency to lift off ground (usually low/negative)
 * - DownforceCoefficient: How much downward force is generated
 *
 * BALANCING PROFILES:
 * Good profiles require trade-offs. If you increase downforce, you should
 * also increase drag. If you reduce drag, reduce downforce too.
 * This creates meaningful player choices.
 */
USTRUCT(BlueprintType)
struct FMGAeroProfileDefinition
{
	GENERATED_BODY()

	/// Unique identifier for this profile (e.g., "HighDownforce_V1")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProfileId;

	/// Player-friendly name shown in UI (e.g., "High Downforce")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Which preset category this profile belongs to.
	/// Used for UI grouping and AI profile selection.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAeroProfile Type = EMGAeroProfile::Standard;

	/// Drag coefficient - how much air resistance the car creates.
	/// Lower = less drag = higher top speed.
	/// Range: 0.20 (extremely aerodynamic) to 0.50 (boxy/draggy).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragCoefficient = 0.30f;

	/// Lift coefficient - tendency to lift off ground.
	/// Should be low or negative for race cars.
	/// Positive values cause instability at high speed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LiftCoefficient = 0.10f;

	/// Downforce coefficient - how much downward force is generated.
	/// Higher = more grip but also more drag.
	/// Range: 0.0 (no aero) to 2.0+ (extreme downforce like F1).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceCoefficient = 0.50f;

	/// Frontal area in square meters.
	/// The "shadow" the car casts when viewed from front.
	/// Larger area = more drag at same coefficient.
	/// Typical: 1.5 (sports car) to 2.5 (truck/SUV).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontalArea = 2.0f;

	/// What percentage of downforce goes to front axle.
	/// 0.5 = 50% front, 50% rear (balanced).
	/// <0.5 = rear-biased, >0.5 = front-biased.
	/// Affects handling balance - more rear = oversteer tendency.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceFrontBias = 0.45f;

	/// How this profile affects top speed (multiplier).
	/// 1.0 = neutral, <1.0 = reduces top speed, >1.0 = increases.
	/// Low drag profiles should have values > 1.0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedEffect = 1.0f;

	/// How this profile affects cornering grip (multiplier).
	/// 1.0 = neutral, >1.0 = more grip in corners.
	/// High downforce profiles should have values > 1.0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorneringGripEffect = 1.0f;

	/// How this profile affects braking stability (multiplier).
	/// 1.0 = neutral, >1.0 = more stable under heavy braking.
	/// Rear-biased downforce improves braking stability.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingStabilityEffect = 1.0f;

	/// How well this profile benefits from slipstreaming (multiplier).
	/// 1.0 = normal, >1.0 = gets more benefit from drafting.
	/// Low-drag cars typically benefit more from drafting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipstreamEffectiveness = 1.0f;
};

/**
 * A slipstream session tracking one vehicle drafting another.
 *
 * When a vehicle enters another's slipstream, a session is created
 * to track the ongoing drafting relationship. The session ends when
 * the follower leaves the slipstream or completes a slingshot.
 *
 * Sessions are used for:
 * - Tracking drafting duration for scoring/achievements
 * - Managing slingshot charge buildup
 * - Statistics and post-race analysis
 * - AI decision making about when to attempt overtakes
 */
USTRUCT(BlueprintType)
struct FMGSlipstreamSession
{
	GENERATED_BODY()

	/// Unique identifier for this specific drafting session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionId;

	/// ID of the vehicle doing the drafting (behind)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FollowerVehicleId;

	/// ID of the vehicle being drafted (in front)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LeaderVehicleId;

	/// Current state of this drafting session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSlipstreamState State = EMGSlipstreamState::None;

	/// Current distance between vehicles (in Unreal units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	/// How long (seconds) this session has been active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	/// Current drag reduction percentage (0.0 to ~0.40)
	/// 0.40 = 40% less drag while drafting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragReduction = 0.0f;

	/// Current speed bonus from drafting (0.0 to ~0.15)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonus = 0.0f;

	/// Slingshot charge level (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeLevel = 0.0f;

	/// How close to slingshot ready (same as ChargeLevel, for clarity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotReady = 0.0f;

	/// True if currently in optimal drafting position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOptimal = false;

	/// When this session started (for duration calculation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;
};

//=============================================================================
// DATA STRUCTURES - WORLD ELEMENTS
// Wind zones that can be placed in the game world
//=============================================================================

/**
 * Definition of a wind zone in the game world.
 *
 * Wind zones are placed by level designers to create areas with
 * specific wind conditions. Vehicles passing through experience
 * the wind effects, adding gameplay variety and realism.
 *
 * USE CASES:
 * - Coastal sections with crosswinds
 * - Tunnel exits with sudden gusts
 * - Open highways with headwinds/tailwinds
 * - Mountain passes with turbulence
 *
 * Place wind zones using a Blueprint Actor that registers with this system.
 */
USTRUCT(BlueprintType)
struct FMGWindZone
{
	GENERATED_BODY()

	/// Unique identifier for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ZoneId;

	/// World position of the zone center
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	/// Radius of effect in Unreal units (cm)
	/// Wind effect may fade at edges
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 5000.0f;

	/// Direction the wind is blowing (normalized vector)
	/// Wind flows FROM this direction toward vehicles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WindDirection = FVector::ForwardVector;

	/// Base wind speed in units per second
	/// Higher = stronger wind effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindSpeed = 0.0f;

	/// How often gusts occur (gusts per second)
	/// 0 = no gusts, steady wind
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GustFrequency = 0.0f;

	/// How much stronger gusts are vs base wind (multiplier)
	/// 1.5 = gusts are 50% stronger than base
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GustIntensity = 0.0f;

	/// Amount of random variation in wind (0-1)
	/// 0 = perfectly steady, 1 = very chaotic
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurbulenceLevel = 0.0f;

	/// Whether wind in this zone disrupts slipstreaming
	/// Strong crosswinds can break drafting connections
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsSlipstream = true;
};

/**
 * Configuration for a vehicle's spoiler/wing.
 *
 * Spoilers are adjustable aerodynamic devices on vehicles.
 * They can be adjusted to change the balance between downforce
 * and drag, allowing players to tune their car for different tracks.
 *
 * HOW SPOILERS WORK:
 * - Spoiler angle determines how much air is deflected downward
 * - More angle = more downforce = more grip but also more drag
 * - Less angle = less drag = higher top speed but less grip
 * - Width and height affect the amount of air the spoiler catches
 *
 * ACTIVE/ADJUSTABLE SPOILERS:
 * Some high-end cars have spoilers that automatically adjust:
 * - Raise at high speeds for more downforce
 * - Lower/retract at low speeds to reduce drag
 * - This is simulated with bAutoAdjusting = true
 */
USTRUCT(BlueprintType)
struct FMGSpoilerConfig
{
	GENERATED_BODY()

	/// Unique identifier for this spoiler configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpoilerId;

	/// Player-friendly name (e.g., "GT Wing - Large")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Angle of the spoiler in degrees (0 = flat, 45 = aggressive)
	/// Higher angle = more downforce + more drag
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleDegrees = 15.0f;

	/// Width of the spoiler in meters
	/// Wider = more effective = more downforce
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width = 1.5f;

	/// Height/depth of the spoiler in meters
	/// Taller = catches more air = more effective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height = 0.2f;

	/// Multiplier for rear downforce when spoiler is active
	/// 1.5 = 50% more rear downforce
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceMultiplier = 1.5f;

	/// Drag penalty from the spoiler (multiplier)
	/// 1.1 = 10% more drag due to spoiler
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragPenalty = 1.1f;

	/// Whether the spoiler is currently deployed
	/// Retractable spoilers can be inactive at low speeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	/// Whether the spoiler auto-adjusts based on speed
	/// If true, angle changes dynamically for optimal performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoAdjusting = false;

	/// Minimum speed (in game units/sec) before spoiler has effect
	/// Below this speed, airflow is too slow for meaningful downforce
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedForEffect = 60.0f;
};

/**
 * Configuration for slipstream/drafting behavior.
 *
 * This is the aero subsystem's local slipstream config.
 * Tunable parameters that control how drafting works in conjunction
 * with the dedicated SlipstreamSubsystem.
 *
 * DESIGN NOTE: There's intentional overlap with MGSlipstreamSubsystem.
 * This config focuses on the aerodynamic/physics aspects, while
 * MGSlipstreamSubsystem handles the gameplay/scoring aspects.
 */
USTRUCT(BlueprintType)
struct FMGSlipstreamConfig
{
	GENERATED_BODY()

	/// Minimum safe distance for drafting (Unreal units)
	/// Closer than this = "too close" warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 200.0f;

	/// Maximum distance where drafting has any effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 2000.0f;

	/// Sweet spot distance for maximum drafting benefit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalDistance = 500.0f;

	/// Angle of the drafting cone behind the lead vehicle
	/// Tighter angle = harder to stay in draft
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConeAngleDegrees = 15.0f;

	/// Maximum drag reduction when perfectly drafting (0.40 = 40% less drag)
	/// This is significant - real-world NASCAR sees ~40% reduction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDragReduction = 0.40f;

	/// Maximum speed bonus from drafting (0.10 = 10% faster)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedBonus = 0.10f;

	/// How fast slingshot charges while drafting (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeRate = 0.2f;

	/// How fast slingshot charge decays when NOT drafting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DischargeRate = 0.5f;

	/// Speed boost when slingshot is activated (0.15 = 15% boost)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotBoost = 0.15f;

	/// How long the slingshot boost lasts (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotDuration = 2.0f;

	/// Minimum speed in MPH for drafting to work
	/// Drafting requires significant speed to matter
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedMPH = 80.0f;

	/// Score points awarded per second of drafting
	/// Integrates with scoring/XP system
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlipstreamPoints = 50;
};

//=============================================================================
// DATA STRUCTURES - STATISTICS
// Tracking data for achievements and progression
//=============================================================================

/**
 * Aerodynamics-related statistics for a player.
 *
 * Tracks player accomplishments related to aerodynamics and drafting.
 * Used for:
 * - Achievements ("Draft 100 miles total")
 * - Leaderboards ("Most slingshot overtakes")
 * - Player progression and skill assessment
 * - Post-race summary screens
 */
USTRUCT(BlueprintType)
struct FMGAeroPlayerStats
{
	GENERATED_BODY()

	/// Player identifier for stat tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/// Total time spent drafting across all sessions (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalSlipstreamTime = 0.0f;

	/// Longest continuous drafting session (seconds)
	/// Requires skill to maintain position for extended periods
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestSlipstreamSession = 0.0f;

	/// Number of slingshot moves executed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlingshotsUsed = 0;

	/// Successful overtakes completed using slingshot
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OvertakesFromSlipstream = 0;

	/// Total score points earned from drafting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlipstreamPointsEarned = 0;

	/// Highest speed achieved while in a slipstream
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedInSlipstream = 0.0f;

	/// Total distance traveled while drafting (in miles for display)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceDraftedMiles = 0.0f;

	/// Number of times optimal/perfect draft position maintained
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectSlipstreams = 0;
};

/**
 * Global configuration for the entire aerodynamics system.
 *
 * These settings affect ALL vehicles and the overall simulation.
 * Designers can toggle features on/off and tune global multipliers.
 *
 * REALISM SETTINGS:
 * The air density modifiers (altitude, temperature, humidity) are
 * based on real physics. They can be used for:
 * - Tracks at high altitude (thinner air = less drag AND less downforce)
 * - Hot weather events (hot air is less dense)
 * - Humid conditions (actually reduces density slightly)
 *
 * Set bSimulateDetailedAero = false for simpler, more arcade-like handling.
 */
USTRUCT(BlueprintType)
struct FMGGlobalAeroConfig
{
	GENERATED_BODY()

	/// Base air density at sea level, 20C, 0% humidity (kg/m^3)
	/// Standard atmospheric value = 1.225
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirDensityBase = 1.225f;

	/// How much altitude affects air density (reduction per meter)
	/// 0.0001 = 0.01% reduction per meter of altitude
	/// At 2000m elevation, air is about 20% less dense
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AltitudeEffect = 0.0001f;

	/// How much temperature affects air density (change per degree C)
	/// Hot air is less dense. 0.004 = 0.4% change per degree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TemperatureEffect = 0.004f;

	/// How much humidity affects air density
	/// Humid air is slightly LESS dense (water vapor is lighter than N2/O2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HumidityEffect = 0.001f;

	/// Maximum grip multiplier from downforce
	/// 1.5 = tires can have up to 50% more grip at high speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceGripMultiplierMax = 1.5f;

	/// Maximum top speed penalty from drag
	/// 0.15 = high-drag setups lose up to 15% top speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragTopSpeedPenaltyMax = 0.15f;

	/// Whether to use detailed aero simulation
	/// False = simplified calculations for arcade feel
	/// True = full physics-based aero forces
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateDetailedAero = true;

	/// Whether wind zones affect vehicles
	/// Can disable for simpler tracks or game modes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableWindEffects = true;

	/// Whether slipstream/drafting is enabled
	/// Some game modes might disable drafting for fair competition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSlipstream = true;
};

//=============================================================================
// DELEGATES (Event Callbacks)
// Allow other systems to react to aerodynamic events
//=============================================================================

/**
 * ABOUT THESE DELEGATES:
 *
 * Delegates are Unreal Engine's event/callback system. They allow this
 * subsystem to notify other systems when aerodynamic events occur,
 * without needing direct code references.
 *
 * USAGE IN BLUEPRINTS:
 * 1. Get the Aerodynamics Subsystem reference
 * 2. Bind an event to the delegate (e.g., OnSlipstreamEntered)
 * 3. Your event handler fires automatically when the event occurs
 *
 * USAGE IN C++:
 * AeroSubsystem->OnSlipstreamEntered.AddDynamic(this, &MyClass::HandleSlipstreamEntered);
 *
 * COMMON USE CASES:
 * - UI updates (show "DRAFTING" indicator)
 * - Sound effects (wind sounds, boost sounds)
 * - Visual effects (air distortion, particles)
 * - Scoring system (award points for drafting)
 * - AI decision making (know when being drafted)
 */

/// Fired when a vehicle enters another's slipstream.
/// Params: Following vehicle ID, Leader vehicle ID, Distance between them.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlipstreamEntered, const FString&, FollowerId, const FString&, LeaderId, float, Distance);

/// Fired when a vehicle exits a slipstream.
/// Params: Follower ID, Total duration of draft, Points earned.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlipstreamExited, const FString&, FollowerId, float, TotalDuration, int32, PointsEarned);

/// Fired when slipstream state transitions (e.g., Entering -> Active -> Optimal).
/// Params: Vehicle ID, Previous state, New state.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlipstreamStateChanged, const FString&, VehicleId, EMGSlipstreamState, OldState, EMGSlipstreamState, NewState);

/// Fired when slingshot becomes available.
/// Params: Vehicle ID, Boost amount available, Duration the boost will last.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlingshotReady, const FString&, VehicleId, float, BoostAmount, float, Duration);

/// Fired when player activates slingshot.
/// Params: Vehicle ID, Actual speed gained.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlingshotUsed, const FString&, VehicleId, float, SpeedGained);

/// Fired when a vehicle's downforce changes significantly.
/// Params: Vehicle ID, Old downforce value, New downforce value.
/// Useful for: Suspension animations, handling feel adjustments.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDownforceChanged, const FString&, VehicleId, float, OldDownforce, float, NewDownforce);

/// Fired when wind starts affecting a vehicle.
/// Params: Vehicle ID, Type of wind effect, Force vector being applied.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWindEffectApplied, const FString&, VehicleId, EMGWindEffect, Effect, FVector, Force);

/// Fired when a vehicle's aero profile is changed.
/// Params: Vehicle ID, Old profile ID, New profile ID.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAeroProfileChanged, const FString&, VehicleId, const FString&, OldProfileId, const FString&, NewProfileId);

/// Fired when a vehicle achieves optimal slipstream position.
/// Params: Vehicle ID, Bonus multiplier being applied.
/// This is a "you're doing great!" event for UI/audio feedback.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOptimalSlipstream, const FString&, VehicleId, float, BonusMultiplier);

//=============================================================================
// MAIN SUBSYSTEM CLASS
//=============================================================================

/**
 * Aerodynamics Subsystem - The brain of all vehicle aerodynamics.
 *
 * This subsystem manages all aerodynamic calculations including:
 * - Drag forces (air resistance)
 * - Lift/Downforce (vertical aerodynamic forces)
 * - Slipstream/Drafting (reduced drag behind other vehicles)
 * - Wind effects (environmental wind zones)
 * - Spoiler effects (adjustable aero devices)
 *
 * KEY DIFFERENCE FROM OTHER SUBSYSTEMS:
 * This is a UGameInstanceSubsystem, NOT a UWorldSubsystem:
 * - Persists across level loads/transitions
 * - Only ONE instance for the entire game session
 * - Player stats and profiles carry between races
 * - Access via: GetGameInstance()->GetSubsystem<UMGAerodynamicsSubsystem>()
 *
 * INTEGRATION WORKFLOW:
 * 1. Vehicle spawns and calls RegisterVehicle()
 * 2. Each frame, vehicle calls UpdateVehicleAero() with current position/velocity
 * 3. Subsystem calculates all forces and updates VehicleAeroState
 * 4. Vehicle queries CalculateTotalAeroForce() and applies to physics
 * 5. Vehicle despawns and calls UnregisterVehicle()
 *
 * COORDINATES WITH:
 * - MGSlipstreamSubsystem: For detailed drafting mechanics
 * - MGNitroBoostSubsystem: Aero profile affects nitro efficiency
 * - Vehicle Physics: Forces are applied to vehicle movement
 * - Weather System: Can provide temperature/humidity for air density
 *
 * @see FMGVehicleAeroState for the output of calculations
 * @see FMGAeroProfileDefinition for configuring vehicle aerodynamics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAerodynamicsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//=========================================================================
	// LIFECYCLE
	//=========================================================================

	/// Called when game instance is created. Sets up default profiles and config.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called when game instance is destroyed. Saves data and cleans up.
	virtual void Deinitialize() override;

	//=========================================================================
	// EVENTS (Bindable Delegates)
	// Subscribe to these to react to aerodynamic events
	//=========================================================================

	/// Broadcast when a vehicle enters another's slipstream.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlipstreamEntered OnSlipstreamEntered;

	/// Broadcast when a vehicle exits a slipstream.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlipstreamExited OnSlipstreamExited;

	/// Broadcast when slipstream state changes.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlipstreamStateChanged OnSlipstreamStateChanged;

	/// Broadcast when slingshot is ready.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlingshotReady OnSlingshotReady;

	/// Broadcast when slingshot is used.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlingshotUsed OnSlingshotUsed;

	/// Broadcast when downforce changes significantly.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnDownforceChanged OnDownforceChanged;

	/// Broadcast when wind affects a vehicle.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnWindEffectApplied OnWindEffectApplied;

	/// Broadcast when vehicle's aero profile changes.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnAeroProfileChanged OnAeroProfileChanged;

	/// Broadcast when optimal slipstream position is achieved.
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnOptimalSlipstream OnOptimalSlipstream;

	//=========================================================================
	// VEHICLE REGISTRATION
	// Vehicles must register to participate in aerodynamics calculations
	//=========================================================================

	/**
	 * Registers a vehicle with the aerodynamics system.
	 *
	 * Call this when a vehicle spawns (typically in BeginPlay).
	 * The vehicle must be registered before any aero calculations happen.
	 *
	 * @param VehicleId Unique string identifier for this vehicle
	 * @param ProfileId ID of the aero profile to use (must be registered)
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Vehicle")
	void RegisterVehicle(const FString& VehicleId, const FString& ProfileId);

	/**
	 * Unregisters a vehicle from the system.
	 *
	 * Call when vehicle is destroyed or leaves the race.
	 * Cleans up all state and slipstream relationships.
	 *
	 * @param VehicleId The vehicle to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Vehicle")
	void UnregisterVehicle(const FString& VehicleId);

	/**
	 * Gets the current aerodynamic state for a vehicle.
	 *
	 * Returns all calculated forces and effects for this frame.
	 * Use this for UI display or physics application.
	 *
	 * @param VehicleId The vehicle to query
	 * @return Current aero state (forces, slipstream, wind, etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Vehicle")
	FMGVehicleAeroState GetVehicleAeroState(const FString& VehicleId) const;

	/**
	 * Changes a vehicle's aerodynamic profile.
	 *
	 * Used when player adjusts aero setup between races or at pit stops.
	 * Takes effect immediately.
	 *
	 * @param VehicleId The vehicle to update
	 * @param ProfileId New profile to apply (must be registered)
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Vehicle")
	void SetVehicleProfile(const FString& VehicleId, const FString& ProfileId);

	//=========================================================================
	// AERODYNAMIC PROFILES
	// Manage the library of aero profiles vehicles can use
	//=========================================================================

	/**
	 * Registers a new aerodynamic profile.
	 *
	 * Profiles are typically registered at game startup from data assets.
	 * Custom profiles can be created by players and registered dynamically.
	 *
	 * @param Profile The profile definition to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Profile")
	void RegisterAeroProfile(const FMGAeroProfileDefinition& Profile);

	/**
	 * Gets a specific aero profile by ID.
	 *
	 * @param ProfileId The profile to retrieve
	 * @return The profile definition (check ProfileId for validity)
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Profile")
	FMGAeroProfileDefinition GetAeroProfile(const FString& ProfileId) const;

	/**
	 * Gets all registered aero profiles.
	 *
	 * Use for populating UI profile selection menus.
	 *
	 * @return Array of all available profiles
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Profile")
	TArray<FMGAeroProfileDefinition> GetAllProfiles() const;

	//=========================================================================
	// FORCE CALCULATIONS
	// Calculate aerodynamic forces for physics integration
	//=========================================================================

	/**
	 * Calculates drag force for a vehicle at given speed.
	 *
	 * Drag = 0.5 * AirDensity * Speed^2 * DragCoefficient * FrontalArea
	 *
	 * @param VehicleId The vehicle to calculate for
	 * @param SpeedMS Speed in meters per second
	 * @return Drag force in Newtons (apply opposite to velocity)
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Forces")
	float CalculateDragForce(const FString& VehicleId, float SpeedMS) const;

	/**
	 * Calculates lift force for a vehicle at given speed.
	 *
	 * Positive lift = upward (bad for racing - reduces grip).
	 * Most race cars have negative lift (= downforce).
	 *
	 * @param VehicleId The vehicle to calculate for
	 * @param SpeedMS Speed in meters per second
	 * @return Lift force in Newtons (positive = upward)
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Forces")
	float CalculateLiftForce(const FString& VehicleId, float SpeedMS) const;

	/**
	 * Calculates downforce for a vehicle at given speed.
	 *
	 * Downforce pushes the car onto the track, increasing grip.
	 * Scales with speed squared - doubles at 1.4x the speed.
	 *
	 * @param VehicleId The vehicle to calculate for
	 * @param SpeedMS Speed in meters per second
	 * @return Downforce in Newtons (positive = pressing car down)
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Forces")
	float CalculateDownforce(const FString& VehicleId, float SpeedMS) const;

	/**
	 * Calculates the total aerodynamic force vector.
	 *
	 * This is the main output - a single force vector combining:
	 * - Drag (opposite to velocity)
	 * - Lift/Downforce (vertical component)
	 * - Wind effects (if any)
	 *
	 * Apply this directly to your vehicle's physics body.
	 *
	 * @param VehicleId The vehicle to calculate for
	 * @param Velocity Current velocity vector
	 * @return Total aero force to apply (in world space)
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Forces")
	FVector CalculateTotalAeroForce(const FString& VehicleId, FVector Velocity) const;

	//=========================================================================
	// UPDATE FUNCTIONS
	// Call these every frame to keep aero state current
	//=========================================================================

	/**
	 * Main update function - call every frame for each vehicle.
	 *
	 * Updates the vehicle's aero state including:
	 * - All force calculations
	 * - Slipstream detection
	 * - Wind zone effects
	 * - Effective grip/top speed modifiers
	 *
	 * @param VehicleId The vehicle to update
	 * @param Position Current world position
	 * @param Velocity Current velocity vector
	 * @param DeltaTime Time since last frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Update")
	void UpdateVehicleAero(const FString& VehicleId, FVector Position, FVector Velocity, float DeltaTime);

	/**
	 * Gets the effective top speed after aero effects.
	 *
	 * High-drag setups reduce top speed. Use this to limit
	 * your vehicle's maximum velocity.
	 *
	 * @param VehicleId The vehicle to query
	 * @param BaseTopSpeed The vehicle's theoretical max speed
	 * @return Actual achievable top speed with current aero
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Update")
	float GetEffectiveTopSpeed(const FString& VehicleId, float BaseTopSpeed) const;

	/**
	 * Gets the effective grip multiplier from downforce.
	 *
	 * More downforce = more grip. Apply this to your tire
	 * friction calculations.
	 *
	 * @param VehicleId The vehicle to query
	 * @param BaseGrip The vehicle's base grip value
	 * @return Effective grip with downforce boost
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Update")
	float GetEffectiveGrip(const FString& VehicleId, float BaseGrip) const;

	//=========================================================================
	// SLIPSTREAM/DRAFTING
	// Functions for managing the drafting mechanic
	//=========================================================================

	/**
	 * Checks and updates slipstream relationship between two vehicles.
	 *
	 * Call this when you know two vehicles might be in a drafting situation.
	 * Usually called automatically during UpdateVehicleAero().
	 *
	 * @param FollowerId ID of the potential drafting vehicle
	 * @param LeaderId ID of the potential lead vehicle
	 * @param FollowerPos Position of follower
	 * @param LeaderPos Position of leader
	 * @param FollowerVelocity Velocity of follower
	 * @param LeaderVelocity Velocity of leader
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Slipstream")
	void CheckSlipstream(const FString& FollowerId, const FString& LeaderId, FVector FollowerPos, FVector LeaderPos, FVector FollowerVelocity, FVector LeaderVelocity);

	/**
	 * Checks if a vehicle is currently in a slipstream.
	 *
	 * @param VehicleId The vehicle to check
	 * @return True if currently drafting behind another vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Slipstream")
	bool IsInSlipstream(const FString& VehicleId) const;

	/**
	 * Gets the current slipstream session for a vehicle.
	 *
	 * @param VehicleId The vehicle to query
	 * @return Session data including duration, charge level, etc.
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Slipstream")
	FMGSlipstreamSession GetSlipstreamSession(const FString& VehicleId) const;

	/**
	 * Gets the slingshot charge level (0.0 to 1.0).
	 *
	 * Charge builds while drafting. At 1.0, slingshot is ready.
	 *
	 * @param VehicleId The vehicle to query
	 * @return Current charge level (0.0-1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Slipstream")
	float GetSlipstreamCharge(const FString& VehicleId) const;

	/**
	 * Checks if slingshot is ready to activate.
	 *
	 * @param VehicleId The vehicle to check
	 * @return True if charge is full and slingshot available
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Slipstream")
	bool IsSlingshotReady(const FString& VehicleId) const;

	/**
	 * Activates the slingshot boost.
	 *
	 * The vehicle gets a temporary speed boost to complete an overtake.
	 * Consumes the accumulated slingshot charge.
	 *
	 * @param VehicleId The vehicle activating slingshot
	 * @return The speed bonus being applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Slipstream")
	float ActivateSlingshot(const FString& VehicleId);

	/**
	 * Forces exit from current slipstream.
	 *
	 * Normally slipstream ends automatically when leaving the cone.
	 * Use this for special cases (vehicle destroyed, race end, etc.)
	 *
	 * @param VehicleId The vehicle to remove from slipstream
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Slipstream")
	void ExitSlipstream(const FString& VehicleId);

	//=========================================================================
	// WIND EFFECTS
	// Environmental wind that affects vehicle aerodynamics
	//=========================================================================

	/**
	 * Registers a wind zone in the world.
	 *
	 * Wind zones are typically placed by level designers using Blueprint actors
	 * that register with this system on BeginPlay.
	 *
	 * @param Zone The wind zone definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Wind")
	void RegisterWindZone(const FMGWindZone& Zone);

	/**
	 * Removes a wind zone from the system.
	 *
	 * @param ZoneId The zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Wind")
	void UnregisterWindZone(const FString& ZoneId);

	/**
	 * Gets a specific wind zone by ID.
	 *
	 * @param ZoneId The zone to retrieve
	 * @return The wind zone definition
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Wind")
	FMGWindZone GetWindZone(const FString& ZoneId) const;

	/**
	 * Gets all registered wind zones.
	 *
	 * @return Array of all wind zones in the current level
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Wind")
	TArray<FMGWindZone> GetAllWindZones() const;

	/**
	 * Sets a global wind that affects the entire level.
	 *
	 * Global wind is combined with local wind zones.
	 * Use for consistent weather conditions.
	 *
	 * @param Direction Direction the wind is blowing
	 * @param Speed Wind speed in game units per second
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Wind")
	void SetGlobalWind(FVector Direction, float Speed);

	/**
	 * Gets the wind vector at a specific location.
	 *
	 * Combines global wind with any local wind zones.
	 * Use for particle effects, flags, etc.
	 *
	 * @param Location World position to query
	 * @return Wind velocity vector at that location
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Wind")
	FVector GetWindAtLocation(FVector Location) const;

	/**
	 * Calculates the force wind applies to a vehicle.
	 *
	 * Considers vehicle's aero profile and current velocity
	 * (headwind vs tailwind vs crosswind).
	 *
	 * @param VehicleId The vehicle to calculate for
	 * @param VehicleVelocity The vehicle's current velocity
	 * @return Force vector to apply to the vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Wind")
	FVector CalculateWindForce(const FString& VehicleId, FVector VehicleVelocity) const;

	//=========================================================================
	// SPOILERS
	// Adjustable rear wing/spoiler management
	//=========================================================================

	/**
	 * Sets the spoiler configuration for a vehicle.
	 *
	 * Used when player equips a new spoiler or changes settings.
	 *
	 * @param VehicleId The vehicle to update
	 * @param Spoiler New spoiler configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Spoiler")
	void SetVehicleSpoiler(const FString& VehicleId, const FMGSpoilerConfig& Spoiler);

	/**
	 * Gets the current spoiler configuration for a vehicle.
	 *
	 * @param VehicleId The vehicle to query
	 * @return Current spoiler config
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Spoiler")
	FMGSpoilerConfig GetVehicleSpoiler(const FString& VehicleId) const;

	/**
	 * Adjusts the spoiler angle for a vehicle.
	 *
	 * Higher angle = more downforce + more drag.
	 * Can be called during gameplay for adjustable spoilers.
	 *
	 * @param VehicleId The vehicle to adjust
	 * @param AngleDegrees New spoiler angle (0 = flat, 45 = aggressive)
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Spoiler")
	void SetSpoilerAngle(const FString& VehicleId, float AngleDegrees);

	/**
	 * Activates or deactivates a vehicle's spoiler.
	 *
	 * For retractable spoilers - can lower at low speeds
	 * to reduce drag, raise at high speeds for downforce.
	 *
	 * @param VehicleId The vehicle to update
	 * @param bActive True to deploy spoiler, false to retract
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Spoiler")
	void SetSpoilerActive(const FString& VehicleId, bool bActive);

	//=========================================================================
	// STATISTICS
	// Player stats for achievements and progression
	//=========================================================================

	/**
	 * Gets accumulated aero stats for a player.
	 *
	 * @param PlayerId The player to query
	 * @return Stats including drafting time, slingshots used, etc.
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Stats")
	FMGAeroPlayerStats GetPlayerStats(const FString& PlayerId) const;

	/**
	 * Resets a player's aero statistics.
	 *
	 * @param PlayerId The player whose stats to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	//=========================================================================
	// CONFIGURATION
	// Global settings for the entire aero system
	//=========================================================================

	/**
	 * Updates the slipstream configuration.
	 *
	 * @param Config New slipstream settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Config")
	void SetSlipstreamConfig(const FMGSlipstreamConfig& Config);

	/**
	 * Gets the current slipstream configuration.
	 *
	 * @return Current slipstream settings
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Config")
	FMGSlipstreamConfig GetSlipstreamConfig() const;

	/**
	 * Updates the global aerodynamics configuration.
	 *
	 * @param Config New global settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Config")
	void SetGlobalAeroConfig(const FMGGlobalAeroConfig& Config);

	/**
	 * Gets the current global aero configuration.
	 *
	 * @return Current global settings
	 */
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Config")
	FMGGlobalAeroConfig GetGlobalAeroConfig() const;

	//=========================================================================
	// SYSTEM UPDATE
	// Main tick function for the aero system
	//=========================================================================

	/**
	 * Updates all aerodynamics calculations.
	 *
	 * Called automatically by timer, but can be called manually
	 * for debugging or custom update schedules.
	 *
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Update")
	void UpdateAerodynamics(float DeltaTime);

	//=========================================================================
	// PERSISTENCE
	// Save/load for player data
	//=========================================================================

	/**
	 * Saves all aero data to persistent storage.
	 *
	 * Includes player stats, custom profiles, etc.
	 * Called automatically on game exit.
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Persistence")
	void SaveAeroData();

	/**
	 * Loads aero data from persistent storage.
	 *
	 * Restores player stats, custom profiles, etc.
	 * Called automatically on subsystem Initialize.
	 */
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Persistence")
	void LoadAeroData();

protected:
	//=========================================================================
	// INTERNAL FUNCTIONS
	// These implement the actual aerodynamic calculations
	//=========================================================================

	/// Main tick - updates all vehicles every frame.
	void TickAerodynamics(float DeltaTime);

	/// Updates all active slipstream sessions.
	void UpdateSlipstreams(float DeltaTime);

	/// Applies wind zone effects to vehicles.
	void UpdateWindEffects(float DeltaTime);

	/// Calculates all aero forces for a single vehicle.
	void CalculateAeroForces(const FString& VehicleId, float DeltaTime);

	/// Gets air density at a world location (considering altitude, temp, etc.)
	float GetAirDensityAtLocation(FVector Location) const;

	/// Geometric check: is the follower inside the leader's slipstream cone?
	bool IsInSlipstreamCone(FVector FollowerPos, FVector LeaderPos, FVector LeaderForward, float ConeAngle, float MaxDist) const;

	/// Updates a slipstream session's state based on current distance.
	void UpdateSlipstreamState(FMGSlipstreamSession& Session, float Distance, float DeltaTime);

	/// Awards score points for successful drafting.
	void AwardSlipstreamPoints(const FString& VehicleId, float Duration);

	/// Updates persistent player statistics from a completed session.
	void UpdatePlayerStats(const FString& PlayerId, const FMGSlipstreamSession& Session);

	/// Generates a unique ID for a new slipstream session.
	FString GenerateSessionId() const;

private:
	//=========================================================================
	// INTERNAL STATE
	// Runtime data stored by the subsystem
	//=========================================================================

	/// Current aero state for all registered vehicles.
	/// Key = VehicleId, Value = Current calculated aero state.
	UPROPERTY()
	TMap<FString, FMGVehicleAeroState> VehicleStates;

	/// Maps vehicles to their assigned aero profiles.
	/// Key = VehicleId, Value = ProfileId.
	UPROPERTY()
	TMap<FString, FString> VehicleProfiles;

	/// Library of all registered aero profiles.
	/// Key = ProfileId, Value = Profile definition.
	UPROPERTY()
	TMap<FString, FMGAeroProfileDefinition> AeroProfiles;

	/// Currently active slipstream sessions.
	/// Key = FollowerVehicleId (a vehicle can only draft one leader at a time).
	UPROPERTY()
	TMap<FString, FMGSlipstreamSession> ActiveSlipstreams;

	/// All registered wind zones in the current level.
	/// Key = ZoneId, Value = Zone definition.
	UPROPERTY()
	TMap<FString, FMGWindZone> WindZones;

	/// Spoiler configurations for each vehicle.
	/// Key = VehicleId, Value = Spoiler config.
	UPROPERTY()
	TMap<FString, FMGSpoilerConfig> VehicleSpoilers;

	/// Persistent player statistics.
	/// Key = PlayerId, Value = Accumulated stats.
	UPROPERTY()
	TMap<FString, FMGAeroPlayerStats> PlayerStats;

	/// Current slipstream configuration.
	UPROPERTY()
	FMGSlipstreamConfig SlipstreamConfig;

	/// Current global aero configuration.
	UPROPERTY()
	FMGGlobalAeroConfig GlobalConfig;

	/// Direction of global wind (affects entire level).
	UPROPERTY()
	FVector GlobalWindDirection = FVector::ZeroVector;

	/// Speed of global wind.
	UPROPERTY()
	float GlobalWindSpeed = 0.0f;

	/// Counter for generating unique session IDs.
	UPROPERTY()
	int32 SessionCounter = 0;

	/// Timer handle for the main tick function.
	FTimerHandle AeroTickTimer;
};

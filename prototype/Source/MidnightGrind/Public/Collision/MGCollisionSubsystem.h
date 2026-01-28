// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * ============================================================================
 * MGCollisionSubsystem.h
 * ============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the Collision Subsystem - a centralized system that manages
 * all vehicle-to-vehicle and vehicle-to-environment collisions in the game.
 * Think of it as the "physics referee" that detects when vehicles crash into
 * things, calculates the damage, and triggers appropriate effects.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. GAME INSTANCE SUBSYSTEM:
 *    - This class inherits from UGameInstanceSubsystem, meaning it exists for
 *      the entire lifetime of the game session (from start to quit)
 *    - There is only ONE instance of this subsystem - all vehicles share it
 *    - Access it from any Blueprint or C++ code using:
 *      GetGameInstance()->GetSubsystem<UMGCollisionSubsystem>()
 *
 * 2. COLLISION TYPES:
 *    - VehicleToVehicle: Two cars hitting each other
 *    - VehicleToWall: Car hitting a solid wall or building
 *    - Sideswipe: Glancing side-to-side contact between vehicles
 *    - TBone: Perpendicular impact (one car hits another's side)
 *    - HeadOn: Two cars colliding front-to-front
 *    - RearEnd: One car hitting another from behind
 *
 * 3. COLLISION SEVERITY:
 *    - Ranges from "Glancing" (minor scratch) to "Catastrophic" (totaled)
 *    - Determined by impact speed and force
 *    - Higher severity = more damage, bigger effects, more points for takedowns
 *
 * 4. IMPACT ZONES:
 *    - Vehicles are divided into 10 zones (front, sides, rear, roof, etc.)
 *    - Each zone can take independent damage
 *    - Hitting certain zones affects specific systems (front hits damage engine)
 *
 * 5. DAMAGE STATES:
 *    - Pristine -> Scratched -> Dented -> Damaged -> Heavy Damage -> Critical -> Wrecked
 *    - Visual appearance changes at each state
 *    - Performance degrades as damage increases
 *
 * 6. TAKEDOWNS:
 *    - Special events when you wreck an opponent through aggressive driving
 *    - Award bonus points, boost, and trigger special camera effects
 *    - Can chain together for combo multipliers
 *
 * 7. INVINCIBILITY:
 *    - Brief period after respawning where you cannot take damage
 *    - Prevents spawn-killing and gives players time to recover
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *   [Physics Engine]
 *         |
 *         v (collision detected)
 *   [UMGCollisionSubsystem] <--- This file!
 *         |
 *         +---> [UMGDamageSubsystem] - Applies damage to vehicle health
 *         |
 *         +---> [UMGTakedownSubsystem] - Handles takedown scoring/cameras
 *         |
 *         +---> [Audio/VFX Systems] - Plays crash sounds and particles
 *         |
 *         +---> [HUD/UI] - Shows damage indicators, points earned
 *
 * TYPICAL USAGE FLOW:
 * -------------------
 * 1. Physics engine detects a collision between two actors
 * 2. Vehicle's collision component calls ProcessCollision() on this subsystem
 * 3. Subsystem determines collision type, severity, and impact zone
 * 4. Damage is applied to the vehicle(s) involved
 * 5. If a takedown occurred, the TakedownSubsystem is notified
 * 6. Events are broadcast so UI/audio/VFX can respond
 * 7. Stats are updated for end-of-race summaries
 *
 * EXAMPLE CODE:
 * -------------
 * // In your vehicle's collision handler:
 * void AMyVehicle::OnHit(AActor* OtherActor, FVector HitLocation, FVector HitNormal)
 * {
 *     UMGCollisionSubsystem* CollisionSys = GetGameInstance()->GetSubsystem<UMGCollisionSubsystem>();
 *
 *     // Process the collision and get detailed event data
 *     FMGCollisionEvent Event = CollisionSys->ProcessCollision(
 *         MyVehicleId,
 *         OtherActor->GetName(),
 *         HitLocation,
 *         HitNormal,
 *         GetVelocity(),
 *         EMGCollisionType::VehicleToVehicle
 *     );
 *
 *     // React to the collision
 *     if (Event.bCausedWreck)
 *     {
 *         // Play wreck animation
 *     }
 * }
 *
 * @see UMGDamageSubsystem - Handles health/damage calculations
 * @see UMGTakedownSubsystem - Handles takedown scoring and crash cameras
 * @see UMGDestructionSubsystem - Handles environmental object destruction
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCollisionSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * Collision type - Categorizes the kind of collision that occurred.
 *
 * This helps the game know what kind of effects to play, how much damage
 * to apply, and whether special mechanics (like takedowns) should trigger.
 */
UENUM(BlueprintType)
enum class EMGCollisionType : uint8
{
	/** No collision (default/invalid state) */
	None				UMETA(DisplayName = "None"),

	/** Two player/AI vehicles colliding - may trigger takedowns */
	VehicleToVehicle	UMETA(DisplayName = "Vehicle to Vehicle"),

	/** Vehicle hitting a solid wall or building - high damage, full stop */
	VehicleToWall		UMETA(DisplayName = "Vehicle to Wall"),

	/** Vehicle hitting race barriers - moderate damage, designed to redirect */
	VehicleToBarrier	UMETA(DisplayName = "Vehicle to Barrier"),

	/** Vehicle hitting civilian traffic cars - can trigger traffic takedowns */
	VehicleToTraffic	UMETA(DisplayName = "Vehicle to Traffic"),

	/** Vehicle hitting misc props (cones, signs) - usually low damage */
	VehicleToObject		UMETA(DisplayName = "Vehicle to Object"),

	/** Vehicle hitting guard rails - scraping damage, keeps you on track */
	VehicleToRail		UMETA(DisplayName = "Vehicle to Rail"),

	/** Side-to-side contact between vehicles - used for grinding maneuvers */
	Sideswipe			UMETA(DisplayName = "Sideswipe"),

	/** Perpendicular impact (hitting someone's door) - high takedown potential */
	TBone				UMETA(DisplayName = "T-Bone"),

	/** Front-to-front collision - massive damage to both vehicles */
	HeadOn				UMETA(DisplayName = "Head On"),

	/** Hitting another vehicle from behind - shunting/ramming */
	RearEnd				UMETA(DisplayName = "Rear End"),

	/** Vehicle flipped over - roof/underbody damage */
	Rollover			UMETA(DisplayName = "Rollover"),

	/** Hard landing from a jump - suspension/underbody damage */
	Airborne			UMETA(DisplayName = "Airborne Landing")
};

/**
 * Collision severity - How intense the impact was.
 *
 * Severity is calculated from impact speed and force. Higher severity means:
 * - More damage to the vehicle
 * - Bigger visual/audio effects
 * - More points if it results in a takedown
 * - More likely to trigger slow-motion crash camera
 */
UENUM(BlueprintType)
enum class EMGCollisionSeverity : uint8
{
	/** Barely touched - paint scratch at worst (< 20 km/h) */
	Glancing			UMETA(DisplayName = "Glancing"),

	/** Light bump - small dent, minimal speed loss (20-40 km/h) */
	Minor				UMETA(DisplayName = "Minor"),

	/** Solid hit - visible damage, noticeable slowdown (40-70 km/h) */
	Moderate			UMETA(DisplayName = "Moderate"),

	/** Heavy impact - significant damage, may affect handling (70-100 km/h) */
	Major				UMETA(DisplayName = "Major"),

	/** Brutal crash - major damage, systems may fail (100-150 km/h) */
	Severe				UMETA(DisplayName = "Severe"),

	/** Devastating - near or total destruction (> 150 km/h) */
	Catastrophic		UMETA(DisplayName = "Catastrophic")
};

/**
 * Impact zone - Which part of the vehicle was hit.
 *
 * The vehicle is divided into zones like a damage diagram. Each zone:
 * - Has its own health pool (can be damaged independently)
 * - Affects different components when damaged
 * - Has different armor ratings (if upgraded)
 *
 * Zone layout (top-down view):
 *
 *     [FrontLeft] [FrontCenter] [FrontRight]
 *     [SideLeft ]              [SideRight ]
 *     [RearLeft ] [RearCenter ] [RearRight ]
 *
 *              [Roof] (from above)
 *         [Undercarriage] (from below)
 */
UENUM(BlueprintType)
enum class EMGImpactZone : uint8
{
	/** Front bumper/grille - affects engine and radiator */
	FrontCenter			UMETA(DisplayName = "Front Center"),

	/** Front-left corner - affects headlight, left fender */
	FrontLeft			UMETA(DisplayName = "Front Left"),

	/** Front-right corner - affects headlight, right fender */
	FrontRight			UMETA(DisplayName = "Front Right"),

	/** Left doors and panels - affects left suspension, driver */
	SideLeft			UMETA(DisplayName = "Side Left"),

	/** Right doors and panels - affects right suspension */
	SideRight			UMETA(DisplayName = "Side Right"),

	/** Rear bumper/trunk - affects fuel tank, exhaust */
	RearCenter			UMETA(DisplayName = "Rear Center"),

	/** Rear-left corner - affects left taillight, quarter panel */
	RearLeft			UMETA(DisplayName = "Rear Left"),

	/** Rear-right corner - affects right taillight, quarter panel */
	RearRight			UMETA(DisplayName = "Rear Right"),

	/** Top of vehicle - damaged during rollovers */
	Roof				UMETA(DisplayName = "Roof"),

	/** Bottom of vehicle - damaged by curbs, rough terrain, hard landings */
	Undercarriage		UMETA(DisplayName = "Undercarriage")
};

/**
 * Vehicle damage state - Overall condition of the vehicle.
 *
 * As damage accumulates, the vehicle progresses through these states.
 * Each state has visual and gameplay implications:
 * - Visual: More dents, scratches, smoke, fire
 * - Gameplay: Reduced performance, eventual destruction
 *
 * State progression: Pristine -> Scratched -> Dented -> Damaged ->
 *                    HeavyDamage -> Critical -> Wrecked
 */
UENUM(BlueprintType)
enum class EMGDamageState : uint8
{
	/** Brand new - no damage whatsoever (100% health) */
	Pristine			UMETA(DisplayName = "Pristine"),

	/** Minor cosmetic damage - small scratches (90-100% health) */
	Scratched			UMETA(DisplayName = "Scratched"),

	/** Visible dents and dings (70-90% health) */
	Dented				UMETA(DisplayName = "Dented"),

	/** Significant damage, body panels bent (50-70% health) */
	Damaged				UMETA(DisplayName = "Damaged"),

	/** Major damage, parts hanging off, smoking (25-50% health) */
	HeavyDamage			UMETA(DisplayName = "Heavy Damage"),

	/** Near destruction, on fire, barely running (10-25% health) */
	Critical			UMETA(DisplayName = "Critical"),

	/** Destroyed - vehicle is totaled and undriveable (0% health) */
	Wrecked				UMETA(DisplayName = "Wrecked")
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Collision event data - Complete information about a single collision.
 *
 * This structure is created whenever a collision occurs and contains
 * everything you need to know about what happened. It's passed to:
 * - Event delegates (for UI/audio/VFX to respond)
 * - Takedown system (to check if it was a takedown)
 * - Stats tracking (for end-of-race summaries)
 *
 * Think of it as a "collision report" that gets filed for each crash.
 */
USTRUCT(BlueprintType)
struct FMGCollisionEvent
{
	GENERATED_BODY()

	/** Unique identifier for this collision event (for tracking/debugging) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CollisionId;

	/** ID of the player/vehicle that was involved in this collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/** ID of the other entity involved (another vehicle, wall, prop, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OtherEntityId;

	/** What kind of collision this was (vehicle-to-vehicle, wall, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollisionType Type = EMGCollisionType::None;

	/** How severe the impact was (determines damage and effects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollisionSeverity Severity = EMGCollisionSeverity::Minor;

	/** Which part of the vehicle was hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGImpactZone ImpactZone = EMGImpactZone::FrontCenter;

	/** World-space position where the collision occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactLocation = FVector::ZeroVector;

	/** Surface normal at impact point (points away from the surface hit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactNormal = FVector::ZeroVector;

	/** Velocity of the vehicle at moment of impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactVelocity = FVector::ZeroVector;

	/** Speed in km/h at moment of impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactSpeed = 0.0f;

	/** Force of the impact in Newtons (mass * deceleration) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactForce = 0.0f;

	/** Speed difference between two colliding vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RelativeSpeed = 0.0f;

	/** How much damage was dealt by this collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageDealt = 0.0f;

	/** How much speed was lost due to this collision (in km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLoss = 0.0f;

	/** Rotational impulse applied to the vehicle (causes spinning) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator SpinImpulse = FRotator::ZeroRotator;

	/** True if this collision destroyed/wrecked a vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCausedWreck = false;

	/** True if this collision counted as a successful takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasTakedown = false;

	/** Points awarded for this collision (takedowns, aggressive driving) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsAwarded = 0;

	/** When this collision occurred (for replay/stats) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Vehicle damage zone - Health and state for one part of the vehicle.
 *
 * Each vehicle has multiple damage zones that track damage independently.
 * This allows for realistic localized damage where the front might be
 * destroyed while the rear is still pristine.
 *
 * Zones can be upgraded with armor to reduce incoming damage.
 */
USTRUCT(BlueprintType)
struct FMGDamageZone
{
	GENERATED_BODY()

	/** Which zone this data represents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGImpactZone Zone = EMGImpactZone::FrontCenter;

	/** Current health points for this zone (0 = destroyed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth = 100.0f;

	/** Maximum health for this zone (can be upgraded) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	/** Multiplier for incoming damage (< 1.0 = resistant, > 1.0 = vulnerable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	/** Current visual/functional state of this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageState State = EMGDamageState::Pristine;

	/** True if this zone has armor upgrade installed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsArmored = false;

	/** Armor rating (0-100) - higher = more damage reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmorRating = 0.0f;
};

/**
 * Vehicle collision state - Complete damage snapshot for one vehicle.
 *
 * This is the "master record" of a vehicle's condition. It contains:
 * - Overall health and damage state
 * - Per-zone damage breakdown
 * - Component health (engine, steering, etc.)
 * - Status flags (wrecked, on fire)
 * - Statistics (total collisions, damage taken)
 *
 * The collision subsystem maintains one of these for each registered vehicle.
 */
USTRUCT(BlueprintType)
struct FMGVehicleCollisionState
{
	GENERATED_BODY()

	/** Unique identifier for this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	/** Current total health (aggregate of all zones) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalHealth = 100.0f;

	/** Maximum total health (sum of all zone max health) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	/** Overall damage state (worst state across all zones) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageState OverallState = EMGDamageState::Pristine;

	/** Damage state for each body zone (front, sides, rear, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGImpactZone, FMGDamageZone> DamageZones;

	/** Engine health - affects power output (0% = engine dead) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineHealth = 100.0f;

	/** Steering health - affects turn response (0% = can't steer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringHealth = 100.0f;

	/** Suspension health - affects handling and ride height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SuspensionHealth = 100.0f;

	/** Transmission health - affects gear changes and acceleration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransmissionHealth = 100.0f;

	/** Status of each tire [FL, FR, RL, RR] - false = flat/destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<bool> TireStatus;

	/** True if vehicle has been destroyed (0% health) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWrecked = false;

	/** True if vehicle is currently on fire (takes ongoing damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnFire = false;

	/** How long the vehicle has been on fire (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireDuration = 0.0f;

	/** Total number of collisions this vehicle has been in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCollisions = 0;

	/** Cumulative damage this vehicle has received */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDamageTaken = 0.0f;
};

/**
 * Collision physics config - Tuning parameters for collision behavior.
 *
 * These values control how collisions "feel" in the game. Designers can
 * adjust these to make collisions more or less punishing, more or less
 * bouncy, etc.
 *
 * This is typically set once at game start and rarely changed during play.
 */
USTRUCT(BlueprintType)
struct FMGCollisionPhysicsConfig
{
	GENERATED_BODY()

	/** How much vehicle mass affects collision outcomes (heavier = more force) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MassInfluence = 1.0f;

	/** How much speed affects damage (higher = speed hurts more) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedInfluence = 1.5f;

	/** How much impact angle affects damage (glancing vs. direct) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleInfluence = 1.2f;

	/** Bounciness of collisions (0 = no bounce, 1 = perfect elastic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestitutionCoefficient = 0.3f;

	/** Friction during collision (affects sliding/grinding) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrictionCoefficient = 0.8f;

	/** Minimum speed (km/h) required to cause any damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedForDamage = 20.0f;

	/** Base damage dealt per MPH of impact speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamagePerMPH = 0.5f;

	/** How much collisions cause vehicles to spin (higher = more spin) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpinImpulseMultiplier = 1.0f;

	/** How much vehicles bounce off walls and each other */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BounceMultiplier = 1.0f;

	/** Minimum percentage of speed lost in any collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLossPercentMin = 0.1f;

	/** Maximum percentage of speed lost in severe collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLossPercentMax = 0.5f;

	/** Brief invincibility after collision to prevent chain-stun (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InvincibilityAfterCollision = 0.5f;
};

/**
 * Crash effect definition - Visual/audio/haptic effects for a collision.
 *
 * Different severity levels trigger different effects. For example:
 * - Minor: Small spark particle, soft bump sound
 * - Catastrophic: Explosion particles, massive crash sound, slow-mo, screen shake
 *
 * These are configured in the editor and selected at runtime based on severity.
 */
USTRUCT(BlueprintType)
struct FMGCrashEffect
{
	GENERATED_BODY()

	/** Minimum severity required to trigger this effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollisionSeverity MinSeverity = EMGCollisionSeverity::Minor;

	/** Particle system to spawn at impact point (sparks, debris, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> ParticleEffect;

	/** Sound to play (crash, crunch, metal sounds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> SoundEffect;

	/** How much to shake the camera (0 = none, 1 = violent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraShakeIntensity = 0.0f;

	/** How long to play slow-motion effect (seconds, 0 = none) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowMotionDuration = 0.0f;

	/** Time scale during slow-motion (0.5 = half speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowMotionScale = 0.5f;

	/** Whether to trigger controller rumble/vibration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTriggerRumble = true;

	/** Intensity of controller rumble (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RumbleIntensity = 0.5f;

	/** Duration of controller rumble (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RumbleDuration = 0.3f;
};

/**
 * Player collision stats - Aggregate statistics for one player.
 *
 * Tracks collision history for:
 * - End-of-race summary screens
 * - Achievement/trophy tracking
 * - Leaderboards (most takedowns, biggest crash, etc.)
 * - Player profile statistics
 *
 * Stats persist across races and are saved to player profile.
 */
USTRUCT(BlueprintType)
struct FMGCollisionStats
{
	GENERATED_BODY()

	/** Player these stats belong to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/** Total number of collisions this player has been in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCollisions = 0;

	/** Breakdown of collisions by type (wall, vehicle, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCollisionType, int32> CollisionsByType;

	/** Breakdown of collisions by severity (minor, major, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCollisionSeverity, int32> CollisionsBySeverity;

	/** Number of times this player has wrecked opponents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TakedownsDealt = 0;

	/** Number of times this player has been wrecked by opponents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TakedownsReceived = 0;

	/** Total number of times this player's vehicle was destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WrecksTotal = 0;

	/** Total damage this player has dealt to others */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDamageDealt = 0.0f;

	/** Total damage this player has received */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDamageReceived = 0.0f;

	/** Fastest speed at which this player has crashed (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestImpactSpeed = 0.0f;

	/** Biggest impact force this player has experienced (Newtons) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestImpactForce = 0.0f;

	/** Number of races completed without any collisions (achievement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanRacesWithoutCollision = 0;

	/** Total points earned from aggressive driving (takedowns, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AggressivePointsEarned = 0;
};

/**
 * Takedown event - Data for a successful takedown (wrecking an opponent).
 *
 * A takedown occurs when you cause another vehicle to crash through
 * aggressive driving. This structure records all the details for:
 * - Scoring (base points + bonuses)
 * - UI display ("TAKEDOWN!" popup)
 * - Achievement tracking
 * - Replay/highlight recording
 */
USTRUCT(BlueprintType)
struct FMGTakedownEvent
{
	GENERATED_BODY()

	/** Unique identifier for this takedown event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TakedownId;

	/** Player who performed the takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackerId;

	/** Player/AI who was taken down */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VictimId;

	/** Type of collision that caused the takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollisionType CollisionType = EMGCollisionType::None;

	/** World location where the takedown occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** Speed at which the takedown collision occurred (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactSpeed = 0.0f;

	/** Total points awarded for this takedown (including bonuses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsAwarded = 0;

	/** Position in takedown chain (1st, 2nd, 3rd consecutive takedown) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainCount = 0;

	/** True if this was revenge for being taken down earlier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasRevenge = false;

	/** True if attacker was drifting during the takedown (bonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasDuringDrift = false;

	/** True if attacker or victim was airborne (bonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasAirborne = false;

	/** When the takedown occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Scoring config for collisions - Point values and multipliers for takedowns.
 *
 * This structure lets designers tune the scoring system. Higher values
 * make certain actions more rewarding, encouraging specific playstyles.
 *
 * Example: Setting AirborneTakedownBonus high encourages players to use
 * ramps and jumps for creative takedowns.
 */
USTRUCT(BlueprintType)
struct FMGCollisionScoringConfig
{
	GENERATED_BODY()

	/** Points awarded for a basic takedown (no bonuses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePointsPerTakedown = 500;

	/** Bonus points for taking down someone who took you down */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RevengeBonus = 250;

	/** Bonus points for takedown while drifting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DriftTakedownBonus = 300;

	/** Bonus points for takedown while airborne */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AirborneTakedownBonus = 400;

	/** Multiplier increase per consecutive takedown (0.5 = +50% each) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainMultiplierPerTakedown = 0.5f;

	/** Maximum chain multiplier (caps the bonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChainMultiplier = 5.0f;

	/** Time window to chain takedowns together (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainWindowSeconds = 5.0f;

	/** Bonus points for specific collision types (T-Bone = +100, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCollisionType, int32> TypeBonusPoints;

	/** Score multiplier based on collision severity (Catastrophic = 2x, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCollisionSeverity, float> SeverityMultipliers;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================
// Delegates are Unreal's event system. Other parts of the game can "subscribe"
// to these events and get notified when they happen. For example, the HUD can
// subscribe to OnTakedownDealt to show a "TAKEDOWN!" popup.
//
// Usage in Blueprint: Bind to the event in the Event Graph
// Usage in C++: SubsystemPtr->OnTakedownDealt.AddDynamic(this, &MyClass::HandleTakedown);
// ============================================================================

/** Fired whenever any collision occurs (for audio/VFX/UI) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionOccurred, const FString&, PlayerId, const FMGCollisionEvent&, Event);

/** Fired when a vehicle takes damage (for damage indicators) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageReceived, const FString&, VehicleId, float, DamageAmount, EMGImpactZone, Zone);

/** Fired when vehicle damage state changes (Pristine->Scratched, etc.) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageStateChanged, const FString&, VehicleId, EMGDamageState, OldState, EMGDamageState, NewState);

/** Fired when a vehicle is completely destroyed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleWrecked, const FString&, VehicleId, const FMGCollisionEvent&, FinalCollision);

/** Fired when a player successfully takes down an opponent */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakedownDealt, const FString&, AttackerId, const FMGTakedownEvent&, Takedown);

/** Fired when a player gets taken down by an opponent */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakedownReceived, const FString&, VictimId, const FMGTakedownEvent&, Takedown);

/** Fired when takedown chain extends (multiple takedowns in sequence) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTakedownChain, const FString&, PlayerId, int32, ChainCount, float, ChainMultiplier);

/** Fired when a revenge takedown is completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRevengeComplete, const FString&, AttackerId, const FString&, OriginalAttackerId);

/** Fired when a vehicle is repaired */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleRepaired, const FString&, VehicleId, float, RepairAmount);

/** Fired when invincibility period starts (after respawn) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInvincibilityStart, const FString&, VehicleId, float, Duration);

/** Fired when invincibility period ends */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInvincibilityEnd, const FString&, VehicleId);

/** Fired when crash effects are triggered (for effect systems) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionEffectsTriggered, const FMGCollisionEvent&, Collision, const FMGCrashEffect&, Effect);

// ============================================================================
// SUBSYSTEM CLASS
// ============================================================================

/**
 * Collision Subsystem - Central manager for all vehicle collision handling.
 *
 * This subsystem is the "brain" of the collision system. It:
 * - Processes all collision events from physics
 * - Calculates damage and applies it to vehicles
 * - Detects and scores takedowns
 * - Triggers visual/audio effects
 * - Maintains statistics for all players
 *
 * ## Lifecycle
 * - Created automatically when the game starts
 * - Persists for the entire game session
 * - Destroyed when the game closes
 *
 * ## How to Use
 *
 * Get the subsystem:
 * @code
 * UMGCollisionSubsystem* CollisionSys = GetGameInstance()->GetSubsystem<UMGCollisionSubsystem>();
 * @endcode
 *
 * Register a vehicle when it spawns:
 * @code
 * CollisionSys->RegisterVehicle("Player_001", 100.0f);
 * @endcode
 *
 * Process a collision when physics detects one:
 * @code
 * FMGCollisionEvent Event = CollisionSys->ProcessCollision(
 *     "Player_001", "Wall_Barrier_42",
 *     HitLocation, HitNormal, Velocity, EMGCollisionType::VehicleToWall);
 * @endcode
 *
 * Listen for events in Blueprint or C++:
 * @code
 * CollisionSys->OnTakedownDealt.AddDynamic(this, &UMyHUD::ShowTakedownPopup);
 * @endcode
 *
 * ## Thread Safety
 * All functions must be called from the game thread only.
 *
 * @see UMGDamageSubsystem - Works closely with this for damage calculations
 * @see UMGTakedownSubsystem - Handles advanced takedown mechanics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCollisionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// LIFECYCLE FUNCTIONS
	// ==========================================
	// These are called automatically by Unreal Engine.
	// You generally don't need to call these yourself.

	/** Called when the subsystem is created (game start) */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called when the subsystem is destroyed (game end) */
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS (DELEGATES)
	// ==========================================
	// Subscribe to these events to react to collisions.
	// Example: OnCollisionOccurred.AddDynamic(this, &UMyClass::HandleCollision);
	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnCollisionOccurred OnCollisionOccurred;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnDamageReceived OnDamageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnDamageStateChanged OnDamageStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnVehicleWrecked OnVehicleWrecked;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnTakedownDealt OnTakedownDealt;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnTakedownReceived OnTakedownReceived;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnTakedownChain OnTakedownChain;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnRevengeComplete OnRevengeComplete;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnVehicleRepaired OnVehicleRepaired;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnInvincibilityStart OnInvincibilityStart;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnInvincibilityEnd OnInvincibilityEnd;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnCollisionEffectsTriggered OnCollisionEffectsTriggered;

	// ==========================================
	// VEHICLE REGISTRATION
	// ==========================================
	// Vehicles must be registered before they can participate in collisions.
	// Call RegisterVehicle when a vehicle spawns, UnregisterVehicle when it's destroyed.

	/**
	 * Register a vehicle with the collision system.
	 * Call this when a vehicle spawns into the world.
	 * @param VehicleId Unique string identifier for this vehicle
	 * @param MaxHealth Maximum health pool (default 100)
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Vehicle")
	void RegisterVehicle(const FString& VehicleId, float MaxHealth);

	/**
	 * Unregister a vehicle from the collision system.
	 * Call this when a vehicle is permanently destroyed/removed.
	 * @param VehicleId The vehicle to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Vehicle")
	void UnregisterVehicle(const FString& VehicleId);

	/**
	 * Get the complete collision state for a vehicle.
	 * @param VehicleId The vehicle to query
	 * @return Copy of the vehicle's collision state
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Vehicle")
	FMGVehicleCollisionState GetVehicleState(const FString& VehicleId) const;

	/**
	 * Check if a vehicle is registered with the system.
	 * @param VehicleId The vehicle to check
	 * @return True if registered
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Vehicle")
	bool IsVehicleRegistered(const FString& VehicleId) const;

	// ==========================================
	// COLLISION PROCESSING
	// ==========================================
	// These functions are the main entry points for collision handling.
	// Call these when physics detects a collision.

	/**
	 * Process a collision event - main entry point for collision handling.
	 * Calculates damage, triggers effects, and returns detailed event data.
	 * @param PlayerId The player/vehicle that was hit
	 * @param OtherEntityId What they hit (another vehicle, wall, etc.)
	 * @param ImpactLocation World-space collision point
	 * @param ImpactNormal Surface normal at collision
	 * @param ImpactVelocity Vehicle velocity at impact
	 * @param Type Type of collision
	 * @return Complete collision event data
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Processing")
	FMGCollisionEvent ProcessCollision(const FString& PlayerId, const FString& OtherEntityId, FVector ImpactLocation, FVector ImpactNormal, FVector ImpactVelocity, EMGCollisionType Type);

	/**
	 * Process a vehicle-to-vehicle collision (both vehicles affected).
	 * Use this for crashes between two racers.
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Processing")
	void ProcessVehicleToVehicle(const FString& VehicleA, const FString& VehicleB, FVector ImpactLocation, FVector RelativeVelocity);

	/**
	 * Process a vehicle hitting a static object (wall, barrier, etc.).
	 * Use this for crashes into scenery.
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Processing")
	void ProcessVehicleToStatic(const FString& VehicleId, FVector ImpactLocation, FVector ImpactNormal, FVector Velocity);

	// ==========================================
	// COLLISION DETECTION HELPERS
	// ==========================================
	// Utility functions to classify and analyze collisions.

	/**
	 * Determine what type of collision occurred based on geometry.
	 * @param ImpactNormal Surface normal at collision
	 * @param VehicleForward Vehicle's forward direction
	 * @param RelativeVelocity Speed difference between objects
	 * @return Detected collision type
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Detection")
	EMGCollisionType DetectCollisionType(FVector ImpactNormal, FVector VehicleForward, FVector RelativeVelocity) const;

	/**
	 * Calculate severity based on impact physics.
	 * @param ImpactSpeed Speed at impact (km/h)
	 * @param ImpactForce Force of impact (Newtons)
	 * @return Calculated severity level
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Detection")
	EMGCollisionSeverity CalculateSeverity(float ImpactSpeed, float ImpactForce) const;

	/**
	 * Determine which zone of the vehicle was hit.
	 * @param LocalImpactPoint Impact point in vehicle-local coordinates
	 * @return The zone that was hit
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Detection")
	EMGImpactZone DetermineImpactZone(FVector LocalImpactPoint) const;

	// ==========================================
	// DAMAGE SYSTEM
	// ==========================================
	// Functions for applying and querying damage.

	/**
	 * Apply damage to a specific zone of a vehicle.
	 * @param VehicleId The vehicle to damage
	 * @param DamageAmount How much damage to apply
	 * @param Zone Which zone to damage
	 * @return Actual damage applied (after armor)
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Damage")
	float ApplyDamage(const FString& VehicleId, float DamageAmount, EMGImpactZone Zone);

	/**
	 * Repair a vehicle by a certain amount.
	 * @param VehicleId The vehicle to repair
	 * @param RepairAmount How much health to restore
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Damage")
	void RepairVehicle(const FString& VehicleId, float RepairAmount);

	/**
	 * Repair a specific zone of a vehicle.
	 * @param VehicleId The vehicle to repair
	 * @param Zone Which zone to repair
	 * @param RepairAmount How much health to restore
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Damage")
	void RepairZone(const FString& VehicleId, EMGImpactZone Zone, float RepairAmount);

	/**
	 * Fully repair a vehicle to pristine condition.
	 * @param VehicleId The vehicle to repair
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Damage")
	void FullRepair(const FString& VehicleId);

	/**
	 * Get total health percentage for a vehicle.
	 * @param VehicleId The vehicle to query
	 * @return Health as percentage (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Damage")
	float GetTotalHealth(const FString& VehicleId) const;

	/**
	 * Get health for a specific zone.
	 * @param VehicleId The vehicle to query
	 * @param Zone Which zone to check
	 * @return Zone health percentage
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Damage")
	float GetZoneHealth(const FString& VehicleId, EMGImpactZone Zone) const;

	/**
	 * Get overall damage state for a vehicle.
	 * @param VehicleId The vehicle to query
	 * @return Current damage state
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Damage")
	EMGDamageState GetDamageState(const FString& VehicleId) const;

	/**
	 * Check if a vehicle is completely destroyed.
	 * @param VehicleId The vehicle to check
	 * @return True if wrecked
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Damage")
	bool IsWrecked(const FString& VehicleId) const;

	// ==========================================
	// TAKEDOWN SYSTEM
	// ==========================================
	// Functions for tracking and scoring takedowns.

	/**
	 * Register a successful takedown event.
	 * Called when a player wrecks an opponent through collision.
	 * @param AttackerId Player who dealt the takedown
	 * @param VictimId Player who was taken down
	 * @param Collision The collision that caused the takedown
	 * @return Complete takedown event data with scoring
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Takedown")
	FMGTakedownEvent RegisterTakedown(const FString& AttackerId, const FString& VictimId, const FMGCollisionEvent& Collision);

	/**
	 * Get current takedown chain count for a player.
	 * Chain = consecutive takedowns within time window.
	 * @param PlayerId The player to query
	 * @return Number of takedowns in current chain
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Takedown")
	int32 GetTakedownChainCount(const FString& PlayerId) const;

	/**
	 * Get current chain score multiplier for a player.
	 * @param PlayerId The player to query
	 * @return Current multiplier (1.0 = no chain, higher = better)
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Takedown")
	float GetTakedownChainMultiplier(const FString& PlayerId) const;

	/**
	 * Check if a player has a revenge target (someone who took them down).
	 * Taking down a revenge target awards bonus points.
	 * @param PlayerId The player checking
	 * @param TargetId The potential revenge target
	 * @return True if TargetId took down PlayerId recently
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Takedown")
	bool IsRevengeTarget(const FString& PlayerId, const FString& TargetId) const;

	/**
	 * Reset a player's takedown chain (called on crash or timeout).
	 * @param PlayerId The player to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Takedown")
	void ClearTakedownChain(const FString& PlayerId);

	// ==========================================
	// INVINCIBILITY SYSTEM
	// ==========================================
	// Temporary invincibility after respawning or special events.

	/**
	 * Grant temporary invincibility to a vehicle.
	 * Used after respawning to prevent spawn-killing.
	 * @param VehicleId The vehicle to protect
	 * @param Duration How long invincibility lasts (seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Invincibility")
	void GrantInvincibility(const FString& VehicleId, float Duration);

	/**
	 * Check if a vehicle is currently invincible.
	 * @param VehicleId The vehicle to check
	 * @return True if invincible (cannot take damage)
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Invincibility")
	bool IsInvincible(const FString& VehicleId) const;

	/**
	 * Get remaining invincibility time.
	 * @param VehicleId The vehicle to check
	 * @return Seconds remaining (0 if not invincible)
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Invincibility")
	float GetRemainingInvincibility(const FString& VehicleId) const;

	// ==========================================
	// PHYSICS RESPONSE
	// ==========================================
	// Calculate how vehicles should react to collisions.

	/**
	 * Calculate post-collision bounce velocity.
	 * Used to determine where the vehicle goes after hitting something.
	 * @param InVelocity Velocity before collision
	 * @param ImpactNormal Surface normal at impact
	 * @param Restitution Bounciness (0-1)
	 * @return Resulting velocity after bounce
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Physics")
	FVector CalculateBounceVelocity(FVector InVelocity, FVector ImpactNormal, float Restitution) const;

	/**
	 * Calculate spin impulse from a collision.
	 * Off-center hits cause the vehicle to spin.
	 * @param Collision The collision event
	 * @return Rotational impulse to apply
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Physics")
	FRotator CalculateSpinImpulse(const FMGCollisionEvent& Collision) const;

	/**
	 * Calculate how much speed is lost from a collision.
	 * @param Collision The collision event
	 * @return Speed loss in km/h
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Physics")
	float CalculateSpeedLoss(const FMGCollisionEvent& Collision) const;

	// ==========================================
	// EFFECTS SYSTEM
	// ==========================================
	// Manage visual and audio effects for crashes.

	/**
	 * Register a crash effect definition.
	 * Called at game start to set up effect configurations.
	 * @param Effect The effect configuration to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Effects")
	void RegisterCrashEffect(const FMGCrashEffect& Effect);

	/**
	 * Get the appropriate crash effect for a severity level.
	 * @param Severity The collision severity
	 * @return Effect configuration to use
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Effects")
	FMGCrashEffect GetCrashEffect(EMGCollisionSeverity Severity) const;

	/**
	 * Trigger crash effects for a collision.
	 * Spawns particles, plays sounds, triggers haptics.
	 * @param Collision The collision event
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Effects")
	void TriggerCrashEffects(const FMGCollisionEvent& Collision);

	// ==========================================
	// STATISTICS
	// ==========================================
	// Track and query player collision statistics.

	/**
	 * Get collision statistics for a player.
	 * Used for end-of-race summaries and achievements.
	 * @param PlayerId The player to query
	 * @return Complete stats structure
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Stats")
	FMGCollisionStats GetPlayerStats(const FString& PlayerId) const;

	/**
	 * Reset a player's collision statistics.
	 * @param PlayerId The player to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	/**
	 * Get recent collision events for a player.
	 * Used for replay/highlight systems.
	 * @param PlayerId The player to query
	 * @param MaxCount Maximum number of events to return
	 * @return Array of recent collisions (newest first)
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Stats")
	TArray<FMGCollisionEvent> GetRecentCollisions(const FString& PlayerId, int32 MaxCount) const;

	// ==========================================
	// CONFIGURATION
	// ==========================================
	// Runtime configuration for collision behavior.

	/**
	 * Set physics configuration for collisions.
	 * @param Config New physics configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Config")
	void SetPhysicsConfig(const FMGCollisionPhysicsConfig& Config);

	/**
	 * Get current physics configuration.
	 * @return Current physics config
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Config")
	FMGCollisionPhysicsConfig GetPhysicsConfig() const;

	/**
	 * Set scoring configuration for takedowns.
	 * @param Config New scoring configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Config")
	void SetScoringConfig(const FMGCollisionScoringConfig& Config);

	/**
	 * Get current scoring configuration.
	 * @return Current scoring config
	 */
	UFUNCTION(BlueprintPure, Category = "Collision|Config")
	FMGCollisionScoringConfig GetScoringConfig() const;

	// ==========================================
	// UPDATE
	// ==========================================
	// Called each frame to update timers and states.

	/**
	 * Update the collision system.
	 * Called each frame to update invincibility timers, fire damage, etc.
	 * @param DeltaTime Time since last frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Update")
	void UpdateCollisionSystem(float DeltaTime);

	// ==========================================
	// SAVE/LOAD
	// ==========================================
	// Persistence for collision data.

	/**
	 * Save collision data to disk.
	 * Called during game save.
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Persistence")
	void SaveCollisionData();

	/**
	 * Load collision data from disk.
	 * Called during game load.
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision|Persistence")
	void LoadCollisionData();

protected:
	// ==========================================
	// INTERNAL HELPER FUNCTIONS
	// ==========================================
	// These are implementation details - called internally, not from outside.

	/** Main tick function - updates all time-based systems */
	void TickCollision(float DeltaTime);

	/** Update invincibility timers and expire them when done */
	void UpdateInvincibility(float DeltaTime);

	/** Update takedown chain timers and reset expired chains */
	void UpdateTakedownChains(float DeltaTime);

	/** Apply ongoing fire damage to burning vehicles */
	void UpdateFireDamage(float DeltaTime);

	/** Calculate damage state from health percentage */
	EMGDamageState CalculateDamageState(float HealthPercent) const;

	/** Check if a collision should result in a wreck */
	void CheckWreckCondition(const FString& VehicleId, const FMGCollisionEvent& Collision);

	/** Recalculate overall vehicle state from all zones */
	void UpdateVehicleState(const FString& VehicleId);

	/** Update player statistics after a collision */
	void UpdatePlayerStats(const FString& PlayerId, const FMGCollisionEvent& Collision);

	/** Generate a unique ID for a new collision event */
	FString GenerateCollisionId() const;

	/** Generate a unique ID for a new takedown event */
	FString GenerateTakedownId() const;

private:
	// ==========================================
	// INTERNAL DATA STORAGE
	// ==========================================
	// These maps store all the runtime data for the collision system.
	// They are managed automatically - you don't need to access them directly.

	/** Collision state for each registered vehicle (keyed by VehicleId) */
	UPROPERTY()
	TMap<FString, FMGVehicleCollisionState> VehicleStates;

	/** Remaining invincibility time for each vehicle (seconds) */
	UPROPERTY()
	TMap<FString, float> InvincibilityTimers;

	/** Current takedown chain count for each player */
	UPROPERTY()
	TMap<FString, int32> TakedownChains;

	/** Remaining time in takedown chain for each player (seconds) */
	UPROPERTY()
	TMap<FString, float> TakedownChainTimers;

	/** Revenge targets: PlayerId -> who took them down last */
	UPROPERTY()
	TMap<FString, FString> RevengeTargets;

	/** Cumulative statistics for each player */
	UPROPERTY()
	TMap<FString, FMGCollisionStats> PlayerStats;

	/** Recent collision history for each player (for replay/stats) */
	UPROPERTY()
	TMap<FString, TArray<FMGCollisionEvent>> RecentCollisions;

	/** Registered crash effects (matched by severity) */
	UPROPERTY()
	TArray<FMGCrashEffect> CrashEffects;

	/** Current physics configuration */
	UPROPERTY()
	FMGCollisionPhysicsConfig PhysicsConfig;

	/** Current scoring configuration */
	UPROPERTY()
	FMGCollisionScoringConfig ScoringConfig;

	/** Counter for generating unique collision IDs */
	UPROPERTY()
	int32 CollisionCounter = 0;

	/** Counter for generating unique takedown IDs */
	UPROPERTY()
	int32 TakedownCounter = 0;

	/** Timer handle for the tick function */
	FTimerHandle CollisionTickTimer;
};

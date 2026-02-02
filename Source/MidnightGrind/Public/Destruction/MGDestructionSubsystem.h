// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * ============================================================================
 * MGDestructionSubsystem.h
 * ============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the Destruction Subsystem - a system that manages all
 * destructible environmental objects in the game world. Think of it as the
 * system that handles everything you can crash through or destroy: trash cans,
 * fences, hydrants, billboards, scaffolding, and more.
 *
 * When you drive through a row of garbage cans and they go flying, this
 * subsystem is what makes that happen, tracks the points you earn, and
 * manages the combo system for chaining destructions together.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. DESTRUCTIBLE OBJECTS:
 *    - Static objects in the world that can be destroyed by vehicles
 *    - Each type has different properties (health, points, effects)
 *    - Some are easy to destroy (cardboard boxes), some are harder (concrete barriers)
 *
 * 2. DEFINITION vs INSTANCE:
 *    - DEFINITION: The template (e.g., "FireHydrant" - 50 HP, 25 points, water spray)
 *    - INSTANCE: An actual hydrant placed in the world at a specific location
 *    - One definition can have many instances throughout the game world
 *
 * 3. DESTRUCTION CATEGORIES:
 *    - Minor: Small props (cones, trash, boxes) - few points
 *    - Standard: Medium objects (fences, signs) - moderate points
 *    - Major: Large objects (billboards, scaffolding) - good points
 *    - Spectacular: Impressive destructions (chain reactions) - bonus points
 *    - Legendary: Epic destructions - achievement-worthy
 *
 * 4. COMBO SYSTEM:
 *    - Destroy multiple objects quickly to build a combo multiplier
 *    - Each destruction extends the combo timer
 *    - Higher combo = more points per destruction
 *    - Combo resets if you go too long without destroying something
 *
 * 5. CHAIN REACTIONS:
 *    - Some objects can trigger others when destroyed
 *    - Example: Destroying a gas pump might ignite nearby barrels
 *    - Chain reactions award bonus points and look spectacular
 *
 * 6. DESTRUCTION ZONES:
 *    - Areas of the map with grouped destructibles
 *    - Destroying all objects in a zone awards a completion bonus
 *    - Encourages exploration and thorough destruction
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *   [Vehicle Physics]
 *         |
 *         v (vehicle hits destructible)
 *   [UMGDestructionSubsystem] <--- This file!
 *         |
 *         +---> [Scoring System] - Awards points, tracks combos
 *         |
 *         +---> [VFX System] - Spawns debris, particles, effects
 *         |
 *         +---> [Audio System] - Plays destruction sounds
 *         |
 *         +---> [HUD] - Shows points popup, combo counter
 *         |
 *         +---> [Damage System] - May damage the vehicle slightly
 *
 * TYPICAL USAGE FLOW:
 * -------------------
 * 1. Game loads: Definitions are registered for each destructible type
 * 2. Level loads: Instances are spawned at their world locations
 * 3. Player drives into object: TryDestroyOnImpact() is called
 * 4. If destroyed: DestroyDestructible() handles effects and scoring
 * 5. Combo system extends if player destroys more quickly
 * 6. Chain reactions trigger if applicable
 * 7. Zone progress updates if object was in a zone
 *
 * EXAMPLE CODE:
 * -------------
 * // Register a destructible definition (usually done at game start)
 * FMGDestructibleDefinition Hydrant;
 * Hydrant.DestructibleId = "FireHydrant";
 * Hydrant.DisplayName = FText::FromString("Fire Hydrant");
 * Hydrant.Type = EMGDestructibleType::Hydrant;
 * Hydrant.Health = 50.0f;
 * Hydrant.BasePoints = 25;
 * Hydrant.DestructionEffect = EMGDestructionEffect::Spray;
 * DestructionSubsystem->RegisterDestructibleDefinition(Hydrant);
 *
 * // Spawn an instance in the world
 * FString InstanceId = DestructionSubsystem->SpawnDestructible(
 *     "FireHydrant",
 *     FVector(1000, 500, 0),  // Location
 *     FRotator::ZeroRotator    // Rotation
 * );
 *
 * // When vehicle hits it:
 * if (DestructionSubsystem->TryDestroyOnImpact(InstanceId, PlayerId, Velocity, Force))
 * {
 *     // Object was destroyed!
 * }
 *
 * @see UMGCollisionSubsystem - Handles vehicle-to-vehicle collisions
 * @see UMGDamageSubsystem - Handles vehicle damage
 * @see UMGTakedownSubsystem - Handles vehicle takedowns
 * ============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDestructionSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * Destructible type - Categories of objects that can be destroyed.
 *
 * This enum helps organize destructibles by what they are. Objects of the
 * same type typically share similar behaviors and sound effects.
 */
UENUM(BlueprintType)
enum class EMGDestructibleType : uint8
{
	/** Invalid/unset type */
	None				UMETA(DisplayName = "None"),

	/** Generic small prop (cones, trash cans, boxes) */
	Prop				UMETA(DisplayName = "Prop"),

	/** Benches, planters, newspaper stands, etc. */
	StreetFurniture		UMETA(DisplayName = "Street Furniture"),

	/** Chain-link, wooden, or metal fences */
	Fence				UMETA(DisplayName = "Fence"),

	/** Traffic barriers, construction barriers */
	Barrier				UMETA(DisplayName = "Barrier"),

	/** Street signs, road signs, shop signs */
	Sign				UMETA(DisplayName = "Sign"),

	/** Large advertising billboards */
	Billboard			UMETA(DisplayName = "Billboard"),

	/** Parked/stationary vehicles (not traffic) */
	Vehicle				UMETA(DisplayName = "Vehicle"),

	/** Wooden or cardboard crates */
	Crate				UMETA(DisplayName = "Crate"),

	/** Shipping containers, dumpsters */
	Container			UMETA(DisplayName = "Container"),

	/** Small trees, bushes, hedges */
	Vegetation			UMETA(DisplayName = "Vegetation"),

	/** Destructible building elements (awnings, overhangs) */
	Building			UMETA(DisplayName = "Building Part"),

	/** Telephone/power poles (can trigger chain reactions) */
	UtilityPole			UMETA(DisplayName = "Utility Pole"),

	/** Fire hydrants (spray water effect) */
	Hydrant				UMETA(DisplayName = "Fire Hydrant"),

	/** Traffic lights and their poles */
	TrafficLight		UMETA(DisplayName = "Traffic Light"),

	/** Bus stop shelters and benches */
	BusStop				UMETA(DisplayName = "Bus Stop"),

	/** Newsstands, phone booths, ATMs */
	Kiosk				UMETA(DisplayName = "Kiosk"),

	/** Construction scaffolding (often triggers chain collapse) */
	Scaffolding			UMETA(DisplayName = "Scaffolding")
};

/**
 * Destruction category for scoring - How significant the destruction was.
 *
 * Higher categories award more points and may trigger special UI effects.
 * Categories are determined by the type of object and circumstances
 * (chain reactions can upgrade the category).
 */
UENUM(BlueprintType)
enum class EMGDestructionCategory : uint8
{
	/** Small/easy objects (cones, signs, small boxes) - few points */
	Minor				UMETA(DisplayName = "Minor"),

	/** Regular objects (benches, hydrants, fences) - moderate points */
	Standard			UMETA(DisplayName = "Standard"),

	/** Large/hard objects (billboards, scaffolding) - good points */
	Major				UMETA(DisplayName = "Major"),

	/** Impressive destructions (chain reactions, multiple objects) - bonus */
	Spectacular			UMETA(DisplayName = "Spectacular"),

	/** Epic destructions (massive chain reactions) - achievement-worthy */
	Legendary			UMETA(DisplayName = "Legendary")
};

/**
 * Destruction effect type - Visual effect when object is destroyed.
 *
 * Different materials and objects break in different ways. This enum
 * tells the VFX system what kind of destruction effect to play.
 */
UENUM(BlueprintType)
enum class EMGDestructionEffect : uint8
{
	/** No special effect (object just disappears) */
	None				UMETA(DisplayName = "None"),

	/** Glass/ceramic breaking into pieces */
	Shatter				UMETA(DisplayName = "Shatter"),

	/** Explosive destruction with fire/smoke */
	Explode				UMETA(DisplayName = "Explode"),

	/** Concrete/brick falling apart */
	Crumble				UMETA(DisplayName = "Crumble"),

	/** Structure falling over (scaffolding, poles) */
	Collapse			UMETA(DisplayName = "Collapse"),

	/** Metal bending/crushing */
	Deform				UMETA(DisplayName = "Deform"),

	/** Catches fire and burns */
	Burn				UMETA(DisplayName = "Burn"),

	/** Wood breaking into splinters */
	Splinter			UMETA(DisplayName = "Splinter"),

	/** Water spraying (fire hydrants) */
	Spray				UMETA(DisplayName = "Water Spray")
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Destructible definition - Template for a type of destructible object.
 *
 * This is like a "blueprint" for a destructible. It defines all the properties
 * that instances of this type will have. For example, all fire hydrants share
 * the same definition, but each one in the world is a separate instance.
 *
 * Definitions are typically created in data assets or registered at game start.
 */
USTRUCT(BlueprintType)
struct FMGDestructibleDefinition
{
	GENERATED_BODY()

	/** Unique identifier for this definition (e.g., "FireHydrant", "WoodenFence") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DestructibleId;

	/** Human-readable name shown in UI (e.g., "Fire Hydrant") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Category of object (fence, sign, hydrant, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructibleType Type = EMGDestructibleType::Prop;

	/** Scoring category (minor, standard, major, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructionCategory Category = EMGDestructionCategory::Minor;

	/** Visual effect when destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructionEffect DestructionEffect = EMGDestructionEffect::Shatter;

	/** How much damage it takes to destroy (0 = instant destruction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	/** Mass in kg (affects physics response when hit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mass = 50.0f;

	/** Minimum vehicle speed (km/h) required to destroy this object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinImpactSpeed = 20.0f;

	/** Base points awarded for destroying this object (before multipliers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 10;

	/** Speed multiplier when driving through (0.95 = 5% slowdown) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowdownFactor = 0.95f;

	/** Damage dealt to the vehicle when destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageToVehicle = 5.0f;

	/** Can this object trigger chain reactions when destroyed? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanChainReact = false;

	/** Radius to check for chain reaction targets (in cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainReactRadius = 0.0f;

	/** List of DestructibleIds that this can trigger in chain reaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ChainReactTriggers;

	/** Time (seconds) before object respawns (0 = never respawns) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 0.0f;

	/** If true, destroyed state persists and blocks respawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBlocksRespawn = false;

	/** Mesh to show after destruction (broken pieces, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> DestroyedMesh;

	/** Particle effect to play when destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> DestructionVFX;

	/** Sound effect to play when destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> DestructionSFX;
};

/**
 * Destructible instance - A specific destructible object placed in the world.
 *
 * While FMGDestructibleDefinition is the template, this struct represents
 * an actual object at a specific location. The game world might have
 * hundreds of instances, each tracking its own state independently.
 *
 * Think of it like: Definition = "Fire Hydrant (the concept)"
 *                   Instance = "The fire hydrant at 5th and Main"
 */
USTRUCT(BlueprintType)
struct FMGDestructibleInstance
{
	GENERATED_BODY()

	/** Unique ID for this specific instance (auto-generated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceId;

	/** ID of the definition this instance uses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DestructibleId;

	/** World-space position of this instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** World-space rotation of this instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Scale of this instance (usually 1,1,1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	/** Current health (from definition's max health) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth = 100.0f;

	/** True if this instance has been destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDestroyed = false;

	/** True if currently in respawn cooldown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRespawning = false;

	/** Seconds remaining until respawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTimer = 0.0f;

	/** Player who destroyed this instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DestroyedByPlayerId;

	/** When this instance was destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime DestructionTime;
};

/**
 * Destruction event - Data about a single destruction occurrence.
 *
 * Created whenever something is destroyed. Contains all the information
 * needed for:
 * - Scoring (points, multipliers)
 * - UI display ("DESTROYED!" popup with points)
 * - Statistics tracking
 * - Achievement checking
 */
USTRUCT(BlueprintType)
struct FMGDestructionEvent
{
	GENERATED_BODY()

	/** Unique ID for this destruction event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	/** Player who caused the destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/** Instance that was destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceId;

	/** Definition ID of what was destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DestructibleId;

	/** Type of object destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructibleType Type = EMGDestructibleType::None;

	/** Scoring category for this destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructionCategory Category = EMGDestructionCategory::Minor;

	/** Where the destruction occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** Vehicle velocity when hitting the object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactVelocity = FVector::ZeroVector;

	/** Vehicle speed in km/h at impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactSpeed = 0.0f;

	/** Total points earned (base * multipliers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsEarned = 0;

	/** Active combo multiplier when destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplier = 1.0f;

	/** Position in current chain reaction (0 if not chain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainCount = 0;

	/** True if this was triggered by a chain reaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasChainReaction = false;

	/** When the destruction occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Destruction combo - Tracks a player's current destruction streak.
 *
 * When you destroy multiple objects quickly, you build a combo. Each
 * destruction within the time window adds to the count and increases
 * the multiplier. Stop destroying things and the combo ends.
 *
 * Example: Destroy 5 objects quickly = 5x combo = 5x points on the 5th object
 */
USTRUCT(BlueprintType)
struct FMGDestructionCombo
{
	GENERATED_BODY()

	/** Player who owns this combo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/** Number of objects destroyed in this combo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCount = 0;

	/** Current point multiplier (increases with each destruction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplier = 1.0f;

	/** Seconds until combo expires (resets on each destruction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	/** Total points earned during this combo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	/** All destruction events in this combo (for replay/UI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDestructionEvent> ComboEvents;
};

/**
 * Player destruction stats - Cumulative destruction statistics for one player.
 *
 * Used for:
 * - End-of-race summaries ("You destroyed 47 objects!")
 * - Achievements ("Destroy 100 hydrants")
 * - Leaderboards ("Most property damage")
 * - Player profile stats
 */
USTRUCT(BlueprintType)
struct FMGDestructionStats
{
	GENERATED_BODY()

	/** Player these stats belong to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/** Count of each type destroyed (hydrants: 5, fences: 12, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDestructibleType, int32> TypeCounts;

	/** Count of each category destroyed (minor: 30, major: 5, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDestructionCategory, int32> CategoryCounts;

	/** Total objects destroyed (all types) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDestroyed = 0;

	/** Total points earned from destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	/** Highest combo count achieved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestCombo = 0;

	/** Longest chain reaction triggered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestChainReaction = 0;

	/** Total value of property destroyed (for "property damage" stat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalPropertyDamage = 0.0f;

	/** Count of spectacular/legendary destructions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpectacularDestructions = 0;
};

/**
 * Destruction zone - A group of destructibles in an area.
 *
 * Zones encourage players to be thorough by offering a bonus for
 * destroying everything in an area. For example, a construction site
 * might be a zone - destroy all the barriers and scaffolding to get
 * a "Zone Complete!" bonus.
 */
USTRUCT(BlueprintType)
struct FMGDestructionZone
{
	GENERATED_BODY()

	/** Unique ID for this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ZoneId;

	/** Display name shown in UI ("Construction Site") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ZoneName;

	/** World-space center of this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	/** Radius of the zone in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 5000.0f;

	/** Point multiplier for objects in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointMultiplier = 1.0f;

	/** List of instance IDs in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> DestructibleInstances;

	/** Total number of destructibles in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDestructibles = 0;

	/** How many have been destroyed so far */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DestroyedCount = 0;

	/** True if all objects in zone have been destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCompleted = false;

	/** Bonus points awarded when zone is completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletionBonus = 0;
};

/**
 * Chain reaction - A series of destructions triggered by one initial impact.
 *
 * Some objects can trigger others when destroyed. For example:
 * - Gas pump explodes -> ignites nearby barrels
 * - Scaffolding collapses -> knocks over utility poles
 * - Utility pole falls -> triggers another pole to fall
 *
 * Chain reactions award bonus points and look spectacular!
 */
USTRUCT(BlueprintType)
struct FMGChainReaction
{
	GENERATED_BODY()

	/** Unique ID for this chain reaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainId;

	/** Player who started the chain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InitiatorPlayerId;

	/** All instances destroyed in this chain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AffectedInstances;

	/** How many objects were destroyed in the chain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainLength = 0;

	/** Total points earned from the entire chain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	/** How long the chain has been running (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainDuration = 0.0f;

	/** True if chain is still in progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;
};

/**
 * Destruction scoring config - Tuning parameters for destruction scoring.
 *
 * Designers can adjust these values to change how rewarding destruction is.
 * Higher multipliers make destruction more lucrative, encouraging mayhem.
 */
USTRUCT(BlueprintType)
struct FMGDestructionScoringConfig
{
	GENERATED_BODY()

	/** Seconds to continue a combo (resets on each destruction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboWindowSeconds = 2.0f;

	/** Multiplier increase per destruction (0.1 = +10% per hit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplierPerHit = 0.1f;

	/** Maximum combo multiplier (caps the bonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxComboMultiplier = 5.0f;

	/** Speed (km/h) threshold for speed bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusThreshold = 100.0f;

	/** Multiplier when destroying at high speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusMultiplier = 1.5f;

	/** Multiplier for chain reaction destructions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainReactionMultiplier = 2.0f;

	/** Combo count required for "Spectacular" bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpectacularThreshold = 5;

	/** Bonus points for reaching spectacular threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpectacularBonus = 500;

	/** Base points for each destruction category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDestructionCategory, int32> CategoryBasePoints;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================
// Delegates allow other systems (UI, audio, VFX) to respond to destruction events.
// Subscribe to these events to react when things get destroyed.
//
// Usage in Blueprint: Bind to the event in the Event Graph
// Usage in C++: SubsystemPtr->OnDestructibleDestroyed.AddDynamic(this, &MyClass::HandleDestruction);
// ============================================================================

/** Fired when any destructible is destroyed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructibleDestroyed, const FString&, PlayerId, const FMGDestructionEvent&, Event);

/** Fired when a player's combo count/multiplier changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDestructionComboUpdated, const FString&, PlayerId, int32, ComboCount, float, Multiplier);

/** Fired when a combo ends (timed out or player crashed) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionComboEnded, const FString&, PlayerId, int32, TotalPoints);

/** Fired when a chain reaction begins */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChainReactionStarted, const FString&, PlayerId, const FString&, ChainId);

/** Fired each time a chain reaction destroys another object */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChainReactionExtended, const FString&, ChainId, int32, ChainLength, int32, NewPoints);

/** Fired when a chain reaction finishes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChainReactionEnded, const FString&, ChainId, int32, FinalLength, int32, TotalPoints);

/** Fired when progress is made in a destruction zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionZoneProgress, const FString&, ZoneId, float, CompletionPercent);

/** Fired when all objects in a zone are destroyed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionZoneCompleted, const FString&, ZoneId, int32, BonusPoints);

/** Fired when a spectacular destruction is achieved */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpectacularDestruction, const FString&, PlayerId, int32, BonusPoints);

/** Fired when a destructible respawns */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructibleRespawned, const FString&, InstanceId, FVector, Location);

// ============================================================================
// SUBSYSTEM CLASS
// ============================================================================

/**
 * Destruction Subsystem - Manages all destructible environmental objects.
 *
 * This subsystem is responsible for:
 * - Registering destructible definitions (templates)
 * - Spawning and tracking destructible instances (actual objects)
 * - Processing destruction events
 * - Managing combo and chain reaction systems
 * - Tracking zones and zone completion
 * - Calculating and awarding points
 *
 * ## How to Use
 *
 * Get the subsystem from anywhere:
 * @code
 * UMGDestructionSubsystem* DestructionSys = GetGameInstance()->GetSubsystem<UMGDestructionSubsystem>();
 * @endcode
 *
 * Register definitions at game start:
 * @code
 * FMGDestructibleDefinition TrashCan;
 * TrashCan.DestructibleId = "TrashCan";
 * TrashCan.Type = EMGDestructibleType::Prop;
 * TrashCan.BasePoints = 10;
 * DestructionSys->RegisterDestructibleDefinition(TrashCan);
 * @endcode
 *
 * Spawn instances when loading a level:
 * @code
 * FString InstanceId = DestructionSys->SpawnDestructible("TrashCan", Location, Rotation);
 * @endcode
 *
 * Handle collision with destructible:
 * @code
 * if (DestructionSys->TryDestroyOnImpact(InstanceId, PlayerId, Velocity, Force))
 * {
 *     // Destroyed! Effects and scoring are handled automatically
 * }
 * @endcode
 *
 * ## Thread Safety
 * All functions must be called from the game thread only.
 *
 * @see UMGCollisionSubsystem - Handles vehicle collisions
 * @see UMGDamageSubsystem - Handles vehicle damage
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDestructionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// LIFECYCLE FUNCTIONS
	// ==========================================

	/** Called when the subsystem is created (game start) */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called when the subsystem is destroyed (game end) */
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS (DELEGATES)
	// ==========================================
	// Subscribe to these to react when destruction happens.
	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructibleDestroyed OnDestructibleDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructionComboUpdated OnDestructionComboUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructionComboEnded OnDestructionComboEnded;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnChainReactionStarted OnChainReactionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnChainReactionExtended OnChainReactionExtended;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnChainReactionEnded OnChainReactionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructionZoneProgress OnDestructionZoneProgress;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructionZoneCompleted OnDestructionZoneCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnSpectacularDestruction OnSpectacularDestruction;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructibleRespawned OnDestructibleRespawned;

	// ==========================================
	// DEFINITION REGISTRATION
	// ==========================================
	// Definitions are templates that describe types of destructibles.
	// Register them once at game start.

	/**
	 * Register a destructible definition (template).
	 * Call this once for each type of destructible object in your game.
	 * @param Definition The definition to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Definition")
	void RegisterDestructibleDefinition(const FMGDestructibleDefinition& Definition);

	/**
	 * Get a definition by its ID.
	 * @param DestructibleId The definition to look up
	 * @return Copy of the definition
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Definition")
	FMGDestructibleDefinition GetDestructibleDefinition(const FString& DestructibleId) const;

	/**
	 * Get all registered definitions.
	 * @return Array of all definitions
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Definition")
	TArray<FMGDestructibleDefinition> GetAllDefinitions() const;

	// ==========================================
	// INSTANCE MANAGEMENT
	// ==========================================
	// Instances are actual objects placed in the world.

	/**
	 * Spawn a destructible instance at a location.
	 * @param DestructibleId Which definition to use
	 * @param Location World-space position
	 * @param Rotation World-space rotation
	 * @return Unique ID for the new instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Instance")
	FString SpawnDestructible(const FString& DestructibleId, FVector Location, FRotator Rotation);

	/**
	 * Remove a destructible instance from the world.
	 * @param InstanceId The instance to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Instance")
	void RemoveDestructible(const FString& InstanceId);

	/**
	 * Get an instance's current state.
	 * @param InstanceId The instance to query
	 * @return Copy of instance data
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Instance")
	FMGDestructibleInstance GetDestructibleInstance(const FString& InstanceId) const;

	/**
	 * Find all destructibles within a radius.
	 * Useful for area effects or chain reactions.
	 * @param Center Point to search from
	 * @param Radius Search radius in cm
	 * @return Array of instances in range
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Instance")
	TArray<FMGDestructibleInstance> GetDestructiblesInRadius(FVector Center, float Radius) const;

	/**
	 * Get all currently destroyed instances.
	 * @return Array of destroyed instances
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Instance")
	TArray<FMGDestructibleInstance> GetDestroyedInstances() const;

	// ==========================================
	// DESTRUCTION
	// ==========================================
	// Functions for destroying objects and handling impacts.

	/**
	 * Immediately destroy a destructible instance.
	 * Triggers effects, awards points, starts chain reactions.
	 * @param InstanceId Instance to destroy
	 * @param PlayerId Player causing the destruction
	 * @param ImpactVelocity Vehicle velocity at impact
	 * @return Complete destruction event data
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Destruction")
	FMGDestructionEvent DestroyDestructible(const FString& InstanceId, const FString& PlayerId, FVector ImpactVelocity);

	/**
	 * Apply damage to a destructible without destroying it.
	 * Used for objects that take multiple hits.
	 * @param InstanceId Instance to damage
	 * @param Damage Amount of damage
	 * @param PlayerId Player causing damage
	 * @return True if object was destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Destruction")
	bool DamageDestructible(const FString& InstanceId, float Damage, const FString& PlayerId);

	/**
	 * Try to destroy based on impact physics.
	 * Main entry point for vehicle collisions with destructibles.
	 * @param InstanceId Instance that was hit
	 * @param PlayerId Player who hit it
	 * @param ImpactVelocity Vehicle velocity at impact
	 * @param ImpactForce Force of impact
	 * @return True if object was destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Destruction")
	bool TryDestroyOnImpact(const FString& InstanceId, const FString& PlayerId, FVector ImpactVelocity, float ImpactForce);

	/**
	 * Check if an object can be destroyed at a given speed.
	 * @param InstanceId Instance to check
	 * @param ImpactSpeed Speed in km/h
	 * @return True if speed is sufficient
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Destruction")
	bool CanBeDestroyed(const FString& InstanceId, float ImpactSpeed) const;

	// ==========================================
	// COMBO SYSTEM
	// ==========================================
	// Tracks consecutive destructions for score multipliers.

	/**
	 * Get a player's current combo state.
	 * @param PlayerId Player to query
	 * @return Current combo data
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Combo")
	FMGDestructionCombo GetCurrentCombo(const FString& PlayerId) const;

	/**
	 * Check if a player has an active combo.
	 * @param PlayerId Player to check
	 * @return True if combo is active
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Combo")
	bool HasActiveCombo(const FString& PlayerId) const;

	/**
	 * Extend a player's combo with a new destruction.
	 * Called automatically by DestroyDestructible.
	 * @param PlayerId Player whose combo to extend
	 * @param Event The destruction event
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Combo")
	void ExtendCombo(const FString& PlayerId, const FMGDestructionEvent& Event);

	/**
	 * End a player's combo (called when timer expires).
	 * @param PlayerId Player whose combo to end
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Combo")
	void EndCombo(const FString& PlayerId);

	/**
	 * Reset a player's combo (called on crash or manual reset).
	 * @param PlayerId Player whose combo to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Combo")
	void ResetCombo(const FString& PlayerId);

	// ==========================================
	// CHAIN REACTIONS
	// ==========================================
	// Handles cascading destruction when objects trigger other objects.

	/**
	 * Start a new chain reaction.
	 * @param PlayerId Player who triggered it
	 * @param InitialInstanceId First object in the chain
	 * @return Chain reaction ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Chain")
	FString StartChainReaction(const FString& PlayerId, const FString& InitialInstanceId);

	/**
	 * Process the next step of a chain reaction.
	 * Called internally each frame while chain is active.
	 * @param ChainId Chain to process
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Chain")
	void ProcessChainReaction(const FString& ChainId);

	/**
	 * Get the current state of a chain reaction.
	 * @param ChainId Chain to query
	 * @return Chain reaction data
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Chain")
	FMGChainReaction GetChainReaction(const FString& ChainId) const;

	/**
	 * Find objects that could be triggered in a chain reaction.
	 * @param Origin Point to search from
	 * @param Radius Search radius
	 * @param ExcludeIds Instances already in chain
	 * @return IDs of chainable instances
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Chain")
	TArray<FString> GetChainableInstances(FVector Origin, float Radius, const TArray<FString>& ExcludeIds) const;

	// ==========================================
	// DESTRUCTION ZONES
	// ==========================================
	// Areas with grouped destructibles for bonus scoring.

	/**
	 * Register a destruction zone.
	 * @param Zone Zone to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Zone")
	void RegisterDestructionZone(const FMGDestructionZone& Zone);

	/**
	 * Get a zone by its ID.
	 * @param ZoneId Zone to query
	 * @return Zone data
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Zone")
	FMGDestructionZone GetDestructionZone(const FString& ZoneId) const;

	/**
	 * Get all registered zones.
	 * @return Array of all zones
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Zone")
	TArray<FMGDestructionZone> GetAllZones() const;

	/**
	 * Update zone progress after an object is destroyed.
	 * @param InstanceId Instance that was destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Zone")
	void UpdateZoneProgress(const FString& InstanceId);

	/**
	 * Get completion percentage for a zone.
	 * @param ZoneId Zone to query
	 * @return Completion percentage (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Zone")
	float GetZoneCompletionPercent(const FString& ZoneId) const;

	// ==========================================
	// SCORING
	// ==========================================
	// Point calculation and configuration.

	/**
	 * Calculate points for destroying an object.
	 * @param DestructibleId What was destroyed
	 * @param ImpactSpeed Speed at destruction
	 * @param ComboMultiplier Current combo multiplier
	 * @return Total points to award
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Scoring")
	int32 CalculateDestructionPoints(const FString& DestructibleId, float ImpactSpeed, float ComboMultiplier) const;

	/**
	 * Set the scoring configuration.
	 * @param Config New scoring configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Scoring")
	void SetScoringConfig(const FMGDestructionScoringConfig& Config);

	/**
	 * Get current scoring configuration.
	 * @return Current scoring config
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Scoring")
	FMGDestructionScoringConfig GetScoringConfig() const;

	// ==========================================
	// STATISTICS
	// ==========================================
	// Player and global destruction statistics.

	/**
	 * Get destruction statistics for a player.
	 * @param PlayerId Player to query
	 * @return Complete stats structure
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Stats")
	FMGDestructionStats GetPlayerStats(const FString& PlayerId) const;

	/**
	 * Reset a player's destruction statistics.
	 * @param PlayerId Player to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	/**
	 * Get total number of objects destroyed (all players).
	 * @return Total destroyed count
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Stats")
	int32 GetTotalDestroyedCount() const;

	/**
	 * Get total property damage value (for leaderboards).
	 * @return Total property damage in dollars
	 */
	UFUNCTION(BlueprintPure, Category = "Destruction|Stats")
	float GetTotalPropertyDamage() const;

	// ==========================================
	// RESPAWNING
	// ==========================================
	// Functions for respawning destroyed objects.

	/**
	 * Respawn a single destroyed instance.
	 * @param InstanceId Instance to respawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Respawn")
	void RespawnDestructible(const FString& InstanceId);

	/**
	 * Respawn all destroyed instances.
	 * Useful for resetting the level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Respawn")
	void RespawnAll();

	/**
	 * Respawn destroyed instances within a radius.
	 * @param Center Point to search from
	 * @param Radius Search radius in cm
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Respawn")
	void RespawnInRadius(FVector Center, float Radius);

	// ==========================================
	// UPDATE
	// ==========================================
	// Called each frame to update timers and states.

	/**
	 * Update the destruction system.
	 * Handles combo timers, respawn timers, chain reactions.
	 * @param DeltaTime Time since last frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Update")
	void UpdateDestruction(float MGDeltaTime);

	// ==========================================
	// SAVE/LOAD
	// ==========================================
	// Persistence for destruction state.

	/**
	 * Save destruction data to disk.
	 * Saves destroyed/respawning states.
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Persistence")
	void SaveDestructionData();

	/**
	 * Load destruction data from disk.
	 * Restores destroyed/respawning states.
	 */
	UFUNCTION(BlueprintCallable, Category = "Destruction|Persistence")
	void LoadDestructionData();

protected:
	// ==========================================
	// INTERNAL HELPER FUNCTIONS
	// ==========================================
	// Implementation details - not called from outside.

	/** Main tick function - updates all time-based systems */
	void TickDestruction(float MGDeltaTime);

	/** Update combo timers and expire finished combos */
	void UpdateCombos(float MGDeltaTime);

	/** Update respawn timers and respawn ready objects */
	void UpdateRespawns(float MGDeltaTime);

	/** Update active chain reactions */
	void UpdateChainReactions(float MGDeltaTime);

	/** Process one step of a chain reaction */
	void ProcessChainReactionStep(const FString& ChainId);

	/** Check if a destruction qualifies as spectacular */
	void CheckSpectacularDestruction(const FString& PlayerId, const FMGDestructionEvent& Event);

	/** Generate a unique instance ID */
	FString GenerateInstanceId() const;

	/** Generate a unique event ID */
	FString GenerateEventId() const;

	/** Generate a unique chain ID */
	FString GenerateChainId() const;

private:
	// ==========================================
	// INTERNAL DATA STORAGE
	// ==========================================
	// These maps store all runtime data. Managed automatically.

	/** All registered definitions (keyed by DestructibleId) */
	UPROPERTY()
	TMap<FString, FMGDestructibleDefinition> Definitions;

	/** All instances in the world (keyed by InstanceId) */
	UPROPERTY()
	TMap<FString, FMGDestructibleInstance> Instances;

	/** Active combos for each player (keyed by PlayerId) */
	UPROPERTY()
	TMap<FString, FMGDestructionCombo> ActiveCombos;

	/** Active chain reactions (keyed by ChainId) */
	UPROPERTY()
	TMap<FString, FMGChainReaction> ActiveChainReactions;

	/** All registered zones (keyed by ZoneId) */
	UPROPERTY()
	TMap<FString, FMGDestructionZone> Zones;

	/** Destruction statistics for each player */
	UPROPERTY()
	TMap<FString, FMGDestructionStats> PlayerStats;

	/** Current scoring configuration */
	UPROPERTY()
	FMGDestructionScoringConfig ScoringConfig;

	/** Counter for generating unique instance IDs */
	UPROPERTY()
	int32 InstanceCounter = 0;

	/** Counter for generating unique event IDs */
	UPROPERTY()
	int32 EventCounter = 0;

	/** Counter for generating unique chain IDs */
	UPROPERTY()
	int32 ChainCounter = 0;

	/** Running total of property damage (dollars) */
	UPROPERTY()
	float TotalPropertyDamage = 0.0f;

	/** Timer handle for the tick function */
	FTimerHandle DestructionTickTimer;
};

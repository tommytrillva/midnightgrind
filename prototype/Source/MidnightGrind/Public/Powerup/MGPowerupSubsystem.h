// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file MGPowerupSubsystem.h
 * @brief Power-up and Combat Item Subsystem for Midnight Grind
 *
 * This subsystem manages all power-up items, offensive/defensive mechanics,
 * projectiles, and hazards in Midnight Grind's racing combat system.
 *
 * ## Overview
 * The Power-up Subsystem implements Mario Kart-style item combat by:
 * - Spawning collectable power-ups at designated pickup points
 * - Managing player inventories with limited power-up slots
 * - Handling projectile weapons (missiles, EMPs) and their trajectories
 * - Placing and tracking environmental hazards (oil slicks, spike strips)
 * - Providing defensive mechanics (shields, ghost mode, invisibility)
 * - Balancing power-up distribution based on race position (rubber-banding)
 *
 * ## Architecture
 * This is a Game Instance Subsystem that persists across gameplay sessions.
 * It coordinates with:
 * - Vehicle physics (speed boosts, slowdown effects)
 * - Damage/health systems (shield blocking, repair pickups)
 * - Race position tracking (rubber-band balance system)
 * - Visual effects system (projectile trails, hazard indicators)
 * - Audio system (pickup sounds, warning alerts)
 *
 * ## Key Concepts for Entry-Level Developers
 *
 * ### Power-up Types (EMGPowerupType)
 * Categories of items players can collect and use:
 *
 * **Offensive (attack other racers):**
 * - Missile: Homing projectile that targets an opponent
 * - EMPBlast: Disables nearby vehicles temporarily
 * - Shockwave: Area-of-effect damage around the player
 *
 * **Defensive (protect yourself):**
 * - Shield: Blocks incoming attacks
 * - GhostMode: Phase through obstacles and attacks
 * - Invisibility: Become untargetable
 *
 * **Boost (improve your performance):**
 * - SpeedBoost/Nitro/TurboCharge: Increase vehicle speed
 * - Repair: Restore vehicle health
 *
 * **Trap (leave hazards for opponents):**
 * - OilSlick: Causes loss of traction
 * - SpikeStrip: Damages vehicles that drive over it
 *
 * ### Power-up States (EMGPowerupState)
 * Lifecycle of a power-up in a player's inventory:
 * - Inactive: Slot is empty, no power-up held
 * - Ready: Power-up is available to use
 * - Active: Power-up effect is currently running (e.g., shield active)
 * - Cooldown: Waiting before power-up can be used again
 * - Depleted: All charges used, power-up removed from inventory
 *
 * ### Target Types (EMGPowerupTarget)
 * How power-ups select their targets:
 * - Self: Affects only the user (shields, speed boosts)
 * - SingleEnemy: Targets one opponent (homing missile)
 * - AreaOfEffect: Affects all players in a radius (EMP blast)
 * - Forward/Backward: Directional projectiles
 * - Homing: Automatically tracks a target
 *
 * ### Inventory System (FMGPowerupInventory)
 * Each player has a limited inventory:
 * - MaxSlots: Usually 1-2 power-up slots
 * - Players must use or discard power-ups to collect new ones
 * - Some power-ups can stack (multiple charges)
 *
 * ### Rubber-Banding (FMGPowerupBalanceConfig)
 * Keeps races competitive by adjusting power-up distribution:
 * - Players in last place get better/stronger power-ups
 * - Race leaders get weaker power-ups or restricted types
 * - Configurable via PositionRarityBoost and restricted lists
 *
 * ### Projectiles (FMGPowerupProjectile)
 * Launched attacks that travel through the world:
 * - Track position, velocity, and lifetime
 * - Can be homing (track a target) or straight-line
 * - Destroyed on impact or when lifetime expires
 *
 * ### Hazards (FMGDroppedHazard)
 * Placed environmental dangers:
 * - Remain at a location for a set duration
 * - Affect any player (optionally including the dropper)
 * - Have effect radius and effect duration when triggered
 *
 * ## Data Flow
 * 1. Pickup spawns at FMGPickupSpawnPoint in the world
 * 2. Player drives through pickup -> TryCollectPickup()
 * 3. Power-up rolled based on position using RollPowerup()
 * 4. Power-up added to player's FMGPowerupInventory
 * 5. Player activates with UsePowerup()
 * 6. Effect applied, projectile launched, or hazard dropped
 * 7. UpdatePowerups() ticks active effects, projectiles, hazards
 *
 * ## Usage Example
 * @code
 * // Get the power-up subsystem
 * UMGPowerupSubsystem* PowerupSystem = GetGameInstance()->GetSubsystem<UMGPowerupSubsystem>();
 *
 * // Initialize a player's inventory at race start
 * PowerupSystem->InitializePlayerInventory(PlayerId, 2); // 2 slots
 *
 * // When player drives through a pickup
 * if (PowerupSystem->TryCollectPickup(PlayerId, SpawnPointId, RacePosition))
 * {
 *     // Power-up added to inventory, play pickup sound
 * }
 *
 * // When player presses the "use power-up" button
 * if (PowerupSystem->UsePowerup(PlayerId, SlotIndex, TargetId))
 * {
 *     // Power-up activated successfully
 * }
 *
 * // Check if player has an active shield
 * if (PowerupSystem->HasActiveShield(PlayerId))
 * {
 *     // Player is protected from attacks
 * }
 * @endcode
 *
 * ## Event System (Delegates)
 * Subscribe to react to power-up activities:
 * - OnPowerupCollected: Player picked up a power-up
 * - OnPowerupActivated: Player used a power-up
 * - OnPowerupHit: A power-up attack hit a target
 * - OnPowerupBlocked: An attack was blocked by a shield
 * - OnShieldActivated/Depleted: Shield status changes
 * - OnProjectileLaunched: A projectile was fired
 * - OnHazardDropped: A hazard was placed in the world
 *
 * @see EMGPowerupType for all power-up categories
 * @see FMGPowerupDefinition for power-up configuration
 * @see FMGPowerupBalanceConfig for rubber-banding settings
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPowerupSubsystem.generated.h"

// ============================================================================
// POWER-UP TYPE ENUMERATION
// ============================================================================

/**
 * @brief All available power-up items in the game
 *
 * Power-ups are categorized by their function:
 * - **Offensive**: Attack other racers (Missile, EMPBlast, Shockwave)
 * - **Defensive**: Protect yourself (Shield, GhostMode, Invisibility)
 * - **Boost**: Improve performance (SpeedBoost, Nitro, TurboCharge)
 * - **Trap**: Leave hazards for others (OilSlick, SpikeStrip)
 * - **Utility**: Special effects (TimeWarp, Magnet, JammerDevice)
 */
UENUM(BlueprintType)
enum class EMGPowerupType : uint8
{
	None				UMETA(DisplayName = "None"),           ///< No power-up (empty slot)
	SpeedBoost			UMETA(DisplayName = "Speed Boost"),    ///< Temporary speed increase
	Shield				UMETA(DisplayName = "Shield"),         ///< Blocks incoming attacks
	Nitro				UMETA(DisplayName = "Nitro"),          ///< Instant burst of speed
	SlowField			UMETA(DisplayName = "Slow Field"),     ///< Area that slows enemies
	EMPBlast			UMETA(DisplayName = "EMP Blast"),      ///< Disables nearby vehicles
	Missile				UMETA(DisplayName = "Missile"),        ///< Homing projectile attack
	OilSlick			UMETA(DisplayName = "Oil Slick"),      ///< Drop slippery hazard
	SpikeStrip			UMETA(DisplayName = "Spike Strip"),    ///< Drop tire-damaging trap
	Shockwave			UMETA(DisplayName = "Shockwave"),      ///< Area-of-effect blast
	Repair				UMETA(DisplayName = "Repair"),         ///< Restore vehicle health
	Invisibility		UMETA(DisplayName = "Invisibility"),   ///< Cannot be targeted
	TimeWarp			UMETA(DisplayName = "Time Warp"),      ///< Slow time briefly
	GhostMode			UMETA(DisplayName = "Ghost Mode"),     ///< Phase through obstacles
	Magnet				UMETA(DisplayName = "Magnet"),         ///< Attract nearby pickups
	RocketBoost			UMETA(DisplayName = "Rocket Boost"),   ///< Powerful forward thrust
	JammerDevice		UMETA(DisplayName = "Jammer Device"),  ///< Disable enemy power-ups
	TurboCharge			UMETA(DisplayName = "Turbo Charge")    ///< Charged speed boost
};

// ============================================================================
// POWER-UP RARITY ENUMERATION
// ============================================================================

/**
 * @brief Rarity tiers affecting power-up strength and availability
 *
 * Rarity influences:
 * - Effect magnitude (higher rarity = stronger effects)
 * - Drop rates from pickups (rarer = less frequent)
 * - Position-based distribution (trailing players get better items)
 */
UENUM(BlueprintType)
enum class EMGPowerupRarity : uint8
{
	Common				UMETA(DisplayName = "Common"),     ///< Basic items, frequent drops
	Uncommon			UMETA(DisplayName = "Uncommon"),   ///< Slightly better effects
	Rare				UMETA(DisplayName = "Rare"),       ///< Good items, less frequent
	Epic				UMETA(DisplayName = "Epic"),       ///< Powerful effects
	Legendary			UMETA(DisplayName = "Legendary")   ///< Best items, very rare
};

// ============================================================================
// POWER-UP STATE ENUMERATION
// ============================================================================

/**
 * @brief Lifecycle states for power-ups in a player's inventory
 *
 * Power-ups progress through these states from pickup to expiration.
 * UI uses these states to show availability and cooldowns.
 */
UENUM(BlueprintType)
enum class EMGPowerupState : uint8
{
	Inactive			UMETA(DisplayName = "Inactive"),   ///< Slot is empty
	Ready				UMETA(DisplayName = "Ready"),      ///< Can be used now
	Active				UMETA(DisplayName = "Active"),     ///< Effect is currently running
	Cooldown			UMETA(DisplayName = "Cooldown"),   ///< Waiting before can use again
	Depleted			UMETA(DisplayName = "Depleted")    ///< All charges used, will be removed
};

// ============================================================================
// POWER-UP TARGET TYPE ENUMERATION
// ============================================================================

/**
 * @brief How power-ups select and affect targets
 *
 * Determines targeting behavior when a power-up is activated.
 * Some require manual targeting, others auto-select or affect areas.
 */
UENUM(BlueprintType)
enum class EMGPowerupTarget : uint8
{
	Self				UMETA(DisplayName = "Self"),           ///< Affects only the user
	SingleEnemy			UMETA(DisplayName = "Single Enemy"),   ///< Requires target selection
	AllEnemies			UMETA(DisplayName = "All Enemies"),    ///< Hits all opponents
	AreaOfEffect		UMETA(DisplayName = "Area of Effect"), ///< Affects radius around user
	Forward				UMETA(DisplayName = "Forward"),        ///< Fires straight ahead
	Backward			UMETA(DisplayName = "Backward"),       ///< Drops/fires behind
	Homing				UMETA(DisplayName = "Homing"),         ///< Auto-tracks nearest target
	Global				UMETA(DisplayName = "Global")          ///< Affects entire race
};

// ============================================================================
// PICKUP SPAWN TYPE ENUMERATION
// ============================================================================

/**
 * @brief How pickup spawn points determine what power-up appears
 *
 * Different spawn types allow for varied gameplay experiences
 * and strategic pickup placement.
 */
UENUM(BlueprintType)
enum class EMGPickupSpawnType : uint8
{
	Fixed				UMETA(DisplayName = "Fixed"),          ///< Always spawns same type
	Random				UMETA(DisplayName = "Random"),         ///< Random from allowed list
	PositionBased		UMETA(DisplayName = "Position Based"), ///< Based on player race position
	TimeBased			UMETA(DisplayName = "Time Based"),     ///< Changes by race time
	EventTriggered		UMETA(DisplayName = "Event Triggered") ///< Spawns on game events
};

// ============================================================================
// POWER-UP DEFINITION STRUCTURE
// ============================================================================

/**
 * @brief Complete definition of a power-up type (data asset)
 *
 * Defines all properties of a power-up for designers to configure.
 * Registered with the subsystem at game start.
 * Think of this as the "blueprint" for a power-up type.
 */
USTRUCT(BlueprintType)
struct FMGPowerupDefinition
{
	GENERATED_BODY()

	/// Unique identifier for this power-up type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PowerupId;

	/// Localized name shown in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Localized description of what the power-up does
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Category of power-up
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType Type = EMGPowerupType::None;

	/// Rarity tier affecting drop rates
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupRarity Rarity = EMGPowerupRarity::Common;

	/// How the power-up selects targets
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupTarget TargetType = EMGPowerupTarget::Self;

	/// How long the effect lasts in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.0f;

	/// Time before power-up can be used again (for multi-charge)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 0.0f;

	/// Number of uses before depleted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCharges = 1;

	/// Strength of the effect (speed boost %, damage, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectMagnitude = 1.0f;

	/// Maximum targeting/projectile range in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 0.0f;

	/// Effect radius for area-of-effect power-ups
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 0.0f;

	/// Whether effect can stack with itself
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanStack = false;

	/// Maximum stack count if stackable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStacks = 1;

	/// If true, player must select a target before using
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresTarget = false;

	/// If true, shields can block this power-up
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeBlocked = true;

	/// UI icon texture reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> IconAsset;

	/// Visual effect to spawn when used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> VFXAsset;

	/// Sound effect to play when used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> SFXAsset;

	/// Color for UI elements and effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PowerupColor = FLinearColor::White;
};

// ============================================================================
// ACTIVE POWER-UP STRUCTURE
// ============================================================================

/**
 * @brief Runtime state of a power-up in a player's inventory or as active effect
 *
 * Created when a player collects a power-up. Tracks current state,
 * remaining charges, active effect timers, and cooldowns.
 * The subsystem ticks these each frame to update timers.
 */
USTRUCT(BlueprintType)
struct FMGActivePowerup
{
	GENERATED_BODY()

	/// Unique instance ID for this power-up instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceId;

	/// Reference to the power-up definition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PowerupId;

	/// Type of power-up (cached from definition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType Type = EMGPowerupType::None;

	/// Current lifecycle state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupState State = EMGPowerupState::Inactive;

	/// Remaining uses of this power-up
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCharges = 0;

	/// Current stack level if stackable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStacks = 0;

	/// Seconds remaining for active effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	/// Seconds until power-up can be used again
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	/// Player who used/owns this power-up
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourcePlayerId;

	/// Player being affected (for offensive power-ups)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerId;

	/// Modifier applied to effect magnitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectMultiplier = 1.0f;

	/// When the power-up was activated (for analytics)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ActivationTime;
};

// ============================================================================
// PICKUP SPAWN POINT STRUCTURE
// ============================================================================

/**
 * @brief World location where power-up pickups appear
 *
 * Designers place these in the level to define where power-ups spawn.
 * Can be configured for specific power-ups or position-based selection.
 * Handles respawning after collection.
 */
USTRUCT(BlueprintType)
struct FMGPickupSpawnPoint
{
	GENERATED_BODY()

	/// Unique identifier for this spawn point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpawnPointId;

	/// World position of the pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Rotation of the pickup visual
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/// How this point determines what power-up spawns
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPickupSpawnType SpawnType = EMGPickupSpawnType::Fixed;

	/// Power-up types that can spawn here (empty = all allowed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPowerupType> AllowedPowerups;

	/// Probability weights for each rarity tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupRarity, float> RarityWeights;

	/// Seconds until respawn after collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 30.0f;

	/// How close player must drive to collect (collision radius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerRadius = 500.0f;

	/// Whether the pickup is currently available
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	/// Countdown until next respawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilRespawn = 0.0f;

	/// Currently spawned power-up type (None if collected)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType CurrentPowerup = EMGPowerupType::None;
};

// ============================================================================
// POWER-UP SLOT STRUCTURE
// ============================================================================

/**
 * @brief A single slot in a player's power-up inventory
 *
 * Players have limited slots (typically 1-2) to hold power-ups.
 * Each slot tracks its held power-up and any slot-specific cooldowns.
 */
USTRUCT(BlueprintType)
struct FMGPowerupSlot
{
	GENERATED_BODY()

	/// Index of this slot (0, 1, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;

	/// Power-up currently in this slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGActivePowerup Powerup;

	/// If true, slot cannot receive new power-ups (progression unlock)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocked = false;

	/// Cooldown before this slot can receive new power-ups
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlotCooldown = 0.0f;
};

// ============================================================================
// POWER-UP INVENTORY STRUCTURE
// ============================================================================

/**
 * @brief Complete power-up inventory for a player
 *
 * Tracks all held power-ups (in slots), active effects, and
 * session statistics. Created per-player at race start.
 */
USTRUCT(BlueprintType)
struct FMGPowerupInventory
{
	GENERATED_BODY()

	/// Player this inventory belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/// Array of power-up slots
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPowerupSlot> Slots;

	/// Maximum number of slots (can increase via upgrades)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSlots = 2;

	/// Currently running power-up effects (speed boost, shield, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGActivePowerup> ActiveEffects;

	/// Quick flag for checking shield status
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasShield = false;

	/// Seconds remaining on active shield
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShieldTimeRemaining = 0.0f;

	/// Total pickups collected this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPowerupsCollected = 0;

	/// Total power-ups used this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPowerupsUsed = 0;
};

// ============================================================================
// PROJECTILE STRUCTURE
// ============================================================================

/**
 * @brief Active projectile launched from a power-up
 *
 * Tracks missiles and other launched attacks as they travel through
 * the world. The subsystem updates positions and checks for collisions
 * each frame via UpdateProjectiles().
 */
USTRUCT(BlueprintType)
struct FMGPowerupProjectile
{
	GENERATED_BODY()

	/// Unique identifier for this projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProjectileId;

	/// Type of power-up that created this projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType PowerupType = EMGPowerupType::None;

	/// Player who fired the projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourcePlayerId;

	/// Target player (for homing projectiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerId;

	/// Current world position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Current movement direction and speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	/// Base speed of the projectile (m/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	/// Seconds until projectile despawns if no hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifetimeRemaining = 0.0f;

	/// Damage dealt on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0.0f;

	/// Whether this projectile tracks a target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHoming = false;

	/// How aggressively the projectile turns toward target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HomingStrength = 0.0f;
};

// ============================================================================
// DROPPED HAZARD STRUCTURE
// ============================================================================

/**
 * @brief Environmental hazard placed by a power-up
 *
 * Tracks oil slicks, spike strips, and other placed obstacles.
 * Hazards persist at a location until their lifetime expires.
 * Players driving through trigger the hazard effect.
 */
USTRUCT(BlueprintType)
struct FMGDroppedHazard
{
	GENERATED_BODY()

	/// Unique identifier for this hazard
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HazardId;

	/// Type of power-up that created this hazard
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType SourcePowerup = EMGPowerupType::None;

	/// Player who dropped the hazard
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourcePlayerId;

	/// World position of the hazard center
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Orientation of the hazard (for directional types)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/// Trigger radius - players within this distance are affected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 200.0f;

	/// Seconds until hazard despawns
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifetimeRemaining = 0.0f;

	/// How long the effect lasts on affected players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectDuration = 0.0f;

	/// Strength of the effect (spin amount, slow %, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectMagnitude = 0.0f;

	/// If true, the dropper can trigger their own hazard
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsOwner = false;
};

// ============================================================================
// POWER-UP STATS STRUCTURE
// ============================================================================

/**
 * @brief Lifetime statistics for a player's power-up usage
 *
 * Tracks detailed stats for achievements, leaderboards, and analytics.
 * Persisted to save data for career statistics.
 */
USTRUCT(BlueprintType)
struct FMGPowerupStats
{
	GENERATED_BODY()

	/// Player these stats belong to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/// Count of each power-up type collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> PowerupsCollected;

	/// Count of each power-up type used/activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> PowerupsUsed;

	/// Successful hits dealt by power-up type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> HitsDealt;

	/// Hits received from each power-up type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> HitsReceived;

	/// Attacks blocked by shields, by attack type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> HitsBlocked;

	/// Total projectile power-ups launched
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalProjectilesLaunched = 0;

	/// Total projectiles that hit a target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalProjectilesHit = 0;

	/// Hit rate percentage (TotalHit / TotalLaunched)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileAccuracy = 0.0f;
};

// ============================================================================
// BALANCE CONFIG STRUCTURE
// ============================================================================

/**
 * @brief Configuration for power-up balancing and rubber-banding
 *
 * Controls the "catch-up" mechanics that keep races competitive.
 * Trailing players get better power-ups, leaders get weaker ones.
 * Adjust these values to tune difficulty and competitiveness.
 */
USTRUCT(BlueprintType)
struct FMGPowerupBalanceConfig
{
	GENERATED_BODY()

	/// Master toggle for position-based balancing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableRubberBanding = true;

	/// Effect multiplier for 1st place (< 1.0 = weaker effects)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeaderPowerupNerf = 0.8f;

	/// Effect multiplier for last place (> 1.0 = stronger effects)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LastPlacePowerupBuff = 1.5f;

	/// Rarity boost by race position (position -> bonus %)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, float> PositionRarityBoost;

	/// Power-ups the leader cannot receive
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPowerupType> LeaderRestrictedPowerups;

	/// Power-ups guaranteed for last place
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPowerupType> LastPlaceGuaranteedPowerups;

	/// Global multiplier for all cooldowns
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalCooldownMultiplier = 1.0f;

	/// Global multiplier for all effect durations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalDurationMultiplier = 1.0f;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/**
 * Delegates allow other systems to react to power-up events.
 * Bind to these in Blueprints or C++ to update UI, play sounds, etc.
 */

/// Fired when a player picks up a power-up
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPowerupCollected, const FString&, PlayerId, EMGPowerupType, PowerupType, int32, SlotIndex);

/// Fired when a player uses/activates a power-up
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPowerupActivated, const FString&, PlayerId, const FMGActivePowerup&, Powerup, const FString&, TargetId);

/// Fired when a power-up effect timer expires
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPowerupExpired, const FString&, PlayerId, EMGPowerupType, PowerupType);

/// Fired when a power-up attack hits a target
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPowerupHit, const FString&, SourceId, const FString&, TargetId, EMGPowerupType, PowerupType);

/// Fired when a shield blocks an incoming attack
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPowerupBlocked, const FString&, TargetId, EMGPowerupType, PowerupType);

/// Fired when a shield becomes active
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldActivated, const FString&, PlayerId, float, Duration);

/// Fired when a shield is destroyed or expires
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldDepleted, const FString&, PlayerId);

/// Fired when a pickup respawns at a spawn point
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPickupSpawned, const FString&, SpawnPointId, EMGPowerupType, PowerupType);

/// Fired when a projectile is launched
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileLaunched, const FString&, ProjectileId, const FMGPowerupProjectile&, Projectile);

/// Fired when a hazard is dropped in the world
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardDropped, const FMGDroppedHazard&, Hazard);

// ============================================================================
// POWER-UP SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem managing all power-up and combat mechanics
 *
 * Game Instance Subsystem that persists across level transitions.
 * Manages power-up definitions, pickups, inventories, projectiles,
 * hazards, and balance configuration.
 *
 * ## Responsibilities
 * - Register and store power-up definitions from data assets
 * - Manage pickup spawn points and respawn timing
 * - Track player inventories (slots, held power-ups)
 * - Handle projectile physics and collision detection
 * - Manage placed hazards in the world
 * - Apply rubber-banding based on race position
 * - Shield mechanics and attack blocking
 * - Persist player power-up statistics
 *
 * ## Update Flow
 * Call UpdatePowerups() each tick to:
 * 1. Tick active effect timers
 * 2. Update projectile positions and check hits
 * 3. Tick hazard lifetimes
 * 4. Check spawn point respawn timers
 *
 * ## Combat Flow
 * 1. Player collects pickup -> TryCollectPickup()
 * 2. Power-up rolled based on position -> RollPowerup()
 * 3. Added to inventory slot
 * 4. Player uses power-up -> UsePowerup()
 * 5. Effect applied, projectile launched, or hazard dropped
 * 6. Impact detected -> OnPowerupHit or blocked by shield
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPowerupSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupCollected OnPowerupCollected;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupActivated OnPowerupActivated;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupExpired OnPowerupExpired;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupHit OnPowerupHit;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupBlocked OnPowerupBlocked;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnShieldActivated OnShieldActivated;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnShieldDepleted OnShieldDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPickupSpawned OnPickupSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnProjectileLaunched OnProjectileLaunched;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnHazardDropped OnHazardDropped;

	// Power-up Definitions
	UFUNCTION(BlueprintCallable, Category = "Powerup|Definition")
	void RegisterPowerupDefinition(const FMGPowerupDefinition& Definition);

	UFUNCTION(BlueprintPure, Category = "Powerup|Definition")
	FMGPowerupDefinition GetPowerupDefinition(EMGPowerupType Type) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Definition")
	TArray<FMGPowerupDefinition> GetAllPowerupDefinitions() const;

	// Spawn Point Management
	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void RegisterSpawnPoint(const FMGPickupSpawnPoint& SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void UnregisterSpawnPoint(const FString& SpawnPointId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void ActivateAllSpawnPoints();

	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void DeactivateAllSpawnPoints();

	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void RespawnPickup(const FString& SpawnPointId);

	UFUNCTION(BlueprintPure, Category = "Powerup|SpawnPoints")
	TArray<FMGPickupSpawnPoint> GetActiveSpawnPoints() const;

	// Collection
	UFUNCTION(BlueprintCallable, Category = "Powerup|Collection")
	bool TryCollectPickup(const FString& PlayerId, const FString& SpawnPointId, int32 RacePosition);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Collection")
	bool GrantPowerup(const FString& PlayerId, EMGPowerupType Type, int32 SlotIndex = -1);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Collection")
	EMGPowerupType RollPowerup(int32 RacePosition, int32 TotalRacers, const TArray<EMGPowerupType>& AllowedTypes);

	// Inventory Management
	UFUNCTION(BlueprintCallable, Category = "Powerup|Inventory")
	void InitializePlayerInventory(const FString& PlayerId, int32 MaxSlots = 2);

	UFUNCTION(BlueprintPure, Category = "Powerup|Inventory")
	FMGPowerupInventory GetPlayerInventory(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Inventory")
	bool HasPowerup(const FString& PlayerId, EMGPowerupType Type) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Inventory")
	FMGPowerupSlot GetSlot(const FString& PlayerId, int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Inventory")
	int32 GetEmptySlot(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Inventory")
	void SwapSlots(const FString& PlayerId, int32 SlotA, int32 SlotB);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Inventory")
	void DiscardSlot(const FString& PlayerId, int32 SlotIndex);

	// Activation
	UFUNCTION(BlueprintCallable, Category = "Powerup|Activation")
	bool UsePowerup(const FString& PlayerId, int32 SlotIndex, const FString& TargetId = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Powerup|Activation")
	bool ActivatePowerupEffect(const FString& PlayerId, EMGPowerupType Type, const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Activation")
	void DeactivatePowerup(const FString& PlayerId, const FString& InstanceId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Activation")
	void DeactivateAllPowerups(const FString& PlayerId);

	// Shield
	UFUNCTION(BlueprintCallable, Category = "Powerup|Shield")
	void ActivateShield(const FString& PlayerId, float Duration);

	UFUNCTION(BlueprintPure, Category = "Powerup|Shield")
	bool HasActiveShield(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Shield")
	float GetShieldTimeRemaining(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Shield")
	bool TryBlockAttack(const FString& TargetId, EMGPowerupType AttackType);

	// Projectiles
	UFUNCTION(BlueprintCallable, Category = "Powerup|Projectiles")
	FString LaunchProjectile(const FMGPowerupProjectile& Projectile);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Projectiles")
	void UpdateProjectiles(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Powerup|Projectiles")
	TArray<FMGPowerupProjectile> GetActiveProjectiles() const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Projectiles")
	void DestroyProjectile(const FString& ProjectileId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Projectiles")
	bool CheckProjectileHit(const FString& ProjectileId, const FString& TargetId, FVector TargetLocation);

	// Hazards
	UFUNCTION(BlueprintCallable, Category = "Powerup|Hazards")
	FString DropHazard(const FMGDroppedHazard& Hazard);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Hazards")
	void UpdateHazards(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Powerup|Hazards")
	TArray<FMGDroppedHazard> GetActiveHazards() const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Hazards")
	void RemoveHazard(const FString& HazardId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Hazards")
	bool CheckHazardCollision(const FString& PlayerId, FVector PlayerLocation);

	// Effects Query
	UFUNCTION(BlueprintPure, Category = "Powerup|Effects")
	bool HasActiveEffect(const FString& PlayerId, EMGPowerupType EffectType) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Effects")
	float GetEffectTimeRemaining(const FString& PlayerId, EMGPowerupType EffectType) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Effects")
	float GetEffectMultiplier(const FString& PlayerId, EMGPowerupType EffectType) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Effects")
	TArray<FMGActivePowerup> GetAllActiveEffects(const FString& PlayerId) const;

	// Balance
	UFUNCTION(BlueprintCallable, Category = "Powerup|Balance")
	void SetBalanceConfig(const FMGPowerupBalanceConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Powerup|Balance")
	FMGPowerupBalanceConfig GetBalanceConfig() const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Balance")
	float GetPositionMultiplier(int32 Position, int32 TotalRacers) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Powerup|Stats")
	FMGPowerupStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Update
	UFUNCTION(BlueprintCallable, Category = "Powerup|Update")
	void UpdatePowerups(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Update")
	void UpdateSpawnPoints(float DeltaTime);

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Powerup|Persistence")
	void SavePowerupData();

	UFUNCTION(BlueprintCallable, Category = "Powerup|Persistence")
	void LoadPowerupData();

protected:
	void TickPowerups(float DeltaTime);
	void ProcessPowerupExpiration(const FString& PlayerId, FMGActivePowerup& Powerup);
	void ApplyPowerupEffect(const FString& PlayerId, const FMGPowerupDefinition& Definition);
	void RemovePowerupEffect(const FString& PlayerId, EMGPowerupType Type);
	FString GenerateInstanceId() const;

private:
	UPROPERTY()
	TMap<EMGPowerupType, FMGPowerupDefinition> PowerupDefinitions;

	UPROPERTY()
	TMap<FString, FMGPickupSpawnPoint> SpawnPoints;

	UPROPERTY()
	TMap<FString, FMGPowerupInventory> PlayerInventories;

	UPROPERTY()
	TMap<FString, FMGPowerupStats> PlayerStats;

	UPROPERTY()
	TArray<FMGPowerupProjectile> ActiveProjectiles;

	UPROPERTY()
	TArray<FMGDroppedHazard> ActiveHazards;

	UPROPERTY()
	FMGPowerupBalanceConfig BalanceConfig;

	UPROPERTY()
	int32 InstanceCounter = 0;

	FTimerHandle PowerupTickTimer;
};

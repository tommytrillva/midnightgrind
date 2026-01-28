// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file MGBonusSubsystem.h
 * @brief Bonus and Reward System Subsystem for Midnight Grind
 *
 * This subsystem manages all bonus pickups, score multipliers, bonus rounds,
 * and reward mechanics in Midnight Grind's racing gameplay.
 *
 * ## Overview
 * The Bonus Subsystem creates an engaging reward loop by:
 * - Managing collectable bonus pickups scattered throughout the world
 * - Tracking active bonuses and their durations (speed boost, invincibility, etc.)
 * - Running timed bonus rounds (point rush, coin collect, drift challenges)
 * - Awarding combo bonuses for skilled play
 * - Handling secret bonus discovery mechanics
 *
 * ## Architecture
 * This is a Game Instance Subsystem, meaning it persists across all game sessions
 * and level transitions. It integrates with:
 * - Player scoring systems (applies multipliers and bonus points)
 * - HUD/UI systems (displays active bonuses, timers, notifications)
 * - Save/Load systems (persists player bonus statistics)
 *
 * ## Key Concepts for Entry-Level Developers
 *
 * ### Bonus Types (EMGBonusType)
 * Different categories of bonuses the player can collect:
 * - **ScoreMultiplier**: Temporarily multiplies all points earned (e.g., 2x, 3x)
 * - **PointBonus**: Instant point award when collected
 * - **SpeedBoost/NitroRefill**: Affects vehicle performance
 * - **Invincibility/GhostMode**: Defensive bonuses preventing damage
 * - **ComboExtender**: Keeps combo chains alive longer
 *
 * ### Bonus Rarity (EMGBonusRarity)
 * Rarity affects spawn chance and bonus magnitude:
 * - Common (frequent, small bonuses) -> Mythic (very rare, powerful bonuses)
 *
 * ### Bonus Triggers (EMGBonusTrigger)
 * How bonuses are activated:
 * - **Pickup**: Player drives through a collectible in the world
 * - **Achievement**: Unlocked by completing specific goals
 * - **Combo**: Triggered by reaching combo thresholds
 * - **SecretArea**: Found by exploring hidden locations
 *
 * ### Bonus Rounds (FMGBonusRound)
 * Timed mini-games with special objectives:
 * - TimeAttack: Complete checkpoints as fast as possible
 * - PointRush: Score as many points as possible before time runs out
 * - DriftChallenge: Accumulate drift points
 * Each round has Bronze/Silver/Gold score thresholds for rewards.
 *
 * ### Spawn Points (FMGBonusSpawnPoint)
 * Locations in the world where bonuses appear:
 * - Can be assigned specific bonus types or random selection
 * - Respawn after collection with configurable timers
 * - Support floating/hovering visual effects
 *
 * ## Data Flow
 * 1. Designer places FMGBonusSpawnPoint actors in the level
 * 2. Subsystem spawns bonuses at these points based on configuration
 * 3. Player collects bonus -> CollectBonus() called
 * 4. Active bonus created (FMGActiveBonus) with duration timer
 * 5. UpdateBonusSystem() ticks active bonuses, expires when timer reaches 0
 * 6. Player stats updated for progression tracking
 *
 * ## Usage Example
 * @code
 * // Get the bonus subsystem from anywhere with access to GameInstance
 * UMGBonusSubsystem* BonusSystem = GetGameInstance()->GetSubsystem<UMGBonusSubsystem>();
 *
 * // Check if player has any active multiplier bonuses
 * if (BonusSystem->HasActiveBonus(PlayerId, EMGBonusType::ScoreMultiplier))
 * {
 *     float Multiplier = BonusSystem->GetScoreMultiplier(PlayerId);
 *     // Apply multiplier to score...
 * }
 *
 * // Grant a bonus directly (e.g., from an achievement)
 * BonusSystem->GrantBonus(PlayerId, "SpeedBoost_Tier2");
 *
 * // Start a bonus round
 * BonusSystem->StartBonusRound(PlayerId, "PointRush_Downtown");
 * @endcode
 *
 * ## Event System (Delegates)
 * Subscribe to these events to react to bonus activities:
 * - OnBonusCollected: Player picked up a bonus
 * - OnBonusActivated: A timed bonus effect started
 * - OnBonusExpired: A timed bonus effect ended
 * - OnMultiplierChanged: Score multiplier value changed
 * - OnBonusRoundStart/Complete/Failed: Bonus round lifecycle events
 * - OnSecretBonusFound: Player discovered a hidden bonus
 *
 * @see EMGBonusType for all bonus categories
 * @see FMGBonusDefinition for bonus configuration data
 * @see FMGActiveBonus for runtime bonus state
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBonusSubsystem.generated.h"

// ============================================================================
// BONUS TYPE ENUMERATION
// ============================================================================

/**
 * @brief All available bonus types in the game
 *
 * Bonuses are categorized by their effect on gameplay:
 * - Score bonuses: Affect points earned (multipliers, point bonuses)
 * - Performance bonuses: Affect vehicle (speed boost, nitro refill)
 * - Defensive bonuses: Protect the player (invincibility, ghost mode)
 * - Progression bonuses: Affect XP, cash, reputation gains
 */
UENUM(BlueprintType)
enum class EMGBonusType : uint8
{
	None					UMETA(DisplayName = "None"),           ///< No bonus (default/invalid state)
	ScoreMultiplier			UMETA(DisplayName = "Score Multiplier"), ///< Multiplies all points earned
	PointBonus				UMETA(DisplayName = "Point Bonus"),    ///< Instant point award
	TimeBonus				UMETA(DisplayName = "Time Bonus"),     ///< Adds time in timed modes
	NitroRefill				UMETA(DisplayName = "Nitro Refill"),   ///< Refills nitrous tank
	SpeedBoost				UMETA(DisplayName = "Speed Boost"),    ///< Temporary speed increase
	Invincibility			UMETA(DisplayName = "Invincibility"),  ///< Cannot take damage
	GhostMode				UMETA(DisplayName = "Ghost Mode"),     ///< Phase through obstacles
	DoublePoints			UMETA(DisplayName = "Double Points"),  ///< 2x point multiplier
	TriplePoints			UMETA(DisplayName = "Triple Points"),  ///< 3x point multiplier
	ComboExtender			UMETA(DisplayName = "Combo Extender"), ///< Extends combo timer
	InstantRepair			UMETA(DisplayName = "Instant Repair"), ///< Fully repairs vehicle
	CashBonus				UMETA(DisplayName = "Cash Bonus"),     ///< Awards in-game currency
	XPBonus					UMETA(DisplayName = "XP Bonus"),       ///< Awards experience points
	RepBonus				UMETA(DisplayName = "Reputation Bonus"), ///< Awards reputation points
	LapBonus				UMETA(DisplayName = "Lap Bonus"),      ///< Bonus for completing a lap
	SecretBonus				UMETA(DisplayName = "Secret Bonus"),   ///< Hidden bonus with special reward
	ChainStarter			UMETA(DisplayName = "Chain Starter")   ///< Initiates a bonus chain
};

// ============================================================================
// BONUS RARITY ENUMERATION
// ============================================================================

/**
 * @brief Rarity tiers for bonuses, affecting spawn rates and value
 *
 * Higher rarity bonuses are more powerful but spawn less frequently.
 * Rarity affects:
 * - Spawn probability at pickup points
 * - Visual effects (glow intensity, particle effects)
 * - Base value/multiplier of the bonus effect
 */
UENUM(BlueprintType)
enum class EMGBonusRarity : uint8
{
	Common					UMETA(DisplayName = "Common"),     ///< Frequent spawns, basic effects
	Uncommon				UMETA(DisplayName = "Uncommon"),   ///< Less frequent, slightly better
	Rare					UMETA(DisplayName = "Rare"),       ///< Uncommon, noticeable improvement
	Epic					UMETA(DisplayName = "Epic"),       ///< Rare, significant bonus
	Legendary				UMETA(DisplayName = "Legendary"),  ///< Very rare, powerful effects
	Mythic					UMETA(DisplayName = "Mythic")      ///< Extremely rare, game-changing
};

// ============================================================================
// BONUS TRIGGER ENUMERATION
// ============================================================================

/**
 * @brief How a bonus is activated/obtained by the player
 *
 * Different triggers allow bonuses to reward various play styles:
 * - Exploration (pickups, secret areas)
 * - Skill (combos, challenges)
 * - Progression (achievements, milestones)
 */
UENUM(BlueprintType)
enum class EMGBonusTrigger : uint8
{
	Pickup					UMETA(DisplayName = "Pickup"),           ///< Collected from world object
	Achievement				UMETA(DisplayName = "Achievement"),      ///< Awarded for completing achievement
	Milestone				UMETA(DisplayName = "Milestone"),        ///< Reached progression milestone
	Combo					UMETA(DisplayName = "Combo"),            ///< Triggered by combo threshold
	TimeBased				UMETA(DisplayName = "Time Based"),       ///< Appears at specific times
	PositionBased			UMETA(DisplayName = "Position Based"),   ///< Based on race position
	EventBased				UMETA(DisplayName = "Event Based"),      ///< Triggered by game events
	RandomDrop				UMETA(DisplayName = "Random Drop"),      ///< Random chance to appear
	SecretArea				UMETA(DisplayName = "Secret Area"),      ///< Found in hidden location
	Challenge				UMETA(DisplayName = "Challenge Complete") ///< Awarded for challenge completion
};

// ============================================================================
// BONUS ROUND TYPE ENUMERATION
// ============================================================================

/**
 * @brief Types of timed bonus round mini-games
 *
 * Bonus rounds are special timed challenges that appear during gameplay.
 * Each type has unique objectives and scoring mechanics.
 * Players earn rewards based on Bronze/Silver/Gold score thresholds.
 */
UENUM(BlueprintType)
enum class EMGBonusRoundType : uint8
{
	None					UMETA(DisplayName = "None"),           ///< No active bonus round
	TimeAttack				UMETA(DisplayName = "Time Attack"),    ///< Reach checkpoints quickly
	PointRush				UMETA(DisplayName = "Point Rush"),     ///< Score maximum points
	CoinCollect				UMETA(DisplayName = "Coin Collect"),   ///< Collect all coins in time
	TargetDestroy			UMETA(DisplayName = "Target Destroy"), ///< Destroy target objects
	Survival				UMETA(DisplayName = "Survival"),       ///< Survive without crashing
	DriftChallenge			UMETA(DisplayName = "Drift Challenge"), ///< Accumulate drift score
	SpeedTrap				UMETA(DisplayName = "Speed Trap"),     ///< Hit max speed through zones
	NearMissRun				UMETA(DisplayName = "Near Miss Run"),  ///< Score near-miss points
	TakedownFrenzy			UMETA(DisplayName = "Takedown Frenzy") ///< Take down opponents
};

// ============================================================================
// BONUS DEFINITION STRUCTURE
// ============================================================================

/**
 * @brief Complete definition of a bonus type (data asset)
 *
 * This struct defines all properties of a bonus, used by designers to
 * configure bonus behavior. Registered with the subsystem at game start.
 * Think of this as the "template" for a bonus type.
 */
USTRUCT(BlueprintType)
struct FMGBonusDefinition
{
	GENERATED_BODY()

	/// Unique identifier for this bonus type (e.g., "SpeedBoost_Tier1")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BonusId;

	/// Localized name shown to players in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Localized description explaining what the bonus does
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Category of bonus effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusType Type = EMGBonusType::PointBonus;

	/// Rarity tier affecting spawn probability
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusRarity Rarity = EMGBonusRarity::Common;

	/// How this bonus is obtained
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusTrigger Trigger = EMGBonusTrigger::Pickup;

	/// Base numeric value (meaning depends on type - speed amount, points, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 100.0f;

	/// How long the effect lasts in seconds (0 = instant effect)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	/// Multiplier applied to scores/effects while active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	/// Points awarded when this bonus is collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointValue = 0;

	/// Whether multiple of this bonus can be active simultaneously
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStackable = false;

	/// Maximum stack count if stackable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStacks = 1;

	/// Probability (0-1) that this bonus spawns when rolled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnChance = 1.0f;

	/// Seconds until the bonus respawns after collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 30.0f;

	/// Reference to the visual mesh/particle for the pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> VisualAsset;

	/// Sound to play when collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> PickupSound;

	/// Color used for UI elements and visual effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DisplayColor = FLinearColor::Yellow;
};

// ============================================================================
// ACTIVE BONUS STRUCTURE
// ============================================================================

/**
 * @brief Runtime state of a bonus currently affecting a player
 *
 * Created when a player collects a timed bonus. Contains the current
 * state including remaining duration and stack count. The subsystem
 * ticks these each frame to update timers and expire when complete.
 */
USTRUCT(BlueprintType)
struct FMGActiveBonus
{
	GENERATED_BODY()

	/// Unique instance ID for this active bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActiveId;

	/// Reference to the bonus definition (FMGBonusDefinition.BonusId)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BonusId;

	/// Player this bonus is affecting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/// Type of bonus effect (cached from definition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusType Type = EMGBonusType::None;

	/// Current effect value (may be modified from base)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;

	/// Current multiplier value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	/// Seconds remaining until bonus expires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	/// Original total duration (for progress bars)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDuration = 0.0f;

	/// Current stack count if bonus is stackable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStacks = 1;

	/// If true, timer is frozen (e.g., during pause menu)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPaused = false;

	/// Timestamp when bonus was activated (for analytics)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ActivatedAt;
};

// ============================================================================
// BONUS SPAWN POINT STRUCTURE
// ============================================================================

/**
 * @brief World location where bonus pickups can appear
 *
 * Designers place these in the level to define where bonuses spawn.
 * Can be configured for specific bonus types or random selection
 * from a weighted pool. Handles respawning after collection.
 */
USTRUCT(BlueprintType)
struct FMGBonusSpawnPoint
{
	GENERATED_BODY()

	/// Unique identifier for this spawn point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpawnId;

	/// World position of the spawn point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Rotation of the spawned bonus visual
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/// Currently spawned bonus type (empty if not spawned)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AssignedBonusId;

	/// Pool of bonus IDs that can spawn here (random selection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PossibleBonusIds;

	/// Whether a bonus is currently spawned and collectible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	/// Whether the bonus was just collected (waiting to respawn)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollected = false;

	/// Countdown until next spawn (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTimer = 0.0f;

	/// How close player must be to collect (collision radius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollectionRadius = 200.0f;

	/// Whether the pickup hovers above the ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFloating = true;

	/// Height above ground when floating (centimeters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloatHeight = 100.0f;
};

// ============================================================================
// BONUS ROUND STRUCTURE
// ============================================================================

/**
 * @brief Definition for a bonus round mini-game
 *
 * Bonus rounds are timed challenges that players can trigger.
 * Each round has specific objectives, duration, and tiered rewards
 * based on performance (Bronze/Silver/Gold thresholds).
 */
USTRUCT(BlueprintType)
struct FMGBonusRound
{
	GENERATED_BODY()

	/// Unique identifier for this bonus round
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoundId;

	/// Display name shown when round starts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RoundName;

	/// Brief description of the objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Type of challenge/objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusRoundType Type = EMGBonusRoundType::TimeAttack;

	/// Time limit for the round in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 30.0f;

	/// Target score to aim for (informational)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetScore = 10000;

	/// Minimum score for Bronze medal
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzeThreshold = 5000;

	/// Minimum score for Silver medal
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverThreshold = 10000;

	/// Minimum score for Gold medal
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldThreshold = 20000;

	/// Multiplier applied to all points during the round
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointMultiplier = 2.0f;

	/// Bonus points for completing the round
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletionBonus = 5000;

	/// IDs of spawn points used during this round
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SpawnPointIds;
};

// ============================================================================
// ACTIVE BONUS ROUND STRUCTURE
// ============================================================================

/**
 * @brief Runtime state of a bonus round currently in progress
 *
 * Created when a player starts a bonus round. Tracks score,
 * progress, and time remaining. Used by UI to display
 * round status and by subsystem to determine completion.
 */
USTRUCT(BlueprintType)
struct FMGActiveBonusRound
{
	GENERATED_BODY()

	/// Reference to the round definition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoundId;

	/// Player participating in the round
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/// Type of round (cached for quick access)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusRoundType Type = EMGBonusRoundType::None;

	/// Whether the round is currently running
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	/// Seconds remaining before round ends
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	/// Points accumulated during this round
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentScore = 0;

	/// Target score for reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetScore = 0;

	/// Number of items collected (for CoinCollect type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemsCollected = 0;

	/// Total items available to collect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalItems = 0;

	/// Best completion time (for TimeAttack type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTime = 0.0f;

	/// True if player achieved minimum threshold
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	/// True if player failed the objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFailed = false;
};

// ============================================================================
// COMBO BONUS EVENT STRUCTURE
// ============================================================================

/**
 * @brief Record of a combo-triggered bonus event
 *
 * When a player reaches combo thresholds, bonus points are awarded.
 * This struct logs those events for analytics and replay features.
 */
USTRUCT(BlueprintType)
struct FMGComboBonusEvent
{
	GENERATED_BODY()

	/// Unique event instance ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	/// Player who triggered the combo bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/// Bonus type that was triggered (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BonusId;

	/// Combo count that triggered this bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboCount = 0;

	/// Points awarded for this combo bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsAwarded = 0;

	/// Multiplier applied to the bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	/// When the event occurred (for analytics)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

// ============================================================================
// PLAYER BONUS STATS STRUCTURE
// ============================================================================

/**
 * @brief Lifetime statistics for a player's bonus collection
 *
 * Tracks all-time stats for achievements, progression, and leaderboards.
 * Persisted to save data and loaded on game start.
 */
USTRUCT(BlueprintType)
struct FMGBonusPlayerStats
{
	GENERATED_BODY()

	/// Player these stats belong to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/// Count of bonuses collected by type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGBonusType, int32> BonusesCollected;

	/// Count of bonuses collected by rarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGBonusRarity, int32> RaritiesCollected;

	/// Total number of bonuses ever collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBonusesCollected = 0;

	/// Lifetime points earned from bonuses
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPointsFromBonuses = 0;

	/// Number of bonus rounds completed successfully
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusRoundsCompleted = 0;

	/// Number of bonus rounds completed with Gold rating
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusRoundsGold = 0;

	/// Number of secret bonuses discovered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SecretBonusesFound = 0;

	/// Longest time a score multiplier was maintained
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestMultiplierDuration = 0.0f;

	/// Highest multiplier value ever achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestMultiplierValue = 1.0f;

	/// Highest combo bonus points in a single combo
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxComboBonus = 0;
};

// ============================================================================
// BONUS CONFIG STRUCTURE
// ============================================================================

/**
 * @brief Global configuration settings for the bonus system
 *
 * Allows tuning of the entire bonus system without code changes.
 * Can be modified at runtime for difficulty modes or special events.
 */
USTRUCT(BlueprintType)
struct FMGBonusConfig
{
	GENERATED_BODY()

	/// Multiplier applied to all bonus values globally
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalMultiplier = 1.0f;

	/// Multiplier for all respawn times (0.5 = respawn twice as fast)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTimeMultiplier = 1.0f;

	/// Multiplier for all bonus durations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationMultiplier = 1.0f;

	/// Combo count required to trigger a combo bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboThresholdForBonus = 10.0f;

	/// Points awarded per combo level above threshold
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboBonusPointsPerLevel = 1000.0f;

	/// Probability weights for each rarity tier when spawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGBonusRarity, float> RaritySpawnWeights;

	/// Whether secret bonuses can be discovered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSecretBonuses = true;

	/// Whether bonus rounds are available
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableBonusRounds = true;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/**
 * Delegates allow other systems to react to bonus events.
 * Bind to these in Blueprints or C++ to update UI, play sounds, etc.
 */

/// Fired when a player collects a bonus pickup
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusCollected, const FString&, PlayerId, const FMGBonusDefinition&, Bonus, int32, PointsAwarded);

/// Fired when a timed bonus effect begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusActivated, const FString&, PlayerId, const FMGActiveBonus&, Bonus, float, Duration);

/// Fired when a timed bonus effect ends naturally
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusExpired, const FString&, PlayerId, const FString&, BonusId, float, TotalValue);

/// Fired when a stackable bonus gains another stack
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusStacked, const FString&, PlayerId, const FString&, BonusId, int32, NewStackCount);

/// Fired when player's score multiplier changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMultiplierChanged, const FString&, PlayerId, float, NewMultiplier);

/// Fired when a bonus round begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBonusRoundStart, const FString&, PlayerId, const FMGBonusRound&, Round);

/// Fired when a bonus round is completed successfully
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusRoundComplete, const FString&, PlayerId, const FString&, RoundId, int32, FinalScore);

/// Fired when a bonus round is failed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBonusRoundFailed, const FString&, PlayerId, const FString&, RoundId);

/// Fired when a combo threshold awards bonus points
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnComboBonusTriggered, const FString&, PlayerId, int32, ComboLevel, int32, BonusPoints);

/// Fired when a secret bonus is discovered
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSecretBonusFound, const FString&, PlayerId, const FString&, SecretId);

/// Fired when a bonus spawns at a spawn point
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBonusSpawned, const FString&, SpawnId, const FString&, BonusId);

/// Fired when a collected bonus respawns
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBonusRespawned, const FString&, SpawnId, float, NextRespawnTime);

// ============================================================================
// BONUS SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem managing all bonus and reward mechanics
 *
 * Game Instance Subsystem that persists across level transitions.
 * Manages bonus definitions, spawn points, active effects, bonus rounds,
 * and player statistics.
 *
 * ## Responsibilities
 * - Register and store bonus definitions from data assets
 * - Manage spawn points and respawn timing
 * - Track active bonuses per player and tick their timers
 * - Run bonus round mini-games
 * - Award combo bonuses based on player performance
 * - Handle secret bonus discovery
 * - Persist player bonus statistics
 *
 * ## Update Flow
 * Call UpdateBonusSystem() each tick to:
 * 1. Decrement active bonus timers
 * 2. Expire bonuses when timers reach zero
 * 3. Tick spawn point respawn timers
 * 4. Update bonus round timers and check completion
 */
UCLASS()
class MIDNIGHTGRIND_API UMGBonusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusCollected OnBonusCollected;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusActivated OnBonusActivated;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusExpired OnBonusExpired;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusStacked OnBonusStacked;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnMultiplierChanged OnMultiplierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusRoundStart OnBonusRoundStart;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusRoundComplete OnBonusRoundComplete;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusRoundFailed OnBonusRoundFailed;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnComboBonusTriggered OnComboBonusTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnSecretBonusFound OnSecretBonusFound;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusSpawned OnBonusSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusRespawned OnBonusRespawned;

	// Definition Registration
	UFUNCTION(BlueprintCallable, Category = "Bonus|Definition")
	void RegisterBonusDefinition(const FMGBonusDefinition& Definition);

	UFUNCTION(BlueprintPure, Category = "Bonus|Definition")
	FMGBonusDefinition GetBonusDefinition(const FString& BonusId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Definition")
	TArray<FMGBonusDefinition> GetAllDefinitions() const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Definition")
	TArray<FMGBonusDefinition> GetDefinitionsByType(EMGBonusType Type) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Definition")
	TArray<FMGBonusDefinition> GetDefinitionsByRarity(EMGBonusRarity Rarity) const;

	// Spawn Points
	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void RegisterSpawnPoint(const FMGBonusSpawnPoint& SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void UnregisterSpawnPoint(const FString& SpawnId);

	UFUNCTION(BlueprintPure, Category = "Bonus|Spawn")
	FMGBonusSpawnPoint GetSpawnPoint(const FString& SpawnId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Spawn")
	TArray<FMGBonusSpawnPoint> GetAllSpawnPoints() const;

	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void SpawnBonus(const FString& SpawnId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void SpawnAllBonuses();

	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void RespawnBonus(const FString& SpawnId);

	// Collection
	UFUNCTION(BlueprintCallable, Category = "Bonus|Collection")
	FMGActiveBonus CollectBonus(const FString& PlayerId, const FString& SpawnId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Collection")
	void GrantBonus(const FString& PlayerId, const FString& BonusId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Collection")
	bool TryCollectAtLocation(const FString& PlayerId, FVector Location);

	UFUNCTION(BlueprintPure, Category = "Bonus|Collection")
	FString GetNearestBonusSpawnId(FVector Location, float MaxDistance) const;

	// Active Bonuses
	UFUNCTION(BlueprintPure, Category = "Bonus|Active")
	TArray<FMGActiveBonus> GetActiveBonuses(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Active")
	FMGActiveBonus GetActiveBonus(const FString& PlayerId, const FString& BonusId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Active")
	bool HasActiveBonus(const FString& PlayerId, EMGBonusType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Bonus|Active")
	void PauseBonus(const FString& PlayerId, const FString& ActiveId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Active")
	void ResumeBonus(const FString& PlayerId, const FString& ActiveId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Active")
	void CancelBonus(const FString& PlayerId, const FString& ActiveId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Active")
	void ClearAllBonuses(const FString& PlayerId);

	// Multipliers
	UFUNCTION(BlueprintPure, Category = "Bonus|Multiplier")
	float GetTotalMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Multiplier")
	float GetScoreMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Multiplier")
	float GetXPMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Multiplier")
	float GetCashMultiplier(const FString& PlayerId) const;

	// Bonus Rounds
	UFUNCTION(BlueprintCallable, Category = "Bonus|Round")
	void RegisterBonusRound(const FMGBonusRound& Round);

	UFUNCTION(BlueprintPure, Category = "Bonus|Round")
	FMGBonusRound GetBonusRound(const FString& RoundId) const;

	UFUNCTION(BlueprintCallable, Category = "Bonus|Round")
	void StartBonusRound(const FString& PlayerId, const FString& RoundId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Round")
	void UpdateBonusRound(const FString& PlayerId, int32 ScoreGained);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Round")
	void EndBonusRound(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Bonus|Round")
	FMGActiveBonusRound GetActiveBonusRound(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Round")
	bool IsInBonusRound(const FString& PlayerId) const;

	// Combo Bonuses
	UFUNCTION(BlueprintCallable, Category = "Bonus|Combo")
	void ProcessComboBonus(const FString& PlayerId, int32 ComboCount, float ComboMultiplier);

	UFUNCTION(BlueprintPure, Category = "Bonus|Combo")
	int32 GetComboBonusPoints(int32 ComboLevel) const;

	// Secret Bonuses
	UFUNCTION(BlueprintCallable, Category = "Bonus|Secret")
	void RegisterSecretBonus(const FString& SecretId, const FString& BonusId, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Secret")
	bool TryDiscoverSecret(const FString& PlayerId, FVector Location);

	UFUNCTION(BlueprintPure, Category = "Bonus|Secret")
	bool IsSecretDiscovered(const FString& SecretId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Secret")
	TArray<FString> GetDiscoveredSecrets(const FString& PlayerId) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Bonus|Stats")
	FMGBonusPlayerStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Bonus|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Bonus|Config")
	void SetBonusConfig(const FMGBonusConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Bonus|Config")
	FMGBonusConfig GetBonusConfig() const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "Bonus|Update")
	void UpdateBonusSystem(float DeltaTime);

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Bonus|Persistence")
	void SaveBonusData();

	UFUNCTION(BlueprintCallable, Category = "Bonus|Persistence")
	void LoadBonusData();

protected:
	void TickBonus(float DeltaTime);
	void UpdateActiveBonuses(float DeltaTime);
	void UpdateSpawnRespawns(float DeltaTime);
	void UpdateBonusRounds(float DeltaTime);
	void ApplyBonusEffect(const FString& PlayerId, const FMGBonusDefinition& Bonus);
	void RemoveBonusEffect(const FString& PlayerId, const FMGActiveBonus& Bonus);
	void UpdatePlayerStats(const FString& PlayerId, const FMGBonusDefinition& Bonus, int32 Points);
	FString SelectRandomBonus(const TArray<FString>& PossibleIds) const;
	FString GenerateActiveId() const;
	FString GenerateEventId() const;

private:
	UPROPERTY()
	TMap<FString, FMGBonusDefinition> Definitions;

	UPROPERTY()
	TMap<FString, FMGBonusSpawnPoint> SpawnPoints;

	UPROPERTY()
	TMap<FString, TArray<FMGActiveBonus>> PlayerActiveBonuses;

	UPROPERTY()
	TMap<FString, FMGBonusRound> BonusRounds;

	UPROPERTY()
	TMap<FString, FMGActiveBonusRound> ActiveBonusRounds;

	UPROPERTY()
	TMap<FString, FMGBonusPlayerStats> PlayerStats;

	UPROPERTY()
	TMap<FString, FString> SecretBonuses;

	UPROPERTY()
	TMap<FString, FVector> SecretLocations;

	UPROPERTY()
	TArray<FString> DiscoveredSecrets;

	UPROPERTY()
	FMGBonusConfig BonusConfig;

	UPROPERTY()
	int32 ActiveIdCounter = 0;

	UPROPERTY()
	int32 EventCounter = 0;

	FTimerHandle BonusTickTimer;
};

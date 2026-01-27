// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGContentGatingSubsystem.h
 * ============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the Content Gating Subsystem - a system that controls what
 * content players can access based on their progression. Think of it as the
 * "bouncer" that checks if you're allowed into certain areas of the game.
 *
 * In racing games like Midnight Grind, players don't start with access to
 * everything. They need to prove themselves by earning "REP" (Reputation) points
 * through winning races, performing well, and building their street cred.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * ---------------------------------
 *
 * 1. REP (Reputation):
 *    - The currency of respect in the street racing world
 *    - Earned by winning races, pulling off impressive moves, evading police
 *    - Lost by losing races, getting busted by cops, crashing out
 *    - REP determines your "tier" which unlocks new content
 *
 * 2. Reputation Tiers:
 *    - Unknown (Tier 0): You're a nobody, just starting out
 *    - Rookie (Tier 1): People are starting to notice you
 *    - Known (Tier 2): You've made a name for yourself
 *    - Respected (Tier 3): Veterans take you seriously
 *    - Feared (Tier 4): You're one of the best
 *    - Legend (Tier 5): You've reached the top of the racing world
 *
 * 3. Content Gating:
 *    - Certain locations, race types, and features are "gated" (locked)
 *    - Players must meet requirements (REP tier, race wins, etc.) to unlock them
 *    - This creates a sense of progression and achievement
 *
 * 4. Pink Slips:
 *    - High-stakes races where you bet your car title
 *    - Winner takes the loser's car
 *    - Restricted by vehicle class to prevent unfair matchups
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ----------------------------------------
 *
 *   [Player completes race]
 *            |
 *            v
 *   [MGRaceRewardsProcessor] -- calculates REP earned
 *            |
 *            v
 *   [THIS SUBSYSTEM] -- AddREP() is called
 *            |
 *            +-- Updates player's REP total
 *            +-- Checks if tier increased
 *            +-- Unlocks new content if requirements met
 *            +-- Broadcasts events for UI updates
 *            |
 *            v
 *   [UI/Menus] -- Listen for events, show unlock notifications
 *
 * This subsystem is a GameInstanceSubsystem, meaning:
 * - One instance exists for the entire game session
 * - It persists across level changes
 * - Access it via: GetGameInstance()->GetSubsystem<UMGContentGatingSubsystem>()
 *
 * USAGE EXAMPLES:
 * ---------------
 *
 * // Check if player can access a location
 * UMGContentGatingSubsystem* GatingSys = GetGameInstance()->GetSubsystem<UMGContentGatingSubsystem>();
 * if (GatingSys->CanAccessLocation(PlayerID, "Downtown"))
 * {
 *     // Allow player to enter Downtown
 * }
 *
 * // Award REP after a race win
 * GatingSys->AddREP(PlayerID, 50, "Circuit race win");
 *
 * // Check player's current tier
 * EMGReputationTier Tier = GatingSys->GetPlayerTier(PlayerID);
 * if (Tier >= EMGReputationTier::Respected)
 * {
 *     // Player is respected or higher - show elite content
 * }
 *
 * RELATED FILES:
 * --------------
 * - MGPlayerProgression.h: Handles XP, levels, and crew-specific reputation
 * - MGRaceRewardsProcessor.h: Calculates rewards including REP after races
 *
 * PRD REFERENCE:
 * --------------
 * This implements Section 4.2 of the Product Requirements Document (PRD),
 * which defines the Reputation System and progression mechanics.
 *
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGContentGatingSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * EMGReputationTier - The player's standing in the street racing world
 *
 * Reputation tiers are the main progression milestones in the game.
 * Each tier unlocks new locations, race types, and features.
 *
 * The tiers follow a hierarchy where each level requires significantly more
 * REP than the last, creating a satisfying progression curve:
 *
 *   Tier       | REP Required | Typical Time to Reach
 *   -----------|--------------|----------------------
 *   Unknown    | 0            | Start of game
 *   Rookie     | 100          | ~1 hour of play
 *   Known      | 500          | ~5 hours of play
 *   Respected  | 1,500        | ~15 hours of play
 *   Feared     | 5,000        | ~40 hours of play
 *   Legend     | 15,000       | ~100+ hours of play
 *
 * Per PRD Section 4.2 - these thresholds are defined at the bottom of this file
 * in the TierThresholds array.
 */
UENUM(BlueprintType)
enum class EMGReputationTier : uint8
{
	// Tier 0: Player has just started, no reputation yet
	// Content available: Basic tutorial area, starter races
	Unknown UMETA(DisplayName = "Unknown (Tier 0)"),

	// Tier 1: Player has proven they can race
	// Content available: First neighborhood unlocks, basic race types
	Rookie UMETA(DisplayName = "Rookie (Tier 1)"),

	// Tier 2: Player is becoming a recognized racer
	// Content available: More neighborhoods, sprint races unlock
	Known UMETA(DisplayName = "Known (Tier 2)"),

	// Tier 3: Player has earned respect from other racers
	// Content available: Pink slip races, advanced race types
	Respected UMETA(DisplayName = "Respected (Tier 3)"),

	// Tier 4: Player is one of the elite, others fear racing them
	// Content available: High-stakes events, exclusive locations
	Feared UMETA(DisplayName = "Feared (Tier 4)"),

	// Tier 5: Player has reached legendary status
	// Content available: Everything - full game access
	Legend UMETA(DisplayName = "Legend (Tier 5)")
};

/**
 * EMGPinkSlipClass - Vehicle performance classes for pink slip (title) races
 *
 * Pink slip races are high-stakes events where players bet their car titles.
 * To ensure fair competition, cars are divided into performance classes.
 * You can only bet your car against cars in the same class.
 *
 * Performance classes are determined by a car's PI (Performance Index),
 * which is calculated from speed, acceleration, handling, and braking stats.
 *
 *   Class | PI Range  | Typical Vehicles
 *   ------|-----------|------------------
 *   D     | 0-300     | Economy cars, old beaters
 *   C     | 301-400   | Basic sports cars, hot hatches
 *   B     | 401-500   | Muscle cars, sport sedans
 *   A     | 501-600   | Sports cars, tuned imports
 *   S     | 601-700   | Supercars, heavily modified builds
 *   X     | 701+      | Hypercars, unlimited class
 *
 * Players must reach certain reputation tiers to participate in higher-class
 * pink slip races, preventing new players from losing expensive cars early.
 */
UENUM(BlueprintType)
enum class EMGPinkSlipClass : uint8
{
	D UMETA(DisplayName = "D Class"),  // Available from the start
	C UMETA(DisplayName = "C Class"),  // Unlocks at Rookie tier
	B UMETA(DisplayName = "B Class"),  // Unlocks at Known tier
	A UMETA(DisplayName = "A Class"),  // Unlocks at Respected tier
	S UMETA(DisplayName = "S Class"),  // Unlocks at Feared tier
	X UMETA(DisplayName = "X Class")   // Unlocks at Legend tier (no restrictions)
};

/**
 * EMGGatedContentType - Categories of content that can be locked/unlocked
 *
 * The content gating system can restrict access to many different types
 * of game content. This enum identifies what category a piece of content
 * belongs to, which helps determine:
 *
 * 1. How to display locked content in the UI (different icons/messages)
 * 2. Which requirements to check for unlocking
 * 3. How to notify the player when content is unlocked
 */
UENUM(BlueprintType)
enum class EMGGatedContentType : uint8
{
	// Map areas/neighborhoods that require reputation to access
	// Example: "Downtown" requires Known tier to enter
	Location UMETA(DisplayName = "Map Location"),

	// Types of races that require reputation/wins to participate in
	// Example: "Drift Events" require 5 circuit wins
	RaceType UMETA(DisplayName = "Race Type"),

	// Pink slip class restrictions - which class cars you can bet
	// Example: "S Class Pink Slips" require Feared tier
	PinkSlipClass UMETA(DisplayName = "Pink Slip Class"),

	// Specific vehicles that require unlocking before purchase
	// Example: "Legendary Car X" requires Legend tier
	Vehicle UMETA(DisplayName = "Vehicle Purchase"),

	// Performance parts or cosmetics that require unlocking
	// Example: "Stage 3 Turbo" requires Respected tier
	Part UMETA(DisplayName = "Part Purchase"),

	// Game features like tuning shops, online modes, etc.
	// Example: "Advanced Tuning" requires Level 10
	Feature UMETA(DisplayName = "Game Feature")
};

// ============================================================================
// STRUCTURES - Defining Unlock Requirements
// ============================================================================

/**
 * FMGUnlockRequirement - Defines what a player needs to unlock something
 *
 * This structure specifies all the conditions that must be met for a player
 * to unlock a piece of content. Multiple conditions can be combined - the
 * player must meet ALL non-zero requirements (AND logic, not OR).
 *
 * EXAMPLE: An unlock that requires:
 * - Respected tier (RequiredTier = EMGReputationTier::Respected)
 * - At least 1000 REP (RequiredREP = 1000)
 * - 5 race wins (RequiredRaceWins = 5)
 *
 * The player must meet ALL three conditions to unlock the content.
 *
 * Zero/None values mean "no requirement" for that field:
 * - RequiredREP = 0 means no specific REP amount needed
 * - RequiredAchievementID = NAME_None means no achievement needed
 */
USTRUCT(BlueprintType)
struct FMGUnlockRequirement
{
	GENERATED_BODY()

	// The minimum reputation tier the player must reach
	// Example: EMGReputationTier::Respected means player needs Respected or higher
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier RequiredTier = EMGReputationTier::Unknown;

	// The exact amount of REP points required (cumulative, not per-tier)
	// This is separate from tier - you might require a specific REP amount
	// Example: 2500 REP even though Respected only needs 1500
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredREP = 0;

	// ID of an achievement that must be completed first
	// Achievements are tracked in a separate system
	// Example: "ACH_FirstPinkSlipWin" - must win a pink slip first
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredAchievementID;

	// Total number of race wins needed (any type of race)
	// Example: 25 total wins to unlock high-stakes events
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredRaceWins = 0;

	// ID of another unlock that must be acquired first
	// Creates unlock chains: A -> B -> C
	// Example: "BasicTurbo" must be unlocked before "AdvancedTurbo"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredPreviousUnlock;

	// Human-readable description of what's needed (for UI display)
	// Example: "Reach Respected tier and win 5 circuit races"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnlockDescription;
};

/**
 * FMGGatedContent - A piece of content with its unlock requirements
 *
 * This combines identity information about the content (what it is, what to
 * call it) with its requirements (what the player needs to access it).
 *
 * The game maintains a list of all gated content, and this system checks
 * each one against the player's current progress whenever they gain REP
 * or complete achievements.
 */
USTRUCT(BlueprintType)
struct FMGGatedContent
{
	GENERATED_BODY()

	// Unique identifier used in code to reference this content
	// Example: "LOC_Downtown", "RACE_Drift", "VEH_SupercarX"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ContentID;

	// Name shown to the player in UI
	// Example: "Downtown District"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	// What category of content this is (for filtering and display)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGatedContentType ContentType = EMGGatedContentType::Location;

	// The requirements to unlock this content
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGUnlockRequirement Requirement;

	// Should this content appear in menus even when locked?
	// true = Show greyed out with lock icon (teases upcoming content)
	// false = Completely hidden until unlocked (surprise content)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowWhenLocked = true;

	// Message shown when player tries to access locked content
	// Example: "Reach Respected tier to access this location"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LockedHintText;
};

// ============================================================================
// STRUCTURES - Game World Data
// ============================================================================

/**
 * FMGLocationData - Information about a map area/neighborhood
 *
 * Midnight Grind's world is divided into distinct neighborhoods, each with
 * its own character, races, and reputation requirements. This structure
 * defines a single location in the game world.
 *
 * PRD Reference: Section 4.2 defines which locations unlock at which tiers.
 */
USTRUCT(BlueprintType)
struct FMGLocationData
{
	GENERATED_BODY()

	// Unique identifier for this location (used in code and save data)
	// Example: "Downtown", "Industrial", "Suburbs"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationID;

	// Displayed name in the UI
	// Example: "Downtown - Neon District"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	// Flavor text describing the location
	// Example: "The heart of the city's nightlife. High stakes, bright lights."
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Minimum tier needed to access this location
	// The player must reach this tier (or higher) to enter
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier RequiredTier = EMGReputationTier::Unknown;

	// List of race type IDs available in this location
	// Different locations offer different race types
	// Example: Downtown might have Circuit and Sprint but not Drift
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AvailableRaceTypes;

	// Is this a hidden location not shown on the map until discovered?
	// Secret locations might be revealed by completing specific challenges
	// or reaching certain milestones
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSecretLocation = false;
};

/**
 * FMGRaceTypeUnlockData - Unlock requirements for a type of race
 *
 * Different race formats (circuit, sprint, drift, drag) become available
 * as the player progresses. This structure defines when each race type
 * unlocks and any prerequisites needed.
 */
USTRUCT(BlueprintType)
struct FMGRaceTypeUnlockData
{
	GENERATED_BODY()

	// Unique identifier for this race type
	// Example: "Circuit", "Sprint", "Drift", "Drag", "PinkSlip"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceTypeID;

	// Display name shown in race selection UI
	// Example: "Drift Challenge"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	// Minimum tier to access this race type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier RequiredTier = EMGReputationTier::Unknown;

	// Number of circuit race wins needed to unlock
	// Circuit races are the most basic type, so other types may require
	// the player to prove themselves in circuits first
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredCircuitWins = 0;

	// Number of sprint race wins needed to unlock
	// Some advanced race types may require sprint experience
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredSprintWins = 0;
};

// ============================================================================
// STRUCTURES - Player State Tracking
// ============================================================================

/**
 * FMGPlayerUnlockState - Complete snapshot of a player's progression state
 *
 * This structure holds everything we need to know about a player's progress
 * in the content gating system. It's used for:
 *
 * 1. Checking if content is unlocked (comparing against requirements)
 * 2. Saving/loading progression data
 * 3. Displaying progress in UI (how close to next unlock, etc.)
 *
 * The system maintains one of these per player (identified by PlayerID).
 * In single-player, there's typically just one. In multiplayer, each
 * connected player has their own state.
 */
USTRUCT(BlueprintType)
struct FMGPlayerUnlockState
{
	GENERATED_BODY()

	// Unique identifier for this player (matches their save data)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PlayerID;

	// ---- REP & Tier ----

	// Current total REP points accumulated
	// This is cumulative - it never decreases (though you can "lose" REP
	// from race losses, which just slows your gain, not actual point loss)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentREP = 0;

	// Current reputation tier (derived from CurrentREP using TierThresholds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier CurrentTier = EMGReputationTier::Unknown;

	// ---- Content Unlocks ----
	// These sets track which specific content has been unlocked
	// Using TSet for O(1) lookup performance

	// IDs of all locations the player can access
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> UnlockedLocations;

	// IDs of all race types the player can participate in
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> UnlockedRaceTypes;

	// IDs of achievements the player has completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> UnlockedAchievements;

	// ---- Statistics for Unlock Requirements ----

	// Win count per race type (key = race type ID, value = wins)
	// Example: {"Circuit": 15, "Sprint": 8, "Drift": 3}
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> RaceTypeWinCounts;

	// Which pink slip vehicle classes the player can participate in
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<EMGPinkSlipClass> UnlockedPinkSlipClasses;

	// ---- Career Statistics ----

	// Total race wins across all race types
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaceWins = 0;

	// Total pink slip races won (high-stakes title races)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPinkSlipWins = 0;

	// Total in-game currency earned (for information/leaderboards)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCash = 0;
};

// ============================================================================
// DELEGATES - Event Broadcasting
// ============================================================================

/**
 * Delegates are Unreal's way of implementing the Observer pattern.
 * Other systems can "subscribe" to these events and be notified when they fire.
 *
 * DYNAMIC = Can be bound in Blueprints (slower but more flexible)
 * MULTICAST = Multiple listeners can subscribe (1-to-many notification)
 *
 * Usage Example (in C++):
 *   GatingSubsystem->OnTierUnlocked.AddDynamic(this, &UMyClass::HandleTierUnlock);
 *
 * Usage Example (in Blueprint):
 *   Bind to "On Tier Unlocked" event in the Event Graph
 */

// Fired when player reaches a new reputation tier
// Parameters: Player's ID, the new tier they just reached
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTierUnlocked, FGuid, PlayerID, EMGReputationTier, NewTier);

// Fired when any content is unlocked (location, race type, etc.)
// Parameters: Player's ID, the content's ID, what type of content it is
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnContentUnlocked, FGuid, PlayerID, FName, ContentID, EMGGatedContentType, ContentType);

// Fired whenever REP changes (useful for UI progress bars)
// Parameters: Player's ID, their new total REP amount
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnREPChanged, FGuid, PlayerID, int32, NewREP);

// ============================================================================
// MAIN CLASS - UMGContentGatingSubsystem
// ============================================================================

/**
 * UMGContentGatingSubsystem - The core content gating system
 *
 * This Game Instance Subsystem is the single source of truth for:
 * - What content exists and its unlock requirements
 * - Each player's current REP and tier
 * - What each player has unlocked
 *
 * SUBSYSTEM LIFECYCLE:
 * --------------------
 * - Created: When the game instance is created (game startup)
 * - Initialize(): Called once, sets up default content definitions
 * - Active: Throughout the entire game session
 * - Deinitialize(): Called on game shutdown, cleanup
 * - Destroyed: When the game instance is destroyed
 *
 * THREAD SAFETY:
 * --------------
 * This subsystem should only be accessed from the game thread.
 * All operations are synchronous.
 *
 * ACCESSING THE SUBSYSTEM:
 * ------------------------
 * // From any Actor or Component:
 * UMGContentGatingSubsystem* Gating = GetGameInstance()->GetSubsystem<UMGContentGatingSubsystem>();
 *
 * // From a UObject:
 * UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld());
 * UMGContentGatingSubsystem* Gating = GI->GetSubsystem<UMGContentGatingSubsystem>();
 *
 * Per PRD Section 4.2: Reputation System
 */
UCLASS()
class MIDNIGHTGRIND_API UMGContentGatingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================================================
	// LIFECYCLE
	// ========================================================================

	/**
	 * Called when the subsystem is created.
	 * Sets up default content definitions (locations, race types, etc.)
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Called when the subsystem is being destroyed.
	 * Cleanup any resources, save state if needed.
	 */
	virtual void Deinitialize() override;

	// ========================================================================
	// REP MANAGEMENT
	// ========================================================================
	// These functions modify and query the player's REP points.
	// REP is the main progression currency that determines tier.

	/**
	 * Award REP to a player (typically after winning a race).
	 *
	 * This function:
	 * 1. Adds the specified amount to the player's total REP
	 * 2. Checks if a tier threshold was crossed
	 * 3. If tier increased, unlocks tier-gated content
	 * 4. Broadcasts OnREPChanged (and OnTierUnlocked if applicable)
	 *
	 * @param PlayerID   The player receiving the REP
	 * @param Amount     How much REP to add (positive number)
	 * @param Reason     Description for debugging/analytics (e.g., "Circuit race win")
	 *
	 * Usage Example:
	 *   // Award 50 REP for winning a race
	 *   GatingSubsystem->AddREP(MyPlayerID, 50, "Won circuit race at Downtown");
	 */
	UFUNCTION(BlueprintCallable, Category = "REP")
	void AddREP(FGuid PlayerID, int32 Amount, const FString& Reason);

	/**
	 * Remove REP from a player (from losing, getting busted, etc.).
	 *
	 * Note: REP typically doesn't go below 0, and you can't lose tier once
	 * reached. This is more about slowing progression than punishment.
	 *
	 * @param PlayerID   The player losing REP
	 * @param Amount     How much REP to remove (positive number)
	 * @param Reason     Description for debugging (e.g., "Lost pink slip race")
	 */
	UFUNCTION(BlueprintCallable, Category = "REP")
	void RemoveREP(FGuid PlayerID, int32 Amount, const FString& Reason);

	/**
	 * Get a player's current REP total.
	 *
	 * @param PlayerID   The player to query
	 * @return           Current REP points (0 if player not found)
	 */
	UFUNCTION(BlueprintPure, Category = "REP")
	int32 GetPlayerREP(FGuid PlayerID) const;

	/**
	 * Get a player's current reputation tier.
	 *
	 * @param PlayerID   The player to query
	 * @return           Current tier (Unknown if player not found)
	 *
	 * Usage Example:
	 *   EMGReputationTier Tier = GatingSubsystem->GetPlayerTier(MyPlayerID);
	 *   if (Tier >= EMGReputationTier::Feared)
	 *   {
	 *       // Show elite content options
	 *   }
	 */
	UFUNCTION(BlueprintPure, Category = "REP")
	EMGReputationTier GetPlayerTier(FGuid PlayerID) const;

	/**
	 * Get how much REP is needed to reach the next tier.
	 *
	 * @param CurrentTier   The tier to check progression from
	 * @return              REP needed for next tier, or 0 if already Legend
	 *
	 * Usage Example:
	 *   int32 NeededForNext = GatingSubsystem->GetREPForNextTier(EMGReputationTier::Known);
	 *   // Returns 1500 (Respected tier threshold)
	 */
	UFUNCTION(BlueprintPure, Category = "REP")
	int32 GetREPForNextTier(EMGReputationTier CurrentTier) const;

	/**
	 * Get progress toward the next tier as a fraction (0.0 to 1.0).
	 * Useful for progress bars in the UI.
	 *
	 * @param PlayerID   The player to query
	 * @return           Progress fraction (0.0 = just reached current tier, 1.0 = about to tier up)
	 *
	 * Usage Example:
	 *   float Progress = GatingSubsystem->GetTierProgress(MyPlayerID);
	 *   ProgressBar->SetPercent(Progress);  // Update UI progress bar
	 */
	UFUNCTION(BlueprintPure, Category = "REP")
	float GetTierProgress(FGuid PlayerID) const;

	// ========================================================================
	// REP REWARDS (per PRD Section 4.2)
	// ========================================================================
	// These functions calculate how much REP to award/deduct for various events.
	// They don't modify state - just return calculated values.

	/**
	 * Calculate REP reward for winning a race.
	 *
	 * The calculation considers multiple factors:
	 * - Base REP for the race type (circuit is worth more than sprint)
	 * - Win margin (dominating earns more than barely winning)
	 * - Clean race bonus (no collisions = bonus REP)
	 * - Comeback bonus (winning after being far behind)
	 * - Opponent count (more racers = more REP)
	 *
	 * @param RaceType      The type of race (Circuit, Sprint, Drift, etc.)
	 * @param WinMargin     How far ahead you finished (in seconds)
	 * @param bCleanRace    True if no collisions during the race
	 * @param bComeback     True if player came from behind to win
	 * @param OpponentCount Number of opponents in the race
	 * @return              Calculated REP reward
	 */
	UFUNCTION(BlueprintPure, Category = "REP|Rewards")
	int32 CalculateRaceWinREP(FName RaceType, float WinMargin, bool bCleanRace, bool bComeback, int32 OpponentCount) const;

	/**
	 * Calculate REP loss for losing a race.
	 * REP loss is typically much smaller than potential gains.
	 *
	 * @param RaceType    The type of race
	 * @param LossMargin  How far behind you finished (in seconds)
	 * @return            Amount of REP to deduct
	 */
	UFUNCTION(BlueprintPure, Category = "REP|Rewards")
	int32 CalculateRaceLossREP(FName RaceType, float LossMargin) const;

	/**
	 * Calculate REP loss for getting busted by police.
	 * Higher heat levels (more wanted stars) = more REP loss.
	 *
	 * @param HeatLevel   Current wanted level (1-5 typically)
	 * @return            Amount of REP to deduct
	 */
	UFUNCTION(BlueprintPure, Category = "REP|Rewards")
	int32 CalculateBustREPLoss(int32 HeatLevel) const;

	// ========================================================================
	// CONTENT ACCESS CHECKS
	// ========================================================================
	// These functions check if a player meets requirements to access content.
	// They don't modify state - just return true/false.

	/**
	 * Check if a player can access a map location.
	 *
	 * @param PlayerID    The player to check
	 * @param LocationID  ID of the location to check access for
	 * @return            True if player can enter this location
	 *
	 * Usage Example:
	 *   if (!GatingSubsystem->CanAccessLocation(PlayerID, "Downtown"))
	 *   {
	 *       ShowLockedLocationUI("Downtown");
	 *   }
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	bool CanAccessLocation(FGuid PlayerID, FName LocationID) const;

	/**
	 * Check if a player can participate in a race type.
	 *
	 * @param PlayerID    The player to check
	 * @param RaceTypeID  ID of the race type (e.g., "Drift", "Drag")
	 * @return            True if player can race in this mode
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	bool CanAccessRaceType(FGuid PlayerID, FName RaceTypeID) const;

	/**
	 * Check if a player can participate in pink slip races of a given class.
	 *
	 * @param PlayerID      The player to check
	 * @param VehicleClass  The performance class to check
	 * @return              True if player can bet cars of this class
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	bool CanAccessPinkSlipClass(FGuid PlayerID, EMGPinkSlipClass VehicleClass) const;

	/**
	 * Generic check if any content is unlocked.
	 * Use this for content that doesn't fit into specific categories.
	 *
	 * @param PlayerID   The player to check
	 * @param ContentID  ID of the content to check
	 * @return           True if content is unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	bool IsContentUnlocked(FGuid PlayerID, FName ContentID) const;

	/**
	 * Get the requirements for unlocking specific content.
	 * Useful for displaying "What you need to unlock this" in UI.
	 *
	 * @param ContentID  ID of the content to query
	 * @return           The unlock requirements (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	FMGUnlockRequirement GetUnlockRequirement(FName ContentID) const;

	// ========================================================================
	// UNLOCK QUERIES
	// ========================================================================
	// Functions to query what content is unlocked or available.

	/**
	 * Get all locations a player has unlocked.
	 *
	 * @param PlayerID  The player to query
	 * @return          Array of location IDs the player can access
	 */
	UFUNCTION(BlueprintPure, Category = "Unlocks")
	TArray<FName> GetUnlockedLocations(FGuid PlayerID) const;

	/**
	 * Get all race types a player has unlocked.
	 *
	 * @param PlayerID  The player to query
	 * @return          Array of race type IDs the player can participate in
	 */
	UFUNCTION(BlueprintPure, Category = "Unlocks")
	TArray<FName> GetUnlockedRaceTypes(FGuid PlayerID) const;

	/**
	 * Get all locations that unlock at a specific tier.
	 * Useful for "Coming Soon" displays in UI.
	 *
	 * @param Tier  The tier to query
	 * @return      Array of location data for that tier
	 */
	UFUNCTION(BlueprintPure, Category = "Unlocks")
	TArray<FMGLocationData> GetLocationsByTier(EMGReputationTier Tier) const;

	/**
	 * Get the next pieces of content the player is closest to unlocking.
	 * Useful for showing "Progress toward:" in the UI.
	 *
	 * @param PlayerID  The player to query
	 * @return          Array of content the player is working toward
	 */
	UFUNCTION(BlueprintPure, Category = "Unlocks")
	TArray<FMGGatedContent> GetNextUnlockableContent(FGuid PlayerID) const;

	// ========================================================================
	// PLAYER STATE MANAGEMENT
	// ========================================================================

	/**
	 * Initialize tracking for a new player.
	 * Call this when a player joins or starts a new game.
	 *
	 * @param PlayerID  Unique ID for the player
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void InitializePlayer(FGuid PlayerID);

	/**
	 * Get the complete unlock state for a player.
	 *
	 * @param PlayerID  The player to query
	 * @param OutState  Output parameter filled with player's state
	 * @return          True if player was found, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "State")
	bool GetPlayerUnlockState(FGuid PlayerID, FMGPlayerUnlockState& OutState) const;

	/**
	 * Record that a player won a race of a specific type.
	 * Updates win counters which may trigger unlock checks.
	 *
	 * @param PlayerID  The player who won
	 * @param RaceType  The type of race won
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void RecordRaceWin(FGuid PlayerID, FName RaceType);

	/**
	 * Record that a player won a pink slip race.
	 *
	 * @param PlayerID      The player who won
	 * @param VehicleClass  The class of vehicle that was won
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void RecordPinkSlipWin(FGuid PlayerID, EMGPinkSlipClass VehicleClass);

	// ========================================================================
	// EVENTS (Delegates)
	// ========================================================================
	// Other systems can bind to these to receive notifications.

	// Broadcast when a player reaches a new reputation tier
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTierUnlocked OnTierUnlocked;

	// Broadcast when any content is unlocked
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnContentUnlocked OnContentUnlocked;

	// Broadcast whenever REP changes (for progress bars, etc.)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnREPChanged OnREPChanged;

protected:
	// ========================================================================
	// INTERNAL DATA
	// ========================================================================

	// Map of player ID -> their complete unlock state
	// This is the primary data store for player progression
	UPROPERTY()
	TMap<FGuid, FMGPlayerUnlockState> PlayerStates;

	// Definition of all locations in the game and their requirements
	// Populated in SetupLocations() during Initialize()
	UPROPERTY()
	TArray<FMGLocationData> LocationDefinitions;

	// Definition of all race types and their requirements
	// Populated in SetupRaceTypes() during Initialize()
	UPROPERTY()
	TArray<FMGRaceTypeUnlockData> RaceTypeDefinitions;

	// Definition of all gated content (generic)
	// Populated in SetupDefaultContent() during Initialize()
	UPROPERTY()
	TArray<FMGGatedContent> GatedContentDefinitions;

	// ========================================================================
	// INTERNAL FUNCTIONS
	// ========================================================================

	// Sets up all default content definitions (calls the other Setup functions)
	void SetupDefaultContent();

	// Populates LocationDefinitions with all game locations
	void SetupLocations();

	// Populates RaceTypeDefinitions with all race types
	void SetupRaceTypes();

	// Configures which pink slip classes unlock at which tiers
	void SetupPinkSlipClasses();

	// Calculates which tier a given REP amount corresponds to
	EMGReputationTier CalculateTierFromREP(int32 REP) const;

	// Checks all gated content and unlocks anything the player now qualifies for
	void CheckAndUnlockContent(FGuid PlayerID);

	// Unlocks all content associated with reaching a specific tier
	void UnlockTierContent(FGuid PlayerID, EMGReputationTier Tier);

	// ========================================================================
	// TIER THRESHOLDS
	// ========================================================================
	/**
	 * REP thresholds for each tier (cumulative values).
	 *
	 * These are static constexpr so they're compiled into the code and
	 * don't take up runtime memory allocation. They define the progression
	 * curve for the entire game.
	 *
	 * To reach a tier, the player must have >= the threshold REP.
	 *
	 * Example progression:
	 * - Player starts at 0 REP = Unknown tier
	 * - Player reaches 100 REP = Rookie tier
	 * - Player reaches 500 REP = Known tier
	 * - etc.
	 */
	static constexpr int32 TierThresholds[] = {
		0,      // Tier 0: Unknown - starting tier
		100,    // Tier 1: Rookie - first milestone
		500,    // Tier 2: Known - establishing yourself
		1500,   // Tier 3: Respected - mid-game milestone
		5000,   // Tier 4: Feared - late-game status
		15000   // Tier 5: Legend - end-game achievement
	};
};

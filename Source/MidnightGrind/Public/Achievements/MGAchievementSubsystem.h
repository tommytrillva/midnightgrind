// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGAchievementSubsystem.h
 * @brief Achievement, Badge, Title, and Milestone Tracking Subsystem
 *
 * This subsystem manages the complete achievement system for Midnight Grind, including:
 * - Achievement definitions, progress tracking, and unlock logic
 * - Player badges (displayable icons earned through gameplay)
 * - Player titles (text displayed alongside player names)
 * - Milestones (cumulative stat-based rewards with multiple thresholds)
 *
 * The subsystem listens to game events via stat reporting functions and automatically
 * checks for achievement unlock conditions. When an achievement is unlocked, rewards
 * are granted and platform achievements (Steam, Xbox, PlayStation) are synchronized.
 *
 * @section usage Basic Usage
 * To report player progress, call ReportStatIncrement() or ReportRaceCompletion().
 * The subsystem will automatically check all relevant achievements and trigger
 * unlock events when conditions are met.
 *
 * @section events Key Events
 * - OnAchievementUnlocked: Fired when any achievement is unlocked
 * - OnAchievementProgress: Fired when progress is made toward an achievement
 * - OnBadgeUnlocked: Fired when a new badge becomes available
 * - OnTitleUnlocked: Fired when a new title is earned
 * - OnMilestoneReached: Fired when a milestone threshold is crossed
 *
 * @note This is a GameInstanceSubsystem, meaning it persists across level loads.
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "PlayerTitle/MGPlayerTitleSubsystem.h"
#include "MGAchievementSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS - Achievement Classification
// ============================================================================

/**
 * @brief Categories for organizing achievements in the UI.
 *
 * Achievements are grouped by category to help players find related goals.
 * Each category typically corresponds to a major game system or playstyle.
 */
UENUM(BlueprintType)
enum class EMGAchievementCategory : uint8
{
	Racing			UMETA(DisplayName = "Racing"),       ///< Race wins, podiums, and race-related accomplishments
	Career			UMETA(DisplayName = "Career"),       ///< Career mode progression and story milestones
	Social			UMETA(DisplayName = "Social"),       ///< Multiplayer, crews, and community features
	Collection		UMETA(DisplayName = "Collection"),   ///< Vehicle collection and customization
	Mastery			UMETA(DisplayName = "Mastery"),      ///< Skill-based accomplishments and perfect runs
	Exploration		UMETA(DisplayName = "Exploration"),  ///< Discovering hidden areas and secrets
	Challenge		UMETA(DisplayName = "Challenge"),    ///< Special challenge completions
	Secret			UMETA(DisplayName = "Secret")        ///< Hidden achievements revealed upon unlock
};

// MOVED TO MGSharedTypes.h
// /**
//  * @brief Rarity classification for achievements, badges, and titles.
//  *
//  * Rarity affects visual presentation (colors, effects) and indicates
//  * the relative difficulty or exclusivity of obtaining the item.
//  */
// UENUM(BlueprintType)
// enum class EMGAchievementRarity : uint8
// {
// 	Common			UMETA(DisplayName = "Common"),       ///< Easy to obtain, ~50%+ of players
// 	Uncommon		UMETA(DisplayName = "Uncommon"),     ///< Moderate effort, ~25-50% of players
// 	Rare			UMETA(DisplayName = "Rare"),         ///< Significant effort, ~10-25% of players
// 	Epic			UMETA(DisplayName = "Epic"),         ///< Difficult to obtain, ~1-10% of players
// 	Legendary		UMETA(DisplayName = "Legendary")     ///< Extremely rare, <1% of players
// };

/**
 * @brief Statistics that trigger achievement progress.
 *
 * Each achievement tracks one or more stat types. When these stats are
 * reported via ReportStatIncrement(), the subsystem checks if any
 * achievements should unlock.
 */
UENUM(BlueprintType)
enum class EMGAchievementStatType : uint8
{
	// ---- Racing Statistics ----
	RacesCompleted,           ///< Total number of races finished (any position)
	RacesWon,                 ///< Races finished in 1st place
	FirstPlaceFinishes,       ///< Alias for RacesWon, used in some contexts
	PodiumFinishes,           ///< Races finished in 1st, 2nd, or 3rd place
	PerfectRaces,             ///< Races completed without crashes or penalties
	TotalDistance,            ///< Cumulative distance driven (in meters)
	TotalDriftDistance,       ///< Cumulative distance while drifting (in meters)
	TotalAirTime,             ///< Cumulative time airborne (in seconds)
	TotalNitroUsed,           ///< Total nitrous oxide consumed (in units)
	NearMisses,               ///< Close calls with traffic or obstacles
	Overtakes,                ///< Opponents passed during races

	// ---- Time-Based Statistics ----
	TotalPlayTime,            ///< Total time spent in-game (in seconds)
	TimeInFirstPlace,         ///< Cumulative time leading races (in seconds)
	FastestLap,               ///< Best lap time achieved (tracks best, not cumulative)

	// ---- Multiplayer Statistics ----
	OnlineRacesWon,           ///< Wins in online multiplayer races
	OnlineRacesCompleted,     ///< Online races finished (any position)
	TournamentWins,           ///< Tournament first-place finishes
	TournamentParticipations, ///< Tournaments entered
	RankedWins,               ///< Wins in ranked competitive mode

	// ---- Social Statistics ----
	CrewsJoined,              ///< Number of crews the player has joined
	CrewChallengesCompleted,  ///< Crew-specific challenges completed
	PhotosTaken,              ///< Photos captured in photo mode
	PhotosShared,             ///< Photos shared to social features
	FriendsAdded,             ///< Friends added through the game

	// ---- Collection Statistics ----
	VehiclesOwned,            ///< Total vehicles in the player's garage
	VehiclesMaxUpgraded,      ///< Vehicles fully upgraded to max level
	LiveriesCreated,          ///< Custom liveries designed
	PartsCollected,           ///< Performance parts acquired

	// ---- Economy Statistics ----
	TotalCashEarned,          ///< Lifetime earnings (not current balance)
	TotalXPEarned,            ///< Lifetime experience points earned
	TotalRepEarned,           ///< Lifetime reputation earned
	ItemsPurchased,           ///< Total items bought from shops

	// ---- Track-Specific Statistics ----
	TrackCompleted,           ///< A specific track finished (uses RequiredTrack filter)
	TrackMastered,            ///< Gold time achieved on a track
	AllTracksCompleted,       ///< All tracks in the game finished at least once

	// ---- Special/Streak Statistics ----
	ConsecutiveWins,          ///< Current win streak (resets on loss)
	ConsecutivePodiums,       ///< Current podium streak (resets if outside top 3)
	ComebackWins,             ///< Wins after being in last place
	PhotoFinishes,            ///< Wins within 0.1 seconds of second place

	// ---- Custom Achievement Triggers ----
	Custom                    ///< For achievements with unique unlock logic
};

// ============================================================================
// STRUCTURES - Reward and Achievement Data
// ============================================================================

/**
 * @brief Rewards granted when an achievement is unlocked.
 *
 * Achievements can grant multiple types of rewards simultaneously.
 * Set unused reward fields to 0 or NAME_None.
 */
USTRUCT(BlueprintType)
struct FMGAchievementReward
{
	GENERATED_BODY()

	/// In-game currency reward amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 CashReward = 0;

	/// Experience points reward amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 XPReward = 0;

	/// Reputation points reward amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 ReputationReward = 0;

	/// ID of a title to unlock (NAME_None if no title)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName TitleUnlock;

	/// ID of a badge to unlock (NAME_None if no badge)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName BadgeUnlock;

	/// ID of a vehicle to unlock (NAME_None if no vehicle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName VehicleUnlock;

	/// ID of a generic item to unlock (NAME_None if no item)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName ItemUnlock;

	/// Platform-specific gamerscore/trophy points (Xbox, Steam, PlayStation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 PlatformGamerScore = 0;
};

/**
 * @brief A single tier within a progressive (multi-stage) achievement.
 *
 * Progressive achievements have multiple unlock thresholds, each granting
 * separate rewards. Example: "Win 10/50/100/500 races" would have 4 tiers.
 */
USTRUCT(BlueprintType)
struct FMGAchievementTier
{
	GENERATED_BODY()

	/// Tier number (1-based, higher is more difficult)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	int32 TierLevel = 1;

	/// Display name for this tier (e.g., "Bronze", "Silver", "Gold")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	FText TierName;

	/// Progress value required to unlock this tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	int32 RequiredProgress = 1;

	/// Rewards granted when this tier is reached
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	FMGAchievementReward Reward;
};

/**
 * @brief Complete definition of an achievement.
 *
 * This structure contains all static data about an achievement, including
 * its display information, unlock conditions, and rewards. Achievement
 * definitions are typically loaded from data assets at startup.
 */
USTRUCT(BlueprintType)
struct FMGAchievementDefinition
{
	GENERATED_BODY()

	/// Unique identifier for this achievement (used for lookups and save data)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName AchievementID;

	/// Localized name shown to players
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText DisplayName;

	/// Localized description of how to unlock (shown after unlock or if not secret)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText Description;

	/// Description shown for secret achievements before they are unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText HiddenDescription;

	/// Category for UI organization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EMGAchievementCategory Category = EMGAchievementCategory::Racing;

	/// Rarity tier affecting visual presentation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EMGAchievementRarity Rarity = EMGAchievementRarity::Common;

	/// Which stat type triggers progress for this achievement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EMGAchievementStatType StatType = EMGAchievementStatType::RacesCompleted;

	/// Target progress value to unlock (ignored if bIsProgressive is true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	int32 TargetProgress = 1;

	/// If true, achievement details are hidden until unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsSecret = false;

	/// If true, this achievement has multiple tiers with separate rewards
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsProgressive = false;

	/// Tier definitions for progressive achievements (empty if not progressive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	TArray<FMGAchievementTier> Tiers;

	/// Rewards for non-progressive achievements (or final tier bonus)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FMGAchievementReward Reward;

	/// Icon displayed when achievement is unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	UTexture2D* Icon = nullptr;

	/// Icon displayed when achievement is still locked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	UTexture2D* LockedIcon = nullptr;

	/// Platform-specific achievement ID for Steam/Xbox/PlayStation integration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName PlatformAchievementID;

	/// IDs of achievements that must be unlocked before this one becomes available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	TArray<FName> PrerequisiteAchievements;

	/// Required track ID (for track-specific achievements, NAME_None for any track)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName RequiredTrack;

	/// Required vehicle ID (for vehicle-specific achievements, NAME_None for any vehicle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName RequiredVehicle;
};

/**
 * @brief Runtime progress data for a single achievement.
 *
 * This structure tracks the player's current progress toward unlocking
 * an achievement. It is saved/loaded with player data.
 */
USTRUCT(BlueprintType)
struct FMGAchievementProgress
{
	GENERATED_BODY()

	/// ID of the achievement this progress relates to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FName AchievementID;

	/// Current progress value (compared against TargetProgress or tier thresholds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentProgress = 0;

	/// For progressive achievements: highest tier unlocked so far (0 = none)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentTier = 0;

	/// Whether the achievement (or all tiers) is fully unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bIsUnlocked = false;

	/// Timestamp when the achievement was first unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FDateTime UnlockTime;

	/// Whether the reward for this achievement has been claimed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bRewardClaimed = false;
};

// ============================================================================
// STRUCTURES - Badges and Titles
// ============================================================================

/**
 * @brief A displayable badge earned through achievements or gameplay.
 *
 * Badges are visual icons players can equip to their profile.
 * Players typically have limited badge slots (see MaxBadgeSlots).
 */
USTRUCT(BlueprintType)
struct FMGBadge
{
	GENERATED_BODY()

	/// Unique identifier for this badge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	FName BadgeID;

	/// Localized display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	FText DisplayName;

	/// Localized description of how the badge was earned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	FText Description;

	/// Visual icon for the badge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	UTexture2D* Icon = nullptr;

	/// Rarity tier for visual effects and sorting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	EMGAchievementRarity Rarity = EMGAchievementRarity::Common;

	/// Whether this badge is currently equipped in a slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	bool bIsEquipped = false;

	/// When this badge was unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	FDateTime UnlockTime;
};

// FMGPlayerTitle - defined in PlayerTitle/MGPlayerTitleSubsystem.h

// ============================================================================
// STRUCTURES - Milestones
// ============================================================================

/**
 * @brief Cumulative stat milestones with multiple reward thresholds.
 *
 * Milestones track lifetime stats and grant rewards at specific thresholds.
 * Unlike progressive achievements, milestones continue tracking beyond
 * the final threshold for display purposes.
 *
 * Example: "Total Distance Driven" with thresholds at 100km, 500km, 1000km, etc.
 */
USTRUCT(BlueprintType)
struct FMGMilestone
{
	GENERATED_BODY()

	/// Unique identifier for this milestone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FName MilestoneID;

	/// Localized display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FText DisplayName;

	/// Localized description of what this milestone tracks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FText Description;

	/// Which stat type this milestone tracks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	EMGAchievementStatType StatType;

	/// Threshold values where rewards are granted (e.g., [100, 500, 1000, 5000])
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	TArray<int32> Thresholds;

	/// Index of the last claimed threshold (0 = first threshold not yet claimed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int32 CurrentThresholdIndex = 0;

	/// Rewards for each threshold (array length should match Thresholds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	TArray<FMGAchievementReward> ThresholdRewards;
};

// ============================================================================
// STRUCTURES - Statistics Summary
// ============================================================================

/**
 * @brief Summary statistics for the achievement system.
 *
 * This structure provides an overview of the player's achievement progress,
 * useful for displaying completion percentages and overall stats.
 */
USTRUCT(BlueprintType)
struct FMGAchievementStats
{
	GENERATED_BODY()

	/// Total number of achievements in the game
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalAchievements = 0;

	/// Number of achievements the player has unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 UnlockedAchievements = 0;

	/// Maximum possible gamerscore across all achievements
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalGamerScore = 0;

	/// Gamerscore the player has earned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 EarnedGamerScore = 0;

	/// Percentage of achievements completed (0.0 to 100.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CompletionPercentage = 0.0f;

	/// Number of badges the player has unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 BadgesUnlocked = 0;

	/// Number of titles the player has unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TitlesUnlocked = 0;

	/// Timestamp of the most recent achievement unlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FDateTime LastAchievementUnlock;

	/// ID of the rarest achievement the player has unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FName RarestAchievement;
};

// ============================================================================
// DELEGATES - Event Callbacks
// ============================================================================

/// Broadcast when an achievement is unlocked. TierUnlocked is 0 for non-progressive achievements.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementUnlocked, const FMGAchievementDefinition&, Achievement, int32, TierUnlocked);

/// Broadcast when progress is made toward an achievement. ProgressPercent is 0.0 to 1.0.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementProgress, FName, AchievementID, float, ProgressPercent);

/// Broadcast when a new badge is unlocked and available to equip.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBadgeUnlocked, const FMGBadge&, Badge);

/// Broadcast when a new title is unlocked and available to equip.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleUnlocked, const FMGPlayerTitle&, Title);

/// Broadcast when a milestone threshold is reached. ThresholdIndex is 0-based.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMilestoneReached, const FMGMilestone&, Milestone, int32, ThresholdIndex);

// ============================================================================
// SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem for achievement, badge, title, and milestone management.
 *
 * UMGAchievementSubsystem is a GameInstanceSubsystem that provides comprehensive
 * achievement tracking functionality. It integrates with the game's stat system
 * to automatically track progress and unlock achievements when conditions are met.
 *
 * ## Key Responsibilities
 * - Loading and storing achievement definitions
 * - Tracking progress toward all achievements
 * - Unlocking achievements and granting rewards
 * - Managing badge equipment slots
 * - Managing equipped player title
 * - Tracking milestones across sessions
 * - Synchronizing with platform achievement systems (Steam, Xbox, PlayStation)
 *
 * ## Integration Points
 * - Call ReportStatIncrement() when relevant game events occur
 * - Call ReportRaceCompletion() after each race for comprehensive stat updates
 * - Listen to OnAchievementUnlocked to trigger unlock animations/sounds
 *
 * @see FMGAchievementDefinition for achievement data structure
 * @see FMGAchievementProgress for player progress tracking
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Initialize the subsystem, load definitions and progress
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Clean up and save progress before shutdown
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS - Subscribe to track unlock notifications
	// ==========================================

	/// Fired when any achievement is unlocked (check TierUnlocked for progressive achievements)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementUnlocked OnAchievementUnlocked;

	/// Fired when progress is made toward any achievement (useful for progress UI)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementProgress OnAchievementProgress;

	/// Fired when a new badge becomes available to equip
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBadgeUnlocked OnBadgeUnlocked;

	/// Fired when a new title becomes available to equip
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTitleUnlocked OnTitleUnlocked;

	/// Fired when a milestone threshold is crossed
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMilestoneReached OnMilestoneReached;

	// ==========================================
	// STAT REPORTING - Call these to update progress
	// ==========================================

	/**
	 * @brief Report an incremental stat increase.
	 * @param StatType The type of stat that increased
	 * @param Amount How much the stat increased (default 1)
	 *
	 * Call this when a countable event occurs (race completed, item collected, etc.).
	 * The subsystem will check all achievements linked to this stat type.
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ReportStatIncrement(EMGAchievementStatType StatType, int32 Amount = 1);

	/**
	 * @brief Report a stat value for max-tracking stats.
	 * @param StatType The type of stat (typically FastestLap)
	 * @param Value The new value to compare against existing best
	 *
	 * Use for stats where we track the best value, not cumulative total.
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ReportStatValue(EMGAchievementStatType StatType, int32 Value);

	/**
	 * @brief Report progress for a specific custom achievement.
	 * @param AchievementID The achievement to update
	 * @param Progress The progress amount to add
	 *
	 * Use for achievements with StatType::Custom that have unique unlock logic.
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ReportCustomProgress(FName AchievementID, int32 Progress);

	/**
	 * @brief Comprehensive race completion report updating multiple stats at once.
	 * @param Position Finishing position (1 = first place)
	 * @param bIsOnline Whether this was an online race
	 * @param bIsPerfect Whether the race was completed without incidents
	 * @param TrackID The track where the race occurred
	 * @param VehicleID The vehicle used in the race
	 *
	 * This is the preferred method for reporting race results as it updates
	 * all relevant stats in a single call.
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ReportRaceCompletion(int32 Position, bool bIsOnline, bool bIsPerfect, FName TrackID, FName VehicleID);

	// ==========================================
	// ACHIEVEMENT QUERIES - Read achievement data
	// ==========================================

	/**
	 * @brief Get all achievement definitions.
	 * @return Array of all achievements in the game
	 */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetAllAchievements() const;

	/**
	 * @brief Get achievements filtered by category.
	 * @param Category The category to filter by
	 * @return Array of achievements in the specified category
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetAchievementsByCategory(EMGAchievementCategory Category) const;

	/**
	 * @brief Look up a specific achievement definition.
	 * @param AchievementID The achievement to find
	 * @param OutAchievement Output parameter filled with achievement data
	 * @return True if the achievement was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	bool GetAchievement(FName AchievementID, FMGAchievementDefinition& OutAchievement) const;

	/**
	 * @brief Get the player's progress for a specific achievement.
	 * @param AchievementID The achievement to check
	 * @return Current progress data (default values if not found)
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	FMGAchievementProgress GetAchievementProgress(FName AchievementID) const;

	/**
	 * @brief Check if an achievement is unlocked.
	 * @param AchievementID The achievement to check
	 * @return True if fully unlocked (all tiers for progressive)
	 */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	bool IsAchievementUnlocked(FName AchievementID) const;

	/**
	 * @brief Get all unlocked achievements.
	 * @return Array of achievement definitions the player has unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetUnlockedAchievements() const;

	/**
	 * @brief Get all locked achievements.
	 * @return Array of achievement definitions not yet unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetLockedAchievements() const;

	/**
	 * @brief Get recently unlocked achievements for UI display.
	 * @param Count Maximum number to return
	 * @return Array sorted by unlock time (most recent first)
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetRecentlyUnlocked(int32 Count = 5) const;

	/**
	 * @brief Get achievements closest to being unlocked.
	 * @param Count Maximum number to return
	 * @return Array sorted by completion percentage (highest first)
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetNearestToCompletion(int32 Count = 5) const;

	// ==========================================
	// BADGES - Equipment and display
	// ==========================================

	/**
	 * @brief Get all badges the player has unlocked.
	 * @return Array of unlocked badges
	 */
	UFUNCTION(BlueprintPure, Category = "Badges")
	TArray<FMGBadge> GetUnlockedBadges() const;

	/**
	 * @brief Get currently equipped badges.
	 * @return Array of equipped badges (up to MaxBadgeSlots)
	 */
	UFUNCTION(BlueprintPure, Category = "Badges")
	TArray<FMGBadge> GetEquippedBadges() const;

	/**
	 * @brief Equip a badge to a specific slot.
	 * @param BadgeID The badge to equip
	 * @param SlotIndex Which slot to equip to (0 to MaxBadgeSlots-1)
	 * @return True if successfully equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Badges")
	bool EquipBadge(FName BadgeID, int32 SlotIndex);

	/**
	 * @brief Remove a badge from a slot.
	 * @param SlotIndex The slot to clear
	 */
	UFUNCTION(BlueprintCallable, Category = "Badges")
	void UnequipBadge(int32 SlotIndex);

	/**
	 * @brief Get the number of available badge slots.
	 * @return Maximum number of badges that can be equipped simultaneously
	 */
	UFUNCTION(BlueprintPure, Category = "Badges")
	int32 GetMaxBadgeSlots() const { return MaxBadgeSlots; }

	// ==========================================
	// TITLES - Player name decoration
	// ==========================================

	/**
	 * @brief Get all titles the player has unlocked.
	 * @return Array of unlocked titles
	 */
	UFUNCTION(BlueprintPure, Category = "Titles")
	TArray<FMGPlayerTitle> GetUnlockedTitles() const;

	/**
	 * @brief Get the currently equipped title.
	 * @return The active title (or default if none equipped)
	 */
	UFUNCTION(BlueprintPure, Category = "Titles")
	FMGPlayerTitle GetEquippedTitle() const;

	/**
	 * @brief Equip a title.
	 * @param TitleID The title to equip (replaces current title)
	 * @return True if successfully equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles")
	bool EquipTitle(FName TitleID);

	// ==========================================
	// MILESTONES - Long-term stat tracking
	// ==========================================

	/**
	 * @brief Get all milestone definitions.
	 * @return Array of all milestones in the game
	 */
	UFUNCTION(BlueprintPure, Category = "Milestones")
	TArray<FMGMilestone> GetAllMilestones() const { return Milestones; }

	/**
	 * @brief Get progress toward the next milestone threshold.
	 * @param MilestoneID The milestone to check
	 * @return Progress percentage (0.0 to 1.0) toward next threshold
	 */
	UFUNCTION(BlueprintCallable, Category = "Milestones")
	float GetMilestoneProgress(FName MilestoneID) const;

	// ==========================================
	// STATS - Overall progress summary
	// ==========================================

	/**
	 * @brief Get a summary of achievement progress.
	 * @return Statistics structure with completion data
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	FMGAchievementStats GetAchievementStats() const;

	/**
	 * @brief Get the current value of a tracked stat.
	 * @param StatType The stat to query
	 * @return Current cumulative value of the stat
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetStatValue(EMGAchievementStatType StatType) const;

	// ==========================================
	// REWARDS - Reward collection
	// ==========================================

	/**
	 * @brief Claim the reward for an unlocked achievement.
	 * @param AchievementID The achievement whose reward to claim
	 * @return True if reward was successfully claimed
	 *
	 * Rewards are not automatically granted on unlock to allow for
	 * celebration UI. Call this when the player dismisses the unlock popup.
	 */
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	bool ClaimAchievementReward(FName AchievementID);

	/**
	 * @brief Check if any unclaimed rewards exist.
	 * @return True if there are rewards to claim
	 */
	UFUNCTION(BlueprintPure, Category = "Rewards")
	bool HasUnclaimedRewards() const;

	/**
	 * @brief Get the count of unclaimed rewards.
	 * @return Number of achievements with unclaimed rewards
	 */
	UFUNCTION(BlueprintPure, Category = "Rewards")
	int32 GetUnclaimedRewardCount() const;

	// ==========================================
	// PLATFORM INTEGRATION - External achievement systems
	// ==========================================

	/**
	 * @brief Synchronize local achievements with platform services.
	 *
	 * Call this at startup and periodically to ensure platform achievements
	 * (Steam, Xbox, PlayStation) are in sync with local progress.
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform")
	void SyncWithPlatform();

	/**
	 * @brief Check if a platform achievement is unlocked.
	 * @param PlatformAchievementID Platform-specific achievement ID
	 * @return True if unlocked on the platform
	 */
	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsPlatformAchievementUnlocked(FName PlatformAchievementID) const;

protected:
	// ==========================================
	// DATA STORAGE
	// ==========================================

	/// All achievement definitions loaded from data assets
	UPROPERTY()
	TArray<FMGAchievementDefinition> AchievementDefinitions;

	/// Player progress for each achievement, keyed by AchievementID
	UPROPERTY()
	TMap<FName, FMGAchievementProgress> AchievementProgressMap;

	/// Current values for all tracked stats
	UPROPERTY()
	TMap<EMGAchievementStatType, int32> StatValues;

	/// Badges the player has unlocked
	UPROPERTY()
	TArray<FMGBadge> UnlockedBadges;

	/// Badge IDs currently equipped in each slot
	UPROPERTY()
	TArray<FName> EquippedBadgeSlots;

	/// Titles the player has unlocked
	UPROPERTY()
	TArray<FMGPlayerTitle> UnlockedTitles;

	/// Currently equipped title ID
	UPROPERTY()
	FName EquippedTitleID;

	/// All milestone definitions and progress
	UPROPERTY()
	TArray<FMGMilestone> Milestones;

	/// Maximum number of badge slots available
	UPROPERTY()
	int32 MaxBadgeSlots = 3;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/// Load achievement definitions from data assets
	void InitializeAchievements();

	/// Load milestone definitions from data assets
	void InitializeMilestones();

	/// Check if any achievements should unlock based on a stat update
	void CheckAchievementUnlocks(EMGAchievementStatType StatType);

	/// Internal method to unlock an achievement and broadcast events
	void UnlockAchievement(FName AchievementID, int32 Tier = 0);

	/// Internal method to unlock a badge
	void UnlockBadge(FName BadgeID);

	/// Internal method to unlock a title
	void UnlockTitle(FName TitleID);

	/// Check if any milestones have been crossed
	void CheckMilestoneProgress(EMGAchievementStatType StatType);

	/// Report an achievement unlock to the platform service
	void ReportToPlatform(FName AchievementID);

	/// Grant rewards from a reward structure to the player
	void ApplyRewards(const FMGAchievementReward& Reward);

	/// Load saved progress from disk
	void LoadProgress();

	/// Save current progress to disk
	void SaveProgress();
};

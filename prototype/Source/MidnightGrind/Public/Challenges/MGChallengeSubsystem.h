/**
 * @file MGChallengeSubsystem.h
 * @brief Time-Limited Challenge and Objective Tracking Subsystem
 *
 * This subsystem manages all challenge-related functionality in Midnight Grind, including:
 * - Daily, Weekly, and Monthly rotating challenges with automatic refresh
 * - Seasonal limited-time challenges tied to live events
 * - Community-wide collaborative challenges with shared progress
 * - Challenge sets (grouped challenges with completion bonuses)
 * - Tutorial challenges for onboarding new players
 *
 * Unlike achievements (permanent goals), challenges are time-sensitive objectives that
 * reset or expire. Players can have a limited number of active challenges at once.
 *
 * @section lifecycle Challenge Lifecycle
 * 1. Locked - Prerequisites not met or not yet available
 * 2. Available - Can be activated by the player
 * 3. Active - Currently being tracked for progress
 * 4. Completed - All objectives met, rewards available
 * 5. Claimed - Rewards collected (for repeatable challenges, resets to Available)
 * 6. Expired/Failed - Time ran out before completion
 *
 * @section integration Integration Points
 * - Call TrackStat() when game events occur (drift distance, races won, etc.)
 * - Subscribe to OnChallengeCompleted to show completion UI
 * - Call RefreshChallenges() to force a challenge pool update
 *
 * @note This is a GameInstanceSubsystem, persisting across level loads.
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGChallengeSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS - Challenge Classification
// ============================================================================

/**
 * @brief Type classification for challenge scheduling and reset behavior.
 *
 * Determines when challenges refresh and how long they remain available.
 */
UENUM(BlueprintType)
enum class EMGChallengeType : uint8
{
	Daily,       ///< Refreshes every 24 hours at server reset time
	Weekly,      ///< Refreshes every 7 days (typically Monday)
	Monthly,     ///< Refreshes on the 1st of each month
	Seasonal,    ///< Available during specific seasonal events (limited time)
	Community,   ///< Server-wide collaborative challenges with shared progress
	Special,     ///< One-time promotional or holiday challenges
	Achievement, ///< Permanent challenges tied to achievement system
	Tutorial     ///< Onboarding challenges for new players
};

/**
 * @brief Category for organizing challenges in the UI by gameplay type.
 */
UENUM(BlueprintType)
enum class EMGChallengeCategory : uint8
{
	Racing,        ///< Win races, finish positions, race completions
	Drifting,      ///< Drift distance, drift combos, drift scores
	Collection,    ///< Collect items, earn currency, acquire vehicles
	Exploration,   ///< Visit locations, discover secrets, complete routes
	Social,        ///< Crew activities, multiplayer races, photo sharing
	Customization, ///< Create liveries, modify vehicles, equip items
	Mastery,       ///< Perfect runs, high scores, skill-based goals
	Speed,         ///< Top speed, speed traps, time trials
	Endurance,     ///< Long races, total distance, play time
	Skill          ///< Near misses, overtakes, technical maneuvers
};

/**
 * @brief Current state of a challenge in its lifecycle.
 */
UENUM(BlueprintType)
enum class EMGChallengeState : uint8
{
	Locked,    ///< Cannot be started (prerequisites not met or not yet available)
	Available, ///< Can be activated but not currently tracked
	Active,    ///< Currently tracking progress toward objectives
	Completed, ///< All objectives met, awaiting reward claim
	Failed,    ///< Failed to complete before time limit (for fail-able challenges)
	Expired,   ///< Challenge period ended before completion
	Claimed    ///< Rewards collected (may reset to Available if repeatable)
};

/**
 * @brief Difficulty rating affecting rewards and complexity.
 */
UENUM(BlueprintType)
enum class EMGChallengeDifficulty : uint8
{
	Easy,      ///< Quick to complete, lower rewards, good for casual play
	Medium,    ///< Moderate effort required, standard rewards
	Hard,      ///< Requires significant skill or time investment
	Expert,    ///< Challenging objectives for experienced players
	Legendary  ///< Extremely difficult, highest rewards, prestige value
};

// ============================================================================
// STRUCTURES - Reward and Objective Data
// ============================================================================

/**
 * @brief Rewards granted upon challenge completion.
 *
 * Challenges can grant multiple reward types. Set unused fields to 0 or NAME_None.
 */
USTRUCT(BlueprintType)
struct FMGChallengeReward
{
	GENERATED_BODY()

	/// Unique identifier for this reward (for tracking/serialization)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	/// Localized display name for reward UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RewardName;

	/// Standard in-game currency (credits/cash) amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyAmount = 0;

	/// Premium currency amount (for special rewards)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PremiumCurrencyAmount = 0;

	/// Player experience points
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExperienceAmount = 0;

	/// Season pass experience points (for battle pass progression)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonXPAmount = 0;

	/// ID of a specific item to unlock (NAME_None if no item)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	/// Icon texture for displaying in reward previews
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> RewardIcon;
};

/**
 * @brief A single trackable objective within a challenge.
 *
 * Challenges can have multiple objectives, all of which must be completed
 * unless marked as optional. Optional objectives grant bonus rewards.
 */
USTRUCT(BlueprintType)
struct FMGChallengeObjective
{
	GENERATED_BODY()

	/// Unique identifier for this objective within the challenge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	/// Localized description shown to the player (e.g., "Win 3 races")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Target value to reach for completion (e.g., 3 for "Win 3 races")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetValue = 1;

	/// Current progress toward the target (updated via TrackStat or UpdateProgress)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentValue = 0;

	/// Whether this specific objective is complete
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	/// Name of the stat to automatically track (links to TrackStat system)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StatToTrack;

	/// If true, this objective is not required for challenge completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOptional = false;

	/// Bonus reward amount for completing this optional objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusRewardAmount = 0;
};

/**
 * @brief Complete definition of a challenge.
 *
 * Contains all static data about a challenge plus runtime state for tracking.
 */
USTRUCT(BlueprintType)
struct FMGChallenge
{
	GENERATED_BODY()

	/// Unique identifier for this challenge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	/// Localized title displayed in the UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/// Localized description explaining the challenge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Type determining refresh schedule
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeType ChallengeType = EMGChallengeType::Daily;

	/// Category for UI organization
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeCategory Category = EMGChallengeCategory::Racing;

	/// Current lifecycle state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeState State = EMGChallengeState::Available;

	/// Difficulty rating affecting rewards and UI presentation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeDifficulty Difficulty = EMGChallengeDifficulty::Easy;

	/// List of objectives that must be completed (or are optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeObjective> Objectives;

	/// Rewards granted upon completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeReward> Rewards;

	/// When this challenge becomes available (for timed challenges)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	/// When this challenge expires (must complete before this time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	/// Display order in UI lists (lower = higher priority)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;

	/// Whether this challenge can be completed multiple times
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRepeatable = false;

	/// Number of times the player has completed this challenge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesCompleted = 0;

	/// Maximum completions allowed (0 = unlimited for repeatable challenges)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCompletions = 1;

	/// IDs of challenges that must be completed before this one unlocks
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> PrerequisiteChallenges;

	/// Minimum player level required to access this challenge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 0;

	/// Icon displayed in the challenge UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ChallengeIcon;

	/// If set, challenge must be completed on this specific track
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrack;

	/// If set, challenge must be completed with this specific vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicle;
};

// ============================================================================
// STRUCTURES - Challenge Sets and Community Challenges
// ============================================================================

/**
 * @brief A group of related challenges with a completion bonus.
 *
 * Challenge sets encourage players to complete multiple related challenges
 * by offering additional rewards when a threshold is reached.
 */
USTRUCT(BlueprintType)
struct FMGChallengeSet
{
	GENERATED_BODY()

	/// Unique identifier for this set
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SetID;

	/// Localized name for the set (e.g., "Drift Master Collection")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SetName;

	/// Challenges included in this set
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallenge> Challenges;

	/// Bonus reward for completing the required number of challenges
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGChallengeReward CompletionReward;

	/// How many challenges must be completed to earn the set bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredCompletions = 0;

	/// How many challenges the player has completed in this set
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCompletions = 0;
};

/**
 * @brief Server-wide challenge with shared progress across all players.
 *
 * Community challenges aggregate contributions from all players toward
 * a massive goal, with milestone rewards along the way.
 */
USTRUCT(BlueprintType)
struct FMGCommunityChallenge
{
	GENERATED_BODY()

	/// Base challenge data (objectives, rewards, timing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGChallenge Challenge;

	/// Community-wide target value (often very large, e.g., 1 billion)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CommunityTarget = 0;

	/// Current community-wide progress (sum of all player contributions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CommunityProgress = 0;

	/// Number of players who have contributed to this challenge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalContributors = 0;

	/// This player's personal contribution amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 PlayerContribution = 0;

	/// Rewards unlocked at each milestone threshold
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeReward> MilestoneRewards;

	/// Progress values where milestones are unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int64> MilestoneThresholds;
};

/**
 * @brief Runtime progress tracking for an active challenge.
 *
 * Stored separately from challenge definitions to track player-specific state.
 */
USTRUCT(BlueprintType)
struct FMGChallengeProgress
{
	GENERATED_BODY()

	/// ID of the challenge this progress belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	/// Progress for each objective, keyed by ObjectiveID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> ObjectiveProgress;

	/// When the player started this challenge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartedAt;

	/// When the player completed this challenge (invalid if not completed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedAt;

	/// Whether rewards have been claimed for this completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRewardsClaimed = false;
};

// ============================================================================
// DELEGATES - Event Callbacks
// ============================================================================

/// Fired when progress is made on any active challenge. ProgressPercent is 0.0 to 1.0.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChallengeProgressUpdated, FName, ChallengeID, float, ProgressPercent);

/// Fired when a challenge is fully completed (all required objectives met).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeCompleted, const FMGChallenge&, Challenge);

/// Fired when rewards are claimed from a completed challenge.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeRewardsClaimed, const TArray<FMGChallengeReward>&, Rewards);

/// Fired when new challenges become available (e.g., daily reset).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewChallengesAvailable, EMGChallengeType, ChallengeType);

/// Fired when a timed challenge expires before completion.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeExpired, FName, ChallengeID);

/// Fired when community challenge progress updates (from server sync).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunityProgressUpdated, FName, ChallengeID, float, CommunityPercent);

/// Fired when all required challenges in a set are completed.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeSetCompleted, const FMGChallengeSet&, Set);

// ============================================================================
// SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem for time-limited challenge management.
 *
 * UMGChallengeSubsystem handles all aspects of the challenge system including
 * daily/weekly/monthly rotating challenges, community events, and challenge sets.
 *
 * ## Key Features
 * - Automatic daily/weekly/monthly challenge rotation
 * - Challenge activation limits (can only track N challenges at once)
 * - Daily reroll system for unwanted challenges
 * - Community challenge progress synchronization
 * - Challenge set completion tracking
 *
 * ## Typical Usage Flow
 * 1. Player views available challenges via GetAvailableChallenges()
 * 2. Player activates a challenge via ActivateChallenge()
 * 3. Game calls TrackStat() as player performs actions
 * 4. Subsystem updates progress and fires OnChallengeProgressUpdated
 * 5. When complete, OnChallengeCompleted fires
 * 6. Player claims rewards via ClaimChallengeRewards()
 *
 * @see FMGChallenge for challenge data structure
 * @see FMGChallengeProgress for progress tracking
 */
UCLASS()
class MIDNIGHTGRIND_API UMGChallengeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Initialize subsystem, load challenge data, and start tick timer
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Clean up timers and save progress
	virtual void Deinitialize() override;

	/// Determine if subsystem should be created (can be disabled in config)
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// CHALLENGE RETRIEVAL - Query challenge data
	// ==========================================

	/**
	 * @brief Get all challenges of a specific type.
	 * @param Type The challenge type to filter by
	 * @return Array of challenges matching the type
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetChallengesByType(EMGChallengeType Type) const;

	/**
	 * @brief Get all challenges in a specific category.
	 * @param Category The category to filter by
	 * @return Array of challenges in the category
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetChallengesByCategory(EMGChallengeCategory Category) const;

	/**
	 * @brief Get all currently active (tracking) challenges.
	 * @return Array of challenges in the Active state
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetActiveChallenges() const;

	/**
	 * @brief Get all completed challenges awaiting reward claim.
	 * @return Array of challenges in the Completed state
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetCompletedChallenges() const;

	/**
	 * @brief Get all challenges available for activation.
	 * @return Array of challenges in the Available state
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetAvailableChallenges() const;

	/**
	 * @brief Look up a specific challenge by ID.
	 * @param ChallengeID The challenge to find
	 * @return The challenge data (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	FMGChallenge GetChallenge(FName ChallengeID) const;

	/**
	 * @brief Check if a challenge can be activated.
	 * @param ChallengeID The challenge to check
	 * @return True if the challenge is available and can be started
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	bool IsChallengeAvailable(FName ChallengeID) const;

	// ==========================================
	// TIMED CHALLENGES - Daily/Weekly/Monthly queries
	// ==========================================

	/**
	 * @brief Get today's daily challenges.
	 * @return Array of daily challenges (refreshes at server reset)
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	TArray<FMGChallenge> GetDailyChallenges() const;

	/**
	 * @brief Get this week's weekly challenges.
	 * @return Array of weekly challenges (refreshes weekly)
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	TArray<FMGChallenge> GetWeeklyChallenges() const;

	/**
	 * @brief Get this month's monthly challenges.
	 * @return Array of monthly challenges (refreshes on the 1st)
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	TArray<FMGChallenge> GetMonthlyChallenges() const;

	/**
	 * @brief Get time remaining until daily challenge reset.
	 * @return Time span until next daily reset
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	FTimespan GetTimeUntilDailyReset() const;

	/**
	 * @brief Get time remaining until weekly challenge reset.
	 * @return Time span until next weekly reset
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	FTimespan GetTimeUntilWeeklyReset() const;

	// ==========================================
	// PROGRESS TRACKING - Update challenge progress
	// ==========================================

	/**
	 * @brief Set absolute progress for a specific objective.
	 * @param ChallengeID The challenge to update
	 * @param ObjectiveID The specific objective within the challenge
	 * @param Progress The new progress value (replaces current)
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Progress")
	void UpdateChallengeProgress(FName ChallengeID, FName ObjectiveID, int32 Progress);

	/**
	 * @brief Add incremental progress to a specific objective.
	 * @param ChallengeID The challenge to update
	 * @param ObjectiveID The specific objective within the challenge
	 * @param Amount How much to add to current progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Progress")
	void AddChallengeProgress(FName ChallengeID, FName ObjectiveID, int32 Amount);

	/**
	 * @brief Report a stat value to automatically update matching objectives.
	 * @param StatName The stat name (matches objective StatToTrack fields)
	 * @param Value The stat value to add
	 *
	 * This is the preferred method for progress updates. The subsystem
	 * will find all active objectives tracking this stat and update them.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Progress")
	void TrackStat(FName StatName, int32 Value);

	/**
	 * @brief Get overall completion percentage for a challenge.
	 * @param ChallengeID The challenge to check
	 * @return Completion percentage (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Progress")
	float GetChallengeProgressPercent(FName ChallengeID) const;

	/**
	 * @brief Get detailed progress data for a challenge.
	 * @param ChallengeID The challenge to check
	 * @return Progress structure with per-objective data
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Progress")
	FMGChallengeProgress GetChallengeProgress(FName ChallengeID) const;

	// ==========================================
	// ACTIVATION - Start and stop tracking
	// ==========================================

	/**
	 * @brief Activate a challenge to begin tracking progress.
	 * @param ChallengeID The challenge to activate
	 * @return True if successfully activated
	 *
	 * May fail if the player has reached MaxActiveChallenges or the
	 * challenge is not in the Available state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Activation")
	bool ActivateChallenge(FName ChallengeID);

	/**
	 * @brief Deactivate a challenge and stop tracking.
	 * @param ChallengeID The challenge to deactivate
	 *
	 * Progress is preserved; the challenge returns to Available state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Activation")
	void DeactivateChallenge(FName ChallengeID);

	/**
	 * @brief Get number of currently active challenges.
	 * @return Count of challenges in Active state
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Activation")
	int32 GetActiveChallengeCount() const;

	/**
	 * @brief Get maximum allowed active challenges.
	 * @return Limit on simultaneous active challenges
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Activation")
	int32 GetMaxActiveChallenges() const { return MaxActiveChallenges; }

	// ==========================================
	// REWARDS - Claim completion rewards
	// ==========================================

	/**
	 * @brief Claim rewards for a completed challenge.
	 * @param ChallengeID The completed challenge
	 * @return True if rewards were successfully claimed
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Rewards")
	bool ClaimChallengeRewards(FName ChallengeID);

	/**
	 * @brief Claim all available rewards from completed challenges.
	 *
	 * Useful for a "Claim All" button in the UI.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Rewards")
	void ClaimAllAvailableRewards();

	/**
	 * @brief Get completed challenges with unclaimed rewards.
	 * @return Array of challenges awaiting reward claim
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Rewards")
	TArray<FMGChallenge> GetUnclaimedCompletedChallenges() const;

	/**
	 * @brief Check if any completed challenges have unclaimed rewards.
	 * @return True if there are rewards to claim
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Rewards")
	bool HasUnclaimedRewards() const;

	// ==========================================
	// CHALLENGE SETS - Grouped challenge bonuses
	// ==========================================

	/**
	 * @brief Get all challenge sets.
	 * @return Array of all defined challenge sets
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Sets")
	TArray<FMGChallengeSet> GetChallengeSets() const;

	/**
	 * @brief Get a specific challenge set by ID.
	 * @param SetID The set to find
	 * @return The challenge set data
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Sets")
	FMGChallengeSet GetChallengeSet(FName SetID) const;

	/**
	 * @brief Get completion progress for a challenge set.
	 * @param SetID The set to check
	 * @return Completion percentage (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Sets")
	float GetChallengeSetProgress(FName SetID) const;

	// ==========================================
	// COMMUNITY CHALLENGES - Server-wide goals
	// ==========================================

	/**
	 * @brief Get all active community challenges.
	 * @return Array of community challenges with shared progress
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Community")
	TArray<FMGCommunityChallenge> GetCommunityChallenges() const;

	/**
	 * @brief Contribute to a community challenge.
	 * @param ChallengeID The community challenge
	 * @param Amount The contribution amount
	 *
	 * Contribution is sent to the server and aggregated with other players.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Community")
	void ContributeToCommunityChallenge(FName ChallengeID, int64 Amount);

	/**
	 * @brief Get community-wide progress percentage.
	 * @param ChallengeID The community challenge
	 * @return Completion percentage (0.0 to 1.0) of community goal
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Community")
	float GetCommunityProgressPercent(FName ChallengeID) const;

	// ==========================================
	// MANAGEMENT - System operations
	// ==========================================

	/**
	 * @brief Force refresh of challenge pools.
	 *
	 * Normally called automatically on timer. Can be called manually
	 * to sync with server or after time zone changes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Management")
	void RefreshChallenges();

	/**
	 * @brief Reroll a daily challenge for a different one.
	 * @param ChallengeID The daily challenge to replace
	 *
	 * Uses one daily reroll. Call GetRerollsRemaining() to check availability.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Management")
	void RerollDailyChallenge(FName ChallengeID);

	/**
	 * @brief Get remaining daily rerolls.
	 * @return Number of rerolls available today
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges|Management")
	int32 GetRerollsRemaining() const { return DailyRerollsRemaining; }

	/**
	 * @brief Save all challenge progress to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Management")
	void SaveChallengeProgress();

	/**
	 * @brief Load challenge progress from disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges|Management")
	void LoadChallengeProgress();

	// ==========================================
	// EVENTS - Subscribable delegates
	// ==========================================

	/// Fired when any active challenge makes progress
	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeProgressUpdated OnChallengeProgressUpdated;

	/// Fired when a challenge is fully completed
	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeCompleted OnChallengeCompleted;

	/// Fired when rewards are claimed
	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeRewardsClaimed OnChallengeRewardsClaimed;

	/// Fired when new challenges become available (daily/weekly reset)
	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnNewChallengesAvailable OnNewChallengesAvailable;

	/// Fired when a timed challenge expires
	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeExpired OnChallengeExpired;

	/// Fired when community challenge progress updates from server
	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnCommunityProgressUpdated OnCommunityProgressUpdated;

	/// Fired when a challenge set is fully completed
	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeSetCompleted OnChallengeSetCompleted;

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/// Called on timer to check for expired/reset challenges
	void OnChallengeTick();

	/// Check and handle any expired challenges
	void CheckForExpiredChallenges();

	/// Check if any active challenges are now complete
	void CheckForCompletedChallenges();

	/// Generate new daily challenges (called at daily reset)
	void GenerateDailyChallenges();

	/// Generate new weekly challenges (called at weekly reset)
	void GenerateWeeklyChallenges();

	/// Load default challenge definitions
	void InitializeDefaultChallenges();

	/// Apply a reward to the player (currency, items, etc.)
	void GrantReward(const FMGChallengeReward& Reward);

	/// Update challenge state based on objectives
	void UpdateChallengeState(FMGChallenge& Challenge);

	/// Check if prerequisite challenges are met
	bool ArePrerequisitesMet(const FMGChallenge& Challenge) const;

	// ==========================================
	// DATA STORAGE
	// ==========================================

	/// All challenge definitions, keyed by ChallengeID
	UPROPERTY()
	TMap<FName, FMGChallenge> AllChallenges;

	/// Player progress for each challenge
	UPROPERTY()
	TMap<FName, FMGChallengeProgress> ChallengeProgress;

	/// IDs of currently active challenges
	UPROPERTY()
	TArray<FName> ActiveChallengeIDs;

	/// All challenge sets, keyed by SetID
	UPROPERTY()
	TMap<FName, FMGChallengeSet> ChallengeSets;

	/// Active community challenges
	UPROPERTY()
	TArray<FMGCommunityChallenge> CommunityChallenges;

	/// Stat values for automatic objective tracking
	UPROPERTY()
	TMap<FName, int32> TrackedStats;

	/// Maximum simultaneous active challenges
	UPROPERTY()
	int32 MaxActiveChallenges = 10;

	/// Remaining daily challenge rerolls
	UPROPERTY()
	int32 DailyRerollsRemaining = 3;

	/// Timestamp of last daily reset
	UPROPERTY()
	FDateTime LastDailyReset;

	/// Timestamp of last weekly reset
	UPROPERTY()
	FDateTime LastWeeklyReset;

	/// Timer handle for periodic challenge tick
	FTimerHandle ChallengeTickHandle;
};

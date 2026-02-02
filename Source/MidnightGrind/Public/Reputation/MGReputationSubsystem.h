// Copyright Midnight Grind. All Rights Reserved.

#pragma once
/**
 * =============================================================================
 * MGReputationSubsystem.h
 * Player Reputation and Progression System for Midnight Grind
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the Reputation Subsystem, which tracks player standing
 * across multiple categories (Racing, Technical, Social, Creative, Competitive).
 * Reputation determines player tier, unlocks exclusive content, and awards
 * titles that display alongside the player's name.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. REPUTATION CATEGORIES (EMGReputationCategory):
 *    Different aspects of gameplay that earn reputation:
 *    - Overall: Combined score from all categories
 *    - Racing: Race wins, podium finishes, participation
 *    - Technical: Clean racing, consistency, no collisions
 *    - Social: Crew activities, community engagement
 *    - Creative: Livery shares, track creations, downloads
 *    - Competitive: Tournament placements, ranked performance
 *
 * 2. REPUTATION TIERS (EMGReputationTier):
 *    Progressive ranks based on accumulated reputation:
 *    - Unknown (0+): Just started
 *    - Rookie (100+): New to the scene
 *    - Regular (1,000+): Established player
 *    - Respected (5,000+): Known in the community
 *    - Elite (25,000+): Top-tier competitor
 *    - Legend (100,000+): Hall of fame status
 *
 * 3. REPUTATION UNLOCKS (FMGReputationUnlock):
 *    Content unlocked by reaching specific tiers:
 *    - Exclusive garages, livery kits, visual effects
 *    - Access to elite-only matchmaking queues
 *    - Special badges displayed on profile
 *
 * 4. PLAYER TITLES (FMGReputationTitle):
 *    Displayable titles shown next to player name:
 *    - Earned by reaching tiers in specific categories
 *    - "Rookie Racer", "Street Legend", "Technical Master"
 *    - Only one can be equipped at a time
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 *
 *   [Race Completion / Activity]
 *          |
 *          v
 *   [MGReputationSubsystem]
 *          |
 *          +-- AddReputation() --------> [Update Category + Overall]
 *          +-- UpdateTier() -----------> [Check tier thresholds]
 *          +-- CheckUnlocks() ---------> [Award new content]
 *          +-- CheckTitles() ----------> [Unlock new titles]
 *          |
 *          v
 *   [Events Broadcast]
 *          +-- OnReputationGained
 *          +-- OnTierReached
 *          +-- OnUnlockEarned
 *          +-- OnTitleUnlocked
 *
 * TYPICAL WORKFLOWS:
 * ------------------
 *
 * After a Race:
 * 1. RaceSubsystem calls OnRaceCompleted(Position, TotalRacers, bWasCleanRace)
 * 2. Racing reputation awarded based on placement
 * 3. Technical reputation awarded if race was clean
 * 4. Overall reputation updated automatically
 * 5. Tier checked and upgraded if threshold crossed
 * 6. UI notified via delegates
 *
 * @see EMGReputationTier - Defined in Core/MGSharedTypes.h
 * @see FMGReputationLevel - Current state of a reputation category
 * @see FMGReputationUnlock - Content unlocked by reputation
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGReputationSubsystem.generated.h"

// =============================================================================
// ENUMERATIONS
// =============================================================================

/**
 * Categories of player reputation.
 *
 * Reputation is tracked separately for different types of activities.
 * This allows players to specialize and be recognized for their strengths.
 * Overall reputation is the sum of all category reputation.
 */
UENUM(BlueprintType)
enum class EMGReputationCategory : uint8
{
	Overall,       ///< Combined reputation across all categories
	Racing,        ///< Race wins, podiums, participation
	Technical,     ///< Clean racing, consistency, precision
	Social,        ///< Crew activities, community engagement
	Creative,      ///< Liveries, tracks created and shared
	Competitive    ///< Tournament and ranked performance
};

// =============================================================================
// DATA STRUCTURES
// =============================================================================

/**
 * Record of a single reputation gain event.
 *
 * Used for history tracking and displaying recent reputation changes.
 * The Source field provides context for where the reputation came from.
 */
USTRUCT(BlueprintType)
struct FMGReputationGain
{
	GENERATED_BODY()

	/// Which category received the reputation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	EMGReputationCategory Category = EMGReputationCategory::Overall;

	/// Amount of reputation gained (always positive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	int64 Amount = 0;

	/// Description of what earned this reputation (e.g., "Race win", "Clean race")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	FString Source;

	/// When this reputation was earned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	FDateTime Timestamp;
};

/**
 * Current reputation state for a single category.
 *
 * Contains the current reputation amount, tier, and progress toward
 * the next tier. Used for UI display and tier calculations.
 */
USTRUCT(BlueprintType)
struct FMGReputationLevel
{
	GENERATED_BODY()

	/// Which category this level represents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	EMGReputationCategory Category = EMGReputationCategory::Overall;

	/// Total accumulated reputation in this category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	int64 CurrentReputation = 0;

	/// Current tier based on reputation amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	EMGReputationTier Tier = EMGReputationTier::Unknown;

	/// Reputation needed to reach the next tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	int64 ReputationToNextTier = 0;

	/// Progress to next tier as percentage (0.0 to 100.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	float TierProgressPercent = 0.0f;
};

/**
 * Content unlocked by reaching a reputation tier.
 *
 * Unlocks can be tied to overall reputation or specific categories.
 * Once unlocked, the content is permanently available to the player.
 */
USTRUCT(BlueprintType)
struct FMGReputationUnlock
{
	GENERATED_BODY()

	/// Unique identifier for this unlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FName UnlockID;

	/// Display name shown in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FText UnlockName;

	/// Description of what is unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FText Description;

	/// Minimum tier required in the specified category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	EMGReputationTier RequiredTier = EMGReputationTier::Rookie;

	/// Which category must reach the required tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	EMGReputationCategory RequiredCategory = EMGReputationCategory::Overall;

	/// True if player has unlocked this content
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	bool bUnlocked = false;
};

/**
 * Displayable title earned through reputation.
 *
 * Titles appear next to the player's name in lobbies, leaderboards,
 * and social features. Players can only equip one title at a time.
 */
USTRUCT(BlueprintType)
struct FMGReputationTitle
{
	GENERATED_BODY()

	/// Unique identifier for this title
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FName TitleID;

	/// The title text displayed (e.g., "Street Legend")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FText TitleText;

	/// Minimum tier required to unlock this title
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	EMGReputationTier RequiredTier = EMGReputationTier::Rookie;

	/// Which category must reach the required tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	EMGReputationCategory RequiredCategory = EMGReputationCategory::Overall;

	/// True if player has unlocked this title
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	bool bUnlocked = false;

	/// True if this title is currently equipped/displayed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	bool bEquipped = false;
};

// =============================================================================
// DELEGATES (Events)
// =============================================================================

/// Fires when reputation is gained in any category
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnReputationGained, EMGReputationCategory, Category, int64, Amount);

/// Fires when player reaches a new tier in any category
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTierReached, EMGReputationCategory, Category, EMGReputationTier, Tier);

/// Fires when a new unlock becomes available
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnUnlockEarned, const FMGReputationUnlock&, Unlock);

/// Fires when a new title is unlocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTitleUnlocked, const FMGReputationTitle&, Title);

// =============================================================================
// REPUTATION SUBSYSTEM CLASS
// =============================================================================

/**
 * Player reputation and progression tracking system.
 *
 * This subsystem manages reputation across multiple categories, determining
 * player tiers, unlocking content, and awarding displayable titles.
 * Reputation is earned through races, tournaments, crew activities, and
 * creative contributions.
 *
 * USAGE EXAMPLES:
 * @code
 * // Get the subsystem
 * UMGReputationSubsystem* Rep = GameInstance->GetSubsystem<UMGReputationSubsystem>();
 *
 * // Check current tier
 * EMGReputationTier Tier = Rep->GetTier(EMGReputationCategory::Overall);
 *
 * // Award reputation after a race
 * Rep->OnRaceCompleted(Position, TotalRacers, bWasCleanRace);
 *
 * // Listen for tier upgrades
 * Rep->OnTierReached.AddDynamic(this, &MyClass::HandleTierReached);
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGReputationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Called automatically when the game instance is created
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called automatically when the game is shutting down
	virtual void Deinitialize() override;

	// ==========================================
	// REPUTATION QUERIES
	// ==========================================

	/**
	 * Gets complete reputation level info for a category.
	 * @param Category Which category to query
	 * @return Full level data including tier and progress
	 */
	UFUNCTION(BlueprintPure, Category = "Reputation")
	FMGReputationLevel GetReputationLevel(EMGReputationCategory Category) const;

	/**
	 * Gets raw reputation value for a category.
	 * @param Category Which category to query
	 * @return Total reputation points accumulated
	 */
	UFUNCTION(BlueprintPure, Category = "Reputation")
	int64 GetReputation(EMGReputationCategory Category) const;

	/**
	 * Gets current tier for a category.
	 * @param Category Which category to query
	 * @return Current tier (Rookie, Regular, etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "Reputation")
	EMGReputationTier GetTier(EMGReputationCategory Category) const;

	/**
	 * Gets localized display name for a tier.
	 * @param Tier The tier to get name for
	 * @return Localized name (e.g., "Legend")
	 */
	UFUNCTION(BlueprintPure, Category = "Reputation")
	FText GetTierName(EMGReputationTier Tier) const;

	// ==========================================
	// REPUTATION EARNING
	// ==========================================

	/**
	 * Directly adds reputation to a category.
	 * @param Category Which category to add to
	 * @param Amount How much reputation to add
	 * @param Source Description of what earned this (for history)
	 */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void AddReputation(EMGReputationCategory Category, int64 Amount, const FString& Source);

	/**
	 * Awards reputation for completing a race.
	 * @param Position Final position (1 = first place)
	 * @param TotalRacers Total racers in the race
	 * @param bWasCleanRace True if no collisions/fouls
	 */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void OnRaceCompleted(int32 Position, int32 TotalRacers, bool bWasCleanRace);

	/**
	 * Awards reputation for tournament performance.
	 * @param Position Final tournament placement
	 * @param TotalParticipants Total tournament entrants
	 */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void OnTournamentResult(int32 Position, int32 TotalParticipants);

	/**
	 * Awards reputation for crew activities.
	 * @param ActivityType Type of activity (e.g., "CrewRace", "CrewWin")
	 */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void OnCrewActivity(const FString& ActivityType);

	/**
	 * Awards reputation for sharing creative content.
	 * @param bIsLivery True if livery, false if track
	 * @param Downloads Number of downloads received
	 */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void OnCreationShared(bool bIsLivery, int32 Downloads);

	// ==========================================
	// UNLOCKS
	// ==========================================

	/** Gets all defined unlocks (locked and unlocked). */
	UFUNCTION(BlueprintPure, Category = "Reputation|Unlocks")
	TArray<FMGReputationUnlock> GetAllUnlocks() const { return Unlocks; }

	/** Gets only unlocked content. */
	UFUNCTION(BlueprintPure, Category = "Reputation|Unlocks")
	TArray<FMGReputationUnlock> GetUnlockedItems() const;

	/** Gets content that hasn't been unlocked yet. */
	UFUNCTION(BlueprintPure, Category = "Reputation|Unlocks")
	TArray<FMGReputationUnlock> GetPendingUnlocks() const;

	/**
	 * Checks if a specific unlock has been earned.
	 * @param UnlockID The unlock to check
	 * @return True if unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Reputation|Unlocks")
	bool HasUnlock(FName UnlockID) const;

	// ==========================================
	// TITLES
	// ==========================================

	/** Gets all defined titles (locked and unlocked). */
	UFUNCTION(BlueprintPure, Category = "Reputation|Titles")
	TArray<FMGReputationTitle> GetAllTitles() const { return Titles; }

	/** Gets only unlocked titles. */
	UFUNCTION(BlueprintPure, Category = "Reputation|Titles")
	TArray<FMGReputationTitle> GetUnlockedTitles() const;

	/**
	 * Equips a title to display next to player name.
	 * @param TitleID The title to equip (must be unlocked)
	 */
	UFUNCTION(BlueprintCallable, Category = "Reputation|Titles")
	void EquipTitle(FName TitleID);

	/** Gets the currently equipped title. */
	UFUNCTION(BlueprintPure, Category = "Reputation|Titles")
	FMGReputationTitle GetEquippedTitle() const;

	// ==========================================
	// HISTORY
	// ==========================================

	/**
	 * Gets recent reputation gain events.
	 * @param Count Maximum entries to return (default 20)
	 * @return Array of recent gains, newest first
	 */
	UFUNCTION(BlueprintPure, Category = "Reputation")
	TArray<FMGReputationGain> GetRecentGains(int32 Count = 20) const;

	// ==========================================
	// EVENTS (Delegates)
	// ==========================================

	/// Fires when reputation is gained
	UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
	FMGOnReputationGained OnReputationGained;

	/// Fires when a new tier is reached
	UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
	FMGOnTierReached OnTierReached;

	/// Fires when content is unlocked
	UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
	FMGOnUnlockEarned OnUnlockEarned;

	/// Fires when a title is unlocked
	UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
	FMGOnTitleUnlocked OnTitleUnlocked;

protected:
	// ==========================================
	// INTERNAL FUNCTIONS
	// ==========================================

	/** Loads reputation data from persistent storage. */
	void LoadReputationData();

	/** Saves reputation data to persistent storage. */
	void SaveReputationData();

	/** Initializes the default unlock definitions. */
	void InitializeUnlocks();

	/** Initializes the default title definitions. */
	void InitializeTitles();

	/** Updates tier based on current reputation. */
	void UpdateTier(EMGReputationCategory Category);

	/** Checks if any new unlocks should be granted. */
	void CheckUnlocks();

	/** Checks if any new titles should be granted. */
	void CheckTitles();

	/** Gets the reputation threshold for a tier. */
	int64 GetReputationForTier(EMGReputationTier Tier) const;

private:
	// ==========================================
	// INTERNAL DATA
	// ==========================================

	/** Current reputation levels for each category */
	UPROPERTY()
	TMap<EMGReputationCategory, FMGReputationLevel> ReputationLevels;

	/** All defined unlocks */
	UPROPERTY()
	TArray<FMGReputationUnlock> Unlocks;

	/** All defined titles */
	UPROPERTY()
	TArray<FMGReputationTitle> Titles;

	/** History of recent reputation gains */
	UPROPERTY()
	TArray<FMGReputationGain> GainHistory;

	/** Currently equipped title ID */
	FName EquippedTitleID;

	/** Maximum history entries to retain */
	int32 MaxGainHistory = 100;
};

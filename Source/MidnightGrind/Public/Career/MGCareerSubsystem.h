// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGCareerSubsystem.h - Player Career Progression & Statistics System
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Career Subsystem, which tracks the player's long-term
 * progression through Midnight Grind. While the Campaign Subsystem handles individual
 * missions, this system manages your overall "career" as a street racer - from
 * newcomer to legend.
 *
 * Think of this as your "racing resume" - it tracks everything you've accomplished
 * across all game modes and determines your standing in the racing world.
 *
 *
 * KEY CONCEPTS & TERMINOLOGY:
 * ---------------------------
 *
 * 1. CAREER CHAPTERS: The five stages of your racing career progression.
 *    - Newcomer: Just starting out, learning the ropes (Chapter 1)
 *    - Rising: Making a name for yourself (Chapter 2)
 *    - Contender: Challenging the established racers (Chapter 3)
 *    - Champion: You've proven your dominance (Chapter 4)
 *    - Legend: Cementing your legacy, the highest tier (Chapter 5)
 *
 *    Unlike Campaign chapters (story beats), these are PROGRESSION tiers
 *    that unlock based on your overall performance.
 *
 * 2. CAREER MILESTONES: One-time achievements that mark significant moments.
 *    - FirstRace, FirstWin, FirstPodium: Early career milestones
 *    - DefeatedRival, WonTournament: Competitive achievements
 *    - BecameChampion, EarnedLegendStatus: High-tier accomplishments
 *
 *    Milestones can never be "un-earned" - they're permanent achievements.
 *
 * 3. CAREER OBJECTIVES: Specific goals to complete within your current chapter.
 *    - MainObjectives: Required to advance to the next chapter
 *    - Side Objectives: Optional goals for bonus rewards
 *    - Each has progress tracking (CurrentProgress / TargetProgress)
 *
 * 4. CAREER STATS: Lifetime statistics that track all your activities.
 *    - Race stats: TotalRaces, Wins, Podiums, CleanRaces
 *    - Combat stats: RivalsDefeated
 *    - Time stats: TotalDistanceKM, TotalRaceTimeHours
 *    - Streak stats: CurrentWinStreak, HighestWinStreak
 *
 * 5. REPUTATION: A numeric value representing your standing.
 *    - Earned through races, objectives, and achievements
 *    - Visible to other players in multiplayer
 *    - Used for matchmaking and unlocking content
 *
 * 6. GRINDCASH: The in-game currency awarded for objectives.
 *    - Used to buy cars, parts, customization items
 *    - Separate from real-money purchases
 *
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 * This is a UGameInstanceSubsystem that:
 * - Persists for the entire game session
 * - Receives data from race results, tournament outcomes, etc.
 * - Provides progression data to UI and other systems
 * - Saves/loads from player profile storage
 *
 * Key relationships:
 * - Race System: Reports race results -> Career tracks stats
 * - Campaign System: Mission completion -> Career checks milestones
 * - Multiplayer: Tournament results -> Career tracks wins
 * - Economy System: Career grants GrindCash rewards
 * - UI System: Displays career progress, chapter, objectives
 *
 *
 * CAREER vs CAMPAIGN - IMPORTANT DISTINCTION:
 * -------------------------------------------
 * - CAMPAIGN = Scripted story missions (finite content)
 * - CAREER = Ongoing progression (infinite, stat-based)
 *
 * You can complete the Campaign story but continue advancing your Career.
 * Career progression is primarily driven by MULTIPLAYER activities.
 *
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * // After every race, report results:
 * TArray<FString> DefeatedRivals = {"Player123", "RivalGuy"};
 * CareerSubsystem->OnRaceCompleted(1, 8, true, DefeatedRivals); // 1st of 8, clean race
 *
 * // Check if player has reached a milestone:
 * if (CareerSubsystem->HasCompletedMilestone(EMGCareerMilestone::BecameChampion))
 * {
 *     // Show champion badge
 * }
 *
 * // Get progress towards next chapter:
 * float Progress = CareerSubsystem->GetChapterProgressPercent();
 *
 * // Listen for chapter advancement:
 * CareerSubsystem->OnChapterAdvanced.AddDynamic(this, &UMyWidget::OnNewChapter);
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCareerSubsystem.generated.h"

/**
 * =============================================================================
 * DESIGN PHILOSOPHY: Career Integrated with Multiplayer
 * =============================================================================
 *
 * Your career progression is tied to the LIVE multiplayer world:
 * - Progress comes from racing real players, not just AI
 * - Story events occur during live multiplayer races
 * - Rivals are actual players you've competed against
 * - Reputation is earned through all online activities
 *
 * This differs from traditional single-player career modes where you
 * progress through scripted content. Here, YOUR story writes itself
 * through your actual gameplay experiences.
 * =============================================================================
 */

/**
 * Career Chapter - The five stages of your racing career
 *
 * Unlike Campaign chapters (story beats), Career chapters are PROGRESSION TIERS
 * based on your overall skill and achievements. You advance by completing
 * objectives and building reputation through multiplayer gameplay.
 *
 * Think of these like "ranks" in a competitive game, but with narrative flavor.
 */
UENUM(BlueprintType)
enum class EMGCareerChapter : uint8
{
	/** Chapter 1: Just starting out. Learning the ropes, first races. */
	Newcomer,

	/** Chapter 2: People are starting to notice you. Building a reputation. */
	Rising,

	/** Chapter 3: You're a serious competitor. Challenging the established racers. */
	Contender,

	/** Chapter 4: You've proven yourself. One of the best in the scene. */
	Champion,

	/** Chapter 5: You ARE the scene. Your legacy is cemented forever. */
	Legend
};

/**
 * Career Milestone - One-time achievements marking significant career moments
 *
 * Milestones are permanent accomplishments that can never be "un-earned."
 * They mark major moments in your career and often come with special rewards
 * or recognition. The game tracks these to customize narrative and UI.
 */
UENUM(BlueprintType)
enum class EMGCareerMilestone : uint8
{
	/** Completed your very first race (any position). The journey begins. */
	FirstRace,

	/** Won a race for the first time. A taste of victory! */
	FirstWin,

	/** Finished in top 3 for the first time. You're competitive now. */
	FirstPodium,

	/** Became a member of a racing crew. Social milestone. */
	JoinedCrew,

	/** Beat your designated rival in a head-to-head. Personal victory. */
	DefeatedRival,

	/** Won a full tournament bracket. Major competitive achievement. */
	WonTournament,

	/** Advanced to the Contender career chapter. Serious racer now. */
	ReachedContender,

	/** Advanced to the Champion career chapter. Elite status. */
	BecameChampion,

	/** Advanced to the Legend career chapter. Highest honor. */
	EarnedLegendStatus
};

/**
 * Career Objective - A goal to achieve within your current career chapter
 *
 * Objectives guide your progression through each career chapter. Completing
 * them earns rewards and advances you toward the next chapter. Main objectives
 * are required for advancement; side objectives provide bonus rewards.
 */
USTRUCT(BlueprintType)
struct FMGCareerObjective
{
	GENERATED_BODY()

	/** Unique identifier for this objective (e.g., "Win5Races_Newcomer"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	/** Short display title (e.g., "Win 5 Races"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** Longer explanation of what to do and why it matters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Which career chapter this objective belongs to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCareerChapter Chapter = EMGCareerChapter::Newcomer;

	/** How much progress is needed to complete this objective. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetProgress = 1;

	/** Current progress (updated by gameplay systems). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentProgress = 0;

	/** Runtime flag: has this objective been completed? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	/** If true, this is required for chapter advancement. If false, it's a bonus. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMainObjective = false;

	/** In-game currency reward for completion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 GrindCashReward = 0;

	/** Reputation points reward for completion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ReputationReward = 0;

	/** Optional: unlock something upon completion (car, part, area, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockReward;
};

/**
 * Career Stats - Lifetime statistics tracking all gameplay
 *
 * These statistics are accumulated across ALL your gameplay and never reset
 * (unless player explicitly resets career). They're used for:
 * - Profile displays and leaderboards
 * - Objective completion checking
 * - Matchmaking considerations
 * - Bragging rights
 */
USTRUCT(BlueprintType)
struct FMGCareerStats
{
	GENERATED_BODY()

	/** Total number of races entered (regardless of outcome). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	/** Total number of first-place finishes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	/** Total number of top-3 finishes (including wins). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Podiums = 0;

	/** Number of designated rivals you've beaten in races. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RivalsDefeated = 0;

	/** Tournament brackets won (not individual tournament races). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TournamentsWon = 0;

	/** Total distance driven across all races (kilometers). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistanceKM = 0.0f;

	/** Total time spent racing (hours, not including menus/garage). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRaceTimeHours = 0.0f;

	/** Races completed without significant collisions (sportsmanship). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanRaces = 0;

	/** Your best-ever consecutive win streak. For profile display. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestWinStreak = 0;

	/** Your current consecutive wins. Resets on loss. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentWinStreak = 0;
};

/**
 * Career Progress - The complete state of a player's career
 *
 * This struct contains everything about where you are in your career.
 * It's the "save data" for the career system and includes your chapter,
 * progress, milestones, and all statistics.
 */
USTRUCT(BlueprintType)
struct FMGCareerProgress
{
	GENERATED_BODY()

	/** Which career tier you're currently in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCareerChapter CurrentChapter = EMGCareerChapter::Newcomer;

	/** How far into the current chapter you are (main objective completions). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChapterProgress = 0;

	/** How much progress is needed to advance to next chapter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChapterProgressRequired = 100;

	/** Lifetime accumulated reputation (never decreases). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalReputation = 0;

	/** All one-time milestones that have been achieved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGCareerMilestone> CompletedMilestones;

	/** All lifetime statistics (nested struct for organization). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGCareerStats Stats;
};

/**
 * =============================================================================
 * DELEGATE DECLARATIONS
 * =============================================================================
 *
 * Events for UI and other systems to react to career progression.
 * These are major moments that typically warrant special celebration UI.
 */

/** Broadcast when you advance to a new career chapter. Major celebration moment! */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnChapterAdvanced, EMGCareerChapter, NewChapter);

/** Broadcast when a one-time milestone is achieved. Show achievement popup. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMilestoneReached, EMGCareerMilestone, Milestone);

/** Broadcast when a career objective is completed. Update objective UI, grant rewards. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnObjectiveCompleted, const FMGCareerObjective&, Objective);

/** Broadcast when chapter progress changes. Update progress bar UI. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCareerProgressUpdated, int32, Progress, int32, Required);

/**
 * =============================================================================
 * UMGCareerSubsystem - Player Career Progression Management
 * =============================================================================
 *
 * This subsystem tracks the player's overall career progression through
 * Midnight Grind. It handles chapter advancement, milestone tracking,
 * objective completion, and lifetime statistics.
 *
 * As a UGameInstanceSubsystem:
 * - Automatically created when game starts
 * - Access via: GetGameInstance()->GetSubsystem<UMGCareerSubsystem>()
 * - Persists across level loads
 *
 * The class is organized into functional sections:
 * - Career Progress: Current chapter, progress percentage
 * - Objectives: Goals to complete for progression
 * - Race Integration: Hooks called by the race system
 * - Stats: Lifetime statistics tracking
 * - Milestones: One-time achievement checking
 * - Events: Delegates for UI updates
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCareerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Called by engine when subsystem is created. Loads saved career data. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called by engine when subsystem is destroyed. Saves career data. */
	virtual void Deinitialize() override;

	// =========================================================================
	// CAREER PROGRESS
	// =========================================================================

	/** Returns the complete career progress state including chapter, stats, and milestones. */
	UFUNCTION(BlueprintPure, Category = "Career")
	FMGCareerProgress GetProgress() const { return Progress; }

	/** Returns which career tier (Newcomer through Legend) the player is currently in. */
	UFUNCTION(BlueprintPure, Category = "Career")
	EMGCareerChapter GetCurrentChapter() const { return Progress.CurrentChapter; }

	/** Returns progress through current chapter as 0.0-1.0 percentage for UI progress bars. */
	UFUNCTION(BlueprintPure, Category = "Career")
	float GetChapterProgressPercent() const;

	/** Returns the localized display name for a career chapter (e.g., "Champion"). */
	UFUNCTION(BlueprintPure, Category = "Career")
	FText GetChapterName(EMGCareerChapter Chapter) const;

	// =========================================================================
	// OBJECTIVES
	// =========================================================================

	/** Returns all objectives for the player's current career chapter. */
	UFUNCTION(BlueprintPure, Category = "Career")
	TArray<FMGCareerObjective> GetCurrentObjectives() const;

	/** Returns only the main (required) objectives for chapter advancement. */
	UFUNCTION(BlueprintPure, Category = "Career")
	TArray<FMGCareerObjective> GetMainObjectives() const;

	/**
	 * Updates progress on a specific objective. Call when objective-related actions occur.
	 * @param ObjectiveID The unique identifier of the objective to update.
	 * @param Progress The amount of progress to add (not set - it's additive).
	 */
	UFUNCTION(BlueprintCallable, Category = "Career")
	void UpdateObjectiveProgress(FName ObjectiveID, int32 Progress);

	// =========================================================================
	// RACE INTEGRATION
	// =========================================================================

	/**
	 * Called by the race system when a race finishes. Updates stats and checks milestones.
	 * @param Position Final position (1 = first place).
	 * @param TotalRacers Number of participants in the race.
	 * @param bWasCleanRace True if completed without significant collisions.
	 * @param DefeatedRivals Player IDs of designated rivals beaten in this race.
	 */
	UFUNCTION(BlueprintCallable, Category = "Career|Race")
	void OnRaceCompleted(int32 Position, int32 TotalRacers, bool bWasCleanRace, const TArray<FString>& DefeatedRivals);

	/**
	 * Called when the player wins a tournament bracket (not individual races).
	 * @param TournamentID Unique identifier of the tournament won.
	 */
	UFUNCTION(BlueprintCallable, Category = "Career|Race")
	void OnTournamentWon(FName TournamentID);

	/**
	 * Called when the player joins a racing crew. Triggers JoinedCrew milestone.
	 * @param CrewID Unique identifier of the crew joined.
	 */
	UFUNCTION(BlueprintCallable, Category = "Career|Race")
	void OnCrewJoined(FName CrewID);

	// =========================================================================
	// STATS
	// =========================================================================

	/** Returns all lifetime career statistics (races, wins, distance, etc.). */
	UFUNCTION(BlueprintPure, Category = "Career")
	FMGCareerStats GetStats() const { return Progress.Stats; }

	/**
	 * Adds distance to the lifetime odometer. Called by race system during gameplay.
	 * @param DistanceKM Distance traveled in kilometers.
	 */
	UFUNCTION(BlueprintCallable, Category = "Career")
	void AddDistance(float DistanceKM);

	/**
	 * Adds time to the lifetime race timer. Called by race system at race end.
	 * @param TimeHours Time spent racing in hours.
	 */
	UFUNCTION(BlueprintCallable, Category = "Career")
	void AddRaceTime(float TimeHours);

	// =========================================================================
	// MILESTONES
	// =========================================================================

	/**
	 * Checks if a specific milestone has been achieved.
	 * @param Milestone The milestone to check.
	 * @return True if the milestone has been completed.
	 */
	UFUNCTION(BlueprintPure, Category = "Career")
	bool HasCompletedMilestone(EMGCareerMilestone Milestone) const;

	/** Returns all milestones that haven't been achieved yet. Useful for tracking UI. */
	UFUNCTION(BlueprintPure, Category = "Career")
	TArray<EMGCareerMilestone> GetPendingMilestones() const;

	// =========================================================================
	// EVENTS
	// =========================================================================

	/** Broadcast when the player advances to a new career chapter. Major celebration moment! */
	UPROPERTY(BlueprintAssignable, Category = "Career|Events")
	FMGOnChapterAdvanced OnChapterAdvanced;

	/** Broadcast when a one-time milestone is achieved. Trigger achievement popup. */
	UPROPERTY(BlueprintAssignable, Category = "Career|Events")
	FMGOnMilestoneReached OnMilestoneReached;

	/** Broadcast when a career objective is completed. Update UI and grant rewards. */
	UPROPERTY(BlueprintAssignable, Category = "Career|Events")
	FMGOnObjectiveCompleted OnObjectiveCompleted;

	/** Broadcast when chapter progress changes. Update progress bar UI. */
	UPROPERTY(BlueprintAssignable, Category = "Career|Events")
	FMGOnCareerProgressUpdated OnCareerProgressUpdated;

protected:
	// =========================================================================
	// INTERNAL HELPERS
	// =========================================================================

	/** Loads career progress from persistent storage. Called on Initialize. */
	void LoadCareerData();

	/** Saves career progress to persistent storage. Called on Deinitialize and key moments. */
	void SaveCareerData();

	/** Populates the Objectives array with chapter-appropriate goals. */
	void InitializeObjectives();

	/** Evaluates if main objectives are complete and triggers chapter advancement. */
	void CheckChapterAdvancement();

	/** Evaluates current stats against milestone requirements. */
	void CheckMilestones();

	/**
	 * Marks a milestone as complete and broadcasts the event.
	 * @param Milestone The milestone to complete.
	 */
	void CompleteMilestone(EMGCareerMilestone Milestone);

	/** Advances to the next career chapter and reinitializes objectives. */
	void AdvanceChapter();

	/**
	 * Grants GrindCash and reputation rewards for completing an objective.
	 * @param Objective The completed objective containing reward data.
	 */
	void GrantObjectiveReward(const FMGCareerObjective& Objective);

private:
	// =========================================================================
	// STATE
	// =========================================================================

	/** Current career progress including chapter, stats, reputation, and milestones. */
	UPROPERTY()
	FMGCareerProgress Progress;

	/** All objectives for the current chapter (both main and side). */
	UPROPERTY()
	TArray<FMGCareerObjective> Objectives;
};

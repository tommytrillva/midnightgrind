#pragma once


/**
 * @file MGTougeHandler.h
 * @brief Touge Handler - Japanese mountain pass battle racing
 *
 * Touge (pronounced "toh-geh") racing originates from Japanese mountain pass
 * street racing culture. Two drivers compete head-to-head on a winding mountain
 * road, taking turns as leader and chaser. The format tests both aggressive
 * driving as leader and precision following as chaser.
 *
 * @section overview Overview
 * Touge battles are intimate 1v1 affairs on narrow, technical mountain roads.
 * The race consists of two runs with alternating leader/chaser roles. Each run
 * can end in three ways: the leader pulls away, the chaser catches up, or
 * someone crashes. This creates intense psychological pressure and rewards
 * consistency over raw speed.
 *
 * @section format Race Format
 * 1. **Run 1**: Player 1 leads, Player 2 chases
 * 2. **Transition**: Cars swap positions
 * 3. **Run 2**: Player 2 leads, Player 1 chases
 * 4. **Tiebreaker**: If needed, sudden death run
 *
 * @section win_conditions Win Conditions
 * A run can end in several ways:
 * - **Leader Pulls Away**: Gap exceeds threshold (default 100m) - Leader wins run
 * - **Chaser Catches Up**: Gap closes to touching distance - Chaser wins run
 * - **Leader Crashes**: Chaser wins run
 * - **Chaser Crashes**: Leader wins run
 * - **Time Expires**: Run judged by final gap
 *
 * @section overall_winner Overall Winner
 * - **Best-of-3**: First to win 2 runs wins the battle
 * - **First-to-Win**: First run winner takes all (quick mode)
 *
 * @section hazards Track Hazards
 * Touge courses feature dangerous elements:
 * - Cliff edges (instant crash if you go over)
 * - Guardrails (can scrape against but costly)
 * - Tight hairpin corners
 * - Variable width roads
 *
 * @see UMGRaceTypeHandler Base class
 * @see EMGTougePhase Race phase state machine
 * @see PRD Section 4.3 for design requirements
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

// Copyright Midnight Grind. All Rights Reserved.

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGTougeHandler.generated.h"

// ============================================================================
// TOUGE STATE ENUMS
// ============================================================================

/**
 * @brief Current phase of the touge battle
 *
 * Tracks progression through the alternating-lead format.
 */
UENUM(BlueprintType)
enum class EMGTougePhase : uint8
{
	/** First run in progress - Player 1 is leading */
	FirstRun_P1Leads,
	/** First run in progress - Player 2 is leading (if randomized) */
	FirstRun_P2Leads,
	/** Between runs - swapping positions */
	Transition,
	/** Second run in progress - roles reversed from first run */
	SecondRun,
	/** Tie after regulation - sudden death run */
	Tiebreaker,
	/** Battle has concluded - results available */
	Complete
};

/**
 * @brief How a run ended (determines who wins the run)
 */
UENUM(BlueprintType)
enum class EMGTougeResultType : uint8
{
	/** Run still in progress */
	Pending,
	/** Leader created decisive gap - Leader wins */
	LeaderPulledAway,
	/** Chaser closed to bumper distance - Chaser wins */
	ChaserCaughtUp,
	/** Chaser hit wall/cliff - Leader wins */
	ChaserCrashed,
	/** Leader hit wall/cliff - Chaser wins */
	LeaderCrashed,
	/** Max run duration reached - judge by final gap */
	TimeExpired
};

// ============================================================================
// TOUGE DATA STRUCTS
// ============================================================================

/**
 * @brief Data for one participant in the touge battle
 */
USTRUCT(BlueprintType)
struct FMGTougeParticipant
{
	GENERATED_BODY()

	/// Reference to the participant's vehicle actor
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Vehicle;

	/// Total runs won in this battle (0-2 typically)
	UPROPERTY(BlueprintReadOnly)
	int32 RoundsWon = 0;

	/// Best time down the mountain (for display)
	UPROPERTY(BlueprintReadOnly)
	float BestTime = 0.0f;

	/// Number of crashes during battle (for ranking tiebreaks)
	UPROPERTY(BlueprintReadOnly)
	int32 Crashes = 0;

	/// Times the opponent caught up while this player led
	UPROPERTY(BlueprintReadOnly)
	int32 TimesCaughtAsLeader = 0;

	/// Times this player lost sight of leader while chasing
	UPROPERTY(BlueprintReadOnly)
	int32 TimesLostAsChaser = 0;
};

/**
 * @brief Data for a single run within the touge battle
 */
USTRUCT(BlueprintType)
struct FMGTougeRunData
{
	GENERATED_BODY()

	/// Index of the leading participant (0 or 1)
	UPROPERTY(BlueprintReadOnly)
	int32 LeaderIndex = 0;

	///
	/// Current gap between cars (positive = leader ahead)
	/// Measured along the track spline, not direct distance
	///
	UPROPERTY(BlueprintReadOnly)
	float GapDistance = 0.0f;

	/// Time elapsed in this run (seconds)
	UPROPERTY(BlueprintReadOnly)
	float RunTime = 0.0f;

	/// How this run ended (or Pending if still racing)
	UPROPERTY(BlueprintReadOnly)
	EMGTougeResultType Result = EMGTougeResultType::Pending;

	/// Winner of this run (-1 if still pending or draw)
	UPROPERTY(BlueprintReadOnly)
	int32 RunWinnerIndex = -1;
};

// ============================================================================
// TOUGE EVENT DELEGATES
// ============================================================================

/** Broadcast when the battle phase changes (new run, transition, complete) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTougePhaseChanged, EMGTougePhase, NewPhase, int32, LeaderIndex);

/** Broadcast when gap distance changes significantly (for UI tension) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTougeGapChanged, float, NewGap, bool, bLeaderAhead);

/** Broadcast when a run completes with result */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTougeRunComplete, int32, RunNumber, int32, WinnerIndex, EMGTougeResultType, ResultType);

/** Broadcast when either participant crashes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTougeCrash, int32, CrashedParticipantIndex);

// ============================================================================
// TOUGE HANDLER CLASS
// ============================================================================

/**
 * @brief Handler for touge (mountain pass battle) racing
 *
 * Implements the alternating-lead format of Japanese touge racing.
 * Two participants race down a mountain pass, taking turns as leader
 * and chaser. Victories are earned by pulling away, catching up, or
 * capitalizing on opponent crashes.
 *
 * @section participant_limit Participant Limit
 * Touge is strictly a 2-participant format. The handler will assert
 * if initialized with more or fewer than 2 racers.
 *
 * @section gap_tracking Gap Tracking
 * Gap is measured along the track spline (racing line), not as direct
 * distance. This ensures fair measurement around corners.
 */
UCLASS(Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGTougeHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGTougeHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE OVERRIDES
	// Note: Uses different function signatures than base
	// ==========================================

	/** Initialize with race configuration (expects exactly 2 participants) */
	virtual void InitializeRace(const FMGRaceConfiguration& Config) override;

	/** Begin the first run */
	virtual void StartRace() override;

	/** Update gap tracking and check for run completion */
	virtual void UpdateRace(float MGDeltaTime) override;

	/** Finalize results */
	virtual void EndRace() override;

	/** Check if battle has concluded */
	virtual bool IsRaceComplete() const override;

	/** Get final results for both participants */
	virtual TArray<FMGRaceResult> GetResults() const override;

	/** Returns "Touge Battle" */
	virtual FText GetRaceTypeName() const override;

	// ==========================================
	// TOUGE-SPECIFIC QUERIES
	// Information about current battle state
	// ==========================================

	/**
	 * @brief Get the current battle phase
	 * @return Current phase in the touge format
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	EMGTougePhase GetCurrentPhase() const { return CurrentPhase; }

	/**
	 * @brief Get which run number we're on
	 * @return 1, 2, or 3 (if tiebreaker)
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	int32 GetCurrentRunNumber() const { return CurrentRunNumber; }

	/**
	 * @brief Get the index of the current leader
	 * @return 0 or 1 (participant index)
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	int32 GetCurrentLeaderIndex() const { return CurrentLeaderIndex; }

	/**
	 * @brief Get data for a participant
	 * @param Index Participant index (0 or 1)
	 * @return Participant stats and state
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	FMGTougeParticipant GetParticipant(int32 Index) const;

	/**
	 * @brief Get data for the current run
	 * @return Current run state and gap
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	FMGTougeRunData GetCurrentRunData() const { return CurrentRun; }

	/**
	 * @brief Get current gap between cars
	 * @return Gap in centimeters (track distance)
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	float GetGapDistance() const { return CurrentRun.GapDistance; }

	/**
	 * @brief Check if leader is ahead or being caught
	 * @return True if gap is positive (leader ahead)
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	bool IsLeaderAhead() const { return CurrentRun.GapDistance > 0.0f; }

	// ==========================================
	// TOUGE-SPECIFIC ACTIONS
	// Methods to report race events
	// ==========================================

	/**
	 * @brief Report that a participant crashed
	 * @param ParticipantIndex Who crashed (0 or 1)
	 * @note Ends the current run with crash result
	 */
	UFUNCTION(BlueprintCallable, Category = "Touge")
	void ReportCrash(int32 ParticipantIndex);

	/**
	 * @brief Report that a participant reached the finish
	 * @param ParticipantIndex Who finished (0 or 1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Touge")
	void ReportFinish(int32 ParticipantIndex);

	// ==========================================
	// CONFIGURATION
	// Battle parameters (can be set before race)
	// ==========================================

	/** Gap distance for leader to win by pulling away (cm) - default 100m */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float LeaderVictoryGap = 10000.0f;

	/** Gap distance for chaser to win by catching up (cm) - default 5m (bumper to bumper) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float ChaserVictoryGap = 500.0f;

	/** Maximum time for a single run (seconds) - default 3 minutes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MaxRunDuration = 180.0f;

	/** Time between runs for position swap (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float TransitionDuration = 5.0f;

	/** True = best-of-3, False = first run wins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bBestOfThree = true;

	// ==========================================
	// EVENTS
	// Delegates for touge battle events
	// ==========================================

	/** Broadcast when phase changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTougePhaseChanged OnPhaseChanged;

	/** Broadcast when gap changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTougeGapChanged OnGapChanged;

	/** Broadcast when a run completes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTougeRunComplete OnRunComplete;

	/** Broadcast when someone crashes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTougeCrash OnCrash;

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Calculate current gap based on track positions */
	void UpdateGap(float MGDeltaTime);

	/** Check if current run should end */
	void CheckRunCompletion();

	/** Initialize the next run (swap leader/chaser) */
	void StartNextRun();

	/** End the current run with given result */
	void CompleteRun(EMGTougeResultType Result, int32 WinnerIndex);

	/** Check if someone has won the overall battle */
	bool CheckForOverallWinner(int32& OutWinnerIndex);

	/** Change the current phase and broadcast */
	void SetPhase(EMGTougePhase NewPhase);

private:
	// ==========================================
	// BATTLE STATE
	// ==========================================

	/// Current phase of the battle
	EMGTougePhase CurrentPhase = EMGTougePhase::FirstRun_P1Leads;

	/// Which run we're on (1, 2, or 3 for tiebreaker)
	int32 CurrentRunNumber = 1;

	/// Who is currently leading (0 or 1)
	int32 CurrentLeaderIndex = 0;

	/// Data for both participants (fixed size array)
	FMGTougeParticipant Participants[2];

	/// Data for the run in progress
	FMGTougeRunData CurrentRun;

	/// History of completed runs
	TArray<FMGTougeRunData> CompletedRuns;

	/// Timer for transition between runs
	float TransitionTimer = 0.0f;

	/// Has the battle concluded?
	bool bRaceComplete = false;

	/// Overall winner index (-1 until complete)
	int32 OverallWinnerIndex = -1;

	/// Finish tracking for current run
	bool bParticipant0Finished = false;
	bool bParticipant1Finished = false;
};

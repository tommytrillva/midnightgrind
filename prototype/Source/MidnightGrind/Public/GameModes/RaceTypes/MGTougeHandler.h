// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGTougeHandler.generated.h"

/**
 * Touge round phase
 */
UENUM(BlueprintType)
enum class EMGTougePhase : uint8
{
	/** First run - Player 1 leads */
	FirstRun_P1Leads,
	/** First run - Player 2 leads */
	FirstRun_P2Leads,
	/** Between runs */
	Transition,
	/** Second run - roles reversed */
	SecondRun,
	/** Tiebreaker needed */
	Tiebreaker,
	/** Race complete */
	Complete
};

/**
 * Touge result type
 */
UENUM(BlueprintType)
enum class EMGTougeResultType : uint8
{
	/** Still racing */
	Pending,
	/** Leader pulled away */
	LeaderPulledAway,
	/** Chaser caught up */
	ChaserCaughtUp,
	/** Chaser crashed */
	ChaserCrashed,
	/** Leader crashed */
	LeaderCrashed,
	/** Time expired */
	TimeExpired
};

/**
 * Touge participant data
 */
USTRUCT(BlueprintType)
struct FMGTougeParticipant
{
	GENERATED_BODY()

	/** Vehicle actor */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Vehicle;

	/** Rounds won */
	UPROPERTY(BlueprintReadOnly)
	int32 RoundsWon = 0;

	/** Best time (seconds) */
	UPROPERTY(BlueprintReadOnly)
	float BestTime = 0.0f;

	/** Crashes this battle */
	UPROPERTY(BlueprintReadOnly)
	int32 Crashes = 0;

	/** Times caught as leader */
	UPROPERTY(BlueprintReadOnly)
	int32 TimesCaughtAsLeader = 0;

	/** Times lost as chaser */
	UPROPERTY(BlueprintReadOnly)
	int32 TimesLostAsChaser = 0;
};

/**
 * Touge run data
 */
USTRUCT(BlueprintType)
struct FMGTougeRunData
{
	GENERATED_BODY()

	/** Leader index for this run */
	UPROPERTY(BlueprintReadOnly)
	int32 LeaderIndex = 0;

	/** Current gap distance */
	UPROPERTY(BlueprintReadOnly)
	float GapDistance = 0.0f;

	/** Run time */
	UPROPERTY(BlueprintReadOnly)
	float RunTime = 0.0f;

	/** Result type */
	UPROPERTY(BlueprintReadOnly)
	EMGTougeResultType Result = EMGTougeResultType::Pending;

	/** Winner index for this run */
	UPROPERTY(BlueprintReadOnly)
	int32 RunWinnerIndex = -1;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTougePhaseChanged, EMGTougePhase, NewPhase, int32, LeaderIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTougeGapChanged, float, NewGap, bool, bLeaderAhead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTougeRunComplete, int32, RunNumber, int32, WinnerIndex, EMGTougeResultType, ResultType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTougeCrash, int32, CrashedParticipantIndex);

/**
 * Touge/Canyon Duel Race Handler
 * Implements mountain pass racing as per PRD Section 4.3
 *
 * Format: Alternating lead
 * Hazards: Cliff edges, guardrails
 *
 * Features:
 * - 2 participants only
 * - Two runs with alternating leader
 * - Gap-based or crash-based outcomes
 * - Best-of-3 or first-to-win format
 * - Cliff/guardrail hazards
 */
UCLASS(Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGTougeHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGTougeHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE
	// ==========================================

	virtual void InitializeRace(const FMGRaceConfiguration& Config) override;
	virtual void StartRace() override;
	virtual void UpdateRace(float DeltaTime) override;
	virtual void EndRace() override;
	virtual bool IsRaceComplete() const override;
	virtual TArray<FMGRaceResult> GetResults() const override;
	virtual FText GetRaceTypeName() const override;

	// ==========================================
	// TOUGE SPECIFIC
	// ==========================================

	/**
	 * Get current phase
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	EMGTougePhase GetCurrentPhase() const { return CurrentPhase; }

	/**
	 * Get current run number (1 or 2)
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	int32 GetCurrentRunNumber() const { return CurrentRunNumber; }

	/**
	 * Get current leader index
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	int32 GetCurrentLeaderIndex() const { return CurrentLeaderIndex; }

	/**
	 * Get participant data
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	FMGTougeParticipant GetParticipant(int32 Index) const;

	/**
	 * Get current run data
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	FMGTougeRunData GetCurrentRunData() const { return CurrentRun; }

	/**
	 * Get gap distance
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	float GetGapDistance() const { return CurrentRun.GapDistance; }

	/**
	 * Is leader ahead
	 */
	UFUNCTION(BlueprintPure, Category = "Touge")
	bool IsLeaderAhead() const { return CurrentRun.GapDistance > 0.0f; }

	/**
	 * Report crash for participant
	 */
	UFUNCTION(BlueprintCallable, Category = "Touge")
	void ReportCrash(int32 ParticipantIndex);

	/**
	 * Report reaching finish for participant
	 */
	UFUNCTION(BlueprintCallable, Category = "Touge")
	void ReportFinish(int32 ParticipantIndex);

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Gap distance for leader victory (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float LeaderVictoryGap = 10000.0f; // 100 meters

	/** Catch-up distance for chaser victory (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float ChaserVictoryGap = 500.0f; // 5 meters (essentially touching)

	/** Maximum run duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MaxRunDuration = 180.0f; // 3 minutes per run

	/** Transition time between runs (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float TransitionDuration = 5.0f;

	/** Is best-of-3 format (vs first-to-win) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bBestOfThree = true;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Phase changed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTougePhaseChanged OnPhaseChanged;

	/** Gap changed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTougeGapChanged OnGapChanged;

	/** Run complete */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTougeRunComplete OnRunComplete;

	/** Participant crashed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTougeCrash OnCrash;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update gap tracking */
	void UpdateGap(float DeltaTime);

	/** Check for run completion */
	void CheckRunCompletion();

	/** Start next run */
	void StartNextRun();

	/** Complete current run with result */
	void CompleteRun(EMGTougeResultType Result, int32 WinnerIndex);

	/** Check for overall winner */
	bool CheckForOverallWinner(int32& OutWinnerIndex);

	/** Set phase */
	void SetPhase(EMGTougePhase NewPhase);

private:
	/** Current phase */
	EMGTougePhase CurrentPhase = EMGTougePhase::FirstRun_P1Leads;

	/** Current run number */
	int32 CurrentRunNumber = 1;

	/** Current leader index */
	int32 CurrentLeaderIndex = 0;

	/** Participant data */
	FMGTougeParticipant Participants[2];

	/** Current run data */
	FMGTougeRunData CurrentRun;

	/** Previous runs data */
	TArray<FMGTougeRunData> CompletedRuns;

	/** Transition timer */
	float TransitionTimer = 0.0f;

	/** Is race complete */
	bool bRaceComplete = false;

	/** Overall winner index */
	int32 OverallWinnerIndex = -1;

	/** Finish flags */
	bool bParticipant0Finished = false;
	bool bParticipant1Finished = false;
};

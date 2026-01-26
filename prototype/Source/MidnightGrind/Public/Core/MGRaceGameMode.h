// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MGRaceGameMode.generated.h"

class UMGTrackSubsystem;
class UMGReplaySubsystem;
class AMGCheckpointActor;

/**
 * Race phase
 */
UENUM(BlueprintType)
enum class EMGRacePhase : uint8
{
	/** Waiting for players */
	WaitingForPlayers,
	/** Pre-race countdown */
	Countdown,
	/** Race in progress */
	Racing,
	/** Race finished, cooldown */
	Finished,
	/** Race ended, processing results */
	Results
};

/**
 * Racer state
 */
USTRUCT(BlueprintType)
struct FMGRacerState
{
	GENERATED_BODY()

	/** Player/AI controller */
	UPROPERTY(BlueprintReadOnly)
	AController* Controller = nullptr;

	/** Vehicle pawn */
	UPROPERTY(BlueprintReadOnly)
	APawn* Vehicle = nullptr;

	/** Current position in race */
	UPROPERTY(BlueprintReadOnly)
	int32 Position = 0;

	/** Current lap (1-based) */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLap = 1;

	/** Last checkpoint passed */
	UPROPERTY(BlueprintReadOnly)
	int32 LastCheckpoint = 0;

	/** Total distance traveled */
	UPROPERTY(BlueprintReadOnly)
	float DistanceTraveled = 0.0f;

	/** Total race time */
	UPROPERTY(BlueprintReadOnly)
	float TotalTime = 0.0f;

	/** Best lap time */
	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = FLT_MAX;

	/** Current lap start time */
	UPROPERTY(BlueprintReadOnly)
	float CurrentLapStartTime = 0.0f;

	/** Lap times */
	UPROPERTY(BlueprintReadOnly)
	TArray<float> LapTimes;

	/** Has finished race */
	UPROPERTY(BlueprintReadOnly)
	bool bHasFinished = false;

	/** Finish time */
	UPROPERTY(BlueprintReadOnly)
	float FinishTime = 0.0f;

	/** Did not finish */
	UPROPERTY(BlueprintReadOnly)
	bool bDNF = false;

	/** Is player (not AI) */
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayer = false;

	/** Player name */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	/** Vehicle ID */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;
};

/**
 * Race configuration
 */
USTRUCT(BlueprintType)
struct FMGRaceConfig
{
	GENERATED_BODY()

	/** Track ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/** Number of laps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 3;

	/** Countdown duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CountdownDuration = 3.0f;

	/** Max race time (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRaceTime = 0.0f;

	/** AI count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AICount = 7;

	/** AI difficulty (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AIDifficulty = 0.5f;

	/** Enable ghost racing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableGhost = true;

	/** Is ranked match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanked = false;
};

/**
 * Race result
 */
USTRUCT(BlueprintType)
struct FMGFinalRaceResult
{
	GENERATED_BODY()

	/** Player name */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	/** Final position */
	UPROPERTY(BlueprintReadOnly)
	int32 Position = 0;

	/** Total time */
	UPROPERTY(BlueprintReadOnly)
	float TotalTime = 0.0f;

	/** Best lap */
	UPROPERTY(BlueprintReadOnly)
	float BestLap = 0.0f;

	/** All lap times */
	UPROPERTY(BlueprintReadOnly)
	TArray<float> LapTimes;

	/** DNF */
	UPROPERTY(BlueprintReadOnly)
	bool bDNF = false;

	/** Cash earned */
	UPROPERTY(BlueprintReadOnly)
	int32 CashEarned = 0;

	/** Reputation earned */
	UPROPERTY(BlueprintReadOnly)
	int32 ReputationEarned = 0;

	/** Is player */
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayer = false;

	/** Vehicle ID */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRacePhaseChanged, EMGRacePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32, SecondsRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRacerLapCompleted, const FMGRacerState&, Racer, int32, LapNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRacerFinished, const FMGRacerState&, Racer, int32, Position);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPositionsUpdated, const TArray<FMGRacerState>&, Positions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceResultsReady, const TArray<FMGFinalRaceResult>&, Results);

/**
 * Race Game Mode
 * Controls the race lifecycle
 *
 * Features:
 * - Race phases (countdown, racing, finished)
 * - Position tracking
 * - Lap timing
 * - Results calculation
 */
UCLASS()
class MIDNIGHTGRIND_API AMGRaceGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMGRaceGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRacePhaseChanged OnRacePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCountdownTick OnCountdownTick;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarted OnRaceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceFinished OnRaceFinished;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRacerLapCompleted OnRacerLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRacerFinished OnRacerFinished;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPositionsUpdated OnPositionsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceResultsReady OnRaceResultsReady;

	// ==========================================
	// RACE CONTROL
	// ==========================================

	/** Initialize race with config */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void InitializeRace(const FMGRaceConfig& Config);

	/** Start countdown */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void StartCountdown();

	/** Force start race (skip countdown) */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void ForceStartRace();

	/** Abort race */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void AbortRace();

	/** Get current phase */
	UFUNCTION(BlueprintPure, Category = "Race")
	EMGRacePhase GetCurrentPhase() const { return CurrentPhase; }

	/** Get race time */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetRaceTime() const { return RaceTime; }

	/** Get countdown time remaining */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetCountdownRemaining() const { return CountdownRemaining; }

	/** Get race config */
	UFUNCTION(BlueprintPure, Category = "Race")
	FMGRaceConfig GetRaceConfig() const { return RaceConfig; }

	// ==========================================
	// RACER MANAGEMENT
	// ==========================================

	/** Register racer */
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	void RegisterRacer(AController* Controller, APawn* Vehicle, bool bIsPlayer, const FString& PlayerName, FName VehicleID);

	/** Unregister racer */
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	void UnregisterRacer(AController* Controller);

	/** Get all racers */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	TArray<FMGRacerState> GetAllRacers() const { return Racers; }

	/** Get racer by controller */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	FMGRacerState GetRacerState(AController* Controller) const;

	/** Get racer count */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	int32 GetRacerCount() const { return Racers.Num(); }

	/** Get finished racer count */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	int32 GetFinishedRacerCount() const;

	// ==========================================
	// CHECKPOINT HANDLING
	// ==========================================

	/** Report checkpoint crossed */
	UFUNCTION(BlueprintCallable, Category = "Race|Checkpoints")
	void ReportCheckpointCrossed(AController* Controller, int32 CheckpointIndex);

	/** Report finish line crossed */
	UFUNCTION(BlueprintCallable, Category = "Race|Checkpoints")
	void ReportFinishLineCrossed(AController* Controller);

	// ==========================================
	// RESULTS
	// ==========================================

	/** Get final results */
	UFUNCTION(BlueprintPure, Category = "Race|Results")
	TArray<FMGFinalRaceResult> GetFinalResults() const { return FinalResults; }

	/** Get player result */
	UFUNCTION(BlueprintPure, Category = "Race|Results")
	FMGFinalRaceResult GetPlayerResult() const;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Race configuration */
	UPROPERTY(BlueprintReadOnly, Category = "Config")
	FMGRaceConfig RaceConfig;

	/** Position update interval */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float PositionUpdateInterval = 0.1f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current race phase */
	EMGRacePhase CurrentPhase = EMGRacePhase::WaitingForPlayers;

	/** Race time (from start) */
	float RaceTime = 0.0f;

	/** Countdown remaining */
	float CountdownRemaining = 0.0f;

	/** Last countdown tick */
	int32 LastCountdownTick = 0;

	/** Position update accumulator */
	float PositionUpdateAccumulator = 0.0f;

	/** Racers */
	UPROPERTY()
	TArray<FMGRacerState> Racers;

	/** Final results */
	UPROPERTY()
	TArray<FMGFinalRaceResult> FinalResults;

	/** Next finish position */
	int32 NextFinishPosition = 1;

	/** Track subsystem reference */
	UPROPERTY()
	UMGTrackSubsystem* TrackSubsystem;

	/** Replay subsystem reference */
	UPROPERTY()
	UMGReplaySubsystem* ReplaySubsystem;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update countdown */
	void UpdateCountdown(float DeltaTime);

	/** Update race */
	void UpdateRace(float DeltaTime);

	/** Update racer positions */
	void UpdatePositions();

	/** Calculate position score for sorting */
	float CalculatePositionScore(const FMGRacerState& Racer) const;

	/** Set race phase */
	void SetPhase(EMGRacePhase NewPhase);

	/** Start race */
	void StartRace();

	/** End race */
	void EndRace();

	/** Process racer finish */
	void ProcessRacerFinish(FMGRacerState& Racer);

	/** Calculate results */
	void CalculateResults();

	/** Calculate rewards */
	void CalculateRewards(FMGFinalRaceResult& Result);

	/** Find racer index */
	int32 FindRacerIndex(AController* Controller) const;
};

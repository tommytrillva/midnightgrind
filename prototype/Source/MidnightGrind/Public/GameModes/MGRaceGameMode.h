// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MGRaceGameMode.generated.h"

class AMGVehiclePawn;
class AMGCheckpoint;
class AMGRacingHUD;
class UMGRacingHUD;

/**
 * Race state enumeration
 */
UENUM(BlueprintType)
enum class EMGRaceState : uint8
{
	/** Pre-race lobby/setup */
	PreRace			UMETA(DisplayName = "Pre-Race"),

	/** Countdown before race start */
	Countdown		UMETA(DisplayName = "Countdown"),

	/** Race in progress */
	Racing			UMETA(DisplayName = "Racing"),

	/** Race paused */
	Paused			UMETA(DisplayName = "Paused"),

	/** Race finished - showing results */
	Finished		UMETA(DisplayName = "Finished"),

	/** Race aborted/canceled */
	Aborted			UMETA(DisplayName = "Aborted")
};

/**
 * Race type
 */
UENUM(BlueprintType)
enum class EMGRaceType : uint8
{
	/** Standard circuit race - complete X laps */
	Circuit			UMETA(DisplayName = "Circuit"),

	/** Sprint race - point A to B */
	Sprint			UMETA(DisplayName = "Sprint"),

	/** Drift competition - highest score wins */
	Drift			UMETA(DisplayName = "Drift"),

	/** Time trial - beat the clock */
	TimeTrial		UMETA(DisplayName = "Time Trial"),

	/** Drag race - quarter mile */
	Drag			UMETA(DisplayName = "Drag"),

	/** Pink slip - winner takes all */
	PinkSlip		UMETA(DisplayName = "Pink Slip")
};

/**
 * Race configuration
 */
USTRUCT(BlueprintType)
struct FMGRaceConfig
{
	GENERATED_BODY()

	/** Race type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	EMGRaceType RaceType = EMGRaceType::Circuit;

	/** Number of laps (for circuit races) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "1", ClampMax = "99"))
	int32 LapCount = 3;

	/** Time limit in seconds (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float TimeLimit = 0.0f;

	/** Maximum number of racers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "1", ClampMax = "16"))
	int32 MaxRacers = 8;

	/** AI difficulty (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AIDifficulty = 0.5f;

	/** Allow collisions between vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bAllowCollisions = true;

	/** Is this a pink slip race? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bPinkSlipRace = false;

	/** Track/level name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName TrackName;

	/** Night/day setting (0 = night, 1 = day) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TimeOfDay = 0.0f; // Default: Night (MIDNIGHT GRIND!)

	/** Weather condition (0 = clear, 1 = rain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Weather = 0.0f;
};

/**
 * Per-racer data tracked during race
 */
USTRUCT(BlueprintType)
struct FMGRacerData
{
	GENERATED_BODY()

	/** Player/AI index */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 RacerIndex = 0;

	/** Reference to vehicle pawn */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	TWeakObjectPtr<AMGVehiclePawn> Vehicle;

	/** Current lap number */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 CurrentLap = 0;

	/** Current checkpoint index */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 CurrentCheckpoint = 0;

	/** Race position (1st, 2nd, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 Position = 0;

	/** Total distance traveled (for position calculation) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float TotalDistance = 0.0f;

	/** Current lap time */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float CurrentLapTime = 0.0f;

	/** Best lap time */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float BestLapTime = 0.0f;

	/** Total race time */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float TotalTime = 0.0f;

	/** All lap times */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	TArray<float> LapTimes;

	/** Has finished the race? */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	bool bFinished = false;

	/** Finish time */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float FinishTime = 0.0f;

	/** Did not finish? */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	bool bDNF = false;

	/** Is this an AI racer? */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	bool bIsAI = false;

	/** Drift score (for drift races) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float DriftScore = 0.0f;

	/** Player display name */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	FText DisplayName;
};

/**
 * Race results
 */
USTRUCT(BlueprintType)
struct FMGRaceResults
{
	GENERATED_BODY()

	/** Race configuration used */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FMGRaceConfig Config;

	/** All racer results, sorted by position */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<FMGRacerData> RacerResults;

	/** Best overall lap time */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float BestLapTime = 0.0f;

	/** Who got best lap */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 BestLapRacerIndex = -1;

	/** Race duration */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float TotalRaceTime = 0.0f;

	/** Credits earned by player (position 0) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int64 CreditsEarned = 0;

	/** Reputation earned */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 ReputationEarned = 0;

	/** Did player win? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bPlayerWon = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceStateChanged, EMGRaceState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32, SecondsRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLapCompleted, int32, RacerIndex, float, LapTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRacerFinished, int32, RacerIndex, int32, Position);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFinished, const FMGRaceResults&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPositionChanged, int32, RacerIndex, int32, NewPosition);

/**
 * Race game mode - manages the entire race flow
 * Handles countdown, timing, positions, and results
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGRaceGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMGRaceGameMode();

	//~ Begin AGameModeBase Interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	//~ End AGameModeBase Interface

	// ==========================================
	// RACE CONFIGURATION
	// ==========================================

	/** Set race configuration */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void SetRaceConfig(const FMGRaceConfig& Config);

	/** Get race configuration */
	UFUNCTION(BlueprintPure, Category = "Race")
	FMGRaceConfig GetRaceConfig() const { return RaceConfig; }

	/** Get current race state */
	UFUNCTION(BlueprintPure, Category = "Race")
	EMGRaceState GetRaceState() const { return CurrentRaceState; }

	// ==========================================
	// RACE CONTROL
	// ==========================================

	/** Start the race countdown */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void StartCountdown();

	/** Abort countdown and return to pre-race */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void AbortCountdown();

	/** Pause the race */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void PauseRace();

	/** Resume the race */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void ResumeRace();

	/** End the race immediately */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void EndRace();

	/** Restart the race */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void RestartRace();

	// ==========================================
	// RACER MANAGEMENT
	// ==========================================

	/** Register a vehicle as a racer */
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	int32 RegisterRacer(AMGVehiclePawn* Vehicle, bool bIsAI = false, FText DisplayName = FText());

	/** Unregister a racer */
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	void UnregisterRacer(int32 RacerIndex);

	/** Get racer data */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	FMGRacerData GetRacerData(int32 RacerIndex) const;

	/** Get all racers */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	TArray<FMGRacerData> GetAllRacers() const { return Racers; }

	/** Get racer count */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	int32 GetRacerCount() const { return Racers.Num(); }

	/** Get player's racer index */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	int32 GetPlayerRacerIndex() const { return PlayerRacerIndex; }

	// ==========================================
	// CHECKPOINT/LAP TRACKING
	// ==========================================

	/** Register a checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Race|Checkpoints")
	void RegisterCheckpoint(AMGCheckpoint* Checkpoint, int32 CheckpointIndex);

	/** Called when a racer passes a checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Race|Checkpoints")
	void OnCheckpointPassed(AMGVehiclePawn* Vehicle, int32 CheckpointIndex);

	/** Get total checkpoint count */
	UFUNCTION(BlueprintPure, Category = "Race|Checkpoints")
	int32 GetCheckpointCount() const { return Checkpoints.Num(); }

	// ==========================================
	// TIMING & POSITIONS
	// ==========================================

	/** Get current race time */
	UFUNCTION(BlueprintPure, Category = "Race|Timing")
	float GetRaceTime() const { return RaceTime; }

	/** Get countdown time remaining */
	UFUNCTION(BlueprintPure, Category = "Race|Timing")
	int32 GetCountdownSeconds() const { return CountdownSeconds; }

	/** Get race results (after race ends) */
	UFUNCTION(BlueprintPure, Category = "Race|Results")
	FMGRaceResults GetRaceResults() const { return RaceResults; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when race state changes */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceStateChanged OnRaceStateChanged;

	/** Called each countdown tick */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnCountdownTick OnCountdownTick;

	/** Called when race starts (GO!) */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceStarted OnRaceStarted;

	/** Called when a racer completes a lap */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnLapCompleted OnLapCompleted;

	/** Called when a racer finishes */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRacerFinished OnRacerFinished;

	/** Called when race ends (all finished or time up) */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceFinished OnRaceFinished;

	/** Called when position changes */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnPositionChanged OnPositionChanged;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Race configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Configuration")
	FMGRaceConfig RaceConfig;

	/** Countdown duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Configuration")
	int32 CountdownDuration = 3;

	/** Position update frequency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Configuration")
	float PositionUpdateRate = 0.1f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current race state */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	EMGRaceState CurrentRaceState = EMGRaceState::PreRace;

	/** All registered racers */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	TArray<FMGRacerData> Racers;

	/** All checkpoints */
	UPROPERTY()
	TArray<TWeakObjectPtr<AMGCheckpoint>> Checkpoints;

	/** Current race time */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	float RaceTime = 0.0f;

	/** Countdown seconds remaining */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	int32 CountdownSeconds = 0;

	/** Race results */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	FMGRaceResults RaceResults;

	/** Player's racer index */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	int32 PlayerRacerIndex = 0;

	/** Number of racers finished */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	int32 FinishedCount = 0;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Set new race state */
	void SetRaceState(EMGRaceState NewState);

	/** Update countdown */
	void UpdateCountdown(float DeltaTime);

	/** Update race timing */
	void UpdateRaceTiming(float DeltaTime);

	/** Update positions */
	void UpdatePositions();

	/** Check if race is complete */
	void CheckRaceComplete();

	/** Calculate final results */
	void CalculateResults();

	/** Calculate credits earned based on position */
	int64 CalculateCreditsEarned(int32 Position, bool bWon) const;

	/** Get racer index for a vehicle */
	int32 GetRacerIndexForVehicle(AMGVehiclePawn* Vehicle) const;

	/** Freeze all vehicles (during countdown) */
	void FreezeAllVehicles(bool bFreeze);

	/** Notify all player controllers that race started */
	void NotifyPlayersRaceStarted();

	/** Notify all player controllers that race ended */
	void NotifyPlayersRaceEnded();

	/** Update HUD subsystem with player race data */
	void UpdateHUDSubsystem();

	/** Notify RaceFlowManager for reward processing */
	void NotifyRaceFlowManager();

private:
	/** Countdown accumulator */
	float CountdownAccumulator = 0.0f;

	/** Position update accumulator */
	float PositionUpdateAccumulator = 0.0f;
};

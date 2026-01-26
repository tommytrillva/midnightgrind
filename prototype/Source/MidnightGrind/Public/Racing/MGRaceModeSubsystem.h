// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRaceModeSubsystem.generated.h"

class AMGVehiclePawn;
class AMGCheckpoint;
class UMGVehicleDefinition;

/**
 * Race type
 */
UENUM(BlueprintType)
enum class EMGRaceType : uint8
{
	Circuit UMETA(DisplayName = "Circuit"),
	Sprint UMETA(DisplayName = "Sprint (Point-to-Point)"),
	Drag UMETA(DisplayName = "Drag Race"),
	Drift UMETA(DisplayName = "Drift"),
	TimeAttack UMETA(DisplayName = "Time Attack"),
	Elimination UMETA(DisplayName = "Elimination"),
	PinkSlip UMETA(DisplayName = "Pink Slip"),
	Touge UMETA(DisplayName = "Touge (Mountain Pass)"),
	Knockout UMETA(DisplayName = "Knockout"),
	Checkpoint UMETA(DisplayName = "Checkpoint Rush"),
	FreeRoam UMETA(DisplayName = "Free Roam")
};

/**
 * Race difficulty
 */
UENUM(BlueprintType)
enum class EMGRaceDifficulty : uint8
{
	Easy UMETA(DisplayName = "Easy"),
	Medium UMETA(DisplayName = "Medium"),
	Hard UMETA(DisplayName = "Hard"),
	Expert UMETA(DisplayName = "Expert"),
	Legendary UMETA(DisplayName = "Legendary")
};

/**
 * Race state
 */
UENUM(BlueprintType)
enum class EMGRaceState : uint8
{
	None UMETA(DisplayName = "None"),
	Setup UMETA(DisplayName = "Setup"),
	Countdown UMETA(DisplayName = "Countdown"),
	Racing UMETA(DisplayName = "Racing"),
	Finished UMETA(DisplayName = "Finished"),
	DNF UMETA(DisplayName = "DNF"),
	Cancelled UMETA(DisplayName = "Cancelled")
};

/**
 * Traffic density setting
 */
UENUM(BlueprintType)
enum class EMGTrafficDensity : uint8
{
	None UMETA(DisplayName = "No Traffic"),
	Light UMETA(DisplayName = "Light Traffic"),
	Medium UMETA(DisplayName = "Medium Traffic"),
	Heavy UMETA(DisplayName = "Heavy Traffic"),
	Rush UMETA(DisplayName = "Rush Hour")
};

/**
 * Performance class for matchmaking/restrictions
 */
UENUM(BlueprintType)
enum class EMGPerformanceClass : uint8
{
	D UMETA(DisplayName = "D Class (0-299 PI)"),
	C UMETA(DisplayName = "C Class (300-399 PI)"),
	B UMETA(DisplayName = "B Class (400-499 PI)"),
	A UMETA(DisplayName = "A Class (500-599 PI)"),
	S UMETA(DisplayName = "S Class (600-699 PI)"),
	S1 UMETA(DisplayName = "S1 Class (700-799 PI)"),
	S2 UMETA(DisplayName = "S2 Class (800-899 PI)"),
	X UMETA(DisplayName = "X Class (900+ PI)"),
	Open UMETA(DisplayName = "Open Class (No Limit)")
};

/**
 * Racer entry (player or AI)
 */
USTRUCT(BlueprintType)
struct FMGRacerEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	FGuid RacerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	FString RacerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	bool bIsPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	bool bIsAI = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	TWeakObjectPtr<AMGVehiclePawn> Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	int32 PerformanceIndex = 400;

	// Race progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentCheckpoint = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float TotalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float RaceProgress = 0.0f; // 0-1 total race progress

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bFinished = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bDNF = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bEliminated = false;

	// Timing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	TArray<float> LapTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	TArray<float> SectorTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float FinishTime = 0.0f;

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TopSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 PerfectShifts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DriftScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 NearMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Collisions = 0;
};

/**
 * Race configuration
 */
USTRUCT(BlueprintType)
struct FMGRaceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName RaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString RaceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EMGRaceType RaceType = EMGRaceType::Circuit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EMGRaceDifficulty Difficulty = EMGRaceDifficulty::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EMGPerformanceClass PerformanceClass = EMGPerformanceClass::Open;

	// Track
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TSoftObjectPtr<UWorld> TrackLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	int32 NumLaps = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float TrackLength = 5000.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	bool bIsReversed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	bool bIsNightRace = true;

	// Opponents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Opponents")
	int32 NumOpponents = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Opponents")
	float AISkillLevel = 0.5f; // 0-1

	// Conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	EMGTrafficDensity TrafficDensity = EMGTrafficDensity::Light;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	bool bPoliceEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	bool bCatchupEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	bool bCollisionDamageEnabled = true;

	// Rewards
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 PrizeMoney1st = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 PrizeMoney2nd = 2500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 PrizeMoney3rd = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 REPReward = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 BuyIn = 0; // Entry fee

	// Pink Slip specific
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	bool bIsPinkSlipRace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	FName PinkSlipVehicleID; // Vehicle at stake

	// Time limits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TimeLimit = 0.0f; // 0 = no limit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float EliminationInterval = 30.0f; // For elimination races
};

/**
 * Race result
 */
USTRUCT(BlueprintType)
struct FMGRaceResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FMGRaceConfig RaceConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FMGRacerEntry> FinalStandings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 PlayerFinishPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPlayerWon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPlayerDNF = false;

	// Rewards earned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 CashEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 REPEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	bool bWonPinkSlip = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName PinkSlipVehicleWon;

	// Timing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float PlayerBestLap = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float PlayerTotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TrackRecordTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	bool bNewTrackRecord = false;

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float PlayerTopSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TotalDriftScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalNearMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalPerfectShifts = 0;
};

/**
 * Checkpoint data
 */
USTRUCT(BlueprintType)
struct FMGCheckpointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 CheckpointIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float DistanceFromStart = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsStartFinish = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsSectorMarker = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 SectorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	TWeakObjectPtr<AMGCheckpoint> CheckpointActor;
};

/**
 * Track record
 */
USTRUCT(BlueprintType)
struct FMGTrackRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	EMGRaceType RaceType = EMGRaceType::Circuit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	float BestTotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	FString RecordHolderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	FName VehicleUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	FDateTime RecordDate;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceStateChanged, EMGRaceState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32, SecondsRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFinished, const FMGRaceResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLapCompleted, const FGuid&, RacerID, int32, LapNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointPassed, const FGuid&, RacerID, int32, CheckpointIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPositionChanged, const FGuid&, RacerID, int32, NewPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRacerEliminated, const FGuid&, RacerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRacerFinished, const FMGRacerEntry&, Racer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewTrackRecord, const FMGTrackRecord&, Record);

/**
 * Race Mode Subsystem
 *
 * Manages all race types, scoring, timing, and race progression.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGRaceModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// DELEGATES
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStateChanged OnRaceStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCountdownTick OnCountdownTick;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarted OnRaceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceFinished OnRaceFinished;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLapCompleted OnLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCheckpointPassed OnCheckpointPassed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPositionChanged OnPositionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRacerEliminated OnRacerEliminated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRacerFinished OnRacerFinished;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNewTrackRecord OnNewTrackRecord;

	// ==========================================
	// RACE SETUP
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	bool SetupRace(const FMGRaceConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	bool AddRacer(const FMGRacerEntry& Racer);

	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	bool RemoveRacer(const FGuid& RacerID);

	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	void ClearRacers();

	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	void RegisterCheckpoint(const FMGCheckpointData& Checkpoint);

	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	void ClearCheckpoints();

	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	bool ValidateRaceSetup() const;

	// ==========================================
	// RACE CONTROL
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	bool StartCountdown();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void StartRace();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void PauseRace();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void ResumeRace();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void FinishRace();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void CancelRace();

	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void RestartRace();

	// ==========================================
	// RACE STATE
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Race State")
	EMGRaceState GetRaceState() const { return CurrentRaceState; }

	UFUNCTION(BlueprintPure, Category = "Race State")
	bool IsRaceInProgress() const { return CurrentRaceState == EMGRaceState::Racing; }

	UFUNCTION(BlueprintPure, Category = "Race State")
	const FMGRaceConfig& GetCurrentRaceConfig() const { return CurrentRaceConfig; }

	UFUNCTION(BlueprintPure, Category = "Race State")
	float GetRaceTime() const { return RaceTime; }

	UFUNCTION(BlueprintPure, Category = "Race State")
	int32 GetCountdownSeconds() const { return CountdownSeconds; }

	// ==========================================
	// RACER PROGRESS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RacerPassedCheckpoint(const FGuid& RacerID, int32 CheckpointIndex);

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RacerCompletedLap(const FGuid& RacerID);

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RacerFinished(const FGuid& RacerID);

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RacerDNF(const FGuid& RacerID);

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void EliminateRacer(const FGuid& RacerID);

	UFUNCTION(BlueprintPure, Category = "Progress")
	FMGRacerEntry GetRacerData(const FGuid& RacerID) const;

	UFUNCTION(BlueprintPure, Category = "Progress")
	TArray<FMGRacerEntry> GetCurrentStandings() const;

	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetRacerPosition(const FGuid& RacerID) const;

	// ==========================================
	// PLAYER SPECIFIC
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerRacer(const FGuid& RacerID);

	UFUNCTION(BlueprintPure, Category = "Player")
	FMGRacerEntry GetPlayerRacerData() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	int32 GetPlayerPosition() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	int32 GetPlayerCurrentLap() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetPlayerBestLap() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetGapToLeader() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetGapToRacerAhead() const;

	UFUNCTION(BlueprintPure, Category = "Player")
	float GetGapToRacerBehind() const;

	// ==========================================
	// TIMING
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Timing")
	void UpdateRacerTime(const FGuid& RacerID, float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Timing")
	float GetRacerLapTime(const FGuid& RacerID) const;

	UFUNCTION(BlueprintPure, Category = "Timing")
	float GetRacerBestLap(const FGuid& RacerID) const;

	UFUNCTION(BlueprintPure, Category = "Timing")
	float GetTrackRecordLap(FName TrackID, EMGRaceType RaceType) const;

	// ==========================================
	// DRIFT SCORING (for drift races)
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Drift")
	void AddDriftScore(const FGuid& RacerID, float Score);

	UFUNCTION(BlueprintPure, Category = "Drift")
	float GetRacerDriftScore(const FGuid& RacerID) const;

	UFUNCTION(BlueprintPure, Category = "Drift")
	TArray<FMGRacerEntry> GetDriftStandings() const;

	// ==========================================
	// DRAG RACING
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Drag")
	void RecordReactionTime(const FGuid& RacerID, float ReactionTime);

	UFUNCTION(BlueprintCallable, Category = "Drag")
	void RecordDragSplit(const FGuid& RacerID, float Distance, float Time);

	UFUNCTION(BlueprintPure, Category = "Drag")
	float GetRacerReactionTime(const FGuid& RacerID) const;

	// ==========================================
	// UTILITIES
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Utilities")
	EMGPerformanceClass GetClassForPI(int32 PerformanceIndex) const;

	UFUNCTION(BlueprintPure, Category = "Utilities")
	FString GetClassDisplayName(EMGPerformanceClass Class) const;

	UFUNCTION(BlueprintPure, Category = "Utilities")
	int64 CalculateRewards(int32 Position, const FMGRaceConfig& Config) const;

	UFUNCTION(BlueprintPure, Category = "Utilities")
	int32 CalculateREPReward(int32 Position, const FMGRaceConfig& Config) const;

	// ==========================================
	// RECORDS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Records")
	void SaveTrackRecord(const FMGTrackRecord& Record);

	UFUNCTION(BlueprintPure, Category = "Records")
	FMGTrackRecord GetTrackRecord(FName TrackID, EMGRaceType RaceType) const;

	UFUNCTION(BlueprintPure, Category = "Records")
	TArray<FMGTrackRecord> GetAllRecordsForTrack(FName TrackID) const;

protected:
	void UpdatePositions();
	void UpdateEliminationTimer(float DeltaTime);
	void CheckRaceCompletion();
	FMGRaceResult GenerateRaceResult();
	void SetRaceState(EMGRaceState NewState);

private:
	// Timer handles
	FTimerHandle UpdateTimerHandle;
	FTimerHandle CountdownTimerHandle;
	FTimerHandle EliminationTimerHandle;

	// Current race state
	UPROPERTY()
	EMGRaceState CurrentRaceState = EMGRaceState::None;

	UPROPERTY()
	FMGRaceConfig CurrentRaceConfig;

	UPROPERTY()
	TArray<FMGRacerEntry> Racers;

	UPROPERTY()
	TArray<FMGCheckpointData> Checkpoints;

	UPROPERTY()
	FGuid PlayerRacerID;

	// Timing
	UPROPERTY()
	float RaceTime = 0.0f;

	UPROPERTY()
	int32 CountdownSeconds = 3;

	UPROPERTY()
	bool bRacePaused = false;

	// Elimination
	UPROPERTY()
	float EliminationTimer = 0.0f;

	UPROPERTY()
	int32 EliminationCount = 0;

	// Drag racing
	UPROPERTY()
	TMap<FGuid, float> ReactionTimes;

	UPROPERTY()
	TMap<FGuid, TArray<float>> DragSplits;

	// Track records
	UPROPERTY()
	TArray<FMGTrackRecord> TrackRecords;

	// Result cache
	UPROPERTY()
	FMGRaceResult LastRaceResult;

	// Position tracking
	UPROPERTY()
	int32 NextFinishPosition = 1;
};

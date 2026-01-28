// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTimeAttackSubsystem.h
 * @brief Time Attack System - Solo racing mode focused on beating lap times and chasing ghosts
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 * ==========================
 * Time Attack is a classic racing game mode where players compete against the clock rather
 * than other racers. This system tracks lap times, manages ghost replays, handles time
 * trials, and calculates real-time deltas (time differences from your best lap).
 *
 * If you've played games like Gran Turismo, TrackMania, or any racing game with a
 * "time trial" mode, this system provides similar functionality.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * --------------------------
 *
 * 1. WHAT IS A "DELTA"?
 *    - Delta means "difference" - in racing, it's how much faster/slower you are compared
 *      to a reference time.
 *    - A delta of -0.5 means you're half a second FASTER than your best time.
 *    - A delta of +1.2 means you're 1.2 seconds SLOWER than your best time.
 *    - The FMGDeltaInfo struct tracks deltas against personal best, session best, and ghosts.
 *
 * 2. WHAT IS A "SECTOR"?
 *    - Tracks are divided into sectors (usually 3) - checkpoints that split the lap.
 *    - Sector times help identify which parts of the track you're gaining or losing time.
 *    - You might have a great Sector 1 but struggle in Sector 3 - this data helps improvement.
 *
 * 3. WHAT IS A "GHOST"?
 *    - A ghost is a recorded replay of a previous lap that appears as a semi-transparent car.
 *    - You can race against your personal best ghost, friends' ghosts, or leaderboard ghosts.
 *    - Ghosts help you see the optimal racing line and braking points.
 *    - FMGGhostData stores the metadata; the actual position/rotation data is stored separately.
 *
 * 4. WHAT IS "THEORETICAL BEST"?
 *    - Your theoretical best is calculated by combining your best sector times.
 *    - If your best Sector 1 is 20.5s, best Sector 2 is 18.2s, and best Sector 3 is 22.1s,
 *      your theoretical best is 60.8s, even if your actual best lap is 61.5s.
 *    - This shows the "perfect lap" you're capable of if you nail every sector.
 *
 * 5. WHAT ARE "TRIALS"?
 *    - Trials are structured challenges with medal targets (Bronze, Silver, Gold, etc.).
 *    - Unlike free Time Attack, trials may have specific requirements (certain car, conditions).
 *    - Medals provide progression goals and rewards.
 *
 * 6. TIME ATTACK MODES:
 *    - SingleLap: Best single lap time (most common)
 *    - FullRace: Best time across multiple laps (e.g., best 3-lap time)
 *    - Sector: Challenge specific track sectors only
 *    - Checkpoint: Point-to-point time between checkpoints
 *    - Endurance: How many laps in a time limit
 *
 * SYSTEM ARCHITECTURE:
 * -------------------
 * The system manages several interconnected features:
 *
 * - SESSION: Current active Time Attack session with real-time timing
 * - RECORDS: Historical best times stored per track/vehicle combination
 * - GHOSTS: Replay data for visualization and competition
 * - TRIALS: Structured challenges with medal thresholds
 * - STATISTICS: Lifetime stats like total laps, personal bests set, etc.
 *
 * DATA FLOW EXAMPLE (Single Lap Time Attack):
 * ------------------------------------------
 * 1. Player calls StartSession(TrackID, VehicleID, SingleLap)
 * 2. System loads personal bests and selected ghosts
 * 3. OnSessionStarted delegate fires
 * 4. As player races, UpdateCurrentTime() is called every frame
 * 5. OnCrossedSector() is called at each sector checkpoint
 * 6. OnDeltaUpdated fires with real-time comparison data
 * 7. OnCrossedStartLine() completes the lap
 * 8. ProcessLapCompletion() checks for new records
 * 9. OnLapCompleted and potentially OnNewPersonalBest fire
 *
 * IMPORTANT FUNCTIONS:
 * -------------------
 * - StartSession/EndSession: Begin/end a Time Attack session
 * - OnCrossedStartLine/OnCrossedSector: Called by track checkpoints
 * - GetCurrentDelta: Get real-time delta info for UI display
 * - LoadGhost/SelectGhostsForSession: Manage ghost opponents
 * - StartTrial/EndTrial: Handle structured trial challenges
 *
 * @see FMGLapTime - Structure holding detailed lap data
 * @see FMGGhostData - Ghost replay metadata
 * @see FMGTrialDefinition - Trial challenge configuration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTimeAttackSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGTimeAttackMode : uint8
{
	SingleLap,
	FullRace,
	Sector,
	Checkpoint,
	Endurance
};

UENUM(BlueprintType)
enum class EMGTrialType : uint8
{
	Speed,
	Drift,
	Precision,
	Technical,
	Mixed
};

UENUM(BlueprintType)
enum class EMGTrialMedal : uint8
{
	None,
	Bronze,
	Silver,
	Gold,
	Platinum,
	Diamond
};

UENUM(BlueprintType)
enum class EMGGhostType : uint8
{
	Personal,
	Friend,
	Leaderboard,
	Developer,
	WorldRecord
};

USTRUCT(BlueprintType)
struct FMGLapTime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SectorTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectCorners = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DriftCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsValid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBest = false;
};

USTRUCT(BlueprintType)
struct FMGTimeAttackRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RecordID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeAttackMode Mode = EMGTimeAttackMode::SingleLap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> BestSectorTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TheoreticalBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLapTime> LapHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SetAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalAttempts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimePlayed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;
};

USTRUCT(BlueprintType)
struct FMGGhostData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid GhostID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VehicleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGhostType GhostType = EMGGhostType::Personal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OwnerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OwnerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RecordedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LeaderboardRank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLoaded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GhostColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMGTrialDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrialID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TrialName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TrialDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrialType TrialType = EMGTrialType::Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BronzeTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SilverTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoldTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlatinumTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiamondTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNightConditions = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeatherCondition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> TrialIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;
};

USTRUCT(BlueprintType)
struct FMGTrialProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrialID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrialMedal BestMedal = EMGTrialMedal::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Attempts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastAttempt;
};

USTRUCT(BlueprintType)
struct FMGTimeAttackSession
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeAttackMode Mode = EMGTimeAttackMode::SingleLap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SessionBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PersonalBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLapTime> SessionLaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSector = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Delta = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGGhostData> ActiveGhosts;
};

USTRUCT(BlueprintType)
struct FMGDeltaInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaToPersonalBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaToSessionBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaToTheoreticalBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> DeltasToGhosts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsImproving = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PredictedLapTime = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLapCompleted, const FMGLapTime&, LapTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSectorCompleted, int32, SectorIndex, float, SectorTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewPersonalBest, FName, TrackID, float, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeltaUpdated, const FMGDeltaInfo&, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrialCompleted, FName, TrialID, EMGTrialMedal, Medal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGhostLoaded, const FMGGhostData&, Ghost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionEnded, const FMGTimeAttackSession&, Session);

UCLASS()
class MIDNIGHTGRIND_API UMGTimeAttackSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Session")
	bool StartSession(FName TrackID, FName VehicleID, EMGTimeAttackMode Mode);

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Session")
	void EndSession();

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Session")
	void RestartLap();

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Session")
	bool IsInSession() const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Session")
	FMGTimeAttackSession GetCurrentSession() const { return CurrentSession; }

	// Timing
	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Timing")
	void OnCrossedStartLine();

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Timing")
	void OnCrossedSector(int32 SectorIndex);

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Timing")
	void OnCrossedCheckpoint(int32 CheckpointIndex, float Time);

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Timing")
	void UpdateCurrentTime(float Time);

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Timing")
	void InvalidateLap();

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Timing")
	float GetCurrentLapTime() const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Timing")
	FMGDeltaInfo GetCurrentDelta() const;

	// Records
	UFUNCTION(BlueprintPure, Category = "TimeAttack|Records")
	FMGTimeAttackRecord GetPersonalBest(FName TrackID, FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Records")
	TArray<FMGTimeAttackRecord> GetAllRecords() const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Records")
	TArray<FMGTimeAttackRecord> GetRecordsForTrack(FName TrackID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Records")
	float GetTheoreticalBest(FName TrackID, FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Records")
	TArray<float> GetBestSectorTimes(FName TrackID, FName VehicleID) const;

	// Ghosts
	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Ghosts")
	void LoadGhost(FGuid GhostID);

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Ghosts")
	void UnloadGhost(FGuid GhostID);

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Ghosts")
	void SelectGhostsForSession(const TArray<FGuid>& GhostIDs);

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Ghosts")
	TArray<FMGGhostData> GetAvailableGhosts(FName TrackID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Ghosts")
	TArray<FMGGhostData> GetPersonalGhosts(FName TrackID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Ghosts")
	TArray<FMGGhostData> GetFriendGhosts(FName TrackID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Ghosts")
	TArray<FMGGhostData> GetLeaderboardGhosts(FName TrackID, int32 Count = 10) const;

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Ghosts")
	void SaveGhost(const FMGGhostData& Ghost);

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Ghosts")
	void DeleteGhost(FGuid GhostID);

	// Trials
	UFUNCTION(BlueprintPure, Category = "TimeAttack|Trials")
	TArray<FMGTrialDefinition> GetAllTrials() const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Trials")
	TArray<FMGTrialDefinition> GetTrialsByType(EMGTrialType Type) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Trials")
	TArray<FMGTrialDefinition> GetTrialsForTrack(FName TrackID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Trials")
	FMGTrialDefinition GetTrial(FName TrialID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Trials")
	FMGTrialProgress GetTrialProgress(FName TrialID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Trials")
	EMGTrialMedal GetMedalForTime(FName TrialID, float Time) const;

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Trials")
	bool StartTrial(FName TrialID);

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Trials")
	void EndTrial(float FinalTime);

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Trials")
	int32 GetTotalMedals(EMGTrialMedal MinMedal = EMGTrialMedal::Bronze) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Trials")
	float GetTrialCompletionPercent() const;

	// Comparison
	UFUNCTION(BlueprintPure, Category = "TimeAttack|Compare")
	float GetDeltaToRecord(FName TrackID, FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Compare")
	TArray<float> GetSectorDeltas(FName TrackID, FName VehicleID) const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "TimeAttack|Stats")
	int32 GetTotalLapsCompleted() const { return TotalLapsCompleted; }

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Stats")
	float GetTotalTimeAttackTime() const { return TotalTimeAttackTime; }

	UFUNCTION(BlueprintPure, Category = "TimeAttack|Stats")
	int32 GetPersonalBestCount() const { return PersonalBestsSet; }

	// Persistence
	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Save")
	void SaveTimeAttackData();

	UFUNCTION(BlueprintCallable, Category = "TimeAttack|Save")
	void LoadTimeAttackData();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "TimeAttack|Events")
	FOnLapCompleted OnLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "TimeAttack|Events")
	FOnSectorCompleted OnSectorCompleted;

	UPROPERTY(BlueprintAssignable, Category = "TimeAttack|Events")
	FOnNewPersonalBest OnNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "TimeAttack|Events")
	FOnDeltaUpdated OnDeltaUpdated;

	UPROPERTY(BlueprintAssignable, Category = "TimeAttack|Events")
	FOnTrialCompleted OnTrialCompleted;

	UPROPERTY(BlueprintAssignable, Category = "TimeAttack|Events")
	FOnGhostLoaded OnGhostLoaded;

	UPROPERTY(BlueprintAssignable, Category = "TimeAttack|Events")
	FOnSessionStarted OnSessionStarted;

	UPROPERTY(BlueprintAssignable, Category = "TimeAttack|Events")
	FOnSessionEnded OnSessionEnded;

protected:
	void UpdateDelta();
	void ProcessLapCompletion(FMGLapTime& Lap);
	void UpdateRecords(const FMGLapTime& Lap);
	void CalculateTheoreticalBest(FMGTimeAttackRecord& Record);
	void InitializeDefaultTrials();
	FName MakeRecordKey(FName TrackID, FName VehicleID) const;

	UPROPERTY()
	FMGTimeAttackSession CurrentSession;

	UPROPERTY()
	TMap<FName, FMGTimeAttackRecord> AllRecords;

	UPROPERTY()
	TArray<FMGGhostData> AllGhosts;

	UPROPERTY()
	TMap<FName, FMGTrialDefinition> AllTrials;

	UPROPERTY()
	TMap<FName, FMGTrialProgress> TrialProgress;

	UPROPERTY()
	FName ActiveTrialID;

	UPROPERTY()
	FMGLapTime CurrentLap;

	UPROPERTY()
	FMGDeltaInfo CurrentDelta;

	UPROPERTY()
	int32 TotalLapsCompleted = 0;

	UPROPERTY()
	float TotalTimeAttackTime = 0.0f;

	UPROPERTY()
	int32 PersonalBestsSet = 0;

	UPROPERTY()
	FName LocalPlayerID;

	UPROPERTY()
	FString LocalPlayerName;
};

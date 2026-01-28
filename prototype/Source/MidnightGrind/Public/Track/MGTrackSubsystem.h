// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTrackSubsystem.h
 * @brief Central track management subsystem for race timing, positions, and progress.
 *
 * This file defines UMGTrackSubsystem, the core system responsible for managing all
 * track-related data during a race. It handles checkpoint validation, lap counting,
 * sector timing, race positions, wrong-way detection, and track records.
 *
 * @section subsystem_concepts Key Concepts
 *
 * WORLD SUBSYSTEM: A UWorldSubsystem is an Unreal Engine singleton that exists
 * once per game world. It is automatically created when the world loads and
 * destroyed when the world unloads. Access it via GetWorld()->GetSubsystem<UMGTrackSubsystem>().
 *
 * CHECKPOINT VALIDATION: Racers must pass checkpoints in the correct order.
 * Skipping a checkpoint (taking a shortcut that bypasses it) invalidates the lap.
 * The subsystem tracks which checkpoint each racer should pass next.
 *
 * LAP TIMING: The subsystem records when racers cross each checkpoint and the
 * finish line. Lap times are calculated as the time between consecutive finish
 * line crossings (after passing all checkpoints).
 *
 * SECTOR TIMING: Tracks are divided into sectors (typically 3). The subsystem
 * records sector times for detailed performance analysis. Personal best sectors
 * are tracked independently of lap times.
 *
 * RACE POSITION: The subsystem calculates each racer's position based on:
 * - Number of laps completed
 * - Number of checkpoints passed
 * - Distance traveled within current segment
 *
 * @section subsystem_architecture Architecture
 *
 * The subsystem is the central hub for track-related information:
 *
 * +------------------+
 * | Track Subsystem  |<----+---- Checkpoint Actors (register & trigger)
 * |                  |     |
 * | - Checkpoints[]  |<----+---- Racing Line Actor (distance queries)
 * | - RacerProgress[]|     |
 * | - TrackConfig    |<----+---- Race Game Mode (init, start/stop)
 * | - RaceTime       |     |
 * +------------------+-----+---- Vehicle Pawns (position updates)
 *         |
 *         v
 *   Events: OnCheckpointPassed, OnLapCompleted, OnPositionChanged, etc.
 *
 * @section subsystem_usage Usage Examples
 *
 * @code
 * // Accessing the subsystem
 * UMGTrackSubsystem* TrackSub = GetWorld()->GetSubsystem<UMGTrackSubsystem>();
 *
 * // Initialize track from data asset
 * TrackSub->InitializeTrack(MyTrackDataAsset);
 *
 * // Register racers at race start
 * for (int32 i = 0; i < NumRacers; ++i)
 * {
 *     TrackSub->RegisterRacer(i, RacerActors[i]);
 * }
 *
 * // Start the race timer
 * TrackSub->StartRaceTimer();
 *
 * // Query race positions during the race
 * int32 PlayerPosition = TrackSub->GetRacerPosition(PlayerRacerID);
 * float GapToLeader = TrackSub->GetGapToRacer(PlayerRacerID, 0); // Gap to first place
 *
 * // Get detailed progress for HUD display
 * FMGRacerProgress Progress;
 * if (TrackSub->GetRacerProgress(PlayerRacerID, Progress))
 * {
 *     DisplayLap(Progress.CurrentLap);
 *     DisplaySectorTimes(Progress.CurrentSectorTimes);
 *     if (Progress.bWrongWay)
 *     {
 *         ShowWrongWayWarning();
 *     }
 * }
 *
 * // Listen to track events
 * TrackSub->OnLapCompleted.AddDynamic(this, &AMyClass::HandleLapCompleted);
 * TrackSub->OnPositionChanged.AddDynamic(this, &AMyClass::HandlePositionChanged);
 *
 * void AMyClass::HandleLapCompleted(int32 RacerID, int32 LapNumber, float LapTime)
 * {
 *     if (RacerID == PlayerRacerID)
 *     {
 *         DisplayLapTime(LapTime);
 *         if (LapNumber == TotalLaps)
 *         {
 *             // Player finished the race
 *         }
 *     }
 * }
 * @endcode
 *
 * @section subsystem_wrongway Wrong Way Detection
 *
 * The subsystem detects when racers are driving the wrong way:
 * - Compares vehicle velocity direction to track direction
 * - Broadcasts OnWrongWayChanged when status changes
 * - UI can display "WRONG WAY" warnings
 *
 * @section subsystem_records Track Records
 *
 * The subsystem tracks the best lap time on the track:
 * - Compares each completed lap to the current record
 * - Broadcasts OnNewTrackRecord when a record is broken
 * - Records persist via the track data asset
 *
 * @section subsystem_structs Key Data Structures
 *
 * FMGCheckpointData: Information about a checkpoint (position, width, sector info)
 * FMGRacerProgress: Complete progress data for a racer (lap, checkpoints, times)
 * FMGTrackConfig: Track configuration (name, length, number of sectors)
 *
 * @section subsystem_related Related Systems
 * - AMGCheckpointActor: Triggers checkpoint crossing events
 * - Race Game Mode: Initializes subsystem and manages race state
 * - Race HUD: Displays timing and position information
 * - AI Controller: May query track data for navigation
 *
 * @see UWorldSubsystem
 * @see FMGCheckpointData
 * @see FMGRacerProgress
 * @see FMGTrackConfig
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGTrackSubsystem.generated.h"

class AMGCheckpointActor;
class AMGRacingLineActor;
class UMGTrackDataAsset;
class USplineComponent;

// Type alias for backwards compatibility
using UMGTrackData = UMGTrackDataAsset;

/**
 * Track surface type
 */
UENUM(BlueprintType)
enum class EMGTrackSurface : uint8
{
	Asphalt,
	Concrete,
	Cobblestone,
	Dirt,
	Gravel,
	Grass,
	Water,
	Ice,
	Metal
};

/**
 * Checkpoint data
 */
USTRUCT(BlueprintType)
struct FMGCheckpointData
{
	GENERATED_BODY()

	/** Checkpoint index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = 0;

	/** World position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/** Forward direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Checkpoint width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width = 1000.0f;

	/** Is this a sector split */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSectorSplit = false;

	/** Sector index (if sector split) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SectorIndex = 0;

	/** Distance from start line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceFromStart = 0.0f;

	/** Associated checkpoint actor */
	UPROPERTY()
	TWeakObjectPtr<AMGCheckpointActor> CheckpointActor;
};

/**
 * Racer checkpoint progress
 */
USTRUCT(BlueprintType)
struct FMGRacerProgress
{
	GENERATED_BODY()

	/** Racer ID */
	UPROPERTY(BlueprintReadWrite)
	int32 RacerID = -1;

	/** Current lap */
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentLap = 0;

	/** Last checkpoint passed */
	UPROPERTY(BlueprintReadWrite)
	int32 LastCheckpoint = -1;

	/** Total checkpoints passed (for ranking) */
	UPROPERTY(BlueprintReadWrite)
	int32 TotalCheckpointsPassed = 0;

	/** Distance within current checkpoint segment */
	UPROPERTY(BlueprintReadWrite)
	float DistanceInSegment = 0.0f;

	/** Total distance traveled */
	UPROPERTY(BlueprintReadWrite)
	float TotalDistance = 0.0f;

	/** Lap times */
	UPROPERTY(BlueprintReadWrite)
	TArray<float> LapTimes;

	/** Sector times for current lap */
	UPROPERTY(BlueprintReadWrite)
	TArray<float> CurrentSectorTimes;

	/** Best sector times */
	UPROPERTY(BlueprintReadWrite)
	TArray<float> BestSectorTimes;

	/** Time entered last checkpoint */
	UPROPERTY(BlueprintReadWrite)
	float LastCheckpointTime = 0.0f;

	/** Has finished race */
	UPROPERTY(BlueprintReadWrite)
	bool bHasFinished = false;

	/** Finish time */
	UPROPERTY(BlueprintReadWrite)
	float FinishTime = 0.0f;

	/** Is going wrong way */
	UPROPERTY(BlueprintReadWrite)
	bool bWrongWay = false;
};

/**
 * Track configuration
 */
USTRUCT(BlueprintType)
struct FMGTrackConfig
{
	GENERATED_BODY()

	/** Track name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackName;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Is circuit (loops) or point-to-point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCircuit = true;

	/** Total track length in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackLength = 0.0f;

	/** Number of sectors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumSectors = 3;

	/** Track record time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackRecordTime = 0.0f;

	/** Track record holder */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackRecordHolder;
};

/**
 * Track Subsystem
 * Manages track data, checkpoints, and racer progress
 *
 * Features:
 * - Checkpoint registration and validation
 * - Lap counting and timing
 * - Sector timing
 * - Wrong way detection
 * - Race position calculation
 * - Track records
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTrackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	// ==========================================
	// TRACK SETUP
	// ==========================================

	/** Initialize track from data asset */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void InitializeTrack(UMGTrackData* TrackData);

	/** Load track by ID (finds checkpoints in world) */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void LoadTrack(FName TrackID);

	/** Set track configuration */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void SetTrackConfig(const FMGTrackConfig& Config);

	/** Get track configuration */
	UFUNCTION(BlueprintPure, Category = "Track")
	FMGTrackConfig GetTrackConfig() const { return TrackConfig; }

	/** Register a checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void RegisterCheckpoint(const FMGCheckpointData& Checkpoint);

	/** Clear all checkpoints */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void ClearCheckpoints();

	/** Get checkpoint count */
	UFUNCTION(BlueprintPure, Category = "Track")
	int32 GetCheckpointCount() const { return Checkpoints.Num(); }

	/** Get checkpoint data */
	UFUNCTION(BlueprintPure, Category = "Track")
	bool GetCheckpoint(int32 Index, FMGCheckpointData& OutCheckpoint) const;

	// ==========================================
	// RACER TRACKING
	// ==========================================

	/** Register a racer */
	UFUNCTION(BlueprintCallable, Category = "Track|Racers")
	void RegisterRacer(int32 RacerID, AActor* RacerActor);

	/** Unregister a racer */
	UFUNCTION(BlueprintCallable, Category = "Track|Racers")
	void UnregisterRacer(int32 RacerID);

	/** Get racer progress */
	UFUNCTION(BlueprintPure, Category = "Track|Racers")
	bool GetRacerProgress(int32 RacerID, FMGRacerProgress& OutProgress) const;

	/** Get all racer progress (sorted by position) */
	UFUNCTION(BlueprintPure, Category = "Track|Racers")
	TArray<FMGRacerProgress> GetAllRacerProgress() const;

	/** Get racer position (1 = first) */
	UFUNCTION(BlueprintPure, Category = "Track|Racers")
	int32 GetRacerPosition(int32 RacerID) const;

	/** Get racer ahead of given racer */
	UFUNCTION(BlueprintPure, Category = "Track|Racers")
	int32 GetRacerAhead(int32 RacerID) const;

	/** Get racer behind given racer */
	UFUNCTION(BlueprintPure, Category = "Track|Racers")
	int32 GetRacerBehind(int32 RacerID) const;

	/** Get gap to racer (negative = ahead) */
	UFUNCTION(BlueprintPure, Category = "Track|Racers")
	float GetGapToRacer(int32 FromRacerID, int32 ToRacerID) const;

	// ==========================================
	// CHECKPOINT CROSSING
	// ==========================================

	/** Called when racer crosses checkpoint (called by checkpoint actor) */
	UFUNCTION(BlueprintCallable, Category = "Track|Checkpoints")
	void OnCheckpointCrossed(int32 RacerID, int32 CheckpointIndex);

	/** Called when racer crosses finish line */
	UFUNCTION(BlueprintCallable, Category = "Track|Checkpoints")
	void OnFinishLineCrossed(int32 RacerID);

	// ==========================================
	// WRONG WAY
	// ==========================================

	/** Check if racer is going wrong way */
	UFUNCTION(BlueprintPure, Category = "Track|WrongWay")
	bool IsRacerWrongWay(int32 RacerID) const;

	/** Update wrong way status for racer */
	UFUNCTION(BlueprintCallable, Category = "Track|WrongWay")
	void UpdateRacerWrongWay(int32 RacerID, FVector Position, FVector Velocity);

	// ==========================================
	// TIMING
	// ==========================================

	/** Get current race time */
	UFUNCTION(BlueprintPure, Category = "Track|Timing")
	float GetRaceTime() const { return RaceTime; }

	/** Start race timer */
	UFUNCTION(BlueprintCallable, Category = "Track|Timing")
	void StartRaceTimer();

	/** Stop race timer */
	UFUNCTION(BlueprintCallable, Category = "Track|Timing")
	void StopRaceTimer();

	/** Reset race timer */
	UFUNCTION(BlueprintCallable, Category = "Track|Timing")
	void ResetRaceTimer();

	/** Is race timer running */
	UFUNCTION(BlueprintPure, Category = "Track|Timing")
	bool IsRaceTimerRunning() const { return bRaceTimerRunning; }

	// ==========================================
	// TRACK QUERY
	// ==========================================

	/** Get distance along track for position */
	UFUNCTION(BlueprintPure, Category = "Track|Query")
	float GetDistanceAlongTrack(FVector WorldPosition) const;

	/** Get position on track at distance */
	UFUNCTION(BlueprintPure, Category = "Track|Query")
	FVector GetPositionAtDistance(float Distance) const;

	/** Get direction at distance */
	UFUNCTION(BlueprintPure, Category = "Track|Query")
	FRotator GetDirectionAtDistance(float Distance) const;

	/** Get next checkpoint for racer */
	UFUNCTION(BlueprintPure, Category = "Track|Query")
	int32 GetNextCheckpointForRacer(int32 RacerID) const;

	/** Get distance to next checkpoint */
	UFUNCTION(BlueprintPure, Category = "Track|Query")
	float GetDistanceToNextCheckpoint(int32 RacerID) const;

	/** Get surface type at position */
	UFUNCTION(BlueprintPure, Category = "Track|Query")
	EMGTrackSurface GetSurfaceAtPosition(FVector Position) const;

	// ==========================================
	// EVENTS
	// ==========================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointPassed, int32, RacerID, int32, CheckpointIndex);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLapCompleted, int32, RacerID, int32, LapNumber, float, LapTime);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSectorCompleted, int32, RacerID, int32, SectorIndex, float, SectorTime, bool, bIsBestSector);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRacerFinished, int32, RacerID, int32, Position, float, TotalTime);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWrongWayChanged, int32, RacerID, bool, bWrongWay);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPositionChanged, int32, RacerID, int32, OldPosition, int32, NewPosition);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewTrackRecord, float, NewRecordTime, const FString&, RecordHolder);

	UPROPERTY(BlueprintAssignable, Category = "Track|Events")
	FOnCheckpointPassed OnCheckpointPassed;

	UPROPERTY(BlueprintAssignable, Category = "Track|Events")
	FOnLapCompleted OnLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Track|Events")
	FOnSectorCompleted OnSectorCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Track|Events")
	FOnRacerFinished OnRacerFinished;

	UPROPERTY(BlueprintAssignable, Category = "Track|Events")
	FOnWrongWayChanged OnWrongWayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Track|Events")
	FOnPositionChanged OnPositionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Track|Events")
	FOnNewTrackRecord OnNewTrackRecord;

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Track configuration */
	FMGTrackConfig TrackConfig;

	/** Checkpoints */
	UPROPERTY()
	TArray<FMGCheckpointData> Checkpoints;

	/** Racer progress */
	UPROPERTY()
	TMap<int32, FMGRacerProgress> RacerProgressMap;

	/** Racer actors */
	UPROPERTY()
	TMap<int32, TWeakObjectPtr<AActor>> RacerActors;

	/** Cached positions (updated each tick) */
	TArray<int32> CachedPositions;

	/** Race timer */
	float RaceTime = 0.0f;
	bool bRaceTimerRunning = false;

	/** Track spline (if available) */
	UPROPERTY()
	TWeakObjectPtr<USplineComponent> TrackSpline;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update racer positions */
	void UpdatePositions();

	/** Calculate total distance for racer */
	float CalculateTotalDistance(const FMGRacerProgress& Progress) const;

	/** Get checkpoint for sector */
	int32 GetCheckpointForSector(int32 SectorIndex) const;

	/** Check if checkpoint is valid progression */
	bool IsValidCheckpointProgression(int32 RacerID, int32 CheckpointIndex) const;

	/** Process lap completion */
	void ProcessLapCompletion(int32 RacerID);

	/** Process sector completion */
	void ProcessSectorCompletion(int32 RacerID, int32 SectorIndex, float SectorTime);

	/** Check for new track record */
	void CheckTrackRecord(float LapTime, int32 RacerID);
};

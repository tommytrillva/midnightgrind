// Copyright Midnight Grind. All Rights Reserved.

#pragma once
/**
 * @file MGTrackSubsystem.h
 * @brief World subsystem managing track data, checkpoints, and racer progress.
 *
 * UMGTrackSubsystem is the central authority for race track management including:
 * - Checkpoint registration and lap counting
 * - Sector timing and race positions
 * - Wrong-way detection
 * - Track records and leaderboards
 *
 * As a UWorldSubsystem, it is created per-world and handles track-specific logic
 * for the current racing session.
 *
 * @section track_subsystem_usage Usage
 * @code
 * UMGTrackSubsystem* TrackSub = GetWorld()->GetSubsystem<UMGTrackSubsystem>();
 * TrackSub->LoadTrack(FName("DowntownCircuit"));
 * TrackSub->RegisterRacer(PlayerID, PlayerVehicle);
 * TrackSub->StartRaceTimer();
 * @endcode
 *
 * @see AMGCheckpointActor for checkpoint placement
 * @see UMGTrackDataAsset for track configuration
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tire/MGTireSubsystem.h"
#include "MGTrackSubsystem.generated.h"

// Forward declarations
class UMGTrackDataAsset;
// Type alias for backward compatibility
using UMGTrackData = UMGTrackDataAsset;
class AMGCheckpointActor;
class USplineComponent;

// Type alias for backwards compatibility
using UMGTrackData = UMGTrackDataAsset;

/**
 * Checkpoint data
 * 
 * Merged from Track and Racing subsystems to provide unified checkpoint data.
 * Contains both layout info (Width) and race validation info (bIsStartFinish).
 */
USTRUCT(BlueprintType)
struct FMGCheckpointData
{
	GENERATED_BODY()

	/** Checkpoint index (sequential, 0 = start/finish) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 Index = 0;

	/** World position of checkpoint center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FVector Position = FVector::ZeroVector;

	/** Forward direction (for directional validation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FRotator Rotation = FRotator::ZeroRotator;

	/** Checkpoint width in units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float Width = 1000.0f;

	/** Distance from start line in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float DistanceFromStart = 0.0f;

	/** True if this is the start/finish line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsStartFinish = false;

	/** True if this checkpoint marks a sector boundary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsSectorSplit = false;

	/** Sector index (if sector split) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 SectorIndex = 0;

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
	virtual void Tick(float MGDeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	// ==========================================
	// TRACK SETUP
	// ==========================================

	/** Initialize track from data asset */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void InitializeTrack(UMGTrackDataAsset* TrackData);

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

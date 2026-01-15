// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGTimeTrialHandler.generated.h"

/**
 * Ghost replay frame
 */
USTRUCT(BlueprintType)
struct FMGGhostFrame
{
	GENERATED_BODY()

	/** Time since start */
	UPROPERTY()
	float Timestamp = 0.0f;

	/** World position */
	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	/** World rotation */
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	/** Current speed */
	UPROPERTY()
	float Speed = 0.0f;

	/** Wheel steer angle */
	UPROPERTY()
	float SteerAngle = 0.0f;

	/** Is braking? */
	UPROPERTY()
	bool bBraking = false;
};

/**
 * Complete ghost replay data
 */
USTRUCT(BlueprintType)
struct FMGGhostReplay
{
	GENERATED_BODY()

	/** Player ID who recorded this */
	UPROPERTY()
	FString PlayerId;

	/** Player display name */
	UPROPERTY()
	FText PlayerName;

	/** Track ID */
	UPROPERTY()
	FName TrackId;

	/** Vehicle model used */
	UPROPERTY()
	FName VehicleModelId;

	/** Total lap time */
	UPROPERTY()
	float LapTime = 0.0f;

	/** Sector times */
	UPROPERTY()
	TArray<float> SectorTimes;

	/** All recorded frames */
	UPROPERTY()
	TArray<FMGGhostFrame> Frames;

	/** Recording date */
	UPROPERTY()
	FDateTime RecordedDate;

	/** Is this valid replay data? */
	bool IsValid() const { return Frames.Num() > 0 && LapTime > 0.0f; }
};

/**
 * Sector comparison result
 */
UENUM(BlueprintType)
enum class EMGSectorComparison : uint8
{
	/** First attempt - no comparison */
	NoComparison,
	/** Faster than comparison time */
	Faster,
	/** Same as comparison time */
	Equal,
	/** Slower than comparison time */
	Slower
};

/**
 * Sector time with comparison
 */
USTRUCT(BlueprintType)
struct FMGSectorResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SectorIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	float SectorTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float ComparisonTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float Delta = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	EMGSectorComparison Comparison = EMGSectorComparison::NoComparison;

	UPROPERTY(BlueprintReadOnly)
	bool bPersonalBest = false;

	UPROPERTY(BlueprintReadOnly)
	bool bTrackRecord = false;
};

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewPersonalBest, float, LapTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewTrackRecord, float, LapTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSectorResult, const FMGSectorResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGhostRecorded, const FMGGhostReplay&, Ghost);

/**
 * Time Trial Handler
 * Solo time attack with ghost racing
 *
 * Win Condition: Best lap time / beat target time
 * Scoring: Time-based with sector splits
 * Features:
 * - Ghost replay recording and playback
 * - Personal best tracking
 * - Sector time comparisons
 * - Track record comparison
 * - Multiple laps to improve
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTimeTrialHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGTimeTrialHandler();

	//~ Begin UMGRaceTypeHandler Interface
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;
	virtual void Reset() override;
	virtual void OnRaceStarted() override;
	virtual void OnRaceTick(float DeltaTime) override;
	virtual void OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex) override;
	virtual void OnLapCompleted(int32 RacerIndex, float LapTime) override;
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::TimeTrial; }
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual bool ShouldShowLapCounter() const override { return true; }
	virtual bool ShouldShowPosition() const override { return false; }
	virtual bool ShouldShowScore() const override { return false; }
	virtual FText GetProgressFormat() const override;

	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// GHOST MANAGEMENT
	// ==========================================

	/** Set the ghost to race against */
	UFUNCTION(BlueprintCallable, Category = "Time Trial")
	void SetComparisonGhost(const FMGGhostReplay& Ghost);

	/** Set personal best ghost */
	UFUNCTION(BlueprintCallable, Category = "Time Trial")
	void SetPersonalBestGhost(const FMGGhostReplay& Ghost);

	/** Set track record ghost */
	UFUNCTION(BlueprintCallable, Category = "Time Trial")
	void SetTrackRecordGhost(const FMGGhostReplay& Ghost);

	/** Get the current recording (for saving) */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	FMGGhostReplay GetCurrentRecording() const { return CurrentRecording; }

	/** Get best lap recording from session */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	FMGGhostReplay GetBestLapRecording() const { return BestLapRecording; }

	/** Get ghost position at time */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	bool GetGhostTransformAtTime(float Time, FVector& OutPosition, FRotator& OutRotation) const;

	// ==========================================
	// TIMING
	// ==========================================

	/** Get current lap time */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetCurrentLapTime() const { return CurrentLapTime; }

	/** Get best lap time this session */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetSessionBestLap() const { return SessionBestLapTime; }

	/** Get personal best lap time */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetPersonalBest() const { return PersonalBestTime; }

	/** Get track record */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetTrackRecord() const { return TrackRecordTime; }

	/** Get target time to beat */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetTargetTime() const { return TargetTime; }

	/** Get current delta to comparison ghost */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetCurrentDelta() const;

	/** Get all sector results for current lap */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	TArray<FMGSectorResult> GetCurrentSectorResults() const { return CurrentSectorResults; }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set total laps allowed */
	UFUNCTION(BlueprintCallable, Category = "Time Trial|Config")
	void SetTotalLaps(int32 Laps) { TotalLaps = Laps; }

	/** Set target time to beat */
	UFUNCTION(BlueprintCallable, Category = "Time Trial|Config")
	void SetTargetTime(float Time) { TargetTime = Time; }

	/** Set recording framerate */
	UFUNCTION(BlueprintCallable, Category = "Time Trial|Config")
	void SetRecordingFramerate(float FPS) { RecordingFramerate = FPS; }

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Time Trial|Events")
	FOnNewPersonalBest OnNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Time Trial|Events")
	FOnNewTrackRecord OnNewTrackRecord;

	UPROPERTY(BlueprintAssignable, Category = "Time Trial|Events")
	FOnSectorResult OnSectorResult;

	UPROPERTY(BlueprintAssignable, Category = "Time Trial|Events")
	FOnGhostRecorded OnGhostRecorded;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Recording framerate */
	UPROPERTY(EditDefaultsOnly, Category = "Time Trial|Config")
	float RecordingFramerate = 30.0f;

	/** Total laps in session */
	UPROPERTY()
	int32 TotalLaps = 5;

	/** Target time to beat (for rewards) */
	UPROPERTY()
	float TargetTime = 0.0f;

	// ==========================================
	// GHOST DATA
	// ==========================================

	/** Ghost being raced against */
	UPROPERTY()
	FMGGhostReplay ComparisonGhost;

	/** Personal best ghost */
	UPROPERTY()
	FMGGhostReplay PersonalBestGhost;

	/** Track record ghost */
	UPROPERTY()
	FMGGhostReplay TrackRecordGhost;

	/** Current lap recording */
	UPROPERTY()
	FMGGhostReplay CurrentRecording;

	/** Best lap recording this session */
	UPROPERTY()
	FMGGhostReplay BestLapRecording;

	// ==========================================
	// TIMING STATE
	// ==========================================

	/** Current lap time */
	float CurrentLapTime = 0.0f;

	/** Session best lap time */
	float SessionBestLapTime = 0.0f;

	/** Personal best time */
	float PersonalBestTime = 0.0f;

	/** Track record time */
	float TrackRecordTime = 0.0f;

	/** Current lap's sector times */
	UPROPERTY()
	TArray<float> CurrentSectorTimes;

	/** Current sector results (with comparisons) */
	UPROPERTY()
	TArray<FMGSectorResult> CurrentSectorResults;

	/** Recording accumulator */
	float RecordingAccumulator = 0.0f;

	/** Current lap number */
	int32 CurrentLap = 0;

	/** Laps completed */
	int32 CompletedLaps = 0;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Record current frame */
	void RecordFrame();

	/** Start new lap recording */
	void StartNewLapRecording();

	/** Finalize lap recording */
	void FinalizeLapRecording(float LapTime);

	/** Calculate sector comparison */
	FMGSectorResult CalculateSectorResult(int32 SectorIndex, float SectorTime) const;

	/** Interpolate ghost position */
	bool InterpolateGhostFrame(const FMGGhostReplay& Ghost, float Time, FMGGhostFrame& OutFrame) const;
};

/**
 * @file MGTimeTrialHandler.h
 * @brief Time Trial Handler - Solo time attack with ghost racing
 *
 * Time Trial is a solo mode where the player races against the clock,
 * attempting to set the fastest lap time. Ghost replays of personal bests
 * and track records provide visual competition without actual opponents.
 *
 * @section overview Overview
 * Time Trial is the pure skill test mode. Without traffic, opponents, or
 * distractions, it's just you, the track, and the clock. This mode is
 * perfect for learning tracks, tuning cars, and competing on leaderboards.
 *
 * @section win_condition Win Condition
 * There's no traditional "win" - success is measured by:
 * - Setting a new personal best lap time
 * - Beating the track record
 * - Meeting a target time (if set)
 *
 * @section ghost_system Ghost System
 * The handler records the player's position, rotation, and vehicle state
 * every frame (configurable rate). This creates a "ghost" replay that can
 * be raced against in future attempts. Multiple ghosts can be loaded:
 * - **Personal Best**: Your fastest lap on this track
 * - **Track Record**: The server's fastest time
 * - **Comparison**: Any selected ghost (friend's time, etc.)
 *
 * @section sector_timing Sector Timing
 * The track is divided into sectors, and split times show whether you're
 * ahead or behind your comparison ghost. Deltas are color-coded:
 * - Green: Faster than comparison
 * - Red: Slower than comparison
 * - Purple: New personal best for this sector
 *
 * @section ui_elements UI Elements
 * - Current lap time (running)
 * - Delta to comparison ghost
 * - Sector split times
 * - Personal best / track record display
 * - Ghost car visualization
 *
 * @see UMGRaceTypeHandler Base class
 * @see FMGGhostReplay Ghost data structure
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGTimeTrialHandler.generated.h"

// ============================================================================
// GHOST REPLAY STRUCTS
// ============================================================================

/**
 * @brief Single frame of ghost replay data
 *
 * Captures the vehicle state at a moment in time for replay.
 * Recorded at a configurable framerate (default 30 FPS).
 */
USTRUCT(BlueprintType)
struct FMGGhostFrame
{
	GENERATED_BODY()

	/// Time since lap started (seconds)
	UPROPERTY()
	float Timestamp = 0.0f;

	/// World position of the vehicle
	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	/// World rotation of the vehicle
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	/// Speed at this moment (for speedometer display)
	UPROPERTY()
	float Speed = 0.0f;

	/// Steering wheel angle (for wheel visualization)
	UPROPERTY()
	float SteerAngle = 0.0f;

	/// Whether brakes were applied (for brake light visualization)
	UPROPERTY()
	bool bBraking = false;
};

/**
 * @brief Complete ghost replay for one lap
 *
 * Contains all the data needed to replay a lap as a ghost car,
 * including metadata about who recorded it and on what vehicle.
 */
USTRUCT(BlueprintType)
struct FMGGhostReplay
{
	GENERATED_BODY()

	// === Metadata ===

	/// Unique identifier of the player who recorded this
	UPROPERTY()
	FString PlayerId;

	/// Display name of the player
	UPROPERTY()
	FText PlayerName;

	/// ID of the track this was recorded on
	UPROPERTY()
	FName TrackId;

	/// ID of the vehicle model used
	UPROPERTY()
	FName VehicleModelId;

	// === Timing ===

	/// Total lap time for this ghost (seconds)
	UPROPERTY()
	float LapTime = 0.0f;

	/// Time for each sector (for split comparisons)
	UPROPERTY()
	TArray<float> SectorTimes;

	// === Replay Data ===

	/// All recorded frames for playback
	UPROPERTY()
	TArray<FMGGhostFrame> Frames;

	/// When this ghost was recorded (for "Set X days ago" display)
	UPROPERTY()
	FDateTime RecordedDate;

	/** Check if this ghost has valid data for playback */
	bool IsValid() const { return Frames.Num() > 0 && LapTime > 0.0f; }
};

// ============================================================================
// SECTOR COMPARISON TYPES
// ============================================================================

/**
 * @brief Result of comparing a sector time to a reference
 */
UENUM(BlueprintType)
enum class EMGSectorComparison : uint8
{
	/** No comparison available (first lap or no ghost) */
	NoComparison,
	/** Faster than the comparison time (green) */
	Faster,
	/** Exactly equal to comparison (rare) */
	Equal,
	/** Slower than the comparison time (red) */
	Slower
};

/**
 * @brief Complete sector result with comparison data
 *
 * Displayed to the player after completing each sector,
 * showing their time and delta vs the comparison ghost.
 */
USTRUCT(BlueprintType)
struct FMGSectorResult
{
	GENERATED_BODY()

	/// Which sector this result is for (0-indexed)
	UPROPERTY(BlueprintReadOnly)
	int32 SectorIndex = 0;

	/// Player's time for this sector (seconds)
	UPROPERTY(BlueprintReadOnly)
	float SectorTime = 0.0f;

	/// Time from comparison ghost (for delta calculation)
	UPROPERTY(BlueprintReadOnly)
	float ComparisonTime = 0.0f;

	/// Difference: positive = slower, negative = faster
	UPROPERTY(BlueprintReadOnly)
	float Delta = 0.0f;

	/// Comparison result (for color coding)
	UPROPERTY(BlueprintReadOnly)
	EMGSectorComparison Comparison = EMGSectorComparison::NoComparison;

	/// True if this is a new personal best for this sector
	UPROPERTY(BlueprintReadOnly)
	bool bPersonalBest = false;

	/// True if this beats the track record for this sector
	UPROPERTY(BlueprintReadOnly)
	bool bTrackRecord = false;
};

// ============================================================================
// TIME TRIAL EVENT DELEGATES
// ============================================================================

/** Broadcast when player sets a new personal best lap time */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewPersonalBest, float, LapTime);

/** Broadcast when player sets a new track record */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewTrackRecord, float, LapTime);

/** Broadcast when player completes a sector with comparison result */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSectorResult, const FMGSectorResult&, Result);

/** Broadcast when a lap recording is finalized (for saving) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGhostRecorded, const FMGGhostReplay&, Ghost);

// ============================================================================
// TIME TRIAL HANDLER CLASS
// ============================================================================

/**
 * @brief Handler for time trial (solo time attack) mode
 *
 * Manages ghost recording/playback, lap timing with sector splits,
 * and personal best/track record tracking. This is a solo mode
 * focused on improving lap times.
 *
 * @section recording Ghost Recording
 * During each lap, the handler records the vehicle state at
 * RecordingFramerate FPS. When the lap is completed, if it's
 * the best lap of the session, it becomes the new best lap ghost.
 *
 * @section playback Ghost Playback
 * Call GetGhostTransformAtTime() each frame to get the ghost
 * position for rendering. The ghost car is typically a
 * semi-transparent version of the original vehicle.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTimeTrialHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGTimeTrialHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE OVERRIDES
	// ==========================================

	//~ Begin UMGRaceTypeHandler Interface

	/** Load personal best and track record ghosts */
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;

	/** Clear session data and recordings */
	virtual void Reset() override;

	/** Start timing and recording */
	virtual void OnRaceStarted() override;

	/** Record ghost frame and update timing */
	virtual void OnRaceTick(float DeltaTime) override;

	/** Record sector time and check for bests */
	virtual void OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex) override;

	/** Finalize lap recording and check for personal best */
	virtual void OnLapCompleted(int32 RacerIndex, float LapTime) override;

	/** Check if lap limit reached */
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;

	/** No positions in time trial (solo mode) */
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	/// Returns EMGRaceType::TimeTrial
	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::TimeTrial; }

	/** Get localized display name "Time Trial" */
	virtual FText GetDisplayName() const override;

	/** Get description of time trial rules */
	virtual FText GetDescription() const override;

	/// Time trials show lap counter
	virtual bool ShouldShowLapCounter() const override { return true; }

	/// Time trials don't show position (solo)
	virtual bool ShouldShowPosition() const override { return false; }

	/// Time trials don't use score
	virtual bool ShouldShowScore() const override { return false; }

	/** Returns lap-based progress format */
	virtual FText GetProgressFormat() const override;

	/** Calculate credits based on time vs target */
	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// GHOST MANAGEMENT
	// Methods for loading and accessing ghosts
	// ==========================================

	/**
	 * @brief Set the ghost to race against
	 * @param Ghost The ghost replay data
	 * @note This is the primary comparison for delta display
	 */
	UFUNCTION(BlueprintCallable, Category = "Time Trial")
	void SetComparisonGhost(const FMGGhostReplay& Ghost);

	/**
	 * @brief Set the personal best ghost
	 * @param Ghost Player's fastest lap ghost
	 */
	UFUNCTION(BlueprintCallable, Category = "Time Trial")
	void SetPersonalBestGhost(const FMGGhostReplay& Ghost);

	/**
	 * @brief Set the track record ghost
	 * @param Ghost Server's fastest lap ghost
	 */
	UFUNCTION(BlueprintCallable, Category = "Time Trial")
	void SetTrackRecordGhost(const FMGGhostReplay& Ghost);

	/**
	 * @brief Get the current lap's recording
	 * @return Recording in progress (may be incomplete)
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	FMGGhostReplay GetCurrentRecording() const { return CurrentRecording; }

	/**
	 * @brief Get the best lap recording from this session
	 * @return Best completed lap (for saving)
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	FMGGhostReplay GetBestLapRecording() const { return BestLapRecording; }

	/**
	 * @brief Get ghost position at a specific time
	 * @param Time Elapsed time to query
	 * @param OutPosition Interpolated position
	 * @param OutRotation Interpolated rotation
	 * @return True if ghost data available at this time
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	bool GetGhostTransformAtTime(float Time, FVector& OutPosition, FRotator& OutRotation) const;

	// ==========================================
	// TIMING
	// Methods for accessing timing information
	// ==========================================

	/**
	 * @brief Get the current lap time (running)
	 * @return Seconds since lap started
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetCurrentLapTime() const { return CurrentLapTime; }

	/**
	 * @brief Get the best lap time this session
	 * @return Best completed lap time (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetSessionBestLap() const { return SessionBestLapTime; }

	/**
	 * @brief Get the all-time personal best
	 * @return Personal best lap time (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetPersonalBest() const { return PersonalBestTime; }

	/**
	 * @brief Get the track record time
	 * @return Track record lap time (seconds)
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetTrackRecord() const { return TrackRecordTime; }

	/**
	 * @brief Get the target time to beat (if set)
	 * @return Target time (0 if not set)
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetTargetTime() const { return TargetTime; }

	/**
	 * @brief Get current delta to comparison ghost
	 * @return Seconds behind (positive) or ahead (negative)
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	float GetCurrentDelta() const;

	/**
	 * @brief Get all sector results for current lap
	 * @return Array of completed sector results
	 */
	UFUNCTION(BlueprintPure, Category = "Time Trial")
	TArray<FMGSectorResult> GetCurrentSectorResults() const { return CurrentSectorResults; }

	// ==========================================
	// CONFIGURATION
	// Session setup methods
	// ==========================================

	/**
	 * @brief Set maximum laps allowed in session
	 * @param Laps Number of laps (0 = unlimited)
	 */
	UFUNCTION(BlueprintCallable, Category = "Time Trial|Config")
	void SetTotalLaps(int32 Laps) { TotalLaps = Laps; }

	/**
	 * @brief Set a target time for bonus rewards
	 * @param Time Target lap time in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Time Trial|Config")
	void SetTargetTime(float Time) { TargetTime = Time; }

	/**
	 * @brief Set ghost recording framerate
	 * @param FPS Frames per second (default 30)
	 */
	UFUNCTION(BlueprintCallable, Category = "Time Trial|Config")
	void SetRecordingFramerate(float FPS) { RecordingFramerate = FPS; }

	// ==========================================
	// EVENTS
	// Delegates for time trial events
	// ==========================================

	/** Broadcast when player beats their personal best */
	UPROPERTY(BlueprintAssignable, Category = "Time Trial|Events")
	FOnNewPersonalBest OnNewPersonalBest;

	/** Broadcast when player beats the track record */
	UPROPERTY(BlueprintAssignable, Category = "Time Trial|Events")
	FOnNewTrackRecord OnNewTrackRecord;

	/** Broadcast when completing a sector with split data */
	UPROPERTY(BlueprintAssignable, Category = "Time Trial|Events")
	FOnSectorResult OnSectorResult;

	/** Broadcast when a lap recording is complete */
	UPROPERTY(BlueprintAssignable, Category = "Time Trial|Events")
	FOnGhostRecorded OnGhostRecorded;

protected:
	// ==========================================
	// RECORDING CONFIGURATION
	// ==========================================

	/** Ghost recording framerate (frames per second) */
	UPROPERTY(EditDefaultsOnly, Category = "Time Trial|Config")
	float RecordingFramerate = 30.0f;

	/** Maximum laps in session (0 = unlimited practice) */
	UPROPERTY()
	int32 TotalLaps = 5;

	/** Target time for bonus rewards (0 = no target) */
	UPROPERTY()
	float TargetTime = 0.0f;

	// ==========================================
	// LOADED GHOSTS
	// Ghost data for comparison
	// ==========================================

	/** Ghost being actively compared against */
	UPROPERTY()
	FMGGhostReplay ComparisonGhost;

	/** Player's all-time personal best ghost */
	UPROPERTY()
	FMGGhostReplay PersonalBestGhost;

	/** Server track record ghost */
	UPROPERTY()
	FMGGhostReplay TrackRecordGhost;

	// ==========================================
	// RECORDING STATE
	// Current session recording data
	// ==========================================

	/** Recording of current lap (in progress) */
	UPROPERTY()
	FMGGhostReplay CurrentRecording;

	/** Best lap recording this session (for saving) */
	UPROPERTY()
	FMGGhostReplay BestLapRecording;

	// ==========================================
	// TIMING STATE
	// Current lap timing data
	// ==========================================

	/** Time elapsed in current lap (seconds) */
	float CurrentLapTime = 0.0f;

	/** Best lap time achieved this session */
	float SessionBestLapTime = 0.0f;

	/** All-time personal best (loaded from profile) */
	float PersonalBestTime = 0.0f;

	/** Track record time (loaded from server) */
	float TrackRecordTime = 0.0f;

	/** Sector times for current lap (in progress) */
	UPROPERTY()
	TArray<float> CurrentSectorTimes;

	/** Sector results with comparisons (for UI) */
	UPROPERTY()
	TArray<FMGSectorResult> CurrentSectorResults;

	/** Accumulator for recording framerate control */
	float RecordingAccumulator = 0.0f;

	/** Current lap number (1-indexed) */
	int32 CurrentLap = 0;

	/** Total laps completed this session */
	int32 CompletedLaps = 0;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Record a frame of ghost data at current vehicle state */
	void RecordFrame();

	/** Start recording a new lap */
	void StartNewLapRecording();

	/** Finalize the lap recording with final time */
	void FinalizeLapRecording(float LapTime);

	/** Calculate sector comparison result */
	FMGSectorResult CalculateSectorResult(int32 SectorIndex, float SectorTime) const;

	/** Interpolate ghost frame at arbitrary time */
	bool InterpolateGhostFrame(const FMGGhostReplay& Ghost, float Time, FMGGhostFrame& OutFrame) const;
};

// Copyright Midnight Grind. All Rights Reserved.

#pragma once
/**
 * @file MGReplaySubsystem.h
 * @brief Replay Subsystem - Central management for replay recording and playback
 *
 * The Replay Subsystem provides high-level replay functionality including:
 * - Recording race data for replay viewing
 * - Ghost racer spawning and management
 * - Full replay playback with multiple speeds and modes
 * - Replay storage and retrieval
 *
 * KEY CONCEPTS:
 * =============
 *
 * RECORDING:
 * The subsystem records vehicle state (position, rotation, inputs) at a
 * fixed frame rate. Recording can be triggered manually or automatically
 * when a race begins.
 *
 * PLAYBACK:
 * Replays can be viewed in full (replay theater) or used to spawn ghost
 * racers that race alongside the player. Multiple playback speeds and
 * modes (normal, slow-mo, frame-by-frame) are supported.
 *
 * GHOST RACERS:
 * Ghost racers are semi-transparent vehicles that follow recorded replay
 * data. They provide visual feedback for time attack modes and can show
 * personal best, friends' times, or world records.
 *
 * STORAGE:
 * Replays are compressed and stored locally. They can also be uploaded
 * to online servers for leaderboards and sharing.
 *
 * RELATIONSHIP TO OTHER SUBSYSTEMS:
 * - UMGReplayBufferSubsystem: Handles continuous circular buffer recording
 * - UMGReplayShareSubsystem: Handles clip creation and social sharing
 * - UMGReplayRecordingComponent: Per-actor recording component
 *
 * @see AMGGhostRacerActor - Ghost visualization
 * @see FMGReplayData - Replay data structure
 * @see FMGGhostConfig - Ghost configuration
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

// EMGReplayState - REMOVED (duplicate)
// Canonical definition in: Core/MGSharedTypes.h

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "ReplayBuffer/MGReplayBufferSubsystem.h"
#include "MGReplaySubsystem.generated.h"

class AMGGhostRacerActor;

/**
 * Replay playback mode
 *
 * Controls how the replay is played back.
 */
UENUM(BlueprintType)
enum class EMGReplayPlaybackMode : uint8
{
	/** Standard real-time playback */
	Normal,
	/** Slow motion playback */
	SlowMotion,
	/** Fast forward */
	FastForward,
	/** Frame by frame */
	FrameByFrame
};

// FMGReplayFrame - REMOVED (duplicate)
// Canonical definition in: ReplayBuffer/MGReplayBufferSubsystem.h

/**
 * Complete replay data for a race
 */
USTRUCT(BlueprintType)
struct FMGReplayData
{
	GENERATED_BODY()

	/** Unique replay identifier */
	UPROPERTY(BlueprintReadOnly)
	FGuid ReplayID;

	/** Track ID this replay is for */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Vehicle ID used */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/** Player name/ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	/** Total race time */
	UPROPERTY(BlueprintReadOnly)
	float TotalTime = 0.0f;

	/** Best lap time */
	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	/** Number of laps completed */
	UPROPERTY(BlueprintReadOnly)
	int32 LapsCompleted = 0;

	/** Date recorded */
	UPROPERTY(BlueprintReadOnly)
	FDateTime RecordedDate;

	/** Replay frames */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGReplayFrame> Frames;

	/** Frame rate used for recording */
	UPROPERTY(BlueprintReadOnly)
	float RecordingFPS = 30.0f;

	/** Is this a valid replay */
	bool IsValid() const { return ReplayID.IsValid() && Frames.Num() > 0; }

	/** Get total duration */
	float GetDuration() const { return Frames.Num() > 0 ? Frames.Last().Timestamp : 0.0f; }

	/** Get frame at time (interpolated) */
	FMGReplayFrame GetFrameAtTime(float Time) const;

	/** Get closest frame index */
	int32 GetFrameIndexAtTime(float Time) const;
};

/**
 * Ghost racer configuration
 */
USTRUCT(BlueprintType)
struct FMGGhostConfig
{
	GENERATED_BODY()

	/** Replay data to use */
	UPROPERTY(BlueprintReadWrite)
	FMGReplayData ReplayData;

	/** Ghost transparency (0-1) */
	UPROPERTY(BlueprintReadWrite)
	float Transparency = 0.5f;

	/** Ghost color tint */
	UPROPERTY(BlueprintReadWrite)
	FLinearColor GhostColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** Show distance/time delta to player */
	UPROPERTY(BlueprintReadWrite)
	bool bShowDelta = true;

	/** Enable collision with ghost */
	UPROPERTY(BlueprintReadWrite)
	bool bEnableCollision = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReplayStateChanged, EMGReplayState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingComplete, const FMGReplayData&, ReplayData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackComplete, const FMGReplayData&, ReplayData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlaybackProgress, float, CurrentTime, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGhostSpawned, AMGGhostRacerActor*, Ghost, const FMGReplayData&, ReplayData);

/**
 * Replay Subsystem
 *
 * World subsystem providing replay recording and playback functionality.
 * This is a per-world subsystem that handles the current race/session replay.
 *
 * USAGE:
 * @code
 * // Get the subsystem
 * UMGReplaySubsystem* ReplaySystem = GetWorld()->GetSubsystem<UMGReplaySubsystem>();
 *
 * // Start recording when race begins
 * ReplaySystem->StartRecording(PlayerVehicle, TrackID, VehicleID);
 *
 * // Stop recording when race ends
 * FMGReplayData Replay = ReplaySystem->StopRecording();
 *
 * // Save for later
 * ReplaySystem->SaveReplay(Replay, true); // bUploadToServer = true
 *
 * // Spawn ghost for time attack
 * FMGGhostConfig Config;
 * Config.ReplayData = ReplaySystem->GetPersonalBestReplay(TrackID);
 * AMGGhostRacerActor* Ghost = ReplaySystem->SpawnGhost(Config);
 * @endcode
 *
 * THREADING:
 * All methods are safe to call from the game thread. Heavy operations
 * (save/load, compression) are performed asynchronously where possible.
 *
 * @see UMGReplayBufferSubsystem - For continuous instant-replay buffer
 * @see UMGReplayRecordingComponent - For per-vehicle recording
 */
UCLASS()
class MIDNIGHTGRIND_API UMGReplaySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float MGDeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when replay state changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnReplayStateChanged OnReplayStateChanged;

	/** Called when recording completes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRecordingComplete OnRecordingComplete;

	/** Called when playback completes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlaybackComplete OnPlaybackComplete;

	/** Called during playback with progress */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlaybackProgress OnPlaybackProgress;

	/** Called when ghost is spawned */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGhostSpawned OnGhostSpawned;

	// ==========================================
	// RECORDING
	// ==========================================

	/** Start recording a replay */
	UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
	void StartRecording(AActor* TargetActor, FName TrackID, FName VehicleID);

	/** Stop recording and finalize replay */
	UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
	FMGReplayData StopRecording();

	/** Cancel recording without saving */
	UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
	void CancelRecording();

	/** Record a single frame */
	UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
	void RecordFrame(const FMGReplayFrame& Frame);

	/** Is currently recording */
	UFUNCTION(BlueprintPure, Category = "Replay|Recording")
	bool IsRecording() const { return CurrentState == EMGReplayState::Recording; }

	/** Get recording duration */
	UFUNCTION(BlueprintPure, Category = "Replay|Recording")
	float GetRecordingDuration() const;

	// ==========================================
	// PLAYBACK
	// ==========================================

	/** Start replay playback */
	UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
	void StartPlayback(const FMGReplayData& ReplayData);

	/** Stop playback */
	UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
	void StopPlayback();

	/** Pause playback */
	UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
	void PausePlayback();

	/** Resume playback */
	UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
	void ResumePlayback();

	/** Set playback speed */
	UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
	void SetPlaybackSpeed(float Speed);

	/** Set playback mode */
	UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
	void SetPlaybackMode(EMGReplayPlaybackMode Mode);

	/** Seek to time */
	UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
	void SeekToTime(float Time);

	/** Seek by frames (for frame-by-frame mode) */
	UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
	void SeekByFrames(int32 FrameDelta);

	/** Get current playback time */
	UFUNCTION(BlueprintPure, Category = "Replay|Playback")
	float GetPlaybackTime() const { return PlaybackTime; }

	/** Get playback progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Replay|Playback")
	float GetPlaybackProgress() const;

	/** Is currently playing */
	UFUNCTION(BlueprintPure, Category = "Replay|Playback")
	bool IsPlaying() const { return CurrentState == EMGReplayState::Playing; }

	/** Is paused */
	UFUNCTION(BlueprintPure, Category = "Replay|Playback")
	bool IsPaused() const { return CurrentState == EMGReplayState::Paused; }

	/** Get current playback frame (interpolated) */
	UFUNCTION(BlueprintPure, Category = "Replay|Playback")
	FMGReplayFrame GetCurrentPlaybackFrame() const;

	// ==========================================
	// GHOST RACERS
	// ==========================================

	/** Spawn a ghost racer from replay data */
	UFUNCTION(BlueprintCallable, Category = "Replay|Ghost")
	AMGGhostRacerActor* SpawnGhost(const FMGGhostConfig& Config);

	/** Spawn personal best ghost */
	UFUNCTION(BlueprintCallable, Category = "Replay|Ghost")
	AMGGhostRacerActor* SpawnPersonalBestGhost(FName TrackID);

	/** Spawn world record ghost */
	UFUNCTION(BlueprintCallable, Category = "Replay|Ghost")
	AMGGhostRacerActor* SpawnWorldRecordGhost(FName TrackID);

	/** Despawn all ghosts */
	UFUNCTION(BlueprintCallable, Category = "Replay|Ghost")
	void DespawnAllGhosts();

	/** Despawn specific ghost */
	UFUNCTION(BlueprintCallable, Category = "Replay|Ghost")
	void DespawnGhost(AMGGhostRacerActor* Ghost);

	/** Get all active ghosts */
	UFUNCTION(BlueprintPure, Category = "Replay|Ghost")
	TArray<AMGGhostRacerActor*> GetActiveGhosts() const { return ActiveGhosts; }

	/** Get delta time to ghost */
	UFUNCTION(BlueprintPure, Category = "Replay|Ghost")
	float GetDeltaToGhost(AMGGhostRacerActor* Ghost, float PlayerDistance) const;

	// ==========================================
	// STORAGE
	// ==========================================

	/** Save replay to storage */
	UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
	bool SaveReplay(const FMGReplayData& ReplayData, bool bUploadToServer = false);

	/** Load replay from storage */
	UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
	FMGReplayData LoadReplay(FGuid ReplayID);

	/** Delete replay from storage */
	UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
	bool DeleteReplay(FGuid ReplayID);

	/** Get all saved replays for track */
	UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
	TArray<FMGReplayData> GetSavedReplays(FName TrackID);

	/** Get personal best replay for track */
	UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
	FMGReplayData GetPersonalBestReplay(FName TrackID);

	// ==========================================
	// STATE
	// ==========================================

	/** Get current replay state */
	UFUNCTION(BlueprintPure, Category = "Replay")
	EMGReplayState GetCurrentState() const { return CurrentState; }

	/** Get current playback mode */
	UFUNCTION(BlueprintPure, Category = "Replay")
	EMGReplayPlaybackMode GetPlaybackMode() const { return CurrentPlaybackMode; }

	/** Get current playback speed */
	UFUNCTION(BlueprintPure, Category = "Replay")
	float GetPlaybackSpeed() const { return PlaybackSpeed; }

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Recording frame rate */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float RecordingFPS = 30.0f;

	/** Maximum recording duration (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float MaxRecordingDuration = 600.0f;

	/** Ghost actor class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AMGGhostRacerActor> GhostActorClass;

	/** Default ghost transparency */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float DefaultGhostTransparency = 0.5f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current replay state */
	EMGReplayState CurrentState = EMGReplayState::Idle;

	/** Current playback mode */
	EMGReplayPlaybackMode CurrentPlaybackMode = EMGReplayPlaybackMode::Normal;

	/** Playback speed multiplier */
	float PlaybackSpeed = 1.0f;

	/** Current playback time */
	float PlaybackTime = 0.0f;

	/** Recording accumulator */
	float RecordingAccumulator = 0.0f;

	/** Recording frame interval */
	float RecordingInterval = 1.0f / 30.0f;

	// ==========================================
	// RECORDING DATA
	// ==========================================

	/** Current recording data */
	FMGReplayData CurrentRecording;

	/** Actor being recorded */
	UPROPERTY()
	TWeakObjectPtr<AActor> RecordingTarget;

	// ==========================================
	// PLAYBACK DATA
	// ==========================================

	/** Data being played back */
	FMGReplayData CurrentPlaybackData;

	/** Active ghost racers */
	UPROPERTY()
	TArray<AMGGhostRacerActor*> ActiveGhosts;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update recording */
	void UpdateRecording(float MGDeltaTime);

	/** Update playback */
	void UpdatePlayback(float MGDeltaTime);

	/** Update ghost racers */
	void UpdateGhosts(float MGDeltaTime);

	/** Set replay state */
	void SetState(EMGReplayState NewState);

	/** Compress replay data for storage */
	TArray<uint8> CompressReplayData(const FMGReplayData& Data);

	/** Decompress replay data */
	FMGReplayData DecompressReplayData(const TArray<uint8>& CompressedData);

	/** Generate replay file path */
	FString GetReplayFilePath(FGuid ReplayID) const;
};

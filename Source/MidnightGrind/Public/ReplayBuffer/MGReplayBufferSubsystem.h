// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGReplayBufferSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * This file defines the Replay Buffer Subsystem for Midnight Grind. It provides
 * a comprehensive replay recording and playback system, including instant replay,
 * cinematic camera controls, and video export functionality.
 *
 * KEY CONCEPTS FOR ENTRY-LEVEL DEVELOPERS:
 *
 * 1. REPLAY BUFFER vs. REPLAY RECORDING:
 *    - BUFFER: Continuously records the last N seconds (circular buffer).
 *      When it fills up, old frames are overwritten. Used for instant replay.
 *    - RECORDING: A complete saved replay from start to finish (like a race
 *      recording). Can be saved to disk and loaded later.
 *
 * 2. FRAME-BASED RECORDING:
 *    - The replay system captures "snapshots" of the game state every frame.
 *    - Each FMGReplayFrame stores: vehicle positions, velocities, inputs,
 *      camera position, and timing information.
 *    - Keyframes (every N frames) store complete data; intermediate frames
 *      may use delta compression to save memory.
 *
 * 3. VEHICLE SNAPSHOTS (FMGVehicleSnapshot):
 *    - Complete state of a vehicle at a moment in time.
 *    - Includes: Transform (position/rotation), velocity, throttle, brake,
 *      steering, gear, RPM, drift state, nitro, and wheel data.
 *    - This allows perfect recreation of vehicle physics during playback.
 *
 * 4. INTERPOLATION:
 *    - When playing back, the system interpolates between recorded frames
 *      to create smooth motion even if recording framerate differs from
 *      playback framerate.
 *    - LerpSnapshot() blends two vehicle states together for smooth transitions.
 *
 * 5. REPLAY EVENTS:
 *    - Significant moments are marked with FMGReplayEvent (overtakes, crashes,
 *      lap completions, etc.).
 *    - Events have an "importance score" to identify the most exciting moments.
 *    - Used for highlight reels and "jump to event" navigation.
 *
 * 6. CAMERA MODES:
 *    - Multiple ways to view the replay: FollowCar, Cinematic, TrackSide,
 *      Helicopter, FreeCam, etc.
 *    - Director mode auto-switches cameras for broadcast-style viewing.
 *    - Photo mode lets players freeze time and position the camera freely.
 *
 * 7. CAMERA KEYFRAMES:
 *    - For cinematic sequences, users can place camera keyframes.
 *    - The system interpolates between keyframes for smooth camera movements.
 *    - Includes FOV, depth of field, and post-processing effects.
 *
 * 8. CIRCULAR BUFFER:
 *    - The replay buffer is circular - when full, oldest frames are discarded.
 *    - Controlled by MaxBufferDurationSeconds and MaxBufferSizeBytes.
 *    - Efficient for instant replay (always have last N seconds available).
 *
 * 9. EXPORT:
 *    - Replays can be exported to various formats: Video (MP4), GIF, Image
 *      Sequence, or native Replay File format.
 *    - Export settings control resolution, framerate, bitrate, and more.
 *
 * 10. INSTANT REPLAY:
 *     - Quick playback of recent action (like sports broadcasts).
 *     - TriggerInstantReplay() pauses game and shows last N seconds.
 *     - Often triggered after exciting moments (finish line, big crash).
 *
 * MEMORY CONSIDERATIONS:
 * - Replay buffers can consume significant memory (hundreds of MB).
 * - Compression reduces memory but increases CPU usage.
 * - The system monitors buffer size and trims when necessary.
 *
 * TYPICAL WORKFLOW:
 * 1. Recording starts automatically when race begins
 * 2. Each tick, RecordFrame() captures current state
 * 3. Events are detected and marked (overtakes, laps, etc.)
 * 4. Race ends, recording is saved (or discarded)
 * 5. Player enters replay viewer, chooses camera mode
 * 6. Player can create clips, add camera keyframes
 * 7. Export to video file for sharing
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGReplayBufferSubsystem.generated.h"

// EMGReplayState - defined in Core/MGSharedTypes.h

// Replay camera modes
UENUM(BlueprintType)
enum class EMGReplayCameraMode : uint8
{
    FollowCar               UMETA(DisplayName = "Follow Car"),
    Cinematic               UMETA(DisplayName = "Cinematic"),
    TrackSide               UMETA(DisplayName = "Track Side"),
    Helicopter              UMETA(DisplayName = "Helicopter"),
    Bumper                  UMETA(DisplayName = "Bumper Cam"),
    Hood                    UMETA(DisplayName = "Hood Cam"),
    Cockpit                 UMETA(DisplayName = "Cockpit"),
    FreeCam                 UMETA(DisplayName = "Free Camera"),
    Drone                   UMETA(DisplayName = "Drone"),
    Director                UMETA(DisplayName = "Director"),
    Orbit                   UMETA(DisplayName = "Orbit"),
    Photo                   UMETA(DisplayName = "Photo Mode")
};

// Replay export formats
UENUM(BlueprintType)
enum class EMGReplayExportFormat : uint8
{
    Video                   UMETA(DisplayName = "Video"),
    GIF                     UMETA(DisplayName = "GIF"),
    ImageSequence           UMETA(DisplayName = "Image Sequence"),
    ReplayFile              UMETA(DisplayName = "Replay File"),
    Thumbnail               UMETA(DisplayName = "Thumbnail Only")
};

// Video quality presets
// EMGReplayQuality - REMOVED (duplicate)
// Canonical definition in: Core/MGSharedTypes.h

// Replay event types
UENUM(BlueprintType)
enum class EMGReplayEventType : uint8
{
    RaceStart               UMETA(DisplayName = "Race Start"),
    RaceFinish              UMETA(DisplayName = "Race Finish"),
    LapComplete             UMETA(DisplayName = "Lap Complete"),
    Overtake                UMETA(DisplayName = "Overtake"),
    Collision               UMETA(DisplayName = "Collision"),
    DriftStart              UMETA(DisplayName = "Drift Start"),
    DriftEnd                UMETA(DisplayName = "Drift End"),
    BigAir                  UMETA(DisplayName = "Big Air"),
    TrickLanded             UMETA(DisplayName = "Trick Landed"),
    NearMiss                UMETA(DisplayName = "Near Miss"),
    NitroActivated          UMETA(DisplayName = "Nitro Activated"),
    BestSector              UMETA(DisplayName = "Best Sector"),
    PersonalBest            UMETA(DisplayName = "Personal Best"),
    Wreck                   UMETA(DisplayName = "Wreck"),
    Respawn                 UMETA(DisplayName = "Respawn"),
    ItemPickup              UMETA(DisplayName = "Item Pickup"),
    ItemUsed                UMETA(DisplayName = "Item Used"),
    PhotoOpportunity        UMETA(DisplayName = "Photo Opportunity")
};

// Vehicle transform snapshot
USTRUCT(BlueprintType)
struct FMGVehicleSnapshot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform Transform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector AngularVelocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Throttle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Brake = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Steering = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentGear = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RPM = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDrifting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DriftAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNitroActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NitroAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAirborne = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> WheelRotations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> SuspensionCompressions;
};

// Replay frame data
USTRUCT(BlueprintType)
struct FMGReplayFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FrameNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MGDeltaTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<int32, FMGVehicleSnapshot> VehicleSnapshots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTransform> DynamicObjectTransforms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform CameraTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CameraFOV = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bKeyFrame = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CompressedDataSize = 0;
};

// Replay event marker
USTRUCT(BlueprintType)
struct FMGReplayEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid EventId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGReplayEventType EventType = EMGReplayEventType::RaceStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FrameNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 VehicleId = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ImportanceScore = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecommendedCameraDistance = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGReplayCameraMode RecommendedCameraMode = EMGReplayCameraMode::FollowCar;
};

// Replay clip definition
USTRUCT(BlueprintType)
struct FMGReplayClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ClipId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StartFrame = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EndFrame = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGReplayCameraMode CameraMode = EMGReplayCameraMode::FollowCar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FocusVehicleId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGReplayEvent> ContainedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFavorite = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedAt;
};

// Replay recording session
USTRUCT(BlueprintType)
struct FMGReplayRecording
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RecordingId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RecordingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime RecordedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TrackId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TrackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GameMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ParticipantNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> VehicleIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalFrames = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 FileSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ThumbnailPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGReplayEvent> Events;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGReplayClip> SavedClips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Version = 1;

    UPROPERTY()
    TArray<FMGReplayFrame> Frames;
};

// Camera keyframe for cinematic playback
USTRUCT(BlueprintType)
struct FMGCameraKeyframe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Time = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform Transform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FOV = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FocusDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aperture = 2.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DepthOfField = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MotionBlur = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChromaticAberration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VignetteIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetVehicleId = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInterpolateToNext = true;
};

// Replay buffer configuration
USTRUCT(BlueprintType)
struct FMGReplayConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxBufferDurationSeconds = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 KeyFrameInterval = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompressFrames = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRecordAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoDetectEvents = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InstantReplayDuration = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MaxBufferSizeBytes = 512 * 1024 * 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCircularBuffer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinEventImportance = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSavedReplays = 50;
};

// Export settings
USTRUCT(BlueprintType)
struct FMGReplayExportSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGReplayExportFormat Format = EMGReplayExportFormat::Video;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGReplayQuality Quality = EMGReplayQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Width = 1920;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Height = 1080;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Bitrate = 20000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIncludeUI = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIncludeAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlaybackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OutputPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAddWatermark = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString WatermarkText;
};

// Replay playback state
USTRUCT(BlueprintType)
struct FMGReplayPlaybackState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGReplayState State = EMGReplayState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentFrame = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlaybackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGReplayCameraMode CameraMode = EMGReplayCameraMode::FollowCar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FocusVehicleId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalFrames = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnRecordingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRecordingStopped, const FMGReplayRecording&, Recording);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPlaybackStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPlaybackStopped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPlaybackPaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPlaybackResumed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPlaybackProgress, float, CurrentTime, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnEventDetected, const FMGReplayEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnClipCreated, const FMGReplayClip&, Clip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnExportProgress, float, Progress, const FString&, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnExportComplete, const FString&, FilePath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnExportFailed, const FString&, Error);

/**
 * UMGReplayBufferSubsystem
 * Comprehensive replay recording and playback system
 * Supports instant replay, cinematic cameras, and video export
 */
UCLASS()
class MIDNIGHTGRIND_API UMGReplayBufferSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Recording control
    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void StartRecording(const FString& RecordingName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void StopRecording();

    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void PauseRecording();

    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void ResumeRecording();

    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void DiscardRecording();

    UFUNCTION(BlueprintPure, Category = "Replay|Recording")
    bool IsRecording() const { return CurrentState == EMGReplayState::Recording; }

    UFUNCTION(BlueprintPure, Category = "Replay|Recording")
    float GetRecordingDuration() const;

    UFUNCTION(BlueprintPure, Category = "Replay|Recording")
    int32 GetRecordedFrameCount() const;

    // Frame recording
    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void RecordFrame(float MGDeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void RecordVehicleState(int32 VehicleId, const FMGVehicleSnapshot& Snapshot);

    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void RecordCameraState(const FTransform& Transform, float FOV);

    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void RecordEvent(const FMGReplayEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Replay|Recording")
    void RecordEventSimple(EMGReplayEventType EventType, int32 VehicleId, const FString& Description);

    // Playback control
    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void StartPlayback();

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void StartPlaybackFromRecording(const FMGReplayRecording& Recording);

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void StopPlayback();

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void PausePlayback();

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void ResumePlayback();

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void TogglePause();

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void SetPlaybackSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void SeekToTime(float Time);

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void SeekToFrame(int32 Frame);

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void SeekToEvent(const FGuid& EventId);

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void SkipForward(float Seconds);

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void SkipBackward(float Seconds);

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void JumpToNextEvent();

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void JumpToPreviousEvent();

    UFUNCTION(BlueprintCallable, Category = "Replay|Playback")
    void SetLooping(bool bLoop);

    UFUNCTION(BlueprintPure, Category = "Replay|Playback")
    bool IsPlaying() const { return CurrentState == EMGReplayState::Playing; }

    UFUNCTION(BlueprintPure, Category = "Replay|Playback")
    bool IsPaused() const { return CurrentState == EMGReplayState::Paused; }

    UFUNCTION(BlueprintPure, Category = "Replay|Playback")
    FMGReplayPlaybackState GetPlaybackState() const { return PlaybackState; }

    UFUNCTION(BlueprintPure, Category = "Replay|Playback")
    float GetPlaybackProgress() const;

    // Instant replay
    UFUNCTION(BlueprintCallable, Category = "Replay|InstantReplay")
    void TriggerInstantReplay(float Duration = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Replay|InstantReplay")
    void EndInstantReplay();

    UFUNCTION(BlueprintPure, Category = "Replay|InstantReplay")
    bool IsInInstantReplay() const { return bInInstantReplay; }

    // Camera control
    UFUNCTION(BlueprintCallable, Category = "Replay|Camera")
    void SetCameraMode(EMGReplayCameraMode Mode);

    UFUNCTION(BlueprintCallable, Category = "Replay|Camera")
    void SetFocusVehicle(int32 VehicleId);

    UFUNCTION(BlueprintCallable, Category = "Replay|Camera")
    void CycleCamera();

    UFUNCTION(BlueprintCallable, Category = "Replay|Camera")
    void CycleFocusVehicle();

    UFUNCTION(BlueprintCallable, Category = "Replay|Camera")
    void SetFreeCamTransform(const FTransform& Transform);

    UFUNCTION(BlueprintCallable, Category = "Replay|Camera")
    void AddCameraKeyframe(const FMGCameraKeyframe& Keyframe);

    UFUNCTION(BlueprintCallable, Category = "Replay|Camera")
    void ClearCameraKeyframes();

    UFUNCTION(BlueprintCallable, Category = "Replay|Camera")
    void PlayCinematicCamera();

    UFUNCTION(BlueprintPure, Category = "Replay|Camera")
    EMGReplayCameraMode GetCurrentCameraMode() const { return PlaybackState.CameraMode; }

    UFUNCTION(BlueprintPure, Category = "Replay|Camera")
    FTransform GetCurrentCameraTransform() const;

    // Clip management
    UFUNCTION(BlueprintCallable, Category = "Replay|Clips")
    FMGReplayClip CreateClip(const FString& ClipName, float StartTime, float EndTime);

    UFUNCTION(BlueprintCallable, Category = "Replay|Clips")
    FMGReplayClip CreateClipFromEvent(const FMGReplayEvent& Event, float PaddingBefore = 3.0f, float PaddingAfter = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Replay|Clips")
    void DeleteClip(const FGuid& ClipId);

    UFUNCTION(BlueprintCallable, Category = "Replay|Clips")
    void PlayClip(const FMGReplayClip& Clip);

    UFUNCTION(BlueprintCallable, Category = "Replay|Clips")
    void SetClipFavorite(const FGuid& ClipId, bool bFavorite);

    UFUNCTION(BlueprintPure, Category = "Replay|Clips")
    TArray<FMGReplayClip> GetAllClips() const;

    UFUNCTION(BlueprintPure, Category = "Replay|Clips")
    TArray<FMGReplayClip> GetFavoriteClips() const;

    // Event management
    UFUNCTION(BlueprintPure, Category = "Replay|Events")
    TArray<FMGReplayEvent> GetAllEvents() const;

    UFUNCTION(BlueprintPure, Category = "Replay|Events")
    TArray<FMGReplayEvent> GetEventsByType(EMGReplayEventType EventType) const;

    UFUNCTION(BlueprintPure, Category = "Replay|Events")
    TArray<FMGReplayEvent> GetEventsInTimeRange(float StartTime, float EndTime) const;

    UFUNCTION(BlueprintPure, Category = "Replay|Events")
    FMGReplayEvent GetNearestEvent(float Time) const;

    UFUNCTION(BlueprintCallable, Category = "Replay|Events")
    void GenerateHighlightReel(float MaxDuration = 60.0f);

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
    bool SaveRecording(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
    bool LoadRecording(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
    bool DeleteSavedRecording(const FString& SlotName);

    UFUNCTION(BlueprintPure, Category = "Replay|Storage")
    TArray<FString> GetSavedRecordingNames() const;

    UFUNCTION(BlueprintPure, Category = "Replay|Storage")
    FMGReplayRecording GetSavedRecordingInfo(const FString& SlotName) const;

    UFUNCTION(BlueprintCallable, Category = "Replay|Storage")
    void CleanupOldRecordings();

    // Export
    UFUNCTION(BlueprintCallable, Category = "Replay|Export")
    void ExportReplay(const FMGReplayExportSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Replay|Export")
    void ExportClip(const FMGReplayClip& Clip, const FMGReplayExportSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Replay|Export")
    void CancelExport();

    UFUNCTION(BlueprintPure, Category = "Replay|Export")
    bool IsExporting() const { return CurrentState == EMGReplayState::Exporting; }

    UFUNCTION(BlueprintPure, Category = "Replay|Export")
    float GetExportProgress() const { return ExportProgress; }

    UFUNCTION(BlueprintCallable, Category = "Replay|Export")
    void CaptureScreenshot(const FString& FilePath);

    // Frame access
    UFUNCTION(BlueprintPure, Category = "Replay|Frame")
    FMGReplayFrame GetFrame(int32 FrameNumber) const;

    UFUNCTION(BlueprintPure, Category = "Replay|Frame")
    FMGReplayFrame GetFrameAtTime(float Time) const;

    UFUNCTION(BlueprintPure, Category = "Replay|Frame")
    FMGVehicleSnapshot GetVehicleSnapshotAtTime(int32 VehicleId, float Time) const;

    UFUNCTION(BlueprintPure, Category = "Replay|Frame")
    FMGVehicleSnapshot InterpolateVehicleSnapshot(int32 VehicleId, float Time) const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Replay|Config")
    void ApplyConfig(const FMGReplayConfig& Config);

    UFUNCTION(BlueprintPure, Category = "Replay|Config")
    FMGReplayConfig GetConfig() const { return ReplayConfig; }

    UFUNCTION(BlueprintCallable, Category = "Replay|Config")
    void SetMaxBufferDuration(float Seconds);

    UFUNCTION(BlueprintCallable, Category = "Replay|Config")
    void SetTargetFrameRate(float FPS);

    UFUNCTION(BlueprintPure, Category = "Replay|Config")
    int64 GetCurrentBufferSize() const { return CurrentBufferSize; }

    // Track/Vehicle info
    UFUNCTION(BlueprintCallable, Category = "Replay|Info")
    void SetTrackInfo(const FString& TrackId, const FString& TrackName);

    UFUNCTION(BlueprintCallable, Category = "Replay|Info")
    void SetGameMode(const FString& Mode);

    UFUNCTION(BlueprintCallable, Category = "Replay|Info")
    void RegisterParticipant(int32 VehicleId, const FString& PlayerName, const FString& VehicleType);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnRecordingStarted OnRecordingStarted;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnRecordingStopped OnRecordingStopped;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnPlaybackStarted OnPlaybackStarted;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnPlaybackStopped OnPlaybackStopped;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnPlaybackPaused OnPlaybackPaused;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnPlaybackResumed OnPlaybackResumed;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnPlaybackProgress OnPlaybackProgress;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnEventDetected OnEventDetected;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnClipCreated OnClipCreated;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnExportProgress OnExportProgress;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnExportComplete OnExportComplete;

    UPROPERTY(BlueprintAssignable, Category = "Replay|Events")
    FMGOnExportFailed OnExportFailed;

protected:
    UPROPERTY()
    FMGReplayRecording CurrentRecording;

    UPROPERTY()
    FMGReplayRecording PlaybackRecording;

    UPROPERTY()
    FMGReplayPlaybackState PlaybackState;

    UPROPERTY()
    FMGReplayConfig ReplayConfig;

    UPROPERTY()
    EMGReplayState CurrentState = EMGReplayState::Idle;

    UPROPERTY()
    TArray<FMGCameraKeyframe> CinematicKeyframes;

    UPROPERTY()
    TMap<FString, FMGReplayRecording> SavedRecordings;

    UPROPERTY()
    bool bInInstantReplay = false;

    UPROPERTY()
    float InstantReplayStartTime = 0.0f;

    UPROPERTY()
    float ExportProgress = 0.0f;

    UPROPERTY()
    int64 CurrentBufferSize = 0;

    UPROPERTY()
    float RecordingStartTime = 0.0f;

    UPROPERTY()
    int32 CurrentRecordingFrame = 0;

    UPROPERTY()
    TMap<int32, FString> ParticipantNames;

    UPROPERTY()
    TMap<int32, FString> ParticipantVehicles;

    FTimerHandle PlaybackTimerHandle;
    FTimerHandle RecordingTimerHandle;
    FTimerHandle ExportTimerHandle;

    FMGReplayFrame CurrentFrameData;
    FTransform FreeCamTransform;
    int32 CinematicKeyframeIndex = 0;
    float CinematicTime = 0.0f;

    void UpdatePlayback(float MGDeltaTime);
    void UpdateRecording(float MGDeltaTime);
    void ProcessExport();
    void TrimBuffer();
    void AnalyzeForEvents();
    FMGVehicleSnapshot LerpSnapshot(const FMGVehicleSnapshot& A, const FMGVehicleSnapshot& B, float Alpha) const;
    FTransform CalculateCameraTransform() const;
    int32 FindFrameAtTime(float Time) const;
    void CompressFrame(FMGReplayFrame& Frame);
    void DecompressFrame(FMGReplayFrame& Frame);
    FString GenerateThumbnail();
};

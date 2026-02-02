// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGInstantReplaySubsystem.h
 * @brief Instant Replay System for Capturing and Playback of Race Moments
 *
 * @section overview_ir Overview
 * The Instant Replay Subsystem provides a complete replay system for capturing
 * exciting moments during races, playing them back with cinematic camera angles,
 * generating highlight reels, and sharing clips with other players. Think of it
 * like the "Game DVR" feature on consoles, but built specifically for racing.
 *
 * @section concepts_ir Key Concepts for Beginners
 *
 * ### What is Instant Replay?
 * During gameplay, the system constantly records a "buffer" of the last N seconds
 * (typically 30 seconds). When something exciting happens - a crash, a near miss,
 * a perfect drift - you can trigger an instant replay to watch it again from
 * different camera angles, in slow motion, etc.
 *
 * ### Circular Buffer Recording
 * The replay buffer works like a circular tape recorder:
 * - It's always recording
 * - When it fills up, it overwrites the oldest data
 * - You only keep the most recent X seconds
 * - Memory usage stays constant
 *
 * This means you can ALWAYS replay the last 30 seconds, without having to
 * explicitly start recording.
 *
 * ### Replay Moments (FMGReplayMoment)
 * A "moment" is a timestamped event worth highlighting:
 * - Takedowns (crashing opponents)
 * - Near misses (close calls)
 * - Drift combos (high score drifts)
 * - Big air (jumps)
 * - Photo finishes
 * - Overtakes
 *
 * The system automatically captures moments when these events occur, scoring
 * them by "importance" to help generate highlight reels.
 *
 * ### Trigger Types (EMGReplayTriggerType)
 * What caused a moment to be captured:
 * - **Takedown**: You wrecked another car
 * - **NearMiss**: Nearly hit something at high speed
 * - **DriftCombo**: Executed a sick drift
 * - **BigAir**: Got serious airtime
 * - **PhotoFinish**: Won/lost by milliseconds
 * - **Crash**: Spectacular crash
 * - **Overtake**: Passed another racer
 * - **PoliceTakedown**: Took out a cop car
 * - **PursuitEscape**: Escaped from police
 * - **RaceFinish**: Crossed the finish line
 * - **Manual**: Player pressed replay button
 * - **Highlight**: Auto-detected as interesting
 *
 * ### Camera Types (EMGReplayCameraType)
 * Different viewpoints for replay playback:
 * - **Chase**: Behind the car (default gameplay view)
 * - **Bumper**: Front bumper view
 * - **Hood**: Hood-mounted camera
 * - **Cinematic**: Dramatic angles, follows action
 * - **Orbital**: Circles around the action
 * - **TrackSide**: Static cameras along the track
 * - **Helicopter**: Aerial view
 * - **Dramatic**: Extreme angles for impact moments
 * - **SlowMotion**: Auto-triggers slow-mo at key moments
 * - **Director**: AI-selected best angles
 * - **Free**: User-controlled camera
 *
 * ### Director Sequences (FMGDirectorSequence)
 * The "Director" feature automatically creates a cinematic edit:
 * - Selects the best moments
 * - Chooses appropriate camera angles
 * - Times cuts to the action
 * - Adds slow motion at dramatic moments
 *
 * ### Highlight Reels
 * Automatically generated "best of" compilations:
 * - Configurable duration and moment count
 * - Filters by moment type (crashes, drifts, etc.)
 * - Auto-selects best camera angles
 * - Smooth transitions between shots
 *
 * @section usage_ir Usage Examples
 *
 * @code
 * // Get the subsystem
 * UMGInstantReplaySubsystem* Replay = GetGameInstance()->GetSubsystem<UMGInstantReplaySubsystem>();
 *
 * // Start recording when race begins
 * Replay->StartRecording(RaceSessionId, EMGReplayQuality::High);
 *
 * // Capture moments during gameplay (called by game events)
 * void OnTakedown(const FString& PlayerId, FVector Location, float Speed)
 * {
 *     Replay->CaptureAutoMoment(
 *         EMGReplayTriggerType::Takedown,
 *         PlayerId,
 *         Location,
 *         Speed,
 *         1000  // Score value
 *     );
 * }
 *
 * // Trigger instant replay when player presses button
 * void OnReplayButtonPressed()
 * {
 *     Replay->TriggerInstantReplay(5.0f, EMGReplayCameraType::Cinematic);
 * }
 *
 * // Generate highlight reel after race
 * void OnRaceComplete()
 * {
 *     FMGHighlightReelConfig Config;
 *     Config.MaxDuration = 60.0f;
 *     Config.MaxMoments = 10;
 *     Config.bIncludeTakedowns = true;
 *     Config.bIncludeDrifts = true;
 *
 *     FMGDirectorSequence Highlights = Replay->GenerateHighlightReel(Config);
 *     Replay->PlayHighlightReel(Highlights);
 * }
 *
 * // Control playback
 * Replay->SetPlaybackSpeed(0.5f);  // Half speed
 * Replay->SetCamera(EMGReplayCameraType::Helicopter);
 * Replay->EnableSlowMotion(0.25f);  // Quarter speed
 *
 * // Save a replay for later
 * FString ReplayId = Replay->SaveReplay(FText::FromString("Epic Takedown Compilation"));
 *
 * // Share a clip
 * FString ShareUrl = Replay->ExportReplayClip(ReplayId, 10.0f, 20.0f);  // 10 second clip
 * @endcode
 *
 * @section events_ir Events to Listen For
 *
 * - **OnReplayMomentCaptured**: A new moment was recorded (update UI)
 * - **OnReplayStarted**: Replay playback began
 * - **OnReplayStopped**: Replay playback ended
 * - **OnReplayTimeUpdated**: Current playback time changed (for timeline UI)
 * - **OnCameraChanged**: Camera angle switched
 * - **OnReplaySaved**: Replay was saved to disk
 * - **OnHighlightReelGenerated**: Auto-highlights are ready
 * - **OnReplayShared**: Replay was shared (got URL)
 * - **OnSlowMotionTriggered**: Slow motion activated
 * - **OnReplayBookmarked**: User bookmarked a timestamp
 *
 * @section quality_ir Quality Settings
 *
 * **EMGReplayQuality levels:**
 * - **Low**: Minimal data, small files, lower fidelity
 * - **Medium**: Balanced quality and size
 * - **High**: Good quality, recommended for most uses
 * - **Ultra**: Maximum quality, large files
 * - **Cinematic**: Highest quality for content creation
 *
 * Quality affects:
 * - Recording frequency (snapshots per second)
 * - Data precision (position accuracy)
 * - File size when saved
 * - Memory usage during recording
 *
 * @section ui_ir UI Integration
 *
 * **During Recording:**
 * - Show recording indicator
 * - Display moment count
 * - Show recent moment notifications ("Nice Drift!")
 *
 * **During Playback:**
 * - Timeline scrubber with moment markers
 * - Camera selection buttons
 * - Speed controls (0.25x, 0.5x, 1x, 2x)
 * - Slow motion toggle
 * - Bookmark button
 *
 * **In Replay Browser:**
 * - Thumbnail previews
 * - Duration and date info
 * - View/like counts
 * - Share and delete options
 *
 * @section performance_ir Performance Considerations
 *
 * - Buffer duration affects memory usage (longer = more RAM)
 * - Higher quality = more CPU during recording
 * - Many concurrent moments = larger buffer size
 * - Consider reducing quality on lower-end hardware
 *
 * @see FMGReplayMoment For individual moment data structure
 * @see FMGReplayBuffer For buffer state information
 * @see FMGSavedReplay For saved replay metadata
 * @see FMGHighlightReelConfig For highlight generation settings
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "ReplayBuffer/MGReplayBufferSubsystem.h"
#include "MGInstantReplaySubsystem.generated.h"

/**
 * Replay trigger type
 */
UENUM(BlueprintType)
enum class EMGReplayTriggerType : uint8
{
	None				UMETA(DisplayName = "None"),
	Takedown			UMETA(DisplayName = "Takedown"),
	NearMiss			UMETA(DisplayName = "Near Miss"),
	DriftCombo			UMETA(DisplayName = "Drift Combo"),
	BigAir				UMETA(DisplayName = "Big Air"),
	PhotoFinish			UMETA(DisplayName = "Photo Finish"),
	Crash				UMETA(DisplayName = "Crash"),
	Overtake			UMETA(DisplayName = "Overtake"),
	PoliceTakedown		UMETA(DisplayName = "Police Takedown"),
	PursuitEscape		UMETA(DisplayName = "Pursuit Escape"),
	RaceFinish			UMETA(DisplayName = "Race Finish"),
	Manual				UMETA(DisplayName = "Manual"),
	Highlight			UMETA(DisplayName = "Auto Highlight")
};

/**
 * Camera angle type
 */
UENUM(BlueprintType)
enum class EMGReplayCameraType : uint8
{
	Chase				UMETA(DisplayName = "Chase Camera"),
	Bumper				UMETA(DisplayName = "Bumper Camera"),
	Hood				UMETA(DisplayName = "Hood Camera"),
	Cinematic			UMETA(DisplayName = "Cinematic"),
	Orbital				UMETA(DisplayName = "Orbital"),
	TrackSide			UMETA(DisplayName = "Track Side"),
	Helicopter			UMETA(DisplayName = "Helicopter"),
	Dramatic			UMETA(DisplayName = "Dramatic"),
	SlowMotion			UMETA(DisplayName = "Slow Motion"),
	Director			UMETA(DisplayName = "Director Cut"),
	Free				UMETA(DisplayName = "Free Camera")
};

/**
 * Replay status
 */
UENUM(BlueprintType)
enum class EMGReplayStatus : uint8
{
	Idle				UMETA(DisplayName = "Idle"),
	Recording			UMETA(DisplayName = "Recording"),
	Playing				UMETA(DisplayName = "Playing"),
	Paused				UMETA(DisplayName = "Paused"),
	Rewinding			UMETA(DisplayName = "Rewinding"),
	FastForward			UMETA(DisplayName = "Fast Forward"),
	Saving				UMETA(DisplayName = "Saving"),
	Loading				UMETA(DisplayName = "Loading")
};

// EMGReplayQuality - defined in Core/MGSharedTypes.h

/**
 * Replay moment
 */
USTRUCT(BlueprintType)
struct FMGReplayMoment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MomentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReplayTriggerType TriggerType = EMGReplayTriggerType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImportanceScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScoreValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGReplayCameraType> SuggestedCameras;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBookmarked = false;
};

/**
 * Replay buffer
 */
USTRUCT(BlueprintType)
struct FMGReplayBuffer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BufferId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BufferDuration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReplayStatus Status = EMGReplayStatus::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReplayMoment> CapturedMoments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrameCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReplayQuality Quality = EMGReplayQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCircularBuffer = true;
};

/**
 * Camera shot
 */
USTRUCT(BlueprintType)
struct FMGReplayCameraShot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShotId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReplayCameraType CameraType = EMGReplayCameraType::Chase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CameraOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaybackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseDepthOfField = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FocusDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Aperture = 2.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMotionBlur = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MotionBlurAmount = 0.5f;
};

/**
 * Director sequence
 */
USTRUCT(BlueprintType)
struct FMGDirectorSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SequenceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SequenceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReplayCameraShot> Shots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoGenerated = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EditCount = 0;
};

/**
 * Saved replay
 */
USTRUCT(BlueprintType)
struct FMGSavedReplay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ReplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RaceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 FileSizeBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RecordedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReplayQuality Quality = EMGReplayQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReplayMoment> Highlights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDirectorSequence DirectorCut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ViewCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LikeCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsShared = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ThumbnailPath;
};

/**
 * Highlight reel config
 */
USTRUCT(BlueprintType)
struct FMGHighlightReelConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxMoments = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinMomentImportance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeTakedowns = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeNearMisses = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeDrifts = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeAirtime = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeOvertakes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeFinish = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoSelectCameras = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionDuration = 0.5f;
};

// FMGReplayPlaybackState - Canonical definition in: ReplayBuffer/MGReplayBufferSubsystem.h (included above)

/**
 * Replay stats
 */
USTRUCT(BlueprintType)
struct FMGReplayStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReplaysRecorded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReplaysSaved = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReplaysShared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalMomentsCaptured = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalHighlightReelsCreated = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRecordedMinutes = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalStorageUsedBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLikesReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalViewsReceived = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReplayMomentCaptured, const FString&, PlayerId, const FMGReplayMoment&, Moment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReplayStarted, const FString&, ReplayId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReplayStopped, const FString&, ReplayId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReplayTimeUpdated, const FString&, ReplayId, float, CurrentTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCameraChanged, const FString&, ReplayId, EMGReplayCameraType, NewCamera);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReplaySaved, const FString&, ReplayId, const FString&, FilePath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighlightReelGenerated, const FMGDirectorSequence&, Sequence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReplayShared, const FString&, ReplayId, const FString&, ShareUrl);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlowMotionTriggered, const FString&, ReplayId, float, SlowMotionFactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReplayBookmarked, const FString&, ReplayId, float, Timestamp);

/**
 * Instant Replay Subsystem
 * Manages in-race replay capture, playback, and highlight generation
 */
UCLASS()
class MIDNIGHTGRIND_API UMGInstantReplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnReplayMomentCaptured OnReplayMomentCaptured;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnReplayStarted OnReplayStarted;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnReplayStopped OnReplayStopped;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnReplayTimeUpdated OnReplayTimeUpdated;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnCameraChanged OnCameraChanged;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnReplaySaved OnReplaySaved;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnHighlightReelGenerated OnHighlightReelGenerated;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnReplayShared OnReplayShared;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnSlowMotionTriggered OnSlowMotionTriggered;

	UPROPERTY(BlueprintAssignable, Category = "InstantReplay|Events")
	FOnReplayBookmarked OnReplayBookmarked;

	// Recording
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Recording")
	void StartRecording(const FString& SessionId, EMGReplayQuality Quality = EMGReplayQuality::High);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Recording")
	void StopRecording();

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Recording")
	void PauseRecording();

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Recording")
	void ResumeRecording();

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Recording")
	bool IsRecording() const;

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Recording")
	FMGReplayBuffer GetCurrentBuffer() const;

	// Moment Capture
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Moments")
	void CaptureMoment(const FMGReplayMoment& Moment);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Moments")
	void CaptureAutoMoment(EMGReplayTriggerType TriggerType, const FString& PlayerId, FVector Location, float Speed, int32 Score);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Moments")
	void BookmarkCurrentTime(const FString& Label);

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Moments")
	TArray<FMGReplayMoment> GetCapturedMoments() const;

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Moments")
	TArray<FMGReplayMoment> GetMomentsByType(EMGReplayTriggerType Type) const;

	// Instant Replay Playback
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Playback")
	void TriggerInstantReplay(float Duration = 5.0f, EMGReplayCameraType Camera = EMGReplayCameraType::Cinematic);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Playback")
	void PlayFromMoment(const FString& MomentId);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Playback")
	void PlayFromTime(float StartTime, float Duration = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Playback")
	void StopPlayback();

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Playback")
	void PausePlayback();

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Playback")
	void ResumePlayback();

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Playback")
	void SeekToTime(float Time);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Playback")
	void SetPlaybackSpeed(float Speed);

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Playback")
	bool IsPlaying() const;

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Playback")
	FMGReplayPlaybackState GetPlaybackState() const;

	// Camera Control
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Camera")
	void SetCamera(EMGReplayCameraType CameraType);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Camera")
	void CycleCamera();

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Camera")
	void SetCameraTarget(const FString& ActorId);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Camera")
	void SetFreeCamera(FVector Position, FRotator Rotation);

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Camera")
	EMGReplayCameraType GetCurrentCamera() const;

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Camera")
	TArray<EMGReplayCameraType> GetAvailableCameras() const;

	// Slow Motion
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|SlowMotion")
	void EnableSlowMotion(float Factor = 0.25f);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|SlowMotion")
	void DisableSlowMotion();

	UFUNCTION(BlueprintPure, Category = "InstantReplay|SlowMotion")
	bool IsSlowMotionActive() const;

	// Highlight Reel
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Highlights")
	FMGDirectorSequence GenerateHighlightReel(const FMGHighlightReelConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Highlights")
	FMGDirectorSequence GenerateAutoHighlights();

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Highlights")
	void PlayHighlightReel(const FMGDirectorSequence& Sequence);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Highlights")
	void AddShotToSequence(FMGDirectorSequence& Sequence, const FMGReplayCameraShot& Shot);

	// Saving/Loading
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Save")
	FString SaveReplay(const FText& ReplayName);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Save")
	bool LoadReplay(const FString& ReplayId);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Save")
	bool DeleteReplay(const FString& ReplayId);

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Save")
	TArray<FMGSavedReplay> GetSavedReplays() const;

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Save")
	FMGSavedReplay GetSavedReplay(const FString& ReplayId) const;

	// Sharing
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Share")
	FString ShareReplay(const FString& ReplayId);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Share")
	FString ExportReplayClip(const FString& ReplayId, float StartTime, float EndTime);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Share")
	void LikeReplay(const FString& ReplayId);

	// Stats
	UFUNCTION(BlueprintPure, Category = "InstantReplay|Stats")
	FMGReplayStats GetReplayStats() const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Config")
	void SetBufferDuration(float Duration);

	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Config")
	void SetDefaultQuality(EMGReplayQuality Quality);

	UFUNCTION(BlueprintPure, Category = "InstantReplay|Config")
	float GetBufferDuration() const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "InstantReplay|Update")
	void UpdateReplaySystem(float MGDeltaTime);

protected:
	void TickRecording(float MGDeltaTime);
	void TickPlayback(float MGDeltaTime);
	float CalculateMomentImportance(EMGReplayTriggerType Type, int32 Score) const;
	TArray<EMGReplayCameraType> GetSuggestedCameras(EMGReplayTriggerType Type) const;
	FMGReplayCameraShot CreateCameraShot(const FMGReplayMoment& Moment, EMGReplayCameraType Camera) const;
	FString GenerateMomentId() const;
	FString GenerateReplayId() const;
	FString GenerateShotId() const;

private:
	UPROPERTY()
	FMGReplayBuffer CurrentBuffer;

	UPROPERTY()
	FMGReplayPlaybackState PlaybackState;

	UPROPERTY()
	TArray<FMGSavedReplay> SavedReplays;

	UPROPERTY()
	FMGReplayStats Stats;

	UPROPERTY()
	FMGHighlightReelConfig DefaultHighlightConfig;

	UPROPERTY()
	EMGReplayQuality DefaultQuality = EMGReplayQuality::High;

	UPROPERTY()
	int32 MomentCounter = 0;

	UPROPERTY()
	int32 ReplayCounter = 0;

	UPROPERTY()
	int32 ShotCounter = 0;

	UPROPERTY()
	TArray<EMGReplayCameraType> CameraOrder;

	UPROPERTY()
	int32 CurrentCameraIndex = 0;

	FTimerHandle ReplayTickTimer;
};

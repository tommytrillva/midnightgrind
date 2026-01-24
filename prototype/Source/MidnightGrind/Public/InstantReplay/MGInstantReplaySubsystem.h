// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
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

/**
 * Replay quality
 */
UENUM(BlueprintType)
enum class EMGReplayQuality : uint8
{
	Low					UMETA(DisplayName = "Low"),
	Medium				UMETA(DisplayName = "Medium"),
	High				UMETA(DisplayName = "High"),
	Ultra				UMETA(DisplayName = "Ultra"),
	Cinematic			UMETA(DisplayName = "Cinematic")
};

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

/**
 * Replay playback state
 */
USTRUCT(BlueprintType)
struct FMGReplayPlaybackState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActiveReplayId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReplayStatus Status = EMGReplayStatus::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaybackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReplayCameraType ActiveCamera = EMGReplayCameraType::Chase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FocusActorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSlowMotion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowMotionFactor = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowHUD = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTimeline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentShotIndex = 0;
};

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
	void UpdateReplaySystem(float DeltaTime);

protected:
	void TickRecording(float DeltaTime);
	void TickPlayback(float DeltaTime);
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

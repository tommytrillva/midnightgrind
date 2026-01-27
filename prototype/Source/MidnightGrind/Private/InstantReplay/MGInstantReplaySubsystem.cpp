// Copyright Epic Games, Inc. All Rights Reserved.

#include "InstantReplay/MGInstantReplaySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGInstantReplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default buffer
	CurrentBuffer.BufferId = TEXT("MAIN_BUFFER");
	CurrentBuffer.BufferDuration = 30.0f;
	CurrentBuffer.bIsCircularBuffer = true;
	CurrentBuffer.Quality = EMGReplayQuality::High;
	CurrentBuffer.Status = EMGReplayStatus::Idle;

	// Initialize camera order for cycling
	CameraOrder.Add(EMGReplayCameraType::Chase);
	CameraOrder.Add(EMGReplayCameraType::Bumper);
	CameraOrder.Add(EMGReplayCameraType::Hood);
	CameraOrder.Add(EMGReplayCameraType::Cinematic);
	CameraOrder.Add(EMGReplayCameraType::Orbital);
	CameraOrder.Add(EMGReplayCameraType::Dramatic);
	CameraOrder.Add(EMGReplayCameraType::Helicopter);
	CameraOrder.Add(EMGReplayCameraType::TrackSide);
	CameraOrder.Add(EMGReplayCameraType::Free);

	// Initialize default highlight config
	DefaultHighlightConfig.MaxDuration = 60.0f;
	DefaultHighlightConfig.MaxMoments = 10;
	DefaultHighlightConfig.MinMomentImportance = 0.5f;
	DefaultHighlightConfig.bIncludeTakedowns = true;
	DefaultHighlightConfig.bIncludeNearMisses = true;
	DefaultHighlightConfig.bIncludeDrifts = true;
	DefaultHighlightConfig.bIncludeAirtime = true;
	DefaultHighlightConfig.bIncludeOvertakes = true;
	DefaultHighlightConfig.bIncludeFinish = true;
	DefaultHighlightConfig.bAutoSelectCameras = true;
	DefaultHighlightConfig.TransitionDuration = 0.5f;

	// Initialize playback state
	PlaybackState.Status = EMGReplayStatus::Idle;
	PlaybackState.PlaybackSpeed = 1.0f;
	PlaybackState.ActiveCamera = EMGReplayCameraType::Chase;

	// Start replay tick
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGInstantReplaySubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(ReplayTickTimer, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->UpdateReplaySystem(0.033f);
			}
		}, 0.033f, true);
	}
}

void UMGInstantReplaySubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReplayTickTimer);
	}

	StopRecording();
	StopPlayback();

	CurrentBuffer.CapturedMoments.Empty();
	SavedReplays.Empty();
	CameraOrder.Empty();

	Super::Deinitialize();
}

// Recording
void UMGInstantReplaySubsystem::StartRecording(const FString& SessionId, EMGReplayQuality Quality)
{
	if (CurrentBuffer.Status == EMGReplayStatus::Recording)
	{
		return;
	}

	CurrentBuffer.BufferId = SessionId;
	CurrentBuffer.Quality = Quality;
	CurrentBuffer.Status = EMGReplayStatus::Recording;
	CurrentBuffer.CurrentTime = 0.0f;
	CurrentBuffer.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CurrentBuffer.FrameCount = 0;
	CurrentBuffer.CapturedMoments.Empty();

	Stats.TotalReplaysRecorded++;
}

void UMGInstantReplaySubsystem::StopRecording()
{
	if (CurrentBuffer.Status != EMGReplayStatus::Recording)
	{
		return;
	}

	CurrentBuffer.EndTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CurrentBuffer.Status = EMGReplayStatus::Idle;

	float Duration = CurrentBuffer.EndTime - CurrentBuffer.StartTime;
	Stats.TotalRecordedMinutes += Duration / 60.0f;
}

void UMGInstantReplaySubsystem::PauseRecording()
{
	if (CurrentBuffer.Status == EMGReplayStatus::Recording)
	{
		CurrentBuffer.Status = EMGReplayStatus::Paused;
	}
}

void UMGInstantReplaySubsystem::ResumeRecording()
{
	if (CurrentBuffer.Status == EMGReplayStatus::Paused)
	{
		CurrentBuffer.Status = EMGReplayStatus::Recording;
	}
}

bool UMGInstantReplaySubsystem::IsRecording() const
{
	return CurrentBuffer.Status == EMGReplayStatus::Recording;
}

FMGReplayBuffer UMGInstantReplaySubsystem::GetCurrentBuffer() const
{
	return CurrentBuffer;
}

// Moment Capture
void UMGInstantReplaySubsystem::CaptureMoment(const FMGReplayMoment& Moment)
{
	FMGReplayMoment NewMoment = Moment;
	if (NewMoment.MomentId.IsEmpty())
	{
		NewMoment.MomentId = GenerateMomentId();
	}

	if (NewMoment.Timestamp == 0.0f)
	{
		NewMoment.Timestamp = CurrentBuffer.CurrentTime;
	}

	if (NewMoment.ImportanceScore == 0.0f)
	{
		NewMoment.ImportanceScore = CalculateMomentImportance(NewMoment.TriggerType, NewMoment.ScoreValue);
	}

	if (NewMoment.SuggestedCameras.Num() == 0)
	{
		NewMoment.SuggestedCameras = GetSuggestedCameras(NewMoment.TriggerType);
	}

	CurrentBuffer.CapturedMoments.Add(NewMoment);
	Stats.TotalMomentsCaptured++;

	OnReplayMomentCaptured.Broadcast(NewMoment.PlayerId, NewMoment);
}

void UMGInstantReplaySubsystem::CaptureAutoMoment(EMGReplayTriggerType TriggerType, const FString& PlayerId, FVector Location, float Speed, int32 Score)
{
	FMGReplayMoment Moment;
	Moment.MomentId = GenerateMomentId();
	Moment.TriggerType = TriggerType;
	Moment.Timestamp = CurrentBuffer.CurrentTime;
	Moment.PlayerId = PlayerId;
	Moment.Location = Location;
	Moment.Speed = Speed;
	Moment.ScoreValue = Score;
	Moment.ImportanceScore = CalculateMomentImportance(TriggerType, Score);
	Moment.SuggestedCameras = GetSuggestedCameras(TriggerType);
	Moment.Duration = 5.0f;

	// Set display text based on trigger type
	switch (TriggerType)
	{
		case EMGReplayTriggerType::Takedown:
			Moment.DisplayText = FText::FromString(TEXT("TAKEDOWN!"));
			break;
		case EMGReplayTriggerType::NearMiss:
			Moment.DisplayText = FText::FromString(TEXT("NEAR MISS!"));
			break;
		case EMGReplayTriggerType::DriftCombo:
			Moment.DisplayText = FText::FromString(TEXT("DRIFT COMBO!"));
			break;
		case EMGReplayTriggerType::BigAir:
			Moment.DisplayText = FText::FromString(TEXT("BIG AIR!"));
			break;
		case EMGReplayTriggerType::PhotoFinish:
			Moment.DisplayText = FText::FromString(TEXT("PHOTO FINISH!"));
			break;
		case EMGReplayTriggerType::Crash:
			Moment.DisplayText = FText::FromString(TEXT("CRASH!"));
			break;
		case EMGReplayTriggerType::Overtake:
			Moment.DisplayText = FText::FromString(TEXT("OVERTAKE!"));
			break;
		case EMGReplayTriggerType::PoliceTakedown:
			Moment.DisplayText = FText::FromString(TEXT("COP TAKEDOWN!"));
			break;
		case EMGReplayTriggerType::PursuitEscape:
			Moment.DisplayText = FText::FromString(TEXT("ESCAPED!"));
			break;
		case EMGReplayTriggerType::RaceFinish:
			Moment.DisplayText = FText::FromString(TEXT("FINISH!"));
			break;
		default:
			Moment.DisplayText = FText::FromString(TEXT("HIGHLIGHT"));
			break;
	}

	CaptureMoment(Moment);
}

void UMGInstantReplaySubsystem::BookmarkCurrentTime(const FString& Label)
{
	FMGReplayMoment Bookmark;
	Bookmark.MomentId = GenerateMomentId();
	Bookmark.TriggerType = EMGReplayTriggerType::Manual;
	Bookmark.Timestamp = CurrentBuffer.CurrentTime;
	Bookmark.DisplayText = FText::FromString(Label);
	Bookmark.bIsBookmarked = true;
	Bookmark.ImportanceScore = 1.0f;

	CaptureMoment(Bookmark);
	OnReplayBookmarked.Broadcast(CurrentBuffer.BufferId, Bookmark.Timestamp);
}

TArray<FMGReplayMoment> UMGInstantReplaySubsystem::GetCapturedMoments() const
{
	return CurrentBuffer.CapturedMoments;
}

TArray<FMGReplayMoment> UMGInstantReplaySubsystem::GetMomentsByType(EMGReplayTriggerType Type) const
{
	TArray<FMGReplayMoment> Result;
	for (const FMGReplayMoment& Moment : CurrentBuffer.CapturedMoments)
	{
		if (Moment.TriggerType == Type)
		{
			Result.Add(Moment);
		}
	}
	return Result;
}

// Instant Replay Playback
void UMGInstantReplaySubsystem::TriggerInstantReplay(float Duration, EMGReplayCameraType Camera)
{
	float StartTime = FMath::Max(0.0f, CurrentBuffer.CurrentTime - Duration);
	PlayFromTime(StartTime, Duration);
	SetCamera(Camera);
}

void UMGInstantReplaySubsystem::PlayFromMoment(const FString& MomentId)
{
	for (const FMGReplayMoment& Moment : CurrentBuffer.CapturedMoments)
	{
		if (Moment.MomentId == MomentId)
		{
			float StartTime = FMath::Max(0.0f, Moment.Timestamp - 2.0f);
			PlayFromTime(StartTime, Moment.Duration + 4.0f);

			if (Moment.SuggestedCameras.Num() > 0)
			{
				SetCamera(Moment.SuggestedCameras[0]);
			}
			return;
		}
	}
}

void UMGInstantReplaySubsystem::PlayFromTime(float StartTime, float Duration)
{
	if (CurrentBuffer.Status == EMGReplayStatus::Recording)
	{
		PauseRecording();
	}

	PlaybackState.ActiveReplayId = CurrentBuffer.BufferId;
	PlaybackState.Status = EMGReplayStatus::Playing;
	PlaybackState.CurrentTime = StartTime;
	PlaybackState.PlaybackSpeed = 1.0f;

	OnReplayStarted.Broadcast(CurrentBuffer.BufferId);
}

void UMGInstantReplaySubsystem::StopPlayback()
{
	if (PlaybackState.Status == EMGReplayStatus::Idle)
	{
		return;
	}

	PlaybackState.Status = EMGReplayStatus::Idle;
	PlaybackState.bIsSlowMotion = false;

	OnReplayStopped.Broadcast(PlaybackState.ActiveReplayId);

	// Resume recording if it was paused
	if (CurrentBuffer.Status == EMGReplayStatus::Paused)
	{
		ResumeRecording();
	}
}

void UMGInstantReplaySubsystem::PausePlayback()
{
	if (PlaybackState.Status == EMGReplayStatus::Playing)
	{
		PlaybackState.Status = EMGReplayStatus::Paused;
	}
}

void UMGInstantReplaySubsystem::ResumePlayback()
{
	if (PlaybackState.Status == EMGReplayStatus::Paused)
	{
		PlaybackState.Status = EMGReplayStatus::Playing;
	}
}

void UMGInstantReplaySubsystem::SeekToTime(float Time)
{
	PlaybackState.CurrentTime = FMath::Clamp(Time, 0.0f, CurrentBuffer.CurrentTime);
	OnReplayTimeUpdated.Broadcast(PlaybackState.ActiveReplayId, PlaybackState.CurrentTime);
}

void UMGInstantReplaySubsystem::SetPlaybackSpeed(float Speed)
{
	PlaybackState.PlaybackSpeed = FMath::Clamp(Speed, 0.1f, 4.0f);
}

bool UMGInstantReplaySubsystem::IsPlaying() const
{
	return PlaybackState.Status == EMGReplayStatus::Playing;
}

FMGReplayPlaybackState UMGInstantReplaySubsystem::GetPlaybackState() const
{
	return PlaybackState;
}

// Camera Control
void UMGInstantReplaySubsystem::SetCamera(EMGReplayCameraType CameraType)
{
	PlaybackState.ActiveCamera = CameraType;
	OnCameraChanged.Broadcast(PlaybackState.ActiveReplayId, CameraType);

	// Update camera index
	for (int32 i = 0; i < CameraOrder.Num(); i++)
	{
		if (CameraOrder[i] == CameraType)
		{
			CurrentCameraIndex = i;
			break;
		}
	}
}

void UMGInstantReplaySubsystem::CycleCamera()
{
	CurrentCameraIndex = (CurrentCameraIndex + 1) % CameraOrder.Num();
	SetCamera(CameraOrder[CurrentCameraIndex]);
}

void UMGInstantReplaySubsystem::SetCameraTarget(const FString& ActorId)
{
	PlaybackState.FocusActorId = ActorId;
}

void UMGInstantReplaySubsystem::SetFreeCamera(FVector Position, FRotator Rotation)
{
	PlaybackState.ActiveCamera = EMGReplayCameraType::Free;
	// Position and rotation would be applied to the actual camera actor
}

EMGReplayCameraType UMGInstantReplaySubsystem::GetCurrentCamera() const
{
	return PlaybackState.ActiveCamera;
}

TArray<EMGReplayCameraType> UMGInstantReplaySubsystem::GetAvailableCameras() const
{
	return CameraOrder;
}

// Slow Motion
void UMGInstantReplaySubsystem::EnableSlowMotion(float Factor)
{
	PlaybackState.bIsSlowMotion = true;
	PlaybackState.SlowMotionFactor = FMath::Clamp(Factor, 0.1f, 1.0f);
	PlaybackState.PlaybackSpeed = PlaybackState.SlowMotionFactor;

	OnSlowMotionTriggered.Broadcast(PlaybackState.ActiveReplayId, Factor);
}

void UMGInstantReplaySubsystem::DisableSlowMotion()
{
	PlaybackState.bIsSlowMotion = false;
	PlaybackState.PlaybackSpeed = 1.0f;
}

bool UMGInstantReplaySubsystem::IsSlowMotionActive() const
{
	return PlaybackState.bIsSlowMotion;
}

// Highlight Reel
FMGDirectorSequence UMGInstantReplaySubsystem::GenerateHighlightReel(const FMGHighlightReelConfig& Config)
{
	FMGDirectorSequence Sequence;
	Sequence.SequenceId = FString::Printf(TEXT("HIGHLIGHT_%lld"), FDateTime::Now().GetTicks());
	Sequence.SequenceName = FText::FromString(TEXT("Auto-Generated Highlights"));
	Sequence.bAutoGenerated = true;

	// Filter and sort moments by importance
	TArray<FMGReplayMoment> FilteredMoments;
	for (const FMGReplayMoment& Moment : CurrentBuffer.CapturedMoments)
	{
		if (Moment.ImportanceScore < Config.MinMomentImportance)
		{
			continue;
		}

		bool bInclude = false;
		switch (Moment.TriggerType)
		{
			case EMGReplayTriggerType::Takedown:
			case EMGReplayTriggerType::PoliceTakedown:
				bInclude = Config.bIncludeTakedowns;
				break;
			case EMGReplayTriggerType::NearMiss:
				bInclude = Config.bIncludeNearMisses;
				break;
			case EMGReplayTriggerType::DriftCombo:
				bInclude = Config.bIncludeDrifts;
				break;
			case EMGReplayTriggerType::BigAir:
				bInclude = Config.bIncludeAirtime;
				break;
			case EMGReplayTriggerType::Overtake:
				bInclude = Config.bIncludeOvertakes;
				break;
			case EMGReplayTriggerType::RaceFinish:
			case EMGReplayTriggerType::PhotoFinish:
				bInclude = Config.bIncludeFinish;
				break;
			default:
				bInclude = true;
				break;
		}

		if (bInclude)
		{
			FilteredMoments.Add(Moment);
		}
	}

	// Sort by importance
	FilteredMoments.Sort([](const FMGReplayMoment& A, const FMGReplayMoment& B)
	{
		return A.ImportanceScore > B.ImportanceScore;
	});

	// Take top moments
	int32 MomentCount = FMath::Min(Config.MaxMoments, FilteredMoments.Num());
	float TotalDuration = 0.0f;

	for (int32 i = 0; i < MomentCount && TotalDuration < Config.MaxDuration; i++)
	{
		const FMGReplayMoment& Moment = FilteredMoments[i];

		// Select camera
		EMGReplayCameraType Camera = EMGReplayCameraType::Cinematic;
		if (Config.bAutoSelectCameras && Moment.SuggestedCameras.Num() > 0)
		{
			Camera = Moment.SuggestedCameras[FMath::RandRange(0, Moment.SuggestedCameras.Num() - 1)];
		}

		FMGReplayCameraShot Shot = CreateCameraShot(Moment, Camera);
		Shot.StartTime = TotalDuration;

		if (TotalDuration + Shot.Duration + Config.TransitionDuration > Config.MaxDuration)
		{
			Shot.Duration = Config.MaxDuration - TotalDuration;
		}

		Shot.EndTime = Shot.StartTime + Shot.Duration;
		TotalDuration += Shot.Duration + Config.TransitionDuration;

		Sequence.Shots.Add(Shot);
	}

	Sequence.TotalDuration = TotalDuration;
	Stats.TotalHighlightReelsCreated++;

	OnHighlightReelGenerated.Broadcast(Sequence);
	return Sequence;
}

FMGDirectorSequence UMGInstantReplaySubsystem::GenerateAutoHighlights()
{
	return GenerateHighlightReel(DefaultHighlightConfig);
}

void UMGInstantReplaySubsystem::PlayHighlightReel(const FMGDirectorSequence& Sequence)
{
	if (Sequence.Shots.Num() == 0)
	{
		return;
	}

	PlaybackState.Status = EMGReplayStatus::Playing;
	PlaybackState.CurrentShotIndex = 0;
	PlaybackState.CurrentTime = 0.0f;
	PlaybackState.PlaybackSpeed = 1.0f;

	const FMGReplayCameraShot& FirstShot = Sequence.Shots[0];
	SetCamera(FirstShot.CameraType);
	SetPlaybackSpeed(FirstShot.PlaybackSpeed);

	OnReplayStarted.Broadcast(Sequence.SequenceId);
}

void UMGInstantReplaySubsystem::AddShotToSequence(FMGDirectorSequence& Sequence, const FMGReplayCameraShot& Shot)
{
	FMGReplayCameraShot NewShot = Shot;
	if (NewShot.ShotId.IsEmpty())
	{
		NewShot.ShotId = GenerateShotId();
	}

	NewShot.StartTime = Sequence.TotalDuration;
	NewShot.EndTime = NewShot.StartTime + NewShot.Duration;

	Sequence.Shots.Add(NewShot);
	Sequence.TotalDuration += NewShot.Duration;
	Sequence.EditCount++;
}

// Saving/Loading
FString UMGInstantReplaySubsystem::SaveReplay(const FText& ReplayName)
{
	FMGSavedReplay Replay;
	Replay.ReplayId = GenerateReplayId();
	Replay.ReplayName = ReplayName;
	Replay.Duration = CurrentBuffer.CurrentTime;
	Replay.RecordedDate = FDateTime::Now();
	Replay.Quality = CurrentBuffer.Quality;
	Replay.Highlights = CurrentBuffer.CapturedMoments;
	Replay.DirectorCut = GenerateAutoHighlights();

	SavedReplays.Add(Replay);
	Stats.TotalReplaysSaved++;

	OnReplaySaved.Broadcast(Replay.ReplayId, TEXT(""));
	return Replay.ReplayId;
}

bool UMGInstantReplaySubsystem::LoadReplay(const FString& ReplayId)
{
	for (const FMGSavedReplay& Replay : SavedReplays)
	{
		if (Replay.ReplayId == ReplayId)
		{
			CurrentBuffer.CapturedMoments = Replay.Highlights;
			CurrentBuffer.CurrentTime = Replay.Duration;
			return true;
		}
	}
	return false;
}

bool UMGInstantReplaySubsystem::DeleteReplay(const FString& ReplayId)
{
	for (int32 i = 0; i < SavedReplays.Num(); i++)
	{
		if (SavedReplays[i].ReplayId == ReplayId)
		{
			SavedReplays.RemoveAt(i);
			return true;
		}
	}
	return false;
}

TArray<FMGSavedReplay> UMGInstantReplaySubsystem::GetSavedReplays() const
{
	return SavedReplays;
}

FMGSavedReplay UMGInstantReplaySubsystem::GetSavedReplay(const FString& ReplayId) const
{
	for (const FMGSavedReplay& Replay : SavedReplays)
	{
		if (Replay.ReplayId == ReplayId)
		{
			return Replay;
		}
	}
	return FMGSavedReplay();
}

// Sharing
FString UMGInstantReplaySubsystem::ShareReplay(const FString& ReplayId)
{
	for (FMGSavedReplay& Replay : SavedReplays)
	{
		if (Replay.ReplayId == ReplayId)
		{
			Replay.bIsShared = true;
			Stats.TotalReplaysShared++;

			FString ShareUrl = FString::Printf(TEXT("https://midnightgrind.com/replay/%s"), *ReplayId);
			OnReplayShared.Broadcast(ReplayId, ShareUrl);
			return ShareUrl;
		}
	}
	return FString();
}

FString UMGInstantReplaySubsystem::ExportReplayClip(const FString& ReplayId, float StartTime, float EndTime)
{
	// Would generate a video file
	return FString::Printf(TEXT("replay_%s_%d_%d.mp4"), *ReplayId, FMath::RoundToInt(StartTime), FMath::RoundToInt(EndTime));
}

void UMGInstantReplaySubsystem::LikeReplay(const FString& ReplayId)
{
	for (FMGSavedReplay& Replay : SavedReplays)
	{
		if (Replay.ReplayId == ReplayId)
		{
			Replay.LikeCount++;
			Stats.TotalLikesReceived++;
			break;
		}
	}
}

// Stats
FMGReplayStats UMGInstantReplaySubsystem::GetReplayStats() const
{
	return Stats;
}

// Configuration
void UMGInstantReplaySubsystem::SetBufferDuration(float Duration)
{
	CurrentBuffer.BufferDuration = FMath::Clamp(Duration, 10.0f, 120.0f);
}

void UMGInstantReplaySubsystem::SetDefaultQuality(EMGReplayQuality Quality)
{
	DefaultQuality = Quality;
}

float UMGInstantReplaySubsystem::GetBufferDuration() const
{
	return CurrentBuffer.BufferDuration;
}

// Update
void UMGInstantReplaySubsystem::UpdateReplaySystem(float DeltaTime)
{
	TickRecording(DeltaTime);
	TickPlayback(DeltaTime);
}

// Protected
void UMGInstantReplaySubsystem::TickRecording(float DeltaTime)
{
	if (CurrentBuffer.Status != EMGReplayStatus::Recording)
	{
		return;
	}

	CurrentBuffer.CurrentTime += DeltaTime;
	CurrentBuffer.FrameCount++;

	// Handle circular buffer - remove old moments
	if (CurrentBuffer.bIsCircularBuffer)
	{
		float OldestAllowed = CurrentBuffer.CurrentTime - CurrentBuffer.BufferDuration;

		CurrentBuffer.CapturedMoments.RemoveAll([OldestAllowed](const FMGReplayMoment& Moment)
		{
			return Moment.Timestamp < OldestAllowed && !Moment.bIsBookmarked;
		});
	}
}

void UMGInstantReplaySubsystem::TickPlayback(float DeltaTime)
{
	if (PlaybackState.Status != EMGReplayStatus::Playing)
	{
		return;
	}

	PlaybackState.CurrentTime += DeltaTime * PlaybackState.PlaybackSpeed;
	OnReplayTimeUpdated.Broadcast(PlaybackState.ActiveReplayId, PlaybackState.CurrentTime);

	// Check if playback is complete
	if (PlaybackState.CurrentTime >= CurrentBuffer.CurrentTime)
	{
		StopPlayback();
	}
}

float UMGInstantReplaySubsystem::CalculateMomentImportance(EMGReplayTriggerType Type, int32 Score) const
{
	float BaseImportance = 0.5f;

	switch (Type)
	{
		case EMGReplayTriggerType::Takedown:
			BaseImportance = 0.9f;
			break;
		case EMGReplayTriggerType::NearMiss:
			BaseImportance = 0.6f;
			break;
		case EMGReplayTriggerType::DriftCombo:
			BaseImportance = 0.7f;
			break;
		case EMGReplayTriggerType::BigAir:
			BaseImportance = 0.8f;
			break;
		case EMGReplayTriggerType::PhotoFinish:
			BaseImportance = 1.0f;
			break;
		case EMGReplayTriggerType::Crash:
			BaseImportance = 0.85f;
			break;
		case EMGReplayTriggerType::Overtake:
			BaseImportance = 0.65f;
			break;
		case EMGReplayTriggerType::PoliceTakedown:
			BaseImportance = 0.95f;
			break;
		case EMGReplayTriggerType::PursuitEscape:
			BaseImportance = 0.9f;
			break;
		case EMGReplayTriggerType::RaceFinish:
			BaseImportance = 0.95f;
			break;
		default:
			BaseImportance = 0.5f;
			break;
	}

	// Score bonus
	float ScoreBonus = FMath::Min(Score / 10000.0f, 0.1f);

	return FMath::Clamp(BaseImportance + ScoreBonus, 0.0f, 1.0f);
}

TArray<EMGReplayCameraType> UMGInstantReplaySubsystem::GetSuggestedCameras(EMGReplayTriggerType Type) const
{
	TArray<EMGReplayCameraType> Cameras;

	switch (Type)
	{
		case EMGReplayTriggerType::Takedown:
		case EMGReplayTriggerType::PoliceTakedown:
			Cameras.Add(EMGReplayCameraType::Dramatic);
			Cameras.Add(EMGReplayCameraType::Cinematic);
			Cameras.Add(EMGReplayCameraType::SlowMotion);
			break;
		case EMGReplayTriggerType::NearMiss:
			Cameras.Add(EMGReplayCameraType::Chase);
			Cameras.Add(EMGReplayCameraType::Cinematic);
			break;
		case EMGReplayTriggerType::DriftCombo:
			Cameras.Add(EMGReplayCameraType::TrackSide);
			Cameras.Add(EMGReplayCameraType::Orbital);
			Cameras.Add(EMGReplayCameraType::Helicopter);
			break;
		case EMGReplayTriggerType::BigAir:
			Cameras.Add(EMGReplayCameraType::Helicopter);
			Cameras.Add(EMGReplayCameraType::Cinematic);
			Cameras.Add(EMGReplayCameraType::SlowMotion);
			break;
		case EMGReplayTriggerType::PhotoFinish:
		case EMGReplayTriggerType::RaceFinish:
			Cameras.Add(EMGReplayCameraType::TrackSide);
			Cameras.Add(EMGReplayCameraType::Dramatic);
			Cameras.Add(EMGReplayCameraType::SlowMotion);
			break;
		case EMGReplayTriggerType::Crash:
			Cameras.Add(EMGReplayCameraType::Dramatic);
			Cameras.Add(EMGReplayCameraType::SlowMotion);
			break;
		case EMGReplayTriggerType::Overtake:
			Cameras.Add(EMGReplayCameraType::Chase);
			Cameras.Add(EMGReplayCameraType::Bumper);
			break;
		case EMGReplayTriggerType::PursuitEscape:
			Cameras.Add(EMGReplayCameraType::Helicopter);
			Cameras.Add(EMGReplayCameraType::Cinematic);
			break;
		default:
			Cameras.Add(EMGReplayCameraType::Cinematic);
			Cameras.Add(EMGReplayCameraType::Chase);
			break;
	}

	return Cameras;
}

FMGReplayCameraShot UMGInstantReplaySubsystem::CreateCameraShot(const FMGReplayMoment& Moment, EMGReplayCameraType Camera) const
{
	FMGReplayCameraShot Shot;
	Shot.ShotId = FString::Printf(TEXT("SHOT_%lld"), FDateTime::Now().GetTicks());
	Shot.CameraType = Camera;
	Shot.Duration = Moment.Duration;
	Shot.TargetActorId = Moment.PlayerId;
	Shot.FieldOfView = 90.0f;
	Shot.PlaybackSpeed = 1.0f;

	// Adjust settings based on camera type
	switch (Camera)
	{
		case EMGReplayCameraType::SlowMotion:
			Shot.PlaybackSpeed = 0.25f;
			Shot.bUseDepthOfField = true;
			Shot.bUseMotionBlur = true;
			Shot.MotionBlurAmount = 0.8f;
			break;
		case EMGReplayCameraType::Dramatic:
			Shot.FieldOfView = 75.0f;
			Shot.bUseDepthOfField = true;
			Shot.Aperture = 2.0f;
			break;
		case EMGReplayCameraType::Cinematic:
			Shot.FieldOfView = 60.0f;
			Shot.bUseDepthOfField = true;
			break;
		case EMGReplayCameraType::Helicopter:
			Shot.FieldOfView = 80.0f;
			Shot.CameraOffset = FVector(0.0f, 0.0f, 2000.0f);
			break;
		default:
			break;
	}

	return Shot;
}

FString UMGInstantReplaySubsystem::GenerateMomentId() const
{
	return FString::Printf(TEXT("MOMENT_%d_%lld"), ++const_cast<UMGInstantReplaySubsystem*>(this)->MomentCounter, FDateTime::Now().GetTicks());
}

FString UMGInstantReplaySubsystem::GenerateReplayId() const
{
	return FString::Printf(TEXT("REPLAY_%d_%lld"), ++const_cast<UMGInstantReplaySubsystem*>(this)->ReplayCounter, FDateTime::Now().GetTicks());
}

FString UMGInstantReplaySubsystem::GenerateShotId() const
{
	return FString::Printf(TEXT("SHOT_%d_%lld"), ++const_cast<UMGInstantReplaySubsystem*>(this)->ShotCounter, FDateTime::Now().GetTicks());
}

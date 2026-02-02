// Copyright Midnight Grind. All Rights Reserved.

// MGReplayBufferSubsystem.cpp
// Midnight Grind - Replay Buffer and Race Recording System

#include "ReplayBuffer/MGReplayBufferSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

void UMGReplayBufferSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize default configuration
    ReplayConfig.MaxBufferDurationSeconds = 300.0f;
    ReplayConfig.TargetFrameRate = 60.0f;
    ReplayConfig.KeyFrameInterval = 30;
    ReplayConfig.bCompressFrames = true;
    ReplayConfig.bRecordAudio = true;
    ReplayConfig.bAutoDetectEvents = true;
    ReplayConfig.InstantReplayDuration = 15.0f;
    ReplayConfig.MaxBufferSizeBytes = 512 * 1024 * 1024;
    ReplayConfig.bCircularBuffer = true;
    ReplayConfig.MinEventImportance = 0.3f;
    ReplayConfig.MaxSavedReplays = 50;

    // Initialize playback state
    PlaybackState.State = EMGReplayState::Idle;
    PlaybackState.CurrentTime = 0.0f;
    PlaybackState.CurrentFrame = 0;
    PlaybackState.PlaybackSpeed = 1.0f;
    PlaybackState.bLooping = false;
    PlaybackState.CameraMode = EMGReplayCameraMode::FollowCar;
    PlaybackState.FocusVehicleId = 0;

    CurrentState = EMGReplayState::Idle;
}

void UMGReplayBufferSubsystem::Deinitialize()
{
    StopRecording();
    StopPlayback();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PlaybackTimerHandle);
        World->GetTimerManager().ClearTimer(RecordingTimerHandle);
        World->GetTimerManager().ClearTimer(ExportTimerHandle);
    }

    Super::Deinitialize();
}

void UMGReplayBufferSubsystem::StartRecording(const FString& RecordingName)
{
    if (CurrentState == EMGReplayState::Recording)
    {
        return;
    }

    CurrentRecording = FMGReplayRecording();
    CurrentRecording.RecordingId = FGuid::NewGuid();
    CurrentRecording.RecordingName = RecordingName.IsEmpty() ?
        FString::Printf(TEXT("Recording_%s"), *FDateTime::Now().ToString()) : RecordingName;
    CurrentRecording.RecordedAt = FDateTime::Now();
    CurrentRecording.FrameRate = ReplayConfig.TargetFrameRate;
    CurrentRecording.Frames.Empty();
    CurrentRecording.Events.Empty();

    RecordingStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    CurrentRecordingFrame = 0;
    CurrentBufferSize = 0;
    CurrentState = EMGReplayState::Recording;

    // Copy participant info
    for (const auto& Pair : ParticipantNames)
    {
        CurrentRecording.ParticipantNames.Add(Pair.Value);
    }
    for (const auto& Pair : ParticipantVehicles)
    {
        CurrentRecording.VehicleIds.Add(Pair.Value);
    }

    OnRecordingStarted.Broadcast();
}

void UMGReplayBufferSubsystem::StopRecording()
{
    if (CurrentState != EMGReplayState::Recording)
    {
        return;
    }

    CurrentState = EMGReplayState::Idle;

    if (CurrentRecording.Frames.Num() > 0)
    {
        CurrentRecording.TotalDuration = CurrentRecording.Frames.Last().Timestamp -
                                          CurrentRecording.Frames[0].Timestamp;
        CurrentRecording.TotalFrames = CurrentRecording.Frames.Num();
        CurrentRecording.FileSizeBytes = CurrentBufferSize;

        // Generate thumbnail
        CurrentRecording.ThumbnailPath = GenerateThumbnail();

        // Analyze for auto-detected events
        if (ReplayConfig.bAutoDetectEvents)
        {
            AnalyzeForEvents();
        }
    }

    OnRecordingStopped.Broadcast(CurrentRecording);
}

void UMGReplayBufferSubsystem::PauseRecording()
{
    if (CurrentState == EMGReplayState::Recording)
    {
        CurrentState = EMGReplayState::Paused;
    }
}

void UMGReplayBufferSubsystem::ResumeRecording()
{
    if (CurrentState == EMGReplayState::Paused)
    {
        CurrentState = EMGReplayState::Recording;
    }
}

void UMGReplayBufferSubsystem::DiscardRecording()
{
    CurrentState = EMGReplayState::Idle;
    CurrentRecording = FMGReplayRecording();
    CurrentBufferSize = 0;
    CurrentRecordingFrame = 0;
}

float UMGReplayBufferSubsystem::GetRecordingDuration() const
{
    if (CurrentRecording.Frames.Num() > 0)
    {
        return CurrentRecording.Frames.Last().Timestamp - CurrentRecording.Frames[0].Timestamp;
    }
    return 0.0f;
}

int32 UMGReplayBufferSubsystem::GetRecordedFrameCount() const
{
    return CurrentRecording.Frames.Num();
}

void UMGReplayBufferSubsystem::RecordFrame(float MGDeltaTime)
{
    if (CurrentState != EMGReplayState::Recording)
    {
        return;
    }

    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    CurrentFrameData.FrameNumber = CurrentRecordingFrame;
    CurrentFrameData.Timestamp = CurrentTime - RecordingStartTime;
    CurrentFrameData.DeltaTime = DeltaTime;
    CurrentFrameData.bKeyFrame = (CurrentRecordingFrame % ReplayConfig.KeyFrameInterval) == 0;

    // Compress if needed
    if (ReplayConfig.bCompressFrames && !CurrentFrameData.bKeyFrame)
    {
        CompressFrame(CurrentFrameData);
    }

    // Calculate frame size
    int64 FrameSize = sizeof(FMGReplayFrame);
    FrameSize += CurrentFrameData.VehicleSnapshots.Num() * sizeof(FMGVehicleSnapshot);
    CurrentFrameData.CompressedDataSize = FrameSize;

    CurrentRecording.Frames.Add(CurrentFrameData);
    CurrentBufferSize += FrameSize;
    CurrentRecordingFrame++;

    // Reset current frame data for next frame
    CurrentFrameData = FMGReplayFrame();
    CurrentFrameData.VehicleSnapshots.Empty();

    // Trim buffer if using circular buffer and over size limit
    if (ReplayConfig.bCircularBuffer)
    {
        TrimBuffer();
    }
}

void UMGReplayBufferSubsystem::RecordVehicleState(int32 VehicleId, const FMGVehicleSnapshot& Snapshot)
{
    if (CurrentState != EMGReplayState::Recording)
    {
        return;
    }

    CurrentFrameData.VehicleSnapshots.Add(VehicleId, Snapshot);
}

void UMGReplayBufferSubsystem::RecordCameraState(const FTransform& Transform, float FOV)
{
    if (CurrentState != EMGReplayState::Recording)
    {
        return;
    }

    CurrentFrameData.CameraTransform = Transform;
    CurrentFrameData.CameraFOV = FOV;
}

void UMGReplayBufferSubsystem::RecordEvent(const FMGReplayEvent& Event)
{
    if (CurrentState != EMGReplayState::Recording)
    {
        return;
    }

    FMGReplayEvent NewEvent = Event;
    if (!NewEvent.EventId.IsValid())
    {
        NewEvent.EventId = FGuid::NewGuid();
    }
    NewEvent.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() - RecordingStartTime : 0.0f;
    NewEvent.FrameNumber = CurrentRecordingFrame;

    CurrentRecording.Events.Add(NewEvent);
    OnEventDetected.Broadcast(NewEvent);
}

void UMGReplayBufferSubsystem::RecordEventSimple(EMGReplayEventType EventType, int32 VehicleId, const FString& Description)
{
    FMGReplayEvent Event;
    Event.EventType = EventType;
    Event.VehicleId = VehicleId;
    Event.Description = Description;

    // Set importance based on event type
    switch (EventType)
    {
    case EMGReplayEventType::RaceStart:
    case EMGReplayEventType::RaceFinish:
    case EMGReplayEventType::PersonalBest:
        Event.ImportanceScore = 1.0f;
        break;
    case EMGReplayEventType::Overtake:
    case EMGReplayEventType::BigAir:
    case EMGReplayEventType::TrickLanded:
        Event.ImportanceScore = 0.8f;
        break;
    case EMGReplayEventType::LapComplete:
    case EMGReplayEventType::DriftEnd:
    case EMGReplayEventType::NitroActivated:
        Event.ImportanceScore = 0.6f;
        break;
    case EMGReplayEventType::Collision:
    case EMGReplayEventType::NearMiss:
        Event.ImportanceScore = 0.5f;
        break;
    default:
        Event.ImportanceScore = 0.4f;
        break;
    }

    RecordEvent(Event);
}

void UMGReplayBufferSubsystem::StartPlayback()
{
    StartPlaybackFromRecording(CurrentRecording);
}

void UMGReplayBufferSubsystem::StartPlaybackFromRecording(const FMGReplayRecording& Recording)
{
    if (Recording.Frames.Num() == 0)
    {
        return;
    }

    PlaybackRecording = Recording;
    PlaybackState.State = EMGReplayState::Playing;
    PlaybackState.CurrentTime = 0.0f;
    PlaybackState.CurrentFrame = 0;
    PlaybackState.TotalDuration = Recording.TotalDuration;
    PlaybackState.TotalFrames = Recording.TotalFrames;
    CurrentState = EMGReplayState::Playing;

    // Start playback timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGReplayBufferSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            PlaybackTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->UpdatePlayback(1.0f / 60.0f);
                }
            },
            1.0f / 60.0f,
            true
        );
    }

    OnPlaybackStarted.Broadcast();
}

void UMGReplayBufferSubsystem::StopPlayback()
{
    if (CurrentState != EMGReplayState::Playing && CurrentState != EMGReplayState::Paused)
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PlaybackTimerHandle);
    }

    CurrentState = EMGReplayState::Idle;
    PlaybackState.State = EMGReplayState::Idle;
    bInInstantReplay = false;

    OnPlaybackStopped.Broadcast();
}

void UMGReplayBufferSubsystem::PausePlayback()
{
    if (CurrentState == EMGReplayState::Playing)
    {
        CurrentState = EMGReplayState::Paused;
        PlaybackState.State = EMGReplayState::Paused;
        OnPlaybackPaused.Broadcast();
    }
}

void UMGReplayBufferSubsystem::ResumePlayback()
{
    if (CurrentState == EMGReplayState::Paused)
    {
        CurrentState = EMGReplayState::Playing;
        PlaybackState.State = EMGReplayState::Playing;
        OnPlaybackResumed.Broadcast();
    }
}

void UMGReplayBufferSubsystem::TogglePause()
{
    if (CurrentState == EMGReplayState::Playing)
    {
        PausePlayback();
    }
    else if (CurrentState == EMGReplayState::Paused)
    {
        ResumePlayback();
    }
}

void UMGReplayBufferSubsystem::SetPlaybackSpeed(float Speed)
{
    PlaybackState.PlaybackSpeed = FMath::Clamp(Speed, 0.1f, 8.0f);
}

void UMGReplayBufferSubsystem::SeekToTime(float Time)
{
    PlaybackState.CurrentTime = FMath::Clamp(Time, 0.0f, PlaybackState.TotalDuration);
    PlaybackState.CurrentFrame = FindFrameAtTime(PlaybackState.CurrentTime);
    PlaybackState.State = EMGReplayState::Scrubbing;
    CurrentState = EMGReplayState::Scrubbing;
}

void UMGReplayBufferSubsystem::SeekToFrame(int32 Frame)
{
    PlaybackState.CurrentFrame = FMath::Clamp(Frame, 0, PlaybackState.TotalFrames - 1);

    if (PlaybackRecording.Frames.IsValidIndex(PlaybackState.CurrentFrame))
    {
        PlaybackState.CurrentTime = PlaybackRecording.Frames[PlaybackState.CurrentFrame].Timestamp;
    }

    PlaybackState.State = EMGReplayState::Scrubbing;
    CurrentState = EMGReplayState::Scrubbing;
}

void UMGReplayBufferSubsystem::SeekToEvent(const FGuid& EventId)
{
    for (const FMGReplayEvent& Event : PlaybackRecording.Events)
    {
        if (Event.EventId == EventId)
        {
            SeekToTime(Event.Timestamp);
            return;
        }
    }
}

void UMGReplayBufferSubsystem::SkipForward(float Seconds)
{
    SeekToTime(PlaybackState.CurrentTime + Seconds);
}

void UMGReplayBufferSubsystem::SkipBackward(float Seconds)
{
    SeekToTime(PlaybackState.CurrentTime - Seconds);
}

void UMGReplayBufferSubsystem::JumpToNextEvent()
{
    for (const FMGReplayEvent& Event : PlaybackRecording.Events)
    {
        if (Event.Timestamp > PlaybackState.CurrentTime + 0.1f)
        {
            SeekToTime(Event.Timestamp);
            return;
        }
    }
}

void UMGReplayBufferSubsystem::JumpToPreviousEvent()
{
    for (int32 i = PlaybackRecording.Events.Num() - 1; i >= 0; --i)
    {
        if (PlaybackRecording.Events[i].Timestamp < PlaybackState.CurrentTime - 0.1f)
        {
            SeekToTime(PlaybackRecording.Events[i].Timestamp);
            return;
        }
    }
}

void UMGReplayBufferSubsystem::SetLooping(bool bLoop)
{
    PlaybackState.bLooping = bLoop;
}

float UMGReplayBufferSubsystem::GetPlaybackProgress() const
{
    if (PlaybackState.TotalDuration <= 0.0f)
    {
        return 0.0f;
    }
    return FMath::Clamp(PlaybackState.CurrentTime / PlaybackState.TotalDuration, 0.0f, 1.0f);
}

void UMGReplayBufferSubsystem::TriggerInstantReplay(float Duration)
{
    if (CurrentRecording.Frames.Num() == 0)
    {
        return;
    }

    float ReplayDuration = Duration > 0.0f ? Duration : ReplayConfig.InstantReplayDuration;

    // Calculate start time for instant replay
    float CurrentRecordingDuration = GetRecordingDuration();
    float StartTime = FMath::Max(0.0f, CurrentRecordingDuration - ReplayDuration);

    // Pause recording during instant replay
    EMGReplayState PreviousState = CurrentState;
    if (CurrentState == EMGReplayState::Recording)
    {
        PauseRecording();
    }

    bInInstantReplay = true;
    InstantReplayStartTime = StartTime;

    // Start playback from start time
    PlaybackRecording = CurrentRecording;
    PlaybackState.State = EMGReplayState::Playing;
    PlaybackState.CurrentTime = StartTime;
    PlaybackState.CurrentFrame = FindFrameAtTime(StartTime);
    PlaybackState.TotalDuration = CurrentRecordingDuration;
    PlaybackState.TotalFrames = CurrentRecording.Frames.Num();
    PlaybackState.PlaybackSpeed = 0.5f; // Slow motion for instant replay
    CurrentState = EMGReplayState::Playing;

    // Start playback timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGReplayBufferSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            PlaybackTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->UpdatePlayback(1.0f / 60.0f);
                }
            },
            1.0f / 60.0f,
            true
        );
    }

    OnPlaybackStarted.Broadcast();
}

void UMGReplayBufferSubsystem::EndInstantReplay()
{
    if (!bInInstantReplay)
    {
        return;
    }

    StopPlayback();
    bInInstantReplay = false;

    // Resume recording if it was active
    ResumeRecording();
}

void UMGReplayBufferSubsystem::SetCameraMode(EMGReplayCameraMode Mode)
{
    PlaybackState.CameraMode = Mode;
}

void UMGReplayBufferSubsystem::SetFocusVehicle(int32 VehicleId)
{
    PlaybackState.FocusVehicleId = VehicleId;
}

void UMGReplayBufferSubsystem::CycleCamera()
{
    int32 CurrentMode = static_cast<int32>(PlaybackState.CameraMode);
    CurrentMode = (CurrentMode + 1) % (static_cast<int32>(EMGReplayCameraMode::Photo) + 1);
    PlaybackState.CameraMode = static_cast<EMGReplayCameraMode>(CurrentMode);
}

void UMGReplayBufferSubsystem::CycleFocusVehicle()
{
    TArray<int32> VehicleIds;
    if (PlaybackRecording.Frames.Num() > 0)
    {
        PlaybackRecording.Frames[0].VehicleSnapshots.GetKeys(VehicleIds);
    }

    if (VehicleIds.Num() == 0)
    {
        return;
    }

    int32 CurrentIndex = VehicleIds.Find(PlaybackState.FocusVehicleId);
    CurrentIndex = (CurrentIndex + 1) % VehicleIds.Num();
    PlaybackState.FocusVehicleId = VehicleIds[CurrentIndex];
}

void UMGReplayBufferSubsystem::SetFreeCamTransform(const FTransform& Transform)
{
    FreeCamTransform = Transform;
}

void UMGReplayBufferSubsystem::AddCameraKeyframe(const FMGCameraKeyframe& Keyframe)
{
    CinematicKeyframes.Add(Keyframe);

    // Sort by time
    CinematicKeyframes.Sort([](const FMGCameraKeyframe& A, const FMGCameraKeyframe& B)
    {
        return A.Time < B.Time;
    });
}

void UMGReplayBufferSubsystem::ClearCameraKeyframes()
{
    CinematicKeyframes.Empty();
    CinematicKeyframeIndex = 0;
    CinematicTime = 0.0f;
}

void UMGReplayBufferSubsystem::PlayCinematicCamera()
{
    if (CinematicKeyframes.Num() < 2)
    {
        return;
    }

    PlaybackState.CameraMode = EMGReplayCameraMode::Cinematic;
    CinematicKeyframeIndex = 0;
    CinematicTime = 0.0f;
}

FTransform UMGReplayBufferSubsystem::GetCurrentCameraTransform() const
{
    return CalculateCameraTransform();
}

FMGReplayClip UMGReplayBufferSubsystem::CreateClip(const FString& ClipName, float StartTime, float EndTime)
{
    FMGReplayClip Clip;
    Clip.ClipId = FGuid::NewGuid();
    Clip.ClipName = ClipName;
    Clip.StartTime = StartTime;
    Clip.EndTime = EndTime;
    Clip.StartFrame = FindFrameAtTime(StartTime);
    Clip.EndFrame = FindFrameAtTime(EndTime);
    Clip.CameraMode = PlaybackState.CameraMode;
    Clip.FocusVehicleId = PlaybackState.FocusVehicleId;
    Clip.CreatedAt = FDateTime::Now();

    // Find events in range
    for (const FMGReplayEvent& Event : CurrentRecording.Events)
    {
        if (Event.Timestamp >= StartTime && Event.Timestamp <= EndTime)
        {
            Clip.ContainedEvents.Add(Event);
        }
    }

    CurrentRecording.SavedClips.Add(Clip);
    OnClipCreated.Broadcast(Clip);

    return Clip;
}

FMGReplayClip UMGReplayBufferSubsystem::CreateClipFromEvent(const FMGReplayEvent& Event, float PaddingBefore, float PaddingAfter)
{
    float StartTime = FMath::Max(0.0f, Event.Timestamp - PaddingBefore);
    float EndTime = FMath::Min(CurrentRecording.TotalDuration, Event.Timestamp + PaddingAfter);

    FString ClipName = FString::Printf(TEXT("%s_%s"),
        *UEnum::GetDisplayValueAsText(Event.EventType).ToString(),
        *FDateTime::Now().ToString(TEXT("%H%M%S")));

    return CreateClip(ClipName, StartTime, EndTime);
}

void UMGReplayBufferSubsystem::DeleteClip(const FGuid& ClipId)
{
    CurrentRecording.SavedClips.RemoveAll([&ClipId](const FMGReplayClip& Clip)
    {
        return Clip.ClipId == ClipId;
    });
}

void UMGReplayBufferSubsystem::PlayClip(const FMGReplayClip& Clip)
{
    PlaybackState.CameraMode = Clip.CameraMode;
    PlaybackState.FocusVehicleId = Clip.FocusVehicleId;
    SeekToTime(Clip.StartTime);
    ResumePlayback();

    // Auto-stop at end of clip
    // (Would need additional logic to handle clip end)
}

void UMGReplayBufferSubsystem::SetClipFavorite(const FGuid& ClipId, bool bFavorite)
{
    for (FMGReplayClip& Clip : CurrentRecording.SavedClips)
    {
        if (Clip.ClipId == ClipId)
        {
            Clip.bIsFavorite = bFavorite;
            return;
        }
    }
}

TArray<FMGReplayClip> UMGReplayBufferSubsystem::GetAllClips() const
{
    return CurrentRecording.SavedClips;
}

TArray<FMGReplayClip> UMGReplayBufferSubsystem::GetFavoriteClips() const
{
    TArray<FMGReplayClip> Favorites;
    for (const FMGReplayClip& Clip : CurrentRecording.SavedClips)
    {
        if (Clip.bIsFavorite)
        {
            Favorites.Add(Clip);
        }
    }
    return Favorites;
}

TArray<FMGReplayEvent> UMGReplayBufferSubsystem::GetAllEvents() const
{
    return CurrentRecording.Events;
}

TArray<FMGReplayEvent> UMGReplayBufferSubsystem::GetEventsByType(EMGReplayEventType EventType) const
{
    TArray<FMGReplayEvent> Filtered;
    for (const FMGReplayEvent& Event : CurrentRecording.Events)
    {
        if (Event.EventType == EventType)
        {
            Filtered.Add(Event);
        }
    }
    return Filtered;
}

TArray<FMGReplayEvent> UMGReplayBufferSubsystem::GetEventsInTimeRange(float StartTime, float EndTime) const
{
    TArray<FMGReplayEvent> InRange;
    for (const FMGReplayEvent& Event : CurrentRecording.Events)
    {
        if (Event.Timestamp >= StartTime && Event.Timestamp <= EndTime)
        {
            InRange.Add(Event);
        }
    }
    return InRange;
}

FMGReplayEvent UMGReplayBufferSubsystem::GetNearestEvent(float Time) const
{
    FMGReplayEvent NearestEvent;
    float NearestDist = TNumericLimits<float>::Max();

    for (const FMGReplayEvent& Event : CurrentRecording.Events)
    {
        float Dist = FMath::Abs(Event.Timestamp - Time);
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            NearestEvent = Event;
        }
    }

    return NearestEvent;
}

void UMGReplayBufferSubsystem::GenerateHighlightReel(float MaxDuration)
{
    // Sort events by importance
    TArray<FMGReplayEvent> SortedEvents = CurrentRecording.Events;
    SortedEvents.Sort([](const FMGReplayEvent& A, const FMGReplayEvent& B)
    {
        return A.ImportanceScore > B.ImportanceScore;
    });

    // Create clips for top events
    float AccumulatedDuration = 0.0f;
    for (const FMGReplayEvent& Event : SortedEvents)
    {
        if (Event.ImportanceScore < ReplayConfig.MinEventImportance)
        {
            continue;
        }

        float ClipDuration = 6.0f; // Default clip length
        if (AccumulatedDuration + ClipDuration > MaxDuration)
        {
            break;
        }

        CreateClipFromEvent(Event, 3.0f, 3.0f);
        AccumulatedDuration += ClipDuration;
    }
}

bool UMGReplayBufferSubsystem::SaveRecording(const FString& SlotName)
{
    if (CurrentRecording.Frames.Num() == 0)
    {
        return false;
    }

    SavedRecordings.Add(SlotName, CurrentRecording);

    // Cleanup old recordings if over limit
    CleanupOldRecordings();

    return true;
}

bool UMGReplayBufferSubsystem::LoadRecording(const FString& SlotName)
{
    if (const FMGReplayRecording* Recording = SavedRecordings.Find(SlotName))
    {
        CurrentRecording = *Recording;
        return true;
    }
    return false;
}

bool UMGReplayBufferSubsystem::DeleteSavedRecording(const FString& SlotName)
{
    return SavedRecordings.Remove(SlotName) > 0;
}

TArray<FString> UMGReplayBufferSubsystem::GetSavedRecordingNames() const
{
    TArray<FString> Names;
    SavedRecordings.GetKeys(Names);
    return Names;
}

FMGReplayRecording UMGReplayBufferSubsystem::GetSavedRecordingInfo(const FString& SlotName) const
{
    if (const FMGReplayRecording* Recording = SavedRecordings.Find(SlotName))
    {
        FMGReplayRecording Info = *Recording;
        Info.Frames.Empty(); // Don't include full frame data in info query
        return Info;
    }
    return FMGReplayRecording();
}

void UMGReplayBufferSubsystem::CleanupOldRecordings()
{
    if (SavedRecordings.Num() <= ReplayConfig.MaxSavedReplays)
    {
        return;
    }

    // Sort by date and remove oldest
    TArray<TPair<FString, FDateTime>> RecordingDates;
    for (const auto& Pair : SavedRecordings)
    {
        RecordingDates.Add(TPair<FString, FDateTime>(Pair.Key, Pair.Value.RecordedAt));
    }

    RecordingDates.Sort([](const auto& A, const auto& B)
    {
        return A.Value < B.Value;
    });

    while (SavedRecordings.Num() > ReplayConfig.MaxSavedReplays && RecordingDates.Num() > 0)
    {
        SavedRecordings.Remove(RecordingDates[0].Key);
        RecordingDates.RemoveAt(0);
    }
}

void UMGReplayBufferSubsystem::ExportReplay(const FMGReplayExportSettings& Settings)
{
    if (CurrentRecording.Frames.Num() == 0)
    {
        OnExportFailed.Broadcast(TEXT("No recording available to export"));
        return;
    }

    CurrentState = EMGReplayState::Exporting;
    ExportProgress = 0.0f;

    // Start export process
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGReplayBufferSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            ExportTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->ProcessExport();
                }
            },
            0.1f,
            true
        );
    }
}

void UMGReplayBufferSubsystem::ExportClip(const FMGReplayClip& Clip, const FMGReplayExportSettings& Settings)
{
    // Set playback range to clip
    SeekToTime(Clip.StartTime);
    // Then export
    ExportReplay(Settings);
}

void UMGReplayBufferSubsystem::CancelExport()
{
    if (CurrentState == EMGReplayState::Exporting)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ExportTimerHandle);
        }
        CurrentState = EMGReplayState::Idle;
        OnExportFailed.Broadcast(TEXT("Export cancelled by user"));
    }
}

void UMGReplayBufferSubsystem::CaptureScreenshot(const FString& FilePath)
{
    if (!GEngine || !GEngine->GameViewport)
    {
        UE_LOG(LogTemp, Warning, TEXT("MGReplayBuffer: Cannot capture screenshot - no game viewport"));
        return;
    }

    FString FullPath = FilePath;
    if (FullPath.IsEmpty())
    {
        // Generate default path with timestamp
        FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Screenshots") / TEXT("Replay");
        IFileManager::Get().MakeDirectory(*SaveDir, true);

        FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
        FullPath = SaveDir / FString::Printf(TEXT("Replay_%s.png"), *Timestamp);
    }

    // Request screenshot from viewport
    FScreenshotRequest::RequestScreenshot(FullPath, false, false);

    UE_LOG(LogTemp, Log, TEXT("MGReplayBuffer: Screenshot requested: %s"), *FullPath);
}

FMGReplayFrame UMGReplayBufferSubsystem::GetFrame(int32 FrameNumber) const
{
    if (PlaybackRecording.Frames.IsValidIndex(FrameNumber))
    {
        return PlaybackRecording.Frames[FrameNumber];
    }
    return FMGReplayFrame();
}

FMGReplayFrame UMGReplayBufferSubsystem::GetFrameAtTime(float Time) const
{
    int32 FrameIndex = FindFrameAtTime(Time);
    return GetFrame(FrameIndex);
}

FMGVehicleSnapshot UMGReplayBufferSubsystem::GetVehicleSnapshotAtTime(int32 VehicleId, float Time) const
{
    FMGReplayFrame Frame = GetFrameAtTime(Time);
    if (const FMGVehicleSnapshot* Snapshot = Frame.VehicleSnapshots.Find(VehicleId))
    {
        return *Snapshot;
    }
    return FMGVehicleSnapshot();
}

FMGVehicleSnapshot UMGReplayBufferSubsystem::InterpolateVehicleSnapshot(int32 VehicleId, float Time) const
{
    int32 FrameIndex = FindFrameAtTime(Time);

    if (!PlaybackRecording.Frames.IsValidIndex(FrameIndex) ||
        !PlaybackRecording.Frames.IsValidIndex(FrameIndex + 1))
    {
        return GetVehicleSnapshotAtTime(VehicleId, Time);
    }

    const FMGReplayFrame& FrameA = PlaybackRecording.Frames[FrameIndex];
    const FMGReplayFrame& FrameB = PlaybackRecording.Frames[FrameIndex + 1];

    const FMGVehicleSnapshot* SnapshotA = FrameA.VehicleSnapshots.Find(VehicleId);
    const FMGVehicleSnapshot* SnapshotB = FrameB.VehicleSnapshots.Find(VehicleId);

    if (!SnapshotA || !SnapshotB)
    {
        return GetVehicleSnapshotAtTime(VehicleId, Time);
    }

    float Alpha = (Time - FrameA.Timestamp) / FMath::Max(0.001f, FrameB.Timestamp - FrameA.Timestamp);
    return LerpSnapshot(*SnapshotA, *SnapshotB, Alpha);
}

void UMGReplayBufferSubsystem::ApplyConfig(const FMGReplayConfig& Config)
{
    ReplayConfig = Config;
}

void UMGReplayBufferSubsystem::SetMaxBufferDuration(float Seconds)
{
    ReplayConfig.MaxBufferDurationSeconds = FMath::Max(10.0f, Seconds);
}

void UMGReplayBufferSubsystem::SetTargetFrameRate(float FPS)
{
    ReplayConfig.TargetFrameRate = FMath::Clamp(FPS, 15.0f, 120.0f);
}

void UMGReplayBufferSubsystem::SetTrackInfo(const FString& TrackId, const FString& TrackName)
{
    CurrentRecording.TrackId = TrackId;
    CurrentRecording.TrackName = TrackName;
}

void UMGReplayBufferSubsystem::SetGameMode(const FString& Mode)
{
    CurrentRecording.GameMode = Mode;
}

void UMGReplayBufferSubsystem::RegisterParticipant(int32 VehicleId, const FString& PlayerName, const FString& VehicleType)
{
    ParticipantNames.Add(VehicleId, PlayerName);
    ParticipantVehicles.Add(VehicleId, VehicleType);
}

void UMGReplayBufferSubsystem::UpdatePlayback(float MGDeltaTime)
{
    if (CurrentState != EMGReplayState::Playing)
    {
        return;
    }

    float AdvanceTime = DeltaTime * PlaybackState.PlaybackSpeed;
    PlaybackState.CurrentTime += AdvanceTime;

    // Check for end of replay
    if (PlaybackState.CurrentTime >= PlaybackState.TotalDuration)
    {
        if (PlaybackState.bLooping)
        {
            PlaybackState.CurrentTime = 0.0f;
            PlaybackState.CurrentFrame = 0;
        }
        else
        {
            if (bInInstantReplay)
            {
                EndInstantReplay();
            }
            else
            {
                StopPlayback();
            }
            return;
        }
    }

    PlaybackState.CurrentFrame = FindFrameAtTime(PlaybackState.CurrentTime);
    OnPlaybackProgress.Broadcast(PlaybackState.CurrentTime, PlaybackState.TotalDuration);
}

void UMGReplayBufferSubsystem::UpdateRecording(float MGDeltaTime)
{
    // Handled by RecordFrame calls from gameplay
}

void UMGReplayBufferSubsystem::ProcessExport()
{
    // Simulate export progress
    ExportProgress += 0.02f;

    OnExportProgress.Broadcast(ExportProgress, TEXT("Rendering frames..."));

    if (ExportProgress >= 1.0f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ExportTimerHandle);
        }
        CurrentState = EMGReplayState::Idle;
        OnExportComplete.Broadcast(TEXT("Export/replay_output.mp4"));
    }
}

void UMGReplayBufferSubsystem::TrimBuffer()
{
    // Remove oldest frames if over duration limit
    float MaxDuration = ReplayConfig.MaxBufferDurationSeconds;

    while (CurrentRecording.Frames.Num() > 1)
    {
        float Duration = CurrentRecording.Frames.Last().Timestamp -
                         CurrentRecording.Frames[0].Timestamp;

        if (Duration <= MaxDuration && CurrentBufferSize <= ReplayConfig.MaxBufferSizeBytes)
        {
            break;
        }

        CurrentBufferSize -= CurrentRecording.Frames[0].CompressedDataSize;
        CurrentRecording.Frames.RemoveAt(0);
    }

    // Also trim old events
    if (CurrentRecording.Frames.Num() > 0)
    {
        float OldestTime = CurrentRecording.Frames[0].Timestamp;
        CurrentRecording.Events.RemoveAll([OldestTime](const FMGReplayEvent& Event)
        {
            return Event.Timestamp < OldestTime;
        });
    }
}

void UMGReplayBufferSubsystem::AnalyzeForEvents()
{
    if (CurrentRecording.Frames.Num() < 2)
    {
        return;
    }

    // Track vehicle states for event detection
    TMap<int32, float> PreviousSpeed;
    TMap<int32, bool> WasDrifting;
    TMap<int32, bool> WasAirborne;
    TMap<int32, int32> PreviousPosition; // For overtake detection
    TMap<int32, FVector> PreviousLocation;

    // Thresholds for event detection
    const float BigAirMinTime = 0.5f; // Minimum airtime for big air event
    const float DriftMinAngle = 15.0f; // Minimum angle to start drift
    const float OvertakeMinSpeedDiff = 5.0f; // MPH difference for overtake
    const float NearMissDistance = 200.0f; // cm for near miss detection

    TMap<int32, float> AirborneStartTime;
    TMap<int32, float> DriftStartTime;

    for (int32 FrameIdx = 1; FrameIdx < CurrentRecording.Frames.Num(); ++FrameIdx)
    {
        const FMGReplayFrame& PrevFrame = CurrentRecording.Frames[FrameIdx - 1];
        const FMGReplayFrame& CurrentFrame = CurrentRecording.Frames[FrameIdx];

        // Sort vehicles by distance traveled to get positions
        TArray<TPair<int32, float>> VehicleDistances;
        for (const auto& Pair : CurrentFrame.VehicleSnapshots)
        {
            // Use distance from origin as proxy for race progress
            float Distance = Pair.Value.Transform.GetLocation().Size();
            VehicleDistances.Add(TPair<int32, float>(Pair.Key, Distance));
        }
        VehicleDistances.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
        {
            return A.Value > B.Value; // Higher distance = further ahead
        });

        // Assign positions
        TMap<int32, int32> CurrentPositions;
        for (int32 i = 0; i < VehicleDistances.Num(); ++i)
        {
            CurrentPositions.Add(VehicleDistances[i].Key, i + 1);
        }

        // Analyze each vehicle
        for (const auto& Pair : CurrentFrame.VehicleSnapshots)
        {
            int32 VehicleId = Pair.Key;
            const FMGVehicleSnapshot& Snapshot = Pair.Value;
            const FMGVehicleSnapshot* PrevSnapshot = PrevFrame.VehicleSnapshots.Find(VehicleId);

            if (!PrevSnapshot)
            {
                continue;
            }

            // Detect drift start/end
            bool bCurrentlyDrifting = Snapshot.bDrifting || FMath::Abs(Snapshot.DriftAngle) >= DriftMinAngle;
            bool* bWasPrevDrifting = WasDrifting.Find(VehicleId);

            if (bCurrentlyDrifting && (!bWasPrevDrifting || !(*bWasPrevDrifting)))
            {
                // Drift started
                FMGReplayEvent Event;
                Event.EventId = FGuid::NewGuid();
                Event.EventType = EMGReplayEventType::DriftStart;
                Event.Timestamp = CurrentFrame.Timestamp;
                Event.FrameNumber = CurrentFrame.FrameNumber;
                Event.VehicleId = VehicleId;
                Event.WorldLocation = Snapshot.Transform.GetLocation();
                CurrentRecording.Events.Add(Event);

                DriftStartTime.Add(VehicleId, CurrentFrame.Timestamp);
            }
            else if (!bCurrentlyDrifting && bWasPrevDrifting && *bWasPrevDrifting)
            {
                // Drift ended
                FMGReplayEvent Event;
                Event.EventId = FGuid::NewGuid();
                Event.EventType = EMGReplayEventType::DriftEnd;
                Event.Timestamp = CurrentFrame.Timestamp;
                Event.FrameNumber = CurrentFrame.FrameNumber;
                Event.VehicleId = VehicleId;
                Event.WorldLocation = Snapshot.Transform.GetLocation();
                CurrentRecording.Events.Add(Event);
            }
            WasDrifting.Add(VehicleId, bCurrentlyDrifting);

            // Detect big air
            bool bCurrentlyAirborne = Snapshot.bAirborne;
            bool* bWasPrevAirborne = WasAirborne.Find(VehicleId);

            if (bCurrentlyAirborne && (!bWasPrevAirborne || !(*bWasPrevAirborne)))
            {
                // Started airborne
                AirborneStartTime.Add(VehicleId, CurrentFrame.Timestamp);
            }
            else if (!bCurrentlyAirborne && bWasPrevAirborne && *bWasPrevAirborne)
            {
                // Landed - check if it was big air
                float* StartTime = AirborneStartTime.Find(VehicleId);
                if (StartTime && (CurrentFrame.Timestamp - *StartTime) >= BigAirMinTime)
                {
                    FMGReplayEvent Event;
                    Event.EventId = FGuid::NewGuid();
                    Event.EventType = EMGReplayEventType::BigAir;
                    Event.Timestamp = *StartTime;
                    Event.FrameNumber = CurrentFrame.FrameNumber;
                    Event.VehicleId = VehicleId;
                    Event.WorldLocation = Snapshot.Transform.GetLocation();
                    CurrentRecording.Events.Add(Event);
                }
            }
            WasAirborne.Add(VehicleId, bCurrentlyAirborne);

            // Detect overtakes
            int32* PrevPos = PreviousPosition.Find(VehicleId);
            int32* CurrPos = CurrentPositions.Find(VehicleId);
            if (PrevPos && CurrPos && *CurrPos < *PrevPos)
            {
                // Position improved (lower number = better position)
                FMGReplayEvent Event;
                Event.EventId = FGuid::NewGuid();
                Event.EventType = EMGReplayEventType::Overtake;
                Event.Timestamp = CurrentFrame.Timestamp;
                Event.FrameNumber = CurrentFrame.FrameNumber;
                Event.VehicleId = VehicleId;
                Event.WorldLocation = Snapshot.Transform.GetLocation();
                CurrentRecording.Events.Add(Event);
            }

            // Detect nitro activation
            if (Snapshot.bNitroActive && !PrevSnapshot->bNitroActive)
            {
                FMGReplayEvent Event;
                Event.EventId = FGuid::NewGuid();
                Event.EventType = EMGReplayEventType::NitroActivated;
                Event.Timestamp = CurrentFrame.Timestamp;
                Event.FrameNumber = CurrentFrame.FrameNumber;
                Event.VehicleId = VehicleId;
                Event.WorldLocation = Snapshot.Transform.GetLocation();
                CurrentRecording.Events.Add(Event);
            }

            // Update previous position tracking
            if (CurrPos)
            {
                PreviousPosition.Add(VehicleId, *CurrPos);
            }
            PreviousLocation.Add(VehicleId, Snapshot.Transform.GetLocation());
        }

        // Detect near misses between vehicles
        TArray<int32> VehicleIds;
        CurrentFrame.VehicleSnapshots.GetKeys(VehicleIds);
        for (int32 i = 0; i < VehicleIds.Num(); ++i)
        {
            for (int32 j = i + 1; j < VehicleIds.Num(); ++j)
            {
                const FMGVehicleSnapshot* SnapA = CurrentFrame.VehicleSnapshots.Find(VehicleIds[i]);
                const FMGVehicleSnapshot* SnapB = CurrentFrame.VehicleSnapshots.Find(VehicleIds[j]);

                if (SnapA && SnapB)
                {
                    float Distance = FVector::Dist(
                        SnapA->Transform.GetLocation(),
                        SnapB->Transform.GetLocation()
                    );

                    // Check if close and both moving fast
                    if (Distance < NearMissDistance &&
                        SnapA->Speed > 50.0f && SnapB->Speed > 50.0f)
                    {
                        // Check they didn't collide (velocities still largely intact)
                        FMGReplayEvent Event;
                        Event.EventId = FGuid::NewGuid();
                        Event.EventType = EMGReplayEventType::NearMiss;
                        Event.Timestamp = CurrentFrame.Timestamp;
                        Event.FrameNumber = CurrentFrame.FrameNumber;
                        Event.VehicleId = VehicleIds[i]; // Primary vehicle
                        Event.WorldLocation = SnapA->Transform.GetLocation();
                        CurrentRecording.Events.Add(Event);
                    }
                }
            }
        }
    }
}

FMGVehicleSnapshot UMGReplayBufferSubsystem::LerpSnapshot(const FMGVehicleSnapshot& A, const FMGVehicleSnapshot& B, float Alpha) const
{
    FMGVehicleSnapshot Result;

    Result.Transform.SetLocation(FMath::Lerp(A.Transform.GetLocation(), B.Transform.GetLocation(), Alpha));
    Result.Transform.SetRotation(FQuat::Slerp(A.Transform.GetRotation(), B.Transform.GetRotation(), Alpha));
    Result.Transform.SetScale3D(FMath::Lerp(A.Transform.GetScale3D(), B.Transform.GetScale3D(), Alpha));

    Result.Velocity = FMath::Lerp(A.Velocity, B.Velocity, Alpha);
    Result.AngularVelocity = FMath::Lerp(A.AngularVelocity, B.AngularVelocity, Alpha);
    Result.Speed = FMath::Lerp(A.Speed, B.Speed, Alpha);
    Result.Throttle = FMath::Lerp(A.Throttle, B.Throttle, Alpha);
    Result.Brake = FMath::Lerp(A.Brake, B.Brake, Alpha);
    Result.Steering = FMath::Lerp(A.Steering, B.Steering, Alpha);
    Result.CurrentGear = Alpha < 0.5f ? A.CurrentGear : B.CurrentGear;
    Result.RPM = FMath::Lerp(A.RPM, B.RPM, Alpha);
    Result.bDrifting = Alpha < 0.5f ? A.bDrifting : B.bDrifting;
    Result.DriftAngle = FMath::Lerp(A.DriftAngle, B.DriftAngle, Alpha);
    Result.bNitroActive = Alpha < 0.5f ? A.bNitroActive : B.bNitroActive;
    Result.NitroAmount = FMath::Lerp(A.NitroAmount, B.NitroAmount, Alpha);
    Result.bAirborne = Alpha < 0.5f ? A.bAirborne : B.bAirborne;

    // Lerp wheel rotations
    int32 WheelCount = FMath::Min(A.WheelRotations.Num(), B.WheelRotations.Num());
    for (int32 i = 0; i < WheelCount; ++i)
    {
        Result.WheelRotations.Add(FMath::Lerp(A.WheelRotations[i], B.WheelRotations[i], Alpha));
    }

    // Lerp suspension
    int32 SuspCount = FMath::Min(A.SuspensionCompressions.Num(), B.SuspensionCompressions.Num());
    for (int32 i = 0; i < SuspCount; ++i)
    {
        Result.SuspensionCompressions.Add(FMath::Lerp(A.SuspensionCompressions[i], B.SuspensionCompressions[i], Alpha));
    }

    return Result;
}

FTransform UMGReplayBufferSubsystem::CalculateCameraTransform() const
{
    if (PlaybackState.CameraMode == EMGReplayCameraMode::FreeCam)
    {
        return FreeCamTransform;
    }

    if (PlaybackState.CameraMode == EMGReplayCameraMode::Cinematic && CinematicKeyframes.Num() >= 2)
    {
        // Interpolate between keyframes
        for (int32 i = 0; i < CinematicKeyframes.Num() - 1; ++i)
        {
            const FMGCameraKeyframe& A = CinematicKeyframes[i];
            const FMGCameraKeyframe& B = CinematicKeyframes[i + 1];

            if (PlaybackState.CurrentTime >= A.Time && PlaybackState.CurrentTime <= B.Time)
            {
                float Alpha = (PlaybackState.CurrentTime - A.Time) / FMath::Max(0.001f, B.Time - A.Time);

                FTransform Result;
                Result.SetLocation(FMath::Lerp(A.Transform.GetLocation(), B.Transform.GetLocation(), Alpha));
                Result.SetRotation(FQuat::Slerp(A.Transform.GetRotation(), B.Transform.GetRotation(), Alpha));
                return Result;
            }
        }
    }

    // Get focus vehicle transform for other camera modes
    FMGVehicleSnapshot VehicleSnapshot = InterpolateVehicleSnapshot(PlaybackState.FocusVehicleId, PlaybackState.CurrentTime);
    FTransform VehicleTransform = VehicleSnapshot.Transform;

    FTransform CameraTransform;

    switch (PlaybackState.CameraMode)
    {
    case EMGReplayCameraMode::FollowCar:
        {
            FVector Offset = VehicleTransform.GetRotation().RotateVector(FVector(-500.0f, 0.0f, 200.0f));
            CameraTransform.SetLocation(VehicleTransform.GetLocation() + Offset);
            CameraTransform.SetRotation(FRotationMatrix::MakeFromX(VehicleTransform.GetLocation() - CameraTransform.GetLocation()).ToQuat());
        }
        break;

    case EMGReplayCameraMode::Bumper:
        {
            FVector Offset = VehicleTransform.GetRotation().RotateVector(FVector(50.0f, 0.0f, 50.0f));
            CameraTransform.SetLocation(VehicleTransform.GetLocation() + Offset);
            CameraTransform.SetRotation(VehicleTransform.GetRotation());
        }
        break;

    case EMGReplayCameraMode::Hood:
        {
            FVector Offset = VehicleTransform.GetRotation().RotateVector(FVector(100.0f, 0.0f, 80.0f));
            CameraTransform.SetLocation(VehicleTransform.GetLocation() + Offset);
            CameraTransform.SetRotation(VehicleTransform.GetRotation());
        }
        break;

    case EMGReplayCameraMode::Helicopter:
        {
            FVector Offset = FVector(0.0f, 0.0f, 1000.0f) + VehicleTransform.GetRotation().RotateVector(FVector(-300.0f, 0.0f, 0.0f));
            CameraTransform.SetLocation(VehicleTransform.GetLocation() + Offset);
            CameraTransform.SetRotation(FRotationMatrix::MakeFromX(VehicleTransform.GetLocation() - CameraTransform.GetLocation()).ToQuat());
        }
        break;

    case EMGReplayCameraMode::Orbit:
        {
            float OrbitAngle = PlaybackState.CurrentTime * 0.5f;
            FVector Offset = FVector(FMath::Cos(OrbitAngle) * 600.0f, FMath::Sin(OrbitAngle) * 600.0f, 200.0f);
            CameraTransform.SetLocation(VehicleTransform.GetLocation() + Offset);
            CameraTransform.SetRotation(FRotationMatrix::MakeFromX(VehicleTransform.GetLocation() - CameraTransform.GetLocation()).ToQuat());
        }
        break;

    default:
        CameraTransform = VehicleTransform;
        break;
    }

    return CameraTransform;
}

int32 UMGReplayBufferSubsystem::FindFrameAtTime(float Time) const
{
    if (PlaybackRecording.Frames.Num() == 0)
    {
        return 0;
    }

    // Binary search for frame
    int32 Low = 0;
    int32 High = PlaybackRecording.Frames.Num() - 1;

    while (Low < High)
    {
        int32 Mid = (Low + High) / 2;

        if (PlaybackRecording.Frames[Mid].Timestamp < Time)
        {
            Low = Mid + 1;
        }
        else
        {
            High = Mid;
        }
    }

    return Low;
}

void UMGReplayBufferSubsystem::CompressFrame(FMGReplayFrame& Frame)
{
    // Delta compression would be implemented here
    // For now, we just mark it as a non-keyframe
    Frame.bKeyFrame = false;
}

void UMGReplayBufferSubsystem::DecompressFrame(FMGReplayFrame& Frame)
{
    // Delta decompression would be implemented here
}

FString UMGReplayBufferSubsystem::GenerateThumbnail()
{
    // Generate thumbnail from middle of recording
    // Return path to saved thumbnail
    return FString::Printf(TEXT("Thumbnails/%s.png"), *CurrentRecording.RecordingId.ToString());
}

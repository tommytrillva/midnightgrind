// Copyright Midnight Grind. All Rights Reserved.

#include "Replay/MGReplaySubsystem.h"
#include "Replay/MGGhostRacerActor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "HAL/PlatformFilemanager.h"

// ==========================================
// FMGReplayData
// ==========================================

FMGReplayFrame FMGReplayData::GetFrameAtTime(float Time) const
{
	if (Frames.Num() == 0)
	{
		return FMGReplayFrame();
	}

	if (Time <= 0.0f)
	{
		return Frames[0];
	}

	if (Time >= GetDuration())
	{
		return Frames.Last();
	}

	// Find surrounding frames
	int32 LowerIndex = GetFrameIndexAtTime(Time);
	int32 UpperIndex = FMath::Min(LowerIndex + 1, Frames.Num() - 1);

	const FMGReplayFrame& Lower = Frames[LowerIndex];
	const FMGReplayFrame& Upper = Frames[UpperIndex];

	// Interpolate
	float FrameDuration = Upper.Timestamp - Lower.Timestamp;
	float Alpha = (FrameDuration > 0.0f) ? (Time - Lower.Timestamp) / FrameDuration : 0.0f;
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	FMGReplayFrame Result;
	Result.Timestamp = Time;
	Result.Position = FMath::Lerp(Lower.Position, Upper.Position, Alpha);
	Result.Rotation = FMath::Lerp(Lower.Rotation, Upper.Rotation, Alpha);
	Result.Velocity = FMath::Lerp(Lower.Velocity, Upper.Velocity, Alpha);
	Result.SpeedKPH = FMath::Lerp(Lower.SpeedKPH, Upper.SpeedKPH, Alpha);
	Result.EngineRPM = FMath::Lerp(Lower.EngineRPM, Upper.EngineRPM, Alpha);
	Result.Gear = Alpha < 0.5f ? Lower.Gear : Upper.Gear;
	Result.ThrottleInput = FMath::Lerp(Lower.ThrottleInput, Upper.ThrottleInput, Alpha);
	Result.BrakeInput = FMath::Lerp(Lower.BrakeInput, Upper.BrakeInput, Alpha);
	Result.SteeringInput = FMath::Lerp(Lower.SteeringInput, Upper.SteeringInput, Alpha);
	Result.bIsDrifting = Alpha < 0.5f ? Lower.bIsDrifting : Upper.bIsDrifting;
	Result.bNOSActive = Alpha < 0.5f ? Lower.bNOSActive : Upper.bNOSActive;

	// Interpolate wheel positions
	if (Lower.WheelPositions.Num() == Upper.WheelPositions.Num())
	{
		Result.WheelPositions.SetNum(Lower.WheelPositions.Num());
		for (int32 i = 0; i < Lower.WheelPositions.Num(); ++i)
		{
			Result.WheelPositions[i] = FMath::Lerp(Lower.WheelPositions[i], Upper.WheelPositions[i], Alpha);
		}
	}

	return Result;
}

int32 FMGReplayData::GetFrameIndexAtTime(float Time) const
{
	if (Frames.Num() == 0)
	{
		return 0;
	}

	// Binary search for frame
	int32 Low = 0;
	int32 High = Frames.Num() - 1;

	while (Low < High)
	{
		int32 Mid = (Low + High + 1) / 2;
		if (Frames[Mid].Timestamp <= Time)
		{
			Low = Mid;
		}
		else
		{
			High = Mid - 1;
		}
	}

	return Low;
}

// ==========================================
// UMGReplaySubsystem
// ==========================================

void UMGReplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RecordingInterval = 1.0f / RecordingFPS;
}

void UMGReplaySubsystem::Deinitialize()
{
	// Cancel any active recording
	if (CurrentState == EMGReplayState::Recording)
	{
		CancelRecording();
	}

	// Stop any playback
	if (CurrentState == EMGReplayState::Playing || CurrentState == EMGReplayState::Paused)
	{
		StopPlayback();
	}

	// Despawn all ghosts
	DespawnAllGhosts();

	Super::Deinitialize();
}

void UMGReplaySubsystem::Tick(float MGDeltaTime)
{
	switch (CurrentState)
	{
		case EMGReplayState::Recording:
			UpdateRecording(DeltaTime);
			break;

		case EMGReplayState::Playing:
			UpdatePlayback(DeltaTime);
			break;

		case EMGReplayState::Paused:
		case EMGReplayState::Idle:
			break;
	}

	// Always update ghosts
	UpdateGhosts(DeltaTime);
}

// ==========================================
// RECORDING
// ==========================================

void UMGReplaySubsystem::StartRecording(AActor* TargetActor, FName TrackID, FName VehicleID)
{
	if (CurrentState != EMGReplayState::Idle)
	{
		return;
	}

	if (!TargetActor)
	{
		return;
	}

	// Initialize recording data
	CurrentRecording = FMGReplayData();
	CurrentRecording.ReplayID = FGuid::NewGuid();
	CurrentRecording.TrackID = TrackID;
	CurrentRecording.VehicleID = VehicleID;
	CurrentRecording.RecordedDate = FDateTime::Now();
	CurrentRecording.RecordingFPS = RecordingFPS;
	CurrentRecording.Frames.Empty();
	CurrentRecording.Frames.Reserve(static_cast<int32>(MaxRecordingDuration * RecordingFPS));

	RecordingTarget = TargetActor;
	RecordingAccumulator = 0.0f;

	SetState(EMGReplayState::Recording);
}

FMGReplayData UMGReplaySubsystem::StopRecording()
{
	if (CurrentState != EMGReplayState::Recording)
	{
		return FMGReplayData();
	}

	// Finalize recording
	if (CurrentRecording.Frames.Num() > 0)
	{
		CurrentRecording.TotalTime = CurrentRecording.Frames.Last().Timestamp;
	}

	FMGReplayData Result = CurrentRecording;

	// Clear state
	CurrentRecording = FMGReplayData();
	RecordingTarget = nullptr;

	SetState(EMGReplayState::Idle);

	OnRecordingComplete.Broadcast(Result);

	return Result;
}

void UMGReplaySubsystem::CancelRecording()
{
	if (CurrentState != EMGReplayState::Recording)
	{
		return;
	}

	CurrentRecording = FMGReplayData();
	RecordingTarget = nullptr;

	SetState(EMGReplayState::Idle);
}

void UMGReplaySubsystem::RecordFrame(const FMGReplayFrame& Frame)
{
	if (CurrentState != EMGReplayState::Recording)
	{
		return;
	}

	CurrentRecording.Frames.Add(Frame);
}

float UMGReplaySubsystem::GetRecordingDuration() const
{
	if (CurrentRecording.Frames.Num() > 0)
	{
		return CurrentRecording.Frames.Last().Timestamp;
	}
	return 0.0f;
}

// ==========================================
// PLAYBACK
// ==========================================

void UMGReplaySubsystem::StartPlayback(const FMGReplayData& ReplayData)
{
	if (CurrentState == EMGReplayState::Recording)
	{
		return;
	}

	if (!ReplayData.IsValid())
	{
		return;
	}

	CurrentPlaybackData = ReplayData;
	PlaybackTime = 0.0f;
	PlaybackSpeed = 1.0f;
	CurrentPlaybackMode = EMGReplayPlaybackMode::Normal;

	SetState(EMGReplayState::Playing);
}

void UMGReplaySubsystem::StopPlayback()
{
	if (CurrentState != EMGReplayState::Playing && CurrentState != EMGReplayState::Paused)
	{
		return;
	}

	OnPlaybackComplete.Broadcast(CurrentPlaybackData);

	CurrentPlaybackData = FMGReplayData();
	PlaybackTime = 0.0f;

	SetState(EMGReplayState::Idle);
}

void UMGReplaySubsystem::PausePlayback()
{
	if (CurrentState == EMGReplayState::Playing)
	{
		SetState(EMGReplayState::Paused);
	}
}

void UMGReplaySubsystem::ResumePlayback()
{
	if (CurrentState == EMGReplayState::Paused)
	{
		SetState(EMGReplayState::Playing);
	}
}

void UMGReplaySubsystem::SetPlaybackSpeed(float Speed)
{
	PlaybackSpeed = FMath::Clamp(Speed, 0.1f, 4.0f);
}

void UMGReplaySubsystem::SetPlaybackMode(EMGReplayPlaybackMode Mode)
{
	CurrentPlaybackMode = Mode;

	// Adjust speed based on mode
	switch (Mode)
	{
		case EMGReplayPlaybackMode::Normal:
			PlaybackSpeed = 1.0f;
			break;
		case EMGReplayPlaybackMode::SlowMotion:
			PlaybackSpeed = 0.25f;
			break;
		case EMGReplayPlaybackMode::FastForward:
			PlaybackSpeed = 2.0f;
			break;
		case EMGReplayPlaybackMode::FrameByFrame:
			// Speed is ignored in frame-by-frame mode
			break;
	}
}

void UMGReplaySubsystem::SeekToTime(float Time)
{
	if (!CurrentPlaybackData.IsValid())
	{
		return;
	}

	PlaybackTime = FMath::Clamp(Time, 0.0f, CurrentPlaybackData.GetDuration());
}

void UMGReplaySubsystem::SeekByFrames(int32 FrameDelta)
{
	if (!CurrentPlaybackData.IsValid() || CurrentPlaybackData.Frames.Num() == 0)
	{
		return;
	}

	int32 CurrentFrame = CurrentPlaybackData.GetFrameIndexAtTime(PlaybackTime);
	int32 NewFrame = FMath::Clamp(CurrentFrame + FrameDelta, 0, CurrentPlaybackData.Frames.Num() - 1);

	PlaybackTime = CurrentPlaybackData.Frames[NewFrame].Timestamp;
}

float UMGReplaySubsystem::GetPlaybackProgress() const
{
	if (!CurrentPlaybackData.IsValid())
	{
		return 0.0f;
	}

	float Duration = CurrentPlaybackData.GetDuration();
	return Duration > 0.0f ? PlaybackTime / Duration : 0.0f;
}

FMGReplayFrame UMGReplaySubsystem::GetCurrentPlaybackFrame() const
{
	if (!CurrentPlaybackData.IsValid())
	{
		return FMGReplayFrame();
	}

	return CurrentPlaybackData.GetFrameAtTime(PlaybackTime);
}

// ==========================================
// GHOST RACERS
// ==========================================

AMGGhostRacerActor* UMGReplaySubsystem::SpawnGhost(const FMGGhostConfig& Config)
{
	if (!Config.ReplayData.IsValid())
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Use default class if none specified
	TSubclassOf<AMGGhostRacerActor> ClassToSpawn = GhostActorClass;
	if (!ClassToSpawn)
	{
		ClassToSpawn = AMGGhostRacerActor::StaticClass();
	}

	// Spawn at first frame position
	FMGReplayFrame FirstFrame = Config.ReplayData.Frames[0];
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMGGhostRacerActor* Ghost = World->SpawnActor<AMGGhostRacerActor>(
		ClassToSpawn,
		FirstFrame.Position,
		FirstFrame.Rotation,
		SpawnParams
	);

	if (Ghost)
	{
		Ghost->InitializeGhost(Config);
		ActiveGhosts.Add(Ghost);
		OnGhostSpawned.Broadcast(Ghost, Config.ReplayData);
	}

	return Ghost;
}

AMGGhostRacerActor* UMGReplaySubsystem::SpawnPersonalBestGhost(FName TrackID)
{
	FMGReplayData PBReplay = GetPersonalBestReplay(TrackID);
	if (!PBReplay.IsValid())
	{
		return nullptr;
	}

	FMGGhostConfig Config;
	Config.ReplayData = PBReplay;
	Config.Transparency = DefaultGhostTransparency;
	Config.GhostColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green for PB
	Config.bShowDelta = true;

	return SpawnGhost(Config);
}

AMGGhostRacerActor* UMGReplaySubsystem::SpawnWorldRecordGhost(FName TrackID)
{
	// World record would be fetched from server
	// For now, return nullptr (would require async server call)
	return nullptr;
}

void UMGReplaySubsystem::DespawnAllGhosts()
{
	for (AMGGhostRacerActor* Ghost : ActiveGhosts)
	{
		if (Ghost && IsValid(Ghost))
		{
			Ghost->Destroy();
		}
	}
	ActiveGhosts.Empty();
}

void UMGReplaySubsystem::DespawnGhost(AMGGhostRacerActor* Ghost)
{
	if (!Ghost)
	{
		return;
	}

	ActiveGhosts.Remove(Ghost);
	Ghost->Destroy();
}

float UMGReplaySubsystem::GetDeltaToGhost(AMGGhostRacerActor* Ghost, float PlayerDistance) const
{
	if (!Ghost)
	{
		return 0.0f;
	}

	return Ghost->GetDeltaAtDistance(PlayerDistance);
}

// ==========================================
// STORAGE
// ==========================================

bool UMGReplaySubsystem::SaveReplay(const FMGReplayData& ReplayData, bool bUploadToServer)
{
	if (!ReplayData.IsValid())
	{
		return false;
	}

	// Compress and save locally
	TArray<uint8> CompressedData = CompressReplayData(ReplayData);
	FString FilePath = GetReplayFilePath(ReplayData.ReplayID);

	bool bSaved = FFileHelper::SaveArrayToFile(CompressedData, *FilePath);

	if (bSaved && bUploadToServer)
	{
		// Server upload would be handled asynchronously
		// Online profile system would handle this
	}

	return bSaved;
}

FMGReplayData UMGReplaySubsystem::LoadReplay(FGuid ReplayID)
{
	FString FilePath = GetReplayFilePath(ReplayID);

	TArray<uint8> CompressedData;
	if (FFileHelper::LoadFileToArray(CompressedData, *FilePath))
	{
		return DecompressReplayData(CompressedData);
	}

	return FMGReplayData();
}

bool UMGReplaySubsystem::DeleteReplay(FGuid ReplayID)
{
	FString FilePath = GetReplayFilePath(ReplayID);
	return IFileManager::Get().Delete(*FilePath);
}

TArray<FMGReplayData> UMGReplaySubsystem::GetSavedReplays(FName TrackID)
{
	TArray<FMGReplayData> Result;

	// Search replay directory for files
	FString ReplayDir = FPaths::ProjectSavedDir() / TEXT("Replays");
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *(ReplayDir / TEXT("*.mgrep")), true, false);

	for (const FString& File : Files)
	{
		FGuid ReplayID;
		if (FGuid::Parse(FPaths::GetBaseFilename(File), ReplayID))
		{
			FMGReplayData Replay = LoadReplay(ReplayID);
			if (Replay.IsValid() && Replay.TrackID == TrackID)
			{
				Result.Add(Replay);
			}
		}
	}

	// Sort by time (fastest first)
	Result.Sort([](const FMGReplayData& A, const FMGReplayData& B)
	{
		return A.BestLapTime < B.BestLapTime;
	});

	return Result;
}

FMGReplayData UMGReplaySubsystem::GetPersonalBestReplay(FName TrackID)
{
	TArray<FMGReplayData> Replays = GetSavedReplays(TrackID);

	if (Replays.Num() > 0)
	{
		return Replays[0]; // Already sorted by best lap time
	}

	return FMGReplayData();
}

// ==========================================
// INTERNAL
// ==========================================

void UMGReplaySubsystem::UpdateRecording(float MGDeltaTime)
{
	if (!RecordingTarget.IsValid())
	{
		CancelRecording();
		return;
	}

	// Check max duration
	if (GetRecordingDuration() >= MaxRecordingDuration)
	{
		StopRecording();
		return;
	}

	RecordingAccumulator += DeltaTime;

	// Record frames at fixed interval
	while (RecordingAccumulator >= RecordingInterval)
	{
		RecordingAccumulator -= RecordingInterval;

		// Build frame from target actor
		FMGReplayFrame Frame;
		Frame.Timestamp = CurrentRecording.Frames.Num() * RecordingInterval;
		Frame.Position = RecordingTarget->GetActorLocation();
		Frame.Rotation = RecordingTarget->GetActorRotation();
		Frame.Velocity = RecordingTarget->GetVelocity();

		// Additional data would be pulled from vehicle component
		// For now just position/rotation

		CurrentRecording.Frames.Add(Frame);
	}
}

void UMGReplaySubsystem::UpdatePlayback(float MGDeltaTime)
{
	if (!CurrentPlaybackData.IsValid())
	{
		StopPlayback();
		return;
	}

	// Frame-by-frame doesn't auto-advance
	if (CurrentPlaybackMode == EMGReplayPlaybackMode::FrameByFrame)
	{
		return;
	}

	// Advance playback time
	PlaybackTime += DeltaTime * PlaybackSpeed;

	// Check for end of playback
	if (PlaybackTime >= CurrentPlaybackData.GetDuration())
	{
		PlaybackTime = CurrentPlaybackData.GetDuration();
		StopPlayback();
		return;
	}

	OnPlaybackProgress.Broadcast(PlaybackTime, CurrentPlaybackData.GetDuration());
}

void UMGReplaySubsystem::UpdateGhosts(float MGDeltaTime)
{
	// Remove invalid ghosts
	ActiveGhosts.RemoveAll([](AMGGhostRacerActor* Ghost)
	{
		return !Ghost || !IsValid(Ghost);
	});

	// Ghosts update themselves in their own tick
}

void UMGReplaySubsystem::SetState(EMGReplayState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnReplayStateChanged.Broadcast(NewState);
	}
}

TArray<uint8> UMGReplaySubsystem::CompressReplayData(const FMGReplayData& Data)
{
	TArray<uint8> UncompressedData;
	FMemoryWriter Writer(UncompressedData);

	// Serialize replay data
	FMGReplayData DataCopy = Data;

	// Write header
	int32 Version = 1;
	Writer << Version;

	// Write metadata
	FString GuidStr = DataCopy.ReplayID.ToString();
	Writer << GuidStr;
	Writer << DataCopy.TrackID;
	Writer << DataCopy.VehicleID;
	Writer << DataCopy.PlayerName;
	Writer << DataCopy.TotalTime;
	Writer << DataCopy.BestLapTime;
	Writer << DataCopy.LapsCompleted;
	Writer << DataCopy.RecordingFPS;

	// Write frames
	int32 FrameCount = DataCopy.Frames.Num();
	Writer << FrameCount;

	for (const FMGReplayFrame& Frame : DataCopy.Frames)
	{
		Writer << const_cast<FMGReplayFrame&>(Frame).Timestamp;
		Writer << const_cast<FMGReplayFrame&>(Frame).Position;
		Writer << const_cast<FMGReplayFrame&>(Frame).Rotation;
		Writer << const_cast<FMGReplayFrame&>(Frame).Velocity;
		Writer << const_cast<FMGReplayFrame&>(Frame).SpeedKPH;
		Writer << const_cast<FMGReplayFrame&>(Frame).EngineRPM;
		Writer << const_cast<FMGReplayFrame&>(Frame).Gear;
		Writer << const_cast<FMGReplayFrame&>(Frame).ThrottleInput;
		Writer << const_cast<FMGReplayFrame&>(Frame).BrakeInput;
		Writer << const_cast<FMGReplayFrame&>(Frame).SteeringInput;
		Writer << const_cast<FMGReplayFrame&>(Frame).bIsDrifting;
		Writer << const_cast<FMGReplayFrame&>(Frame).bNOSActive;
	}

	// Would apply compression here (zlib/lz4)
	// For now, return uncompressed
	return UncompressedData;
}

FMGReplayData UMGReplaySubsystem::DecompressReplayData(const TArray<uint8>& CompressedData)
{
	FMGReplayData Result;

	if (CompressedData.Num() == 0)
	{
		return Result;
	}

	// Would decompress here if compression was applied
	TArray<uint8> UncompressedData = CompressedData;

	FMemoryReader Reader(UncompressedData);

	// Read header
	int32 Version;
	Reader << Version;

	if (Version != 1)
	{
		return Result; // Unsupported version
	}

	// Read metadata
	FString GuidStr;
	Reader << GuidStr;
	FGuid::Parse(GuidStr, Result.ReplayID);

	Reader << Result.TrackID;
	Reader << Result.VehicleID;
	Reader << Result.PlayerName;
	Reader << Result.TotalTime;
	Reader << Result.BestLapTime;
	Reader << Result.LapsCompleted;
	Reader << Result.RecordingFPS;

	// Read frames
	int32 FrameCount;
	Reader << FrameCount;

	Result.Frames.Reserve(FrameCount);

	for (int32 i = 0; i < FrameCount; ++i)
	{
		FMGReplayFrame Frame;
		Reader << Frame.Timestamp;
		Reader << Frame.Position;
		Reader << Frame.Rotation;
		Reader << Frame.Velocity;
		Reader << Frame.SpeedKPH;
		Reader << Frame.EngineRPM;
		Reader << Frame.Gear;
		Reader << Frame.ThrottleInput;
		Reader << Frame.BrakeInput;
		Reader << Frame.SteeringInput;
		Reader << Frame.bIsDrifting;
		Reader << Frame.bNOSActive;

		Result.Frames.Add(Frame);
	}

	return Result;
}

FString UMGReplaySubsystem::GetReplayFilePath(FGuid ReplayID) const
{
	FString ReplayDir = FPaths::ProjectSavedDir() / TEXT("Replays");
	IFileManager::Get().MakeDirectory(*ReplayDir, true);

	return ReplayDir / (ReplayID.ToString() + TEXT(".mgrep"));
}

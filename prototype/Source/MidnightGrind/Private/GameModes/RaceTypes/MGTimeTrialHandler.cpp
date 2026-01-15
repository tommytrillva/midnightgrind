// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGTimeTrialHandler.h"
#include "GameModes/MGRaceGameMode.h"
#include "Vehicle/MGVehiclePawn.h"

UMGTimeTrialHandler::UMGTimeTrialHandler()
{
	RecordingFramerate = 30.0f;
	TotalLaps = 5;
}

void UMGTimeTrialHandler::Initialize(AMGRaceGameMode* InGameMode)
{
	Super::Initialize(InGameMode);

	if (InGameMode)
	{
		TotalLaps = InGameMode->GetRaceConfig().LapCount;
	}
}

void UMGTimeTrialHandler::Reset()
{
	Super::Reset();

	CurrentRecording = FMGGhostReplay();
	BestLapRecording = FMGGhostReplay();

	CurrentLapTime = 0.0f;
	SessionBestLapTime = 0.0f;
	CurrentSectorTimes.Empty();
	CurrentSectorResults.Empty();
	RecordingAccumulator = 0.0f;
	CurrentLap = 0;
	CompletedLaps = 0;
}

void UMGTimeTrialHandler::OnRaceStarted()
{
	Super::OnRaceStarted();

	CurrentLap = 1;
	StartNewLapRecording();

	UE_LOG(LogTemp, Log, TEXT("Time Trial: Started with %d laps"), TotalLaps);
}

void UMGTimeTrialHandler::OnRaceTick(float DeltaTime)
{
	Super::OnRaceTick(DeltaTime);

	// Update lap time
	CurrentLapTime += DeltaTime;

	// Record frames at specified framerate
	RecordingAccumulator += DeltaTime;
	float FrameInterval = 1.0f / RecordingFramerate;

	while (RecordingAccumulator >= FrameInterval)
	{
		RecordFrame();
		RecordingAccumulator -= FrameInterval;
	}
}

void UMGTimeTrialHandler::OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex)
{
	// In time trial, only track player (racer 0)
	if (RacerIndex != 0)
	{
		return;
	}

	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	// Record sector time
	float SectorTime = CurrentLapTime;

	// If not first sector, calculate delta from previous
	if (CurrentSectorTimes.Num() > 0)
	{
		SectorTime = CurrentLapTime - CurrentSectorTimes.Last();
	}

	CurrentSectorTimes.Add(CurrentLapTime);

	// Calculate comparison
	FMGSectorResult Result = CalculateSectorResult(CheckpointIndex, SectorTime);
	CurrentSectorResults.Add(Result);

	// Broadcast sector result
	OnSectorResult.Broadcast(Result);
	Super::OnCheckpointPassed(RacerIndex, CheckpointIndex);

	UE_LOG(LogTemp, Log, TEXT("Time Trial: Sector %d - %.3fs (Delta: %+.3fs)"),
		CheckpointIndex, SectorTime, Result.Delta);
}

void UMGTimeTrialHandler::OnLapCompleted(int32 RacerIndex, float LapTime)
{
	Super::OnLapCompleted(RacerIndex, LapTime);

	if (RacerIndex != 0)
	{
		return;
	}

	// Finalize recording
	FinalizeLapRecording(LapTime);

	// Check for new bests
	bool bNewSessionBest = (SessionBestLapTime <= 0.0f || LapTime < SessionBestLapTime);
	bool bNewPersonalBest = (PersonalBestTime <= 0.0f || LapTime < PersonalBestTime);
	bool bNewTrackRecord = (TrackRecordTime <= 0.0f || LapTime < TrackRecordTime);

	if (bNewSessionBest)
	{
		SessionBestLapTime = LapTime;
		BestLapRecording = CurrentRecording;

		UE_LOG(LogTemp, Log, TEXT("Time Trial: New session best! %.3fs"), LapTime);
	}

	if (bNewPersonalBest)
	{
		PersonalBestTime = LapTime;
		PersonalBestGhost = CurrentRecording;
		OnNewPersonalBest.Broadcast(LapTime);

		UE_LOG(LogTemp, Log, TEXT("Time Trial: NEW PERSONAL BEST! %.3fs"), LapTime);
	}

	if (bNewTrackRecord)
	{
		TrackRecordTime = LapTime;
		TrackRecordGhost = CurrentRecording;
		OnNewTrackRecord.Broadcast(LapTime);

		UE_LOG(LogTemp, Log, TEXT("Time Trial: NEW TRACK RECORD! %.3fs"), LapTime);
	}

	// Broadcast ghost recorded
	OnGhostRecorded.Broadcast(CurrentRecording);

	// Increment lap
	CompletedLaps++;
	CurrentLap++;

	// Start new recording if more laps
	if (CurrentLap <= TotalLaps)
	{
		StartNewLapRecording();
	}
}

EMGRaceCompletionResult UMGTimeTrialHandler::CheckCompletionCondition(int32 RacerIndex)
{
	if (RacerIndex != 0)
	{
		return EMGRaceCompletionResult::InProgress;
	}

	// Time trial ends when all laps complete
	if (CompletedLaps >= TotalLaps)
	{
		return EMGRaceCompletionResult::Finished;
	}

	// Check time limit
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		FMGRaceConfig Config = GM->GetRaceConfig();
		if (Config.TimeLimit > 0.0f && GM->GetRaceTime() >= Config.TimeLimit)
		{
			return EMGRaceCompletionResult::Finished;
		}
	}

	return EMGRaceCompletionResult::InProgress;
}

void UMGTimeTrialHandler::CalculatePositions(TArray<int32>& OutPositions)
{
	// Time trial is solo - position is always 1
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		OutPositions.SetNum(GM->GetRacerCount());
		for (int32 i = 0; i < OutPositions.Num(); ++i)
		{
			OutPositions[i] = i + 1;
		}
	}
}

FText UMGTimeTrialHandler::GetDisplayName() const
{
	return NSLOCTEXT("RaceType", "TimeTrialName", "Time Trial");
}

FText UMGTimeTrialHandler::GetDescription() const
{
	return NSLOCTEXT("RaceType", "TimeTrialDesc", "Race against the clock! Set your best time and compete against ghost replays.");
}

FText UMGTimeTrialHandler::GetProgressFormat() const
{
	return NSLOCTEXT("RaceType", "TimeTrialProgress", "LAP {0}/{1}");
}

int64 UMGTimeTrialHandler::CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const
{
	// Time trial rewards based on beating target time
	int64 BaseCredits = 2000;

	if (TargetTime > 0.0f && SessionBestLapTime > 0.0f)
	{
		if (SessionBestLapTime <= TargetTime)
		{
			// Beat target - bonus!
			float BeatBy = (TargetTime - SessionBestLapTime) / TargetTime;
			BaseCredits = static_cast<int64>(5000 + (BeatBy * 10000));
		}
	}

	// Personal best bonus
	if (PersonalBestTime > 0.0f && SessionBestLapTime <= PersonalBestTime)
	{
		BaseCredits = static_cast<int64>(BaseCredits * 1.5f);
	}

	return BaseCredits;
}

void UMGTimeTrialHandler::SetComparisonGhost(const FMGGhostReplay& Ghost)
{
	ComparisonGhost = Ghost;
}

void UMGTimeTrialHandler::SetPersonalBestGhost(const FMGGhostReplay& Ghost)
{
	PersonalBestGhost = Ghost;
	if (Ghost.IsValid())
	{
		PersonalBestTime = Ghost.LapTime;
	}
}

void UMGTimeTrialHandler::SetTrackRecordGhost(const FMGGhostReplay& Ghost)
{
	TrackRecordGhost = Ghost;
	if (Ghost.IsValid())
	{
		TrackRecordTime = Ghost.LapTime;
	}
}

bool UMGTimeTrialHandler::GetGhostTransformAtTime(float Time, FVector& OutPosition, FRotator& OutRotation) const
{
	// Use comparison ghost if available, otherwise personal best
	const FMGGhostReplay* GhostToUse = nullptr;

	if (ComparisonGhost.IsValid())
	{
		GhostToUse = &ComparisonGhost;
	}
	else if (PersonalBestGhost.IsValid())
	{
		GhostToUse = &PersonalBestGhost;
	}

	if (!GhostToUse)
	{
		return false;
	}

	FMGGhostFrame Frame;
	if (InterpolateGhostFrame(*GhostToUse, Time, Frame))
	{
		OutPosition = Frame.Position;
		OutRotation = Frame.Rotation;
		return true;
	}

	return false;
}

float UMGTimeTrialHandler::GetCurrentDelta() const
{
	if (!ComparisonGhost.IsValid())
	{
		return 0.0f;
	}

	// Find ghost position at current time
	FMGGhostFrame GhostFrame;
	if (!InterpolateGhostFrame(ComparisonGhost, CurrentLapTime, GhostFrame))
	{
		return 0.0f;
	}

	// Get player position
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return 0.0f;
	}

	FMGRacerData PlayerData = GM->GetRacerData(0);
	if (!PlayerData.Vehicle.IsValid())
	{
		return 0.0f;
	}

	// Calculate distance difference
	// Positive = player ahead, Negative = ghost ahead
	FVector PlayerPos = PlayerData.Vehicle->GetActorLocation();
	FVector GhostPos = GhostFrame.Position;

	// For proper delta, would need to project onto track spline
	// For now, use simple distance comparison
	float GhostProgress = GhostFrame.Timestamp;
	float PlayerProgress = CurrentLapTime;

	// Time delta (negative = player is behind)
	return GhostProgress - PlayerProgress;
}

void UMGTimeTrialHandler::RecordFrame()
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	FMGRacerData PlayerData = GM->GetRacerData(0);
	if (!PlayerData.Vehicle.IsValid())
	{
		return;
	}

	AMGVehiclePawn* Vehicle = PlayerData.Vehicle.Get();

	FMGGhostFrame Frame;
	Frame.Timestamp = CurrentLapTime;
	Frame.Position = Vehicle->GetActorLocation();
	Frame.Rotation = Vehicle->GetActorRotation();
	Frame.Speed = Vehicle->GetVelocity().Size();
	// Frame.SteerAngle and bBraking would come from vehicle state

	CurrentRecording.Frames.Add(Frame);
}

void UMGTimeTrialHandler::StartNewLapRecording()
{
	CurrentRecording = FMGGhostReplay();
	CurrentRecording.RecordedDate = FDateTime::UtcNow();

	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		CurrentRecording.TrackId = GM->GetRaceConfig().TrackName;
	}

	CurrentLapTime = 0.0f;
	CurrentSectorTimes.Empty();
	CurrentSectorResults.Empty();
	RecordingAccumulator = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Time Trial: Started recording lap %d"), CurrentLap);
}

void UMGTimeTrialHandler::FinalizeLapRecording(float LapTime)
{
	CurrentRecording.LapTime = LapTime;
	CurrentRecording.SectorTimes = CurrentSectorTimes;

	UE_LOG(LogTemp, Log, TEXT("Time Trial: Finalized lap recording - %.3fs with %d frames"),
		LapTime, CurrentRecording.Frames.Num());
}

FMGSectorResult UMGTimeTrialHandler::CalculateSectorResult(int32 SectorIndex, float SectorTime) const
{
	FMGSectorResult Result;
	Result.SectorIndex = SectorIndex;
	Result.SectorTime = SectorTime;

	// Compare against personal best ghost
	if (PersonalBestGhost.IsValid() && PersonalBestGhost.SectorTimes.IsValidIndex(SectorIndex))
	{
		Result.ComparisonTime = PersonalBestGhost.SectorTimes[SectorIndex];
		Result.Delta = SectorTime - Result.ComparisonTime;

		if (FMath::Abs(Result.Delta) < 0.001f)
		{
			Result.Comparison = EMGSectorComparison::Equal;
		}
		else if (Result.Delta < 0.0f)
		{
			Result.Comparison = EMGSectorComparison::Faster;
		}
		else
		{
			Result.Comparison = EMGSectorComparison::Slower;
		}

		// Check if this is a new PB for this sector
		if (Result.Delta < 0.0f)
		{
			Result.bPersonalBest = true;
		}
	}
	else
	{
		Result.Comparison = EMGSectorComparison::NoComparison;
	}

	// Check track record
	if (TrackRecordGhost.IsValid() && TrackRecordGhost.SectorTimes.IsValidIndex(SectorIndex))
	{
		float RecordSectorTime = TrackRecordGhost.SectorTimes[SectorIndex];
		if (SectorTime < RecordSectorTime)
		{
			Result.bTrackRecord = true;
		}
	}

	return Result;
}

bool UMGTimeTrialHandler::InterpolateGhostFrame(const FMGGhostReplay& Ghost, float Time, FMGGhostFrame& OutFrame) const
{
	if (!Ghost.IsValid() || Ghost.Frames.Num() < 2)
	{
		return false;
	}

	// Find surrounding frames
	int32 FrameIndex = 0;
	for (int32 i = 0; i < Ghost.Frames.Num() - 1; ++i)
	{
		if (Ghost.Frames[i + 1].Timestamp > Time)
		{
			FrameIndex = i;
			break;
		}
	}

	// Clamp to valid range
	if (FrameIndex >= Ghost.Frames.Num() - 1)
	{
		OutFrame = Ghost.Frames.Last();
		return true;
	}

	const FMGGhostFrame& Frame1 = Ghost.Frames[FrameIndex];
	const FMGGhostFrame& Frame2 = Ghost.Frames[FrameIndex + 1];

	// Calculate interpolation factor
	float TimeDelta = Frame2.Timestamp - Frame1.Timestamp;
	float Alpha = (TimeDelta > 0.0f) ? (Time - Frame1.Timestamp) / TimeDelta : 0.0f;
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	// Interpolate
	OutFrame.Timestamp = Time;
	OutFrame.Position = FMath::Lerp(Frame1.Position, Frame2.Position, Alpha);
	OutFrame.Rotation = FMath::Lerp(Frame1.Rotation, Frame2.Rotation, Alpha);
	OutFrame.Speed = FMath::Lerp(Frame1.Speed, Frame2.Speed, Alpha);
	OutFrame.SteerAngle = FMath::Lerp(Frame1.SteerAngle, Frame2.SteerAngle, Alpha);
	OutFrame.bBraking = Frame1.bBraking; // Use nearest

	return true;
}

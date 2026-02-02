// Copyright Midnight Grind. All Rights Reserved.

#include "Telemetry/MGTelemetrySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

void UMGTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RecordingInterval = 0.05f;
	bIsRecording = false;
	bIsPaused = false;
	bOverlayVisible = true;

	OverlayConfig.Style = EMGTelemetryOverlayStyle::Standard;
	OverlayConfig.bShowSpeed = true;
	OverlayConfig.bShowRPM = true;
	OverlayConfig.bShowGear = true;
	OverlayConfig.bShowDelta = true;
	OverlayConfig.bShowInputs = true;
	OverlayConfig.OverlayOpacity = 0.8f;
}

void UMGTelemetrySubsystem::Deinitialize()
{
	StopRecording();
	Super::Deinitialize();
}

bool UMGTelemetrySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGTelemetrySubsystem::StartRecording()
{
	if (bIsRecording)
	{
		return;
	}

	bIsRecording = true;
	bIsPaused = false;

	CurrentLap = FMGLapTelemetry();
	CurrentLap.LapNumber = CurrentSession.TotalLaps + 1;
	CurrentLap.Frames.Empty();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TelemetryTickHandle,
			this,
			&UMGTelemetrySubsystem::OnTelemetryTick,
			RecordingInterval,
			true
		);
	}
}

void UMGTelemetrySubsystem::StopRecording()
{
	if (!bIsRecording)
	{
		return;
	}

	bIsRecording = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TelemetryTickHandle);
	}
}

void UMGTelemetrySubsystem::PauseRecording()
{
	bIsPaused = true;
}

void UMGTelemetrySubsystem::ResumeRecording()
{
	bIsPaused = false;
}

void UMGTelemetrySubsystem::SetRecordingRate(float FramesPerSecond)
{
	RecordingInterval = 1.0f / FMath::Max(1.0f, FramesPerSecond);

	if (bIsRecording)
	{
		StopRecording();
		StartRecording();
	}
}

void UMGTelemetrySubsystem::RecordFrame(const FMGTelemetryFrame& Frame)
{
	CurrentFrame = Frame;
	ProcessCurrentFrame();
}

FMGTelemetryFrame UMGTelemetrySubsystem::GetFrameAtTime(float Timestamp) const
{
	if (CurrentLap.Frames.Num() == 0)
	{
		return FMGTelemetryFrame();
	}

	for (int32 i = 0; i < CurrentLap.Frames.Num() - 1; ++i)
	{
		if (CurrentLap.Frames[i].Timestamp <= Timestamp && CurrentLap.Frames[i + 1].Timestamp >= Timestamp)
		{
			float Alpha = (Timestamp - CurrentLap.Frames[i].Timestamp) /
				(CurrentLap.Frames[i + 1].Timestamp - CurrentLap.Frames[i].Timestamp);
			return InterpolateFrames(CurrentLap.Frames[i], CurrentLap.Frames[i + 1], Alpha);
		}
	}

	return CurrentLap.Frames.Last();
}

void UMGTelemetrySubsystem::StartLap()
{
	CurrentLap = FMGLapTelemetry();
	CurrentLap.LapNumber = CurrentSession.TotalLaps + 1;
	CurrentLap.Frames.Empty();
	CurrentLap.SectorTimes.Empty();
	TotalDistance = 0.0f;
}

void UMGTelemetrySubsystem::CompleteLap(float LapTime)
{
	CurrentLap.LapTime = LapTime;
	UpdateLapStatistics();

	// Check for personal best
	if (BestLap.LapTime <= 0.0f || LapTime < BestLap.LapTime)
	{
		CurrentLap.bIsPersonalBest = true;
		BestLap = CurrentLap;
		OnPersonalBest.Broadcast(BestLap);
	}

	CurrentSession.Laps.Add(CurrentLap);
	CurrentSession.TotalLaps++;
	CurrentSession.TotalTime += LapTime;
	CurrentSession.TotalDistance += TotalDistance;

	OnLapCompleted.Broadcast(CurrentLap);

	StartLap();
}

void UMGTelemetrySubsystem::CompleteSector(int32 Sector, float SectorTime)
{
	while (CurrentLap.SectorTimes.Num() <= Sector)
	{
		CurrentLap.SectorTimes.Add(0.0f);
	}

	CurrentLap.SectorTimes[Sector] = SectorTime;
	OnSectorCompleted.Broadcast(Sector, SectorTime);
}

TArray<FMGLapTelemetry> UMGTelemetrySubsystem::GetAllLapsTelemetry() const
{
	return CurrentSession.Laps;
}

void UMGTelemetrySubsystem::StartSession(FName TrackID, FName VehicleID)
{
	CurrentSession = FMGTelemetrySession();
	CurrentSession.SessionID = FGuid::NewGuid();
	CurrentSession.StartTime = FDateTime::UtcNow();
	CurrentSession.TrackID = TrackID;
	CurrentSession.VehicleID = VehicleID;
	CurrentSession.TotalLaps = 0;
	CurrentSession.TotalTime = 0.0f;
	CurrentSession.TotalDistance = 0.0f;

	BestLap = FMGLapTelemetry();

	StartRecording();
}

void UMGTelemetrySubsystem::EndSession()
{
	StopRecording();

	CurrentSession.EndTime = FDateTime::UtcNow();
	CurrentSession.BestLap = BestLap;
}

void UMGTelemetrySubsystem::SaveSession(const FString& Filename)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	JsonObject->SetStringField(TEXT("SessionID"), CurrentSession.SessionID.ToString());
	JsonObject->SetStringField(TEXT("TrackID"), CurrentSession.TrackID.ToString());
	JsonObject->SetStringField(TEXT("VehicleID"), CurrentSession.VehicleID.ToString());
	JsonObject->SetNumberField(TEXT("TotalLaps"), CurrentSession.TotalLaps);
	JsonObject->SetNumberField(TEXT("TotalTime"), CurrentSession.TotalTime);
	JsonObject->SetNumberField(TEXT("TotalDistance"), CurrentSession.TotalDistance);

	TArray<TSharedPtr<FJsonValue>> LapsArray;
	for (const FMGLapTelemetry& Lap : CurrentSession.Laps)
	{
		TSharedPtr<FJsonObject> LapObject = MakeShareable(new FJsonObject());
		LapObject->SetNumberField(TEXT("LapNumber"), Lap.LapNumber);
		LapObject->SetNumberField(TEXT("LapTime"), Lap.LapTime);
		LapObject->SetNumberField(TEXT("MaxSpeed"), Lap.MaxSpeed);
		LapObject->SetNumberField(TEXT("AverageSpeed"), Lap.AverageSpeed);
		LapObject->SetBoolField(TEXT("IsPersonalBest"), Lap.bIsPersonalBest);
		LapsArray.Add(MakeShareable(new FJsonValueObject(LapObject)));
	}
	JsonObject->SetArrayField(TEXT("Laps"), LapsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(OutputString, *Filename);
}

bool UMGTelemetrySubsystem::LoadSession(const FString& Filename)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *Filename))
	{
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		return false;
	}

	FGuid::Parse(JsonObject->GetStringField(TEXT("SessionID")), CurrentSession.SessionID);
	CurrentSession.TrackID = FName(*JsonObject->GetStringField(TEXT("TrackID")));
	CurrentSession.VehicleID = FName(*JsonObject->GetStringField(TEXT("VehicleID")));
	CurrentSession.TotalLaps = JsonObject->GetIntegerField(TEXT("TotalLaps"));
	CurrentSession.TotalTime = JsonObject->GetNumberField(TEXT("TotalTime"));
	CurrentSession.TotalDistance = JsonObject->GetNumberField(TEXT("TotalDistance"));

	return true;
}

void UMGTelemetrySubsystem::SetReferenceLap(const FMGLapTelemetry& Lap)
{
	ReferenceLap = Lap;
	Comparison.ReferenceLap = Lap;
}

void UMGTelemetrySubsystem::SetReferenceLapFromBest()
{
	if (BestLap.LapTime > 0.0f)
	{
		SetReferenceLap(BestLap);
	}
}

void UMGTelemetrySubsystem::SetReferenceLapFromGhost(FName GhostID)
{
	// Load ghost lap data from replay system
}

float UMGTelemetrySubsystem::GetCurrentDelta() const
{
	return Comparison.DeltaTime;
}

float UMGTelemetrySubsystem::GetDeltaAtDistance(float Distance) const
{
	if (Comparison.DeltaAtDistance.Num() == 0)
	{
		return 0.0f;
	}

	int32 Index = FMath::Clamp(FMath::FloorToInt(Distance), 0, Comparison.DeltaAtDistance.Num() - 1);
	return Comparison.DeltaAtDistance[Index];
}

void UMGTelemetrySubsystem::SetOverlayConfig(const FMGTelemetryOverlayConfig& Config)
{
	OverlayConfig = Config;
}

void UMGTelemetrySubsystem::SetOverlayVisible(bool bVisible)
{
	bOverlayVisible = bVisible;
}

void UMGTelemetrySubsystem::SetOverlayStyle(EMGTelemetryOverlayStyle Style)
{
	OverlayConfig.Style = Style;

	switch (Style)
	{
	case EMGTelemetryOverlayStyle::Minimal:
		OverlayConfig.bShowSpeed = true;
		OverlayConfig.bShowRPM = false;
		OverlayConfig.bShowGear = true;
		OverlayConfig.bShowInputs = false;
		OverlayConfig.bShowGForce = false;
		OverlayConfig.bShowTireInfo = false;
		OverlayConfig.bShowDelta = true;
		break;

	case EMGTelemetryOverlayStyle::Standard:
		OverlayConfig.bShowSpeed = true;
		OverlayConfig.bShowRPM = true;
		OverlayConfig.bShowGear = true;
		OverlayConfig.bShowInputs = true;
		OverlayConfig.bShowGForce = false;
		OverlayConfig.bShowTireInfo = false;
		OverlayConfig.bShowDelta = true;
		break;

	case EMGTelemetryOverlayStyle::Detailed:
		OverlayConfig.bShowSpeed = true;
		OverlayConfig.bShowRPM = true;
		OverlayConfig.bShowGear = true;
		OverlayConfig.bShowInputs = true;
		OverlayConfig.bShowGForce = true;
		OverlayConfig.bShowTireInfo = true;
		OverlayConfig.bShowDelta = true;
		break;

	case EMGTelemetryOverlayStyle::Professional:
		OverlayConfig.bShowSpeed = true;
		OverlayConfig.bShowRPM = true;
		OverlayConfig.bShowGear = true;
		OverlayConfig.bShowInputs = true;
		OverlayConfig.bShowGForce = true;
		OverlayConfig.bShowTireInfo = true;
		OverlayConfig.bShowDelta = true;
		OverlayConfig.bShowMinimap = true;
		break;

	case EMGTelemetryOverlayStyle::Streamer:
		OverlayConfig.bShowSpeed = true;
		OverlayConfig.bShowRPM = true;
		OverlayConfig.bShowGear = true;
		OverlayConfig.bShowInputs = true;
		OverlayConfig.bShowGForce = false;
		OverlayConfig.bShowTireInfo = false;
		OverlayConfig.bShowDelta = true;
		OverlayConfig.OverlayOpacity = 0.6f;
		break;

	default:
		break;
	}
}

void UMGTelemetrySubsystem::ExportToCSV(const FString& Filename)
{
	FString CSVContent;

	// Header
	CSVContent += TEXT("Timestamp,Speed,SpeedMPH,RPM,Gear,Throttle,Brake,Steering,");
	CSVContent += TEXT("PosX,PosY,PosZ,LateralG,LongitudinalG,DriftAngle,TrackPercent,Lap\n");

	// Data
	for (const FMGTelemetryFrame& Frame : CurrentLap.Frames)
	{
		CSVContent += FString::Printf(
			TEXT("%.3f,%.2f,%.2f,%.0f,%d,%.3f,%.3f,%.3f,"),
			Frame.Timestamp, Frame.Speed, Frame.SpeedMPH, Frame.RPM, Frame.Gear,
			Frame.ThrottleInput, Frame.BrakeInput, Frame.SteeringInput
		);
		CSVContent += FString::Printf(
			TEXT("%.2f,%.2f,%.2f,%.3f,%.3f,%.2f,%.4f,%d\n"),
			Frame.Position.X, Frame.Position.Y, Frame.Position.Z,
			Frame.LateralG, Frame.LongitudinalG, Frame.DriftAngle,
			Frame.TrackPercentage, Frame.CurrentLap
		);
	}

	FFileHelper::SaveStringToFile(CSVContent, *Filename);
}

void UMGTelemetrySubsystem::ExportToJSON(const FString& Filename)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	TArray<TSharedPtr<FJsonValue>> FramesArray;
	for (const FMGTelemetryFrame& Frame : CurrentLap.Frames)
	{
		TSharedPtr<FJsonObject> FrameObject = MakeShareable(new FJsonObject());
		FrameObject->SetNumberField(TEXT("timestamp"), Frame.Timestamp);
		FrameObject->SetNumberField(TEXT("speed"), Frame.Speed);
		FrameObject->SetNumberField(TEXT("rpm"), Frame.RPM);
		FrameObject->SetNumberField(TEXT("gear"), Frame.Gear);
		FrameObject->SetNumberField(TEXT("throttle"), Frame.ThrottleInput);
		FrameObject->SetNumberField(TEXT("brake"), Frame.BrakeInput);
		FrameObject->SetNumberField(TEXT("steering"), Frame.SteeringInput);
		FrameObject->SetNumberField(TEXT("lateralG"), Frame.LateralG);
		FrameObject->SetNumberField(TEXT("longitudinalG"), Frame.LongitudinalG);
		FramesArray.Add(MakeShareable(new FJsonValueObject(FrameObject)));
	}

	JsonObject->SetArrayField(TEXT("frames"), FramesArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(OutputString, *Filename);
}

FString UMGTelemetrySubsystem::GetTelemetryAsString() const
{
	return FString::Printf(
		TEXT("Speed: %.1f | RPM: %.0f | Gear: %d | Delta: %+.3f"),
		CurrentFrame.Speed, CurrentFrame.RPM, CurrentFrame.Gear, Comparison.DeltaTime
	);
}

float UMGTelemetrySubsystem::GetAverageSpeed() const
{
	if (CurrentLap.Frames.Num() == 0)
	{
		return 0.0f;
	}

	float TotalSpeed = 0.0f;
	for (const FMGTelemetryFrame& Frame : CurrentLap.Frames)
	{
		TotalSpeed += Frame.Speed;
	}

	return TotalSpeed / CurrentLap.Frames.Num();
}

float UMGTelemetrySubsystem::GetMaxSpeed() const
{
	float MaxSpeed = 0.0f;
	for (const FMGTelemetryFrame& Frame : CurrentLap.Frames)
	{
		MaxSpeed = FMath::Max(MaxSpeed, Frame.Speed);
	}
	return MaxSpeed;
}

float UMGTelemetrySubsystem::GetMaxGForce() const
{
	float MaxG = 0.0f;
	for (const FMGTelemetryFrame& Frame : CurrentLap.Frames)
	{
		float TotalG = FMath::Sqrt(Frame.LateralG * Frame.LateralG + Frame.LongitudinalG * Frame.LongitudinalG);
		MaxG = FMath::Max(MaxG, TotalG);
	}
	return MaxG;
}

float UMGTelemetrySubsystem::GetBrakingEfficiency() const
{
	float TotalBrakingTime = 0.0f;
	float TotalBrakingDistance = 0.0f;

	for (int32 i = 1; i < CurrentLap.Frames.Num(); ++i)
	{
		const FMGTelemetryFrame& Prev = CurrentLap.Frames[i - 1];
		const FMGTelemetryFrame& Curr = CurrentLap.Frames[i];

		if (Curr.BrakeInput > 0.5f)
		{
			TotalBrakingTime += (Curr.Timestamp - Prev.Timestamp);
			TotalBrakingDistance += FVector::Dist(Prev.Position, Curr.Position);
		}
	}

	if (TotalBrakingTime <= 0.0f)
	{
		return 0.0f;
	}

	return TotalBrakingDistance / TotalBrakingTime;
}

TArray<FVector> UMGTelemetrySubsystem::GetDrivingLine() const
{
	TArray<FVector> Line;
	for (const FMGTelemetryFrame& Frame : CurrentLap.Frames)
	{
		Line.Add(Frame.Position);
	}
	return Line;
}

void UMGTelemetrySubsystem::OnTelemetryTick()
{
	if (bIsPaused)
	{
		return;
	}

	// Frame is expected to be recorded externally via RecordFrame
	// This tick handles comparison updates
	UpdateComparison();
}

void UMGTelemetrySubsystem::ProcessCurrentFrame()
{
	if (!bIsRecording || bIsPaused)
	{
		return;
	}

	// Update distance tracking
	if (CurrentLap.Frames.Num() > 0)
	{
		FVector LastPos = CurrentLap.Frames.Last().Position;
		TotalDistance += FVector::Dist(LastPos, CurrentFrame.Position);
	}

	CurrentLap.Frames.Add(CurrentFrame);
	OnTelemetryFrameRecorded.Broadcast(CurrentFrame);
}

void UMGTelemetrySubsystem::UpdateComparison()
{
	if (ReferenceLap.Frames.Num() == 0 || CurrentLap.Frames.Num() == 0)
	{
		Comparison.DeltaTime = 0.0f;
		return;
	}

	float CurrentTrackPos = CurrentFrame.TrackPercentage;

	// Find reference frame at same track position
	const FMGTelemetryFrame* RefFrame = nullptr;
	for (const FMGTelemetryFrame& Frame : ReferenceLap.Frames)
	{
		if (Frame.TrackPercentage >= CurrentTrackPos)
		{
			RefFrame = &Frame;
			break;
		}
	}

	if (RefFrame)
	{
		Comparison.DeltaTime = CurrentFrame.Timestamp - RefFrame->Timestamp;
		Comparison.bIsAhead = Comparison.DeltaTime < 0.0f;
		Comparison.CurrentLap = CurrentLap;

		OnDeltaUpdated.Broadcast(Comparison.DeltaTime);
	}
}

void UMGTelemetrySubsystem::UpdateLapStatistics()
{
	if (CurrentLap.Frames.Num() == 0)
	{
		return;
	}

	CurrentLap.MaxSpeed = GetMaxSpeed();
	CurrentLap.AverageSpeed = GetAverageSpeed();
	CurrentLap.MaxLateralG = 0.0f;
	CurrentLap.MaxLongitudinalG = 0.0f;

	int32 GearShifts = 0;
	int32 LastGear = 0;
	float TotalDriftAngle = 0.0f;
	int32 DriftCount = 0;
	bool bWasDrifting = false;

	for (const FMGTelemetryFrame& Frame : CurrentLap.Frames)
	{
		CurrentLap.MaxLateralG = FMath::Max(CurrentLap.MaxLateralG, FMath::Abs(Frame.LateralG));
		CurrentLap.MaxLongitudinalG = FMath::Max(CurrentLap.MaxLongitudinalG, FMath::Abs(Frame.LongitudinalG));

		if (Frame.Gear != LastGear && LastGear != 0)
		{
			GearShifts++;
		}
		LastGear = Frame.Gear;

		if (Frame.bIsDrifting)
		{
			TotalDriftAngle += FMath::Abs(Frame.DriftAngle);
			if (!bWasDrifting)
			{
				DriftCount++;
			}
			bWasDrifting = true;
		}
		else
		{
			bWasDrifting = false;
		}

		if (Frame.bNitroActive)
		{
			CurrentLap.NitroUsed += RecordingInterval;
		}

		CurrentLap.TopGear = FMath::Max(CurrentLap.TopGear, (float)Frame.Gear);
	}

	CurrentLap.GearShifts = GearShifts;
	CurrentLap.TotalDriftAngle = TotalDriftAngle;
	CurrentLap.DriftCount = DriftCount;
}

FMGTelemetryFrame UMGTelemetrySubsystem::InterpolateFrames(const FMGTelemetryFrame& A, const FMGTelemetryFrame& B, float Alpha) const
{
	FMGTelemetryFrame Result;

	Result.Timestamp = FMath::Lerp(A.Timestamp, B.Timestamp, Alpha);
	Result.Speed = FMath::Lerp(A.Speed, B.Speed, Alpha);
	Result.SpeedMPH = FMath::Lerp(A.SpeedMPH, B.SpeedMPH, Alpha);
	Result.RPM = FMath::Lerp(A.RPM, B.RPM, Alpha);
	Result.Gear = (Alpha < 0.5f) ? A.Gear : B.Gear;
	Result.ThrottleInput = FMath::Lerp(A.ThrottleInput, B.ThrottleInput, Alpha);
	Result.BrakeInput = FMath::Lerp(A.BrakeInput, B.BrakeInput, Alpha);
	Result.SteeringInput = FMath::Lerp(A.SteeringInput, B.SteeringInput, Alpha);
	Result.Position = FMath::Lerp(A.Position, B.Position, Alpha);
	Result.Rotation = FMath::Lerp(A.Rotation, B.Rotation, Alpha);
	Result.LateralG = FMath::Lerp(A.LateralG, B.LateralG, Alpha);
	Result.LongitudinalG = FMath::Lerp(A.LongitudinalG, B.LongitudinalG, Alpha);
	Result.TrackPercentage = FMath::Lerp(A.TrackPercentage, B.TrackPercentage, Alpha);

	return Result;
}

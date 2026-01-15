// Copyright Midnight Grind. All Rights Reserved.

#include "Replay/MGReplayRecordingComponent.h"
#include "Replay/MGReplaySubsystem.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UMGReplayRecordingComponent::UMGReplayRecordingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UMGReplayRecordingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get replay subsystem
	if (UWorld* World = GetWorld())
	{
		ReplaySubsystem = World->GetSubsystem<UMGReplaySubsystem>();
	}

	// Pre-allocate frame buffer
	int32 ExpectedFrames = static_cast<int32>(MaxRecordingDuration * RecordingFPS);
	if (RecordingMode == EMGRecordingMode::Continuous)
	{
		RecordedFrames.SetNum(CircularBufferSize);
	}
	else
	{
		RecordedFrames.Reserve(ExpectedFrames);
	}

	FrameInterval = 1.0f / RecordingFPS;

	// Auto-start for continuous mode
	if (RecordingMode == EMGRecordingMode::Continuous)
	{
		StartRecording();
	}
}

void UMGReplayRecordingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsRecording)
	{
		CancelRecording();
	}

	Super::EndPlay(EndPlayReason);
}

void UMGReplayRecordingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsRecording)
	{
		return;
	}

	RecordingAccumulator += DeltaTime;

	// Record frames at fixed interval
	while (RecordingAccumulator >= FrameInterval)
	{
		RecordingAccumulator -= FrameInterval;
		RecordFrame();

		// Check max duration (non-continuous modes)
		if (RecordingMode != EMGRecordingMode::Continuous)
		{
			if (GetRecordingDuration() >= MaxRecordingDuration)
			{
				StopRecording();
				return;
			}
		}
	}
}

// ==========================================
// RECORDING CONTROL
// ==========================================

void UMGReplayRecordingComponent::StartRecording()
{
	if (bIsRecording)
	{
		return;
	}

	bIsRecording = true;
	RecordingAccumulator = 0.0f;
	RecordingStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (RecordingMode != EMGRecordingMode::Continuous)
	{
		RecordedFrames.Empty();
	}

	CircularHead = 0;

	OnRecordingStarted.Broadcast();
}

FMGReplayData UMGReplayRecordingComponent::StopRecording()
{
	if (!bIsRecording)
	{
		return FMGReplayData();
	}

	bIsRecording = false;

	FMGReplayData ReplayData = GetCurrentRecordingData();

	OnRecordingStopped.Broadcast(ReplayData);

	// Clear frames for non-continuous modes
	if (RecordingMode != EMGRecordingMode::Continuous)
	{
		RecordedFrames.Empty();
	}

	return ReplayData;
}

void UMGReplayRecordingComponent::CancelRecording()
{
	bIsRecording = false;

	if (RecordingMode != EMGRecordingMode::Continuous)
	{
		RecordedFrames.Empty();
	}
}

float UMGReplayRecordingComponent::GetRecordingDuration() const
{
	if (RecordingMode == EMGRecordingMode::Continuous)
	{
		// Duration is the buffer size or actual recorded time
		int32 FrameCount = FMath::Min(CircularHead, CircularBufferSize);
		return FrameCount * FrameInterval;
	}
	else
	{
		return RecordedFrames.Num() * FrameInterval;
	}
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGReplayRecordingComponent::SetRecordingFPS(float FPS)
{
	RecordingFPS = FMath::Clamp(FPS, 10.0f, 60.0f);
	FrameInterval = 1.0f / RecordingFPS;
}

// ==========================================
// DATA ACCESS
// ==========================================

FMGReplayData UMGReplayRecordingComponent::GetCurrentRecordingData() const
{
	FMGReplayData ReplayData;
	ReplayData.ReplayID = FGuid::NewGuid();
	ReplayData.TrackID = TrackID;
	ReplayData.VehicleID = VehicleID;
	ReplayData.PlayerName = PlayerName;
	ReplayData.BestLapTime = BestLapTime;
	ReplayData.LapsCompleted = LapsCompleted;
	ReplayData.RecordedDate = FDateTime::Now();
	ReplayData.RecordingFPS = RecordingFPS;

	if (RecordingMode == EMGRecordingMode::Continuous)
	{
		// Unroll circular buffer
		int32 FrameCount = FMath::Min(CircularHead, CircularBufferSize);
		ReplayData.Frames.Reserve(FrameCount);

		if (CircularHead >= CircularBufferSize)
		{
			// Buffer has wrapped, start from oldest frame
			int32 StartIndex = CircularHead % CircularBufferSize;

			for (int32 i = 0; i < CircularBufferSize; ++i)
			{
				int32 Index = (StartIndex + i) % CircularBufferSize;
				ReplayData.Frames.Add(RecordedFrames[Index]);
			}
		}
		else
		{
			// Buffer hasn't wrapped yet
			for (int32 i = 0; i < CircularHead; ++i)
			{
				ReplayData.Frames.Add(RecordedFrames[i]);
			}
		}

		// Renormalize timestamps to start from 0
		if (ReplayData.Frames.Num() > 0)
		{
			float FirstTimestamp = ReplayData.Frames[0].Timestamp;
			for (FMGReplayFrame& Frame : ReplayData.Frames)
			{
				Frame.Timestamp -= FirstTimestamp;
			}
		}
	}
	else
	{
		ReplayData.Frames = RecordedFrames;
	}

	if (ReplayData.Frames.Num() > 0)
	{
		ReplayData.TotalTime = ReplayData.Frames.Last().Timestamp;
	}

	return ReplayData;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGReplayRecordingComponent::RecordFrame()
{
	FMGReplayFrame Frame = BuildCurrentFrame();

	if (RecordingMode == EMGRecordingMode::Continuous)
	{
		// Circular buffer
		int32 Index = CircularHead % CircularBufferSize;
		RecordedFrames[Index] = Frame;
		CircularHead++;
	}
	else
	{
		RecordedFrames.Add(Frame);
	}
}

FMGReplayFrame UMGReplayRecordingComponent::BuildCurrentFrame() const
{
	FMGReplayFrame Frame;

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return Frame;
	}

	// Calculate timestamp
	if (RecordingMode == EMGRecordingMode::Continuous)
	{
		Frame.Timestamp = CircularHead * FrameInterval;
	}
	else
	{
		Frame.Timestamp = RecordedFrames.Num() * FrameInterval;
	}

	// Position and rotation
	Frame.Position = Owner->GetActorLocation();
	Frame.Rotation = Owner->GetActorRotation();
	Frame.Velocity = Owner->GetVelocity();

	// Vehicle inputs
	GetVehicleInputs(Frame.ThrottleInput, Frame.BrakeInput, Frame.SteeringInput);

	// Vehicle state
	GetVehicleState(Frame.SpeedKPH, Frame.EngineRPM, Frame.Gear, Frame.bIsDrifting, Frame.bNOSActive);

	// Wheel positions
	Frame.WheelPositions = GetWheelPositions();

	return Frame;
}

void UMGReplayRecordingComponent::GetVehicleInputs(float& OutThrottle, float& OutBrake, float& OutSteering) const
{
	OutThrottle = 0.0f;
	OutBrake = 0.0f;
	OutSteering = 0.0f;

	// Would query the vehicle movement component for actual inputs
	// For now, use default values

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Example: Try to get from pawn's movement component
	// This would be customized per vehicle implementation
}

void UMGReplayRecordingComponent::GetVehicleState(float& OutSpeedKPH, float& OutRPM, int32& OutGear, bool& OutDrifting, bool& OutNOS) const
{
	OutSpeedKPH = 0.0f;
	OutRPM = 0.0f;
	OutGear = 1;
	OutDrifting = false;
	OutNOS = false;

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Calculate speed from velocity
	FVector Velocity = Owner->GetVelocity();
	float SpeedCMS = Velocity.Size(); // cm/s
	OutSpeedKPH = SpeedCMS * 0.036f; // Convert to KPH

	// Would query vehicle component for other values
	// This would be customized per vehicle implementation
}

TArray<FVector> UMGReplayRecordingComponent::GetWheelPositions() const
{
	TArray<FVector> WheelPositions;

	// Would query wheel positions from vehicle mesh/physics
	// For now, return empty array

	return WheelPositions;
}

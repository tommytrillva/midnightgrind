// Copyright Midnight Grind. All Rights Reserved.

#include "Multiplayer/MGNetworkReplicationComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"

UMGNetworkReplicationComponent::UMGNetworkReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UMGNetworkReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize snapshot buffer
	SnapshotBuffer.SetNum(SnapshotBufferSize);

	// Check if locally controlled
	if (AActor* Owner = GetOwner())
	{
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			bIsLocallyControlled = Pawn->IsLocallyControlled();
		}
	}
}

void UMGNetworkReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update interpolation time
	InterpolationTime += DeltaTime;

	// Send snapshots if locally controlled
	if (bIsLocallyControlled)
	{
		SendAccumulator += DeltaTime;
		float SendInterval = 1.0f / SendRate;

		while (SendAccumulator >= SendInterval)
		{
			SendAccumulator -= SendInterval;

			// Build and send snapshot
			FMGNetworkSnapshot Snapshot;
			Snapshot.Timestamp = InterpolationTime;

			if (AActor* Owner = GetOwner())
			{
				Snapshot.Position = Owner->GetActorLocation();
				Snapshot.Rotation = Owner->GetActorRotation();
				Snapshot.Velocity = Owner->GetVelocity();
			}

			SendSnapshot(Snapshot);
		}
	}
	else
	{
		// Update interpolated position for remote vehicles
		AActor* Owner = GetOwner();
		if (Owner && SnapshotBuffer.Num() > 0)
		{
			FMGNetworkSnapshot Interpolated = GetInterpolatedSnapshot(InterpolationTime - InterpolationDelay);

			// Apply interpolated state
			Owner->SetActorLocation(Interpolated.Position);
			Owner->SetActorRotation(Interpolated.Rotation);
		}
	}
}

void UMGNetworkReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMGNetworkReplicationComponent, ServerSnapshot);
}

// ==========================================
// REPLICATION
// ==========================================

void UMGNetworkReplicationComponent::SendSnapshot(const FMGNetworkSnapshot& Snapshot)
{
	// In a real implementation, this would RPC to server
	// Server validates and broadcasts to other clients

	// For local testing, just add to buffer
	AddSnapshotToBuffer(Snapshot);
}

void UMGNetworkReplicationComponent::ReceiveSnapshot(const FMGNetworkSnapshot& Snapshot)
{
	// Calculate latency
	float Latency = InterpolationTime - Snapshot.Timestamp;
	UpdateLatencyStats(Latency);

	// Add to buffer
	AddSnapshotToBuffer(Snapshot);

	SnapshotsReceived++;

	// Check for reconciliation (if locally controlled)
	if (bIsLocallyControlled)
	{
		FMGNetworkSnapshot LocalSnapshot;
		LocalSnapshot.Timestamp = Snapshot.Timestamp;

		if (AActor* Owner = GetOwner())
		{
			LocalSnapshot.Position = Owner->GetActorLocation();
			LocalSnapshot.Rotation = Owner->GetActorRotation();
		}

		if (NeedsReconciliation(LocalSnapshot, Snapshot))
		{
			ApplyReconciliation(Snapshot);
		}
	}
}

void UMGNetworkReplicationComponent::ForceReconcile()
{
	if (SnapshotBuffer.Num() > 0)
	{
		// Get latest server snapshot
		int32 LatestIndex = (BufferHead - 1 + SnapshotBufferSize) % SnapshotBufferSize;
		ApplyReconciliation(SnapshotBuffer[LatestIndex]);
	}
}

// ==========================================
// INTERPOLATION
// ==========================================

FVector UMGNetworkReplicationComponent::GetInterpolatedPosition() const
{
	FMGNetworkSnapshot Snapshot = GetInterpolatedSnapshot(InterpolationTime - InterpolationDelay);
	return Snapshot.Position;
}

FRotator UMGNetworkReplicationComponent::GetInterpolatedRotation() const
{
	FMGNetworkSnapshot Snapshot = GetInterpolatedSnapshot(InterpolationTime - InterpolationDelay);
	return Snapshot.Rotation;
}

FVector UMGNetworkReplicationComponent::GetInterpolatedVelocity() const
{
	FMGNetworkSnapshot Snapshot = GetInterpolatedSnapshot(InterpolationTime - InterpolationDelay);
	return Snapshot.Velocity;
}

FMGNetworkSnapshot UMGNetworkReplicationComponent::GetInterpolatedSnapshot(float Time) const
{
	FMGNetworkSnapshot Prev, Next;
	float Alpha;

	if (GetInterpolationSnapshots(Prev, Next, Alpha))
	{
		return InterpolateSnapshots(Prev, Next, Alpha);
	}

	// No valid snapshots for interpolation, try extrapolation
	if (BufferHead > 0)
	{
		int32 LatestIndex = (BufferHead - 1 + SnapshotBufferSize) % SnapshotBufferSize;
		const FMGNetworkSnapshot& Latest = SnapshotBuffer[LatestIndex];

		float ExtrapolationTime = Time - Latest.Timestamp;
		if (ExtrapolationTime > 0.0f && ExtrapolationTime <= MaxExtrapolationTime)
		{
			return ExtrapolateSnapshot(Latest, ExtrapolationTime);
		}

		return Latest;
	}

	return FMGNetworkSnapshot();
}

// ==========================================
// INTERNAL
// ==========================================

void UMGNetworkReplicationComponent::AddSnapshotToBuffer(const FMGNetworkSnapshot& Snapshot)
{
	SnapshotBuffer[BufferHead] = Snapshot;
	BufferHead = (BufferHead + 1) % SnapshotBufferSize;
}

bool UMGNetworkReplicationComponent::GetInterpolationSnapshots(FMGNetworkSnapshot& OutPrev, FMGNetworkSnapshot& OutNext, float& OutAlpha) const
{
	float TargetTime = InterpolationTime - InterpolationDelay;

	// Find surrounding snapshots
	int32 PrevIndex = -1;
	int32 NextIndex = -1;

	for (int32 i = 0; i < SnapshotBufferSize; ++i)
	{
		int32 Index = (BufferHead - 1 - i + SnapshotBufferSize) % SnapshotBufferSize;
		const FMGNetworkSnapshot& Snapshot = SnapshotBuffer[Index];

		if (Snapshot.Timestamp == 0.0f)
		{
			continue; // Empty slot
		}

		if (Snapshot.Timestamp <= TargetTime)
		{
			PrevIndex = Index;
			break;
		}

		NextIndex = Index;
	}

	if (PrevIndex == -1)
	{
		return false;
	}

	if (NextIndex == -1)
	{
		// Only have previous, use it directly
		OutPrev = SnapshotBuffer[PrevIndex];
		OutNext = OutPrev;
		OutAlpha = 0.0f;
		return true;
	}

	OutPrev = SnapshotBuffer[PrevIndex];
	OutNext = SnapshotBuffer[NextIndex];

	float Duration = OutNext.Timestamp - OutPrev.Timestamp;
	OutAlpha = Duration > 0.0f ? (TargetTime - OutPrev.Timestamp) / Duration : 0.0f;
	OutAlpha = FMath::Clamp(OutAlpha, 0.0f, 1.0f);

	return true;
}

FMGNetworkSnapshot UMGNetworkReplicationComponent::InterpolateSnapshots(const FMGNetworkSnapshot& A, const FMGNetworkSnapshot& B, float Alpha) const
{
	FMGNetworkSnapshot Result;

	Result.Timestamp = FMath::Lerp(A.Timestamp, B.Timestamp, Alpha);

	switch (InterpolationMode)
	{
		case EMGNetInterpolationMode::Linear:
		{
			Result.Position = FMath::Lerp(A.Position, B.Position, Alpha);
			Result.Rotation = FMath::Lerp(A.Rotation, B.Rotation, Alpha);
			Result.Velocity = FMath::Lerp(A.Velocity, B.Velocity, Alpha);
			Result.AngularVelocity = FMath::Lerp(A.AngularVelocity, B.AngularVelocity, Alpha);
			break;
		}

		case EMGNetInterpolationMode::Hermite:
		{
			// Use Hermite interpolation for smoother results
			float SmoothedAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
			Result.Position = FMath::Lerp(A.Position, B.Position, SmoothedAlpha);
			Result.Rotation = FMath::Lerp(A.Rotation, B.Rotation, SmoothedAlpha);
			Result.Velocity = FMath::Lerp(A.Velocity, B.Velocity, SmoothedAlpha);
			Result.AngularVelocity = FMath::Lerp(A.AngularVelocity, B.AngularVelocity, SmoothedAlpha);
			break;
		}

		case EMGNetInterpolationMode::Predictive:
		{
			// Use velocity for prediction
			float TimeDelta = B.Timestamp - A.Timestamp;
			float TargetTime = A.Timestamp + TimeDelta * Alpha;

			// Predict position using velocity
			FVector PredictedA = A.Position + A.Velocity * (TargetTime - A.Timestamp);
			FVector PredictedB = B.Position - B.Velocity * (B.Timestamp - TargetTime);

			Result.Position = FMath::Lerp(PredictedA, PredictedB, Alpha);
			Result.Rotation = FMath::Lerp(A.Rotation, B.Rotation, Alpha);
			Result.Velocity = FMath::Lerp(A.Velocity, B.Velocity, Alpha);
			Result.AngularVelocity = FMath::Lerp(A.AngularVelocity, B.AngularVelocity, Alpha);
			break;
		}
	}

	// Interpolate inputs
	Result.ThrottleInput = FMath::Lerp(A.ThrottleInput, B.ThrottleInput, Alpha);
	Result.BrakeInput = FMath::Lerp(A.BrakeInput, B.BrakeInput, Alpha);
	Result.SteeringInput = FMath::Lerp(A.SteeringInput, B.SteeringInput, Alpha);

	// Discrete values take from nearest
	Result.Gear = Alpha < 0.5f ? A.Gear : B.Gear;
	Result.bIsDrifting = Alpha < 0.5f ? A.bIsDrifting : B.bIsDrifting;
	Result.bNOSActive = Alpha < 0.5f ? A.bNOSActive : B.bNOSActive;

	return Result;
}

FMGNetworkSnapshot UMGNetworkReplicationComponent::ExtrapolateSnapshot(const FMGNetworkSnapshot& Snapshot, float DeltaTime) const
{
	FMGNetworkSnapshot Result = Snapshot;

	// Extrapolate position using velocity
	Result.Position = Snapshot.Position + Snapshot.Velocity * DeltaTime;

	// Extrapolate rotation using angular velocity
	FRotator AngularDelta(
		Snapshot.AngularVelocity.X * DeltaTime,
		Snapshot.AngularVelocity.Y * DeltaTime,
		Snapshot.AngularVelocity.Z * DeltaTime
	);
	Result.Rotation = Snapshot.Rotation + AngularDelta;

	Result.Timestamp = Snapshot.Timestamp + DeltaTime;

	return Result;
}

void UMGNetworkReplicationComponent::UpdateLatencyStats(float NewLatency)
{
	// Exponential moving average for latency
	const float Alpha = 0.1f;
	float OldLatency = CurrentLatency;
	CurrentLatency = CurrentLatency * (1.0f - Alpha) + NewLatency * Alpha;

	// Update jitter (variance in latency)
	float LatencyDiff = FMath::Abs(NewLatency - OldLatency);
	CurrentJitter = CurrentJitter * (1.0f - Alpha) + LatencyDiff * Alpha;
}

bool UMGNetworkReplicationComponent::NeedsReconciliation(const FMGNetworkSnapshot& Local, const FMGNetworkSnapshot& Server) const
{
	// Check position error
	float PositionError = FVector::Dist(Local.Position, Server.Position);
	if (PositionError > PositionErrorThreshold)
	{
		return true;
	}

	// Check rotation error
	FRotator RotationDiff = (Local.Rotation - Server.Rotation).GetNormalized();
	float RotationError = FMath::Abs(RotationDiff.Yaw) + FMath::Abs(RotationDiff.Pitch) + FMath::Abs(RotationDiff.Roll);
	if (RotationError > RotationErrorThreshold)
	{
		return true;
	}

	return false;
}

void UMGNetworkReplicationComponent::ApplyReconciliation(const FMGNetworkSnapshot& ServerState)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Snap to server position
	// In a more sophisticated implementation, this would replay inputs from the reconciliation point
	Owner->SetActorLocation(ServerState.Position);
	Owner->SetActorRotation(ServerState.Rotation);

	// Reset velocity (would need physics component access)
}

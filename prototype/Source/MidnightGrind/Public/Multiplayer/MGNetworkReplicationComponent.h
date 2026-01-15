// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGNetworkReplicationComponent.generated.h"

/**
 * Network snapshot for replication
 */
USTRUCT()
struct FMGNetworkSnapshot
{
	GENERATED_BODY()

	/** Server timestamp */
	UPROPERTY()
	float Timestamp = 0.0f;

	/** Position */
	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	/** Rotation */
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	/** Velocity */
	UPROPERTY()
	FVector Velocity = FVector::ZeroVector;

	/** Angular velocity */
	UPROPERTY()
	FVector AngularVelocity = FVector::ZeroVector;

	/** Throttle input */
	UPROPERTY()
	float ThrottleInput = 0.0f;

	/** Brake input */
	UPROPERTY()
	float BrakeInput = 0.0f;

	/** Steering input */
	UPROPERTY()
	float SteeringInput = 0.0f;

	/** Current gear */
	UPROPERTY()
	int32 Gear = 1;

	/** Is drifting */
	UPROPERTY()
	bool bIsDrifting = false;

	/** NOS active */
	UPROPERTY()
	bool bNOSActive = false;
};

/**
 * Interpolation mode
 */
UENUM(BlueprintType)
enum class EMGNetInterpolationMode : uint8
{
	/** Simple linear interpolation */
	Linear,
	/** Smooth Hermite interpolation */
	Hermite,
	/** Predictive with extrapolation */
	Predictive
};

/**
 * Network Replication Component
 * Handles network state synchronization for vehicles
 *
 * Features:
 * - Snapshot-based state replication
 * - Client-side interpolation
 * - Server reconciliation
 * - Lag compensation
 */
UCLASS(ClassGroup = (Network), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGNetworkReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGNetworkReplicationComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==========================================
	// REPLICATION
	// ==========================================

	/** Send snapshot to server (client authority) */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendSnapshot(const FMGNetworkSnapshot& Snapshot);

	/** Receive snapshot from server */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ReceiveSnapshot(const FMGNetworkSnapshot& Snapshot);

	/** Force reconciliation with server state */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void ForceReconcile();

	// ==========================================
	// INTERPOLATION
	// ==========================================

	/** Get interpolated position */
	UFUNCTION(BlueprintPure, Category = "Network")
	FVector GetInterpolatedPosition() const;

	/** Get interpolated rotation */
	UFUNCTION(BlueprintPure, Category = "Network")
	FRotator GetInterpolatedRotation() const;

	/** Get interpolated velocity */
	UFUNCTION(BlueprintPure, Category = "Network")
	FVector GetInterpolatedVelocity() const;

	/** Get interpolated snapshot at time */
	UFUNCTION(BlueprintPure, Category = "Network")
	FMGNetworkSnapshot GetInterpolatedSnapshot(float Time) const;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set interpolation mode */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void SetInterpolationMode(EMGNetInterpolationMode Mode) { InterpolationMode = Mode; }

	/** Set interpolation delay */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void SetInterpolationDelay(float Delay) { InterpolationDelay = Delay; }

	/** Set send rate */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void SetSendRate(float Rate) { SendRate = Rate; }

	// ==========================================
	// QUERY
	// ==========================================

	/** Is this locally controlled */
	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsLocallyControlled() const { return bIsLocallyControlled; }

	/** Get current latency */
	UFUNCTION(BlueprintPure, Category = "Network")
	float GetCurrentLatency() const { return CurrentLatency; }

	/** Get jitter */
	UFUNCTION(BlueprintPure, Category = "Network")
	float GetJitter() const { return CurrentJitter; }

	/** Get packet loss percentage */
	UFUNCTION(BlueprintPure, Category = "Network")
	float GetPacketLoss() const { return PacketLossPercent; }

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Interpolation mode */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	EMGNetInterpolationMode InterpolationMode = EMGNetInterpolationMode::Hermite;

	/** Interpolation delay (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float InterpolationDelay = 0.1f;

	/** Send rate (Hz) */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float SendRate = 30.0f;

	/** Snapshot buffer size */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	int32 SnapshotBufferSize = 60;

	/** Position error threshold for reconciliation */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float PositionErrorThreshold = 50.0f;

	/** Rotation error threshold (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float RotationErrorThreshold = 10.0f;

	/** Max extrapolation time */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float MaxExtrapolationTime = 0.25f;

	// ==========================================
	// REPLICATED STATE
	// ==========================================

	/** Latest server snapshot */
	UPROPERTY(Replicated)
	FMGNetworkSnapshot ServerSnapshot;

	// ==========================================
	// STATE
	// ==========================================

	/** Is this locally controlled */
	bool bIsLocallyControlled = false;

	/** Snapshot buffer (ring buffer) */
	TArray<FMGNetworkSnapshot> SnapshotBuffer;

	/** Buffer head index */
	int32 BufferHead = 0;

	/** Current interpolation time */
	float InterpolationTime = 0.0f;

	/** Send accumulator */
	float SendAccumulator = 0.0f;

	/** Current latency estimate */
	float CurrentLatency = 0.0f;

	/** Latency jitter */
	float CurrentJitter = 0.0f;

	/** Packet loss percentage */
	float PacketLossPercent = 0.0f;

	/** Snapshots received count (for packet loss calc) */
	int32 SnapshotsReceived = 0;

	/** Snapshots expected count */
	int32 SnapshotsExpected = 0;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Add snapshot to buffer */
	void AddSnapshotToBuffer(const FMGNetworkSnapshot& Snapshot);

	/** Get snapshots for interpolation */
	bool GetInterpolationSnapshots(FMGNetworkSnapshot& OutPrev, FMGNetworkSnapshot& OutNext, float& OutAlpha) const;

	/** Interpolate between snapshots */
	FMGNetworkSnapshot InterpolateSnapshots(const FMGNetworkSnapshot& A, const FMGNetworkSnapshot& B, float Alpha) const;

	/** Extrapolate from snapshot */
	FMGNetworkSnapshot ExtrapolateSnapshot(const FMGNetworkSnapshot& Snapshot, float DeltaTime) const;

	/** Update latency statistics */
	void UpdateLatencyStats(float NewLatency);

	/** Check if reconciliation needed */
	bool NeedsReconciliation(const FMGNetworkSnapshot& Local, const FMGNetworkSnapshot& Server) const;

	/** Apply reconciliation */
	void ApplyReconciliation(const FMGNetworkSnapshot& ServerState);
};

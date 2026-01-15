// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Replay/MGReplaySubsystem.h"
#include "MGReplayRecordingComponent.generated.h"

class UMGReplaySubsystem;

/**
 * Recording mode
 */
UENUM(BlueprintType)
enum class EMGRecordingMode : uint8
{
	/** Manual start/stop */
	Manual,
	/** Auto-start on race begin */
	AutoRace,
	/** Always recording (circular buffer) */
	Continuous
};

/**
 * Delegate for recording events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingStopped, const FMGReplayData&, ReplayData);

/**
 * Replay Recording Component
 * Attached to vehicles to record replay data
 *
 * Features:
 * - Automatic frame recording at configurable rate
 * - Multiple recording modes
 * - Circular buffer for continuous recording
 * - Integration with vehicle components
 */
UCLASS(ClassGroup = (Replay), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGReplayRecordingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGReplayRecordingComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when recording starts */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRecordingStarted OnRecordingStarted;

	/** Called when recording stops */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRecordingStopped OnRecordingStopped;

	// ==========================================
	// RECORDING CONTROL
	// ==========================================

	/** Start recording */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void StartRecording();

	/** Stop recording and return replay data */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	FMGReplayData StopRecording();

	/** Cancel recording without saving */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void CancelRecording();

	/** Is currently recording */
	UFUNCTION(BlueprintPure, Category = "Recording")
	bool IsRecording() const { return bIsRecording; }

	/** Get current recording duration */
	UFUNCTION(BlueprintPure, Category = "Recording")
	float GetRecordingDuration() const;

	/** Get current frame count */
	UFUNCTION(BlueprintPure, Category = "Recording")
	int32 GetFrameCount() const { return RecordedFrames.Num(); }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set track ID for recording */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void SetTrackID(FName NewTrackID) { TrackID = NewTrackID; }

	/** Set vehicle ID for recording */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void SetVehicleID(FName NewVehicleID) { VehicleID = NewVehicleID; }

	/** Set recording mode */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void SetRecordingMode(EMGRecordingMode Mode) { RecordingMode = Mode; }

	/** Set recording frame rate */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void SetRecordingFPS(float FPS);

	// ==========================================
	// DATA ACCESS
	// ==========================================

	/** Set best lap time for this recording */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void SetBestLapTime(float LapTime) { BestLapTime = LapTime; }

	/** Set laps completed */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void SetLapsCompleted(int32 Laps) { LapsCompleted = Laps; }

	/** Set player name */
	UFUNCTION(BlueprintCallable, Category = "Recording")
	void SetPlayerName(const FString& Name) { PlayerName = Name; }

	/** Get recording data without stopping */
	UFUNCTION(BlueprintPure, Category = "Recording")
	FMGReplayData GetCurrentRecordingData() const;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Recording mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EMGRecordingMode RecordingMode = EMGRecordingMode::Manual;

	/** Recording frame rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float RecordingFPS = 30.0f;

	/** Maximum recording duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxRecordingDuration = 600.0f;

	/** Circular buffer size for continuous mode (frames) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 CircularBufferSize = 9000; // 5 minutes at 30fps

	/** Track ID for this recording */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName TrackID;

	/** Vehicle ID for this recording */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName VehicleID;

	/** Player name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString PlayerName;

	// ==========================================
	// STATE
	// ==========================================

	/** Is currently recording */
	bool bIsRecording = false;

	/** Recording accumulator */
	float RecordingAccumulator = 0.0f;

	/** Frame interval */
	float FrameInterval = 1.0f / 30.0f;

	/** Recorded frames */
	TArray<FMGReplayFrame> RecordedFrames;

	/** Circular buffer head (for continuous mode) */
	int32 CircularHead = 0;

	/** Best lap time */
	float BestLapTime = 0.0f;

	/** Laps completed */
	int32 LapsCompleted = 0;

	/** Recording start time */
	float RecordingStartTime = 0.0f;

	/** Replay subsystem reference */
	UPROPERTY()
	UMGReplaySubsystem* ReplaySubsystem = nullptr;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Record a frame */
	void RecordFrame();

	/** Build frame from current state */
	FMGReplayFrame BuildCurrentFrame() const;

	/** Get vehicle input values */
	void GetVehicleInputs(float& OutThrottle, float& OutBrake, float& OutSteering) const;

	/** Get vehicle state */
	void GetVehicleState(float& OutSpeedKPH, float& OutRPM, int32& OutGear, bool& OutDrifting, bool& OutNOS) const;

	/** Get wheel positions */
	TArray<FVector> GetWheelPositions() const;
};

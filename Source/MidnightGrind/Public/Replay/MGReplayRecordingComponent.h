// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGReplayRecordingComponent.h
 * @brief Actor Component for recording vehicle replay data during gameplay
 *
 * @section overview Overview
 * The Replay Recording Component is an Actor Component that captures vehicle
 * state data at regular intervals during gameplay. This recorded data can later
 * be used for replay playback, ghost racing, and performance analysis.
 *
 * The component supports multiple recording modes: manual (explicit start/stop),
 * automatic (starts when race begins), and continuous (always recording with
 * a circular buffer).
 *
 * @section concepts Key Concepts for Beginners
 *
 * **What is an Actor Component?**
 * In Unreal Engine, components are modular pieces of functionality that can be
 * attached to Actors. By making the replay recorder a component, any vehicle
 * Actor can gain recording capability simply by adding this component - no need
 * to modify the vehicle class itself.
 *
 * **Frame-Based Recording**
 * Replay data is captured as discrete "frames" at a fixed rate (e.g., 30 FPS).
 * Each frame stores the vehicle's position, rotation, velocity, inputs, and
 * other state data. Lower frame rates produce smaller files but choppier playback;
 * higher rates give smoother results but larger files.
 *
 * **Recording Modes Explained:**
 * - Manual: You explicitly call StartRecording() and StopRecording(). Best for
 *   controlled scenarios like time trials.
 * - AutoRace: Recording starts automatically when a race begins (detected via
 *   race manager events). Simplest setup for standard races.
 * - Continuous: Always recording into a circular buffer. Useful for "instant replay"
 *   features where you want to capture the last N minutes at any time.
 *
 * **Circular Buffer**
 * In continuous mode, the component uses a circular buffer - when full, new frames
 * overwrite the oldest ones. This keeps memory usage constant while always having
 * recent gameplay available for replay.
 *
 * **Delegates and Events**
 * The component broadcasts events (OnRecordingStarted, OnRecordingStopped) that
 * other systems can listen to. This follows the Observer pattern, allowing loose
 * coupling between the recording system and UI, analytics, etc.
 *
 * @section usage Usage Examples
 *
 * **Adding Component to a Vehicle (Blueprint):**
 * 1. Open your vehicle Blueprint
 * 2. Click "Add Component"
 * 3. Search for "MG Replay Recording"
 * 4. Configure recording mode and FPS in Details panel
 *
 * **Adding Component in C++:**
 * @code
 * // In your vehicle's constructor
 * ReplayRecorder = CreateDefaultSubobject<UMGReplayRecordingComponent>(TEXT("ReplayRecorder"));
 * ReplayRecorder->SetRecordingMode(EMGRecordingMode::AutoRace);
 * ReplayRecorder->SetRecordingFPS(30.0f);
 * @endcode
 *
 * **Manual Recording:**
 * @code
 * // Get the recording component
 * UMGReplayRecordingComponent* Recorder = Vehicle->FindComponentByClass<UMGReplayRecordingComponent>();
 *
 * // Configure for this race
 * Recorder->SetTrackID(FName("Track_Shibuya"));
 * Recorder->SetVehicleID(FName("Vehicle_GT500"));
 * Recorder->SetPlayerName(TEXT("PlayerOne"));
 *
 * // Start recording when race begins
 * Recorder->StartRecording();
 *
 * // ... gameplay happens ...
 *
 * // Stop and get the replay data
 * FMGReplayData ReplayData = Recorder->StopRecording();
 *
 * // Save or use the replay
 * ReplaySubsystem->SaveReplay(ReplayData);
 * @endcode
 *
 * **Listening to Recording Events:**
 * @code
 * // Bind to recording events
 * Recorder->OnRecordingStarted.AddDynamic(this, &AMyClass::HandleRecordingStarted);
 * Recorder->OnRecordingStopped.AddDynamic(this, &AMyClass::HandleRecordingStopped);
 *
 * void AMyClass::HandleRecordingStopped(const FMGReplayData& Data)
 * {
 *     UE_LOG(LogReplay, Log, TEXT("Recorded %d frames over %.2f seconds"),
 *         Data.Frames.Num(), Data.GetDuration());
 * }
 * @endcode
 *
 * **Using Continuous Mode for Instant Replay:**
 * @code
 * // Setup continuous recording (in BeginPlay or similar)
 * Recorder->SetRecordingMode(EMGRecordingMode::Continuous);
 * Recorder->StartRecording();
 *
 * // Later, when player wants to see last 30 seconds
 * FMGReplayData RecentData = Recorder->GetCurrentRecordingData();
 * // RecentData contains up to CircularBufferSize frames
 * @endcode
 *
 * @section bestpractices Best Practices
 * - Use AutoRace mode for standard gameplay to minimize code
 * - Set TrackID and VehicleID before recording for proper replay organization
 * - In Continuous mode, balance buffer size vs memory (9000 frames = ~5min at 30fps)
 * - Always check IsRecording() before calling StopRecording()
 * - Consider calling GetCurrentRecordingData() periodically for auto-save features
 *
 * @see UMGReplaySubsystem
 * @see FMGReplayData
 * @see FMGReplayFrame
 * @see EMGRecordingMode
 */

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
	virtual void TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * @file MGInputBufferSubsystem.h
 * @brief Input Buffering System - Frame-Perfect Input Handling for Racing
 * =============================================================================
 *
 * @section Overview
 * This subsystem provides a professional-grade input buffering system for
 * Midnight Grind. It captures, buffers, and processes player inputs to enable
 * frame-perfect trick execution, combo detection, and timing-based mechanics.
 * Think of it like the input system in fighting games - inputs are stored
 * briefly so they can be executed at the perfect moment.
 *
 * @section WhyBuffer Why Buffer Inputs?
 *
 * WITHOUT BUFFERING:
 * - Player presses drift at frame 59
 * - Game checks for drift input at frame 60
 * - Input missed! Drift doesn't happen. Player frustrated.
 *
 * WITH BUFFERING:
 * - Player presses drift at frame 59
 * - Input stored in buffer with timestamp
 * - Game checks buffer at frame 60
 * - Buffered input found! Drift executes. Player happy.
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * 1. GAME INSTANCE SUBSYSTEM
 *    Inherits from UGameInstanceSubsystem:
 *    - One instance for entire game session
 *    - Persists across level loads
 *    - Access via: GetGameInstance()->GetSubsystem<UMGInputBufferSubsystem>()
 *
 * 2. INPUT ACTIONS (EMGInputAction)
 *    Named actions mapped to physical inputs:
 *    - Movement: Accelerate, Brake, SteerLeft, SteerRight
 *    - Drift: DriftStart, DriftRelease
 *    - Tricks: TrickUp, TrickDown, TrickLeft, TrickRight, TrickSpin, TrickFlip
 *    - Other: Nitro, ShiftUp, ShiftDown, Horn, UseItem
 *
 * 3. INPUT STATES (EMGInputState)
 *    The lifecycle of a button press:
 *    - Pressed: Just pressed this frame
 *    - Held: Still held down
 *    - Released: Just released this frame
 *    - None: Not active
 *
 * 4. BUFFERED INPUTS (FMGBufferedInput)
 *    Each input is stored with metadata:
 *    - Timestamp: When the input occurred
 *    - FrameNumber: Which game frame
 *    - AnalogValue: For analog inputs (trigger pressure, stick position)
 *    - HoldDuration: How long held (for charge attacks)
 *    - bConsumed: Has this input been "used" by game logic?
 *
 * 5. BUFFER WINDOW
 *    How long inputs stay valid (default: 150ms / ~9 frames at 60fps):
 *    - Too short: Inputs feel unresponsive
 *    - Too long: Inputs feel "laggy" or delayed
 *    - BufferWindowSeconds configures this
 *
 * 6. INPUT CONSUMPTION
 *    Consuming an input "uses" it:
 *    - ConsumeBufferedInput() marks input as consumed
 *    - Prevents same input triggering multiple actions
 *    - Example: Drift input shouldn't start two drifts
 *
 * 7. COMBO SYSTEM (FMGInputCombo)
 *    Define complex input sequences:
 *    - Sequence: Inputs in order (A then B then C)
 *    - Simultaneous: Multiple inputs at once (A + B)
 *    - ChargeRelease: Hold then release (charge drift)
 *    - DoubleTap: Same input twice quickly
 *
 * 8. TIMING WINDOWS (FMGTimingWindow)
 *    For rhythm-based mechanics (drift timing, boost timing):
 *    - Perfect: Tightest window, best reward
 *    - Great: Good timing, good reward
 *    - Good: Acceptable timing, small reward
 *    - Missed: Outside all windows, no reward
 *
 * 9. INPUT RECORDING
 *    Record and replay input sequences:
 *    - Ghost/replay systems
 *    - Tutorial playback
 *    - QA testing
 *
 * @section Usage Common Usage Patterns
 *
 * @code
 * // Get the input buffer subsystem
 * UMGInputBufferSubsystem* InputBuffer =
 *     GetGameInstance()->GetSubsystem<UMGInputBufferSubsystem>();
 *
 * // Called from your input handler (PlayerController or EnhancedInput):
 * void AMyPlayerController::OnDriftPressed()
 * {
 *     InputBuffer->BufferInput(EMGInputAction::DriftStart, EMGInputState::Pressed);
 * }
 *
 * void AMyPlayerController::OnDriftReleased()
 * {
 *     InputBuffer->BufferInput(EMGInputAction::DriftRelease, EMGInputState::Pressed);
 * }
 *
 * // In your vehicle's Tick (check for buffered drift input):
 * void AMyVehicle::Tick(float DeltaTime)
 * {
 *     // Call this each frame to update internal state
 *     InputBuffer->ProcessInputFrame(DeltaTime);
 *
 *     // Check if we can start a drift
 *     if (CanStartDrift() &&
 *         InputBuffer->ConsumeBufferedInput(EMGInputAction::DriftStart))
 *     {
 *         StartDrift();
 *     }
 * }
 *
 * // Setting up a combo (e.g., double-tap nitro for mega boost):
 * FMGInputCombo DoubleTapNitro;
 * DoubleTapNitro.ComboName = "MegaBoost";
 * DoubleTapNitro.ComboType = EMGComboType::DoubleTap;
 * DoubleTapNitro.RequiredInputs.Add(EMGInputAction::Nitro);
 * DoubleTapNitro.WindowSeconds = 0.3f;  // Must double-tap within 300ms
 * InputBuffer->RegisterCombo(DoubleTapNitro);
 *
 * // Listen for combo completion
 * InputBuffer->OnComboDetected.AddDynamic(this, &AMyVehicle::HandleCombo);
 *
 * void AMyVehicle::HandleCombo(const FMGComboResult& Result)
 * {
 *     if (Result.ComboName == "MegaBoost" && Result.bSuccess)
 *     {
 *         ActivateMegaBoost();
 *     }
 * }
 *
 * // Setting up a timing window (perfect drift release):
 * InputBuffer->StartTimingWindow(
 *     "DriftRelease",           // Window name
 *     0.5f,                     // Duration in seconds
 *     EMGInputAction::DriftRelease  // Expected action
 * );
 *
 * // Later, check the timing quality:
 * FMGComboResult Result = InputBuffer->EvaluateTimingInput(
 *     "DriftRelease",
 *     EMGInputAction::DriftRelease
 * );
 * if (Result.Timing == EMGInputTiming::Perfect)
 * {
 *     GrantPerfectDriftBonus();
 * }
 *
 * // Check action state (is drift currently held?)
 * if (InputBuffer->IsActionHeld(EMGInputAction::DriftStart))
 * {
 *     float HoldTime = InputBuffer->GetActionHeldDuration(EMGInputAction::DriftStart);
 *     // Longer hold = more drift charge
 * }
 * @endcode
 *
 * @section Architecture Architecture Notes
 *
 * BUFFER MAINTENANCE:
 * - Old inputs automatically cleaned by CleanExpiredBuffers()
 * - Runs on timer, not every frame
 * - MaxBufferedInputs prevents memory growth
 *
 * TIMING PRECISION:
 * - Uses high-resolution timestamps
 * - Frame numbers tracked for frame-perfect detection
 * - LatencyCompensation can offset for display lag
 *
 * CONFIGURATION:
 * - FMGInputBufferConfig holds all tuning parameters
 * - Can be adjusted at runtime for accessibility
 * - Per-action configuration possible via custom logic
 *
 * EVENTS/DELEGATES:
 * - OnInputBuffered: New input received
 * - OnInputConsumed: Input was used
 * - OnComboDetected: Combo completed
 * - OnTimingEvaluated: Timing window checked
 * - OnDoubleTapDetected: Double-tap recognized
 * - OnHoldCompleted: Button held long enough
 *
 * @see UMGHapticsSubsystem - Provides haptic feedback for inputs
 * @see UMGRacingWheelSubsystem - Specialized input for racing wheels
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "MGInputBufferSubsystem.generated.h"

// Input action types for racing
UENUM(BlueprintType)
enum class EMGInputAction : uint8
{
    Accelerate              UMETA(DisplayName = "Accelerate"),
    Brake                   UMETA(DisplayName = "Brake"),
    SteerLeft               UMETA(DisplayName = "Steer Left"),
    SteerRight              UMETA(DisplayName = "Steer Right"),
    DriftStart              UMETA(DisplayName = "Drift Start"),
    DriftRelease            UMETA(DisplayName = "Drift Release"),
    Nitro                   UMETA(DisplayName = "Nitro"),
    TrickUp                 UMETA(DisplayName = "Trick Up"),
    TrickDown               UMETA(DisplayName = "Trick Down"),
    TrickLeft               UMETA(DisplayName = "Trick Left"),
    TrickRight              UMETA(DisplayName = "Trick Right"),
    TrickSpin               UMETA(DisplayName = "Trick Spin"),
    TrickFlip               UMETA(DisplayName = "Trick Flip"),
    ShiftUp                 UMETA(DisplayName = "Shift Up"),
    ShiftDown               UMETA(DisplayName = "Shift Down"),
    Horn                    UMETA(DisplayName = "Horn"),
    LookBack                UMETA(DisplayName = "Look Back"),
    Reset                   UMETA(DisplayName = "Reset Position"),
    Pause                   UMETA(DisplayName = "Pause"),
    UseItem                 UMETA(DisplayName = "Use Item"),
    Custom1                 UMETA(DisplayName = "Custom 1"),
    Custom2                 UMETA(DisplayName = "Custom 2"),
    Custom3                 UMETA(DisplayName = "Custom 3")
};

// Input state types
UENUM(BlueprintType)
enum class EMGInputState : uint8
{
    None                    UMETA(DisplayName = "None"),
    Pressed                 UMETA(DisplayName = "Pressed"),
    Held                    UMETA(DisplayName = "Held"),
    Released                UMETA(DisplayName = "Released")
};

// Combo input types
UENUM(BlueprintType)
enum class EMGComboType : uint8
{
    Sequence                UMETA(DisplayName = "Sequence"),
    Simultaneous            UMETA(DisplayName = "Simultaneous"),
    ChargeRelease           UMETA(DisplayName = "Charge and Release"),
    DoubleTap               UMETA(DisplayName = "Double Tap"),
    TapHold                 UMETA(DisplayName = "Tap then Hold"),
    Direction               UMETA(DisplayName = "Directional")
};

// Input timing precision
UENUM(BlueprintType)
enum class EMGInputTiming : uint8
{
    Perfect                 UMETA(DisplayName = "Perfect"),
    Great                   UMETA(DisplayName = "Great"),
    Good                    UMETA(DisplayName = "Good"),
    Early                   UMETA(DisplayName = "Early"),
    Late                    UMETA(DisplayName = "Late"),
    Missed                  UMETA(DisplayName = "Missed")
};

// Buffered input entry
USTRUCT(BlueprintType)
struct FMGBufferedInput
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid InputId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInputAction Action = EMGInputAction::Accelerate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInputState State = EMGInputState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FrameNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnalogValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D AxisValue = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HoldDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bConsumed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBufferExpired = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DeviceId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKey SourceKey;
};

// Input combo definition
USTRUCT(BlueprintType)
struct FMGInputCombo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ComboName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGComboType ComboType = EMGComboType::Sequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGInputAction> RequiredInputs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGInputState> RequiredStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindowSeconds = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinHoldTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHoldTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bStrictOrder = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowRepeats = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
};

// Combo detection result
USTRUCT(BlueprintType)
struct FMGComboResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ComboName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInputTiming Timing = EMGInputTiming::Missed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimingOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CompletionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChargeTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGBufferedInput> MatchedInputs;
};

// Input timing window
USTRUCT(BlueprintType)
struct FMGTimingWindow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName WindowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PerfectStart = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PerfectEnd = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GreatStart = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GreatEnd = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoodStart = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoodEnd = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInputAction ExpectedAction = EMGInputAction::Accelerate;
};

// Input recording for replays
USTRUCT(BlueprintType)
struct FMGInputRecording
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RecordingId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RecordingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGBufferedInput> RecordedInputs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalFrames = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime RecordedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TrackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VehicleName;
};

// Input statistics
USTRUCT(BlueprintType)
struct FMGInputStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalInputs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BufferedInputs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ConsumedInputs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExpiredInputs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PerfectTimings = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GreatTimings = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoodTimings = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MissedTimings = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CombosExecuted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLatency = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InputRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGInputAction, int32> ActionCounts;
};

// Input buffer configuration
USTRUCT(BlueprintType)
struct FMGInputBufferConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BufferWindowSeconds = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxBufferedInputs = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DoubleTapWindow = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HoldThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnalogDeadzone = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnalogSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PerfectWindowMs = 33.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GreatWindowMs = 66.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoodWindowMs = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableInputRecording = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableComboDetection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnablePredictiveBuffering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompensateForLatency = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LatencyCompensationMs = 0.0f;
};

// Input action state tracking
USTRUCT(BlueprintType)
struct FMGInputActionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInputAction Action = EMGInputAction::Accelerate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInputState CurrentState = EMGInputState::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnalogValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D AxisValue = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StateStartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeldDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TapCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastTapTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWasConsumed = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnInputBuffered, const FMGBufferedInput&, Input, int32, BufferPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnInputConsumed, const FMGBufferedInput&, Input);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnInputExpired, const FMGBufferedInput&, Input);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnComboDetected, const FMGComboResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTimingEvaluated, EMGInputTiming, Timing, float, Offset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnActionStateChanged, EMGInputAction, Action, EMGInputState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnDoubleTapDetected, EMGInputAction, Action, float, IntervalSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnHoldCompleted, EMGInputAction, Action, float, HoldDuration);

/**
 * UMGInputBufferSubsystem
 * Frame-perfect input buffering for precise racing controls
 * Supports combos, timing windows, and input recording
 */
UCLASS()
class MIDNIGHTGRIND_API UMGInputBufferSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core input buffering
    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Core")
    void BufferInput(EMGInputAction Action, EMGInputState State, float AnalogValue = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Core")
    void BufferAxisInput(EMGInputAction Action, FVector2D AxisValue);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Core")
    void BufferRawInput(const FMGBufferedInput& Input);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Core")
    void ProcessInputFrame(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Core")
    void ClearBuffer();

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Core")
    void ClearBufferForAction(EMGInputAction Action);

    // Input consumption
    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Consumption")
    bool ConsumeBufferedInput(EMGInputAction Action, EMGInputState RequiredState = EMGInputState::Pressed);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Consumption")
    bool ConsumeAnyBufferedInput(const TArray<EMGInputAction>& Actions, EMGInputAction& OutConsumedAction);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Consumption")
    FMGBufferedInput PopOldestInput(EMGInputAction Action);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Consumption")
    FMGBufferedInput PopNewestInput(EMGInputAction Action);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Consumption")
    void MarkInputConsumed(const FGuid& InputId);

    // Input queries
    UFUNCTION(BlueprintPure, Category = "InputBuffer|Query")
    bool HasBufferedInput(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Query")
    bool HasBufferedInputWithState(EMGInputAction Action, EMGInputState State) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Query")
    int32 GetBufferedInputCount(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Query")
    TArray<FMGBufferedInput> GetBufferedInputs(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Query")
    TArray<FMGBufferedInput> GetAllBufferedInputs() const { return InputBuffer; }

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Query")
    float GetTimeSinceInput(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Query")
    int32 GetFramesSinceInput(EMGInputAction Action) const;

    // Action state tracking
    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    EMGInputState GetActionState(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    float GetActionAnalogValue(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    FVector2D GetActionAxisValue(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    float GetActionHeldDuration(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    bool IsActionPressed(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    bool IsActionHeld(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    bool WasActionJustPressed(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    bool WasActionJustReleased(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|State")
    FMGInputActionState GetFullActionState(EMGInputAction Action) const;

    // Combo system
    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Combo")
    void RegisterCombo(const FMGInputCombo& Combo);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Combo")
    void UnregisterCombo(const FName& ComboName);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Combo")
    void ClearAllCombos();

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Combo")
    TArray<FMGInputCombo> GetRegisteredCombos() const { return RegisteredCombos; }

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Combo")
    bool IsComboInProgress(const FName& ComboName) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Combo")
    float GetComboProgress(const FName& ComboName) const;

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Combo")
    void ResetComboProgress(const FName& ComboName);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Combo")
    void ResetAllComboProgress();

    // Timing windows
    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Timing")
    void StartTimingWindow(const FName& WindowName, float Duration, EMGInputAction ExpectedAction);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Timing")
    void StartTimingWindowAdvanced(const FMGTimingWindow& Window);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Timing")
    void EndTimingWindow(const FName& WindowName);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Timing")
    void ClearAllTimingWindows();

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Timing")
    bool IsTimingWindowActive(const FName& WindowName) const;

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Timing")
    FMGComboResult EvaluateTimingInput(const FName& WindowName, EMGInputAction Action);

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Timing")
    EMGInputTiming EvaluateTiming(const FName& WindowName, float InputTime) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Timing")
    float GetTimingWindowRemaining(const FName& WindowName) const;

    // Input recording
    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Recording")
    void StartRecording(const FString& RecordingName);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Recording")
    void StopRecording();

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Recording")
    void PauseRecording();

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Recording")
    void ResumeRecording();

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Recording")
    bool IsRecording() const { return bIsRecording; }

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Recording")
    FMGInputRecording GetCurrentRecording() const { return CurrentRecording; }

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Recording")
    void SaveRecording(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Recording")
    bool LoadRecording(const FString& SlotName);

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Recording")
    TArray<FString> GetSavedRecordingNames() const;

    // Input playback
    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Playback")
    void StartPlayback(const FMGInputRecording& Recording);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Playback")
    void StopPlayback();

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Playback")
    void PausePlayback();

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Playback")
    void ResumePlayback();

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Playback")
    void SetPlaybackSpeed(float Speed);

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Playback")
    bool IsPlayingBack() const { return bIsPlayingBack; }

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Playback")
    float GetPlaybackProgress() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Config")
    void ApplyConfig(const FMGInputBufferConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Config")
    void SetBufferWindow(float WindowSeconds);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Config")
    void SetAnalogDeadzone(float Deadzone);

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Config")
    void SetLatencyCompensation(float CompensationMs);

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Config")
    FMGInputBufferConfig GetConfig() const { return BufferConfig; }

    // Statistics
    UFUNCTION(BlueprintPure, Category = "InputBuffer|Stats")
    FMGInputStats GetInputStats() const { return InputStats; }

    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Stats")
    void ResetStats();

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Stats")
    int32 GetCurrentFrame() const { return CurrentFrame; }

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Stats")
    float GetCurrentTime() const { return CurrentTime; }

    // Utility
    UFUNCTION(BlueprintCallable, Category = "InputBuffer|Utility")
    void RegisterDefaultCombos();

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Utility")
    FString GetActionDisplayName(EMGInputAction Action) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Utility")
    FString GetTimingDisplayName(EMGInputTiming Timing) const;

    UFUNCTION(BlueprintPure, Category = "InputBuffer|Utility")
    FLinearColor GetTimingColor(EMGInputTiming Timing) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "InputBuffer|Events")
    FMGOnInputBuffered OnInputBuffered;

    UPROPERTY(BlueprintAssignable, Category = "InputBuffer|Events")
    FMGOnInputConsumed OnInputConsumed;

    UPROPERTY(BlueprintAssignable, Category = "InputBuffer|Events")
    FMGOnInputExpired OnInputExpired;

    UPROPERTY(BlueprintAssignable, Category = "InputBuffer|Events")
    FMGOnComboDetected OnComboDetected;

    UPROPERTY(BlueprintAssignable, Category = "InputBuffer|Events")
    FMGOnTimingEvaluated OnTimingEvaluated;

    UPROPERTY(BlueprintAssignable, Category = "InputBuffer|Events")
    FMGOnActionStateChanged OnActionStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "InputBuffer|Events")
    FMGOnDoubleTapDetected OnDoubleTapDetected;

    UPROPERTY(BlueprintAssignable, Category = "InputBuffer|Events")
    FMGOnHoldCompleted OnHoldCompleted;

protected:
    UPROPERTY()
    TArray<FMGBufferedInput> InputBuffer;

    UPROPERTY()
    TMap<EMGInputAction, FMGInputActionState> ActionStates;

    UPROPERTY()
    TArray<FMGInputCombo> RegisteredCombos;

    UPROPERTY()
    TMap<FName, TArray<FMGBufferedInput>> ComboProgress;

    UPROPERTY()
    TMap<FName, FMGTimingWindow> ActiveTimingWindows;

    UPROPERTY()
    FMGInputBufferConfig BufferConfig;

    UPROPERTY()
    FMGInputStats InputStats;

    UPROPERTY()
    FMGInputRecording CurrentRecording;

    UPROPERTY()
    FMGInputRecording PlaybackRecording;

    UPROPERTY()
    TMap<FString, FMGInputRecording> SavedRecordings;

    UPROPERTY()
    bool bIsRecording = false;

    UPROPERTY()
    bool bIsRecordingPaused = false;

    UPROPERTY()
    bool bIsPlayingBack = false;

    UPROPERTY()
    bool bIsPlaybackPaused = false;

    UPROPERTY()
    float PlaybackSpeed = 1.0f;

    UPROPERTY()
    float PlaybackTime = 0.0f;

    UPROPERTY()
    int32 PlaybackIndex = 0;

    UPROPERTY()
    float CurrentTime = 0.0f;

    UPROPERTY()
    int32 CurrentFrame = 0;

    FTimerHandle BufferMaintenanceHandle;
    FTimerHandle ComboCheckHandle;
    FTimerHandle PlaybackHandle;

    void UpdateActionStates(float DeltaTime);
    void CleanExpiredBuffers();
    void UpdateTimingWindows(float DeltaTime);
    void CheckForCombos();
    void CheckForDoubleTaps(EMGInputAction Action);
    void ProcessPlayback(float DeltaTime);
    bool MatchesCombo(const FMGInputCombo& Combo, const TArray<FMGBufferedInput>& Inputs) const;
    EMGInputTiming CalculateTiming(float InputTime, const FMGTimingWindow& Window) const;
    void RecordInput(const FMGBufferedInput& Input);
    FMGBufferedInput CreateBufferedInput(EMGInputAction Action, EMGInputState State, float AnalogValue);
};

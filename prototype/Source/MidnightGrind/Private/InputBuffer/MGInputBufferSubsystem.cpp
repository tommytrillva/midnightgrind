// MGInputBufferSubsystem.cpp
// Midnight Grind - Input Buffering and Precision Input System

#include "InputBuffer/MGInputBufferSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Guid.h"

void UMGInputBufferSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize default configuration
    BufferConfig.BufferWindowSeconds = 0.15f;
    BufferConfig.MaxBufferedInputs = 32;
    BufferConfig.DoubleTapWindow = 0.25f;
    BufferConfig.HoldThreshold = 0.2f;
    BufferConfig.AnalogDeadzone = 0.2f;
    BufferConfig.AnalogSensitivity = 1.0f;
    BufferConfig.PerfectWindowMs = 33.0f;
    BufferConfig.GreatWindowMs = 66.0f;
    BufferConfig.GoodWindowMs = 100.0f;
    BufferConfig.bEnableInputRecording = false;
    BufferConfig.bEnableComboDetection = true;
    BufferConfig.bEnablePredictiveBuffering = true;
    BufferConfig.bCompensateForLatency = true;
    BufferConfig.LatencyCompensationMs = 0.0f;

    // Initialize action states for all actions
    for (int32 i = 0; i <= static_cast<int32>(EMGInputAction::Custom3); ++i)
    {
        EMGInputAction Action = static_cast<EMGInputAction>(i);
        FMGInputActionState State;
        State.Action = Action;
        State.CurrentState = EMGInputState::None;
        ActionStates.Add(Action, State);
    }

    // Register default combos
    RegisterDefaultCombos();

    // Start maintenance timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGInputBufferSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            BufferMaintenanceHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->CleanExpiredBuffers();
                }
            },
            0.05f,
            true
        );

        World->GetTimerManager().SetTimer(
            ComboCheckHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid() && WeakThis->BufferConfig.bEnableComboDetection)
                {
                    WeakThis->CheckForCombos();
                }
            },
            0.016f,
            true
        );
    }
}

void UMGInputBufferSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BufferMaintenanceHandle);
        World->GetTimerManager().ClearTimer(ComboCheckHandle);
        World->GetTimerManager().ClearTimer(PlaybackHandle);
    }

    ClearBuffer();
    ClearAllCombos();
    ClearAllTimingWindows();

    Super::Deinitialize();
}

void UMGInputBufferSubsystem::RegisterDefaultCombos()
{
    // Perfect Drift Start combo: Brake + Steer in quick succession
    FMGInputCombo DriftStartCombo;
    DriftStartCombo.ComboName = FName("PerfectDriftStart");
    DriftStartCombo.ComboType = EMGComboType::Simultaneous;
    DriftStartCombo.RequiredInputs.Add(EMGInputAction::Brake);
    DriftStartCombo.RequiredInputs.Add(EMGInputAction::SteerLeft);
    DriftStartCombo.RequiredStates.Add(EMGInputState::Pressed);
    DriftStartCombo.RequiredStates.Add(EMGInputState::Pressed);
    DriftStartCombo.WindowSeconds = 0.1f;
    DriftStartCombo.bStrictOrder = false;
    DriftStartCombo.Priority = 10;
    DriftStartCombo.Description = TEXT("Perfect drift initiation");
    RegisterCombo(DriftStartCombo);

    // Drift Boost: Hold drift, release at right moment
    FMGInputCombo DriftBoostCombo;
    DriftBoostCombo.ComboName = FName("DriftBoost");
    DriftBoostCombo.ComboType = EMGComboType::ChargeRelease;
    DriftBoostCombo.RequiredInputs.Add(EMGInputAction::DriftStart);
    DriftBoostCombo.RequiredInputs.Add(EMGInputAction::DriftRelease);
    DriftBoostCombo.RequiredStates.Add(EMGInputState::Held);
    DriftBoostCombo.RequiredStates.Add(EMGInputState::Released);
    DriftBoostCombo.MinHoldTime = 0.5f;
    DriftBoostCombo.MaxHoldTime = 3.0f;
    DriftBoostCombo.Priority = 15;
    DriftBoostCombo.Description = TEXT("Drift boost on release");
    RegisterCombo(DriftBoostCombo);

    // Trick Combo: Sequence of trick inputs
    FMGInputCombo TrickCombo;
    TrickCombo.ComboName = FName("TrickSequence");
    TrickCombo.ComboType = EMGComboType::Sequence;
    TrickCombo.RequiredInputs.Add(EMGInputAction::TrickUp);
    TrickCombo.RequiredInputs.Add(EMGInputAction::TrickSpin);
    TrickCombo.RequiredInputs.Add(EMGInputAction::TrickFlip);
    TrickCombo.RequiredStates.Add(EMGInputState::Pressed);
    TrickCombo.RequiredStates.Add(EMGInputState::Pressed);
    TrickCombo.RequiredStates.Add(EMGInputState::Pressed);
    TrickCombo.WindowSeconds = 0.8f;
    TrickCombo.bStrictOrder = true;
    TrickCombo.Priority = 20;
    TrickCombo.Description = TEXT("Multi-trick combo");
    RegisterCombo(TrickCombo);

    // Quick Nitro: Double tap accelerate
    FMGInputCombo QuickNitroCombo;
    QuickNitroCombo.ComboName = FName("QuickNitro");
    QuickNitroCombo.ComboType = EMGComboType::DoubleTap;
    QuickNitroCombo.RequiredInputs.Add(EMGInputAction::Nitro);
    QuickNitroCombo.RequiredStates.Add(EMGInputState::Pressed);
    QuickNitroCombo.WindowSeconds = 0.3f;
    QuickNitroCombo.bAllowRepeats = true;
    QuickNitroCombo.Priority = 5;
    QuickNitroCombo.Description = TEXT("Quick nitro activation");
    RegisterCombo(QuickNitroCombo);

    // Burnout Start: Hold brake, tap accelerate
    FMGInputCombo BurnoutCombo;
    BurnoutCombo.ComboName = FName("Burnout");
    BurnoutCombo.ComboType = EMGComboType::TapHold;
    BurnoutCombo.RequiredInputs.Add(EMGInputAction::Brake);
    BurnoutCombo.RequiredInputs.Add(EMGInputAction::Accelerate);
    BurnoutCombo.RequiredStates.Add(EMGInputState::Held);
    BurnoutCombo.RequiredStates.Add(EMGInputState::Pressed);
    BurnoutCombo.MinHoldTime = 0.3f;
    BurnoutCombo.Priority = 8;
    BurnoutCombo.Description = TEXT("Burnout launch");
    RegisterCombo(BurnoutCombo);
}

void UMGInputBufferSubsystem::BufferInput(EMGInputAction Action, EMGInputState State, float AnalogValue)
{
    FMGBufferedInput Input = CreateBufferedInput(Action, State, AnalogValue);
    BufferRawInput(Input);
}

void UMGInputBufferSubsystem::BufferAxisInput(EMGInputAction Action, FVector2D AxisValue)
{
    // Apply deadzone
    float Magnitude = AxisValue.Size();
    if (Magnitude < BufferConfig.AnalogDeadzone)
    {
        return;
    }

    FMGBufferedInput Input = CreateBufferedInput(Action, EMGInputState::Held, Magnitude);
    Input.AxisValue = AxisValue * BufferConfig.AnalogSensitivity;
    BufferRawInput(Input);
}

void UMGInputBufferSubsystem::BufferRawInput(const FMGBufferedInput& Input)
{
    // Add to buffer
    InputBuffer.Add(Input);
    InputStats.TotalInputs++;
    InputStats.BufferedInputs++;

    // Track action counts
    int32& ActionCount = InputStats.ActionCounts.FindOrAdd(Input.Action);
    ActionCount++;

    // Trim buffer if too large
    while (InputBuffer.Num() > BufferConfig.MaxBufferedInputs)
    {
        InputBuffer.RemoveAt(0);
        InputStats.BufferedInputs--;
    }

    // Update action state
    if (FMGInputActionState* ActionState = ActionStates.Find(Input.Action))
    {
        EMGInputState OldState = ActionState->CurrentState;
        ActionState->CurrentState = Input.State;
        ActionState->AnalogValue = Input.AnalogValue;
        ActionState->AxisValue = Input.AxisValue;

        if (Input.State == EMGInputState::Pressed)
        {
            ActionState->StateStartTime = CurrentTime;
            ActionState->TapCount++;
            ActionState->LastTapTime = CurrentTime;
            CheckForDoubleTaps(Input.Action);
        }
        else if (Input.State == EMGInputState::Released)
        {
            ActionState->HeldDuration = CurrentTime - ActionState->StateStartTime;
        }

        if (OldState != Input.State)
        {
            OnActionStateChanged.Broadcast(Input.Action, Input.State);
        }
    }

    // Record if recording
    if (bIsRecording && !bIsRecordingPaused)
    {
        RecordInput(Input);
    }

    OnInputBuffered.Broadcast(Input, InputBuffer.Num() - 1);
}

FMGBufferedInput UMGInputBufferSubsystem::CreateBufferedInput(EMGInputAction Action, EMGInputState State, float AnalogValue)
{
    FMGBufferedInput Input;
    Input.InputId = FGuid::NewGuid();
    Input.Action = Action;
    Input.State = State;
    Input.Timestamp = CurrentTime;
    Input.FrameNumber = CurrentFrame;
    Input.AnalogValue = AnalogValue;
    Input.bConsumed = false;
    Input.bBufferExpired = false;

    // Apply latency compensation
    if (BufferConfig.bCompensateForLatency && BufferConfig.LatencyCompensationMs > 0.0f)
    {
        Input.Timestamp -= BufferConfig.LatencyCompensationMs / 1000.0f;
    }

    return Input;
}

void UMGInputBufferSubsystem::ProcessInputFrame(float DeltaTime)
{
    CurrentTime += DeltaTime;
    CurrentFrame++;

    UpdateActionStates(DeltaTime);
    UpdateTimingWindows(DeltaTime);

    if (bIsPlayingBack && !bIsPlaybackPaused)
    {
        ProcessPlayback(DeltaTime);
    }

    // Update input rate stat
    static float RateSampleTime = 0.0f;
    static int32 RateSampleCount = 0;
    RateSampleTime += DeltaTime;
    RateSampleCount++;
    if (RateSampleTime >= 1.0f)
    {
        InputStats.InputRate = static_cast<float>(RateSampleCount) / RateSampleTime;
        RateSampleTime = 0.0f;
        RateSampleCount = 0;
    }
}

void UMGInputBufferSubsystem::ClearBuffer()
{
    for (const FMGBufferedInput& Input : InputBuffer)
    {
        if (!Input.bConsumed)
        {
            OnInputExpired.Broadcast(Input);
            InputStats.ExpiredInputs++;
        }
    }

    InputBuffer.Empty();
    InputStats.BufferedInputs = 0;
}

void UMGInputBufferSubsystem::ClearBufferForAction(EMGInputAction Action)
{
    InputBuffer.RemoveAll([Action, this](const FMGBufferedInput& Input)
    {
        if (Input.Action == Action && !Input.bConsumed)
        {
            OnInputExpired.Broadcast(Input);
            InputStats.ExpiredInputs++;
            return true;
        }
        return false;
    });
}

bool UMGInputBufferSubsystem::ConsumeBufferedInput(EMGInputAction Action, EMGInputState RequiredState)
{
    for (int32 i = 0; i < InputBuffer.Num(); ++i)
    {
        FMGBufferedInput& Input = InputBuffer[i];

        if (Input.Action == Action &&
            (RequiredState == EMGInputState::None || Input.State == RequiredState) &&
            !Input.bConsumed &&
            !Input.bBufferExpired)
        {
            Input.bConsumed = true;
            InputStats.ConsumedInputs++;

            OnInputConsumed.Broadcast(Input);

            // Mark action state as consumed
            if (FMGInputActionState* ActionState = ActionStates.Find(Action))
            {
                ActionState->bWasConsumed = true;
            }

            return true;
        }
    }

    return false;
}

bool UMGInputBufferSubsystem::ConsumeAnyBufferedInput(const TArray<EMGInputAction>& Actions, EMGInputAction& OutConsumedAction)
{
    for (EMGInputAction Action : Actions)
    {
        if (ConsumeBufferedInput(Action))
        {
            OutConsumedAction = Action;
            return true;
        }
    }

    return false;
}

FMGBufferedInput UMGInputBufferSubsystem::PopOldestInput(EMGInputAction Action)
{
    for (int32 i = 0; i < InputBuffer.Num(); ++i)
    {
        if (InputBuffer[i].Action == Action && !InputBuffer[i].bConsumed && !InputBuffer[i].bBufferExpired)
        {
            FMGBufferedInput Input = InputBuffer[i];
            Input.bConsumed = true;
            InputBuffer[i].bConsumed = true;
            InputStats.ConsumedInputs++;
            OnInputConsumed.Broadcast(Input);
            return Input;
        }
    }

    return FMGBufferedInput();
}

FMGBufferedInput UMGInputBufferSubsystem::PopNewestInput(EMGInputAction Action)
{
    for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
    {
        if (InputBuffer[i].Action == Action && !InputBuffer[i].bConsumed && !InputBuffer[i].bBufferExpired)
        {
            FMGBufferedInput Input = InputBuffer[i];
            Input.bConsumed = true;
            InputBuffer[i].bConsumed = true;
            InputStats.ConsumedInputs++;
            OnInputConsumed.Broadcast(Input);
            return Input;
        }
    }

    return FMGBufferedInput();
}

void UMGInputBufferSubsystem::MarkInputConsumed(const FGuid& InputId)
{
    for (FMGBufferedInput& Input : InputBuffer)
    {
        if (Input.InputId == InputId)
        {
            Input.bConsumed = true;
            InputStats.ConsumedInputs++;
            OnInputConsumed.Broadcast(Input);
            return;
        }
    }
}

bool UMGInputBufferSubsystem::HasBufferedInput(EMGInputAction Action) const
{
    for (const FMGBufferedInput& Input : InputBuffer)
    {
        if (Input.Action == Action && !Input.bConsumed && !Input.bBufferExpired)
        {
            return true;
        }
    }
    return false;
}

bool UMGInputBufferSubsystem::HasBufferedInputWithState(EMGInputAction Action, EMGInputState State) const
{
    for (const FMGBufferedInput& Input : InputBuffer)
    {
        if (Input.Action == Action && Input.State == State && !Input.bConsumed && !Input.bBufferExpired)
        {
            return true;
        }
    }
    return false;
}

int32 UMGInputBufferSubsystem::GetBufferedInputCount(EMGInputAction Action) const
{
    int32 Count = 0;
    for (const FMGBufferedInput& Input : InputBuffer)
    {
        if (Input.Action == Action && !Input.bConsumed && !Input.bBufferExpired)
        {
            Count++;
        }
    }
    return Count;
}

TArray<FMGBufferedInput> UMGInputBufferSubsystem::GetBufferedInputs(EMGInputAction Action) const
{
    TArray<FMGBufferedInput> Result;
    for (const FMGBufferedInput& Input : InputBuffer)
    {
        if (Input.Action == Action && !Input.bConsumed && !Input.bBufferExpired)
        {
            Result.Add(Input);
        }
    }
    return Result;
}

float UMGInputBufferSubsystem::GetTimeSinceInput(EMGInputAction Action) const
{
    for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
    {
        if (InputBuffer[i].Action == Action)
        {
            return CurrentTime - InputBuffer[i].Timestamp;
        }
    }
    return TNumericLimits<float>::Max();
}

int32 UMGInputBufferSubsystem::GetFramesSinceInput(EMGInputAction Action) const
{
    for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
    {
        if (InputBuffer[i].Action == Action)
        {
            return CurrentFrame - InputBuffer[i].FrameNumber;
        }
    }
    return TNumericLimits<int32>::Max();
}

EMGInputState UMGInputBufferSubsystem::GetActionState(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        return State->CurrentState;
    }
    return EMGInputState::None;
}

float UMGInputBufferSubsystem::GetActionAnalogValue(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        return State->AnalogValue;
    }
    return 0.0f;
}

FVector2D UMGInputBufferSubsystem::GetActionAxisValue(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        return State->AxisValue;
    }
    return FVector2D::ZeroVector;
}

float UMGInputBufferSubsystem::GetActionHeldDuration(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        if (State->CurrentState == EMGInputState::Held || State->CurrentState == EMGInputState::Pressed)
        {
            return CurrentTime - State->StateStartTime;
        }
        return State->HeldDuration;
    }
    return 0.0f;
}

bool UMGInputBufferSubsystem::IsActionPressed(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        return State->CurrentState == EMGInputState::Pressed || State->CurrentState == EMGInputState::Held;
    }
    return false;
}

bool UMGInputBufferSubsystem::IsActionHeld(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        return State->CurrentState == EMGInputState::Held;
    }
    return false;
}

bool UMGInputBufferSubsystem::WasActionJustPressed(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        return State->CurrentState == EMGInputState::Pressed &&
               (CurrentTime - State->StateStartTime) < 0.033f; // Within 1-2 frames
    }
    return false;
}

bool UMGInputBufferSubsystem::WasActionJustReleased(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        return State->CurrentState == EMGInputState::Released &&
               (CurrentTime - State->StateStartTime) < 0.033f;
    }
    return false;
}

FMGInputActionState UMGInputBufferSubsystem::GetFullActionState(EMGInputAction Action) const
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        return *State;
    }
    return FMGInputActionState();
}

void UMGInputBufferSubsystem::RegisterCombo(const FMGInputCombo& Combo)
{
    // Check if combo with same name exists
    for (FMGInputCombo& Existing : RegisteredCombos)
    {
        if (Existing.ComboName == Combo.ComboName)
        {
            Existing = Combo;
            return;
        }
    }

    RegisteredCombos.Add(Combo);

    // Sort by priority (higher priority first)
    RegisteredCombos.Sort([](const FMGInputCombo& A, const FMGInputCombo& B)
    {
        return A.Priority > B.Priority;
    });
}

void UMGInputBufferSubsystem::UnregisterCombo(const FName& ComboName)
{
    RegisteredCombos.RemoveAll([&ComboName](const FMGInputCombo& Combo)
    {
        return Combo.ComboName == ComboName;
    });

    ComboProgress.Remove(ComboName);
}

void UMGInputBufferSubsystem::ClearAllCombos()
{
    RegisteredCombos.Empty();
    ComboProgress.Empty();
}

bool UMGInputBufferSubsystem::IsComboInProgress(const FName& ComboName) const
{
    if (const TArray<FMGBufferedInput>* Progress = ComboProgress.Find(ComboName))
    {
        return Progress->Num() > 0;
    }
    return false;
}

float UMGInputBufferSubsystem::GetComboProgress(const FName& ComboName) const
{
    const FMGInputCombo* Combo = nullptr;
    for (const FMGInputCombo& C : RegisteredCombos)
    {
        if (C.ComboName == ComboName)
        {
            Combo = &C;
            break;
        }
    }

    if (!Combo || Combo->RequiredInputs.Num() == 0)
    {
        return 0.0f;
    }

    if (const TArray<FMGBufferedInput>* Progress = ComboProgress.Find(ComboName))
    {
        return static_cast<float>(Progress->Num()) / static_cast<float>(Combo->RequiredInputs.Num());
    }

    return 0.0f;
}

void UMGInputBufferSubsystem::ResetComboProgress(const FName& ComboName)
{
    ComboProgress.Remove(ComboName);
}

void UMGInputBufferSubsystem::ResetAllComboProgress()
{
    ComboProgress.Empty();
}

void UMGInputBufferSubsystem::StartTimingWindow(const FName& WindowName, float Duration, EMGInputAction ExpectedAction)
{
    FMGTimingWindow Window;
    Window.WindowName = WindowName;
    Window.StartTime = CurrentTime;
    Window.EndTime = CurrentTime + Duration;
    Window.ExpectedAction = ExpectedAction;
    Window.bActive = true;

    // Set up timing zones (proportional)
    float PerfectRatio = BufferConfig.PerfectWindowMs / (Duration * 1000.0f);
    float GreatRatio = BufferConfig.GreatWindowMs / (Duration * 1000.0f);
    float GoodRatio = BufferConfig.GoodWindowMs / (Duration * 1000.0f);

    float Center = CurrentTime + (Duration * 0.5f);

    Window.PerfectStart = Center - (Duration * PerfectRatio * 0.5f);
    Window.PerfectEnd = Center + (Duration * PerfectRatio * 0.5f);
    Window.GreatStart = Center - (Duration * GreatRatio * 0.5f);
    Window.GreatEnd = Center + (Duration * GreatRatio * 0.5f);
    Window.GoodStart = Center - (Duration * GoodRatio * 0.5f);
    Window.GoodEnd = Center + (Duration * GoodRatio * 0.5f);

    ActiveTimingWindows.Add(WindowName, Window);
}

void UMGInputBufferSubsystem::StartTimingWindowAdvanced(const FMGTimingWindow& Window)
{
    FMGTimingWindow NewWindow = Window;
    NewWindow.bActive = true;
    ActiveTimingWindows.Add(Window.WindowName, NewWindow);
}

void UMGInputBufferSubsystem::EndTimingWindow(const FName& WindowName)
{
    ActiveTimingWindows.Remove(WindowName);
}

void UMGInputBufferSubsystem::ClearAllTimingWindows()
{
    ActiveTimingWindows.Empty();
}

bool UMGInputBufferSubsystem::IsTimingWindowActive(const FName& WindowName) const
{
    if (const FMGTimingWindow* Window = ActiveTimingWindows.Find(WindowName))
    {
        return Window->bActive && CurrentTime <= Window->EndTime;
    }
    return false;
}

FMGComboResult UMGInputBufferSubsystem::EvaluateTimingInput(const FName& WindowName, EMGInputAction Action)
{
    FMGComboResult Result;
    Result.bSuccess = false;

    if (const FMGTimingWindow* Window = ActiveTimingWindows.Find(WindowName))
    {
        if (Window->bActive && Action == Window->ExpectedAction)
        {
            Result.Timing = CalculateTiming(CurrentTime, *Window);
            Result.TimingOffset = CurrentTime - ((Window->PerfectStart + Window->PerfectEnd) * 0.5f);
            Result.CompletionTime = CurrentTime;
            Result.bSuccess = Result.Timing != EMGInputTiming::Missed;

            // Update stats
            switch (Result.Timing)
            {
            case EMGInputTiming::Perfect:
                InputStats.PerfectTimings++;
                break;
            case EMGInputTiming::Great:
                InputStats.GreatTimings++;
                break;
            case EMGInputTiming::Good:
                InputStats.GoodTimings++;
                break;
            default:
                InputStats.MissedTimings++;
                break;
            }

            OnTimingEvaluated.Broadcast(Result.Timing, Result.TimingOffset);
        }
    }

    return Result;
}

EMGInputTiming UMGInputBufferSubsystem::EvaluateTiming(const FName& WindowName, float InputTime) const
{
    if (const FMGTimingWindow* Window = ActiveTimingWindows.Find(WindowName))
    {
        return CalculateTiming(InputTime, *Window);
    }
    return EMGInputTiming::Missed;
}

float UMGInputBufferSubsystem::GetTimingWindowRemaining(const FName& WindowName) const
{
    if (const FMGTimingWindow* Window = ActiveTimingWindows.Find(WindowName))
    {
        return FMath::Max(0.0f, Window->EndTime - CurrentTime);
    }
    return 0.0f;
}

EMGInputTiming UMGInputBufferSubsystem::CalculateTiming(float InputTime, const FMGTimingWindow& Window) const
{
    if (InputTime < Window.StartTime || InputTime > Window.EndTime)
    {
        return EMGInputTiming::Missed;
    }

    if (InputTime >= Window.PerfectStart && InputTime <= Window.PerfectEnd)
    {
        return EMGInputTiming::Perfect;
    }

    if (InputTime >= Window.GreatStart && InputTime <= Window.GreatEnd)
    {
        return EMGInputTiming::Great;
    }

    if (InputTime >= Window.GoodStart && InputTime <= Window.GoodEnd)
    {
        return EMGInputTiming::Good;
    }

    float Center = (Window.PerfectStart + Window.PerfectEnd) * 0.5f;
    if (InputTime < Center)
    {
        return EMGInputTiming::Early;
    }
    else
    {
        return EMGInputTiming::Late;
    }
}

void UMGInputBufferSubsystem::StartRecording(const FString& RecordingName)
{
    if (bIsRecording)
    {
        StopRecording();
    }

    CurrentRecording = FMGInputRecording();
    CurrentRecording.RecordingId = FGuid::NewGuid();
    CurrentRecording.RecordingName = RecordingName;
    CurrentRecording.RecordedAt = FDateTime::Now();
    CurrentRecording.RecordedInputs.Empty();

    bIsRecording = true;
    bIsRecordingPaused = false;
}

void UMGInputBufferSubsystem::StopRecording()
{
    if (!bIsRecording)
    {
        return;
    }

    bIsRecording = false;
    bIsRecordingPaused = false;

    if (CurrentRecording.RecordedInputs.Num() > 0)
    {
        CurrentRecording.TotalDuration = CurrentRecording.RecordedInputs.Last().Timestamp -
                                         CurrentRecording.RecordedInputs[0].Timestamp;
        CurrentRecording.TotalFrames = CurrentRecording.RecordedInputs.Last().FrameNumber -
                                       CurrentRecording.RecordedInputs[0].FrameNumber;
    }
}

void UMGInputBufferSubsystem::PauseRecording()
{
    bIsRecordingPaused = true;
}

void UMGInputBufferSubsystem::ResumeRecording()
{
    bIsRecordingPaused = false;
}

void UMGInputBufferSubsystem::SaveRecording(const FString& SlotName)
{
    SavedRecordings.Add(SlotName, CurrentRecording);
}

bool UMGInputBufferSubsystem::LoadRecording(const FString& SlotName)
{
    if (const FMGInputRecording* Recording = SavedRecordings.Find(SlotName))
    {
        CurrentRecording = *Recording;
        return true;
    }
    return false;
}

TArray<FString> UMGInputBufferSubsystem::GetSavedRecordingNames() const
{
    TArray<FString> Names;
    SavedRecordings.GetKeys(Names);
    return Names;
}

void UMGInputBufferSubsystem::StartPlayback(const FMGInputRecording& Recording)
{
    if (bIsPlayingBack)
    {
        StopPlayback();
    }

    PlaybackRecording = Recording;
    PlaybackTime = 0.0f;
    PlaybackIndex = 0;
    bIsPlayingBack = true;
    bIsPlaybackPaused = false;
}

void UMGInputBufferSubsystem::StopPlayback()
{
    bIsPlayingBack = false;
    bIsPlaybackPaused = false;
    PlaybackTime = 0.0f;
    PlaybackIndex = 0;
}

void UMGInputBufferSubsystem::PausePlayback()
{
    bIsPlaybackPaused = true;
}

void UMGInputBufferSubsystem::ResumePlayback()
{
    bIsPlaybackPaused = false;
}

void UMGInputBufferSubsystem::SetPlaybackSpeed(float Speed)
{
    PlaybackSpeed = FMath::Clamp(Speed, 0.1f, 4.0f);
}

float UMGInputBufferSubsystem::GetPlaybackProgress() const
{
    if (!bIsPlayingBack || PlaybackRecording.TotalDuration <= 0.0f)
    {
        return 0.0f;
    }

    return FMath::Clamp(PlaybackTime / PlaybackRecording.TotalDuration, 0.0f, 1.0f);
}

void UMGInputBufferSubsystem::ApplyConfig(const FMGInputBufferConfig& Config)
{
    BufferConfig = Config;
}

void UMGInputBufferSubsystem::SetBufferWindow(float WindowSeconds)
{
    BufferConfig.BufferWindowSeconds = FMath::Max(0.016f, WindowSeconds);
}

void UMGInputBufferSubsystem::SetAnalogDeadzone(float Deadzone)
{
    BufferConfig.AnalogDeadzone = FMath::Clamp(Deadzone, 0.0f, 0.9f);
}

void UMGInputBufferSubsystem::SetLatencyCompensation(float CompensationMs)
{
    BufferConfig.LatencyCompensationMs = FMath::Max(0.0f, CompensationMs);
}

void UMGInputBufferSubsystem::ResetStats()
{
    InputStats = FMGInputStats();
}

FString UMGInputBufferSubsystem::GetActionDisplayName(EMGInputAction Action) const
{
    switch (Action)
    {
    case EMGInputAction::Accelerate: return TEXT("Accelerate");
    case EMGInputAction::Brake: return TEXT("Brake");
    case EMGInputAction::SteerLeft: return TEXT("Steer Left");
    case EMGInputAction::SteerRight: return TEXT("Steer Right");
    case EMGInputAction::DriftStart: return TEXT("Drift");
    case EMGInputAction::DriftRelease: return TEXT("Drift Release");
    case EMGInputAction::Nitro: return TEXT("Nitro");
    case EMGInputAction::TrickUp: return TEXT("Trick Up");
    case EMGInputAction::TrickDown: return TEXT("Trick Down");
    case EMGInputAction::TrickLeft: return TEXT("Trick Left");
    case EMGInputAction::TrickRight: return TEXT("Trick Right");
    case EMGInputAction::TrickSpin: return TEXT("Spin");
    case EMGInputAction::TrickFlip: return TEXT("Flip");
    case EMGInputAction::ShiftUp: return TEXT("Shift Up");
    case EMGInputAction::ShiftDown: return TEXT("Shift Down");
    case EMGInputAction::Horn: return TEXT("Horn");
    case EMGInputAction::LookBack: return TEXT("Look Back");
    case EMGInputAction::Reset: return TEXT("Reset");
    case EMGInputAction::Pause: return TEXT("Pause");
    case EMGInputAction::UseItem: return TEXT("Use Item");
    default: return TEXT("Unknown");
    }
}

FString UMGInputBufferSubsystem::GetTimingDisplayName(EMGInputTiming Timing) const
{
    switch (Timing)
    {
    case EMGInputTiming::Perfect: return TEXT("PERFECT!");
    case EMGInputTiming::Great: return TEXT("GREAT!");
    case EMGInputTiming::Good: return TEXT("GOOD");
    case EMGInputTiming::Early: return TEXT("EARLY");
    case EMGInputTiming::Late: return TEXT("LATE");
    case EMGInputTiming::Missed: return TEXT("MISS");
    default: return TEXT("");
    }
}

FLinearColor UMGInputBufferSubsystem::GetTimingColor(EMGInputTiming Timing) const
{
    switch (Timing)
    {
    case EMGInputTiming::Perfect:
        return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
    case EMGInputTiming::Great:
        return FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Cyan-Green
    case EMGInputTiming::Good:
        return FLinearColor(0.0f, 0.8f, 0.0f, 1.0f); // Green
    case EMGInputTiming::Early:
        return FLinearColor(0.3f, 0.5f, 1.0f, 1.0f); // Blue
    case EMGInputTiming::Late:
        return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
    case EMGInputTiming::Missed:
        return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
    default:
        return FLinearColor::White;
    }
}

void UMGInputBufferSubsystem::UpdateActionStates(float DeltaTime)
{
    for (auto& Pair : ActionStates)
    {
        FMGInputActionState& State = Pair.Value;

        // Update held state if pressed long enough
        if (State.CurrentState == EMGInputState::Pressed)
        {
            float HeldTime = CurrentTime - State.StateStartTime;
            if (HeldTime >= BufferConfig.HoldThreshold)
            {
                State.CurrentState = EMGInputState::Held;
                OnActionStateChanged.Broadcast(State.Action, EMGInputState::Held);
            }
        }

        // Check for hold completion
        if (State.CurrentState == EMGInputState::Held)
        {
            State.HeldDuration = CurrentTime - State.StateStartTime;
        }

        // Reset consumed flag each frame
        State.bWasConsumed = false;
    }
}

void UMGInputBufferSubsystem::CleanExpiredBuffers()
{
    TArray<int32> ToRemove;

    for (int32 i = 0; i < InputBuffer.Num(); ++i)
    {
        FMGBufferedInput& Input = InputBuffer[i];

        if (!Input.bConsumed && !Input.bBufferExpired)
        {
            float Age = CurrentTime - Input.Timestamp;
            if (Age > BufferConfig.BufferWindowSeconds)
            {
                Input.bBufferExpired = true;
                OnInputExpired.Broadcast(Input);
                InputStats.ExpiredInputs++;
                ToRemove.Add(i);
            }
        }
        else if (Input.bConsumed || Input.bBufferExpired)
        {
            // Clean up old consumed/expired inputs after a grace period
            float Age = CurrentTime - Input.Timestamp;
            if (Age > BufferConfig.BufferWindowSeconds * 3.0f)
            {
                ToRemove.Add(i);
            }
        }
    }

    // Remove in reverse order to maintain indices
    for (int32 i = ToRemove.Num() - 1; i >= 0; --i)
    {
        InputBuffer.RemoveAt(ToRemove[i]);
    }

    InputStats.BufferedInputs = InputBuffer.Num();
}

void UMGInputBufferSubsystem::UpdateTimingWindows(float DeltaTime)
{
    TArray<FName> ToRemove;

    for (auto& Pair : ActiveTimingWindows)
    {
        FMGTimingWindow& Window = Pair.Value;

        if (Window.bActive && CurrentTime > Window.EndTime)
        {
            Window.bActive = false;
            ToRemove.Add(Pair.Key);
        }
    }

    for (const FName& Name : ToRemove)
    {
        ActiveTimingWindows.Remove(Name);
    }
}

void UMGInputBufferSubsystem::CheckForCombos()
{
    for (const FMGInputCombo& Combo : RegisteredCombos)
    {
        TArray<FMGBufferedInput> RecentInputs;

        // Gather recent inputs within combo window
        for (const FMGBufferedInput& Input : InputBuffer)
        {
            float Age = CurrentTime - Input.Timestamp;
            if (Age <= Combo.WindowSeconds && !Input.bConsumed && !Input.bBufferExpired)
            {
                RecentInputs.Add(Input);
            }
        }

        if (MatchesCombo(Combo, RecentInputs))
        {
            FMGComboResult Result;
            Result.ComboName = Combo.ComboName;
            Result.bSuccess = true;
            Result.MatchedInputs = RecentInputs;
            Result.CompletionTime = CurrentTime;

            // Calculate timing for the combo
            if (RecentInputs.Num() > 0)
            {
                Result.Timing = EMGInputTiming::Good; // Default

                // Check charge time for charge-release combos
                if (Combo.ComboType == EMGComboType::ChargeRelease && RecentInputs.Num() >= 2)
                {
                    Result.ChargeTime = RecentInputs.Last().Timestamp - RecentInputs[0].Timestamp;

                    if (Result.ChargeTime >= Combo.MinHoldTime)
                    {
                        float ChargeRatio = FMath::Clamp(Result.ChargeTime / Combo.MaxHoldTime, 0.0f, 1.0f);
                        if (ChargeRatio > 0.9f)
                        {
                            Result.Timing = EMGInputTiming::Perfect;
                        }
                        else if (ChargeRatio > 0.7f)
                        {
                            Result.Timing = EMGInputTiming::Great;
                        }
                    }
                }
            }

            // Mark matched inputs as consumed
            for (const FMGBufferedInput& Matched : RecentInputs)
            {
                MarkInputConsumed(Matched.InputId);
            }

            InputStats.CombosExecuted++;
            OnComboDetected.Broadcast(Result);

            // Clear combo progress
            ComboProgress.Remove(Combo.ComboName);
        }
    }
}

void UMGInputBufferSubsystem::CheckForDoubleTaps(EMGInputAction Action)
{
    if (const FMGInputActionState* State = ActionStates.Find(Action))
    {
        if (State->TapCount >= 2)
        {
            float TimeSinceLastTap = CurrentTime - State->LastTapTime;
            if (TimeSinceLastTap <= BufferConfig.DoubleTapWindow)
            {
                OnDoubleTapDetected.Broadcast(Action, TimeSinceLastTap);

                // Reset tap count
                FMGInputActionState* MutableState = ActionStates.Find(Action);
                if (MutableState)
                {
                    MutableState->TapCount = 0;
                }
            }
        }
    }
}

void UMGInputBufferSubsystem::ProcessPlayback(float DeltaTime)
{
    if (!bIsPlayingBack || bIsPlaybackPaused)
    {
        return;
    }

    PlaybackTime += DeltaTime * PlaybackSpeed;

    // Find and inject inputs that should have played by now
    while (PlaybackIndex < PlaybackRecording.RecordedInputs.Num())
    {
        const FMGBufferedInput& RecordedInput = PlaybackRecording.RecordedInputs[PlaybackIndex];

        // Calculate relative time within recording
        float RecordedRelativeTime = RecordedInput.Timestamp -
                                      (PlaybackRecording.RecordedInputs.Num() > 0 ?
                                       PlaybackRecording.RecordedInputs[0].Timestamp : 0.0f);

        if (PlaybackTime >= RecordedRelativeTime)
        {
            // Create a copy with current timestamp
            FMGBufferedInput PlaybackInput = RecordedInput;
            PlaybackInput.InputId = FGuid::NewGuid();
            PlaybackInput.Timestamp = CurrentTime;
            PlaybackInput.FrameNumber = CurrentFrame;

            BufferRawInput(PlaybackInput);
            PlaybackIndex++;
        }
        else
        {
            break;
        }
    }

    // Check if playback is complete
    if (PlaybackIndex >= PlaybackRecording.RecordedInputs.Num())
    {
        StopPlayback();
    }
}

bool UMGInputBufferSubsystem::MatchesCombo(const FMGInputCombo& Combo, const TArray<FMGBufferedInput>& Inputs) const
{
    if (Inputs.Num() < Combo.RequiredInputs.Num())
    {
        return false;
    }

    switch (Combo.ComboType)
    {
    case EMGComboType::Sequence:
        {
            int32 MatchIndex = 0;
            for (const FMGBufferedInput& Input : Inputs)
            {
                if (Input.Action == Combo.RequiredInputs[MatchIndex])
                {
                    if (Combo.RequiredStates.Num() > MatchIndex)
                    {
                        if (Input.State != Combo.RequiredStates[MatchIndex])
                        {
                            if (Combo.bStrictOrder)
                            {
                                return false;
                            }
                            continue;
                        }
                    }

                    MatchIndex++;
                    if (MatchIndex >= Combo.RequiredInputs.Num())
                    {
                        return true;
                    }
                }
                else if (Combo.bStrictOrder)
                {
                    MatchIndex = 0; // Reset on wrong input
                }
            }
            return false;
        }

    case EMGComboType::Simultaneous:
        {
            TSet<EMGInputAction> FoundActions;
            for (const FMGBufferedInput& Input : Inputs)
            {
                if (Combo.RequiredInputs.Contains(Input.Action))
                {
                    FoundActions.Add(Input.Action);
                }
            }
            return FoundActions.Num() >= Combo.RequiredInputs.Num();
        }

    case EMGComboType::ChargeRelease:
        {
            if (Combo.RequiredInputs.Num() < 2)
            {
                return false;
            }

            // Find charge start
            const FMGBufferedInput* ChargeStart = nullptr;
            const FMGBufferedInput* ChargeRelease = nullptr;

            for (const FMGBufferedInput& Input : Inputs)
            {
                if (Input.Action == Combo.RequiredInputs[0] &&
                    (Input.State == EMGInputState::Pressed || Input.State == EMGInputState::Held))
                {
                    ChargeStart = &Input;
                }
                else if (Input.Action == Combo.RequiredInputs[1] &&
                         Input.State == EMGInputState::Released && ChargeStart)
                {
                    ChargeRelease = &Input;
                }
            }

            if (ChargeStart && ChargeRelease)
            {
                float HoldTime = ChargeRelease->Timestamp - ChargeStart->Timestamp;
                return HoldTime >= Combo.MinHoldTime &&
                       (Combo.MaxHoldTime <= 0.0f || HoldTime <= Combo.MaxHoldTime);
            }
            return false;
        }

    case EMGComboType::DoubleTap:
        {
            if (Combo.RequiredInputs.Num() < 1)
            {
                return false;
            }

            int32 TapCount = 0;
            float LastTapTime = -1.0f;

            for (const FMGBufferedInput& Input : Inputs)
            {
                if (Input.Action == Combo.RequiredInputs[0] && Input.State == EMGInputState::Pressed)
                {
                    if (LastTapTime < 0.0f || (Input.Timestamp - LastTapTime) <= BufferConfig.DoubleTapWindow)
                    {
                        TapCount++;
                        LastTapTime = Input.Timestamp;
                    }
                    else
                    {
                        TapCount = 1;
                        LastTapTime = Input.Timestamp;
                    }
                }
            }

            return TapCount >= 2;
        }

    case EMGComboType::TapHold:
        {
            if (Combo.RequiredInputs.Num() < 2)
            {
                return false;
            }

            bool bHoldFound = false;
            bool bTapFound = false;

            for (const FMGBufferedInput& Input : Inputs)
            {
                if (Input.Action == Combo.RequiredInputs[0] && Input.State == EMGInputState::Held)
                {
                    if (Input.HoldDuration >= Combo.MinHoldTime)
                    {
                        bHoldFound = true;
                    }
                }
                else if (Input.Action == Combo.RequiredInputs[1] && Input.State == EMGInputState::Pressed)
                {
                    bTapFound = true;
                }
            }

            return bHoldFound && bTapFound;
        }

    default:
        return false;
    }
}

void UMGInputBufferSubsystem::RecordInput(const FMGBufferedInput& Input)
{
    CurrentRecording.RecordedInputs.Add(Input);
}

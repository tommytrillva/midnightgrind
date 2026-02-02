# Midnight Grind - Input System Quick Reference
## Developer Cheat Sheet

---

## Setup in Vehicle Blueprint

### Basic Setup (C++)
```cpp
// In your vehicle class header
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UMGEnhancedInputHandler> InputHandler;

// In constructor
InputHandler = CreateDefaultSubobject<UMGEnhancedInputHandler>(TEXT("InputHandler"));

// In BeginPlay
InputHandler->ApplyBalancedPreset(); // or Competitive/Casual
```

### Blueprint Setup
1. Add `MGEnhancedInputHandler` component to vehicle
2. Set preset in Details panel: `Balanced`
3. Enable `Use Input Buffer`: `true`
4. Set `Buffer Window Seconds`: `0.10`

---

## Common Operations

### Getting Input Values

```cpp
// Get processed steering (with all curves/smoothing applied)
float Steering = InputHandler->GetProcessedInput(FName("Steering"));

// Get raw steering (before processing)
float RawSteering = InputHandler->GetRawInput(FName("Steering"));

// Get from input state
float Throttle = InputHandler->GetThrottle();
float Brake = InputHandler->GetBrake();
```

### Applying Presets

```cpp
// Competitive (minimal processing)
InputHandler->ApplyCompetitivePreset();

// Balanced (recommended)
InputHandler->ApplyBalancedPreset();

// Casual (forgiving)
InputHandler->ApplyCasualPreset();

// Custom preset by name
InputHandler->LoadPreset(FName("CustomPreset"));
```

### Configuring Response Curves

```cpp
FMGEnhancedInputConfig Config = InputHandler->GetEnhancedConfig();

// Set steering to aggressive curve
Config.SteeringCurve.CurveType = EMGResponseCurveType::Aggressive;
Config.SteeringCurve.Sensitivity = 1.2f;
Config.SteeringCurve.InnerDeadzone = 0.08f;

// Set throttle to progressive
Config.ThrottleCurve.CurveType = EMGResponseCurveType::Progressive;
Config.ThrottleCurve.ExponentPower = 1.5f;

// Apply config
InputHandler->SetEnhancedConfig(Config);
```

### Using Input Buffer

```cpp
// Check if drift input is buffered
float BufferedDrift;
if (InputHandler->GetBufferedInput(FName("DriftStart"), BufferedDrift))
{
    // Input was pressed recently
    if (CanStartDrift())
    {
        StartDrift();
        InputHandler->ConsumeBufferedInput(FName("DriftStart"));
    }
}
```

### Keyboard Simulation

```cpp
// Enable keyboard simulation
InputHandler->SetKeyboardSimulationEnabled(true);

// Get keyboard simulator
UMGKeyboardInputSimulator* KBSim = InputHandler->GetKeyboardSimulator();

// Set custom ramp times for steering
KBSim->SetChannelRampSpeed(FName("Steering"), 0.08f, 0.06f);

// Disable tap detection for throttle
KBSim->SetTapDetectionEnabled(FName("Throttle"), false);
```

---

## Response Curve Examples

### Ultra-Responsive (Drift Cars)
```cpp
FMGAxisResponseConfig DriftSteering;
DriftSteering.CurveType = EMGResponseCurveType::Aggressive;
DriftSteering.Sensitivity = 1.4f;
DriftSteering.InnerDeadzone = 0.05f;
DriftSteering.OuterDeadzone = 0.02f;
DriftSteering.ExponentPower = 0.6f;
```

### Smooth & Precise (GT Cars)
```cpp
FMGAxisResponseConfig GTSteering;
GTSteering.CurveType = EMGResponseCurveType::Progressive;
GTSteering.Sensitivity = 1.1f;
GTSteering.InnerDeadzone = 0.10f;
GTSteering.OuterDeadzone = 0.05f;
GTSteering.ExponentPower = 2.0f;
```

### Beginner-Friendly
```cpp
FMGAxisResponseConfig BeginnerSteering;
BeginnerSteering.CurveType = EMGResponseCurveType::SCurve;
BeginnerSteering.Sensitivity = 1.0f;
BeginnerSteering.InnerDeadzone = 0.15f;
BeginnerSteering.OuterDeadzone = 0.08f;
BeginnerSteering.ExponentPower = 1.0f;
```

---

## Assist Configuration

### Pro Settings (No Assists)
```cpp
FMGInputAssistSettings ProAssists;
ProAssists.bSteeringAssist = false;
ProAssists.bCounterSteerAssist = false;
ProAssists.bBrakingAssist = false;
ProAssists.bThrottleAssist = false;
ProAssists.bAutoShift = false;
ProAssists.bSpeedSensitiveSteering = false;

InputHandler->SetAssistSettings(ProAssists);
```

### Balanced Settings
```cpp
FMGInputAssistSettings BalancedAssists;
BalancedAssists.bSteeringAssist = false;
BalancedAssists.bCounterSteerAssist = true;
BalancedAssists.CounterSteerStrength = 0.3f;
BalancedAssists.bSpeedSensitiveSteering = true;
BalancedAssists.HighSpeedSteeringSensitivity = 0.6f;

InputHandler->SetAssistSettings(BalancedAssists);
```

### Casual Settings
```cpp
FMGInputAssistSettings CasualAssists;
CasualAssists.bSteeringAssist = true;
CasualAssists.SteeringAssistStrength = 0.5f;
CasualAssists.bCounterSteerAssist = true;
CasualAssists.CounterSteerStrength = 0.6f;
CasualAssists.bBrakingAssist = true;
CasualAssists.bAutoShift = true;

InputHandler->SetAssistSettings(CasualAssists);
```

---

## Analytics

### Get Input Quality Metrics
```cpp
FMGInputAnalytics Analytics = InputHandler->GetAnalytics();

// Check smoothness (0-1, higher = better)
if (Analytics.AverageSmoothness < 0.5f)
{
    ShowTip("Try smoother steering inputs!");
}

// Check correction rate (corrections per second)
if (Analytics.CorrectionRate > 10.0f)
{
    ShowTip("Too many steering corrections - be gentle!");
}

// Total inputs processed
UE_LOG(LogInput, Log, TEXT("Total inputs: %d"), Analytics.TotalInputsProcessed);
```

### Input History for Visualization
```cpp
// Get last 60 frames of steering input
TArray<float> SteeringHistory = InputHandler->GetInputHistory(FName("Steering"), 60);

// Draw on HUD or graph
for (int32 i = 0; i < SteeringHistory.Num(); ++i)
{
    float Value = SteeringHistory[i];
    FVector2D Position(i * 10.0f, 500.0f - Value * 100.0f);
    Canvas->DrawPoint(Position, FLinearColor::Green);
}
```

---

## Keyboard-Specific Code

### Detecting Keyboard Input
```cpp
// Auto-detect keyboard usage
bool bIsKeyboard = !UMGInputUtility::IsUsingGamepad(GetWorld());
InputHandler->SetKeyboardSimulationEnabled(bIsKeyboard);
```

### Custom Keyboard Config
```cpp
FMGKeyboardSimulationConfig KBConfig;
KBConfig.RampUpTime = 0.08f;      // Fast response
KBConfig.RampDownTime = 0.06f;    // Even faster release
KBConfig.bInstantReversal = true;  // Snap to opposite direction
KBConfig.TapDetectionTime = 0.12f; // Quick taps detected
KBConfig.TapMaxOutput = 0.65f;     // Taps only 65% power

InputHandler->GetKeyboardSimulator()->SetConfiguration(KBConfig);
```

---

## Per-Vehicle Tuning

### Drift Car
```cpp
void ADriftCar::ConfigureInput()
{
    InputHandler->ApplyCompetitivePreset();
    
    FMGEnhancedInputConfig Config = InputHandler->GetEnhancedConfig();
    Config.SteeringCurve.CurveType = EMGResponseCurveType::Aggressive;
    Config.SteeringCurve.Sensitivity = 1.5f;
    Config.ProcessingMethod = EMGInputProcessingMethod::Direct;
    InputHandler->SetEnhancedConfig(Config);
    
    FMGInputAssistSettings Assists;
    Assists.bCounterSteerAssist = true;
    Assists.CounterSteerStrength = 0.4f;
    InputHandler->SetAssistSettings(Assists);
}
```

### GT Car
```cpp
void AGTCar::ConfigureInput()
{
    InputHandler->ApplyBalancedPreset();
    
    FMGEnhancedInputConfig Config = InputHandler->GetEnhancedConfig();
    Config.SteeringCurve.CurveType = EMGResponseCurveType::Progressive;
    Config.SteeringCurve.ExponentPower = 2.0f;
    Config.ProcessingMethod = EMGInputProcessingMethod::Smoothed;
    Config.SmoothingStrength = 0.3f;
    InputHandler->SetEnhancedConfig(Config);
    
    FMGInputAssistSettings Assists;
    Assists.bSpeedSensitiveSteering = true;
    Assists.HighSpeedSteeringSensitivity = 0.5f;
    InputHandler->SetAssistSettings(Assists);
}
```

### Rally Car
```cpp
void ARallyCar::ConfigureInput()
{
    InputHandler->ApplyBalancedPreset();
    
    FMGEnhancedInputConfig Config = InputHandler->GetEnhancedConfig();
    Config.SteeringCurve.CurveType = EMGResponseCurveType::SCurve;
    Config.ProcessingMethod = EMGInputProcessingMethod::Filtered;
    Config.SmoothingStrength = 0.4f; // More smoothing for rough terrain
    InputHandler->SetEnhancedConfig(Config);
}
```

---

## Blueprint Nodes

### Common Nodes
- `Get Processed Input` - Get final processed value
- `Get Raw Input` - Get unprocessed value
- `Apply [Preset] Preset` - Load preset configuration
- `Set Enhanced Config` - Custom configuration
- `Get Analytics` - Performance metrics
- `Enable Keyboard Simulation` - Toggle keyboard mode

### Input Buffer Nodes
- `Get Buffered Input` - Check for buffered input
- `Consume Buffered Input` - Mark input as used
- `Has Buffered Input` - Simple check

---

## Common Patterns

### Drift Initiation
```cpp
// Frame-perfect drift start
if (InputHandler->ConsumeBufferedInput(FName("Brake")) &&
    FMath::Abs(InputHandler->GetProcessedInput(FName("Steering"))) > 0.7f)
{
    StartDrift();
}
```

### Launch Control
```cpp
// Burnout/launch sequence
if (CurrentInputState.Brake > 0.9f && 
    CurrentInputState.Throttle > 0.9f &&
    GetVehicleSpeed() < 5.0f)
{
    ActivateLaunchControl();
}
```

### Adaptive Difficulty
```cpp
// Adjust assists based on performance
void AdjustDifficultyBasedOnPerformance()
{
    FMGInputAnalytics Analytics = InputHandler->GetAnalytics();
    
    if (Analytics.AverageSmoothness < 0.4f)
    {
        // Player struggling - add assists
        FMGInputAssistSettings Assists = InputHandler->GetAssistSettings();
        Assists.bSteeringAssist = true;
        Assists.SteeringAssistStrength = 0.3f;
        InputHandler->SetAssistSettings(Assists);
    }
}
```

---

## Debug Commands

### Console Commands (Blueprint Exposed)
```
// In Blueprint, create console commands:
DebugInput.ShowInputs 1/0        // Toggle input visualization
DebugInput.ShowAnalytics 1/0     // Toggle analytics display
DebugInput.SetPreset <name>      // Change preset
DebugInput.ResetAnalytics        // Clear analytics
DebugInput.DumpHistory           // Print input history to log
```

### Visual Debugging
```cpp
// Enable on-screen input display
InputHandler->SetInputVisualizationEnabled(true);

// Get history for custom visualization
TArray<float> History = InputHandler->GetInputHistory(FName("Steering"));
```

---

## Performance Tips

1. **Use Direct processing** for competitive mode (lowest latency)
2. **Limit history size** to 60 frames if not using analytics
3. **Disable analytics** in shipping builds
4. **Cache input values** instead of calling Get every frame
5. **Use input buffer** only for special moves, not continuous input

---

## Common Issues & Quick Fixes

### Issue: Input feels delayed
```cpp
// Quick fix
Config.ProcessingMethod = EMGInputProcessingMethod::Direct;
Config.SmoothingStrength = 0.0f;
```

### Issue: Gamepad drift
```cpp
// Quick fix
Config.SteeringCurve.InnerDeadzone = 0.20f; // Increase deadzone
```

### Issue: Keyboard too sensitive
```cpp
// Quick fix
KeyboardSimulator->SetChannelRampSpeed(FName("Steering"), 0.15f, 0.10f);
```

### Issue: Can't hit frame-perfect inputs
```cpp
// Quick fix
Config.bUseInputBuffer = true;
Config.BufferWindowSeconds = 0.15f; // Increase window
```

---

## API Reference Summary

| Class | Purpose | Key Methods |
|-------|---------|-------------|
| `UMGEnhancedInputHandler` | Main input handler | `ApplyPreset()`, `GetProcessedInput()` |
| `UMGInputResponseCurves` | Response curves | `ApplyResponseCurve()`, `GetPreset()` |
| `UMGKeyboardInputSimulator` | Keyboard smoothing | `UpdateDualChannel()`, `SetConfiguration()` |
| `UMGInputBufferSubsystem` | Frame-perfect inputs | `BufferInput()`, `ConsumeBufferedInput()` |

---

## Where to Find Things

| What | Where |
|------|-------|
| Presets | `MGEnhancedInputHandler::ApplyXXXPreset()` |
| Curve Types | `EMGResponseCurveType` enum |
| Assist Settings | `FMGInputAssistSettings` struct |
| Analytics | `FMGInputAnalytics` struct |
| Config | `FMGEnhancedInputConfig` struct |

---

**Quick Reference Version 1.0**  
Last Updated: 2024  
For full documentation, see `InputSystemImplementationGuide.md`

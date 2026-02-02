// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGEnhancedInputHandler.cpp
 * @brief Implementation of enhanced competitive-grade input processing
 */

#include "Input/MGEnhancedInputHandler.h"
#include "Input/MGInputResponseCurves.h"
#include "Input/MGKeyboardInputSimulator.h"
#include "InputBuffer/MGInputBufferSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UMGEnhancedInputHandler::UMGEnhancedInputHandler()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics; // Process before physics

	// Create keyboard simulator
	KeyboardSimulator = CreateDefaultSubobject<UMGKeyboardInputSimulator>(TEXT("KeyboardSimulator"));

	// Initialize with balanced preset
	ApplyBalancedPreset();
}

void UMGEnhancedInputHandler::BeginPlay()
{
	Super::BeginPlay();

	// Cache input buffer subsystem
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		InputBufferSubsystem = GameInstance->GetSubsystem<UMGInputBufferSubsystem>();
		
		if (InputBufferSubsystem && EnhancedConfig.bUseInputBuffer)
		{
			// Configure buffer window
			InputBufferSubsystem->SetBufferWindow(EnhancedConfig.BufferWindowSeconds);
		}
	}

	// Detect if player is using keyboard
	// In a real implementation, you'd hook into input device detection
	// For now, we'll enable keyboard simulation if no gamepad is detected
	bKeyboardSimulationEnabled = false; // Will be set dynamically
}

void UMGEnhancedInputHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// Call base implementation first
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update analytics if enabled
	if (EnhancedConfig.bEnableAnalytics)
	{
		UpdateAnalytics(DeltaTime);
	}

	// Process input buffer if using it
	if (InputBufferSubsystem && EnhancedConfig.bUseInputBuffer)
	{
		InputBufferSubsystem->ProcessInputFrame(DeltaTime);
	}
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGEnhancedInputHandler::SetEnhancedConfig(const FMGEnhancedInputConfig& NewConfig)
{
	EnhancedConfig = NewConfig;

	// Apply keyboard config to simulator
	if (KeyboardSimulator)
	{
		KeyboardSimulator->SetConfiguration(EnhancedConfig.KeyboardConfig);
	}

	// Update buffer window if changed
	if (InputBufferSubsystem && EnhancedConfig.bUseInputBuffer)
	{
		InputBufferSubsystem->SetBufferWindow(EnhancedConfig.BufferWindowSeconds);
	}
}

void UMGEnhancedInputHandler::LoadPreset(FName PresetName)
{
	if (PresetName == FName("Competitive"))
	{
		ApplyCompetitivePreset();
	}
	else if (PresetName == FName("Balanced"))
	{
		ApplyBalancedPreset();
	}
	else if (PresetName == FName("Casual"))
	{
		ApplyCasualPreset();
	}
}

void UMGEnhancedInputHandler::ApplyCompetitivePreset()
{
	// Minimal processing, maximum responsiveness
	EnhancedConfig.SteeringCurve = UMGInputResponseCurves::GetCompetitivePreset();
	EnhancedConfig.ThrottleCurve = UMGInputResponseCurves::GetCompetitivePreset();
	EnhancedConfig.BrakeCurve = UMGInputResponseCurves::GetCompetitivePreset();
	
	EnhancedConfig.ProcessingMethod = EMGInputProcessingMethod::Direct;
	EnhancedConfig.SmoothingStrength = 0.0f;
	EnhancedConfig.BufferWindowSeconds = 0.1f;
	
	// Fast keyboard response
	EnhancedConfig.KeyboardConfig.RampUpTime = 0.05f;
	EnhancedConfig.KeyboardConfig.RampDownTime = 0.04f;
	EnhancedConfig.KeyboardConfig.bInstantReversal = true;

	// Disable assists in base handler
	FMGInputAssistSettings Assists;
	Assists.bSteeringAssist = false;
	Assists.bCounterSteerAssist = false;
	Assists.bBrakingAssist = false;
	Assists.bThrottleAssist = false;
	Assists.bAutoShift = false;
	Assists.bSpeedSensitiveSteering = false;
	SetAssistSettings(Assists);

	// Set minimal sensitivity settings in base handler
	FMGInputSensitivity Sensitivity;
	Sensitivity.SteeringSensitivity = 1.0f;
	Sensitivity.SteeringLinearity = 1.0f;
	Sensitivity.SteeringDeadzone = 0.05f;
	Sensitivity.ThrottleSensitivity = 1.0f;
	Sensitivity.ThrottleDeadzone = 0.02f;
	Sensitivity.BrakeSensitivity = 1.0f;
	Sensitivity.BrakeDeadzone = 0.02f;
	SetSensitivitySettings(Sensitivity);

	SetEnhancedConfig(EnhancedConfig);
}

void UMGEnhancedInputHandler::ApplyBalancedPreset()
{
	// Good balance of responsiveness and forgiveness
	EnhancedConfig.SteeringCurve = UMGInputResponseCurves::GetBalancedPreset();
	EnhancedConfig.ThrottleCurve = UMGInputResponseCurves::GetBalancedPreset();
	EnhancedConfig.BrakeCurve = UMGInputResponseCurves::GetBalancedPreset();
	
	EnhancedConfig.ProcessingMethod = EMGInputProcessingMethod::Smoothed;
	EnhancedConfig.SmoothingStrength = 0.3f;
	EnhancedConfig.BufferWindowSeconds = 0.12f;
	
	// Moderate keyboard response
	EnhancedConfig.KeyboardConfig.RampUpTime = 0.1f;
	EnhancedConfig.KeyboardConfig.RampDownTime = 0.08f;
	EnhancedConfig.KeyboardConfig.bInstantReversal = true;

	// Some assists enabled
	FMGInputAssistSettings Assists;
	Assists.bSteeringAssist = false;
	Assists.bCounterSteerAssist = true;
	Assists.CounterSteerStrength = 0.3f;
	Assists.bBrakingAssist = false;
	Assists.bThrottleAssist = false;
	Assists.bAutoShift = false;
	Assists.bSpeedSensitiveSteering = true;
	Assists.HighSpeedSteeringSensitivity = 0.6f;
	SetAssistSettings(Assists);

	// Balanced sensitivity
	FMGInputSensitivity Sensitivity;
	Sensitivity.SteeringSensitivity = 1.1f;
	Sensitivity.SteeringLinearity = 1.5f;
	Sensitivity.SteeringDeadzone = 0.10f;
	Sensitivity.ThrottleSensitivity = 1.0f;
	Sensitivity.ThrottleDeadzone = 0.05f;
	Sensitivity.BrakeSensitivity = 1.0f;
	Sensitivity.BrakeDeadzone = 0.05f;
	SetSensitivitySettings(Sensitivity);

	SetEnhancedConfig(EnhancedConfig);
}

void UMGEnhancedInputHandler::ApplyCasualPreset()
{
	// Forgiving, beginner-friendly
	EnhancedConfig.SteeringCurve = UMGInputResponseCurves::GetCasualPreset();
	EnhancedConfig.ThrottleCurve = UMGInputResponseCurves::GetCasualPreset();
	EnhancedConfig.BrakeCurve = UMGInputResponseCurves::GetCasualPreset();
	
	EnhancedConfig.ProcessingMethod = EMGInputProcessingMethod::Filtered;
	EnhancedConfig.SmoothingStrength = 0.5f;
	EnhancedConfig.BufferWindowSeconds = 0.15f;
	
	// Slower keyboard response
	EnhancedConfig.KeyboardConfig.RampUpTime = 0.15f;
	EnhancedConfig.KeyboardConfig.RampDownTime = 0.12f;
	EnhancedConfig.KeyboardConfig.bInstantReversal = false;

	// All assists enabled
	FMGInputAssistSettings Assists;
	Assists.bSteeringAssist = true;
	Assists.SteeringAssistStrength = 0.5f;
	Assists.bCounterSteerAssist = true;
	Assists.CounterSteerStrength = 0.6f;
	Assists.bBrakingAssist = true;
	Assists.bThrottleAssist = true;
	Assists.bAutoShift = true;
	Assists.bSpeedSensitiveSteering = true;
	Assists.HighSpeedSteeringSensitivity = 0.5f;
	SetAssistSettings(Assists);

	// Forgiving sensitivity
	FMGInputSensitivity Sensitivity;
	Sensitivity.SteeringSensitivity = 1.3f;
	Sensitivity.SteeringLinearity = 2.0f;
	Sensitivity.SteeringDeadzone = 0.15f;
	Sensitivity.ThrottleSensitivity = 1.0f;
	Sensitivity.ThrottleDeadzone = 0.08f;
	Sensitivity.BrakeSensitivity = 1.0f;
	Sensitivity.BrakeDeadzone = 0.08f;
	SetSensitivitySettings(Sensitivity);

	SetEnhancedConfig(EnhancedConfig);
}

// ==========================================
// ENHANCED INPUT OVERRIDES
// ==========================================

void UMGEnhancedInputHandler::OnSteeringInput(const FInputActionValue& Value)
{
	float RawInput = Value.Get<float>();
	
	// Store raw value
	RawInputValues.Add(FName("Steering"), RawInput);
	
	// Process with enhancements
	float ProcessedInput = ProcessSteeringInput(RawInput, GetWorld()->GetDeltaSeconds());
	
	// Store processed value
	ProcessedInputValues.Add(FName("Steering"), ProcessedInput);
	
	// Apply to base handler's CurrentInputState
	CurrentInputState.Steering = ProcessedInput;

	// Record history for analytics
	RecordInputHistory(FName("Steering"), ProcessedInput);
	
	// Buffer the input if enabled
	if (InputBufferSubsystem && EnhancedConfig.bUseInputBuffer)
	{
		InputBufferSubsystem->BufferInput(
			EMGInputAction::SteerLeft, 
			ProcessedInput < 0.0f ? EMGInputState::Held : EMGInputState::None,
			FMath::Abs(ProcessedInput)
		);
		InputBufferSubsystem->BufferInput(
			EMGInputAction::SteerRight,
			ProcessedInput > 0.0f ? EMGInputState::Held : EMGInputState::None,
			FMath::Abs(ProcessedInput)
		);
	}
}

void UMGEnhancedInputHandler::OnThrottleInput(const FInputActionValue& Value)
{
	float RawInput = Value.Get<float>();
	
	RawInputValues.Add(FName("Throttle"), RawInput);
	float ProcessedInput = ProcessThrottleInput(RawInput, GetWorld()->GetDeltaSeconds());
	ProcessedInputValues.Add(FName("Throttle"), ProcessedInput);
	
	CurrentInputState.Throttle = ProcessedInput;
	RecordInputHistory(FName("Throttle"), ProcessedInput);
	
	if (InputBufferSubsystem && EnhancedConfig.bUseInputBuffer)
	{
		InputBufferSubsystem->BufferInput(
			EMGInputAction::Accelerate,
			ProcessedInput > 0.1f ? EMGInputState::Held : EMGInputState::None,
			ProcessedInput
		);
	}
}

void UMGEnhancedInputHandler::OnBrakeInput(const FInputActionValue& Value)
{
	float RawInput = Value.Get<float>();
	
	RawInputValues.Add(FName("Brake"), RawInput);
	float ProcessedInput = ProcessBrakeInput(RawInput, GetWorld()->GetDeltaSeconds());
	ProcessedInputValues.Add(FName("Brake"), ProcessedInput);
	
	CurrentInputState.Brake = ProcessedInput;
	RecordInputHistory(FName("Brake"), ProcessedInput);
	
	if (InputBufferSubsystem && EnhancedConfig.bUseInputBuffer)
	{
		InputBufferSubsystem->BufferInput(
			EMGInputAction::Brake,
			ProcessedInput > 0.1f ? EMGInputState::Held : EMGInputState::None,
			ProcessedInput
		);
	}
}

// ==========================================
// ENHANCED PROCESSING
// ==========================================

float UMGEnhancedInputHandler::ProcessSteeringInput(float RawInput, float DeltaTime)
{
	float ProcessedInput = RawInput;

	// Apply response curve
	ProcessedInput = UMGInputResponseCurves::ApplyResponseCurve(ProcessedInput, EnhancedConfig.SteeringCurve);

	// Apply smoothing based on processing method
	switch (EnhancedConfig.ProcessingMethod)
	{
	case EMGInputProcessingMethod::Direct:
		// No additional processing
		break;

	case EMGInputProcessingMethod::Smoothed:
	{
		float* PrevValue = ProcessedInputValues.Find(FName("Steering"));
		if (PrevValue)
		{
			ProcessedInput = ApplySmoothing(ProcessedInput, *PrevValue, EnhancedConfig.SmoothingStrength, DeltaTime);
		}
		break;
	}

	case EMGInputProcessingMethod::Filtered:
	{
		TArray<float>& History = InputHistories.FindOrAdd(FName("Steering"));
		ProcessedInput = ApplyFiltering(ProcessedInput, History, 5);
		break;
	}

	case EMGInputProcessingMethod::Predictive:
	{
		TArray<float>& History = InputHistories.FindOrAdd(FName("Steering"));
		ProcessedInput = ApplyPrediction(ProcessedInput, History);
		break;
	}
	}

	return ProcessedInput;
}

float UMGEnhancedInputHandler::ProcessThrottleInput(float RawInput, float DeltaTime)
{
	float ProcessedInput = RawInput;

	// Apply response curve
	ProcessedInput = UMGInputResponseCurves::ApplyResponseCurve(ProcessedInput, EnhancedConfig.ThrottleCurve);

	// Apply smoothing if enabled
	if (EnhancedConfig.ProcessingMethod != EMGInputProcessingMethod::Direct)
	{
		float* PrevValue = ProcessedInputValues.Find(FName("Throttle"));
		if (PrevValue)
		{
			ProcessedInput = ApplySmoothing(ProcessedInput, *PrevValue, EnhancedConfig.SmoothingStrength * 0.5f, DeltaTime);
		}
	}

	return ProcessedInput;
}

float UMGEnhancedInputHandler::ProcessBrakeInput(float RawInput, float DeltaTime)
{
	float ProcessedInput = RawInput;

	// Apply response curve
	ProcessedInput = UMGInputResponseCurves::ApplyResponseCurve(ProcessedInput, EnhancedConfig.BrakeCurve);

	// Brake typically needs less smoothing for responsive feel
	if (EnhancedConfig.ProcessingMethod == EMGInputProcessingMethod::Smoothed)
	{
		float* PrevValue = ProcessedInputValues.Find(FName("Brake"));
		if (PrevValue)
		{
			ProcessedInput = ApplySmoothing(ProcessedInput, *PrevValue, EnhancedConfig.SmoothingStrength * 0.3f, DeltaTime);
		}
	}

	return ProcessedInput;
}

// ==========================================
// SMOOTHING ALGORITHMS
// ==========================================

float UMGEnhancedInputHandler::ApplySmoothing(float RawInput, float PreviousInput, float SmoothingFactor, float DeltaTime)
{
	if (SmoothingFactor <= KINDA_SMALL_NUMBER)
	{
		return RawInput; // No smoothing
	}

	// Exponential smoothing (IIR filter)
	// Lower alpha = more smoothing
	float Alpha = 1.0f - (SmoothingFactor * 0.95f); // Map 0-1 to 1.0-0.05
	float SmoothedValue = FMath::Lerp(PreviousInput, RawInput, Alpha);
	
	return SmoothedValue;
}

float UMGEnhancedInputHandler::ApplyFiltering(float RawInput, TArray<float>& History, int32 WindowSize)
{
	// Add to history
	History.Add(RawInput);
	if (History.Num() > WindowSize)
	{
		History.RemoveAt(0);
	}

	// Simple moving average
	if (History.Num() > 0)
	{
		float Sum = 0.0f;
		for (float Value : History)
		{
			Sum += Value;
		}
		return Sum / History.Num();
	}

	return RawInput;
}

float UMGEnhancedInputHandler::ApplyPrediction(float RawInput, TArray<float>& History)
{
	// Add to history
	History.Add(RawInput);
	if (History.Num() > 5)
	{
		History.RemoveAt(0);
	}

	// Simple linear prediction based on recent trend
	if (History.Num() >= 3)
	{
		float Delta1 = History[History.Num() - 1] - History[History.Num() - 2];
		float Delta2 = History[History.Num() - 2] - History[History.Num() - 3];
		float AverageDelta = (Delta1 + Delta2) * 0.5f;
		
		// Predict next value (slight anticipation)
		float PredictedValue = RawInput + (AverageDelta * 0.25f);
		return FMath::Clamp(PredictedValue, -1.0f, 1.0f);
	}

	return RawInput;
}

// ==========================================
// ADVANCED INPUT
// ==========================================

float UMGEnhancedInputHandler::GetProcessedInput(FName InputName) const
{
	if (const float* Value = ProcessedInputValues.Find(InputName))
	{
		return *Value;
	}
	return 0.0f;
}

float UMGEnhancedInputHandler::GetRawInput(FName InputName) const
{
	if (const float* Value = RawInputValues.Find(InputName))
	{
		return *Value;
	}
	return 0.0f;
}

bool UMGEnhancedInputHandler::GetBufferedInput(FName InputName, float& OutValue)
{
	if (!InputBufferSubsystem)
	{
		return false;
	}

	// Map input names to actions
	EMGInputAction Action = EMGInputAction::Accelerate; // Default
	
	if (InputName == FName("Throttle"))
		Action = EMGInputAction::Accelerate;
	else if (InputName == FName("Brake"))
		Action = EMGInputAction::Brake;
	else if (InputName == FName("Steering"))
		Action = EMGInputAction::SteerLeft; // Will check both

	if (InputBufferSubsystem->HasBufferedInput(Action))
	{
		OutValue = InputBufferSubsystem->GetActionAnalogValue(Action);
		return true;
	}

	return false;
}

bool UMGEnhancedInputHandler::ConsumeBufferedInput(FName InputName)
{
	if (!InputBufferSubsystem)
	{
		return false;
	}

	EMGInputAction Action = EMGInputAction::Accelerate;
	
	if (InputName == FName("Throttle"))
		Action = EMGInputAction::Accelerate;
	else if (InputName == FName("Brake"))
		Action = EMGInputAction::Brake;

	return InputBufferSubsystem->ConsumeBufferedInput(Action);
}

// ==========================================
// KEYBOARD SIMULATION
// ==========================================

void UMGEnhancedInputHandler::SetKeyboardSimulationEnabled(bool bEnabled)
{
	bKeyboardSimulationEnabled = bEnabled;
}

// ==========================================
// ANALYTICS
// ==========================================

void UMGEnhancedInputHandler::ResetAnalytics()
{
	Analytics = FMGInputAnalytics();
	AnalyticsTimeAccumulator = 0.0f;
}

void UMGEnhancedInputHandler::SetAnalyticsEnabled(bool bEnabled)
{
	EnhancedConfig.bEnableAnalytics = bEnabled;
	if (!bEnabled)
	{
		ResetAnalytics();
	}
}

void UMGEnhancedInputHandler::UpdateAnalytics(float DeltaTime)
{
	AnalyticsTimeAccumulator += DeltaTime;

	// Update analytics every 0.5 seconds
	if (AnalyticsTimeAccumulator >= 0.5f)
	{
		AnalyticsTimeAccumulator = 0.0f;

		// Calculate smoothness based on steering corrections
		TArray<float>* SteeringHistory = InputHistories.Find(FName("Steering"));
		if (SteeringHistory && SteeringHistory->Num() > 10)
		{
			int32 Corrections = 0;
			for (int32 i = 1; i < SteeringHistory->Num(); ++i)
			{
				float Delta = (*SteeringHistory)[i] - (*SteeringHistory)[i - 1];
				if (FMath::Abs(Delta) > 0.1f)
				{
					Corrections++;
				}
			}
			Analytics.CorrectionRate = static_cast<float>(Corrections) / 0.5f; // Per second
			Analytics.AverageSmoothness = 1.0f - FMath::Clamp(Analytics.CorrectionRate / 20.0f, 0.0f, 1.0f);
		}

		Analytics.TotalInputsProcessed++;
	}
}

void UMGEnhancedInputHandler::RecordInputForAnalytics(FName InputName, float Value)
{
	Analytics.PeakInputValue = FMath::Max(Analytics.PeakInputValue, FMath::Abs(Value));
}

void UMGEnhancedInputHandler::RecordInputHistory(FName InputName, float Value)
{
	TArray<float>& History = InputHistories.FindOrAdd(InputName);
	History.Add(Value);
	
	// Keep history size manageable
	if (History.Num() > MaxHistorySize)
	{
		History.RemoveAt(0);
	}
}

// ==========================================
// INPUT DEBUGGING
// ==========================================

TArray<float> UMGEnhancedInputHandler::GetInputHistory(FName InputName, int32 FrameCount) const
{
	if (const TArray<float>* History = InputHistories.Find(InputName))
	{
		int32 StartIndex = FMath::Max(0, History->Num() - FrameCount);
		TArray<float> RecentHistory;
		for (int32 i = StartIndex; i < History->Num(); ++i)
		{
			RecentHistory.Add((*History)[i]);
		}
		return RecentHistory;
	}
	return TArray<float>();
}

void UMGEnhancedInputHandler::SetInputVisualizationEnabled(bool bEnabled)
{
	bInputVisualizationEnabled = bEnabled;
	// In a full implementation, this would enable an on-screen widget showing inputs
}

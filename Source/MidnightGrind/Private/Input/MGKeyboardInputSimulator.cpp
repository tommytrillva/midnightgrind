// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGKeyboardInputSimulator.cpp
 * @brief Implementation of keyboard-to-analog input simulation
 */

#include "Input/MGKeyboardInputSimulator.h"
#include "Input/MGInputResponseCurves.h"

UMGKeyboardInputSimulator::UMGKeyboardInputSimulator()
{
	// Default configuration
	Config.RampUpTime = 0.1f;
	Config.RampDownTime = 0.08f;
	Config.RampUpCurve = EMGResponseCurveType::Progressive;
	Config.RampDownCurve = EMGResponseCurveType::Linear;
	Config.bInstantReversal = true;
	Config.TapDetectionTime = 0.15f;
	Config.TapMaxOutput = 0.6f;
}

void UMGKeyboardInputSimulator::SetConfiguration(const FMGKeyboardSimulationConfig& NewConfig)
{
	Config = NewConfig;
}

// ==========================================
// INPUT PROCESSING
// ==========================================

float UMGKeyboardInputSimulator::UpdateSingleChannel(bool bKeyPressed, float DeltaTime, FName ChannelName)
{
	FMGKeyboardChannelState& State = GetOrCreateChannel(ChannelName);

	// Update hold time
	if (bKeyPressed)
	{
		State.HoldTime += DeltaTime;
	}
	else
	{
		State.HoldTime = 0.0f;
	}

	// Determine target value
	float NewTargetValue = 0.0f;
	if (bKeyPressed)
	{
		// Check if tap detection is enabled and this is a tap
		bool bUseTapDetection = !TapDetectionEnabled.Contains(ChannelName) || TapDetectionEnabled[ChannelName];
		
		if (bUseTapDetection && IsTap(State.HoldTime))
		{
			NewTargetValue = GetTapTargetValue(State.HoldTime);
			State.bWasTap = true;
		}
		else
		{
			NewTargetValue = 1.0f;
			State.bWasTap = false;
		}
	}

	bool bTargetChanged = !FMath::IsNearlyEqual(State.TargetValue, NewTargetValue, 0.01f);
	if (bTargetChanged)
	{
		State.TargetValue = NewTargetValue;
		State.TimeSinceChange = 0.0f;
	}

	State.TimeSinceChange += DeltaTime;

	// Determine ramp direction
	State.bRampingUp = State.CurrentValue < State.TargetValue;

	// Get ramp time (use custom if set, otherwise use config)
	float RampTime = State.bRampingUp ? Config.RampUpTime : Config.RampDownTime;
	if (CustomRampSpeeds.Contains(ChannelName))
	{
		FVector2D CustomSpeed = CustomRampSpeeds[ChannelName];
		RampTime = State.bRampingUp ? CustomSpeed.X : CustomSpeed.Y;
	}

	// Ramp toward target
	State.CurrentValue = RampValue(State.CurrentValue, State.TargetValue, RampTime, DeltaTime, State.bRampingUp);
	State.bPositivePressed = bKeyPressed;

	return State.CurrentValue;
}

float UMGKeyboardInputSimulator::UpdateDualChannel(bool bPositivePressed, bool bNegativePressed, float DeltaTime, FName ChannelName)
{
	FMGKeyboardChannelState& State = GetOrCreateChannel(ChannelName);

	// Handle instant reversal if enabled
	if (Config.bInstantReversal)
	{
		// If switching from one direction to opposite, snap to center first
		if ((bPositivePressed && State.bNegativePressed && !State.bPositivePressed) ||
			(bNegativePressed && State.bPositivePressed && !State.bNegativePressed))
		{
			State.CurrentValue = 0.0f;
			State.TargetValue = 0.0f;
			State.TimeSinceChange = 0.0f;
			State.HoldTime = 0.0f;
		}
	}

	// Update hold time
	if (bPositivePressed || bNegativePressed)
	{
		State.HoldTime += DeltaTime;
	}
	else
	{
		State.HoldTime = 0.0f;
	}

	// Determine target value
	float NewTargetValue = 0.0f;
	if (bPositivePressed && !bNegativePressed)
	{
		// Check if tap detection is enabled
		bool bUseTapDetection = !TapDetectionEnabled.Contains(ChannelName) || TapDetectionEnabled[ChannelName];
		
		if (bUseTapDetection && IsTap(State.HoldTime))
		{
			NewTargetValue = GetTapTargetValue(State.HoldTime);
			State.bWasTap = true;
		}
		else
		{
			NewTargetValue = 1.0f;
			State.bWasTap = false;
		}
	}
	else if (bNegativePressed && !bPositivePressed)
	{
		bool bUseTapDetection = !TapDetectionEnabled.Contains(ChannelName) || TapDetectionEnabled[ChannelName];
		
		if (bUseTapDetection && IsTap(State.HoldTime))
		{
			NewTargetValue = -GetTapTargetValue(State.HoldTime);
			State.bWasTap = true;
		}
		else
		{
			NewTargetValue = -1.0f;
			State.bWasTap = false;
		}
	}
	else if (bPositivePressed && bNegativePressed)
	{
		// Both pressed - cancel out to zero
		NewTargetValue = 0.0f;
	}

	bool bTargetChanged = !FMath::IsNearlyEqual(State.TargetValue, NewTargetValue, 0.01f);
	if (bTargetChanged)
	{
		State.TargetValue = NewTargetValue;
		State.TimeSinceChange = 0.0f;
	}

	State.TimeSinceChange += DeltaTime;

	// Update state tracking
	State.bPositivePressed = bPositivePressed;
	State.bNegativePressed = bNegativePressed;

	// Determine ramp direction
	State.bRampingUp = FMath::Abs(State.TargetValue) > FMath::Abs(State.CurrentValue);

	// Get ramp time
	float RampTime = State.bRampingUp ? Config.RampUpTime : Config.RampDownTime;
	if (CustomRampSpeeds.Contains(ChannelName))
	{
		FVector2D CustomSpeed = CustomRampSpeeds[ChannelName];
		RampTime = State.bRampingUp ? CustomSpeed.X : CustomSpeed.Y;
	}

	// Ramp toward target
	State.CurrentValue = RampValue(State.CurrentValue, State.TargetValue, RampTime, DeltaTime, State.bRampingUp);

	return State.CurrentValue;
}

void UMGKeyboardInputSimulator::ResetChannel(FName ChannelName)
{
	if (Channels.Contains(ChannelName))
	{
		FMGKeyboardChannelState& State = Channels[ChannelName];
		State.CurrentValue = 0.0f;
		State.TargetValue = 0.0f;
		State.HoldTime = 0.0f;
		State.bPositivePressed = false;
		State.bNegativePressed = false;
		State.bWasTap = false;
		State.TimeSinceChange = 0.0f;
		State.bRampingUp = false;
	}
}

void UMGKeyboardInputSimulator::ResetAllChannels()
{
	for (auto& Pair : Channels)
	{
		FMGKeyboardChannelState& State = Pair.Value;
		State.CurrentValue = 0.0f;
		State.TargetValue = 0.0f;
		State.HoldTime = 0.0f;
		State.bPositivePressed = false;
		State.bNegativePressed = false;
		State.bWasTap = false;
		State.TimeSinceChange = 0.0f;
		State.bRampingUp = false;
	}
}

// ==========================================
// QUERY
// ==========================================

float UMGKeyboardInputSimulator::GetChannelValue(FName ChannelName) const
{
	if (const FMGKeyboardChannelState* State = Channels.Find(ChannelName))
	{
		return State->CurrentValue;
	}
	return 0.0f;
}

FMGKeyboardChannelState UMGKeyboardInputSimulator::GetChannelState(FName ChannelName) const
{
	if (const FMGKeyboardChannelState* State = Channels.Find(ChannelName))
	{
		return *State;
	}
	return FMGKeyboardChannelState();
}

bool UMGKeyboardInputSimulator::IsChannelRamping(FName ChannelName) const
{
	if (const FMGKeyboardChannelState* State = Channels.Find(ChannelName))
	{
		return !FMath::IsNearlyEqual(State->CurrentValue, State->TargetValue, 0.01f);
	}
	return false;
}

float UMGKeyboardInputSimulator::GetChannelRampProgress(FName ChannelName) const
{
	if (const FMGKeyboardChannelState* State = Channels.Find(ChannelName))
	{
		float Distance = FMath::Abs(State->TargetValue - State->CurrentValue);
		return FMath::Clamp(Distance, 0.0f, 1.0f);
	}
	return 0.0f;
}

// ==========================================
// ADVANCED
// ==========================================

void UMGKeyboardInputSimulator::SetChannelRampSpeed(FName ChannelName, float RampUpTime, float RampDownTime)
{
	CustomRampSpeeds.Add(ChannelName, FVector2D(RampUpTime, RampDownTime));
}

void UMGKeyboardInputSimulator::ClearChannelRampSpeed(FName ChannelName)
{
	CustomRampSpeeds.Remove(ChannelName);
}

void UMGKeyboardInputSimulator::SetTapDetectionEnabled(FName ChannelName, bool bEnabled)
{
	TapDetectionEnabled.Add(ChannelName, bEnabled);
}

// ==========================================
// INTERNAL HELPERS
// ==========================================

FMGKeyboardChannelState& UMGKeyboardInputSimulator::GetOrCreateChannel(FName ChannelName)
{
	if (!Channels.Contains(ChannelName))
	{
		FMGKeyboardChannelState NewState;
		Channels.Add(ChannelName, NewState);
	}
	return Channels[ChannelName];
}

float UMGKeyboardInputSimulator::RampValue(float Current, float Target, float RampTime, float DeltaTime, bool bRampUp) const
{
	if (RampTime <= KINDA_SMALL_NUMBER)
	{
		return Target; // Instant
	}

	// Calculate how much to change this frame
	float MaxChange = (1.0f / RampTime) * DeltaTime;
	float Distance = Target - Current;
	float Direction = FMath::Sign(Distance);
	float AbsDistance = FMath::Abs(Distance);

	// Apply curve to the ramp
	float T = FMath::Clamp(1.0f - (AbsDistance / MaxChange), 0.0f, 1.0f);
	EMGResponseCurveType CurveType = bRampUp ? Config.RampUpCurve : Config.RampDownCurve;
	float CurvedT = ApplyRampCurve(T, CurveType);
	
	// Move toward target
	float NewValue = Current + (Direction * MaxChange);

	// Don't overshoot
	if ((Direction > 0.0f && NewValue > Target) || (Direction < 0.0f && NewValue < Target))
	{
		NewValue = Target;
	}

	return NewValue;
}

bool UMGKeyboardInputSimulator::IsTap(float HoldTime) const
{
	return HoldTime <= Config.TapDetectionTime;
}

float UMGKeyboardInputSimulator::GetTapTargetValue(float HoldTime) const
{
	// Linearly interpolate between tap max and full based on hold time
	float T = FMath::Clamp(HoldTime / Config.TapDetectionTime, 0.0f, 1.0f);
	return FMath::Lerp(Config.TapMaxOutput, 1.0f, T);
}

float UMGKeyboardInputSimulator::ApplyRampCurve(float T, EMGResponseCurveType CurveType) const
{
	return UMGInputResponseCurves::ApplyCurveType(T, CurveType, 2.0f);
}

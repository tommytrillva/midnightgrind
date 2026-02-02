// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGInputResponseCurves.cpp
 * @brief Implementation of advanced input response curves
 */

#include "Input/MGInputResponseCurves.h"
#include "Curves/CurveFloat.h"

// ==========================================
// 1D INPUT PROCESSING
// ==========================================

float UMGInputResponseCurves::ApplyResponseCurve(float RawInput, const FMGAxisResponseConfig& Config)
{
	// Store sign for later
	float Sign = FMath::Sign(RawInput);
	float AbsInput = FMath::Abs(RawInput);

	// Apply deadzone first
	float DeadzonedInput = ApplyDeadzone(AbsInput, Config.InnerDeadzone, Config.OuterDeadzone);
	
	if (FMath::IsNearlyZero(DeadzonedInput))
	{
		return 0.0f;
	}

	float ProcessedInput = DeadzonedInput;

	// Apply curve (before or after sensitivity based on config)
	if (Config.bCurveBeforeSensitivity)
	{
		ProcessedInput = ApplyCurveType(ProcessedInput, Config.CurveType, Config.ExponentPower);
		if (Config.CustomCurve)
		{
			ProcessedInput = CustomCurve(ProcessedInput, Config.CustomCurve);
		}
		ProcessedInput = ApplySensitivity(ProcessedInput, Config.Sensitivity);
	}
	else
	{
		ProcessedInput = ApplySensitivity(ProcessedInput, Config.Sensitivity);
		ProcessedInput = ApplyCurveType(ProcessedInput, Config.CurveType, Config.ExponentPower);
		if (Config.CustomCurve)
		{
			ProcessedInput = CustomCurve(ProcessedInput, Config.CustomCurve);
		}
	}

	// Apply inversion if needed
	if (Config.bInverted)
	{
		Sign *= -1.0f;
	}

	return FMath::Clamp(ProcessedInput * Sign, -1.0f, 1.0f);
}

float UMGInputResponseCurves::ApplyCurveType(float NormalizedInput, EMGResponseCurveType CurveType, float ExponentPower)
{
	switch (CurveType)
	{
	case EMGResponseCurveType::Linear:
		return LinearCurve(NormalizedInput);

	case EMGResponseCurveType::Progressive:
		return ProgressiveCurve(NormalizedInput, ExponentPower);

	case EMGResponseCurveType::Aggressive:
		return AggressiveCurve(NormalizedInput, 0.5f);

	case EMGResponseCurveType::SCurve:
		return SCurve(NormalizedInput, 5.0f);

	case EMGResponseCurveType::Exponential:
		return ExponentialCurve(NormalizedInput, ExponentPower);

	case EMGResponseCurveType::Custom:
		// Custom handled separately
		return NormalizedInput;

	default:
		return NormalizedInput;
	}
}

float UMGInputResponseCurves::ApplyDeadzone(float RawInput, float InnerDeadzone, float OuterDeadzone)
{
	float AbsInput = FMath::Abs(RawInput);
	
	// Inner deadzone - ignore inputs below threshold
	if (AbsInput < InnerDeadzone)
	{
		return 0.0f;
	}

	// Remap from [InnerDeadzone, 1-OuterDeadzone] to [0, 1]
	float MaxValue = 1.0f - OuterDeadzone;
	float RemappedInput = (AbsInput - InnerDeadzone) / (MaxValue - InnerDeadzone);
	
	return FMath::Clamp(RemappedInput, 0.0f, 1.0f);
}

float UMGInputResponseCurves::ApplySensitivity(float Input, float Sensitivity)
{
	return FMath::Clamp(Input * Sensitivity, 0.0f, 1.0f);
}

// ==========================================
// 2D INPUT PROCESSING
// ==========================================

FVector2D UMGInputResponseCurves::Apply2DResponseCurve(FVector2D RawInput, const FMG2DInputResponseConfig& Config)
{
	FVector2D ProcessedInput = RawInput;

	// Apply radial deadzone first if configured
	if (Config.DeadzoneShape != EMGDeadzoneShape::Axial)
	{
		ProcessedInput = ApplyRadialDeadzone(RawInput, Config.RadialDeadzone, Config.DeadzoneShape);
	}

	// Apply individual axis curves
	float ProcessedX = ApplyResponseCurve(ProcessedInput.X, Config.XAxisConfig);
	float ProcessedY = ApplyResponseCurve(ProcessedInput.Y, Config.YAxisConfig);

	FVector2D Result(ProcessedX, ProcessedY);

	// Compensate for diagonal magnitude reduction if needed
	if (Config.bCompensateDiagonals)
	{
		float InputMagnitude = RawInput.Size();
		float OutputMagnitude = Result.Size();
		
		if (OutputMagnitude > KINDA_SMALL_NUMBER && InputMagnitude > KINDA_SMALL_NUMBER)
		{
			// Scale output to match input magnitude (prevents "dead corners")
			float MagnitudeRatio = InputMagnitude / OutputMagnitude;
			Result *= FMath::Min(MagnitudeRatio, 1.414f); // Max sqrt(2) for diagonal
			Result.X = FMath::Clamp(Result.X, -1.0f, 1.0f);
			Result.Y = FMath::Clamp(Result.Y, -1.0f, 1.0f);
		}
	}

	return Result;
}

FVector2D UMGInputResponseCurves::ApplyRadialDeadzone(FVector2D Input, float Deadzone, EMGDeadzoneShape Shape)
{
	switch (Shape)
	{
	case EMGDeadzoneShape::Radial:
	case EMGDeadzoneShape::ScaledRadial:
		return ApplyScaledRadialDeadzone(Input, Deadzone);

	case EMGDeadzoneShape::Hybrid:
	{
		// Radial inner deadzone, axial processing
		float Magnitude = Input.Size();
		if (Magnitude < Deadzone)
		{
			return FVector2D::ZeroVector;
		}
		
		// Remap magnitude
		float RemappedMagnitude = (Magnitude - Deadzone) / (1.0f - Deadzone);
		FVector2D Direction = Input.GetSafeNormal();
		return Direction * RemappedMagnitude;
	}

	case EMGDeadzoneShape::Axial:
	default:
		// Axial handled per-axis in individual configs
		return Input;
	}
}

FVector2D UMGInputResponseCurves::ApplyScaledRadialDeadzone(FVector2D Input, float Deadzone)
{
	float Magnitude = Input.Size();
	
	if (Magnitude < Deadzone)
	{
		return FVector2D::ZeroVector;
	}

	// Remap from [Deadzone, 1.0] to [0.0, 1.0]
	float RemappedMagnitude = (Magnitude - Deadzone) / (1.0f - Deadzone);
	
	// Preserve direction, apply remapped magnitude
	FVector2D Direction = Input / Magnitude;
	FVector2D Result = Direction * RemappedMagnitude;

	// Scale to ensure we can reach (1, 1) at corners
	float MaxComponent = FMath::Max(FMath::Abs(Result.X), FMath::Abs(Result.Y));
	if (MaxComponent > KINDA_SMALL_NUMBER)
	{
		Result /= MaxComponent;
	}

	return Result;
}

// ==========================================
// CURVE ALGORITHMS
// ==========================================

float UMGInputResponseCurves::LinearCurve(float Input)
{
	return Input;
}

float UMGInputResponseCurves::ProgressiveCurve(float Input, float Power)
{
	// Power curve: y = x^n (n > 1)
	// More precision at low inputs, aggressive at high inputs
	return FastPow(Input, Power);
}

float UMGInputResponseCurves::AggressiveCurve(float Input, float Power)
{
	// Inverse power curve: y = x^(1/n)
	// Quick response at low inputs, flattens at high inputs
	return FastPow(Input, Power);
}

float UMGInputResponseCurves::SCurve(float Input, float Steepness)
{
	// Sigmoid-based S-curve
	// Maps [0, 1] input to smooth S-shape
	// Steepness controls how "sharp" the middle transition is
	
	// Remap input from [0, 1] to [-Steepness, Steepness]
	float X = (Input * 2.0f - 1.0f) * Steepness;
	
	// Fast sigmoid approximation
	float Sigmoid = FastSigmoid(X, 1.0f);
	
	return Sigmoid;
}

float UMGInputResponseCurves::ExponentialCurve(float Input, float Power)
{
	return FastPow(Input, Power);
}

float UMGInputResponseCurves::CustomCurve(float Input, UCurveFloat* Curve)
{
	if (!Curve)
	{
		return Input;
	}

	// Sample the curve at the input value
	// Curves are typically defined from 0-1 on X axis
	return Curve->GetFloatValue(Input);
}

// ==========================================
// PRESETS
// ==========================================

FMGAxisResponseConfig UMGInputResponseCurves::GetCompetitivePreset()
{
	FMGAxisResponseConfig Config;
	Config.CurveType = EMGResponseCurveType::Linear;
	Config.Sensitivity = 1.0f;
	Config.InnerDeadzone = 0.05f; // Minimal deadzone
	Config.OuterDeadzone = 0.02f;
	Config.ExponentPower = 1.0f;
	Config.bInverted = false;
	Config.bCurveBeforeSensitivity = true;
	return Config;
}

FMGAxisResponseConfig UMGInputResponseCurves::GetBalancedPreset()
{
	FMGAxisResponseConfig Config;
	Config.CurveType = EMGResponseCurveType::Progressive;
	Config.Sensitivity = 1.1f;
	Config.InnerDeadzone = 0.10f;
	Config.OuterDeadzone = 0.05f;
	Config.ExponentPower = 1.5f; // Gentle progressive curve
	Config.bInverted = false;
	Config.bCurveBeforeSensitivity = true;
	return Config;
}

FMGAxisResponseConfig UMGInputResponseCurves::GetCasualPreset()
{
	FMGAxisResponseConfig Config;
	Config.CurveType = EMGResponseCurveType::Progressive;
	Config.Sensitivity = 1.3f;
	Config.InnerDeadzone = 0.15f; // Forgiving deadzone
	Config.OuterDeadzone = 0.08f;
	Config.ExponentPower = 2.0f; // Heavy curve for precision
	Config.bInverted = false;
	Config.bCurveBeforeSensitivity = true;
	return Config;
}

FMGAxisResponseConfig UMGInputResponseCurves::GetSimulationPreset()
{
	FMGAxisResponseConfig Config;
	Config.CurveType = EMGResponseCurveType::Linear;
	Config.Sensitivity = 1.0f;
	Config.InnerDeadzone = 0.03f; // Very minimal
	Config.OuterDeadzone = 0.01f;
	Config.ExponentPower = 1.0f;
	Config.bInverted = false;
	Config.bCurveBeforeSensitivity = true;
	return Config;
}

// ==========================================
// UTILITIES
// ==========================================

bool UMGInputResponseCurves::IsInDeadzone(float Input, float Deadzone)
{
	return FMath::Abs(Input) < Deadzone;
}

bool UMGInputResponseCurves::IsInRadialDeadzone(FVector2D Input, float Deadzone)
{
	return Input.Size() < Deadzone;
}

float UMGInputResponseCurves::GetInputMagnitude(FVector2D Input)
{
	return Input.Size();
}

FVector2D UMGInputResponseCurves::NormalizeInput(FVector2D Input)
{
	float Magnitude = Input.Size();
	if (Magnitude > 1.0f)
	{
		return Input / Magnitude;
	}
	return Input;
}

// ==========================================
// FAST APPROXIMATIONS
// ==========================================

float UMGInputResponseCurves::FastPow(float Base, float Exp)
{
	// Use FMath::Pow for now - can optimize later with lookup tables
	// or polynomial approximations if profiling shows this is a bottleneck
	return FMath::Pow(Base, Exp);
}

float UMGInputResponseCurves::FastSigmoid(float X, float Steepness)
{
	// Fast sigmoid approximation: 1 / (1 + e^(-x * steepness))
	// Using tanh for better performance: (tanh(x/2) + 1) / 2
	
	// Even faster approximation for real-time use:
	// f(x) = x / (1 + |x|)  -- maps (-inf, inf) to (-1, 1)
	float Scaled = X * Steepness;
	float Result = Scaled / (1.0f + FMath::Abs(Scaled));
	
	// Remap from [-1, 1] to [0, 1]
	return (Result + 1.0f) * 0.5f;
}

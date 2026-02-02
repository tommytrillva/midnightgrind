// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGInputResponseCurves.h
 * @brief Advanced input response curves for competitive racing feel
 *
 * Provides multiple response curve algorithms to transform raw input into
 * vehicle control values. Different curves suit different driving styles
 * and experience levels.
 */

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "MGInputResponseCurves.generated.h"

/**
 * Pre-defined response curve types
 */
UENUM(BlueprintType)
enum class EMGResponseCurveType : uint8
{
	/** Direct 1:1 mapping - no curve applied */
	Linear			UMETA(DisplayName = "Linear"),
	
	/** Gradual at start, aggressive at end - beginner friendly */
	Progressive		UMETA(DisplayName = "Progressive"),
	
	/** Quick response even at low inputs - for fine control */
	Aggressive		UMETA(DisplayName = "Aggressive"),
	
	/** Smooth S-shaped curve - balanced precision */
	SCurve			UMETA(DisplayName = "S-Curve"),
	
	/** Custom exponential with adjustable power */
	Exponential		UMETA(DisplayName = "Exponential"),
	
	/** Custom curve asset (UCurveFloat) */
	Custom			UMETA(DisplayName = "Custom Curve")
};

/**
 * Deadzone shape types
 */
UENUM(BlueprintType)
enum class EMGDeadzoneShape : uint8
{
	/** Standard axial deadzone (separate X/Y) */
	Axial			UMETA(DisplayName = "Axial"),
	
	/** Radial deadzone (circular, more natural for sticks) */
	Radial			UMETA(DisplayName = "Radial"),
	
	/** Scaled radial (radial with scaling to prevent diagonal clipping) */
	ScaledRadial	UMETA(DisplayName = "Scaled Radial"),
	
	/** Hybrid (radial inner, axial outer) */
	Hybrid			UMETA(DisplayName = "Hybrid")
};

/**
 * Configuration for a single axis response
 */
USTRUCT(BlueprintType)
struct FMGAxisResponseConfig
{
	GENERATED_BODY()

	/** Response curve type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	EMGResponseCurveType CurveType = EMGResponseCurveType::Progressive;

	/** Curve power for exponential mode (1.0 = linear, >1 = progressive, <1 = aggressive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float ExponentPower = 2.0f;

	/** Custom curve asset (used when CurveType = Custom) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	TObjectPtr<UCurveFloat> CustomCurve = nullptr;

	/** Overall sensitivity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensitivity", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float Sensitivity = 1.0f;

	/** Inner deadzone (inputs below this are ignored) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deadzone", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float InnerDeadzone = 0.1f;

	/** Outer deadzone (inputs above 1.0 - this value are clamped to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deadzone", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float OuterDeadzone = 0.05f;

	/** Invert the axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	bool bInverted = false;

	/** Apply curve before or after sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	bool bCurveBeforeSensitivity = true;
};

/**
 * Configuration for 2D input (analog stick)
 */
USTRUCT(BlueprintType)
struct FMG2DInputResponseConfig
{
	GENERATED_BODY()

	/** Deadzone shape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deadzone")
	EMGDeadzoneShape DeadzoneShape = EMGDeadzoneShape::ScaledRadial;

	/** Radial deadzone value (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deadzone", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float RadialDeadzone = 0.15f;

	/** Response configuration for X axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	FMGAxisResponseConfig XAxisConfig;

	/** Response configuration for Y axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	FMGAxisResponseConfig YAxisConfig;

	/** Compensate for diagonal magnitude reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
	bool bCompensateDiagonals = true;
};

/**
 * Keyboard input simulation configuration
 */
USTRUCT(BlueprintType)
struct FMGKeyboardSimulationConfig
{
	GENERATED_BODY()

	/** Time to ramp from 0 to 100% when key pressed (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RampUpTime = 0.1f;

	/** Time to ramp from 100% to 0 when key released (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RampDownTime = 0.08f;

	/** Curve type for ramp up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	EMGResponseCurveType RampUpCurve = EMGResponseCurveType::Progressive;

	/** Curve type for ramp down */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	EMGResponseCurveType RampDownCurve = EMGResponseCurveType::Linear;

	/** When opposite key pressed, snap to center instantly before ramping opposite direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
	bool bInstantReversal = true;

	/** Tap detection threshold (taps shorter than this are gentler) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float TapDetectionTime = 0.15f;

	/** Maximum output value for quick taps (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float TapMaxOutput = 0.6f;
};

/**
 * Utility class for applying response curves
 */
UCLASS()
class MIDNIGHTGRIND_API UMGInputResponseCurves : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==========================================
	// 1D INPUT PROCESSING
	// ==========================================

	/**
	 * Apply complete response curve to 1D input
	 */
	UFUNCTION(BlueprintPure, Category = "Input|ResponseCurves")
	static float ApplyResponseCurve(float RawInput, const FMGAxisResponseConfig& Config);

	/**
	 * Apply specific curve type
	 */
	UFUNCTION(BlueprintPure, Category = "Input|ResponseCurves")
	static float ApplyCurveType(float NormalizedInput, EMGResponseCurveType CurveType, float ExponentPower = 2.0f);

	/**
	 * Apply deadzone (remaps [deadzone, 1-outerDeadzone] to [0, 1])
	 */
	UFUNCTION(BlueprintPure, Category = "Input|ResponseCurves")
	static float ApplyDeadzone(float RawInput, float InnerDeadzone, float OuterDeadzone = 0.0f);

	/**
	 * Apply sensitivity multiplier with clamping
	 */
	UFUNCTION(BlueprintPure, Category = "Input|ResponseCurves")
	static float ApplySensitivity(float Input, float Sensitivity);

	// ==========================================
	// 2D INPUT PROCESSING (ANALOG STICKS)
	// ==========================================

	/**
	 * Apply complete response curve to 2D input (analog stick)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|ResponseCurves")
	static FVector2D Apply2DResponseCurve(FVector2D RawInput, const FMG2DInputResponseConfig& Config);

	/**
	 * Apply radial deadzone
	 */
	UFUNCTION(BlueprintPure, Category = "Input|ResponseCurves")
	static FVector2D ApplyRadialDeadzone(FVector2D Input, float Deadzone, EMGDeadzoneShape Shape);

	/**
	 * Apply scaled radial deadzone (prevents diagonal clipping)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|ResponseCurves")
	static FVector2D ApplyScaledRadialDeadzone(FVector2D Input, float Deadzone);

	// ==========================================
	// CURVE ALGORITHMS
	// ==========================================

	/**
	 * Linear curve (no transformation)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Curves")
	static float LinearCurve(float Input);

	/**
	 * Progressive curve (slow start, fast end)
	 * Good for beginners - more precision at low inputs
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Curves")
	static float ProgressiveCurve(float Input, float Power = 2.0f);

	/**
	 * Aggressive curve (fast start, slow end)
	 * Good for experts - immediate response
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Curves")
	static float AggressiveCurve(float Input, float Power = 0.5f);

	/**
	 * S-Curve (smooth sigmoid)
	 * Balanced - precision in middle, aggressive at edges
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Curves")
	static float SCurve(float Input, float Steepness = 5.0f);

	/**
	 * Exponential curve with custom power
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Curves")
	static float ExponentialCurve(float Input, float Power);

	/**
	 * Custom curve from curve asset
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Curves")
	static float CustomCurve(float Input, UCurveFloat* Curve);

	// ==========================================
	// PRESETS
	// ==========================================

	/**
	 * Get competitive/pro preset (minimal processing, immediate response)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Presets")
	static FMGAxisResponseConfig GetCompetitivePreset();

	/**
	 * Get balanced preset (good for most players)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Presets")
	static FMGAxisResponseConfig GetBalancedPreset();

	/**
	 * Get casual preset (forgiving, assists enabled)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Presets")
	static FMGAxisResponseConfig GetCasualPreset();

	/**
	 * Get simulation/hardcore preset (realistic, no assists)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Presets")
	static FMGAxisResponseConfig GetSimulationPreset();

	// ==========================================
	// UTILITIES
	// ==========================================

	/**
	 * Test if input is within deadzone
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Utility")
	static bool IsInDeadzone(float Input, float Deadzone);

	/**
	 * Test if 2D input is within radial deadzone
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Utility")
	static bool IsInRadialDeadzone(FVector2D Input, float Deadzone);

	/**
	 * Get magnitude of 2D input
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Utility")
	static float GetInputMagnitude(FVector2D Input);

	/**
	 * Normalize 2D input to unit circle (preserves direction, clamps magnitude)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Utility")
	static FVector2D NormalizeInput(FVector2D Input);

private:
	// Fast approximations for performance
	static float FastPow(float Base, float Exp);
	static float FastSigmoid(float X, float Steepness);
};

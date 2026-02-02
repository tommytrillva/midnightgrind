// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGEnhancedInputHandler.h
 * @brief Enhanced vehicle input handler with competitive-level responsiveness
 *
 * This is an improved version of MGVehicleInputHandler that integrates:
 * - Advanced response curves for precise control
 * - Keyboard pseudo-analog simulation
 * - Improved input smoothing
 * - Better controller detection
 * - Frame-perfect input buffering integration
 * - Comprehensive force feedback
 *
 * Use this handler for competitive racing where input precision is critical.
 * Falls back gracefully if advanced features aren't needed.
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "Input/MGVehicleInputHandler.h" // Base class
#include "MGInputResponseCurves.h"
#include "MGKeyboardInputSimulator.h"
#include "MGEnhancedInputHandler.generated.h"

class UMGInputBufferSubsystem;
class UMGRacingWheelSubsystem;

/**
 * Input processing method
 */
UENUM(BlueprintType)
enum class EMGInputProcessingMethod : uint8
{
	/** Direct - No smoothing, immediate response */
	Direct			UMETA(DisplayName = "Direct"),
	
	/** Smoothed - Gentle interpolation */
	Smoothed		UMETA(DisplayName = "Smoothed"),
	
	/** Filtered - Jitter reduction */
	Filtered		UMETA(DisplayName = "Filtered"),
	
	/** Predictive - Slight input prediction */
	Predictive		UMETA(DisplayName = "Predictive")
};

/**
 * Enhanced input configuration
 */
USTRUCT(BlueprintType)
struct FMGEnhancedInputConfig
{
	GENERATED_BODY()

	/** Steering response curve configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response Curves")
	FMGAxisResponseConfig SteeringCurve;

	/** Throttle response curve configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response Curves")
	FMGAxisResponseConfig ThrottleCurve;

	/** Brake response curve configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response Curves")
	FMGAxisResponseConfig BrakeCurve;

	/** Keyboard simulation config */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keyboard")
	FMGKeyboardSimulationConfig KeyboardConfig;

	/** Input processing method */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing")
	EMGInputProcessingMethod ProcessingMethod = EMGInputProcessingMethod::Smoothed;

	/** Smoothing strength (0 = none, 1 = maximum) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Processing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SmoothingStrength = 0.3f;

	/** Use input buffer subsystem for frame-perfect inputs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bUseInputBuffer = true;

	/** Buffer window in seconds (smaller = more precise, harder to execute) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (ClampMin = "0.05", ClampMax = "0.3"))
	float BufferWindowSeconds = 0.1f;

	/** Enable input analytics (for debugging/improvement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bEnableAnalytics = false;
};

/**
 * Input analytics data
 */
USTRUCT(BlueprintType)
struct FMGInputAnalytics
{
	GENERATED_BODY()

	/** Average input smoothness (0-1, higher = smoother) */
	UPROPERTY(BlueprintReadOnly)
	float AverageSmoothness = 0.0f;

	/** Number of steering corrections per second */
	UPROPERTY(BlueprintReadOnly)
	float CorrectionRate = 0.0f;

	/** Average input latency in milliseconds */
	UPROPERTY(BlueprintReadOnly)
	float AverageLatencyMs = 0.0f;

	/** Total inputs processed */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalInputsProcessed = 0;

	/** Peak input value this session */
	UPROPERTY(BlueprintReadOnly)
	float PeakInputValue = 0.0f;
};

/**
 * Enhanced Vehicle Input Handler
 * Competitive-grade input processing for racing games
 */
UCLASS(ClassGroup=(MidnightGrind), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGEnhancedInputHandler : public UMGVehicleInputHandler
{
	GENERATED_BODY()

public:
	UMGEnhancedInputHandler();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * Set enhanced input configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Enhanced")
	void SetEnhancedConfig(const FMGEnhancedInputConfig& NewConfig);

	/**
	 * Get current enhanced configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Enhanced")
	FMGEnhancedInputConfig GetEnhancedConfig() const { return EnhancedConfig; }

	/**
	 * Load preset configuration by name
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Enhanced")
	void LoadPreset(FName PresetName);

	/**
	 * Apply competitive preset (minimal processing, max responsiveness)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Enhanced")
	void ApplyCompetitivePreset();

	/**
	 * Apply balanced preset (good for most players)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Enhanced")
	void ApplyBalancedPreset();

	/**
	 * Apply casual preset (forgiving, assists on)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Enhanced")
	void ApplyCasualPreset();

	// ==========================================
	// ADVANCED INPUT
	// ==========================================

	/**
	 * Get processed input with all enhancements applied
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Enhanced")
	float GetProcessedInput(FName InputName) const;

	/**
	 * Get raw input before processing
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Enhanced")
	float GetRawInput(FName InputName) const;

	/**
	 * Get input from buffer subsystem if available
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Enhanced")
	bool GetBufferedInput(FName InputName, float& OutValue);

	/**
	 * Consume buffered input (marks it as used)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Enhanced")
	bool ConsumeBufferedInput(FName InputName);

	// ==========================================
	// KEYBOARD SIMULATION
	// ==========================================

	/**
	 * Enable/disable keyboard simulation
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	void SetKeyboardSimulationEnabled(bool bEnabled);

	/**
	 * Check if keyboard simulation is active
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Keyboard")
	bool IsKeyboardSimulationEnabled() const { return bKeyboardSimulationEnabled; }

	/**
	 * Get keyboard simulator component
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Keyboard")
	UMGKeyboardInputSimulator* GetKeyboardSimulator() const { return KeyboardSimulator; }

	// ==========================================
	// ANALYTICS
	// ==========================================

	/**
	 * Get input analytics data
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Analytics")
	FMGInputAnalytics GetAnalytics() const { return Analytics; }

	/**
	 * Reset analytics data
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Analytics")
	void ResetAnalytics();

	/**
	 * Enable/disable analytics collection
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Analytics")
	void SetAnalyticsEnabled(bool bEnabled);

	// ==========================================
	// INPUT DEBUGGING
	// ==========================================

	/**
	 * Get input history for visualization (last N frames)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Debug")
	TArray<float> GetInputHistory(FName InputName, int32 FrameCount = 60) const;

	/**
	 * Enable input visualization overlay
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Debug")
	void SetInputVisualizationEnabled(bool bEnabled);

protected:
	// Override base class input handlers to add enhancements
	virtual void OnSteeringInput(const FInputActionValue& Value) override;
	virtual void OnThrottleInput(const FInputActionValue& Value) override;
	virtual void OnBrakeInput(const FInputActionValue& Value) override;

	// Enhanced processing
	float ProcessSteeringInput(float RawInput, float DeltaTime);
	float ProcessThrottleInput(float RawInput, float DeltaTime);
	float ProcessBrakeInput(float RawInput, float DeltaTime);

	// Smoothing algorithms
	float ApplySmoothing(float RawInput, float PreviousInput, float SmoothingFactor, float DeltaTime);
	float ApplyFiltering(float RawInput, TArray<float>& History, int32 WindowSize);
	float ApplyPrediction(float RawInput, TArray<float>& History);

	// Analytics
	void UpdateAnalytics(float DeltaTime);
	void RecordInputForAnalytics(FName InputName, float Value);

	// Input history for analytics
	void RecordInputHistory(FName InputName, float Value);

private:
	/** Enhanced configuration */
	UPROPERTY(EditAnywhere, Category = "Config")
	FMGEnhancedInputConfig EnhancedConfig;

	/** Keyboard input simulator */
	UPROPERTY()
	TObjectPtr<UMGKeyboardInputSimulator> KeyboardSimulator;

	/** Input buffer subsystem reference */
	UPROPERTY()
	TObjectPtr<UMGInputBufferSubsystem> InputBufferSubsystem;

	/** Keyboard simulation enabled */
	UPROPERTY()
	bool bKeyboardSimulationEnabled = false;

	/** Input analytics */
	UPROPERTY()
	FMGInputAnalytics Analytics;

	/** Raw input values (before processing) */
	UPROPERTY()
	TMap<FName, float> RawInputValues;

	/** Processed input values (after curves/smoothing) */
	UPROPERTY()
	TMap<FName, float> ProcessedInputValues;

	/** Input history for smoothing/prediction (circular buffers) */
	UPROPERTY()
	TMap<FName, TArray<float>> InputHistories;

	/** Maximum history size per input */
	static constexpr int32 MaxHistorySize = 120; // 2 seconds at 60fps

	/** Input visualization enabled */
	bool bInputVisualizationEnabled = false;

	/** Time accumulator for analytics */
	float AnalyticsTimeAccumulator = 0.0f;
};

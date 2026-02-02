// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGKeyboardInputSimulator.h
 * @brief Converts binary keyboard inputs into smooth analog-like values
 *
 * Keyboard controls are digital (on/off), but racing games need analog control.
 * This system simulates analog behavior by smoothly ramping inputs up/down,
 * detecting taps vs holds, and handling quick reversals naturally.
 *
 * Goal: Make keyboard feel as responsive as gamepad for competitive play.
 */

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MGInputResponseCurves.h"
#include "MGKeyboardInputSimulator.generated.h"

/**
 * Keyboard input channel state
 * Tracks the state of a single axis being controlled by keyboard
 */
USTRUCT(BlueprintType)
struct FMGKeyboardChannelState
{
	GENERATED_BODY()

	/** Current output value (0-1 or -1 to 1) */
	UPROPERTY(BlueprintReadOnly)
	float CurrentValue = 0.0f;

	/** Target value being ramped toward */
	UPROPERTY(BlueprintReadOnly)
	float TargetValue = 0.0f;

	/** Time current key has been held */
	UPROPERTY(BlueprintReadOnly)
	float HoldTime = 0.0f;

	/** Positive key currently pressed */
	UPROPERTY(BlueprintReadOnly)
	bool bPositivePressed = false;

	/** Negative key currently pressed */
	UPROPERTY(BlueprintReadOnly)
	bool bNegativePressed = false;

	/** Last press was a quick tap */
	UPROPERTY(BlueprintReadOnly)
	bool bWasTap = false;

	/** Time since last state change */
	UPROPERTY(BlueprintReadOnly)
	float TimeSinceChange = 0.0f;

	/** Ramping up or down */
	UPROPERTY(BlueprintReadOnly)
	bool bRampingUp = false;
};

/**
 * Keyboard input simulator
 * Converts keyboard on/off inputs into smooth analog values
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGKeyboardInputSimulator : public UObject
{
	GENERATED_BODY()

public:
	UMGKeyboardInputSimulator();

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * Set keyboard simulation configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	void SetConfiguration(const FMGKeyboardSimulationConfig& NewConfig);

	/**
	 * Get current configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Keyboard")
	FMGKeyboardSimulationConfig GetConfiguration() const { return Config; }

	// ==========================================
	// INPUT PROCESSING
	// ==========================================

	/**
	 * Update a single-ended channel (like throttle/brake, 0-1)
	 * @param bKeyPressed - Is the key currently held down
	 * @param DeltaTime - Frame delta time
	 * @param ChannelName - Unique identifier for this channel
	 * @return Current smoothed output value (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	float UpdateSingleChannel(bool bKeyPressed, float DeltaTime, FName ChannelName);

	/**
	 * Update a dual-ended channel (like steering, -1 to 1)
	 * @param bPositivePressed - Is positive key pressed (e.g., D for right)
	 * @param bNegativePressed - Is negative key pressed (e.g., A for left)
	 * @param DeltaTime - Frame delta time
	 * @param ChannelName - Unique identifier for this channel
	 * @return Current smoothed output value (-1 to 1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	float UpdateDualChannel(bool bPositivePressed, bool bNegativePressed, float DeltaTime, FName ChannelName);

	/**
	 * Manually reset a channel to zero (for mode switching, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	void ResetChannel(FName ChannelName);

	/**
	 * Reset all channels
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	void ResetAllChannels();

	// ==========================================
	// QUERY
	// ==========================================

	/**
	 * Get current value of a channel
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Keyboard")
	float GetChannelValue(FName ChannelName) const;

	/**
	 * Get channel state info
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Keyboard")
	FMGKeyboardChannelState GetChannelState(FName ChannelName) const;

	/**
	 * Is a channel currently ramping
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Keyboard")
	bool IsChannelRamping(FName ChannelName) const;

	/**
	 * How far is channel from target (0 = at target, 1 = max distance)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Keyboard")
	float GetChannelRampProgress(FName ChannelName) const;

	// ==========================================
	// ADVANCED
	// ==========================================

	/**
	 * Set custom ramp speeds for a specific channel (overrides config)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	void SetChannelRampSpeed(FName ChannelName, float RampUpTime, float RampDownTime);

	/**
	 * Clear custom ramp speeds (revert to config defaults)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	void ClearChannelRampSpeed(FName ChannelName);

	/**
	 * Enable/disable tap detection for a channel
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Keyboard")
	void SetTapDetectionEnabled(FName ChannelName, bool bEnabled);

protected:
	/** Configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FMGKeyboardSimulationConfig Config;

	/** Channel states */
	UPROPERTY()
	TMap<FName, FMGKeyboardChannelState> Channels;

	/** Per-channel custom ramp speeds (optional overrides) */
	UPROPERTY()
	TMap<FName, FVector2D> CustomRampSpeeds; // X = RampUp, Y = RampDown

	/** Per-channel tap detection enable state */
	UPROPERTY()
	TMap<FName, bool> TapDetectionEnabled;

	// Internal helpers
	FMGKeyboardChannelState& GetOrCreateChannel(FName ChannelName);
	float RampValue(float Current, float Target, float RampTime, float DeltaTime, bool bRampUp) const;
	bool IsTap(float HoldTime) const;
	float GetTapTargetValue(float HoldTime) const;
	float ApplyRampCurve(float T, EMGResponseCurveType CurveType) const;
};

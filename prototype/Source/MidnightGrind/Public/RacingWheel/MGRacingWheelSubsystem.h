// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dom/JsonObject.h"
#include "MGRacingWheelTypes.h"
#include "MGRacingWheelSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRacingWheel, Log, All);

class UMGWheelFFBProcessor;
class FMGDirectInputManager;

/**
 * Racing Wheel Subsystem
 *
 * Central manager for racing wheel and specialty controller support.
 * Handles device detection, input processing, force feedback, and profile management.
 *
 * Features:
 * - DirectInput device enumeration and connection management
 * - Support for Logitech, Thrustmaster, and Fanatec wheels
 * - Full force feedback effect support (constant, spring, damper, periodic)
 * - Per-wheel profile configuration
 * - Hot-plug support
 * - High-level FFB methods for common racing scenarios
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRacingWheelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// DEVICE MANAGEMENT
	// ==========================================

	/**
	 * Scan for connected racing wheels
	 * @return Number of wheels found
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Device")
	int32 ScanForWheels();

	/**
	 * Check if a racing wheel is currently connected
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Device")
	bool IsWheelConnected() const;

	/**
	 * Get the connected wheel model
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Device")
	EMGWheelModel GetConnectedWheelModel() const;

	/**
	 * Get the connected wheel's capabilities
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Device")
	FMGWheelCapabilities GetWheelCapabilities() const;

	/**
	 * Get the current wheel input state
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Device")
	FMGWheelState GetWheelState() const;

	/**
	 * Get connection state
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Device")
	EMGWheelConnectionState GetConnectionState() const;

	/**
	 * Manually disconnect the wheel (for testing)
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Device")
	void DisconnectWheel();

	// ==========================================
	// INPUT
	// ==========================================

	/**
	 * Get processed steering input (-1 to 1)
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Input")
	float GetSteeringInput() const;

	/**
	 * Get processed throttle input (0 to 1)
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Input")
	float GetThrottleInput() const;

	/**
	 * Get processed brake input (0 to 1)
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Input")
	float GetBrakeInput() const;

	/**
	 * Get processed clutch input (0 to 1)
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Input")
	float GetClutchInput() const;

	/**
	 * Check if a wheel button is pressed
	 * @param ButtonIndex Button index (0-based)
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Input")
	bool IsButtonPressed(int32 ButtonIndex) const;

	/**
	 * Check if left paddle shifter was pressed this frame
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Input")
	bool WasShiftDownPressed() const;

	/**
	 * Check if right paddle shifter was pressed this frame
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Input")
	bool WasShiftUpPressed() const;

	// ==========================================
	// FORCE FEEDBACK - LOW LEVEL
	// ==========================================

	/**
	 * Play a force feedback effect
	 * @param Effect Effect parameters
	 * @return Effect ID for controlling playback
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB")
	FGuid PlayFFBEffect(const FMGFFBEffect& Effect);

	/**
	 * Update an existing effect's parameters
	 * @param EffectID Effect to update
	 * @param Effect New parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB")
	void UpdateFFBEffect(FGuid EffectID, const FMGFFBEffect& Effect);

	/**
	 * Stop a specific effect
	 * @param EffectID Effect to stop
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB")
	void StopFFBEffect(FGuid EffectID);

	/**
	 * Stop all force feedback effects
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB")
	void StopAllFFBEffects();

	/**
	 * Pause/unpause an effect
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB")
	void SetFFBEffectPaused(FGuid EffectID, bool bPaused);

	/**
	 * Set the global FFB gain (master volume)
	 * @param Gain 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB")
	void SetFFBGlobalGain(float Gain);

	/**
	 * Enable or disable all FFB
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB")
	void SetFFBEnabled(bool bEnabled);

	/**
	 * Check if FFB is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|FFB")
	bool IsFFBEnabled() const;

	// ==========================================
	// FORCE FEEDBACK - HIGH LEVEL GAMEPLAY
	// ==========================================

	/**
	 * Update vehicle physics data for FFB calculation
	 * Call this every frame from the vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB|Gameplay")
	void UpdateFFBFromVehicle(const FMGFFBInputData& VehicleData);

	/**
	 * Trigger a collision impact effect
	 * @param Force Impact force (normalized 0-1)
	 * @param Direction Local-space impact direction
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB|Gameplay")
	void TriggerCollisionFFB(float Force, FVector Direction);

	/**
	 * Trigger a kerb/rumble strip effect
	 * @param Intensity Effect intensity (0-1)
	 * @param Duration How long to play (-1 = until stopped)
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB|Gameplay")
	FGuid TriggerKerbFFB(float Intensity, float Duration = -1.0f);

	/**
	 * Trigger surface change effect (gravel, dirt, etc.)
	 * @param SurfaceType Surface name
	 * @param Intensity Base intensity
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB|Gameplay")
	FGuid TriggerSurfaceFFB(FName SurfaceType, float Intensity);

	/**
	 * Update engine vibration effect
	 * @param RPMPercent Current RPM as percentage of max
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB|Gameplay")
	void UpdateEngineFFB(float RPMPercent);

	/**
	 * Set the self-centering spring parameters
	 * @param Strength Spring strength (0-1)
	 * @param Coefficient Spring coefficient (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB|Gameplay")
	void SetSelfCentering(float Strength, float Coefficient);

	/**
	 * Set the steering damper strength
	 * @param Strength Damper coefficient (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|FFB|Gameplay")
	void SetDamperStrength(float Strength);

	// ==========================================
	// PROFILES
	// ==========================================

	/**
	 * Load a wheel profile
	 * @param ProfileName Name of the profile to load
	 * @return True if loaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Profile")
	bool LoadProfile(const FString& ProfileName);

	/**
	 * Save the current profile
	 * @param ProfileName Name to save as
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Profile")
	void SaveProfile(const FString& ProfileName);

	/**
	 * Get the current active profile
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Profile")
	FMGWheelProfile GetCurrentProfile() const;

	/**
	 * Set the current profile
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Profile")
	void SetCurrentProfile(const FMGWheelProfile& Profile);

	/**
	 * Get list of available profile names
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Profile")
	TArray<FString> GetAvailableProfiles() const;

	/**
	 * Load the default profile for the connected wheel
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Profile")
	void LoadDefaultProfileForWheel();

	/**
	 * Reset profile to defaults
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Profile")
	void ResetProfileToDefaults();

	// ==========================================
	// CALIBRATION
	// ==========================================

	/**
	 * Start wheel calibration mode
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Calibration")
	void StartCalibration();

	/**
	 * Finish calibration and save results
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Calibration")
	void FinishCalibration();

	/**
	 * Cancel calibration
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Calibration")
	void CancelCalibration();

	/**
	 * Check if in calibration mode
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Calibration")
	bool IsCalibrating() const;

	/**
	 * Set the wheel center point (during calibration)
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingWheel|Calibration")
	void SetWheelCenter();

	// ==========================================
	// DIAGNOSTICS
	// ==========================================

	/**
	 * Get FFB clipping amount (0-1, 0 = no clipping)
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Diagnostics")
	float GetFFBClippingAmount() const;

	/**
	 * Get raw axis values for debugging
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Diagnostics")
	void GetRawAxisValues(int32& OutSteering, int32& OutThrottle, int32& OutBrake, int32& OutClutch) const;

	/**
	 * Get FFB latency in milliseconds
	 */
	UFUNCTION(BlueprintPure, Category = "RacingWheel|Diagnostics")
	float GetFFBLatencyMs() const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when a wheel is connected */
	UPROPERTY(BlueprintAssignable, Category = "RacingWheel|Events")
	FOnWheelConnected OnWheelConnected;

	/** Called when a wheel is disconnected */
	UPROPERTY(BlueprintAssignable, Category = "RacingWheel|Events")
	FOnWheelDisconnected OnWheelDisconnected;

	/** Called when wheel input state is updated */
	UPROPERTY(BlueprintAssignable, Category = "RacingWheel|Events")
	FOnWheelStateUpdated OnWheelStateUpdated;

	/** Called when FFB is clipping */
	UPROPERTY(BlueprintAssignable, Category = "RacingWheel|Events")
	FOnFFBClipping OnFFBClipping;

protected:
	/** Tick function for input polling */
	void OnWheelTick();

	/** Initialize the DirectInput manager */
	void InitializeDirectInput();

	/** Shutdown DirectInput */
	void ShutdownDirectInput();

	/** Process raw input and apply deadzone/curves */
	void ProcessInput();

	/** Check for wheel connection changes */
	void CheckConnectionState();

	/** Initialize the known wheel database */
	void InitializeKnownWheelDatabase();

	/** Get wheel info from VID/PID */
	FMGKnownWheelEntry GetKnownWheelInfo(int32 VendorID, int32 ProductID) const;

	/** Load profiles from disk */
	void LoadProfilesFromDisk();

	/** Save profiles to disk */
	void SaveProfilesToDisk();

	/** Load a single profile from JSON */
	void LoadProfileFromJson(const TSharedPtr<FJsonObject>& JsonObject, FMGWheelProfile& OutProfile);

	/** Get profile file path */
	FString GetProfilePath() const;

private:
	/** DirectInput manager (Windows only) */
#if PLATFORM_WINDOWS
	TSharedPtr<FMGDirectInputManager> DirectInputManager;
#endif

	/** FFB processor */
	UPROPERTY()
	TObjectPtr<UMGWheelFFBProcessor> FFBProcessor;

	/** Current wheel state */
	FMGWheelState CurrentWheelState;

	/** Processed input values */
	float ProcessedSteering = 0.0f;
	float ProcessedThrottle = 0.0f;
	float ProcessedBrake = 0.0f;
	float ProcessedClutch = 0.0f;

	/** Previous frame paddle states for edge detection */
	bool bPrevLeftPaddle = false;
	bool bPrevRightPaddle = false;
	bool bShiftDownThisFrame = false;
	bool bShiftUpThisFrame = false;

	/** Connection state */
	EMGWheelConnectionState ConnectionState = EMGWheelConnectionState::Disconnected;

	/** Connected wheel info */
	FMGWheelCapabilities ConnectedWheelCapabilities;
	EMGWheelModel ConnectedWheelModel = EMGWheelModel::Unknown;

	/** Current profile */
	FMGWheelProfile CurrentProfile;

	/** Available profiles */
	TMap<FString, FMGWheelProfile> AvailableProfiles;

	/** Known wheel database */
	TArray<FMGKnownWheelEntry> KnownWheelDatabase;

	/** FFB enabled state */
	bool bFFBEnabled = true;

	/** Global FFB gain */
	float GlobalFFBGain = 1.0f;

	/** FFB clipping tracking */
	float CurrentFFBClipping = 0.0f;

	/** Calibration state */
	bool bIsCalibrating = false;
	int32 CalibrationCenterOffset = 0;

	/** Tick timer */
	FTimerHandle WheelTickHandle;

	/** Active FFB effect IDs */
	FGuid SelfCenteringEffectID;
	FGuid DamperEffectID;
	FGuid SurfaceEffectID;
	FGuid EngineEffectID;

	/** Last FFB update time for latency tracking */
	double LastFFBUpdateTime = 0.0;
	float FFBLatencyMs = 0.0f;
};

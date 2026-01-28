// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGRacingWheelSubsystem.h
 * =============================================================================
 *
 * PURPOSE:
 * This is the main interface for racing wheel support in Midnight Grind.
 * It handles EVERYTHING related to racing wheel hardware:
 * - Detecting when wheels are connected/disconnected
 * - Reading input (steering, pedals, buttons)
 * - Sending force feedback effects to the wheel
 * - Managing user profiles and settings
 *
 * WHAT IS A SUBSYSTEM?
 * In Unreal Engine, a Subsystem is a singleton-like object that lives
 * alongside a specific "outer" object. This is a GameInstanceSubsystem,
 * meaning:
 * - ONE instance exists per game instance
 * - It's created automatically when the game starts
 * - It persists across level loads (unlike Actors)
 * - You access it via UGameInstance::GetSubsystem<UMGRacingWheelSubsystem>()
 *
 * WHY A SUBSYSTEM (not an Actor or Component)?
 * - Wheels are hardware, not game objects - they exist outside any level
 * - We need it to persist across level transitions
 * - We want exactly one manager for all wheel operations
 * - Subsystems are the modern Unreal way to do singletons
 *
 * HOW TO USE THIS (for vehicle developers):
 *
 * 1. GET THE SUBSYSTEM:
 *    UMGRacingWheelSubsystem* WheelSub = GetGameInstance()->GetSubsystem<UMGRacingWheelSubsystem>();
 *
 * 2. CHECK FOR WHEEL:
 *    if (WheelSub && WheelSub->IsWheelConnected()) { ... }
 *
 * 3. READ INPUT:
 *    float Steering = WheelSub->GetSteeringInput();  // -1 to 1
 *    float Throttle = WheelSub->GetThrottleInput();  // 0 to 1
 *    float Brake = WheelSub->GetBrakeInput();        // 0 to 1
 *
 * 4. SEND FFB (every frame from your vehicle tick):
 *    FMGFFBInputData Data;
 *    Data.SpeedKmh = Vehicle->GetSpeed();
 *    Data.FrontSlipAngle = Vehicle->GetFrontSlipAngle();
 *    // ... fill other fields ...
 *    WheelSub->UpdateFFBFromVehicle(Data);
 *
 * ARCHITECTURE OVERVIEW:
 *
 *   [Your Vehicle Code]
 *         |
 *         | Calls GetSteeringInput(), UpdateFFBFromVehicle()
 *         v
 *   [UMGRacingWheelSubsystem] <-- This file
 *         |
 *         | Uses for FFB calculations
 *         v
 *   [UMGWheelFFBProcessor]
 *         |
 *         | Sends low-level commands via
 *         v
 *   [FMGDirectInputManager] (Windows only)
 *         |
 *         | DirectInput API calls
 *         v
 *   [Racing Wheel Hardware]
 *
 * PLATFORM SUPPORT:
 * Currently Windows-only (DirectInput). The architecture is designed to
 * support additional platforms by implementing alternative input managers
 * (e.g., SDL2 for cross-platform, or platform-specific APIs).
 *
 * THREADING NOTES:
 * - Input reading happens on the game thread
 * - FFB updates are batched and sent at a controlled rate
 * - DirectInput callbacks may come from other threads (handled internally)
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dom/JsonObject.h"
#include "MGRacingWheelTypes.h"
#include "MGRacingWheelSubsystem.generated.h"

// Log category for wheel-related messages. Use: UE_LOG(LogRacingWheel, Log, TEXT("..."))
DECLARE_LOG_CATEGORY_EXTERN(LogRacingWheel, Log, All);

// Forward declarations - these classes are defined elsewhere
class UMGWheelFFBProcessor;  // Handles FFB force calculations
class FMGDirectInputManager; // Windows DirectInput interface (non-UObject, hence F prefix)

/**
 * Racing Wheel Subsystem
 *
 * Central manager for racing wheel and specialty controller support.
 * Handles device detection, input processing, force feedback, and profile management.
 *
 * See file header comment for detailed usage instructions.
 *
 * KEY FEATURES:
 * - DirectInput device enumeration and connection management
 * - Support for Logitech, Thrustmaster, and Fanatec wheels
 * - Full force feedback effect support (constant, spring, damper, periodic)
 * - Per-wheel profile configuration with save/load
 * - Hot-plug support (connect/disconnect during gameplay)
 * - High-level FFB methods for common racing scenarios (collisions, kerbs, etc.)
 * - Calibration system for user adjustment
 * - Diagnostic tools for FFB clipping and latency
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRacingWheelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// SUBSYSTEM LIFECYCLE
	// ==========================================
	// These are Unreal Engine subsystem overrides - called automatically by the engine.
	// You don't need to call these yourself.

	/** Called when the subsystem is created. Sets up DirectInput and starts scanning for wheels. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called when the subsystem is destroyed. Cleans up DirectInput and releases wheel. */
	virtual void Deinitialize() override;

	/** Returns true if this subsystem should be created. We only create on Windows (DirectInput support). */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// DEVICE MANAGEMENT
	// ==========================================
	// Functions for detecting and managing connected wheel hardware.

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
	// Functions for reading processed input values from the wheel.
	// These values have deadzone and sensitivity curves applied.
	// Call these every frame from your vehicle tick to get player input.

	/**
	 * Get processed steering input (-1 to 1)
	 * @return Negative = left, Positive = right, Zero = center
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
	// Direct control over FFB effects. Use these when you need precise control
	// over specific effects, or for custom effects not covered by the high-level API.
	//
	// TYPICAL WORKFLOW:
	// 1. Create an FMGFFBEffect and configure it
	// 2. Call PlayFFBEffect() - save the returned GUID
	// 3. Use the GUID to update, pause, or stop the effect
	// 4. Effects with finite duration stop automatically

	/**
	 * Play a force feedback effect
	 * @param Effect Effect parameters (see FMGFFBEffect for options)
	 * @return Effect ID (GUID) for controlling playback later
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
	// These are the functions you'll use most often. They handle common
	// racing game scenarios without needing to understand low-level FFB.
	//
	// The main function is UpdateFFBFromVehicle() - call it every frame
	// with your vehicle's physics state, and the system handles everything.
	//
	// The other functions (TriggerCollisionFFB, TriggerKerbFFB, etc.) are
	// for one-shot events that need immediate feedback.

	/**
	 * Update vehicle physics data for FFB calculation
	 *
	 * THIS IS THE MAIN FUNCTION FOR VEHICLE FFB.
	 * Call it every frame from your vehicle's Tick, passing current physics state.
	 * The FFB processor will calculate appropriate forces based on:
	 * - Self-centering (speed-dependent)
	 * - Tire grip and slip (the "feel" of the limit)
	 * - Weight transfer (cornering forces)
	 * - Understeer/oversteer feedback
	 *
	 * @param VehicleData Current vehicle physics state (see FMGFFBInputData)
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
	// Profile management for saving/loading user preferences.
	//
	// Profiles are saved as JSON files in the game's save directory.
	// Players can have multiple profiles for different wheels or preferences.
	// The system automatically loads a matching profile when a wheel connects.

	/**
	 * Load a wheel profile from disk
	 * @param ProfileName Name of the profile to load (without extension)
	 * @return True if loaded successfully, false if profile doesn't exist
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
	// Wheel calibration allows players to set the center point and limits.
	//
	// TYPICAL CALIBRATION FLOW:
	// 1. UI calls StartCalibration()
	// 2. Prompt user: "Center the wheel and press a button"
	// 3. Call SetWheelCenter() when they confirm
	// 4. Optionally: "Turn fully left, then right" for range calibration
	// 5. Call FinishCalibration() to save, or CancelCalibration() to discard
	//
	// Calibration data is stored in the profile and persists across sessions.

	/**
	 * Start wheel calibration mode
	 * While calibrating, normal input processing is paused.
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
	// Tools for debugging and displaying wheel status to players.
	//
	// FFB CLIPPING:
	// When the calculated FFB force exceeds the wheel's physical capability,
	// it "clips" - the wheel can't produce more force. This reduces feel
	// quality because nuanced forces get lost. Show a warning when this happens
	// so players know to reduce FFB strength.

	/**
	 * Get FFB clipping amount (0-1, 0 = no clipping, 1 = fully clipped)
	 * Use this to display a clipping indicator in your HUD.
	 * If frequently > 0, player should reduce FFB strength in settings.
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
	// EVENTS (Delegates)
	// ==========================================
	// Blueprint-assignable events for wheel state changes.
	//
	// HOW TO USE IN BLUEPRINT:
	// 1. Get a reference to this subsystem
	// 2. Drag off the event (e.g., "On Wheel Connected")
	// 3. Select "Bind" to connect your handler
	//
	// HOW TO USE IN C++:
	// WheelSubsystem->OnWheelConnected.AddDynamic(this, &UMyClass::HandleWheelConnected);
	//
	// Remember to unbind in your class's destructor or EndPlay!

	/** Called when a wheel is connected - show wheel UI, enable wheel controls */
	UPROPERTY(BlueprintAssignable, Category = "RacingWheel|Events")
	FOnWheelConnected OnWheelConnected;

	/** Called when a wheel is disconnected - show gamepad prompt, disable wheel features */
	UPROPERTY(BlueprintAssignable, Category = "RacingWheel|Events")
	FOnWheelDisconnected OnWheelDisconnected;

	/** Called when wheel input state is updated (every frame) - WARNING: high frequency! */
	UPROPERTY(BlueprintAssignable, Category = "RacingWheel|Events")
	FOnWheelStateUpdated OnWheelStateUpdated;

	/** Called when FFB forces clip - use to show visual indicator to player */
	UPROPERTY(BlueprintAssignable, Category = "RacingWheel|Events")
	FOnFFBClipping OnFFBClipping;

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================
	// These are implementation details - not exposed to Blueprint.
	// You generally don't need to call these directly.

	/** Called on timer to poll wheel state. Runs at high frequency (typically 125-1000Hz). */
	void OnWheelTick();

	/** Initialize the DirectInput manager and enumerate devices (Windows only) */
	void InitializeDirectInput();

	/** Shutdown DirectInput and release all resources */
	void ShutdownDirectInput();

	/** Process raw input values: apply deadzone, sensitivity curves, and normalization */
	void ProcessInput();

	/** Check for wheel connect/disconnect events (hot-plug handling) */
	void CheckConnectionState();

	/** Build the database of known wheel VID/PID mappings */
	void InitializeKnownWheelDatabase();

	/** Look up a wheel in our database by USB VID/PID. Returns Unknown if not found. */
	FMGKnownWheelEntry GetKnownWheelInfo(int32 VendorID, int32 ProductID) const;

	/** Load all saved profiles from the JSON file on disk */
	void LoadProfilesFromDisk();

	/** Save all profiles to the JSON file on disk */
	void SaveProfilesToDisk();

	/** Parse a single profile from JSON into a struct */
	void LoadProfileFromJson(const TSharedPtr<FJsonObject>& JsonObject, FMGWheelProfile& OutProfile);

	/** Get the file path where profiles are saved (in Saved/Config directory) */
	FString GetProfilePath() const;

private:
	// ==========================================
	// PRIVATE MEMBER VARIABLES
	// ==========================================
	// Internal state that drives the subsystem. These are implementation
	// details and should not be accessed directly from outside code.

	// --- Platform-Specific Input Layer ---

	/** DirectInput manager - handles Windows-specific USB communication
	 *  Wrapped in platform check so the code compiles on non-Windows platforms.
	 *  On other platforms, this would be replaced with SDL2 or platform API. */
#if PLATFORM_WINDOWS
	TSharedPtr<FMGDirectInputManager> DirectInputManager;
#endif

	// --- FFB Processing ---

	/** FFB processor - calculates forces from vehicle physics.
	 *  This is a UObject so it participates in garbage collection.
	 *  UPROPERTY() ensures the GC knows about this reference. */
	UPROPERTY()
	TObjectPtr<UMGWheelFFBProcessor> FFBProcessor;

	// --- Input State ---

	/** Raw wheel state from hardware (before processing) */
	FMGWheelState CurrentWheelState;

	/** Processed input values (after deadzone, curves, etc.)
	 *  These are what GetSteeringInput() etc. return. */
	float ProcessedSteering = 0.0f;
	float ProcessedThrottle = 0.0f;
	float ProcessedBrake = 0.0f;
	float ProcessedClutch = 0.0f;

	/** Edge detection for paddle shifters - we need to track previous frame
	 *  to detect the moment they're pressed (not just "is pressed"). */
	bool bPrevLeftPaddle = false;
	bool bPrevRightPaddle = false;
	bool bShiftDownThisFrame = false;  // True only on the frame the paddle was pressed
	bool bShiftUpThisFrame = false;

	// --- Connection State ---

	/** Current connection state machine state */
	EMGWheelConnectionState ConnectionState = EMGWheelConnectionState::Disconnected;

	/** Information about the connected wheel (populated on connect) */
	FMGWheelCapabilities ConnectedWheelCapabilities;
	EMGWheelModel ConnectedWheelModel = EMGWheelModel::Unknown;

	// --- Profile System ---

	/** Currently active profile (being used for input processing and FFB) */
	FMGWheelProfile CurrentProfile;

	/** All loaded profiles, keyed by profile name */
	TMap<FString, FMGWheelProfile> AvailableProfiles;

	/** Database of known wheels (VID/PID to model mapping) */
	TArray<FMGKnownWheelEntry> KnownWheelDatabase;

	// --- FFB State ---

	/** Master FFB on/off switch */
	bool bFFBEnabled = true;

	/** Master gain applied to all FFB effects (0-1) */
	float GlobalFFBGain = 1.0f;

	/** Current amount of clipping (0 = none, 1 = fully clipped) */
	float CurrentFFBClipping = 0.0f;

	// --- Calibration ---

	/** True when in calibration mode */
	bool bIsCalibrating = false;

	/** Stored center offset from calibration (raw axis units) */
	int32 CalibrationCenterOffset = 0;

	// --- Timing ---

	/** Timer handle for the high-frequency input polling tick */
	FTimerHandle WheelTickHandle;

	// --- Persistent Effect IDs ---
	// Some effects run continuously and need to be tracked for updates.
	// These GUIDs identify the current instances of each effect type.

	FGuid SelfCenteringEffectID;  // Spring effect for self-centering
	FGuid DamperEffectID;         // Damper effect for smoothing
	FGuid SurfaceEffectID;        // Periodic effect for road surface
	FGuid EngineEffectID;         // Periodic effect for engine vibration

	// --- Performance Monitoring ---

	/** Timestamp of last FFB update (for latency calculation) */
	double LastFFBUpdateTime = 0.0;

	/** Measured FFB latency in milliseconds */
	float FFBLatencyMs = 0.0f;
};

// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGInputRemapSubsystem.h
 * @brief Complete input remapping system with accessibility features and wheel support
 *
 * =============================================================================
 * OVERVIEW
 * =============================================================================
 *
 * The Input Remap Subsystem provides comprehensive control customization for
 * Midnight Grind. Players can remap any action to any key/button, choose from
 * preset control schemes, adjust sensitivity settings, and enable accessibility
 * assists. The system also supports racing wheels with force feedback.
 *
 * =============================================================================
 * @section concepts KEY CONCEPTS FOR BEGINNERS
 * =============================================================================
 *
 * 1. INPUT ACTIONS (EMGInputAction):
 *    - Game actions that can be bound to keys/buttons.
 *    - Examples: Accelerate, Brake, SteerLeft, Nitro, Handbrake.
 *    - Each action can have a primary key, secondary key, AND gamepad binding.
 *    - This allows keyboard+gamepad players to have both working.
 *
 * 2. INPUT BINDINGS (FMGInputBinding):
 *    - Maps an action to physical inputs.
 *    - PrimaryKey: Main keyboard/mouse binding (e.g., W for accelerate).
 *    - SecondaryKey: Alternate binding (e.g., Up Arrow).
 *    - GamepadKey: Controller binding (e.g., Right Trigger).
 *    - AxisScale: Multiplier for analog inputs (1.0 = normal).
 *    - DeadZone: Minimum input before registering (avoids stick drift).
 *    - bInvertAxis: Flip the input direction.
 *
 * 3. CONTROL SCHEMES (EMGControlScheme):
 *    - Preset binding configurations for quick setup.
 *    - Default: Standard racing game layout.
 *    - Alternate: Alternative button layout.
 *    - Racing: Optimized for racing (triggers for gas/brake).
 *    - Casual: Simplified controls.
 *    - OneHanded_Left/Right: Accessibility for single-hand play.
 *    - Custom1/2/3: Player-saved custom schemes.
 *
 * 4. CONTROL PROFILES (FMGControlProfile):
 *    - Complete settings package including bindings and preferences.
 *    - SteeringSensitivity: How responsive steering is.
 *    - TriggerDeadZone/StickDeadZone: Input thresholds.
 *    - VibrationEnabled/VibrationIntensity: Haptic feedback settings.
 *    - bSwapSticksEnabled: Swap left/right stick functions.
 *    - bSwapTriggersEnabled: Swap trigger functions.
 *
 * 5. DRIVING ASSISTS (FMGDrivingAssists):
 *    - Accessibility features that help players drive.
 *    - bAutoAccelerate: Vehicle accelerates automatically.
 *    - bSteeringAssist: Helps keep the car on track.
 *    - bBrakingAssist: Automatic braking for corners.
 *    - bAutoShift: Automatic transmission.
 *    - bTractionControl: Prevents wheel spin.
 *    - bStabilityControl: Prevents spinouts.
 *    - bAntiLockBrakes: Prevents wheel lockup.
 *    - bSimplifiedControls: Reduced input complexity.
 *
 * 6. RACING WHEEL SUPPORT (FMGWheelSettings):
 *    - Settings for racing wheel peripherals.
 *    - SteeringRotation: Wheel rotation range (900, 1080 degrees).
 *    - SteeringLinearity: Center precision curve.
 *    - Pedal deadzones: Throttle, Brake, Clutch individually.
 *    - Force Feedback settings: Self-centering, road feel, collisions.
 *    - bCombinedPedals: Support for older wheels with combined axis.
 *
 * 7. INPUT DEVICE DETECTION:
 *    - System auto-detects which device the player is using.
 *    - EMGInputDevice: Keyboard, Gamepad, Wheel, Touch.
 *    - UI can change prompts based on GetActiveDevice().
 *    - SetPreferredDevice() lets player choose their default.
 *
 * 8. PERSISTENCE:
 *    - Bindings are automatically saved and loaded.
 *    - ExportBindingsToString() creates shareable config text.
 *    - ImportBindingsFromString() applies shared configs.
 *    - Useful for sharing setups with friends or community.
 *
 * =============================================================================
 * @section usage USAGE EXAMPLE
 * =============================================================================
 *
 * @code
 * // Get the subsystem
 * UMGInputRemapSubsystem* InputRemap = GetGameInstance()->GetSubsystem<UMGInputRemapSubsystem>();
 *
 * // Change a single binding
 * InputRemap->SetBinding(EMGInputAction::Nitro, EKeys::SpaceBar, true); // Primary
 * InputRemap->SetGamepadBinding(EMGInputAction::Nitro, EKeys::Gamepad_FaceButton_Bottom);
 *
 * // Apply a preset control scheme
 * InputRemap->SetControlScheme(EMGControlScheme::Racing);
 *
 * // Enable accessibility assists
 * FMGDrivingAssists Assists;
 * Assists.bAutoAccelerate = true;
 * Assists.bSteeringAssist = true;
 * Assists.SteeringAssistStrength = 0.7f;
 * InputRemap->SetDrivingAssists(Assists);
 *
 * // Check if auto-accelerate is on (for vehicle code)
 * if (InputRemap->IsAutoAccelerateEnabled())
 * {
 *     ThrottleInput = 1.0f; // Full throttle
 * }
 *
 * // Configure racing wheel
 * FMGWheelSettings WheelConfig;
 * WheelConfig.SteeringRotation = 900.0f;
 * WheelConfig.ForceFeedbackStrength = 0.8f;
 * WheelConfig.RoadFeelStrength = 0.6f;
 * InputRemap->SetWheelSettings(WheelConfig);
 *
 * // Save current setup as custom scheme
 * InputRemap->SaveCurrentAsCustomScheme(0); // Saves to Custom1
 *
 * // Share bindings with a friend
 * FString ExportedConfig = InputRemap->ExportBindingsToString();
 * // ... send to friend ...
 * InputRemap->ImportBindingsFromString(ReceivedConfig);
 *
 * // Listen for device changes (for UI prompt switching)
 * InputRemap->OnInputDeviceChanged.AddDynamic(this, &UMyWidget::UpdateButtonPrompts);
 * @endcode
 *
 * =============================================================================
 * @section accessibility ACCESSIBILITY CONSIDERATIONS
 * =============================================================================
 *
 * This system was designed with accessibility as a priority:
 *
 * 1. ONE-HANDED PLAY:
 *    - OneHanded_Left/Right schemes consolidate controls.
 *    - All essential actions reachable with one hand.
 *
 * 2. MOTOR ACCESSIBILITY:
 *    - Auto-accelerate removes need for constant throttle input.
 *    - Steering/braking assists reduce precision requirements.
 *    - Adjustable dead zones accommodate limited mobility.
 *
 * 3. REMAPPING EVERYTHING:
 *    - Every action can be rebound to any key/button.
 *    - Multiple bindings per action (primary + secondary + gamepad).
 *    - Axis inversion for those who prefer it.
 *
 * 4. SIMPLIFIED MODE:
 *    - bSimplifiedControls reduces the number of required inputs.
 *    - Good for new players or those who prefer simpler controls.
 *
 * @see FMGDrivingAssists for accessibility assist options
 * @see FMGWheelSettings for racing wheel configuration
 * @see EMGControlScheme for available preset schemes
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerInput.h"
#include "Core/MGSharedTypes.h"
#include "MGInputRemapSubsystem.generated.h"

// EMGInputAction - REMOVED (duplicate)
// Canonical definition in: InputBuffer/MGInputBufferSubsystem.h

UENUM(BlueprintType)
enum class EMGControlScheme : uint8
{
	Default,
	Alternate,
	Racing,
	Casual,
	OneHanded_Left,
	OneHanded_Right,
	Custom1,
	Custom2,
	Custom3
};

UENUM(BlueprintType)
enum class EMGInputDevice : uint8
{
	Keyboard,
	Gamepad,
	Wheel,
	Touch
};

USTRUCT(BlueprintType)
struct FMGInputBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGInputAction Action = EMGInputAction::Accelerate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey PrimaryKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey SecondaryKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey GamepadKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AxisScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZone = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInvertAxis = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAxisInput = false;
};

USTRUCT(BlueprintType)
struct FMGControlProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGControlScheme Scheme = EMGControlScheme::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGInputBinding> Bindings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerDeadZone = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StickDeadZone = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVibrationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VibrationIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSwapSticksEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSwapTriggersEnabled = false;
};

USTRUCT(BlueprintType)
struct FMGDrivingAssists
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoAccelerate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSteeringAssist = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringAssistStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBrakingAssist = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingAssistStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoShift = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTractionControl = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TractionControlStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStabilityControl = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StabilityControlStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAntiLockBrakes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoNitro = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimplifiedControls = false;
};

USTRUCT(BlueprintType)
struct FMGWheelSettings
{
	GENERATED_BODY()

	// === Steering Settings ===

	/** Wheel rotation range in degrees (e.g., 900, 1080) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float SteeringRotation = 900.0f;

	/** Steering input curve (1.0 = linear, >1 = more center precision) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float SteeringLinearity = 1.0f;

	/** Steering deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float SteeringDeadzone = 0.0f;

	/** Invert steering direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	bool bInvertSteering = false;

	// === Pedal Settings ===

	/** Throttle pedal deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedals", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float ThrottleDeadzone = 0.05f;

	/** Brake pedal deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedals", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float BrakeDeadzone = 0.05f;

	/** Clutch pedal deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedals", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float ClutchDeadzone = 0.1f;

	/** Legacy: general pedal deadzone (deprecated, use specific deadzones) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedals")
	float PedalDeadZone = 0.05f;

	/** Use combined pedal axis (for older wheels) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedals")
	bool bCombinedPedals = false;

	/** Invert clutch pedal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pedals")
	bool bInvertClutch = false;

	// === Force Feedback Settings ===

	/** Enable force feedback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback")
	bool bForceFeedbackEnabled = true;

	/** Master force feedback strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ForceFeedbackStrength = 0.7f;

	/** Self-centering spring strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SelfCenteringStrength = 0.5f;

	/** Road feel / tire feedback strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RoadFeelStrength = 0.6f;

	/** Collision impact feedback strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CollisionStrength = 0.8f;

	/** Curb/rumble strip feedback strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurbStrength = 0.5f;

	/** Engine vibration at redline strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EngineVibrationStrength = 0.3f;

	/** Damper effect strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamperStrength = 0.2f;

	/** Minimum force threshold (helps with weak FFB motors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float MinForceThreshold = 0.02f;

	/** Show FFB clipping indicator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ForceFeedback")
	bool bShowFFBClipping = true;

	// === Profile Settings ===

	/** Currently selected wheel profile name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString ActiveProfileName = TEXT("Default");
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnBindingChanged, EMGInputAction, Action, FKey, NewKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnControlSchemeChanged, EMGControlScheme, NewScheme);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnInputDeviceChanged, EMGInputDevice, NewDevice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnBindingsReset);

UCLASS()
class MIDNIGHTGRIND_API UMGInputRemapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Binding Management
	UFUNCTION(BlueprintCallable, Category = "InputRemap|Binding")
	void SetBinding(EMGInputAction Action, FKey NewKey, bool bPrimary = true);

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Binding")
	void SetGamepadBinding(EMGInputAction Action, FKey NewKey);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Binding")
	FMGInputBinding GetBinding(EMGInputAction Action) const;

	UFUNCTION(BlueprintPure, Category = "InputRemap|Binding")
	FKey GetPrimaryKey(EMGInputAction Action) const;

	UFUNCTION(BlueprintPure, Category = "InputRemap|Binding")
	FKey GetGamepadKey(EMGInputAction Action) const;

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Binding")
	void ClearBinding(EMGInputAction Action, bool bPrimary = true);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Binding")
	bool IsKeyBound(FKey Key) const;

	UFUNCTION(BlueprintPure, Category = "InputRemap|Binding")
	EMGInputAction GetActionForKey(FKey Key) const;

	// Control Schemes
	UFUNCTION(BlueprintCallable, Category = "InputRemap|Scheme")
	void SetControlScheme(EMGControlScheme Scheme);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Scheme")
	EMGControlScheme GetCurrentScheme() const { return CurrentScheme; }

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Scheme")
	void ResetToDefaultBindings();

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Scheme")
	void SaveCurrentAsCustomScheme(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Scheme")
	TArray<FMGControlProfile> GetAvailableProfiles() const { return ControlProfiles; }

	// Driving Assists
	UFUNCTION(BlueprintCallable, Category = "InputRemap|Assists")
	void SetDrivingAssists(const FMGDrivingAssists& Assists);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Assists")
	FMGDrivingAssists GetDrivingAssists() const { return CurrentAssists; }

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Assists")
	void SetAutoAccelerate(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Assists")
	void SetSteeringAssist(bool bEnabled, float Strength = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Assists")
	void SetBrakingAssist(bool bEnabled, float Strength = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Assists")
	void SetTractionControl(bool bEnabled, float Strength = 0.5f);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Assists")
	bool IsAutoAccelerateEnabled() const { return CurrentAssists.bAutoAccelerate; }

	// Sensitivity Settings
	UFUNCTION(BlueprintCallable, Category = "InputRemap|Sensitivity")
	void SetSteeringSensitivity(float Sensitivity);

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Sensitivity")
	void SetDeadZone(float LeftStick, float RightStick, float Triggers);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Sensitivity")
	float GetSteeringSensitivity() const { return CurrentProfile.SteeringSensitivity; }

	// Vibration
	UFUNCTION(BlueprintCallable, Category = "InputRemap|Vibration")
	void SetVibrationEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Vibration")
	void SetVibrationIntensity(float Intensity);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Vibration")
	bool IsVibrationEnabled() const { return CurrentProfile.bVibrationEnabled; }

	// Wheel Support
	UFUNCTION(BlueprintCallable, Category = "InputRemap|Wheel")
	void SetWheelSettings(const FMGWheelSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "InputRemap|Wheel")
	FMGWheelSettings GetWheelSettings() const { return WheelSettings; }

	UFUNCTION(BlueprintPure, Category = "InputRemap|Wheel")
	bool IsWheelConnected() const { return bWheelConnected; }

	// Device Detection
	UFUNCTION(BlueprintPure, Category = "InputRemap|Device")
	EMGInputDevice GetActiveDevice() const { return ActiveDevice; }

	UFUNCTION(BlueprintCallable, Category = "InputRemap|Device")
	void SetPreferredDevice(EMGInputDevice Device);

	// Import/Export
	UFUNCTION(BlueprintCallable, Category = "InputRemap|IO")
	FString ExportBindingsToString() const;

	UFUNCTION(BlueprintCallable, Category = "InputRemap|IO")
	bool ImportBindingsFromString(const FString& Data);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "InputRemap|Events")
	FMGOnBindingChanged OnBindingChanged;

	UPROPERTY(BlueprintAssignable, Category = "InputRemap|Events")
	FMGOnControlSchemeChanged OnControlSchemeChanged;

	UPROPERTY(BlueprintAssignable, Category = "InputRemap|Events")
	FMGOnInputDeviceChanged OnInputDeviceChanged;

	UPROPERTY(BlueprintAssignable, Category = "InputRemap|Events")
	FMGOnBindingsReset OnBindingsReset;

protected:
	void InitializeDefaultBindings();
	void InitializeControlProfiles();
	void ApplyBindingsToPlayerInput();
	void LoadSavedBindings();
	void SaveBindings();
	FString GetActionName(EMGInputAction Action) const;
	void DetectInputDevice();
	void OnAnyKeyPressed(FKey Key);

private:
	FMGControlProfile CurrentProfile;
	FMGDrivingAssists CurrentAssists;
	FMGWheelSettings WheelSettings;
	TArray<FMGControlProfile> ControlProfiles;
	EMGControlScheme CurrentScheme = EMGControlScheme::Default;
	EMGInputDevice ActiveDevice = EMGInputDevice::Gamepad;
	EMGInputDevice PreferredDevice = EMGInputDevice::Gamepad;
	FTimerHandle DeviceDetectionHandle;
	bool bWheelConnected = false;
	FDateTime LastKeyboardInput;
	FDateTime LastGamepadInput;
};

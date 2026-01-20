// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerInput.h"
#include "MGInputRemapSubsystem.generated.h"

/**
 * Input Remap System - Full Controller Customization
 * - Complete control remapping for all inputs
 * - Multiple control schemes and presets
 * - One-handed control options
 * - Accessibility assists (auto-accelerate, steering assist)
 * - Per-profile bindings
 * - Export/import configurations
 */

UENUM(BlueprintType)
enum class EMGInputAction : uint8
{
	Accelerate,
	Brake,
	SteerLeft,
	SteerRight,
	Handbrake,
	Nitro,
	ShiftUp,
	ShiftDown,
	LookBack,
	Horn,
	ResetVehicle,
	ToggleCamera,
	Pause,
	Map,
	QuickChat1,
	QuickChat2,
	QuickChat3,
	QuickChat4,
	PhotoMode,
	Rewind
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringRotation = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringLinearity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForceFeedbackStrength = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForceFeedbackEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PedalDeadZone = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCombinedPedals = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInvertClutch = false;
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

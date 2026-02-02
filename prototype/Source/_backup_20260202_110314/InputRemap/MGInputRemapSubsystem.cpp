// Copyright Midnight Grind. All Rights Reserved.

#include "InputRemap/MGInputRemapSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"
#include "Serialization/JsonSerializer.h"

void UMGInputRemapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultBindings();
	InitializeControlProfiles();
	LoadSavedBindings();

	// Set up device detection
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DeviceDetectionHandle,
			this,
			&UMGInputRemapSubsystem::DetectInputDevice,
			0.5f,
			true
		);
	}
}

void UMGInputRemapSubsystem::Deinitialize()
{
	SaveBindings();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeviceDetectionHandle);
	}
	Super::Deinitialize();
}

void UMGInputRemapSubsystem::SetBinding(EMGInputAction Action, FKey NewKey, bool bPrimary)
{
	// Check for conflicts
	for (FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		if (Binding.Action != Action)
		{
			if (bPrimary && Binding.PrimaryKey == NewKey)
			{
				Binding.PrimaryKey = FKey();
			}
			else if (!bPrimary && Binding.SecondaryKey == NewKey)
			{
				Binding.SecondaryKey = FKey();
			}
		}
	}

	// Set the new binding
	for (FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		if (Binding.Action == Action)
		{
			if (bPrimary)
			{
				Binding.PrimaryKey = NewKey;
			}
			else
			{
				Binding.SecondaryKey = NewKey;
			}
			break;
		}
	}

	ApplyBindingsToPlayerInput();
	OnBindingChanged.Broadcast(Action, NewKey);
	SaveBindings();
}

void UMGInputRemapSubsystem::SetGamepadBinding(EMGInputAction Action, FKey NewKey)
{
	// Check for conflicts
	for (FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		if (Binding.Action != Action && Binding.GamepadKey == NewKey)
		{
			Binding.GamepadKey = FKey();
		}
	}

	for (FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		if (Binding.Action == Action)
		{
			Binding.GamepadKey = NewKey;
			break;
		}
	}

	ApplyBindingsToPlayerInput();
	OnBindingChanged.Broadcast(Action, NewKey);
	SaveBindings();
}

FMGInputBinding UMGInputRemapSubsystem::GetBinding(EMGInputAction Action) const
{
	for (const FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		if (Binding.Action == Action)
		{
			return Binding;
		}
	}
	return FMGInputBinding();
}

FKey UMGInputRemapSubsystem::GetPrimaryKey(EMGInputAction Action) const
{
	return GetBinding(Action).PrimaryKey;
}

FKey UMGInputRemapSubsystem::GetGamepadKey(EMGInputAction Action) const
{
	return GetBinding(Action).GamepadKey;
}

void UMGInputRemapSubsystem::ClearBinding(EMGInputAction Action, bool bPrimary)
{
	for (FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		if (Binding.Action == Action)
		{
			if (bPrimary)
			{
				Binding.PrimaryKey = FKey();
			}
			else
			{
				Binding.SecondaryKey = FKey();
			}
			break;
		}
	}

	ApplyBindingsToPlayerInput();
	SaveBindings();
}

bool UMGInputRemapSubsystem::IsKeyBound(FKey Key) const
{
	for (const FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		if (Binding.PrimaryKey == Key || Binding.SecondaryKey == Key || Binding.GamepadKey == Key)
		{
			return true;
		}
	}
	return false;
}

EMGInputAction UMGInputRemapSubsystem::GetActionForKey(FKey Key) const
{
	for (const FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		if (Binding.PrimaryKey == Key || Binding.SecondaryKey == Key || Binding.GamepadKey == Key)
		{
			return Binding.Action;
		}
	}
	return EMGInputAction::Accelerate; // Default
}

void UMGInputRemapSubsystem::SetControlScheme(EMGControlScheme Scheme)
{
	if (CurrentScheme == Scheme)
		return;

	CurrentScheme = Scheme;

	// Load the appropriate profile
	for (const FMGControlProfile& Profile : ControlProfiles)
	{
		if (Profile.Scheme == Scheme)
		{
			CurrentProfile = Profile;
			break;
		}
	}

	ApplyBindingsToPlayerInput();
	OnControlSchemeChanged.Broadcast(Scheme);
	SaveBindings();
}

void UMGInputRemapSubsystem::ResetToDefaultBindings()
{
	CurrentScheme = EMGControlScheme::Default;
	InitializeDefaultBindings();
	ApplyBindingsToPlayerInput();
	OnBindingsReset.Broadcast();
	SaveBindings();
}

void UMGInputRemapSubsystem::SaveCurrentAsCustomScheme(int32 SlotIndex)
{
	EMGControlScheme TargetScheme;
	switch (SlotIndex)
	{
	case 1: TargetScheme = EMGControlScheme::Custom1; break;
	case 2: TargetScheme = EMGControlScheme::Custom2; break;
	case 3: TargetScheme = EMGControlScheme::Custom3; break;
	default: return;
	}

	// Find or create custom slot
	bool bFound = false;
	for (FMGControlProfile& Profile : ControlProfiles)
	{
		if (Profile.Scheme == TargetScheme)
		{
			Profile = CurrentProfile;
			Profile.Scheme = TargetScheme;
			Profile.ProfileName = FString::Printf(TEXT("Custom %d"), SlotIndex);
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		FMGControlProfile NewProfile = CurrentProfile;
		NewProfile.Scheme = TargetScheme;
		NewProfile.ProfileName = FString::Printf(TEXT("Custom %d"), SlotIndex);
		ControlProfiles.Add(NewProfile);
	}

	SaveBindings();
}

void UMGInputRemapSubsystem::SetDrivingAssists(const FMGDrivingAssists& Assists)
{
	CurrentAssists = Assists;
	SaveBindings();
}

void UMGInputRemapSubsystem::SetAutoAccelerate(bool bEnabled)
{
	CurrentAssists.bAutoAccelerate = bEnabled;
	SaveBindings();
}

void UMGInputRemapSubsystem::SetSteeringAssist(bool bEnabled, float Strength)
{
	CurrentAssists.bSteeringAssist = bEnabled;
	CurrentAssists.SteeringAssistStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
	SaveBindings();
}

void UMGInputRemapSubsystem::SetBrakingAssist(bool bEnabled, float Strength)
{
	CurrentAssists.bBrakingAssist = bEnabled;
	CurrentAssists.BrakingAssistStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
	SaveBindings();
}

void UMGInputRemapSubsystem::SetTractionControl(bool bEnabled, float Strength)
{
	CurrentAssists.bTractionControl = bEnabled;
	CurrentAssists.TractionControlStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
	SaveBindings();
}

void UMGInputRemapSubsystem::SetSteeringSensitivity(float Sensitivity)
{
	CurrentProfile.SteeringSensitivity = FMath::Clamp(Sensitivity, 0.1f, 3.0f);
	SaveBindings();
}

void UMGInputRemapSubsystem::SetDeadZone(float LeftStick, float RightStick, float Triggers)
{
	CurrentProfile.StickDeadZone = FMath::Clamp(LeftStick, 0.0f, 0.5f);
	CurrentProfile.TriggerDeadZone = FMath::Clamp(Triggers, 0.0f, 0.5f);
	SaveBindings();
}

void UMGInputRemapSubsystem::SetVibrationEnabled(bool bEnabled)
{
	CurrentProfile.bVibrationEnabled = bEnabled;
	SaveBindings();
}

void UMGInputRemapSubsystem::SetVibrationIntensity(float Intensity)
{
	CurrentProfile.VibrationIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	SaveBindings();
}

void UMGInputRemapSubsystem::SetWheelSettings(const FMGWheelSettings& Settings)
{
	WheelSettings = Settings;
	SaveBindings();
}

void UMGInputRemapSubsystem::SetPreferredDevice(EMGInputDevice Device)
{
	PreferredDevice = Device;
	SaveBindings();
}

FString UMGInputRemapSubsystem::ExportBindingsToString() const
{
	TSharedPtr<FJsonObject> JsonRoot = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> BindingsArray;
	for (const FMGInputBinding& Binding : CurrentProfile.Bindings)
	{
		TSharedPtr<FJsonObject> BindingObj = MakeShareable(new FJsonObject);
		BindingObj->SetNumberField(TEXT("action"), static_cast<int32>(Binding.Action));
		BindingObj->SetStringField(TEXT("primary"), Binding.PrimaryKey.ToString());
		BindingObj->SetStringField(TEXT("secondary"), Binding.SecondaryKey.ToString());
		BindingObj->SetStringField(TEXT("gamepad"), Binding.GamepadKey.ToString());
		BindingObj->SetNumberField(TEXT("scale"), Binding.AxisScale);
		BindingObj->SetNumberField(TEXT("deadzone"), Binding.DeadZone);
		BindingObj->SetBoolField(TEXT("invert"), Binding.bInvertAxis);
		BindingsArray.Add(MakeShareable(new FJsonValueObject(BindingObj)));
	}
	JsonRoot->SetArrayField(TEXT("bindings"), BindingsArray);

	JsonRoot->SetNumberField(TEXT("sensitivity"), CurrentProfile.SteeringSensitivity);
	JsonRoot->SetNumberField(TEXT("stick_deadzone"), CurrentProfile.StickDeadZone);
	JsonRoot->SetNumberField(TEXT("trigger_deadzone"), CurrentProfile.TriggerDeadZone);
	JsonRoot->SetBoolField(TEXT("vibration"), CurrentProfile.bVibrationEnabled);
	JsonRoot->SetNumberField(TEXT("vibration_intensity"), CurrentProfile.VibrationIntensity);

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(JsonRoot.ToSharedRef(), Writer);

	return Output;
}

bool UMGInputRemapSubsystem::ImportBindingsFromString(const FString& Data)
{
	TSharedPtr<FJsonObject> JsonRoot;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Data);

	if (!FJsonSerializer::Deserialize(Reader, JsonRoot) || !JsonRoot.IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* BindingsArray;
	if (JsonRoot->TryGetArrayField(TEXT("bindings"), BindingsArray))
	{
		CurrentProfile.Bindings.Empty();

		for (const TSharedPtr<FJsonValue>& Value : *BindingsArray)
		{
			TSharedPtr<FJsonObject> BindingObj = Value->AsObject();
			if (BindingObj.IsValid())
			{
				FMGInputBinding Binding;
				Binding.Action = static_cast<EMGInputAction>(BindingObj->GetIntegerField(TEXT("action")));
				Binding.PrimaryKey = FKey(*BindingObj->GetStringField(TEXT("primary")));
				Binding.SecondaryKey = FKey(*BindingObj->GetStringField(TEXT("secondary")));
				Binding.GamepadKey = FKey(*BindingObj->GetStringField(TEXT("gamepad")));
				Binding.AxisScale = BindingObj->GetNumberField(TEXT("scale"));
				Binding.DeadZone = BindingObj->GetNumberField(TEXT("deadzone"));
				Binding.bInvertAxis = BindingObj->GetBoolField(TEXT("invert"));
				CurrentProfile.Bindings.Add(Binding);
			}
		}
	}

	CurrentProfile.SteeringSensitivity = JsonRoot->GetNumberField(TEXT("sensitivity"));
	CurrentProfile.StickDeadZone = JsonRoot->GetNumberField(TEXT("stick_deadzone"));
	CurrentProfile.TriggerDeadZone = JsonRoot->GetNumberField(TEXT("trigger_deadzone"));
	CurrentProfile.bVibrationEnabled = JsonRoot->GetBoolField(TEXT("vibration"));
	CurrentProfile.VibrationIntensity = JsonRoot->GetNumberField(TEXT("vibration_intensity"));

	ApplyBindingsToPlayerInput();
	SaveBindings();

	return true;
}

void UMGInputRemapSubsystem::InitializeDefaultBindings()
{
	CurrentProfile.ProfileName = TEXT("Default");
	CurrentProfile.Scheme = EMGControlScheme::Default;
	CurrentProfile.SteeringSensitivity = 1.0f;
	CurrentProfile.StickDeadZone = 0.15f;
	CurrentProfile.TriggerDeadZone = 0.1f;
	CurrentProfile.bVibrationEnabled = true;
	CurrentProfile.VibrationIntensity = 1.0f;
	CurrentProfile.Bindings.Empty();

	// Accelerate
	FMGInputBinding Accelerate;
	Accelerate.Action = EMGInputAction::Accelerate;
	Accelerate.PrimaryKey = EKeys::W;
	Accelerate.SecondaryKey = EKeys::Up;
	Accelerate.GamepadKey = EKeys::Gamepad_RightTrigger;
	Accelerate.bIsAxisInput = true;
	CurrentProfile.Bindings.Add(Accelerate);

	// Brake
	FMGInputBinding Brake;
	Brake.Action = EMGInputAction::Brake;
	Brake.PrimaryKey = EKeys::S;
	Brake.SecondaryKey = EKeys::Down;
	Brake.GamepadKey = EKeys::Gamepad_LeftTrigger;
	Brake.bIsAxisInput = true;
	CurrentProfile.Bindings.Add(Brake);

	// Steer Left
	FMGInputBinding SteerLeft;
	SteerLeft.Action = EMGInputAction::SteerLeft;
	SteerLeft.PrimaryKey = EKeys::A;
	SteerLeft.SecondaryKey = EKeys::Left;
	SteerLeft.GamepadKey = EKeys::Gamepad_LeftX;
	SteerLeft.bIsAxisInput = true;
	SteerLeft.AxisScale = -1.0f;
	CurrentProfile.Bindings.Add(SteerLeft);

	// Steer Right
	FMGInputBinding SteerRight;
	SteerRight.Action = EMGInputAction::SteerRight;
	SteerRight.PrimaryKey = EKeys::D;
	SteerRight.SecondaryKey = EKeys::Right;
	SteerRight.GamepadKey = EKeys::Gamepad_LeftX;
	SteerRight.bIsAxisInput = true;
	SteerRight.AxisScale = 1.0f;
	CurrentProfile.Bindings.Add(SteerRight);

	// Handbrake
	FMGInputBinding Handbrake;
	Handbrake.Action = EMGInputAction::Handbrake;
	Handbrake.PrimaryKey = EKeys::SpaceBar;
	Handbrake.GamepadKey = EKeys::Gamepad_FaceButton_Bottom;
	CurrentProfile.Bindings.Add(Handbrake);

	// Nitro
	FMGInputBinding Nitro;
	Nitro.Action = EMGInputAction::Nitro;
	Nitro.PrimaryKey = EKeys::LeftShift;
	Nitro.GamepadKey = EKeys::Gamepad_FaceButton_Left;
	CurrentProfile.Bindings.Add(Nitro);

	// Shift Up
	FMGInputBinding ShiftUp;
	ShiftUp.Action = EMGInputAction::ShiftUp;
	ShiftUp.PrimaryKey = EKeys::E;
	ShiftUp.GamepadKey = EKeys::Gamepad_RightShoulder;
	CurrentProfile.Bindings.Add(ShiftUp);

	// Shift Down
	FMGInputBinding ShiftDown;
	ShiftDown.Action = EMGInputAction::ShiftDown;
	ShiftDown.PrimaryKey = EKeys::Q;
	ShiftDown.GamepadKey = EKeys::Gamepad_LeftShoulder;
	CurrentProfile.Bindings.Add(ShiftDown);

	// Look Back
	FMGInputBinding LookBack;
	LookBack.Action = EMGInputAction::LookBack;
	LookBack.PrimaryKey = EKeys::C;
	LookBack.GamepadKey = EKeys::Gamepad_RightThumbstick;
	CurrentProfile.Bindings.Add(LookBack);

	// Horn
	FMGInputBinding Horn;
	Horn.Action = EMGInputAction::Horn;
	Horn.PrimaryKey = EKeys::H;
	Horn.GamepadKey = EKeys::Gamepad_LeftThumbstick;
	CurrentProfile.Bindings.Add(Horn);

	// Reset Vehicle
	FMGInputBinding Reset;
	Reset.Action = EMGInputAction::ResetVehicle;
	Reset.PrimaryKey = EKeys::R;
	Reset.GamepadKey = EKeys::Gamepad_FaceButton_Top;
	CurrentProfile.Bindings.Add(Reset);

	// Toggle Camera
	FMGInputBinding Camera;
	Camera.Action = EMGInputAction::ToggleCamera;
	Camera.PrimaryKey = EKeys::V;
	Camera.GamepadKey = EKeys::Gamepad_FaceButton_Right;
	CurrentProfile.Bindings.Add(Camera);

	// Pause
	FMGInputBinding Pause;
	Pause.Action = EMGInputAction::Pause;
	Pause.PrimaryKey = EKeys::Escape;
	Pause.GamepadKey = EKeys::Gamepad_Special_Right;
	CurrentProfile.Bindings.Add(Pause);

	// Map
	FMGInputBinding Map;
	Map.Action = EMGInputAction::Map;
	Map.PrimaryKey = EKeys::M;
	Map.GamepadKey = EKeys::Gamepad_Special_Left;
	CurrentProfile.Bindings.Add(Map);

	// Photo Mode
	FMGInputBinding PhotoMode;
	PhotoMode.Action = EMGInputAction::PhotoMode;
	PhotoMode.PrimaryKey = EKeys::P;
	PhotoMode.GamepadKey = EKeys::Gamepad_DPad_Up;
	CurrentProfile.Bindings.Add(PhotoMode);

	// Rewind
	FMGInputBinding Rewind;
	Rewind.Action = EMGInputAction::Rewind;
	Rewind.PrimaryKey = EKeys::BackSpace;
	Rewind.GamepadKey = EKeys::Gamepad_DPad_Left;
	CurrentProfile.Bindings.Add(Rewind);

	// Quick Chat bindings
	FMGInputBinding QC1;
	QC1.Action = EMGInputAction::QuickChat1;
	QC1.PrimaryKey = EKeys::One;
	QC1.GamepadKey = EKeys::Gamepad_DPad_Up;
	CurrentProfile.Bindings.Add(QC1);

	FMGInputBinding QC2;
	QC2.Action = EMGInputAction::QuickChat2;
	QC2.PrimaryKey = EKeys::Two;
	QC2.GamepadKey = EKeys::Gamepad_DPad_Right;
	CurrentProfile.Bindings.Add(QC2);

	FMGInputBinding QC3;
	QC3.Action = EMGInputAction::QuickChat3;
	QC3.PrimaryKey = EKeys::Three;
	QC3.GamepadKey = EKeys::Gamepad_DPad_Down;
	CurrentProfile.Bindings.Add(QC3);

	FMGInputBinding QC4;
	QC4.Action = EMGInputAction::QuickChat4;
	QC4.PrimaryKey = EKeys::Four;
	QC4.GamepadKey = EKeys::Gamepad_DPad_Left;
	CurrentProfile.Bindings.Add(QC4);

	// Initialize default assists
	CurrentAssists.bAutoAccelerate = false;
	CurrentAssists.bSteeringAssist = false;
	CurrentAssists.bBrakingAssist = false;
	CurrentAssists.bAutoShift = true;
	CurrentAssists.bTractionControl = true;
	CurrentAssists.TractionControlStrength = 0.5f;
	CurrentAssists.bStabilityControl = true;
	CurrentAssists.StabilityControlStrength = 0.5f;
	CurrentAssists.bAntiLockBrakes = true;
}

void UMGInputRemapSubsystem::InitializeControlProfiles()
{
	ControlProfiles.Empty();

	// Default profile
	ControlProfiles.Add(CurrentProfile);

	// One-handed left profile
	FMGControlProfile OneHandedLeft = CurrentProfile;
	OneHandedLeft.ProfileName = TEXT("One-Handed (Left)");
	OneHandedLeft.Scheme = EMGControlScheme::OneHanded_Left;
	// Remap all controls to left side of controller
	for (FMGInputBinding& Binding : OneHandedLeft.Bindings)
	{
		if (Binding.Action == EMGInputAction::Accelerate)
			Binding.GamepadKey = EKeys::Gamepad_LeftTrigger;
		if (Binding.Action == EMGInputAction::Brake)
			Binding.GamepadKey = EKeys::Gamepad_LeftShoulder;
		if (Binding.Action == EMGInputAction::Nitro)
			Binding.GamepadKey = EKeys::Gamepad_LeftThumbstick;
	}
	ControlProfiles.Add(OneHandedLeft);

	// One-handed right profile
	FMGControlProfile OneHandedRight = CurrentProfile;
	OneHandedRight.ProfileName = TEXT("One-Handed (Right)");
	OneHandedRight.Scheme = EMGControlScheme::OneHanded_Right;
	for (FMGInputBinding& Binding : OneHandedRight.Bindings)
	{
		if (Binding.Action == EMGInputAction::SteerLeft || Binding.Action == EMGInputAction::SteerRight)
			Binding.GamepadKey = EKeys::Gamepad_RightX;
	}
	ControlProfiles.Add(OneHandedRight);

	// Casual profile (simplified)
	FMGControlProfile Casual = CurrentProfile;
	Casual.ProfileName = TEXT("Casual");
	Casual.Scheme = EMGControlScheme::Casual;
	ControlProfiles.Add(Casual);

	// Racing profile (advanced)
	FMGControlProfile Racing = CurrentProfile;
	Racing.ProfileName = TEXT("Racing");
	Racing.Scheme = EMGControlScheme::Racing;
	Racing.SteeringSensitivity = 1.2f;
	ControlProfiles.Add(Racing);
}

void UMGInputRemapSubsystem::ApplyBindingsToPlayerInput()
{
	// This would apply bindings to the actual UE input system
	// In practice, you'd use UInputSettings or directly modify PlayerInput

	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	if (!InputSettings)
		return;

	// The actual implementation would iterate through bindings and set them via InputSettings
	// This requires careful handling to avoid conflicts with default bindings
}

void UMGInputRemapSubsystem::LoadSavedBindings()
{
	// Would load from SaveGame or platform-specific storage
}

void UMGInputRemapSubsystem::SaveBindings()
{
	// Would save to SaveGame or platform-specific storage
}

FString UMGInputRemapSubsystem::GetActionName(EMGInputAction Action) const
{
	switch (Action)
	{
	case EMGInputAction::Accelerate: return TEXT("Accelerate");
	case EMGInputAction::Brake: return TEXT("Brake");
	case EMGInputAction::SteerLeft: return TEXT("Steer Left");
	case EMGInputAction::SteerRight: return TEXT("Steer Right");
	case EMGInputAction::Handbrake: return TEXT("Handbrake");
	case EMGInputAction::Nitro: return TEXT("Nitro");
	case EMGInputAction::ShiftUp: return TEXT("Shift Up");
	case EMGInputAction::ShiftDown: return TEXT("Shift Down");
	case EMGInputAction::LookBack: return TEXT("Look Back");
	case EMGInputAction::Horn: return TEXT("Horn");
	case EMGInputAction::ResetVehicle: return TEXT("Reset Vehicle");
	case EMGInputAction::ToggleCamera: return TEXT("Toggle Camera");
	case EMGInputAction::Pause: return TEXT("Pause");
	case EMGInputAction::Map: return TEXT("Map");
	case EMGInputAction::PhotoMode: return TEXT("Photo Mode");
	case EMGInputAction::Rewind: return TEXT("Rewind");
	default: return TEXT("Unknown");
	}
}

void UMGInputRemapSubsystem::DetectInputDevice()
{
	// Would monitor input to detect which device is being used
	// Switch UI prompts accordingly

	EMGInputDevice DetectedDevice = ActiveDevice;

	FTimespan KeyboardAge = FDateTime::UtcNow() - LastKeyboardInput;
	FTimespan GamepadAge = FDateTime::UtcNow() - LastGamepadInput;

	if (KeyboardAge < GamepadAge && KeyboardAge.GetTotalSeconds() < 2.0f)
	{
		DetectedDevice = EMGInputDevice::Keyboard;
	}
	else if (GamepadAge.GetTotalSeconds() < 2.0f)
	{
		DetectedDevice = EMGInputDevice::Gamepad;
	}

	if (DetectedDevice != ActiveDevice)
	{
		ActiveDevice = DetectedDevice;
		OnInputDeviceChanged.Broadcast(ActiveDevice);
	}
}

void UMGInputRemapSubsystem::OnAnyKeyPressed(FKey Key)
{
	if (Key.IsGamepadKey())
	{
		LastGamepadInput = FDateTime::UtcNow();
	}
	else
	{
		LastKeyboardInput = FDateTime::UtcNow();
	}
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Input/MGInputConfig.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// ==========================================
// UMGVehicleInputConfig
// ==========================================

FKey UMGVehicleInputConfig::GetDefaultGamepadBinding(FName ActionName)
{
	// Default gamepad bindings
	static TMap<FName, FKey> GamepadDefaults = {
		// Driving
		{ TEXT("Throttle"), EKeys::Gamepad_RightTrigger },
		{ TEXT("Brake"), EKeys::Gamepad_LeftTrigger },
		{ TEXT("Steering"), EKeys::Gamepad_LeftX },
		{ TEXT("Handbrake"), EKeys::Gamepad_FaceButton_Bottom }, // A
		{ TEXT("Nitrous"), EKeys::Gamepad_FaceButton_Left }, // X

		// Transmission
		{ TEXT("ShiftUp"), EKeys::Gamepad_RightShoulder },
		{ TEXT("ShiftDown"), EKeys::Gamepad_LeftShoulder },

		// Camera
		{ TEXT("CameraCycle"), EKeys::Gamepad_FaceButton_Right }, // B
		{ TEXT("LookBehind"), EKeys::Gamepad_FaceButton_Top }, // Y
		{ TEXT("FreeLook"), EKeys::Gamepad_RightX },

		// Game
		{ TEXT("Reset"), EKeys::Gamepad_Special_Right }, // Menu
		{ TEXT("Pause"), EKeys::Gamepad_Special_Left }, // View
		{ TEXT("Map"), EKeys::Gamepad_DPad_Down }
	};

	if (const FKey* Found = GamepadDefaults.Find(ActionName))
	{
		return *Found;
	}

	return EKeys::Invalid;
}

FKey UMGVehicleInputConfig::GetDefaultKeyboardBinding(FName ActionName)
{
	// Default keyboard bindings
	static TMap<FName, FKey> KeyboardDefaults = {
		// Driving
		{ TEXT("Throttle"), EKeys::W },
		{ TEXT("Brake"), EKeys::S },
		{ TEXT("SteeringLeft"), EKeys::A },
		{ TEXT("SteeringRight"), EKeys::D },
		{ TEXT("Handbrake"), EKeys::SpaceBar },
		{ TEXT("Nitrous"), EKeys::LeftShift },

		// Transmission
		{ TEXT("ShiftUp"), EKeys::E },
		{ TEXT("ShiftDown"), EKeys::Q },

		// Camera
		{ TEXT("CameraCycle"), EKeys::C },
		{ TEXT("LookBehind"), EKeys::V },

		// Game
		{ TEXT("Reset"), EKeys::R },
		{ TEXT("Pause"), EKeys::Escape },
		{ TEXT("Map"), EKeys::M }
	};

	if (const FKey* Found = KeyboardDefaults.Find(ActionName))
	{
		return *Found;
	}

	return EKeys::Invalid;
}

// ==========================================
// UMGInputUtility
// ==========================================

TSoftObjectPtr<UTexture2D> UMGInputUtility::GetKeyIcon(FKey Key, bool bGamepad)
{
	// Return icon texture paths based on key type
	// Icons are expected at /Game/UI/Icons/Input/
	// Naming convention: Xbox_{Button}, PS_{Button}, Keyboard_{Key}

	FString IconPath;

	if (bGamepad)
	{
		// Xbox controller icons (default for gamepad)
		static TMap<FKey, FString> GamepadIcons = {
			{ EKeys::Gamepad_FaceButton_Bottom, TEXT("/Game/UI/Icons/Input/Xbox_A") },
			{ EKeys::Gamepad_FaceButton_Right, TEXT("/Game/UI/Icons/Input/Xbox_B") },
			{ EKeys::Gamepad_FaceButton_Left, TEXT("/Game/UI/Icons/Input/Xbox_X") },
			{ EKeys::Gamepad_FaceButton_Top, TEXT("/Game/UI/Icons/Input/Xbox_Y") },
			{ EKeys::Gamepad_LeftShoulder, TEXT("/Game/UI/Icons/Input/Xbox_LB") },
			{ EKeys::Gamepad_RightShoulder, TEXT("/Game/UI/Icons/Input/Xbox_RB") },
			{ EKeys::Gamepad_LeftTrigger, TEXT("/Game/UI/Icons/Input/Xbox_LT") },
			{ EKeys::Gamepad_RightTrigger, TEXT("/Game/UI/Icons/Input/Xbox_RT") },
			{ EKeys::Gamepad_LeftThumbstick, TEXT("/Game/UI/Icons/Input/Xbox_LS") },
			{ EKeys::Gamepad_RightThumbstick, TEXT("/Game/UI/Icons/Input/Xbox_RS") },
			{ EKeys::Gamepad_DPad_Up, TEXT("/Game/UI/Icons/Input/Xbox_DPad_Up") },
			{ EKeys::Gamepad_DPad_Down, TEXT("/Game/UI/Icons/Input/Xbox_DPad_Down") },
			{ EKeys::Gamepad_DPad_Left, TEXT("/Game/UI/Icons/Input/Xbox_DPad_Left") },
			{ EKeys::Gamepad_DPad_Right, TEXT("/Game/UI/Icons/Input/Xbox_DPad_Right") },
			{ EKeys::Gamepad_Special_Left, TEXT("/Game/UI/Icons/Input/Xbox_View") },
			{ EKeys::Gamepad_Special_Right, TEXT("/Game/UI/Icons/Input/Xbox_Menu") },
		};

		if (const FString* Found = GamepadIcons.Find(Key))
		{
			IconPath = *Found;
		}
	}
	else
	{
		// Keyboard/mouse icons
		static TMap<FKey, FString> KeyboardIcons = {
			{ EKeys::W, TEXT("/Game/UI/Icons/Input/Key_W") },
			{ EKeys::A, TEXT("/Game/UI/Icons/Input/Key_A") },
			{ EKeys::S, TEXT("/Game/UI/Icons/Input/Key_S") },
			{ EKeys::D, TEXT("/Game/UI/Icons/Input/Key_D") },
			{ EKeys::E, TEXT("/Game/UI/Icons/Input/Key_E") },
			{ EKeys::Q, TEXT("/Game/UI/Icons/Input/Key_Q") },
			{ EKeys::R, TEXT("/Game/UI/Icons/Input/Key_R") },
			{ EKeys::C, TEXT("/Game/UI/Icons/Input/Key_C") },
			{ EKeys::V, TEXT("/Game/UI/Icons/Input/Key_V") },
			{ EKeys::M, TEXT("/Game/UI/Icons/Input/Key_M") },
			{ EKeys::SpaceBar, TEXT("/Game/UI/Icons/Input/Key_Space") },
			{ EKeys::LeftShift, TEXT("/Game/UI/Icons/Input/Key_Shift") },
			{ EKeys::LeftControl, TEXT("/Game/UI/Icons/Input/Key_Ctrl") },
			{ EKeys::LeftAlt, TEXT("/Game/UI/Icons/Input/Key_Alt") },
			{ EKeys::Escape, TEXT("/Game/UI/Icons/Input/Key_Esc") },
			{ EKeys::Enter, TEXT("/Game/UI/Icons/Input/Key_Enter") },
			{ EKeys::LeftMouseButton, TEXT("/Game/UI/Icons/Input/Mouse_Left") },
			{ EKeys::RightMouseButton, TEXT("/Game/UI/Icons/Input/Mouse_Right") },
			{ EKeys::MiddleMouseButton, TEXT("/Game/UI/Icons/Input/Mouse_Middle") },
		};

		if (const FString* Found = KeyboardIcons.Find(Key))
		{
			IconPath = *Found;
		}
	}

	// Return soft reference to texture (actual loading deferred)
	if (!IconPath.IsEmpty())
	{
		return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconPath));
	}

	return nullptr;
}

FText UMGInputUtility::GetKeyDisplayText(FKey Key)
{
	// Special display names for common keys
	static TMap<FKey, FText> DisplayNames = {
		{ EKeys::Gamepad_FaceButton_Bottom, NSLOCTEXT("Input", "Xbox_A", "A") },
		{ EKeys::Gamepad_FaceButton_Right, NSLOCTEXT("Input", "Xbox_B", "B") },
		{ EKeys::Gamepad_FaceButton_Left, NSLOCTEXT("Input", "Xbox_X", "X") },
		{ EKeys::Gamepad_FaceButton_Top, NSLOCTEXT("Input", "Xbox_Y", "Y") },
		{ EKeys::Gamepad_LeftShoulder, NSLOCTEXT("Input", "Xbox_LB", "LB") },
		{ EKeys::Gamepad_RightShoulder, NSLOCTEXT("Input", "Xbox_RB", "RB") },
		{ EKeys::Gamepad_LeftTrigger, NSLOCTEXT("Input", "Xbox_LT", "LT") },
		{ EKeys::Gamepad_RightTrigger, NSLOCTEXT("Input", "Xbox_RT", "RT") },
		{ EKeys::Gamepad_LeftThumbstick, NSLOCTEXT("Input", "Xbox_LS", "LS") },
		{ EKeys::Gamepad_RightThumbstick, NSLOCTEXT("Input", "Xbox_RS", "RS") },
		{ EKeys::Gamepad_DPad_Up, NSLOCTEXT("Input", "DPad_Up", "D-Up") },
		{ EKeys::Gamepad_DPad_Down, NSLOCTEXT("Input", "DPad_Down", "D-Down") },
		{ EKeys::Gamepad_DPad_Left, NSLOCTEXT("Input", "DPad_Left", "D-Left") },
		{ EKeys::Gamepad_DPad_Right, NSLOCTEXT("Input", "DPad_Right", "D-Right") },
		{ EKeys::SpaceBar, NSLOCTEXT("Input", "Space", "Space") },
		{ EKeys::LeftShift, NSLOCTEXT("Input", "LShift", "Shift") },
		{ EKeys::LeftControl, NSLOCTEXT("Input", "LCtrl", "Ctrl") },
		{ EKeys::LeftAlt, NSLOCTEXT("Input", "LAlt", "Alt") },
		{ EKeys::Escape, NSLOCTEXT("Input", "Esc", "Esc") },
		{ EKeys::Enter, NSLOCTEXT("Input", "Enter", "Enter") },
		{ EKeys::BackSpace, NSLOCTEXT("Input", "Backspace", "Back") },
	};

	if (const FText* Found = DisplayNames.Find(Key))
	{
		return *Found;
	}

	// Fallback to key's display name
	return Key.GetDisplayName();
}

bool UMGInputUtility::IsUsingGamepad(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
	{
		return false;
	}

	// Use common subsystem to track last input device type
	// Check if the player has any active gamepad input by checking analog stick values
	// This detects gamepad usage based on recent input activity

	// Check if any gamepad axis has non-zero value (indicates gamepad is being used)
	float LeftStickX = PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
	float LeftStickY = PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftY);
	float RightStickX = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX);
	float RightStickY = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightY);
	float LeftTrigger = PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftTriggerAxis);
	float RightTrigger = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightTriggerAxis);

	// If any gamepad input is active, player is using gamepad
	const float Threshold = 0.1f;
	if (FMath::Abs(LeftStickX) > Threshold ||
		FMath::Abs(LeftStickY) > Threshold ||
		FMath::Abs(RightStickX) > Threshold ||
		FMath::Abs(RightStickY) > Threshold ||
		LeftTrigger > Threshold ||
		RightTrigger > Threshold)
	{
		return true;
	}

	// Also check for any gamepad button being pressed
	if (PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Bottom) ||
		PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Right) ||
		PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Left) ||
		PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Top) ||
		PC->IsInputKeyDown(EKeys::Gamepad_LeftShoulder) ||
		PC->IsInputKeyDown(EKeys::Gamepad_RightShoulder))
	{
		return true;
	}

	return false;
}

float UMGInputUtility::ApplyDeadZone(float Value, float DeadZone, float MaxValue)
{
	float AbsValue = FMath::Abs(Value);

	if (AbsValue < DeadZone)
	{
		return 0.0f;
	}

	// Remap value from [DeadZone, MaxValue] to [0, 1]
	float Sign = FMath::Sign(Value);
	float RemappedValue = (AbsValue - DeadZone) / (MaxValue - DeadZone);

	return Sign * FMath::Clamp(RemappedValue, 0.0f, 1.0f);
}

float UMGInputUtility::ApplySensitivityCurve(float Value, float Sensitivity, float Exponent)
{
	// Apply exponential curve for finer control at low values
	float Sign = FMath::Sign(Value);
	float AbsValue = FMath::Abs(Value);

	// Apply exponent for curve
	float CurvedValue = FMath::Pow(AbsValue, Exponent);

	// Apply sensitivity multiplier
	return Sign * CurvedValue * Sensitivity;
}

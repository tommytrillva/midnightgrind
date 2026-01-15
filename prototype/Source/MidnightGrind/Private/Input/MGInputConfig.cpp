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
	// TODO: Return actual icon textures from a data table or asset
	// This would typically reference textures like:
	// /Game/UI/Icons/Input/Xbox_A.Xbox_A
	// /Game/UI/Icons/Input/Keyboard_W.Keyboard_W

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

	// Check if any gamepad input was received recently
	// This is a simplified check - a real implementation would track input device
	return false; // TODO: Implement proper input device tracking
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

// Copyright Midnight Grind. All Rights Reserved.

#include "RacingWheel/MGRacingWheelInputDevice.h"
#include "RacingWheel/MGRacingWheelSubsystem.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "InputCoreTypes.h"

// Define custom FKeys for racing wheel axes and buttons
// These should be registered with the input system

FMGRacingWheelInputDevice::FMGRacingWheelInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler)
{
	// Initialize previous state
	PreviousState = FMGWheelState();
}

FMGRacingWheelInputDevice::~FMGRacingWheelInputDevice()
{
}

void FMGRacingWheelInputDevice::Tick(float DeltaTime)
{
	// Update connection state
	if (WheelSubsystem.IsValid())
	{
		bWheelConnected = WheelSubsystem->IsWheelConnected();
	}
	else
	{
		bWheelConnected = false;
	}
}

void FMGRacingWheelInputDevice::SendControllerEvents()
{
	if (!WheelSubsystem.IsValid() || !bWheelConnected)
	{
		return;
	}

	// Get current wheel state
	FMGWheelState CurrentState = WheelSubsystem->GetWheelState();

	// Send axis events for steering
	SendAxisEvent(EKeys::Gamepad_LeftX, CurrentState.SteeringNormalized, PreviousState.SteeringNormalized);

	// Send axis events for pedals
	// Map to right trigger (throttle) and left trigger (brake)
	SendAxisEvent(EKeys::Gamepad_RightTriggerAxis, CurrentState.ThrottlePedal, PreviousState.ThrottlePedal);
	SendAxisEvent(EKeys::Gamepad_LeftTriggerAxis, CurrentState.BrakePedal, PreviousState.BrakePedal);

	// Clutch could map to a custom axis or left Y
	SendAxisEvent(EKeys::Gamepad_LeftY, CurrentState.ClutchPedal, PreviousState.ClutchPedal);

	// Send button events
	for (int32 i = 0; i < 16; i++)
	{
		bool bCurrentPressed = (CurrentState.ButtonStates & (1 << i)) != 0;
		bool bPrevPressed = (PreviousState.ButtonStates & (1 << i)) != 0;

		FKey ButtonKey = GetWheelButtonKey(i);
		if (ButtonKey.IsValid())
		{
			SendButtonEvent(ButtonKey, bCurrentPressed, bPrevPressed);
		}
	}

	// Paddle shifters
	SendButtonEvent(EKeys::Gamepad_LeftShoulder, CurrentState.bLeftPaddlePressed, PreviousState.bLeftPaddlePressed);
	SendButtonEvent(EKeys::Gamepad_RightShoulder, CurrentState.bRightPaddlePressed, PreviousState.bRightPaddlePressed);

	// D-pad
	bool bDPadUp = (CurrentState.DPadDirection == 0 || CurrentState.DPadDirection == 1 || CurrentState.DPadDirection == 7);
	bool bDPadRight = (CurrentState.DPadDirection == 1 || CurrentState.DPadDirection == 2 || CurrentState.DPadDirection == 3);
	bool bDPadDown = (CurrentState.DPadDirection == 3 || CurrentState.DPadDirection == 4 || CurrentState.DPadDirection == 5);
	bool bDPadLeft = (CurrentState.DPadDirection == 5 || CurrentState.DPadDirection == 6 || CurrentState.DPadDirection == 7);

	bool bPrevDPadUp = (PreviousState.DPadDirection == 0 || PreviousState.DPadDirection == 1 || PreviousState.DPadDirection == 7);
	bool bPrevDPadRight = (PreviousState.DPadDirection == 1 || PreviousState.DPadDirection == 2 || PreviousState.DPadDirection == 3);
	bool bPrevDPadDown = (PreviousState.DPadDirection == 3 || PreviousState.DPadDirection == 4 || PreviousState.DPadDirection == 5);
	bool bPrevDPadLeft = (PreviousState.DPadDirection == 5 || PreviousState.DPadDirection == 6 || PreviousState.DPadDirection == 7);

	SendButtonEvent(EKeys::Gamepad_DPad_Up, bDPadUp, bPrevDPadUp);
	SendButtonEvent(EKeys::Gamepad_DPad_Right, bDPadRight, bPrevDPadRight);
	SendButtonEvent(EKeys::Gamepad_DPad_Down, bDPadDown, bPrevDPadDown);
	SendButtonEvent(EKeys::Gamepad_DPad_Left, bDPadLeft, bPrevDPadLeft);

	// Store current state for next frame
	PreviousState = CurrentState;
}

void FMGRacingWheelInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FMGRacingWheelInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

void FMGRacingWheelInputDevice::SetChannelValue(int32 InControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	// Force feedback through the wheel subsystem instead
	// This is called by UE's standard FFB system (PlayDynamicForceFeedback)
	// We intercept and route to DirectInput FFB

	if (InControllerId != ControllerId || !WheelSubsystem.IsValid())
	{
		return;
	}

	// Map UE FFB channels to wheel FFB
	// For wheels, we typically want directional force, not left/right vibration
	if (ChannelType == FForceFeedbackChannelType::LEFT_LARGE ||
		ChannelType == FForceFeedbackChannelType::RIGHT_LARGE)
	{
		// Combine left and right as a single constant force
		// This is a simplified mapping - in practice you might want more sophisticated handling
	}
}

void FMGRacingWheelInputDevice::SetChannelValues(int32 InControllerId, const FForceFeedbackValues& Values)
{
	if (InControllerId != ControllerId || !WheelSubsystem.IsValid())
	{
		return;
	}

	// Similar to SetChannelValue but with all channels at once
	// We could use the difference between left and right for directional force
	float LeftForce = FMath::Max(Values.LeftLarge, Values.LeftSmall);
	float RightForce = FMath::Max(Values.RightLarge, Values.RightSmall);

	// If there's a significant difference, apply as directional force
	float ForceDifference = RightForce - LeftForce;
	float TotalForce = (LeftForce + RightForce) * 0.5f;

	// This could trigger an FFB effect, but we prefer to use the dedicated FFB system
	// through the wheel subsystem for proper DirectInput FFB
}

bool FMGRacingWheelInputDevice::IsGamepadAttached() const
{
	return bWheelConnected;
}

void FMGRacingWheelInputDevice::SetWheelSubsystem(UMGRacingWheelSubsystem* InSubsystem)
{
	WheelSubsystem = InSubsystem;
}

bool FMGRacingWheelInputDevice::IsWheelConnected() const
{
	return bWheelConnected;
}

void FMGRacingWheelInputDevice::SendAxisEvent(FKey Key, float Value, float PreviousValue)
{
	// Only send if value changed significantly
	if (FMath::Abs(Value - PreviousValue) > 0.001f)
	{
		MessageHandler->OnControllerAnalog(Key.GetFName(), ControllerId, Value);
	}
}

void FMGRacingWheelInputDevice::SendButtonEvent(FKey Key, bool bPressed, bool bWasPressed)
{
	if (bPressed && !bWasPressed)
	{
		MessageHandler->OnControllerButtonPressed(Key.GetFName(), ControllerId, false);
	}
	else if (!bPressed && bWasPressed)
	{
		MessageHandler->OnControllerButtonReleased(Key.GetFName(), ControllerId, false);
	}
}

FKey FMGRacingWheelInputDevice::GetWheelAxisKey(int32 AxisIndex) const
{
	switch (AxisIndex)
	{
	case 0: return EKeys::Gamepad_LeftX;       // Steering
	case 1: return EKeys::Gamepad_RightTriggerAxis; // Throttle
	case 2: return EKeys::Gamepad_LeftTriggerAxis;  // Brake
	case 3: return EKeys::Gamepad_LeftY;       // Clutch
	default: return FKey();
	}
}

FKey FMGRacingWheelInputDevice::GetWheelButtonKey(int32 ButtonIndex) const
{
	// Map wheel buttons to gamepad buttons
	// This mapping is based on typical Logitech wheel layout
	switch (ButtonIndex)
	{
	case 0: return EKeys::Gamepad_FaceButton_Bottom; // A / X
	case 1: return EKeys::Gamepad_FaceButton_Right;  // B / Circle
	case 2: return EKeys::Gamepad_FaceButton_Left;   // X / Square
	case 3: return EKeys::Gamepad_FaceButton_Top;    // Y / Triangle
	case 4: return EKeys::Gamepad_LeftShoulder;      // Left paddle
	case 5: return EKeys::Gamepad_RightShoulder;     // Right paddle
	case 6: return EKeys::Gamepad_Special_Left;      // Menu/Select
	case 7: return EKeys::Gamepad_Special_Right;     // Start/Options
	case 8: return EKeys::Gamepad_LeftThumbstick;    // Left stick click (if available)
	case 9: return EKeys::Gamepad_RightThumbstick;   // Right stick click (if available)
	case 10: return EKeys::Gamepad_LeftTrigger;      // Left trigger button
	case 11: return EKeys::Gamepad_RightTrigger;     // Right trigger button
	default: return FKey();
	}
}

// Module implementation
FMGRacingWheelInputDeviceModule& FMGRacingWheelInputDeviceModule::Get()
{
	static FMGRacingWheelInputDeviceModule Instance;
	return Instance;
}

TSharedPtr<IInputDevice> FMGRacingWheelInputDeviceModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	InputDevice = MakeShared<FMGRacingWheelInputDevice>(InMessageHandler);
	return InputDevice;
}

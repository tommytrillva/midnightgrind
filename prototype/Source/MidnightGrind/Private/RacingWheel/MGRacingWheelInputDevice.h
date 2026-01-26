// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IInputDevice.h"
#include "IInputDeviceModule.h"
#include "RacingWheel/MGRacingWheelTypes.h"

class UMGRacingWheelSubsystem;

/**
 * Racing Wheel Input Device
 *
 * Implements Unreal's IInputDevice interface to integrate racing wheel input
 * with the Enhanced Input System.
 *
 * This bridges the gap between our DirectInput-based wheel subsystem and
 * UE5's standard input processing pipeline.
 */
class FMGRacingWheelInputDevice : public IInputDevice
{
public:
	FMGRacingWheelInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);
	virtual ~FMGRacingWheelInputDevice();

	// IInputDevice interface
	virtual void Tick(float DeltaTime) override;
	virtual void SendControllerEvents() override;
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override;
	virtual bool IsGamepadAttached() const override;

	/**
	 * Set the wheel subsystem reference
	 */
	void SetWheelSubsystem(UMGRacingWheelSubsystem* InSubsystem);

	/**
	 * Check if wheel is connected
	 */
	bool IsWheelConnected() const;

private:
	/** Message handler for input events */
	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;

	/** Wheel subsystem reference */
	TWeakObjectPtr<UMGRacingWheelSubsystem> WheelSubsystem;

	/** Controller ID for this wheel */
	int32 ControllerId = 0;

	/** Previous input state for change detection */
	FMGWheelState PreviousState;

	/** Is the wheel connected */
	bool bWheelConnected = false;

	/** Map wheel axes to input keys */
	void SendAxisEvent(FKey Key, float Value, float PreviousValue);

	/** Map wheel buttons to input keys */
	void SendButtonEvent(FKey Key, bool bPressed, bool bWasPressed);

	/** Get the Enhanced Input key for a wheel axis */
	FKey GetWheelAxisKey(int32 AxisIndex) const;

	/** Get the Enhanced Input key for a wheel button */
	FKey GetWheelButtonKey(int32 ButtonIndex) const;
};

/**
 * Input Device Module for Racing Wheel
 *
 * Factory class that creates FMGRacingWheelInputDevice instances.
 */
class FMGRacingWheelInputDeviceModule : public IInputDeviceModule
{
public:
	static FMGRacingWheelInputDeviceModule& Get();

	/**
	 * Create the input device
	 */
	virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	/**
	 * Get the created input device
	 */
	TSharedPtr<FMGRacingWheelInputDevice> GetInputDevice() const { return InputDevice; }

private:
	TSharedPtr<FMGRacingWheelInputDevice> InputDevice;
};

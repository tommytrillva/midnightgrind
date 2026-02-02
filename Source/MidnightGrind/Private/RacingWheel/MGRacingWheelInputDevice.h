// Copyright Midnight Grind. All Rights Reserved.

/*******************************************************************************
 * FILE: MGRacingWheelInputDevice.h
 *
 * PURPOSE:
 * This file defines the bridge between physical racing wheel hardware and
 * Unreal Engine's input system. It contains two classes that work together
 * to make racing wheel input feel native to UE5.
 *
 * WHAT THIS FILE DOES:
 * - Translates raw racing wheel data (steering angle, pedal positions, buttons)
 *   into standard Unreal Engine input events
 * - Allows racing wheels to work with UE5's Enhanced Input System, meaning
 *   you can bind wheel controls in Input Action assets just like gamepad buttons
 * - Handles force feedback (rumble/resistance) requests from the game
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 *
 * 1. IInputDevice Interface:
 *    Unreal provides this interface for custom input hardware. By implementing
 *    it, our racing wheel becomes a "first-class citizen" in UE5's input world.
 *    The engine calls methods like Tick() and SendControllerEvents() every frame.
 *
 * 2. IInputDeviceModule Interface:
 *    This is a "factory" pattern - it creates instances of our input device.
 *    Unreal's input system uses this to instantiate our device at startup.
 *
 * 3. Message Handler:
 *    FGenericApplicationMessageHandler is how we send input events to Unreal.
 *    When the player turns the wheel, we call methods on this handler to
 *    notify the engine "hey, this axis moved to this value."
 *
 * 4. FKey:
 *    Unreal represents all input (keyboard keys, gamepad buttons, axes) as FKey.
 *    We map wheel axes and buttons to FKey values so they can be used in
 *    Input Action bindings.
 *
 * 5. Force Feedback Channels:
 *    Games can request haptic feedback through "channels" (like left/right motors).
 *    We receive these requests and translate them into DirectInput force effects.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 *
 *   Physical Wheel Hardware
 *          |
 *          v
 *   [MGDirectInputManager] - Low-level Windows DirectInput API calls
 *          |
 *          v
 *   [MGRacingWheelSubsystem] - Game-level wheel management (UE5 subsystem)
 *          |
 *          v
 *   [FMGRacingWheelInputDevice] <-- THIS FILE - Bridges to UE5 input system
 *          |
 *          v
 *   Unreal Enhanced Input System (Input Actions, Input Mappings)
 *          |
 *          v
 *   Your Game Code (Blueprints, C++ receiving input)
 *
 * USAGE FLOW:
 * 1. At engine startup, FMGRacingWheelInputDeviceModule::CreateInputDevice() is called
 * 2. Every frame, Tick() updates internal state from the wheel subsystem
 * 3. SendControllerEvents() fires input events for any changed axes/buttons
 * 4. When game wants force feedback, SetChannelValue/SetChannelValues() is called
 *
 * RELATED FILES:
 * - MGRacingWheelSubsystem.h - The subsystem that manages wheel connection/state
 * - MGRacingWheelTypes.h - Data structures like FMGWheelState
 * - MGDirectInputManager.h - Low-level DirectInput implementation (Windows only)
 *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "IInputDevice.h"
#include "IInputDeviceModule.h"
#include "RacingWheel/MGRacingWheelTypes.h"

// Forward declaration - the actual subsystem that manages wheel hardware
class UMGRacingWheelSubsystem;

/*******************************************************************************
 * CLASS: FMGRacingWheelInputDevice
 *
 * The main input device class that implements Unreal's IInputDevice interface.
 *
 * Think of this as a "translator" - it speaks two languages:
 * - It understands our custom wheel subsystem's data format (FMGWheelState)
 * - It can express that data in Unreal's input language (FKey events)
 *
 * IMPORTANT METHODS:
 * - Tick(): Called every frame, use for polling/state updates
 * - SendControllerEvents(): Called every frame to dispatch input events
 * - SetChannelValue(): Receives force feedback requests from the game
 *
 * The "F" prefix is Unreal convention for non-UObject C++ classes.
 ******************************************************************************/
class FMGRacingWheelInputDevice : public IInputDevice
{
public:
	FMGRacingWheelInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);
	virtual ~FMGRacingWheelInputDevice();

	// =========================================================================
	// IInputDevice Interface Implementation
	// These methods are called by Unreal's input system every frame
	// =========================================================================

	/**
	 * Called every frame by the input system.
	 * Use this for any per-frame updates that don't involve sending events.
	 * @param DeltaTime - Time since last frame in seconds
	 */
	virtual void Tick(float MGDeltaTime) override;

	/**
	 * Called every frame to dispatch input events to Unreal.
	 * This is where we compare current vs. previous state and fire events
	 * for any axes that moved or buttons that changed.
	 */
	virtual void SendControllerEvents() override;

	/**
	 * Called when Unreal wants to change the message handler.
	 * The message handler is how we send input events to the engine.
	 * @param InMessageHandler - The new message handler to use
	 */
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	/**
	 * Console command handler - allows debug commands like "wheel status".
	 * @param InWorld - Current world context
	 * @param Cmd - The command string to parse
	 * @param Ar - Output device for printing results
	 * @return True if we handled the command
	 */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	/**
	 * Receives force feedback requests from the game (single channel).
	 * Called when game code triggers haptic feedback, e.g., from a UForceFeedbackEffect.
	 * @param ControllerId - Which controller (we check if it matches our wheel)
	 * @param ChannelType - Which motor/channel (left, right, etc.)
	 * @param Value - Intensity from 0.0 to 1.0
	 */
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;

	/**
	 * Receives force feedback requests (all channels at once).
	 * More efficient than multiple SetChannelValue calls.
	 * @param ControllerId - Which controller
	 * @param Values - Struct containing all channel intensities
	 */
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override;

	/**
	 * Tells Unreal whether a "gamepad" is connected.
	 * We return true when our wheel is connected so Unreal knows
	 * there's a valid input device available.
	 * @return True if wheel is connected and working
	 */
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
	// =========================================================================
	// Member Variables
	// =========================================================================

	/**
	 * Message handler for sending input events to Unreal.
	 * TSharedRef = a non-nullable smart pointer (always valid after construction).
	 * This is Unreal's way of ensuring we always have a valid handler.
	 */
	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;

	/**
	 * Weak reference to the wheel subsystem.
	 * TWeakObjectPtr = a smart pointer that doesn't prevent garbage collection.
	 * We use weak because the subsystem is owned by the game instance,
	 * and we don't want to keep it alive if the game shuts down.
	 */
	TWeakObjectPtr<UMGRacingWheelSubsystem> WheelSubsystem;

	/**
	 * Controller ID assigned to this wheel.
	 * Unreal supports multiple controllers (player 1, player 2, etc.).
	 * We default to 0 (first controller) since most setups have one wheel.
	 */
	int32 ControllerId = 0;

	/**
	 * Stores the input state from the previous frame.
	 * We compare this against current state to detect changes.
	 * Example: If steering was 0.5 last frame and is 0.6 now, we fire an event.
	 */
	FMGWheelState PreviousState;

	/**
	 * Cached connection status.
	 * Checking actual hardware every frame is expensive, so we cache this.
	 */
	bool bWheelConnected = false;

	// =========================================================================
	// Helper Methods
	// =========================================================================

	/**
	 * Sends an axis (analog) input event to Unreal.
	 * Only sends if the value actually changed to avoid spamming events.
	 * @param Key - The FKey representing this axis (e.g., "GenericUSBController_Axis1")
	 * @param Value - Current axis value (typically -1.0 to 1.0 or 0.0 to 1.0)
	 * @param PreviousValue - Last frame's value for change detection
	 */
	void SendAxisEvent(FKey Key, float Value, float PreviousValue);

	/**
	 * Sends a button (digital) input event to Unreal.
	 * Fires "pressed" or "released" events when state changes.
	 * @param Key - The FKey representing this button
	 * @param bPressed - Is the button currently pressed?
	 * @param bWasPressed - Was it pressed last frame?
	 */
	void SendButtonEvent(FKey Key, bool bPressed, bool bWasPressed);

	/**
	 * Maps a wheel axis index to an Unreal FKey.
	 * Example: Axis 0 (steering) -> "GenericUSBController_Axis1"
	 * This allows wheel axes to be bound in Input Action assets.
	 * @param AxisIndex - 0=steering, 1=throttle, 2=brake, 3=clutch
	 * @return The corresponding FKey for this axis
	 */
	FKey GetWheelAxisKey(int32 AxisIndex) const;

	/**
	 * Maps a wheel button index to an Unreal FKey.
	 * Example: Button 0 -> "GenericUSBController_Button1"
	 * @param ButtonIndex - Which button (0-31 typically)
	 * @return The corresponding FKey for this button
	 */
	FKey GetWheelButtonKey(int32 ButtonIndex) const;
};

/*******************************************************************************
 * CLASS: FMGRacingWheelInputDeviceModule
 *
 * This is a "Factory" class - its job is to create FMGRacingWheelInputDevice
 * instances when Unreal's input system asks for them.
 *
 * WHY DO WE NEED THIS?
 * Unreal's modular design means input devices are loaded dynamically.
 * The engine doesn't know about our wheel device at compile time.
 * Instead, it asks "hey module, please create your input device" at runtime.
 *
 * THE SINGLETON PATTERN:
 * Notice the static Get() method - this implements the "Singleton" pattern.
 * There's only ever ONE module instance, ensuring we don't accidentally
 * create multiple wheel devices fighting for the same hardware.
 *
 * LIFECYCLE:
 * 1. Engine starts up
 * 2. Input system calls Get() to find our module
 * 3. Input system calls CreateInputDevice() to get our device
 * 4. Our device is then polled every frame until shutdown
 *
 ******************************************************************************/
class FMGRacingWheelInputDeviceModule : public IInputDeviceModule
{
public:
	/**
	 * Get the singleton instance of this module.
	 * "Singleton" = only one instance ever exists.
	 * @return Reference to the one and only module instance
	 */
	static FMGRacingWheelInputDeviceModule& Get();

	/**
	 * Factory method called by Unreal to create our input device.
	 * This is called once during engine initialization.
	 * @param InMessageHandler - The handler our device should use for events
	 * @return Shared pointer to the created input device
	 */
	virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	/**
	 * Accessor to get the input device after it's been created.
	 * Useful for other systems that need to interact with the wheel device
	 * (like the subsystem setting itself as the data source).
	 * @return Shared pointer to our wheel input device (may be null if not created yet)
	 */
	TSharedPtr<FMGRacingWheelInputDevice> GetInputDevice() const { return InputDevice; }

private:
	/**
	 * Cached pointer to the input device we created.
	 * TSharedPtr = nullable smart pointer with reference counting.
	 * The device stays alive as long as either we or Unreal hold a reference.
	 */
	TSharedPtr<FMGRacingWheelInputDevice> InputDevice;
};

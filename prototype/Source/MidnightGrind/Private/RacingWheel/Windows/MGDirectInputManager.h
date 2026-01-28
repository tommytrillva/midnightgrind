// Copyright Midnight Grind. All Rights Reserved.

/*******************************************************************************
 * FILE: MGDirectInputManager.h
 *
 * PURPOSE:
 * This file provides low-level access to racing wheel hardware on Windows
 * using Microsoft's DirectInput API. It's the foundation layer that actually
 * talks to the physical wheel device.
 *
 * WHAT THIS FILE DOES:
 * - Discovers racing wheels plugged into the computer
 * - Reads raw input data (steering position, pedal positions, button states)
 * - Sends force feedback effects to the wheel (rumble, resistance, etc.)
 * - Manages the lifecycle of wheel connections
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 *
 * 1. DirectInput (DirectInput8 / DInput8):
 *    A Microsoft API from the DirectX family specifically for input devices.
 *    While newer APIs exist (XInput, Windows.Gaming.Input), DirectInput
 *    remains the standard for racing wheels because:
 *    - It supports many axes (steering, throttle, brake, clutch, handbrake)
 *    - It has rich force feedback support
 *    - Most wheel manufacturers target DirectInput
 *
 * 2. Device Enumeration:
 *    The process of discovering what devices are connected. DirectInput
 *    uses a "callback" pattern - you provide a function, and DirectInput
 *    calls it once for each device it finds.
 *
 * 3. Device Acquisition:
 *    Before reading from a device, you must "acquire" it. This tells Windows
 *    your application wants exclusive (or shared) access. If another app
 *    has the device, acquisition may fail.
 *
 * 4. Force Feedback (FFB):
 *    The wheel's ability to push back against the player. Effects include:
 *    - Constant Force: Steady push in one direction (e.g., simulating wind)
 *    - Spring: Resistance that increases with distance from center
 *    - Damper: Resistance proportional to speed of movement
 *    - Friction: Constant resistance to any movement
 *    - Periodic: Vibration patterns (sine wave, square wave, etc.)
 *
 * 5. GUID (Globally Unique Identifier):
 *    Windows uses 128-bit identifiers for devices and effect types.
 *    Example: GUID_ConstantForce identifies the constant force effect type.
 *
 * 6. Polling vs. Event-Driven:
 *    This implementation uses polling - we actively ask the device
 *    "what's your current state?" every frame. This is simple and reliable
 *    for game input where we need continuous position data.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 *
 *   Physical Racing Wheel (USB)
 *          |
 *          v
 *   Windows DirectInput Driver (dinput8.dll)
 *          |
 *          v
 *   [FMGDirectInputManager] <-- THIS FILE - Our wrapper around DirectInput
 *          |
 *          v
 *   [UMGRacingWheelSubsystem] - UE5 subsystem layer
 *          |
 *          v
 *   [FMGRacingWheelInputDevice] - Integration with UE5 input
 *          |
 *          v
 *   Your Game Code
 *
 * PLATFORM NOTES:
 * - This file is Windows-only (#if PLATFORM_WINDOWS)
 * - On other platforms (Mac, Linux, consoles), different implementations
 *   would be needed (SDL2, platform-specific APIs, etc.)
 * - Requires linking: dinput8.lib, dxguid.lib
 *
 * COMMON ISSUES:
 * - Device not found: Check USB connection, drivers installed
 * - Acquisition fails: Another app may have exclusive access (close it)
 * - FFB not working: Not all wheels support all effect types
 * - Values seem wrong: Check axis calibration in Windows Game Controllers
 *
 * RELATED FILES:
 * - MGRacingWheelTypes.h - Data structures used here (FMGWheelState, FMGFFBEffect)
 * - MGRacingWheelSubsystem.cpp - The subsystem that uses this class
 *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"

// PLATFORM_WINDOWS is a UE4/5 macro that's true only when compiling for Windows.
// This entire file is skipped on Mac, Linux, consoles, etc.
#if PLATFORM_WINDOWS

#include "RacingWheel/MGRacingWheelTypes.h"

/*******************************************************************************
 * DirectInput Type Forward Declarations
 *
 * These are the core DirectInput interfaces we use. We forward-declare them
 * here rather than #including the Windows headers to:
 * - Speed up compilation (Windows headers are huge)
 * - Avoid polluting the global namespace with Windows macros
 *
 * The actual definitions come from <dinput.h> in the .cpp file.
 ******************************************************************************/

// IDirectInput8W: The main DirectInput interface (W = Wide/Unicode version)
// This is our entry point - we use it to enumerate and create device objects.
struct IDirectInput8W;

// IDirectInputDevice8W: Represents a single input device (one wheel)
// We use this to poll input state and create force feedback effects.
struct IDirectInputDevice8W;

// IDirectInputEffect: Represents a single force feedback effect
// Each rumble, spring, or resistance effect is one of these objects.
struct IDirectInputEffect;

// LP = "Long Pointer" - classic Windows naming for pointers
// These typedefs make code cleaner: LPDIRECTINPUT8W instead of IDirectInput8W*
typedef struct IDirectInput8W* LPDIRECTINPUT8W;
typedef struct IDirectInputDevice8W* LPDIRECTINPUTDEVICE8W;
typedef struct IDirectInputEffect* LPDIRECTINPUTEFFECT;

/*******************************************************************************
 * CLASS: FMGDirectInputManager
 *
 * Manages all DirectInput operations for racing wheels.
 *
 * USAGE PATTERN:
 * 1. Create instance: FMGDirectInputManager Manager;
 * 2. Initialize:      Manager.Initialize();
 * 3. Find wheels:     int32 Count = Manager.EnumerateDevices();
 * 4. Acquire wheel:   Manager.AcquireDevice(0);  // First wheel
 * 5. Each frame:
 *    - Manager.PollDevice(0);
 *    - Manager.GetInputState(0, OutState);
 * 6. For FFB:         FGuid EffectId = Manager.CreateEffect(0, Effect, 1.0f);
 * 7. Cleanup:         Manager.Shutdown();
 *
 * THREAD SAFETY:
 * This class is NOT thread-safe. All calls should be made from the game thread.
 * DirectInput operations should not be performed from multiple threads.
 *
 ******************************************************************************/
class FMGDirectInputManager
{
public:
	// =========================================================================
	// Constructor / Destructor
	// =========================================================================

	/**
	 * Constructor - initializes member variables to safe defaults.
	 * Does NOT initialize DirectInput - call Initialize() for that.
	 */
	FMGDirectInputManager();

	/**
	 * Destructor - automatically calls Shutdown() to clean up resources.
	 * Always release DirectInput resources properly to avoid driver issues.
	 */
	~FMGDirectInputManager();

	// =========================================================================
	// Initialization & Shutdown
	// =========================================================================

	/**
	 * Initialize the DirectInput system.
	 *
	 * This creates the main IDirectInput8 interface by calling DirectInput8Create().
	 * Must be called before any other methods.
	 *
	 * WHAT IT DOES:
	 * 1. Loads dinput8.dll if not already loaded
	 * 2. Creates the main DirectInput interface
	 * 3. Prepares for device enumeration
	 *
	 * @return True if initialization succeeded, false if DirectInput unavailable
	 */
	bool Initialize();

	/**
	 * Shutdown and release all DirectInput resources.
	 *
	 * IMPORTANT: Always call this before destroying the manager, or when
	 * you're done with wheel input. Failing to release resources can cause:
	 * - Memory leaks
	 * - Driver issues requiring restart
	 * - Other applications unable to access the wheel
	 *
	 * WHAT IT DOES:
	 * 1. Stops all active force feedback effects
	 * 2. Releases all device interfaces
	 * 3. Releases the main DirectInput interface
	 */
	void Shutdown();

	// =========================================================================
	// Device Discovery
	// =========================================================================

	/**
	 * Scan for connected DirectInput devices (wheels, joysticks, gamepads).
	 *
	 * Call this to refresh the list of available devices. Useful when:
	 * - At startup to find initially connected wheels
	 * - When user reports wheel not detected
	 * - After a USB device change notification
	 *
	 * HOW IT WORKS:
	 * Uses DirectInput's EnumDevices() with a callback. For each device found,
	 * our callback (EnumDevicesCallback) is invoked with device info.
	 *
	 * @return Number of devices found (0 if no wheels connected)
	 */
	int32 EnumerateDevices();

	/**
	 * Get identifying information for a discovered device.
	 *
	 * USB devices have Vendor ID (VID) and Product ID (PID) that uniquely
	 * identify the manufacturer and model. Examples:
	 * - Logitech G29: VID=0x046D, PID=0xC24F
	 * - Thrustmaster T300: VID=0x044F, PID=0xB66E
	 *
	 * You can use this to detect specific wheel models and apply
	 * model-specific settings or force feedback tuning.
	 *
	 * @param DeviceIndex - Index from 0 to (EnumerateDevices()-1)
	 * @param OutVendorID - Receives the USB Vendor ID
	 * @param OutProductID - Receives the USB Product ID
	 * @param OutDeviceName - Receives the human-readable device name
	 * @return True if device exists at this index, false if out of range
	 */
	bool GetDeviceInfo(int32 DeviceIndex, int32& OutVendorID, int32& OutProductID, FString& OutDeviceName) const;

	// =========================================================================
	// Device Acquisition & Polling
	// =========================================================================

	/**
	 * Acquire a device for use by this application.
	 *
	 * Before you can read input or send FFB, you must "acquire" the device.
	 * This tells Windows your app wants to use the wheel.
	 *
	 * ACQUISITION MODES:
	 * - Exclusive: Only your app can use the device (needed for some FFB)
	 * - Non-exclusive: Multiple apps can read from the device
	 * We typically request exclusive access for full FFB support.
	 *
	 * COMMON FAILURES:
	 * - Device unplugged
	 * - Another app has exclusive access (close it first)
	 * - Driver issues (try replugging the wheel)
	 *
	 * @param DeviceIndex - Which device to acquire (0 = first wheel found)
	 * @return True if acquisition succeeded, false otherwise
	 */
	bool AcquireDevice(int32 DeviceIndex);

	/**
	 * Release a previously acquired device.
	 *
	 * Call this when:
	 * - Switching to a different wheel
	 * - Application is minimizing/losing focus
	 * - Shutting down
	 *
	 * After releasing, the device can be used by other applications.
	 *
	 * @param DeviceIndex - Which device to release
	 */
	void ReleaseDevice(int32 DeviceIndex);

	/**
	 * Poll the device to update its input state.
	 *
	 * IMPORTANT: You must call this every frame BEFORE GetInputState().
	 * Polling tells the device to update its internal buffers with the
	 * latest physical positions of all axes and buttons.
	 *
	 * RETURN VALUE:
	 * - True: Poll succeeded, device is healthy
	 * - False: Poll failed, device may be disconnected or lost
	 *
	 * If polling fails, you should:
	 * 1. Check if the device is still connected
	 * 2. Try to re-acquire it
	 * 3. If still failing, re-enumerate devices
	 *
	 * @param DeviceIndex - Which device to poll
	 * @return True if poll succeeded, false if device may be lost
	 */
	bool PollDevice(int32 DeviceIndex);

	/**
	 * Get the current input state (steering, pedals, buttons) from a device.
	 *
	 * Call this after PollDevice() to read the latest input values.
	 * The state is copied into your provided structure.
	 *
	 * AXIS VALUES:
	 * - Steering: -1.0 (full left) to +1.0 (full right)
	 * - Throttle: 0.0 (released) to 1.0 (fully pressed)
	 * - Brake: 0.0 (released) to 1.0 (fully pressed)
	 * - Clutch: 0.0 (released) to 1.0 (fully pressed)
	 *
	 * @param DeviceIndex - Which device to read
	 * @param OutState - Structure to receive the input state
	 */
	void GetInputState(int32 DeviceIndex, FMGWheelState& OutState) const;

	// =========================================================================
	// Force Feedback - Query Capabilities
	// =========================================================================

	/**
	 * Check if a device supports force feedback.
	 *
	 * Not all wheels support FFB. Entry-level wheels often lack motors.
	 * Call this before trying to create effects.
	 *
	 * @param DeviceIndex - Which device to check
	 * @return True if the device has force feedback capability
	 */
	bool SupportsFFB(int32 DeviceIndex) const;

	/**
	 * Get the list of FFB effect types this device supports.
	 *
	 * Different wheels support different effects. A high-end wheel might
	 * support everything, while a budget wheel might only support
	 * constant force and periodic effects.
	 *
	 * COMMON EFFECT TYPES:
	 * - ConstantForce: Push in one direction (steering resistance)
	 * - Spring: Resistance proportional to displacement
	 * - Damper: Resistance proportional to velocity
	 * - Friction: Constant resistance to any movement
	 * - Periodic/Sine: Oscillating vibration
	 *
	 * @param DeviceIndex - Which device to query
	 * @return Array of supported effect types (may be empty)
	 */
	TArray<EMGFFBEffectType> GetSupportedEffects(int32 DeviceIndex) const;

	// =========================================================================
	// Force Feedback - Effect Lifecycle
	// =========================================================================

	/**
	 * Create a new force feedback effect and start playing it.
	 *
	 * EFFECT LIFECYCLE:
	 * 1. CreateEffect() - Creates effect in device memory, starts playing
	 * 2. UpdateEffect() - Modify parameters while playing
	 * 3. StopEffect() - Stop the effect (can restart with StartEffect)
	 * 4. Effect destroyed when device released or new effect replaces it
	 *
	 * EXAMPLE - Create a constant resistance effect:
	 *   FMGFFBEffect Effect;
	 *   Effect.Type = EMGFFBEffectType::ConstantForce;
	 *   Effect.Direction = 0; // Centered
	 *   Effect.Magnitude = 0.5f; // 50% strength
	 *   FGuid Id = Manager.CreateEffect(0, Effect, 1.0f);
	 *
	 * @param DeviceIndex - Which device to create the effect on
	 * @param Effect - Effect parameters (type, magnitude, duration, etc.)
	 * @param GlobalGain - Master intensity multiplier (0.0 to 1.0)
	 * @return Unique ID for this effect, use for UpdateEffect/StopEffect
	 */
	FGuid CreateEffect(int32 DeviceIndex, const FMGFFBEffect& Effect, float GlobalGain);

	/**
	 * Update an existing effect's parameters in real-time.
	 *
	 * Call this to smoothly change effect intensity without stopping/starting.
	 * Great for dynamic feedback that changes with game state, like:
	 * - Tire grip changing as you corner
	 * - Road texture effects
	 * - Collision impacts
	 *
	 * @param DeviceIndex - Which device owns the effect
	 * @param EffectID - ID returned from CreateEffect()
	 * @param Effect - New effect parameters
	 * @param GlobalGain - Master intensity multiplier
	 */
	void UpdateEffect(int32 DeviceIndex, FGuid EffectID, const FMGFFBEffect& Effect, float GlobalGain);

	/**
	 * Start a previously stopped effect.
	 *
	 * Effects remain in device memory after StopEffect().
	 * Use this to restart them without recreating.
	 *
	 * @param DeviceIndex - Which device owns the effect
	 * @param EffectID - ID of effect to start
	 */
	void StartEffect(int32 DeviceIndex, FGuid EffectID);

	/**
	 * Stop a currently playing effect.
	 *
	 * The effect remains in memory and can be restarted with StartEffect().
	 * To completely remove an effect, create a new effect with the same ID
	 * or release the device.
	 *
	 * @param DeviceIndex - Which device owns the effect
	 * @param EffectID - ID of effect to stop
	 */
	void StopEffect(int32 DeviceIndex, FGuid EffectID);

	/**
	 * Emergency stop - halt all effects on a device immediately.
	 *
	 * Use this when:
	 * - Game is paused
	 * - Player enters a menu
	 * - An error occurs
	 * - For safety (user reports wheel going crazy)
	 *
	 * @param DeviceIndex - Which device to silence
	 */
	void StopAllEffects(int32 DeviceIndex);

	// =========================================================================
	// Force Feedback - Global Settings
	// =========================================================================

	/**
	 * Set the master gain (overall FFB strength) for a device.
	 *
	 * This affects ALL effects on the device. Use for:
	 * - User preference setting ("FFB Strength" in options)
	 * - Temporary reduction (during cutscenes)
	 *
	 * DirectInput uses a 0-10000 scale internally.
	 *
	 * @param DeviceIndex - Which device to adjust
	 * @param Gain - Gain value from 0 (silent) to 10000 (full strength)
	 */
	void SetGain(int32 DeviceIndex, int32 Gain);

	/**
	 * Enable or disable the wheel's auto-center spring.
	 *
	 * WHAT IS AUTO-CENTER?
	 * Most wheels have a built-in spring that pulls them back to center.
	 * This is separate from your game's FFB effects.
	 *
	 * FOR RACING GAMES:
	 * - Usually DISABLE auto-center (bEnabled = false)
	 * - Your game should provide its own centering through FFB
	 * - The built-in spring feels artificial and fights your effects
	 *
	 * FOR MENUS/PAUSED:
	 * - Consider ENABLING so the wheel doesn't flop around
	 *
	 * @param DeviceIndex - Which device to configure
	 * @param bEnabled - True to enable auto-center, false to disable
	 */
	void SetAutoCenter(int32 DeviceIndex, bool bEnabled);

private:
	// =========================================================================
	// Internal Data Structures
	// =========================================================================

	/**
	 * Information about a single discovered/acquired device.
	 *
	 * We maintain one of these for each wheel found during enumeration.
	 * It holds both the DirectInput interface pointer and cached information
	 * about the device's capabilities.
	 */
	struct FDeviceInfo
	{
		/** The DirectInput device interface - our handle to talk to hardware */
		LPDIRECTINPUTDEVICE8W Device = nullptr;

		/** Human-readable name like "Logitech G29 Racing Wheel" */
		FString DeviceName;

		/** USB Vendor ID - identifies the manufacturer */
		int32 VendorID = 0;

		/** USB Product ID - identifies the specific product model */
		int32 ProductID = 0;

		/** Have we successfully acquired this device? */
		bool bIsAcquired = false;

		/** Does this device have force feedback motors? */
		bool bSupportsFFB = false;

		/** List of FFB effect types this device can play */
		TArray<EMGFFBEffectType> SupportedEffects;

		/** Current input state - updated each time PollDevice() is called */
		FMGWheelState CurrentState;

		/**
		 * Map of active force feedback effects.
		 * Key: Our FGuid identifier (returned from CreateEffect)
		 * Value: DirectInput's effect interface pointer
		 */
		TMap<FGuid, LPDIRECTINPUTEFFECT> ActiveEffects;
	};

	// =========================================================================
	// Member Variables
	// =========================================================================

	/**
	 * The main DirectInput8 interface.
	 * Created in Initialize(), released in Shutdown().
	 * This is our "entry point" to all DirectInput functionality.
	 */
	LPDIRECTINPUT8W DirectInput = nullptr;

	/**
	 * All devices discovered during enumeration.
	 * Index 0 = first device found, etc.
	 * Use DeviceIndex parameters to access specific devices.
	 */
	TArray<FDeviceInfo> Devices;

	/**
	 * Temporary storage for device GUIDs during enumeration.
	 * DirectInput identifies devices by GUID, so we store these
	 * to create device interfaces after enumeration completes.
	 */
	TArray<GUID> EnumeratedGUIDs;

	// =========================================================================
	// Static Callbacks for DirectInput Enumeration
	// =========================================================================

	/**
	 * Callback function invoked by DirectInput for each discovered device.
	 *
	 * WHY STATIC?
	 * DirectInput is a C API that uses function pointers for callbacks.
	 * It can't call non-static member functions directly. We use the
	 * Context parameter to pass "this" and access member data.
	 *
	 * CALLBACK PATTERN:
	 * 1. We call DirectInput->EnumDevices(..., EnumDevicesCallback, this)
	 * 2. DirectInput calls EnumDevicesCallback() once per device
	 * 3. We cast Context back to FMGDirectInputManager* to access members
	 * 4. Return DIENUM_CONTINUE to keep enumerating, DIENUM_STOP to stop
	 *
	 * @param DeviceInstance - DirectInput device info structure
	 * @param Context - User data (our "this" pointer)
	 * @return DIENUM_CONTINUE or DIENUM_STOP
	 */
	static int32 __stdcall EnumDevicesCallback(const void* DeviceInstance, void* Context);

	/**
	 * Callback function invoked when enumerating supported FFB effects.
	 * Same pattern as EnumDevicesCallback - called once per supported effect type.
	 *
	 * @param EffectInfo - Information about one supported effect type
	 * @param Context - User data (pointer to FDeviceInfo being queried)
	 * @return DIENUM_CONTINUE or DIENUM_STOP
	 */
	static int32 __stdcall EnumEffectsCallback(const void* EffectInfo, void* Context);

	// =========================================================================
	// Internal Helper Methods
	// =========================================================================

	/**
	 * Create a DirectInput effect object from our high-level effect description.
	 *
	 * Translates our game-friendly FMGFFBEffect structure into the complex
	 * DIEFFECT structure that DirectInput expects. Handles all the gnarly
	 * details like:
	 * - Allocating effect type-specific parameter structures
	 * - Converting our 0-1 ranges to DirectInput's 0-10000 ranges
	 * - Setting up axes and directions
	 *
	 * @param Device - The DirectInput device to create the effect on
	 * @param Effect - Our high-level effect description
	 * @param GlobalGain - Master gain multiplier
	 * @return DirectInput effect interface, or nullptr on failure
	 */
	LPDIRECTINPUTEFFECT CreateDIEffect(LPDIRECTINPUTDEVICE8W Device, const FMGFFBEffect& Effect, float GlobalGain);

	/**
	 * Convert our effect type enum to DirectInput's GUID for that effect type.
	 *
	 * DirectInput identifies effect types by GUID, not enum.
	 * Examples:
	 * - ConstantForce -> GUID_ConstantForce
	 * - Spring -> GUID_Spring
	 *
	 * @param EffectType - Our effect type enum
	 * @param OutGUID - Receives the DirectInput GUID
	 * @return True if conversion succeeded, false if unsupported type
	 */
	bool GetEffectGUID(EMGFFBEffectType EffectType, GUID& OutGUID) const;

	/**
	 * Convert DirectInput axis value to normalized float.
	 *
	 * DirectInput reports axes in different ranges:
	 * - Centered axes (steering): 0 to 65535, center at 32767
	 * - Non-centered axes (pedals): 0 to 65535, rest at 65535 (unpressed)
	 *
	 * We normalize to:
	 * - Centered: -1.0 to +1.0
	 * - Non-centered: 0.0 to 1.0
	 *
	 * @param Value - Raw DirectInput value (0-65535)
	 * @param bCentered - True for steering-type axes, false for pedals
	 * @return Normalized value
	 */
	float NormalizeAxis(int32 Value, bool bCentered = true) const;

	/**
	 * Convert our 0-1 magnitude to DirectInput's 0-10000 range.
	 *
	 * DirectInput FFB uses "DI_FFNOMINALMAX" (10000) as full strength.
	 * This helper handles the conversion and applies global gain.
	 *
	 * @param Magnitude - Our magnitude (0.0 to 1.0)
	 * @param GlobalGain - Global gain multiplier (0.0 to 1.0)
	 * @return DirectInput magnitude (0 to 10000)
	 */
	int32 ScaleMagnitude(float Magnitude, float GlobalGain) const;

	/**
	 * Extract button press states from DirectInput's button array.
	 *
	 * DirectInput reports buttons as a byte array where bit 7 indicates pressed.
	 * We pack these into a single int32 bitmask for easier handling.
	 *
	 * @param Buttons - DirectInput button array (from DIJOYSTATE2)
	 * @param Count - Number of buttons to check
	 * @return Bitmask where bit N = 1 if button N is pressed
	 */
	int32 ParseButtonStates(const uint8* Buttons, int32 Count) const;

	/**
	 * Convert DirectInput POV (D-pad) value to direction enum.
	 *
	 * DirectInput reports D-pad as centidegrees (0-35999) or -1 for centered.
	 * - 0 = Up
	 * - 9000 = Right
	 * - 18000 = Down
	 * - 27000 = Left
	 * - -1 = Centered (no direction pressed)
	 *
	 * @param POVValue - Raw DirectInput POV value
	 * @return Direction as our game enum value
	 */
	int32 ParseDPad(int32 POVValue) const;
};

#endif // PLATFORM_WINDOWS

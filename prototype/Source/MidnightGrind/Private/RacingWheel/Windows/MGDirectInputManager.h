// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS

#include "RacingWheel/MGRacingWheelTypes.h"

// Forward declarations for DirectInput types
struct IDirectInput8W;
struct IDirectInputDevice8W;
struct IDirectInputEffect;
typedef struct IDirectInput8W* LPDIRECTINPUT8W;
typedef struct IDirectInputDevice8W* LPDIRECTINPUTDEVICE8W;
typedef struct IDirectInputEffect* LPDIRECTINPUTEFFECT;

/**
 * DirectInput Manager for Windows
 *
 * Handles low-level DirectInput8 operations for racing wheels:
 * - Device enumeration and acquisition
 * - Input state polling
 * - Force feedback effect creation and management
 *
 * This class is Windows-only and requires linking with dinput8.lib and dxguid.lib
 */
class FMGDirectInputManager
{
public:
	FMGDirectInputManager();
	~FMGDirectInputManager();

	/**
	 * Initialize DirectInput
	 * @return True if initialization succeeded
	 */
	bool Initialize();

	/**
	 * Shutdown and release all resources
	 */
	void Shutdown();

	/**
	 * Enumerate connected DirectInput devices (wheels/joysticks)
	 * @return Number of devices found
	 */
	int32 EnumerateDevices();

	/**
	 * Get device information
	 * @param DeviceIndex Index of device
	 * @param OutVendorID USB Vendor ID
	 * @param OutProductID USB Product ID
	 * @param OutDeviceName Device name string
	 * @return True if device exists at index
	 */
	bool GetDeviceInfo(int32 DeviceIndex, int32& OutVendorID, int32& OutProductID, FString& OutDeviceName) const;

	/**
	 * Acquire a device for exclusive use
	 * @param DeviceIndex Index of device to acquire
	 * @return True if acquisition succeeded
	 */
	bool AcquireDevice(int32 DeviceIndex);

	/**
	 * Release a previously acquired device
	 * @param DeviceIndex Index of device to release
	 */
	void ReleaseDevice(int32 DeviceIndex);

	/**
	 * Poll the device for new input state
	 * @param DeviceIndex Index of device to poll
	 * @return True if poll succeeded (false may indicate device lost)
	 */
	bool PollDevice(int32 DeviceIndex);

	/**
	 * Get the current input state from a device
	 * @param DeviceIndex Index of device
	 * @param OutState Output state structure
	 */
	void GetInputState(int32 DeviceIndex, FMGWheelState& OutState) const;

	/**
	 * Check if a device supports force feedback
	 * @param DeviceIndex Index of device
	 * @return True if FFB is supported
	 */
	bool SupportsFFB(int32 DeviceIndex) const;

	/**
	 * Get supported FFB effects for a device
	 * @param DeviceIndex Index of device
	 * @return Array of supported effect types
	 */
	TArray<EMGFFBEffectType> GetSupportedEffects(int32 DeviceIndex) const;

	/**
	 * Create and start a force feedback effect
	 * @param DeviceIndex Index of device
	 * @param Effect Effect parameters
	 * @param GlobalGain Master gain (0-1)
	 * @return Effect ID for later control
	 */
	FGuid CreateEffect(int32 DeviceIndex, const FMGFFBEffect& Effect, float GlobalGain);

	/**
	 * Update an existing effect's parameters
	 * @param DeviceIndex Index of device
	 * @param EffectID ID of effect to update
	 * @param Effect New effect parameters
	 * @param GlobalGain Master gain (0-1)
	 */
	void UpdateEffect(int32 DeviceIndex, FGuid EffectID, const FMGFFBEffect& Effect, float GlobalGain);

	/**
	 * Start a previously created effect
	 * @param DeviceIndex Index of device
	 * @param EffectID ID of effect to start
	 */
	void StartEffect(int32 DeviceIndex, FGuid EffectID);

	/**
	 * Stop a playing effect
	 * @param DeviceIndex Index of device
	 * @param EffectID ID of effect to stop
	 */
	void StopEffect(int32 DeviceIndex, FGuid EffectID);

	/**
	 * Stop all effects on a device
	 * @param DeviceIndex Index of device
	 */
	void StopAllEffects(int32 DeviceIndex);

	/**
	 * Set the global FFB gain for a device
	 * @param DeviceIndex Index of device
	 * @param Gain Gain value (0-10000)
	 */
	void SetGain(int32 DeviceIndex, int32 Gain);

	/**
	 * Enable or disable auto-center spring
	 * @param DeviceIndex Index of device
	 * @param bEnabled True to enable auto-center
	 */
	void SetAutoCenter(int32 DeviceIndex, bool bEnabled);

private:
	/** Device information structure */
	struct FDeviceInfo
	{
		LPDIRECTINPUTDEVICE8W Device = nullptr;
		FString DeviceName;
		int32 VendorID = 0;
		int32 ProductID = 0;
		bool bIsAcquired = false;
		bool bSupportsFFB = false;
		TArray<EMGFFBEffectType> SupportedEffects;

		/** Current input state */
		FMGWheelState CurrentState;

		/** Active effects */
		TMap<FGuid, LPDIRECTINPUTEFFECT> ActiveEffects;
	};

	/** DirectInput interface */
	LPDIRECTINPUT8W DirectInput = nullptr;

	/** Enumerated devices */
	TArray<FDeviceInfo> Devices;

	/** Device GUID storage for enumeration */
	TArray<GUID> EnumeratedGUIDs;

	/** Static callback for device enumeration */
	static int32 __stdcall EnumDevicesCallback(const void* DeviceInstance, void* Context);

	/** Static callback for effects enumeration */
	static int32 __stdcall EnumEffectsCallback(const void* EffectInfo, void* Context);

	/** Create a DirectInput effect from our effect structure */
	LPDIRECTINPUTEFFECT CreateDIEffect(LPDIRECTINPUTDEVICE8W Device, const FMGFFBEffect& Effect, float GlobalGain);

	/** Convert our effect type to DirectInput GUID */
	bool GetEffectGUID(EMGFFBEffectType EffectType, GUID& OutGUID) const;

	/** Normalize axis value from DirectInput range to -1..1 or 0..1 */
	float NormalizeAxis(int32 Value, bool bCentered = true) const;

	/** Scale magnitude for DirectInput (0-10000 range) */
	int32 ScaleMagnitude(float Magnitude, float GlobalGain) const;

	/** Parse button states from DIJOYSTATE2 */
	int32 ParseButtonStates(const uint8* Buttons, int32 Count) const;

	/** Parse D-pad direction from POV value */
	int32 ParseDPad(int32 POVValue) const;
};

#endif // PLATFORM_WINDOWS

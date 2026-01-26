// Copyright Midnight Grind. All Rights Reserved.

#include "RacingWheel/Windows/MGDirectInputManager.h"

#if PLATFORM_WINDOWS

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <initguid.h>
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"

// Link with DirectInput libraries
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

FMGDirectInputManager::FMGDirectInputManager()
{
}

FMGDirectInputManager::~FMGDirectInputManager()
{
	Shutdown();
}

bool FMGDirectInputManager::Initialize()
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	HRESULT hr = DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8W,
		reinterpret_cast<void**>(&DirectInput),
		nullptr
	);

	if (FAILED(hr))
	{
		UE_LOG(LogTemp, Error, TEXT("DirectInput8Create failed with error 0x%08X"), hr);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("DirectInput initialized successfully"));
	return true;
}

void FMGDirectInputManager::Shutdown()
{
	// Release all devices
	for (FDeviceInfo& DeviceInfo : Devices)
	{
		// Stop and release all effects
		for (auto& EffectPair : DeviceInfo.ActiveEffects)
		{
			if (EffectPair.Value)
			{
				EffectPair.Value->Stop();
				EffectPair.Value->Release();
			}
		}
		DeviceInfo.ActiveEffects.Empty();

		// Release device
		if (DeviceInfo.Device)
		{
			if (DeviceInfo.bIsAcquired)
			{
				DeviceInfo.Device->Unacquire();
			}
			DeviceInfo.Device->Release();
			DeviceInfo.Device = nullptr;
		}
	}
	Devices.Empty();

	// Release DirectInput
	if (DirectInput)
	{
		DirectInput->Release();
		DirectInput = nullptr;
	}
}

int32 FMGDirectInputManager::EnumerateDevices()
{
	if (!DirectInput)
	{
		return 0;
	}

	// Clear previous enumeration
	EnumeratedGUIDs.Empty();
	Devices.Empty();

	// Enumerate game controllers (wheels are typically DI8DEVCLASS_GAMECTRL)
	HRESULT hr = DirectInput->EnumDevices(
		DI8DEVCLASS_GAMECTRL,
		reinterpret_cast<LPDIENUMDEVICESCALLBACKW>(EnumDevicesCallback),
		this,
		DIEDFL_ATTACHEDONLY
	);

	if (FAILED(hr))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnumDevices failed with error 0x%08X"), hr);
		return 0;
	}

	// Create device interfaces for each enumerated device
	for (const GUID& DeviceGUID : EnumeratedGUIDs)
	{
		LPDIRECTINPUTDEVICE8W Device = nullptr;
		hr = DirectInput->CreateDevice(DeviceGUID, &Device, nullptr);

		if (SUCCEEDED(hr) && Device)
		{
			// Get device capabilities
			DIDEVCAPS caps;
			caps.dwSize = sizeof(DIDEVCAPS);
			Device->GetCapabilities(&caps);

			// Get device instance info
			DIDEVICEINSTANCEW instance;
			instance.dwSize = sizeof(DIDEVICEINSTANCEW);
			Device->GetDeviceInfo(&instance);

			FDeviceInfo DeviceInfo;
			DeviceInfo.Device = Device;
			DeviceInfo.DeviceName = FString(instance.tszProductName);
			DeviceInfo.VendorID = LOWORD(instance.guidProduct.Data1);
			DeviceInfo.ProductID = HIWORD(instance.guidProduct.Data1);
			DeviceInfo.bSupportsFFB = (caps.dwFlags & DIDC_FORCEFEEDBACK) != 0;

			UE_LOG(LogTemp, Log, TEXT("Found device: %s (VID: 0x%04X, PID: 0x%04X, FFB: %s)"),
				*DeviceInfo.DeviceName,
				DeviceInfo.VendorID,
				DeviceInfo.ProductID,
				DeviceInfo.bSupportsFFB ? TEXT("Yes") : TEXT("No"));

			Devices.Add(DeviceInfo);
		}
	}

	return Devices.Num();
}

int32 __stdcall FMGDirectInputManager::EnumDevicesCallback(const void* DeviceInstance, void* Context)
{
	const DIDEVICEINSTANCEW* Instance = static_cast<const DIDEVICEINSTANCEW*>(DeviceInstance);
	FMGDirectInputManager* Manager = static_cast<FMGDirectInputManager*>(Context);

	if (Instance && Manager)
	{
		// Check if this is likely a racing wheel (has steering axis and is a driving control)
		// DI8DEVTYPE_DRIVING subtypes: 0x16 = wheel, etc.
		BYTE DeviceType = GET_DIDEVICE_TYPE(Instance->dwDevType);
		BYTE DeviceSubtype = GET_DIDEVICE_SUBTYPE(Instance->dwDevType);

		// Accept driving controllers, joysticks with many axes, or first-person controllers
		if (DeviceType == DI8DEVTYPE_DRIVING ||
			DeviceType == DI8DEVTYPE_JOYSTICK ||
			DeviceType == DI8DEVTYPE_1STPERSON ||
			DeviceType == DI8DEVTYPE_GAMEPAD)
		{
			Manager->EnumeratedGUIDs.Add(Instance->guidInstance);
		}
	}

	return DIENUM_CONTINUE;
}

bool FMGDirectInputManager::GetDeviceInfo(int32 DeviceIndex, int32& OutVendorID, int32& OutProductID, FString& OutDeviceName) const
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return false;
	}

	const FDeviceInfo& Info = Devices[DeviceIndex];
	OutVendorID = Info.VendorID;
	OutProductID = Info.ProductID;
	OutDeviceName = Info.DeviceName;
	return true;
}

bool FMGDirectInputManager::AcquireDevice(int32 DeviceIndex)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return false;
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];
	if (!DeviceInfo.Device)
	{
		return false;
	}

	// Set cooperative level (exclusive for FFB)
	HWND hWnd = GetForegroundWindow();
	if (!hWnd)
	{
		hWnd = GetDesktopWindow();
	}

	HRESULT hr = DeviceInfo.Device->SetCooperativeLevel(
		hWnd,
		DISCL_EXCLUSIVE | DISCL_FOREGROUND
	);

	if (FAILED(hr))
	{
		// Try non-exclusive if exclusive fails
		hr = DeviceInfo.Device->SetCooperativeLevel(
			hWnd,
			DISCL_NONEXCLUSIVE | DISCL_FOREGROUND
		);

		if (FAILED(hr))
		{
			UE_LOG(LogTemp, Warning, TEXT("SetCooperativeLevel failed: 0x%08X"), hr);
		}
	}

	// Set data format
	hr = DeviceInfo.Device->SetDataFormat(&c_dfDIJoystick2);
	if (FAILED(hr))
	{
		UE_LOG(LogTemp, Error, TEXT("SetDataFormat failed: 0x%08X"), hr);
		return false;
	}

	// Set axis mode to absolute
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.diph.dwObj = 0;
	dipdw.dwData = DIPROPAXISMODE_ABS;

	DeviceInfo.Device->SetProperty(DIPROP_AXISMODE, &dipdw.diph);

	// Set axis range (-32768 to 32767)
	DIPROPRANGE dipr;
	dipr.diph.dwSize = sizeof(DIPROPRANGE);
	dipr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipr.diph.dwHow = DIPH_DEVICE;
	dipr.diph.dwObj = 0;
	dipr.lMin = -32768;
	dipr.lMax = 32767;

	DeviceInfo.Device->SetProperty(DIPROP_RANGE, &dipr.diph);

	// Acquire device
	hr = DeviceInfo.Device->Acquire();
	if (FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
	{
		UE_LOG(LogTemp, Warning, TEXT("Device acquisition failed: 0x%08X"), hr);
		// Still mark as acquired - we'll retry on poll
	}

	DeviceInfo.bIsAcquired = true;

	// Enumerate supported FFB effects
	if (DeviceInfo.bSupportsFFB)
	{
		DeviceInfo.SupportedEffects.Empty();
		DeviceInfo.Device->EnumEffects(
			reinterpret_cast<LPDIENUMEFFECTSCALLBACKW>(EnumEffectsCallback),
			&DeviceInfo.SupportedEffects,
			DIEFT_ALL
		);

		// Disable auto-center if we have FFB
		SetAutoCenter(DeviceIndex, false);
	}

	UE_LOG(LogTemp, Log, TEXT("Acquired device: %s"), *DeviceInfo.DeviceName);
	return true;
}

int32 __stdcall FMGDirectInputManager::EnumEffectsCallback(const void* EffectInfo, void* Context)
{
	const DIEFFECTINFOW* Effect = static_cast<const DIEFFECTINFOW*>(EffectInfo);
	TArray<EMGFFBEffectType>* SupportedEffects = static_cast<TArray<EMGFFBEffectType>*>(Context);

	if (Effect && SupportedEffects)
	{
		// Map DirectInput effect types to our enum
		if (IsEqualGUID(Effect->guid, GUID_ConstantForce))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::ConstantForce);
		}
		else if (IsEqualGUID(Effect->guid, GUID_Spring))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::Spring);
		}
		else if (IsEqualGUID(Effect->guid, GUID_Damper))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::Damper);
		}
		else if (IsEqualGUID(Effect->guid, GUID_Friction))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::Friction);
		}
		else if (IsEqualGUID(Effect->guid, GUID_Inertia))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::Inertia);
		}
		else if (IsEqualGUID(Effect->guid, GUID_Sine))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::SineWave);
		}
		else if (IsEqualGUID(Effect->guid, GUID_Square))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::SquareWave);
		}
		else if (IsEqualGUID(Effect->guid, GUID_Triangle))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::TriangleWave);
		}
		else if (IsEqualGUID(Effect->guid, GUID_SawtoothUp))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::SawtoothUp);
		}
		else if (IsEqualGUID(Effect->guid, GUID_SawtoothDown))
		{
			SupportedEffects->AddUnique(EMGFFBEffectType::SawtoothDown);
		}
	}

	return DIENUM_CONTINUE;
}

void FMGDirectInputManager::ReleaseDevice(int32 DeviceIndex)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return;
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];

	// Stop and release all effects
	for (auto& EffectPair : DeviceInfo.ActiveEffects)
	{
		if (EffectPair.Value)
		{
			EffectPair.Value->Stop();
			EffectPair.Value->Release();
		}
	}
	DeviceInfo.ActiveEffects.Empty();

	// Unacquire device
	if (DeviceInfo.Device && DeviceInfo.bIsAcquired)
	{
		DeviceInfo.Device->Unacquire();
		DeviceInfo.bIsAcquired = false;
	}
}

bool FMGDirectInputManager::PollDevice(int32 DeviceIndex)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return false;
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];
	if (!DeviceInfo.Device || !DeviceInfo.bIsAcquired)
	{
		return false;
	}

	// Poll device
	HRESULT hr = DeviceInfo.Device->Poll();
	if (FAILED(hr))
	{
		// Try to reacquire
		hr = DeviceInfo.Device->Acquire();
		if (FAILED(hr))
		{
			return false;
		}
		hr = DeviceInfo.Device->Poll();
		if (FAILED(hr))
		{
			return false;
		}
	}

	// Get device state
	DIJOYSTATE2 js;
	hr = DeviceInfo.Device->GetDeviceState(sizeof(DIJOYSTATE2), &js);
	if (FAILED(hr))
	{
		return false;
	}

	// Update our state structure
	FMGWheelState& State = DeviceInfo.CurrentState;

	// Store raw values
	State.RawSteering = js.lX;
	State.RawThrottle = js.lY;    // Usually Y axis or separate
	State.RawBrake = js.lRz;       // Usually Rz or separate
	State.RawClutch = js.rglSlider[0]; // Usually slider

	// Normalize steering (-1 to 1)
	State.SteeringNormalized = NormalizeAxis(js.lX, true);

	// For separate pedals (common on racing wheels)
	// Many wheels report pedals inverted (full press = 0, released = max)
	// Adjust based on typical behavior
	State.ThrottlePedal = 1.0f - NormalizeAxis(js.lY, false);
	State.BrakePedal = 1.0f - NormalizeAxis(js.lRz, false);
	State.ClutchPedal = 1.0f - NormalizeAxis(js.rglSlider[0], false);

	// Clamp pedal values
	State.ThrottlePedal = FMath::Clamp(State.ThrottlePedal, 0.0f, 1.0f);
	State.BrakePedal = FMath::Clamp(State.BrakePedal, 0.0f, 1.0f);
	State.ClutchPedal = FMath::Clamp(State.ClutchPedal, 0.0f, 1.0f);

	// Calculate steering angle based on rotation
	float MaxRotation = 900.0f; // Default, should come from capabilities
	State.SteeringAngle = State.SteeringNormalized * (MaxRotation / 2.0f);

	// Parse buttons
	State.ButtonStates = ParseButtonStates(js.rgbButtons, 32);

	// Parse D-pad (POV)
	State.DPadDirection = ParseDPad(js.rgdwPOV[0]);

	// Paddle shifters are typically buttons 4 and 5 on Logitech wheels
	bool bLeftPaddle = (js.rgbButtons[4] & 0x80) != 0;
	bool bRightPaddle = (js.rgbButtons[5] & 0x80) != 0;
	State.bLeftPaddlePressed = bLeftPaddle;
	State.bRightPaddlePressed = bRightPaddle;

	return true;
}

void FMGDirectInputManager::GetInputState(int32 DeviceIndex, FMGWheelState& OutState) const
{
	if (Devices.IsValidIndex(DeviceIndex))
	{
		OutState = Devices[DeviceIndex].CurrentState;
	}
}

bool FMGDirectInputManager::SupportsFFB(int32 DeviceIndex) const
{
	if (Devices.IsValidIndex(DeviceIndex))
	{
		return Devices[DeviceIndex].bSupportsFFB;
	}
	return false;
}

TArray<EMGFFBEffectType> FMGDirectInputManager::GetSupportedEffects(int32 DeviceIndex) const
{
	if (Devices.IsValidIndex(DeviceIndex))
	{
		return Devices[DeviceIndex].SupportedEffects;
	}
	return TArray<EMGFFBEffectType>();
}

FGuid FMGDirectInputManager::CreateEffect(int32 DeviceIndex, const FMGFFBEffect& Effect, float GlobalGain)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return FGuid();
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];
	if (!DeviceInfo.Device || !DeviceInfo.bSupportsFFB)
	{
		return FGuid();
	}

	LPDIRECTINPUTEFFECT DIEffect = CreateDIEffect(DeviceInfo.Device, Effect, GlobalGain);
	if (!DIEffect)
	{
		return FGuid();
	}

	// Start the effect
	DIEffect->Start(Effect.Duration < 0 ? INFINITE : 1, 0);

	// Store and return the ID
	FGuid EffectID = Effect.EffectID;
	DeviceInfo.ActiveEffects.Add(EffectID, DIEffect);

	return EffectID;
}

LPDIRECTINPUTEFFECT FMGDirectInputManager::CreateDIEffect(LPDIRECTINPUTDEVICE8W Device, const FMGFFBEffect& Effect, float GlobalGain)
{
	GUID EffectGUID;
	if (!GetEffectGUID(Effect.EffectType, EffectGUID))
	{
		return nullptr;
	}

	// Prepare effect structure
	DIEFFECT diEffect = {};
	diEffect.dwSize = sizeof(DIEFFECT);
	diEffect.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	diEffect.dwDuration = Effect.Duration < 0 ? INFINITE : static_cast<DWORD>(Effect.Duration * 1000000); // microseconds
	diEffect.dwGain = ScaleMagnitude(GlobalGain, 1.0f);
	diEffect.dwTriggerButton = DIEB_NOTRIGGER;
	diEffect.dwTriggerRepeatInterval = 0;

	// Set axes (X axis for steering wheel)
	DWORD rgdwAxes[1] = { DIJOFS_X };
	LONG rglDirection[1] = { 0 };
	diEffect.cAxes = 1;
	diEffect.rgdwAxes = rgdwAxes;
	diEffect.rglDirection = rglDirection;

	// Set direction
	rglDirection[0] = static_cast<LONG>(Effect.DirectionDegrees * 100);

	// Type-specific parameters
	DICONSTANTFORCE constantForce = {};
	DIPERIODIC periodicForce = {};
	DICONDITION condition[2] = {};
	DIENVELOPE envelope = {};

	// Set up envelope if needed
	if (Effect.AttackTime > 0.0f || Effect.FadeTime > 0.0f)
	{
		envelope.dwSize = sizeof(DIENVELOPE);
		envelope.dwAttackLevel = ScaleMagnitude(Effect.AttackLevel, 1.0f);
		envelope.dwAttackTime = static_cast<DWORD>(Effect.AttackTime * 1000000);
		envelope.dwFadeLevel = ScaleMagnitude(Effect.FadeLevel, 1.0f);
		envelope.dwFadeTime = static_cast<DWORD>(Effect.FadeTime * 1000000);
		diEffect.lpEnvelope = &envelope;
	}

	switch (Effect.EffectType)
	{
	case EMGFFBEffectType::ConstantForce:
		constantForce.lMagnitude = ScaleMagnitude(Effect.Magnitude, GlobalGain);
		diEffect.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
		diEffect.lpvTypeSpecificParams = &constantForce;
		break;

	case EMGFFBEffectType::SineWave:
	case EMGFFBEffectType::SquareWave:
	case EMGFFBEffectType::TriangleWave:
	case EMGFFBEffectType::SawtoothUp:
	case EMGFFBEffectType::SawtoothDown:
		periodicForce.dwMagnitude = ScaleMagnitude(FMath::Abs(Effect.Magnitude), GlobalGain);
		periodicForce.lOffset = ScaleMagnitude(Effect.Offset, 1.0f);
		periodicForce.dwPhase = static_cast<DWORD>(Effect.Phase * 100); // Hundredths of degree
		periodicForce.dwPeriod = Effect.Frequency > 0.0f ? static_cast<DWORD>(1000000.0f / Effect.Frequency) : 1000000; // microseconds
		diEffect.cbTypeSpecificParams = sizeof(DIPERIODIC);
		diEffect.lpvTypeSpecificParams = &periodicForce;
		break;

	case EMGFFBEffectType::Spring:
	case EMGFFBEffectType::Damper:
	case EMGFFBEffectType::Friction:
	case EMGFFBEffectType::Inertia:
		condition[0].lOffset = ScaleMagnitude(Effect.CenterOffset, 1.0f);
		condition[0].lPositiveCoefficient = ScaleMagnitude(Effect.Coefficient * Effect.Magnitude, GlobalGain);
		condition[0].lNegativeCoefficient = ScaleMagnitude(Effect.Coefficient * Effect.Magnitude, GlobalGain);
		condition[0].dwPositiveSaturation = ScaleMagnitude(Effect.Saturation, 1.0f);
		condition[0].dwNegativeSaturation = ScaleMagnitude(Effect.Saturation, 1.0f);
		condition[0].lDeadBand = ScaleMagnitude(Effect.Deadband, 1.0f);
		diEffect.cbTypeSpecificParams = sizeof(DICONDITION);
		diEffect.lpvTypeSpecificParams = condition;
		break;

	default:
		return nullptr;
	}

	// Create the effect
	LPDIRECTINPUTEFFECT DIEffect = nullptr;
	HRESULT hr = Device->CreateEffect(EffectGUID, &diEffect, &DIEffect, nullptr);

	if (FAILED(hr))
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateEffect failed: 0x%08X"), hr);
		return nullptr;
	}

	return DIEffect;
}

bool FMGDirectInputManager::GetEffectGUID(EMGFFBEffectType EffectType, GUID& OutGUID) const
{
	switch (EffectType)
	{
	case EMGFFBEffectType::ConstantForce:
		OutGUID = GUID_ConstantForce;
		return true;
	case EMGFFBEffectType::Spring:
		OutGUID = GUID_Spring;
		return true;
	case EMGFFBEffectType::Damper:
		OutGUID = GUID_Damper;
		return true;
	case EMGFFBEffectType::Friction:
		OutGUID = GUID_Friction;
		return true;
	case EMGFFBEffectType::Inertia:
		OutGUID = GUID_Inertia;
		return true;
	case EMGFFBEffectType::SineWave:
		OutGUID = GUID_Sine;
		return true;
	case EMGFFBEffectType::SquareWave:
		OutGUID = GUID_Square;
		return true;
	case EMGFFBEffectType::TriangleWave:
		OutGUID = GUID_Triangle;
		return true;
	case EMGFFBEffectType::SawtoothUp:
		OutGUID = GUID_SawtoothUp;
		return true;
	case EMGFFBEffectType::SawtoothDown:
		OutGUID = GUID_SawtoothDown;
		return true;
	default:
		return false;
	}
}

void FMGDirectInputManager::UpdateEffect(int32 DeviceIndex, FGuid EffectID, const FMGFFBEffect& Effect, float GlobalGain)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return;
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];
	LPDIRECTINPUTEFFECT* DIEffectPtr = DeviceInfo.ActiveEffects.Find(EffectID);
	if (!DIEffectPtr || !*DIEffectPtr)
	{
		return;
	}

	// For now, recreate the effect - could be optimized to update in place
	(*DIEffectPtr)->Stop();
	(*DIEffectPtr)->Release();
	DeviceInfo.ActiveEffects.Remove(EffectID);

	CreateEffect(DeviceIndex, Effect, GlobalGain);
}

void FMGDirectInputManager::StartEffect(int32 DeviceIndex, FGuid EffectID)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return;
	}

	LPDIRECTINPUTEFFECT* DIEffectPtr = Devices[DeviceIndex].ActiveEffects.Find(EffectID);
	if (DIEffectPtr && *DIEffectPtr)
	{
		(*DIEffectPtr)->Start(1, 0);
	}
}

void FMGDirectInputManager::StopEffect(int32 DeviceIndex, FGuid EffectID)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return;
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];
	LPDIRECTINPUTEFFECT* DIEffectPtr = DeviceInfo.ActiveEffects.Find(EffectID);
	if (DIEffectPtr && *DIEffectPtr)
	{
		(*DIEffectPtr)->Stop();
		(*DIEffectPtr)->Release();
		DeviceInfo.ActiveEffects.Remove(EffectID);
	}
}

void FMGDirectInputManager::StopAllEffects(int32 DeviceIndex)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return;
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];
	for (auto& EffectPair : DeviceInfo.ActiveEffects)
	{
		if (EffectPair.Value)
		{
			EffectPair.Value->Stop();
			EffectPair.Value->Release();
		}
	}
	DeviceInfo.ActiveEffects.Empty();
}

void FMGDirectInputManager::SetGain(int32 DeviceIndex, int32 Gain)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return;
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];
	if (!DeviceInfo.Device)
	{
		return;
	}

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.diph.dwObj = 0;
	dipdw.dwData = FMath::Clamp(Gain, 0, 10000);

	DeviceInfo.Device->SetProperty(DIPROP_FFGAIN, &dipdw.diph);
}

void FMGDirectInputManager::SetAutoCenter(int32 DeviceIndex, bool bEnabled)
{
	if (!Devices.IsValidIndex(DeviceIndex))
	{
		return;
	}

	FDeviceInfo& DeviceInfo = Devices[DeviceIndex];
	if (!DeviceInfo.Device)
	{
		return;
	}

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.diph.dwObj = 0;
	dipdw.dwData = bEnabled ? DIPROPAUTOCENTER_ON : DIPROPAUTOCENTER_OFF;

	DeviceInfo.Device->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph);
}

float FMGDirectInputManager::NormalizeAxis(int32 Value, bool bCentered) const
{
	if (bCentered)
	{
		// -32768 to 32767 -> -1 to 1
		return static_cast<float>(Value) / 32767.0f;
	}
	else
	{
		// -32768 to 32767 -> 0 to 1 (for pedals)
		return (static_cast<float>(Value) + 32768.0f) / 65535.0f;
	}
}

int32 FMGDirectInputManager::ScaleMagnitude(float Magnitude, float GlobalGain) const
{
	return static_cast<int32>(FMath::Clamp(Magnitude * GlobalGain, -1.0f, 1.0f) * 10000.0f);
}

int32 FMGDirectInputManager::ParseButtonStates(const uint8* Buttons, int32 Count) const
{
	int32 ButtonMask = 0;
	for (int32 i = 0; i < FMath::Min(Count, 32); i++)
	{
		if (Buttons[i] & 0x80)
		{
			ButtonMask |= (1 << i);
		}
	}
	return ButtonMask;
}

int32 FMGDirectInputManager::ParseDPad(int32 POVValue) const
{
	if (LOWORD(POVValue) == 0xFFFF)
	{
		return -1; // Centered
	}

	// POV is in hundredths of degrees (0 = up, 9000 = right, etc.)
	int32 Direction = (POVValue / 4500) % 8;
	return Direction;
}

#endif // PLATFORM_WINDOWS

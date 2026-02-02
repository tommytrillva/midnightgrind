// Copyright Midnight Grind. All Rights Reserved.

#include "RacingWheel/MGRacingWheelSubsystem.h"
#include "RacingWheel/MGWheelFFBProcessor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#if PLATFORM_WINDOWS
#include "RacingWheel/Windows/MGDirectInputManager.h"
#endif

DEFINE_LOG_CATEGORY(LogRacingWheel);

void UMGRacingWheelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize known wheel database
	InitializeKnownWheelDatabase();

	// Load profiles from disk
	LoadProfilesFromDisk();

	// Create FFB processor
	FFBProcessor = NewObject<UMGWheelFFBProcessor>(this);
	if (FFBProcessor)
	{
		FFBProcessor->Initialize(this);
	}

	// Initialize DirectInput on Windows
#if PLATFORM_WINDOWS
	InitializeDirectInput();
#endif

	// Start tick timer for input polling (120Hz for low latency)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WheelTickHandle,
			this,
			&UMGRacingWheelSubsystem::OnWheelTick,
			0.00833f, // ~120 Hz
			true
		);
	}

	// Initial scan for wheels
	ScanForWheels();

	UE_LOG(LogRacingWheel, Log, TEXT("MGRacingWheelSubsystem initialized"));
}

void UMGRacingWheelSubsystem::Deinitialize()
{
	// Stop tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WheelTickHandle);
	}

	// Stop all FFB
	StopAllFFBEffects();

	// Shutdown DirectInput
#if PLATFORM_WINDOWS
	ShutdownDirectInput();
#endif

	// Save profiles
	SaveProfilesToDisk();

	Super::Deinitialize();
}

bool UMGRacingWheelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGRacingWheelSubsystem::InitializeKnownWheelDatabase()
{
	// Logitech wheels
	// G920 (Xbox/PC)
	{
		FMGKnownWheelEntry Entry(0x046D, 0xC262, EMGWheelManufacturer::Logitech, EMGWheelModel::Logitech_G920);
		Entry.DefaultCapabilities.DeviceName = TEXT("Logitech G920 Driving Force");
		Entry.DefaultCapabilities.MaxRotationDegrees = 900.0f;
		Entry.DefaultCapabilities.PedalCount = 3;
		Entry.DefaultCapabilities.bHasClutch = true;
		Entry.DefaultCapabilities.bHasPaddleShifters = true;
		Entry.DefaultCapabilities.ButtonCount = 11;
		Entry.DefaultCapabilities.MaxFFBForceNm = 2.5f;
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Spring);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Damper);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Friction);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SineWave);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SquareWave);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::TriangleWave);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SawtoothUp);
		KnownWheelDatabase.Add(Entry);
	}

	// G29 (PlayStation/PC)
	{
		FMGKnownWheelEntry Entry(0x046D, 0xC24F, EMGWheelManufacturer::Logitech, EMGWheelModel::Logitech_G29);
		Entry.DefaultCapabilities.DeviceName = TEXT("Logitech G29 Driving Force");
		Entry.DefaultCapabilities.MaxRotationDegrees = 900.0f;
		Entry.DefaultCapabilities.PedalCount = 3;
		Entry.DefaultCapabilities.bHasClutch = true;
		Entry.DefaultCapabilities.bHasPaddleShifters = true;
		Entry.DefaultCapabilities.ButtonCount = 14;
		Entry.DefaultCapabilities.MaxFFBForceNm = 2.5f;
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Spring);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Damper);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SineWave);
		KnownWheelDatabase.Add(Entry);
	}

	// G923 (TrueForce)
	{
		FMGKnownWheelEntry Entry(0x046D, 0xC266, EMGWheelManufacturer::Logitech, EMGWheelModel::Logitech_G923);
		Entry.DefaultCapabilities.DeviceName = TEXT("Logitech G923 Racing Wheel");
		Entry.DefaultCapabilities.MaxRotationDegrees = 900.0f;
		Entry.DefaultCapabilities.PedalCount = 3;
		Entry.DefaultCapabilities.bHasClutch = true;
		Entry.DefaultCapabilities.bHasPaddleShifters = true;
		Entry.DefaultCapabilities.ButtonCount = 11;
		Entry.DefaultCapabilities.MaxFFBForceNm = 3.0f;
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Spring);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Damper);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Friction);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SineWave);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SquareWave);
		KnownWheelDatabase.Add(Entry);
	}

	// G27
	{
		FMGKnownWheelEntry Entry(0x046D, 0xC29B, EMGWheelManufacturer::Logitech, EMGWheelModel::Logitech_G27);
		Entry.DefaultCapabilities.DeviceName = TEXT("Logitech G27 Racing Wheel");
		Entry.DefaultCapabilities.MaxRotationDegrees = 900.0f;
		Entry.DefaultCapabilities.PedalCount = 3;
		Entry.DefaultCapabilities.bHasClutch = true;
		Entry.DefaultCapabilities.bHasPaddleShifters = true;
		Entry.DefaultCapabilities.bHasHPatternShifter = true;
		Entry.DefaultCapabilities.ButtonCount = 11;
		Entry.DefaultCapabilities.MaxFFBForceNm = 2.3f;
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Spring);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Damper);
		KnownWheelDatabase.Add(Entry);
	}

	// Thrustmaster T300RS
	{
		FMGKnownWheelEntry Entry(0x044F, 0xB66E, EMGWheelManufacturer::Thrustmaster, EMGWheelModel::Thrustmaster_T300RS);
		Entry.DefaultCapabilities.DeviceName = TEXT("Thrustmaster T300RS");
		Entry.DefaultCapabilities.MaxRotationDegrees = 1080.0f;
		Entry.DefaultCapabilities.PedalCount = 2;
		Entry.DefaultCapabilities.bHasClutch = false;
		Entry.DefaultCapabilities.bHasPaddleShifters = true;
		Entry.DefaultCapabilities.ButtonCount = 12;
		Entry.DefaultCapabilities.MaxFFBForceNm = 3.9f;
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Spring);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Damper);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Friction);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SineWave);
		KnownWheelDatabase.Add(Entry);
	}

	// Thrustmaster TX
	{
		FMGKnownWheelEntry Entry(0x044F, 0xB669, EMGWheelManufacturer::Thrustmaster, EMGWheelModel::Thrustmaster_TX);
		Entry.DefaultCapabilities.DeviceName = TEXT("Thrustmaster TX Racing Wheel");
		Entry.DefaultCapabilities.MaxRotationDegrees = 900.0f;
		Entry.DefaultCapabilities.PedalCount = 2;
		Entry.DefaultCapabilities.bHasPaddleShifters = true;
		Entry.DefaultCapabilities.ButtonCount = 12;
		Entry.DefaultCapabilities.MaxFFBForceNm = 3.5f;
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Spring);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Damper);
		KnownWheelDatabase.Add(Entry);
	}

	// Thrustmaster TMX
	{
		FMGKnownWheelEntry Entry(0x044F, 0xB67F, EMGWheelManufacturer::Thrustmaster, EMGWheelModel::Thrustmaster_TMX);
		Entry.DefaultCapabilities.DeviceName = TEXT("Thrustmaster TMX Force Feedback");
		Entry.DefaultCapabilities.MaxRotationDegrees = 900.0f;
		Entry.DefaultCapabilities.PedalCount = 2;
		Entry.DefaultCapabilities.bHasPaddleShifters = true;
		Entry.DefaultCapabilities.ButtonCount = 10;
		Entry.DefaultCapabilities.MaxFFBForceNm = 2.0f;
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Spring);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Damper);
		KnownWheelDatabase.Add(Entry);
	}

	// Fanatec CSL DD
	{
		FMGKnownWheelEntry Entry(0x0EB7, 0x0020, EMGWheelManufacturer::Fanatec, EMGWheelModel::Fanatec_CSL_DD);
		Entry.DefaultCapabilities.DeviceName = TEXT("Fanatec CSL DD");
		Entry.DefaultCapabilities.MaxRotationDegrees = 1080.0f;
		Entry.DefaultCapabilities.PedalCount = 2;
		Entry.DefaultCapabilities.bHasPaddleShifters = true;
		Entry.DefaultCapabilities.ButtonCount = 12;
		Entry.DefaultCapabilities.MaxFFBForceNm = 8.0f;
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Spring);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Damper);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::Friction);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SineWave);
		Entry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::SquareWave);
		KnownWheelDatabase.Add(Entry);
	}

	UE_LOG(LogRacingWheel, Log, TEXT("Initialized known wheel database with %d entries"), KnownWheelDatabase.Num());
}

FMGKnownWheelEntry UMGRacingWheelSubsystem::GetKnownWheelInfo(int32 VendorID, int32 ProductID) const
{
	for (const FMGKnownWheelEntry& Entry : KnownWheelDatabase)
	{
		if (Entry.VendorID == VendorID && Entry.ProductID == ProductID)
		{
			return Entry;
		}
	}

	// Return generic entry
	FMGKnownWheelEntry GenericEntry;
	GenericEntry.Manufacturer = EMGWheelManufacturer::Generic;
	GenericEntry.Model = EMGWheelModel::Generic_DirectInput;
	GenericEntry.DefaultCapabilities.DeviceName = TEXT("Generic Racing Wheel");
	GenericEntry.DefaultCapabilities.MaxRotationDegrees = 900.0f;
	GenericEntry.DefaultCapabilities.SupportedEffects.Add(EMGFFBEffectType::ConstantForce);
	return GenericEntry;
}

#if PLATFORM_WINDOWS
void UMGRacingWheelSubsystem::InitializeDirectInput()
{
	DirectInputManager = MakeShared<FMGDirectInputManager>();
	if (!DirectInputManager->Initialize())
	{
		UE_LOG(LogRacingWheel, Error, TEXT("Failed to initialize DirectInput"));
		DirectInputManager.Reset();
	}
}

void UMGRacingWheelSubsystem::ShutdownDirectInput()
{
	if (DirectInputManager.IsValid())
	{
		DirectInputManager->Shutdown();
		DirectInputManager.Reset();
	}
}
#else
void UMGRacingWheelSubsystem::InitializeDirectInput() {}
void UMGRacingWheelSubsystem::ShutdownDirectInput() {}
#endif

int32 UMGRacingWheelSubsystem::ScanForWheels()
{
#if PLATFORM_WINDOWS
	if (!DirectInputManager.IsValid())
	{
		return 0;
	}

	int32 WheelsFound = DirectInputManager->EnumerateDevices();

	if (WheelsFound > 0 && ConnectionState != EMGWheelConnectionState::Connected)
	{
		// Get device info
		int32 VID, PID;
		FString DeviceName;
		if (DirectInputManager->GetDeviceInfo(0, VID, PID, DeviceName))
		{
			FMGKnownWheelEntry WheelInfo = GetKnownWheelInfo(VID, PID);
			ConnectedWheelModel = WheelInfo.Model;
			ConnectedWheelCapabilities = WheelInfo.DefaultCapabilities;
			ConnectedWheelCapabilities.VendorID = VID;
			ConnectedWheelCapabilities.ProductID = PID;
			ConnectedWheelCapabilities.Manufacturer = WheelInfo.Manufacturer;
			ConnectedWheelCapabilities.Model = WheelInfo.Model;

			if (DirectInputManager->AcquireDevice(0))
			{
				ConnectionState = EMGWheelConnectionState::Connected;
				LoadDefaultProfileForWheel();

				UE_LOG(LogRacingWheel, Log, TEXT("Connected to wheel: %s (VID: 0x%04X, PID: 0x%04X)"),
					*ConnectedWheelCapabilities.DeviceName, VID, PID);

				OnWheelConnected.Broadcast(ConnectedWheelModel, ConnectedWheelCapabilities);
			}
		}
	}

	return WheelsFound;
#else
	return 0;
#endif
}

bool UMGRacingWheelSubsystem::IsWheelConnected() const
{
	return ConnectionState == EMGWheelConnectionState::Connected;
}

EMGWheelModel UMGRacingWheelSubsystem::GetConnectedWheelModel() const
{
	return ConnectedWheelModel;
}

FMGWheelCapabilities UMGRacingWheelSubsystem::GetWheelCapabilities() const
{
	return ConnectedWheelCapabilities;
}

FMGWheelState UMGRacingWheelSubsystem::GetWheelState() const
{
	return CurrentWheelState;
}

EMGWheelConnectionState UMGRacingWheelSubsystem::GetConnectionState() const
{
	return ConnectionState;
}

void UMGRacingWheelSubsystem::DisconnectWheel()
{
#if PLATFORM_WINDOWS
	if (DirectInputManager.IsValid())
	{
		DirectInputManager->ReleaseDevice(0);
	}
#endif

	EMGWheelModel OldModel = ConnectedWheelModel;
	ConnectionState = EMGWheelConnectionState::Disconnected;
	ConnectedWheelModel = EMGWheelModel::Unknown;
	ConnectedWheelCapabilities = FMGWheelCapabilities();
	CurrentWheelState = FMGWheelState();

	OnWheelDisconnected.Broadcast(OldModel);
}

void UMGRacingWheelSubsystem::OnWheelTick()
{
	// Check connection state
	CheckConnectionState();

	if (ConnectionState != EMGWheelConnectionState::Connected)
	{
		return;
	}

#if PLATFORM_WINDOWS
	if (DirectInputManager.IsValid())
	{
		// Poll device
		if (!DirectInputManager->PollDevice(0))
		{
			// Device lost
			DisconnectWheel();
			return;
		}

		// Get input state
		DirectInputManager->GetInputState(0, CurrentWheelState);

		// Process input
		ProcessInput();

		// Update FFB processor
		if (FFBProcessor && bFFBEnabled)
		{
			FFBProcessor->Tick(0.00833f);
		}

		// Broadcast state update
		OnWheelStateUpdated.Broadcast(CurrentWheelState);
	}
#endif
}

void UMGRacingWheelSubsystem::ProcessInput()
{
	// Apply profile settings to raw input

	// Steering
	float RawSteering = CurrentWheelState.SteeringNormalized;
	if (CurrentProfile.bInvertSteering)
	{
		RawSteering = -RawSteering;
	}

	// Apply deadzone
	float AbsSteering = FMath::Abs(RawSteering);
	if (AbsSteering < CurrentProfile.SteeringDeadzone)
	{
		ProcessedSteering = 0.0f;
	}
	else
	{
		float Sign = FMath::Sign(RawSteering);
		float Remapped = (AbsSteering - CurrentProfile.SteeringDeadzone) / (1.0f - CurrentProfile.SteeringDeadzone);
		float Curved = FMath::Pow(Remapped, CurrentProfile.SteeringLinearity);
		ProcessedSteering = FMath::Clamp(Curved * Sign, -1.0f, 1.0f);
	}

	// Throttle
	float RawThrottle = CurrentWheelState.ThrottlePedal;
	if (RawThrottle < CurrentProfile.ThrottleDeadzone)
	{
		ProcessedThrottle = 0.0f;
	}
	else
	{
		float Remapped = (RawThrottle - CurrentProfile.ThrottleDeadzone) / (1.0f - CurrentProfile.ThrottleDeadzone);
		ProcessedThrottle = FMath::Clamp(FMath::Pow(Remapped, CurrentProfile.ThrottleGamma), 0.0f, 1.0f);
	}

	// Brake
	float RawBrake = CurrentWheelState.BrakePedal;
	if (RawBrake < CurrentProfile.BrakeDeadzone)
	{
		ProcessedBrake = 0.0f;
	}
	else
	{
		float Remapped = (RawBrake - CurrentProfile.BrakeDeadzone) / (1.0f - CurrentProfile.BrakeDeadzone);
		ProcessedBrake = FMath::Clamp(FMath::Pow(Remapped, CurrentProfile.BrakeGamma), 0.0f, 1.0f);
	}

	// Clutch
	float RawClutch = CurrentWheelState.ClutchPedal;
	if (CurrentProfile.bInvertClutch)
	{
		RawClutch = 1.0f - RawClutch;
	}
	if (RawClutch < CurrentProfile.ClutchDeadzone)
	{
		ProcessedClutch = 0.0f;
	}
	else
	{
		float Remapped = (RawClutch - CurrentProfile.ClutchDeadzone) / (1.0f - CurrentProfile.ClutchDeadzone);
		ProcessedClutch = FMath::Clamp(Remapped, 0.0f, 1.0f);
	}

	// Paddle shifter edge detection
	bShiftDownThisFrame = CurrentWheelState.bLeftPaddlePressed && !bPrevLeftPaddle;
	bShiftUpThisFrame = CurrentWheelState.bRightPaddlePressed && !bPrevRightPaddle;
	bPrevLeftPaddle = CurrentWheelState.bLeftPaddlePressed;
	bPrevRightPaddle = CurrentWheelState.bRightPaddlePressed;
}

void UMGRacingWheelSubsystem::CheckConnectionState()
{
	// Periodically rescan for wheels if disconnected
	static float TimeSinceLastScan = 0.0f;
	TimeSinceLastScan += 0.00833f;

	if (ConnectionState == EMGWheelConnectionState::Disconnected && TimeSinceLastScan > 2.0f)
	{
		TimeSinceLastScan = 0.0f;
		ScanForWheels();
	}
}

float UMGRacingWheelSubsystem::GetSteeringInput() const
{
	return ProcessedSteering;
}

float UMGRacingWheelSubsystem::GetThrottleInput() const
{
	return ProcessedThrottle;
}

float UMGRacingWheelSubsystem::GetBrakeInput() const
{
	return ProcessedBrake;
}

float UMGRacingWheelSubsystem::GetClutchInput() const
{
	return ProcessedClutch;
}

bool UMGRacingWheelSubsystem::IsButtonPressed(int32 ButtonIndex) const
{
	if (ButtonIndex < 0 || ButtonIndex >= 32)
	{
		return false;
	}
	return (CurrentWheelState.ButtonStates & (1 << ButtonIndex)) != 0;
}

bool UMGRacingWheelSubsystem::WasShiftDownPressed() const
{
	return bShiftDownThisFrame;
}

bool UMGRacingWheelSubsystem::WasShiftUpPressed() const
{
	return bShiftUpThisFrame;
}

// FFB Methods
FGuid UMGRacingWheelSubsystem::PlayFFBEffect(const FMGFFBEffect& Effect)
{
	if (!bFFBEnabled || ConnectionState != EMGWheelConnectionState::Connected)
	{
		return FGuid();
	}

#if PLATFORM_WINDOWS
	if (DirectInputManager.IsValid())
	{
		return DirectInputManager->CreateEffect(0, Effect, GlobalFFBGain * CurrentProfile.FFBStrength);
	}
#endif
	return FGuid();
}

void UMGRacingWheelSubsystem::UpdateFFBEffect(FGuid EffectID, const FMGFFBEffect& Effect)
{
#if PLATFORM_WINDOWS
	if (DirectInputManager.IsValid() && EffectID.IsValid())
	{
		DirectInputManager->UpdateEffect(0, EffectID, Effect, GlobalFFBGain * CurrentProfile.FFBStrength);
	}
#endif
}

void UMGRacingWheelSubsystem::StopFFBEffect(FGuid EffectID)
{
#if PLATFORM_WINDOWS
	if (DirectInputManager.IsValid() && EffectID.IsValid())
	{
		DirectInputManager->StopEffect(0, EffectID);
	}
#endif
}

void UMGRacingWheelSubsystem::StopAllFFBEffects()
{
#if PLATFORM_WINDOWS
	if (DirectInputManager.IsValid())
	{
		DirectInputManager->StopAllEffects(0);
	}
#endif

	SelfCenteringEffectID.Invalidate();
	DamperEffectID.Invalidate();
	SurfaceEffectID.Invalidate();
	EngineEffectID.Invalidate();
}

void UMGRacingWheelSubsystem::SetFFBEffectPaused(FGuid EffectID, bool bPaused)
{
#if PLATFORM_WINDOWS
	if (DirectInputManager.IsValid() && EffectID.IsValid())
	{
		if (bPaused)
		{
			DirectInputManager->StopEffect(0, EffectID);
		}
		else
		{
			DirectInputManager->StartEffect(0, EffectID);
		}
	}
#endif
}

void UMGRacingWheelSubsystem::SetFFBGlobalGain(float Gain)
{
	GlobalFFBGain = FMath::Clamp(Gain, 0.0f, 1.0f);
}

void UMGRacingWheelSubsystem::SetFFBEnabled(bool bEnabled)
{
	bFFBEnabled = bEnabled;
	if (!bEnabled)
	{
		StopAllFFBEffects();
	}
}

bool UMGRacingWheelSubsystem::IsFFBEnabled() const
{
	return bFFBEnabled && CurrentProfile.bFFBEnabled;
}

void UMGRacingWheelSubsystem::UpdateFFBFromVehicle(const FMGFFBInputData& VehicleData)
{
	if (!IsFFBEnabled() || !FFBProcessor)
	{
		return;
	}

	FFBProcessor->ProcessVehicleData(VehicleData, CurrentProfile);

	// Track FFB latency
	double CurrentTime = FPlatformTime::Seconds();
	FFBLatencyMs = static_cast<float>((CurrentTime - LastFFBUpdateTime) * 1000.0);
	LastFFBUpdateTime = CurrentTime;

	// Check for clipping
	float TotalForce = FFBProcessor->GetTotalOutputForce();
	if (TotalForce > 1.0f)
	{
		CurrentFFBClipping = TotalForce - 1.0f;
		if (CurrentProfile.bShowFFBClipping)
		{
			OnFFBClipping.Broadcast(CurrentFFBClipping, 0.1f);
		}
	}
	else
	{
		CurrentFFBClipping = 0.0f;
	}
}

void UMGRacingWheelSubsystem::TriggerCollisionFFB(float Force, FVector Direction)
{
	if (!IsFFBEnabled())
	{
		return;
	}

	FMGFFBEffect Effect;
	Effect.EffectType = EMGFFBEffectType::SawtoothUp;
	Effect.Magnitude = FMath::Clamp(Force, 0.0f, 1.0f) * CurrentProfile.CollisionStrength;
	Effect.Duration = 0.15f;
	Effect.Frequency = 60.0f;

	// Determine direction based on impact
	if (Direction.Y > 0.3f)
	{
		Effect.DirectionDegrees = 90.0f; // Right impact
	}
	else if (Direction.Y < -0.3f)
	{
		Effect.DirectionDegrees = 270.0f; // Left impact
	}

	PlayFFBEffect(Effect);
}

FGuid UMGRacingWheelSubsystem::TriggerKerbFFB(float Intensity, float Duration)
{
	if (!IsFFBEnabled())
	{
		return FGuid();
	}

	FMGFFBEffect Effect;
	Effect.EffectType = EMGFFBEffectType::SquareWave;
	Effect.Magnitude = Intensity * CurrentProfile.CurbStrength;
	Effect.Duration = Duration;
	Effect.Frequency = 40.0f; // 40Hz rumble

	return PlayFFBEffect(Effect);
}

FGuid UMGRacingWheelSubsystem::TriggerSurfaceFFB(FName SurfaceType, float Intensity)
{
	if (!IsFFBEnabled())
	{
		return FGuid();
	}

	// Stop previous surface effect
	if (SurfaceEffectID.IsValid())
	{
		StopFFBEffect(SurfaceEffectID);
	}

	FMGFFBEffect Effect;
	Effect.EffectType = EMGFFBEffectType::SineWave;
	Effect.Duration = -1.0f; // Infinite

	// Adjust parameters based on surface type
	if (SurfaceType == FName("Gravel"))
	{
		Effect.Magnitude = 0.3f * Intensity;
		Effect.Frequency = 25.0f;
	}
	else if (SurfaceType == FName("Dirt"))
	{
		Effect.Magnitude = 0.2f * Intensity;
		Effect.Frequency = 18.0f;
	}
	else if (SurfaceType == FName("Grass"))
	{
		Effect.Magnitude = 0.15f * Intensity;
		Effect.Frequency = 12.0f;
	}
	else if (SurfaceType == FName("Sand"))
	{
		Effect.Magnitude = 0.25f * Intensity;
		Effect.Frequency = 20.0f;
	}
	else
	{
		// Default asphalt - minimal effect
		Effect.Magnitude = 0.0f;
	}

	SurfaceEffectID = PlayFFBEffect(Effect);
	return SurfaceEffectID;
}

void UMGRacingWheelSubsystem::UpdateEngineFFB(float RPMPercent)
{
	if (!IsFFBEnabled() || CurrentProfile.EngineVibrationStrength <= 0.0f)
	{
		return;
	}

	FMGFFBEffect Effect;
	Effect.EffectType = EMGFFBEffectType::SineWave;
	Effect.Duration = -1.0f;
	Effect.Magnitude = CurrentProfile.EngineVibrationStrength * RPMPercent * 0.15f;
	Effect.Frequency = 30.0f + RPMPercent * 80.0f; // 30-110 Hz based on RPM

	if (!EngineEffectID.IsValid())
	{
		EngineEffectID = PlayFFBEffect(Effect);
	}
	else
	{
		UpdateFFBEffect(EngineEffectID, Effect);
	}
}

void UMGRacingWheelSubsystem::SetSelfCentering(float Strength, float Coefficient)
{
	if (!IsFFBEnabled())
	{
		return;
	}

	FMGFFBEffect Effect;
	Effect.EffectType = EMGFFBEffectType::Spring;
	Effect.Magnitude = Strength * CurrentProfile.SelfCenteringStrength;
	Effect.Coefficient = Coefficient;
	Effect.Duration = -1.0f;
	Effect.CenterOffset = 0.0f;
	Effect.Deadband = 0.02f;
	Effect.Saturation = 1.0f;

	if (!SelfCenteringEffectID.IsValid())
	{
		SelfCenteringEffectID = PlayFFBEffect(Effect);
	}
	else
	{
		UpdateFFBEffect(SelfCenteringEffectID, Effect);
	}
}

void UMGRacingWheelSubsystem::SetDamperStrength(float Strength)
{
	if (!IsFFBEnabled())
	{
		return;
	}

	FMGFFBEffect Effect;
	Effect.EffectType = EMGFFBEffectType::Damper;
	Effect.Coefficient = Strength * CurrentProfile.DamperStrength;
	Effect.Duration = -1.0f;

	if (!DamperEffectID.IsValid())
	{
		DamperEffectID = PlayFFBEffect(Effect);
	}
	else
	{
		UpdateFFBEffect(DamperEffectID, Effect);
	}
}

// Profile Management
bool UMGRacingWheelSubsystem::LoadProfile(const FString& ProfileName)
{
	if (const FMGWheelProfile* Profile = AvailableProfiles.Find(ProfileName))
	{
		CurrentProfile = *Profile;
		UE_LOG(LogRacingWheel, Log, TEXT("Loaded wheel profile: %s"), *ProfileName);
		return true;
	}
	return false;
}

void UMGRacingWheelSubsystem::SaveProfile(const FString& ProfileName)
{
	CurrentProfile.ProfileName = ProfileName;
	AvailableProfiles.Add(ProfileName, CurrentProfile);
	SaveProfilesToDisk();
}

FMGWheelProfile UMGRacingWheelSubsystem::GetCurrentProfile() const
{
	return CurrentProfile;
}

void UMGRacingWheelSubsystem::SetCurrentProfile(const FMGWheelProfile& Profile)
{
	CurrentProfile = Profile;
}

TArray<FString> UMGRacingWheelSubsystem::GetAvailableProfiles() const
{
	TArray<FString> ProfileNames;
	AvailableProfiles.GetKeys(ProfileNames);
	return ProfileNames;
}

void UMGRacingWheelSubsystem::LoadDefaultProfileForWheel()
{
	FString DefaultProfileName;
	switch (ConnectedWheelModel)
	{
	case EMGWheelModel::Logitech_G920:
		DefaultProfileName = TEXT("Logitech_G920");
		break;
	case EMGWheelModel::Logitech_G29:
		DefaultProfileName = TEXT("Logitech_G29");
		break;
	case EMGWheelModel::Logitech_G923:
		DefaultProfileName = TEXT("Logitech_G923");
		break;
	default:
		DefaultProfileName = TEXT("Generic");
		break;
	}

	if (!LoadProfile(DefaultProfileName))
	{
		ResetProfileToDefaults();
	}
}

void UMGRacingWheelSubsystem::ResetProfileToDefaults()
{
	CurrentProfile = FMGWheelProfile();
	CurrentProfile.ProfileName = TEXT("Default");

	// Set wheel-specific defaults
	if (ConnectedWheelCapabilities.MaxRotationDegrees > 0.0f)
	{
		CurrentProfile.SteeringRotation = ConnectedWheelCapabilities.MaxRotationDegrees;
	}
}

void UMGRacingWheelSubsystem::StartCalibration()
{
	bIsCalibrating = true;
	CalibrationCenterOffset = 0;
}

void UMGRacingWheelSubsystem::FinishCalibration()
{
	if (bIsCalibrating)
	{
		// Apply calibration center offset
		CalibrationCenterOffset = CurrentWheelState.RawSteering;
		bIsCalibrating = false;
		SaveProfilesToDisk();
	}
}

void UMGRacingWheelSubsystem::CancelCalibration()
{
	bIsCalibrating = false;
}

bool UMGRacingWheelSubsystem::IsCalibrating() const
{
	return bIsCalibrating;
}

void UMGRacingWheelSubsystem::SetWheelCenter()
{
	CalibrationCenterOffset = CurrentWheelState.RawSteering;
}

float UMGRacingWheelSubsystem::GetFFBClippingAmount() const
{
	return CurrentFFBClipping;
}

void UMGRacingWheelSubsystem::GetRawAxisValues(int32& OutSteering, int32& OutThrottle, int32& OutBrake, int32& OutClutch) const
{
	OutSteering = CurrentWheelState.RawSteering;
	OutThrottle = CurrentWheelState.RawThrottle;
	OutBrake = CurrentWheelState.RawBrake;
	OutClutch = CurrentWheelState.RawClutch;
}

float UMGRacingWheelSubsystem::GetFFBLatencyMs() const
{
	return FFBLatencyMs;
}

FString UMGRacingWheelSubsystem::GetProfilePath() const
{
	return FPaths::ProjectSavedDir() / TEXT("WheelProfiles");
}

void UMGRacingWheelSubsystem::LoadProfilesFromDisk()
{
	// Load default profiles from Content/Data/Input/WheelProfiles/
	TArray<FString> ProfileFiles;
	FString ContentPath = FPaths::ProjectContentDir() / TEXT("Data/Input/WheelProfiles");
	IFileManager::Get().FindFiles(ProfileFiles, *(ContentPath / TEXT("*.json")), true, false);

	for (const FString& FileName : ProfileFiles)
	{
		FString FilePath = ContentPath / FileName;
		FString JsonString;
		if (FFileHelper::LoadFileToString(JsonString, *FilePath))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				FMGWheelProfile Profile;
				Profile.ProfileName = JsonObject->GetStringField(TEXT("ProfileName"));
				Profile.SteeringRotation = JsonObject->GetNumberField(TEXT("SteeringRotation"));
				Profile.SteeringDeadzone = JsonObject->GetNumberField(TEXT("SteeringDeadzone"));
				Profile.SteeringLinearity = JsonObject->GetNumberField(TEXT("SteeringLinearity"));
				Profile.FFBStrength = JsonObject->GetNumberField(TEXT("FFBStrength"));
				Profile.SelfCenteringStrength = JsonObject->GetNumberField(TEXT("SelfCenteringStrength"));
				Profile.RoadFeelStrength = JsonObject->GetNumberField(TEXT("RoadFeelStrength"));
				Profile.CollisionStrength = JsonObject->GetNumberField(TEXT("CollisionStrength"));
				Profile.CurbStrength = JsonObject->GetNumberField(TEXT("CurbStrength"));
				Profile.ThrottleDeadzone = JsonObject->GetNumberField(TEXT("ThrottleDeadzone"));
				Profile.BrakeDeadzone = JsonObject->GetNumberField(TEXT("BrakeDeadzone"));

				AvailableProfiles.Add(Profile.ProfileName, Profile);
			}
		}
	}

	// Load user-saved profiles from Saved/WheelProfiles/
	FString SavedPath = GetProfilePath();
	ProfileFiles.Empty();
	IFileManager::Get().FindFiles(ProfileFiles, *(SavedPath / TEXT("*.json")), true, false);

	for (const FString& FileName : ProfileFiles)
	{
		FString FilePath = SavedPath / FileName;
		FString JsonString;
		if (FFileHelper::LoadFileToString(JsonString, *FilePath))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				FMGWheelProfile Profile;
				LoadProfileFromJson(JsonObject, Profile);
				AvailableProfiles.Add(Profile.ProfileName, Profile);
			}
		}
	}

	UE_LOG(LogRacingWheel, Log, TEXT("Loaded %d wheel profiles"), AvailableProfiles.Num());
}

void UMGRacingWheelSubsystem::LoadProfileFromJson(const TSharedPtr<FJsonObject>& JsonObject, FMGWheelProfile& OutProfile)
{
	if (!JsonObject.IsValid())
	{
		return;
	}

	// Basic info
	OutProfile.ProfileName = JsonObject->GetStringField(TEXT("ProfileName"));

	// Steering settings - check for nested "Steering" object or flat structure
	const TSharedPtr<FJsonObject>* SteeringObj = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("Steering"), SteeringObj) && SteeringObj->IsValid())
	{
		const TSharedPtr<FJsonObject>& Steering = *SteeringObj;
		if (Steering->HasField(TEXT("SteeringRotation")))
			OutProfile.SteeringRotation = Steering->GetNumberField(TEXT("SteeringRotation"));
		if (Steering->HasField(TEXT("SteeringDeadzone")))
			OutProfile.SteeringDeadzone = Steering->GetNumberField(TEXT("SteeringDeadzone"));
		if (Steering->HasField(TEXT("SteeringLinearity")))
			OutProfile.SteeringLinearity = Steering->GetNumberField(TEXT("SteeringLinearity"));
		if (Steering->HasField(TEXT("InvertSteering")))
			OutProfile.bInvertSteering = Steering->GetBoolField(TEXT("InvertSteering"));
	}
	else
	{
		// Flat structure fallback
		if (JsonObject->HasField(TEXT("SteeringRotation")))
			OutProfile.SteeringRotation = JsonObject->GetNumberField(TEXT("SteeringRotation"));
		if (JsonObject->HasField(TEXT("SteeringDeadzone")))
			OutProfile.SteeringDeadzone = JsonObject->GetNumberField(TEXT("SteeringDeadzone"));
		if (JsonObject->HasField(TEXT("SteeringLinearity")))
			OutProfile.SteeringLinearity = JsonObject->GetNumberField(TEXT("SteeringLinearity"));
		if (JsonObject->HasField(TEXT("bInvertSteering")))
			OutProfile.bInvertSteering = JsonObject->GetBoolField(TEXT("bInvertSteering"));
	}

	// Pedal settings - check for nested "Pedals" object or flat structure
	const TSharedPtr<FJsonObject>* PedalsObj = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("Pedals"), PedalsObj) && PedalsObj->IsValid())
	{
		const TSharedPtr<FJsonObject>& Pedals = *PedalsObj;
		if (Pedals->HasField(TEXT("ThrottleDeadzone")))
			OutProfile.ThrottleDeadzone = Pedals->GetNumberField(TEXT("ThrottleDeadzone"));
		if (Pedals->HasField(TEXT("BrakeDeadzone")))
			OutProfile.BrakeDeadzone = Pedals->GetNumberField(TEXT("BrakeDeadzone"));
		if (Pedals->HasField(TEXT("ClutchDeadzone")))
			OutProfile.ClutchDeadzone = Pedals->GetNumberField(TEXT("ClutchDeadzone"));
		if (Pedals->HasField(TEXT("ThrottleGamma")))
			OutProfile.ThrottleGamma = Pedals->GetNumberField(TEXT("ThrottleGamma"));
		if (Pedals->HasField(TEXT("BrakeGamma")))
			OutProfile.BrakeGamma = Pedals->GetNumberField(TEXT("BrakeGamma"));
		if (Pedals->HasField(TEXT("CombinedPedals")))
			OutProfile.bCombinedPedals = Pedals->GetBoolField(TEXT("CombinedPedals"));
		if (Pedals->HasField(TEXT("InvertClutch")))
			OutProfile.bInvertClutch = Pedals->GetBoolField(TEXT("InvertClutch"));
	}
	else
	{
		// Flat structure fallback
		if (JsonObject->HasField(TEXT("ThrottleDeadzone")))
			OutProfile.ThrottleDeadzone = JsonObject->GetNumberField(TEXT("ThrottleDeadzone"));
		if (JsonObject->HasField(TEXT("BrakeDeadzone")))
			OutProfile.BrakeDeadzone = JsonObject->GetNumberField(TEXT("BrakeDeadzone"));
		if (JsonObject->HasField(TEXT("ClutchDeadzone")))
			OutProfile.ClutchDeadzone = JsonObject->GetNumberField(TEXT("ClutchDeadzone"));
		if (JsonObject->HasField(TEXT("ThrottleGamma")))
			OutProfile.ThrottleGamma = JsonObject->GetNumberField(TEXT("ThrottleGamma"));
		if (JsonObject->HasField(TEXT("BrakeGamma")))
			OutProfile.BrakeGamma = JsonObject->GetNumberField(TEXT("BrakeGamma"));
		if (JsonObject->HasField(TEXT("bCombinedPedals")))
			OutProfile.bCombinedPedals = JsonObject->GetBoolField(TEXT("bCombinedPedals"));
		if (JsonObject->HasField(TEXT("bInvertClutch")))
			OutProfile.bInvertClutch = JsonObject->GetBoolField(TEXT("bInvertClutch"));
	}

	// FFB settings - check for nested "ForceFeedback" object or flat structure
	const TSharedPtr<FJsonObject>* FFBObj = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("ForceFeedback"), FFBObj) && FFBObj->IsValid())
	{
		const TSharedPtr<FJsonObject>& FFB = *FFBObj;
		if (FFB->HasField(TEXT("FFBEnabled")))
			OutProfile.bFFBEnabled = FFB->GetBoolField(TEXT("FFBEnabled"));
		if (FFB->HasField(TEXT("FFBStrength")))
			OutProfile.FFBStrength = FFB->GetNumberField(TEXT("FFBStrength"));
		if (FFB->HasField(TEXT("SelfCenteringStrength")))
			OutProfile.SelfCenteringStrength = FFB->GetNumberField(TEXT("SelfCenteringStrength"));
		if (FFB->HasField(TEXT("RoadFeelStrength")))
			OutProfile.RoadFeelStrength = FFB->GetNumberField(TEXT("RoadFeelStrength"));
		if (FFB->HasField(TEXT("CollisionStrength")))
			OutProfile.CollisionStrength = FFB->GetNumberField(TEXT("CollisionStrength"));
		if (FFB->HasField(TEXT("CurbStrength")))
			OutProfile.CurbStrength = FFB->GetNumberField(TEXT("CurbStrength"));
		if (FFB->HasField(TEXT("EngineVibrationStrength")))
			OutProfile.EngineVibrationStrength = FFB->GetNumberField(TEXT("EngineVibrationStrength"));
		if (FFB->HasField(TEXT("UndersteerStrength")))
			OutProfile.UndersteerStrength = FFB->GetNumberField(TEXT("UndersteerStrength"));
		if (FFB->HasField(TEXT("OversteerStrength")))
			OutProfile.OversteerStrength = FFB->GetNumberField(TEXT("OversteerStrength"));
		if (FFB->HasField(TEXT("MinForceThreshold")))
			OutProfile.MinForceThreshold = FFB->GetNumberField(TEXT("MinForceThreshold"));
		if (FFB->HasField(TEXT("DamperStrength")))
			OutProfile.DamperStrength = FFB->GetNumberField(TEXT("DamperStrength"));
		if (FFB->HasField(TEXT("FrictionStrength")))
			OutProfile.FrictionStrength = FFB->GetNumberField(TEXT("FrictionStrength"));
		if (FFB->HasField(TEXT("ShowFFBClipping")))
			OutProfile.bShowFFBClipping = FFB->GetBoolField(TEXT("ShowFFBClipping"));
	}
	else
	{
		// Flat structure fallback
		if (JsonObject->HasField(TEXT("FFBStrength")))
			OutProfile.FFBStrength = JsonObject->GetNumberField(TEXT("FFBStrength"));
		if (JsonObject->HasField(TEXT("bFFBEnabled")))
			OutProfile.bFFBEnabled = JsonObject->GetBoolField(TEXT("bFFBEnabled"));
		if (JsonObject->HasField(TEXT("SelfCenteringStrength")))
			OutProfile.SelfCenteringStrength = JsonObject->GetNumberField(TEXT("SelfCenteringStrength"));
		if (JsonObject->HasField(TEXT("RoadFeelStrength")))
			OutProfile.RoadFeelStrength = JsonObject->GetNumberField(TEXT("RoadFeelStrength"));
		if (JsonObject->HasField(TEXT("CollisionStrength")))
			OutProfile.CollisionStrength = JsonObject->GetNumberField(TEXT("CollisionStrength"));
		if (JsonObject->HasField(TEXT("CurbStrength")))
			OutProfile.CurbStrength = JsonObject->GetNumberField(TEXT("CurbStrength"));
		if (JsonObject->HasField(TEXT("EngineVibrationStrength")))
			OutProfile.EngineVibrationStrength = JsonObject->GetNumberField(TEXT("EngineVibrationStrength"));
		if (JsonObject->HasField(TEXT("UndersteerStrength")))
			OutProfile.UndersteerStrength = JsonObject->GetNumberField(TEXT("UndersteerStrength"));
		if (JsonObject->HasField(TEXT("OversteerStrength")))
			OutProfile.OversteerStrength = JsonObject->GetNumberField(TEXT("OversteerStrength"));
		if (JsonObject->HasField(TEXT("MinForceThreshold")))
			OutProfile.MinForceThreshold = JsonObject->GetNumberField(TEXT("MinForceThreshold"));
		if (JsonObject->HasField(TEXT("DamperStrength")))
			OutProfile.DamperStrength = JsonObject->GetNumberField(TEXT("DamperStrength"));
		if (JsonObject->HasField(TEXT("FrictionStrength")))
			OutProfile.FrictionStrength = JsonObject->GetNumberField(TEXT("FrictionStrength"));
		if (JsonObject->HasField(TEXT("bShowFFBClipping")))
			OutProfile.bShowFFBClipping = JsonObject->GetBoolField(TEXT("bShowFFBClipping"));
	}
}

void UMGRacingWheelSubsystem::SaveProfilesToDisk()
{
	FString SavePath = GetProfilePath();
	IFileManager::Get().MakeDirectory(*SavePath, true);

	for (const auto& Pair : AvailableProfiles)
	{
		const FMGWheelProfile& Profile = Pair.Value;

		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

		// Basic info
		JsonObject->SetStringField(TEXT("ProfileName"), Profile.ProfileName);

		// Steering settings
		JsonObject->SetNumberField(TEXT("SteeringRotation"), Profile.SteeringRotation);
		JsonObject->SetNumberField(TEXT("SteeringDeadzone"), Profile.SteeringDeadzone);
		JsonObject->SetNumberField(TEXT("SteeringLinearity"), Profile.SteeringLinearity);
		JsonObject->SetBoolField(TEXT("bInvertSteering"), Profile.bInvertSteering);

		// Pedal settings
		JsonObject->SetNumberField(TEXT("ThrottleDeadzone"), Profile.ThrottleDeadzone);
		JsonObject->SetNumberField(TEXT("BrakeDeadzone"), Profile.BrakeDeadzone);
		JsonObject->SetNumberField(TEXT("ClutchDeadzone"), Profile.ClutchDeadzone);
		JsonObject->SetNumberField(TEXT("ThrottleGamma"), Profile.ThrottleGamma);
		JsonObject->SetNumberField(TEXT("BrakeGamma"), Profile.BrakeGamma);
		JsonObject->SetBoolField(TEXT("bCombinedPedals"), Profile.bCombinedPedals);
		JsonObject->SetBoolField(TEXT("bInvertClutch"), Profile.bInvertClutch);

		// FFB settings
		JsonObject->SetNumberField(TEXT("FFBStrength"), Profile.FFBStrength);
		JsonObject->SetBoolField(TEXT("bFFBEnabled"), Profile.bFFBEnabled);
		JsonObject->SetNumberField(TEXT("SelfCenteringStrength"), Profile.SelfCenteringStrength);
		JsonObject->SetNumberField(TEXT("RoadFeelStrength"), Profile.RoadFeelStrength);
		JsonObject->SetNumberField(TEXT("CollisionStrength"), Profile.CollisionStrength);
		JsonObject->SetNumberField(TEXT("CurbStrength"), Profile.CurbStrength);
		JsonObject->SetNumberField(TEXT("EngineVibrationStrength"), Profile.EngineVibrationStrength);
		JsonObject->SetNumberField(TEXT("UndersteerStrength"), Profile.UndersteerStrength);
		JsonObject->SetNumberField(TEXT("OversteerStrength"), Profile.OversteerStrength);
		JsonObject->SetNumberField(TEXT("MinForceThreshold"), Profile.MinForceThreshold);
		JsonObject->SetNumberField(TEXT("DamperStrength"), Profile.DamperStrength);
		JsonObject->SetNumberField(TEXT("FrictionStrength"), Profile.FrictionStrength);
		JsonObject->SetBoolField(TEXT("bShowFFBClipping"), Profile.bShowFFBClipping);

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

		FString FilePath = SavePath / Profile.ProfileName + TEXT(".json");
		FFileHelper::SaveStringToFile(OutputString, *FilePath);
	}
}

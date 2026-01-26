// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MGRacingWheelTypes.generated.h"

/**
 * Racing wheel manufacturer identification
 */
UENUM(BlueprintType)
enum class EMGWheelManufacturer : uint8
{
	Unknown,
	Logitech,
	Thrustmaster,
	Fanatec,
	Generic
};

/**
 * Specific wheel model identification
 */
UENUM(BlueprintType)
enum class EMGWheelModel : uint8
{
	Unknown,
	// Logitech
	Logitech_G920,
	Logitech_G29,
	Logitech_G923,
	Logitech_G27,
	Logitech_G25,
	Logitech_DFGT,
	// Thrustmaster
	Thrustmaster_T300RS,
	Thrustmaster_T500RS,
	Thrustmaster_TX,
	Thrustmaster_TMX,
	Thrustmaster_T150,
	Thrustmaster_T248,
	// Fanatec
	Fanatec_CSL_DD,
	Fanatec_DD_Pro,
	Fanatec_Podium,
	Fanatec_CSL_Elite,
	// Generic DirectInput
	Generic_DirectInput
};

/**
 * Force feedback effect types
 */
UENUM(BlueprintType)
enum class EMGFFBEffectType : uint8
{
	None,
	/** Constant directional force */
	ConstantForce,
	/** Spring effect - resists displacement from center */
	Spring,
	/** Damper effect - resists velocity of movement */
	Damper,
	/** Friction effect - constant resistance to movement */
	Friction,
	/** Inertia effect - resists acceleration */
	Inertia,
	/** Sine wave periodic effect */
	SineWave,
	/** Square wave periodic effect */
	SquareWave,
	/** Triangle wave periodic effect */
	TriangleWave,
	/** Sawtooth (up) periodic effect */
	SawtoothUp,
	/** Sawtooth (down) periodic effect */
	SawtoothDown,
	/** Custom effect loaded from file */
	Custom
};

/**
 * FFB effect playback state
 */
UENUM(BlueprintType)
enum class EMGFFBEffectState : uint8
{
	Stopped,
	Playing,
	Paused
};

/**
 * Wheel connection state
 */
UENUM(BlueprintType)
enum class EMGWheelConnectionState : uint8
{
	Disconnected,
	Connecting,
	Connected,
	Error
};

/**
 * Wheel capabilities structure
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGWheelCapabilities
{
	GENERATED_BODY()

	/** Manufacturer of the wheel */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	EMGWheelManufacturer Manufacturer = EMGWheelManufacturer::Unknown;

	/** Specific wheel model */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	EMGWheelModel Model = EMGWheelModel::Unknown;

	/** Device name as reported by the driver */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	FString DeviceName;

	/** USB Vendor ID */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	int32 VendorID = 0;

	/** USB Product ID */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	int32 ProductID = 0;

	/** Maximum wheel rotation in degrees (e.g., 900, 1080) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	float MaxRotationDegrees = 900.0f;

	/** Number of pedals (typically 2 or 3) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	int32 PedalCount = 2;

	/** Whether the wheel has a clutch pedal */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	bool bHasClutch = false;

	/** Whether the wheel has paddle shifters */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	bool bHasPaddleShifters = true;

	/** Whether the wheel has an H-pattern shifter */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	bool bHasHPatternShifter = false;

	/** Whether force feedback is supported */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	bool bSupportsForceFeedback = true;

	/** Number of FFB axes */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	int32 FFBAxisCount = 1;

	/** Number of buttons */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	int32 ButtonCount = 11;

	/** Whether the wheel has a D-pad */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	bool bHasDPad = true;

	/** Maximum FFB force in Nm (if known) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	float MaxFFBForceNm = 2.5f;

	/** Supported FFB effect types */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|Capabilities")
	TArray<EMGFFBEffectType> SupportedEffects;
};

/**
 * Current wheel input state
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGWheelState
{
	GENERATED_BODY()

	/** Steering angle in degrees (-MaxRotation/2 to +MaxRotation/2) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	float SteeringAngle = 0.0f;

	/** Normalized steering input (-1 to 1) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	float SteeringNormalized = 0.0f;

	/** Throttle pedal position (0 to 1) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	float ThrottlePedal = 0.0f;

	/** Brake pedal position (0 to 1) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	float BrakePedal = 0.0f;

	/** Clutch pedal position (0 to 1) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	float ClutchPedal = 0.0f;

	/** Handbrake position (0 to 1) if available */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	float Handbrake = 0.0f;

	/** Button states (bitmask) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	int32 ButtonStates = 0;

	/** D-pad direction (-1 = none, 0-7 = direction) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	int32 DPadDirection = -1;

	/** Current H-pattern shifter gear (0 = neutral, -1 = reverse) */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	int32 ShifterGear = 0;

	/** Left paddle shifter pressed this frame */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	bool bLeftPaddlePressed = false;

	/** Right paddle shifter pressed this frame */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State")
	bool bRightPaddlePressed = false;

	/** Raw steering axis value before processing */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State|Raw")
	int32 RawSteering = 0;

	/** Raw throttle axis value */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State|Raw")
	int32 RawThrottle = 0;

	/** Raw brake axis value */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State|Raw")
	int32 RawBrake = 0;

	/** Raw clutch axis value */
	UPROPERTY(BlueprintReadOnly, Category = "Wheel|State|Raw")
	int32 RawClutch = 0;
};

/**
 * Force feedback effect parameters
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGFFBEffect
{
	GENERATED_BODY()

	/** Unique identifier for this effect instance */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Effect")
	FGuid EffectID;

	/** Type of force feedback effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect")
	EMGFFBEffectType EffectType = EMGFFBEffectType::None;

	/** Effect magnitude (0 to 1, or -1 to 1 for directional effects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Magnitude = 0.0f;

	/** Direction of force in degrees (0-360, only for directional effects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect", meta = (ClampMin = "0", ClampMax = "360"))
	float DirectionDegrees = 0.0f;

	/** Effect duration in seconds (-1 = infinite) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect")
	float Duration = -1.0f;

	/** Delay before effect starts in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect")
	float StartDelay = 0.0f;

	/** Frequency in Hz for periodic effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Periodic", meta = (ClampMin = "0", ClampMax = "500"))
	float Frequency = 40.0f;

	/** Phase offset for periodic effects (0-360 degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Periodic", meta = (ClampMin = "0", ClampMax = "360"))
	float Phase = 0.0f;

	/** Offset for periodic effects (-1 to 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Periodic", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Offset = 0.0f;

	/** Attack time in seconds (fade in) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Envelope")
	float AttackTime = 0.0f;

	/** Attack level (starting magnitude) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Envelope", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackLevel = 0.0f;

	/** Fade time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Envelope")
	float FadeTime = 0.0f;

	/** Fade level (ending magnitude) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Envelope", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FadeLevel = 0.0f;

	/** For spring/damper: coefficient (stiffness/resistance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Coefficient = 0.5f;

	/** For spring: center point offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Condition", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float CenterOffset = 0.0f;

	/** For spring: deadband around center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Deadband = 0.0f;

	/** For spring: saturation (max force limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect|Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Saturation = 1.0f;

	/** Current playback state */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Effect")
	EMGFFBEffectState State = EMGFFBEffectState::Stopped;

	/** Priority for effect blending (higher = more important) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FFB|Effect")
	int32 Priority = 0;

	FMGFFBEffect()
	{
		EffectID = FGuid::NewGuid();
	}
};

/**
 * Per-wheel profile configuration
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGWheelProfile
{
	GENERATED_BODY()

	/** Profile name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString ProfileName = TEXT("Default");

	/** Target wheel model (Unknown = apply to all) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	EMGWheelModel TargetModel = EMGWheelModel::Unknown;

	// === Steering Configuration ===

	/** Steering rotation range in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Steering", meta = (ClampMin = "180", ClampMax = "1440"))
	float SteeringRotation = 900.0f;

	/** Steering deadzone (normalized 0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Steering", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float SteeringDeadzone = 0.0f;

	/** Steering sensitivity/linearity curve exponent (1 = linear) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Steering", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float SteeringLinearity = 1.0f;

	/** Invert steering axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Steering")
	bool bInvertSteering = false;

	// === Pedal Configuration ===

	/** Throttle pedal deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Pedals", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float ThrottleDeadzone = 0.05f;

	/** Brake pedal deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Pedals", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float BrakeDeadzone = 0.05f;

	/** Clutch pedal deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Pedals", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float ClutchDeadzone = 0.1f;

	/** Throttle sensitivity curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Pedals", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float ThrottleGamma = 1.0f;

	/** Brake sensitivity curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Pedals", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float BrakeGamma = 1.0f;

	/** Use combined pedal axis (old wheels) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Pedals")
	bool bCombinedPedals = false;

	/** Invert clutch pedal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|Pedals")
	bool bInvertClutch = false;

	// === Force Feedback Configuration ===

	/** Master FFB strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FFBStrength = 0.7f;

	/** Enable force feedback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB")
	bool bFFBEnabled = true;

	/** Self-centering spring strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SelfCenteringStrength = 0.5f;

	/** Road feel / aligning torque strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RoadFeelStrength = 0.6f;

	/** Collision impact strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CollisionStrength = 0.8f;

	/** Curb/rumble strip strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurbStrength = 0.5f;

	/** Engine vibration strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EngineVibrationStrength = 0.3f;

	/** Understeer feedback strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UndersteerStrength = 0.4f;

	/** Oversteer feedback strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OversteerStrength = 0.6f;

	/** Minimum force threshold (helps with weak motors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float MinForceThreshold = 0.02f;

	/** Damper strength (smooths FFB) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamperStrength = 0.2f;

	/** Friction strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FrictionStrength = 0.1f;

	/** Enable FFB clipping notification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|FFB")
	bool bShowFFBClipping = true;
};

/**
 * Data for calculating FFB from vehicle physics
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGFFBInputData
{
	GENERATED_BODY()

	/** Vehicle speed in km/h */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float SpeedKmh = 0.0f;

	/** Current steering angle (normalized -1 to 1) */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float SteeringAngle = 0.0f;

	/** Front tire slip angle in degrees */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float FrontSlipAngle = 0.0f;

	/** Rear tire slip angle in degrees */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float RearSlipAngle = 0.0f;

	/** Front left tire slip ratio */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float FrontLeftSlipRatio = 0.0f;

	/** Front right tire slip ratio */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float FrontRightSlipRatio = 0.0f;

	/** Lateral G-force */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float LateralG = 0.0f;

	/** Longitudinal G-force */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float LongitudinalG = 0.0f;

	/** Yaw rate in degrees/second */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float YawRate = 0.0f;

	/** Is the vehicle understeering */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	bool bIsUndersteering = false;

	/** Is the vehicle oversteering */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	bool bIsOversteering = false;

	/** Current surface type */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	FName SurfaceType = NAME_None;

	/** On rumble strip */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	bool bOnRumbleStrip = false;

	/** Engine RPM */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float EngineRPM = 0.0f;

	/** Max engine RPM */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float MaxEngineRPM = 7000.0f;

	/** Collision impact force this frame (0 if none) */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float CollisionImpact = 0.0f;

	/** Collision impact direction (local space) */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	FVector CollisionDirection = FVector::ZeroVector;

	/** Is currently drifting */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	bool bIsDrifting = false;

	/** Drift angle in degrees */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float DriftAngle = 0.0f;

	/** Suspension travel front left (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float SuspensionFL = 0.5f;

	/** Suspension travel front right (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float SuspensionFR = 0.5f;

	/** Front tire load (normalized) */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	float FrontTireLoad = 1.0f;

	/** Is airborne */
	UPROPERTY(BlueprintReadOnly, Category = "FFB|Input")
	bool bIsAirborne = false;
};

/**
 * Known wheel database entry
 */
USTRUCT()
struct FMGKnownWheelEntry
{
	GENERATED_BODY()

	/** USB Vendor ID */
	int32 VendorID = 0;

	/** USB Product ID */
	int32 ProductID = 0;

	/** Wheel manufacturer */
	EMGWheelManufacturer Manufacturer = EMGWheelManufacturer::Unknown;

	/** Wheel model */
	EMGWheelModel Model = EMGWheelModel::Unknown;

	/** Default capabilities */
	FMGWheelCapabilities DefaultCapabilities;

	FMGKnownWheelEntry() {}

	FMGKnownWheelEntry(int32 InVID, int32 InPID, EMGWheelManufacturer InMfr, EMGWheelModel InModel)
		: VendorID(InVID), ProductID(InPID), Manufacturer(InMfr), Model(InModel)
	{
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWheelConnected, EMGWheelModel, Model, const FMGWheelCapabilities&, Capabilities);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWheelDisconnected, EMGWheelModel, Model);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWheelStateUpdated, const FMGWheelState&, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFFBClipping, float, ClipAmount, float, Duration);

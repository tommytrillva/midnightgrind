// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGRacingWheelTypes.h
 * =============================================================================
 *
 * PURPOSE:
 * This file defines all the data structures (structs) and enumerations (enums)
 * used by the Racing Wheel system. Think of it as the "dictionary" that defines
 * what kinds of data we need to represent racing wheel hardware and its features.
 *
 * WHY THIS FILE EXISTS:
 * In Unreal Engine (and C++ in general), it's a best practice to separate type
 * definitions from the code that uses them. This allows multiple files to include
 * just the types they need without creating circular dependencies.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. ENUMS (Enumerations):
 *    Enums are lists of named values. For example, EMGWheelManufacturer lists
 *    all wheel brands we support (Logitech, Thrustmaster, etc.). Using enums
 *    instead of strings or numbers makes code more readable and prevents typos.
 *    The "E" prefix is Unreal convention for enum types.
 *
 * 2. STRUCTS (Structures):
 *    Structs bundle related data together. For example, FMGWheelState groups
 *    all input values (steering angle, pedal positions, buttons) into one package.
 *    The "F" prefix is Unreal convention for struct types.
 *
 * 3. UPROPERTY / UENUM / USTRUCT Macros:
 *    These Unreal macros expose C++ code to the engine's reflection system.
 *    This enables:
 *    - Blueprint access (visual scripting)
 *    - Editor visibility (property panels)
 *    - Serialization (saving/loading)
 *    - Garbage collection awareness
 *
 * 4. BlueprintType / BlueprintReadOnly / BlueprintReadWrite:
 *    These specifiers control how Blueprints can interact with the data:
 *    - BlueprintType: The type can be used as a variable in Blueprints
 *    - BlueprintReadOnly: Blueprints can read but not modify
 *    - BlueprintReadWrite: Blueprints can read and modify
 *
 * 5. Force Feedback (FFB):
 *    FFB is what makes the steering wheel push back against your hands.
 *    Different "effects" create different sensations:
 *    - Constant: Steady push in one direction (like wind or banking)
 *    - Spring: Pulls wheel back to center (like real car steering)
 *    - Damper: Resists fast movements (smooths out jerky inputs)
 *    - Periodic: Vibrations (rumble strips, engine vibration)
 *
 * HOW THIS FITS IN THE ARCHITECTURE:
 *
 *   [MGRacingWheelTypes.h] <-- You are here (data definitions)
 *          ^
 *          | (includes)
 *          |
 *   [MGRacingWheelSubsystem.h] -- Main controller class
 *          ^
 *          | (uses)
 *          |
 *   [MGWheelFFBProcessor.h] -- FFB calculation logic
 *          ^
 *          | (receives data from)
 *          |
 *   [Vehicle/Physics Code] -- Your car's physics system
 *
 * COMMON MODIFICATIONS:
 * - Add new wheel models to EMGWheelModel when supporting new hardware
 * - Add new effect types to EMGFFBEffectType for custom sensations
 * - Extend FMGWheelProfile with new tuning parameters
 * - Add fields to FMGFFBInputData if vehicle physics provide more data
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "MGRacingWheelTypes.generated.h"

/**
 * Racing wheel manufacturer identification
 *
 * Each manufacturer uses different communication protocols and has different
 * FFB capabilities. We identify the manufacturer to apply appropriate defaults
 * and enable manufacturer-specific features.
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
 *
 * Each wheel model has different capabilities (rotation range, button count,
 * FFB strength, etc.). By identifying the specific model, we can:
 * - Apply correct default settings
 * - Enable/disable features the wheel supports
 * - Display the correct wheel name to the user
 *
 * ADDING NEW WHEELS:
 * When adding support for a new wheel model:
 * 1. Add the enum value here
 * 2. Add its VID/PID to the KnownWheelDatabase in MGRacingWheelSubsystem.cpp
 * 3. Set up default capabilities for the wheel
 */
UENUM(BlueprintType)
enum class EMGWheelModel : uint8
{
	Unknown,
	// Logitech wheels - Belt/gear-driven, good mid-range FFB
	Logitech_G920,      // Xbox/PC version
	Logitech_G29,       // PlayStation/PC version
	Logitech_G923,      // Latest generation with TrueForce
	Logitech_G27,       // Legacy wheel with H-pattern shifter
	Logitech_G25,       // Older legacy wheel
	Logitech_DFGT,      // Driving Force GT - budget option
	// Thrustmaster wheels - Belt-driven, strong FFB
	Thrustmaster_T300RS,  // PlayStation/PC, excellent FFB
	Thrustmaster_T500RS,  // Older high-end model
	Thrustmaster_TX,      // Xbox/PC version of T300
	Thrustmaster_TMX,     // Budget Xbox/PC wheel
	Thrustmaster_T150,    // Budget PlayStation/PC wheel
	Thrustmaster_T248,    // Mid-range hybrid drive
	// Fanatec wheels - Direct-drive, professional-grade FFB
	Fanatec_CSL_DD,       // Entry direct-drive
	Fanatec_DD_Pro,       // PlayStation direct-drive
	Fanatec_Podium,       // High-end direct-drive
	Fanatec_CSL_Elite,    // Belt-driven high-end
	// Generic DirectInput - For any wheel we don't specifically recognize
	Generic_DirectInput
};

/**
 * Force feedback effect types
 *
 * FFB effects are categorized into several families:
 *
 * 1. CONSTANT FORCES:
 *    Push the wheel steadily in one direction. Used for simulating forces
 *    like road camber, wind resistance, or weight transfer.
 *
 * 2. CONDITION EFFECTS (Spring, Damper, Friction, Inertia):
 *    These react to wheel position or movement rather than applying
 *    a fixed force. They're fundamental to making the wheel feel "alive."
 *
 * 3. PERIODIC EFFECTS (Sine, Square, Triangle, Sawtooth):
 *    Oscillating forces that create vibrations. Different waveforms
 *    create different textures - sine is smooth, square is harsh,
 *    triangle is somewhere in between.
 *
 * REAL-WORLD EXAMPLES:
 * - Hitting a rumble strip: Square wave periodic effect
 * - Engine vibration: Sine wave at engine frequency
 * - Self-centering: Spring effect centered at neutral
 * - Smooth steering: Damper effect to reduce oscillation
 */
UENUM(BlueprintType)
enum class EMGFFBEffectType : uint8
{
	None,
	/** Constant directional force - steady push in one direction */
	ConstantForce,
	/** Spring effect - resists displacement from center (self-centering) */
	Spring,
	/** Damper effect - resists velocity of movement (smooths steering) */
	Damper,
	/** Friction effect - constant resistance to movement (heavy steering feel) */
	Friction,
	/** Inertia effect - resists acceleration (wheel has "weight") */
	Inertia,
	/** Sine wave periodic effect - smooth vibration (engine rumble) */
	SineWave,
	/** Square wave periodic effect - harsh vibration (rumble strips) */
	SquareWave,
	/** Triangle wave periodic effect - medium vibration texture */
	TriangleWave,
	/** Sawtooth (up) periodic effect - asymmetric vibration */
	SawtoothUp,
	/** Sawtooth (down) periodic effect - asymmetric vibration (reverse direction) */
	SawtoothDown,
	/** Custom effect loaded from file - for special scenarios */
	Custom
};

/**
 * FFB effect playback state
 *
 * Effects can be in one of three states. This is similar to how audio
 * playback works - you can play, pause, or stop an effect.
 *
 * Pausing is useful for menus or cutscenes where you want to resume
 * the exact effect state when gameplay returns.
 */
UENUM(BlueprintType)
enum class EMGFFBEffectState : uint8
{
	Stopped,    // Effect is not running and will restart from beginning
	Playing,    // Effect is actively applying forces
	Paused      // Effect is suspended but remembers its state
};

/**
 * Wheel connection state
 *
 * Tracks the USB connection lifecycle. This is important for:
 * - Showing appropriate UI (controller disconnected warnings)
 * - Gracefully handling hot-plug (connecting/disconnecting during play)
 * - Recovering from errors (USB reset, driver issues)
 */
UENUM(BlueprintType)
enum class EMGWheelConnectionState : uint8
{
	Disconnected,  // No wheel detected on the system
	Connecting,    // Wheel found, initializing communication
	Connected,     // Wheel is ready and responding
	Error          // Wheel found but not responding correctly
};

/**
 * Wheel capabilities structure
 *
 * This struct describes what a wheel can do. Different wheels have
 * different features - some have clutch pedals, some have H-pattern
 * shifters, some have more buttons, etc.
 *
 * WHY THIS MATTERS:
 * - We don't want to show clutch options if the wheel has no clutch
 * - We need to know max rotation to calculate steering angles correctly
 * - FFB strength varies wildly between wheels (2.5Nm budget vs 25Nm direct-drive)
 * - Some wheels don't support certain FFB effect types
 *
 * This struct is populated when a wheel connects, either from our
 * known wheel database or by querying the device directly.
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
 *
 * This struct contains the current position/state of all wheel controls.
 * It's updated every frame (or faster) by reading from the hardware.
 *
 * TWO REPRESENTATIONS OF DATA:
 * The struct contains both "processed" and "raw" values:
 *
 * - PROCESSED values (SteeringNormalized, ThrottlePedal, etc.):
 *   Converted to useful ranges (-1 to 1 for steering, 0 to 1 for pedals).
 *   These have deadzone and sensitivity curves applied.
 *   USE THESE for gameplay code.
 *
 * - RAW values (RawSteering, RawThrottle, etc.):
 *   The exact values from the hardware (usually 0-65535 or similar).
 *   USE THESE for calibration, debugging, or custom processing.
 *
 * COORDINATE SYSTEM:
 * - Steering: Negative = left, Positive = right, Zero = center
 * - Pedals: 0 = released, 1 = fully pressed
 * - D-pad: Uses clock positions (0=up, 2=right, 4=down, 6=left, etc.)
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
 *
 * This struct defines everything about a single FFB effect - what type it is,
 * how strong, how long it lasts, its shape over time, etc.
 *
 * EFFECT LIFECYCLE:
 * 1. Create an FMGFFBEffect and set its parameters
 * 2. Call PlayFFBEffect() on the subsystem - you get back an EffectID (GUID)
 * 3. Use the EffectID to update, pause, or stop the effect later
 * 4. Effects with Duration > 0 stop automatically; Duration = -1 plays forever
 *
 * ENVELOPE (Attack/Fade):
 * Effects can ramp up (attack) and ramp down (fade) for smoother feel.
 * Example: A collision impact might have instant attack, slow fade.
 *
 *   Force ^
 *         |    /--------\
 *         |   /          \
 *         |  /            \
 *         | /              \
 *         |/________________\____> Time
 *          |Attack| Main |Fade|
 *
 * CONDITION PARAMETERS (Spring/Damper):
 * For condition effects, additional parameters control the response curve:
 * - Coefficient: How strong the effect is (stiffness for spring)
 * - CenterOffset: Where the "center point" is (usually 0)
 * - Deadband: Range around center with no force
 * - Saturation: Maximum force limit (clipping)
 *
 * PERIODIC PARAMETERS (Waves):
 * For periodic effects, these control the wave shape:
 * - Frequency: How fast it oscillates (Hz)
 * - Phase: Starting point in the wave cycle (degrees)
 * - Offset: DC offset (shifts the wave up/down)
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
 *
 * A "profile" is a saved configuration of all wheel settings. This allows:
 * - Different settings for different wheels (a G920 needs different FFB than a CSL DD)
 * - Player preferences (some like heavy FFB, others light)
 * - Game mode settings (casual vs simulation)
 * - Car-specific tuning (drift car vs grip car)
 *
 * PROFILE CATEGORIES:
 *
 * 1. STEERING SETTINGS:
 *    Control how wheel rotation maps to in-game steering.
 *    - Rotation: How many degrees of wheel turn = full lock
 *    - Deadzone: Small movements near center are ignored
 *    - Linearity: 1.0 = linear, <1 = more sensitive near center, >1 = less
 *
 * 2. PEDAL SETTINGS:
 *    Control throttle, brake, and clutch response.
 *    - Deadzone: Ignore tiny inputs (prevents creeping)
 *    - Gamma: Response curve (1.0 = linear, <1 = progressive, >1 = aggressive)
 *    - Combined pedals: For old wheels where throttle+brake share one axis
 *
 * 3. FORCE FEEDBACK SETTINGS:
 *    Control what you feel through the wheel.
 *    - Master strength: Overall FFB intensity
 *    - Per-effect strengths: Fine-tune individual sensations
 *    - Damper/Friction: How "heavy" the wheel feels
 *
 * TUNING TIPS FOR DEVELOPERS:
 * - Start with FFBStrength around 0.7 (70%) and adjust based on wheel
 * - Direct-drive wheels need MUCH lower values than belt-driven
 * - MinForceThreshold helps weak motors feel responsive
 * - Too much damper makes the wheel feel sluggish
 * - Too little damper makes it feel twitchy/oscillating
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGWheelProfile
{
	GENERATED_BODY()

	/** Profile name for save/load */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString ProfileName = TEXT("Default");

	/** Target wheel model - profile only applies to this wheel (Unknown = any wheel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	EMGWheelModel TargetModel = EMGWheelModel::Unknown;

	// === Steering Configuration ===
	// These settings control how physical wheel rotation translates to game input

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
	// These settings control throttle, brake, and clutch pedal response

	/** Throttle pedal deadzone - inputs below this threshold are ignored */
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
	// These settings control what forces you feel through the wheel
	// Adjust these based on your wheel's power and personal preference

	/** Master FFB strength (0-1) - scales ALL force feedback effects */
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
 *
 * This struct is the "bridge" between your vehicle physics and the FFB system.
 * The vehicle fills this with its current state every frame, and the FFB
 * processor uses it to calculate appropriate forces.
 *
 * WHERE THIS DATA COMES FROM:
 * Most fields map directly to Chaos Vehicle or custom physics outputs:
 * - Speed, RPM: Direct vehicle state
 * - Slip angles/ratios: Tire physics calculations
 * - G-forces: Acceleration divided by gravity
 * - Suspension: Wheel query results
 *
 * KEY PHYSICS CONCEPTS:
 *
 * SLIP ANGLE:
 * The angle between where the tire is pointing and where it's actually going.
 * Small slip angle = grip. Large slip angle = sliding/drifting.
 * This is THE most important value for realistic FFB!
 *
 * SLIP RATIO:
 * The difference between wheel speed and road speed.
 * 0 = perfect grip, positive = wheelspin, negative = lockup.
 *
 * UNDERSTEER vs OVERSTEER:
 * - Understeer: Front tires slip more than rears. Car pushes wide.
 *   FFB: Wheel goes "light" (less self-centering)
 * - Oversteer: Rear tires slip more than fronts. Rear swings out.
 *   FFB: Counter-steer force helps player catch the slide
 *
 * IMPLEMENTATION TIP:
 * Don't worry about filling every field initially. Start with the basics
 * (speed, steering angle, maybe slip angles) and add more as needed.
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
 *
 * This struct stores information about a wheel model we recognize.
 * When a wheel connects, we check its USB VID/PID against our database
 * to identify it and apply appropriate defaults.
 *
 * USB IDENTIFICATION:
 * Every USB device has two ID numbers:
 * - VID (Vendor ID): Identifies the manufacturer (e.g., Logitech = 0x046D)
 * - PID (Product ID): Identifies the specific product
 *
 * Together, VID+PID uniquely identify a device model. This is how we know
 * a Logitech G920 from a G29, even though they're functionally similar.
 *
 * EXTENDING THE DATABASE:
 * To add support for a new wheel:
 * 1. Find its VID/PID (Windows Device Manager, or USB descriptor tools)
 * 2. Create an entry with the VID, PID, manufacturer, and model
 * 3. Fill in DefaultCapabilities with the wheel's actual specs
 * 4. Add the entry to KnownWheelDatabase in MGRacingWheelSubsystem.cpp
 */
USTRUCT()
struct FMGKnownWheelEntry
{
	GENERATED_BODY()

	/** USB Vendor ID - identifies the manufacturer */
	int32 VendorID = 0;

	/** USB Product ID - identifies the specific product */
	int32 ProductID = 0;

	/** Wheel manufacturer enum */
	EMGWheelManufacturer Manufacturer = EMGWheelManufacturer::Unknown;

	/** Wheel model enum */
	EMGWheelModel Model = EMGWheelModel::Unknown;

	/** Pre-configured capabilities for this wheel model */
	FMGWheelCapabilities DefaultCapabilities;

	FMGKnownWheelEntry() {}

	FMGKnownWheelEntry(int32 InVID, int32 InPID, EMGWheelManufacturer InMfr, EMGWheelModel InModel)
		: VendorID(InVID), ProductID(InPID), Manufacturer(InMfr), Model(InModel)
	{
	}
};

// =============================================================================
// DELEGATE DECLARATIONS
// =============================================================================
//
// Delegates are Unreal's event/callback system. They let you "subscribe" to
// events and get notified when they happen.
//
// HOW TO USE DELEGATES:
//
// In Blueprint:
//   1. Get a reference to the RacingWheelSubsystem
//   2. Find the event (e.g., "On Wheel Connected")
//   3. Drag off it and select "Bind Event"
//   4. Connect it to your handler function
//
// In C++:
//   WheelSubsystem->OnWheelConnected.AddDynamic(this, &MyClass::HandleWheelConnected);
//
// IMPORTANT: Dynamic delegates (Blueprint-compatible) have some overhead.
// For performance-critical code called every frame, consider using the
// getter functions directly instead of subscribing to OnWheelStateUpdated.
//
// =============================================================================

/** Called when a racing wheel is connected. Use this to show wheel-specific UI or enable features. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWheelConnected, EMGWheelModel, Model, const FMGWheelCapabilities&, Capabilities);

/** Called when a racing wheel is disconnected. Use this to show reconnection prompts or fall back to gamepad. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWheelDisconnected, EMGWheelModel, Model);

/** Called every frame with updated wheel state. WARNING: High frequency - prefer polling GetWheelState() instead. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWheelStateUpdated, const FMGWheelState&, State);

/** Called when FFB forces exceed the wheel's capacity (clipping). Use this to show a warning indicator. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFFBClipping, float, ClipAmount, float, Duration);

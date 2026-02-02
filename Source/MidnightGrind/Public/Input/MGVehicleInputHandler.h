// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGVehicleInputHandler.h
 * @brief Vehicle input processing component with controller support, input assists, and force feedback.
 *
 * @section Overview
 * The Vehicle Input Handler is a component that processes all player input for
 * vehicle control in Midnight Grind. It sits between raw input devices and the
 * vehicle movement system, handling input smoothing, assists, and feedback.
 *
 * This component supports multiple input devices (keyboard/mouse, gamepad, racing
 * wheel), automatically detecting which device the player is using and adapting
 * the input processing accordingly.
 *
 * @section Architecture
 *
 * Input Flow:
 * ```
 * Input Device (Gamepad/Keyboard/Wheel)
 *     |
 *     v
 * Enhanced Input System (UE5)
 *     |
 *     v
 * UMGVehicleInputHandler
 *     |-- Apply Deadzone
 *     |-- Apply Sensitivity/Linearity
 *     |-- Apply Input Assists (steering, counter-steer, braking)
 *     |-- Apply Speed-Sensitive Steering
 *     v
 * UMGVehicleMovementComponent (receives processed input)
 * ```
 *
 * Key Components:
 * - **Input Actions**: UE5 Enhanced Input actions for each control
 * - **Input State**: Current frame's input values (FMGVehicleInputState)
 * - **Assist Settings**: Configurable assists (FMGInputAssistSettings)
 * - **Sensitivity Settings**: Customizable response curves (FMGInputSensitivity)
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Enhanced Input System**:
 * UE5's new input system uses "Input Actions" (abstract actions like "Accelerate")
 * that are bound to physical controls via "Input Mapping Contexts". This allows
 * easy rebinding and multi-device support.
 *
 * **Deadzone**:
 * A small range around the center of analog sticks where input is ignored.
 * This prevents "drift" from worn controllers. A deadzone of 0.1 means inputs
 * below 10% are treated as zero.
 *
 * **Sensitivity**:
 * How much the output changes relative to input. Sensitivity of 1.5 means
 * moving the stick 50% results in 75% output.
 *
 * **Linearity (Response Curve)**:
 * Controls how input ramps up. Linear (1.0) = straight line. Higher values
 * (like 2.0) give more precision near center with faster response near edges.
 * ```
 * Linearity 1.0:  |----/        Linearity 2.0:  |    __/
 *                 | /                           | _-
 *                 |/___                         |/____
 * ```
 *
 * **Speed-Sensitive Steering**:
 * Reduces steering sensitivity at high speeds to prevent overcorrection.
 * At 200 km/h, full stick might only turn 50% as much as at low speed.
 *
 * **Counter-Steer Assist**:
 * Automatically applies opposite steering input when the car is sliding
 * (oversteer) to help maintain control. Strength is configurable.
 *
 * **Input Mode (EMGVehicleInputMode)**:
 * The handler can operate in different modes:
 * - Racing: Normal driving controls
 * - PhotoMode: Camera controls, vehicle frozen
 * - Menu: Disabled, UI handles input
 * - Garage: Limited controls for customization view
 *
 * **Force Feedback**:
 * Rumble effects sent to controllers/wheels to simulate road feel, collisions,
 * and engine vibration. Intensity is configurable per effect type.
 *
 * @section Usage Example Usage
 *
 * @code
 * // The input handler is typically attached to AMGVehiclePawn
 * // Input binding happens automatically in BeginPlay
 *
 * // Query current input state (useful for HUD):
 * float CurrentThrottle = InputHandler->GetThrottle();
 * float CurrentSteering = InputHandler->GetSteering();
 * float ProcessedSteering = InputHandler->GetProcessedSteering(); // With assists
 *
 * // Check button states:
 * if (InputHandler->IsNOSPressed())
 * {
 *     // Activate NOS boost
 * }
 *
 * // Adjust assists for difficulty:
 * FMGInputAssistSettings NewAssists;
 * NewAssists.bSteeringAssist = true;
 * NewAssists.SteeringAssistStrength = 0.75f;
 * NewAssists.bCounterSteerAssist = true;
 * NewAssists.bAutoShift = true;  // Automatic transmission
 * InputHandler->SetAssistSettings(NewAssists);
 *
 * // Customize sensitivity:
 * FMGInputSensitivity Sensitivity;
 * Sensitivity.SteeringSensitivity = 1.2f;
 * Sensitivity.SteeringLinearity = 2.0f;  // More center precision
 * Sensitivity.SteeringDeadzone = 0.08f;
 * InputHandler->SetSensitivitySettings(Sensitivity);
 *
 * // Trigger force feedback for collision:
 * InputHandler->TriggerForceFeedback(0.8f, 0.3f); // 80% intensity, 0.3 seconds
 *
 * // Switch to photo mode:
 * InputHandler->SetInputMode(EMGVehicleInputMode::PhotoMode);
 *
 * // Listen for events:
 * InputHandler->OnNOSActivated.AddDynamic(this, &AMyClass::HandleNOSActivated);
 * InputHandler->OnGearShift.AddDynamic(this, &AMyClass::HandleGearShift);
 * InputHandler->OnControllerTypeChanged.AddDynamic(this, &AMyClass::OnDeviceChanged);
 * @endcode
 *
 * @section RacingWheel Racing Wheel Support
 * The handler integrates with UMGRacingWheelSubsystem for dedicated wheel hardware:
 * - Full rotation range (900+ degrees)
 * - Pedal input with clutch support
 * - Advanced force feedback (road texture, weight transfer, collisions)
 * - H-pattern shifter support
 *
 * Use IsRacingWheelConnected() to detect wheel presence and adapt UI accordingly.
 *
 * @see UMGVehicleMovementComponent For how input affects vehicle physics
 * @see UMGInputConfig For input action definitions
 * @see UMGRacingWheelSubsystem For racing wheel hardware integration
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "MGVehicleInputHandler.generated.h"

class UInputMappingContext;
class UInputAction;
class AMGVehiclePawn;
class UMGRacingWheelSubsystem;

/**
 * Input mode for vehicle control
 */
UENUM(BlueprintType)
enum class EMGVehicleInputMode : uint8
{
	/** Standard racing controls */
	Racing,
	/** Photo mode controls */
	PhotoMode,
	/** Menu/paused */
	Menu,
	/** Spectating */
	Spectating,
	/** Garage/customization */
	Garage,
	/** Disabled */
	Disabled
};

/**
 * Controller type
 */
UENUM(BlueprintType)
enum class EMGControllerType : uint8
{
	/** Keyboard and mouse */
	KeyboardMouse,
	/** Gamepad (Xbox/PlayStation) */
	Gamepad,
	/** Racing wheel */
	RacingWheel
};

/**
 * Vehicle input state
 */
USTRUCT(BlueprintType)
struct FMGVehicleInputState
{
	GENERATED_BODY()

	/** Throttle input (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float Throttle = 0.0f;

	/** Brake input (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float Brake = 0.0f;

	/** Steering input (-1 to 1) */
	UPROPERTY(BlueprintReadOnly)
	float Steering = 0.0f;

	/** Handbrake input (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float Handbrake = 0.0f;

	/** Clutch input (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float Clutch = 0.0f;

	/** NOS button pressed */
	UPROPERTY(BlueprintReadOnly)
	bool bNOSPressed = false;

	/** Horn button pressed */
	UPROPERTY(BlueprintReadOnly)
	bool bHornPressed = false;

	/** Shift up requested */
	UPROPERTY(BlueprintReadOnly)
	bool bShiftUpRequested = false;

	/** Shift down requested */
	UPROPERTY(BlueprintReadOnly)
	bool bShiftDownRequested = false;

	/** Headlights toggle requested */
	UPROPERTY(BlueprintReadOnly)
	bool bHeadlightsToggle = false;

	/** Look left input */
	UPROPERTY(BlueprintReadOnly)
	float LookLeft = 0.0f;

	/** Look right input */
	UPROPERTY(BlueprintReadOnly)
	float LookRight = 0.0f;

	/** Look back pressed */
	UPROPERTY(BlueprintReadOnly)
	bool bLookBack = false;

	/** Camera change requested */
	UPROPERTY(BlueprintReadOnly)
	bool bCameraChange = false;
};

/**
 * Input assist settings
 */
USTRUCT(BlueprintType)
struct FMGInputAssistSettings
{
	GENERATED_BODY()

	/** Enable steering assist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSteeringAssist = true;

	/** Steering assist strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SteeringAssistStrength = 0.5f;

	/** Enable counter-steer assist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCounterSteerAssist = true;

	/** Counter-steer assist strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CounterSteerStrength = 0.5f;

	/** Enable braking assist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBrakingAssist = false;

	/** Enable throttle assist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bThrottleAssist = false;

	/** Auto-shift enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoShift = true;

	/** Speed-sensitive steering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpeedSensitiveSteering = true;

	/** Speed at which steering sensitivity is minimum (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighSpeedSteeringThreshold = 200.0f;

	/** Minimum steering sensitivity at high speed (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float HighSpeedSteeringSensitivity = 0.5f;
};

/**
 * Sensitivity settings
 */
USTRUCT(BlueprintType)
struct FMGInputSensitivity
{
	GENERATED_BODY()

	/** Steering sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float SteeringSensitivity = 1.0f;

	/** Steering linearity (1 = linear, >1 = more center precision) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "4.0"))
	float SteeringLinearity = 1.5f;

	/** Steering deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float SteeringDeadzone = 0.1f;

	/** Throttle sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float ThrottleSensitivity = 1.0f;

	/** Throttle deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float ThrottleDeadzone = 0.05f;

	/** Brake sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float BrakeSensitivity = 1.0f;

	/** Brake deadzone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float BrakeDeadzone = 0.05f;

	/** Force feedback strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ForceFeedbackStrength = 0.7f;

	/** Vibration strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VibrationStrength = 0.8f;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputModeChanged, EMGVehicleInputMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnControllerTypeChanged, EMGControllerType, NewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNOSActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHornActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGearShift, bool, bShiftUp);

/**
 * Vehicle Input Handler Component
 * Handles all player input for vehicle control per PRD TechnicalDesign.md
 *
 * Features:
 * - Enhanced Input System integration
 * - Multiple controller support (KB/M, Gamepad, Wheel)
 * - Input assists (steering, counter-steer, braking)
 * - Sensitivity and deadzone settings
 * - Speed-sensitive steering
 * - Force feedback support
 * - Mode switching (Racing, Photo, Menu)
 */
UCLASS(ClassGroup=(MidnightGrind), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGVehicleInputHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGVehicleInputHandler();

	virtual void BeginPlay() override;
	virtual void TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// INPUT MODE
	// ==========================================

	/**
	 * Set input mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Mode")
	void SetInputMode(EMGVehicleInputMode NewMode);

	/**
	 * Get current input mode
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Mode")
	EMGVehicleInputMode GetInputMode() const { return CurrentInputMode; }

	/**
	 * Get detected controller type
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Mode")
	EMGControllerType GetControllerType() const { return DetectedControllerType; }

	/**
	 * Check if a racing wheel is connected
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Mode")
	bool IsRacingWheelConnected() const;

	/**
	 * Get the racing wheel subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Mode")
	UMGRacingWheelSubsystem* GetRacingWheelSubsystem() const { return RacingWheelSubsystem; }

	// ==========================================
	// INPUT STATE
	// ==========================================

	/**
	 * Get current input state
	 */
	UFUNCTION(BlueprintPure, Category = "Input|State")
	FMGVehicleInputState GetInputState() const { return CurrentInputState; }

	/**
	 * Get raw throttle input
	 */
	UFUNCTION(BlueprintPure, Category = "Input|State")
	float GetThrottle() const { return CurrentInputState.Throttle; }

	/**
	 * Get raw brake input
	 */
	UFUNCTION(BlueprintPure, Category = "Input|State")
	float GetBrake() const { return CurrentInputState.Brake; }

	/**
	 * Get raw steering input
	 */
	UFUNCTION(BlueprintPure, Category = "Input|State")
	float GetSteering() const { return CurrentInputState.Steering; }

	/**
	 * Get processed steering (with assists applied)
	 */
	UFUNCTION(BlueprintPure, Category = "Input|State")
	float GetProcessedSteering() const { return ProcessedSteering; }

	/**
	 * Is NOS button pressed
	 */
	UFUNCTION(BlueprintPure, Category = "Input|State")
	bool IsNOSPressed() const { return CurrentInputState.bNOSPressed; }

	/**
	 * Is handbrake pressed
	 */
	UFUNCTION(BlueprintPure, Category = "Input|State")
	bool IsHandbrakePressed() const { return CurrentInputState.Handbrake > 0.5f; }

	// ==========================================
	// SETTINGS
	// ==========================================

	/**
	 * Set assist settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Settings")
	void SetAssistSettings(const FMGInputAssistSettings& Settings);

	/**
	 * Get assist settings
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Settings")
	FMGInputAssistSettings GetAssistSettings() const { return AssistSettings; }

	/**
	 * Set sensitivity settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Settings")
	void SetSensitivitySettings(const FMGInputSensitivity& Settings);

	/**
	 * Get sensitivity settings
	 */
	UFUNCTION(BlueprintPure, Category = "Input|Settings")
	FMGInputSensitivity GetSensitivitySettings() const { return SensitivitySettings; }

	// ==========================================
	// FORCE FEEDBACK
	// ==========================================

	/**
	 * Trigger force feedback effect
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Feedback")
	void TriggerForceFeedback(float Intensity, float Duration);

	/**
	 * Set continuous force feedback
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Feedback")
	void SetContinuousForceFeedback(float LeftIntensity, float RightIntensity);

	/**
	 * Stop all force feedback
	 */
	UFUNCTION(BlueprintCallable, Category = "Input|Feedback")
	void StopForceFeedback();

	// ==========================================
	// EVENTS
	// ==========================================

	/** Input mode changed */
	UPROPERTY(BlueprintAssignable, Category = "Input|Events")
	FOnInputModeChanged OnInputModeChanged;

	/** Controller type changed */
	UPROPERTY(BlueprintAssignable, Category = "Input|Events")
	FOnControllerTypeChanged OnControllerTypeChanged;

	/** NOS activated */
	UPROPERTY(BlueprintAssignable, Category = "Input|Events")
	FOnNOSActivated OnNOSActivated;

	/** Horn activated */
	UPROPERTY(BlueprintAssignable, Category = "Input|Events")
	FOnHornActivated OnHornActivated;

	/** Gear shift requested */
	UPROPERTY(BlueprintAssignable, Category = "Input|Events")
	FOnGearShift OnGearShift;

	// ==========================================
	// INPUT ACTIONS
	// ==========================================

	/** Throttle input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ThrottleAction;

	/** Brake input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> BrakeAction;

	/** Steering input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> SteeringAction;

	/** Handbrake input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> HandbrakeAction;

	/** NOS input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> NOSAction;

	/** Shift up input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ShiftUpAction;

	/** Shift down input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ShiftDownAction;

	/** Horn input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> HornAction;

	/** Camera change input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> CameraChangeAction;

	/** Look back input action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> LookBackAction;

protected:
	// ==========================================
	// INPUT HANDLERS
	// ==========================================

	void OnThrottleInput(const FInputActionValue& Value);
	void OnBrakeInput(const FInputActionValue& Value);
	void OnSteeringInput(const FInputActionValue& Value);
	void OnHandbrakeInput(const FInputActionValue& Value);
	void OnNOSInput(const FInputActionValue& Value);
	void OnShiftUpInput(const FInputActionValue& Value);
	void OnShiftDownInput(const FInputActionValue& Value);
	void OnHornInput(const FInputActionValue& Value);
	void OnCameraChangeInput(const FInputActionValue& Value);
	void OnLookBackInput(const FInputActionValue& Value);

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Apply sensitivity and deadzone */
	float ApplySensitivity(float RawInput, float Sensitivity, float Deadzone, float Linearity = 1.0f) const;

	/** Process steering with assists */
	void ProcessSteering(float MGDeltaTime);

	/** Apply speed-sensitive steering */
	float ApplySpeedSensitiveSteering(float Input) const;

	/** Apply counter-steer assist */
	float ApplyCounterSteerAssist(float Input) const;

	/** Update force feedback based on vehicle state */
	void UpdateForceFeedback();

	/** Apply inputs to vehicle */
	void ApplyInputsToVehicle();

	/** Detect controller type */
	void DetectControllerType();

private:
	/** Current input mode */
	EMGVehicleInputMode CurrentInputMode = EMGVehicleInputMode::Racing;

	/** Detected controller type */
	EMGControllerType DetectedControllerType = EMGControllerType::Gamepad;

	/** Current input state */
	FMGVehicleInputState CurrentInputState;

	/** Assist settings */
	UPROPERTY(EditAnywhere, Category = "Settings")
	FMGInputAssistSettings AssistSettings;

	/** Sensitivity settings */
	UPROPERTY(EditAnywhere, Category = "Settings")
	FMGInputSensitivity SensitivitySettings;

	/** Processed steering value */
	float ProcessedSteering = 0.0f;

	/** Previous steering for smoothing */
	float PreviousSteering = 0.0f;

	/** Owned vehicle reference */
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> OwnedVehicle;

	/** Force feedback active */
	bool bForceFeedbackActive = false;

	/** Force feedback timer */
	float ForceFeedbackTimer = 0.0f;

	/** Racing wheel subsystem reference */
	UPROPERTY()
	TObjectPtr<UMGRacingWheelSubsystem> RacingWheelSubsystem;

	/** Cache wheel subsystem */
	void CacheRacingWheelSubsystem();

	/** Process input from racing wheel if connected */
	void ProcessRacingWheelInput();

	/** Route FFB to wheel subsystem when connected */
	void RouteFFBToWheel(float LeftIntensity, float RightIntensity);
};

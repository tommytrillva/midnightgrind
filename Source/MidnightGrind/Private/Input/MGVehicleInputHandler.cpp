// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehicleInputHandler.cpp
 * @brief Implementation of vehicle input processing with assists and force feedback.
 *
 * @see MGVehicleInputHandler.h for full documentation
 */

#include "Input/MGVehicleInputHandler.h"
#include "Vehicle/MGVehiclePawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "RacingWheel/MGRacingWheelSubsystem.h"
#include "Kismet/GameplayStatics.h"

UMGVehicleInputHandler::UMGVehicleInputHandler()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UMGVehicleInputHandler::BeginPlay()
{
	Super::BeginPlay();

	// Get owner vehicle
	OwnedVehicle = Cast<AMGVehiclePawn>(GetOwner());

	// Setup input bindings
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PC->InputComponent))
			{
				// Bind input actions
				if (ThrottleAction)
				{
					EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &UMGVehicleInputHandler::OnThrottleInput);
					EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &UMGVehicleInputHandler::OnThrottleInput);
				}

				if (BrakeAction)
				{
					EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &UMGVehicleInputHandler::OnBrakeInput);
					EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Completed, this, &UMGVehicleInputHandler::OnBrakeInput);
				}

				if (SteeringAction)
				{
					EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &UMGVehicleInputHandler::OnSteeringInput);
					EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Completed, this, &UMGVehicleInputHandler::OnSteeringInput);
				}

				if (HandbrakeAction)
				{
					EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &UMGVehicleInputHandler::OnHandbrakeInput);
					EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &UMGVehicleInputHandler::OnHandbrakeInput);
				}

				if (NOSAction)
				{
					EnhancedInput->BindAction(NOSAction, ETriggerEvent::Triggered, this, &UMGVehicleInputHandler::OnNOSInput);
					EnhancedInput->BindAction(NOSAction, ETriggerEvent::Completed, this, &UMGVehicleInputHandler::OnNOSInput);
				}

				if (ShiftUpAction)
				{
					EnhancedInput->BindAction(ShiftUpAction, ETriggerEvent::Started, this, &UMGVehicleInputHandler::OnShiftUpInput);
				}

				if (ShiftDownAction)
				{
					EnhancedInput->BindAction(ShiftDownAction, ETriggerEvent::Started, this, &UMGVehicleInputHandler::OnShiftDownInput);
				}

				if (HornAction)
				{
					EnhancedInput->BindAction(HornAction, ETriggerEvent::Triggered, this, &UMGVehicleInputHandler::OnHornInput);
					EnhancedInput->BindAction(HornAction, ETriggerEvent::Completed, this, &UMGVehicleInputHandler::OnHornInput);
				}

				if (CameraChangeAction)
				{
					EnhancedInput->BindAction(CameraChangeAction, ETriggerEvent::Started, this, &UMGVehicleInputHandler::OnCameraChangeInput);
				}

				if (LookBackAction)
				{
					EnhancedInput->BindAction(LookBackAction, ETriggerEvent::Triggered, this, &UMGVehicleInputHandler::OnLookBackInput);
					EnhancedInput->BindAction(LookBackAction, ETriggerEvent::Completed, this, &UMGVehicleInputHandler::OnLookBackInput);
				}
			}
		}
	}

	// Detect initial controller type
	DetectControllerType();

	// Cache racing wheel subsystem
	CacheRacingWheelSubsystem();
}

void UMGVehicleInputHandler::TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentInputMode == EMGVehicleInputMode::Disabled || CurrentInputMode == EMGVehicleInputMode::Menu)
	{
		return;
	}

	// Check for racing wheel input first
	if (IsRacingWheelConnected())
	{
		ProcessRacingWheelInput();
	}

	// Process steering with assists
	ProcessSteering(DeltaTime);

	// Update force feedback
	UpdateForceFeedback();

	// Apply inputs to vehicle
	ApplyInputsToVehicle();

	// Clear one-shot inputs
	CurrentInputState.bShiftUpRequested = false;
	CurrentInputState.bShiftDownRequested = false;
	CurrentInputState.bHeadlightsToggle = false;
	CurrentInputState.bCameraChange = false;

	// Update force feedback timer
	if (bForceFeedbackActive)
	{
		ForceFeedbackTimer -= DeltaTime;
		if (ForceFeedbackTimer <= 0.0f)
		{
			StopForceFeedback();
		}
	}
}

void UMGVehicleInputHandler::SetInputMode(EMGVehicleInputMode NewMode)
{
	if (CurrentInputMode != NewMode)
	{
		CurrentInputMode = NewMode;

		// Reset input state on mode change
		if (NewMode == EMGVehicleInputMode::Disabled || NewMode == EMGVehicleInputMode::Menu)
		{
			CurrentInputState = FMGVehicleInputState();
			ProcessedSteering = 0.0f;
		}

		OnInputModeChanged.Broadcast(NewMode);
	}
}

void UMGVehicleInputHandler::SetAssistSettings(const FMGInputAssistSettings& Settings)
{
	AssistSettings = Settings;
}

void UMGVehicleInputHandler::SetSensitivitySettings(const FMGInputSensitivity& Settings)
{
	SensitivitySettings = Settings;
}

void UMGVehicleInputHandler::TriggerForceFeedback(float Intensity, float Duration)
{
	if (SensitivitySettings.ForceFeedbackStrength <= 0.0f)
	{
		return;
	}

	float ScaledIntensity = Intensity * SensitivitySettings.ForceFeedbackStrength;

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			PC->PlayDynamicForceFeedback(ScaledIntensity, Duration, true, true, true, true);
		}
	}

	bForceFeedbackActive = true;
	ForceFeedbackTimer = Duration;
}

void UMGVehicleInputHandler::SetContinuousForceFeedback(float LeftIntensity, float RightIntensity)
{
	if (SensitivitySettings.ForceFeedbackStrength <= 0.0f)
	{
		return;
	}

	float ScaledLeft = LeftIntensity * SensitivitySettings.ForceFeedbackStrength;
	float ScaledRight = RightIntensity * SensitivitySettings.ForceFeedbackStrength;

	// Route to racing wheel if connected
	if (IsRacingWheelConnected())
	{
		RouteFFBToWheel(ScaledLeft, ScaledRight);
		bForceFeedbackActive = true;
		return;
	}

	// Fall back to gamepad vibration
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			PC->PlayDynamicForceFeedback(ScaledLeft, -1.0f, true, false, false, false);
			PC->PlayDynamicForceFeedback(ScaledRight, -1.0f, false, true, false, false);
		}
	}

	bForceFeedbackActive = true;
}

void UMGVehicleInputHandler::StopForceFeedback()
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
		{
			PC->StopForceFeedback(nullptr, NAME_None);
		}
	}

	bForceFeedbackActive = false;
	ForceFeedbackTimer = 0.0f;
}

// ==========================================
// INPUT HANDLERS
// ==========================================

void UMGVehicleInputHandler::OnThrottleInput(const FInputActionValue& Value)
{
	float RawInput = Value.Get<float>();
	CurrentInputState.Throttle = ApplySensitivity(RawInput, SensitivitySettings.ThrottleSensitivity, SensitivitySettings.ThrottleDeadzone);
}

void UMGVehicleInputHandler::OnBrakeInput(const FInputActionValue& Value)
{
	float RawInput = Value.Get<float>();
	CurrentInputState.Brake = ApplySensitivity(RawInput, SensitivitySettings.BrakeSensitivity, SensitivitySettings.BrakeDeadzone);
}

void UMGVehicleInputHandler::OnSteeringInput(const FInputActionValue& Value)
{
	float RawInput = Value.Get<float>();
	CurrentInputState.Steering = ApplySensitivity(RawInput, SensitivitySettings.SteeringSensitivity, SensitivitySettings.SteeringDeadzone, SensitivitySettings.SteeringLinearity);
}

void UMGVehicleInputHandler::OnHandbrakeInput(const FInputActionValue& Value)
{
	CurrentInputState.Handbrake = Value.Get<float>();
}

void UMGVehicleInputHandler::OnNOSInput(const FInputActionValue& Value)
{
	bool bWasPressed = CurrentInputState.bNOSPressed;
	CurrentInputState.bNOSPressed = Value.Get<bool>();

	if (CurrentInputState.bNOSPressed && !bWasPressed)
	{
		OnNOSActivated.Broadcast();
	}
}

void UMGVehicleInputHandler::OnShiftUpInput(const FInputActionValue& Value)
{
	if (!AssistSettings.bAutoShift)
	{
		CurrentInputState.bShiftUpRequested = true;
		OnGearShift.Broadcast(true);
	}
}

void UMGVehicleInputHandler::OnShiftDownInput(const FInputActionValue& Value)
{
	if (!AssistSettings.bAutoShift)
	{
		CurrentInputState.bShiftDownRequested = true;
		OnGearShift.Broadcast(false);
	}
}

void UMGVehicleInputHandler::OnHornInput(const FInputActionValue& Value)
{
	bool bWasPressed = CurrentInputState.bHornPressed;
	CurrentInputState.bHornPressed = Value.Get<bool>();

	if (CurrentInputState.bHornPressed && !bWasPressed)
	{
		OnHornActivated.Broadcast();
	}
}

void UMGVehicleInputHandler::OnCameraChangeInput(const FInputActionValue& Value)
{
	CurrentInputState.bCameraChange = true;
}

void UMGVehicleInputHandler::OnLookBackInput(const FInputActionValue& Value)
{
	CurrentInputState.bLookBack = Value.Get<bool>();
}

// ==========================================
// INTERNAL
// ==========================================

float UMGVehicleInputHandler::ApplySensitivity(float RawInput, float Sensitivity, float Deadzone, float Linearity) const
{
	// Apply deadzone
	float AbsInput = FMath::Abs(RawInput);
	if (AbsInput < Deadzone)
	{
		return 0.0f;
	}

	// Remap from deadzone to 1.0
	float Sign = FMath::Sign(RawInput);
	float RemappedInput = (AbsInput - Deadzone) / (1.0f - Deadzone);

	// Apply linearity (response curve)
	float CurvedInput = FMath::Pow(RemappedInput, Linearity);

	// Apply sensitivity
	float FinalInput = CurvedInput * Sensitivity;

	return FMath::Clamp(FinalInput * Sign, -1.0f, 1.0f);
}

void UMGVehicleInputHandler::ProcessSteering(float MGDeltaTime)
{
	float RawSteering = CurrentInputState.Steering;

	// Apply speed-sensitive steering
	if (AssistSettings.bSpeedSensitiveSteering)
	{
		RawSteering = ApplySpeedSensitiveSteering(RawSteering);
	}

	// Apply counter-steer assist
	if (AssistSettings.bCounterSteerAssist)
	{
		RawSteering = ApplyCounterSteerAssist(RawSteering);
	}

	// Smooth steering for better feel
	float SteeringSmoothSpeed = 10.0f;
	ProcessedSteering = FMath::FInterpTo(PreviousSteering, RawSteering, DeltaTime, SteeringSmoothSpeed);
	PreviousSteering = ProcessedSteering;
}

float UMGVehicleInputHandler::ApplySpeedSensitiveSteering(float Input) const
{
	if (!OwnedVehicle.IsValid())
	{
		return Input;
	}

	// Get current speed in km/h
	float SpeedKmh = OwnedVehicle->GetVelocity().Size() * 0.036f; // cm/s to km/h

	// Calculate sensitivity multiplier based on speed
	float SpeedRatio = FMath::Clamp(SpeedKmh / AssistSettings.HighSpeedSteeringThreshold, 0.0f, 1.0f);
	float SensitivityMultiplier = FMath::Lerp(1.0f, AssistSettings.HighSpeedSteeringSensitivity, SpeedRatio);

	return Input * SensitivityMultiplier;
}

float UMGVehicleInputHandler::ApplyCounterSteerAssist(float Input) const
{
	if (!OwnedVehicle.IsValid())
	{
		return Input;
	}

	// Get vehicle's angular velocity (yaw rate)
	FVector AngularVelocity = OwnedVehicle->GetPhysicsAngularVelocityInDegrees();
	float YawRate = AngularVelocity.Z;

	// Get vehicle's forward velocity
	FVector Velocity = OwnedVehicle->GetVelocity();
	FVector Forward = OwnedVehicle->GetActorForwardVector();
	float ForwardSpeed = FVector::DotProduct(Velocity, Forward);

	// Only apply counter-steer assist when moving forward and spinning
	if (ForwardSpeed < 1000.0f) // ~36 km/h threshold
	{
		return Input;
	}

	// Calculate slip angle indicator from yaw rate
	float SlipIndicator = FMath::Clamp(YawRate / 90.0f, -1.0f, 1.0f);

	// Blend in counter-steer based on strength setting
	float CounterSteerAmount = -SlipIndicator * AssistSettings.CounterSteerStrength;

	// Only add counter-steer, don't override player input
	return FMath::Clamp(Input + CounterSteerAmount * 0.5f, -1.0f, 1.0f);
}

void UMGVehicleInputHandler::UpdateForceFeedback()
{
	if (SensitivitySettings.VibrationStrength <= 0.0f)
	{
		return;
	}

	if (!OwnedVehicle.IsValid())
	{
		return;
	}

	// Would implement force feedback based on:
	// - Road surface
	// - Tire slip
	// - Impacts
	// - Engine vibration at redline
	// For now, this is a placeholder for vehicle integration
}

void UMGVehicleInputHandler::ApplyInputsToVehicle()
{
	if (!OwnedVehicle.IsValid())
	{
		return;
	}

	// Apply throttle assist if enabled
	float FinalThrottle = CurrentInputState.Throttle;
	if (AssistSettings.bThrottleAssist)
	{
		// Reduce throttle when wheels are spinning excessively
		// Implementation would check wheel slip ratios
		FinalThrottle = CurrentInputState.Throttle;
	}

	// Apply brake assist if enabled
	float FinalBrake = CurrentInputState.Brake;
	if (AssistSettings.bBrakingAssist)
	{
		// ABS-like behavior - pulse brakes when wheels lock
		// Implementation would check wheel lock state
		FinalBrake = CurrentInputState.Brake;
	}

	// Apply inputs to vehicle pawn
	// The vehicle pawn should have methods to receive these values
	// OwnedVehicle->SetThrottleInput(FinalThrottle);
	// OwnedVehicle->SetBrakeInput(FinalBrake);
	// OwnedVehicle->SetSteeringInput(ProcessedSteering);
	// OwnedVehicle->SetHandbrakeInput(CurrentInputState.Handbrake);
	// etc.
}

void UMGVehicleInputHandler::DetectControllerType()
{
	// Check for racing wheel first
	if (RacingWheelSubsystem && RacingWheelSubsystem->IsWheelConnected())
	{
		if (DetectedControllerType != EMGControllerType::RacingWheel)
		{
			DetectedControllerType = EMGControllerType::RacingWheel;
			OnControllerTypeChanged.Broadcast(DetectedControllerType);
		}
		return;
	}

	// Default to gamepad
	EMGControllerType NewType = EMGControllerType::Gamepad;

	if (DetectedControllerType != NewType)
	{
		DetectedControllerType = NewType;
		OnControllerTypeChanged.Broadcast(NewType);
	}
}

bool UMGVehicleInputHandler::IsRacingWheelConnected() const
{
	return RacingWheelSubsystem && RacingWheelSubsystem->IsWheelConnected();
}

void UMGVehicleInputHandler::CacheRacingWheelSubsystem()
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		RacingWheelSubsystem = GameInstance->GetSubsystem<UMGRacingWheelSubsystem>();
	}
}

void UMGVehicleInputHandler::ProcessRacingWheelInput()
{
	if (!RacingWheelSubsystem || !RacingWheelSubsystem->IsWheelConnected())
	{
		return;
	}

	// Get input directly from wheel subsystem
	CurrentInputState.Steering = RacingWheelSubsystem->GetSteeringInput();
	CurrentInputState.Throttle = RacingWheelSubsystem->GetThrottleInput();
	CurrentInputState.Brake = RacingWheelSubsystem->GetBrakeInput();
	CurrentInputState.Clutch = RacingWheelSubsystem->GetClutchInput();

	// Handle paddle shifter events
	if (RacingWheelSubsystem->WasShiftUpPressed() && !AssistSettings.bAutoShift)
	{
		CurrentInputState.bShiftUpRequested = true;
		OnGearShift.Broadcast(true);
	}
	if (RacingWheelSubsystem->WasShiftDownPressed() && !AssistSettings.bAutoShift)
	{
		CurrentInputState.bShiftDownRequested = true;
		OnGearShift.Broadcast(false);
	}
}

void UMGVehicleInputHandler::RouteFFBToWheel(float LeftIntensity, float RightIntensity)
{
	if (!RacingWheelSubsystem || !RacingWheelSubsystem->IsWheelConnected())
	{
		return;
	}

	// Convert stereo gamepad vibration to wheel FFB
	// For wheels, we typically want directional force rather than vibration
	// Use the difference between left and right for direction
	float Direction = RightIntensity - LeftIntensity;
	float Magnitude = (LeftIntensity + RightIntensity) * 0.5f;

	if (FMath::Abs(Magnitude) > 0.01f)
	{
		FMGFFBEffect Effect;
		Effect.EffectType = EMGFFBEffectType::ConstantForce;
		Effect.Magnitude = FMath::Clamp(Direction, -1.0f, 1.0f) * Magnitude;
		Effect.Duration = 0.1f; // Short pulse

		RacingWheelSubsystem->PlayFFBEffect(Effect);
	}
}

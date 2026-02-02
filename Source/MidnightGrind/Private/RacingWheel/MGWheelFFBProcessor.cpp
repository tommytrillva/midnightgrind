// Copyright Midnight Grind. All Rights Reserved.

#include "RacingWheel/MGWheelFFBProcessor.h"
#include "RacingWheel/MGRacingWheelSubsystem.h"

UMGWheelFFBProcessor::UMGWheelFFBProcessor()
{
}

void UMGWheelFFBProcessor::Initialize(UMGRacingWheelSubsystem* InWheelSubsystem)
{
	WheelSubsystem = InWheelSubsystem;
	Reset();
}

void UMGWheelFFBProcessor::Tick(float MGDeltaTime)
{
	TimeSinceLastUpdate += DeltaTime;
	LastDeltaTime = DeltaTime;
}

void UMGWheelFFBProcessor::ProcessVehicleData(const FMGFFBInputData& VehicleData, const FMGWheelProfile& Profile)
{
	if (!WheelSubsystem.IsValid())
	{
		return;
	}

	float MGDeltaTime = FMath::Max(TimeSinceLastUpdate, 0.001f);
	TimeSinceLastUpdate = 0.0f;

	// Track drift state
	const bool bIsDriftingNow = FMath::Abs(VehicleData.DriftAngle) > DriftAngleThreshold;
	if (bIsDriftingNow && !bWasDrifting)
	{
		// Just started drifting
		DriftEntryTime = 0.0f;
		PeakDriftAngle = FMath::Abs(VehicleData.DriftAngle);
	}
	if (bIsDriftingNow)
	{
		CurrentDriftDuration += DeltaTime;
		PeakDriftAngle = FMath::Max(PeakDriftAngle, FMath::Abs(VehicleData.DriftAngle));
	}
	else
	{
		CurrentDriftDuration = 0.0f;
		PeakDriftAngle = 0.0f;
	}
	bWasDrifting = bIsDriftingNow;

	// Check airborne state
	bIsAirborne = VehicleData.bIsAirborne;

	// === CALCULATE TARGET FORCES ===
	// When airborne, smoothly reduce all forces to zero

	if (bIsAirborne)
	{
		TargetSelfCentering = 0.0f;
		TargetAligningTorque = 0.0f;
		TargetGripFeedback = 0.0f;
		TargetDriftFeedback = 0.0f;
		TargetWeightTransfer = 0.0f;
		TargetGForce = 0.0f;
	}
	else
	{
		// Calculate each force component
		TargetSelfCentering = CalculateSelfCenteringForce(VehicleData, Profile);
		TargetAligningTorque = CalculateAligningTorque(VehicleData, Profile);
		TargetGripFeedback = CalculateGripFeedback(VehicleData, Profile);
		TargetDriftFeedback = CalculateDriftFeedback(VehicleData, Profile);
		TargetWeightTransfer = CalculateWeightTransferFeedback(VehicleData, Profile);
		TargetGForce = CalculateGForceFeedback(VehicleData, Profile);
	}

	// === SMOOTH FORCE TRANSITIONS ===
	// Different smoothing times for different effects create a layered, natural feel

	CurrentSelfCentering = SmoothForce(CurrentSelfCentering, TargetSelfCentering, SelfCenteringSmoothTime, DeltaTime);
	CurrentAligningTorque = SmoothForce(CurrentAligningTorque, TargetAligningTorque, AligningTorqueSmoothTime, DeltaTime);
	CurrentGripFeedback = SmoothForce(CurrentGripFeedback, TargetGripFeedback, GripFeedbackSmoothTime, DeltaTime);
	CurrentDriftFeedback = SmoothForce(CurrentDriftFeedback, TargetDriftFeedback, DriftFeedbackSmoothTime, DeltaTime);
	CurrentWeightTransfer = SmoothForce(CurrentWeightTransfer, TargetWeightTransfer, WeightTransferSmoothTime, DeltaTime);
	CurrentGForce = SmoothForce(CurrentGForce, TargetGForce, GForceSmoothTime, DeltaTime);

	// === UPDATE PERIODIC EFFECTS ===
	UpdateKerbEffect(VehicleData, Profile);
	UpdateSurfaceEffect(VehicleData, Profile);
	UpdateEngineEffect(VehicleData, Profile);
	UpdateCollisionEffect(VehicleData, Profile);

	// === APPLY TO WHEEL ===
	ApplyEffects(Profile);

	// Store for next frame
	LastVehicleData = VehicleData;
}

// ============================================================================
// SELF-CENTERING FORCE
// The spring that pulls the wheel back to center. Critical for feeling speed
// and grip level. Reduces when grip is lost!
// ============================================================================
float UMGWheelFFBProcessor::CalculateSelfCenteringForce(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	// No centering at very low speeds (feels unnatural when parking)
	if (Data.SpeedKmh < MinSpeedForCentering)
	{
		return 0.0f;
	}

	// Speed factor: centering increases with speed (0 to 1)
	float SpeedFactor = FMath::Clamp((Data.SpeedKmh - MinSpeedForCentering) / (MaxCenteringSpeed - MinSpeedForCentering), 0.0f, 1.0f);

	// Apply a curve so centering builds more gradually at low speed
	SpeedFactor = FMath::Pow(SpeedFactor, 0.7f);

	// Calculate front tire grip (0 = no grip, 1 = full grip)
	// This is THE critical feedback - when you're losing grip, centering reduces
	float FrontGrip = CalculateTireGripFromSlip(Data.FrontSlipAngle);

	// Base centering strength, modified by speed and grip
	float CenteringStrength = BaseCenteringStrength * SpeedFactor * FrontGrip;

	// Apply profile setting
	CenteringStrength *= Profile.SelfCenteringStrength;

	return FMath::Clamp(CenteringStrength, 0.0f, 1.0f);
}

// ============================================================================
// ALIGNING TORQUE (Self-Aligning Torque / SAT)
// This is what makes steering feel "alive". It's the force created by the tire's
// contact patch wanting to align with the direction of travel.
// - Increases with slip angle up to a point
// - Then DECREASES as the tire starts sliding (the wheel goes "light")
// - This is your primary grip indicator!
// ============================================================================
float UMGWheelFFBProcessor::CalculateAligningTorque(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	// Get front slip angle (use average if we have both wheels)
	float SlipAngle = FMath::Abs(Data.FrontSlipAngle);
	float SlipSign = FMath::Sign(Data.FrontSlipAngle);

	// Normalize slip angle to our scale
	float NormalizedSlip = SlipAngle / MaxSlipAngle;

	// The magic: SAT curve
	// - Linear increase up to optimal slip angle
	// - Peak at optimal slip
	// - Decrease as tire slides (goes light!)
	float SAT = 0.0f;

	if (SlipAngle <= OptimalSlipAngle)
	{
		// Building up to peak - linear increase
		float T = SlipAngle / OptimalSlipAngle;
		SAT = PeakAligningTorque * T;
	}
	else if (SlipAngle <= MaxSlipAngle)
	{
		// Past peak, dropping off - tire is sliding
		// This is the "lightening" that tells you grip is being lost
		float T = (SlipAngle - OptimalSlipAngle) / (MaxSlipAngle - OptimalSlipAngle);
		SAT = FMath::Lerp(PeakAligningTorque, SlidingAligningTorque, T);
	}
	else
	{
		// Full slide - minimal feedback
		SAT = SlidingAligningTorque;
	}

	// Modify by tire load (more load = more feedback)
	SAT *= FMath::Clamp(Data.FrontTireLoad, 0.3f, 1.5f);

	// Speed scaling - more feedback at higher speeds
	float SpeedScale = FMath::Clamp(Data.SpeedKmh / 100.0f, 0.2f, 1.0f);
	SAT *= SpeedScale;

	// Apply profile strength
	SAT *= Profile.RoadFeelStrength;

	// Apply direction (force is opposite to slip)
	return FMath::Clamp(-SAT * SlipSign, -1.0f, 1.0f);
}

// ============================================================================
// GRIP FEEDBACK
// Additional feedback that modifies how "heavy" the wheel feels based on
// available grip. When understeering, the wheel should feel lighter.
// ============================================================================
float UMGWheelFFBProcessor::CalculateGripFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	// Calculate grip loss (0 = full grip, 1 = no grip)
	float FrontSlip = FMath::Abs(Data.FrontSlipAngle);

	float GripLoss = 0.0f;
	if (FrontSlip > GripLossStartAngle)
	{
		GripLoss = FMath::Clamp((FrontSlip - GripLossStartAngle) / (GripLossFullAngle - GripLossStartAngle), 0.0f, 1.0f);
	}

	// When understeering, the feedback should reduce (wheel goes light)
	// This is returned as a negative modifier to overall force
	float UndersteerModifier = 0.0f;
	if (Data.bIsUndersteering)
	{
		UndersteerModifier = GripLoss * Profile.UndersteerStrength;
	}

	return -UndersteerModifier;
}

// ============================================================================
// DRIFT FEEDBACK
// Counter-steer force that helps players catch and maintain drifts.
// When the rear is sliding, the wheel pushes in the counter-steer direction.
// This teaches correct drift technique through feel!
// ============================================================================
float UMGWheelFFBProcessor::CalculateDriftFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	if (!Data.bIsDrifting && !Data.bIsOversteering)
	{
		return 0.0f;
	}

	float DriftAngle = Data.DriftAngle;
	float AbsDriftAngle = FMath::Abs(DriftAngle);

	// No feedback below threshold
	if (AbsDriftAngle < DriftAngleThreshold)
	{
		return 0.0f;
	}

	// Calculate counter-steer force
	// - Starts at base force when drift begins
	// - Increases up to max as drift angle increases
	// - Direction is opposite to drift angle (helps player counter-steer)

	float DriftMagnitude = FMath::Clamp(
		(AbsDriftAngle - DriftAngleThreshold) / (MaxDriftAngleForFeedback - DriftAngleThreshold),
		0.0f, 1.0f
	);

	// Apply a curve so small drifts feel controllable, big drifts feel powerful
	DriftMagnitude = FMath::Pow(DriftMagnitude, 0.8f);

	float CounterForce = FMath::Lerp(DriftCounterForceBase, DriftCounterForceMax, DriftMagnitude);

	// Add bonus force for sustained drifts (rewards holding the drift)
	if (CurrentDriftDuration > 0.5f)
	{
		float DurationBonus = FMath::Clamp((CurrentDriftDuration - 0.5f) / 2.0f, 0.0f, 0.15f);
		CounterForce += DurationBonus;
	}

	// Apply profile strength
	CounterForce *= Profile.OversteerStrength;

	// Direction: positive drift angle (sliding right) needs negative force (counter-steer left)
	float Direction = -FMath::Sign(DriftAngle);

	return FMath::Clamp(CounterForce * Direction, -1.0f, 1.0f);
}

// ============================================================================
// WEIGHT TRANSFER FEEDBACK
// Feel the car's weight shifting as you brake, accelerate, and corner.
// This adds depth to the driving feel.
// ============================================================================
float UMGWheelFFBProcessor::CalculateWeightTransferFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	// Longitudinal weight transfer (braking/acceleration)
	// Braking = weight forward = heavier steering
	// Acceleration = weight rearward = lighter steering
	float LongG = FMath::Clamp(Data.LongitudinalG, -MaxLongitudinalGForFeedback, MaxLongitudinalGForFeedback);
	float LongTransfer = -LongG / MaxLongitudinalGForFeedback * 0.15f; // Subtle effect

	// Lateral weight transfer (cornering)
	// Weight transfers to outside tires, which changes steering feel slightly
	float LatG = FMath::Clamp(Data.LateralG, -MaxLateralGForFeedback, MaxLateralGForFeedback);
	float LatTransfer = LatG / MaxLateralGForFeedback * 0.1f; // Very subtle

	return FMath::Clamp(LongTransfer + LatTransfer, -0.3f, 0.3f);
}

// ============================================================================
// G-FORCE FEEDBACK
// Direct feel of lateral and longitudinal G-forces through the wheel.
// Creates that "seat of the pants" feeling.
// ============================================================================
float UMGWheelFFBProcessor::CalculateGForceFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	// Lateral G creates a force that resists the turn
	// This simulates the feeling of fighting the car through corners
	float LatG = Data.LateralG;
	float LatGForce = FMath::Clamp(LatG / MaxLateralGForFeedback, -1.0f, 1.0f) * 0.2f;

	return LatGForce;
}

// ============================================================================
// KERB EFFECT
// Sharp, aggressive rumble when hitting kerbs/rumble strips.
// Higher frequency than surface texture for clear distinction.
// ============================================================================
void UMGWheelFFBProcessor::UpdateKerbEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	bool bOnKerbNow = Data.bOnRumbleStrip;

	if (bOnKerbNow && !bWasOnKerb)
	{
		// Start kerb effect - aggressive square wave
		if (WheelSubsystem.IsValid())
		{
			float Intensity = FMath::Clamp(Data.SpeedKmh / 150.0f, 0.4f, 1.0f);
			KerbEffectID = WheelSubsystem->TriggerKerbFFB(Intensity * Profile.CurbStrength, -1.0f);
		}
	}
	else if (!bOnKerbNow && bWasOnKerb)
	{
		// Stop kerb effect
		if (WheelSubsystem.IsValid() && KerbEffectID.IsValid())
		{
			WheelSubsystem->StopFFBEffect(KerbEffectID);
			KerbEffectID.Invalidate();
		}
	}

	bWasOnKerb = bOnKerbNow;
}

// ============================================================================
// SURFACE EFFECT
// Continuous texture feedback based on road surface.
// Each surface should feel distinct and provide useful information.
// ============================================================================
void UMGWheelFFBProcessor::UpdateSurfaceEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	if (Data.SurfaceType == CurrentSurfaceType)
	{
		return; // No change
	}

	CurrentSurfaceType = Data.SurfaceType;

	if (!WheelSubsystem.IsValid())
	{
		return;
	}

	// Stop previous effect
	if (SurfaceEffectID.IsValid())
	{
		WheelSubsystem->StopFFBEffect(SurfaceEffectID);
		SurfaceEffectID.Invalidate();
	}

	// Start new surface effect based on type
	float SpeedIntensity = FMath::Clamp(Data.SpeedKmh / 120.0f, 0.1f, 1.0f);

	FMGFFBEffect Effect;
	Effect.EffectType = EMGFFBEffectType::SineWave;
	Effect.Duration = -1.0f; // Continuous

	// Configure based on surface
	if (Data.SurfaceType == FName("Gravel"))
	{
		// Gravel: Strong, chaotic feel
		Effect.Magnitude = 0.35f * SpeedIntensity;
		Effect.Frequency = 22.0f + FMath::RandRange(-3.0f, 3.0f);
	}
	else if (Data.SurfaceType == FName("Dirt"))
	{
		// Dirt: Moderate rumble, lower frequency
		Effect.Magnitude = 0.25f * SpeedIntensity;
		Effect.Frequency = 15.0f;
	}
	else if (Data.SurfaceType == FName("Grass"))
	{
		// Grass: Soft, low-frequency
		Effect.Magnitude = 0.18f * SpeedIntensity;
		Effect.Frequency = 10.0f;
	}
	else if (Data.SurfaceType == FName("Sand"))
	{
		// Sand: Heavy, sluggish feel
		Effect.Magnitude = 0.30f * SpeedIntensity;
		Effect.Frequency = 18.0f;
	}
	else if (Data.SurfaceType == FName("Wet"))
	{
		// Wet: Subtle texture, reduced grip feel comes from other systems
		Effect.Magnitude = 0.08f * SpeedIntensity;
		Effect.Frequency = 25.0f;
	}
	else if (Data.SurfaceType == FName("Ice"))
	{
		// Ice: Almost no texture (that's the scary part)
		Effect.Magnitude = 0.03f * SpeedIntensity;
		Effect.Frequency = 40.0f;
	}
	else
	{
		// Asphalt: Minimal texture at high speed, smooth road
		Effect.Magnitude = 0.02f * SpeedIntensity;
		Effect.Frequency = 35.0f;
	}

	if (Effect.Magnitude > 0.01f)
	{
		SurfaceEffectID = WheelSubsystem->PlayFFBEffect(Effect);
	}
}

// ============================================================================
// ENGINE EFFECT
// Vibration from the engine, especially near redline.
// Provides auditory-haptic feedback synchronization.
// ============================================================================
void UMGWheelFFBProcessor::UpdateEngineEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	if (Profile.EngineVibrationStrength <= 0.01f)
	{
		return;
	}

	float RPMPercent = Data.EngineRPM / FMath::Max(Data.MaxEngineRPM, 1000.0f);

	// Only vibrate near/at redline (above 85%)
	if (RPMPercent < 0.85f)
	{
		if (EngineEffectID.IsValid() && WheelSubsystem.IsValid())
		{
			WheelSubsystem->StopFFBEffect(EngineEffectID);
			EngineEffectID.Invalidate();
		}
		CurrentEngineVibration = 0.0f;
		return;
	}

	// Calculate intensity based on how far into redline we are
	float RedlineAmount = (RPMPercent - 0.85f) / 0.15f; // 0 at 85%, 1 at 100%
	CurrentEngineVibration = RedlineAmount * Profile.EngineVibrationStrength * 0.25f;

	FMGFFBEffect Effect;
	Effect.EffectType = EMGFFBEffectType::SineWave;
	Effect.Magnitude = CurrentEngineVibration;
	Effect.Frequency = 35.0f + RedlineAmount * 45.0f; // 35-80 Hz
	Effect.Duration = -1.0f;

	if (WheelSubsystem.IsValid())
	{
		if (!EngineEffectID.IsValid())
		{
			EngineEffectID = WheelSubsystem->PlayFFBEffect(Effect);
		}
		else
		{
			WheelSubsystem->UpdateFFBEffect(EngineEffectID, Effect);
		}
	}
}

// ============================================================================
// COLLISION EFFECT
// Sharp impact when collision data is present.
// ============================================================================
void UMGWheelFFBProcessor::UpdateCollisionEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile)
{
	if (Data.CollisionImpact <= 0.0f)
	{
		return;
	}

	if (!WheelSubsystem.IsValid())
	{
		return;
	}

	// Trigger collision FFB
	WheelSubsystem->TriggerCollisionFFB(Data.CollisionImpact * Profile.CollisionStrength, Data.CollisionDirection);
}

// ============================================================================
// APPLY EFFECTS
// Combines all calculated forces and sends them to the wheel.
// ============================================================================
void UMGWheelFFBProcessor::ApplyEffects(const FMGWheelProfile& Profile)
{
	if (!WheelSubsystem.IsValid())
	{
		return;
	}

	// === COMBINE CONSTANT FORCES ===
	// These are directional forces that push the wheel one way or another

	// Aligning torque - the main "road feel"
	float ConstantForce = CurrentAligningTorque;

	// Add drift counter-steer force
	ConstantForce += CurrentDriftFeedback;

	// Add G-force feedback
	ConstantForce += CurrentGForce;

	// Add weight transfer (subtle)
	ConstantForce += CurrentWeightTransfer;

	// Grip feedback reduces overall force magnitude (understeer = light wheel)
	float GripModifier = 1.0f + CurrentGripFeedback; // GripFeedback is negative when losing grip
	ConstantForce *= FMath::Max(GripModifier, 0.2f);

	// Apply minimum force threshold
	if (FMath::Abs(ConstantForce) < Profile.MinForceThreshold)
	{
		ConstantForce = 0.0f;
	}

	TotalConstantForce = ConstantForce;

	// === APPLY SPRING (SELF-CENTERING) ===
	// Modified by grip - when understeering, centering reduces

	float SpringStrength = CurrentSelfCentering;
	SpringStrength *= FMath::Max(GripModifier, 0.3f);

	TotalSpringForce = SpringStrength;

	// === SEND TO WHEEL ===

	// Spring effect (self-centering)
	WheelSubsystem->SetSelfCentering(SpringStrength, 0.5f + SpringStrength * 0.3f);

	// Damper (smooths out the FFB, reduces oscillation)
	WheelSubsystem->SetDamperStrength(Profile.DamperStrength);

	// Constant force (road feel, drift feedback, etc.)
	if (FMath::Abs(ConstantForce) > 0.01f)
	{
		FMGFFBEffect Effect;
		Effect.EffectType = EMGFFBEffectType::ConstantForce;
		Effect.Magnitude = FMath::Clamp(ConstantForce, -1.0f, 1.0f);
		Effect.Duration = -1.0f;

		if (!ConstantForceEffectID.IsValid())
		{
			ConstantForceEffectID = WheelSubsystem->PlayFFBEffect(Effect);
		}
		else
		{
			WheelSubsystem->UpdateFFBEffect(ConstantForceEffectID, Effect);
		}
	}
	else if (ConstantForceEffectID.IsValid())
	{
		WheelSubsystem->StopFFBEffect(ConstantForceEffectID);
		ConstantForceEffectID.Invalidate();
	}

	// Calculate total output for clipping detection
	TotalOutputForce = FMath::Abs(ConstantForce) + SpringStrength * 0.5f + CurrentEngineVibration + CurrentSurfaceRumble;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

float UMGWheelFFBProcessor::SmoothForce(float Current, float Target, float SmoothTime, float MGDeltaTime)
{
	if (SmoothTime <= 0.0f || DeltaTime <= 0.0f)
	{
		return Target;
	}

	// Exponential smoothing for natural feel
	float Alpha = 1.0f - FMath::Exp(-DeltaTime / SmoothTime);
	return FMath::Lerp(Current, Target, Alpha);
}

float UMGWheelFFBProcessor::ApplyDeadzone(float Value, float Deadzone)
{
	if (FMath::Abs(Value) < Deadzone)
	{
		return 0.0f;
	}

	float Sign = FMath::Sign(Value);
	float Magnitude = FMath::Abs(Value);
	return Sign * (Magnitude - Deadzone) / (1.0f - Deadzone);
}

float UMGWheelFFBProcessor::NormalizeSlipAngle(float SlipAngleDeg)
{
	return FMath::Clamp(SlipAngleDeg / MaxSlipAngle, -1.0f, 1.0f);
}

float UMGWheelFFBProcessor::CalculateTireGripFromSlip(float SlipAngle)
{
	// Grip model: full grip until GripLossStartAngle, then linear drop to zero
	float AbsSlip = FMath::Abs(SlipAngle);

	if (AbsSlip <= GripLossStartAngle)
	{
		return 1.0f;
	}
	else if (AbsSlip >= GripLossFullAngle)
	{
		return 0.1f; // Never quite zero - always some feedback
	}
	else
	{
		float T = (AbsSlip - GripLossStartAngle) / (GripLossFullAngle - GripLossStartAngle);
		return FMath::Lerp(1.0f, 0.1f, T);
	}
}

void UMGWheelFFBProcessor::GetEffectContributions(float& OutSelfCentering, float& OutAligningTorque,
                                                   float& OutUndersteer, float& OutOversteer,
                                                   float& OutSurface, float& OutEngine) const
{
	OutSelfCentering = CurrentSelfCentering;
	OutAligningTorque = CurrentAligningTorque;
	OutUndersteer = CurrentGripFeedback;
	OutOversteer = CurrentDriftFeedback;
	OutSurface = CurrentSurfaceRumble;
	OutEngine = CurrentEngineVibration;
}

void UMGWheelFFBProcessor::Reset()
{
	CurrentSelfCentering = 0.0f;
	CurrentAligningTorque = 0.0f;
	CurrentGripFeedback = 0.0f;
	CurrentDriftFeedback = 0.0f;
	CurrentWeightTransfer = 0.0f;
	CurrentGForce = 0.0f;
	CurrentSurfaceRumble = 0.0f;
	CurrentEngineVibration = 0.0f;

	TargetSelfCentering = 0.0f;
	TargetAligningTorque = 0.0f;
	TargetGripFeedback = 0.0f;
	TargetDriftFeedback = 0.0f;
	TargetWeightTransfer = 0.0f;
	TargetGForce = 0.0f;

	TotalOutputForce = 0.0f;
	TotalConstantForce = 0.0f;
	TotalSpringForce = 0.0f;

	TimeSinceLastUpdate = 0.0f;
	LastDeltaTime = 0.016f;

	ConstantForceEffectID.Invalidate();
	SpringEffectID.Invalidate();
	DamperEffectID.Invalidate();
	SurfaceEffectID.Invalidate();
	EngineEffectID.Invalidate();
	KerbEffectID.Invalidate();
	CollisionEffectID.Invalidate();

	bWasOnKerb = false;
	CurrentSurfaceType = NAME_None;
	bIsAirborne = false;
	bWasDrifting = false;
	DriftEntryTime = 0.0f;
	CurrentDriftDuration = 0.0f;
	PeakDriftAngle = 0.0f;
}

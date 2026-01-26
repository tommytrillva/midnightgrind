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

void UMGWheelFFBProcessor::Tick(float DeltaTime)
{
	TimeSinceLastUpdate += DeltaTime;
}

void UMGWheelFFBProcessor::ProcessVehicleData(const FMGFFBInputData& VehicleData, const FMGWheelProfile& Profile)
{
	if (!WheelSubsystem.IsValid())
	{
		return;
	}

	float DeltaTime = TimeSinceLastUpdate;
	TimeSinceLastUpdate = 0.0f;

	// Store current data
	LastVehicleData = VehicleData;
	bIsAirborne = VehicleData.bIsAirborne;

	// Skip FFB processing if airborne
	if (bIsAirborne)
	{
		// Reduce all forces smoothly
		CurrentSelfCenteringForce = SmoothForce(CurrentSelfCenteringForce, 0.0f, 0.1f, DeltaTime);
		CurrentAligningTorque = SmoothForce(CurrentAligningTorque, 0.0f, 0.1f, DeltaTime);
		CurrentUndersteerForce = SmoothForce(CurrentUndersteerForce, 0.0f, 0.1f, DeltaTime);
		CurrentOversteerForce = SmoothForce(CurrentOversteerForce, 0.0f, 0.1f, DeltaTime);
		ApplyEffects(Profile);
		return;
	}

	// Calculate self-centering force
	TargetSelfCenteringForce = CalculateSelfCenteringForce(
		VehicleData.SpeedKmh,
		VehicleData.SteeringAngle,
		Profile
	);

	// Calculate aligning torque (road feel)
	TargetAligningTorque = CalculateAligningTorque(
		VehicleData.FrontSlipAngle,
		VehicleData.FrontTireLoad,
		Profile
	);

	// Calculate understeer feedback
	if (VehicleData.bIsUndersteering)
	{
		TargetUndersteerForce = CalculateUndersteerFeedback(
			VehicleData.FrontSlipAngle,
			VehicleData.RearSlipAngle,
			Profile
		);
	}
	else
	{
		TargetUndersteerForce = 0.0f;
	}

	// Calculate oversteer feedback
	if (VehicleData.bIsOversteering)
	{
		TargetOversteerForce = CalculateOversteerFeedback(
			VehicleData.FrontSlipAngle,
			VehicleData.RearSlipAngle,
			VehicleData.YawRate,
			Profile
		);
	}
	else
	{
		TargetOversteerForce = 0.0f;
	}

	// Smooth force transitions
	CurrentSelfCenteringForce = SmoothForce(CurrentSelfCenteringForce, TargetSelfCenteringForce, SelfCenteringSmoothTime, DeltaTime);
	CurrentAligningTorque = SmoothForce(CurrentAligningTorque, TargetAligningTorque, AligningTorqueSmoothTime, DeltaTime);
	CurrentUndersteerForce = SmoothForce(CurrentUndersteerForce, TargetUndersteerForce, SlipFeedbackSmoothTime, DeltaTime);
	CurrentOversteerForce = SmoothForce(CurrentOversteerForce, TargetOversteerForce, SlipFeedbackSmoothTime, DeltaTime);

	// Update periodic effects
	UpdateKerbEffect(VehicleData.bOnRumbleStrip, VehicleData.SpeedKmh, Profile);
	UpdateSurfaceEffect(VehicleData.SurfaceType, VehicleData.SpeedKmh, Profile);
	UpdateEngineEffect(VehicleData.EngineRPM / VehicleData.MaxEngineRPM, Profile);

	// Apply all effects to the wheel
	ApplyEffects(Profile);
}

float UMGWheelFFBProcessor::CalculateSelfCenteringForce(float Speed, float SteeringAngle, const FMGWheelProfile& Profile)
{
	if (Speed < MinSelfCenteringSpeed)
	{
		return 0.0f;
	}

	// Self-centering increases with speed
	float SpeedFactor = FMath::Clamp(Speed / MaxSelfCenteringSpeed, 0.0f, 1.0f);

	// Spring force proportional to steering angle
	float SpringForce = -SteeringAngle * SpeedFactor;

	// Apply profile strength
	SpringForce *= Profile.SelfCenteringStrength;

	// Apply a curve for more natural feel (stronger at center, softer at extremes)
	float AbsForce = FMath::Abs(SpringForce);
	float CurvedForce = FMath::Pow(AbsForce, 0.8f) * FMath::Sign(SpringForce);

	return FMath::Clamp(CurvedForce, -1.0f, 1.0f);
}

float UMGWheelFFBProcessor::CalculateAligningTorque(float SlipAngle, float TireLoad, const FMGWheelProfile& Profile)
{
	// Aligning torque from pneumatic trail
	// Gives the driver information about tire grip

	// Normalize slip angle
	float NormalizedSlip = FMath::Clamp(SlipAngle / MaxSlipAngleForFeedback, -1.0f, 1.0f);

	// Self-aligning torque curve (peaks around 6-8 degrees, then drops off)
	// Using a simplified model: SAT = slip * (1 - slip^2)
	float SlipSquared = NormalizedSlip * NormalizedSlip;
	float SAT = NormalizedSlip * (1.0f - SlipSquared * 0.5f);

	// Scale by tire load (more load = more feedback)
	SAT *= FMath::Clamp(TireLoad, 0.5f, 1.5f);

	// Apply profile strength
	SAT *= Profile.RoadFeelStrength;

	return FMath::Clamp(SAT, -1.0f, 1.0f);
}

float UMGWheelFFBProcessor::CalculateUndersteerFeedback(float FrontSlip, float RearSlip, const FMGWheelProfile& Profile)
{
	// Understeer: front tires are slipping more than rear
	// Feedback: reduce spring force / wheel feels "light"

	float SlipDifference = FMath::Abs(FrontSlip) - FMath::Abs(RearSlip);
	if (SlipDifference <= 0.0f)
	{
		return 0.0f;
	}

	// Normalize understeer amount
	float UndersteerAmount = FMath::Clamp(SlipDifference / MaxSlipAngleForFeedback, 0.0f, 1.0f);

	// Apply profile strength
	UndersteerAmount *= Profile.UndersteerStrength;

	// This will reduce the overall force (implemented as negative modifier to spring)
	return UndersteerAmount;
}

float UMGWheelFFBProcessor::CalculateOversteerFeedback(float FrontSlip, float RearSlip, float YawRate, const FMGWheelProfile& Profile)
{
	// Oversteer: rear tires are sliding out
	// Feedback: counter-force in direction opposite to slide

	// Calculate oversteer amount from rear slip
	float OversteerAmount = FMath::Clamp(FMath::Abs(RearSlip) / MaxSlipAngleForFeedback, 0.0f, 1.0f);

	// Add yaw rate contribution
	float YawContribution = FMath::Clamp(FMath::Abs(YawRate) / 90.0f, 0.0f, 0.5f);
	OversteerAmount = FMath::Clamp(OversteerAmount + YawContribution, 0.0f, 1.0f);

	// Determine direction of counter-force
	float Direction = FMath::Sign(RearSlip);

	// Apply profile strength
	float Force = OversteerAmount * Profile.OversteerStrength * Direction;

	return FMath::Clamp(Force, -1.0f, 1.0f);
}

void UMGWheelFFBProcessor::UpdateKerbEffect(bool bOnKerb, float Speed, const FMGWheelProfile& Profile)
{
	if (bOnKerb && !bOnKerb)
	{
		// Start kerb effect
		if (WheelSubsystem.IsValid())
		{
			float Intensity = FMath::Clamp(Speed / 200.0f, 0.3f, 1.0f);
			KerbEffectID = WheelSubsystem->TriggerKerbFFB(Intensity * Profile.CurbStrength, -1.0f);
		}
	}
	else if (!bOnKerb && bOnKerb)
	{
		// Stop kerb effect
		if (WheelSubsystem.IsValid() && KerbEffectID.IsValid())
		{
			WheelSubsystem->StopFFBEffect(KerbEffectID);
			KerbEffectID.Invalidate();
		}
	}

	bOnKerb = bOnKerb;
}

void UMGWheelFFBProcessor::UpdateSurfaceEffect(FName SurfaceType, float Speed, const FMGWheelProfile& Profile)
{
	if (SurfaceType != CurrentSurface)
	{
		CurrentSurface = SurfaceType;

		if (WheelSubsystem.IsValid())
		{
			// Stop previous surface effect
			if (SurfaceEffectID.IsValid())
			{
				WheelSubsystem->StopFFBEffect(SurfaceEffectID);
			}

			// Start new surface effect
			float Intensity = FMath::Clamp(Speed / 150.0f, 0.1f, 1.0f);
			SurfaceEffectID = WheelSubsystem->TriggerSurfaceFFB(SurfaceType, Intensity);
		}
	}
}

void UMGWheelFFBProcessor::UpdateEngineEffect(float RPMPercent, const FMGWheelProfile& Profile)
{
	if (Profile.EngineVibrationStrength <= 0.0f)
	{
		return;
	}

	// Engine vibration only near redline (above 90%)
	if (RPMPercent > 0.9f)
	{
		CurrentEngineVibration = (RPMPercent - 0.9f) * 10.0f * Profile.EngineVibrationStrength;
	}
	else
	{
		CurrentEngineVibration = 0.0f;
	}

	if (WheelSubsystem.IsValid())
	{
		WheelSubsystem->UpdateEngineFFB(RPMPercent);
	}
}

void UMGWheelFFBProcessor::ApplyEffects(const FMGWheelProfile& Profile)
{
	if (!WheelSubsystem.IsValid())
	{
		return;
	}

	// Combine all forces
	// Self-centering is a spring effect, others are constant force modifiers

	// Calculate final constant force (aligning torque + oversteer)
	float ConstantForce = CurrentAligningTorque + CurrentOversteerForce;

	// Understeer reduces the overall force feel
	float UndersteerModifier = 1.0f - (CurrentUndersteerForce * 0.5f);

	// Apply understeer modifier
	ConstantForce *= UndersteerModifier;

	// Track total force for clipping detection
	TotalOutputForce = FMath::Abs(ConstantForce) + FMath::Abs(CurrentSelfCenteringForce);

	// Apply spring effect (self-centering)
	float SpringCoefficient = FMath::Clamp(CurrentSelfCenteringForce * UndersteerModifier, 0.0f, 1.0f);
	WheelSubsystem->SetSelfCentering(SpringCoefficient, 0.5f);

	// Apply damper
	WheelSubsystem->SetDamperStrength(Profile.DamperStrength);

	// Apply constant force for road feel
	if (FMath::Abs(ConstantForce) > Profile.MinForceThreshold)
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
}

float UMGWheelFFBProcessor::SmoothForce(float CurrentForce, float TargetForce, float SmoothTime, float DeltaTime)
{
	if (SmoothTime <= 0.0f)
	{
		return TargetForce;
	}

	float Alpha = FMath::Clamp(DeltaTime / SmoothTime, 0.0f, 1.0f);
	return FMath::Lerp(CurrentForce, TargetForce, Alpha);
}

float UMGWheelFFBProcessor::ApplyForceCurve(float Force, float Gamma)
{
	float AbsForce = FMath::Abs(Force);
	float CurvedForce = FMath::Pow(AbsForce, Gamma);
	return CurvedForce * FMath::Sign(Force);
}

void UMGWheelFFBProcessor::GetEffectContributions(float& OutSelfCentering, float& OutAligningTorque,
												   float& OutUndersteer, float& OutOversteer,
												   float& OutSurface, float& OutEngine) const
{
	OutSelfCentering = CurrentSelfCenteringForce;
	OutAligningTorque = CurrentAligningTorque;
	OutUndersteer = CurrentUndersteerForce;
	OutOversteer = CurrentOversteerForce;
	OutSurface = CurrentSurfaceForce;
	OutEngine = CurrentEngineVibration;
}

void UMGWheelFFBProcessor::Reset()
{
	CurrentSelfCenteringForce = 0.0f;
	CurrentAligningTorque = 0.0f;
	CurrentUndersteerForce = 0.0f;
	CurrentOversteerForce = 0.0f;
	CurrentSurfaceForce = 0.0f;
	CurrentEngineVibration = 0.0f;
	CurrentDamperForce = 0.0f;

	TargetSelfCenteringForce = 0.0f;
	TargetAligningTorque = 0.0f;
	TargetUndersteerForce = 0.0f;
	TargetOversteerForce = 0.0f;

	TotalOutputForce = 0.0f;
	TimeSinceLastUpdate = 0.0f;

	ConstantForceEffectID.Invalidate();
	SpringEffectID.Invalidate();
	DamperEffectID.Invalidate();
	SurfaceEffectID.Invalidate();
	EngineEffectID.Invalidate();
	KerbEffectID.Invalidate();

	bOnKerb = false;
	CurrentSurface = NAME_None;
	bIsAirborne = false;
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"

UMGVehicleMovementComponent::UMGVehicleMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set tick to happen every frame
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UMGVehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize in neutral
	CurrentGear = 0;
	EngineState.CurrentRPM = 800.0f; // Idle
}

void UMGVehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update our custom systems
	UpdateEngineSimulation(DeltaTime);
	UpdateBoostSimulation(DeltaTime);
	UpdateDriftPhysics(DeltaTime);
	UpdateNitrousSystem(DeltaTime);
	ApplyStabilityControl(DeltaTime);
	ApplyAntiFlipForce(DeltaTime);

	// Update shift cooldown
	if (ShiftCooldown > 0.0f)
	{
		ShiftCooldown -= DeltaTime;
	}

	// Smooth steering input
	const float SteeringDelta = TargetSteering - CurrentSteering;
	if (FMath::Abs(SteeringDelta) > KINDA_SMALL_NUMBER)
	{
		const float SteerSpeed = TargetSteering != 0.0f ? ArcadeSteeringSpeed : ArcadeSteeringReturnSpeed;
		CurrentSteering = FMath::FInterpTo(CurrentSteering, TargetSteering, DeltaTime, SteerSpeed);

		// Apply speed-sensitive steering
		const float SpeedFactor = FMath::Clamp(GetSpeedMPH() / 100.0f, 0.0f, 1.0f);
		const float SteerReduction = FMath::Lerp(1.0f, 1.0f - SpeedSensitiveSteeringFactor, SpeedFactor);

		SetSteeringInput(CurrentSteering * SteerReduction);
	}
}

// ==========================================
// INPUT METHODS
// ==========================================

void UMGVehicleMovementComponent::SetThrottleInput(float Value)
{
	EngineState.ThrottlePosition = FMath::Clamp(Value, 0.0f, 1.0f);

	// Pass to base class
	Super::SetThrottleInput(Value);
}

void UMGVehicleMovementComponent::SetBrakeInput(float Value)
{
	Super::SetBrakeInput(FMath::Clamp(Value, 0.0f, 1.0f));
}

void UMGVehicleMovementComponent::SetSteeringInput(float Value)
{
	TargetSteering = FMath::Clamp(Value, -1.0f, 1.0f);
}

void UMGVehicleMovementComponent::SetHandbrakeInput(bool bEngaged)
{
	bHandbrakeEngaged = bEngaged;
	Super::SetHandbrakeInput(bEngaged);
}

void UMGVehicleMovementComponent::ActivateNitrous()
{
	// Check if we can activate
	if (EngineState.NitrousRemaining <= 0.0f)
	{
		return;
	}

	// Check minimum RPM
	const float MinRPM = CurrentConfiguration.Stats.Redline * NitrousMinimumRPMPercent;
	if (EngineState.CurrentRPM < MinRPM)
	{
		return;
	}

	if (!EngineState.bNitrousActive)
	{
		EngineState.bNitrousActive = true;
		OnNitrousStateChanged.Broadcast(true);
	}
}

void UMGVehicleMovementComponent::DeactivateNitrous()
{
	if (EngineState.bNitrousActive)
	{
		EngineState.bNitrousActive = false;
		OnNitrousStateChanged.Broadcast(false);
	}
}

void UMGVehicleMovementComponent::ShiftUp()
{
	if (ShiftCooldown > 0.0f)
	{
		return;
	}

	const int32 MaxGear = CurrentConfiguration.Drivetrain.GearCount;
	if (CurrentGear < MaxGear)
	{
		PerformGearShift(CurrentGear + 1);
	}
}

void UMGVehicleMovementComponent::ShiftDown()
{
	if (ShiftCooldown > 0.0f)
	{
		return;
	}

	if (CurrentGear > -1) // Can go to reverse
	{
		PerformGearShift(CurrentGear - 1);
	}
}

void UMGVehicleMovementComponent::PerformGearShift(int32 NewGear)
{
	CurrentGear = NewGear;
	ShiftCooldown = CurrentConfiguration.Drivetrain.ShiftTimeSeconds;

	OnGearChanged.Broadcast(CurrentGear);

	// TODO: Apply transmission gear to physics system
}

// ==========================================
// STATE QUERIES
// ==========================================

float UMGVehicleMovementComponent::GetSpeedMPH() const
{
	// Get forward velocity in cm/s, convert to MPH
	// 1 mph = 44.704 cm/s
	const FVector Velocity = GetOwner()->GetVelocity();
	const float ForwardSpeed = FVector::DotProduct(Velocity, GetOwner()->GetActorForwardVector());
	return FMath::Abs(ForwardSpeed) / 44.704f;
}

float UMGVehicleMovementComponent::GetSpeedKPH() const
{
	return GetSpeedMPH() * 1.60934f;
}

bool UMGVehicleMovementComponent::IsGrounded() const
{
	// Check all wheels
	for (int32 i = 0; i < 4; ++i)
	{
		// TODO: Implement proper wheel grounded check using Chaos
	}
	return true; // Placeholder
}

bool UMGVehicleMovementComponent::IsWheelSlipping(int32 WheelIndex) const
{
	const float SlipThreshold = 0.2f;
	return GetWheelSlipRatio(WheelIndex) > SlipThreshold;
}

float UMGVehicleMovementComponent::GetWheelSlipAngle(int32 WheelIndex) const
{
	// TODO: Get from Chaos wheel state
	return 0.0f;
}

float UMGVehicleMovementComponent::GetWheelSlipRatio(int32 WheelIndex) const
{
	// TODO: Get from Chaos wheel state
	return 0.0f;
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGVehicleMovementComponent::ApplyVehicleConfiguration(const FMGVehicleData& VehicleData)
{
	CurrentConfiguration = VehicleData;

	// Apply tire grip based on compound
	const float FrontGrip = GetTireCompoundGrip(VehicleData.WheelsTires.FrontTireCompound);
	const float RearGrip = GetTireCompoundGrip(VehicleData.WheelsTires.RearTireCompound);

	// TODO: Apply to Chaos wheel setup

	// Apply suspension settings
	// TODO: Configure Chaos suspension from our data

	// Apply aero
	// TODO: Apply downforce calculations

	// Initialize nitrous
	if (VehicleData.Engine.Nitrous.bInstalled)
	{
		EngineState.NitrousRemaining = VehicleData.Engine.Nitrous.CurrentFillPercent;
	}
	else
	{
		EngineState.NitrousRemaining = 0.0f;
	}
}

// ==========================================
// SIMULATION UPDATES
// ==========================================

void UMGVehicleMovementComponent::UpdateEngineSimulation(float DeltaTime)
{
	// Calculate target RPM based on speed and gear
	const float Speed = GetSpeedMPH();

	if (CurrentGear == 0)
	{
		// Neutral - rev freely with throttle
		const float IdleRPM = 800.0f;
		const float MaxRPM = CurrentConfiguration.Stats.Redline;
		const float TargetRPM = FMath::Lerp(IdleRPM, MaxRPM, EngineState.ThrottlePosition);
		EngineState.CurrentRPM = FMath::FInterpTo(EngineState.CurrentRPM, TargetRPM, DeltaTime, 10.0f);
	}
	else if (CurrentGear > 0)
	{
		// Calculate RPM from wheel speed
		const int32 GearIndex = CurrentGear - 1;
		if (GearIndex < CurrentConfiguration.Drivetrain.GearRatios.Num())
		{
			const float GearRatio = CurrentConfiguration.Drivetrain.GearRatios[GearIndex];
			const float FinalDrive = CurrentConfiguration.Drivetrain.FinalDriveRatio;

			// Simplified RPM calculation
			// Real formula would involve tire circumference
			const float WheelRPM = Speed * 14.0f; // Approximation
			const float EngineRPM = WheelRPM * GearRatio * FinalDrive;

			EngineState.CurrentRPM = FMath::Clamp(EngineRPM, 800.0f, (float)CurrentConfiguration.Stats.Redline);
		}
	}
	else // Reverse
	{
		const float ReverseRatio = FMath::Abs(CurrentConfiguration.Drivetrain.ReverseGearRatio);
		const float FinalDrive = CurrentConfiguration.Drivetrain.FinalDriveRatio;
		const float WheelRPM = Speed * 14.0f;
		const float EngineRPM = WheelRPM * ReverseRatio * FinalDrive;

		EngineState.CurrentRPM = FMath::Clamp(EngineRPM, 800.0f, (float)CurrentConfiguration.Stats.Redline);
	}

	// Rev limiter
	EngineState.bRevLimiterActive = EngineState.CurrentRPM >= CurrentConfiguration.Stats.Redline;

	// Engine load
	EngineState.EngineLoad = EngineState.ThrottlePosition * (EngineState.CurrentRPM / CurrentConfiguration.Stats.Redline);
}

void UMGVehicleMovementComponent::UpdateBoostSimulation(float DeltaTime)
{
	const FMGForcedInductionConfig& FI = CurrentConfiguration.Engine.ForcedInduction;

	if (FI.Type == EMGForcedInductionType::None)
	{
		EngineState.CurrentBoostPSI = 0.0f;
		EngineState.BoostBuildupPercent = 0.0f;
		return;
	}

	// Turbo simulation
	if (FI.Type == EMGForcedInductionType::Turbo_Single || FI.Type == EMGForcedInductionType::Turbo_Twin)
	{
		// Check if above boost threshold
		const bool bAboveThreshold = EngineState.CurrentRPM >= FI.BoostThresholdRPM;

		if (bAboveThreshold && EngineState.ThrottlePosition > 0.5f)
		{
			// Build boost
			const float TargetBoost = FI.MaxBoostPSI * EngineState.ThrottlePosition;
			const float BuildRate = BoostBuildupRate / (FI.SpoolTimeSeconds * TurboLagSimulation);
			EngineState.BoostBuildupPercent = FMath::FInterpConstantTo(EngineState.BoostBuildupPercent, 1.0f, DeltaTime, BuildRate);
			EngineState.CurrentBoostPSI = TargetBoost * EngineState.BoostBuildupPercent;
		}
		else
		{
			// Decay boost
			EngineState.BoostBuildupPercent = FMath::FInterpConstantTo(EngineState.BoostBuildupPercent, 0.0f, DeltaTime, BoostDecayRate);
			EngineState.CurrentBoostPSI = FI.MaxBoostPSI * EngineState.BoostBuildupPercent * 0.5f;
		}
	}
	// Supercharger simulation (instant response)
	else if (FI.Type == EMGForcedInductionType::Supercharger_Roots ||
	         FI.Type == EMGForcedInductionType::Supercharger_TwinScrew ||
	         FI.Type == EMGForcedInductionType::Supercharger_Centrifugal)
	{
		// Instant response tied to RPM and throttle
		const float RPMFactor = EngineState.CurrentRPM / CurrentConfiguration.Stats.Redline;
		EngineState.CurrentBoostPSI = FI.MaxBoostPSI * EngineState.ThrottlePosition * RPMFactor;
		EngineState.BoostBuildupPercent = RPMFactor;
	}

	// Broadcast boost changes
	if (FMath::Abs(EngineState.CurrentBoostPSI - LastBoostBroadcast) > 1.0f)
	{
		LastBoostBroadcast = EngineState.CurrentBoostPSI;
		OnBoostChanged.Broadcast(EngineState.CurrentBoostPSI, FI.MaxBoostPSI);
	}
}

void UMGVehicleMovementComponent::UpdateDriftPhysics(float DeltaTime)
{
	// Calculate vehicle slip angle
	const FVector Velocity = GetOwner()->GetVelocity();
	const FVector Forward = GetOwner()->GetActorForwardVector();

	if (Velocity.SizeSquared() > 100.0f) // Minimum speed
	{
		const FVector VelocityDir = Velocity.GetSafeNormal();
		const float DotProduct = FVector::DotProduct(Forward, VelocityDir);
		DriftState.DriftAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

		// Determine drift direction (left or right)
		const FVector Cross = FVector::CrossProduct(Forward, VelocityDir);
		if (Cross.Z < 0)
		{
			DriftState.DriftAngle = -DriftState.DriftAngle;
		}
	}
	else
	{
		DriftState.DriftAngle = 0.0f;
	}

	// Update drift state
	const bool bWasDrifting = DriftState.bIsDrifting;
	DriftState.bIsDrifting = FMath::Abs(DriftState.DriftAngle) > DriftAngleThreshold;

	if (DriftState.bIsDrifting)
	{
		DriftState.DriftDuration += DeltaTime;

		// Score based on angle, speed, and duration
		const float AngleScore = FMath::Abs(DriftState.DriftAngle) / 90.0f; // Normalize to 90 degrees
		const float SpeedScore = FMath::Clamp(GetSpeedMPH() / 60.0f, 0.0f, 1.0f);
		DriftState.DriftScore += AngleScore * SpeedScore * 100.0f * DeltaTime;
	}
	else
	{
		// Reset drift tracking
		if (bWasDrifting)
		{
			// Drift ended - could trigger scoring event here
		}
		DriftState.DriftDuration = 0.0f;
		DriftState.DriftScore = 0.0f;
	}

	// Apply drift physics modifications
	// This would modify tire friction through the Chaos system
}

void UMGVehicleMovementComponent::UpdateNitrousSystem(float DeltaTime)
{
	if (!EngineState.bNitrousActive)
	{
		return;
	}

	// Consume nitrous
	EngineState.NitrousRemaining -= NitrousConsumptionRate * DeltaTime;

	if (EngineState.NitrousRemaining <= 0.0f)
	{
		EngineState.NitrousRemaining = 0.0f;
		DeactivateNitrous();
	}

	// Apply power boost
	// This affects CalculateCurrentPower()
}

void UMGVehicleMovementComponent::ApplyStabilityControl(float DeltaTime)
{
	if (StabilityControl <= 0.0f)
	{
		return;
	}

	// Simple stability: reduce oversteer/understeer
	const float SlipAngle = FMath::Abs(DriftState.DriftAngle);

	if (SlipAngle > DriftAngleThreshold && !bHandbrakeEngaged)
	{
		// Apply corrective yaw torque
		const float CorrectionStrength = SlipAngle * StabilityControl * 100.0f;
		const float CorrectionDir = DriftState.DriftAngle > 0 ? -1.0f : 1.0f;

		// TODO: Apply torque through physics system
		// GetOwner()->GetRootComponent()->AddTorqueInDegrees(FVector(0, 0, CorrectionStrength * CorrectionDir));
	}
}

void UMGVehicleMovementComponent::ApplyAntiFlipForce(float DeltaTime)
{
	if (AntiFlipTorque <= 0.0f)
	{
		return;
	}

	// Check roll angle
	const FRotator Rotation = GetOwner()->GetActorRotation();
	const float RollAngle = FMath::Abs(Rotation.Roll);

	if (RollAngle > 45.0f && RollAngle < 135.0f)
	{
		// Apply anti-flip torque
		const float FlipDir = Rotation.Roll > 0 ? -1.0f : 1.0f;

		// TODO: Apply torque through physics system
		// GetOwner()->GetRootComponent()->AddTorqueInDegrees(FVector(AntiFlipTorque * FlipDir, 0, 0));
	}
}

// ==========================================
// CALCULATIONS
// ==========================================

float UMGVehicleMovementComponent::CalculateTireFriction(int32 WheelIndex) const
{
	float BaseFriction = BaseTireGrip;

	// Get compound for this wheel
	EMGTireCompound Compound;
	float Condition;
	if (WheelIndex < 2) // Front wheels
	{
		Compound = CurrentConfiguration.WheelsTires.FrontTireCompound;
		Condition = CurrentConfiguration.WheelsTires.FrontTireCondition;
	}
	else // Rear wheels
	{
		Compound = CurrentConfiguration.WheelsTires.RearTireCompound;
		Condition = CurrentConfiguration.WheelsTires.RearTireCondition;
	}

	// Apply compound grip
	BaseFriction *= GetTireCompoundGrip(Compound);

	// Apply condition degradation
	const float ConditionFactor = FMath::Lerp(0.5f, 1.0f, Condition / 100.0f);
	BaseFriction *= ConditionFactor;

	// Apply drift modifier
	if (DriftState.bIsDrifting && WheelIndex >= 2) // Rear wheels during drift
	{
		BaseFriction *= DriftFrictionMultiplier;
	}

	// Apply handbrake modifier
	if (bHandbrakeEngaged && WheelIndex >= 2)
	{
		BaseFriction *= HandbrakeFrictionMultiplier;
	}

	return BaseFriction;
}

float UMGVehicleMovementComponent::CalculateCurrentPower() const
{
	float Power = CurrentConfiguration.Stats.Horsepower;

	// Apply boost multiplier
	if (EngineState.CurrentBoostPSI > 0.0f)
	{
		const float BoostMultiplier = 1.0f + (EngineState.CurrentBoostPSI / 20.0f); // ~5% per PSI
		Power *= BoostMultiplier;
	}

	// Apply nitrous
	if (EngineState.bNitrousActive)
	{
		Power *= NitrousPowerMultiplier;
	}

	// Apply engine condition
	// TODO: Factor in part conditions

	return Power;
}

float UMGVehicleMovementComponent::GetTireCompoundGrip(EMGTireCompound Compound)
{
	switch (Compound)
	{
		case EMGTireCompound::Economy:     return 0.70f;
		case EMGTireCompound::AllSeason:   return 0.78f;
		case EMGTireCompound::Sport:       return 0.85f;
		case EMGTireCompound::Performance: return 0.95f;
		case EMGTireCompound::SemiSlick:   return 1.05f;
		case EMGTireCompound::Slick:       return 1.15f;
		case EMGTireCompound::DragRadial:  return 1.10f;
		case EMGTireCompound::Drift:       return 0.80f;
		default:                           return 1.00f;
	}
}

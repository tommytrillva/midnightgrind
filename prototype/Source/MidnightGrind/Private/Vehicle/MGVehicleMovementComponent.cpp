// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "SimpleVehicle/SimpleWheelSim.h"
#include "Weather/MGWeatherSubsystem.h"

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

	// Update core systems
	UpdateEngineSimulation(DeltaTime);
	UpdateBoostSimulation(DeltaTime);
	UpdateTurboShaftSimulation(DeltaTime); // Advanced turbo physics
	UpdateDriftPhysics(DeltaTime);
	UpdateDriftScoring(DeltaTime); // Enhanced drift scoring with chains
	UpdateNitrousSystem(DeltaTime);
	ApplyStabilityControl(DeltaTime);
	ApplyAntiFlipForce(DeltaTime);

	// Update advanced physics systems
	UpdateTireTemperatures(DeltaTime);
	UpdateWeightTransfer(DeltaTime);
	UpdateAerodynamics(DeltaTime);
	UpdateAntiLag(DeltaTime);
	UpdateLaunchControl(DeltaTime);
	UpdateBrakeSystem(DeltaTime);
	UpdateSurfaceDetection(DeltaTime); // Surface grip detection with weather integration
	ApplyDifferentialBehavior(DeltaTime);
	UpdateClutchWear(DeltaTime); // Clutch wear and temperature simulation

	// Update shift cooldown
	if (ShiftCooldown > 0.0f)
	{
		ShiftCooldown -= DeltaTime;
	}

	// Smooth steering input with speed-sensitive reduction
	const float SteeringDelta = TargetSteering - CurrentSteering;
	if (FMath::Abs(SteeringDelta) > KINDA_SMALL_NUMBER)
	{
		const float SteerSpeed = TargetSteering != 0.0f ? ArcadeSteeringSpeed : ArcadeSteeringReturnSpeed;
		CurrentSteering = FMath::FInterpTo(CurrentSteering, TargetSteering, DeltaTime, SteerSpeed);

		// Apply speed-sensitive steering
		const float SteerReduction = CalculateSpeedSteeringFactor();
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

void UMGVehicleMovementComponent::SetTireGripMultiplier(float Multiplier)
{
	TireGripMultiplier = FMath::Clamp(Multiplier, 0.1f, 1.0f);

	// Apply grip multiplier to base tire grip
	// This affects the CalculateTireFriction function
	UE_LOG(LogTemp, Log, TEXT("Tire grip multiplier set to: %.2f"), TireGripMultiplier);
}

void UMGVehicleMovementComponent::SetMaxSpeedMultiplier(float Multiplier)
{
	MaxSpeedMultiplier = FMath::Clamp(Multiplier, 0.3f, 1.0f);

	// This affects max speed calculations
	UE_LOG(LogTemp, Log, TEXT("Max speed multiplier set to: %.2f"), MaxSpeedMultiplier);
}

void UMGVehicleMovementComponent::PerformGearShift(int32 NewGear)
{
	CurrentGear = NewGear;
	ShiftCooldown = CurrentConfiguration.Drivetrain.ShiftTimeSeconds;

	OnGearChanged.Broadcast(CurrentGear);

	// Apply gear to Chaos transmission
	if (NewGear < 0)
	{
		// Reverse
		SetTargetGear(-1, true);
	}
	else if (NewGear == 0)
	{
		// Neutral - handled by transmission auto-clutch
		SetTargetGear(0, true);
	}
	else
	{
		// Forward gears
		SetTargetGear(NewGear, true);
	}
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
	// Check if all wheels are in contact with ground using Chaos wheel state
	int32 WheelsInContact = 0;
	const int32 NumWheels = WheelSetups.Num();

	if (NumWheels == 0)
	{
		return false;
	}

	for (int32 i = 0; i < NumWheels; ++i)
	{
		if (const FChaosWheelSetup* WheelSetup = GetWheelSetup(i))
		{
			// Access the wheel state from the physics simulation
			const Chaos::FSimpleWheelSim& WheelSim = PVehicleOutput->Wheels[i];
			if (WheelSim.InContact())
			{
				WheelsInContact++;
			}
		}
	}

	// Consider grounded if at least 3 wheels are touching
	return WheelsInContact >= FMath::Min(3, NumWheels);
}

bool UMGVehicleMovementComponent::IsWheelSlipping(int32 WheelIndex) const
{
	const float SlipThreshold = 0.2f;
	return GetWheelSlipRatio(WheelIndex) > SlipThreshold;
}

float UMGVehicleMovementComponent::GetWheelSlipAngle(int32 WheelIndex) const
{
	// Get slip angle from Chaos wheel simulation
	if (WheelIndex >= 0 && WheelIndex < WheelSetups.Num() && PVehicleOutput)
	{
		if (WheelIndex < PVehicleOutput->Wheels.Num())
		{
			const Chaos::FSimpleWheelSim& WheelSim = PVehicleOutput->Wheels[WheelIndex];
			return WheelSim.GetSlipAngle();
		}
	}
	return 0.0f;
}

float UMGVehicleMovementComponent::GetWheelSlipRatio(int32 WheelIndex) const
{
	// Get longitudinal slip ratio from Chaos wheel simulation
	if (WheelIndex >= 0 && WheelIndex < WheelSetups.Num() && PVehicleOutput)
	{
		if (WheelIndex < PVehicleOutput->Wheels.Num())
		{
			const Chaos::FSimpleWheelSim& WheelSim = PVehicleOutput->Wheels[WheelIndex];
			return WheelSim.GetSlipMagnitude();
		}
	}
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

	// Apply friction to Chaos wheels (front wheels are indices 0,1, rear are 2,3)
	for (int32 i = 0; i < WheelSetups.Num(); ++i)
	{
		if (UChaosVehicleWheel* Wheel = Cast<UChaosVehicleWheel>(Wheels[i]))
		{
			const bool bIsFrontWheel = (i < 2);
			const float GripMultiplier = bIsFrontWheel ? FrontGrip : RearGrip;

			// Scale friction coefficient by our tire compound grip
			Wheel->FrictionForceMultiplier = BaseTireGrip * GripMultiplier;
		}
	}

	// Apply suspension settings from our data
	// Front suspension
	if (WheelSetups.Num() >= 2)
	{
		for (int32 i = 0; i < 2; ++i)
		{
			if (UChaosVehicleWheel* Wheel = Cast<UChaosVehicleWheel>(Wheels[i]))
			{
				Wheel->SuspensionMaxRaise = VehicleData.Suspension.FrontRideHeightMM / 10.0f; // mm to cm
				Wheel->SuspensionMaxDrop = VehicleData.Suspension.FrontRideHeightMM / 10.0f;
			}
		}
	}
	// Rear suspension
	if (WheelSetups.Num() >= 4)
	{
		for (int32 i = 2; i < 4; ++i)
		{
			if (UChaosVehicleWheel* Wheel = Cast<UChaosVehicleWheel>(Wheels[i]))
			{
				Wheel->SuspensionMaxRaise = VehicleData.Suspension.RearRideHeightMM / 10.0f;
				Wheel->SuspensionMaxDrop = VehicleData.Suspension.RearRideHeightMM / 10.0f;
			}
		}
	}

	// Apply aero downforce - affects grip at high speeds
	// Downforce is applied through the stability/anti-flip systems based on aero config
	const float DownforceMultiplier = VehicleData.Aero.FrontSplitter.bInstalled ? 1.1f : 1.0f;
	const float RearDownforce = VehicleData.Aero.RearWing.bInstalled ?
		(1.0f + VehicleData.Aero.RearWing.DownforceLevelPercent * 0.01f * 0.3f) : 1.0f;

	// Store aero values for use in physics updates (applied dynamically based on speed)
	BaseTireGrip *= DownforceMultiplier * RearDownforce;

	// Configure transmission
	if (TransmissionSetup.bUseAutomaticGears != (VehicleData.Drivetrain.TransmissionType == EMGTransmissionType::Automatic))
	{
		TransmissionSetup.bUseAutomaticGears = (VehicleData.Drivetrain.TransmissionType == EMGTransmissionType::Automatic);
	}

	// Initialize nitrous
	if (VehicleData.Engine.Nitrous.bInstalled)
	{
		EngineState.NitrousRemaining = VehicleData.Engine.Nitrous.CurrentFillPercent;
	}
	else
	{
		EngineState.NitrousRemaining = 0.0f;
	}

	// Initialize turbo state for advanced simulation
	if (VehicleData.Engine.ForcedInduction.Type == EMGForcedInductionType::Turbo_Single ||
		VehicleData.Engine.ForcedInduction.Type == EMGForcedInductionType::Turbo_Twin)
	{
		// Set max shaft RPM based on turbo size (smaller turbos spin faster)
		// Twin turbos use smaller units
		EngineState.TurboState.MaxShaftRPM = (VehicleData.Engine.ForcedInduction.Type == EMGForcedInductionType::Turbo_Twin)
			? 180000.0f : 150000.0f;
		EngineState.TurboState.ShaftRPM = 0.0f;
		EngineState.TurboState.CompressorEfficiency = 0.0f;
	}

	// Calculate and apply part wear effects
	UpdatePartWearEffects();

	UE_LOG(LogTemp, Log, TEXT("Applied vehicle configuration: %s (HP: %.0f, Grip F/R: %.2f/%.2f)"),
		*VehicleData.DisplayName, VehicleData.Stats.Horsepower, FrontGrip, RearGrip);
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

	// Engine load calculation - considers throttle, RPM, and boost
	EngineState.EngineLoad = EngineState.ThrottlePosition * (EngineState.CurrentRPM / CurrentConfiguration.Stats.Redline);

	// Apply boost to engine load
	if (EngineState.CurrentBoostPSI > 0.0f)
	{
		const float BoostLoadIncrease = EngineState.CurrentBoostPSI / 30.0f; // ~3.3% load increase per PSI
		EngineState.EngineLoad = FMath::Min(EngineState.EngineLoad + BoostLoadIncrease, 1.5f);
	}

	// ==========================================
	// ENGINE TEMPERATURE SIMULATION
	// ==========================================

	// Base heat generation from engine operation
	// Heat generation factors: RPM, throttle position, boost, nitrous
	const float RPMHeatFactor = EngineState.CurrentRPM / CurrentConfiguration.Stats.Redline;
	const float LoadHeatFactor = EngineState.EngineLoad;

	// Base heat generation rate (degrees per second at full load/RPM)
	const float BaseHeatRate = 5.0f;
	float HeatGenerated = BaseHeatRate * RPMHeatFactor * LoadHeatFactor * DeltaTime;

	// Additional heat from forced induction
	if (EngineState.CurrentBoostPSI > 0.0f)
	{
		const float BoostHeat = EngineState.CurrentBoostPSI * 0.15f * DeltaTime;
		HeatGenerated += BoostHeat;
	}

	// Significant additional heat from nitrous (combustion temps spike)
	if (EngineState.bNitrousActive)
	{
		HeatGenerated += 8.0f * DeltaTime;
	}

	// Rev limiter generates extra heat (fuel cut causes hot exhaust)
	if (EngineState.bRevLimiterActive)
	{
		HeatGenerated += 3.0f * DeltaTime;
	}

	// Cooling from radiator/airflow
	// Cooling is based on vehicle speed (airflow) and temperature delta
	const float AirflowCooling = FMath::Clamp(Speed / 60.0f, 0.2f, 1.5f);
	const float RadiatorEfficiency = 1.0f; // Could be modified by radiator upgrades
	const float TargetTemp = 90.0f; // Optimal operating temperature
	const float TempDelta = EngineState.EngineTemperature - AmbientTemperature;

	// Cooling rate increases with temperature delta
	const float CoolingRate = 3.0f * RadiatorEfficiency * AirflowCooling;
	const float HeatDissipated = CoolingRate * (TempDelta / 100.0f) * DeltaTime;

	// Apply temperature changes
	EngineState.EngineTemperature += HeatGenerated - HeatDissipated;

	// Clamp temperature to reasonable range
	EngineState.EngineTemperature = FMath::Clamp(EngineState.EngineTemperature, AmbientTemperature, 150.0f);

	// Update overheating status
	const float OverheatThreshold = 115.0f;
	const float CriticalTemp = 130.0f;
	EngineState.bOverheating = EngineState.EngineTemperature >= OverheatThreshold;

	// Apply power reduction when overheating
	// Gradual power loss from 115C to 130C, with more severe loss above 130C
	if (EngineState.bOverheating)
	{
		if (EngineState.EngineTemperature >= CriticalTemp)
		{
			// Critical temperature - severe power reduction to protect engine
			// Would trigger limp mode in a real vehicle
			UE_LOG(LogTemp, Warning, TEXT("Engine critical temperature: %.1fC - Power reduced"),
				EngineState.EngineTemperature);
		}
	}
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
		// Apply corrective yaw torque through the mesh component
		if (UPrimitiveComponent* MeshPrimitive = Cast<UPrimitiveComponent>(UpdatedPrimitive))
		{
			const float CorrectionStrength = SlipAngle * StabilityControl * 10000.0f;
			const float CorrectionDir = DriftState.DriftAngle > 0 ? -1.0f : 1.0f;

			// Apply yaw correction torque in world space
			const FVector TorqueToApply = FVector(0.0f, 0.0f, CorrectionStrength * CorrectionDir * DeltaTime);
			MeshPrimitive->AddTorqueInRadians(TorqueToApply, NAME_None, true);
		}
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
		// Apply anti-flip torque through the mesh component
		if (UPrimitiveComponent* MeshPrimitive = Cast<UPrimitiveComponent>(UpdatedPrimitive))
		{
			const float FlipDir = Rotation.Roll > 0 ? -1.0f : 1.0f;

			// Apply roll correction torque in local space
			const FVector LocalTorque = FVector(AntiFlipTorque * FlipDir * DeltaTime, 0.0f, 0.0f);
			const FVector WorldTorque = GetOwner()->GetActorRotation().RotateVector(LocalTorque);
			MeshPrimitive->AddTorqueInRadians(WorldTorque, NAME_None, true);
		}
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

	// NEW: Apply surface type grip multiplier
	if (WheelIndex >= 0 && WheelIndex < 4)
	{
		const EMGSurfaceType SurfaceType = WheelSurfaceStates[WheelIndex].SurfaceType;
		const float SurfaceGrip = GetSurfaceGripMultiplier(SurfaceType);
		BaseFriction *= SurfaceGrip;

		// Additional wetness modifier for surfaces that can be wet
		const float Wetness = WheelSurfaceStates[WheelIndex].WetnessLevel;
		if (Wetness > 0.0f && SurfaceType != EMGSurfaceType::Ice && SurfaceType != EMGSurfaceType::Sand)
		{
			// Wetness further reduces grip (interpolate toward wet surface grip)
			const float DryGrip = SurfaceGrip;
			const float WetGrip = SurfaceGrip * 0.65f; // Wet reduces by 35%
			BaseFriction *= FMath::Lerp(1.0f, WetGrip / DryGrip, Wetness);
		}
	}

	// Apply tire temperature effects
	if (WheelIndex >= 0 && WheelIndex < 4)
	{
		const float TempGripMultiplier = TireTemperatures[WheelIndex].GetGripMultiplier();
		BaseFriction *= FMath::Lerp(1.0f, TempGripMultiplier, TireTempGripInfluence);
	}

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

	// Apply damage multiplier
	BaseFriction *= TireGripMultiplier;

	// Apply suspension wear effects (worn suspension reduces effective grip)
	BaseFriction *= PartWearEffects.SuspensionEfficiency;

	return BaseFriction;
}

float UMGVehicleMovementComponent::CalculateCurrentPower() const
{
	float Power = 0.0f;
	float Torque = 0.0f;

	// Sample power curve at current RPM for realistic power delivery
	SamplePowerCurve(EngineState.CurrentRPM, Power, Torque);

	// Store current dyno values in engine state (const_cast for caching)
	const_cast<FMGEngineState&>(EngineState).CurrentHorsepower = Power;
	const_cast<FMGEngineState&>(EngineState).CurrentTorque = Torque;

	// Apply boost multiplier with forced induction efficiency
	if (EngineState.CurrentBoostPSI > 0.0f)
	{
		float BoostMultiplier = 1.0f + (EngineState.CurrentBoostPSI / 20.0f); // ~5% per PSI

		// Apply forced induction wear (affects boost effectiveness)
		BoostMultiplier = 1.0f + (BoostMultiplier - 1.0f) * PartWearEffects.ForcedInductionEfficiency;

		Power *= BoostMultiplier;
	}

	// Apply nitrous
	if (EngineState.bNitrousActive)
	{
		Power *= NitrousPowerMultiplier;
	}

	// Apply part wear effects from the wear system
	Power *= PartWearEffects.EngineEfficiency;
	Power *= PartWearEffects.DrivetrainEfficiency;

	// Apply overheating penalty
	if (EngineState.bOverheating)
	{
		const float OverheatThreshold = 115.0f;
		const float CriticalTemp = 130.0f;

		if (EngineState.EngineTemperature >= CriticalTemp)
		{
			// Critical - severe power loss (limp mode)
			Power *= 0.5f;
		}
		else
		{
			// Gradual power loss between overheat threshold and critical
			const float OverheatProgress = (EngineState.EngineTemperature - OverheatThreshold) / (CriticalTemp - OverheatThreshold);
			Power *= FMath::Lerp(1.0f, 0.5f, OverheatProgress);
		}
	}

	// Apply max speed multiplier (from damage system)
	Power *= MaxSpeedMultiplier;

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

// ==========================================
// ADVANCED PHYSICS SYSTEMS
// ==========================================

void UMGVehicleMovementComponent::UpdateTireTemperatures(float DeltaTime)
{
	const float SpeedMPH = GetSpeedMPH();

	for (int32 i = 0; i < 4 && i < WheelSetups.Num(); ++i)
	{
		FMGTireTemperature& Temp = TireTemperatures[i];
		const float SlipAmount = GetWheelSlipRatio(i) + FMath::Abs(GetWheelSlipAngle(i)) / 90.0f;

		// Heat generation from slip
		const float HeatGenerated = SlipAmount * TireHeatRate * DeltaTime;

		// Cooling from ambient and airflow
		const float AirflowCooling = (SpeedMPH / 100.0f) * TireCoolRate * 0.5f;
		const float TotalCooling = (TireCoolRate + AirflowCooling) * DeltaTime;

		// Calculate temperature differential across tire width
		const float CamberEffect = (i < 2) ?
			CurrentConfiguration.Suspension.FrontCamber :
			CurrentConfiguration.Suspension.RearCamber;

		// Apply temperature changes
		const float AverageHeat = HeatGenerated / 3.0f;
		Temp.InnerTemp += AverageHeat * (1.0f + CamberEffect * 0.1f);
		Temp.MiddleTemp += AverageHeat;
		Temp.OuterTemp += AverageHeat * (1.0f - CamberEffect * 0.1f);

		// Cooling toward ambient
		const float CoolFactor = TotalCooling / 3.0f;
		Temp.InnerTemp -= (Temp.InnerTemp - AmbientTemperature) * CoolFactor * 0.01f;
		Temp.MiddleTemp -= (Temp.MiddleTemp - AmbientTemperature) * CoolFactor * 0.01f;
		Temp.OuterTemp -= (Temp.OuterTemp - AmbientTemperature) * CoolFactor * 0.01f;

		// Clamp temperatures
		Temp.InnerTemp = FMath::Clamp(Temp.InnerTemp, -20.0f, 200.0f);
		Temp.MiddleTemp = FMath::Clamp(Temp.MiddleTemp, -20.0f, 200.0f);
		Temp.OuterTemp = FMath::Clamp(Temp.OuterTemp, -20.0f, 200.0f);
	}
}

void UMGVehicleMovementComponent::UpdateWeightTransfer(float DeltaTime)
{
	if (!GetOwner())
	{
		return;
	}

	// Get acceleration in local space
	const FVector Velocity = GetOwner()->GetVelocity();
	const FVector Acceleration = (Velocity - LastFrameVelocity) / FMath::Max(DeltaTime, 0.001f);
	LastFrameVelocity = Velocity;

	const FVector LocalAccel = GetOwner()->GetActorTransform().InverseTransformVector(Acceleration);

	// Calculate target weight transfer
	// Positive X = accelerating forward = weight shifts rear
	// Positive Y = accelerating right = weight shifts left
	const float TargetLongitudinal = -LocalAccel.X * LongitudinalTransferFactor * 0.0001f;
	const float TargetLateral = -LocalAccel.Y * LateralTransferFactor * 0.0001f;

	// Smooth interpolation
	WeightTransferState.LongitudinalTransfer = FMath::FInterpTo(
		WeightTransferState.LongitudinalTransfer,
		FMath::Clamp(TargetLongitudinal, -1.0f, 1.0f),
		DeltaTime,
		WeightTransferRate
	);

	WeightTransferState.LateralTransfer = FMath::FInterpTo(
		WeightTransferState.LateralTransfer,
		FMath::Clamp(TargetLateral, -1.0f, 1.0f),
		DeltaTime,
		WeightTransferRate
	);
}

void UMGVehicleMovementComponent::UpdateAerodynamics(float DeltaTime)
{
	const float SpeedMPS = GetSpeedMPH() * 0.44704f; // Convert to m/s
	const float AirDensity = 1.225f; // kg/m³ at sea level

	// Downforce = 0.5 * rho * v² * Cl * A
	const float DynamicPressure = 0.5f * AirDensity * SpeedMPS * SpeedMPS;

	// Base downforce from configuration
	float TotalDownforceCoef = DownforceCoefficient;

	// Add aero parts contribution
	if (CurrentConfiguration.Aero.FrontSplitter.bInstalled)
	{
		TotalDownforceCoef += CurrentConfiguration.Aero.FrontSplitter.DownforceCoefficient *
			(CurrentConfiguration.Aero.FrontSplitter.DownforceLevelPercent / 100.0f);
	}

	if (CurrentConfiguration.Aero.RearWing.bInstalled)
	{
		TotalDownforceCoef += CurrentConfiguration.Aero.RearWing.DownforceCoefficient *
			(CurrentConfiguration.Aero.RearWing.DownforceLevelPercent / 100.0f);
	}

	TotalDownforceCoef += CurrentConfiguration.Aero.DiffuserDownforceCoefficient;

	// Calculate total downforce in Newtons
	CurrentDownforceN = DynamicPressure * TotalDownforceCoef * FrontalArea;

	// Apply downforce to vehicle physics
	// Increases grip at high speed but also increases tire wear
	if (CurrentDownforceN > 100.0f && GetOwner())
	{
		if (UPrimitiveComponent* MeshPrimitive = Cast<UPrimitiveComponent>(UpdatedPrimitive))
		{
			// Apply force downward in world space
			const FVector DownforceVector = FVector(0.0f, 0.0f, -CurrentDownforceN * DeltaTime * 50.0f);
			MeshPrimitive->AddForce(DownforceVector, NAME_None, true);
		}
	}

	// Calculate drag (reduces top speed)
	const float DragForce = DynamicPressure * DragCoefficient * FrontalArea;
	if (DragForce > 50.0f && GetOwner() && SpeedMPS > 10.0f)
	{
		if (UPrimitiveComponent* MeshPrimitive = Cast<UPrimitiveComponent>(UpdatedPrimitive))
		{
			const FVector VelocityDir = GetOwner()->GetVelocity().GetSafeNormal();
			const FVector DragVector = -VelocityDir * DragForce * DeltaTime * 10.0f;
			MeshPrimitive->AddForce(DragVector, NAME_None, true);
		}
	}
}

void UMGVehicleMovementComponent::UpdateAntiLag(float DeltaTime)
{
	const FMGForcedInductionConfig& FI = CurrentConfiguration.Engine.ForcedInduction;

	// Only for turbo vehicles with anti-lag enabled
	if (FI.Type != EMGForcedInductionType::Turbo_Single &&
		FI.Type != EMGForcedInductionType::Turbo_Twin)
	{
		EngineState.bAntiLagActive = false;
		return;
	}

	if (!bAntiLagEnabled)
	{
		EngineState.bAntiLagActive = false;
		return;
	}

	// Anti-lag activates when off throttle but RPM is high enough
	const bool bConditionsMet = EngineState.ThrottlePosition < 0.3f &&
		EngineState.CurrentRPM >= AntiLagMinRPM &&
		CurrentGear > 0;

	EngineState.bAntiLagActive = bConditionsMet;

	if (EngineState.bAntiLagActive)
	{
		// Maintain boost pressure when off throttle
		EngineState.BoostBuildupPercent = FMath::FInterpTo(
			EngineState.BoostBuildupPercent,
			AntiLagBoostRetention,
			DeltaTime,
			BoostBuildupRate * 0.5f
		);

		EngineState.CurrentBoostPSI = FI.MaxBoostPSI * EngineState.BoostBuildupPercent;
	}
}

void UMGVehicleMovementComponent::UpdateLaunchControl(float DeltaTime)
{
	if (!EngineState.bLaunchControlEngaged)
	{
		return;
	}

	// Launch control holds RPM at target with boost building
	const float TargetRPM = EngineState.LaunchControlRPM;
	EngineState.CurrentRPM = FMath::FInterpTo(EngineState.CurrentRPM, TargetRPM, DeltaTime, 15.0f);

	// Build boost while stationary
	const FMGForcedInductionConfig& FI = CurrentConfiguration.Engine.ForcedInduction;
	if (FI.Type == EMGForcedInductionType::Turbo_Single ||
		FI.Type == EMGForcedInductionType::Turbo_Twin)
	{
		EngineState.BoostBuildupPercent = FMath::FInterpTo(
			EngineState.BoostBuildupPercent,
			1.0f,
			DeltaTime,
			BoostBuildupRate * LaunchControlBoostBuild
		);
		EngineState.CurrentBoostPSI = FI.MaxBoostPSI * EngineState.BoostBuildupPercent;
	}

	LaunchControlTimer += DeltaTime;
}

void UMGVehicleMovementComponent::UpdateBrakeSystem(float DeltaTime)
{
	// Get brake input from parent class
	const float BrakeInput = GetBrakeInput();

	// Heat generation from braking
	if (BrakeInput > 0.1f)
	{
		const float SpeedFactor = FMath::Clamp(GetSpeedMPH() / 60.0f, 0.0f, 1.5f);
		const float HeatGen = BrakeHeatRate * BrakeInput * SpeedFactor * DeltaTime;
		EngineState.BrakeTemperature += HeatGen;
	}

	// Cooling
	const float SpeedCooling = (GetSpeedMPH() / 100.0f) * BrakeCoolRate * 0.3f;
	const float TotalCooling = (BrakeCoolRate + SpeedCooling) * DeltaTime;
	EngineState.BrakeTemperature -= (EngineState.BrakeTemperature - AmbientTemperature) * TotalCooling * 0.01f;

	// Clamp temperature
	EngineState.BrakeTemperature = FMath::Clamp(EngineState.BrakeTemperature, AmbientTemperature, 800.0f);

	// Calculate brake fade
	if (EngineState.BrakeTemperature <= BrakeFadeStartTemp)
	{
		EngineState.BrakeFadeMultiplier = 1.0f;
	}
	else if (EngineState.BrakeTemperature >= BrakeFadeMaxTemp)
	{
		EngineState.BrakeFadeMultiplier = BrakeFadeMinEfficiency;
	}
	else
	{
		// Linear interpolation between fade start and max
		const float FadeProgress = (EngineState.BrakeTemperature - BrakeFadeStartTemp) /
			(BrakeFadeMaxTemp - BrakeFadeStartTemp);
		EngineState.BrakeFadeMultiplier = FMath::Lerp(1.0f, BrakeFadeMinEfficiency, FadeProgress);
	}

	// Apply brake pad wear effect (worn pads have reduced stopping power)
	EngineState.BrakeFadeMultiplier *= PartWearEffects.BrakePadEfficiency;
}

void UMGVehicleMovementComponent::ApplyDifferentialBehavior(float DeltaTime)
{
	if (!IsGrounded() || WheelSetups.Num() < 4)
	{
		return;
	}

	const EMGDifferentialType DiffType = CurrentConfiguration.Drivetrain.DifferentialType;
	const float LockFactor = GetDifferentialLockFactor();

	// Apply differential behavior based on type
	// This affects how power is distributed between driven wheels
	switch (CurrentConfiguration.Drivetrain.DrivetrainType)
	{
		case EMGDrivetrainType::RWD:
		{
			// Rear wheel drive - differential affects wheels 2 and 3
			const float LeftSlip = GetWheelSlipRatio(2);
			const float RightSlip = GetWheelSlipRatio(3);

			if (DiffType == EMGDifferentialType::LSD_2Way ||
				DiffType == EMGDifferentialType::Locked)
			{
				// Locked diff tends to cause understeer on power
				// but better for acceleration
				if (DriftState.bIsDrifting)
				{
					// Help maintain drift angle
					const float DriftCorrection = DriftState.DriftAngle * 0.01f * LockFactor;
					// Would apply torque correction here
				}
			}
			break;
		}

		case EMGDrivetrainType::FWD:
		{
			// Front wheel drive - torque steer simulation
			if (EngineState.ThrottlePosition > 0.7f && GetSpeedMPH() < 40.0f)
			{
				// Torque steer pulls toward stronger grip side
				// LSD reduces this effect
				const float TorqueSteerAmount = (1.0f - LockFactor * 0.5f) * 0.05f;
				TargetSteering += TorqueSteerAmount * FMath::Sign(TargetSteering + 0.001f);
			}
			break;
		}

		case EMGDrivetrainType::AWD:
		{
			// All wheel drive - power split behavior
			// Center diff behavior would go here
			break;
		}
	}
}

FMGTireTemperature UMGVehicleMovementComponent::GetTireTemperature(int32 WheelIndex) const
{
	if (WheelIndex >= 0 && WheelIndex < 4)
	{
		return TireTemperatures[WheelIndex];
	}
	return FMGTireTemperature();
}

float UMGVehicleMovementComponent::GetCurrentDownforce() const
{
	return CurrentDownforceN;
}

bool UMGVehicleMovementComponent::IsLaunchControlAvailable() const
{
	// Available when:
	// - Stationary or very slow
	// - In first gear
	// - Brake held
	return GetSpeedMPH() < 5.0f &&
		CurrentGear == 1 &&
		GetBrakeInput() > 0.8f;
}

void UMGVehicleMovementComponent::EngageLaunchControl()
{
	if (!IsLaunchControlAvailable())
	{
		return;
	}

	EngineState.bLaunchControlEngaged = true;
	EngineState.LaunchControlRPM = LaunchControlDefaultRPM;
	LaunchControlTimer = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Launch control engaged at %d RPM"), FMath::RoundToInt(LaunchControlDefaultRPM));
}

void UMGVehicleMovementComponent::ReleaseLaunchControl()
{
	if (!EngineState.bLaunchControlEngaged)
	{
		return;
	}

	EngineState.bLaunchControlEngaged = false;

	// Apply clutch slip for smooth launch
	EngineState.ClutchEngagement = 1.0f - LaunchControlClutchSlip;

	UE_LOG(LogTemp, Log, TEXT("Launch control released after %.2f seconds"), LaunchControlTimer);
}

void UMGVehicleMovementComponent::SetAntiLagEnabled(bool bEnabled)
{
	bAntiLagEnabled = bEnabled;

	if (bEnabled)
	{
		UE_LOG(LogTemp, Log, TEXT("Anti-lag system enabled"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Anti-lag system disabled"));
	}
}

void UMGVehicleMovementComponent::SetClutchInput(float Value)
{
	ClutchInput = FMath::Clamp(Value, 0.0f, 1.0f);
	EngineState.ClutchEngagement = ClutchInput;
}

float UMGVehicleMovementComponent::CalculateSpeedSteeringFactor() const
{
	const float SpeedFactor = FMath::Clamp(GetSpeedMPH() / 120.0f, 0.0f, 1.0f);

	// Non-linear reduction - more sensitive at higher speeds
	const float Reduction = FMath::Pow(SpeedFactor, 1.5f) * SpeedSensitiveSteeringFactor;

	// Apply steering wear effect (worn steering is less precise)
	const float SteeringPrecision = PartWearEffects.SteeringPrecision;

	// Worn steering adds slight imprecision (reduced responsiveness)
	return (1.0f - Reduction) * SteeringPrecision;
}

float UMGVehicleMovementComponent::GetDifferentialLockFactor() const
{
	switch (CurrentConfiguration.Drivetrain.DifferentialType)
	{
		case EMGDifferentialType::Open:
			return 0.0f;
		case EMGDifferentialType::LSD_1Way:
			return 0.3f;
		case EMGDifferentialType::LSD_1_5Way:
			return 0.5f;
		case EMGDifferentialType::LSD_2Way:
			return 0.7f;
		case EMGDifferentialType::Torsen:
			return 0.6f;
		case EMGDifferentialType::Locked:
			return 1.0f;
		default:
			return 0.0f;
	}
}

// ==========================================
// SURFACE DETECTION SYSTEM
// ==========================================

float UMGVehicleMovementComponent::GetSurfaceGripMultiplier(EMGSurfaceType SurfaceType) const
{
	switch (SurfaceType)
	{
		case EMGSurfaceType::Asphalt:  return SurfaceGrip_Asphalt;
		case EMGSurfaceType::Concrete: return SurfaceGrip_Concrete;
		case EMGSurfaceType::Wet:      return SurfaceGrip_Wet;
		case EMGSurfaceType::Dirt:     return SurfaceGrip_Dirt;
		case EMGSurfaceType::Gravel:   return SurfaceGrip_Gravel;
		case EMGSurfaceType::Ice:      return SurfaceGrip_Ice;
		case EMGSurfaceType::Snow:     return SurfaceGrip_Snow;
		case EMGSurfaceType::Grass:    return SurfaceGrip_Grass;
		case EMGSurfaceType::Sand:     return SurfaceGrip_Sand;
		case EMGSurfaceType::OffRoad:  return SurfaceGrip_OffRoad;
		default:                        return 1.0f;
	}
}

EMGSurfaceType UMGVehicleMovementComponent::DetectWheelSurfaceType(int32 WheelIndex) const
{
	if (!GetOwner())
	{
		return EMGSurfaceType::Asphalt;
	}

	// Get wheel world location for trace
	// For now, we'll use the vehicle's location as approximation
	// In full implementation, you'd get actual wheel socket locations
	const FVector VehicleLocation = GetOwner()->GetActorLocation();
	const FVector TraceStart = VehicleLocation + FVector(0.0f, 0.0f, 50.0f);
	const FVector TraceEnd = VehicleLocation - FVector(0.0f, 0.0f, 200.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	// Perform line trace to detect surface
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		// Check physical material to determine surface type
		if (UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get())
		{
			// Surface type detection based on physical material properties
			// This is a simplified version - in production you'd use custom physical materials
			const float Friction = PhysMat->Friction;

			if (Friction < 0.3f)
			{
				return EMGSurfaceType::Ice;
			}
			else if (Friction < 0.5f)
			{
				return EMGSurfaceType::Wet;
			}
			else if (Friction < 0.7f)
			{
				return EMGSurfaceType::Dirt;
			}
			else if (Friction < 0.85f)
			{
				return EMGSurfaceType::Concrete;
			}
			else
			{
				return EMGSurfaceType::Asphalt;
			}
		}

		// Check surface name/tags for more specific detection
		if (AActor* HitActor = HitResult.GetActor())
		{
			// Check for surface tags (designers can tag surfaces)
			if (HitActor->Tags.Contains(FName("Surface_Wet")))
			{
				return EMGSurfaceType::Wet;
			}
			else if (HitActor->Tags.Contains(FName("Surface_Dirt")))
			{
				return EMGSurfaceType::Dirt;
			}
			else if (HitActor->Tags.Contains(FName("Surface_Gravel")))
			{
				return EMGSurfaceType::Gravel;
			}
			else if (HitActor->Tags.Contains(FName("Surface_Ice")))
			{
				return EMGSurfaceType::Ice;
			}
			else if (HitActor->Tags.Contains(FName("Surface_Snow")))
			{
				return EMGSurfaceType::Snow;
			}
			else if (HitActor->Tags.Contains(FName("Surface_Grass")))
			{
				return EMGSurfaceType::Grass;
			}
			else if (HitActor->Tags.Contains(FName("Surface_Sand")))
			{
				return EMGSurfaceType::Sand;
			}
			else if (HitActor->Tags.Contains(FName("Surface_OffRoad")))
			{
				return EMGSurfaceType::OffRoad;
			}
		}
	}

	// Default to asphalt if no surface detected
	return EMGSurfaceType::Asphalt;
}

void UMGVehicleMovementComponent::UpdateSurfaceDetection(float DeltaTime)
{
	// Update surface state for each wheel
	for (int32 WheelIdx = 0; WheelIdx < 4; ++WheelIdx)
	{
		FMGWheelSurfaceState& WheelSurface = WheelSurfaceStates[WheelIdx];

		// Detect current surface type
		const EMGSurfaceType NewSurfaceType = DetectWheelSurfaceType(WheelIdx);

		// Check if surface changed
		if (NewSurfaceType != WheelSurface.SurfaceType)
		{
			WheelSurface.SurfaceType = NewSurfaceType;
			WheelSurface.TimeOnSurface = 0.0f;

			// Log surface change for debugging
			//UE_LOG(LogTemp, VeryVerbose, TEXT("Wheel %d surface changed to %d"), WheelIdx, (int32)NewSurfaceType);
		}
		else
		{
			// Accumulate time on surface
			WheelSurface.TimeOnSurface += DeltaTime;
		}

		// Update wetness level based on weather system
		float TargetWetness = 0.0f;

		// Get weather subsystem for precipitation data
		if (UWorld* World = GetWorld())
		{
			if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
			{
				const FMGWeatherState& WeatherState = WeatherSubsystem->GetCurrentWeather();

				// Set target wetness based on road condition from weather
				switch (WeatherState.RoadCondition)
				{
					case EMGRoadCondition::Dry:
						TargetWetness = 0.0f;
						break;
					case EMGRoadCondition::Damp:
						TargetWetness = 0.3f;
						break;
					case EMGRoadCondition::Wet:
						TargetWetness = 0.7f;
						break;
					case EMGRoadCondition::StandingWater:
						TargetWetness = 1.0f;
						break;
					case EMGRoadCondition::Icy:
						TargetWetness = 0.2f; // Ice is slippery but not wet
						break;
					case EMGRoadCondition::Snowy:
						TargetWetness = 0.5f; // Snow melts under tires
						break;
					default:
						TargetWetness = 0.0f;
				}

				// Add precipitation contribution
				TargetWetness = FMath::Max(TargetWetness, WeatherState.Intensity.Precipitation);
			}
		}

		// Surface type can override wetness (e.g., explicitly wet surface)
		if (WheelSurface.SurfaceType == EMGSurfaceType::Wet)
		{
			TargetWetness = FMath::Max(TargetWetness, 0.8f);
		}

		// Interpolate wetness level for smooth transitions
		const float WetnessChangeRate = TargetWetness > WheelSurface.WetnessLevel ? 2.0f : 0.5f;
		WheelSurface.WetnessLevel = FMath::FInterpTo(WheelSurface.WetnessLevel, TargetWetness, DeltaTime, WetnessChangeRate);

		// Update contact state (simplified - full implementation would check actual wheel contact)
		WheelSurface.bHasContact = IsGrounded();
	}
}

// ==========================================
// POWER CURVE / DYNO INTEGRATION
// ==========================================

void UMGVehicleMovementComponent::SamplePowerCurve(float RPM, float& OutHorsepower, float& OutTorque) const
{
	const FMGPowerCurve& PowerCurve = CurrentConfiguration.PowerCurve;

	// Handle empty power curve - fall back to flat power model
	if (PowerCurve.CurvePoints.Num() == 0)
	{
		OutHorsepower = CurrentConfiguration.Stats.Horsepower;
		OutTorque = CurrentConfiguration.Stats.Torque;
		return;
	}

	// Clamp RPM to valid range
	const float ClampedRPM = FMath::Clamp(RPM, 800.0f, (float)PowerCurve.Redline);

	// Find the two curve points to interpolate between
	int32 LowerIndex = 0;
	int32 UpperIndex = 0;

	for (int32 i = 0; i < PowerCurve.CurvePoints.Num(); ++i)
	{
		if (PowerCurve.CurvePoints[i].RPM <= ClampedRPM)
		{
			LowerIndex = i;
		}
		if (PowerCurve.CurvePoints[i].RPM >= ClampedRPM)
		{
			UpperIndex = i;
			break;
		}
		UpperIndex = i; // Handle case where RPM is above all points
	}

	// If we're at exact point or only have one point
	if (LowerIndex == UpperIndex || PowerCurve.CurvePoints.Num() == 1)
	{
		OutHorsepower = PowerCurve.CurvePoints[LowerIndex].Horsepower;
		OutTorque = PowerCurve.CurvePoints[LowerIndex].Torque;
		return;
	}

	// Linear interpolation between points
	const FMGPowerCurvePoint& LowerPoint = PowerCurve.CurvePoints[LowerIndex];
	const FMGPowerCurvePoint& UpperPoint = PowerCurve.CurvePoints[UpperIndex];

	const float RPMRange = (float)(UpperPoint.RPM - LowerPoint.RPM);
	const float Alpha = (RPMRange > 0.0f) ? (ClampedRPM - LowerPoint.RPM) / RPMRange : 0.0f;

	OutHorsepower = FMath::Lerp(LowerPoint.Horsepower, UpperPoint.Horsepower, Alpha);
	OutTorque = FMath::Lerp(LowerPoint.Torque, UpperPoint.Torque, Alpha);
}

// ==========================================
// ENHANCED DRIFT SCORING SYSTEM
// ==========================================

EMGDriftAngleTier UMGVehicleMovementComponent::CalculateDriftAngleTier(float AbsAngle) const
{
	if (AbsAngle < DriftAngleThreshold)
	{
		return EMGDriftAngleTier::None;
	}
	else if (AbsAngle < DriftAngleTierMild)
	{
		return EMGDriftAngleTier::Mild;
	}
	else if (AbsAngle < DriftAngleTierStandard)
	{
		return EMGDriftAngleTier::Standard;
	}
	else if (AbsAngle < DriftAngleTierAggressive)
	{
		return EMGDriftAngleTier::Aggressive;
	}
	else if (AbsAngle < DriftAngleTierExtreme)
	{
		return EMGDriftAngleTier::Extreme;
	}
	else
	{
		return EMGDriftAngleTier::Insane;
	}
}

float UMGVehicleMovementComponent::GetDriftTierBonusMultiplier(EMGDriftAngleTier Tier) const
{
	switch (Tier)
	{
		case EMGDriftAngleTier::None:       return 0.0f;
		case EMGDriftAngleTier::Mild:       return 1.0f;
		case EMGDriftAngleTier::Standard:   return 1.5f;
		case EMGDriftAngleTier::Aggressive: return 2.0f;
		case EMGDriftAngleTier::Extreme:    return 3.0f;
		case EMGDriftAngleTier::Insane:     return 5.0f;
		default:                            return 1.0f;
	}
}

void UMGVehicleMovementComponent::UpdateDriftScoring(float DeltaTime)
{
	const float AbsDriftAngle = FMath::Abs(DriftState.DriftAngle);
	const bool bWasDrifting = DriftState.bIsDrifting;

	// Update drift status
	DriftState.bIsDrifting = AbsDriftAngle > DriftAngleThreshold;

	if (DriftState.bIsDrifting)
	{
		// Update drift duration
		DriftState.DriftDuration += DeltaTime;

		// Track peak angle for this drift
		if (AbsDriftAngle > DriftState.PeakAngle)
		{
			DriftState.PeakAngle = AbsDriftAngle;
		}

		// Update angle tier
		DriftState.CurrentAngleTier = CalculateDriftAngleTier(AbsDriftAngle);

		// Check for direction change (e-brake transitions, etc.)
		const float CurrentDirection = FMath::Sign(DriftState.DriftAngle);
		if (LastDriftDirection != 0.0f && CurrentDirection != LastDriftDirection)
		{
			DriftState.bDirectionChanged = true;
		}
		LastDriftDirection = CurrentDirection;

		// Build chain multiplier over time
		DriftChainBuildTimer += DeltaTime;
		if (DriftChainBuildTimer >= DriftChainBuildTime)
		{
			DriftChainBuildTimer = 0.0f;
			if (DriftState.ChainMultiplier < DriftMaxChainMultiplier)
			{
				DriftState.ChainMultiplier++;
			}
		}

		// Reset chain continuation timer
		DriftState.TimeSinceLastDrift = 0.0f;

		// Calculate score for this frame
		// Base score from time drifting
		float FrameScore = DriftBasePointsPerSecond * DeltaTime;

		// Angle bonus (more angle = more points)
		const float AngleBonusFactor = (AbsDriftAngle - DriftAngleThreshold) * DriftAngleBonusMultiplier * 0.01f;
		FrameScore *= (1.0f + AngleBonusFactor);

		// Speed bonus (faster = more points)
		const float SpeedFactor = FMath::Clamp(GetSpeedMPH() / 100.0f, 0.0f, 2.0f);
		FrameScore *= (1.0f + SpeedFactor * DriftSpeedBonusMultiplier * 0.5f);

		// Tier bonus
		const float TierBonus = GetDriftTierBonusMultiplier(DriftState.CurrentAngleTier);
		FrameScore *= TierBonus;

		// Direction change bonus
		if (DriftState.bDirectionChanged)
		{
			FrameScore *= DriftDirectionChangeBonusMultiplier;
		}

		// Apply chain multiplier
		FrameScore *= DriftState.ChainMultiplier;

		// Accumulate score
		DriftState.DriftScore += FrameScore;
		DriftState.ChainTotalScore += FrameScore;

		// Broadcast score update periodically (every 0.5 seconds of drift time)
		static float ScoreBroadcastAccumulator = 0.0f;
		ScoreBroadcastAccumulator += DeltaTime;
		if (ScoreBroadcastAccumulator >= 0.5f)
		{
			ScoreBroadcastAccumulator = 0.0f;
			AwardDriftScore(DriftState.DriftScore, TierBonus);
		}
	}
	else
	{
		// Not currently drifting
		if (bWasDrifting)
		{
			// Just ended a drift - award final score for this drift
			const float TierBonus = GetDriftTierBonusMultiplier(DriftState.CurrentAngleTier);
			AwardDriftScore(DriftState.DriftScore, TierBonus);

			// Increment drifts in chain
			DriftState.DriftsInChain++;

			// Reset single-drift tracking
			DriftState.DriftScore = 0.0f;
			DriftState.DriftDuration = 0.0f;
			DriftState.PeakAngle = 0.0f;
			DriftState.bDirectionChanged = false;
			DriftState.CurrentAngleTier = EMGDriftAngleTier::None;
			LastDriftDirection = 0.0f;
		}

		// Update chain continuation window
		DriftState.TimeSinceLastDrift += DeltaTime;

		// Check if chain should break
		if (DriftState.TimeSinceLastDrift > DriftChainContinuationWindow && DriftState.ChainMultiplier > 1)
		{
			BreakDriftChain();
		}
	}
}

void UMGVehicleMovementComponent::AwardDriftScore(float BaseScore, float AngleBonus)
{
	if (BaseScore > 0.0f)
	{
		OnDriftScoreAwarded.Broadcast(BaseScore, DriftState.ChainMultiplier, AngleBonus);
	}
}

void UMGVehicleMovementComponent::BreakDriftChain()
{
	// Broadcast chain broken with total score
	if (DriftState.ChainTotalScore > 0.0f)
	{
		OnDriftChainBroken.Broadcast(DriftState.ChainTotalScore);
	}

	// Reset chain state
	DriftState.ChainMultiplier = 1;
	DriftState.ChainTotalScore = 0.0f;
	DriftState.DriftsInChain = 0;
	DriftState.TimeSinceLastDrift = 0.0f;
	DriftChainBuildTimer = 0.0f;
}

// ==========================================
// ADVANCED TURBO SHAFT SIMULATION
// ==========================================

void UMGVehicleMovementComponent::UpdateTurboShaftSimulation(float DeltaTime)
{
	const FMGForcedInductionConfig& FI = CurrentConfiguration.Engine.ForcedInduction;

	// Only for turbo vehicles
	if (FI.Type != EMGForcedInductionType::Turbo_Single &&
		FI.Type != EMGForcedInductionType::Turbo_Twin)
	{
		EngineState.TurboState.ShaftRPM = 0.0f;
		EngineState.TurboState.CompressorEfficiency = 0.0f;
		return;
	}

	FMGTurboState& Turbo = EngineState.TurboState;

	// Calculate exhaust gas energy based on RPM, throttle, and engine load
	// Higher RPM and throttle = more exhaust energy = faster spool
	const float RPMFactor = EngineState.CurrentRPM / CurrentConfiguration.Stats.Redline;
	const float LoadFactor = EngineState.ThrottlePosition * EngineState.EngineLoad;

	// Exhaust gas temperature increases with load (affects spool rate)
	const float TargetEGT = 400.0f + (600.0f * RPMFactor * LoadFactor); // 400-1000C range
	Turbo.ExhaustGasTemp = FMath::FInterpTo(Turbo.ExhaustGasTemp, TargetEGT, DeltaTime, 5.0f);

	// Calculate exhaust flow energy (drives turbine)
	const float ExhaustEnergy = RPMFactor * LoadFactor * TurboExhaustFlowCoef;

	// Turbine wheel acceleration (shaft inertia affects response)
	// F = ma -> a = F/m, where higher inertia = lower acceleration
	const float InertiaFactor = 1.0f / FMath::Max(TurboShaftInertia, 0.1f);

	// Target shaft RPM based on exhaust energy and turbo size
	const float MaxShaftRPM = Turbo.MaxShaftRPM;
	const float TargetShaftRPM = ExhaustEnergy * MaxShaftRPM;

	// Apply spool-up/spool-down with inertia
	// Twin turbo spools slightly faster (smaller turbines)
	float SpoolRate = BoostBuildupRate * InertiaFactor;
	if (FI.Type == EMGForcedInductionType::Turbo_Twin)
	{
		SpoolRate *= 1.3f; // Twin turbos spool faster
	}

	// Spool up faster when on throttle, decay faster when off
	if (EngineState.ThrottlePosition > 0.3f && EngineState.CurrentRPM >= FI.BoostThresholdRPM)
	{
		Turbo.ShaftRPM = FMath::FInterpTo(Turbo.ShaftRPM, TargetShaftRPM, DeltaTime, SpoolRate);
	}
	else
	{
		// Coast down with natural friction
		Turbo.ShaftRPM = FMath::FInterpTo(Turbo.ShaftRPM, 0.0f, DeltaTime, BoostDecayRate * InertiaFactor);
	}

	// Calculate compressor efficiency based on operating point
	// Efficiency is best at mid-range, drops at extremes (surge/choke)
	const float ShaftRatio = Turbo.ShaftRPM / MaxShaftRPM;
	if (ShaftRatio < 0.3f)
	{
		// Below optimal - low efficiency
		Turbo.CompressorEfficiency = ShaftRatio * TurboCompressorPeakEfficiency / 0.3f;
	}
	else if (ShaftRatio < 0.8f)
	{
		// Optimal range
		Turbo.CompressorEfficiency = TurboCompressorPeakEfficiency;
	}
	else
	{
		// Approaching choke - efficiency drops
		Turbo.CompressorEfficiency = TurboCompressorPeakEfficiency * (1.0f - (ShaftRatio - 0.8f) * 0.5f);
	}

	// Check for compressor surge (high boost, low airflow)
	// Surge occurs when throttle is suddenly closed at high boost
	const float CurrentBoostRatio = EngineState.CurrentBoostPSI / FMath::Max(FI.MaxBoostPSI, 1.0f);
	Turbo.bInSurge = (CurrentBoostRatio > 0.7f && EngineState.ThrottlePosition < 0.2f && Turbo.ShaftRPM > MaxShaftRPM * 0.6f);

	if (Turbo.bInSurge)
	{
		// Surge causes boost fluctuation and efficiency loss
		Turbo.CompressorEfficiency *= 0.5f;
		// Could add audio/visual effects here
	}

	// Calculate actual boost from shaft RPM and efficiency
	const float BoostFromShaft = (Turbo.ShaftRPM / MaxShaftRPM) * FI.MaxBoostPSI * Turbo.CompressorEfficiency;

	// Apply backpressure effect (exhaust restrictions reduce boost)
	Turbo.BackpressureFactor = 1.0f; // Could be affected by exhaust mods
	const float FinalBoost = BoostFromShaft * Turbo.BackpressureFactor;

	// Update engine state boost (this replaces the simpler model in UpdateBoostSimulation)
	EngineState.CurrentBoostPSI = FinalBoost;
	EngineState.BoostBuildupPercent = Turbo.ShaftRPM / MaxShaftRPM;
}

// ==========================================
// PART WEAR EFFECTS SYSTEM
// ==========================================

void UMGVehicleMovementComponent::UpdatePartWearEffects()
{
	// Reset effects
	PartWearEffects = FMGPartWearEffects();

	const TMap<FName, float>& PartConditions = CurrentConfiguration.PartConditions;

	// Helper lambda to get condition with default
	auto GetCondition = [&PartConditions](const FName& PartName, float DefaultValue = 100.0f) -> float
	{
		if (const float* Condition = PartConditions.Find(PartName))
		{
			return *Condition;
		}
		return DefaultValue;
	};

	// Suspension wear affects damping and handling
	const float FrontSuspCondition = GetCondition(FName("FrontSuspension"));
	const float RearSuspCondition = GetCondition(FName("RearSuspension"));
	const float AvgSuspCondition = (FrontSuspCondition + RearSuspCondition) * 0.5f;
	// At 100% = 1.0 efficiency, at 0% = (1 - impact) efficiency
	PartWearEffects.SuspensionEfficiency = 1.0f - ((100.0f - AvgSuspCondition) / 100.0f * SuspensionWearHandlingImpact);

	// Brake wear affects stopping power
	const float FrontBrakeCondition = GetCondition(FName("FrontBrakes"));
	const float RearBrakeCondition = GetCondition(FName("RearBrakes"));
	const float AvgBrakeCondition = (FrontBrakeCondition + RearBrakeCondition) * 0.5f;
	PartWearEffects.BrakePadEfficiency = 1.0f - ((100.0f - AvgBrakeCondition) / 100.0f * BrakeWearStoppingImpact);

	// Steering wear affects responsiveness
	const float SteeringCondition = GetCondition(FName("Steering"));
	PartWearEffects.SteeringPrecision = 1.0f - ((100.0f - SteeringCondition) / 100.0f * SteeringWearPrecisionImpact);

	// Drivetrain wear affects power delivery
	const float ClutchCondition = GetCondition(FName("Clutch"));
	const float TransmissionCondition = GetCondition(FName("Transmission"));
	const float DifferentialCondition = GetCondition(FName("Differential"));
	const float AvgDrivetrainCondition = (ClutchCondition + TransmissionCondition + DifferentialCondition) / 3.0f;
	PartWearEffects.DrivetrainEfficiency = FMath::Lerp(0.85f, 1.0f, AvgDrivetrainCondition / 100.0f);

	// Engine wear affects power output
	const float EngineCondition = GetCondition(FName("Engine"));
	PartWearEffects.EngineEfficiency = FMath::Lerp(0.70f, 1.0f, EngineCondition / 100.0f);

	// Forced induction wear affects boost
	const float TurboCondition = GetCondition(FName("Turbo"));
	const float SuperchargerCondition = GetCondition(FName("Supercharger"));
	const float FICondition = FMath::Max(TurboCondition, SuperchargerCondition);
	PartWearEffects.ForcedInductionEfficiency = FMath::Lerp(0.60f, 1.0f, FICondition / 100.0f);

	// Broadcast warnings for critically worn parts
	for (const auto& PartEntry : PartConditions)
	{
		if (PartEntry.Value <= PartWearWarningThreshold && PartEntry.Value > 0.0f)
		{
			OnPartWearWarning.Broadcast(PartEntry.Key, PartEntry.Value);
		}
	}

	// Apply wear effects to handling
	ApplyPartWearToHandling();
}

void UMGVehicleMovementComponent::ApplyPartWearToHandling()
{
	// This method applies the wear effects to actual physics parameters
	// Called after UpdatePartWearEffects and when configuration changes

	// Note: Some effects are applied directly in CalculateTireFriction, CalculateCurrentPower, etc.
	// This method handles effects that need to modify component properties directly

	// Worn steering reduces effective steering speed
	// (Applied through CalculateSpeedSteeringFactor indirectly)

	// Worn suspension affects stability
	// (Applied through ApplyStabilityControl indirectly by reducing effectiveness)

	// Log wear status for debugging
	if (PartWearEffects.EngineEfficiency < 0.9f ||
		PartWearEffects.DrivetrainEfficiency < 0.95f ||
		PartWearEffects.BrakePadEfficiency < 0.9f)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Part wear affecting performance - Engine: %.0f%%, Drivetrain: %.0f%%, Brakes: %.0f%%"),
			PartWearEffects.EngineEfficiency * 100.0f,
			PartWearEffects.DrivetrainEfficiency * 100.0f,
			PartWearEffects.BrakePadEfficiency * 100.0f);
	}
}

// ==========================================
// CLUTCH WEAR SIMULATION
// ==========================================

void UMGVehicleMovementComponent::UpdateClutchWear(float DeltaTime)
{
	// Early out if clutch is already burnt out
	if (ClutchWearState.bIsBurntOut)
	{
		return;
	}

	// Calculate clutch slip amount
	// Slip occurs when clutch is partially engaged while there's RPM difference
	float ClutchSlip = 0.0f;
	bool bWasSlipping = ClutchWearState.bIsSlipping;

	if (ClutchInput < 1.0f && EngineState.CurrentRPM > 0.0f)
	{
		// Calculate expected wheel RPM based on gear and vehicle speed
		const float SpeedMPS = GetSpeedMPH() * 0.44704f;
		const float WheelRadius = 0.35f; // meters (typical)
		const float WheelRPM = (SpeedMPS / WheelRadius) * 60.0f / (2.0f * PI);

		// Get gear ratio (simplified - would normally come from transmission data)
		float GearRatio = 3.5f; // Default first gear
		if (CurrentGear > 0 && CurrentGear <= 6)
		{
			const float GearRatios[] = { 0.0f, 3.5f, 2.2f, 1.5f, 1.1f, 0.9f, 0.75f };
			GearRatio = GearRatios[CurrentGear];
		}

		const float FinalDrive = 3.9f;
		const float ExpectedEngineRPM = WheelRPM * GearRatio * FinalDrive;

		// Slip is the difference between actual and expected RPM
		const float RPMDifference = FMath::Abs(EngineState.CurrentRPM - ExpectedEngineRPM);
		ClutchSlip = (1.0f - ClutchInput) * (RPMDifference / EngineState.CurrentRPM);
		ClutchSlip = FMath::Clamp(ClutchSlip, 0.0f, 1.0f);
	}

	// Update slip state
	ClutchWearState.bIsSlipping = ClutchSlip > ClutchSlipDetectionThreshold;

	if (ClutchWearState.bIsSlipping)
	{
		ClutchWearState.CurrentSlipDuration += DeltaTime;
		ClutchWearState.SessionSlipDamage += ClutchSlip * DeltaTime;

		// Generate heat based on slip amount and engine torque
		float HeatGenerated = ClutchSlip * ClutchHeatRate * DeltaTime;

		// More heat if engine is producing high torque
		HeatGenerated *= (1.0f + EngineState.ThrottlePosition * 0.5f);

		// Engine RPM affects heat generation
		if (EngineState.CurrentRPM > 4000.0f)
		{
			HeatGenerated *= (1.0f + (EngineState.CurrentRPM - 4000.0f) / 4000.0f);
		}

		ClutchWearState.ClutchTemperature += HeatGenerated;

		// Accumulate wear
		float WearAccumulated = ClutchWearRate * ClutchSlip * DeltaTime;

		// Overheating accelerates wear
		if (ClutchWearState.ClutchTemperature > ClutchDegradeTemp)
		{
			WearAccumulated *= ClutchOverheatWearMultiplier;
		}

		ClutchWearState.WearLevel = FMath::Min(ClutchWearState.WearLevel + WearAccumulated, 1.0f);
	}
	else
	{
		ClutchWearState.CurrentSlipDuration = 0.0f;

		// Cool down when not slipping
		const float CoolAmount = ClutchCoolRate * DeltaTime;
		ClutchWearState.ClutchTemperature = FMath::Max(ClutchAmbientTemp, ClutchWearState.ClutchTemperature - CoolAmount);
	}

	// Update overheating state
	bool bWasOverheating = ClutchWearState.bIsOverheating;
	ClutchWearState.bIsOverheating = ClutchWearState.ClutchTemperature > ClutchDegradeTemp;

	// Broadcast overheating event
	if (ClutchWearState.bIsOverheating && !bWasOverheating)
	{
		OnClutchOverheating.Broadcast(ClutchWearState.ClutchTemperature, ClutchWearState.WearLevel);
	}

	// Check for burnout
	if (ClutchWearState.ClutchTemperature >= ClutchBurnoutTemp || ClutchWearState.WearLevel >= 1.0f)
	{
		ClutchWearState.bIsBurntOut = true;
		OnClutchBurnout.Broadcast();
	}

	// Update friction coefficient based on temperature and wear
	ClutchWearState.FrictionCoefficient = 1.0f;

	// Temperature reduces friction
	if (ClutchWearState.ClutchTemperature > ClutchDegradeTemp)
	{
		float HeatFactor = (ClutchWearState.ClutchTemperature - ClutchDegradeTemp) / (ClutchBurnoutTemp - ClutchDegradeTemp);
		ClutchWearState.FrictionCoefficient *= (1.0f - HeatFactor * 0.4f);
	}

	// Wear reduces friction
	ClutchWearState.FrictionCoefficient *= (1.0f - ClutchWearState.WearLevel * 0.3f);

	// Detect hard launches
	if (CurrentGear == 1 && !bWasSlipping && ClutchWearState.bIsSlipping)
	{
		if (EngineState.CurrentRPM > HardLaunchRPMThreshold)
		{
			ClutchWearState.HardLaunchCount++;

			// Hard launches cause extra wear
			ClutchWearState.WearLevel += 0.005f;
		}
	}

	// Update clutch engagement efficiency in engine state
	EngineState.ClutchEngagement = ClutchInput * ClutchWearState.GetTorqueTransferEfficiency();
}

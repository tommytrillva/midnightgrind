// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "SimpleVehicle/SimpleWheelSim.h"
#include "Environment/MGWeatherSubsystem.h"

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

	// ==========================================
	// OPTIMIZATION: Cache frequently used values
	// ==========================================
	TickFrameCounter++;
	CachedSpeedMPH = GetSpeedMPH();
	bIsVehicleMoving = CachedSpeedMPH > 1.0f;

	// Update grounded state periodically (not needed every frame)
	if ((TickFrameCounter % MediumUpdateInterval) == 0)
	{
		bCachedIsGrounded = IsGrounded();
	}

	// ==========================================
	// CORE SYSTEMS (every frame)
	// ==========================================
	UpdateEngineSimulation(DeltaTime);
	UpdateBoostSimulation(DeltaTime);

	// Turbo physics only if turbo is installed
	if (CurrentConfiguration.Engine.ForcedInduction.Type == EMGForcedInductionType::Turbo_Single ||
		CurrentConfiguration.Engine.ForcedInduction.Type == EMGForcedInductionType::Turbo_Twin)
	{
		UpdateTurboShaftSimulation(DeltaTime);
		UpdateAntiLag(DeltaTime);
	}

	// Drift physics only when moving
	if (bIsVehicleMoving)
	{
		UpdateDriftPhysics(DeltaTime);
		UpdateDriftScoring(DeltaTime);
		ApplyStabilityControl(DeltaTime);
	}

	// Nitrous only if installed and active
	if (CurrentConfiguration.Engine.Nitrous.bInstalled)
	{
		UpdateNitrousSystem(DeltaTime);
	}

	// Anti-flip only when airborne or on slopes
	if (!bCachedIsGrounded)
	{
		ApplyAntiFlipForce(DeltaTime);
	}

	// ==========================================
	// MEDIUM FREQUENCY UPDATES (every 2 frames)
	// ==========================================
	if ((TickFrameCounter % MediumUpdateInterval) == 0)
	{
		UpdateWeightTransfer(DeltaTime * MediumUpdateInterval);
		UpdateAerodynamics(DeltaTime * MediumUpdateInterval);
		UpdateBrakeSystem(DeltaTime * MediumUpdateInterval);
		ApplyDifferentialBehavior(DeltaTime * MediumUpdateInterval);
	}

	// Launch control only when stationary with throttle
	if (!bIsVehicleMoving && EngineState.ThrottlePosition > 0.5f)
	{
		UpdateLaunchControl(DeltaTime);
	}

	// ==========================================
	// SLOW UPDATES (every 5 frames)
	// Wear, temperatures, and slow-changing state
	// ==========================================
	if ((TickFrameCounter % SlowUpdateInterval) == 0)
	{
		UpdateTireTemperatures(DeltaTime * SlowUpdateInterval);
		UpdateSurfaceDetection(DeltaTime * SlowUpdateInterval);
		UpdateClutchWear(DeltaTime * SlowUpdateInterval);
		UpdateSuspensionGeometry(DeltaTime * SlowUpdateInterval);
		UpdateTirePressure(DeltaTime * SlowUpdateInterval);
	}

	// ==========================================
	// ALWAYS UPDATE
	// ==========================================

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
	// Use cached speed for performance (updated at start of tick)
	const float Speed = CachedSpeedMPH;

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

	// Apply suspension geometry effects (camber, toe, caster)
	if (WheelIndex >= 0 && WheelIndex < 4)
	{
		const float GeometryGripModifier = SuspensionGeometryEffects.WheelContactPatch[WheelIndex].CombinedGripModifier;
		BaseFriction *= GeometryGripModifier;
	}

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

	// Apply fuel starvation effect (from fuel consumption system)
	// Starvation reduces power delivery as fuel cannot reach the engine properly
	if (FuelStarvationMultiplier < 0.99f)
	{
		Power *= FuelStarvationMultiplier;

		// Severe starvation can cause misfires - add slight randomization
		if (FuelStarvationMultiplier < 0.5f)
		{
			// Random power fluctuations simulating misfires
			const float MisfireFactor = FMath::FRandRange(0.7f, 1.0f);
			Power *= MisfireFactor;
		}
	}

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
	const float SpeedMPH = CachedSpeedMPH;

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
	const float SpeedMPS = CachedSpeedMPH * 0.44704f; // Convert to m/s
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

// ==========================================
// REALISTIC DIFFERENTIAL SIMULATION
// ==========================================

void UMGVehicleMovementComponent::ApplyDifferentialBehavior(float DeltaTime)
{
	if (!IsGrounded() || WheelSetups.Num() < 4)
	{
		return;
	}

	// Update wheel angular velocities for differential calculations
	UpdateWheelAngularVelocities(DeltaTime);

	const EMGDifferentialType DiffType = CurrentConfiguration.Drivetrain.DifferentialType;
	const EMGDrivetrainType DriveType = CurrentConfiguration.Drivetrain.DrivetrainType;

	// Calculate input torque to drivetrain based on current power output
	const float EngineTorque = EngineState.CurrentTorque * ClutchWearState.GetTorqueTransferEfficiency();
	const float DrivetrainTorque = EngineTorque * PartWearEffects.DrivetrainEfficiency;

	// Determine if we're accelerating or decelerating (engine braking)
	const bool bIsAccelerating = EngineState.ThrottlePosition > 0.1f;

	// Simulate differentials based on drivetrain type
	switch (DriveType)
	{
		case EMGDrivetrainType::RWD:
		{
			// Rear wheel drive - only rear differential active
			SimulateDifferentialByType(
				DeltaTime,
				RearDiffState,
				DiffType,
				RearLSDConfig,
				2, 3,  // Rear left, rear right wheel indices
				DrivetrainTorque,
				bIsAccelerating);

			// Apply torque effects to rear wheels
			ApplyDifferentialTorqueToWheels(DeltaTime, RearDiffState, 2, 3);

			// Front diff is not driven in RWD
			FrontDiffState = FMGDifferentialState();
			break;
		}

		case EMGDrivetrainType::FWD:
		{
			// Front wheel drive - only front differential active
			SimulateDifferentialByType(
				DeltaTime,
				FrontDiffState,
				DiffType,
				FrontLSDConfig,
				0, 1,  // Front left, front right wheel indices
				DrivetrainTorque,
				bIsAccelerating);

			// Apply torque effects to front wheels
			ApplyDifferentialTorqueToWheels(DeltaTime, FrontDiffState, 0, 1);

			// Torque steer simulation for FWD
			if (EngineState.ThrottlePosition > 0.5f && GetSpeedMPH() < 50.0f)
			{
				// Torque steer pulls toward wheel with less grip
				// LSD reduces this effect by evening out torque distribution
				const float TorqueSteerReduction = FrontDiffState.LockPercent * 0.7f;
				const float TorqueSteerAmount = (1.0f - TorqueSteerReduction) * 0.03f * EngineState.ThrottlePosition;

				// Weight transfer affects which side pulls
				const float LateralBias = WeightTransferState.LateralTransfer;
				TargetSteering += TorqueSteerAmount * FMath::Sign(LateralBias + 0.001f);
			}

			// Rear diff is not driven in FWD
			RearDiffState = FMGDifferentialState();
			break;
		}

		case EMGDrivetrainType::AWD:
		{
			// All wheel drive - center diff splits torque front/rear, then axle diffs split left/right
			const float FrontTorque = DrivetrainTorque * AWDFrontBias;
			const float RearTorque = DrivetrainTorque * (1.0f - AWDFrontBias);

			// Simulate center differential
			SimulateCenterDifferential(DeltaTime, CenterDiffState, DrivetrainTorque, bIsAccelerating);

			// Simulate front differential
			SimulateDifferentialByType(
				DeltaTime,
				FrontDiffState,
				DiffType,
				FrontLSDConfig,
				0, 1,
				FrontTorque * CenterDiffState.LeftWheelTorqueRatio * 2.0f,
				bIsAccelerating);

			// Simulate rear differential
			SimulateDifferentialByType(
				DeltaTime,
				RearDiffState,
				DiffType,
				RearLSDConfig,
				2, 3,
				RearTorque * CenterDiffState.RightWheelTorqueRatio * 2.0f,
				bIsAccelerating);

			// Apply torque effects
			ApplyDifferentialTorqueToWheels(DeltaTime, FrontDiffState, 0, 1);
			ApplyDifferentialTorqueToWheels(DeltaTime, RearDiffState, 2, 3);
			break;
		}
	}

	// Integrate differential behavior with weight transfer
	IntegrateDifferentialWithWeightTransfer(DeltaTime);

	// Update power distribution visualization data
	UpdatePowerDistributionData(DeltaTime);

	// Broadcast lockup changes for UI/audio (with hysteresis)
	const float CurrentLockPercent = (DriveType == EMGDrivetrainType::FWD) ?
		FrontDiffState.LockPercent : RearDiffState.LockPercent;

	if (FMath::Abs(CurrentLockPercent - LastBroadcastLockPercent) > 0.15f)
	{
		LastBroadcastLockPercent = CurrentLockPercent;
		OnDifferentialLockup.Broadcast(CurrentLockPercent, bIsAccelerating);
	}
}

void UMGVehicleMovementComponent::SimulateDifferentialByType(
	float DeltaTime,
	FMGDifferentialState& OutState,
	EMGDifferentialType DiffType,
	const FMGLSDConfiguration& Config,
	int32 LeftWheelIdx,
	int32 RightWheelIdx,
	float InputTorque,
	bool bIsAccelerating)
{
	switch (DiffType)
	{
		case EMGDifferentialType::Open:
			SimulateOpenDifferential(DeltaTime, OutState, LeftWheelIdx, RightWheelIdx, InputTorque);
			break;

		case EMGDifferentialType::LSD_1Way:
			Simulate1WayLSD(DeltaTime, OutState, Config, LeftWheelIdx, RightWheelIdx, InputTorque, bIsAccelerating);
			break;

		case EMGDifferentialType::LSD_1_5Way:
			Simulate1Point5WayLSD(DeltaTime, OutState, Config, LeftWheelIdx, RightWheelIdx, InputTorque, bIsAccelerating);
			break;

		case EMGDifferentialType::LSD_2Way:
			Simulate2WayLSD(DeltaTime, OutState, Config, LeftWheelIdx, RightWheelIdx, InputTorque, bIsAccelerating);
			break;

		case EMGDifferentialType::Torsen:
			SimulateTorsenDifferential(DeltaTime, OutState, Config, LeftWheelIdx, RightWheelIdx, InputTorque);
			break;

		case EMGDifferentialType::Locked:
			SimulateLockedDifferential(DeltaTime, OutState, LeftWheelIdx, RightWheelIdx, InputTorque);
			break;
	}
}

void UMGVehicleMovementComponent::UpdateWheelAngularVelocities(float DeltaTime)
{
	for (int32 i = 0; i < 4 && i < WheelSetups.Num(); ++i)
	{
		if (PVehicleOutput && i < PVehicleOutput->Wheels.Num())
		{
			// Get wheel angular velocity from Chaos simulation
			const Chaos::FSimpleWheelSim& WheelSim = PVehicleOutput->Wheels[i];
			WheelAngularVelocities[i] = WheelSim.GetAngularVelocity();
		}
	}
}

void UMGVehicleMovementComponent::SimulateOpenDifferential(
	float DeltaTime,
	FMGDifferentialState& OutState,
	int32 LeftWheelIdx,
	int32 RightWheelIdx,
	float InputTorque)
{
	// Open differential: torque goes to path of least resistance
	// This means the wheel with less grip spins freely
	OutState.InputTorque = InputTorque;
	OutState.LockPercent = 0.0f;
	OutState.AccelLockPercent = 0.0f;
	OutState.DecelLockPercent = 0.0f;
	OutState.bIsLocking = false;
	OutState.ActivePreloadTorque = 0.0f;

	// Get wheel speeds
	const float LeftSpeed = WheelAngularVelocities[LeftWheelIdx];
	const float RightSpeed = WheelAngularVelocities[RightWheelIdx];

	OutState.LeftWheelAngularVelocity = LeftSpeed;
	OutState.RightWheelAngularVelocity = RightSpeed;
	OutState.WheelSpeedDifferential = LeftSpeed - RightSpeed;
	OutState.NormalizedSpeedDiff = FMath::Clamp(OutState.WheelSpeedDifferential / 10.0f, -1.0f, 1.0f);

	// Get grip/load for each wheel
	const float LeftGrip = CalculateTireFriction(LeftWheelIdx) * WeightTransferState.GetWheelLoadMultiplier(LeftWheelIdx);
	const float RightGrip = CalculateTireFriction(RightWheelIdx) * WeightTransferState.GetWheelLoadMultiplier(RightWheelIdx);
	const float TotalGrip = LeftGrip + RightGrip;

	// Open diff distributes torque inversely proportional to grip
	// Less grip = more torque (wheel spins)
	if (TotalGrip > KINDA_SMALL_NUMBER)
	{
		// Inverse grip distribution - open diff sends torque to spinning wheel
		const float LeftRatio = RightGrip / TotalGrip;  // Inverse!
		const float RightRatio = LeftGrip / TotalGrip;

		// Check for significant speed differential (inside wheel spinning)
		if (FMath::Abs(OutState.WheelSpeedDifferential) > OpenDiffSpinThreshold)
		{
			// Amplify the bias toward spinning wheel
			const float SpinBias = FMath::Clamp(FMath::Abs(OutState.WheelSpeedDifferential) / 5.0f, 1.0f, 3.0f);
			if (LeftSpeed > RightSpeed)
			{
				// Left spinning more - send more torque to left (open diff behavior)
				OutState.LeftWheelTorqueRatio = FMath::Clamp(LeftRatio * SpinBias, 0.2f, 0.9f);
				OutState.RightWheelTorqueRatio = 1.0f - OutState.LeftWheelTorqueRatio;
			}
			else
			{
				OutState.RightWheelTorqueRatio = FMath::Clamp(RightRatio * SpinBias, 0.2f, 0.9f);
				OutState.LeftWheelTorqueRatio = 1.0f - OutState.RightWheelTorqueRatio;
			}
		}
		else
		{
			// Normal distribution
			OutState.LeftWheelTorqueRatio = LeftRatio;
			OutState.RightWheelTorqueRatio = RightRatio;
		}
	}
	else
	{
		// Equal split fallback
		OutState.LeftWheelTorqueRatio = 0.5f;
		OutState.RightWheelTorqueRatio = 0.5f;
	}

	OutState.BiasTorque = 0.0f;
}

void UMGVehicleMovementComponent::Simulate1WayLSD(
	float DeltaTime,
	FMGDifferentialState& OutState,
	const FMGLSDConfiguration& Config,
	int32 LeftWheelIdx,
	int32 RightWheelIdx,
	float InputTorque,
	bool bIsAccelerating)
{
	/**
	 * 1-Way LSD: Locks under acceleration ONLY
	 *
	 * Popular for drifting because:
	 * - On throttle: Both wheels drive together for traction
	 * - Off throttle: Acts like open diff, allows easy rotation
	 * - Easy to initiate and maintain drifts
	 */

	OutState.InputTorque = InputTorque;
	OutState.bUnderAcceleration = bIsAccelerating;

	// Get wheel speeds
	const float LeftSpeed = WheelAngularVelocities[LeftWheelIdx];
	const float RightSpeed = WheelAngularVelocities[RightWheelIdx];

	OutState.LeftWheelAngularVelocity = LeftSpeed;
	OutState.RightWheelAngularVelocity = RightSpeed;
	OutState.WheelSpeedDifferential = LeftSpeed - RightSpeed;
	OutState.NormalizedSpeedDiff = FMath::Clamp(OutState.WheelSpeedDifferential / 10.0f, -1.0f, 1.0f);

	float TargetLockPercent = 0.0f;
	float AccelLock = 0.0f;
	float DecelLock = 0.0f;

	if (bIsAccelerating && InputTorque > Config.MinSpeedDiffThreshold)
	{
		// Calculate acceleration lockup using ramp angle
		AccelLock = CalculateLSDLockup(
			InputTorque,
			Config.AccelRampAngleDeg,
			Config.PreloadTorqueNm,
			Config.MaxLockPercent,
			Config.ClutchFrictionCoef,
			Config.ClutchPlateCount);

		TargetLockPercent = AccelLock;
		OutState.bIsLocking = AccelLock > 0.05f;
	}
	else
	{
		// 1-way: NO lockup on deceleration
		DecelLock = 0.0f;
		TargetLockPercent = 0.0f;
		OutState.bIsLocking = false;
	}

	// Smooth transition to target lock
	OutState.LockPercent = FMath::FInterpTo(OutState.LockPercent, TargetLockPercent, DeltaTime, Config.LockResponseRate);
	OutState.AccelLockPercent = AccelLock;
	OutState.DecelLockPercent = DecelLock;
	OutState.ActivePreloadTorque = Config.PreloadTorqueNm;

	// Calculate torque distribution based on lockup
	CalculateTorqueDistribution(OutState, LeftWheelIdx, RightWheelIdx, InputTorque);
}

void UMGVehicleMovementComponent::Simulate1Point5WayLSD(
	float DeltaTime,
	FMGDifferentialState& OutState,
	const FMGLSDConfiguration& Config,
	int32 LeftWheelIdx,
	int32 RightWheelIdx,
	float InputTorque,
	bool bIsAccelerating)
{
	/**
	 * 1.5-Way LSD: Full lock on accel, partial lock on decel
	 *
	 * Good balance between:
	 * - Full acceleration traction (like 2-way)
	 * - Easier rotation on decel than 2-way (but not as free as 1-way)
	 * - More predictable trail braking behavior
	 */

	OutState.InputTorque = InputTorque;
	OutState.bUnderAcceleration = bIsAccelerating;

	// Get wheel speeds
	const float LeftSpeed = WheelAngularVelocities[LeftWheelIdx];
	const float RightSpeed = WheelAngularVelocities[RightWheelIdx];

	OutState.LeftWheelAngularVelocity = LeftSpeed;
	OutState.RightWheelAngularVelocity = RightSpeed;
	OutState.WheelSpeedDifferential = LeftSpeed - RightSpeed;
	OutState.NormalizedSpeedDiff = FMath::Clamp(OutState.WheelSpeedDifferential / 10.0f, -1.0f, 1.0f);

	float TargetLockPercent = 0.0f;
	float AccelLock = 0.0f;
	float DecelLock = 0.0f;

	// Calculate base lockup from torque/preload
	const float BaseLockup = CalculateLSDLockup(
		FMath::Abs(InputTorque),
		Config.AccelRampAngleDeg,
		Config.PreloadTorqueNm,
		Config.MaxLockPercent,
		Config.ClutchFrictionCoef,
		Config.ClutchPlateCount);

	if (bIsAccelerating)
	{
		// Full lockup on acceleration
		AccelLock = BaseLockup;
		TargetLockPercent = AccelLock;
	}
	else
	{
		// Partial lockup on deceleration (use coast factor)
		DecelLock = BaseLockup * Config.CoastLockFactor;

		// Also use the decel ramp angle for additional scaling
		const float DecelRampFactor = FMath::Tan(FMath::DegreesToRadians(Config.AccelRampAngleDeg)) /
			FMath::Max(FMath::Tan(FMath::DegreesToRadians(Config.DecelRampAngleDeg)), 0.1f);
		DecelLock *= FMath::Min(DecelRampFactor, 1.0f);

		TargetLockPercent = DecelLock;
	}

	OutState.bIsLocking = TargetLockPercent > 0.05f;

	// Smooth transition
	OutState.LockPercent = FMath::FInterpTo(OutState.LockPercent, TargetLockPercent, DeltaTime, Config.LockResponseRate);
	OutState.AccelLockPercent = AccelLock;
	OutState.DecelLockPercent = DecelLock;
	OutState.ActivePreloadTorque = Config.PreloadTorqueNm;

	// Calculate torque distribution
	CalculateTorqueDistribution(OutState, LeftWheelIdx, RightWheelIdx, InputTorque);
}

void UMGVehicleMovementComponent::Simulate2WayLSD(
	float DeltaTime,
	FMGDifferentialState& OutState,
	const FMGLSDConfiguration& Config,
	int32 LeftWheelIdx,
	int32 RightWheelIdx,
	float InputTorque,
	bool bIsAccelerating)
{
	/**
	 * 2-Way LSD: Equal lockup in both directions
	 *
	 * Most aggressive LSD type:
	 * - Maximum traction on acceleration
	 * - Also locks on deceleration (can cause push on entry)
	 * - Most stable but least forgiving
	 * - Good for grip racing, challenging for drifting
	 */

	OutState.InputTorque = InputTorque;
	OutState.bUnderAcceleration = bIsAccelerating;

	// Get wheel speeds
	const float LeftSpeed = WheelAngularVelocities[LeftWheelIdx];
	const float RightSpeed = WheelAngularVelocities[RightWheelIdx];

	OutState.LeftWheelAngularVelocity = LeftSpeed;
	OutState.RightWheelAngularVelocity = RightSpeed;
	OutState.WheelSpeedDifferential = LeftSpeed - RightSpeed;
	OutState.NormalizedSpeedDiff = FMath::Clamp(OutState.WheelSpeedDifferential / 10.0f, -1.0f, 1.0f);

	// 2-way uses same ramp angle for both directions
	const float EffectiveRampAngle = Config.AccelRampAngleDeg;

	const float Lockup = CalculateLSDLockup(
		FMath::Abs(InputTorque),
		EffectiveRampAngle,
		Config.PreloadTorqueNm,
		Config.MaxLockPercent,
		Config.ClutchFrictionCoef,
		Config.ClutchPlateCount);

	// Same lockup for both directions
	OutState.AccelLockPercent = Lockup;
	OutState.DecelLockPercent = Lockup;
	OutState.bIsLocking = Lockup > 0.05f;

	// Smooth transition
	OutState.LockPercent = FMath::FInterpTo(OutState.LockPercent, Lockup, DeltaTime, Config.LockResponseRate);
	OutState.ActivePreloadTorque = Config.PreloadTorqueNm;

	// Calculate torque distribution
	CalculateTorqueDistribution(OutState, LeftWheelIdx, RightWheelIdx, InputTorque);
}

void UMGVehicleMovementComponent::SimulateTorsenDifferential(
	float DeltaTime,
	FMGDifferentialState& OutState,
	const FMGLSDConfiguration& Config,
	int32 LeftWheelIdx,
	int32 RightWheelIdx,
	float InputTorque)
{
	/**
	 * Torsen (Torque-Sensing) Differential
	 *
	 * Uses worm gears instead of clutch packs:
	 * - Smooth, progressive torque biasing
	 * - Instant response (no clutch engagement delay)
	 * - Limited by Torque Bias Ratio (TBR)
	 * - Cannot transfer torque to wheel with zero traction
	 */

	OutState.InputTorque = InputTorque;
	OutState.bUnderAcceleration = EngineState.ThrottlePosition > 0.1f;

	// Get wheel speeds
	const float LeftSpeed = WheelAngularVelocities[LeftWheelIdx];
	const float RightSpeed = WheelAngularVelocities[RightWheelIdx];

	OutState.LeftWheelAngularVelocity = LeftSpeed;
	OutState.RightWheelAngularVelocity = RightSpeed;
	OutState.WheelSpeedDifferential = LeftSpeed - RightSpeed;
	OutState.NormalizedSpeedDiff = FMath::Clamp(OutState.WheelSpeedDifferential / 10.0f, -1.0f, 1.0f);

	// Get grip for each wheel
	const float LeftGrip = CalculateTireFriction(LeftWheelIdx) * WeightTransferState.GetWheelLoadMultiplier(LeftWheelIdx);
	const float RightGrip = CalculateTireFriction(RightWheelIdx) * WeightTransferState.GetWheelLoadMultiplier(RightWheelIdx);

	// Calculate grip ratio
	const float MinGrip = FMath::Min(LeftGrip, RightGrip);
	const float MaxGrip = FMath::Max(LeftGrip, RightGrip);
	const float GripRatio = (MinGrip > KINDA_SMALL_NUMBER) ? MaxGrip / MinGrip : Config.TorsenBiasRatio;

	// Torsen can only bias up to TBR
	const float EffectiveBiasRatio = FMath::Min(GripRatio, Config.TorsenBiasRatio);
	OutState.TorsenBiasRatio = EffectiveBiasRatio;

	// Calculate effective lock based on speed difference and TBR
	const float SpeedDiffMagnitude = FMath::Abs(OutState.WheelSpeedDifferential);
	const float SpeedDiffFactor = FMath::Clamp(SpeedDiffMagnitude * Config.TorsenSensitivity, 0.0f, 1.0f);

	// Torsen "lockup" is based on torque bias capability
	const float TorsenLock = ((EffectiveBiasRatio - 1.0f) / (Config.TorsenBiasRatio - 1.0f)) * SpeedDiffFactor;
	OutState.LockPercent = FMath::FInterpTo(OutState.LockPercent, TorsenLock, DeltaTime, Config.LockResponseRate * 2.0f);
	OutState.AccelLockPercent = OutState.LockPercent;
	OutState.DecelLockPercent = OutState.LockPercent;
	OutState.bIsLocking = OutState.LockPercent > 0.1f;
	OutState.ActivePreloadTorque = 0.0f; // Torsen has no preload

	// Calculate torque distribution based on grip and TBR
	if (LeftGrip < RightGrip && SpeedDiffMagnitude > Config.MinSpeedDiffThreshold)
	{
		// Left has less grip - bias torque to right (up to TBR limit)
		const float BiasAmount = FMath::Min(EffectiveBiasRatio, Config.TorsenBiasRatio);
		const float TotalRatio = 1.0f + BiasAmount;
		OutState.LeftWheelTorqueRatio = 1.0f / TotalRatio;
		OutState.RightWheelTorqueRatio = BiasAmount / TotalRatio;
	}
	else if (RightGrip < LeftGrip && SpeedDiffMagnitude > Config.MinSpeedDiffThreshold)
	{
		// Right has less grip - bias torque to left
		const float BiasAmount = FMath::Min(EffectiveBiasRatio, Config.TorsenBiasRatio);
		const float TotalRatio = 1.0f + BiasAmount;
		OutState.RightWheelTorqueRatio = 1.0f / TotalRatio;
		OutState.LeftWheelTorqueRatio = BiasAmount / TotalRatio;
	}
	else
	{
		// Equal grip or below threshold - equal torque split
		OutState.LeftWheelTorqueRatio = 0.5f;
		OutState.RightWheelTorqueRatio = 0.5f;
	}

	OutState.BiasTorque = InputTorque * FMath::Abs(OutState.LeftWheelTorqueRatio - OutState.RightWheelTorqueRatio);
}

void UMGVehicleMovementComponent::SimulateLockedDifferential(
	float DeltaTime,
	FMGDifferentialState& OutState,
	int32 LeftWheelIdx,
	int32 RightWheelIdx,
	float InputTorque)
{
	/**
	 * Locked/Welded Differential
	 *
	 * Both wheels ALWAYS rotate at same speed:
	 * - Maximum traction for straight-line acceleration
	 * - Very poor turning behavior (tire scrub)
	 * - Both wheels spin together or not at all
	 * - Common in drag racing, difficult for cornering
	 */

	OutState.InputTorque = InputTorque;
	OutState.bUnderAcceleration = EngineState.ThrottlePosition > 0.1f;

	// Get wheel speeds (should be nearly identical with locked diff)
	const float LeftSpeed = WheelAngularVelocities[LeftWheelIdx];
	const float RightSpeed = WheelAngularVelocities[RightWheelIdx];

	OutState.LeftWheelAngularVelocity = LeftSpeed;
	OutState.RightWheelAngularVelocity = RightSpeed;

	// With a truly locked diff, any speed difference is from tire slip
	OutState.WheelSpeedDifferential = LeftSpeed - RightSpeed;
	OutState.NormalizedSpeedDiff = FMath::Clamp(OutState.WheelSpeedDifferential / 10.0f, -1.0f, 1.0f);

	// Always 100% locked
	OutState.LockPercent = 1.0f;
	OutState.AccelLockPercent = 1.0f;
	OutState.DecelLockPercent = 1.0f;
	OutState.bIsLocking = true;
	OutState.ActivePreloadTorque = 0.0f;
	OutState.TorsenBiasRatio = FLT_MAX; // Infinite bias capability

	// Equal torque split (locked means 50/50)
	OutState.LeftWheelTorqueRatio = 0.5f;
	OutState.RightWheelTorqueRatio = 0.5f;

	// Calculate the bias torque needed to maintain equal speed (tire binding)
	// This torque works against the tires and causes understeer
	const float GripLeft = CalculateTireFriction(LeftWheelIdx) * WeightTransferState.GetWheelLoadMultiplier(LeftWheelIdx);
	const float GripRight = CalculateTireFriction(RightWheelIdx) * WeightTransferState.GetWheelLoadMultiplier(RightWheelIdx);
	OutState.BiasTorque = InputTorque * FMath::Abs(GripLeft - GripRight) / FMath::Max(GripLeft + GripRight, 0.001f);
}

void UMGVehicleMovementComponent::SimulateCenterDifferential(
	float DeltaTime,
	FMGDifferentialState& OutState,
	float InputTorque,
	bool bIsAccelerating)
{
	/**
	 * Center Differential (AWD systems)
	 *
	 * Distributes torque between front and rear axles.
	 * Uses the CenterLSDConfig for behavior.
	 */

	OutState.InputTorque = InputTorque;
	OutState.bUnderAcceleration = bIsAccelerating;

	// Calculate front/rear speed differential
	const float FrontAxleSpeed = (WheelAngularVelocities[0] + WheelAngularVelocities[1]) * 0.5f;
	const float RearAxleSpeed = (WheelAngularVelocities[2] + WheelAngularVelocities[3]) * 0.5f;

	OutState.LeftWheelAngularVelocity = FrontAxleSpeed;  // "Left" = Front for center diff
	OutState.RightWheelAngularVelocity = RearAxleSpeed;  // "Right" = Rear for center diff
	OutState.WheelSpeedDifferential = FrontAxleSpeed - RearAxleSpeed;
	OutState.NormalizedSpeedDiff = FMath::Clamp(OutState.WheelSpeedDifferential / 10.0f, -1.0f, 1.0f);

	// Calculate lockup based on center diff settings
	const float CenterLock = CalculateLSDLockup(
		FMath::Abs(InputTorque),
		CenterLSDConfig.AccelRampAngleDeg,
		CenterLSDConfig.PreloadTorqueNm,
		CenterLSDConfig.MaxLockPercent,
		CenterLSDConfig.ClutchFrictionCoef,
		CenterLSDConfig.ClutchPlateCount);

	OutState.LockPercent = FMath::FInterpTo(OutState.LockPercent, CenterLock, DeltaTime, CenterLSDConfig.LockResponseRate);
	OutState.bIsLocking = OutState.LockPercent > 0.1f;

	// Front/rear torque split based on AWDFrontBias and lockup
	// More lockup = closer to the base bias
	// Less lockup = can deviate more based on grip
	const float FrontGrip = (CalculateTireFriction(0) + CalculateTireFriction(1)) * 0.5f;
	const float RearGrip = (CalculateTireFriction(2) + CalculateTireFriction(3)) * 0.5f;
	const float TotalGrip = FrontGrip + RearGrip;

	if (TotalGrip > KINDA_SMALL_NUMBER && OutState.LockPercent < 0.9f)
	{
		// Blend between grip-based distribution and fixed bias based on lockup
		const float GripBasedFront = RearGrip / TotalGrip; // More rear grip = more front torque
		const float FinalFrontRatio = FMath::Lerp(GripBasedFront, AWDFrontBias, OutState.LockPercent);
		OutState.LeftWheelTorqueRatio = FinalFrontRatio;
		OutState.RightWheelTorqueRatio = 1.0f - FinalFrontRatio;
	}
	else
	{
		// Fully locked or no grip data - use base bias
		OutState.LeftWheelTorqueRatio = AWDFrontBias;
		OutState.RightWheelTorqueRatio = 1.0f - AWDFrontBias;
	}

	OutState.BiasTorque = InputTorque * FMath::Abs(OutState.LeftWheelTorqueRatio - 0.5f);
}

float UMGVehicleMovementComponent::CalculateLSDLockup(
	float InputTorque,
	float RampAngleDeg,
	float Preload,
	float MaxLock,
	float FrictionCoef,
	int32 PlateCount) const
{
	/**
	 * LSD Lockup Calculation using Ramp Angle
	 *
	 * The ramp angle determines how much the clutch plates are compressed
	 * for a given input torque. Lower angles = more aggressive compression.
	 *
	 * LockForce = InputTorque / tan(RampAngle)
	 * ClutchTorque = LockForce * FrictionCoef * PlateCount * MeanRadius
	 */

	if (InputTorque < Preload)
	{
		// Below preload - minimal lockup
		return FMath::Clamp(InputTorque / FMath::Max(Preload, 1.0f) * 0.1f, 0.0f, 0.1f);
	}

	// Calculate ramp force (lower angle = more force)
	const float RampAngleRad = FMath::DegreesToRadians(FMath::Clamp(RampAngleDeg, 20.0f, 89.0f));
	const float TanRamp = FMath::Tan(RampAngleRad);

	// Axial force from torque through ramp
	const float EffectiveTorque = InputTorque - Preload;
	const float AxialForce = EffectiveTorque / FMath::Max(TanRamp, 0.1f);

	// Clutch torque capacity based on friction and plate count
	const float MeanClutchRadius = 0.05f; // 5cm mean radius (typical for automotive)
	const float ClutchTorqueCapacity = AxialForce * FrictionCoef * PlateCount * MeanClutchRadius;

	// Lock percentage is ratio of clutch torque to input torque
	const float RawLockPercent = ClutchTorqueCapacity / FMath::Max(InputTorque, 1.0f);

	// Add preload contribution
	const float PreloadLock = Preload / 500.0f; // Normalized preload contribution

	// Final lock percentage
	return FMath::Clamp(RawLockPercent + PreloadLock, 0.0f, MaxLock);
}

void UMGVehicleMovementComponent::CalculateTorqueDistribution(
	FMGDifferentialState& OutState,
	int32 LeftWheelIdx,
	int32 RightWheelIdx,
	float InputTorque)
{
	// Get grip for each wheel
	const float LeftGrip = CalculateTireFriction(LeftWheelIdx) * WeightTransferState.GetWheelLoadMultiplier(LeftWheelIdx);
	const float RightGrip = CalculateTireFriction(RightWheelIdx) * WeightTransferState.GetWheelLoadMultiplier(RightWheelIdx);
	const float TotalGrip = LeftGrip + RightGrip;

	if (TotalGrip < KINDA_SMALL_NUMBER)
	{
		// No grip - equal split
		OutState.LeftWheelTorqueRatio = 0.5f;
		OutState.RightWheelTorqueRatio = 0.5f;
		OutState.BiasTorque = 0.0f;
		return;
	}

	// Calculate ideal (grip-based) torque split
	const float IdealLeftRatio = LeftGrip / TotalGrip;
	const float IdealRightRatio = RightGrip / TotalGrip;

	// Blend between open-diff behavior (follows grip) and locked behavior (50/50) based on lock percent
	OutState.LeftWheelTorqueRatio = FMath::Lerp(IdealLeftRatio, 0.5f, OutState.LockPercent);
	OutState.RightWheelTorqueRatio = FMath::Lerp(IdealRightRatio, 0.5f, OutState.LockPercent);

	// Calculate bias torque (torque transferred by the LSD mechanism)
	const float OpenDiffBias = FMath::Abs(IdealLeftRatio - 0.5f) * InputTorque;
	OutState.BiasTorque = OpenDiffBias * OutState.LockPercent;
}

void UMGVehicleMovementComponent::ApplyDifferentialTorqueToWheels(
	float DeltaTime,
	const FMGDifferentialState& DiffState,
	int32 LeftWheelIdx,
	int32 RightWheelIdx)
{
	// Apply the torque distribution calculated by the differential simulation
	// This affects wheel acceleration/deceleration

	if (!GetOwner() || !PVehicleOutput)
	{
		return;
	}

	// For locked diff or high lock percentage, apply forces to equalize wheel speeds
	if (DiffState.LockPercent > 0.1f)
	{
		if (UPrimitiveComponent* MeshPrimitive = Cast<UPrimitiveComponent>(UpdatedPrimitive))
		{
			const float SpeedDiff = DiffState.WheelSpeedDifferential;
			const float CorrectionStrength = DiffState.LockPercent * DifferentialViscosity * FMath::Abs(SpeedDiff);

			// Apply counter-torque to slow down the faster wheel
			// This simulates the binding effect of the LSD
			if (FMath::Abs(SpeedDiff) > 0.5f)
			{
				// Create a yaw moment from the differential binding
				// This affects vehicle rotation during turns
				const float YawCorrection = CorrectionStrength * FMath::Sign(SpeedDiff) * DeltaTime;

				// Apply as torque (locked diff creates understeer)
				const FVector TorqueVector = FVector(0.0f, 0.0f, YawCorrection * 100.0f);
				MeshPrimitive->AddTorqueInRadians(TorqueVector, NAME_None, true);
			}
		}
	}
}

void UMGVehicleMovementComponent::UpdatePowerDistributionData(float DeltaTime)
{
	const EMGDrivetrainType DriveType = CurrentConfiguration.Drivetrain.DrivetrainType;

	// Copy differential states for UI
	PowerDistributionData.RearDiffState = RearDiffState;
	PowerDistributionData.FrontDiffState = FrontDiffState;
	PowerDistributionData.CenterDiffState = CenterDiffState;

	// Calculate per-wheel power percentages
	switch (DriveType)
	{
		case EMGDrivetrainType::RWD:
			PowerDistributionData.FrontLeftPower = 0.0f;
			PowerDistributionData.FrontRightPower = 0.0f;
			PowerDistributionData.RearLeftPower = RearDiffState.LeftWheelTorqueRatio * 100.0f;
			PowerDistributionData.RearRightPower = RearDiffState.RightWheelTorqueRatio * 100.0f;
			PowerDistributionData.FrontAxlePower = 0.0f;
			PowerDistributionData.RearAxlePower = 100.0f;
			PowerDistributionData.CenterDiffBias = 0.0f;
			break;

		case EMGDrivetrainType::FWD:
			PowerDistributionData.FrontLeftPower = FrontDiffState.LeftWheelTorqueRatio * 100.0f;
			PowerDistributionData.FrontRightPower = FrontDiffState.RightWheelTorqueRatio * 100.0f;
			PowerDistributionData.RearLeftPower = 0.0f;
			PowerDistributionData.RearRightPower = 0.0f;
			PowerDistributionData.FrontAxlePower = 100.0f;
			PowerDistributionData.RearAxlePower = 0.0f;
			PowerDistributionData.CenterDiffBias = 1.0f;
			break;

		case EMGDrivetrainType::AWD:
			{
				const float FrontPower = CenterDiffState.LeftWheelTorqueRatio * 100.0f;
				const float RearPower = CenterDiffState.RightWheelTorqueRatio * 100.0f;

				PowerDistributionData.FrontLeftPower = FrontDiffState.LeftWheelTorqueRatio * FrontPower;
				PowerDistributionData.FrontRightPower = FrontDiffState.RightWheelTorqueRatio * FrontPower;
				PowerDistributionData.RearLeftPower = RearDiffState.LeftWheelTorqueRatio * RearPower;
				PowerDistributionData.RearRightPower = RearDiffState.RightWheelTorqueRatio * RearPower;
				PowerDistributionData.FrontAxlePower = FrontPower;
				PowerDistributionData.RearAxlePower = RearPower;
				PowerDistributionData.CenterDiffBias = AWDFrontBias;
			}
			break;
	}

	// Update wheel slip ratios and spin status
	for (int32 i = 0; i < 4; ++i)
	{
		PowerDistributionData.WheelSlipRatios[i] = GetWheelSlipRatio(i);
		PowerDistributionData.bWheelSpinning[i] = IsWheelSpinningExcessively(i);
	}

	// Calculate drivetrain loss
	PowerDistributionData.DrivetrainLossPercent = (1.0f - PartWearEffects.DrivetrainEfficiency) * 100.0f;
}

void UMGVehicleMovementComponent::IntegrateDifferentialWithWeightTransfer(float DeltaTime)
{
	// Weight transfer affects differential behavior through grip changes
	// This is already accounted for in CalculateTireFriction via GetWheelLoadMultiplier

	// Additionally, under hard acceleration, weight transfer to rear
	// increases rear tire grip, which affects LSD lockup behavior
	if (EngineState.ThrottlePosition > 0.7f && WeightTransferState.LongitudinalTransfer < -0.3f)
	{
		// Hard acceleration - weight on rear
		// This can reduce LSD lockup needs as both rear wheels have good grip
	}

	// Under hard braking, weight transfers forward
	// This reduces rear grip and may cause one-wheel spin with open diff
	if (GetBrakeInput() > 0.7f && WeightTransferState.LongitudinalTransfer > 0.3f)
	{
		// Weight on front - rear wheels light
		// Open diff would have inside wheel spin on corner entry
	}

	// Lateral weight transfer affects inside/outside wheel grip
	// LSD helps keep both wheels driving despite grip difference
}

// ==========================================
// DIFFERENTIAL QUERY METHODS
// ==========================================

float UMGVehicleMovementComponent::GetAxleLockPercent(bool bFrontAxle) const
{
	return bFrontAxle ? FrontDiffState.LockPercent : RearDiffState.LockPercent;
}

float UMGVehicleMovementComponent::GetWheelAngularVelocity(int32 WheelIndex) const
{
	if (WheelIndex >= 0 && WheelIndex < 4)
	{
		return WheelAngularVelocities[WheelIndex];
	}
	return 0.0f;
}

float UMGVehicleMovementComponent::GetAxleSpeedDifferential(bool bFrontAxle) const
{
	if (bFrontAxle)
	{
		return WheelAngularVelocities[0] - WheelAngularVelocities[1];
	}
	else
	{
		return WheelAngularVelocities[2] - WheelAngularVelocities[3];
	}
}

bool UMGVehicleMovementComponent::IsWheelSpinningExcessively(int32 WheelIndex) const
{
	const float SlipRatio = GetWheelSlipRatio(WheelIndex);
	const float SlipThreshold = 0.3f; // 30% slip = significant wheelspin
	return SlipRatio > SlipThreshold;
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
	const float SpeedFactor = FMath::Clamp(CachedSpeedMPH / 120.0f, 0.0f, 1.0f);

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
// CLUTCH WEAR SIMULATION
// ==========================================

void UMGVehicleMovementComponent::UpdateClutchWear(float DeltaTime)
{
	// Skip if no clutch engagement changes (automatic transmissions handle this internally)
	if (CurrentConfiguration.Drivetrain.TransmissionType == EMGTransmissionType::Automatic ||
		CurrentConfiguration.Drivetrain.TransmissionType == EMGTransmissionType::CVT)
	{
		return;
	}

	// Clutch wear occurs when:
	// 1. Slipping (partial engagement with throttle)
	// 2. Launch control with high throttle
	// 3. Dropping clutch at high RPM

	const float ClutchSlip = 1.0f - EngineState.ClutchEngagement;
	const float ThrottlePosition = EngineState.ThrottlePosition;

	// Calculate slip-based heat generation
	if (ClutchSlip > 0.1f && ThrottlePosition > 0.3f)
	{
		// Heat generation proportional to slip amount, throttle, and engine torque
		const float TorqueFactor = EngineState.CurrentTorque / FMath::Max(1.0f, CurrentConfiguration.Drivetrain.ClutchTorqueCapacity);
		const float HeatGenerated = ClutchSlip * ThrottlePosition * TorqueFactor * 50.0f * DeltaTime;

		// This would affect clutch temperature in the clutch state struct
		// For now, log excessive clutch abuse
		if (HeatGenerated > 5.0f * DeltaTime)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Clutch slip detected: %.1f%% slip at %.0f%% throttle"),
				ClutchSlip * 100.0f, ThrottlePosition * 100.0f);
		}
	}
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

// ==========================================
// ECU MAP CONTROLS
// ==========================================

bool UMGVehicleMovementComponent::SwitchECUMap(EMGECUMapType NewMapType)
{
	// Check if map is available
	if (!IsECUMapAvailable(NewMapType))
	{
		return false;
	}

	// Check if real-time switching is supported (if engine is running)
	if (EngineState.CurrentRPM > 0.0f && !CurrentConfiguration.Engine.ECU.bSupportsRealTimeMapSwitch)
	{
		// Can only switch while engine is off
		return false;
	}

	// Get the new map parameters
	EMGECUMapType OldMapType = CurrentConfiguration.Engine.ECU.ActiveMapType;
	CurrentConfiguration.Engine.ECU.ActiveMapType = NewMapType;

	// Apply immediate effects
	const FMGECUMapParameters& NewMap = CurrentConfiguration.Engine.ECU.GetActiveMap();

	// Update rev limiter
	if (NewMap.RevLimitRPM > 0)
	{
		// This would update the physics engine rev limiter
		// Engine.Transmission.RevLimitRPM = NewMap.RevLimitRPM;
	}

	// Update launch control if available
	if (NewMap.LaunchControlRPM > 0)
	{
		EngineState.LaunchControlRPM = static_cast<float>(NewMap.LaunchControlRPM);
	}

	// Update anti-lag state
	EngineState.bAntiLagActive = NewMap.bAntiLagEnabled && CurrentConfiguration.Engine.ForcedInduction.Type != EMGForcedInductionType::None;

	return true;
}

EMGECUMapType UMGVehicleMovementComponent::GetActiveECUMapType() const
{
	return CurrentConfiguration.Engine.ECU.ActiveMapType;
}

FMGECUMapParameters UMGVehicleMovementComponent::GetActiveECUMapParameters() const
{
	return CurrentConfiguration.Engine.ECU.GetActiveMap();
}

bool UMGVehicleMovementComponent::IsECUMapAvailable(EMGECUMapType MapType) const
{
	// Stock map is always available
	if (MapType == EMGECUMapType::Stock)
	{
		return true;
	}

	// Check if map is in available list
	const TArray<EMGECUMapType>& AvailableMaps = CurrentConfiguration.Engine.ECU.AvailableMaps;
	return AvailableMaps.Contains(MapType);
}

TArray<EMGECUMapType> UMGVehicleMovementComponent::GetAvailableECUMaps() const
{
	TArray<EMGECUMapType> Result;
	Result.Add(EMGECUMapType::Stock); // Always available

	// Add other available maps
	for (EMGECUMapType MapType : CurrentConfiguration.Engine.ECU.AvailableMaps)
	{
		if (MapType != EMGECUMapType::Stock)
		{
			Result.AddUnique(MapType);
		}
	}

	return Result;
}

float UMGVehicleMovementComponent::GetECUPowerMultiplier() const
{
	const FMGECUMapParameters& ActiveMap = CurrentConfiguration.Engine.ECU.GetActiveMap();

	float PowerMultiplier = ActiveMap.PowerMultiplier;

	// Check fuel octane requirements
	// If fuel octane is too low, reduce power to prevent knock
	// This would check against actual fuel in tank
	// For now, assume correct fuel is being used

	// Apply knock protection if we don't have wideband AFR
	if (!CurrentConfiguration.Engine.ECU.bHasWidebandAFR && ActiveMap.KnockProbability > 0.0f)
	{
		// Randomly detect "knock" and pull timing
		if (FMath::FRand() < ActiveMap.KnockProbability * 0.01f) // Per-frame check
		{
			// This would trigger knock retard
			PowerMultiplier *= 0.95f;
		}
	}

	return PowerMultiplier;
}

// ==========================================
// WEATHER EFFECTS INTEGRATION
// ==========================================

void UMGVehicleMovementComponent::SetWeatherGripMultiplier(float Multiplier)
{
	WeatherGripMultiplier = FMath::Clamp(Multiplier, 0.1f, 1.0f);
}

void UMGVehicleMovementComponent::ApplyAquaplaning(float Intensity, const TArray<float>& WheelFactors)
{
	bIsAquaplaning = Intensity > 0.1f;
	AquaplaningIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	// Store per-wheel factors
	WheelAquaplaningFactors = WheelFactors;

	// Ensure we have 4 wheel factors
	while (WheelAquaplaningFactors.Num() < 4)
	{
		WheelAquaplaningFactors.Add(0.0f);
	}

	if (bIsAquaplaning)
	{
		// During aquaplaning, severely reduce grip on affected wheels
		// This creates the characteristic loss of steering control
		const float GripReduction = FMath::Lerp(1.0f, 0.1f, AquaplaningIntensity);

		// Apply as additional multiplier through weather grip
		SetWeatherGripMultiplier(GetWeatherGripMultiplier() * GripReduction);

		// Add slight random steering drift during aquaplaning
		if (AquaplaningIntensity > 0.5f)
		{
			const float DriftAmount = (FMath::FRand() - 0.5f) * 0.1f * AquaplaningIntensity;
			TargetSteering += DriftAmount;
		}
	}
}

void UMGVehicleMovementComponent::ApplyWindForce(const FVector& WindForce)
{
	PendingWindForce = WindForce;

	// Apply wind force to the vehicle mesh
	if (UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(UpdatedComponent))
	{
		// Apply at center of pressure (slightly above center of mass for realistic behavior)
		const FVector ForceLocation = GetOwner()->GetActorLocation() + FVector(0, 0, 50.0f);
		Mesh->AddForceAtLocation(WindForce, ForceLocation);

		// Add slight torque for realistic yaw response to crosswind
		const FVector ForwardDir = GetOwner()->GetActorForwardVector();
		const FVector RightDir = GetOwner()->GetActorRightVector();
		const float CrosswindComponent = FVector::DotProduct(WindForce.GetSafeNormal(), RightDir);

		// Yaw torque - wind pushes the tail
		const FVector YawTorque = FVector::UpVector * CrosswindComponent * WindForce.Size() * 0.01f;
		Mesh->AddTorqueInDegrees(YawTorque);
	}
}

TArray<FVector> UMGVehicleMovementComponent::GetWheelWorldLocations() const
{
	TArray<FVector> Locations;
	Locations.SetNum(4);

	// Get owner location and rotation
	if (AActor* Owner = GetOwner())
	{
		const FVector OwnerLoc = Owner->GetActorLocation();
		const FRotator OwnerRot = Owner->GetActorRotation();
		const FVector Forward = Owner->GetActorForwardVector();
		const FVector Right = Owner->GetActorRightVector();

		// Estimate wheel positions based on typical vehicle dimensions
		// These would ideally come from wheel components
		const float Wheelbase = 270.0f; // cm
		const float TrackWidth = 160.0f; // cm

		// Front Left (FL)
		Locations[0] = OwnerLoc + Forward * (Wheelbase * 0.5f) - Right * (TrackWidth * 0.5f);
		// Front Right (FR)
		Locations[1] = OwnerLoc + Forward * (Wheelbase * 0.5f) + Right * (TrackWidth * 0.5f);
		// Rear Left (RL)
		Locations[2] = OwnerLoc - Forward * (Wheelbase * 0.5f) - Right * (TrackWidth * 0.5f);
		// Rear Right (RR)
		Locations[3] = OwnerLoc - Forward * (Wheelbase * 0.5f) + Right * (TrackWidth * 0.5f);
	}

	return Locations;
}

// ==========================================
// TIRE PRESSURE SIMULATION
// ==========================================

void UMGVehicleMovementComponent::UpdateTirePressure(float DeltaTime)
{
	for (int32 i = 0; i < 4; ++i)
	{
		FMGTirePressureState& Pressure = TirePressures[i];
		const FMGTireTemperature& Temp = TireTemperatures[i];

		// Update pressure based on tire temperature
		float AvgTireTemp = Temp.GetAverageTemp();
		Pressure.UpdatePressureFromTemperature(AvgTireTemp, AmbientTemperature);

		// Apply any leak damage
		Pressure.ApplyLeak(DeltaTime);
	}
}

float UMGVehicleMovementComponent::GetTirePressureGripMultiplier(int32 WheelIndex) const
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return 1.0f;
	}

	const FMGTirePressureState& Pressure = TirePressures[WheelIndex];
	float PressureGrip = Pressure.GetGripMultiplier();

	// Scale by influence parameter
	return FMath::Lerp(1.0f, PressureGrip, TirePressureGripInfluence);
}

float UMGVehicleMovementComponent::GetTirePressureWearMultiplier(int32 WheelIndex) const
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return 1.0f;
	}

	const FMGTirePressureState& Pressure = TirePressures[WheelIndex];
	float PressureWear = Pressure.GetWearRateMultiplier();

	// Scale by influence parameter
	return FMath::Lerp(1.0f, PressureWear, TirePressureWearInfluence);
}

void UMGVehicleMovementComponent::SetTirePressure(int32 WheelIndex, float ColdPressurePSI)
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return;
	}

	TirePressures[WheelIndex].ColdPressurePSI = FMath::Clamp(ColdPressurePSI, 20.0f, 50.0f);
	TirePressures[WheelIndex].CurrentPressurePSI = TirePressures[WheelIndex].ColdPressurePSI;
}

void UMGVehicleMovementComponent::SetAllTirePressures(float FrontPSI, float RearPSI)
{
	// Front tires
	SetTirePressure(0, FrontPSI);
	SetTirePressure(1, FrontPSI);

	// Rear tires
	SetTirePressure(2, RearPSI);
	SetTirePressure(3, RearPSI);
}

bool UMGVehicleMovementComponent::IsTirePressureWarning(int32 WheelIndex) const
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return false;
	}

	const FMGTirePressureState& Pressure = TirePressures[WheelIndex];
	float Deviation = FMath::Abs(Pressure.CurrentPressurePSI - Pressure.OptimalHotPressurePSI);

	return Deviation > PressureWarningThreshold;
}

void UMGVehicleMovementComponent::InitializeTirePressures()
{
	for (int32 i = 0; i < 4; ++i)
	{
		TirePressures[i].ColdPressurePSI = DefaultColdPressurePSI;
		TirePressures[i].CurrentPressurePSI = DefaultColdPressurePSI;
		TirePressures[i].OptimalHotPressurePSI = OptimalHotPressurePSI;
		TirePressures[i].bHasSlowLeak = false;
		TirePressures[i].LeakRatePSIPerSecond = 0.0f;
		TirePressures[i].bIsFlat = false;
	}
}

// ==========================================
// SUSPENSION GEOMETRY SYSTEM
// ==========================================

void UMGVehicleMovementComponent::UpdateSuspensionGeometry(float DeltaTime)
{
	// Calculate current body roll angle from lateral acceleration
	const FVector Velocity = GetOwner()->GetVelocity();
	const FVector RightVector = GetOwner()->GetActorRightVector();
	const float LateralVelocity = FVector::DotProduct(Velocity, RightVector);

	// Estimate lateral acceleration from velocity change (simplified)
	// In a full implementation, this would come from physics simulation
	const float Speed = GetSpeedMPH();
	const float SteeringAngle = FMath::Abs(CurrentSteering) * 35.0f; // Assume 35 deg max steering

	// Lateral acceleration approximation: v^2 / r, where r is based on steering angle
	float LateralAccelG = 0.0f;
	if (Speed > 5.0f && FMath::Abs(SteeringAngle) > 1.0f)
	{
		// Convert to lateral G-force estimate
		const float TurningRadius = 500.0f / FMath::Tan(FMath::DegreesToRadians(SteeringAngle));
		const float SpeedCmPerSec = Speed * 44.704f;
		LateralAccelG = (SpeedCmPerSec * SpeedCmPerSec) / (TurningRadius * 980.665f);
		LateralAccelG = FMath::Clamp(LateralAccelG, 0.0f, 2.0f);
	}

	// Calculate body roll from lateral acceleration
	CurrentBodyRollDeg = CalculateBodyRollAngle(LateralAccelG);

	// Determine if we're cornering (for grip modifier calculations)
	const bool bIsCornering = FMath::Abs(CurrentSteering) > 0.1f || FMath::Abs(LateralAccelG) > 0.1f;

	// Update contact patch state for each wheel
	for (int32 WheelIndex = 0; WheelIndex < 4; ++WheelIndex)
	{
		// Calculate dynamic camber change from suspension compression and body roll
		float SuspensionCompression = 0.0f;
		if (PVehicleOutput && WheelIndex < PVehicleOutput->Wheels.Num())
		{
			// Get suspension compression ratio (0 = extended, 1 = fully compressed)
			const Chaos::FSimpleWheelSim& WheelSim = PVehicleOutput->Wheels[WheelIndex];
			SuspensionCompression = WheelSim.GetSuspensionOffset() / 20.0f; // Normalize to typical travel
			SuspensionCompression = FMath::Clamp(SuspensionCompression, 0.0f, 1.0f);
		}

		// Calculate effective camber including dynamic changes
		const float DynamicCamberChange = CalculateDynamicCamberChange(WheelIndex, SuspensionCompression);
		const FMGSuspensionGeometry& Geometry = GetWheelGeometry(WheelIndex);
		EffectiveCamberAngles[WheelIndex] = Geometry.CamberAngleDeg + DynamicCamberChange;

		// Calculate complete contact patch state for this wheel
		CalculateContactPatchState(WheelIndex, SuspensionGeometryEffects.WheelContactPatch[WheelIndex]);
	}

	// Calculate aggregate vehicle-level effects
	// Steering response is primarily affected by front suspension geometry
	const FMGContactPatchState& FLPatch = SuspensionGeometryEffects.WheelContactPatch[0];
	const FMGContactPatchState& FRPatch = SuspensionGeometryEffects.WheelContactPatch[1];
	SuspensionGeometryEffects.SteeringResponseModifier = (FLPatch.ToeTurnInMultiplier + FRPatch.ToeTurnInMultiplier) * 0.5f;

	// Straight-line stability is affected by toe-in (rear primarily) and caster
	const FMGContactPatchState& RLPatch = SuspensionGeometryEffects.WheelContactPatch[2];
	const FMGContactPatchState& RRPatch = SuspensionGeometryEffects.WheelContactPatch[3];
	SuspensionGeometryEffects.StraightLineStabilityModifier =
		(FLPatch.ToeStabilityMultiplier + FRPatch.ToeStabilityMultiplier +
		 RLPatch.ToeStabilityMultiplier + RRPatch.ToeStabilityMultiplier +
		 FLPatch.CasterStabilityMultiplier + FRPatch.CasterStabilityMultiplier) / 6.0f;

	// Cornering grip is primarily affected by camber
	if (bIsCornering)
	{
		SuspensionGeometryEffects.CorneringGripModifier =
			(FLPatch.CamberLateralGripMultiplier + FRPatch.CamberLateralGripMultiplier +
			 RLPatch.CamberLateralGripMultiplier + RRPatch.CamberLateralGripMultiplier) / 4.0f;
	}
	else
	{
		SuspensionGeometryEffects.CorneringGripModifier = 1.0f;
	}

	// Tire wear rate is affected by excessive toe angles
	SuspensionGeometryEffects.TireWearRateModifier =
		(FLPatch.ToeWearMultiplier + FRPatch.ToeWearMultiplier +
		 RLPatch.ToeWearMultiplier + RRPatch.ToeWearMultiplier) / 4.0f;

	// Self-centering strength from caster
	SuspensionGeometryEffects.SelfCenteringStrength =
		(FLPatch.CasterSelfCenteringMultiplier + FRPatch.CasterSelfCenteringMultiplier) * 0.5f;

	// Steering weight from caster
	SuspensionGeometryEffects.SteeringWeightModifier =
		(FLPatch.CasterSteeringWeightMultiplier + FRPatch.CasterSteeringWeightMultiplier) * 0.5f;

	// Apply steering self-centering effect
	ApplySteeringSelfCentering(DeltaTime);
}

void UMGVehicleMovementComponent::CalculateContactPatchState(int32 WheelIndex, FMGContactPatchState& OutContactPatch) const
{
	// Get geometry for this wheel
	const FMGSuspensionGeometry& Geometry = GetWheelGeometry(WheelIndex);
	const float EffectiveCamber = EffectiveCamberAngles[WheelIndex];

	// Calculate camber effects
	float LateralGrip, LongitudinalGrip, ContactPatchWidth;
	CalculateCamberEffects(EffectiveCamber, CurrentBodyRollDeg, LateralGrip, LongitudinalGrip, ContactPatchWidth);

	OutContactPatch.CamberLateralGripMultiplier = LateralGrip;
	OutContactPatch.CamberLongitudinalGripMultiplier = LongitudinalGrip;
	OutContactPatch.EffectiveWidthRatio = ContactPatchWidth;

	// Calculate toe effects
	float TurnInResponse, Stability, TireWearRate;
	CalculateToeEffects(Geometry.ToeAngleDeg, TurnInResponse, Stability, TireWearRate);

	OutContactPatch.ToeTurnInMultiplier = TurnInResponse;
	OutContactPatch.ToeStabilityMultiplier = Stability;
	OutContactPatch.ToeWearMultiplier = TireWearRate;

	// Calculate caster effects (only applies to front wheels)
	float SelfCentering, CasterStability, SteeringWeight;
	const float SpeedMPH = GetSpeedMPH();

	if (WheelIndex < 2) // Front wheels
	{
		CalculateCasterEffects(Geometry.CasterAngleDeg, SpeedMPH, SelfCentering, CasterStability, SteeringWeight);
	}
	else // Rear wheels don't have caster effects
	{
		SelfCentering = 1.0f;
		CasterStability = 1.0f;
		SteeringWeight = 1.0f;
	}

	OutContactPatch.CasterSelfCenteringMultiplier = SelfCentering;
	OutContactPatch.CasterStabilityMultiplier = CasterStability;
	OutContactPatch.CasterSteeringWeightMultiplier = SteeringWeight;

	// Calculate combined grip modifier
	const bool bIsCornering = FMath::Abs(CurrentSteering) > 0.1f;
	OutContactPatch.CombinedGripModifier = CalculateCombinedGeometryGripModifier(WheelIndex, bIsCornering);
}

void UMGVehicleMovementComponent::CalculateCamberEffects(float CamberAngleDeg, float BodyRollEffect,
	float& OutLateralGrip, float& OutLongitudinalGrip, float& OutContactPatchWidth) const
{
	// Camber effects on grip:
	// - Negative camber: Improves cornering grip (tire leans into turn), reduces straight-line contact
	// - Positive camber: Reduces overall grip (almost never used in performance applications)
	// - Zero camber: Maximum straight-line contact patch, but wheel leans outward in corners

	// Optimal camber for cornering is typically around -2 to -4 degrees (after body roll)
	const float OptimalCorneringCamber = -3.0f;
	const float EffectiveCamberWithRoll = CamberAngleDeg - BodyRollEffect;

	// Lateral grip peaks at optimal negative camber
	// Using a bell curve centered around optimal camber
	const float CamberDifferenceFromOptimal = EffectiveCamberWithRoll - OptimalCorneringCamber;
	const float LateralGripFactor = FMath::Exp(-0.1f * CamberDifferenceFromOptimal * CamberDifferenceFromOptimal);

	// Scale the effect by SuspensionGeometryInfluence
	OutLateralGrip = FMath::Lerp(1.0f, 0.85f + 0.25f * LateralGripFactor, SuspensionGeometryInfluence);

	// Longitudinal grip (straight-line traction) is maximum at zero camber
	// Negative camber reduces contact patch for straight-line grip
	const float AbsCamber = FMath::Abs(CamberAngleDeg);
	const float LongitudinalGripFactor = FMath::Clamp(1.0f - AbsCamber * 0.03f, 0.7f, 1.0f);
	OutLongitudinalGrip = FMath::Lerp(1.0f, LongitudinalGripFactor, SuspensionGeometryInfluence);

	// Contact patch width ratio - negative camber reduces effective width
	// at rest, but maintains width better during cornering
	OutContactPatchWidth = FMath::Clamp(1.0f - AbsCamber * 0.02f, 0.8f, 1.0f);
}

void UMGVehicleMovementComponent::CalculateToeEffects(float ToeAngleDeg,
	float& OutTurnInResponse, float& OutStability, float& OutTireWearRate) const
{
	// Toe effects:
	// - Toe-out (positive): Improves turn-in response, reduces stability, increases tire wear
	// - Toe-in (negative): Improves stability, reduces turn-in response, moderate tire wear
	// - Zero toe: Neutral handling, minimal tire wear

	// Turn-in response improves with toe-out (positive toe)
	// Toe-out points wheels outward, so when you turn, the inside wheel is already pointing into the turn
	if (ToeAngleDeg > 0.0f)
	{
		// Toe-out: Better turn-in
		OutTurnInResponse = FMath::Lerp(1.0f, 1.0f + ToeAngleDeg * 0.15f, SuspensionGeometryInfluence);
	}
	else
	{
		// Toe-in: Worse turn-in
		OutTurnInResponse = FMath::Lerp(1.0f, 1.0f + ToeAngleDeg * 0.1f, SuspensionGeometryInfluence);
	}
	OutTurnInResponse = FMath::Clamp(OutTurnInResponse, 0.7f, 1.3f);

	// Stability improves with toe-in (negative toe)
	if (ToeAngleDeg < 0.0f)
	{
		// Toe-in: Better stability
		OutStability = FMath::Lerp(1.0f, 1.0f - ToeAngleDeg * 0.1f, SuspensionGeometryInfluence);
	}
	else
	{
		// Toe-out: Worse stability
		OutStability = FMath::Lerp(1.0f, 1.0f - ToeAngleDeg * 0.08f, SuspensionGeometryInfluence);
	}
	OutStability = FMath::Clamp(OutStability, 0.8f, 1.2f);

	// Tire wear increases with any toe angle (tires scrubbing)
	const float AbsToe = FMath::Abs(ToeAngleDeg);
	OutTireWearRate = 1.0f + AbsToe * 0.25f; // 25% more wear per degree of toe
	OutTireWearRate = FMath::Min(OutTireWearRate, 2.0f); // Cap at 2x wear rate
}

void UMGVehicleMovementComponent::CalculateCasterEffects(float CasterAngleDeg, float VehicleSpeedMPH,
	float& OutSelfCentering, float& OutStability, float& OutSteeringWeight) const
{
	// Caster effects:
	// - More caster: Stronger self-centering, better stability, heavier steering
	// - Less caster: Lighter steering, reduced self-centering, less stability
	// - Caster provides mechanical trail that creates self-centering torque

	// Reference caster angle (typical street car: 3-5 deg, race car: 7-10 deg)
	const float ReferenceCaster = 5.0f;
	const float CasterRatio = CasterAngleDeg / ReferenceCaster;

	// Self-centering force scales with caster and speed
	// More caster = stronger self-centering, and it increases with speed
	const float SpeedFactor = FMath::Clamp(VehicleSpeedMPH / 60.0f, 0.5f, 2.0f);
	OutSelfCentering = FMath::Lerp(1.0f, CasterRatio * SpeedFactor, SuspensionGeometryInfluence);
	OutSelfCentering = FMath::Clamp(OutSelfCentering, 0.3f, 2.5f);

	// High-speed stability improves with more caster
	OutStability = FMath::Lerp(1.0f, 0.9f + CasterRatio * 0.1f, SuspensionGeometryInfluence);
	OutStability = FMath::Clamp(OutStability, 0.8f, 1.2f);

	// Steering weight (effort) increases with caster due to mechanical trail
	// Trail creates a moment arm that requires more force to turn
	const float TrailEffect = CasterTrailCm * FMath::Sin(FMath::DegreesToRadians(CasterAngleDeg));
	OutSteeringWeight = FMath::Lerp(1.0f, 1.0f + TrailEffect * 0.05f, SuspensionGeometryInfluence);
	OutSteeringWeight = FMath::Clamp(OutSteeringWeight, 0.7f, 1.5f);
}

float UMGVehicleMovementComponent::CalculateBodyRollAngle(float LateralAcceleration) const
{
	// Calculate body roll based on lateral acceleration and suspension stiffness
	// Stiffer suspension = less body roll
	// More lateral G = more body roll

	// Reference body roll at 1G lateral acceleration
	const float RollPerG = ReferenceBodyRollDeg;

	// Get effective roll stiffness from suspension configuration
	// Stiffer springs and anti-roll bars reduce body roll
	float RollStiffnessFactor = 1.0f;

	// Front suspension stiffness effect
	const float FrontStiffnessNormalized = CurrentConfiguration.Suspension.FrontSpringRate / 50000.0f;
	const float RearStiffnessNormalized = CurrentConfiguration.Suspension.RearSpringRate / 50000.0f;
	RollStiffnessFactor = FMath::Clamp((FrontStiffnessNormalized + RearStiffnessNormalized) * 0.5f, 0.5f, 2.0f);

	// Calculate roll angle
	const float RollAngle = (LateralAcceleration * RollPerG) / RollStiffnessFactor;

	// Clamp to realistic values (typical street cars: 3-5 deg max, race cars: 1-2 deg max)
	return FMath::Clamp(RollAngle, -8.0f, 8.0f);
}

float UMGVehicleMovementComponent::CalculateDynamicCamberChange(int32 WheelIndex, float SuspensionCompressionRatio) const
{
	if (!bEnableDynamicCamber)
	{
		return 0.0f;
	}

	float DynamicCamber = 0.0f;

	// Camber change from body roll
	// During cornering, the outside wheels gain positive camber (bad) and inside wheels gain negative (good)
	// This is why static negative camber is used - to compensate for this gain
	const bool bIsLeftWheel = (WheelIndex == 0 || WheelIndex == 2);
	const float RollSign = bIsLeftWheel ? 1.0f : -1.0f;

	// Roll-induced camber change (positive roll = turning right)
	DynamicCamber += CurrentBodyRollDeg * CamberGainPerDegreeRoll * RollSign;

	// Camber change from suspension compression (bump camber)
	// Most suspension geometries gain negative camber in compression
	// This is beneficial during cornering as the loaded wheel compresses
	const float BumpCamberGain = -0.3f; // degrees per unit compression ratio
	DynamicCamber += SuspensionCompressionRatio * BumpCamberGain;

	return DynamicCamber;
}

void UMGVehicleMovementComponent::ApplySteeringSelfCentering(float DeltaTime)
{
	// Self-centering effect from caster
	// When no steering input, the wheels naturally want to return to center

	if (FMath::Abs(TargetSteering) < 0.05f && FMath::Abs(CurrentSteering) > 0.01f)
	{
		// No input but wheels are turned - apply self-centering
		const float SelfCenteringStrength = SuspensionGeometryEffects.SelfCenteringStrength;
		const float SpeedFactor = FMath::Clamp(GetSpeedMPH() / 30.0f, 0.5f, 2.0f);

		// Stronger centering at higher speeds
		const float CenteringRate = 3.0f * SelfCenteringStrength * SpeedFactor;
		CurrentSteering = FMath::FInterpTo(CurrentSteering, 0.0f, DeltaTime, CenteringRate);
	}
}

float UMGVehicleMovementComponent::CalculateCombinedGeometryGripModifier(int32 WheelIndex, bool bIsCornering) const
{
	const FMGContactPatchState& Patch = SuspensionGeometryEffects.WheelContactPatch[WheelIndex];

	float GripModifier = 1.0f;

	if (bIsCornering)
	{
		// During cornering, lateral grip is primary
		GripModifier *= Patch.CamberLateralGripMultiplier;
		GripModifier *= Patch.ToeStabilityMultiplier;
	}
	else
	{
		// Straight-line driving, longitudinal grip is primary
		GripModifier *= Patch.CamberLongitudinalGripMultiplier;
		GripModifier *= Patch.ToeStabilityMultiplier;
	}

	// Contact patch width always affects grip
	GripModifier *= Patch.EffectiveWidthRatio;

	return FMath::Clamp(GripModifier, 0.5f, 1.5f);
}

const FMGSuspensionGeometry& UMGVehicleMovementComponent::GetWheelGeometry(int32 WheelIndex) const
{
	// Front wheels (0, 1) use front geometry, rear wheels (2, 3) use rear geometry
	if (WheelIndex < 2)
	{
		return FrontSuspensionGeometry;
	}
	return RearSuspensionGeometry;
}

FMGContactPatchState UMGVehicleMovementComponent::GetWheelContactPatchState(int32 WheelIndex) const
{
	if (WheelIndex >= 0 && WheelIndex < 4)
	{
		return SuspensionGeometryEffects.WheelContactPatch[WheelIndex];
	}
	return FMGContactPatchState();
}

float UMGVehicleMovementComponent::GetEffectiveCamberAngle(int32 WheelIndex) const
{
	if (WheelIndex >= 0 && WheelIndex < 4)
	{
		return EffectiveCamberAngles[WheelIndex];
	}
	return 0.0f;
}

float UMGVehicleMovementComponent::GetSteeringSelfCenteringForce() const
{
	return SuspensionGeometryEffects.SelfCenteringStrength;
}

float UMGVehicleMovementComponent::GetGeometryGripModifier(int32 WheelIndex) const
{
	if (WheelIndex >= 0 && WheelIndex < 4)
	{
		return SuspensionGeometryEffects.WheelContactPatch[WheelIndex].CombinedGripModifier;
	}
	return 1.0f;
}

// ==========================================
// FUEL SYSTEM INTEGRATION
// ==========================================

void UMGVehicleMovementComponent::SetFuelStarvationMultiplier(float Multiplier)
{
	FuelStarvationMultiplier = FMath::Clamp(Multiplier, 0.0f, 1.0f);

	if (FuelStarvationMultiplier < 0.99f)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Fuel starvation: Power reduced to %.0f%%"), FuelStarvationMultiplier * 100.0f);
	}
}

void UMGVehicleMovementComponent::SetCurrentFuelWeightKg(float WeightKg)
{
	const float PreviousWeight = CurrentFuelWeightKg;
	CurrentFuelWeightKg = FMath::Max(0.0f, WeightKg);

	// Update vehicle mass based on fuel weight change
	// This affects acceleration, braking, and handling
	if (FMath::Abs(CurrentFuelWeightKg - PreviousWeight) > 0.5f)
	{
		// Calculate new total mass
		const float NewTotalMass = BaseMassKg + CurrentFuelWeightKg;

		// Apply to Chaos vehicle (mass is in kg)
		if (UPrimitiveComponent* MeshPrimitive = Cast<UPrimitiveComponent>(UpdatedPrimitive))
		{
			// Note: Direct mass modification would require physics body recreation
			// Instead, we'll factor weight into acceleration calculations
			// The weight difference affects power-to-weight ratio in CalculateCurrentPower
		}

		UE_LOG(LogTemp, Verbose, TEXT("Fuel weight updated: %.1f kg (Total vehicle: %.1f kg)"),
			CurrentFuelWeightKg, NewTotalMass);
	}
}

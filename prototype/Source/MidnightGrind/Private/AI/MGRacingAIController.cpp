// Copyright Midnight Grind. All Rights Reserved.

#include "AI/MGRacingAIController.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Track/MGTrackSpline.h"
#include "Track/MGCheckpoint.h"

AMGRacingAIController::AMGRacingAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// Default driver profile
	DriverProfile = FMGAIDriverProfile::CreateForDifficulty(EMGAIDifficulty::Amateur);
}

void AMGRacingAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledVehicle = Cast<AMGVehiclePawn>(InPawn);
	if (!ControlledVehicle)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGRacingAIController: Possessed pawn is not a vehicle!"));
	}
}

void AMGRacingAIController::OnUnPossess()
{
	StopRacing();
	ControlledVehicle = nullptr;
	Super::OnUnPossess();
}

void AMGRacingAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsRacing() && ControlledVehicle && TrackSpline)
	{
		UpdateDriving(DeltaTime);
	}
}

// ==========================================
// INITIALIZATION
// ==========================================

void AMGRacingAIController::SetTrackSpline(AMGTrackSpline* InTrackSpline)
{
	TrackSpline = InTrackSpline;
}

void AMGRacingAIController::SetDriverProfile(const FMGAIDriverProfile& InProfile)
{
	DriverProfile = InProfile;
}

void AMGRacingAIController::SetRubberBandingConfig(const FMGRubberBandingConfig& InConfig)
{
	RubberBandingConfig = InConfig;
}

void AMGRacingAIController::InitializeForRace()
{
	if (!ControlledVehicle || !TrackSpline)
	{
		return;
	}

	// Find starting position on track
	FVector VehicleLocation = ControlledVehicle->GetActorLocation();
	DrivingState.CurrentTrackDistance = TrackSpline->GetClosestDistanceOnTrack(VehicleLocation);
	DrivingState.TargetTrackDistance = DrivingState.CurrentTrackDistance;

	// Reset state
	DrivingState.ThrottleOutput = 0.0f;
	DrivingState.SteeringOutput = 0.0f;
	DrivingState.BrakeOutput = 0.0f;
	DrivingState.bHandbrake = false;
	DrivingState.bNitrous = false;
	DrivingState.bAttemptingOvertake = false;
	DrivingState.bDefendingPosition = false;

	SkillVariationTimer = 0.0f;
	CurrentSteeringNoise = 0.0f;
	CurrentThrottleNoise = 0.0f;
	TimeSinceNitrous = 10.0f; // Allow immediate nitrous use

	bIsRacing = false;
	bIsPaused = false;
}

// ==========================================
// RACE CONTROL
// ==========================================

void AMGRacingAIController::StartRacing()
{
	bIsRacing = true;
	bIsPaused = false;
}

void AMGRacingAIController::StopRacing()
{
	bIsRacing = false;

	// Release all inputs
	if (ControlledVehicle)
	{
		DrivingState.ThrottleOutput = 0.0f;
		DrivingState.SteeringOutput = 0.0f;
		DrivingState.BrakeOutput = 0.0f;
		DrivingState.bHandbrake = false;
		DrivingState.bNitrous = false;

		// Apply zero inputs to vehicle
		// ControlledVehicle->SetThrottleInput(0.0f);
		// ControlledVehicle->SetSteeringInput(0.0f);
		// ControlledVehicle->SetBrakeInput(0.0f);
	}
}

void AMGRacingAIController::SetPaused(bool bPaused)
{
	bIsPaused = bPaused;
}

void AMGRacingAIController::UpdateRacePosition(int32 Position, float DistanceToAhead, float DistanceToBehind)
{
	DrivingState.CurrentPosition = Position;
	DrivingState.DistanceToVehicleAhead = DistanceToAhead;
	DrivingState.DistanceToVehicleBehind = DistanceToBehind;
}

// ==========================================
// DRIVING LOGIC
// ==========================================

void AMGRacingAIController::UpdateDriving(float DeltaTime)
{
	// Update track position
	FVector VehicleLocation = ControlledVehicle->GetActorLocation();
	DrivingState.CurrentTrackDistance = TrackSpline->GetClosestDistanceOnTrack(VehicleLocation);

	// Update target point
	UpdateTargetPoint();

	// Apply skill-based variation (mistakes, reaction time)
	ApplySkillVariation(DeltaTime);

	// Calculate steering
	DrivingState.SteeringOutput = CalculateSteering();

	// Calculate throttle/brake
	CalculateSpeedControl(DrivingState.ThrottleOutput, DrivingState.BrakeOutput);

	// Overtake/defense logic
	UpdateOvertakeLogic();
	UpdateDefenseLogic();

	// Nitrous decision
	DrivingState.bNitrous = ShouldUseNitrous();
	TimeSinceNitrous = DrivingState.bNitrous ? 0.0f : TimeSinceNitrous + DeltaTime;

	// Drift/handbrake decision
	DrivingState.bHandbrake = ShouldUseDriftHandbrake();

	// Apply inputs to vehicle
	// These would call into the vehicle pawn's input handling
	// ControlledVehicle->SetThrottleInput(DrivingState.ThrottleOutput);
	// ControlledVehicle->SetSteeringInput(DrivingState.SteeringOutput);
	// ControlledVehicle->SetBrakeInput(DrivingState.BrakeOutput);
	// ControlledVehicle->SetHandbrakeInput(DrivingState.bHandbrake);
	// ControlledVehicle->SetNitrousInput(DrivingState.bNitrous);
}

void AMGRacingAIController::UpdateTargetPoint()
{
	float LookaheadDist = GetLookaheadDistance();
	DrivingState.TargetTrackDistance = DrivingState.CurrentTrackDistance + LookaheadDist;

	// Handle track wrap-around
	float TrackLength = TrackSpline->GetTrackLength();
	if (DrivingState.TargetTrackDistance > TrackLength)
	{
		DrivingState.TargetTrackDistance -= TrackLength;
	}

	// Get world position for target with some lateral offset based on racing line
	DrivingState.TargetPosition = TrackSpline->GetPositionAtDistance(DrivingState.TargetTrackDistance);

	// Add line variation based on skill (worse drivers wander from ideal line)
	if (DriverProfile.LineVariation > 0.0f)
	{
		FVector TrackRight = TrackSpline->GetRightVectorAtDistance(DrivingState.TargetTrackDistance);
		float MaxOffset = 200.0f * DriverProfile.LineVariation; // Up to 2m offset
		float Offset = CurrentSteeringNoise * MaxOffset;
		DrivingState.TargetPosition += TrackRight * Offset;
	}
}

float AMGRacingAIController::CalculateSteering()
{
	if (!ControlledVehicle)
	{
		return 0.0f;
	}

	FVector VehicleLocation = ControlledVehicle->GetActorLocation();
	FVector VehicleForward = ControlledVehicle->GetActorForwardVector();
	FVector ToTarget = DrivingState.TargetPosition - VehicleLocation;
	ToTarget.Z = 0.0f; // Ignore vertical difference
	ToTarget.Normalize();

	// Calculate angle to target
	float DotRight = FVector::DotProduct(ToTarget, ControlledVehicle->GetActorRightVector());

	// Apply steering sensitivity
	float Steering = FMath::Clamp(DotRight * SteeringSensitivity, -1.0f, 1.0f);

	// Add steering noise from skill variation
	Steering += CurrentSteeringNoise * 0.1f;
	Steering = FMath::Clamp(Steering, -1.0f, 1.0f);

	return Steering;
}

void AMGRacingAIController::CalculateSpeedControl(float& OutThrottle, float& OutBrake)
{
	if (!ControlledVehicle)
	{
		OutThrottle = 0.0f;
		OutBrake = 0.0f;
		return;
	}

	// Get current speed
	float CurrentSpeed = ControlledVehicle->GetVelocity().Size();

	// Get track curvature ahead
	float Curvature = TrackSpline->GetCurvatureAtDistance(DrivingState.TargetTrackDistance);

	// Calculate target speed based on curvature
	float BaseTargetSpeed = GetOptimalSpeedForCurvature(Curvature);

	// Apply driver skill to target speed
	BaseTargetSpeed *= DriverProfile.SpeedMultiplier;

	// Apply rubber-banding
	DrivingState.TargetSpeed = ApplyRubberBanding(BaseTargetSpeed);

	// Apply throttle noise from skill
	float ThrottleSkillFactor = FMath::Lerp(0.85f, 1.0f, DriverProfile.ThrottleControl);

	// Speed difference for throttle/brake decision
	float SpeedDiff = DrivingState.TargetSpeed - CurrentSpeed;

	// Braking skill affects how early/late we brake
	float BrakingThreshold = FMath::Lerp(-500.0f, 0.0f, DriverProfile.BrakingSkill);

	if (SpeedDiff < BrakingThreshold)
	{
		// Need to brake
		OutThrottle = 0.0f;
		float BrakeStrength = FMath::Clamp((-SpeedDiff) / 1000.0f, 0.0f, 1.0f);
		OutBrake = BrakeStrength * DriverProfile.BrakingSkill;
	}
	else if (SpeedDiff > 200.0f)
	{
		// Accelerate
		float ThrottleStrength = FMath::Clamp(SpeedDiff / 500.0f, 0.0f, 1.0f);
		OutThrottle = ThrottleStrength * ThrottleSkillFactor;
		OutThrottle += CurrentThrottleNoise * 0.1f; // Add noise
		OutThrottle = FMath::Clamp(OutThrottle, 0.0f, 1.0f);
		OutBrake = 0.0f;
	}
	else
	{
		// Maintain speed
		OutThrottle = 0.5f * ThrottleSkillFactor;
		OutBrake = 0.0f;
	}
}

bool AMGRacingAIController::ShouldUseNitrous()
{
	// Check if we have nitrous and cooldown
	if (TimeSinceNitrous < 5.0f)
	{
		return false;
	}

	// Get track curvature - only use on straights
	float Curvature = TrackSpline->GetCurvatureAtDistance(DrivingState.CurrentTrackDistance);
	if (FMath::Abs(Curvature) > 0.01f)
	{
		return false; // Too curvy
	}

	// Usage depends on personality and situation
	float UseChance = DriverProfile.NitrousUsage;

	// More likely if behind
	if (DrivingState.CurrentPosition > 1)
	{
		UseChance *= 1.5f;
	}

	// More likely if being chased closely
	if (DrivingState.DistanceToVehicleBehind >= 0 && DrivingState.DistanceToVehicleBehind < 1000.0f)
	{
		UseChance *= 1.3f;
	}

	// Random chance based on personality
	return FMath::FRand() < (UseChance * 0.01f); // Check each frame with small chance
}

bool AMGRacingAIController::ShouldUseDriftHandbrake()
{
	// Only drifter personality uses handbrake intentionally
	if (DriverProfile.Personality != EMGAIPersonality::Drifter)
	{
		return false;
	}

	// Check for sharp corner ahead
	float Curvature = TrackSpline->GetCurvatureAtDistance(DrivingState.TargetTrackDistance);
	if (FMath::Abs(Curvature) > 0.05f)
	{
		// Check current speed - need to be going fast enough
		float CurrentSpeed = ControlledVehicle ? ControlledVehicle->GetVelocity().Size() : 0.0f;
		if (CurrentSpeed > 2000.0f)
		{
			return true;
		}
	}

	return false;
}

float AMGRacingAIController::ApplyRubberBanding(float BaseSpeed)
{
	if (!RubberBandingConfig.bEnabled)
	{
		return BaseSpeed;
	}

	float SpeedMultiplier = 1.0f;

	// Check distance to leader (use distance ahead as proxy)
	if (DrivingState.DistanceToVehicleAhead > 0)
	{
		// Behind - catch up
		if (DrivingState.DistanceToVehicleAhead > RubberBandingConfig.CatchUpStartDistance)
		{
			float DistanceFactor = (DrivingState.DistanceToVehicleAhead - RubberBandingConfig.CatchUpStartDistance) / 10000.0f;
			DistanceFactor = FMath::Pow(DistanceFactor, RubberBandingConfig.ScalingExponent);
			SpeedMultiplier = FMath::Lerp(1.0f, RubberBandingConfig.MaxCatchUpBoost, FMath::Clamp(DistanceFactor, 0.0f, 1.0f));
		}
	}
	else if (DrivingState.CurrentPosition == 1 && DrivingState.DistanceToVehicleBehind > 0)
	{
		// Leading - potentially slow down
		if (DrivingState.DistanceToVehicleBehind > RubberBandingConfig.SlowDownStartDistance)
		{
			float DistanceFactor = (DrivingState.DistanceToVehicleBehind - RubberBandingConfig.SlowDownStartDistance) / 10000.0f;
			DistanceFactor = FMath::Pow(DistanceFactor, RubberBandingConfig.ScalingExponent);
			SpeedMultiplier = FMath::Lerp(1.0f, RubberBandingConfig.MinSlowDownFactor, FMath::Clamp(DistanceFactor, 0.0f, 1.0f));
		}
	}

	return BaseSpeed * SpeedMultiplier;
}

void AMGRacingAIController::UpdateOvertakeLogic()
{
	DrivingState.bAttemptingOvertake = false;

	if (DrivingState.DistanceToVehicleAhead < 0)
	{
		return; // No one ahead
	}

	// Check if close enough to attempt overtake
	if (DrivingState.DistanceToVehicleAhead < 1500.0f) // 15 meters
	{
		// Chance to attempt based on aggression
		if (FMath::FRand() < DriverProfile.OvertakeAggression * 0.1f)
		{
			DrivingState.bAttemptingOvertake = true;

			// Modify target position to side of track for overtake
			FVector TrackRight = TrackSpline->GetRightVectorAtDistance(DrivingState.TargetTrackDistance);
			float OvertakeSide = FMath::RandBool() ? 1.0f : -1.0f;
			DrivingState.TargetPosition += TrackRight * OvertakeSide * 300.0f;
		}
	}
}

void AMGRacingAIController::UpdateDefenseLogic()
{
	DrivingState.bDefendingPosition = false;

	if (DrivingState.DistanceToVehicleBehind < 0)
	{
		return; // No one behind
	}

	// Check if being pressured from behind
	if (DrivingState.DistanceToVehicleBehind < 1000.0f) // 10 meters
	{
		// Chance to defend based on aggression
		if (FMath::FRand() < DriverProfile.DefensiveAggression * 0.1f)
		{
			DrivingState.bDefendingPosition = true;

			// Block the inside line on corners
			float Curvature = TrackSpline->GetCurvatureAtDistance(DrivingState.TargetTrackDistance);
			if (FMath::Abs(Curvature) > 0.01f)
			{
				FVector TrackRight = TrackSpline->GetRightVectorAtDistance(DrivingState.TargetTrackDistance);
				float InsideSide = Curvature > 0.0f ? -1.0f : 1.0f;
				DrivingState.TargetPosition += TrackRight * InsideSide * 150.0f;
			}
		}
	}
}

void AMGRacingAIController::ApplySkillVariation(float DeltaTime)
{
	SkillVariationTimer += DeltaTime;

	// Update noise values periodically based on reaction time
	if (SkillVariationTimer >= DriverProfile.ReactionTime)
	{
		SkillVariationTimer = 0.0f;

		// Higher skill = less noise/mistakes
		float SkillFactor = 1.0f - (DriverProfile.SkillRating / 100.0f);

		// Random steering wobble
		CurrentSteeringNoise = FMath::FRandRange(-1.0f, 1.0f) * SkillFactor;

		// Random throttle inconsistency
		CurrentThrottleNoise = FMath::FRandRange(-1.0f, 1.0f) * SkillFactor;
	}
}

float AMGRacingAIController::GetLookaheadDistance() const
{
	float CurrentSpeed = ControlledVehicle ? ControlledVehicle->GetVelocity().Size() : 0.0f;
	float Lookahead = BaseLookaheadDistance + (CurrentSpeed * LookaheadSpeedFactor);
	return FMath::Clamp(Lookahead, BaseLookaheadDistance, MaxLookaheadDistance);
}

float AMGRacingAIController::GetOptimalSpeedForCurvature(float Curvature) const
{
	// Convert curvature to corner tightness (0 = straight, 1 = hairpin)
	float Tightness = FMath::Clamp(FMath::Abs(Curvature) * 10.0f, 0.0f, 1.0f);

	// Interpolate between max straight speed and min corner speed
	return FMath::Lerp(MaxStraightSpeed, MinCornerSpeed, Tightness);
}

// ==========================================
// STATIC FUNCTIONS
// ==========================================

FMGAIDriverProfile FMGAIDriverProfile::CreateForDifficulty(EMGAIDifficulty Difficulty)
{
	FMGAIDriverProfile Profile;
	Profile.Difficulty = Difficulty;

	switch (Difficulty)
	{
		case EMGAIDifficulty::Rookie:
			Profile.DriverName = NSLOCTEXT("AI", "RookieDriver", "Rookie Driver");
			Profile.SkillRating = 25.0f;
			Profile.ReactionTime = 0.4f;
			Profile.LineVariation = 0.6f;
			Profile.BrakingSkill = 0.5f;
			Profile.ThrottleControl = 0.5f;
			Profile.DefensiveAggression = 0.2f;
			Profile.OvertakeAggression = 0.2f;
			Profile.NitrousUsage = 0.2f;
			Profile.SpeedMultiplier = 0.85f;
			break;

		case EMGAIDifficulty::Amateur:
			Profile.DriverName = NSLOCTEXT("AI", "AmateurDriver", "Amateur Driver");
			Profile.SkillRating = 45.0f;
			Profile.ReactionTime = 0.3f;
			Profile.LineVariation = 0.4f;
			Profile.BrakingSkill = 0.65f;
			Profile.ThrottleControl = 0.65f;
			Profile.DefensiveAggression = 0.35f;
			Profile.OvertakeAggression = 0.35f;
			Profile.NitrousUsage = 0.4f;
			Profile.SpeedMultiplier = 0.92f;
			break;

		case EMGAIDifficulty::Pro:
			Profile.DriverName = NSLOCTEXT("AI", "ProDriver", "Pro Driver");
			Profile.SkillRating = 65.0f;
			Profile.ReactionTime = 0.2f;
			Profile.LineVariation = 0.25f;
			Profile.BrakingSkill = 0.8f;
			Profile.ThrottleControl = 0.8f;
			Profile.DefensiveAggression = 0.5f;
			Profile.OvertakeAggression = 0.5f;
			Profile.NitrousUsage = 0.6f;
			Profile.SpeedMultiplier = 1.0f;
			break;

		case EMGAIDifficulty::Expert:
			Profile.DriverName = NSLOCTEXT("AI", "ExpertDriver", "Expert Driver");
			Profile.SkillRating = 85.0f;
			Profile.ReactionTime = 0.12f;
			Profile.LineVariation = 0.1f;
			Profile.BrakingSkill = 0.92f;
			Profile.ThrottleControl = 0.92f;
			Profile.DefensiveAggression = 0.7f;
			Profile.OvertakeAggression = 0.7f;
			Profile.NitrousUsage = 0.8f;
			Profile.SpeedMultiplier = 1.05f;
			break;

		case EMGAIDifficulty::Legend:
			Profile.DriverName = NSLOCTEXT("AI", "LegendDriver", "Legend Driver");
			Profile.SkillRating = 98.0f;
			Profile.ReactionTime = 0.08f;
			Profile.LineVariation = 0.02f;
			Profile.BrakingSkill = 0.98f;
			Profile.ThrottleControl = 0.98f;
			Profile.DefensiveAggression = 0.85f;
			Profile.OvertakeAggression = 0.85f;
			Profile.NitrousUsage = 0.95f;
			Profile.SpeedMultiplier = 1.1f;
			break;
	}

	return Profile;
}

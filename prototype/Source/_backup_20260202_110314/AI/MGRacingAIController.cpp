// Copyright Midnight Grind. All Rights Reserved.

#include "AI/MGRacingAIController.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Track/MGCheckpoint.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// Generate profile from difficulty
void FMGAIDriverProfile::GenerateFromDifficulty(EMGAIDifficulty InDifficulty)
{
	Difficulty = InDifficulty;

	switch (InDifficulty)
	{
	case EMGAIDifficulty::Rookie:
		SkillRating = 0.3f;
		CorneringSkill = 0.3f;
		BrakingSkill = 0.3f;
		OvertakeAggression = 0.2f;
		DefensiveSkill = 0.2f;
		Consistency = 0.5f;
		RiskTolerance = 0.2f;
		ReactionTime = 0.6f;
		TopSpeedFactor = 0.75f;
		NOSAggression = 0.2f;
		break;

	case EMGAIDifficulty::Amateur:
		SkillRating = 0.5f;
		CorneringSkill = 0.5f;
		BrakingSkill = 0.5f;
		OvertakeAggression = 0.35f;
		DefensiveSkill = 0.35f;
		Consistency = 0.65f;
		RiskTolerance = 0.35f;
		ReactionTime = 0.45f;
		TopSpeedFactor = 0.85f;
		NOSAggression = 0.35f;
		break;

	case EMGAIDifficulty::Professional:
		SkillRating = 0.7f;
		CorneringSkill = 0.7f;
		BrakingSkill = 0.7f;
		OvertakeAggression = 0.5f;
		DefensiveSkill = 0.5f;
		Consistency = 0.75f;
		RiskTolerance = 0.5f;
		ReactionTime = 0.35f;
		TopSpeedFactor = 0.92f;
		NOSAggression = 0.5f;
		break;

	case EMGAIDifficulty::Expert:
		SkillRating = 0.85f;
		CorneringSkill = 0.85f;
		BrakingSkill = 0.85f;
		OvertakeAggression = 0.65f;
		DefensiveSkill = 0.65f;
		Consistency = 0.85f;
		RiskTolerance = 0.6f;
		ReactionTime = 0.25f;
		TopSpeedFactor = 0.96f;
		NOSAggression = 0.65f;
		break;

	case EMGAIDifficulty::Master:
		SkillRating = 0.95f;
		CorneringSkill = 0.95f;
		BrakingSkill = 0.95f;
		OvertakeAggression = 0.8f;
		DefensiveSkill = 0.8f;
		Consistency = 0.92f;
		RiskTolerance = 0.7f;
		ReactionTime = 0.18f;
		TopSpeedFactor = 0.98f;
		NOSAggression = 0.8f;
		break;

	case EMGAIDifficulty::Legend:
		SkillRating = 1.0f;
		CorneringSkill = 1.0f;
		BrakingSkill = 1.0f;
		OvertakeAggression = 0.9f;
		DefensiveSkill = 0.9f;
		Consistency = 0.98f;
		RiskTolerance = 0.8f;
		ReactionTime = 0.12f;
		TopSpeedFactor = 1.0f;
		NOSAggression = 0.9f;
		break;
	}

	// Add personality variation
	switch (Personality)
	{
	case EMGAIPersonality::Aggressive:
		OvertakeAggression += 0.15f;
		RiskTolerance += 0.15f;
		DefensiveSkill -= 0.1f;
		break;

	case EMGAIPersonality::Defensive:
		DefensiveSkill += 0.15f;
		OvertakeAggression -= 0.15f;
		RiskTolerance -= 0.1f;
		break;

	case EMGAIPersonality::Showoff:
		RiskTolerance += 0.1f;
		Consistency -= 0.1f;
		break;

	case EMGAIPersonality::Calculated:
		Consistency += 0.1f;
		RiskTolerance -= 0.1f;
		break;

	case EMGAIPersonality::Wildcard:
		// Add randomization
		SkillRating += FMath::RandRange(-0.1f, 0.1f);
		RiskTolerance += FMath::RandRange(-0.2f, 0.2f);
		break;

	default:
		break;
	}

	// Clamp all values
	SkillRating = FMath::Clamp(SkillRating, 0.0f, 1.0f);
	CorneringSkill = FMath::Clamp(CorneringSkill, 0.0f, 1.0f);
	BrakingSkill = FMath::Clamp(BrakingSkill, 0.0f, 1.0f);
	OvertakeAggression = FMath::Clamp(OvertakeAggression, 0.0f, 1.0f);
	DefensiveSkill = FMath::Clamp(DefensiveSkill, 0.0f, 1.0f);
	Consistency = FMath::Clamp(Consistency, 0.0f, 1.0f);
	RiskTolerance = FMath::Clamp(RiskTolerance, 0.0f, 1.0f);
}

AMGRacingAIController::AMGRacingAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// Default profile
	DriverProfile.GenerateFromDifficulty(EMGAIDifficulty::Professional);
}

void AMGRacingAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AMGRacingAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bAIEnabled || !ControlledVehicle)
	{
		return;
	}

	UpdateState(DeltaTime);
	UpdateNavigation(DeltaTime);
	UpdateVehicleInputs(DeltaTime);
	ApplyInputsToVehicle();

	// Update timers
	StateTime += DeltaTime;
	if (MistakeCooldown > 0.0f) MistakeCooldown -= DeltaTime;
	if (OvertakeCooldown > 0.0f) OvertakeCooldown -= DeltaTime;
}

void AMGRacingAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledVehicle = Cast<AMGVehiclePawn>(InPawn);
}

void AMGRacingAIController::SetDriverProfile(const FMGAIDriverProfile& Profile)
{
	DriverProfile = Profile;
}

void AMGRacingAIController::SetDifficulty(EMGAIDifficulty Difficulty)
{
	DriverProfile.GenerateFromDifficulty(Difficulty);
}

void AMGRacingAIController::SetRacingLine(USplineComponent* Spline)
{
	RacingLineSpline = Spline;
}

void AMGRacingAIController::SetAIEnabled(bool bEnabled)
{
	bAIEnabled = bEnabled;

	if (!bEnabled)
	{
		// Clear inputs
		ThrottleOutput = 0.0f;
		BrakeOutput = 0.0f;
		SteeringOutput = 0.0f;
		bWantsNOS = false;
	}
}

void AMGRacingAIController::StartRacing()
{
	SetState(EMGAIState::Racing);
}

void AMGRacingAIController::StopRacing()
{
	SetState(EMGAIState::Waiting);
	ThrottleOutput = 0.0f;
	BrakeOutput = 1.0f;
	SteeringOutput = 0.0f;
}

void AMGRacingAIController::SetTargetCheckpoint(AMGCheckpoint* Checkpoint)
{
	TargetCheckpoint = Checkpoint;
}

void AMGRacingAIController::SetRacePosition(int32 Position, int32 Total)
{
	CurrentPosition = Position;
	TotalRacers = Total;
}

void AMGRacingAIController::SetDistanceToLeader(float DistanceCm)
{
	DistanceToLeader = DistanceCm;
}

void AMGRacingAIController::SetRubberBandingConfig(const FMGRubberBandingConfig& Config)
{
	RubberBandingConfig = Config;
}

void AMGRacingAIController::UpdateState(float DeltaTime)
{
	if (CurrentState == EMGAIState::Waiting || CurrentState == EMGAIState::Finished)
	{
		return;
	}

	// Check for vehicles ahead
	float DistanceToVehicle = 0.0f;
	AActor* VehicleAhead = DetectVehicleAhead(DistanceToVehicle);

	// State transitions
	switch (CurrentState)
	{
	case EMGAIState::Racing:
		// Check if we should attempt overtake
		if (VehicleAhead && DistanceToVehicle < 1500.0f && OvertakeCooldown <= 0.0f)
		{
			if (ShouldAttemptOvertake(VehicleAhead, DistanceToVehicle))
			{
				SetState(EMGAIState::Overtaking);
				OvertakeCooldown = 3.0f;
			}
		}
		// Check if we're falling behind
		else if (CurrentPosition > TotalRacers * 0.5f && DriverProfile.bUseCatchup)
		{
			SetState(EMGAIState::CatchingUp);
		}
		break;

	case EMGAIState::Overtaking:
		// Complete overtake after a few seconds
		if (StateTime > 4.0f || !VehicleAhead || DistanceToVehicle > 2000.0f)
		{
			OnOvertakeAttempt.Broadcast(VehicleAhead, DistanceToVehicle > 1000.0f);
			SetState(EMGAIState::Racing);
		}
		break;

	case EMGAIState::CatchingUp:
		// Return to normal racing when caught up
		if (CurrentPosition <= TotalRacers * 0.4f)
		{
			SetState(EMGAIState::Racing);
		}
		break;

	case EMGAIState::Defending:
		// Defend for a few seconds then resume
		if (StateTime > 5.0f)
		{
			SetState(EMGAIState::Racing);
		}
		break;

	case EMGAIState::Recovering:
		// Recover for a short time
		if (StateTime > 2.0f)
		{
			SetState(EMGAIState::Racing);
		}
		break;

	default:
		break;
	}

	// Random mistakes
	if (ShouldMakeMistake() && MistakeCooldown <= 0.0f)
	{
		OnMakeMistake();
		MistakeCooldown = 5.0f + FMath::RandRange(0.0f, 10.0f);
	}
}

void AMGRacingAIController::UpdateNavigation(float DeltaTime)
{
	if (!ControlledVehicle)
	{
		return;
	}

	FVector VehicleLocation = ControlledVehicle->GetActorLocation();
	FVector VehicleForward = ControlledVehicle->GetActorForwardVector();

	// Get target from racing line or checkpoint
	FVector TargetLocation;

	if (RacingLineSpline.IsValid())
	{
		TargetLocation = GetRacingLineTarget(LookaheadDistance);
	}
	else if (TargetCheckpoint.IsValid())
	{
		TargetLocation = TargetCheckpoint->GetActorLocation();
	}
	else
	{
		// Fall back to forward direction
		TargetLocation = VehicleLocation + VehicleForward * LookaheadDistance;
	}

	// Apply personality-based line offset
	if (DriverProfile.PreferredLineOffset != 0.0f)
	{
		FVector Right = FVector::CrossProduct(FVector::UpVector, VehicleForward);
		TargetLocation += Right * DriverProfile.PreferredLineOffset * 200.0f;
	}

	// Update current target
	CurrentTarget.Location = TargetLocation;
	CurrentTarget.Distance = FVector::Dist(VehicleLocation, TargetLocation);
	CurrentTarget.TargetSpeed = GetTargetSpeedForSection();
}

void AMGRacingAIController::UpdateVehicleInputs(float DeltaTime)
{
	if (!ControlledVehicle)
	{
		return;
	}

	float CurrentSpeed = ControlledVehicle->GetVehicleSpeed();

	// Calculate base inputs
	float DesiredSteering = CalculateSteering(CurrentTarget.Location);
	float DesiredThrottle = CalculateThrottle(CurrentTarget.TargetSpeed, CurrentSpeed);
	float DesiredBrake = CalculateBrake(CurrentTarget.TargetSpeed, CurrentSpeed, CurrentTarget.Distance);

	// Apply difficulty modifiers
	DesiredThrottle *= DriverProfile.TopSpeedFactor;

	// Smooth steering
	SteeringOutput = FMath::FInterpTo(PreviousSteering, DesiredSteering, DeltaTime, SteeringSmoothSpeed);
	PreviousSteering = SteeringOutput;

	// Apply reaction delay (simple simulation)
	ThrottleOutput = FMath::FInterpTo(ThrottleOutput, DesiredThrottle, DeltaTime, 1.0f / DriverProfile.ReactionTime);
	BrakeOutput = FMath::FInterpTo(BrakeOutput, DesiredBrake, DeltaTime, 1.0f / DriverProfile.ReactionTime);

	// NOS decision
	bWantsNOS = ShouldActivateNOS();

	// Apply rubber-banding adjustment (works in all racing states)
	float RubberBandingAdjustment = CalculateCatchupBoost();
	if (!FMath::IsNearlyZero(RubberBandingAdjustment))
	{
		if (RubberBandingAdjustment > 0.0f)
		{
			// Catch-up boost - increase throttle
			ThrottleOutput = FMath::Min(1.0f, ThrottleOutput + RubberBandingAdjustment);
		}
		else
		{
			// Slow-down penalty - reduce throttle (but don't go below minimum)
			ThrottleOutput = FMath::Max(0.3f, ThrottleOutput + RubberBandingAdjustment);
		}
	}

	// State-specific modifiers
	switch (CurrentState)
	{
	case EMGAIState::Overtaking:
		// More aggressive when overtaking
		ThrottleOutput = FMath::Min(1.0f, ThrottleOutput * 1.1f);
		break;

	case EMGAIState::Defending:
		// Slightly defensive line adjustment happens in navigation
		break;

	case EMGAIState::Recovering:
		// Slow down during recovery
		ThrottleOutput *= 0.7f;
		break;

	default:
		break;
	}
}

void AMGRacingAIController::ApplyInputsToVehicle()
{
	if (!ControlledVehicle)
	{
		return;
	}

	// Apply inputs through vehicle interface
	ControlledVehicle->SetThrottleInput(ThrottleOutput);
	ControlledVehicle->SetBrakeInput(BrakeOutput);
	ControlledVehicle->SetSteeringInput(SteeringOutput);

	if (bWantsNOS)
	{
		ControlledVehicle->ActivateNOS();
	}
}

float AMGRacingAIController::CalculateSteering_Implementation(const FVector& TargetLocation)
{
	if (!ControlledVehicle)
	{
		return 0.0f;
	}

	FVector VehicleLocation = ControlledVehicle->GetActorLocation();
	FVector VehicleForward = ControlledVehicle->GetActorForwardVector();
	FVector ToTarget = (TargetLocation - VehicleLocation).GetSafeNormal();

	// Calculate angle to target
	float DotProduct = FVector::DotProduct(VehicleForward, ToTarget);
	FVector CrossProduct = FVector::CrossProduct(VehicleForward, ToTarget);
	float Angle = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));

	// Determine direction
	float Direction = FMath::Sign(CrossProduct.Z);

	// Scale based on angle
	float Steering = Direction * FMath::Min(Angle / FMath::DegreesToRadians(45.0f), 1.0f);

	// Apply skill modifier - less skilled = more oversteering
	float SkillModifier = 1.0f + (1.0f - DriverProfile.CorneringSkill) * 0.3f;
	Steering *= SkillModifier;

	return FMath::Clamp(Steering, -1.0f, 1.0f);
}

float AMGRacingAIController::CalculateThrottle_Implementation(float TargetSpeed, float CurrentSpeed)
{
	if (CurrentSpeed < TargetSpeed * 0.9f)
	{
		return 1.0f;
	}
	else if (CurrentSpeed < TargetSpeed)
	{
		return 0.5f + 0.5f * (1.0f - (CurrentSpeed / TargetSpeed));
	}
	else
	{
		// Above target speed
		return 0.3f;
	}
}

float AMGRacingAIController::CalculateBrake_Implementation(float TargetSpeed, float CurrentSpeed, float DistanceToCorner)
{
	// Need to brake if going too fast
	float SpeedDiff = CurrentSpeed - TargetSpeed;

	if (SpeedDiff <= 0.0f)
	{
		return 0.0f;
	}

	// Calculate braking intensity based on skill
	float BrakeIntensity = SpeedDiff / 50.0f; // 50 kph over = full brake
	BrakeIntensity *= (2.0f - DriverProfile.BrakingSkill); // Less skilled = harder braking

	return FMath::Clamp(BrakeIntensity, 0.0f, 1.0f);
}

bool AMGRacingAIController::ShouldAttemptOvertake_Implementation(AActor* VehicleAhead, float Distance)
{
	if (!VehicleAhead || Distance > 1500.0f)
	{
		return false;
	}

	// Base chance from aggression
	float OvertakeChance = DriverProfile.OvertakeAggression;

	// Modify by risk tolerance
	OvertakeChance *= (0.5f + DriverProfile.RiskTolerance * 0.5f);

	// More likely if close
	if (Distance < 500.0f)
	{
		OvertakeChance *= 1.5f;
	}

	// Less likely if just attempted
	if (OvertakeCooldown > 0.0f)
	{
		OvertakeChance *= 0.3f;
	}

	return FMath::FRand() < OvertakeChance;
}

bool AMGRacingAIController::ShouldActivateNOS_Implementation()
{
	if (!ControlledVehicle)
	{
		return false;
	}

	// Don't use in corners (simplified check using steering)
	if (FMath::Abs(SteeringOutput) > 0.3f)
	{
		return false;
	}

	// Use based on aggression
	float UseChance = DriverProfile.NOSAggression * 0.1f; // Low chance per frame

	// More likely when overtaking
	if (CurrentState == EMGAIState::Overtaking)
	{
		UseChance *= 3.0f;
	}

	// More likely when catching up
	if (CurrentState == EMGAIState::CatchingUp)
	{
		UseChance *= 2.0f;
	}

	return FMath::FRand() < UseChance;
}

void AMGRacingAIController::OnMakeMistake_Implementation()
{
	// Simulate a mistake - reduce control briefly
	SetState(EMGAIState::Recovering);
	OnMistake.Broadcast();
}

FVector AMGRacingAIController::GetRacingLineTarget(float LookaheadDist) const
{
	if (!RacingLineSpline.IsValid() || !ControlledVehicle)
	{
		return ControlledVehicle ? ControlledVehicle->GetActorLocation() + ControlledVehicle->GetActorForwardVector() * LookaheadDist : FVector::ZeroVector;
	}

	USplineComponent* Spline = RacingLineSpline.Get();
	FVector VehicleLocation = ControlledVehicle->GetActorLocation();

	// Find closest point on spline
	float ClosestInputKey = Spline->FindInputKeyClosestToWorldLocation(VehicleLocation);
	float SplineLength = Spline->GetSplineLength();

	// Convert to distance
	float CurrentDistance = Spline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);

	// Get point ahead
	float TargetDistance = FMath::Fmod(CurrentDistance + LookaheadDist, SplineLength);
	return Spline->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
}

AActor* AMGRacingAIController::DetectVehicleAhead(float& OutDistance) const
{
	if (!ControlledVehicle)
	{
		return nullptr;
	}

	// Simple raycast forward
	FVector Start = ControlledVehicle->GetActorLocation();
	FVector Forward = ControlledVehicle->GetActorForwardVector();
	FVector End = Start + Forward * VehicleDetectionRange;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ControlledVehicle);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Vehicle, Params))
	{
		OutDistance = HitResult.Distance;
		return HitResult.GetActor();
	}

	OutDistance = VehicleDetectionRange;
	return nullptr;
}

float AMGRacingAIController::CalculateCatchupBoost() const
{
	if (!DriverProfile.bUseCatchup || TotalRacers <= 1)
	{
		return 0.0f;
	}

	// Calculate difficulty scaling factor (harder difficulty = less help)
	float DifficultyFactor = 1.0f;
	if (RubberBandingConfig.DifficultyScaling > 0.0f)
	{
		// Scale from 1.0 (Rookie) to 0.0 (Legend)
		float DifficultyRatio = static_cast<float>(DriverProfile.Difficulty) / static_cast<float>(EMGAIDifficulty::Legend);
		DifficultyFactor = 1.0f - (DifficultyRatio * RubberBandingConfig.DifficultyScaling);
	}

	float Adjustment = 0.0f;

	// Distance-based catch-up (behind leader)
	if (RubberBandingConfig.bEnableCatchUp && DistanceToLeader > RubberBandingConfig.CatchUpDistanceThreshold)
	{
		// Calculate how far into the catch-up range we are
		float DistanceIntoRange = DistanceToLeader - RubberBandingConfig.CatchUpDistanceThreshold;
		float MaxRange = RubberBandingConfig.MaxCatchUpDistance - RubberBandingConfig.CatchUpDistanceThreshold;
		float CatchUpRatio = FMath::Clamp(DistanceIntoRange / MaxRange, 0.0f, 1.0f);

		// Apply boost (quadratic curve for more natural feel)
		Adjustment = CatchUpRatio * CatchUpRatio * RubberBandingConfig.MaxCatchUpBoost;
	}
	// Distance-based slow-down (ahead of pack)
	else if (RubberBandingConfig.bEnableSlowDown && DistanceToLeader < -RubberBandingConfig.SlowDownDistanceThreshold)
	{
		// We're the leader and far ahead - slow down
		float DistanceAhead = FMath::Abs(DistanceToLeader) - RubberBandingConfig.SlowDownDistanceThreshold;
		float MaxSlowDownRange = RubberBandingConfig.MaxCatchUpDistance; // Use same range for symmetry
		float SlowDownRatio = FMath::Clamp(DistanceAhead / MaxSlowDownRange, 0.0f, 1.0f);

		// Apply penalty (negative adjustment)
		Adjustment = -SlowDownRatio * RubberBandingConfig.MaxSlowDownPenalty;
	}

	// Also consider position-based adjustment for races where distance isn't tracked
	if (FMath::IsNearlyZero(DistanceToLeader))
	{
		float PositionRatio = static_cast<float>(CurrentPosition) / TotalRacers;
		if (PositionRatio > 0.5f && RubberBandingConfig.bEnableCatchUp)
		{
			// Fallback to position-based catch-up
			Adjustment = FMath::Max(Adjustment, (PositionRatio - 0.5f) * 0.2f * RubberBandingConfig.MaxCatchUpBoost / 0.15f);
		}
		else if (CurrentPosition == 1 && TotalRacers > 2 && RubberBandingConfig.bEnableSlowDown)
		{
			// Leader penalty
			Adjustment = FMath::Min(Adjustment, -RubberBandingConfig.MaxSlowDownPenalty * 0.5f);
		}
	}

	// Apply difficulty scaling
	Adjustment *= DifficultyFactor;

	// Store for debugging/queries
	const_cast<AMGRacingAIController*>(this)->CurrentRubberBandingAdjustment = Adjustment;

	return Adjustment;
}

bool AMGRacingAIController::ShouldMakeMistake() const
{
	// Probability based on consistency
	float MistakeChance = (1.0f - DriverProfile.Consistency) * 0.005f; // Per frame

	// Increase chance in high-stress situations
	if (CurrentState == EMGAIState::Overtaking || CurrentState == EMGAIState::Defending)
	{
		MistakeChance *= 2.0f;
	}

	return FMath::FRand() < MistakeChance;
}

void AMGRacingAIController::SetState(EMGAIState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		StateTime = 0.0f;
		OnAIStateChanged.Broadcast(NewState);
	}
}

float AMGRacingAIController::GetTargetSpeedForSection() const
{
	// Base target speed (would come from racing line data in real implementation)
	float BaseSpeed = 200.0f; // kph

	// Modify by skill
	BaseSpeed *= (0.7f + DriverProfile.SkillRating * 0.3f);

	// Modify by top speed factor
	BaseSpeed *= DriverProfile.TopSpeedFactor;

	return BaseSpeed;
}

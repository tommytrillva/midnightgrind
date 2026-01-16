// Copyright Midnight Grind. All Rights Reserved.

#include "AI/MGAIRacerController.h"
#include "AI/MGAIDriverProfile.h"
#include "Track/MGTrackSubsystem.h"
#include "Core/MGRaceGameMode.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AMGAIRacerController::AMGAIRacerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AMGAIRacerController::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		TrackSubsystem = World->GetSubsystem<UMGTrackSubsystem>();
		RaceGameMode = Cast<AMGRaceGameMode>(UGameplayStatics::GetGameMode(World));
	}
}

void AMGAIRacerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!VehiclePawn || CurrentState == EMGAIDrivingState::Waiting || CurrentState == EMGAIDrivingState::Finished)
	{
		return;
	}

	// Update subsystems
	UpdatePerception();
	UpdateRacingLineProgress();

	// State machine
	UpdateStateMachine(DeltaTime);

	// Calculate and apply steering
	CalculateSteering(DeltaTime);
	ApplySteering();

	TimeInState += DeltaTime;
}

void AMGAIRacerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	VehiclePawn = InPawn;
}

void AMGAIRacerController::OnUnPossess()
{
	VehiclePawn = nullptr;
	Super::OnUnPossess();
}

// ==========================================
// CONFIGURATION
// ==========================================

void AMGAIRacerController::SetDriverProfile(UMGAIDriverProfile* Profile)
{
	DriverProfile = Profile;
}

void AMGAIRacerController::SetDifficultyMultiplier(float Multiplier)
{
	DifficultyMultiplier = FMath::Clamp(Multiplier, 0.5f, 1.5f);
}

void AMGAIRacerController::SetRubberBandingEnabled(bool bEnabled)
{
	bRubberBandingEnabled = bEnabled;
}

void AMGAIRacerController::SetRacingLine(const TArray<FMGAIRacingLinePoint>& RacingLine)
{
	RacingLinePoints = RacingLine;
	CurrentRacingLineIndex = 0;
}

// ==========================================
// STATE
// ==========================================

float AMGAIRacerController::GetCurrentSpeed() const
{
	if (VehiclePawn)
	{
		return VehiclePawn->GetVelocity().Size();
	}
	return 0.0f;
}

float AMGAIRacerController::GetDistanceToRacingLine() const
{
	if (RacingLinePoints.Num() == 0 || !VehiclePawn)
	{
		return 0.0f;
	}

	FVector CurrentPos = VehiclePawn->GetActorLocation();
	int32 ClosestIndex = FindClosestRacingLinePoint(CurrentPos);

	if (ClosestIndex >= 0 && ClosestIndex < RacingLinePoints.Num())
	{
		return FVector::Dist(CurrentPos, RacingLinePoints[ClosestIndex].Position);
	}

	return 0.0f;
}

// ==========================================
// CONTROL
// ==========================================

void AMGAIRacerController::StartRacing()
{
	SetState(EMGAIDrivingState::Racing);
}

void AMGAIRacerController::StopRacing()
{
	SetState(EMGAIDrivingState::Finished);
}

void AMGAIRacerController::ForceState(EMGAIDrivingState NewState)
{
	SetState(NewState);
}

void AMGAIRacerController::NotifyCollision(AActor* OtherActor, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	// If significant collision, enter recovery
	if (CurrentState == EMGAIDrivingState::Racing || CurrentState == EMGAIDrivingState::Overtaking)
	{
		SetState(EMGAIDrivingState::Recovering);
		RecoveryTimer = 1.5f;
	}
}

void AMGAIRacerController::NotifyOffTrack()
{
	if (CurrentState != EMGAIDrivingState::Recovering)
	{
		SetState(EMGAIDrivingState::Recovering);
		RecoveryTimer = 2.0f;
	}
}

// ==========================================
// CORE LOGIC
// ==========================================

void AMGAIRacerController::UpdatePerception()
{
	PerceivedVehicles.Empty();

	if (!VehiclePawn)
	{
		return;
	}

	FVector MyLocation = VehiclePawn->GetActorLocation();
	FVector MyVelocity = VehiclePawn->GetVelocity();
	FVector MyForward = VehiclePawn->GetActorForwardVector();

	// Find all vehicles in perception radius
	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

	for (AActor* Actor : NearbyActors)
	{
		if (Actor == VehiclePawn)
		{
			continue;
		}

		APawn* OtherPawn = Cast<APawn>(Actor);
		if (!OtherPawn)
		{
			continue;
		}

		FVector OtherLocation = OtherPawn->GetActorLocation();
		float Distance = FVector::Dist(MyLocation, OtherLocation);

		if (Distance > PerceptionRadius * 100.0f) // Convert to cm
		{
			continue;
		}

		FMGAIVehiclePerception Perception;
		Perception.Vehicle = OtherPawn;
		Perception.RelativePosition = OtherLocation - MyLocation;
		Perception.RelativeVelocity = OtherPawn->GetVelocity() - MyVelocity;
		Perception.Distance = Distance;

		// Calculate angle
		FVector ToOther = Perception.RelativePosition.GetSafeNormal();
		float DotForward = FVector::DotProduct(MyForward, ToOther);
		float DotRight = FVector::DotProduct(VehiclePawn->GetActorRightVector(), ToOther);
		Perception.Angle = FMath::RadiansToDegrees(FMath::Atan2(DotRight, DotForward));

		Perception.bIsAhead = DotForward > 0.0f;
		Perception.bIsOnLeft = DotRight < 0.0f;

		// Time to collision
		float ClosingSpeed = -FVector::DotProduct(Perception.RelativeVelocity, ToOther);
		if (ClosingSpeed > 10.0f)
		{
			Perception.TimeToCollision = Distance / ClosingSpeed;
		}

		// Check if player
		APlayerController* PC = Cast<APlayerController>(OtherPawn->GetController());
		Perception.bIsPlayer = PC != nullptr;

		PerceivedVehicles.Add(Perception);
	}

	// Sort by distance
	PerceivedVehicles.Sort([](const FMGAIVehiclePerception& A, const FMGAIVehiclePerception& B)
	{
		return A.Distance < B.Distance;
	});
}

void AMGAIRacerController::UpdateStateMachine(float DeltaTime)
{
	switch (CurrentState)
	{
		case EMGAIDrivingState::Racing:
			HandleRacingState(DeltaTime);
			break;

		case EMGAIDrivingState::Overtaking:
			HandleOvertakingState(DeltaTime);
			break;

		case EMGAIDrivingState::Defending:
			HandleDefendingState(DeltaTime);
			break;

		case EMGAIDrivingState::Recovering:
			HandleRecoveringState(DeltaTime);
			break;

		case EMGAIDrivingState::CatchingUp:
			HandleCatchingUpState(DeltaTime);
			break;

		case EMGAIDrivingState::SlowingDown:
			HandleSlowingDownState(DeltaTime);
			break;

		default:
			break;
	}
}

void AMGAIRacerController::CalculateSteering(float DeltaTime)
{
	switch (CurrentState)
	{
		case EMGAIDrivingState::Racing:
		case EMGAIDrivingState::CatchingUp:
		case EMGAIDrivingState::SlowingDown:
			CurrentSteering = CalculateRacingLineSteering();
			break;

		case EMGAIDrivingState::Overtaking:
			CurrentSteering = CalculateOvertakeSteering();
			break;

		case EMGAIDrivingState::Defending:
			CurrentSteering = CalculateDefenseSteering();
			break;

		case EMGAIDrivingState::Recovering:
			CurrentSteering = CalculateRecoverySteering();
			break;

		default:
			CurrentSteering = FMGAISteeringOutput();
			break;
	}

	// Calculate target speed
	CurrentTargetSpeed = CalculateTargetSpeed();

	// Apply rubber banding
	if (bRubberBandingEnabled)
	{
		CurrentTargetSpeed += CalculateRubberBandAdjustment();
	}

	// Apply profile modifiers
	if (DriverProfile)
	{
		ApplyProfileModifiers(CurrentSteering);
	}
}

void AMGAIRacerController::ApplySteering()
{
	if (!VehiclePawn)
	{
		return;
	}

	// Apply inputs to vehicle
	// This would interface with the vehicle's input component
	// For now, we store the values and let the vehicle read them

	// Vehicle would call GetSteeringOutput() to get these values
}

void AMGAIRacerController::UpdateRacingLineProgress()
{
	if (RacingLinePoints.Num() == 0 || !VehiclePawn)
	{
		return;
	}

	FVector CurrentPos = VehiclePawn->GetActorLocation();
	int32 ClosestIndex = FindClosestRacingLinePoint(CurrentPos);

	// Only move forward (prevent going backwards on track)
	if (ClosestIndex > CurrentRacingLineIndex)
	{
		CurrentRacingLineIndex = ClosestIndex;
	}

	// Handle lap wrap
	if (CurrentRacingLineIndex == 0 && RacingLinePoints.Num() > 10)
	{
		int32 LastIndex = RacingLinePoints.Num() - 1;
		float DistToLast = FVector::Dist(CurrentPos, RacingLinePoints[LastIndex].Position);
		float DistToFirst = FVector::Dist(CurrentPos, RacingLinePoints[0].Position);

		if (DistToLast < DistToFirst)
		{
			CurrentRacingLineIndex = LastIndex;
		}
	}
}

// ==========================================
// STATE HANDLERS
// ==========================================

void AMGAIRacerController::HandleWaitingState(float DeltaTime)
{
	// Wait for race start signal
}

void AMGAIRacerController::HandleRacingState(float DeltaTime)
{
	// Check for overtake opportunity
	if (ShouldAttemptOvertake())
	{
		FMGAIVehiclePerception Ahead = GetVehicleAhead();
		if (Ahead.Vehicle)
		{
			// Determine overtake side
			bOvertakeOnLeft = IsOvertakePathClear(true);
			if (!bOvertakeOnLeft && !IsOvertakePathClear(false))
			{
				// No clear path, stay behind
				return;
			}

			SetState(EMGAIDrivingState::Overtaking);
			OvertakeTimer = 0.0f;
		}
	}

	// Check if we should defend
	if (ShouldDefendPosition())
	{
		SetState(EMGAIDrivingState::Defending);
	}
}

void AMGAIRacerController::HandleOvertakingState(float DeltaTime)
{
	OvertakeTimer += DeltaTime;

	// Timeout overtake
	float MaxOvertakeTime = DriverProfile ? DriverProfile->Aggression.OvertakePatience * 2.0f : 6.0f;
	if (OvertakeTimer > MaxOvertakeTime)
	{
		SetState(EMGAIDrivingState::Racing);
		return;
	}

	// Check if overtake complete
	FMGAIVehiclePerception Ahead = GetVehicleAhead();
	if (!Ahead.Vehicle || !Ahead.bIsAhead)
	{
		// We passed them
		SetState(EMGAIDrivingState::Racing);
	}
}

void AMGAIRacerController::HandleDefendingState(float DeltaTime)
{
	// Check if still need to defend
	FMGAIVehiclePerception Behind = GetVehicleBehind();
	if (!Behind.Vehicle || Behind.Distance > 20.0f * 100.0f) // 20m in cm
	{
		SetState(EMGAIDrivingState::Racing);
	}
}

void AMGAIRacerController::HandleRecoveringState(float DeltaTime)
{
	RecoveryTimer -= DeltaTime;
	if (RecoveryTimer <= 0.0f)
	{
		SetState(EMGAIDrivingState::Racing);
	}
}

void AMGAIRacerController::HandleCatchingUpState(float DeltaTime)
{
	// Automatically handled by rubber banding in speed calc
	// Switch back to racing after catching up
	APawn* PlayerVehicle = GetPlayerVehicle();
	if (PlayerVehicle && VehiclePawn)
	{
		float DistToPlayer = FVector::Dist(VehiclePawn->GetActorLocation(), PlayerVehicle->GetActorLocation());
		if (DistToPlayer < 30.0f * 100.0f) // Within 30m
		{
			SetState(EMGAIDrivingState::Racing);
		}
	}
}

void AMGAIRacerController::HandleSlowingDownState(float DeltaTime)
{
	// Switch back to racing if player catches up
	APawn* PlayerVehicle = GetPlayerVehicle();
	if (PlayerVehicle && VehiclePawn)
	{
		float DistToPlayer = FVector::Dist(VehiclePawn->GetActorLocation(), PlayerVehicle->GetActorLocation());
		if (DistToPlayer < 50.0f * 100.0f) // Within 50m
		{
			SetState(EMGAIDrivingState::Racing);
		}
	}
}

// ==========================================
// STEERING CALCULATIONS
// ==========================================

FMGAISteeringOutput AMGAIRacerController::CalculateRacingLineSteering()
{
	FMGAISteeringOutput Output;

	if (RacingLinePoints.Num() == 0 || !VehiclePawn)
	{
		return Output;
	}

	// Get target point ahead on racing line
	FMGAIRacingLinePoint TargetPoint = GetRacingLinePointAhead(SteeringLookAhead);
	Output.TargetPoint = TargetPoint.Position;

	// Add avoidance offset
	FVector AvoidanceOffset = CalculateAvoidanceOffset();
	Output.TargetPoint += AvoidanceOffset;

	// Calculate steering angle
	FVector ToTarget = Output.TargetPoint - VehiclePawn->GetActorLocation();
	ToTarget.Z = 0;
	ToTarget.Normalize();

	FVector Forward = VehiclePawn->GetActorForwardVector();
	Forward.Z = 0;
	Forward.Normalize();

	float DotRight = FVector::DotProduct(VehiclePawn->GetActorRightVector(), ToTarget);
	float DotForward = FVector::DotProduct(Forward, ToTarget);

	// PID steering
	float SteeringError = FMath::Atan2(DotRight, FMath::Max(DotForward, 0.1f));
	SteeringErrorIntegral += SteeringError * GetWorld()->GetDeltaSeconds();
	SteeringErrorIntegral = FMath::Clamp(SteeringErrorIntegral, -1.0f, 1.0f);

	float SteeringDerivative = (SteeringError - LastSteeringError) / FMath::Max(GetWorld()->GetDeltaSeconds(), 0.001f);
	LastSteeringError = SteeringError;

	float Kp = 2.0f;
	float Ki = 0.1f;
	float Kd = 0.5f;

	Output.Steering = Kp * SteeringError + Ki * SteeringErrorIntegral + Kd * SteeringDerivative;
	Output.Steering = FMath::Clamp(Output.Steering, -1.0f, 1.0f);

	// Add noise for personality
	Output.Steering = AddSteeringNoise(Output.Steering);

	// Calculate throttle/brake
	float CurrentSpeed = GetCurrentSpeed();
	float SpeedDiff = CurrentTargetSpeed - CurrentSpeed;

	if (SpeedDiff > 50.0f)
	{
		Output.Throttle = 1.0f;
		Output.Brake = 0.0f;
	}
	else if (SpeedDiff < -100.0f)
	{
		Output.Throttle = 0.0f;
		Output.Brake = FMath::Clamp(-SpeedDiff / 500.0f, 0.0f, 1.0f);
	}
	else
	{
		Output.Throttle = FMath::Clamp(SpeedDiff / 200.0f + 0.5f, 0.0f, 1.0f);
		Output.Brake = 0.0f;
	}

	// Brake in braking zones
	FMGAIRacingLinePoint AheadPoint = GetRacingLinePointAhead(SpeedLookAhead);
	if (AheadPoint.bIsBrakingZone && CurrentSpeed > AheadPoint.TargetSpeed * 100.0f)
	{
		Output.Throttle = 0.0f;
		Output.Brake = 0.8f;
	}

	// NOS usage
	if (DriverProfile && FMath::FRand() < DriverProfile->Speed.NOSUsageFrequency * 0.01f)
	{
		if (AheadPoint.bIsAccelerationZone && !AheadPoint.bIsBrakingZone)
		{
			Output.bNOS = true;
		}
	}

	return Output;
}

FMGAISteeringOutput AMGAIRacerController::CalculateOvertakeSteering()
{
	FMGAISteeringOutput Output = CalculateRacingLineSteering();

	// Offset target to overtake side
	float OvertakeOffset = bOvertakeOnLeft ? -400.0f : 400.0f;

	if (VehiclePawn)
	{
		FVector RightVector = VehiclePawn->GetActorRightVector();
		Output.TargetPoint += RightVector * OvertakeOffset;
	}

	// More aggressive throttle during overtake
	Output.Throttle = FMath::Min(Output.Throttle + 0.2f, 1.0f);

	// Use NOS if available during overtake
	if (DriverProfile && DriverProfile->Aggression.OvertakeAggression > 0.7f)
	{
		Output.bNOS = true;
	}

	return Output;
}

FMGAISteeringOutput AMGAIRacerController::CalculateDefenseSteering()
{
	FMGAISteeringOutput Output = CalculateRacingLineSteering();

	// Check where attacker is and block
	FMGAIVehiclePerception Behind = GetVehicleBehind();
	if (Behind.Vehicle)
	{
		float BlockOffset = Behind.bIsOnLeft ? -200.0f : 200.0f;

		if (VehiclePawn)
		{
			FVector RightVector = VehiclePawn->GetActorRightVector();
			Output.TargetPoint += RightVector * BlockOffset;
		}
	}

	return Output;
}

FMGAISteeringOutput AMGAIRacerController::CalculateRecoverySteering()
{
	// Steer back toward racing line
	FMGAISteeringOutput Output;

	if (RacingLinePoints.Num() == 0 || !VehiclePawn)
	{
		return Output;
	}

	int32 ClosestIndex = FindClosestRacingLinePoint(VehiclePawn->GetActorLocation());
	if (ClosestIndex >= 0)
	{
		Output.TargetPoint = RacingLinePoints[ClosestIndex].Position;
	}

	// Calculate steering
	FVector ToTarget = Output.TargetPoint - VehiclePawn->GetActorLocation();
	ToTarget.Z = 0;
	ToTarget.Normalize();

	FVector Forward = VehiclePawn->GetActorForwardVector();
	Forward.Z = 0;
	Forward.Normalize();

	float DotRight = FVector::DotProduct(VehiclePawn->GetActorRightVector(), ToTarget);
	Output.Steering = FMath::Clamp(DotRight * 2.0f, -1.0f, 1.0f);

	// Gentle throttle during recovery
	Output.Throttle = 0.5f;
	Output.Brake = 0.0f;

	return Output;
}

FVector AMGAIRacerController::CalculateAvoidanceOffset()
{
	FVector Offset = FVector::ZeroVector;

	if (!VehiclePawn)
	{
		return Offset;
	}

	for (const FMGAIVehiclePerception& Perception : PerceivedVehicles)
	{
		// Only avoid vehicles ahead and close
		if (!Perception.bIsAhead || Perception.Distance > 15.0f * 100.0f)
		{
			continue;
		}

		// Calculate avoidance direction
		float AvoidanceStrength = 1.0f - (Perception.Distance / (15.0f * 100.0f));
		AvoidanceStrength = FMath::Square(AvoidanceStrength);

		// Avoid to opposite side
		float AvoidDirection = Perception.bIsOnLeft ? 1.0f : -1.0f;
		Offset += VehiclePawn->GetActorRightVector() * AvoidDirection * AvoidanceStrength * 300.0f;
	}

	return Offset;
}

float AMGAIRacerController::CalculateTargetSpeed()
{
	if (RacingLinePoints.Num() == 0)
	{
		return 50.0f * 100.0f; // Default 50 m/s in cm/s
	}

	// Get target speed from racing line
	FMGAIRacingLinePoint CurrentPoint = GetRacingLinePointAhead(0.0f);
	FMGAIRacingLinePoint AheadPoint = GetRacingLinePointAhead(SpeedLookAhead);

	// Use lower of current and ahead target speeds
	float BaseSpeed = FMath::Min(CurrentPoint.TargetSpeed, AheadPoint.TargetSpeed);

	// Apply difficulty
	BaseSpeed *= DifficultyMultiplier;

	// Apply profile modifiers
	if (DriverProfile)
	{
		BaseSpeed *= DriverProfile->Speed.BaseSpeedPercent;

		if (CurrentPoint.bIsApex)
		{
			BaseSpeed *= DriverProfile->Speed.CornerSpeedMultiplier;
		}
		else if (!CurrentPoint.bIsBrakingZone && !CurrentPoint.bIsApex)
		{
			BaseSpeed *= DriverProfile->Speed.StraightSpeedMultiplier;
		}
	}

	return BaseSpeed * 100.0f; // Convert to cm/s
}

float AMGAIRacerController::CalculateRubberBandAdjustment()
{
	APawn* PlayerVehicle = GetPlayerVehicle();
	if (!PlayerVehicle || !VehiclePawn)
	{
		return 0.0f;
	}

	// Calculate distance to player (positive = ahead, negative = behind)
	float MyProgress = RacingLineProgress;
	// Would need to get player progress from race manager

	// For now, use simple distance
	float DistToPlayer = FVector::Dist(VehiclePawn->GetActorLocation(), PlayerVehicle->GetActorLocation());
	FVector ToPlayer = PlayerVehicle->GetActorLocation() - VehiclePawn->GetActorLocation();
	bool bAheadOfPlayer = FVector::DotProduct(VehiclePawn->GetActorForwardVector(), ToPlayer) < 0;

	float Adjustment = 0.0f;

	if (bAheadOfPlayer && DistToPlayer > 100.0f * 100.0f) // More than 100m ahead
	{
		// Slow down
		float SlowAmount = DriverProfile ? DriverProfile->Speed.SlowDownAmount : 0.05f;
		Adjustment = -CurrentTargetSpeed * SlowAmount * RubberBandStrength;

		if (CurrentState != EMGAIDrivingState::SlowingDown)
		{
			SetState(EMGAIDrivingState::SlowingDown);
		}
	}
	else if (!bAheadOfPlayer && DistToPlayer > 50.0f * 100.0f) // More than 50m behind
	{
		// Speed up
		float CatchUpAmount = DriverProfile ? DriverProfile->Speed.CatchUpBoost : 0.1f;
		Adjustment = CurrentTargetSpeed * CatchUpAmount * RubberBandStrength;

		if (CurrentState != EMGAIDrivingState::CatchingUp)
		{
			SetState(EMGAIDrivingState::CatchingUp);
		}
	}

	return Adjustment;
}

// ==========================================
// UTILITIES
// ==========================================

FMGAIRacingLinePoint AMGAIRacerController::GetRacingLinePointAhead(float Distance) const
{
	if (RacingLinePoints.Num() == 0)
	{
		return FMGAIRacingLinePoint();
	}

	// Find point at distance ahead
	float DistanceAccum = 0.0f;
	int32 Index = CurrentRacingLineIndex;

	while (DistanceAccum < Distance && Index < RacingLinePoints.Num() - 1)
	{
		DistanceAccum += FVector::Dist(RacingLinePoints[Index].Position, RacingLinePoints[Index + 1].Position);
		Index++;
	}

	// Wrap around for circuits
	if (Index >= RacingLinePoints.Num())
	{
		Index = 0;
	}

	return RacingLinePoints[Index];
}

int32 AMGAIRacerController::FindClosestRacingLinePoint(const FVector& Position) const
{
	if (RacingLinePoints.Num() == 0)
	{
		return -1;
	}

	int32 ClosestIndex = 0;
	float ClosestDist = FLT_MAX;

	// Search around current index first for efficiency
	int32 SearchStart = FMath::Max(0, CurrentRacingLineIndex - 10);
	int32 SearchEnd = FMath::Min(RacingLinePoints.Num() - 1, CurrentRacingLineIndex + 20);

	for (int32 i = SearchStart; i <= SearchEnd; i++)
	{
		float Dist = FVector::DistSquared(Position, RacingLinePoints[i].Position);
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			ClosestIndex = i;
		}
	}

	return ClosestIndex;
}

bool AMGAIRacerController::ShouldAttemptOvertake() const
{
	FMGAIVehiclePerception Ahead = GetVehicleAhead();
	if (!Ahead.Vehicle)
	{
		return false;
	}

	// Check if close enough to consider overtake
	if (Ahead.Distance > 15.0f * 100.0f) // 15m
	{
		return false;
	}

	// Check aggression threshold
	float OvertakeChance = DriverProfile ? DriverProfile->Aggression.OvertakeAggression : 0.5f;
	OvertakeChance *= DifficultyMultiplier;

	// Higher chance if stuck behind for a while
	if (TimeInState > 3.0f && CurrentState == EMGAIDrivingState::Racing)
	{
		OvertakeChance += 0.2f;
	}

	return FMath::FRand() < OvertakeChance * 0.1f; // Check every tick, so reduce chance
}

bool AMGAIRacerController::ShouldDefendPosition() const
{
	FMGAIVehiclePerception Behind = GetVehicleBehind();
	if (!Behind.Vehicle)
	{
		return false;
	}

	// Check if close enough to defend
	if (Behind.Distance > 10.0f * 100.0f) // 10m
	{
		return false;
	}

	float DefendChance = DriverProfile ? DriverProfile->Aggression.DefenseAggression : 0.5f;

	// Higher chance against player
	if (Behind.bIsPlayer && DriverProfile && DriverProfile->Aggression.bTargetsPlayer)
	{
		DefendChance += 0.3f;
	}

	return FMath::FRand() < DefendChance * 0.05f;
}

APawn* AMGAIRacerController::GetPlayerVehicle() const
{
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			return PC->GetPawn();
		}
	}
	return nullptr;
}

FMGAIVehiclePerception AMGAIRacerController::GetVehicleAhead() const
{
	for (const FMGAIVehiclePerception& Perception : PerceivedVehicles)
	{
		if (Perception.bIsAhead && FMath::Abs(Perception.Angle) < 45.0f)
		{
			return Perception;
		}
	}
	return FMGAIVehiclePerception();
}

FMGAIVehiclePerception AMGAIRacerController::GetVehicleBehind() const
{
	for (const FMGAIVehiclePerception& Perception : PerceivedVehicles)
	{
		if (!Perception.bIsAhead && FMath::Abs(Perception.Angle) > 135.0f)
		{
			return Perception;
		}
	}
	return FMGAIVehiclePerception();
}

bool AMGAIRacerController::IsOvertakePathClear(bool bOnLeft) const
{
	for (const FMGAIVehiclePerception& Perception : PerceivedVehicles)
	{
		if (Perception.Distance > 20.0f * 100.0f)
		{
			continue;
		}

		if ((bOnLeft && Perception.bIsOnLeft) || (!bOnLeft && !Perception.bIsOnLeft))
		{
			if (FMath::Abs(Perception.Angle) < 90.0f)
			{
				return false;
			}
		}
	}
	return true;
}

void AMGAIRacerController::ApplyProfileModifiers(FMGAISteeringOutput& Output)
{
	if (!DriverProfile)
	{
		return;
	}

	// Apply skill-based inaccuracy
	float Inaccuracy = 1.0f - DriverProfile->Skill.LineAccuracy;
	Output.Steering += FMath::FRandRange(-Inaccuracy, Inaccuracy) * 0.1f;
	Output.Steering = FMath::Clamp(Output.Steering, -1.0f, 1.0f);

	// Braking accuracy
	float BrakeInaccuracy = 1.0f - DriverProfile->Skill.BrakingAccuracy;
	Output.Brake *= 1.0f + FMath::FRandRange(-BrakeInaccuracy, BrakeInaccuracy) * 0.2f;
	Output.Brake = FMath::Clamp(Output.Brake, 0.0f, 1.0f);

	// Random mistakes
	if (FMath::FRand() < DriverProfile->Skill.MistakeFrequency * 0.01f)
	{
		// Throttle lift or brake error
		if (FMath::RandBool())
		{
			Output.Throttle *= 0.5f;
		}
		else
		{
			Output.Steering += FMath::FRandRange(-0.3f, 0.3f);
		}
	}
}

void AMGAIRacerController::SetState(EMGAIDrivingState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		TimeInState = 0.0f;
	}
}

float AMGAIRacerController::AddSteeringNoise(float BaseValue)
{
	if (!DriverProfile)
	{
		return BaseValue;
	}

	// Add personality-based noise
	float NoiseAmount = 0.0f;

	switch (DriverProfile->Personality)
	{
		case EMGDriverPersonality::Unpredictable:
			NoiseAmount = 0.1f;
			break;
		case EMGDriverPersonality::Rookie:
			NoiseAmount = 0.05f;
			break;
		default:
			NoiseAmount = 0.02f;
			break;
	}

	NoiseAmount *= (1.0f - DriverProfile->Skill.Consistency);

	return BaseValue + FMath::FRandRange(-NoiseAmount, NoiseAmount);
}

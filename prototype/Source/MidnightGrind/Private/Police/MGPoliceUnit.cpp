// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPoliceUnit.cpp
 * @brief Implementation of police vehicle AI unit for pursuit mechanics.
 *
 * This file implements the state-driven police AI including pursuit tactics,
 * PIT maneuvers, ramming, boxing formations, and intercept behaviors.
 */

#include "Police/MGPoliceUnit.h"
#include "Vehicle/MGVehicleMovementComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// ============================================================================
// CONSTRUCTION
// ============================================================================

AMGPoliceUnit::AMGPoliceUnit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMGVehicleMovementComponent>(AWheeledVehiclePawn::VehicleMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	SetupComponents();

	Health = MaxHealth;
}

void AMGPoliceUnit::SetupComponents()
{
	// Create siren audio component
	SirenAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("SirenAudio"));
	SirenAudio->SetupAttachment(GetMesh());
	SirenAudio->bAutoActivate = false;

	// Create engine audio component
	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->SetupAttachment(GetMesh());
	EngineAudio->bAutoActivate = true;

	// Create light bar VFX (Niagara)
	LightBarVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightBarVFX"));
	LightBarVFX->SetupAttachment(GetMesh());
	LightBarVFX->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	LightBarVFX->bAutoActivate = false;
}

void AMGPoliceUnit::BindCollisionEvents()
{
	if (UPrimitiveComponent* MeshComp = Cast<UPrimitiveComponent>(GetMesh()))
	{
		MeshComp->OnComponentHit.AddDynamic(this, &AMGPoliceUnit::OnCollisionHit);
	}
}

// ============================================================================
// ACTOR LIFECYCLE
// ============================================================================

void AMGPoliceUnit::BeginPlay()
{
	Super::BeginPlay();

	BindCollisionEvents();
}

void AMGPoliceUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update visual tracking for target
	UpdateVisualOnTarget();

	// State machine - update based on current AI state
	switch (CurrentState)
	{
		case EMGPoliceState::Patrolling:
			UpdatePatrol(DeltaTime);
			break;

		case EMGPoliceState::Alerted:
		case EMGPoliceState::Pursuing:
			UpdatePursuit(DeltaTime);
			break;

		case EMGPoliceState::Ramming:
			UpdateRamming(DeltaTime);
			break;

		case EMGPoliceState::BoxingIn:
			UpdateBoxing(DeltaTime);
			break;

		case EMGPoliceState::PITManeuver:
			UpdatePIT(DeltaTime);
			break;

		case EMGPoliceState::Intercepting:
			UpdateIntercept(DeltaTime);
			break;

		case EMGPoliceState::Disabled:
			UpdateDisabled(DeltaTime);
			break;

		case EMGPoliceState::Idle:
		case EMGPoliceState::Despawning:
		default:
			break;
	}

	// Update cooldown timers
	TimeSinceLastRam += DeltaTime;
	TimeSinceLastPIT += DeltaTime;
}

// ============================================================================
// UNIT CONFIGURATION
// ============================================================================

void AMGPoliceUnit::InitializeUnit(int32 InUnitID, EMGPoliceState InitialState)
{
	UnitID = InUnitID;
	Health = MaxHealth;
	TimeSinceLastRam = RamCooldown; // Ready to ram immediately if needed
	TimeSinceLastPIT = PITCooldown; // Ready to PIT immediately if needed
	SetPoliceState(InitialState);
}

void AMGPoliceUnit::SetPursuitTarget(APawn* Target)
{
	PursuitTarget = Target;
	if (Target)
	{
		LastKnownTargetPosition = Target->GetActorLocation();
		TimeSinceSawTarget = 0.0f;
		bHasVisual = true;
	}
}

void AMGPoliceUnit::SetPoliceState(EMGPoliceState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EMGPoliceState OldState = CurrentState;
	OnStateExit(OldState);

	CurrentState = NewState;
	OnStateEnter(NewState);

	// Broadcast state change event
	OnStateChanged.Broadcast(UnitID, NewState);
}

void AMGPoliceUnit::OnStateEnter(EMGPoliceState State)
{
	switch (State)
	{
		case EMGPoliceState::Pursuing:
		case EMGPoliceState::Ramming:
		case EMGPoliceState::BoxingIn:
		case EMGPoliceState::Intercepting:
			// Activate siren for all active pursuit states
			SetSirenEnabled(true);
			break;

		case EMGPoliceState::PITManeuver:
			// Activate siren and determine which side to approach from for PIT
			SetSirenEnabled(true);
			if (PursuitTarget.IsValid())
			{
				FVector ToTarget = PursuitTarget->GetActorLocation() - GetActorLocation();
				FVector RightVector = GetActorRightVector();
				PITSide = (FVector::DotProduct(ToTarget, RightVector) > 0.0f) ? 1.0f : -1.0f;
			}
			bExecutingPIT = true;
			break;

		case EMGPoliceState::Disabled:
			// Disable siren and apply brakes
			SetSirenEnabled(false);
			if (UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent()))
			{
				Movement->SetThrottleInput(0.0f);
				Movement->SetBrakeInput(1.0f);
			}
			OnUnitDisabled.Broadcast(UnitID);
			break;

		case EMGPoliceState::Patrolling:
			// Turn off siren when returning to patrol
			SetSirenEnabled(false);
			break;

		default:
			break;
	}
}

void AMGPoliceUnit::OnStateExit(EMGPoliceState State)
{
	switch (State)
	{
		case EMGPoliceState::PITManeuver:
			bExecutingPIT = false;
			break;

		default:
			break;
	}
}

// ============================================================================
// PURSUIT BEHAVIOR
// ============================================================================

void AMGPoliceUnit::StartPursuit()
{
	if (CurrentState == EMGPoliceState::Disabled || CurrentState == EMGPoliceState::Despawning)
	{
		return;
	}

	SetPoliceState(EMGPoliceState::Pursuing);
}

void AMGPoliceUnit::StopPursuit()
{
	if (CurrentState == EMGPoliceState::Disabled || CurrentState == EMGPoliceState::Despawning)
	{
		return;
	}

	SetPoliceState(EMGPoliceState::Patrolling);
	SetSirenEnabled(false);
}

float AMGPoliceUnit::GetDistanceToTarget() const
{
	if (PursuitTarget.IsValid())
	{
		return FVector::Dist(GetActorLocation(), PursuitTarget->GetActorLocation());
	}
	return FVector::Dist(GetActorLocation(), LastKnownTargetPosition);
}

void AMGPoliceUnit::ExecuteRam()
{
	if (CurrentState == EMGPoliceState::Disabled || CurrentState == EMGPoliceState::Despawning)
	{
		return;
	}

	if (TimeSinceLastRam >= RamCooldown)
	{
		SetPoliceState(EMGPoliceState::Ramming);
	}
}

void AMGPoliceUnit::ExecutePIT()
{
	if (CurrentState == EMGPoliceState::Disabled || CurrentState == EMGPoliceState::Despawning)
	{
		return;
	}

	if (TimeSinceLastPIT >= PITCooldown && PursuitTarget.IsValid())
	{
		// Check if we're in valid position for PIT
		if (IsValidPITAngle())
		{
			SetPoliceState(EMGPoliceState::PITManeuver);
		}
	}
}

void AMGPoliceUnit::ExecuteBoxing(FVector BoxPosition)
{
	if (CurrentState == EMGPoliceState::Disabled || CurrentState == EMGPoliceState::Despawning)
	{
		return;
	}

	BoxingTargetPosition = BoxPosition;
	SetPoliceState(EMGPoliceState::BoxingIn);
}

// ============================================================================
// DAMAGE SYSTEM
// ============================================================================

void AMGPoliceUnit::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
	if (CurrentState == EMGPoliceState::Disabled)
	{
		return;
	}

	Health -= DamageAmount;

	if (Health <= 0.0f)
	{
		Health = 0.0f;
		SetPoliceState(EMGPoliceState::Disabled);
	}
}

// ============================================================================
// SIREN AND LIGHTS
// ============================================================================

void AMGPoliceUnit::SetSirenEnabled(bool bEnabled)
{
	bSirenActive = bEnabled;

	// Control siren audio
	if (SirenAudio)
	{
		if (bEnabled)
		{
			if (!SirenAudio->IsPlaying())
			{
				SirenAudio->Play();
			}
		}
		else
		{
			SirenAudio->Stop();
		}
	}

	// Control light bar VFX
	if (LightBarVFX)
	{
		if (bEnabled)
		{
			LightBarVFX->Activate();
		}
		else
		{
			LightBarVFX->Deactivate();
		}
	}
}

// ============================================================================
// AI BEHAVIOR UPDATES
// ============================================================================

void AMGPoliceUnit::UpdatePatrol(float DeltaTime)
{
	// Basic patrol behavior - drive forward at moderate speed
	if (UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		Movement->SetThrottleInput(0.3f);
		Movement->SetSteeringInput(0.0f);
		Movement->SetBrakeInput(0.0f);
	}
}

void AMGPoliceUnit::UpdatePursuit(float DeltaTime)
{
	if (!PursuitTarget.IsValid())
	{
		// No valid target - head to last known position
		float DistToLastKnown = FVector::Dist(GetActorLocation(), LastKnownTargetPosition);
		if (DistToLastKnown < 300.0f)
		{
			// Reached last known position, lost target
			StopPursuit();
		}
		return;
	}

	UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!Movement)
	{
		return;
	}

	// Calculate steering and throttle to pursue target
	float Steering = CalculateSteeringToTarget();
	float Throttle = CalculateThrottleToTarget();

	// Apply aggression modifier
	Throttle *= (PursuitSpeedMultiplier * AggressionLevel);

	Movement->SetSteeringInput(Steering);
	Movement->SetThrottleInput(FMath::Clamp(Throttle, 0.0f, 1.0f));
	Movement->SetBrakeInput(0.0f);

	// Evaluate tactical options based on distance and situation
	float Distance = GetDistanceToTarget();

	// Check for PIT opportunity (requires proper positioning)
	if (Distance < PITDistance && Distance > PITDistance * 0.5f &&
		TimeSinceLastPIT >= PITCooldown && bHasVisual && IsValidPITAngle())
	{
		// Good position for PIT maneuver
		SetPoliceState(EMGPoliceState::PITManeuver);
		return;
	}

	// Check for ram opportunity (very close, has visual)
	if (Distance < RamDistance && TimeSinceLastRam >= RamCooldown && bHasVisual)
	{
		SetPoliceState(EMGPoliceState::Ramming);
		return;
	}

	// Track visual status
	if (!bHasVisual)
	{
		TimeSinceSawTarget += DeltaTime;
		if (TimeSinceSawTarget > VisualLostTime)
		{
			// Lost target for too long - return to patrol
			StopPursuit();
		}
	}
	else
	{
		TimeSinceSawTarget = 0.0f;
	}
}

void AMGPoliceUnit::UpdateRamming(float DeltaTime)
{
	if (!PursuitTarget.IsValid())
	{
		SetPoliceState(EMGPoliceState::Pursuing);
		return;
	}

	UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!Movement)
	{
		return;
	}

	// Aggressive steering directly at target
	float Steering = CalculateSteeringToTarget();

	// Full throttle for ramming attack
	Movement->SetSteeringInput(Steering);
	Movement->SetThrottleInput(1.0f);
	Movement->SetBrakeInput(0.0f);

	// Check if ram attempt is complete (we've passed the target or time limit)
	float Distance = GetDistanceToTarget();

	// Ram complete if we've moved past the target or gotten too far
	if (Distance > RamDistance * 2.5f)
	{
		TimeSinceLastRam = 0.0f;
		SetPoliceState(EMGPoliceState::Pursuing);
	}
}

void AMGPoliceUnit::UpdateBoxing(float DeltaTime)
{
	if (!PursuitTarget.IsValid())
	{
		SetPoliceState(EMGPoliceState::Pursuing);
		return;
	}

	UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!Movement)
	{
		return;
	}

	// Calculate target's velocity for prediction
	FVector TargetLocation = PursuitTarget->GetActorLocation();
	FVector TargetVelocity = PursuitTarget->GetVelocity();
	FVector MyLocation = GetActorLocation();

	// Calculate the desired box position relative to target
	// Box position should move with target
	FVector TargetForward = TargetVelocity.GetSafeNormal();
	if (TargetForward.IsNearlyZero())
	{
		TargetForward = PursuitTarget->GetActorForwardVector();
	}

	// Update box position relative to target's current position
	FVector BoxOffset = BoxingTargetPosition - LastKnownTargetPosition;
	FVector CurrentBoxTarget = TargetLocation + BoxOffset;

	// Steer toward box position
	FVector ToBoxPosition = CurrentBoxTarget - MyLocation;
	ToBoxPosition.Z = 0.0f;
	ToBoxPosition.Normalize();

	float DesiredYaw = FMath::Atan2(ToBoxPosition.Y, ToBoxPosition.X);
	float CurrentYaw = FMath::DegreesToRadians(GetActorRotation().Yaw);
	float YawDiff = FMath::FindDeltaAngleRadians(CurrentYaw, DesiredYaw);

	float Steering = FMath::Clamp(YawDiff * 2.5f, -1.0f, 1.0f);

	// Match target speed with slight adjustment based on position
	float TargetSpeed = TargetVelocity.Size();
	float MySpeed = GetVelocity().Size();
	float DistToBoxPos = FVector::Dist(MyLocation, CurrentBoxTarget);

	float Throttle;
	if (DistToBoxPos > 500.0f)
	{
		// Far from position - accelerate to catch up
		Throttle = 1.0f;
	}
	else if (DistToBoxPos > 200.0f)
	{
		// Medium distance - moderate speed
		Throttle = 0.7f;
	}
	else
	{
		// Close to position - match target speed
		Throttle = (TargetSpeed > MySpeed) ? 0.6f : 0.3f;
	}

	Movement->SetSteeringInput(Steering);
	Movement->SetThrottleInput(Throttle);
	Movement->SetBrakeInput(0.0f);

	// If target is nearly stopped, apply gentle pressure
	if (TargetSpeed < 100.0f && DistToBoxPos < 300.0f)
	{
		// Target slowing down - gentle squeeze
		Movement->SetThrottleInput(0.4f);
	}
}

void AMGPoliceUnit::UpdatePIT(float DeltaTime)
{
	if (!PursuitTarget.IsValid())
	{
		SetPoliceState(EMGPoliceState::Pursuing);
		return;
	}

	UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!Movement)
	{
		return;
	}

	float Distance = GetDistanceToTarget();

	// Check if we've lost the opportunity
	if (!bHasVisual || Distance > PITDistance * 1.5f)
	{
		TimeSinceLastPIT = 0.0f;
		OnPITAttempt.Broadcast(UnitID, false);
		SetPoliceState(EMGPoliceState::Pursuing);
		return;
	}

	// Calculate PIT approach steering
	float Steering = CalculatePITSteering();

	// Throttle based on distance - we need to match target speed and get alongside
	FVector TargetVelocity = PursuitTarget->GetVelocity();
	float TargetSpeed = TargetVelocity.Size();
	float MySpeed = GetVelocity().Size();

	float Throttle;
	if (Distance > PITDistance * 0.6f)
	{
		// Catching up - go faster
		Throttle = FMath::Clamp(1.2f - (MySpeed / (TargetSpeed + 1.0f)), 0.5f, 1.0f);
	}
	else if (Distance > PITDistance * 0.3f)
	{
		// Getting alongside - match speed
		Throttle = (MySpeed < TargetSpeed) ? 0.9f : 0.6f;
	}
	else
	{
		// In position - execute the turn
		Throttle = 0.7f;

		// Sharp turn into target's rear quarter
		Steering = PITSide * 0.8f;
	}

	Movement->SetSteeringInput(Steering);
	Movement->SetThrottleInput(Throttle);
	Movement->SetBrakeInput(0.0f);

	// Check if we're very close - this means PIT contact
	if (Distance < 200.0f)
	{
		// PIT attempt made - collision handling will determine success
		TimeSinceLastPIT = 0.0f;
		SetPoliceState(EMGPoliceState::Pursuing);
	}
}

void AMGPoliceUnit::UpdateIntercept(float DeltaTime)
{
	if (!PursuitTarget.IsValid())
	{
		SetPoliceState(EMGPoliceState::Pursuing);
		return;
	}

	UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!Movement)
	{
		return;
	}

	// Predict where target will be based on velocity
	FVector TargetLocation = PursuitTarget->GetActorLocation();
	FVector TargetVelocity = PursuitTarget->GetVelocity();

	// Predict 2-3 seconds ahead
	float PredictionTime = 2.5f;
	FVector PredictedPosition = TargetLocation + (TargetVelocity * PredictionTime);

	// Calculate interception point
	FVector MyLocation = GetActorLocation();
	FVector ToIntercept = PredictedPosition - MyLocation;
	ToIntercept.Z = 0.0f;

	float DistanceToIntercept = ToIntercept.Size();
	ToIntercept.Normalize();

	// Calculate steering toward intercept point
	float DesiredYaw = FMath::Atan2(ToIntercept.Y, ToIntercept.X);
	float CurrentYaw = FMath::DegreesToRadians(GetActorRotation().Yaw);
	float YawDiff = FMath::FindDeltaAngleRadians(CurrentYaw, DesiredYaw);

	float Steering = FMath::Clamp(YawDiff * 2.0f, -1.0f, 1.0f);

	// Throttle based on intercept distance
	float Throttle = (DistanceToIntercept > 2000.0f) ? 1.0f : 0.8f;

	Movement->SetSteeringInput(Steering);
	Movement->SetThrottleInput(Throttle * PursuitSpeedMultiplier);
	Movement->SetBrakeInput(0.0f);

	// Check if we've reached intercept zone - switch to pursuit
	float DistanceToTarget = GetDistanceToTarget();
	if (DistanceToTarget < PITDistance)
	{
		SetPoliceState(EMGPoliceState::Pursuing);
	}
}

void AMGPoliceUnit::UpdateDisabled(float DeltaTime)
{
	// Vehicle is disabled - apply brakes and coast to stop
	if (UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		Movement->SetThrottleInput(0.0f);
		Movement->SetBrakeInput(1.0f);
		Movement->SetSteeringInput(0.0f);
	}
}

// ============================================================================
// STEERING AND NAVIGATION CALCULATIONS
// ============================================================================

float AMGPoliceUnit::CalculateSteeringToTarget() const
{
	FVector TargetLocation;
	if (PursuitTarget.IsValid())
	{
		// Predict target position slightly ahead for smoother pursuit
		FVector TargetVelocity = PursuitTarget->GetVelocity();
		TargetLocation = PursuitTarget->GetActorLocation() + (TargetVelocity * 0.5f);
	}
	else
	{
		TargetLocation = LastKnownTargetPosition;
	}

	FVector ToTarget = TargetLocation - GetActorLocation();
	ToTarget.Z = 0.0f;
	ToTarget.Normalize();

	float DesiredYaw = FMath::Atan2(ToTarget.Y, ToTarget.X);
	float CurrentYaw = FMath::DegreesToRadians(GetActorRotation().Yaw);
	float YawDiff = FMath::FindDeltaAngleRadians(CurrentYaw, DesiredYaw);

	// Scale steering based on speed for smoother high-speed handling
	float MySpeed = GetVelocity().Size();
	float SteeringMultiplier = FMath::Lerp(2.5f, 1.5f, FMath::Clamp(MySpeed / 3000.0f, 0.0f, 1.0f));

	return FMath::Clamp(YawDiff * SteeringMultiplier, -1.0f, 1.0f);
}

float AMGPoliceUnit::CalculatePITSteering() const
{
	if (!PursuitTarget.IsValid())
	{
		return 0.0f;
	}

	// For PIT, we want to aim at the rear quarter panel of the target
	FVector TargetLocation = PursuitTarget->GetActorLocation();
	FVector TargetForward = PursuitTarget->GetActorForwardVector();
	FVector TargetRight = PursuitTarget->GetActorRightVector();

	// Target point is rear quarter panel on the chosen side
	FVector PITTargetPoint = TargetLocation - (TargetForward * 200.0f) + (TargetRight * PITSide * 100.0f);

	FVector ToPITTarget = PITTargetPoint - GetActorLocation();
	ToPITTarget.Z = 0.0f;
	ToPITTarget.Normalize();

	float DesiredYaw = FMath::Atan2(ToPITTarget.Y, ToPITTarget.X);
	float CurrentYaw = FMath::DegreesToRadians(GetActorRotation().Yaw);
	float YawDiff = FMath::FindDeltaAngleRadians(CurrentYaw, DesiredYaw);

	return FMath::Clamp(YawDiff * 2.0f, -1.0f, 1.0f);
}

float AMGPoliceUnit::CalculateThrottleToTarget() const
{
	float Distance = GetDistanceToTarget();
	float MySpeed = GetVelocity().Size();

	// Factor in target speed for pursuit
	float TargetSpeed = 0.0f;
	if (PursuitTarget.IsValid())
	{
		TargetSpeed = PursuitTarget->GetVelocity().Size();
	}

	// Dynamic throttle based on distance and relative speed
	if (Distance < 300.0f)
	{
		// Very close - match speed or slow slightly
		return (MySpeed > TargetSpeed * 1.1f) ? 0.4f : 0.6f;
	}
	else if (Distance < 800.0f)
	{
		// Medium distance - moderate pursuit
		return 0.8f;
	}
	else if (Distance < 1500.0f)
	{
		// Far - accelerate to catch up
		return 0.95f;
	}
	else
	{
		// Very far - full throttle
		return 1.0f;
	}
}

void AMGPoliceUnit::UpdateVisualOnTarget()
{
	if (!PursuitTarget.IsValid())
	{
		bHasVisual = false;
		return;
	}

	// Line trace to check visual contact
	FVector Start = GetActorLocation() + FVector(0, 0, 100);
	FVector End = PursuitTarget->GetActorLocation() + FVector(0, 0, 50);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(PursuitTarget.Get());

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

	// If nothing blocking, we have visual
	bHasVisual = !bHit;

	if (bHasVisual)
	{
		LastKnownTargetPosition = PursuitTarget->GetActorLocation();
	}
}

bool AMGPoliceUnit::IsValidPITAngle() const
{
	if (!PursuitTarget.IsValid())
	{
		return false;
	}

	// For a valid PIT, we need to be approaching from behind and to the side
	FVector ToTarget = PursuitTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	ToTarget.Normalize();

	FVector TargetForward = PursuitTarget->GetActorForwardVector();
	TargetForward.Z = 0.0f;
	TargetForward.Normalize();

	FVector MyForward = GetActorForwardVector();
	MyForward.Z = 0.0f;
	MyForward.Normalize();

	// Check if we're approaching from behind (within angle threshold)
	float ApproachDot = FVector::DotProduct(MyForward, TargetForward);
	float ApproachAngle = FMath::Acos(FMath::Clamp(ApproachDot, -1.0f, 1.0f));
	float ApproachAngleDegrees = FMath::RadiansToDegrees(ApproachAngle);

	// We want to be mostly parallel, traveling in same direction
	if (ApproachAngleDegrees > PITAngleThreshold)
	{
		return false;
	}

	// Check if we're positioned to the side (not directly behind)
	float SideDot = FVector::DotProduct(ToTarget, PursuitTarget->GetActorRightVector());
	float SideAngle = FMath::Abs(SideDot);

	// Need some lateral offset
	return SideAngle > 0.2f;
}

// ============================================================================
// COLLISION HANDLING
// ============================================================================

void AMGPoliceUnit::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor)
	{
		return;
	}

	// Calculate impact force
	float ImpactForce = NormalImpulse.Size();

	// Check if we hit the pursuit target
	if (OtherActor == PursuitTarget.Get())
	{
		// Handle based on current state
		if (CurrentState == EMGPoliceState::Ramming)
		{
			// Successful ram!
			OnRammedTarget.Broadcast(UnitID, ImpactForce);
			TimeSinceLastRam = 0.0f;
			SetPoliceState(EMGPoliceState::Pursuing);
		}
		else if (CurrentState == EMGPoliceState::PITManeuver)
		{
			// Check if PIT was successful based on impact angle
			FVector ImpactDirection = NormalImpulse.GetSafeNormal();
			FVector TargetRight = PursuitTarget->GetActorRightVector();

			float PITDot = FMath::Abs(FVector::DotProduct(ImpactDirection, TargetRight));

			// PIT is successful if impact is mostly lateral to target
			bool bPITSuccess = PITDot > 0.5f && ImpactForce > 50000.0f;

			OnPITAttempt.Broadcast(UnitID, bPITSuccess);
			TimeSinceLastPIT = 0.0f;

			if (bPITSuccess)
			{
				// Also count as a ram for damage purposes
				OnRammedTarget.Broadcast(UnitID, ImpactForce);
			}

			SetPoliceState(EMGPoliceState::Pursuing);
		}
	}
	else
	{
		// Hit something else - take damage based on impact
		float DamageFromCollision = ImpactForce / 10000.0f;
		if (DamageFromCollision > 5.0f)
		{
			ApplyDamage(DamageFromCollision, OtherActor);
		}
	}
}

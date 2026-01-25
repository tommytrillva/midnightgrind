// Copyright Midnight Grind. All Rights Reserved.

#include "Police/MGPoliceUnit.h"
#include "Vehicle/MGVehicleMovementComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

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
	// Create siren audio
	SirenAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("SirenAudio"));
	SirenAudio->SetupAttachment(GetMesh());
	SirenAudio->bAutoActivate = false;

	// Create engine audio
	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->SetupAttachment(GetMesh());
	EngineAudio->bAutoActivate = true;

	// Create light bar VFX
	LightBarVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightBarVFX"));
	LightBarVFX->SetupAttachment(GetMesh());
	LightBarVFX->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	LightBarVFX->bAutoActivate = false;
}

void AMGPoliceUnit::BeginPlay()
{
	Super::BeginPlay();
}

void AMGPoliceUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update visual tracking
	UpdateVisualOnTarget();

	// Update based on current state
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

		case EMGPoliceState::Disabled:
			UpdateDisabled(DeltaTime);
			break;

		default:
			break;
	}

	// Update ram cooldown
	TimeSinceLastRam += DeltaTime;
}

void AMGPoliceUnit::InitializeUnit(int32 InUnitID, EMGPoliceState InitialState)
{
	UnitID = InUnitID;
	Health = MaxHealth;
	SetPoliceState(InitialState);
}

void AMGPoliceUnit::SetPursuitTarget(APawn* Target)
{
	PursuitTarget = Target;
	if (Target)
	{
		LastKnownTargetPosition = Target->GetActorLocation();
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

	OnStateChanged.Broadcast(UnitID, NewState);
}

void AMGPoliceUnit::OnStateEnter(EMGPoliceState State)
{
	switch (State)
	{
		case EMGPoliceState::Pursuing:
		case EMGPoliceState::Ramming:
		case EMGPoliceState::BoxingIn:
			SetSirenEnabled(true);
			break;

		case EMGPoliceState::Disabled:
			SetSirenEnabled(false);
			// Apply vehicle disable effects
			if (UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent()))
			{
				Movement->SetThrottleInput(0.0f);
				Movement->SetBrakeInput(1.0f);
			}
			OnUnitDisabled.Broadcast(UnitID);
			break;

		default:
			break;
	}
}

void AMGPoliceUnit::OnStateExit(EMGPoliceState State)
{
	// Cleanup for previous state
}

void AMGPoliceUnit::StartPursuit()
{
	if (CurrentState == EMGPoliceState::Disabled)
	{
		return;
	}

	SetPoliceState(EMGPoliceState::Pursuing);
}

void AMGPoliceUnit::StopPursuit()
{
	if (CurrentState == EMGPoliceState::Disabled)
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

void AMGPoliceUnit::SetSirenEnabled(bool bEnabled)
{
	bSirenActive = bEnabled;

	if (SirenAudio)
	{
		if (bEnabled)
		{
			SirenAudio->Play();
		}
		else
		{
			SirenAudio->Stop();
		}
	}

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

void AMGPoliceUnit::UpdatePatrol(float DeltaTime)
{
	// Basic patrol behavior - drive forward slowly
	if (UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		Movement->SetThrottleInput(0.3f);
		Movement->SetSteeringInput(0.0f);
	}
}

void AMGPoliceUnit::UpdatePursuit(float DeltaTime)
{
	if (!PursuitTarget.IsValid())
	{
		return;
	}

	UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!Movement)
	{
		return;
	}

	// Calculate steering and throttle
	float Steering = CalculateSteeringToTarget();
	float Throttle = CalculateThrottleToTarget();

	Movement->SetSteeringInput(Steering);
	Movement->SetThrottleInput(Throttle * PursuitSpeedMultiplier);

	// Check if close enough to ram
	float Distance = GetDistanceToTarget();
	if (Distance < RamDistance && TimeSinceLastRam >= RamCooldown && bHasVisual)
	{
		SetPoliceState(EMGPoliceState::Ramming);
	}

	// Update time since saw target
	if (!bHasVisual)
	{
		TimeSinceSawTarget += DeltaTime;
		if (TimeSinceSawTarget > VisualLostTime)
		{
			// Lost target - return to patrol
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

	// Aggressive steering toward target
	float Steering = CalculateSteeringToTarget();

	// Full throttle for ramming
	Movement->SetSteeringInput(Steering);
	Movement->SetThrottleInput(1.0f);

	// Check if ram completed (distance increased significantly or time expired)
	float Distance = GetDistanceToTarget();
	if (Distance > RamDistance * 2.0f)
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

	// Try to position alongside target
	FVector TargetLocation = PursuitTarget->GetActorLocation();
	FVector MyLocation = GetActorLocation();
	FVector ToTarget = TargetLocation - MyLocation;
	ToTarget.Z = 0.0f;

	// Aim for a position beside the target
	FVector TargetVelocity = PursuitTarget->GetVelocity();
	TargetVelocity.Normalize();

	FVector BoxPosition = TargetLocation + FVector::CrossProduct(TargetVelocity, FVector::UpVector) * 200.0f;

	FVector ToBoxPosition = BoxPosition - MyLocation;
	ToBoxPosition.Normalize();

	float DesiredYaw = FMath::Atan2(ToBoxPosition.Y, ToBoxPosition.X);
	float CurrentYaw = FMath::DegreesToRadians(GetActorRotation().Yaw);
	float YawDiff = FMath::FindDeltaAngleRadians(CurrentYaw, DesiredYaw);

	float Steering = FMath::Clamp(YawDiff * 2.0f, -1.0f, 1.0f);

	// Match target speed
	float TargetSpeed = PursuitTarget->GetVelocity().Size();
	float MySpeed = GetVelocity().Size();
	float Throttle = (TargetSpeed > MySpeed) ? 0.8f : 0.3f;

	Movement->SetSteeringInput(Steering);
	Movement->SetThrottleInput(Throttle);
}

void AMGPoliceUnit::UpdateDisabled(float DeltaTime)
{
	// Vehicle is disabled - just apply brakes and wait
	if (UMGVehicleMovementComponent* Movement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent()))
	{
		Movement->SetThrottleInput(0.0f);
		Movement->SetBrakeInput(1.0f);
	}
}

float AMGPoliceUnit::CalculateSteeringToTarget() const
{
	FVector TargetLocation;
	if (PursuitTarget.IsValid())
	{
		TargetLocation = PursuitTarget->GetActorLocation();
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

	return FMath::Clamp(YawDiff * 2.0f, -1.0f, 1.0f);
}

float AMGPoliceUnit::CalculateThrottleToTarget() const
{
	float Distance = GetDistanceToTarget();

	// Close = less throttle, far = more throttle
	if (Distance < 300.0f)
	{
		return 0.5f;
	}
	else if (Distance < 800.0f)
	{
		return 0.8f;
	}
	else
	{
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

	// Line trace to target
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

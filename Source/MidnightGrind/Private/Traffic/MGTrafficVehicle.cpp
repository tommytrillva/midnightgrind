// Copyright Midnight Grind. All Rights Reserved.

#include "Traffic/MGTrafficVehicle.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AMGTrafficVehicle::AMGTrafficVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create collision box as root
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(250.0f, 100.0f, 75.0f)); // Approximate car size
	CollisionBox->SetCollisionProfileName(TEXT("Vehicle"));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->OnComponentHit.AddDynamic(this, &AMGTrafficVehicle::OnCollisionHit);
	RootComponent = CollisionBox;

	// Create vehicle mesh
	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	VehicleMesh->SetupAttachment(RootComponent);
	VehicleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMGTrafficVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Set headlights based on time of day (default on for night racing game)
	bHeadlightsOn = true;
}

void AMGTrafficVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind collision events
	if (CollisionBox)
	{
		CollisionBox->OnComponentHit.RemoveDynamic(this, &AMGTrafficVehicle::OnCollisionHit);
	}

	Super::EndPlay(EndPlayReason);
}

void AMGTrafficVehicle::Tick(float MGDeltaTime)
{
	Super::Tick(DeltaTime);

	// Early exit for parked vehicles
	if (CurrentBehavior == EMGTrafficBehavior::Parked)
	{
		return;
	}

	UpdateBehavior(DeltaTime);
	UpdateMovement(DeltaTime);
}

void AMGTrafficVehicle::InitializeVehicle(int32 InVehicleID, EMGTrafficVehicleType InType, EMGTrafficBehavior InBehavior)
{
	VehicleID = InVehicleID;
	VehicleType = InType;
	CurrentBehavior = InBehavior;

	// Set collision box size based on vehicle type
	FVector BoxExtent = GetVehicleDimensions();
	CollisionBox->SetBoxExtent(BoxExtent);

	// Set default target speed based on behavior
	switch (InBehavior)
	{
		case EMGTrafficBehavior::Aggressive:
			TargetSpeed = 45.0f;
			break;
		case EMGTrafficBehavior::Cautious:
			TargetSpeed = 25.0f;
			break;
		case EMGTrafficBehavior::Distracted:
			TargetSpeed = 28.0f;
			break;
		case EMGTrafficBehavior::Parked:
			TargetSpeed = 0.0f;
			break;
		default:
			TargetSpeed = 35.0f;
			break;
	}
}

void AMGTrafficVehicle::SetTargetSpeed(float SpeedMPH)
{
	TargetSpeed = FMath::Max(0.0f, SpeedMPH);
}

void AMGTrafficVehicle::SetTargetLocation(FVector Location)
{
	TargetLocation = Location;
	bHasTargetLocation = true;
}

void AMGTrafficVehicle::SetBehavior(EMGTrafficBehavior NewBehavior)
{
	CurrentBehavior = NewBehavior;

	// React to behavior change
	switch (NewBehavior)
	{
		case EMGTrafficBehavior::Panicked:
			TargetSpeed *= 1.5f;
			break;
		case EMGTrafficBehavior::Pulling:
			TargetSpeed = 10.0f;
			break;
		case EMGTrafficBehavior::Parked:
			TargetSpeed = 0.0f;
			break;
		default:
			break;
	}
}

void AMGTrafficVehicle::StopImmediately()
{
	CurrentSpeed = 0.0f;
	TargetSpeed = 0.0f;
}

void AMGTrafficVehicle::SetVehicleMesh(UStaticMesh* Mesh)
{
	if (VehicleMesh && Mesh)
	{
		VehicleMesh->SetStaticMesh(Mesh);
	}
}

void AMGTrafficVehicle::SetHeadlightsEnabled(bool bEnabled)
{
	bHeadlightsOn = bEnabled;
	// Visual update would be handled by Blueprint or material parameter
}

void AMGTrafficVehicle::SetBrakeLightsEnabled(bool bEnabled)
{
	bBrakeLightsOn = bEnabled;
	// Visual update would be handled by Blueprint or material parameter
}

void AMGTrafficVehicle::SetTurnSignal(bool bLeft, bool bRight)
{
	bLeftTurnSignal = bLeft;
	bRightTurnSignal = bRight;
}

void AMGTrafficVehicle::UpdateMovement(float MGDeltaTime)
{
	// Accelerate or decelerate towards target speed
	float SpeedDiff = TargetSpeed - CurrentSpeed;
	float SpeedChange = 0.0f;

	if (SpeedDiff > 0.1f)
	{
		// Accelerating
		SpeedChange = FMath::Min(AccelerationRate * DeltaTime, SpeedDiff);
		bBrakeLightsOn = false;
	}
	else if (SpeedDiff < -0.1f)
	{
		// Braking
		float BrakeAmount = (TargetSpeed < 5.0f) ? BrakeRate : DecelerationRate;
		SpeedChange = FMath::Max(-BrakeAmount * DeltaTime, SpeedDiff);
		bBrakeLightsOn = true;
	}
	else
	{
		bBrakeLightsOn = false;
	}

	CurrentSpeed = FMath::Max(0.0f, CurrentSpeed + SpeedChange);

	// Convert MPH to cm/s (1 mph = 44.704 cm/s)
	float SpeedCmPerSec = CurrentSpeed * 44.704f;

	// Move forward
	FVector Forward = GetActorForwardVector();
	FVector NewLocation = GetActorLocation() + Forward * SpeedCmPerSec * DeltaTime;
	SetActorLocation(NewLocation);

	// Steer towards target if set
	if (bHasTargetLocation)
	{
		FVector ToTarget = TargetLocation - GetActorLocation();
		float DistanceToTarget = ToTarget.Size2D();

		if (DistanceToTarget < 200.0f)
		{
			bHasTargetLocation = false;
		}
		else
		{
			FRotator TargetRotation = ToTarget.Rotation();
			FRotator CurrentRotation = GetActorRotation();
			FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, SteeringSpeed);
			SetActorRotation(NewRotation);
		}
	}
}

void AMGTrafficVehicle::UpdateBehavior(float MGDeltaTime)
{
	switch (CurrentBehavior)
	{
		case EMGTrafficBehavior::Panicked:
			// Erratic speed changes
			if (FMath::FRand() < 0.1f)
			{
				TargetSpeed = FMath::FRandRange(35.0f, 60.0f);
			}
			break;

		case EMGTrafficBehavior::Distracted:
			// Occasional sudden braking
			if (FMath::FRand() < 0.01f)
			{
				TargetSpeed *= 0.5f;
			}
			break;

		case EMGTrafficBehavior::Aggressive:
			// Try to maintain higher speed
			if (CurrentSpeed < TargetSpeed * 0.9f && FMath::FRand() < 0.05f)
			{
				TargetSpeed += 5.0f;
			}
			break;

		default:
			break;
	}
}

FVector AMGTrafficVehicle::GetVehicleDimensions() const
{
	// Return box extent (half-size) based on vehicle type
	switch (VehicleType)
	{
		case EMGTrafficVehicleType::Sedan:
			return FVector(230.0f, 95.0f, 70.0f);
		case EMGTrafficVehicleType::SUV:
			return FVector(250.0f, 100.0f, 90.0f);
		case EMGTrafficVehicleType::Truck:
			return FVector(300.0f, 100.0f, 100.0f);
		case EMGTrafficVehicleType::Van:
			return FVector(280.0f, 100.0f, 110.0f);
		case EMGTrafficVehicleType::SportsCar:
			return FVector(220.0f, 95.0f, 60.0f);
		case EMGTrafficVehicleType::Motorcycle:
			return FVector(120.0f, 45.0f, 70.0f);
		case EMGTrafficVehicleType::Bus:
			return FVector(600.0f, 130.0f, 150.0f);
		case EMGTrafficVehicleType::Semi:
			return FVector(800.0f, 130.0f, 180.0f);
		case EMGTrafficVehicleType::Taxi:
			return FVector(240.0f, 95.0f, 75.0f);
		case EMGTrafficVehicleType::DeliveryVan:
			return FVector(300.0f, 100.0f, 120.0f);
		default:
			return FVector(230.0f, 95.0f, 70.0f);
	}
}

void AMGTrafficVehicle::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bHasCollided)
	{
		bHasCollided = true;

		// React to collision
		SetBehavior(EMGTrafficBehavior::Panicked);

		// Notify subsystem
		if (UWorld* World = GetWorld())
		{
			if (UMGTrafficSubsystem* TrafficSubsystem = World->GetSubsystem<UMGTrafficSubsystem>())
			{
				TrafficSubsystem->NotifyPlayerCollision(VehicleID);
			}
		}
	}
}

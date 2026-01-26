// Copyright Midnight Grind. All Rights Reserved.

#include "Police/MGPoliceRoadblock.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"

AMGPoliceRoadblock::AMGPoliceRoadblock()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create collision box as root
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(200.0f, Width / 2.0f, 100.0f));
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->OnComponentHit.AddDynamic(this, &AMGPoliceRoadblock::OnCollisionHit);
	RootComponent = CollisionBox;

	// Create main barricade mesh placeholder
	BarricadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarricadeMesh"));
	BarricadeMesh->SetupAttachment(RootComponent);
	BarricadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentHealth = MaxHealth;
}

void AMGPoliceRoadblock::BeginPlay()
{
	Super::BeginPlay();
}

void AMGPoliceRoadblock::Initialize(int32 InBlockadeID, FVector Direction)
{
	BlockadeID = InBlockadeID;

	// Orient the roadblock perpendicular to the given direction
	if (!Direction.IsNearlyZero())
	{
		FRotator Rotation = Direction.Rotation();
		// Rotate 90 degrees to be perpendicular
		Rotation.Yaw += 90.0f;
		SetActorRotation(Rotation);
	}
}

void AMGPoliceRoadblock::SetNumVehicles(int32 Count)
{
	NumVehicles = FMath::Clamp(Count, 1, MaxVehicles);

	// Adjust collision width based on number of vehicles
	float AdjustedWidth = (NumVehicles * 400.0f); // Each vehicle ~400cm wide
	CollisionBox->SetBoxExtent(FVector(200.0f, AdjustedWidth / 2.0f, 100.0f));
}

void AMGPoliceRoadblock::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
	if (bIsBreached)
	{
		return;
	}

	CurrentHealth -= DamageAmount;

	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		BreachRoadblock();
	}
}

void AMGPoliceRoadblock::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bIsBreached || !OtherActor)
	{
		return;
	}

	// Calculate impact speed from impulse
	float ImpulseMagnitude = NormalImpulse.Size();
	float EstimatedSpeedMPH = ImpulseMagnitude / 10000.0f; // Rough conversion

	OnRoadblockHit.Broadcast(OtherActor, EstimatedSpeedMPH);

	// Check if this is a player vehicle
	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (HitPawn->IsPlayerControlled())
		{
			// High speed impact can breach the roadblock
			if (EstimatedSpeedMPH >= BreachSpeedThreshold)
			{
				// Damage based on speed
				float Damage = (EstimatedSpeedMPH - BreachSpeedThreshold) * 2.0f;
				ApplyDamage(Damage, OtherActor);
			}
			else
			{
				// Slowing hit, apply less damage
				ApplyDamage(EstimatedSpeedMPH * 0.5f, OtherActor);
			}
		}
	}
}

void AMGPoliceRoadblock::BreachRoadblock()
{
	bIsBreached = true;

	// Disable collision
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OnRoadblockDestroyed.Broadcast();

	// Start despawn timer
	SetLifeSpan(5.0f);
}

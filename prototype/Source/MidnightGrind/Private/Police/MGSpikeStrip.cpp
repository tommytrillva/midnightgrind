// Copyright Midnight Grind. All Rights Reserved.

#include "Police/MGSpikeStrip.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Vehicle/MGVehiclePawn.h"

AMGSpikeStrip::AMGSpikeStrip()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create trigger box as root
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(50.0f, DefaultLength / 2.0f, 10.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMGSpikeStrip::OnTriggerBeginOverlap);
	RootComponent = TriggerBox;

	// Create spike mesh placeholder
	SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMesh"));
	SpikeMesh->SetupAttachment(RootComponent);
	SpikeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpikeMesh->SetVisibility(false); // Hidden until deployed

	CurrentLength = DefaultLength;
}

void AMGSpikeStrip::BeginPlay()
{
	Super::BeginPlay();
}

void AMGSpikeStrip::Initialize(int32 InStripID, FVector Direction)
{
	StripID = InStripID;

	// Orient perpendicular to the given direction
	if (!Direction.IsNearlyZero())
	{
		FRotator Rotation = Direction.Rotation();
		Rotation.Yaw += 90.0f;
		SetActorRotation(Rotation);
	}
}

void AMGSpikeStrip::SetLength(float Length)
{
	CurrentLength = FMath::Clamp(Length, 200.0f, 1500.0f);
	TriggerBox->SetBoxExtent(FVector(50.0f, CurrentLength / 2.0f, 10.0f));
}

void AMGSpikeStrip::Deploy()
{
	if (bIsDeployed)
	{
		return;
	}

	bIsDeployed = true;

	// Enable collision and show mesh
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SpikeMesh->SetVisibility(true);
}

void AMGSpikeStrip::Retract()
{
	bIsDeployed = false;

	// Disable collision and hide mesh
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpikeMesh->SetVisibility(false);
}

void AMGSpikeStrip::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsDeployed || bHasBeenTriggered || !OtherActor)
	{
		return;
	}

	// Check if this is a vehicle pawn
	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		ApplyTireDamage(OtherActor);
		OnVehicleHitSpikes.Broadcast(OtherActor);

		if (bSingleUse)
		{
			bHasBeenTriggered = true;
			// Disable and prepare for despawn
			TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SetLifeSpan(3.0f);
		}
	}
}

void AMGSpikeStrip::ApplyTireDamage(AActor* Vehicle)
{
	if (!Vehicle)
	{
		return;
	}

	// Try to apply damage to vehicle tires
	if (AMGVehiclePawn* VehiclePawn = Cast<AMGVehiclePawn>(Vehicle))
	{
		// Apply tire damage to all wheels that crossed the strip
		// For simplicity, damage all tires proportionally
		VehiclePawn->ApplyTireDamage(TireDamageAmount);
	}
}

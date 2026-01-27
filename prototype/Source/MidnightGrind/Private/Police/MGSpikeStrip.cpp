// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGSpikeStrip.cpp
 * @brief Implementation of police spike strip trap actor.
 *
 * Spike strips provide a non-lethal tactical option for police
 * to slow down fleeing vehicles by damaging their tires.
 */

#include "Police/MGSpikeStrip.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Pawn.h"
#include "Vehicle/MGVehiclePawn.h"

// ============================================================================
// CONSTRUCTION
// ============================================================================

AMGSpikeStrip::AMGSpikeStrip()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetupComponents();

	CurrentLength = DefaultLength;
}

void AMGSpikeStrip::SetupComponents()
{
	// Create trigger box as root
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(50.0f, DefaultLength / 2.0f, 10.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disabled until deployed
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMGSpikeStrip::OnTriggerBeginOverlap);
	RootComponent = TriggerBox;

	// Create spike mesh placeholder
	SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMesh"));
	SpikeMesh->SetupAttachment(RootComponent);
	SpikeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpikeMesh->SetVisibility(false); // Hidden until deployed

	// Create hit audio
	HitAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("HitAudio"));
	HitAudio->SetupAttachment(RootComponent);
	HitAudio->bAutoActivate = false;
}

// ============================================================================
// ACTOR LIFECYCLE
// ============================================================================

void AMGSpikeStrip::BeginPlay()
{
	Super::BeginPlay();
}

void AMGSpikeStrip::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind overlap events
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &AMGSpikeStrip::OnTriggerBeginOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

void AMGSpikeStrip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDeploying)
	{
		UpdateDeployAnimation(DeltaTime);
	}
}

// ============================================================================
// CONFIGURATION
// ============================================================================

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
	CurrentLength = FMath::Clamp(Length, MinLength, MaxLength);
	TriggerBox->SetBoxExtent(FVector(50.0f, CurrentLength / 2.0f, 10.0f));

	// Scale spike mesh to match length
	if (SpikeMesh)
	{
		float ScaleFactor = CurrentLength / DefaultLength;
		SpikeMesh->SetRelativeScale3D(FVector(1.0f, ScaleFactor, 1.0f));
	}
}

void AMGSpikeStrip::Deploy()
{
	if (bIsDeployed || bIsDeploying || bHasBeenTriggered)
	{
		return;
	}

	bIsDeploying = true;
	DeployProgress = 0.0f;

	// Enable tick for deployment animation
	SetActorTickEnabled(true);

	// Start showing mesh immediately (will animate in)
	if (SpikeMesh)
	{
		SpikeMesh->SetVisibility(true);
		SpikeMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.01f));
	}
}

void AMGSpikeStrip::Retract()
{
	if (!bIsDeployed || bHasBeenTriggered)
	{
		return;
	}

	bIsDeployed = false;

	// Disable collision and hide mesh
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (SpikeMesh)
	{
		SpikeMesh->SetVisibility(false);
	}
}

void AMGSpikeStrip::UpdateDeployAnimation(float DeltaTime)
{
	DeployProgress += DeltaTime / DeployTime;

	if (DeployProgress >= 1.0f)
	{
		// Deployment complete
		bIsDeploying = false;
		bIsDeployed = true;

		// Enable collision
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Set final mesh scale
		if (SpikeMesh)
		{
			float LengthScale = CurrentLength / DefaultLength;
			SpikeMesh->SetRelativeScale3D(FVector(1.0f, LengthScale, 1.0f));
		}

		// Disable tick (no longer needed for animation)
		SetActorTickEnabled(false);

		// Broadcast deployment complete
		OnSpikeStripDeployed.Broadcast(StripID);
	}
	else
	{
		// Animate spikes extending
		float Alpha = FMath::InterpEaseOut(0.0f, 1.0f, DeployProgress, 3.0f);
		float LengthScale = CurrentLength / DefaultLength;

		if (SpikeMesh)
		{
			SpikeMesh->SetRelativeScale3D(FVector(Alpha, LengthScale * Alpha, Alpha));
		}
	}
}

// ============================================================================
// OVERLAP HANDLING
// ============================================================================

void AMGSpikeStrip::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsDeployed || bHasBeenTriggered || !OtherActor)
	{
		return;
	}

	// Check if this is a vehicle pawn
	APawn* HitPawn = Cast<APawn>(OtherActor);
	if (!HitPawn)
	{
		return;
	}

	// Get vehicle speed for damage calculation
	float VehicleSpeed = OtherActor->GetVelocity().Size();
	float VehicleSpeedMPH = VehicleSpeed * 0.0223694f; // Convert cm/s to MPH

	// Apply tire damage
	ApplyTireDamage(OtherActor, VehicleSpeedMPH);

	// Increment hit count
	HitCount++;

	// Broadcast the hit event
	OnVehicleHitSpikes.Broadcast(OtherActor);

	// Play hit audio
	if (HitAudio)
	{
		HitAudio->Play();
	}

	// Handle single-use destruction
	if (bSingleUse)
	{
		bHasBeenTriggered = true;
		DestroyStrip();
	}
}

void AMGSpikeStrip::ApplyTireDamage(AActor* Vehicle, float VehicleSpeed)
{
	if (!Vehicle)
	{
		return;
	}

	// Try to apply damage to vehicle tires
	AMGVehiclePawn* VehiclePawn = Cast<AMGVehiclePawn>(Vehicle);
	if (!VehiclePawn)
	{
		return;
	}

	// Calculate damage based on speed if progressive
	float FinalDamage = TireDamageAmount;
	if (bProgressiveDamage)
	{
		// More damage at higher speeds (60 MPH = base, 120 MPH = 2x)
		float SpeedMultiplier = FMath::Clamp(VehicleSpeed / 60.0f, 0.5f, 2.0f);
		FinalDamage *= SpeedMultiplier;
	}

	// Apply tire damage - affects grip and handling
	VehiclePawn->ApplyTireDamage(FinalDamage);

	// Apply speed reduction effect (temporary slowdown)
	// This simulates the immediate impact of hitting the spikes
	float SpeedReductionFactor = 1.0f - (SpeedReductionPercent / 100.0f);

	// The vehicle movement component should handle speed reduction
	// via the tire damage system, but we can also apply a direct impulse
	FVector CurrentVelocity = VehiclePawn->GetVelocity();
	FVector ReducedVelocity = CurrentVelocity * SpeedReductionFactor;

	// Apply as a sudden velocity change (simulates drag from flattening tires)
	if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(VehiclePawn->GetRootComponent()))
	{
		if (RootComp->IsSimulatingPhysics())
		{
			FVector ImpulseToApply = (ReducedVelocity - CurrentVelocity);
			float VehicleMass = RootComp->GetMass();
			RootComp->AddImpulse(ImpulseToApply * VehicleMass * 0.1f, NAME_None, true);
		}
	}
}

void AMGSpikeStrip::DestroyStrip()
{
	// Disable collision immediately
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Broadcast destruction event
	OnSpikeStripDestroyed.Broadcast(StripID);

	// Start despawn timer (give time for audio to play)
	SetLifeSpan(3.0f);
}

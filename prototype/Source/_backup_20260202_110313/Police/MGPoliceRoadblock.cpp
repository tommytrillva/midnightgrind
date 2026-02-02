// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPoliceRoadblock.cpp
 * @brief Implementation of police roadblock obstacle actor.
 *
 * Roadblocks create tactical obstacles during pursuits that players
 * must breach or avoid. They scale with heat level and provide
 * feedback through damage, visual effects, and audio.
 */

#include "Police/MGPoliceRoadblock.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/Pawn.h"
#include "Vehicle/MGVehiclePawn.h"

// ============================================================================
// CONSTRUCTION
// ============================================================================

AMGPoliceRoadblock::AMGPoliceRoadblock()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetupComponents();

	CurrentHealth = MaxHealth;
}

void AMGPoliceRoadblock::SetupComponents()
{
	// Create collision box as root
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(200.0f, VehicleWidth, 100.0f));
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disabled until deployed
	CollisionBox->OnComponentHit.AddDynamic(this, &AMGPoliceRoadblock::OnCollisionHit);
	RootComponent = CollisionBox;

	// Create main barricade mesh placeholder
	BarricadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarricadeMesh"));
	BarricadeMesh->SetupAttachment(RootComponent);
	BarricadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BarricadeMesh->SetVisibility(false);

	// Create police radio audio
	RadioAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RadioAudio"));
	RadioAudio->SetupAttachment(RootComponent);
	RadioAudio->bAutoActivate = false;

	// Create light bar VFX
	LightBarVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightBarVFX"));
	LightBarVFX->SetupAttachment(RootComponent);
	LightBarVFX->bAutoActivate = false;
}

// ============================================================================
// ACTOR LIFECYCLE
// ============================================================================

void AMGPoliceRoadblock::BeginPlay()
{
	Super::BeginPlay();
}

void AMGPoliceRoadblock::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind collision events
	if (CollisionBox)
	{
		CollisionBox->OnComponentHit.RemoveDynamic(this, &AMGPoliceRoadblock::OnCollisionHit);
	}

	Super::EndPlay(EndPlayReason);
}

void AMGPoliceRoadblock::Tick(float DeltaTime)
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

void AMGPoliceRoadblock::Initialize(int32 InBlockadeID, FVector Direction)
{
	BlockadeID = InBlockadeID;

	// Orient the roadblock perpendicular to the given direction
	if (!Direction.IsNearlyZero())
	{
		FRotator Rotation = Direction.Rotation();
		// Rotate 90 degrees to be perpendicular to traffic direction
		Rotation.Yaw += 90.0f;
		SetActorRotation(Rotation);
	}
}

void AMGPoliceRoadblock::SetNumVehicles(int32 Count)
{
	NumVehicles = FMath::Clamp(Count, 1, MaxVehicles);

	// Adjust collision width based on number of vehicles
	float AdjustedWidth = NumVehicles * VehicleWidth;
	CollisionBox->SetBoxExtent(FVector(200.0f, AdjustedWidth / 2.0f, 100.0f));

	// Increase health based on vehicle count
	MaxHealth = 100.0f + (NumVehicles * 50.0f);
	CurrentHealth = MaxHealth;
}

void AMGPoliceRoadblock::Deploy()
{
	if (bIsDeployed || bIsDeploying)
	{
		return;
	}

	bIsDeploying = true;
	DeployProgress = 0.0f;

	// Enable tick for deployment animation
	SetActorTickEnabled(true);

	// Start audio/visual effects
	if (RadioAudio)
	{
		RadioAudio->Play();
	}

	if (LightBarVFX)
	{
		LightBarVFX->Activate();
	}

	// Show barricade mesh
	if (BarricadeMesh)
	{
		BarricadeMesh->SetVisibility(true);
	}
}

void AMGPoliceRoadblock::UpdateDeployAnimation(float DeltaTime)
{
	DeployProgress += DeltaTime / DeployTime;

	if (DeployProgress >= 1.0f)
	{
		// Deployment complete
		bIsDeploying = false;
		bIsDeployed = true;

		// Enable collision
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// Disable tick (no longer needed)
		SetActorTickEnabled(false);

		// Broadcast deployment complete
		OnRoadblockDeployed.Broadcast(BlockadeID);
	}
	else
	{
		// Animate barricade rising or vehicles moving into position
		// This would be expanded with proper animation in a full implementation
		float Alpha = FMath::InterpEaseOut(0.0f, 1.0f, DeployProgress, 2.0f);

		if (BarricadeMesh)
		{
			// Simple scale-up animation
			BarricadeMesh->SetRelativeScale3D(FVector(Alpha, Alpha, Alpha));
		}
	}
}

// ============================================================================
// DAMAGE
// ============================================================================

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

	// Calculate impact speed from impulse magnitude
	float ImpulseMagnitude = NormalImpulse.Size();

	// Convert impulse to approximate speed (MPH)
	// This is a rough estimation - actual conversion depends on vehicle mass
	float EstimatedSpeedMPH = ImpulseMagnitude / 10000.0f;

	// Broadcast the hit event
	OnRoadblockHit.Broadcast(OtherActor, EstimatedSpeedMPH);

	// Check if this is a vehicle pawn
	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		// Apply damage to the hitting vehicle
		if (AMGVehiclePawn* VehiclePawn = Cast<AMGVehiclePawn>(OtherActor))
		{
			// Scale damage based on speed
			float SpeedDamageMult = FMath::Clamp(EstimatedSpeedMPH / BreachSpeedThreshold, 0.5f, 2.0f);
			VehiclePawn->ApplyDamage(VehicleDamageOnImpact * SpeedDamageMult, this);
		}

		// Calculate damage to roadblock
		if (EstimatedSpeedMPH >= BreachSpeedThreshold)
		{
			// High speed impact - significant damage
			float Damage = (EstimatedSpeedMPH - BreachSpeedThreshold) * HighSpeedDamageMultiplier;
			Damage = FMath::Max(Damage, 50.0f); // Minimum damage on breach attempt
			ApplyDamage(Damage, OtherActor);
		}
		else
		{
			// Low speed hit - minor damage
			float Damage = EstimatedSpeedMPH * 0.5f;
			ApplyDamage(Damage, OtherActor);
		}
	}
}

void AMGPoliceRoadblock::BreachRoadblock()
{
	bIsBreached = true;

	// Disable collision
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Stop audio
	if (RadioAudio)
	{
		RadioAudio->Stop();
	}

	// Deactivate light effects
	if (LightBarVFX)
	{
		LightBarVFX->Deactivate();
	}

	// Broadcast destruction event
	OnRoadblockDestroyed.Broadcast();

	// Start despawn timer (give time for destruction effects)
	SetLifeSpan(5.0f);
}

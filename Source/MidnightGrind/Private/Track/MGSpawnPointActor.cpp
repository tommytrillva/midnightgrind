// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGSpawnPointActor.cpp
 * @brief Implementation of race starting grid position markers.
 *
 * Handles spawn point configuration, grid positioning, availability tracking,
 * and editor visualization for race start positions.
 */

#include "Track/MGSpawnPointActor.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"

AMGSpawnPointActor::AMGSpawnPointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootSceneComponent);

	// Create direction arrow
	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(RootSceneComponent);
	DirectionArrow->SetArrowColor(FLinearColor::Green);
	DirectionArrow->ArrowSize = 2.0f;
	DirectionArrow->ArrowLength = 200.0f;
	DirectionArrow->bIsScreenSizeScaled = false;

	// Create spawn area box
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(RootSceneComponent);
	SpawnArea->SetBoxExtent(FVector(250.0f, 100.0f, 50.0f));
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnArea->SetHiddenInGame(true);
	SpawnArea->ShapeColor = FColor::Green;

#if WITH_EDITORONLY_DATA
	// Create billboard
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(RootSceneComponent);
	Billboard->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	Billboard->bHiddenInGame = true;
#endif
}

void AMGSpawnPointActor::BeginPlay()
{
	Super::BeginPlay();

	// Reset state
	AssignedRacerID = -1;
	bIsOccupied = false;
}

#if WITH_EDITOR
void AMGSpawnPointActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateVisuals();
}
#endif

FTransform AMGSpawnPointActor::GetSpawnTransform() const
{
	return GetActorTransform();
}

void AMGSpawnPointActor::MarkAsUsed(int32 RacerID)
{
	AssignedRacerID = RacerID;
	bIsOccupied = true;
}

void AMGSpawnPointActor::ResetAvailability()
{
	AssignedRacerID = -1;
	bIsOccupied = false;
}

void AMGSpawnPointActor::UpdateVisuals()
{
#if WITH_EDITOR
	// Update arrow color based on position
	if (DirectionArrow)
	{
		if (bIsPolePosition)
		{
			DirectionArrow->SetArrowColor(FLinearColor(1.0f, 0.84f, 0.0f)); // Gold
		}
		else if (GridPosition <= 3)
		{
			DirectionArrow->SetArrowColor(FLinearColor::Green);
		}
		else
		{
			DirectionArrow->SetArrowColor(FLinearColor(0.5f, 0.5f, 0.5f)); // Gray
		}
	}

	// Update spawn area color
	if (SpawnArea)
	{
		SpawnArea->ShapeColor = bIsPolePosition ? FColor::Yellow : FColor::Green;
	}
#endif
}

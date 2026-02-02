// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCheckpoint.cpp
 * @brief Implementation of checkpoint and start/finish line actors.
 *
 * Handles checkpoint trigger detection, respawn positioning, grid position
 * calculation, and visual state updates for track checkpoints.
 */

#include "Track/MGCheckpoint.h"
#include "Vehicle/MGVehiclePawn.h"
#include "GameModes/MGRaceGameMode.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

// ==========================================
// AMGCheckpoint
// ==========================================

AMGCheckpoint::AMGCheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create trigger volume
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(Root);
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->SetGenerateOverlapEvents(true);
	TriggerVolume->SetBoxExtent(FVector(CheckpointDepth * 0.5f, CheckpointWidth * 0.5f, CheckpointHeight * 0.5f));

	// Create visual mesh
	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointMesh"));
	CheckpointMesh->SetupAttachment(Root);
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CheckpointMesh->SetVisibility(false); // Hidden by default at runtime

	// Create direction arrow (editor visualization)
	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(Root);
	DirectionArrow->SetArrowColor(FColor::Green);
	DirectionArrow->ArrowLength = 200.0f;
	DirectionArrow->SetHiddenInGame(true);
}

void AMGCheckpoint::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap event
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AMGCheckpoint::OnTriggerOverlap);

	// Register with game mode
	if (AMGRaceGameMode* GameMode = Cast<AMGRaceGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->RegisterCheckpoint(this, CheckpointIndex);
	}

	// Update runtime visibility
	CheckpointMesh->SetVisibility(bShowMeshAtRuntime);
	UpdateVisuals();
}

void AMGCheckpoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind delegate to prevent dangling references
	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &AMGCheckpoint::OnTriggerOverlap);
	}

	Super::EndPlay(EndPlayReason);
}

void AMGCheckpoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Update trigger volume size
	TriggerVolume->SetBoxExtent(FVector(CheckpointDepth * 0.5f, CheckpointWidth * 0.5f, CheckpointHeight * 0.5f));

	// Update visuals
	UpdateVisuals();

	// Update arrow color based on type
	switch (CheckpointType)
	{
	case EMGCheckpointType::StartFinish:
		DirectionArrow->SetArrowColor(FColor::White);
		break;
	case EMGCheckpointType::Sector:
		DirectionArrow->SetArrowColor(FColor::Yellow);
		break;
	case EMGCheckpointType::Split:
		DirectionArrow->SetArrowColor(FColor::Cyan);
		break;
	default:
		DirectionArrow->SetArrowColor(FColor::Green);
		break;
	}
}

FTransform AMGCheckpoint::GetRespawnTransform() const
{
	FTransform RespawnTransform = GetActorTransform();

	// Offset slightly behind the checkpoint
	FVector ForwardOffset = GetActorForwardVector() * -300.0f;
	RespawnTransform.SetLocation(RespawnTransform.GetLocation() + ForwardOffset + FVector(0.0f, 0.0f, 50.0f));

	return RespawnTransform;
}

FVector AMGCheckpoint::GetDirectionToNext() const
{
	if (NextCheckpoint)
	{
		return (NextCheckpoint->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	}

	return GetActorForwardVector();
}

void AMGCheckpoint::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(OtherActor);
	if (!Vehicle)
	{
		return;
	}

	// Check if vehicle is going the right direction (dot product with forward)
	FVector VehicleVelocity = Vehicle->GetVelocity();
	if (VehicleVelocity.SizeSquared() > 1.0f)
	{
		FVector ForwardDir = GetActorForwardVector();
		float DotProduct = FVector::DotProduct(VehicleVelocity.GetSafeNormal(), ForwardDir);

		// Only trigger if going roughly forward (allow some angle)
		if (DotProduct < 0.0f)
		{
			return; // Going backwards, ignore
		}
	}

	// Broadcast event
	OnCheckpointTriggered.Broadcast(Vehicle, CheckpointIndex);

	// Notify game mode
	if (CheckpointType != EMGCheckpointType::Split)
	{
		if (AMGRaceGameMode* GameMode = Cast<AMGRaceGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			GameMode->OnCheckpointPassed(Vehicle, CheckpointIndex);
		}
	}
}

void AMGCheckpoint::UpdateVisuals()
{
	// Update mesh material/color based on type
	// This would typically set material instance parameters

	// Color mapping for checkpoints
	FLinearColor CheckpointColor;
	switch (CheckpointType)
	{
	case EMGCheckpointType::StartFinish:
		CheckpointColor = FLinearColor(1.0f, 1.0f, 1.0f); // White
		break;
	case EMGCheckpointType::Sector:
		CheckpointColor = FLinearColor(1.0f, 0.8f, 0.0f); // Yellow
		break;
	case EMGCheckpointType::Split:
		CheckpointColor = FLinearColor(0.0f, 1.0f, 1.0f); // Cyan
		break;
	default:
		CheckpointColor = FLinearColor(0.0f, 1.0f, 0.0f); // Green
		break;
	}

	// Apply color to mesh material via dynamic material instance
	if (CheckpointMesh && CheckpointMesh->GetStaticMesh())
	{
		UMaterialInterface* BaseMaterial = CheckpointMesh->GetMaterial(0);
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* DynamicMaterial = CheckpointMesh->CreateAndSetMaterialInstanceDynamic(0);
			if (DynamicMaterial)
			{
				DynamicMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), CheckpointColor);
				DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), CheckpointColor);
			}
		}
	}
}

// ==========================================
// AMGStartFinishLine
// ==========================================

AMGStartFinishLine::AMGStartFinishLine()
{
	CheckpointType = EMGCheckpointType::StartFinish;
	CheckpointIndex = 0;
}

void AMGStartFinishLine::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Draw debug grid positions in editor
#if WITH_EDITOR
	// Grid visualization is handled in the editor module
#endif
}

FTransform AMGStartFinishLine::GetGridPositionTransform(int32 GridPosition) const
{
	if (GridPosition < 0 || GridPosition >= GridPositionCount || PositionsPerRow <= 0)
	{
		return GetActorTransform();
	}

	// Calculate row and column
	int32 Row = GridPosition / PositionsPerRow;
	int32 Column = GridPosition % PositionsPerRow;

	// Calculate offset
	float LateralOffset = 0.0f;
	if (PositionsPerRow > 1)
	{
		// Center the row
		float TotalWidth = (PositionsPerRow - 1) * GridLateralSpacing;
		LateralOffset = (Column * GridLateralSpacing) - (TotalWidth * 0.5f);
	}

	// Stagger odd rows
	if (Row % 2 == 1)
	{
		LateralOffset += GridLateralSpacing * 0.5f;
	}

	float ForwardOffset = -Row * GridRowSpacing; // Negative because grid is behind start line

	// Build transform
	FVector Position = GetActorLocation();
	Position += GetActorForwardVector() * ForwardOffset;
	Position += GetActorRightVector() * LateralOffset;
	Position.Z += 10.0f; // Slight lift to avoid ground clipping

	FTransform GridTransform;
	GridTransform.SetLocation(Position);
	GridTransform.SetRotation(GetActorRotation().Quaternion());
	GridTransform.SetScale3D(FVector::OneVector);

	return GridTransform;
}

TArray<FTransform> AMGStartFinishLine::GetAllGridPositions() const
{
	TArray<FTransform> Positions;
	Positions.Reserve(GridPositionCount);

	for (int32 i = 0; i < GridPositionCount; ++i)
	{
		Positions.Add(GetGridPositionTransform(i));
	}

	return Positions;
}

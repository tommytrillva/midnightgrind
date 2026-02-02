// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCheckpointActor.cpp
 * @brief Implementation of checkpoint trigger actors for race tracking.
 *
 * Handles checkpoint overlap detection, track subsystem registration,
 * crossing validation, and racer progress reporting.
 */

#include "Track/MGCheckpointActor.h"
#include "Track/MGTrackSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

AMGCheckpointActor::AMGCheckpointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// Create trigger box
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetBoxExtent(FVector(CheckpointDepth / 2.0f, CheckpointWidth / 2.0f, CheckpointHeight / 2.0f));
	TriggerBox->SetHiddenInGame(true);

	// Create direction arrow
	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
	DirectionArrow->SetupAttachment(RootComponent);
	DirectionArrow->SetArrowColor(FColor::Green);
	DirectionArrow->ArrowSize = 2.0f;
	DirectionArrow->bIsEditorOnly = true;

#if WITH_EDITORONLY_DATA
	// Create billboard
	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(RootComponent);
	Billboard->bIsEditorOnly = true;
#endif
}

void AMGCheckpointActor::BeginPlay()
{
	Super::BeginPlay();

	// Update trigger size
	UpdateTriggerSize();

	// Bind overlap events
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMGCheckpointActor::OnTriggerOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AMGCheckpointActor::OnTriggerOverlapEnd);

	// Show/hide visual
	TriggerBox->SetHiddenInGame(!bShowVisualInGame);

	// Register with track subsystem
	RegisterWithTrackSubsystem();
}

void AMGCheckpointActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind overlap events
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &AMGCheckpointActor::OnTriggerOverlapBegin);
		TriggerBox->OnComponentEndOverlap.RemoveDynamic(this, &AMGCheckpointActor::OnTriggerOverlapEnd);
	}

	UnregisterFromTrackSubsystem();

	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void AMGCheckpointActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AMGCheckpointActor, CheckpointWidth) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AMGCheckpointActor, CheckpointHeight) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AMGCheckpointActor, CheckpointDepth))
	{
		UpdateTriggerSize();
	}

	// Update arrow color based on type
	if (DirectionArrow)
	{
		if (bIsFinishLine)
		{
			DirectionArrow->SetArrowColor(FColor::White);
		}
		else if (bIsSectorSplit)
		{
			DirectionArrow->SetArrowColor(FColor::Yellow);
		}
		else
		{
			DirectionArrow->SetArrowColor(FColor::Green);
		}
	}
}
#endif

// ==========================================
// FUNCTIONS
// ==========================================

FMGCheckpointData AMGCheckpointActor::GetCheckpointData() const
{
	FMGCheckpointData Data;
	Data.Index = CheckpointIndex;
	Data.Position = GetActorLocation();
	Data.Rotation = GetActorRotation();
	Data.Width = CheckpointWidth;
	Data.bIsSectorSplit = bIsSectorSplit;
	Data.SectorIndex = SectorIndex;
	Data.DistanceFromStart = DistanceFromStart;

	return Data;
}

void AMGCheckpointActor::UpdateTriggerSize()
{
	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(FVector(CheckpointDepth / 2.0f, CheckpointWidth / 2.0f, CheckpointHeight / 2.0f));
	}
}

// ==========================================
// INTERNAL
// ==========================================

void AMGCheckpointActor::RegisterWithTrackSubsystem()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TrackSubsystem = World->GetSubsystem<UMGTrackSubsystem>();
	if (TrackSubsystem.IsValid())
	{
		FMGCheckpointData Data = GetCheckpointData();
		Data.CheckpointActor = this;
		TrackSubsystem->RegisterCheckpoint(Data);
	}
}

void AMGCheckpointActor::UnregisterFromTrackSubsystem()
{
	// Checkpoint removal would be handled by track subsystem cleanup
}

void AMGCheckpointActor::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// Only track pawns (vehicles)
	if (!Cast<APawn>(OtherActor))
	{
		return;
	}

	OverlappingActors.Add(OtherActor);
	PreviousPositions.Add(OtherActor, OtherActor->GetActorLocation());

	// For OnOverlap mode, trigger immediately
	if (TriggerMode == EMGCheckpointTriggerMode::OnOverlap)
	{
		int32 RacerID = GetRacerIDFromActor(OtherActor);
		if (RacerID >= 0 && TrackSubsystem.IsValid())
		{
			if (bIsFinishLine)
			{
				TrackSubsystem->OnFinishLineCrossed(RacerID);
			}
			else
			{
				TrackSubsystem->OnCheckpointCrossed(RacerID, CheckpointIndex);
			}
		}
	}
}

void AMGCheckpointActor::OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	// Check if crossed the checkpoint line (for CenterCross/FrontCross modes)
	if (TriggerMode != EMGCheckpointTriggerMode::OnOverlap)
	{
		if (HasCrossedCheckpointLine(OtherActor))
		{
			int32 RacerID = GetRacerIDFromActor(OtherActor);
			if (RacerID >= 0 && TrackSubsystem.IsValid())
			{
				if (bIsFinishLine)
				{
					TrackSubsystem->OnFinishLineCrossed(RacerID);
				}
				else
				{
					TrackSubsystem->OnCheckpointCrossed(RacerID, CheckpointIndex);
				}
			}
		}
	}

	OverlappingActors.Remove(OtherActor);
	PreviousPositions.Remove(OtherActor);
}

bool AMGCheckpointActor::HasCrossedCheckpointLine(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	const FVector* PrevPos = PreviousPositions.Find(Actor);
	if (!PrevPos)
	{
		return false;
	}

	FVector CurrentPos = Actor->GetActorLocation();
	FVector CheckpointPos = GetActorLocation();
	FVector CheckpointForward = GetActorForwardVector();

	// Calculate which side of the checkpoint plane each position is on
	FVector ToPrev = *PrevPos - CheckpointPos;
	FVector ToCurrent = CurrentPos - CheckpointPos;

	float PrevDot = FVector::DotProduct(ToPrev, CheckpointForward);
	float CurrentDot = FVector::DotProduct(ToCurrent, CheckpointForward);

	// Crossed if signs are different (went from one side to the other)
	// And crossed in the correct direction (from negative to positive)
	bool bCrossed = (PrevDot < 0.0f && CurrentDot >= 0.0f);

	return bCrossed;
}

int32 AMGCheckpointActor::GetRacerIDFromActor(AActor* Actor) const
{
	// This would typically get the racer ID from a vehicle component
	// For now, use a simple hash of the actor pointer
	// In practice, vehicles would have a component with their racer ID

	if (!Actor)
	{
		return -1;
	}

	// Try to get from pawn
	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		// Would get from vehicle component
		// For now, use controller ID or generate from pointer
		if (AController* Controller = Pawn->GetController())
		{
			// Player controller = ID 0, AI = sequential IDs
			if (Controller->IsPlayerController())
			{
				return 0;
			}
		}
	}

	// Fallback: generate ID from pointer (not ideal but works for testing)
	return GetTypeHash(Actor) % 100;
}

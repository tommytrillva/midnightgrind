// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Track/MGTrackSubsystem.h"
#include "MGCheckpointActor.generated.h"

class UBoxComponent;
class UArrowComponent;
class UBillboardComponent;

/**
 * Checkpoint trigger mode
 */
UENUM(BlueprintType)
enum class EMGCheckpointTriggerMode : uint8
{
	/** Trigger on any overlap */
	OnOverlap,
	/** Trigger when center of vehicle crosses */
	OnCenterCross,
	/** Trigger when front of vehicle crosses */
	OnFrontCross
};

/**
 * Checkpoint Actor
 * Placed in level to define checkpoint locations
 *
 * Features:
 * - Automatic registration with Track Subsystem
 * - Configurable trigger volume
 * - Support for sector splits
 * - Visual debugging
 */
UCLASS()
class MIDNIGHTGRIND_API AMGCheckpointActor : public AActor
{
	GENERATED_BODY()

public:
	AMGCheckpointActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Checkpoint index (order in sequence) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 CheckpointIndex = 0;

	/** Checkpoint width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float CheckpointWidth = 1500.0f;

	/** Checkpoint height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float CheckpointHeight = 500.0f;

	/** Checkpoint depth (trigger zone thickness) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float CheckpointDepth = 100.0f;

	/** Is this checkpoint a sector split */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsSectorSplit = false;

	/** Sector index (if sector split) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (EditCondition = "bIsSectorSplit"))
	int32 SectorIndex = 0;

	/** Is this the finish line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsFinishLine = false;

	/** Distance from start line (calculated or manual) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float DistanceFromStart = 0.0f;

	/** Trigger mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EMGCheckpointTriggerMode TriggerMode = EMGCheckpointTriggerMode::OnCenterCross;

	/** Show visual in game (for debugging) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint|Debug")
	bool bShowVisualInGame = false;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	/** Get checkpoint data struct */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FMGCheckpointData GetCheckpointData() const;

	/** Update trigger box size */
	UFUNCTION(BlueprintCallable, Category = "Checkpoint")
	void UpdateTriggerSize();

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** Trigger box */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	/** Direction arrow (editor only) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* DirectionArrow;

#if WITH_EDITORONLY_DATA
	/** Billboard for visibility in editor */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBillboardComponent* Billboard;
#endif

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Register with track subsystem */
	void RegisterWithTrackSubsystem();

	/** Unregister from track subsystem */
	void UnregisterFromTrackSubsystem();

	/** Handle overlap begin */
	UFUNCTION()
	void OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Handle overlap end */
	UFUNCTION()
	void OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Check if actor has crossed the checkpoint line */
	bool HasCrossedCheckpointLine(AActor* Actor) const;

	/** Get racer ID from actor */
	int32 GetRacerIDFromActor(AActor* Actor) const;

	/** Actors currently overlapping */
	UPROPERTY()
	TSet<AActor*> OverlappingActors;

	/** Previous positions for crossing detection */
	TMap<AActor*, FVector> PreviousPositions;

	/** Cached track subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMGTrackSubsystem> TrackSubsystem;
};

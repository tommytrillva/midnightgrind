// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGCheckpointActor.h
 * =============================================================================
 *
 * PURPOSE:
 * This file defines the AMGCheckpointActor class, which represents physical
 * checkpoint markers placed in the game world. Checkpoints are invisible
 * trigger volumes that detect when vehicles pass through them, enabling the
 * game to track race progress, lap times, and sector splits.
 *
 * KEY CONCEPTS:
 *
 * 1. CHECKPOINT: An invisible trigger zone that vehicles must pass through.
 *    Think of checkpoints like gates on a ski course - racers must pass
 *    through them in order to complete a valid lap.
 *
 * 2. TRIGGER VOLUME: A 3D box (UBoxComponent) that fires an "overlap" event
 *    when another actor (like a vehicle) enters or exits its bounds.
 *    The overlap event is how Unreal Engine detects collisions without
 *    actually blocking movement.
 *
 * 3. SECTOR SPLIT: Checkpoints can optionally mark sector boundaries.
 *    Racing tracks are typically divided into 3 sectors (S1, S2, S3),
 *    allowing players to compare their performance in each track section.
 *
 * 4. FINISH LINE: A special checkpoint that marks the end of each lap.
 *    When a racer crosses the finish line after passing all checkpoints,
 *    their lap is complete.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * - AMGCheckpointActor is placed in the level by designers (one per checkpoint)
 * - On BeginPlay, it registers itself with UMGTrackSubsystem
 * - UMGTrackSubsystem tracks all checkpoints and calculates race positions
 * - When a vehicle overlaps the trigger box, this actor notifies the subsystem
 * - The subsystem validates the checkpoint (correct order) and updates timing
 *
 * USAGE EXAMPLE:
 * 1. Drag AMGCheckpointActor into your level
 * 2. Set CheckpointIndex (0 = start/finish, 1 = first checkpoint, etc.)
 * 3. Adjust CheckpointWidth/Height/Depth to fit your track
 * 4. For sector timing, enable bIsSectorSplit and set SectorIndex
 * 5. The actor automatically registers with the Track Subsystem at runtime
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Track/MGTrackSubsystem.h"
#include "MGCheckpointActor.generated.h"

// Forward declarations - these tell the compiler these classes exist without
// including their full headers. This speeds up compilation.
class UBoxComponent;      // Unreal's box-shaped collision component
class UArrowComponent;    // Visual arrow showing direction (editor only)
class UBillboardComponent; // 2D sprite that always faces the camera (editor only)

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

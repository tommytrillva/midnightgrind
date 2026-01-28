// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCheckpoint.h
 * @brief Checkpoint system for lap validation, timing, and race progression.
 *
 * This file defines checkpoint actors used to track race progress. Checkpoints
 * are invisible trigger volumes placed around the track that vehicles must pass
 * through in sequence to complete a valid lap. They enable lap counting, split
 * timing, position tracking, and anti-cheat validation.
 *
 * @section checkpoint_concepts Key Concepts
 *
 * CHECKPOINT: An invisible trigger zone that detects when vehicles pass through.
 * Think of checkpoints like timing gates in a ski race - racers must pass through
 * each one in order for their run to count.
 *
 * TRIGGER VOLUME: A UBoxComponent configured to generate "overlap" events when
 * actors enter or exit its bounds. The overlap system allows detection without
 * blocking vehicle movement.
 *
 * START/FINISH LINE: A special checkpoint that marks the beginning and end of
 * each lap. When a vehicle crosses the start/finish line after passing all
 * required checkpoints, a lap is complete.
 *
 * SECTOR: Racing tracks are typically divided into sectors (usually 3) for
 * detailed performance analysis. Sector checkpoints record split times,
 * allowing players to compare their performance in each track section.
 *
 * @section checkpoint_types Checkpoint Types
 *
 * The system supports several checkpoint types:
 * - Normal: Standard checkpoint that must be passed for lap validation
 * - StartFinish: The main timing line that counts laps
 * - Split: Timing marker that does not affect lap validation
 * - Sector: Boundary between track sectors for detailed timing
 *
 * @section checkpoint_architecture Architecture
 *
 * The checkpoint system works with UMGTrackSubsystem:
 * 1. Checkpoints are placed around the track in the level editor
 * 2. Each checkpoint has an index (0 = start/finish, 1 = first checkpoint, etc.)
 * 3. At runtime, checkpoints register with UMGTrackSubsystem
 * 4. When a vehicle overlaps a checkpoint, it notifies the subsystem
 * 5. The subsystem validates the checkpoint (correct order) and updates timing
 * 6. Invalid checkpoint passes (wrong order, skipped) are rejected
 *
 * @section checkpoint_usage Usage Examples
 *
 * @code
 * // Setting up checkpoints (done in level editor, but shown for reference)
 * AMGCheckpoint* StartFinish = SpawnActor<AMGCheckpoint>();
 * StartFinish->CheckpointType = EMGCheckpointType::StartFinish;
 * StartFinish->CheckpointIndex = 0;
 *
 * AMGCheckpoint* Checkpoint1 = SpawnActor<AMGCheckpoint>();
 * Checkpoint1->CheckpointType = EMGCheckpointType::Normal;
 * Checkpoint1->CheckpointIndex = 1;
 *
 * // Linking checkpoints for respawn direction
 * StartFinish->NextCheckpoint = Checkpoint1;
 *
 * // Getting respawn location from last checkpoint
 * AMGCheckpoint* LastCheckpoint = GetLastPassedCheckpoint(RacerID);
 * FTransform RespawnTransform = LastCheckpoint->GetRespawnTransform();
 *
 * // Responding to checkpoint events
 * Checkpoint->OnCheckpointTriggered.AddDynamic(this, &AMyClass::HandleCheckpoint);
 *
 * void AMyClass::HandleCheckpoint(AMGVehiclePawn* Vehicle, int32 Index)
 * {
 *     // Update UI, play sound, record split time, etc.
 *     UpdateRaceHUD(Vehicle, Index);
 * }
 * @endcode
 *
 * @section checkpoint_start_grid Start/Finish Line Grid
 *
 * AMGStartFinishLine extends AMGCheckpoint to provide starting grid functionality:
 * - Defines spawn positions for all racers in grid formation
 * - Supports staggered (offset) or inline grid layouts
 * - Configurable spacing between grid positions
 *
 * @section checkpoint_related Related Systems
 * - UMGTrackSubsystem: Manages checkpoint validation and race timing
 * - Race HUD: Displays checkpoint split times and lap counts
 * - Respawn System: Uses last passed checkpoint for respawn position
 * - Ghost Recording: Records checkpoint passage times for playback
 *
 * @see AMGCheckpoint
 * @see AMGStartFinishLine
 * @see EMGCheckpointType
 * @see FOnCheckpointTriggered
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGCheckpoint.generated.h"

class UBoxComponent;
class UArrowComponent;
class UStaticMeshComponent;
class AMGVehiclePawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointTriggered, AMGVehiclePawn*, Vehicle, int32, CheckpointIndex);

/**
 * Checkpoint type
 */
UENUM(BlueprintType)
enum class EMGCheckpointType : uint8
{
	/** Normal checkpoint */
	Normal		UMETA(DisplayName = "Normal"),

	/** Start/Finish line */
	StartFinish	UMETA(DisplayName = "Start/Finish"),

	/** Split time marker (doesn't count as checkpoint) */
	Split		UMETA(DisplayName = "Split Timer"),

	/** Sector boundary */
	Sector		UMETA(DisplayName = "Sector")
};

/**
 * Checkpoint actor - triggers when vehicle passes through
 * Used for lap counting, position tracking, and respawn points
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGCheckpoint : public AActor
{
	GENERATED_BODY()

public:
	AMGCheckpoint();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	//~ End AActor Interface

	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Trigger volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	/** Visual mesh (for editor and optionally runtime) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	/** Direction arrow (editor only) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> DirectionArrow;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Checkpoint type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EMGCheckpointType CheckpointType = EMGCheckpointType::Normal;

	/** Index in checkpoint sequence (0 = start/finish) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 CheckpointIndex = 0;

	/** Width of the checkpoint trigger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (ClampMin = "100.0"))
	float CheckpointWidth = 1000.0f;

	/** Height of the checkpoint trigger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (ClampMin = "100.0"))
	float CheckpointHeight = 500.0f;

	/** Depth of the checkpoint trigger (how far vehicle must pass) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint", meta = (ClampMin = "50.0"))
	float CheckpointDepth = 200.0f;

	/** Show visual mesh at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bShowMeshAtRuntime = false;

	/** Reference to next checkpoint (for respawn direction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	TObjectPtr<AMGCheckpoint> NextCheckpoint;

	/** Ideal racing line position (for AI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FVector RacingLineOffset = FVector::ZeroVector;

	// ==========================================
	// RESPAWN
	// ==========================================

	/** Get respawn transform for this checkpoint */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FTransform GetRespawnTransform() const;

	/** Get direction to next checkpoint */
	UFUNCTION(BlueprintPure, Category = "Checkpoint")
	FVector GetDirectionToNext() const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when a vehicle passes through this checkpoint */
	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnCheckpointTriggered OnCheckpointTriggered;

protected:
	/** Handle overlap events */
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Update visual appearance based on type */
	void UpdateVisuals();
};

/**
 * Start/Finish line - special checkpoint with grid positions
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGStartFinishLine : public AMGCheckpoint
{
	GENERATED_BODY()

public:
	AMGStartFinishLine();

	// ==========================================
	// GRID POSITIONS
	// ==========================================

	/** Number of grid positions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "1", ClampMax = "16"))
	int32 GridPositionCount = 8;

	/** Spacing between grid positions (lateral) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridLateralSpacing = 400.0f;

	/** Spacing between grid rows (forward) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridRowSpacing = 600.0f;

	/** Positions per row (for staggered grid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 PositionsPerRow = 2;

	/** Get world transform for a grid position */
	UFUNCTION(BlueprintPure, Category = "Grid")
	FTransform GetGridPositionTransform(int32 GridPosition) const;

	/** Get all grid positions */
	UFUNCTION(BlueprintPure, Category = "Grid")
	TArray<FTransform> GetAllGridPositions() const;

protected:
	/** Draw grid positions in editor */
	virtual void OnConstruction(const FTransform& Transform) override;
};

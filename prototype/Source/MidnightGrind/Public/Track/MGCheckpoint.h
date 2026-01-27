// Copyright Midnight Grind. All Rights Reserved.

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

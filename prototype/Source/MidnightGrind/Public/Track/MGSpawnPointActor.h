// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSpawnPointActor.generated.h"

class UArrowComponent;
class UBillboardComponent;
class UBoxComponent;

/**
 * Spawn Point Actor
 * Placed in level to define vehicle spawn/start positions
 *
 * Features:
 * - Grid position assignment (row/column)
 * - Support for staggered and inline starts
 * - Visual debugging in editor
 * - Automatic registration with race game mode
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGSpawnPointActor : public AActor
{
	GENERATED_BODY()

public:
	AMGSpawnPointActor();

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Grid position (1 = pole position, 2 = second, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	int32 GridPosition = 1;

	/** Row index (0 = front row) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	int32 RowIndex = 0;

	/** Column index (0 = left, 1 = right for 2-wide grid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	int32 ColumnIndex = 0;

	/** Is this spawn point active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	bool bIsActive = true;

	/** Is this the pole position spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	bool bIsPolePosition = false;

	/** Optional: specific racer ID that must use this spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point")
	int32 ReservedForRacerID = -1;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	/** Get spawn transform */
	UFUNCTION(BlueprintPure, Category = "Spawn Point")
	FTransform GetSpawnTransform() const;

	/** Get grid position */
	UFUNCTION(BlueprintPure, Category = "Spawn Point")
	int32 GetGridPosition() const { return GridPosition; }

	/** Check if available for spawning */
	UFUNCTION(BlueprintPure, Category = "Spawn Point")
	bool IsAvailable() const { return bIsActive; }

	/** Mark as used */
	UFUNCTION(BlueprintCallable, Category = "Spawn Point")
	void MarkAsUsed(int32 RacerID);

	/** Reset to available */
	UFUNCTION(BlueprintCallable, Category = "Spawn Point")
	void ResetAvailability();

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** Direction arrow */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* DirectionArrow;

	/** Spawn area visualization */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* SpawnArea;

#if WITH_EDITORONLY_DATA
	/** Billboard for visibility */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBillboardComponent* Billboard;
#endif

	// ==========================================
	// STATE
	// ==========================================

	/** Currently assigned racer */
	UPROPERTY(BlueprintReadOnly, Category = "Spawn Point")
	int32 AssignedRacerID = -1;

	/** Is currently occupied */
	UPROPERTY(BlueprintReadOnly, Category = "Spawn Point")
	bool bIsOccupied = false;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update visual in editor */
	void UpdateVisuals();
};

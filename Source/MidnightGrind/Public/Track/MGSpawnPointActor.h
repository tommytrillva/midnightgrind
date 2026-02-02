// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGSpawnPointActor.h
 * @brief Starting grid position markers for race vehicle spawning.
 *
 * This file defines the AMGSpawnPointActor class, which represents individual
 * starting positions on the race grid. These actors are placed in the level to
 * define where vehicles spawn at the start of a race, including their position
 * in the starting grid hierarchy (pole position, second place, etc.).
 *
 * @section spawn_concepts Key Concepts
 *
 * STARTING GRID: The formation of vehicles before a race begins. In real racing,
 * the grid order is typically determined by qualifying times. The "pole position"
 * (first place on the grid) has the best starting advantage.
 *
 * GRID POSITION: A number indicating where in the starting order a vehicle
 * sits. Position 1 is pole position (front of the grid), with higher numbers
 * further back.
 *
 * ROW/COLUMN: Most racing grids are arranged in rows (front to back) and columns
 * (left to right). A 2-wide grid has 2 columns, with odd-numbered grid positions
 * typically on one side and even on the other.
 *
 * STAGGERED START: A grid arrangement where vehicles are offset rather than
 * directly behind each other, reducing the chance of first-corner collisions.
 *
 * @section spawn_architecture Architecture
 *
 * The spawn point system integrates with race management:
 * 1. Designer places AMGSpawnPointActor instances in the level
 * 2. Each spawn point is configured with grid position and row/column indices
 * 3. On race start, the Game Mode queries spawn points by grid position
 * 4. Vehicles are spawned at the transform provided by each spawn point
 * 5. Spawn points track usage to prevent double-spawning
 *
 * @section spawn_usage Usage Examples
 *
 * @code
 * // Finding all spawn points in a level
 * TArray<AMGSpawnPointActor*> SpawnPoints;
 * for (TActorIterator<AMGSpawnPointActor> It(GetWorld()); It; ++It)
 * {
 *     if (It->IsAvailable())
 *     {
 *         SpawnPoints.Add(*It);
 *     }
 * }
 *
 * // Sort by grid position for proper order
 * SpawnPoints.Sort([](const AMGSpawnPointActor& A, const AMGSpawnPointActor& B)
 * {
 *     return A.GetGridPosition() < B.GetGridPosition();
 * });
 *
 * // Spawn a vehicle at pole position
 * AMGSpawnPointActor* PoleSpawn = SpawnPoints[0];
 * FTransform SpawnTransform = PoleSpawn->GetSpawnTransform();
 * AVehicle* NewVehicle = GetWorld()->SpawnActor<AVehicle>(VehicleClass, SpawnTransform);
 * PoleSpawn->MarkAsUsed(RacerID);
 *
 * // Reset spawn points for a new race
 * for (AMGSpawnPointActor* SpawnPoint : SpawnPoints)
 * {
 *     SpawnPoint->ResetAvailability();
 * }
 * @endcode
 *
 * @section spawn_editor Editor Setup
 *
 * When placing spawn points in the editor:
 * 1. Drag AMGSpawnPointActor into the level at each starting position
 * 2. Set GridPosition (1 = pole, 2 = second, etc.)
 * 3. Set RowIndex and ColumnIndex for proper grid layout
 * 4. Rotate the actor to face the track direction
 * 5. The arrow component shows the forward direction for spawned vehicles
 *
 * @section spawn_related Related Systems
 * - Race Game Mode: Uses spawn points to place vehicles at race start
 * - Vehicle Pawn: Spawned at the transform provided by spawn points
 * - UMGTrackSubsystem: May register spawn points for easy lookup
 *
 * @see AMGStartFinishLine (alternative grid system in MGCheckpoint.h)
 */

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

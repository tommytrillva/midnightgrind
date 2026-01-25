// Copyright Midnight Grind. All Rights Reserved.
// Stage 52: Vehicle Spawn Subsystem - MVP Vehicle Spawning

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGVehicleSpawnSubsystem.generated.h"

class AMGVehiclePawn;
class AMGSpawnPointActor;
class APlayerController;

/**
 * Vehicle spawn request
 */
USTRUCT(BlueprintType)
struct FMGVehicleSpawnRequest
{
	GENERATED_BODY()

	/** Vehicle ID to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName VehicleID;

	/** Grid position (1 = pole position) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 GridPosition = 1;

	/** Is this the player's vehicle? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bIsPlayer = false;

	/** Is this an AI vehicle? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bIsAI = false;

	/** Display name for racer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FString DisplayName;

	/** AI skill level (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float AISkill = 0.5f;
};

/**
 * Spawned vehicle info
 */
USTRUCT(BlueprintType)
struct FMGSpawnedVehicle
{
	GENERATED_BODY()

	/** The spawned vehicle actor */
	UPROPERTY(BlueprintReadOnly, Category = "Spawn")
	TWeakObjectPtr<AMGVehiclePawn> Vehicle;

	/** Grid position */
	UPROPERTY(BlueprintReadOnly, Category = "Spawn")
	int32 GridPosition = 0;

	/** Is player vehicle */
	UPROPERTY(BlueprintReadOnly, Category = "Spawn")
	bool bIsPlayer = false;

	/** Racer index assigned by game mode */
	UPROPERTY(BlueprintReadOnly, Category = "Spawn")
	int32 RacerIndex = -1;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleSpawned, AMGVehiclePawn*, Vehicle, bool, bIsPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllVehiclesSpawned);

/**
 * Vehicle Spawn Subsystem
 *
 * Handles spawning vehicles at race start:
 * - Finds spawn points in level
 * - Spawns player and AI vehicles
 * - Registers vehicles with race game mode
 * - Possesses player vehicle
 *
 * This is a WorldSubsystem - it exists per-level.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when a vehicle is spawned */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnVehicleSpawned OnVehicleSpawned;

	/** Called when all vehicles have been spawned */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllVehiclesSpawned OnAllVehiclesSpawned;

	// ==========================================
	// SPAWN CONTROL
	// ==========================================

	/**
	 * Spawn all race vehicles
	 * @param PlayerVehicleID - Vehicle ID for player
	 * @param AIVehicles - Array of AI vehicle spawns
	 * @return True if spawn successful
	 */
	UFUNCTION(BlueprintCallable, Category = "VehicleSpawn")
	bool SpawnRaceVehicles(FName PlayerVehicleID, const TArray<FMGVehicleSpawnRequest>& AIVehicles);

	/**
	 * Spawn a single vehicle at grid position
	 */
	UFUNCTION(BlueprintCallable, Category = "VehicleSpawn")
	AMGVehiclePawn* SpawnVehicle(const FMGVehicleSpawnRequest& Request);

	/**
	 * Spawn player vehicle at pole position
	 */
	UFUNCTION(BlueprintCallable, Category = "VehicleSpawn")
	AMGVehiclePawn* SpawnPlayerVehicle(FName VehicleID, int32 GridPosition = 1);

	/**
	 * Despawn all race vehicles
	 */
	UFUNCTION(BlueprintCallable, Category = "VehicleSpawn")
	void DespawnAllVehicles();

	/**
	 * Possess the player vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "VehicleSpawn")
	bool PossessPlayerVehicle(APlayerController* PC);

	// ==========================================
	// QUERIES
	// ==========================================

	/** Get player vehicle */
	UFUNCTION(BlueprintPure, Category = "VehicleSpawn")
	AMGVehiclePawn* GetPlayerVehicle() const;

	/** Get all spawned vehicles */
	UFUNCTION(BlueprintPure, Category = "VehicleSpawn")
	TArray<AMGVehiclePawn*> GetAllSpawnedVehicles() const;

	/** Get spawn point at grid position */
	UFUNCTION(BlueprintPure, Category = "VehicleSpawn")
	AMGSpawnPointActor* GetSpawnPoint(int32 GridPosition) const;

	/** Get total spawn point count */
	UFUNCTION(BlueprintPure, Category = "VehicleSpawn")
	int32 GetSpawnPointCount() const { return SpawnPoints.Num(); }

	/** Are all vehicles spawned? */
	UFUNCTION(BlueprintPure, Category = "VehicleSpawn")
	bool AreVehiclesSpawned() const { return bVehiclesSpawned; }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Default vehicle class to spawn if specific not found */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AMGVehiclePawn> DefaultVehicleClass;

	/** Vehicle class map (VehicleID -> Class) */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TMap<FName, TSubclassOf<AMGVehiclePawn>> VehicleClassMap;

protected:
	// ==========================================
	// STATE
	// ==========================================

	/** All spawn points in level */
	UPROPERTY()
	TArray<AMGSpawnPointActor*> SpawnPoints;

	/** All spawned vehicles */
	UPROPERTY()
	TArray<FMGSpawnedVehicle> SpawnedVehicles;

	/** Player vehicle */
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> PlayerVehicle;

	/** Vehicles have been spawned */
	bool bVehiclesSpawned = false;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Find all spawn points in level */
	void CollectSpawnPoints();

	/** Get vehicle class for ID */
	TSubclassOf<AMGVehiclePawn> GetVehicleClass(FName VehicleID) const;

	/** Get spawn transform for grid position */
	FTransform GetSpawnTransform(int32 GridPosition) const;

	/** Register vehicle with race game mode */
	void RegisterWithGameMode(AMGVehiclePawn* Vehicle, bool bIsAI, const FString& DisplayName);
};

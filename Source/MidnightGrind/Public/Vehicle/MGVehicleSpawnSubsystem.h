// Copyright Midnight Grind. All Rights Reserved.


#pragma once
// Stage 52: Vehicle Spawn Subsystem - MVP Vehicle Spawning

/**
 * @file MGVehicleSpawnSubsystem.h
 * @brief World subsystem for spawning and managing race vehicles.
 *
 * @section Overview
 * This subsystem handles all vehicle spawning at race start, including player
 * vehicles and AI opponents. It finds spawn points in the level, creates vehicle
 * actors, assigns controllers, and registers vehicles with the race game mode.
 *
 * @section Architecture
 * As a World Subsystem, this system exists once per level and is automatically
 * created/destroyed with the world. This makes it ideal for per-race vehicle
 * management.
 *
 * Spawning Flow:
 * 1. Race mode calls SpawnRaceVehicles() with player and AI configurations
 * 2. Subsystem finds all AMGSpawnPointActor instances in level
 * 3. Vehicles spawn at grid positions (1 = pole position)
 * 4. Player vehicle is possessed, AI vehicles get AI controllers
 * 5. All vehicles register with the race game mode
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **World Subsystem**: A special UE5 class that exists per-world (level).
 * Unlike Game Instance subsystems that persist across levels, World Subsystems
 * are created when a level loads and destroyed when it unloads. Perfect for
 * level-specific systems like race vehicle management.
 *
 * **Grid Position**: Starting position on the race grid. Position 1 is "pole
 * position" (front of grid), higher numbers are further back. Spawn points
 * in the level are tagged with their grid positions.
 *
 * **Possession**: When a controller "possesses" a pawn, it takes control.
 * The player controller possesses the player's vehicle, AI controllers
 * possess AI vehicles. This is how input gets routed to the right pawn.
 *
 * **Spawn Point Actor**: A level-placed actor marking where vehicles spawn.
 * Contains grid position, forward direction, and any spawn-specific settings.
 *
 * @section Usage Example Usage
 * @code
 * // In your race game mode BeginPlay:
 * UMGVehicleSpawnSubsystem* SpawnSystem = GetWorld()->GetSubsystem<UMGVehicleSpawnSubsystem>();
 *
 * // Create AI opponent requests
 * TArray<FMGVehicleSpawnRequest> AIVehicles;
 * FMGVehicleSpawnRequest AI1;
 * AI1.VehicleID = "Silvia_S15";
 * AI1.GridPosition = 2;
 * AI1.bIsAI = true;
 * AI1.AISkill = 0.7f;
 * AI1.DisplayName = "Rival 1";
 * AIVehicles.Add(AI1);
 *
 * // Spawn all vehicles
 * SpawnSystem->SpawnRaceVehicles("Supra_MK4", AIVehicles);
 *
 * // Possess player vehicle
 * SpawnSystem->PossessPlayerVehicle(GetWorld()->GetFirstPlayerController());
 * @endcode
 *
 * @see AMGSpawnPointActor Level-placed spawn point markers
 * @see AMGRacingAIController AI controller for opponent vehicles
 * @see AMGRaceGameMode Game mode that orchestrates race flow
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGVehicleSpawnSubsystem.generated.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class AMGVehiclePawn;
class AMGSpawnPointActor;
class APlayerController;
class AMGRacingAIController;

// ============================================================================
// SPAWN REQUEST STRUCTURE
// ============================================================================

/**
 * @brief Request structure for spawning a single vehicle.
 *
 * Contains all information needed to spawn and configure a vehicle,
 * including identity, grid position, controller type, and AI settings.
 *
 * **USTRUCT(BlueprintType)** exposes this to Blueprint for easy use
 * in race setup UI and game mode configuration.
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

// ============================================================================
// VEHICLE SPAWN SUBSYSTEM CLASS
// ============================================================================

/**
 * @class UMGVehicleSpawnSubsystem
 * @brief World subsystem managing vehicle spawning and lifecycle.
 *
 * This subsystem is the central authority for spawning race vehicles.
 * It coordinates with spawn points, vehicle classes, and controllers to
 * create a properly configured starting grid.
 *
 * @section Features Features
 * - **Spawn Point Discovery**: Finds all spawn points in the current level
 * - **Grid Positioning**: Places vehicles at correct starting positions
 * - **Vehicle Instantiation**: Creates vehicle actors from class references
 * - **Controller Assignment**: Possesses vehicles with player/AI controllers
 * - **Game Mode Integration**: Registers all vehicles with the race manager
 *
 * @section UnrealMacros Unreal Engine Macro Explanations
 *
 * **UCLASS()** with inheritance from UWorldSubsystem:
 * - Automatically created when a world (level) is created
 * - One instance per world, accessible via GetWorld()->GetSubsystem<T>()
 * - Destroyed when the world is torn down (level unloaded)
 *
 * **virtual void Initialize(FSubsystemCollectionBase& Collection) override**
 * - Called when the subsystem is created
 * - Use for initial setup, finding references, binding delegates
 *
 * **virtual bool ShouldCreateSubsystem(UObject* Outer) const override**
 * - Return false to prevent creation in certain contexts (e.g., editor-only)
 *
 * **TSubclassOf<T>**: A template type holding a class reference.
 * Used to specify which Blueprint class to spawn for vehicles.
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

	/** AI controller class to spawn for AI vehicles */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AMGRacingAIController> AIControllerClass;

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

	/** Spawn and assign AI controller for AI vehicle */
	void SpawnAIController(AMGVehiclePawn* Vehicle, float AISkill);
};

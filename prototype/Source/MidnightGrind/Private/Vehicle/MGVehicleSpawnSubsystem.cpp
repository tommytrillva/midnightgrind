// Copyright Midnight Grind. All Rights Reserved.
// Stage 52: Vehicle Spawn Subsystem - MVP Vehicle Spawning

#include "Vehicle/MGVehicleSpawnSubsystem.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Track/MGSpawnPointActor.h"
#include "GameModes/MGRaceGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogMGVehicleSpawn, Log, All);

// ==========================================
// INITIALIZATION
// ==========================================

void UMGVehicleSpawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogMGVehicleSpawn, Log, TEXT("Vehicle Spawn Subsystem initialized"));

	// Collect spawn points after a short delay (level needs to be fully loaded)
	if (UWorld* World = GetWorld())
	{
		// Try immediate collection
		CollectSpawnPoints();
	}
}

void UMGVehicleSpawnSubsystem::Deinitialize()
{
	// Clean up spawned vehicles
	DespawnAllVehicles();

	Super::Deinitialize();
}

bool UMGVehicleSpawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only create in game worlds (not editor preview)
	if (UWorld* World = Cast<UWorld>(Outer))
	{
		return World->IsGameWorld();
	}
	return false;
}

// ==========================================
// SPAWN CONTROL
// ==========================================

bool UMGVehicleSpawnSubsystem::SpawnRaceVehicles(FName PlayerVehicleID, const TArray<FMGVehicleSpawnRequest>& AIVehicles)
{
	// Ensure spawn points are collected
	if (SpawnPoints.Num() == 0)
	{
		CollectSpawnPoints();
	}

	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogMGVehicleSpawn, Error, TEXT("No spawn points found in level"));
		return false;
	}

	// Clear any existing vehicles
	DespawnAllVehicles();

	// Spawn player vehicle at position 1
	AMGVehiclePawn* Player = SpawnPlayerVehicle(PlayerVehicleID, 1);
	if (!Player)
	{
		UE_LOG(LogMGVehicleSpawn, Error, TEXT("Failed to spawn player vehicle"));
		return false;
	}

	// Spawn AI vehicles
	int32 GridPos = 2;
	for (const FMGVehicleSpawnRequest& AIRequest : AIVehicles)
	{
		FMGVehicleSpawnRequest ModifiedRequest = AIRequest;
		ModifiedRequest.GridPosition = GridPos++;
		ModifiedRequest.bIsAI = true;

		AMGVehiclePawn* AIVehicle = SpawnVehicle(ModifiedRequest);
		if (AIVehicle)
		{
			OnVehicleSpawned.Broadcast(AIVehicle, false);
		}
		else
		{
			UE_LOG(LogMGVehicleSpawn, Warning, TEXT("Failed to spawn AI vehicle at position %d"), ModifiedRequest.GridPosition);
		}
	}

	bVehiclesSpawned = true;
	OnAllVehiclesSpawned.Broadcast();

	UE_LOG(LogMGVehicleSpawn, Log, TEXT("Spawned %d vehicles (1 player + %d AI)"),
		SpawnedVehicles.Num(), SpawnedVehicles.Num() - 1);

	return true;
}

AMGVehiclePawn* UMGVehicleSpawnSubsystem::SpawnVehicle(const FMGVehicleSpawnRequest& Request)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Get vehicle class
	TSubclassOf<AMGVehiclePawn> VehicleClass = GetVehicleClass(Request.VehicleID);
	if (!VehicleClass)
	{
		UE_LOG(LogMGVehicleSpawn, Warning, TEXT("No vehicle class for ID: %s, using default"), *Request.VehicleID.ToString());
		VehicleClass = DefaultVehicleClass;
	}

	if (!VehicleClass)
	{
		// Fallback: try to find any vehicle pawn class
		VehicleClass = AMGVehiclePawn::StaticClass();
	}

	// Get spawn transform
	FTransform SpawnTransform = GetSpawnTransform(Request.GridPosition);

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn vehicle
	AMGVehiclePawn* Vehicle = World->SpawnActor<AMGVehiclePawn>(VehicleClass, SpawnTransform, SpawnParams);
	if (!Vehicle)
	{
		UE_LOG(LogMGVehicleSpawn, Error, TEXT("Failed to spawn vehicle actor"));
		return nullptr;
	}

	// Configure vehicle for AI if needed
	// TODO: Set up AI controller for AI vehicles

	// Track spawned vehicle
	FMGSpawnedVehicle SpawnedInfo;
	SpawnedInfo.Vehicle = Vehicle;
	SpawnedInfo.GridPosition = Request.GridPosition;
	SpawnedInfo.bIsPlayer = Request.bIsPlayer;
	SpawnedVehicles.Add(SpawnedInfo);

	// Track player vehicle
	if (Request.bIsPlayer)
	{
		PlayerVehicle = Vehicle;
	}

	// Register with race game mode
	RegisterWithGameMode(Vehicle, Request.bIsAI, Request.DisplayName);

	UE_LOG(LogMGVehicleSpawn, Log, TEXT("Spawned vehicle %s at position %d (Player: %d)"),
		*Request.VehicleID.ToString(), Request.GridPosition, Request.bIsPlayer);

	return Vehicle;
}

AMGVehiclePawn* UMGVehicleSpawnSubsystem::SpawnPlayerVehicle(FName VehicleID, int32 GridPosition)
{
	FMGVehicleSpawnRequest Request;
	Request.VehicleID = VehicleID;
	Request.GridPosition = GridPosition;
	Request.bIsPlayer = true;
	Request.bIsAI = false;
	Request.DisplayName = TEXT("Player");

	AMGVehiclePawn* Vehicle = SpawnVehicle(Request);
	if (Vehicle)
	{
		OnVehicleSpawned.Broadcast(Vehicle, true);
	}
	return Vehicle;
}

void UMGVehicleSpawnSubsystem::DespawnAllVehicles()
{
	for (FMGSpawnedVehicle& SpawnedInfo : SpawnedVehicles)
	{
		if (SpawnedInfo.Vehicle.IsValid())
		{
			SpawnedInfo.Vehicle->Destroy();
		}
	}

	SpawnedVehicles.Empty();
	PlayerVehicle = nullptr;
	bVehiclesSpawned = false;

	UE_LOG(LogMGVehicleSpawn, Log, TEXT("All vehicles despawned"));
}

bool UMGVehicleSpawnSubsystem::PossessPlayerVehicle(APlayerController* PC)
{
	if (!PC)
	{
		UE_LOG(LogMGVehicleSpawn, Error, TEXT("No player controller provided"));
		return false;
	}

	if (!PlayerVehicle.IsValid())
	{
		UE_LOG(LogMGVehicleSpawn, Error, TEXT("No player vehicle spawned"));
		return false;
	}

	PC->Possess(PlayerVehicle.Get());
	UE_LOG(LogMGVehicleSpawn, Log, TEXT("Player possessed vehicle"));

	return true;
}

// ==========================================
// QUERIES
// ==========================================

AMGVehiclePawn* UMGVehicleSpawnSubsystem::GetPlayerVehicle() const
{
	return PlayerVehicle.Get();
}

TArray<AMGVehiclePawn*> UMGVehicleSpawnSubsystem::GetAllSpawnedVehicles() const
{
	TArray<AMGVehiclePawn*> Vehicles;
	for (const FMGSpawnedVehicle& SpawnedInfo : SpawnedVehicles)
	{
		if (SpawnedInfo.Vehicle.IsValid())
		{
			Vehicles.Add(SpawnedInfo.Vehicle.Get());
		}
	}
	return Vehicles;
}

AMGSpawnPointActor* UMGVehicleSpawnSubsystem::GetSpawnPoint(int32 GridPosition) const
{
	for (AMGSpawnPointActor* SpawnPoint : SpawnPoints)
	{
		if (SpawnPoint && SpawnPoint->GetGridPosition() == GridPosition)
		{
			return SpawnPoint;
		}
	}
	return nullptr;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGVehicleSpawnSubsystem::CollectSpawnPoints()
{
	SpawnPoints.Empty();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Find all spawn point actors
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, AMGSpawnPointActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AMGSpawnPointActor* SpawnPoint = Cast<AMGSpawnPointActor>(Actor))
		{
			if (SpawnPoint->IsAvailable())
			{
				SpawnPoints.Add(SpawnPoint);
			}
		}
	}

	// Sort by grid position
	SpawnPoints.Sort([](const AMGSpawnPointActor& A, const AMGSpawnPointActor& B)
	{
		return A.GetGridPosition() < B.GetGridPosition();
	});

	UE_LOG(LogMGVehicleSpawn, Log, TEXT("Collected %d spawn points"), SpawnPoints.Num());
}

TSubclassOf<AMGVehiclePawn> UMGVehicleSpawnSubsystem::GetVehicleClass(FName VehicleID) const
{
	if (const TSubclassOf<AMGVehiclePawn>* Class = VehicleClassMap.Find(VehicleID))
	{
		return *Class;
	}
	return DefaultVehicleClass;
}

FTransform UMGVehicleSpawnSubsystem::GetSpawnTransform(int32 GridPosition) const
{
	// Find spawn point for this position
	AMGSpawnPointActor* SpawnPoint = GetSpawnPoint(GridPosition);
	if (SpawnPoint)
	{
		return SpawnPoint->GetSpawnTransform();
	}

	// Fallback: calculate grid position procedurally
	// Assuming 2-wide grid with 5m spacing
	const float RowSpacing = 8.0f * 100.0f;  // 8m between rows
	const float ColumnSpacing = 4.0f * 100.0f;  // 4m between columns

	int32 Row = (GridPosition - 1) / 2;
	int32 Column = (GridPosition - 1) % 2;

	FVector Location;
	Location.X = -Row * RowSpacing;  // Cars behind each other
	Location.Y = (Column == 0) ? -ColumnSpacing / 2.0f : ColumnSpacing / 2.0f;
	Location.Z = 50.0f;  // Slight offset from ground

	FRotator Rotation = FRotator::ZeroRotator;

	UE_LOG(LogMGVehicleSpawn, Warning, TEXT("Using fallback spawn transform for position %d"), GridPosition);

	return FTransform(Rotation, Location);
}

void UMGVehicleSpawnSubsystem::RegisterWithGameMode(AMGVehiclePawn* Vehicle, bool bIsAI, const FString& DisplayName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get race game mode
	AMGRaceGameMode* GameMode = Cast<AMGRaceGameMode>(World->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogMGVehicleSpawn, Warning, TEXT("No race game mode found - vehicle not registered"));
		return;
	}

	// Register with game mode
	FText Name = DisplayName.IsEmpty() ? FText::FromString(TEXT("Racer")) : FText::FromString(DisplayName);
	int32 RacerIndex = GameMode->RegisterRacer(Vehicle, bIsAI, Name);

	// Update our tracking
	for (FMGSpawnedVehicle& SpawnedInfo : SpawnedVehicles)
	{
		if (SpawnedInfo.Vehicle == Vehicle)
		{
			SpawnedInfo.RacerIndex = RacerIndex;
			break;
		}
	}

	UE_LOG(LogMGVehicleSpawn, Log, TEXT("Registered vehicle with game mode, racer index: %d"), RacerIndex);
}

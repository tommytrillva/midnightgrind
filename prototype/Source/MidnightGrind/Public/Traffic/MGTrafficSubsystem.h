// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGTrafficSubsystem.generated.h"

class AMGTrafficVehicle;

/**
 * Traffic vehicle type
 */
UENUM(BlueprintType)
enum class EMGTrafficVehicleType : uint8
{
	Sedan UMETA(DisplayName = "Sedan"),
	SUV UMETA(DisplayName = "SUV"),
	Truck UMETA(DisplayName = "Truck"),
	Van UMETA(DisplayName = "Van"),
	SportsCar UMETA(DisplayName = "Sports Car"),
	Motorcycle UMETA(DisplayName = "Motorcycle"),
	Bus UMETA(DisplayName = "Bus"),
	Semi UMETA(DisplayName = "Semi Truck"),
	Taxi UMETA(DisplayName = "Taxi"),
	DeliveryVan UMETA(DisplayName = "Delivery Van")
};

/**
 * Traffic behavior mode
 */
UENUM(BlueprintType)
enum class EMGTrafficBehavior : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Aggressive UMETA(DisplayName = "Aggressive"),
	Cautious UMETA(DisplayName = "Cautious"),
	Distracted UMETA(DisplayName = "Distracted"),
	Racing UMETA(DisplayName = "Racing"),
	Panicked UMETA(DisplayName = "Panicked"),
	Parked UMETA(DisplayName = "Parked"),
	Pulling UMETA(DisplayName = "Pulling Over")
};

/**
 * Traffic density preset
 */
UENUM(BlueprintType)
enum class EMGTrafficDensityPreset : uint8
{
	None UMETA(DisplayName = "No Traffic"),
	VeryLight UMETA(DisplayName = "Very Light"),
	Light UMETA(DisplayName = "Light"),
	Medium UMETA(DisplayName = "Medium"),
	Heavy UMETA(DisplayName = "Heavy"),
	RushHour UMETA(DisplayName = "Rush Hour"),
	Gridlock UMETA(DisplayName = "Gridlock")
};

/**
 * Lane type
 */
UENUM(BlueprintType)
enum class EMGLaneType : uint8
{
	Regular UMETA(DisplayName = "Regular"),
	HOV UMETA(DisplayName = "HOV/Carpool"),
	Bus UMETA(DisplayName = "Bus Only"),
	Bike UMETA(DisplayName = "Bike Lane"),
	Parking UMETA(DisplayName = "Parking"),
	Emergency UMETA(DisplayName = "Emergency"),
	TurnOnly UMETA(DisplayName = "Turn Only")
};

/**
 * Traffic vehicle state
 */
USTRUCT(BlueprintType)
struct FMGTrafficVehicleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 VehicleID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	EMGTrafficVehicleType VehicleType = EMGTrafficVehicleType::Sedan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	EMGTrafficBehavior Behavior = EMGTrafficBehavior::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	TWeakObjectPtr<AMGTrafficVehicle> VehicleActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float TargetSpeed = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 CurrentLaneIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 CurrentRoadIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float DistanceFromPlayer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bIsVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bHasCollided = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float TimeAlive = 0.0f;
};

/**
 * Road segment for traffic routing
 */
USTRUCT(BlueprintType)
struct FMGRoadSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	int32 RoadID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	FName RoadName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	TArray<FVector> SplinePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	int32 NumLanes = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	float LaneWidth = 350.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	float SpeedLimit = 35.0f; // mph

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	bool bIsOneWay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	bool bIsHighway = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	TArray<int32> ConnectedRoadIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	float TrafficDensityMultiplier = 1.0f;
};

/**
 * Intersection data
 */
USTRUCT(BlueprintType)
struct FMGIntersection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	int32 IntersectionID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	TArray<int32> ConnectedRoadIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	bool bHasTrafficLight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	float LightCycleDuration = 30.0f; // seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	int32 CurrentGreenRoadIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	float LightTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection")
	bool bIsYellow = false;
};

/**
 * Spawn point for traffic
 */
USTRUCT(BlueprintType)
struct FMGTrafficSpawnPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 RoadID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 LaneIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<EMGTrafficVehicleType> AllowedTypes;
};

/**
 * Traffic settings
 */
USTRUCT(BlueprintType)
struct FMGTrafficSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EMGTrafficDensityPreset DensityPreset = EMGTrafficDensityPreset::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 MaxActiveVehicles = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float SpawnDistance = 15000.0f; // Distance from player to spawn

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DespawnDistance = 20000.0f; // Distance to despawn

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MinSpawnInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MaxSpawnInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float AggressiveDriverChance = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DistractedDriverChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableMotorcycles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableTrucks = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bReactToPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bReactToPolice = true;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrafficVehicleSpawned, int32, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrafficVehicleDespawned, int32, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrafficCollision, int32, VehicleID, AActor*, OtherActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrafficLightChanged, int32, IntersectionID, bool, bIsGreen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrafficDensityChanged, EMGTrafficDensityPreset, NewDensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrafficPanicked, int32, VehicleID, FVector, PanicSource);

/**
 * Traffic AI Subsystem
 *
 * Manages civilian traffic vehicles, their AI behavior,
 * spawning/despawning, and traffic light control.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrafficSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficVehicleSpawned OnTrafficVehicleSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficVehicleDespawned OnTrafficVehicleDespawned;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficCollision OnTrafficCollision;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficLightChanged OnTrafficLightChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficDensityChanged OnTrafficDensityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficPanicked OnTrafficPanicked;

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetTrafficSettings(const FMGTrafficSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Settings")
	FMGTrafficSettings GetTrafficSettings() const { return Settings; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetTrafficDensity(EMGTrafficDensityPreset Density);

	UFUNCTION(BlueprintPure, Category = "Settings")
	EMGTrafficDensityPreset GetTrafficDensity() const { return Settings.DensityPreset; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetTrafficEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool IsTrafficEnabled() const { return bTrafficEnabled; }

	// Spawning
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	int32 SpawnTrafficVehicle(FVector Location, FRotator Rotation, EMGTrafficVehicleType Type);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void DespawnTrafficVehicle(int32 VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void DespawnAllTraffic();

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void RegisterSpawnPoint(const FMGTrafficSpawnPoint& SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void ClearSpawnPoints();

	// Vehicle Queries
	UFUNCTION(BlueprintPure, Category = "Vehicles")
	int32 GetActiveVehicleCount() const;

	UFUNCTION(BlueprintPure, Category = "Vehicles")
	TArray<FMGTrafficVehicleState> GetAllTrafficVehicles() const;

	UFUNCTION(BlueprintPure, Category = "Vehicles")
	FMGTrafficVehicleState GetTrafficVehicle(int32 VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Vehicles")
	TArray<FMGTrafficVehicleState> GetVehiclesInRadius(FVector Center, float Radius) const;

	UFUNCTION(BlueprintPure, Category = "Vehicles")
	FMGTrafficVehicleState GetNearestVehicle(FVector Location) const;

	// Vehicle Control
	UFUNCTION(BlueprintCallable, Category = "Control")
	void SetVehicleBehavior(int32 VehicleID, EMGTrafficBehavior Behavior);

	UFUNCTION(BlueprintCallable, Category = "Control")
	void SetVehicleTargetSpeed(int32 VehicleID, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Control")
	void ForceVehicleLaneChange(int32 VehicleID, bool bMoveRight);

	UFUNCTION(BlueprintCallable, Category = "Control")
	void PanicVehiclesInRadius(FVector Center, float Radius);

	UFUNCTION(BlueprintCallable, Category = "Control")
	void StopVehicle(int32 VehicleID);

	// Roads
	UFUNCTION(BlueprintCallable, Category = "Roads")
	void RegisterRoad(const FMGRoadSegment& Road);

	UFUNCTION(BlueprintCallable, Category = "Roads")
	void RegisterIntersection(const FMGIntersection& Intersection);

	UFUNCTION(BlueprintPure, Category = "Roads")
	FMGRoadSegment GetRoad(int32 RoadID) const;

	UFUNCTION(BlueprintPure, Category = "Roads")
	FMGIntersection GetIntersection(int32 IntersectionID) const;

	UFUNCTION(BlueprintPure, Category = "Roads")
	TArray<FMGRoadSegment> GetAllRoads() const;

	// Traffic Lights
	UFUNCTION(BlueprintCallable, Category = "Lights")
	void SetTrafficLightState(int32 IntersectionID, int32 GreenRoadIndex);

	UFUNCTION(BlueprintCallable, Category = "Lights")
	void ForceAllLightsGreen();

	UFUNCTION(BlueprintCallable, Category = "Lights")
	void ResumeNormalLightCycle();

	UFUNCTION(BlueprintPure, Category = "Lights")
	bool IsLightGreenForRoad(int32 IntersectionID, int32 RoadID) const;

	// Player
	UFUNCTION(BlueprintCallable, Category = "Player")
	void UpdatePlayerPosition(FVector Position, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Player")
	void NotifyPlayerCollision(int32 VehicleID);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetTotalVehiclesSpawned() const { return TotalVehiclesSpawned; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetTotalCollisions() const { return TotalCollisions; }

protected:
	void UpdateTraffic(float DeltaTime);
	void UpdateVehicleAI(FMGTrafficVehicleState& Vehicle, float DeltaTime);
	void UpdateTrafficLights(float DeltaTime);
	void SpawnTrafficIfNeeded();
	void DespawnDistantVehicles();
	FVector GetLanePosition(int32 RoadID, int32 LaneIndex, float Distance) const;
	EMGTrafficVehicleType SelectRandomVehicleType() const;
	float GetDensityMultiplier() const;

private:
	FTimerHandle UpdateTimerHandle;

	// Settings
	UPROPERTY()
	FMGTrafficSettings Settings;

	UPROPERTY()
	bool bTrafficEnabled = true;

	UPROPERTY()
	bool bForcedGreenLights = false;

	// Active vehicles
	UPROPERTY()
	TMap<int32, FMGTrafficVehicleState> ActiveVehicles;

	UPROPERTY()
	int32 NextVehicleID = 1;

	// Road network
	UPROPERTY()
	TMap<int32, FMGRoadSegment> Roads;

	UPROPERTY()
	TMap<int32, FMGIntersection> Intersections;

	UPROPERTY()
	TArray<FMGTrafficSpawnPoint> SpawnPoints;

	// Player tracking
	UPROPERTY()
	FVector PlayerPosition = FVector::ZeroVector;

	UPROPERTY()
	float PlayerSpeed = 0.0f;

	// Spawn timing
	UPROPERTY()
	float TimeSinceLastSpawn = 0.0f;

	UPROPERTY()
	float NextSpawnInterval = 2.0f;

	// Stats
	UPROPERTY()
	int32 TotalVehiclesSpawned = 0;

	UPROPERTY()
	int32 TotalCollisions = 0;
};

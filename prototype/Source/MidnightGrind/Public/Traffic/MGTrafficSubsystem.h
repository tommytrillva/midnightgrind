// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGTrafficSubsystem.generated.h"

class AMGTrafficVehicle;
class UMGTimeOfDaySubsystem;
class UMGNearMissSubsystem;

/**
 * @brief Traffic vehicle type enumeration
 *
 * Defines the different types of civilian and special vehicles
 * that can populate the streets of Midnight City.
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
	DeliveryVan UMETA(DisplayName = "Delivery Van"),
	// Emergency vehicles
	Ambulance UMETA(DisplayName = "Ambulance"),
	FireTruck UMETA(DisplayName = "Fire Truck"),
	PoliceCar UMETA(DisplayName = "Police Car")
};

/**
 * @brief Traffic behavior mode enumeration
 *
 * Describes the current behavioral state of a traffic vehicle,
 * affecting how it responds to the environment and player.
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
	Pulling UMETA(DisplayName = "Pulling Over"),
	// New behaviors for enhanced AI
	LaneChanging UMETA(DisplayName = "Lane Changing"),
	Turning UMETA(DisplayName = "Turning"),
	StoppedAtLight UMETA(DisplayName = "Stopped At Light"),
	Yielding UMETA(DisplayName = "Yielding"),
	Honking UMETA(DisplayName = "Honking"),
	Swerving UMETA(DisplayName = "Swerving"),
	EmergencyResponse UMETA(DisplayName = "Emergency Response")
};

/**
 * @brief Traffic density preset enumeration
 *
 * Presets for different traffic density levels, affecting
 * spawn rates and maximum vehicle counts.
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
 * @brief Lane type enumeration
 *
 * Defines the different types of lanes that vehicles can use.
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
 * @brief Turn signal state
 */
UENUM(BlueprintType)
enum class EMGTurnSignal : uint8
{
	None UMETA(DisplayName = "None"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	Hazards UMETA(DisplayName = "Hazards")
};

/**
 * @brief Traffic reaction type for player interactions
 */
UENUM(BlueprintType)
enum class EMGTrafficReaction : uint8
{
	None UMETA(DisplayName = "None"),
	Honk UMETA(DisplayName = "Honk"),
	SwerveLeft UMETA(DisplayName = "Swerve Left"),
	SwerveRight UMETA(DisplayName = "Swerve Right"),
	BrakeHard UMETA(DisplayName = "Brake Hard"),
	Accelerate UMETA(DisplayName = "Accelerate"),
	PullOver UMETA(DisplayName = "Pull Over"),
	Panic UMETA(DisplayName = "Panic")
};

/**
 * @brief Traffic vehicle state structure
 *
 * Comprehensive state data for a traffic vehicle including
 * position, behavior, and interaction data.
 */
USTRUCT(BlueprintType)
struct FMGTrafficVehicleState
{
	GENERATED_BODY()

	/** Unique identifier for this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 VehicleID = 0;

	/** Type of vehicle (sedan, truck, emergency, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	EMGTrafficVehicleType VehicleType = EMGTrafficVehicleType::Sedan;

	/** Current behavioral state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	EMGTrafficBehavior Behavior = EMGTrafficBehavior::Normal;

	/** Reference to the spawned vehicle actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	TWeakObjectPtr<AMGTrafficVehicle> VehicleActor;

	/** Current world location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FVector Location = FVector::ZeroVector;

	/** Current rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FRotator Rotation = FRotator::ZeroRotator;

	/** Current speed in MPH */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float CurrentSpeed = 0.0f;

	/** Target speed in MPH */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float TargetSpeed = 35.0f;

	/** Current lane index on the road */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 CurrentLaneIndex = 0;

	/** Target lane for lane changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 TargetLaneIndex = 0;

	/** Current road segment ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 CurrentRoadIndex = 0;

	/** Distance from player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float DistanceFromPlayer = 0.0f;

	/** Whether the vehicle is currently visible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bIsVisible = true;

	/** Whether the vehicle has collided */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bHasCollided = false;

	/** Time this vehicle has been alive */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float TimeAlive = 0.0f;

	// === Enhanced AI state ===

	/** Current turn signal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	EMGTurnSignal TurnSignal = EMGTurnSignal::None;

	/** Whether headlights are on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	bool bHeadlightsOn = false;

	/** Whether brake lights are currently active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	bool bBrakeLightsOn = false;

	/** Whether the vehicle is currently honking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	bool bIsHonking = false;

	/** Time remaining for current honk */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	float HonkTimeRemaining = 0.0f;

	/** Whether this is an emergency vehicle with active sirens */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	bool bSirensActive = false;

	/** Time until next lane change consideration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	float LaneChangeCooldown = 0.0f;

	/** Progress through current lane change (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	float LaneChangeProgress = 0.0f;

	/** Current reaction to player or emergency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	EMGTrafficReaction CurrentReaction = EMGTrafficReaction::None;

	/** Time remaining for current reaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	float ReactionTimeRemaining = 0.0f;

	/** Distance to vehicle ahead */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	float DistanceToVehicleAhead = FLT_MAX;

	/** Whether this vehicle has triggered a near-miss event recently */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	float NearMissCooldown = 0.0f;

	/** Personality aggressiveness (0-1, affects lane change frequency, following distance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	float Aggressiveness = 0.5f;

	/** Destination intersection ID for routing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	int32 DestinationIntersectionID = -1;

	/** Planned turn at next intersection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|AI")
	int32 PlannedTurnDirection = 0; // -1 left, 0 straight, 1 right
};

/**
 * @brief Road segment for traffic routing
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
 * @brief Intersection data
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
 * @brief Spawn point for traffic
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
 * @brief Traffic settings structure
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

	// === Enhanced AI settings ===

	/** Enable emergency vehicles (ambulances, fire trucks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	bool bEnableEmergencyVehicles = true;

	/** Chance to spawn emergency vehicle per spawn cycle (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	float EmergencyVehicleChance = 0.02f;

	/** Enable lane changing behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	bool bEnableLaneChanging = true;

	/** Average time between lane change considerations (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	float LaneChangeCooldownBase = 10.0f;

	/** Enable turn signal usage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	bool bEnableTurnSignals = true;

	/** Enable vehicle honking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	bool bEnableHonking = true;

	/** Distance at which traffic reacts to player (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	float PlayerReactionDistance = 3000.0f;

	/** Speed difference threshold for reaction (mph) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	float SpeedDifferenceForReaction = 20.0f;

	/** Near miss detection radius (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	float NearMissDetectionRadius = 500.0f;

	/** Cooldown between near-miss events for same vehicle (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	float NearMissCooldown = 5.0f;

	/** Whether to use time-of-day density scaling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	bool bUseTimeOfDayDensity = true;

	/** Minimum following distance multiplier (car lengths) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	float MinFollowingDistance = 2.0f;

	/** Maximum following distance multiplier (car lengths) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Enhanced")
	float MaxFollowingDistance = 5.0f;
};

/**
 * @brief Near-miss event data from traffic
 */
USTRUCT(BlueprintType)
struct FMGTrafficNearMissEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	int32 VehicleID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	EMGTrafficVehicleType VehicleType = EMGTrafficVehicleType::Sedan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	float PlayerSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	float TrafficSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	float RelativeSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	bool bWasOncoming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NearMiss")
	FDateTime Timestamp;
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTrafficReaction, int32, VehicleID, EMGTrafficReaction, Reaction, FVector, ReactionSource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrafficNearMiss, const FMGTrafficNearMissEvent&, NearMissEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrafficHonk, int32, VehicleID, FVector, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmergencyVehicleApproaching, int32, VehicleID, float, Distance);

/**
 * @brief Traffic AI Subsystem
 *
 * Manages civilian traffic vehicles, their AI behavior,
 * spawning/despawning, traffic light control, and interactions
 * with the player and environment.
 *
 * Features:
 * - Realistic traffic behavior (lane changes, stops, turns)
 * - Reaction to racing (honking, swerving)
 * - Near-miss detection for scoring integration
 * - Traffic density based on time of day
 * - Emergency vehicles (ambulances, fire trucks)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrafficSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	//~ End USubsystem Interface

	// === Delegates ===

	/** Fired when a new traffic vehicle is spawned */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficVehicleSpawned OnTrafficVehicleSpawned;

	/** Fired when a traffic vehicle is despawned */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficVehicleDespawned OnTrafficVehicleDespawned;

	/** Fired when a traffic vehicle collides with another actor */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficCollision OnTrafficCollision;

	/** Fired when a traffic light changes state */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficLightChanged OnTrafficLightChanged;

	/** Fired when traffic density preset changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficDensityChanged OnTrafficDensityChanged;

	/** Fired when traffic panics (collision, police, etc.) */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficPanicked OnTrafficPanicked;

	/** Fired when traffic reacts to player or emergency */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficReaction OnTrafficReaction;

	/** Fired when a near-miss with traffic occurs */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficNearMiss OnTrafficNearMiss;

	/** Fired when a vehicle honks */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrafficHonk OnTrafficHonk;

	/** Fired when an emergency vehicle approaches the player */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEmergencyVehicleApproaching OnEmergencyVehicleApproaching;

	// === Settings ===

	/**
	 * @brief Apply new traffic settings
	 * @param NewSettings The settings to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetTrafficSettings(const FMGTrafficSettings& NewSettings);

	/**
	 * @brief Get current traffic settings
	 * @return Current settings
	 */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FMGTrafficSettings GetTrafficSettings() const { return Settings; }

	/**
	 * @brief Set traffic density preset
	 * @param Density The density preset to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetTrafficDensity(EMGTrafficDensityPreset Density);

	/**
	 * @brief Get current traffic density preset
	 * @return Current density preset
	 */
	UFUNCTION(BlueprintPure, Category = "Settings")
	EMGTrafficDensityPreset GetTrafficDensity() const { return Settings.DensityPreset; }

	/**
	 * @brief Enable or disable traffic system
	 * @param bEnabled Whether traffic should be enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetTrafficEnabled(bool bEnabled);

	/**
	 * @brief Check if traffic is enabled
	 * @return True if traffic is active
	 */
	UFUNCTION(BlueprintPure, Category = "Settings")
	bool IsTrafficEnabled() const { return bTrafficEnabled; }

	// === Spawning ===

	/**
	 * @brief Spawn a new traffic vehicle
	 * @param Location Spawn location
	 * @param Rotation Spawn rotation
	 * @param Type Vehicle type to spawn
	 * @return Vehicle ID or -1 on failure
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	int32 SpawnTrafficVehicle(FVector Location, FRotator Rotation, EMGTrafficVehicleType Type);

	/**
	 * @brief Spawn an emergency vehicle
	 * @param Location Spawn location
	 * @param Rotation Spawn rotation
	 * @param Type Emergency vehicle type (Ambulance, FireTruck, PoliceCar)
	 * @param bActivateSirens Whether to activate sirens immediately
	 * @return Vehicle ID or -1 on failure
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	int32 SpawnEmergencyVehicle(FVector Location, FRotator Rotation, EMGTrafficVehicleType Type, bool bActivateSirens = true);

	/**
	 * @brief Despawn a specific traffic vehicle
	 * @param VehicleID ID of vehicle to despawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void DespawnTrafficVehicle(int32 VehicleID);

	/**
	 * @brief Despawn all traffic vehicles
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void DespawnAllTraffic();

	/**
	 * @brief Register a spawn point for traffic
	 * @param SpawnPoint Spawn point data
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void RegisterSpawnPoint(const FMGTrafficSpawnPoint& SpawnPoint);

	/**
	 * @brief Clear all registered spawn points
	 */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void ClearSpawnPoints();

	// === Vehicle Queries ===

	/**
	 * @brief Get count of active traffic vehicles
	 * @return Number of active vehicles
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicles")
	int32 GetActiveVehicleCount() const;

	/**
	 * @brief Get count of active emergency vehicles
	 * @return Number of active emergency vehicles
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicles")
	int32 GetEmergencyVehicleCount() const;

	/**
	 * @brief Get all active traffic vehicles
	 * @return Array of vehicle states
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicles")
	TArray<FMGTrafficVehicleState> GetAllTrafficVehicles() const;

	/**
	 * @brief Get a specific traffic vehicle state
	 * @param VehicleID ID of vehicle to query
	 * @return Vehicle state (invalid if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicles")
	FMGTrafficVehicleState GetTrafficVehicle(int32 VehicleID) const;

	/**
	 * @brief Get all vehicles within a radius
	 * @param Center Center point
	 * @param Radius Search radius
	 * @return Array of vehicle states within radius
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicles")
	TArray<FMGTrafficVehicleState> GetVehiclesInRadius(FVector Center, float Radius) const;

	/**
	 * @brief Get the nearest traffic vehicle to a location
	 * @param Location Reference location
	 * @return Nearest vehicle state
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicles")
	FMGTrafficVehicleState GetNearestVehicle(FVector Location) const;

	/**
	 * @brief Check if a vehicle is an emergency vehicle
	 * @param VehicleType Type to check
	 * @return True if emergency vehicle type
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicles")
	static bool IsEmergencyVehicle(EMGTrafficVehicleType VehicleType);

	// === Vehicle Control ===

	/**
	 * @brief Set the behavior of a specific vehicle
	 * @param VehicleID Target vehicle
	 * @param Behavior New behavior state
	 */
	UFUNCTION(BlueprintCallable, Category = "Control")
	void SetVehicleBehavior(int32 VehicleID, EMGTrafficBehavior Behavior);

	/**
	 * @brief Set the target speed of a specific vehicle
	 * @param VehicleID Target vehicle
	 * @param Speed New target speed (MPH)
	 */
	UFUNCTION(BlueprintCallable, Category = "Control")
	void SetVehicleTargetSpeed(int32 VehicleID, float Speed);

	/**
	 * @brief Force a vehicle to change lanes
	 * @param VehicleID Target vehicle
	 * @param bMoveRight True for right, false for left
	 */
	UFUNCTION(BlueprintCallable, Category = "Control")
	void ForceVehicleLaneChange(int32 VehicleID, bool bMoveRight);

	/**
	 * @brief Cause vehicles in radius to panic
	 * @param Center Panic epicenter
	 * @param Radius Effect radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Control")
	void PanicVehiclesInRadius(FVector Center, float Radius);

	/**
	 * @brief Stop a specific vehicle
	 * @param VehicleID Target vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Control")
	void StopVehicle(int32 VehicleID);

	/**
	 * @brief Make a vehicle honk
	 * @param VehicleID Target vehicle
	 * @param Duration Honk duration in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Control")
	void MakeVehicleHonk(int32 VehicleID, float Duration = 0.5f);

	/**
	 * @brief Activate or deactivate sirens on emergency vehicle
	 * @param VehicleID Target vehicle
	 * @param bActivate Whether to activate sirens
	 */
	UFUNCTION(BlueprintCallable, Category = "Control")
	void SetEmergencySirens(int32 VehicleID, bool bActivate);

	// === Roads ===

	/**
	 * @brief Register a road segment
	 * @param Road Road data
	 */
	UFUNCTION(BlueprintCallable, Category = "Roads")
	void RegisterRoad(const FMGRoadSegment& Road);

	/**
	 * @brief Register an intersection
	 * @param Intersection Intersection data
	 */
	UFUNCTION(BlueprintCallable, Category = "Roads")
	void RegisterIntersection(const FMGIntersection& Intersection);

	/**
	 * @brief Get road data
	 * @param RoadID Road to query
	 * @return Road data
	 */
	UFUNCTION(BlueprintPure, Category = "Roads")
	FMGRoadSegment GetRoad(int32 RoadID) const;

	/**
	 * @brief Get intersection data
	 * @param IntersectionID Intersection to query
	 * @return Intersection data
	 */
	UFUNCTION(BlueprintPure, Category = "Roads")
	FMGIntersection GetIntersection(int32 IntersectionID) const;

	/**
	 * @brief Get all registered roads
	 * @return Array of road segments
	 */
	UFUNCTION(BlueprintPure, Category = "Roads")
	TArray<FMGRoadSegment> GetAllRoads() const;

	// === Traffic Lights ===

	/**
	 * @brief Manually set traffic light state
	 * @param IntersectionID Target intersection
	 * @param GreenRoadIndex Index of road to make green
	 */
	UFUNCTION(BlueprintCallable, Category = "Lights")
	void SetTrafficLightState(int32 IntersectionID, int32 GreenRoadIndex);

	/**
	 * @brief Force all lights green (for testing/events)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lights")
	void ForceAllLightsGreen();

	/**
	 * @brief Resume normal light cycling
	 */
	UFUNCTION(BlueprintCallable, Category = "Lights")
	void ResumeNormalLightCycle();

	/**
	 * @brief Check if light is green for a road
	 * @param IntersectionID Target intersection
	 * @param RoadID Road to check
	 * @return True if green
	 */
	UFUNCTION(BlueprintPure, Category = "Lights")
	bool IsLightGreenForRoad(int32 IntersectionID, int32 RoadID) const;

	// === Player Interaction ===

	/**
	 * @brief Update player position for traffic AI
	 * @param Position Player world position
	 * @param Speed Player speed (MPH)
	 */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void UpdatePlayerPosition(FVector Position, float Speed);

	/**
	 * @brief Update player velocity for near-miss detection
	 * @param Position Player world position
	 * @param Velocity Player velocity vector
	 * @param Speed Player speed (MPH)
	 * @param bIsDrifting Whether player is currently drifting
	 */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void UpdatePlayerState(FVector Position, FVector Velocity, float Speed, bool bIsDrifting);

	/**
	 * @brief Notify system of player collision with traffic
	 * @param VehicleID Vehicle that was hit
	 */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void NotifyPlayerCollision(int32 VehicleID);

	// === Time of Day Integration ===

	/**
	 * @brief Get effective density multiplier considering time of day
	 * @return Density multiplier (0.0 - 2.0+)
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	float GetEffectiveDensityMultiplier() const;

	/**
	 * @brief Force refresh of time-of-day settings
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void RefreshTimeOfDaySettings();

	// === Near Miss Integration ===

	/**
	 * @brief Get recent near-miss events
	 * @param MaxCount Maximum events to return
	 * @return Array of recent near-miss events
	 */
	UFUNCTION(BlueprintPure, Category = "NearMiss")
	TArray<FMGTrafficNearMissEvent> GetRecentNearMisses(int32 MaxCount = 10) const;

	/**
	 * @brief Get total near-miss count for session
	 * @return Total near-misses
	 */
	UFUNCTION(BlueprintPure, Category = "NearMiss")
	int32 GetNearMissCount() const { return NearMissCount; }

	// === Stats ===

	/**
	 * @brief Get total vehicles spawned this session
	 * @return Total spawned count
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetTotalVehiclesSpawned() const { return TotalVehiclesSpawned; }

	/**
	 * @brief Get total collision count this session
	 * @return Total collisions
	 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetTotalCollisions() const { return TotalCollisions; }

protected:
	/** Main traffic update tick */
	void UpdateTraffic(float DeltaTime);

	/** Update individual vehicle AI */
	void UpdateVehicleAI(FMGTrafficVehicleState& Vehicle, float DeltaTime);

	/** Process realistic driving behavior */
	void ProcessDrivingBehavior(FMGTrafficVehicleState& Vehicle, float DeltaTime);

	/** Process lane change logic */
	void ProcessLaneChange(FMGTrafficVehicleState& Vehicle, float DeltaTime);

	/** Process intersection behavior */
	void ProcessIntersectionBehavior(FMGTrafficVehicleState& Vehicle, float DeltaTime);

	/** Process reaction to player */
	void ProcessPlayerReaction(FMGTrafficVehicleState& Vehicle, float DeltaTime);

	/** Process reaction to emergency vehicles */
	void ProcessEmergencyReaction(FMGTrafficVehicleState& Vehicle, float DeltaTime);

	/** Process near-miss detection */
	void ProcessNearMissDetection(FMGTrafficVehicleState& Vehicle, float DeltaTime);

	/** Update traffic lights */
	void UpdateTrafficLights(float DeltaTime);

	/** Spawn traffic as needed based on density */
	void SpawnTrafficIfNeeded();

	/** Despawn vehicles too far from player */
	void DespawnDistantVehicles();

	/** Get lane position on road */
	FVector GetLanePosition(int32 RoadID, int32 LaneIndex, float Distance) const;

	/** Select random vehicle type based on weights */
	EMGTrafficVehicleType SelectRandomVehicleType() const;

	/** Get density multiplier from preset */
	float GetDensityMultiplier() const;

	/** Check if lane change is safe */
	bool CanChangeLane(const FMGTrafficVehicleState& Vehicle, bool bMoveRight) const;

	/** Get vehicle ahead in same lane */
	FMGTrafficVehicleState* GetVehicleAhead(const FMGTrafficVehicleState& Vehicle);

	/** Calculate safe following distance */
	float CalculateFollowingDistance(const FMGTrafficVehicleState& Vehicle) const;

	/** Register near-miss with scoring system */
	void RegisterNearMissWithScoringSystem(const FMGTrafficNearMissEvent& Event);

	/** Determine appropriate reaction to player */
	EMGTrafficReaction DeterminePlayerReaction(const FMGTrafficVehicleState& Vehicle) const;

	/** Apply reaction behavior to vehicle */
	void ApplyReaction(FMGTrafficVehicleState& Vehicle, EMGTrafficReaction Reaction);

private:
	/** Update timer handle */
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
	FVector PlayerVelocity = FVector::ZeroVector;

	UPROPERTY()
	float PlayerSpeed = 0.0f;

	UPROPERTY()
	bool bPlayerIsDrifting = false;

	// Spawn timing
	UPROPERTY()
	float TimeSinceLastSpawn = 0.0f;

	UPROPERTY()
	float NextSpawnInterval = 2.0f;

	// Near miss tracking
	UPROPERTY()
	TArray<FMGTrafficNearMissEvent> RecentNearMisses;

	UPROPERTY()
	int32 NearMissCount = 0;

	static constexpr int32 MaxRecentNearMisses = 50;

	// Stats
	UPROPERTY()
	int32 TotalVehiclesSpawned = 0;

	UPROPERTY()
	int32 TotalCollisions = 0;

	// Cached subsystem references
	UPROPERTY()
	TWeakObjectPtr<UMGTimeOfDaySubsystem> TimeOfDaySubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGNearMissSubsystem> NearMissSubsystem;
};

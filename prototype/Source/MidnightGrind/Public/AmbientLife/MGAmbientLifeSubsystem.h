// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGAmbientLifeSubsystem.h
 * @brief Ambient Life and World Population Subsystem
 *
 * This subsystem manages all ambient life in Midnight Grind's open world,
 * including traffic vehicles, pedestrians, and crowd behaviors.
 *
 * ## Overview
 * The Ambient Life Subsystem creates a living, breathing world by:
 * - Spawning and managing traffic vehicles
 * - Controlling pedestrian populations and behaviors
 * - Creating crowd gatherings at events and races
 * - Managing parked vehicles for atmosphere
 * - Reacting dynamically to player actions
 *
 * ## Architecture
 * This is a Game Instance Subsystem that coordinates world population
 * across all gameplay modes. It integrates with:
 * - MGTimeOfDaySubsystem (density varies by time)
 * - MGWeatherSubsystem (population affected by weather)
 * - MGWorldEventsSubsystem (crowds at events)
 * - Vehicle physics (traffic AI collision avoidance)
 *
 * ## Key Concepts
 * - **Traffic**: AI-controlled vehicles following road networks
 * - **Pedestrians**: NPCs walking on sidewalks and reacting to player
 * - **Crowd Zones**: Areas where pedestrians gather (races, meets)
 * - **Density**: Population levels adjusted by time/weather/location
 *
 * ## Performance Strategy
 * - Vehicles/pedestrians spawn within a radius around player
 * - Entities despawn when beyond a larger despawn radius
 * - LOD systems reduce detail for distant entities
 * - Population caps prevent excessive entity counts
 *
 * ## Usage Example
 * @code
 * // Get the ambient life subsystem
 * UMGAmbientLifeSubsystem* AmbientLife = GetGameInstance()->GetSubsystem<UMGAmbientLifeSubsystem>();
 *
 * // Reduce traffic for a race
 * AmbientLife->ClearTrafficInRadius(RaceStartLocation, 500.0f);
 *
 * // Setup spectators along the race route
 * AmbientLife->SetupRaceSpectators(SpectatorLocations);
 * @endcode
 *
 * @see FMGTrafficSettings for traffic configuration
 * @see FMGPedestrianSettings for pedestrian configuration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAmbientLifeSubsystem.generated.h"

// ============================================================================
// TRAFFIC DENSITY ENUMERATION
// ============================================================================

/**
 * @brief Traffic density levels affecting vehicle spawn rates
 *
 * Different density levels create varied driving experiences,
 * from empty streets to rush-hour congestion.
 */
UENUM(BlueprintType)
enum class EMGTrafficDensity : uint8
{
	None,    ///< No traffic vehicles (empty streets, race mode)
	Light,   ///< Few vehicles, easy to navigate (late night, rural)
	Medium,  ///< Normal traffic flow (default daytime)
	Heavy,   ///< Congested traffic, requires careful navigation
	Rush     ///< Rush hour density, slow-moving traffic jams
};

// ============================================================================
// PEDESTRIAN BEHAVIOR ENUMERATION
// ============================================================================

/**
 * @brief Pedestrian behavior states
 *
 * Pedestrians dynamically switch between behaviors based on
 * player actions and world events.
 */
UENUM(BlueprintType)
enum class EMGPedestrianBehavior : uint8
{
	Walking,     ///< Normal walking along sidewalks
	Standing,    ///< Stationary, waiting or loitering
	Spectating,  ///< Watching a race or event
	Fleeing,     ///< Running away from danger (crashes, chases)
	Cheering,    ///< Excited reactions to player racing
	Recording    ///< Using phone to record player's car/driving
};

// ============================================================================
// TRAFFIC SETTINGS STRUCTURE
// ============================================================================

/**
 * @brief Configuration for traffic vehicle spawning and behavior
 *
 * Controls how many vehicles spawn, their behavior, and
 * what types of vehicles appear.
 */
USTRUCT(BlueprintType)
struct FMGTrafficSettings
{
	GENERATED_BODY()

	/// Current traffic density level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrafficDensity Density = EMGTrafficDensity::Medium;

	/// Multiplier applied to base density (0.0 - 2.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DensityMultiplier = 1.0f;

	/// Radius around player where vehicles spawn (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRadius = 500.0f;

	/// Radius at which vehicles despawn (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DespawnRadius = 800.0f;

	/// Maximum concurrent traffic vehicles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxVehicles = 50;

	/// Whether to include trucks/large vehicles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowTrucks = true;

	/// Whether to include motorcycles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowMotorcycles = true;

	/// AI driving aggression (0 = passive, 1 = very aggressive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionLevel = 0.3f;
};

// ============================================================================
// PEDESTRIAN SETTINGS STRUCTURE
// ============================================================================

/**
 * @brief Configuration for pedestrian spawning and behavior
 *
 * Controls pedestrian density, reactions, and interactive behaviors.
 */
USTRUCT(BlueprintType)
struct FMGPedestrianSettings
{
	GENERATED_BODY()

	/// Multiplier for base pedestrian count (0.0 - 2.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DensityMultiplier = 1.0f;

	/// Maximum concurrent pedestrians
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPedestrians = 100;

	/// Radius around player where pedestrians spawn (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRadius = 300.0f;

	/// Whether pedestrians react to racing/stunts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReactToRacing = true;

	/// Whether pedestrians can pull out phones to record
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRecordPlayer = true;

	/// Probability of cheering reaction (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CheerChance = 0.3f;
};

// ============================================================================
// AMBIENT VEHICLE STRUCTURE
// ============================================================================

/**
 * @brief Data for an ambient (traffic or parked) vehicle instance
 *
 * Tracks all information needed to manage a single ambient vehicle.
 */
USTRUCT(BlueprintType)
struct FMGAmbientVehicle
{
	GENERATED_BODY()

	/// Unique identifier for this vehicle instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleID;

	/// Vehicle type/model identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleType;

	/// Current world transform (location, rotation, scale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;

	/// True if this vehicle is parked (not moving)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsParked = false;

	/// True if this is an active traffic vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTraffic = true;

	/// Current driving speed (m/s), 0 if parked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	/// Vehicle paint color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PaintColor = FLinearColor::White;
};

// ============================================================================
// AMBIENT PEDESTRIAN STRUCTURE
// ============================================================================

/**
 * @brief Data for a pedestrian instance
 *
 * Tracks pedestrian state, location, and current behavior.
 */
USTRUCT(BlueprintType)
struct FMGAmbientPedestrian
{
	GENERATED_BODY()

	/// Unique identifier for this pedestrian
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PedestrianID;

	/// Current world position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Current behavior state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPedestrianBehavior Behavior = EMGPedestrianBehavior::Walking;

	/// True if pedestrian is holding a phone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPhone = false;

	/// True if actively recording video of player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRecording = false;
};

// ============================================================================
// CROWD ZONE STRUCTURE
// ============================================================================

/**
 * @brief Defines an area where pedestrians gather as a crowd
 *
 * Crowd zones create clusters of spectators at events, races,
 * and car meets. They attract and hold pedestrians within their radius.
 */
USTRUCT(BlueprintType)
struct FMGCrowdZone
{
	GENERATED_BODY()

	/// Unique identifier for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	/// Center point of the crowd area
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Radius of the crowd zone (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 50.0f;

	/// Desired number of pedestrians in this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetCrowdSize = 20;

	/// Current number of pedestrians in zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCrowdSize = 0;

	/// True if this is a race spectator area
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRaceSpectators = false;

	/// True if this is a car meet gathering
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCarMeet = false;
};

// ============================================================================
// WORLD POPULATION STRUCTURE
// ============================================================================

/**
 * @brief Snapshot of current world population statistics
 *
 * Provides real-time counts of all ambient entities for
 * debugging, performance monitoring, and UI display.
 */
USTRUCT(BlueprintType)
struct FMGWorldPopulation
{
	GENERATED_BODY()

	/// Number of active traffic vehicles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActiveTrafficVehicles = 0;

	/// Number of active pedestrians
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActivePedestrians = 0;

	/// Number of parked vehicles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParkedVehicles = 0;

	/// Number of active crowd zones
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpectatorCrowds = 0;

	/// Average pedestrians per area (density metric)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AveragePedestrianDensity = 0.0f;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/// Broadcast when a pedestrian changes behavior (e.g., starts recording)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPedestrianReaction, const FMGAmbientPedestrian&, Pedestrian, EMGPedestrianBehavior, NewBehavior);

/// Broadcast when a crowd zone reaches target capacity
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrowdGathered, const FMGCrowdZone&, Zone);

/// Broadcast when traffic density level changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTrafficDensityChanged, EMGTrafficDensity, OldDensity, EMGTrafficDensity, NewDensity);

// ============================================================================
// AMBIENT LIFE SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem managing world population and ambient life
 *
 * This Game Instance Subsystem creates and manages all NPCs and
 * ambient vehicles that populate Midnight Grind's open world.
 *
 * ## Features
 * - Dynamic traffic vehicle spawning and AI
 * - Pedestrian crowds with reactive behaviors
 * - Crowd zones for events and spectators
 * - Parked vehicle placement for atmosphere
 * - Time-of-day and weather population adjustments
 * - Race mode integration (traffic clearing, spectators)
 *
 * ## Spawning Behavior
 * - Entities spawn within configured radius around player
 * - Population is capped to maintain performance
 * - Density varies by time of day and location type
 * - Weather affects outdoor pedestrian counts
 *
 * ## Performance Considerations
 * - Entities beyond despawn radius are removed
 * - LOD systems reduce update frequency for distant entities
 * - Crowd zones batch-update their populations
 * - Traffic AI uses simplified pathfinding
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAmbientLifeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// TRAFFIC CONTROL
	// ==========================================
	/** @name Traffic Control
	 *  Functions for managing traffic vehicles.
	 */
	///@{

	/**
	 * @brief Applies new traffic settings
	 * @param Settings Configuration for traffic behavior
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Traffic")
	void SetTrafficSettings(const FMGTrafficSettings& Settings);

	/// Returns current traffic settings
	UFUNCTION(BlueprintPure, Category = "AmbientLife|Traffic")
	FMGTrafficSettings GetTrafficSettings() const { return TrafficSettings; }

	/**
	 * @brief Changes traffic density level
	 * @param Density New density level
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Traffic")
	void SetTrafficDensity(EMGTrafficDensity Density);

	/**
	 * @brief Removes traffic vehicles in an area (for races)
	 * @param Location Center of clear zone
	 * @param Radius Clear zone radius (meters)
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Traffic")
	void ClearTrafficInRadius(FVector Location, float Radius);

	/**
	 * @brief Pauses/resumes all traffic movement
	 * @param bPause True to freeze traffic, false to resume
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Traffic")
	void PauseTraffic(bool bPause);

	/**
	 * @brief Finds traffic vehicles in an area
	 * @param Location Search center
	 * @param Radius Search radius (meters)
	 * @return Array of vehicles within range
	 */
	UFUNCTION(BlueprintPure, Category = "AmbientLife|Traffic")
	TArray<FMGAmbientVehicle> GetNearbyTraffic(FVector Location, float Radius) const;

	///@}

	// ==========================================
	// PEDESTRIAN CONTROL
	// ==========================================
	/** @name Pedestrian Control
	 *  Functions for managing pedestrian NPCs.
	 */
	///@{

	/**
	 * @brief Applies new pedestrian settings
	 * @param Settings Configuration for pedestrian behavior
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Pedestrians")
	void SetPedestrianSettings(const FMGPedestrianSettings& Settings);

	/// Returns current pedestrian settings
	UFUNCTION(BlueprintPure, Category = "AmbientLife|Pedestrians")
	FMGPedestrianSettings GetPedestrianSettings() const { return PedestrianSettings; }

	/**
	 * @brief Triggers a behavior change in pedestrians near a location
	 * @param Location Center of effect
	 * @param Radius Effect radius (meters)
	 * @param Behavior The behavior to trigger (Fleeing, Cheering, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Pedestrians")
	void TriggerPedestrianReaction(FVector Location, float Radius, EMGPedestrianBehavior Behavior);

	/**
	 * @brief Removes pedestrians from an area
	 * @param Location Center of clear zone
	 * @param Radius Clear zone radius (meters)
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Pedestrians")
	void ClearPedestriansInRadius(FVector Location, float Radius);

	/**
	 * @brief Finds pedestrians in an area
	 * @param Location Search center
	 * @param Radius Search radius (meters)
	 * @return Array of pedestrians within range
	 */
	UFUNCTION(BlueprintPure, Category = "AmbientLife|Pedestrians")
	TArray<FMGAmbientPedestrian> GetNearbyPedestrians(FVector Location, float Radius) const;

	///@}

	// ==========================================
	// CROWD ZONES
	// ==========================================
	/** @name Crowd Zones
	 *  Functions for managing crowd gathering areas.
	 */
	///@{

	/**
	 * @brief Creates a new crowd zone
	 * @param Zone Configuration for the crowd area
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Crowds")
	void CreateCrowdZone(const FMGCrowdZone& Zone);

	/**
	 * @brief Removes a crowd zone and disperses its pedestrians
	 * @param ZoneID ID of zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Crowds")
	void RemoveCrowdZone(FName ZoneID);

	/**
	 * @brief Creates spectator crowds along a race route
	 * @param SpectatorLocations Array of spectator positions
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Crowds")
	void SetupRaceSpectators(const TArray<FVector>& SpectatorLocations);

	/// Returns all active crowd zones
	UFUNCTION(BlueprintPure, Category = "AmbientLife|Crowds")
	TArray<FMGCrowdZone> GetActiveCrowdZones() const { return CrowdZones; }

	///@}

	// ==========================================
	// PARKED VEHICLES
	// ==========================================
	/** @name Parked Vehicles
	 *  Functions for managing parked/stationary vehicles.
	 */
	///@{

	/**
	 * @brief Spawns parked cars in an area for atmosphere
	 * @param Location Center of spawn area
	 * @param Count Number of cars to spawn
	 * @param Radius Spawn area radius (meters)
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Parked")
	void SpawnParkedCars(FVector Location, int32 Count, float Radius);

	/**
	 * @brief Sets up show cars for a car meet event
	 * @param Location Meet location
	 * @param VehicleTypes Specific vehicle types to spawn
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Parked")
	void SetupCarMeetVehicles(FVector Location, const TArray<FName>& VehicleTypes);

	///@}

	// ==========================================
	// WORLD POPULATION
	// ==========================================
	/** @name World Population
	 *  Functions for querying overall world state.
	 */
	///@{

	/**
	 * @brief Returns current population statistics
	 * @return Snapshot of all entity counts
	 */
	UFUNCTION(BlueprintPure, Category = "AmbientLife")
	FMGWorldPopulation GetWorldPopulation() const;

	/**
	 * @brief Updates density multipliers based on time of day
	 * @param TrafficMult Traffic density multiplier
	 * @param PedestrianMult Pedestrian density multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife")
	void SetTimeOfDayMultipliers(float TrafficMult, float PedestrianMult);

	///@}

	// ==========================================
	// RACE MODE
	// ==========================================
	/** @name Race Mode
	 *  Functions for race-specific ambient life behavior.
	 */
	///@{

	/**
	 * @brief Prepares ambient life for a race (clears route, adds spectators)
	 * @param RaceRoute Array of points defining the race path
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Race")
	void OnRaceStarting(const TArray<FVector>& RaceRoute);

	/**
	 * @brief Restores normal ambient life after a race
	 */
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Race")
	void OnRaceEnded();

	/// Returns true if a race is currently active
	UFUNCTION(BlueprintPure, Category = "AmbientLife|Race")
	bool IsRaceActive() const { return bRaceActive; }

	///@}

	// ==========================================
	// EVENTS
	// ==========================================
	/** @name Event Delegates
	 *  Subscribe to respond to ambient life changes.
	 */
	///@{

	/// Fires when a pedestrian reacts to player
	UPROPERTY(BlueprintAssignable, Category = "AmbientLife|Events")
	FMGOnPedestrianReaction OnPedestrianReaction;

	/// Fires when a crowd zone fills up
	UPROPERTY(BlueprintAssignable, Category = "AmbientLife|Events")
	FMGOnCrowdGathered OnCrowdGathered;

	/// Fires when traffic density changes
	UPROPERTY(BlueprintAssignable, Category = "AmbientLife|Events")
	FMGOnTrafficDensityChanged OnTrafficDensityChanged;

	///@}

protected:
	// ==========================================
	// INTERNAL UPDATE METHODS
	// ==========================================
	/** @name Internal Methods
	 *  Private implementation for ambient life management.
	 */
	///@{

	/**
	 * @brief Main update loop for all ambient entities
	 * @param DeltaTime Time since last update
	 */
	void UpdateAmbientLife(float DeltaTime);

	/**
	 * @brief Updates crowd zone populations
	 * @param DeltaTime Time since last update
	 */
	void UpdateCrowds(float DeltaTime);

	/**
	 * @brief Spawns a new traffic vehicle if under cap
	 */
	void SpawnTrafficVehicle();

	/**
	 * @brief Spawns a new pedestrian if under cap
	 */
	void SpawnPedestrian();

	/**
	 * @brief Calculates target traffic count based on settings
	 * @return Desired number of traffic vehicles
	 */
	int32 GetTargetTrafficCount() const;

	/**
	 * @brief Calculates target pedestrian count based on settings
	 * @return Desired number of pedestrians
	 */
	int32 GetTargetPedestrianCount() const;

	///@}

private:
	// ==========================================
	// STATE VARIABLES
	// ==========================================

	/// All active traffic vehicles
	UPROPERTY()
	TArray<FMGAmbientVehicle> TrafficVehicles;

	/// All parked vehicles
	UPROPERTY()
	TArray<FMGAmbientVehicle> ParkedVehicles;

	/// All active pedestrians
	UPROPERTY()
	TArray<FMGAmbientPedestrian> Pedestrians;

	/// All active crowd zones
	UPROPERTY()
	TArray<FMGCrowdZone> CrowdZones;

	/// Current traffic configuration
	FMGTrafficSettings TrafficSettings;

	/// Current pedestrian configuration
	FMGPedestrianSettings PedestrianSettings;

	/// Timer for periodic updates
	FTimerHandle UpdateTimerHandle;

	/// Time-of-day traffic density modifier
	float TimeOfDayTrafficMultiplier = 1.0f;

	/// Time-of-day pedestrian density modifier
	float TimeOfDayPedestrianMultiplier = 1.0f;

	/// Whether traffic is currently paused
	bool bTrafficPaused = false;

	/// Whether a race is currently active
	bool bRaceActive = false;

	/// Cached race route for spectator placement
	TArray<FVector> CurrentRaceRoute;
};

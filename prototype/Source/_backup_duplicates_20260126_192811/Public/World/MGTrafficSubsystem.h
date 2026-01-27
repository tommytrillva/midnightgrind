// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGTrafficSubsystem.generated.h"

class AMGTrafficVehicle;
class USplineComponent;

/**
 * Traffic density level
 */
UENUM(BlueprintType)
enum class EMGTrafficDensity : uint8
{
	/** No traffic */
	None,
	/** Very light traffic */
	VeryLight,
	/** Light traffic */
	Light,
	/** Moderate traffic */
	Moderate,
	/** Heavy traffic */
	Heavy
};

/**
 * Traffic vehicle type
 */
UENUM(BlueprintType)
enum class EMGTrafficVehicleType : uint8
{
	/** Standard sedan */
	Sedan,
	/** Compact car */
	Compact,
	/** SUV/Crossover */
	SUV,
	/** Pickup truck */
	Truck,
	/** Sports car */
	Sports,
	/** Van */
	Van,
	/** Bus */
	Bus,
	/** Semi/18-wheeler */
	Semi
};

/**
 * Traffic lane data
 */
USTRUCT(BlueprintType)
struct FMGTrafficLane
{
	GENERATED_BODY()

	/** Lane ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LaneID;

	/** Spline component for lane path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<USplineComponent> LaneSpline;

	/** Speed limit (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimit = 60.0f;

	/** Is one-way */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOneWay = true;

	/** Lane direction (for two-way) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Direction = 1;

	/** Allowed vehicle types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGTrafficVehicleType> AllowedTypes;
};

/**
 * Traffic spawn point
 */
USTRUCT(BlueprintType)
struct FMGTrafficSpawnPoint
{
	GENERATED_BODY()

	/** Spawn location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** Initial direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Associated lane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LaneID;

	/** District this spawn belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DistrictID;

	/** Is entry point (spawn) or exit point (despawn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEntryPoint = true;
};

/**
 * Traffic configuration per time period
 */
USTRUCT(BlueprintType)
struct FMGTrafficConfig
{
	GENERATED_BODY()

	/** Base density */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrafficDensity BaseDensity = EMGTrafficDensity::Moderate;

	/** Vehicle type distribution (must sum to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGTrafficVehicleType, float> TypeDistribution;

	/** Speed variance (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedVariance = 0.15f;

	/** Aggression level (0-1, how likely to avoid player) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionLevel = 0.3f;

	FMGTrafficConfig()
	{
		TypeDistribution.Add(EMGTrafficVehicleType::Sedan, 0.35f);
		TypeDistribution.Add(EMGTrafficVehicleType::Compact, 0.2f);
		TypeDistribution.Add(EMGTrafficVehicleType::SUV, 0.2f);
		TypeDistribution.Add(EMGTrafficVehicleType::Truck, 0.1f);
		TypeDistribution.Add(EMGTrafficVehicleType::Sports, 0.05f);
		TypeDistribution.Add(EMGTrafficVehicleType::Van, 0.05f);
		TypeDistribution.Add(EMGTrafficVehicleType::Bus, 0.03f);
		TypeDistribution.Add(EMGTrafficVehicleType::Semi, 0.02f);
	}
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrafficCollision, AMGTrafficVehicle*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNearMiss, AMGTrafficVehicle*, Vehicle, float, Distance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrafficDensityChanged, EMGTrafficDensity, NewDensity);

/**
 * Traffic Manager Subsystem
 * Manages AI traffic vehicles in the open world
 *
 * Features:
 * - Time-based traffic density (per PRD Section 5.2)
 * - District-specific traffic patterns
 * - Collision and near-miss detection
 * - Performance-optimized spawning/despawning
 * - Lane-based navigation
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTrafficSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// TRAFFIC CONTROL
	// ==========================================

	/**
	 * Set traffic density
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetTrafficDensity(EMGTrafficDensity Density);

	/**
	 * Get current traffic density
	 */
	UFUNCTION(BlueprintPure, Category = "Traffic")
	EMGTrafficDensity GetTrafficDensity() const { return CurrentDensity; }

	/**
	 * Set traffic enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic")
	void SetTrafficEnabled(bool bEnabled);

	/**
	 * Is traffic enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Traffic")
	bool IsTrafficEnabled() const { return bTrafficEnabled; }

	/**
	 * Get active traffic count
	 */
	UFUNCTION(BlueprintPure, Category = "Traffic")
	int32 GetActiveTrafficCount() const { return ActiveTrafficVehicles.Num(); }

	/**
	 * Get maximum traffic count
	 */
	UFUNCTION(BlueprintPure, Category = "Traffic")
	int32 GetMaxTrafficCount() const { return MaxTrafficVehicles; }

	// ==========================================
	// DISTRICT MANAGEMENT
	// ==========================================

	/**
	 * Set district traffic config
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|District")
	void SetDistrictConfig(FName DistrictID, const FMGTrafficConfig& Config);

	/**
	 * Get district traffic config
	 */
	UFUNCTION(BlueprintPure, Category = "Traffic|District")
	FMGTrafficConfig GetDistrictConfig(FName DistrictID) const;

	/**
	 * Clear traffic in district
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|District")
	void ClearDistrictTraffic(FName DistrictID);

	// ==========================================
	// TIME-BASED CONTROL
	// ==========================================

	/**
	 * Update traffic for time of day (per PRD)
	 * Dusk (7-9PM): Moderate
	 * Night (9PM-4AM): Light-Moderate
	 * Late Night (12-3AM): Light
	 * Dawn (4-6AM): Very Light
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|Time")
	void UpdateForTimeOfDay(float GameHour);

	// ==========================================
	// RACE INTEGRATION
	// ==========================================

	/**
	 * Set race active (modifies traffic behavior)
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|Race")
	void SetRaceActive(bool bActive, TArray<AActor*> RaceParticipants);

	/**
	 * Clear traffic from race path
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|Race")
	void ClearRacePath(const TArray<FVector>& PathPoints, float ClearRadius);

	// ==========================================
	// SPAWNING
	// ==========================================

	/**
	 * Register traffic lane
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|Setup")
	void RegisterLane(const FMGTrafficLane& Lane);

	/**
	 * Register spawn point
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|Setup")
	void RegisterSpawnPoint(const FMGTrafficSpawnPoint& SpawnPoint);

	/**
	 * Force spawn traffic vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|Spawn")
	AMGTrafficVehicle* SpawnTrafficVehicle(const FMGTrafficSpawnPoint& SpawnPoint, EMGTrafficVehicleType Type);

	/**
	 * Despawn traffic vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Traffic|Spawn")
	void DespawnTrafficVehicle(AMGTrafficVehicle* Vehicle);

	// ==========================================
	// QUERIES
	// ==========================================

	/**
	 * Get nearest traffic vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Traffic|Query")
	AMGTrafficVehicle* GetNearestTrafficVehicle(FVector Location, float MaxDistance) const;

	/**
	 * Get traffic vehicles in radius
	 */
	UFUNCTION(BlueprintPure, Category = "Traffic|Query")
	TArray<AMGTrafficVehicle*> GetTrafficInRadius(FVector Location, float Radius) const;

	/**
	 * Is location in traffic lane
	 */
	UFUNCTION(BlueprintPure, Category = "Traffic|Query")
	bool IsLocationInLane(FVector Location, FName& OutLaneID) const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Traffic vehicle collision with player */
	UPROPERTY(BlueprintAssignable, Category = "Traffic|Events")
	FOnTrafficCollision OnTrafficCollision;

	/** Near miss with traffic */
	UPROPERTY(BlueprintAssignable, Category = "Traffic|Events")
	FOnNearMiss OnNearMiss;

	/** Traffic density changed */
	UPROPERTY(BlueprintAssignable, Category = "Traffic|Events")
	FOnTrafficDensityChanged OnTrafficDensityChanged;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update traffic spawning */
	void UpdateSpawning(float DeltaTime);

	/** Update traffic culling */
	void UpdateCulling();

	/** Get spawn count for density */
	int32 GetTargetCountForDensity(EMGTrafficDensity Density) const;

	/** Select random vehicle type based on config */
	EMGTrafficVehicleType SelectRandomVehicleType(const FMGTrafficConfig& Config) const;

	/** Find valid spawn point */
	bool FindValidSpawnPoint(FName DistrictID, FMGTrafficSpawnPoint& OutSpawnPoint) const;

	/** Check near miss for player */
	void CheckNearMisses();

	/** Tick function */
	void OnTick();

private:
	/** Active traffic vehicles */
	UPROPERTY()
	TArray<TObjectPtr<AMGTrafficVehicle>> ActiveTrafficVehicles;

	/** Registered lanes */
	TArray<FMGTrafficLane> RegisteredLanes;

	/** Registered spawn points */
	TArray<FMGTrafficSpawnPoint> RegisteredSpawnPoints;

	/** District configurations */
	TMap<FName, FMGTrafficConfig> DistrictConfigs;

	/** Current density */
	EMGTrafficDensity CurrentDensity = EMGTrafficDensity::Moderate;

	/** Is traffic enabled */
	bool bTrafficEnabled = true;

	/** Is race active */
	bool bRaceActive = false;

	/** Race participants (to avoid) */
	TArray<TWeakObjectPtr<AActor>> RaceParticipants;

	/** Maximum traffic vehicles */
	int32 MaxTrafficVehicles = 50;

	/** Spawn interval timer */
	float SpawnTimer = 0.0f;

	/** Spawn interval */
	float SpawnInterval = 0.5f;

	/** Cull distance */
	float CullDistance = 15000.0f;

	/** Near miss threshold */
	float NearMissThreshold = 200.0f;

	/** Timer handle */
	FTimerHandle TickTimer;

	/** Default traffic config */
	FMGTrafficConfig DefaultConfig;
};

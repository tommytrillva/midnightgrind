// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGPoliceSubsystem.generated.h"

class AMGVehiclePawn;
class AMGPoliceVehicle;

/**
 * Heat level as defined in PRD Section 4.4
 */
UENUM(BlueprintType)
enum class EMGHeatLevel : uint8
{
	/** No police attention */
	Clean = 0,
	/** Occasional patrol passes */
	Noticed = 1,
	/** Active patrol searching */
	Wanted = 2,
	/** Multiple units, roadblocks */
	Pursuit = 3,
	/** All units, spike strips */
	Manhunt = 4
};

/**
 * Police event types
 */
UENUM(BlueprintType)
enum class EMGPoliceEvent : uint8
{
	/** Speed violation observed */
	Speeding,
	/** Near miss with civilian */
	RecklessDriving,
	/** Collision with civilian */
	HitAndRun,
	/** Evaded a patrol */
	Evading,
	/** Completed pursuit escape */
	EscapedPursuit,
	/** Got busted */
	Busted,
	/** Heat decayed naturally */
	HeatDecay,
	/** Entered safe zone (garage, etc) */
	EnteredSafeZone
};

/**
 * Bust consequences data
 */
USTRUCT(BlueprintType)
struct FMGBustConsequences
{
	GENERATED_BODY()

	/** Vehicle was impounded */
	UPROPERTY(BlueprintReadOnly)
	bool bVehicleImpounded = false;

	/** Fine amount (5-15% of car value) */
	UPROPERTY(BlueprintReadOnly)
	int64 FineAmount = 0;

	/** REP lost */
	UPROPERTY(BlueprintReadOnly)
	int32 REPLost = 0;

	/** Days until auction if not retrieved */
	UPROPERTY(BlueprintReadOnly)
	int32 DaysUntilAuction = 7;

	/** Retrieval cost at impound */
	UPROPERTY(BlueprintReadOnly)
	int64 RetrievalCost = 0;
};

/**
 * Pursuit state information
 */
USTRUCT(BlueprintType)
struct FMGPursuitState
{
	GENERATED_BODY()

	/** Current heat level */
	UPROPERTY(BlueprintReadOnly)
	EMGHeatLevel HeatLevel = EMGHeatLevel::Clean;

	/** Time in current heat level */
	UPROPERTY(BlueprintReadOnly)
	float TimeInHeat = 0.0f;

	/** Total pursuit time */
	UPROPERTY(BlueprintReadOnly)
	float TotalPursuitTime = 0.0f;

	/** Number of pursuing units */
	UPROPERTY(BlueprintReadOnly)
	int32 PursuingUnits = 0;

	/** Maximum units that will pursue */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPursuingUnits = 0;

	/** Roadblocks active */
	UPROPERTY(BlueprintReadOnly)
	int32 ActiveRoadblocks = 0;

	/** Spike strips deployed */
	UPROPERTY(BlueprintReadOnly)
	int32 DeployedSpikeStrips = 0;

	/** Distance from nearest unit */
	UPROPERTY(BlueprintReadOnly)
	float DistanceToNearestUnit = 0.0f;

	/** Is in cooldown (hiding) */
	UPROPERTY(BlueprintReadOnly)
	bool bInCooldown = false;

	/** Cooldown progress (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float CooldownProgress = 0.0f;

	/** Is in safe zone */
	UPROPERTY(BlueprintReadOnly)
	bool bInSafeZone = false;
};

/**
 * Police spawn point data
 */
USTRUCT(BlueprintType)
struct FMGPoliceSpawnPoint
{
	GENERATED_BODY()

	/** Spawn location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** Initial direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Is patrol route start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPatrolStart = false;

	/** Patrol route spline (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PatrolRouteID;

	/** District this spawn belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DistrictID;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatLevelChanged, EMGHeatLevel, OldLevel, EMGHeatLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoliceEvent, EMGPoliceEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBusted, const FMGBustConsequences&, Consequences);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPursuitStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPursuitEnded, bool, bEscaped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownProgress, float, Progress);

/**
 * Police Subsystem
 * Manages the police/heat system as defined in PRD Section 4.4
 *
 * Heat Levels:
 * - Clean (0): No police attention
 * - Noticed (1): Occasional patrol passes
 * - Wanted (2): Active patrol searching
 * - Pursuit (3): Multiple units, roadblocks
 * - Manhunt (4): All units, spike strips
 *
 * Bust Consequences:
 * - Car impounded
 * - Fine (5-15% of car value)
 * - REP loss (-200 to -1000)
 * - Must retrieve car at impound (cost + time)
 * - After 7 days: Car auctioned (lost)
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPoliceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// HEAT MANAGEMENT
	// ==========================================

	/**
	 * Add heat from an action
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Heat")
	void AddHeat(int32 Amount, EMGPoliceEvent Reason);

	/**
	 * Remove heat (escape, time decay)
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Heat")
	void RemoveHeat(int32 Amount);

	/**
	 * Get current heat level
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	EMGHeatLevel GetHeatLevel() const { return CurrentPursuitState.HeatLevel; }

	/**
	 * Get current heat points (raw value)
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	int32 GetHeatPoints() const { return HeatPoints; }

	/**
	 * Get pursuit state
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	FMGPursuitState GetPursuitState() const { return CurrentPursuitState; }

	/**
	 * Is in active pursuit
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	bool IsInPursuit() const { return CurrentPursuitState.HeatLevel >= EMGHeatLevel::Pursuit; }

	/**
	 * Clear all heat (debug/cheat)
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Heat")
	void ClearHeat();

	// ==========================================
	// PURSUIT MANAGEMENT
	// ==========================================

	/**
	 * Start pursuit on player
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void StartPursuit(AMGVehiclePawn* Target);

	/**
	 * End pursuit (escape or bust)
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void EndPursuit(bool bEscaped);

	/**
	 * Player entered safe zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void EnterSafeZone();

	/**
	 * Player exited safe zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void ExitSafeZone();

	/**
	 * Bust the player
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	FMGBustConsequences BustPlayer();

	// ==========================================
	// PATROL MANAGEMENT
	// ==========================================

	/**
	 * Set police activity level for district
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Patrol")
	void SetDistrictActivity(FName DistrictID, float ActivityLevel);

	/**
	 * Get police activity level for district
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Patrol")
	float GetDistrictActivity(FName DistrictID) const;

	/**
	 * Spawn patrol unit
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Patrol")
	AMGPoliceVehicle* SpawnPatrolUnit(const FMGPoliceSpawnPoint& SpawnPoint);

	/**
	 * Despawn patrol unit
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Patrol")
	void DespawnPatrolUnit(AMGPoliceVehicle* Unit);

	/**
	 * Get active patrol units
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Patrol")
	TArray<AMGPoliceVehicle*> GetActivePatrolUnits() const { return ActivePatrolUnits; }

	/**
	 * Get pursuing units
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Patrol")
	TArray<AMGPoliceVehicle*> GetPursuingUnits() const { return PursuingUnits; }

	// ==========================================
	// ROADBLOCKS & TACTICS
	// ==========================================

	/**
	 * Request roadblock deployment
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Tactics")
	void RequestRoadblock(FVector Location);

	/**
	 * Request spike strip deployment
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Tactics")
	void RequestSpikeStrip(FVector Location, FRotator Direction);

	/**
	 * Clear all roadblocks
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Tactics")
	void ClearAllRoadblocks();

	// ==========================================
	// IMPOUND MANAGEMENT
	// ==========================================

	/**
	 * Get impounded vehicle data
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Impound")
	bool HasImpoundedVehicles() const;

	/**
	 * Get retrieval cost for vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Impound")
	int64 GetRetrievalCost(FGuid VehicleID) const;

	/**
	 * Retrieve vehicle from impound
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Impound")
	bool RetrieveVehicle(FGuid VehicleID);

	// ==========================================
	// SETTINGS
	// ==========================================

	/**
	 * Set police enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Settings")
	void SetPoliceEnabled(bool bEnabled) { bPoliceEnabled = bEnabled; }

	/**
	 * Is police enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Settings")
	bool IsPoliceEnabled() const { return bPoliceEnabled; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Heat level changed */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnHeatLevelChanged OnHeatLevelChanged;

	/** Police event occurred */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceEvent OnPoliceEvent;

	/** Player was busted */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPlayerBusted OnPlayerBusted;

	/** Pursuit started */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPursuitStarted OnPursuitStarted;

	/** Pursuit ended */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPursuitEnded OnPursuitEnded;

	/** Cooldown progress updated */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnCooldownProgress OnCooldownProgress;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update heat decay */
	void UpdateHeatDecay(float DeltaTime);

	/** Update pursuit state */
	void UpdatePursuit(float DeltaTime);

	/** Update cooldown */
	void UpdateCooldown(float DeltaTime);

	/** Calculate heat level from points */
	EMGHeatLevel CalculateHeatLevel(int32 Points) const;

	/** Get max units for heat level */
	int32 GetMaxUnitsForHeatLevel(EMGHeatLevel Level) const;

	/** Spawn pursuit units */
	void SpawnPursuitUnits(int32 Count);

	/** Calculate bust consequences */
	FMGBustConsequences CalculateBustConsequences() const;

	/** Tick function */
	void OnTick();

private:
	/** Current heat points (0-1000) */
	int32 HeatPoints = 0;

	/** Current pursuit state */
	FMGPursuitState CurrentPursuitState;

	/** Active patrol units */
	UPROPERTY()
	TArray<TObjectPtr<AMGPoliceVehicle>> ActivePatrolUnits;

	/** Pursuing units */
	UPROPERTY()
	TArray<TObjectPtr<AMGPoliceVehicle>> PursuingUnits;

	/** Pursuit target */
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> PursuitTarget;

	/** District activity levels */
	TMap<FName, float> DistrictActivityLevels;

	/** Impounded vehicle IDs with timestamps */
	TMap<FGuid, FDateTime> ImpoundedVehicles;

	/** Is police enabled */
	bool bPoliceEnabled = true;

	/** Timer handle */
	FTimerHandle TickTimer;

	/** Heat thresholds */
	static constexpr int32 HeatNoticed = 100;
	static constexpr int32 HeatWanted = 250;
	static constexpr int32 HeatPursuit = 500;
	static constexpr int32 HeatManhunt = 800;

	/** Heat decay rate per second (when not in pursuit) */
	static constexpr float HeatDecayRate = 5.0f;

	/** Cooldown time required to escape (seconds) */
	static constexpr float CooldownDuration = 30.0f;

	/** Heat values for events */
	static constexpr int32 HeatSpeeding = 25;
	static constexpr int32 HeatReckless = 50;
	static constexpr int32 HeatHitAndRun = 100;
	static constexpr int32 HeatEvading = 75;
};

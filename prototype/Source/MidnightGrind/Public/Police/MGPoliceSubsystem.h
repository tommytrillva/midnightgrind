// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGPoliceSubsystem.generated.h"

class AMGVehiclePawn;
class AMGPoliceUnit;

/**
 * Heat level - determines police response intensity
 */
UENUM(BlueprintType)
enum class EMGHeatLevel : uint8
{
	None UMETA(DisplayName = "Clear"),
	Level1 UMETA(DisplayName = "Heat Level 1 - Patrol"),
	Level2 UMETA(DisplayName = "Heat Level 2 - Pursuit"),
	Level3 UMETA(DisplayName = "Heat Level 3 - Aggressive"),
	Level4 UMETA(DisplayName = "Heat Level 4 - Roadblocks"),
	Level5 UMETA(DisplayName = "Heat Level 5 - Maximum"),
	Busted UMETA(DisplayName = "Busted")
};

/**
 * Types of violations that increase heat
 */
UENUM(BlueprintType)
enum class EMGViolationType : uint8
{
	Speeding UMETA(DisplayName = "Speeding"),
	Reckless UMETA(DisplayName = "Reckless Driving"),
	RunRedLight UMETA(DisplayName = "Running Red Light"),
	HitCivilian UMETA(DisplayName = "Hit Civilian Vehicle"),
	HitPolice UMETA(DisplayName = "Hit Police Vehicle"),
	EvadePursuit UMETA(DisplayName = "Evading Pursuit"),
	StreetRacing UMETA(DisplayName = "Street Racing"),
	PropertyDamage UMETA(DisplayName = "Property Damage"),
	WrongWay UMETA(DisplayName = "Wrong Way Driving"),
	Nitrous UMETA(DisplayName = "Nitrous Use")
};

/**
 * Police unit types
 */
UENUM(BlueprintType)
enum class EMGPoliceUnitType : uint8
{
	Patrol UMETA(DisplayName = "Patrol Car"),
	Interceptor UMETA(DisplayName = "Interceptor"),
	SUV UMETA(DisplayName = "Police SUV"),
	Undercover UMETA(DisplayName = "Undercover"),
	Helicopter UMETA(DisplayName = "Police Helicopter"),
	Roadblock UMETA(DisplayName = "Roadblock Unit"),
	SpikeStrip UMETA(DisplayName = "Spike Strip Unit")
};

/**
 * Police behavior state
 */
UENUM(BlueprintType)
enum class EMGPoliceBehavior : uint8
{
	Patrolling UMETA(DisplayName = "Patrolling"),
	Alerted UMETA(DisplayName = "Alerted"),
	Pursuing UMETA(DisplayName = "Pursuing"),
	Ramming UMETA(DisplayName = "Ramming"),
	Boxing UMETA(DisplayName = "Boxing In"),
	SettingRoadblock UMETA(DisplayName = "Setting Roadblock"),
	Disabled UMETA(DisplayName = "Disabled")
};

/**
 * Pursuit outcome
 */
UENUM(BlueprintType)
enum class EMGPursuitOutcome : uint8
{
	InProgress UMETA(DisplayName = "In Progress"),
	Escaped UMETA(DisplayName = "Escaped"),
	Busted UMETA(DisplayName = "Busted"),
	CooldownPending UMETA(DisplayName = "Cooldown Pending")
};

/**
 * Individual police unit state
 */
USTRUCT(BlueprintType)
struct FMGPoliceUnitState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 UnitID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	EMGPoliceUnitType UnitType = EMGPoliceUnitType::Patrol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	EMGPoliceBehavior Behavior = EMGPoliceBehavior::Patrolling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	TWeakObjectPtr<AMGPoliceUnit> UnitActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	FVector LastKnownPlayerPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	float DistanceToPlayer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	float Health = 100.0f; // Damage before disabled

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	bool bHasVisualOnPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	float TimeSinceSawPlayer = 0.0f;
};

/**
 * Violation record
 */
USTRUCT(BlueprintType)
struct FMGViolationRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	EMGViolationType Type = EMGViolationType::Speeding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	int32 HeatGained = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	int64 FineAmount = 0;
};

/**
 * Pursuit statistics
 */
USTRUCT(BlueprintType)
struct FMGPursuitStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Duration = 0.0f; // Seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TopSpeed = 0.0f; // mph

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DistanceTraveled = 0.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CopsEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CopsDisabled = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 RoadblocksEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 SpikeStripsEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CiviliansHit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 PropertyDestroyed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 NearMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	TArray<FMGViolationRecord> Violations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 TotalFines = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 BountyEarned = 0; // For escaping
};

/**
 * Cooldown zone (where player can hide)
 */
USTRUCT(BlueprintType)
struct FMGCooldownZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	float Radius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	float CooldownMultiplier = 2.0f; // Faster cooldown here

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FString ZoneName; // "Underground Parking", "Alley", etc.
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatLevelChanged, EMGHeatLevel, OldLevel, EMGHeatLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPursuitStarted, EMGHeatLevel, InitialHeat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitEnded, EMGPursuitOutcome, Outcome, const FMGPursuitStats&, Stats);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViolationCommitted, const FMGViolationRecord&, Violation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitSpawned, int32, UnitID, EMGPoliceUnitType, UnitType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoliceUnitDisabled, int32, UnitID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBusted, const FMGPursuitStats&, Stats);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEscaped, const FMGPursuitStats&, Stats);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownStarted, float, CooldownDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooldownComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnteredCooldownZone, const FMGCooldownZone&, Zone);

/**
 * Police and Wanted System
 *
 * Manages police AI, pursuit mechanics, heat levels,
 * and consequences for illegal activities.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGPoliceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// ==========================================
	// DELEGATES
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHeatLevelChanged OnHeatLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPursuitStarted OnPursuitStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPursuitEnded OnPursuitEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnViolationCommitted OnViolationCommitted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPoliceUnitSpawned OnPoliceUnitSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPoliceUnitDisabled OnPoliceUnitDisabled;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerBusted OnPlayerBusted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerEscaped OnPlayerEscaped;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCooldownStarted OnCooldownStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCooldownComplete OnCooldownComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnteredCooldownZone OnEnteredCooldownZone;

	// ==========================================
	// HEAT MANAGEMENT
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Heat")
	void AddHeat(int32 Amount, EMGViolationType Reason);

	UFUNCTION(BlueprintCallable, Category = "Heat")
	void SetHeatLevel(EMGHeatLevel NewLevel);

	UFUNCTION(BlueprintPure, Category = "Heat")
	EMGHeatLevel GetCurrentHeatLevel() const { return CurrentHeatLevel; }

	UFUNCTION(BlueprintPure, Category = "Heat")
	int32 GetCurrentHeatPoints() const { return CurrentHeatPoints; }

	UFUNCTION(BlueprintPure, Category = "Heat")
	float GetHeatDecayProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Heat")
	void ClearHeat();

	UFUNCTION(BlueprintPure, Category = "Heat")
	int32 GetHeatPointsForLevel(EMGHeatLevel Level) const;

	// ==========================================
	// PURSUIT STATE
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Pursuit")
	bool IsInPursuit() const { return bInPursuit; }

	UFUNCTION(BlueprintPure, Category = "Pursuit")
	bool IsInCooldown() const { return bInCooldown; }

	UFUNCTION(BlueprintPure, Category = "Pursuit")
	float GetCooldownProgress() const;

	UFUNCTION(BlueprintPure, Category = "Pursuit")
	float GetCooldownTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Pursuit")
	const FMGPursuitStats& GetCurrentPursuitStats() const { return CurrentPursuitStats; }

	UFUNCTION(BlueprintCallable, Category = "Pursuit")
	void StartPursuit();

	UFUNCTION(BlueprintCallable, Category = "Pursuit")
	void EndPursuit(EMGPursuitOutcome Outcome);

	UFUNCTION(BlueprintCallable, Category = "Pursuit")
	void StartCooldown();

	// ==========================================
	// VIOLATIONS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Violations")
	void ReportViolation(EMGViolationType Type, FVector Location);

	UFUNCTION(BlueprintPure, Category = "Violations")
	int32 GetHeatForViolation(EMGViolationType Type) const;

	UFUNCTION(BlueprintPure, Category = "Violations")
	int64 GetFineForViolation(EMGViolationType Type) const;

	// ==========================================
	// POLICE UNITS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Units")
	int32 SpawnPoliceUnit(EMGPoliceUnitType UnitType, FVector SpawnLocation);

	UFUNCTION(BlueprintCallable, Category = "Units")
	void DespawnPoliceUnit(int32 UnitID);

	UFUNCTION(BlueprintCallable, Category = "Units")
	void DespawnAllUnits();

	UFUNCTION(BlueprintPure, Category = "Units")
	int32 GetActiveUnitCount() const;

	UFUNCTION(BlueprintPure, Category = "Units")
	int32 GetMaxUnitsForHeatLevel(EMGHeatLevel Level) const;

	UFUNCTION(BlueprintPure, Category = "Units")
	TArray<FMGPoliceUnitState> GetActiveUnits() const { return ActiveUnits; }

	UFUNCTION(BlueprintCallable, Category = "Units")
	void DisableUnit(int32 UnitID);

	UFUNCTION(BlueprintCallable, Category = "Units")
	void SpawnRoadblock(FVector Location, FVector Direction);

	UFUNCTION(BlueprintCallable, Category = "Units")
	void DeploySpikeStrip(FVector Location, FVector Direction);

	// ==========================================
	// BUSTED MECHANICS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Busted")
	void PlayerBusted();

	UFUNCTION(BlueprintPure, Category = "Busted")
	float GetBustedProgress() const { return BustedProgress; }

	UFUNCTION(BlueprintPure, Category = "Busted")
	bool IsGettingBusted() const { return bGettingBusted; }

	UFUNCTION(BlueprintCallable, Category = "Busted")
	void CancelBusted(); // Player escaped during bust

	UFUNCTION(BlueprintPure, Category = "Busted")
	int64 CalculateBustPenalty() const;

	// ==========================================
	// COOLDOWN ZONES
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Zones")
	void RegisterCooldownZone(const FMGCooldownZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "Zones")
	void UnregisterCooldownZone(FName ZoneID);

	UFUNCTION(BlueprintPure, Category = "Zones")
	bool IsInCooldownZone() const { return bInCooldownZone; }

	UFUNCTION(BlueprintPure, Category = "Zones")
	FMGCooldownZone GetCurrentCooldownZone() const { return CurrentCooldownZone; }

	// ==========================================
	// PLAYER STATE
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerVehicle(AMGVehiclePawn* Vehicle);

	UFUNCTION(BlueprintPure, Category = "Player")
	FVector GetPlayerLastKnownPosition() const { return PlayerLastKnownPosition; }

	UFUNCTION(BlueprintPure, Category = "Player")
	bool CanPoliceCurrentlySeePlayer() const;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Config")
	void SetPoliceEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Config")
	bool IsPoliceEnabled() const { return bPoliceEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Config")
	void SetCooldownDuration(float BaseDuration);

	// ==========================================
	// STATS AND HISTORY
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetTotalBusts() const { return TotalBusts; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetTotalEscapes() const { return TotalEscapes; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int64 GetTotalFinesPaid() const { return TotalFinesPaid; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int64 GetTotalBountyEarned() const { return TotalBountyEarned; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetLongestPursuitTime() const { return LongestPursuitTime; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetHighestHeatLevelReached() const;

protected:
	// Tick for pursuit logic
	void UpdatePursuit(float DeltaTime);
	void UpdateCooldown(float DeltaTime);
	void UpdatePoliceAI(float DeltaTime);
	void UpdateBustedState(float DeltaTime);
	void CheckCooldownZones();
	void SpawnUnitsForHeatLevel();
	EMGHeatLevel CalculateHeatLevel() const;

private:
	// Timer handle for updates
	FTimerHandle UpdateTimerHandle;

	// Current state
	UPROPERTY()
	EMGHeatLevel CurrentHeatLevel = EMGHeatLevel::None;

	UPROPERTY()
	int32 CurrentHeatPoints = 0;

	UPROPERTY()
	bool bInPursuit = false;

	UPROPERTY()
	bool bInCooldown = false;

	UPROPERTY()
	float CooldownTimer = 0.0f;

	UPROPERTY()
	float CooldownDuration = 30.0f; // Base cooldown time

	UPROPERTY()
	bool bGettingBusted = false;

	UPROPERTY()
	float BustedProgress = 0.0f; // 0-1, 1 = busted

	UPROPERTY()
	float BustedTimer = 0.0f;

	UPROPERTY()
	float BustedDuration = 5.0f; // Seconds to complete bust

	// Police units
	UPROPERTY()
	TArray<FMGPoliceUnitState> ActiveUnits;

	UPROPERTY()
	int32 NextUnitID = 1;

	// Player tracking
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> PlayerVehicle;

	UPROPERTY()
	FVector PlayerLastKnownPosition = FVector::ZeroVector;

	UPROPERTY()
	float TimeSincePlayerSeen = 0.0f;

	// Cooldown zones
	UPROPERTY()
	TArray<FMGCooldownZone> CooldownZones;

	UPROPERTY()
	bool bInCooldownZone = false;

	UPROPERTY()
	FMGCooldownZone CurrentCooldownZone;

	// Current pursuit
	UPROPERTY()
	FMGPursuitStats CurrentPursuitStats;

	// Configuration
	UPROPERTY()
	bool bPoliceEnabled = true;

	// Lifetime stats
	UPROPERTY()
	int32 TotalBusts = 0;

	UPROPERTY()
	int32 TotalEscapes = 0;

	UPROPERTY()
	int64 TotalFinesPaid = 0;

	UPROPERTY()
	int64 TotalBountyEarned = 0;

	UPROPERTY()
	float LongestPursuitTime = 0.0f;

	UPROPERTY()
	EMGHeatLevel HighestHeatReached = EMGHeatLevel::None;

	// Heat thresholds
	static constexpr int32 HeatLevel1Threshold = 100;
	static constexpr int32 HeatLevel2Threshold = 300;
	static constexpr int32 HeatLevel3Threshold = 600;
	static constexpr int32 HeatLevel4Threshold = 1000;
	static constexpr int32 HeatLevel5Threshold = 1500;
};

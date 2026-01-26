// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGPoliceSubsystem.generated.h"

class AMGVehiclePawn;
class AMGPoliceUnit;
class AMGPoliceRoadblock;
class AMGSpikeStrip;

/**
 * @enum EMGHeatLevel
 * @brief Heat level enumeration determining police response intensity.
 *
 * Heat levels escalate based on player infractions and evasion. Higher levels
 * bring more aggressive tactics, faster units, and additional countermeasures.
 * Aligned with GDD Section 4.4 Police System specifications.
 */
UENUM(BlueprintType)
enum class EMGHeatLevel : uint8
{
	/** No police attention - player is clean */
	None UMETA(DisplayName = "Clear"),

	/** Occasional patrol passes - minor infractions noticed */
	Level1 UMETA(DisplayName = "Heat Level 1 - Noticed"),

	/** Active patrol searching - player is wanted */
	Level2 UMETA(DisplayName = "Heat Level 2 - Wanted"),

	/** Multiple units in pursuit - aggressive tactics enabled */
	Level3 UMETA(DisplayName = "Heat Level 3 - Pursuit"),

	/** Roadblocks deployed - coordinated response */
	Level4 UMETA(DisplayName = "Heat Level 4 - Manhunt"),

	/** Maximum response - helicopter, spike strips, all units */
	Level5 UMETA(DisplayName = "Heat Level 5 - Maximum"),

	/** Player has been apprehended */
	Busted UMETA(DisplayName = "Busted")
};

/**
 * @enum EMGViolationType
 * @brief Types of traffic violations that increase heat level.
 *
 * Each violation type has associated heat gain and fine amounts.
 * Violations stack and escalate pursuit intensity.
 */
UENUM(BlueprintType)
enum class EMGViolationType : uint8
{
	/** Exceeding speed limit in police presence */
	Speeding UMETA(DisplayName = "Speeding"),

	/** Dangerous driving maneuvers */
	Reckless UMETA(DisplayName = "Reckless Driving"),

	/** Running a red light */
	RunRedLight UMETA(DisplayName = "Running Red Light"),

	/** Collision with civilian vehicle */
	HitCivilian UMETA(DisplayName = "Hit Civilian Vehicle"),

	/** Collision with police vehicle */
	HitPolice UMETA(DisplayName = "Hit Police Vehicle"),

	/** Fleeing from police */
	EvadePursuit UMETA(DisplayName = "Evading Pursuit"),

	/** Participating in illegal street race */
	StreetRacing UMETA(DisplayName = "Street Racing"),

	/** Damage to property (signs, barriers, etc.) */
	PropertyDamage UMETA(DisplayName = "Property Damage"),

	/** Driving against traffic flow */
	WrongWay UMETA(DisplayName = "Wrong Way Driving"),

	/** Using nitrous in police presence */
	Nitrous UMETA(DisplayName = "Nitrous Use"),

	/** Breaking through police roadblock */
	RoadblockBreach UMETA(DisplayName = "Roadblock Breach"),

	/** Disabling a police unit */
	PoliceVehicleDestroyed UMETA(DisplayName = "Police Vehicle Destroyed"),

	/** Near miss with police vehicle */
	NearMissPolice UMETA(DisplayName = "Near Miss - Police"),

	/** Evading spike strip */
	SpikeStripEvaded UMETA(DisplayName = "Spike Strip Evaded")
};

/**
 * @enum EMGPoliceUnitType
 * @brief Types of police units available for pursuit.
 *
 * Different unit types become available at higher heat levels
 * and have varying performance characteristics and tactics.
 */
UENUM(BlueprintType)
enum class EMGPoliceUnitType : uint8
{
	/** Standard patrol car - base response unit */
	Patrol UMETA(DisplayName = "Patrol Car"),

	/** High-speed pursuit vehicle */
	Interceptor UMETA(DisplayName = "Interceptor"),

	/** Heavy pursuit vehicle with ramming capability */
	SUV UMETA(DisplayName = "Police SUV"),

	/** Unmarked vehicle for surprise tactics */
	Undercover UMETA(DisplayName = "Undercover"),

	/** Aerial pursuit and tracking */
	Helicopter UMETA(DisplayName = "Police Helicopter"),

	/** Static roadblock deployment unit */
	Roadblock UMETA(DisplayName = "Roadblock Unit"),

	/** Spike strip deployment unit */
	SpikeStrip UMETA(DisplayName = "Spike Strip Unit"),

	/** Heavy armored response (Heat Level 5 only) */
	Rhino UMETA(DisplayName = "Rhino Unit")
};

/**
 * @enum EMGPoliceBehavior
 * @brief Current behavior state of a police unit.
 *
 * Determines AI decision-making and pursuit tactics.
 */
UENUM(BlueprintType)
enum class EMGPoliceBehavior : uint8
{
	/** Driving patrol route */
	Patrolling UMETA(DisplayName = "Patrolling"),

	/** Noticed suspicious activity, investigating */
	Alerted UMETA(DisplayName = "Alerted"),

	/** Actively chasing target */
	Pursuing UMETA(DisplayName = "Pursuing"),

	/** Attempting to ram target vehicle */
	Ramming UMETA(DisplayName = "Ramming"),

	/** Coordinating with other units to box in target */
	Boxing UMETA(DisplayName = "Boxing In"),

	/** Executing PIT maneuver */
	PITManeuver UMETA(DisplayName = "PIT Maneuver"),

	/** Setting up roadblock ahead of target */
	SettingRoadblock UMETA(DisplayName = "Setting Roadblock"),

	/** Unit has been disabled/destroyed */
	Disabled UMETA(DisplayName = "Disabled"),

	/** Moving to intercept based on predicted path */
	Intercepting UMETA(DisplayName = "Intercepting")
};

/**
 * @enum EMGPursuitOutcome
 * @brief Possible outcomes of a police pursuit.
 */
UENUM(BlueprintType)
enum class EMGPursuitOutcome : uint8
{
	/** Pursuit is still active */
	InProgress UMETA(DisplayName = "In Progress"),

	/** Player successfully evaded police */
	Escaped UMETA(DisplayName = "Escaped"),

	/** Player was apprehended */
	Busted UMETA(DisplayName = "Busted"),

	/** Player has lost visual, cooldown started */
	CooldownPending UMETA(DisplayName = "Cooldown Pending")
};

/**
 * @enum EMGPoliceTactic
 * @brief Coordinated police tactics used during pursuit.
 */
UENUM(BlueprintType)
enum class EMGPoliceTactic : uint8
{
	/** Standard chase behavior */
	StandardPursuit UMETA(DisplayName = "Standard Pursuit"),

	/** Multiple units surround target */
	BoxingManeuver UMETA(DisplayName = "Boxing Maneuver"),

	/** Setup roadblock ahead of target */
	RoadblockAhead UMETA(DisplayName = "Roadblock Ahead"),

	/** Deploy spike strips on predicted path */
	SpikeStripTrap UMETA(DisplayName = "Spike Strip Trap"),

	/** Helicopter tracking with ground coordination */
	AerialSupport UMETA(DisplayName = "Aerial Support"),

	/** Rolling roadblock with multiple units */
	RollingRoadblock UMETA(DisplayName = "Rolling Roadblock"),

	/** Force target into bottleneck area */
	Funneling UMETA(DisplayName = "Funneling")
};

/**
 * @struct FMGPoliceUnitState
 * @brief Runtime state data for an individual police unit.
 */
USTRUCT(BlueprintType)
struct FMGPoliceUnitState
{
	GENERATED_BODY()

	/** Unique identifier for this unit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 UnitID = 0;

	/** Type of police vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	EMGPoliceUnitType UnitType = EMGPoliceUnitType::Patrol;

	/** Current AI behavior state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	EMGPoliceBehavior Behavior = EMGPoliceBehavior::Patrolling;

	/** Reference to spawned actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	TWeakObjectPtr<AMGPoliceUnit> UnitActor;

	/** Last confirmed player position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	FVector LastKnownPlayerPosition = FVector::ZeroVector;

	/** Current distance to player in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	float DistanceToPlayer = 0.0f;

	/** Unit health before disabled (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	float Health = 100.0f;

	/** Whether unit currently has line of sight to player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	bool bHasVisualOnPlayer = false;

	/** Time since last visual contact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	float TimeSinceSawPlayer = 0.0f;

	/** Aggression level multiplier (1.0 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	float AggressionLevel = 1.0f;

	/** Number of successful rams/PITs performed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	int32 TakedownAttempts = 0;

	/** Time this unit has been in pursuit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	float TimeInPursuit = 0.0f;

	/** Assigned tactic for coordinated pursuit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
	EMGPoliceTactic AssignedTactic = EMGPoliceTactic::StandardPursuit;
};

/**
 * @struct FMGViolationRecord
 * @brief Record of a single traffic violation.
 */
USTRUCT(BlueprintType)
struct FMGViolationRecord
{
	GENERATED_BODY()

	/** Type of violation committed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	EMGViolationType Type = EMGViolationType::Speeding;

	/** When the violation occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FDateTime Timestamp;

	/** World location of violation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FVector Location = FVector::ZeroVector;

	/** Heat points gained from this violation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	int32 HeatGained = 0;

	/** Fine amount in dollars */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	int64 FineAmount = 0;

	/** Whether this violation was witnessed by police */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	bool bWasWitnessed = true;

	/** ID of witnessing unit (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	int32 WitnessUnitID = -1;
};

/**
 * @struct FMGPursuitStats
 * @brief Statistics tracked during a police pursuit.
 */
USTRUCT(BlueprintType)
struct FMGPursuitStats
{
	GENERATED_BODY()

	/** Total pursuit duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Duration = 0.0f;

	/** Maximum speed achieved during pursuit (mph) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TopSpeed = 0.0f;

	/** Total distance traveled during pursuit (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TotalDistance = 0.0f;

	/** Number of times player evaded visual contact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CopsEvaded = 0;

	/** Number of police units disabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CopsDisabled = 0;

	/** Number of roadblocks successfully evaded/breached */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 RoadblocksEvaded = 0;

	/** Number of spike strips avoided */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 SpikeStripsEvaded = 0;

	/** Number of civilian vehicles hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CiviliansHit = 0;

	/** Number of destructible props destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 PropertyDestroyed = 0;

	/** Number of near misses (close calls) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 NearMisses = 0;

	/** All violations committed during pursuit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	TArray<FMGViolationRecord> Violations;

	/** Total accumulated fines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 TotalFines = 0;

	/** Bounty earned for successful escape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 BountyEarned = 0;

	/** Highest heat level reached during pursuit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	EMGHeatLevel PeakHeatLevel = EMGHeatLevel::None;

	/** Number of times cooldown was interrupted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CooldownsInterrupted = 0;

	/** Whether helicopter was deployed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bHelicopterDeployed = false;
};

/**
 * @struct FMGCooldownZone
 * @brief Defines a zone where player can hide to escape pursuit.
 */
USTRUCT(BlueprintType)
struct FMGCooldownZone
{
	GENERATED_BODY()

	/** Unique identifier for this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FName ZoneID;

	/** Center point of cooldown zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FVector Location = FVector::ZeroVector;

	/** Radius of zone in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	float Radius = 1000.0f;

	/** Cooldown speed multiplier (higher = faster escape) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	float CooldownMultiplier = 2.0f;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FString ZoneName;

	/** Maximum heat level this zone is effective for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	EMGHeatLevel MaxEffectiveHeatLevel = EMGHeatLevel::Level3;

	/** Whether zone is currently available (unlocked) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	bool bIsUnlocked = true;

	/** Cost to use this zone (safe house fee) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	int64 UseCost = 0;

	/** Zone type for visuals/audio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FName ZoneType; // "Parking", "Alley", "SafeHouse", etc.
};

/**
 * @struct FMGBustConsequences
 * @brief Consequences applied when player is busted.
 * Per GDD: Car impounded, fine (5-15% of car value), REP loss,
 * must retrieve from impound, after 7 days car auctioned.
 */
USTRUCT(BlueprintType)
struct FMGBustConsequences
{
	GENERATED_BODY()

	/** Whether the vehicle was impounded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequences")
	bool bVehicleImpounded = true;

	/** Fine amount based on heat level and violations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequences")
	int64 FineAmount = 0;

	/** Reputation points lost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequences")
	int32 REPLost = 0;

	/** Cost to retrieve vehicle from impound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequences")
	int64 ImpoundRetrievalCost = 0;

	/** Days until vehicle is auctioned if not retrieved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequences")
	int32 DaysUntilAuction = 7;

	/** Time vehicle was impounded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequences")
	FDateTime ImpoundTime;

	/** Impounded vehicle ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequences")
	FGuid ImpoundedVehicleID;

	/** Whether player has criminal record entry added */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consequences")
	bool bCriminalRecordUpdated = true;
};

/**
 * @struct FMGCriminalRecord
 * @brief Persistent criminal record for player.
 */
USTRUCT(BlueprintType)
struct FMGCriminalRecord
{
	GENERATED_BODY()

	/** Total number of times busted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	int32 TotalBusts = 0;

	/** Total number of successful escapes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	int32 TotalEscapes = 0;

	/** Total fines paid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	int64 TotalFinesPaid = 0;

	/** Total bounty earned from escapes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	int64 TotalBountyEarned = 0;

	/** Total police units disabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	int32 TotalCopsDisabled = 0;

	/** Longest pursuit survived (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	float LongestPursuitSurvived = 0.0f;

	/** Highest heat level ever reached */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	EMGHeatLevel HighestHeatReached = EMGHeatLevel::None;

	/** Violation counts by type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	TMap<EMGViolationType, int32> ViolationCounts;

	/** Total pursuit time accumulated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	float TotalPursuitTime = 0.0f;

	/** Total roadblocks evaded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	int32 TotalRoadblocksEvaded = 0;

	/** "Heat notoriety" level affecting base police aggression */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	float NotorietyLevel = 0.0f;
};

/**
 * @struct FMGImpoundedVehicle
 * @brief Data for a vehicle currently in impound.
 */
USTRUCT(BlueprintType)
struct FMGImpoundedVehicle
{
	GENERATED_BODY()

	/** Vehicle unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impound")
	FGuid VehicleID;

	/** When vehicle was impounded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impound")
	FDateTime ImpoundTime;

	/** Original retrieval cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impound")
	int64 BaseRetrievalCost = 0;

	/** Daily storage fee */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impound")
	int64 DailyStorageFee = 500;

	/** Number of days until auction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impound")
	int32 DaysUntilAuction = 7;

	/** Vehicle name for display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impound")
	FString VehicleDisplayName;

	/** Estimated vehicle value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impound")
	int64 VehicleValue = 0;
};

// ==========================================
// DELEGATE DECLARATIONS
// ==========================================

/** Broadcast when heat level changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatLevelChanged, EMGHeatLevel, OldLevel, EMGHeatLevel, NewLevel);

/** Broadcast when pursuit begins */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPursuitStarted, EMGHeatLevel, InitialHeat);

/** Broadcast when pursuit ends */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitEnded, EMGPursuitOutcome, Outcome, const FMGPursuitStats&, Stats);

/** Broadcast when violation is committed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViolationCommitted, const FMGViolationRecord&, Violation);

/** Broadcast when police unit is spawned */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitSpawned, int32, UnitID, EMGPoliceUnitType, UnitType);

/** Broadcast when police unit is disabled */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoliceUnitDisabled, int32, UnitID);

/** Broadcast when player is busted */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBusted, const FMGBustConsequences&, Consequences);

/** Broadcast when player escapes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEscaped, const FMGPursuitStats&, Stats);

/** Broadcast when cooldown period starts */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownStarted, float, CooldownDuration);

/** Broadcast when cooldown completes (escaped) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooldownComplete);

/** Broadcast when player enters cooldown zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnteredCooldownZone, const FMGCooldownZone&, Zone);

/** Broadcast when player exits cooldown zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitedCooldownZone);

/** Broadcast when roadblock is spawned */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadblockSpawned, FVector, Location);

/** Broadcast when spike strip is deployed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpikeStripDeployed, FVector, Location);

/** Broadcast when helicopter is deployed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHelicopterDeployed);

/** Broadcast when bust progress updates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBustProgressUpdated, float, Progress);

/** Broadcast when police tactic changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoliceTacticChanged, EMGPoliceTactic, NewTactic);

/** Broadcast when vehicle is impounded */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleImpounded, const FMGImpoundedVehicle&, ImpoundData);

/** Broadcast when vehicle is retrieved from impound */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleRetrieved, FGuid, VehicleID);

/**
 * @class UMGPoliceSubsystem
 * @brief Police and Wanted System - World Subsystem
 *
 * Manages police AI, pursuit mechanics, heat levels, and consequences
 * for illegal activities in MIDNIGHT GRIND.
 *
 * Key Systems:
 * - Heat Level Escalation: Minor infractions to full manhunt
 * - Pursuit AI: Police vehicles, coordinated tactics, roadblocks
 * - Escape Mechanics: Cooldown zones, hiding spots, line-of-sight
 * - Consequences: Fines, impound, criminal record, car auctions
 * - Racing Integration: Street races attract police attention
 *
 * Per GDD Section 4.4:
 * - Heat Level 0 (CLEAN): No police attention
 * - Heat Level 1 (NOTICED): Occasional patrol passes
 * - Heat Level 2 (WANTED): Active patrol searching
 * - Heat Level 3 (PURSUIT): Multiple units, aggressive tactics
 * - Heat Level 4 (MANHUNT): Roadblocks, spike strips
 * - Heat Level 5 (MAXIMUM): All units, helicopter support
 *
 * Bust Consequences:
 * - Car impounded
 * - Fine (5-15% of car value)
 * - REP loss (-200 to -1000)
 * - Must retrieve car at impound lot (cost + time)
 * - After 7 days: Car auctioned (lost permanently)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGPoliceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin UWorldSubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	//~ End UWorldSubsystem Interface

	// ==========================================
	// DELEGATES
	// ==========================================

	/** Fired when heat level changes */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnHeatLevelChanged OnHeatLevelChanged;

	/** Fired when pursuit begins */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPursuitStarted OnPursuitStarted;

	/** Fired when pursuit ends (escaped or busted) */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPursuitEnded OnPursuitEnded;

	/** Fired when player commits a violation */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnViolationCommitted OnViolationCommitted;

	/** Fired when police unit spawns */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceUnitSpawned OnPoliceUnitSpawned;

	/** Fired when police unit is disabled */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceUnitDisabled OnPoliceUnitDisabled;

	/** Fired when player is busted */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPlayerBusted OnPlayerBusted;

	/** Fired when player escapes pursuit */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPlayerEscaped OnPlayerEscaped;

	/** Fired when cooldown period starts */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnCooldownStarted OnCooldownStarted;

	/** Fired when cooldown completes */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnCooldownComplete OnCooldownComplete;

	/** Fired when entering cooldown zone */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnEnteredCooldownZone OnEnteredCooldownZone;

	/** Fired when exiting cooldown zone */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnExitedCooldownZone OnExitedCooldownZone;

	/** Fired when roadblock spawns */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnRoadblockSpawned OnRoadblockSpawned;

	/** Fired when spike strip deploys */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnSpikeStripDeployed OnSpikeStripDeployed;

	/** Fired when helicopter deploys */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnHelicopterDeployed OnHelicopterDeployed;

	/** Fired when bust progress updates */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnBustProgressUpdated OnBustProgressUpdated;

	/** Fired when police tactic changes */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceTacticChanged OnPoliceTacticChanged;

	/** Fired when vehicle is impounded */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnVehicleImpounded OnVehicleImpounded;

	/** Fired when vehicle is retrieved */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnVehicleRetrieved OnVehicleRetrieved;

	// ==========================================
	// HEAT MANAGEMENT
	// ==========================================

	/**
	 * @brief Add heat points from a violation or action.
	 * @param Amount Heat points to add
	 * @param Reason Type of violation causing heat gain
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Heat")
	void AddHeat(int32 Amount, EMGViolationType Reason);

	/**
	 * @brief Directly set heat level (for debug/special events).
	 * @param NewLevel Target heat level
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Heat")
	void SetHeatLevel(EMGHeatLevel NewLevel);

	/**
	 * @brief Get current heat level.
	 * @return Current EMGHeatLevel
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	EMGHeatLevel GetCurrentHeatLevel() const { return CurrentHeatLevel; }

	/**
	 * @brief Get raw heat points value.
	 * @return Current heat points (0-2000+)
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	int32 GetCurrentHeatPoints() const { return CurrentHeatPoints; }

	/**
	 * @brief Get heat decay progress (0-1).
	 * @return Progress toward heat decay
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	float GetHeatDecayProgress() const;

	/**
	 * @brief Clear all heat and end pursuit.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Heat")
	void ClearHeat();

	/**
	 * @brief Get heat point threshold for a level.
	 * @param Level Target heat level
	 * @return Heat points required
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	int32 GetHeatPointsForLevel(EMGHeatLevel Level) const;

	/**
	 * @brief Get normalized heat percentage (0-1).
	 * @return Current heat as percentage of max
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	float GetHeatPercentage() const;

	/**
	 * @brief Get progress toward next heat level (0-1).
	 * @return Progress within current level
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Heat")
	float GetHeatLevelProgress() const;

	// ==========================================
	// PURSUIT STATE
	// ==========================================

	/**
	 * @brief Check if player is in active pursuit.
	 * @return True if pursuit is active
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	bool IsInPursuit() const { return bInPursuit; }

	/**
	 * @brief Check if player is in cooldown phase.
	 * @return True if cooldown is active
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	bool IsInCooldown() const { return bInCooldown; }

	/**
	 * @brief Get cooldown progress (0-1).
	 * @return Progress toward escape
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	float GetCooldownProgress() const;

	/**
	 * @brief Get remaining cooldown time in seconds.
	 * @return Seconds until escape
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	float GetCooldownTimeRemaining() const;

	/**
	 * @brief Get current pursuit statistics.
	 * @return Current FMGPursuitStats
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	const FMGPursuitStats& GetCurrentPursuitStats() const { return CurrentPursuitStats; }

	/**
	 * @brief Manually start pursuit (usually automatic via heat).
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void StartPursuit();

	/**
	 * @brief End pursuit with specified outcome.
	 * @param Outcome How pursuit ended
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void EndPursuit(EMGPursuitOutcome Outcome);

	/**
	 * @brief Begin cooldown phase (lost visual).
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void StartCooldown();

	/**
	 * @brief Interrupt cooldown (regained visual).
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void InterruptCooldown();

	/**
	 * @brief Get current coordinated police tactic.
	 * @return Active EMGPoliceTactic
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	EMGPoliceTactic GetCurrentTactic() const { return CurrentTactic; }

	// ==========================================
	// VIOLATIONS
	// ==========================================

	/**
	 * @brief Report a violation at location.
	 * @param Type Violation type
	 * @param Location World position of violation
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Violations")
	void ReportViolation(EMGViolationType Type, FVector Location);

	/**
	 * @brief Report violation with witness info.
	 * @param Type Violation type
	 * @param Location World position
	 * @param bWasWitnessed Whether police saw it
	 * @param WitnessUnitID ID of witnessing unit
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Violations")
	void ReportViolationWithWitness(EMGViolationType Type, FVector Location, bool bWasWitnessed, int32 WitnessUnitID = -1);

	/**
	 * @brief Get heat value for violation type.
	 * @param Type Violation type
	 * @return Heat points gained
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Violations")
	int32 GetHeatForViolation(EMGViolationType Type) const;

	/**
	 * @brief Get fine amount for violation type.
	 * @param Type Violation type
	 * @return Fine in dollars
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Violations")
	int64 GetFineForViolation(EMGViolationType Type) const;

	// ==========================================
	// POLICE UNITS
	// ==========================================

	/**
	 * @brief Spawn a police unit at location.
	 * @param UnitType Type of unit to spawn
	 * @param SpawnLocation World spawn position
	 * @return Unit ID (-1 if failed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Units")
	int32 SpawnPoliceUnit(EMGPoliceUnitType UnitType, FVector SpawnLocation);

	/**
	 * @brief Despawn specific unit by ID.
	 * @param UnitID Unit to despawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Units")
	void DespawnPoliceUnit(int32 UnitID);

	/**
	 * @brief Despawn all active units.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Units")
	void DespawnAllUnits();

	/**
	 * @brief Get count of active units.
	 * @return Number of active police units
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Units")
	int32 GetActiveUnitCount() const;

	/**
	 * @brief Get max units for heat level.
	 * @param Level Target heat level
	 * @return Maximum unit count
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Units")
	int32 GetMaxUnitsForHeatLevel(EMGHeatLevel Level) const;

	/**
	 * @brief Get all active unit states.
	 * @return Array of unit states
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Units")
	TArray<FMGPoliceUnitState> GetActiveUnits() const { return ActiveUnits; }

	/**
	 * @brief Get units currently with visual on player.
	 * @return Array of unit states with visual
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Units")
	TArray<FMGPoliceUnitState> GetUnitsWithVisual() const;

	/**
	 * @brief Get nearest unit to player.
	 * @return Nearest unit state (invalid if none)
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Units")
	FMGPoliceUnitState GetNearestUnit() const;

	/**
	 * @brief Disable a specific unit.
	 * @param UnitID Unit to disable
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Units")
	void DisableUnit(int32 UnitID);

	/**
	 * @brief Spawn roadblock at location.
	 * @param Location Center position
	 * @param Direction Facing direction
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Units")
	void SpawnRoadblock(FVector Location, FVector Direction);

	/**
	 * @brief Deploy spike strip at location.
	 * @param Location Center position
	 * @param Direction Orientation
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Units")
	void DeploySpikeStrip(FVector Location, FVector Direction);

	/**
	 * @brief Deploy helicopter for aerial pursuit.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Units")
	void DeployHelicopter();

	/**
	 * @brief Check if helicopter is active.
	 * @return True if helicopter deployed
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Units")
	bool IsHelicopterActive() const { return bHelicopterActive; }

	// ==========================================
	// TACTICS
	// ==========================================

	/**
	 * @brief Request coordinated tactic.
	 * @param Tactic Desired tactic
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Tactics")
	void RequestTactic(EMGPoliceTactic Tactic);

	/**
	 * @brief Check if tactic is available at current heat.
	 * @param Tactic Tactic to check
	 * @return True if available
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Tactics")
	bool IsTacticAvailable(EMGPoliceTactic Tactic) const;

	// ==========================================
	// BUSTED MECHANICS
	// ==========================================

	/**
	 * @brief Complete bust and apply consequences.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Busted")
	void PlayerBusted();

	/**
	 * @brief Get current bust progress (0-1).
	 * @return Progress toward being busted
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Busted")
	float GetBustedProgress() const { return BustedProgress; }

	/**
	 * @brief Check if player is being busted.
	 * @return True if bust in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Busted")
	bool IsGettingBusted() const { return bGettingBusted; }

	/**
	 * @brief Cancel bust (player escaped during).
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Busted")
	void CancelBusted();

	/**
	 * @brief Calculate total bust penalty.
	 * @return Total fine amount
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Busted")
	int64 CalculateBustPenalty() const;

	/**
	 * @brief Calculate full consequences for bust.
	 * @return Consequences structure
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Busted")
	FMGBustConsequences CalculateBustConsequences() const;

	// ==========================================
	// COOLDOWN ZONES
	// ==========================================

	/**
	 * @brief Register a cooldown zone.
	 * @param Zone Zone data to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Zones")
	void RegisterCooldownZone(const FMGCooldownZone& Zone);

	/**
	 * @brief Unregister a cooldown zone.
	 * @param ZoneID Zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Zones")
	void UnregisterCooldownZone(FName ZoneID);

	/**
	 * @brief Check if player in cooldown zone.
	 * @return True if in zone
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Zones")
	bool IsInCooldownZone() const { return bInCooldownZone; }

	/**
	 * @brief Get current cooldown zone.
	 * @return Current zone (invalid if none)
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Zones")
	FMGCooldownZone GetCurrentCooldownZone() const { return CurrentCooldownZone; }

	/**
	 * @brief Get all registered cooldown zones.
	 * @return Array of all zones
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Zones")
	TArray<FMGCooldownZone> GetAllCooldownZones() const { return CooldownZones; }

	/**
	 * @brief Get nearest cooldown zone to player.
	 * @param OutZone Output zone
	 * @param OutDistance Output distance in cm
	 * @return True if zone found
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Zones")
	bool GetNearestCooldownZone(FMGCooldownZone& OutZone, float& OutDistance) const;

	// ==========================================
	// IMPOUND SYSTEM
	// ==========================================

	/**
	 * @brief Impound a vehicle.
	 * @param VehicleID Vehicle to impound
	 * @param VehicleValue Current vehicle value
	 * @param DisplayName Vehicle name for UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Impound")
	void ImpoundVehicle(FGuid VehicleID, int64 VehicleValue, const FString& DisplayName);

	/**
	 * @brief Calculate retrieval cost for impounded vehicle.
	 * @param VehicleID Impounded vehicle
	 * @return Total retrieval cost
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Impound")
	int64 GetVehicleRetrievalCost(FGuid VehicleID) const;

	/**
	 * @brief Retrieve vehicle from impound.
	 * @param VehicleID Vehicle to retrieve
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Impound")
	bool RetrieveVehicle(FGuid VehicleID);

	/**
	 * @brief Check if vehicle is impounded.
	 * @param VehicleID Vehicle to check
	 * @return True if impounded
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Impound")
	bool IsVehicleImpounded(FGuid VehicleID) const;

	/**
	 * @brief Get all impounded vehicles.
	 * @return Array of impound data
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Impound")
	TArray<FMGImpoundedVehicle> GetImpoundedVehicles() const { return ImpoundedVehicles; }

	/**
	 * @brief Check for expired impound vehicles (auction).
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Impound")
	void ProcessExpiredImpounds();

	// ==========================================
	// PLAYER STATE
	// ==========================================

	/**
	 * @brief Set reference to player vehicle.
	 * @param Vehicle Player vehicle pawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Player")
	void SetPlayerVehicle(AMGVehiclePawn* Vehicle);

	/**
	 * @brief Get player last known position.
	 * @return Last confirmed player location
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Player")
	FVector GetPlayerLastKnownPosition() const { return PlayerLastKnownPosition; }

	/**
	 * @brief Check if any unit has visual on player.
	 * @return True if player is visible
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Player")
	bool CanPoliceCurrentlySeePlayer() const;

	/**
	 * @brief Get distance to nearest pursuing unit.
	 * @return Distance in cm (-1 if none)
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Player")
	float GetDistanceToNearestPursuer() const;

	// ==========================================
	// CRIMINAL RECORD
	// ==========================================

	/**
	 * @brief Get player criminal record.
	 * @return Criminal record data
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Record")
	FMGCriminalRecord GetCriminalRecord() const { return CriminalRecord; }

	/**
	 * @brief Get player notoriety level (0-1).
	 * @return Notoriety affecting base aggression
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Record")
	float GetNotorietyLevel() const { return CriminalRecord.NotorietyLevel; }

	// ==========================================
	// RACING INTEGRATION
	// ==========================================

	/**
	 * @brief Notify that street race started.
	 * @param RaceLocation Race start position
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Racing")
	void OnStreetRaceStarted(FVector RaceLocation);

	/**
	 * @brief Notify that street race ended.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Racing")
	void OnStreetRaceEnded();

	/**
	 * @brief Check if street race is in progress.
	 * @return True if racing
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Racing")
	bool IsStreetRaceActive() const { return bStreetRaceActive; }

	/**
	 * @brief Get heat multiplier during race.
	 * @return Multiplier for heat gain
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Racing")
	float GetRaceHeatMultiplier() const { return RaceHeatMultiplier; }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * @brief Enable or disable police system.
	 * @param bEnabled Whether police are active
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Config")
	void SetPoliceEnabled(bool bEnabled);

	/**
	 * @brief Check if police system is enabled.
	 * @return True if enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Config")
	bool IsPoliceEnabled() const { return bPoliceEnabled; }

	/**
	 * @brief Set base cooldown duration.
	 * @param BaseDuration Duration in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Config")
	void SetCooldownDuration(float BaseDuration);

	/**
	 * @brief Set global aggression multiplier.
	 * @param Multiplier Aggression scaling (1.0 = normal)
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Config")
	void SetAggressionMultiplier(float Multiplier);

	// ==========================================
	// STATS AND HISTORY
	// ==========================================

	/**
	 * @brief Get total times busted.
	 * @return Bust count
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Stats")
	int32 GetTotalBusts() const { return CriminalRecord.TotalBusts; }

	/**
	 * @brief Get total escapes.
	 * @return Escape count
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Stats")
	int32 GetTotalEscapes() const { return CriminalRecord.TotalEscapes; }

	/**
	 * @brief Get total fines paid.
	 * @return Total fines in dollars
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Stats")
	int64 GetTotalFinesPaid() const { return CriminalRecord.TotalFinesPaid; }

	/**
	 * @brief Get total bounty earned.
	 * @return Total bounty in dollars
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Stats")
	int64 GetTotalBountyEarned() const { return CriminalRecord.TotalBountyEarned; }

	/**
	 * @brief Get longest pursuit survived.
	 * @return Duration in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Stats")
	float GetLongestPursuitTime() const { return CriminalRecord.LongestPursuitSurvived; }

	/**
	 * @brief Get highest heat level ever reached.
	 * @return Peak heat level as int
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Stats")
	int32 GetHighestHeatLevelReached() const;

protected:
	// ==========================================
	// UPDATE FUNCTIONS
	// ==========================================

	/** Update pursuit state each tick */
	void UpdatePursuit(float DeltaTime);

	/** Update cooldown timer */
	void UpdateCooldown(float DeltaTime);

	/** Update all police unit AI */
	void UpdatePoliceAI(float DeltaTime);

	/** Update bust progress if stopped near cops */
	void UpdateBustedState(float DeltaTime);

	/** Check if player in cooldown zone */
	void CheckCooldownZones();

	/** Spawn units to match heat level */
	void SpawnUnitsForHeatLevel();

	/** Calculate heat level from points */
	EMGHeatLevel CalculateHeatLevel() const;

	/** Update pursuit statistics */
	void UpdatePursuitStats(float DeltaTime);

	/** Evaluate and assign tactics */
	void EvaluateTactics();

	/** Update heat decay when not in pursuit */
	void UpdateHeatDecay(float DeltaTime);

	/** Calculate bounty reward for escape */
	int64 CalculateBountyReward() const;

	/** Update criminal record after pursuit */
	void UpdateCriminalRecord(EMGPursuitOutcome Outcome);

	/** Calculate impound cost from vehicle value */
	int64 CalculateImpoundCost(int64 VehicleValue) const;

	/** Get unit types available at heat level */
	TArray<EMGPoliceUnitType> GetAvailableUnitTypes(EMGHeatLevel Level) const;

	/** Find optimal spawn point for new unit */
	FVector FindOptimalSpawnLocation() const;

	/** Check if tactic should change based on situation */
	bool ShouldChangeTactic() const;

private:
	// Timer handle for updates
	FTimerHandle UpdateTimerHandle;

	// ==========================================
	// CURRENT STATE
	// ==========================================

	/** Current heat level */
	UPROPERTY()
	EMGHeatLevel CurrentHeatLevel = EMGHeatLevel::None;

	/** Raw heat points */
	UPROPERTY()
	int32 CurrentHeatPoints = 0;

	/** Is pursuit active */
	UPROPERTY()
	bool bInPursuit = false;

	/** Is cooldown active */
	UPROPERTY()
	bool bInCooldown = false;

	/** Cooldown timer */
	UPROPERTY()
	float CooldownTimer = 0.0f;

	/** Total cooldown duration for this pursuit */
	UPROPERTY()
	float CooldownDuration = 30.0f;

	/** Is bust in progress */
	UPROPERTY()
	bool bGettingBusted = false;

	/** Bust progress (0-1) */
	UPROPERTY()
	float BustedProgress = 0.0f;

	/** Time in bust state */
	UPROPERTY()
	float BustedTimer = 0.0f;

	/** Duration to complete bust */
	UPROPERTY()
	float BustedDuration = 5.0f;

	/** Current coordinated tactic */
	UPROPERTY()
	EMGPoliceTactic CurrentTactic = EMGPoliceTactic::StandardPursuit;

	/** Is helicopter deployed */
	UPROPERTY()
	bool bHelicopterActive = false;

	/** Helicopter unit ID */
	UPROPERTY()
	int32 HelicopterUnitID = -1;

	// ==========================================
	// POLICE UNITS
	// ==========================================

	/** Active police units */
	UPROPERTY()
	TArray<FMGPoliceUnitState> ActiveUnits;

	/** Active roadblocks */
	UPROPERTY()
	TArray<TWeakObjectPtr<AMGPoliceRoadblock>> ActiveRoadblocks;

	/** Active spike strips */
	UPROPERTY()
	TArray<TWeakObjectPtr<AMGSpikeStrip>> ActiveSpikeStrips;

	/** Next unit ID to assign */
	UPROPERTY()
	int32 NextUnitID = 1;

	/** Time since last unit spawn */
	UPROPERTY()
	float TimeSinceLastSpawn = 0.0f;

	/** Time since last tactic evaluation */
	UPROPERTY()
	float TimeSinceTacticEvaluation = 0.0f;

	// ==========================================
	// PLAYER TRACKING
	// ==========================================

	/** Reference to player vehicle */
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> PlayerVehicle;

	/** Last confirmed player position */
	UPROPERTY()
	FVector PlayerLastKnownPosition = FVector::ZeroVector;

	/** Time since any unit saw player */
	UPROPERTY()
	float TimeSincePlayerSeen = 0.0f;

	/** Player vehicle ID for impound */
	UPROPERTY()
	FGuid PlayerVehicleID;

	/** Player vehicle value for fines */
	UPROPERTY()
	int64 PlayerVehicleValue = 0;

	// ==========================================
	// COOLDOWN ZONES
	// ==========================================

	/** Registered cooldown zones */
	UPROPERTY()
	TArray<FMGCooldownZone> CooldownZones;

	/** Is player in cooldown zone */
	UPROPERTY()
	bool bInCooldownZone = false;

	/** Current cooldown zone */
	UPROPERTY()
	FMGCooldownZone CurrentCooldownZone;

	// ==========================================
	// PURSUIT DATA
	// ==========================================

	/** Current pursuit statistics */
	UPROPERTY()
	FMGPursuitStats CurrentPursuitStats;

	// ==========================================
	// CRIMINAL RECORD & IMPOUND
	// ==========================================

	/** Persistent criminal record */
	UPROPERTY()
	FMGCriminalRecord CriminalRecord;

	/** Impounded vehicles */
	UPROPERTY()
	TArray<FMGImpoundedVehicle> ImpoundedVehicles;

	// ==========================================
	// RACING INTEGRATION
	// ==========================================

	/** Is street race active */
	UPROPERTY()
	bool bStreetRaceActive = false;

	/** Heat multiplier during race */
	UPROPERTY()
	float RaceHeatMultiplier = 1.5f;

	/** Race start location for spawning */
	UPROPERTY()
	FVector RaceStartLocation = FVector::ZeroVector;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Is police system enabled */
	UPROPERTY()
	bool bPoliceEnabled = true;

	/** Global aggression multiplier */
	UPROPERTY()
	float AggressionMultiplier = 1.0f;

	// ==========================================
	// THRESHOLDS (aligned with GDD Section 4.4)
	// ==========================================

	/** Heat points for Level 1 (Noticed) */
	static constexpr int32 HeatLevel1Threshold = 100;

	/** Heat points for Level 2 (Wanted) */
	static constexpr int32 HeatLevel2Threshold = 250;

	/** Heat points for Level 3 (Pursuit) */
	static constexpr int32 HeatLevel3Threshold = 500;

	/** Heat points for Level 4 (Manhunt) */
	static constexpr int32 HeatLevel4Threshold = 800;

	/** Heat points for Level 5 (Maximum) */
	static constexpr int32 HeatLevel5Threshold = 1200;

	/** Maximum heat points */
	static constexpr int32 MaxHeatPoints = 2000;

	/** Base heat decay rate per second */
	static constexpr float BaseHeatDecayRate = 2.0f;

	/** Spawn interval between units */
	static constexpr float UnitSpawnInterval = 10.0f;

	/** Tactic evaluation interval */
	static constexpr float TacticEvaluationInterval = 5.0f;
};

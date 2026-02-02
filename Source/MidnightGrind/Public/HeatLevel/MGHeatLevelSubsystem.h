// Copyright Midnight Grind. All Rights Reserved.

#pragma once
/**
 * @file MGHeatLevelSubsystem.h
 * @brief Heat/Wanted level management system for Midnight Grind
 *
 * This subsystem implements a "heat" or "wanted level" system similar to
 * classic open-world racing games. Heat accumulates from traffic infractions
 * and illegal activities, determining police response intensity.
 *
 * Core Concepts:
 * - Heat: A numeric value (0 to MaxHeat) that represents accumulated infractions
 * - Heat Level: A tier (1-5 stars) derived from heat amount, determining police response
 * - Infractions: Individual violations (speeding, collisions, etc.) that add heat
 * - Pursuit State: Whether police are actively chasing, searching, or cooldown
 * - Cooldown Spots: Safe locations where heat can be reduced faster
 *
 * The heat system drives the Pursuit Subsystem - when heat reaches certain
 * thresholds, pursuits are initiated with appropriate intensity.
 *
 * @note Heat persists until manually cleared, reduced over time in cooldown,
 *       or reset when busted.
 *
 * @see UMGPursuitSubsystem for active pursuit handling
 * @see UMGBountySubsystem for bounty rewards
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGHeatLevelSubsystem.generated.h"

//=============================================================================
// Heat Level Enumerations
//=============================================================================

// MOVED TO MGSharedTypes.h
// /**
//  * @brief Heat level tiers representing police response intensity
//  *
//  * Similar to "wanted stars" in other games. Higher levels mean more
//  * aggressive police response, more unit types, and harder escapes.
//  */
// UENUM(BlueprintType)
// enum class EMGHeatLevel : uint8
// {
// 	/// No heat - player is free, no police attention
// 	None			UMETA(DisplayName = "No Heat"),
// 	/// 1 star - Light police presence, patrol cars only
// 	Level1			UMETA(DisplayName = "Heat Level 1"),
// 	/// 2 stars - Increased patrols, interceptors may appear
// 	Level2			UMETA(DisplayName = "Heat Level 2"),
// 	/// 3 stars - Roadblocks deployed, SUVs join pursuit
// 	Level3			UMETA(DisplayName = "Heat Level 3"),
// 	/// 4 stars - Spike strips, undercover units, aggressive tactics
// 	Level4			UMETA(DisplayName = "Heat Level 4"),
// 	/// 5 stars - Helicopter support, SWAT, maximum response
// 	Level5			UMETA(DisplayName = "Heat Level 5"),
// 	/// Maximum possible heat - all available resources deployed
// 	MaxHeat			UMETA(DisplayName = "Maximum Heat")
// };

/**
 * @brief Types of infractions that can add heat
 *
 * Each infraction type has configurable heat gain, cost penalty,
 * and whether it requires a police witness to count.
 */
UENUM(BlueprintType)
enum class EMGHeatSource : uint8
{
	/// Exceeding speed limit (scales with excess speed)
	Speeding		UMETA(DisplayName = "Speeding"),
	/// Dangerous driving without collision
	Reckless		UMETA(DisplayName = "Reckless Driving"),
	/// Driving against traffic flow
	WrongWay		UMETA(DisplayName = "Wrong Way"),
	/// Running a traffic signal
	RedLight		UMETA(DisplayName = "Running Red Light"),
	/// Hitting objects, signs, fences, etc.
	PropertyDamage	UMETA(DisplayName = "Property Damage"),
	/// Hitting civilian vehicles
	VehicleDamage	UMETA(DisplayName = "Vehicle Damage"),
	/// Hitting a police vehicle
	Collision		UMETA(DisplayName = "Police Collision"),
	/// Fleeing from police (accumulates over time)
	Evading			UMETA(DisplayName = "Evading"),
	/// Close pass to police vehicle without collision
	NearMiss		UMETA(DisplayName = "Police Near Miss"),
	/// Caught by speed camera/radar trap
	SpeedTrap		UMETA(DisplayName = "Speed Trap"),
	/// Breaking through a police roadblock
	RoadBlock		UMETA(DisplayName = "Roadblock Breach"),
	/// Running over spike strip (adds heat even if tires damaged)
	SpikeStrip		UMETA(DisplayName = "Spike Strip"),
	/// Escaping from helicopter (adds significant heat)
	AirSupport		UMETA(DisplayName = "Helicopter Evade"),
	/// Participating in illegal street races
	IllegalRace		UMETA(DisplayName = "Illegal Racing")
};

// MOVED TO MGSharedTypes.h
// /**
//  * @brief Current pursuit/chase state
//  *
//  * Tracks whether police are actively pursuing, searching for,
//  * or have lost the player.
//  */
// UENUM(BlueprintType)
// enum class EMGPursuitState : uint8
// {
// 	/// No active pursuit - player may or may not have heat
// 	None			UMETA(DisplayName = "No Pursuit"),
// 	/// Police have seen the player, pursuit about to start
// 	Spotted			UMETA(DisplayName = "Spotted"),
// 	/// Active pursuit - police are chasing
// 	Pursuit			UMETA(DisplayName = "In Pursuit"),
// 	/// Player broke visual contact, trying to escape
// 	Escaping		UMETA(DisplayName = "Escaping"),
// 	/// Escape timer active, heat reducing
// 	Cooldown		UMETA(DisplayName = "Cooldown"),
// 	/// Successfully escaped pursuit
// 	Evaded			UMETA(DisplayName = "Evaded"),
// 	/// Caught by police - pursuit failed
// 	Busted			UMETA(DisplayName = "Busted")
// };

// MOVED TO MGSharedTypes.h
// /**
//  * @brief Types of police units that can be deployed
//  *
//  * Different unit types are unlocked at different heat levels
//  * and have varying speed, aggression, and tactics.
//  */
// UENUM(BlueprintType)
// enum class EMGPoliceUnitType : uint8
// {
// 	/// Standard patrol car - slowest, least aggressive
// 	Patrol			UMETA(DisplayName = "Patrol Car"),
// 	/// High-speed pursuit vehicle - faster, uses PIT maneuvers
// 	Interceptor		UMETA(DisplayName = "Interceptor"),
// 	/// Police SUV - tough, can take more damage
// 	SUV				UMETA(DisplayName = "SUV"),
// 	/// Muscle car unit - high speed, aggressive ramming
// 	Muscle			UMETA(DisplayName = "Muscle Unit"),
// 	/// Exotic police car - fastest ground unit
// 	Supercar		UMETA(DisplayName = "Supercar Unit"),
// 	/// Unmarked vehicle - harder to spot, surprise tactics
// 	Undercover		UMETA(DisplayName = "Undercover"),
// 	/// Aerial unit - provides tracking, never loses visual
// 	Helicopter		UMETA(DisplayName = "Helicopter"),
// 	/// Stationary units forming blockades
// 	Roadblock		UMETA(DisplayName = "Roadblock Unit"),
// 	/// Units that deploy tire-damage strips
// 	Spike			UMETA(DisplayName = "Spike Strip Unit"),
// 	/// Tactical team - most aggressive ground units
// 	SWAT			UMETA(DisplayName = "SWAT"),
// 	/// Heavy armored vehicle - nearly indestructible
// 	Rhino			UMETA(DisplayName = "Rhino/Heavy")
// };

//=============================================================================
// Heat Level Data Structures
//=============================================================================

/**
 * @brief Record of a single infraction event
 *
 * Created each time the player commits an infraction that adds heat.
 * Used for session tracking, UI feedback, and post-race summaries.
 */
USTRUCT(BlueprintType)
struct FMGHeatInfraction
{
	GENERATED_BODY()

	/// Unique ID for this infraction instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InfractionId;

	/// Type of infraction committed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatSource Source = EMGHeatSource::Speeding;

	/// Amount of heat added by this infraction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeatGained = 0;

	/// Monetary cost/fine for this infraction (applied on bust)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CostPenalty = 0;

	/// World location where infraction occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// When the infraction occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/// Whether a police unit saw this infraction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasWitnessed = false;

	/// ID of the unit that witnessed (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WitnessUnitId;
};

/**
 * @brief Configuration for a specific heat source/infraction type
 *
 * Defines how much heat is gained, stacking behavior, and
 * whether witness is required for each infraction type.
 */
USTRUCT(BlueprintType)
struct FMGHeatSourceConfig
{
	GENERATED_BODY()

	/// Which infraction type this configures
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatSource Source = EMGHeatSource::Speeding;

	/// Base heat gained per infraction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseHeatGain = 10;

	/// Base cost penalty per infraction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseCostPenalty = 100;

	/// Seconds between repeat infractions before stack resets
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 5.0f;

	/// If true, infraction only counts if police unit sees it
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresWitness = true;

	/// If true, repeated infractions increase heat gain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStackable = true;

	/// Multiplier applied per stack (e.g., 1.5x per repeat)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StackMultiplier = 1.5f;

	/// Maximum number of stacks allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStacks = 5;
};

/**
 * @brief Configuration for a specific heat level tier
 *
 * Defines thresholds, available police response, and
 * gameplay modifiers for each heat level.
 */
USTRUCT(BlueprintType)
struct FMGHeatLevelConfig
{
	GENERATED_BODY()

	/// Which heat level this configures
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatLevel Level = EMGHeatLevel::None;

	/// Heat value needed to reach this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeatThreshold = 0;

	/// Maximum active police units at this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxUnits = 1;

	/// Unit types that can spawn at this heat level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPoliceUnitType> AvailableUnits;

	/// AI aggression multiplier (1.0 = normal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionMultiplier = 1.0f;

	/// Seconds between unit spawns
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRate = 10.0f;

	/// Seconds needed to escape at this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 30.0f;

	/// Whether roadblocks can deploy at this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRoadblocksEnabled = false;

	/// Whether spike strips can deploy at this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpikeStripsEnabled = false;

	/// Whether helicopter can be called at this level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHelicopterEnabled = false;

	/// Multiplier on bust timer (higher = faster bust)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustTimeMultiplier = 1.0f;

	/// UI color representing this heat level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HeatColor = FLinearColor::White;
};

/**
 * @brief Runtime data for an active police unit
 *
 * Tracks the current state of a single police unit that is
 * part of the current pursuit/patrol.
 */
USTRUCT(BlueprintType)
struct FMGActivePoliceUnit
{
	GENERATED_BODY()

	/// Unique identifier for this unit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnitId;

	/// Type of police vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPoliceUnitType UnitType = EMGPoliceUnitType::Patrol;

	/// Current world position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Distance to player in world units
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToPlayer = 0.0f;

	/// Whether this unit is actively chasing (vs patrolling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInPursuit = false;

	/// Whether this unit can see the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasVisual = false;

	/// Whether this unit has been wrecked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDisabled = false;

	/// How aggressive this unit behaves (0-2, 1=normal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionLevel = 1.0f;

	/// Number of times this unit has hit the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TakedownCount = 0;

	/// Seconds this unit has been in pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeInPursuit = 0.0f;
};

/**
 * @brief Complete pursuit status snapshot
 *
 * Contains all information about the current pursuit state,
 * heat level, active units, and progress meters.
 */
USTRUCT(BlueprintType)
struct FMGPursuitStatus
{
	GENERATED_BODY()

	/// Current pursuit state machine value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitState State = EMGPursuitState::None;

	/// Current heat level tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatLevel CurrentHeatLevel = EMGHeatLevel::None;

	/// Raw heat value (0 to MaxHeat)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentHeat = 0;

	/// Maximum possible heat value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHeat = 1000;

	/// Seconds since pursuit started
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PursuitDuration = 0.0f;

	/// Seconds until cooldown completes (0 if not cooling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	/// Progress toward being busted (0-1, bust at 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustProgress = 0.0f;

	/// Number of active (non-disabled) police units
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActiveUnits = 0;

	/// Number of units player has wrecked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitsDisabled = 0;

	/// Number of roadblocks player has evaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoadblocksEvaded = 0;

	/// Total infractions committed this pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalInfractions = 0;

	/// Total cost/fines accumulated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AccumulatedCost = 0;

	/// Whether helicopter is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHelicopterActive = false;
};

/**
 * @brief Safe house / cooldown location data
 *
 * Defines locations where players can hide to reduce heat
 * faster than normal. May have usage costs or restrictions.
 */
USTRUCT(BlueprintType)
struct FMGCooldownSpot
{
	GENERATED_BODY()

	/// Unique identifier for this location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpotId;

	/// Display name shown in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpotName;

	/// World position of the cooldown zone center
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Radius of the safe zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 50.0f;

	/// Heat decay multiplier while in this zone (2.0 = 2x faster)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownMultiplier = 2.0f;

	/// Maximum heat level this spot is effective at
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatLevel MaxEffectiveHeat = EMGHeatLevel::Level3;

	/// Currency cost to use this spot (0 = free)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UseCost = 0;

	/// Whether player has unlocked this location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = true;

	/// How many times this spot has been used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesUsed = 0;
};

/**
 * @brief Session-wide heat and pursuit statistics
 *
 * Tracks cumulative stats for the current game session.
 */
USTRUCT(BlueprintType)
struct FMGHeatSessionStats
{
	GENERATED_BODY()

	/// Total pursuits initiated this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPursuits = 0;

	/// Pursuits that ended in successful escape
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PursuitsEvaded = 0;

	/// Times player was caught by police
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesBusted = 0;

	/// Longest pursuit duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestPursuit = 0.0f;

	/// Highest heat level reached (as int for stats)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestHeatLevel = 0;

	/// Total police units wrecked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalUnitsDisabled = 0;

	/// Total roadblocks evaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRoadblocksEvaded = 0;

	/// Total infractions committed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalInfractions = 0;

	/// Total fines/costs accumulated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCostAccumulated = 0;

	/// Total bounty earned from escapes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountyEarned = 0;

	/// Breakdown of infractions by type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGHeatSource, int32> InfractionsByType;
};

/**
 * @brief Bounty reward configuration
 *
 * Defines how bounty is calculated during pursuits.
 * Bounty is earned for surviving pursuit and lost when busted.
 */
USTRUCT(BlueprintType)
struct FMGBountyConfig
{
	GENERATED_BODY()

	/// Base bounty earned per second in pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseBountyPerSecond = 10.0f;

	/// Multiplier applied per heat level (compounds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatLevelMultiplier = 1.5f;

	/// Bonus bounty for disabling a police unit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitDisabledBonus = 500;

	/// Bonus bounty for evading a roadblock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoadblockBonus = 250;

	/// Bonus bounty for escaping helicopter
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HelicopterEvadeBonus = 1000;

	/// Final multiplier applied when successfully evading
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EvadeMultiplier = 1.0f;

	/// Percentage of bounty lost when busted (0.5 = 50%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustedPenaltyPercent = 0.5f;
};

//=============================================================================
// Event Delegates
//=============================================================================

/// Fired when heat level tier changes (up or down)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatLevelChanged, EMGHeatLevel, OldLevel, EMGHeatLevel, NewLevel);

/// Fired when pursuit state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitStateChanged, EMGPursuitState, OldState, EMGPursuitState, NewState);

/// Fired when player commits an infraction
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfractionCommitted, const FMGHeatInfraction&, Infraction, int32, TotalHeat);

/// Fired when a new police unit spawns
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitSpawned, const FString&, UnitId, EMGPoliceUnitType, UnitType);

/// Fired when player wrecks a police unit
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitDisabled, const FString&, UnitId, int32, BonusPoints);

/// Fired when player successfully escapes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitEvaded, float, Duration, int32, BountyEarned);

/// Fired when player is caught by police
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerBusted, int32, TotalCost, float, PursuitDuration);

/// Fired when cooldown/escape timer begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownStarted, float, CooldownDuration);

/// Fired when cooldown completes and heat clears
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooldownComplete);

/// Fired when helicopter joins pursuit
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHelicopterDeployed);

/// Fired when a roadblock is set up ahead
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadblockSpawned, FVector, Location);

/// Fired when bust progress changes (for UI meter)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBustProgressUpdate, float, Progress);

//=============================================================================
// Main Heat Level Subsystem Class
//=============================================================================

/**
 * @brief Core subsystem managing heat/wanted levels and police response
 *
 * The Heat Level Subsystem tracks player infractions, calculates heat
 * levels, and coordinates police response. It works alongside the
 * Pursuit Subsystem which handles the actual chase mechanics.
 *
 * Typical Flow:
 * 1. Player commits infraction -> AddHeat() called
 * 2. Heat accumulates, heat level increases
 * 3. At threshold, pursuit begins
 * 4. Player escapes or gets busted
 * 5. Heat clears (escape) or costs applied (busted)
 *
 * @note Access via UGameInstance::GetSubsystem<UMGHeatLevelSubsystem>()
 */
UCLASS()
class MIDNIGHTGRIND_API UMGHeatLevelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//=========================================================================
	// Lifecycle
	//=========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//=========================================================================
	// Event Delegates
	// Subscribe to these for heat and pursuit notifications
	//=========================================================================

	/// Fires when player reaches a new heat level tier
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnHeatLevelChanged OnHeatLevelChanged;

	/// Fires on pursuit state transitions
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPursuitStateChanged OnPursuitStateChanged;

	/// Fires when an infraction adds heat
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnInfractionCommitted OnInfractionCommitted;

	/// Fires when a police unit joins
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPoliceUnitSpawned OnPoliceUnitSpawned;

	/// Fires when player wrecks a cop
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPoliceUnitDisabled OnPoliceUnitDisabled;

	/// Fires on successful escape
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPursuitEvaded OnPursuitEvaded;

	/// Fires when player gets caught
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPlayerBusted OnPlayerBusted;

	/// Fires when escape timer starts
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnCooldownStarted OnCooldownStarted;

	/// Fires when cooldown finishes
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnCooldownComplete OnCooldownComplete;

	/// Fires when helicopter is called in
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnHelicopterDeployed OnHelicopterDeployed;

	/// Fires when roadblock appears ahead
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnRoadblockSpawned OnRoadblockSpawned;

	/// Fires when bust meter changes
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnBustProgressUpdate OnBustProgressUpdate;

	//=========================================================================
	// Heat Management
	// Core functions for adding, removing, and querying heat
	//=========================================================================

	/**
	 * @brief Adds heat from a specific infraction
	 * @param Source Type of infraction committed
	 * @param Location World position where it occurred
	 * @param bWasWitnessed Whether a police unit saw it
	 * @param WitnessId ID of witnessing unit (if applicable)
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel")
	void AddHeat(EMGHeatSource Source, FVector Location, bool bWasWitnessed = true, const FString& WitnessId = TEXT(""));

	/**
	 * @brief Removes heat (used by cooldown system)
	 * @param Amount Heat points to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel")
	void RemoveHeat(int32 Amount);

	/**
	 * @brief Instantly clears all heat (debug/cheat function)
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel")
	void ClearAllHeat();

	/**
	 * @brief Gets raw heat value
	 * @return Current heat (0 to MaxHeat)
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel")
	int32 GetCurrentHeat() const;

	/**
	 * @brief Gets current heat level tier
	 * @return Current EMGHeatLevel
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel")
	EMGHeatLevel GetCurrentHeatLevel() const;

	/**
	 * @brief Gets heat as percentage of max
	 * @return 0.0 to 1.0 representing heat fullness
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel")
	float GetHeatPercent() const;

	/**
	 * @brief Gets progress within current heat level
	 * @return 0.0 to 1.0 representing progress to next level
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel")
	float GetHeatLevelProgress() const;

	//=========================================================================
	// Pursuit State
	// Functions to query and control pursuit state
	//=========================================================================

	/**
	 * @brief Gets complete pursuit status snapshot
	 * @return Current pursuit status struct
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	FMGPursuitStatus GetPursuitStatus() const;

	/**
	 * @brief Gets current pursuit state
	 * @return Current EMGPursuitState value
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	EMGPursuitState GetPursuitState() const;

	/**
	 * @brief Checks if actively being pursued
	 * @return True if in Spotted, Pursuit, or Escaping state
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	bool IsInPursuit() const;

	/**
	 * @brief Checks if in escape attempt state
	 * @return True if in Escaping or Cooldown state
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	bool IsEvading() const;

	/**
	 * @brief Gets how long current pursuit has lasted
	 * @return Pursuit duration in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	float GetPursuitDuration() const;

	/**
	 * @brief Manually starts a pursuit
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Pursuit")
	void StartPursuit();

	/**
	 * @brief Transitions to escaping state
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Pursuit")
	void StartEscaping();

	/**
	 * @brief Updates bust progress meter
	 * @param DeltaProgress Amount to add (can be negative)
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Pursuit")
	void UpdateBustProgress(float DeltaProgress);

	//=========================================================================
	// Police Unit Management
	// Functions to track active police units
	//=========================================================================

	/**
	 * @brief Registers a new police unit
	 * @param Unit Unit data to register
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Units")
	void RegisterPoliceUnit(const FMGActivePoliceUnit& Unit);

	/**
	 * @brief Updates police unit status
	 * @param UnitId Unit to update
	 * @param Location New position
	 * @param bHasVisual Whether unit can see player
	 * @param bIsInPursuit Whether unit is actively chasing
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Units")
	void UpdatePoliceUnit(const FString& UnitId, FVector Location, bool bHasVisual, bool bIsInPursuit);

	/**
	 * @brief Marks a unit as disabled/wrecked
	 * @param UnitId Unit to disable
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Units")
	void DisablePoliceUnit(const FString& UnitId);

	/**
	 * @brief Completely removes a unit from tracking
	 * @param UnitId Unit to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Units")
	void RemovePoliceUnit(const FString& UnitId);

	/**
	 * @brief Gets all tracked police units
	 * @return Array of active unit data
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Units")
	TArray<FMGActivePoliceUnit> GetActiveUnits() const;

	/**
	 * @brief Gets count of non-disabled units
	 * @return Number of active units
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Units")
	int32 GetActiveUnitCount() const;

	/**
	 * @brief Gets the closest police unit to player
	 * @return Nearest unit data
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Units")
	FMGActivePoliceUnit GetNearestUnit() const;

	/**
	 * @brief Checks if any unit has line-of-sight
	 * @return True if at least one unit can see player
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Units")
	bool AnyUnitHasVisual() const;

	//=========================================================================
	// Cooldown and Evasion
	// Functions for escape mechanics
	//=========================================================================

	/**
	 * @brief Begins the cooldown/escape timer
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Cooldown")
	void StartCooldown();

	/**
	 * @brief Called when player enters a safe zone
	 * @param SpotId ID of the cooldown spot entered
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Cooldown")
	void EnterCooldownSpot(const FString& SpotId);

	/**
	 * @brief Called when player leaves a safe zone
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Cooldown")
	void ExitCooldownSpot();

	/**
	 * @brief Gets remaining cooldown time
	 * @return Seconds until cooldown completes
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Cooldown")
	float GetCooldownRemaining() const;

	/**
	 * @brief Gets cooldown progress
	 * @return 0.0 to 1.0 (1.0 = complete)
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Cooldown")
	float GetCooldownProgress() const;

	/**
	 * @brief Checks if player is in a safe zone
	 * @return True if in cooldown spot
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Cooldown")
	bool IsInCooldownSpot() const;

	//=========================================================================
	// Cooldown Spot Management
	// Functions to register and query safe houses
	//=========================================================================

	/**
	 * @brief Registers a new cooldown spot
	 * @param Spot Cooldown spot data
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Spots")
	void RegisterCooldownSpot(const FMGCooldownSpot& Spot);

	/**
	 * @brief Gets a specific cooldown spot
	 * @param SpotId ID of spot to retrieve
	 * @return Cooldown spot data
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Spots")
	FMGCooldownSpot GetCooldownSpot(const FString& SpotId) const;

	/**
	 * @brief Gets all registered cooldown spots
	 * @return Array of all cooldown spots
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Spots")
	TArray<FMGCooldownSpot> GetAllCooldownSpots() const;

	/**
	 * @brief Finds nearest cooldown spot to a location
	 * @param Location Position to search from
	 * @return Nearest cooldown spot
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Spots")
	FMGCooldownSpot GetNearestCooldownSpot(FVector Location) const;

	//=========================================================================
	// Configuration
	// Functions to get/set system parameters
	//=========================================================================

	/**
	 * @brief Sets configuration for a heat source
	 * @param Source Which source to configure
	 * @param Config New configuration values
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Config")
	void SetHeatSourceConfig(EMGHeatSource Source, const FMGHeatSourceConfig& Config);

	/**
	 * @brief Gets configuration for a heat source
	 * @param Source Which source to query
	 * @return Configuration for that source
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Config")
	FMGHeatSourceConfig GetHeatSourceConfig(EMGHeatSource Source) const;

	/**
	 * @brief Sets configuration for a heat level
	 * @param Level Which level to configure
	 * @param Config New configuration values
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Config")
	void SetHeatLevelConfig(EMGHeatLevel Level, const FMGHeatLevelConfig& Config);

	/**
	 * @brief Gets configuration for a heat level
	 * @param Level Which level to query
	 * @return Configuration for that level
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Config")
	FMGHeatLevelConfig GetHeatLevelConfig(EMGHeatLevel Level) const;

	/**
	 * @brief Sets bounty reward configuration
	 * @param Config New bounty configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Config")
	void SetBountyConfig(const FMGBountyConfig& Config);

	/**
	 * @brief Gets bounty reward configuration
	 * @return Current bounty config
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Config")
	FMGBountyConfig GetBountyConfig() const;

	//=========================================================================
	// Bounty System
	// Functions for pursuit bounty rewards
	//=========================================================================

	/**
	 * @brief Gets current bounty value
	 * @return Bounty accumulated this pursuit
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Bounty")
	int32 GetCurrentBounty() const;

	/**
	 * @brief Calculates bounty earned if escaped now
	 * @return Projected escape bounty
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Bounty")
	int32 CalculateEvadeBounty() const;

	/**
	 * @brief Awards bonus for evading roadblock
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Bounty")
	void NotifyRoadblockEvaded();

	/**
	 * @brief Awards bonus for escaping helicopter
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Bounty")
	void NotifyHelicopterEvaded();

	//=========================================================================
	// Special Events
	// Functions for major pursuit events
	//=========================================================================

	/**
	 * @brief Calls in helicopter support
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Special")
	void DeployHelicopter();

	/**
	 * @brief Sets up a roadblock at location
	 * @param Location Where to spawn roadblock
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Special")
	void SpawnRoadblock(FVector Location);

	/**
	 * @brief Deploys spike strip at location
	 * @param Location Where to spawn spike strip
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Special")
	void SpawnSpikeStrip(FVector Location);

	//=========================================================================
	// Bust Mechanics
	// Functions for being caught
	//=========================================================================

	/**
	 * @brief Triggers the bust sequence
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Bust")
	void TriggerBust();

	/**
	 * @brief Gets current bust progress
	 * @return 0.0 to 1.0 (1.0 = busted)
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Bust")
	float GetBustProgress() const;

	/**
	 * @brief Gets total cost/fines if busted now
	 * @return Projected bust cost
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Bust")
	int32 GetBustCost() const;

	//=========================================================================
	// Session Management
	// Functions for game session tracking
	//=========================================================================

	/**
	 * @brief Starts a new game session
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Session")
	void StartSession();

	/**
	 * @brief Ends the current session
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Session")
	void EndSession();

	/**
	 * @brief Checks if session is active
	 * @return True if session is running
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Session")
	bool IsSessionActive() const;

	/**
	 * @brief Gets session statistics
	 * @return Current session stats
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Session")
	FMGHeatSessionStats GetSessionStats() const;

	//=========================================================================
	// Utility Functions
	// Helper functions for UI and display
	//=========================================================================

	/**
	 * @brief Gets display name for a heat level
	 * @param Level Heat level to get name for
	 * @return Localized display name
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Utility")
	FText GetHeatLevelDisplayName(EMGHeatLevel Level) const;

	/**
	 * @brief Gets UI color for a heat level
	 * @param Level Heat level to get color for
	 * @return Color for UI display
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Utility")
	FLinearColor GetHeatLevelColor(EMGHeatLevel Level) const;

	/**
	 * @brief Gets star count for a heat level
	 * @param Level Heat level to convert
	 * @return Number of stars (0-6)
	 */
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Utility")
	int32 GetHeatLevelStars(EMGHeatLevel Level) const;

	//=========================================================================
	// Persistence
	// Functions for save/load
	//=========================================================================

	/**
	 * @brief Saves heat data to disk
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Persistence")
	void SaveHeatData();

	/**
	 * @brief Loads heat data from disk
	 */
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Persistence")
	void LoadHeatData();

protected:
	//=========================================================================
	// Internal Functions
	//=========================================================================

	/// Recalculates heat level from raw heat value
	void UpdateHeatLevel();

	/// Per-tick pursuit state updates
	void TickPursuit(float MGDeltaTime);

	/// Per-tick cooldown timer updates
	void TickCooldown(float MGDeltaTime);

	/// Per-tick bounty accumulation
	void TickBounty(float MGDeltaTime);

	/// Called when cooldown timer finishes
	void CompleteCooldown();

	/// Sets up default configuration values
	void InitializeDefaultConfigs();

	/// Calculates heat gain with stacking
	int32 CalculateInfractionHeat(EMGHeatSource Source) const;

private:
	//=========================================================================
	// Private Data Members
	//=========================================================================

	/// Current pursuit status data
	UPROPERTY()
	FMGPursuitStatus PursuitStatus;

	/// Session statistics
	UPROPERTY()
	FMGHeatSessionStats SessionStats;

	/// Bounty reward configuration
	UPROPERTY()
	FMGBountyConfig BountyConfig;

	/// Configuration per heat source type
	UPROPERTY()
	TMap<EMGHeatSource, FMGHeatSourceConfig> HeatSourceConfigs;

	/// Configuration per heat level tier
	UPROPERTY()
	TMap<EMGHeatLevel, FMGHeatLevelConfig> HeatLevelConfigs;

	/// Currently tracked police units
	UPROPERTY()
	TMap<FString, FMGActivePoliceUnit> ActiveUnits;

	/// Registered cooldown spots
	UPROPERTY()
	TMap<FString, FMGCooldownSpot> CooldownSpots;

	/// Current infraction stack counts per source
	UPROPERTY()
	TMap<EMGHeatSource, int32> InfractionStacks;

	/// Cooldown timers per infraction source
	UPROPERTY()
	TMap<EMGHeatSource, float> InfractionCooldowns;

	/// Current accumulated bounty
	UPROPERTY()
	int32 CurrentBounty = 0;

	/// Total cooldown time for current escape
	UPROPERTY()
	float CooldownTotal = 0.0f;

	/// ID of current cooldown spot (empty if not in one)
	UPROPERTY()
	FString CurrentCooldownSpotId;

	/// Whether a game session is active
	UPROPERTY()
	bool bSessionActive = false;

	/// Timer for per-tick updates
	FTimerHandle PursuitTickTimer;
};

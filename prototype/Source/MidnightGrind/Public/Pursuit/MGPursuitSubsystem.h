// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file MGPursuitSubsystem.h
 * @brief Core pursuit/police chase system for Midnight Grind
 *
 * This subsystem manages all aspects of police pursuits including:
 * - Active police unit spawning, tracking, and AI coordination
 * - Pursuit state machine (searching, engaged, cooldown, escaped, busted)
 * - Tactical responses (roadblocks, spike strips, helicopters, PIT maneuvers)
 * - Escape and bust meter mechanics
 * - Bounty accumulation during pursuits
 * - Session statistics and scoring
 *
 * The pursuit system works closely with the Heat Level Subsystem to determine
 * police response intensity. Higher heat levels trigger more aggressive tactics
 * and spawn more advanced police unit types.
 *
 * @note This is a GameInstanceSubsystem, meaning it persists across level loads
 *       and is shared by all players in the game instance.
 *
 * @see UMGHeatLevelSubsystem for heat/wanted level management
 * @see UMGBountySubsystem for bounty reward handling
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPursuitSubsystem.generated.h"

//=============================================================================
// Pursuit Enumerations
//=============================================================================

/**
 * @brief Defines the role of a participant in a pursuit
 *
 * Used to categorize both player vehicles (Runner) and various police units.
 * Each role has different AI behaviors and capabilities.
 */
UENUM(BlueprintType)
enum class EMGPursuitRole : uint8
{
	/// No active role in pursuit
	None				UMETA(DisplayName = "None"),
	/// The player being chased - trying to escape
	Runner				UMETA(DisplayName = "Runner/Escapee"),
	/// Standard police unit actively chasing the runner
	Pursuer				UMETA(DisplayName = "Pursuer/Cop"),
	/// High-speed police unit that attempts to cut off the runner
	Interceptor			UMETA(DisplayName = "Interceptor"),
	/// Aerial unit providing tracking and coordination from above
	Helicopter			UMETA(DisplayName = "Helicopter"),
	/// Stationary police units forming a blockade
	RoadBlock			UMETA(DisplayName = "Road Block")
};

/**
 * @brief Current state of a pursuit for a specific player
 *
 * Represents the pursuit state machine. Transitions typically flow:
 * Inactive -> Searching -> Sighted -> Engaged -> PursuitActive -> (Escaped/Busted) -> Cooldown -> Inactive
 */
UENUM(BlueprintType)
enum class EMGPursuitState : uint8
{
	/// No active pursuit - player is free
	Inactive			UMETA(DisplayName = "Inactive"),
	/// Police are looking for the player but don't have visual
	Searching			UMETA(DisplayName = "Searching"),
	/// Police have spotted the player but pursuit not yet engaged
	Sighted				UMETA(DisplayName = "Sighted"),
	/// Police are actively engaging the player
	Engaged				UMETA(DisplayName = "Engaged"),
	/// Full pursuit mode - all units actively chasing
	PursuitActive		UMETA(DisplayName = "Pursuit Active"),
	/// Player was caught - pursuit ended in failure
	Busted				UMETA(DisplayName = "Busted"),
	/// Player successfully evaded all units
	Escaped				UMETA(DisplayName = "Escaped"),
	/// Grace period after escaping before heat fully clears
	Cooldown			UMETA(DisplayName = "Cooldown")
};

/**
 * @brief Tactical maneuvers available to police units
 *
 * Different tactics become available at higher pursuit intensities.
 * More aggressive tactics deal more damage but are easier to evade.
 */
UENUM(BlueprintType)
enum class EMGPursuitTactic : uint8
{
	/// Basic following behavior - maintain pursuit distance
	Follow				UMETA(DisplayName = "Follow"),
	/// Aggressive ramming to damage player vehicle
	Ram					UMETA(DisplayName = "Ram"),
	/// Precision Immobilization Technique - spin out the player
	PitManeuver			UMETA(DisplayName = "PIT Maneuver"),
	/// Multiple units coordinate to surround the player
	BoxIn				UMETA(DisplayName = "Box In"),
	/// Deploy a stationary roadblock ahead of player
	Roadblock			UMETA(DisplayName = "Roadblock"),
	/// Deploy tire-puncturing strips on the road
	SpikeStrip			UMETA(DisplayName = "Spike Strip"),
	/// Call in helicopter support for tracking
	Helicopter			UMETA(DisplayName = "Helicopter"),
	/// Electromagnetic pulse to disable vehicle electronics
	EMPDisable			UMETA(DisplayName = "EMP Disable"),
	/// Sharpshooter targeting player's tires
	TireShot			UMETA(DisplayName = "Tire Shot")
};

/**
 * @brief Overall intensity level of the pursuit
 *
 * Intensity affects the number of units deployed, available tactics,
 * and the aggressiveness of police AI. Intensity typically increases
 * over time during a pursuit based on player actions.
 */
UENUM(BlueprintType)
enum class EMGPursuitIntensity : uint8
{
	/// 1-2 patrol cars, basic follow tactics only
	Low					UMETA(DisplayName = "Low"),
	/// 3-4 units, ramming enabled
	Medium				UMETA(DisplayName = "Medium"),
	/// 5-6 units, roadblocks and spike strips
	High				UMETA(DisplayName = "High"),
	/// 7+ units, helicopter support, aggressive tactics
	Extreme				UMETA(DisplayName = "Extreme"),
	/// All available units, all tactics, SWAT/heavy units
	Maximum				UMETA(DisplayName = "Maximum")
};

/**
 * @brief Types of events that occur during a pursuit
 *
 * Used for event logging, UI notifications, and triggering
 * audio/visual feedback during pursuits.
 */
UENUM(BlueprintType)
enum class EMGPursuitEventType : uint8
{
	/// Player was spotted by police
	Spotted				UMETA(DisplayName = "Spotted"),
	/// Additional police units called in
	BackupCalled		UMETA(DisplayName = "Backup Called"),
	/// A roadblock has been set up ahead
	RoadblockDeployed	UMETA(DisplayName = "Roadblock Deployed"),
	/// Spike strips placed on the road
	SpikeStripDeployed	UMETA(DisplayName = "Spike Strip Deployed"),
	/// Helicopter has been dispatched
	HelicopterCalled	UMETA(DisplayName = "Helicopter Called"),
	/// Player disabled a police unit
	PursuerTakenDown	UMETA(DisplayName = "Pursuer Taken Down"),
	/// Player is close to escaping
	NearEscape			UMETA(DisplayName = "Near Escape"),
	/// Escape cooldown period started
	CooldownStarted		UMETA(DisplayName = "Cooldown Started"),
	/// Pursuit intensity has increased
	IntensityIncreased	UMETA(DisplayName = "Intensity Increased")
};

//=============================================================================
// Pursuit Data Structures
//=============================================================================

/**
 * @brief Data for a single police unit participating in a pursuit
 *
 * Contains all runtime information needed to track and manage
 * an individual police vehicle or helicopter during a pursuit.
 */
USTRUCT(BlueprintType)
struct FMGPursuitUnit
{
	GENERATED_BODY()

	/// Unique identifier for this unit instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnitId;

	/// What role this unit plays in the pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitRole Role = EMGPursuitRole::Pursuer;

	/// Current world location of the unit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Current facing direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/// Current movement velocity vector
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	/// Player ID this unit is targeting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerId;

	/// The tactic this unit is currently executing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitTactic CurrentTactic = EMGPursuitTactic::Follow;

	/// Distance in units to the target player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToTarget = 0.0f;

	/// Current health points (disabled when reaches 0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	/// Maximum health for this unit type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	/// True if unit has been wrecked/disabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDisabled = false;

	/// True if unit currently has line-of-sight to target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasVisual = false;

	/// Seconds this unit has been actively pursuing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeEngaged = 0.0f;

	/// Total damage this unit has dealt to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DamageDealt = 0;

	/// Reference to the unit's visual/gameplay asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> UnitAsset;
};

/**
 * @brief Complete pursuit status for a single player
 *
 * This struct contains all information about a player's current pursuit,
 * including state, intensity, active units, and progress meters.
 */
USTRUCT(BlueprintType)
struct FMGPursuitStatus
{
	GENERATED_BODY()

	/// The player this pursuit status belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	/// Current state in the pursuit state machine
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitState State = EMGPursuitState::Inactive;

	/// Current intensity level of the pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitIntensity Intensity = EMGPursuitIntensity::Low;

	/// All police units currently in this pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPursuitUnit> ActiveUnits;

	/// Total units that have engaged since pursuit started
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalUnitsEngaged = 0;

	/// Number of units the player has disabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitsDisabled = 0;

	/// How long the pursuit has been active in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PursuitDuration = 0.0f;

	/// Time remaining in cooldown period (0 if not in cooldown)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	/// Progress toward escape (0-100, escape at 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeMeter = 0.0f;

	/// Progress toward being busted (0-100, busted at threshold)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustedMeter = 0.0f;

	/// Whether a helicopter is currently tracking this player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHelicopterActive = false;

	/// Number of roadblocks successfully evaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoadblocksEvaded = 0;

	/// Number of spike strips successfully evaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpikeStripsEvaded = 0;

	/// Multiplier applied to infraction costs (increases over time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InfractionMultiplier = 1.0f;

	/// Current bounty value accumulated during this pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentBounty = 0;
};

/**
 * @brief Configuration data for a deployed roadblock
 *
 * Roadblocks are stationary obstacles set up ahead of the player
 * to force them to slow down, take damage, or find alternate routes.
 */
USTRUCT(BlueprintType)
struct FMGRoadblock
{
	GENERATED_BODY()

	/// Unique identifier for this roadblock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoadblockId;

	/// World position of the roadblock center
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Rotation/facing of the roadblock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/// Width of the roadblock in units (how much road it covers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width = 1000.0f;

	/// Number of police vehicles in this roadblock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumUnits = 2;

	/// Whether spike strips are part of this roadblock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSpikeStrip = false;

	/// Distance from roadblock center to spike strip placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpikeStripOffset = 0.0f;

	/// Whether the roadblock is fully set up and dangerous
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	/// Seconds until roadblock becomes active (setup time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilActive = 0.0f;

	/// True if player has passed this roadblock without hitting it
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasBeenEvaded = false;

	/// Number of vehicles damaged by this roadblock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VehiclesDamaged = 0;
};

/**
 * @brief Record of a single event that occurred during a pursuit
 *
 * Used for pursuit history, replay systems, and post-pursuit summaries.
 */
USTRUCT(BlueprintType)
struct FMGPursuitEvent
{
	GENERATED_BODY()

	/// Type of event that occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitEventType Type = EMGPursuitEventType::Spotted;

	/// Human-readable description of the event
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	/// World location where the event occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Game time when event occurred (seconds since pursuit start)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	/// Change in bounty caused by this event (can be positive or negative)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BountyChange = 0;

	/// ID of the police unit involved in this event (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InvolvedUnitId;
};

//=============================================================================
// Scoring and Configuration Structures
//=============================================================================

/**
 * @brief Scoring values for pursuit-related actions
 *
 * Defines the point values and multipliers awarded for various
 * pursuit achievements. Used to calculate final escape bonuses.
 */
USTRUCT(BlueprintType)
struct FMGPursuitScoring
{
	GENERATED_BODY()

	/// Base points awarded for any successful escape
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseEscapeBonus = 1000;

	/// Points per police unit disabled during pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerUnitDisabledBonus = 500;

	/// Points per roadblock successfully evaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerRoadblockEvadedBonus = 750;

	/// Points per spike strip successfully evaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerSpikeStripEvadedBonus = 300;

	/// Multiplier increase per minute of pursuit duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationMultiplierPerMinute = 0.25f;

	/// Multiplier based on final pursuit intensity level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IntensityMultiplier = 1.5f;

	/// Bonus for escaping without taking any damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanEscapeBonus = 2000;

	/// Points for near-miss maneuvers during pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NearMissBonus = 100;

	/// Bonus for escaping while helicopter was active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HelicopterEvadeBonus = 1500;
};

/**
 * @brief Configuration parameters for pursuit behavior
 *
 * Tweakable values that control escape timing, bust mechanics,
 * and intensity scaling. Can be adjusted for difficulty balancing.
 */
USTRUCT(BlueprintType)
struct FMGPursuitConfig
{
	GENERATED_BODY()

	/// Base seconds required out of sight to escape
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseEscapeTime = 15.0f;

	/// Busted meter value at which player is caught
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustedThreshold = 100.0f;

	/// How fast busted meter fills when stopped near police
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustedFillRate = 25.0f;

	/// How fast escape meter fills when out of sight
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeFillRate = 10.0f;

	/// How fast escape meter drains when spotted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeDrainRate = 5.0f;

	/// Minimum distance from all units to start escape progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistanceForEscape = 5000.0f;

	/// Duration of cooldown period after escaping
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownDuration = 10.0f;

	/// Maximum distance units can see the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VisualRange = 15000.0f;

	/// Maximum police units allowed at each intensity level
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPursuitIntensity, int32> MaxUnitsPerIntensity;

	/// Threshold values that trigger intensity upgrades
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPursuitIntensity, float> IntensityUpgradeThresholds;

	/// List of tactics enabled for this configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPursuitTactic> AvailableTactics;
};

/**
 * @brief Session-wide pursuit statistics
 *
 * Tracks cumulative stats across all pursuits in the current game session.
 * Reset when starting a new session or manually via ResetSessionStats().
 */
USTRUCT(BlueprintType)
struct FMGPursuitSessionStats
{
	GENERATED_BODY()

	/// Total pursuits initiated this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPursuitsStarted = 0;

	/// Number of successful escapes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalEscapes = 0;

	/// Number of times player was busted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBusted = 0;

	/// Total police units disabled across all pursuits
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalUnitsDisabled = 0;

	/// Total roadblocks evaded across all pursuits
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRoadblocksEvaded = 0;

	/// Total spike strips evaded across all pursuits
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalSpikeStripsEvaded = 0;

	/// Longest single pursuit duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestPursuitDuration = 0.0f;

	/// Highest bounty achieved in a single pursuit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestBounty = 0;

	/// Total bounty collected from escapes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountyEarned = 0;

	/// Total bounty lost from being busted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountyLost = 0;

	/// Highest intensity level reached
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitIntensity HighestIntensity = EMGPursuitIntensity::Low;

	/// Most units engaged at one time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MostUnitsEngagedAtOnce = 0;
};

//=============================================================================
// Event Delegates
//=============================================================================

/// Broadcast when a pursuit begins for a player
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitStarted, const FString&, PlayerId, EMGPursuitIntensity, Intensity);

/// Broadcast when a pursuit ends (escaped or busted)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPursuitEnded, const FString&, PlayerId, bool, bEscaped, int32, FinalBounty);

/// Broadcast when pursuit intensity changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPursuitIntensityChanged, const FString&, PlayerId, EMGPursuitIntensity, OldIntensity, EMGPursuitIntensity, NewIntensity);

/// Broadcast when pursuit state transitions
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitStateChanged, const FString&, PlayerId, EMGPursuitState, NewState);

/// Broadcast when a new police unit joins the pursuit
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitEngaged, const FString&, PlayerId, const FMGPursuitUnit&, Unit);

/// Broadcast when a police unit is disabled
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitDisabled, const FString&, PlayerId, const FMGPursuitUnit&, Unit);

/// Broadcast when a roadblock is deployed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoadblockDeployed, const FString&, PlayerId, const FMGRoadblock&, Roadblock);

/// Broadcast when a roadblock is evaded
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoadblockEvaded, const FString&, PlayerId, const FString&, RoadblockId);

/// Broadcast when helicopter support is called in
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHelicopterCalled, const FString&, PlayerId);

/// Broadcast when player escapes helicopter tracking
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHelicopterEvaded, const FString&, PlayerId);

/// Broadcast when bounty amount changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBountyChanged, const FString&, PlayerId, int32, NewBounty);

/// Broadcast when cooldown period begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownStarted, const FString&, PlayerId, float, CooldownTime);

/// Broadcast when escape meter value changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEscapeMeterChanged, const FString&, PlayerId, float, NewValue);

/// Broadcast when busted meter value changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBustedMeterChanged, const FString&, PlayerId, float, NewValue);

/// Broadcast when any pursuit event occurs (for logging/UI)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitEvent, const FString&, PlayerId, const FMGPursuitEvent&, Event);

//=============================================================================
// Main Pursuit Subsystem Class
//=============================================================================

/**
 * @brief Main subsystem managing police pursuits and escape mechanics
 *
 * The Pursuit Subsystem is the central controller for all police chase gameplay.
 * It coordinates between the heat level system, individual police units, and
 * the player to create dynamic pursuit experiences.
 *
 * Key responsibilities:
 * - Starting/ending pursuits based on heat level thresholds
 * - Spawning and managing police unit AI
 * - Tracking escape and bust progress
 * - Deploying tactical responses (roadblocks, helicopters, etc.)
 * - Calculating and awarding bounty rewards
 *
 * @note Access via UGameInstance::GetSubsystem<UMGPursuitSubsystem>()
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPursuitSubsystem : public UGameInstanceSubsystem
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
	// Subscribe to these to receive notifications about pursuit changes
	//=========================================================================

	/// Fired when a new pursuit begins
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitStarted OnPursuitStarted;

	/// Fired when a pursuit concludes
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitEnded OnPursuitEnded;

	/// Fired when intensity escalates or de-escalates
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitIntensityChanged OnPursuitIntensityChanged;

	/// Fired on any pursuit state machine transition
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitStateChanged OnPursuitStateChanged;

	/// Fired when a new police unit joins the chase
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnUnitEngaged OnUnitEngaged;

	/// Fired when player wrecks a police unit
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnUnitDisabled OnUnitDisabled;

	/// Fired when police set up a roadblock
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnRoadblockDeployed OnRoadblockDeployed;

	/// Fired when player passes a roadblock without hitting it
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnRoadblockEvaded OnRoadblockEvaded;

	/// Fired when helicopter joins the pursuit
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnHelicopterCalled OnHelicopterCalled;

	/// Fired when player loses helicopter tracking
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnHelicopterEvaded OnHelicopterEvaded;

	/// Fired whenever bounty value changes
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnBountyChanged OnBountyChanged;

	/// Fired when escape cooldown begins
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnCooldownStarted OnCooldownStarted;

	/// Fired when escape meter changes
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnEscapeMeterChanged OnEscapeMeterChanged;

	/// Fired when bust meter changes
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnBustedMeterChanged OnBustedMeterChanged;

	/// Fired for all pursuit events (useful for logging/UI)
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitEvent OnPursuitEvent;

	//=========================================================================
	// Pursuit Control
	// Functions to start, end, and modify active pursuits
	//=========================================================================

	/**
	 * @brief Initiates a new pursuit for the specified player
	 * @param PlayerId The player to start pursuing
	 * @param InitialIntensity Starting intensity level (default: Low)
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Control")
	void StartPursuit(const FString& PlayerId, EMGPursuitIntensity InitialIntensity = EMGPursuitIntensity::Low);

	/**
	 * @brief Ends the current pursuit for a player
	 * @param PlayerId The player whose pursuit is ending
	 * @param bEscaped True if player escaped, false if busted
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Control")
	void EndPursuit(const FString& PlayerId, bool bEscaped);

	/**
	 * @brief Directly sets pursuit intensity level
	 * @param PlayerId Target player
	 * @param Intensity New intensity level to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Control")
	void SetPursuitIntensity(const FString& PlayerId, EMGPursuitIntensity Intensity);

	/**
	 * @brief Increases pursuit intensity by one level
	 * @param PlayerId Target player
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Control")
	void IncreaseIntensity(const FString& PlayerId);

	/**
	 * @brief Checks if a player has an active pursuit
	 * @param PlayerId Player to check
	 * @return True if pursuit is active (not Inactive or Cooldown)
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Control")
	bool IsPursuitActive(const FString& PlayerId) const;

	/**
	 * @brief Checks if player is in post-escape cooldown
	 * @param PlayerId Player to check
	 * @return True if in cooldown period
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Control")
	bool IsInCooldown(const FString& PlayerId) const;

	//=========================================================================
	// Status Queries
	// Functions to read current pursuit state
	//=========================================================================

	/**
	 * @brief Gets complete pursuit status for a player
	 * @param PlayerId Player to query
	 * @return Full pursuit status struct
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	FMGPursuitStatus GetPursuitStatus(const FString& PlayerId) const;

	/**
	 * @brief Gets current pursuit state machine state
	 * @param PlayerId Player to query
	 * @return Current EMGPursuitState value
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	EMGPursuitState GetPursuitState(const FString& PlayerId) const;

	/**
	 * @brief Gets current pursuit intensity level
	 * @param PlayerId Player to query
	 * @return Current intensity level
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	EMGPursuitIntensity GetPursuitIntensity(const FString& PlayerId) const;

	/**
	 * @brief Gets escape meter progress (0-100)
	 * @param PlayerId Player to query
	 * @return Current escape meter value
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	float GetEscapeMeter(const FString& PlayerId) const;

	/**
	 * @brief Gets bust meter progress (0-threshold)
	 * @param PlayerId Player to query
	 * @return Current bust meter value
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	float GetBustedMeter(const FString& PlayerId) const;

	/**
	 * @brief Gets current bounty value for active pursuit
	 * @param PlayerId Player to query
	 * @return Current bounty amount
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	int32 GetBounty(const FString& PlayerId) const;

	/**
	 * @brief Gets remaining cooldown time
	 * @param PlayerId Player to query
	 * @return Seconds remaining in cooldown (0 if not in cooldown)
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	float GetCooldownRemaining(const FString& PlayerId) const;

	//=========================================================================
	// Police Unit Management
	// Functions to spawn, track, and control police units
	//=========================================================================

	/**
	 * @brief Spawns a new police unit to join the pursuit
	 * @param PlayerId Player to pursue
	 * @param Role Type of unit to spawn
	 * @param SpawnLocation World position to spawn at
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Units")
	void SpawnPursuitUnit(const FString& PlayerId, EMGPursuitRole Role, FVector SpawnLocation);

	/**
	 * @brief Removes a specific unit from the pursuit
	 * @param PlayerId Target player's pursuit
	 * @param UnitId Unit to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Units")
	void RemovePursuitUnit(const FString& PlayerId, const FString& UnitId);

	/**
	 * @brief Deals damage to a unit, potentially disabling it
	 * @param PlayerId Target player's pursuit
	 * @param UnitId Unit to damage
	 * @param Damage Amount of damage to deal
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Units")
	void DisableUnit(const FString& PlayerId, const FString& UnitId, float Damage);

	/**
	 * @brief Gets all active units in a pursuit
	 * @param PlayerId Player to query
	 * @return Array of all active pursuit units
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Units")
	TArray<FMGPursuitUnit> GetActiveUnits(const FString& PlayerId) const;

	/**
	 * @brief Gets count of active (non-disabled) units
	 * @param PlayerId Player to query
	 * @return Number of active units
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Units")
	int32 GetActiveUnitCount(const FString& PlayerId) const;

	/**
	 * @brief Finds the nearest unit to a location
	 * @param PlayerId Player to query
	 * @param Location Position to measure from
	 * @return The closest pursuit unit
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Units")
	FMGPursuitUnit GetClosestUnit(const FString& PlayerId, FVector Location) const;

	//=========================================================================
	// Tactical Systems
	// Functions for roadblocks, spike strips, and helicopter
	//=========================================================================

	/**
	 * @brief Deploys a new roadblock
	 * @param PlayerId Player the roadblock targets
	 * @param Roadblock Configuration for the roadblock
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void DeployRoadblock(const FString& PlayerId, const FMGRoadblock& Roadblock);

	/**
	 * @brief Marks a roadblock as evaded (player passed without hitting)
	 * @param PlayerId Player who evaded
	 * @param RoadblockId ID of the evaded roadblock
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void EvadeRoadblock(const FString& PlayerId, const FString& RoadblockId);

	/**
	 * @brief Called when player hits a roadblock
	 * @param PlayerId Player who hit
	 * @param RoadblockId ID of the hit roadblock
	 * @param Damage Damage dealt to player
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void HitRoadblock(const FString& PlayerId, const FString& RoadblockId, float Damage);

	/**
	 * @brief Calls in helicopter support
	 * @param PlayerId Player to track
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void CallHelicopter(const FString& PlayerId);

	/**
	 * @brief Marks helicopter as evaded (player escaped tracking)
	 * @param PlayerId Player who evaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void EvadeHelicopter(const FString& PlayerId);

	/**
	 * @brief Called when player hits a spike strip
	 * @param PlayerId Player who hit the spike strip
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void HitSpikeStrip(const FString& PlayerId);

	/**
	 * @brief Gets all active roadblocks for a pursuit
	 * @param PlayerId Player to query
	 * @return Array of active roadblock data
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Tactics")
	TArray<FMGRoadblock> GetActiveRoadblocks(const FString& PlayerId) const;

	//=========================================================================
	// Bounty System
	// Functions for bounty accumulation and collection
	//=========================================================================

	/**
	 * @brief Adds bounty value to player's pursuit
	 * @param PlayerId Target player
	 * @param Amount Bounty to add
	 * @param Reason Optional description for logging
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Bounty")
	void AddBounty(const FString& PlayerId, int32 Amount, const FString& Reason = TEXT(""));

	/**
	 * @brief Resets bounty to zero (called on bust)
	 * @param PlayerId Target player
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Bounty")
	void ResetBounty(const FString& PlayerId);

	/**
	 * @brief Collects accumulated bounty (called on escape)
	 * @param PlayerId Target player
	 * @return Bounty amount collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Bounty")
	int32 CollectBounty(const FString& PlayerId);

	//=========================================================================
	// Per-Frame Updates
	// Functions called to update pursuit state each tick
	//=========================================================================

	/**
	 * @brief Main pursuit update - call each frame
	 * @param PlayerId Player to update
	 * @param PlayerLocation Current player position
	 * @param PlayerVelocity Current player velocity
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Update")
	void UpdatePursuit(const FString& PlayerId, FVector PlayerLocation, FVector PlayerVelocity, float DeltaTime);

	/**
	 * @brief Updates AI for all units in a pursuit
	 * @param PlayerId Target pursuit
	 * @param DeltaTime Time since last update
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Update")
	void UpdateUnitAI(const FString& PlayerId, float DeltaTime);

	//=========================================================================
	// Configuration
	// Functions to get/set pursuit system parameters
	//=========================================================================

	/**
	 * @brief Sets pursuit configuration parameters
	 * @param Config New configuration to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Config")
	void SetPursuitConfig(const FMGPursuitConfig& Config);

	/**
	 * @brief Gets current pursuit configuration
	 * @return Current config struct
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Config")
	FMGPursuitConfig GetPursuitConfig() const;

	/**
	 * @brief Sets scoring parameters
	 * @param Scoring New scoring values to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Config")
	void SetPursuitScoring(const FMGPursuitScoring& Scoring);

	/**
	 * @brief Gets current scoring configuration
	 * @return Current scoring struct
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Config")
	FMGPursuitScoring GetPursuitScoring() const;

	//=========================================================================
	// Statistics
	// Functions for session statistics tracking
	//=========================================================================

	/**
	 * @brief Gets cumulative session statistics
	 * @return Session stats struct
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Stats")
	FMGPursuitSessionStats GetSessionStats() const;

	/**
	 * @brief Resets all session statistics to zero
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Stats")
	void ResetSessionStats();

	//=========================================================================
	// Scoring Calculations
	// Functions to calculate final pursuit scores
	//=========================================================================

	/**
	 * @brief Calculates total escape score based on pursuit performance
	 * @param PlayerId Player to calculate for
	 * @return Total escape score
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Scoring")
	int32 CalculateEscapeScore(const FString& PlayerId) const;

	/**
	 * @brief Calculates penalty for being busted
	 * @param PlayerId Player to calculate for
	 * @return Penalty amount
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Scoring")
	int32 CalculateBustedPenalty(const FString& PlayerId) const;

	//=========================================================================
	// Event History
	// Functions to access pursuit event logs
	//=========================================================================

	/**
	 * @brief Gets all events from current/last pursuit
	 * @param PlayerId Player to query
	 * @return Array of pursuit events in chronological order
	 */
	UFUNCTION(BlueprintPure, Category = "Pursuit|Events")
	TArray<FMGPursuitEvent> GetPursuitEvents(const FString& PlayerId) const;

	//=========================================================================
	// Persistence
	// Functions for save/load functionality
	//=========================================================================

	/**
	 * @brief Saves current pursuit data to disk
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Persistence")
	void SavePursuitData();

	/**
	 * @brief Loads pursuit data from disk
	 */
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Persistence")
	void LoadPursuitData();

protected:
	//=========================================================================
	// Internal Update Functions
	//=========================================================================

	/// Called on timer to update all active pursuits
	void TickPursuit(float DeltaTime);

	/// Updates escape meter based on player visibility
	void UpdateEscapeProgress(const FString& PlayerId, FVector PlayerLocation, float DeltaTime);

	/// Updates bust meter based on player proximity to units
	void UpdateBustedProgress(const FString& PlayerId, float DeltaTime);

	/// Checks if conditions are met to increase intensity
	void CheckIntensityUpgrade(const FString& PlayerId);

	/// Spawns additional units as backup
	void SpawnBackup(const FString& PlayerId);

	/// Internal state transition handler
	void SetPursuitState(const FString& PlayerId, EMGPursuitState NewState);

	/// Records an event to the pursuit history
	void RecordEvent(const FString& PlayerId, EMGPursuitEventType Type, const FString& Description, FVector Location);

	/// Generates a unique ID for new units
	FString GenerateUnitId() const;

private:
	//=========================================================================
	// Private Data Members
	//=========================================================================

	/// Map of player ID to their active pursuit status
	UPROPERTY()
	TMap<FString, FMGPursuitStatus> ActivePursuits;

	/// Map of player ID to their active roadblocks
	UPROPERTY()
	TMap<FString, TArray<FMGRoadblock>> ActiveRoadblocks;

	/// Map of player ID to their pursuit event history
	UPROPERTY()
	TMap<FString, TArray<FMGPursuitEvent>> PursuitEvents;

	/// Current pursuit configuration parameters
	UPROPERTY()
	FMGPursuitConfig PursuitConfig;

	/// Current scoring configuration
	UPROPERTY()
	FMGPursuitScoring PursuitScoring;

	/// Cumulative session statistics
	UPROPERTY()
	FMGPursuitSessionStats SessionStats;

	/// Counter for generating unique unit IDs
	UPROPERTY()
	int32 UnitCounter = 0;

	/// Timer handle for periodic pursuit updates
	FTimerHandle PursuitTickTimer;
};

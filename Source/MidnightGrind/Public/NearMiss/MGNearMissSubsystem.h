// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGNearMissSubsystem.h
 * @brief Near Miss Detection and Scoring Subsystem
 *
 * This subsystem manages the near miss gameplay mechanic in Midnight Grind.
 * Near misses occur when the player narrowly avoids collision with obstacles,
 * traffic, or other hazards, rewarding skillful and risky driving.
 *
 * Key Features:
 * - Real-time proximity detection for various target types
 * - Quality tiers based on how close the near miss was
 * - Combo system for chaining multiple near misses
 * - Style bonuses for combining near misses with other actions
 * - Frenzy mode for extended combo streaks
 * - Configurable thresholds per target type
 * - Rewards system (cash, reputation, nitro)
 *
 * How Near Misses Work:
 * 1. System tracks proximity targets (traffic, obstacles, walls)
 * 2. When player passes within threshold distance, near miss triggers
 * 3. Quality determined by how close: Basic -> Good -> Great -> Perfect -> Insane
 * 4. Points awarded based on quality, speed, and active bonuses
 * 5. Combos build multiplier for consecutive near misses
 * 6. Frenzy mode activates at high combo counts for massive points
 *
 * @see FMGNearMissEvent for individual near miss data
 * @see FMGActiveCombo for combo state
 * @see FMGNearMissThresholds for distance configuration
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNearMissSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Type of object involved in a near miss.
 *
 * Different types have different distance thresholds and point values.
 */
UENUM(BlueprintType)
enum class EMGNearMissType : uint8
{
	Vehicle			UMETA(DisplayName = "Vehicle Near Miss"),    ///< AI or player vehicle
	Traffic			UMETA(DisplayName = "Traffic Near Miss"),    ///< Generic traffic car
	Oncoming		UMETA(DisplayName = "Oncoming Traffic"),     ///< Wrong-way traffic (bonus)
	Pedestrian		UMETA(DisplayName = "Pedestrian Close Call"),///< Pedestrian (rare)
	Obstacle		UMETA(DisplayName = "Obstacle Near Miss"),   ///< Static obstacle
	Wall			UMETA(DisplayName = "Wall Graze"),           ///< Track boundary
	Barrier			UMETA(DisplayName = "Barrier Scrape"),       ///< Guardrail/barrier
	Cliff			UMETA(DisplayName = "Cliff Edge"),           ///< Near falling off edge
	Train			UMETA(DisplayName = "Train Dodge"),          ///< Train crossing
	Police			UMETA(DisplayName = "Police Evade")          ///< Police vehicle
};

/**
 * @brief Quality tier of a near miss based on distance.
 *
 * Closer distance = higher quality = more points.
 */
UENUM(BlueprintType)
enum class EMGNearMissQuality : uint8
{
	Basic			UMETA(DisplayName = "Close"),            ///< Standard near miss
	Good			UMETA(DisplayName = "Very Close"),       ///< Better timing
	Great			UMETA(DisplayName = "Dangerously Close"),///< Skilled maneuver
	Perfect			UMETA(DisplayName = "Hair's Breadth"),   ///< Expert level
	Insane			UMETA(DisplayName = "Impossible")        ///< Frame-perfect, legendary
};

/**
 * @brief Categories for style point bonuses.
 *
 * Style bonuses stack when performing multiple actions simultaneously.
 */
UENUM(BlueprintType)
enum class EMGStyleCategory : uint8
{
	NearMiss		UMETA(DisplayName = "Near Miss"),   ///< Basic near miss points
	Drift			UMETA(DisplayName = "Drift"),       ///< Drifting while near missing
	Air				UMETA(DisplayName = "Air Time"),    ///< Airborne near miss
	Speed			UMETA(DisplayName = "High Speed"),  ///< Speed threshold bonus
	Combo			UMETA(DisplayName = "Combo"),       ///< Chain bonus
	Skill			UMETA(DisplayName = "Skill Move")   ///< Special maneuver
};

/**
 * @brief Current state of the combo system.
 *
 * Combos progress through states as near misses chain together.
 */
UENUM(BlueprintType)
enum class EMGComboState : uint8
{
	Inactive		UMETA(DisplayName = "Inactive"),   ///< No combo active
	Building		UMETA(DisplayName = "Building"),   ///< Combo started, building up
	Active			UMETA(DisplayName = "Active"),     ///< Combo in full swing
	Frenzy			UMETA(DisplayName = "Frenzy"),     ///< High combo, bonus multiplier
	Expiring		UMETA(DisplayName = "Expiring"),   ///< Timer running out
	Banked			UMETA(DisplayName = "Banked")      ///< Points secured
};

//=============================================================================
// Data Structures - Events
//=============================================================================

/**
 * @brief Data for a single near miss event.
 *
 * Created each time a near miss is registered.
 */
USTRUCT(BlueprintType)
struct FMGNearMissEvent
{
	GENERATED_BODY()

	/// Unique identifier for this event
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	/// Type of target involved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNearMissType MissType = EMGNearMissType::Vehicle;

	/// Quality tier based on distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNearMissQuality Quality = EMGNearMissQuality::Basic;

	/// Closest distance to target in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	/// Player speed at time of near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	/// Combined closing speed (player + target)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RelativeSpeed = 0.0f;

	/// Points before multipliers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 0;

	/// Final points after all multipliers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MultipliedPoints = 0;

	/// Active combo multiplier when event occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplier = 1.0f;

	/// World position where near miss occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// When the near miss occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/// ID of the target object
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	/// True if player was drifting during near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasDrifting = false;

	/// True if player was airborne during near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasAirborne = false;

	/// True if target was oncoming (wrong way)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasOncoming = false;
};

//=============================================================================
// Data Structures - Configuration
//=============================================================================

/**
 * @brief Distance thresholds for near miss quality determination.
 *
 * Each target type can have different thresholds.
 */
USTRUCT(BlueprintType)
struct FMGNearMissThresholds
{
	GENERATED_BODY()

	/// Target type these thresholds apply to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNearMissType MissType = EMGNearMissType::Vehicle;

	/// Maximum distance for Basic quality (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasicDistance = 3.0f;

	/// Maximum distance for Good quality (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodDistance = 2.0f;

	/// Maximum distance for Great quality (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GreatDistance = 1.0f;

	/// Maximum distance for Perfect quality (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectDistance = 0.5f;

	/// Maximum distance for Insane quality (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InsaneDistance = 0.25f;

	/// Base points for this target type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 100;

	/// Multiplier for Good quality
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodMultiplier = 1.5f;

	/// Multiplier for Great quality
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GreatMultiplier = 2.0f;

	/// Multiplier for Perfect quality
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectMultiplier = 3.0f;

	/// Multiplier for Insane quality
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InsaneMultiplier = 5.0f;
};

//=============================================================================
// Data Structures - Combo System
//=============================================================================

/**
 * @brief Current state of an active combo chain.
 *
 * Combos build multiplier and lead to frenzy mode.
 */
USTRUCT(BlueprintType)
struct FMGActiveCombo
{
	GENERATED_BODY()

	/// Current combo state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGComboState State = EMGComboState::Inactive;

	/// Number of near misses in current combo
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboCount = 0;

	/// Total points earned in this combo
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	/// Current combo multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMultiplier = 1.0f;

	/// Seconds until combo expires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	/// Maximum time allowed between near misses
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTime = 3.0f;

	/// All events in this combo chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGNearMissEvent> ComboEvents;

	/// Highest combo count reached before banking
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxComboReached = 0;

	/// True if combo includes a drift near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasDrift = false;

	/// True if combo includes an airborne near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasAir = false;

	/// True if combo includes an oncoming near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasOncoming = false;
};

//=============================================================================
// Data Structures - Style Bonuses
//=============================================================================

/**
 * @brief Definition of a style bonus modifier.
 *
 * Style bonuses provide extra points when conditions are met.
 */
USTRUCT(BlueprintType)
struct FMGStyleBonus
{
	GENERATED_BODY()

	/// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BonusId;

	/// Display name shown to player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BonusName;

	/// Category of this bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStyleCategory Category = EMGStyleCategory::NearMiss;

	/// Score multiplier when bonus applies
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	/// Flat points added when bonus applies
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FlatBonus = 0;

	/// True if drifting is required
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresDrift = false;

	/// True if airborne is required
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresAirborne = false;

	/// True if oncoming traffic is required
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresOncoming = false;

	/// Minimum speed required (0 = no requirement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeed = 0.0f;

	/// Minimum combo count required (0 = no requirement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCombo = 0;
};

//=============================================================================
// Data Structures - Statistics
//=============================================================================

/**
 * @brief Statistics tracked during a gameplay session.
 */
USTRUCT(BlueprintType)
struct FMGStyleSessionStats
{
	GENERATED_BODY()

	/// Total near misses this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalNearMisses = 0;

	/// Total style points earned
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalStylePoints = 0;

	/// Highest combo achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestCombo = 0;

	/// Highest points from single near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestSingleEvent = 0;

	/// Closest near miss distance in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClosestDistance = FLT_MAX;

	/// Count of near misses by type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGNearMissType, int32> NearMissByType;

	/// Count of near misses by quality
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGNearMissQuality, int32> NearMissByQuality;

	/// Number of Perfect quality near misses
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectMisses = 0;

	/// Number of Insane quality near misses
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InsaneMisses = 0;

	/// Near misses against oncoming traffic
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OncomingMisses = 0;

	/// Near misses while drifting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DriftNearMisses = 0;

	/// Near misses while airborne
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AirNearMisses = 0;

	/// Total time spent drifting this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeDrifting = 0.0f;

	/// Total time airborne this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalAirTime = 0.0f;
};

//=============================================================================
// Data Structures - Rewards
//=============================================================================

/**
 * @brief Configuration for near miss rewards.
 *
 * Defines how style points convert to in-game rewards.
 */
USTRUCT(BlueprintType)
struct FMGNearMissRewards
{
	GENERATED_BODY()

	/// Cash earned per style point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CashPerPoint = 0.1f;

	/// Reputation earned per style point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReputationPerPoint = 0.01f;

	/// Nitro charge for each Perfect near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroPerPerfectMiss = 5.0f;

	/// Speed boost multiplier on Insane near miss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBoostOnInsane = 1.1f;

	/// Duration of Insane speed boost in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBoostDuration = 2.0f;

	/// Bonus cash when entering Frenzy mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusCashOnFrenzy = 500;

	/// Bonus points at each combo milestone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboMilestoneBonus = 1000;

	/// Combo counts that trigger milestone bonuses (e.g., 10, 25, 50)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ComboMilestones;
};

//=============================================================================
// Data Structures - Detection
//=============================================================================

/**
 * @brief Tracking data for a potential near miss target.
 *
 * Updated each frame for proximity detection.
 */
USTRUCT(BlueprintType)
struct FMGProximityTarget
{
	GENERATED_BODY()

	/// Unique identifier for this target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	/// Type of target for threshold lookup
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNearMissType TargetType = EMGNearMissType::Vehicle;

	/// Current world position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Current velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	/// Collision radius in centimeters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoundingRadius = 100.0f;

	/// Current distance to player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDistance = FLT_MAX;

	/// Closest distance recorded during approach
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClosestApproach = FLT_MAX;

	/// True if target is getting closer
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsApproaching = false;

	/// True if near miss has already triggered for this pass
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNearMissTriggered = false;

	/// Game time of last update
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LastUpdateTime = 0.0f;
};

//=============================================================================
// Delegates (Event Callbacks)
//=============================================================================

/// Fired when a near miss is registered
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNearMissOccurred, const FMGNearMissEvent&, Event, int32, TotalPoints);

/// Fired when combo state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnComboUpdated, int32, ComboCount, float, Multiplier, int32, TotalPoints);

/// Fired when combo is successfully banked
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboBanked, int32, FinalCombo, int32, TotalPoints);

/// Fired when combo expires (points lost)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboLost, int32, LostPoints);

/// Fired when frenzy mode activates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFrenzyActivated, int32, ComboCount, float, Multiplier);

/// Fired when a combo milestone is reached
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMilestoneReached, int32, Milestone, int32, BonusPoints);

/// Fired when a style bonus is applied
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStyleBonusApplied, const FMGStyleBonus&, Bonus, int32, BonusPoints);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Core subsystem managing near miss detection and scoring.
 *
 * This subsystem handles:
 * - Proximity target registration and tracking
 * - Near miss detection and quality calculation
 * - Combo chain management and frenzy mode
 * - Style bonus application
 * - Session statistics tracking
 * - Reward calculation
 *
 * Access this subsystem from any Blueprint or C++ code via:
 * @code
 * UMGNearMissSubsystem* NearMissSystem = GetGameInstance()->GetSubsystem<UMGNearMissSubsystem>();
 * @endcode
 *
 * @note This is a GameInstanceSubsystem, so data persists across level loads.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGNearMissSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//=========================================================================
	// Lifecycle
	//=========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//=========================================================================
	// Delegates (Bindable Events)
	//=========================================================================

	/// Broadcast when near miss occurs
	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnNearMissOccurred OnNearMissOccurred;

	/// Broadcast when combo updates
	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnComboUpdated OnComboUpdated;

	/// Broadcast when combo is banked
	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnComboBanked OnComboBanked;

	/// Broadcast when combo is lost
	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnComboLost OnComboLost;

	/// Broadcast when frenzy mode activates
	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnFrenzyActivated OnFrenzyActivated;

	/// Broadcast when milestone is reached
	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnMilestoneReached OnMilestoneReached;

	/// Broadcast when style bonus applies
	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnStyleBonusApplied OnStyleBonusApplied;

	//=========================================================================
	// Detection Functions
	//=========================================================================

	/**
	 * @brief Registers a target for proximity tracking.
	 * @param Target Target data to track
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void RegisterProximityTarget(const FMGProximityTarget& Target);

	/**
	 * @brief Removes a target from tracking.
	 * @param TargetId ID of target to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void UnregisterProximityTarget(const FString& TargetId);

	/**
	 * @brief Updates a target's position and velocity.
	 * @param TargetId ID of target to update
	 * @param NewLocation New world position
	 * @param NewVelocity New velocity vector
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void UpdateProximityTarget(const FString& TargetId, FVector NewLocation, FVector NewVelocity);

	/**
	 * @brief Updates the player's state for near miss detection.
	 * @param PlayerLocation Current player position
	 * @param PlayerVelocity Current player velocity
	 * @param bIsDrifting True if player is currently drifting
	 * @param bIsAirborne True if player is currently airborne
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void UpdatePlayerState(FVector PlayerLocation, FVector PlayerVelocity, bool bIsDrifting, bool bIsAirborne);

	/**
	 * @brief Processes all targets for near miss detection.
	 *
	 * Call this each frame after updating positions.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void ProcessProximityCheck();

	/**
	 * @brief Removes all registered targets.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void ClearAllTargets();

	//=========================================================================
	// Near Miss Registration
	//=========================================================================

	/**
	 * @brief Manually registers a near miss event.
	 * @param MissType Type of target involved
	 * @param Distance Closest distance in meters
	 * @param Speed Player speed at time of near miss
	 * @param TargetId ID of the target
	 * @return The created near miss event
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss")
	FMGNearMissEvent RegisterNearMiss(EMGNearMissType MissType, float Distance, float Speed, const FString& TargetId);

	/**
	 * @brief Calculates quality tier based on distance.
	 * @param MissType Type of target (affects thresholds)
	 * @param Distance Distance in meters
	 * @return Quality tier
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss")
	EMGNearMissQuality CalculateQuality(EMGNearMissType MissType, float Distance) const;

	/**
	 * @brief Calculates base points for a near miss.
	 * @param MissType Type of target
	 * @param Quality Quality tier achieved
	 * @param Speed Player speed
	 * @return Base points before multipliers
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss")
	int32 CalculateBasePoints(EMGNearMissType MissType, EMGNearMissQuality Quality, float Speed) const;

	//=========================================================================
	// Combo Management
	//=========================================================================

	/**
	 * @brief Extends the current combo with a new event.
	 * @param Event Near miss event to add
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Combo")
	void ExtendCombo(const FMGNearMissEvent& Event);

	/**
	 * @brief Banks the current combo, securing points.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Combo")
	void BankCombo();

	/**
	 * @brief Called when combo timer expires (points lost).
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Combo")
	void LoseCombo();

	/**
	 * @brief Resets combo to inactive state.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Combo")
	void ResetCombo();

	/** @return Current combo state */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Combo")
	FMGActiveCombo GetActiveCombo() const;

	/** @return True if a combo is currently active */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Combo")
	bool IsComboActive() const;

	/** @return Seconds remaining before combo expires */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Combo")
	float GetComboTimeRemaining() const;

	/** @return Current combo multiplier */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Combo")
	float GetComboMultiplier() const;

	//=========================================================================
	// Threshold Configuration
	//=========================================================================

	/**
	 * @brief Sets distance thresholds for a target type.
	 * @param MissType Target type to configure
	 * @param Thresholds Threshold values
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Config")
	void SetThresholds(EMGNearMissType MissType, const FMGNearMissThresholds& Thresholds);

	/**
	 * @brief Gets thresholds for a target type.
	 * @param MissType Target type to query
	 * @return Threshold values
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Config")
	FMGNearMissThresholds GetThresholds(EMGNearMissType MissType) const;

	//=========================================================================
	// Style Bonus Functions
	//=========================================================================

	/**
	 * @brief Registers a new style bonus.
	 * @param Bonus Bonus definition
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Style")
	void RegisterStyleBonus(const FMGStyleBonus& Bonus);

	/**
	 * @brief Gets all bonuses that apply to an event.
	 * @param Event Near miss event to evaluate
	 * @return Applicable bonuses
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Style")
	TArray<FMGStyleBonus> GetApplicableBonuses(const FMGNearMissEvent& Event) const;

	/**
	 * @brief Applies all applicable style bonuses to an event.
	 * @param Event Event to modify
	 * @return Total bonus points added
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Style")
	int32 ApplyStyleBonuses(FMGNearMissEvent& Event);

	//=========================================================================
	// Reward Functions
	//=========================================================================

	/**
	 * @brief Sets the reward configuration.
	 * @param Config Reward settings
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	void SetRewardConfig(const FMGNearMissRewards& Config);

	/** @return Current reward configuration */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	FMGNearMissRewards GetRewardConfig() const;

	/**
	 * @brief Calculates cash reward for style points.
	 * @param StylePoints Points to convert
	 * @return Cash amount
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	int32 CalculateCashReward(int32 StylePoints) const;

	/**
	 * @brief Calculates reputation reward for style points.
	 * @param StylePoints Points to convert
	 * @return Reputation amount
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	float CalculateReputationReward(int32 StylePoints) const;

	/**
	 * @brief Calculates nitro bonus for a near miss.
	 * @param Event Near miss event
	 * @return Nitro charge amount
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	float CalculateNitroBonus(const FMGNearMissEvent& Event) const;

	//=========================================================================
	// Session Management
	//=========================================================================

	/**
	 * @brief Starts a new tracking session.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Session")
	void StartSession();

	/**
	 * @brief Ends the current session and finalizes stats.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Session")
	void EndSession();

	/**
	 * @brief Pauses session tracking.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Session")
	void PauseSession();

	/**
	 * @brief Resumes session tracking.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Session")
	void ResumeSession();

	/** @return True if session is active */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Session")
	bool IsSessionActive() const;

	/** @return Current session statistics */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Session")
	FMGStyleSessionStats GetSessionStats() const;

	//=========================================================================
	// Statistics Queries
	//=========================================================================

	/** @return Total style points this session */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	int32 GetTotalStylePoints() const;

	/** @return Total near miss count this session */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	int32 GetTotalNearMisses() const;

	/** @return Best combo achieved this session */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	int32 GetBestCombo() const;

	/** @return Closest near miss distance in meters */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	float GetClosestNearMiss() const;

	/**
	 * @brief Gets recent near miss events.
	 * @param Count Maximum events to return
	 * @return Recent events, newest first
	 */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	TArray<FMGNearMissEvent> GetRecentEvents(int32 Count) const;

	//=========================================================================
	// Utility Functions
	//=========================================================================

	/**
	 * @brief Gets display text for a quality tier.
	 * @param Quality Quality to describe
	 * @return Localized display text
	 */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Utility")
	FText GetQualityDisplayText(EMGNearMissQuality Quality) const;

	/**
	 * @brief Gets display text for a miss type.
	 * @param MissType Type to describe
	 * @return Localized display text
	 */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Utility")
	FText GetMissTypeDisplayText(EMGNearMissType MissType) const;

	/**
	 * @brief Gets the UI color for a quality tier.
	 * @param Quality Quality to color
	 * @return Display color
	 */
	UFUNCTION(BlueprintPure, Category = "NearMiss|Utility")
	FLinearColor GetQualityColor(EMGNearMissQuality Quality) const;

	//=========================================================================
	// Persistence
	//=========================================================================

	/**
	 * @brief Saves near miss data to save game.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Persistence")
	void SaveNearMissData();

	/**
	 * @brief Loads near miss data from save game.
	 */
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Persistence")
	void LoadNearMissData();

protected:
	//=========================================================================
	// Internal Functions
	//=========================================================================

	/// Updates combo timer each tick
	void TickCombo(float MGDeltaTime);

	/// Checks for combo milestone achievements
	void CheckMilestones();

	/// Checks if frenzy mode should activate
	void CheckFrenzyState();

	/// Calculates multiplier based on combo count
	float CalculateComboMultiplier(int32 ComboCount) const;

	/// Sets up default threshold values
	void InitializeDefaultThresholds();

	/// Sets up default style bonuses
	void InitializeDefaultBonuses();

private:
	//=========================================================================
	// Internal State
	//=========================================================================

	/// Thresholds per target type
	UPROPERTY()
	TMap<EMGNearMissType, FMGNearMissThresholds> NearMissThresholds;

	/// Registered style bonuses
	UPROPERTY()
	TMap<FString, FMGStyleBonus> RegisteredBonuses;

	/// Current combo state
	UPROPERTY()
	FMGActiveCombo ActiveCombo;

	/// Session statistics
	UPROPERTY()
	FMGStyleSessionStats SessionStats;

	/// Reward configuration
	UPROPERTY()
	FMGNearMissRewards RewardConfig;

	/// Tracked proximity targets
	UPROPERTY()
	TMap<FString, FMGProximityTarget> ProximityTargets;

	/// Recent near miss events (for replay/UI)
	UPROPERTY()
	TArray<FMGNearMissEvent> RecentEvents;

	/// Current player position
	UPROPERTY()
	FVector PlayerLocation;

	/// Current player velocity
	UPROPERTY()
	FVector PlayerVelocity;

	/// True if player is drifting
	UPROPERTY()
	bool bPlayerDrifting = false;

	/// True if player is airborne
	UPROPERTY()
	bool bPlayerAirborne = false;

	/// True if session is active
	UPROPERTY()
	bool bSessionActive = false;

	/// True if session is paused
	UPROPERTY()
	bool bSessionPaused = false;

	/// Index of next milestone to check
	UPROPERTY()
	int32 NextMilestoneIndex = 0;

	/// Timer for combo updates
	FTimerHandle ComboTickTimer;

	/// Maximum recent events to store
	static constexpr int32 MaxRecentEvents = 100;
};

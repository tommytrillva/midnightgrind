// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGNitroBoostSubsystem.h
 * Nitro Boost System for Midnight Grind Racing Game
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem manages the nitro boost mechanic - the "turbo" or "NOS" button
 * that gives players a temporary speed burst. Think Fast & Furious style nitrous
 * oxide injection, or arcade racing game boost mechanics.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. NITRO/NOS (Nitrous Oxide System):
 *    In real cars, nitrous oxide provides extra oxygen for combustion,
 *    creating a power boost. In games, it's simplified to a "boost meter"
 *    that depletes while providing extra speed.
 *
 * 2. NITRO TANK:
 *    - Has a capacity (how much nitro you can store)
 *    - Has a current amount (how much is left)
 *    - Depletes while boosting
 *    - Recharges through various methods
 *
 * 3. CHARGING METHODS:
 *    Unlike real NOS which needs refilling, game nitro "charges" through:
 *    - Time (passive regeneration)
 *    - Drifting (skill-based)
 *    - Near misses (risky driving rewards)
 *    - Drafting (staying behind other cars)
 *    - Pickups (collecting items on track)
 *
 * 4. BOOST LEVELS:
 *    Many racing games have multi-level boosts (Level 1, 2, 3).
 *    Higher levels = more power but faster depletion.
 *    Players must decide: use small boosts frequently or save for big ones.
 *
 * 5. PERFECT BOOST:
 *    A timing-based skill mechanic. If the player activates boost at the
 *    exact right moment (e.g., when a gear shifts or at a visual cue),
 *    they get bonus power. Rewards skilled players.
 *
 * 6. OVERHEATING:
 *    Using boost continuously generates "heat". If heat reaches 100%,
 *    the system overheats and boost is disabled until cooldown completes.
 *    This prevents players from just holding boost constantly.
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 * - This is a UWorldSubsystem (one instance per game world/level)
 * - Works alongside:
 *   - MGSlipstreamSubsystem: Drafting charges nitro
 *   - MGAerodynamicsSubsystem: Some aero profiles affect nitro efficiency
 *   - Vehicle Pawn: Applies the speed multiplier to movement
 *   - UI System: Binds to events to show boost meter/heat gauge
 *
 * USAGE FLOW:
 * 1. Vehicle spawns, gets reference to this subsystem
 * 2. Configure nitro type and charge sources
 * 3. Each frame: Update vehicle location, call charging methods
 * 4. When player presses boost: Call ActivateBoost()
 * 5. While boosting: Apply GetCurrentBoostMultiplier() to speed
 * 6. When player releases or nitro depletes: Call DeactivateBoost()
 *
 * EXAMPLE USAGE:
 * --------------
 *   // In your vehicle pawn:
 *   UMGNitroBoostSubsystem* Nitro = GetWorld()->GetSubsystem<UMGNitroBoostSubsystem>();
 *
 *   // Setup (in BeginPlay):
 *   Nitro->SetNitroType(EMGNitroType::Standard);
 *   Nitro->RegisterChargeSource(DriftingSource);
 *
 *   // In Tick:
 *   if (bBoostButtonPressed && Nitro->CanActivateBoost())
 *   {
 *       Nitro->ActivateBoost();
 *   }
 *
 *   if (Nitro->IsBoostActive())
 *   {
 *       float Multiplier = Nitro->GetCurrentBoostMultiplier();
 *       CurrentSpeed *= Multiplier;
 *   }
 *
 * @see FMGNitroState for current nitro tank state
 * @see FMGNitroConfig for tuning parameters
 * @see UMGSlipstreamSubsystem for drafting-based nitro charging
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGNitroBoostSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Types of nitro systems available to vehicles.
 *
 * Different nitro types provide varied boost characteristics.
 * Players unlock types through progression.
 */
UENUM(BlueprintType)
enum class EMGNitroType : uint8
{
	Standard,      ///< Basic nitro, balanced stats
	Supercharged,  ///< High power but fast depletion
	Cryogenic,     ///< Cannot overheat, slower recharge
	Electric,      ///< Instant activation, low capacity
	Plasma,        ///< Highest power, requires skill to manage
	Experimental   ///< Unpredictable effects, high risk/reward
};

/**
 * @brief Methods for charging the nitro tank.
 *
 * Multiple methods can be active simultaneously.
 * Configure which methods contribute via RegisterChargeSource().
 */
UENUM(BlueprintType)
enum class EMGNitroChargeMethod : uint8
{
	Time,          ///< Passive regeneration over time
	Drifting,      ///< Charged by performing drifts
	NearMiss,      ///< Charged by near misses with obstacles/traffic
	Drafting,      ///< Charged by following close behind other vehicles
	CleanSection,  ///< Bonus charge for clean racing sections
	Combo,         ///< Charged by maintaining scoring combos
	Pickup         ///< Instant charge from collectible pickups in world
};

/**
 * @brief Current operational state of the nitro system.
 *
 * The state determines what actions are available.
 */
UENUM(BlueprintType)
enum class EMGBoostState : uint8
{
	Idle,       ///< Not enough nitro to activate
	Charging,   ///< Nitro is building up
	Ready,      ///< Sufficient nitro to activate boost
	Active,     ///< Boost is currently engaged
	Cooldown,   ///< Brief pause after boost ends
	Overheated  ///< Cannot activate until heat dissipates
};

//=============================================================================
// Data Structures - Configuration
//=============================================================================

/**
 * @brief Configuration parameters for the nitro system.
 *
 * These values determine how nitro behaves.
 * Can be modified by upgrades or vehicle type.
 */
USTRUCT(BlueprintType)
struct FMGNitroConfig
{
	GENERATED_BODY()

	/// Type of nitro system installed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNitroType NitroType = EMGNitroType::Standard;

	/// Maximum nitro capacity (full tank)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxCapacity = 100.0f;

	/// How fast nitro depletes while boosting (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConsumptionRate = 25.0f;

	/// Passive recharge rate when not boosting (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RechargeRate = 10.0f;

	/// Speed multiplier when boost is active (1.5 = 50% faster)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostMultiplier = 1.5f;

	/// Minimum nitro required to activate boost
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinActivationAmount = 10.0f;

	/// Time (seconds) before nitro can be activated again
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 0.5f;

	/// If true, can activate with less than full tank
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPartialBoost = true;

	/// If true, extended use causes overheating
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanOverheat = false;

	/// Heat level (0-100) that triggers overheat state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverheatThreshold = 80.0f;

	/// Time (seconds) to recover from overheat
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverheatCooldownTime = 3.0f;
};

//=============================================================================
// Data Structures - Runtime State
//=============================================================================

/**
 * @brief Current state of the nitro tank and boost system.
 *
 * Query this to update UI elements and check availability.
 */
USTRUCT(BlueprintType)
struct FMGNitroState
{
	GENERATED_BODY()

	/// Current nitro amount (0 to MaxAmount)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentAmount = 100.0f;

	/// Maximum nitro (may differ from config due to upgrades)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAmount = 100.0f;

	/// Current operational state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBoostState State = EMGBoostState::Ready;

	/// Current heat level (0 = cool, 100 = overheated)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatLevel = 0.0f;

	/// How long current boost has been active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActiveTime = 0.0f;

	/// Time remaining in cooldown state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	/// Current boost power level (1-3 typically)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BoostLevel = 1;

	/// True if perfect timing was achieved on activation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPerfectBoost = false;
};

/**
 * @brief Defines a charge source and its contribution rate.
 *
 * Multiple sources can be registered to allow various
 * methods of charging nitro.
 */
USTRUCT(BlueprintType)
struct FMGNitroChargeSource
{
	GENERATED_BODY()

	/// What activity charges nitro
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNitroChargeMethod Method = EMGNitroChargeMethod::Time;

	/// Base charge amount per trigger
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeAmount = 1.0f;

	/// Multiplier applied to charge (can be modified by perks)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeMultiplier = 1.0f;

	/// Whether this source is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;
};

//=============================================================================
// Data Structures - Upgrades
//=============================================================================

/**
 * @brief Definition of a nitro system upgrade.
 *
 * Upgrades improve nitro performance and are purchased
 * with in-game currency.
 */
USTRUCT(BlueprintType)
struct FMGNitroUpgrade
{
	GENERATED_BODY()

	/// Unique identifier for this upgrade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UpgradeID;

	/// Name shown to players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Additional tank capacity provided
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CapacityBonus = 0.0f;

	/// Additional recharge rate provided
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RechargeBonus = 0.0f;

	/// Additional boost power provided
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerBonus = 0.0f;

	/// Reduction in consumption rate (0.1 = 10% more efficient)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EfficiencyBonus = 0.0f;

	/// Player level required to purchase
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockLevel = 1;

	/// In-game currency cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cost = 0;
};

//=============================================================================
// Data Structures - World Elements
//=============================================================================

/**
 * @brief Definition of a boost zone in the game world.
 *
 * Boost zones are areas that enhance nitro effectiveness
 * when driving through them.
 */
USTRUCT(BlueprintType)
struct FMGBoostZone
{
	GENERATED_BODY()

	/// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	/// World position of zone center
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Activation radius in centimeters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 500.0f;

	/// Boost power multiplier within zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostMultiplier = 1.25f;

	/// Extra boost duration when entering zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationBonus = 0.0f;

	/// If true, instantly fills nitro tank when entered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInstantRefill = false;
};

/**
 * @brief Definition of a nitro pickup in the game world.
 *
 * Pickups are collectible items that provide instant
 * nitro charge when collected.
 */
USTRUCT(BlueprintType)
struct FMGNitroPickup
{
	GENERATED_BODY()

	/// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PickupID;

	/// Amount of nitro provided when collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeAmount = 25.0f;

	/// Time (seconds) before pickup respawns
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 30.0f;

	/// World position of pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// True if pickup can currently be collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAvailable = true;
};

//=============================================================================
// Delegates (Event Callbacks)
//=============================================================================

/// Fired when nitro boost is activated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNitroActivated, int32, BoostLevel);

/// Fired when nitro boost is deactivated
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNitroDeactivated);

/// Fired when nitro amount changes (for UI updates)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNitroAmountChanged, float, NewAmount, float, MaxAmount);

/// Fired when nitro tank is completely empty
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNitroDepleted);

/// Fired when system enters overheat state
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNitroOverheat);

/// Fired when perfect boost timing is achieved
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerfectBoostAchieved, float, BonusPower);

/// Fired when nitro is charged from any source
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNitroCharged, EMGNitroChargeMethod, Method, float, Amount);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Core subsystem managing nitro boost mechanics.
 *
 * This subsystem handles:
 * - Nitro tank state and capacity management
 * - Boost activation, deactivation, and power calculation
 * - Multiple charging methods (drifting, pickups, time, etc.)
 * - Heat management and overheat prevention
 * - Boost zones and pickups in the game world
 * - Upgrade system for nitro improvements
 *
 * Access this subsystem from any Blueprint or C++ code via:
 * @code
 * UMGNitroBoostSubsystem* NitroSystem = GetWorld()->GetSubsystem<UMGNitroBoostSubsystem>();
 * @endcode
 *
 * @note This is a WorldSubsystem, so it exists per-world/level.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGNitroBoostSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//=========================================================================
	// Lifecycle
	//=========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//=========================================================================
	// Core Boost Control
	//=========================================================================

	/**
	 * @brief Attempts to activate nitro boost.
	 * @return True if boost was successfully activated
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Control")
	bool ActivateBoost();

	/**
	 * @brief Deactivates nitro boost.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Control")
	void DeactivateBoost();

	/**
	 * @brief Checks if boost can currently be activated.
	 * @return True if activation requirements are met
	 */
	UFUNCTION(BlueprintPure, Category = "Nitro|Control")
	bool CanActivateBoost() const;

	/** @return True if boost is currently active */
	UFUNCTION(BlueprintPure, Category = "Nitro|Control")
	bool IsBoostActive() const { return NitroState.State == EMGBoostState::Active; }

	/**
	 * @brief Gets the current effective boost multiplier.
	 * @return Speed multiplier (1.0 = no boost, higher = faster)
	 */
	UFUNCTION(BlueprintPure, Category = "Nitro|Control")
	float GetCurrentBoostMultiplier() const;

	//=========================================================================
	// Nitro State Queries
	//=========================================================================

	/** @return Complete current nitro state */
	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	FMGNitroState GetNitroState() const { return NitroState; }

	/** @return Current nitro amount */
	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	float GetNitroAmount() const { return NitroState.CurrentAmount; }

	/** @return Current nitro as percentage (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	float GetNitroPercent() const;

	/** @return Current operational state */
	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	EMGBoostState GetBoostState() const { return NitroState.State; }

	/** @return Current heat level (0-100) */
	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	float GetHeatLevel() const { return NitroState.HeatLevel; }

	/** @return True if system is overheated */
	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	bool IsOverheated() const { return NitroState.State == EMGBoostState::Overheated; }

	//=========================================================================
	// Charging Functions
	//=========================================================================

	/**
	 * @brief Adds nitro charge from a specific source.
	 * @param Amount Base charge amount
	 * @param Method Source of the charge (affects multipliers)
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void AddNitroCharge(float Amount, EMGNitroChargeMethod Method = EMGNitroChargeMethod::Time);

	/**
	 * @brief Directly sets nitro amount (bypasses charging logic).
	 * @param Amount New nitro amount
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void SetNitroAmount(float Amount);

	/**
	 * @brief Instantly fills nitro to maximum capacity.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void RefillNitro();

	/**
	 * @brief Removes nitro from the tank.
	 * @param Amount Amount to drain
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void DrainNitro(float Amount);

	/**
	 * @brief Registers a new charge source.
	 * @param Source Source definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void RegisterChargeSource(const FMGNitroChargeSource& Source);

	/** @return All registered charge sources */
	UFUNCTION(BlueprintPure, Category = "Nitro|Charge")
	TArray<FMGNitroChargeSource> GetChargeSources() const { return ChargeSources; }

	//=========================================================================
	// Configuration Functions
	//=========================================================================

	/**
	 * @brief Updates nitro configuration.
	 * @param Config New configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Config")
	void SetNitroConfig(const FMGNitroConfig& Config);

	/** @return Current nitro configuration */
	UFUNCTION(BlueprintPure, Category = "Nitro|Config")
	FMGNitroConfig GetNitroConfig() const { return NitroConfig; }

	/**
	 * @brief Changes the nitro type.
	 * @param Type New nitro type
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Config")
	void SetNitroType(EMGNitroType Type);

	/** @return Current nitro type */
	UFUNCTION(BlueprintPure, Category = "Nitro|Config")
	EMGNitroType GetNitroType() const { return NitroConfig.NitroType; }

	//=========================================================================
	// Multi-Level Boost System
	//=========================================================================

	/**
	 * @brief Sets the current boost level (1 = normal, higher = more power).
	 * @param Level New boost level
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Level")
	void SetBoostLevel(int32 Level);

	/** @return Current boost level */
	UFUNCTION(BlueprintPure, Category = "Nitro|Level")
	int32 GetBoostLevel() const { return NitroState.BoostLevel; }

	/** @return Maximum achievable boost level */
	UFUNCTION(BlueprintPure, Category = "Nitro|Level")
	int32 GetMaxBoostLevel() const { return MaxBoostLevel; }

	/** @return True if boost level can be increased */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Level")
	bool CanUpgradeBoostLevel() const;

	//=========================================================================
	// Perfect Boost System
	//=========================================================================

	/**
	 * @brief Triggers the perfect boost timing window.
	 *
	 * Call this at the appropriate moment (e.g., shift point).
	 * If player activates boost during window, they get bonus power.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Perfect")
	void TriggerPerfectBoostWindow();

	/** @return True if currently in perfect boost window */
	UFUNCTION(BlueprintPure, Category = "Nitro|Perfect")
	bool IsInPerfectBoostWindow() const { return bPerfectBoostWindowActive; }

	/** @return Seconds remaining in perfect boost window */
	UFUNCTION(BlueprintPure, Category = "Nitro|Perfect")
	float GetPerfectBoostWindowRemaining() const;

	//=========================================================================
	// Upgrade System
	//=========================================================================

	/**
	 * @brief Applies an upgrade to the nitro system.
	 * @param Upgrade Upgrade definition to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Upgrades")
	void ApplyUpgrade(const FMGNitroUpgrade& Upgrade);

	/** @return All available (purchasable) upgrades */
	UFUNCTION(BlueprintPure, Category = "Nitro|Upgrades")
	TArray<FMGNitroUpgrade> GetAvailableUpgrades() const { return AvailableUpgrades; }

	/** @return Currently installed upgrades */
	UFUNCTION(BlueprintPure, Category = "Nitro|Upgrades")
	TArray<FMGNitroUpgrade> GetInstalledUpgrades() const { return InstalledUpgrades; }

	//=========================================================================
	// Boost Zone Functions
	//=========================================================================

	/**
	 * @brief Registers a boost zone in the world.
	 * @param Zone Zone definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Zones")
	void RegisterBoostZone(const FMGBoostZone& Zone);

	/**
	 * @brief Removes a boost zone.
	 * @param ZoneID Zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Zones")
	void UnregisterBoostZone(FName ZoneID);

	/** @return True if vehicle is currently in a boost zone */
	UFUNCTION(BlueprintPure, Category = "Nitro|Zones")
	bool IsInBoostZone() const { return ActiveBoostZone.ZoneID != NAME_None; }

	/** @return Current boost zone data */
	UFUNCTION(BlueprintPure, Category = "Nitro|Zones")
	FMGBoostZone GetActiveBoostZone() const { return ActiveBoostZone; }

	//=========================================================================
	// Pickup Functions
	//=========================================================================

	/**
	 * @brief Registers a nitro pickup in the world.
	 * @param Pickup Pickup definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Pickups")
	void RegisterPickup(const FMGNitroPickup& Pickup);

	/**
	 * @brief Collects a pickup (adds charge and starts respawn timer).
	 * @param PickupID Pickup to collect
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Pickups")
	void CollectPickup(FName PickupID);

	/** @return All pickups that are currently available to collect */
	UFUNCTION(BlueprintPure, Category = "Nitro|Pickups")
	TArray<FMGNitroPickup> GetActivePickups() const;

	//=========================================================================
	// Update Functions
	//=========================================================================

	/**
	 * @brief Updates vehicle location for zone/pickup detection.
	 *
	 * Call this each frame from your vehicle pawn.
	 *
	 * @param Location Current world position of vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Nitro|Update")
	void UpdateVehicleLocation(FVector Location);

	//=========================================================================
	// Delegates (Bindable Events)
	//=========================================================================

	/// Broadcast when boost activates
	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroActivated OnNitroActivated;

	/// Broadcast when boost deactivates
	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroDeactivated OnNitroDeactivated;

	/// Broadcast when nitro amount changes
	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroAmountChanged OnNitroAmountChanged;

	/// Broadcast when nitro is fully depleted
	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroDepleted OnNitroDepleted;

	/// Broadcast when overheat occurs
	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroOverheat OnNitroOverheat;

	/// Broadcast when perfect boost is achieved
	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnPerfectBoostAchieved OnPerfectBoostAchieved;

	/// Broadcast when nitro is charged
	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroCharged OnNitroCharged;

protected:
	//=========================================================================
	// Internal Update Functions
	//=========================================================================

	/// Main tick function for nitro updates
	void OnNitroTick();

	/// Updates the boost state machine
	void UpdateBoostState();

	/// Processes passive charging
	void UpdateCharging();

	/// Updates heat level
	void UpdateHeat();

	/// Checks if vehicle is in any boost zones
	void CheckBoostZones();

	/// Updates pickup respawn timers
	void UpdatePickups();

	/// Initializes default configuration values
	void InitializeDefaultConfig();

	/// Calculates final boost multiplier with all bonuses
	float CalculateEffectiveMultiplier() const;

	//=========================================================================
	// Internal State
	//=========================================================================

	/// Current nitro configuration
	UPROPERTY()
	FMGNitroConfig NitroConfig;

	/// Current runtime state
	UPROPERTY()
	FMGNitroState NitroState;

	/// Registered charge sources
	UPROPERTY()
	TArray<FMGNitroChargeSource> ChargeSources;

	/// Available upgrades for purchase
	UPROPERTY()
	TArray<FMGNitroUpgrade> AvailableUpgrades;

	/// Currently installed upgrades
	UPROPERTY()
	TArray<FMGNitroUpgrade> InstalledUpgrades;

	/// Registered boost zones (keyed by ZoneID)
	UPROPERTY()
	TMap<FName, FMGBoostZone> BoostZones;

	/// Currently active boost zone (if any)
	UPROPERTY()
	FMGBoostZone ActiveBoostZone;

	/// Registered pickups (keyed by PickupID)
	UPROPERTY()
	TMap<FName, FMGNitroPickup> Pickups;

	/// Current vehicle world location
	UPROPERTY()
	FVector CurrentVehicleLocation = FVector::ZeroVector;

	/// Maximum achievable boost level
	UPROPERTY()
	int32 MaxBoostLevel = 3;

	/// True if perfect boost window is open
	UPROPERTY()
	bool bPerfectBoostWindowActive = false;

	/// Duration of perfect boost window in seconds
	UPROPERTY()
	float PerfectBoostWindowDuration = 0.3f;

	/// Time remaining in current perfect boost window
	UPROPERTY()
	float PerfectBoostWindowTimer = 0.0f;

	/// Bonus power granted for perfect boost timing
	UPROPERTY()
	float PerfectBoostBonusPower = 0.25f;

	/// Timer handle for nitro tick updates
	FTimerHandle NitroTickHandle;
};

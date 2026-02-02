// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGRaceModifiers.h
 * @brief Race modifier system for Midnight Grind racing game.
 *
 * This file contains the race modifier framework that allows dynamic
 * gameplay changes during races. Modifiers can alter physics, rules,
 * visuals, and difficulty to create varied racing experiences.
 *
 * @section overview_mod Modifier System Overview
 * The modifier system provides:
 * - Base UMGRaceModifier class for creating custom modifiers
 * - Pre-built modifiers for common gameplay variations
 * - UMGRaceModifierManager for activation and lifecycle management
 * - Compatibility checking between modifiers
 * - Reward multipliers for challenging modifiers
 *
 * @section creating_mod Creating Custom Modifiers
 * 1. Create a new class inheriting from UMGRaceModifier
 * 2. Override lifecycle events (OnActivated, OnTick, etc.)
 * 3. Set identification properties (ID, name, description)
 * 4. Configure compatibility with other modifiers
 * 5. Register the modifier class with the modifier manager
 *
 * @section builtin_mod Built-in Modifiers
 * This file includes these ready-to-use modifiers:
 * - NoNOS / UnlimitedNOS: Nitrous availability
 * - GhostMode: Vehicles pass through each other
 * - CatchUp: Rubber-banding for trailing racers
 * - SlipstreamBoost: Enhanced drafting effect
 * - OneHitKO: Elimination on collision
 * - Elimination: Last place eliminated each lap
 * - MirrorMode: Track is mirrored
 * - TimeAttack: Checkpoint-based time extension
 * - NightVision: Limited visibility racing
 * - Traffic: AI traffic on track
 * - DriftOnly: Must drift to gain position
 * - RandomEvents: Periodic random occurrences
 *
 * @see UMGRaceModifier
 * @see UMGRaceModifierManager
 */

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MGRaceModifiers.generated.h"

class AMGRaceGameMode;
class AMGVehiclePawn;
class AController;

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * Modifier category for UI grouping
 *
 * Categories help organize modifiers in selection menus and
 * indicate the primary aspect of gameplay each modifier affects.
 */
UENUM(BlueprintType)
enum class EMGModifierCategory : uint8
{
	/** Affects vehicle physics/handling */
	Physics,
	/** Affects speed/acceleration */
	Speed,
	/** Affects race rules/scoring */
	Rules,
	/** Affects visuals/environment */
	Visual,
	/** Affects difficulty/AI */
	Difficulty,
	/** Fun/party modifiers */
	Party,
	/** Hardcore/challenge modifiers */
	Challenge
};

/**
 * Modifier effect severity
 *
 * Indicates how significantly a modifier changes gameplay.
 * Higher severity modifiers typically offer better reward multipliers
 * but may not be allowed in ranked play.
 */
UENUM(BlueprintType)
enum class EMGModifierSeverity : uint8
{
	/** Minor tweak */
	Minor,
	/** Noticeable change */
	Moderate,
	/** Major game changer */
	Major,
	/** Completely transforms gameplay */
	Extreme
};

// ============================================================================
// BASE MODIFIER CLASS
// ============================================================================

/**
 * Base race modifier class
 *
 * Abstract base class for all race modifiers. Inherit from this class
 * to create custom gameplay modifications. Override the lifecycle
 * event functions to implement modifier behavior.
 *
 * @section lifecycle_mod Lifecycle Events
 * Modifiers receive callbacks at key points:
 * - OnActivated: When modifier is turned on (setup time)
 * - OnDeactivated: When modifier is turned off (cleanup time)
 * - OnTick: Every frame while active (continuous effects)
 * - OnRaceStarted: When countdown finishes and race begins
 * - OnRaceEnded: When race completes
 * - OnVehicleSpawned: When a vehicle enters the race
 * - OnLapCompleted: When any racer completes a lap
 *
 * @section compat_mod Compatibility
 * Use IncompatibleModifiers to list modifiers that conflict.
 * Use RequiredModifiers to list modifiers that must also be active.
 * The modifier manager enforces these rules during activation.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGRaceModifier : public UObject
{
	GENERATED_BODY()

public:
	UMGRaceModifier();

	// ==========================================
	// IDENTIFICATION
	// ==========================================
	// Properties that identify and describe the modifier.

	/// Unique identifier for this modifier (e.g., "MOD_NoNOS", "MOD_GhostMode")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FName ModifierID;

	/// Localized display name shown in modifier selection UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FText DisplayName;

	/// Full description explaining what the modifier does
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FText Description;

	/// Brief one-line description for compact UI displays
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FText ShortDescription;

	/// Icon texture for modifier selection and HUD display
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	UTexture2D* Icon;

	/// Category for UI organization
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	EMGModifierCategory Category = EMGModifierCategory::Rules;

	/// How significantly this modifier changes gameplay
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	EMGModifierSeverity Severity = EMGModifierSeverity::Moderate;

	// ==========================================
	// COMPATIBILITY
	// ==========================================
	// Rules for combining modifiers.

	/// List of modifier IDs that cannot be active at the same time as this one
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Compatibility")
	TArray<FName> IncompatibleModifiers;

	/// List of modifier IDs that must be active for this modifier to work
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Compatibility")
	TArray<FName> RequiredModifiers;

	/// If false, this modifier is disabled in ranked/competitive races
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Compatibility")
	bool bAllowedInRanked = false;

	/// If false, this modifier is disabled in multiplayer races
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Compatibility")
	bool bAllowedInMultiplayer = true;

	// ==========================================
	// REWARDS
	// ==========================================
	// Multipliers applied to race rewards when this modifier is active.

	/// XP reward multiplier (1.0 = normal, 1.5 = 50% bonus, etc.)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Rewards")
	float XPMultiplier = 1.0f;

	/// Cash reward multiplier (1.0 = normal, 1.5 = 50% bonus, etc.)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Rewards")
	float CashMultiplier = 1.0f;

	// ==========================================
	// LIFECYCLE
	// ==========================================
	// Event callbacks - override these to implement modifier behavior.

	/**
	 * Called when the modifier is activated.
	 * Use for initial setup, registering delegates, modifying game state.
	 * @param GameMode Reference to the race game mode
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnActivated(AMGRaceGameMode* GameMode);

	/**
	 * Called when the modifier is deactivated.
	 * Use for cleanup, unregistering delegates, restoring game state.
	 * @param GameMode Reference to the race game mode
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnDeactivated(AMGRaceGameMode* GameMode);

	/**
	 * Called every frame while the modifier is active.
	 * Use for continuous effects that need per-frame updates.
	 * @param DeltaTime Time since last tick in seconds
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnTick(float MGDeltaTime);

	/**
	 * Called when the race countdown finishes and racing begins.
	 * Use for effects that should start with the race.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnRaceStarted();

	/**
	 * Called when the race completes.
	 * Use for final calculations or cleanup before results screen.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnRaceEnded();

	/**
	 * Called when a vehicle spawns into the race.
	 * Use to modify vehicle properties or attach components.
	 * @param Vehicle The spawned vehicle pawn
	 * @param Controller The controller (player or AI) driving the vehicle
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnVehicleSpawned(AMGVehiclePawn* Vehicle, AController* Controller);

	/**
	 * Called when any racer completes a lap.
	 * Use for lap-based effects like elimination or lap bonuses.
	 * @param Controller The controller that completed the lap
	 * @param LapNumber The lap number just completed (1-indexed)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnLapCompleted(AController* Controller, int32 LapNumber);

	/**
	 * Checks if this modifier can be used together with another.
	 * @param Other The other modifier to check compatibility with
	 * @return True if both modifiers can be active simultaneously
	 */
	UFUNCTION(BlueprintPure, Category = "Modifier")
	bool IsCompatibleWith(const UMGRaceModifier* Other) const;

protected:
	/// Cached reference to the game mode for quick access
	UPROPERTY()
	TWeakObjectPtr<AMGRaceGameMode> CachedGameMode;

	/// Tracks whether this modifier is currently active
	bool bIsActive = false;
};

// ============================================================================
// CONCRETE MODIFIERS - NOS RELATED
// ============================================================================

/**
 * No NOS modifier - disables nitrous
 *
 * Disables the nitrous boost system for all vehicles, creating a
 * more traditional racing experience where pure driving skill
 * determines the outcome.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_NoNOS : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_NoNOS();
	virtual void OnVehicleSpawned_Implementation(AMGVehiclePawn* Vehicle, AController* Controller) override;
};

/**
 * Unlimited NOS modifier - infinite nitrous
 *
 * Provides infinite nitrous to all vehicles. NOS never depletes,
 * allowing constant boosting. Creates chaotic, high-speed races.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_UnlimitedNOS : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_UnlimitedNOS();
	virtual void OnTick_Implementation(float MGDeltaTime) override;
};

// ============================================================================
// CONCRETE MODIFIERS - COLLISION & PHYSICS
// ============================================================================

/**
 * Ghost Mode - vehicles pass through each other
 *
 * Disables vehicle-to-vehicle collisions, allowing racers to pass
 * through each other. Prevents ramming tactics and reduces frustration
 * from collisions. Wall collisions remain active.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_GhostMode : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_GhostMode();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnVehicleSpawned_Implementation(AMGVehiclePawn* Vehicle, AController* Controller) override;
};

/**
 * Catch Up mode - rubber banding for trailing racers
 *
 * Applies dynamic speed adjustments based on race position.
 * Trailing racers get a speed boost while leaders are slightly slowed,
 * keeping the pack closer together for more competitive racing.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_CatchUp : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_CatchUp();
	virtual void OnTick_Implementation(float MGDeltaTime) override;

	/// Maximum speed multiplier for last place (1.15 = 15% faster)
	UPROPERTY(EditDefaultsOnly, Category = "Catch Up")
	float MaxSpeedBoost = 1.15f;

	/// Minimum speed multiplier for first place (0.95 = 5% slower)
	UPROPERTY(EditDefaultsOnly, Category = "Catch Up")
	float MaxSpeedReduction = 0.95f;
};

/**
 * Slipstream Boost - increased drafting effect
 *
 * Amplifies the slipstream/drafting effect when following closely
 * behind another vehicle. Encourages close racing and strategic
 * positioning for overtakes.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_SlipstreamBoost : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_SlipstreamBoost();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;

	/// Multiplier applied to normal slipstream effect (2.0 = double effect)
	UPROPERTY(EditDefaultsOnly, Category = "Slipstream")
	float SlipstreamMultiplier = 2.0f;
};

// ============================================================================
// CONCRETE MODIFIERS - ELIMINATION
// ============================================================================

/**
 * One Hit Knockout - single collision eliminates
 *
 * Any significant collision instantly eliminates the vehicle from
 * the race. Creates tense, careful racing where contact is fatal.
 * Threshold can be configured to ignore minor bumps.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_OneHitKO : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_OneHitKO();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;

	/**
	 * Handles collision detection and elimination logic.
	 * Bound to vehicle collision events when modifier is active.
	 */
	UFUNCTION()
	void OnVehicleCollision(AMGVehiclePawn* Vehicle, AActor* OtherActor, float ImpactForce);

	/// Minimum impact force required to trigger elimination (filters minor bumps)
	UPROPERTY(EditDefaultsOnly, Category = "OneHitKO")
	float MinKOImpactForce = 50.0f;
};

/**
 * Elimination - last place eliminated each lap
 *
 * At the end of each lap, the racer in last place is eliminated
 * from the race. Creates increasing pressure as the field shrinks.
 * Final lap is winner-take-all between remaining racers.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_Elimination : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_Elimination();
	virtual void OnLapCompleted_Implementation(AController* Controller, int32 LapNumber) override;
	virtual void OnTick_Implementation(float MGDeltaTime) override;

private:
	/// Set of controllers that have been eliminated
	TSet<AController*> EliminatedRacers;

	/// Accumulator for elimination check timing
	float EliminationCheckTimer = 0.0f;

	/// How often to check for elimination conditions
	float EliminationCheckInterval = 1.0f;
};

// ============================================================================
// CONCRETE MODIFIERS - TRACK VARIATIONS
// ============================================================================

/**
 * Mirror Mode - track is mirrored
 *
 * Flips the track layout horizontally, making left turns become
 * right turns and vice versa. Familiar tracks become fresh challenges
 * as muscle memory no longer applies.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_MirrorMode : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_MirrorMode();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;
};

/**
 * Time Attack - checkpoints add time
 *
 * Race against a countdown timer instead of other racers.
 * Each checkpoint passed adds time to the clock. Run out of
 * time and you're eliminated. Fastest total time wins.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_TimeAttack : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_TimeAttack();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnTick_Implementation(float MGDeltaTime) override;

	/// Initial time on the clock when race starts
	UPROPERTY(EditDefaultsOnly, Category = "Time Attack")
	float StartingTime = 60.0f;

	/// Seconds added to clock for each checkpoint crossed
	UPROPERTY(EditDefaultsOnly, Category = "Time Attack")
	float TimePerCheckpoint = 10.0f;

private:
	/// Tracks remaining time for each racer
	TMap<AController*, float> RacerTimes;
};

// ============================================================================
// CONCRETE MODIFIERS - VISUAL & ENVIRONMENT
// ============================================================================

/**
 * Night Vision - dark track with limited visibility
 *
 * Dramatically reduces visibility distance, simulating night
 * racing with limited headlights. Requires memorization and
 * careful speed management through blind corners.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_NightVision : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_NightVision();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;

	/// Maximum visibility distance in meters (fog wall beyond this)
	UPROPERTY(EditDefaultsOnly, Category = "Night Vision")
	float VisibilityDistance = 50.0f;
};

/**
 * Traffic Mode - adds traffic vehicles to track
 *
 * Spawns AI-controlled civilian traffic on the race track.
 * Traffic follows normal driving patterns and must be avoided.
 * Adds unpredictability and requires situational awareness.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_Traffic : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_Traffic();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnTick_Implementation(float MGDeltaTime) override;

	/// Number of traffic vehicles per kilometer of track
	UPROPERTY(EditDefaultsOnly, Category = "Traffic")
	float TrafficDensity = 5.0f;

	/// Actor classes to spawn as traffic (randomly selected)
	UPROPERTY(EditDefaultsOnly, Category = "Traffic")
	TArray<TSubclassOf<AActor>> TrafficVehicleClasses;

private:
	/// References to spawned traffic actors for cleanup
	UPROPERTY()
	TArray<AActor*> SpawnedTraffic;
};

// ============================================================================
// CONCRETE MODIFIERS - SPECIAL RULES
// ============================================================================

/**
 * Drift Only - can only gain position while drifting
 *
 * Vehicles cannot overtake unless they are actively drifting.
 * Encourages aggressive, stylish driving and mastery of drift
 * techniques for competitive advantage.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_DriftOnly : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_DriftOnly();
	virtual void OnTick_Implementation(float MGDeltaTime) override;

	/// Minimum angle between velocity and facing direction to count as drifting
	UPROPERTY(EditDefaultsOnly, Category = "Drift Only")
	float MinDriftAngle = 15.0f;
};

/**
 * Random Events - periodic random events during race
 *
 * Triggers random gameplay events at unpredictable intervals.
 * Events may include temporary speed boosts, grip changes,
 * obstacle spawns, or other surprises. Adds chaos and variety.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_RandomEvents : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_RandomEvents();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnTick_Implementation(float MGDeltaTime) override;

	/// Minimum seconds between random events
	UPROPERTY(EditDefaultsOnly, Category = "Random Events")
	float MinEventInterval = 15.0f;

	/// Maximum seconds between random events
	UPROPERTY(EditDefaultsOnly, Category = "Random Events")
	float MaxEventInterval = 45.0f;

private:
	/// Accumulator for event timing
	float EventTimer = 0.0f;

	/// Randomly determined time until next event
	float NextEventTime = 0.0f;

	/// Selects and triggers a random event
	void TriggerRandomEvent();
};

// ============================================================================
// MODIFIER MANAGER
// ============================================================================

/**
 * Race Modifier Manager
 *
 * Central system for managing race modifiers. Handles activation,
 * deactivation, compatibility checking, and lifecycle notifications.
 * Create one instance per race and initialize with the game mode.
 *
 * @section usage_mgr Basic Usage
 * 1. Create manager: NewObject<UMGRaceModifierManager>()
 * 2. Initialize: Manager->Initialize(GameMode)
 * 3. Activate modifiers: Manager->ActivateModifier("MOD_GhostMode")
 * 4. Tick each frame: Manager->TickModifiers(DeltaTime)
 * 5. Notify events: Manager->NotifyRaceStarted(), etc.
 * 6. Cleanup: Manager->DeactivateAllModifiers()
 *
 * @section compat_mgr Compatibility
 * The manager enforces modifier compatibility rules. ActivateModifier
 * will return false if the requested modifier conflicts with an
 * already-active modifier or if required modifiers are missing.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGRaceModifierManager : public UObject
{
	GENERATED_BODY()

public:
	UMGRaceModifierManager();

	/**
	 * Initializes the manager with a game mode reference.
	 * Must be called before activating any modifiers.
	 * @param GameMode The race game mode instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Modifier Manager")
	void Initialize(AMGRaceGameMode* GameMode);

	/**
	 * Activates a modifier by its ID.
	 * @param ModifierID The unique modifier identifier
	 * @return True if activation succeeded, false if incompatible or not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Modifier Manager")
	bool ActivateModifier(FName ModifierID);

	/**
	 * Deactivates a currently active modifier.
	 * @param ModifierID The unique modifier identifier
	 * @return True if deactivation succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Modifier Manager")
	bool DeactivateModifier(FName ModifierID);

	/**
	 * Deactivates all currently active modifiers.
	 * Call during race cleanup.
	 */
	UFUNCTION(BlueprintCallable, Category = "Modifier Manager")
	void DeactivateAllModifiers();

	/**
	 * Checks if a specific modifier is currently active.
	 * @param ModifierID The modifier to check
	 * @return True if the modifier is active
	 */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	bool IsModifierActive(FName ModifierID) const;

	/**
	 * Gets all currently active modifier instances.
	 * @return Array of active modifier objects
	 */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	TArray<UMGRaceModifier*> GetActiveModifiers() const;

	/**
	 * Gets all registered modifier classes that can be activated.
	 * @return Array of modifier class types
	 */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	TArray<TSubclassOf<UMGRaceModifier>> GetAvailableModifiers() const;

	/**
	 * Calculates the combined XP multiplier from all active modifiers.
	 * @return Total XP multiplier (multiplicative)
	 */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	float GetTotalXPMultiplier() const;

	/**
	 * Calculates the combined cash multiplier from all active modifiers.
	 * @return Total cash multiplier (multiplicative)
	 */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	float GetTotalCashMultiplier() const;

	/**
	 * Checks if a modifier can be activated (compatibility check).
	 * @param ModifierID The modifier to check
	 * @return True if activation would succeed
	 */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	bool CanActivateModifier(FName ModifierID) const;

	/**
	 * Updates all active modifiers. Call once per frame.
	 * @param DeltaTime Time since last tick in seconds
	 */
	void TickModifiers(float MGDeltaTime);

	/// Notifies all active modifiers that the race has started
	void NotifyRaceStarted();

	/// Notifies all active modifiers that the race has ended
	void NotifyRaceEnded();

	/**
	 * Notifies all active modifiers that a vehicle has spawned.
	 * @param Vehicle The spawned vehicle pawn
	 * @param Controller The controller driving the vehicle
	 */
	void NotifyVehicleSpawned(AMGVehiclePawn* Vehicle, AController* Controller);

	/**
	 * Notifies all active modifiers that a lap was completed.
	 * @param Controller The controller that completed the lap
	 * @param LapNumber The lap number that was completed
	 */
	void NotifyLapCompleted(AController* Controller, int32 LapNumber);

private:
	/// Weak reference to the race game mode
	UPROPERTY()
	TWeakObjectPtr<AMGRaceGameMode> GameModeRef;

	/// Map of active modifier ID to modifier instance
	UPROPERTY()
	TMap<FName, UMGRaceModifier*> ActiveModifiers;

	/// All registered modifier classes available for activation
	UPROPERTY()
	TArray<TSubclassOf<UMGRaceModifier>> RegisteredModifiers;

	/**
	 * Creates an instance of a modifier class.
	 * @param ModifierClass The class to instantiate
	 * @return New modifier instance
	 */
	UMGRaceModifier* CreateModifier(TSubclassOf<UMGRaceModifier> ModifierClass);
};

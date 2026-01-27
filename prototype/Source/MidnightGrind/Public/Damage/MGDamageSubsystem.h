// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGDamageSubsystem.h
 * @brief Vehicle damage system for Midnight Grind racing game.
 *
 * This subsystem manages all aspects of vehicle damage including collision detection,
 * visual deformation, mechanical failures, and repair functionality. It provides a
 * realistic damage model where impacts affect both vehicle appearance and performance.
 *
 * The damage system features:
 * - **Zone-Based Damage**: Vehicle divided into 10 damage zones (front, sides, rear, roof, underbody)
 * - **Component Damage**: Individual mechanical systems can be damaged (engine, transmission, suspension, etc.)
 * - **Visual Deformation**: Mesh deformation data for realistic crumple zones
 * - **Performance Impact**: Damage reduces handling, acceleration, top speed, and braking
 * - **Part Detachment**: Severely damaged body parts can fall off
 * - **Fire System**: Critical damage can ignite vehicles
 * - **Repair System**: Zone and component repair with cost calculation
 *
 * Key concepts for new developers:
 * - Damage flows: Collision -> Zone -> Components -> Performance multipliers
 * - Each vehicle is identified by FName (VehicleID) and must be registered
 * - Damage severity levels: Cosmetic < Light < Moderate < Heavy < Critical < Totaled
 * - Performance is degraded via multipliers (0.0 = disabled, 1.0 = full performance)
 *
 * Example usage:
 * @code
 * // Get the subsystem
 * UMGDamageSubsystem* DamageSys = GetGameInstance()->GetSubsystem<UMGDamageSubsystem>();
 *
 * // Register a vehicle
 * DamageSys->RegisterVehicle("PlayerCar_001", 100.0f);
 *
 * // Apply collision damage
 * DamageSys->ApplyCollisionDamage("PlayerCar_001", HitPoint, HitNormal, 45.0f, "AICar_003");
 *
 * // Check performance impact
 * float EngineMult = DamageSys->GetEnginePerformanceMultiplier("PlayerCar_001");
 * CurrentHP *= EngineMult; // Reduce power output based on engine damage
 *
 * // Repair the vehicle
 * int32 Cost = DamageSys->GetRepairCost("PlayerCar_001");
 * if (PlayerMoney >= Cost)
 * {
 *     DamageSys->RepairVehicleFull("PlayerCar_001");
 * }
 * @endcode
 *
 * @see FMGVehicleDamageState Complete damage state structure
 * @see FMGDamageConfig Configuration options
 * @see EMGDamageZone Body zone enumeration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDamageSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Types of damage that can be inflicted on vehicles.
 *
 * Different damage types may have different severity multipliers and
 * affect different components. Used for damage calculation and visual effects.
 */
UENUM(BlueprintType)
enum class EMGDamageType : uint8
{
	None,			///< No damage (default/invalid)
	Collision,		///< Generic collision with object or vehicle
	SideSwipe,		///< Glancing side-to-side contact (lower damage)
	TBone,			///< Perpendicular side impact (high damage)
	RearEnd,		///< Impact to rear of vehicle
	FrontalImpact,	///< Head-on collision (affects engine/radiator)
	Rollover,		///< Vehicle rolled over (roof/underbody damage)
	WallScrape,		///< Grinding against wall (cosmetic, low damage)
	SpikeTrap,		///< Police spike strips (tires only)
	EMP				///< Electromagnetic pulse (electronics disabled)
};

/**
 * @brief Damage zones dividing the vehicle body.
 *
 * The vehicle is divided into 10 zones for localized damage tracking.
 * Each zone has its own health pool and can affect different components
 * when damaged.
 *
 * Zone layout (top-down view):
 * @code
 *     [FrontLeft] [FrontCenter] [FrontRight]
 *     [SideLeft ]              [SideRight ]
 *     [RearLeft ] [RearCenter ] [RearRight ]
 *              [Roof]
 *            [Underbody]
 * @endcode
 */
UENUM(BlueprintType)
enum class EMGDamageZone : uint8
{
	None,			///< Invalid/unspecified zone
	FrontLeft,		///< Front-left corner (headlight, fender)
	FrontCenter,	///< Front center (grille, bumper, radiator)
	FrontRight,		///< Front-right corner
	SideLeft,		///< Left side (doors, rocker panels)
	SideRight,		///< Right side
	RearLeft,		///< Rear-left corner (taillight, quarter panel)
	RearCenter,		///< Rear center (trunk, bumper, exhaust)
	RearRight,		///< Rear-right corner
	Roof,			///< Top of vehicle (rollover damage)
	Underbody		///< Undercarriage (curb strikes, rough terrain)
};

/**
 * @brief Severity levels for damage assessment.
 *
 * Severity determines visual effects, performance penalties, and repair costs.
 * Progression: None -> Cosmetic -> Light -> Moderate -> Heavy -> Critical -> Totaled
 */
UENUM(BlueprintType)
enum class EMGDamageSeverity : uint8
{
	None,		///< No damage (100% health)
	Cosmetic,	///< Minor scratches/dents, no performance impact
	Light,		///< Visible damage, minimal performance impact
	Moderate,	///< Significant damage, noticeable performance loss
	Heavy,		///< Major damage, severe performance penalties
	Critical,	///< Near-failure, may cause component shutdown
	Totaled		///< Vehicle destroyed, cannot be driven
};

/**
 * @brief Mechanical components that can be individually damaged.
 *
 * Each component has its own health and affects specific vehicle systems
 * when damaged. Component damage spreads from zone damage based on
 * which components are located in that zone.
 */
UENUM(BlueprintType)
enum class EMGVehicleComponent : uint8
{
	None,			///< Invalid/unspecified
	Engine,			///< Affects power output and can cause fire
	Transmission,	///< Affects gear changes and acceleration
	Suspension,		///< Affects handling and ride height
	Steering,		///< Affects turn response and alignment
	Brakes,			///< Affects stopping power
	Tires,			///< Affects grip and can cause blowouts
	Exhaust,		///< Affects power slightly, causes backfires
	NitroSystem,	///< Affects nitrous effectiveness
	Radiator,		///< Affects cooling, can cause overheating
	FuelTank,		///< Can cause leaks and fire risk
	Electronics		///< Affects gauges, lights, and systems
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief Single damage event data.
 *
 * Represents one instance of damage being applied to a vehicle.
 * Created by damage functions and passed to event delegates.
 */
USTRUCT(BlueprintType)
struct FMGDamageInstance
{
	GENERATED_BODY()

	/// Type of damage being applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageType DamageType = EMGDamageType::None;

	/// Which body zone received the damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageZone Zone = EMGDamageZone::None;

	/// Calculated severity based on damage amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageSeverity Severity = EMGDamageSeverity::None;

	/// Initial damage value before modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RawDamage = 0.0f;

	/// Actual damage applied after armor/resistance modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalDamage = 0.0f;

	/// World-space location where damage was received
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactPoint = FVector::ZeroVector;

	/// Surface normal at impact point (points away from surface)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactNormal = FVector::ZeroVector;

	/// Speed at moment of impact in km/h
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactVelocity = 0.0f;

	/// ID of the vehicle/object that caused the damage (NAME_None for environment)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName InstigatorID;

	/// True if damage was blocked by armor/reinforcement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasBlocked = false;
};

/**
 * @brief Damage state for a single body zone.
 *
 * Tracks cumulative damage to one of the 10 body zones.
 */
USTRUCT(BlueprintType)
struct FMGZoneDamageState
{
	GENERATED_BODY()

	/// Which zone this state represents
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageZone Zone = EMGDamageZone::None;

	/// Accumulated damage points in this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDamage = 0.0f;

	/// Maximum damage before zone is destroyed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDamage = 100.0f;

	/// Current severity level based on damage percentage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageSeverity Severity = EMGDamageSeverity::None;

	/// Visual deformation level (0-5, affects mesh deformation intensity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeformationLevel = 0;

	/// False if zone damage has disabled associated functions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFunctional = true;

	/// List of part IDs that have detached from this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> DetachedParts;
};

/**
 * @brief Damage state for a single mechanical component.
 *
 * Tracks health and functionality of vehicle mechanical systems.
 */
USTRUCT(BlueprintType)
struct FMGComponentDamageState
{
	GENERATED_BODY()

	/// Which component this state represents
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleComponent Component = EMGVehicleComponent::None;

	/// Current health points
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	/// Maximum health points
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	/// Performance multiplier (1.0 = full, 0.5 = 50% effectiveness)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EfficiencyMultiplier = 1.0f;

	/// True if component is working (even if degraded)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFunctional = true;

	/// True if component is completely disabled (0% efficiency)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDisabled = false;

	/// Cost in game currency to repair this component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RepairCost = 0.0f;
};

/**
 * @brief Complete damage state for a vehicle.
 *
 * Master structure containing all damage information for one vehicle.
 * This is the primary data structure managed by the damage subsystem.
 */
USTRUCT(BlueprintType)
struct FMGVehicleDamageState
{
	GENERATED_BODY()

	/// Unique identifier linking to the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Overall vehicle health (aggregate of all zones)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverallHealth = 100.0f;

	/// Maximum overall health
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	/// Worst severity across all zones
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageSeverity OverallSeverity = EMGDamageSeverity::None;

	/// Damage state for each body zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDamageZone, FMGZoneDamageState> ZoneDamage;

	/// Damage state for each mechanical component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGVehicleComponent, FMGComponentDamageState> ComponentDamage;

	/// True if vehicle can still be driven (not totaled, has functional drivetrain)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDriveable = true;

	/// True if damage exceeds totaled threshold (cannot be repaired normally)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTotaled = false;

	/// Sum of all repair costs for full restoration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRepairCost = 0;

	// --- Visual State Flags ---

	/// True if vehicle is currently on fire
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnFire = false;

	/// Smoke particle intensity (0 = none, 1 = heavy smoke)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmokeLevel = 0.0f;

	/// True if fuel is leaking (fire hazard)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeakingFuel = false;

	/// True if oil is leaking (handling hazard, leaves trail)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeakingOil = false;
};

/**
 * @brief Configuration options for the damage system.
 *
 * Allows tuning damage behavior for different game modes or difficulty settings.
 */
USTRUCT(BlueprintType)
struct FMGDamageConfig
{
	GENERATED_BODY()

	/// Enable visual mesh deformation (can disable for performance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisualDamageEnabled = true;

	/// Enable mechanical/performance damage (false = cosmetic only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMechanicalDamageEnabled = true;

	/// Global damage multiplier (0.5 = half damage, 2.0 = double damage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	/// Minimum collision speed (km/h) required to cause damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinImpactVelocityForDamage = 20.0f;

	/// Health percentage at which vehicle is considered totaled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotaledHealthThreshold = 10.0f;

	/// Health percentage at which critical damage effects begin
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalHealthThreshold = 25.0f;

	/// Allow body parts to detach from severe damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPartDetachment = true;

	/// Zone damage percentage required for parts to detach
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PartDetachmentThreshold = 75.0f;

	/// Allow vehicles to catch fire from damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowFire = true;

	/// Zone damage percentage required for fire ignition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireIgnitionThreshold = 50.0f;

	/// How much zone damage spreads to connected components (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComponentDamageSpreadFactor = 0.3f;

	/// Automatically repair vehicle when respawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoRepairOnRespawn = true;
};

/**
 * @brief Options for selective vehicle repair.
 *
 * Allows partial repairs targeting specific zones or components.
 */
USTRUCT(BlueprintType)
struct FMGRepairOptions
{
	GENERATED_BODY()

	/// Repair visual/cosmetic damage (dents, deformation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairVisual = true;

	/// Repair mechanical component damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairMechanical = true;

	/// If true, repair all zones; if false, use SpecificZones list
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairAllZones = true;

	/// Specific zones to repair (only used if bRepairAllZones is false)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGDamageZone> SpecificZones;

	/// If true, repair all components; if false, use SpecificComponents list
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairAllComponents = true;

	/// Specific components to repair (only used if bRepairAllComponents is false)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGVehicleComponent> SpecificComponents;

	/// How much to repair (100 = full repair, 50 = half repair)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RepairPercentage = 100.0f;
};

/**
 * @brief Mesh deformation data for visual damage.
 *
 * Stores vertex offset information for deforming the vehicle mesh
 * to show crumple damage. Used by the rendering system.
 */
USTRUCT(BlueprintType)
struct FMGDeformationData
{
	GENERATED_BODY()

	/// Full list of deformed vertex positions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> DeformedVertices;

	/// Map of vertex index to offset vector (sparse representation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, FVector> VertexOffsets;

	/// Maximum depth any vertex has been pushed inward
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDeformationDepth = 0.0f;

	/// Center point of the deformation area
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DeformationCenter = FVector::ZeroVector;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/// Fired when any damage is applied to a vehicle
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageReceived, FName, VehicleID, const FMGDamageInstance&, Damage);

/// Fired when a body zone's severity level changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneDamaged, EMGDamageZone, Zone, EMGDamageSeverity, Severity);

/// Fired when a component takes damage
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComponentDamaged, EMGVehicleComponent, Component, float, HealthRemaining);

/// Fired when a component becomes completely disabled
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentDisabled, EMGVehicleComponent, Component);

/// Fired when a body part detaches from the vehicle
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartDetached, FName, VehicleID, FName, PartName);

/// Fired when a vehicle is totaled (destroyed beyond repair)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleTotaled, FName, VehicleID);

/// Fired when a vehicle catches fire
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleOnFire, FName, VehicleID);

/// Fired when repairs are completed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleRepaired, FName, VehicleID, float, HealthRestored);

/// Fired whenever vehicle health changes (for UI updates)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, FName, VehicleID, float, OldHealth, float, NewHealth);

// ============================================================================
// SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Game instance subsystem managing vehicle damage simulation.
 *
 * UMGDamageSubsystem provides a comprehensive damage model for racing vehicles.
 * It tracks localized damage across body zones, propagates damage to mechanical
 * components, and calculates performance penalties.
 *
 * ## Core Concepts
 *
 * **Zone-Based Damage Model**:
 * The vehicle body is divided into 10 zones. Collisions are mapped to zones
 * based on impact location. Each zone has its own health pool and severity.
 *
 * **Component Damage Propagation**:
 * When a zone takes damage, a portion spreads to components in that zone:
 * - Front zones affect: Engine, Radiator, Steering, Electronics
 * - Side zones affect: Suspension, Tires, FuelTank
 * - Rear zones affect: Transmission, Exhaust, FuelTank, NitroSystem
 *
 * **Performance Multipliers**:
 * Component damage reduces performance via multipliers (0.0 to 1.0):
 * - Engine damage -> Power output
 * - Transmission damage -> Acceleration
 * - Suspension damage -> Handling
 * - Brake damage -> Braking force
 *
 * ## Usage Pattern
 * 1. Register vehicles with RegisterVehicle() at spawn
 * 2. Apply damage via ApplyCollisionDamage() or ApplyDamage()
 * 3. Query performance multipliers each tick to modify physics
 * 4. Listen to events for UI/audio/visual feedback
 * 5. Repair via RepairVehicle() at garages
 *
 * ## Thread Safety
 * All functions should be called from the game thread only.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDamageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// LIFECYCLE
	// ==========================================

	/** @brief Initialize subsystem and load configuration. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** @brief Cleanup and save any pending data. */
	virtual void Deinitialize() override;

	/** @brief Determine if subsystem should be created (always true for game). */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// DAMAGE APPLICATION
	// ==========================================

	/**
	 * @brief Apply a pre-configured damage instance to a vehicle.
	 * @param VehicleID Target vehicle identifier.
	 * @param Damage Damage instance with all parameters set.
	 * @return The damage instance with FinalDamage and Severity populated.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	FMGDamageInstance ApplyDamage(FName VehicleID, const FMGDamageInstance& Damage);

	/**
	 * @brief Apply collision damage from physics impact data.
	 *
	 * This is the primary damage entry point for collision events. It automatically
	 * determines damage type and zone from impact geometry.
	 *
	 * @param VehicleID Target vehicle identifier.
	 * @param ImpactPoint World-space collision location.
	 * @param ImpactNormal Surface normal at impact.
	 * @param ImpactVelocity Speed at impact in km/h.
	 * @param InstigatorID ID of hitting vehicle/object (NAME_None for environment).
	 * @return The calculated and applied damage instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	FMGDamageInstance ApplyCollisionDamage(FName VehicleID, const FVector& ImpactPoint, const FVector& ImpactNormal, float ImpactVelocity, FName InstigatorID = NAME_None);

	/**
	 * @brief Apply damage directly to a specific zone.
	 * @param VehicleID Target vehicle.
	 * @param Zone Zone to damage.
	 * @param DamageAmount Raw damage points.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	void ApplyZoneDamage(FName VehicleID, EMGDamageZone Zone, float DamageAmount);

	/**
	 * @brief Apply damage directly to a specific component.
	 * @param VehicleID Target vehicle.
	 * @param Component Component to damage.
	 * @param DamageAmount Raw damage points.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	void ApplyComponentDamage(FName VehicleID, EMGVehicleComponent Component, float DamageAmount);

	/**
	 * @brief Apply environmental/special damage (EMP, spike strips, etc.).
	 * @param VehicleID Target vehicle.
	 * @param DamageType Type of environmental damage.
	 * @param DamageAmount Raw damage points.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	void ApplyEnvironmentalDamage(FName VehicleID, EMGDamageType DamageType, float DamageAmount);

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/**
	 * @brief Get the complete damage state for a vehicle.
	 * @param VehicleID Vehicle to query.
	 * @return Copy of the vehicle's damage state.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	FMGVehicleDamageState GetVehicleDamageState(FName VehicleID) const;

	/**
	 * @brief Get current health points.
	 * @param VehicleID Vehicle to query.
	 * @return Current health (0 to MaxHealth).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	float GetVehicleHealth(FName VehicleID) const;

	/**
	 * @brief Get health as a percentage.
	 * @param VehicleID Vehicle to query.
	 * @return Health percentage (0.0 to 1.0).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	float GetVehicleHealthPercent(FName VehicleID) const;

	/**
	 * @brief Get overall damage severity level.
	 * @param VehicleID Vehicle to query.
	 * @return Highest severity across all zones.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	EMGDamageSeverity GetVehicleSeverity(FName VehicleID) const;

	/**
	 * @brief Check if vehicle can still be driven.
	 * @param VehicleID Vehicle to query.
	 * @return True if vehicle has functional drivetrain.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	bool IsVehicleDriveable(FName VehicleID) const;

	/**
	 * @brief Check if vehicle is totaled (destroyed).
	 * @param VehicleID Vehicle to query.
	 * @return True if damage exceeds totaled threshold.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	bool IsVehicleTotaled(FName VehicleID) const;

	// ==========================================
	// ZONE QUERIES
	// ==========================================

	/**
	 * @brief Get damage state for a specific zone.
	 * @param VehicleID Vehicle to query.
	 * @param Zone Zone to query.
	 * @return Zone damage state.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Zone")
	FMGZoneDamageState GetZoneDamageState(FName VehicleID, EMGDamageZone Zone) const;

	/**
	 * @brief Get zone damage as percentage.
	 * @param VehicleID Vehicle to query.
	 * @param Zone Zone to query.
	 * @return Damage percentage (0.0 = pristine, 1.0 = destroyed).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Zone")
	float GetZoneDamagePercent(FName VehicleID, EMGDamageZone Zone) const;

	/**
	 * @brief Find the zone with the most damage.
	 * @param VehicleID Vehicle to query.
	 * @return Zone with highest damage percentage.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Zone")
	EMGDamageZone GetMostDamagedZone(FName VehicleID) const;

	/**
	 * @brief Get list of all detached parts.
	 * @param VehicleID Vehicle to query.
	 * @return Array of part names that have fallen off.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Zone")
	TArray<FName> GetDetachedParts(FName VehicleID) const;

	// ==========================================
	// COMPONENT QUERIES
	// ==========================================

	/**
	 * @brief Get damage state for a specific component.
	 * @param VehicleID Vehicle to query.
	 * @param Component Component to query.
	 * @return Component damage state.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Component")
	FMGComponentDamageState GetComponentDamageState(FName VehicleID, EMGVehicleComponent Component) const;

	/**
	 * @brief Get component efficiency multiplier.
	 * @param VehicleID Vehicle to query.
	 * @param Component Component to query.
	 * @return Efficiency (0.0 = disabled, 1.0 = full function).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Component")
	float GetComponentEfficiency(FName VehicleID, EMGVehicleComponent Component) const;

	/**
	 * @brief Check if component is still functioning.
	 * @param VehicleID Vehicle to query.
	 * @param Component Component to query.
	 * @return True if component is working (even if degraded).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Component")
	bool IsComponentFunctional(FName VehicleID, EMGVehicleComponent Component) const;

	/**
	 * @brief Get list of completely disabled components.
	 * @param VehicleID Vehicle to query.
	 * @return Array of components with 0% efficiency.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Component")
	TArray<EMGVehicleComponent> GetDisabledComponents(FName VehicleID) const;

	// ==========================================
	// PERFORMANCE IMPACT
	// ==========================================

	/**
	 * @brief Get engine power multiplier.
	 * @param VehicleID Vehicle to query.
	 * @return Multiplier for horsepower/torque (0.0 to 1.0).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetEnginePerformanceMultiplier(FName VehicleID) const;

	/**
	 * @brief Get handling/cornering multiplier.
	 * @param VehicleID Vehicle to query.
	 * @return Multiplier for steering response and grip (0.0 to 1.0).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetHandlingMultiplier(FName VehicleID) const;

	/**
	 * @brief Get top speed multiplier.
	 * @param VehicleID Vehicle to query.
	 * @return Multiplier for maximum velocity (0.0 to 1.0).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetTopSpeedMultiplier(FName VehicleID) const;

	/**
	 * @brief Get acceleration multiplier.
	 * @param VehicleID Vehicle to query.
	 * @return Multiplier for acceleration rate (0.0 to 1.0).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetAccelerationMultiplier(FName VehicleID) const;

	/**
	 * @brief Get braking power multiplier.
	 * @param VehicleID Vehicle to query.
	 * @return Multiplier for braking force (0.0 to 1.0).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetBrakingMultiplier(FName VehicleID) const;

	/**
	 * @brief Get nitrous system efficiency.
	 * @param VehicleID Vehicle to query.
	 * @return Multiplier for nitrous boost effect (0.0 to 1.0).
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetNitroEfficiency(FName VehicleID) const;

	// ==========================================
	// REPAIR
	// ==========================================

	/**
	 * @brief Repair vehicle with specific options.
	 * @param VehicleID Vehicle to repair.
	 * @param Options Repair configuration (zones, components, amount).
	 * @return Cost of the repair in game currency.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Repair")
	int32 RepairVehicle(FName VehicleID, const FMGRepairOptions& Options);

	/**
	 * @brief Fully repair all damage on a vehicle.
	 * @param VehicleID Vehicle to repair.
	 * @return Total cost of full repair.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Repair")
	int32 RepairVehicleFull(FName VehicleID);

	/**
	 * @brief Repair a single body zone.
	 * @param VehicleID Vehicle to repair.
	 * @param Zone Zone to repair.
	 * @return Cost of repairing this zone.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Repair")
	int32 RepairZone(FName VehicleID, EMGDamageZone Zone);

	/**
	 * @brief Repair a single component.
	 * @param VehicleID Vehicle to repair.
	 * @param Component Component to repair.
	 * @return Cost of repairing this component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Repair")
	int32 RepairComponent(FName VehicleID, EMGVehicleComponent Component);

	/**
	 * @brief Get total cost to fully repair a vehicle.
	 * @param VehicleID Vehicle to query.
	 * @return Repair cost in game currency.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Repair")
	int32 GetRepairCost(FName VehicleID) const;

	/**
	 * @brief Get cost to repair a specific zone.
	 * @param VehicleID Vehicle to query.
	 * @param Zone Zone to query.
	 * @return Zone repair cost.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Repair")
	int32 GetZoneRepairCost(FName VehicleID, EMGDamageZone Zone) const;

	/**
	 * @brief Get cost to repair a specific component.
	 * @param VehicleID Vehicle to query.
	 * @param Component Component to query.
	 * @return Component repair cost.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Repair")
	int32 GetComponentRepairCost(FName VehicleID, EMGVehicleComponent Component) const;

	// ==========================================
	// VEHICLE REGISTRATION
	// ==========================================

	/**
	 * @brief Register a vehicle with the damage system.
	 *
	 * Must be called when a vehicle spawns before any damage can be applied.
	 * Initializes all zones and components to full health.
	 *
	 * @param VehicleID Unique identifier for the vehicle.
	 * @param MaxHealth Maximum health pool (default 100).
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Registration")
	void RegisterVehicle(FName VehicleID, float MaxHealth = 100.0f);

	/**
	 * @brief Unregister a vehicle from the damage system.
	 * @param VehicleID Vehicle to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Registration")
	void UnregisterVehicle(FName VehicleID);

	/**
	 * @brief Reset all damage on a vehicle (free repair).
	 * @param VehicleID Vehicle to reset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Registration")
	void ResetVehicleDamage(FName VehicleID);

	// ==========================================
	// DEFORMATION
	// ==========================================

	/**
	 * @brief Get mesh deformation data for a zone.
	 * @param VehicleID Vehicle to query.
	 * @param Zone Zone to get deformation for.
	 * @return Deformation vertex data.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Deformation")
	FMGDeformationData GetDeformationData(FName VehicleID, EMGDamageZone Zone) const;

	/**
	 * @brief Add a deformation point from an impact.
	 * @param VehicleID Vehicle to deform.
	 * @param Zone Zone being deformed.
	 * @param ImpactPoint Local-space impact location.
	 * @param Depth How deep the deformation goes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Deformation")
	void AddDeformation(FName VehicleID, EMGDamageZone Zone, const FVector& ImpactPoint, float Depth);

	// ==========================================
	// FIRE
	// ==========================================

	/**
	 * @brief Start a fire on a vehicle.
	 * @param VehicleID Vehicle to ignite.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Fire")
	void IgniteVehicle(FName VehicleID);

	/**
	 * @brief Extinguish fire on a vehicle.
	 * @param VehicleID Vehicle to extinguish.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Fire")
	void ExtinguishVehicle(FName VehicleID);

	/**
	 * @brief Check if a vehicle is currently on fire.
	 * @param VehicleID Vehicle to check.
	 * @return True if on fire.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Fire")
	bool IsVehicleOnFire(FName VehicleID) const;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * @brief Set damage system configuration.
	 * @param NewConfig New configuration to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Config")
	void SetDamageConfig(const FMGDamageConfig& NewConfig);

	/**
	 * @brief Get current damage configuration.
	 * @return Current config copy.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage|Config")
	FMGDamageConfig GetDamageConfig() const { return Config; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** @brief Fired when any damage is received. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnDamageReceived OnDamageReceived;

	/** @brief Fired when zone severity changes. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnZoneDamaged OnZoneDamaged;

	/** @brief Fired when component takes damage. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnComponentDamaged OnComponentDamaged;

	/** @brief Fired when component becomes disabled. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnComponentDisabled OnComponentDisabled;

	/** @brief Fired when body part detaches. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnPartDetached OnPartDetached;

	/** @brief Fired when vehicle is totaled. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnVehicleTotaled OnVehicleTotaled;

	/** @brief Fired when vehicle catches fire. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnVehicleOnFire OnVehicleOnFire;

	/** @brief Fired when repairs complete. */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnVehicleRepaired OnVehicleRepaired;

	/** @brief Fired when health changes (for HUD). */
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnHealthChanged OnHealthChanged;

protected:
	// ==========================================
	// INTERNAL HELPERS
	// ==========================================

	/**
	 * @brief Determine which zone an impact hit based on geometry.
	 * @param ImpactPoint World-space impact location.
	 * @param VehicleForward Vehicle's forward vector.
	 * @param VehicleRight Vehicle's right vector.
	 * @return Calculated impact zone.
	 */
	EMGDamageZone CalculateImpactZone(const FVector& ImpactPoint, const FVector& VehicleForward, const FVector& VehicleRight) const;

	/**
	 * @brief Determine collision type from impact angle.
	 * @param ImpactNormal Surface normal at impact.
	 * @param VehicleForward Vehicle's forward vector.
	 * @return Detected collision type.
	 */
	EMGDamageType DetermineCollisionType(const FVector& ImpactNormal, const FVector& VehicleForward) const;

	/**
	 * @brief Calculate severity from damage percentage.
	 * @param DamagePercent Damage as percentage of max (0.0 to 1.0).
	 * @return Corresponding severity level.
	 */
	EMGDamageSeverity CalculateSeverity(float DamagePercent) const;

	/**
	 * @brief Apply damage multipliers and armor to raw damage.
	 * @param Damage Damage instance to process.
	 * @return Final damage value after modifiers.
	 */
	float CalculateFinalDamage(const FMGDamageInstance& Damage) const;

	/**
	 * @brief Propagate zone damage to affected components.
	 * @param VehicleID Target vehicle.
	 * @param Zone Zone that was damaged.
	 * @param DamageAmount Damage to spread.
	 */
	void SpreadComponentDamage(FName VehicleID, EMGDamageZone Zone, float DamageAmount);

	/**
	 * @brief Check if any parts should detach from zone damage.
	 * @param VehicleID Target vehicle.
	 * @param Zone Zone to check.
	 */
	void CheckPartDetachment(FName VehicleID, EMGDamageZone Zone);

	/**
	 * @brief Check if damage should ignite the vehicle.
	 * @param VehicleID Target vehicle.
	 */
	void CheckFireIgnition(FName VehicleID);

	/**
	 * @brief Recalculate overall state from all zones/components.
	 * @param VehicleID Vehicle to update.
	 */
	void UpdateOverallState(FName VehicleID);

	/**
	 * @brief Calculate repair cost based on damage and severity.
	 * @param DamageAmount Amount of damage to repair.
	 * @param Severity Severity level (affects cost multiplier).
	 * @return Repair cost in game currency.
	 */
	int32 CalculateRepairCost(float DamageAmount, EMGDamageSeverity Severity) const;

	/**
	 * @brief Initialize a new vehicle damage state.
	 * @param State State to initialize.
	 * @param MaxHealth Maximum health value.
	 */
	void InitializeVehicleState(FMGVehicleDamageState& State, float MaxHealth);

	// ==========================================
	// DATA
	// ==========================================

	/** @brief Damage states for all registered vehicles. */
	UPROPERTY()
	TMap<FName, FMGVehicleDamageState> VehicleDamageStates;

	/** @brief Deformation data per vehicle per zone. */
	UPROPERTY()
	TMap<FName, TMap<EMGDamageZone, FMGDeformationData>> VehicleDeformation;

	/** @brief Current damage system configuration. */
	UPROPERTY()
	FMGDamageConfig Config;
};

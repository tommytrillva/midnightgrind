// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehicleDamageSystem.h
 * @brief Vehicle damage component for collision damage, component health, and repairs.
 *
 * @section Overview
 * This component handles all immediate damage from collisions, impacts, and hazards.
 * Unlike the wear system (gradual degradation), this handles sudden damage events
 * that occur from crashes, scrapes, and environmental hazards.
 *
 * @section Architecture
 * The damage system uses a zone-based approach where the vehicle is divided into
 * regions (Front, Rear, Left, Right, Top, Bottom) and damage is calculated based
 * on which zone was hit. Each zone can have different resistance values.
 *
 * Component Architecture:
 * - **Zones**: Physical areas of the vehicle (Front, FrontLeft, etc.)
 * - **Components**: Functional systems that can be damaged (Engine, Transmission, etc.)
 * - **Visual State**: Cosmetic damage (deformation, scratches, broken lights)
 *
 * Damage Flow:
 * 1. Collision detected via NotifyHit or ApplyCollisionDamage
 * 2. Zone determined from hit location
 * 3. Resistance applied based on zone and upgrades
 * 4. Damage propagated to relevant components
 * 5. Visual damage updated for rendering
 * 6. Performance effects applied to movement component
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Damage Zones**: The vehicle is divided into logical regions. A frontal
 * collision affects the Front zone, which may damage the Engine and Cooling
 * components. A rear collision affects the Rear zone, potentially damaging
 * the fuel tank or trunk-mounted components.
 *
 * **Component Health**: Each functional component (Engine, Transmission, Brakes)
 * has its own health value (0-100). When damaged:
 * - 100%: Full performance
 * - 75-99%: Minor performance loss
 * - 50-74%: Noticeable degradation
 * - 25-49%: Severe issues
 * - <25%: Component may fail completely
 *
 * **Performance Multiplier**: A value (0-1) applied to component effectiveness.
 * A damaged engine might have 0.8 multiplier = 80% power output.
 *
 * **Damage Resistance**: Some vehicles or upgrades provide damage resistance.
 * A value of 0.25 means 25% of damage is absorbed (only 75% applied).
 *
 * **Visual Damage**: Separate from functional damage, this tracks cosmetic state:
 * - Deformation: Mesh vertex displacement from impacts
 * - Scratches: Surface damage from scrapes
 * - Broken lights: Individual light state
 * - Smoke/Fire: Critical damage visual effects
 *
 * @section Usage Example Usage
 * @code
 * // The damage system is a component on AMGVehiclePawn
 * // It receives collision callbacks automatically
 *
 * // Manual damage application (for hazards, weapons, etc.):
 * DamageSystem->ApplyZoneDamage(EMGDamageZone::Front, 25.0f);
 * DamageSystem->ApplyComponentDamage(EMGDamageComponent::Engine, 10.0f);
 *
 * // Query damage state for HUD:
 * float OverallDamage = DamageSystem->GetOverallDamagePercent();
 * FMGComponentDamageState EngineState = DamageSystem->GetComponentState(EMGDamageComponent::Engine);
 *
 * // Check if component is broken:
 * if (DamageSystem->IsComponentBroken(EMGDamageComponent::Transmission))
 * {
 *     // Transmission is non-functional!
 * }
 *
 * // Repair at garage:
 * DamageSystem->InstantRepair(EMGDamageComponent::Engine);
 * DamageSystem->InstantRepairAll(); // Full repair
 *
 * // Bind to damage events:
 * DamageSystem->OnDamageTaken.AddDynamic(this, &AMyClass::HandleDamage);
 * DamageSystem->OnComponentBroken.AddDynamic(this, &AMyClass::HandleComponentFailure);
 * @endcode
 *
 * @see UMGVehicleWearSubsystem For gradual wear over time
 * @see UMGVehicleMovementComponent For performance effect application
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGVehicleDamageSystem.generated.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class AMGVehiclePawn;
class UMGVehicleMovementComponent;

// ============================================================================
// DAMAGE ZONE ENUMERATION
// ============================================================================

/**
 * @brief Physical damage zones on the vehicle body.
 *
 * The vehicle is divided into distinct zones for damage calculation.
 * Impact location is mapped to the nearest zone, which determines:
 * - Which components receive damage
 * - Resistance values applied
 * - Visual deformation regions
 *
 * Zone layout (top-down view):
 *
 *     FrontLeft --- Front --- FrontRight
 *         |                       |
 *       Left      (Top/Bottom)   Right
 *         |                       |
 *     RearLeft ---- Rear ---- RearRight
 */
UENUM(BlueprintType)
enum class EMGDamageZone : uint8
{
	Front,
	FrontLeft,
	FrontRight,
	Left,
	Right,
	RearLeft,
	RearRight,
	Rear,
	Top,
	Bottom
};

// ============================================================================
// DAMAGEABLE COMPONENT ENUMERATION
// ============================================================================

/**
 * @brief Functional vehicle components that can receive damage.
 *
 * Each component has independent health and affects specific performance
 * aspects when damaged. The mapping between zones and components is:
 * - Front zone: Engine, Cooling, Aero
 * - Rear zone: NOS (if trunk-mounted), Aero
 * - Side zones: Suspension, Wheels
 * - All zones can affect Body
 */
UENUM(BlueprintType)
enum class EMGDamageComponent : uint8
{
	/** Body/chassis */
	Body,
	/** Engine performance */
	Engine,
	/** Transmission */
	Transmission,
	/** Suspension */
	Suspension,
	/** Steering */
	Steering,
	/** Brakes */
	Brakes,
	/** Wheels/tires */
	Wheels,
	/** Aerodynamics (downforce) */
	Aero,
	/** Cooling system */
	Cooling,
	/** NOS system */
	NOS
};

// ============================================================================
// COMPONENT DAMAGE STATE STRUCTURE
// ============================================================================

/**
 * @brief Runtime state of a single damageable component.
 *
 * Contains health, performance multiplier, repair state, and broken flag.
 * Used for both internal state tracking and UI display.
 */
USTRUCT(BlueprintType)
struct FMGComponentDamageState
{
	GENERATED_BODY()

	/** Component type */
	UPROPERTY(BlueprintReadOnly)
	EMGDamageComponent Component = EMGDamageComponent::Body;

	/** Current health (0-100) */
	UPROPERTY(BlueprintReadOnly)
	float Health = 100.0f;

	/** Is component fully broken */
	UPROPERTY(BlueprintReadOnly)
	bool bIsBroken = false;

	/** Performance multiplier (1.0 = full, lower = damaged) */
	UPROPERTY(BlueprintReadOnly)
	float PerformanceMultiplier = 1.0f;

	/** Is currently being repaired */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRepairing = false;

	/** Repair progress (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float RepairProgress = 0.0f;
};

// ============================================================================
// VISUAL DAMAGE STATE STRUCTURE
// ============================================================================

/**
 * @brief Cosmetic damage state for rendering and VFX.
 *
 * Separate from functional damage, this tracks visual-only state
 * like deformation, scratches, and broken lights. Used by the
 * rendering system to display damage without affecting physics.
 */
USTRUCT(BlueprintType)
struct FMGVisualDamageState
{
	GENERATED_BODY()

	/** Deformation amount per zone (0-1) */
	UPROPERTY(BlueprintReadOnly)
	TMap<EMGDamageZone, float> ZoneDeformation;

	/** Scratch/paint damage per zone (0-1) */
	UPROPERTY(BlueprintReadOnly)
	TMap<EMGDamageZone, float> ZoneScratchDamage;

	/** Are headlights broken */
	UPROPERTY(BlueprintReadOnly)
	bool bHeadlightsBroken = false;

	/** Are taillights broken */
	UPROPERTY(BlueprintReadOnly)
	bool bTaillightsBroken = false;

	/** Window damage (0-1, 1 = fully shattered) */
	UPROPERTY(BlueprintReadOnly)
	float WindowDamage = 0.0f;

	/** Is smoking from engine */
	UPROPERTY(BlueprintReadOnly)
	bool bIsSmoking = false;

	/** Is on fire */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOnFire = false;
};

// ============================================================================
// DAMAGE EVENT STRUCTURE
// ============================================================================

/**
 * @brief Data passed with damage events for effect spawning and logging.
 *
 * Contains all information about a damage event including impact details,
 * zone hit, damage amount, and involved actors. Passed to delegates so
 * listeners can spawn appropriate effects and sounds.
 */
USTRUCT(BlueprintType)
struct FMGDamageEvent
{
	GENERATED_BODY()

	/** Impact force */
	UPROPERTY(BlueprintReadOnly)
	float ImpactForce = 0.0f;

	/** Impact location (world space) */
	UPROPERTY(BlueprintReadOnly)
	FVector ImpactLocation = FVector::ZeroVector;

	/** Impact normal */
	UPROPERTY(BlueprintReadOnly)
	FVector ImpactNormal = FVector::UpVector;

	/** Damage zone hit */
	UPROPERTY(BlueprintReadOnly)
	EMGDamageZone DamageZone = EMGDamageZone::Front;

	/** Other actor involved */
	UPROPERTY(BlueprintReadOnly)
	AActor* OtherActor = nullptr;

	/** Was collision with another vehicle */
	UPROPERTY(BlueprintReadOnly)
	bool bWasVehicleCollision = false;

	/** Damage dealt to body */
	UPROPERTY(BlueprintReadOnly)
	float DamageDealt = 0.0f;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/**
 * @brief Multicast delegates for damage system events.
 *
 * These delegates allow other systems (VFX, SFX, HUD, AI) to respond
 * to damage events without tight coupling. Bind in BeginPlay, unbind
 * in EndPlay to avoid dangling references.
 *
 * **DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(...)**
 * - Dynamic: Can be bound from Blueprint
 * - Multicast: Multiple listeners can subscribe
 * - OneParam/TwoParams: Number of parameters passed to bound functions
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageTaken, const FMGDamageEvent&, DamageEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComponentDamaged, EMGDamageComponent, Component, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentBroken, EMGDamageComponent, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentRepaired, EMGDamageComponent, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleTotaled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisualDamageUpdated, const FMGVisualDamageState&, VisualState);

// ============================================================================
// VEHICLE DAMAGE SYSTEM CLASS
// ============================================================================

/**
 * @class UMGVehicleDamageSystem
 * @brief Actor component handling collision damage and repairs.
 *
 * This component is attached to the vehicle pawn and receives collision
 * callbacks. It manages all immediate damage from impacts and provides
 * repair functionality for gameplay.
 *
 * @section Features Features
 * - **Zone-Based Damage**: Impact location determines affected components
 * - **Component Health**: Independent health with performance effects
 * - **Visual Damage**: Deformation, scratches, broken lights, smoke/fire
 * - **Repair System**: Timed or instant repair with cost calculation
 * - **Resistance System**: Per-zone and global damage resistance
 * - **Auto-Repair**: Optional slow healing when stationary
 *
 * @section UnrealMacros Unreal Engine Macro Explanations
 *
 * **UCLASS(ClassGroup=(Vehicle), meta=(BlueprintSpawnableComponent))**
 * - ClassGroup=(Vehicle): Groups with other vehicle components in editor
 * - BlueprintSpawnableComponent: Can be added to actors via Blueprint
 *
 * **UPROPERTY(BlueprintAssignable, Category = "...")**
 * - BlueprintAssignable: This delegate can be bound in Blueprint event graphs
 * - Used for events that Blueprint needs to respond to
 *
 * **TMap<K,V>**: Unreal's dictionary type, maps keys to values.
 * Used here to store per-zone resistance and per-component damage states.
 *
 * **TWeakObjectPtr<T>**: A "weak" pointer that doesn't prevent garbage collection.
 * Used for cached references that might become invalid (actor destroyed).
 */
UCLASS(ClassGroup=(Vehicle), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGVehicleDamageSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGVehicleDamageSystem();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Base damage resistance (reduces all damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Resistance")
	float BaseDamageResistance = 0.0f;

	/** Damage resistance per zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Resistance")
	TMap<EMGDamageZone, float> ZoneDamageResistance;

	/** Impact force threshold to cause damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Thresholds")
	float MinImpactForceForDamage = 10.0f;

	/** Impact force for maximum damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Thresholds")
	float MaxImpactForce = 100.0f;

	/** Maximum body damage (health at which vehicle is totaled) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Thresholds")
	float TotaledThreshold = 0.0f;

	/** Component damage multipliers (how much damage transfers to each component) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Components")
	TMap<EMGDamageComponent, float> ComponentDamageMultipliers;

	/** Auto-repair when stationary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Repair")
	bool bAutoRepairWhenStationary = false;

	/** Stationary time before auto-repair starts (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Repair")
	float AutoRepairDelay = 5.0f;

	/** Auto-repair rate (health per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Repair")
	float AutoRepairRate = 5.0f;

	/** Enable visual damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visual")
	bool bEnableVisualDamage = true;

	/** Deformation multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visual")
	float DeformationMultiplier = 1.0f;

	// ==========================================
	// DAMAGE APPLICATION
	// ==========================================

	/** Apply damage from collision */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyCollisionDamage(const FHitResult& HitResult, float ImpactForce, AActor* OtherActor);

	/** Apply direct damage to a zone */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyZoneDamage(EMGDamageZone Zone, float DamageAmount);

	/** Apply direct damage to a component */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyComponentDamage(EMGDamageComponent Component, float DamageAmount);

	/** Apply percentage damage to all components */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyGlobalDamage(float DamagePercent);

	// ==========================================
	// REPAIR
	// ==========================================

	/** Start repairing a component */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void StartRepair(EMGDamageComponent Component, float RepairDuration);

	/** Repair component instantly */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void InstantRepair(EMGDamageComponent Component);

	/** Repair all components instantly */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void InstantRepairAll();

	/** Cancel ongoing repair */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void CancelRepair(EMGDamageComponent Component);

	/** Get repair cost for component */
	UFUNCTION(BlueprintPure, Category = "Repair")
	int32 GetRepairCost(EMGDamageComponent Component) const;

	/** Get total repair cost */
	UFUNCTION(BlueprintPure, Category = "Repair")
	int32 GetTotalRepairCost() const;

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/** Get overall damage percent (0-100, 100 = totaled) */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	float GetOverallDamagePercent() const;

	/** Get component damage state */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	FMGComponentDamageState GetComponentState(EMGDamageComponent Component) const;

	/** Get all component states */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	TArray<FMGComponentDamageState> GetAllComponentStates() const;

	/** Get visual damage state */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	FMGVisualDamageState GetVisualDamageState() const { return VisualDamage; }

	/** Is component broken */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	bool IsComponentBroken(EMGDamageComponent Component) const;

	/** Is vehicle totaled */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	bool IsVehicleTotaled() const { return bIsTotaled; }

	/** Is any repair in progress */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	bool IsRepairing() const;

	/** Get performance multiplier for component */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	float GetComponentPerformance(EMGDamageComponent Component) const;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnDamageTaken OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnComponentDamaged OnComponentDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnComponentBroken OnComponentBroken;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnComponentRepaired OnComponentRepaired;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnVehicleTotaled OnVehicleTotaled;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnVisualDamageUpdated OnVisualDamageUpdated;

	/** Called when scraping starts */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScrapeStart, FVector, ContactPoint, float, Intensity);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnScrapeStart OnScrapeStart;

	/** Called when scraping ends */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScrapeEnd);
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnScrapeEnd OnScrapeEnd;

	/** Is currently scraping */
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	bool IsScraping() const { return bIsScraping; }

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Initialize component states */
	void InitializeComponents();

	/** Determine damage zone from hit location */
	EMGDamageZone DetermineZoneFromHit(const FVector& LocalHitLocation) const;

	/** Calculate damage after resistance */
	float CalculateDamageAfterResistance(float RawDamage, EMGDamageZone Zone) const;

	/** Propagate damage to components based on zone */
	void PropagateToComponents(EMGDamageZone Zone, float Damage);

	/** Update component performance multiplier */
	void UpdateComponentPerformance(EMGDamageComponent Component);

	/** Apply performance effects to vehicle */
	void ApplyPerformanceEffects();

	/** Update visual damage state */
	void UpdateVisualDamage(EMGDamageZone Zone, float Damage);

	/** Check if vehicle should be totaled */
	void CheckTotaledState();

	/** Update auto-repair */
	void UpdateAutoRepair(float DeltaTime);

	/** Update ongoing repairs */
	void UpdateRepairs(float DeltaTime);

private:
	// ==========================================
	// STATE
	// ==========================================

	/** Component damage states */
	TMap<EMGDamageComponent, FMGComponentDamageState> ComponentStates;

	/** Visual damage state */
	FMGVisualDamageState VisualDamage;

	/** Is vehicle totaled */
	bool bIsTotaled = false;

	/** Time spent stationary (for auto-repair) */
	float StationaryTime = 0.0f;

	/** Was stationary last frame */
	bool bWasStationary = false;

	/** Is currently scraping against something */
	bool bIsScraping = false;

	/** Time since last collision (for scrape detection) */
	float TimeSinceLastCollision = 0.0f;

	/** Recent collision count for scrape detection */
	int32 RecentCollisionCount = 0;

	/** Last scrape contact point */
	FVector LastScrapePoint = FVector::ZeroVector;

	/** Scrape detection window (seconds) */
	float ScrapeDetectionWindow = 0.2f;

	// ==========================================
	// REFERENCES
	// ==========================================

	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> VehiclePawn;

	UPROPERTY()
	TWeakObjectPtr<UMGVehicleMovementComponent> MovementComponent;
};

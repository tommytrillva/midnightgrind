// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGVehicleDamageSystem.generated.h"

class AMGVehiclePawn;
class UMGVehicleMovementComponent;

/**
 * Damage zone on vehicle
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

/**
 * Component damage types
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

/**
 * Damage state of a component
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

/**
 * Visual damage state
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

/**
 * Damage event data
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

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageTaken, const FMGDamageEvent&, DamageEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComponentDamaged, EMGDamageComponent, Component, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentBroken, EMGDamageComponent, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentRepaired, EMGDamageComponent, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleTotaled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisualDamageUpdated, const FMGVisualDamageState&, VisualState);

/**
 * Vehicle Damage System
 * Handles collision damage, component degradation, and repairs
 *
 * Features:
 * - Zone-based damage calculation
 * - Component health with performance effects
 * - Visual damage (deformation, scratches)
 * - Repair mechanics
 * - Damage resistance/armor support
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

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGPoliceRoadblock.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UAudioComponent;
class UNiagaraComponent;

/**
 * @brief Broadcast when a vehicle collides with the roadblock.
 * @param HitActor The actor that hit the roadblock
 * @param ImpactSpeed Estimated speed at impact in MPH
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoadblockHit, AActor*, HitActor, float, ImpactSpeed);

/**
 * @brief Broadcast when roadblock is breached/destroyed.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoadblockDestroyed);

/**
 * @brief Broadcast when roadblock is fully deployed and active.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadblockDeployed, int32, BlockadeID);

/**
 * @class AMGPoliceRoadblock
 * @brief Police roadblock obstacle actor for pursuit mechanics.
 *
 * Roadblocks are spawned during high heat level pursuits to create
 * obstacles that players must either avoid, breach at high speed,
 * or find alternate routes around. They consist of police vehicles
 * and barricades positioned to block roads.
 *
 * Key Features:
 * - Configurable number of blocking vehicles (1-5)
 * - Health-based breach system (requires high speed or multiple hits)
 * - Dynamic width based on vehicle count
 * - Visual/audio feedback (police lights, radio chatter)
 * - Speed threshold for successful breach
 * - Damage feedback to player vehicle on impact
 *
 * Usage:
 * - Spawned by MGPoliceSubsystem during Level 3+ heat
 * - Positioned ahead of player based on road prediction
 * - Player can breach at 50+ MPH or find alternate route
 */
UCLASS(BlueprintType, Blueprintable)
class MIDNIGHTGRIND_API AMGPoliceRoadblock : public AActor
{
	GENERATED_BODY()

public:
	AMGPoliceRoadblock();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * @brief Initialize roadblock with ID and facing direction.
	 * @param InBlockadeID Unique identifier assigned by PoliceSubsystem
	 * @param Direction Direction the roadblock should face (perpendicular)
	 */
	UFUNCTION(BlueprintCallable, Category = "Roadblock")
	void Initialize(int32 InBlockadeID, FVector Direction);

	/**
	 * @brief Set the number of police vehicles in the blockade.
	 * @param Count Number of vehicles (clamped to 1-MaxVehicles)
	 */
	UFUNCTION(BlueprintCallable, Category = "Roadblock")
	void SetNumVehicles(int32 Count);

	/**
	 * @brief Deploy the roadblock (activate collision and visuals).
	 */
	UFUNCTION(BlueprintCallable, Category = "Roadblock")
	void Deploy();

	/**
	 * @brief Check if the roadblock is fully deployed.
	 * @return True if deployed and active
	 */
	UFUNCTION(BlueprintPure, Category = "Roadblock")
	bool IsDeployed() const { return bIsDeployed; }

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/**
	 * @brief Get the unique blockade identifier.
	 * @return Blockade ID assigned by PoliceSubsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Roadblock")
	int32 GetBlockadeID() const { return BlockadeID; }

	/**
	 * @brief Check if the roadblock has been breached.
	 * @return True if breached/destroyed
	 */
	UFUNCTION(BlueprintPure, Category = "Roadblock")
	bool IsBreached() const { return bIsBreached; }

	/**
	 * @brief Get current roadblock health.
	 * @return Health value (0 = breached)
	 */
	UFUNCTION(BlueprintPure, Category = "Roadblock")
	float GetHealth() const { return CurrentHealth; }

	/**
	 * @brief Get health as percentage (0-1).
	 * @return Normalized health value
	 */
	UFUNCTION(BlueprintPure, Category = "Roadblock")
	float GetHealthPercent() const { return CurrentHealth / MaxHealth; }

	/**
	 * @brief Get the number of vehicles in this roadblock.
	 * @return Vehicle count
	 */
	UFUNCTION(BlueprintPure, Category = "Roadblock")
	int32 GetNumVehicles() const { return NumVehicles; }

	/**
	 * @brief Get the effective width of the roadblock.
	 * @return Width in centimeters
	 */
	UFUNCTION(BlueprintPure, Category = "Roadblock")
	float GetEffectiveWidth() const { return NumVehicles * VehicleWidth; }

	// ==========================================
	// DAMAGE
	// ==========================================

	/**
	 * @brief Apply damage to the roadblock.
	 * @param DamageAmount Damage points to apply
	 * @param DamageCauser Actor that caused the damage
	 */
	UFUNCTION(BlueprintCallable, Category = "Roadblock")
	void ApplyDamage(float DamageAmount, AActor* DamageCauser);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when a vehicle hits the roadblock */
	UPROPERTY(BlueprintAssignable, Category = "Roadblock|Events")
	FOnRoadblockHit OnRoadblockHit;

	/** Fired when the roadblock is breached/destroyed */
	UPROPERTY(BlueprintAssignable, Category = "Roadblock|Events")
	FOnRoadblockDestroyed OnRoadblockDestroyed;

	/** Fired when roadblock is deployed and active */
	UPROPERTY(BlueprintAssignable, Category = "Roadblock|Events")
	FOnRoadblockDeployed OnRoadblockDeployed;

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Main collision volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;

	/** Barricade mesh (cones, barriers, etc.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BarricadeMesh;

	/** Police vehicle meshes in the blockade */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<TObjectPtr<UStaticMeshComponent>> PoliceVehicleMeshes;

	/** Police radio/siren audio */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> RadioAudio;

	/** Police light bar effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> LightBarVFX;

	// ==========================================
	// CONFIGURATION PROPERTIES
	// ==========================================

	/** Maximum health before breach */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float MaxHealth = 200.0f;

	/** Width per police vehicle in cm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float VehicleWidth = 400.0f;

	/** Maximum vehicles in a single roadblock */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	int32 MaxVehicles = 5;

	/** Speed required to breach the roadblock (MPH) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float BreachSpeedThreshold = 50.0f;

	/** Damage multiplier for high-speed impacts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float HighSpeedDamageMultiplier = 2.0f;

	/** Time to deploy the roadblock (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float DeployTime = 1.5f;

	/** Damage dealt to vehicles that hit the roadblock */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float VehicleDamageOnImpact = 15.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Unique blockade identifier */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 BlockadeID = -1;

	/** Current health value */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentHealth = 200.0f;

	/** Has the roadblock been breached */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsBreached = false;

	/** Is the roadblock deployed and active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDeployed = false;

	/** Is the roadblock currently deploying */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDeploying = false;

	/** Number of vehicles in the blockade */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 NumVehicles = 2;

	/** Deploy animation progress */
	float DeployProgress = 0.0f;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/**
	 * @brief Handle collision with vehicles.
	 */
	UFUNCTION()
	void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/**
	 * @brief Trigger the breach sequence.
	 */
	void BreachRoadblock();

	/**
	 * @brief Update deploy animation.
	 * @param DeltaTime Time since last update
	 */
	void UpdateDeployAnimation(float DeltaTime);

	/**
	 * @brief Setup component hierarchy.
	 */
	void SetupComponents();
};

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGSpikeStrip.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UAudioComponent;

/**
 * @brief Broadcast when a vehicle drives over the spike strip.
 * @param Vehicle The vehicle actor that hit the spikes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleHitSpikes, AActor*, Vehicle);

/**
 * @brief Broadcast when spike strip is deployed and active.
 * @param StripID Unique identifier of the spike strip
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpikeStripDeployed, int32, StripID);

/**
 * @brief Broadcast when spike strip is destroyed or expired.
 * @param StripID Unique identifier of the spike strip
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpikeStripDestroyed, int32, StripID);

/**
 * @class AMGSpikeStrip
 * @brief Police spike strip trap actor for pursuit mechanics.
 *
 * Spike strips are tactical tools deployed by police to puncture
 * vehicle tires and reduce speed. They create a risk/reward scenario
 * where players must decide whether to attempt jumping/avoiding
 * the strip or taking the tire damage.
 *
 * Key Features:
 * - Configurable length to cover different road widths
 * - Deploy/retract functionality for timing-based placement
 * - Tire damage system (reduces grip and max speed)
 * - Speed reduction effect on hit
 * - Optional single-use vs reusable configuration
 * - Visual feedback (spikes visible when deployed)
 * - Audio feedback (metal scraping sound on hit)
 *
 * Usage:
 * - Spawned by MGPoliceSubsystem during Level 4+ heat
 * - Placed ahead of player on predicted path
 * - Can be avoided by quick reflexes or alternate routes
 * - Hitting spikes damages tires but doesn't stop vehicle
 */
UCLASS(BlueprintType, Blueprintable)
class MIDNIGHTGRIND_API AMGSpikeStrip : public AActor
{
	GENERATED_BODY()

public:
	AMGSpikeStrip();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * @brief Initialize spike strip with ID and direction.
	 * @param InStripID Unique identifier assigned by PoliceSubsystem
	 * @param Direction Direction the strip should face (perpendicular)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpikeStrip")
	void Initialize(int32 InStripID, FVector Direction);

	/**
	 * @brief Set the length of the spike strip.
	 * @param Length Desired length in centimeters (200-1500)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpikeStrip")
	void SetLength(float Length);

	/**
	 * @brief Deploy the spike strip (activate collision).
	 */
	UFUNCTION(BlueprintCallable, Category = "SpikeStrip")
	void Deploy();

	/**
	 * @brief Retract the spike strip (deactivate collision).
	 */
	UFUNCTION(BlueprintCallable, Category = "SpikeStrip")
	void Retract();

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/**
	 * @brief Get the unique strip identifier.
	 * @return Strip ID assigned by PoliceSubsystem
	 */
	UFUNCTION(BlueprintPure, Category = "SpikeStrip")
	int32 GetStripID() const { return StripID; }

	/**
	 * @brief Check if the spike strip is deployed.
	 * @return True if deployed and active
	 */
	UFUNCTION(BlueprintPure, Category = "SpikeStrip")
	bool IsDeployed() const { return bIsDeployed; }

	/**
	 * @brief Check if the strip has been triggered (single-use).
	 * @return True if already triggered
	 */
	UFUNCTION(BlueprintPure, Category = "SpikeStrip")
	bool IsTriggered() const { return bHasBeenTriggered; }

	/**
	 * @brief Get current strip length.
	 * @return Length in centimeters
	 */
	UFUNCTION(BlueprintPure, Category = "SpikeStrip")
	float GetLength() const { return CurrentLength; }

	/**
	 * @brief Get the number of vehicles that have hit this strip.
	 * @return Hit count
	 */
	UFUNCTION(BlueprintPure, Category = "SpikeStrip")
	int32 GetHitCount() const { return HitCount; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when a vehicle drives over the spikes */
	UPROPERTY(BlueprintAssignable, Category = "SpikeStrip|Events")
	FOnVehicleHitSpikes OnVehicleHitSpikes;

	/** Fired when spike strip is deployed */
	UPROPERTY(BlueprintAssignable, Category = "SpikeStrip|Events")
	FOnSpikeStripDeployed OnSpikeStripDeployed;

	/** Fired when spike strip is destroyed/removed */
	UPROPERTY(BlueprintAssignable, Category = "SpikeStrip|Events")
	FOnSpikeStripDestroyed OnSpikeStripDestroyed;

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Trigger collision volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Spike mesh visual */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SpikeMesh;

	/** Audio for spike deployment/hit */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> HitAudio;

	// ==========================================
	// CONFIGURATION PROPERTIES
	// ==========================================

	/** Default length of the spike strip (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float DefaultLength = 600.0f;

	/** Minimum length allowed (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float MinLength = 200.0f;

	/** Maximum length allowed (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float MaxLength = 1500.0f;

	/** Base damage to apply to each tire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float TireDamageAmount = 50.0f;

	/** Percentage speed reduction when hit (0-100) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float SpeedReductionPercent = 30.0f;

	/** Whether the strip is destroyed after first hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	bool bSingleUse = true;

	/** Deploy animation time (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float DeployTime = 0.5f;

	/** Grip reduction applied to affected tires (0-1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float GripReductionMultiplier = 0.4f;

	/** Whether to apply progressive damage (more damage at higher speed) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	bool bProgressiveDamage = true;

	// ==========================================
	// STATE
	// ==========================================

	/** Unique strip identifier */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 StripID = -1;

	/** Whether the strip is deployed and active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDeployed = false;

	/** Whether the strip is currently deploying */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDeploying = false;

	/** Whether the strip has been triggered (single-use mode) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bHasBeenTriggered = false;

	/** Current strip length */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentLength = 600.0f;

	/** Number of vehicles that have hit this strip */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 HitCount = 0;

	/** Deploy animation progress (0-1) */
	float DeployProgress = 0.0f;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/**
	 * @brief Handle overlap begin event.
	 */
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * @brief Apply tire damage to a vehicle.
	 * @param Vehicle The vehicle to damage
	 * @param VehicleSpeed Speed of vehicle at time of hit
	 */
	void ApplyTireDamage(AActor* Vehicle, float VehicleSpeed);

	/**
	 * @brief Update deploy animation.
	 * @param DeltaTime Time since last update
	 */
	void UpdateDeployAnimation(float DeltaTime);

	/**
	 * @brief Setup component hierarchy.
	 */
	void SetupComponents();

	/**
	 * @brief Destroy the spike strip.
	 */
	void DestroyStrip();
};

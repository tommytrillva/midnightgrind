// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "MGPoliceUnit.generated.h"

class UMGVehicleMovementComponent;
class UAudioComponent;
class UNiagaraComponent;

/**
 * @enum EMGPoliceState
 * @brief State machine states for police unit AI behavior.
 */
UENUM(BlueprintType)
enum class EMGPoliceState : uint8
{
	/** Unit is inactive/waiting */
	Idle,

	/** Driving patrol route */
	Patrolling,

	/** Noticed suspicious activity */
	Alerted,

	/** Actively chasing target */
	Pursuing,

	/** Attempting to ram target */
	Ramming,

	/** Coordinating to surround target */
	BoxingIn,

	/** Executing PIT maneuver */
	PITManeuver,

	/** Moving to intercept target */
	Intercepting,

	/** Unit has been disabled */
	Disabled,

	/** Unit is being removed from world */
	Despawning
};

/** Broadcast when police unit is disabled */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoliceUnitDisabledUnit, int32, UnitID);

/** Broadcast when police unit state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitStateChanged, int32, UnitID, EMGPoliceState, NewState);

/** Broadcast when unit successfully rams target */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitRammedTarget, int32, UnitID, float, ImpactForce);

/** Broadcast when unit executes PIT maneuver */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitPITAttempt, int32, UnitID, bool, bSuccess);

/**
 * @class AMGPoliceUnit
 * @brief Police vehicle AI unit for pursuit mechanics.
 *
 * Handles pursuit behaviors including chasing, ramming, boxing in,
 * PIT maneuvers, and roadblock coordination. Each unit maintains
 * its own state machine and responds to commands from the PoliceSubsystem.
 *
 * Key Features:
 * - State-driven AI with smooth transitions
 * - Visual tracking with line-of-sight checks
 * - Multiple pursuit tactics (ram, PIT, box)
 * - Damage system leading to disabled state
 * - Siren and lightbar visual/audio feedback
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGPoliceUnit : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	AMGPoliceUnit(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	// ==========================================
	// UNIT CONFIGURATION
	// ==========================================

	/**
	 * @brief Initialize police unit with ID and initial state.
	 * @param InUnitID Unique identifier assigned by PoliceSubsystem
	 * @param InitialState Starting AI state
	 */
	UFUNCTION(BlueprintCallable, Category = "Police")
	void InitializeUnit(int32 InUnitID, EMGPoliceState InitialState);

	/**
	 * @brief Set the target pawn to pursue.
	 * @param Target Target pawn (usually player vehicle)
	 */
	UFUNCTION(BlueprintCallable, Category = "Police")
	void SetPursuitTarget(APawn* Target);

	/**
	 * @brief Get this unit's unique ID.
	 * @return Unit identifier
	 */
	UFUNCTION(BlueprintPure, Category = "Police")
	int32 GetUnitID() const { return UnitID; }

	/**
	 * @brief Get current AI state.
	 * @return Current EMGPoliceState
	 */
	UFUNCTION(BlueprintPure, Category = "Police")
	EMGPoliceState GetCurrentState() const { return CurrentState; }

	/**
	 * @brief Force state transition.
	 * @param NewState Target AI state
	 */
	UFUNCTION(BlueprintCallable, Category = "Police")
	void SetPoliceState(EMGPoliceState NewState);

	// ==========================================
	// PURSUIT BEHAVIOR
	// ==========================================

	/**
	 * @brief Begin pursuit of target.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void StartPursuit();

	/**
	 * @brief Stop pursuing and return to patrol.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void StopPursuit();

	/**
	 * @brief Check if unit has visual on target.
	 * @return True if target is visible
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	bool HasVisualOnTarget() const { return bHasVisual; }

	/**
	 * @brief Get distance to pursuit target in cm.
	 * @return Distance to target
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	float GetDistanceToTarget() const;

	/**
	 * @brief Get last known target position.
	 * @return Last confirmed target location
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	FVector GetLastKnownTargetPosition() const { return LastKnownTargetPosition; }

	/**
	 * @brief Get time since target was last seen.
	 * @return Seconds since visual contact
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	float GetTimeSinceSawTarget() const { return TimeSinceSawTarget; }

	/**
	 * @brief Command unit to execute ram attack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void ExecuteRam();

	/**
	 * @brief Command unit to execute PIT maneuver.
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void ExecutePIT();

	/**
	 * @brief Command unit to participate in boxing maneuver.
	 * @param BoxPosition Assigned position in formation
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void ExecuteBoxing(FVector BoxPosition);

	// ==========================================
	// DAMAGE SYSTEM
	// ==========================================

	/**
	 * @brief Apply damage to unit.
	 * @param DamageAmount Damage points to apply
	 * @param DamageCauser Actor that caused damage
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Damage")
	void ApplyDamage(float DamageAmount, AActor* DamageCauser);

	/**
	 * @brief Get current health.
	 * @return Health value (0-MaxHealth)
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Damage")
	float GetHealth() const { return Health; }

	/**
	 * @brief Get health as percentage (0-1).
	 * @return Normalized health value
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Damage")
	float GetHealthPercent() const { return Health / MaxHealth; }

	/**
	 * @brief Check if unit is disabled.
	 * @return True if disabled
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Damage")
	bool IsDisabled() const { return CurrentState == EMGPoliceState::Disabled; }

	// ==========================================
	// SIREN AND LIGHTS
	// ==========================================

	/**
	 * @brief Enable or disable siren and lights.
	 * @param bEnabled Whether siren should be active
	 */
	UFUNCTION(BlueprintCallable, Category = "Police|Siren")
	void SetSirenEnabled(bool bEnabled);

	/**
	 * @brief Check if siren is active.
	 * @return True if siren enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Police|Siren")
	bool IsSirenEnabled() const { return bSirenActive; }

	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Siren audio component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> SirenAudio;

	/** Engine audio component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> EngineAudio;

	/** Light bar visual effect (Niagara) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> LightBarVFX;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Speed multiplier during pursuit (1.0 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float PursuitSpeedMultiplier = 1.1f;

	/** Minimum time between ram attempts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float RamCooldown = 3.0f;

	/** Distance at which ramming is attempted (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float RamDistance = 500.0f;

	/** Time before lost visual counts as lost target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float VisualLostTime = 5.0f;

	/** Maximum unit health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float MaxHealth = 100.0f;

	/** Aggression level affecting behavior (1.0 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float AggressionLevel = 1.0f;

	/** Distance at which PIT maneuver is attempted (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float PITDistance = 800.0f;

	/** Minimum time between PIT attempts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float PITCooldown = 5.0f;

	/** Angle threshold for valid PIT approach (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float PITAngleThreshold = 45.0f;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when unit is disabled */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceUnitDisabledUnit OnUnitDisabled;

	/** Fired when state changes */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceUnitStateChanged OnStateChanged;

	/** Fired when unit rams target */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceUnitRammedTarget OnRammedTarget;

	/** Fired when PIT maneuver is attempted */
	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceUnitPITAttempt OnPITAttempt;

protected:
	// ==========================================
	// AI BEHAVIOR UPDATES
	// ==========================================

	/**
	 * @brief Update patrol behavior (idle driving).
	 * @param DeltaTime Time since last update
	 */
	virtual void UpdatePatrol(float DeltaTime);

	/**
	 * @brief Update pursuit behavior (chasing target).
	 * @param DeltaTime Time since last update
	 */
	virtual void UpdatePursuit(float DeltaTime);

	/**
	 * @brief Update ramming behavior.
	 * @param DeltaTime Time since last update
	 */
	virtual void UpdateRamming(float DeltaTime);

	/**
	 * @brief Update boxing behavior (surrounding target).
	 * @param DeltaTime Time since last update
	 */
	virtual void UpdateBoxing(float DeltaTime);

	/**
	 * @brief Update PIT maneuver behavior.
	 * @param DeltaTime Time since last update
	 */
	virtual void UpdatePIT(float DeltaTime);

	/**
	 * @brief Update intercept behavior.
	 * @param DeltaTime Time since last update
	 */
	virtual void UpdateIntercept(float DeltaTime);

	/**
	 * @brief Update disabled behavior (coast to stop).
	 * @param DeltaTime Time since last update
	 */
	virtual void UpdateDisabled(float DeltaTime);

	/**
	 * @brief Calculate steering input toward target.
	 * @return Steering value (-1 to 1)
	 */
	float CalculateSteeringToTarget() const;

	/**
	 * @brief Calculate steering for PIT approach.
	 * @return Steering value (-1 to 1)
	 */
	float CalculatePITSteering() const;

	/**
	 * @brief Calculate throttle based on distance.
	 * @return Throttle value (0 to 1)
	 */
	float CalculateThrottleToTarget() const;

	/**
	 * @brief Update visual tracking of target.
	 */
	void UpdateVisualOnTarget();

	/**
	 * @brief Check if PIT angle is valid.
	 * @return True if approach angle is good for PIT
	 */
	bool IsValidPITAngle() const;

	/**
	 * @brief Handle state enter logic.
	 * @param State New state being entered
	 */
	void OnStateEnter(EMGPoliceState State);

	/**
	 * @brief Handle state exit logic.
	 * @param State State being exited
	 */
	void OnStateExit(EMGPoliceState State);

	/**
	 * @brief Called when collision occurs.
	 * @param HitComponent Component that was hit
	 * @param OtherActor Actor that was hit
	 * @param OtherComp Other component involved
	 * @param NormalImpulse Impact impulse
	 * @param Hit Hit result details
	 */
	UFUNCTION()
	void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	/** Unique unit identifier */
	UPROPERTY()
	int32 UnitID = -1;

	/** Current AI state */
	UPROPERTY()
	EMGPoliceState CurrentState = EMGPoliceState::Idle;

	/** Reference to pursuit target */
	UPROPERTY()
	TWeakObjectPtr<APawn> PursuitTarget;

	/** Last confirmed target position */
	UPROPERTY()
	FVector LastKnownTargetPosition = FVector::ZeroVector;

	/** Assigned position for boxing maneuver */
	UPROPERTY()
	FVector BoxingTargetPosition = FVector::ZeroVector;

	/** Current unit health */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float Health = 100.0f;

	/** Time since target was last seen */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float TimeSinceSawTarget = 0.0f;

	/** Time since last ram attempt */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float TimeSinceLastRam = 0.0f;

	/** Time since last PIT attempt */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float TimeSinceLastPIT = 0.0f;

	/** Whether unit has visual on target */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bHasVisual = false;

	/** Whether siren is active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bSirenActive = false;

	/** Whether currently executing PIT */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bExecutingPIT = false;

	/** Side to approach for PIT (-1 = left, 1 = right) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float PITSide = 1.0f;

	/**
	 * @brief Setup component hierarchy.
	 */
	void SetupComponents();

	/**
	 * @brief Bind collision events.
	 */
	void BindCollisionEvents();
};

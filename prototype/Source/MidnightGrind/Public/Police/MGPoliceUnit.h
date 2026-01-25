// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "MGPoliceUnit.generated.h"

class UMGVehicleMovementComponent;
class UAudioComponent;
class UNiagaraComponent;

UENUM(BlueprintType)
enum class EMGPoliceState : uint8
{
	Idle,
	Patrolling,
	Alerted,
	Pursuing,
	Ramming,
	BoxingIn,
	Disabled,
	Despawning
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoliceUnitDisabled, int32, UnitID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitStateChanged, int32, UnitID, EMGPoliceState, NewState);

/**
 * Police vehicle AI unit for pursuit mechanics
 * Handles chasing, ramming, boxing in, and roadblock behavior
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGPoliceUnit : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	AMGPoliceUnit(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ==========================================
	// UNIT CONFIGURATION
	// ==========================================

	/** Initialize police unit with ID and type */
	UFUNCTION(BlueprintCallable, Category = "Police")
	void InitializeUnit(int32 InUnitID, EMGPoliceState InitialState);

	/** Set the target to pursue */
	UFUNCTION(BlueprintCallable, Category = "Police")
	void SetPursuitTarget(APawn* Target);

	/** Get unit ID */
	UFUNCTION(BlueprintPure, Category = "Police")
	int32 GetUnitID() const { return UnitID; }

	/** Get current state */
	UFUNCTION(BlueprintPure, Category = "Police")
	EMGPoliceState GetCurrentState() const { return CurrentState; }

	/** Set police state */
	UFUNCTION(BlueprintCallable, Category = "Police")
	void SetPoliceState(EMGPoliceState NewState);

	// ==========================================
	// PURSUIT BEHAVIOR
	// ==========================================

	/** Start pursuit of target */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void StartPursuit();

	/** Stop pursuit */
	UFUNCTION(BlueprintCallable, Category = "Police|Pursuit")
	void StopPursuit();

	/** Check if unit has visual on player */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	bool HasVisualOnTarget() const { return bHasVisual; }

	/** Get distance to target */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	float GetDistanceToTarget() const;

	/** Get last known target position */
	UFUNCTION(BlueprintPure, Category = "Police|Pursuit")
	FVector GetLastKnownTargetPosition() const { return LastKnownTargetPosition; }

	// ==========================================
	// DAMAGE SYSTEM
	// ==========================================

	/** Apply damage to unit */
	UFUNCTION(BlueprintCallable, Category = "Police|Damage")
	void ApplyDamage(float DamageAmount, AActor* DamageCauser);

	/** Get current health */
	UFUNCTION(BlueprintPure, Category = "Police|Damage")
	float GetHealth() const { return Health; }

	/** Check if unit is disabled */
	UFUNCTION(BlueprintPure, Category = "Police|Damage")
	bool IsDisabled() const { return CurrentState == EMGPoliceState::Disabled; }

	// ==========================================
	// SIREN
	// ==========================================

	/** Enable/disable siren */
	UFUNCTION(BlueprintCallable, Category = "Police|Siren")
	void SetSirenEnabled(bool bEnabled);

	/** Check if siren is active */
	UFUNCTION(BlueprintPure, Category = "Police|Siren")
	bool IsSirenEnabled() const { return bSirenActive; }

	// ==========================================
	// COMPONENTS
	// ==========================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> SirenAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> EngineAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> LightBarVFX;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Max pursuit speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float PursuitSpeedMultiplier = 1.1f;

	/** Ram cooldown time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float RamCooldown = 3.0f;

	/** Distance to switch to ramming behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float RamDistance = 500.0f;

	/** Time before losing visual counts as lost target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pursuit")
	float VisualLostTime = 5.0f;

	/** Max health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float MaxHealth = 100.0f;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceUnitDisabled OnUnitDisabled;

	UPROPERTY(BlueprintAssignable, Category = "Police|Events")
	FOnPoliceUnitStateChanged OnStateChanged;

protected:
	// ==========================================
	// AI BEHAVIOR
	// ==========================================

	virtual void UpdatePatrol(float DeltaTime);
	virtual void UpdatePursuit(float DeltaTime);
	virtual void UpdateRamming(float DeltaTime);
	virtual void UpdateBoxing(float DeltaTime);
	virtual void UpdateDisabled(float DeltaTime);

	/** Calculate steering toward target */
	float CalculateSteeringToTarget() const;

	/** Calculate throttle based on distance */
	float CalculateThrottleToTarget() const;

	/** Check visual line of sight to target */
	void UpdateVisualOnTarget();

	/** Handle state transitions */
	void OnStateEnter(EMGPoliceState State);
	void OnStateExit(EMGPoliceState State);

private:
	UPROPERTY()
	int32 UnitID = -1;

	UPROPERTY()
	EMGPoliceState CurrentState = EMGPoliceState::Idle;

	UPROPERTY()
	TWeakObjectPtr<APawn> PursuitTarget;

	UPROPERTY()
	FVector LastKnownTargetPosition = FVector::ZeroVector;

	float Health = 100.0f;
	float TimeSinceSawTarget = 0.0f;
	float TimeSinceLastRam = 0.0f;
	bool bHasVisual = false;
	bool bSirenActive = false;

	void SetupComponents();
};

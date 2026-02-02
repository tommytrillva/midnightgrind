// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MG_VHCL_ArcadeEnhancements.h
 * @brief Need for Speed Underground style arcade enhancements for vehicle movement
 *
 * This component adds NFSU-style assists and arcade behaviors on top of the
 * existing simulation physics, without replacing the core systems.
 *
 * Key Features:
 * - Drift assist (auto-maintains optimal drift angle)
 * - Collision recovery (bounce off walls instead of stopping)
 * - Oversteer correction (prevents spinouts)
 * - Speed maintenance during drifts
 * - Simulation system disabling (no wear/temperature penalties)
 *
 * Usage:
 * Enable bNFSUMode on the vehicle movement component to activate these assists.
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MG_VHCL_ArcadeEnhancements.generated.h"

class UMGVehicleMovementComponent;

/**
 * @brief Arcade assist types for selective enabling/disabling
 */
UENUM(BlueprintType)
enum class EMGArcadeAssistType : uint8
{
	None            UMETA(DisplayName = "None"),
	DriftAssist     UMETA(DisplayName = "Drift Assist"),
	CollisionBounce UMETA(DisplayName = "Collision Bounce"),
	OversteerCorrect UMETA(DisplayName = "Oversteer Correction"),
	SpeedMaintain   UMETA(DisplayName = "Speed Maintenance"),
	SimDisable      UMETA(DisplayName = "Disable Simulation Systems")
};

/**
 * @brief Configuration for arcade assists
 */
USTRUCT(BlueprintType)
struct FMGArcadeAssistConfig
{
	GENERATED_BODY()

	/** Enable drift angle auto-correction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	bool bDriftAssist = true;

	/** Drift assist strength (0-1, higher = more assist) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DriftAssistStrength = 0.7f;

	/** Ideal drift angle to maintain (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float IdealDriftAngle = 30.0f;

	/** Tolerance around ideal angle before assist kicks in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float DriftAngleTolerance = 15.0f;

	/** Minimum speed for drift assist to engage (MPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float MinDriftSpeedMPH = 30.0f;

	/** Enable collision bounce recovery */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	bool bCollisionBounce = true;

	/** Collision bounce force multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float BounceMult = 1.5f;

	/** Recovery duration after collision (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float RecoveryDuration = 0.5f;

	/** Enable automatic oversteer correction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	bool bOversteerCorrect = true;

	/** Oversteer correction strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OversteerCorrectStrength = 0.6f;

	/** Slip angle threshold to trigger oversteer correction (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float OversteerThreshold = 15.0f;

	/** Enable speed maintenance during drifts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	bool bMaintainSpeedInDrift = true;

	/** Minimum speed to maintain during drift (MPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	float MinDriftMaintenanceSpeed = 40.0f;

	/** Throttle boost to apply when below min speed (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float SpeedBoostAmount = 0.3f;

	/** Disable simulation systems (tire temp, wear, clutch, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	bool bDisableSimSystems = true;
};

/**
 * @brief Actor component that adds arcade assists to vehicle movement
 *
 * Attach this to a vehicle pawn alongside the movement component.
 * It reads from and modifies the movement component's state to provide
 * NFSU-style arcade handling without replacing the physics simulation.
 */
UCLASS(ClassGroup = (MidnightGrind), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGVehicleArcadeEnhancements : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGVehicleArcadeEnhancements();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Enable all arcade assists */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	bool bEnableArcadeMode = true;

	/** Arcade assist configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade")
	FMGArcadeAssistConfig Config;

	/** Show debug visualizations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arcade|Debug")
	bool bShowDebugInfo = false;

	// ==========================================
	// ASSIST FUNCTIONS
	// ==========================================

	/** Apply drift angle correction assist */
	UFUNCTION(BlueprintCallable, Category = "Arcade|Assist")
	void ApplyDriftAssist(float DeltaTime);

	/** Apply collision bounce recovery */
	UFUNCTION(BlueprintCallable, Category = "Arcade|Assist")
	void ApplyCollisionRecovery(float DeltaTime);

	/** Apply oversteer correction */
	UFUNCTION(BlueprintCallable, Category = "Arcade|Assist")
	void ApplyOversteerCorrection(float DeltaTime);

	/** Maintain speed during drifts */
	UFUNCTION(BlueprintCallable, Category = "Arcade|Assist")
	void MaintainDriftSpeed(float DeltaTime);

	/** Disable simulation systems for arcade feel */
	UFUNCTION(BlueprintCallable, Category = "Arcade|Assist")
	void DisableSimulationSystems();

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/** Check if drift assist is currently active */
	UFUNCTION(BlueprintPure, Category = "Arcade|State")
	bool IsDriftAssistActive() const { return bDriftAssistActive; }

	/** Check if collision recovery is active */
	UFUNCTION(BlueprintPure, Category = "Arcade|State")
	bool IsCollisionRecoveryActive() const { return bCollisionRecoveryActive; }

	/** Get current drift assist correction amount */
	UFUNCTION(BlueprintPure, Category = "Arcade|State")
	float GetDriftAssistCorrection() const { return CurrentDriftCorrection; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Broadcast when drift assist engages */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriftAssistEngaged, float, CorrectionAmount);
	UPROPERTY(BlueprintAssignable, Category = "Arcade|Events")
	FOnDriftAssistEngaged OnDriftAssistEngaged;

	/** Broadcast when collision bounce triggers */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionBounce, FVector, BounceDirection, float, BounceForce);
	UPROPERTY(BlueprintAssignable, Category = "Arcade|Events")
	FOnCollisionBounce OnCollisionBounce;

protected:
	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/** Cached reference to vehicle movement component */
	UPROPERTY()
	TObjectPtr<UMGVehicleMovementComponent> MovementComponent;

	/** Is drift assist currently applying corrections */
	UPROPERTY()
	bool bDriftAssistActive = false;

	/** Is collision recovery currently active */
	UPROPERTY()
	bool bCollisionRecoveryActive = false;

	/** Current drift correction amount (-1 to 1) */
	UPROPERTY()
	float CurrentDriftCorrection = 0.0f;

	/** Time since last collision */
	UPROPERTY()
	float TimeSinceCollision = 999.0f;

	/** Last collision normal vector */
	UPROPERTY()
	FVector LastCollisionNormal = FVector::ZeroVector;

	/** Last collision time stamp */
	UPROPERTY()
	float LastCollisionTime = 0.0f;

	// ==========================================
	// INTERNAL HELPERS
	// ==========================================

	/** Get current drift angle from movement component */
	float GetCurrentDriftAngle() const;

	/** Get current vehicle speed in MPH */
	float GetCurrentSpeedMPH() const;

	/** Check if handbrake is engaged */
	bool IsHandbrakeEngaged() const;

	/** Get steering input value */
	float GetSteeringInput() const;

	/** Apply steering correction to movement component */
	void ApplySteeringCorrection(float CorrectionAmount);

	/** Apply throttle boost to movement component */
	void ApplyThrottleBoost(float BoostAmount);

	/** Apply impulse force for collision bounce */
	void ApplyBounceImpulse(const FVector& Direction, float Force);

	/** Register collision event handler */
	void OnVehicleCollision(AActor* OtherActor, FVector HitLocation, FVector HitNormal);

	/** Draw debug info on screen */
	void DrawDebugInfo();
};

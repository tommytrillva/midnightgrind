// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "MGVehicleData.h"
#include "MGVehicleMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGearChanged, int32, NewGear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNitrousStateChanged, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBoostChanged, float, CurrentBoost, float, MaxBoost);

/**
 * Drift state information
 */
USTRUCT(BlueprintType)
struct FMGDriftState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsDrifting = false;

	UPROPERTY(BlueprintReadOnly)
	float DriftAngle = 0.0f; // Current angle in degrees

	UPROPERTY(BlueprintReadOnly)
	float DriftDuration = 0.0f; // How long current drift has lasted

	UPROPERTY(BlueprintReadOnly)
	float DriftScore = 0.0f; // Accumulated score for current drift
};

/**
 * Engine state information
 */
USTRUCT(BlueprintType)
struct FMGEngineState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float CurrentRPM = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float ThrottlePosition = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float EngineLoad = 0.0f; // 0-1

	UPROPERTY(BlueprintReadOnly)
	float CurrentBoostPSI = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float BoostBuildupPercent = 0.0f; // 0-1, how close to full boost

	UPROPERTY(BlueprintReadOnly)
	bool bNitrousActive = false;

	UPROPERTY(BlueprintReadOnly)
	float NitrousRemaining = 100.0f; // Percent

	UPROPERTY(BlueprintReadOnly)
	float EngineTemperature = 90.0f; // Celsius

	UPROPERTY(BlueprintReadOnly)
	bool bOverheating = false;

	UPROPERTY(BlueprintReadOnly)
	bool bRevLimiterActive = false;
};

/**
 * Custom vehicle movement component for MIDNIGHT GRIND
 * Extends Chaos vehicle physics with arcade-tuned handling
 */
UCLASS(ClassGroup = (MidnightGrind), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGVehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
{
	GENERATED_BODY()

public:
	UMGVehicleMovementComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	// ==========================================
	// VEHICLE DATA
	// ==========================================

	/** Apply vehicle configuration data to this component */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Configuration")
	void ApplyVehicleConfiguration(const FMGVehicleData& VehicleData);

	/** Get current vehicle configuration */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Configuration")
	const FMGVehicleData& GetVehicleConfiguration() const { return CurrentConfiguration; }

	// ==========================================
	// INPUT
	// ==========================================

	/** Set throttle input (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetThrottleInput(float Value);

	/** Set brake input (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetBrakeInput(float Value);

	/** Set steering input (-1 to 1) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetSteeringInput(float Value);

	/** Set handbrake state */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetHandbrakeInput(bool bEngaged);

	/** Activate nitrous */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void ActivateNitrous();

	/** Deactivate nitrous */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void DeactivateNitrous();

	/** Request upshift */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void ShiftUp();

	/** Request downshift */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void ShiftDown();

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/** Get current engine state */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FMGEngineState GetEngineState() const { return EngineState; }

	/** Get current drift state */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FMGDriftState GetDriftState() const { return DriftState; }

	/** Get current gear (0 = neutral, -1 = reverse, 1+ = forward gears) */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	int32 GetCurrentGear() const { return CurrentGear; }

	/** Get current speed in MPH */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	float GetSpeedMPH() const;

	/** Get current speed in KPH */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	float GetSpeedKPH() const;

	/** Check if vehicle is grounded (all wheels on surface) */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	bool IsGrounded() const;

	/** Check if any wheel is slipping */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	bool IsWheelSlipping(int32 WheelIndex) const;

	/** Get slip angle for a wheel */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	float GetWheelSlipAngle(int32 WheelIndex) const;

	/** Get slip ratio for a wheel */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	float GetWheelSlipRatio(int32 WheelIndex) const;

	// ==========================================
	// TUNING PARAMETERS
	// ==========================================

	/** Drift physics settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Drift")
	float DriftAngleThreshold = 15.0f; // Degrees to start counting as drift

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Drift")
	float DriftFrictionMultiplier = 0.7f; // Friction reduction while drifting

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Drift")
	float DriftCounterSteerAssist = 0.3f; // 0-1, how much we help counter-steer

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Drift")
	float DriftRecoveryRate = 2.0f; // How fast drift angle reduces

	/** Handbrake settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Handbrake")
	float HandbrakeFrictionMultiplier = 0.3f; // Rear tire friction when handbrake engaged

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Handbrake")
	float HandbrakeRearBias = 1.0f; // How much handbrake affects rear vs front

	/** Arcade tuning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Arcade")
	float ArcadeSteeringSpeed = 5.0f; // How fast steering responds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Arcade")
	float ArcadeSteeringReturnSpeed = 8.0f; // How fast steering centers

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Arcade")
	float SpeedSensitiveSteeringFactor = 0.5f; // Reduce steering at high speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Arcade")
	float StabilityControl = 0.3f; // 0-1, automatic correction strength

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Arcade")
	float AntiFlipTorque = 5000.0f; // Force to prevent flipping

	/** Tire model */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Tires")
	float BaseTireGrip = 1.0f; // Multiplier for all tire grip

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Tires")
	float TireWearRate = 1.0f; // How fast tires degrade

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Tires")
	float WetGripMultiplier = 0.7f; // Grip reduction in wet conditions

	/** Turbo simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo")
	float TurboLagSimulation = 1.0f; // 0 = no lag, 1 = realistic lag

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo")
	float BoostBuildupRate = 2.0f; // How fast boost builds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo")
	float BoostDecayRate = 4.0f; // How fast boost falls off throttle

	/** Nitrous settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Nitrous")
	float NitrousPowerMultiplier = 1.5f; // Power boost when active

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Nitrous")
	float NitrousConsumptionRate = 10.0f; // Percent per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Nitrous")
	float NitrousMinimumRPMPercent = 0.5f; // Min RPM to activate (% of redline)

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when gear changes */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnGearChanged OnGearChanged;

	/** Called when nitrous state changes */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnNitrousStateChanged OnNitrousStateChanged;

	/** Called when boost level changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnBoostChanged OnBoostChanged;

protected:
	// ==========================================
	// INTERNAL STATE
	// ==========================================

	UPROPERTY()
	FMGVehicleData CurrentConfiguration;

	UPROPERTY()
	FMGEngineState EngineState;

	UPROPERTY()
	FMGDriftState DriftState;

	UPROPERTY()
	int32 CurrentGear = 0;

	UPROPERTY()
	float TargetSteering = 0.0f;

	UPROPERTY()
	float CurrentSteering = 0.0f;

	UPROPERTY()
	bool bHandbrakeEngaged = false;

	UPROPERTY()
	float ShiftCooldown = 0.0f;

	UPROPERTY()
	float LastBoostBroadcast = 0.0f;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Update engine simulation */
	virtual void UpdateEngineSimulation(float DeltaTime);

	/** Update turbo/boost simulation */
	virtual void UpdateBoostSimulation(float DeltaTime);

	/** Update drift detection and physics */
	virtual void UpdateDriftPhysics(float DeltaTime);

	/** Update nitrous system */
	virtual void UpdateNitrousSystem(float DeltaTime);

	/** Apply stability control */
	virtual void ApplyStabilityControl(float DeltaTime);

	/** Apply anti-flip torque */
	virtual void ApplyAntiFlipForce(float DeltaTime);

	/** Calculate modified tire friction */
	virtual float CalculateTireFriction(int32 WheelIndex) const;

	/** Calculate current power output including all modifiers */
	virtual float CalculateCurrentPower() const;

	/** Get tire grip coefficient for compound */
	static float GetTireCompoundGrip(EMGTireCompound Compound);

	/** Perform gear shift */
	void PerformGearShift(int32 NewGear);
};

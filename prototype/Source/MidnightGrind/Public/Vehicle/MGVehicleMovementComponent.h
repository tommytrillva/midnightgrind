// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "MGVehicleData.h"
#include "MGVehicleMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGearChanged, int32, NewGear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNitrousStateChanged, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBoostChanged, float, CurrentBoost, float, MaxBoost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDriftScoreAwarded, float, Score, int32, ChainMultiplier, float, AngleBonus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriftChainBroken, float, TotalChainScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartWearWarning, FName, PartName, float, Condition);

/**
 * @brief Drift scoring tier thresholds for angle bonuses
 */
UENUM(BlueprintType)
enum class EMGDriftAngleTier : uint8
{
	None UMETA(DisplayName = "No Drift"),
	Mild UMETA(DisplayName = "Mild (15-30 deg)"),
	Standard UMETA(DisplayName = "Standard (30-45 deg)"),
	Aggressive UMETA(DisplayName = "Aggressive (45-60 deg)"),
	Extreme UMETA(DisplayName = "Extreme (60-75 deg)"),
	Insane UMETA(DisplayName = "Insane (75+ deg)")
};

/**
 * @brief Drift state information with enhanced scoring system
 */
USTRUCT(BlueprintType)
struct FMGDriftState
{
	GENERATED_BODY()

	/** Whether the vehicle is currently drifting */
	UPROPERTY(BlueprintReadOnly, Category = "Drift")
	bool bIsDrifting = false;

	/** Current angle in degrees (positive = right, negative = left) */
	UPROPERTY(BlueprintReadOnly, Category = "Drift")
	float DriftAngle = 0.0f;

	/** How long current drift has lasted in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Drift")
	float DriftDuration = 0.0f;

	/** Accumulated score for current drift */
	UPROPERTY(BlueprintReadOnly, Category = "Drift")
	float DriftScore = 0.0f;

	/** Current drift chain multiplier (increases with sustained drifts) */
	UPROPERTY(BlueprintReadOnly, Category = "Drift|Chain")
	int32 ChainMultiplier = 1;

	/** Time since last drift ended (for chain continuation window) */
	UPROPERTY(BlueprintReadOnly, Category = "Drift|Chain")
	float TimeSinceLastDrift = 0.0f;

	/** Total score accumulated in current drift chain */
	UPROPERTY(BlueprintReadOnly, Category = "Drift|Chain")
	float ChainTotalScore = 0.0f;

	/** Number of drifts in current chain */
	UPROPERTY(BlueprintReadOnly, Category = "Drift|Chain")
	int32 DriftsInChain = 0;

	/** Current angle tier for bonus calculation */
	UPROPERTY(BlueprintReadOnly, Category = "Drift|Scoring")
	EMGDriftAngleTier CurrentAngleTier = EMGDriftAngleTier::None;

	/** Peak angle achieved during this drift */
	UPROPERTY(BlueprintReadOnly, Category = "Drift|Scoring")
	float PeakAngle = 0.0f;

	/** Whether drift direction changed during this drift (style bonus) */
	UPROPERTY(BlueprintReadOnly, Category = "Drift|Scoring")
	bool bDirectionChanged = false;

	/** Time spent in "near-miss" state (close to walls/obstacles) */
	UPROPERTY(BlueprintReadOnly, Category = "Drift|Scoring")
	float NearMissTime = 0.0f;
};

/**
 * @brief Tire temperature state with zone-based modeling
 */
USTRUCT(BlueprintType)
struct FMGTireTemperature
{
	GENERATED_BODY()

	/** Inner edge temperature (Celsius) */
	UPROPERTY(BlueprintReadOnly, Category = "Temperature")
	float InnerTemp = 25.0f;

	/** Middle tread temperature (Celsius) */
	UPROPERTY(BlueprintReadOnly, Category = "Temperature")
	float MiddleTemp = 25.0f;

	/** Outer edge temperature (Celsius) */
	UPROPERTY(BlueprintReadOnly, Category = "Temperature")
	float OuterTemp = 25.0f;

	/**
	 * @brief Get average tire temperature
	 * @return Average temperature across all three zones
	 */
	float GetAverageTemp() const { return (InnerTemp + MiddleTemp + OuterTemp) / 3.0f; }

	/**
	 * @brief Get grip multiplier based on temperature (optimal around 80-100C)
	 * @return Grip coefficient multiplier (0.7 to 1.05)
	 */
	float GetGripMultiplier() const
	{
		float AvgTemp = GetAverageTemp();
		if (AvgTemp < 50.0f) return 0.7f + (AvgTemp / 50.0f) * 0.2f; // Cold tires = less grip
		if (AvgTemp <= 100.0f) return 0.9f + ((AvgTemp - 50.0f) / 50.0f) * 0.15f; // Warming up
		if (AvgTemp <= 120.0f) return 1.05f; // Optimal
		return 1.05f - ((AvgTemp - 120.0f) / 50.0f) * 0.25f; // Overheating = degradation
	}
};

/**
 * @brief Weight transfer state for dynamic load calculation
 */
USTRUCT(BlueprintType)
struct FMGWeightTransfer
{
	GENERATED_BODY()

	/** Front-rear weight transfer (positive = more front, negative = more rear) */
	UPROPERTY(BlueprintReadOnly, Category = "Weight")
	float LongitudinalTransfer = 0.0f;

	/** Left-right weight transfer (positive = more right, negative = more left) */
	UPROPERTY(BlueprintReadOnly, Category = "Weight")
	float LateralTransfer = 0.0f;

	/**
	 * @brief Get weight multiplier for specific wheel
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Load multiplier for the specified wheel
	 */
	float GetWheelLoadMultiplier(int32 WheelIndex) const
	{
		float BaseLoad = 1.0f;
		bool bFront = WheelIndex < 2;
		bool bRight = (WheelIndex == 1 || WheelIndex == 3);

		// Longitudinal transfer
		if (bFront) BaseLoad += LongitudinalTransfer * 0.15f;
		else BaseLoad -= LongitudinalTransfer * 0.15f;

		// Lateral transfer
		if (bRight) BaseLoad += LateralTransfer * 0.12f;
		else BaseLoad -= LateralTransfer * 0.12f;

		return FMath::Clamp(BaseLoad, 0.3f, 1.8f);
	}
};

/**
 * @brief Turbo state for advanced lag simulation
 */
USTRUCT(BlueprintType)
struct FMGTurboState
{
	GENERATED_BODY()

	/** Current turbine shaft speed (RPM) */
	UPROPERTY(BlueprintReadOnly, Category = "Turbo")
	float ShaftRPM = 0.0f;

	/** Maximum shaft RPM for this turbo configuration */
	UPROPERTY(BlueprintReadOnly, Category = "Turbo")
	float MaxShaftRPM = 150000.0f;

	/** Current compressor efficiency (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Turbo")
	float CompressorEfficiency = 0.0f;

	/** Exhaust gas temperature (Celsius) - affects spool rate */
	UPROPERTY(BlueprintReadOnly, Category = "Turbo")
	float ExhaustGasTemp = 400.0f;

	/** Backpressure factor (affects spool and power) */
	UPROPERTY(BlueprintReadOnly, Category = "Turbo")
	float BackpressureFactor = 1.0f;

	/** Whether turbo is in surge condition (compressor stall) */
	UPROPERTY(BlueprintReadOnly, Category = "Turbo")
	bool bInSurge = false;
};

/**
 * @brief Part wear effects on vehicle handling
 */
USTRUCT(BlueprintType)
struct FMGPartWearEffects
{
	GENERATED_BODY()

	/** Suspension wear reduces damping effectiveness */
	UPROPERTY(BlueprintReadOnly, Category = "Wear")
	float SuspensionEfficiency = 1.0f;

	/** Brake pad wear affects stopping power */
	UPROPERTY(BlueprintReadOnly, Category = "Wear")
	float BrakePadEfficiency = 1.0f;

	/** Steering component wear affects responsiveness */
	UPROPERTY(BlueprintReadOnly, Category = "Wear")
	float SteeringPrecision = 1.0f;

	/** Drivetrain wear affects power delivery */
	UPROPERTY(BlueprintReadOnly, Category = "Wear")
	float DrivetrainEfficiency = 1.0f;

	/** Engine wear affects power output */
	UPROPERTY(BlueprintReadOnly, Category = "Wear")
	float EngineEfficiency = 1.0f;

	/** Turbo/supercharger wear affects boost */
	UPROPERTY(BlueprintReadOnly, Category = "Wear")
	float ForcedInductionEfficiency = 1.0f;
};

/**
 * @brief Engine state information with enhanced turbo modeling
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

	/** Anti-lag system active (turbo) */
	UPROPERTY(BlueprintReadOnly)
	bool bAntiLagActive = false;

	/** Launch control engaged */
	UPROPERTY(BlueprintReadOnly)
	bool bLaunchControlEngaged = false;

	/** Launch control target RPM */
	UPROPERTY(BlueprintReadOnly)
	float LaunchControlRPM = 4500.0f;

	/** Clutch engagement (0 = disengaged, 1 = fully engaged) */
	UPROPERTY(BlueprintReadOnly)
	float ClutchEngagement = 1.0f;

	/** Brake temperature (affects fade) */
	UPROPERTY(BlueprintReadOnly)
	float BrakeTemperature = 50.0f;

	/** Brake fade multiplier (1 = no fade, lower = faded) */
	UPROPERTY(BlueprintReadOnly)
	float BrakeFadeMultiplier = 1.0f;

	/** Current power output from dyno curve (HP) */
	UPROPERTY(BlueprintReadOnly)
	float CurrentHorsepower = 0.0f;

	/** Current torque output from dyno curve (lb-ft) */
	UPROPERTY(BlueprintReadOnly)
	float CurrentTorque = 0.0f;

	/** Advanced turbo state */
	UPROPERTY(BlueprintReadOnly)
	FMGTurboState TurboState;
};

/**
 * @brief Custom vehicle movement component for MIDNIGHT GRIND
 *
 * Extends Chaos vehicle physics with arcade-tuned handling, featuring:
 * - Advanced drift scoring with chain multipliers and angle bonuses
 * - Realistic turbo lag simulation with shaft inertia modeling
 * - Power curve / dyno integration for RPM-dependent power output
 * - Part wear effects on handling characteristics
 * - Tire temperature and weight transfer physics
 * - Aerodynamic downforce and drag simulation
 * - Anti-lag and launch control systems
 *
 * @see UChaosWheeledVehicleMovementComponent
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

	/**
	 * @brief Apply vehicle configuration data to this component
	 * @param VehicleData Configuration data to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Configuration")
	void ApplyVehicleConfiguration(const FMGVehicleData& VehicleData);

	/**
	 * @brief Get current vehicle configuration
	 * @return Reference to current configuration data
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Configuration")
	const FMGVehicleData& GetVehicleConfiguration() const { return CurrentConfiguration; }

	// ==========================================
	// INPUT
	// ==========================================

	/**
	 * @brief Set throttle input
	 * @param Value Throttle position (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetThrottleInput(float Value);

	/**
	 * @brief Set brake input
	 * @param Value Brake pressure (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetBrakeInput(float Value);

	/**
	 * @brief Set steering input
	 * @param Value Steering angle (-1 to 1, negative = left)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetSteeringInput(float Value);

	/**
	 * @brief Set handbrake state
	 * @param bEngaged Whether handbrake is engaged
	 */
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
	// DAMAGE EFFECTS
	// ==========================================

	/**
	 * @brief Set tire grip multiplier from external damage system
	 * @param Multiplier Grip multiplier (0.1-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void SetTireGripMultiplier(float Multiplier);

	/** Get current tire grip multiplier */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	float GetTireGripMultiplier() const { return TireGripMultiplier; }

	/**
	 * @brief Set max speed multiplier from external damage system
	 * @param Multiplier Speed multiplier (0.3-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void SetMaxSpeedMultiplier(float Multiplier);

	/** Get current max speed multiplier */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	float GetMaxSpeedMultiplier() const { return MaxSpeedMultiplier; }

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

	/** Check if handbrake is engaged */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	bool IsHandbrakeEngaged() const { return bHandbrakeEngaged; }

	/** Get tire temperature for wheel */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FMGTireTemperature GetTireTemperature(int32 WheelIndex) const;

	/** Get weight transfer state */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FMGWeightTransfer GetWeightTransfer() const { return WeightTransferState; }

	/** Get current downforce in Newtons */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	float GetCurrentDownforce() const;

	/** Check if launch control is available */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	bool IsLaunchControlAvailable() const;

	/** Get current part wear effects */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FMGPartWearEffects GetPartWearEffects() const { return PartWearEffects; }

	/**
	 * @brief Sample power curve at specific RPM
	 * @param RPM Engine RPM to sample
	 * @param OutHorsepower Output horsepower at this RPM
	 * @param OutTorque Output torque at this RPM
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Dyno")
	void SamplePowerCurve(float RPM, float& OutHorsepower, float& OutTorque) const;

	// ==========================================
	// ADVANCED INPUT
	// ==========================================

	/** Engage launch control */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void EngageLaunchControl();

	/** Release launch control (launch the car) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void ReleaseLaunchControl();

	/** Toggle anti-lag system */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetAntiLagEnabled(bool bEnabled);

	/** Set clutch input (0 = disengaged, 1 = fully engaged) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Input")
	void SetClutchInput(float Value);

	/**
	 * @brief Manually update part wear effects
	 * Called automatically when configuration changes, but can be triggered manually
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Wear")
	void UpdatePartWearEffects();

	// ==========================================
	// TUNING PARAMETERS - DRIFT
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

	// ==========================================
	// TUNING PARAMETERS - DRIFT SCORING
	// ==========================================

	/** Base points per second while drifting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftBasePointsPerSecond = 100.0f;

	/** Multiplier for angle bonus (applied per degree over threshold) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftAngleBonusMultiplier = 2.0f;

	/** Speed bonus multiplier (normalized to 100 MPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftSpeedBonusMultiplier = 1.5f;

	/** Time window to continue chain after drift ends (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftChainContinuationWindow = 2.0f;

	/** Maximum chain multiplier achievable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	int32 DriftMaxChainMultiplier = 10;

	/** Duration required to increase chain multiplier (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftChainBuildTime = 3.0f;

	/** Bonus multiplier for direction changes during drift */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftDirectionChangeBonusMultiplier = 1.25f;

	/** Angle thresholds for tier bonuses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftAngleTierMild = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftAngleTierStandard = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftAngleTierAggressive = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|DriftScoring")
	float DriftAngleTierExtreme = 75.0f;

	// ==========================================
	// TUNING PARAMETERS - HANDBRAKE
	// ==========================================

	/** Handbrake settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Handbrake")
	float HandbrakeFrictionMultiplier = 0.3f; // Rear tire friction when handbrake engaged

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Handbrake")
	float HandbrakeRearBias = 1.0f; // How much handbrake affects rear vs front

	// ==========================================
	// TUNING PARAMETERS - ARCADE
	// ==========================================

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

	// ==========================================
	// TUNING PARAMETERS - TIRES
	// ==========================================

	/** Tire model */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Tires")
	float BaseTireGrip = 1.0f; // Multiplier for all tire grip

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Tires")
	float TireWearRate = 1.0f; // How fast tires degrade

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Tires")
	float WetGripMultiplier = 0.7f; // Grip reduction in wet conditions

	// ==========================================
	// TUNING PARAMETERS - TURBO (ADVANCED)
	// ==========================================

	/** Basic turbo simulation multiplier (0 = no lag, 1 = realistic lag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo")
	float TurboLagSimulation = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo")
	float BoostBuildupRate = 2.0f; // How fast boost builds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo")
	float BoostDecayRate = 4.0f; // How fast boost falls off throttle

	/** Turbo shaft inertia factor (higher = more lag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo|Advanced")
	float TurboShaftInertia = 1.0f;

	/** Exhaust flow coefficient (affects spool rate at high RPM) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo|Advanced")
	float TurboExhaustFlowCoef = 1.0f;

	/** Compressor surge threshold (boost PSI where surge occurs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo|Advanced")
	float TurboSurgeThreshold = 25.0f;

	/** Compressor efficiency at optimal point (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Turbo|Advanced")
	float TurboCompressorPeakEfficiency = 0.78f;

	// ==========================================
	// TUNING PARAMETERS - NITROUS
	// ==========================================

	/** Nitrous settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Nitrous")
	float NitrousPowerMultiplier = 1.5f; // Power boost when active

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Nitrous")
	float NitrousConsumptionRate = 10.0f; // Percent per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Nitrous")
	float NitrousMinimumRPMPercent = 0.5f; // Min RPM to activate (% of redline)

	// ==========================================
	// TUNING PARAMETERS - TIRE TEMPERATURE
	// ==========================================

	/** Tire temperature settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|TireTemp")
	float TireHeatRate = 5.0f; // Degrees per second at full slip

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|TireTemp")
	float TireCoolRate = 2.0f; // Degrees per second ambient cooling

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|TireTemp")
	float AmbientTemperature = 25.0f; // Environmental temperature

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|TireTemp")
	float OptimalTireTemp = 90.0f; // Best grip temperature

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|TireTemp")
	float TireTempGripInfluence = 0.5f; // 0-1, how much temp affects grip

	// ==========================================
	// TUNING PARAMETERS - WEIGHT TRANSFER
	// ==========================================

	/** Weight transfer settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|WeightTransfer")
	float WeightTransferRate = 8.0f; // How fast weight shifts

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|WeightTransfer")
	float LongitudinalTransferFactor = 1.0f; // Front-rear sensitivity

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|WeightTransfer")
	float LateralTransferFactor = 1.0f; // Left-right sensitivity

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|WeightTransfer")
	float CenterOfGravityHeight = 0.5f; // Meters, affects rollover

	// ==========================================
	// TUNING PARAMETERS - AERODYNAMICS
	// ==========================================

	/** Aerodynamics settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Aero")
	float DownforceCoefficient = 0.3f; // Base downforce factor

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Aero")
	float DragCoefficient = 0.35f; // Air resistance factor

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Aero")
	float FrontalArea = 2.2f; // Square meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Aero")
	float DownforceFrontBias = 0.4f; // 0-1, distribution front vs rear

	// ==========================================
	// TUNING PARAMETERS - ANTI-LAG
	// ==========================================

	/** Anti-lag settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|AntiLag")
	float AntiLagBoostRetention = 0.7f; // How much boost kept off-throttle

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|AntiLag")
	float AntiLagMinRPM = 3500.0f; // Min RPM for anti-lag

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|AntiLag")
	float AntiLagFuelConsumption = 1.5f; // Extra fuel used

	// ==========================================
	// TUNING PARAMETERS - LAUNCH CONTROL
	// ==========================================

	/** Launch control settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Launch")
	float LaunchControlDefaultRPM = 4500.0f; // Default launch RPM

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Launch")
	float LaunchControlClutchSlip = 0.15f; // Clutch slip for smooth launch

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Launch")
	float LaunchControlBoostBuild = 1.5f; // Boost buildup multiplier

	// ==========================================
	// TUNING PARAMETERS - BRAKES
	// ==========================================

	/** Brake settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Brakes")
	float BrakeHeatRate = 15.0f; // Degrees per second under heavy braking

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Brakes")
	float BrakeCoolRate = 8.0f; // Degrees per second cooling

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Brakes")
	float BrakeFadeStartTemp = 400.0f; // Temperature where fade begins

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Brakes")
	float BrakeFadeMaxTemp = 600.0f; // Maximum brake temp (total fade)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Brakes")
	float BrakeFadeMinEfficiency = 0.3f; // Minimum brake efficiency when fully faded

	// ==========================================
	// TUNING PARAMETERS - PART WEAR
	// ==========================================

	/** Part wear threshold for warning (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|PartWear")
	float PartWearWarningThreshold = 30.0f;

	/** How much worn suspension affects handling (0 = no effect, 1 = full effect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|PartWear")
	float SuspensionWearHandlingImpact = 0.3f;

	/** How much worn brakes affect stopping power */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|PartWear")
	float BrakeWearStoppingImpact = 0.4f;

	/** How much worn steering affects precision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|PartWear")
	float SteeringWearPrecisionImpact = 0.2f;

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

	/** Called when drift score is awarded (during drift) */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnDriftScoreAwarded OnDriftScoreAwarded;

	/** Called when drift chain is broken */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnDriftChainBroken OnDriftChainBroken;

	/** Called when a part's wear reaches warning threshold */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnPartWearWarning OnPartWearWarning;

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
	FMGPartWearEffects PartWearEffects;

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

	// Damage multipliers
	UPROPERTY()
	float TireGripMultiplier = 1.0f;

	UPROPERTY()
	float MaxSpeedMultiplier = 1.0f;

	// Tire temperature per wheel (FL, FR, RL, RR)
	UPROPERTY()
	FMGTireTemperature TireTemperatures[4];

	// Weight transfer state
	UPROPERTY()
	FMGWeightTransfer WeightTransferState;

	// Current downforce in Newtons
	UPROPERTY()
	float CurrentDownforceN = 0.0f;

	// Anti-lag system enabled
	UPROPERTY()
	bool bAntiLagEnabled = false;

	// Clutch input (0 = disengaged, 1 = engaged)
	UPROPERTY()
	float ClutchInput = 1.0f;

	// Launch control active timer
	UPROPERTY()
	float LaunchControlTimer = 0.0f;

	// Previous frame velocity for acceleration calculation
	FVector LastFrameVelocity = FVector::ZeroVector;

	// Last drift direction for direction change detection
	float LastDriftDirection = 0.0f;

	// Drift chain build timer
	float DriftChainBuildTimer = 0.0f;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Update engine simulation */
	virtual void UpdateEngineSimulation(float DeltaTime);

	/** Update turbo/boost simulation with advanced shaft modeling */
	virtual void UpdateBoostSimulation(float DeltaTime);

	/** Update drift detection and enhanced scoring system */
	virtual void UpdateDriftPhysics(float DeltaTime);

	/** Update nitrous system */
	virtual void UpdateNitrousSystem(float DeltaTime);

	/** Apply stability control */
	virtual void ApplyStabilityControl(float DeltaTime);

	/** Apply anti-flip torque */
	virtual void ApplyAntiFlipForce(float DeltaTime);

	/** Calculate modified tire friction including wear effects */
	virtual float CalculateTireFriction(int32 WheelIndex) const;

	/** Calculate current power output from dyno curve with all modifiers */
	virtual float CalculateCurrentPower() const;

	/** Get tire grip coefficient for compound */
	static float GetTireCompoundGrip(EMGTireCompound Compound);

	/** Perform gear shift */
	void PerformGearShift(int32 NewGear);

	/** Update tire temperature simulation */
	virtual void UpdateTireTemperatures(float DeltaTime);

	/** Update weight transfer based on acceleration */
	virtual void UpdateWeightTransfer(float DeltaTime);

	/** Update aerodynamic forces */
	virtual void UpdateAerodynamics(float DeltaTime);

	/** Update anti-lag system with improved turbo modeling */
	virtual void UpdateAntiLag(float DeltaTime);

	/** Update launch control */
	virtual void UpdateLaunchControl(float DeltaTime);

	/** Update brake temperature and fade */
	virtual void UpdateBrakeSystem(float DeltaTime);

	/** Apply differential behavior based on type */
	virtual void ApplyDifferentialBehavior(float DeltaTime);

	/** Calculate speed-dependent steering angle reduction */
	float CalculateSpeedSteeringFactor() const;

	/** Get differential lock factor based on type */
	float GetDifferentialLockFactor() const;

	/**
	 * @brief Calculate drift angle tier from current angle
	 * @param AbsAngle Absolute drift angle in degrees
	 * @return Drift angle tier enum
	 */
	EMGDriftAngleTier CalculateDriftAngleTier(float AbsAngle) const;

	/**
	 * @brief Get bonus multiplier for drift angle tier
	 * @param Tier Drift angle tier
	 * @return Bonus multiplier for scoring
	 */
	float GetDriftTierBonusMultiplier(EMGDriftAngleTier Tier) const;

	/**
	 * @brief Update drift scoring with chain multipliers and bonuses
	 * @param DeltaTime Frame delta time
	 */
	void UpdateDriftScoring(float DeltaTime);

	/**
	 * @brief Award drift score and broadcast event
	 * @param BaseScore Base score before multipliers
	 * @param AngleBonus Angle-based bonus multiplier
	 */
	void AwardDriftScore(float BaseScore, float AngleBonus);

	/**
	 * @brief Break the current drift chain
	 */
	void BreakDriftChain();

	/**
	 * @brief Update advanced turbo shaft simulation
	 * @param DeltaTime Frame delta time
	 */
	void UpdateTurboShaftSimulation(float DeltaTime);

	/**
	 * @brief Apply part wear effects to handling parameters
	 */
	void ApplyPartWearToHandling();
};

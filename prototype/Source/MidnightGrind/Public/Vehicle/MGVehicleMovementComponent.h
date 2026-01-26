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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClutchOverheating, float, Temperature, float, WearLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClutchBurnout);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyShift, float, OverRevAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDifferentialLockup, float, LockPercent, bool, bUnderAccel);

/**
 * @brief Differential lock state for detailed simulation
 *
 * Provides comprehensive differential behavior data including lock percentage,
 * torque distribution, and tire speed differentials for realistic LSD simulation.
 */
USTRUCT(BlueprintType)
struct FMGDifferentialState
{
	GENERATED_BODY()

	/** Current lock percentage (0 = fully open, 1 = fully locked) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential")
	float LockPercent = 0.0f;

	/** Lock percentage from acceleration-side clutch pack */
	UPROPERTY(BlueprintReadOnly, Category = "Differential")
	float AccelLockPercent = 0.0f;

	/** Lock percentage from deceleration-side clutch pack */
	UPROPERTY(BlueprintReadOnly, Category = "Differential")
	float DecelLockPercent = 0.0f;

	/** Torque sent to left wheel (normalized 0-1, 0.5 = equal split) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Distribution")
	float LeftWheelTorqueRatio = 0.5f;

	/** Torque sent to right wheel (normalized 0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Distribution")
	float RightWheelTorqueRatio = 0.5f;

	/** Left wheel angular velocity (rad/s) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Speed")
	float LeftWheelAngularVelocity = 0.0f;

	/** Right wheel angular velocity (rad/s) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Speed")
	float RightWheelAngularVelocity = 0.0f;

	/** Speed differential between wheels (rad/s, positive = left faster) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Speed")
	float WheelSpeedDifferential = 0.0f;

	/** Normalized speed differential (-1 to 1, for UI) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Speed")
	float NormalizedSpeedDiff = 0.0f;

	/** Input torque to differential (Nm) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Torque")
	float InputTorque = 0.0f;

	/** Bias torque from LSD mechanism (Nm) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Torque")
	float BiasTorque = 0.0f;

	/** Whether differential is currently transferring torque via LSD */
	UPROPERTY(BlueprintReadOnly, Category = "Differential")
	bool bIsLocking = false;

	/** Whether lockup is occurring under acceleration */
	UPROPERTY(BlueprintReadOnly, Category = "Differential")
	bool bUnderAcceleration = false;

	/** Current preload torque being applied (Nm) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|LSD")
	float ActivePreloadTorque = 0.0f;

	/** Torsen torque bias ratio currently active */
	UPROPERTY(BlueprintReadOnly, Category = "Differential|Torsen")
	float TorsenBiasRatio = 1.0f;
};

/**
 * @brief LSD configuration parameters for tuning differential behavior
 *
 * Provides detailed control over clutch-type LSD behavior including
 * preload settings and ramp angles for acceleration/deceleration phases.
 */
USTRUCT(BlueprintType)
struct FMGLSDConfiguration
{
	GENERATED_BODY()

	/**
	 * @brief Preload torque in Nm
	 *
	 * Minimum torque at which the differential begins to lock.
	 * Higher values provide more initial lockup even at low torque,
	 * making the diff more aggressive on corner entry/exit.
	 * Typical range: 10-100 Nm
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Preload", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float PreloadTorqueNm = 30.0f;

	/**
	 * @brief Acceleration ramp angle in degrees
	 *
	 * Controls how aggressively the diff locks under acceleration.
	 * Lower angles = more aggressive lockup (steeper ramp).
	 * Higher angles = smoother, more progressive lockup.
	 *
	 * Real-world typical values:
	 * - 30-40 deg: Aggressive (drift/track)
	 * - 45-55 deg: Balanced (street/sport)
	 * - 60-80 deg: Mild (comfort/traction)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Ramp Angles", meta = (ClampMin = "20.0", ClampMax = "90.0"))
	float AccelRampAngleDeg = 45.0f;

	/**
	 * @brief Deceleration ramp angle in degrees
	 *
	 * Controls lockup behavior during engine braking/coast.
	 * 1-way LSD: Use 90 degrees (no decel lock)
	 * 1.5-way LSD: Higher than accel angle (partial decel lock)
	 * 2-way LSD: Equal to accel angle (symmetric lockup)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Ramp Angles", meta = (ClampMin = "20.0", ClampMax = "90.0"))
	float DecelRampAngleDeg = 65.0f;

	/**
	 * @brief Maximum lock percentage achievable
	 *
	 * Limits how much the differential can lock up.
	 * 1.0 = can fully lock (welded behavior at max)
	 * Lower values = always some slip allowed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Limits", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxLockPercent = 0.85f;

	/**
	 * @brief Lock response speed (how fast lockup changes)
	 *
	 * Higher values = faster lockup response (more aggressive)
	 * Lower values = smoother, more predictable transitions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Response", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float LockResponseRate = 15.0f;

	/**
	 * @brief Clutch pack friction coefficient
	 *
	 * Affects how much torque the clutch packs can transfer.
	 * Higher = more aggressive lockup for given input torque.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Clutch", meta = (ClampMin = "0.1", ClampMax = "1.5"))
	float ClutchFrictionCoef = 0.8f;

	/**
	 * @brief Number of clutch plates (affects lockup aggression)
	 *
	 * More plates = higher torque capacity and faster lockup.
	 * Typical range: 4-12 plates for performance applications.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Clutch", meta = (ClampMin = "2", ClampMax = "16"))
	int32 ClutchPlateCount = 6;

	/**
	 * @brief Torsen torque bias ratio (TBR)
	 *
	 * Only used for Torsen-type differentials.
	 * Defines max torque split ratio (e.g., 3.0 = can send 3x torque to slower wheel).
	 * Higher values = more aggressive torque transfer capability.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Torsen", meta = (ClampMin = "1.0", ClampMax = "6.0"))
	float TorsenBiasRatio = 2.5f;

	/**
	 * @brief Torsen response sensitivity
	 *
	 * How quickly Torsen responds to speed differences.
	 * Higher = more immediate torque vectoring.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Torsen", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float TorsenSensitivity = 1.5f;

	/**
	 * @brief Minimum speed differential to trigger lockup (rad/s)
	 *
	 * Below this threshold, diff behaves more openly.
	 * Prevents lockup during normal straight-line driving.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|Thresholds", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float MinSpeedDiffThreshold = 0.5f;

	/**
	 * @brief Coast behavior factor for 1.5-way type
	 *
	 * Multiplier for decel-side lockup relative to accel-side.
	 * 0.0 = pure 1-way (no decel lock)
	 * 1.0 = pure 2-way (equal accel/decel)
	 * 0.3-0.6 = typical 1.5-way range
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LSD|1.5-Way", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CoastLockFactor = 0.4f;
};

/**
 * @brief Power distribution visualization data for UI display
 *
 * Provides all data needed to render real-time power distribution
 * and differential state in the vehicle HUD or telemetry overlay.
 */
USTRUCT(BlueprintType)
struct FMGPowerDistributionData
{
	GENERATED_BODY()

	/** Front-left wheel power percentage (0-100) */
	UPROPERTY(BlueprintReadOnly, Category = "Distribution")
	float FrontLeftPower = 0.0f;

	/** Front-right wheel power percentage (0-100) */
	UPROPERTY(BlueprintReadOnly, Category = "Distribution")
	float FrontRightPower = 0.0f;

	/** Rear-left wheel power percentage (0-100) */
	UPROPERTY(BlueprintReadOnly, Category = "Distribution")
	float RearLeftPower = 0.0f;

	/** Rear-right wheel power percentage (0-100) */
	UPROPERTY(BlueprintReadOnly, Category = "Distribution")
	float RearRightPower = 0.0f;

	/** Front axle total power percentage */
	UPROPERTY(BlueprintReadOnly, Category = "Distribution")
	float FrontAxlePower = 0.0f;

	/** Rear axle total power percentage */
	UPROPERTY(BlueprintReadOnly, Category = "Distribution")
	float RearAxlePower = 0.0f;

	/** Center differential bias (AWD only, 0 = rear, 1 = front) */
	UPROPERTY(BlueprintReadOnly, Category = "Distribution")
	float CenterDiffBias = 0.5f;

	/** Front differential state */
	UPROPERTY(BlueprintReadOnly, Category = "Differential")
	FMGDifferentialState FrontDiffState;

	/** Rear differential state */
	UPROPERTY(BlueprintReadOnly, Category = "Differential")
	FMGDifferentialState RearDiffState;

	/** Center differential state (AWD only) */
	UPROPERTY(BlueprintReadOnly, Category = "Differential")
	FMGDifferentialState CenterDiffState;

	/** Total drivetrain power loss percentage */
	UPROPERTY(BlueprintReadOnly, Category = "Efficiency")
	float DrivetrainLossPercent = 0.0f;

	/** Per-wheel slip ratios for visualization */
	UPROPERTY(BlueprintReadOnly, Category = "Slip")
	float WheelSlipRatios[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	/** Per-wheel spin status (true if significant wheelspin) */
	UPROPERTY(BlueprintReadOnly, Category = "Slip")
	bool bWheelSpinning[4] = { false, false, false, false };
};

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
 * @brief Road surface types with different grip characteristics
 */
UENUM(BlueprintType)
enum class EMGSurfaceType : uint8
{
	/** Smooth asphalt (optimal grip) */
	Asphalt UMETA(DisplayName = "Asphalt"),

	/** Concrete surface (slightly less grip than asphalt) */
	Concrete UMETA(DisplayName = "Concrete"),

	/** Wet asphalt/concrete (significantly reduced grip) */
	Wet UMETA(DisplayName = "Wet Surface"),

	/** Dirt road (reduced grip, loose surface) */
	Dirt UMETA(DisplayName = "Dirt"),

	/** Gravel surface (low grip, high slip) */
	Gravel UMETA(DisplayName = "Gravel"),

	/** Ice (extremely low grip) */
	Ice UMETA(DisplayName = "Ice"),

	/** Snow (low grip, soft surface) */
	Snow UMETA(DisplayName = "Snow"),

	/** Grass (very low grip when wet, moderate when dry) */
	Grass UMETA(DisplayName = "Grass"),

	/** Sand (extremely low grip, high resistance) */
	Sand UMETA(DisplayName = "Sand"),

	/** Off-road trail (mixed surface, reduced grip) */
	OffRoad UMETA(DisplayName = "Off-Road")
};

/**
 * @brief Per-wheel surface tracking for dynamic grip calculation
 */
USTRUCT(BlueprintType)
struct FMGWheelSurfaceState
{
	GENERATED_BODY()

	/** Current surface type this wheel is on */
	UPROPERTY(BlueprintReadOnly, Category = "Surface")
	EMGSurfaceType SurfaceType = EMGSurfaceType::Asphalt;

	/** Time spent on current surface (for temperature/wear effects) */
	UPROPERTY(BlueprintReadOnly, Category = "Surface")
	float TimeOnSurface = 0.0f;

	/** Surface wetness level (0 = dry, 1 = fully wet) */
	UPROPERTY(BlueprintReadOnly, Category = "Surface")
	float WetnessLevel = 0.0f;

	/** Whether wheel is currently in contact with ground */
	UPROPERTY(BlueprintReadOnly, Category = "Surface")
	bool bHasContact = true;
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
 * @brief Clutch wear state for realistic clutch simulation
 */
USTRUCT(BlueprintType)
struct FMGClutchWearState
{
	GENERATED_BODY()

	/** Current clutch temperature (Celsius) */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	float ClutchTemperature = 50.0f;

	/** Clutch wear level (0 = new, 1 = completely worn) */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	float WearLevel = 0.0f;

	/** Current friction coefficient (degrades with heat and wear) */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	float FrictionCoefficient = 1.0f;

	/** Time spent slipping this engagement */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	float CurrentSlipDuration = 0.0f;

	/** Accumulated slip damage this session */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	float SessionSlipDamage = 0.0f;

	/** Is clutch currently slipping */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	bool bIsSlipping = false;

	/** Is clutch overheating (smell of burning clutch!) */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	bool bIsOverheating = false;

	/** Clutch burnt out (needs replacement) */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	bool bIsBurntOut = false;

	/** Number of hard launches this session */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	int32 HardLaunchCount = 0;

	/** Number of money shifts (missed downshift) */
	UPROPERTY(BlueprintReadOnly, Category = "Clutch")
	int32 MoneyShiftCount = 0;

	/**
	 * @brief Get torque transfer efficiency based on temperature and wear
	 * @return Efficiency multiplier (0-1, lower when hot/worn)
	 */
	float GetTorqueTransferEfficiency() const
	{
		if (bIsBurntOut) return 0.1f; // Clutch barely works

		float Efficiency = 1.0f;

		// Wear reduces efficiency
		Efficiency *= (1.0f - WearLevel * 0.3f);

		// High temperature causes slip
		if (ClutchTemperature > 200.0f)
		{
			float HeatPenalty = (ClutchTemperature - 200.0f) / 200.0f;
			Efficiency *= FMath::Max(0.5f, 1.0f - HeatPenalty * 0.4f);
		}

		return FMath::Clamp(Efficiency, 0.0f, 1.0f);
	}

	/**
	 * @brief Get maximum holdable torque
	 * @param BaseClutchTorque The clutch's rated torque capacity
	 * @return Actual holdable torque
	 */
	float GetMaxHoldableTorque(float BaseClutchTorque) const
	{
		return BaseClutchTorque * FrictionCoefficient * GetTorqueTransferEfficiency();
	}
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

	/** Get current clutch wear state */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FMGClutchWearState GetClutchWearState() const { return ClutchWearState; }

	/** Get clutch temperature as percentage of burnout threshold */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	float GetClutchTemperaturePercent() const
	{
		return (ClutchWearState.ClutchTemperature - ClutchAmbientTemp) / (ClutchBurnoutTemp - ClutchAmbientTemp);
	}

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

	// ==========================================
	// DIFFERENTIAL STATE QUERIES
	// ==========================================

	/**
	 * @brief Get current rear differential state
	 * @return Rear differential state including lock percent and torque distribution
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Differential")
	FMGDifferentialState GetRearDifferentialState() const { return RearDiffState; }

	/**
	 * @brief Get current front differential state (FWD/AWD)
	 * @return Front differential state
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Differential")
	FMGDifferentialState GetFrontDifferentialState() const { return FrontDiffState; }

	/**
	 * @brief Get current center differential state (AWD only)
	 * @return Center differential state
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Differential")
	FMGDifferentialState GetCenterDifferentialState() const { return CenterDiffState; }

	/**
	 * @brief Get complete power distribution data for UI visualization
	 * @return Power distribution across all wheels and differentials
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Differential")
	FMGPowerDistributionData GetPowerDistribution() const { return PowerDistributionData; }

	/**
	 * @brief Get effective differential lock percentage for a given axle
	 * @param bFrontAxle True for front, false for rear
	 * @return Lock percentage (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Differential")
	float GetAxleLockPercent(bool bFrontAxle) const;

	/**
	 * @brief Get wheel angular velocity
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Angular velocity in rad/s
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Differential")
	float GetWheelAngularVelocity(int32 WheelIndex) const;

	/**
	 * @brief Get tire speed differential between left and right wheels on an axle
	 * @param bFrontAxle True for front axle, false for rear
	 * @return Speed differential in rad/s (positive = left faster)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Differential")
	float GetAxleSpeedDifferential(bool bFrontAxle) const;

	/**
	 * @brief Check if a wheel is spinning (excessive slip)
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return True if wheel is experiencing significant wheelspin
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Differential")
	bool IsWheelSpinningExcessively(int32 WheelIndex) const;

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
	// TUNING PARAMETERS - SURFACE GRIP
	// ==========================================

	/**
	 * @brief Surface type grip multipliers
	 * These values affect tire grip based on road surface
	 */

	/** Asphalt grip (baseline = 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float SurfaceGrip_Asphalt = 1.0f;

	/** Concrete grip (slightly less than asphalt) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float SurfaceGrip_Concrete = 0.95f;

	/** Wet surface grip (significantly reduced) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.3", ClampMax = "1.0"))
	float SurfaceGrip_Wet = 0.65f;

	/** Dirt road grip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.3", ClampMax = "0.9"))
	float SurfaceGrip_Dirt = 0.70f;

	/** Gravel surface grip (loose, high slip) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.2", ClampMax = "0.8"))
	float SurfaceGrip_Gravel = 0.55f;

	/** Ice grip (extremely low) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.1", ClampMax = "0.5"))
	float SurfaceGrip_Ice = 0.20f;

	/** Snow grip (low, soft) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.2", ClampMax = "0.6"))
	float SurfaceGrip_Snow = 0.40f;

	/** Grass grip (very low) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.2", ClampMax = "0.7"))
	float SurfaceGrip_Grass = 0.45f;

	/** Sand grip (extremely low, high resistance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.1", ClampMax = "0.5"))
	float SurfaceGrip_Sand = 0.30f;

	/** Off-road trail grip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Surface", meta = (ClampMin = "0.3", ClampMax = "0.8"))
	float SurfaceGrip_OffRoad = 0.60f;

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
	// TUNING PARAMETERS - DIFFERENTIAL
	// ==========================================

	/**
	 * @brief LSD configuration for rear differential
	 *
	 * Controls clutch-pack LSD behavior including preload,
	 * ramp angles, and lockup characteristics.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential")
	FMGLSDConfiguration RearLSDConfig;

	/**
	 * @brief LSD configuration for front differential (FWD/AWD)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential")
	FMGLSDConfiguration FrontLSDConfig;

	/**
	 * @brief LSD configuration for center differential (AWD only)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential")
	FMGLSDConfiguration CenterLSDConfig;

	/**
	 * @brief AWD front/rear torque split (0 = full rear, 1 = full front)
	 *
	 * Default bias for center differential torque distribution.
	 * 0.5 = 50/50 split
	 * 0.3 = 30% front, 70% rear (sporty RWD-biased AWD)
	 * 0.6 = 60% front, 40% rear (FWD-biased AWD)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AWDFrontBias = 0.35f;

	/**
	 * @brief Whether to apply torque vectoring via differential
	 *
	 * When enabled, allows differential to actively redistribute
	 * torque based on cornering conditions (for Torsen/eLSD behavior).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential")
	bool bEnableTorqueVectoring = false;

	/**
	 * @brief Torque vectoring intensity (0 = off, 1 = maximum)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TorqueVectoringIntensity = 0.5f;

	/**
	 * @brief Differential viscosity for viscous LSD simulation
	 *
	 * Higher values = more resistance to speed differences.
	 * Affects how "stiff" the diff feels during transitions.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DifferentialViscosity = 25.0f;

	/**
	 * @brief Enable realistic differential coast behavior
	 *
	 * When true, simulates drivetrain backlash and coast behavior.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential")
	bool bSimulateCoastBehavior = true;

	/**
	 * @brief Wheel speed differential threshold for open diff spin (rad/s)
	 *
	 * When exceeded, simulates inside wheel spinning freely
	 * for open differential behavior.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Differential", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float OpenDiffSpinThreshold = 2.0f;

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
	// TUNING PARAMETERS - CLUTCH WEAR
	// ==========================================

	/** Base clutch torque capacity (Nm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchTorqueCapacity = 500.0f;

	/** Clutch heat generation rate (degrees per second while slipping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchHeatRate = 50.0f;

	/** Clutch cooling rate (degrees per second when not slipping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchCoolRate = 15.0f;

	/** Ambient clutch temperature (Celsius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchAmbientTemp = 50.0f;

	/** Temperature where clutch starts to degrade (Celsius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchDegradeTemp = 200.0f;

	/** Temperature where clutch burns out (Celsius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchBurnoutTemp = 400.0f;

	/** Wear accumulation rate during slipping (per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchWearRate = 0.001f;

	/** Extra wear multiplier when overheating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchOverheatWearMultiplier = 3.0f;

	/** RPM difference threshold for detecting "money shift" (over-rev) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float MoneyShiftRPMThreshold = 2000.0f;

	/** Hard launch RPM threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float HardLaunchRPMThreshold = 5000.0f;

	/** Clutch slip threshold for detecting slip (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Clutch")
	float ClutchSlipDetectionThreshold = 0.1f;

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

	/** Called when clutch starts overheating */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnClutchOverheating OnClutchOverheating;

	/** Called when clutch burns out */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnClutchBurnout OnClutchBurnout;

	/** Called when a money shift (over-rev downshift) occurs */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnMoneyShift OnMoneyShift;

	/** Called when differential lockup changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnDifferentialLockup OnDifferentialLockup;

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

	/** Surface state per wheel (FL, FR, RL, RR) for dynamic grip calculation */
	UPROPERTY()
	FMGWheelSurfaceState WheelSurfaceStates[4];

	/** Current clutch wear state */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Clutch")
	FMGClutchWearState ClutchWearState;

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

	// ==========================================
	// DIFFERENTIAL STATE
	// ==========================================

	/** Rear differential state */
	UPROPERTY()
	FMGDifferentialState RearDiffState;

	/** Front differential state (FWD/AWD) */
	UPROPERTY()
	FMGDifferentialState FrontDiffState;

	/** Center differential state (AWD) */
	UPROPERTY()
	FMGDifferentialState CenterDiffState;

	/** Complete power distribution data for UI */
	UPROPERTY()
	FMGPowerDistributionData PowerDistributionData;

	/** Cached wheel angular velocities (rad/s) */
	float WheelAngularVelocities[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	/** Last broadcast lock percent (for event hysteresis) */
	float LastBroadcastLockPercent = 0.0f;

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

	/**
	 * @brief Get grip multiplier for specific surface type
	 * @param SurfaceType The surface type to query
	 * @return Grip multiplier for this surface (0.1 to 1.5)
	 */
	float GetSurfaceGripMultiplier(EMGSurfaceType SurfaceType) const;

	/**
	 * @brief Detect surface type from wheel contact
	 * @param WheelIndex Wheel index to detect (0-3)
	 * @return Detected surface type
	 */
	EMGSurfaceType DetectWheelSurfaceType(int32 WheelIndex) const;

	/**
	 * @brief Update surface detection for all wheels
	 * @param DeltaTime Time step for simulation
	 */
	virtual void UpdateSurfaceDetection(float DeltaTime);

	/** Perform gear shift */
	void PerformGearShift(int32 NewGear);

	/** Update tire temperature simulation */
	virtual void UpdateTireTemperatures(float DeltaTime);

	/** Update weight transfer based on acceleration */
	virtual void UpdateWeightTransfer(float DeltaTime);

	/** Update clutch wear and temperature simulation */
	virtual void UpdateClutchWear(float DeltaTime);

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

	/**
	 * @brief Update wheel angular velocities from physics simulation
	 * @param DeltaTime Frame delta time
	 */
	void UpdateWheelAngularVelocities(float DeltaTime);

	/**
	 * @brief Simulate open differential behavior (no lockup)
	 *
	 * Models classic open diff where torque goes to wheel with least
	 * resistance, causing inside wheel to spin freely in corners.
	 *
	 * @param DeltaTime Frame delta time
	 * @param OutState Differential state to update
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 * @param InputTorque Torque input to differential
	 */
	void SimulateOpenDifferential(
		float DeltaTime,
		FMGDifferentialState& OutState,
		int32 LeftWheelIdx,
		int32 RightWheelIdx,
		float InputTorque);

	/**
	 * @brief Simulate 1-way LSD behavior (locks under acceleration only)
	 *
	 * Popular for drift applications - allows free spin on deceleration
	 * for easy rotation initiation, locks under power for traction.
	 *
	 * @param DeltaTime Frame delta time
	 * @param OutState Differential state to update
	 * @param Config LSD configuration parameters
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 * @param InputTorque Torque input to differential
	 * @param bIsAccelerating True if under acceleration
	 */
	void Simulate1WayLSD(
		float DeltaTime,
		FMGDifferentialState& OutState,
		const FMGLSDConfiguration& Config,
		int32 LeftWheelIdx,
		int32 RightWheelIdx,
		float InputTorque,
		bool bIsAccelerating);

	/**
	 * @brief Simulate 1.5-way LSD behavior (full accel lock, partial decel)
	 *
	 * Compromise between 1-way and 2-way - good balance of drift
	 * initiation and stability under braking.
	 *
	 * @param DeltaTime Frame delta time
	 * @param OutState Differential state to update
	 * @param Config LSD configuration parameters
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 * @param InputTorque Torque input to differential
	 * @param bIsAccelerating True if under acceleration
	 */
	void Simulate1Point5WayLSD(
		float DeltaTime,
		FMGDifferentialState& OutState,
		const FMGLSDConfiguration& Config,
		int32 LeftWheelIdx,
		int32 RightWheelIdx,
		float InputTorque,
		bool bIsAccelerating);

	/**
	 * @brief Simulate 2-way LSD behavior (equal lock both directions)
	 *
	 * Most aggressive LSD - locks equally under accel and decel.
	 * Provides maximum stability but can cause push on corner entry.
	 *
	 * @param DeltaTime Frame delta time
	 * @param OutState Differential state to update
	 * @param Config LSD configuration parameters
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 * @param InputTorque Torque input to differential
	 * @param bIsAccelerating True if under acceleration
	 */
	void Simulate2WayLSD(
		float DeltaTime,
		FMGDifferentialState& OutState,
		const FMGLSDConfiguration& Config,
		int32 LeftWheelIdx,
		int32 RightWheelIdx,
		float InputTorque,
		bool bIsAccelerating);

	/**
	 * @brief Simulate Torsen differential behavior (torque-sensing)
	 *
	 * Gear-type differential that uses worm gears for smooth,
	 * progressive torque biasing without clutch packs.
	 *
	 * @param DeltaTime Frame delta time
	 * @param OutState Differential state to update
	 * @param Config LSD configuration parameters
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 * @param InputTorque Torque input to differential
	 */
	void SimulateTorsenDifferential(
		float DeltaTime,
		FMGDifferentialState& OutState,
		const FMGLSDConfiguration& Config,
		int32 LeftWheelIdx,
		int32 RightWheelIdx,
		float InputTorque);

	/**
	 * @brief Simulate locked/welded differential behavior
	 *
	 * Both wheels always rotate at same speed - maximum traction
	 * but poor turning behavior and tire scrub.
	 *
	 * @param DeltaTime Frame delta time
	 * @param OutState Differential state to update
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 * @param InputTorque Torque input to differential
	 */
	void SimulateLockedDifferential(
		float DeltaTime,
		FMGDifferentialState& OutState,
		int32 LeftWheelIdx,
		int32 RightWheelIdx,
		float InputTorque);

	/**
	 * @brief Calculate LSD lockup based on ramp angle and torque
	 *
	 * Uses the ramp angle formula: LockForce = InputTorque * tan(RampAngle)
	 * Lower ramp angles produce more aggressive lockup.
	 *
	 * @param InputTorque Input torque to differential
	 * @param RampAngleDeg Ramp angle in degrees
	 * @param Preload Preload torque
	 * @param MaxLock Maximum lock percentage
	 * @param FrictionCoef Clutch friction coefficient
	 * @param PlateCount Number of clutch plates
	 * @return Calculated lock percentage (0-1)
	 */
	float CalculateLSDLockup(
		float InputTorque,
		float RampAngleDeg,
		float Preload,
		float MaxLock,
		float FrictionCoef,
		int32 PlateCount) const;

	/**
	 * @brief Calculate torque distribution based on lock state and slip
	 *
	 * @param OutState Differential state (modified)
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 * @param InputTorque Total input torque
	 */
	void CalculateTorqueDistribution(
		FMGDifferentialState& OutState,
		int32 LeftWheelIdx,
		int32 RightWheelIdx,
		float InputTorque);

	/**
	 * @brief Update power distribution visualization data
	 * @param DeltaTime Frame delta time
	 */
	void UpdatePowerDistributionData(float DeltaTime);

	/**
	 * @brief Apply differential torque effects to wheels
	 *
	 * Applies calculated torque split to wheel physics.
	 *
	 * @param DeltaTime Frame delta time
	 * @param DiffState Differential state with torque ratios
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 */
	void ApplyDifferentialTorqueToWheels(
		float DeltaTime,
		const FMGDifferentialState& DiffState,
		int32 LeftWheelIdx,
		int32 RightWheelIdx);

	/**
	 * @brief Integrate differential with weight transfer system
	 *
	 * Adjusts differential behavior based on current weight distribution.
	 * More weight on a side increases that wheel's traction and
	 * affects lockup behavior.
	 *
	 * @param DeltaTime Frame delta time
	 */
	void IntegrateDifferentialWithWeightTransfer(float DeltaTime);

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

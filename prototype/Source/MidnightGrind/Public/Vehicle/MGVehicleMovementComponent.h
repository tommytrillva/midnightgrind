// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehicleMovementComponent.h
 * @brief Advanced vehicle movement component with simulation-grade physics.
 *
 * @section Overview
 * This is the core vehicle physics component for MIDNIGHT GRIND. It extends
 * Unreal's Chaos Vehicle physics with extensive additional simulation systems
 * for turbo, clutch, differential, tire pressure, weight transfer, and more.
 *
 * @section Architecture
 * The component builds on ChaosWheeledVehicleMovementComponent and adds:
 *
 * **Engine Simulation**:
 * - Torque curves from vehicle data
 * - Turbo/supercharger simulation with spool and lag
 * - Rev limiter and overrev protection
 * - Temperature and wear effects
 *
 * **Drivetrain Simulation**:
 * - Clutch with slip and wear modeling
 * - Sequential and H-pattern gearbox modes
 * - Limited-slip differential with multiple types (1-way, 1.5-way, 2-way, Torsen)
 * - AWD torque split control
 *
 * **Tire Simulation**:
 * - Pressure-based grip modeling
 * - Temperature buildup and optimal range
 * - Compound-specific characteristics
 * - Blowout mechanics
 *
 * **Handling**:
 * - Dynamic weight transfer (longitudinal and lateral)
 * - Arcade, Balanced, and Simulation handling modes
 * - Stability assists (configurable)
 * - Drift mechanics with scoring
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Chaos Vehicle Physics**: Unreal Engine 5's vehicle physics system based
 * on the Chaos physics engine. Provides base wheel simulation, suspension,
 * and basic drivetrain. We extend it significantly.
 *
 * **Torque vs Horsepower**: Torque is rotational force (how hard the engine pushes).
 * Horsepower is power (torque x RPM). High torque = strong acceleration at low RPM.
 * High HP = high top speed potential.
 *
 * **Turbo Lag/Spool**: Turbochargers need exhaust gas to spin up (spool). There's
 * a delay (lag) between pressing throttle and getting boost. This is simulated
 * realistically based on turbo size and RPM.
 *
 * **Limited-Slip Differential (LSD)**: A differential that allows some wheel speed
 * difference but limits it. When one wheel spins, the LSD transfers some torque
 * to the gripping wheel. Different types behave differently:
 * - 1-way: Only locks under acceleration
 * - 1.5-way: Partial lock under deceleration
 * - 2-way: Equal lock accel and decel
 * - Torsen: Gear-based, speed-sensitive
 *
 * **Weight Transfer**: As you brake, weight shifts forward (more front grip).
 * As you accelerate, weight shifts rearward. Cornering shifts weight to outside.
 * This affects grip at each wheel.
 *
 * **Handling Modes**: Three physics "feels" from the same simulation:
 * - Arcade: Maximum assists, forgiving, great for controllers
 * - Balanced: Moderate assists, good for most players
 * - Simulation: Minimal assists, realistic, best with wheel
 *
 * @section Usage Example Usage
 * @code
 * // Get the movement component from a vehicle pawn
 * UMGVehicleMovementComponent* Movement = Vehicle->GetMGVehicleMovement();
 *
 * // Apply input
 * Movement->SetThrottleInput(ThrottleValue);
 * Movement->SetSteeringInput(SteeringValue);
 * Movement->SetBrakeInput(BrakeValue);
 * Movement->SetHandbrakeInput(bHandbrake);
 *
 * // Activate NOS
 * Movement->ActivateNitrous();
 *
 * // Check state for HUD
 * float RPM = Movement->GetEngineRPM();
 * int32 Gear = Movement->GetCurrentGear();
 * float Boost = Movement->GetCurrentBoostPSI();
 * bool bDrifting = Movement->IsDrifting();
 *
 * // Configure handling mode
 * Movement->SetHandlingMode(EMGPhysicsHandlingMode::Simulation);
 *
 * // Bind to events
 * Movement->OnGearChanged.AddDynamic(this, &AMyClass::HandleGearChange);
 * Movement->OnDriftScoreAwarded.AddDynamic(this, &AMyClass::HandleDriftScore);
 * @endcode
 *
 * @see FMGVehicleData Vehicle configuration data applied to this component
 * @see AMGVehiclePawn The pawn that owns this component
 * @see EMGPhysicsHandlingMode Handling mode presets
 */

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "MGVehicleData.h"
#include "MGTirePressureTypes.h"
#include "MGPhysicsConstants.h"
#include "RacingWheel/MGRacingWheelTypes.h"
#include "MGVehicleMovementComponent.generated.h"

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/**
 * @brief Movement component event delegates.
 *
 * These delegates broadcast state changes for other systems to respond to.
 * Used by audio, VFX, HUD, and scoring systems.
 *
 * **DECLARE_DYNAMIC_MULTICAST_DELEGATE_...**: Creates Blueprint-compatible events.
 * The suffix indicates parameter count (OneParam, TwoParams, ThreeParams).
 */
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTirePressureWarning, int32, WheelIndex, float, CurrentPressure, EMGPressureLossCause, Cause);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTireBlowout, int32, WheelIndex, EMGPressureLossCause, Cause);

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
 * @brief Suspension geometry configuration for a single axle
 *
 * Models the effects of camber, toe, and caster angles on tire behavior.
 * These settings significantly impact handling characteristics:
 * - Camber: Affects contact patch shape during cornering
 * - Toe: Affects straight-line stability and turn-in response
 * - Caster: Affects steering feel and self-centering (front only)
 */
USTRUCT(BlueprintType)
struct FMGSuspensionGeometry
{
	GENERATED_BODY()

	/**
	 * @brief Camber angle in degrees (negative = top of tire tilted inward)
	 *
	 * Typical street car range: -0.5 to -2.0 degrees
	 * Typical race car range: -2.0 to -4.0 degrees
	 * Extreme drift setup: -4.0 to -8.0 degrees
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry", meta = (ClampMin = "-10.0", ClampMax = "5.0"))
	float CamberAngleDeg = -1.0f;

	/**
	 * @brief Toe angle in degrees per side (positive = toe-in, negative = toe-out)
	 *
	 * Toe-in (positive): Improves straight-line stability, reduces turn-in
	 * Toe-out (negative): Improves turn-in response, increases tire wear
	 * Typical range: -0.5 to +0.5 degrees per side
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry", meta = (ClampMin = "-2.0", ClampMax = "2.0"))
	float ToeAngleDeg = 0.0f;

	/**
	 * @brief Caster angle in degrees (positive = top of steering axis tilted rearward)
	 *
	 * Higher caster: Better high-speed stability, stronger self-centering, heavier steering
	 * Lower caster: Lighter steering, less self-centering, reduced stability
	 * Typical street car range: 3.0 to 7.0 degrees
	 * Typical race car range: 5.0 to 10.0 degrees
	 *
	 * Note: Only affects front wheels (steering axis)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry", meta = (ClampMin = "0.0", ClampMax = "15.0"))
	float CasterAngleDeg = 5.0f;
};

/**
 * @brief Calculated tire contact patch effects from suspension geometry
 *
 * These values are computed from suspension geometry and vehicle state,
 * and are used to modify grip and handling characteristics per wheel.
 */
USTRUCT(BlueprintType)
struct FMGContactPatchState
{
	GENERATED_BODY()

	/**
	 * @brief Effective contact patch width ratio (0.5 to 1.2)
	 *
	 * Affected by camber: negative camber reduces contact patch width on straights
	 * but increases it during cornering when body roll compresses the outside tire.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float EffectiveWidthRatio = 1.0f;

	/**
	 * @brief Lateral grip multiplier from camber effect (0.7 to 1.15)
	 *
	 * Negative camber improves lateral grip during cornering by keeping
	 * the tire more perpendicular to the road surface under body roll.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float CamberLateralGripMultiplier = 1.0f;

	/**
	 * @brief Longitudinal (acceleration/braking) grip multiplier (0.8 to 1.05)
	 *
	 * Excessive camber reduces straight-line traction due to smaller contact patch.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float CamberLongitudinalGripMultiplier = 1.0f;

	/**
	 * @brief Turn-in response multiplier from toe setting (0.8 to 1.2)
	 *
	 * Toe-out improves initial turn-in response.
	 * Toe-in reduces turn-in but improves stability.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float ToeTurnInMultiplier = 1.0f;

	/**
	 * @brief Straight-line stability multiplier from toe setting (0.8 to 1.2)
	 *
	 * Toe-in improves straight-line stability.
	 * Toe-out reduces stability but improves agility.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float ToeStabilityMultiplier = 1.0f;

	/**
	 * @brief Tire wear rate multiplier from toe setting (1.0 to 2.0)
	 *
	 * Both toe-in and toe-out increase tire wear due to scrubbing.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float ToeWearMultiplier = 1.0f;

	/**
	 * @brief Steering self-centering force multiplier from caster (0.5 to 1.5)
	 *
	 * Higher caster increases the self-centering torque.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float CasterSelfCenteringMultiplier = 1.0f;

	/**
	 * @brief High-speed stability multiplier from caster (0.8 to 1.2)
	 *
	 * Higher caster improves directional stability at speed.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float CasterStabilityMultiplier = 1.0f;

	/**
	 * @brief Steering weight/feel multiplier from caster (0.7 to 1.5)
	 *
	 * Higher caster increases steering effort but improves feedback.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float CasterSteeringWeightMultiplier = 1.0f;

	/**
	 * @brief Combined geometry grip modifier for this wheel
	 *
	 * Final multiplier applied to base tire grip, combining all geometry effects.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ContactPatch")
	float CombinedGripModifier = 1.0f;
};

/**
 * @brief Complete suspension geometry effects data for the entire vehicle
 *
 * Contains calculated contact patch states for all four wheels and
 * aggregated handling modifiers derived from suspension geometry settings.
 */
USTRUCT(BlueprintType)
struct FMGSuspensionGeometryEffects
{
	GENERATED_BODY()

	/** Contact patch state for each wheel (FL, FR, RL, RR) */
	UPROPERTY(BlueprintReadOnly, Category = "Geometry")
	FMGContactPatchState WheelContactPatch[4];

	/** Overall steering response modifier from geometry (affects turn-in speed) */
	UPROPERTY(BlueprintReadOnly, Category = "Geometry|Handling")
	float SteeringResponseModifier = 1.0f;

	/** Overall straight-line stability modifier */
	UPROPERTY(BlueprintReadOnly, Category = "Geometry|Handling")
	float StraightLineStabilityModifier = 1.0f;

	/** Overall cornering grip modifier */
	UPROPERTY(BlueprintReadOnly, Category = "Geometry|Handling")
	float CorneringGripModifier = 1.0f;

	/** Tire wear rate modifier from geometry */
	UPROPERTY(BlueprintReadOnly, Category = "Geometry|Wear")
	float TireWearRateModifier = 1.0f;

	/** Steering self-centering strength */
	UPROPERTY(BlueprintReadOnly, Category = "Geometry|Steering")
	float SelfCenteringStrength = 1.0f;

	/** Steering weight/effort modifier */
	UPROPERTY(BlueprintReadOnly, Category = "Geometry|Steering")
	float SteeringWeightModifier = 1.0f;
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
 * @brief Tire pressure state for pressure-based grip and handling effects
 *
 * Comprehensive tire pressure simulation including:
 * - Cold pressure setting and hot pressure from temperature
 * - Pressure effects on grip, wear, heat generation, and fuel economy
 * - Multiple pressure loss scenarios (punctures, slow leaks, blowouts)
 * - Contact patch size variation with pressure
 * - Optimal pressure varies by tire compound
 *
 * Physical relationships modeled:
 * - Lower pressure: More grip (larger patch), faster wear, more heat, worse economy
 * - Higher pressure: Less grip, slower wear, less heat, better economy
 * - Extremely low pressure: Grip loss from sidewall collapse
 *
 * @see EMGPressureLossCause for damage types
 * @see FMGTirePressureConfig for simulation tuning
 */
USTRUCT(BlueprintType)
struct FMGTirePressureState
{
	GENERATED_BODY()

	// ==========================================
	// PRESSURE VALUES
	// ==========================================

	/** Cold pressure setting (PSI) - what the tire is set to when cold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure", meta = (ClampMin = "20.0", ClampMax = "50.0"))
	float ColdPressurePSI = 32.0f;

	/** Current actual pressure (PSI) - changes with temperature and leaks */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure")
	float CurrentPressurePSI = 32.0f;

	/** Target/optimal hot pressure for this tire (PSI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure", meta = (ClampMin = "25.0", ClampMax = "55.0"))
	float OptimalHotPressurePSI = 36.0f;

	/** Pressure change per degree Celsius (ideal gas law coefficient) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure", meta = (ClampMin = "0.01", ClampMax = "0.2"))
	float PressurePerDegree = 0.04f; // ~0.04 PSI per degree C is realistic

	// ==========================================
	// DAMAGE STATE
	// ==========================================

	/** Whether tire has any active pressure loss */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bHasSlowLeak = false;

	/** Whether tire is flat (pressure below functional threshold) */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bIsFlat = false;

	/** Whether tire has suffered a catastrophic blowout */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bIsBlownOut = false;

	/** Current cause of pressure loss */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	EMGPressureLossCause LeakCause = EMGPressureLossCause::None;

	/** Leak rate if leaking (PSI per second) */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float LeakRatePSIPerSecond = 0.0f;

	/** Time since leak started in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float LeakDuration = 0.0f;

	/** Total pressure lost to leaks this session in PSI */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float TotalPressureLost = 0.0f;

	/** Number of damage events this tire has sustained */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	int32 DamageEventCount = 0;

	// ==========================================
	// CACHED EFFECT MULTIPLIERS
	// ==========================================

	/** Contact patch size multiplier (cached for performance) */
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float ContactPatchMultiplier = 1.0f;

	/** Heat generation multiplier (lower pressure = more heat) */
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float HeatGenerationMultiplier = 1.0f;

	/** Rolling resistance multiplier (lower pressure = more resistance) */
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float RollingResistanceMultiplier = 1.0f;

	// ==========================================
	// SIMULATION METHODS
	// ==========================================

	/**
	 * @brief Update pressure based on tire temperature
	 *
	 * Uses ideal gas law approximation: P_hot = P_cold * (1 + k * deltaT)
	 *
	 * @param TireTemp Current tire temperature in Celsius
	 * @param AmbientTemp Ambient temperature in Celsius
	 */
	void UpdatePressureFromTemperature(float TireTemp, float AmbientTemp)
	{
		if (bIsFlat || bIsBlownOut) return;

		// Pressure increases with temperature (ideal gas law approximation)
		float TempDelta = TireTemp - AmbientTemp;
		CurrentPressurePSI = ColdPressurePSI + (TempDelta * PressurePerDegree);

		// Clamp to prevent unrealistic values
		CurrentPressurePSI = FMath::Clamp(CurrentPressurePSI, 5.0f, 60.0f);

		// Update cached multipliers
		UpdateCachedEffects();
	}

	/**
	 * @brief Apply leak damage over time
	 * @param DeltaTime Frame delta time in seconds
	 */
	void ApplyLeak(float DeltaTime)
	{
		if (!bHasSlowLeak || bIsFlat || bIsBlownOut)
		{
			return;
		}

		// Track leak duration
		LeakDuration += DeltaTime;

		// Apply pressure loss
		float PressureLoss = LeakRatePSIPerSecond * DeltaTime;
		CurrentPressurePSI -= PressureLoss;
		ColdPressurePSI -= PressureLoss;
		TotalPressureLost += PressureLoss;

		// Clamp cold pressure (can't go below leak destination)
		ColdPressurePSI = FMath::Max(ColdPressurePSI, 0.0f);

		// Check if tire is now flat
		if (CurrentPressurePSI <= 5.0f)
		{
			bIsFlat = true;
			CurrentPressurePSI = FMath::Max(CurrentPressurePSI, 0.0f);
		}

		UpdateCachedEffects();
	}

	/**
	 * @brief Start a pressure leak with specified cause and severity
	 * @param Cause The cause of the leak
	 * @param LeakRate Pressure loss rate in PSI per second
	 */
	void StartLeak(EMGPressureLossCause Cause, float LeakRate)
	{
		if (bIsBlownOut) return; // Can't add leak to blown tire

		bHasSlowLeak = true;
		LeakCause = Cause;
		LeakRatePSIPerSecond = FMath::Max(LeakRatePSIPerSecond, LeakRate); // Take worst leak
		LeakDuration = 0.0f;
		DamageEventCount++;
	}

	/**
	 * @brief Cause immediate blowout
	 * @param Cause The cause of the blowout
	 */
	void Blowout(EMGPressureLossCause Cause = EMGPressureLossCause::Blowout)
	{
		bIsBlownOut = true;
		bIsFlat = true;
		bHasSlowLeak = false;
		LeakCause = Cause;
		TotalPressureLost += CurrentPressurePSI;
		CurrentPressurePSI = 0.0f;
		ColdPressurePSI = 0.0f;
		DamageEventCount++;
		UpdateCachedEffects();
	}

	/**
	 * @brief Repair the tire (pit stop)
	 * @param NewColdPressure New cold pressure setting
	 * @param OptimalPressure New optimal pressure
	 */
	void Repair(float NewColdPressure = 32.0f, float OptimalPressure = 36.0f)
	{
		ColdPressurePSI = NewColdPressure;
		CurrentPressurePSI = NewColdPressure;
		OptimalHotPressurePSI = OptimalPressure;
		bHasSlowLeak = false;
		bIsFlat = false;
		bIsBlownOut = false;
		LeakCause = EMGPressureLossCause::None;
		LeakRatePSIPerSecond = 0.0f;
		LeakDuration = 0.0f;
		TotalPressureLost = 0.0f;
		DamageEventCount = 0;
		UpdateCachedEffects();
	}

	/**
	 * @brief Update all cached effect multipliers
	 * Call after pressure changes for performance
	 */
	void UpdateCachedEffects()
	{
		ContactPatchMultiplier = CalculateContactPatchMultiplier();
		HeatGenerationMultiplier = CalculateHeatGenerationMultiplier();
		RollingResistanceMultiplier = CalculateRollingResistanceMultiplier();
	}

	// ==========================================
	// EFFECT CALCULATIONS
	// ==========================================

	/**
	 * @brief Get grip multiplier based on pressure vs optimal
	 * @return Grip multiplier (0.1 to 1.07)
	 *
	 * Grip curve:
	 * - Optimal pressure: 1.0
	 * - Slightly under-inflated (5-10%): 1.02-1.07 (larger contact patch)
	 * - Significantly under-inflated: Drops due to sidewall flex
	 * - Over-inflated: Reduces due to smaller contact patch
	 * - Flat/Blowout: Minimal grip (0.1-0.15)
	 */
	float GetGripMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 0.1f; // Severely compromised grip
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalHotPressurePSI, 1.0f);

		if (PressureRatio < 0.5f)
		{
			// Critically low - severe grip loss from sidewall collapse
			return 0.3f + (PressureRatio * 0.4f);
		}
		else if (PressureRatio < 0.85f)
		{
			// Under-inflated but functional
			const float UnderInflateRatio = (PressureRatio - 0.5f) / 0.35f;
			return FMath::Lerp(0.5f, 1.05f, UnderInflateRatio);
		}
		else if (PressureRatio <= 0.95f)
		{
			// Slightly under-inflated - sweet spot for max grip
			return 1.03f + (0.95f - PressureRatio) * 0.4f;
		}
		else if (PressureRatio <= 1.05f)
		{
			// Near optimal
			return 1.0f;
		}
		else if (PressureRatio <= 1.15f)
		{
			// Slightly over-inflated
			return 1.0f - ((PressureRatio - 1.05f) * 0.5f);
		}
		else
		{
			// Significantly over-inflated
			return FMath::Max(0.7f, 0.95f - ((PressureRatio - 1.15f) * 1.5f));
		}
	}

	/**
	 * @brief Get tire wear rate multiplier
	 * @return Wear multiplier (1.0 to 10.0)
	 *
	 * Under-inflated: Excessive shoulder wear, more heat
	 * Over-inflated: Center wear
	 * Flat: Catastrophic wear
	 */
	float GetWearRateMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 10.0f; // Catastrophic wear
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalHotPressurePSI, 1.0f);

		if (PressureRatio < 0.7f)
		{
			// Severely under-inflated - rapid wear
			return 3.0f - (PressureRatio * 2.0f);
		}
		else if (PressureRatio < 0.9f)
		{
			// Under-inflated - accelerated wear
			return 1.0f + ((0.9f - PressureRatio) * 3.0f);
		}
		else if (PressureRatio <= 1.1f)
		{
			// Optimal range
			return 1.0f;
		}
		else
		{
			// Over-inflated - moderate wear increase
			return 1.0f + ((PressureRatio - 1.1f) * 1.5f);
		}
	}

	/**
	 * @brief Calculate contact patch size multiplier
	 * @return Contact patch multiplier (0.3 to 1.4)
	 */
	float CalculateContactPatchMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 0.3f;
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalHotPressurePSI, 1.0f);

		// Inverse relationship - lower pressure = larger patch
		if (PressureRatio < 0.5f)
		{
			return 0.8f; // Collapsed sidewall
		}

		return FMath::Clamp(1.5f - (PressureRatio * 0.5f), 0.7f, 1.4f);
	}

	/**
	 * @brief Get contact patch multiplier (cached value)
	 * @return Contact patch multiplier
	 */
	float GetContactPatchMultiplier() const
	{
		return ContactPatchMultiplier;
	}

	/**
	 * @brief Calculate heat generation multiplier
	 * @return Heat multiplier (1.0 to 3.0)
	 */
	float CalculateHeatGenerationMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 3.0f;
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalHotPressurePSI, 1.0f);

		if (PressureRatio < 0.7f)
		{
			return 2.5f - (PressureRatio * 1.5f);
		}
		else if (PressureRatio < 0.9f)
		{
			return 1.0f + ((0.9f - PressureRatio) * 2.5f);
		}
		else if (PressureRatio <= 1.1f)
		{
			return 1.0f;
		}
		else
		{
			return FMath::Max(0.85f, 1.0f - ((PressureRatio - 1.1f) * 0.5f));
		}
	}

	/**
	 * @brief Calculate rolling resistance multiplier
	 * @return Rolling resistance multiplier (0.95 to 3.0)
	 */
	float CalculateRollingResistanceMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 3.0f;
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalHotPressurePSI, 1.0f);

		if (PressureRatio < 0.85f)
		{
			return 1.0f + ((0.85f - PressureRatio) * 2.0f);
		}
		else if (PressureRatio <= 1.1f)
		{
			return 1.0f;
		}
		else
		{
			return FMath::Max(0.95f, 1.0f - ((PressureRatio - 1.1f) * 0.3f));
		}
	}

	/**
	 * @brief Get fuel economy multiplier (inverse of rolling resistance)
	 * @return Fuel economy multiplier
	 */
	float GetFuelEconomyMultiplier() const
	{
		return 1.0f / FMath::Max(RollingResistanceMultiplier, 0.1f);
	}

	// ==========================================
	// STATUS QUERIES
	// ==========================================

	/**
	 * @brief Check if tire is under-inflated
	 * @return True if significantly under-inflated (>15% below optimal)
	 */
	bool IsUnderInflated() const
	{
		return CurrentPressurePSI < (OptimalHotPressurePSI * 0.85f);
	}

	/**
	 * @brief Check if tire is over-inflated
	 * @return True if significantly over-inflated (>15% above optimal)
	 */
	bool IsOverInflated() const
	{
		return CurrentPressurePSI > (OptimalHotPressurePSI * 1.15f);
	}

	/**
	 * @brief Check if pressure needs driver attention
	 * @return True if pressure is off by more than 10%
	 */
	bool NeedsAttention() const
	{
		if (bIsFlat || bIsBlownOut || bHasSlowLeak)
		{
			return true;
		}

		const float DeviationPercent = FMath::Abs(CurrentPressurePSI - OptimalHotPressurePSI) / FMath::Max(OptimalHotPressurePSI, 1.0f) * 100.0f;
		return DeviationPercent > 10.0f;
	}

	/**
	 * @brief Check if tire is in critical state
	 * @return True if flat, blown out, or rapidly losing pressure
	 */
	bool IsCritical() const
	{
		return bIsFlat || bIsBlownOut || CurrentPressurePSI < 15.0f || LeakRatePSIPerSecond > 1.0f;
	}

	/**
	 * @brief Get pressure as percentage of optimal
	 * @return Pressure percentage (0-150+)
	 */
	float GetPressurePercent() const
	{
		return (CurrentPressurePSI / FMath::Max(OptimalHotPressurePSI, 1.0f)) * 100.0f;
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
	 *
	 * Calculates dynamic wheel load based on weight transfer state.
	 * See MGPhysicsConstants.h for detailed documentation of the physics constants.
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Load multiplier for the specified wheel (typically 0.3 to 1.8)
	 */
	float GetWheelLoadMultiplier(int32 WheelIndex) const
	{
		using namespace MGPhysicsConstants::WeightTransfer;

		float BaseLoad = 1.0f;
		const bool bFront = WheelIndex < 2;
		const bool bRight = (WheelIndex == 1 || WheelIndex == 3);

		// Longitudinal transfer (front-rear)
		// Under braking: front wheels gain load, rear wheels lose load
		// Under acceleration: rear wheels gain load, front wheels lose load
		if (bFront) BaseLoad += LongitudinalTransfer * LONGITUDINAL_RATIO;
		else BaseLoad -= LongitudinalTransfer * LONGITUDINAL_RATIO;

		// Lateral transfer (left-right)
		// In right turn: left wheels gain load, right wheels lose load
		if (bRight) BaseLoad += LateralTransfer * LATERAL_RATIO;
		else BaseLoad -= LateralTransfer * LATERAL_RATIO;

		return FMath::Clamp(BaseLoad, LOAD_MIN, LOAD_MAX);
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
	// PHYSICS HANDLING MODE
	// ==========================================

	/**
	 * @brief Apply physics handling mode preset
	 *
	 * Configures assists, physics simulation depth, and steering response
	 * based on the selected handling mode (Arcade/Balanced/Simulation).
	 *
	 * @param Mode The handling mode to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Handling")
	void ApplyPhysicsHandlingMode(EMGPhysicsHandlingMode Mode);

	/**
	 * @brief Apply custom handling settings
	 *
	 * For fine-grained control over handling parameters.
	 *
	 * @param Settings Custom handling configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Handling")
	void ApplyPhysicsHandlingSettings(const FMGPhysicsHandlingSettings& Settings);

	/**
	 * @brief Get current physics handling mode
	 * @return Current handling mode
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Handling")
	EMGPhysicsHandlingMode GetPhysicsHandlingMode() const { return CurrentHandlingMode; }

	/**
	 * @brief Get current handling settings
	 * @return Current handling configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Handling")
	const FMGPhysicsHandlingSettings& GetPhysicsHandlingSettings() const { return CurrentHandlingSettings; }

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
	// ECU MAP CONTROLS
	// ==========================================

	/** Switch to a different ECU map */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|ECU")
	bool SwitchECUMap(EMGECUMapType NewMapType);

	/** Get the currently active ECU map type */
	UFUNCTION(BlueprintPure, Category = "Vehicle|ECU")
	EMGECUMapType GetActiveECUMapType() const;

	/** Get the current ECU map parameters */
	UFUNCTION(BlueprintPure, Category = "Vehicle|ECU")
	FMGECUMapParameters GetActiveECUMapParameters() const;

	/** Check if a specific ECU map is available */
	UFUNCTION(BlueprintPure, Category = "Vehicle|ECU")
	bool IsECUMapAvailable(EMGECUMapType MapType) const;

	/** Get all available ECU maps */
	UFUNCTION(BlueprintPure, Category = "Vehicle|ECU")
	TArray<EMGECUMapType> GetAvailableECUMaps() const;

	/** Get the power multiplier from current ECU map */
	UFUNCTION(BlueprintPure, Category = "Vehicle|ECU")
	float GetECUPowerMultiplier() const;

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

	/**
	 * @brief Set engine power multiplier from damage
	 * @param Multiplier Power output multiplier (0.25-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void SetEngineDamageMultiplier(float Multiplier);

	/** Get engine damage multiplier */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	float GetEngineDamageMultiplier() const { return EngineDamageMultiplier; }

	/**
	 * @brief Set transmission efficiency from damage
	 * @param Multiplier Transmission efficiency (0.25-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void SetTransmissionDamageMultiplier(float Multiplier);

	/** Get transmission damage multiplier */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	float GetTransmissionDamageMultiplier() const { return TransmissionDamageMultiplier; }

	/**
	 * @brief Set steering response from damage
	 * @param Multiplier Steering response (0.25-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void SetSteeringDamageMultiplier(float Multiplier);

	/** Get steering damage multiplier */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	float GetSteeringDamageMultiplier() const { return SteeringDamageMultiplier; }

	/**
	 * @brief Set brake effectiveness from damage
	 * @param Multiplier Brake power (0.25-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void SetBrakeDamageMultiplier(float Multiplier);

	/** Get brake damage multiplier */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	float GetBrakeDamageMultiplier() const { return BrakeDamageMultiplier; }

	/**
	 * @brief Set suspension handling from damage
	 * @param Multiplier Handling response (0.25-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void SetSuspensionDamageMultiplier(float Multiplier);

	/** Get suspension damage multiplier */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	float GetSuspensionDamageMultiplier() const { return SuspensionDamageMultiplier; }

	/**
	 * @brief Check if engine is misfiring due to damage
	 * @return True if engine is sputtering/misfiring
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	bool IsEngineMisfiring() const { return EngineDamageMultiplier < 0.7f; }

	/**
	 * @brief Check if vehicle is severely damaged and limping
	 * @return True if multiple systems are critically damaged
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	bool IsLimping() const;

	// ==========================================
	// FUEL SYSTEM INTEGRATION
	// ==========================================

	/**
	 * @brief Set fuel starvation power multiplier
	 *
	 * Called by UMGFuelConsumptionComponent when fuel starvation occurs
	 * due to low fuel and high lateral G-forces.
	 *
	 * @param Multiplier Power multiplier (0.0-1.0, where 0 = no fuel delivery)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Fuel")
	void SetFuelStarvationMultiplier(float Multiplier);

	/**
	 * @brief Get current fuel starvation power multiplier
	 * @return Fuel starvation power modifier (1.0 = normal, <1.0 = starving)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Fuel")
	float GetFuelStarvationMultiplier() const { return FuelStarvationMultiplier; }

	/**
	 * @brief Check if vehicle is experiencing fuel starvation
	 * @return True if fuel delivery is reduced
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Fuel")
	bool IsFuelStarving() const { return FuelStarvationMultiplier < 0.99f; }

	/**
	 * @brief Set the vehicle's current fuel weight for mass calculations
	 *
	 * Fuel weight affects vehicle handling, acceleration, and braking.
	 * Called by UMGFuelConsumptionComponent as fuel is consumed.
	 *
	 * @param WeightKg Current fuel weight in kilograms
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Fuel")
	void SetCurrentFuelWeightKg(float WeightKg);

	/**
	 * @brief Get current fuel weight
	 * @return Fuel weight in kilograms
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Fuel")
	float GetCurrentFuelWeightKg() const { return CurrentFuelWeightKg; }

	// ==========================================
	// WEATHER EFFECTS INTEGRATION
	// ==========================================

	/**
	 * @brief Set weather-based grip multiplier
	 * @param Multiplier Grip multiplier from weather conditions (0.1-1.0)
	 * @note This is separate from damage grip multiplier and stacks multiplicatively
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Weather")
	void SetWeatherGripMultiplier(float Multiplier);

	/**
	 * @brief Get current weather grip multiplier
	 * @return Weather-based grip modifier
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Weather")
	float GetWeatherGripMultiplier() const { return WeatherGripMultiplier; }

	/**
	 * @brief Apply aquaplaning effect to vehicle
	 * @param Intensity Aquaplaning intensity (0-1)
	 * @param WheelFactors Per-wheel aquaplaning factors (FL, FR, RL, RR)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Weather")
	void ApplyAquaplaning(float Intensity, const TArray<float>& WheelFactors);

	/**
	 * @brief Apply wind force to vehicle
	 * @param WindForce Wind force vector in world space (Newtons)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Weather")
	void ApplyWindForce(const FVector& WindForce);

	/**
	 * @brief Check if vehicle is currently aquaplaning
	 * @return True if aquaplaning
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Weather")
	bool IsAquaplaning() const { return bIsAquaplaning; }

	/**
	 * @brief Get current aquaplaning intensity
	 * @return Aquaplaning intensity (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Weather")
	float GetAquaplaningIntensity() const { return AquaplaningIntensity; }

	/**
	 * @brief Get wheel locations for weather system puddle checks
	 * @return Array of wheel world locations (FL, FR, RL, RR)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Weather")
	TArray<FVector> GetWheelWorldLocations() const;

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

	/** Get brake temperature (Celsius) */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	float GetBrakeTemperature() const { return BrakeTemperature; }

	/**
	 * Get normalized brake glow intensity (0-1).
	 * 0 = cold brakes, 1 = glowing hot.
	 * Based on temp range ~100C (cold) to ~700C (glowing).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	float GetBrakeGlowIntensity() const
	{
		const float ColdTemp = 100.0f;
		const float GlowTemp = 700.0f;
		return FMath::Clamp((BrakeTemperature - ColdTemp) / (GlowTemp - ColdTemp), 0.0f, 1.0f);
	}

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

	/**
	 * Get cached speed in MPH (updated once per tick).
	 * Use this for frequent access within the same frame to avoid recalculation.
	 * For single-shot queries, use GetSpeedMPH() instead.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FORCEINLINE float GetCachedSpeedMPH() const { return CachedSpeedMPH; }

	/**
	 * Check if vehicle is currently moving (speed > 1 MPH).
	 * Cached value updated each tick.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FORCEINLINE bool IsVehicleMoving() const { return bIsVehicleMoving; }

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
	// FORCE FEEDBACK DATA
	// ==========================================

	/**
	 * @brief Get physics data for force feedback calculation
	 *
	 * Provides all vehicle physics state needed by the racing wheel FFB system
	 * to calculate appropriate force feedback effects.
	 *
	 * @return FFB input data structure with current vehicle state
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|FFB")
	FMGFFBInputData GetFFBInputData() const;

	/**
	 * @brief Check if vehicle is currently understeering
	 * @return True if front tires are slipping more than rear
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|FFB")
	bool IsUndersteering() const;

	/**
	 * @brief Check if vehicle is currently oversteering
	 * @return True if rear tires are slipping more than front
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|FFB")
	bool IsOversteering() const;

	/**
	 * @brief Get current drift angle in degrees
	 * @return Angle between velocity and heading
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|FFB")
	float GetDriftAngle() const;

	/**
	 * @brief Get the lateral G-force currently experienced
	 * @return Lateral acceleration in G
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|FFB")
	float GetLateralG() const;

	/**
	 * @brief Get the longitudinal G-force currently experienced
	 * @return Longitudinal acceleration in G (positive = acceleration)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|FFB")
	float GetLongitudinalG() const;

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

	// ==========================================
	// SUSPENSION GEOMETRY STATE QUERIES
	// ==========================================

	/**
	 * @brief Get current suspension geometry effects for all wheels
	 * @return Complete geometry effects including per-wheel contact patch states
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|SuspensionGeometry")
	FMGSuspensionGeometryEffects GetSuspensionGeometryEffects() const { return SuspensionGeometryEffects; }

	/**
	 * @brief Get contact patch state for a specific wheel
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Contact patch state for the specified wheel
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|SuspensionGeometry")
	FMGContactPatchState GetWheelContactPatchState(int32 WheelIndex) const;

	/**
	 * @brief Get the effective camber angle for a wheel considering body roll
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Effective camber angle in degrees (includes dynamic camber from roll)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|SuspensionGeometry")
	float GetEffectiveCamberAngle(int32 WheelIndex) const;

	/**
	 * @brief Get the current steering self-centering force multiplier
	 * @return Self-centering force multiplier based on caster and speed
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|SuspensionGeometry")
	float GetSteeringSelfCenteringForce() const;

	/**
	 * @brief Get geometry-based grip modifier for a specific wheel
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param bForLateralGrip True for cornering grip, false for longitudinal
	 * @return Grip multiplier from suspension geometry
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|SuspensionGeometry")
	float GetGeometryGripModifier(int32 WheelIndex, bool bForLateralGrip) const;

	/**
	 * @brief Sample power curve at specific RPM
	 * @param RPM Engine RPM to sample
	 * @param OutHorsepower Output horsepower at this RPM
	 * @param OutTorque Output torque at this RPM
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Dyno")
	void SamplePowerCurve(float RPM, float& OutHorsepower, float& OutTorque) const;

	// ==========================================
	// TIRE PRESSURE STATE QUERIES
	// ==========================================

	/**
	 * @brief Get tire pressure state for a specific wheel
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Current tire pressure state including all effects
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|TirePressure")
	FMGTirePressureState GetTirePressureState(int32 WheelIndex) const;

	/**
	 * @brief Get current tire pressure in PSI for a wheel
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Current pressure in PSI
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|TirePressure")
	float GetTirePressurePSI(int32 WheelIndex) const;

	/**
	 * @brief Get optimal tire pressure for current compound
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Optimal pressure in PSI for current compound
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|TirePressure")
	float GetOptimalTirePressurePSI(int32 WheelIndex) const;

	/**
	 * @brief Check if any tire has a pressure warning
	 * @return True if any tire needs attention
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|TirePressure")
	bool HasTirePressureWarning() const;

	/**
	 * @brief Check if any tire is flat or blown out
	 * @return True if any tire is critically damaged
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|TirePressure")
	bool HasFlatTire() const;

	/**
	 * @brief Get average tire pressure across all wheels
	 * @return Average pressure in PSI
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|TirePressure")
	float GetAverageTirePressure() const;

	/**
	 * @brief Get total rolling resistance from tire pressure effects
	 * @return Combined rolling resistance multiplier from all tires
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|TirePressure")
	float GetTotalRollingResistanceFromPressure() const;

	// ==========================================
	// TIRE PRESSURE DAMAGE API
	// ==========================================

	/**
	 * @brief Apply puncture damage to a specific tire
	 *
	 * Simulates tire damage from various sources:
	 * - Spike strips (rapid deflation)
	 * - Road debris (slow leak)
	 * - Collision damage (moderate leak)
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param Cause The cause of the puncture
	 * @param Severity Damage severity (0-1, affects leak rate)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|TirePressure")
	void ApplyTirePuncture(int32 WheelIndex, EMGPressureLossCause Cause, float Severity = 1.0f);

	/**
	 * @brief Apply spike strip damage to tires
	 *
	 * Applies rapid pressure loss to specified tires, simulating
	 * driving over a police spike strip.
	 *
	 * @param AffectedWheels Array of wheel indices to damage (0=FL, 1=FR, 2=RL, 3=RR)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|TirePressure")
	void ApplySpikeStripDamage(const TArray<int32>& AffectedWheels);

	/**
	 * @brief Cause an immediate tire blowout
	 *
	 * Catastrophic tire failure - instant pressure loss and
	 * severe handling degradation.
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param Cause The cause of the blowout
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|TirePressure")
	void CauseTireBlowout(int32 WheelIndex, EMGPressureLossCause Cause = EMGPressureLossCause::Blowout);

	/**
	 * @brief Set tire cold pressure (pit stop adjustment)
	 *
	 * Adjusts the baseline pressure for a tire. Used during pit stops
	 * or garage tuning sessions.
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param PressurePSI New cold pressure setting in PSI
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|TirePressure")
	void SetTireColdPressure(int32 WheelIndex, float PressurePSI);

	/**
	 * @brief Set cold pressure for all tires
	 * @param FrontPressurePSI Front tires pressure
	 * @param RearPressurePSI Rear tires pressure
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|TirePressure")
	void SetAllTiresColdPressure(float FrontPressurePSI, float RearPressurePSI);

	/**
	 * @brief Repair/replace a damaged tire
	 *
	 * Restores tire to optimal condition. Used in pit stops
	 * or garage repairs.
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|TirePressure")
	void RepairTire(int32 WheelIndex);

	/**
	 * @brief Repair all tires
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|TirePressure")
	void RepairAllTires();

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

	/** Engine braking strength multiplier (how much engine slows car when off throttle) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Drivetrain", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float EngineBrakingMultiplier = 1.0f;

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
	// TUNING PARAMETERS - TIRE PRESSURE
	// ==========================================

	/**
	 * @brief Tire pressure simulation configuration
	 *
	 * Contains all tunable parameters for tire pressure physics including:
	 * - Pressure ranges and thresholds
	 * - Temperature-pressure relationship
	 * - Leak rates for various damage types
	 * - Compound-specific optimal pressures
	 * - Blowout conditions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|TirePressure")
	FMGTirePressureConfig TirePressureConfig;

	// ==========================================
	// TUNING PARAMETERS - SUSPENSION GEOMETRY
	// ==========================================

	/**
	 * @brief Front axle suspension geometry settings
	 *
	 * Controls camber, toe, and caster for front wheels.
	 * Caster only affects front wheels as they are steered.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|SuspensionGeometry")
	FMGSuspensionGeometry FrontSuspensionGeometry;

	/**
	 * @brief Rear axle suspension geometry settings
	 *
	 * Controls camber and toe for rear wheels.
	 * Caster has no effect on rear wheels.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|SuspensionGeometry")
	FMGSuspensionGeometry RearSuspensionGeometry;

	/**
	 * @brief Enable dynamic camber calculation based on body roll
	 *
	 * When enabled, camber angle changes dynamically based on
	 * suspension compression and body roll during cornering.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|SuspensionGeometry")
	bool bEnableDynamicCamber = true;

	/**
	 * @brief Camber gain per degree of body roll
	 *
	 * How much the effective camber changes per degree of chassis roll.
	 * Typical range: 0.3 to 0.8 degrees camber per degree roll.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|SuspensionGeometry", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float CamberGainPerDegreeRoll = 0.5f;

	/**
	 * @brief Reference body roll angle for full geometry effects
	 *
	 * The body roll angle at which geometry effects reach their
	 * maximum effectiveness during cornering.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|SuspensionGeometry", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float ReferenceBodyRollDeg = 3.0f;

	/**
	 * @brief Base caster trail in centimeters
	 *
	 * The mechanical trail created by caster angle, affects
	 * steering self-centering force. Higher values = stronger centering.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|SuspensionGeometry", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float CasterTrailCm = 6.0f;

	/**
	 * @brief Suspension geometry influence on grip (0 = disabled, 1 = full effect)
	 *
	 * Master multiplier for how much suspension geometry affects
	 * overall tire grip calculations.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|SuspensionGeometry", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SuspensionGeometryInfluence = 1.0f;

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

	/** Called when a tire pressure warning threshold is reached */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnTirePressureWarning OnTirePressureWarning;

	/** Called when a tire blows out */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnTireBlowout OnTireBlowout;

protected:
	// ==========================================
	// INTERNAL STATE
	// ==========================================

	UPROPERTY()
	FMGVehicleData CurrentConfiguration;

	/** Current physics handling mode */
	UPROPERTY()
	EMGPhysicsHandlingMode CurrentHandlingMode = EMGPhysicsHandlingMode::Balanced;

	/** Current physics handling settings */
	UPROPERTY()
	FMGPhysicsHandlingSettings CurrentHandlingSettings;

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

	UPROPERTY()
	float EngineDamageMultiplier = 1.0f;

	UPROPERTY()
	float TransmissionDamageMultiplier = 1.0f;

	UPROPERTY()
	float SteeringDamageMultiplier = 1.0f;

	UPROPERTY()
	float BrakeDamageMultiplier = 1.0f;

	UPROPERTY()
	float SuspensionDamageMultiplier = 1.0f;

	// Fuel system state
	/** Power multiplier from fuel starvation (1.0 = normal, 0.0 = no fuel) */
	UPROPERTY()
	float FuelStarvationMultiplier = 1.0f;

	/** Current fuel weight in kilograms for mass calculations */
	UPROPERTY()
	float CurrentFuelWeightKg = 0.0f;

	/** Initial vehicle mass without fuel (cached on configuration) */
	UPROPERTY()
	float BaseMassKg = 0.0f;

	// Weather effect multipliers
	UPROPERTY()
	float WeatherGripMultiplier = 1.0f;

	UPROPERTY()
	bool bIsAquaplaning = false;

	UPROPERTY()
	float AquaplaningIntensity = 0.0f;

	UPROPERTY()
	TArray<float> WheelAquaplaningFactors;

	UPROPERTY()
	FVector PendingWindForce = FVector::ZeroVector;

	// Tire temperature per wheel (FL, FR, RL, RR)
	UPROPERTY()
	FMGTireTemperature TireTemperatures[4];

	/** Surface state per wheel (FL, FR, RL, RR) for dynamic grip calculation */
	UPROPERTY()
	FMGWheelSurfaceState WheelSurfaceStates[4];

	/** Tire pressure state per wheel (FL, FR, RL, RR) */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|Tires")
	FMGTirePressureState TirePressures[4];

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
	// TICK OPTIMIZATION
	// ==========================================

	/** Frame counter for update frequency management */
	int32 TickFrameCounter = 0;

	/** Cached speed value updated once per tick (MPH) */
	float CachedSpeedMPH = 0.0f;

	/** Whether vehicle is moving (speed > threshold) */
	bool bIsVehicleMoving = false;

	/** Cached grounded state (updated every few frames) */
	bool bCachedIsGrounded = true;

	/** Frame interval for non-critical updates (wear, temperature) */
	static constexpr int32 SlowUpdateInterval = 5;

	/** Frame interval for physics-related but non-essential updates */
	static constexpr int32 MediumUpdateInterval = 2;

	// ==========================================
	// SUSPENSION GEOMETRY STATE
	// ==========================================

	/** Current suspension geometry effects for all wheels */
	UPROPERTY()
	FMGSuspensionGeometryEffects SuspensionGeometryEffects;

	/** Current body roll angle in degrees (positive = rolling right) */
	float CurrentBodyRollDeg = 0.0f;

	/** Effective camber angles per wheel including dynamic camber from roll */
	float EffectiveCamberAngles[4] = { -1.0f, -1.0f, -1.0f, -1.0f };

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

	/**
	 * @brief Update tire pressure simulation for all wheels
	 *
	 * Handles:
	 * - Temperature-pressure relationship (ideal gas law)
	 * - Natural pressure loss over time
	 * - Active leak simulation (punctures, damage)
	 * - Blowout risk calculation
	 * - Effect multiplier updates
	 *
	 * @param DeltaTime Frame delta time in seconds
	 */
	virtual void UpdateTirePressure(float DeltaTime);

	/**
	 * @brief Calculate hot pressure from cold pressure and temperature
	 *
	 * Uses ideal gas law approximation: P_hot = P_cold * (1 + k * deltaT)
	 * where k is the pressure coefficient per degree Celsius.
	 *
	 * @param ColdPressure Cold tire pressure in PSI
	 * @param TireTemp Current tire temperature in Celsius
	 * @return Hot pressure in PSI
	 */
	float CalculateHotPressure(float ColdPressure, float TireTemp) const;

	/**
	 * @brief Get optimal pressure for a tire compound
	 * @param Compound The tire compound type
	 * @return Optimal pressure in PSI for the compound
	 */
	float GetOptimalPressureForCompound(EMGTireCompound Compound) const;

	/**
	 * @brief Check and apply blowout risk for a tire
	 *
	 * Evaluates blowout conditions based on:
	 * - Temperature exceeding threshold
	 * - Pressure below critical ratio
	 * - Vehicle speed
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param DeltaTime Frame delta time for probability calculation
	 * @return True if a blowout occurred
	 */
	bool CheckAndApplyBlowoutRisk(int32 WheelIndex, float DeltaTime);

	/**
	 * @brief Apply leak to a tire based on leak cause
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param Cause The cause of the leak
	 * @param Severity Severity multiplier (0-1)
	 */
	void ApplyLeak(int32 WheelIndex, EMGPressureLossCause Cause, float Severity);

	/**
	 * @brief Initialize tire pressure states based on current compound
	 */
	void InitializeTirePressures();

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
	 * @brief Route to appropriate differential simulation based on type
	 * @param DeltaTime Frame delta time
	 * @param OutState Differential state to update
	 * @param DiffType Type of differential to simulate
	 * @param Config LSD configuration parameters
	 * @param LeftWheelIdx Left wheel index
	 * @param RightWheelIdx Right wheel index
	 * @param InputTorque Input torque to differential
	 * @param bIsAccelerating True if under acceleration
	 */
	void SimulateDifferentialByType(
		float DeltaTime,
		FMGDifferentialState& OutState,
		EMGDifferentialType DiffType,
		const FMGLSDConfiguration& Config,
		int32 LeftWheelIdx,
		int32 RightWheelIdx,
		float InputTorque,
		bool bIsAccelerating);

	/**
	 * @brief Simulate center differential for AWD vehicles
	 *
	 * Distributes torque between front and rear axles based on
	 * center LSD configuration and grip conditions.
	 *
	 * @param DeltaTime Frame delta time
	 * @param OutState Center differential state to update
	 * @param InputTorque Input torque from transmission
	 * @param bIsAccelerating True if under acceleration
	 */
	void SimulateCenterDifferential(
		float DeltaTime,
		FMGDifferentialState& OutState,
		float InputTorque,
		bool bIsAccelerating);

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

	// ==========================================
	// SUSPENSION GEOMETRY METHODS
	// ==========================================

	/**
	 * @brief Update suspension geometry effects based on current vehicle state
	 *
	 * Calculates contact patch modifications, grip multipliers, and steering
	 * characteristics based on camber, toe, and caster settings.
	 *
	 * @param DeltaTime Frame delta time
	 */
	virtual void UpdateSuspensionGeometry(float DeltaTime);

	/**
	 * @brief Calculate contact patch state for a specific wheel
	 *
	 * Computes the effective contact patch width and grip multipliers
	 * based on static geometry settings and dynamic body roll.
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param OutContactPatch Output contact patch state
	 */
	void CalculateContactPatchState(int32 WheelIndex, FMGContactPatchState& OutContactPatch) const;

	/**
	 * @brief Calculate camber effects on tire grip
	 *
	 * Negative camber improves cornering grip but reduces straight-line traction.
	 * Effects are modulated by body roll to simulate real suspension behavior.
	 *
	 * @param CamberAngleDeg Static camber angle in degrees
	 * @param BodyRollEffect Contribution from body roll (positive = outside wheel)
	 * @param OutLateralGrip Output lateral grip multiplier
	 * @param OutLongitudinalGrip Output longitudinal grip multiplier
	 * @param OutContactPatchWidth Output contact patch width ratio
	 */
	void CalculateCamberEffects(
		float CamberAngleDeg,
		float BodyRollEffect,
		float& OutLateralGrip,
		float& OutLongitudinalGrip,
		float& OutContactPatchWidth) const;

	/**
	 * @brief Calculate toe effects on handling
	 *
	 * Toe-out improves turn-in but increases tire wear.
	 * Toe-in improves stability but reduces agility.
	 *
	 * @param ToeAngleDeg Toe angle in degrees (positive = toe-in)
	 * @param OutTurnInResponse Output turn-in response multiplier
	 * @param OutStability Output straight-line stability multiplier
	 * @param OutTireWearRate Output tire wear rate multiplier
	 */
	void CalculateToeEffects(
		float ToeAngleDeg,
		float& OutTurnInResponse,
		float& OutStability,
		float& OutTireWearRate) const;

	/**
	 * @brief Calculate caster effects on steering
	 *
	 * Higher caster improves stability and self-centering but
	 * increases steering effort.
	 *
	 * @param CasterAngleDeg Caster angle in degrees
	 * @param VehicleSpeedMPH Current vehicle speed in MPH
	 * @param OutSelfCentering Output self-centering force multiplier
	 * @param OutStability Output high-speed stability multiplier
	 * @param OutSteeringWeight Output steering weight multiplier
	 */
	void CalculateCasterEffects(
		float CasterAngleDeg,
		float VehicleSpeedMPH,
		float& OutSelfCentering,
		float& OutStability,
		float& OutSteeringWeight) const;

	/**
	 * @brief Calculate body roll angle from lateral acceleration
	 *
	 * Estimates current body roll based on lateral G-forces and
	 * suspension characteristics.
	 *
	 * @param LateralAcceleration Lateral acceleration in G
	 * @return Body roll angle in degrees
	 */
	float CalculateBodyRollAngle(float LateralAcceleration) const;

	/**
	 * @brief Calculate dynamic camber change from suspension compression
	 *
	 * Models how camber changes as suspension compresses/extends,
	 * which is crucial for realistic cornering grip simulation.
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param SuspensionCompressionRatio Current compression ratio (0-1)
	 * @return Additional camber angle in degrees from suspension travel
	 */
	float CalculateDynamicCamberChange(int32 WheelIndex, float SuspensionCompressionRatio) const;

	/**
	 * @brief Apply steering self-centering force based on caster
	 *
	 * Applies a restoring torque to the steering based on caster angle
	 * and vehicle speed, simulating real steering feel.
	 *
	 * @param DeltaTime Frame delta time
	 */
	void ApplySteeringSelfCentering(float DeltaTime);

	/**
	 * @brief Calculate combined geometry grip modifier for tire friction
	 *
	 * Combines all geometry effects (camber, toe, caster) into a single
	 * grip multiplier for use in the tire friction calculation.
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @param bIsCornering True if vehicle is cornering (uses lateral grip)
	 * @return Combined grip modifier (typically 0.8 to 1.15)
	 */
	float CalculateCombinedGeometryGripModifier(int32 WheelIndex, bool bIsCornering) const;

	/**
	 * @brief Get geometry configuration for a specific wheel
	 *
	 * Returns the appropriate front or rear geometry based on wheel index.
	 *
	 * @param WheelIndex Wheel index (0=FL, 1=FR, 2=RL, 3=RR)
	 * @return Reference to the appropriate geometry configuration
	 */
	const FMGSuspensionGeometry& GetWheelGeometry(int32 WheelIndex) const;
};

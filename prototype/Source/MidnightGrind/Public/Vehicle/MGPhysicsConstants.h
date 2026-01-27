// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPhysicsConstants.h
 * @brief Physics constants and handling mode presets for vehicle simulation
 *
 * This file documents all physics "magic numbers" used throughout the vehicle
 * simulation and provides designer-friendly handling presets that control
 * the overall physics feel separate from per-vehicle tuning.
 *
 * Handling Mode vs Driving Style:
 * - Handling Mode (here): Overall physics behavior (Arcade vs Simulation)
 * - Driving Style (MGDynoTuningSubsystem): Per-vehicle tuning for race type
 */

#pragma once

#include "CoreMinimal.h"
#include "MGPhysicsConstants.generated.h"

// ============================================================================
// PHYSICS HANDLING MODE
// ============================================================================

/**
 * @brief Physics handling mode preset
 *
 * Controls the overall physics feel of the game, affecting assists,
 * stability, and simulation complexity. This is separate from driving
 * style presets which tune vehicles for specific race types.
 *
 * Design Note: Per GDD Pillar 5 "Unified Challenge", all handling modes
 * use the same underlying physics - Arcade mode adds assists and forgiveness,
 * not physics advantages.
 */
UENUM(BlueprintType)
enum class EMGPhysicsHandlingMode : uint8
{
	/**
	 * Forgiving, accessible physics with strong assists.
	 * Best for casual players and controller users.
	 * - High stability control
	 * - Reduced weight transfer effects
	 * - Increased base grip
	 * - Strong steering assist
	 * - Anti-flip protection
	 */
	Arcade UMETA(DisplayName = "Arcade"),

	/**
	 * Default balanced physics with moderate assists.
	 * Good for most players on controller or wheel.
	 * - Moderate stability control
	 * - Standard weight transfer
	 * - Normal grip levels
	 * - Light steering assist
	 */
	Balanced UMETA(DisplayName = "Balanced"),

	/**
	 * Realistic simulation physics with minimal assists.
	 * Designed for wheel users and enthusiasts.
	 * - No stability control
	 * - Full weight transfer simulation
	 * - Realistic grip response
	 * - No steering assist
	 * - Full tire temperature effects
	 * - Realistic turbo lag
	 */
	Simulation UMETA(DisplayName = "Simulation")
};

// ============================================================================
// PHYSICS CONSTANTS - WEIGHT TRANSFER
// ============================================================================

/**
 * @brief Weight transfer physics constants
 *
 * These constants control how vehicle weight shifts during acceleration,
 * braking, and cornering. The values are tuned for gameplay feel while
 * maintaining physical plausibility.
 */
namespace MGPhysicsConstants
{
	namespace WeightTransfer
	{
		/**
		 * @brief Longitudinal (front-rear) weight transfer ratio
		 *
		 * How much load transfers per unit of longitudinal acceleration.
		 * Higher values = more nose dive under braking, more squat under accel.
		 * Default: 0.15 (15% of load can transfer per G)
		 */
		constexpr float LONGITUDINAL_RATIO = 0.15f;

		/**
		 * @brief Lateral (left-right) weight transfer ratio
		 *
		 * How much load transfers per unit of lateral acceleration.
		 * Higher values = more body roll effect on grip.
		 * Default: 0.12 (12% of load can transfer per G)
		 */
		constexpr float LATERAL_RATIO = 0.12f;

		/**
		 * @brief Minimum wheel load multiplier
		 *
		 * Prevents wheels from becoming completely unloaded.
		 * At 0.3, even a fully unweighted wheel has 30% of normal load.
		 */
		constexpr float LOAD_MIN = 0.3f;

		/**
		 * @brief Maximum wheel load multiplier
		 *
		 * Caps maximum load on heavily weighted wheels.
		 * At 1.8, maximum load is 180% of static load.
		 */
		constexpr float LOAD_MAX = 1.8f;

		/**
		 * @brief Acceleration to weight transfer conversion factor
		 *
		 * Converts raw acceleration (cm/s^2) to normalized weight transfer.
		 * This accounts for:
		 * - Unit conversion (cm to m)
		 * - Gravity normalization
		 * - Gameplay tuning
		 *
		 * Formula: WeightTransfer = Acceleration * ACCEL_TO_TRANSFER
		 * At 981 cm/s^2 (1G), this produces ~0.1 transfer
		 */
		constexpr float ACCEL_TO_TRANSFER = 0.0001f;

		/**
		 * @brief Default weight transfer interpolation rate
		 *
		 * How quickly weight shifts occur (units/second).
		 * Higher = snappier response, lower = more gradual.
		 */
		constexpr float DEFAULT_RATE = 8.0f;
	}

	// ============================================================================
	// PHYSICS CONSTANTS - TIRE TEMPERATURE
	// ============================================================================

	namespace TireTemperature
	{
		/** Cold tire starting temperature (Celsius) */
		constexpr float AMBIENT = 25.0f;

		/** Optimal grip temperature (Celsius) */
		constexpr float OPTIMAL = 90.0f;

		/** Peak grip temperature (Celsius) */
		constexpr float PEAK = 110.0f;

		/** Overheating threshold (Celsius) */
		constexpr float OVERHEAT = 120.0f;

		/**
		 * @brief Grip multiplier at cold temps (below AMBIENT + 25)
		 *
		 * Tires have reduced grip when cold.
		 */
		constexpr float COLD_GRIP_MIN = 0.7f;

		/**
		 * @brief Grip multiplier at optimal temp
		 */
		constexpr float OPTIMAL_GRIP = 1.0f;

		/**
		 * @brief Peak grip multiplier at PEAK temperature
		 */
		constexpr float PEAK_GRIP = 1.05f;

		/**
		 * @brief Grip reduction rate when overheating
		 *
		 * Per degree above OVERHEAT, grip reduces by this factor.
		 */
		constexpr float OVERHEAT_GRIP_LOSS_PER_DEG = 0.005f;
	}

	// ============================================================================
	// PHYSICS CONSTANTS - SURFACE DETECTION
	// ============================================================================

	namespace Surface
	{
		/**
		 * @brief Physical material friction threshold for ice detection
		 *
		 * If a physical material has friction below this value,
		 * it's treated as ice/very slippery surface.
		 */
		constexpr float ICE_FRICTION_THRESHOLD = 0.3f;

		/**
		 * @brief Number of line traces for surface detection per frame
		 *
		 * More traces = more accurate but higher CPU cost.
		 * 4 traces = one per wheel.
		 */
		constexpr int32 TRACES_PER_FRAME = 4;
	}

	// ============================================================================
	// PHYSICS CONSTANTS - SUSPENSION GEOMETRY
	// ============================================================================

	namespace Geometry
	{
		/**
		 * @brief Toe angle effect on turn-in response
		 *
		 * How much each degree of toe affects steering response.
		 * Positive toe-in improves straight-line stability.
		 * Negative toe-out improves turn-in response.
		 */
		constexpr float TOE_EFFECT_FACTOR = 0.15f;

		/**
		 * @brief Camber lateral grip coefficient
		 *
		 * Each degree of negative camber adds this much lateral grip
		 * (up to optimal camber angle, typically -2 to -4 degrees).
		 */
		constexpr float CAMBER_GRIP_PER_DEG = 0.02f;

		/**
		 * @brief Optimal negative camber for maximum grip
		 *
		 * Beyond this angle, additional camber reduces contact patch.
		 */
		constexpr float OPTIMAL_CAMBER_DEG = -3.0f;
	}

	// ============================================================================
	// PHYSICS CONSTANTS - DIFFERENTIAL
	// ============================================================================

	namespace Differential
	{
		/**
		 * @brief Coast lock factor for 1.5-way LSD
		 *
		 * Ratio of decel lock vs accel lock for 1.5-way differentials.
		 * 0.0 = 1-way (no decel lock)
		 * 0.5 = 1.5-way (half decel lock)
		 * 1.0 = 2-way (equal decel lock)
		 */
		constexpr float ONE_POINT_FIVE_WAY_COAST_FACTOR = 0.4f;

		/**
		 * @brief Minimum speed differential to trigger LSD action
		 *
		 * Below this wheel speed difference (rad/s), diff acts as open.
		 */
		constexpr float MIN_SPEED_DIFF_THRESHOLD = 0.5f;
	}

	// ============================================================================
	// PHYSICS CONSTANTS - WEAR
	// ============================================================================

	namespace Wear
	{
		/**
		 * @brief Suspension wear to damping degradation factor
		 *
		 * As suspension wears, damping effectiveness reduces.
		 * At 100% wear, damping is reduced by this factor.
		 */
		constexpr float SUSPENSION_DAMPING_DEGRADATION = 0.3f;

		/**
		 * @brief Tire wear to grip degradation factor
		 *
		 * At 100% wear, grip is reduced to (1.0 - this factor).
		 */
		constexpr float TIRE_GRIP_DEGRADATION = 0.4f;
	}
}

// ============================================================================
// HANDLING MODE SETTINGS
// ============================================================================

/**
 * @brief Physics handling mode configuration
 *
 * Bundles all the parameters that differ between handling modes.
 * Applied to the vehicle movement component when mode changes.
 */
USTRUCT(BlueprintType)
struct FMGPhysicsHandlingSettings
{
	GENERATED_BODY()

	/** Handling mode this setting is for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handling")
	EMGPhysicsHandlingMode Mode = EMGPhysicsHandlingMode::Balanced;

	// ---- Stability Assists ----

	/**
	 * @brief Stability control strength (0-1)
	 *
	 * Automatically corrects oversteer/understeer.
	 * 0 = Off, 1 = Maximum intervention
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assists", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StabilityControl = 0.3f;

	/**
	 * @brief Anti-flip torque (Nm)
	 *
	 * Torque applied to prevent vehicle from flipping.
	 * Higher = more resistant to rollovers.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assists", meta = (ClampMin = "0.0", ClampMax = "50000.0"))
	float AntiFlipTorque = 5000.0f;

	/**
	 * @brief Speed-sensitive steering reduction (0-1)
	 *
	 * Reduces steering angle at high speeds.
	 * 0 = No reduction, 1 = Maximum reduction
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assists", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpeedSensitiveSteeringFactor = 0.5f;

	// ---- Physics Simulation ----

	/**
	 * @brief Weight transfer rate multiplier
	 *
	 * How quickly weight shifts occur.
	 * Lower = more forgiving, higher = more realistic.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float WeightTransferRate = 8.0f;

	/**
	 * @brief Base tire grip multiplier
	 *
	 * Overall grip level adjustment.
	 * Higher = more forgiving.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float BaseTireGrip = 1.0f;

	/**
	 * @brief Tire temperature effect strength (0-1)
	 *
	 * How much tire temperature affects grip.
	 * 0 = No effect, 1 = Full effect
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TireTempInfluence = 0.5f;

	/**
	 * @brief Turbo lag simulation strength (0-1)
	 *
	 * 0 = Instant boost, 1 = Realistic lag
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TurboLagSimulation = 1.0f;

	/**
	 * @brief Engine braking strength multiplier
	 *
	 * Lower = less aggressive engine braking.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float EngineBrakingMultiplier = 1.0f;

	// ---- Steering Response ----

	/**
	 * @brief Arcade steering speed (degrees/sec)
	 *
	 * How quickly steering responds to input.
	 * Higher = snappier response.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float ArcadeSteeringSpeed = 5.0f;

	/**
	 * @brief Arcade steering return speed (degrees/sec)
	 *
	 * How quickly steering centers when released.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering", meta = (ClampMin = "1.0", ClampMax = "20.0"))
	float ArcadeSteeringReturnSpeed = 8.0f;

	/** Returns default settings for specified mode */
	static FMGPhysicsHandlingSettings GetDefaultsForMode(EMGPhysicsHandlingMode InMode);
};

/**
 * @brief Physics handling mode subsystem interface
 *
 * Used by MGVehicleMovementComponent to query current handling mode settings.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPhysicsHandlingConfig : public UObject
{
	GENERATED_BODY()

public:
	/** Get default settings for each handling mode */
	UFUNCTION(BlueprintPure, Category = "Physics|Handling")
	static FMGPhysicsHandlingSettings GetArcadeSettings();

	UFUNCTION(BlueprintPure, Category = "Physics|Handling")
	static FMGPhysicsHandlingSettings GetBalancedSettings();

	UFUNCTION(BlueprintPure, Category = "Physics|Handling")
	static FMGPhysicsHandlingSettings GetSimulationSettings();

	/** Get settings for specified mode */
	UFUNCTION(BlueprintPure, Category = "Physics|Handling")
	static FMGPhysicsHandlingSettings GetSettingsForMode(EMGPhysicsHandlingMode Mode);
};

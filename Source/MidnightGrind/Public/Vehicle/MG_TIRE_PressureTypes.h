// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MG_TIRE_PressureTypes.h
 * @brief Tire pressure simulation types for realistic tire behavior.
 *
 * @section Overview
 * This file defines all types related to tire pressure simulation in MIDNIGHT GRIND.
 * Tire pressure is a critical factor affecting grip, wear, fuel economy, and blowout risk.
 *
 * @section Architecture
 * The tire pressure system integrates with:
 * - UMGVehicleMovementComponent: Applies pressure effects to grip
 * - UMGVehicleWearSubsystem: Tracks pressure over time
 * - UMGVehicleDamageSystem: Causes pressure loss from impacts
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Tire Pressure Effects**:
 * - Lower pressure = larger contact patch = more grip but faster wear
 * - Higher pressure = smaller contact patch = less grip but better economy
 * - Temperature increases pressure (ideal gas law)
 * - Optimal pressure varies by tire compound
 *
 * **Pressure Loss Causes**:
 * - Natural: All tires slowly lose pressure over time
 * - Damage: Impacts, punctures, and cuts cause faster loss
 * - Blowout: Catastrophic failure with instant pressure loss
 *
 * **Pressure Units**: The system uses PSI (pounds per square inch),
 * the standard unit for tire pressure in the US. Typical car tires
 * run 30-35 PSI cold, increasing 2-4 PSI when hot.
 *
 * @section Usage Example Usage
 * @code
 * // Check for pressure warnings
 * FMGTirePressureState PressureState = Movement->GetTirePressureState(WheelIndex);
 * if (PressureState.CurrentPressurePSI < PressureState.CriticalLowPressurePSI)
 * {
 *     ShowWarning("Low tire pressure!");
 * }
 *
 * // Cause pressure loss from damage
 * Movement->ApplyPressureLoss(WheelIndex, EMGPressureLossCause::ModerateLeakDamage);
 * @endcode
 *
 * @see UMGVehicleMovementComponent Where pressure affects physics
 * @see FMGTirePressureConfig Tunable pressure parameters
 */

#include "CoreMinimal.h"
#include "MG_TIRE_PressureTypes.generated.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

// Forward declarations
enum class EMGTireCompound : uint8;

/**
 * @brief Tire pressure loss cause enumeration
 *
 * Describes the reason for tire pressure loss during gameplay.
 * Used for damage feedback, audio cues, and repair cost calculation.
 *
 * Different causes have different leak rates and severity:
 * - Natural/Slow leaks: Minutes to hours to become problematic
 * - Moderate damage: Minutes to become critical
 * - Spike strips: Seconds to flatten
 * - Blowout: Instant catastrophic failure
 */
UENUM(BlueprintType)
enum class EMGPressureLossCause : uint8
{
	/** No pressure loss occurring */
	None UMETA(DisplayName = "None"),

	/** Gradual natural pressure loss over time (normal physics) */
	NaturalLeak UMETA(DisplayName = "Natural Leak"),

	/** Slow leak from minor damage such as nail or road debris */
	SlowLeak UMETA(DisplayName = "Slow Leak"),

	/** Moderate leak from curb strike or minor collision damage */
	ModerateLeakDamage UMETA(DisplayName = "Moderate Leak (Damage)"),

	/** Rapid pressure loss from spike strip puncture */
	SpikeStripPuncture UMETA(DisplayName = "Spike Strip Puncture"),

	/** Catastrophic blowout from severe damage or extreme overheating */
	Blowout UMETA(DisplayName = "Blowout"),

	/** Valve stem damage causing rapid pressure loss */
	ValveStemDamage UMETA(DisplayName = "Valve Stem Damage"),

	/** Bead separation from rim damage (severe impact) */
	BeadSeparation UMETA(DisplayName = "Bead Separation")
};

/**
 * @brief Configuration for tire pressure simulation tuning
 *
 * Contains all tunable parameters for the tire pressure physics system.
 * Designers can adjust these values to balance realism vs gameplay.
 *
 * Default values are based on typical passenger car tires with
 * some acceleration for gameplay purposes.
 *
 * Key relationships:
 * - Lower pressure = more grip (larger contact patch) but faster wear
 * - Higher pressure = less grip but better fuel economy and slower wear
 * - Temperature increases pressure (ideal gas law)
 * - Leaks and damage cause pressure loss over time
 *
 * @see FMGTirePressureState for runtime pressure state
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGTirePressureConfig
{
	GENERATED_BODY()

	// ==========================================
	// PRESSURE RANGES
	// ==========================================

	/**
	 * @brief Default cold tire pressure in PSI
	 *
	 * Standard pressure when tires are at ambient temperature (cold).
	 * Typical passenger car range: 30-35 PSI
	 * Performance/track: 28-32 PSI (lower for more grip)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Ranges", meta = (ClampMin = "20.0", ClampMax = "50.0"))
	float DefaultColdPressurePSI = 32.0f;

	/**
	 * @brief Minimum functional pressure in PSI
	 *
	 * Below this pressure, tire is considered flat and
	 * vehicle handling is severely compromised.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Ranges", meta = (ClampMin = "5.0", ClampMax = "20.0"))
	float MinFunctionalPressurePSI = 12.0f;

	/**
	 * @brief Critical low pressure warning threshold in PSI
	 *
	 * Triggers immediate warning and potential damage accumulation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Ranges", meta = (ClampMin = "10.0", ClampMax = "25.0"))
	float CriticalLowPressurePSI = 18.0f;

	/**
	 * @brief Maximum safe pressure in PSI
	 *
	 * Exceeding this pressure risks blowout, especially when hot.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Ranges", meta = (ClampMin = "40.0", ClampMax = "60.0"))
	float MaxSafePressurePSI = 50.0f;

	// ==========================================
	// TEMPERATURE-PRESSURE RELATIONSHIP
	// ==========================================

	/**
	 * @brief Pressure increase per degree Celsius of temperature rise
	 *
	 * Based on ideal gas law: P1/T1 = P2/T2
	 * Typical value: ~0.1-0.15 PSI per degree C
	 * At 32 PSI cold, expect ~36-38 PSI when hot (80-100C)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Temperature", meta = (ClampMin = "0.05", ClampMax = "0.3"))
	float PressurePerDegreeC = 0.12f;

	/**
	 * @brief Reference ambient temperature for cold pressure in Celsius
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Temperature", meta = (ClampMin = "10.0", ClampMax = "40.0"))
	float ReferenceAmbientTempC = 20.0f;

	// ==========================================
	// LEAK RATES
	// ==========================================

	/**
	 * @brief Natural pressure loss rate in PSI per hour
	 *
	 * All tires slowly lose pressure through rubber permeation.
	 * Real-world: 1-2 PSI per month = ~0.001-0.003 PSI/hour
	 * Accelerated for gameplay: 0.01-0.05 PSI/hour
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Leaks", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float NaturalLeakRatePSIPerHour = 0.02f;

	/**
	 * @brief Slow leak rate from minor damage in PSI per second
	 *
	 * Nail puncture, small debris damage, minor bead leak.
	 * Takes several minutes to become critical.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Leaks", meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float SlowLeakRatePSIPerSec = 0.05f;

	/**
	 * @brief Moderate leak rate from collision damage in PSI per second
	 *
	 * Curb strike, minor rim damage, sidewall cut.
	 * Noticeable within a minute of driving.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Leaks", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float ModerateLeakRatePSIPerSec = 0.3f;

	/**
	 * @brief Spike strip puncture rate in PSI per second
	 *
	 * Rapid but not instant - tire deflates over 10-30 seconds.
	 * Creates tense gameplay as player loses control gradually.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Leaks", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float SpikeStripLeakRatePSIPerSec = 1.5f;

	/**
	 * @brief Blowout instant pressure loss in PSI
	 *
	 * Catastrophic failure - immediate loss to near-zero.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Leaks", meta = (ClampMin = "20.0", ClampMax = "50.0"))
	float BlowoutInstantLossPSI = 30.0f;

	/**
	 * @brief Valve stem damage leak rate in PSI per second
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Leaks", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float ValveStemLeakRatePSIPerSec = 1.0f;

	/**
	 * @brief Bead separation leak rate in PSI per second
	 *
	 * Very rapid - tire separates from rim.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Leaks", meta = (ClampMin = "2.0", ClampMax = "10.0"))
	float BeadSeparationLeakRatePSIPerSec = 5.0f;

	// ==========================================
	// COMPOUND-SPECIFIC OPTIMAL PRESSURES
	// ==========================================

	/**
	 * @brief Optimal pressure for Street compound in PSI
	 *
	 * Street tires work best at standard pressures.
	 * Good all-around balance of grip and longevity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "28.0", ClampMax = "38.0"))
	float OptimalPressure_Street = 32.0f;

	/**
	 * @brief Optimal pressure for Sport compound in PSI
	 *
	 * Sport tires benefit from slightly lower pressure
	 * for increased contact patch and grip.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "26.0", ClampMax = "36.0"))
	float OptimalPressure_Sport = 30.0f;

	/**
	 * @brief Optimal pressure for Track compound in PSI
	 *
	 * Track tires run lower for maximum contact patch.
	 * Requires more careful heat management.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "24.0", ClampMax = "34.0"))
	float OptimalPressure_Track = 28.0f;

	/**
	 * @brief Optimal pressure for Drift compound in PSI
	 *
	 * Drift tires often run slightly higher in rear
	 * to promote controlled oversteer and slip.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "28.0", ClampMax = "40.0"))
	float OptimalPressure_Drift = 34.0f;

	/**
	 * @brief Optimal pressure for Rain compound in PSI
	 *
	 * Rain tires need higher pressure to maintain tread shape
	 * and resist hydroplaning through water channels.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "30.0", ClampMax = "40.0"))
	float OptimalPressure_Rain = 35.0f;

	/**
	 * @brief Optimal pressure for OffRoad compound in PSI
	 *
	 * Off-road tires run lower for better traction on loose surfaces.
	 * Larger contact patch helps with soft terrain.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "20.0", ClampMax = "32.0"))
	float OptimalPressure_OffRoad = 26.0f;

	// ==========================================
	// BLOWOUT THRESHOLDS
	// ==========================================

	/**
	 * @brief Temperature threshold for blowout risk in Celsius
	 *
	 * When tire exceeds this temp, blowout chance increases.
	 * Combined with low pressure = high blowout risk.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Blowout", meta = (ClampMin = "100.0", ClampMax = "200.0"))
	float BlowoutTempThresholdC = 140.0f;

	/**
	 * @brief Pressure ratio at which blowout risk begins (vs optimal)
	 *
	 * Running below this percentage of optimal pressure
	 * significantly increases blowout risk, especially at speed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Blowout", meta = (ClampMin = "0.3", ClampMax = "0.7"))
	float BlowoutPressureRatioThreshold = 0.5f;

	/**
	 * @brief Base blowout probability per second when conditions met
	 *
	 * Probability increases with speed, temperature, and low pressure.
	 * Keep low to make blowouts feel dramatic but not frustrating.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Blowout", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float BlowoutBaseProbabilityPerSec = 0.01f;

	/**
	 * @brief Speed multiplier for blowout probability (per 100 mph)
	 *
	 * Higher speeds increase blowout risk when other conditions are met.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Blowout", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BlowoutSpeedMultiplier = 0.5f;

	// ==========================================
	// SIMULATION SETTINGS
	// ==========================================

	/**
	 * @brief Enable natural pressure loss over time
	 *
	 * When true, tires slowly lose pressure even without damage.
	 * Adds strategic element to longer races/sessions.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Simulation")
	bool bEnableNaturalPressureLoss = true;

	/**
	 * @brief Enable temperature-based pressure changes
	 *
	 * When true, tire pressure increases as tires heat up.
	 * Creates realistic "cold tire" behavior at race start.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Simulation")
	bool bEnableTemperaturePressureEffect = true;

	/**
	 * @brief Enable random blowout chance when conditions are dangerous
	 *
	 * When true, severely compromised tires can blow out randomly.
	 * Creates tension and consequences for poor tire management.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Simulation")
	bool bEnableBlowoutSimulation = true;

	/**
	 * @brief Time scale for pressure simulation (1.0 = real time)
	 *
	 * Higher values accelerate pressure changes for gameplay.
	 * Use > 1.0 for arcade feel, 1.0 for simulation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Simulation", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PressureSimulationTimeScale = 1.0f;
};

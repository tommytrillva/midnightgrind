// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MGTirePressureTypes.generated.h"

// Forward declarations
enum class EMGTireCompound : uint8;

/**
 * @brief Tire pressure loss cause enumeration
 *
 * Describes the reason for tire pressure loss during gameplay.
 * Used for damage feedback, audio cues, and repair cost calculation.
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
 * @brief Tire pressure state with comprehensive physics simulation
 *
 * Models realistic tire pressure behavior including:
 * - Temperature-pressure relationship (ideal gas law approximation)
 * - Pressure effects on contact patch size and shape
 * - Pressure influence on grip, wear rate, and fuel economy
 * - Multiple pressure loss scenarios (punctures, slow leaks, blowouts)
 * - Optimal pressure ranges that vary by tire compound
 *
 * Physical relationships modeled:
 *
 * Lower pressure provides:
 * - Larger contact patch = more mechanical grip (up to a point)
 * - Faster tire wear due to increased deformation and heat
 * - Higher rolling resistance = worse fuel economy
 * - Higher operating temperatures from sidewall flex
 * - More compliant ride over bumps
 *
 * Higher pressure provides:
 * - Smaller contact patch = reduced grip area
 * - Slower tire wear from reduced deformation
 * - Lower rolling resistance = better fuel economy
 * - Lower operating temperatures
 * - Harsher ride, less compliance
 * - Risk of reduced traction on uneven surfaces
 *
 * @see FMGTireTemperature for temperature modeling
 * @see FMGTireCompoundData for compound-specific optimal pressures
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGTirePressureState
{
	GENERATED_BODY()

	// ==========================================
	// CURRENT STATE
	// ==========================================

	/** Current tire pressure in PSI (pounds per square inch) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Current", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float CurrentPressurePSI = 32.0f;

	/** Cold pressure setting (baseline pressure when tire is at ambient temperature) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Current", meta = (ClampMin = "20.0", ClampMax = "50.0"))
	float ColdPressurePSI = 32.0f;

	/** Hot pressure (calculated from temperature using ideal gas law approximation) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Current")
	float HotPressurePSI = 32.0f;

	/** Target optimal pressure for current compound and conditions */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Current")
	float OptimalPressurePSI = 32.0f;

	// ==========================================
	// PRESSURE LOSS STATE
	// ==========================================

	/** Whether the tire has any active pressure loss */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Loss")
	bool bHasLeak = false;

	/** Whether the tire is completely flat (pressure below functional threshold) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Loss")
	bool bIsFlat = false;

	/** Whether the tire has suffered a catastrophic blowout */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Loss")
	bool bIsBlownOut = false;

	/** Current cause of pressure loss */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Loss")
	EMGPressureLossCause LeakCause = EMGPressureLossCause::None;

	/** Rate of pressure loss in PSI per second */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Loss")
	float LeakRatePSIPerSecond = 0.0f;

	/** Time since leak started in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Loss")
	float LeakDuration = 0.0f;

	/** Total pressure lost to leaks this session in PSI */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Loss")
	float TotalPressureLost = 0.0f;

	// ==========================================
	// PERFORMANCE EFFECTS
	// ==========================================

	/** Contact patch size multiplier (1.0 = optimal, >1.0 = larger patch from low pressure) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Effects")
	float ContactPatchMultiplier = 1.0f;

	/** Grip multiplier from pressure (optimal pressure = 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Effects")
	float PressureGripMultiplier = 1.0f;

	/** Wear rate multiplier from pressure (lower pressure = faster wear) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Effects")
	float PressureWearMultiplier = 1.0f;

	/** Heat generation multiplier from pressure (lower pressure = more heat) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Effects")
	float PressureHeatMultiplier = 1.0f;

	/** Rolling resistance multiplier (lower pressure = higher resistance) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Effects")
	float RollingResistanceMultiplier = 1.0f;

	/** Fuel economy effect (1.0 = no effect, <1.0 = worse economy from low pressure) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Effects")
	float FuelEconomyMultiplier = 1.0f;

	// ==========================================
	// DIAGNOSTIC DATA
	// ==========================================

	/** Pressure deviation from optimal (negative = under-inflated, positive = over-inflated) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Diagnostics")
	float PressureDeviationPSI = 0.0f;

	/** Percentage of optimal pressure (100% = perfect) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Diagnostics")
	float PressurePercent = 100.0f;

	/** Time spent at critically low pressure in seconds (for damage tracking) */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Diagnostics")
	float TimeAtCriticalPressure = 0.0f;

	/** Number of punctures/damage events this tire has sustained */
	UPROPERTY(BlueprintReadOnly, Category = "Pressure|Diagnostics")
	int32 DamageEventCount = 0;

	// ==========================================
	// HELPER METHODS
	// ==========================================

	/**
	 * @brief Calculate grip multiplier based on current pressure vs optimal
	 *
	 * Grip curve models real tire behavior:
	 * - At optimal pressure: 1.0 grip (baseline)
	 * - Slightly under-inflated (5-10%): 1.02-1.07 grip (larger contact patch bonus)
	 * - Significantly under-inflated (>15%): Grip drops due to sidewall flex instability
	 * - Over-inflated: Grip reduces linearly due to smaller contact patch
	 * - Flat tire: Minimal grip (0.1-0.3) with severe handling issues
	 *
	 * @return Grip multiplier in range [0.15, 1.07]
	 */
	float CalculateGripMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 0.15f; // Severely compromised grip
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalPressurePSI, 1.0f);

		if (PressureRatio < 0.5f)
		{
			// Critically low - severe grip loss from sidewall collapse
			return 0.3f + (PressureRatio * 0.4f);
		}
		else if (PressureRatio < 0.85f)
		{
			// Under-inflated but functional - grip improves then plateaus
			const float UnderInflateRatio = (PressureRatio - 0.5f) / 0.35f;
			return FMath::Lerp(0.5f, 1.05f, UnderInflateRatio);
		}
		else if (PressureRatio <= 0.95f)
		{
			// Slightly under-inflated - sweet spot for maximum grip
			return 1.03f + (0.95f - PressureRatio) * 0.4f; // 1.03 to 1.07
		}
		else if (PressureRatio <= 1.05f)
		{
			// Near optimal - baseline grip
			return 1.0f;
		}
		else if (PressureRatio <= 1.15f)
		{
			// Slightly over-inflated - reduced contact patch
			return 1.0f - ((PressureRatio - 1.05f) * 0.5f); // 1.0 to 0.95
		}
		else
		{
			// Significantly over-inflated - accelerating grip loss
			return FMath::Max(0.7f, 0.95f - ((PressureRatio - 1.15f) * 1.5f));
		}
	}

	/**
	 * @brief Calculate wear rate multiplier based on pressure
	 *
	 * Under-inflated tires wear faster due to:
	 * - Increased contact patch area under load
	 * - More sidewall flex generating internal heat
	 * - Uneven wear patterns (edges wear faster than center)
	 *
	 * Over-inflated tires have moderate wear increase:
	 * - Concentrated wear in center of tread
	 * - Less total contact area but higher pressure per unit area
	 *
	 * @return Wear rate multiplier (1.0 = normal, >1.0 = faster wear)
	 */
	float CalculateWearMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 5.0f; // Catastrophic wear when running flat
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalPressurePSI, 1.0f);

		if (PressureRatio < 0.7f)
		{
			// Severely under-inflated - rapid wear from excessive deformation
			return 3.0f - (PressureRatio * 2.0f);
		}
		else if (PressureRatio < 0.9f)
		{
			// Under-inflated - accelerated wear
			return 1.0f + ((0.9f - PressureRatio) * 3.0f);
		}
		else if (PressureRatio <= 1.1f)
		{
			// Optimal range - normal wear
			return 1.0f;
		}
		else
		{
			// Over-inflated - moderate wear increase from center wear
			return 1.0f + ((PressureRatio - 1.1f) * 1.5f);
		}
	}

	/**
	 * @brief Calculate contact patch size multiplier
	 *
	 * Lower pressure = larger contact patch (more rubber contacting road)
	 * Higher pressure = smaller contact patch (less rubber on road)
	 * Relationship is approximately inverse within normal operating range.
	 *
	 * @return Contact patch size multiplier in range [0.5, 1.4]
	 */
	float CalculateContactPatchMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 0.5f; // Deformed tire has inconsistent, reduced contact
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalPressurePSI, 1.0f);

		// Inverse relationship - lower pressure = larger patch
		// But extremely low pressure causes sidewall collapse reducing effective contact
		if (PressureRatio < 0.5f)
		{
			return 0.8f; // Collapsed sidewall reduces effective contact
		}

		return FMath::Clamp(1.5f - (PressureRatio * 0.5f), 0.7f, 1.4f);
	}

	/**
	 * @brief Calculate heat generation multiplier based on pressure
	 *
	 * Lower pressure = more sidewall flex = more internal heat generation
	 * This is a primary cause of tire failures when running under-inflated.
	 *
	 * @return Heat generation multiplier (1.0 = normal, >1.0 = more heat)
	 */
	float CalculateHeatMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 3.0f; // Massive heat from running on sidewall/rim
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalPressurePSI, 1.0f);

		if (PressureRatio < 0.7f)
		{
			// Severely under-inflated - extreme heat buildup risk
			return 2.5f - (PressureRatio * 1.5f);
		}
		else if (PressureRatio < 0.9f)
		{
			// Under-inflated - increased heat from sidewall flex
			return 1.0f + ((0.9f - PressureRatio) * 2.5f);
		}
		else if (PressureRatio <= 1.1f)
		{
			// Optimal range - normal heat generation
			return 1.0f;
		}
		else
		{
			// Over-inflated - slightly less heat (smaller contact, less deformation)
			return FMath::Max(0.85f, 1.0f - ((PressureRatio - 1.1f) * 0.5f));
		}
	}

	/**
	 * @brief Calculate rolling resistance multiplier for fuel economy
	 *
	 * Lower pressure increases rolling resistance due to:
	 * - More tire deformation per rotation
	 * - Larger contact patch creating more friction
	 * - Energy lost to internal tire heating
	 *
	 * @return Rolling resistance multiplier (1.0 = optimal, >1.0 = more resistance)
	 */
	float CalculateRollingResistanceMultiplier() const
	{
		if (bIsBlownOut || bIsFlat)
		{
			return 3.0f; // Massive resistance when flat
		}

		const float PressureRatio = CurrentPressurePSI / FMath::Max(OptimalPressurePSI, 1.0f);

		if (PressureRatio < 0.85f)
		{
			// Under-inflated - significant rolling resistance increase
			return 1.0f + ((0.85f - PressureRatio) * 2.0f);
		}
		else if (PressureRatio <= 1.1f)
		{
			// Optimal range
			return 1.0f;
		}
		else
		{
			// Over-inflated - slightly reduced resistance (also less grip though)
			return FMath::Max(0.95f, 1.0f - ((PressureRatio - 1.1f) * 0.3f));
		}
	}

	/**
	 * @brief Check if pressure is in a warning state requiring driver attention
	 * @return True if pressure is significantly off optimal or has active issues
	 */
	bool NeedsAttention() const
	{
		if (bIsFlat || bIsBlownOut || bHasLeak)
		{
			return true;
		}

		const float DeviationPercent = FMath::Abs(PressureDeviationPSI) / FMath::Max(OptimalPressurePSI, 1.0f) * 100.0f;
		return DeviationPercent > 10.0f; // More than 10% off optimal
	}

	/**
	 * @brief Check if tire is in critical state requiring immediate action
	 * @return True if tire is critically low, damaged, or rapidly losing pressure
	 */
	bool IsCritical() const
	{
		return bIsFlat || bIsBlownOut || CurrentPressurePSI < 15.0f || LeakRatePSIPerSecond > 1.0f;
	}

	/**
	 * @brief Reset pressure state to defaults
	 * @param DefaultPressure The default cold pressure to set in PSI
	 * @param Optimal The optimal pressure for current compound in PSI
	 */
	void Reset(float DefaultPressure = 32.0f, float Optimal = 32.0f)
	{
		CurrentPressurePSI = DefaultPressure;
		ColdPressurePSI = DefaultPressure;
		HotPressurePSI = DefaultPressure;
		OptimalPressurePSI = Optimal;
		bHasLeak = false;
		bIsFlat = false;
		bIsBlownOut = false;
		LeakCause = EMGPressureLossCause::None;
		LeakRatePSIPerSecond = 0.0f;
		LeakDuration = 0.0f;
		TotalPressureLost = 0.0f;
		TimeAtCriticalPressure = 0.0f;
		DamageEventCount = 0;
		UpdateEffects();
	}

	/**
	 * @brief Update all calculated effect multipliers based on current pressure
	 * Should be called after any pressure value changes
	 */
	void UpdateEffects()
	{
		PressureGripMultiplier = CalculateGripMultiplier();
		PressureWearMultiplier = CalculateWearMultiplier();
		ContactPatchMultiplier = CalculateContactPatchMultiplier();
		PressureHeatMultiplier = CalculateHeatMultiplier();
		RollingResistanceMultiplier = CalculateRollingResistanceMultiplier();
		FuelEconomyMultiplier = 1.0f / FMath::Max(RollingResistanceMultiplier, 0.1f);
		PressureDeviationPSI = CurrentPressurePSI - OptimalPressurePSI;
		PressurePercent = (CurrentPressurePSI / FMath::Max(OptimalPressurePSI, 1.0f)) * 100.0f;
	}
};

/**
 * @brief Configuration for tire pressure simulation tuning
 *
 * Contains all tunable parameters for the tire pressure physics system.
 * Default values are based on typical passenger car tires.
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
	 * Typical: 1-2 PSI per month = ~0.001-0.003 PSI/hour
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
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "28.0", ClampMax = "38.0"))
	float OptimalPressure_Street = 32.0f;

	/**
	 * @brief Optimal pressure for Sport compound in PSI
	 *
	 * Sport tires benefit from slightly lower pressure.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "26.0", ClampMax = "36.0"))
	float OptimalPressure_Sport = 30.0f;

	/**
	 * @brief Optimal pressure for Track compound in PSI
	 *
	 * Track tires run lower for maximum contact patch.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "24.0", ClampMax = "34.0"))
	float OptimalPressure_Track = 28.0f;

	/**
	 * @brief Optimal pressure for Drift compound in PSI
	 *
	 * Drift tires often run slightly higher in rear for slip.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "28.0", ClampMax = "40.0"))
	float OptimalPressure_Drift = 34.0f;

	/**
	 * @brief Optimal pressure for Rain compound in PSI
	 *
	 * Rain tires need higher pressure to resist hydroplaning.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Compounds", meta = (ClampMin = "30.0", ClampMax = "40.0"))
	float OptimalPressure_Rain = 35.0f;

	/**
	 * @brief Optimal pressure for OffRoad compound in PSI
	 *
	 * Off-road tires run lower for better traction on loose surfaces.
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
	 * Probability increases with speed and temperature.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Blowout", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float BlowoutBaseProbabilityPerSec = 0.01f;

	/**
	 * @brief Speed multiplier for blowout probability (per 100 km/h)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Blowout", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BlowoutSpeedMultiplier = 0.5f;

	// ==========================================
	// SIMULATION SETTINGS
	// ==========================================

	/**
	 * @brief Enable natural pressure loss over time
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Simulation")
	bool bEnableNaturalPressureLoss = true;

	/**
	 * @brief Enable temperature-based pressure changes
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Simulation")
	bool bEnableTemperaturePressureEffect = true;

	/**
	 * @brief Enable random blowout chance when conditions are dangerous
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Simulation")
	bool bEnableBlowoutSimulation = true;

	/**
	 * @brief Time scale for pressure simulation (1.0 = real time)
	 *
	 * Higher values accelerate pressure changes for gameplay.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure|Simulation", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PressureSimulationTimeScale = 1.0f;
};

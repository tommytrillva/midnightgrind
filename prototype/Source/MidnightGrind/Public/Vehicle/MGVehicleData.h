// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehicleData.h
 * @brief Vehicle data structures and configuration types for MIDNIGHT GRIND.
 *
 * This file contains all data structures used to define vehicle specifications,
 * including engine configuration, drivetrain setup, suspension tuning, brake
 * configuration, wheel/tire specs, and aerodynamic components.
 *
 * The data structures support:
 * - Complete vehicle customization and tuning
 * - Part-based upgrade system with tiers
 * - Power curve (dyno) data for realistic engine simulation
 * - Ownership and race history tracking
 * - Performance Index (PI) classification system
 *
 * @see AMGVehiclePawn The vehicle pawn that uses this data
 * @see UMGVehicleMovementComponent Physics component that applies this data
 * @see UMGStatCalculator Utility class for calculating stats from this data
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGVehicleData.generated.h"

// ==========================================
// ENUMERATIONS
// ==========================================

/**
 * @brief Vehicle performance class based on Performance Index (PI).
 *
 * Performance classes are used to balance matchmaking and race categories.
 * Each class represents a range of PI values calculated from vehicle specs.
 */
UENUM(BlueprintType)
enum class EMGPerformanceClass : uint8
{
	/** D Class: Entry-level vehicles (100-299 PI) - Economical daily drivers */
	D UMETA(DisplayName = "D Class (100-299 PI)"),
	/** C Class: Sport compact (300-449 PI) - Hot hatches, entry sports cars */
	C UMETA(DisplayName = "C Class (300-449 PI)"),
	/** B Class: Sports cars (450-599 PI) - Mid-range performance vehicles */
	B UMETA(DisplayName = "B Class (450-599 PI)"),
	/** A Class: High performance (600-749 PI) - Tuned sports cars, muscle */
	A UMETA(DisplayName = "A Class (600-749 PI)"),
	/** S Class: Supercars (750-900 PI) - Elite performance machines */
	S UMETA(DisplayName = "S Class (750-900 PI)"),
	/** X Class: Hypercars (901+ PI) - No-limit extreme builds */
	X UMETA(DisplayName = "X Class (901+ PI)")
};

/**
 * @brief Aftermarket part quality/upgrade tier.
 *
 * Each tier represents a level of part quality and performance.
 * Higher tiers provide better stats but cost more and may affect reliability.
 */
UENUM(BlueprintType)
enum class EMGPartTier : uint8
{
	/** Stock: Factory original parts - Balanced reliability and cost */
	Stock UMETA(DisplayName = "Stock"),
	/** Street: Basic aftermarket upgrades - Slight performance gain */
	Street UMETA(DisplayName = "Street"),
	/** Sport: Mid-tier performance parts - Noticeable improvement */
	Sport UMETA(DisplayName = "Sport"),
	/** Race: Competition-grade parts - Significant performance boost */
	Race UMETA(DisplayName = "Race"),
	/** Pro: Professional motorsport quality - Near-maximum performance */
	Pro UMETA(DisplayName = "Pro"),
	/** Legendary: Ultra-rare exotic parts - Maximum performance, unique */
	Legendary UMETA(DisplayName = "Legendary")
};

/**
 * @brief Engine cylinder configuration/layout type.
 *
 * Determines the engine's physical layout which affects power delivery,
 * sound character, and natural balance/vibration characteristics.
 */
UENUM(BlueprintType)
enum class EMGEngineType : uint8
{
	/** Inline 4: Common in sport compacts - Good torque curve, efficient */
	I4 UMETA(DisplayName = "Inline 4"),
	/** Inline 6: Smooth power delivery - Naturally balanced, linear power */
	I6 UMETA(DisplayName = "Inline 6"),
	/** V6: Compact design - Good power-to-size ratio */
	V6 UMETA(DisplayName = "V6"),
	/** V8: American muscle - High torque, distinctive sound */
	V8 UMETA(DisplayName = "V8"),
	/** Rotary (Wankel): High-revving - Unique power band, turbo-friendly */
	Rotary UMETA(DisplayName = "Rotary"),
	/** Flat 4 (Boxer): Low center of gravity - Good handling balance */
	Flat4 UMETA(DisplayName = "Flat 4 (Boxer)"),
	/** Flat 6 (Boxer): Porsche-style - Excellent balance, high-rev capable */
	Flat6 UMETA(DisplayName = "Flat 6 (Boxer)")
};

/**
 * @brief Vehicle drivetrain/power delivery configuration.
 *
 * Determines which wheels receive power from the engine, significantly
 * affecting handling characteristics, traction, and driving style.
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
	/** Front Wheel Drive: Power to front wheels - Understeer tendency, efficient */
	FWD UMETA(DisplayName = "Front Wheel Drive"),
	/** Rear Wheel Drive: Power to rear wheels - Classic drift-friendly layout */
	RWD UMETA(DisplayName = "Rear Wheel Drive"),
	/** All Wheel Drive: Power to all wheels - Maximum traction, balanced handling */
	AWD UMETA(DisplayName = "All Wheel Drive")
};

/**
 * @brief Engine forced induction (boost) type.
 *
 * Determines how additional air is forced into the engine for more power.
 * Each type has different power delivery characteristics and lag behavior.
 */
UENUM(BlueprintType)
enum class EMGForcedInductionType : uint8
{
	/** Naturally Aspirated: No forced induction - Linear power, instant response */
	None UMETA(DisplayName = "Naturally Aspirated"),
	/** Single Turbo: One exhaust-driven turbine - Turbo lag, high peak power */
	Turbo_Single UMETA(DisplayName = "Single Turbo"),
	/** Twin Turbo: Two turbos - Reduced lag, broader powerband */
	Turbo_Twin UMETA(DisplayName = "Twin Turbo"),
	/** Roots Supercharger: Belt-driven positive displacement - Instant boost, linear */
	Supercharger_Roots UMETA(DisplayName = "Roots Supercharger"),
	/** Twin-Screw Supercharger: Efficient positive displacement - Good low-end boost */
	Supercharger_TwinScrew UMETA(DisplayName = "Twin-Screw Supercharger"),
	/** Centrifugal Supercharger: Belt-driven centrifugal - RPM-dependent boost curve */
	Supercharger_Centrifugal UMETA(DisplayName = "Centrifugal Supercharger")
};

/**
 * @brief Differential type affecting power distribution to wheels.
 *
 * The differential type significantly affects handling, especially during
 * cornering and power application. LSD types provide better traction.
 */
UENUM(BlueprintType)
enum class EMGDifferentialType : uint8
{
	/** Open: Standard differential - Power goes to wheel with least resistance */
	Open UMETA(DisplayName = "Open"),
	/** 1-Way LSD: Locks under acceleration only - Good for drag, predictable cornering */
	LSD_1Way UMETA(DisplayName = "1-Way LSD"),
	/** 1.5-Way LSD: Partial decel lock - Balanced for street/track use */
	LSD_1_5Way UMETA(DisplayName = "1.5-Way LSD"),
	/** 2-Way LSD: Locks on accel and decel - Aggressive, drift-friendly */
	LSD_2Way UMETA(DisplayName = "2-Way LSD"),
	/** Torsen: Gear-based torque sensing - Smooth, progressive lockup */
	Torsen UMETA(DisplayName = "Torsen"),
	/** Locked/Welded: Permanently locked - Maximum traction, difficult cornering */
	Locked UMETA(DisplayName = "Locked/Welded")
};

/**
 * @brief Transmission/gearbox type.
 *
 * Determines how gears are selected and shifted, affecting shift speed,
 * engagement feel, and driver interaction requirements.
 */
UENUM(BlueprintType)
enum class EMGTransmissionType : uint8
{
	/** Manual: Traditional H-pattern or dog-leg - Full driver control */
	Manual UMETA(DisplayName = "Manual"),
	/** Automatic: Torque converter auto - Smooth, comfortable shifts */
	Automatic UMETA(DisplayName = "Automatic"),
	/** Sequential: Race-style sequential - Fast, precise paddle/lever shifts */
	Sequential UMETA(DisplayName = "Sequential"),
	/** DCT: Dual-Clutch Transmission - Lightning-fast automated shifts */
	DCT UMETA(DisplayName = "Dual-Clutch (DCT)"),
	/** CVT: Continuously Variable - Seamless ratio changes, optimal efficiency */
	CVT UMETA(DisplayName = "CVT")
};

/**
 * @brief Tire rubber compound type affecting grip and wear characteristics.
 *
 * Different compounds offer trade-offs between grip level, operating
 * temperature window, wear rate, and wet weather performance.
 */
UENUM(BlueprintType)
enum class EMGTireCompound : uint8
{
	/** Economy: Budget tires - Low grip, long life, all-weather */
	Economy UMETA(DisplayName = "Economy"),
	/** All-Season: Versatile compound - Moderate grip, good in varied conditions */
	AllSeason UMETA(DisplayName = "All-Season"),
	/** Sport: Street performance - Good dry grip, reasonable wear */
	Sport UMETA(DisplayName = "Sport"),
	/** Performance: High-grip street legal - Excellent dry grip, faster wear */
	Performance UMETA(DisplayName = "Performance"),
	/** Semi-Slick: Track day tires - Very high grip, minimal tread, limited wet use */
	SemiSlick UMETA(DisplayName = "Semi-Slick"),
	/** Slick: Full racing slicks - Maximum dry grip, no tread, track only */
	Slick UMETA(DisplayName = "Slick"),
	/** Drag Radial: Specialized drag tires - Extreme launch traction, soft compound */
	DragRadial UMETA(DisplayName = "Drag Radial"),
	/** Drift: Controlled slip compound - Predictable breakaway, slide-friendly */
	Drift UMETA(DisplayName = "Drift")
};

// ==========================================
// DATA STRUCTURES
// ==========================================

/**
 * @brief Historical ownership record for vehicle provenance tracking.
 *
 * Tracks the complete ownership history of a vehicle, including how it
 * was acquired (purchase, pink slip win, trade) and its condition at transfer.
 */
USTRUCT(BlueprintType)
struct FMGOwnershipRecord
{
	GENERATED_BODY()

	/** @brief Unique identifier of the owner player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid OwnerID;

	/** @brief Date and time when this owner acquired the vehicle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AcquiredDate;

	/** @brief Date and time when this owner sold/lost the vehicle (invalid if current owner). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SoldDate;

	/** @brief How the vehicle was acquired ("Purchase", "PinkSlip", "Trade", "Gift", "Prize"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AcquisitionMethod;

	/** @brief Vehicle mileage at the time of acquisition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MileageAtAcquisition;
};

/**
 * @brief Single data point on the engine's power/dyno curve.
 *
 * Represents horsepower and torque output at a specific RPM.
 * Multiple points are interpolated to create the full power curve.
 */
USTRUCT(BlueprintType)
struct FMGPowerCurvePoint
{
	GENERATED_BODY()

	/** @brief Engine RPM at this measurement point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RPM;

	/** @brief Horsepower output at this RPM (wheel HP). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Horsepower;

	/** @brief Torque output at this RPM in lb-ft (wheel torque). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Torque;
};

/**
 * @brief Complete engine power/torque curve (dyno graph data).
 *
 * Contains all data points that define the engine's power characteristics
 * across its RPM range, plus summary statistics for quick reference.
 *
 * Used by the movement component to determine actual power output
 * at any given RPM during simulation.
 */
USTRUCT(BlueprintType)
struct FMGPowerCurve
{
	GENERATED_BODY()

	/** @brief Array of data points defining the power curve. Should be sorted by RPM. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPowerCurvePoint> CurvePoints;

	/** @brief Peak horsepower value achieved on the curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakHP;

	/** @brief RPM at which peak horsepower occurs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PeakHPRPM;

	/** @brief Peak torque value achieved on the curve (lb-ft). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakTorque;

	/** @brief RPM at which peak torque occurs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PeakTorqueRPM;

	/** @brief Engine redline RPM (rev limiter activates here). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Redline;
};

/**
 * @brief Forced induction (turbo/supercharger) system configuration.
 *
 * Contains all parameters needed to simulate turbo/supercharger behavior
 * including boost levels, spool characteristics, and supporting components.
 */
USTRUCT(BlueprintType)
struct FMGForcedInductionConfig
{
	GENERATED_BODY()

	/** @brief Type of forced induction system installed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGForcedInductionType Type = EMGForcedInductionType::None;

	/** @brief Part ID reference for the turbo/supercharger unit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TurboID;

	/** @brief Maximum boost pressure in PSI at full spool. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxBoostPSI = 0.0f;

	/** @brief Time in seconds from no boost to full boost at WOT. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpoolTimeSeconds = 0.0f;

	/** @brief Minimum RPM where boost begins to build. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BoostThresholdRPM = 0;

	/** @brief Part ID for the wastegate controlling max boost. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WastegateID;

	/** @brief Part ID for the blow-off valve (compressor surge protection). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BlowOffValveID;

	/** @brief Part ID for the intercooler (charge air cooling). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName IntercoolerID;

	/** @brief Intercooler efficiency (0.0-1.0) affecting charge air temperature. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IntercoolerEfficiency = 0.85f;
};

/**
 * @brief Nitrous oxide (NOS) system configuration.
 *
 * Defines the nitrous system setup including type, power delivery,
 * and current bottle fill level.
 */
USTRUCT(BlueprintType)
struct FMGNitrousConfig
{
	GENERATED_BODY()

	/** @brief Whether a nitrous system is installed on this vehicle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInstalled = false;

	/** @brief Part ID reference for the nitrous system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SystemID;

	/** @brief System type: "Dry" (fuel added separately), "Wet" (fuel mixed), "DirectPort" (individual injectors). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SystemType = TEXT("Wet");

	/** @brief Horsepower added when nitrous is activated. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShotSizeHP = 100.0f;

	/** @brief Nitrous bottle capacity in pounds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BottleSizeLbs = 10.0f;

	/** @brief Current nitrous fill level as percentage (0-100). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFillPercent = 100.0f;
};

/**
 * @brief Fuel type enumeration for different fuel grades
 *
 * Different fuel types affect performance, consumption, and cost.
 * Higher octane fuels allow more aggressive tuning without knock.
 */
UENUM(BlueprintType)
enum class EMGFuelType : uint8
{
	/** Regular unleaded (87 octane) - baseline fuel */
	Regular UMETA(DisplayName = "Regular (87)"),

	/** Mid-grade (89 octane) - slight improvement */
	MidGrade UMETA(DisplayName = "Mid-Grade (89)"),

	/** Premium (91-93 octane) - required for performance engines */
	Premium UMETA(DisplayName = "Premium (91-93)"),

	/** Race fuel (100+ octane) - maximum performance, highest cost */
	RaceFuel UMETA(DisplayName = "Race (100+)"),

	/** E85 (85% ethanol) - higher octane, requires flex-fuel system */
	E85 UMETA(DisplayName = "E85 Ethanol")
};

/**
 * @brief Fuel tank configuration for vehicle fuel system
 *
 * Defines the physical characteristics of the vehicle's fuel tank,
 * including capacity, current level, and starvation parameters.
 * Used by UMGFuelConsumptionComponent for consumption simulation.
 */
USTRUCT(BlueprintType)
struct FMGFuelTankConfig
{
	GENERATED_BODY()

	/** Tank capacity in US gallons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float CapacityGallons = 15.0f;

	/** Current fuel level in gallons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "0.0"))
	float CurrentFuelGallons = 15.0f;

	/**
	 * @brief Fuel weight per gallon in pounds
	 *
	 * Reference values:
	 * - Gasoline: ~6.0 lbs/gal
	 * - E85: ~6.6 lbs/gal
	 * - Diesel: ~7.1 lbs/gal
	 * - Race fuel: ~5.8 lbs/gal
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "5.0", ClampMax = "8.0"))
	float FuelWeightPerGallon = 6.0f;

	/** Type of fuel currently in tank */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank")
	EMGFuelType FuelType = EMGFuelType::Regular;

	/**
	 * @brief Whether tank has internal baffles
	 *
	 * Baffled tanks reduce fuel slosh and starvation during cornering.
	 * Racing tanks typically have baffles; stock tanks may not.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank")
	bool bHasBaffles = true;

	/**
	 * @brief Fuel pickup height from tank bottom (inches)
	 *
	 * Affects when fuel starvation begins during cornering.
	 * Lower pickup = less starvation risk but cannot drain tank completely.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float PickupHeightInches = 1.0f;

	/** Reserve fuel warning threshold (gallons) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float ReserveCapacityGallons = 2.0f;

	/** Critical fuel level for starvation risk (gallons) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "0.25", ClampMax = "2.0"))
	float CriticalLevelGallons = 0.5f;

	/** Base fuel consumption rate at idle (gallons per hour) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumption", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float BaseIdleConsumptionGPH = 0.3f;

	/**
	 * @brief Engine efficiency factor affecting consumption
	 *
	 * Larger, less efficient engines have lower values.
	 * Modern fuel-injected engines: 0.8-1.0
	 * Carbureted engines: 0.6-0.8
	 * Performance engines: 0.5-0.7 (drink more fuel)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumption", meta = (ClampMin = "0.3", ClampMax = "1.2"))
	float EngineEfficiencyFactor = 0.8f;

	/**
	 * @brief Calculate current fuel weight in kilograms
	 * @return Fuel weight in kg
	 */
	float GetFuelWeightKg() const
	{
		// Convert lbs to kg (1 lb = 0.453592 kg)
		return CurrentFuelGallons * FuelWeightPerGallon * 0.453592f;
	}

	/**
	 * @brief Get fuel percentage remaining
	 * @return Percentage (0.0 to 1.0)
	 */
	float GetFuelPercentage() const
	{
		return (CapacityGallons > 0.0f) ? (CurrentFuelGallons / CapacityGallons) : 0.0f;
	}

	/**
	 * @brief Check if at reserve level
	 * @return True if at or below reserve
	 */
	bool IsAtReserve() const
	{
		return CurrentFuelGallons <= ReserveCapacityGallons;
	}

	/**
	 * @brief Check if critically low
	 * @return True if at critical level
	 */
	bool IsCritical() const
	{
		return CurrentFuelGallons <= CriticalLevelGallons;
	}

	/**
	 * @brief Get octane rating for current fuel type
	 * @return Octane rating value
	 */
	int32 GetOctaneRating() const
	{
		switch (FuelType)
		{
			case EMGFuelType::Regular:   return 87;
			case EMGFuelType::MidGrade:  return 89;
			case EMGFuelType::Premium:   return 93;
			case EMGFuelType::RaceFuel:  return 104;
			case EMGFuelType::E85:       return 105;
			default:                     return 87;
		}
	}
};

/**
 * ECU map type for different driving modes
 */
UENUM(BlueprintType)
enum class EMGECUMapType : uint8
{
	/** Stock conservative map - balanced for everyday driving */
	Stock UMETA(DisplayName = "Stock"),

	/** Economy map - prioritizes fuel efficiency */
	Economy UMETA(DisplayName = "Economy"),

	/** Sport map - increased performance with some efficiency loss */
	Sport UMETA(DisplayName = "Sport"),

	/** Performance map - aggressive timing, higher power, increased wear */
	Performance UMETA(DisplayName = "Performance"),

	/** Race map - maximum power, requires premium fuel, increased engine stress */
	Race UMETA(DisplayName = "Race"),

	/** Custom map - user-defined parameters */
	Custom UMETA(DisplayName = "Custom"),

	/** Valet mode - reduced power output for protection */
	Valet UMETA(DisplayName = "Valet")
};

/**
 * ECU map parameters for fuel and ignition tuning
 */
USTRUCT(BlueprintType)
struct FMGECUMapParameters
{
	GENERATED_BODY()

	/** Map identifier name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString MapName = TEXT("Stock");

	/** Description of the map's characteristics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString MapDescription;

	// ==========================================
	// FUEL PARAMETERS
	// ==========================================

	/** Air-fuel ratio target (stoichiometric = 14.7, rich < 14.7, lean > 14.7) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuel", meta = (ClampMin = "10.0", ClampMax = "18.0"))
	float TargetAFR = 14.7f;

	/** Fuel enrichment at WOT (wide open throttle) - reduces AFR by this amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuel", meta = (ClampMin = "0.0", ClampMax = "4.0"))
	float WOTEnrichment = 1.5f;

	/** Cold start enrichment multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuel", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float ColdStartEnrichment = 1.2f;

	/** Injector duty cycle limit (safety) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuel", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float MaxInjectorDutyCycle = 0.85f;

	/** Fuel cut on overrun (throttle closed, high RPM) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuel")
	bool bFuelCutOnOverrun = true;

	/** Fuel cut RPM threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuel", meta = (ClampMin = "1500", ClampMax = "5000"))
	int32 FuelCutRPM = 2000;

	// ==========================================
	// IGNITION PARAMETERS
	// ==========================================

	/** Base ignition timing advance (degrees BTDC) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ignition", meta = (ClampMin = "-10.0", ClampMax = "40.0"))
	float BaseTimingAdvance = 15.0f;

	/** Maximum ignition timing advance at peak */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ignition", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float MaxTimingAdvance = 35.0f;

	/** Timing retard under boost (degrees per PSI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ignition", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float BoostTimingRetard = 1.0f;

	/** Knock retard amount (degrees to pull when knock detected) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ignition", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float KnockRetardAmount = 3.0f;

	/** Rev limiter RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ignition", meta = (ClampMin = "4000", ClampMax = "12000"))
	int32 RevLimitRPM = 7000;

	/** Rev limiter type (hard = fuel cut, soft = ignition retard) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ignition")
	bool bHardRevLimiter = false;

	/** Two-step launch control RPM (0 = disabled) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ignition", meta = (ClampMin = "0", ClampMax = "8000"))
	int32 LaunchControlRPM = 0;

	// ==========================================
	// BOOST CONTROL (if turbo/supercharged)
	// ==========================================

	/** Target boost pressure (PSI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float TargetBoostPSI = 0.0f;

	/** Boost cut threshold (safety) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float BoostCutPSI = 25.0f;

	/** Wastegate duty cycle (0-1, higher = more boost) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WastegateDutyCycle = 0.5f;

	/** Anti-lag enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	bool bAntiLagEnabled = false;

	// ==========================================
	// PERFORMANCE EFFECTS
	// ==========================================

	/** Power multiplier from this map (1.0 = baseline) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.5", ClampMax = "1.3"))
	float PowerMultiplier = 1.0f;

	/** Fuel consumption multiplier (1.0 = baseline, higher = more fuel used) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.7", ClampMax = "2.0"))
	float FuelConsumptionMultiplier = 1.0f;

	/** Engine wear rate multiplier (aggressive maps wear faster) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float EngineWearMultiplier = 1.0f;

	/** Knock probability (0-1, higher with aggressive timing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float KnockProbability = 0.0f;

	/** Minimum fuel octane required (87, 91, 93, 100, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "87", ClampMax = "110"))
	int32 MinimumOctaneRequired = 87;

	/** Throttle response sharpness (0-1, higher = snappier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ThrottleResponse = 0.5f;
};

/**
 * Complete ECU configuration with multiple maps
 */
USTRUCT(BlueprintType)
struct FMGECUConfiguration
{
	GENERATED_BODY()

	/** ECU part ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName ECUID;

	/** ECU brand/name for display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString ECUBrand = TEXT("Stock ECU");

	/** Currently active map type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EMGECUMapType ActiveMapType = EMGECUMapType::Stock;

	/** Stock map (always available) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	FMGECUMapParameters StockMap;

	/** Economy map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	FMGECUMapParameters EconomyMap;

	/** Sport map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	FMGECUMapParameters SportMap;

	/** Performance map (may require supporting mods) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	FMGECUMapParameters PerformanceMap;

	/** Race map (requires premium fuel and supporting mods) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	FMGECUMapParameters RaceMap;

	/** Custom user-defined map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	FMGECUMapParameters CustomMap;

	/** Valet mode map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maps")
	FMGECUMapParameters ValetMap;

	/** Maps available for switching (depends on ECU upgrade level) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Availability")
	TArray<EMGECUMapType> AvailableMaps;

	/** Can real-time map switching be done while driving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bSupportsRealTimeMapSwitch = false;

	/** Has data logging capability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bHasDataLogging = false;

	/** Has knock detection sensors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bHasKnockDetection = true;

	/** Has closed-loop wideband AFR control */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bHasWidebandAFR = false;

	/** Has flex fuel support */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
	bool bSupportsFlexFuel = false;

	/**
	 * Get the active map parameters
	 */
	const FMGECUMapParameters& GetActiveMap() const
	{
		switch (ActiveMapType)
		{
			case EMGECUMapType::Economy: return EconomyMap;
			case EMGECUMapType::Sport: return SportMap;
			case EMGECUMapType::Performance: return PerformanceMap;
			case EMGECUMapType::Race: return RaceMap;
			case EMGECUMapType::Custom: return CustomMap;
			case EMGECUMapType::Valet: return ValetMap;
			default: return StockMap;
		}
	}
};

/**
 * Engine configuration
 */
USTRUCT(BlueprintType)
struct FMGEngineConfiguration
{
	GENERATED_BODY()

	// Base engine
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EngineBlockID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEngineType EngineType = EMGEngineType::I4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DisplacementCC = 2000;

	// Cylinder head
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CylinderHeadID;

	// Valvetrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CamshaftID;

	// Aspiration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName IntakeManifoldID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ThrottleBodyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AirFilterID;

	// Exhaust
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ExhaustManifoldID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ExhaustSystemID;

	// Rotating assembly
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PistonsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ConnectingRodsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrankshaftID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FlywheelID;

	// Fuel system
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FuelInjectorsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FuelPumpID;

	// Ignition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SparkPlugsID;

	// ECU - Full configuration with tuning maps
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGECUConfiguration ECU;

	/** Legacy tune level (deprecated - use ECU.ActiveMapType instead) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TuneLevel = 0; // 0=Stock, 1=Stage1, 2=Stage2, 3=Custom

	// Forced induction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGForcedInductionConfig ForcedInduction;

	// Nitrous
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGNitrousConfig Nitrous;

	// ==========================================
	// PART TIERS (for stat calculation)
	// ==========================================

	/** Air filter upgrade tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiers")
	EMGPartTier AirFilterTier = EMGPartTier::Stock;

	/** Exhaust system upgrade tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiers")
	EMGPartTier ExhaustTier = EMGPartTier::Stock;

	/** Camshaft upgrade tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiers")
	EMGPartTier CamshaftTier = EMGPartTier::Stock;

	/** Engine internals (pistons/rods) upgrade tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiers")
	EMGPartTier InternalsTier = EMGPartTier::Stock;
};

/**
 * Drivetrain configuration
 */
USTRUCT(BlueprintType)
struct FMGDrivetrainConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDrivetrainType DrivetrainType = EMGDrivetrainType::RWD;

	// Clutch
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClutchID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClutchTorqueCapacity = 400.0f;

	// Transmission
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TransmissionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTransmissionType TransmissionType = EMGTransmissionType::Manual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GearCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> GearRatios;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverseGearRatio = -3.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShiftTimeSeconds = 0.15f;

	// Final drive
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalDriveRatio = 3.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDifferentialType DifferentialType = EMGDifferentialType::Open;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DifferentialID;

	// Driveshaft
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DriveshaftID;

	FMGDrivetrainConfiguration()
	{
		// Default 6-speed ratios
		GearRatios.Add(3.2f);  // 1st
		GearRatios.Add(2.05f); // 2nd
		GearRatios.Add(1.45f); // 3rd
		GearRatios.Add(1.05f); // 4th
		GearRatios.Add(0.80f); // 5th
		GearRatios.Add(0.65f); // 6th
	}
};

/**
 * Suspension configuration
 */
USTRUCT(BlueprintType)
struct FMGSuspensionConfiguration
{
	GENERATED_BODY()

	// Front
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrontSpringsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontSpringRate = 300.0f; // lbs/in

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrontDampersID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontCompressionDamping = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontReboundDamping = 4500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrontSwayBarID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontSwayBarStiffness = 1.0f;

	// Rear
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RearSpringsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearSpringRate = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RearDampersID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearCompressionDamping = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearReboundDamping = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RearSwayBarID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearSwayBarStiffness = 0.8f;

	// Geometry
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontCamber = -1.0f; // degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearCamber = -0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontToe = 0.0f; // degrees, positive = toe-in

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearToe = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RideHeightOffsetMM = 0.0f; // negative = lower

	// Ride height per axle (mm from ground to chassis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontRideHeightMM = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearRideHeightMM = 160.0f;
};

/**
 * Brake configuration
 */
USTRUCT(BlueprintType)
struct FMGBrakeConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrontRotorsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontRotorDiameterMM = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrontCalipersID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrontPistonCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrontPadsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RearRotorsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearRotorDiameterMM = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RearCalipersID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RearPistonCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RearPadsID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BrakeLinesID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakeBias = 0.65f; // 0.0 = all rear, 1.0 = all front

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasHydraulicHandbrake = false;
};

/**
 * Wheel and tire configuration
 */
USTRUCT(BlueprintType)
struct FMGWheelTireConfiguration
{
	GENERATED_BODY()

	// Front wheels
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrontWheelID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrontWheelDiameter = 18; // inches

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontWheelWidth = 8.5f; // inches

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrontWheelOffset = 35; // mm

	// Front tires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrontTireWidth = 245; // mm

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrontTireAspectRatio = 40; // percent

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCompound FrontTireCompound = EMGTireCompound::Sport;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontTireCondition = 100.0f; // percent

	// Rear wheels
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RearWheelID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RearWheelDiameter = 18;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearWheelWidth = 9.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RearWheelOffset = 38;

	// Rear tires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RearTireWidth = 275;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RearTireAspectRatio = 35;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCompound RearTireCompound = EMGTireCompound::Sport;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearTireCondition = 100.0f;
};

/**
 * Front splitter configuration
 */
USTRUCT(BlueprintType)
struct FMGFrontSplitterConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInstalled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SplitterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceCoefficient = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceLevelPercent = 50.0f; // 0-100
};

/**
 * Rear wing configuration
 */
USTRUCT(BlueprintType)
struct FMGRearWingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInstalled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WingID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceCoefficient = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceLevelPercent = 50.0f; // 0-100, adjustable angle

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WingAngle = 0.0f; // degrees
};

/**
 * Aerodynamic configuration
 */
USTRUCT(BlueprintType)
struct FMGAeroConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGFrontSplitterConfig FrontSplitter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRearWingConfig RearWing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DiffuserID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiffuserDownforceCoefficient = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragCoefficient = 0.32f;
};

/**
 * Calculated vehicle statistics
 */
USTRUCT(BlueprintType)
struct FMGVehicleStats
{
	GENERATED_BODY()

	// Power
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Horsepower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Torque = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BoostPSI = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Redline = 7000;

	// Weight
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WeightKG = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WeightDistributionFront = 0.55f; // 0-1, front percentage

	// Performance
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PowerToWeightRatio = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ZeroTo60MPH = 0.0f; // seconds

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ZeroTo100MPH = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float QuarterMileTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float QuarterMileTrapMPH = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TopSpeedMPH = 0.0f;

	// Handling
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GripFront = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GripRear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HandlingRating = 0.0f; // 0-100

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BrakingRating = 0.0f; // 0-100

	// Classification
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PerformanceIndex = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMGPerformanceClass PerformanceClass = EMGPerformanceClass::D;

	// Economy
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float EstimatedValue = 0.0f;

	// Reliability
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ReliabilityRating = 100.0f; // 0-100
};

/**
 * Race history for a vehicle
 */
USTRUCT(BlueprintType)
struct FMGRaceHistory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Podiums = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PinkSlipWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PinkSlipLosses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalEarnings = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestQuarterMile = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTopSpeed = 0.0f;
};

/**
 * Complete vehicle instance data
 */
USTRUCT(BlueprintType)
struct FMGVehicleData
{
	GENERATED_BODY()

	// Identification
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VIN; // Unique identifier string

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BaseModelID; // Reference to vehicle model data asset

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	// Ownership
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CurrentOwnerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGOwnershipRecord> OwnershipHistory;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEngineConfiguration Engine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDrivetrainConfiguration Drivetrain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGSuspensionConfiguration Suspension;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGBrakeConfiguration Brakes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWheelTireConfiguration WheelsTires;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGAeroConfiguration Aero;

	// Calculated stats (updated when configuration changes)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMGVehicleStats Stats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMGPowerCurve PowerCurve;

	// Condition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> PartConditions; // PartID -> Condition (0-100)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Mileage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AccidentCount = 0;

	// History
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRaceHistory RaceHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime DateAcquired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRaced;

	FMGVehicleData()
	{
		VehicleID = FGuid::NewGuid();
		DateAcquired = FDateTime::Now();
	}
};

/**
 * Base vehicle model definition (Data Asset)
 * Defines the base specs for a vehicle type before customization
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleModelData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Identification
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName ModelID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Manufacturer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	int32 Year = 1999;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Description;

	// Base specifications
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specifications")
	EMGEngineType BaseEngineType = EMGEngineType::I4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specifications")
	int32 BaseDisplacementCC = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specifications")
	float BaseHorsepower = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specifications")
	float BaseTorque = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specifications")
	int32 BaseRedline = 7000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specifications")
	EMGDrivetrainType BaseDrivetrain = EMGDrivetrainType::RWD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specifications")
	float BaseWeightKG = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specifications")
	float BaseWeightDistributionFront = 0.55f;

	// Pricing
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 BasePriceMSRP = 25000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float DepreciationRate = 0.15f; // Per year

	// Assets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TSoftObjectPtr<USkeletalMesh> VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TSoftClassPtr<AActor> VehicleBlueprintClass;

	// Customization compatibility
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TArray<FName> CompatibleEngineFamilies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TArray<FName> CompatibleBodyKits;

	// Power curve baseline
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	FMGPowerCurve BasePowerCurve;
};

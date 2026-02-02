// Copyright Midnight Grind. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "MG_VHCL_Data.generated.h"

// EMGPerformanceClass - REMOVED (duplicate)
// Canonical definition in: Catalog/MGCatalogTypes.h

// EMGPartTier - REMOVED (duplicate)
// Canonical definition in: Catalog/MGCatalogTypes.h

// EMGEngineType - REMOVED (duplicate)
// Canonical definition in: EngineAudio/MGEngineAudioSubsystem.h

// EMGDrivetrainType - REMOVED (duplicate)
// Canonical definition in: Core/MGCoreEnums.h

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

// EMGTransmissionType - REMOVED (duplicate)
// Canonical definition in: Data/MGVehicleDatabase.h

// EMGTireCompound - REMOVED (duplicate)
// Canonical definition in: PitStop/MGPitStopSubsystem.h

// ==========================================
// DATA STRUCTURES
// ==========================================

// EMGFuelType - REMOVED (duplicate)
// Canonical definition in: Fuel/MG_FUEL_Subsystem.h

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
 * @brief Forced induction (turbo/supercharger) configuration
 *
 * Defines the boost system configuration for turbocharged or supercharged engines.
 * Used by the engine simulation to calculate boost-related power gains.
 */
USTRUCT(BlueprintType)
struct FMGForcedInductionConfig
{
	GENERATED_BODY()

	/** Type of forced induction installed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Induction")
	EMGForcedInductionType Type = EMGForcedInductionType::None;

	/** Target boost pressure in PSI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Induction", meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float TargetBoostPSI = 0.0f;

	/** Maximum safe boost pressure in PSI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Induction", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float MaxBoostPSI = 15.0f;

	/** Turbo spool time in seconds (turbo lag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Induction", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float SpoolTime = 0.5f;

	/** Intercooler efficiency (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Induction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IntercoolerEfficiency = 0.7f;

	/** Whether blow-off valve is recirculating (quieter) or atmospheric (loud) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Induction")
	bool bRecirculatingBOV = true;

	/** Anti-lag system enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forced Induction")
	bool bAntiLagEnabled = false;
};

/**
 * @brief Nitrous oxide system configuration
 *
 * Defines the NOS setup for temporary power boosts.
 * Nitrous provides a significant but limited power increase.
 */
USTRUCT(BlueprintType)
struct FMGNitrousConfig
{
	GENERATED_BODY()

	/** Whether nitrous is installed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous")
	bool bInstalled = false;

	/** Bottle capacity in pounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float BottleCapacityLbs = 10.0f;

	/** Current bottle fill level (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurrentFillLevel = 1.0f;

	/** Shot size - horsepower gain when activated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous", meta = (ClampMin = "25", ClampMax = "500"))
	int32 ShotSizeHP = 75;

	/** Bottle pressure in PSI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous", meta = (ClampMin = "800", ClampMax = "1200"))
	int32 BottlePressure = 950;

	/** Minimum RPM for activation (safety) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous", meta = (ClampMin = "2000", ClampMax = "8000"))
	int32 MinActivationRPM = 3500;

	/** Wet shot (fuel enriched) vs dry shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous")
	bool bWetShot = true;

	/** Progressive controller enabled (ramps power instead of instant) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous")
	bool bProgressiveController = false;

	/** Purge system enabled (for show and line clearing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitrous")
	bool bPurgeEnabled = true;
};

/**
 * @brief Power curve data point
 *
 * A single point on the power/torque curve at a specific RPM.
 */
USTRUCT(BlueprintType)
struct FMGPowerCurvePoint
{
	GENERATED_BODY()

	/** Engine RPM for this data point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	int32 RPM = 0;

	/** Horsepower at this RPM */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	float Horsepower = 0.0f;

	/** Torque in lb-ft at this RPM */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	float TorqueLbFt = 0.0f;
};

/**
 * @brief Complete power curve for an engine
 *
 * Contains all data points from idle to redline, plus peak values.
 * Generated from engine configuration by the stat calculator.
 */
USTRUCT(BlueprintType)
struct FMGPowerCurve
{
	GENERATED_BODY()

	/** All data points on the curve */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	TArray<FMGPowerCurvePoint> DataPoints;

	/** Peak horsepower value */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	float PeakHorsepower = 0.0f;

	/** RPM at peak horsepower */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	int32 PeakHorsepowerRPM = 0;

	/** Peak torque value in lb-ft */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	float PeakTorque = 0.0f;

	/** RPM at peak torque */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	int32 PeakTorqueRPM = 0;

	/** Idle RPM */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	int32 IdleRPM = 800;

	/** Redline RPM */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power Curve")
	int32 RedlineRPM = 7000;
};

/**
 * @brief Vehicle ownership history record
 *
 * Tracks previous owners, purchase dates, and mileage at transfer.
 * Used for vehicle history reports and provenance.
 */
USTRUCT(BlueprintType)
struct FMGOwnershipRecord
{
	GENERATED_BODY()

	/** Player ID of the owner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	FGuid OwnerID;

	/** Display name of the owner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	FString OwnerName;

	/** Date of acquisition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	FDateTime AcquiredDate;

	/** Date of sale/transfer (invalid if current owner) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	FDateTime SoldDate;

	/** Mileage when acquired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	int32 MileageAtAcquisition = 0;

	/** Mileage when sold (0 if current owner) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	int32 MileageAtSale = 0;

	/** Purchase price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	int32 PurchasePrice = 0;

	/** Sale price (0 if current owner or gifted) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ownership")
	int32 SalePrice = 0;
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
	// FUEL SYSTEM
	// ==========================================

	/**
	 * @brief Fuel tank configuration for consumption simulation
	 *
	 * Defines tank capacity, current fuel level, and consumption parameters.
	 * Used by UMGFuelConsumptionComponent for realistic fuel simulation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fuel")
	FMGFuelTankConfig FuelTank;

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
 * Vehicle performance specifications - calculated from parts and tuning.
 *
 * This struct holds the computed performance metrics for a vehicle based on
 * its installed parts and tuning configuration. Updated when configuration changes.
 *
 * NOTE: This is distinct from FMGVehicleRacingStats (defined in Stats/MGStatsTracker.h)
 * which tracks per-vehicle racing history (races entered, wins, best times, etc.)
 *
 * @see FMGVehicleRacingStats for per-vehicle racing history
 * @see UMGTuningSubsystem for stat calculation
 */
USTRUCT(BlueprintType)
struct FMGVehicleSpecs
{
	GENERATED_BODY()

	// Power
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power")
	float Horsepower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power")
	float Torque = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power")
	float BoostPSI = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power")
	int32 Redline = 7000;

	// Weight
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weight")
	float WeightKG = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weight")
	float WeightDistributionFront = 0.55f; // 0-1, front percentage

	// Performance
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float PowerToWeightRatio = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float ZeroTo60MPH = 0.0f; // seconds

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float ZeroTo100MPH = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float QuarterMileTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float QuarterMileTrapMPH = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float TopSpeedMPH = 0.0f;

	// Handling
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float GripFront = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float GripRear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float HandlingRating = 0.0f; // 0-100

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Handling")
	float BrakingRating = 0.0f; // 0-100

	// Classification
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	float PerformanceIndex = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EMGPerformanceClass PerformanceClass = EMGPerformanceClass::D;

	// Economy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float EstimatedValue = 0.0f;

	// Reliability
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reliability")
	float ReliabilityRating = 100.0f; // 0-100
};

// Legacy typedef for backward compatibility during migration
// TODO: Remove after all usages are updated to FMGVehicleSpecs
typedef FMGVehicleSpecs FMGVehicleStats;

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

	// Calculated specs (updated when configuration changes)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMGVehicleSpecs Specs;

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

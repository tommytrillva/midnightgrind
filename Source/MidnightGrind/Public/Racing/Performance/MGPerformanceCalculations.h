// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGPerformanceCalculations.h
 * @brief Pure C++ vehicle performance calculation utilities
 * 
 * @namespace MidnightGrind::Racing::Performance
 * This namespace contains pure calculation logic for vehicle performance metrics.
 * All functions are static and free of UObject dependencies, making them:
 * - Easy to unit test
 * - Reusable from any C++ code
 * - Optimizable by the compiler
 * 
 * These functions are called by UMGStatCalculator which provides Blueprint wrappers.
 * 
 * Created: Phase 2 - Hybrid Namespacing Refactor
 */

#include "CoreMinimal.h"
#include "Vehicle/MG_VHCL_Data.h"

// Forward declarations for UObjects we reference
class UMGVehicleModelData;

/**
 * @namespace MidnightGrind::Racing::Performance
 * @brief Vehicle performance calculation utilities
 * 
 * Pure C++ functions for calculating vehicle stats from configuration data.
 * Used by UMGStatCalculator Blueprint function library and other C++ systems.
 */
namespace MidnightGrind::Racing::Performance
{
	/**
	 * @class FPowerCalculator
	 * @brief Static utility class for engine power calculations
	 * 
	 * Calculates horsepower, torque, and full power curves from engine configuration.
	 * Considers base specs, installed parts, forced induction, and tune level.
	 */
	class MIDNIGHTGRIND_API FPowerCalculator
	{
	public:
		/**
		 * @brief Calculate total peak horsepower from engine configuration
		 * 
		 * Considers:
		 * - Base model specifications
		 * - Installed parts (intake, exhaust, cams, internals)
		 * - Forced induction (turbo/supercharger)
		 * - Tune level (street/race/competition)
		 * 
		 * @param Engine The engine configuration to evaluate
		 * @param BaseModel The base vehicle model data (provides stock specs)
		 * @return Peak horsepower at wheels
		 */
		static float CalculateHorsepower(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

		/**
		 * @brief Calculate total peak torque from engine configuration
		 * 
		 * Similar to horsepower calculation but for torque output.
		 * Torque peaks earlier in RPM range than horsepower.
		 * 
		 * @param Engine The engine configuration to evaluate
		 * @param BaseModel The base vehicle model data
		 * @return Peak torque in lb-ft at wheels
		 */
		static float CalculateTorque(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

		/**
		 * @brief Generate complete power/torque curve across RPM range
		 * 
		 * Creates a full dyno-style power curve with multiple data points
		 * from idle to redline. Useful for:
		 * - Dyno UI visualization
		 * - Optimal shift point calculation
		 * - Gear ratio optimization
		 * 
		 * @param Engine The engine configuration to evaluate
		 * @param BaseModel The base vehicle model data
		 * @return Complete power curve structure with all data points
		 */
		static FMGPowerCurve CalculatePowerCurve(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

		/**
		 * @brief Calculate engine redline RPM based on upgrades
		 * 
		 * Upgraded internals (forged pistons, rods, crankshaft) allow higher safe RPM.
		 * Stock internals: typically 6000-8000 RPM
		 * Race internals: can push 9000+ RPM
		 * 
		 * @param Engine The engine configuration to evaluate
		 * @param BaseModel The base vehicle model data
		 * @return Maximum safe RPM before rev limiter
		 */
		static int32 CalculateRedline(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	private:
		/** Calculate power multiplier from all installed parts */
		static float GetCombinedPowerMultiplier(const FMGEngineConfiguration& Engine);
		
		/** Calculate torque multiplier from all installed parts */
		static float GetCombinedTorqueMultiplier(const FMGEngineConfiguration& Engine);
		
		/** Calculate boost contribution to power (if forced induction equipped) */
		static float GetBoostPowerContribution(const FMGEngineConfiguration& Engine);
		
		/** Calculate tune level multiplier */
		static float GetTuneMultiplier(EMGTuneLevel TuneLevel);
	};

	/**
	 * @class FWeightCalculator
	 * @brief Static utility class for weight calculations
	 * 
	 * Calculates total weight and weight distribution considering all modifications.
	 */
	class MIDNIGHTGRIND_API FWeightCalculator
	{
	public:
		/**
		 * @brief Calculate total vehicle weight including all modifications
		 * 
		 * Sums base vehicle weight with all part weight deltas:
		 * - Weight reductions: carbon fiber body panels, lightweight wheels, battery, exhaust
		 * - Weight additions: roll cage, intercooler, sound system, NOS bottles
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @param BaseModel The base vehicle model data
		 * @return Total weight in kilograms
		 */
		static float CalculateTotalWeight(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

		/**
		 * @brief Calculate front-to-rear weight distribution
		 * 
		 * Considers:
		 * - Drivetrain type (FWD pushes weight forward, RWD pushes back)
		 * - Engine position (front/mid/rear)
		 * - Heavy modifications affecting balance (FMIC, rear seat delete, fuel cell)
		 * - Driver weight (assumed 75kg in front)
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @param BaseModel The base vehicle model data
		 * @return Front axle weight percentage (0.0 to 1.0, e.g., 0.55 = 55% front / 45% rear)
		 */
		static float CalculateWeightDistribution(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	private:
		/** Sum weight deltas from all installed parts */
		static float GetPartsWeightDelta(const FMGVehicleData& Vehicle);
		
		/** Calculate weight distribution shift from modifications */
		static float GetWeightDistributionShift(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);
	};

	/**
	 * @class FHandlingCalculator
	 * @brief Static utility class for handling performance calculations
	 * 
	 * Calculates grip coefficients and handling ratings.
	 */
	class MIDNIGHTGRIND_API FHandlingCalculator
	{
	public:
		/**
		 * @brief Calculate front axle grip coefficient
		 * 
		 * Based on:
		 * - Front tire compound (street/sport/race/slick)
		 * - Tire width (wider = more grip)
		 * - Suspension setup (geometry, springs, dampers)
		 * - Alignment settings (camber, toe)
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @return Front grip coefficient (typically 0.8 to 1.2)
		 */
		static float CalculateFrontGrip(const FMGVehicleData& Vehicle);

		/**
		 * @brief Calculate rear axle grip coefficient
		 * 
		 * Based on rear tire compound, width, and suspension setup.
		 * Higher values mean more rear grip (less oversteer tendency).
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @return Rear grip coefficient (typically 0.8 to 1.2)
		 */
		static float CalculateRearGrip(const FMGVehicleData& Vehicle);

		/**
		 * @brief Calculate overall handling rating for UI display
		 * 
		 * Composite rating considering:
		 * - Grip balance (understeer/oversteer tendency)
		 * - Weight distribution
		 * - Suspension tuning quality
		 * - Aerodynamic downforce
		 * - Power-to-weight ratio (too much power = harder to handle)
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @param BaseModel The base vehicle model data
		 * @return Handling rating from 0 (terrible) to 100 (perfect race car)
		 */
		static float CalculateHandlingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

		/**
		 * @brief Calculate braking performance rating for UI display
		 * 
		 * Based on:
		 * - Brake rotor size (mm)
		 * - Caliper piston count (4/6/8+ pistons)
		 * - Pad compound (street/sport/race)
		 * - Vehicle weight (heavier = needs more braking force)
		 * - Tire grip (can't brake harder than tires can grip)
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @param BaseModel The base vehicle model data
		 * @return Braking rating from 0 (poor) to 100 (race brakes)
		 */
		static float CalculateBrakingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	private:
		/** Get base grip coefficient for tire compound */
		static float GetTireCompoundGrip(EMGTireCompound Compound);
		
		/** Calculate suspension contribution to grip */
		static float GetSuspensionGripModifier(const FMGSuspensionConfiguration& Suspension);
		
		/** Calculate alignment contribution to grip (camber/toe) */
		static float GetAlignmentGripModifier(float Camber, float Toe);
	};

	/**
	 * @class FPerformancePredictor
	 * @brief Static utility class for predicting acceleration and top speed
	 * 
	 * Uses physics models to estimate real-world performance numbers.
	 */
	class MIDNIGHTGRIND_API FPerformancePredictor
	{
	public:
		/**
		 * @brief Estimate 0-60 MPH acceleration time
		 * 
		 * Uses simplified physics model:
		 * - Power-to-weight ratio
		 * - Drivetrain efficiency (AWD > RWD > FWD for launches)
		 * - Tire grip (limits launch acceleration)
		 * - Turbo lag consideration
		 * 
		 * @param Stats Pre-calculated vehicle statistics
		 * @param Drivetrain Drivetrain configuration for traction/efficiency
		 * @return Estimated time in seconds (e.g., 4.5 seconds)
		 */
		static float EstimateZeroTo60(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain);

		/**
		 * @brief Estimate 0-100 MPH acceleration time
		 * 
		 * Similar to 0-60 but accounts for:
		 * - Gearing (needs 2-3 shifts typically)
		 * - Aerodynamic drag at higher speeds
		 * - Power curve characteristics
		 * 
		 * @param Stats Pre-calculated vehicle statistics
		 * @param Drivetrain Drivetrain configuration
		 * @return Estimated time in seconds
		 */
		static float EstimateZeroTo100(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain);

		/**
		 * @brief Estimate quarter mile elapsed time
		 * 
		 * Classic drag racing metric. Accounts for:
		 * - Launch traction (60ft time heavily influences ET)
		 * - Power delivery through mid-range
		 * - Gearing optimization (want to cross finish at redline in top gear)
		 * 
		 * @param Stats Pre-calculated vehicle statistics
		 * @param Drivetrain Drivetrain configuration
		 * @return Estimated ET in seconds (e.g., 11.5 seconds)
		 */
		static float EstimateQuarterMile(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain);

		/**
		 * @brief Estimate quarter mile trap speed
		 * 
		 * Speed achieved at the finish line (1320 feet).
		 * Indicates sustained acceleration capability and power delivery.
		 * 
		 * @param Stats Pre-calculated vehicle statistics
		 * @return Estimated trap speed in MPH
		 */
		static float EstimateQuarterMileTrap(const FMGVehicleSpecs& Stats);

		/**
		 * @brief Estimate theoretical top speed
		 * 
		 * Top speed is reached when:
		 * - Engine power = Aerodynamic drag + rolling resistance
		 * 
		 * Considers:
		 * - Power output at redline
		 * - Drag coefficient and frontal area
		 * - Final drive ratio and tire diameter
		 * - Altitude/air density (sea level assumed)
		 * 
		 * @param Stats Pre-calculated vehicle statistics
		 * @param Drivetrain Drivetrain config for final drive ratio
		 * @param Aero Aerodynamic config for drag coefficient
		 * @return Estimated top speed in MPH
		 */
		static float EstimateTopSpeed(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain, const FMGAeroConfiguration& Aero);

	private:
		/** Calculate drivetrain efficiency for different types */
		static float GetDrivetrainEfficiency(EMGDrivetrainType DrivetrainType);
		
		/** Calculate launch traction coefficient */
		static float GetLaunchTractionCoefficient(EMGDrivetrainType DrivetrainType, float RearGrip);
		
		/** Estimate 60-foot time (launch performance) */
		static float EstimateSixtyFootTime(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain);
	};

	/**
	 * @class FPerformanceIndexCalculator
	 * @brief Static utility class for PI calculation and classification
	 * 
	 * Performance Index is a composite score for matchmaking and class restrictions.
	 */
	class MIDNIGHTGRIND_API FPerformanceIndexCalculator
	{
	public:
		/**
		 * @brief Calculate Performance Index (PI) from vehicle stats
		 * 
		 * PI formula balances:
		 * - Power (30% weight)
		 * - Weight (20% weight)
		 * - Handling (25% weight)
		 * - Braking (15% weight)
		 * - Acceleration (10% weight)
		 * 
		 * Result is normalized to 100-999 range.
		 * Higher PI = faster/more capable vehicle.
		 * 
		 * @param Stats Pre-calculated vehicle statistics
		 * @return Performance Index value (typically 100-999)
		 */
		static float CalculatePI(const FMGVehicleSpecs& Stats);

		/**
		 * @brief Determine performance class from PI value
		 * 
		 * Maps PI to letter classes:
		 * - D: 100-399 (entry-level)
		 * - C: 400-499 (street)
		 * - B: 500-599 (sport)
		 * - A: 600-699 (super)
		 * - S: 700-799 (supercar)
		 * - S+: 800-899 (elite)
		 * - Hyper: 900-998 (hypercar)
		 * - Legend: 999+ (unrestricted)
		 * 
		 * @param PI The calculated PI value
		 * @return Performance class enumeration
		 */
		static EMGPerformanceClass GetPerformanceClass(float PI);

	private:
		/** Component weights for PI calculation */
		static constexpr float POWER_WEIGHT = 0.30f;
		static constexpr float WEIGHT_WEIGHT = 0.20f;
		static constexpr float HANDLING_WEIGHT = 0.25f;
		static constexpr float BRAKING_WEIGHT = 0.15f;
		static constexpr float ACCELERATION_WEIGHT = 0.10f;

		/** PI normalization constants */
		static constexpr float PI_MIN = 100.0f;
		static constexpr float PI_MAX = 999.0f;
	};

	/**
	 * @class FValueCalculator
	 * @brief Static utility class for vehicle and parts valuation
	 * 
	 * Calculates monetary value considering depreciation, modifications, and prestige.
	 */
	class MIDNIGHTGRIND_API FValueCalculator
	{
	public:
		/**
		 * @brief Calculate estimated total vehicle value
		 * 
		 * Formula:
		 * Base Value (with depreciation)
		 * + Installed Parts Value (70% of retail - parts lose value when installed)
		 * + Race History Prestige Bonus
		 * - Damage/Wear Penalties
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @param BaseModel The base vehicle model data
		 * @return Estimated value in game currency
		 */
		static float CalculateVehicleValue(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

		/**
		 * @brief Calculate total value of all installed aftermarket parts
		 * 
		 * Sums the base cost of all non-stock parts currently installed.
		 * Does not include labor costs or installation fees.
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @return Total parts value in game currency
		 */
		static float CalculatePartsValue(const FMGVehicleData& Vehicle);

	private:
		/** Calculate depreciation multiplier based on mileage and age */
		static float GetDepreciationMultiplier(float Mileage, float Age);
		
		/** Calculate prestige bonus from race history */
		static float GetPrestigeBonus(const FMGVehicleData& Vehicle);
		
		/** Installed parts value modifier (parts lose value when installed) */
		static constexpr float INSTALLED_PARTS_VALUE_MULTIPLIER = 0.70f;
	};

	/**
	 * @class FReliabilityCalculator
	 * @brief Static utility class for reliability rating
	 * 
	 * Higher performance parts = lower reliability (require more maintenance).
	 */
	class MIDNIGHTGRIND_API FReliabilityCalculator
	{
	public:
		/**
		 * @brief Calculate vehicle reliability rating
		 * 
		 * Based on part quality tiers:
		 * - OEM/Stock parts: 100% reliable
		 * - Street parts: 90% reliable
		 * - Sport parts: 75% reliable
		 * - Race parts: 50% reliable
		 * - Pro parts: 30% reliable
		 * - Legendary parts: 10% reliable (prototype/experimental)
		 * 
		 * Lower reliability = more frequent maintenance required.
		 * 
		 * @param Vehicle Complete vehicle configuration data
		 * @return Reliability rating from 0 (unreliable) to 100 (stock reliability)
		 */
		static float CalculateReliability(const FMGVehicleData& Vehicle);

	private:
		/** Get reliability rating for part tier */
		static float GetTierReliability(EMGPartTier Tier);
	};

} // namespace MidnightGrind::Racing::Performance

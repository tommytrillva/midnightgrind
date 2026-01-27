// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGStatCalculator.h
 * @brief Static utility functions for calculating vehicle statistics.
 *
 * This file provides a Blueprint function library for computing vehicle
 * performance metrics from configuration data. It calculates:
 * - Power and torque from engine configuration
 * - Complete power curves (dyno simulation)
 * - Weight and weight distribution
 * - Handling and braking ratings
 * - Performance predictions (0-60, 1/4 mile, top speed)
 * - Performance Index (PI) for class balancing
 * - Vehicle and parts valuation
 *
 * All functions are static and can be called from Blueprint or C++.
 *
 * Example usage:
 * @code
 * // Calculate all stats for a vehicle
 * FMGVehicleStats Stats = UMGStatCalculator::CalculateAllStats(VehicleData, BaseModel);
 *
 * // Get specific calculations
 * float HP = UMGStatCalculator::CalculateHorsepower(VehicleData.Engine, BaseModel);
 * float PI = UMGStatCalculator::CalculatePerformanceIndex(Stats);
 * @endcode
 *
 * @see FMGVehicleData
 * @see FMGVehicleStats
 * @see UMGVehicleModelData
 */

#pragma once

#include "CoreMinimal.h"
#include "MGVehicleData.h"
#include "MGStatCalculator.generated.h"

/**
 * @brief Aggregate modifiers from installed parts affecting vehicle stats.
 *
 * When parts are installed, their individual modifiers are combined into
 * this structure to determine overall effects on vehicle performance.
 * Used internally by stat calculation functions.
 */
USTRUCT(BlueprintType)
struct FMGPartModifiers
{
	GENERATED_BODY()

	/** @brief Multiplier applied to base engine horsepower (1.0 = no change). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerMultiplier = 1.0f;

	/** @brief Multiplier applied to base engine torque (1.0 = no change). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueMultiplier = 1.0f;

	/** @brief Weight change in kilograms (positive = heavier, negative = lighter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeightDelta = 0.0f;

	/** @brief Additional RPM added to base redline from upgraded internals. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RedlineBonus = 0;

	/** @brief Maximum boost pressure this part configuration can support (PSI). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostCapacity = 0.0f;

	/** @brief Flow capacity rating (1.0 = stock, higher = better flow for fuel/air). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlowRating = 1.0f;

	/** @brief Part durability rating affecting wear rate (1.0 = stock, higher = more durable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurabilityRating = 1.0f;

	/** @brief Impact on reliability rating (negative = less reliable, positive = more reliable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReliabilityImpact = 0.0f;
};

/**
 * @brief Data asset defining an aftermarket part's properties and effects.
 *
 * This data asset represents a single aftermarket part that can be installed
 * on vehicles. It contains identity info, performance modifiers, compatibility
 * rules, and visual/audio assets.
 *
 * Create instances of this class in the Content Browser for each available part.
 *
 * @note Empty CompatibleVehicles array means the part is universal (fits all vehicles).
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGPartData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// IDENTITY
	// ==========================================

	/** @brief Unique identifier for this part, used in configuration references. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName PartID;

	/** @brief Human-readable name shown in UI (e.g., "HKS GT3 Turbo Kit"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** @brief Detailed description of the part for tooltips/info screens. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Description;

	/** @brief Brand/manufacturer name (e.g., "HKS", "Greddy", "Eibach"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Manufacturer;

	// ==========================================
	// CLASSIFICATION
	// ==========================================

	/** @brief Primary category (e.g., "Engine", "Exhaust", "Suspension", "Brakes"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	FName Category;

	/** @brief Sub-category within primary (e.g., "AirFilter", "Camshaft", "Turbo"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	FName SubCategory;

	/** @brief Quality tier affecting stats and price. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EMGPartTier Tier = EMGPartTier::Street;

	// ==========================================
	// ECONOMY
	// ==========================================

	/** @brief Base purchase price in game currency. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 BaseCost = 100;

	/** @brief Additional labor cost for professional installation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float InstallationCost = 0.0f;

	// ==========================================
	// STATS
	// ==========================================

	/** @brief Performance modifiers this part provides when installed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FMGPartModifiers Modifiers;

	// ==========================================
	// COMPATIBILITY
	// ==========================================

	/** @brief Vehicle model IDs this part fits. Empty array = universal fitment. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> CompatibleVehicles;

	/** @brief Parts that must be installed before this one (prerequisites). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> RequiredParts;

	/** @brief Parts that cannot be installed alongside this one (conflicts). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> IncompatibleParts;

	// ==========================================
	// ASSETS
	// ==========================================

	/** @brief Visual mesh for garage/customization preview (optional). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TSoftObjectPtr<UStaticMesh> VisualMesh;

	/** @brief Sound effect played when part is installed (optional). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TSoftObjectPtr<USoundBase> InstallSound;
};

/**
 * @brief Static utility class for calculating vehicle statistics and performance metrics.
 *
 * This Blueprint function library provides all calculations needed to derive
 * vehicle performance statistics from configuration data. It handles:
 *
 * - **Power Calculations**: HP, torque, and full dyno curves from engine config
 * - **Weight Calculations**: Total weight and front/rear distribution
 * - **Handling Calculations**: Grip coefficients and handling ratings
 * - **Performance Predictions**: 0-60, 0-100, 1/4 mile times, top speed
 * - **Performance Index**: PI calculation for race class balancing
 * - **Value Calculations**: Vehicle and parts monetary values
 *
 * All functions are static and can be called without instantiation.
 * Use CalculateAllStats() to compute all metrics at once.
 *
 * @see FMGVehicleData Input configuration data
 * @see FMGVehicleStats Output statistics structure
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGStatCalculator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==========================================
	// POWER CALCULATIONS
	// ==========================================

	/**
	 * @brief Calculate total peak horsepower from engine configuration.
	 *
	 * Considers base model specs, installed parts (intake, exhaust, cams, etc.),
	 * forced induction, and tune level to determine final power output.
	 *
	 * @param Engine The engine configuration to evaluate.
	 * @param BaseModel The base vehicle model data asset (provides stock specs).
	 * @return Peak horsepower at wheels.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateHorsepower(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	/**
	 * @brief Calculate total peak torque from engine configuration.
	 *
	 * Similar to horsepower calculation but for torque output.
	 *
	 * @param Engine The engine configuration to evaluate.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Peak torque in lb-ft at wheels.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateTorque(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	/**
	 * @brief Generate complete power/torque curve across RPM range.
	 *
	 * Creates a full dyno-style power curve with multiple data points
	 * from idle to redline, including peak values and their RPMs.
	 *
	 * @param Engine The engine configuration to evaluate.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Complete power curve structure with all data points.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Stats")
	static FMGPowerCurve CalculatePowerCurve(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	/**
	 * @brief Calculate engine redline RPM based on upgrades.
	 *
	 * Upgraded internals (pistons, rods, crank) allow higher safe RPM.
	 *
	 * @param Engine The engine configuration to evaluate.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Maximum safe RPM before rev limiter.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static int32 CalculateRedline(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	// ==========================================
	// WEIGHT CALCULATIONS
	// ==========================================

	/**
	 * @brief Calculate total vehicle weight including all modifications.
	 *
	 * Sums base vehicle weight with all part weight deltas (additions/reductions
	 * from lightweight components, roll cages, etc.).
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Total weight in kilograms.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateWeight(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	/**
	 * @brief Calculate front-to-rear weight distribution.
	 *
	 * Considers drivetrain type, engine position, and heavy modifications
	 * that affect balance (front mount intercoolers, rear seat delete, etc.).
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Front axle weight percentage (0.0 to 1.0, e.g., 0.55 = 55% front).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateWeightDistribution(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	// ==========================================
	// HANDLING CALCULATIONS
	// ==========================================

	/**
	 * @brief Calculate front axle grip coefficient.
	 *
	 * Based on front tire compound, width, and suspension setup.
	 * Higher values mean more front grip (less understeer).
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @return Front grip coefficient (typically 0.8 to 1.2).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateFrontGrip(const FMGVehicleData& Vehicle);

	/**
	 * @brief Calculate rear axle grip coefficient.
	 *
	 * Based on rear tire compound, width, and suspension setup.
	 * Higher values mean more rear grip (less oversteer).
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @return Rear grip coefficient (typically 0.8 to 1.2).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateRearGrip(const FMGVehicleData& Vehicle);

	/**
	 * @brief Calculate overall handling rating for display.
	 *
	 * Composite rating considering grip balance, weight distribution,
	 * suspension tuning, and aerodynamics.
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Handling rating from 0 (terrible) to 100 (race car).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateHandlingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	/**
	 * @brief Calculate braking performance rating for display.
	 *
	 * Based on brake rotor size, caliper piston count, pad compound,
	 * and vehicle weight.
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Braking rating from 0 (poor) to 100 (race brakes).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateBrakingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	/**
	 * @brief Calculate vehicle reliability rating based on part quality tiers.
	 *
	 * Higher performance parts (Race, Pro, Legendary) are less reliable than
	 * OEM or Street tier parts. Based on the principle that more extreme
	 * builds require more maintenance.
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @return Reliability rating from 0 (unreliable) to 100 (stock reliability).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateReliability(const FMGVehicleData& Vehicle);

	// ==========================================
	// PERFORMANCE PREDICTIONS
	// ==========================================

	/**
	 * @brief Estimate 0-60 MPH acceleration time.
	 *
	 * Uses power-to-weight ratio, drivetrain efficiency, and grip to
	 * predict acceleration performance. AWD gets traction bonus.
	 *
	 * @param Stats Pre-calculated vehicle statistics.
	 * @param Drivetrain Drivetrain configuration for efficiency/traction.
	 * @return Estimated time in seconds (e.g., 4.5 seconds).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateZeroTo60(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain);

	/**
	 * @brief Estimate 0-100 MPH acceleration time.
	 *
	 * Similar to 0-60 but accounts for gearing and aerodynamic drag
	 * at higher speeds.
	 *
	 * @param Stats Pre-calculated vehicle statistics.
	 * @param Drivetrain Drivetrain configuration.
	 * @return Estimated time in seconds.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateZeroTo100(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain);

	/**
	 * @brief Estimate quarter mile elapsed time.
	 *
	 * Classic drag racing metric. Accounts for launch traction,
	 * power delivery, and gearing optimization.
	 *
	 * @param Stats Pre-calculated vehicle statistics.
	 * @param Drivetrain Drivetrain configuration.
	 * @return Estimated ET in seconds (e.g., 11.5 seconds).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateQuarterMile(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain);

	/**
	 * @brief Estimate quarter mile trap (finish line) speed.
	 *
	 * Speed achieved at the end of the 1/4 mile, indicates
	 * sustained acceleration capability.
	 *
	 * @param Stats Pre-calculated vehicle statistics.
	 * @return Estimated trap speed in MPH.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateQuarterMileTrap(const FMGVehicleStats& Stats);

	/**
	 * @brief Estimate theoretical top speed.
	 *
	 * Considers power output, aerodynamic drag, final drive ratio,
	 * and tire diameter to find terminal velocity.
	 *
	 * @param Stats Pre-calculated vehicle statistics.
	 * @param Drivetrain Drivetrain configuration for final drive ratio.
	 * @param Aero Aerodynamic configuration for drag coefficient.
	 * @return Estimated top speed in MPH.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateTopSpeed(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain, const FMGAeroConfiguration& Aero);

	// ==========================================
	// PERFORMANCE INDEX
	// ==========================================

	/**
	 * @brief Calculate Performance Index (PI) from vehicle stats.
	 *
	 * PI is a composite score balancing power, weight, handling, and braking
	 * into a single number for matchmaking and race class restrictions.
	 * Higher PI = faster/more capable vehicle.
	 *
	 * @param Stats Pre-calculated vehicle statistics.
	 * @return Performance Index value (typically 100-999).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculatePerformanceIndex(const FMGVehicleStats& Stats);

	/**
	 * @brief Determine performance class from PI value.
	 *
	 * Maps the numeric PI to a letter class (D through X) for
	 * race categorization and UI display.
	 *
	 * @param PerformanceIndex The calculated PI value.
	 * @return Performance class enumeration value.
	 *
	 * @see EMGPerformanceClass for class definitions and PI ranges.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static EMGPerformanceClass GetPerformanceClass(float PerformanceIndex);

	// ==========================================
	// VALUE CALCULATIONS
	// ==========================================

	/**
	 * @brief Calculate estimated total vehicle value.
	 *
	 * Combines base vehicle value (with depreciation), installed parts value,
	 * condition modifiers, and race history prestige.
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Estimated value in game currency.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateVehicleValue(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	/**
	 * @brief Calculate total value of all installed aftermarket parts.
	 *
	 * Sums the base cost of all non-stock parts currently installed.
	 * Does not include labor costs.
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @return Total parts value in game currency.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculatePartsValue(const FMGVehicleData& Vehicle);

	// ==========================================
	// COMPLETE STATS CALCULATION
	// ==========================================

	/**
	 * @brief Calculate all statistics for a vehicle in one call.
	 *
	 * Comprehensive function that computes all performance metrics and
	 * populates a complete FMGVehicleStats structure. This is the primary
	 * entry point for stat calculation.
	 *
	 * Should be called when:
	 * - Vehicle is first loaded/created
	 * - After any modification is installed
	 * - Before race matchmaking
	 *
	 * @param Vehicle Complete vehicle configuration data.
	 * @param BaseModel The base vehicle model data asset.
	 * @return Fully populated vehicle statistics structure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Stats")
	static FMGVehicleStats CalculateAllStats(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	// ==========================================
	// UTILITIES
	// ==========================================

	/**
	 * @brief Retrieve part data asset by its unique ID.
	 *
	 * Looks up the UMGPartData asset from the asset registry using
	 * the part's FName identifier.
	 *
	 * @param PartID Unique identifier of the part to find.
	 * @return Pointer to the part data asset, or nullptr if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Parts")
	static UMGPartData* GetPartData(FName PartID);

	/**
	 * @brief Aggregate modifiers from all parts in an engine configuration.
	 *
	 * Combines the modifiers from intake, exhaust, cams, internals, etc.
	 * into a single FMGPartModifiers structure for calculation use.
	 *
	 * @param Engine The engine configuration containing part references.
	 * @return Combined modifiers from all installed engine parts.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Parts")
	static FMGPartModifiers GetCombinedModifiers(const FMGEngineConfiguration& Engine);

	/**
	 * @brief Check if a part can be installed on a specific vehicle.
	 *
	 * Validates vehicle compatibility, prerequisite parts, and
	 * conflict checks against currently installed parts.
	 *
	 * @param Part The part data to check compatibility for.
	 * @param VehicleModelID The vehicle model ID to check against.
	 * @param Vehicle Current vehicle configuration (for prerequisite/conflict checks).
	 * @return True if the part can be installed, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Parts")
	static bool IsPartCompatible(const UMGPartData* Part, FName VehicleModelID, const FMGVehicleData& Vehicle);

	/**
	 * @brief Get base grip coefficient for a tire compound type.
	 *
	 * Returns the inherent grip multiplier for each compound,
	 * used as a base for grip calculations.
	 *
	 * @param Compound The tire compound type to query.
	 * @return Grip coefficient (typically 0.7 to 1.3).
	 *
	 * @note Higher values = more grip. Slicks have highest, Economy lowest.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float GetTireCompoundGrip(EMGTireCompound Compound);

	/**
	 * @brief Get grip reduction modifier for wet conditions.
	 *
	 * Returns how much grip is retained when driving on wet surfaces.
	 * Treaded tires perform better in wet than slicks.
	 *
	 * @param Compound The tire compound type to query.
	 * @return Wet grip multiplier (0.0 to 1.0, applied to base grip).
	 *
	 * @note Slicks have very low wet grip, All-Season has best wet performance.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float GetWetGripModifier(EMGTireCompound Compound);
};

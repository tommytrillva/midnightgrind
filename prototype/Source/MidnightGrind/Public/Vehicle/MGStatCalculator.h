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
	 * Calculate front grip coefficient
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateFrontGrip(const FMGVehicleData& Vehicle);

	/**
	 * Calculate rear grip coefficient
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateRearGrip(const FMGVehicleData& Vehicle);

	/**
	 * Calculate overall handling rating (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateHandlingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	/**
	 * Calculate braking rating (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateBrakingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	// ==========================================
	// PERFORMANCE PREDICTIONS
	// ==========================================

	/**
	 * Estimate 0-60 MPH time
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateZeroTo60(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain);

	/**
	 * Estimate 0-100 MPH time
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateZeroTo100(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain);

	/**
	 * Estimate quarter mile time
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateQuarterMile(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain);

	/**
	 * Estimate quarter mile trap speed
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateQuarterMileTrap(const FMGVehicleStats& Stats);

	/**
	 * Estimate top speed
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float EstimateTopSpeed(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain, const FMGAeroConfiguration& Aero);

	// ==========================================
	// PERFORMANCE INDEX
	// ==========================================

	/**
	 * Calculate Performance Index (PI) from vehicle stats
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculatePerformanceIndex(const FMGVehicleStats& Stats);

	/**
	 * Get performance class from PI value
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static EMGPerformanceClass GetPerformanceClass(float PerformanceIndex);

	// ==========================================
	// VALUE CALCULATIONS
	// ==========================================

	/**
	 * Calculate estimated vehicle value
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateVehicleValue(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	/**
	 * Calculate total parts value
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculatePartsValue(const FMGVehicleData& Vehicle);

	// ==========================================
	// COMPLETE STATS CALCULATION
	// ==========================================

	/**
	 * Calculate all stats for a vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Stats")
	static FMGVehicleStats CalculateAllStats(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	// ==========================================
	// UTILITIES
	// ==========================================

	/**
	 * Get part data by ID
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Parts")
	static UMGPartData* GetPartData(FName PartID);

	/**
	 * Get combined modifiers for all installed parts
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Parts")
	static FMGPartModifiers GetCombinedModifiers(const FMGEngineConfiguration& Engine);

	/**
	 * Check if part is compatible with vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Parts")
	static bool IsPartCompatible(const UMGPartData* Part, FName VehicleModelID, const FMGVehicleData& Vehicle);

	/**
	 * Get tire grip coefficient for compound
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float GetTireCompoundGrip(EMGTireCompound Compound);

	/**
	 * Get tire grip modifier for wet conditions
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float GetWetGripModifier(EMGTireCompound Compound);
};

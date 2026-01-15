// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MGVehicleData.h"
#include "MGStatCalculator.generated.h"

/**
 * Part modification data
 */
USTRUCT(BlueprintType)
struct FMGPartModifiers
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeightDelta = 0.0f; // kg

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RedlineBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostCapacity = 0.0f; // Max boost this part can support

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlowRating = 1.0f; // For fuel system limits

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurabilityRating = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReliabilityImpact = 0.0f; // Negative = less reliable
};

/**
 * Part data asset
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGPartData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName PartID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Manufacturer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	FName Category; // Engine, Exhaust, Suspension, etc.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	FName SubCategory; // AirFilter, Camshaft, etc.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EMGPartTier Tier = EMGPartTier::Street;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 BaseCost = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	float InstallationCost = 0.0f; // Additional labor

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FMGPartModifiers Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> CompatibleVehicles; // Empty = universal

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> RequiredParts; // Parts that must be installed first

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> IncompatibleParts; // Cannot be used with these

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TSoftObjectPtr<UStaticMesh> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
	TSoftObjectPtr<USoundBase> InstallSound;
};

/**
 * Static utility class for calculating vehicle statistics
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
	 * Calculate total horsepower from engine configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateHorsepower(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	/**
	 * Calculate total torque from engine configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateTorque(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	/**
	 * Calculate full power curve
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Stats")
	static FMGPowerCurve CalculatePowerCurve(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	/**
	 * Calculate redline based on internal upgrades
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static int32 CalculateRedline(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel);

	// ==========================================
	// WEIGHT CALCULATIONS
	// ==========================================

	/**
	 * Calculate total vehicle weight
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Stats")
	static float CalculateWeight(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel);

	/**
	 * Calculate weight distribution (returns front percentage)
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

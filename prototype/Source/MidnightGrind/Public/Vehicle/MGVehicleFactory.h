// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Vehicle/MGVehicleData.h"
#include "MGVehicleFactory.generated.h"

/**
 * Preset vehicle types for quick creation
 */
UENUM(BlueprintType)
enum class EMGVehiclePreset : uint8
{
	/** Entry-level JDM tuner (e.g., Civic, Miata) */
	JDM_Entry UMETA(DisplayName = "JDM Entry (D-Class)"),

	/** Mid-tier JDM (e.g., Silvia, RX-7) */
	JDM_Mid UMETA(DisplayName = "JDM Sports (C-Class)"),

	/** High-end JDM (e.g., Supra, GTR) */
	JDM_High UMETA(DisplayName = "JDM Legend (B-Class)"),

	/** American muscle entry (e.g., Mustang V6) */
	Muscle_Entry UMETA(DisplayName = "Muscle Entry (D-Class)"),

	/** American muscle mid (e.g., Mustang GT) */
	Muscle_Mid UMETA(DisplayName = "Muscle Sports (C-Class)"),

	/** American muscle high (e.g., Hellcat) */
	Muscle_High UMETA(DisplayName = "Muscle Super (A-Class)"),

	/** Euro entry (e.g., Golf GTI) */
	Euro_Entry UMETA(DisplayName = "Euro Hot Hatch (D-Class)"),

	/** Euro mid (e.g., M3, RS4) */
	Euro_Mid UMETA(DisplayName = "Euro Sports (B-Class)"),

	/** Euro high (e.g., AMG, M5) */
	Euro_High UMETA(DisplayName = "Euro Super (A-Class)"),

	/** Hypercar */
	Hypercar UMETA(DisplayName = "Hypercar (S-Class)"),

	/** Custom/Empty - use for manual configuration */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Vehicle Factory
 * Creates ready-to-use vehicle configurations for testing and gameplay
 *
 * Use this to:
 * - Generate vehicles without needing actual content
 * - Create balanced AI opponent vehicles
 * - Quickly prototype gameplay mechanics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==========================================
	// VEHICLE CREATION
	// ==========================================

	/** Create a vehicle from preset */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static FMGVehicleData CreateVehicleFromPreset(EMGVehiclePreset Preset);

	/** Create a random vehicle within performance range */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static FMGVehicleData CreateRandomVehicle(float MinPI, float MaxPI);

	/** Create a vehicle matching a specific performance class */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static FMGVehicleData CreateVehicleForClass(EMGPerformanceClass TargetClass);

	/** Create a starter vehicle (entry-level, affordable) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static FMGVehicleData CreateStarterVehicle();

	/** Create an AI opponent vehicle matched to player */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static FMGVehicleData CreateAIOpponent(const FMGVehicleData& PlayerVehicle, float DifficultyScale = 1.0f);

	// ==========================================
	// VEHICLE MODIFICATION
	// ==========================================

	/** Upgrade vehicle to target PI (returns modified copy) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static FMGVehicleData UpgradeToTargetPI(const FMGVehicleData& BaseVehicle, float TargetPI);

	/** Apply turbo upgrade to vehicle */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static FMGVehicleData ApplyTurboUpgrade(const FMGVehicleData& BaseVehicle, EMGForcedInductionType TurboType, float BoostPSI);

	/** Apply tire upgrade */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static FMGVehicleData ApplyTireUpgrade(const FMGVehicleData& BaseVehicle, EMGTireCompound Compound);

	// ==========================================
	// STAT CALCULATION
	// ==========================================

	/** Recalculate all stats for a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Factory")
	static void RecalculateStats(UPARAM(ref) FMGVehicleData& Vehicle);

	/** Calculate performance index from stats */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Factory")
	static float CalculatePerformanceIndex(const FMGVehicleStats& Stats);

	/** Get performance class from PI */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Factory")
	static EMGPerformanceClass GetPerformanceClassFromPI(float PI);

	// ==========================================
	// HELPERS
	// ==========================================

	/** Generate a realistic random name for a vehicle */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Factory")
	static FString GenerateVehicleName(EMGVehiclePreset Preset);

	/** Get preset display name */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Factory")
	static FText GetPresetDisplayName(EMGVehiclePreset Preset);

private:
	/** Internal creation with full parameters */
	static FMGVehicleData CreateVehicle(
		EMGEngineType EngineType,
		int32 DisplacementCC,
		float BaseHP,
		float BaseTorque,
		int32 Redline,
		EMGDrivetrainType Drivetrain,
		float WeightKG,
		EMGForcedInductionType FI,
		float BoostPSI
	);

	/** Apply random variation to make vehicles feel unique */
	static void ApplyRandomVariation(FMGVehicleData& Vehicle, float VariationAmount = 0.05f);
};

// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGVehicleFactory.h
 * @brief Factory functions for creating vehicle configurations without content assets.
 *
 * @section Overview
 * This file provides a Blueprint Function Library for generating vehicle data
 * programmatically. It's essential for prototyping, AI opponent creation, and
 * scenarios where you need vehicles without requiring actual mesh/sound assets.
 *
 * @section Architecture
 * The factory uses a preset system combined with procedural generation:
 *
 * 1. **Presets**: Predefined vehicle archetypes (JDM, Muscle, Euro, Hypercar)
 * 2. **Class Targeting**: Create vehicles matched to a performance class
 * 3. **PI Matching**: Generate vehicles within a specific Performance Index range
 * 4. **AI Opponents**: Create balanced opponents based on player's vehicle
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Blueprint Function Library**: A collection of static functions that can be
 * called from any Blueprint without needing an instance of the class. Think of
 * it like a toolbox of utility functions.
 *
 * **Performance Index (PI)**: A single number representing overall vehicle
 * capability. Used for matchmaking and race class restrictions. Higher PI =
 * faster vehicle. Ranges from ~100 (economy car) to 999 (hypercar).
 *
 * **Performance Classes**:
 * - D Class (100-299 PI): Entry-level daily drivers
 * - C Class (300-449 PI): Sport compacts, hot hatches
 * - B Class (450-599 PI): Sports cars, tuned vehicles
 * - A Class (600-749 PI): High-performance, serious tuning
 * - S Class (750-900 PI): Supercars, elite builds
 * - X Class (901+ PI): Hypercars, no-limit builds
 *
 * **Vehicle Archetypes**:
 * - JDM (Japanese Domestic Market): Silvia, Supra, RX-7, GTR
 * - Muscle: Mustang, Camaro, Challenger, Hellcat
 * - Euro: M3, RS4, AMG, Golf GTI
 *
 * @section Usage Example Usage
 * @code
 * // Create a starter vehicle for new players
 * FMGVehicleData StarterCar = UMGVehicleFactory::CreateStarterVehicle();
 *
 * // Create an AI opponent matched to the player
 * FMGVehicleData Opponent = UMGVehicleFactory::CreateAIOpponent(PlayerVehicle, 0.9f);
 *
 * // Create a specific preset
 * FMGVehicleData Supra = UMGVehicleFactory::CreateVehicleFromPreset(EMGVehiclePreset::JDM_High);
 *
 * // Create a vehicle targeting a specific PI range
 * FMGVehicleData RaceCar = UMGVehicleFactory::CreateRandomVehicle(700.0f, 750.0f);
 * @endcode
 *
 * @see FMGVehicleData The data structure this factory creates
 * @see EMGPerformanceClass Performance class enumeration
 */

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Vehicle/MG_VHCL_Data.h"
#include "MGVehicleFactory.generated.h"

// ============================================================================
// VEHICLE PRESET ENUMERATION
// ============================================================================

/**
 * @brief Preset vehicle types for quick creation.
 *
 * Each preset represents a category and tier of vehicle, providing
 * reasonable default values for engine type, power, weight, and drivetrain.
 * Use these for rapid prototyping or AI vehicle generation.
 *
 * The naming convention is: Category_Tier
 * - Category: JDM, Muscle, Euro, Hypercar
 * - Tier: Entry, Mid, High (corresponding to D/C, C/B, B/A/S classes)
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

// ============================================================================
// VEHICLE FACTORY CLASS
// ============================================================================

/**
 * @class UMGVehicleFactory
 * @brief Static factory for creating vehicle data configurations.
 *
 * This Blueprint Function Library provides utility functions to generate
 * complete vehicle configurations without requiring actual content assets.
 * Essential for AI opponents, testing, and procedural content generation.
 *
 * @section UseCases Primary Use Cases
 * - **Prototyping**: Test gameplay without creating actual vehicles
 * - **AI Opponents**: Generate balanced competitors for races
 * - **Starter Vehicles**: Create affordable entry-level cars for new players
 * - **Random Events**: Generate vehicles for traffic, encounters, etc.
 *
 * @section UnrealMacros Unreal Engine Macro Explanations
 *
 * **UCLASS()** with no specifiers creates a minimal UObject-derived class.
 * For a Blueprint Function Library, we inherit from UBlueprintFunctionLibrary.
 *
 * **UFUNCTION(BlueprintCallable, Category = "...")**
 * - BlueprintCallable: Function appears in Blueprint's action menu
 * - Category: Groups functions in the Blueprint menu (e.g., "Vehicle|Factory")
 *
 * **UFUNCTION(BlueprintPure, Category = "...")**
 * - BlueprintPure: Function has no side effects, can be called without execution pin
 * - Pure functions appear as "getter" nodes in Blueprint
 *
 * **static**: All functions are static because this is a utility library.
 * No instance is needed - call directly on the class.
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
	static float CalculatePerformanceIndex(const FMGVehicleSpecs& Stats);

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

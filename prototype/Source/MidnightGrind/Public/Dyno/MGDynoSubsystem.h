// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGDynoSubsystem.h - Chassis Dynamometer Testing System
 * =============================================================================
 *
 * OVERVIEW FOR BEGINNERS:
 * -----------------------
 * This file implements a "dyno" (dynamometer) system for Midnight Grind. In the
 * real world, a chassis dynamometer is a machine used to measure the power output
 * of a vehicle's engine. The car drives onto large rollers, and as the driver
 * accelerates through the RPM range, sensors measure how much force the wheels
 * produce. This data is then converted into horsepower and torque numbers.
 *
 * WHAT IS A DYNO RUN?
 * -------------------
 * A dyno run (or "dyno pull") is the process of:
 * 1. Strapping the vehicle securely to the dyno
 * 2. Warming up the engine to operating temperature
 * 3. Doing a "pull" - accelerating from low RPM to redline in a high gear
 * 4. Recording power and torque at each RPM point
 * 5. Generating a power curve graph showing the results
 *
 * WHY DO PLAYERS CARE ABOUT DYNO?
 * -------------------------------
 * In a car tuning/racing game like Midnight Grind, the dyno serves several purposes:
 *
 * 1. BRAGGING RIGHTS: "My Skyline makes 650 wheel horsepower!"
 *
 * 2. TUNING VALIDATION: After installing performance parts, players want to see
 *    the actual power gains. "That turbo upgrade gave me +87 HP!"
 *
 * 3. COMPARISON: Before/after comparisons let players see exactly how much
 *    their modifications improved the car.
 *
 * 4. DIAGNOSIS: If a car isn't performing well, the power curve might reveal
 *    issues (like boost falling off at high RPM).
 *
 * 5. IMMERSION: Real car enthusiasts love dyno culture. This feature makes
 *    the game feel authentic.
 *
 * KEY TERMINOLOGY:
 * ----------------
 *
 * HORSEPOWER (HP):
 * A measure of how much work the engine can do. Higher HP = higher top speed.
 * In this game, we track both:
 * - Wheel HP (WHP): Power measured at the wheels
 * - Crank HP: Estimated power at the crankshaft (higher than wheel HP)
 *
 * TORQUE:
 * A measure of rotational force. Higher torque = better acceleration.
 * Like horsepower, we track wheel torque and crank torque.
 *
 * DRIVETRAIN LOSS:
 * Power is lost between the engine and wheels due to friction in the
 * transmission, differential, and wheel bearings. Typical losses:
 * - FWD (Front Wheel Drive): 10-15%
 * - RWD (Rear Wheel Drive): 12-17%
 * - AWD (All Wheel Drive): 18-25%
 *
 * The system estimates crank HP by adding back the drivetrain loss.
 * Example: 400 WHP with 15% loss = 400 / 0.85 = ~471 crank HP
 *
 * POWER CURVE:
 * A graph showing HP and torque across the RPM range. Helps visualize:
 * - Where peak power occurs (important for shift points)
 * - The "power band" - the RPM range where the engine makes good power
 * - Turbo spool characteristics (boost builds with RPM)
 *
 * CORRECTION STANDARDS:
 * Real dynos adjust for atmospheric conditions (temperature, pressure, humidity)
 * because these affect engine performance. Different standards (SAE, DIN, JIS)
 * use different correction formulas. SAE J1349 is most common in North America.
 *
 * DATA STRUCTURES:
 * ----------------
 * - EMGDynoRunState: What phase the dyno run is in (Preparing, Running, etc.)
 * - EMGDynoCorrectionStandard: Which correction formula to use
 * - FMGDynoDataPoint: Power/torque at a single RPM point
 * - FMGDynoResult: Complete results from a dyno run
 * - FMGDynoComparison: Before/after comparison data
 *
 * USAGE EXAMPLE:
 * --------------
 * @code
 * UMGDynoSubsystem* Dyno = GetGameInstance()->GetSubsystem<UMGDynoSubsystem>();
 *
 * // Check if player can afford a dyno pull
 * if (Dyno->CanAffordDynoPull())
 * {
 *     // Subscribe to completion event
 *     Dyno->OnDynoRunComplete.AddDynamic(this, &UMyClass::HandleDynoComplete);
 *
 *     // Start the dyno run
 *     Dyno->StartDynoRun(VehicleID, VehicleData, BaseModel, "Stage 2 Turbo Tune");
 * }
 *
 * // Later, compare before and after mods
 * if (Dyno->HasComparisonBaseline(VehicleID))
 * {
 *     FMGDynoComparison Comparison;
 *     if (Dyno->CompareLatestToBaseline(VehicleID, Comparison))
 *     {
 *         UE_LOG(LogTemp, Log, TEXT("Gained %.1f WHP!"), Comparison.WheelHPGain);
 *     }
 * }
 *
 * // Export results for sharing
 * FString CSVData = Dyno->ExportDynoData(ResultID, EMGDynoExportFormat::CSV);
 * @endcode
 *
 * INTEGRATION WITH OTHER SYSTEMS:
 * -------------------------------
 * - ECONOMY: Dyno pulls cost credits, teaching resource management
 * - VEHICLE DATA: Results are based on the actual vehicle configuration
 * - ANALYTICS: Track which builds players create and how they tune
 *
 * GAMEPLAY BALANCE CONSIDERATIONS:
 * --------------------------------
 * - Dyno cost should be meaningful but not prohibitive
 * - Power numbers should feel "right" for the parts installed
 * - Before/after gains should match player expectations
 * - Export feature lets players share builds on social media
 *
 * @file MGDynoSubsystem.h
 * @brief Dyno testing subsystem for power and torque visualization
 *
 * Provides a chassis dynamometer simulation for measuring vehicle power output.
 * Integrates with the economy system for dyno rental costs and supports
 * before/after comparisons when testing modifications.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDynoSubsystem.generated.h"

class UMGEconomySubsystem;
class UMGVehicleModelData;
struct FMGVehicleData;
struct FMGEngineConfiguration;

/**
 * @brief Current state of a dyno run
 */
UENUM(BlueprintType)
enum class EMGDynoRunState : uint8
{
	/** Dyno is idle, ready to start */
	Idle UMETA(DisplayName = "Idle"),

	/** Vehicle is being strapped down and prepared */
	Preparing UMETA(DisplayName = "Preparing"),

	/** Engine warming up to operating temperature */
	WarmingUp UMETA(DisplayName = "Warming Up"),

	/** Dyno pull in progress, collecting data */
	Running UMETA(DisplayName = "Running"),

	/** Dyno pull complete, engine cooling down */
	CoolingDown UMETA(DisplayName = "Cooling Down"),

	/** Results available */
	Complete UMETA(DisplayName = "Complete"),

	/** Run was cancelled or failed */
	Failed UMETA(DisplayName = "Failed")
};

/**
 * @brief Correction standard used for dyno results
 *
 * Different standards account for atmospheric conditions differently,
 * affecting the reported power numbers.
 */
UENUM(BlueprintType)
enum class EMGDynoCorrectionStandard : uint8
{
	/** No correction applied - raw wheel numbers */
	Uncorrected UMETA(DisplayName = "Uncorrected"),

	/** SAE J1349 standard (North American) */
	SAE UMETA(DisplayName = "SAE J1349"),

	/** DIN 70020 standard (European) */
	DIN UMETA(DisplayName = "DIN 70020"),

	/** JIS D 1001 standard (Japanese) */
	JIS UMETA(DisplayName = "JIS D1001"),

	/** ECE R85 standard (European Community) */
	ECE UMETA(DisplayName = "ECE R85")
};

/**
 * @brief Single data point on the dyno curve
 *
 * Represents power and torque values at a specific RPM during a dyno pull.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGDynoDataPoint
{
	GENERATED_BODY()

	/** Engine RPM at this sample point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno")
	int32 RPM = 0;

	/** Horsepower measured at the wheels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno")
	float WheelHP = 0.0f;

	/** Torque measured at the wheels (lb-ft) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno")
	float WheelTorque = 0.0f;

	/** Estimated crank horsepower (accounting for drivetrain loss) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno")
	float CrankHP = 0.0f;

	/** Estimated crank torque (lb-ft) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno")
	float CrankTorque = 0.0f;

	/** Boost pressure at this RPM (PSI, 0 for N/A engines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno")
	float BoostPSI = 0.0f;

	/** Air/Fuel ratio at this RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno")
	float AirFuelRatio = 14.7f;
};

/**
 * @brief Complete dyno test result with power curve data
 *
 * Contains all data points from a dyno pull along with analyzed peak values
 * and comparison data. This is the primary output from a dyno run.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGDynoResult
{
	GENERATED_BODY()

	/** Unique identifier for this dyno run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Identity")
	FGuid ResultID;

	/** Vehicle that was tested */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Identity")
	FGuid VehicleID;

	/** Timestamp when the run completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Identity")
	FDateTime Timestamp;

	/** User-provided description of the tune/configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Identity")
	FString TuneDescription;

	// ==========================================
	// POWER CURVE DATA
	// ==========================================

	/** Array of RPM points sampled during the pull */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Curve")
	TArray<int32> RPMPoints;

	/** Wheel horsepower at each RPM point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Curve")
	TArray<float> WheelHorsepowerCurve;

	/** Wheel torque (lb-ft) at each RPM point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Curve")
	TArray<float> WheelTorqueCurve;

	/** Crank horsepower at each RPM point (estimated from wheel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Curve")
	TArray<float> CrankHorsepowerCurve;

	/** Crank torque at each RPM point (estimated from wheel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Curve")
	TArray<float> CrankTorqueCurve;

	/** Boost pressure curve (for forced induction vehicles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Curve")
	TArray<float> BoostCurve;

	/** Air/fuel ratio curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Curve")
	TArray<float> AFRCurve;

	// ==========================================
	// PEAK VALUES
	// ==========================================

	/** Peak wheel horsepower */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	float PeakWheelHP = 0.0f;

	/** RPM where peak wheel HP occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	int32 PeakWheelHPRPM = 0;

	/** Peak wheel torque (lb-ft) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	float PeakWheelTorque = 0.0f;

	/** RPM where peak wheel torque occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	int32 PeakWheelTorqueRPM = 0;

	/** Peak crank horsepower (estimated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	float PeakCrankHP = 0.0f;

	/** RPM where peak crank HP occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	int32 PeakCrankHPRPM = 0;

	/** Peak crank torque (lb-ft, estimated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	float PeakCrankTorque = 0.0f;

	/** RPM where peak crank torque occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	int32 PeakCrankTorqueRPM = 0;

	/** Peak boost pressure (PSI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Peaks")
	float PeakBoostPSI = 0.0f;

	// ==========================================
	// DRIVETRAIN ANALYSIS
	// ==========================================

	/** Drivetrain loss percentage (wheel HP vs crank HP) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Drivetrain")
	float DrivetrainLossPercent = 15.0f;

	/** Estimated drivetrain loss in HP */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Drivetrain")
	float DrivetrainLossHP = 0.0f;

	// ==========================================
	// POWER BAND ANALYSIS
	// ==========================================

	/** RPM where usable power band starts (90% of peak) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|PowerBand")
	int32 PowerBandStartRPM = 0;

	/** RPM where usable power band ends (90% of peak) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|PowerBand")
	int32 PowerBandEndRPM = 0;

	/** Width of usable power band in RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|PowerBand")
	int32 PowerBandWidth = 0;

	/** Redline RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|PowerBand")
	int32 RedlineRPM = 0;

	// ==========================================
	// ATMOSPHERIC CONDITIONS
	// ==========================================

	/** Ambient temperature during test (Celsius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Conditions")
	float AmbientTempC = 25.0f;

	/** Barometric pressure (mbar) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Conditions")
	float BarometricPressure = 1013.25f;

	/** Relative humidity (0-100%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Conditions")
	float RelativeHumidity = 50.0f;

	/** Correction factor applied to results */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Conditions")
	float CorrectionFactor = 1.0f;

	/** Correction standard used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Conditions")
	EMGDynoCorrectionStandard CorrectionStandard = EMGDynoCorrectionStandard::SAE;

	// ==========================================
	// HELPER METHODS
	// ==========================================

	/**
	 * @brief Get horsepower at a specific RPM via interpolation
	 * @param RPM Target RPM to query
	 * @param bWheelHP If true, returns wheel HP; if false, returns crank HP
	 * @return Interpolated horsepower value
	 */
	float GetHorsepowerAtRPM(int32 RPM, bool bWheelHP = true) const;

	/**
	 * @brief Get torque at a specific RPM via interpolation
	 * @param RPM Target RPM to query
	 * @param bWheelTorque If true, returns wheel torque; if false, returns crank torque
	 * @return Interpolated torque value (lb-ft)
	 */
	float GetTorqueAtRPM(int32 RPM, bool bWheelTorque = true) const;

	/**
	 * @brief Check if results are valid
	 * @return True if the result contains valid data
	 */
	bool IsValid() const { return ResultID.IsValid() && RPMPoints.Num() > 0; }
};

/**
 * @brief Comparison between two dyno runs (before/after mods)
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGDynoComparison
{
	GENERATED_BODY()

	/** The baseline (before) dyno result */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	FGuid BaselineResultID;

	/** The comparison (after) dyno result */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	FGuid ComparisonResultID;

	/** Change in peak wheel horsepower */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	float WheelHPGain = 0.0f;

	/** Percentage change in wheel HP */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	float WheelHPGainPercent = 0.0f;

	/** Change in peak wheel torque */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	float WheelTorqueGain = 0.0f;

	/** Percentage change in wheel torque */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	float WheelTorqueGainPercent = 0.0f;

	/** Change in peak crank HP */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	float CrankHPGain = 0.0f;

	/** Change in peak crank torque */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	float CrankTorqueGain = 0.0f;

	/** Change in power band width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	int32 PowerBandWidthChange = 0;

	/** Shift in peak HP RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	int32 PeakHPRPMShift = 0;

	/** Shift in peak torque RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	int32 PeakTorqueRPMShift = 0;

	/** Summary description of gains */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dyno|Comparison")
	FText GainSummary;
};

/**
 * @brief Export format for dyno data
 */
UENUM(BlueprintType)
enum class EMGDynoExportFormat : uint8
{
	/** Comma-separated values */
	CSV UMETA(DisplayName = "CSV"),

	/** JSON format */
	JSON UMETA(DisplayName = "JSON"),

	/** Human-readable text */
	Text UMETA(DisplayName = "Text")
};

// ==========================================
// DELEGATES
// ==========================================

/** Broadcast when a dyno run starts */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDynoRunStarted, FGuid, VehicleID, EMGDynoRunState, InitialState);

/** Broadcast when a dyno run completes with results */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDynoRunComplete, FGuid, VehicleID, const FMGDynoResult&, Result);

/** Broadcast during a run with live data points */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDynoLiveData, FGuid, VehicleID, int32, CurrentRPM, float, CurrentWheelHP);

/** Broadcast when dyno run state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDynoStateChanged, FGuid, VehicleID, EMGDynoRunState, OldState, EMGDynoRunState, NewState);

/** Broadcast when a dyno run fails or is cancelled */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDynoRunFailed, FGuid, VehicleID, FText, FailureReason);

/**
 * @class UMGDynoSubsystem
 * @brief Game Instance Subsystem for chassis dynamometer testing
 *
 * Provides a complete dyno testing experience for measuring and visualizing
 * vehicle power output. Features include:
 * - Realistic power curve generation based on vehicle configuration
 * - Wheel HP vs Crank HP calculations with drivetrain loss
 * - Before/after comparison for testing modifications
 * - Integration with economy system for dyno rental fees
 * - Data export for tuning reference
 *
 * Usage:
 * @code
 * UMGDynoSubsystem* DynoSystem = GetGameInstance()->GetSubsystem<UMGDynoSubsystem>();
 * if (DynoSystem->CanAffordDynoPull())
 * {
 *     DynoSystem->StartDynoRun(VehicleID, VehicleData, BaseModel);
 * }
 * @endcode
 *
 * @see FMGDynoResult for output data structure
 * @see UMGEconomySubsystem for rental cost handling
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDynoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ==========================================
	// DYNO OPERATIONS
	// ==========================================

	/**
	 * @brief Start a dyno pull for the specified vehicle
	 *
	 * Initiates a dyno run which will progress through warmup, pull, and cooldown
	 * phases. The dyno rental cost is deducted from the player's credits.
	 * Subscribe to OnDynoRunComplete to receive results.
	 *
	 * @param VehicleID Unique identifier for the vehicle being tested
	 * @param VehicleData Current vehicle configuration data
	 * @param BaseModel Base vehicle model data asset
	 * @param TuneDescription Optional description of the current tune/mods
	 * @return True if the dyno run was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	bool StartDynoRun(FGuid VehicleID, const FMGVehicleData& VehicleData,
					  const UMGVehicleModelData* BaseModel, const FString& TuneDescription = TEXT(""));

	/**
	 * @brief Cancel an in-progress dyno run
	 *
	 * Stops the current dyno pull. No refund is provided for cancelled runs.
	 *
	 * @param VehicleID Vehicle whose dyno run should be cancelled
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	void CancelDynoRun(FGuid VehicleID);

	/**
	 * @brief Get the results from the most recent completed dyno run
	 *
	 * @param VehicleID Vehicle to get results for
	 * @param OutResult Output parameter for the dyno result
	 * @return True if results were found and valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	bool GetDynoResults(FGuid VehicleID, FMGDynoResult& OutResult) const;

	/**
	 * @brief Get a specific dyno result by its ID
	 *
	 * @param ResultID The unique ID of the dyno result
	 * @param OutResult Output parameter for the dyno result
	 * @return True if the result was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	bool GetDynoResultByID(FGuid ResultID, FMGDynoResult& OutResult) const;

	/**
	 * @brief Get all dyno history for a vehicle
	 *
	 * @param VehicleID Vehicle to get history for
	 * @param MaxResults Maximum number of results to return (0 = all)
	 * @return Array of dyno results, sorted by date (newest first)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	TArray<FMGDynoResult> GetDynoHistory(FGuid VehicleID, int32 MaxResults = 10) const;

	/**
	 * @brief Export dyno data to a string for tuning reference
	 *
	 * Generates a formatted export of the dyno data in the specified format.
	 * Useful for sharing results or importing into external tuning tools.
	 *
	 * @param ResultID The dyno result to export
	 * @param Format The export format to use
	 * @return Formatted string containing the dyno data
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	FString ExportDynoData(FGuid ResultID, EMGDynoExportFormat Format = EMGDynoExportFormat::CSV) const;

	/**
	 * @brief Export dyno data to a file
	 *
	 * @param ResultID The dyno result to export
	 * @param FilePath Path to save the file
	 * @param Format The export format to use
	 * @return True if export was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	bool ExportDynoDataToFile(FGuid ResultID, const FString& FilePath, EMGDynoExportFormat Format = EMGDynoExportFormat::CSV) const;

	// ==========================================
	// COMPARISON
	// ==========================================

	/**
	 * @brief Set the baseline result for before/after comparison
	 *
	 * The baseline is used when comparing the effects of modifications.
	 * Call this before installing mods, then run another dyno after mods.
	 *
	 * @param VehicleID Vehicle to set baseline for
	 * @param ResultID The dyno result to use as baseline
	 * @return True if baseline was set successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno|Comparison")
	bool SetComparisonBaseline(FGuid VehicleID, FGuid ResultID);

	/**
	 * @brief Clear the comparison baseline for a vehicle
	 *
	 * @param VehicleID Vehicle to clear baseline for
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno|Comparison")
	void ClearComparisonBaseline(FGuid VehicleID);

	/**
	 * @brief Compare two dyno results (before/after mods)
	 *
	 * Generates a detailed comparison between two dyno runs, typically
	 * used to measure the effect of modifications.
	 *
	 * @param BaselineResultID The "before" result
	 * @param ComparisonResultID The "after" result
	 * @param OutComparison Output comparison data
	 * @return True if comparison was generated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno|Comparison")
	bool CompareDynoResults(FGuid BaselineResultID, FGuid ComparisonResultID, FMGDynoComparison& OutComparison) const;

	/**
	 * @brief Compare latest result against baseline
	 *
	 * Convenience method that compares the most recent dyno result
	 * against the stored baseline for the vehicle.
	 *
	 * @param VehicleID Vehicle to compare
	 * @param OutComparison Output comparison data
	 * @return True if comparison was generated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno|Comparison")
	bool CompareLatestToBaseline(FGuid VehicleID, FMGDynoComparison& OutComparison) const;

	/**
	 * @brief Check if a baseline is set for comparison
	 *
	 * @param VehicleID Vehicle to check
	 * @return True if a baseline is set
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|Comparison")
	bool HasComparisonBaseline(FGuid VehicleID) const;

	// ==========================================
	// ECONOMY
	// ==========================================

	/**
	 * @brief Get the cost of a single dyno pull
	 * @return Cost in credits
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|Economy")
	int64 GetDynoPullCost() const { return DynoPullCost; }

	/**
	 * @brief Check if player can afford a dyno pull
	 * @return True if player has sufficient credits
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|Economy")
	bool CanAffordDynoPull() const;

	/**
	 * @brief Get total amount spent on dyno pulls this session
	 * @return Total credits spent
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|Economy")
	int64 GetTotalDynoSpending() const { return TotalDynoSpending; }

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/**
	 * @brief Get current dyno run state for a vehicle
	 *
	 * @param VehicleID Vehicle to query
	 * @return Current state of the dyno run
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|State")
	EMGDynoRunState GetDynoRunState(FGuid VehicleID) const;

	/**
	 * @brief Check if a dyno run is in progress for a vehicle
	 *
	 * @param VehicleID Vehicle to check
	 * @return True if a run is active (not Idle, Complete, or Failed)
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|State")
	bool IsDynoRunInProgress(FGuid VehicleID) const;

	/**
	 * @brief Get the current RPM during an active dyno run
	 *
	 * @param VehicleID Vehicle being tested
	 * @return Current RPM, or 0 if no active run
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|State")
	int32 GetCurrentDynoRPM(FGuid VehicleID) const;

	/**
	 * @brief Get progress of current dyno run (0.0 to 1.0)
	 *
	 * @param VehicleID Vehicle being tested
	 * @return Progress percentage, or 0 if no active run
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|State")
	float GetDynoRunProgress(FGuid VehicleID) const;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * @brief Set the drivetrain loss percentage for HP calculations
	 *
	 * Typical values:
	 * - FWD: 10-15%
	 * - RWD: 12-17%
	 * - AWD: 18-25%
	 *
	 * @param VehicleID Vehicle to configure
	 * @param LossPercent Drivetrain loss percentage (0-50)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno|Config")
	void SetDrivetrainLossPercent(FGuid VehicleID, float LossPercent);

	/**
	 * @brief Set the correction standard for dyno results
	 *
	 * @param Standard The correction standard to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno|Config")
	void SetCorrectionStandard(EMGDynoCorrectionStandard Standard);

	/**
	 * @brief Get the current correction standard
	 * @return Active correction standard
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno|Config")
	EMGDynoCorrectionStandard GetCorrectionStandard() const { return CurrentCorrectionStandard; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Broadcast when a dyno run starts */
	UPROPERTY(BlueprintAssignable, Category = "Dyno|Events")
	FOnDynoRunStarted OnDynoRunStarted;

	/** Broadcast when a dyno run completes with results */
	UPROPERTY(BlueprintAssignable, Category = "Dyno|Events")
	FOnDynoRunComplete OnDynoRunComplete;

	/** Broadcast during a run with live data points */
	UPROPERTY(BlueprintAssignable, Category = "Dyno|Events")
	FOnDynoLiveData OnDynoLiveData;

	/** Broadcast when dyno run state changes */
	UPROPERTY(BlueprintAssignable, Category = "Dyno|Events")
	FOnDynoStateChanged OnDynoStateChanged;

	/** Broadcast when a dyno run fails or is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Dyno|Events")
	FOnDynoRunFailed OnDynoRunFailed;

protected:
	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/** Active dyno session data */
	struct FDynoSession
	{
		FGuid VehicleID;
		FMGVehicleData VehicleData;
		const UMGVehicleModelData* BaseModel = nullptr;
		FString TuneDescription;
		EMGDynoRunState State = EMGDynoRunState::Idle;
		int32 CurrentRPM = 0;
		float Progress = 0.0f;
		float DrivetrainLossPercent = 15.0f;
		TArray<FMGDynoDataPoint> LiveData;
		FDateTime StartTime;
	};

	/** Currently active dyno sessions */
	TMap<FGuid, FDynoSession> ActiveSessions;

	/** Stored dyno results history */
	UPROPERTY()
	TMap<FGuid, FMGDynoResult> DynoHistory;

	/** Most recent result per vehicle */
	UPROPERTY()
	TMap<FGuid, FGuid> LatestResultByVehicle;

	/** Comparison baselines per vehicle */
	UPROPERTY()
	TMap<FGuid, FGuid> ComparisonBaselines;

	/** Current correction standard */
	UPROPERTY()
	EMGDynoCorrectionStandard CurrentCorrectionStandard = EMGDynoCorrectionStandard::SAE;

	/** Total credits spent on dyno pulls this session */
	UPROPERTY()
	int64 TotalDynoSpending = 0;

	/** Cost per dyno pull in credits */
	UPROPERTY()
	int64 DynoPullCost = 500;

	/** Timer handle for dyno simulation tick */
	FTimerHandle DynoTickHandle;

	// ==========================================
	// SIMULATION PARAMETERS
	// ==========================================

	/** Starting RPM for dyno pull */
	static constexpr int32 DynoStartRPM = 2000;

	/** RPM increment per tick */
	static constexpr int32 DynoRPMStep = 100;

	/** Tick interval for simulation (50ms = 20Hz) */
	static constexpr float DynoTickInterval = 0.05f;

	/** Duration of warmup phase (seconds) */
	static constexpr float WarmupDuration = 2.0f;

	/** Duration of cooldown phase (seconds) */
	static constexpr float CooldownDuration = 1.5f;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/**
	 * @brief Process a tick of the dyno simulation
	 * @param VehicleID Vehicle being simulated
	 */
	void TickDynoSimulation(FGuid VehicleID);

	/**
	 * @brief Simulate a data point at the given RPM
	 * @param Session Active dyno session
	 * @param RPM Current RPM
	 * @return Simulated data point
	 */
	FMGDynoDataPoint SimulateDataPoint(const FDynoSession& Session, int32 RPM) const;

	/**
	 * @brief Calculate final results from collected data
	 * @param Session Completed dyno session
	 * @return Processed dyno result
	 */
	FMGDynoResult CalculateResults(const FDynoSession& Session) const;

	/**
	 * @brief Calculate SAE correction factor for atmospheric conditions
	 * @param TempC Ambient temperature in Celsius
	 * @param PressureMbar Barometric pressure in mbar
	 * @param HumidityPercent Relative humidity percentage
	 * @return Correction factor multiplier
	 */
	float CalculateSAECorrectionFactor(float TempC, float PressureMbar, float HumidityPercent) const;

	/**
	 * @brief Change dyno run state and broadcast event
	 * @param VehicleID Vehicle whose state is changing
	 * @param NewState New state to transition to
	 */
	void SetDynoState(FGuid VehicleID, EMGDynoRunState NewState);

	/**
	 * @brief Get the economy subsystem
	 * @return Economy subsystem pointer, or nullptr if unavailable
	 */
	UMGEconomySubsystem* GetEconomySubsystem() const;

	/**
	 * @brief Generate CSV export string
	 * @param Result Dyno result to export
	 * @return CSV formatted string
	 */
	FString GenerateCSVExport(const FMGDynoResult& Result) const;

	/**
	 * @brief Generate JSON export string
	 * @param Result Dyno result to export
	 * @return JSON formatted string
	 */
	FString GenerateJSONExport(const FMGDynoResult& Result) const;

	/**
	 * @brief Generate text export string
	 * @param Result Dyno result to export
	 * @return Human-readable formatted string
	 */
	FString GenerateTextExport(const FMGDynoResult& Result) const;
};

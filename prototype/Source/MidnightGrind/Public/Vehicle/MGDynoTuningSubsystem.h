// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDynoTuningSubsystem.generated.h"

/**
 * Dyno run status
 */
UENUM(BlueprintType)
enum class EMGDynoStatus : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	WarmingUp UMETA(DisplayName = "Warming Up"),
	Running UMETA(DisplayName = "Running"),
	Cooling UMETA(DisplayName = "Cooling Down"),
	Complete UMETA(DisplayName = "Complete"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * Tuning category
 */
UENUM(BlueprintType)
enum class EMGTuningCategory : uint8
{
	ECU UMETA(DisplayName = "ECU/Engine Management"),
	ForcedInduction UMETA(DisplayName = "Forced Induction"),
	Transmission UMETA(DisplayName = "Transmission"),
	Suspension UMETA(DisplayName = "Suspension"),
	Alignment UMETA(DisplayName = "Alignment"),
	Differential UMETA(DisplayName = "Differential"),
	Brakes UMETA(DisplayName = "Brakes"),
	Nitrous UMETA(DisplayName = "Nitrous")
};

/**
 * Driving style preset
 */
UENUM(BlueprintType)
enum class EMGDrivingStylePreset : uint8
{
	Balanced UMETA(DisplayName = "Balanced"),
	Grip UMETA(DisplayName = "Grip Racing"),
	Drift UMETA(DisplayName = "Drift"),
	Drag UMETA(DisplayName = "Drag Racing"),
	TopSpeed UMETA(DisplayName = "Top Speed"),
	Touge UMETA(DisplayName = "Touge/Canyon"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Single dyno data point
 */
USTRUCT(BlueprintType)
struct FMGDynoDataPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RPM = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Horsepower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Torque = 0.0f; // lb-ft

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirFuelRatio = 14.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostPSI = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExhaustGasTemp = 0.0f; // Celsius

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OilTemp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolantTemp = 0.0f;
};

/**
 * Complete dyno run result
 */
USTRUCT(BlueprintType)
struct FMGDynoRunResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RunID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDynoDataPoint> DataPoints;

	// Peak values
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakHP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PeakHPRPM = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakTorque = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PeakTorqueRPM = 0;

	// Power band analysis
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PowerBandStartRPM = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PowerBandEndRPM = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RedlineRPM = 0;

	// Correction factors
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AmbientTemp = 25.0f; // Celsius

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BarometricPressure = 1013.25f; // mbar

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorrectionFactor = 1.0f; // SAE correction

	// Comparison to previous
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HPGainFromPrevious = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueGainFromPrevious = 0.0f;

	// Notes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TuneDescription;
};

/**
 * ECU tuning parameters
 */
USTRUCT(BlueprintType)
struct FMGECUTuneData
{
	GENERATED_BODY()

	// Fuel map adjustment (-20% to +20%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-20.0", ClampMax = "20.0"))
	float FuelMapLow = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-20.0", ClampMax = "20.0"))
	float FuelMapMid = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-20.0", ClampMax = "20.0"))
	float FuelMapHigh = 0.0f;

	// Ignition timing adjustment (-10 to +10 degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-10.0", ClampMax = "10.0"))
	float IgnitionTimingLow = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-10.0", ClampMax = "10.0"))
	float IgnitionTimingMid = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-10.0", ClampMax = "10.0"))
	float IgnitionTimingHigh = 0.0f;

	// Rev limiter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "4000", ClampMax = "12000"))
	int32 RevLimiter = 7000;

	// Launch control RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2000", ClampMax = "8000"))
	int32 LaunchControlRPM = 4000;

	// Speed limiter (0 = disabled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "300"))
	int32 SpeedLimiterMPH = 0;

	// Anti-lag enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAntiLagEnabled = false;

	// Flat-foot shifting enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlatFootShifting = false;
};

/**
 * Forced induction tuning
 */
USTRUCT(BlueprintType)
struct FMGForcedInductionTuneData
{
	GENERATED_BODY()

	// Has turbo/supercharger
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasTurbo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSupercharger = false;

	// Turbo settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float TargetBoostPSI = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float WastegateSpringPSI = 7.0f;

	// Boost by gear (multiplier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> BoostByGear; // 0-1 multiplier per gear

	// Supercharger pulley ratio (smaller = more boost)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float SuperchargerPulleyRatio = 1.0f;

	// Intercooler efficiency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IntercoolerEfficiency = 0.7f;

	// Blow-off valve type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRecirculatingBOV = true;

	// Anti-lag aggression (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AntiLagAggression = 0.5f;
};

/**
 * Transmission tuning
 */
USTRUCT(BlueprintType)
struct FMGTransmissionTuneData
{
	GENERATED_BODY()

	// Gear ratios (1st through 6th+)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> GearRatios;

	// Final drive ratio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2.0", ClampMax = "6.0"))
	float FinalDriveRatio = 3.73f;

	// Automatic shift points (% of redline)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float UpshiftPoint = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.2", ClampMax = "0.8"))
	float DownshiftPoint = 0.4f;

	// Clutch settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float ClutchEngagementSpeed = 1.0f;

	// Sequential/H-pattern
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSequentialMode = false;

	// Auto-blip on downshift
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoBlip = true;
};

/**
 * Suspension tuning
 */
USTRUCT(BlueprintType)
struct FMGSuspensionTuneData
{
	GENERATED_BODY()

	// Spring rates (N/mm) - Front/Rear
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "20.0", ClampMax = "300.0"))
	float FrontSpringRate = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "20.0", ClampMax = "300.0"))
	float RearSpringRate = 70.0f;

	// Damper settings (clicks 1-30)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "30"))
	int32 FrontCompressionDamping = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "30"))
	int32 FrontReboundDamping = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "30"))
	int32 RearCompressionDamping = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "30"))
	int32 RearReboundDamping = 15;

	// Anti-roll bars (N/mm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float FrontAntiRollBar = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float RearAntiRollBar = 15.0f;

	// Ride height (mm from default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-80.0", ClampMax = "50.0"))
	float FrontRideHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-80.0", ClampMax = "50.0"))
	float RearRideHeight = 0.0f;

	// Bump stops
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "50.0"))
	float BumpStopRate = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10.0", ClampMax = "100.0"))
	float BumpStopRange = 30.0f;
};

/**
 * Alignment tuning
 */
USTRUCT(BlueprintType)
struct FMGAlignmentTuneData
{
	GENERATED_BODY()

	// Camber (degrees, negative = top of tire leans in)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-8.0", ClampMax = "2.0"))
	float FrontCamber = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-8.0", ClampMax = "2.0"))
	float RearCamber = -0.5f;

	// Toe (degrees, positive = toe in)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-2.0", ClampMax = "2.0"))
	float FrontToe = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-2.0", ClampMax = "2.0"))
	float RearToe = 0.1f;

	// Caster (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2.0", ClampMax = "12.0"))
	float Caster = 5.0f;

	// Steering ratio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10.0", ClampMax = "20.0"))
	float SteeringRatio = 14.0f;

	// Steering angle (max degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "25.0", ClampMax = "70.0"))
	float MaxSteeringAngle = 35.0f;

	// Ackermann (0 = parallel, 1 = full Ackermann)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AckermannPercentage = 0.5f;
};

/**
 * Differential tuning
 */
USTRUCT(BlueprintType)
struct FMGDifferentialTuneData
{
	GENERATED_BODY()

	// Differential type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLimitedSlip = true;

	// LSD settings (0 = open, 100 = locked)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 AccelerationLock = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 DecelerationLock = 40;

	// Preload (Nm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float Preload = 50.0f;

	// AWD power split (front %)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 AWDFrontBias = 40;

	// Center diff (for AWD)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 CenterDiffLock = 50;
};

/**
 * Brake tuning
 */
USTRUCT(BlueprintType)
struct FMGBrakeTuneData
{
	GENERATED_BODY()

	// Brake bias (front %)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "40", ClampMax = "80"))
	int32 BrakeBias = 60;

	// Brake pressure (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float BrakePressure = 1.0f;

	// ABS enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bABSEnabled = true;

	// ABS aggression (lower = more aggressive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "10"))
	int32 ABSSetting = 5;

	// Handbrake drift mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHandbrakeDriftMode = false;
};

/**
 * Nitrous tuning
 */
USTRUCT(BlueprintType)
struct FMGNitrousTuneData
{
	GENERATED_BODY()

	// Shot size (HP gain)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "25", ClampMax = "500"))
	int32 ShotSizeHP = 75;

	// Bottle pressure (PSI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "800", ClampMax = "1200"))
	int32 BottlePressure = 950;

	// Activation RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2000", ClampMax = "8000"))
	int32 ActivationRPM = 3500;

	// Purge enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPurgeEnabled = true;

	// Progressive controller (ramps up over time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bProgressiveController = false;

	// Wet vs dry shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWetShot = true;
};

/**
 * Complete vehicle tune
 */
USTRUCT(BlueprintType)
struct FMGVehicleTuneProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TuneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TuneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDrivingStylePreset StylePreset = EMGDrivingStylePreset::Balanced;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastModified;

	// Tuning categories
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGECUTuneData ECU;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGForcedInductionTuneData ForcedInduction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTransmissionTuneData Transmission;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGSuspensionTuneData Suspension;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGAlignmentTuneData Alignment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDifferentialTuneData Differential;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGBrakeTuneData Brakes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGNitrousTuneData Nitrous;

	// Associated dyno run
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid LastDynoRunID;

	// Sharing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPublic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DownloadCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rating = 0.0f;
};

/**
 * Dyno session state
 */
USTRUCT(BlueprintType)
struct FMGDynoSession
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDynoStatus Status = EMGDynoStatus::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Progress = 0.0f; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentRPM = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentTorque = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDynoDataPoint> LiveData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDynoStatusChanged, FGuid, SessionID, EMGDynoStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDynoDataPoint, FGuid, SessionID, FMGDynoDataPoint, DataPoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDynoRunComplete, FGuid, SessionID, FMGDynoRunResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTuneApplied, FGuid, VehicleID, FGuid, TuneID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTuneSaved, FGuid, PlayerID, FGuid, VehicleID, FMGVehicleTuneProfile, Tune);

/**
 * Dyno and Tuning Subsystem
 * Manages dyno runs and vehicle tuning per PRD Section 5.2
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDynoTuningSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// DYNO OPERATIONS
	// ==========================================

	/**
	 * Start a dyno session for vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	FGuid StartDynoSession(FGuid PlayerID, FGuid VehicleID);

	/**
	 * Begin the dyno run
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	bool StartDynoRun(FGuid SessionID);

	/**
	 * Cancel current dyno run
	 */
	UFUNCTION(BlueprintCallable, Category = "Dyno")
	void CancelDynoRun(FGuid SessionID);

	/**
	 * Get current dyno session state
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno")
	bool GetDynoSession(FGuid SessionID, FMGDynoSession& OutSession) const;

	/**
	 * Get dyno run result
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno")
	bool GetDynoRunResult(FGuid RunID, FMGDynoRunResult& OutResult) const;

	/**
	 * Get dyno history for vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno")
	TArray<FMGDynoRunResult> GetDynoHistory(FGuid VehicleID, int32 MaxResults = 10) const;

	/**
	 * Compare two dyno runs
	 */
	UFUNCTION(BlueprintPure, Category = "Dyno")
	void CompareDynoRuns(FGuid RunID1, FGuid RunID2, float& OutHPDiff, float& OutTorqueDiff, int32& OutPowerBandDiff) const;

	// ==========================================
	// TUNING OPERATIONS
	// ==========================================

	/**
	 * Create new tune profile for vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning")
	FGuid CreateTuneProfile(FGuid PlayerID, FGuid VehicleID, const FString& TuneName);

	/**
	 * Save current tune profile
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning")
	bool SaveTuneProfile(FGuid TuneID);

	/**
	 * Delete tune profile
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning")
	bool DeleteTuneProfile(FGuid TuneID);

	/**
	 * Get tune profile
	 */
	UFUNCTION(BlueprintPure, Category = "Tuning")
	bool GetTuneProfile(FGuid TuneID, FMGVehicleTuneProfile& OutProfile) const;

	/**
	 * Get all tune profiles for vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Tuning")
	TArray<FMGVehicleTuneProfile> GetVehicleTuneProfiles(FGuid VehicleID) const;

	/**
	 * Apply tune profile to vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning")
	bool ApplyTuneProfile(FGuid VehicleID, FGuid TuneID);

	/**
	 * Load driving style preset
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning")
	bool LoadPreset(FGuid TuneID, EMGDrivingStylePreset Preset);

	// ==========================================
	// INDIVIDUAL TUNING PARAMETERS
	// ==========================================

	/**
	 * Update ECU tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning|ECU")
	bool UpdateECUTune(FGuid TuneID, const FMGECUTuneData& ECUData);

	/**
	 * Update forced induction tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning|ForcedInduction")
	bool UpdateForcedInductionTune(FGuid TuneID, const FMGForcedInductionTuneData& FIData);

	/**
	 * Update transmission tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning|Transmission")
	bool UpdateTransmissionTune(FGuid TuneID, const FMGTransmissionTuneData& TransData);

	/**
	 * Update suspension tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning|Suspension")
	bool UpdateSuspensionTune(FGuid TuneID, const FMGSuspensionTuneData& SuspData);

	/**
	 * Update alignment tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning|Alignment")
	bool UpdateAlignmentTune(FGuid TuneID, const FMGAlignmentTuneData& AlignData);

	/**
	 * Update differential tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning|Differential")
	bool UpdateDifferentialTune(FGuid TuneID, const FMGDifferentialTuneData& DiffData);

	/**
	 * Update brake tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning|Brakes")
	bool UpdateBrakeTune(FGuid TuneID, const FMGBrakeTuneData& BrakeData);

	/**
	 * Update nitrous tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Tuning|Nitrous")
	bool UpdateNitrousTune(FGuid TuneID, const FMGNitrousTuneData& NitroData);

	// ==========================================
	// TUNE SHARING
	// ==========================================

	/**
	 * Make tune public for sharing
	 */
	UFUNCTION(BlueprintCallable, Category = "Sharing")
	bool PublishTune(FGuid TuneID);

	/**
	 * Make tune private
	 */
	UFUNCTION(BlueprintCallable, Category = "Sharing")
	bool UnpublishTune(FGuid TuneID);

	/**
	 * Download shared tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Sharing")
	FGuid DownloadSharedTune(FGuid PlayerID, FGuid SharedTuneID, FGuid TargetVehicleID);

	/**
	 * Search public tunes
	 */
	UFUNCTION(BlueprintCallable, Category = "Sharing")
	TArray<FMGVehicleTuneProfile> SearchPublicTunes(FName VehicleModelID, EMGDrivingStylePreset StyleFilter, int32 MaxResults = 20);

	/**
	 * Rate a shared tune
	 */
	UFUNCTION(BlueprintCallable, Category = "Sharing")
	bool RateTune(FGuid TuneID, float Rating);

	// ==========================================
	// ANALYSIS
	// ==========================================

	/**
	 * Analyze tune for issues/improvements
	 */
	UFUNCTION(BlueprintPure, Category = "Analysis")
	TArray<FString> AnalyzeTune(FGuid TuneID) const;

	/**
	 * Calculate expected performance changes
	 */
	UFUNCTION(BlueprintPure, Category = "Analysis")
	void PredictPerformanceChange(FGuid VehicleID, const FMGVehicleTuneProfile& NewTune, float& OutHPChange, float& OutHandlingChange, float& OutTopSpeedChange) const;

	/**
	 * Get recommended gear ratios for track type
	 */
	UFUNCTION(BlueprintPure, Category = "Analysis")
	TArray<float> GetRecommendedGearRatios(FGuid VehicleID, FName TrackType) const;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDynoStatusChanged OnDynoStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDynoDataPoint OnDynoDataPoint;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDynoRunComplete OnDynoRunComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTuneApplied OnTuneApplied;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTuneSaved OnTuneSaved;

protected:
	// Active dyno sessions
	UPROPERTY()
	TMap<FGuid, FMGDynoSession> ActiveSessions;

	// Dyno history
	UPROPERTY()
	TMap<FGuid, FMGDynoRunResult> DynoHistory;

	// Tune profiles
	UPROPERTY()
	TMap<FGuid, FMGVehicleTuneProfile> TuneProfiles;

	// Active tunes per vehicle
	UPROPERTY()
	TMap<FGuid, FGuid> ActiveVehicleTunes;

	// Public tunes index
	UPROPERTY()
	TArray<FGuid> PublicTuneIDs;

	// ==========================================
	// INTERNAL
	// ==========================================

	void TickDynoSession(FGuid SessionID, float DeltaTime);
	FMGDynoDataPoint SimulateDynoDataPoint(FGuid VehicleID, int32 RPM) const;
	FMGDynoRunResult CalculateDynoResult(const FMGDynoSession& Session) const;
	void ApplyTuneToVehicle(FGuid VehicleID, const FMGVehicleTuneProfile& Tune);
	FMGVehicleTuneProfile GeneratePresetTune(EMGDrivingStylePreset Preset) const;

	// Timer handle for dyno simulation
	FTimerHandle DynoTickTimerHandle;

	// Dyno simulation settings
	static constexpr float DynoTickInterval = 0.05f; // 20 Hz update
	static constexpr int32 DynoStartRPM = 2000;
	static constexpr int32 DynoEndRPM = 9000;
	static constexpr int32 DynoRPMStep = 100;
};

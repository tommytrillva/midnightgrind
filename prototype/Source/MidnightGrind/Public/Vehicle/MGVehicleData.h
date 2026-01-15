// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGVehicleData.generated.h"

/**
 * Performance class enumeration
 */
UENUM(BlueprintType)
enum class EMGPerformanceClass : uint8
{
	D UMETA(DisplayName = "D Class (100-299 PI)"),
	C UMETA(DisplayName = "C Class (300-449 PI)"),
	B UMETA(DisplayName = "B Class (450-599 PI)"),
	A UMETA(DisplayName = "A Class (600-749 PI)"),
	S UMETA(DisplayName = "S Class (750-900 PI)"),
	X UMETA(DisplayName = "X Class (901+ PI)")
};

/**
 * Part tier enumeration
 */
UENUM(BlueprintType)
enum class EMGPartTier : uint8
{
	Stock UMETA(DisplayName = "Stock"),
	Street UMETA(DisplayName = "Street"),
	Sport UMETA(DisplayName = "Sport"),
	Race UMETA(DisplayName = "Race"),
	Pro UMETA(DisplayName = "Pro"),
	Legendary UMETA(DisplayName = "Legendary")
};

/**
 * Engine configuration type
 */
UENUM(BlueprintType)
enum class EMGEngineType : uint8
{
	I4 UMETA(DisplayName = "Inline 4"),
	I6 UMETA(DisplayName = "Inline 6"),
	V6 UMETA(DisplayName = "V6"),
	V8 UMETA(DisplayName = "V8"),
	Rotary UMETA(DisplayName = "Rotary"),
	Flat4 UMETA(DisplayName = "Flat 4 (Boxer)"),
	Flat6 UMETA(DisplayName = "Flat 6 (Boxer)")
};

/**
 * Drivetrain type
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
	FWD UMETA(DisplayName = "Front Wheel Drive"),
	RWD UMETA(DisplayName = "Rear Wheel Drive"),
	AWD UMETA(DisplayName = "All Wheel Drive")
};

/**
 * Forced induction type
 */
UENUM(BlueprintType)
enum class EMGForcedInductionType : uint8
{
	None UMETA(DisplayName = "Naturally Aspirated"),
	Turbo_Single UMETA(DisplayName = "Single Turbo"),
	Turbo_Twin UMETA(DisplayName = "Twin Turbo"),
	Supercharger_Roots UMETA(DisplayName = "Roots Supercharger"),
	Supercharger_TwinScrew UMETA(DisplayName = "Twin-Screw Supercharger"),
	Supercharger_Centrifugal UMETA(DisplayName = "Centrifugal Supercharger")
};

/**
 * Differential type
 */
UENUM(BlueprintType)
enum class EMGDifferentialType : uint8
{
	Open UMETA(DisplayName = "Open"),
	LSD_1Way UMETA(DisplayName = "1-Way LSD"),
	LSD_1_5Way UMETA(DisplayName = "1.5-Way LSD"),
	LSD_2Way UMETA(DisplayName = "2-Way LSD"),
	Torsen UMETA(DisplayName = "Torsen"),
	Locked UMETA(DisplayName = "Locked/Welded")
};

/**
 * Tire compound type
 */
UENUM(BlueprintType)
enum class EMGTireCompound : uint8
{
	Economy UMETA(DisplayName = "Economy"),
	AllSeason UMETA(DisplayName = "All-Season"),
	Sport UMETA(DisplayName = "Sport"),
	Performance UMETA(DisplayName = "Performance"),
	SemiSlick UMETA(DisplayName = "Semi-Slick"),
	Slick UMETA(DisplayName = "Slick"),
	DragRadial UMETA(DisplayName = "Drag Radial"),
	Drift UMETA(DisplayName = "Drift")
};

/**
 * Ownership record for vehicle history
 */
USTRUCT(BlueprintType)
struct FMGOwnershipRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid OwnerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AcquiredDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SoldDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AcquisitionMethod; // "Purchase", "PinkSlip", "Trade", etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MileageAtAcquisition;
};

/**
 * Power curve data point
 */
USTRUCT(BlueprintType)
struct FMGPowerCurvePoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RPM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Horsepower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Torque;
};

/**
 * Complete power curve
 */
USTRUCT(BlueprintType)
struct FMGPowerCurve
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPowerCurvePoint> CurvePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PeakHPRPM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakTorque;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PeakTorqueRPM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Redline;
};

/**
 * Forced induction configuration
 */
USTRUCT(BlueprintType)
struct FMGForcedInductionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGForcedInductionType Type = EMGForcedInductionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TurboID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxBoostPSI = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpoolTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BoostThresholdRPM = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WastegateID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BlowOffValveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName IntercoolerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IntercoolerEfficiency = 0.85f;
};

/**
 * Nitrous configuration
 */
USTRUCT(BlueprintType)
struct FMGNitrousConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInstalled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SystemID;

	// Dry, Wet, DirectPort
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SystemType = TEXT("Wet");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShotSizeHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BottleSizeLbs = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFillPercent = 100.0f;
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

	// ECU
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ECUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TuneLevel = 0; // 0=Stock, 1=Stage1, 2=Stage2, 3=Custom

	// Forced induction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGForcedInductionConfig ForcedInduction;

	// Nitrous
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGNitrousConfig Nitrous;
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
 * Aerodynamic configuration
 */
USTRUCT(BlueprintType)
struct FMGAeroConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrontSplitterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontDownforceCoefficient = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RearWingID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearDownforceCoefficient = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WingAngle = 0.0f; // degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DiffuserID;

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

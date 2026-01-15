// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGVehicleContentAssets.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UMaterialInterface;
class USoundBase;
class UNiagaraSystem;
class UAnimMontage;

/**
 * Vehicle class/category
 */
UENUM(BlueprintType)
enum class EMGVehicleClass : uint8
{
	/** Starter vehicles - balanced */
	D_Class,
	/** Improved performance */
	C_Class,
	/** Sports cars */
	B_Class,
	/** Supercars */
	A_Class,
	/** Hypercars */
	S_Class,
	/** Legendary vehicles */
	S_Plus
};

/**
 * Vehicle drivetrain type
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
	FWD,  // Front-wheel drive
	RWD,  // Rear-wheel drive
	AWD   // All-wheel drive
};

/**
 * Vehicle body style
 */
UENUM(BlueprintType)
enum class EMGBodyStyle : uint8
{
	Coupe,
	Sedan,
	Hatchback,
	SUV,
	Truck,
	Muscle,
	Sports,
	Supercar,
	Classic,
	JDM
};

/**
 * Engine specification
 */
USTRUCT(BlueprintType)
struct FMGEngineSpec
{
	GENERATED_BODY()

	/** Engine name (e.g., "2.0L Turbocharged I4") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EngineName;

	/** Displacement in liters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Displacement = 2.0f;

	/** Cylinder count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cylinders = 4;

	/** Is turbocharged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTurbocharged = false;

	/** Is supercharged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSupercharged = false;

	/** Base horsepower */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Horsepower = 200.0f;

	/** Base torque (Nm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Torque = 250.0f;

	/** Redline RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RedlineRPM = 7000.0f;

	/** Idle RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdleRPM = 800.0f;

	/** Power band start RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerBandStart = 4000.0f;

	/** Power band peak RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerBandPeak = 6000.0f;
};

/**
 * Transmission specification
 */
USTRUCT(BlueprintType)
struct FMGTransmissionSpec
{
	GENERATED_BODY()

	/** Number of gears */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GearCount = 6;

	/** Gear ratios */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> GearRatios;

	/** Final drive ratio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalDriveRatio = 3.5f;

	/** Shift time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShiftTime = 0.2f;

	/** Is automatic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomatic = true;

	FMGTransmissionSpec()
	{
		// Default 6-speed ratios
		GearRatios = { 3.5f, 2.2f, 1.5f, 1.1f, 0.9f, 0.75f };
	}
};

/**
 * Handling characteristics
 */
USTRUCT(BlueprintType)
struct FMGHandlingSpec
{
	GENERATED_BODY()

	/** Drivetrain type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDrivetrainType Drivetrain = EMGDrivetrainType::RWD;

	/** Steering sensitivity (0.5-2.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float SteeringSensitivity = 1.0f;

	/** Steering speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringSpeed = 5.0f;

	/** Maximum steering angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSteerAngle = 35.0f;

	/** Grip level (0.5-2.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float GripMultiplier = 1.0f;

	/** Drift propensity (0-1, higher = easier drift) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DriftPropensity = 0.5f;

	/** Drift angle sustainability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftStability = 0.7f;

	/** Downforce coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceCoefficient = 0.5f;

	/** Weight distribution (front, 0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WeightDistribution = 0.5f;
};

/**
 * Performance stats (normalized 0-100)
 */
USTRUCT(BlueprintType)
struct FMGPerformanceStats
{
	GENERATED_BODY()

	/** Top speed rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 TopSpeed = 50;

	/** Acceleration rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Acceleration = 50;

	/** Handling rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Handling = 50;

	/** Braking rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Braking = 50;

	/** Drift rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Drift = 50;

	/** NOS power rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	int32 Nitro = 50;

	/** Get overall rating */
	int32 GetOverallRating() const
	{
		return (TopSpeed + Acceleration + Handling + Braking + Drift + Nitro) / 6;
	}
};

/**
 * Visual customization slot
 */
USTRUCT(BlueprintType)
struct FMGCustomizationSlot
{
	GENERATED_BODY()

	/** Slot identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Bone/socket to attach to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachmentSocket;

	/** Default mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* DefaultMesh;

	/** Available meshes for this slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UStaticMesh*> AvailableMeshes;
};

/**
 * Paint/livery option
 */
USTRUCT(BlueprintType)
struct FMGPaintOption
{
	GENERATED_BODY()

	/** Paint ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PaintID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Base color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BaseColor = FLinearColor::White;

	/** Metallic value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Metallic = 0.5f;

	/** Roughness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Roughness = 0.3f;

	/** Clearcoat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Clearcoat = 0.8f;

	/** Is special/premium paint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPremium = false;

	/** Unlock price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockPrice = 0;
};

/**
 * Vehicle unlock requirements
 */
USTRUCT(BlueprintType)
struct FMGVehicleUnlockRequirements
{
	GENERATED_BODY()

	/** Is available by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlockedByDefault = false;

	/** Required player level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/** Required reputation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredReputation = 0;

	/** Purchase price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PurchasePrice = 10000;

	/** Required achievement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredAchievement;

	/** Required vehicle to own first */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicle;
};

/**
 * Vehicle Data Asset
 * Complete definition of a vehicle
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// IDENTITY
	// ==========================================

	/** Unique vehicle ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName VehicleID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText VehicleName;

	/** Manufacturer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Manufacturer;

	/** Year */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	int32 Year = 2024;

	/** Vehicle class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGVehicleClass VehicleClass = EMGVehicleClass::D_Class;

	/** Body style */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGBodyStyle BodyStyle = EMGBodyStyle::Coupe;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	// ==========================================
	// VISUALS
	// ==========================================

	/** Vehicle mesh */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	USkeletalMesh* VehicleMesh;

	/** Thumbnail image */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	UTexture2D* Thumbnail;

	/** Showroom preview mesh */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	UStaticMesh* ShowroomMesh;

	/** Default material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	UMaterialInterface* BaseMaterial;

	/** Available paint options */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TArray<FMGPaintOption> PaintOptions;

	/** Customization slots */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TArray<FMGCustomizationSlot> CustomizationSlots;

	// ==========================================
	// SPECS
	// ==========================================

	/** Engine specification */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGEngineSpec Engine;

	/** Transmission specification */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGTransmissionSpec Transmission;

	/** Handling characteristics */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGHandlingSpec Handling;

	/** Performance stats */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGPerformanceStats Stats;

	/** Vehicle mass (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	float Mass = 1400.0f;

	/** Top speed (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	float TopSpeedKPH = 250.0f;

	/** 0-100 KPH time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	float ZeroToHundredTime = 5.0f;

	// ==========================================
	// NOS
	// ==========================================

	/** NOS capacity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NOS")
	float NOSCapacity = 100.0f;

	/** NOS power boost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NOS")
	float NOSPowerBoost = 1.5f;

	/** NOS consumption rate */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NOS")
	float NOSConsumptionRate = 20.0f;

	/** NOS recharge rate */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NOS")
	float NOSRechargeRate = 5.0f;

	// ==========================================
	// AUDIO
	// ==========================================

	/** Engine sound base */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* EngineSound;

	/** Engine startup sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* StartupSound;

	/** Turbo whistle/BOV sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* TurboSound;

	/** Exhaust backfire sound */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* BackfireSound;

	// ==========================================
	// VFX
	// ==========================================

	/** Exhaust effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* ExhaustEffect;

	/** Tire smoke effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* TireSmokeEffect;

	/** NOS effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* NOSEffect;

	// ==========================================
	// UNLOCK
	// ==========================================

	/** Unlock requirements */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	FMGVehicleUnlockRequirements UnlockRequirements;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Get formatted specs string */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	FText GetFormattedSpecs() const;

	/** Get class display name */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	FText GetClassDisplayName() const;
};

/**
 * Vehicle Collection Asset
 * Groups vehicles by manufacturer or theme
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleCollectionAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Collection ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FName CollectionID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FText CollectionName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FText Description;

	/** Vehicles in collection */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	TArray<TSoftObjectPtr<UMGVehicleDataAsset>> Vehicles;

	/** Collection thumbnail */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	UTexture2D* Thumbnail;

	/** Bonus for completing collection */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	int32 CompletionBonus = 0;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

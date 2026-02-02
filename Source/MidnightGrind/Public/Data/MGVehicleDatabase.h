// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/MGSharedTypes.h"
#include "MGVehicleDatabase.generated.h"

// EMGBodyStyle - REMOVED (duplicate)
// Canonical definition in: Content/MGVehicleContentAssets.h

// EMGDrivetrain - REMOVED (duplicate)
// Canonical definition in: Catalog/MGCatalogTypes.h

/**
 * Engine configuration
 */
UENUM(BlueprintType)
enum class EMGEngineConfig : uint8
{
	I4 UMETA(DisplayName = "Inline 4"),
	I5 UMETA(DisplayName = "Inline 5"),
	I6 UMETA(DisplayName = "Inline 6"),
	V6 UMETA(DisplayName = "V6"),
	V8 UMETA(DisplayName = "V8"),
	V10 UMETA(DisplayName = "V10"),
	V12 UMETA(DisplayName = "V12"),
	Flat4 UMETA(DisplayName = "Flat 4 (Boxer)"),
	Flat6 UMETA(DisplayName = "Flat 6 (Boxer)"),
	Rotary UMETA(DisplayName = "Rotary"),
	Electric UMETA(DisplayName = "Electric")
};

/**
 * Aspiration type
 */
UENUM(BlueprintType)
enum class EMGAspiration : uint8
{
	NaturallyAspirated UMETA(DisplayName = "Naturally Aspirated"),
	Turbocharged UMETA(DisplayName = "Turbocharged"),
	TwinTurbo UMETA(DisplayName = "Twin Turbo"),
	Supercharged UMETA(DisplayName = "Supercharged"),
	TwinCharged UMETA(DisplayName = "Twincharged")
};

// EMGTransmissionType - MOVED TO MGSharedTypes.h
// UENUM(BlueprintType)
// enum class EMGTransmissionType : uint8
// {
// 	Manual UMETA(DisplayName = "Manual"),
// 	Automatic UMETA(DisplayName = "Automatic"),
// 	DCT UMETA(DisplayName = "Dual-Clutch"),
// 	CVT UMETA(DisplayName = "CVT"),
// 	Sequential UMETA(DisplayName = "Sequential")
// };

// EMGVehicleEra defined in Core/MGCoreEnums.h

/**
 * Vehicle rarity
 */
UENUM(BlueprintType)
enum class EMGVehicleRarity : uint8
{
	Common UMETA(DisplayName = "Common"),
	Uncommon UMETA(DisplayName = "Uncommon"),
	Rare UMETA(DisplayName = "Rare"),
	Epic UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary"),
	Mythic UMETA(DisplayName = "Mythic")
};

/**
 * @brief Engine specifications defining powerplant characteristics.
 *
 * Contains all data about a vehicle's engine including configuration,
 * aspiration, displacement, and tuning potential. Used for physics
 * calculations, sound selection, and customization validation.
 */
USTRUCT(BlueprintType)
struct FMGEngineSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	EMGEngineConfig Configuration = EMGEngineConfig::I4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	EMGAspiration Aspiration = EMGAspiration::NaturallyAspirated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine", meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float DisplacementLiters = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	FString EngineName; // e.g., "2JZ-GTE", "RB26DETT", "SR20DET"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	int32 StockHorsepower = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	int32 StockTorque = 200; // lb-ft

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	int32 Redline = 7000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	int32 PeakHPRPM = 6000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine")
	int32 PeakTorqueRPM = 4500;

	// Tuning potential
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	int32 MaxPotentialHP = 500; // With full mods

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float TuningDifficulty = 1.0f; // 1.0 = average, higher = harder
};

/**
 * @brief Transmission specifications defining gearbox characteristics.
 *
 * Contains gear ratios, final drive, and transmission type data.
 * Used for acceleration calculations and drivetrain simulation.
 */
USTRUCT(BlueprintType)
struct FMGTransmissionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	EMGTransmissionType Type = EMGTransmissionType::Manual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	int32 NumGears = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	TArray<float> GearRatios;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	float FinalDrive = 3.73f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	float ReverseRatio = -3.0f;
};

/**
 * @brief Suspension specifications defining handling geometry.
 *
 * Contains suspension type, stiffness, and vehicle dimensions.
 * Used for handling physics, ride height calculations, and wheel fitment.
 */
USTRUCT(BlueprintType)
struct FMGSuspensionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	FString FrontType; // e.g., "MacPherson Strut", "Double Wishbone"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	FString RearType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float FrontStiffness = 50.0f; // N/mm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float RearStiffness = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float RideHeight = 140.0f; // mm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float Wheelbase = 2600.0f; // mm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float FrontTrack = 1500.0f; // mm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float RearTrack = 1480.0f;
};

/**
 * @brief Weight distribution and mass properties.
 *
 * Defines curb weight, front/rear bias, and center of gravity height.
 * Critical for handling physics, weight transfer, and acceleration.
 */
USTRUCT(BlueprintType)
struct FMGWeightSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float CurbWeight = 1400.0f; // kg

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float FrontWeightBias = 55.0f; // Percentage

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float CenterOfGravityHeight = 500.0f; // mm from ground
};

/**
 * @brief Stock performance statistics and benchmarks.
 *
 * Contains factory performance data including acceleration times,
 * top speed, and handling metrics. Used for PI calculation and
 * vehicle comparison displays.
 */
USTRUCT(BlueprintType)
struct FMGPerformanceSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float ZeroToSixty = 6.5f; // seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float QuarterMile = 14.5f; // seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float QuarterMileSpeed = 98.0f; // mph trap speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float TopSpeed = 155.0f; // mph

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float SkidpadG = 0.85f; // lateral G

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float BrakingDistance60 = 120.0f; // feet from 60 mph
};

/**
 * @brief Visual customization options available for a vehicle.
 *
 * Defines the number of body kit options and special features
 * available for customization. Used by the customization UI to
 * populate available parts and validate fitment.
 */
USTRUCT(BlueprintType)
struct FMGCustomizationOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	bool bHasWidebodyKit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 NumFrontBumperOptions = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 NumRearBumperOptions = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 NumSideSkirtOptions = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 NumHoodOptions = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 NumSpoilerOptions = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 NumRoofOptions = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	bool bCanRemoveRoof = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	bool bHasPopupHeadlights = false;
};

/**
 * @brief Sound configuration for vehicle audio.
 *
 * Contains soft references to audio assets for engine sounds,
 * turbo effects, and exhaust notes. Used by the audio subsystem
 * to initialize vehicle-specific sound components.
 */
USTRUCT(BlueprintType)
struct FMGSoundConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> EngineIdleSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> EngineRevSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> ExhaustSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> TurboSpoolSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> BlowOffSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> BackfireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float BasePitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float ExhaustVolume = 1.0f;
};

/**
 * Complete vehicle definition data asset
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// IDENTIFICATION
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName VehicleID; // Unique identifier

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGVehicleMake Make = EMGVehicleMake::Generic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Model; // e.g., "Skyline", "Supra", "RX-7"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Variant; // e.g., "GT-R", "Turbo", "Type R"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Generation; // e.g., "R34", "A80", "FD"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	int32 Year = 1999;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGVehicleEra Era = EMGVehicleEra::GoldenAge;

	// ==========================================
	// CLASSIFICATION
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EMGBodyStyle BodyStyle = EMGBodyStyle::Coupe;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EMGDrivetrain Drivetrain = EMGDrivetrain::RWD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	EMGVehicleRarity Rarity = EMGVehicleRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	int32 BasePI = 400; // Performance Index stock

	// ==========================================
	// SPECIFICATIONS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGEngineSpec Engine;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGTransmissionSpec Transmission;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGSuspensionSpec Suspension;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGWeightSpec Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Specs")
	FMGPerformanceSpec StockPerformance;

	// ==========================================
	// VISUALS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<USkeletalMesh> VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UPhysicsAsset> PhysicsAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> ThumbnailImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> ShowroomImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	FMGCustomizationOptions CustomizationOptions;

	// ==========================================
	// AUDIO
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FMGSoundConfig Sounds;

	// ==========================================
	// ECONOMY
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int64 PurchasePrice = 25000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int64 SellValue = 18000; // Base sell price (stock)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 RequiredREP = 0; // REP needed to purchase

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 RequiredLevel = 1;

	// ==========================================
	// UNLOCK CONDITIONS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	bool bIsStarterVehicle = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	bool bRequiresPinkSlipWin = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	FName RequiredAchievement; // Achievement to unlock

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	FString UnlockDescription;

	// ==========================================
	// LORE/FLAVOR
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lore", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lore", meta = (MultiLine = true))
	FText HistoryText; // Real-world history/trivia

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lore")
	TArray<FString> FamousOwners; // Pop culture references

	// ==========================================
	// HELPERS
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	FString GetDisplayName() const
	{
		return FString::Printf(TEXT("%d %s %s"), Year, *GetMakeName(), *Model);
	}

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	FString GetFullName() const
	{
		if (Variant.IsEmpty())
		{
			return GetDisplayName();
		}
		return FString::Printf(TEXT("%d %s %s %s"), Year, *GetMakeName(), *Model, *Variant);
	}

	UFUNCTION(BlueprintPure, Category = "Vehicle")
	FString GetMakeName() const
	{
		// Convert enum to display string
		const UEnum* EnumPtr = StaticEnum<EMGVehicleMake>();
		return EnumPtr ? EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Make)).ToString() : TEXT("Unknown");
	}

	// Primary asset type for asset manager
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("VehicleDefinition", VehicleID);
	}
};

/**
 * Vehicle database - holds all vehicle definitions
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleDatabase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Database")
	TArray<TSoftObjectPtr<UMGVehicleDefinition>> AllVehicles;

	UFUNCTION(BlueprintCallable, Category = "Database")
	UMGVehicleDefinition* GetVehicleByID(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMGVehicleDefinition*> GetVehiclesByMake(EMGVehicleMake Make) const;

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMGVehicleDefinition*> GetVehiclesByEra(EMGVehicleEra Era) const;

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMGVehicleDefinition*> GetStarterVehicles() const;

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMGVehicleDefinition*> GetVehiclesInPriceRange(int64 MinPrice, int64 MaxPrice) const;

	UFUNCTION(BlueprintCallable, Category = "Database")
	TArray<UMGVehicleDefinition*> GetVehiclesInPIRange(int32 MinPI, int32 MaxPI) const;
};

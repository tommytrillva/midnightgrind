// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehicleDatabase.h
 * @brief Vehicle database and definition system for MIDNIGHT GRIND.
 *
 * This file contains the complete vehicle catalog system including:
 * - Vehicle identity (make, model, year, generation)
 * - Classification (body style, drivetrain, rarity, performance index)
 * - Technical specifications (engine, transmission, suspension, weight)
 * - Performance data (0-60, quarter mile, top speed, lateral G)
 * - Visual assets and customization options
 * - Audio configuration
 * - Economy data (pricing, unlock requirements)
 * - Lore and flavor text
 *
 * The database system uses Unreal Engine's Primary Data Asset system for
 * efficient loading and asset management. Vehicles are defined as data assets
 * and loaded on-demand through the Asset Manager.
 *
 * @note This system focuses on Y2K-era Japanese tuner culture (1990-2009),
 *       featuring iconic vehicles from the golden age of street racing.
 *
 * @see UMGVehicleDefinition For individual vehicle data
 * @see UMGVehicleDatabase For the master vehicle catalog
 * @see FMGVehicleData For runtime vehicle configuration (MGVehicleData.h)
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/MGCoreEnums.h"
#include "MGVehicleDatabase.generated.h"

// ============================================================================
// VEHICLE IDENTITY ENUMS
// ============================================================================

/**
 * @brief Vehicle manufacturer/make enumeration.
 *
 * Defines all supported vehicle manufacturers in MIDNIGHT GRIND.
 * Organized by region of origin for easy categorization.
 */
UENUM(BlueprintType)
enum class EMGVehicleMake : uint8
{
	/// @name Japanese Manufacturers
	/// @{
	Nissan UMETA(DisplayName = "Nissan"),           ///< Nissan Motor Co. (Skyline, 240SX, 350Z)
	Toyota UMETA(DisplayName = "Toyota"),           ///< Toyota Motor Corp. (Supra, AE86, MR2)
	Honda UMETA(DisplayName = "Honda"),             ///< Honda Motor Co. (Civic, S2000, NSX)
	Mazda UMETA(DisplayName = "Mazda"),             ///< Mazda Motor Corp. (RX-7, Miata, RX-8)
	Mitsubishi UMETA(DisplayName = "Mitsubishi"),   ///< Mitsubishi Motors (Lancer Evo, Eclipse, 3000GT)
	Subaru UMETA(DisplayName = "Subaru"),           ///< Subaru Corp. (WRX STI, Legacy, BRZ)
	Lexus UMETA(DisplayName = "Lexus"),             ///< Toyota's luxury division (IS300, SC300)
	Acura UMETA(DisplayName = "Acura"),             ///< Honda's luxury division (Integra, NSX)
	Infiniti UMETA(DisplayName = "Infiniti"),       ///< Nissan's luxury division (G35, Q45)
	/// @}

	/// @name American Manufacturers
	/// @{
	Ford UMETA(DisplayName = "Ford"),               ///< Ford Motor Company (Mustang, Focus, GT)
	Chevrolet UMETA(DisplayName = "Chevrolet"),     ///< General Motors (Camaro, Corvette, Cobalt SS)
	Dodge UMETA(DisplayName = "Dodge"),             ///< Stellantis (Viper, Neon SRT-4, Charger)
	Pontiac UMETA(DisplayName = "Pontiac"),         ///< General Motors (GTO, Firebird, G8)
	Shelby UMETA(DisplayName = "Shelby"),           ///< Shelby American (GT500, GT350)
	/// @}

	/// @name European Manufacturers
	/// @{
	BMW UMETA(DisplayName = "BMW"),                 ///< Bayerische Motoren Werke (E46 M3, E30)
	Mercedes UMETA(DisplayName = "Mercedes-Benz"),  ///< Mercedes-Benz AG (C63 AMG, SL)
	Audi UMETA(DisplayName = "Audi"),               ///< Audi AG (S4, RS4, TT)
	Volkswagen UMETA(DisplayName = "Volkswagen"),   ///< Volkswagen AG (Golf R32, Jetta GLI)
	Porsche UMETA(DisplayName = "Porsche"),         ///< Porsche AG (911, Cayman, Boxster)
	/// @}

	/// @name Korean Manufacturers
	/// @{
	Hyundai UMETA(DisplayName = "Hyundai"),         ///< Hyundai Motor Group (Genesis Coupe, Tiburon)
	Kia UMETA(DisplayName = "Kia"),                 ///< Kia Corporation (Stinger)
	/// @}

	/// @name Generic/Fictional
	/// @{
	Generic UMETA(DisplayName = "Generic")          ///< Generic/fictional vehicles for testing
	/// @}
};

/**
 * @brief Vehicle body style classification.
 *
 * Defines the physical form factor of the vehicle, affecting both
 * visual appearance and some physics characteristics (weight distribution,
 * center of gravity height, drag coefficient).
 */
UENUM(BlueprintType)
enum class EMGBodyStyle : uint8
{
	Coupe UMETA(DisplayName = "Coupe"),             ///< Two-door sports car (Supra, RX-7, 350Z)
	Sedan UMETA(DisplayName = "Sedan"),             ///< Four-door sedan (Skyline R34, Lancer Evo)
	Hatchback UMETA(DisplayName = "Hatchback"),     ///< Compact hatchback (Civic Si, Golf R32)
	Wagon UMETA(DisplayName = "Wagon"),             ///< Station wagon (Legacy GT, IS SportCross)
	Convertible UMETA(DisplayName = "Convertible"), ///< Open-top vehicle (S2000, Miata)
	Truck UMETA(DisplayName = "Truck"),             ///< Pickup truck (Ford Lightning, SRT-10)
	SUV UMETA(DisplayName = "SUV"),                 ///< Sport utility vehicle (Evo X SUV concept)
	Van UMETA(DisplayName = "Van")                  ///< Van/minivan (Odyssey, Alphard)
};

/**
 * Drivetrain configuration
 */
UENUM(BlueprintType)
enum class EMGDrivetrain : uint8
{
	FWD UMETA(DisplayName = "Front-Wheel Drive"),
	RWD UMETA(DisplayName = "Rear-Wheel Drive"),
	AWD UMETA(DisplayName = "All-Wheel Drive"),
	FourWD UMETA(DisplayName = "4-Wheel Drive")
};

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

/**
 * Transmission type
 */
UENUM(BlueprintType)
enum class EMGTransmissionType : uint8
{
	Manual UMETA(DisplayName = "Manual"),
	Automatic UMETA(DisplayName = "Automatic"),
	DCT UMETA(DisplayName = "Dual-Clutch"),
	CVT UMETA(DisplayName = "CVT"),
	Sequential UMETA(DisplayName = "Sequential")
};

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
 * Engine specifications
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
 * Transmission specifications
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
 * Suspension specifications
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
 * Weight distribution
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
 * Performance statistics (stock)
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
 * Visual customization options
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
 * Sound configuration
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

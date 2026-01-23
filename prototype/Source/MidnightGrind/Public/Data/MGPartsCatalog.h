// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGVehicleDatabase.h"
#include "MGPartsCatalog.generated.h"

/**
 * Part category
 */
UENUM(BlueprintType)
enum class EMGPartCategory : uint8
{
	// Engine
	Engine_Internals UMETA(DisplayName = "Engine Internals"),
	Engine_Intake UMETA(DisplayName = "Intake"),
	Engine_Exhaust UMETA(DisplayName = "Exhaust"),
	Engine_ForcedInduction UMETA(DisplayName = "Forced Induction"),
	Engine_FuelSystem UMETA(DisplayName = "Fuel System"),
	Engine_ECU UMETA(DisplayName = "ECU/Engine Management"),
	Engine_Cooling UMETA(DisplayName = "Cooling"),

	// Drivetrain
	Drivetrain_Clutch UMETA(DisplayName = "Clutch"),
	Drivetrain_Flywheel UMETA(DisplayName = "Flywheel"),
	Drivetrain_Transmission UMETA(DisplayName = "Transmission"),
	Drivetrain_Differential UMETA(DisplayName = "Differential"),
	Drivetrain_Driveshaft UMETA(DisplayName = "Driveshaft"),
	Drivetrain_Axles UMETA(DisplayName = "Axles/CV"),

	// Suspension
	Suspension_Springs UMETA(DisplayName = "Springs"),
	Suspension_Dampers UMETA(DisplayName = "Dampers/Shocks"),
	Suspension_Coilovers UMETA(DisplayName = "Coilovers"),
	Suspension_SwayBars UMETA(DisplayName = "Sway Bars"),
	Suspension_Arms UMETA(DisplayName = "Control Arms"),
	Suspension_Bushings UMETA(DisplayName = "Bushings"),

	// Brakes
	Brakes_Pads UMETA(DisplayName = "Brake Pads"),
	Brakes_Rotors UMETA(DisplayName = "Brake Rotors"),
	Brakes_Calipers UMETA(DisplayName = "Brake Calipers"),
	Brakes_Lines UMETA(DisplayName = "Brake Lines"),
	Brakes_BigBrakeKit UMETA(DisplayName = "Big Brake Kit"),

	// Wheels/Tires
	Wheels_Rims UMETA(DisplayName = "Wheels"),
	Wheels_Tires UMETA(DisplayName = "Tires"),
	Wheels_Spacers UMETA(DisplayName = "Wheel Spacers"),

	// Aero
	Aero_FrontBumper UMETA(DisplayName = "Front Bumper"),
	Aero_RearBumper UMETA(DisplayName = "Rear Bumper"),
	Aero_SideSkirts UMETA(DisplayName = "Side Skirts"),
	Aero_Spoiler UMETA(DisplayName = "Spoiler/Wing"),
	Aero_Hood UMETA(DisplayName = "Hood"),
	Aero_Diffuser UMETA(DisplayName = "Diffuser"),
	Aero_Splitter UMETA(DisplayName = "Front Splitter"),
	Aero_Canards UMETA(DisplayName = "Canards"),
	Aero_WidebodyKit UMETA(DisplayName = "Widebody Kit"),

	// Interior
	Interior_Seats UMETA(DisplayName = "Seats"),
	Interior_Harness UMETA(DisplayName = "Harness"),
	Interior_Cage UMETA(DisplayName = "Roll Cage"),
	Interior_Steering UMETA(DisplayName = "Steering Wheel"),
	Interior_Gauges UMETA(DisplayName = "Gauges"),
	Interior_ShiftKnob UMETA(DisplayName = "Shift Knob"),

	// Nitrous
	Nitrous_Kit UMETA(DisplayName = "Nitrous Kit"),
	Nitrous_Bottle UMETA(DisplayName = "Nitrous Bottle"),
	Nitrous_Purge UMETA(DisplayName = "Purge Kit"),

	// Lighting
	Lighting_Headlights UMETA(DisplayName = "Headlights"),
	Lighting_Taillights UMETA(DisplayName = "Taillights"),
	Lighting_Underglow UMETA(DisplayName = "Underglow"),

	// Audio
	Audio_Speakers UMETA(DisplayName = "Speakers"),
	Audio_Subwoofer UMETA(DisplayName = "Subwoofer"),
	Audio_HeadUnit UMETA(DisplayName = "Head Unit"),

	// Weight Reduction
	Weight_Carbon UMETA(DisplayName = "Carbon Fiber Parts"),
	Weight_StripInterior UMETA(DisplayName = "Interior Strip")
};

/**
 * Part quality/tier
 */
UENUM(BlueprintType)
enum class EMGPartTier : uint8
{
	Stock UMETA(DisplayName = "Stock"),
	Street UMETA(DisplayName = "Street"),
	Sport UMETA(DisplayName = "Sport"),
	Race UMETA(DisplayName = "Race"),
	Pro UMETA(DisplayName = "Pro"),
	Elite UMETA(DisplayName = "Elite"),
	Ultimate UMETA(DisplayName = "Ultimate")
};

/**
 * Part brand
 */
UENUM(BlueprintType)
enum class EMGPartBrand : uint8
{
	// Generic
	Generic UMETA(DisplayName = "Generic"),

	// Performance
	HKS UMETA(DisplayName = "HKS"),
	GReddy UMETA(DisplayName = "GReddy"),
	APEXi UMETA(DisplayName = "A'PEXi"),
	Tomei UMETA(DisplayName = "Tomei"),
	JUN UMETA(DisplayName = "JUN Auto"),
	Blitz UMETA(DisplayName = "Blitz"),
	Trust UMETA(DisplayName = "Trust"),
	NISMO UMETA(DisplayName = "NISMO"),
	TRD UMETA(DisplayName = "TRD"),
	Mugen UMETA(DisplayName = "Mugen"),
	STI UMETA(DisplayName = "STI"),
	Ralliart UMETA(DisplayName = "Ralliart"),

	// Suspension
	Tein UMETA(DisplayName = "Tein"),
	KW UMETA(DisplayName = "KW"),
	Ohlins UMETA(DisplayName = "Ohlins"),
	Bilstein UMETA(DisplayName = "Bilstein"),
	Cusco UMETA(DisplayName = "Cusco"),

	// Brakes
	Brembo UMETA(DisplayName = "Brembo"),
	Wilwood UMETA(DisplayName = "Wilwood"),
	StopTech UMETA(DisplayName = "StopTech"),
	ProjectMu UMETA(DisplayName = "Project Mu"),

	// Wheels
	Volk UMETA(DisplayName = "Volk Racing"),
	Work UMETA(DisplayName = "Work"),
	BBS UMETA(DisplayName = "BBS"),
	Enkei UMETA(DisplayName = "Enkei"),
	SSR UMETA(DisplayName = "SSR"),
	WedsSport UMETA(DisplayName = "WedsSport"),

	// Tires
	Toyo UMETA(DisplayName = "Toyo"),
	Bridgestone UMETA(DisplayName = "Bridgestone"),
	Yokohama UMETA(DisplayName = "Yokohama"),
	Nitto UMETA(DisplayName = "Nitto"),

	// Aero
	Rocket UMETA(DisplayName = "Rocket Bunny"),
	Liberty UMETA(DisplayName = "Liberty Walk"),
	Veilside UMETA(DisplayName = "Veilside"),
	TopSecret UMETA(DisplayName = "Top Secret"),
	Bomex UMETA(DisplayName = "Bomex"),
	Chargespeed UMETA(DisplayName = "ChargeSpeed"),

	// Interior
	Bride UMETA(DisplayName = "Bride"),
	Recaro UMETA(DisplayName = "Recaro"),
	Sparco UMETA(DisplayName = "Sparco"),
	Momo UMETA(DisplayName = "Momo"),
	Nardi UMETA(DisplayName = "Nardi"),

	// Nitrous
	NOS UMETA(DisplayName = "NOS"),
	Nitrous_Express UMETA(DisplayName = "Nitrous Express"),
	ZEX UMETA(DisplayName = "ZEX")
};

/**
 * Effect that a part has on vehicle stats
 */
USTRUCT(BlueprintType)
struct FMGPartEffect
{
	GENERATED_BODY()

	// Power
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	int32 HorsepowerBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	int32 TorqueBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float HorsepowerMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float TorqueMultiplier = 1.0f;

	// Weight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float WeightChange = 0.0f; // kg (positive = heavier)

	// Handling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float GripMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float TractionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float StabilityMultiplier = 1.0f;

	// Braking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float BrakingMultiplier = 1.0f;

	// Acceleration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float AccelerationMultiplier = 1.0f;

	// Top Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float TopSpeedMultiplier = 1.0f;

	// Nitrous
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float NitrousDurationBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float NitrousPowerBonus = 0.0f;

	// Performance Index impact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	int32 PIChange = 0;

	// Durability/Wear
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float WearRateMultiplier = 1.0f; // Higher = wears faster

	// Aesthetics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	int32 StylePoints = 0; // Visual appeal
};

/**
 * Tuning option for a part
 */
USTRUCT(BlueprintType)
struct FMGPartTuningOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FName OptionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float MaxValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float DefaultValue = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FString Unit; // e.g., "PSI", "mm", "%"
};

/**
 * Complete part definition
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGPartDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// IDENTIFICATION
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName PartID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGPartCategory Category = EMGPartCategory::Engine_Intake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGPartTier Tier = EMGPartTier::Street;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGPartBrand Brand = EMGPartBrand::Generic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString PartNumber; // Real-world part number reference

	// ==========================================
	// COMPATIBILITY
	// ==========================================

	// If empty, fits all vehicles
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> CompatibleVehicleIDs;

	// If empty, fits all makes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<EMGVehicleMake> CompatibleMakes;

	// If empty, fits all engine configs
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<EMGEngineConfig> CompatibleEngines;

	// Required parts to install this part
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> RequiredPartIDs;

	// Parts that conflict (can't be installed together)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compatibility")
	TArray<FName> ConflictingPartIDs;

	// ==========================================
	// EFFECTS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	FMGPartEffect Effects;

	// ==========================================
	// TUNING
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
	bool bIsTunable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning")
	TArray<FMGPartTuningOption> TuningOptions;

	// ==========================================
	// ECONOMY
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int64 PurchasePrice = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int64 InstallationCost = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int64 SellValue = 250;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 RequiredREP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
	int32 RequiredLevel = 1;

	// ==========================================
	// VISUALS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UStaticMesh> PartMesh; // For visible parts

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> ThumbnailImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	bool bHasVisualChange = false;

	// ==========================================
	// AUDIO
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> InstallSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	bool bChangesExhaustSound = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> ExhaustSoundOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	bool bChangesTurboSound = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> TurboSoundOverride;

	// ==========================================
	// DESCRIPTION
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Description", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Description")
	TArray<FString> Features; // Bullet points

	// ==========================================
	// HELPERS
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Part")
	FString GetFullName() const
	{
		const UEnum* BrandEnum = StaticEnum<EMGPartBrand>();
		FString BrandName = BrandEnum ? BrandEnum->GetDisplayNameTextByValue(static_cast<int64>(Brand)).ToString() : TEXT("Generic");
		return FString::Printf(TEXT("%s %s"), *BrandName, *DisplayName.ToString());
	}

	UFUNCTION(BlueprintPure, Category = "Part")
	bool IsCompatibleWithVehicle(const UMGVehicleDefinition* Vehicle) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("PartDefinition", PartID);
	}
};

/**
 * Wheel/Rim definition (special case)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGWheelDefinition : public UMGPartDefinition
{
	GENERATED_BODY()

public:
	// ==========================================
	// WHEEL SPECS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	FString WheelName; // e.g., "TE37", "CE28N"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	TArray<int32> AvailableDiameters; // e.g., 15, 16, 17, 18, 19

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	TArray<float> AvailableWidths; // e.g., 7.0, 8.0, 9.0, 10.0

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	TArray<int32> AvailableOffsets; // e.g., +35, +40, +45

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	float WeightPerWheel = 9.0f; // kg

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	FString Material; // e.g., "Forged Aluminum", "Cast", "Flow Formed"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	int32 SpokeCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
	TArray<FLinearColor> AvailableColors;
};

/**
 * Tire definition (special case)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTireDefinition : public UMGPartDefinition
{
	GENERATED_BODY()

public:
	// ==========================================
	// TIRE SPECS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	FString TireName; // e.g., "R888R", "AD08RS"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	FString TireType; // "Street", "Sport", "Semi-Slick", "Slick", "Drift"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	TArray<FString> AvailableSizes; // e.g., "225/45R17", "255/35R18"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	float TreadwearRating = 200.0f; // UTQG rating

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	FString TractionRating; // "AA", "A", "B", "C"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tire")
	FString TemperatureRating; // "A", "B", "C"

	// Performance characteristics
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float DryGrip = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float WetGrip = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float HeatupRate = 1.0f; // How fast tires reach optimal temp

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float OptimalTempMin = 60.0f; // Celsius

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float OptimalTempMax = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
	float WearRate = 1.0f;
};

/**
 * Turbo kit definition (special case)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTurboDefinition : public UMGPartDefinition
{
	GENERATED_BODY()

public:
	// ==========================================
	// TURBO SPECS
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	FString TurboName; // e.g., "GT3076R", "T04Z"

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	bool bIsTwinTurbo = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	float CompressorSize = 60.0f; // mm

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	float TurbineSize = 55.0f; // mm

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	float MaxBoostPSI = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	float SpoolRPM = 3500.0f; // RPM where boost starts building

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	float FullBoostRPM = 5000.0f; // RPM where max boost achieved

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	float LagFactor = 1.0f; // Higher = more lag

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turbo")
	int32 MaxSupportedHP = 600; // HP ceiling with this turbo
};

/**
 * Parts catalog database
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGPartsCatalog : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Catalog")
	TArray<TSoftObjectPtr<UMGPartDefinition>> AllParts;

	// ==========================================
	// QUERY FUNCTIONS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	UMGPartDefinition* GetPartByID(FName PartID) const;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGPartDefinition*> GetPartsByCategory(EMGPartCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGPartDefinition*> GetPartsByTier(EMGPartTier Tier) const;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGPartDefinition*> GetPartsByBrand(EMGPartBrand Brand) const;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGPartDefinition*> GetPartsForVehicle(const UMGVehicleDefinition* Vehicle) const;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGPartDefinition*> GetPartsInPriceRange(int64 MinPrice, int64 MaxPrice) const;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGPartDefinition*> GetUpgradesForCategory(const UMGVehicleDefinition* Vehicle, EMGPartCategory Category) const;

	// Specialized getters
	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGWheelDefinition*> GetAllWheels() const;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGTireDefinition*> GetAllTires() const;

	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGTurboDefinition*> GetAllTurbos() const;
};

/**
 * Installed part instance (on a player's vehicle)
 */
USTRUCT(BlueprintType)
struct FMGInstalledPart
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FName PartID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FGuid InstanceID; // Unique instance

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	float WearLevel = 0.0f; // 0-1, 1 = worn out

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FDateTime InstallDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	int64 PurchasePrice = 0; // What player paid

	// Tuning values (key = option ID, value = setting)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	TMap<FName, float> TuningValues;

	// For visual parts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 ColorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor CustomColor = FLinearColor::White;
};

/**
 * Complete vehicle build (all installed parts)
 */
USTRUCT(BlueprintType)
struct FMGVehicleBuild
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	FGuid BuildID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	FString BuildName; // Player-given name

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	TMap<EMGPartCategory, FMGInstalledPart> InstalledParts;

	// Calculated stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalHorsepower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalTorque = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TotalWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 PerformanceIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 TotalInvestment = 0; // Total spent on parts
};

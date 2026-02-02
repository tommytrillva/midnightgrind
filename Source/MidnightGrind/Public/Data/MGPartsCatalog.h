// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Vehicle/MG_VHCL_Data.h"
#include "MGVehicleDatabase.h"
#include "MGPartQuality.h"
#include "Tuning/MGPartInstallation.h"
#include "MGPartsCatalog.generated.h"

// EMGPartCategory - REMOVED (duplicate)
// Canonical definition in: Catalog/MGCatalogTypes.h

// NOTE: EMGPartTier is defined in Vehicle/MG_VHCL_Data.h
// Values: Stock, Street, Sport, Race, Pro, Legendary

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

	/**
	 * @brief Manufacturing quality tier of this part.
	 *
	 * Affects performance multipliers, durability, wear rate, weight,
	 * cost, and failure chance. See EMGPartQuality for tier descriptions.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGPartQuality Quality = EMGPartQuality::Aftermarket;

	/**
	 * @brief Brand reputation level for this manufacturer.
	 *
	 * Affects perceived quality, resale value, warranty coverage,
	 * and failure chance modifiers. Higher reputation brands command
	 * premium prices but offer better quality assurance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGBrandReputation BrandReputation = EMGBrandReputation::Standard;

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
	// INSTALLATION
	// ==========================================

	/**
	 * @brief Installation difficulty level for this part
	 *
	 * Determines base installation time, DIY success rates, and
	 * whether the part can be installed by the player or requires
	 * professional shop service.
	 *
	 * @see EMGInstallDifficulty for difficulty descriptions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation")
	EMGInstallDifficulty InstallDifficulty = EMGInstallDifficulty::Moderate;

	/**
	 * @brief Base installation time in minutes
	 *
	 * Time estimates by difficulty:
	 * - Simple: 15 min (air filters, shift knobs)
	 * - Moderate: 60 min (exhaust, brake pads)
	 * - Complex: 240 min (turbo kits, big brakes)
	 * - Expert: 480 min (engine builds, swaps)
	 *
	 * Actual time varies based on mechanic skill and method.
	 * DIY may take longer; shop time is fixed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation", meta = (ClampMin = "5", ClampMax = "2880", UIMin = "5", UIMax = "480"))
	int32 InstallTimeMinutes = 60;

	/**
	 * @brief Whether installation requires vehicle to be on a lift
	 *
	 * Parts underneath the vehicle (exhaust, suspension, transmission)
	 * typically require lift access. If player doesn't have a lift,
	 * they must use shop installation for these parts.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation")
	bool bRequiresLift = false;

	/**
	 * @brief Whether installation requires special tools
	 *
	 * Beyond basic hand tools - torque wrenches, spring compressors,
	 * bearing pullers, etc. Affects DIY success rate if player
	 * doesn't own the required tools.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation")
	bool bRequiresSpecialTools = false;

	/**
	 * @brief List of specific tool IDs required for installation
	 *
	 * Used for checking player's tool inventory and displaying
	 * requirements in the UI. Empty if no special tools needed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation")
	TArray<FName> RequiredToolIDs;

	/**
	 * @brief Whether engine removal is required for installation
	 *
	 * Significantly increases complexity and time. Examples:
	 * rear main seal, clutch on longitudinal engines, some
	 * turbo kit installations.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation")
	bool bRequiresEngineRemoval = false;

	/**
	 * @brief Whether transmission removal is required
	 *
	 * Required for clutch replacements, flywheel swaps, some
	 * drivetrain modifications.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation")
	bool bRequiresTransmissionRemoval = false;

	/**
	 * @brief Whether dyno tuning is required after installation
	 *
	 * Performance parts affecting fuel/air mixture need tuning:
	 * turbo kits, fuel injectors, ECU upgrades, etc.
	 * Adds additional cost to shop installations.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation")
	bool bRequiresDynoTuning = false;

	/**
	 * @brief Labor cost multiplier for this specific part
	 *
	 * Defaults to 1.0 (standard rate). Higher for parts that are
	 * particularly difficult to access or require extra care.
	 * Lower for simple bolt-on parts.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Installation", meta = (ClampMin = "0.5", ClampMax = "3.0", UIMin = "0.5", UIMax = "3.0"))
	float LaborCostMultiplier = 1.0f;

	/**
	 * @brief Convert part installation settings to requirements struct
	 *
	 * Creates an FMGInstallationRequirements struct from this part's
	 * installation properties, for use with the installation subsystem.
	 *
	 * @return Installation requirements for this part
	 */
	UFUNCTION(BlueprintPure, Category = "Installation")
	FMGInstallationRequirements GetInstallationRequirements() const
	{
		FMGInstallationRequirements Requirements;
		Requirements.Difficulty = InstallDifficulty;
		Requirements.InstallTimeMinutes = InstallTimeMinutes;
		Requirements.bRequiresLift = bRequiresLift;
		Requirements.bRequiresSpecialTools = bRequiresSpecialTools;
		Requirements.RequiredToolIDs = RequiredToolIDs;
		Requirements.bRequiresEngineRemoval = bRequiresEngineRemoval;
		Requirements.bRequiresTransmissionRemoval = bRequiresTransmissionRemoval;
		Requirements.bRequiresDynoTuning = bRequiresDynoTuning;
		Requirements.LaborCostMultiplier = LaborCostMultiplier;
		return Requirements;
	}

	/**
	 * @brief Get estimated shop labor cost for installation
	 *
	 * Calculates the labor cost based on install time and multiplier.
	 * Does not include part purchase price or dyno tuning fees.
	 *
	 * @param HourlyRate Shop's hourly labor rate (default $75/hour)
	 * @return Labor cost in credits
	 */
	UFUNCTION(BlueprintPure, Category = "Installation")
	int64 GetEstimatedLaborCost(int64 HourlyRate = 75) const
	{
		const float Hours = InstallTimeMinutes / 60.0f;
		return static_cast<int64>(Hours * HourlyRate * LaborCostMultiplier);
	}

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

	/**
	 * @brief Gets all parts matching the specified quality tier.
	 *
	 * @param Quality The quality tier to filter by.
	 * @return Array of parts with the specified quality.
	 */
	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGPartDefinition*> GetPartsByQuality(EMGPartQuality Quality) const;

	/**
	 * @brief Gets all parts matching the specified brand reputation.
	 *
	 * @param Reputation The brand reputation level to filter by.
	 * @return Array of parts from brands with the specified reputation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Catalog")
	TArray<UMGPartDefinition*> GetPartsByReputation(EMGBrandReputation Reputation) const;

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
 * @struct FMGInstalledPart
 * @brief Represents an installed part instance on a player's vehicle.
 *
 * Contains both the part identification and instance-specific data
 * such as wear level, quality tier, and tuning configuration.
 */
USTRUCT(BlueprintType)
struct FMGInstalledPart
{
	GENERATED_BODY()

	// ==========================================
	// IDENTIFICATION
	// ==========================================

	/** Unique identifier referencing the part definition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FName PartID;

	/** Unique instance ID for this specific part. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FGuid InstanceID;

	// ==========================================
	// QUALITY
	// ==========================================

	/**
	 * @brief Quality tier of this specific part instance.
	 *
	 * Cached from the part definition at install time.
	 * Affects all quality-based calculations.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	EMGPartQuality Quality = EMGPartQuality::Aftermarket;

	/**
	 * @brief Brand reputation at time of purchase.
	 *
	 * Cached for resale value and failure calculations.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	EMGBrandReputation BrandReputation = EMGBrandReputation::Standard;

	// ==========================================
	// CONDITION
	// ==========================================

	/**
	 * @brief Current wear level (0 = new, 1 = worn out).
	 *
	 * Increases over time based on usage and quality.
	 * Affects performance and failure chance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WearLevel = 0.0f;

	/**
	 * @brief Remaining durability points.
	 *
	 * Decreases with use. When depleted, part needs replacement.
	 * Initial value determined by quality tier.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float CurrentDurability = 100.0f;

	/**
	 * @brief Maximum durability for this part instance.
	 *
	 * Set at installation based on quality tier.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float MaxDurability = 100.0f;

	/**
	 * @brief Whether this part has failed and needs repair.
	 *
	 * Failed parts impose performance penalties until repaired.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bIsFailed = false;

	/**
	 * @brief Severity of current failure (if failed).
	 *
	 * 0 = minor, 1 = catastrophic. Affects repair cost.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FailureSeverity = 0.0f;

	/**
	 * @brief Accumulated stress from high-performance use.
	 *
	 * Increases during redline, nitrous use, etc.
	 * Resets partially over time. Affects failure chance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AccumulatedStress = 0.0f;

	// ==========================================
	// HISTORY
	// ==========================================

	/** When this part was installed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	FDateTime InstallDate;

	/** Original purchase price paid by player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	int64 PurchasePrice = 0;

	/** Total distance driven with this part (km). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	float TotalDistanceKM = 0.0f;

	/** Number of races completed with this part. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	int32 RacesCompleted = 0;

	/** Number of times this part has failed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	int32 FailureCount = 0;

	/** Total repair costs spent on this part. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	int64 TotalRepairCosts = 0;

	// ==========================================
	// TUNING
	// ==========================================

	/**
	 * @brief Current tuning values (key = option ID, value = setting).
	 *
	 * Stores player-configured tuning for adjustable parts.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	TMap<FName, float> TuningValues;

	// ==========================================
	// VISUAL
	// ==========================================

	/** Selected color variant index for visual parts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 ColorIndex = 0;

	/** Custom color if applicable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor CustomColor = FLinearColor::White;

	// ==========================================
	// CONSTRUCTORS AND METHODS
	// ==========================================

	FMGInstalledPart()
		: Quality(EMGPartQuality::Aftermarket)
		, BrandReputation(EMGBrandReputation::Standard)
		, WearLevel(0.0f)
		, CurrentDurability(100.0f)
		, MaxDurability(100.0f)
		, bIsFailed(false)
		, FailureSeverity(0.0f)
		, AccumulatedStress(0.0f)
		, PurchasePrice(0)
		, TotalDistanceKM(0.0f)
		, RacesCompleted(0)
		, FailureCount(0)
		, TotalRepairCosts(0)
		, ColorIndex(0)
		, CustomColor(FLinearColor::White)
	{
		InstanceID = FGuid::NewGuid();
		InstallDate = FDateTime::Now();
	}

	/**
	 * @brief Initialize part with quality settings.
	 *
	 * Sets up durability based on quality tier.
	 *
	 * @param InQuality The quality tier of the part.
	 * @param InReputation The brand reputation level.
	 */
	void InitializeWithQuality(EMGPartQuality InQuality, EMGBrandReputation InReputation)
	{
		Quality = InQuality;
		BrandReputation = InReputation;

		// Set durability based on quality
		const FMGQualityEffects Effects = UMGPartQualityStatics::GetQualityEffects(Quality);
		MaxDurability = Effects.BaseDurability;
		CurrentDurability = MaxDurability;
	}

	/**
	 * @brief Calculate current resale value of this part.
	 *
	 * @return Current resale value based on wear and quality.
	 */
	int64 GetCurrentResaleValue() const
	{
		return UMGPartQualityStatics::CalculateResaleValue(
			PurchasePrice, Quality, BrandReputation, WearLevel);
	}

	/**
	 * @brief Check if part needs replacement due to wear.
	 *
	 * @return True if wear level exceeds 90% or durability depleted.
	 */
	bool NeedsReplacement() const
	{
		return WearLevel >= 0.9f || CurrentDurability <= 0.0f;
	}

	/**
	 * @brief Get the current performance multiplier accounting for wear and failure.
	 *
	 * @return Effective performance multiplier (0-1+).
	 */
	float GetEffectivePerformanceMultiplier() const
	{
		const FMGQualityEffects Effects = UMGPartQualityStatics::GetQualityEffects(Quality);
		float Multiplier = Effects.PerformanceMultiplier;

		// Reduce for wear (up to 15% loss at max wear)
		Multiplier *= (1.0f - (WearLevel * 0.15f));

		// Apply failure penalty if failed
		if (bIsFailed)
		{
			Multiplier *= (1.0f - (0.1f + FailureSeverity * 0.6f));
		}

		return Multiplier;
	}
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

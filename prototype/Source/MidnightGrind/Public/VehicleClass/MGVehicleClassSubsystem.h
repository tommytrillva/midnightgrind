// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGVehicleClassSubsystem.generated.h"

/**
 * Vehicle class tier
 */
UENUM(BlueprintType)
enum class EMGVehicleClassTier : uint8
{
	D			UMETA(DisplayName = "D Class"),
	C			UMETA(DisplayName = "C Class"),
	B			UMETA(DisplayName = "B Class"),
	A			UMETA(DisplayName = "A Class"),
	S			UMETA(DisplayName = "S Class"),
	SPlus		UMETA(DisplayName = "S+ Class"),
	Hyper		UMETA(DisplayName = "Hyper Class"),
	Legend		UMETA(DisplayName = "Legend Class"),
	Custom		UMETA(DisplayName = "Custom Class")
};

/**
 * Vehicle body type
 */
UENUM(BlueprintType)
enum class EMGVehicleBodyType : uint8
{
	Compact		UMETA(DisplayName = "Compact"),
	Coupe		UMETA(DisplayName = "Coupe"),
	Sedan		UMETA(DisplayName = "Sedan"),
	Hatchback	UMETA(DisplayName = "Hatchback"),
	Sports		UMETA(DisplayName = "Sports Car"),
	Muscle		UMETA(DisplayName = "Muscle Car"),
	Supercar	UMETA(DisplayName = "Supercar"),
	Hypercar	UMETA(DisplayName = "Hypercar"),
	SUV			UMETA(DisplayName = "SUV"),
	Truck		UMETA(DisplayName = "Truck"),
	Wagon		UMETA(DisplayName = "Wagon"),
	Roadster	UMETA(DisplayName = "Roadster"),
	Kei			UMETA(DisplayName = "Kei Car"),
	Van			UMETA(DisplayName = "Van"),
	Classic		UMETA(DisplayName = "Classic"),
	Exotic		UMETA(DisplayName = "Exotic")
};

/**
 * Vehicle drivetrain type
 */
UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
	FWD			UMETA(DisplayName = "Front-Wheel Drive"),
	RWD			UMETA(DisplayName = "Rear-Wheel Drive"),
	AWD			UMETA(DisplayName = "All-Wheel Drive"),
	MR			UMETA(DisplayName = "Mid-Engine RWD"),
	RR			UMETA(DisplayName = "Rear-Engine RWD"),
	F4WD		UMETA(DisplayName = "Full-Time 4WD")
};

/**
 * Vehicle era/generation
 */
UENUM(BlueprintType)
enum class EMGVehicleEra : uint8
{
	Classic		UMETA(DisplayName = "Classic (Pre-1980)"),
	Retro		UMETA(DisplayName = "Retro (1980-1999)"),
	Modern		UMETA(DisplayName = "Modern (2000-2015)"),
	Current		UMETA(DisplayName = "Current (2015+)"),
	Future		UMETA(DisplayName = "Future Concept")
};

/**
 * Performance stat category
 */
UENUM(BlueprintType)
enum class EMGPerformanceStat : uint8
{
	Speed		UMETA(DisplayName = "Top Speed"),
	Acceleration UMETA(DisplayName = "Acceleration"),
	Handling	UMETA(DisplayName = "Handling"),
	Braking		UMETA(DisplayName = "Braking"),
	Nitro		UMETA(DisplayName = "Nitro"),
	Durability	UMETA(DisplayName = "Durability")
};

/**
 * PI calculation weight preset
 */
UENUM(BlueprintType)
enum class EMGPIWeightPreset : uint8
{
	Balanced	UMETA(DisplayName = "Balanced"),
	SpeedFocus	UMETA(DisplayName = "Speed Focus"),
	AccelFocus	UMETA(DisplayName = "Acceleration Focus"),
	HandlingFocus UMETA(DisplayName = "Handling Focus"),
	DriftTuned	UMETA(DisplayName = "Drift Tuned"),
	DragTuned	UMETA(DisplayName = "Drag Tuned"),
	Custom		UMETA(DisplayName = "Custom Weights")
};

/**
 * Individual performance stat value
 */
USTRUCT(BlueprintType)
struct FMGPerformanceStatValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPerformanceStat Stat = EMGPerformanceStat::Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ModifiedValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UpgradeBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TuneBonus = 0.0f;
};

/**
 * Vehicle performance profile
 */
USTRUCT(BlueprintType)
struct FMGVehiclePerformanceProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Handling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Braking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Nitro;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceStatValue Durability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedKMH = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZeroToSixtyTime = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QuarterMileTime = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerHP = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorqueNM = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeightKG = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerToWeightRatio = 0.0f;
};

/**
 * Class tier definition with PI range
 */
USTRUCT(BlueprintType)
struct FMGClassTierDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleClassTier Tier = EMGVehicleClassTier::D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPI = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPI = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ClassColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ClassIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedEventTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseRewardMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DifficultyMultiplier = 1.0f;
};

/**
 * Vehicle classification data
 */
USTRUCT(BlueprintType)
struct FMGVehicleClassification
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VehicleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Manufacturer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Year = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleClassTier ClassTier = EMGVehicleClassTier::D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleBodyType BodyType = EMGVehicleBodyType::Coupe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDrivetrainType Drivetrain = EMGDrivetrainType::RWD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleEra Era = EMGVehicleEra::Modern;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePI = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPI = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPI = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGVehiclePerformanceProfile PerformanceProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SpecialTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CountryOfOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRare = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLegendary = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlockable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePurchasePrice = 0;
};

/**
 * PI calculation weights
 */
USTRUCT(BlueprintType)
struct FMGPIWeights
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HandlingWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingWeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroWeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurabilityWeight = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerToWeightFactor = 1.5f;
};

/**
 * Class restriction for events
 */
USTRUCT(BlueprintType)
struct FMGClassRestriction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RestrictionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RestrictionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGVehicleClassTier> AllowedTiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPI = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPI = 999;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGVehicleBodyType> AllowedBodyTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGDrivetrainType> AllowedDrivetrains;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGVehicleEra> AllowedEras;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedCountries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedManufacturers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ExcludedVehicleIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireStock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowUpgrades = true;
};

/**
 * Upgrade impact on PI
 */
USTRUCT(BlueprintType)
struct FMGUpgradePIImpact
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UpgradeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PIChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPerformanceStat, float> StatChanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMayChangeClass = false;
};

/**
 * Vehicle comparison result
 */
USTRUCT(BlueprintType)
struct FMGVehicleComparison
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PIDifference = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPerformanceStat, float> StatDifferences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RecommendedChoice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComparisonSummary;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVehicleClassChanged, const FString&, VehicleId, EMGVehicleClassTier, OldClass, EMGVehicleClassTier, NewClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVehiclePIChanged, const FString&, VehicleId, int32, OldPI, int32, NewPI);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleRegistered, const FString&, VehicleId, EMGVehicleClassTier, ClassTier);

/**
 * Vehicle Class Subsystem
 * Manages vehicle classification, performance index, and class restrictions
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleClassSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "VehicleClass|Events")
	FOnVehicleClassChanged OnVehicleClassChanged;

	UPROPERTY(BlueprintAssignable, Category = "VehicleClass|Events")
	FOnVehiclePIChanged OnVehiclePIChanged;

	UPROPERTY(BlueprintAssignable, Category = "VehicleClass|Events")
	FOnVehicleRegistered OnVehicleRegistered;

	// Vehicle Registration
	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	bool RegisterVehicle(const FMGVehicleClassification& Classification);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	bool UnregisterVehicle(const FString& VehicleId);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	FMGVehicleClassification GetVehicleClassification(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	bool IsVehicleRegistered(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass")
	TArray<FMGVehicleClassification> GetAllVehicles() const;

	// Class Tier Management
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Tier")
	bool RegisterClassTier(const FMGClassTierDefinition& TierDef);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Tier")
	FMGClassTierDefinition GetClassTierDefinition(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Tier")
	EMGVehicleClassTier GetVehicleClassTier(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Tier")
	EMGVehicleClassTier CalculateClassTierFromPI(int32 PI) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Tier")
	FLinearColor GetClassColor(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Tier")
	FText GetClassDisplayName(EMGVehicleClassTier Tier) const;

	// Performance Index
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	int32 GetVehiclePI(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	int32 CalculatePI(const FMGVehiclePerformanceProfile& Profile, const FMGPIWeights& Weights) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	int32 CalculatePIWithPreset(const FMGVehiclePerformanceProfile& Profile, EMGPIWeightPreset Preset) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	bool UpdateVehiclePI(const FString& VehicleId, int32 NewPI);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	FMGPIWeights GetPIWeightsForPreset(EMGPIWeightPreset Preset) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	FMGUpgradePIImpact CalculateUpgradeImpact(const FString& VehicleId, const FString& UpgradeId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|PI")
	int32 GetPIHeadroomForClass(const FString& VehicleId) const;

	// Performance Stats
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Stats")
	FMGVehiclePerformanceProfile GetPerformanceProfile(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Stats")
	bool UpdatePerformanceProfile(const FString& VehicleId, const FMGVehiclePerformanceProfile& Profile);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Stats")
	float GetPerformanceStat(const FString& VehicleId, EMGPerformanceStat Stat) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Stats")
	FMGVehicleComparison CompareVehicles(const FString& VehicleIdA, const FString& VehicleIdB) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	float CalculatePowerToWeightRatio(float PowerHP, float WeightKG) const;

	// Class Restrictions
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	bool RegisterRestriction(const FMGClassRestriction& Restriction);

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	FMGClassRestriction GetRestriction(const FString& RestrictionId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	bool DoesVehicleMeetRestriction(const FString& VehicleId, const FString& RestrictionId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	TArray<FString> GetViolatedRestrictions(const FString& VehicleId, const FString& RestrictionId) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Restrictions")
	TArray<FMGVehicleClassification> GetEligibleVehicles(const FString& RestrictionId) const;

	// Queries
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByClass(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByBodyType(EMGVehicleBodyType BodyType) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByDrivetrain(EMGDrivetrainType Drivetrain) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByEra(EMGVehicleEra Era) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByPIRange(int32 MinPI, int32 MaxPI) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByManufacturer(const FString& Manufacturer) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FMGVehicleClassification> GetVehiclesByCountry(const FString& Country) const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FString> GetAllManufacturers() const;

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Query")
	TArray<FString> GetAllCountries() const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	int32 GetTotalVehicleCount() const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	int32 GetVehicleCountInClass(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	float GetAveragePIInClass(EMGVehicleClassTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	FMGVehicleClassification GetHighestPIVehicle() const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Stats")
	FMGVehicleClassification GetLowestPIVehicle() const;

	// Utility
	UFUNCTION(BlueprintPure, Category = "VehicleClass|Utility")
	FText GetBodyTypeDisplayName(EMGVehicleBodyType BodyType) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Utility")
	FText GetDrivetrainDisplayName(EMGDrivetrainType Drivetrain) const;

	UFUNCTION(BlueprintPure, Category = "VehicleClass|Utility")
	FText GetEraDisplayName(EMGVehicleEra Era) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Persistence")
	void SaveVehicleClassData();

	UFUNCTION(BlueprintCallable, Category = "VehicleClass|Persistence")
	void LoadVehicleClassData();

protected:
	void InitializeDefaultClassTiers();
	void RecalculateVehicleClass(const FString& VehicleId);

private:
	UPROPERTY()
	TMap<FString, FMGVehicleClassification> RegisteredVehicles;

	UPROPERTY()
	TMap<EMGVehicleClassTier, FMGClassTierDefinition> ClassTierDefinitions;

	UPROPERTY()
	TMap<FString, FMGClassRestriction> RegisteredRestrictions;

	UPROPERTY()
	TMap<EMGPIWeightPreset, FMGPIWeights> PIWeightPresets;
};

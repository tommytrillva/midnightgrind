// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTuningSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGTuningCategory : uint8
{
	Engine,
	Transmission,
	Suspension,
	Brakes,
	Tires,
	Nitro,
	Weight,
	Aerodynamics
};

UENUM(BlueprintType)
enum class EMGTuningLevel : uint8
{
	Stock,
	Street,
	Sport,
	Race,
	Pro,
	Elite,
	Ultimate
};

UENUM(BlueprintType)
enum class EMGDrivetrainType : uint8
{
	FWD,
	RWD,
	AWD
};

USTRUCT(BlueprintType)
struct FMGTuningPart
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PartID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PartName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTuningCategory Category = EMGTuningCategory::Engine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTuningLevel Level = EMGTuningLevel::Stock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PurchasePrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstallPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerformanceIndexChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HandlingBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeightChange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> CompatibleVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredPreviousPart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredPlayerLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOwned = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInstalled = false;
};

USTRUCT(BlueprintType)
struct FMGTuningSlider
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SliderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTuningCategory Category = EMGTuningCategory::Suspension;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultValue = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StepSize = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MinLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MaxLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsHandling = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsSpeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsAcceleration = false;
};

USTRUCT(BlueprintType)
struct FMGVehicleTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGTuningCategory, FName> InstalledParts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> SliderValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDrivetrainType DrivetrainSwap = EMGDrivetrainType::RWD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasDrivetrainSwap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalGearRatio = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> GearRatios;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontDownforce = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearDownforce = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RideHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CamberFront = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CamberRear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ToeFront = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ToeRear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AntiRollFront = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AntiRollRear = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpringStiffnessFront = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpringStiffnessRear = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamperReboundFront = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamperReboundRear = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakeBias = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakePressure = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DifferentialFront = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DifferentialRear = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CenterDifferential = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TirePressureFront = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TirePressureRear = 32.0f;
};

USTRUCT(BlueprintType)
struct FMGVehicleStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Acceleration = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Handling = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Braking = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Nitro = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerformanceIndex = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Power = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Torque = 400.0f;
};

USTRUCT(BlueprintType)
struct FMGTuningPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGVehicleTuning TuningData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDefault = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsShared = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CreatorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Downloads = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rating = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartInstalled, FName, VehicleID, const FMGTuningPart&, Part);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartRemoved, FName, VehicleID, EMGTuningCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTuningChanged, FName, VehicleID, const FMGVehicleTuning&, Tuning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatsChanged, FName, VehicleID, const FMGVehicleStats&, NewStats);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPresetSaved, FName, VehicleID, FName, PresetID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPresetLoaded, FName, VehicleID, FName, PresetID);

UCLASS()
class MIDNIGHTGRIND_API UMGTuningSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Part Management
	UFUNCTION(BlueprintCallable, Category = "Tuning|Parts")
	bool InstallPart(FName VehicleID, FName PartID);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Parts")
	bool RemovePart(FName VehicleID, EMGTuningCategory Category);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Parts")
	bool PurchasePart(FName PartID);

	UFUNCTION(BlueprintPure, Category = "Tuning|Parts")
	bool OwnsPart(FName PartID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Parts")
	bool CanInstallPart(FName VehicleID, FName PartID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Parts")
	FMGTuningPart GetInstalledPart(FName VehicleID, EMGTuningCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Parts")
	TArray<FMGTuningPart> GetAvailableParts(FName VehicleID, EMGTuningCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Parts")
	TArray<FMGTuningPart> GetOwnedParts() const;

	// Slider Tuning
	UFUNCTION(BlueprintCallable, Category = "Tuning|Sliders")
	void SetSliderValue(FName VehicleID, FName SliderID, float Value);

	UFUNCTION(BlueprintPure, Category = "Tuning|Sliders")
	float GetSliderValue(FName VehicleID, FName SliderID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Sliders")
	TArray<FMGTuningSlider> GetAvailableSliders(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Tuning|Sliders")
	void ResetSliderToDefault(FName VehicleID, FName SliderID);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Sliders")
	void ResetAllSlidersToDefault(FName VehicleID);

	// Advanced Tuning
	UFUNCTION(BlueprintCallable, Category = "Tuning|Advanced")
	void SetGearRatio(FName VehicleID, int32 GearIndex, float Ratio);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Advanced")
	void SetFinalDrive(FName VehicleID, float Ratio);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Advanced")
	void SetDrivetrainSwap(FName VehicleID, EMGDrivetrainType NewDrivetrain);

	UFUNCTION(BlueprintPure, Category = "Tuning|Advanced")
	FMGVehicleTuning GetVehicleTuning(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Tuning|Advanced")
	void SetVehicleTuning(FName VehicleID, const FMGVehicleTuning& Tuning);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Tuning|Stats")
	FMGVehicleStats GetBaseStats(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Stats")
	FMGVehicleStats GetTunedStats(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Stats")
	int32 GetPerformanceIndex(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Stats")
	FMGVehicleStats PreviewPartInstall(FName VehicleID, FName PartID) const;

	// Presets
	UFUNCTION(BlueprintCallable, Category = "Tuning|Presets")
	FName SavePreset(FName VehicleID, const FText& PresetName);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Presets")
	bool LoadPreset(FName VehicleID, FName PresetID);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Presets")
	bool DeletePreset(FName PresetID);

	UFUNCTION(BlueprintPure, Category = "Tuning|Presets")
	TArray<FMGTuningPreset> GetSavedPresets(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Presets")
	TArray<FMGTuningPreset> GetCommunityPresets(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Tuning|Presets")
	void SharePreset(FName PresetID);

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Tuning|Registration")
	void RegisterVehicle(FName VehicleID, const FMGVehicleStats& BaseStats);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Registration")
	void RegisterPart(const FMGTuningPart& Part);

	UFUNCTION(BlueprintCallable, Category = "Tuning|Registration")
	void RegisterSlider(const FMGTuningSlider& Slider);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Tuning|Events")
	FOnPartInstalled OnPartInstalled;

	UPROPERTY(BlueprintAssignable, Category = "Tuning|Events")
	FOnPartRemoved OnPartRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Tuning|Events")
	FOnTuningChanged OnTuningChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tuning|Events")
	FOnStatsChanged OnStatsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tuning|Events")
	FOnPresetSaved OnPresetSaved;

	UPROPERTY(BlueprintAssignable, Category = "Tuning|Events")
	FOnPresetLoaded OnPresetLoaded;

protected:
	FMGVehicleStats CalculateTunedStats(FName VehicleID) const;
	void RecalculateStats(FName VehicleID);
	void SaveTuningData();
	void LoadTuningData();

	UPROPERTY()
	TMap<FName, FMGVehicleTuning> VehicleTunings;

	UPROPERTY()
	TMap<FName, FMGVehicleStats> BaseVehicleStats;

	UPROPERTY()
	TMap<FName, FMGVehicleStats> TunedVehicleStats;

	UPROPERTY()
	TMap<FName, FMGTuningPart> PartDatabase;

	UPROPERTY()
	TMap<FName, FMGTuningSlider> SliderDatabase;

	UPROPERTY()
	TSet<FName> OwnedParts;

	UPROPERTY()
	TMap<FName, FMGTuningPreset> SavedPresets;

	UPROPERTY()
	TArray<FMGTuningPreset> CommunityPresets;
};

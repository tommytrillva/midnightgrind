// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGTuningSubsystem.h
 * @brief Vehicle fine-tuning subsystem for adjusting performance parameters
 *
 * The Tuning Subsystem handles detailed vehicle customization beyond basic parts.
 * While the Garage Subsystem manages what parts are installed, the Tuning Subsystem
 * manages HOW those parts are configured - suspension geometry, gear ratios,
 * differential settings, and more.
 *
 * ## Key Responsibilities
 * - **Part Upgrades**: Managing tiered performance parts (Street, Sport, Race, Pro, etc.)
 * - **Slider Tuning**: Fine-tuning parameters via slider controls (ride height, camber, etc.)
 * - **Advanced Tuning**: Gear ratios, differential lock, drivetrain swaps
 * - **Preset Management**: Saving, loading, and sharing tuning configurations
 * - **Stats Calculation**: Computing final vehicle stats from base + parts + tuning
 *
 * ## Tuning Philosophy
 * The system is designed to be approachable for beginners while offering depth
 * for experienced players:
 * - **Casual Players**: Install pre-configured parts, use community presets
 * - **Intermediate**: Adjust key sliders (ride height, downforce, brake bias)
 * - **Advanced**: Fine-tune gear ratios, suspension geometry, differential behavior
 *
 * ## Integration
 * Works closely with UMGGarageSubsystem - parts must be installed in the garage
 * before they can be tuned here. The dyno subsystem can verify tuning changes.
 *
 * @see UMGGarageSubsystem for part installation
 * @see UMGDynoSubsystem for power verification
 * @see FMGVehicleTuning for the tuning data structure
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPartInstallation.h"
#include "Core/MGSharedTypes.h"
#include "Vehicle/MG_VHCL_Data.h"
#include "MGTuningSubsystem.generated.h"

// ============================================================================
// TUNING ENUMERATIONS
// ============================================================================

// MOVED TO MGSharedTypes.h
// /**
//  * @enum EMGTuningCategory
//  * @brief Categories of tunable vehicle systems
//  *
//  * Each category groups related parts and sliders. Mechanics may specialize
//  * in specific categories, affecting installation quality and cost.
//  */
// UENUM(BlueprintType)
// enum class EMGTuningCategory : uint8
// {
// 	/// Engine power modifications (intake, exhaust, ECU, forced induction)
// 	Engine,
// 	/// Gearbox and drivetrain (ratios, clutch, differential)
// 	Transmission,
// 	/// Handling setup (springs, dampers, geometry)
// 	Suspension,
// 	/// Stopping power (rotors, calipers, lines)
// 	Brakes,
// 	/// Tire compound and pressure settings
// 	Tires,
// 	/// Nitrous oxide system configuration
// 	Nitro,
// 	/// Weight reduction and ballast
// 	Weight,
// 	/// Downforce and drag (wings, splitters, diffusers)
// 	Aerodynamics
// };

/**
 * @enum EMGTuningLevel
 * @brief Upgrade tiers for performance parts
 *
 * Higher tiers provide better performance but cost more and may have
 * stricter installation requirements.
 *
 * ## Tier Progression
 * | Tier     | Target PI | Unlock Requirement |
 * |----------|-----------|-------------------|
 * | Stock    | 100-300   | Default           |
 * | Street   | 200-400   | Player Level 5    |
 * | Sport    | 300-500   | Player Level 15   |
 * | Race     | 400-600   | Player Level 30   |
 * | Pro      | 500-700   | Player Level 50   |
 * | Elite    | 600-800   | Player Level 75   |
 * | Ultimate | 700-999   | Player Level 100  |
 */
UENUM(BlueprintType)
enum class EMGTuningLevel : uint8
{
	/// Factory original parts - baseline performance
	Stock,
	/// Entry-level aftermarket - mild improvements
	Street,
	/// Enthusiast grade - noticeable gains
	Sport,
	/// Track-focused - significant performance
	Race,
	/// Professional grade - near-maximum potential
	Pro,
	/// Top-tier - exceptional performance
	Elite,
	/// Maximum performance - no compromises
	Ultimate
};

// EMGDrivetrainType - REMOVED (duplicate)
// Canonical definition in: Core/MGCoreEnums.h

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

	// ==========================================
	// INSTALLATION PROPERTIES
	// ==========================================

	/**
	 * @brief Installation difficulty level
	 *
	 * Determines DIY success rates, required skill level, and base install time.
	 * Simple = bolt-on parts, Expert = engine builds, ShopOnly = requires professional
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	EMGInstallDifficulty InstallDifficulty = EMGInstallDifficulty::Moderate;

	/**
	 * @brief Base installation time in minutes
	 *
	 * Default times by difficulty:
	 * - Simple: 15 min
	 * - Moderate: 60 min
	 * - Complex: 240 min (4 hours)
	 * - Expert: 480 min (8 hours)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation", meta = (ClampMin = "5", ClampMax = "2880"))
	int32 InstallTimeMinutes = 60;

	/**
	 * @brief Whether installation requires vehicle on a lift
	 *
	 * Parts underneath the car (exhaust, suspension, drivetrain)
	 * require lift access for DIY installation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	bool bRequiresLift = false;

	/**
	 * @brief Whether installation requires special tools
	 *
	 * Torque wrenches, spring compressors, bearing pullers, etc.
	 * Affects DIY success rate if player lacks required tools
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	bool bRequiresSpecialTools = false;

	/**
	 * @brief Specific tool IDs required for installation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	TArray<FName> RequiredToolIDs;

	/**
	 * @brief Whether dyno tuning is required after installation
	 *
	 * Performance parts affecting fuel/air need professional tuning
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Installation")
	bool bRequiresDynoTuning = false;

	/**
	 * @brief Convert to FMGInstallationRequirements struct
	 */
	FMGInstallationRequirements GetInstallationRequirements() const
	{
		FMGInstallationRequirements Reqs;
		Reqs.Difficulty = InstallDifficulty;
		Reqs.InstallTimeMinutes = InstallTimeMinutes;
		Reqs.bRequiresLift = bRequiresLift;
		Reqs.bRequiresSpecialTools = bRequiresSpecialTools;
		Reqs.RequiredToolIDs = RequiredToolIDs;
		Reqs.bRequiresDynoTuning = bRequiresDynoTuning;
		return Reqs;
	}
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

// FMGVehicleSpecs is defined in Vehicle/MG_VHCL_Data.h (included above)
// This holds calculated performance metrics (horsepower, handling, etc.)
// Distinct from FMGVehicleRacingStats which tracks per-vehicle racing history

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatsChanged, FName, VehicleID, const FMGVehicleSpecs&, NewStats);
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
	FMGVehicleSpecs GetBaseStats(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Stats")
	FMGVehicleSpecs GetTunedStats(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Stats")
	int32 GetPerformanceIndex(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tuning|Stats")
	FMGVehicleSpecs PreviewPartInstall(FName VehicleID, FName PartID) const;

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
	void RegisterVehicle(FName VehicleID, const FMGVehicleSpecs& BaseStats);

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
	FMGVehicleSpecs CalculateTunedStats(FName VehicleID) const;
	void RecalculateStats(FName VehicleID);
	void SaveTuningData();
	void LoadTuningData();

	UPROPERTY()
	TMap<FName, FMGVehicleTuning> VehicleTunings;

	UPROPERTY()
	TMap<FName, FMGVehicleSpecs> BaseVehicleStats;

	UPROPERTY()
	TMap<FName, FMGVehicleSpecs> TunedVehicleStats;

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

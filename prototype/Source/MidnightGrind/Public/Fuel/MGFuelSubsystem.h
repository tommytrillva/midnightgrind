// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGFuelSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGFuelType : uint8
{
	Regular,
	Premium,
	Racing,
	Diesel,
	Electric,
	Hybrid,
	Nitromethane,
	E85
};

UENUM(BlueprintType)
enum class EMGFuelState : uint8
{
	Full,
	Adequate,
	Low,
	Critical,
	Empty,
	Reserved
};

UENUM(BlueprintType)
enum class EMGFuelMode : uint8
{
	Standard,
	Economy,
	Performance,
	Qualifying,
	Attack,
	Defend,
	Limp
};

UENUM(BlueprintType)
enum class EMGFuelAlert : uint8
{
	None,
	LowFuel,
	CriticalFuel,
	FuelSaveRecommended,
	PitWindowOpen,
	WontFinish
};

USTRUCT(BlueprintType)
struct FMGFuelTypeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelType FuelType = EMGFuelType::Regular;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FuelID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnergyDensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EfficiencyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CostPerLiter = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresSpecialTank = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OctaneRating = 95.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnvironmentalImpact = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FuelColor = FLinearColor::Green;
};

USTRUCT(BlueprintType)
struct FMGVehicleFuelState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelType CurrentFuelType = EMGFuelType::Regular;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFuel = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TankCapacity = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelPercentage = 0.833f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelState State = EMGFuelState::Adequate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelMode FuelMode = EMGFuelMode::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConsumptionRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InstantConsumption = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageConsumption = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalFuelUsed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceTraveled = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedLapsRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelWeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelTemperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFuelSaveActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelSavePercentage = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGFuelConsumptionFactors
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseConsumption = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RPMMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GearMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdleConsumption = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DraftingBonus = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InclineMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeatherMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TireWearMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGFuelModeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelMode Mode = EMGFuelMode::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ModeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConsumptionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRPMMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleResponseMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowNitro = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelSaveTarget = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGFuelTelemetry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> ConsumptionHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> LapConsumption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakConsumption = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowestConsumption = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestEfficiencyLap = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WorstEfficiencyLap = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalFuelUsedSession = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistanceSession = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SessionEfficiency = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGFuelStrategy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StrategyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartingFuel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetFuelAtFinish = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> PlannedPitLaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> PlannedFuelLoads;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGFuelMode> PlannedFuelModes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedConsumptionPerLap = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUndercut = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFuelSaveRequired = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FuelSaveFromLap = 0;
};

USTRUCT(BlueprintType)
struct FMGFuelSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateFuel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalConsumptionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFuelAffectsWeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelWeightPerLiter = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowFuelAlerts = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowFuelThreshold = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalFuelThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoFuelMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowConsumptionHUD = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRangeEstimate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RefuelRate = 10.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFuelStateChanged, FName, VehicleID, EMGFuelState, OldState, EMGFuelState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelAlert, FName, VehicleID, EMGFuelAlert, Alert);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelModeChanged, FName, VehicleID, EMGFuelMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelConsumed, FName, VehicleID, float, AmountConsumed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelAdded, FName, VehicleID, float, AmountAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelEmpty, FName, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLapFuelUsage, FName, VehicleID, int32, LapNumber, float, FuelUsed);

UCLASS()
class MIDNIGHTGRIND_API UMGFuelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Vehicle Registration
	UFUNCTION(BlueprintCallable, Category = "Fuel|Vehicle")
	void RegisterVehicle(FName VehicleID, float TankCapacity, EMGFuelType FuelType = EMGFuelType::Regular);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Vehicle")
	void UnregisterVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Vehicle")
	void SetVehicleFuel(FName VehicleID, float FuelAmount);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Vehicle")
	void SetTankCapacity(FName VehicleID, float Capacity);

	// Fuel State
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	FMGVehicleFuelState GetFuelState(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetCurrentFuel(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetFuelPercentage(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	EMGFuelState GetFuelStatus(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetEstimatedRange(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetEstimatedLapsRemaining(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	bool CanFinishRace(FName VehicleID, int32 RemainingLaps) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetFuelWeight(FName VehicleID) const;

	// Consumption
	UFUNCTION(BlueprintCallable, Category = "Fuel|Consumption")
	void UpdateFuelConsumption(FName VehicleID, float Throttle, float Speed, float RPM, int32 Gear, bool bNitroActive, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Consumption")
	void ConsumeFuel(FName VehicleID, float Amount);

	UFUNCTION(BlueprintPure, Category = "Fuel|Consumption")
	float GetInstantConsumption(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Consumption")
	float GetAverageConsumption(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Consumption")
	float GetConsumptionPerLap(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Fuel|Consumption")
	void SetConsumptionFactors(FName VehicleID, const FMGFuelConsumptionFactors& Factors);

	UFUNCTION(BlueprintPure, Category = "Fuel|Consumption")
	FMGFuelConsumptionFactors GetConsumptionFactors(FName VehicleID) const;

	// Fuel Modes
	UFUNCTION(BlueprintCallable, Category = "Fuel|Mode")
	void SetFuelMode(FName VehicleID, EMGFuelMode Mode);

	UFUNCTION(BlueprintPure, Category = "Fuel|Mode")
	EMGFuelMode GetFuelMode(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Fuel|Mode")
	void CycleFuelMode(FName VehicleID, bool bForward = true);

	UFUNCTION(BlueprintPure, Category = "Fuel|Mode")
	FMGFuelModeSettings GetFuelModeSettings(EMGFuelMode Mode) const;

	UFUNCTION(BlueprintCallable, Category = "Fuel|Mode")
	void SetFuelModeSettings(EMGFuelMode Mode, const FMGFuelModeSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "Fuel|Mode")
	TArray<EMGFuelMode> GetAvailableFuelModes() const;

	// Fuel Save
	UFUNCTION(BlueprintCallable, Category = "Fuel|Save")
	void ActivateFuelSave(FName VehicleID, float TargetPercentage);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Save")
	void DeactivateFuelSave(FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "Fuel|Save")
	bool IsFuelSaveActive(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Save")
	float GetFuelSaveAmount(FName VehicleID) const;

	// Refueling
	UFUNCTION(BlueprintCallable, Category = "Fuel|Refuel")
	void AddFuel(FName VehicleID, float Amount);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Refuel")
	void FillTank(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Refuel")
	void StartRefueling(FName VehicleID, float TargetAmount);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Refuel")
	void StopRefueling(FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "Fuel|Refuel")
	bool IsRefueling(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Refuel")
	float GetRefuelingProgress(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Refuel")
	float CalculateRefuelTime(float Amount) const;

	// Fuel Types
	UFUNCTION(BlueprintCallable, Category = "Fuel|Type")
	void RegisterFuelType(const FMGFuelTypeData& FuelData);

	UFUNCTION(BlueprintPure, Category = "Fuel|Type")
	FMGFuelTypeData GetFuelTypeData(EMGFuelType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Fuel|Type")
	void SetVehicleFuelType(FName VehicleID, EMGFuelType Type);

	UFUNCTION(BlueprintPure, Category = "Fuel|Type")
	EMGFuelType GetVehicleFuelType(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Type")
	TArray<FMGFuelTypeData> GetAllFuelTypes() const;

	// Strategy
	UFUNCTION(BlueprintCallable, Category = "Fuel|Strategy")
	void SetFuelStrategy(FName VehicleID, const FMGFuelStrategy& Strategy);

	UFUNCTION(BlueprintPure, Category = "Fuel|Strategy")
	FMGFuelStrategy GetFuelStrategy(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Strategy")
	FMGFuelStrategy CalculateOptimalStrategy(FName VehicleID, int32 TotalLaps, float LapLength) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Strategy")
	float CalculateRequiredFuel(FName VehicleID, int32 Laps) const;

	UFUNCTION(BlueprintPure, Category = "Fuel|Strategy")
	int32 GetRecommendedPitLap(FName VehicleID, int32 RemainingLaps) const;

	// Telemetry
	UFUNCTION(BlueprintPure, Category = "Fuel|Telemetry")
	FMGFuelTelemetry GetFuelTelemetry(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Fuel|Telemetry")
	void RecordLapFuelUsage(FName VehicleID, int32 LapNumber);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Telemetry")
	void ResetTelemetry(FName VehicleID);

	// Lap Events
	UFUNCTION(BlueprintCallable, Category = "Fuel|Lap")
	void OnLapStarted(FName VehicleID, int32 LapNumber);

	UFUNCTION(BlueprintCallable, Category = "Fuel|Lap")
	void OnLapCompleted(FName VehicleID, int32 LapNumber);

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Fuel|Settings")
	void SetFuelSettings(const FMGFuelSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Fuel|Settings")
	FMGFuelSettings GetFuelSettings() const { return Settings; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelStateChanged OnFuelStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelAlert OnFuelAlert;

	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelModeChanged OnFuelModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelConsumed OnFuelConsumed;

	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelAdded OnFuelAdded;

	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelEmpty OnFuelEmpty;

	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnLapFuelUsage OnLapFuelUsage;

protected:
	void OnFuelTick();
	void UpdateRefueling(float DeltaTime);
	void UpdateFuelStates();
	void CheckFuelAlerts(FName VehicleID);
	EMGFuelState CalculateFuelState(float Percentage) const;
	void InitializeDefaultFuelTypes();
	void InitializeDefaultFuelModes();
	void SaveFuelData();
	void LoadFuelData();

	UPROPERTY()
	TMap<FName, FMGVehicleFuelState> VehicleFuelStates;

	UPROPERTY()
	TMap<FName, FMGFuelConsumptionFactors> VehicleConsumptionFactors;

	UPROPERTY()
	TMap<FName, FMGFuelTelemetry> VehicleTelemetry;

	UPROPERTY()
	TMap<FName, FMGFuelStrategy> VehicleStrategies;

	UPROPERTY()
	TMap<FName, float> VehicleLapStartFuel;

	UPROPERTY()
	TMap<EMGFuelType, FMGFuelTypeData> FuelTypes;

	UPROPERTY()
	TMap<EMGFuelMode, FMGFuelModeSettings> FuelModes;

	UPROPERTY()
	TMap<FName, bool> RefuelingVehicles;

	UPROPERTY()
	TMap<FName, float> RefuelingTargets;

	UPROPERTY()
	TMap<FName, float> RefuelingProgress;

	UPROPERTY()
	FMGFuelSettings Settings;

	FTimerHandle FuelTickHandle;
};

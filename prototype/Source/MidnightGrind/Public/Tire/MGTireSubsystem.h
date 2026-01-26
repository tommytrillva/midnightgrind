// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTireSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGTireCompoundType : uint8
{
	UltraSoft,
	Soft,
	Medium,
	Hard,
	Intermediate,
	FullWet,
	AllSeason,
	DriftCompound,
	OffRoad,
	Rally,
	Slick,
	Vintage
};

UENUM(BlueprintType)
enum class EMGTirePosition : uint8
{
	FrontLeft,
	FrontRight,
	RearLeft,
	RearRight
};

UENUM(BlueprintType)
enum class EMGTireCondition : uint8
{
	Optimal,
	Good,
	Worn,
	Critical,
	Punctured,
	Blown
};

UENUM(BlueprintType)
enum class EMGTireWearPattern : uint8
{
	Even,
	InsideEdge,
	OutsideEdge,
	Center,
	Flat,
	Cupping
};

UENUM(BlueprintType)
enum class EMGTrackSurface : uint8
{
	Asphalt,
	Concrete,
	Gravel,
	Dirt,
	Grass,
	Sand,
	Snow,
	Ice,
	Wet,
	Puddle,
	Oil
};

USTRUCT(BlueprintType)
struct FMGTireCompoundData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCompoundType CompoundType = EMGTireCompoundType::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompoundID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CompoundColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseGrip = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakGripTemperature = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalTempMin = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalTempMax = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WearRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatUpRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolDownRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WetPerformance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurabilityFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralGripMod = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongitudinalGripMod = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExpectedLaps = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllWeather = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStudded = false;
};

USTRUCT(BlueprintType)
struct FMGTireState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTirePosition Position = EMGTirePosition::FrontLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCompoundType Compound = EMGTireCompoundType::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCondition Condition = EMGTireCondition::Optimal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireWearPattern WearPattern = EMGTireWearPattern::Even;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WearLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceTemperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoreTemperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pressure = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentGrip = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralGrip = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongitudinalGrip = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipRatio = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoadForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpinning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapsOnTire = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceOnTire = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGVehicleTireSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireState FrontLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireState FrontRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireState RearLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireState RearRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageWear = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageTemperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageGrip = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMixedCompounds = false;
};

USTRUCT(BlueprintType)
struct FMGTireWearFactors
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationWear = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingWear = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorneringWear = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipWear = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockupWear = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TemperatureWear = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceWear = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoadWear = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGTireTemperatureZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InnerTemp = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MiddleTemp = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OuterTemp = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageTemp = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TempSpread = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverheating = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUndercooled = false;
};

USTRUCT(BlueprintType)
struct FMGTireTelemetry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTireState> TireHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakFrontLeftTemp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakFrontRightTemp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakRearLeftTemp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakRearRightTemp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Lockups = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wheelspin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalSlipDistance = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGTireSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalWearMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalGripMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TemperatureSimSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AmbientTemperature = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackTemperature = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulatePressure = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateTemperature = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateWear = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPunctures = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PunctureChance = 0.001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisualizeTemperature = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisualizeWear = true;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTireWearUpdated, FName, VehicleID, EMGTirePosition, Position, float, NewWear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTireConditionChanged, FName, VehicleID, EMGTirePosition, Position, EMGTireCondition, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTireTemperatureWarning, FName, VehicleID, EMGTirePosition, Position, float, Temperature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTirePunctured, FName, VehicleID, EMGTirePosition, Position);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTireLockup, FName, VehicleID, EMGTirePosition, Position);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTireWheelspin, FName, VehicleID, EMGTirePosition, Position);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTiresChanged, FName, VehicleID, EMGTireCompoundType, NewCompound);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGripChanged, FName, VehicleID, EMGTirePosition, Position, float, NewGrip);

UCLASS()
class MIDNIGHTGRIND_API UMGTireSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Tire State
	UFUNCTION(BlueprintCallable, Category = "Tire|State")
	void RegisterVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Tire|State")
	void UnregisterVehicle(FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	FMGTireState GetTireState(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	FMGVehicleTireSet GetVehicleTires(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetTireWear(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetTireTemperature(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetTireGrip(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetTirePressure(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	EMGTireCondition GetTireCondition(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetAverageWear(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tire|State")
	float GetAverageGrip(FName VehicleID) const;

	// Tire Update
	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void UpdateTireState(FName VehicleID, EMGTirePosition Position, float SlipRatio, float SlipAngle, float Load, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ApplyWear(FName VehicleID, EMGTirePosition Position, float WearAmount);

	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ApplyHeat(FName VehicleID, EMGTirePosition Position, float HeatAmount);

	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void CoolTire(FName VehicleID, EMGTirePosition Position, float CoolAmount);

	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void SetTirePressure(FName VehicleID, EMGTirePosition Position, float Pressure);

	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ReportLockup(FName VehicleID, EMGTirePosition Position);

	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ReportWheelspin(FName VehicleID, EMGTirePosition Position);

	UFUNCTION(BlueprintCallable, Category = "Tire|Update")
	void ReportSurfaceContact(FName VehicleID, EMGTirePosition Position, EMGTrackSurface Surface);

	// Tire Change
	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void ChangeTires(FName VehicleID, EMGTireCompoundType NewCompound);

	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void ChangeSingleTire(FName VehicleID, EMGTirePosition Position, EMGTireCompoundType NewCompound);

	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void ChangeFrontTires(FName VehicleID, EMGTireCompoundType NewCompound);

	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void ChangeRearTires(FName VehicleID, EMGTireCompoundType NewCompound);

	UFUNCTION(BlueprintCallable, Category = "Tire|Change")
	void PunctureRepair(FName VehicleID, EMGTirePosition Position);

	// Compound Info
	UFUNCTION(BlueprintCallable, Category = "Tire|Compound")
	void RegisterCompound(const FMGTireCompoundData& CompoundData);

	UFUNCTION(BlueprintPure, Category = "Tire|Compound")
	FMGTireCompoundData GetCompoundData(EMGTireCompoundType CompoundType) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Compound")
	TArray<FMGTireCompoundData> GetAllCompounds() const;

	UFUNCTION(BlueprintPure, Category = "Tire|Compound")
	EMGTireCompoundType GetRecommendedCompound(float TrackTemp, bool bWet) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Compound")
	int32 GetExpectedTireLaps(EMGTireCompoundType CompoundType) const;

	// Grip Calculation
	UFUNCTION(BlueprintPure, Category = "Tire|Grip")
	float CalculateGrip(const FMGTireState& TireState, const FMGTireCompoundData& Compound) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Grip")
	float GetGripFromTemperature(float Temperature, const FMGTireCompoundData& Compound) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Grip")
	float GetGripFromWear(float WearLevel) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Grip")
	float GetSurfaceGripMultiplier(EMGTrackSurface Surface, EMGTireCompoundType Compound) const;

	// Temperature Zones
	UFUNCTION(BlueprintPure, Category = "Tire|Temperature")
	FMGTireTemperatureZone GetTireTemperatureZones(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Temperature")
	bool IsTireOverheating(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Temperature")
	bool IsTireCold(FName VehicleID, EMGTirePosition Position) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Temperature")
	bool IsTireInOptimalWindow(FName VehicleID, EMGTirePosition Position) const;

	// Telemetry
	UFUNCTION(BlueprintPure, Category = "Tire|Telemetry")
	FMGTireTelemetry GetTireTelemetry(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Tire|Telemetry")
	void RecordTelemetry(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Tire|Telemetry")
	void ClearTelemetry(FName VehicleID);

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Tire|Settings")
	void SetTireSettings(const FMGTireSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Tire|Settings")
	FMGTireSettings GetTireSettings() const { return Settings; }

	UFUNCTION(BlueprintCallable, Category = "Tire|Settings")
	void SetWearFactors(const FMGTireWearFactors& Factors);

	UFUNCTION(BlueprintPure, Category = "Tire|Settings")
	FMGTireWearFactors GetWearFactors() const { return WearFactors; }

	UFUNCTION(BlueprintCallable, Category = "Tire|Settings")
	void SetTrackTemperature(float Temperature);

	UFUNCTION(BlueprintCallable, Category = "Tire|Settings")
	void SetAmbientTemperature(float Temperature);

	// Prediction
	UFUNCTION(BlueprintPure, Category = "Tire|Prediction")
	int32 PredictTireLapsRemaining(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Prediction")
	float PredictWearAfterLaps(FName VehicleID, int32 Laps) const;

	UFUNCTION(BlueprintPure, Category = "Tire|Prediction")
	bool ShouldChangeTires(FName VehicleID, int32 RemainingLaps) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireWearUpdated OnTireWearUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireConditionChanged OnTireConditionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireTemperatureWarning OnTireTemperatureWarning;

	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTirePunctured OnTirePunctured;

	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireLockup OnTireLockup;

	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTireWheelspin OnTireWheelspin;

	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnTiresChanged OnTiresChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tire|Events")
	FOnGripChanged OnGripChanged;

protected:
	void OnTireTick();
	void UpdateAllTires(float DeltaTime);
	void UpdateTireTemperature(FMGTireState& Tire, const FMGTireCompoundData& Compound, float DeltaTime);
	void UpdateTireWear(FMGTireState& Tire, const FMGTireCompoundData& Compound, float DeltaTime);
	void UpdateTireGrip(FMGTireState& Tire, const FMGTireCompoundData& Compound);
	void CheckForPuncture(FName VehicleID, FMGTireState& Tire);
	EMGTireCondition CalculateCondition(float WearLevel, bool bFlat) const;
	void InitializeDefaultCompounds();
	void SaveTireData();
	void LoadTireData();

	UPROPERTY()
	TMap<FName, FMGVehicleTireSet> VehicleTires;

	UPROPERTY()
	TMap<EMGTireCompoundType, FMGTireCompoundData> CompoundDatabase;

	UPROPERTY()
	TMap<FName, FMGTireTelemetry> VehicleTelemetry;

	UPROPERTY()
	FMGTireSettings Settings;

	UPROPERTY()
	FMGTireWearFactors WearFactors;

	FTimerHandle TireTickHandle;
};

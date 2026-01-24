// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDamageSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGDamageType : uint8
{
	None,
	Collision,
	SideSwipe,
	TBone,
	RearEnd,
	FrontalImpact,
	Rollover,
	WallScrape,
	SpikeTrap,
	EMP
};

UENUM(BlueprintType)
enum class EMGDamageZone : uint8
{
	None,
	FrontLeft,
	FrontCenter,
	FrontRight,
	SideLeft,
	SideRight,
	RearLeft,
	RearCenter,
	RearRight,
	Roof,
	Underbody
};

UENUM(BlueprintType)
enum class EMGDamageSeverity : uint8
{
	None,
	Cosmetic,
	Light,
	Moderate,
	Heavy,
	Critical,
	Totaled
};

UENUM(BlueprintType)
enum class EMGVehicleComponent : uint8
{
	None,
	Engine,
	Transmission,
	Suspension,
	Steering,
	Brakes,
	Tires,
	Exhaust,
	NitroSystem,
	Radiator,
	FuelTank,
	Electronics
};

USTRUCT(BlueprintType)
struct FMGDamageInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageType DamageType = EMGDamageType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageZone Zone = EMGDamageZone::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageSeverity Severity = EMGDamageSeverity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RawDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactNormal = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactVelocity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName InstigatorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasBlocked = false;
};

USTRUCT(BlueprintType)
struct FMGZoneDamageState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageZone Zone = EMGDamageZone::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDamage = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageSeverity Severity = EMGDamageSeverity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeformationLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFunctional = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> DetachedParts;
};

USTRUCT(BlueprintType)
struct FMGComponentDamageState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVehicleComponent Component = EMGVehicleComponent::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EfficiencyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFunctional = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDisabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RepairCost = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGVehicleDamageState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverallHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageSeverity OverallSeverity = EMGDamageSeverity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDamageZone, FMGZoneDamageState> ZoneDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGVehicleComponent, FMGComponentDamageState> ComponentDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDriveable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTotaled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRepairCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmokeLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeakingFuel = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeakingOil = false;
};

USTRUCT(BlueprintType)
struct FMGDamageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisualDamageEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMechanicalDamageEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinImpactVelocityForDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotaledHealthThreshold = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalHealthThreshold = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPartDetachment = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PartDetachmentThreshold = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowFire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireIgnitionThreshold = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComponentDamageSpreadFactor = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoRepairOnRespawn = true;
};

USTRUCT(BlueprintType)
struct FMGRepairOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairVisual = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairMechanical = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairAllZones = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGDamageZone> SpecificZones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairAllComponents = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGVehicleComponent> SpecificComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RepairPercentage = 100.0f;
};

USTRUCT(BlueprintType)
struct FMGDeformationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> DeformedVertices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, FVector> VertexOffsets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDeformationDepth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DeformationCenter = FVector::ZeroVector;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageReceived, FName, VehicleID, const FMGDamageInstance&, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneDamaged, EMGDamageZone, Zone, EMGDamageSeverity, Severity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComponentDamaged, EMGVehicleComponent, Component, float, HealthRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentDisabled, EMGVehicleComponent, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartDetached, FName, VehicleID, FName, PartName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleTotaled, FName, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleOnFire, FName, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleRepaired, FName, VehicleID, float, HealthRestored);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, FName, VehicleID, float, OldHealth, float, NewHealth);

UCLASS()
class MIDNIGHTGRIND_API UMGDamageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Damage Application
	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	FMGDamageInstance ApplyDamage(FName VehicleID, const FMGDamageInstance& Damage);

	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	FMGDamageInstance ApplyCollisionDamage(FName VehicleID, const FVector& ImpactPoint, const FVector& ImpactNormal, float ImpactVelocity, FName InstigatorID = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	void ApplyZoneDamage(FName VehicleID, EMGDamageZone Zone, float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	void ApplyComponentDamage(FName VehicleID, EMGVehicleComponent Component, float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Damage|Apply")
	void ApplyEnvironmentalDamage(FName VehicleID, EMGDamageType DamageType, float DamageAmount);

	// State Queries
	UFUNCTION(BlueprintPure, Category = "Damage|State")
	FMGVehicleDamageState GetVehicleDamageState(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|State")
	float GetVehicleHealth(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|State")
	float GetVehicleHealthPercent(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|State")
	EMGDamageSeverity GetVehicleSeverity(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|State")
	bool IsVehicleDriveable(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|State")
	bool IsVehicleTotaled(FName VehicleID) const;

	// Zone Queries
	UFUNCTION(BlueprintPure, Category = "Damage|Zone")
	FMGZoneDamageState GetZoneDamageState(FName VehicleID, EMGDamageZone Zone) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Zone")
	float GetZoneDamagePercent(FName VehicleID, EMGDamageZone Zone) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Zone")
	EMGDamageZone GetMostDamagedZone(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Zone")
	TArray<FName> GetDetachedParts(FName VehicleID) const;

	// Component Queries
	UFUNCTION(BlueprintPure, Category = "Damage|Component")
	FMGComponentDamageState GetComponentDamageState(FName VehicleID, EMGVehicleComponent Component) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Component")
	float GetComponentEfficiency(FName VehicleID, EMGVehicleComponent Component) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Component")
	bool IsComponentFunctional(FName VehicleID, EMGVehicleComponent Component) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Component")
	TArray<EMGVehicleComponent> GetDisabledComponents(FName VehicleID) const;

	// Performance Impact
	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetEnginePerformanceMultiplier(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetHandlingMultiplier(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetTopSpeedMultiplier(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetAccelerationMultiplier(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetBrakingMultiplier(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Performance")
	float GetNitroEfficiency(FName VehicleID) const;

	// Repair
	UFUNCTION(BlueprintCallable, Category = "Damage|Repair")
	int32 RepairVehicle(FName VehicleID, const FMGRepairOptions& Options);

	UFUNCTION(BlueprintCallable, Category = "Damage|Repair")
	int32 RepairVehicleFull(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Damage|Repair")
	int32 RepairZone(FName VehicleID, EMGDamageZone Zone);

	UFUNCTION(BlueprintCallable, Category = "Damage|Repair")
	int32 RepairComponent(FName VehicleID, EMGVehicleComponent Component);

	UFUNCTION(BlueprintPure, Category = "Damage|Repair")
	int32 GetRepairCost(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Repair")
	int32 GetZoneRepairCost(FName VehicleID, EMGDamageZone Zone) const;

	UFUNCTION(BlueprintPure, Category = "Damage|Repair")
	int32 GetComponentRepairCost(FName VehicleID, EMGVehicleComponent Component) const;

	// Vehicle Registration
	UFUNCTION(BlueprintCallable, Category = "Damage|Registration")
	void RegisterVehicle(FName VehicleID, float MaxHealth = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "Damage|Registration")
	void UnregisterVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Damage|Registration")
	void ResetVehicleDamage(FName VehicleID);

	// Deformation
	UFUNCTION(BlueprintPure, Category = "Damage|Deformation")
	FMGDeformationData GetDeformationData(FName VehicleID, EMGDamageZone Zone) const;

	UFUNCTION(BlueprintCallable, Category = "Damage|Deformation")
	void AddDeformation(FName VehicleID, EMGDamageZone Zone, const FVector& ImpactPoint, float Depth);

	// Fire
	UFUNCTION(BlueprintCallable, Category = "Damage|Fire")
	void IgniteVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Damage|Fire")
	void ExtinguishVehicle(FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "Damage|Fire")
	bool IsVehicleOnFire(FName VehicleID) const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Damage|Config")
	void SetDamageConfig(const FMGDamageConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "Damage|Config")
	FMGDamageConfig GetDamageConfig() const { return Config; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnDamageReceived OnDamageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnZoneDamaged OnZoneDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnComponentDamaged OnComponentDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnComponentDisabled OnComponentDisabled;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnPartDetached OnPartDetached;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnVehicleTotaled OnVehicleTotaled;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnVehicleOnFire OnVehicleOnFire;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnVehicleRepaired OnVehicleRepaired;

	UPROPERTY(BlueprintAssignable, Category = "Damage|Events")
	FOnHealthChanged OnHealthChanged;

protected:
	EMGDamageZone CalculateImpactZone(const FVector& ImpactPoint, const FVector& VehicleForward, const FVector& VehicleRight) const;
	EMGDamageType DetermineCollisionType(const FVector& ImpactNormal, const FVector& VehicleForward) const;
	EMGDamageSeverity CalculateSeverity(float DamagePercent) const;
	float CalculateFinalDamage(const FMGDamageInstance& Damage) const;
	void SpreadComponentDamage(FName VehicleID, EMGDamageZone Zone, float DamageAmount);
	void CheckPartDetachment(FName VehicleID, EMGDamageZone Zone);
	void CheckFireIgnition(FName VehicleID);
	void UpdateOverallState(FName VehicleID);
	int32 CalculateRepairCost(float DamageAmount, EMGDamageSeverity Severity) const;
	void InitializeVehicleState(FMGVehicleDamageState& State, float MaxHealth);

	UPROPERTY()
	TMap<FName, FMGVehicleDamageState> VehicleDamageStates;

	UPROPERTY()
	TMap<FName, TMap<EMGDamageZone, FMGDeformationData>> VehicleDeformation;

	UPROPERTY()
	FMGDamageConfig Config;
};

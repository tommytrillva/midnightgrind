// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGNitroBoostSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGNitroType : uint8
{
	Standard,
	Supercharged,
	Cryogenic,
	Electric,
	Plasma,
	Experimental
};

UENUM(BlueprintType)
enum class EMGNitroChargeMethod : uint8
{
	Time,
	Drifting,
	NearMiss,
	Drafting,
	CleanSection,
	Combo,
	Pickup
};

UENUM(BlueprintType)
enum class EMGBoostState : uint8
{
	Idle,
	Charging,
	Ready,
	Active,
	Cooldown,
	Overheated
};

USTRUCT(BlueprintType)
struct FMGNitroConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNitroType NitroType = EMGNitroType::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxCapacity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConsumptionRate = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RechargeRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinActivationAmount = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowPartialBoost = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanOverheat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverheatThreshold = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverheatCooldownTime = 3.0f;
};

USTRUCT(BlueprintType)
struct FMGNitroState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentAmount = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAmount = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBoostState State = EMGBoostState::Ready;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ActiveTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BoostLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPerfectBoost = false;
};

USTRUCT(BlueprintType)
struct FMGNitroChargeSource
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNitroChargeMethod Method = EMGNitroChargeMethod::Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct FMGNitroUpgrade
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UpgradeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CapacityBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RechargeBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EfficiencyBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cost = 0;
};

USTRUCT(BlueprintType)
struct FMGBoostZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostMultiplier = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInstantRefill = false;
};

USTRUCT(BlueprintType)
struct FMGNitroPickup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PickupID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeAmount = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAvailable = true;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNitroActivated, int32, BoostLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNitroDeactivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNitroAmountChanged, float, NewAmount, float, MaxAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNitroDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNitroOverheat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerfectBoostAchieved, float, BonusPower);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNitroCharged, EMGNitroChargeMethod, Method, float, Amount);

UCLASS()
class MIDNIGHTGRIND_API UMGNitroBoostSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Core Boost Functions
	UFUNCTION(BlueprintCallable, Category = "Nitro|Control")
	bool ActivateBoost();

	UFUNCTION(BlueprintCallable, Category = "Nitro|Control")
	void DeactivateBoost();

	UFUNCTION(BlueprintPure, Category = "Nitro|Control")
	bool CanActivateBoost() const;

	UFUNCTION(BlueprintPure, Category = "Nitro|Control")
	bool IsBoostActive() const { return NitroState.State == EMGBoostState::Active; }

	UFUNCTION(BlueprintPure, Category = "Nitro|Control")
	float GetCurrentBoostMultiplier() const;

	// Nitro State
	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	FMGNitroState GetNitroState() const { return NitroState; }

	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	float GetNitroAmount() const { return NitroState.CurrentAmount; }

	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	float GetNitroPercent() const;

	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	EMGBoostState GetBoostState() const { return NitroState.State; }

	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	float GetHeatLevel() const { return NitroState.HeatLevel; }

	UFUNCTION(BlueprintPure, Category = "Nitro|State")
	bool IsOverheated() const { return NitroState.State == EMGBoostState::Overheated; }

	// Charging
	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void AddNitroCharge(float Amount, EMGNitroChargeMethod Method = EMGNitroChargeMethod::Time);

	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void SetNitroAmount(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void RefillNitro();

	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void DrainNitro(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Nitro|Charge")
	void RegisterChargeSource(const FMGNitroChargeSource& Source);

	UFUNCTION(BlueprintPure, Category = "Nitro|Charge")
	TArray<FMGNitroChargeSource> GetChargeSources() const { return ChargeSources; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Nitro|Config")
	void SetNitroConfig(const FMGNitroConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Nitro|Config")
	FMGNitroConfig GetNitroConfig() const { return NitroConfig; }

	UFUNCTION(BlueprintCallable, Category = "Nitro|Config")
	void SetNitroType(EMGNitroType Type);

	UFUNCTION(BlueprintPure, Category = "Nitro|Config")
	EMGNitroType GetNitroType() const { return NitroConfig.NitroType; }

	// Multi-Level Boost
	UFUNCTION(BlueprintCallable, Category = "Nitro|Level")
	void SetBoostLevel(int32 Level);

	UFUNCTION(BlueprintPure, Category = "Nitro|Level")
	int32 GetBoostLevel() const { return NitroState.BoostLevel; }

	UFUNCTION(BlueprintPure, Category = "Nitro|Level")
	int32 GetMaxBoostLevel() const { return MaxBoostLevel; }

	UFUNCTION(BlueprintCallable, Category = "Nitro|Level")
	bool CanUpgradeBoostLevel() const;

	// Perfect Boost System
	UFUNCTION(BlueprintCallable, Category = "Nitro|Perfect")
	void TriggerPerfectBoostWindow();

	UFUNCTION(BlueprintPure, Category = "Nitro|Perfect")
	bool IsInPerfectBoostWindow() const { return bPerfectBoostWindowActive; }

	UFUNCTION(BlueprintPure, Category = "Nitro|Perfect")
	float GetPerfectBoostWindowRemaining() const;

	// Upgrades
	UFUNCTION(BlueprintCallable, Category = "Nitro|Upgrades")
	void ApplyUpgrade(const FMGNitroUpgrade& Upgrade);

	UFUNCTION(BlueprintPure, Category = "Nitro|Upgrades")
	TArray<FMGNitroUpgrade> GetAvailableUpgrades() const { return AvailableUpgrades; }

	UFUNCTION(BlueprintPure, Category = "Nitro|Upgrades")
	TArray<FMGNitroUpgrade> GetInstalledUpgrades() const { return InstalledUpgrades; }

	// Boost Zones
	UFUNCTION(BlueprintCallable, Category = "Nitro|Zones")
	void RegisterBoostZone(const FMGBoostZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "Nitro|Zones")
	void UnregisterBoostZone(FName ZoneID);

	UFUNCTION(BlueprintPure, Category = "Nitro|Zones")
	bool IsInBoostZone() const { return ActiveBoostZone.ZoneID != NAME_None; }

	UFUNCTION(BlueprintPure, Category = "Nitro|Zones")
	FMGBoostZone GetActiveBoostZone() const { return ActiveBoostZone; }

	// Pickups
	UFUNCTION(BlueprintCallable, Category = "Nitro|Pickups")
	void RegisterPickup(const FMGNitroPickup& Pickup);

	UFUNCTION(BlueprintCallable, Category = "Nitro|Pickups")
	void CollectPickup(FName PickupID);

	UFUNCTION(BlueprintPure, Category = "Nitro|Pickups")
	TArray<FMGNitroPickup> GetActivePickups() const;

	// Location Update
	UFUNCTION(BlueprintCallable, Category = "Nitro|Update")
	void UpdateVehicleLocation(FVector Location);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroActivated OnNitroActivated;

	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroDeactivated OnNitroDeactivated;

	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroAmountChanged OnNitroAmountChanged;

	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroDepleted OnNitroDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroOverheat OnNitroOverheat;

	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnPerfectBoostAchieved OnPerfectBoostAchieved;

	UPROPERTY(BlueprintAssignable, Category = "Nitro|Events")
	FOnNitroCharged OnNitroCharged;

protected:
	void OnNitroTick();
	void UpdateBoostState();
	void UpdateCharging();
	void UpdateHeat();
	void CheckBoostZones();
	void UpdatePickups();
	void InitializeDefaultConfig();
	float CalculateEffectiveMultiplier() const;

	UPROPERTY()
	FMGNitroConfig NitroConfig;

	UPROPERTY()
	FMGNitroState NitroState;

	UPROPERTY()
	TArray<FMGNitroChargeSource> ChargeSources;

	UPROPERTY()
	TArray<FMGNitroUpgrade> AvailableUpgrades;

	UPROPERTY()
	TArray<FMGNitroUpgrade> InstalledUpgrades;

	UPROPERTY()
	TMap<FName, FMGBoostZone> BoostZones;

	UPROPERTY()
	FMGBoostZone ActiveBoostZone;

	UPROPERTY()
	TMap<FName, FMGNitroPickup> Pickups;

	UPROPERTY()
	FVector CurrentVehicleLocation = FVector::ZeroVector;

	UPROPERTY()
	int32 MaxBoostLevel = 3;

	UPROPERTY()
	bool bPerfectBoostWindowActive = false;

	UPROPERTY()
	float PerfectBoostWindowDuration = 0.3f;

	UPROPERTY()
	float PerfectBoostWindowTimer = 0.0f;

	UPROPERTY()
	float PerfectBoostBonusPower = 0.25f;

	FTimerHandle NitroTickHandle;
};

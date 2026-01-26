// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMemoryBudgetSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGMemoryPool : uint8
{
	Textures,
	Meshes,
	Audio,
	Animation,
	Physics,
	Particles,
	UI,
	Streaming,
	General
};

UENUM(BlueprintType)
enum class EMGMemoryPressure : uint8
{
	None,
	Low,
	Medium,
	High,
	Critical
};

USTRUCT(BlueprintType)
struct FMGMemoryPoolStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EMGMemoryPool Pool = EMGMemoryPool::General;

	UPROPERTY(BlueprintReadOnly)
	int64 UsedMB = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 BudgetMB = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 PeakMB = 0;

	UPROPERTY(BlueprintReadOnly)
	float UsagePercent = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGMemoryBudgetConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalBudgetMB = 4096;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TextureBudgetMB = 1536;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MeshBudgetMB = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AudioBudgetMB = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 StreamingBudgetMB = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WarningThreshold = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalThreshold = 0.95f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMemoryPressureChanged, EMGMemoryPressure, Pressure);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPoolOverBudget, EMGMemoryPool, Pool, int64, OverageMB);

UCLASS()
class MIDNIGHTGRIND_API UMGMemoryBudgetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Budget Configuration
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void SetBudgetConfig(const FMGMemoryBudgetConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Memory")
	FMGMemoryBudgetConfig GetBudgetConfig() const { return BudgetConfig; }

	// Pool Stats
	UFUNCTION(BlueprintPure, Category = "Memory")
	FMGMemoryPoolStats GetPoolStats(EMGMemoryPool Pool) const;

	UFUNCTION(BlueprintPure, Category = "Memory")
	TArray<FMGMemoryPoolStats> GetAllPoolStats() const;

	UFUNCTION(BlueprintPure, Category = "Memory")
	int64 GetTotalUsedMB() const;

	UFUNCTION(BlueprintPure, Category = "Memory")
	int64 GetTotalBudgetMB() const { return BudgetConfig.TotalBudgetMB; }

	// Pressure Monitoring
	UFUNCTION(BlueprintPure, Category = "Memory")
	EMGMemoryPressure GetMemoryPressure() const { return CurrentPressure; }

	UFUNCTION(BlueprintPure, Category = "Memory")
	bool IsUnderPressure() const { return CurrentPressure >= EMGMemoryPressure::Medium; }

	// Memory Management
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void RequestMemoryCleanup(EMGMemoryPool Pool);

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void ForceGarbageCollection();

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void TrimStreamingPool(int64 TargetFreeMB);

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void PurgeUnusedAssets();

	// Quality Adjustment
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void SetAutoQualityAdjustment(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Memory")
	int32 GetRecommendedTextureQuality() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
	FMGOnMemoryPressureChanged OnMemoryPressureChanged;

	UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
	FMGOnPoolOverBudget OnPoolOverBudget;

protected:
	void UpdateMemoryStats();
	void CheckMemoryPressure();
	void ApplyPlatformBudgets();
	int64 GetPoolBudget(EMGMemoryPool Pool) const;

private:
	FMGMemoryBudgetConfig BudgetConfig;
	TMap<EMGMemoryPool, FMGMemoryPoolStats> PoolStats;
	EMGMemoryPressure CurrentPressure = EMGMemoryPressure::None;
	bool bAutoQualityAdjustment = true;
	FTimerHandle MonitorTimerHandle;
};

// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGMemoryBudgetSubsystem.cpp
 * @brief Implementation of the Memory Budget Management Subsystem
 */

#include "MemoryBudget/MGMemoryBudgetSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGMemoryBudgetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ApplyPlatformBudgets();

	// Initialize pool stats
	for (int32 i = 0; i <= (int32)EMGMemoryPool::General; i++)
	{
		FMGMemoryPoolStats Stats;
		Stats.Pool = (EMGMemoryPool)i;
		Stats.BudgetMB = GetPoolBudget((EMGMemoryPool)i);
		PoolStats.Add((EMGMemoryPool)i, Stats);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MonitorTimerHandle,
			this,
			&UMGMemoryBudgetSubsystem::UpdateMemoryStats,
			1.0f,
			true
		);
	}
}

void UMGMemoryBudgetSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MonitorTimerHandle);
	}
	Super::Deinitialize();
}

void UMGMemoryBudgetSubsystem::SetBudgetConfig(const FMGMemoryBudgetConfig& Config)
{
	BudgetConfig = Config;

	for (auto& Pair : PoolStats)
	{
		Pair.Value.BudgetMB = GetPoolBudget(Pair.Key);
	}
}

FMGMemoryPoolStats UMGMemoryBudgetSubsystem::GetPoolStats(EMGMemoryPool Pool) const
{
	const FMGMemoryPoolStats* Stats = PoolStats.Find(Pool);
	return Stats ? *Stats : FMGMemoryPoolStats();
}

TArray<FMGMemoryPoolStats> UMGMemoryBudgetSubsystem::GetAllPoolStats() const
{
	TArray<FMGMemoryPoolStats> AllStats;
	for (const auto& Pair : PoolStats)
	{
		AllStats.Add(Pair.Value);
	}
	return AllStats;
}

int64 UMGMemoryBudgetSubsystem::GetTotalUsedMB() const
{
	int64 Total = 0;
	for (const auto& Pair : PoolStats)
	{
		Total += Pair.Value.UsedMB;
	}
	return Total;
}

void UMGMemoryBudgetSubsystem::RequestMemoryCleanup(EMGMemoryPool Pool)
{
	// Would trigger cleanup for specific pool
	switch (Pool)
	{
	case EMGMemoryPool::Textures:
		// Reduce texture streaming pool
		break;
	case EMGMemoryPool::Audio:
		// Unload unused audio
		break;
	case EMGMemoryPool::Streaming:
		TrimStreamingPool(256);
		break;
	default:
		break;
	}
}

void UMGMemoryBudgetSubsystem::ForceGarbageCollection()
{
	GEngine->ForceGarbageCollection(true);
}

void UMGMemoryBudgetSubsystem::TrimStreamingPool(int64 TargetFreeMB)
{
	// Would work with streaming subsystem to unload lowest priority assets
}

void UMGMemoryBudgetSubsystem::PurgeUnusedAssets()
{
	// Collect garbage
	ForceGarbageCollection();

	// Flush async loading
	FlushAsyncLoading();
}

void UMGMemoryBudgetSubsystem::SetAutoQualityAdjustment(bool bEnabled)
{
	bAutoQualityAdjustment = bEnabled;
}

int32 UMGMemoryBudgetSubsystem::GetRecommendedTextureQuality() const
{
	FMGMemoryPoolStats TextureStats = GetPoolStats(EMGMemoryPool::Textures);

	if (TextureStats.UsagePercent > BudgetConfig.CriticalThreshold)
		return 0; // Low
	else if (TextureStats.UsagePercent > BudgetConfig.WarningThreshold)
		return 1; // Medium
	else if (TextureStats.UsagePercent > 0.5f)
		return 2; // High
	else
		return 3; // Ultra
}

void UMGMemoryBudgetSubsystem::UpdateMemoryStats()
{
	// Would query actual memory usage from platform APIs
	// For now, simulate

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

	int64 TotalUsed = MemStats.UsedPhysical / (1024 * 1024);

	// Distribute across pools (simulation)
	PoolStats[EMGMemoryPool::Textures].UsedMB = TotalUsed * 0.4f;
	PoolStats[EMGMemoryPool::Meshes].UsedMB = TotalUsed * 0.15f;
	PoolStats[EMGMemoryPool::Audio].UsedMB = TotalUsed * 0.08f;
	PoolStats[EMGMemoryPool::Streaming].UsedMB = TotalUsed * 0.2f;
	PoolStats[EMGMemoryPool::General].UsedMB = TotalUsed * 0.17f;

	for (auto& Pair : PoolStats)
	{
		FMGMemoryPoolStats& Stats = Pair.Value;
		Stats.PeakMB = FMath::Max(Stats.PeakMB, Stats.UsedMB);
		Stats.UsagePercent = (Stats.BudgetMB > 0) ? ((float)Stats.UsedMB / (float)Stats.BudgetMB) : 0.0f;

		if (Stats.UsagePercent > 1.0f)
		{
			OnPoolOverBudget.Broadcast(Pair.Key, Stats.UsedMB - Stats.BudgetMB);
		}
	}

	CheckMemoryPressure();
}

void UMGMemoryBudgetSubsystem::CheckMemoryPressure()
{
	float TotalUsage = (float)GetTotalUsedMB() / (float)BudgetConfig.TotalBudgetMB;

	EMGMemoryPressure NewPressure;

	if (TotalUsage >= BudgetConfig.CriticalThreshold)
		NewPressure = EMGMemoryPressure::Critical;
	else if (TotalUsage >= BudgetConfig.WarningThreshold)
		NewPressure = EMGMemoryPressure::High;
	else if (TotalUsage >= 0.6f)
		NewPressure = EMGMemoryPressure::Medium;
	else if (TotalUsage >= 0.4f)
		NewPressure = EMGMemoryPressure::Low;
	else
		NewPressure = EMGMemoryPressure::None;

	if (NewPressure != CurrentPressure)
	{
		CurrentPressure = NewPressure;
		OnMemoryPressureChanged.Broadcast(CurrentPressure);

		if (bAutoQualityAdjustment && CurrentPressure >= EMGMemoryPressure::High)
		{
			// Would auto-adjust quality settings
			PurgeUnusedAssets();
		}
	}
}

void UMGMemoryBudgetSubsystem::ApplyPlatformBudgets()
{
	// Set budgets based on platform
#if PLATFORM_PS5
	BudgetConfig.TotalBudgetMB = 12288; // 12GB for games
	BudgetConfig.TextureBudgetMB = 5120;
#elif PLATFORM_XSX
	BudgetConfig.TotalBudgetMB = 13312; // 13GB for games
	BudgetConfig.TextureBudgetMB = 5632;
#elif PLATFORM_SWITCH
	BudgetConfig.TotalBudgetMB = 3072; // 3GB
	BudgetConfig.TextureBudgetMB = 1024;
#else
	// PC - detect available memory
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	int64 AvailableGB = MemStats.TotalPhysical / (1024 * 1024 * 1024);

	if (AvailableGB >= 32)
		BudgetConfig.TotalBudgetMB = 8192;
	else if (AvailableGB >= 16)
		BudgetConfig.TotalBudgetMB = 6144;
	else
		BudgetConfig.TotalBudgetMB = 4096;

	BudgetConfig.TextureBudgetMB = BudgetConfig.TotalBudgetMB * 0.4f;
#endif
}

int64 UMGMemoryBudgetSubsystem::GetPoolBudget(EMGMemoryPool Pool) const
{
	switch (Pool)
	{
	case EMGMemoryPool::Textures: return BudgetConfig.TextureBudgetMB;
	case EMGMemoryPool::Meshes: return BudgetConfig.MeshBudgetMB;
	case EMGMemoryPool::Audio: return BudgetConfig.AudioBudgetMB;
	case EMGMemoryPool::Streaming: return BudgetConfig.StreamingBudgetMB;
	default: return BudgetConfig.TotalBudgetMB / 10;
	}
}

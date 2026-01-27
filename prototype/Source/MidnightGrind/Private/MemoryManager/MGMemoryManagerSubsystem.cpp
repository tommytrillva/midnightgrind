// MGMemoryManagerSubsystem.cpp
// Memory Management System - Implementation
// Midnight Grind - Y2K Arcade Street Racing

#include "MemoryManager/MGMemoryManagerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"

void UMGMemoryManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentPressureLevel = EMemoryPressureLevel::None;
    CurrentConcurrentLoads = 0;
    GameTimeSeconds = 0.0f;
    LastPlayerPosition = FVector::ZeroVector;
    LastPlayerVelocity = FVector::ZeroVector;

    // Initialize default settings
    StreamingSettings.bEnabled = true;
    StreamingSettings.UnloadStrategy = EUnloadStrategy::Hybrid;
    StreamingSettings.DefaultLoadStrategy = ELoadingStrategy::Async;
    StreamingSettings.MaxStreamingBandwidthMBps = 100.0f;
    StreamingSettings.MaxConcurrentLoads = 8;
    StreamingSettings.MaxQueueSize = 100;
    StreamingSettings.PredictiveLoadDistance = 10000.0f;
    StreamingSettings.UnloadDelaySeconds = 30.0f;
    StreamingSettings.AssetRetentionTimeSeconds = 60.0f;
    StreamingSettings.bPredictiveLoading = true;
    StreamingSettings.MemoryPressureThreshold = 0.85f;

    InitializeDefaultBudgets();

    // Start processing timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(LoadQueueTimer, this, &UMGMemoryManagerSubsystem::ProcessLoadQueue, 0.05f, true);
        World->GetTimerManager().SetTimer(MemoryCheckTimer, this, &UMGMemoryManagerSubsystem::UpdateMemoryPressure, 1.0f, true);
    }

    UE_LOG(LogTemp, Log, TEXT("MGMemoryManagerSubsystem initialized"));
}

void UMGMemoryManagerSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LoadQueueTimer);
        World->GetTimerManager().ClearTimer(MemoryCheckTimer);
        World->GetTimerManager().ClearTimer(GCTimer);
    }

    TrackedAssets.Empty();
    PendingRequests.Empty();
    UnloadQueue.Empty();

    Super::Deinitialize();
}

void UMGMemoryManagerSubsystem::InitializeDefaultBudgets()
{
    // Default budget allocations (assumes 16GB system)
    int64 GB = 1024LL * 1024LL * 1024LL;
    int64 MB = 1024LL * 1024LL;

    FMemoryBudget TextureBudget;
    TextureBudget.Pool = EMemoryPool::Textures;
    TextureBudget.BudgetBytes = 4 * GB;
    TextureBudget.WarningThreshold = 0.8f;
    TextureBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::Textures, TextureBudget);

    FMemoryBudget MeshBudget;
    MeshBudget.Pool = EMemoryPool::Meshes;
    MeshBudget.BudgetBytes = 2 * GB;
    MeshBudget.WarningThreshold = 0.8f;
    MeshBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::Meshes, MeshBudget);

    FMemoryBudget AudioBudget;
    AudioBudget.Pool = EMemoryPool::Audio;
    AudioBudget.BudgetBytes = 512 * MB;
    AudioBudget.WarningThreshold = 0.8f;
    AudioBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::Audio, AudioBudget);

    FMemoryBudget PhysicsBudget;
    PhysicsBudget.Pool = EMemoryPool::Physics;
    PhysicsBudget.BudgetBytes = 256 * MB;
    PhysicsBudget.WarningThreshold = 0.8f;
    PhysicsBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::Physics, PhysicsBudget);

    FMemoryBudget AnimBudget;
    AnimBudget.Pool = EMemoryPool::Animation;
    AnimBudget.BudgetBytes = 512 * MB;
    AnimBudget.WarningThreshold = 0.8f;
    AnimBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::Animation, AnimBudget);

    FMemoryBudget ParticleBudget;
    ParticleBudget.Pool = EMemoryPool::Particles;
    ParticleBudget.BudgetBytes = 256 * MB;
    ParticleBudget.WarningThreshold = 0.8f;
    ParticleBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::Particles, ParticleBudget);

    FMemoryBudget UIBudget;
    UIBudget.Pool = EMemoryPool::UI;
    UIBudget.BudgetBytes = 256 * MB;
    UIBudget.WarningThreshold = 0.8f;
    UIBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::UI, UIBudget);

    FMemoryBudget GeneralBudget;
    GeneralBudget.Pool = EMemoryPool::General;
    GeneralBudget.BudgetBytes = 1 * GB;
    GeneralBudget.WarningThreshold = 0.8f;
    GeneralBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::General, GeneralBudget);

    FMemoryBudget StreamingBudget;
    StreamingBudget.Pool = EMemoryPool::Streaming;
    StreamingBudget.BudgetBytes = 2 * GB;
    StreamingBudget.WarningThreshold = 0.8f;
    StreamingBudget.CriticalThreshold = 0.95f;
    PoolBudgets.Add(EMemoryPool::Streaming, StreamingBudget);
}

// ============================================================================
// Memory Budgets
// ============================================================================

void UMGMemoryManagerSubsystem::SetPoolBudget(EMemoryPool Pool, int64 BudgetBytes)
{
    if (PoolBudgets.Contains(Pool))
    {
        PoolBudgets[Pool].BudgetBytes = BudgetBytes;
    }
    else
    {
        FMemoryBudget Budget;
        Budget.Pool = Pool;
        Budget.BudgetBytes = BudgetBytes;
        PoolBudgets.Add(Pool, Budget);
    }
}

FMemoryBudget UMGMemoryManagerSubsystem::GetPoolBudget(EMemoryPool Pool) const
{
    if (PoolBudgets.Contains(Pool))
    {
        return PoolBudgets[Pool];
    }
    return FMemoryBudget();
}

int64 UMGMemoryManagerSubsystem::GetTotalBudget() const
{
    int64 Total = 0;
    for (const auto& Pair : PoolBudgets)
    {
        Total += Pair.Value.BudgetBytes;
    }
    return Total;
}

int64 UMGMemoryManagerSubsystem::GetTotalUsage() const
{
    int64 Total = 0;
    for (const auto& Pair : PoolBudgets)
    {
        Total += Pair.Value.CurrentUsageBytes;
    }
    return Total;
}

float UMGMemoryManagerSubsystem::GetOverallUsagePercent() const
{
    int64 TotalBudget = GetTotalBudget();
    if (TotalBudget > 0)
    {
        return (static_cast<float>(GetTotalUsage()) / static_cast<float>(TotalBudget)) * 100.0f;
    }
    return 0.0f;
}

void UMGMemoryManagerSubsystem::SetBudgetThresholds(EMemoryPool Pool, float Warning, float Critical)
{
    if (PoolBudgets.Contains(Pool))
    {
        PoolBudgets[Pool].WarningThreshold = Warning;
        PoolBudgets[Pool].CriticalThreshold = Critical;
    }
}

bool UMGMemoryManagerSubsystem::IsPoolOverBudget(EMemoryPool Pool) const
{
    if (PoolBudgets.Contains(Pool))
    {
        return PoolBudgets[Pool].bOverBudget;
    }
    return false;
}

bool UMGMemoryManagerSubsystem::IsAnyPoolOverBudget() const
{
    for (const auto& Pair : PoolBudgets)
    {
        if (Pair.Value.bOverBudget)
        {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Asset Streaming
// ============================================================================

FGuid UMGMemoryManagerSubsystem::RequestAssetLoad(const FString& AssetPath, EAssetPriority Priority, const FString& RequesterTag)
{
    // Check if asset is already loaded
    for (const auto& Pair : TrackedAssets)
    {
        if (Pair.Value.AssetPath == AssetPath && Pair.Value.State == EStreamingState::Loaded)
        {
            return Pair.Key;
        }
    }

    // Check queue size
    if (PendingRequests.Num() >= StreamingSettings.MaxQueueSize)
    {
        // Remove lowest priority request
        int32 LowestIndex = -1;
        EAssetPriority LowestPriority = EAssetPriority::Critical;

        for (int32 i = 0; i < PendingRequests.Num(); i++)
        {
            if (PendingRequests[i].Priority > LowestPriority)
            {
                LowestPriority = PendingRequests[i].Priority;
                LowestIndex = i;
            }
        }

        if (LowestIndex >= 0 && Priority < LowestPriority)
        {
            PendingRequests.RemoveAt(LowestIndex);
        }
        else
        {
            return FGuid();
        }
    }

    // Create load request
    FAssetLoadRequest Request;
    Request.RequestId = FGuid::NewGuid();
    Request.AssetPath = AssetPath;
    Request.Priority = Priority;
    Request.Strategy = StreamingSettings.DefaultLoadStrategy;
    Request.RequestTime = FDateTime::Now();
    Request.RequesterTag = RequesterTag;
    Request.TimeoutSeconds = 30.0f;

    // Insert based on priority
    int32 InsertIndex = PendingRequests.Num();
    for (int32 i = 0; i < PendingRequests.Num(); i++)
    {
        if (Priority < PendingRequests[i].Priority)
        {
            InsertIndex = i;
            break;
        }
    }

    PendingRequests.Insert(Request, InsertIndex);

    // Create tracked asset entry
    FStreamingAsset Asset;
    Asset.AssetId = Request.RequestId;
    Asset.AssetPath = AssetPath;
    Asset.AssetName = FPaths::GetBaseFilename(AssetPath);
    Asset.Priority = Priority;
    Asset.State = EStreamingState::Queued;
    Asset.SizeBytes = FMath::RandRange(1, 50) * 1024 * 1024; // Simulated size

    TrackedAssets.Add(Asset.AssetId, Asset);

    UE_LOG(LogTemp, Log, TEXT("Asset load requested: %s (Priority: %d)"), *AssetPath, static_cast<int32>(Priority));

    return Request.RequestId;
}

void UMGMemoryManagerSubsystem::RequestAssetUnload(const FGuid& AssetId)
{
    if (TrackedAssets.Contains(AssetId))
    {
        FStreamingAsset& Asset = TrackedAssets[AssetId];

        if (Asset.bNeverUnload)
        {
            return;
        }

        if (Asset.ReferenceCount > 0)
        {
            // Add to unload queue for later
            UnloadQueue.AddUnique(AssetId);
            return;
        }

        Asset.State = EStreamingState::Unloading;
        UnloadQueue.AddUnique(AssetId);
    }
}

void UMGMemoryManagerSubsystem::CancelLoadRequest(const FGuid& RequestId)
{
    for (int32 i = PendingRequests.Num() - 1; i >= 0; i--)
    {
        if (PendingRequests[i].RequestId == RequestId)
        {
            PendingRequests.RemoveAt(i);

            if (TrackedAssets.Contains(RequestId))
            {
                TrackedAssets.Remove(RequestId);
            }
            return;
        }
    }
}

EStreamingState UMGMemoryManagerSubsystem::GetAssetState(const FGuid& AssetId) const
{
    if (TrackedAssets.Contains(AssetId))
    {
        return TrackedAssets[AssetId].State;
    }
    return EStreamingState::NotLoaded;
}

FStreamingAsset UMGMemoryManagerSubsystem::GetAssetInfo(const FGuid& AssetId) const
{
    if (TrackedAssets.Contains(AssetId))
    {
        return TrackedAssets[AssetId];
    }
    return FStreamingAsset();
}

TArray<FStreamingAsset> UMGMemoryManagerSubsystem::GetLoadedAssets() const
{
    TArray<FStreamingAsset> LoadedAssets;
    for (const auto& Pair : TrackedAssets)
    {
        if (Pair.Value.State == EStreamingState::Loaded)
        {
            LoadedAssets.Add(Pair.Value);
        }
    }
    return LoadedAssets;
}

void UMGMemoryManagerSubsystem::SetAssetPriority(const FGuid& AssetId, EAssetPriority Priority)
{
    if (TrackedAssets.Contains(AssetId))
    {
        TrackedAssets[AssetId].Priority = Priority;
    }
}

void UMGMemoryManagerSubsystem::MarkAssetUsed(const FGuid& AssetId)
{
    if (TrackedAssets.Contains(AssetId))
    {
        TrackedAssets[AssetId].LastUsedTime = GameTimeSeconds;
        TrackedAssets[AssetId].UsageCount++;
    }
}

void UMGMemoryManagerSubsystem::SetAssetNeverUnload(const FGuid& AssetId, bool bNeverUnload)
{
    if (TrackedAssets.Contains(AssetId))
    {
        TrackedAssets[AssetId].bNeverUnload = bNeverUnload;
    }
}

// ============================================================================
// Level Streaming
// ============================================================================

void UMGMemoryManagerSubsystem::RegisterStreamingLevel(const FStreamingLevel& Level)
{
    // Check for duplicate
    for (const FStreamingLevel& Existing : StreamingLevels)
    {
        if (Existing.LevelName == Level.LevelName)
        {
            return;
        }
    }

    StreamingLevels.Add(Level);
}

void UMGMemoryManagerSubsystem::UnregisterStreamingLevel(const FString& LevelName)
{
    for (int32 i = StreamingLevels.Num() - 1; i >= 0; i--)
    {
        if (StreamingLevels[i].LevelName == LevelName)
        {
            StreamingLevels.RemoveAt(i);
            return;
        }
    }
}

void UMGMemoryManagerSubsystem::RequestLevelLoad(const FString& LevelName)
{
    for (FStreamingLevel& Level : StreamingLevels)
    {
        if (Level.LevelName == LevelName)
        {
            Level.bShouldBeLoaded = true;
            Level.State = EStreamingState::Loading;
            UE_LOG(LogTemp, Log, TEXT("Level load requested: %s"), *LevelName);
            return;
        }
    }
}

void UMGMemoryManagerSubsystem::RequestLevelUnload(const FString& LevelName)
{
    for (FStreamingLevel& Level : StreamingLevels)
    {
        if (Level.LevelName == LevelName)
        {
            Level.bShouldBeLoaded = false;
            Level.State = EStreamingState::Unloading;
            UE_LOG(LogTemp, Log, TEXT("Level unload requested: %s"), *LevelName);
            return;
        }
    }
}

void UMGMemoryManagerSubsystem::UpdateStreamingLevels(const FVector& ViewerLocation)
{
    for (FStreamingLevel& Level : StreamingLevels)
    {
        float Distance = FVector::Distance(ViewerLocation, Level.StreamingOrigin);
        bool bInRange = Distance <= Level.StreamingRadius;

        if (bInRange && !Level.bShouldBeLoaded)
        {
            RequestLevelLoad(Level.LevelName);
        }
        else if (!bInRange && Level.bShouldBeLoaded && Level.State == EStreamingState::Loaded)
        {
            RequestLevelUnload(Level.LevelName);
        }
    }
}

// ============================================================================
// Memory Pressure
// ============================================================================

void UMGMemoryManagerSubsystem::UpdateMemoryPressure()
{
    // Get system memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    float UsagePercent = MemStats.TotalPhysical > 0 ?
        static_cast<float>(MemStats.UsedPhysical) / static_cast<float>(MemStats.TotalPhysical) : 0.0f;

    // Update pool budgets
    for (auto& Pair : PoolBudgets)
    {
        Pair.Value.UsagePercent = Pair.Value.BudgetBytes > 0 ?
            static_cast<float>(Pair.Value.CurrentUsageBytes) / static_cast<float>(Pair.Value.BudgetBytes) : 0.0f;
        Pair.Value.bOverBudget = Pair.Value.UsagePercent >= 1.0f;

        if (Pair.Value.CurrentUsageBytes > Pair.Value.PeakUsageBytes)
        {
            Pair.Value.PeakUsageBytes = Pair.Value.CurrentUsageBytes;
        }
    }

    // Determine pressure level
    EMemoryPressureLevel NewLevel;
    if (UsagePercent >= 0.95f)
    {
        NewLevel = EMemoryPressureLevel::Critical;
    }
    else if (UsagePercent >= 0.85f)
    {
        NewLevel = EMemoryPressureLevel::High;
    }
    else if (UsagePercent >= 0.70f)
    {
        NewLevel = EMemoryPressureLevel::Medium;
    }
    else if (UsagePercent >= 0.50f)
    {
        NewLevel = EMemoryPressureLevel::Low;
    }
    else
    {
        NewLevel = EMemoryPressureLevel::None;
    }

    if (NewLevel != CurrentPressureLevel)
    {
        CurrentPressureLevel = NewLevel;
        OnMemoryPressureChanged.Broadcast(NewLevel);

        if (NewLevel >= EMemoryPressureLevel::High)
        {
            Stats.MemoryPressureEvents++;
            HandleMemoryPressure(NewLevel);
        }
    }

    CheckBudgetViolations();
}

void UMGMemoryManagerSubsystem::HandleMemoryPressure(EMemoryPressureLevel Level)
{
    switch (Level)
    {
        case EMemoryPressureLevel::Medium:
            // Schedule GC
            ScheduleIncrementalGC(5.0f);
            break;

        case EMemoryPressureLevel::High:
            // More aggressive cleanup
            TrimMemoryPools();
            RequestGarbageCollection(false);
            break;

        case EMemoryPressureLevel::Critical:
            // Emergency cleanup
            ForceMemoryCleanup();
            RequestGarbageCollection(true);
            break;

        default:
            break;
    }
}

void UMGMemoryManagerSubsystem::ForceMemoryCleanup()
{
    // Unload all non-essential assets
    TArray<FGuid> ToUnload;
    for (const auto& Pair : TrackedAssets)
    {
        if (Pair.Value.State == EStreamingState::Loaded &&
            !Pair.Value.bNeverUnload &&
            Pair.Value.Priority >= EAssetPriority::Low)
        {
            ToUnload.Add(Pair.Key);
        }
    }

    for (const FGuid& Id : ToUnload)
    {
        RequestAssetUnload(Id);
    }

    UE_LOG(LogTemp, Warning, TEXT("Force memory cleanup - queued %d assets for unload"), ToUnload.Num());
}

int64 UMGMemoryManagerSubsystem::FreeMemory(int64 TargetBytes)
{
    TArray<FGuid> AssetsToUnload = SelectAssetsToUnload(TargetBytes);

    int64 FreedBytes = 0;
    for (const FGuid& Id : AssetsToUnload)
    {
        if (TrackedAssets.Contains(Id))
        {
            FreedBytes += TrackedAssets[Id].LoadedSizeBytes;
            RequestAssetUnload(Id);
        }
    }

    return FreedBytes;
}

void UMGMemoryManagerSubsystem::TrimMemoryPools()
{
    for (auto& Pair : PoolBudgets)
    {
        if (Pair.Value.UsagePercent > Pair.Value.WarningThreshold)
        {
            int64 TargetReduction = static_cast<int64>(
                (Pair.Value.UsagePercent - 0.7f) * Pair.Value.BudgetBytes
            );
            FreeMemory(TargetReduction);
        }
    }
}

// ============================================================================
// Garbage Collection
// ============================================================================

void UMGMemoryManagerSubsystem::RequestGarbageCollection(bool bFullPurge)
{
    float StartTime = FPlatformTime::Seconds();

    if (bFullPurge)
    {
        CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    }
    else
    {
        TryCollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    }

    float EndTime = FPlatformTime::Seconds();
    float CollectionTime = (EndTime - StartTime) * 1000.0f;

    // Update stats
    Stats.GCStats.TotalCollections++;
    Stats.GCStats.TotalCollectionTimeMs += CollectionTime;
    Stats.GCStats.AverageCollectionTimeMs = Stats.GCStats.TotalCollectionTimeMs / Stats.GCStats.TotalCollections;
    Stats.GCStats.MaxCollectionTimeMs = FMath::Max(Stats.GCStats.MaxCollectionTimeMs, CollectionTime);
    Stats.GCStats.LastCollectionTime = FDateTime::Now();

    OnGarbageCollectionComplete.Broadcast(CollectionTime);

    UE_LOG(LogTemp, Log, TEXT("GC completed in %.2f ms (Full: %s)"), CollectionTime, bFullPurge ? TEXT("Yes") : TEXT("No"));
}

void UMGMemoryManagerSubsystem::ScheduleIncrementalGC(float TimeSliceMs)
{
    // Schedule incremental GC
    IncrementalPurgeGarbage(false);
}

void UMGMemoryManagerSubsystem::SetGCFrequency(float IntervalSeconds)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(GCTimer);
        TWeakObjectPtr<UMGMemoryManagerSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(GCTimer, [WeakThis]()
        {
            if (WeakThis.IsValid() && WeakThis->CurrentPressureLevel >= EMemoryPressureLevel::Medium)
            {
                WeakThis->RequestGarbageCollection(false);
            }
        }, IntervalSeconds, true);
    }
}

// ============================================================================
// Predictive Loading
// ============================================================================

void UMGMemoryManagerSubsystem::UpdatePlayerPosition(const FVector& Position, const FVector& Velocity)
{
    LastPlayerPosition = Position;
    LastPlayerVelocity = Velocity;

    if (StreamingSettings.bPredictiveLoading && Velocity.Size() > 100.0f)
    {
        FVector PredictedPosition = Position + Velocity * 5.0f;
        PreloadAreaAssets(PredictedPosition, StreamingSettings.PredictiveLoadDistance * 0.5f);
    }

    UpdateStreamingLevels(Position);
}

void UMGMemoryManagerSubsystem::PreloadAreaAssets(const FVector& Center, float Radius)
{
    for (auto& Pair : TrackedAssets)
    {
        if (Pair.Value.State == EStreamingState::NotLoaded)
        {
            float Distance = FVector::Distance(Center, Pair.Value.LastKnownPosition);
            if (Distance <= Radius)
            {
                RequestAssetLoad(Pair.Value.AssetPath, EAssetPriority::Low, TEXT("Predictive"));
            }
        }
    }
}

void UMGMemoryManagerSubsystem::SetPredictiveLoadDistance(float Distance)
{
    StreamingSettings.PredictiveLoadDistance = Distance;
}

void UMGMemoryManagerSubsystem::AddPredictiveLoadPath(const TArray<FVector>& PathPoints)
{
    for (const FVector& Point : PathPoints)
    {
        PreloadAreaAssets(Point, StreamingSettings.PredictiveLoadDistance);
    }
}

// ============================================================================
// Snapshots
// ============================================================================

FMemorySnapshot UMGMemoryManagerSubsystem::TakeMemorySnapshot()
{
    FMemorySnapshot Snapshot;
    Snapshot.CapturedAt = FDateTime::Now();

    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Snapshot.TotalAllocated = MemStats.TotalPhysical;
    Snapshot.TotalUsed = MemStats.UsedPhysical;
    Snapshot.TotalAvailable = MemStats.AvailablePhysical;

    Snapshot.PoolBudgets = PoolBudgets;

    int32 LoadedCount = 0;
    for (const auto& Pair : TrackedAssets)
    {
        if (Pair.Value.State == EStreamingState::Loaded)
        {
            LoadedCount++;
        }
    }
    Snapshot.TotalLoadedAssets = LoadedCount;
    Snapshot.StreamingQueueSize = PendingRequests.Num();
    Snapshot.PressureLevel = CurrentPressureLevel;

    SnapshotHistory.Add(Snapshot);

    // Limit history size
    if (SnapshotHistory.Num() > 100)
    {
        SnapshotHistory.RemoveAt(0);
    }

    return Snapshot;
}

void UMGMemoryManagerSubsystem::ClearSnapshotHistory()
{
    SnapshotHistory.Empty();
}

// ============================================================================
// Settings
// ============================================================================

void UMGMemoryManagerSubsystem::UpdateStreamingSettings(const FStreamingSettings& NewSettings)
{
    StreamingSettings = NewSettings;
}

void UMGMemoryManagerSubsystem::SetMaxConcurrentLoads(int32 MaxLoads)
{
    StreamingSettings.MaxConcurrentLoads = FMath::Clamp(MaxLoads, 1, 32);
}

void UMGMemoryManagerSubsystem::SetStreamingBandwidth(float MBps)
{
    StreamingSettings.MaxStreamingBandwidthMBps = FMath::Clamp(MBps, 10.0f, 500.0f);
}

// ============================================================================
// Statistics
// ============================================================================

void UMGMemoryManagerSubsystem::ResetStats()
{
    Stats = FMemoryManagerStats();
}

FString UMGMemoryManagerSubsystem::GenerateMemoryReport() const
{
    FString Report = TEXT("=== Memory Manager Report ===\n\n");

    // Overall stats
    Report += FString::Printf(TEXT("Total Budget: %lld MB\n"), GetTotalBudget() / (1024 * 1024));
    Report += FString::Printf(TEXT("Total Usage: %lld MB (%.1f%%)\n"), GetTotalUsage() / (1024 * 1024), GetOverallUsagePercent());
    Report += FString::Printf(TEXT("Pressure Level: %d\n\n"), static_cast<int32>(CurrentPressureLevel));

    // Pool breakdown
    Report += TEXT("--- Pool Budgets ---\n");
    for (const auto& Pair : PoolBudgets)
    {
        Report += FString::Printf(TEXT("%d: %lld / %lld MB (%.1f%%)\n"),
            static_cast<int32>(Pair.Key),
            Pair.Value.CurrentUsageBytes / (1024 * 1024),
            Pair.Value.BudgetBytes / (1024 * 1024),
            Pair.Value.UsagePercent * 100.0f
        );
    }

    // Streaming stats
    Report += TEXT("\n--- Streaming ---\n");
    Report += FString::Printf(TEXT("Loaded Assets: %d\n"), GetLoadedAssets().Num());
    Report += FString::Printf(TEXT("Pending Requests: %d\n"), PendingRequests.Num());
    Report += FString::Printf(TEXT("Total Loads: %d\n"), Stats.TotalAssetLoads);
    Report += FString::Printf(TEXT("Total Unloads: %d\n"), Stats.TotalAssetUnloads);
    Report += FString::Printf(TEXT("Load Failures: %d\n"), Stats.LoadFailures);

    // GC stats
    Report += TEXT("\n--- Garbage Collection ---\n");
    Report += FString::Printf(TEXT("Total Collections: %d\n"), Stats.GCStats.TotalCollections);
    Report += FString::Printf(TEXT("Average Time: %.2f ms\n"), Stats.GCStats.AverageCollectionTimeMs);
    Report += FString::Printf(TEXT("Max Time: %.2f ms\n"), Stats.GCStats.MaxCollectionTimeMs);

    return Report;
}

// ============================================================================
// Internal Processing
// ============================================================================

void UMGMemoryManagerSubsystem::ProcessLoadQueue()
{
    GameTimeSeconds += 0.05f;

    if (PendingRequests.Num() == 0)
    {
        return;
    }

    // Check concurrent load limit
    if (CurrentConcurrentLoads >= StreamingSettings.MaxConcurrentLoads)
    {
        return;
    }

    // Check memory pressure
    if (CurrentPressureLevel >= EMemoryPressureLevel::Critical)
    {
        return;
    }

    // Process next request
    FAssetLoadRequest Request = PendingRequests[0];
    PendingRequests.RemoveAt(0);

    if (TrackedAssets.Contains(Request.RequestId))
    {
        FStreamingAsset& Asset = TrackedAssets[Request.RequestId];
        Asset.State = EStreamingState::Loading;
        CurrentConcurrentLoads++;

        // Simulate async load
        if (UWorld* World = GetWorld())
        {
            float LoadTime = FMath::RandRange(0.1f, 0.5f);
            World->GetTimerManager().SetTimerForNextTick([this, Request, LoadTime]()
            {
                if (UWorld* InnerWorld = GetWorld())
                {
                    FTimerHandle TempHandle;
                    InnerWorld->GetTimerManager().SetTimer(TempHandle, [this, Request]()
                    {
                        CompleteAssetLoad(Request.RequestId);
                    }, LoadTime, false);
                }
            });
        }
    }
}

void UMGMemoryManagerSubsystem::CompleteAssetLoad(FGuid AssetId)
{
    CurrentConcurrentLoads--;

    if (TrackedAssets.Contains(AssetId))
    {
        FStreamingAsset& Asset = TrackedAssets[AssetId];
        Asset.State = EStreamingState::Loaded;
        Asset.LoadedSizeBytes = Asset.SizeBytes;
        Asset.LastUsedTime = GameTimeSeconds;
        Asset.bIsResident = true;

        // Update pool usage
        if (PoolBudgets.Contains(Asset.Pool))
        {
            PoolBudgets[Asset.Pool].CurrentUsageBytes += Asset.LoadedSizeBytes;
            PoolBudgets[Asset.Pool].AssetCount++;
        }

        Stats.TotalAssetLoads++;
        Stats.TotalBytesLoaded += Asset.LoadedSizeBytes;

        OnAssetLoaded.Broadcast(Asset);

        UE_LOG(LogTemp, Log, TEXT("Asset loaded: %s (%lld bytes)"), *Asset.AssetName, Asset.LoadedSizeBytes);
    }
}

void UMGMemoryManagerSubsystem::ProcessUnloadQueue()
{
    while (UnloadQueue.Num() > 0)
    {
        FGuid AssetId = UnloadQueue[0];
        UnloadQueue.RemoveAt(0);

        if (TrackedAssets.Contains(AssetId))
        {
            FStreamingAsset& Asset = TrackedAssets[AssetId];

            if (Asset.ReferenceCount > 0)
            {
                continue;
            }

            // Update pool usage
            if (PoolBudgets.Contains(Asset.Pool))
            {
                PoolBudgets[Asset.Pool].CurrentUsageBytes -= Asset.LoadedSizeBytes;
                PoolBudgets[Asset.Pool].AssetCount--;
            }

            Stats.TotalAssetUnloads++;
            Stats.TotalBytesUnloaded += Asset.LoadedSizeBytes;

            OnAssetUnloaded.Broadcast(Asset);

            Asset.State = EStreamingState::NotLoaded;
            Asset.LoadedSizeBytes = 0;
            Asset.bIsResident = false;

            UE_LOG(LogTemp, Log, TEXT("Asset unloaded: %s"), *Asset.AssetName);
        }
    }
}

void UMGMemoryManagerSubsystem::CheckBudgetViolations()
{
    for (const auto& Pair : PoolBudgets)
    {
        if (Pair.Value.UsagePercent >= Pair.Value.CriticalThreshold)
        {
            Stats.BudgetViolations++;
            OnBudgetViolation.Broadcast(Pair.Key, Pair.Value.UsagePercent);
        }
    }
}

TArray<FGuid> UMGMemoryManagerSubsystem::SelectAssetsToUnload(int64 TargetBytes)
{
    TArray<FGuid> Selected;
    int64 SelectedBytes = 0;

    // Score all loaded assets
    TArray<TPair<FGuid, float>> ScoredAssets;
    for (const auto& Pair : TrackedAssets)
    {
        if (Pair.Value.State == EStreamingState::Loaded && !Pair.Value.bNeverUnload && Pair.Value.ReferenceCount == 0)
        {
            float Score = CalculateAssetUnloadScore(Pair.Value);
            ScoredAssets.Add(TPair<FGuid, float>(Pair.Key, Score));
        }
    }

    // Sort by score (higher = more likely to unload)
    ScoredAssets.Sort([](const TPair<FGuid, float>& A, const TPair<FGuid, float>& B)
    {
        return A.Value > B.Value;
    });

    // Select assets until target is reached
    for (const auto& ScoredAsset : ScoredAssets)
    {
        if (SelectedBytes >= TargetBytes)
        {
            break;
        }

        Selected.Add(ScoredAsset.Key);
        SelectedBytes += TrackedAssets[ScoredAsset.Key].LoadedSizeBytes;
    }

    return Selected;
}

float UMGMemoryManagerSubsystem::CalculateAssetUnloadScore(const FStreamingAsset& Asset)
{
    float Score = 0.0f;

    // Time since last use (higher = more unloadable)
    float TimeSinceUse = GameTimeSeconds - Asset.LastUsedTime;
    Score += FMath::Clamp(TimeSinceUse / StreamingSettings.AssetRetentionTimeSeconds, 0.0f, 1.0f) * 40.0f;

    // Usage frequency (lower = more unloadable)
    Score += FMath::Clamp(1.0f - (Asset.UsageCount / 100.0f), 0.0f, 1.0f) * 20.0f;

    // Priority (lower priority = more unloadable)
    Score += (static_cast<float>(Asset.Priority) / 4.0f) * 20.0f;

    // Size (larger = more impact, might want to unload)
    Score += FMath::Clamp(Asset.LoadedSizeBytes / (100.0f * 1024.0f * 1024.0f), 0.0f, 1.0f) * 20.0f;

    return Score;
}

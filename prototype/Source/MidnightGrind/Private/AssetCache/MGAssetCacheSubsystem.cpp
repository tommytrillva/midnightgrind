// MGAssetCacheSubsystem.cpp
// Midnight Grind - Asset Caching and Preloading System

#include "AssetCache/MGAssetCacheSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Guid.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/AssetManager.h"

void UMGAssetCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize default configuration
    CacheConfig.MaxCacheSizeBytes = 1024 * 1024 * 1024; // 1 GB
    CacheConfig.TargetCacheSizeBytes = 768 * 1024 * 1024; // 768 MB
    CacheConfig.EvictionPolicy = EMGCacheEvictionPolicy::LRU;
    CacheConfig.MaxConcurrentLoads = 4;
    CacheConfig.EvictionThreshold = 0.9f;
    CacheConfig.AggressiveEvictionThreshold = 0.95f;
    CacheConfig.bEnablePredictiveLoading = true;
    CacheConfig.bEnableCompression = false;
    CacheConfig.PredictionLookaheadSeconds = 10.0f;
    CacheConfig.MinConfidenceForPreload = 0.7f;
    CacheConfig.bPreferStreamedAssets = true;
    CacheConfig.bTrackAccessPatterns = true;

    // Set default category budgets (proportional)
    CacheConfig.CategoryBudgets.Add(EMGAssetCategory::Vehicle, 256 * 1024 * 1024);     // 256 MB
    CacheConfig.CategoryBudgets.Add(EMGAssetCategory::Track, 384 * 1024 * 1024);       // 384 MB
    CacheConfig.CategoryBudgets.Add(EMGAssetCategory::Environment, 128 * 1024 * 1024); // 128 MB
    CacheConfig.CategoryBudgets.Add(EMGAssetCategory::UI, 64 * 1024 * 1024);           // 64 MB
    CacheConfig.CategoryBudgets.Add(EMGAssetCategory::Audio, 128 * 1024 * 1024);       // 128 MB
    CacheConfig.CategoryBudgets.Add(EMGAssetCategory::VFX, 96 * 1024 * 1024);          // 96 MB

    // Initialize stats
    CacheStats.MaxCacheSizeBytes = CacheConfig.MaxCacheSizeBytes;

    // Initialize default bundles
    InitializeDefaultBundles();

    // Start cache maintenance timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGAssetCacheSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            CacheMaintenanceHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->PerformCacheMaintenance();
                }
            },
            5.0f,
            true
        );

        // Start load queue processor
        World->GetTimerManager().SetTimer(
            LoadQueueHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->ProcessLoadQueue();
                }
            },
            0.1f,
            true
        );

        // Start prediction timer
        if (CacheConfig.bEnablePredictiveLoading)
        {
            World->GetTimerManager().SetTimer(
                PredictionHandle,
                [WeakThis]()
                {
                    if (WeakThis.IsValid())
                    {
                        WeakThis->GeneratePredictions();
                    }
                },
                2.0f,
                true
            );
        }
    }
}

void UMGAssetCacheSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CacheMaintenanceHandle);
        World->GetTimerManager().ClearTimer(LoadQueueHandle);
        World->GetTimerManager().ClearTimer(PredictionHandle);
    }

    // Clear all streamable handles
    for (auto& Pair : StreamableHandles)
    {
        if (Pair.Value.IsValid())
        {
            Pair.Value->CancelHandle();
        }
    }
    StreamableHandles.Empty();

    CachedAssets.Empty();
    AssetBundles.Empty();
    ActivePreloads.Empty();

    Super::Deinitialize();
}

void UMGAssetCacheSubsystem::InitializeDefaultBundles()
{
    // Create common bundles for the game

    // UI Bundle
    FMGAssetBundle UIBundle;
    UIBundle.BundleId = FGuid::NewGuid();
    UIBundle.BundleName = FName("CommonUI");
    UIBundle.Priority = EMGCachePriority::High;
    UIBundle.Strategy = EMGPreloadStrategy::Immediate;
    UIBundle.Tags.Add(TEXT("UI"));
    UIBundle.Tags.Add(TEXT("Persistent"));
    AssetBundles.Add(UIBundle);

    // Main Menu Bundle
    FMGAssetBundle MenuBundle;
    MenuBundle.BundleId = FGuid::NewGuid();
    MenuBundle.BundleName = FName("MainMenu");
    MenuBundle.Priority = EMGCachePriority::High;
    MenuBundle.Strategy = EMGPreloadStrategy::Immediate;
    MenuBundle.Tags.Add(TEXT("Menu"));
    AssetBundles.Add(MenuBundle);

    // Garage Bundle
    FMGAssetBundle GarageBundle;
    GarageBundle.BundleId = FGuid::NewGuid();
    GarageBundle.BundleName = FName("Garage");
    GarageBundle.Priority = EMGCachePriority::Normal;
    GarageBundle.Strategy = EMGPreloadStrategy::Predictive;
    GarageBundle.Tags.Add(TEXT("Garage"));
    AssetBundles.Add(GarageBundle);

    // Race Common Bundle
    FMGAssetBundle RaceBundle;
    RaceBundle.BundleId = FGuid::NewGuid();
    RaceBundle.BundleName = FName("RaceCommon");
    RaceBundle.Priority = EMGCachePriority::Critical;
    RaceBundle.Strategy = EMGPreloadStrategy::Immediate;
    RaceBundle.Tags.Add(TEXT("Race"));
    RaceBundle.Tags.Add(TEXT("Core"));
    AssetBundles.Add(RaceBundle);
}

void UMGAssetCacheSubsystem::LoadAsset(const FSoftObjectPath& AssetPath, EMGCachePriority Priority)
{
    if (AssetPath.IsNull())
    {
        return;
    }

    // Check if already cached
    if (FMGCachedAsset* Existing = CachedAssets.Find(AssetPath))
    {
        if (Existing->LoadState == EMGAssetLoadState::Loaded || Existing->LoadState == EMGAssetLoadState::Cached)
        {
            Existing->LastAccessTime = FDateTime::Now();
            Existing->AccessCount++;
            CacheStats.CacheHits++;
            return;
        }
    }

    CacheStats.CacheMisses++;

    // Check if already loading or queued
    if (CurrentlyLoading.Contains(AssetPath) || LoadQueue.Contains(AssetPath))
    {
        return;
    }

    // Create cache entry
    FMGCachedAsset Entry;
    Entry.AssetId = FGuid::NewGuid();
    Entry.AssetPath = AssetPath;
    Entry.AssetName = FName(*AssetPath.GetAssetName());
    Entry.Category = DetermineAssetCategory(AssetPath);
    Entry.Priority = Priority;
    Entry.LoadState = EMGAssetLoadState::Queued;
    CachedAssets.Add(AssetPath, Entry);

    // Add to queue based on priority
    int32 InsertIndex = 0;
    for (int32 i = 0; i < LoadQueue.Num(); ++i)
    {
        if (FMGCachedAsset* QueuedAsset = CachedAssets.Find(LoadQueue[i]))
        {
            if (static_cast<int32>(Priority) < static_cast<int32>(QueuedAsset->Priority))
            {
                InsertIndex = i;
                break;
            }
        }
        InsertIndex = i + 1;
    }

    LoadQueue.Insert(AssetPath, InsertIndex);
    CacheStats.PendingLoads = LoadQueue.Num();
}

void UMGAssetCacheSubsystem::LoadAssets(const TArray<FSoftObjectPath>& AssetPaths, EMGCachePriority Priority)
{
    for (const FSoftObjectPath& Path : AssetPaths)
    {
        LoadAsset(Path, Priority);
    }
}

FGuid UMGAssetCacheSubsystem::RequestPreload(const TArray<FSoftObjectPath>& AssetPaths, EMGPreloadStrategy Strategy, EMGCachePriority Priority)
{
    FMGPreloadRequest Request;
    Request.RequestId = FGuid::NewGuid();
    Request.AssetPaths = AssetPaths;
    Request.Priority = Priority;
    Request.Strategy = Strategy;
    Request.RequestTime = FDateTime::Now();
    Request.TotalCount = AssetPaths.Num();
    Request.LoadedCount = 0;
    Request.Progress = 0.0f;
    Request.bCompleted = false;
    Request.bCancelled = false;

    ActivePreloads.Add(Request.RequestId, Request);

    // Start loading based on strategy
    switch (Strategy)
    {
    case EMGPreloadStrategy::Immediate:
        LoadAssets(AssetPaths, Priority);
        break;

    case EMGPreloadStrategy::Progressive:
        // Load first few immediately, queue rest
        for (int32 i = 0; i < AssetPaths.Num(); ++i)
        {
            EMGCachePriority ProgressivePriority = (i < 3) ? Priority : EMGCachePriority::Low;
            LoadAsset(AssetPaths[i], ProgressivePriority);
        }
        break;

    case EMGPreloadStrategy::Lazy:
        // Just queue them with low priority
        for (const FSoftObjectPath& Path : AssetPaths)
        {
            LoadAsset(Path, EMGCachePriority::Background);
        }
        break;

    case EMGPreloadStrategy::Predictive:
    case EMGPreloadStrategy::OnDemand:
        // Queue but don't start loading yet
        for (const FSoftObjectPath& Path : AssetPaths)
        {
            FMGCachedAsset Entry;
            Entry.AssetId = FGuid::NewGuid();
            Entry.AssetPath = Path;
            Entry.AssetName = FName(*Path.GetAssetName());
            Entry.Category = DetermineAssetCategory(Path);
            Entry.Priority = Priority;
            Entry.LoadState = EMGAssetLoadState::NotLoaded;
            CachedAssets.Add(Path, Entry);
        }
        break;
    }

    return Request.RequestId;
}

void UMGAssetCacheSubsystem::CancelPreload(const FGuid& RequestId)
{
    if (FMGPreloadRequest* Request = ActivePreloads.Find(RequestId))
    {
        Request->bCancelled = true;

        // Remove queued assets that haven't started loading
        for (const FSoftObjectPath& Path : Request->AssetPaths)
        {
            LoadQueue.Remove(Path);

            if (!CurrentlyLoading.Contains(Path))
            {
                CachedAssets.Remove(Path);
            }
        }

        ActivePreloads.Remove(RequestId);
    }
}

void UMGAssetCacheSubsystem::CancelAllPreloads()
{
    TArray<FGuid> RequestIds;
    ActivePreloads.GetKeys(RequestIds);

    for (const FGuid& Id : RequestIds)
    {
        CancelPreload(Id);
    }
}

float UMGAssetCacheSubsystem::GetPreloadProgress(const FGuid& RequestId) const
{
    if (const FMGPreloadRequest* Request = ActivePreloads.Find(RequestId))
    {
        return Request->Progress;
    }
    return 0.0f;
}

bool UMGAssetCacheSubsystem::IsPreloadComplete(const FGuid& RequestId) const
{
    if (const FMGPreloadRequest* Request = ActivePreloads.Find(RequestId))
    {
        return Request->bCompleted;
    }
    return false;
}

void UMGAssetCacheSubsystem::SetLoadPriority(const FSoftObjectPath& AssetPath, EMGCachePriority NewPriority)
{
    if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        Asset->Priority = NewPriority;

        // Re-sort queue if asset is queued
        if (Asset->LoadState == EMGAssetLoadState::Queued && LoadQueue.Contains(AssetPath))
        {
            LoadQueue.Remove(AssetPath);

            int32 InsertIndex = 0;
            for (int32 i = 0; i < LoadQueue.Num(); ++i)
            {
                if (FMGCachedAsset* QueuedAsset = CachedAssets.Find(LoadQueue[i]))
                {
                    if (static_cast<int32>(NewPriority) < static_cast<int32>(QueuedAsset->Priority))
                    {
                        InsertIndex = i;
                        break;
                    }
                }
                InsertIndex = i + 1;
            }

            LoadQueue.Insert(AssetPath, InsertIndex);
        }
    }
}

UObject* UMGAssetCacheSubsystem::GetCachedAsset(const FSoftObjectPath& AssetPath)
{
    if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        if (Asset->LoadState == EMGAssetLoadState::Loaded || Asset->LoadState == EMGAssetLoadState::Cached)
        {
            Asset->LastAccessTime = FDateTime::Now();
            Asset->AccessCount++;
            CacheStats.CacheHits++;

            if (CacheConfig.bTrackAccessPatterns)
            {
                RecordAssetAccess(AssetPath, CurrentPredictionContext);
            }

            return Asset->CachedObject.Get();
        }
    }

    CacheStats.CacheMisses++;
    return nullptr;
}

bool UMGAssetCacheSubsystem::IsAssetCached(const FSoftObjectPath& AssetPath) const
{
    if (const FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        return Asset->LoadState == EMGAssetLoadState::Loaded || Asset->LoadState == EMGAssetLoadState::Cached;
    }
    return false;
}

bool UMGAssetCacheSubsystem::IsAssetLoading(const FSoftObjectPath& AssetPath) const
{
    if (const FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        return Asset->LoadState == EMGAssetLoadState::Loading || Asset->LoadState == EMGAssetLoadState::Queued;
    }
    return false;
}

EMGAssetLoadState UMGAssetCacheSubsystem::GetAssetLoadState(const FSoftObjectPath& AssetPath) const
{
    if (const FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        return Asset->LoadState;
    }
    return EMGAssetLoadState::NotLoaded;
}

TArray<FSoftObjectPath> UMGAssetCacheSubsystem::GetCachedAssetPaths() const
{
    TArray<FSoftObjectPath> Paths;

    for (const auto& Pair : CachedAssets)
    {
        if (Pair.Value.LoadState == EMGAssetLoadState::Loaded || Pair.Value.LoadState == EMGAssetLoadState::Cached)
        {
            Paths.Add(Pair.Key);
        }
    }

    return Paths;
}

TArray<FSoftObjectPath> UMGAssetCacheSubsystem::GetLoadingAssetPaths() const
{
    TArray<FSoftObjectPath> Paths;

    for (const auto& Pair : CachedAssets)
    {
        if (Pair.Value.LoadState == EMGAssetLoadState::Loading || Pair.Value.LoadState == EMGAssetLoadState::Queued)
        {
            Paths.Add(Pair.Key);
        }
    }

    return Paths;
}

FGuid UMGAssetCacheSubsystem::CreateBundle(const FName& BundleName, const TArray<FSoftObjectPath>& Assets)
{
    FMGAssetBundle Bundle;
    Bundle.BundleId = FGuid::NewGuid();
    Bundle.BundleName = BundleName;
    Bundle.Assets = Assets;
    Bundle.Priority = EMGCachePriority::Normal;
    Bundle.Strategy = EMGPreloadStrategy::Immediate;
    Bundle.TotalSizeBytes = EstimateBundleSize(Assets);
    Bundle.LoadProgress = 0.0f;
    Bundle.bFullyLoaded = false;

    AssetBundles.Add(Bundle);
    return Bundle.BundleId;
}

void UMGAssetCacheSubsystem::LoadBundle(const FGuid& BundleId, EMGPreloadStrategy Strategy)
{
    for (FMGAssetBundle& Bundle : AssetBundles)
    {
        if (Bundle.BundleId == BundleId)
        {
            Bundle.Strategy = Strategy;
            LoadAssets(Bundle.Assets, Bundle.Priority);
            return;
        }
    }
}

void UMGAssetCacheSubsystem::LoadBundleByName(const FName& BundleName, EMGPreloadStrategy Strategy)
{
    for (FMGAssetBundle& Bundle : AssetBundles)
    {
        if (Bundle.BundleName == BundleName)
        {
            LoadBundle(Bundle.BundleId, Strategy);
            return;
        }
    }
}

void UMGAssetCacheSubsystem::UnloadBundle(const FGuid& BundleId)
{
    for (FMGAssetBundle& Bundle : AssetBundles)
    {
        if (Bundle.BundleId == BundleId)
        {
            for (const FSoftObjectPath& AssetPath : Bundle.Assets)
            {
                EvictAsset(AssetPath);
            }
            Bundle.bFullyLoaded = false;
            Bundle.LoadProgress = 0.0f;
            return;
        }
    }
}

void UMGAssetCacheSubsystem::AddAssetToBundle(const FGuid& BundleId, const FSoftObjectPath& AssetPath)
{
    for (FMGAssetBundle& Bundle : AssetBundles)
    {
        if (Bundle.BundleId == BundleId)
        {
            if (!Bundle.Assets.Contains(AssetPath))
            {
                Bundle.Assets.Add(AssetPath);
                Bundle.TotalSizeBytes = EstimateBundleSize(Bundle.Assets);
            }
            return;
        }
    }
}

void UMGAssetCacheSubsystem::RemoveAssetFromBundle(const FGuid& BundleId, const FSoftObjectPath& AssetPath)
{
    for (FMGAssetBundle& Bundle : AssetBundles)
    {
        if (Bundle.BundleId == BundleId)
        {
            Bundle.Assets.Remove(AssetPath);
            Bundle.TotalSizeBytes = EstimateBundleSize(Bundle.Assets);
            return;
        }
    }
}

bool UMGAssetCacheSubsystem::IsBundleLoaded(const FGuid& BundleId) const
{
    for (const FMGAssetBundle& Bundle : AssetBundles)
    {
        if (Bundle.BundleId == BundleId)
        {
            return Bundle.bFullyLoaded;
        }
    }
    return false;
}

float UMGAssetCacheSubsystem::GetBundleLoadProgress(const FGuid& BundleId) const
{
    for (const FMGAssetBundle& Bundle : AssetBundles)
    {
        if (Bundle.BundleId == BundleId)
        {
            if (Bundle.Assets.Num() == 0)
            {
                return 1.0f;
            }

            int32 LoadedCount = 0;
            for (const FSoftObjectPath& AssetPath : Bundle.Assets)
            {
                if (IsAssetCached(AssetPath))
                {
                    LoadedCount++;
                }
            }

            return static_cast<float>(LoadedCount) / static_cast<float>(Bundle.Assets.Num());
        }
    }
    return 0.0f;
}

FMGAssetBundle UMGAssetCacheSubsystem::GetBundleById(const FGuid& BundleId) const
{
    for (const FMGAssetBundle& Bundle : AssetBundles)
    {
        if (Bundle.BundleId == BundleId)
        {
            return Bundle;
        }
    }
    return FMGAssetBundle();
}

void UMGAssetCacheSubsystem::EvictAsset(const FSoftObjectPath& AssetPath)
{
    if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        if (Asset->bPersistent)
        {
            return; // Don't evict persistent assets
        }

        int64 FreedBytes = Asset->SizeBytes;

        // Cancel any active handle
        if (TSharedPtr<FStreamableHandle>* HandlePtr = StreamableHandles.Find(AssetPath))
        {
            if (HandlePtr->IsValid())
            {
                (*HandlePtr)->CancelHandle();
            }
            StreamableHandles.Remove(AssetPath);
        }

        Asset->CachedObject.Reset();
        Asset->LoadState = EMGAssetLoadState::Evicted;

        CacheStats.EvictionCount++;
        CacheStats.BytesEvicted += FreedBytes;
        CacheStats.TotalCacheSizeBytes -= FreedBytes;

        CachedAssets.Remove(AssetPath);

        OnAssetEvicted.Broadcast(AssetPath, FreedBytes);
    }
}

void UMGAssetCacheSubsystem::EvictAssetsByCategory(EMGAssetCategory Category)
{
    TArray<FSoftObjectPath> ToEvict;

    for (const auto& Pair : CachedAssets)
    {
        if (Pair.Value.Category == Category && !Pair.Value.bPersistent)
        {
            ToEvict.Add(Pair.Key);
        }
    }

    for (const FSoftObjectPath& Path : ToEvict)
    {
        EvictAsset(Path);
    }
}

void UMGAssetCacheSubsystem::EvictAssetsByTag(const FString& Tag)
{
    TArray<FSoftObjectPath> ToEvict;

    for (const auto& Pair : CachedAssets)
    {
        if (Pair.Value.Tags.Contains(Tag) && !Pair.Value.bPersistent)
        {
            ToEvict.Add(Pair.Key);
        }
    }

    for (const FSoftObjectPath& Path : ToEvict)
    {
        EvictAsset(Path);
    }
}

void UMGAssetCacheSubsystem::EvictOldestAssets(int64 BytesToFree)
{
    EvictByPolicy(BytesToFree);
}

void UMGAssetCacheSubsystem::ClearCache()
{
    TArray<FSoftObjectPath> ToEvict;

    for (const auto& Pair : CachedAssets)
    {
        if (!Pair.Value.bPersistent)
        {
            ToEvict.Add(Pair.Key);
        }
    }

    for (const FSoftObjectPath& Path : ToEvict)
    {
        EvictAsset(Path);
    }
}

void UMGAssetCacheSubsystem::TrimCache()
{
    if (CacheStats.TotalCacheSizeBytes > CacheConfig.TargetCacheSizeBytes)
    {
        int64 BytesToFree = CacheStats.TotalCacheSizeBytes - CacheConfig.TargetCacheSizeBytes;
        EvictByPolicy(BytesToFree);
    }
}

void UMGAssetCacheSubsystem::SetAssetPersistent(const FSoftObjectPath& AssetPath, bool bPersistent)
{
    if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        Asset->bPersistent = bPersistent;
    }
}

void UMGAssetCacheSubsystem::TagAsset(const FSoftObjectPath& AssetPath, const FString& Tag)
{
    if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        if (!Asset->Tags.Contains(Tag))
        {
            Asset->Tags.Add(Tag);
        }
    }
}

void UMGAssetCacheSubsystem::UntagAsset(const FSoftObjectPath& AssetPath, const FString& Tag)
{
    if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        Asset->Tags.Remove(Tag);
    }
}

TArray<FSoftObjectPath> UMGAssetCacheSubsystem::GetAssetsByTag(const FString& Tag) const
{
    TArray<FSoftObjectPath> Result;

    for (const auto& Pair : CachedAssets)
    {
        if (Pair.Value.Tags.Contains(Tag))
        {
            Result.Add(Pair.Key);
        }
    }

    return Result;
}

TArray<FSoftObjectPath> UMGAssetCacheSubsystem::GetAssetsByCategory(EMGAssetCategory Category) const
{
    TArray<FSoftObjectPath> Result;

    for (const auto& Pair : CachedAssets)
    {
        if (Pair.Value.Category == Category)
        {
            Result.Add(Pair.Key);
        }
    }

    return Result;
}

void UMGAssetCacheSubsystem::ApplyCacheConfig(const FMGCacheConfig& Config)
{
    CacheConfig = Config;
    CacheStats.MaxCacheSizeBytes = Config.MaxCacheSizeBytes;

    // Restart prediction timer if needed
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PredictionHandle);

        if (CacheConfig.bEnablePredictiveLoading)
        {
            TWeakObjectPtr<UMGAssetCacheSubsystem> WeakThis(this);
            World->GetTimerManager().SetTimer(
                PredictionHandle,
                [WeakThis]()
                {
                    if (WeakThis.IsValid())
                    {
                        WeakThis->GeneratePredictions();
                    }
                },
                2.0f,
                true
            );
        }
    }

    // Trim if over new limits
    if (CacheStats.TotalCacheSizeBytes > Config.MaxCacheSizeBytes)
    {
        TrimCache();
    }
}

void UMGAssetCacheSubsystem::SetMaxCacheSize(int64 MaxSizeBytes)
{
    CacheConfig.MaxCacheSizeBytes = MaxSizeBytes;
    CacheStats.MaxCacheSizeBytes = MaxSizeBytes;

    if (CacheStats.TotalCacheSizeBytes > MaxSizeBytes)
    {
        TrimCache();
    }
}

void UMGAssetCacheSubsystem::SetEvictionPolicy(EMGCacheEvictionPolicy Policy)
{
    CacheConfig.EvictionPolicy = Policy;
}

void UMGAssetCacheSubsystem::SetMaxConcurrentLoads(int32 MaxLoads)
{
    CacheConfig.MaxConcurrentLoads = FMath::Max(1, MaxLoads);
}

void UMGAssetCacheSubsystem::SetCategoryBudget(EMGAssetCategory Category, int64 BudgetBytes)
{
    CacheConfig.CategoryBudgets.Add(Category, BudgetBytes);
}

void UMGAssetCacheSubsystem::EnablePredictiveLoading(bool bEnabled)
{
    CacheConfig.bEnablePredictiveLoading = bEnabled;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PredictionHandle);

        if (bEnabled)
        {
            TWeakObjectPtr<UMGAssetCacheSubsystem> WeakThis(this);
            World->GetTimerManager().SetTimer(
                PredictionHandle,
                [WeakThis]()
                {
                    if (WeakThis.IsValid())
                    {
                        WeakThis->GeneratePredictions();
                    }
                },
                2.0f,
                true
            );
        }
    }
}

void UMGAssetCacheSubsystem::ResetStats()
{
    CacheStats.CacheHits = 0;
    CacheStats.CacheMisses = 0;
    CacheStats.EvictionCount = 0;
    CacheStats.BytesEvicted = 0;
    CacheStats.TotalBytesLoaded = 0;
    CacheStats.AverageLoadTimeSeconds = 0.0f;
    CacheStats.AssetCountByCategory.Empty();
    CacheStats.BytesByCategory.Empty();
}

void UMGAssetCacheSubsystem::RecordAssetAccess(const FSoftObjectPath& AssetPath, const FString& Context)
{
    if (!CacheConfig.bTrackAccessPatterns)
    {
        return;
    }

    FMGAssetAccessPattern& Pattern = AccessPatterns.FindOrAdd(AssetPath);
    Pattern.AssetPath = AssetPath;
    Pattern.TotalAccessCount++;
    Pattern.ContextWhenAccessed = Context;

    // Record what asset was accessed after this one (for prediction)
    static FSoftObjectPath LastAccessedAsset;
    if (!LastAccessedAsset.IsNull() && LastAccessedAsset != AssetPath)
    {
        FMGAssetAccessPattern& LastPattern = AccessPatterns.FindOrAdd(LastAccessedAsset);

        int32 ExistingIndex = LastPattern.FollowedByAssets.Find(AssetPath);
        if (ExistingIndex != INDEX_NONE)
        {
            LastPattern.FollowedByCount[ExistingIndex]++;
        }
        else
        {
            LastPattern.FollowedByAssets.Add(AssetPath);
            LastPattern.FollowedByCount.Add(1);
        }
    }

    LastAccessedAsset = AssetPath;
}

void UMGAssetCacheSubsystem::SetPredictionContext(const FString& Context)
{
    CurrentPredictionContext = Context;
}

void UMGAssetCacheSubsystem::TriggerPredictivePreload()
{
    GeneratePredictions();
}

TArray<FSoftObjectPath> UMGAssetCacheSubsystem::GetPredictedAssets(const FString& Context) const
{
    TArray<FSoftObjectPath> Predicted;

    // Find assets commonly accessed in this context
    for (const auto& Pair : AccessPatterns)
    {
        if (Pair.Value.ContextWhenAccessed == Context)
        {
            // Add assets that commonly follow this one
            for (int32 i = 0; i < Pair.Value.FollowedByAssets.Num(); ++i)
            {
                float Confidence = static_cast<float>(Pair.Value.FollowedByCount[i]) /
                                   static_cast<float>(Pair.Value.TotalAccessCount);

                if (Confidence >= CacheConfig.MinConfidenceForPreload)
                {
                    Predicted.AddUnique(Pair.Value.FollowedByAssets[i]);
                }
            }
        }
    }

    return Predicted;
}

void UMGAssetCacheSubsystem::ClearAccessPatterns()
{
    AccessPatterns.Empty();
}

void UMGAssetCacheSubsystem::PreloadForTrack(const FName& TrackId)
{
    SetPredictionContext(FString::Printf(TEXT("Track_%s"), *TrackId.ToString()));

    // Load track-specific bundle if exists
    FName TrackBundleName = FName(*FString::Printf(TEXT("Track_%s"), *TrackId.ToString()));
    LoadBundleByName(TrackBundleName, EMGPreloadStrategy::Immediate);

    // Also load common race assets
    LoadBundleByName(FName("RaceCommon"), EMGPreloadStrategy::Immediate);
}

void UMGAssetCacheSubsystem::PreloadForVehicle(const FName& VehicleId)
{
    SetPredictionContext(FString::Printf(TEXT("Vehicle_%s"), *VehicleId.ToString()));

    // Load vehicle-specific bundle if exists
    FName VehicleBundleName = FName(*FString::Printf(TEXT("Vehicle_%s"), *VehicleId.ToString()));
    LoadBundleByName(VehicleBundleName, EMGPreloadStrategy::Immediate);
}

void UMGAssetCacheSubsystem::PreloadForMenu(const FName& MenuId)
{
    SetPredictionContext(FString::Printf(TEXT("Menu_%s"), *MenuId.ToString()));

    // Load common UI bundle
    LoadBundleByName(FName("CommonUI"), EMGPreloadStrategy::Immediate);
}

void UMGAssetCacheSubsystem::OnSceneTransitionStart(const FName& TargetScene)
{
    SetPredictionContext(FString::Printf(TEXT("Transition_%s"), *TargetScene.ToString()));

    // Predictively load assets for target scene
    TArray<FSoftObjectPath> PredictedAssets = GetPredictedAssets(TargetScene.ToString());
    if (PredictedAssets.Num() > 0)
    {
        RequestPreload(PredictedAssets, EMGPreloadStrategy::Progressive, EMGCachePriority::High);
    }
}

void UMGAssetCacheSubsystem::OnSceneTransitionComplete(const FName& NewScene)
{
    SetPredictionContext(NewScene.ToString());

    // Consider trimming cache of assets from previous scene
    TrimCache();
}

int64 UMGAssetCacheSubsystem::EstimateAssetSize(const FSoftObjectPath& AssetPath) const
{
    // Estimate based on asset type
    FString PathString = AssetPath.ToString();

    if (PathString.Contains(TEXT("Texture")))
    {
        return 4 * 1024 * 1024; // 4 MB estimate for textures
    }
    else if (PathString.Contains(TEXT("StaticMesh")) || PathString.Contains(TEXT("SkeletalMesh")))
    {
        return 8 * 1024 * 1024; // 8 MB estimate for meshes
    }
    else if (PathString.Contains(TEXT("Material")))
    {
        return 1 * 1024 * 1024; // 1 MB estimate for materials
    }
    else if (PathString.Contains(TEXT("Sound")) || PathString.Contains(TEXT("Audio")))
    {
        return 2 * 1024 * 1024; // 2 MB estimate for audio
    }
    else if (PathString.Contains(TEXT("Blueprint")) || PathString.Contains(TEXT("BP_")))
    {
        return 512 * 1024; // 512 KB estimate for blueprints
    }
    else if (PathString.Contains(TEXT("Animation")) || PathString.Contains(TEXT("Anim")))
    {
        return 3 * 1024 * 1024; // 3 MB estimate for animations
    }

    return 1 * 1024 * 1024; // 1 MB default estimate
}

int64 UMGAssetCacheSubsystem::EstimateBundleSize(const TArray<FSoftObjectPath>& Assets) const
{
    int64 TotalSize = 0;

    for (const FSoftObjectPath& Asset : Assets)
    {
        TotalSize += EstimateAssetSize(Asset);
    }

    return TotalSize;
}

EMGAssetCategory UMGAssetCacheSubsystem::DetermineAssetCategory(const FSoftObjectPath& AssetPath) const
{
    FString PathString = AssetPath.ToString();

    if (PathString.Contains(TEXT("Vehicle")) || PathString.Contains(TEXT("Car")))
    {
        return EMGAssetCategory::Vehicle;
    }
    else if (PathString.Contains(TEXT("Track")) || PathString.Contains(TEXT("Race")))
    {
        return EMGAssetCategory::Track;
    }
    else if (PathString.Contains(TEXT("Environment")) || PathString.Contains(TEXT("Prop")))
    {
        return EMGAssetCategory::Environment;
    }
    else if (PathString.Contains(TEXT("Character")) || PathString.Contains(TEXT("Player")))
    {
        return EMGAssetCategory::Character;
    }
    else if (PathString.Contains(TEXT("UI")) || PathString.Contains(TEXT("Widget")))
    {
        return EMGAssetCategory::UI;
    }
    else if (PathString.Contains(TEXT("Sound")) || PathString.Contains(TEXT("Audio")) || PathString.Contains(TEXT("Music")))
    {
        return EMGAssetCategory::Audio;
    }
    else if (PathString.Contains(TEXT("VFX")) || PathString.Contains(TEXT("Particle")) || PathString.Contains(TEXT("Niagara")))
    {
        return EMGAssetCategory::VFX;
    }
    else if (PathString.Contains(TEXT("Material")) || PathString.Contains(TEXT("M_")))
    {
        return EMGAssetCategory::Material;
    }
    else if (PathString.Contains(TEXT("Texture")) || PathString.Contains(TEXT("T_")))
    {
        return EMGAssetCategory::Texture;
    }
    else if (PathString.Contains(TEXT("Animation")) || PathString.Contains(TEXT("Anim")))
    {
        return EMGAssetCategory::Animation;
    }
    else if (PathString.Contains(TEXT("Blueprint")) || PathString.Contains(TEXT("BP_")))
    {
        return EMGAssetCategory::Blueprint;
    }
    else if (PathString.Contains(TEXT("Data")) || PathString.Contains(TEXT("Table")))
    {
        return EMGAssetCategory::Data;
    }

    return EMGAssetCategory::Miscellaneous;
}

FString UMGAssetCacheSubsystem::GenerateCacheReport() const
{
    FString Report;

    Report += TEXT("=== MIDNIGHT GRIND ASSET CACHE REPORT ===\n\n");
    Report += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());

    Report += TEXT("CACHE OVERVIEW\n");
    Report += TEXT("--------------\n");
    Report += FString::Printf(TEXT("Total Cached: %d assets\n"), CacheStats.TotalCachedAssets);
    Report += FString::Printf(TEXT("Cache Size: %.2f MB / %.2f MB (%.1f%%)\n"),
        CacheStats.TotalCacheSizeBytes / (1024.0f * 1024.0f),
        CacheStats.MaxCacheSizeBytes / (1024.0f * 1024.0f),
        CacheStats.CacheUtilization * 100.0f);
    Report += FString::Printf(TEXT("Hit Rate: %.1f%% (%d hits, %d misses)\n"),
        CacheStats.HitRate * 100.0f,
        CacheStats.CacheHits,
        CacheStats.CacheMisses);
    Report += TEXT("\n");

    Report += TEXT("LOAD STATISTICS\n");
    Report += TEXT("---------------\n");
    Report += FString::Printf(TEXT("Pending Loads: %d\n"), CacheStats.PendingLoads);
    Report += FString::Printf(TEXT("Active Loads: %d\n"), CacheStats.ActiveLoads);
    Report += FString::Printf(TEXT("Avg Load Time: %.3f seconds\n"), CacheStats.AverageLoadTimeSeconds);
    Report += FString::Printf(TEXT("Total Loaded: %.2f MB\n"), CacheStats.TotalBytesLoaded / (1024.0f * 1024.0f));
    Report += TEXT("\n");

    Report += TEXT("EVICTION STATISTICS\n");
    Report += TEXT("-------------------\n");
    Report += FString::Printf(TEXT("Evictions: %d\n"), CacheStats.EvictionCount);
    Report += FString::Printf(TEXT("Bytes Evicted: %.2f MB\n"), CacheStats.BytesEvicted / (1024.0f * 1024.0f));
    Report += FString::Printf(TEXT("Eviction Policy: %s\n"), *UEnum::GetDisplayValueAsText(CacheConfig.EvictionPolicy).ToString());
    Report += TEXT("\n");

    Report += TEXT("BUNDLES\n");
    Report += TEXT("-------\n");
    for (const FMGAssetBundle& Bundle : AssetBundles)
    {
        Report += FString::Printf(TEXT("- %s: %d assets, %.1f%% loaded\n"),
            *Bundle.BundleName.ToString(),
            Bundle.Assets.Num(),
            GetBundleLoadProgress(Bundle.BundleId) * 100.0f);
    }

    return Report;
}

void UMGAssetCacheSubsystem::DumpCacheToLog() const
{
    UE_LOG(LogTemp, Log, TEXT("%s"), *GenerateCacheReport());
}

void UMGAssetCacheSubsystem::ProcessLoadQueue()
{
    if (LoadQueue.Num() == 0)
    {
        return;
    }

    // Process up to MaxConcurrentLoads at a time
    while (CurrentlyLoading.Num() < CacheConfig.MaxConcurrentLoads && LoadQueue.Num() > 0)
    {
        FSoftObjectPath AssetPath = LoadQueue[0];
        LoadQueue.RemoveAt(0);

        if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
        {
            Asset->LoadState = EMGAssetLoadState::Loading;
            CurrentlyLoading.Add(AssetPath);

            // Start async load
            TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
                AssetPath,
                [this, AssetPath]()
                {
                    // Load complete callback
                    CurrentlyLoading.Remove(AssetPath);

                    UObject* LoadedAsset = AssetPath.ResolveObject();
                    if (LoadedAsset)
                    {
                        HandleAssetLoadComplete(AssetPath, LoadedAsset);
                    }
                    else
                    {
                        HandleAssetLoadFailed(AssetPath, TEXT("Failed to resolve asset"));
                    }
                },
                FStreamableManager::AsyncLoadHighPriority
            );

            StreamableHandles.Add(AssetPath, Handle);
        }
    }

    CacheStats.PendingLoads = LoadQueue.Num();
    CacheStats.ActiveLoads = CurrentlyLoading.Num();
}

void UMGAssetCacheSubsystem::PerformCacheMaintenance()
{
    UpdateCacheStats();
    CheckCacheThresholds();

    // Update bundle progress
    for (FMGAssetBundle& Bundle : AssetBundles)
    {
        Bundle.LoadProgress = GetBundleLoadProgress(Bundle.BundleId);

        bool bAllLoaded = true;
        for (const FSoftObjectPath& Path : Bundle.Assets)
        {
            if (!IsAssetCached(Path))
            {
                bAllLoaded = false;
                break;
            }
        }

        if (bAllLoaded && !Bundle.bFullyLoaded && Bundle.Assets.Num() > 0)
        {
            Bundle.bFullyLoaded = true;
            OnBundleLoaded.Broadcast(Bundle.BundleId);
        }
    }

    // Update preload progress
    TArray<FGuid> CompletedPreloads;
    for (auto& Pair : ActivePreloads)
    {
        UpdatePreloadProgress(Pair.Key);

        if (Pair.Value.bCompleted)
        {
            CompletedPreloads.Add(Pair.Key);
        }
    }

    // Clean up completed preloads
    for (const FGuid& Id : CompletedPreloads)
    {
        NotifyPreloadComplete(Id);
        ActivePreloads.Remove(Id);
    }
}

void UMGAssetCacheSubsystem::UpdateCacheStats()
{
    CacheStats.TotalCachedAssets = 0;
    CacheStats.TotalCacheSizeBytes = 0;
    CacheStats.AssetCountByCategory.Empty();
    CacheStats.BytesByCategory.Empty();

    for (const auto& Pair : CachedAssets)
    {
        if (Pair.Value.LoadState == EMGAssetLoadState::Loaded || Pair.Value.LoadState == EMGAssetLoadState::Cached)
        {
            CacheStats.TotalCachedAssets++;
            CacheStats.TotalCacheSizeBytes += Pair.Value.SizeBytes;

            int32& CategoryCount = CacheStats.AssetCountByCategory.FindOrAdd(Pair.Value.Category);
            CategoryCount++;

            int64& CategoryBytes = CacheStats.BytesByCategory.FindOrAdd(Pair.Value.Category);
            CategoryBytes += Pair.Value.SizeBytes;
        }
    }

    CacheStats.CacheUtilization = (CacheStats.MaxCacheSizeBytes > 0)
        ? static_cast<float>(CacheStats.TotalCacheSizeBytes) / static_cast<float>(CacheStats.MaxCacheSizeBytes)
        : 0.0f;

    int32 TotalRequests = CacheStats.CacheHits + CacheStats.CacheMisses;
    CacheStats.HitRate = (TotalRequests > 0)
        ? static_cast<float>(CacheStats.CacheHits) / static_cast<float>(TotalRequests)
        : 0.0f;
}

void UMGAssetCacheSubsystem::CheckCacheThresholds()
{
    if (CacheStats.CacheUtilization >= CacheConfig.AggressiveEvictionThreshold)
    {
        int64 BytesToFree = CacheStats.TotalCacheSizeBytes - CacheConfig.TargetCacheSizeBytes;
        EvictByPolicy(BytesToFree);

        int64 BytesOverBudget = CacheStats.TotalCacheSizeBytes - CacheConfig.MaxCacheSizeBytes;
        OnCacheThresholdReached.Broadcast(CacheStats.CacheUtilization, BytesOverBudget);
    }
    else if (CacheStats.CacheUtilization >= CacheConfig.EvictionThreshold)
    {
        TrimCache();
    }
}

void UMGAssetCacheSubsystem::EvictByPolicy(int64 BytesToFree)
{
    if (BytesToFree <= 0)
    {
        return;
    }

    // Build list of evictable assets
    TArray<TPair<FSoftObjectPath, FMGCachedAsset>> Evictable;
    for (const auto& Pair : CachedAssets)
    {
        if (!Pair.Value.bPersistent &&
            (Pair.Value.LoadState == EMGAssetLoadState::Loaded || Pair.Value.LoadState == EMGAssetLoadState::Cached))
        {
            Evictable.Add(TPair<FSoftObjectPath, FMGCachedAsset>(Pair.Key, Pair.Value));
        }
    }

    // Sort based on eviction policy
    switch (CacheConfig.EvictionPolicy)
    {
    case EMGCacheEvictionPolicy::LRU:
        Evictable.Sort([](const auto& A, const auto& B)
        {
            return A.Value.LastAccessTime < B.Value.LastAccessTime;
        });
        break;

    case EMGCacheEvictionPolicy::LFU:
        Evictable.Sort([](const auto& A, const auto& B)
        {
            return A.Value.AccessCount < B.Value.AccessCount;
        });
        break;

    case EMGCacheEvictionPolicy::FIFO:
        Evictable.Sort([](const auto& A, const auto& B)
        {
            return A.Value.LoadedTime < B.Value.LoadedTime;
        });
        break;

    case EMGCacheEvictionPolicy::Priority:
        Evictable.Sort([](const auto& A, const auto& B)
        {
            return static_cast<int32>(A.Value.Priority) > static_cast<int32>(B.Value.Priority);
        });
        break;

    case EMGCacheEvictionPolicy::Size:
        Evictable.Sort([](const auto& A, const auto& B)
        {
            return A.Value.SizeBytes > B.Value.SizeBytes;
        });
        break;

    case EMGCacheEvictionPolicy::Adaptive:
        // Score based on multiple factors
        Evictable.Sort([](const auto& A, const auto& B)
        {
            float ScoreA = static_cast<float>(A.Value.AccessCount) / FMath::Max(1.0f, static_cast<float>(A.Value.SizeBytes) / (1024.0f * 1024.0f));
            float ScoreB = static_cast<float>(B.Value.AccessCount) / FMath::Max(1.0f, static_cast<float>(B.Value.SizeBytes) / (1024.0f * 1024.0f));
            return ScoreA < ScoreB;
        });
        break;
    }

    // Evict until we've freed enough
    int64 FreedBytes = 0;
    for (const auto& Pair : Evictable)
    {
        if (FreedBytes >= BytesToFree)
        {
            break;
        }

        FreedBytes += Pair.Value.SizeBytes;
        EvictAsset(Pair.Key);
    }
}

void UMGAssetCacheSubsystem::HandleAssetLoadComplete(const FSoftObjectPath& AssetPath, UObject* LoadedAsset)
{
    if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        Asset->LoadState = EMGAssetLoadState::Loaded;
        Asset->CachedObject = LoadedAsset;
        Asset->LoadedTime = FDateTime::Now();
        Asset->LastAccessTime = FDateTime::Now();
        Asset->SizeBytes = CalculateAssetSize(LoadedAsset);

        CacheStats.TotalBytesLoaded += Asset->SizeBytes;

        OnAssetLoaded.Broadcast(AssetPath, LoadedAsset);
    }

    UpdateCacheStats();
}

void UMGAssetCacheSubsystem::HandleAssetLoadFailed(const FSoftObjectPath& AssetPath, const FString& Error)
{
    if (FMGCachedAsset* Asset = CachedAssets.Find(AssetPath))
    {
        Asset->LoadState = EMGAssetLoadState::Failed;
    }

    OnAssetLoadFailed.Broadcast(AssetPath, Error);
}

void UMGAssetCacheSubsystem::UpdatePreloadProgress(const FGuid& RequestId)
{
    if (FMGPreloadRequest* Request = ActivePreloads.Find(RequestId))
    {
        int32 LoadedCount = 0;
        for (const FSoftObjectPath& Path : Request->AssetPaths)
        {
            if (IsAssetCached(Path))
            {
                LoadedCount++;
            }
        }

        Request->LoadedCount = LoadedCount;
        Request->Progress = (Request->TotalCount > 0)
            ? static_cast<float>(LoadedCount) / static_cast<float>(Request->TotalCount)
            : 0.0f;

        Request->bCompleted = (LoadedCount >= Request->TotalCount);

        OnPreloadProgress.Broadcast(RequestId, Request->Progress);
    }
}

void UMGAssetCacheSubsystem::GeneratePredictions()
{
    if (!CacheConfig.bEnablePredictiveLoading || CurrentPredictionContext.IsEmpty())
    {
        return;
    }

    TArray<FSoftObjectPath> PredictedAssets = GetPredictedAssets(CurrentPredictionContext);

    if (PredictedAssets.Num() > 0)
    {
        FMGPreloadPrediction Prediction;
        Prediction.ContextId = CurrentPredictionContext;
        Prediction.PredictedAssets = PredictedAssets;
        Prediction.PredictionTime = FDateTime::Now();
        Prediction.TimeToNeedSeconds = CacheConfig.PredictionLookaheadSeconds;

        // Calculate confidence for each asset
        for (const FSoftObjectPath& Asset : PredictedAssets)
        {
            if (const FMGAssetAccessPattern* Pattern = AccessPatterns.Find(Asset))
            {
                float Confidence = FMath::Min(1.0f, Pattern->TotalAccessCount / 10.0f);
                Prediction.Confidence.Add(Confidence);
            }
            else
            {
                Prediction.Confidence.Add(CacheConfig.MinConfidenceForPreload);
            }
        }

        OnPredictionGenerated.Broadcast(Prediction);

        // Preload predicted assets that aren't already cached
        TArray<FSoftObjectPath> ToPreload;
        for (const FSoftObjectPath& Asset : PredictedAssets)
        {
            if (!IsAssetCached(Asset) && !IsAssetLoading(Asset))
            {
                ToPreload.Add(Asset);
            }
        }

        if (ToPreload.Num() > 0)
        {
            RequestPreload(ToPreload, EMGPreloadStrategy::Lazy, EMGCachePriority::Low);
        }
    }
}

FMGCachedAsset UMGAssetCacheSubsystem::CreateCacheEntry(const FSoftObjectPath& AssetPath, UObject* LoadedAsset)
{
    FMGCachedAsset Entry;
    Entry.AssetId = FGuid::NewGuid();
    Entry.AssetPath = AssetPath;
    Entry.AssetName = FName(*AssetPath.GetAssetName());
    Entry.Category = DetermineAssetCategory(AssetPath);
    Entry.Priority = EMGCachePriority::Normal;
    Entry.LoadState = EMGAssetLoadState::Loaded;
    Entry.SizeBytes = CalculateAssetSize(LoadedAsset);
    Entry.LoadedTime = FDateTime::Now();
    Entry.LastAccessTime = FDateTime::Now();
    Entry.AccessCount = 1;
    Entry.CachedObject = LoadedAsset;

    return Entry;
}

int64 UMGAssetCacheSubsystem::CalculateAssetSize(UObject* Asset) const
{
    if (!Asset)
    {
        return 0;
    }

    // Use resource size if available
    int64 Size = Asset->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);

    if (Size <= 0)
    {
        // Fall back to estimation
        Size = EstimateAssetSize(FSoftObjectPath(Asset));
    }

    return Size;
}

void UMGAssetCacheSubsystem::NotifyPreloadComplete(const FGuid& RequestId)
{
    OnPreloadComplete.Broadcast(RequestId);
}

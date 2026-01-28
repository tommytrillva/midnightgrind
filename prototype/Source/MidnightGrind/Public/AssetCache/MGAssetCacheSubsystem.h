// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGAssetCacheSubsystem.h
 * @brief Intelligent Asset Caching and Preloading System for optimized performance
 *
 * @section Overview
 * This header defines the Asset Cache Subsystem - a system that manages loading
 * and caching game assets (textures, meshes, sounds, etc.) to optimize performance
 * and memory usage.
 *
 * @section Problem Problem it Solves
 *   - Games have LOTS of assets (models, textures, sounds, animations)
 *   - Loading everything at once = long load times + memory explosion
 *   - Loading on-demand = stutters and hitches during gameplay
 *   - SOLUTION: Smart caching + predictive preloading!
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * @subsection WhatIsAsset 1. What is an "Asset"?
 *   - Any file loaded into memory: textures, meshes, sounds, blueprints
 *   - Identified by FSoftObjectPath (path like "/Game/Vehicles/SportsCar.SportsCar")
 *   - "Soft reference" = knows the path but doesn't load until needed
 *
 * @subsection CachingBasics 2. Caching Basics
 *   - Cache = temporary storage for recently/frequently used items
 *   - Hit = asset found in cache (fast!)
 *   - Miss = asset not in cache, must load (slow)
 *   - HitRate = hits / (hits + misses) - higher is better!
 *
 * @subsection PriorityLevels 3. Priority Levels (EMGCachePriority)
 *   - Critical: MUST be in memory (core gameplay)
 *   - High: Very important (current level assets)
 *   - Normal: Standard priority (most things)
 *   - Low: Nice to have but can wait
 *   - Background: Load when nothing else needs CPU
 *
 * @subsection EvictionPolicies 4. Eviction Policies (EMGCacheEvictionPolicy)
 * When cache is full, which assets do we remove?
 *   - LRU (Least Recently Used): Remove oldest-accessed
 *   - LFU (Least Frequently Used): Remove rarely-accessed
 *   - FIFO (First In First Out): Remove oldest-loaded
 *   - Priority: Remove lowest-priority first
 *   - Size: Remove largest assets first
 *   - Adaptive: AI-like combination of above
 *
 * @subsection PreloadStrategies 5. Preload Strategies (EMGPreloadStrategy)
 *   - Immediate: Load RIGHT NOW (blocking)
 *   - OnDemand: Load only when actually needed
 *   - Predictive: Guess what's needed and preload (AI-like)
 *   - Progressive: Load gradually over time
 *   - Lazy: Delay loading as long as possible
 *
 * @subsection LoadStates 6. Asset Load States (EMGAssetLoadState)
 * @code
 *    NotLoaded -> Queued -> Loading -> Loaded -> Cached
 *                                            -> Evicted (removed)
 *                          -> Failed (error)
 * @endcode
 *
 * @subsection Bundles 7. Bundles (FMGAssetBundle)
 *   - Group related assets together
 *   - Example: "VehicleBundle_SportsCar" contains model, textures, sounds
 *   - Load/unload bundles as a unit
 *   - More organized than individual assets
 *
 * @subsection PredictiveLoading 8. Predictive Loading
 *   - System learns what assets are used together
 *   - AccessPatterns track: "After loading X, Y is usually needed"
 *   - TriggerPredictivePreload() uses this data to preload
 *   - Like how Netflix preloads the next episode!
 *
 * @section DataStructures Important Data Structures
 *
 * @subsection CachedAsset FMGCachedAsset - Data about a cached asset
 *   - AssetPath: Where to find it
 *   - SizeBytes: Memory usage
 *   - AccessCount: How often used
 *   - LastAccessTime: When last used
 *   - bPersistent: Never evict this one
 *
 * @subsection CacheStats FMGCacheStats - Performance metrics
 *   - CacheHits/CacheMisses: Did we find it?
 *   - HitRate: Success percentage
 *   - PendingLoads: Queue size
 *   - AverageLoadTimeSeconds: Performance tracking
 *
 * @subsection PreloadRequest FMGPreloadRequest - A batch load request
 *   - AssetPaths: What to load
 *   - Progress: 0.0 to 1.0
 *   - bCompleted: Is it done?
 *
 * @section CodeExamples Code Examples
 *
 * @subsection BasicLoading Basic Loading
 * @code
 * // Get the subsystem
 * UMGAssetCacheSubsystem* AssetCache = GetGameInstance()->GetSubsystem<UMGAssetCacheSubsystem>();
 *
 * // Load a single asset
 * FSoftObjectPath Path("/Game/Textures/CarPaint.CarPaint");
 * AssetCache->LoadAsset(Path, EMGCachePriority::Normal);
 *
 * // Later, retrieve it
 * UObject* Asset = AssetCache->GetCachedAsset(Path);
 * UTexture2D* Texture = Cast<UTexture2D>(Asset);
 * @endcode
 *
 * @subsection BatchLoading Batch Loading
 * @code
 * // Load multiple assets at once
 * TArray<FSoftObjectPath> Paths;
 * Paths.Add(FSoftObjectPath("/Game/Textures/CarPaint.CarPaint"));
 * Paths.Add(FSoftObjectPath("/Game/Meshes/CarBody.CarBody"));
 * Paths.Add(FSoftObjectPath("/Game/Sounds/Engine.Engine"));
 *
 * // Request preload and get tracking ID
 * FGuid RequestId = AssetCache->RequestPreload(Paths, EMGPreloadStrategy::Progressive);
 *
 * // Check progress (e.g., for loading screen)
 * void ALoadingScreen::Tick(float DeltaTime)
 * {
 *     float Progress = AssetCache->GetPreloadProgress(RequestId);
 *     UpdateProgressBar(Progress);
 *
 *     if (AssetCache->IsPreloadComplete(RequestId))
 *     {
 *         StartGame();
 *     }
 * }
 * @endcode
 *
 * @subsection BundleUsage Using Bundles
 * @code
 * // Create a bundle for a vehicle's assets
 * TArray<FSoftObjectPath> CarAssets;
 * CarAssets.Add(FSoftObjectPath("/Game/Vehicles/Skyline/Mesh.Mesh"));
 * CarAssets.Add(FSoftObjectPath("/Game/Vehicles/Skyline/Textures/Body.Body"));
 * CarAssets.Add(FSoftObjectPath("/Game/Vehicles/Skyline/Sounds/Engine.Engine"));
 *
 * FGuid BundleId = AssetCache->CreateBundle(FName("VehicleBundle_Skyline"), CarAssets);
 *
 * // Load the entire bundle when entering garage
 * void AGarage::OnVehicleSelected(FName VehicleId)
 * {
 *     AssetCache->LoadBundleByName(FName("VehicleBundle_" + VehicleId.ToString()));
 * }
 *
 * // Unload when switching vehicles
 * void AGarage::OnVehicleDeselected(FGuid BundleId)
 * {
 *     AssetCache->UnloadBundle(BundleId);
 * }
 * @endcode
 *
 * @subsection SceneTransitions Scene Transitions
 * @code
 * // When starting to load a new track
 * void ARaceManager::StartLoadingTrack(FName TrackId)
 * {
 *     AssetCache->OnSceneTransitionStart(TrackId);
 *     // System preloads predicted assets for that scene
 *     ShowLoadingScreen();
 * }
 *
 * // When loading is complete
 * void ARaceManager::OnTrackLoadComplete()
 * {
 *     AssetCache->OnSceneTransitionComplete(CurrentTrackId);
 *     HideLoadingScreen();
 * }
 * @endcode
 *
 * @subsection PredictiveExample Predictive Loading
 * @code
 * // Record what assets are used in each context
 * void AGarage::OnVehicleViewed(FSoftObjectPath VehiclePath)
 * {
 *     AssetCache->RecordAssetAccess(VehiclePath, "GarageScreen");
 * }
 *
 * // When entering garage, system predicts what's needed
 * void AGarage::OnEnter()
 * {
 *     AssetCache->SetPredictionContext("GarageScreen");
 *     AssetCache->TriggerPredictivePreload();
 *     // Automatically loads assets commonly used in garage!
 * }
 * @endcode
 *
 * @subsection MemoryManagement Memory Management
 * @code
 * // Manual eviction when you know an asset won't be needed
 * AssetCache->EvictAsset(OldAssetPath);
 * AssetCache->EvictAssetsByCategory(EMGAssetCategory::VFX);
 *
 * // Keep important assets that should never be evicted
 * AssetCache->SetAssetPersistent(ImportantPath, true);
 *
 * // Free up memory space
 * AssetCache->TrimCache();  // Evict to target size
 * AssetCache->ClearCache(); // Remove everything (level transition)
 * @endcode
 *
 * @subsection Monitoring Monitoring Cache Performance
 * @code
 * FMGCacheStats Stats = AssetCache->GetCacheStats();
 * UE_LOG(LogGame, Log, TEXT("Hit Rate: %.1f%%"), Stats.HitRate * 100.0f);
 * UE_LOG(LogGame, Log, TEXT("Cache Size: %lld MB"), Stats.TotalCacheSizeBytes / (1024 * 1024));
 * UE_LOG(LogGame, Log, TEXT("Pending Loads: %d"), Stats.PendingLoads);
 *
 * // Generate detailed report for debugging
 * FString Report = AssetCache->GenerateCacheReport();
 * @endcode
 *
 * @section WhyMatters Why This Matters
 *   - No stutters: Assets preloaded before needed
 *   - Fast loads: Frequently-used assets stay cached
 *   - Memory efficient: Unused assets evicted automatically
 *   - Smart: Learns player patterns and predicts needs
 *
 * @author Midnight Grind Team
 */

// MGAssetCacheSubsystem.h
// Midnight Grind - Asset Caching and Preloading System
// Manages intelligent asset preloading, caching strategies, and memory-efficient loading

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "MGAssetCacheSubsystem.generated.h"

// Cache priority levels
UENUM(BlueprintType)
enum class EMGCachePriority : uint8
{
    Critical                UMETA(DisplayName = "Critical"),
    High                    UMETA(DisplayName = "High"),
    Normal                  UMETA(DisplayName = "Normal"),
    Low                     UMETA(DisplayName = "Low"),
    Background              UMETA(DisplayName = "Background")
};

// Asset category types for caching
UENUM(BlueprintType)
enum class EMGAssetCategory : uint8
{
    Vehicle                 UMETA(DisplayName = "Vehicle"),
    Track                   UMETA(DisplayName = "Track"),
    Environment             UMETA(DisplayName = "Environment"),
    Character               UMETA(DisplayName = "Character"),
    UI                      UMETA(DisplayName = "UI"),
    Audio                   UMETA(DisplayName = "Audio"),
    VFX                     UMETA(DisplayName = "VFX"),
    Material                UMETA(DisplayName = "Material"),
    Texture                 UMETA(DisplayName = "Texture"),
    Animation               UMETA(DisplayName = "Animation"),
    Blueprint               UMETA(DisplayName = "Blueprint"),
    Data                    UMETA(DisplayName = "Data"),
    Miscellaneous           UMETA(DisplayName = "Miscellaneous")
};

// Cache eviction policies
UENUM(BlueprintType)
enum class EMGCacheEvictionPolicy : uint8
{
    LRU                     UMETA(DisplayName = "Least Recently Used"),
    LFU                     UMETA(DisplayName = "Least Frequently Used"),
    FIFO                    UMETA(DisplayName = "First In First Out"),
    Priority                UMETA(DisplayName = "Priority Based"),
    Size                    UMETA(DisplayName = "Largest First"),
    Adaptive                UMETA(DisplayName = "Adaptive")
};

// Preload strategy types
UENUM(BlueprintType)
enum class EMGPreloadStrategy : uint8
{
    Immediate               UMETA(DisplayName = "Immediate"),
    OnDemand                UMETA(DisplayName = "On Demand"),
    Predictive              UMETA(DisplayName = "Predictive"),
    Progressive             UMETA(DisplayName = "Progressive"),
    Lazy                    UMETA(DisplayName = "Lazy")
};

// Asset load state
UENUM(BlueprintType)
enum class EMGAssetLoadState : uint8
{
    NotLoaded               UMETA(DisplayName = "Not Loaded"),
    Queued                  UMETA(DisplayName = "Queued"),
    Loading                 UMETA(DisplayName = "Loading"),
    Loaded                  UMETA(DisplayName = "Loaded"),
    Cached                  UMETA(DisplayName = "Cached"),
    Failed                  UMETA(DisplayName = "Failed"),
    Evicted                 UMETA(DisplayName = "Evicted")
};

// Cached asset entry
USTRUCT(BlueprintType)
struct FMGCachedAsset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid AssetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSoftObjectPath AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGAssetCategory Category = EMGAssetCategory::Miscellaneous;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCachePriority Priority = EMGCachePriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGAssetLoadState LoadState = EMGAssetLoadState::NotLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 SizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 CompressedSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LoadedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastAccessTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AccessCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LoadTimeSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPersistent = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompressed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGuid> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;

    UPROPERTY()
    TWeakObjectPtr<UObject> CachedObject;
};

// Asset bundle for grouped loading
USTRUCT(BlueprintType)
struct FMGAssetBundle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid BundleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BundleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSoftObjectPath> Assets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCachePriority Priority = EMGCachePriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPreloadStrategy Strategy = EMGPreloadStrategy::Immediate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LoadProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFullyLoaded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;
};

// Preload request
USTRUCT(BlueprintType)
struct FMGPreloadRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RequestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSoftObjectPath> AssetPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCachePriority Priority = EMGCachePriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPreloadStrategy Strategy = EMGPreloadStrategy::Immediate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime RequestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Progress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LoadedCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCancelled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CallbackContext;
};

// Cache statistics
USTRUCT(BlueprintType)
struct FMGCacheStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalCachedAssets = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalCacheSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MaxCacheSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CacheUtilization = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CacheHits = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CacheMisses = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HitRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EvictionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 BytesEvicted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PendingLoads = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveLoads = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLoadTimeSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalBytesLoaded = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGAssetCategory, int32> AssetCountByCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGAssetCategory, int64> BytesByCategory;
};

// Preload prediction data
USTRUCT(BlueprintType)
struct FMGPreloadPrediction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ContextId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSoftObjectPath> PredictedAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> Confidence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeToNeedSeconds = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime PredictionTime;
};

// Cache configuration
USTRUCT(BlueprintType)
struct FMGCacheConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MaxCacheSizeBytes = 1024 * 1024 * 1024; // 1 GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TargetCacheSizeBytes = 768 * 1024 * 1024; // 768 MB

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCacheEvictionPolicy EvictionPolicy = EMGCacheEvictionPolicy::LRU;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxConcurrentLoads = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EvictionThreshold = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressiveEvictionThreshold = 0.95f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnablePredictiveLoading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableCompression = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredictionLookaheadSeconds = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinConfidenceForPreload = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPreferStreamedAssets = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTrackAccessPatterns = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGAssetCategory, int64> CategoryBudgets;
};

// Asset access pattern for prediction
USTRUCT(BlueprintType)
struct FMGAssetAccessPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSoftObjectPath AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSoftObjectPath> FollowedByAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> FollowedByCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ContextWhenAccessed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalAccessCount = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAssetLoaded, const FSoftObjectPath&, AssetPath, UObject*, LoadedAsset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAssetLoadFailed, const FSoftObjectPath&, AssetPath, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPreloadProgress, const FGuid&, RequestId, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPreloadComplete, const FGuid&, RequestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBundleLoaded, const FGuid&, BundleId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAssetEvicted, const FSoftObjectPath&, AssetPath, int64, BytesFreed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCacheThresholdReached, float, CurrentUtilization, int64, BytesOverBudget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPredictionGenerated, const FMGPreloadPrediction&, Prediction);

/**
 * UMGAssetCacheSubsystem
 * Intelligent asset caching and preloading system
 * Manages memory-efficient asset loading with predictive capabilities
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAssetCacheSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Asset loading
    UFUNCTION(BlueprintCallable, Category = "AssetCache|Loading")
    void LoadAsset(const FSoftObjectPath& AssetPath, EMGCachePriority Priority = EMGCachePriority::Normal);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Loading")
    void LoadAssets(const TArray<FSoftObjectPath>& AssetPaths, EMGCachePriority Priority = EMGCachePriority::Normal);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Loading")
    FGuid RequestPreload(const TArray<FSoftObjectPath>& AssetPaths, EMGPreloadStrategy Strategy = EMGPreloadStrategy::Immediate, EMGCachePriority Priority = EMGCachePriority::Normal);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Loading")
    void CancelPreload(const FGuid& RequestId);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Loading")
    void CancelAllPreloads();

    UFUNCTION(BlueprintPure, Category = "AssetCache|Loading")
    float GetPreloadProgress(const FGuid& RequestId) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Loading")
    bool IsPreloadComplete(const FGuid& RequestId) const;

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Loading")
    void SetLoadPriority(const FSoftObjectPath& AssetPath, EMGCachePriority NewPriority);

    // Asset retrieval
    UFUNCTION(BlueprintCallable, Category = "AssetCache|Retrieval")
    UObject* GetCachedAsset(const FSoftObjectPath& AssetPath);

    UFUNCTION(BlueprintPure, Category = "AssetCache|Retrieval")
    bool IsAssetCached(const FSoftObjectPath& AssetPath) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Retrieval")
    bool IsAssetLoading(const FSoftObjectPath& AssetPath) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Retrieval")
    EMGAssetLoadState GetAssetLoadState(const FSoftObjectPath& AssetPath) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Retrieval")
    TArray<FSoftObjectPath> GetCachedAssetPaths() const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Retrieval")
    TArray<FSoftObjectPath> GetLoadingAssetPaths() const;

    // Bundle management
    UFUNCTION(BlueprintCallable, Category = "AssetCache|Bundles")
    FGuid CreateBundle(const FName& BundleName, const TArray<FSoftObjectPath>& Assets);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Bundles")
    void LoadBundle(const FGuid& BundleId, EMGPreloadStrategy Strategy = EMGPreloadStrategy::Immediate);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Bundles")
    void LoadBundleByName(const FName& BundleName, EMGPreloadStrategy Strategy = EMGPreloadStrategy::Immediate);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Bundles")
    void UnloadBundle(const FGuid& BundleId);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Bundles")
    void AddAssetToBundle(const FGuid& BundleId, const FSoftObjectPath& AssetPath);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Bundles")
    void RemoveAssetFromBundle(const FGuid& BundleId, const FSoftObjectPath& AssetPath);

    UFUNCTION(BlueprintPure, Category = "AssetCache|Bundles")
    bool IsBundleLoaded(const FGuid& BundleId) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Bundles")
    float GetBundleLoadProgress(const FGuid& BundleId) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Bundles")
    TArray<FMGAssetBundle> GetAllBundles() const { return AssetBundles; }

    UFUNCTION(BlueprintPure, Category = "AssetCache|Bundles")
    FMGAssetBundle GetBundleById(const FGuid& BundleId) const;

    // Cache management
    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void EvictAsset(const FSoftObjectPath& AssetPath);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void EvictAssetsByCategory(EMGAssetCategory Category);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void EvictAssetsByTag(const FString& Tag);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void EvictOldestAssets(int64 BytesToFree);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void ClearCache();

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void TrimCache();

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void SetAssetPersistent(const FSoftObjectPath& AssetPath, bool bPersistent);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void TagAsset(const FSoftObjectPath& AssetPath, const FString& Tag);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Cache")
    void UntagAsset(const FSoftObjectPath& AssetPath, const FString& Tag);

    UFUNCTION(BlueprintPure, Category = "AssetCache|Cache")
    TArray<FSoftObjectPath> GetAssetsByTag(const FString& Tag) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Cache")
    TArray<FSoftObjectPath> GetAssetsByCategory(EMGAssetCategory Category) const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "AssetCache|Config")
    void ApplyCacheConfig(const FMGCacheConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Config")
    void SetMaxCacheSize(int64 MaxSizeBytes);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Config")
    void SetEvictionPolicy(EMGCacheEvictionPolicy Policy);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Config")
    void SetMaxConcurrentLoads(int32 MaxLoads);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Config")
    void SetCategoryBudget(EMGAssetCategory Category, int64 BudgetBytes);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Config")
    void EnablePredictiveLoading(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "AssetCache|Config")
    FMGCacheConfig GetCacheConfig() const { return CacheConfig; }

    // Statistics
    UFUNCTION(BlueprintPure, Category = "AssetCache|Stats")
    FMGCacheStats GetCacheStats() const { return CacheStats; }

    UFUNCTION(BlueprintPure, Category = "AssetCache|Stats")
    int64 GetCurrentCacheSize() const { return CacheStats.TotalCacheSizeBytes; }

    UFUNCTION(BlueprintPure, Category = "AssetCache|Stats")
    float GetCacheUtilization() const { return CacheStats.CacheUtilization; }

    UFUNCTION(BlueprintPure, Category = "AssetCache|Stats")
    float GetHitRate() const { return CacheStats.HitRate; }

    UFUNCTION(BlueprintPure, Category = "AssetCache|Stats")
    int32 GetPendingLoadCount() const { return CacheStats.PendingLoads; }

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Stats")
    void ResetStats();

    // Predictive loading
    UFUNCTION(BlueprintCallable, Category = "AssetCache|Prediction")
    void RecordAssetAccess(const FSoftObjectPath& AssetPath, const FString& Context);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Prediction")
    void SetPredictionContext(const FString& Context);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Prediction")
    void TriggerPredictivePreload();

    UFUNCTION(BlueprintPure, Category = "AssetCache|Prediction")
    TArray<FSoftObjectPath> GetPredictedAssets(const FString& Context) const;

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Prediction")
    void ClearAccessPatterns();

    // Scene/level specific
    UFUNCTION(BlueprintCallable, Category = "AssetCache|Scene")
    void PreloadForTrack(const FName& TrackId);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Scene")
    void PreloadForVehicle(const FName& VehicleId);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Scene")
    void PreloadForMenu(const FName& MenuId);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Scene")
    void OnSceneTransitionStart(const FName& TargetScene);

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Scene")
    void OnSceneTransitionComplete(const FName& NewScene);

    // Utility
    UFUNCTION(BlueprintPure, Category = "AssetCache|Utility")
    int64 EstimateAssetSize(const FSoftObjectPath& AssetPath) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Utility")
    int64 EstimateBundleSize(const TArray<FSoftObjectPath>& Assets) const;

    UFUNCTION(BlueprintPure, Category = "AssetCache|Utility")
    EMGAssetCategory DetermineAssetCategory(const FSoftObjectPath& AssetPath) const;

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Utility")
    FString GenerateCacheReport() const;

    UFUNCTION(BlueprintCallable, Category = "AssetCache|Utility")
    void DumpCacheToLog() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "AssetCache|Events")
    FMGOnAssetLoaded OnAssetLoaded;

    UPROPERTY(BlueprintAssignable, Category = "AssetCache|Events")
    FMGOnAssetLoadFailed OnAssetLoadFailed;

    UPROPERTY(BlueprintAssignable, Category = "AssetCache|Events")
    FMGOnPreloadProgress OnPreloadProgress;

    UPROPERTY(BlueprintAssignable, Category = "AssetCache|Events")
    FMGOnPreloadComplete OnPreloadComplete;

    UPROPERTY(BlueprintAssignable, Category = "AssetCache|Events")
    FMGOnBundleLoaded OnBundleLoaded;

    UPROPERTY(BlueprintAssignable, Category = "AssetCache|Events")
    FMGOnAssetEvicted OnAssetEvicted;

    UPROPERTY(BlueprintAssignable, Category = "AssetCache|Events")
    FMGOnCacheThresholdReached OnCacheThresholdReached;

    UPROPERTY(BlueprintAssignable, Category = "AssetCache|Events")
    FMGOnPredictionGenerated OnPredictionGenerated;

protected:
    UPROPERTY()
    TMap<FSoftObjectPath, FMGCachedAsset> CachedAssets;

    UPROPERTY()
    TArray<FMGAssetBundle> AssetBundles;

    UPROPERTY()
    TMap<FGuid, FMGPreloadRequest> ActivePreloads;

    UPROPERTY()
    FMGCacheConfig CacheConfig;

    UPROPERTY()
    FMGCacheStats CacheStats;

    UPROPERTY()
    TMap<FSoftObjectPath, FMGAssetAccessPattern> AccessPatterns;

    UPROPERTY()
    FString CurrentPredictionContext;

    UPROPERTY()
    TArray<FSoftObjectPath> LoadQueue;

    UPROPERTY()
    TArray<FSoftObjectPath> CurrentlyLoading;

    FStreamableManager StreamableManager;
    TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> StreamableHandles;

    FTimerHandle CacheMaintenanceHandle;
    FTimerHandle PredictionHandle;
    FTimerHandle LoadQueueHandle;

    void ProcessLoadQueue();
    void PerformCacheMaintenance();
    void UpdateCacheStats();
    void CheckCacheThresholds();
    void EvictByPolicy(int64 BytesToFree);
    void HandleAssetLoadComplete(const FSoftObjectPath& AssetPath, UObject* LoadedAsset);
    void HandleAssetLoadFailed(const FSoftObjectPath& AssetPath, const FString& Error);
    void UpdatePreloadProgress(const FGuid& RequestId);
    void GeneratePredictions();
    FMGCachedAsset CreateCacheEntry(const FSoftObjectPath& AssetPath, UObject* LoadedAsset);
    int64 CalculateAssetSize(UObject* Asset) const;
    void NotifyPreloadComplete(const FGuid& RequestId);
    void InitializeDefaultBundles();
};

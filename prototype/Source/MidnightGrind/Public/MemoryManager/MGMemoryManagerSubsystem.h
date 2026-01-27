// Copyright Midnight Grind. All Rights Reserved.

// MGMemoryManagerSubsystem.h
// Memory Management System - Budgets, streaming, loading optimization
// Midnight Grind - Y2K Arcade Street Racing

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMemoryManagerSubsystem.generated.h"

// Forward declarations
class UWorld;

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EMemoryPool : uint8
{
    Textures,
    Meshes,
    Audio,
    Physics,
    Animation,
    Particles,
    UI,
    Scripts,
    General,
    Streaming
};

UENUM(BlueprintType)
enum class EAssetPriority : uint8
{
    Critical,
    High,
    Medium,
    Low,
    Background
};

UENUM(BlueprintType)
enum class EStreamingState : uint8
{
    NotLoaded,
    Queued,
    Loading,
    Loaded,
    Unloading,
    Error
};

UENUM(BlueprintType)
enum class EMemoryPressureLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

UENUM(BlueprintType)
enum class EUnloadStrategy : uint8
{
    LeastRecentlyUsed,
    LeastFrequentlyUsed,
    DistanceBased,
    PriorityBased,
    Hybrid
};

UENUM(BlueprintType)
enum class ELoadingStrategy : uint8
{
    Immediate,
    Async,
    Predictive,
    OnDemand
};

// ============================================================================
// Structs
// ============================================================================

USTRUCT(BlueprintType)
struct FMemoryBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMemoryPool Pool = EMemoryPool::General;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 BudgetBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 CurrentUsageBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 PeakUsageBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UsagePercent = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WarningThreshold = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CriticalThreshold = 0.95f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOverBudget = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AssetCount = 0;
};

USTRUCT(BlueprintType)
struct FStreamingAsset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid AssetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMemoryPool Pool = EMemoryPool::General;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAssetPriority Priority = EAssetPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStreamingState State = EStreamingState::NotLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 SizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 LoadedSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ReferenceCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastUsedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UsageCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LoadTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StreamingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsResident = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNeverUnload = false;
};

USTRUCT(BlueprintType)
struct FAssetLoadRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RequestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAssetPriority Priority = EAssetPriority::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ELoadingStrategy Strategy = ELoadingStrategy::Async;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWaitForComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeoutSeconds = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime RequestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequesterTag;
};

USTRUCT(BlueprintType)
struct FStreamingLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LevelPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStreamingState State = EStreamingState::NotLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 EstimatedSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StreamingOrigin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StreamingRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShouldBeLoaded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LoadProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGuid> ContainedAssets;
};

USTRUCT(BlueprintType)
struct FMemorySnapshot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CapturedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalAllocated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalUsed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalAvailable = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMemoryPool, FMemoryBudget> PoolBudgets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalLoadedAssets = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StreamingQueueSize = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StreamingBandwidth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMemoryPressureLevel PressureLevel = EMemoryPressureLevel::None;
};

USTRUCT(BlueprintType)
struct FStreamingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EUnloadStrategy UnloadStrategy = EUnloadStrategy::Hybrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ELoadingStrategy DefaultLoadStrategy = ELoadingStrategy::Async;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxStreamingBandwidthMBps = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxConcurrentLoads = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxQueueSize = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PredictiveLoadDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float UnloadDelaySeconds = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AssetRetentionTimeSeconds = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAggressiveUnloading = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPredictiveLoading = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryPressureThreshold = 0.85f;
};

USTRUCT(BlueprintType)
struct FGarbageCollectionStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalCollections = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalCollectionTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageCollectionTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxCollectionTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalBytesFreed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ObjectsCollected = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastCollectionTime;
};

USTRUCT(BlueprintType)
struct FMemoryManagerStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalAssetLoads = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalAssetUnloads = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LoadFailures = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLoadTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalBytesLoaded = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalBytesUnloaded = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MemoryPressureEvents = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BudgetViolations = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGarbageCollectionStats GCStats;
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssetLoaded, const FStreamingAsset&, Asset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssetUnloaded, const FStreamingAsset&, Asset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAssetLoadFailed, const FString&, AssetPath, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryPressureChanged, EMemoryPressureLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBudgetViolation, EMemoryPool, Pool, float, UsagePercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreamingLevelLoaded, const FStreamingLevel&, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGarbageCollectionComplete, float, CollectionTimeMs);

// ============================================================================
// Main Subsystem
// ============================================================================

UCLASS()
class MIDNIGHTGRIND_API UMGMemoryManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // Memory Budgets
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Memory|Budgets")
    void SetPoolBudget(EMemoryPool Pool, int64 BudgetBytes);

    UFUNCTION(BlueprintPure, Category = "Memory|Budgets")
    FMemoryBudget GetPoolBudget(EMemoryPool Pool) const;

    UFUNCTION(BlueprintPure, Category = "Memory|Budgets")
    TMap<EMemoryPool, FMemoryBudget> GetAllBudgets() const { return PoolBudgets; }

    UFUNCTION(BlueprintPure, Category = "Memory|Budgets")
    int64 GetTotalBudget() const;

    UFUNCTION(BlueprintPure, Category = "Memory|Budgets")
    int64 GetTotalUsage() const;

    UFUNCTION(BlueprintPure, Category = "Memory|Budgets")
    float GetOverallUsagePercent() const;

    UFUNCTION(BlueprintCallable, Category = "Memory|Budgets")
    void SetBudgetThresholds(EMemoryPool Pool, float Warning, float Critical);

    UFUNCTION(BlueprintPure, Category = "Memory|Budgets")
    bool IsPoolOverBudget(EMemoryPool Pool) const;

    UFUNCTION(BlueprintPure, Category = "Memory|Budgets")
    bool IsAnyPoolOverBudget() const;

    // ========================================================================
    // Asset Streaming
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    FGuid RequestAssetLoad(const FString& AssetPath, EAssetPriority Priority, const FString& RequesterTag);

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void RequestAssetUnload(const FGuid& AssetId);

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void CancelLoadRequest(const FGuid& RequestId);

    UFUNCTION(BlueprintPure, Category = "Memory|Streaming")
    EStreamingState GetAssetState(const FGuid& AssetId) const;

    UFUNCTION(BlueprintPure, Category = "Memory|Streaming")
    FStreamingAsset GetAssetInfo(const FGuid& AssetId) const;

    UFUNCTION(BlueprintPure, Category = "Memory|Streaming")
    TArray<FStreamingAsset> GetLoadedAssets() const;

    UFUNCTION(BlueprintPure, Category = "Memory|Streaming")
    TArray<FAssetLoadRequest> GetPendingRequests() const { return PendingRequests; }

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void SetAssetPriority(const FGuid& AssetId, EAssetPriority Priority);

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void MarkAssetUsed(const FGuid& AssetId);

    UFUNCTION(BlueprintCallable, Category = "Memory|Streaming")
    void SetAssetNeverUnload(const FGuid& AssetId, bool bNeverUnload);

    // ========================================================================
    // Level Streaming
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Memory|Levels")
    void RegisterStreamingLevel(const FStreamingLevel& Level);

    UFUNCTION(BlueprintCallable, Category = "Memory|Levels")
    void UnregisterStreamingLevel(const FString& LevelName);

    UFUNCTION(BlueprintCallable, Category = "Memory|Levels")
    void RequestLevelLoad(const FString& LevelName);

    UFUNCTION(BlueprintCallable, Category = "Memory|Levels")
    void RequestLevelUnload(const FString& LevelName);

    UFUNCTION(BlueprintPure, Category = "Memory|Levels")
    TArray<FStreamingLevel> GetStreamingLevels() const { return StreamingLevels; }

    UFUNCTION(BlueprintCallable, Category = "Memory|Levels")
    void UpdateStreamingLevels(const FVector& ViewerLocation);

    // ========================================================================
    // Memory Pressure
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Memory|Pressure")
    EMemoryPressureLevel GetCurrentPressureLevel() const { return CurrentPressureLevel; }

    UFUNCTION(BlueprintCallable, Category = "Memory|Pressure")
    void HandleMemoryPressure(EMemoryPressureLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Memory|Pressure")
    void ForceMemoryCleanup();

    UFUNCTION(BlueprintCallable, Category = "Memory|Pressure")
    int64 FreeMemory(int64 TargetBytes);

    UFUNCTION(BlueprintCallable, Category = "Memory|Pressure")
    void TrimMemoryPools();

    // ========================================================================
    // Garbage Collection
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Memory|GC")
    void RequestGarbageCollection(bool bFullPurge);

    UFUNCTION(BlueprintCallable, Category = "Memory|GC")
    void ScheduleIncrementalGC(float TimeSliceMs);

    UFUNCTION(BlueprintPure, Category = "Memory|GC")
    FGarbageCollectionStats GetGCStats() const { return Stats.GCStats; }

    UFUNCTION(BlueprintCallable, Category = "Memory|GC")
    void SetGCFrequency(float IntervalSeconds);

    // ========================================================================
    // Predictive Loading
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Memory|Predictive")
    void UpdatePlayerPosition(const FVector& Position, const FVector& Velocity);

    UFUNCTION(BlueprintCallable, Category = "Memory|Predictive")
    void PreloadAreaAssets(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Memory|Predictive")
    void SetPredictiveLoadDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Memory|Predictive")
    void AddPredictiveLoadPath(const TArray<FVector>& PathPoints);

    // ========================================================================
    // Snapshots
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Memory|Snapshot")
    FMemorySnapshot TakeMemorySnapshot();

    UFUNCTION(BlueprintPure, Category = "Memory|Snapshot")
    TArray<FMemorySnapshot> GetSnapshotHistory() const { return SnapshotHistory; }

    UFUNCTION(BlueprintCallable, Category = "Memory|Snapshot")
    void ClearSnapshotHistory();

    // ========================================================================
    // Settings
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Memory|Settings")
    FStreamingSettings GetStreamingSettings() const { return StreamingSettings; }

    UFUNCTION(BlueprintCallable, Category = "Memory|Settings")
    void UpdateStreamingSettings(const FStreamingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Memory|Settings")
    void SetMaxConcurrentLoads(int32 MaxLoads);

    UFUNCTION(BlueprintCallable, Category = "Memory|Settings")
    void SetStreamingBandwidth(float MBps);

    // ========================================================================
    // Statistics
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Memory|Stats")
    FMemoryManagerStats GetStats() const { return Stats; }

    UFUNCTION(BlueprintCallable, Category = "Memory|Stats")
    void ResetStats();

    UFUNCTION(BlueprintCallable, Category = "Memory|Stats")
    FString GenerateMemoryReport() const;

    // ========================================================================
    // Events
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnAssetLoaded OnAssetLoaded;

    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnAssetUnloaded OnAssetUnloaded;

    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnAssetLoadFailed OnAssetLoadFailed;

    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnMemoryPressureChanged OnMemoryPressureChanged;

    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnBudgetViolation OnBudgetViolation;

    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnStreamingLevelLoaded OnStreamingLevelLoaded;

    UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
    FOnGarbageCollectionComplete OnGarbageCollectionComplete;

protected:
    void InitializeDefaultBudgets();
    void ProcessLoadQueue();
    void ProcessUnloadQueue();
    void UpdateMemoryPressure();
    void CheckBudgetViolations();
    TArray<FGuid> SelectAssetsToUnload(int64 TargetBytes);
    float CalculateAssetUnloadScore(const FStreamingAsset& Asset);

private:
    UPROPERTY()
    TMap<EMemoryPool, FMemoryBudget> PoolBudgets;

    UPROPERTY()
    TMap<FGuid, FStreamingAsset> TrackedAssets;

    UPROPERTY()
    TArray<FAssetLoadRequest> PendingRequests;

    UPROPERTY()
    TArray<FGuid> UnloadQueue;

    UPROPERTY()
    TArray<FStreamingLevel> StreamingLevels;

    UPROPERTY()
    TArray<FMemorySnapshot> SnapshotHistory;

    UPROPERTY()
    FStreamingSettings StreamingSettings;

    UPROPERTY()
    FMemoryManagerStats Stats;

    UPROPERTY()
    EMemoryPressureLevel CurrentPressureLevel;

    UPROPERTY()
    FVector LastPlayerPosition;

    UPROPERTY()
    FVector LastPlayerVelocity;

    UPROPERTY()
    int32 CurrentConcurrentLoads;

    UPROPERTY()
    float GameTimeSeconds;

    FTimerHandle LoadQueueTimer;
    FTimerHandle MemoryCheckTimer;
    FTimerHandle GCTimer;
};

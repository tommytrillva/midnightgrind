// MGPerformanceMetricsSubsystem.h
// Performance Metrics System - Runtime monitoring, optimization hints, profiling
// Midnight Grind - Y2K Arcade Street Racing

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPerformanceMetricsSubsystem.generated.h"

// Forward declarations
class UWorld;

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EPerformanceTier : uint8
{
    Low,
    Medium,
    High,
    Ultra,
    Custom
};

UENUM(BlueprintType)
enum class EPerformanceMetricType : uint8
{
    FrameRate,
    FrameTime,
    GPUTime,
    CPUTime,
    DrawCalls,
    TriangleCount,
    MemoryUsage,
    VRAMUsage,
    DiskIO,
    NetworkLatency,
    ThreadUtilization,
    GarbageCollection
};

UENUM(BlueprintType)
enum class EPerformanceWarningLevel : uint8
{
    None,
    Info,
    Warning,
    Critical
};

UENUM(BlueprintType)
enum class EOptimizationCategory : uint8
{
    Rendering,
    Physics,
    Audio,
    AI,
    Networking,
    Memory,
    Loading,
    General
};

UENUM(BlueprintType)
enum class EBenchmarkType : uint8
{
    Quick,
    Standard,
    Extended,
    Stress,
    Custom
};

UENUM(BlueprintType)
enum class EFramePacingMode : uint8
{
    Uncapped,
    VSync,
    Fixed30,
    Fixed60,
    Fixed120,
    Variable,
    Custom
};

// ============================================================================
// Structs
// ============================================================================

USTRUCT(BlueprintType)
struct FFrameMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FrameTimeMs = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GPUTimeMs = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CPUTimeMs = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GameThreadMs = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RenderThreadMs = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RHIThreadMs = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DrawCalls = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TrianglesDrawn = 1000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PrimitivesDrawn = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;
};

USTRUCT(BlueprintType)
struct FMemoryMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalPhysicalMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 UsedPhysicalMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 AvailablePhysicalMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalVRAM = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 UsedVRAM = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 AvailableVRAM = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TextureMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MeshMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 AudioMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 PhysicsMemory = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryPressure = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GCCollectionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastGCDurationMs = 0.0f;
};

USTRUCT(BlueprintType)
struct FPerformanceSnapshot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SnapshotId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CapturedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FFrameMetrics FrameMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMemoryMetrics MemoryMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SceneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveVehicles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveParticles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveLights = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LoadedTextures = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LoadedMeshes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NetworkLatencyMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> CustomMetrics;
};

USTRUCT(BlueprintType)
struct FPerformanceHistory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> FPSHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> FrameTimeHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> GPUTimeHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> CPUTimeHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> MemoryUsageHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OnePercentLowFPS = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PointOnePercentLowFPS = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FrameDropCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StutterCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalRecordingTimeSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FPerformanceWarning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid WarningId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPerformanceWarningLevel Level = EPerformanceWarningLevel::Warning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPerformanceMetricType MetricType = EPerformanceMetricType::FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Suggestion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThresholdValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OccurrenceCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bResolved = false;
};

USTRUCT(BlueprintType)
struct FOptimizationHint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid HintId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOptimizationCategory Category = EOptimizationCategory::General;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ActionCommand;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExpectedImprovement = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoApplicable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bApplied = false;
};

USTRUCT(BlueprintType)
struct FBenchmarkResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid BenchmarkId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBenchmarkType Type = EBenchmarkType::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageFPS = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinFPS = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFPS = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OnePercentLow = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageGPUTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageCPUTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 PeakMemoryUsage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 PeakVRAMUsage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalFrames = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DroppedFrames = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Score = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPerformanceTier RecommendedTier = EPerformanceTier::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> SceneScores;
};

USTRUCT(BlueprintType)
struct FPerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinAcceptableFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFrameTimeMs = 33.33f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxGPUTimeMs = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxCPUTimeMs = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxDrawCalls = 3000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MaxTriangles = 5000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryWarningThreshold = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryCriticalThreshold = 0.95f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VRAMWarningThreshold = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StutterThresholdMs = 50.0f;
};

USTRUCT(BlueprintType)
struct FPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableMetrics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableWarnings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableOptimizationHints = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoOptimize = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SampleIntervalSeconds = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HistorySampleCount = 600;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLogToFile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LogFilePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFramePacingMode FramePacing = EFramePacingMode::Fixed60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CustomFrameRateCap = 60;
};

USTRUCT(BlueprintType)
struct FHardwareInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CPUBrand;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CPUCores = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CPUThreads = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GPUBrand;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 GPUMemoryMB = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 SystemMemoryMB = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OSVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RHIName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DriverVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsRayTracing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsNanite = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsLumen = false;
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceWarning, const FPerformanceWarning&, Warning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptimizationHintGenerated, const FOptimizationHint&, Hint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBenchmarkComplete, const FBenchmarkResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFrameRateChanged, float, OldFPS, float, NewFPS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceTierChanged, EPerformanceTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryPressure, float, PressureLevel);

// ============================================================================
// Main Subsystem
// ============================================================================

UCLASS()
class MIDNIGHTGRIND_API UMGPerformanceMetricsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // Real-Time Metrics
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Performance|Metrics")
    FFrameMetrics GetCurrentFrameMetrics() const { return CurrentFrameMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance|Metrics")
    FMemoryMetrics GetCurrentMemoryMetrics() const { return CurrentMemoryMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance|Metrics")
    float GetCurrentFPS() const { return CurrentFrameMetrics.FPS; }

    UFUNCTION(BlueprintPure, Category = "Performance|Metrics")
    float GetCurrentFrameTime() const { return CurrentFrameMetrics.FrameTimeMs; }

    UFUNCTION(BlueprintPure, Category = "Performance|Metrics")
    float GetGPUTime() const { return CurrentFrameMetrics.GPUTimeMs; }

    UFUNCTION(BlueprintPure, Category = "Performance|Metrics")
    float GetCPUTime() const { return CurrentFrameMetrics.CPUTimeMs; }

    UFUNCTION(BlueprintPure, Category = "Performance|Metrics")
    float GetMemoryUsagePercent() const;

    UFUNCTION(BlueprintPure, Category = "Performance|Metrics")
    float GetVRAMUsagePercent() const;

    // ========================================================================
    // History
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Performance|History")
    FPerformanceHistory GetPerformanceHistory() const { return History; }

    UFUNCTION(BlueprintCallable, Category = "Performance|History")
    void ClearHistory();

    UFUNCTION(BlueprintCallable, Category = "Performance|History")
    void StartRecording();

    UFUNCTION(BlueprintCallable, Category = "Performance|History")
    void StopRecording();

    UFUNCTION(BlueprintPure, Category = "Performance|History")
    bool IsRecording() const { return bIsRecording; }

    UFUNCTION(BlueprintCallable, Category = "Performance|History")
    FPerformanceSnapshot TakeSnapshot();

    UFUNCTION(BlueprintPure, Category = "Performance|History")
    TArray<FPerformanceSnapshot> GetSnapshots() const { return Snapshots; }

    // ========================================================================
    // Warnings
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Performance|Warnings")
    TArray<FPerformanceWarning> GetActiveWarnings() const { return ActiveWarnings; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Warnings")
    void ClearWarnings();

    UFUNCTION(BlueprintCallable, Category = "Performance|Warnings")
    void DismissWarning(const FGuid& WarningId);

    UFUNCTION(BlueprintPure, Category = "Performance|Warnings")
    bool HasCriticalWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Warnings")
    void SetWarningsEnabled(bool bEnabled);

    // ========================================================================
    // Optimization
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Performance|Optimization")
    TArray<FOptimizationHint> GetOptimizationHints() const { return OptimizationHints; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    void GenerateOptimizationHints();

    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    bool ApplyOptimizationHint(const FGuid& HintId);

    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    void ApplyAllAutoHints();

    UFUNCTION(BlueprintCallable, Category = "Performance|Optimization")
    void RevertOptimization(const FGuid& HintId);

    // ========================================================================
    // Benchmarking
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Performance|Benchmark")
    void StartBenchmark(EBenchmarkType Type);

    UFUNCTION(BlueprintCallable, Category = "Performance|Benchmark")
    void StopBenchmark();

    UFUNCTION(BlueprintPure, Category = "Performance|Benchmark")
    bool IsBenchmarking() const { return bIsBenchmarking; }

    UFUNCTION(BlueprintPure, Category = "Performance|Benchmark")
    FBenchmarkResult GetLastBenchmarkResult() const { return LastBenchmarkResult; }

    UFUNCTION(BlueprintPure, Category = "Performance|Benchmark")
    TArray<FBenchmarkResult> GetBenchmarkHistory() const { return BenchmarkHistory; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Benchmark")
    int32 CalculateBenchmarkScore(const FBenchmarkResult& Result) const;

    // ========================================================================
    // Performance Tier
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Performance|Tier")
    EPerformanceTier GetCurrentTier() const { return CurrentTier; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Tier")
    void SetPerformanceTier(EPerformanceTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Performance|Tier")
    EPerformanceTier DetectRecommendedTier() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Tier")
    void AutoDetectAndApplyTier();

    // ========================================================================
    // Thresholds
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Performance|Thresholds")
    FPerformanceThresholds GetThresholds() const { return Thresholds; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Thresholds")
    void SetThresholds(const FPerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance|Thresholds")
    void SetTargetFrameRate(float TargetFPS);

    // ========================================================================
    // Hardware Info
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Performance|Hardware")
    FHardwareInfo GetHardwareInfo() const { return HardwareInfo; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Hardware")
    void RefreshHardwareInfo();

    // ========================================================================
    // Frame Pacing
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Performance|FramePacing")
    void SetFramePacingMode(EFramePacingMode Mode);

    UFUNCTION(BlueprintPure, Category = "Performance|FramePacing")
    EFramePacingMode GetFramePacingMode() const { return Settings.FramePacing; }

    UFUNCTION(BlueprintCallable, Category = "Performance|FramePacing")
    void SetCustomFrameRateCap(int32 FPS);

    // ========================================================================
    // Settings
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Performance|Settings")
    FPerformanceSettings GetSettings() const { return Settings; }

    UFUNCTION(BlueprintCallable, Category = "Performance|Settings")
    void UpdateSettings(const FPerformanceSettings& NewSettings);

    // ========================================================================
    // Export
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Performance|Export")
    void ExportMetricsToCSV(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Performance|Export")
    FString GeneratePerformanceReport();

    // ========================================================================
    // Events
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnPerformanceWarning OnPerformanceWarning;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnOptimizationHintGenerated OnOptimizationHintGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnBenchmarkComplete OnBenchmarkComplete;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnFrameRateChanged OnFrameRateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnPerformanceTierChanged OnPerformanceTierChanged;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnMemoryPressure OnMemoryPressure;

protected:
    void SampleMetrics();
    void UpdateHistory();
    void CheckThresholds();
    void GenerateWarning(EPerformanceMetricType Type, EPerformanceWarningLevel Level, const FString& Message, float Current, float Threshold);

private:
    UPROPERTY()
    FFrameMetrics CurrentFrameMetrics;

    UPROPERTY()
    FMemoryMetrics CurrentMemoryMetrics;

    UPROPERTY()
    FPerformanceHistory History;

    UPROPERTY()
    TArray<FPerformanceSnapshot> Snapshots;

    UPROPERTY()
    TArray<FPerformanceWarning> ActiveWarnings;

    UPROPERTY()
    TArray<FOptimizationHint> OptimizationHints;

    UPROPERTY()
    TArray<FBenchmarkResult> BenchmarkHistory;

    UPROPERTY()
    FBenchmarkResult LastBenchmarkResult;

    UPROPERTY()
    FBenchmarkResult CurrentBenchmark;

    UPROPERTY()
    FPerformanceThresholds Thresholds;

    UPROPERTY()
    FPerformanceSettings Settings;

    UPROPERTY()
    FHardwareInfo HardwareInfo;

    UPROPERTY()
    EPerformanceTier CurrentTier;

    UPROPERTY()
    bool bIsRecording;

    UPROPERTY()
    bool bIsBenchmarking;

    UPROPERTY()
    float LastFPSForChange;

    FTimerHandle MetricsSampleTimer;
    FTimerHandle BenchmarkTimer;
};

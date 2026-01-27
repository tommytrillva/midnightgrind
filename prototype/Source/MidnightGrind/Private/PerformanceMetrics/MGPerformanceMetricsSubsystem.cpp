// MGPerformanceMetricsSubsystem.cpp
// Performance Metrics System - Implementation
// Midnight Grind - Y2K Arcade Street Racing

#include "PerformanceMetrics/MGPerformanceMetricsSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "RHI.h"
#include "RenderCore.h"
#include "Stats/Stats.h"

void UMGPerformanceMetricsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize settings
    Settings.bEnableMetrics = true;
    Settings.bEnableWarnings = true;
    Settings.bEnableOptimizationHints = true;
    Settings.bAutoOptimize = false;
    Settings.SampleIntervalSeconds = 0.1f;
    Settings.HistorySampleCount = 600;
    Settings.FramePacing = EFramePacingMode::Fixed60;
    Settings.CustomFrameRateCap = 60;

    // Initialize thresholds
    Thresholds.TargetFPS = 60.0f;
    Thresholds.MinAcceptableFPS = 30.0f;
    Thresholds.MaxFrameTimeMs = 33.33f;
    Thresholds.MaxGPUTimeMs = 16.0f;
    Thresholds.MaxCPUTimeMs = 16.0f;
    Thresholds.MaxDrawCalls = 3000;
    Thresholds.MaxTriangles = 5000000;
    Thresholds.MemoryWarningThreshold = 0.8f;
    Thresholds.MemoryCriticalThreshold = 0.95f;
    Thresholds.VRAMWarningThreshold = 0.8f;
    Thresholds.StutterThresholdMs = 50.0f;

    CurrentTier = EPerformanceTier::Medium;
    bIsRecording = false;
    bIsBenchmarking = false;
    LastFPSForChange = 60.0f;

    // Initialize history
    History.AverageFPS = 60.0f;
    History.MinFPS = 60.0f;
    History.MaxFPS = 60.0f;

    RefreshHardwareInfo();

    // Start sampling
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MetricsSampleTimer, this, &UMGPerformanceMetricsSubsystem::SampleMetrics, Settings.SampleIntervalSeconds, true);
    }

    UE_LOG(LogTemp, Log, TEXT("MGPerformanceMetricsSubsystem initialized"));
}

void UMGPerformanceMetricsSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsSampleTimer);
        World->GetTimerManager().ClearTimer(BenchmarkTimer);
    }

    Super::Deinitialize();
}

void UMGPerformanceMetricsSubsystem::SampleMetrics()
{
    if (!Settings.bEnableMetrics)
    {
        return;
    }

    // Sample frame metrics
    CurrentFrameMetrics.Timestamp = FDateTime::Now();

    // Get frame timing (simulated values for now - would use FPlatformTime in real implementation)
    float DeltaTime = FApp::GetDeltaTime();
    CurrentFrameMetrics.FrameTimeMs = DeltaTime * 1000.0f;
    CurrentFrameMetrics.FPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 60.0f;

    // Get actual thread times from engine stats
    extern ENGINE_API float GAverageFPS;
    extern ENGINE_API uint32 GGameThreadTime;
    extern ENGINE_API uint32 GRenderThreadTime;
    extern ENGINE_API uint32 GRHIThreadTime;

    CurrentFrameMetrics.GameThreadMs = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentFrameMetrics.RenderThreadMs = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    CurrentFrameMetrics.RHIThreadMs = FPlatformTime::ToMilliseconds(GRHIThreadTime);

    // CPU time is the max of game and render thread (they run in parallel)
    CurrentFrameMetrics.CPUTimeMs = FMath::Max(CurrentFrameMetrics.GameThreadMs, CurrentFrameMetrics.RenderThreadMs);

    // GPU time estimate (frame time minus CPU work, clamped)
    CurrentFrameMetrics.GPUTimeMs = FMath::Max(0.0f, CurrentFrameMetrics.FrameTimeMs - CurrentFrameMetrics.CPUTimeMs * 0.5f);

    // Get actual draw call and primitive counts from RHI
    CurrentFrameMetrics.DrawCalls = GNumDrawCallsRHI;
    CurrentFrameMetrics.TrianglesDrawn = GNumPrimitivesDrawnRHI;
    CurrentFrameMetrics.PrimitivesDrawn = GNumPrimitivesDrawnRHI;

    // Sample memory metrics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMemoryMetrics.TotalPhysicalMemory = MemStats.TotalPhysical;
    CurrentMemoryMetrics.UsedPhysicalMemory = MemStats.UsedPhysical;
    CurrentMemoryMetrics.AvailablePhysicalMemory = MemStats.AvailablePhysical;

    // Get actual VRAM stats from RHI
    FTextureMemoryStats TextureMemStats;
    RHIGetTextureMemoryStats(TextureMemStats);

    if (TextureMemStats.DedicatedVideoMemory > 0)
    {
        CurrentMemoryMetrics.TotalVRAM = TextureMemStats.DedicatedVideoMemory;
        CurrentMemoryMetrics.UsedVRAM = TextureMemStats.DedicatedVideoMemory - TextureMemStats.StreamingPool;
        CurrentMemoryMetrics.AvailableVRAM = TextureMemStats.StreamingPool;
    }
    else
    {
        // Fallback to hardware info estimate
        CurrentMemoryMetrics.TotalVRAM = HardwareInfo.GPUMemoryMB * 1024 * 1024;
        CurrentMemoryMetrics.UsedVRAM = CurrentMemoryMetrics.TotalVRAM / 2;
        CurrentMemoryMetrics.AvailableVRAM = CurrentMemoryMetrics.TotalVRAM - CurrentMemoryMetrics.UsedVRAM;
    }

    CurrentMemoryMetrics.MemoryPressure = CurrentMemoryMetrics.TotalPhysicalMemory > 0 ?
        static_cast<float>(CurrentMemoryMetrics.UsedPhysicalMemory) / static_cast<float>(CurrentMemoryMetrics.TotalPhysicalMemory) : 0.0f;

    UpdateHistory();
    CheckThresholds();

    // Check for FPS change
    float FPSDiff = FMath::Abs(CurrentFrameMetrics.FPS - LastFPSForChange);
    if (FPSDiff > 10.0f)
    {
        OnFrameRateChanged.Broadcast(LastFPSForChange, CurrentFrameMetrics.FPS);
        LastFPSForChange = CurrentFrameMetrics.FPS;
    }

    // Check memory pressure
    if (CurrentMemoryMetrics.MemoryPressure > Thresholds.MemoryWarningThreshold)
    {
        OnMemoryPressure.Broadcast(CurrentMemoryMetrics.MemoryPressure);
    }
}

void UMGPerformanceMetricsSubsystem::UpdateHistory()
{
    if (!bIsRecording && !bIsBenchmarking)
    {
        return;
    }

    // Add to history
    History.FPSHistory.Add(CurrentFrameMetrics.FPS);
    History.FrameTimeHistory.Add(CurrentFrameMetrics.FrameTimeMs);
    History.GPUTimeHistory.Add(CurrentFrameMetrics.GPUTimeMs);
    History.CPUTimeHistory.Add(CurrentFrameMetrics.CPUTimeMs);
    History.MemoryUsageHistory.Add(CurrentMemoryMetrics.MemoryPressure);

    // Limit history size
    while (History.FPSHistory.Num() > Settings.HistorySampleCount)
    {
        History.FPSHistory.RemoveAt(0);
        History.FrameTimeHistory.RemoveAt(0);
        History.GPUTimeHistory.RemoveAt(0);
        History.CPUTimeHistory.RemoveAt(0);
        History.MemoryUsageHistory.RemoveAt(0);
    }

    // Calculate statistics
    if (History.FPSHistory.Num() > 0)
    {
        float Sum = 0.0f;
        History.MinFPS = History.FPSHistory[0];
        History.MaxFPS = History.FPSHistory[0];

        for (float FPS : History.FPSHistory)
        {
            Sum += FPS;
            History.MinFPS = FMath::Min(History.MinFPS, FPS);
            History.MaxFPS = FMath::Max(History.MaxFPS, FPS);
        }

        History.AverageFPS = Sum / History.FPSHistory.Num();

        // Calculate 1% and 0.1% lows
        TArray<float> SortedFPS = History.FPSHistory;
        SortedFPS.Sort();

        int32 OnePercentIndex = FMath::Max(0, SortedFPS.Num() / 100);
        int32 PointOnePercentIndex = FMath::Max(0, SortedFPS.Num() / 1000);

        History.OnePercentLowFPS = SortedFPS[OnePercentIndex];
        History.PointOnePercentLowFPS = SortedFPS[PointOnePercentIndex];

        // Count frame drops and stutters
        for (int32 i = 1; i < History.FrameTimeHistory.Num(); i++)
        {
            if (History.FrameTimeHistory[i] > Thresholds.MaxFrameTimeMs)
            {
                History.FrameDropCount++;
            }
            if (History.FrameTimeHistory[i] > Thresholds.StutterThresholdMs)
            {
                History.StutterCount++;
            }
        }
    }

    History.TotalRecordingTimeSeconds += Settings.SampleIntervalSeconds;
}

void UMGPerformanceMetricsSubsystem::CheckThresholds()
{
    if (!Settings.bEnableWarnings)
    {
        return;
    }

    // Check FPS
    if (CurrentFrameMetrics.FPS < Thresholds.MinAcceptableFPS)
    {
        GenerateWarning(
            EPerformanceMetricType::FrameRate,
            EPerformanceWarningLevel::Critical,
            TEXT("Frame rate critically low"),
            CurrentFrameMetrics.FPS,
            Thresholds.MinAcceptableFPS
        );
    }
    else if (CurrentFrameMetrics.FPS < Thresholds.TargetFPS * 0.8f)
    {
        GenerateWarning(
            EPerformanceMetricType::FrameRate,
            EPerformanceWarningLevel::Warning,
            TEXT("Frame rate below target"),
            CurrentFrameMetrics.FPS,
            Thresholds.TargetFPS
        );
    }

    // Check GPU time
    if (CurrentFrameMetrics.GPUTimeMs > Thresholds.MaxGPUTimeMs * 1.5f)
    {
        GenerateWarning(
            EPerformanceMetricType::GPUTime,
            EPerformanceWarningLevel::Critical,
            TEXT("GPU time critically high"),
            CurrentFrameMetrics.GPUTimeMs,
            Thresholds.MaxGPUTimeMs
        );
    }

    // Check memory
    if (CurrentMemoryMetrics.MemoryPressure > Thresholds.MemoryCriticalThreshold)
    {
        GenerateWarning(
            EPerformanceMetricType::MemoryUsage,
            EPerformanceWarningLevel::Critical,
            TEXT("Memory usage critically high"),
            CurrentMemoryMetrics.MemoryPressure * 100.0f,
            Thresholds.MemoryCriticalThreshold * 100.0f
        );
    }
    else if (CurrentMemoryMetrics.MemoryPressure > Thresholds.MemoryWarningThreshold)
    {
        GenerateWarning(
            EPerformanceMetricType::MemoryUsage,
            EPerformanceWarningLevel::Warning,
            TEXT("Memory usage high"),
            CurrentMemoryMetrics.MemoryPressure * 100.0f,
            Thresholds.MemoryWarningThreshold * 100.0f
        );
    }

    // Check draw calls
    if (CurrentFrameMetrics.DrawCalls > Thresholds.MaxDrawCalls)
    {
        GenerateWarning(
            EPerformanceMetricType::DrawCalls,
            EPerformanceWarningLevel::Warning,
            TEXT("Draw call count high"),
            static_cast<float>(CurrentFrameMetrics.DrawCalls),
            static_cast<float>(Thresholds.MaxDrawCalls)
        );
    }
}

void UMGPerformanceMetricsSubsystem::GenerateWarning(EPerformanceMetricType Type, EPerformanceWarningLevel Level, const FString& Message, float Current, float Threshold)
{
    // Check if warning already exists
    for (FPerformanceWarning& Warning : ActiveWarnings)
    {
        if (Warning.MetricType == Type && !Warning.bResolved)
        {
            Warning.CurrentValue = Current;
            Warning.OccurrenceCount++;
            Warning.Timestamp = FDateTime::Now();
            return;
        }
    }

    // Create new warning
    FPerformanceWarning Warning;
    Warning.WarningId = FGuid::NewGuid();
    Warning.Level = Level;
    Warning.MetricType = Type;
    Warning.Message = Message;
    Warning.CurrentValue = Current;
    Warning.ThresholdValue = Threshold;
    Warning.Timestamp = FDateTime::Now();
    Warning.OccurrenceCount = 1;

    // Generate suggestion
    switch (Type)
    {
        case EPerformanceMetricType::FrameRate:
            Warning.Suggestion = TEXT("Consider lowering graphics settings or resolution");
            break;
        case EPerformanceMetricType::GPUTime:
            Warning.Suggestion = TEXT("Reduce effects quality or shadow resolution");
            break;
        case EPerformanceMetricType::MemoryUsage:
            Warning.Suggestion = TEXT("Close background applications or lower texture quality");
            break;
        case EPerformanceMetricType::DrawCalls:
            Warning.Suggestion = TEXT("Enable instancing or reduce object density");
            break;
        default:
            Warning.Suggestion = TEXT("Review performance settings");
    }

    ActiveWarnings.Add(Warning);
    OnPerformanceWarning.Broadcast(Warning);
}

// ============================================================================
// Real-Time Metrics
// ============================================================================

float UMGPerformanceMetricsSubsystem::GetMemoryUsagePercent() const
{
    return CurrentMemoryMetrics.MemoryPressure * 100.0f;
}

float UMGPerformanceMetricsSubsystem::GetVRAMUsagePercent() const
{
    if (CurrentMemoryMetrics.TotalVRAM > 0)
    {
        return (static_cast<float>(CurrentMemoryMetrics.UsedVRAM) / static_cast<float>(CurrentMemoryMetrics.TotalVRAM)) * 100.0f;
    }
    return 0.0f;
}

// ============================================================================
// History
// ============================================================================

void UMGPerformanceMetricsSubsystem::ClearHistory()
{
    History = FPerformanceHistory();
    Snapshots.Empty();
}

void UMGPerformanceMetricsSubsystem::StartRecording()
{
    bIsRecording = true;
    ClearHistory();
    UE_LOG(LogTemp, Log, TEXT("Performance recording started"));
}

void UMGPerformanceMetricsSubsystem::StopRecording()
{
    bIsRecording = false;
    UE_LOG(LogTemp, Log, TEXT("Performance recording stopped - %d samples collected"), History.FPSHistory.Num());
}

FPerformanceSnapshot UMGPerformanceMetricsSubsystem::TakeSnapshot()
{
    FPerformanceSnapshot Snapshot;
    Snapshot.SnapshotId = FGuid::NewGuid();
    Snapshot.CapturedAt = FDateTime::Now();
    Snapshot.FrameMetrics = CurrentFrameMetrics;
    Snapshot.MemoryMetrics = CurrentMemoryMetrics;

    if (UWorld* World = GetWorld())
    {
        Snapshot.SceneName = World->GetMapName();
    }

    Snapshots.Add(Snapshot);
    return Snapshot;
}

// ============================================================================
// Warnings
// ============================================================================

void UMGPerformanceMetricsSubsystem::ClearWarnings()
{
    ActiveWarnings.Empty();
}

void UMGPerformanceMetricsSubsystem::DismissWarning(const FGuid& WarningId)
{
    for (int32 i = ActiveWarnings.Num() - 1; i >= 0; i--)
    {
        if (ActiveWarnings[i].WarningId == WarningId)
        {
            ActiveWarnings[i].bResolved = true;
            ActiveWarnings.RemoveAt(i);
            return;
        }
    }
}

bool UMGPerformanceMetricsSubsystem::HasCriticalWarnings() const
{
    for (const FPerformanceWarning& Warning : ActiveWarnings)
    {
        if (Warning.Level == EPerformanceWarningLevel::Critical && !Warning.bResolved)
        {
            return true;
        }
    }
    return false;
}

void UMGPerformanceMetricsSubsystem::SetWarningsEnabled(bool bEnabled)
{
    Settings.bEnableWarnings = bEnabled;
}

// ============================================================================
// Optimization
// ============================================================================

void UMGPerformanceMetricsSubsystem::GenerateOptimizationHints()
{
    OptimizationHints.Empty();

    // Generate hints based on current metrics
    if (CurrentFrameMetrics.GPUTimeMs > Thresholds.MaxGPUTimeMs)
    {
        FOptimizationHint Hint;
        Hint.HintId = FGuid::NewGuid();
        Hint.Category = EOptimizationCategory::Rendering;
        Hint.Title = TEXT("Reduce Shadow Quality");
        Hint.Description = TEXT("Lowering shadow resolution can significantly improve GPU performance");
        Hint.ExpectedImprovement = 15.0f;
        Hint.Priority = 8;
        Hint.bAutoApplicable = true;
        OptimizationHints.Add(Hint);

        OnOptimizationHintGenerated.Broadcast(Hint);
    }

    if (CurrentFrameMetrics.DrawCalls > Thresholds.MaxDrawCalls * 0.8f)
    {
        FOptimizationHint Hint;
        Hint.HintId = FGuid::NewGuid();
        Hint.Category = EOptimizationCategory::Rendering;
        Hint.Title = TEXT("Enable Draw Call Batching");
        Hint.Description = TEXT("Batching similar objects can reduce draw call overhead");
        Hint.ExpectedImprovement = 10.0f;
        Hint.Priority = 6;
        Hint.bAutoApplicable = true;
        OptimizationHints.Add(Hint);

        OnOptimizationHintGenerated.Broadcast(Hint);
    }

    if (CurrentMemoryMetrics.MemoryPressure > 0.7f)
    {
        FOptimizationHint Hint;
        Hint.HintId = FGuid::NewGuid();
        Hint.Category = EOptimizationCategory::Memory;
        Hint.Title = TEXT("Reduce Texture Streaming Pool");
        Hint.Description = TEXT("Lowering texture quality can free up significant memory");
        Hint.ExpectedImprovement = 20.0f;
        Hint.Priority = 7;
        Hint.bAutoApplicable = true;
        OptimizationHints.Add(Hint);

        OnOptimizationHintGenerated.Broadcast(Hint);
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d optimization hints"), OptimizationHints.Num());
}

bool UMGPerformanceMetricsSubsystem::ApplyOptimizationHint(const FGuid& HintId)
{
    for (FOptimizationHint& Hint : OptimizationHints)
    {
        if (Hint.HintId == HintId && !Hint.bApplied)
        {
            Hint.bApplied = true;
            UE_LOG(LogTemp, Log, TEXT("Applied optimization hint: %s"), *Hint.Title);
            return true;
        }
    }
    return false;
}

void UMGPerformanceMetricsSubsystem::ApplyAllAutoHints()
{
    for (FOptimizationHint& Hint : OptimizationHints)
    {
        if (Hint.bAutoApplicable && !Hint.bApplied)
        {
            ApplyOptimizationHint(Hint.HintId);
        }
    }
}

void UMGPerformanceMetricsSubsystem::RevertOptimization(const FGuid& HintId)
{
    for (FOptimizationHint& Hint : OptimizationHints)
    {
        if (Hint.HintId == HintId && Hint.bApplied)
        {
            Hint.bApplied = false;
            UE_LOG(LogTemp, Log, TEXT("Reverted optimization hint: %s"), *Hint.Title);
            return;
        }
    }
}

// ============================================================================
// Benchmarking
// ============================================================================

void UMGPerformanceMetricsSubsystem::StartBenchmark(EBenchmarkType Type)
{
    if (bIsBenchmarking)
    {
        return;
    }

    bIsBenchmarking = true;
    ClearHistory();
    bIsRecording = true;

    CurrentBenchmark.BenchmarkId = FGuid::NewGuid();
    CurrentBenchmark.Type = Type;
    CurrentBenchmark.StartTime = FDateTime::Now();
    CurrentBenchmark.TotalFrames = 0;
    CurrentBenchmark.DroppedFrames = 0;

    float Duration = 30.0f;
    switch (Type)
    {
        case EBenchmarkType::Quick: Duration = 15.0f; break;
        case EBenchmarkType::Standard: Duration = 30.0f; break;
        case EBenchmarkType::Extended: Duration = 60.0f; break;
        case EBenchmarkType::Stress: Duration = 120.0f; break;
        default: Duration = 30.0f;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(BenchmarkTimer, this, &UMGPerformanceMetricsSubsystem::StopBenchmark, Duration, false);
    }

    UE_LOG(LogTemp, Log, TEXT("Benchmark started: %d"), static_cast<int32>(Type));
}

void UMGPerformanceMetricsSubsystem::StopBenchmark()
{
    if (!bIsBenchmarking)
    {
        return;
    }

    bIsBenchmarking = false;
    bIsRecording = false;

    CurrentBenchmark.EndTime = FDateTime::Now();
    CurrentBenchmark.DurationSeconds = (CurrentBenchmark.EndTime - CurrentBenchmark.StartTime).GetTotalSeconds();

    // Copy history stats
    CurrentBenchmark.AverageFPS = History.AverageFPS;
    CurrentBenchmark.MinFPS = History.MinFPS;
    CurrentBenchmark.MaxFPS = History.MaxFPS;
    CurrentBenchmark.OnePercentLow = History.OnePercentLowFPS;
    CurrentBenchmark.TotalFrames = History.FPSHistory.Num();
    CurrentBenchmark.DroppedFrames = History.FrameDropCount;

    // Calculate averages
    float TotalFrameTime = 0.0f, TotalGPU = 0.0f, TotalCPU = 0.0f;
    for (int32 i = 0; i < History.FrameTimeHistory.Num(); i++)
    {
        TotalFrameTime += History.FrameTimeHistory[i];
        TotalGPU += History.GPUTimeHistory[i];
        TotalCPU += History.CPUTimeHistory[i];
    }

    int32 Count = History.FrameTimeHistory.Num();
    if (Count > 0)
    {
        CurrentBenchmark.AverageFrameTime = TotalFrameTime / Count;
        CurrentBenchmark.AverageGPUTime = TotalGPU / Count;
        CurrentBenchmark.AverageCPUTime = TotalCPU / Count;
    }

    // Calculate score
    CurrentBenchmark.Score = CalculateBenchmarkScore(CurrentBenchmark);

    // Determine recommended tier
    CurrentBenchmark.RecommendedTier = DetectRecommendedTier();

    LastBenchmarkResult = CurrentBenchmark;
    BenchmarkHistory.Add(CurrentBenchmark);

    OnBenchmarkComplete.Broadcast(CurrentBenchmark);

    UE_LOG(LogTemp, Log, TEXT("Benchmark complete - Score: %d, Avg FPS: %.1f, Recommended: %d"),
        CurrentBenchmark.Score, CurrentBenchmark.AverageFPS, static_cast<int32>(CurrentBenchmark.RecommendedTier));
}

int32 UMGPerformanceMetricsSubsystem::CalculateBenchmarkScore(const FBenchmarkResult& Result) const
{
    // Score formula based on various metrics
    float Score = 0.0f;

    // FPS contribution (40% weight)
    Score += (Result.AverageFPS / 60.0f) * 4000.0f;

    // 1% low contribution (20% weight)
    Score += (Result.OnePercentLow / 30.0f) * 2000.0f;

    // Frame time consistency (20% weight)
    float Consistency = 1.0f - (Result.AverageFrameTime / 50.0f);
    Score += FMath::Max(0.0f, Consistency) * 2000.0f;

    // Dropped frames penalty (20% weight)
    float DropPenalty = 1.0f - (static_cast<float>(Result.DroppedFrames) / FMath::Max(1, Result.TotalFrames));
    Score += DropPenalty * 2000.0f;

    return FMath::RoundToInt(FMath::Clamp(Score, 0.0f, 10000.0f));
}

// ============================================================================
// Performance Tier
// ============================================================================

void UMGPerformanceMetricsSubsystem::SetPerformanceTier(EPerformanceTier Tier)
{
    if (CurrentTier != Tier)
    {
        CurrentTier = Tier;
        OnPerformanceTierChanged.Broadcast(Tier);
    }
}

EPerformanceTier UMGPerformanceMetricsSubsystem::DetectRecommendedTier() const
{
    float AvgFPS = History.FPSHistory.Num() > 0 ? History.AverageFPS : CurrentFrameMetrics.FPS;

    if (AvgFPS >= 120.0f)
    {
        return EPerformanceTier::Ultra;
    }
    else if (AvgFPS >= 60.0f)
    {
        return EPerformanceTier::High;
    }
    else if (AvgFPS >= 45.0f)
    {
        return EPerformanceTier::Medium;
    }
    else
    {
        return EPerformanceTier::Low;
    }
}

void UMGPerformanceMetricsSubsystem::AutoDetectAndApplyTier()
{
    EPerformanceTier Recommended = DetectRecommendedTier();
    SetPerformanceTier(Recommended);
}

// ============================================================================
// Thresholds
// ============================================================================

void UMGPerformanceMetricsSubsystem::SetThresholds(const FPerformanceThresholds& NewThresholds)
{
    Thresholds = NewThresholds;
}

void UMGPerformanceMetricsSubsystem::SetTargetFrameRate(float TargetFPS)
{
    Thresholds.TargetFPS = TargetFPS;
    Thresholds.MaxFrameTimeMs = 1000.0f / TargetFPS;
}

// ============================================================================
// Hardware Info
// ============================================================================

void UMGPerformanceMetricsSubsystem::RefreshHardwareInfo()
{
    HardwareInfo.CPUBrand = FPlatformMisc::GetCPUBrand();
    HardwareInfo.CPUCores = FPlatformMisc::NumberOfCores();
    HardwareInfo.CPUThreads = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
    HardwareInfo.GPUBrand = FPlatformMisc::GetPrimaryGPUBrand();
    HardwareInfo.OSVersion = FPlatformMisc::GetOSVersion();

    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    HardwareInfo.SystemMemoryMB = MemStats.TotalPhysical / (1024 * 1024);

    // Simulated GPU memory (would query actual in real implementation)
    HardwareInfo.GPUMemoryMB = 8192;

    HardwareInfo.RHIName = TEXT("D3D12");
    HardwareInfo.bSupportsRayTracing = true;
    HardwareInfo.bSupportsNanite = true;
    HardwareInfo.bSupportsLumen = true;

    UE_LOG(LogTemp, Log, TEXT("Hardware Info - CPU: %s, GPU: %s, RAM: %lld MB"),
        *HardwareInfo.CPUBrand, *HardwareInfo.GPUBrand, HardwareInfo.SystemMemoryMB);
}

// ============================================================================
// Frame Pacing
// ============================================================================

void UMGPerformanceMetricsSubsystem::SetFramePacingMode(EFramePacingMode Mode)
{
    Settings.FramePacing = Mode;

    switch (Mode)
    {
        case EFramePacingMode::Fixed30:
            SetTargetFrameRate(30.0f);
            break;
        case EFramePacingMode::Fixed60:
            SetTargetFrameRate(60.0f);
            break;
        case EFramePacingMode::Fixed120:
            SetTargetFrameRate(120.0f);
            break;
        case EFramePacingMode::Custom:
            SetTargetFrameRate(static_cast<float>(Settings.CustomFrameRateCap));
            break;
        default:
            break;
    }
}

void UMGPerformanceMetricsSubsystem::SetCustomFrameRateCap(int32 FPS)
{
    Settings.CustomFrameRateCap = FMath::Clamp(FPS, 30, 300);
    if (Settings.FramePacing == EFramePacingMode::Custom)
    {
        SetTargetFrameRate(static_cast<float>(Settings.CustomFrameRateCap));
    }
}

// ============================================================================
// Settings
// ============================================================================

void UMGPerformanceMetricsSubsystem::UpdateSettings(const FPerformanceSettings& NewSettings)
{
    bool bIntervalChanged = Settings.SampleIntervalSeconds != NewSettings.SampleIntervalSeconds;
    Settings = NewSettings;

    if (bIntervalChanged)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(MetricsSampleTimer);
            World->GetTimerManager().SetTimer(MetricsSampleTimer, this, &UMGPerformanceMetricsSubsystem::SampleMetrics, Settings.SampleIntervalSeconds, true);
        }
    }
}

// ============================================================================
// Export
// ============================================================================

void UMGPerformanceMetricsSubsystem::ExportMetricsToCSV(const FString& FilePath)
{
    FString CSV = TEXT("Frame,FPS,FrameTime,GPUTime,CPUTime,MemoryUsage\n");

    for (int32 i = 0; i < History.FPSHistory.Num(); i++)
    {
        CSV += FString::Printf(TEXT("%d,%.2f,%.2f,%.2f,%.2f,%.2f\n"),
            i,
            History.FPSHistory[i],
            History.FrameTimeHistory[i],
            History.GPUTimeHistory[i],
            History.CPUTimeHistory[i],
            History.MemoryUsageHistory[i] * 100.0f
        );
    }

    FFileHelper::SaveStringToFile(CSV, *FilePath);
    UE_LOG(LogTemp, Log, TEXT("Exported metrics to: %s"), *FilePath);
}

FString UMGPerformanceMetricsSubsystem::GeneratePerformanceReport()
{
    return FString::Printf(TEXT(
        "=== Performance Report ===\n"
        "Current FPS: %.1f\n"
        "Average FPS: %.1f\n"
        "Min FPS: %.1f\n"
        "Max FPS: %.1f\n"
        "1%% Low: %.1f\n"
        "Frame Time: %.2f ms\n"
        "GPU Time: %.2f ms\n"
        "CPU Time: %.2f ms\n"
        "Memory Usage: %.1f%%\n"
        "VRAM Usage: %.1f%%\n"
        "Draw Calls: %d\n"
        "Triangles: %lld\n"
        "Current Tier: %d\n"
        "Warnings: %d\n"
    ),
        CurrentFrameMetrics.FPS,
        History.AverageFPS,
        History.MinFPS,
        History.MaxFPS,
        History.OnePercentLowFPS,
        CurrentFrameMetrics.FrameTimeMs,
        CurrentFrameMetrics.GPUTimeMs,
        CurrentFrameMetrics.CPUTimeMs,
        GetMemoryUsagePercent(),
        GetVRAMUsagePercent(),
        CurrentFrameMetrics.DrawCalls,
        CurrentFrameMetrics.TrianglesDrawn,
        static_cast<int32>(CurrentTier),
        ActiveWarnings.Num()
    );
}

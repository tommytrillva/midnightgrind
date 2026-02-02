// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGPerformanceMonitorSubsystem.h
 * Real-Time Performance Monitoring and Dynamic Quality Adjustment
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem provides real-time monitoring of game performance and
 * automatic quality adjustments to maintain smooth gameplay. While
 * MGPerformanceMetricsSubsystem focuses on detailed analysis and reporting,
 * this subsystem focuses on ACTIVE monitoring and AUTOMATIC responses.
 *
 * Think of it as an "autopilot" for performance - it watches metrics and
 * automatically adjusts settings to keep the game running smoothly.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. PERFORMANCE LEVELS (EMGPerformanceLevel):
 *    A simplified rating of current performance:
 *    - Excellent: Well above target, could increase quality
 *    - High: At or above target, running great
 *    - Medium: Slightly below target, acceptable
 *    - Low: Significantly below target, needs attention
 *    - Critical: Game is struggling, immediate action needed
 *
 * 2. DYNAMIC QUALITY ADJUSTMENT:
 *    Instead of static quality presets, dynamic quality automatically
 *    adjusts settings in real-time based on performance:
 *    - If FPS drops, reduce effects/resolution
 *    - If FPS is high, increase quality
 *    - Keeps the game smooth regardless of scene complexity
 *
 * 3. RESOLUTION SCALING:
 *    Rendering at a lower resolution then upscaling to display resolution.
 *    - 1.0 = native resolution (best quality)
 *    - 0.7 = 70% resolution (significant performance boost)
 *    - Most effective way to gain performance quickly
 *    - Modern upscaling (DLSS, FSR) makes this less noticeable
 *
 * 4. FRAME TIME VARIANCE:
 *    How consistent frame times are. High variance = stuttery even if
 *    average FPS is good. Low variance = smooth consistent gameplay.
 *
 * 5. QUALITY PRESETS (EMGQualityPreset):
 *    Predefined collections of settings:
 *    - Low: Minimum quality, maximum performance
 *    - Medium: Balanced
 *    - High: Good quality, good performance
 *    - Ultra: Maximum quality, requires powerful hardware
 *    - Custom: User has manually adjusted settings
 *    - Auto: System chooses based on detected hardware
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *    [Every Frame] --> [Performance Monitor] --> [Quality Decision]
 *          |                   |                        |
 *          v                   v                        v
 *    [Collect Stats]    [Check Thresholds]      [Adjust Settings]
 *          |                   |                        |
 *          v                   v                        v
 *    [Update History]    [Generate Alerts]      [Apply Changes]
 *
 * This subsystem works closely with:
 * - MGPerformanceMetricsSubsystem: Shares data, different focus
 * - MGLODSubsystem: May adjust LOD bias for performance
 * - MGStreamingSubsystem: May limit streaming during low performance
 * - Graphics Settings: Directly modifies quality settings
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * 1. Enable automatic quality adjustment:
 *    Monitor->SetDynamicQualityEnabled(true);
 *
 * 2. Check if performance is acceptable:
 *    if (Monitor->GetOverallPerformanceLevel() <= EMGPerformanceLevel::Low) {
 *        // Show warning to player
 *    }
 *
 * 3. Apply a quality preset:
 *    Monitor->ApplyQualityPreset(EMGQualityPreset::Medium);
 *
 * 4. Get current stats for UI display:
 *    FMGFrameTimeStats Stats = Monitor->GetFrameStats();
 *    DisplayFPS(Stats.CurrentFPS);
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPerformanceMonitorSubsystem.generated.h"

/**
 * EMGPerformanceLevel - Simplified rating of current performance.
 *
 * Unlike raw FPS numbers, this gives a quick assessment that's easy
 * to understand and act upon. Used for alerts and dynamic adjustments.
 */
UENUM(BlueprintType)
enum class EMGPerformanceLevel : uint8
{
	Critical,  // Severely below target, game may be unplayable (<50% of target FPS)
	Low,       // Significantly below target, noticeable issues (50-75% of target)
	Medium,    // Slightly below target, acceptable but not ideal (75-90% of target)
	High,      // At or above target, running well (90-110% of target)
	Excellent  // Well above target, could increase quality (>110% of target)
};

/**
 * EMGPerformanceCategory - Areas of the game that can affect performance.
 *
 * When an alert is generated, the category helps identify what's causing
 * the issue and what kind of solution might help.
 */
UENUM(BlueprintType)
enum class EMGPerformanceCategory : uint8
{
	FrameRate,  // Overall FPS / frame timing issues
	Memory,     // RAM usage problems
	GPU,        // Graphics card bottleneck
	CPU,        // Processor bottleneck
	Network,    // Multiplayer latency/bandwidth issues
	Streaming,  // Asset loading affecting performance
	Physics,    // Physics simulation taking too long
	Audio       // Audio processing issues (rare)
};

/**
 * EMGQualityPreset - Graphics quality preset levels.
 *
 * Each preset configures multiple settings together. Using presets is
 * easier than adjusting individual settings and ensures good combinations.
 */
UENUM(BlueprintType)
enum class EMGQualityPreset : uint8
{
	Low,     // Minimum quality - for weak hardware, integrated graphics
	Medium,  // Balanced - for mid-range systems
	High,    // High quality - for gaming PCs with dedicated GPUs
	Ultra,   // Maximum quality - for high-end hardware
	Custom,  // User has manually changed individual settings
	Auto     // System automatically selects based on hardware detection
};

/**
 * FMGFrameTimeStats - Frame rate and timing statistics.
 *
 * Contains all the important FPS-related metrics. For a racing game,
 * consistent frame times are crucial - even small stutters can affect
 * player control and immersion.
 *
 * VARIANCE EXPLAINED:
 * Low variance means consistent frame times (smooth).
 * High variance means frame times jump around (stuttery).
 * A game at 55 FPS with low variance feels smoother than
 * 60 FPS average with high variance.
 */
USTRUCT(BlueprintType)
struct FMGFrameTimeStats
{
	GENERATED_BODY()

	// Current instantaneous FPS (may fluctuate rapidly)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFPS = 60.0f;

	// Average FPS over the sampling period
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageFPS = 60.0f;

	// Lowest FPS recorded in the sampling period
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinFPS = 60.0f;

	// Highest FPS recorded in the sampling period
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFPS = 60.0f;

	// Current frame time in milliseconds (inverse of FPS)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrameTimeMs = 16.67f;

	// How much frame times vary (0 = perfectly consistent)
	// High variance indicates stuttering even if average FPS is good
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrameTimeVariance = 0.0f;

	// Number of frames that took significantly longer than target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrameDropCount = 0;

	// 1% low FPS - average of the worst 1% of frames
	// Best single metric for perceived smoothness
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OnePercentLow = 60.0f;

	// 0.1% low FPS - captures severe but rare stutters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointOnePercentLow = 60.0f;
};

/**
 * FMGMemoryStats - Current memory usage statistics.
 *
 * Monitors both system RAM and specific memory pools. Running out of
 * memory can cause crashes or severe stuttering as the OS swaps to disk.
 */
USTRUCT(BlueprintType)
struct FMGMemoryStats
{
	GENERATED_BODY()

	// RAM currently used (MB) - watch for continuous growth (leak)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 UsedPhysicalMemoryMB = 0;

	// RAM currently free (MB) - if this gets very low, trouble incoming
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AvailablePhysicalMemoryMB = 0;

	// Total system RAM (MB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalPhysicalMemoryMB = 0;

	// Virtual memory used (includes page file)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 UsedVirtualMemoryMB = 0;

	// Memory used specifically by textures (usually largest category)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TextureMemoryMB = 0;

	// Memory used by 3D mesh geometry
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MeshMemoryMB = 0;

	// Percentage of total memory used (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MemoryUsagePercent = 0.0f;

	// True when system is running low on memory
	// When true, the system should try to free memory
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMemoryPressure = false;
};

/**
 * FMGGPUStats - Graphics card performance statistics.
 *
 * When GPUTimeMs is high (close to or above frame budget), the game
 * is "GPU-bound" - the graphics card is the bottleneck. Reduce graphics
 * quality settings to improve performance.
 */
USTRUCT(BlueprintType)
struct FMGGPUStats
{
	GENERATED_BODY()

	// Time GPU spent rendering this frame (ms)
	// For 60 FPS, this should be under 16.67ms
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GPUTimeMs = 0.0f;

	// GPU utilization percentage (0-100)
	// Near 100% means GPU-bound
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GPUUtilization = 0.0f;

	// Video memory currently used (MB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 VRAMUsedMB = 0;

	// Video memory available (MB)
	// Running out of VRAM causes severe stuttering
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 VRAMAvailableMB = 0;

	// Number of draw calls this frame
	// Too many draw calls (>3000) causes CPU overhead
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DrawCalls = 0;

	// Number of triangles rendered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Triangles = 0;

	// Time CPU spent preparing render commands (ms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RenderThreadTimeMs = 0.0f;

	// Time spent in RHI (Render Hardware Interface) thread
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RHIThreadTimeMs = 0.0f;
};

/**
 * FMGCPUStats - Processor performance statistics.
 *
 * When GameThreadTimeMs is high, the game is "CPU-bound" - the processor
 * is the bottleneck. This is harder to fix than GPU issues because it
 * often requires code optimization rather than settings changes.
 *
 * For racing games, physics is often a significant CPU cost.
 */
USTRUCT(BlueprintType)
struct FMGCPUStats
{
	GENERATED_BODY()

	// Time spent on the main game thread (ms)
	// This includes gameplay logic, physics results, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GameThreadTimeMs = 0.0f;

	// Overall CPU utilization percentage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CPUUtilization = 0.0f;

	// Number of active threads (for debugging multithreading)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActiveThreads = 0;

	// Time spent on physics simulation (ms)
	// Racing games with many vehicles can have high physics cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhysicsTimeMs = 0.0f;

	// Time spent on AI processing (ms)
	// Opponent vehicle AI, traffic, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AITimeMs = 0.0f;

	// Time spent evaluating animations (ms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimationTimeMs = 0.0f;
};

/**
 * FMGNetworkStats - Multiplayer networking statistics.
 *
 * For multiplayer racing, network quality is critical. High ping or
 * packet loss can make the game feel unresponsive or cause opponent
 * vehicles to teleport/rubberband.
 */
USTRUCT(BlueprintType)
struct FMGNetworkStats
{
	GENERATED_BODY()

	// Round-trip time to server (ms) - lower is better
	// <50ms = excellent, <100ms = good, >150ms = problematic for racing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PingMs = 0.0f;

	// Percentage of packets that didn't arrive
	// Even 1-2% loss can cause noticeable issues
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PacketLossPercent = 0.0f;

	// Data received per second (KB/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncomingBandwidthKBps = 0.0f;

	// Data sent per second (KB/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutgoingBandwidthKBps = 0.0f;

	// Total packets received this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PacketsReceived = 0;

	// Total packets sent this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PacketsSent = 0;

	// Total packets lost this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PacketsLost = 0;

	// Variation in ping (ms) - high jitter = inconsistent connection
	// Causes unpredictable lag spikes even with low average ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Jitter = 0.0f;
};

/**
 * FMGPerformanceSnapshot - Complete performance picture at one moment.
 *
 * Combines all stat categories into a single snapshot. The history
 * of snapshots can be used to track performance over time, identify
 * patterns, and generate reports.
 */
USTRUCT(BlueprintType)
struct FMGPerformanceSnapshot
{
	GENERATED_BODY()

	// When this snapshot was captured
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	// Frame rate and timing data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGFrameTimeStats FrameStats;

	// Memory usage data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGMemoryStats MemoryStats;

	// Graphics card data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGGPUStats GPUStats;

	// Processor data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGCPUStats CPUStats;

	// Network/multiplayer data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGNetworkStats NetworkStats;

	// Simplified overall performance rating
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPerformanceLevel OverallLevel = EMGPerformanceLevel::High;

	// Which map/track was loaded (for per-level analysis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentMapName;
};

/**
 * FMGPerformanceAlert - Notification of a performance issue.
 *
 * Generated when performance drops below acceptable levels. Alerts
 * include the problem category, severity, and suggested fixes.
 */
USTRUCT(BlueprintType)
struct FMGPerformanceAlert
{
	GENERATED_BODY()

	// What area is having problems
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPerformanceCategory Category = EMGPerformanceCategory::FrameRate;

	// Human-readable description of the problem
	// FText supports localization for multiple languages
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText AlertMessage;

	// How severe is this alert
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPerformanceLevel Severity = EMGPerformanceLevel::Medium;

	// When the alert was generated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	// Recommended action to fix the issue
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SuggestedAction;
};

/**
 * FMGPerformanceThresholds - Limits that define "acceptable" performance.
 *
 * When metrics exceed these thresholds, alerts are generated.
 * Thresholds can be adjusted based on target platform or user preferences.
 */
USTRUCT(BlueprintType)
struct FMGPerformanceThresholds
{
	GENERATED_BODY()

	// FPS we're trying to achieve
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetFPS = 60.0f;

	// FPS below which the game is considered unplayable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinAcceptableFPS = 30.0f;

	// Maximum acceptable frame time (33.33ms = 30 FPS)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFrameTimeMs = 33.33f;

	// Memory usage percentage that triggers warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMemoryUsagePercent = 85.0f;

	// GPU time limit in milliseconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxGPUTimeMs = 16.0f;

	// Network ping limit for multiplayer (ms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPingMs = 100.0f;

	// Packet loss percentage that triggers warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPacketLossPercent = 2.0f;

	// Draw call limit (CPU overhead concern)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxDrawCalls = 5000;
};

/**
 * FMGDynamicQualitySettings - Configuration for automatic quality adjustment.
 *
 * When enabled, the system automatically adjusts graphics settings to
 * maintain the target frame rate. This is like having an AI constantly
 * tweaking settings to keep the game smooth.
 *
 * RESOLUTION SCALING (for beginners):
 * Instead of rendering at full resolution (e.g., 1920x1080), the game
 * renders at a lower resolution (e.g., 1344x756 at 70%) and upscales.
 * This significantly improves performance with relatively small visual impact,
 * especially with modern upscaling techniques (DLSS, FSR, TSR).
 */
USTRUCT(BlueprintType)
struct FMGDynamicQualitySettings
{
	GENERATED_BODY()

	// Master switch for dynamic quality adjustment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	// FPS target the system tries to maintain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetFrameRate = 60.0f;

	// How close to target is "good enough" (54-66 FPS for 60 target at 10%)
	// Prevents constant adjustments when performance is close to target
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TolerancePercent = 10.0f;

	// How often to evaluate and adjust (seconds)
	// Too frequent = jarring, too slow = slow response to changes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AdjustmentInterval = 2.0f;

	// Can the system change render resolution?
	// Most effective single adjustment for performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowResolutionScaling = true;

	// Lowest resolution scale allowed (0.7 = 70% resolution)
	// Going below 70% often looks too blurry
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinResolutionScale = 0.7f;

	// Highest resolution scale (1.0 = native, can go higher for supersampling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxResolutionScale = 1.0f;

	// Can the system adjust shadow quality?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowShadowQualityAdjustment = true;

	// Can the system adjust particle/effects quality?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowEffectsQualityAdjustment = true;

	// Can the system reduce foliage/vegetation density?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowFoliageDensityAdjustment = true;
};

// ============================================================================
// DELEGATES - Events that other systems can subscribe to
// ============================================================================

// Fired when a performance problem is detected
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceAlert, const FMGPerformanceAlert&, Alert);

// Fired when overall performance level changes (e.g., High to Medium)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceLevelChanged, EMGPerformanceLevel, OldLevel, EMGPerformanceLevel, NewLevel);

// Fired when quality preset changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQualitySettingsChanged, EMGQualityPreset, NewPreset);

// Fired when dynamic resolution scaling adjusts the render resolution
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDynamicResolutionChanged, float, NewScale);

/**
 * UMGPerformanceMonitorSubsystem - Active performance monitoring and control.
 *
 * This subsystem runs throughout the game, collecting stats, generating
 * alerts, and optionally auto-adjusting quality settings to maintain
 * smooth performance.
 *
 * DIFFERENCE FROM MGPerformanceMetricsSubsystem:
 * - Metrics: Detailed analysis, benchmarking, reporting, optimization hints
 * - Monitor: Real-time monitoring, alerts, automatic quality adjustment
 *
 * Both can coexist and share data, but have different primary purposes.
 *
 * ACCESS:
 *   auto* Monitor = GetGameInstance()->GetSubsystem<UMGPerformanceMonitorSubsystem>();
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPerformanceMonitorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Called when game starts - initializes monitoring timers
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Called when game ends - cleanup
	virtual void Deinitialize() override;

	// Can return false to prevent subsystem creation (e.g., dedicated server)
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Real-time Stats
	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGFrameTimeStats GetFrameStats() const { return CurrentSnapshot.FrameStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGMemoryStats GetMemoryStats() const { return CurrentSnapshot.MemoryStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGGPUStats GetGPUStats() const { return CurrentSnapshot.GPUStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGCPUStats GetCPUStats() const { return CurrentSnapshot.CPUStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGNetworkStats GetNetworkStats() const { return CurrentSnapshot.NetworkStats; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	FMGPerformanceSnapshot GetCurrentSnapshot() const { return CurrentSnapshot; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	EMGPerformanceLevel GetOverallPerformanceLevel() const { return CurrentSnapshot.OverallLevel; }

	UFUNCTION(BlueprintPure, Category = "Performance|Stats")
	float GetCurrentFPS() const { return CurrentSnapshot.FrameStats.CurrentFPS; }

	// Performance History
	UFUNCTION(BlueprintPure, Category = "Performance|History")
	TArray<FMGPerformanceSnapshot> GetPerformanceHistory() const { return PerformanceHistory; }

	UFUNCTION(BlueprintPure, Category = "Performance|History")
	FMGPerformanceSnapshot GetAverageSnapshot(float DurationSeconds = 60.0f) const;

	UFUNCTION(BlueprintCallable, Category = "Performance|History")
	void ClearHistory();

	// Alerts
	UFUNCTION(BlueprintPure, Category = "Performance|Alerts")
	TArray<FMGPerformanceAlert> GetActiveAlerts() const { return ActiveAlerts; }

	UFUNCTION(BlueprintCallable, Category = "Performance|Alerts")
	void ClearAlerts();

	UFUNCTION(BlueprintCallable, Category = "Performance|Alerts")
	void SetThresholds(const FMGPerformanceThresholds& NewThresholds);

	UFUNCTION(BlueprintPure, Category = "Performance|Alerts")
	FMGPerformanceThresholds GetThresholds() const { return Thresholds; }

	// Quality Settings
	UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
	void ApplyQualityPreset(EMGQualityPreset Preset);

	UFUNCTION(BlueprintPure, Category = "Performance|Quality")
	EMGQualityPreset GetCurrentQualityPreset() const { return CurrentQualityPreset; }

	UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
	void SetResolutionScale(float Scale);

	UFUNCTION(BlueprintPure, Category = "Performance|Quality")
	float GetResolutionScale() const { return CurrentResolutionScale; }

	// Dynamic Quality
	UFUNCTION(BlueprintCallable, Category = "Performance|Dynamic")
	void SetDynamicQualityEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Performance|Dynamic")
	bool IsDynamicQualityEnabled() const { return DynamicQualitySettings.bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Performance|Dynamic")
	void SetDynamicQualitySettings(const FMGDynamicQualitySettings& Settings);

	UFUNCTION(BlueprintPure, Category = "Performance|Dynamic")
	FMGDynamicQualitySettings GetDynamicQualitySettings() const { return DynamicQualitySettings; }

	// Monitoring Control
	UFUNCTION(BlueprintCallable, Category = "Performance|Control")
	void StartMonitoring();

	UFUNCTION(BlueprintCallable, Category = "Performance|Control")
	void StopMonitoring();

	UFUNCTION(BlueprintPure, Category = "Performance|Control")
	bool IsMonitoring() const { return bIsMonitoring; }

	UFUNCTION(BlueprintCallable, Category = "Performance|Control")
	void SetSampleRate(float SamplesPerSecond);

	// Benchmarking
	UFUNCTION(BlueprintCallable, Category = "Performance|Benchmark")
	void StartBenchmark(float DurationSeconds = 60.0f);

	UFUNCTION(BlueprintCallable, Category = "Performance|Benchmark")
	void StopBenchmark();

	UFUNCTION(BlueprintPure, Category = "Performance|Benchmark")
	bool IsBenchmarking() const { return bIsBenchmarking; }

	UFUNCTION(BlueprintPure, Category = "Performance|Benchmark")
	FMGPerformanceSnapshot GetBenchmarkResults() const { return BenchmarkResults; }

	// Profiling
	UFUNCTION(BlueprintCallable, Category = "Performance|Profile")
	void BeginProfileScope(FName ScopeName);

	UFUNCTION(BlueprintCallable, Category = "Performance|Profile")
	void EndProfileScope(FName ScopeName);

	UFUNCTION(BlueprintPure, Category = "Performance|Profile")
	float GetProfileScopeTime(FName ScopeName) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
	FOnPerformanceAlert OnPerformanceAlert;

	UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
	FOnPerformanceLevelChanged OnPerformanceLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
	FOnQualitySettingsChanged OnQualitySettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
	FOnDynamicResolutionChanged OnDynamicResolutionChanged;

protected:
	void CollectStats();
	void UpdateFrameStats();
	void UpdateMemoryStats();
	void UpdateGPUStats();
	void UpdateCPUStats();
	void UpdateNetworkStats();
	void EvaluatePerformanceLevel();
	void CheckThresholds();
	void ProcessDynamicQuality();
	void AddAlert(EMGPerformanceCategory Category, const FString& Message, EMGPerformanceLevel Severity);
	void ApplyLowQualitySettings();
	void ApplyMediumQualitySettings();
	void ApplyHighQualitySettings();
	void ApplyUltraQualitySettings();

	UPROPERTY()
	FMGPerformanceSnapshot CurrentSnapshot;

	UPROPERTY()
	TArray<FMGPerformanceSnapshot> PerformanceHistory;

	UPROPERTY()
	TArray<FMGPerformanceAlert> ActiveAlerts;

	UPROPERTY()
	FMGPerformanceThresholds Thresholds;

	UPROPERTY()
	FMGDynamicQualitySettings DynamicQualitySettings;

	UPROPERTY()
	EMGQualityPreset CurrentQualityPreset = EMGQualityPreset::High;

	UPROPERTY()
	float CurrentResolutionScale = 1.0f;

	UPROPERTY()
	bool bIsMonitoring = false;

	UPROPERTY()
	bool bIsBenchmarking = false;

	UPROPERTY()
	FMGPerformanceSnapshot BenchmarkResults;

	UPROPERTY()
	float SampleInterval = 0.1f;

	UPROPERTY()
	int32 MaxHistorySize = 600;

	UPROPERTY()
	TMap<FName, double> ProfileScopes;

	UPROPERTY()
	TMap<FName, double> ProfileScopeTimes;

	TArray<float> FrameTimeBuffer;
	int32 FrameTimeBufferIndex = 0;
	static const int32 FrameTimeBufferSize = 120;

	FTimerHandle MonitoringTimerHandle;
	FTimerHandle DynamicQualityTimerHandle;
	FTimerHandle BenchmarkTimerHandle;
	float BenchmarkDuration = 0.0f;
	float BenchmarkElapsed = 0.0f;
};

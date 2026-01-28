// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGPerformanceMetricsSubsystem.h
 * Performance Metrics and Profiling System for Midnight Grind
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem collects, analyzes, and reports on game performance data.
 * It tracks frame rates, GPU/CPU usage, memory consumption, and other metrics
 * to help developers optimize the game and allow runtime quality adjustments.
 *
 * Think of it as the game's "health monitor" - constantly checking vital signs
 * and alerting when something is wrong (like a doctor monitoring a patient).
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. FRAME TIME vs FRAME RATE (FPS):
 *    - Frame Rate (FPS): How many frames rendered per second (60 FPS = smooth)
 *    - Frame Time: How long each frame takes in milliseconds
 *    - They're inversely related: 60 FPS = 16.67ms frame time, 30 FPS = 33.33ms
 *    - Frame time is often MORE useful for optimization because it shows
 *      exactly how much time you have per frame
 *
 * 2. GPU TIME vs CPU TIME:
 *    - GPU Time: How long the graphics card takes to render the frame
 *    - CPU Time: How long the processor takes for game logic, physics, etc.
 *    - The LONGER one is your "bottleneck" - the limiting factor
 *    - If GPU Time > CPU Time: You're GPU-bound (optimize rendering)
 *    - If CPU Time > GPU Time: You're CPU-bound (optimize game logic)
 *
 * 3. PERCENTILE FPS (1% Low, 0.1% Low):
 *    - Average FPS can hide stuttering problems
 *    - 1% Low: The FPS during the worst 1% of frames
 *    - 0.1% Low: The FPS during the worst 0.1% of frames (major stutters)
 *    - Example: 60 FPS average but 15 FPS 1% low = game stutters badly
 *
 * 4. DRAW CALLS:
 *    - Each "draw call" is a command sent to the GPU to render something
 *    - Too many draw calls can bottleneck performance (CPU overhead)
 *    - Batching combines multiple objects into fewer draw calls
 *    - Typical targets: <3000 for mobile, <5000 for PC
 *
 * 5. VRAM (Video RAM):
 *    - Memory on your graphics card for textures, shaders, render targets
 *    - Running out of VRAM causes severe stuttering or crashes
 *    - Different from system RAM - both need to be monitored
 *
 * 6. PERFORMANCE TIERS:
 *    - Preset quality levels (Low, Medium, High, Ultra)
 *    - Each tier adjusts multiple settings together
 *    - Auto-detection recommends a tier based on hardware
 *
 * 7. BENCHMARKING:
 *    - Running a standardized test to measure performance
 *    - Produces comparable scores across different hardware
 *    - Used for: quality preset recommendations, comparing builds
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *    [Game Loop] --> [Metrics Subsystem] --> [Performance Data]
 *         |                 |                       |
 *         v                 v                       v
 *    [Each Frame]    [Sample Timer]          [History/Stats]
 *         |                 |                       |
 *         v                 v                       v
 *    [Render/Logic]  [Threshold Check]       [Optimization Hints]
 *                           |
 *                           v
 *                    [Quality Adjustment]
 *
 * The subsystem connects to:
 * - MGPerformanceMonitorSubsystem: Similar system, may work together
 * - MGMemoryBudgetSubsystem: Memory metrics come from here
 * - MGStreamingSubsystem: May reduce streaming if performance drops
 * - MGLODSubsystem: May increase LOD distances if GPU-bound
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * 1. Display current FPS:
 *    float fps = MetricsSubsystem->GetCurrentFPS();
 *
 * 2. Run a benchmark and get recommended settings:
 *    MetricsSubsystem->StartBenchmark(EBenchmarkType::Standard);
 *    // Later, when OnBenchmarkComplete fires:
 *    EPerformanceTier tier = Result.RecommendedTier;
 *
 * 3. Listen for performance warnings:
 *    MetricsSubsystem->OnPerformanceWarning.AddDynamic(this, &MyClass::HandleWarning);
 *
 * 4. Get optimization suggestions:
 *    TArray<FOptimizationHint> hints = MetricsSubsystem->GetOptimizationHints();
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPerformanceMetricsSubsystem.generated.h"

// Forward declarations
class UWorld;

// ============================================================================
// ENUMS - Categories and types used throughout the performance system
// ============================================================================

/**
 * EPerformanceTier - Quality preset levels for the game.
 *
 * Each tier represents a collection of graphics/performance settings
 * balanced for different hardware capabilities. The game can auto-detect
 * the best tier based on benchmarking results.
 *
 * For beginners: Think of these like difficulty settings, but for graphics.
 * Low = runs on weak hardware, Ultra = requires powerful hardware.
 */
UENUM(BlueprintType)
enum class EPerformanceTier : uint8
{
    Low,     // Minimum settings for weak/integrated graphics
    Medium,  // Balanced settings for mid-range hardware
    High,    // High quality for gaming PCs
    Ultra,   // Maximum quality, requires powerful hardware
    Custom   // User has manually adjusted individual settings
};

/**
 * EPerformanceMetricType - Types of performance data that can be measured.
 *
 * Each metric type represents a different aspect of game performance.
 * Understanding which metrics are problematic helps diagnose issues.
 *
 * For beginners: These are like different gauges in a car dashboard -
 * speed (FPS), engine temp (GPU), fuel (memory), etc.
 */
UENUM(BlueprintType)
enum class EPerformanceMetricType : uint8
{
    FrameRate,          // Frames Per Second (FPS) - overall smoothness measure
    FrameTime,          // Milliseconds per frame - inverse of FPS, more precise
    GPUTime,            // Time GPU spends rendering - high = graphics bottleneck
    CPUTime,            // Time CPU spends on game logic - high = logic bottleneck
    DrawCalls,          // Number of render commands - too many = CPU overhead
    TriangleCount,      // Total triangles rendered - affects GPU workload
    MemoryUsage,        // System RAM usage - too high = potential crashes
    VRAMUsage,          // Graphics card memory - too high = severe stuttering
    DiskIO,             // Disk read/write activity - affects loading, streaming
    NetworkLatency,     // Ping/lag for multiplayer - affects responsiveness
    ThreadUtilization,  // How well we use multiple CPU cores
    GarbageCollection   // UE's memory cleanup - spikes cause stutters
};

/**
 * EPerformanceWarningLevel - Severity levels for performance warnings.
 *
 * Used to categorize how serious a performance issue is.
 * Critical warnings may trigger automatic quality reductions.
 */
UENUM(BlueprintType)
enum class EPerformanceWarningLevel : uint8
{
    None,      // No issue detected
    Info,      // Minor observation, no action needed
    Warning,   // Performance degraded, consider adjustments
    Critical   // Severe issue, game may be unplayable without action
};

/**
 * EOptimizationCategory - Areas of the game that can be optimized.
 *
 * Optimization hints are categorized to help developers understand
 * which system needs attention.
 */
UENUM(BlueprintType)
enum class EOptimizationCategory : uint8
{
    Rendering,   // Graphics, shaders, post-processing, draw calls
    Physics,     // Collision detection, vehicle physics, ragdolls
    Audio,       // Sound effects, music, voice, audio processing
    AI,          // NPC behavior, pathfinding, decision making
    Networking,  // Multiplayer sync, replication, bandwidth
    Memory,      // RAM usage, allocations, garbage collection
    Loading,     // Asset streaming, level loading, disk I/O
    General      // Miscellaneous or cross-cutting concerns
};

/**
 * EBenchmarkType - Types of performance benchmarks that can be run.
 *
 * Benchmarks run standardized tests to measure hardware capability.
 * Different types trade off between accuracy and time required.
 */
UENUM(BlueprintType)
enum class EBenchmarkType : uint8
{
    Quick,     // ~30 seconds, rough estimate, good for quick checks
    Standard,  // ~2 minutes, balanced accuracy vs time
    Extended,  // ~5 minutes, more accurate, tests more scenarios
    Stress,    // ~10 minutes, pushes hardware to limits, finds instability
    Custom     // User-defined duration and parameters
};

/**
 * EFramePacingMode - How the game controls frame timing.
 *
 * Frame pacing affects smoothness and input latency.
 *
 * VSYNC (for beginners): Synchronizes frames with monitor refresh rate.
 * Prevents "screen tearing" (visible horizontal lines during fast motion)
 * but can add input lag. Racing games often prefer tearing over lag.
 */
UENUM(BlueprintType)
enum class EFramePacingMode : uint8
{
    Uncapped,   // No limit, render as fast as possible (may cause tearing)
    VSync,      // Sync to monitor refresh rate (eliminates tearing, adds lag)
    Fixed30,    // Lock to 30 FPS (consistent but not smooth)
    Fixed60,    // Lock to 60 FPS (standard target for most games)
    Fixed120,   // Lock to 120 FPS (for high refresh rate monitors)
    Variable,   // Adaptive sync (G-Sync/FreeSync) - best of both worlds
    Custom      // User-defined frame rate cap
};

// ============================================================================
// STRUCTS - Data containers for performance information
// ============================================================================

/**
 * FFrameMetrics - Detailed timing data for a single frame.
 *
 * This struct captures WHERE time is being spent each frame. By comparing
 * these values, you can identify bottlenecks:
 *
 * - If GPUTimeMs > CPUTimeMs: GPU-bound (reduce graphics quality)
 * - If GameThreadMs is high: Game logic bottleneck (optimize code)
 * - If RenderThreadMs is high: Draw call bottleneck (reduce objects)
 *
 * THREAD EXPLANATION (for beginners):
 * Unreal runs multiple threads (parallel execution paths):
 * - Game Thread: Runs gameplay logic, physics, AI
 * - Render Thread: Prepares draw commands for GPU
 * - RHI Thread: Submits commands to graphics API (DirectX/Vulkan)
 *
 * For 60 FPS, total frame time must be under 16.67ms.
 */
USTRUCT(BlueprintType)
struct FFrameMetrics
{
    GENERATED_BODY()

    // Total time for the entire frame (should be < 16.67ms for 60 FPS)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FrameTimeMs = 16.67f;

    // Time the GPU spent rendering (high = graphics bottleneck)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GPUTimeMs = 8.0f;

    // Time the CPU spent on all processing (high = logic bottleneck)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CPUTimeMs = 8.0f;

    // Time specifically for game logic (physics, AI, gameplay code)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GameThreadMs = 4.0f;

    // Time for preparing render commands (visibility, culling, batching)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RenderThreadMs = 4.0f;

    // Time for RHI (Render Hardware Interface) - low-level GPU communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RHIThreadMs = 2.0f;

    // Frames per second (derived from FrameTimeMs: FPS = 1000 / FrameTimeMs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FPS = 60.0f;

    // Number of draw calls this frame (target: < 3000)
    // Each draw call has CPU overhead; too many = CPU bottleneck
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DrawCalls = 500;

    // Total triangles rendered (affects GPU workload)
    // Modern GPUs can handle millions, but more = slower
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TrianglesDrawn = 1000000;

    // Number of primitive objects drawn (meshes, sprites, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PrimitivesDrawn = 5000;

    // When this data was captured
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;
};

/**
 * FMemoryMetrics - Current memory usage across different pools.
 *
 * Tracks both system RAM and VRAM (video memory). Running out of either
 * causes serious problems - crashes or severe stuttering.
 *
 * MEMORY POOLS (for beginners):
 * Memory is divided into "pools" for different purposes. This helps
 * identify what's using too much memory:
 * - Textures: Usually the biggest consumer (images, normal maps)
 * - Meshes: 3D model geometry
 * - Audio: Sound effects, music (can be huge for high-quality audio)
 * - Physics: Collision shapes, simulation data
 */
USTRUCT(BlueprintType)
struct FMemoryMetrics
{
    GENERATED_BODY()

    // -------------------------------------------------------------------------
    // SYSTEM RAM (Physical Memory)
    // -------------------------------------------------------------------------

    // Total RAM installed in the system (e.g., 16GB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalPhysicalMemory = 0;

    // How much RAM is currently in use (by this game + other apps)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 UsedPhysicalMemory = 0;

    // How much RAM is free and available
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 AvailablePhysicalMemory = 0;

    // -------------------------------------------------------------------------
    // VIDEO MEMORY (VRAM on GPU)
    // -------------------------------------------------------------------------

    // Total VRAM on the graphics card (e.g., 8GB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalVRAM = 0;

    // How much VRAM is currently used
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 UsedVRAM = 0;

    // How much VRAM is free
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 AvailableVRAM = 0;

    // -------------------------------------------------------------------------
    // MEMORY BY CATEGORY
    // -------------------------------------------------------------------------

    // Memory used by textures (usually the biggest category)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TextureMemory = 0;

    // Memory used by 3D mesh geometry
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MeshMemory = 0;

    // Memory used by audio data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 AudioMemory = 0;

    // Memory used by physics simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 PhysicsMemory = 0;

    // -------------------------------------------------------------------------
    // PRESSURE AND GARBAGE COLLECTION
    // -------------------------------------------------------------------------

    // Memory pressure level (0.0 = fine, 1.0 = critical)
    // High pressure triggers cleanup and quality reductions
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryPressure = 0.0f;

    // How many times Unreal's garbage collector has run
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GCCollectionCount = 0;

    // How long the last garbage collection took (can cause stutters if high)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastGCDurationMs = 0.0f;
};

/**
 * FPerformanceSnapshot - A complete picture of performance at one moment.
 *
 * Snapshots capture everything about performance at a specific instant.
 * Comparing snapshots helps identify when and why performance changed.
 *
 * Use cases:
 * - "Performance was fine until we added the new particle system"
 * - "This map runs slower than others - let's compare snapshots"
 * - Save snapshots to file for later analysis
 */
USTRUCT(BlueprintType)
struct FPerformanceSnapshot
{
    GENERATED_BODY()

    // Unique identifier for this snapshot
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SnapshotId;

    // When this snapshot was taken
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CapturedAt;

    // Frame timing data at this moment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FFrameMetrics FrameMetrics;

    // Memory usage data at this moment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMemoryMetrics MemoryMetrics;

    // Which map/level was loaded (for comparison between scenes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SceneName;

    // -------------------------------------------------------------------------
    // SCENE COMPLEXITY METRICS
    // These help understand WHY performance is what it is
    // -------------------------------------------------------------------------

    // Number of vehicles being simulated (cars on track)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveVehicles = 0;

    // Number of active particle emitters (smoke, sparks, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveParticles = 0;

    // Number of dynamic lights in scene (expensive!)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveLights = 0;

    // Number of texture assets currently loaded
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LoadedTextures = 0;

    // Number of mesh assets currently loaded
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LoadedMeshes = 0;

    // Network latency (for multiplayer)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NetworkLatencyMs = 0.0f;

    // Custom metrics added by game code (for game-specific tracking)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> CustomMetrics;
};

/**
 * FPerformanceHistory - Accumulated performance data over time.
 *
 * While FPerformanceSnapshot is a single moment, FPerformanceHistory
 * tracks trends and calculates statistics over a recording session.
 *
 * KEY STATISTICS EXPLAINED:
 * - Average FPS: Mean frame rate (can hide problems)
 * - 1% Low FPS: Average of the worst 1% of frames (shows stutters)
 * - 0.1% Low FPS: Average of the worst 0.1% of frames (severe stutters)
 * - Frame Drops: Frames that took much longer than target
 * - Stutters: Noticeable hitches the player would feel
 *
 * For beginners: A game with 60 FPS average but 20 FPS 1% low will FEEL
 * worse than a game with 45 FPS average but 40 FPS 1% low. Consistency matters!
 */
USTRUCT(BlueprintType)
struct FPerformanceHistory
{
    GENERATED_BODY()

    // -------------------------------------------------------------------------
    // RAW HISTORY DATA (arrays of samples over time)
    // -------------------------------------------------------------------------

    // FPS values over time (for graphs, trend analysis)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> FPSHistory;

    // Frame time values over time
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> FrameTimeHistory;

    // GPU time values over time
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> GPUTimeHistory;

    // CPU time values over time
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> CPUTimeHistory;

    // Memory usage values over time (detect leaks)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> MemoryUsageHistory;

    // -------------------------------------------------------------------------
    // CALCULATED STATISTICS
    // -------------------------------------------------------------------------

    // Mean FPS across the recording period
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageFPS = 60.0f;

    // Lowest FPS recorded (worst moment)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinFPS = 60.0f;

    // Highest FPS recorded (best moment)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFPS = 60.0f;

    // 1% percentile FPS - average of the worst 1% of frames
    // This is THE most important metric for perceived smoothness
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OnePercentLowFPS = 0.0f;

    // 0.1% percentile FPS - average of the worst 0.1% of frames
    // Captures rare but severe stutters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PointOnePercentLowFPS = 0.0f;

    // Number of frames that missed the target frame time significantly
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FrameDropCount = 0;

    // Number of noticeable stutters (frame time > threshold)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StutterCount = 0;

    // How long we've been recording (for calculating rates)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalRecordingTimeSeconds = 0.0f;
};

/**
 * FPerformanceWarning - Alert when a performance metric crosses a threshold.
 *
 * The system generates warnings when performance drops below acceptable
 * levels. These can be displayed to developers during testing or trigger
 * automatic quality adjustments in release builds.
 *
 * Example warning: "GPU Time exceeded 20ms (currently 24ms). Consider
 * reducing shadow quality or draw distance."
 */
USTRUCT(BlueprintType)
struct FPerformanceWarning
{
    GENERATED_BODY()

    // Unique identifier for this warning
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid WarningId;

    // How severe is this warning?
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPerformanceWarningLevel Level = EPerformanceWarningLevel::Warning;

    // Which metric triggered this warning?
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPerformanceMetricType MetricType = EPerformanceMetricType::FrameRate;

    // Human-readable description of the problem
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;

    // Recommended action to fix the issue
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Suggestion;

    // The actual value that triggered the warning
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentValue = 0.0f;

    // The threshold that was exceeded
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThresholdValue = 0.0f;

    // When this warning was first generated
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    // How many times this same warning has occurred (for recurring issues)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OccurrenceCount = 1;

    // Has this warning been addressed/fixed?
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bResolved = false;
};

/**
 * FOptimizationHint - Suggested optimization action based on performance data.
 *
 * The system analyzes performance patterns and suggests specific actions
 * that could improve performance. Some hints can be applied automatically.
 *
 * Example hint: "Reduce shadow cascade count from 4 to 2. Expected
 * improvement: 3ms GPU time. Auto-applicable: Yes"
 */
USTRUCT(BlueprintType)
struct FOptimizationHint
{
    GENERATED_BODY()

    // Unique identifier for this hint
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid HintId;

    // Which area of the game this affects
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOptimizationCategory Category = EOptimizationCategory::General;

    // Short summary (e.g., "Reduce Shadow Quality")
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    // Detailed explanation of the issue and solution
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    // Console command or action to apply this hint (if auto-applicable)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ActionCommand;

    // Estimated frame time improvement in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExpectedImprovement = 0.0f;

    // Priority rank (1 = highest priority, apply first)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 5;

    // Can this hint be applied automatically without user intervention?
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoApplicable = false;

    // Has this hint already been applied?
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bApplied = false;
};

/**
 * FBenchmarkResult - Results from running a performance benchmark.
 *
 * Benchmarks run standardized tests to measure system capability.
 * Results include a numeric score and recommended quality tier.
 *
 * The Score is calculated from multiple factors (FPS, stability, etc.)
 * and can be compared across different hardware configurations.
 */
USTRUCT(BlueprintType)
struct FBenchmarkResult
{
    GENERATED_BODY()

    // Unique identifier for this benchmark run
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid BenchmarkId;

    // What type of benchmark was run
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBenchmarkType Type = EBenchmarkType::Standard;

    // When the benchmark started
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    // When the benchmark ended
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    // Total benchmark duration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DurationSeconds = 0.0f;

    // -------------------------------------------------------------------------
    // FPS STATISTICS
    // -------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageFPS = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinFPS = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFPS = 0.0f;

    // 1% low - critical for assessing real-world smoothness
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OnePercentLow = 0.0f;

    // -------------------------------------------------------------------------
    // TIMING STATISTICS
    // -------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageGPUTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageCPUTime = 0.0f;

    // -------------------------------------------------------------------------
    // RESOURCE USAGE
    // -------------------------------------------------------------------------

    // Maximum RAM usage during benchmark
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 PeakMemoryUsage = 0;

    // Maximum VRAM usage during benchmark
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 PeakVRAMUsage = 0;

    // -------------------------------------------------------------------------
    // FRAME STATISTICS
    // -------------------------------------------------------------------------

    // Total frames rendered during benchmark
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalFrames = 0;

    // Frames that missed the target significantly
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DroppedFrames = 0;

    // -------------------------------------------------------------------------
    // FINAL RESULTS
    // -------------------------------------------------------------------------

    // Numeric score (higher = better, comparable across hardware)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Score = 0;

    // Recommended quality preset based on benchmark results
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPerformanceTier RecommendedTier = EPerformanceTier::Medium;

    // Per-scene breakdown (if benchmark tests multiple scenes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> SceneScores;
};

/**
 * FPerformanceThresholds - Limits that trigger warnings when exceeded.
 *
 * These values define "acceptable" performance. When a metric crosses
 * its threshold, a warning is generated. Thresholds can be adjusted
 * based on target platform (PC vs console) or quality preset.
 *
 * For beginners: Think of these as "speed limits" - exceeding them
 * isn't immediately fatal, but it indicates a problem.
 */
USTRUCT(BlueprintType)
struct FPerformanceThresholds
{
    GENERATED_BODY()

    // The FPS we're trying to achieve (60 = standard, 120 = high refresh)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetFPS = 60.0f;

    // Below this FPS, the game is considered unplayable
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinAcceptableFPS = 30.0f;

    // Maximum acceptable frame time (33.33ms = 30 FPS minimum)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxFrameTimeMs = 33.33f;

    // GPU time limit before warning (16ms = 60 FPS GPU budget)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxGPUTimeMs = 16.0f;

    // CPU time limit before warning
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxCPUTimeMs = 16.0f;

    // Draw call limit (too many = CPU bottleneck)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxDrawCalls = 3000;

    // Triangle count limit
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MaxTriangles = 5000000;

    // Memory usage percentage that triggers warning (0.8 = 80%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryWarningThreshold = 0.8f;

    // Memory usage percentage that triggers critical alert
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryCriticalThreshold = 0.95f;

    // VRAM usage percentage that triggers warning
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VRAMWarningThreshold = 0.8f;

    // Frame time spike that counts as a "stutter" (noticeable hitch)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StutterThresholdMs = 50.0f;
};

/**
 * FPerformanceSettings - Configuration for the metrics subsystem.
 *
 * Controls how the subsystem collects data, what features are enabled,
 * and how frame pacing works. These can be adjusted at runtime.
 */
USTRUCT(BlueprintType)
struct FPerformanceSettings
{
    GENERATED_BODY()

    // Master switch for metrics collection
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableMetrics = true;

    // Whether to generate performance warnings
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableWarnings = true;

    // Whether to analyze data and generate optimization hints
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableOptimizationHints = true;

    // Automatically apply safe optimizations when performance drops
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoOptimize = false;

    // How often to sample metrics (0.1 = 10 times per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SampleIntervalSeconds = 0.1f;

    // How many samples to keep in history (600 @ 0.1s = 60 seconds of data)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HistorySampleCount = 600;

    // Write metrics to a log file (for offline analysis)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLogToFile = false;

    // Path for the metrics log file
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LogFilePath;

    // How frames are paced/limited
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFramePacingMode FramePacing = EFramePacingMode::Fixed60;

    // Custom FPS cap when FramePacing is Custom
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CustomFrameRateCap = 60;
};

/**
 * FHardwareInfo - Information about the player's computer/console.
 *
 * Detected at startup, this info helps determine appropriate quality
 * settings and is included in crash reports for debugging.
 *
 * TERMINOLOGY FOR BEGINNERS:
 * - CPU Cores: Physical processor units (more = better multitasking)
 * - CPU Threads: Logical processors (often 2x cores with hyperthreading)
 * - RHI: Render Hardware Interface (DirectX 11/12, Vulkan, Metal)
 * - Ray Tracing: Realistic lighting simulation (requires RTX/RDNA2+)
 * - Nanite: UE5's virtualized geometry system
 * - Lumen: UE5's global illumination system
 */
USTRUCT(BlueprintType)
struct FHardwareInfo
{
    GENERATED_BODY()

    // CPU model name (e.g., "AMD Ryzen 7 5800X")
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CPUBrand;

    // Number of physical CPU cores
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CPUCores = 0;

    // Number of logical threads (with hyperthreading)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CPUThreads = 0;

    // GPU model name (e.g., "NVIDIA GeForce RTX 3080")
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GPUBrand;

    // Total VRAM in megabytes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 GPUMemoryMB = 0;

    // Total system RAM in megabytes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 SystemMemoryMB = 0;

    // Operating system version
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OSVersion;

    // Graphics API being used (DX11, DX12, Vulkan, Metal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RHIName;

    // GPU driver version
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DriverVersion;

    // -------------------------------------------------------------------------
    // FEATURE SUPPORT FLAGS
    // -------------------------------------------------------------------------

    // Can use hardware ray tracing (RTX, DXR)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsRayTracing = false;

    // Can use UE5 Nanite virtualized geometry
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsNanite = false;

    // Can use UE5 Lumen global illumination
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSupportsLumen = false;
};

// ============================================================================
// DELEGATES - Event callbacks that other systems can subscribe to
// ============================================================================

/**
 * DELEGATES EXPLAINED (for beginners):
 *
 * Delegates are Unreal's way of implementing the Observer pattern.
 * Instead of constantly checking "did something happen?", you subscribe
 * a function to be called WHEN it happens.
 *
 * Example usage:
 *   MetricsSubsystem->OnPerformanceWarning.AddDynamic(this, &MyClass::HandleWarning);
 *
 *   void MyClass::HandleWarning(const FPerformanceWarning& Warning) {
 *       // React to the warning
 *   }
 */

// Fired when a performance threshold is exceeded
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceWarning, const FPerformanceWarning&, Warning);

// Fired when a new optimization suggestion is generated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptimizationHintGenerated, const FOptimizationHint&, Hint);

// Fired when a benchmark run completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBenchmarkComplete, const FBenchmarkResult&, Result);

// Fired when FPS changes significantly (for adaptive UI, etc.)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFrameRateChanged, float, OldFPS, float, NewFPS);

// Fired when the quality tier changes (auto-adjusted or user-changed)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceTierChanged, EPerformanceTier, NewTier);

// Fired when memory pressure increases (may need to free memory)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryPressure, float, PressureLevel);

// ============================================================================
// MAIN SUBSYSTEM CLASS
// ============================================================================

/**
 * UMGPerformanceMetricsSubsystem - The central performance monitoring system.
 *
 * This subsystem runs throughout the game session, continuously collecting
 * performance data, analyzing trends, generating warnings, and providing
 * optimization suggestions.
 *
 * ACCESS PATTERN:
 *   UMGPerformanceMetricsSubsystem* Metrics =
 *       GetGameInstance()->GetSubsystem<UMGPerformanceMetricsSubsystem>();
 *
 * Or in Blueprints: Get Game Instance -> Get Subsystem (select this class)
 *
 * LIFECYCLE:
 * - Created when game starts (Initialize called)
 * - Runs throughout the entire game session
 * - Survives level transitions
 * - Destroyed when game exits (Deinitialize called)
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPerformanceMetricsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Called when game instance is created - sets up timers and initial state
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Called when game instance is destroyed - cleanup
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

// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGMemoryBudgetSubsystem.h
 * Memory Budget Management System for Midnight Grind
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem manages memory "budgets" - pre-defined limits on how much
 * memory different parts of the game can use. By enforcing budgets, we prevent
 * the game from using too much memory and crashing or stuttering.
 *
 * Think of it like a household budget - you allocate specific amounts for
 * rent, food, entertainment. If one category overspends, something has to
 * give. Same with memory!
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. MEMORY POOLS (EMGMemoryPool):
 *    Memory is divided into "pools" for different purposes:
 *    - Textures: Images for surfaces (usually the biggest!)
 *    - Meshes: 3D geometry data
 *    - Audio: Sound effects, music
 *    - Animation: Character/vehicle animation data
 *    - Physics: Collision shapes, physics simulation data
 *    - Particles: Particle effect data
 *    - UI: User interface elements
 *    - Streaming: Dynamically loaded content
 *    - General: Everything else
 *
 *    Each pool has its own budget that can be monitored and enforced.
 *
 * 2. MEMORY PRESSURE (EMGMemoryPressure):
 *    A measure of how "stressed" the memory system is:
 *    - None: Plenty of free memory, no concerns
 *    - Low: Getting close to budget, monitor situation
 *    - Medium: Approaching limits, consider freeing memory
 *    - High: Near budget limits, actively free unused memory
 *    - Critical: Over budget! Must free memory immediately
 *
 * 3. WHY BUDGETS MATTER:
 *    - Consoles have fixed memory (can't add more RAM)
 *    - Running out of memory = crash
 *    - Approaching limits = stuttering as OS swaps to disk
 *    - Budgets help catch problems BEFORE they cause issues
 *
 * 4. THRESHOLD PERCENTAGES:
 *    - Warning Threshold (e.g., 80%): "We're using a lot, be careful"
 *    - Critical Threshold (e.g., 95%): "Danger! Free memory NOW"
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *    [Asset Loading] --> [Budget Check] --> [Allow/Deny Load]
 *          |                   |                   |
 *          v                   v                   v
 *    [Memory Request]    [Pool Stats]        [Load Asset]
 *          |                   |                or
 *          v                   v              [Reject/Queue]
 *    [Track Usage]       [Pressure Check]
 *                              |
 *                              v
 *                       [Trigger Cleanup if needed]
 *
 * The subsystem connects to:
 * - MGStreamingSubsystem: Checks budgets before loading, may reject loads
 * - MGMemoryManagerSubsystem: Works together for detailed memory management
 * - MGPerformanceMonitorSubsystem: Reports memory stats for monitoring
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * 1. Check if we can load something:
 *    if (!BudgetSubsystem->IsUnderPressure()) {
 *        // Safe to load more assets
 *    }
 *
 * 2. Get texture budget status:
 *    FMGMemoryPoolStats TextureStats =
 *        BudgetSubsystem->GetPoolStats(EMGMemoryPool::Textures);
 *    if (TextureStats.UsagePercent > 0.8f) {
 *        // Consider using lower resolution textures
 *    }
 *
 * 3. Force cleanup when needed:
 *    BudgetSubsystem->ForceGarbageCollection();
 *    BudgetSubsystem->PurgeUnusedAssets();
 *
 * 4. React to pressure changes:
 *    BudgetSubsystem->OnMemoryPressureChanged.AddDynamic(
 *        this, &MyClass::HandlePressureChange);
 *
 * PLATFORM CONSIDERATIONS:
 * ------------------------
 * - PC: More flexible, can use system RAM estimation
 * - Console: Fixed memory, budgets are CRITICAL
 * - Mobile: Very limited memory, aggressive budgets needed
 *
 * The subsystem can detect the platform and apply appropriate budgets.
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMemoryBudgetSubsystem.generated.h"

/**
 * EMGMemoryPool - Categories of memory usage.
 *
 * Each pool represents a different type of game data. By tracking pools
 * separately, we can identify which category is using too much memory.
 *
 * TYPICAL MEMORY DISTRIBUTION (rough estimates):
 * - Textures: 40-60% (usually the biggest consumer)
 * - Meshes: 15-25%
 * - Audio: 5-15%
 * - Streaming: 10-20% (for dynamically loaded content)
 * - Others: 10-20% combined
 */
UENUM(BlueprintType)
enum class EMGMemoryPool : uint8
{
	Textures,   // Surface images - diffuse, normal, roughness maps, etc.
	Meshes,     // 3D model vertex/index data
	Audio,      // Sound effects, music, voice lines
	Animation,  // Skeletal animation data, montages
	Physics,    // Collision shapes, physics simulation data
	Particles,  // Particle system templates and instance data
	UI,         // User interface textures, fonts, widgets
	Streaming,  // Dynamically loaded level/asset data
	General     // Everything else that doesn't fit above
};

/**
 * EMGMemoryPressure - How stressed the memory system is.
 *
 * Different pressure levels trigger different responses:
 * - None/Low: Normal operation
 * - Medium: Stop preloading optional assets
 * - High: Start unloading unused assets
 * - Critical: Aggressive cleanup, may reduce quality
 */
UENUM(BlueprintType)
enum class EMGMemoryPressure : uint8
{
	None,      // < 60% budget used, plenty of headroom
	Low,       // 60-75% budget used, monitor but no action needed
	Medium,    // 75-85% budget used, consider freeing memory
	High,      // 85-95% budget used, actively free memory
	Critical   // > 95% budget used, emergency cleanup required
};

/**
 * FMGMemoryPoolStats - Current status of a single memory pool.
 *
 * Use this to check how a specific category (e.g., Textures) is doing
 * relative to its budget. The UsagePercent is particularly useful for
 * quick checks.
 */
USTRUCT(BlueprintType)
struct FMGMemoryPoolStats
{
	GENERATED_BODY()

	// Which pool this data is for
	UPROPERTY(BlueprintReadOnly)
	EMGMemoryPool Pool = EMGMemoryPool::General;

	// Current memory usage in megabytes
	UPROPERTY(BlueprintReadOnly)
	int64 UsedMB = 0;

	// Maximum allowed memory for this pool (the "budget")
	UPROPERTY(BlueprintReadOnly)
	int64 BudgetMB = 0;

	// Highest memory usage ever recorded (for leak detection)
	// If this keeps growing, you might have a memory leak
	UPROPERTY(BlueprintReadOnly)
	int64 PeakMB = 0;

	// UsedMB / BudgetMB as a percentage (0.0 to 1.0+)
	// Values over 1.0 mean over budget!
	UPROPERTY(BlueprintReadOnly)
	float UsagePercent = 0.0f;
};

/**
 * FMGMemoryBudgetConfig - Configuration for memory budgets.
 *
 * This struct defines how much memory each pool is allowed to use.
 * Values should be tuned based on target platform:
 *
 * Example configurations:
 * - PC (16GB RAM): TotalBudget ~4096-6144 MB
 * - Console (fixed): TotalBudget based on console specs minus OS overhead
 * - Mobile: TotalBudget ~1024-2048 MB
 *
 * The thresholds determine when warnings/cleanup triggers:
 * - At 80% (Warning): Start being careful, avoid preloading
 * - At 95% (Critical): Stop all loading, aggressively free memory
 */
USTRUCT(BlueprintType)
struct FMGMemoryBudgetConfig
{
	GENERATED_BODY()

	// Total memory budget for the game (MB)
	// This should be less than actual RAM to leave room for OS, drivers, etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalBudgetMB = 4096;

	// Budget for textures (MB) - usually the largest allocation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TextureBudgetMB = 1536;

	// Budget for 3D mesh geometry (MB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MeshBudgetMB = 512;

	// Budget for audio data (MB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 AudioBudgetMB = 256;

	// Budget for streaming system (MB) - dynamically loaded content
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 StreamingBudgetMB = 1024;

	// At this usage percentage, start warning and stop optional loads
	// 0.8 = 80%
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WarningThreshold = 0.8f;

	// At this usage percentage, trigger emergency cleanup
	// 0.95 = 95%
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalThreshold = 0.95f;
};

// ============================================================================
// DELEGATES - Events for memory status changes
// ============================================================================

// Fired when memory pressure level changes (e.g., from Low to High)
// Subscribe to this to react to memory situations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMemoryPressureChanged, EMGMemoryPressure, Pressure);

// Fired when a specific pool exceeds its budget
// OverageMB tells you how much over budget the pool is
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPoolOverBudget, EMGMemoryPool, Pool, int64, OverageMB);

/**
 * UMGMemoryBudgetSubsystem - Memory budget tracking and enforcement.
 *
 * This subsystem tracks memory usage per pool, compares against budgets,
 * and provides tools for managing memory pressure.
 *
 * IMPORTANT: This is a lightweight budget TRACKING system. For full memory
 * management with streaming and loading control, see MGMemoryManagerSubsystem.
 *
 * ACCESS:
 *   auto* Budget = GetGameInstance()->GetSubsystem<UMGMemoryBudgetSubsystem>();
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMemoryBudgetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Called on game start - initializes default budgets based on platform
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Called on game end - cleanup
	virtual void Deinitialize() override;

	// =========================================================================
	// BUDGET CONFIGURATION
	// Set and get the memory budget settings
	// =========================================================================
	// Apply new budget configuration (e.g., after detecting hardware)
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void SetBudgetConfig(const FMGMemoryBudgetConfig& Config);

	// Get current budget configuration
	UFUNCTION(BlueprintPure, Category = "Memory")
	FMGMemoryBudgetConfig GetBudgetConfig() const { return BudgetConfig; }

	// =========================================================================
	// POOL STATISTICS
	// Query current memory usage per pool
	// =========================================================================
	// Get stats for a specific memory pool
	UFUNCTION(BlueprintPure, Category = "Memory")
	FMGMemoryPoolStats GetPoolStats(EMGMemoryPool Pool) const;

	// Get stats for all pools at once
	UFUNCTION(BlueprintPure, Category = "Memory")
	TArray<FMGMemoryPoolStats> GetAllPoolStats() const;

	// Get total memory used across all pools
	UFUNCTION(BlueprintPure, Category = "Memory")
	int64 GetTotalUsedMB() const;

	// Get total memory budget
	UFUNCTION(BlueprintPure, Category = "Memory")
	int64 GetTotalBudgetMB() const { return BudgetConfig.TotalBudgetMB; }

	// =========================================================================
	// PRESSURE MONITORING
	// Check overall memory health
	// =========================================================================
	// Get current pressure level (None, Low, Medium, High, Critical)
	UFUNCTION(BlueprintPure, Category = "Memory")
	EMGMemoryPressure GetMemoryPressure() const { return CurrentPressure; }

	// Quick check: should we avoid loading more assets?
	// Returns true if pressure is Medium or higher
	UFUNCTION(BlueprintPure, Category = "Memory")
	bool IsUnderPressure() const { return CurrentPressure >= EMGMemoryPressure::Medium; }

	// =========================================================================
	// MEMORY MANAGEMENT
	// Actions to free memory
	// =========================================================================
	// Request cleanup of a specific memory pool
	// The system will try to free unused assets from that pool
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void RequestMemoryCleanup(EMGMemoryPool Pool);

	// Force Unreal's garbage collector to run
	// WARNING: This can cause a frame hitch! Use sparingly.
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void ForceGarbageCollection();

	// Reduce streaming pool usage to free up specified amount of memory
	// Will unload least-recently-used streamed assets
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void TrimStreamingPool(int64 TargetFreeMB);

	// Unload all assets that aren't currently being used
	// More aggressive than TrimStreamingPool
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void PurgeUnusedAssets();

	// =========================================================================
	// QUALITY ADJUSTMENT
	// Automatic quality scaling based on memory
	// =========================================================================
	// Enable/disable automatic texture quality reduction when under pressure
	// When enabled, system will reduce texture quality to fit in budget
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void SetAutoQualityAdjustment(bool bEnabled);

	// Get recommended texture quality level based on current memory situation
	// Returns 0 (highest) to 3 (lowest) texture streaming pool setting
	UFUNCTION(BlueprintPure, Category = "Memory")
	int32 GetRecommendedTextureQuality() const;

	// =========================================================================
	// EVENTS
	// Subscribe to these for notifications about memory status
	// =========================================================================
	// Broadcast when pressure level changes - use for adaptive behavior
	UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
	FMGOnMemoryPressureChanged OnMemoryPressureChanged;

	// Broadcast when a pool exceeds its budget
	UPROPERTY(BlueprintAssignable, Category = "Memory|Events")
	FMGOnPoolOverBudget OnPoolOverBudget;

protected:
	// =========================================================================
	// INTERNAL METHODS
	// =========================================================================
	void UpdateMemoryStats();
	void CheckMemoryPressure();
	void ApplyPlatformBudgets();
	int64 GetPoolBudget(EMGMemoryPool Pool) const;

private:
	// =========================================================================
	// INTERNAL DATA
	// =========================================================================

	// Current budget configuration
	FMGMemoryBudgetConfig BudgetConfig;

	// Current stats for each memory pool
	TMap<EMGMemoryPool, FMGMemoryPoolStats> PoolStats;

	// Current pressure level
	EMGMemoryPressure CurrentPressure = EMGMemoryPressure::None;

	// Whether to automatically reduce texture quality when over budget
	bool bAutoQualityAdjustment = true;

	// Timer for periodic memory checks
	FTimerHandle MonitorTimerHandle;
};

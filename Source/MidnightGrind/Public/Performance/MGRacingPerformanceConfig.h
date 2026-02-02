// Copyright Midnight Grind. All Rights Reserved.


#pragma once
// Stage 50: Racing Performance Configuration - 60 FPS Target

/**
 * =============================================================================
 * MGRacingPerformanceConfig.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines performance configuration settings for Midnight Grind's
 * racing gameplay. Racing games are particularly demanding because they require:
 * - Consistent high frame rates (60+ FPS) for smooth controls
 * - Fast-moving visuals with many objects on screen
 * - Complex physics for vehicle handling
 * - Real-time AI for opponent racers and traffic
 *
 * WHY PERFORMANCE CONFIGURATION MATTERS:
 * --------------------------------------
 * Different players have different hardware. A gaming PC with an RTX 4090 can
 * handle ultra settings, while a Nintendo Switch needs reduced settings.
 * This system allows the game to automatically adjust quality settings to
 * maintain smooth gameplay across all platforms.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. DATA ASSET (UDataAsset):
 *    A Data Asset is a special Unreal class that holds configuration data.
 *    Unlike regular C++ constants, Data Assets can be:
 *    - Edited in the Unreal Editor without recompiling
 *    - Tweaked by designers, not just programmers
 *    - Saved as separate files for easy version control
 *    Think of it as a "settings file" that the game reads at runtime.
 *
 * 2. FRAME RATE AND FRAME TIME:
 *    - Frame Rate (FPS): How many images the game draws per second
 *    - Frame Time: How long each frame takes (16.67ms = 60 FPS)
 *    - Budget: How much time each system (physics, AI, rendering) gets per frame
 *    If all budgets exceed the frame time, the game stutters!
 *
 * 3. LOD (Level of Detail):
 *    Objects far away don't need as much detail as nearby objects.
 *    LOD systems swap high-detail models for simpler ones at distance:
 *    - LOD0: Full detail (player's car, close objects)
 *    - LOD1: Slightly reduced (medium distance)
 *    - LOD2: Low detail (far away)
 *    - LOD3: Very low detail (background)
 *    - Culled: Not rendered at all (too far to see)
 *
 * 4. PLATFORM TIERS:
 *    Hardware is categorized into performance tiers:
 *    - Low: Nintendo Switch, older mobile devices, entry-level PCs
 *    - Medium: Steam Deck, mid-range PCs
 *    - High: PS5, Xbox Series X, good gaming PCs
 *    - Ultra: High-end gaming PCs with top-tier GPUs
 *
 * PERFORMANCE BUDGET BREAKDOWN:
 * -----------------------------
 * For 60 FPS, each frame has ~16.67ms total budget:
 *
 *    |-- Game Thread (8ms) --|-- Render Thread (10ms) --|
 *    |  Physics (3ms)        |  Draw calls, materials   |
 *    |  AI (2ms)             |  Shadows, reflections    |
 *    |  Game logic           |  Post-processing         |
 *
 *    |---------------- GPU (14ms) -------------------|
 *    |  Geometry   |  Shading   |  Post FX   |  UI  |
 *
 * Note: Some work happens in parallel (CPU and GPU work simultaneously)
 *
 * WHAT EACH SETTINGS STRUCT CONTROLS:
 * -----------------------------------
 *
 * FMGVehicleRenderSettings:
 *    How cars look - reflections, shadows, damage effects, interior detail.
 *    Cars are the visual focus, so their quality is prioritized.
 *
 * FMGVFXSettings:
 *    Visual effects - tire smoke, sparks, nitrous flames, weather particles.
 *    These are "nice to have" and scale down first when performance is tight.
 *
 * FMGPhysicsSettings:
 *    How the cars feel - suspension, tire grip, collision detection.
 *    Physics must stay consistent for fair gameplay; substepping ensures accuracy.
 *
 * FMGAudioSettings:
 *    Sound effects - engine sounds, tire squeals, ambient sounds.
 *    Audio has lower CPU impact but still needs budgeting for many sound sources.
 *
 * FMGAISettings:
 *    Opponent and traffic behavior - how often AI recalculates, how many AI cars.
 *    Far-away AI can use simplified logic without players noticing.
 *
 * FMGWorldStreamingSettings:
 *    Loading content as you drive - tracks are too big to load all at once.
 *    The game loads nearby areas and unloads distant ones dynamically.
 *
 * HOW TO USE THIS SYSTEM:
 * -----------------------
 * 1. At game startup, call DetectPlatformTier() to identify hardware
 * 2. Get the appropriate profile using GetProfile(DetectedTier)
 * 3. Apply the profile's settings to game systems
 * 4. Optionally let players override with custom settings
 *
 * Example:
 *    EMGPlatformTier Tier = UMGRacingPerformanceConfig::DetectPlatformTier();
 *    const FMGPerformanceProfile& Profile = ConfigAsset->GetProfile(Tier);
 *    PhysicsSystem->ApplySettings(Profile.PhysicsSettings);
 *    VFXSystem->ApplySettings(Profile.VFXSettings);
 *
 * TUNING TIPS FOR DESIGNERS:
 * --------------------------
 * - Always test on lowest-tier target hardware
 * - If FPS drops below target, reduce settings in this order:
 *   1. VFX (particle counts, effects)
 *   2. Shadows and reflections
 *   3. LOD distances
 *   4. AI complexity (last resort - affects gameplay)
 * - Monitor frame time in milliseconds, not FPS (more precise)
 *
 * RELATED FILES:
 * --------------
 * - MGRacingPerformanceConfig.cpp: Default profile initialization
 * - MGSettingsSubsystem: Applies these settings to game systems
 * - MGVehicleRenderComponent: Uses VehicleRenderSettings
 * - MGPhysicsVehicle: Uses PhysicsSettings
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Audio/MGAudioSubsystem.h"
#include "MGRacingPerformanceConfig.generated.h"

/**
 * Platform performance tier
 */
UENUM(BlueprintType)
enum class EMGPlatformTier : uint8
{
	Low,        // Low-end PC, Switch
	Medium,     // Mid-range PC, Steam Deck
	High,       // High-end PC, PS5, Xbox Series X
	Ultra       // Enthusiast PC
};

/**
 * LOD distance settings per platform
 */
USTRUCT(BlueprintType)
struct FMGLODDistances
{
	GENERATED_BODY()

	/** Distance to LOD1 (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD1Distance = 50.0f;

	/** Distance to LOD2 (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD2Distance = 100.0f;

	/** Distance to LOD3 (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LOD3Distance = 200.0f;

	/** Distance to cull (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CullDistance = 500.0f;
};

/**
 * Vehicle rendering quality settings
 */
USTRUCT(BlueprintType)
struct FMGVehicleRenderSettings
{
	GENERATED_BODY()

	/** LOD distances for vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLODDistances LODDistances;

	/** Max vehicles with full quality materials */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxFullQualityVehicles = 4;

	/** Max vehicles rendered at any time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRenderedVehicles = 12;

	/** Enable vehicle reflections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableReflections = true;

	/** Reflection quality (0-3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReflectionQuality = 2;

	/** Enable vehicle shadows */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableShadows = true;

	/** Shadow cascade count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShadowCascades = 3;

	/** Enable damage deformation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableDamageDeformation = true;

	/** Enable wheel blur effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableWheelBlur = true;

	/** Enable interior detail when in cockpit view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableInteriorDetail = true;

	/** Material complexity level (0=simple, 3=full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialComplexity = 3;
};

/**
 * VFX quality settings
 */
USTRUCT(BlueprintType)
struct FMGVFXSettings
{
	GENERATED_BODY()

	/** Tire smoke particle count multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TireSmokeMultiplier = 1.0f;

	/** Max concurrent tire smoke emitters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTireSmokeEmitters = 16;

	/** Spark particle count multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SparkMultiplier = 1.0f;

	/** Max sparks per collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSparksPerCollision = 50;

	/** Enable nitrous visual effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableNitrousVFX = true;

	/** Enable exhaust flames/backfire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableExhaustFlames = true;

	/** Enable rain/weather particles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableWeatherVFX = true;

	/** Weather particle density multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeatherDensity = 1.0f;

	/** Enable screen effects (speed lines, blur) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableScreenEffects = true;

	/** Motion blur intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MotionBlurIntensity = 0.5f;

	/** Enable lens flare */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableLensFlare = true;
};

/**
 * Physics simulation settings
 */
USTRUCT(BlueprintType)
struct FMGPhysicsSettings
{
	GENERATED_BODY()

	/** Physics sub-stepping enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSubstepping = true;

	/** Max physics sub-steps per frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSubsteps = 4;

	/** Sub-step delta time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SubstepDeltaTime = 0.008333f; // 120 Hz

	/** Collision complexity for vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VehicleCollisionComplexity = 2; // 0=simple box, 2=convex hull, 3=per-poly

	/** Max active wheel traces per vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxWheelTraces = 4;

	/** Enable tire deformation simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableTireDeformation = true;

	/** Enable suspension compression visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSuspensionVis = true;

	/** Max physics bodies simulated simultaneously */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSimulatedBodies = 100;
};

// FMGAudioSettings - defined in Audio/MGAudioSubsystem.h

/**
 * AI performance settings
 */
USTRUCT(BlueprintType)
struct FMGAISettings
{
	GENERATED_BODY()

	/** AI update rate (Hz) - how often AI recalculates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AIUpdateRate = 30.0f;

	/** Max active AI racers with full behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxFullAIRacers = 7;

	/** Max total AI racers (including simplified) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTotalAIRacers = 11;

	/** Distance for full AI (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FullAIDistance = 300.0f;

	/** Enable AI rubber-banding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableRubberBanding = true;

	/** Enable traffic AI during races */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableTrafficAI = true;

	/** Max traffic vehicles during race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTrafficVehicles = 20;

	/** Traffic spawn distance (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrafficSpawnDistance = 500.0f;
};

/**
 * World streaming settings
 * NOTE: Renamed from FMGStreamingSettings to avoid conflict with same-named struct in MGMemoryManagerSubsystem.h
 */
USTRUCT(BlueprintType)
struct FMGWorldStreamingSettings
{
	GENERATED_BODY()

	/** Stream-in distance (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreamInDistance = 500.0f;

	/** Stream-out distance (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreamOutDistance = 600.0f;

	/** Enable async loading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableAsyncLoading = true;

	/** Priority boost for race track chunks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceTrackPriorityBoost = 2.0f;

	/** Max concurrent streaming requests */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxConcurrentStreams = 4;

	/** Texture streaming pool size (MB) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TextureStreamingPoolMB = 1024;

	/** Mesh streaming pool size (MB) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MeshStreamingPoolMB = 512;
};

/**
 * Performance budget for racing
 */
USTRUCT(BlueprintType)
struct FMGPerformanceBudget
{
	GENERATED_BODY()

	/** Target frame rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetFPS = 60.0f;

	/** Maximum acceptable frame time (ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFrameTimeMs = 16.67f;

	/** Game thread budget (ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GameThreadBudgetMs = 8.0f;

	/** Render thread budget (ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RenderThreadBudgetMs = 10.0f;

	/** GPU budget (ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GPUBudgetMs = 14.0f;

	/** Physics budget (ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhysicsBudgetMs = 3.0f;

	/** AI budget (ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AIBudgetMs = 2.0f;

	/** Max draw calls per frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxDrawCalls = 3000;

	/** Max triangles per frame (millions) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTrianglesMillion = 5.0f;

	/** Target memory usage (MB) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetMemoryMB = 4096;
};

/**
 * Complete performance profile for a platform tier
 */
USTRUCT(BlueprintType)
struct FMGPerformanceProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformTier PlatformTier = EMGPlatformTier::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPerformanceBudget Budget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGVehicleRenderSettings VehicleSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGVFXSettings VFXSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPhysicsSettings PhysicsSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGAudioSettings AudioSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGAISettings AISettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWorldStreamingSettings StreamingSettings;
};

/**
 * Data asset containing all performance profiles
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGRacingPerformanceConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Low tier profile (30 FPS, mobile/Switch) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profiles")
	FMGPerformanceProfile LowProfile;

	/** Medium tier profile (60 FPS, mid-range) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profiles")
	FMGPerformanceProfile MediumProfile;

	/** High tier profile (60 FPS, current gen) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profiles")
	FMGPerformanceProfile HighProfile;

	/** Ultra tier profile (120+ FPS, enthusiast) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Profiles")
	FMGPerformanceProfile UltraProfile;

	/** Get profile by platform tier */
	UFUNCTION(BlueprintPure, Category = "Performance")
	const FMGPerformanceProfile& GetProfile(EMGPlatformTier Tier) const
	{
		switch (Tier)
		{
			case EMGPlatformTier::Low: return LowProfile;
			case EMGPlatformTier::Medium: return MediumProfile;
			case EMGPlatformTier::High: return HighProfile;
			case EMGPlatformTier::Ultra: return UltraProfile;
			default: return HighProfile;
		}
	}

	/** Auto-detect and recommend a platform tier */
	UFUNCTION(BlueprintCallable, Category = "Performance")
	static EMGPlatformTier DetectPlatformTier();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UMGRacingPerformanceConfig();
};

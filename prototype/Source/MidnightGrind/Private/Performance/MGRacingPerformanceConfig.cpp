// Copyright Midnight Grind. All Rights Reserved.
// Stage 50: Racing Performance Configuration - 60 FPS Target

#include "Performance/MGRacingPerformanceConfig.h"
#include "HAL/PlatformMisc.h"
#include "RHI.h"

UMGRacingPerformanceConfig::UMGRacingPerformanceConfig()
{
	// ==========================================
	// LOW TIER PROFILE (30 FPS Target)
	// Mobile, Switch, Very Low-End PC
	// ==========================================
	LowProfile.PlatformTier = EMGPlatformTier::Low;
	LowProfile.ProfileName = FText::FromString(TEXT("Performance Mode"));

	// Budget
	LowProfile.Budget.TargetFPS = 30.0f;
	LowProfile.Budget.MaxFrameTimeMs = 33.33f;
	LowProfile.Budget.GameThreadBudgetMs = 15.0f;
	LowProfile.Budget.RenderThreadBudgetMs = 20.0f;
	LowProfile.Budget.GPUBudgetMs = 30.0f;
	LowProfile.Budget.PhysicsBudgetMs = 5.0f;
	LowProfile.Budget.AIBudgetMs = 3.0f;
	LowProfile.Budget.MaxDrawCalls = 1500;
	LowProfile.Budget.MaxTrianglesMillion = 1.5f;
	LowProfile.Budget.TargetMemoryMB = 2048;

	// Vehicle
	LowProfile.VehicleSettings.LODDistances.LOD1Distance = 30.0f;
	LowProfile.VehicleSettings.LODDistances.LOD2Distance = 60.0f;
	LowProfile.VehicleSettings.LODDistances.LOD3Distance = 100.0f;
	LowProfile.VehicleSettings.LODDistances.CullDistance = 250.0f;
	LowProfile.VehicleSettings.MaxFullQualityVehicles = 2;
	LowProfile.VehicleSettings.MaxRenderedVehicles = 8;
	LowProfile.VehicleSettings.bEnableReflections = false;
	LowProfile.VehicleSettings.ReflectionQuality = 0;
	LowProfile.VehicleSettings.bEnableShadows = true;
	LowProfile.VehicleSettings.ShadowCascades = 1;
	LowProfile.VehicleSettings.bEnableDamageDeformation = false;
	LowProfile.VehicleSettings.bEnableWheelBlur = false;
	LowProfile.VehicleSettings.bEnableInteriorDetail = false;
	LowProfile.VehicleSettings.MaterialComplexity = 1;

	// VFX
	LowProfile.VFXSettings.TireSmokeMultiplier = 0.3f;
	LowProfile.VFXSettings.MaxTireSmokeEmitters = 4;
	LowProfile.VFXSettings.SparkMultiplier = 0.25f;
	LowProfile.VFXSettings.MaxSparksPerCollision = 10;
	LowProfile.VFXSettings.bEnableNitrousVFX = true;
	LowProfile.VFXSettings.bEnableExhaustFlames = false;
	LowProfile.VFXSettings.bEnableWeatherVFX = false;
	LowProfile.VFXSettings.WeatherDensity = 0.0f;
	LowProfile.VFXSettings.bEnableScreenEffects = false;
	LowProfile.VFXSettings.MotionBlurIntensity = 0.0f;
	LowProfile.VFXSettings.bEnableLensFlare = false;

	// Physics
	LowProfile.PhysicsSettings.bEnableSubstepping = true;
	LowProfile.PhysicsSettings.MaxSubsteps = 2;
	LowProfile.PhysicsSettings.SubstepDeltaTime = 0.016666f; // 60 Hz
	LowProfile.PhysicsSettings.VehicleCollisionComplexity = 1;
	LowProfile.PhysicsSettings.MaxWheelTraces = 4;
	LowProfile.PhysicsSettings.bEnableTireDeformation = false;
	LowProfile.PhysicsSettings.bEnableSuspensionVis = false;
	LowProfile.PhysicsSettings.MaxSimulatedBodies = 50;

	// Audio
	LowProfile.AudioSettings.MaxEngineSounds = 4;
	LowProfile.AudioSettings.MaxTireSounds = 4;
	LowProfile.AudioSettings.MaxEnvironmentSounds = 8;
	LowProfile.AudioSettings.AudioQuality = 1;
	LowProfile.AudioSettings.bEnable3DAudio = true;
	LowProfile.AudioSettings.bEnableReverb = false;
	LowProfile.AudioSettings.AttenuationStart = 30.0f;
	LowProfile.AudioSettings.AttenuationEnd = 150.0f;

	// AI
	LowProfile.AISettings.AIUpdateRate = 15.0f;
	LowProfile.AISettings.MaxFullAIRacers = 3;
	LowProfile.AISettings.MaxTotalAIRacers = 7;
	LowProfile.AISettings.FullAIDistance = 150.0f;
	LowProfile.AISettings.bEnableRubberBanding = true;
	LowProfile.AISettings.bEnableTrafficAI = false;
	LowProfile.AISettings.MaxTrafficVehicles = 0;
	LowProfile.AISettings.TrafficSpawnDistance = 0.0f;

	// Streaming
	LowProfile.StreamingSettings.StreamInDistance = 300.0f;
	LowProfile.StreamingSettings.StreamOutDistance = 400.0f;
	LowProfile.StreamingSettings.bEnableAsyncLoading = true;
	LowProfile.StreamingSettings.RaceTrackPriorityBoost = 3.0f;
	LowProfile.StreamingSettings.MaxConcurrentStreams = 2;
	LowProfile.StreamingSettings.TextureStreamingPoolMB = 512;
	LowProfile.StreamingSettings.MeshStreamingPoolMB = 256;

	// ==========================================
	// MEDIUM TIER PROFILE (60 FPS Target)
	// Steam Deck, Mid-Range PC, Last-Gen Consoles
	// ==========================================
	MediumProfile.PlatformTier = EMGPlatformTier::Medium;
	MediumProfile.ProfileName = FText::FromString(TEXT("Balanced Mode"));

	// Budget
	MediumProfile.Budget.TargetFPS = 60.0f;
	MediumProfile.Budget.MaxFrameTimeMs = 16.67f;
	MediumProfile.Budget.GameThreadBudgetMs = 7.0f;
	MediumProfile.Budget.RenderThreadBudgetMs = 9.0f;
	MediumProfile.Budget.GPUBudgetMs = 14.0f;
	MediumProfile.Budget.PhysicsBudgetMs = 2.5f;
	MediumProfile.Budget.AIBudgetMs = 2.0f;
	MediumProfile.Budget.MaxDrawCalls = 2500;
	MediumProfile.Budget.MaxTrianglesMillion = 3.0f;
	MediumProfile.Budget.TargetMemoryMB = 4096;

	// Vehicle
	MediumProfile.VehicleSettings.LODDistances.LOD1Distance = 40.0f;
	MediumProfile.VehicleSettings.LODDistances.LOD2Distance = 80.0f;
	MediumProfile.VehicleSettings.LODDistances.LOD3Distance = 150.0f;
	MediumProfile.VehicleSettings.LODDistances.CullDistance = 400.0f;
	MediumProfile.VehicleSettings.MaxFullQualityVehicles = 3;
	MediumProfile.VehicleSettings.MaxRenderedVehicles = 10;
	MediumProfile.VehicleSettings.bEnableReflections = true;
	MediumProfile.VehicleSettings.ReflectionQuality = 1;
	MediumProfile.VehicleSettings.bEnableShadows = true;
	MediumProfile.VehicleSettings.ShadowCascades = 2;
	MediumProfile.VehicleSettings.bEnableDamageDeformation = true;
	MediumProfile.VehicleSettings.bEnableWheelBlur = true;
	MediumProfile.VehicleSettings.bEnableInteriorDetail = false;
	MediumProfile.VehicleSettings.MaterialComplexity = 2;

	// VFX
	MediumProfile.VFXSettings.TireSmokeMultiplier = 0.6f;
	MediumProfile.VFXSettings.MaxTireSmokeEmitters = 8;
	MediumProfile.VFXSettings.SparkMultiplier = 0.5f;
	MediumProfile.VFXSettings.MaxSparksPerCollision = 25;
	MediumProfile.VFXSettings.bEnableNitrousVFX = true;
	MediumProfile.VFXSettings.bEnableExhaustFlames = true;
	MediumProfile.VFXSettings.bEnableWeatherVFX = true;
	MediumProfile.VFXSettings.WeatherDensity = 0.5f;
	MediumProfile.VFXSettings.bEnableScreenEffects = true;
	MediumProfile.VFXSettings.MotionBlurIntensity = 0.3f;
	MediumProfile.VFXSettings.bEnableLensFlare = false;

	// Physics
	MediumProfile.PhysicsSettings.bEnableSubstepping = true;
	MediumProfile.PhysicsSettings.MaxSubsteps = 3;
	MediumProfile.PhysicsSettings.SubstepDeltaTime = 0.008333f; // 120 Hz
	MediumProfile.PhysicsSettings.VehicleCollisionComplexity = 2;
	MediumProfile.PhysicsSettings.MaxWheelTraces = 4;
	MediumProfile.PhysicsSettings.bEnableTireDeformation = true;
	MediumProfile.PhysicsSettings.bEnableSuspensionVis = true;
	MediumProfile.PhysicsSettings.MaxSimulatedBodies = 75;

	// Audio
	MediumProfile.AudioSettings.MaxEngineSounds = 6;
	MediumProfile.AudioSettings.MaxTireSounds = 6;
	MediumProfile.AudioSettings.MaxEnvironmentSounds = 12;
	MediumProfile.AudioSettings.AudioQuality = 2;
	MediumProfile.AudioSettings.bEnable3DAudio = true;
	MediumProfile.AudioSettings.bEnableReverb = true;
	MediumProfile.AudioSettings.AttenuationStart = 40.0f;
	MediumProfile.AudioSettings.AttenuationEnd = 175.0f;

	// AI
	MediumProfile.AISettings.AIUpdateRate = 20.0f;
	MediumProfile.AISettings.MaxFullAIRacers = 5;
	MediumProfile.AISettings.MaxTotalAIRacers = 9;
	MediumProfile.AISettings.FullAIDistance = 200.0f;
	MediumProfile.AISettings.bEnableRubberBanding = true;
	MediumProfile.AISettings.bEnableTrafficAI = true;
	MediumProfile.AISettings.MaxTrafficVehicles = 10;
	MediumProfile.AISettings.TrafficSpawnDistance = 350.0f;

	// Streaming
	MediumProfile.StreamingSettings.StreamInDistance = 400.0f;
	MediumProfile.StreamingSettings.StreamOutDistance = 500.0f;
	MediumProfile.StreamingSettings.bEnableAsyncLoading = true;
	MediumProfile.StreamingSettings.RaceTrackPriorityBoost = 2.5f;
	MediumProfile.StreamingSettings.MaxConcurrentStreams = 3;
	MediumProfile.StreamingSettings.TextureStreamingPoolMB = 768;
	MediumProfile.StreamingSettings.MeshStreamingPoolMB = 384;

	// ==========================================
	// HIGH TIER PROFILE (60 FPS Target - Primary)
	// PS5, Xbox Series X, High-End PC
	// ==========================================
	HighProfile.PlatformTier = EMGPlatformTier::High;
	HighProfile.ProfileName = FText::FromString(TEXT("Quality Mode"));

	// Budget - 60 FPS primary target
	HighProfile.Budget.TargetFPS = 60.0f;
	HighProfile.Budget.MaxFrameTimeMs = 16.67f;
	HighProfile.Budget.GameThreadBudgetMs = 8.0f;
	HighProfile.Budget.RenderThreadBudgetMs = 10.0f;
	HighProfile.Budget.GPUBudgetMs = 14.0f;
	HighProfile.Budget.PhysicsBudgetMs = 3.0f;
	HighProfile.Budget.AIBudgetMs = 2.0f;
	HighProfile.Budget.MaxDrawCalls = 3500;
	HighProfile.Budget.MaxTrianglesMillion = 5.0f;
	HighProfile.Budget.TargetMemoryMB = 6144;

	// Vehicle
	HighProfile.VehicleSettings.LODDistances.LOD1Distance = 50.0f;
	HighProfile.VehicleSettings.LODDistances.LOD2Distance = 100.0f;
	HighProfile.VehicleSettings.LODDistances.LOD3Distance = 200.0f;
	HighProfile.VehicleSettings.LODDistances.CullDistance = 500.0f;
	HighProfile.VehicleSettings.MaxFullQualityVehicles = 4;
	HighProfile.VehicleSettings.MaxRenderedVehicles = 12;
	HighProfile.VehicleSettings.bEnableReflections = true;
	HighProfile.VehicleSettings.ReflectionQuality = 2;
	HighProfile.VehicleSettings.bEnableShadows = true;
	HighProfile.VehicleSettings.ShadowCascades = 3;
	HighProfile.VehicleSettings.bEnableDamageDeformation = true;
	HighProfile.VehicleSettings.bEnableWheelBlur = true;
	HighProfile.VehicleSettings.bEnableInteriorDetail = true;
	HighProfile.VehicleSettings.MaterialComplexity = 3;

	// VFX
	HighProfile.VFXSettings.TireSmokeMultiplier = 1.0f;
	HighProfile.VFXSettings.MaxTireSmokeEmitters = 16;
	HighProfile.VFXSettings.SparkMultiplier = 1.0f;
	HighProfile.VFXSettings.MaxSparksPerCollision = 50;
	HighProfile.VFXSettings.bEnableNitrousVFX = true;
	HighProfile.VFXSettings.bEnableExhaustFlames = true;
	HighProfile.VFXSettings.bEnableWeatherVFX = true;
	HighProfile.VFXSettings.WeatherDensity = 1.0f;
	HighProfile.VFXSettings.bEnableScreenEffects = true;
	HighProfile.VFXSettings.MotionBlurIntensity = 0.5f;
	HighProfile.VFXSettings.bEnableLensFlare = true;

	// Physics
	HighProfile.PhysicsSettings.bEnableSubstepping = true;
	HighProfile.PhysicsSettings.MaxSubsteps = 4;
	HighProfile.PhysicsSettings.SubstepDeltaTime = 0.008333f; // 120 Hz
	HighProfile.PhysicsSettings.VehicleCollisionComplexity = 2;
	HighProfile.PhysicsSettings.MaxWheelTraces = 4;
	HighProfile.PhysicsSettings.bEnableTireDeformation = true;
	HighProfile.PhysicsSettings.bEnableSuspensionVis = true;
	HighProfile.PhysicsSettings.MaxSimulatedBodies = 100;

	// Audio
	HighProfile.AudioSettings.MaxEngineSounds = 8;
	HighProfile.AudioSettings.MaxTireSounds = 8;
	HighProfile.AudioSettings.MaxEnvironmentSounds = 16;
	HighProfile.AudioSettings.AudioQuality = 3;
	HighProfile.AudioSettings.bEnable3DAudio = true;
	HighProfile.AudioSettings.bEnableReverb = true;
	HighProfile.AudioSettings.AttenuationStart = 50.0f;
	HighProfile.AudioSettings.AttenuationEnd = 200.0f;

	// AI
	HighProfile.AISettings.AIUpdateRate = 30.0f;
	HighProfile.AISettings.MaxFullAIRacers = 7;
	HighProfile.AISettings.MaxTotalAIRacers = 11;
	HighProfile.AISettings.FullAIDistance = 300.0f;
	HighProfile.AISettings.bEnableRubberBanding = true;
	HighProfile.AISettings.bEnableTrafficAI = true;
	HighProfile.AISettings.MaxTrafficVehicles = 20;
	HighProfile.AISettings.TrafficSpawnDistance = 500.0f;

	// Streaming
	HighProfile.StreamingSettings.StreamInDistance = 500.0f;
	HighProfile.StreamingSettings.StreamOutDistance = 600.0f;
	HighProfile.StreamingSettings.bEnableAsyncLoading = true;
	HighProfile.StreamingSettings.RaceTrackPriorityBoost = 2.0f;
	HighProfile.StreamingSettings.MaxConcurrentStreams = 4;
	HighProfile.StreamingSettings.TextureStreamingPoolMB = 1024;
	HighProfile.StreamingSettings.MeshStreamingPoolMB = 512;

	// ==========================================
	// ULTRA TIER PROFILE (120+ FPS Target)
	// Enthusiast PC, High-refresh displays
	// ==========================================
	UltraProfile.PlatformTier = EMGPlatformTier::Ultra;
	UltraProfile.ProfileName = FText::FromString(TEXT("Ultra Mode"));

	// Budget - 120 FPS target
	UltraProfile.Budget.TargetFPS = 120.0f;
	UltraProfile.Budget.MaxFrameTimeMs = 8.33f;
	UltraProfile.Budget.GameThreadBudgetMs = 4.0f;
	UltraProfile.Budget.RenderThreadBudgetMs = 5.0f;
	UltraProfile.Budget.GPUBudgetMs = 7.0f;
	UltraProfile.Budget.PhysicsBudgetMs = 2.0f;
	UltraProfile.Budget.AIBudgetMs = 1.5f;
	UltraProfile.Budget.MaxDrawCalls = 5000;
	UltraProfile.Budget.MaxTrianglesMillion = 8.0f;
	UltraProfile.Budget.TargetMemoryMB = 12288;

	// Vehicle
	UltraProfile.VehicleSettings.LODDistances.LOD1Distance = 75.0f;
	UltraProfile.VehicleSettings.LODDistances.LOD2Distance = 150.0f;
	UltraProfile.VehicleSettings.LODDistances.LOD3Distance = 300.0f;
	UltraProfile.VehicleSettings.LODDistances.CullDistance = 800.0f;
	UltraProfile.VehicleSettings.MaxFullQualityVehicles = 6;
	UltraProfile.VehicleSettings.MaxRenderedVehicles = 16;
	UltraProfile.VehicleSettings.bEnableReflections = true;
	UltraProfile.VehicleSettings.ReflectionQuality = 3;
	UltraProfile.VehicleSettings.bEnableShadows = true;
	UltraProfile.VehicleSettings.ShadowCascades = 4;
	UltraProfile.VehicleSettings.bEnableDamageDeformation = true;
	UltraProfile.VehicleSettings.bEnableWheelBlur = true;
	UltraProfile.VehicleSettings.bEnableInteriorDetail = true;
	UltraProfile.VehicleSettings.MaterialComplexity = 3;

	// VFX
	UltraProfile.VFXSettings.TireSmokeMultiplier = 1.5f;
	UltraProfile.VFXSettings.MaxTireSmokeEmitters = 24;
	UltraProfile.VFXSettings.SparkMultiplier = 1.5f;
	UltraProfile.VFXSettings.MaxSparksPerCollision = 100;
	UltraProfile.VFXSettings.bEnableNitrousVFX = true;
	UltraProfile.VFXSettings.bEnableExhaustFlames = true;
	UltraProfile.VFXSettings.bEnableWeatherVFX = true;
	UltraProfile.VFXSettings.WeatherDensity = 1.5f;
	UltraProfile.VFXSettings.bEnableScreenEffects = true;
	UltraProfile.VFXSettings.MotionBlurIntensity = 0.6f;
	UltraProfile.VFXSettings.bEnableLensFlare = true;

	// Physics
	UltraProfile.PhysicsSettings.bEnableSubstepping = true;
	UltraProfile.PhysicsSettings.MaxSubsteps = 6;
	UltraProfile.PhysicsSettings.SubstepDeltaTime = 0.004166f; // 240 Hz
	UltraProfile.PhysicsSettings.VehicleCollisionComplexity = 3;
	UltraProfile.PhysicsSettings.MaxWheelTraces = 4;
	UltraProfile.PhysicsSettings.bEnableTireDeformation = true;
	UltraProfile.PhysicsSettings.bEnableSuspensionVis = true;
	UltraProfile.PhysicsSettings.MaxSimulatedBodies = 150;

	// Audio
	UltraProfile.AudioSettings.MaxEngineSounds = 12;
	UltraProfile.AudioSettings.MaxTireSounds = 12;
	UltraProfile.AudioSettings.MaxEnvironmentSounds = 24;
	UltraProfile.AudioSettings.AudioQuality = 3;
	UltraProfile.AudioSettings.bEnable3DAudio = true;
	UltraProfile.AudioSettings.bEnableReverb = true;
	UltraProfile.AudioSettings.AttenuationStart = 60.0f;
	UltraProfile.AudioSettings.AttenuationEnd = 250.0f;

	// AI
	UltraProfile.AISettings.AIUpdateRate = 60.0f;
	UltraProfile.AISettings.MaxFullAIRacers = 11;
	UltraProfile.AISettings.MaxTotalAIRacers = 15;
	UltraProfile.AISettings.FullAIDistance = 500.0f;
	UltraProfile.AISettings.bEnableRubberBanding = true;
	UltraProfile.AISettings.bEnableTrafficAI = true;
	UltraProfile.AISettings.MaxTrafficVehicles = 40;
	UltraProfile.AISettings.TrafficSpawnDistance = 800.0f;

	// Streaming
	UltraProfile.StreamingSettings.StreamInDistance = 800.0f;
	UltraProfile.StreamingSettings.StreamOutDistance = 1000.0f;
	UltraProfile.StreamingSettings.bEnableAsyncLoading = true;
	UltraProfile.StreamingSettings.RaceTrackPriorityBoost = 1.5f;
	UltraProfile.StreamingSettings.MaxConcurrentStreams = 8;
	UltraProfile.StreamingSettings.TextureStreamingPoolMB = 2048;
	UltraProfile.StreamingSettings.MeshStreamingPoolMB = 1024;
}

EMGPlatformTier UMGRacingPerformanceConfig::DetectPlatformTier()
{
	// Get GPU adapter info
	FString GPUBrand = GRHIAdapterName;

	// Get VRAM
	FTextureMemoryStats TextureStats;
	RHIGetTextureMemoryStats(TextureStats);
	int64 VRAM_MB = TextureStats.DedicatedVideoMemory / (1024 * 1024);

	// Get RAM
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	int64 RAM_MB = (MemStats.UsedPhysical + MemStats.AvailablePhysical) / (1024 * 1024);

	// CPU cores
	int32 CPUCores = FPlatformMisc::NumberOfCores();

	// Determine tier based on hardware
	if (VRAM_MB >= 8192 && RAM_MB >= 16384 && CPUCores >= 8)
	{
		return EMGPlatformTier::Ultra;
	}
	else if (VRAM_MB >= 4096 && RAM_MB >= 8192 && CPUCores >= 6)
	{
		return EMGPlatformTier::High;
	}
	else if (VRAM_MB >= 2048 && RAM_MB >= 4096 && CPUCores >= 4)
	{
		return EMGPlatformTier::Medium;
	}
	else
	{
		return EMGPlatformTier::Low;
	}
}

#if WITH_EDITOR
void UMGRacingPerformanceConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Validate settings when edited
	auto ValidateProfile = [](FMGPerformanceProfile& Profile)
	{
		// Ensure LOD distances are progressive
		Profile.VehicleSettings.LODDistances.LOD2Distance = FMath::Max(
			Profile.VehicleSettings.LODDistances.LOD2Distance,
			Profile.VehicleSettings.LODDistances.LOD1Distance + 10.0f);

		Profile.VehicleSettings.LODDistances.LOD3Distance = FMath::Max(
			Profile.VehicleSettings.LODDistances.LOD3Distance,
			Profile.VehicleSettings.LODDistances.LOD2Distance + 10.0f);

		Profile.VehicleSettings.LODDistances.CullDistance = FMath::Max(
			Profile.VehicleSettings.LODDistances.CullDistance,
			Profile.VehicleSettings.LODDistances.LOD3Distance + 50.0f);

		// Ensure reasonable limits
		Profile.AISettings.MaxTotalAIRacers = FMath::Max(
			Profile.AISettings.MaxTotalAIRacers,
			Profile.AISettings.MaxFullAIRacers);

		Profile.VehicleSettings.MaxRenderedVehicles = FMath::Max(
			Profile.VehicleSettings.MaxRenderedVehicles,
			Profile.VehicleSettings.MaxFullQualityVehicles);
	};

	ValidateProfile(LowProfile);
	ValidateProfile(MediumProfile);
	ValidateProfile(HighProfile);
	ValidateProfile(UltraProfile);
}
#endif

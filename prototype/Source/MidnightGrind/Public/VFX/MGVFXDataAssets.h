// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "VFX/MGVFXSubsystem.h"
#include "VFX/MGVehicleVFXComponent.h"
#include "VFX/MGEnvironmentVFXManager.h"
#include "VFX/MGCameraVFXComponent.h"
#include "MGVFXDataAssets.generated.h"

/**
 * Vehicle VFX Preset Data Asset
 * Configure all VFX for a vehicle type
 *
 * Create in Editor: Right-click > Miscellaneous > Data Asset > MGVehicleVFXPresetData
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleVFXPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX")
	FName PresetName;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX", meta = (MultiLine = true))
	FText Description;

	// ==========================================
	// TIRE SYSTEMS
	// ==========================================

	/** Tire smoke system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Tires")
	UNiagaraSystem* TireSmokeSystem = nullptr;

	/** Skidmark ribbon system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Tires")
	UNiagaraSystem* SkidmarkSystem = nullptr;

	/** Drift trail system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Tires")
	UNiagaraSystem* DriftTrailSystem = nullptr;

	/** Burnout smoke system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Tires")
	UNiagaraSystem* BurnoutSmokeSystem = nullptr;

	/** Default drift trail color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Tires")
	FLinearColor DefaultDriftColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	// ==========================================
	// EXHAUST SYSTEMS
	// ==========================================

	/** Exhaust flame system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Exhaust")
	UNiagaraSystem* ExhaustFlameSystem = nullptr;

	/** Backfire system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Exhaust")
	UNiagaraSystem* BackfireSystem = nullptr;

	/** NOS flame system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Exhaust")
	UNiagaraSystem* NOSFlameSystem = nullptr;

	/** NOS trail system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Exhaust")
	UNiagaraSystem* NOSTrailSystem = nullptr;

	/** Exhaust configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Exhaust")
	TArray<FMGExhaustConfig> ExhaustConfigs;

	// ==========================================
	// DAMAGE SYSTEMS
	// ==========================================

	/** Collision sparks system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Damage")
	UNiagaraSystem* CollisionSparksSystem = nullptr;

	/** Scrape sparks system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Damage")
	UNiagaraSystem* ScrapeSparksSystem = nullptr;

	/** Debris particles system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Damage")
	UNiagaraSystem* DebrisSystem = nullptr;

	/** Engine smoke system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Damage")
	UNiagaraSystem* EngineSmokeSystem = nullptr;

	/** Engine fire system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Damage")
	UNiagaraSystem* EngineFireSystem = nullptr;

	// ==========================================
	// ENVIRONMENT INTERACTION
	// ==========================================

	/** Puddle splash system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Environment")
	UNiagaraSystem* PuddleSplashSystem = nullptr;

	/** Dust cloud system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Environment")
	UNiagaraSystem* DustCloudSystem = nullptr;

	/** Debris scatter system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Environment")
	UNiagaraSystem* DebrisScatterSystem = nullptr;

	/** Rain interaction system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Environment")
	UNiagaraSystem* RainInteractionSystem = nullptr;

	// ==========================================
	// SPEED EFFECTS
	// ==========================================

	/** Speed lines system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Speed")
	UNiagaraSystem* SpeedLinesSystem = nullptr;

	/** Heat distortion system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Speed")
	UNiagaraSystem* HeatDistortionSystem = nullptr;

	/** Wind particles system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Speed")
	UNiagaraSystem* WindParticlesSystem = nullptr;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Wheel socket names */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Config")
	TArray<FName> WheelSocketNames;

	/** Engine socket name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Config")
	FName EngineSocketName = FName("Engine");

	/** Speed effects threshold (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Config")
	float SpeedEffectsThreshold = 120.0f;

	/** Tire smoke slip threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Config")
	float TireSmokeSlipThreshold = 0.3f;

	/** Drift trail minimum angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle VFX|Config")
	float DriftTrailMinAngle = 15.0f;

	/** Apply this preset to a component */
	UFUNCTION(BlueprintCallable, Category = "Vehicle VFX")
	void ApplyToComponent(UMGVehicleVFXComponent* Component) const;
};

/**
 * Weather VFX Preset Data Asset
 * Configure VFX for a weather type
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGWeatherVFXPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
	FName PresetName;

	/** Weather type this preset is for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
	EMGWeatherType WeatherType = EMGWeatherType::Clear;

	/** Weather configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
	FMGWeatherConfig WeatherConfig;

	// ==========================================
	// PARTICLE SYSTEMS
	// ==========================================

	/** Rain particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|Systems")
	UNiagaraSystem* RainSystem = nullptr;

	/** Heavy rain system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|Systems")
	UNiagaraSystem* HeavyRainSystem = nullptr;

	/** Rain ripples system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|Systems")
	UNiagaraSystem* RainRipplesSystem = nullptr;

	/** Fog particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|Systems")
	UNiagaraSystem* FogSystem = nullptr;

	/** Lightning system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|Systems")
	UNiagaraSystem* LightningSystem = nullptr;

	/** Storm debris system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|Systems")
	UNiagaraSystem* StormDebrisSystem = nullptr;

	/** Heat shimmer system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|Systems")
	UNiagaraSystem* HeatShimmerSystem = nullptr;

	// ==========================================
	// POST PROCESS
	// ==========================================

	/** Post-process saturation adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|PostProcess")
	float SaturationAdjustment = 0.0f;

	/** Post-process color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX|PostProcess")
	FLinearColor ColorTint = FLinearColor::White;

	/** Apply this preset to environment manager */
	UFUNCTION(BlueprintCallable, Category = "Weather VFX")
	void ApplyToEnvironmentManager(AMGEnvironmentVFXManager* Manager) const;
};

/**
 * Zone VFX Preset Data Asset
 * Configure ambient VFX for an environment zone
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGZoneVFXPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX")
	FName PresetName;

	/** Zone type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX")
	EMGEnvironmentZone ZoneType = EMGEnvironmentZone::Downtown;

	/** Zone particle configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX")
	FMGZoneParticleConfig ParticleConfig;

	// ==========================================
	// AMBIENT SYSTEMS (per time of day)
	// ==========================================

	/** Day ambient particles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX|Day")
	UNiagaraSystem* DayAmbientSystem = nullptr;

	/** Night ambient particles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX|Night")
	UNiagaraSystem* NightAmbientSystem = nullptr;

	/** Night secondary particles (neon glow, fireflies, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX|Night")
	UNiagaraSystem* NightSecondarySystem = nullptr;

	/** Rain ambient particles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX|Weather")
	UNiagaraSystem* RainAmbientSystem = nullptr;

	// ==========================================
	// ZONE PROPERTIES
	// ==========================================

	/** Particle density multiplier for this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX|Config")
	float ParticleDensityMultiplier = 1.0f;

	/** Zone ambient color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX|Config")
	FLinearColor AmbientColor = FLinearColor::White;

	/** Neon intensity in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone VFX|Config")
	float NeonIntensity = 1.0f;

	/** Register this zone with environment manager */
	UFUNCTION(BlueprintCallable, Category = "Zone VFX")
	void RegisterWithEnvironmentManager(AMGEnvironmentVFXManager* Manager) const;
};

/**
 * Camera VFX Preset Data Asset
 * Configure camera effects for different scenarios
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGCameraVFXPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX")
	FName PresetName;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX")
	FText DisplayName;

	// ==========================================
	// SPEED EFFECTS
	// ==========================================

	/** Speed effect configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX|Speed")
	FMGSpeedEffectConfig SpeedEffectConfig;

	// ==========================================
	// DRIFT EFFECTS
	// ==========================================

	/** Drift camera configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX|Drift")
	FMGDriftCameraConfig DriftCameraConfig;

	// ==========================================
	// SHAKE SETTINGS
	// ==========================================

	/** Global shake intensity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX|Shake")
	float ShakeIntensityMultiplier = 1.0f;

	/** Light shake intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX|Shake")
	float LightShakeIntensity = 0.3f;

	/** Medium shake intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX|Shake")
	float MediumShakeIntensity = 0.6f;

	/** Heavy shake intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX|Shake")
	float HeavyShakeIntensity = 1.0f;

	// ==========================================
	// IMPACT EFFECTS
	// ==========================================

	/** Default impact flash config */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX|Impact")
	FMGImpactFlashConfig DefaultImpactFlash;

	// ==========================================
	// FOV
	// ==========================================

	/** Base FOV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera VFX|FOV")
	float BaseFOV = 90.0f;

	/** Apply this preset to camera component */
	UFUNCTION(BlueprintCallable, Category = "Camera VFX")
	void ApplyToComponent(UMGCameraVFXComponent* Component) const;
};

/**
 * Event VFX Preset Data Asset
 * Configure VFX triggered by race events
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGEventVFXPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event VFX")
	FName PresetName;

	// ==========================================
	// EVENT SYSTEMS
	// ==========================================

	/** Map of event types to Niagara systems */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event VFX")
	TMap<EMGVFXEvent, UNiagaraSystem*> EventSystems;

	/** Map of event types to priorities */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event VFX")
	TMap<EMGVFXEvent, int32> EventPriorities;

	// ==========================================
	// SPECIAL EVENTS
	// ==========================================

	/** Finish line celebration system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event VFX|Special")
	UNiagaraSystem* FinishLineCelebrationSystem = nullptr;

	/** New record celebration system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event VFX|Special")
	UNiagaraSystem* NewRecordSystem = nullptr;

	/** Perfect start system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event VFX|Special")
	UNiagaraSystem* PerfectStartSystem = nullptr;

	/** Near miss reward system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event VFX|Special")
	UNiagaraSystem* NearMissRewardSystem = nullptr;

	/** Register all events with VFX subsystem */
	UFUNCTION(BlueprintCallable, Category = "Event VFX")
	void RegisterWithVFXSubsystem(UMGVFXSubsystem* Subsystem) const;
};

/**
 * Master VFX Config Data Asset
 * Top-level configuration combining all VFX settings
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVFXConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Config name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
	FName ConfigName;

	// ==========================================
	// VEHICLE PRESETS
	// ==========================================

	/** Default vehicle VFX preset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Vehicle")
	UMGVehicleVFXPresetData* DefaultVehiclePreset = nullptr;

	/** Vehicle presets by class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Vehicle")
	TMap<FName, UMGVehicleVFXPresetData*> VehiclePresetsByClass;

	// ==========================================
	// WEATHER PRESETS
	// ==========================================

	/** Weather presets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Weather")
	TMap<EMGWeatherType, UMGWeatherVFXPresetData*> WeatherPresets;

	// ==========================================
	// ZONE PRESETS
	// ==========================================

	/** Zone presets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Zones")
	TArray<UMGZoneVFXPresetData*> ZonePresets;

	// ==========================================
	// CAMERA PRESET
	// ==========================================

	/** Default camera VFX preset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Camera")
	UMGCameraVFXPresetData* DefaultCameraPreset = nullptr;

	// ==========================================
	// EVENT PRESETS
	// ==========================================

	/** Event VFX preset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Events")
	UMGEventVFXPresetData* EventPreset = nullptr;

	// ==========================================
	// GLOBAL SETTINGS
	// ==========================================

	/** Default VFX quality */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Global")
	EMGVFXQuality DefaultQuality = EMGVFXQuality::High;

	/** Maximum pooled instances per system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Global")
	int32 MaxPooledPerSystem = 10;

	/** Maximum active VFX at once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Global")
	int32 MaxActiveVFX = 100;

	/** Material parameter collection for global params */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config|Global")
	UMaterialParameterCollection* GlobalParamCollection = nullptr;

	// ==========================================
	// HELPERS
	// ==========================================

	/** Get vehicle preset for class */
	UFUNCTION(BlueprintPure, Category = "VFX Config")
	UMGVehicleVFXPresetData* GetVehiclePresetForClass(FName VehicleClass) const;

	/** Get weather preset */
	UFUNCTION(BlueprintPure, Category = "VFX Config")
	UMGWeatherVFXPresetData* GetWeatherPreset(EMGWeatherType Weather) const;

	/** Get zone preset */
	UFUNCTION(BlueprintPure, Category = "VFX Config")
	UMGZoneVFXPresetData* GetZonePreset(EMGEnvironmentZone Zone) const;
};

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "MGEnvironmentVFXManager.generated.h"

class UMGVFXSubsystem;
class UNiagaraSystem;
class UNiagaraComponent;
class UPostProcessComponent;
class UExponentialHeightFogComponent;

/**
 * Weather type
 */
UENUM(BlueprintType)
enum class EMGWeatherType : uint8
{
	Clear,
	Overcast,
	LightRain,
	HeavyRain,
	Storm,
	Fog,
	Heat
};

/**
 * Time of day period
 */
UENUM(BlueprintType)
enum class EMGTimeOfDay : uint8
{
	Dawn,
	Morning,
	Noon,
	Afternoon,
	Sunset,
	Dusk,
	Night,
	Midnight
};

/**
 * Environment zone type (affects ambient particles)
 */
UENUM(BlueprintType)
enum class EMGEnvironmentZone : uint8
{
	Downtown,     // Neon, dense traffic, tall buildings
	Industrial,   // Steam, smoke, sparks
	Waterfront,   // Sea spray, seagulls, fog
	Residential,  // Quieter, leaves, fireflies at night
	Highway,      // Wind, minimal particles
	Tunnel,       // Enclosed, dust, lights
	Underground   // Racing beneath city
};

/**
 * Weather configuration
 */
USTRUCT(BlueprintType)
struct FMGWeatherConfig
{
	GENERATED_BODY()

	/** Weather type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType WeatherType = EMGWeatherType::Clear;

	/** Rain intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RainIntensity = 0.0f;

	/** Fog density */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogDensity = 0.0f;

	/** Wind strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindStrength = 0.0f;

	/** Wind direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

	/** Lightning frequency (per minute) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LightningFrequency = 0.0f;

	/** Wet surface amount (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WetSurfaces = 0.0f;
};

/**
 * Ambient particle configuration per zone
 */
USTRUCT(BlueprintType)
struct FMGZoneParticleConfig
{
	GENERATED_BODY()

	/** Zone type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEnvironmentZone Zone = EMGEnvironmentZone::Downtown;

	/** Primary ambient particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* AmbientParticles = nullptr;

	/** Secondary particles (e.g., fireflies at night) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* SecondaryParticles = nullptr;

	/** Particle density multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DensityMultiplier = 1.0f;

	/** Zone-specific color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ZoneTint = FLinearColor::White;
};

/**
 * Environment VFX Manager
 * Controls atmospheric and environmental visual effects
 *
 * Features:
 * - Dynamic weather system with transitions
 * - Time-of-day lighting synchronization
 * - Zone-based ambient particles
 * - Street-level atmosphere (steam, trash, leaves)
 * - Neon glow and light reflection effects
 * - Dynamic fog and volumetric effects
 */
UCLASS()
class MIDNIGHTGRIND_API AMGEnvironmentVFXManager : public AActor
{
	GENERATED_BODY()

public:
	AMGEnvironmentVFXManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ==========================================
	// WEATHER CONTROL
	// ==========================================

	/** Set weather immediately */
	UFUNCTION(BlueprintCallable, Category = "Environment|Weather")
	void SetWeather(EMGWeatherType Weather);

	/** Transition to weather over time */
	UFUNCTION(BlueprintCallable, Category = "Environment|Weather")
	void TransitionToWeather(EMGWeatherType Weather, float TransitionTime = 10.0f);

	/** Get current weather */
	UFUNCTION(BlueprintPure, Category = "Environment|Weather")
	EMGWeatherType GetCurrentWeather() const { return CurrentWeatherConfig.WeatherType; }

	/** Set weather configuration directly */
	UFUNCTION(BlueprintCallable, Category = "Environment|Weather")
	void SetWeatherConfig(const FMGWeatherConfig& Config);

	/** Get weather configuration */
	UFUNCTION(BlueprintPure, Category = "Environment|Weather")
	FMGWeatherConfig GetWeatherConfig() const { return CurrentWeatherConfig; }

	/** Trigger lightning flash */
	UFUNCTION(BlueprintCallable, Category = "Environment|Weather")
	void TriggerLightning();

	// ==========================================
	// TIME OF DAY
	// ==========================================

	/** Set time of day (0-24) */
	UFUNCTION(BlueprintCallable, Category = "Environment|Time")
	void SetTimeOfDay(float Hour);

	/** Get current hour */
	UFUNCTION(BlueprintPure, Category = "Environment|Time")
	float GetTimeOfDay() const { return CurrentHour; }

	/** Get time period */
	UFUNCTION(BlueprintPure, Category = "Environment|Time")
	EMGTimeOfDay GetTimePeriod() const;

	/** Set time progression speed (1 = real time) */
	UFUNCTION(BlueprintCallable, Category = "Environment|Time")
	void SetTimeSpeed(float Speed);

	/** Pause/resume time progression */
	UFUNCTION(BlueprintCallable, Category = "Environment|Time")
	void SetTimePaused(bool bPaused);

	// ==========================================
	// ENVIRONMENT ZONES
	// ==========================================

	/** Enter a zone */
	UFUNCTION(BlueprintCallable, Category = "Environment|Zones")
	void EnterZone(EMGEnvironmentZone Zone);

	/** Get current zone */
	UFUNCTION(BlueprintPure, Category = "Environment|Zones")
	EMGEnvironmentZone GetCurrentZone() const { return CurrentZone; }

	/** Register zone particle configuration */
	UFUNCTION(BlueprintCallable, Category = "Environment|Zones")
	void RegisterZoneConfig(const FMGZoneParticleConfig& Config);

	// ==========================================
	// AMBIENT EFFECTS
	// ==========================================

	/** Spawn steam vent effect */
	UFUNCTION(BlueprintCallable, Category = "Environment|Ambient")
	void SpawnSteamVent(FVector Location, float Intensity = 1.0f);

	/** Spawn street trash/debris */
	UFUNCTION(BlueprintCallable, Category = "Environment|Ambient")
	void SpawnStreetDebris(FVector Location, FVector WindDirection);

	/** Spawn neon reflection puddle */
	UFUNCTION(BlueprintCallable, Category = "Environment|Ambient")
	void SpawnPuddleReflection(FVector Location, FLinearColor NeonColor);

	/** Enable/disable global ambient particles */
	UFUNCTION(BlueprintCallable, Category = "Environment|Ambient")
	void SetAmbientParticlesEnabled(bool bEnabled);

	// ==========================================
	// CITY LIGHTS
	// ==========================================

	/** Set overall city light intensity */
	UFUNCTION(BlueprintCallable, Category = "Environment|Lights")
	void SetCityLightIntensity(float Intensity);

	/** Set neon glow intensity */
	UFUNCTION(BlueprintCallable, Category = "Environment|Lights")
	void SetNeonGlowIntensity(float Intensity);

	/** Trigger neon sign flicker */
	UFUNCTION(BlueprintCallable, Category = "Environment|Lights")
	void TriggerNeonFlicker(FVector Location, float Duration = 0.5f);

	// ==========================================
	// EVENTS
	// ==========================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, EMGWeatherType, NewWeather);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, EMGTimeOfDay, NewPeriod);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZoneChanged, EMGEnvironmentZone, NewZone);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLightningStrike);

	UPROPERTY(BlueprintAssignable, Category = "Environment|Events")
	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "Environment|Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Environment|Events")
	FOnZoneChanged OnZoneChanged;

	UPROPERTY(BlueprintAssignable, Category = "Environment|Events")
	FOnLightningStrike OnLightningStrike;

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPostProcessComponent* PostProcessComponent;

	// ==========================================
	// WEATHER SYSTEMS
	// ==========================================

	/** Rain particle system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Weather")
	UNiagaraSystem* RainSystem = nullptr;

	/** Heavy rain system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Weather")
	UNiagaraSystem* HeavyRainSystem = nullptr;

	/** Rain ripples on surfaces */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Weather")
	UNiagaraSystem* RainRipplesSystem = nullptr;

	/** Fog particles */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Weather")
	UNiagaraSystem* FogSystem = nullptr;

	/** Lightning flash system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Weather")
	UNiagaraSystem* LightningSystem = nullptr;

	/** Storm wind debris */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Weather")
	UNiagaraSystem* StormDebrisSystem = nullptr;

	/** Heat shimmer system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Weather")
	UNiagaraSystem* HeatShimmerSystem = nullptr;

	// ==========================================
	// AMBIENT SYSTEMS
	// ==========================================

	/** City dust particles */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* CityDustSystem = nullptr;

	/** Steam vent system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* SteamVentSystem = nullptr;

	/** Street trash particles */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* StreetTrashSystem = nullptr;

	/** Falling leaves system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* FallingLeavesSystem = nullptr;

	/** Fireflies system (night) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* FirefliesSystem = nullptr;

	/** Neon glow particles */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* NeonGlowSystem = nullptr;

	/** Light bloom particles */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* LightBloomSystem = nullptr;

	/** Industrial sparks */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* IndustrialSparksSystem = nullptr;

	/** Seagulls/birds */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Ambient")
	UNiagaraSystem* BirdsSystem = nullptr;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Weather transition speed */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float DefaultWeatherTransitionTime = 10.0f;

	/** Zone particle configs */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TArray<FMGZoneParticleConfig> ZoneConfigs;

	/** Time speed (1 = real time, 60 = 1 hour per minute) */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float TimeSpeed = 1.0f;

	/** Global ambient particle density */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AmbientParticleDensity = 1.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current weather configuration */
	FMGWeatherConfig CurrentWeatherConfig;

	/** Target weather configuration (for transitions) */
	FMGWeatherConfig TargetWeatherConfig;

	/** Weather transition progress */
	float WeatherTransitionAlpha = 1.0f;
	float WeatherTransitionDuration = 0.0f;

	/** Current hour (0-24) */
	float CurrentHour = 22.0f; // Default to night

	/** Is time paused */
	bool bTimePaused = true;

	/** Previous time period (for change detection) */
	EMGTimeOfDay PreviousTimePeriod = EMGTimeOfDay::Night;

	/** Current zone */
	EMGEnvironmentZone CurrentZone = EMGEnvironmentZone::Downtown;

	/** City light intensity */
	float CityLightIntensity = 1.0f;

	/** Neon glow intensity */
	float NeonGlowIntensity = 1.0f;

	/** Ambient particles enabled */
	bool bAmbientParticlesEnabled = true;

	/** Lightning timer */
	float LightningTimer = 0.0f;

	// ==========================================
	// ACTIVE COMPONENTS
	// ==========================================

	UPROPERTY()
	UNiagaraComponent* RainComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* RainRipplesComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* FogComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* StormDebrisComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* HeatShimmerComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* AmbientParticlesComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* SecondaryAmbientComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* NeonGlowComp = nullptr;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Get VFX subsystem */
	UMGVFXSubsystem* GetVFXSubsystem() const;

	/** Update weather effects */
	void UpdateWeather(float DeltaTime);

	/** Update time of day */
	void UpdateTimeOfDay(float DeltaTime);

	/** Update ambient particles for current zone */
	void UpdateAmbientParticles();

	/** Get default config for weather type */
	FMGWeatherConfig GetDefaultWeatherConfig(EMGWeatherType Weather) const;

	/** Interpolate between weather configs */
	FMGWeatherConfig LerpWeatherConfig(const FMGWeatherConfig& A, const FMGWeatherConfig& B, float Alpha) const;

	/** Apply weather config to active components */
	void ApplyWeatherConfig(const FMGWeatherConfig& Config);

	/** Update lighting based on time of day */
	void UpdateLighting();

	/** Update post-process based on time and weather */
	void UpdatePostProcess();

	/** Spawn component attached to this actor */
	UNiagaraComponent* SpawnManagedNiagara(UNiagaraSystem* System);
};

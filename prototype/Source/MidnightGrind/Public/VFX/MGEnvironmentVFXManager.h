// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGEnvironmentVFXManager.h
 * @brief Manages atmospheric and environmental visual effects for the game world
 *
 * @section Overview
 * AMGEnvironmentVFXManager is an Actor that controls all environmental visual effects
 * including weather systems, time-of-day lighting, zone-based ambient particles, and
 * city atmosphere. It creates a living, breathing urban environment that responds to
 * game state and enhances immersion.
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Weather System**
 * The weather system simulates different atmospheric conditions that affect both
 * visuals and gameplay:
 * - Clear: No precipitation, normal visibility
 * - Overcast: Cloudy skies, diffused lighting
 * - LightRain/HeavyRain: Varying rain intensity with wet surfaces
 * - Storm: Heavy rain plus lightning and wind debris
 * - Fog: Reduced visibility with volumetric fog effects
 * - Heat: Heat shimmer distortion effects
 *
 * Weather transitions happen smoothly over time using interpolation between
 * configurations (FMGWeatherConfig structs).
 *
 * **Time of Day**
 * The manager tracks a 24-hour clock that affects lighting and ambient particles:
 * - Dawn/Dusk: Golden hour lighting with warm tones
 * - Day (Morning/Noon/Afternoon): Bright, high-contrast lighting
 * - Night (Sunset/Dusk/Night/Midnight): Dark with artificial lighting emphasis
 *
 * Time can progress in real-time or be accelerated/paused for gameplay purposes.
 *
 * **Environment Zones**
 * Different areas of the city have unique ambient particle systems:
 * - Downtown: Neon glow, dense atmosphere, light bloom
 * - Industrial: Steam vents, smoke, sparks from machinery
 * - Waterfront: Sea spray, birds, coastal fog
 * - Residential: Quieter, falling leaves, fireflies at night
 * - Highway: Wind particles, minimal ambient effects
 * - Tunnel/Underground: Dust, enclosed atmosphere
 *
 * **Niagara Integration**
 * All particle effects use Unreal's Niagara system. The manager spawns and controls
 * UNiagaraComponent instances for each active effect, managing their lifecycle and
 * parameters based on current conditions.
 *
 * @section Architecture
 * The manager is a world-placed Actor that:
 * 1. Maintains current weather, time, and zone state
 * 2. Updates active Niagara components each tick based on state
 * 3. Manages smooth transitions between states using interpolation
 * 4. Broadcasts events when significant changes occur (weather change, zone entry)
 * 5. Synchronizes with post-processing for screen-space effects
 *
 * Place one instance of this actor in each level. It can be accessed via:
 * - Direct reference (level Blueprint)
 * - Finding by class: GetWorld()->SpawnActor or FindActorOfClass
 *
 * @section UsageExamples Usage Examples
 *
 * **Setting Up Weather in Level Blueprint:**
 * @code
 * // Get reference to the environment manager in the level
 * AMGEnvironmentVFXManager* EnvManager = Cast<AMGEnvironmentVFXManager>(
 *     UGameplayStatics::GetActorOfClass(GetWorld(), AMGEnvironmentVFXManager::StaticClass()));
 *
 * // Start a rainy night race
 * EnvManager->SetTimeOfDay(22.0f);  // 10 PM
 * EnvManager->TransitionToWeather(EMGWeatherType::HeavyRain, 5.0f);  // 5 second transition
 * @endcode
 *
 * **Responding to Weather Changes:**
 * @code
 * // In your game mode or vehicle class
 * void AMyGameMode::BeginPlay()
 * {
 *     Super::BeginPlay();
 *
 *     // Find and bind to environment manager
 *     EnvManager = Cast<AMGEnvironmentVFXManager>(
 *         UGameplayStatics::GetActorOfClass(GetWorld(), AMGEnvironmentVFXManager::StaticClass()));
 *
 *     if (EnvManager)
 *     {
 *         EnvManager->OnWeatherChanged.AddDynamic(this, &AMyGameMode::HandleWeatherChange);
 *     }
 * }
 *
 * void AMyGameMode::HandleWeatherChange(EMGWeatherType NewWeather)
 * {
 *     // Adjust vehicle handling for wet conditions
 *     if (NewWeather == EMGWeatherType::HeavyRain || NewWeather == EMGWeatherType::Storm)
 *     {
 *         NotifyAllVehicles_WetConditions(true);
 *     }
 * }
 * @endcode
 *
 * **Zone-Based Ambient Effects:**
 * @code
 * // When player enters a new area (via trigger volume or track section)
 * void ATrackSection::OnVehicleEnter(AActor* Vehicle)
 * {
 *     if (Vehicle->IsA<APlayerVehicle>())
 *     {
 *         AMGEnvironmentVFXManager* EnvManager = GetEnvManager();
 *         EnvManager->EnterZone(ZoneType);  // e.g., EMGEnvironmentZone::Industrial
 *     }
 * }
 * @endcode
 *
 * **Triggering Atmospheric Events:**
 * @code
 * // Manual lightning strike for dramatic moment
 * void ATriggerVolume::OnDramaticMoment()
 * {
 *     EnvManager->TriggerLightning();
 *
 *     // Spawn steam vent for cinematic effect
 *     EnvManager->SpawnSteamVent(GetActorLocation(), 1.5f);
 * }
 * @endcode
 *
 * **Blueprint Usage:**
 * 1. Place AMGEnvironmentVFXManager in your level
 * 2. Configure default weather/time in the Details panel
 * 3. Assign Niagara systems for each weather type
 * 4. Call "Transition To Weather" or "Set Time Of Day" from gameplay events
 * 5. Bind to OnWeatherChanged/OnZoneChanged events for gameplay responses
 *
 * @see UMGVFXSubsystem For global VFX spawning and pooling
 * @see UMGWeatherVFXPresetData For configuring weather VFX presets
 * @see UMGZoneVFXPresetData For configuring zone-specific effects
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Core/MGCoreEnums.h"
#include "MGEnvironmentVFXManager.generated.h"

class UMGVFXSubsystem;
class UNiagaraSystem;
class UNiagaraComponent;
class UPostProcessComponent;
class UExponentialHeightFogComponent;

// EMGWeatherType and EMGTimeOfDay defined in Core/MGCoreEnums.h

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

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGWeatherSubsystem.generated.h"

class UMaterialParameterCollection;
class UNiagaraSystem;
class UPostProcessComponent;
class ADirectionalLight;
class ASkyAtmosphere;
class AVolumetricCloud;

/**
 * Weather type
 */
UENUM(BlueprintType)
enum class EMGWeatherType : uint8
{
	/** Clear skies */
	Clear,
	/** Partly cloudy */
	PartlyCloudy,
	/** Overcast */
	Overcast,
	/** Light rain */
	LightRain,
	/** Heavy rain */
	HeavyRain,
	/** Thunderstorm */
	Thunderstorm,
	/** Fog */
	Fog,
	/** Heavy fog */
	HeavyFog,
	/** Snow */
	Snow,
	/** Blizzard */
	Blizzard,
	/** Dust storm */
	DustStorm,
	/** Night clear */
	NightClear,
	/** Night rain */
	NightRain
};

/**
 * Time of day
 */
UENUM(BlueprintType)
enum class EMGTimeOfDay : uint8
{
	/** Dawn (5-7 AM) */
	Dawn,
	/** Morning (7-10 AM) */
	Morning,
	/** Midday (10 AM - 2 PM) */
	Midday,
	/** Afternoon (2-5 PM) */
	Afternoon,
	/** Sunset (5-7 PM) */
	Sunset,
	/** Evening (7-10 PM) */
	Evening,
	/** Night (10 PM - 5 AM) */
	Night
};

/**
 * Road surface condition
 */
UENUM(BlueprintType)
enum class EMGRoadCondition : uint8
{
	/** Dry road */
	Dry,
	/** Damp road */
	Damp,
	/** Wet road */
	Wet,
	/** Standing water */
	StandingWater,
	/** Icy road */
	Icy,
	/** Snowy road */
	Snowy
};

/**
 * Weather intensity
 */
USTRUCT(BlueprintType)
struct FMGWeatherIntensity
{
	GENERATED_BODY()

	/** Precipitation intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Precipitation = 0.0f;

	/** Wind intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Wind = 0.0f;

	/** Fog density (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogDensity = 0.0f;

	/** Cloud coverage (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CloudCoverage = 0.0f;

	/** Lightning frequency (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LightningFrequency = 0.0f;
};

/**
 * Weather state data
 */
USTRUCT(BlueprintType)
struct FMGWeatherState
{
	GENERATED_BODY()

	/** Weather type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType Type = EMGWeatherType::Clear;

	/** Intensity values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWeatherIntensity Intensity;

	/** Road condition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRoadCondition RoadCondition = EMGRoadCondition::Dry;

	/** Temperature (Celsius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 20.0f;

	/** Visibility range (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Visibility = 10000.0f;

	/** Wind direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WindDirection = FVector(1, 0, 0);

	/** Wind speed (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindSpeed = 0.0f;
};

/**
 * Lighting preset for time of day
 */
USTRUCT(BlueprintType)
struct FMGLightingPreset
{
	GENERATED_BODY()

	/** Sun intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunIntensity = 10.0f;

	/** Sun color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SunColor = FLinearColor::White;

	/** Sun pitch angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunPitch = 45.0f;

	/** Sky light intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkyLightIntensity = 1.0f;

	/** Ambient color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AmbientColor = FLinearColor(0.05f, 0.05f, 0.1f);

	/** Fog color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f);

	/** Fog start distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogStartDistance = 500.0f;

	/** Horizon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HorizonColor = FLinearColor(0.7f, 0.8f, 1.0f);
};

/**
 * Weather transition
 */
USTRUCT(BlueprintType)
struct FMGWeatherTransition
{
	GENERATED_BODY()

	/** From state */
	UPROPERTY(BlueprintReadOnly)
	FMGWeatherState FromState;

	/** To state */
	UPROPERTY(BlueprintReadOnly)
	FMGWeatherState ToState;

	/** Transition progress (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float Progress = 0.0f;

	/** Transition duration */
	UPROPERTY(BlueprintReadOnly)
	float Duration = 30.0f;

	/** Is transitioning */
	UPROPERTY(BlueprintReadOnly)
	bool bIsTransitioning = false;
};

/**
 * Weather schedule entry
 */
USTRUCT(BlueprintType)
struct FMGWeatherScheduleEntry
{
	GENERATED_BODY()

	/** Game time (minutes from midnight) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GameTimeMinutes = 0;

	/** Weather type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType WeatherType = EMGWeatherType::Clear;

	/** Transition duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionDuration = 60.0f;
};

/**
 * Track weather settings
 */
USTRUCT(BlueprintType)
struct FMGTrackWeatherSettings
{
	GENERATED_BODY()

	/** Track ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/** Allowed weather types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGWeatherType> AllowedWeather;

	/** Default weather */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType DefaultWeather = EMGWeatherType::Clear;

	/** Allow dynamic weather changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowDynamicWeather = true;

	/** Time of day settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeOfDay DefaultTimeOfDay = EMGTimeOfDay::Midday;

	/** Allow time progression */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowTimeProgression = false;
};

/**
 * Weather effect config
 */
USTRUCT(BlueprintType)
struct FMGWeatherEffectConfig
{
	GENERATED_BODY()

	/** Particle system for precipitation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* PrecipitationEffect = nullptr;

	/** Particle spawn rate multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ParticleRateMultiplier = 1.0f;

	/** Ground splash effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* GroundSplashEffect = nullptr;

	/** Windshield effect material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* WindshieldMaterial = nullptr;

	/** Post process settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPostProcessSettings PostProcessSettings;

	/** Ambient sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* AmbientSound = nullptr;

	/** Ambient sound volume */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AmbientVolume = 1.0f;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, const FMGWeatherState&, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherTransitionStarted, EMGWeatherType, FromType, EMGWeatherType, ToType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherTransitionCompleted, EMGWeatherType, NewType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, EMGTimeOfDay, NewTimeOfDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadConditionChanged, EMGRoadCondition, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLightningStrike);

/**
 * Weather Subsystem
 * Manages dynamic weather and time of day
 *
 * Features:
 * - Multiple weather types
 * - Smooth transitions
 * - Time of day system
 * - Road condition effects
 * - Visual effects integration
 * - Track-specific settings
 */
UCLASS()
class MIDNIGHTGRIND_API UMGWeatherSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherTransitionStarted OnWeatherTransitionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherTransitionCompleted OnWeatherTransitionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoadConditionChanged OnRoadConditionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLightningStrike OnLightningStrike;

	// ==========================================
	// WEATHER CONTROL
	// ==========================================

	/** Set weather type with transition */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(EMGWeatherType NewWeather, float TransitionTime = 30.0f);

	/** Set weather instantly */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeatherInstant(EMGWeatherType NewWeather);

	/** Get current weather state */
	UFUNCTION(BlueprintPure, Category = "Weather")
	FMGWeatherState GetCurrentWeather() const { return CurrentWeather; }

	/** Get current weather type */
	UFUNCTION(BlueprintPure, Category = "Weather")
	EMGWeatherType GetCurrentWeatherType() const { return CurrentWeather.Type; }

	/** Get weather transition */
	UFUNCTION(BlueprintPure, Category = "Weather")
	FMGWeatherTransition GetWeatherTransition() const { return WeatherTransition; }

	/** Is weather transitioning */
	UFUNCTION(BlueprintPure, Category = "Weather")
	bool IsWeatherTransitioning() const { return WeatherTransition.bIsTransitioning; }

	/** Set weather intensity */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeatherIntensity(float Intensity);

	/** Get weather intensity (precipitation) */
	UFUNCTION(BlueprintPure, Category = "Weather")
	float GetWeatherIntensity() const { return CurrentWeather.Intensity.Precipitation; }

	// ==========================================
	// TIME OF DAY
	// ==========================================

	/** Set time of day */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeOfDay(EMGTimeOfDay NewTime, float TransitionTime = 60.0f);

	/** Set time of day instantly */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeOfDayInstant(EMGTimeOfDay NewTime);

	/** Get current time of day */
	UFUNCTION(BlueprintPure, Category = "Time")
	EMGTimeOfDay GetTimeOfDay() const { return CurrentTimeOfDay; }

	/** Set game time (0-1440 minutes) */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetGameTime(float TimeInMinutes);

	/** Get game time */
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetGameTime() const { return GameTimeMinutes; }

	/** Get formatted time string */
	UFUNCTION(BlueprintPure, Category = "Time")
	FText GetFormattedTime() const;

	/** Set time progression enabled */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeProgressionEnabled(bool bEnabled) { bTimeProgressionEnabled = bEnabled; }

	/** Is time progression enabled */
	UFUNCTION(BlueprintPure, Category = "Time")
	bool IsTimeProgressionEnabled() const { return bTimeProgressionEnabled; }

	/** Set time scale (real seconds per game minute) */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeScale(float SecondsPerGameMinute) { TimeScale = SecondsPerGameMinute; }

	// ==========================================
	// ROAD CONDITIONS
	// ==========================================

	/** Get current road condition */
	UFUNCTION(BlueprintPure, Category = "Road")
	EMGRoadCondition GetRoadCondition() const { return CurrentWeather.RoadCondition; }

	/** Get road grip multiplier */
	UFUNCTION(BlueprintPure, Category = "Road")
	float GetRoadGripMultiplier() const;

	/** Get hydroplaning risk (0-1) */
	UFUNCTION(BlueprintPure, Category = "Road")
	float GetHydroplaningRisk() const;

	/** Get visibility distance */
	UFUNCTION(BlueprintPure, Category = "Road")
	float GetVisibilityDistance() const { return CurrentWeather.Visibility; }

	// ==========================================
	// WIND
	// ==========================================

	/** Get wind direction */
	UFUNCTION(BlueprintPure, Category = "Wind")
	FVector GetWindDirection() const { return CurrentWeather.WindDirection; }

	/** Get wind speed */
	UFUNCTION(BlueprintPure, Category = "Wind")
	float GetWindSpeed() const { return CurrentWeather.WindSpeed; }

	/** Get wind force vector */
	UFUNCTION(BlueprintPure, Category = "Wind")
	FVector GetWindForce() const { return CurrentWeather.WindDirection * CurrentWeather.WindSpeed; }

	// ==========================================
	// TRACK SETTINGS
	// ==========================================

	/** Set track weather settings */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void SetTrackWeatherSettings(const FMGTrackWeatherSettings& Settings);

	/** Get track weather settings */
	UFUNCTION(BlueprintPure, Category = "Track")
	FMGTrackWeatherSettings GetTrackWeatherSettings() const { return TrackSettings; }

	/** Apply track default weather */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void ApplyTrackDefaults();

	// ==========================================
	// WEATHER SCHEDULE
	// ==========================================

	/** Set weather schedule */
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void SetWeatherSchedule(const TArray<FMGWeatherScheduleEntry>& Schedule);

	/** Clear weather schedule */
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void ClearWeatherSchedule();

	/** Enable scheduled weather */
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void SetScheduledWeatherEnabled(bool bEnabled) { bScheduledWeatherEnabled = bEnabled; }

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get weather display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetWeatherDisplayName(EMGWeatherType Type);

	/** Get time of day display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetTimeOfDayDisplayName(EMGTimeOfDay Time);

	/** Get road condition display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetRoadConditionDisplayName(EMGRoadCondition Condition);

	/** Is precipitation weather */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static bool IsPrecipitationWeather(EMGWeatherType Type);

	/** Get default state for weather type */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FMGWeatherState GetDefaultWeatherState(EMGWeatherType Type);

	/** Get default lighting for time of day */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FMGLightingPreset GetDefaultLighting(EMGTimeOfDay Time);

protected:
	// ==========================================
	// CURRENT STATE
	// ==========================================

	/** Current weather state */
	UPROPERTY()
	FMGWeatherState CurrentWeather;

	/** Weather transition data */
	UPROPERTY()
	FMGWeatherTransition WeatherTransition;

	/** Current time of day */
	UPROPERTY()
	EMGTimeOfDay CurrentTimeOfDay = EMGTimeOfDay::Midday;

	/** Game time in minutes (0-1440) */
	float GameTimeMinutes = 720.0f; // Noon

	/** Time progression enabled */
	bool bTimeProgressionEnabled = false;

	/** Time scale (real seconds per game minute) */
	float TimeScale = 1.0f;

	// ==========================================
	// TRACK SETTINGS
	// ==========================================

	/** Current track settings */
	UPROPERTY()
	FMGTrackWeatherSettings TrackSettings;

	// ==========================================
	// SCHEDULE
	// ==========================================

	/** Weather schedule */
	UPROPERTY()
	TArray<FMGWeatherScheduleEntry> WeatherSchedule;

	/** Scheduled weather enabled */
	bool bScheduledWeatherEnabled = false;

	// ==========================================
	// LIGHTING
	// ==========================================

	/** Current lighting preset */
	FMGLightingPreset CurrentLighting;

	/** Target lighting preset */
	FMGLightingPreset TargetLighting;

	/** Lighting transition progress */
	float LightingTransitionProgress = 1.0f;

	/** Lighting transition duration */
	float LightingTransitionDuration = 60.0f;

	// ==========================================
	// LIGHTNING
	// ==========================================

	/** Time until next lightning */
	float NextLightningTime = 0.0f;

	/** Lightning cooldown */
	float LightningCooldown = 5.0f;

	// ==========================================
	// EFFECT REFERENCES
	// ==========================================

	/** Material parameter collection */
	UPROPERTY()
	UMaterialParameterCollection* WeatherMPC;

	/** Sun light reference */
	UPROPERTY()
	ADirectionalLight* SunLight;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update weather transition */
	void UpdateWeatherTransition(float DeltaTime);

	/** Update time progression */
	void UpdateTimeProgression(float DeltaTime);

	/** Update lighting transition */
	void UpdateLightingTransition(float DeltaTime);

	/** Update weather schedule */
	void UpdateWeatherSchedule();

	/** Update road condition based on weather */
	void UpdateRoadCondition();

	/** Update lightning */
	void UpdateLightning(float DeltaTime);

	/** Apply weather state */
	void ApplyWeatherState(const FMGWeatherState& State);

	/** Apply lighting preset */
	void ApplyLightingPreset(const FMGLightingPreset& Preset);

	/** Blend weather states */
	FMGWeatherState BlendWeatherStates(const FMGWeatherState& A, const FMGWeatherState& B, float Alpha);

	/** Blend lighting presets */
	FMGLightingPreset BlendLightingPresets(const FMGLightingPreset& A, const FMGLightingPreset& B, float Alpha);

	/** Get time of day from game time */
	EMGTimeOfDay GetTimeOfDayFromGameTime(float TimeMinutes) const;

	/** Trigger lightning strike */
	void TriggerLightningStrike();

	/** Find scene references */
	void FindSceneReferences();

	/** Update material parameters */
	void UpdateMaterialParameters();
};

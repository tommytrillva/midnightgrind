// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGWeatherSubsystem.h
 * @brief Dynamic Weather and Environmental Conditions Subsystem
 *
 * This subsystem manages all weather-related systems in Midnight Grind, providing
 * a comprehensive simulation of environmental conditions that affect gameplay.
 *
 * ## Overview
 * The Weather Subsystem is responsible for:
 * - Dynamic weather states (clear, rain, fog, snow, storms, etc.)
 * - Time of day management and lighting transitions
 * - Road surface conditions that affect vehicle physics
 * - Visual effects integration (precipitation, fog, lightning)
 * - Track-specific weather configurations
 *
 * ## Architecture
 * This is a World Subsystem, meaning one instance exists per game world.
 * It integrates with:
 * - Vehicle physics systems (grip, handling)
 * - Visual rendering (materials, post-processing)
 * - Audio systems (ambient weather sounds)
 * - AI systems (perception, behavior)
 *
 * ## Key Concepts
 * - **Weather State**: Complete description of current conditions (type, intensity, wind, etc.)
 * - **Weather Transition**: Smooth interpolation between two weather states over time
 * - **Road Condition**: Surface state (dry, wet, icy) derived from weather
 * - **Lighting Preset**: Sun/sky settings for different times of day
 *
 * ## Usage Example
 * @code
 * // Get the weather subsystem
 * UMGWeatherSubsystem* Weather = GetWorld()->GetSubsystem<UMGWeatherSubsystem>();
 *
 * // Change weather with 30-second transition
 * Weather->SetWeather(EMGWeatherType::HeavyRain, 30.0f);
 *
 * // Check road conditions for physics
 * float Grip = Weather->GetUnifiedGripMultiplier(VehicleLocation, SpeedKPH);
 * @endcode
 *
 * @see UMGWeatherRacingSubsystem for racing-specific weather effects (aquaplaning, puddles)
 * @see EMGWeatherType for available weather conditions
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/MGCoreEnums.h"
#include "MGWeatherSubsystem.generated.h"

class UMaterialParameterCollection;
class UNiagaraSystem;
class UPostProcessComponent;
class ADirectionalLight;
class ASkyAtmosphere;
class AVolumetricCloud;
class UMGWeatherRacingSubsystem;

// ============================================================================
// WEATHER TYPE ENUMERATION
// ============================================================================

/**
 * @brief Defines all available weather conditions in the game
 *
 * Weather types are organized by atmospheric conditions and affect:
 * - Visual rendering (sky, fog, precipitation particles)
 * - Road surface conditions
 * - Vehicle physics (grip, visibility)
 * - AI behavior and perception
 *
 * Some weather types are time-specific (NightClear, NightRain) and are
 * typically set automatically based on time of day.
 */
// EMGWeatherType and EMGTimeOfDay defined in Core/MGCoreEnums.h

// ============================================================================
// ROAD CONDITION ENUMERATION
// ============================================================================

/**
 * @brief Surface conditions affecting tire grip and vehicle handling
 *
 * Road conditions are automatically derived from the current weather state
 * and duration of precipitation. They directly influence:
 * - Tire grip multipliers
 * - Braking distances
 * - Hydroplaning risk at high speeds
 * - AI driving behavior
 */
UENUM(BlueprintType)
enum class EMGRoadCondition : uint8
{
	/** Dry road - Full grip, optimal handling */
	Dry,
	/** Damp road - Slightly wet, minor grip reduction */
	Damp,
	/** Wet road - Moderate grip loss, visible water on surface */
	Wet,
	/** Standing water - Puddles present, high hydroplaning risk */
	StandingWater,
	/** Icy road - Extremely low grip, very dangerous */
	Icy,
	/** Snowy road - Packed snow, significantly reduced grip */
	Snowy
};

// ============================================================================
// WEATHER INTENSITY STRUCTURE
// ============================================================================

/**
 * @brief Normalized intensity values (0-1) for various weather parameters
 *
 * These values control the visual and physical intensity of weather effects.
 * All values are normalized between 0.0 (none) and 1.0 (maximum).
 * Used for smooth interpolation during weather transitions.
 */
USTRUCT(BlueprintType)
struct FMGWeatherIntensity
{
	GENERATED_BODY()

	/// Precipitation intensity: 0 = none, 1 = maximum rain/snow rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Precipitation = 0.0f;

	/// Wind intensity: 0 = calm, 1 = maximum wind speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Wind = 0.0f;

	/// Fog density: 0 = clear, 1 = dense fog with minimal visibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogDensity = 0.0f;

	/// Cloud coverage: 0 = clear sky, 1 = complete overcast
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CloudCoverage = 0.0f;

	/// Lightning frequency: 0 = none, 1 = frequent strikes (thunderstorms only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LightningFrequency = 0.0f;
};

// ============================================================================
// WEATHER STATE STRUCTURE
// ============================================================================

/**
 * @brief Complete description of current weather conditions
 *
 * This struct represents the full weather state at any given moment,
 * including type, intensity values, road conditions, and atmospheric data.
 * Weather states can be interpolated for smooth transitions.
 */
USTRUCT(BlueprintType)
struct FMGWeatherState
{
	GENERATED_BODY()

	/// The categorical weather type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType Type = EMGWeatherType::Clear;

	/// Normalized intensity values for all weather parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWeatherIntensity Intensity;

	/// Current road surface condition derived from weather
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRoadCondition RoadCondition = EMGRoadCondition::Dry;

	/// Ambient temperature in Celsius (affects ice/snow formation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 20.0f;

	/// Maximum visibility range in meters (affects rendering and AI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Visibility = 10000.0f;

	/// Normalized wind direction vector
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WindDirection = FVector(1, 0, 0);

	/// Wind speed in meters per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindSpeed = 0.0f;
};

// ============================================================================
// LIGHTING PRESET STRUCTURE
// ============================================================================

/**
 * @brief Lighting configuration for a specific time of day
 *
 * Defines all lighting parameters needed to render a particular time period.
 * Presets are interpolated during time transitions for smooth day/night cycles.
 */
USTRUCT(BlueprintType)
struct FMGLightingPreset
{
	GENERATED_BODY()

	/// Directional light (sun/moon) intensity in lux
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunIntensity = 10.0f;

	/// Color tint of the directional light
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SunColor = FLinearColor::White;

	/// Sun elevation angle in degrees (0 = horizon, 90 = overhead)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunPitch = 45.0f;

	/// Sky light ambient intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkyLightIntensity = 1.0f;

	/// Ambient fill light color (shadows and indirect areas)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AmbientColor = FLinearColor(0.05f, 0.05f, 0.1f);

	/// Atmospheric fog color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f);

	/// Distance where fog begins to affect visibility (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogStartDistance = 500.0f;

	/// Color at the horizon line
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HorizonColor = FLinearColor(0.7f, 0.8f, 1.0f);
};

// ============================================================================
// WEATHER TRANSITION STRUCTURE
// ============================================================================

/**
 * @brief Tracks the state of an ongoing weather transition
 *
 * When weather changes, this struct manages the interpolation between
 * the previous and target states over the specified duration.
 */
USTRUCT(BlueprintType)
struct FMGWeatherTransition
{
	GENERATED_BODY()

	/// The weather state being transitioned from
	UPROPERTY(BlueprintReadOnly)
	FMGWeatherState FromState;

	/// The target weather state being transitioned to
	UPROPERTY(BlueprintReadOnly)
	FMGWeatherState ToState;

	/// Current transition progress: 0 = start, 1 = complete
	UPROPERTY(BlueprintReadOnly)
	float Progress = 0.0f;

	/// Total transition duration in seconds
	UPROPERTY(BlueprintReadOnly)
	float Duration = 30.0f;

	/// True while a transition is actively in progress
	UPROPERTY(BlueprintReadOnly)
	bool bIsTransitioning = false;
};

// ============================================================================
// WEATHER SCHEDULE STRUCTURE
// ============================================================================

/**
 * @brief Defines a scheduled weather change at a specific game time
 *
 * Used to create weather schedules for races or free roam sessions.
 * Multiple entries can be combined to create realistic weather patterns.
 */
USTRUCT(BlueprintType)
struct FMGWeatherScheduleEntry
{
	GENERATED_BODY()

	/// Game time when this weather should begin (minutes from midnight, 0-1440)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GameTimeMinutes = 0;

	/// The weather type to transition to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType WeatherType = EMGWeatherType::Clear;

	/// How long the transition should take in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionDuration = 60.0f;
};

// ============================================================================
// TRACK WEATHER SETTINGS STRUCTURE
// ============================================================================

/**
 * @brief Per-track weather configuration
 *
 * Each race track can define its own weather constraints, including
 * which weather types are allowed and whether dynamic changes occur.
 * This allows for weather-appropriate track designs (e.g., no snow on desert tracks).
 */
USTRUCT(BlueprintType)
struct FMGTrackWeatherSettings
{
	GENERATED_BODY()

	/// Unique identifier for the track
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/// List of weather types permitted on this track
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGWeatherType> AllowedWeather;

	/// Weather type used when no specific weather is requested
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType DefaultWeather = EMGWeatherType::Clear;

	/// If true, weather can change dynamically during a race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowDynamicWeather = true;

	/// Starting time of day for races on this track
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeOfDay DefaultTimeOfDay = EMGTimeOfDay::Midday;

	/// If true, time will advance during gameplay on this track
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowTimeProgression = false;
};

// ============================================================================
// WEATHER EFFECT CONFIG STRUCTURE
// ============================================================================

/**
 * @brief Visual and audio assets for a specific weather type
 *
 * Defines all the effects (particles, materials, sounds) needed to
 * represent a particular weather condition. Each weather type has
 * its own configuration.
 */
USTRUCT(BlueprintType)
struct FMGWeatherEffectConfig
{
	GENERATED_BODY()

	/// Niagara system for rain, snow, or other precipitation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* PrecipitationEffect = nullptr;

	/// Multiplier for particle spawn rate (scales with intensity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ParticleRateMultiplier = 1.0f;

	/// Niagara system for rain splashing on ground surfaces
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* GroundSplashEffect = nullptr;

	/// Material for rain droplets/effects on vehicle windshield
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* WindshieldMaterial = nullptr;

	/// Post-process settings (color grading, blur, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPostProcessSettings PostProcessSettings;

	/// Looping ambient sound (rain, wind, thunder)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* AmbientSound = nullptr;

	/// Volume level for ambient sound (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AmbientVolume = 1.0f;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/// Broadcast when weather state changes (after transition completes or instant change)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, const FMGWeatherState&, NewWeather);

/// Broadcast when a weather transition begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherTransitionStarted, EMGWeatherType, FromType, EMGWeatherType, ToType);

/// Broadcast when a weather transition finishes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherTransitionCompleted, EMGWeatherType, NewType);

/// Broadcast when time of day period changes (e.g., Midday -> Afternoon)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, EMGTimeOfDay, NewTimeOfDay);

/// Broadcast when road surface condition changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadConditionChanged, EMGRoadCondition, NewCondition);

/// Broadcast when a lightning strike occurs (for visual/audio effects)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLightningStrike);

// ============================================================================
// WEATHER SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem managing dynamic weather and time of day
 *
 * This World Subsystem provides a centralized interface for all weather
 * and environmental condition management in Midnight Grind.
 *
 * ## Features
 * - Multiple weather types with smooth transitions
 * - Time of day system with lighting presets
 * - Road condition calculations for physics
 * - Wind simulation for particle effects
 * - Track-specific weather configurations
 * - Scheduled weather changes for races
 * - Unified API for physics and AI integration
 *
 * ## Thread Safety
 * All public methods should be called from the game thread only.
 *
 * ## Performance Notes
 * - Weather transitions are evaluated every tick
 * - Material parameter updates are batched
 * - Lightning strikes use cooldown timers
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
	/** @name Event Delegates
	 *  Subscribe to these delegates to respond to weather changes.
	 *  All events are broadcast on the game thread.
	 */
	///@{

	/// Fires when the weather state changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherChanged OnWeatherChanged;

	/// Fires when a weather transition begins
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherTransitionStarted OnWeatherTransitionStarted;

	/// Fires when a weather transition completes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherTransitionCompleted OnWeatherTransitionCompleted;

	/// Fires when time of day period changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	/// Fires when road condition changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoadConditionChanged OnRoadConditionChanged;

	/// Fires when lightning strikes occur
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLightningStrike OnLightningStrike;

	///@}

	// ==========================================
	// WEATHER CONTROL
	// ==========================================
	/** @name Weather Control
	 *  Functions for getting and setting weather conditions.
	 */
	///@{

	/**
	 * @brief Initiates a gradual transition to a new weather type
	 * @param NewWeather The target weather type
	 * @param TransitionTime Duration of the transition in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(EMGWeatherType NewWeather, float TransitionTime = 30.0f);

	/**
	 * @brief Immediately changes to a new weather type without transition
	 * @param NewWeather The weather type to set immediately
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeatherInstant(EMGWeatherType NewWeather);

	/// Returns the complete current weather state
	UFUNCTION(BlueprintPure, Category = "Weather")
	FMGWeatherState GetCurrentWeather() const { return CurrentWeather; }

	/// Returns just the current weather type enum value
	UFUNCTION(BlueprintPure, Category = "Weather")
	EMGWeatherType GetCurrentWeatherType() const { return CurrentWeather.Type; }

	/// Returns the current transition state (check bIsTransitioning)
	UFUNCTION(BlueprintPure, Category = "Weather")
	FMGWeatherTransition GetWeatherTransition() const { return WeatherTransition; }

	/// Returns true if weather is currently transitioning between states
	UFUNCTION(BlueprintPure, Category = "Weather")
	bool IsWeatherTransitioning() const { return WeatherTransition.bIsTransitioning; }

	/**
	 * @brief Adjusts the intensity of the current weather
	 * @param Intensity Normalized value 0-1 affecting precipitation/fog/wind
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeatherIntensity(float Intensity);

	/// Returns current precipitation intensity (0-1)
	UFUNCTION(BlueprintPure, Category = "Weather")
	float GetWeatherIntensity() const { return CurrentWeather.Intensity.Precipitation; }

	///@}

	// ==========================================
	// TIME OF DAY
	// ==========================================
	/** @name Time of Day
	 *  Functions for controlling the game's time system.
	 *  Time affects lighting, NPC schedules, and available events.
	 */
	///@{

	/**
	 * @brief Transitions to a new time of day with smooth lighting change
	 * @param NewTime The target time period
	 * @param TransitionTime Duration of the lighting transition in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeOfDay(EMGTimeOfDay NewTime, float TransitionTime = 60.0f);

	/**
	 * @brief Immediately sets time of day without transition
	 * @param NewTime The time period to set immediately
	 */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeOfDayInstant(EMGTimeOfDay NewTime);

	/// Returns the current time period
	UFUNCTION(BlueprintPure, Category = "Time")
	EMGTimeOfDay GetTimeOfDay() const { return CurrentTimeOfDay; }

	/**
	 * @brief Sets the exact game time
	 * @param TimeInMinutes Minutes from midnight (0-1440), wraps at 1440
	 */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetGameTime(float TimeInMinutes);

	/// Returns current game time in minutes from midnight
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetGameTime() const { return GameTimeMinutes; }

	/// Returns time as displayable text (e.g., "10:30 PM")
	UFUNCTION(BlueprintPure, Category = "Time")
	FText GetFormattedTime() const;

	/// Enables or disables automatic time progression
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeProgressionEnabled(bool bEnabled) { bTimeProgressionEnabled = bEnabled; }

	/// Returns true if time is automatically advancing
	UFUNCTION(BlueprintPure, Category = "Time")
	bool IsTimeProgressionEnabled() const { return bTimeProgressionEnabled; }

	/**
	 * @brief Sets how fast game time progresses
	 * @param SecondsPerGameMinute Real seconds per game minute (1.0 = real-time)
	 */
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeScale(float SecondsPerGameMinute) { TimeScale = SecondsPerGameMinute; }

	///@}

	// ==========================================
	// ROAD CONDITIONS
	// ==========================================
	/** @name Road Conditions
	 *  Functions for querying road surface state for physics calculations.
	 */
	///@{

	/// Returns the current road surface condition
	UFUNCTION(BlueprintPure, Category = "Road")
	EMGRoadCondition GetRoadCondition() const { return CurrentWeather.RoadCondition; }

	/**
	 * @brief Returns tire grip multiplier based on road condition
	 * @return Multiplier where 1.0 = full grip, lower = less grip
	 */
	UFUNCTION(BlueprintPure, Category = "Road")
	float GetRoadGripMultiplier() const;

	/**
	 * @brief Returns hydroplaning risk based on speed and water depth
	 * @return Risk factor 0-1, where 1.0 = certain hydroplaning
	 */
	UFUNCTION(BlueprintPure, Category = "Road")
	float GetHydroplaningRisk() const;

	/// Returns maximum visibility distance in meters
	UFUNCTION(BlueprintPure, Category = "Road")
	float GetVisibilityDistance() const { return CurrentWeather.Visibility; }

	///@}

	// ==========================================
	// UNIFIED WEATHER API
	// ==========================================
	/**
	 * @name Unified Weather API
	 * These functions provide a single source of truth for weather effects,
	 * combining base weather state with racing-specific effects (aquaplaning,
	 * puddles, fog density, etc.) when MGWeatherRacingSubsystem is active.
	 *
	 * **Use these functions for vehicle physics and AI calculations.**
	 */
	///@{

	/**
	 * @brief Get unified grip multiplier combining all weather effects
	 *
	 * This is the primary function for vehicle physics to query tire grip.
	 * It combines:
	 * - Base road condition grip (dry/wet/icy/snowy)
	 * - Racing-specific effects (aquaplaning, puddle depth)
	 * - Weather intensity modifiers
	 *
	 * @param VehicleLocation Optional vehicle location for position-specific effects
	 * @param VehicleSpeedKPH Optional vehicle speed for speed-dependent effects (aquaplaning)
	 * @return Combined grip multiplier (0.1 to 1.0, where 1.0 = full grip)
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Unified")
	float GetUnifiedGripMultiplier(const FVector& VehicleLocation = FVector::ZeroVector, float VehicleSpeedKPH = 0.0f) const;

	/**
	 * @brief Get unified visibility distance combining all weather effects
	 *
	 * Combines base weather visibility with:
	 * - Fog effects
	 * - Night/time-of-day effects
	 * - Rain spray effects
	 *
	 * @param Location Optional location for position-specific fog density
	 * @return Effective visibility distance in meters
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Unified")
	float GetUnifiedVisibilityDistance(const FVector& Location = FVector::ZeroVector) const;

	/**
	 * @brief Get unified AI perception multiplier
	 *
	 * Returns a multiplier (0.1 to 1.0) for AI perception range based on:
	 * - Visibility conditions (fog, rain, night)
	 * - Weather intensity
	 *
	 * @return AI perception range multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Unified")
	float GetUnifiedAIPerceptionMultiplier() const;

	/**
	 * @brief Check if conditions are hazardous for racing
	 *
	 * @return True if current conditions pose significant racing hazards
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Unified")
	bool AreConditionsHazardous() const;

	/**
	 * @brief Get combined weather difficulty rating (1-5)
	 *
	 * @return Difficulty where 1=easy (clear day), 5=extreme (storm)
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|Unified")
	int32 GetWeatherDifficultyRating() const;

	///@}

	// ==========================================
	// WIND
	// ==========================================
	/** @name Wind
	 *  Functions for querying wind conditions (affects particles and high-profile vehicles).
	 */
	///@{

	/// Returns normalized wind direction vector
	UFUNCTION(BlueprintPure, Category = "Wind")
	FVector GetWindDirection() const { return CurrentWeather.WindDirection; }

	/// Returns wind speed in meters per second
	UFUNCTION(BlueprintPure, Category = "Wind")
	float GetWindSpeed() const { return CurrentWeather.WindSpeed; }

	/// Returns wind as a force vector (direction * speed)
	UFUNCTION(BlueprintPure, Category = "Wind")
	FVector GetWindForce() const { return CurrentWeather.WindDirection * CurrentWeather.WindSpeed; }

	///@}

	// ==========================================
	// TRACK SETTINGS
	// ==========================================
	/** @name Track Settings
	 *  Functions for managing track-specific weather configurations.
	 */
	///@{

	/**
	 * @brief Applies weather settings for a specific track
	 * @param Settings The track's weather configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void SetTrackWeatherSettings(const FMGTrackWeatherSettings& Settings);

	/// Returns current track weather settings
	UFUNCTION(BlueprintPure, Category = "Track")
	FMGTrackWeatherSettings GetTrackWeatherSettings() const { return TrackSettings; }

	/// Resets weather to track's default settings
	UFUNCTION(BlueprintCallable, Category = "Track")
	void ApplyTrackDefaults();

	///@}

	// ==========================================
	// WEATHER SCHEDULE
	// ==========================================
	/** @name Weather Schedule
	 *  Functions for managing timed weather sequences.
	 */
	///@{

	/**
	 * @brief Sets up a weather schedule for automatic changes
	 * @param Schedule Array of scheduled weather entries
	 */
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void SetWeatherSchedule(const TArray<FMGWeatherScheduleEntry>& Schedule);

	/// Removes all scheduled weather changes
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void ClearWeatherSchedule();

	/// Enables or disables scheduled weather system
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void SetScheduledWeatherEnabled(bool bEnabled) { bScheduledWeatherEnabled = bEnabled; }

	///@}

	// ==========================================
	// UTILITY
	// ==========================================
	/** @name Utility Functions
	 *  Static helper functions for weather-related operations.
	 */
	///@{

	/// Returns localized display name for a weather type
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetWeatherDisplayName(EMGWeatherType Type);

	/// Returns localized display name for a time of day
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetTimeOfDayDisplayName(EMGTimeOfDay Time);

	/// Returns localized display name for a road condition
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetRoadConditionDisplayName(EMGRoadCondition Condition);

	/// Returns true if the weather type includes precipitation
	UFUNCTION(BlueprintPure, Category = "Utility")
	static bool IsPrecipitationWeather(EMGWeatherType Type);

	/// Returns default weather state values for a weather type
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FMGWeatherState GetDefaultWeatherState(EMGWeatherType Type);

	/// Returns default lighting preset for a time of day
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FMGLightingPreset GetDefaultLighting(EMGTimeOfDay Time);

	///@}

protected:
	// ==========================================
	// CURRENT STATE
	// ==========================================
	/** @name State Variables
	 *  Internal state tracking for weather and time systems.
	 */
	///@{

	/// Active weather state being rendered
	UPROPERTY()
	FMGWeatherState CurrentWeather;

	/// Transition state (valid when bIsTransitioning is true)
	UPROPERTY()
	FMGWeatherTransition WeatherTransition;

	/// Current time period
	UPROPERTY()
	EMGTimeOfDay CurrentTimeOfDay = EMGTimeOfDay::Midday;

	/// Precise game time in minutes (0-1440), wraps at midnight
	float GameTimeMinutes = 720.0f; // Noon

	/// Whether time automatically advances
	bool bTimeProgressionEnabled = false;

	/// Real seconds per game minute (affects time progression speed)
	float TimeScale = 1.0f;

	///@}

	// ==========================================
	// TRACK SETTINGS
	// ==========================================

	/// Current track's weather configuration
	UPROPERTY()
	FMGTrackWeatherSettings TrackSettings;

	// ==========================================
	// SCHEDULE
	// ==========================================

	/// List of scheduled weather changes
	UPROPERTY()
	TArray<FMGWeatherScheduleEntry> WeatherSchedule;

	/// Whether scheduled weather system is active
	bool bScheduledWeatherEnabled = false;

	// ==========================================
	// LIGHTING
	// ==========================================
	/** @name Lighting State
	 *  Variables for lighting transition management.
	 */
	///@{

	/// Current applied lighting values
	FMGLightingPreset CurrentLighting;

	/// Lighting values being transitioned to
	FMGLightingPreset TargetLighting;

	/// Lighting transition progress (0-1)
	float LightingTransitionProgress = 1.0f;

	/// Duration of current lighting transition
	float LightingTransitionDuration = 60.0f;

	///@}

	// ==========================================
	// LIGHTNING
	// ==========================================
	/** @name Lightning State
	 *  Variables for thunderstorm lightning effects.
	 */
	///@{

	/// Countdown to next lightning strike (seconds)
	float NextLightningTime = 0.0f;

	/// Minimum time between lightning strikes
	float LightningCooldown = 5.0f;

	///@}

	// ==========================================
	// EFFECT REFERENCES
	// ==========================================
	/** @name Scene References
	 *  Cached references to world actors/components for weather effects.
	 */
	///@{

	/// Material parameter collection for weather shader values
	UPROPERTY()
	UMaterialParameterCollection* WeatherMPC;

	/// Reference to the main directional light (sun)
	UPROPERTY()
	ADirectionalLight* SunLight;

	/// Optional racing subsystem for advanced effects (aquaplaning, etc.)
	mutable TWeakObjectPtr<UMGWeatherRacingSubsystem> CachedRacingSubsystem;

	/// Flag to avoid repeated failed lookups
	mutable bool bRacingSubsystemSearched = false;

	///@}

	// ==========================================
	// INTERNAL
	// ==========================================
	/** @name Internal Methods
	 *  Private implementation methods for weather system updates.
	 */
	///@{

	/// Advances weather transition and applies interpolated state
	void UpdateWeatherTransition(float DeltaTime);

	/// Advances game time and checks for period changes
	void UpdateTimeProgression(float DeltaTime);

	/// Advances lighting transition and applies interpolated values
	void UpdateLightingTransition(float DeltaTime);

	/// Checks schedule and triggers weather changes if needed
	void UpdateWeatherSchedule();

	/// Calculates road condition from current weather state
	void UpdateRoadCondition();

	/// Manages lightning strike timing during thunderstorms
	void UpdateLightning(float DeltaTime);

	/// Applies a complete weather state to all systems
	void ApplyWeatherState(const FMGWeatherState& State);

	/// Applies a lighting preset to scene lights
	void ApplyLightingPreset(const FMGLightingPreset& Preset);

	/// Interpolates between two weather states
	FMGWeatherState BlendWeatherStates(const FMGWeatherState& A, const FMGWeatherState& B, float Alpha);

	/// Interpolates between two lighting presets
	FMGLightingPreset BlendLightingPresets(const FMGLightingPreset& A, const FMGLightingPreset& B, float Alpha);

	/// Converts game time to time of day period
	EMGTimeOfDay GetTimeOfDayFromGameTime(float TimeMinutes) const;

	/// Executes a lightning strike effect
	void TriggerLightningStrike();

	/// Finds and caches references to scene actors
	void FindSceneReferences();

	/// Updates material parameter collection with current weather values
	void UpdateMaterialParameters();

	/// Lazy getter for optional racing subsystem
	UMGWeatherRacingSubsystem* GetRacingSubsystem() const;

	///@}
};

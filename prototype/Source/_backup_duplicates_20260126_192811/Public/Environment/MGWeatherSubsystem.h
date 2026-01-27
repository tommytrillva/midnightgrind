// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGWeatherSubsystem.generated.h"

// Forward declarations for weather racing integration
class UMGWeatherRacingSubsystem;

/**
 * Weather condition types
 */
UENUM(BlueprintType)
enum class EMGWeatherType : uint8
{
	Clear UMETA(DisplayName = "Clear"),
	Cloudy UMETA(DisplayName = "Cloudy"),
	Overcast UMETA(DisplayName = "Overcast"),
	Fog UMETA(DisplayName = "Fog"),
	LightRain UMETA(DisplayName = "Light Rain"),
	HeavyRain UMETA(DisplayName = "Heavy Rain"),
	Thunderstorm UMETA(DisplayName = "Thunderstorm"),
	Drizzle UMETA(DisplayName = "Drizzle"),
	Mist UMETA(DisplayName = "Mist"),
	Snow UMETA(DisplayName = "Snow"), // For certain locations
	Dust UMETA(DisplayName = "Dust Storm") // Desert areas
};

/**
 * Time of day period
 */
UENUM(BlueprintType)
enum class EMGTimeOfDay : uint8
{
	Dawn UMETA(DisplayName = "Dawn"),          // 5:00 - 7:00
	Morning UMETA(DisplayName = "Morning"),    // 7:00 - 11:00
	Midday UMETA(DisplayName = "Midday"),      // 11:00 - 14:00
	Afternoon UMETA(DisplayName = "Afternoon"),// 14:00 - 17:00
	Dusk UMETA(DisplayName = "Dusk"),          // 17:00 - 20:00
	Night UMETA(DisplayName = "Night"),        // 20:00 - 23:00
	Midnight UMETA(DisplayName = "Midnight"),  // 23:00 - 2:00
	LateNight UMETA(DisplayName = "Late Night")// 2:00 - 5:00
};

/**
 * Road surface condition
 */
UENUM(BlueprintType)
enum class EMGRoadCondition : uint8
{
	Dry UMETA(DisplayName = "Dry"),
	Damp UMETA(DisplayName = "Damp"),
	Wet UMETA(DisplayName = "Wet"),
	Flooded UMETA(DisplayName = "Flooded"),
	Icy UMETA(DisplayName = "Icy"),
	Dusty UMETA(DisplayName = "Dusty"),
	Oily UMETA(DisplayName = "Oily") // From wrecks/spills
};

/**
 * Weather intensity
 */
UENUM(BlueprintType)
enum class EMGWeatherIntensity : uint8
{
	None UMETA(DisplayName = "None"),
	Light UMETA(DisplayName = "Light"),
	Moderate UMETA(DisplayName = "Moderate"),
	Heavy UMETA(DisplayName = "Heavy"),
	Extreme UMETA(DisplayName = "Extreme")
};

/**
 * Weather event (special occurrences)
 */
UENUM(BlueprintType)
enum class EMGWeatherEvent : uint8
{
	None UMETA(DisplayName = "None"),
	LightningStrike UMETA(DisplayName = "Lightning Strike"),
	ThunderClap UMETA(DisplayName = "Thunder Clap"),
	WindGust UMETA(DisplayName = "Wind Gust"),
	Downpour UMETA(DisplayName = "Sudden Downpour"),
	RainbowAppearance UMETA(DisplayName = "Rainbow"),
	PoliceHelicopter UMETA(DisplayName = "Police Helicopter")
};

/**
 * Complete weather state
 */
USTRUCT(BlueprintType)
struct FMGWeatherState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType WeatherType = EMGWeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherIntensity Intensity = EMGWeatherIntensity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRoadCondition RoadCondition = EMGRoadCondition::Dry;

	// Precipitation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RainIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SnowIntensity = 0.0f;

	// Visibility factors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "50000.0"))
	float VisibilityDistance = 10000.0f; // Meters

	// Wind
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float WindSpeed = 0.0f; // km/h

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WindDirection = FVector::ForwardVector;

	// Temperature (affects grip, engine performance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-20.0", ClampMax = "50.0"))
	float AmbientTemperature = 25.0f; // Celsius

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Humidity = 50.0f;

	// Track conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float GripMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WaterPuddleCoverage = 0.0f;

	// Cloud cover (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloudCover = 0.0f;
};

/**
 * Time of day state
 */
USTRUCT(BlueprintType)
struct FMGTimeOfDayState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeOfDay Period = EMGTimeOfDay::Night; // Midnight Grind = night racing

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Hour = 23.0f; // 0-24

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Minute = 0.0f;

	// Lighting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SunIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunAngle = -30.0f; // Below horizon at night

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MoonIntensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MoonPhase = 0.5f; // 0 = new, 0.5 = full, 1.0 = new

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AmbientLightLevel = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SkyColor = FLinearColor(0.02f, 0.02f, 0.08f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HorizonColor = FLinearColor(0.1f, 0.05f, 0.15f, 1.0f);

	// Street lights
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStreetLightsOn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StreetLightIntensity = 1.0f;

	// Neon signs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNeonSignsOn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NeonIntensity = 1.0f;

	// Traffic density modifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TrafficDensity = 0.3f; // Lower at night

	// Police activity modifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float PoliceActivityMultiplier = 0.7f; // Lower at late night
};

/**
 * Weather forecast entry
 */
USTRUCT(BlueprintType)
struct FMGWeatherForecast
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeOffset = 0.0f; // Hours from now

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherType PredictedWeather = EMGWeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWeatherIntensity PredictedIntensity = EMGWeatherIntensity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Confidence = 1.0f; // Accuracy of forecast
};

/**
 * Weather preset for events/races
 */
USTRUCT(BlueprintType)
struct FMGWeatherPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWeatherState WeatherState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDynamic = false; // Can change during race

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float REPMultiplier = 1.0f; // Bonus for difficult conditions

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CashMultiplier = 1.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EMGWeatherType, OldWeather, EMGWeatherType, NewWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, EMGTimeOfDay, OldPeriod, EMGTimeOfDay, NewPeriod);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoadConditionChanged, EMGRoadCondition, OldCondition, EMGRoadCondition, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherEvent, EMGWeatherEvent, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVisibilityChanged, float, OldVisibility, float, NewVisibility);

/**
 * Weather Subsystem
 * Manages weather, time of day, and environmental conditions
 */
UCLASS()
class MIDNIGHTGRIND_API UMGWeatherSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);
	virtual TStatId GetStatId() const override;

	// ==========================================
	// WEATHER STATE
	// ==========================================

	/**
	 * Get current weather state
	 */
	UFUNCTION(BlueprintPure, Category = "Weather")
	FMGWeatherState GetCurrentWeather() const { return CurrentWeather; }

	/**
	 * Get current weather type
	 */
	UFUNCTION(BlueprintPure, Category = "Weather")
	EMGWeatherType GetWeatherType() const { return CurrentWeather.WeatherType; }

	/**
	 * Get current road condition
	 */
	UFUNCTION(BlueprintPure, Category = "Weather")
	EMGRoadCondition GetRoadCondition() const { return CurrentWeather.RoadCondition; }

	/**
	 * Get current grip multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "Weather")
	float GetGripMultiplier() const { return CurrentWeather.GripMultiplier; }

	/**
	 * Get visibility distance
	 */
	UFUNCTION(BlueprintPure, Category = "Weather")
	float GetVisibilityDistance() const { return CurrentWeather.VisibilityDistance; }

	/**
	 * Is it currently raining?
	 */
	UFUNCTION(BlueprintPure, Category = "Weather")
	bool IsRaining() const;

	/**
	 * Get rain intensity
	 */
	UFUNCTION(BlueprintPure, Category = "Weather")
	float GetRainIntensity() const { return CurrentWeather.RainIntensity; }

	// ==========================================
	// TIME OF DAY
	// ==========================================

	/**
	 * Get current time of day state
	 */
	UFUNCTION(BlueprintPure, Category = "Time")
	FMGTimeOfDayState GetTimeOfDayState() const { return TimeOfDay; }

	/**
	 * Get current time period
	 */
	UFUNCTION(BlueprintPure, Category = "Time")
	EMGTimeOfDay GetTimePeriod() const { return TimeOfDay.Period; }

	/**
	 * Get current hour (0-24)
	 */
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetCurrentHour() const { return TimeOfDay.Hour; }

	/**
	 * Get formatted time string
	 */
	UFUNCTION(BlueprintPure, Category = "Time")
	FString GetFormattedTime() const;

	/**
	 * Is it night time?
	 */
	UFUNCTION(BlueprintPure, Category = "Time")
	bool IsNightTime() const;

	/**
	 * Get ambient light level
	 */
	UFUNCTION(BlueprintPure, Category = "Time")
	float GetAmbientLightLevel() const { return TimeOfDay.AmbientLightLevel; }

	// ==========================================
	// WEATHER CONTROL
	// ==========================================

	/**
	 * Set weather instantly
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|Control")
	void SetWeather(EMGWeatherType NewWeather, EMGWeatherIntensity Intensity);

	/**
	 * Transition to new weather over time
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|Control")
	void TransitionToWeather(EMGWeatherType NewWeather, EMGWeatherIntensity Intensity, float TransitionSeconds);

	/**
	 * Apply weather preset
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|Control")
	void ApplyWeatherPreset(FName PresetID);

	/**
	 * Enable/disable dynamic weather
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|Control")
	void SetDynamicWeather(bool bEnabled);

	/**
	 * Force rain start
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|Control")
	void StartRain(EMGWeatherIntensity Intensity, float DurationMinutes);

	/**
	 * Stop rain
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|Control")
	void StopRain(float FadeSeconds);

	// ==========================================
	// TIME CONTROL
	// ==========================================

	/**
	 * Set current time
	 */
	UFUNCTION(BlueprintCallable, Category = "Time|Control")
	void SetTime(float Hour, float Minute);

	/**
	 * Set time speed multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Time|Control")
	void SetTimeSpeed(float Multiplier);

	/**
	 * Pause time progression
	 */
	UFUNCTION(BlueprintCallable, Category = "Time|Control")
	void PauseTime(bool bPaused);

	/**
	 * Skip to time period
	 */
	UFUNCTION(BlueprintCallable, Category = "Time|Control")
	void SkipToTimePeriod(EMGTimeOfDay Period);

	// ==========================================
	// FORECAST
	// ==========================================

	/**
	 * Get weather forecast
	 */
	UFUNCTION(BlueprintPure, Category = "Forecast")
	TArray<FMGWeatherForecast> GetWeatherForecast(int32 HoursAhead = 6) const;

	/**
	 * Get predicted weather at time offset
	 */
	UFUNCTION(BlueprintPure, Category = "Forecast")
	EMGWeatherType GetPredictedWeather(float HoursFromNow) const;

	// ==========================================
	// EFFECTS ON GAMEPLAY
	// ==========================================

	/**
	 * Get speed penalty for current conditions
	 */
	UFUNCTION(BlueprintPure, Category = "Effects")
	float GetSpeedPenalty() const;

	/**
	 * Get visibility penalty (0-1, lower = worse)
	 */
	UFUNCTION(BlueprintPure, Category = "Effects")
	float GetVisibilityPenalty() const;

	/**
	 * Get AI difficulty modifier
	 */
	UFUNCTION(BlueprintPure, Category = "Effects")
	float GetAIDifficultyModifier() const;

	/**
	 * Get REP bonus for current conditions
	 */
	UFUNCTION(BlueprintPure, Category = "Effects")
	float GetConditionREPBonus() const;

	/**
	 * Get cash bonus for current conditions
	 */
	UFUNCTION(BlueprintPure, Category = "Effects")
	float GetConditionCashBonus() const;

	/**
	 * Should headlights be on?
	 */
	UFUNCTION(BlueprintPure, Category = "Effects")
	bool ShouldUseHeadlights() const;

	/**
	 * Should wipers be on?
	 */
	UFUNCTION(BlueprintPure, Category = "Effects")
	bool ShouldUseWipers() const;

	// ==========================================
	// PRESETS
	// ==========================================

	/**
	 * Get all weather presets
	 */
	UFUNCTION(BlueprintPure, Category = "Presets")
	TArray<FMGWeatherPreset> GetAllPresets() const { return WeatherPresets; }

	/**
	 * Get preset by ID
	 */
	UFUNCTION(BlueprintPure, Category = "Presets")
	bool GetPreset(FName PresetID, FMGWeatherPreset& OutPreset) const;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoadConditionChanged OnRoadConditionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWeatherEvent OnWeatherEvent;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnVisibilityChanged OnVisibilityChanged;

protected:
	// Current states
	UPROPERTY()
	FMGWeatherState CurrentWeather;

	UPROPERTY()
	FMGWeatherState TargetWeather;

	UPROPERTY()
	FMGTimeOfDayState TimeOfDay;

	// Transition
	bool bIsTransitioning = false;
	float TransitionProgress = 0.0f;
	float TransitionDuration = 0.0f;

	// Time settings
	float TimeSpeedMultiplier = 1.0f; // Real-time by default
	bool bTimePaused = false;

	// Dynamic weather
	bool bDynamicWeatherEnabled = true;
	float NextWeatherChangeTime = 0.0f;
	float WeatherChangeCheckInterval = 300.0f; // 5 min checks

	// Presets
	UPROPERTY()
	TArray<FMGWeatherPreset> WeatherPresets;

	// ==========================================
	// INTERNAL
	// ==========================================

	void SetupDefaultPresets();
	void UpdateWeatherTransition(float DeltaTime);
	void UpdateTimeOfDay(float DeltaTime);
	void UpdateRoadCondition();
	void CheckDynamicWeather();
	void CalculateGripMultiplier();
	void CalculateVisibility();
	EMGTimeOfDay CalculateTimePeriod(float Hour) const;
	void TriggerWeatherEvent(EMGWeatherEvent Event);
	FMGWeatherState LerpWeatherState(const FMGWeatherState& A, const FMGWeatherState& B, float Alpha) const;

	// Game time accumulator
	float GameTimeAccumulator = 0.0f;

	// Weather event timers
	float LastLightningTime = 0.0f;
	float LightningInterval = 15.0f; // Average seconds between strikes

	// Road drying rate
	static constexpr float RoadDryingRate = 0.01f; // Per second
	static constexpr float RoadWettingRate = 0.05f; // Per second (rain)
};

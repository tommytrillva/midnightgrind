// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGTimeOfDaySubsystem.generated.h"

// EMGTimeOfDay - MOVED TO MGSharedTypes.h
// Canonical definition in: Core/MGSharedTypes.h

// ============================================================================
// TIME OF DAY SETTINGS STRUCTURE
// ============================================================================

/**
 * @brief Visual and gameplay settings for a specific time period
 *
 * This struct defines all the parameters that change based on time of day.
 * The subsystem interpolates between adjacent period settings for smooth
 * transitions.
 */
USTRUCT(BlueprintType)
struct FMGTimeOfDaySettings
{
	GENERATED_BODY()

	// ----- Lighting Settings -----

	/// Intensity of the sun (directional light), 0 at night
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SunIntensity = 1.0f;

	/// Intensity of the moon during night hours
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoonIntensity = 0.3f;

	/// Overall sky brightness multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkyBrightness = 1.0f;

	/// Color tint of the sun light (warm at dawn/dusk, white at midday)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f);

	/// Ambient fill light color for shadowed areas
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AmbientColor = FLinearColor(0.3f, 0.35f, 0.4f);

	/// Atmospheric fog color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FogColor = FLinearColor(0.5f, 0.55f, 0.6f);

	/// Fog density multiplier (higher at night/dawn)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogDensity = 0.02f;

	// ----- Artificial Lighting Settings -----

	/// Neon sign brightness multiplier (0 during day, 1 at night)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NeonIntensity = 0.0f;

	/// Street light brightness multiplier (0 during day, 1 at night)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreetLightIntensity = 0.0f;

	// ----- World Population Settings -----

	/// Multiplier for traffic vehicle spawning (lower late night)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrafficDensityMultiplier = 1.0f;

	/// Multiplier for pedestrian spawning (varies by time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PedestrianDensityMultiplier = 1.0f;

	// ----- Sky Settings -----

	/// Visibility of stars in the sky (0 during day, 1 at night)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StarVisibility = 0.0f;
};

// ============================================================================
// TIME PERIOD EVENTS STRUCTURE
// ============================================================================

/**
 * @brief Gameplay modifiers and available events for a time period
 *
 * Defines what activities are available and how rewards are modified
 * during specific time periods. Encourages players to race at different
 * times for varied experiences and bonuses.
 */
USTRUCT(BlueprintType)
struct FMGTimePeriodEvents
{
	GENERATED_BODY()

	/// The time period these events apply to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeOfDay TimePeriod = EMGTimeOfDay::Night;

	/// Race types available during this period (e.g., "Sprint", "Circuit", "Drift")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AvailableRaceTypes;

	/// True if special midnight races can be triggered (Night/LateNight only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMidnightRacesAvailable = false;

	/// True if police patrol more frequently during this period
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCopActivityIncreased = false;

	/// Reputation earned is multiplied by this value (e.g., 1.5x at night)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReputationMultiplier = 1.0f;

	/// Cash rewards are multiplied by this value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CashMultiplier = 1.0f;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/**
 * @brief Broadcast when the time period changes (e.g., Morning -> Midday)
 * @param OldTime The previous time period
 * @param NewTime The new time period
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTimeOfDayChanged, EMGTimeOfDay, OldTime, EMGTimeOfDay, NewTime);

/**
 * @brief Broadcast when the clock strikes midnight
 * @param GameDay The new game day number
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMidnightReached, int32, GameDay);

/**
 * @brief Broadcast every time a new hour begins
 * @param NewHour The hour that just started (0-23)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHourChanged, int32, NewHour);

// ============================================================================
// TIME OF DAY SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem managing the game's time and day/night cycle
 *
 * This Game Instance Subsystem provides centralized time management for
 * Midnight Grind, controlling lighting, atmosphere, and time-based events.
 *
 * ## Features
 * - Continuous time progression with adjustable speed
 * - Eight distinct time periods with unique settings
 * - Smooth interpolation between lighting states
 * - Multiplayer time synchronization support
 * - Event system for time-based triggers
 * - Reward multipliers based on time of day
 *
 * ## Default Behavior
 * - Starts at 10:00 PM (prime racing time)
 * - Time scale: 1 real second = 1 game minute
 * - Time progression can be paused for races/events
 *
 * ## Thread Safety
 * All public methods should be called from the game thread only.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTimeOfDaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// TIME QUERIES
	// ==========================================
	/** @name Time Queries
	 *  Functions for reading the current game time state.
	 */
	///@{

	/**
	 * @brief Returns current time as a decimal hour value
	 * @return Time in hours (0.0 - 23.99), e.g., 14.5 = 2:30 PM
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	float GetCurrentTime() const { return CurrentTimeHours; }

	/**
	 * @brief Returns the current hour (integer)
	 * @return Hour value 0-23
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	int32 GetCurrentHour() const { return FMath::FloorToInt(CurrentTimeHours); }

	/**
	 * @brief Returns the current minute within the hour
	 * @return Minute value 0-59
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	int32 GetCurrentMinute() const { return FMath::FloorToInt(FMath::Frac(CurrentTimeHours) * 60.0f); }

	/**
	 * @brief Returns the current categorical time period
	 * @return The active time period enum value
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	EMGTimeOfDay GetTimePeriod() const { return CurrentTimePeriod; }

	/**
	 * @brief Returns time as a formatted display string
	 * @return Localized time string (e.g., "10:30 PM")
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	FText GetTimeString() const;

	/**
	 * @brief Checks if current time is during night hours
	 * @return True if time period is Night or LateNight
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	bool IsNightTime() const;

	/**
	 * @brief Checks if it's currently the midnight hour (00:00 - 00:59)
	 * @return True if hour is 0
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	bool IsMidnightHour() const;

	/**
	 * @brief Returns how many game days have passed
	 * @return Day counter starting from 1
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	int32 GetGameDay() const { return GameDay; }

	///@}

	// ==========================================
	// TIME CONTROL
	// ==========================================
	/** @name Time Control
	 *  Functions for manipulating the game time.
	 */
	///@{

	/**
	 * @brief Sets the current time directly
	 * @param TimeHours Time in decimal hours (0-24), values wrap automatically
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void SetTime(float TimeHours);

	/**
	 * @brief Jumps to a specific time period
	 * @param Period The target time period (will set time to middle of period)
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void SetTimePeriod(EMGTimeOfDay Period);

	/**
	 * @brief Sets how fast game time passes
	 * @param Scale Seconds of real time per game minute (60.0 = 1 hour per minute)
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void SetTimeScale(float Scale);

	/**
	 * @brief Returns current time scale
	 * @return Real seconds per game minute
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay")
	float GetTimeScale() const { return TimeScale; }

	/**
	 * @brief Pauses or resumes time progression
	 * @param bPause True to pause, false to resume
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void PauseTime(bool bPause);

	/**
	 * @brief Synchronizes local time with server (multiplayer)
	 * @param ServerTimeHours The authoritative server time
	 * @param ServerGameDay The authoritative game day
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeOfDay")
	void SyncWithServer(float ServerTimeHours, int32 ServerGameDay);

	///@}

	// ==========================================
	// LIGHTING SETTINGS
	// ==========================================
	/** @name Lighting Settings
	 *  Functions for querying current lighting parameters.
	 */
	///@{

	/**
	 * @brief Returns the interpolated settings for current time
	 * @return Complete settings struct for current moment
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Lighting")
	FMGTimeOfDaySettings GetCurrentSettings() const { return CurrentSettings; }

	/**
	 * @brief Returns the sun's elevation angle
	 * @return Angle in degrees (-90 to 90, negative = below horizon)
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Lighting")
	float GetSunAngle() const;

	/**
	 * @brief Returns the sun's full rotation for directional light
	 * @return Rotation with pitch (elevation) and yaw (azimuth)
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Lighting")
	FRotator GetSunRotation() const;

	/**
	 * @brief Returns time as a 0-1 value for shader interpolation
	 * @return Normalized time (0 = midnight, 0.5 = noon)
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Lighting")
	float GetNormalizedTime() const { return CurrentTimeHours / 24.0f; }

	///@}

	// ==========================================
	// PERIOD EVENTS
	// ==========================================
	/** @name Period Events
	 *  Functions for querying time-based gameplay modifiers.
	 */
	///@{

	/**
	 * @brief Returns event settings for current time period
	 * @return Event configuration for current period
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Events")
	FMGTimePeriodEvents GetPeriodEvents() const;

	/**
	 * @brief Returns reputation reward multiplier for current time
	 * @return Multiplier value (typically 1.0 - 2.0)
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Events")
	float GetCurrentReputationMultiplier() const;

	/**
	 * @brief Returns cash reward multiplier for current time
	 * @return Multiplier value (typically 1.0 - 2.0)
	 */
	UFUNCTION(BlueprintPure, Category = "TimeOfDay|Events")
	float GetCurrentCashMultiplier() const;

	///@}

	// ==========================================
	// EVENTS
	// ==========================================
	/** @name Event Delegates
	 *  Subscribe to these delegates to respond to time changes.
	 */
	///@{

	/// Broadcast when time period changes
	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay|Events")
	FMGOnTimeOfDayChanged OnTimeOfDayChanged;

	/// Broadcast at midnight each game day
	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay|Events")
	FMGOnMidnightReached OnMidnightReached;

	/// Broadcast at the start of each hour
	UPROPERTY(BlueprintAssignable, Category = "TimeOfDay|Events")
	FMGOnHourChanged OnHourChanged;

	///@}

protected:
	// ==========================================
	// INTERNAL UPDATE METHODS
	// ==========================================
	/** @name Internal Methods
	 *  Private implementation for time system updates.
	 */
	///@{

	/**
	 * @brief Advances time based on delta and time scale
	 * @param DeltaTime Frame delta time in seconds
	 */
	void UpdateTime(float MGDeltaTime);

	/**
	 * @brief Recalculates lighting based on current time
	 */
	void UpdateLightingSettings();

	/**
	 * @brief Determines time period from hour value
	 * @param TimeHours Decimal hour value
	 * @return Corresponding time period
	 */
	EMGTimeOfDay CalculateTimePeriod(float TimeHours) const;

	/**
	 * @brief Blends settings between adjacent time periods
	 * @param TimeHours Current decimal hour
	 * @return Interpolated settings
	 */
	FMGTimeOfDaySettings InterpolateSettings(float TimeHours) const;

	/**
	 * @brief Sets up default settings for all time periods
	 */
	void InitializePeriodSettings();

	///@}

private:
	// ==========================================
	// STATE VARIABLES
	// ==========================================

	/// Current time in decimal hours (0.0 - 23.99)
	float CurrentTimeHours = 22.0f; // Start at 10 PM - prime racing time

	/// Time progression speed (real seconds per game minute)
	float TimeScale = 60.0f; // 1 real second = 1 game minute

	/// Current game day counter (increments at midnight)
	int32 GameDay = 1;

	/// Last hour value (for detecting hour changes)
	int32 LastHour = -1;

	/// Current categorical time period
	EMGTimeOfDay CurrentTimePeriod = EMGTimeOfDay::Night;

	/// Interpolated settings for current moment
	FMGTimeOfDaySettings CurrentSettings;

	/// Settings defined for each time period
	TMap<EMGTimeOfDay, FMGTimeOfDaySettings> PeriodSettings;

	/// Event configurations for each time period
	TMap<EMGTimeOfDay, FMGTimePeriodEvents> PeriodEvents;

	/// Timer handle for periodic time updates
	FTimerHandle TimeUpdateHandle;

	/// Whether time progression is paused
	bool bTimePaused = false;
};

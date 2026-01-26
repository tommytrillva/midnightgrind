// Copyright Midnight Grind. All Rights Reserved.

#include "Environment/MGWeatherSubsystem.h"

void UMGWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize to late night (Midnight Grind theme)
	TimeOfDay.Period = EMGTimeOfDay::Midnight;
	TimeOfDay.Hour = 23.5f;
	TimeOfDay.Minute = 30.0f;
	TimeOfDay.SunIntensity = 0.0f;
	TimeOfDay.AmbientLightLevel = 0.15f;
	TimeOfDay.bStreetLightsOn = true;
	TimeOfDay.bNeonSignsOn = true;
	TimeOfDay.TrafficDensity = 0.25f;
	TimeOfDay.PoliceActivityMultiplier = 0.6f;

	// Initialize to clear night
	CurrentWeather.WeatherType = EMGWeatherType::Clear;
	CurrentWeather.Intensity = EMGWeatherIntensity::None;
	CurrentWeather.RoadCondition = EMGRoadCondition::Dry;
	CurrentWeather.GripMultiplier = 1.0f;
	CurrentWeather.VisibilityDistance = 5000.0f; // Reduced at night
	CurrentWeather.AmbientTemperature = 20.0f;

	TargetWeather = CurrentWeather;

	SetupDefaultPresets();

	// Set first weather change check
	NextWeatherChangeTime = FMath::RandRange(600.0f, 1800.0f); // 10-30 minutes

	UE_LOG(LogTemp, Log, TEXT("MGWeatherSubsystem: Initialized with clear night conditions"));
}

void UMGWeatherSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMGWeatherSubsystem::Tick(float DeltaTime)
{
	// Update weather transition
	if (bIsTransitioning)
	{
		UpdateWeatherTransition(DeltaTime);
	}

	// Update time of day
	if (!bTimePaused)
	{
		UpdateTimeOfDay(DeltaTime);
	}

	// Update road condition based on weather
	UpdateRoadCondition();

	// Check for dynamic weather changes
	if (bDynamicWeatherEnabled)
	{
		CheckDynamicWeather();
	}

	// Random weather events during storms
	if (CurrentWeather.WeatherType == EMGWeatherType::Thunderstorm)
	{
		LastLightningTime += DeltaTime;
		if (LastLightningTime >= LightningInterval)
		{
			TriggerWeatherEvent(EMGWeatherEvent::LightningStrike);
			LastLightningTime = 0.0f;
			LightningInterval = FMath::RandRange(5.0f, 30.0f);
		}
	}
}

TStatId UMGWeatherSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMGWeatherSubsystem, STATGROUP_Tickables);
}

// ==========================================
// WEATHER STATE
// ==========================================

bool UMGWeatherSubsystem::IsRaining() const
{
	return CurrentWeather.WeatherType == EMGWeatherType::LightRain ||
		   CurrentWeather.WeatherType == EMGWeatherType::HeavyRain ||
		   CurrentWeather.WeatherType == EMGWeatherType::Thunderstorm ||
		   CurrentWeather.WeatherType == EMGWeatherType::Drizzle;
}

// ==========================================
// TIME OF DAY
// ==========================================

FString UMGWeatherSubsystem::GetFormattedTime() const
{
	int32 Hour = FMath::FloorToInt(TimeOfDay.Hour);
	int32 Minute = FMath::FloorToInt(TimeOfDay.Minute);

	bool bPM = Hour >= 12;
	int32 DisplayHour = Hour % 12;
	if (DisplayHour == 0) DisplayHour = 12;

	return FString::Printf(TEXT("%d:%02d %s"), DisplayHour, Minute, bPM ? TEXT("PM") : TEXT("AM"));
}

bool UMGWeatherSubsystem::IsNightTime() const
{
	return TimeOfDay.Period == EMGTimeOfDay::Night ||
		   TimeOfDay.Period == EMGTimeOfDay::Midnight ||
		   TimeOfDay.Period == EMGTimeOfDay::LateNight ||
		   TimeOfDay.Period == EMGTimeOfDay::Dawn;
}

// ==========================================
// WEATHER CONTROL
// ==========================================

void UMGWeatherSubsystem::SetWeather(EMGWeatherType NewWeather, EMGWeatherIntensity Intensity)
{
	EMGWeatherType OldWeather = CurrentWeather.WeatherType;

	CurrentWeather.WeatherType = NewWeather;
	CurrentWeather.Intensity = Intensity;

	// Set appropriate values based on weather type
	switch (NewWeather)
	{
	case EMGWeatherType::Clear:
		CurrentWeather.RainIntensity = 0.0f;
		CurrentWeather.FogDensity = 0.0f;
		CurrentWeather.CloudCover = 0.1f;
		CurrentWeather.VisibilityDistance = IsNightTime() ? 5000.0f : 20000.0f;
		break;

	case EMGWeatherType::Cloudy:
		CurrentWeather.RainIntensity = 0.0f;
		CurrentWeather.FogDensity = 0.05f;
		CurrentWeather.CloudCover = 0.6f;
		CurrentWeather.VisibilityDistance = IsNightTime() ? 4000.0f : 15000.0f;
		break;

	case EMGWeatherType::Overcast:
		CurrentWeather.RainIntensity = 0.0f;
		CurrentWeather.FogDensity = 0.1f;
		CurrentWeather.CloudCover = 0.9f;
		CurrentWeather.VisibilityDistance = IsNightTime() ? 3000.0f : 10000.0f;
		break;

	case EMGWeatherType::Fog:
		CurrentWeather.RainIntensity = 0.0f;
		CurrentWeather.FogDensity = 0.5f + (static_cast<float>(Intensity) * 0.1f);
		CurrentWeather.CloudCover = 1.0f;
		CurrentWeather.VisibilityDistance = 200.0f + (1.0f - CurrentWeather.FogDensity) * 800.0f;
		break;

	case EMGWeatherType::LightRain:
		CurrentWeather.RainIntensity = 0.3f;
		CurrentWeather.FogDensity = 0.1f;
		CurrentWeather.CloudCover = 0.8f;
		CurrentWeather.VisibilityDistance = IsNightTime() ? 2500.0f : 8000.0f;
		break;

	case EMGWeatherType::HeavyRain:
		CurrentWeather.RainIntensity = 0.7f + (static_cast<float>(Intensity) * 0.1f);
		CurrentWeather.FogDensity = 0.2f;
		CurrentWeather.CloudCover = 1.0f;
		CurrentWeather.VisibilityDistance = IsNightTime() ? 1000.0f : 3000.0f;
		break;

	case EMGWeatherType::Thunderstorm:
		CurrentWeather.RainIntensity = 0.9f;
		CurrentWeather.FogDensity = 0.15f;
		CurrentWeather.CloudCover = 1.0f;
		CurrentWeather.VisibilityDistance = IsNightTime() ? 800.0f : 2000.0f;
		CurrentWeather.WindSpeed = 40.0f + FMath::RandRange(0.0f, 30.0f);
		break;

	case EMGWeatherType::Drizzle:
		CurrentWeather.RainIntensity = 0.15f;
		CurrentWeather.FogDensity = 0.15f;
		CurrentWeather.CloudCover = 0.7f;
		CurrentWeather.VisibilityDistance = IsNightTime() ? 3000.0f : 10000.0f;
		break;

	case EMGWeatherType::Mist:
		CurrentWeather.RainIntensity = 0.0f;
		CurrentWeather.FogDensity = 0.3f;
		CurrentWeather.CloudCover = 0.5f;
		CurrentWeather.VisibilityDistance = IsNightTime() ? 1500.0f : 3000.0f;
		break;

	default:
		break;
	}

	CalculateGripMultiplier();
	CalculateVisibility();

	TargetWeather = CurrentWeather;
	bIsTransitioning = false;

	if (OldWeather != NewWeather)
	{
		OnWeatherChanged.Broadcast(OldWeather, NewWeather);
	}
}

void UMGWeatherSubsystem::TransitionToWeather(EMGWeatherType NewWeather, EMGWeatherIntensity Intensity, float TransitionSeconds)
{
	// Store current as starting point
	// Set target weather
	TargetWeather = CurrentWeather;
	TargetWeather.WeatherType = NewWeather;
	TargetWeather.Intensity = Intensity;

	// Set target values based on weather type (same logic as SetWeather)
	switch (NewWeather)
	{
	case EMGWeatherType::Clear:
		TargetWeather.RainIntensity = 0.0f;
		TargetWeather.FogDensity = 0.0f;
		TargetWeather.CloudCover = 0.1f;
		TargetWeather.VisibilityDistance = IsNightTime() ? 5000.0f : 20000.0f;
		break;

	case EMGWeatherType::LightRain:
		TargetWeather.RainIntensity = 0.3f;
		TargetWeather.FogDensity = 0.1f;
		TargetWeather.CloudCover = 0.8f;
		TargetWeather.VisibilityDistance = IsNightTime() ? 2500.0f : 8000.0f;
		break;

	case EMGWeatherType::HeavyRain:
		TargetWeather.RainIntensity = 0.8f;
		TargetWeather.FogDensity = 0.2f;
		TargetWeather.CloudCover = 1.0f;
		TargetWeather.VisibilityDistance = IsNightTime() ? 1000.0f : 3000.0f;
		break;

	case EMGWeatherType::Fog:
		TargetWeather.RainIntensity = 0.0f;
		TargetWeather.FogDensity = 0.6f;
		TargetWeather.CloudCover = 1.0f;
		TargetWeather.VisibilityDistance = 400.0f;
		break;

	case EMGWeatherType::Thunderstorm:
		TargetWeather.RainIntensity = 0.9f;
		TargetWeather.FogDensity = 0.15f;
		TargetWeather.CloudCover = 1.0f;
		TargetWeather.VisibilityDistance = IsNightTime() ? 800.0f : 2000.0f;
		TargetWeather.WindSpeed = 50.0f;
		break;

	default:
		// Use defaults
		break;
	}

	bIsTransitioning = true;
	TransitionProgress = 0.0f;
	TransitionDuration = TransitionSeconds;

	UE_LOG(LogTemp, Log, TEXT("MGWeatherSubsystem: Starting weather transition to %d over %.1f seconds"),
		static_cast<int32>(NewWeather), TransitionSeconds);
}

void UMGWeatherSubsystem::ApplyWeatherPreset(FName PresetID)
{
	FMGWeatherPreset Preset;
	if (GetPreset(PresetID, Preset))
	{
		CurrentWeather = Preset.WeatherState;
		TargetWeather = CurrentWeather;
		bIsTransitioning = false;
		bDynamicWeatherEnabled = Preset.bDynamic;

		UE_LOG(LogTemp, Log, TEXT("MGWeatherSubsystem: Applied preset '%s'"), *PresetID.ToString());
	}
}

void UMGWeatherSubsystem::SetDynamicWeather(bool bEnabled)
{
	bDynamicWeatherEnabled = bEnabled;

	if (bEnabled)
	{
		NextWeatherChangeTime = FMath::RandRange(600.0f, 1800.0f);
	}
}

void UMGWeatherSubsystem::StartRain(EMGWeatherIntensity Intensity, float DurationMinutes)
{
	EMGWeatherType RainType = (Intensity >= EMGWeatherIntensity::Heavy) ?
		EMGWeatherType::HeavyRain : EMGWeatherType::LightRain;

	TransitionToWeather(RainType, Intensity, 60.0f); // 1 minute transition

	// Schedule rain stop
	// In production, would use timer
	UE_LOG(LogTemp, Log, TEXT("MGWeatherSubsystem: Starting rain for %.1f minutes"), DurationMinutes);
}

void UMGWeatherSubsystem::StopRain(float FadeSeconds)
{
	TransitionToWeather(EMGWeatherType::Cloudy, EMGWeatherIntensity::Light, FadeSeconds);
}

// ==========================================
// TIME CONTROL
// ==========================================

void UMGWeatherSubsystem::SetTime(float Hour, float Minute)
{
	EMGTimeOfDay OldPeriod = TimeOfDay.Period;

	TimeOfDay.Hour = FMath::Clamp(Hour, 0.0f, 24.0f);
	TimeOfDay.Minute = FMath::Clamp(Minute, 0.0f, 60.0f);

	TimeOfDay.Period = CalculateTimePeriod(TimeOfDay.Hour);

	// Update lighting based on time
	if (TimeOfDay.Hour >= 6.0f && TimeOfDay.Hour < 20.0f)
	{
		// Daytime
		float DayProgress = (TimeOfDay.Hour - 6.0f) / 14.0f;
		TimeOfDay.SunIntensity = FMath::Sin(DayProgress * PI);
		TimeOfDay.AmbientLightLevel = 0.3f + TimeOfDay.SunIntensity * 0.7f;
		TimeOfDay.bStreetLightsOn = TimeOfDay.Hour >= 18.0f;
	}
	else
	{
		// Nighttime
		TimeOfDay.SunIntensity = 0.0f;
		TimeOfDay.AmbientLightLevel = 0.1f + TimeOfDay.MoonIntensity * 0.1f;
		TimeOfDay.bStreetLightsOn = true;
	}

	if (OldPeriod != TimeOfDay.Period)
	{
		OnTimeOfDayChanged.Broadcast(OldPeriod, TimeOfDay.Period);
	}
}

void UMGWeatherSubsystem::SetTimeSpeed(float Multiplier)
{
	TimeSpeedMultiplier = FMath::Clamp(Multiplier, 0.0f, 100.0f);
}

void UMGWeatherSubsystem::PauseTime(bool bPaused)
{
	bTimePaused = bPaused;
}

void UMGWeatherSubsystem::SkipToTimePeriod(EMGTimeOfDay Period)
{
	float TargetHour = 0.0f;

	switch (Period)
	{
	case EMGTimeOfDay::Dawn: TargetHour = 5.5f; break;
	case EMGTimeOfDay::Morning: TargetHour = 8.0f; break;
	case EMGTimeOfDay::Midday: TargetHour = 12.0f; break;
	case EMGTimeOfDay::Afternoon: TargetHour = 15.0f; break;
	case EMGTimeOfDay::Dusk: TargetHour = 18.5f; break;
	case EMGTimeOfDay::Night: TargetHour = 21.0f; break;
	case EMGTimeOfDay::Midnight: TargetHour = 0.0f; break;
	case EMGTimeOfDay::LateNight: TargetHour = 3.0f; break;
	}

	SetTime(TargetHour, 0.0f);
}

// ==========================================
// FORECAST
// ==========================================

TArray<FMGWeatherForecast> UMGWeatherSubsystem::GetWeatherForecast(int32 HoursAhead) const
{
	TArray<FMGWeatherForecast> Forecast;

	// Simple forecast based on current weather
	EMGWeatherType CurrentType = CurrentWeather.WeatherType;

	for (int32 i = 1; i <= HoursAhead; i++)
	{
		FMGWeatherForecast Entry;
		Entry.TimeOffset = static_cast<float>(i);
		Entry.PredictedWeather = CurrentType;
		Entry.PredictedIntensity = CurrentWeather.Intensity;
		Entry.Confidence = 1.0f - (i * 0.1f); // Confidence decreases with time

		// Random chance of weather change
		if (FMath::RandRange(0.0f, 1.0f) < 0.15f * i)
		{
			// Weather might change
			int32 RandomWeather = FMath::RandRange(0, 4);
			switch (RandomWeather)
			{
			case 0: Entry.PredictedWeather = EMGWeatherType::Clear; break;
			case 1: Entry.PredictedWeather = EMGWeatherType::Cloudy; break;
			case 2: Entry.PredictedWeather = EMGWeatherType::LightRain; break;
			case 3: Entry.PredictedWeather = EMGWeatherType::Fog; break;
			default: Entry.PredictedWeather = CurrentType; break;
			}
			Entry.Confidence *= 0.7f;
		}

		Forecast.Add(Entry);
	}

	return Forecast;
}

EMGWeatherType UMGWeatherSubsystem::GetPredictedWeather(float HoursFromNow) const
{
	TArray<FMGWeatherForecast> Forecast = GetWeatherForecast(FMath::CeilToInt(HoursFromNow));

	for (const FMGWeatherForecast& Entry : Forecast)
	{
		if (Entry.TimeOffset >= HoursFromNow)
		{
			return Entry.PredictedWeather;
		}
	}

	return CurrentWeather.WeatherType;
}

// ==========================================
// EFFECTS ON GAMEPLAY
// ==========================================

float UMGWeatherSubsystem::GetSpeedPenalty() const
{
	float Penalty = 0.0f;

	// Rain reduces speed through reduced grip
	Penalty += CurrentWeather.RainIntensity * 0.15f; // Up to 15% in heavy rain

	// Fog doesn't affect speed directly but visibility

	// Wind affects speed slightly
	Penalty += (CurrentWeather.WindSpeed / 100.0f) * 0.05f; // Up to 5% in strong wind

	return FMath::Clamp(Penalty, 0.0f, 0.25f);
}

float UMGWeatherSubsystem::GetVisibilityPenalty() const
{
	// 1.0 = perfect visibility, 0.0 = blind
	float BaseVisibility = CurrentWeather.VisibilityDistance / 10000.0f;

	// Reduce further for fog
	BaseVisibility *= (1.0f - CurrentWeather.FogDensity * 0.5f);

	// Night reduces visibility
	if (IsNightTime())
	{
		BaseVisibility *= 0.7f;
	}

	// Rain reduces visibility
	BaseVisibility *= (1.0f - CurrentWeather.RainIntensity * 0.3f);

	return FMath::Clamp(BaseVisibility, 0.1f, 1.0f);
}

float UMGWeatherSubsystem::GetAIDifficultyModifier() const
{
	float Modifier = 1.0f;

	// AI is slightly worse in bad conditions
	if (IsRaining())
	{
		Modifier -= CurrentWeather.RainIntensity * 0.15f;
	}

	if (CurrentWeather.FogDensity > 0.3f)
	{
		Modifier -= (CurrentWeather.FogDensity - 0.3f) * 0.1f;
	}

	// AI is better during day (can see better)
	if (!IsNightTime())
	{
		Modifier += 0.05f;
	}

	return FMath::Clamp(Modifier, 0.7f, 1.1f);
}

float UMGWeatherSubsystem::GetConditionREPBonus() const
{
	float Bonus = 0.0f;

	// Bonus for racing in difficult conditions
	if (CurrentWeather.WeatherType == EMGWeatherType::HeavyRain)
	{
		Bonus += 15.0f;
	}
	else if (CurrentWeather.WeatherType == EMGWeatherType::Thunderstorm)
	{
		Bonus += 25.0f;
	}
	else if (CurrentWeather.WeatherType == EMGWeatherType::Fog &&
			 CurrentWeather.FogDensity > 0.5f)
	{
		Bonus += 20.0f;
	}
	else if (IsRaining())
	{
		Bonus += 10.0f;
	}

	// Night bonus (Midnight Grind theme)
	if (IsNightTime())
	{
		Bonus += 5.0f;
	}

	return Bonus;
}

float UMGWeatherSubsystem::GetConditionCashBonus() const
{
	// Cash bonus is half of REP bonus
	return GetConditionREPBonus() * 0.5f;
}

bool UMGWeatherSubsystem::ShouldUseHeadlights() const
{
	return IsNightTime() ||
		   CurrentWeather.FogDensity > 0.2f ||
		   CurrentWeather.VisibilityDistance < 3000.0f;
}

bool UMGWeatherSubsystem::ShouldUseWipers() const
{
	return IsRaining() && CurrentWeather.RainIntensity > 0.1f;
}

// ==========================================
// PRESETS
// ==========================================

bool UMGWeatherSubsystem::GetPreset(FName PresetID, FMGWeatherPreset& OutPreset) const
{
	for (const FMGWeatherPreset& Preset : WeatherPresets)
	{
		if (Preset.PresetID == PresetID)
		{
			OutPreset = Preset;
			return true;
		}
	}
	return false;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGWeatherSubsystem::SetupDefaultPresets()
{
	WeatherPresets.Empty();

	// Clear Night (default)
	{
		FMGWeatherPreset Preset;
		Preset.PresetID = FName(TEXT("ClearNight"));
		Preset.DisplayName = NSLOCTEXT("Weather", "ClearNight", "Clear Night");
		Preset.WeatherState.WeatherType = EMGWeatherType::Clear;
		Preset.WeatherState.RoadCondition = EMGRoadCondition::Dry;
		Preset.WeatherState.GripMultiplier = 1.0f;
		Preset.WeatherState.VisibilityDistance = 5000.0f;
		Preset.bDynamic = true;
		Preset.REPMultiplier = 1.0f;
		Preset.CashMultiplier = 1.0f;
		WeatherPresets.Add(Preset);
	}

	// Rainy Night
	{
		FMGWeatherPreset Preset;
		Preset.PresetID = FName(TEXT("RainyNight"));
		Preset.DisplayName = NSLOCTEXT("Weather", "RainyNight", "Rainy Night");
		Preset.WeatherState.WeatherType = EMGWeatherType::HeavyRain;
		Preset.WeatherState.Intensity = EMGWeatherIntensity::Heavy;
		Preset.WeatherState.RoadCondition = EMGRoadCondition::Wet;
		Preset.WeatherState.RainIntensity = 0.8f;
		Preset.WeatherState.GripMultiplier = 0.7f;
		Preset.WeatherState.VisibilityDistance = 1500.0f;
		Preset.bDynamic = false;
		Preset.REPMultiplier = 1.2f;
		Preset.CashMultiplier = 1.1f;
		WeatherPresets.Add(Preset);
	}

	// Foggy
	{
		FMGWeatherPreset Preset;
		Preset.PresetID = FName(TEXT("DenseFog"));
		Preset.DisplayName = NSLOCTEXT("Weather", "DenseFog", "Dense Fog");
		Preset.WeatherState.WeatherType = EMGWeatherType::Fog;
		Preset.WeatherState.Intensity = EMGWeatherIntensity::Heavy;
		Preset.WeatherState.RoadCondition = EMGRoadCondition::Damp;
		Preset.WeatherState.FogDensity = 0.7f;
		Preset.WeatherState.GripMultiplier = 0.9f;
		Preset.WeatherState.VisibilityDistance = 300.0f;
		Preset.bDynamic = false;
		Preset.REPMultiplier = 1.25f;
		Preset.CashMultiplier = 1.15f;
		WeatherPresets.Add(Preset);
	}

	// Thunderstorm
	{
		FMGWeatherPreset Preset;
		Preset.PresetID = FName(TEXT("Thunderstorm"));
		Preset.DisplayName = NSLOCTEXT("Weather", "Thunderstorm", "Thunderstorm");
		Preset.WeatherState.WeatherType = EMGWeatherType::Thunderstorm;
		Preset.WeatherState.Intensity = EMGWeatherIntensity::Extreme;
		Preset.WeatherState.RoadCondition = EMGRoadCondition::Flooded;
		Preset.WeatherState.RainIntensity = 1.0f;
		Preset.WeatherState.WindSpeed = 60.0f;
		Preset.WeatherState.GripMultiplier = 0.6f;
		Preset.WeatherState.VisibilityDistance = 500.0f;
		Preset.bDynamic = false;
		Preset.REPMultiplier = 1.5f;
		Preset.CashMultiplier = 1.25f;
		WeatherPresets.Add(Preset);
	}

	// Neon Rain (atmospheric)
	{
		FMGWeatherPreset Preset;
		Preset.PresetID = FName(TEXT("NeonRain"));
		Preset.DisplayName = NSLOCTEXT("Weather", "NeonRain", "Neon Rain");
		Preset.WeatherState.WeatherType = EMGWeatherType::LightRain;
		Preset.WeatherState.Intensity = EMGWeatherIntensity::Light;
		Preset.WeatherState.RoadCondition = EMGRoadCondition::Wet;
		Preset.WeatherState.RainIntensity = 0.3f;
		Preset.WeatherState.GripMultiplier = 0.85f;
		Preset.WeatherState.VisibilityDistance = 3000.0f;
		Preset.bDynamic = true;
		Preset.REPMultiplier = 1.1f;
		Preset.CashMultiplier = 1.05f;
		WeatherPresets.Add(Preset);
	}
}

void UMGWeatherSubsystem::UpdateWeatherTransition(float DeltaTime)
{
	if (!bIsTransitioning)
	{
		return;
	}

	TransitionProgress += DeltaTime / TransitionDuration;

	if (TransitionProgress >= 1.0f)
	{
		// Transition complete
		EMGWeatherType OldWeather = CurrentWeather.WeatherType;
		CurrentWeather = TargetWeather;
		bIsTransitioning = false;
		TransitionProgress = 1.0f;

		if (OldWeather != CurrentWeather.WeatherType)
		{
			OnWeatherChanged.Broadcast(OldWeather, CurrentWeather.WeatherType);
		}
	}
	else
	{
		// Interpolate weather values
		CurrentWeather = LerpWeatherState(CurrentWeather, TargetWeather, TransitionProgress);
	}

	CalculateGripMultiplier();
	CalculateVisibility();
}

void UMGWeatherSubsystem::UpdateTimeOfDay(float DeltaTime)
{
	// Accumulate game time (1 real second = 1 game minute by default)
	GameTimeAccumulator += DeltaTime * TimeSpeedMultiplier;

	// Convert to minutes
	while (GameTimeAccumulator >= 1.0f)
	{
		GameTimeAccumulator -= 1.0f;
		TimeOfDay.Minute += 1.0f;

		// Roll over minutes to hours
		if (TimeOfDay.Minute >= 60.0f)
		{
			TimeOfDay.Minute = 0.0f;
			TimeOfDay.Hour += 1.0f;

			// Roll over hours
			if (TimeOfDay.Hour >= 24.0f)
			{
				TimeOfDay.Hour = 0.0f;
			}
		}
	}

	// Update period
	EMGTimeOfDay OldPeriod = TimeOfDay.Period;
	TimeOfDay.Period = CalculateTimePeriod(TimeOfDay.Hour);

	if (OldPeriod != TimeOfDay.Period)
	{
		OnTimeOfDayChanged.Broadcast(OldPeriod, TimeOfDay.Period);

		// Update lighting
		SetTime(TimeOfDay.Hour, TimeOfDay.Minute);
	}
}

void UMGWeatherSubsystem::UpdateRoadCondition()
{
	EMGRoadCondition OldCondition = CurrentWeather.RoadCondition;

	if (IsRaining())
	{
		// Road gets wetter
		CurrentWeather.WaterPuddleCoverage = FMath::Min(1.0f,
			CurrentWeather.WaterPuddleCoverage + RoadWettingRate * CurrentWeather.RainIntensity);

		if (CurrentWeather.RainIntensity > 0.8f)
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Flooded;
		}
		else if (CurrentWeather.WaterPuddleCoverage > 0.5f)
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Wet;
		}
		else
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Damp;
		}
	}
	else
	{
		// Road dries
		CurrentWeather.WaterPuddleCoverage = FMath::Max(0.0f,
			CurrentWeather.WaterPuddleCoverage - RoadDryingRate);

		if (CurrentWeather.WaterPuddleCoverage > 0.3f)
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Wet;
		}
		else if (CurrentWeather.WaterPuddleCoverage > 0.1f)
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Damp;
		}
		else
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Dry;
		}
	}

	if (OldCondition != CurrentWeather.RoadCondition)
	{
		OnRoadConditionChanged.Broadcast(OldCondition, CurrentWeather.RoadCondition);
	}
}

void UMGWeatherSubsystem::CheckDynamicWeather()
{
	NextWeatherChangeTime -= GetWorld()->GetDeltaSeconds();

	if (NextWeatherChangeTime <= 0.0f)
	{
		// Chance of weather change
		float ChangeChance = 0.3f; // 30% chance to change

		if (FMath::RandRange(0.0f, 1.0f) < ChangeChance)
		{
			// Pick new weather based on current
			EMGWeatherType NewWeather = CurrentWeather.WeatherType;
			float Random = FMath::RandRange(0.0f, 1.0f);

			switch (CurrentWeather.WeatherType)
			{
			case EMGWeatherType::Clear:
				if (Random < 0.4f) NewWeather = EMGWeatherType::Cloudy;
				else if (Random < 0.5f) NewWeather = EMGWeatherType::Mist;
				break;

			case EMGWeatherType::Cloudy:
				if (Random < 0.3f) NewWeather = EMGWeatherType::Clear;
				else if (Random < 0.5f) NewWeather = EMGWeatherType::LightRain;
				else if (Random < 0.6f) NewWeather = EMGWeatherType::Overcast;
				break;

			case EMGWeatherType::LightRain:
				if (Random < 0.3f) NewWeather = EMGWeatherType::HeavyRain;
				else if (Random < 0.5f) NewWeather = EMGWeatherType::Drizzle;
				else if (Random < 0.7f) NewWeather = EMGWeatherType::Cloudy;
				break;

			case EMGWeatherType::HeavyRain:
				if (Random < 0.2f) NewWeather = EMGWeatherType::Thunderstorm;
				else if (Random < 0.5f) NewWeather = EMGWeatherType::LightRain;
				break;

			case EMGWeatherType::Thunderstorm:
				if (Random < 0.6f) NewWeather = EMGWeatherType::HeavyRain;
				break;

			default:
				if (Random < 0.4f) NewWeather = EMGWeatherType::Clear;
				break;
			}

			if (NewWeather != CurrentWeather.WeatherType)
			{
				TransitionToWeather(NewWeather, EMGWeatherIntensity::Moderate, 120.0f);
			}
		}

		// Schedule next check
		NextWeatherChangeTime = FMath::RandRange(600.0f, 1800.0f);
	}
}

void UMGWeatherSubsystem::CalculateGripMultiplier()
{
	float Grip = 1.0f;

	// Rain effect
	Grip -= CurrentWeather.RainIntensity * 0.3f;

	// Road condition effect
	switch (CurrentWeather.RoadCondition)
	{
	case EMGRoadCondition::Damp: Grip -= 0.05f; break;
	case EMGRoadCondition::Wet: Grip -= 0.15f; break;
	case EMGRoadCondition::Flooded: Grip -= 0.35f; break;
	case EMGRoadCondition::Icy: Grip -= 0.5f; break;
	case EMGRoadCondition::Dusty: Grip -= 0.1f; break;
	case EMGRoadCondition::Oily: Grip -= 0.4f; break;
	default: break;
	}

	// Temperature effect (optimal around 25C)
	float TempDiff = FMath::Abs(CurrentWeather.AmbientTemperature - 25.0f);
	Grip -= TempDiff * 0.005f;

	CurrentWeather.GripMultiplier = FMath::Clamp(Grip, 0.4f, 1.0f);
}

void UMGWeatherSubsystem::CalculateVisibility()
{
	float OldVisibility = CurrentWeather.VisibilityDistance;

	// Base visibility
	float Visibility = 10000.0f;

	// Night reduction
	if (IsNightTime())
	{
		Visibility *= 0.5f;
	}

	// Fog reduction
	Visibility *= (1.0f - CurrentWeather.FogDensity * 0.9f);

	// Rain reduction
	Visibility *= (1.0f - CurrentWeather.RainIntensity * 0.5f);

	CurrentWeather.VisibilityDistance = FMath::Max(100.0f, Visibility);

	// Fire event if significant change
	if (FMath::Abs(OldVisibility - CurrentWeather.VisibilityDistance) > 500.0f)
	{
		OnVisibilityChanged.Broadcast(OldVisibility, CurrentWeather.VisibilityDistance);
	}
}

EMGTimeOfDay UMGWeatherSubsystem::CalculateTimePeriod(float Hour) const
{
	if (Hour >= 5.0f && Hour < 7.0f) return EMGTimeOfDay::Dawn;
	if (Hour >= 7.0f && Hour < 11.0f) return EMGTimeOfDay::Morning;
	if (Hour >= 11.0f && Hour < 14.0f) return EMGTimeOfDay::Midday;
	if (Hour >= 14.0f && Hour < 17.0f) return EMGTimeOfDay::Afternoon;
	if (Hour >= 17.0f && Hour < 20.0f) return EMGTimeOfDay::Dusk;
	if (Hour >= 20.0f && Hour < 23.0f) return EMGTimeOfDay::Night;
	if (Hour >= 23.0f || Hour < 2.0f) return EMGTimeOfDay::Midnight;
	return EMGTimeOfDay::LateNight; // 2-5
}

void UMGWeatherSubsystem::TriggerWeatherEvent(EMGWeatherEvent Event)
{
	OnWeatherEvent.Broadcast(Event);

	UE_LOG(LogTemp, Verbose, TEXT("MGWeatherSubsystem: Weather event triggered: %d"), static_cast<int32>(Event));
}

FMGWeatherState UMGWeatherSubsystem::LerpWeatherState(const FMGWeatherState& A, const FMGWeatherState& B, float Alpha) const
{
	FMGWeatherState Result;

	// Use target weather type when past halfway
	Result.WeatherType = Alpha < 0.5f ? A.WeatherType : B.WeatherType;
	Result.Intensity = Alpha < 0.5f ? A.Intensity : B.Intensity;
	Result.RoadCondition = Alpha < 0.5f ? A.RoadCondition : B.RoadCondition;

	// Lerp numeric values
	Result.RainIntensity = FMath::Lerp(A.RainIntensity, B.RainIntensity, Alpha);
	Result.SnowIntensity = FMath::Lerp(A.SnowIntensity, B.SnowIntensity, Alpha);
	Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
	Result.VisibilityDistance = FMath::Lerp(A.VisibilityDistance, B.VisibilityDistance, Alpha);
	Result.WindSpeed = FMath::Lerp(A.WindSpeed, B.WindSpeed, Alpha);
	Result.WindDirection = FMath::Lerp(A.WindDirection, B.WindDirection, Alpha);
	Result.AmbientTemperature = FMath::Lerp(A.AmbientTemperature, B.AmbientTemperature, Alpha);
	Result.Humidity = FMath::Lerp(A.Humidity, B.Humidity, Alpha);
	Result.GripMultiplier = FMath::Lerp(A.GripMultiplier, B.GripMultiplier, Alpha);
	Result.WaterPuddleCoverage = FMath::Lerp(A.WaterPuddleCoverage, B.WaterPuddleCoverage, Alpha);
	Result.CloudCover = FMath::Lerp(A.CloudCover, B.CloudCover, Alpha);

	return Result;
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Weather/MGWeatherSubsystem.h"
#include "Environment/MGWeatherRacingEffects.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

void UMGWeatherSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default weather
	CurrentWeather = GetDefaultWeatherState(EMGWeatherType::Clear);
	CurrentTimeOfDay = EMGTimeOfDay::Midday;
	CurrentLighting = GetDefaultLighting(EMGTimeOfDay::Midday);
	TargetLighting = CurrentLighting;

	// Find scene references
	FindSceneReferences();
}

void UMGWeatherSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMGWeatherSubsystem::Tick(float MGDeltaTime)
{
	// Update weather transition
	UpdateWeatherTransition(DeltaTime);

	// Update time progression
	UpdateTimeProgression(DeltaTime);

	// Update lighting transition
	UpdateLightingTransition(DeltaTime);

	// Update weather schedule
	if (bScheduledWeatherEnabled)
	{
		UpdateWeatherSchedule();
	}

	// Update lightning for storms
	UpdateLightning(DeltaTime);

	// Update material parameters
	UpdateMaterialParameters();
}

// ==========================================
// WEATHER CONTROL
// ==========================================

void UMGWeatherSubsystem::SetWeather(EMGWeatherType NewWeather, float TransitionTime)
{
	if (NewWeather == CurrentWeather.Type && !WeatherTransition.bIsTransitioning)
	{
		return;
	}

	// Check if allowed by track settings
	if (TrackSettings.AllowedWeather.Num() > 0 && !TrackSettings.AllowedWeather.Contains(NewWeather))
	{
		return;
	}

	// Setup transition
	WeatherTransition.FromState = CurrentWeather;
	WeatherTransition.ToState = GetDefaultWeatherState(NewWeather);
	WeatherTransition.Progress = 0.0f;
	WeatherTransition.Duration = TransitionTime;
	WeatherTransition.bIsTransitioning = true;

	// Broadcast event
	OnWeatherTransitionStarted.Broadcast(CurrentWeather.Type, NewWeather);
}

void UMGWeatherSubsystem::SetWeatherInstant(EMGWeatherType NewWeather)
{
	// Check if allowed
	if (TrackSettings.AllowedWeather.Num() > 0 && !TrackSettings.AllowedWeather.Contains(NewWeather))
	{
		return;
	}

	// Cancel any transition
	WeatherTransition.bIsTransitioning = false;

	// Apply new weather
	CurrentWeather = GetDefaultWeatherState(NewWeather);
	ApplyWeatherState(CurrentWeather);

	// Update road condition
	UpdateRoadCondition();

	// Broadcast events
	OnWeatherChanged.Broadcast(CurrentWeather);
	OnWeatherTransitionCompleted.Broadcast(NewWeather);
}

void UMGWeatherSubsystem::SetWeatherIntensity(float Intensity)
{
	Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

	CurrentWeather.Intensity.Precipitation = Intensity;

	// Scale other effects based on precipitation
	if (IsPrecipitationWeather(CurrentWeather.Type))
	{
		CurrentWeather.Intensity.Wind = Intensity * 0.5f;
		CurrentWeather.Visibility = FMath::Lerp(10000.0f, 500.0f, Intensity);
	}

	ApplyWeatherState(CurrentWeather);
	UpdateRoadCondition();
}

// ==========================================
// TIME OF DAY
// ==========================================

void UMGWeatherSubsystem::SetTimeOfDay(EMGTimeOfDay NewTime, float TransitionTime)
{
	if (NewTime == CurrentTimeOfDay)
	{
		return;
	}

	TargetLighting = GetDefaultLighting(NewTime);
	LightingTransitionProgress = 0.0f;
	LightingTransitionDuration = TransitionTime;

	EMGTimeOfDay OldTime = CurrentTimeOfDay;
	CurrentTimeOfDay = NewTime;

	OnTimeOfDayChanged.Broadcast(NewTime);
}

void UMGWeatherSubsystem::SetTimeOfDayInstant(EMGTimeOfDay NewTime)
{
	CurrentTimeOfDay = NewTime;
	CurrentLighting = GetDefaultLighting(NewTime);
	TargetLighting = CurrentLighting;
	LightingTransitionProgress = 1.0f;

	ApplyLightingPreset(CurrentLighting);
	OnTimeOfDayChanged.Broadcast(NewTime);
}

void UMGWeatherSubsystem::SetGameTime(float TimeInMinutes)
{
	GameTimeMinutes = FMath::Fmod(TimeInMinutes, 1440.0f);
	if (GameTimeMinutes < 0.0f)
	{
		GameTimeMinutes += 1440.0f;
	}

	EMGTimeOfDay NewTimeOfDay = GetTimeOfDayFromGameTime(GameTimeMinutes);
	if (NewTimeOfDay != CurrentTimeOfDay)
	{
		SetTimeOfDay(NewTimeOfDay, 60.0f);
	}
}

FText UMGWeatherSubsystem::GetFormattedTime() const
{
	int32 Hours = FMath::FloorToInt(GameTimeMinutes / 60.0f);
	int32 Minutes = FMath::FloorToInt(FMath::Fmod(GameTimeMinutes, 60.0f));

	bool bPM = Hours >= 12;
	int32 DisplayHours = Hours % 12;
	if (DisplayHours == 0) DisplayHours = 12;

	return FText::Format(
		NSLOCTEXT("Weather", "TimeFormat", "{0}:{1} {2}"),
		FText::AsNumber(DisplayHours),
		FText::FromString(FString::Printf(TEXT("%02d"), Minutes)),
		bPM ? NSLOCTEXT("Weather", "PM", "PM") : NSLOCTEXT("Weather", "AM", "AM")
	);
}

// ==========================================
// ROAD CONDITIONS
// ==========================================

float UMGWeatherSubsystem::GetRoadGripMultiplier() const
{
	switch (CurrentWeather.RoadCondition)
	{
	case EMGRoadCondition::Dry:
		return 1.0f;
	case EMGRoadCondition::Damp:
		return 0.9f;
	case EMGRoadCondition::Wet:
		return 0.75f;
	case EMGRoadCondition::StandingWater:
		return 0.6f;
	case EMGRoadCondition::Icy:
		return 0.3f;
	case EMGRoadCondition::Snowy:
		return 0.5f;
	default:
		return 1.0f;
	}
}

float UMGWeatherSubsystem::GetHydroplaningRisk() const
{
	switch (CurrentWeather.RoadCondition)
	{
	case EMGRoadCondition::Dry:
	case EMGRoadCondition::Damp:
		return 0.0f;
	case EMGRoadCondition::Wet:
		return 0.2f;
	case EMGRoadCondition::StandingWater:
		return 0.7f;
	default:
		return 0.0f;
	}
}

// ==========================================
// TRACK SETTINGS
// ==========================================

void UMGWeatherSubsystem::SetTrackWeatherSettings(const FMGTrackWeatherSettings& Settings)
{
	TrackSettings = Settings;
}

void UMGWeatherSubsystem::ApplyTrackDefaults()
{
	SetWeatherInstant(TrackSettings.DefaultWeather);
	SetTimeOfDayInstant(TrackSettings.DefaultTimeOfDay);
	bTimeProgressionEnabled = TrackSettings.bAllowTimeProgression;
}

// ==========================================
// WEATHER SCHEDULE
// ==========================================

void UMGWeatherSubsystem::SetWeatherSchedule(const TArray<FMGWeatherScheduleEntry>& Schedule)
{
	WeatherSchedule = Schedule;

	// Sort by time
	WeatherSchedule.Sort([](const FMGWeatherScheduleEntry& A, const FMGWeatherScheduleEntry& B)
	{
		return A.GameTimeMinutes < B.GameTimeMinutes;
	});
}

void UMGWeatherSubsystem::ClearWeatherSchedule()
{
	WeatherSchedule.Empty();
	bScheduledWeatherEnabled = false;
}

// ==========================================
// UTILITY
// ==========================================

FText UMGWeatherSubsystem::GetWeatherDisplayName(EMGWeatherType Type)
{
	switch (Type)
	{
	case EMGWeatherType::Clear:
		return NSLOCTEXT("Weather", "Clear", "Clear");
	case EMGWeatherType::PartlyCloudy:
		return NSLOCTEXT("Weather", "PartlyCloudy", "Partly Cloudy");
	case EMGWeatherType::Overcast:
		return NSLOCTEXT("Weather", "Overcast", "Overcast");
	case EMGWeatherType::LightRain:
		return NSLOCTEXT("Weather", "LightRain", "Light Rain");
	case EMGWeatherType::HeavyRain:
		return NSLOCTEXT("Weather", "HeavyRain", "Heavy Rain");
	case EMGWeatherType::Thunderstorm:
		return NSLOCTEXT("Weather", "Thunderstorm", "Thunderstorm");
	case EMGWeatherType::Fog:
		return NSLOCTEXT("Weather", "Fog", "Fog");
	case EMGWeatherType::HeavyFog:
		return NSLOCTEXT("Weather", "HeavyFog", "Heavy Fog");
	case EMGWeatherType::Snow:
		return NSLOCTEXT("Weather", "Snow", "Snow");
	case EMGWeatherType::Blizzard:
		return NSLOCTEXT("Weather", "Blizzard", "Blizzard");
	case EMGWeatherType::DustStorm:
		return NSLOCTEXT("Weather", "DustStorm", "Dust Storm");
	case EMGWeatherType::NightClear:
		return NSLOCTEXT("Weather", "NightClear", "Night (Clear)");
	case EMGWeatherType::NightRain:
		return NSLOCTEXT("Weather", "NightRain", "Night (Rain)");
	default:
		return FText::GetEmpty();
	}
}

FText UMGWeatherSubsystem::GetTimeOfDayDisplayName(EMGTimeOfDay Time)
{
	switch (Time)
	{
	case EMGTimeOfDay::Dawn:
		return NSLOCTEXT("Weather", "Dawn", "Dawn");
	case EMGTimeOfDay::Morning:
		return NSLOCTEXT("Weather", "Morning", "Morning");
	case EMGTimeOfDay::Midday:
		return NSLOCTEXT("Weather", "Midday", "Midday");
	case EMGTimeOfDay::Afternoon:
		return NSLOCTEXT("Weather", "Afternoon", "Afternoon");
	case EMGTimeOfDay::Sunset:
		return NSLOCTEXT("Weather", "Sunset", "Sunset");
	case EMGTimeOfDay::Evening:
		return NSLOCTEXT("Weather", "Evening", "Evening");
	case EMGTimeOfDay::Night:
		return NSLOCTEXT("Weather", "Night", "Night");
	default:
		return FText::GetEmpty();
	}
}

FText UMGWeatherSubsystem::GetRoadConditionDisplayName(EMGRoadCondition Condition)
{
	switch (Condition)
	{
	case EMGRoadCondition::Dry:
		return NSLOCTEXT("Weather", "RoadDry", "Dry");
	case EMGRoadCondition::Damp:
		return NSLOCTEXT("Weather", "RoadDamp", "Damp");
	case EMGRoadCondition::Wet:
		return NSLOCTEXT("Weather", "RoadWet", "Wet");
	case EMGRoadCondition::StandingWater:
		return NSLOCTEXT("Weather", "RoadStandingWater", "Standing Water");
	case EMGRoadCondition::Icy:
		return NSLOCTEXT("Weather", "RoadIcy", "Icy");
	case EMGRoadCondition::Snowy:
		return NSLOCTEXT("Weather", "RoadSnowy", "Snowy");
	default:
		return FText::GetEmpty();
	}
}

bool UMGWeatherSubsystem::IsPrecipitationWeather(EMGWeatherType Type)
{
	switch (Type)
	{
	case EMGWeatherType::LightRain:
	case EMGWeatherType::HeavyRain:
	case EMGWeatherType::Thunderstorm:
	case EMGWeatherType::Snow:
	case EMGWeatherType::Blizzard:
	case EMGWeatherType::NightRain:
		return true;
	default:
		return false;
	}
}

FMGWeatherState UMGWeatherSubsystem::GetDefaultWeatherState(EMGWeatherType Type)
{
	FMGWeatherState State;
	State.Type = Type;

	switch (Type)
	{
	case EMGWeatherType::Clear:
		State.Intensity.Precipitation = 0.0f;
		State.Intensity.Wind = 0.1f;
		State.Intensity.FogDensity = 0.0f;
		State.Intensity.CloudCoverage = 0.1f;
		State.RoadCondition = EMGRoadCondition::Dry;
		State.Visibility = 15000.0f;
		State.WindSpeed = 2.0f;
		break;

	case EMGWeatherType::PartlyCloudy:
		State.Intensity.Precipitation = 0.0f;
		State.Intensity.Wind = 0.2f;
		State.Intensity.FogDensity = 0.0f;
		State.Intensity.CloudCoverage = 0.4f;
		State.RoadCondition = EMGRoadCondition::Dry;
		State.Visibility = 12000.0f;
		State.WindSpeed = 5.0f;
		break;

	case EMGWeatherType::Overcast:
		State.Intensity.Precipitation = 0.0f;
		State.Intensity.Wind = 0.3f;
		State.Intensity.FogDensity = 0.1f;
		State.Intensity.CloudCoverage = 0.9f;
		State.RoadCondition = EMGRoadCondition::Dry;
		State.Visibility = 8000.0f;
		State.WindSpeed = 8.0f;
		break;

	case EMGWeatherType::LightRain:
		State.Intensity.Precipitation = 0.3f;
		State.Intensity.Wind = 0.3f;
		State.Intensity.FogDensity = 0.2f;
		State.Intensity.CloudCoverage = 0.8f;
		State.RoadCondition = EMGRoadCondition::Wet;
		State.Visibility = 5000.0f;
		State.WindSpeed = 10.0f;
		break;

	case EMGWeatherType::HeavyRain:
		State.Intensity.Precipitation = 0.8f;
		State.Intensity.Wind = 0.6f;
		State.Intensity.FogDensity = 0.4f;
		State.Intensity.CloudCoverage = 1.0f;
		State.RoadCondition = EMGRoadCondition::StandingWater;
		State.Visibility = 1500.0f;
		State.WindSpeed = 20.0f;
		break;

	case EMGWeatherType::Thunderstorm:
		State.Intensity.Precipitation = 1.0f;
		State.Intensity.Wind = 0.8f;
		State.Intensity.FogDensity = 0.3f;
		State.Intensity.CloudCoverage = 1.0f;
		State.Intensity.LightningFrequency = 0.7f;
		State.RoadCondition = EMGRoadCondition::StandingWater;
		State.Visibility = 800.0f;
		State.WindSpeed = 30.0f;
		break;

	case EMGWeatherType::Fog:
		State.Intensity.Precipitation = 0.0f;
		State.Intensity.Wind = 0.1f;
		State.Intensity.FogDensity = 0.6f;
		State.Intensity.CloudCoverage = 0.5f;
		State.RoadCondition = EMGRoadCondition::Damp;
		State.Visibility = 500.0f;
		State.WindSpeed = 2.0f;
		break;

	case EMGWeatherType::HeavyFog:
		State.Intensity.Precipitation = 0.0f;
		State.Intensity.Wind = 0.05f;
		State.Intensity.FogDensity = 0.9f;
		State.Intensity.CloudCoverage = 0.7f;
		State.RoadCondition = EMGRoadCondition::Damp;
		State.Visibility = 100.0f;
		State.WindSpeed = 1.0f;
		break;

	case EMGWeatherType::Snow:
		State.Intensity.Precipitation = 0.5f;
		State.Intensity.Wind = 0.4f;
		State.Intensity.FogDensity = 0.3f;
		State.Intensity.CloudCoverage = 0.9f;
		State.RoadCondition = EMGRoadCondition::Snowy;
		State.Visibility = 2000.0f;
		State.WindSpeed = 12.0f;
		State.Temperature = -5.0f;
		break;

	case EMGWeatherType::Blizzard:
		State.Intensity.Precipitation = 1.0f;
		State.Intensity.Wind = 1.0f;
		State.Intensity.FogDensity = 0.7f;
		State.Intensity.CloudCoverage = 1.0f;
		State.RoadCondition = EMGRoadCondition::Icy;
		State.Visibility = 200.0f;
		State.WindSpeed = 40.0f;
		State.Temperature = -15.0f;
		break;

	case EMGWeatherType::DustStorm:
		State.Intensity.Precipitation = 0.0f;
		State.Intensity.Wind = 0.9f;
		State.Intensity.FogDensity = 0.8f;
		State.Intensity.CloudCoverage = 0.3f;
		State.RoadCondition = EMGRoadCondition::Dry;
		State.Visibility = 300.0f;
		State.WindSpeed = 35.0f;
		State.Temperature = 35.0f;
		break;

	case EMGWeatherType::NightClear:
		State.Intensity.Precipitation = 0.0f;
		State.Intensity.Wind = 0.1f;
		State.Intensity.FogDensity = 0.1f;
		State.Intensity.CloudCoverage = 0.2f;
		State.RoadCondition = EMGRoadCondition::Dry;
		State.Visibility = 8000.0f;
		State.WindSpeed = 3.0f;
		break;

	case EMGWeatherType::NightRain:
		State.Intensity.Precipitation = 0.5f;
		State.Intensity.Wind = 0.4f;
		State.Intensity.FogDensity = 0.3f;
		State.Intensity.CloudCoverage = 0.9f;
		State.RoadCondition = EMGRoadCondition::Wet;
		State.Visibility = 2000.0f;
		State.WindSpeed = 15.0f;
		break;
	}

	// Set wind direction (randomized slightly)
	State.WindDirection = FVector(1.0f, FMath::RandRange(-0.3f, 0.3f), 0.0f).GetSafeNormal();

	return State;
}

FMGLightingPreset UMGWeatherSubsystem::GetDefaultLighting(EMGTimeOfDay Time)
{
	FMGLightingPreset Preset;

	switch (Time)
	{
	case EMGTimeOfDay::Dawn:
		Preset.SunIntensity = 3.0f;
		Preset.SunColor = FLinearColor(1.0f, 0.7f, 0.5f);
		Preset.SunPitch = 10.0f;
		Preset.SkyLightIntensity = 0.5f;
		Preset.AmbientColor = FLinearColor(0.1f, 0.05f, 0.1f);
		Preset.FogColor = FLinearColor(0.8f, 0.6f, 0.5f);
		Preset.HorizonColor = FLinearColor(1.0f, 0.6f, 0.4f);
		break;

	case EMGTimeOfDay::Morning:
		Preset.SunIntensity = 6.0f;
		Preset.SunColor = FLinearColor(1.0f, 0.9f, 0.8f);
		Preset.SunPitch = 30.0f;
		Preset.SkyLightIntensity = 0.8f;
		Preset.AmbientColor = FLinearColor(0.1f, 0.1f, 0.15f);
		Preset.FogColor = FLinearColor(0.7f, 0.75f, 0.8f);
		Preset.HorizonColor = FLinearColor(0.9f, 0.85f, 0.8f);
		break;

	case EMGTimeOfDay::Midday:
		Preset.SunIntensity = 10.0f;
		Preset.SunColor = FLinearColor(1.0f, 1.0f, 0.95f);
		Preset.SunPitch = 70.0f;
		Preset.SkyLightIntensity = 1.0f;
		Preset.AmbientColor = FLinearColor(0.1f, 0.12f, 0.15f);
		Preset.FogColor = FLinearColor(0.6f, 0.7f, 0.8f);
		Preset.HorizonColor = FLinearColor(0.7f, 0.8f, 0.95f);
		break;

	case EMGTimeOfDay::Afternoon:
		Preset.SunIntensity = 8.0f;
		Preset.SunColor = FLinearColor(1.0f, 0.95f, 0.85f);
		Preset.SunPitch = 45.0f;
		Preset.SkyLightIntensity = 0.9f;
		Preset.AmbientColor = FLinearColor(0.1f, 0.1f, 0.12f);
		Preset.FogColor = FLinearColor(0.7f, 0.7f, 0.75f);
		Preset.HorizonColor = FLinearColor(0.85f, 0.8f, 0.75f);
		break;

	case EMGTimeOfDay::Sunset:
		Preset.SunIntensity = 4.0f;
		Preset.SunColor = FLinearColor(1.0f, 0.5f, 0.2f);
		Preset.SunPitch = 5.0f;
		Preset.SkyLightIntensity = 0.4f;
		Preset.AmbientColor = FLinearColor(0.15f, 0.05f, 0.1f);
		Preset.FogColor = FLinearColor(0.9f, 0.5f, 0.3f);
		Preset.HorizonColor = FLinearColor(1.0f, 0.4f, 0.2f);
		break;

	case EMGTimeOfDay::Evening:
		Preset.SunIntensity = 0.5f;
		Preset.SunColor = FLinearColor(0.3f, 0.2f, 0.4f);
		Preset.SunPitch = -15.0f;
		Preset.SkyLightIntensity = 0.2f;
		Preset.AmbientColor = FLinearColor(0.05f, 0.03f, 0.08f);
		Preset.FogColor = FLinearColor(0.2f, 0.15f, 0.25f);
		Preset.HorizonColor = FLinearColor(0.3f, 0.2f, 0.4f);
		break;

	case EMGTimeOfDay::Night:
		Preset.SunIntensity = 0.1f;
		Preset.SunColor = FLinearColor(0.2f, 0.2f, 0.4f);
		Preset.SunPitch = -45.0f;
		Preset.SkyLightIntensity = 0.1f;
		Preset.AmbientColor = FLinearColor(0.02f, 0.02f, 0.05f);
		Preset.FogColor = FLinearColor(0.05f, 0.05f, 0.1f);
		Preset.HorizonColor = FLinearColor(0.1f, 0.1f, 0.2f);
		break;
	}

	return Preset;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGWeatherSubsystem::UpdateWeatherTransition(float MGDeltaTime)
{
	if (!WeatherTransition.bIsTransitioning)
	{
		return;
	}

	// Update progress
	WeatherTransition.Progress += DeltaTime / WeatherTransition.Duration;

	if (WeatherTransition.Progress >= 1.0f)
	{
		// Transition complete
		WeatherTransition.Progress = 1.0f;
		WeatherTransition.bIsTransitioning = false;

		CurrentWeather = WeatherTransition.ToState;
		ApplyWeatherState(CurrentWeather);
		UpdateRoadCondition();

		OnWeatherChanged.Broadcast(CurrentWeather);
		OnWeatherTransitionCompleted.Broadcast(CurrentWeather.Type);
	}
	else
	{
		// Blend states
		FMGWeatherState BlendedState = BlendWeatherStates(
			WeatherTransition.FromState,
			WeatherTransition.ToState,
			WeatherTransition.Progress
		);

		CurrentWeather = BlendedState;
		// Keep target type during transition
		CurrentWeather.Type = WeatherTransition.ToState.Type;

		ApplyWeatherState(CurrentWeather);
	}
}

void UMGWeatherSubsystem::UpdateTimeProgression(float MGDeltaTime)
{
	if (!bTimeProgressionEnabled)
	{
		return;
	}

	// Advance game time
	if (TimeScale > 0.0f)
	{
		float GameMinutesToAdd = DeltaTime / TimeScale;
		SetGameTime(GameTimeMinutes + GameMinutesToAdd);
	}
}

void UMGWeatherSubsystem::UpdateLightingTransition(float MGDeltaTime)
{
	if (LightingTransitionProgress >= 1.0f)
	{
		return;
	}

	// Update progress
	if (LightingTransitionDuration > 0.0f)
	{
		LightingTransitionProgress += DeltaTime / LightingTransitionDuration;
	}
	else
	{
		LightingTransitionProgress = 1.0f;
	}
	LightingTransitionProgress = FMath::Min(LightingTransitionProgress, 1.0f);

	// Blend lighting
	CurrentLighting = BlendLightingPresets(CurrentLighting, TargetLighting, LightingTransitionProgress);
	ApplyLightingPreset(CurrentLighting);
}

void UMGWeatherSubsystem::UpdateWeatherSchedule()
{
	if (WeatherSchedule.Num() == 0)
	{
		return;
	}

	// Find current scheduled weather
	for (int32 i = WeatherSchedule.Num() - 1; i >= 0; i--)
	{
		if (GameTimeMinutes >= WeatherSchedule[i].GameTimeMinutes)
		{
			EMGWeatherType ScheduledWeather = WeatherSchedule[i].WeatherType;

			if (ScheduledWeather != CurrentWeather.Type && !WeatherTransition.bIsTransitioning)
			{
				SetWeather(ScheduledWeather, WeatherSchedule[i].TransitionDuration);
			}
			break;
		}
	}
}

void UMGWeatherSubsystem::UpdateRoadCondition()
{
	EMGRoadCondition OldCondition = CurrentWeather.RoadCondition;

	// Road condition based on weather type and intensity
	if (CurrentWeather.Type == EMGWeatherType::Snow || CurrentWeather.Type == EMGWeatherType::Blizzard)
	{
		if (CurrentWeather.Temperature < -10.0f)
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Icy;
		}
		else
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Snowy;
		}
	}
	else if (IsPrecipitationWeather(CurrentWeather.Type))
	{
		if (CurrentWeather.Intensity.Precipitation > 0.7f)
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::StandingWater;
		}
		else if (CurrentWeather.Intensity.Precipitation > 0.3f)
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Wet;
		}
		else
		{
			CurrentWeather.RoadCondition = EMGRoadCondition::Damp;
		}
	}
	else if (CurrentWeather.Type == EMGWeatherType::Fog || CurrentWeather.Type == EMGWeatherType::HeavyFog)
	{
		CurrentWeather.RoadCondition = EMGRoadCondition::Damp;
	}
	else
	{
		CurrentWeather.RoadCondition = EMGRoadCondition::Dry;
	}

	if (OldCondition != CurrentWeather.RoadCondition)
	{
		OnRoadConditionChanged.Broadcast(CurrentWeather.RoadCondition);
	}
}

void UMGWeatherSubsystem::UpdateLightning(float MGDeltaTime)
{
	if (CurrentWeather.Intensity.LightningFrequency <= 0.0f)
	{
		return;
	}

	// Decrement timer
	NextLightningTime -= DeltaTime;

	if (NextLightningTime <= 0.0f)
	{
		TriggerLightningStrike();

		// Set next lightning time based on frequency
		float MinTime = LightningCooldown * (1.0f - CurrentWeather.Intensity.LightningFrequency);
		float MaxTime = 30.0f * (1.0f - CurrentWeather.Intensity.LightningFrequency * 0.8f);
		NextLightningTime = FMath::RandRange(MinTime, MaxTime);
	}
}

void UMGWeatherSubsystem::ApplyWeatherState(const FMGWeatherState& State)
{
	// Apply fog density
	// Apply cloud coverage
	// Apply wind
	// Update particle effects

	// These would typically affect post-process, sky system, and particles
	// Implementation depends on specific scene setup
}

void UMGWeatherSubsystem::ApplyLightingPreset(const FMGLightingPreset& Preset)
{
	if (SunLight)
	{
		if (UDirectionalLightComponent* LightComp = SunLight->GetComponent())
		{
			LightComp->SetIntensity(Preset.SunIntensity);
			LightComp->SetLightColor(Preset.SunColor);

			// Set rotation based on pitch
			FRotator NewRotation = SunLight->GetActorRotation();
			NewRotation.Pitch = -Preset.SunPitch;
			SunLight->SetActorRotation(NewRotation);
		}
	}
}

FMGWeatherState UMGWeatherSubsystem::BlendWeatherStates(const FMGWeatherState& A, const FMGWeatherState& B, float Alpha)
{
	FMGWeatherState Result;

	// Use smooth step for more natural transitions
	float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	Result.Type = Alpha < 0.5f ? A.Type : B.Type;

	// Blend intensity values
	Result.Intensity.Precipitation = FMath::Lerp(A.Intensity.Precipitation, B.Intensity.Precipitation, SmoothAlpha);
	Result.Intensity.Wind = FMath::Lerp(A.Intensity.Wind, B.Intensity.Wind, SmoothAlpha);
	Result.Intensity.FogDensity = FMath::Lerp(A.Intensity.FogDensity, B.Intensity.FogDensity, SmoothAlpha);
	Result.Intensity.CloudCoverage = FMath::Lerp(A.Intensity.CloudCoverage, B.Intensity.CloudCoverage, SmoothAlpha);
	Result.Intensity.LightningFrequency = FMath::Lerp(A.Intensity.LightningFrequency, B.Intensity.LightningFrequency, SmoothAlpha);

	// Blend other values
	Result.Temperature = FMath::Lerp(A.Temperature, B.Temperature, SmoothAlpha);
	Result.Visibility = FMath::Lerp(A.Visibility, B.Visibility, SmoothAlpha);
	Result.WindSpeed = FMath::Lerp(A.WindSpeed, B.WindSpeed, SmoothAlpha);
	Result.WindDirection = FMath::Lerp(A.WindDirection, B.WindDirection, SmoothAlpha).GetSafeNormal();

	// Road condition - use the worse condition during transition
	Result.RoadCondition = Alpha < 0.5f ? A.RoadCondition : B.RoadCondition;

	return Result;
}

FMGLightingPreset UMGWeatherSubsystem::BlendLightingPresets(const FMGLightingPreset& A, const FMGLightingPreset& B, float Alpha)
{
	FMGLightingPreset Result;

	float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, SmoothAlpha);
	Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, SmoothAlpha);
	Result.SunPitch = FMath::Lerp(A.SunPitch, B.SunPitch, SmoothAlpha);
	Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, SmoothAlpha);
	Result.AmbientColor = FMath::Lerp(A.AmbientColor, B.AmbientColor, SmoothAlpha);
	Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, SmoothAlpha);
	Result.FogStartDistance = FMath::Lerp(A.FogStartDistance, B.FogStartDistance, SmoothAlpha);
	Result.HorizonColor = FMath::Lerp(A.HorizonColor, B.HorizonColor, SmoothAlpha);

	return Result;
}

EMGTimeOfDay UMGWeatherSubsystem::GetTimeOfDayFromGameTime(float TimeMinutes) const
{
	// Convert minutes to hours
	float Hours = TimeMinutes / 60.0f;

	if (Hours >= 5.0f && Hours < 7.0f)
	{
		return EMGTimeOfDay::Dawn;
	}
	else if (Hours >= 7.0f && Hours < 10.0f)
	{
		return EMGTimeOfDay::Morning;
	}
	else if (Hours >= 10.0f && Hours < 14.0f)
	{
		return EMGTimeOfDay::Midday;
	}
	else if (Hours >= 14.0f && Hours < 17.0f)
	{
		return EMGTimeOfDay::Afternoon;
	}
	else if (Hours >= 17.0f && Hours < 19.0f)
	{
		return EMGTimeOfDay::Sunset;
	}
	else if (Hours >= 19.0f && Hours < 22.0f)
	{
		return EMGTimeOfDay::Evening;
	}
	else
	{
		return EMGTimeOfDay::Night;
	}
}

void UMGWeatherSubsystem::TriggerLightningStrike()
{
	OnLightningStrike.Broadcast();

	// Flash effect would be handled by listening systems
}

void UMGWeatherSubsystem::FindSceneReferences()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Find directional light (sun)
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		SunLight = *It;
		break; // Use first found
	}
}

void UMGWeatherSubsystem::UpdateMaterialParameters()
{
	if (!WeatherMPC)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Update MPC with current weather values
	// This would set parameters like:
	// - WetAmount
	// - WindStrength
	// - FogDensity
	// - RainIntensity
	// etc.
}

// ==========================================
// UNIFIED WEATHER API
// ==========================================

UMGWeatherRacingSubsystem* UMGWeatherSubsystem::GetRacingSubsystem() const
{
	// Lazy initialization of racing subsystem reference
	if (!bRacingSubsystemSearched)
	{
		bRacingSubsystemSearched = true;

		if (UWorld* World = GetWorld())
		{
			CachedRacingSubsystem = World->GetSubsystem<UMGWeatherRacingSubsystem>();
		}
	}

	return CachedRacingSubsystem.Get();
}

float UMGWeatherSubsystem::GetUnifiedGripMultiplier(const FVector& VehicleLocation, float VehicleSpeedKPH) const
{
	// Start with base road condition grip
	float BaseGrip = GetRoadGripMultiplier();

	// Check for racing subsystem enhancements
	if (UMGWeatherRacingSubsystem* RacingSubsystem = GetRacingSubsystem())
	{
		const FMGWeatherRacingEffects& Effects = RacingSubsystem->GetCurrentEffects();

		// Apply aquaplaning effects if vehicle is moving through puddles
		if (Effects.bHasPuddles && VehicleSpeedKPH > 80.0f)
		{
			// Aquaplaning reduces grip significantly at speed
			BaseGrip *= Effects.AquaplaningState.GetGripMultiplier();
		}

		// Apply any additional racing-specific grip modifiers
		BaseGrip *= Effects.EffectiveGripMultiplier;
	}

	// Apply precipitation intensity modifier (heavier rain = less grip)
	if (IsPrecipitationWeather(CurrentWeather.Type))
	{
		const float PrecipitationPenalty = CurrentWeather.Intensity.Precipitation * 0.15f;
		BaseGrip -= PrecipitationPenalty;
	}

	// Temperature affects grip (cold tires, icy conditions)
	if (CurrentWeather.Temperature < 5.0f)
	{
		const float ColdPenalty = FMath::Clamp((5.0f - CurrentWeather.Temperature) / 20.0f, 0.0f, 0.2f);
		BaseGrip -= ColdPenalty;
	}

	return FMath::Clamp(BaseGrip, 0.1f, 1.0f);
}

float UMGWeatherSubsystem::GetUnifiedVisibilityDistance(const FVector& Location) const
{
	// Start with base weather visibility
	float Visibility = CurrentWeather.Visibility;

	// Check for racing subsystem enhancements
	if (UMGWeatherRacingSubsystem* RacingSubsystem = GetRacingSubsystem())
	{
		// Racing subsystem may have more detailed visibility calculations
		// (night effects, headlights, local fog patches)
		const float RacingVisibility = RacingSubsystem->GetEffectiveVisibility(Location);

		// Use the more restrictive visibility
		Visibility = FMath::Min(Visibility, RacingVisibility);
	}

	// Apply time-of-day visibility reduction for night
	if (CurrentTimeOfDay == EMGTimeOfDay::Night || CurrentTimeOfDay == EMGTimeOfDay::Evening)
	{
		// Night significantly reduces visibility
		const float NightMultiplier = (CurrentTimeOfDay == EMGTimeOfDay::Night) ? 0.15f : 0.4f;
		Visibility *= NightMultiplier;

		// But not below minimum headlight range
		Visibility = FMath::Max(Visibility, 150.0f);
	}

	return FMath::Max(30.0f, Visibility);
}

float UMGWeatherSubsystem::GetUnifiedAIPerceptionMultiplier() const
{
	float Perception = 1.0f;

	// Fog reduces perception
	if (CurrentWeather.Intensity.FogDensity > 0.1f)
	{
		Perception *= (1.0f - CurrentWeather.Intensity.FogDensity * 0.7f);
	}

	// Rain reduces perception
	if (IsPrecipitationWeather(CurrentWeather.Type))
	{
		Perception *= (1.0f - CurrentWeather.Intensity.Precipitation * 0.3f);
	}

	// Night reduces perception
	if (CurrentTimeOfDay == EMGTimeOfDay::Night)
	{
		Perception *= 0.5f;
	}
	else if (CurrentTimeOfDay == EMGTimeOfDay::Evening)
	{
		Perception *= 0.7f;
	}

	// Check racing subsystem for additional modifiers
	if (UMGWeatherRacingSubsystem* RacingSubsystem = GetRacingSubsystem())
	{
		// Use the more restrictive of the two calculations
		const float RacingPerception = RacingSubsystem->GetAIPerceptionMultiplier();
		Perception = FMath::Min(Perception, RacingPerception);
	}

	return FMath::Clamp(Perception, 0.1f, 1.0f);
}

bool UMGWeatherSubsystem::AreConditionsHazardous() const
{
	// Check road condition
	if (CurrentWeather.RoadCondition == EMGRoadCondition::StandingWater ||
		CurrentWeather.RoadCondition == EMGRoadCondition::Icy)
	{
		return true;
	}

	// Check visibility
	if (CurrentWeather.Visibility < 500.0f)
	{
		return true;
	}

	// Check severe weather types
	switch (CurrentWeather.Type)
	{
	case EMGWeatherType::Thunderstorm:
	case EMGWeatherType::Blizzard:
	case EMGWeatherType::HeavyFog:
	case EMGWeatherType::DustStorm:
		return true;
	default:
		break;
	}

	// Check high wind
	if (CurrentWeather.WindSpeed > 25.0f)
	{
		return true;
	}

	// Check racing subsystem for additional hazards
	if (UMGWeatherRacingSubsystem* RacingSubsystem = GetRacingSubsystem())
	{
		const FMGWeatherRacingEffects& Effects = RacingSubsystem->GetCurrentEffects();

		// Aquaplaning is hazardous
		if (Effects.AquaplaningState.bIsAquaplaning && Effects.AquaplaningState.AquaplaningIntensity > 0.5f)
		{
			return true;
		}
	}

	return false;
}

int32 UMGWeatherSubsystem::GetWeatherDifficultyRating() const
{
	int32 Rating = 1; // Base rating for clear conditions

	// Road condition
	switch (CurrentWeather.RoadCondition)
	{
	case EMGRoadCondition::Damp:
		Rating += 1;
		break;
	case EMGRoadCondition::Wet:
	case EMGRoadCondition::Snowy:
		Rating += 1;
		break;
	case EMGRoadCondition::StandingWater:
		Rating += 2;
		break;
	case EMGRoadCondition::Icy:
		Rating += 2;
		break;
	default:
		break;
	}

	// Visibility
	if (CurrentWeather.Visibility < 1000.0f)
	{
		Rating += 1;
	}
	if (CurrentWeather.Visibility < 300.0f)
	{
		Rating += 1;
	}

	// Time of day
	if (CurrentTimeOfDay == EMGTimeOfDay::Night)
	{
		Rating += 1;
	}

	// Wind
	if (CurrentWeather.WindSpeed > 30.0f)
	{
		Rating += 1;
	}

	// Check racing subsystem for additional difficulty
	if (UMGWeatherRacingSubsystem* RacingSubsystem = GetRacingSubsystem())
	{
		// Use the racing subsystem's difficulty calculation if available
		const FMGWeatherRacingEffects& Effects = RacingSubsystem->GetCurrentEffects();
		const int32 RacingDifficulty = Effects.GetDifficultyRating();

		// Take the higher of the two ratings
		Rating = FMath::Max(Rating, RacingDifficulty);
	}

	return FMath::Clamp(Rating, 1, 5);
}

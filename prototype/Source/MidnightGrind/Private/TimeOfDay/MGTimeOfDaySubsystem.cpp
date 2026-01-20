// Copyright Midnight Grind. All Rights Reserved.

#include "TimeOfDay/MGTimeOfDaySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGTimeOfDaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializePeriodSettings();
	CurrentTimePeriod = CalculateTimePeriod(CurrentTimeHours);
	UpdateLightingSettings();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TimeUpdateHandle,
			[this]() { UpdateTime(0.1f); },
			0.1f,
			true
		);
	}
}

void UMGTimeOfDaySubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimeUpdateHandle);
	}
	Super::Deinitialize();
}

FText UMGTimeOfDaySubsystem::GetTimeString() const
{
	int32 Hour = GetCurrentHour();
	int32 Minute = GetCurrentMinute();
	bool bPM = Hour >= 12;
	int32 DisplayHour = Hour % 12;
	if (DisplayHour == 0) DisplayHour = 12;

	return FText::FromString(FString::Printf(TEXT("%d:%02d %s"),
		DisplayHour, Minute, bPM ? TEXT("PM") : TEXT("AM")));
}

bool UMGTimeOfDaySubsystem::IsNightTime() const
{
	return CurrentTimePeriod == EMGTimeOfDay::Night ||
		   CurrentTimePeriod == EMGTimeOfDay::LateNight ||
		   CurrentTimePeriod == EMGTimeOfDay::Evening;
}

bool UMGTimeOfDaySubsystem::IsMidnightHour() const
{
	int32 Hour = GetCurrentHour();
	return Hour == 0 || Hour == 23;
}

void UMGTimeOfDaySubsystem::SetTime(float TimeHours)
{
	CurrentTimeHours = FMath::Fmod(TimeHours, 24.0f);
	if (CurrentTimeHours < 0.0f)
		CurrentTimeHours += 24.0f;

	EMGTimeOfDay NewPeriod = CalculateTimePeriod(CurrentTimeHours);
	if (NewPeriod != CurrentTimePeriod)
	{
		EMGTimeOfDay OldPeriod = CurrentTimePeriod;
		CurrentTimePeriod = NewPeriod;
		OnTimeOfDayChanged.Broadcast(OldPeriod, NewPeriod);
	}

	UpdateLightingSettings();
}

void UMGTimeOfDaySubsystem::SetTimePeriod(EMGTimeOfDay Period)
{
	float TargetTime = 12.0f;

	switch (Period)
	{
	case EMGTimeOfDay::Dawn: TargetTime = 6.0f; break;
	case EMGTimeOfDay::Morning: TargetTime = 9.0f; break;
	case EMGTimeOfDay::Midday: TargetTime = 12.5f; break;
	case EMGTimeOfDay::Afternoon: TargetTime = 15.5f; break;
	case EMGTimeOfDay::Dusk: TargetTime = 18.0f; break;
	case EMGTimeOfDay::Evening: TargetTime = 20.5f; break;
	case EMGTimeOfDay::Night: TargetTime = 23.0f; break;
	case EMGTimeOfDay::LateNight: TargetTime = 3.0f; break;
	}

	SetTime(TargetTime);
}

void UMGTimeOfDaySubsystem::SetTimeScale(float Scale)
{
	TimeScale = FMath::Clamp(Scale, 0.0f, 3600.0f);
}

void UMGTimeOfDaySubsystem::PauseTime(bool bPause)
{
	bTimePaused = bPause;
}

void UMGTimeOfDaySubsystem::SyncWithServer(float ServerTimeHours, int32 ServerGameDay)
{
	CurrentTimeHours = ServerTimeHours;
	GameDay = ServerGameDay;
	CurrentTimePeriod = CalculateTimePeriod(CurrentTimeHours);
	UpdateLightingSettings();
}

float UMGTimeOfDaySubsystem::GetSunAngle() const
{
	// Sun rises at 6:00, peaks at 12:00, sets at 18:00
	float NormalizedDayTime = FMath::Fmod(CurrentTimeHours - 6.0f, 24.0f);
	if (NormalizedDayTime < 0.0f)
		NormalizedDayTime += 24.0f;

	if (NormalizedDayTime <= 12.0f)
	{
		// Sun is up
		return (NormalizedDayTime / 12.0f) * 180.0f;
	}

	return -1.0f; // Sun is below horizon
}

FRotator UMGTimeOfDaySubsystem::GetSunRotation() const
{
	float Angle = GetSunAngle();
	if (Angle < 0.0f)
	{
		return FRotator(-30.0f, 0.0f, 0.0f); // Below horizon
	}

	float Pitch = -90.0f + Angle;
	return FRotator(Pitch, 0.0f, 0.0f);
}

FMGTimePeriodEvents UMGTimeOfDaySubsystem::GetPeriodEvents() const
{
	if (const FMGTimePeriodEvents* Events = PeriodEvents.Find(CurrentTimePeriod))
	{
		return *Events;
	}
	return FMGTimePeriodEvents();
}

float UMGTimeOfDaySubsystem::GetCurrentReputationMultiplier() const
{
	FMGTimePeriodEvents Events = GetPeriodEvents();
	return Events.ReputationMultiplier;
}

float UMGTimeOfDaySubsystem::GetCurrentCashMultiplier() const
{
	FMGTimePeriodEvents Events = GetPeriodEvents();
	return Events.CashMultiplier;
}

void UMGTimeOfDaySubsystem::UpdateTime(float DeltaTime)
{
	if (bTimePaused)
		return;

	float PreviousTime = CurrentTimeHours;
	int32 PreviousHour = GetCurrentHour();

	// Advance time: TimeScale seconds of game time per real second
	CurrentTimeHours += (DeltaTime * TimeScale) / 3600.0f;

	// Handle day rollover
	if (CurrentTimeHours >= 24.0f)
	{
		CurrentTimeHours = FMath::Fmod(CurrentTimeHours, 24.0f);
		GameDay++;
		OnMidnightReached.Broadcast(GameDay);
	}

	// Check for hour change
	int32 NewHour = GetCurrentHour();
	if (NewHour != PreviousHour && NewHour != LastHour)
	{
		LastHour = NewHour;
		OnHourChanged.Broadcast(NewHour);
	}

	// Check for time period change
	EMGTimeOfDay NewPeriod = CalculateTimePeriod(CurrentTimeHours);
	if (NewPeriod != CurrentTimePeriod)
	{
		EMGTimeOfDay OldPeriod = CurrentTimePeriod;
		CurrentTimePeriod = NewPeriod;
		OnTimeOfDayChanged.Broadcast(OldPeriod, NewPeriod);
	}

	UpdateLightingSettings();
}

void UMGTimeOfDaySubsystem::UpdateLightingSettings()
{
	CurrentSettings = InterpolateSettings(CurrentTimeHours);
}

EMGTimeOfDay UMGTimeOfDaySubsystem::CalculateTimePeriod(float TimeHours) const
{
	if (TimeHours >= 5.0f && TimeHours < 7.0f) return EMGTimeOfDay::Dawn;
	if (TimeHours >= 7.0f && TimeHours < 11.0f) return EMGTimeOfDay::Morning;
	if (TimeHours >= 11.0f && TimeHours < 14.0f) return EMGTimeOfDay::Midday;
	if (TimeHours >= 14.0f && TimeHours < 17.0f) return EMGTimeOfDay::Afternoon;
	if (TimeHours >= 17.0f && TimeHours < 19.0f) return EMGTimeOfDay::Dusk;
	if (TimeHours >= 19.0f && TimeHours < 22.0f) return EMGTimeOfDay::Evening;
	if (TimeHours >= 22.0f || TimeHours < 2.0f) return EMGTimeOfDay::Night;
	return EMGTimeOfDay::LateNight;
}

FMGTimeOfDaySettings UMGTimeOfDaySubsystem::InterpolateSettings(float TimeHours) const
{
	EMGTimeOfDay CurrentPeriodCalc = CalculateTimePeriod(TimeHours);

	if (const FMGTimeOfDaySettings* Settings = PeriodSettings.Find(CurrentPeriodCalc))
	{
		return *Settings;
	}

	return FMGTimeOfDaySettings();
}

void UMGTimeOfDaySubsystem::InitializePeriodSettings()
{
	// Dawn - Warm golden hour
	FMGTimeOfDaySettings Dawn;
	Dawn.SunIntensity = 0.6f;
	Dawn.SkyBrightness = 0.7f;
	Dawn.SunColor = FLinearColor(1.0f, 0.7f, 0.4f);
	Dawn.AmbientColor = FLinearColor(0.4f, 0.35f, 0.5f);
	Dawn.NeonIntensity = 0.3f;
	Dawn.StreetLightIntensity = 0.2f;
	Dawn.TrafficDensityMultiplier = 0.3f;
	Dawn.PedestrianDensityMultiplier = 0.2f;
	PeriodSettings.Add(EMGTimeOfDay::Dawn, Dawn);

	// Morning
	FMGTimeOfDaySettings Morning;
	Morning.SunIntensity = 0.9f;
	Morning.SkyBrightness = 0.9f;
	Morning.SunColor = FLinearColor(1.0f, 0.95f, 0.85f);
	Morning.AmbientColor = FLinearColor(0.5f, 0.55f, 0.6f);
	Morning.NeonIntensity = 0.0f;
	Morning.StreetLightIntensity = 0.0f;
	Morning.TrafficDensityMultiplier = 0.8f;
	Morning.PedestrianDensityMultiplier = 0.7f;
	PeriodSettings.Add(EMGTimeOfDay::Morning, Morning);

	// Midday
	FMGTimeOfDaySettings Midday;
	Midday.SunIntensity = 1.0f;
	Midday.SkyBrightness = 1.0f;
	Midday.SunColor = FLinearColor(1.0f, 1.0f, 0.95f);
	Midday.AmbientColor = FLinearColor(0.6f, 0.65f, 0.7f);
	Midday.NeonIntensity = 0.0f;
	Midday.StreetLightIntensity = 0.0f;
	Midday.TrafficDensityMultiplier = 1.0f;
	Midday.PedestrianDensityMultiplier = 1.0f;
	PeriodSettings.Add(EMGTimeOfDay::Midday, Midday);

	// Afternoon
	FMGTimeOfDaySettings Afternoon;
	Afternoon.SunIntensity = 0.95f;
	Afternoon.SkyBrightness = 0.95f;
	Afternoon.SunColor = FLinearColor(1.0f, 0.9f, 0.8f);
	Afternoon.AmbientColor = FLinearColor(0.55f, 0.55f, 0.6f);
	Afternoon.NeonIntensity = 0.0f;
	Afternoon.StreetLightIntensity = 0.0f;
	Afternoon.TrafficDensityMultiplier = 1.2f;
	Afternoon.PedestrianDensityMultiplier = 1.0f;
	PeriodSettings.Add(EMGTimeOfDay::Afternoon, Afternoon);

	// Dusk - Golden/purple hour
	FMGTimeOfDaySettings Dusk;
	Dusk.SunIntensity = 0.5f;
	Dusk.SkyBrightness = 0.6f;
	Dusk.SunColor = FLinearColor(1.0f, 0.5f, 0.2f);
	Dusk.AmbientColor = FLinearColor(0.5f, 0.4f, 0.6f);
	Dusk.NeonIntensity = 0.5f;
	Dusk.StreetLightIntensity = 0.5f;
	Dusk.TrafficDensityMultiplier = 1.3f;
	Dusk.PedestrianDensityMultiplier = 0.8f;
	PeriodSettings.Add(EMGTimeOfDay::Dusk, Dusk);

	// Evening - Prime racing time starts
	FMGTimeOfDaySettings Evening;
	Evening.SunIntensity = 0.0f;
	Evening.MoonIntensity = 0.2f;
	Evening.SkyBrightness = 0.3f;
	Evening.AmbientColor = FLinearColor(0.2f, 0.2f, 0.35f);
	Evening.NeonIntensity = 0.8f;
	Evening.StreetLightIntensity = 1.0f;
	Evening.TrafficDensityMultiplier = 0.9f;
	Evening.PedestrianDensityMultiplier = 0.6f;
	Evening.StarVisibility = 0.3f;
	PeriodSettings.Add(EMGTimeOfDay::Evening, Evening);

	// Night - The Grind
	FMGTimeOfDaySettings Night;
	Night.SunIntensity = 0.0f;
	Night.MoonIntensity = 0.4f;
	Night.SkyBrightness = 0.15f;
	Night.AmbientColor = FLinearColor(0.1f, 0.1f, 0.2f);
	Night.NeonIntensity = 1.0f;
	Night.StreetLightIntensity = 1.0f;
	Night.TrafficDensityMultiplier = 0.5f;
	Night.PedestrianDensityMultiplier = 0.3f;
	Night.StarVisibility = 0.8f;
	PeriodSettings.Add(EMGTimeOfDay::Night, Night);

	// Late Night - Underground scene peaks
	FMGTimeOfDaySettings LateNight;
	LateNight.SunIntensity = 0.0f;
	LateNight.MoonIntensity = 0.3f;
	LateNight.SkyBrightness = 0.1f;
	LateNight.AmbientColor = FLinearColor(0.05f, 0.05f, 0.15f);
	LateNight.NeonIntensity = 1.0f;
	LateNight.StreetLightIntensity = 0.8f;
	LateNight.TrafficDensityMultiplier = 0.2f;
	LateNight.PedestrianDensityMultiplier = 0.1f;
	LateNight.StarVisibility = 1.0f;
	PeriodSettings.Add(EMGTimeOfDay::LateNight, LateNight);

	// Period Events
	FMGTimePeriodEvents NightEvents;
	NightEvents.TimePeriod = EMGTimeOfDay::Night;
	NightEvents.bMidnightRacesAvailable = true;
	NightEvents.ReputationMultiplier = 1.5f;
	NightEvents.CashMultiplier = 1.25f;
	PeriodEvents.Add(EMGTimeOfDay::Night, NightEvents);

	FMGTimePeriodEvents LateNightEvents;
	LateNightEvents.TimePeriod = EMGTimeOfDay::LateNight;
	LateNightEvents.bMidnightRacesAvailable = true;
	LateNightEvents.bCopActivityIncreased = true;
	LateNightEvents.ReputationMultiplier = 2.0f;
	LateNightEvents.CashMultiplier = 1.5f;
	PeriodEvents.Add(EMGTimeOfDay::LateNight, LateNightEvents);

	FMGTimePeriodEvents MiddayEvents;
	MiddayEvents.TimePeriod = EMGTimeOfDay::Midday;
	MiddayEvents.ReputationMultiplier = 0.75f;
	MiddayEvents.CashMultiplier = 1.0f;
	PeriodEvents.Add(EMGTimeOfDay::Midday, MiddayEvents);
}

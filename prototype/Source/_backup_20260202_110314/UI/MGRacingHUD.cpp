// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGRacingHUD.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Weather/MGWeatherSubsystem.h"
#include "Kismet/KismetTextLibrary.h"

UMGRacingHUD::UMGRacingHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMGRacingHUD::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMGRacingHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update cached state
	UpdateCachedState();

	// Check for nitrous activation
	if (CachedState.bNitrousActive && !bWasNitrousActive)
	{
		OnNitrousActivate();
	}
	else if (!CachedState.bNitrousActive && bWasNitrousActive)
	{
		OnNitrousDeplete();
	}
	bWasNitrousActive = CachedState.bNitrousActive;

	// Notify Blueprint of update
	OnVehicleStateUpdated();
}

void UMGRacingHUD::SetVehicle(AMGVehiclePawn* Vehicle)
{
	TargetVehicle = Vehicle;

	if (Vehicle)
	{
		BindVehicleEvents();
		UpdateCachedState();
	}
}

void UMGRacingHUD::ApplyConfig(const FMGHUDConfig& Config)
{
	HUDConfig = Config;
}

void UMGRacingHUD::UpdateCachedState()
{
	if (AMGVehiclePawn* Vehicle = TargetVehicle.Get())
	{
		CachedState = Vehicle->GetRuntimeState();
	}
}

void UMGRacingHUD::BindVehicleEvents()
{
	if (AMGVehiclePawn* Vehicle = TargetVehicle.Get())
	{
		// Bind to vehicle events
		Vehicle->OnLapCompleted.AddDynamic(this, &UMGRacingHUD::TriggerLapCompleteAnimation);
	}
}

// ==========================================
// SPEED DATA
// ==========================================

float UMGRacingHUD::GetCurrentSpeed() const
{
	return HUDConfig.bUseMPH ? CachedState.SpeedMPH : CachedState.SpeedKPH;
}

FText UMGRacingHUD::GetSpeedText() const
{
	int32 Speed = FMath::RoundToInt(GetCurrentSpeed());
	return FText::AsNumber(Speed);
}

FText UMGRacingHUD::GetSpeedUnitText() const
{
	return HUDConfig.bUseMPH
		? NSLOCTEXT("HUD", "MPH", "MPH")
		: NSLOCTEXT("HUD", "KPH", "KPH");
}

// ==========================================
// ENGINE DATA
// ==========================================

float UMGRacingHUD::GetRPMPercent() const
{
	return FMath::Clamp(CachedState.RPM / MaxDisplayRPM, 0.0f, 1.0f);
}

FText UMGRacingHUD::GetRPMText() const
{
	int32 RPM = FMath::RoundToInt(CachedState.RPM);
	return FText::AsNumber(RPM);
}

FText UMGRacingHUD::GetGearText() const
{
	int32 Gear = CachedState.CurrentGear;

	if (Gear == 0)
	{
		return NSLOCTEXT("HUD", "Gear_N", "N");
	}
	else if (Gear < 0)
	{
		return NSLOCTEXT("HUD", "Gear_R", "R");
	}
	else
	{
		return FText::AsNumber(Gear);
	}
}

int32 UMGRacingHUD::GetCurrentGear() const
{
	return CachedState.CurrentGear;
}

bool UMGRacingHUD::IsRevLimiterActive() const
{
	return CachedState.bRevLimiter;
}

// ==========================================
// BOOST/NITROUS DATA
// ==========================================

float UMGRacingHUD::GetBoostPercent() const
{
	// Normalize boost PSI to 0-1 (assuming max ~25 PSI)
	return FMath::Clamp(CachedState.BoostPSI / 25.0f, 0.0f, 1.0f);
}

FText UMGRacingHUD::GetBoostText() const
{
	return FText::Format(NSLOCTEXT("HUD", "BoostFormat", "{0} PSI"),
		FText::AsNumber(FMath::RoundToInt(CachedState.BoostPSI)));
}

float UMGRacingHUD::GetNitrousPercent() const
{
	return CachedState.NitrousPercent / 100.0f;
}

bool UMGRacingHUD::IsNitrousActive() const
{
	return CachedState.bNitrousActive;
}

// ==========================================
// RACE DATA
// ==========================================

int32 UMGRacingHUD::GetCurrentLap() const
{
	return CachedState.CurrentLap;
}

int32 UMGRacingHUD::GetTotalLaps() const
{
	return TotalLapCount;
}

FText UMGRacingHUD::GetLapText() const
{
	return FText::Format(NSLOCTEXT("HUD", "LapFormat", "{0}/{1}"),
		FText::AsNumber(FMath::Max(1, CachedState.CurrentLap)),
		FText::AsNumber(TotalLapCount));
}

int32 UMGRacingHUD::GetRacePosition() const
{
	return CachedState.RacePosition;
}

FText UMGRacingHUD::GetPositionText() const
{
	int32 Position = FMath::Max(1, CachedState.RacePosition);
	return FText::Format(NSLOCTEXT("HUD", "PositionFormat", "{0}{1}"),
		FText::AsNumber(Position),
		GetOrdinalSuffix(Position));
}

FText UMGRacingHUD::GetLapTimeText() const
{
	return FormatLapTime(CachedState.CurrentLapTime);
}

FText UMGRacingHUD::GetBestLapTimeText() const
{
	if (CachedState.BestLapTime <= 0.0f)
	{
		return NSLOCTEXT("HUD", "NoBestTime", "--:--.---");
	}
	return FormatLapTime(CachedState.BestLapTime);
}

FText UMGRacingHUD::GetTotalTimeText() const
{
	return FormatLapTime(CachedState.TotalRaceTime);
}

// ==========================================
// DRIFT DATA
// ==========================================

bool UMGRacingHUD::IsDrifting() const
{
	return CachedState.bIsDrifting;
}

float UMGRacingHUD::GetDriftAngle() const
{
	return CachedState.DriftAngle;
}

float UMGRacingHUD::GetDriftScore() const
{
	return CachedState.DriftScore;
}

FText UMGRacingHUD::GetDriftScoreText() const
{
	int32 Score = FMath::RoundToInt(CachedState.DriftScore);
	return FText::AsNumber(Score);
}

float UMGRacingHUD::GetDriftMultiplier() const
{
	// Calculate multiplier based on drift angle
	float AngleMultiplier = FMath::Abs(CachedState.DriftAngle) / 45.0f;
	return FMath::Clamp(1.0f + AngleMultiplier, 1.0f, 5.0f);
}

// ==========================================
// VISUAL HELPERS
// ==========================================

float UMGRacingHUD::GetTachNeedleRotation() const
{
	// Map RPM 0-max to 0-270 degrees
	float NormalizedRPM = GetRPMPercent();
	return NormalizedRPM * 270.0f;
}

float UMGRacingHUD::GetSpeedNeedleRotation() const
{
	// Map speed 0-max to 0-270 degrees
	float NormalizedSpeed = FMath::Clamp(GetCurrentSpeed() / MaxDisplaySpeed, 0.0f, 1.0f);
	return NormalizedSpeed * 270.0f;
}

FLinearColor UMGRacingHUD::GetRPMZoneColor() const
{
	float RPM = CachedState.RPM;

	if (RPM >= RedlineRPM)
	{
		return RPMRedlineColor;
	}
	else if (RPM >= OptimalShiftRPM)
	{
		return RPMWarningColor;
	}
	else
	{
		return RPMNormalColor;
	}
}

FLinearColor UMGRacingHUD::GetGearColor() const
{
	if (ShouldShowShiftIndicator())
	{
		return ShiftColor;
	}
	return GearColor;
}

bool UMGRacingHUD::ShouldShowShiftIndicator() const
{
	// Show shift indicator when approaching optimal shift point
	return CachedState.RPM >= OptimalShiftRPM && CachedState.CurrentGear > 0;
}

FText UMGRacingHUD::FormatLapTime(float TimeInSeconds)
{
	if (TimeInSeconds < 0.0f)
	{
		return NSLOCTEXT("HUD", "InvalidTime", "--:--.---");
	}

	int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
	float Seconds = FMath::Fmod(TimeInSeconds, 60.0f);
	int32 WholeSeconds = FMath::FloorToInt(Seconds);
	int32 Milliseconds = FMath::FloorToInt((Seconds - WholeSeconds) * 1000.0f);

	return FText::Format(
		NSLOCTEXT("HUD", "TimeFormat", "{0}:{1}.{2}"),
		FText::AsNumber(Minutes),
		FText::FromString(FString::Printf(TEXT("%02d"), WholeSeconds)),
		FText::FromString(FString::Printf(TEXT("%03d"), Milliseconds))
	);
}

FText UMGRacingHUD::GetOrdinalSuffix(int32 Number)
{
	// Handle special cases for 11, 12, 13
	int32 Mod100 = Number % 100;
	if (Mod100 >= 11 && Mod100 <= 13)
	{
		return NSLOCTEXT("HUD", "Ordinal_TH", "th");
	}

	switch (Number % 10)
	{
	case 1:
		return NSLOCTEXT("HUD", "Ordinal_ST", "st");
	case 2:
		return NSLOCTEXT("HUD", "Ordinal_ND", "nd");
	case 3:
		return NSLOCTEXT("HUD", "Ordinal_RD", "rd");
	default:
		return NSLOCTEXT("HUD", "Ordinal_TH", "th");
	}
}

// ==========================================
// EVENTS/ANIMATIONS
// ==========================================

void UMGRacingHUD::TriggerLapCompleteAnimation(int32 LapNumber, float LapTime, bool bNewBest)
{
	OnLapComplete(LapNumber, LapTime, bNewBest);
}

void UMGRacingHUD::TriggerCheckpointAnimation(float SplitTime, bool bAhead)
{
	OnCheckpointPassed(SplitTime, bAhead);
}

void UMGRacingHUD::TriggerDriftScorePopup(float Score)
{
	OnDriftScoreAwarded(Score);
}

void UMGRacingHUD::TriggerNitrousFlash()
{
	OnNitrousActivate();
}

void UMGRacingHUD::ShowCountdown(int32 CountdownValue)
{
	OnCountdownTick(CountdownValue);
}

void UMGRacingHUD::ShowGoSignal()
{
	OnRaceStart();
}

// ==========================================
// WEATHER DATA
// ==========================================

int32 UMGRacingHUD::GetWeatherDifficulty() const
{
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			return WeatherSubsystem->GetWeatherDifficultyRating();
		}
	}
	return 1; // Default to easy if no weather system
}

FText UMGRacingHUD::GetWeatherDifficultyText() const
{
	const int32 Difficulty = GetWeatherDifficulty();

	switch (Difficulty)
	{
	case 1:
		return NSLOCTEXT("HUD", "Weather_Easy", "Clear");
	case 2:
		return NSLOCTEXT("HUD", "Weather_Moderate", "Moderate");
	case 3:
		return NSLOCTEXT("HUD", "Weather_Challenging", "Challenging");
	case 4:
		return NSLOCTEXT("HUD", "Weather_Severe", "Severe");
	case 5:
	default:
		return NSLOCTEXT("HUD", "Weather_Extreme", "Extreme");
	}
}

FText UMGRacingHUD::GetWeatherTypeText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			return UMGWeatherSubsystem::GetWeatherDisplayName(WeatherSubsystem->GetCurrentWeatherType());
		}
	}
	return NSLOCTEXT("HUD", "Weather_Unknown", "Unknown");
}

float UMGRacingHUD::GetCurrentGripLevel() const
{
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			// Get grip at vehicle location if available
			FVector Location = FVector::ZeroVector;
			float SpeedKPH = 0.0f;

			if (TargetVehicle.IsValid())
			{
				Location = TargetVehicle->GetActorLocation();
				// Assume cached state has speed in some form
				SpeedKPH = GetCurrentSpeed() * (HUDConfig.bUseMPH ? 1.60934f : 1.0f);
			}

			return WeatherSubsystem->GetUnifiedGripMultiplier(Location, SpeedKPH);
		}
	}
	return 1.0f;
}

FText UMGRacingHUD::GetGripLevelText() const
{
	const float Grip = GetCurrentGripLevel();
	const int32 GripPercent = FMath::RoundToInt(Grip * 100.0f);

	return FText::Format(
		NSLOCTEXT("HUD", "GripFormat", "{0}%"),
		FText::AsNumber(GripPercent)
	);
}

bool UMGRacingHUD::AreConditionsHazardous() const
{
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			return WeatherSubsystem->AreConditionsHazardous();
		}
	}
	return false;
}

FText UMGRacingHUD::GetHazardWarningText() const
{
	if (!AreConditionsHazardous())
	{
		return FText::GetEmpty();
	}

	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			const float Grip = GetCurrentGripLevel();
			const float Visibility = WeatherSubsystem->GetUnifiedVisibilityDistance();

			// Prioritize the most severe warning
			if (Grip < 0.4f)
			{
				return NSLOCTEXT("HUD", "Hazard_LowGrip", "LOW GRIP");
			}
			else if (Visibility < 200.0f)
			{
				return NSLOCTEXT("HUD", "Hazard_LowVis", "LOW VISIBILITY");
			}
			else if (WeatherSubsystem->GetCurrentWeather().WindSpeed > 30.0f)
			{
				return NSLOCTEXT("HUD", "Hazard_StrongWind", "STRONG WIND");
			}
			else
			{
				return NSLOCTEXT("HUD", "Hazard_Caution", "CAUTION");
			}
		}
	}

	return NSLOCTEXT("HUD", "Hazard_Caution", "CAUTION");
}

float UMGRacingHUD::GetVisibilityPercent() const
{
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			// Max visibility is around 15000m (clear day)
			const float Visibility = WeatherSubsystem->GetUnifiedVisibilityDistance();
			return FMath::Clamp(Visibility / 10000.0f, 0.0f, 1.0f);
		}
	}
	return 1.0f;
}

FName UMGRacingHUD::GetWeatherIconName() const
{
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			const EMGWeatherType Type = WeatherSubsystem->GetCurrentWeatherType();

			switch (Type)
			{
			case EMGWeatherType::Clear:
				return FName(TEXT("Weather_Clear"));
			case EMGWeatherType::PartlyCloudy:
				return FName(TEXT("Weather_PartlyCloudy"));
			case EMGWeatherType::Overcast:
				return FName(TEXT("Weather_Cloudy"));
			case EMGWeatherType::LightRain:
				return FName(TEXT("Weather_LightRain"));
			case EMGWeatherType::HeavyRain:
				return FName(TEXT("Weather_HeavyRain"));
			case EMGWeatherType::Thunderstorm:
				return FName(TEXT("Weather_Storm"));
			case EMGWeatherType::Fog:
			case EMGWeatherType::HeavyFog:
				return FName(TEXT("Weather_Fog"));
			case EMGWeatherType::Snow:
				return FName(TEXT("Weather_Snow"));
			case EMGWeatherType::Blizzard:
				return FName(TEXT("Weather_Blizzard"));
			case EMGWeatherType::DustStorm:
				return FName(TEXT("Weather_Dust"));
			case EMGWeatherType::NightClear:
				return FName(TEXT("Weather_NightClear"));
			case EMGWeatherType::NightRain:
				return FName(TEXT("Weather_NightRain"));
			default:
				return FName(TEXT("Weather_Unknown"));
			}
		}
	}
	return FName(TEXT("Weather_Unknown"));
}

FLinearColor UMGRacingHUD::GetWeatherIndicatorColor() const
{
	const int32 Difficulty = GetWeatherDifficulty();

	switch (Difficulty)
	{
	case 1:
		return WeatherSafeColor;
	case 2:
		return FMath::Lerp(WeatherSafeColor, WeatherCautionColor, 0.5f);
	case 3:
		return WeatherCautionColor;
	case 4:
		return WeatherDangerColor;
	case 5:
	default:
		return WeatherExtremeColor;
	}
}

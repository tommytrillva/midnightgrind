// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGWeatherRaceHandler.h"
#include "Environment/MGWeatherRacingEffects.h"
#include "Weather/MGWeatherSubsystem.h"
#include "AI/MGRacingAIController.h"
#include "GameModes/MGRaceGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UMGWeatherRaceHandler::UMGWeatherRaceHandler()
{
	ActiveWeatherRaceType = EMGWeatherRaceType::Standard;
	BaseRaceType = EMGRaceType::Circuit;
}

// ============================================================================
// UMGRaceTypeHandler INTERFACE
// ============================================================================

void UMGWeatherRaceHandler::Initialize(AMGRaceGameMode* InGameMode)
{
	Super::Initialize(InGameMode);

	InitializeSubsystems();
}

void UMGWeatherRaceHandler::Activate()
{
	Super::Activate();

	// Apply weather race type configuration
	if (WeatherRacingSubsystem.IsValid())
	{
		if (bUsingCustomConfig)
		{
			WeatherRacingSubsystem->ApplyWeatherRaceConfig(CustomConfig);
		}
		else
		{
			WeatherRacingSubsystem->SetWeatherRaceType(ActiveWeatherRaceType);
		}

		// Bind to events
		WeatherRacingSubsystem->OnPuddleEntered.AddDynamic(this, &UMGWeatherRaceHandler::OnPuddleHit);
		WeatherRacingSubsystem->OnWindGust.AddDynamic(this, &UMGWeatherRaceHandler::OnWindGustOccurred);
	}

	UE_LOG(LogTemp, Log, TEXT("MGWeatherRaceHandler: Activated with race type %d"),
		static_cast<int32>(ActiveWeatherRaceType));
}

void UMGWeatherRaceHandler::Deactivate()
{
	// Unbind events
	if (WeatherRacingSubsystem.IsValid())
	{
		WeatherRacingSubsystem->OnPuddleEntered.RemoveDynamic(this, &UMGWeatherRaceHandler::OnPuddleHit);
		WeatherRacingSubsystem->OnWindGust.RemoveDynamic(this, &UMGWeatherRaceHandler::OnWindGustOccurred);

		// Reset to standard weather
		WeatherRacingSubsystem->SetWeatherRaceType(EMGWeatherRaceType::Standard);
	}

	Super::Deactivate();
}

void UMGWeatherRaceHandler::Reset()
{
	Super::Reset();

	PuddlesHit = 0;
	TotalAquaplaningTime = 0.0f;
	MaxGustSurvived = 0.0f;
	bWasAquaplaning = false;
}

void UMGWeatherRaceHandler::OnRaceStarted()
{
	Super::OnRaceStarted();

	// Update all AI controllers with weather modifiers
	UpdateAIForWeather();
}

void UMGWeatherRaceHandler::OnRaceTick(float MGDeltaTime)
{
	Super::OnRaceTick(DeltaTime);

	// Update weather effects
	UpdateWeatherEffects(DeltaTime);

	// Track statistics
	TrackAquaplaning(DeltaTime);
	TrackWindGusts();
}

void UMGWeatherRaceHandler::OnRaceEnded()
{
	Super::OnRaceEnded();

	// Log final weather race statistics
	UE_LOG(LogTemp, Log, TEXT("MGWeatherRaceHandler: Race ended - Puddles hit: %d, Aquaplaning time: %.1fs, Max gust: %.2f"),
		PuddlesHit, TotalAquaplaningTime, MaxGustSurvived);
}

EMGRaceType UMGWeatherRaceHandler::GetRaceType() const
{
	// Return the base race type (Circuit, Sprint, etc.)
	return BaseRaceType;
}

FText UMGWeatherRaceHandler::GetDisplayName() const
{
	// Get the weather race type display name
	switch (ActiveWeatherRaceType)
	{
	case EMGWeatherRaceType::RainRace:
		return NSLOCTEXT("WeatherRace", "RainRace", "Rain Race");
	case EMGWeatherRaceType::MidnightRun:
		return NSLOCTEXT("WeatherRace", "MidnightRun", "Midnight Run");
	case EMGWeatherRaceType::FogRally:
		return NSLOCTEXT("WeatherRace", "FogRally", "Fog Rally");
	case EMGWeatherRaceType::StormChase:
		return NSLOCTEXT("WeatherRace", "StormChase", "Storm Chase");
	case EMGWeatherRaceType::WindSprint:
		return NSLOCTEXT("WeatherRace", "WindSprint", "Wind Sprint");
	default:
		return NSLOCTEXT("WeatherRace", "Standard", "Standard Race");
	}
}

FText UMGWeatherRaceHandler::GetDescription() const
{
	switch (ActiveWeatherRaceType)
	{
	case EMGWeatherRaceType::RainRace:
		return NSLOCTEXT("WeatherRace", "RainRaceDesc",
			"Race through the rain. Watch for puddles that cause aquaplaning. Reduced grip on wet surfaces. "
			"Bonus rewards for completing this challenging event.");

	case EMGWeatherRaceType::MidnightRun:
		return NSLOCTEXT("WeatherRace", "MidnightRunDesc",
			"Race in the dead of night. Your headlights are your lifeline. Limited visibility demands focus. "
			"Embrace the midnight grind for bonus rewards.");

	case EMGWeatherRaceType::FogRally:
		return NSLOCTEXT("WeatherRace", "FogRallyDesc",
			"Navigate through thick fog. Visibility severely limited. Trust your memory of the track. "
			"Significant bonus rewards for mastering the conditions.");

	case EMGWeatherRaceType::StormChase:
		return NSLOCTEXT("WeatherRace", "StormChaseDesc",
			"Race through a violent storm. Heavy rain, strong winds, and lightning create the ultimate challenge. "
			"Maximum bonus rewards for the brave.");

	case EMGWeatherRaceType::WindSprint:
		return NSLOCTEXT("WeatherRace", "WindSprintDesc",
			"High-speed challenge with strong crosswinds. Keep your car stable through powerful gusts. "
			"Bonus rewards for aerodynamic mastery.");

	default:
		return NSLOCTEXT("WeatherRace", "StandardDesc",
			"Race with current weather conditions.");
	}
}

int64 UMGWeatherRaceHandler::CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const
{
	// Get base credits from parent
	int64 BaseCredits = Super::CalculateCreditsForPosition(Position, TotalRacers);

	// Apply weather cash bonus
	const float CashMultiplier = GetCashBonusMultiplier();
	BaseCredits = static_cast<int64>(BaseCredits * CashMultiplier);

	// Additional bonus for clean run (no aquaplaning)
	if (WasCleanRun() && ActiveWeatherRaceType == EMGWeatherRaceType::RainRace)
	{
		BaseCredits = static_cast<int64>(BaseCredits * 1.1f); // Extra 10% for clean run
	}

	return BaseCredits;
}

int32 UMGWeatherRaceHandler::CalculateXPForPosition(int32 Position, int32 TotalRacers) const
{
	// Get base XP from parent
	int32 BaseXP = Super::CalculateXPForPosition(Position, TotalRacers);

	// Apply weather XP bonus
	const float XPMultiplier = GetXPBonusMultiplier();
	BaseXP = static_cast<int32>(BaseXP * XPMultiplier);

	return BaseXP;
}

int32 UMGWeatherRaceHandler::CalculateReputationEarned(int32 Position, bool bWon) const
{
	// Get base reputation from parent
	int32 BaseRep = Super::CalculateReputationEarned(Position, bWon);

	// Apply weather REP bonus
	const float REPMultiplier = GetREPBonusMultiplier();
	BaseRep = static_cast<int32>(BaseRep * REPMultiplier);

	// Additional REP for surviving max storm conditions
	if (ActiveWeatherRaceType == EMGWeatherRaceType::StormChase && MaxGustSurvived > 0.8f)
	{
		BaseRep = static_cast<int32>(BaseRep * 1.15f); // Extra 15% for storm survival
	}

	return BaseRep;
}

// ============================================================================
// WEATHER RACE CONFIGURATION
// ============================================================================

void UMGWeatherRaceHandler::SetWeatherRaceType(EMGWeatherRaceType RaceType)
{
	ActiveWeatherRaceType = RaceType;
	bUsingCustomConfig = false;

	// Apply immediately if we're already active
	if (bIsActive && WeatherRacingSubsystem.IsValid())
	{
		WeatherRacingSubsystem->SetWeatherRaceType(RaceType);
	}
}

void UMGWeatherRaceHandler::ApplyCustomConfig(const FMGWeatherRaceConfig& Config)
{
	CustomConfig = Config;
	ActiveWeatherRaceType = Config.RaceType;
	bUsingCustomConfig = true;

	// Apply immediately if we're already active
	if (bIsActive && WeatherRacingSubsystem.IsValid())
	{
		WeatherRacingSubsystem->ApplyWeatherRaceConfig(Config);
	}
}

FMGWeatherRacingEffects UMGWeatherRaceHandler::GetCurrentWeatherEffects() const
{
	if (WeatherRacingSubsystem.IsValid())
	{
		return WeatherRacingSubsystem->GetCurrentEffects();
	}

	return FMGWeatherRacingEffects();
}

// ============================================================================
// WEATHER BONUSES
// ============================================================================

float UMGWeatherRaceHandler::GetREPBonusMultiplier() const
{
	if (WeatherRacingSubsystem.IsValid())
	{
		return WeatherRacingSubsystem->GetCurrentEffects().REPMultiplier;
	}

	// Fallback based on race type
	switch (ActiveWeatherRaceType)
	{
	case EMGWeatherRaceType::RainRace:    return 1.2f;
	case EMGWeatherRaceType::MidnightRun: return 1.25f;
	case EMGWeatherRaceType::FogRally:    return 1.3f;
	case EMGWeatherRaceType::StormChase:  return 1.5f;
	case EMGWeatherRaceType::WindSprint:  return 1.15f;
	default:                              return 1.0f;
	}
}

float UMGWeatherRaceHandler::GetCashBonusMultiplier() const
{
	if (WeatherRacingSubsystem.IsValid())
	{
		return WeatherRacingSubsystem->GetCurrentEffects().CashMultiplier;
	}

	// Fallback based on race type
	switch (ActiveWeatherRaceType)
	{
	case EMGWeatherRaceType::RainRace:    return 1.15f;
	case EMGWeatherRaceType::MidnightRun: return 1.2f;
	case EMGWeatherRaceType::FogRally:    return 1.25f;
	case EMGWeatherRaceType::StormChase:  return 1.4f;
	case EMGWeatherRaceType::WindSprint:  return 1.1f;
	default:                              return 1.0f;
	}
}

float UMGWeatherRaceHandler::GetXPBonusMultiplier() const
{
	if (WeatherRacingSubsystem.IsValid())
	{
		return WeatherRacingSubsystem->GetCurrentEffects().XPMultiplier;
	}

	// Fallback based on race type
	switch (ActiveWeatherRaceType)
	{
	case EMGWeatherRaceType::RainRace:    return 1.1f;
	case EMGWeatherRaceType::MidnightRun: return 1.15f;
	case EMGWeatherRaceType::FogRally:    return 1.2f;
	case EMGWeatherRaceType::StormChase:  return 1.35f;
	case EMGWeatherRaceType::WindSprint:  return 1.1f;
	default:                              return 1.0f;
	}
}

FText UMGWeatherRaceHandler::GetBonusDescription() const
{
	if (WeatherRacingSubsystem.IsValid())
	{
		return WeatherRacingSubsystem->GetWeatherBonusDescription();
	}

	// Build description manually
	TArray<FString> Bonuses;

	const float REP = GetREPBonusMultiplier();
	const float Cash = GetCashBonusMultiplier();
	const float XP = GetXPBonusMultiplier();

	if (REP > 1.0f)
	{
		Bonuses.Add(FString::Printf(TEXT("REP +%d%%"), FMath::RoundToInt((REP - 1.0f) * 100.0f)));
	}
	if (Cash > 1.0f)
	{
		Bonuses.Add(FString::Printf(TEXT("Cash +%d%%"), FMath::RoundToInt((Cash - 1.0f) * 100.0f)));
	}
	if (XP > 1.0f)
	{
		Bonuses.Add(FString::Printf(TEXT("XP +%d%%"), FMath::RoundToInt((XP - 1.0f) * 100.0f)));
	}

	return FText::FromString(FString::Join(Bonuses, TEXT(" | ")));
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

void UMGWeatherRaceHandler::InitializeSubsystems()
{
	if (AMGRaceGameMode* GM = GetGameMode())
	{
		if (const UWorld* World = GM->GetWorld())
		{
			WeatherRacingSubsystem = World->GetSubsystem<UMGWeatherRacingSubsystem>();
			WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>();
		}
	}
}

void UMGWeatherRaceHandler::UpdateWeatherEffects(float MGDeltaTime)
{
	// Weather subsystem handles its own tick, but we can trigger updates here if needed
	if (WeatherRacingSubsystem.IsValid())
	{
		WeatherRacingSubsystem->Tick(DeltaTime);
	}
}

void UMGWeatherRaceHandler::TrackAquaplaning(float MGDeltaTime)
{
	if (!WeatherRacingSubsystem.IsValid())
	{
		return;
	}

	// Get player vehicle
	if (AMGRaceGameMode* GM = GetGameMode())
	{
		if (const UWorld* World = GM->GetWorld())
		{
			if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
			{
				// Check if player is aquaplaning (this would check the vehicle's state)
				const FMGWeatherRacingEffects& Effects = WeatherRacingSubsystem->GetCurrentEffects();
				const bool bIsAquaplaning = Effects.AquaplaningState.bIsAquaplaning;

				if (bIsAquaplaning)
				{
					TotalAquaplaningTime += DeltaTime;
				}

				bWasAquaplaning = bIsAquaplaning;
			}
		}
	}
}

void UMGWeatherRaceHandler::TrackWindGusts()
{
	if (!WeatherRacingSubsystem.IsValid())
	{
		return;
	}

	const FMGWindState& Wind = WeatherRacingSubsystem->GetWindState();

	if (Wind.bInGust && Wind.GustIntensity > MaxGustSurvived)
	{
		// Check if player is still on track (not crashed)
		// For now, just track the maximum gust encountered
		MaxGustSurvived = Wind.GustIntensity;
	}
}

void UMGWeatherRaceHandler::UpdateAIForWeather()
{
	if (!WeatherRacingSubsystem.IsValid())
	{
		return;
	}

	// Get all AI controllers and update them with weather modifiers
	if (AMGRaceGameMode* GM = GetGameMode())
	{
		if (UWorld* World = GM->GetWorld())
		{
			for (TActorIterator<AMGRacingAIController> It(World); It; ++It)
			{
				AMGRacingAIController* AIController = *It;
				if (AIController)
				{
					WeatherRacingSubsystem->UpdateAIForWeather(AIController);
				}
			}
		}
	}
}

void UMGWeatherRaceHandler::OnPuddleHit(AActor* Vehicle, const FMGPuddleInstance& Puddle)
{
	// Check if this is the player vehicle
	if (AMGRaceGameMode* GM = GetGameMode())
	{
		if (const UWorld* World = GM->GetWorld())
		{
			if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
			{
				if (Vehicle == PlayerPawn || Vehicle == PlayerPawn->GetOwner())
				{
					PuddlesHit++;

					UE_LOG(LogTemp, Verbose, TEXT("MGWeatherRaceHandler: Player hit puddle #%d"), PuddlesHit);
				}
			}
		}
	}
}

void UMGWeatherRaceHandler::OnWindGustOccurred(float Intensity, FVector Direction)
{
	// Track for potential achievements or statistics
	UE_LOG(LogTemp, Verbose, TEXT("MGWeatherRaceHandler: Wind gust occurred - Intensity: %.2f"), Intensity);
}

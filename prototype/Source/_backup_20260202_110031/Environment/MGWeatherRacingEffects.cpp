// Copyright Midnight Grind. All Rights Reserved.

#include "Environment/MGWeatherRacingEffects.h"
#include "Weather/MGWeatherSubsystem.h"
#include "AI/MGRacingAIController.h"
#include "Vehicle/MGVehicleMovementComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ============================================================================
// SUBSYSTEM LIFECYCLE
// ============================================================================

void UMGWeatherRacingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get reference to weather subsystem
	if (UWorld* World = GetWorld())
	{
		WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>();
	}

	// Setup default configurations for each race type
	SetupDefaultConfigurations();

	// Initialize to standard race type
	CurrentEffects.ActiveRaceType = EMGWeatherRaceType::Standard;

	UE_LOG(LogTemp, Log, TEXT("MGWeatherRacingSubsystem: Initialized"));
}

void UMGWeatherRacingSubsystem::Deinitialize()
{
	ActivePuddles.Empty();
	VehicleAquaplaningStates.Empty();

	Super::Deinitialize();
}

bool UMGWeatherRacingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Create in game worlds only
	if (const UWorld* World = Cast<UWorld>(Outer))
	{
		return World->IsGameWorld();
	}
	return false;
}

void UMGWeatherRacingSubsystem::Tick(float DeltaTime)
{
	// Update from base weather subsystem
	UpdateFromWeatherSubsystem();

	// Update puddle simulation
	UpdatePuddles(DeltaTime);

	// Update wind gusts
	UpdateWindGusts(DeltaTime);

	// Recalculate aggregate modifiers
	UpdateAggregateModifiers();
}

TStatId UMGWeatherRacingSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMGWeatherRacingSubsystem, STATGROUP_Tickables);
}

// ============================================================================
// WEATHER RACE TYPES
// ============================================================================

void UMGWeatherRacingSubsystem::SetWeatherRaceType(EMGWeatherRaceType RaceType)
{
	if (CurrentEffects.ActiveRaceType == RaceType)
	{
		return;
	}

	const EMGWeatherRaceType OldType = CurrentEffects.ActiveRaceType;
	CurrentEffects.ActiveRaceType = RaceType;

	// Apply default configuration for this race type
	if (DefaultConfigurations.Contains(RaceType))
	{
		ApplyConfiguration(DefaultConfigurations[RaceType]);
	}

	// Broadcast change
	OnWeatherRaceTypeChanged.Broadcast(OldType, RaceType);

	UE_LOG(LogTemp, Log, TEXT("MGWeatherRacingSubsystem: Race type changed from %d to %d"),
		static_cast<int32>(OldType), static_cast<int32>(RaceType));
}

void UMGWeatherRacingSubsystem::ApplyWeatherRaceConfig(const FMGWeatherRaceConfig& Config)
{
	const EMGWeatherRaceType OldType = CurrentEffects.ActiveRaceType;

	ApplyConfiguration(Config);

	if (OldType != Config.RaceType)
	{
		OnWeatherRaceTypeChanged.Broadcast(OldType, Config.RaceType);
	}
}

FMGWeatherRaceConfig UMGWeatherRacingSubsystem::GetDefaultConfigForType(EMGWeatherRaceType RaceType) const
{
	if (DefaultConfigurations.Contains(RaceType))
	{
		return DefaultConfigurations[RaceType];
	}

	// Return empty standard config
	FMGWeatherRaceConfig DefaultConfig;
	DefaultConfig.RaceType = EMGWeatherRaceType::Standard;
	DefaultConfig.DisplayName = NSLOCTEXT("WeatherRace", "StandardConfig", "Standard Race");
	return DefaultConfig;
}

// ============================================================================
// PUDDLES AND AQUAPLANING
// ============================================================================

FMGPuddleInstance UMGWeatherRacingSubsystem::SpawnPuddle(const FVector& Location, float Radius, float Depth)
{
	// Check puddle limit
	if (ActivePuddles.Num() >= MaxPuddles)
	{
		// Remove oldest puddle
		if (ActivePuddles.Num() > 0)
		{
			ActivePuddles.RemoveAt(0);
		}
	}

	FMGPuddleInstance NewPuddle;
	NewPuddle.Location = Location;
	NewPuddle.Radius = Radius;
	NewPuddle.Depth = Depth;
	NewPuddle.PuddleID = NextPuddleID++;
	NewPuddle.Age = 0.0f;

	ActivePuddles.Add(NewPuddle);

	return NewPuddle;
}

void UMGWeatherRacingSubsystem::ClearAllPuddles()
{
	ActivePuddles.Empty();
	CurrentEffects.bHasPuddles = false;
}

bool UMGWeatherRacingSubsystem::IsPointInPuddle(const FVector& Location, FMGPuddleInstance& OutPuddle) const
{
	for (const FMGPuddleInstance& Puddle : ActivePuddles)
	{
		const float DistSq = FVector::DistSquared2D(Location, Puddle.Location);
		if (DistSq <= FMath::Square(Puddle.Radius))
		{
			OutPuddle = Puddle;
			return true;
		}
	}
	return false;
}

FMGAquaplaningState UMGWeatherRacingSubsystem::CalculateAquaplaning(
	const FVector& VehicleLocation,
	float VehicleSpeedKPH,
	const TArray<FVector>& WheelLocations)
{
	FMGAquaplaningState State;

	// Check each wheel
	if (WheelLocations.Num() >= 4)
	{
		float MaxAquaplaning = 0.0f;
		int32 WheelsInPuddle = 0;

		for (int32 i = 0; i < 4; ++i)
		{
			FMGPuddleInstance Puddle;
			if (IsPointInPuddle(WheelLocations[i], Puddle))
			{
				const float WheelAquaplaning = Puddle.CalculateAquaplaningFactor(VehicleSpeedKPH);
				State.WheelAquaplaningFactors[i] = WheelAquaplaning;
				MaxAquaplaning = FMath::Max(MaxAquaplaning, WheelAquaplaning);
				WheelsInPuddle++;

				if (State.CurrentPuddleID < 0)
				{
					State.CurrentPuddleID = Puddle.PuddleID;
				}
			}
		}

		// Aquaplaning occurs when at least 2 wheels are in puddles at speed
		if (WheelsInPuddle >= 2 && MaxAquaplaning > 0.2f)
		{
			State.bIsAquaplaning = true;
			State.AquaplaningIntensity = MaxAquaplaning;
		}
	}

	return State;
}

// ============================================================================
// VISIBILITY AND PERCEPTION
// ============================================================================

float UMGWeatherRacingSubsystem::GetEffectiveVisibility(const FVector& Location) const
{
	float Visibility = CurrentEffects.EffectiveVisibilityDistance;

	// Apply local fog density variations (could be expanded with volume checks)
	if (CurrentEffects.bIsFoggy)
	{
		Visibility *= CurrentEffects.FogState.LocalDensityMultiplier;
	}

	return FMath::Max(30.0f, Visibility);
}

void UMGWeatherRacingSubsystem::UpdateAIForWeather(AMGRacingAIController* AIController)
{
	if (!AIController)
	{
		return;
	}

	// Get current AI profile and modify based on weather
	FMGAIDriverProfile Profile = AIController->GetDriverProfile();

	// Reduce consistency in bad weather (AI makes more mistakes)
	const float WeatherPenalty = 1.0f - CurrentEffects.AIPerceptionMultiplier;
	Profile.Consistency = FMath::Clamp(Profile.Consistency - WeatherPenalty * 0.3f, 0.3f, 1.0f);

	// Reduce risk tolerance in adverse conditions
	if (CurrentEffects.bIsWetSurface || CurrentEffects.bIsFoggy)
	{
		Profile.RiskTolerance = FMath::Clamp(Profile.RiskTolerance - 0.2f, 0.1f, 1.0f);
	}

	// Increase reaction time in low visibility
	if (CurrentEffects.EffectiveVisibilityDistance < 200.0f)
	{
		Profile.ReactionTime = FMath::Clamp(Profile.ReactionTime + 0.1f, 0.1f, 1.0f);
	}

	// Apply modified profile
	AIController->SetDriverProfile(Profile);
}

// ============================================================================
// WIND EFFECTS
// ============================================================================

FVector UMGWeatherRacingSubsystem::CalculateWindForce(
	const FVector& VehicleForward,
	float VehicleSpeedKPH,
	float FrontalArea,
	float DragCoefficient) const
{
	if (!CurrentEffects.bIsWindy || CurrentEffects.WindState.GetEffectiveWindSpeed() < MinEffectiveWindSpeed)
	{
		return FVector::ZeroVector;
	}

	// Calculate lateral force magnitude
	const float LateralForce = CurrentEffects.WindState.CalculateCrosswindForce(
		VehicleForward, VehicleSpeedKPH, FrontalArea, DragCoefficient);

	// Determine force direction (perpendicular to vehicle)
	const FVector WindDir = CurrentEffects.WindState.WindDirection.GetSafeNormal();
	const FVector VehicleRight = FVector::CrossProduct(FVector::UpVector, VehicleForward).GetSafeNormal();
	const float CrosswindSign = FMath::Sign(FVector::DotProduct(WindDir, VehicleRight));

	return VehicleRight * LateralForce * CrosswindSign;
}

void UMGWeatherRacingSubsystem::TriggerWindGust(float Intensity, float Duration)
{
	FMGWindState& Wind = CurrentEffects.WindState;

	Wind.bInGust = true;
	Wind.GustIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	Wind.GustDuration = Duration;

	// Broadcast gust event
	OnWindGust.Broadcast(Wind.GustIntensity, Wind.WindDirection);

	UE_LOG(LogTemp, Verbose, TEXT("MGWeatherRacingSubsystem: Wind gust triggered - Intensity: %.2f, Duration: %.1fs"),
		Intensity, Duration);
}

// ============================================================================
// REWARDS
// ============================================================================

void UMGWeatherRacingSubsystem::GetRewardMultipliers(
	float& OutREPMultiplier,
	float& OutCashMultiplier,
	float& OutXPMultiplier) const
{
	OutREPMultiplier = CurrentEffects.REPMultiplier;
	OutCashMultiplier = CurrentEffects.CashMultiplier;
	OutXPMultiplier = CurrentEffects.XPMultiplier;
}

FText UMGWeatherRacingSubsystem::GetWeatherBonusDescription() const
{
	TArray<FString> Bonuses;

	if (CurrentEffects.REPMultiplier > 1.0f)
	{
		Bonuses.Add(FString::Printf(TEXT("REP +%d%%"),
			FMath::RoundToInt((CurrentEffects.REPMultiplier - 1.0f) * 100.0f)));
	}

	if (CurrentEffects.CashMultiplier > 1.0f)
	{
		Bonuses.Add(FString::Printf(TEXT("Cash +%d%%"),
			FMath::RoundToInt((CurrentEffects.CashMultiplier - 1.0f) * 100.0f)));
	}

	if (CurrentEffects.XPMultiplier > 1.0f)
	{
		Bonuses.Add(FString::Printf(TEXT("XP +%d%%"),
			FMath::RoundToInt((CurrentEffects.XPMultiplier - 1.0f) * 100.0f)));
	}

	if (Bonuses.Num() == 0)
	{
		return FText::GetEmpty();
	}

	// Build combined string
	FString Combined = FString::Join(Bonuses, TEXT(" | "));
	return FText::FromString(Combined);
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

void UMGWeatherRacingSubsystem::UpdateFromWeatherSubsystem()
{
	if (!WeatherSubsystem.IsValid())
	{
		return;
	}

	const FMGWeatherState& Weather = WeatherSubsystem->GetCurrentWeather();
	const FMGTimeOfDayState& TimeOfDay = WeatherSubsystem->GetTimeOfDayState();

	// Update condition flags
	CurrentEffects.bIsRaining = WeatherSubsystem->IsRaining();
	CurrentEffects.bIsNight = WeatherSubsystem->IsNightTime();
	CurrentEffects.bIsWetSurface = Weather.RoadCondition == EMGRoadCondition::Wet ||
	                               Weather.RoadCondition == EMGRoadCondition::Flooded ||
	                               Weather.RoadCondition == EMGRoadCondition::Damp;

	// Update fog state
	CurrentEffects.bIsFoggy = Weather.FogDensity > 0.2f;
	CurrentEffects.FogState.FogDensity = Weather.FogDensity;
	CurrentEffects.FogState.VisibilityDistance = Weather.VisibilityDistance;

	// Update wind state
	CurrentEffects.bIsWindy = Weather.WindSpeed > MinEffectiveWindSpeed;
	CurrentEffects.WindState.WindSpeed = Weather.WindSpeed;
	CurrentEffects.WindState.WindDirection = Weather.WindDirection;

	// Update night state
	CurrentEffects.NightState.AmbientLight = TimeOfDay.AmbientLightLevel;
	CurrentEffects.NightState.MoonIllumination = TimeOfDay.MoonIntensity;
	CurrentEffects.NightState.StreetLightCoverage = TimeOfDay.bStreetLightsOn ? TimeOfDay.StreetLightIntensity : 0.0f;

	// Inherit base grip multiplier
	CurrentEffects.EffectiveGripMultiplier = Weather.GripMultiplier;

	// Update puddle state based on rain
	CurrentEffects.bHasPuddles = ActivePuddles.Num() > 0 || Weather.WaterPuddleCoverage > 0.1f;
}

void UMGWeatherRacingSubsystem::UpdatePuddles(float DeltaTime)
{
	// Spawn new puddles if raining
	if (CurrentEffects.bIsRaining && WeatherSubsystem.IsValid())
	{
		SpawnRainPuddles(DeltaTime);
	}

	// Update existing puddles (evaporation)
	for (int32 i = ActivePuddles.Num() - 1; i >= 0; --i)
	{
		FMGPuddleInstance& Puddle = ActivePuddles[i];
		Puddle.Age += DeltaTime;

		// Evaporation when not raining
		if (!CurrentEffects.bIsRaining)
		{
			Puddle.Depth -= PuddleEvaporationRate * DeltaTime;

			if (Puddle.Depth <= 0.0f)
			{
				ActivePuddles.RemoveAt(i);
			}
		}
	}

	CurrentEffects.bHasPuddles = ActivePuddles.Num() > 0;
}

void UMGWeatherRacingSubsystem::UpdateWindGusts(float DeltaTime)
{
	FMGWindState& Wind = CurrentEffects.WindState;

	// Update active gust
	if (Wind.bInGust)
	{
		Wind.GustDuration -= DeltaTime;
		if (Wind.GustDuration <= 0.0f)
		{
			Wind.bInGust = false;
			Wind.GustIntensity = 0.0f;
		}
	}
	else if (CurrentEffects.bIsWindy)
	{
		// Check for random gust
		GustTimer += DeltaTime;
		if (GustTimer >= Wind.NextGustTime)
		{
			// Trigger random gust
			const float Intensity = FMath::RandRange(0.3f, 1.0f);
			const float Duration = FMath::RandRange(GustDurationRange.X, GustDurationRange.Y);
			TriggerWindGust(Intensity, Duration);

			// Reset timer with randomized interval
			GustTimer = 0.0f;
			Wind.NextGustTime = FMath::RandRange(AverageGustInterval * 0.5f, AverageGustInterval * 1.5f);
		}
	}
}

void UMGWeatherRacingSubsystem::UpdateAggregateModifiers()
{
	// Calculate AI perception multiplier
	float AIPerception = 1.0f;

	if (CurrentEffects.bIsFoggy)
	{
		AIPerception *= CurrentEffects.FogState.CalculateAIPerceptionModifier();
	}

	if (CurrentEffects.bIsNight)
	{
		// Night reduces AI perception
		AIPerception *= 0.6f + (CurrentEffects.NightState.AmbientLight * 0.4f);
	}

	if (CurrentEffects.bIsRaining)
	{
		// Rain spray reduces visibility
		AIPerception *= 0.85f;
	}

	CurrentEffects.AIPerceptionMultiplier = FMath::Clamp(AIPerception, 0.1f, 1.0f);

	// Calculate effective visibility
	float Visibility = 10000.0f; // Default clear visibility

	if (CurrentEffects.bIsNight)
	{
		Visibility = CurrentEffects.NightState.CalculateEffectiveVisibility();
	}

	if (CurrentEffects.bIsFoggy)
	{
		Visibility = FMath::Min(Visibility, CurrentEffects.FogState.VisibilityDistance);
	}

	CurrentEffects.EffectiveVisibilityDistance = Visibility;

	// Calculate top speed modifier from wind
	if (CurrentEffects.bIsWindy)
	{
		// Headwind reduces top speed
		const float EffectiveWind = CurrentEffects.WindState.GetEffectiveWindSpeed();
		CurrentEffects.TopSpeedModifier = 1.0f - (EffectiveWind / 200.0f) * 0.1f;
		CurrentEffects.TopSpeedModifier = FMath::Clamp(CurrentEffects.TopSpeedModifier, 0.85f, 1.0f);
	}
	else
	{
		CurrentEffects.TopSpeedModifier = 1.0f;
	}
}

void UMGWeatherRacingSubsystem::SpawnRainPuddles(float DeltaTime)
{
	if (!WeatherSubsystem.IsValid())
	{
		return;
	}

	const float RainIntensity = WeatherSubsystem->GetRainIntensity();
	if (RainIntensity <= 0.0f)
	{
		return;
	}

	// Accumulate spawn time
	PuddleSpawnAccumulator += DeltaTime * RainIntensity * PuddleSpawnRate;

	while (PuddleSpawnAccumulator >= 1.0f && ActivePuddles.Num() < MaxPuddles)
	{
		PuddleSpawnAccumulator -= 1.0f;

		// Spawn puddle at random location around player
		// In production, this would use track spline data for realistic placement
		if (const UWorld* World = GetWorld())
		{
			if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
			{
				const FVector PlayerLoc = PlayerPawn->GetActorLocation();

				// Random offset within 200 meters
				const FVector Offset(
					FMath::RandRange(-20000.0f, 20000.0f),
					FMath::RandRange(-20000.0f, 20000.0f),
					0.0f
				);

				FVector SpawnLoc = PlayerLoc + Offset;

				// Trace down to find ground
				FHitResult Hit;
				const FVector TraceStart = SpawnLoc + FVector(0, 0, 500.0f);
				const FVector TraceEnd = SpawnLoc - FVector(0, 0, 1000.0f);

				if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
				{
					SpawnLoc = Hit.Location;

					// Random puddle size
					const float Radius = FMath::RandRange(100.0f, 400.0f);
					const float Depth = FMath::RandRange(1.0f, 5.0f) * RainIntensity;

					SpawnPuddle(SpawnLoc, Radius, Depth);
				}
			}
		}
	}
}

void UMGWeatherRacingSubsystem::SetupDefaultConfigurations()
{
	// Standard - No special conditions
	{
		FMGWeatherRaceConfig Config;
		Config.RaceType = EMGWeatherRaceType::Standard;
		Config.DisplayName = NSLOCTEXT("WeatherRace", "Standard", "Standard");
		Config.Description = NSLOCTEXT("WeatherRace", "StandardDesc", "Race with current weather conditions.");
		Config.REPMultiplier = 1.0f;
		Config.CashMultiplier = 1.0f;
		Config.XPMultiplier = 1.0f;
		DefaultConfigurations.Add(EMGWeatherRaceType::Standard, Config);
	}

	// Rain Race - Wet conditions with puddles
	{
		FMGWeatherRaceConfig Config;
		Config.RaceType = EMGWeatherRaceType::RainRace;
		Config.DisplayName = NSLOCTEXT("WeatherRace", "RainRace", "Rain Race");
		Config.Description = NSLOCTEXT("WeatherRace", "RainRaceDesc",
			"Race through the rain. Watch for puddles that cause aquaplaning at speed. Reduced grip on wet surfaces.");
		Config.RainIntensity = 0.7f;
		Config.PuddleDensity = 0.6f;
		Config.AquaplaningSeverity = 1.0f;
		Config.VisibilityDistance = 2000.0f;
		Config.REPMultiplier = 1.2f;  // 20% REP bonus
		Config.CashMultiplier = 1.15f; // 15% cash bonus
		Config.XPMultiplier = 1.1f;   // 10% XP bonus
		DefaultConfigurations.Add(EMGWeatherRaceType::RainRace, Config);
	}

	// Midnight Run - Deep night with limited visibility
	{
		FMGWeatherRaceConfig Config;
		Config.RaceType = EMGWeatherRaceType::MidnightRun;
		Config.DisplayName = NSLOCTEXT("WeatherRace", "MidnightRun", "Midnight Run");
		Config.Description = NSLOCTEXT("WeatherRace", "MidnightRunDesc",
			"Race in the dead of night. Your headlights are your lifeline. Limited visibility demands focus.");
		Config.bForceNight = true;
		Config.AmbientLightOverride = 0.05f;
		Config.HeadlightImportance = 2.0f;
		Config.VisibilityDistance = 150.0f;
		Config.AIFogPerceptionModifier = 0.6f;
		Config.REPMultiplier = 1.25f;  // 25% REP bonus
		Config.CashMultiplier = 1.2f;  // 20% cash bonus
		Config.XPMultiplier = 1.15f;   // 15% XP bonus
		DefaultConfigurations.Add(EMGWeatherRaceType::MidnightRun, Config);
	}

	// Fog Rally - Dense fog
	{
		FMGWeatherRaceConfig Config;
		Config.RaceType = EMGWeatherRaceType::FogRally;
		Config.DisplayName = NSLOCTEXT("WeatherRace", "FogRally", "Fog Rally");
		Config.Description = NSLOCTEXT("WeatherRace", "FogRallyDesc",
			"Navigate through thick fog. Visibility severely limited. Trust your memory of the track.");
		Config.FogDensity = 0.7f;
		Config.VisibilityDistance = 150.0f;
		Config.AIFogPerceptionModifier = 0.4f;
		Config.bForceNight = true;
		Config.AmbientLightOverride = 0.15f;
		Config.REPMultiplier = 1.3f;   // 30% REP bonus
		Config.CashMultiplier = 1.25f; // 25% cash bonus
		Config.XPMultiplier = 1.2f;    // 20% XP bonus
		DefaultConfigurations.Add(EMGWeatherRaceType::FogRally, Config);
	}

	// Storm Chase - Maximum challenge
	{
		FMGWeatherRaceConfig Config;
		Config.RaceType = EMGWeatherRaceType::StormChase;
		Config.DisplayName = NSLOCTEXT("WeatherRace", "StormChase", "Storm Chase");
		Config.Description = NSLOCTEXT("WeatherRace", "StormChaseDesc",
			"Race through a violent storm. Heavy rain, strong winds, and lightning. The ultimate weather challenge.");
		Config.RainIntensity = 0.95f;
		Config.PuddleDensity = 0.8f;
		Config.AquaplaningSeverity = 1.5f;
		Config.bForceNight = true;
		Config.AmbientLightOverride = 0.1f;
		Config.FogDensity = 0.3f;
		Config.VisibilityDistance = 500.0f;
		Config.WindSpeed = 60.0f;
		Config.bEnableGusts = true;
		Config.MaxGustIntensity = 0.8f;
		Config.AIFogPerceptionModifier = 0.3f;
		Config.REPMultiplier = 1.5f;   // 50% REP bonus
		Config.CashMultiplier = 1.4f;  // 40% cash bonus
		Config.XPMultiplier = 1.35f;   // 35% XP bonus
		DefaultConfigurations.Add(EMGWeatherRaceType::StormChase, Config);
	}

	// Wind Sprint - Clear but windy
	{
		FMGWeatherRaceConfig Config;
		Config.RaceType = EMGWeatherRaceType::WindSprint;
		Config.DisplayName = NSLOCTEXT("WeatherRace", "WindSprint", "Wind Sprint");
		Config.Description = NSLOCTEXT("WeatherRace", "WindSprintDesc",
			"High-speed challenge with strong crosswinds. Keep your car stable through powerful gusts.");
		Config.WindSpeed = 50.0f;
		Config.WindDirectionAngle = 90.0f; // Crosswind
		Config.bEnableGusts = true;
		Config.MaxGustIntensity = 1.0f;
		Config.VisibilityDistance = 8000.0f;
		Config.REPMultiplier = 1.15f;  // 15% REP bonus
		Config.CashMultiplier = 1.1f;  // 10% cash bonus
		Config.XPMultiplier = 1.1f;    // 10% XP bonus
		DefaultConfigurations.Add(EMGWeatherRaceType::WindSprint, Config);
	}
}

void UMGWeatherRacingSubsystem::ApplyConfiguration(const FMGWeatherRaceConfig& Config)
{
	CurrentEffects.ActiveRaceType = Config.RaceType;

	// Apply reward multipliers
	CurrentEffects.REPMultiplier = Config.REPMultiplier;
	CurrentEffects.CashMultiplier = Config.CashMultiplier;
	CurrentEffects.XPMultiplier = Config.XPMultiplier;

	// Apply to weather subsystem if available
	if (WeatherSubsystem.IsValid())
	{
		// Apply rain
		if (Config.RainIntensity > 0.0f)
		{
			const EMGWeatherIntensity Intensity = Config.RainIntensity > 0.7f ?
				EMGWeatherIntensity::Heavy : EMGWeatherIntensity::Moderate;
			WeatherSubsystem->SetWeather(EMGWeatherType::HeavyRain, Intensity);
		}

		// Apply fog
		if (Config.FogDensity > 0.0f)
		{
			const EMGWeatherIntensity Intensity = Config.FogDensity > 0.5f ?
				EMGWeatherIntensity::Heavy : EMGWeatherIntensity::Moderate;

			// Only apply fog if no rain (fog + rain handled by StormChase separately)
			if (Config.RainIntensity <= 0.0f)
			{
				WeatherSubsystem->SetWeather(EMGWeatherType::Fog, Intensity);
			}
		}

		// Apply night
		if (Config.bForceNight)
		{
			WeatherSubsystem->SetTime(0.0f, 0.0f); // Midnight
		}
	}

	// Apply direct wind settings
	CurrentEffects.WindState.WindSpeed = Config.WindSpeed;
	CurrentEffects.WindState.WindDirection = Config.GetWindDirectionVector();
	CurrentEffects.bIsWindy = Config.WindSpeed > MinEffectiveWindSpeed;

	// Apply fog settings
	CurrentEffects.FogState.FogDensity = Config.FogDensity;
	CurrentEffects.FogState.VisibilityDistance = Config.VisibilityDistance;
	CurrentEffects.FogState.AIPerceptionMultiplier = Config.AIFogPerceptionModifier;
	CurrentEffects.bIsFoggy = Config.FogDensity > 0.2f;

	// Apply night settings
	if (Config.bForceNight)
	{
		CurrentEffects.bIsNight = true;
		CurrentEffects.NightState.AmbientLight = Config.AmbientLightOverride;
		CurrentEffects.NightState.HeadlightEffectiveness = 1.0f / Config.HeadlightImportance;
	}

	// Apply rain settings
	CurrentEffects.bIsRaining = Config.RainIntensity > 0.0f;
	if (Config.RainIntensity > 0.3f)
	{
		CurrentEffects.bIsWetSurface = true;
	}

	// Clear or populate puddles based on config
	if (Config.PuddleDensity <= 0.0f)
	{
		ClearAllPuddles();
	}

	// Enable gusts if configured
	if (Config.bEnableGusts)
	{
		GustTimer = 0.0f;
		CurrentEffects.WindState.NextGustTime = FMath::RandRange(3.0f, AverageGustInterval);
	}

	// Update visibility
	CurrentEffects.EffectiveVisibilityDistance = Config.VisibilityDistance;

	UE_LOG(LogTemp, Log, TEXT("MGWeatherRacingSubsystem: Applied configuration for race type %d"),
		static_cast<int32>(Config.RaceType));
}

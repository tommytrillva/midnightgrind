// Copyright Midnight Grind. All Rights Reserved.

#include "VFX/MGEnvironmentVFXManager.h"
#include "VFX/MGVFXSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AMGEnvironmentVFXManager::AMGEnvironmentVFXManager()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcessComponent->SetupAttachment(RootComponent);
	PostProcessComponent->bUnbound = true;
}

void AMGEnvironmentVFXManager::BeginPlay()
{
	Super::BeginPlay();

	// Initialize to default weather
	SetWeather(EMGWeatherType::Clear);

	// Setup ambient particles for initial zone
	UpdateAmbientParticles();

	// Initialize lighting
	UpdateLighting();
}

void AMGEnvironmentVFXManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateWeather(DeltaTime);
	UpdateTimeOfDay(DeltaTime);
	UpdatePostProcess();
}

// ==========================================
// WEATHER CONTROL
// ==========================================

void AMGEnvironmentVFXManager::SetWeather(EMGWeatherType Weather)
{
	FMGWeatherConfig NewConfig = GetDefaultWeatherConfig(Weather);
	CurrentWeatherConfig = NewConfig;
	TargetWeatherConfig = NewConfig;
	WeatherTransitionAlpha = 1.0f;

	ApplyWeatherConfig(CurrentWeatherConfig);

	OnWeatherChanged.Broadcast(Weather);
}

void AMGEnvironmentVFXManager::TransitionToWeather(EMGWeatherType Weather, float TransitionTime)
{
	TargetWeatherConfig = GetDefaultWeatherConfig(Weather);
	WeatherTransitionDuration = TransitionTime;
	WeatherTransitionAlpha = 0.0f;
}

void AMGEnvironmentVFXManager::SetWeatherConfig(const FMGWeatherConfig& Config)
{
	CurrentWeatherConfig = Config;
	TargetWeatherConfig = Config;
	WeatherTransitionAlpha = 1.0f;
	ApplyWeatherConfig(Config);
}

void AMGEnvironmentVFXManager::TriggerLightning()
{
	if (LightningSystem)
	{
		// Spawn lightning at random position in sky
		FVector LightningPos = GetActorLocation() + FVector(
			FMath::RandRange(-5000.0f, 5000.0f),
			FMath::RandRange(-5000.0f, 5000.0f),
			FMath::RandRange(2000.0f, 5000.0f)
		);

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LightningSystem,
			LightningPos
		);
	}

	// Flash screen
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		VFXSub->FlashScreen(FLinearColor(0.9f, 0.95f, 1.0f, 1.0f), 0.15f, 0.8f);
		VFXSub->TriggerScreenShake(0.3f, 0.5f, true);
	}

	OnLightningStrike.Broadcast();
}

// ==========================================
// TIME OF DAY
// ==========================================

void AMGEnvironmentVFXManager::SetTimeOfDay(float Hour)
{
	CurrentHour = FMath::Fmod(Hour, 24.0f);
	if (CurrentHour < 0.0f) CurrentHour += 24.0f;

	UpdateLighting();

	// Check for period change
	EMGTimeOfDay NewPeriod = GetTimePeriod();
	if (NewPeriod != PreviousTimePeriod)
	{
		PreviousTimePeriod = NewPeriod;
		OnTimeOfDayChanged.Broadcast(NewPeriod);

		// Update ambient particles for new time period
		UpdateAmbientParticles();
	}

	// Update VFX subsystem
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		FMGGlobalVFXParams Params = VFXSub->GetGlobalParams();
		Params.TimeOfDay = CurrentHour / 24.0f;
		VFXSub->SetGlobalParams(Params);
	}
}

EMGTimeOfDay AMGEnvironmentVFXManager::GetTimePeriod() const
{
	if (CurrentHour >= 5.0f && CurrentHour < 7.0f) return EMGTimeOfDay::Dawn;
	if (CurrentHour >= 7.0f && CurrentHour < 10.0f) return EMGTimeOfDay::Morning;
	if (CurrentHour >= 10.0f && CurrentHour < 14.0f) return EMGTimeOfDay::Noon;
	if (CurrentHour >= 14.0f && CurrentHour < 17.0f) return EMGTimeOfDay::Afternoon;
	if (CurrentHour >= 17.0f && CurrentHour < 19.0f) return EMGTimeOfDay::Sunset;
	if (CurrentHour >= 19.0f && CurrentHour < 21.0f) return EMGTimeOfDay::Dusk;
	if (CurrentHour >= 21.0f || CurrentHour < 1.0f) return EMGTimeOfDay::Night;
	return EMGTimeOfDay::Midnight; // 1-5
}

void AMGEnvironmentVFXManager::SetTimeSpeed(float Speed)
{
	TimeSpeed = FMath::Max(Speed, 0.0f);
}

void AMGEnvironmentVFXManager::SetTimePaused(bool bPaused)
{
	bTimePaused = bPaused;
}

// ==========================================
// ENVIRONMENT ZONES
// ==========================================

void AMGEnvironmentVFXManager::EnterZone(EMGEnvironmentZone Zone)
{
	if (CurrentZone == Zone)
	{
		return;
	}

	CurrentZone = Zone;
	UpdateAmbientParticles();

	OnZoneChanged.Broadcast(Zone);

	UE_LOG(LogTemp, Log, TEXT("MGEnvironmentVFX: Entered zone %d"), static_cast<int32>(Zone));
}

void AMGEnvironmentVFXManager::RegisterZoneConfig(const FMGZoneParticleConfig& Config)
{
	// Replace if exists, add if new
	for (int32 i = 0; i < ZoneConfigs.Num(); ++i)
	{
		if (ZoneConfigs[i].Zone == Config.Zone)
		{
			ZoneConfigs[i] = Config;
			return;
		}
	}
	ZoneConfigs.Add(Config);
}

// ==========================================
// AMBIENT EFFECTS
// ==========================================

void AMGEnvironmentVFXManager::SpawnSteamVent(FVector Location, float Intensity)
{
	if (!SteamVentSystem)
	{
		return;
	}

	UNiagaraComponent* SteamComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		SteamVentSystem,
		Location,
		FRotator(-90.0f, 0.0f, 0.0f) // Point up
	);

	if (SteamComp)
	{
		SteamComp->SetNiagaraVariableFloat(FString("SteamIntensity"), Intensity);
	}
}

void AMGEnvironmentVFXManager::SpawnStreetDebris(FVector Location, FVector WindDirection)
{
	if (!StreetTrashSystem)
	{
		return;
	}

	UNiagaraComponent* DebrisComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		StreetTrashSystem,
		Location,
		WindDirection.Rotation()
	);

	if (DebrisComp)
	{
		DebrisComp->SetNiagaraVariableVec3(FString("WindDirection"), WindDirection.GetSafeNormal());
		DebrisComp->SetNiagaraVariableFloat(FString("WindStrength"), WindDirection.Size() / 100.0f);
	}
}

void AMGEnvironmentVFXManager::SpawnPuddleReflection(FVector Location, FLinearColor NeonColor)
{
	// This would spawn a decal or particle effect for puddle reflections
	// Implementation depends on material setup
}

void AMGEnvironmentVFXManager::SetAmbientParticlesEnabled(bool bEnabled)
{
	bAmbientParticlesEnabled = bEnabled;

	if (AmbientParticlesComp)
	{
		if (bEnabled && !AmbientParticlesComp->IsActive())
		{
			AmbientParticlesComp->Activate();
		}
		else if (!bEnabled && AmbientParticlesComp->IsActive())
		{
			AmbientParticlesComp->Deactivate();
		}
	}

	if (SecondaryAmbientComp)
	{
		if (bEnabled && !SecondaryAmbientComp->IsActive())
		{
			SecondaryAmbientComp->Activate();
		}
		else if (!bEnabled && SecondaryAmbientComp->IsActive())
		{
			SecondaryAmbientComp->Deactivate();
		}
	}
}

// ==========================================
// CITY LIGHTS
// ==========================================

void AMGEnvironmentVFXManager::SetCityLightIntensity(float Intensity)
{
	CityLightIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
	UpdateLighting();
}

void AMGEnvironmentVFXManager::SetNeonGlowIntensity(float Intensity)
{
	NeonGlowIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);

	if (NeonGlowComp)
	{
		NeonGlowComp->SetNiagaraVariableFloat(FString("GlowIntensity"), NeonGlowIntensity);
	}
}

void AMGEnvironmentVFXManager::TriggerNeonFlicker(FVector Location, float Duration)
{
	// Spawn localized flicker effect
	// Would typically spawn a particle or trigger a light flickering
}

// ==========================================
// INTERNAL
// ==========================================

UMGVFXSubsystem* AMGEnvironmentVFXManager::GetVFXSubsystem() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		return World->GetSubsystem<UMGVFXSubsystem>();
	}
	return nullptr;
}

void AMGEnvironmentVFXManager::UpdateWeather(float DeltaTime)
{
	// Handle weather transition
	if (WeatherTransitionAlpha < 1.0f && WeatherTransitionDuration > 0.0f)
	{
		WeatherTransitionAlpha += DeltaTime / WeatherTransitionDuration;
		WeatherTransitionAlpha = FMath::Min(WeatherTransitionAlpha, 1.0f);

		FMGWeatherConfig InterpolatedConfig = LerpWeatherConfig(
			CurrentWeatherConfig,
			TargetWeatherConfig,
			WeatherTransitionAlpha
		);

		ApplyWeatherConfig(InterpolatedConfig);

		if (WeatherTransitionAlpha >= 1.0f)
		{
			CurrentWeatherConfig = TargetWeatherConfig;
			OnWeatherChanged.Broadcast(CurrentWeatherConfig.WeatherType);
		}
	}

	// Handle lightning during storms
	if (CurrentWeatherConfig.LightningFrequency > 0.0f)
	{
		LightningTimer += DeltaTime;

		float LightningInterval = 60.0f / CurrentWeatherConfig.LightningFrequency;
		float RandomVariation = FMath::RandRange(0.5f, 1.5f);

		if (LightningTimer >= LightningInterval * RandomVariation)
		{
			TriggerLightning();
			LightningTimer = 0.0f;
		}
	}

	// Update VFX subsystem with weather intensity
	if (UMGVFXSubsystem* VFXSub = GetVFXSubsystem())
	{
		FMGGlobalVFXParams Params = VFXSub->GetGlobalParams();
		Params.WeatherIntensity = CurrentWeatherConfig.RainIntensity;
		VFXSub->SetGlobalParams(Params);
	}
}

void AMGEnvironmentVFXManager::UpdateTimeOfDay(float DeltaTime)
{
	if (bTimePaused)
	{
		return;
	}

	// Progress time
	// TimeSpeed of 60 = 1 game hour per real minute
	float HoursPerSecond = TimeSpeed / 3600.0f;
	float NewHour = CurrentHour + HoursPerSecond * DeltaTime;

	SetTimeOfDay(NewHour);
}

void AMGEnvironmentVFXManager::UpdateAmbientParticles()
{
	if (!bAmbientParticlesEnabled)
	{
		return;
	}

	// Find config for current zone
	FMGZoneParticleConfig* ZoneConfig = nullptr;
	for (FMGZoneParticleConfig& Config : ZoneConfigs)
	{
		if (Config.Zone == CurrentZone)
		{
			ZoneConfig = &Config;
			break;
		}
	}

	// Cleanup existing
	if (AmbientParticlesComp)
	{
		AmbientParticlesComp->DestroyComponent();
		AmbientParticlesComp = nullptr;
	}
	if (SecondaryAmbientComp)
	{
		SecondaryAmbientComp->DestroyComponent();
		SecondaryAmbientComp = nullptr;
	}

	// Default particles based on zone if no config
	UNiagaraSystem* PrimarySystem = nullptr;
	UNiagaraSystem* SecondarySystem = nullptr;
	float Density = AmbientParticleDensity;

	if (ZoneConfig)
	{
		PrimarySystem = ZoneConfig->AmbientParticles;
		SecondarySystem = ZoneConfig->SecondaryParticles;
		Density *= ZoneConfig->DensityMultiplier;
	}
	else
	{
		// Default based on zone type
		switch (CurrentZone)
		{
		case EMGEnvironmentZone::Downtown:
			PrimarySystem = CityDustSystem;
			// Night gets neon glow particles
			if (GetTimePeriod() == EMGTimeOfDay::Night || GetTimePeriod() == EMGTimeOfDay::Midnight)
			{
				SecondarySystem = NeonGlowSystem;
			}
			break;

		case EMGEnvironmentZone::Industrial:
			PrimarySystem = CityDustSystem;
			SecondarySystem = IndustrialSparksSystem;
			break;

		case EMGEnvironmentZone::Waterfront:
			PrimarySystem = FogSystem;
			SecondarySystem = BirdsSystem;
			break;

		case EMGEnvironmentZone::Residential:
			PrimarySystem = FallingLeavesSystem;
			// Fireflies at night
			if (GetTimePeriod() == EMGTimeOfDay::Night || GetTimePeriod() == EMGTimeOfDay::Dusk)
			{
				SecondarySystem = FirefliesSystem;
			}
			break;

		case EMGEnvironmentZone::Highway:
		case EMGEnvironmentZone::Tunnel:
		case EMGEnvironmentZone::Underground:
			PrimarySystem = CityDustSystem;
			Density *= 0.5f; // Less particles in these areas
			break;
		}
	}

	// Spawn primary
	if (PrimarySystem)
	{
		AmbientParticlesComp = SpawnManagedNiagara(PrimarySystem);
		if (AmbientParticlesComp)
		{
			AmbientParticlesComp->SetNiagaraVariableFloat(FString("ParticleDensity"), Density);
		}
	}

	// Spawn secondary
	if (SecondarySystem)
	{
		SecondaryAmbientComp = SpawnManagedNiagara(SecondarySystem);
		if (SecondaryAmbientComp)
		{
			SecondaryAmbientComp->SetNiagaraVariableFloat(FString("ParticleDensity"), Density * 0.5f);
		}
	}
}

FMGWeatherConfig AMGEnvironmentVFXManager::GetDefaultWeatherConfig(EMGWeatherType Weather) const
{
	FMGWeatherConfig Config;
	Config.WeatherType = Weather;

	switch (Weather)
	{
	case EMGWeatherType::Clear:
		Config.RainIntensity = 0.0f;
		Config.FogDensity = 0.0f;
		Config.WindStrength = 0.1f;
		Config.WetSurfaces = 0.0f;
		break;

	case EMGWeatherType::Overcast:
		Config.RainIntensity = 0.0f;
		Config.FogDensity = 0.1f;
		Config.WindStrength = 0.2f;
		Config.WetSurfaces = 0.0f;
		break;

	case EMGWeatherType::LightRain:
		Config.RainIntensity = 0.3f;
		Config.FogDensity = 0.15f;
		Config.WindStrength = 0.3f;
		Config.WetSurfaces = 0.5f;
		break;

	case EMGWeatherType::HeavyRain:
		Config.RainIntensity = 0.8f;
		Config.FogDensity = 0.25f;
		Config.WindStrength = 0.5f;
		Config.WetSurfaces = 1.0f;
		break;

	case EMGWeatherType::Storm:
		Config.RainIntensity = 1.0f;
		Config.FogDensity = 0.3f;
		Config.WindStrength = 0.8f;
		Config.LightningFrequency = 5.0f; // 5 strikes per minute
		Config.WetSurfaces = 1.0f;
		break;

	case EMGWeatherType::Fog:
		Config.RainIntensity = 0.0f;
		Config.FogDensity = 0.6f;
		Config.WindStrength = 0.05f;
		Config.WetSurfaces = 0.3f; // Morning dew
		break;

	case EMGWeatherType::Heat:
		Config.RainIntensity = 0.0f;
		Config.FogDensity = 0.05f;
		Config.WindStrength = 0.1f;
		Config.WetSurfaces = 0.0f;
		break;
	}

	return Config;
}

FMGWeatherConfig AMGEnvironmentVFXManager::LerpWeatherConfig(const FMGWeatherConfig& A, const FMGWeatherConfig& B, float Alpha) const
{
	FMGWeatherConfig Result;

	// Use target weather type once past halfway
	Result.WeatherType = Alpha < 0.5f ? A.WeatherType : B.WeatherType;

	Result.RainIntensity = FMath::Lerp(A.RainIntensity, B.RainIntensity, Alpha);
	Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
	Result.WindStrength = FMath::Lerp(A.WindStrength, B.WindStrength, Alpha);
	Result.WindDirection = FMath::Lerp(A.WindDirection, B.WindDirection, Alpha).GetSafeNormal();
	Result.LightningFrequency = FMath::Lerp(A.LightningFrequency, B.LightningFrequency, Alpha);
	Result.WetSurfaces = FMath::Lerp(A.WetSurfaces, B.WetSurfaces, Alpha);

	return Result;
}

void AMGEnvironmentVFXManager::ApplyWeatherConfig(const FMGWeatherConfig& Config)
{
	// Rain
	if (Config.RainIntensity > 0.0f)
	{
		UNiagaraSystem* RainToUse = Config.RainIntensity > 0.5f ? HeavyRainSystem : RainSystem;

		if (RainToUse)
		{
			if (!RainComp)
			{
				RainComp = SpawnManagedNiagara(RainToUse);
			}
			else if (RainComp->GetAsset() != RainToUse)
			{
				RainComp->SetAsset(RainToUse);
			}

			if (RainComp)
			{
				RainComp->SetNiagaraVariableFloat(FString("RainIntensity"), Config.RainIntensity);
				RainComp->SetNiagaraVariableVec3(FString("WindDirection"), Config.WindDirection * Config.WindStrength);

				if (!RainComp->IsActive())
				{
					RainComp->Activate();
				}
			}
		}

		// Rain ripples
		if (RainRipplesSystem && Config.WetSurfaces > 0.3f)
		{
			if (!RainRipplesComp)
			{
				RainRipplesComp = SpawnManagedNiagara(RainRipplesSystem);
			}

			if (RainRipplesComp)
			{
				RainRipplesComp->SetNiagaraVariableFloat(FString("RippleIntensity"), Config.RainIntensity);

				if (!RainRipplesComp->IsActive())
				{
					RainRipplesComp->Activate();
				}
			}
		}
	}
	else
	{
		if (RainComp && RainComp->IsActive())
		{
			RainComp->Deactivate();
		}
		if (RainRipplesComp && RainRipplesComp->IsActive())
		{
			RainRipplesComp->Deactivate();
		}
	}

	// Fog
	if (Config.FogDensity > 0.1f && FogSystem)
	{
		if (!FogComp)
		{
			FogComp = SpawnManagedNiagara(FogSystem);
		}

		if (FogComp)
		{
			FogComp->SetNiagaraVariableFloat(FString("FogDensity"), Config.FogDensity);

			if (!FogComp->IsActive())
			{
				FogComp->Activate();
			}
		}
	}
	else if (FogComp && FogComp->IsActive())
	{
		FogComp->Deactivate();
	}

	// Storm debris
	if (Config.WindStrength > 0.5f && StormDebrisSystem)
	{
		if (!StormDebrisComp)
		{
			StormDebrisComp = SpawnManagedNiagara(StormDebrisSystem);
		}

		if (StormDebrisComp)
		{
			StormDebrisComp->SetNiagaraVariableVec3(FString("WindDirection"), Config.WindDirection * Config.WindStrength * 100.0f);

			if (!StormDebrisComp->IsActive())
			{
				StormDebrisComp->Activate();
			}
		}
	}
	else if (StormDebrisComp && StormDebrisComp->IsActive())
	{
		StormDebrisComp->Deactivate();
	}

	// Heat shimmer
	if (Config.WeatherType == EMGWeatherType::Heat && HeatShimmerSystem)
	{
		if (!HeatShimmerComp)
		{
			HeatShimmerComp = SpawnManagedNiagara(HeatShimmerSystem);
		}

		if (HeatShimmerComp && !HeatShimmerComp->IsActive())
		{
			HeatShimmerComp->Activate();
		}
	}
	else if (HeatShimmerComp && HeatShimmerComp->IsActive())
	{
		HeatShimmerComp->Deactivate();
	}
}

void AMGEnvironmentVFXManager::UpdateLighting()
{
	// Calculate sun/moon position and intensity based on time
	// This would typically update a directional light

	EMGTimeOfDay Period = GetTimePeriod();
	bool bIsNight = (Period == EMGTimeOfDay::Night || Period == EMGTimeOfDay::Midnight || Period == EMGTimeOfDay::Dusk);

	// Adjust neon intensity based on time
	float TargetNeonIntensity = bIsNight ? 1.5f : 0.3f;
	NeonGlowIntensity = FMath::FInterpTo(NeonGlowIntensity, TargetNeonIntensity, GetWorld()->GetDeltaSeconds(), 1.0f);

	if (NeonGlowComp)
	{
		NeonGlowComp->SetNiagaraVariableFloat(FString("GlowIntensity"), NeonGlowIntensity * CityLightIntensity);
	}
}

void AMGEnvironmentVFXManager::UpdatePostProcess()
{
	if (!PostProcessComponent)
	{
		return;
	}

	// Adjust post-process based on time and weather
	EMGTimeOfDay Period = GetTimePeriod();

	// Night gets more contrast, cooler colors
	// Day gets warmer colors
	// Rain gets desaturation

	// This would set post-process settings dynamically
}

UNiagaraComponent* AMGEnvironmentVFXManager::SpawnManagedNiagara(UNiagaraSystem* System)
{
	if (!System)
	{
		return nullptr;
	}

	UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		System,
		RootComponent,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		false // Don't auto-destroy
	);

	return Comp;
}

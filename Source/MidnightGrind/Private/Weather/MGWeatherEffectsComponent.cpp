// Copyright Midnight Grind. All Rights Reserved.

#include "Weather/MGWeatherEffectsComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// ==========================================
// UMGWeatherEffectsComponent
// ==========================================

UMGWeatherEffectsComponent::UMGWeatherEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UMGWeatherEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get weather subsystem
	UWorld* World = GetWorld();
	if (World)
	{
		WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>();

		if (WeatherSubsystem)
		{
			WeatherSubsystem->OnWeatherChanged.AddDynamic(this, &UMGWeatherEffectsComponent::OnWeatherChanged);
			WeatherSubsystem->OnLightningStrike.AddDynamic(this, &UMGWeatherEffectsComponent::OnLightningStrike);

			// Initialize with current weather
			LastWeatherType = WeatherSubsystem->GetCurrentWeatherType();
		}
	}

	// Initialize components
	InitializeParticleComponents();
	InitializeAudioComponents();

	// Create windshield material instance
	if (WindshieldMaterial)
	{
		WindshieldMID = UMaterialInstanceDynamic::Create(WindshieldMaterial, this);
	}

	// Initial update
	ForceUpdateEffects();
}

void UMGWeatherEffectsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (WeatherSubsystem)
	{
		WeatherSubsystem->OnWeatherChanged.RemoveDynamic(this, &UMGWeatherEffectsComponent::OnWeatherChanged);
		WeatherSubsystem->OnLightningStrike.RemoveDynamic(this, &UMGWeatherEffectsComponent::OnLightningStrike);
	}

	Super::EndPlay(EndPlayReason);
}

void UMGWeatherEffectsComponent::TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEffectsEnabled || !WeatherSubsystem)
	{
		return;
	}

	// Update windshield effects
	UpdateWindshieldEffects(DeltaTime);

	// Update wipers
	UpdateWipers(DeltaTime);

	// Update screen effects
	UpdateScreenEffects();
}

// ==========================================
// CONTROL
// ==========================================

void UMGWeatherEffectsComponent::SetWeatherEffectsEnabled(bool bEnabled)
{
	bEffectsEnabled = bEnabled;

	if (!bEnabled)
	{
		// Disable all particle components
		if (RainParticleComponent)
		{
			RainParticleComponent->Deactivate();
		}
		if (SnowParticleComponent)
		{
			SnowParticleComponent->Deactivate();
		}
		if (DustParticleComponent)
		{
			DustParticleComponent->Deactivate();
		}
		if (AmbientAudioComponent)
		{
			AmbientAudioComponent->Stop();
		}
	}
	else
	{
		ForceUpdateEffects();
	}
}

void UMGWeatherEffectsComponent::ForceUpdateEffects()
{
	if (!WeatherSubsystem)
	{
		return;
	}

	FMGWeatherState Weather = WeatherSubsystem->GetCurrentWeather();

	// Update based on weather type
	switch (Weather.Type)
	{
	case EMGWeatherType::LightRain:
	case EMGWeatherType::HeavyRain:
	case EMGWeatherType::Thunderstorm:
	case EMGWeatherType::NightRain:
		UpdateRainEffects(Weather.Intensity.Precipitation);
		UpdateSnowEffects(0.0f);
		UpdateDustEffects(0.0f);
		break;

	case EMGWeatherType::Snow:
	case EMGWeatherType::Blizzard:
		UpdateRainEffects(0.0f);
		UpdateSnowEffects(Weather.Intensity.Precipitation);
		UpdateDustEffects(0.0f);
		break;

	case EMGWeatherType::DustStorm:
		UpdateRainEffects(0.0f);
		UpdateSnowEffects(0.0f);
		UpdateDustEffects(Weather.Intensity.Wind);
		break;

	default:
		UpdateRainEffects(0.0f);
		UpdateSnowEffects(0.0f);
		UpdateDustEffects(0.0f);
		break;
	}

	UpdateFogEffects(Weather.Intensity.FogDensity);
	UpdateAudio();
}

void UMGWeatherEffectsComponent::SetInteriorMode(bool bIsInterior)
{
	bInteriorMode = bIsInterior;

	// Adjust audio attenuation for interior
	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->SetVolumeMultiplier(bIsInterior ? 0.5f : 1.0f);
	}
}

// ==========================================
// WINDSHIELD
// ==========================================

void UMGWeatherEffectsComponent::ActivateWipers()
{
	WiperTimer = 0.0f;

	// Single wipe animation
	// In a real implementation, this would trigger a wiper animation
}

void UMGWeatherEffectsComponent::SetWiperMode(int32 Mode)
{
	WiperMode = FMath::Clamp(Mode, 0, 3);

	switch (WiperMode)
	{
	case 0: // Off
		WiperInterval = 0.0f;
		break;
	case 1: // Intermittent
		WiperInterval = 3.0f;
		break;
	case 2: // Normal
		WiperInterval = 1.0f;
		break;
	case 3: // Fast
		WiperInterval = 0.5f;
		break;
	}
}

// ==========================================
// SCREEN EFFECTS
// ==========================================

void UMGWeatherEffectsComponent::SetPostProcessComponent(UPostProcessComponent* PostProcess)
{
	PostProcessComponent = PostProcess;
	ApplyPostProcessSettings();
}

// ==========================================
// INTERNAL
// ==========================================

void UMGWeatherEffectsComponent::InitializeParticleComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Create rain particle component
	if (RainParticleSystem)
	{
		RainParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			RainParticleSystem,
			Owner->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false
		);
		if (RainParticleComponent)
		{
			RainParticleComponent->Deactivate();
		}
	}

	// Create snow particle component
	if (SnowParticleSystem)
	{
		SnowParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			SnowParticleSystem,
			Owner->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false
		);
		if (SnowParticleComponent)
		{
			SnowParticleComponent->Deactivate();
		}
	}

	// Create dust particle component
	if (DustParticleSystem)
	{
		DustParticleComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			DustParticleSystem,
			Owner->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false
		);
		if (DustParticleComponent)
		{
			DustParticleComponent->Deactivate();
		}
	}
}

void UMGWeatherEffectsComponent::InitializeAudioComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Create ambient audio component
	AmbientAudioComponent = NewObject<UAudioComponent>(Owner);
	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->SetupAttachment(Owner->GetRootComponent());
		AmbientAudioComponent->bAutoActivate = false;
		AmbientAudioComponent->bIsUISound = false;
		AmbientAudioComponent->RegisterComponent();
	}
}

void UMGWeatherEffectsComponent::UpdateRainEffects(float Intensity)
{
	if (!RainParticleComponent)
	{
		return;
	}

	if (Intensity > 0.0f)
	{
		RainParticleComponent->Activate();

		// Set spawn rate based on intensity
		RainParticleComponent->SetVariableFloat(TEXT("SpawnRateMultiplier"), Intensity);

		// Adjust droplet size based on intensity
		float DropletSize = FMath::Lerp(0.5f, 2.0f, Intensity);
		RainParticleComponent->SetVariableFloat(TEXT("DropletSize"), DropletSize);
	}
	else
	{
		RainParticleComponent->Deactivate();
	}

	// Update windshield rain data
	WindshieldEffects.DropletCount = Intensity * 100.0f;
	WindshieldEffects.DropletSize = FMath::Lerp(1.0f, 1.5f, Intensity);
	WindshieldEffects.StreakAmount = Intensity * 0.5f;
	WindshieldEffects.WetnessAmount = Intensity;
}

void UMGWeatherEffectsComponent::UpdateSnowEffects(float Intensity)
{
	if (!SnowParticleComponent)
	{
		return;
	}

	if (Intensity > 0.0f)
	{
		SnowParticleComponent->Activate();
		SnowParticleComponent->SetVariableFloat(TEXT("SpawnRateMultiplier"), Intensity);

		// Adjust flake size based on intensity
		float FlakeSize = FMath::Lerp(0.8f, 1.5f, Intensity);
		SnowParticleComponent->SetVariableFloat(TEXT("FlakeSize"), FlakeSize);
	}
	else
	{
		SnowParticleComponent->Deactivate();
	}
}

void UMGWeatherEffectsComponent::UpdateFogEffects(float Density)
{
	// Update screen fog effects
	ScreenEffects.BlurAmount = Density * 0.5f;
	ScreenEffects.ContrastMultiplier = 1.0f - (Density * 0.3f);

	// Fog tint
	if (Density > 0.5f)
	{
		ScreenEffects.ColorTint = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
	}
	else
	{
		ScreenEffects.ColorTint = FLinearColor::White;
	}
}

void UMGWeatherEffectsComponent::UpdateDustEffects(float Intensity)
{
	if (!DustParticleComponent)
	{
		return;
	}

	if (Intensity > 0.0f)
	{
		DustParticleComponent->Activate();
		DustParticleComponent->SetVariableFloat(TEXT("SpawnRateMultiplier"), Intensity);

		// Dust color tint
		ScreenEffects.ColorTint = FMath::Lerp(FLinearColor::White, FLinearColor(0.9f, 0.8f, 0.6f), Intensity);
	}
	else
	{
		DustParticleComponent->Deactivate();
	}
}

void UMGWeatherEffectsComponent::UpdateWindshieldEffects(float MGDeltaTime)
{
	if (!WeatherSubsystem)
	{
		return;
	}

	FMGWeatherState Weather = WeatherSubsystem->GetCurrentWeather();

	// Update droplet accumulation
	if (UMGWeatherSubsystem::IsPrecipitationWeather(Weather.Type))
	{
		// Accumulate droplets
		float AccumulationRate = Weather.Intensity.Precipitation * 50.0f * DeltaTime;
		WindshieldEffects.DropletCount = FMath::Min(WindshieldEffects.DropletCount + AccumulationRate, 100.0f);
	}
	else
	{
		// Evaporate droplets
		float EvaporationRate = 5.0f * DeltaTime;
		WindshieldEffects.DropletCount = FMath::Max(WindshieldEffects.DropletCount - EvaporationRate, 0.0f);
	}

	// Update interior fog based on temperature difference
	float TempDiff = FMath::Abs(Weather.Temperature - 20.0f); // 20 = interior temp
	WindshieldEffects.InteriorFog = FMath::Clamp(TempDiff / 30.0f, 0.0f, 1.0f) * Weather.Intensity.FogDensity;

	// Update windshield material
	if (WindshieldMID)
	{
		WindshieldMID->SetScalarParameterValue(TEXT("DropletCount"), WindshieldEffects.DropletCount);
		WindshieldMID->SetScalarParameterValue(TEXT("DropletSize"), WindshieldEffects.DropletSize);
		WindshieldMID->SetScalarParameterValue(TEXT("StreakAmount"), WindshieldEffects.StreakAmount);
		WindshieldMID->SetScalarParameterValue(TEXT("WiperPosition"), WindshieldEffects.WiperPosition);
		WindshieldMID->SetScalarParameterValue(TEXT("Wetness"), WindshieldEffects.WetnessAmount);
		WindshieldMID->SetScalarParameterValue(TEXT("InteriorFog"), WindshieldEffects.InteriorFog);
	}
}

void UMGWeatherEffectsComponent::UpdateScreenEffects()
{
	if (!WeatherSubsystem)
	{
		return;
	}

	FMGWeatherState Weather = WeatherSubsystem->GetCurrentWeather();

	// Calculate vignette based on visibility
	float VisibilityFactor = FMath::Clamp(1.0f - (Weather.Visibility / 10000.0f), 0.0f, 1.0f);
	ScreenEffects.VignetteIntensity = VisibilityFactor * 0.5f;

	// Film grain for atmospheric effect
	ScreenEffects.FilmGrainIntensity = Weather.Intensity.FogDensity * 0.1f;

	// Saturation reduction in bad weather
	ScreenEffects.SaturationMultiplier = 1.0f - (Weather.Intensity.CloudCoverage * 0.3f);

	ApplyPostProcessSettings();
}

void UMGWeatherEffectsComponent::UpdateAudio()
{
	if (!AmbientAudioComponent || !WeatherSubsystem)
	{
		return;
	}

	FMGWeatherState Weather = WeatherSubsystem->GetCurrentWeather();
	USoundBase* NewSound = nullptr;
	float Volume = 1.0f;

	// Select ambient sound based on weather
	switch (Weather.Type)
	{
	case EMGWeatherType::LightRain:
	case EMGWeatherType::NightRain:
		NewSound = RainAmbientSound;
		Volume = Weather.Intensity.Precipitation;
		break;

	case EMGWeatherType::HeavyRain:
	case EMGWeatherType::Thunderstorm:
		NewSound = HeavyRainAmbientSound;
		Volume = Weather.Intensity.Precipitation;
		break;

	case EMGWeatherType::Snow:
	case EMGWeatherType::Blizzard:
		NewSound = SnowAmbientSound;
		Volume = Weather.Intensity.Wind;
		break;

	case EMGWeatherType::DustStorm:
		NewSound = WindAmbientSound;
		Volume = Weather.Intensity.Wind;
		break;

	default:
		// Check for wind
		if (Weather.WindSpeed > 10.0f)
		{
			NewSound = WindAmbientSound;
			Volume = FMath::Clamp(Weather.WindSpeed / 40.0f, 0.0f, 1.0f);
		}
		break;
	}

	// Update sound
	if (NewSound && NewSound != AmbientAudioComponent->Sound)
	{
		AmbientAudioComponent->SetSound(NewSound);
		AmbientAudioComponent->Play();
	}
	else if (!NewSound && AmbientAudioComponent->IsPlaying())
	{
		AmbientAudioComponent->FadeOut(2.0f, 0.0f);
	}

	// Update volume
	if (NewSound)
	{
		float InteriorMultiplier = bInteriorMode ? 0.5f : 1.0f;
		AmbientAudioComponent->SetVolumeMultiplier(Volume * InteriorMultiplier);
	}
}

void UMGWeatherEffectsComponent::UpdateWipers(float MGDeltaTime)
{
	if (WiperMode == 0)
	{
		return;
	}

	WiperTimer += DeltaTime;

	if (WiperTimer >= WiperInterval)
	{
		WiperTimer = 0.0f;
		ActivateWipers();
	}

	// Update wiper position for animation
	float WipeDuration = 0.5f;
	if (WiperTimer < WipeDuration)
	{
		// Going up
		WindshieldEffects.WiperPosition = WiperTimer / WipeDuration;
	}
	else if (WiperTimer < WipeDuration * 2.0f)
	{
		// Going down
		WindshieldEffects.WiperPosition = 1.0f - ((WiperTimer - WipeDuration) / WipeDuration);
	}
	else
	{
		WindshieldEffects.WiperPosition = 0.0f;
	}

	// Clear droplets when wiper passes
	if (WindshieldEffects.WiperPosition > 0.1f)
	{
		float ClearRate = DeltaTime * 100.0f;
		WindshieldEffects.DropletCount = FMath::Max(WindshieldEffects.DropletCount - ClearRate, 0.0f);
	}
}

void UMGWeatherEffectsComponent::OnWeatherChanged(const FMGWeatherState& NewWeather)
{
	ForceUpdateEffects();
	LastWeatherType = NewWeather.Type;
}

void UMGWeatherEffectsComponent::OnLightningStrike()
{
	// Flash effect
	// Could trigger a brief screen flash or light intensity spike

	// Play thunder after delay (sound travels slower than light)
	float Delay = FMath::RandRange(1.0f, 5.0f);
	PlayThunderSound(Delay);
}

void UMGWeatherEffectsComponent::PlayThunderSound(float Delay)
{
	if (ThunderSounds.Num() == 0)
	{
		return;
	}

	// Select random thunder sound
	int32 Index = FMath::RandRange(0, ThunderSounds.Num() - 1);
	USoundBase* ThunderSound = ThunderSounds[Index];

	if (ThunderSound && GetWorld())
	{
		TWeakObjectPtr<UMGWeatherEffectsComponent> WeakThis(this);
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[WeakThis, ThunderSound]()
			{
				if (WeakThis.IsValid() && WeakThis->GetWorld())
				{
					UGameplayStatics::PlaySound2D(WeakThis->GetWorld(), ThunderSound);
				}
			},
			Delay,
			false
		);
	}
}

void UMGWeatherEffectsComponent::ApplyPostProcessSettings()
{
	if (!PostProcessComponent)
	{
		return;
	}

	FPostProcessSettings& Settings = PostProcessComponent->Settings;

	// Apply vignette
	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = ScreenEffects.VignetteIntensity;

	// Apply color adjustments
	Settings.bOverride_ColorSaturation = true;
	Settings.ColorSaturation = FVector4(ScreenEffects.SaturationMultiplier, ScreenEffects.SaturationMultiplier, ScreenEffects.SaturationMultiplier, 1.0f);

	Settings.bOverride_ColorContrast = true;
	Settings.ColorContrast = FVector4(ScreenEffects.ContrastMultiplier, ScreenEffects.ContrastMultiplier, ScreenEffects.ContrastMultiplier, 1.0f);

	// Apply film grain
	Settings.bOverride_FilmGrainIntensity = true;
	Settings.FilmGrainIntensity = ScreenEffects.FilmGrainIntensity;
}

// ==========================================
// AMGWeatherEffectActor
// ==========================================

AMGWeatherEffectActor::AMGWeatherEffectActor()
{
	PrimaryActorTick.bCanEverTick = true;

	WeatherEffectsComponent = CreateDefaultSubobject<UMGWeatherEffectsComponent>(TEXT("WeatherEffects"));
}

void AMGWeatherEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AMGWeatherEffectActor::Tick(float MGDeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateFollowPosition();
}

void AMGWeatherEffectActor::UpdateFollowPosition()
{
	if (!FollowTarget)
	{
		return;
	}

	FVector TargetLocation = FollowTarget->GetActorLocation();
	TargetLocation.Z += HeightOffset;

	SetActorLocation(TargetLocation);
}

// ==========================================
// UMGRoadSurfaceEffectComponent
// ==========================================

UMGRoadSurfaceEffectComponent::UMGRoadSurfaceEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMGRoadSurfaceEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>();

		if (WeatherSubsystem)
		{
			WeatherSubsystem->OnRoadConditionChanged.AddDynamic(this, &UMGRoadSurfaceEffectComponent::OnRoadConditionChanged);
		}
	}
}

void UMGRoadSurfaceEffectComponent::TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSprayEffects();
}

void UMGRoadSurfaceEffectComponent::SetVehicleSpeed(float Speed)
{
	VehicleSpeed = Speed;
}

void UMGRoadSurfaceEffectComponent::UpdateSprayEffects()
{
	if (!WeatherSubsystem)
	{
		return;
	}

	EMGRoadCondition Condition = WeatherSubsystem->GetRoadCondition();

	// Calculate spray intensity based on road condition and speed
	float BaseIntensity = 0.0f;

	switch (Condition)
	{
	case EMGRoadCondition::Damp:
		BaseIntensity = 0.1f;
		break;
	case EMGRoadCondition::Wet:
		BaseIntensity = 0.4f;
		break;
	case EMGRoadCondition::StandingWater:
		BaseIntensity = 1.0f;
		break;
	case EMGRoadCondition::Snowy:
		BaseIntensity = 0.3f;
		break;
	default:
		BaseIntensity = 0.0f;
		break;
	}

	// Speed factor (more spray at higher speeds)
	float SpeedFactor = FMath::Clamp(VehicleSpeed / 150.0f, 0.0f, 1.0f); // 150 km/h = full spray

	CurrentSprayIntensity = BaseIntensity * SpeedFactor * SprayIntensityMultiplier;

	// Update tire spray particle components
	for (UNiagaraComponent* TireSpray : TireSprayComponents)
	{
		if (TireSpray)
		{
			if (CurrentSprayIntensity > 0.0f)
			{
				TireSpray->Activate();
				TireSpray->SetVariableFloat(TEXT("SprayIntensity"), CurrentSprayIntensity);
			}
			else
			{
				TireSpray->Deactivate();
			}
		}
	}
}

void UMGRoadSurfaceEffectComponent::OnRoadConditionChanged(EMGRoadCondition NewCondition)
{
	// Could trigger transition effects here
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Audio/MGVehicleSFXComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UMGVehicleSFXComponent::UMGVehicleSFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	// Default thresholds
	LightCollisionThreshold = 50000.0f;
	MediumCollisionThreshold = 200000.0f;
	HeavyCollisionThreshold = 500000.0f;

	WindNoiseMinSpeed = 2000.0f;  // ~72 km/h
	WindNoiseMaxSpeed = 8000.0f;  // ~288 km/h

	CollisionCooldown = 0.1f;
}

void UMGVehicleSFXComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeAudioComponents();
}

void UMGVehicleSFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupAudioComponents();
	Super::EndPlay(EndPlayReason);
}

void UMGVehicleSFXComponent::TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsEnabled)
	{
		return;
	}

	TimeSinceLastCollision += DeltaTime;

	UpdateTireSounds(DeltaTime);
	UpdateWindNoise(DeltaTime);
	UpdateBrakeSounds(DeltaTime);
}

// ==========================================
// VEHICLE STATE INPUT
// ==========================================

void UMGVehicleSFXComponent::SetSpeed(float Speed)
{
	CurrentSpeed = FMath::Max(0.0f, Speed);
}

void UMGVehicleSFXComponent::SetTireSlip(float FrontSlip, float RearSlip)
{
	CurrentFrontSlip = FMath::Clamp(FrontSlip, 0.0f, 2.0f);
	CurrentRearSlip = FMath::Clamp(RearSlip, 0.0f, 2.0f);
}

void UMGVehicleSFXComponent::SetSurfaceType(EMGSurfaceType Surface)
{
	if (CurrentSurface != Surface)
	{
		CurrentSurface = Surface;

		// Update tire sounds for new surface
		const FMGSurfaceSoundConfig* Config = GetCurrentSurfaceConfig();
		if (Config)
		{
			if (TireRollComponent && Config->TireRollSound)
			{
				TireRollComponent->SetSound(Config->TireRollSound);
			}
			if (TireSkidComponent && Config->TireSkidSound)
			{
				TireSkidComponent->SetSound(Config->TireSkidSound);
			}
		}
	}
}

void UMGVehicleSFXComponent::SetSurfaceFromPhysMat(UPhysicalMaterial* PhysMat)
{
	SetSurfaceType(PhysMatToSurfaceType(PhysMat));
}

void UMGVehicleSFXComponent::SetBrakeInput(float Brake)
{
	CurrentBrake = FMath::Clamp(Brake, 0.0f, 1.0f);
}

void UMGVehicleSFXComponent::SetAirborne(bool bInAir)
{
	bIsAirborne = bInAir;
}

// ==========================================
// COLLISION EVENTS
// ==========================================

void UMGVehicleSFXComponent::OnCollision(float ImpactForce, FVector ImpactLocation, FVector ImpactNormal)
{
	if (!bIsEnabled || TimeSinceLastCollision < CollisionCooldown)
	{
		return;
	}

	EMGCollisionIntensity Intensity = GetCollisionIntensity(ImpactForce);
	USoundBase* CollisionSound = GetRandomCollisionSound(Intensity);

	if (CollisionSound)
	{
		// Calculate volume based on intensity
		float Volume = MasterVolume;
		switch (Intensity)
		{
		case EMGCollisionIntensity::Light:
			Volume *= 0.5f;
			break;
		case EMGCollisionIntensity::Medium:
			Volume *= 0.75f;
			break;
		case EMGCollisionIntensity::Heavy:
			Volume *= 1.0f;
			break;
		case EMGCollisionIntensity::Extreme:
			Volume *= 1.2f;
			break;
		}

		// Play at impact location
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(), CollisionSound, ImpactLocation,
			Volume, FMath::RandRange(0.9f, 1.1f));

		TimeSinceLastCollision = 0.0f;

		UE_LOG(LogTemp, Verbose, TEXT("VehicleSFX: Collision - Intensity: %d, Force: %.0f"),
			static_cast<int32>(Intensity), ImpactForce);
	}
}

void UMGVehicleSFXComponent::StartScrape(float Intensity)
{
	bIsScraping = true;

	if (ScrapeComponent && CollisionConfig.ScrapeLoop)
	{
		if (!ScrapeComponent->IsPlaying())
		{
			ScrapeComponent->SetSound(CollisionConfig.ScrapeLoop);
			ScrapeComponent->Play();
		}
		ScrapeComponent->SetVolumeMultiplier(Intensity * MasterVolume);
	}
}

void UMGVehicleSFXComponent::StopScrape()
{
	bIsScraping = false;

	if (ScrapeComponent)
	{
		ScrapeComponent->Stop();
	}
}

void UMGVehicleSFXComponent::PlayGlassBreak(FVector Location)
{
	if (CollisionConfig.GlassBreak)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(), CollisionConfig.GlassBreak, Location,
			MasterVolume, FMath::RandRange(0.9f, 1.1f));
	}
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGVehicleSFXComponent::AddSurfaceConfig(const FMGSurfaceSoundConfig& Config)
{
	SurfaceConfigs.Add(Config.SurfaceType, Config);
}

void UMGVehicleSFXComponent::SetCollisionConfig(const FMGCollisionSoundConfig& Config)
{
	CollisionConfig = Config;
}

void UMGVehicleSFXComponent::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
}

void UMGVehicleSFXComponent::SetEnabled(bool bEnabled)
{
	bIsEnabled = bEnabled;

	if (!bEnabled)
	{
		// Mute all components
		if (TireRollComponent) TireRollComponent->SetVolumeMultiplier(0.0f);
		if (TireSkidComponent) TireSkidComponent->SetVolumeMultiplier(0.0f);
		if (WindNoiseComponent) WindNoiseComponent->SetVolumeMultiplier(0.0f);
		if (ScrapeComponent) ScrapeComponent->SetVolumeMultiplier(0.0f);
		if (BrakeComponent) BrakeComponent->SetVolumeMultiplier(0.0f);
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGVehicleSFXComponent::InitializeAudioComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Create tire roll component
	TireRollComponent = NewObject<UAudioComponent>(Owner);
	TireRollComponent->SetupAttachment(Owner->GetRootComponent());
	TireRollComponent->bAutoActivate = false;
	TireRollComponent->bAllowSpatialization = true;
	TireRollComponent->RegisterComponent();

	// Create tire skid component
	TireSkidComponent = NewObject<UAudioComponent>(Owner);
	TireSkidComponent->SetupAttachment(Owner->GetRootComponent());
	TireSkidComponent->bAutoActivate = false;
	TireSkidComponent->bAllowSpatialization = true;
	TireSkidComponent->RegisterComponent();

	// Create wind noise component
	if (WindNoiseSound)
	{
		WindNoiseComponent = NewObject<UAudioComponent>(Owner);
		WindNoiseComponent->SetupAttachment(Owner->GetRootComponent());
		WindNoiseComponent->SetSound(WindNoiseSound);
		WindNoiseComponent->bAutoActivate = true;
		WindNoiseComponent->bAllowSpatialization = true;
		WindNoiseComponent->SetVolumeMultiplier(0.0f);
		WindNoiseComponent->RegisterComponent();
		WindNoiseComponent->Play();
	}

	// Create scrape component
	ScrapeComponent = NewObject<UAudioComponent>(Owner);
	ScrapeComponent->SetupAttachment(Owner->GetRootComponent());
	ScrapeComponent->bAutoActivate = false;
	ScrapeComponent->bAllowSpatialization = true;
	ScrapeComponent->RegisterComponent();

	// Create brake component
	if (BrakeSquealSound)
	{
		BrakeComponent = NewObject<UAudioComponent>(Owner);
		BrakeComponent->SetupAttachment(Owner->GetRootComponent());
		BrakeComponent->SetSound(BrakeSquealSound);
		BrakeComponent->bAutoActivate = false;
		BrakeComponent->bAllowSpatialization = true;
		BrakeComponent->RegisterComponent();
	}

	// Initialize with default surface config
	const FMGSurfaceSoundConfig* Config = GetCurrentSurfaceConfig();
	if (Config)
	{
		if (TireRollComponent && Config->TireRollSound)
		{
			TireRollComponent->SetSound(Config->TireRollSound);
		}
		if (TireSkidComponent && Config->TireSkidSound)
		{
			TireSkidComponent->SetSound(Config->TireSkidSound);
		}
	}
}

void UMGVehicleSFXComponent::CleanupAudioComponents()
{
	auto DestroyComp = [](UAudioComponent*& Comp)
	{
		if (Comp)
		{
			Comp->Stop();
			Comp->DestroyComponent();
			Comp = nullptr;
		}
	};

	DestroyComp(TireRollComponent);
	DestroyComp(TireSkidComponent);
	DestroyComp(WindNoiseComponent);
	DestroyComp(ScrapeComponent);
	DestroyComp(BrakeComponent);
}

void UMGVehicleSFXComponent::UpdateTireSounds(float MGDeltaTime)
{
	if (bIsAirborne)
	{
		// No tire sounds when airborne
		if (TireRollComponent) TireRollComponent->SetVolumeMultiplier(0.0f);
		if (TireSkidComponent) TireSkidComponent->SetVolumeMultiplier(0.0f);
		bIsSkidding = false;
		return;
	}

	const FMGSurfaceSoundConfig* Config = GetCurrentSurfaceConfig();
	float SurfaceVolMult = Config ? Config->VolumeMultiplier : 1.0f;
	float SurfacePitchMult = Config ? Config->PitchMultiplier : 1.0f;
	float SkidThreshold = Config ? Config->SkidThreshold : 0.2f;

	// Tire roll sound
	if (TireRollComponent)
	{
		if (!TireRollComponent->IsPlaying() && Config && Config->TireRollSound)
		{
			TireRollComponent->Play();
		}

		// Volume based on speed
		float SpeedNorm = FMath::Clamp(CurrentSpeed / 5000.0f, 0.0f, 1.0f);
		float RollVolume = SpeedNorm * SurfaceVolMult * MasterVolume;

		// Pitch based on speed
		float RollPitch = FMath::Lerp(0.8f, 1.5f, SpeedNorm) * SurfacePitchMult;

		TireRollComponent->SetVolumeMultiplier(RollVolume);
		TireRollComponent->SetPitchMultiplier(RollPitch);
	}

	// Tire skid sound
	float MaxSlip = FMath::Max(CurrentFrontSlip, CurrentRearSlip);
	bIsSkidding = MaxSlip > SkidThreshold && CurrentSpeed > 500.0f;

	if (TireSkidComponent)
	{
		if (bIsSkidding)
		{
			if (!TireSkidComponent->IsPlaying() && Config && Config->TireSkidSound)
			{
				TireSkidComponent->Play();
			}

			// Volume based on slip amount
			float SlipFactor = FMath::Clamp((MaxSlip - SkidThreshold) / (1.0f - SkidThreshold), 0.0f, 1.0f);
			float SkidVolume = SlipFactor * SurfaceVolMult * MasterVolume;

			// Pitch varies slightly with speed
			float SkidPitch = FMath::Lerp(0.9f, 1.2f, FMath::Clamp(CurrentSpeed / 4000.0f, 0.0f, 1.0f)) * SurfacePitchMult;

			TireSkidComponent->SetVolumeMultiplier(SkidVolume);
			TireSkidComponent->SetPitchMultiplier(SkidPitch);
		}
		else
		{
			// Fade out skid
			float CurrentVol = TireSkidComponent->VolumeMultiplier;
			TireSkidComponent->SetVolumeMultiplier(FMath::FInterpTo(CurrentVol, 0.0f, DeltaTime, 10.0f));
		}
	}
}

void UMGVehicleSFXComponent::UpdateWindNoise(float MGDeltaTime)
{
	if (!WindNoiseComponent)
	{
		return;
	}

	// Wind noise increases with speed
	float WindFactor = FMath::GetMappedRangeValueClamped(
		FVector2D(WindNoiseMinSpeed, WindNoiseMaxSpeed),
		FVector2D(0.0f, 1.0f),
		CurrentSpeed);

	float WindVolume = WindFactor * MasterVolume * 0.5f; // Wind is generally quieter
	float WindPitch = FMath::Lerp(0.8f, 1.3f, WindFactor);

	WindNoiseComponent->SetVolumeMultiplier(WindVolume);
	WindNoiseComponent->SetPitchMultiplier(WindPitch);
}

void UMGVehicleSFXComponent::UpdateBrakeSounds(float MGDeltaTime)
{
	if (!BrakeComponent)
	{
		return;
	}

	// Brake squeal when braking hard at speed
	bool bShouldSqueal = CurrentBrake > 0.7f && CurrentSpeed > 1500.0f && !bIsAirborne;

	if (bShouldSqueal)
	{
		if (!BrakeComponent->IsPlaying())
		{
			BrakeComponent->Play();
		}

		float BrakeVolume = (CurrentBrake - 0.7f) / 0.3f * MasterVolume * 0.6f;
		BrakeComponent->SetVolumeMultiplier(BrakeVolume);
	}
	else
	{
		// Fade out
		float CurrentVol = BrakeComponent->VolumeMultiplier;
		if (CurrentVol > 0.01f)
		{
			BrakeComponent->SetVolumeMultiplier(FMath::FInterpTo(CurrentVol, 0.0f, DeltaTime, 8.0f));
		}
		else if (BrakeComponent->IsPlaying())
		{
			BrakeComponent->Stop();
		}
	}
}

EMGCollisionIntensity UMGVehicleSFXComponent::GetCollisionIntensity(float Force) const
{
	if (Force >= HeavyCollisionThreshold * 1.5f)
	{
		return EMGCollisionIntensity::Extreme;
	}
	else if (Force >= HeavyCollisionThreshold)
	{
		return EMGCollisionIntensity::Heavy;
	}
	else if (Force >= MediumCollisionThreshold)
	{
		return EMGCollisionIntensity::Medium;
	}
	else
	{
		return EMGCollisionIntensity::Light;
	}
}

USoundBase* UMGVehicleSFXComponent::GetRandomCollisionSound(EMGCollisionIntensity Intensity) const
{
	const TArray<USoundBase*>* Sounds = nullptr;

	switch (Intensity)
	{
	case EMGCollisionIntensity::Light:
		Sounds = &CollisionConfig.LightImpacts;
		break;
	case EMGCollisionIntensity::Medium:
		Sounds = &CollisionConfig.MediumImpacts;
		break;
	case EMGCollisionIntensity::Heavy:
		Sounds = &CollisionConfig.HeavyImpacts;
		break;
	case EMGCollisionIntensity::Extreme:
		Sounds = &CollisionConfig.ExtremeImpacts;
		break;
	}

	if (Sounds && Sounds->Num() > 0)
	{
		int32 Index = FMath::RandRange(0, Sounds->Num() - 1);
		return (*Sounds)[Index];
	}

	return nullptr;
}

const FMGSurfaceSoundConfig* UMGVehicleSFXComponent::GetCurrentSurfaceConfig() const
{
	return SurfaceConfigs.Find(CurrentSurface);
}

EMGSurfaceType UMGVehicleSFXComponent::PhysMatToSurfaceType(UPhysicalMaterial* PhysMat) const
{
	if (!PhysMat)
	{
		return EMGSurfaceType::Asphalt;
	}

	// Map physical material name to surface type
	// This would ideally use a custom physical material subclass or data asset
	FString MatName = PhysMat->GetName().ToLower();

	if (MatName.Contains(TEXT("gravel")) || MatName.Contains(TEXT("rock")))
	{
		return EMGSurfaceType::Gravel;
	}
	else if (MatName.Contains(TEXT("dirt")) || MatName.Contains(TEXT("mud")))
	{
		return EMGSurfaceType::Dirt;
	}
	else if (MatName.Contains(TEXT("grass")))
	{
		return EMGSurfaceType::Grass;
	}
	else if (MatName.Contains(TEXT("sand")))
	{
		return EMGSurfaceType::Sand;
	}
	else if (MatName.Contains(TEXT("water")) || MatName.Contains(TEXT("wet")))
	{
		return EMGSurfaceType::Water;
	}
	else if (MatName.Contains(TEXT("metal")))
	{
		return EMGSurfaceType::Metal;
	}
	else if (MatName.Contains(TEXT("wood")))
	{
		return EMGSurfaceType::Wood;
	}
	else if (MatName.Contains(TEXT("concrete")))
	{
		return EMGSurfaceType::Concrete;
	}

	return EMGSurfaceType::Asphalt;
}

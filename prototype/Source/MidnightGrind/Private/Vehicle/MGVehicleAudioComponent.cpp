// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleAudioComponent.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UMGVehicleAudioComponent::UMGVehicleAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UMGVehicleAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	// Auto-initialize with owner if it's a vehicle pawn
	if (AMGVehiclePawn* VehiclePawn = Cast<AMGVehiclePawn>(GetOwner()))
	{
		Initialize(VehiclePawn);
	}
}

void UMGVehicleAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAudio();
	Super::EndPlay(EndPlayReason);
}

void UMGVehicleAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAudioActive || !OwnerVehicle.IsValid())
	{
		return;
	}

	UpdateEngineSounds(DeltaTime);
	UpdateTireSounds(DeltaTime);
	UpdateWindSound(DeltaTime);
	UpdateForcedInductionSounds(DeltaTime);
	UpdateBackfires(DeltaTime);

	// Update cooldowns
	if (BackfireCooldown > 0.0f)
	{
		BackfireCooldown -= DeltaTime;
	}
	if (ScrapeSoundTimer > 0.0f)
	{
		ScrapeSoundTimer -= DeltaTime;
	}
}

// ==========================================
// CONTROL
// ==========================================

void UMGVehicleAudioComponent::Initialize(AMGVehiclePawn* InVehicle)
{
	OwnerVehicle = InVehicle;

	// Create engine layer components
	if (EngineLayers.Num() > 0)
	{
		for (int32 i = 0; i < EngineLayers.Num(); ++i)
		{
			const FMGEngineSoundLayer& Layer = EngineLayers[i];
			if (Layer.Sound)
			{
				UAudioComponent* LayerComp = GetOrCreateAudioComponent(
					*FString::Printf(TEXT("EngineLayer_%d"), i),
					Layer.Sound
				);
				if (LayerComp)
				{
					EngineLayerComponents.Add(LayerComp);
					LayerComp->bAutoDestroy = false;
				}
			}
		}
	}
	else if (SimpleEngineSound)
	{
		// Use simple engine sound
		SimpleEngineComponent = GetOrCreateAudioComponent(TEXT("SimpleEngine"), SimpleEngineSound);
	}

	// Create exhaust component
	if (ExhaustSound)
	{
		ExhaustComponent = GetOrCreateAudioComponent(TEXT("Exhaust"), ExhaustSound);
	}

	// Create turbo component
	if (TurboSound)
	{
		TurboComponent = GetOrCreateAudioComponent(TEXT("Turbo"), TurboSound);
	}

	// Create supercharger component
	if (SuperchargerSound)
	{
		SuperchargerComponent = GetOrCreateAudioComponent(TEXT("Supercharger"), SuperchargerSound);
	}

	// Create tire skid components (4 wheels)
	if (DefaultTireSounds.SkidSound)
	{
		for (int32 i = 0; i < 4; ++i)
		{
			UAudioComponent* SkidComp = GetOrCreateAudioComponent(
				*FString::Printf(TEXT("TireSkid_%d"), i),
				DefaultTireSounds.SkidSound
			);
			if (SkidComp)
			{
				TireSkidComponents.Add(SkidComp);
			}
		}
	}

	// Create wind component
	if (WindSound)
	{
		WindComponent = GetOrCreateAudioComponent(TEXT("Wind"), WindSound);
	}

	// Create NOS component
	if (NOSLoopSound)
	{
		NOSComponent = GetOrCreateAudioComponent(TEXT("NOS"), NOSLoopSound);
	}

	// Create scrape component
	if (ScrapeSound)
	{
		ScrapeComponent = GetOrCreateAudioComponent(TEXT("Scrape"), ScrapeSound);
	}
}

void UMGVehicleAudioComponent::StartAudio()
{
	bAudioActive = true;

	// Start engine layers
	for (UAudioComponent* Comp : EngineLayerComponents)
	{
		if (Comp && !Comp->IsPlaying())
		{
			Comp->Play();
		}
	}

	// Start simple engine
	if (SimpleEngineComponent && !SimpleEngineComponent->IsPlaying())
	{
		SimpleEngineComponent->Play();
	}

	// Start exhaust
	if (ExhaustComponent && !ExhaustComponent->IsPlaying())
	{
		ExhaustComponent->Play();
	}

	// Wind starts quiet and fades in with speed
	if (WindComponent && !WindComponent->IsPlaying())
	{
		WindComponent->SetVolumeMultiplier(0.0f);
		WindComponent->Play();
	}
}

void UMGVehicleAudioComponent::StopAudio()
{
	bAudioActive = false;

	// Stop all components
	for (UAudioComponent* Comp : EngineLayerComponents)
	{
		if (Comp)
		{
			Comp->Stop();
		}
	}

	if (SimpleEngineComponent)
	{
		SimpleEngineComponent->Stop();
	}

	if (ExhaustComponent)
	{
		ExhaustComponent->Stop();
	}

	if (TurboComponent)
	{
		TurboComponent->Stop();
	}

	if (SuperchargerComponent)
	{
		SuperchargerComponent->Stop();
	}

	for (UAudioComponent* Comp : TireSkidComponents)
	{
		if (Comp)
		{
			Comp->Stop();
		}
	}

	if (WindComponent)
	{
		WindComponent->Stop();
	}

	if (NOSComponent)
	{
		NOSComponent->Stop();
	}

	if (ScrapeComponent)
	{
		ScrapeComponent->Stop();
	}
}

void UMGVehicleAudioComponent::PauseAudio(bool bPause)
{
	// Pause/unpause all audio components
	auto PauseComponent = [bPause](UAudioComponent* Comp)
	{
		if (Comp)
		{
			Comp->SetPaused(bPause);
		}
	};

	for (UAudioComponent* Comp : EngineLayerComponents)
	{
		PauseComponent(Comp);
	}
	PauseComponent(SimpleEngineComponent);
	PauseComponent(ExhaustComponent);
	PauseComponent(TurboComponent);
	PauseComponent(SuperchargerComponent);
	for (UAudioComponent* Comp : TireSkidComponents)
	{
		PauseComponent(Comp);
	}
	PauseComponent(WindComponent);
	PauseComponent(NOSComponent);
	PauseComponent(ScrapeComponent);
}

void UMGVehicleAudioComponent::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
}

// ==========================================
// EVENTS
// ==========================================

void UMGVehicleAudioComponent::OnGearChanged(int32 OldGear, int32 NewGear)
{
	if (!bAudioActive)
	{
		return;
	}

	// Neutral to gear or reverse
	if (OldGear == 0 && NewGear != 0)
	{
		PlayOneShot2D(ClutchSound, 0.7f);
		return;
	}

	// Shift up
	if (NewGear > OldGear && NewGear > 0)
	{
		PlayOneShot2D(ShiftUpSound);

		// Check for blow-off on shift (if turbocharged)
		if (TurboComponent && TurboComponent->IsPlaying())
		{
			PlayBlowOff();
		}
	}
	// Shift down
	else if (NewGear < OldGear)
	{
		PlayOneShot2D(ShiftDownSound);
	}

	PreviousGear = NewGear;
}

void UMGVehicleAudioComponent::OnNOSStateChanged(bool bActive)
{
	if (bNOSActive == bActive)
	{
		return;
	}

	bNOSActive = bActive;

	if (bActive)
	{
		// Play activation sound
		PlayOneShot2D(NOSActivateSound);

		// Start loop
		if (NOSComponent && !NOSComponent->IsPlaying())
		{
			NOSComponent->Play();
		}
	}
	else
	{
		// Play deactivation sound
		PlayOneShot2D(NOSDeactivateSound);

		// Stop loop
		if (NOSComponent)
		{
			NOSComponent->Stop();
		}
	}
}

void UMGVehicleAudioComponent::OnCollision(float ImpactVelocity, bool bIsScrape)
{
	if (!bAudioActive)
	{
		return;
	}

	if (bIsScrape)
	{
		bIsScraping = true;

		// Start scrape sound if not playing
		if (ScrapeComponent && !ScrapeComponent->IsPlaying())
		{
			ScrapeComponent->Play();
		}
	}
	else
	{
		// One-shot impact sound
		if (ImpactVelocity >= HeavyImpactThreshold)
		{
			PlayOneShotAtLocation(HeavyImpactSound, GetOwner()->GetActorLocation());
		}
		else if (ImpactVelocity >= 5.0f)
		{
			float VolumeScale = FMath::GetMappedRangeValueClamped(
				FVector2D(5.0f, HeavyImpactThreshold),
				FVector2D(0.3f, 1.0f),
				ImpactVelocity
			);
			PlayOneShotAtLocation(LightImpactSound, GetOwner()->GetActorLocation(), VolumeScale);
		}
	}
}

void UMGVehicleAudioComponent::PlayBackfire()
{
	if (BackfireConfig.BackfireSounds.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, BackfireConfig.BackfireSounds.Num() - 1);
		USoundBase* Sound = BackfireConfig.BackfireSounds[Index];

		if (GetOwner())
		{
			PlayOneShotAtLocation(Sound, GetOwner()->GetActorLocation());
		}
	}
}

void UMGVehicleAudioComponent::PlayBlowOff()
{
	if (BlowOffSound && GetOwner())
	{
		PlayOneShotAtLocation(BlowOffSound, GetOwner()->GetActorLocation(), 0.8f);
	}
}

// ==========================================
// UPDATE
// ==========================================

void UMGVehicleAudioComponent::UpdateEngineSounds(float DeltaTime)
{
	if (!OwnerVehicle.IsValid())
	{
		return;
	}

	// Get current engine state
	UMGVehicleMovementComponent* Movement = OwnerVehicle->GetVehicleMovementComponent();
	if (!Movement)
	{
		return;
	}

	FMGEngineState EngineState = Movement->GetEngineState();
	float CurrentRPM = EngineState.CurrentRPM;
	float MaxRPM = EngineState.MaxRPM;
	float Throttle = EngineState.ThrottlePosition;

	// Smooth RPM for audio (reduces harshness)
	SmoothedRPM = FMath::FInterpTo(SmoothedRPM, CurrentRPM, DeltaTime, RPMSmoothingFactor);

	// Update engine layers
	if (EngineLayerComponents.Num() > 0)
	{
		for (int32 i = 0; i < EngineLayerComponents.Num() && i < EngineLayers.Num(); ++i)
		{
			UAudioComponent* Comp = EngineLayerComponents[i];
			const FMGEngineSoundLayer& Layer = EngineLayers[i];

			if (!Comp)
			{
				continue;
			}

			float Volume = GetLayerVolume(Layer, SmoothedRPM, Throttle) * MasterVolume;
			float Pitch = GetLayerPitch(Layer, SmoothedRPM);

			Comp->SetVolumeMultiplier(Volume);
			Comp->SetPitchMultiplier(Pitch);
		}
	}
	else if (SimpleEngineComponent)
	{
		// Simple engine - basic pitch and volume control
		float RPMPercent = FMath::Clamp(SmoothedRPM / MaxRPM, 0.0f, 1.0f);
		float Pitch = FMath::Lerp(0.5f, 2.0f, RPMPercent);
		float Volume = FMath::Lerp(0.3f, 1.0f, Throttle) * MasterVolume;

		SimpleEngineComponent->SetPitchMultiplier(Pitch);
		SimpleEngineComponent->SetVolumeMultiplier(Volume);
	}

	// Update exhaust (follows engine but with resonance characteristics)
	if (ExhaustComponent)
	{
		float RPMPercent = FMath::Clamp(SmoothedRPM / MaxRPM, 0.0f, 1.0f);
		float ExhaustPitch = FMath::Lerp(0.6f, 1.8f, RPMPercent);
		float ExhaustVolume = FMath::Lerp(0.2f, 0.8f, Throttle) * MasterVolume;

		ExhaustComponent->SetPitchMultiplier(ExhaustPitch);
		ExhaustComponent->SetVolumeMultiplier(ExhaustVolume);
	}

	// Store throttle for backfire detection
	PreviousThrottle = Throttle;
}

void UMGVehicleAudioComponent::UpdateTireSounds(float DeltaTime)
{
	if (!OwnerVehicle.IsValid())
	{
		return;
	}

	UMGVehicleMovementComponent* Movement = OwnerVehicle->GetVehicleMovementComponent();
	if (!Movement)
	{
		return;
	}

	// Get slip values for each wheel (simplified - using overall drift state)
	FMGDriftState DriftState = Movement->GetDriftState();
	float SlipAmount = DriftState.DriftAngle / 90.0f; // Normalize to 0-1
	bool bShouldSkid = SlipAmount > SkidThreshold;

	for (int32 i = 0; i < TireSkidComponents.Num(); ++i)
	{
		UAudioComponent* SkidComp = TireSkidComponents[i];
		if (!SkidComp)
		{
			continue;
		}

		if (bShouldSkid)
		{
			if (!SkidComp->IsPlaying())
			{
				SkidComp->Play();
			}

			// Volume based on slip amount
			float SkidVolume = FMath::GetMappedRangeValueClamped(
				FVector2D(SkidThreshold, 0.8f),
				FVector2D(0.1f, 1.0f),
				SlipAmount
			) * MasterVolume;

			SkidComp->SetVolumeMultiplier(SkidVolume);
		}
		else
		{
			if (SkidComp->IsPlaying())
			{
				SkidComp->Stop();
			}
		}
	}

	// Stop scraping if no longer colliding
	if (ScrapeComponent && ScrapeComponent->IsPlaying())
	{
		// Scraping stops automatically after a timeout
		ScrapeSoundTimer += DeltaTime;
		if (ScrapeSoundTimer > 0.3f)
		{
			bIsScraping = false;
			ScrapeComponent->Stop();
		}
	}
}

void UMGVehicleAudioComponent::UpdateWindSound(float DeltaTime)
{
	if (!WindComponent || !OwnerVehicle.IsValid())
	{
		return;
	}

	UMGVehicleMovementComponent* Movement = OwnerVehicle->GetVehicleMovementComponent();
	if (!Movement)
	{
		return;
	}

	float SpeedKPH = Movement->GetForwardSpeed() * 0.036f; // cm/s to km/h
	float WindVolume = FMath::GetMappedRangeValueClamped(
		FVector2D(30.0f, WindFullSpeedKPH),
		FVector2D(0.0f, 0.6f),
		SpeedKPH
	) * MasterVolume;

	float WindPitch = FMath::GetMappedRangeValueClamped(
		FVector2D(30.0f, WindFullSpeedKPH),
		FVector2D(0.8f, 1.3f),
		SpeedKPH
	);

	WindComponent->SetVolumeMultiplier(WindVolume);
	WindComponent->SetPitchMultiplier(WindPitch);
}

void UMGVehicleAudioComponent::UpdateForcedInductionSounds(float DeltaTime)
{
	if (!OwnerVehicle.IsValid())
	{
		return;
	}

	UMGVehicleMovementComponent* Movement = OwnerVehicle->GetVehicleMovementComponent();
	if (!Movement)
	{
		return;
	}

	FMGEngineState EngineState = Movement->GetEngineState();

	// Turbo whine - increases with RPM and boost pressure
	if (TurboComponent)
	{
		float BoostPressure = EngineState.BoostPressure; // Assuming this exists
		float RPMFactor = FMath::Clamp(SmoothedRPM / EngineState.MaxRPM, 0.0f, 1.0f);

		float TurboVolume = FMath::Clamp(BoostPressure * RPMFactor * 0.5f, 0.0f, 0.7f) * MasterVolume;
		float TurboPitch = FMath::Lerp(0.8f, 2.0f, RPMFactor);

		if (TurboVolume > 0.05f)
		{
			if (!TurboComponent->IsPlaying())
			{
				TurboComponent->Play();
			}
			TurboComponent->SetVolumeMultiplier(TurboVolume);
			TurboComponent->SetPitchMultiplier(TurboPitch);
		}
		else
		{
			if (TurboComponent->IsPlaying())
			{
				TurboComponent->Stop();
			}
		}
	}

	// Supercharger whine - constant pitch, volume based on RPM
	if (SuperchargerComponent)
	{
		float RPMFactor = FMath::Clamp(SmoothedRPM / EngineState.MaxRPM, 0.0f, 1.0f);
		float SCVolume = FMath::Lerp(0.0f, 0.5f, RPMFactor) * MasterVolume;
		float SCPitch = FMath::Lerp(0.9f, 1.5f, RPMFactor);

		if (SCVolume > 0.05f)
		{
			if (!SuperchargerComponent->IsPlaying())
			{
				SuperchargerComponent->Play();
			}
			SuperchargerComponent->SetVolumeMultiplier(SCVolume);
			SuperchargerComponent->SetPitchMultiplier(SCPitch);
		}
		else
		{
			if (SuperchargerComponent->IsPlaying())
			{
				SuperchargerComponent->Stop();
			}
		}
	}
}

void UMGVehicleAudioComponent::UpdateBackfires(float DeltaTime)
{
	if (BackfireConfig.BackfireSounds.Num() == 0 || BackfireCooldown > 0.0f)
	{
		return;
	}

	if (!OwnerVehicle.IsValid())
	{
		return;
	}

	UMGVehicleMovementComponent* Movement = OwnerVehicle->GetVehicleMovementComponent();
	if (!Movement)
	{
		return;
	}

	FMGEngineState EngineState = Movement->GetEngineState();
	float CurrentThrottle = EngineState.ThrottlePosition;

	// Check for throttle lift at high RPM
	if (SmoothedRPM >= BackfireConfig.MinRPM &&
		PreviousThrottle > BackfireConfig.ThrottleLiftThreshold &&
		CurrentThrottle < BackfireConfig.ThrottleLiftThreshold)
	{
		// Random chance for backfire
		if (FMath::FRand() < BackfireConfig.Probability)
		{
			PlayBackfire();
			BackfireCooldown = BackfireConfig.CooldownTime;
		}
	}
}

// ==========================================
// HELPERS
// ==========================================

float UMGVehicleAudioComponent::GetLayerVolume(const FMGEngineSoundLayer& Layer, float RPM, float Load) const
{
	float Volume = 0.0f;

	// Fade in region
	if (RPM < Layer.FadeInRPM)
	{
		Volume = 0.0f;
	}
	// Ramp up
	else if (RPM < Layer.FullVolumeRPM)
	{
		Volume = FMath::GetMappedRangeValueClamped(
			FVector2D(Layer.FadeInRPM, Layer.FullVolumeRPM),
			FVector2D(0.0f, 1.0f),
			RPM
		);
	}
	// Full volume
	else if (RPM < Layer.FadeOutRPM)
	{
		Volume = 1.0f;
	}
	// Fade out
	else if (RPM < Layer.SilentRPM)
	{
		Volume = FMath::GetMappedRangeValueClamped(
			FVector2D(Layer.FadeOutRPM, Layer.SilentRPM),
			FVector2D(1.0f, 0.0f),
			RPM
		);
	}
	else
	{
		Volume = 0.0f;
	}

	// Apply base volume
	Volume *= Layer.BaseVolume;

	// Apply load modulation (throttle affects volume)
	if (Layer.bApplyLoadModulation)
	{
		float LoadMod = FMath::Lerp(1.0f - Layer.LoadModulationStrength, 1.0f, Load);
		Volume *= LoadMod;
	}

	return Volume;
}

float UMGVehicleAudioComponent::GetLayerPitch(const FMGEngineSoundLayer& Layer, float RPM) const
{
	float RPMRange = Layer.SilentRPM - Layer.FadeInRPM;
	if (RPMRange <= 0.0f)
	{
		return Layer.PitchRange.X;
	}

	float NormalizedRPM = FMath::Clamp((RPM - Layer.FadeInRPM) / RPMRange, 0.0f, 1.0f);
	return FMath::Lerp(Layer.PitchRange.X, Layer.PitchRange.Y, NormalizedRPM);
}

UAudioComponent* UMGVehicleAudioComponent::GetOrCreateAudioComponent(FName Name, USoundBase* Sound)
{
	if (!Sound || !GetOwner())
	{
		return nullptr;
	}

	UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetOwner(), Name);
	if (AudioComp)
	{
		AudioComp->SetSound(Sound);
		AudioComp->bAutoActivate = false;
		AudioComp->bAutoDestroy = false;
		AudioComp->RegisterComponent();
		AudioComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}

	return AudioComp;
}

void UMGVehicleAudioComponent::PlayOneShotAtLocation(USoundBase* Sound, FVector Location, float Volume, float Pitch)
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			Sound,
			Location,
			FRotator::ZeroRotator,
			Volume * MasterVolume,
			Pitch
		);
	}
}

void UMGVehicleAudioComponent::PlayOneShot2D(USoundBase* Sound, float Volume, float Pitch)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(
			this,
			Sound,
			Volume * MasterVolume,
			Pitch
		);
	}
}

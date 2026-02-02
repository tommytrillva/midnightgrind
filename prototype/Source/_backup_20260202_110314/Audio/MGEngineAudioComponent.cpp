// Copyright Midnight Grind. All Rights Reserved.

#include "Audio/MGEngineAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

UMGEngineAudioComponent::UMGEngineAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	// Default preset (generic sport car)
	Preset.PresetName = FName("Default");
	Preset.IdleRPM = 800.0f;
	Preset.RedlineRPM = 7000.0f;
	Preset.LimiterRPM = 7200.0f;
	Preset.ExhaustPopProbability = 0.3f;
	Preset.ExhaustPopCooldown = 0.15f;
}

void UMGEngineAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentRPM = Preset.IdleRPM;
	TargetRPM = Preset.IdleRPM;

	InitializeAudioComponents();
}

void UMGEngineAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupAudioComponents();
	Super::EndPlay(EndPlayReason);
}

void UMGEngineAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAudioEnabled)
	{
		return;
	}

	// Smooth RPM changes
	CurrentRPM = FMath::FInterpTo(CurrentRPM, TargetRPM, DeltaTime, RPMSmoothingSpeed);

	// Clamp to valid range
	CurrentRPM = FMath::Clamp(CurrentRPM, 0.0f, Preset.LimiterRPM);

	// Check rev limiter
	bool bAtLimiter = CurrentRPM >= Preset.LimiterRPM - 50.0f;
	if (bAtLimiter && !bWasAtLimiter)
	{
		OnRevLimiter.Broadcast();
	}
	bWasAtLimiter = bAtLimiter;

	// Update audio
	UpdateLayers(DeltaTime);
	UpdateExhaustPops(DeltaTime);

	if (bUseProceduralFallback && LayerComponents.Num() == 0)
	{
		UpdateProceduralSound();
	}

	// Update damage audio (misfires)
	if (bIsMisfiring)
	{
		TimeSinceLastMisfire += DeltaTime;
		if (TimeSinceLastMisfire >= MisfireInterval)
		{
			TriggerMisfire();
			TimeSinceLastMisfire = 0.0f;
			// Randomize next misfire interval based on damage
			MisfireInterval = FMath::RandRange(0.1f, 0.5f) / FMath::Max(EngineDamageLevel, 0.1f);
		}
	}

	// Track throttle state
	bWasOnThrottle = IsOnThrottle();
}

// ==========================================
// ENGINE STATE INPUT
// ==========================================

void UMGEngineAudioComponent::SetRPM(float NewRPM)
{
	TargetRPM = FMath::Clamp(NewRPM, 0.0f, Preset.LimiterRPM);
}

void UMGEngineAudioComponent::SetThrottle(float NewThrottle)
{
	CurrentThrottle = FMath::Clamp(NewThrottle, 0.0f, 1.0f);
}

void UMGEngineAudioComponent::SetLoad(float NewLoad)
{
	CurrentLoad = FMath::Clamp(NewLoad, 0.0f, 1.0f);
}

void UMGEngineAudioComponent::SetGear(int32 NewGear)
{
	CurrentGear = FMath::Max(0, NewGear);
}

void UMGEngineAudioComponent::OnGearChange(int32 FromGear, int32 ToGear)
{
	CurrentGear = ToGear;

	// Play gear change sound if configured
	// Could trigger a one-shot gear whine/clunk here
}

void UMGEngineAudioComponent::SetEngineState(float RPM, float Throttle, float Load, int32 Gear)
{
	SetRPM(RPM);
	SetThrottle(Throttle);
	SetLoad(Load);
	SetGear(Gear);
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGEngineAudioComponent::SetPreset(const FMGEngineAudioPreset& NewPreset)
{
	Preset = NewPreset;

	// Reinitialize audio components with new layers
	CleanupAudioComponents();
	InitializeAudioComponents();
}

void UMGEngineAudioComponent::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
}

void UMGEngineAudioComponent::SetEnabled(bool bEnabled)
{
	bAudioEnabled = bEnabled;

	// Mute/unmute all components
	for (auto& Pair : LayerComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->SetVolumeMultiplier(bEnabled ? MasterVolume : 0.0f);
		}
	}

	if (ProceduralComponent)
	{
		ProceduralComponent->SetVolumeMultiplier(bEnabled ? MasterVolume : 0.0f);
	}
}

// ==========================================
// STATE QUERIES
// ==========================================

bool UMGEngineAudioComponent::IsAtRedline() const
{
	return CurrentRPM >= Preset.RedlineRPM;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGEngineAudioComponent::InitializeAudioComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Create audio components for each configured layer
	for (const FMGEngineSoundLayer& Layer : Preset.Layers)
	{
		if (Layer.Sound)
		{
			UAudioComponent* AudioComp = NewObject<UAudioComponent>(Owner);
			AudioComp->SetupAttachment(Owner->GetRootComponent());
			AudioComp->SetSound(Layer.Sound);
			AudioComp->bAutoActivate = true;
			AudioComp->bIsUISound = false;
			AudioComp->bAllowSpatialization = true;
			AudioComp->SetVolumeMultiplier(0.0f); // Start silent
			AudioComp->RegisterComponent();
			AudioComp->Play();

			LayerComponents.Add(Layer.LayerType, AudioComp);
		}
	}

	// If no layers configured and procedural fallback enabled, create procedural component
	if (LayerComponents.Num() == 0 && bUseProceduralFallback)
	{
		ProceduralComponent = NewObject<UAudioComponent>(Owner);
		ProceduralComponent->SetupAttachment(Owner->GetRootComponent());
		ProceduralComponent->bAutoActivate = true;
		ProceduralComponent->bIsUISound = false;
		ProceduralComponent->bAllowSpatialization = true;
		ProceduralComponent->RegisterComponent();

		UE_LOG(LogTemp, Log, TEXT("MGEngineAudio: Using procedural fallback (no sound assets configured)"));
	}
}

void UMGEngineAudioComponent::CleanupAudioComponents()
{
	for (auto& Pair : LayerComponents)
	{
		if (Pair.Value)
		{
			Pair.Value->Stop();
			Pair.Value->DestroyComponent();
		}
	}
	LayerComponents.Empty();

	if (ProceduralComponent)
	{
		ProceduralComponent->Stop();
		ProceduralComponent->DestroyComponent();
		ProceduralComponent = nullptr;
	}
}

void UMGEngineAudioComponent::UpdateLayers(float DeltaTime)
{
	float Pitch = CalculatePitch();

	for (const FMGEngineSoundLayer& Layer : Preset.Layers)
	{
		UAudioComponent** CompPtr = LayerComponents.Find(Layer.LayerType);
		if (!CompPtr || !*CompPtr)
		{
			continue;
		}

		UAudioComponent* Comp = *CompPtr;

		// Calculate layer volume
		float Volume = CalculateLayerVolume(Layer);

		// Apply master volume
		Volume *= MasterVolume;

		// Apply throttle response
		if (Layer.bThrottleResponse)
		{
			if (Layer.bOffThrottleOnly)
			{
				// Off-throttle layers (decel, pops)
				Volume *= (1.0f - CurrentThrottle);
			}
			else
			{
				// On-throttle layers
				float ThrottleFactor = FMath::Lerp(0.3f, 1.0f, CurrentThrottle);
				Volume *= ThrottleFactor;
			}
		}

		// Apply load response
		if (Preset.LoadToVolumeCurve)
		{
			Volume *= Preset.LoadToVolumeCurve->GetFloatValue(CurrentLoad);
		}

		// Set audio component parameters
		Comp->SetVolumeMultiplier(Volume);
		Comp->SetPitchMultiplier(Pitch * Layer.BasePitch);
	}

	// Update turbo sound if present
	if (Preset.bHasTurbo)
	{
		UAudioComponent** TurboComp = LayerComponents.Find(EMGEngineSoundLayer::Turbo);
		if (TurboComp && *TurboComp)
		{
			// Turbo spools with throttle and RPM
			float TurboVolume = CurrentThrottle * GetNormalizedRPM() * MasterVolume;
			(*TurboComp)->SetVolumeMultiplier(TurboVolume);

			// Turbo pitch increases with RPM
			float TurboPitch = FMath::Lerp(0.8f, 1.5f, GetNormalizedRPM());
			(*TurboComp)->SetPitchMultiplier(TurboPitch);
		}
	}

	// Update supercharger sound if present
	if (Preset.bHasSupercharger)
	{
		UAudioComponent** SCComp = LayerComponents.Find(EMGEngineSoundLayer::Supercharger);
		if (SCComp && *SCComp)
		{
			// Supercharger is directly tied to RPM
			float SCVolume = GetNormalizedRPM() * MasterVolume * 0.7f;
			(*SCComp)->SetVolumeMultiplier(SCVolume);

			// Pitch follows RPM closely
			float SCPitch = FMath::Lerp(0.5f, 2.0f, GetNormalizedRPM());
			(*SCComp)->SetPitchMultiplier(SCPitch);
		}
	}
}

float UMGEngineAudioComponent::CalculateLayerVolume(const FMGEngineSoundLayer& Layer) const
{
	// Check if RPM is in this layer's range
	if (CurrentRPM < Layer.RPMMin - Layer.FadeInRange || CurrentRPM > Layer.RPMMax + Layer.FadeOutRange)
	{
		return 0.0f;
	}

	float Volume = Layer.BaseVolume;

	// Fade in
	if (CurrentRPM < Layer.RPMMin)
	{
		float FadeProgress = (CurrentRPM - (Layer.RPMMin - Layer.FadeInRange)) / Layer.FadeInRange;
		Volume *= FMath::Clamp(FadeProgress, 0.0f, 1.0f);
	}
	// Fade out
	else if (CurrentRPM > Layer.RPMMax)
	{
		float FadeProgress = 1.0f - ((CurrentRPM - Layer.RPMMax) / Layer.FadeOutRange);
		Volume *= FMath::Clamp(FadeProgress, 0.0f, 1.0f);
	}

	// Apply RPM volume curve if available
	if (Preset.RPMToVolumeCurve)
	{
		Volume *= Preset.RPMToVolumeCurve->GetFloatValue(GetNormalizedRPM());
	}

	return Volume;
}

float UMGEngineAudioComponent::CalculatePitch() const
{
	float NormalizedRPM = GetNormalizedRPM();

	if (Preset.RPMToPitchCurve)
	{
		return Preset.RPMToPitchCurve->GetFloatValue(NormalizedRPM);
	}

	// Default linear pitch mapping
	// At idle: pitch ~0.5, at redline: pitch ~2.0
	return FMath::Lerp(0.5f, 2.0f, NormalizedRPM);
}

void UMGEngineAudioComponent::UpdateExhaustPops(float DeltaTime)
{
	TimeSinceLastPop += DeltaTime;

	// Only pop on deceleration (throttle released, RPM dropping)
	bool bDecel = !IsOnThrottle() && bWasOnThrottle && CurrentRPM > Preset.IdleRPM + 500.0f;

	if (bDecel && TimeSinceLastPop >= Preset.ExhaustPopCooldown)
	{
		// Random chance to pop
		if (FMath::FRand() < Preset.ExhaustPopProbability)
		{
			TimeSinceLastPop = 0.0f;
			OnExhaustPop.Broadcast();

			// Play pop sound if configured
			UAudioComponent** PopComp = LayerComponents.Find(EMGEngineSoundLayer::ExhaustPops);
			if (PopComp && *PopComp)
			{
				// Trigger one-shot pop
				(*PopComp)->SetVolumeMultiplier(MasterVolume * FMath::RandRange(0.7f, 1.0f));
				(*PopComp)->SetPitchMultiplier(FMath::RandRange(0.9f, 1.1f));
			}
		}
	}
}

void UMGEngineAudioComponent::UpdateProceduralSound()
{
	// Procedural synthesis placeholder
	// In a full implementation, this would generate a basic engine tone
	// using oscillators, filters, and modulation based on RPM
	//
	// For now, this is a stub that can be expanded or replaced with
	// actual MetaSound procedural audio

	if (!ProceduralComponent)
	{
		return;
	}

	// Calculate procedural parameters
	float NormalizedRPM = GetNormalizedRPM();
	float Volume = FMath::Lerp(0.3f, 1.0f, NormalizedRPM) * MasterVolume;
	float Pitch = FMath::Lerp(0.5f, 2.0f, NormalizedRPM);

	// Apply throttle
	if (IsOnThrottle())
	{
		Volume *= FMath::Lerp(0.5f, 1.0f, CurrentThrottle);
	}
	else
	{
		Volume *= 0.4f;
	}

	ProceduralComponent->SetVolumeMultiplier(Volume);
	ProceduralComponent->SetPitchMultiplier(Pitch);

	// Note: To properly implement procedural audio, you would:
	// 1. Create a USoundWaveProcedural subclass
	// 2. Generate samples based on RPM (sawtooth/square waves)
	// 3. Apply filters for engine character
	// 4. Mix multiple harmonics for realistic sound
	//
	// OR use MetaSounds which provides nodes for:
	// - Oscillators (Sine, Saw, Square)
	// - Filters (Lowpass, Highpass, Bandpass)
	// - Envelopes and modulation
	// - All controllable via Blueprint parameters
}

float UMGEngineAudioComponent::GetNormalizedRPM() const
{
	float Range = Preset.RedlineRPM - Preset.IdleRPM;
	if (Range <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp((CurrentRPM - Preset.IdleRPM) / Range, 0.0f, 1.0f);
}

// ==========================================
// AUDIO PARAMETER OUTPUT
// ==========================================

void UMGEngineAudioComponent::SetBoost(float NewBoost)
{
	// Detect BOV condition: throttle closed quickly while at high boost
	if (CurrentBoost > 0.5f && CurrentThrottle < 0.2f && PreviousThrottle > 0.7f)
	{
		TriggerBlowOffValve();
	}

	PreviousThrottle = CurrentThrottle;
	CurrentBoost = FMath::Clamp(NewBoost, 0.0f, 1.0f);
}

FMGEngineAudioParams UMGEngineAudioComponent::GetAudioParams() const
{
	FMGEngineAudioParams Params;

	Params.RPMNormalized = GetNormalizedRPM();
	Params.LoadNormalized = FMath::Clamp(CurrentLoad, 0.0f, 1.0f);
	Params.ThrottleNormalized = FMath::Clamp(CurrentThrottle, 0.0f, 1.0f);
	Params.BoostNormalized = FMath::Clamp(CurrentBoost, 0.0f, 1.0f);

	// Normalize gear (0 = reverse, 0.5 = neutral, 1 = max gear)
	if (MaxGears > 0)
	{
		Params.GearNormalized = FMath::Clamp((float)(CurrentGear + 1) / (float)(MaxGears + 1), 0.0f, 1.0f);
	}

	Params.bOnThrottle = CurrentThrottle > ThrottleOnThreshold;
	Params.bAtLimiter = CurrentRPM >= Preset.LimiterRPM - 50.0f;
	Params.bDecel = !Params.bOnThrottle && Params.RPMNormalized > 0.3f;

	return Params;
}

void UMGEngineAudioComponent::TriggerBackfire()
{
	bBackfireTriggered = true;
	OnExhaustPop.Broadcast();

	// Reset on next tick with weak reference for safety
	UWorld* World = GetWorld();
	if (World)
	{
		TWeakObjectPtr<UMGEngineAudioComponent> WeakThis(this);
		World->GetTimerManager().SetTimerForNextTick([WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->bBackfireTriggered = false;
			}
		});
	}
}

void UMGEngineAudioComponent::TriggerBlowOffValve()
{
	bBOVTriggered = true;

	// Reset on next tick with weak reference for safety
	UWorld* World = GetWorld();
	if (World)
	{
		TWeakObjectPtr<UMGEngineAudioComponent> WeakThis(this);
		World->GetTimerManager().SetTimerForNextTick([WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->bBOVTriggered = false;
			}
		});
	}
}

// ==========================================
// ENGINE DAMAGE AUDIO
// ==========================================

void UMGEngineAudioComponent::SetEngineDamageLevel(float DamageLevel)
{
	EngineDamageLevel = FMath::Clamp(DamageLevel, 0.0f, 1.0f);

	// Update misfire/knock state based on damage
	bIsMisfiring = EngineDamageLevel > 0.3f;
	bIsKnocking = EngineDamageLevel > 0.6f;

	// Adjust pitch/volume for damaged engine
	// More damage = rougher, lower pitch, unstable idle
	if (EngineDamageLevel > 0.1f)
	{
		// Initial misfire interval - more damage = more frequent misfires
		MisfireInterval = FMath::Lerp(2.0f, 0.2f, EngineDamageLevel);
	}
}

void UMGEngineAudioComponent::TriggerMisfire()
{
	// Briefly cut power/pitch to simulate misfire
	OnEngineMisfire.Broadcast();

	// Also trigger a backfire for audible effect at higher damage levels
	if (EngineDamageLevel > 0.5f && FMath::RandRange(0.0f, 1.0f) < EngineDamageLevel)
	{
		TriggerBackfire();
	}
}

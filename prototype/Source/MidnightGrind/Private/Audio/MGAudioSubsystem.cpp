// Copyright Midnight Grind. All Rights Reserved.

#include "Audio/MGAudioSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UMGAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default duck amounts
	DuckAmounts.Add(EMGSoundCategory::Master, 0.0f);
	DuckAmounts.Add(EMGSoundCategory::Music, 0.0f);
	DuckAmounts.Add(EMGSoundCategory::SFX, 0.0f);
	DuckAmounts.Add(EMGSoundCategory::Engine, 0.0f);
	DuckAmounts.Add(EMGSoundCategory::Environment, 0.0f);
	DuckAmounts.Add(EMGSoundCategory::UI, 0.0f);
	DuckAmounts.Add(EMGSoundCategory::Voice, 0.0f);

	UE_LOG(LogTemp, Log, TEXT("MGAudioSubsystem initialized"));
}

void UMGAudioSubsystem::Deinitialize()
{
	// Stop all active sounds
	for (auto& Pair : ActiveComponents)
	{
		for (auto& WeakComp : Pair.Value)
		{
			if (UAudioComponent* Comp = WeakComp.Get())
			{
				Comp->Stop();
			}
		}
	}
	ActiveComponents.Empty();

	Super::Deinitialize();
}

// ==========================================
// VOLUME CONTROL
// ==========================================

void UMGAudioSubsystem::SetCategoryVolume(EMGSoundCategory Category, float Volume)
{
	Volume = FMath::Clamp(Volume, 0.0f, 1.0f);

	switch (Category)
	{
	case EMGSoundCategory::Master:
		AudioSettings.MasterVolume = Volume;
		// Apply to all categories
		for (int32 i = 0; i < static_cast<int32>(EMGSoundCategory::Voice) + 1; ++i)
		{
			ApplyCategoryVolume(static_cast<EMGSoundCategory>(i));
		}
		break;
	case EMGSoundCategory::Music:
		AudioSettings.MusicVolume = Volume;
		break;
	case EMGSoundCategory::SFX:
		AudioSettings.SFXVolume = Volume;
		break;
	case EMGSoundCategory::Engine:
		AudioSettings.EngineVolume = Volume;
		break;
	case EMGSoundCategory::Environment:
		AudioSettings.EnvironmentVolume = Volume;
		break;
	case EMGSoundCategory::UI:
		AudioSettings.UIVolume = Volume;
		break;
	case EMGSoundCategory::Voice:
		AudioSettings.VoiceVolume = Volume;
		break;
	}

	ApplyCategoryVolume(Category);
}

float UMGAudioSubsystem::GetCategoryVolume(EMGSoundCategory Category) const
{
	switch (Category)
	{
	case EMGSoundCategory::Master:
		return AudioSettings.MasterVolume;
	case EMGSoundCategory::Music:
		return AudioSettings.MusicVolume;
	case EMGSoundCategory::SFX:
		return AudioSettings.SFXVolume;
	case EMGSoundCategory::Engine:
		return AudioSettings.EngineVolume;
	case EMGSoundCategory::Environment:
		return AudioSettings.EnvironmentVolume;
	case EMGSoundCategory::UI:
		return AudioSettings.UIVolume;
	case EMGSoundCategory::Voice:
		return AudioSettings.VoiceVolume;
	default:
		return 1.0f;
	}
}

float UMGAudioSubsystem::GetEffectiveVolume(EMGSoundCategory Category) const
{
	float CategoryVol = GetCategoryVolume(Category);
	float MasterVol = AudioSettings.MasterVolume;

	// Apply duck amount
	const float* DuckAmount = DuckAmounts.Find(Category);
	float Duck = DuckAmount ? *DuckAmount : 0.0f;

	return CategoryVol * MasterVol * (1.0f - Duck);
}

void UMGAudioSubsystem::SetAudioSettings(const FMGAudioSettings& Settings)
{
	AudioSettings = Settings;

	// Apply to all categories
	for (int32 i = 0; i < static_cast<int32>(EMGSoundCategory::Voice) + 1; ++i)
	{
		ApplyCategoryVolume(static_cast<EMGSoundCategory>(i));
	}
}

// ==========================================
// SOUND PLAYBACK
// ==========================================

UAudioComponent* UMGAudioSubsystem::PlaySound2D(UObject* WorldContextObject, USoundBase* Sound,
	EMGSoundCategory Category, float VolumeMultiplier, float PitchMultiplier)
{
	if (!Sound || !WorldContextObject)
	{
		return nullptr;
	}

	float EffectiveVol = GetEffectiveVolume(Category) * VolumeMultiplier;
	float EffectivePitch = PitchMultiplier * GlobalPitch;

	UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(
		WorldContextObject, Sound, EffectiveVol, EffectivePitch);

	if (AudioComp)
	{
		RegisterActiveComponent(Category, AudioComp);
	}

	return AudioComp;
}

UAudioComponent* UMGAudioSubsystem::PlaySoundAtLocation(UObject* WorldContextObject, USoundBase* Sound,
	FVector Location, EMGSoundCategory Category, float VolumeMultiplier, float PitchMultiplier)
{
	if (!Sound || !WorldContextObject)
	{
		return nullptr;
	}

	float EffectiveVol = GetEffectiveVolume(Category) * VolumeMultiplier;
	float EffectivePitch = PitchMultiplier * GlobalPitch;

	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
		WorldContextObject, Sound, Location, FRotator::ZeroRotator,
		EffectiveVol, EffectivePitch);

	if (AudioComp)
	{
		RegisterActiveComponent(Category, AudioComp);
	}

	return AudioComp;
}

UAudioComponent* UMGAudioSubsystem::PlaySoundRequest(UObject* WorldContextObject, const FMGSoundRequest& Request)
{
	if (Request.bIs3D)
	{
		return PlaySoundAtLocation(WorldContextObject, Request.Sound, Request.Location,
			Request.Category, Request.VolumeMultiplier, Request.PitchMultiplier);
	}
	else
	{
		return PlaySound2D(WorldContextObject, Request.Sound, Request.Category,
			Request.VolumeMultiplier, Request.PitchMultiplier);
	}
}

void UMGAudioSubsystem::StopCategorySounds(EMGSoundCategory Category)
{
	TArray<TWeakObjectPtr<UAudioComponent>>* Components = ActiveComponents.Find(Category);
	if (Components)
	{
		for (auto& WeakComp : *Components)
		{
			if (UAudioComponent* Comp = WeakComp.Get())
			{
				Comp->Stop();
			}
		}
		Components->Empty();
	}
}

// ==========================================
// AUDIO DUCKING
// ==========================================

void UMGAudioSubsystem::DuckCategory(EMGSoundCategory Category, float DuckAmount, float FadeTime)
{
	DuckAmount = FMath::Clamp(DuckAmount, 0.0f, 1.0f);
	DuckAmounts.Add(Category, DuckAmount);

	// Apply with fade (simplified - immediate for now)
	ApplyCategoryVolume(Category);
}

void UMGAudioSubsystem::RestoreCategory(EMGSoundCategory Category, float FadeTime)
{
	DuckAmounts.Add(Category, 0.0f);
	ApplyCategoryVolume(Category);
}

void UMGAudioSubsystem::DuckAllExcept(EMGSoundCategory Exception, float DuckAmount, float FadeTime)
{
	for (int32 i = 0; i < static_cast<int32>(EMGSoundCategory::Voice) + 1; ++i)
	{
		EMGSoundCategory Cat = static_cast<EMGSoundCategory>(i);
		if (Cat != Exception && Cat != EMGSoundCategory::Master)
		{
			DuckCategory(Cat, DuckAmount, FadeTime);
		}
	}
}

void UMGAudioSubsystem::RestoreAllDucking(float FadeTime)
{
	for (int32 i = 0; i < static_cast<int32>(EMGSoundCategory::Voice) + 1; ++i)
	{
		RestoreCategory(static_cast<EMGSoundCategory>(i), FadeTime);
	}
}

// ==========================================
// UTILITY
// ==========================================

void UMGAudioSubsystem::PauseAllAudio()
{
	bAudioPaused = true;

	for (auto& Pair : ActiveComponents)
	{
		for (auto& WeakComp : Pair.Value)
		{
			if (UAudioComponent* Comp = WeakComp.Get())
			{
				Comp->SetPaused(true);
			}
		}
	}
}

void UMGAudioSubsystem::ResumeAllAudio()
{
	bAudioPaused = false;

	for (auto& Pair : ActiveComponents)
	{
		for (auto& WeakComp : Pair.Value)
		{
			if (UAudioComponent* Comp = WeakComp.Get())
			{
				Comp->SetPaused(false);
			}
		}
	}
}

void UMGAudioSubsystem::SetGlobalPitch(float Pitch)
{
	GlobalPitch = FMath::Clamp(Pitch, 0.1f, 3.0f);

	// Apply to all active components
	for (auto& Pair : ActiveComponents)
	{
		for (auto& WeakComp : Pair.Value)
		{
			if (UAudioComponent* Comp = WeakComp.Get())
			{
				Comp->SetPitchMultiplier(GlobalPitch);
			}
		}
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGAudioSubsystem::ApplyCategoryVolume(EMGSoundCategory Category)
{
	float EffectiveVol = GetEffectiveVolume(Category);

	TArray<TWeakObjectPtr<UAudioComponent>>* Components = ActiveComponents.Find(Category);
	if (Components)
	{
		for (auto& WeakComp : *Components)
		{
			if (UAudioComponent* Comp = WeakComp.Get())
			{
				Comp->SetVolumeMultiplier(EffectiveVol);
			}
		}
	}
}

void UMGAudioSubsystem::RegisterActiveComponent(EMGSoundCategory Category, UAudioComponent* Component)
{
	if (!Component)
	{
		return;
	}

	// Clean up finished components first
	CleanupFinishedComponents();

	TArray<TWeakObjectPtr<UAudioComponent>>& Components = ActiveComponents.FindOrAdd(Category);
	Components.Add(Component);
}

void UMGAudioSubsystem::CleanupFinishedComponents()
{
	for (auto& Pair : ActiveComponents)
	{
		Pair.Value.RemoveAll([](const TWeakObjectPtr<UAudioComponent>& WeakComp)
		{
			UAudioComponent* Comp = WeakComp.Get();
			return !Comp || !Comp->IsPlaying();
		});
	}
}

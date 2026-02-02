// Copyright Midnight Grind. All Rights Reserved.

#include "DynamicMix/MGDynamicMixSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGDynamicMixSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultBusSettings();
	InitializeDefaultSnapshots();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MixTickHandle,
			this,
			&UMGDynamicMixSubsystem::OnMixTick,
			0.016f, // ~60Hz update
			true
		);
	}
}

void UMGDynamicMixSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MixTickHandle);
	}
	Super::Deinitialize();
}

bool UMGDynamicMixSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// State Management
void UMGDynamicMixSubsystem::SetAudioState(EMGAudioState NewState, float TransitionTime)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EMGAudioState OldState = CurrentState;
	CurrentState = NewState;

	// Transition to appropriate snapshot
	FName SnapshotName;
	switch (NewState)
	{
	case EMGAudioState::Idle:
		SnapshotName = FName("Idle");
		break;
	case EMGAudioState::Cruising:
		SnapshotName = FName("Cruising");
		break;
	case EMGAudioState::Racing:
		SnapshotName = FName("Racing");
		break;
	case EMGAudioState::Intense:
		SnapshotName = FName("Intense");
		break;
	case EMGAudioState::PoliceChase:
		SnapshotName = FName("PoliceChase");
		break;
	case EMGAudioState::PhotoMode:
		SnapshotName = FName("PhotoMode");
		break;
	case EMGAudioState::Cutscene:
		SnapshotName = FName("Cutscene");
		break;
	case EMGAudioState::Menu:
		SnapshotName = FName("Menu");
		break;
	default:
		SnapshotName = FName("Default");
		break;
	}

	if (Snapshots.Contains(SnapshotName))
	{
		TransitionToSnapshot(SnapshotName, TransitionTime);
	}

	OnAudioStateChanged.Broadcast(OldState, NewState);
}

void UMGDynamicMixSubsystem::PushAudioState(EMGAudioState State, float TransitionTime)
{
	StateStack.Push(CurrentState);
	SetAudioState(State, TransitionTime);
}

void UMGDynamicMixSubsystem::PopAudioState(float TransitionTime)
{
	if (StateStack.Num() > 0)
	{
		EMGAudioState PreviousState = StateStack.Pop();
		SetAudioState(PreviousState, TransitionTime);
	}
}

// Bus Control
void UMGDynamicMixSubsystem::SetBusVolume(EMGAudioBusType Bus, float Volume, float FadeTime)
{
	FMGAudioBusSettings* Settings = BusSettingsMap.Find(Bus);
	if (Settings)
	{
		Settings->Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
		ApplyBusSettings(Bus, *Settings);
	}
}

float UMGDynamicMixSubsystem::GetBusVolume(EMGAudioBusType Bus) const
{
	const FMGAudioBusSettings* Settings = BusSettingsMap.Find(Bus);
	return Settings ? Settings->Volume : 1.0f;
}

void UMGDynamicMixSubsystem::SetBusMuted(EMGAudioBusType Bus, bool bMuted)
{
	FMGAudioBusSettings* Settings = BusSettingsMap.Find(Bus);
	if (Settings)
	{
		Settings->bMuted = bMuted;
		ApplyBusSettings(Bus, *Settings);
	}
}

bool UMGDynamicMixSubsystem::IsBusMuted(EMGAudioBusType Bus) const
{
	const FMGAudioBusSettings* Settings = BusSettingsMap.Find(Bus);
	return Settings ? Settings->bMuted : false;
}

void UMGDynamicMixSubsystem::SetBusSettings(EMGAudioBusType Bus, const FMGAudioBusSettings& Settings, float TransitionTime)
{
	BusSettingsMap.Add(Bus, Settings);
	ApplyBusSettings(Bus, Settings);
}

FMGAudioBusSettings UMGDynamicMixSubsystem::GetBusSettings(EMGAudioBusType Bus) const
{
	const FMGAudioBusSettings* Settings = BusSettingsMap.Find(Bus);
	return Settings ? *Settings : FMGAudioBusSettings();
}

void UMGDynamicMixSubsystem::ApplyLowPassFilter(EMGAudioBusType Bus, float Frequency, float TransitionTime)
{
	FMGAudioBusSettings* Settings = BusSettingsMap.Find(Bus);
	if (Settings)
	{
		Settings->LowPassFrequency = FMath::Clamp(Frequency, 20.0f, 20000.0f);
		ApplyBusSettings(Bus, *Settings);
	}
}

void UMGDynamicMixSubsystem::ClearLowPassFilter(EMGAudioBusType Bus, float TransitionTime)
{
	ApplyLowPassFilter(Bus, 20000.0f, TransitionTime);
}

// Snapshots
void UMGDynamicMixSubsystem::TransitionToSnapshot(FName SnapshotName, float TransitionTime)
{
	const FMGAudioMixSnapshot* Snapshot = Snapshots.Find(SnapshotName);
	if (!Snapshot)
	{
		return;
	}

	FName OldName = CurrentSnapshotName;
	OnSnapshotTransitionStarted.Broadcast(OldName, SnapshotName);

	TargetSnapshot = *Snapshot;
	TransitionDuration = TransitionTime;
	TransitionProgress = 0.0f;
	bIsTransitioning = true;

	if (TransitionTime <= 0.0f)
	{
		CurrentSnapshot = TargetSnapshot;
		CurrentSnapshotName = SnapshotName;
		bIsTransitioning = false;
		TransitionProgress = 1.0f;

		for (const auto& Pair : CurrentSnapshot.BusSettings)
		{
			BusSettingsMap.Add(Pair.Key, Pair.Value);
			ApplyBusSettings(Pair.Key, Pair.Value);
		}

		OnSnapshotTransitionComplete.Broadcast(SnapshotName);
	}
}

void UMGDynamicMixSubsystem::RegisterSnapshot(const FMGAudioMixSnapshot& Snapshot)
{
	Snapshots.Add(Snapshot.SnapshotName, Snapshot);
}

// Intensity
void UMGDynamicMixSubsystem::UpdateIntensityParams(const FMGAudioIntensityParams& Params)
{
	IntensityParams = Params;
}

void UMGDynamicMixSubsystem::SetIntensityOverride(float Intensity, float Duration)
{
	bIntensityOverride = true;
	IntensityOverrideValue = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGDynamicMixSubsystem::ClearIntensityOverride()
{
	bIntensityOverride = false;
}

// Ducking
void UMGDynamicMixSubsystem::AddDuckingRule(const FMGAudioDuckingRule& Rule)
{
	DuckingRules.Add(Rule);
}

void UMGDynamicMixSubsystem::RemoveDuckingRule(EMGAudioBusType SourceBus, EMGAudioBusType TargetBus)
{
	DuckingRules.RemoveAll([SourceBus, TargetBus](const FMGAudioDuckingRule& R)
	{
		return R.SourceBus == SourceBus && R.TargetBus == TargetBus;
	});
}

void UMGDynamicMixSubsystem::SetDuckingEnabled(bool bEnabled)
{
	bDuckingEnabled = bEnabled;
}

// Effects
void UMGDynamicMixSubsystem::ApplyEffectPreset(const FMGAudioEffectPreset& Preset, float TransitionTime)
{
	// Would apply to audio engine
}

void UMGDynamicMixSubsystem::SetReverbSettings(float DecayTime, float WetLevel, float TransitionTime)
{
	// Would apply reverb settings
}

void UMGDynamicMixSubsystem::SetGlobalPitch(float Pitch, float TransitionTime)
{
	GlobalPitch = FMath::Clamp(Pitch, 0.1f, 2.0f);
}

// Audio Zones
void UMGDynamicMixSubsystem::RegisterAudioZone(const FMGAudioZone& Zone)
{
	AudioZones.Add(Zone);
}

void UMGDynamicMixSubsystem::UnregisterAudioZone(FName ZoneID)
{
	AudioZones.RemoveAll([ZoneID](const FMGAudioZone& Z)
	{
		return Z.ZoneID == ZoneID;
	});
}

void UMGDynamicMixSubsystem::UpdateListenerPosition(FVector Position)
{
	ListenerPosition = Position;
}

FMGAudioZone UMGDynamicMixSubsystem::GetCurrentAudioZone() const
{
	const FMGAudioZone* Zone = AudioZones.FindByPredicate([this](const FMGAudioZone& Z)
	{
		return Z.ZoneID == CurrentZoneID;
	});

	return Zone ? *Zone : FMGAudioZone();
}

// Slow Motion
void UMGDynamicMixSubsystem::SetSlowMotionAudio(float TimeScale, float TransitionTime)
{
	float TargetPitch = FMath::Clamp(TimeScale, 0.1f, 1.0f);
	SetGlobalPitch(TargetPitch, TransitionTime);

	// Apply low-pass filter proportional to time scale
	float FilterFreq = FMath::Lerp(2000.0f, 20000.0f, TimeScale);
	ApplyLowPassFilter(EMGAudioBusType::SFX, FilterFreq, TransitionTime);
	ApplyLowPassFilter(EMGAudioBusType::Engine, FilterFreq, TransitionTime);
}

void UMGDynamicMixSubsystem::ResetSlowMotionAudio(float TransitionTime)
{
	SetGlobalPitch(1.0f, TransitionTime);
	ClearLowPassFilter(EMGAudioBusType::SFX, TransitionTime);
	ClearLowPassFilter(EMGAudioBusType::Engine, TransitionTime);
}

// Master Control
void UMGDynamicMixSubsystem::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMGDynamicMixSubsystem::MuteAll(bool bMute)
{
	bAllMuted = bMute;
}

void UMGDynamicMixSubsystem::PauseAllAudio(bool bPause)
{
	bAllPaused = bPause;
}

// Internal
void UMGDynamicMixSubsystem::OnMixTick()
{
	float MGDeltaTime = 0.016f;

	if (bIsTransitioning)
	{
		UpdateTransition(DeltaTime);
	}

	UpdateIntensity();
	UpdateDucking();
	UpdateAudioZones();
}

void UMGDynamicMixSubsystem::UpdateTransition(float MGDeltaTime)
{
	if (!bIsTransitioning)
	{
		return;
	}

	TransitionProgress += DeltaTime / TransitionDuration;

	if (TransitionProgress >= 1.0f)
	{
		TransitionProgress = 1.0f;
		bIsTransitioning = false;
		CurrentSnapshot = TargetSnapshot;
		CurrentSnapshotName = TargetSnapshot.SnapshotName;

		for (const auto& Pair : CurrentSnapshot.BusSettings)
		{
			BusSettingsMap.Add(Pair.Key, Pair.Value);
			ApplyBusSettings(Pair.Key, Pair.Value);
		}

		OnSnapshotTransitionComplete.Broadcast(CurrentSnapshotName);
	}
	else
	{
		// Interpolate bus settings
		for (const auto& Pair : TargetSnapshot.BusSettings)
		{
			EMGAudioBusType Bus = Pair.Key;
			const FMGAudioBusSettings& Target = Pair.Value;

			FMGAudioBusSettings* Current = BusSettingsMap.Find(Bus);
			if (Current)
			{
				FMGAudioBusSettings Interpolated;
				Interpolated.BusType = Bus;
				Interpolated.Volume = FMath::Lerp(Current->Volume, Target.Volume, TransitionProgress);
				Interpolated.Pitch = FMath::Lerp(Current->Pitch, Target.Pitch, TransitionProgress);
				Interpolated.LowPassFrequency = FMath::Lerp(Current->LowPassFrequency, Target.LowPassFrequency, TransitionProgress);
				Interpolated.HighPassFrequency = FMath::Lerp(Current->HighPassFrequency, Target.HighPassFrequency, TransitionProgress);
				Interpolated.ReverbSend = FMath::Lerp(Current->ReverbSend, Target.ReverbSend, TransitionProgress);

				ApplyBusSettings(Bus, Interpolated);
			}
		}
	}
}

void UMGDynamicMixSubsystem::UpdateIntensity()
{
	float NewIntensity;

	if (bIntensityOverride)
	{
		NewIntensity = IntensityOverrideValue;
	}
	else
	{
		NewIntensity = CalculateIntensity(IntensityParams);
	}

	if (!FMath::IsNearlyEqual(NewIntensity, CurrentIntensity, 0.01f))
	{
		CurrentIntensity = FMath::FInterpTo(CurrentIntensity, NewIntensity, 0.016f, 5.0f);
		OnIntensityChanged.Broadcast(CurrentIntensity);
	}
}

void UMGDynamicMixSubsystem::UpdateDucking()
{
	if (!bDuckingEnabled)
	{
		return;
	}

	// Would check audio levels and apply ducking
}

void UMGDynamicMixSubsystem::UpdateAudioZones()
{
	FName NewZoneID = NAME_None;
	float ClosestDist = FLT_MAX;

	for (const FMGAudioZone& Zone : AudioZones)
	{
		float Dist = FVector::Dist(ListenerPosition, Zone.Center);
		if (Dist < Zone.Radius && Dist < ClosestDist)
		{
			ClosestDist = Dist;
			NewZoneID = Zone.ZoneID;
		}
	}

	if (NewZoneID != CurrentZoneID)
	{
		if (CurrentZoneID != NAME_None)
		{
			const FMGAudioZone* OldZone = AudioZones.FindByPredicate([this](const FMGAudioZone& Z)
			{
				return Z.ZoneID == CurrentZoneID;
			});
			if (OldZone)
			{
				OnAudioZoneExited.Broadcast(*OldZone);
			}
		}

		CurrentZoneID = NewZoneID;

		if (CurrentZoneID != NAME_None)
		{
			const FMGAudioZone* NewZone = AudioZones.FindByPredicate([this](const FMGAudioZone& Z)
			{
				return Z.ZoneID == CurrentZoneID;
			});
			if (NewZone)
			{
				OnAudioZoneEntered.Broadcast(*NewZone);
				TransitionToSnapshot(NewZone->ZoneSnapshot.SnapshotName, 1.0f);
			}
		}
	}
}

void UMGDynamicMixSubsystem::ApplyBusSettings(EMGAudioBusType Bus, const FMGAudioBusSettings& Settings)
{
	// Would apply to actual audio engine (submix, sound class, etc.)
}

float UMGDynamicMixSubsystem::CalculateIntensity(const FMGAudioIntensityParams& Params) const
{
	float Intensity = 0.0f;

	// Speed contribution (0-0.3)
	float SpeedFactor = FMath::Clamp(Params.CurrentSpeed / Params.MaxSpeed, 0.0f, 1.0f);
	Intensity += SpeedFactor * 0.3f;

	// Position contribution (0-0.2)
	if (Params.TotalRacers > 1)
	{
		float PositionFactor = 1.0f - (static_cast<float>(Params.RacePosition - 1) / (Params.TotalRacers - 1));
		Intensity += PositionFactor * 0.2f;
	}

	// Close racing contribution (0-0.15)
	if (Params.GapToAhead < 2.0f && Params.GapToAhead > 0.0f)
	{
		Intensity += 0.15f * (1.0f - Params.GapToAhead / 2.0f);
	}

	// Police contribution (0-0.2)
	Intensity += FMath::Clamp(Params.PoliceHeatLevel / 5.0f, 0.0f, 1.0f) * 0.2f;

	// Special states contribution (0-0.15)
	if (Params.bInNitro) Intensity += 0.05f;
	if (Params.bDrifting) Intensity += 0.05f;
	if (Params.bNearMiss) Intensity += 0.05f;

	// Final lap boost
	if (Params.bFinalLap)
	{
		Intensity *= 1.2f;
	}

	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGDynamicMixSubsystem::InitializeDefaultSnapshots()
{
	// Idle snapshot
	{
		FMGAudioMixSnapshot Snapshot;
		Snapshot.SnapshotName = FName("Idle");
		Snapshot.TransitionTime = 2.0f;

		FMGAudioBusSettings Music;
		Music.BusType = EMGAudioBusType::Music;
		Music.Volume = 0.8f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Music, Music);

		FMGAudioBusSettings Ambience;
		Ambience.BusType = EMGAudioBusType::Ambience;
		Ambience.Volume = 1.0f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Ambience, Ambience);

		Snapshots.Add(Snapshot.SnapshotName, Snapshot);
	}

	// Racing snapshot
	{
		FMGAudioMixSnapshot Snapshot;
		Snapshot.SnapshotName = FName("Racing");
		Snapshot.TransitionTime = 0.5f;

		FMGAudioBusSettings Music;
		Music.BusType = EMGAudioBusType::Music;
		Music.Volume = 0.6f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Music, Music);

		FMGAudioBusSettings Engine;
		Engine.BusType = EMGAudioBusType::Engine;
		Engine.Volume = 1.0f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Engine, Engine);

		Snapshots.Add(Snapshot.SnapshotName, Snapshot);
	}

	// Police chase snapshot
	{
		FMGAudioMixSnapshot Snapshot;
		Snapshot.SnapshotName = FName("PoliceChase");
		Snapshot.TransitionTime = 0.3f;

		FMGAudioBusSettings Music;
		Music.BusType = EMGAudioBusType::Music;
		Music.Volume = 0.4f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Music, Music);

		FMGAudioBusSettings Police;
		Police.BusType = EMGAudioBusType::Police;
		Police.Volume = 1.0f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Police, Police);

		FMGAudioBusSettings Engine;
		Engine.BusType = EMGAudioBusType::Engine;
		Engine.Volume = 1.0f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Engine, Engine);

		Snapshots.Add(Snapshot.SnapshotName, Snapshot);
	}

	// Photo mode snapshot
	{
		FMGAudioMixSnapshot Snapshot;
		Snapshot.SnapshotName = FName("PhotoMode");
		Snapshot.TransitionTime = 0.5f;

		FMGAudioBusSettings Music;
		Music.BusType = EMGAudioBusType::Music;
		Music.Volume = 0.3f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Music, Music);

		FMGAudioBusSettings Ambience;
		Ambience.BusType = EMGAudioBusType::Ambience;
		Ambience.Volume = 0.5f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Ambience, Ambience);

		FMGAudioBusSettings Engine;
		Engine.BusType = EMGAudioBusType::Engine;
		Engine.Volume = 0.0f;
		Snapshot.BusSettings.Add(EMGAudioBusType::Engine, Engine);

		Snapshots.Add(Snapshot.SnapshotName, Snapshot);
	}
}

void UMGDynamicMixSubsystem::InitializeDefaultBusSettings()
{
	TArray<EMGAudioBusType> AllBuses = {
		EMGAudioBusType::Master,
		EMGAudioBusType::Music,
		EMGAudioBusType::Ambience,
		EMGAudioBusType::SFX,
		EMGAudioBusType::Vehicle,
		EMGAudioBusType::Engine,
		EMGAudioBusType::UI,
		EMGAudioBusType::Voice,
		EMGAudioBusType::Crowd,
		EMGAudioBusType::Weather,
		EMGAudioBusType::Police,
		EMGAudioBusType::Cinematics
	};

	for (EMGAudioBusType Bus : AllBuses)
	{
		FMGAudioBusSettings Settings;
		Settings.BusType = Bus;
		Settings.Volume = 1.0f;
		Settings.Pitch = 1.0f;
		BusSettingsMap.Add(Bus, Settings);
	}
}

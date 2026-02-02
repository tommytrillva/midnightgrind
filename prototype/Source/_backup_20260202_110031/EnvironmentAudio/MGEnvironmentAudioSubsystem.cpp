// Copyright Midnight Grind. All Rights Reserved.

#include "EnvironmentAudio/MGEnvironmentAudioSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGEnvironmentAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultSoundscapes();

	AudioState.MasterVolume = 1.0f;
	AudioState.TimeOfDay = EMGTimeOfDayAudio::Afternoon;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			EnvironmentTickHandle,
			this,
			&UMGEnvironmentAudioSubsystem::OnEnvironmentTick,
			0.05f,
			true
		);
	}
}

void UMGEnvironmentAudioSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EnvironmentTickHandle);
	}
	Super::Deinitialize();
}

bool UMGEnvironmentAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// Zone Management
void UMGEnvironmentAudioSubsystem::RegisterEnvironmentZone(const FMGEnvironmentZone& Zone)
{
	EnvironmentZones.Add(Zone.ZoneID, Zone);
}

void UMGEnvironmentAudioSubsystem::UnregisterEnvironmentZone(FName ZoneID)
{
	EnvironmentZones.Remove(ZoneID);
}

FMGEnvironmentZone UMGEnvironmentAudioSubsystem::GetEnvironmentZone(FName ZoneID) const
{
	const FMGEnvironmentZone* Zone = EnvironmentZones.Find(ZoneID);
	return Zone ? *Zone : FMGEnvironmentZone();
}

TArray<FMGEnvironmentZone> UMGEnvironmentAudioSubsystem::GetAllZones() const
{
	TArray<FMGEnvironmentZone> Result;
	EnvironmentZones.GenerateValueArray(Result);
	return Result;
}

// Soundscape Management
void UMGEnvironmentAudioSubsystem::RegisterSoundscape(const FMGEnvironmentSoundscape& Soundscape)
{
	Soundscapes.Add(Soundscape.SoundscapeID, Soundscape);
}

void UMGEnvironmentAudioSubsystem::TransitionToSoundscape(FName SoundscapeID, float TransitionTime)
{
	const FMGEnvironmentSoundscape* NewSoundscape = Soundscapes.Find(SoundscapeID);
	if (!NewSoundscape)
	{
		return;
	}

	// Fade out current layers
	for (const FMGAmbientSoundLayer& Layer : CurrentSoundscape.Layers)
	{
		TargetLayerVolumes.Add(Layer.LayerID, 0.0f);
	}

	// Set up new layers
	CurrentSoundscape = *NewSoundscape;

	for (const FMGAmbientSoundLayer& Layer : CurrentSoundscape.Layers)
	{
		LayerVolumes.Add(Layer.LayerID, 0.0f);
		TargetLayerVolumes.Add(Layer.LayerID, Layer.Volume);
	}
}

void UMGEnvironmentAudioSubsystem::SetLayerVolume(FName LayerID, float Volume, float FadeTime)
{
	TargetLayerVolumes.Add(LayerID, FMath::Clamp(Volume, 0.0f, 1.0f));
}

void UMGEnvironmentAudioSubsystem::MuteLayer(FName LayerID, bool bMute, float FadeTime)
{
	if (bMute)
	{
		TargetLayerVolumes.Add(LayerID, 0.0f);
	}
	else
	{
		// Restore to default
		for (const FMGAmbientSoundLayer& Layer : CurrentSoundscape.Layers)
		{
			if (Layer.LayerID == LayerID)
			{
				TargetLayerVolumes.Add(LayerID, Layer.Volume);
				break;
			}
		}
	}
}

// One-Shots
void UMGEnvironmentAudioSubsystem::RegisterOneShot(const FMGOneShot& OneShot)
{
	OneShots.Add(OneShot);
}

void UMGEnvironmentAudioSubsystem::PlayOneShot(FName OneShotID, FVector Location)
{
	const FMGOneShot* OneShot = OneShots.FindByPredicate([OneShotID](const FMGOneShot& O)
	{
		return O.OneShotID == OneShotID;
	});

	if (OneShot)
	{
		PlayOneShotAtLocation(*OneShot, Location);
	}
}

void UMGEnvironmentAudioSubsystem::PlayRandomOneShot(EMGEnvironmentType Environment)
{
	TArray<const FMGOneShot*> ValidOneShots;

	for (const FMGOneShot& OneShot : OneShots)
	{
		if (OneShot.ValidEnvironments.Contains(Environment) || OneShot.ValidEnvironments.Num() == 0)
		{
			if (OneShot.ValidTimes.Contains(AudioState.TimeOfDay) || OneShot.ValidTimes.Num() == 0)
			{
				ValidOneShots.Add(&OneShot);
			}
		}
	}

	if (ValidOneShots.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, ValidOneShots.Num() - 1);
		const FMGOneShot* Selected = ValidOneShots[Index];

		float Distance = FMath::RandRange(Selected->MinDistance, Selected->MaxDistance);
		float Angle = FMath::RandRange(0.0f, 360.0f);
		FVector Offset = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
			FMath::RandRange(-50.0f, 100.0f)
		);

		PlayOneShotAtLocation(*Selected, ListenerLocation + Offset);
	}
}

void UMGEnvironmentAudioSubsystem::SetOneShotEnabled(bool bEnabled)
{
	bOneShotsEnabled = bEnabled;
}

// State Updates
void UMGEnvironmentAudioSubsystem::UpdateListenerLocation(FVector Location)
{
	ListenerLocation = Location;
}

void UMGEnvironmentAudioSubsystem::SetPlayerSpeed(float Speed)
{
	AudioState.CurrentSpeed = Speed;
}

void UMGEnvironmentAudioSubsystem::SetTimeOfDay(EMGTimeOfDayAudio Time)
{
	if (AudioState.TimeOfDay != Time)
	{
		EMGTimeOfDayAudio OldTime = AudioState.TimeOfDay;
		AudioState.TimeOfDay = Time;
		OnTimeOfDayChanged.Broadcast(OldTime, Time);
	}
}

void UMGEnvironmentAudioSubsystem::SetInsideVehicle(bool bInside)
{
	AudioState.bIsInsideVehicle = bInside;
}

// Weather Integration
void UMGEnvironmentAudioSubsystem::SetRainIntensity(float Intensity)
{
	float OldIntensity = AudioState.RainIntensity;
	AudioState.RainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	AudioState.bIsRaining = Intensity > 0.0f;

	if (!FMath::IsNearlyEqual(OldIntensity, AudioState.RainIntensity, 0.05f))
	{
		OnWeatherAudioChanged.Broadcast(AudioState.RainIntensity);
	}
}

void UMGEnvironmentAudioSubsystem::SetWindIntensity(float Intensity)
{
	AudioState.WindIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGEnvironmentAudioSubsystem::TriggerThunder(float Distance, float Intensity)
{
	// Would play thunder sound with delay based on distance
	float Delay = Distance / 343.0f; // Speed of sound
	// Schedule thunder sound after delay
}

// Wind Audio
void UMGEnvironmentAudioSubsystem::UpdateWindAudio(float Speed, FVector Direction)
{
	// Calculate wind audio based on vehicle speed and direction
	float WindVolume = FMath::Clamp(Speed / 200.0f, 0.0f, 1.0f);
	AudioState.WindIntensity = WindVolume;
}

// Volume Control
void UMGEnvironmentAudioSubsystem::SetMasterVolume(float Volume)
{
	AudioState.MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMGEnvironmentAudioSubsystem::FadeOutAll(float FadeTime)
{
	for (auto& Pair : TargetLayerVolumes)
	{
		Pair.Value = 0.0f;
	}
}

void UMGEnvironmentAudioSubsystem::FadeInAll(float FadeTime)
{
	for (const FMGAmbientSoundLayer& Layer : CurrentSoundscape.Layers)
	{
		TargetLayerVolumes.Add(Layer.LayerID, Layer.Volume);
	}
}

// Internal
void UMGEnvironmentAudioSubsystem::OnEnvironmentTick()
{
	float DeltaTime = 0.05f;

	UpdateZoneTransitions();
	UpdateLayerVolumes();
	UpdateSpeedBasedAudio();
	UpdateWeatherAudio();
	ProcessOneShots();
}

void UMGEnvironmentAudioSubsystem::UpdateZoneTransitions()
{
	FMGEnvironmentZone NewZone = FindZoneAtLocation(ListenerLocation);

	if (NewZone.ZoneID != AudioState.CurrentZoneID)
	{
		FName OldZoneID = AudioState.CurrentZoneID;
		AudioState.CurrentZoneID = NewZone.ZoneID;
		AudioState.CurrentEnvironment = NewZone.Soundscape.EnvironmentType;

		if (NewZone.ZoneID != NAME_None)
		{
			TransitionToSoundscape(NewZone.Soundscape.SoundscapeID, NewZone.Soundscape.TransitionTime);
		}

		OnEnvironmentZoneChanged.Broadcast(OldZoneID, NewZone.ZoneID);
	}
}

void UMGEnvironmentAudioSubsystem::UpdateLayerVolumes()
{
	float DeltaTime = 0.05f;

	for (auto& Pair : LayerVolumes)
	{
		float* Target = TargetLayerVolumes.Find(Pair.Key);
		if (Target)
		{
			Pair.Value = FMath::FInterpTo(Pair.Value, *Target, DeltaTime, 2.0f);
		}
	}
}

void UMGEnvironmentAudioSubsystem::ProcessOneShots()
{
	if (!bOneShotsEnabled)
	{
		return;
	}

	float DeltaTime = 0.05f;
	OneShotTimer += DeltaTime;

	if (OneShotTimer >= NextOneShotTime)
	{
		OneShotTimer = 0.0f;
		NextOneShotTime = FMath::RandRange(5.0f, 20.0f);

		PlayRandomOneShot(AudioState.CurrentEnvironment);
	}
}

void UMGEnvironmentAudioSubsystem::UpdateWeatherAudio()
{
	// Adjust layer volumes based on weather
	if (AudioState.bIsRaining)
	{
		// Boost rain layer if it exists
		SetLayerVolume(FName("Rain"), AudioState.RainIntensity, 0.5f);

		// Reduce other ambient sounds slightly
		float ReductionFactor = 1.0f - (AudioState.RainIntensity * 0.3f);
		for (const FMGAmbientSoundLayer& Layer : CurrentSoundscape.Layers)
		{
			if (Layer.LayerType != EMGAmbientLayerType::Weather)
			{
				float* CurrentTarget = TargetLayerVolumes.Find(Layer.LayerID);
				if (CurrentTarget)
				{
					*CurrentTarget = Layer.Volume * ReductionFactor;
				}
			}
		}
	}
}

void UMGEnvironmentAudioSubsystem::UpdateSpeedBasedAudio()
{
	for (FMGAmbientSoundLayer& Layer : CurrentSoundscape.Layers)
	{
		if (Layer.bAffectedBySpeed)
		{
			float SpeedFactor = FMath::Clamp(AudioState.CurrentSpeed / 150.0f, 0.0f, 1.0f);
			float AdjustedVolume = Layer.Volume * (1.0f - SpeedFactor * Layer.SpeedVolumeMultiplier);
			TargetLayerVolumes.Add(Layer.LayerID, AdjustedVolume);
		}
	}
}

FMGEnvironmentZone UMGEnvironmentAudioSubsystem::FindZoneAtLocation(FVector Location) const
{
	FMGEnvironmentZone BestZone;
	int32 HighestPriority = -1;

	for (const auto& Pair : EnvironmentZones)
	{
		const FMGEnvironmentZone& Zone = Pair.Value;

		bool bInside = false;
		if (Zone.bUseBoxShape)
		{
			FBox ZoneBox(Zone.Center - Zone.Extent, Zone.Center + Zone.Extent);
			bInside = ZoneBox.IsInside(Location);
		}
		else
		{
			float Dist = FVector::Dist(Location, Zone.Center);
			bInside = Dist <= Zone.SphereRadius;
		}

		if (bInside && Zone.Soundscape.Priority > HighestPriority)
		{
			HighestPriority = Zone.Soundscape.Priority;
			BestZone = Zone;
		}
	}

	return BestZone;
}

void UMGEnvironmentAudioSubsystem::InitializeDefaultSoundscapes()
{
	// Urban soundscape
	{
		FMGEnvironmentSoundscape S;
		S.SoundscapeID = FName("Urban");
		S.EnvironmentType = EMGEnvironmentType::Urban;
		S.BaseVolume = 1.0f;
		S.TransitionTime = 3.0f;

		FMGAmbientSoundLayer Base;
		Base.LayerID = FName("Urban_Base");
		Base.LayerType = EMGAmbientLayerType::Base;
		Base.Volume = 0.8f;
		S.Layers.Add(Base);

		FMGAmbientSoundLayer Traffic;
		Traffic.LayerID = FName("Urban_Traffic");
		Traffic.LayerType = EMGAmbientLayerType::Traffic;
		Traffic.Volume = 0.6f;
		Traffic.bAffectedBySpeed = true;
		Traffic.SpeedVolumeMultiplier = 0.5f;
		S.Layers.Add(Traffic);

		Soundscapes.Add(S.SoundscapeID, S);
	}

	// Industrial soundscape
	{
		FMGEnvironmentSoundscape S;
		S.SoundscapeID = FName("Industrial");
		S.EnvironmentType = EMGEnvironmentType::Industrial;
		S.BaseVolume = 1.0f;

		FMGAmbientSoundLayer Base;
		Base.LayerID = FName("Industrial_Base");
		Base.LayerType = EMGAmbientLayerType::Industrial;
		Base.Volume = 0.9f;
		S.Layers.Add(Base);

		Soundscapes.Add(S.SoundscapeID, S);
	}

	// Highway soundscape
	{
		FMGEnvironmentSoundscape S;
		S.SoundscapeID = FName("Highway");
		S.EnvironmentType = EMGEnvironmentType::Highway;
		S.BaseVolume = 0.7f;

		FMGAmbientSoundLayer Wind;
		Wind.LayerID = FName("Highway_Wind");
		Wind.LayerType = EMGAmbientLayerType::Base;
		Wind.Volume = 0.8f;
		Wind.bAffectedBySpeed = true;
		Wind.SpeedVolumeMultiplier = -0.3f;
		S.Layers.Add(Wind);

		Soundscapes.Add(S.SoundscapeID, S);
	}

	// Register default one-shots
	{
		FMGOneShot Siren;
		Siren.OneShotID = FName("DistantSiren");
		Siren.MinInterval = 30.0f;
		Siren.MaxInterval = 120.0f;
		Siren.MinDistance = 200.0f;
		Siren.MaxDistance = 800.0f;
		Siren.ValidEnvironments.Add(EMGEnvironmentType::Urban);
		Siren.ValidEnvironments.Add(EMGEnvironmentType::Downtown);
		OneShots.Add(Siren);

		FMGOneShot Honk;
		Honk.OneShotID = FName("CarHorn");
		Honk.MinInterval = 10.0f;
		Honk.MaxInterval = 45.0f;
		Honk.MinDistance = 50.0f;
		Honk.MaxDistance = 300.0f;
		Honk.ValidEnvironments.Add(EMGEnvironmentType::Urban);
		Honk.ValidEnvironments.Add(EMGEnvironmentType::Downtown);
		Honk.ValidEnvironments.Add(EMGEnvironmentType::Commercial);
		OneShots.Add(Honk);

		FMGOneShot Dog;
		Dog.OneShotID = FName("DogBark");
		Dog.MinInterval = 20.0f;
		Dog.MaxInterval = 90.0f;
		Dog.MinDistance = 100.0f;
		Dog.MaxDistance = 400.0f;
		Dog.ValidEnvironments.Add(EMGEnvironmentType::Residential);
		Dog.ValidEnvironments.Add(EMGEnvironmentType::Suburbs);
		OneShots.Add(Dog);
	}
}

void UMGEnvironmentAudioSubsystem::PlayOneShotAtLocation(const FMGOneShot& OneShot, FVector Location)
{
	float Volume = FMath::RandRange(OneShot.VolumeMin, OneShot.VolumeMax);
	float Pitch = FMath::RandRange(OneShot.PitchMin, OneShot.PitchMax);

	// Would play sound at location with volume and pitch
	OnOneShotPlayed.Broadcast(OneShot.OneShotID);
}

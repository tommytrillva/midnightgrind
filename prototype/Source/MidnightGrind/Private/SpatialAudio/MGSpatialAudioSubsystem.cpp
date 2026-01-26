// Copyright Midnight Grind. All Rights Reserved.

#include "SpatialAudio/MGSpatialAudioSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGSpatialAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeEnvironmentPresets();

	Settings.bOcclusionEnabled = true;
	Settings.bReflectionsEnabled = true;
	Settings.bDopplerEnabled = true;
	Settings.DopplerScale = 1.0f;
	Settings.SpeedOfSound = 34300.0f;
	Settings.MaxReflections = 4;
	Settings.OcclusionUpdateRate = 0.1f;
	Settings.MaxActiveSources = 32;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SpatialTickHandle,
			this,
			&UMGSpatialAudioSubsystem::OnSpatialTick,
			0.016f,
			true
		);
	}
}

void UMGSpatialAudioSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpatialTickHandle);
	}
	Super::Deinitialize();
}

bool UMGSpatialAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// Zone Management
void UMGSpatialAudioSubsystem::RegisterAcousticZone(const FMGAcousticZone& Zone)
{
	AcousticZones.Add(Zone.ZoneID, Zone);
}

void UMGSpatialAudioSubsystem::UnregisterAcousticZone(FName ZoneID)
{
	AcousticZones.Remove(ZoneID);
}

void UMGSpatialAudioSubsystem::UpdateAcousticZone(const FMGAcousticZone& Zone)
{
	if (AcousticZones.Contains(Zone.ZoneID))
	{
		AcousticZones[Zone.ZoneID] = Zone;
	}
}

FMGAcousticZone UMGSpatialAudioSubsystem::GetAcousticZone(FName ZoneID) const
{
	const FMGAcousticZone* Zone = AcousticZones.Find(ZoneID);
	return Zone ? *Zone : FMGAcousticZone();
}

TArray<FMGAcousticZone> UMGSpatialAudioSubsystem::GetAllAcousticZones() const
{
	TArray<FMGAcousticZone> Result;
	AcousticZones.GenerateValueArray(Result);
	return Result;
}

FMGAcousticZone UMGSpatialAudioSubsystem::GetZoneAtLocation(FVector Location) const
{
	FMGAcousticZone BestZone;
	int32 HighestPriority = -1;

	for (const auto& Pair : AcousticZones)
	{
		const FMGAcousticZone& Zone = Pair.Value;

		FBox ZoneBox(Zone.Center - Zone.Extent, Zone.Center + Zone.Extent);
		if (ZoneBox.IsInside(Location))
		{
			if (Zone.Priority > HighestPriority)
			{
				HighestPriority = Zone.Priority;
				BestZone = Zone;
			}
		}
	}

	return BestZone;
}

// Sound Source Management
void UMGSpatialAudioSubsystem::RegisterSoundSource(const FMGSpatialSoundSource& Source)
{
	SoundSources.Add(Source.SourceID, Source);
}

void UMGSpatialAudioSubsystem::UnregisterSoundSource(FName SourceID)
{
	SoundSources.Remove(SourceID);
	SourceReflections.Remove(SourceID);
}

void UMGSpatialAudioSubsystem::UpdateSoundSource(FName SourceID, FVector Location, FVector Velocity)
{
	FMGSpatialSoundSource* Source = SoundSources.Find(SourceID);
	if (Source)
	{
		Source->Location = Location;
		Source->Velocity = Velocity;
	}
}

void UMGSpatialAudioSubsystem::SetSourceVolume(FName SourceID, float Volume)
{
	FMGSpatialSoundSource* Source = SoundSources.Find(SourceID);
	if (Source)
	{
		Source->Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
	}
}

void UMGSpatialAudioSubsystem::SetSourceActive(FName SourceID, bool bActive)
{
	FMGSpatialSoundSource* Source = SoundSources.Find(SourceID);
	if (Source)
	{
		Source->bIsActive = bActive;
	}
}

FMGSpatialSoundSource UMGSpatialAudioSubsystem::GetSoundSource(FName SourceID) const
{
	const FMGSpatialSoundSource* Source = SoundSources.Find(SourceID);
	return Source ? *Source : FMGSpatialSoundSource();
}

TArray<FName> UMGSpatialAudioSubsystem::GetActiveSoundSources() const
{
	TArray<FName> Result;

	for (const auto& Pair : SoundSources)
	{
		if (Pair.Value.bIsActive)
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

// Listener
void UMGSpatialAudioSubsystem::UpdateListener(FVector Location, FRotator Rotation, FVector Velocity)
{
	ListenerState.Location = Location;
	ListenerState.Rotation = Rotation;
	ListenerState.Velocity = Velocity;
	ListenerState.Speed = Velocity.Size();
}

void UMGSpatialAudioSubsystem::SetListenerInsideVehicle(bool bInside)
{
	ListenerState.bInsideVehicle = bInside;
}

// Occlusion
void UMGSpatialAudioSubsystem::SetOcclusionEnabled(bool bEnabled)
{
	Settings.bOcclusionEnabled = bEnabled;
}

float UMGSpatialAudioSubsystem::GetOcclusionForSource(FName SourceID) const
{
	const FMGSpatialSoundSource* Source = SoundSources.Find(SourceID);
	return Source ? Source->CurrentOcclusion : 0.0f;
}

void UMGSpatialAudioSubsystem::ForceOcclusionUpdate()
{
	UpdateOcclusion();
}

// Doppler
void UMGSpatialAudioSubsystem::SetDopplerEnabled(bool bEnabled)
{
	Settings.bDopplerEnabled = bEnabled;
}

void UMGSpatialAudioSubsystem::SetDopplerScale(float Scale)
{
	Settings.DopplerScale = FMath::Max(0.0f, Scale);
}

float UMGSpatialAudioSubsystem::CalculateDopplerPitch(FName SourceID) const
{
	if (!Settings.bDopplerEnabled)
	{
		return 1.0f;
	}

	const FMGSpatialSoundSource* Source = SoundSources.Find(SourceID);
	if (!Source || !Source->bDopplerEnabled)
	{
		return 1.0f;
	}

	FVector ToListener = ListenerState.Location - Source->Location;
	float Distance = ToListener.Size();

	if (Distance < 1.0f)
	{
		return 1.0f;
	}

	FVector Direction = ToListener / Distance;

	float SourceApproachSpeed = FVector::DotProduct(Source->Velocity, Direction);
	float ListenerApproachSpeed = -FVector::DotProduct(ListenerState.Velocity, Direction);

	float RelativeSpeed = SourceApproachSpeed + ListenerApproachSpeed;
	float DopplerPitch = (Settings.SpeedOfSound + ListenerApproachSpeed) /
						  (Settings.SpeedOfSound - SourceApproachSpeed);

	DopplerPitch = FMath::Lerp(1.0f, DopplerPitch, Settings.DopplerScale * Source->DopplerFactor);
	return FMath::Clamp(DopplerPitch, 0.5f, 2.0f);
}

// Reflections
void UMGSpatialAudioSubsystem::SetReflectionsEnabled(bool bEnabled)
{
	Settings.bReflectionsEnabled = bEnabled;
}

TArray<FMGAudioReflection> UMGSpatialAudioSubsystem::GetReflectionsForSource(FName SourceID) const
{
	const TArray<FMGAudioReflection>* Reflections = SourceReflections.Find(SourceID);
	return Reflections ? *Reflections : TArray<FMGAudioReflection>();
}

// Environment Presets
void UMGSpatialAudioSubsystem::ApplyEnvironmentPreset(EMGAcousticEnvironment Environment, float TransitionTime)
{
	const FMGAcousticZone* Preset = EnvironmentPresets.Find(Environment);
	if (Preset)
	{
		ApplyZoneEffects(*Preset);

		EMGAcousticEnvironment OldEnv = ListenerState.CurrentEnvironment;
		ListenerState.CurrentEnvironment = Environment;

		if (OldEnv != Environment)
		{
			OnEnvironmentChanged.Broadcast(OldEnv, Environment);
		}
	}
}

FMGAcousticZone UMGSpatialAudioSubsystem::GetEnvironmentPreset(EMGAcousticEnvironment Environment) const
{
	const FMGAcousticZone* Preset = EnvironmentPresets.Find(Environment);
	return Preset ? *Preset : FMGAcousticZone();
}

// Distance Attenuation
float UMGSpatialAudioSubsystem::CalculateAttenuation(FName SourceID) const
{
	const FMGSpatialSoundSource* Source = SoundSources.Find(SourceID);
	if (!Source)
	{
		return 0.0f;
	}

	float Distance = FVector::Dist(ListenerState.Location, Source->Location);

	if (Distance <= Source->MinDistance)
	{
		return 1.0f;
	}

	if (Distance >= Source->MaxDistance)
	{
		return 0.0f;
	}

	float NormalizedDistance = (Distance - Source->MinDistance) / (Source->MaxDistance - Source->MinDistance);
	float Attenuation = 1.0f - FMath::Pow(NormalizedDistance, 2.0f);

	return FMath::Clamp(Attenuation * Settings.AttenuationScale, 0.0f, 1.0f);
}

void UMGSpatialAudioSubsystem::SetAttenuationScale(float Scale)
{
	Settings.AttenuationScale = FMath::Max(0.0f, Scale);
}

// Settings
void UMGSpatialAudioSubsystem::UpdateSettings(const FMGSpatialAudioSettings& NewSettings)
{
	Settings = NewSettings;
}

// Internal
void UMGSpatialAudioSubsystem::OnSpatialTick()
{
	float DeltaTime = 0.016f;

	UpdateZoneTransitions();

	OcclusionUpdateTimer += DeltaTime;
	if (OcclusionUpdateTimer >= Settings.OcclusionUpdateRate)
	{
		UpdateOcclusion();
		OcclusionUpdateTimer = 0.0f;
	}

	if (Settings.bReflectionsEnabled)
	{
		for (auto& Pair : SoundSources)
		{
			if (Pair.Value.bIsActive)
			{
				CalculateReflections(Pair.Value);
			}
		}
	}
}

void UMGSpatialAudioSubsystem::UpdateOcclusion()
{
	if (!Settings.bOcclusionEnabled)
	{
		return;
	}

	for (auto& Pair : SoundSources)
	{
		FMGSpatialSoundSource& Source = Pair.Value;

		if (!Source.bIsActive || Source.OcclusionType == EMGOcclusionType::None)
		{
			Source.CurrentOcclusion = 0.0f;
			continue;
		}

		float OldOcclusion = Source.CurrentOcclusion;
		Source.CurrentOcclusion = CalculateOcclusionForSource(Source);

		if (!FMath::IsNearlyEqual(OldOcclusion, Source.CurrentOcclusion, 0.05f))
		{
			OnSoundOccluded.Broadcast(Source.SourceID, Source.CurrentOcclusion);
		}
	}
}

void UMGSpatialAudioSubsystem::UpdateZoneTransitions()
{
	FMGAcousticZone CurrentZone = GetZoneAtLocation(ListenerState.Location);

	if (CurrentZone.ZoneID != ListenerState.CurrentZoneID)
	{
		FName OldZoneID = ListenerState.CurrentZoneID;
		ListenerState.CurrentZoneID = CurrentZone.ZoneID;

		if (CurrentZone.ZoneID != NAME_None)
		{
			ApplyZoneEffects(CurrentZone);

			if (ListenerState.CurrentEnvironment != CurrentZone.Environment)
			{
				EMGAcousticEnvironment OldEnv = ListenerState.CurrentEnvironment;
				ListenerState.CurrentEnvironment = CurrentZone.Environment;
				OnEnvironmentChanged.Broadcast(OldEnv, CurrentZone.Environment);
			}
		}

		OnAcousticZoneChanged.Broadcast(OldZoneID, CurrentZone.ZoneID);
	}
}

void UMGSpatialAudioSubsystem::ApplyZoneEffects(const FMGAcousticZone& Zone)
{
	// Would apply reverb, low-pass filter, etc. to audio engine
}

void UMGSpatialAudioSubsystem::CalculateReflections(FMGSpatialSoundSource& Source)
{
	TArray<FMGAudioReflection> Reflections;

	// Simplified ray-based reflection calculation
	// Would do proper ray tracing in full implementation

	FVector ToListener = ListenerState.Location - Source.Location;
	float Distance = ToListener.Size();

	// Floor reflection
	{
		FMGAudioReflection R;
		R.ReflectionPoint = FVector(Source.Location.X, Source.Location.Y, 0.0f);
		R.Normal = FVector::UpVector;
		R.Distance = Source.Location.Z + ListenerState.Location.Z;
		R.Delay = R.Distance / Settings.SpeedOfSound;
		R.Intensity = 0.3f / FMath::Max(1.0f, R.Distance / 500.0f);
		Reflections.Add(R);
	}

	// Wall reflections (simplified)
	FMGAcousticZone Zone = GetZoneAtLocation(Source.Location);
	if (Zone.ZoneID != NAME_None)
	{
		// Would calculate actual wall reflections based on zone geometry
		if (Zone.EchoDelay > 0.0f)
		{
			FMGAudioReflection R;
			R.Delay = Zone.EchoDelay;
			R.Intensity = 0.2f;
			Reflections.Add(R);
		}
	}

	// Limit reflections
	if (Reflections.Num() > Settings.MaxReflections)
	{
		Reflections.Sort([](const FMGAudioReflection& A, const FMGAudioReflection& B)
		{
			return A.Intensity > B.Intensity;
		});
		Reflections.SetNum(Settings.MaxReflections);
	}

	SourceReflections.Add(Source.SourceID, Reflections);
}

float UMGSpatialAudioSubsystem::CalculateOcclusionForSource(const FMGSpatialSoundSource& Source) const
{
	if (Source.OcclusionType == EMGOcclusionType::None)
	{
		return 0.0f;
	}

	if (Source.OcclusionType == EMGOcclusionType::Full)
	{
		return 1.0f;
	}

	if (Source.OcclusionType == EMGOcclusionType::Partial)
	{
		return 0.5f;
	}

	// Dynamic occlusion - would do line traces in full implementation
	FVector Direction = ListenerState.Location - Source.Location;
	float Distance = Direction.Size();

	// Check if inside vehicle
	if (ListenerState.bInsideVehicle && Source.Priority != EMGSoundPriority::Player)
	{
		return 0.3f;
	}

	// Check zone occlusion
	FMGAcousticZone SourceZone = GetZoneAtLocation(Source.Location);
	FMGAcousticZone ListenerZone = GetZoneAtLocation(ListenerState.Location);

	if (SourceZone.ZoneID != ListenerZone.ZoneID)
	{
		return FMath::Max(SourceZone.OcclusionFactor, ListenerZone.OcclusionFactor);
	}

	return 0.0f;
}

void UMGSpatialAudioSubsystem::SortSourcesByPriority()
{
	// Would sort and cull sources based on priority and distance
}

void UMGSpatialAudioSubsystem::InitializeEnvironmentPresets()
{
	// Outdoor
	{
		FMGAcousticZone Z;
		Z.Environment = EMGAcousticEnvironment::Outdoor;
		Z.ReverbDecay = 0.5f;
		Z.ReverbWetLevel = 0.1f;
		Z.LowPassFrequency = 20000.0f;
		Z.EchoDelay = 0.0f;
		EnvironmentPresets.Add(EMGAcousticEnvironment::Outdoor, Z);
	}

	// Tunnel
	{
		FMGAcousticZone Z;
		Z.Environment = EMGAcousticEnvironment::Tunnel;
		Z.ReverbDecay = 3.0f;
		Z.ReverbWetLevel = 0.6f;
		Z.LowPassFrequency = 8000.0f;
		Z.EchoDelay = 0.15f;
		Z.OcclusionFactor = 0.3f;
		EnvironmentPresets.Add(EMGAcousticEnvironment::Tunnel, Z);
	}

	// Garage
	{
		FMGAcousticZone Z;
		Z.Environment = EMGAcousticEnvironment::Garage;
		Z.ReverbDecay = 2.0f;
		Z.ReverbWetLevel = 0.5f;
		Z.LowPassFrequency = 6000.0f;
		Z.EchoDelay = 0.08f;
		Z.OcclusionFactor = 0.5f;
		EnvironmentPresets.Add(EMGAcousticEnvironment::Garage, Z);
	}

	// Underpass
	{
		FMGAcousticZone Z;
		Z.Environment = EMGAcousticEnvironment::Underpass;
		Z.ReverbDecay = 2.5f;
		Z.ReverbWetLevel = 0.4f;
		Z.LowPassFrequency = 10000.0f;
		Z.EchoDelay = 0.1f;
		EnvironmentPresets.Add(EMGAcousticEnvironment::Underpass, Z);
	}

	// Downtown
	{
		FMGAcousticZone Z;
		Z.Environment = EMGAcousticEnvironment::Downtown;
		Z.ReverbDecay = 1.5f;
		Z.ReverbWetLevel = 0.3f;
		Z.LowPassFrequency = 15000.0f;
		Z.EchoDelay = 0.05f;
		EnvironmentPresets.Add(EMGAcousticEnvironment::Downtown, Z);
	}

	// Highway
	{
		FMGAcousticZone Z;
		Z.Environment = EMGAcousticEnvironment::Highway;
		Z.ReverbDecay = 0.8f;
		Z.ReverbWetLevel = 0.15f;
		Z.LowPassFrequency = 18000.0f;
		Z.EchoDelay = 0.0f;
		EnvironmentPresets.Add(EMGAcousticEnvironment::Highway, Z);
	}

	// Industrial
	{
		FMGAcousticZone Z;
		Z.Environment = EMGAcousticEnvironment::Industrial;
		Z.ReverbDecay = 2.0f;
		Z.ReverbWetLevel = 0.35f;
		Z.LowPassFrequency = 12000.0f;
		Z.EchoDelay = 0.12f;
		EnvironmentPresets.Add(EMGAcousticEnvironment::Industrial, Z);
	}
}

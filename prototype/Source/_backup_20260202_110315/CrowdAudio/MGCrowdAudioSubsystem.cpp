// Copyright Midnight Grind. All Rights Reserved.

#include "CrowdAudio/MGCrowdAudioSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGCrowdAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultReactions();

	CrowdState.CurrentMood = EMGCrowdMood::Calm;
	CrowdState.ExcitementLevel = 0.0f;
	CrowdState.TensionLevel = 0.0f;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CrowdTickHandle,
			this,
			&UMGCrowdAudioSubsystem::OnCrowdTick,
			0.05f,
			true
		);
	}
}

void UMGCrowdAudioSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CrowdTickHandle);
	}
	Super::Deinitialize();
}

bool UMGCrowdAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// Zone Management
void UMGCrowdAudioSubsystem::RegisterCrowdZone(const FMGCrowdZone& Zone)
{
	CrowdZones.Add(Zone);
}

void UMGCrowdAudioSubsystem::UnregisterCrowdZone(FName ZoneID)
{
	CrowdZones.RemoveAll([ZoneID](const FMGCrowdZone& Z)
	{
		return Z.ZoneID == ZoneID;
	});
}

TArray<FMGCrowdZone> UMGCrowdAudioSubsystem::GetAllCrowdZones() const
{
	return CrowdZones;
}

FMGCrowdZone UMGCrowdAudioSubsystem::GetCrowdZone(FName ZoneID) const
{
	const FMGCrowdZone* Zone = CrowdZones.FindByPredicate([ZoneID](const FMGCrowdZone& Z)
	{
		return Z.ZoneID == ZoneID;
	});

	return Zone ? *Zone : FMGCrowdZone();
}

FMGCrowdZone UMGCrowdAudioSubsystem::GetNearestCrowdZone(FVector Location) const
{
	FMGCrowdZone Nearest;
	float ClosestDist = FLT_MAX;

	for (const FMGCrowdZone& Zone : CrowdZones)
	{
		float Dist = FVector::Dist(Location, Zone.Location);
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			Nearest = Zone;
		}
	}

	return Nearest;
}

void UMGCrowdAudioSubsystem::SetZoneDensity(FName ZoneID, int32 Density)
{
	FMGCrowdZone* Zone = CrowdZones.FindByPredicate([ZoneID](const FMGCrowdZone& Z)
	{
		return Z.ZoneID == ZoneID;
	});

	if (Zone)
	{
		Zone->CrowdDensity = FMath::Max(0, Density);
	}
}

// Event Triggers
void UMGCrowdAudioSubsystem::TriggerCrowdEvent(EMGCrowdEventType Event, FVector EventLocation, float Intensity)
{
	// Check cooldown
	float* LastTrigger = EventCooldowns.Find(Event);
	if (LastTrigger && CrowdState.TimeSinceLastReaction < *LastTrigger)
	{
		return;
	}

	FMGCrowdReaction Reaction = GetReactionForEvent(Event);
	Reaction.Intensity = Intensity;

	// Update excitement based on event
	float ExcitementBoost = Intensity * 0.2f;
	CrowdState.ExcitementLevel = FMath::Clamp(CrowdState.ExcitementLevel + ExcitementBoost, 0.0f, 1.0f);

	// Trigger reaction in nearby zones
	for (const FMGCrowdZone& Zone : CrowdZones)
	{
		float Dist = FVector::Dist(EventLocation, Zone.Location);
		if (Dist <= Reaction.MaxDistance)
		{
			float DistanceFactor = 1.0f - (Dist / Reaction.MaxDistance);
			PlayReactionSound(Reaction, Zone.Location);
		}
	}

	// Set cooldown
	EventCooldowns.Add(Event, Reaction.CooldownTime);
	CrowdState.TimeSinceLastReaction = 0.0f;

	// Update mood
	EMGCrowdMood OldMood = CrowdState.CurrentMood;
	CrowdState.CurrentMood = Reaction.ResultingMood;

	if (OldMood != CrowdState.CurrentMood)
	{
		OnCrowdMoodChanged.Broadcast(OldMood, CrowdState.CurrentMood);
	}

	OnCrowdReaction.Broadcast(Event, Intensity);
	OnExcitementChanged.Broadcast(CrowdState.ExcitementLevel);

	// Trigger wave effect
	if (WaveSettings.bEnabled && Intensity >= WaveSettings.MinIntensityForWave)
	{
		ProcessCrowdWave(EventLocation, Intensity);
	}
}

void UMGCrowdAudioSubsystem::TriggerGlobalCrowdEvent(EMGCrowdEventType Event, float Intensity)
{
	FMGCrowdReaction Reaction = GetReactionForEvent(Event);
	Reaction.Intensity = Intensity;
	Reaction.bAffectsAllZones = true;

	// Update all zones
	for (const FMGCrowdZone& Zone : CrowdZones)
	{
		PlayReactionSound(Reaction, Zone.Location);
	}

	// Update global excitement
	float ExcitementBoost = Intensity * 0.3f;
	CrowdState.ExcitementLevel = FMath::Clamp(CrowdState.ExcitementLevel + ExcitementBoost, 0.0f, 1.0f);

	EMGCrowdMood OldMood = CrowdState.CurrentMood;
	CrowdState.CurrentMood = Reaction.ResultingMood;

	if (OldMood != CrowdState.CurrentMood)
	{
		OnCrowdMoodChanged.Broadcast(OldMood, CrowdState.CurrentMood);
	}

	OnCrowdReaction.Broadcast(Event, Intensity);
	OnExcitementChanged.Broadcast(CrowdState.ExcitementLevel);
}

void UMGCrowdAudioSubsystem::TriggerZoneCrowdEvent(FName ZoneID, EMGCrowdEventType Event, float Intensity)
{
	FMGCrowdZone Zone = GetCrowdZone(ZoneID);
	if (Zone.ZoneID != NAME_None)
	{
		TriggerCrowdEvent(Event, Zone.Location, Intensity);
	}
}

// Mood Control
void UMGCrowdAudioSubsystem::SetGlobalMood(EMGCrowdMood Mood, float TransitionTime)
{
	EMGCrowdMood OldMood = CrowdState.CurrentMood;
	CrowdState.CurrentMood = Mood;

	if (OldMood != Mood)
	{
		OnCrowdMoodChanged.Broadcast(OldMood, Mood);
	}
}

void UMGCrowdAudioSubsystem::SetExcitementLevel(float Level)
{
	float OldLevel = CrowdState.ExcitementLevel;
	CrowdState.ExcitementLevel = FMath::Clamp(Level, 0.0f, 1.0f);

	if (!FMath::IsNearlyEqual(OldLevel, CrowdState.ExcitementLevel, 0.01f))
	{
		OnExcitementChanged.Broadcast(CrowdState.ExcitementLevel);
	}
}

void UMGCrowdAudioSubsystem::SetTensionLevel(float Level)
{
	CrowdState.TensionLevel = FMath::Clamp(Level, 0.0f, 1.0f);
}

// Race State Integration
void UMGCrowdAudioSubsystem::OnRaceStarted()
{
	TriggerGlobalCrowdEvent(EMGCrowdEventType::RaceStart, 1.0f);
	SetGlobalMood(EMGCrowdMood::Excited, 0.5f);
	SetExcitementLevel(0.7f);
}

void UMGCrowdAudioSubsystem::OnRaceFinished(bool bPlayerWon)
{
	if (bPlayerWon)
	{
		TriggerGlobalCrowdEvent(EMGCrowdEventType::PlayerWin, 1.0f);
		SetGlobalMood(EMGCrowdMood::Celebrating, 0.5f);
	}
	else
	{
		TriggerGlobalCrowdEvent(EMGCrowdEventType::RaceFinish, 0.7f);
		SetGlobalMood(EMGCrowdMood::Cheering, 0.5f);
	}
	SetExcitementLevel(1.0f);
}

void UMGCrowdAudioSubsystem::OnFinalLapStarted()
{
	TriggerGlobalCrowdEvent(EMGCrowdEventType::FinalLap, 0.9f);
	SetGlobalMood(EMGCrowdMood::Tense, 1.0f);
	SetTensionLevel(0.8f);
}

void UMGCrowdAudioSubsystem::OnLeadChange(const FString& NewLeaderID)
{
	TriggerGlobalCrowdEvent(EMGCrowdEventType::LeadChange, 0.8f);
}

void UMGCrowdAudioSubsystem::OnOvertake(const FString& OvertakerID, const FString& OvertakenID, FVector Location)
{
	TriggerCrowdEvent(EMGCrowdEventType::Overtake, Location, 0.6f);
}

void UMGCrowdAudioSubsystem::OnCrash(const FString& PlayerID, FVector Location, float Severity)
{
	EMGCrowdEventType Event = PlayerID == TEXT("LocalPlayer") ?
		EMGCrowdEventType::PlayerCrash : EMGCrowdEventType::Crash;

	TriggerCrowdEvent(Event, Location, Severity);
}

// Listener
void UMGCrowdAudioSubsystem::UpdateListenerLocation(FVector Location)
{
	ListenerLocation = Location;

	// Check zone transitions
	FName NewZoneID = NAME_None;

	for (const FMGCrowdZone& Zone : CrowdZones)
	{
		float Dist = FVector::Dist(Location, Zone.Location);
		if (Dist <= Zone.Radius)
		{
			NewZoneID = Zone.ZoneID;
			break;
		}
	}

	if (NewZoneID != CrowdState.ActiveZoneID)
	{
		if (CrowdState.ActiveZoneID != NAME_None)
		{
			FMGCrowdZone OldZone = GetCrowdZone(CrowdState.ActiveZoneID);
			OnCrowdZoneExited.Broadcast(OldZone);
		}

		CrowdState.ActiveZoneID = NewZoneID;

		if (NewZoneID != NAME_None)
		{
			FMGCrowdZone NewZone = GetCrowdZone(NewZoneID);
			OnCrowdZoneEntered.Broadcast(NewZone);
		}
	}
}

// Reaction Configuration
void UMGCrowdAudioSubsystem::RegisterReaction(const FMGCrowdReaction& Reaction)
{
	// Remove existing reaction for this event
	Reactions.RemoveAll([&Reaction](const FMGCrowdReaction& R)
	{
		return R.TriggerEvent == Reaction.TriggerEvent;
	});

	Reactions.Add(Reaction);
}

void UMGCrowdAudioSubsystem::SetWaveSettings(const FMGCrowdWaveSettings& Settings)
{
	WaveSettings = Settings;
}

// Volume Control
void UMGCrowdAudioSubsystem::SetCrowdVolume(float Volume)
{
	MasterCrowdVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMGCrowdAudioSubsystem::FadeOutCrowd(float FadeTime)
{
	CrowdState.TargetVolume = 0.0f;
}

void UMGCrowdAudioSubsystem::FadeInCrowd(float FadeTime)
{
	CrowdState.TargetVolume = 1.0f;
}

// Internal
void UMGCrowdAudioSubsystem::OnCrowdTick()
{
	float DeltaTime = 0.05f;

	CrowdState.TimeSinceLastReaction += DeltaTime;

	// Update cooldowns
	TArray<EMGCrowdEventType> ExpiredCooldowns;
	for (auto& Pair : EventCooldowns)
	{
		Pair.Value -= DeltaTime;
		if (Pair.Value <= 0.0f)
		{
			ExpiredCooldowns.Add(Pair.Key);
		}
	}
	for (EMGCrowdEventType Event : ExpiredCooldowns)
	{
		EventCooldowns.Remove(Event);
	}

	DecayExcitement(DeltaTime);
	UpdateMood();
	UpdateZoneAudio();

	// Interpolate volume
	CrowdState.CurrentVolume = FMath::FInterpTo(
		CrowdState.CurrentVolume,
		CrowdState.TargetVolume,
		DeltaTime,
		2.0f
	);
}

void UMGCrowdAudioSubsystem::UpdateZoneAudio()
{
	for (const FMGCrowdZone& Zone : CrowdZones)
	{
		float Distance = FVector::Dist(ListenerLocation, Zone.Location);
		if (Distance > Zone.Radius * 2.0f)
		{
			continue; // Too far, skip
		}

		float DistanceFactor = 1.0f - FMath::Clamp(Distance / (Zone.Radius * 2.0f), 0.0f, 1.0f);
		float DensityFactor = static_cast<float>(Zone.CrowdDensity) / 100.0f;
		float ExcitementFactor = 0.5f + (CrowdState.ExcitementLevel * 0.5f * Zone.ExcitementMultiplier);

		float Volume = Zone.BaseVolume * DistanceFactor * DensityFactor * ExcitementFactor * MasterCrowdVolume * CrowdState.CurrentVolume;

		// Would set volume on audio component
	}
}

void UMGCrowdAudioSubsystem::UpdateMood()
{
	// Auto-transition moods based on excitement
	if (CrowdState.ExcitementLevel > 0.8f && CrowdState.CurrentMood != EMGCrowdMood::Cheering)
	{
		if (CrowdState.TensionLevel > 0.5f)
		{
			SetGlobalMood(EMGCrowdMood::Tense, 1.0f);
		}
		else
		{
			SetGlobalMood(EMGCrowdMood::Cheering, 1.0f);
		}
	}
	else if (CrowdState.ExcitementLevel < 0.2f && CrowdState.CurrentMood != EMGCrowdMood::Calm)
	{
		SetGlobalMood(EMGCrowdMood::Calm, 2.0f);
	}
}

void UMGCrowdAudioSubsystem::DecayExcitement(float DeltaTime)
{
	float OldExcitement = CrowdState.ExcitementLevel;

	CrowdState.ExcitementLevel = FMath::Max(0.0f, CrowdState.ExcitementLevel - (ExcitementDecayRate * DeltaTime));
	CrowdState.TensionLevel = FMath::Max(0.0f, CrowdState.TensionLevel - (TensionDecayRate * DeltaTime));

	if (!FMath::IsNearlyEqual(OldExcitement, CrowdState.ExcitementLevel, 0.01f))
	{
		OnExcitementChanged.Broadcast(CrowdState.ExcitementLevel);
	}
}

void UMGCrowdAudioSubsystem::ProcessCrowdWave(FVector Origin, float Intensity)
{
	// Wave propagates outward through zones
	TArray<FMGCrowdZone> SortedZones = CrowdZones;
	SortedZones.Sort([&Origin](const FMGCrowdZone& A, const FMGCrowdZone& B)
	{
		return FVector::Dist(Origin, A.Location) < FVector::Dist(Origin, B.Location);
	});

	float CurrentIntensity = Intensity;
	for (const FMGCrowdZone& Zone : SortedZones)
	{
		if (CurrentIntensity <= 0.1f)
		{
			break;
		}

		float Delay = FVector::Dist(Origin, Zone.Location) / WaveSettings.WaveSpeed;
		CurrentIntensity *= (1.0f - WaveSettings.WaveDecay);

		// Would schedule delayed reaction in zone
	}
}

void UMGCrowdAudioSubsystem::PlayReactionSound(const FMGCrowdReaction& Reaction, FVector Location)
{
	// Would play the reaction sound at the location
}

FMGCrowdReaction UMGCrowdAudioSubsystem::GetReactionForEvent(EMGCrowdEventType Event) const
{
	const FMGCrowdReaction* Reaction = Reactions.FindByPredicate([Event](const FMGCrowdReaction& R)
	{
		return R.TriggerEvent == Event;
	});

	if (Reaction)
	{
		return *Reaction;
	}

	// Return default reaction
	FMGCrowdReaction Default;
	Default.TriggerEvent = Event;
	Default.ResultingMood = EMGCrowdMood::Excited;
	Default.Intensity = 1.0f;
	Default.Duration = 2.0f;
	return Default;
}

void UMGCrowdAudioSubsystem::InitializeDefaultReactions()
{
	// Race Start
	{
		FMGCrowdReaction R;
		R.TriggerEvent = EMGCrowdEventType::RaceStart;
		R.ResultingMood = EMGCrowdMood::Excited;
		R.Intensity = 1.0f;
		R.Duration = 5.0f;
		R.CooldownTime = 0.0f;
		R.bAffectsAllZones = true;
		Reactions.Add(R);
	}

	// Overtake
	{
		FMGCrowdReaction R;
		R.TriggerEvent = EMGCrowdEventType::Overtake;
		R.ResultingMood = EMGCrowdMood::Cheering;
		R.Intensity = 0.6f;
		R.Duration = 2.0f;
		R.CooldownTime = 0.5f;
		R.MaxDistance = 1500.0f;
		Reactions.Add(R);
	}

	// Crash
	{
		FMGCrowdReaction R;
		R.TriggerEvent = EMGCrowdEventType::Crash;
		R.ResultingMood = EMGCrowdMood::Gasping;
		R.Intensity = 0.8f;
		R.Duration = 3.0f;
		R.CooldownTime = 1.0f;
		R.MaxDistance = 2000.0f;
		Reactions.Add(R);
	}

	// Near Miss
	{
		FMGCrowdReaction R;
		R.TriggerEvent = EMGCrowdEventType::NearMiss;
		R.ResultingMood = EMGCrowdMood::Gasping;
		R.Intensity = 0.5f;
		R.Duration = 1.5f;
		R.CooldownTime = 0.3f;
		R.MaxDistance = 1000.0f;
		Reactions.Add(R);
	}

	// Final Lap
	{
		FMGCrowdReaction R;
		R.TriggerEvent = EMGCrowdEventType::FinalLap;
		R.ResultingMood = EMGCrowdMood::Tense;
		R.Intensity = 0.9f;
		R.Duration = 10.0f;
		R.CooldownTime = 0.0f;
		R.bAffectsAllZones = true;
		Reactions.Add(R);
	}

	// Player Win
	{
		FMGCrowdReaction R;
		R.TriggerEvent = EMGCrowdEventType::PlayerWin;
		R.ResultingMood = EMGCrowdMood::Celebrating;
		R.Intensity = 1.0f;
		R.Duration = 15.0f;
		R.CooldownTime = 0.0f;
		R.bAffectsAllZones = true;
		Reactions.Add(R);
	}

	// Lead Change
	{
		FMGCrowdReaction R;
		R.TriggerEvent = EMGCrowdEventType::LeadChange;
		R.ResultingMood = EMGCrowdMood::Cheering;
		R.Intensity = 0.8f;
		R.Duration = 3.0f;
		R.CooldownTime = 2.0f;
		R.bAffectsAllZones = true;
		Reactions.Add(R);
	}
}

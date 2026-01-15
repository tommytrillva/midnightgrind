// Copyright Midnight Grind. All Rights Reserved.

#include "Audio/MGMusicManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeAudioComponents();

	UE_LOG(LogTemp, Log, TEXT("MGMusicManager initialized"));
}

void UMGMusicManager::Deinitialize()
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	CleanupAudioComponents();

	Super::Deinitialize();
}

// ==========================================
// MUSIC CONTROL
// ==========================================

void UMGMusicManager::SetMusicState(EMGMusicState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EMGMusicState OldState = CurrentState;
	CurrentState = NewState;

	// Determine track for new state
	FName NewTrack = GetTrackForState(NewState);

	if (!NewTrack.IsNone() && NewTrack != CurrentTrackName)
	{
		PlayTrack(NewTrack, TrackFadeTime);
	}

	// Adjust intensity based on state
	switch (NewState)
	{
	case EMGMusicState::None:
		StopMusic(TrackFadeTime);
		break;

	case EMGMusicState::MainMenu:
	case EMGMusicState::Garage:
	case EMGMusicState::PreRace:
	case EMGMusicState::Results:
		SetIntensity(0.0f);
		break;

	case EMGMusicState::RacingLow:
		SetIntensity(0.25f);
		break;

	case EMGMusicState::RacingMedium:
		SetIntensity(0.5f);
		break;

	case EMGMusicState::RacingHigh:
		SetIntensity(0.85f);
		break;

	case EMGMusicState::Victory:
		SetIntensity(1.0f);
		break;

	case EMGMusicState::Defeat:
		SetIntensity(0.3f);
		break;
	}

	OnMusicStateChanged.Broadcast(NewState);

	UE_LOG(LogTemp, Log, TEXT("MGMusicManager: State changed from %d to %d"),
		static_cast<int32>(OldState), static_cast<int32>(NewState));
}

void UMGMusicManager::SetIntensity(float Intensity)
{
	TargetIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGMusicManager::PlayTrack(FName TrackName, float FadeTime)
{
	FMGMusicTrack* Track = Tracks.Find(TrackName);
	if (!Track)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGMusicManager: Track '%s' not found"), *TrackName.ToString());
		return;
	}

	StartTrack(*Track, FadeTime);
	CurrentTrackName = TrackName;
	PlaybackTime = 0.0f;
	BeatTimer = 0.0f;

	OnTrackChanged.Broadcast(TrackName);

	// Start update timer if not running
	if (UWorld* World = GetWorld())
	{
		if (!UpdateTimerHandle.IsValid())
		{
			World->GetTimerManager().SetTimer(UpdateTimerHandle,
				FTimerDelegate::CreateUObject(this, &UMGMusicManager::OnUpdateTimer),
				0.016f, true); // ~60fps update
		}
	}
}

void UMGMusicManager::StopMusic(float FadeTime)
{
	// Fade out all layers
	auto FadeOutComp = [FadeTime](UAudioComponent* Comp)
	{
		if (Comp && Comp->IsPlaying())
		{
			Comp->FadeOut(FadeTime, 0.0f);
		}
	};

	FadeOutComp(BaseLayerComp);
	FadeOutComp(LowLayerComp);
	FadeOutComp(MediumLayerComp);
	FadeOutComp(HighLayerComp);
	FadeOutComp(ClimaxLayerComp);

	CurrentTrackName = NAME_None;
}

void UMGMusicManager::PauseMusic()
{
	bIsPaused = true;

	auto PauseComp = [](UAudioComponent* Comp)
	{
		if (Comp) Comp->SetPaused(true);
	};

	PauseComp(BaseLayerComp);
	PauseComp(LowLayerComp);
	PauseComp(MediumLayerComp);
	PauseComp(HighLayerComp);
	PauseComp(ClimaxLayerComp);
}

void UMGMusicManager::ResumeMusic()
{
	bIsPaused = false;

	auto ResumeComp = [](UAudioComponent* Comp)
	{
		if (Comp) Comp->SetPaused(false);
	};

	ResumeComp(BaseLayerComp);
	ResumeComp(LowLayerComp);
	ResumeComp(MediumLayerComp);
	ResumeComp(HighLayerComp);
	ResumeComp(ClimaxLayerComp);
}

// ==========================================
// STINGERS
// ==========================================

void UMGMusicManager::PlayStinger(FName StingerName)
{
	FMGMusicStinger* Stinger = Stingers.Find(StingerName);
	if (!Stinger || !Stinger->Sound)
	{
		return;
	}

	if (StingerComp)
	{
		StingerComp->SetSound(Stinger->Sound);
		StingerComp->SetVolumeMultiplier(MusicVolume);
		StingerComp->Play();

		// Duck music if configured
		if (Stinger->bDuckMusic)
		{
			float DuckedVolume = MusicVolume * (1.0f - Stinger->DuckAmount);

			if (BaseLayerComp) BaseLayerComp->SetVolumeMultiplier(DuckedVolume);
			if (LowLayerComp) LowLayerComp->SetVolumeMultiplier(DuckedVolume);
			if (MediumLayerComp) MediumLayerComp->SetVolumeMultiplier(DuckedVolume);
			if (HighLayerComp) HighLayerComp->SetVolumeMultiplier(DuckedVolume);
			if (ClimaxLayerComp) ClimaxLayerComp->SetVolumeMultiplier(DuckedVolume);

			// Restore after stinger (simplified - would need proper tracking)
		}
	}
}

void UMGMusicManager::RegisterStinger(const FMGMusicStinger& Stinger)
{
	Stingers.Add(Stinger.StingerName, Stinger);
}

// ==========================================
// TRACK MANAGEMENT
// ==========================================

void UMGMusicManager::RegisterTrack(const FMGMusicTrack& Track)
{
	Tracks.Add(Track.TrackName, Track);
	Playlist.Add(Track.TrackName);
}

bool UMGMusicManager::GetTrack(FName TrackName, FMGMusicTrack& OutTrack) const
{
	const FMGMusicTrack* Track = Tracks.Find(TrackName);
	if (Track)
	{
		OutTrack = *Track;
		return true;
	}
	return false;
}

void UMGMusicManager::NextTrack()
{
	if (Playlist.Num() == 0)
	{
		return;
	}

	PlaylistIndex = (PlaylistIndex + 1) % Playlist.Num();
	PlayTrack(Playlist[PlaylistIndex], TrackFadeTime);
}

void UMGMusicManager::PreviousTrack()
{
	if (Playlist.Num() == 0)
	{
		return;
	}

	PlaylistIndex--;
	if (PlaylistIndex < 0)
	{
		PlaylistIndex = Playlist.Num() - 1;
	}

	PlayTrack(Playlist[PlaylistIndex], TrackFadeTime);
}

void UMGMusicManager::ShufflePlaylist()
{
	// Fisher-Yates shuffle
	for (int32 i = Playlist.Num() - 1; i > 0; --i)
	{
		int32 j = FMath::RandRange(0, i);
		Playlist.Swap(i, j);
	}

	PlaylistIndex = 0;
}

// ==========================================
// VOLUME
// ==========================================

void UMGMusicManager::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

// ==========================================
// INTERNAL
// ==========================================

void UMGMusicManager::InitializeAudioComponents()
{
	// Create audio components for each layer
	// These will be parented to a persistent actor or created as standalone

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	auto CreateComp = [World]() -> UAudioComponent*
	{
		UAudioComponent* Comp = NewObject<UAudioComponent>(World);
		Comp->bAutoActivate = false;
		Comp->bIsUISound = true; // Non-spatialized
		Comp->bAllowSpatialization = false;
		Comp->RegisterComponent();
		return Comp;
	};

	BaseLayerComp = CreateComp();
	LowLayerComp = CreateComp();
	MediumLayerComp = CreateComp();
	HighLayerComp = CreateComp();
	ClimaxLayerComp = CreateComp();
	StingerComp = CreateComp();
}

void UMGMusicManager::CleanupAudioComponents()
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

	DestroyComp(BaseLayerComp);
	DestroyComp(LowLayerComp);
	DestroyComp(MediumLayerComp);
	DestroyComp(HighLayerComp);
	DestroyComp(ClimaxLayerComp);
	DestroyComp(StingerComp);
}

void UMGMusicManager::UpdateLayerVolumes(float DeltaTime)
{
	// Smooth intensity transitions
	CurrentIntensity = FMath::FInterpTo(CurrentIntensity, TargetIntensity, DeltaTime, 3.0f);

	// Calculate layer volumes based on intensity
	// Base layer always audible
	float BaseVol = MusicVolume;

	// Low layer: full 0-0.3, fade 0.3-0.5
	float LowVol = 0.0f;
	if (CurrentIntensity <= 0.3f)
	{
		LowVol = CurrentIntensity / 0.3f;
	}
	else if (CurrentIntensity <= 0.5f)
	{
		LowVol = 1.0f - (CurrentIntensity - 0.3f) / 0.2f;
	}
	LowVol *= MusicVolume;

	// Medium layer: fade in 0.25-0.5, full 0.5-0.7, fade 0.7-0.85
	float MedVol = 0.0f;
	if (CurrentIntensity >= 0.25f && CurrentIntensity <= 0.5f)
	{
		MedVol = (CurrentIntensity - 0.25f) / 0.25f;
	}
	else if (CurrentIntensity > 0.5f && CurrentIntensity <= 0.7f)
	{
		MedVol = 1.0f;
	}
	else if (CurrentIntensity > 0.7f && CurrentIntensity <= 0.85f)
	{
		MedVol = 1.0f - (CurrentIntensity - 0.7f) / 0.15f;
	}
	MedVol *= MusicVolume;

	// High layer: fade in 0.6-0.8, full 0.8-1.0
	float HighVol = 0.0f;
	if (CurrentIntensity >= 0.6f && CurrentIntensity <= 0.8f)
	{
		HighVol = (CurrentIntensity - 0.6f) / 0.2f;
	}
	else if (CurrentIntensity > 0.8f)
	{
		HighVol = 1.0f;
	}
	HighVol *= MusicVolume;

	// Climax layer: only at max intensity
	float ClimaxVol = 0.0f;
	if (CurrentIntensity >= 0.9f)
	{
		ClimaxVol = (CurrentIntensity - 0.9f) / 0.1f;
	}
	ClimaxVol *= MusicVolume;

	// Apply volumes
	if (BaseLayerComp) BaseLayerComp->SetVolumeMultiplier(BaseVol);
	if (LowLayerComp) LowLayerComp->SetVolumeMultiplier(LowVol);
	if (MediumLayerComp) MediumLayerComp->SetVolumeMultiplier(MedVol);
	if (HighLayerComp) HighLayerComp->SetVolumeMultiplier(HighVol);
	if (ClimaxLayerComp) ClimaxLayerComp->SetVolumeMultiplier(ClimaxVol);
}

void UMGMusicManager::StartTrack(const FMGMusicTrack& Track, float FadeTime)
{
	// Set sounds for each layer
	auto SetupLayer = [FadeTime](UAudioComponent* Comp, USoundBase* Sound)
	{
		if (Comp)
		{
			if (Comp->IsPlaying())
			{
				Comp->FadeOut(FadeTime, 0.0f);
			}

			if (Sound)
			{
				Comp->SetSound(Sound);
				Comp->FadeIn(FadeTime, 1.0f);
			}
		}
	};

	SetupLayer(BaseLayerComp, Track.BaseLayer);
	SetupLayer(LowLayerComp, Track.LowLayer);
	SetupLayer(MediumLayerComp, Track.MediumLayer);
	SetupLayer(HighLayerComp, Track.HighLayer);
	SetupLayer(ClimaxLayerComp, Track.ClimaxLayer);

	UE_LOG(LogTemp, Log, TEXT("MGMusicManager: Started track '%s'"), *Track.TrackName.ToString());
}

void UMGMusicManager::UpdateBeatTracking(float DeltaTime)
{
	FMGMusicTrack* CurrentTrack = Tracks.Find(CurrentTrackName);
	if (!CurrentTrack || CurrentTrack->BPM <= 0.0f)
	{
		return;
	}

	float BeatInterval = 60.0f / CurrentTrack->BPM;

	BeatTimer += DeltaTime;
	if (BeatTimer >= BeatInterval)
	{
		BeatTimer -= BeatInterval;
		OnBeat.Broadcast();
	}

	PlaybackTime += DeltaTime;

	// Handle track looping
	if (CurrentTrack->bCanLoop && PlaybackTime >= CurrentTrack->Duration)
	{
		PlaybackTime = 0.0f;
	}
}

FName UMGMusicManager::GetTrackForState(EMGMusicState State) const
{
	// This would ideally be configured via data asset
	// For now, return first track in playlist or specific named tracks

	switch (State)
	{
	case EMGMusicState::MainMenu:
		if (Tracks.Contains(FName("Menu"))) return FName("Menu");
		break;

	case EMGMusicState::Garage:
		if (Tracks.Contains(FName("Garage"))) return FName("Garage");
		break;

	case EMGMusicState::PreRace:
		if (Tracks.Contains(FName("PreRace"))) return FName("PreRace");
		break;

	case EMGMusicState::RacingLow:
	case EMGMusicState::RacingMedium:
	case EMGMusicState::RacingHigh:
		// Return current racing track or random from playlist
		if (Playlist.Num() > 0)
		{
			return Playlist[PlaylistIndex];
		}
		break;

	case EMGMusicState::Victory:
		if (Tracks.Contains(FName("Victory"))) return FName("Victory");
		break;

	case EMGMusicState::Defeat:
		if (Tracks.Contains(FName("Defeat"))) return FName("Defeat");
		break;

	case EMGMusicState::Results:
		if (Tracks.Contains(FName("Results"))) return FName("Results");
		break;

	default:
		break;
	}

	// Fallback to first track
	if (Playlist.Num() > 0)
	{
		return Playlist[0];
	}

	return NAME_None;
}

void UMGMusicManager::OnUpdateTimer()
{
	if (bIsPaused)
	{
		return;
	}

	float DeltaTime = 0.016f; // Timer interval

	UpdateLayerVolumes(DeltaTime);
	UpdateBeatTracking(DeltaTime);
}

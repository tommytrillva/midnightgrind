// Copyright Midnight Grind. All Rights Reserved.

#include "Audio/MGMusicManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UMGMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize layer volumes
	LayerVolumes.Add(EMGMusicLayer::Base, 1.0f);
	LayerVolumes.Add(EMGMusicLayer::Melody, 1.0f);
	LayerVolumes.Add(EMGMusicLayer::Synths, 0.8f);
	LayerVolumes.Add(EMGMusicLayer::Bass, 1.0f);
	LayerVolumes.Add(EMGMusicLayer::Percussion, 0.9f);
	LayerVolumes.Add(EMGMusicLayer::Vocals, 0.7f);
	LayerVolumes.Add(EMGMusicLayer::Stinger, 1.0f);

	InitializeDefaultTracks();
	InitializeDefaultPlaylists();

	// Set up tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&UMGMusicManager::OnTick,
			0.05f,
			true
		);
	}
}

void UMGMusicManager::Deinitialize()
{
	Stop();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	Super::Deinitialize();
}

void UMGMusicManager::SetMusicState(EMGMusicState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
	OnMusicStateChanged.Broadcast(NewState);

	// Get appropriate track for state
	FName TrackID = GetTrackForState(NewState);
	if (!TrackID.IsNone())
	{
		CrossfadeTo(TrackID, 1.5f);
	}

	// Update intensity based on state
	switch (NewState)
	{
	case EMGMusicState::RacingLow:
		SetRaceIntensity(0.3f);
		break;
	case EMGMusicState::RacingMedium:
		SetRaceIntensity(0.6f);
		break;
	case EMGMusicState::RacingHigh:
		SetRaceIntensity(0.9f);
		break;
	case EMGMusicState::FinalLap:
		SetRaceIntensity(1.0f);
		break;
	default:
		SetRaceIntensity(0.5f);
		break;
	}
}

void UMGMusicManager::SetRaceIntensity(float Intensity)
{
	TargetIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGMusicManager::TriggerMusicEvent(const FMGMusicEvent& Event)
{
	// Adjust intensity
	if (Event.IntensityModifier != 0.0f)
	{
		TargetIntensity = FMath::Clamp(TargetIntensity + Event.IntensityModifier, 0.0f, 1.0f);
	}

	// Play stinger if requested
	if (Event.bTriggerStinger && !Event.StingerID.IsNone())
	{
		// Would play one-shot stinger sound here
	}
}

void UMGMusicManager::PlayTrack(FName TrackID)
{
	if (!TrackLibrary.Contains(TrackID))
	{
		return;
	}

	CurrentTrack = TrackLibrary[TrackID];
	CurrentTrack.PlayCount++;
	PlaybackPosition = 0.0f;
	bIsPlaying = true;

	// Update beat tracking
	SecondsPerBeat = 60.0f / CurrentTrack.BPM;
	BeatAccumulator = 0.0f;

	OnTrackChanged.Broadcast(CurrentTrack);

	// Would actually start audio playback here
}

void UMGMusicManager::PlayNext()
{
	if (CurrentPlaylist.TrackIDs.Num() == 0)
	{
		return;
	}

	if (CurrentPlaylist.bShuffle)
	{
		PlaylistIndex = FMath::RandRange(0, CurrentPlaylist.TrackIDs.Num() - 1);
	}
	else
	{
		PlaylistIndex = (PlaylistIndex + 1) % CurrentPlaylist.TrackIDs.Num();
	}

	PlayTrack(CurrentPlaylist.TrackIDs[PlaylistIndex]);
}

void UMGMusicManager::PlayPrevious()
{
	if (CurrentPlaylist.TrackIDs.Num() == 0)
	{
		return;
	}

	// If more than 3 seconds in, restart current track
	if (PlaybackPosition > 3.0f)
	{
		PlaybackPosition = 0.0f;
		return;
	}

	PlaylistIndex--;
	if (PlaylistIndex < 0)
	{
		PlaylistIndex = CurrentPlaylist.TrackIDs.Num() - 1;
	}

	PlayTrack(CurrentPlaylist.TrackIDs[PlaylistIndex]);
}

void UMGMusicManager::Pause()
{
	bIsPlaying = false;
}

void UMGMusicManager::Resume()
{
	bIsPlaying = true;
}

void UMGMusicManager::Stop()
{
	bIsPlaying = false;
	PlaybackPosition = 0.0f;
}

void UMGMusicManager::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMGMusicManager::SetLayerVolume(EMGMusicLayer Layer, float Volume)
{
	LayerVolumes.FindOrAdd(Layer) = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UMGMusicManager::FadeToVolume(float TargetVolume, float Duration)
{
	bFading = true;
	FadeStartVolume = MusicVolume;
	FadeTargetVolume = FMath::Clamp(TargetVolume, 0.0f, 1.0f);
	FadeDuration = Duration;
	FadeElapsed = 0.0f;
}

void UMGMusicManager::DuckMusic(float Amount, float Duration)
{
	bDucking = true;
	DuckAmount = FMath::Clamp(Amount, 0.0f, 1.0f);
	DuckDuration = Duration;
	DuckElapsed = 0.0f;
}

void UMGMusicManager::SetPlaylist(FName PlaylistID)
{
	if (Playlists.Contains(PlaylistID))
	{
		CurrentPlaylist = Playlists[PlaylistID];
		PlaylistIndex = 0;

		if (CurrentPlaylist.TrackIDs.Num() > 0)
		{
			if (CurrentPlaylist.bShuffle)
			{
				PlaylistIndex = FMath::RandRange(0, CurrentPlaylist.TrackIDs.Num() - 1);
			}
			PlayTrack(CurrentPlaylist.TrackIDs[PlaylistIndex]);
		}
	}
}

TArray<FMGPlaylist> UMGMusicManager::GetAllPlaylists() const
{
	TArray<FMGPlaylist> Result;
	Playlists.GenerateValueArray(Result);
	return Result;
}

void UMGMusicManager::SetShuffle(bool bEnabled)
{
	CurrentPlaylist.bShuffle = bEnabled;
}

void UMGMusicManager::SetRepeat(bool bEnabled)
{
	CurrentPlaylist.bRepeat = bEnabled;
}

TArray<FMGMusicTrack> UMGMusicManager::GetAllTracks() const
{
	TArray<FMGMusicTrack> Result;
	TrackLibrary.GenerateValueArray(Result);
	return Result;
}

TArray<FMGMusicTrack> UMGMusicManager::GetTracksByGenre(FName Genre) const
{
	TArray<FMGMusicTrack> Result;
	for (const auto& Pair : TrackLibrary)
	{
		if (Pair.Value.Genre == Genre)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

void UMGMusicManager::ToggleFavorite(FName TrackID)
{
	if (TrackLibrary.Contains(TrackID))
	{
		TrackLibrary[TrackID].bFavorite = !TrackLibrary[TrackID].bFavorite;
	}
}

TArray<FMGMusicTrack> UMGMusicManager::GetFavorites() const
{
	TArray<FMGMusicTrack> Result;
	for (const auto& Pair : TrackLibrary)
	{
		if (Pair.Value.bFavorite)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

float UMGMusicManager::GetTimeToNextBeat() const
{
	return SecondsPerBeat - BeatAccumulator;
}

float UMGMusicManager::GetCurrentBPM() const
{
	return CurrentTrack.BPM;
}

bool UMGMusicManager::IsOnBeat(float Tolerance) const
{
	return BeatAccumulator < Tolerance || (SecondsPerBeat - BeatAccumulator) < Tolerance;
}

void UMGMusicManager::OnTick()
{
	const float DeltaTime = 0.05f;

	if (!bIsPlaying)
	{
		return;
	}

	// Update playback position
	PlaybackPosition += DeltaTime;
	if (PlaybackPosition >= CurrentTrack.Duration)
	{
		if (CurrentPlaylist.bRepeat)
		{
			PlayNext();
		}
		else
		{
			Stop();
		}
	}

	// Smooth intensity
	float IntensityDelta = TargetIntensity - CurrentIntensity;
	if (FMath::Abs(IntensityDelta) > 0.01f)
	{
		CurrentIntensity += FMath::Sign(IntensityDelta) * IntensitySmoothRate * DeltaTime;
		CurrentIntensity = FMath::Clamp(CurrentIntensity, 0.0f, 1.0f);
		UpdateIntensityMixing();
		OnIntensityChanged.Broadcast(CurrentIntensity);
	}

	// Update fading
	if (bFading)
	{
		FadeElapsed += DeltaTime;
		float Alpha = FMath::Clamp(FadeElapsed / FadeDuration, 0.0f, 1.0f);
		MusicVolume = FMath::Lerp(FadeStartVolume, FadeTargetVolume, Alpha);

		if (FadeElapsed >= FadeDuration)
		{
			bFading = false;
			MusicVolume = FadeTargetVolume;
		}
	}

	// Update ducking
	if (bDucking)
	{
		DuckElapsed += DeltaTime;
		if (DuckElapsed >= DuckDuration)
		{
			bDucking = false;
		}
	}

	// Beat tracking
	UpdateBeatTracking(DeltaTime);
}

void UMGMusicManager::UpdateIntensityMixing()
{
	// Adjust layer volumes based on intensity
	// Low intensity: Base, Bass prominent
	// High intensity: All layers, Percussion/Stinger prominent

	float BaseVol = 1.0f;
	float MelodyVol = 0.5f + CurrentIntensity * 0.5f;
	float SynthVol = 0.3f + CurrentIntensity * 0.7f;
	float BassVol = 1.0f;
	float PercVol = 0.4f + CurrentIntensity * 0.6f;

	LayerVolumes[EMGMusicLayer::Base] = BaseVol;
	LayerVolumes[EMGMusicLayer::Melody] = MelodyVol;
	LayerVolumes[EMGMusicLayer::Synths] = SynthVol;
	LayerVolumes[EMGMusicLayer::Bass] = BassVol;
	LayerVolumes[EMGMusicLayer::Percussion] = PercVol;
}

void UMGMusicManager::CrossfadeTo(FName TrackID, float Duration)
{
	// Start fade out current
	FadeToVolume(0.0f, Duration * 0.5f);

	// Queue new track (simplified - would use proper async in real implementation)
	PlayTrack(TrackID);
	FadeToVolume(MusicVolume, Duration * 0.5f);
}

FName UMGMusicManager::GetTrackForState(EMGMusicState State) const
{
	switch (State)
	{
	case EMGMusicState::MainMenu:
		return FName(TEXT("MainTheme"));
	case EMGMusicState::Garage:
		return FName(TEXT("GarageAmbient"));
	case EMGMusicState::Lobby:
		return FName(TEXT("LobbyVibes"));
	case EMGMusicState::RacingLow:
	case EMGMusicState::RacingMedium:
	case EMGMusicState::RacingHigh:
	case EMGMusicState::FinalLap:
		return FName(TEXT("RaceTrack01"));
	case EMGMusicState::Victory:
		return FName(TEXT("VictoryFanfare"));
	case EMGMusicState::Defeat:
		return FName(TEXT("DefeatTheme"));
	case EMGMusicState::Results:
		return FName(TEXT("ResultsScreen"));
	default:
		return NAME_None;
	}
}

void UMGMusicManager::InitializeDefaultTracks()
{
	// Create default track library with Y2K synthwave aesthetic
	auto AddTrack = [this](FName ID, const TCHAR* Name, const TCHAR* Artist, FName Genre, float BPM, float Duration)
	{
		FMGMusicTrack Track;
		Track.TrackID = ID;
		Track.DisplayName = FText::FromString(Name);
		Track.Artist = FText::FromString(Artist);
		Track.Genre = Genre;
		Track.BPM = BPM;
		Track.Duration = Duration;
		TrackLibrary.Add(ID, Track);
	};

	// Menu/Ambient
	AddTrack(FName(TEXT("MainTheme")), TEXT("Midnight Grind"), TEXT("Neon Riders"), FName(TEXT("Synthwave")), 110.0f, 240.0f);
	AddTrack(FName(TEXT("GarageAmbient")), TEXT("Chrome Dreams"), TEXT("Digital Sunset"), FName(TEXT("Ambient")), 90.0f, 300.0f);
	AddTrack(FName(TEXT("LobbyVibes")), TEXT("Pre-Race Tension"), TEXT("Turbo Knights"), FName(TEXT("Synthwave")), 125.0f, 180.0f);

	// Racing tracks
	AddTrack(FName(TEXT("RaceTrack01")), TEXT("Neon Highway"), TEXT("Laser Grid"), FName(TEXT("Electro")), 140.0f, 210.0f);
	AddTrack(FName(TEXT("RaceTrack02")), TEXT("Velocity"), TEXT("Cyber Pulse"), FName(TEXT("DnB")), 174.0f, 195.0f);
	AddTrack(FName(TEXT("RaceTrack03")), TEXT("Downtown Rush"), TEXT("Street Phantom"), FName(TEXT("House")), 128.0f, 225.0f);
	AddTrack(FName(TEXT("RaceTrack04")), TEXT("Turbo Drift"), TEXT("Retro Wave"), FName(TEXT("Synthwave")), 132.0f, 200.0f);
	AddTrack(FName(TEXT("RaceTrack05")), TEXT("Night Chase"), TEXT("Neon Samurai"), FName(TEXT("Electro")), 145.0f, 215.0f);
	AddTrack(FName(TEXT("RaceTrack06")), TEXT("Pink Slip"), TEXT("The Midnight"), FName(TEXT("Synthwave")), 118.0f, 250.0f);
	AddTrack(FName(TEXT("RaceTrack07")), TEXT("Max Speed"), TEXT("Power Glove"), FName(TEXT("Electro")), 150.0f, 185.0f);
	AddTrack(FName(TEXT("RaceTrack08")), TEXT("Final Lap"), TEXT("Scandroid"), FName(TEXT("Synthwave")), 135.0f, 220.0f);

	// Victory/Defeat
	AddTrack(FName(TEXT("VictoryFanfare")), TEXT("Champion"), TEXT("Victory Sound"), FName(TEXT("Fanfare")), 120.0f, 30.0f);
	AddTrack(FName(TEXT("DefeatTheme")), TEXT("Next Time"), TEXT("Loss Music"), FName(TEXT("Ambient")), 80.0f, 25.0f);
	AddTrack(FName(TEXT("ResultsScreen")), TEXT("Tallying Up"), TEXT("Score Music"), FName(TEXT("Ambient")), 95.0f, 120.0f);
}

void UMGMusicManager::InitializeDefaultPlaylists()
{
	// Racing playlist
	FMGPlaylist RacingPlaylist;
	RacingPlaylist.PlaylistID = FName(TEXT("Racing"));
	RacingPlaylist.DisplayName = NSLOCTEXT("MG", "RacingPlaylist", "Racing Mix");
	RacingPlaylist.TrackIDs = {
		FName(TEXT("RaceTrack01")),
		FName(TEXT("RaceTrack02")),
		FName(TEXT("RaceTrack03")),
		FName(TEXT("RaceTrack04")),
		FName(TEXT("RaceTrack05")),
		FName(TEXT("RaceTrack06")),
		FName(TEXT("RaceTrack07")),
		FName(TEXT("RaceTrack08"))
	};
	RacingPlaylist.bShuffle = true;
	RacingPlaylist.bRepeat = true;
	Playlists.Add(RacingPlaylist.PlaylistID, RacingPlaylist);

	// Synthwave only
	FMGPlaylist SynthwavePlaylist;
	SynthwavePlaylist.PlaylistID = FName(TEXT("Synthwave"));
	SynthwavePlaylist.DisplayName = NSLOCTEXT("MG", "SynthwavePlaylist", "Synthwave Only");
	SynthwavePlaylist.TrackIDs = {
		FName(TEXT("MainTheme")),
		FName(TEXT("RaceTrack04")),
		FName(TEXT("RaceTrack06")),
		FName(TEXT("RaceTrack08"))
	};
	SynthwavePlaylist.bShuffle = true;
	SynthwavePlaylist.bRepeat = true;
	Playlists.Add(SynthwavePlaylist.PlaylistID, SynthwavePlaylist);

	// High energy
	FMGPlaylist HighEnergyPlaylist;
	HighEnergyPlaylist.PlaylistID = FName(TEXT("HighEnergy"));
	HighEnergyPlaylist.DisplayName = NSLOCTEXT("MG", "HighEnergyPlaylist", "High Energy");
	HighEnergyPlaylist.TrackIDs = {
		FName(TEXT("RaceTrack02")),
		FName(TEXT("RaceTrack05")),
		FName(TEXT("RaceTrack07"))
	};
	HighEnergyPlaylist.bShuffle = true;
	HighEnergyPlaylist.bRepeat = true;
	Playlists.Add(HighEnergyPlaylist.PlaylistID, HighEnergyPlaylist);
}

void UMGMusicManager::UpdateBeatTracking(float DeltaTime)
{
	BeatAccumulator += DeltaTime;

	if (BeatAccumulator >= SecondsPerBeat)
	{
		BeatAccumulator -= SecondsPerBeat;
		BeatCount++;
		OnBeat.Broadcast();
	}
}

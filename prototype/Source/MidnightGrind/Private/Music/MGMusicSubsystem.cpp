// Copyright Midnight Grind. All Rights Reserved.

#include "Music/MGMusicSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UMGMusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeMusicLibrary();
	InitializeRadioStations();
}

void UMGMusicSubsystem::Deinitialize()
{
	Stop();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BeatTimerHandle);
	}
	Super::Deinitialize();
}

void UMGMusicSubsystem::Play()
{
	if (CurrentQueue.Num() == 0)
	{
		BuildQueue();
	}

	if (CurrentQueue.IsValidIndex(CurrentTrackIndex))
	{
		PlayTrack(CurrentQueue[CurrentTrackIndex]);
	}
}

void UMGMusicSubsystem::Pause()
{
	bIsPlaying = false;
	if (MusicAudioComponent)
	{
		MusicAudioComponent->SetPaused(true);
	}
}

void UMGMusicSubsystem::Stop()
{
	bIsPlaying = false;
	PlaybackPosition = 0.0f;
	if (MusicAudioComponent)
	{
		MusicAudioComponent->Stop();
	}
}

void UMGMusicSubsystem::NextTrack()
{
	CurrentTrackIndex++;
	if (CurrentTrackIndex >= CurrentQueue.Num())
	{
		if (MusicSettings.bRepeat)
		{
			CurrentTrackIndex = 0;
			if (MusicSettings.bShuffle) ShuffleQueue();
		}
		else
		{
			Stop();
			return;
		}
	}

	if (CurrentQueue.IsValidIndex(CurrentTrackIndex))
	{
		if (MusicSettings.bCrossfade)
			CrossfadeToTrack(CurrentQueue[CurrentTrackIndex]);
		else
			PlayTrack(CurrentQueue[CurrentTrackIndex]);
	}
}

void UMGMusicSubsystem::PreviousTrack()
{
	if (PlaybackPosition > 3.0f)
	{
		Seek(0.0f);
		return;
	}

	CurrentTrackIndex--;
	if (CurrentTrackIndex < 0)
		CurrentTrackIndex = CurrentQueue.Num() - 1;

	if (CurrentQueue.IsValidIndex(CurrentTrackIndex))
		PlayTrack(CurrentQueue[CurrentTrackIndex]);
}

void UMGMusicSubsystem::Seek(float PositionSeconds)
{
	PlaybackPosition = PositionSeconds;
	// Would seek audio component
}

float UMGMusicSubsystem::GetPlaybackPosition() const
{
	return PlaybackPosition;
}

FMGMusicTrack UMGMusicSubsystem::GetCurrentTrack() const
{
	if (FMGMusicTrack* Track = const_cast<UMGMusicSubsystem*>(this)->FindTrack(CurrentTrackID))
		return *Track;
	return FMGMusicTrack();
}

void UMGMusicSubsystem::SetStation(FName StationID)
{
	if (FMGRadioStation* Station = FindStation(StationID))
	{
		CurrentStationID = StationID;
		CurrentQueue = Station->TrackIDs;
		CurrentTrackIndex = 0;
		if (MusicSettings.bShuffle) ShuffleQueue();
		OnStationChanged.Broadcast(*Station);
		Play();
	}
}

void UMGMusicSubsystem::NextStation()
{
	if (RadioStations.Num() == 0)
	{
		return;
	}
	int32 CurrentIndex = RadioStations.IndexOfByPredicate([this](const FMGRadioStation& S) { return S.StationID == CurrentStationID; });
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 0;
	}
	else
	{
		CurrentIndex = (CurrentIndex + 1) % RadioStations.Num();
	}
	SetStation(RadioStations[CurrentIndex].StationID);
}

void UMGMusicSubsystem::PreviousStation()
{
	if (RadioStations.Num() == 0)
	{
		return;
	}
	int32 CurrentIndex = RadioStations.IndexOfByPredicate([this](const FMGRadioStation& S) { return S.StationID == CurrentStationID; });
	if (CurrentIndex == INDEX_NONE)
	{
		CurrentIndex = 0;
	}
	else
	{
		CurrentIndex = (CurrentIndex - 1 + RadioStations.Num()) % RadioStations.Num();
	}
	SetStation(RadioStations[CurrentIndex].StationID);
}

FMGRadioStation UMGMusicSubsystem::GetCurrentStation() const
{
	if (FMGRadioStation* Station = const_cast<UMGMusicSubsystem*>(this)->FindStation(CurrentStationID))
		return *Station;
	return FMGRadioStation();
}

TArray<FMGRadioStation> UMGMusicSubsystem::GetUnlockedStations() const
{
	TArray<FMGRadioStation> Unlocked;
	for (const FMGRadioStation& Station : RadioStations)
	{
		if (Station.bIsUnlocked)
			Unlocked.Add(Station);
	}
	return Unlocked;
}

void UMGMusicSubsystem::PlayTrack(FName TrackID)
{
	FMGMusicTrack* Track = FindTrack(TrackID);
	if (!Track) return;

	CurrentTrackID = TrackID;
	bIsPlaying = true;
	PlaybackPosition = 0.0f;

	// Would play audio through audio component
	OnTrackChanged.Broadcast(*Track);

	// Setup beat timer
	if (Track->BPM > 0)
	{
		float BeatInterval = 60.0f / Track->BPM;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(BeatTimerHandle, this, &UMGMusicSubsystem::BeatTick, BeatInterval, true);
		}
	}
}

TArray<FMGMusicTrack> UMGMusicSubsystem::GetTracksByGenre(EMGMusicGenre Genre) const
{
	TArray<FMGMusicTrack> Result;
	for (const FMGMusicTrack& Track : MusicTracks)
	{
		if (Track.Genre == Genre)
			Result.Add(Track);
	}
	return Result;
}

TArray<FMGMusicTrack> UMGMusicSubsystem::GetFavoriteTracks() const
{
	TArray<FMGMusicTrack> Result;
	for (const FMGMusicTrack& Track : MusicTracks)
	{
		if (Track.bIsFavorite)
			Result.Add(Track);
	}
	return Result;
}

void UMGMusicSubsystem::ToggleFavorite(FName TrackID)
{
	if (FMGMusicTrack* Track = FindTrack(TrackID))
		Track->bIsFavorite = !Track->bIsFavorite;
}

FString UMGMusicSubsystem::CreatePlaylist(FText Name)
{
	FMGPlaylist NewPlaylist;
	NewPlaylist.PlaylistID = FGuid::NewGuid().ToString();
	NewPlaylist.PlaylistName = Name;
	NewPlaylist.bIsUserCreated = true;
	NewPlaylist.CreatedDate = FDateTime::Now();
	Playlists.Add(NewPlaylist);
	return NewPlaylist.PlaylistID;
}

void UMGMusicSubsystem::AddToPlaylist(const FString& PlaylistID, FName TrackID)
{
	FMGPlaylist* Playlist = Playlists.FindByPredicate([&PlaylistID](const FMGPlaylist& P) { return P.PlaylistID == PlaylistID; });
	if (Playlist)
		Playlist->TrackIDs.AddUnique(TrackID);
}

void UMGMusicSubsystem::RemoveFromPlaylist(const FString& PlaylistID, FName TrackID)
{
	FMGPlaylist* Playlist = Playlists.FindByPredicate([&PlaylistID](const FMGPlaylist& P) { return P.PlaylistID == PlaylistID; });
	if (Playlist)
		Playlist->TrackIDs.Remove(TrackID);
}

void UMGMusicSubsystem::DeletePlaylist(const FString& PlaylistID)
{
	Playlists.RemoveAll([&PlaylistID](const FMGPlaylist& P) { return P.PlaylistID == PlaylistID && P.bIsUserCreated; });
}

void UMGMusicSubsystem::PlayPlaylist(const FString& PlaylistID)
{
	FMGPlaylist* Playlist = Playlists.FindByPredicate([&PlaylistID](const FMGPlaylist& P) { return P.PlaylistID == PlaylistID; });
	if (Playlist)
	{
		CurrentQueue = Playlist->TrackIDs;
		CurrentTrackIndex = 0;
		if (MusicSettings.bShuffle) ShuffleQueue();
		Play();
	}
}

void UMGMusicSubsystem::SetMusicContext(EMGMusicContext Context)
{
	if (CurrentContext == Context) return;

	CurrentContext = Context;
	OnMusicContextChanged.Broadcast(Context);

	if (MusicSettings.bDynamicMusic)
	{
		if (FName* StationID = MusicSettings.ContextStations.Find(Context))
		{
			SetStation(*StationID);
		}
	}
}

void UMGMusicSubsystem::SetContextStation(EMGMusicContext Context, FName StationID)
{
	MusicSettings.ContextStations.Add(Context, StationID);
}

void UMGMusicSubsystem::SetMusicSettings(const FMGMusicSettings& Settings)
{
	MusicSettings = Settings;
}

void UMGMusicSubsystem::SetVolume(float Volume)
{
	MusicSettings.MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (MusicAudioComponent)
		MusicAudioComponent->SetVolumeMultiplier(MusicSettings.MusicVolume);
}

void UMGMusicSubsystem::SetShuffle(bool bEnabled)
{
	MusicSettings.bShuffle = bEnabled;
	if (bEnabled) ShuffleQueue();
}

void UMGMusicSubsystem::SetRepeat(bool bEnabled)
{
	MusicSettings.bRepeat = bEnabled;
}

float UMGMusicSubsystem::GetCurrentBPM() const
{
	if (FMGMusicTrack* Track = const_cast<UMGMusicSubsystem*>(this)->FindTrack(CurrentTrackID))
		return Track->BPM;
	return 120.0f;
}

float UMGMusicSubsystem::GetTimeSinceLastBeat() const
{
	if (UWorld* World = const_cast<UMGMusicSubsystem*>(this)->GetWorld())
		return World->GetTimeSeconds() - LastBeatTime;
	return 0.0f;
}

float UMGMusicSubsystem::GetBeatProgress() const
{
	float BPM = GetCurrentBPM();
	if (BPM <= 0.0f)
	{
		return 0.0f;
	}
	float BeatInterval = 60.0f / BPM;
	return GetTimeSinceLastBeat() / BeatInterval;
}

void UMGMusicSubsystem::InitializeMusicLibrary()
{
	// Mock track data
	TArray<TPair<FString, FString>> TrackData = {
		{TEXT("Midnight Drive"), TEXT("Neon Pulse")},
		{TEXT("Street Lights"), TEXT("Chrome Dreams")},
		{TEXT("Velocity"), TEXT("Bass Cannon")},
		{TEXT("Downtown Drift"), TEXT("Synth Riders")},
		{TEXT("Night Cruise"), TEXT("Retro Wave")},
		{TEXT("Full Throttle"), TEXT("Electric Storm")},
		{TEXT("Urban Chase"), TEXT("Beat Machine")},
		{TEXT("Turbo Mode"), TEXT("Future Sound")}
	};

	int32 Index = 0;
	for (const auto& Data : TrackData)
	{
		FMGMusicTrack Track;
		Track.TrackID = FName(*FString::Printf(TEXT("Track_%02d"), Index));
		Track.Title = FText::FromString(Data.Key);
		Track.Artist = FText::FromString(Data.Value);
		Track.Genre = (EMGMusicGenre)(Index % 5);
		Track.Duration = 180.0f + FMath::RandRange(0, 120);
		Track.BPM = 120.0f + FMath::RandRange(-20, 40);
		MusicTracks.Add(Track);
		Index++;
	}
}

void UMGMusicSubsystem::InitializeRadioStations()
{
	FMGRadioStation Synthwave;
	Synthwave.StationID = FName(TEXT("Station_Synthwave"));
	Synthwave.StationName = FText::FromString(TEXT("Neon FM"));
	Synthwave.Genre = EMGMusicGenre::Synthwave;
	Synthwave.StationColor = FLinearColor(1.0f, 0.0f, 0.5f);
	for (const FMGMusicTrack& Track : MusicTracks)
		if (Track.Genre == EMGMusicGenre::Synthwave || Track.Genre == EMGMusicGenre::Electronic)
			Synthwave.TrackIDs.Add(Track.TrackID);
	RadioStations.Add(Synthwave);

	FMGRadioStation AllHits;
	AllHits.StationID = FName(TEXT("Station_AllHits"));
	AllHits.StationName = FText::FromString(TEXT("Midnight Radio"));
	AllHits.Genre = EMGMusicGenre::Mixed;
	AllHits.StationColor = FLinearColor(0.0f, 0.5f, 1.0f);
	for (const FMGMusicTrack& Track : MusicTracks)
		AllHits.TrackIDs.Add(Track.TrackID);
	RadioStations.Add(AllHits);

	CurrentStationID = AllHits.StationID;
}

void UMGMusicSubsystem::BuildQueue()
{
	if (FMGRadioStation* Station = FindStation(CurrentStationID))
	{
		CurrentQueue = Station->TrackIDs;
		if (MusicSettings.bShuffle) ShuffleQueue();
	}
}

void UMGMusicSubsystem::ShuffleQueue()
{
	for (int32 i = CurrentQueue.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		CurrentQueue.Swap(i, j);
	}
}

void UMGMusicSubsystem::CrossfadeToTrack(FName TrackID)
{
	// Would implement crossfade with two audio components
	PlayTrack(TrackID);
}

void UMGMusicSubsystem::OnTrackFinished()
{
	NextTrack();
}

void UMGMusicSubsystem::BeatTick()
{
	if (UWorld* World = GetWorld())
		LastBeatTime = World->GetTimeSeconds();
	BeatCounter++;
	OnMusicBeat.Broadcast(GetCurrentBPM(), BeatCounter);
}

FMGMusicTrack* UMGMusicSubsystem::FindTrack(FName TrackID)
{
	return MusicTracks.FindByPredicate([TrackID](const FMGMusicTrack& T) { return T.TrackID == TrackID; });
}

FMGRadioStation* UMGMusicSubsystem::FindStation(FName StationID)
{
	return RadioStations.FindByPredicate([StationID](const FMGRadioStation& S) { return S.StationID == StationID; });
}

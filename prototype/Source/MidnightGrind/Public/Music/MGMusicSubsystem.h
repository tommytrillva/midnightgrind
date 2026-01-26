// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMusicSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGMusicGenre : uint8
{
	Electronic		UMETA(DisplayName = "Electronic"),
	HipHop			UMETA(DisplayName = "Hip-Hop"),
	Rock			UMETA(DisplayName = "Rock"),
	Synthwave		UMETA(DisplayName = "Synthwave"),
	DrumAndBass		UMETA(DisplayName = "Drum & Bass"),
	LoFi			UMETA(DisplayName = "Lo-Fi"),
	Classical		UMETA(DisplayName = "Classical"),
	Mixed			UMETA(DisplayName = "Mixed")
};

UENUM(BlueprintType)
enum class EMGMusicContext : uint8
{
	Menu			UMETA(DisplayName = "Menu"),
	Garage			UMETA(DisplayName = "Garage"),
	PreRace			UMETA(DisplayName = "Pre-Race"),
	Racing			UMETA(DisplayName = "Racing"),
	Victory			UMETA(DisplayName = "Victory"),
	Defeat			UMETA(DisplayName = "Defeat"),
	PhotoMode		UMETA(DisplayName = "Photo Mode"),
	Editor			UMETA(DisplayName = "Editor")
};

USTRUCT(BlueprintType)
struct FMGMusicTrack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TrackID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Title;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Artist;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Album;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGMusicGenre Genre;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BPM = 120.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* Audio = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* AlbumArt = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsUnlocked = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsFavorite = false;
};

USTRUCT(BlueprintType)
struct FMGRadioStation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName StationID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText StationName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGMusicGenre Genre;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> TrackIDs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* StationLogo = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor StationColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsUnlocked = true;
};

USTRUCT(BlueprintType)
struct FMGPlaylist
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString PlaylistID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText PlaylistName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> TrackIDs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsUserCreated = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FDateTime CreatedDate;
};

USTRUCT(BlueprintType)
struct FMGMusicSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MusicVolume = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDynamicMusic = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShuffle = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRepeat = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCrossfade = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float CrossfadeDuration = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowNowPlaying = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EMGMusicContext, FName> ContextStations;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackChanged, const FMGMusicTrack&, NewTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStationChanged, const FMGRadioStation&, NewStation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicContextChanged, EMGMusicContext, NewContext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMusicBeat, float, BPM, int32, BeatNumber);

UCLASS()
class MIDNIGHTGRIND_API UMGMusicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable) FOnTrackChanged OnTrackChanged;
	UPROPERTY(BlueprintAssignable) FOnStationChanged OnStationChanged;
	UPROPERTY(BlueprintAssignable) FOnMusicContextChanged OnMusicContextChanged;
	UPROPERTY(BlueprintAssignable) FOnMusicBeat OnMusicBeat;

	// Playback Control
	UFUNCTION(BlueprintCallable) void Play();
	UFUNCTION(BlueprintCallable) void Pause();
	UFUNCTION(BlueprintCallable) void Stop();
	UFUNCTION(BlueprintCallable) void NextTrack();
	UFUNCTION(BlueprintCallable) void PreviousTrack();
	UFUNCTION(BlueprintCallable) void Seek(float PositionSeconds);
	UFUNCTION(BlueprintPure) bool IsPlaying() const { return bIsPlaying; }
	UFUNCTION(BlueprintPure) float GetPlaybackPosition() const;
	UFUNCTION(BlueprintPure) FMGMusicTrack GetCurrentTrack() const;

	// Station Control
	UFUNCTION(BlueprintCallable) void SetStation(FName StationID);
	UFUNCTION(BlueprintCallable) void NextStation();
	UFUNCTION(BlueprintCallable) void PreviousStation();
	UFUNCTION(BlueprintPure) FMGRadioStation GetCurrentStation() const;
	UFUNCTION(BlueprintPure) TArray<FMGRadioStation> GetAllStations() const { return RadioStations; }
	UFUNCTION(BlueprintPure) TArray<FMGRadioStation> GetUnlockedStations() const;

	// Track Control
	UFUNCTION(BlueprintCallable) void PlayTrack(FName TrackID);
	UFUNCTION(BlueprintPure) TArray<FMGMusicTrack> GetAllTracks() const { return MusicTracks; }
	UFUNCTION(BlueprintPure) TArray<FMGMusicTrack> GetTracksByGenre(EMGMusicGenre Genre) const;
	UFUNCTION(BlueprintPure) TArray<FMGMusicTrack> GetFavoriteTracks() const;
	UFUNCTION(BlueprintCallable) void ToggleFavorite(FName TrackID);

	// Playlists
	UFUNCTION(BlueprintCallable) FString CreatePlaylist(FText Name);
	UFUNCTION(BlueprintCallable) void AddToPlaylist(const FString& PlaylistID, FName TrackID);
	UFUNCTION(BlueprintCallable) void RemoveFromPlaylist(const FString& PlaylistID, FName TrackID);
	UFUNCTION(BlueprintCallable) void DeletePlaylist(const FString& PlaylistID);
	UFUNCTION(BlueprintCallable) void PlayPlaylist(const FString& PlaylistID);
	UFUNCTION(BlueprintPure) TArray<FMGPlaylist> GetPlaylists() const { return Playlists; }

	// Context/Dynamic
	UFUNCTION(BlueprintCallable) void SetMusicContext(EMGMusicContext Context);
	UFUNCTION(BlueprintPure) EMGMusicContext GetCurrentContext() const { return CurrentContext; }
	UFUNCTION(BlueprintCallable) void SetContextStation(EMGMusicContext Context, FName StationID);

	// Settings
	UFUNCTION(BlueprintCallable) void SetMusicSettings(const FMGMusicSettings& Settings);
	UFUNCTION(BlueprintPure) FMGMusicSettings GetMusicSettings() const { return MusicSettings; }
	UFUNCTION(BlueprintCallable) void SetVolume(float Volume);
	UFUNCTION(BlueprintCallable) void SetShuffle(bool bEnabled);
	UFUNCTION(BlueprintCallable) void SetRepeat(bool bEnabled);

	// Beat Detection
	UFUNCTION(BlueprintPure) float GetCurrentBPM() const;
	UFUNCTION(BlueprintPure) float GetTimeSinceLastBeat() const;
	UFUNCTION(BlueprintPure) float GetBeatProgress() const;

protected:
	UPROPERTY() TArray<FMGMusicTrack> MusicTracks;
	UPROPERTY() TArray<FMGRadioStation> RadioStations;
	UPROPERTY() TArray<FMGPlaylist> Playlists;
	UPROPERTY() FMGMusicSettings MusicSettings;
	UPROPERTY() FName CurrentStationID;
	UPROPERTY() FName CurrentTrackID;
	UPROPERTY() int32 CurrentTrackIndex = 0;
	UPROPERTY() TArray<FName> CurrentQueue;
	UPROPERTY() bool bIsPlaying = false;
	UPROPERTY() float PlaybackPosition = 0.0f;
	UPROPERTY() EMGMusicContext CurrentContext = EMGMusicContext::Menu;
	UPROPERTY() UAudioComponent* MusicAudioComponent = nullptr;
	UPROPERTY() float LastBeatTime = 0.0f;
	UPROPERTY() int32 BeatCounter = 0;

	FTimerHandle BeatTimerHandle;

	void InitializeMusicLibrary();
	void InitializeRadioStations();
	void BuildQueue();
	void ShuffleQueue();
	void CrossfadeToTrack(FName TrackID);
	void OnTrackFinished();
	void BeatTick();
	FMGMusicTrack* FindTrack(FName TrackID);
	FMGRadioStation* FindStation(FName StationID);
};

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundBase.h"
#include "MGMusicManager.generated.h"

class UAudioComponent;

UENUM(BlueprintType)
enum class EMGMusicState : uint8
{
	Silent,
	MainMenu,
	Garage,
	Lobby,
	Countdown,
	RacingLow,
	RacingMedium,
	RacingHigh,
	FinalLap,
	Victory,
	Defeat,
	Results,
	Cutscene
};

UENUM(BlueprintType)
enum class EMGMusicLayer : uint8
{
	Base,
	Melody,
	Synths,
	Bass,
	Percussion,
	Vocals,
	Stinger
};

USTRUCT(BlueprintType)
struct FMGMusicTrack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Artist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Genre;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BPM = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SoundAsset;

	UPROPERTY(BlueprintReadOnly)
	bool bUnlocked = true;

	UPROPERTY(BlueprintReadOnly)
	int32 PlayCount = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bFavorite = false;
};

USTRUCT(BlueprintType)
struct FMGPlaylist
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlaylistID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> TrackIDs;

	UPROPERTY(BlueprintReadWrite)
	bool bShuffle = true;

	UPROPERTY(BlueprintReadWrite)
	bool bRepeat = true;
};

USTRUCT(BlueprintType)
struct FMGMusicEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName EventType;

	UPROPERTY(BlueprintReadWrite)
	float IntensityModifier = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	bool bTriggerStinger = false;

	UPROPERTY(BlueprintReadWrite)
	FName StingerID;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicStateChanged, EMGMusicState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackChanged, const FMGMusicTrack&, NewTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntensityChanged, float, NewIntensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeat);

UCLASS()
class MIDNIGHTGRIND_API UMGMusicManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Music")
	void SetMusicState(EMGMusicState NewState);

	UFUNCTION(BlueprintPure, Category = "Music")
	EMGMusicState GetMusicState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Music")
	void SetRaceIntensity(float Intensity);

	UFUNCTION(BlueprintPure, Category = "Music")
	float GetRaceIntensity() const { return CurrentIntensity; }

	UFUNCTION(BlueprintCallable, Category = "Music")
	void TriggerMusicEvent(const FMGMusicEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void PlayTrack(FName TrackID);

	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void PlayNext();

	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void PlayPrevious();

	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void Pause();

	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void Resume();

	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void Stop();

	UFUNCTION(BlueprintPure, Category = "Music|Playback")
	bool IsPlaying() const { return bIsPlaying; }

	UFUNCTION(BlueprintPure, Category = "Music|Playback")
	FMGMusicTrack GetCurrentTrack() const { return CurrentTrack; }

	UFUNCTION(BlueprintCallable, Category = "Music|Volume")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Music|Volume")
	float GetMusicVolume() const { return MusicVolume; }

	UFUNCTION(BlueprintCallable, Category = "Music|Volume")
	void SetLayerVolume(EMGMusicLayer Layer, float Volume);

	UFUNCTION(BlueprintCallable, Category = "Music|Volume")
	void FadeToVolume(float TargetVolume, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Music|Volume")
	void DuckMusic(float DuckAmount, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Music|Playlist")
	void SetPlaylist(FName PlaylistID);

	UFUNCTION(BlueprintPure, Category = "Music|Playlist")
	FMGPlaylist GetCurrentPlaylist() const { return CurrentPlaylist; }

	UFUNCTION(BlueprintPure, Category = "Music|Playlist")
	TArray<FMGPlaylist> GetAllPlaylists() const;

	UFUNCTION(BlueprintCallable, Category = "Music|Playlist")
	void SetShuffle(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Music|Playlist")
	void SetRepeat(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Music|Library")
	TArray<FMGMusicTrack> GetAllTracks() const;

	UFUNCTION(BlueprintPure, Category = "Music|Library")
	TArray<FMGMusicTrack> GetTracksByGenre(FName Genre) const;

	UFUNCTION(BlueprintCallable, Category = "Music|Library")
	void ToggleFavorite(FName TrackID);

	UFUNCTION(BlueprintPure, Category = "Music|Library")
	TArray<FMGMusicTrack> GetFavorites() const;

	UFUNCTION(BlueprintPure, Category = "Music|Beat")
	float GetTimeToNextBeat() const;

	UFUNCTION(BlueprintPure, Category = "Music|Beat")
	float GetCurrentBPM() const;

	UFUNCTION(BlueprintPure, Category = "Music|Beat")
	bool IsOnBeat(float Tolerance = 0.1f) const;

	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnMusicStateChanged OnMusicStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnTrackChanged OnTrackChanged;

	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnIntensityChanged OnIntensityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnBeat OnBeat;

protected:
	void OnTick();
	void UpdateIntensityMixing();
	void CrossfadeTo(FName TrackID, float Duration = 1.0f);
	FName GetTrackForState(EMGMusicState State) const;
	void InitializeDefaultTracks();
	void InitializeDefaultPlaylists();
	void UpdateBeatTracking(float DeltaTime);

private:
	EMGMusicState CurrentState = EMGMusicState::Silent;
	float CurrentIntensity = 0.5f;
	float TargetIntensity = 0.5f;
	FMGMusicTrack CurrentTrack;
	FMGPlaylist CurrentPlaylist;
	int32 PlaylistIndex = 0;
	bool bIsPlaying = false;
	float PlaybackPosition = 0.0f;
	float MusicVolume = 0.8f;
	TMap<EMGMusicLayer, float> LayerVolumes;
	TMap<FName, FMGMusicTrack> TrackLibrary;
	TMap<FName, FMGPlaylist> Playlists;
	float BeatAccumulator = 0.0f;
	float SecondsPerBeat = 0.5f;
	int32 BeatCount = 0;
	bool bFading = false;
	float FadeStartVolume = 0.0f;
	float FadeTargetVolume = 0.0f;
	float FadeDuration = 0.0f;
	float FadeElapsed = 0.0f;
	bool bDucking = false;
	float DuckAmount = 0.0f;
	float DuckDuration = 0.0f;
	float DuckElapsed = 0.0f;
	FTimerHandle TickTimer;
	static constexpr float IntensitySmoothRate = 2.0f;
};

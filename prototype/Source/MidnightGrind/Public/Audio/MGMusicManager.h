// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGMusicManager.h
 * @brief Dynamic Music System with Adaptive Racing Soundtrack
 *
 * @section overview Overview
 * The Music Manager handles all background music in Midnight Grind, including
 * adaptive soundtrack that responds to gameplay intensity, playlist management,
 * beat tracking for synchronized visual effects, and smooth transitions between
 * game states.
 *
 * @section beginners Key Concepts for Beginners
 *
 * @subsection adaptive_music What is Adaptive Music?
 * Unlike static music that plays the same way every time, adaptive music
 * changes based on what's happening in the game:
 * - Cruising at low speed: Chill, ambient mix
 * - Racing at high speed: Energetic, full instrumentation
 * - Final lap: More intense, building tension
 * - Victory: Triumphant music
 *
 * This is achieved through "intensity" values that blend between music layers.
 *
 * @subsection music_states Music States (EMGMusicState)
 * The manager operates in distinct states:
 * - Silent: No music playing
 * - MainMenu: Menu theme
 * - Garage: Relaxed customization music
 * - Lobby: Pre-race waiting music
 * - Countdown: Building tension before start
 * - RacingLow/Medium/High: Intensity-based racing music
 * - FinalLap: Heightened intensity for last lap
 * - Victory/Defeat: Race outcome stingers
 * - Results: Post-race summary music
 * - Cutscene: Story/cinematic music
 *
 * @subsection music_layers Music Layers (EMGMusicLayer)
 * Tracks can have multiple layers mixed together:
 * - Base: Foundation rhythm/pad
 * - Melody: Lead instruments
 * - Synths: Electronic elements
 * - Bass: Low-end groove
 * - Percussion: Drums and rhythm
 * - Vocals: Voice/choir elements
 * - Stinger: One-shot impact sounds
 *
 * Layers can be individually adjusted based on intensity.
 *
 * @subsection beat_tracking Beat Tracking
 * The system tracks BPM (beats per minute) and can:
 * - Trigger visual effects on beat
 * - Sync screen effects to music rhythm
 * - Time transitions to musical phrases
 * - Drive rhythm-based gameplay features
 *
 * @section data_structures Key Data Structures
 *
 * @subsection music_track FMGMusicTrack
 * A single music track with metadata:
 * - TrackID, DisplayName, Artist
 * - Genre (Electronic, Rock, HipHop, etc.)
 * - BPM: Beats per minute for sync
 * - Duration: Track length
 * - SoundAsset: The actual audio
 * - PlayCount, bFavorite: Player preferences
 *
 * @subsection playlist FMGPlaylist
 * An ordered collection of tracks:
 * - PlaylistID, DisplayName
 * - TrackIDs: Ordered list of tracks
 * - bShuffle: Randomize order
 * - bRepeat: Loop when finished
 *
 * @subsection music_event FMGMusicEvent
 * A gameplay event that affects music:
 * - EventType: What happened (e.g., "Collision", "NitroStart")
 * - IntensityModifier: Temporarily boost/reduce intensity
 * - bTriggerStinger: Play a one-shot sound
 * - StingerID: Which stinger to play
 *
 * @section usage Usage Examples
 *
 * @subsection state_changes Changing Music State
 * @code
 * UMGMusicManager* Music = GetGameInstance()->GetSubsystem<UMGMusicManager>();
 *
 * // Menu opened
 * Music->SetMusicState(EMGMusicState::MainMenu);
 *
 * // Entering garage
 * Music->SetMusicState(EMGMusicState::Garage);
 *
 * // Race starting
 * Music->SetMusicState(EMGMusicState::Countdown);
 *
 * // During race - let intensity control the vibe
 * Music->SetMusicState(EMGMusicState::RacingMedium);
 *
 * // Final lap!
 * Music->SetMusicState(EMGMusicState::FinalLap);
 *
 * // Race won
 * Music->SetMusicState(EMGMusicState::Victory);
 * @endcode
 *
 * @subsection intensity Racing Intensity
 * @code
 * // Update intensity based on gameplay (0.0 - 1.0)
 * // Typically called every frame or on significant events
 *
 * float Intensity = 0.0f;
 *
 * // Factor in speed (faster = more intense)
 * Intensity += FMath::GetMappedRangeValueClamped(
 *     FVector2D(50, 200),  // Speed range (km/h)
 *     FVector2D(0, 0.5f),  // Contribution to intensity
 *     CurrentSpeedKPH
 * );
 *
 * // Factor in position (close to first = more intense)
 * if (RacePosition <= 3)
 * {
 *     Intensity += 0.2f;
 * }
 *
 * // Factor in proximity to other racers
 * if (NearbyRacerCount > 0)
 * {
 *     Intensity += 0.3f;
 * }
 *
 * Music->SetRaceIntensity(FMath::Clamp(Intensity, 0.0f, 1.0f));
 * @endcode
 *
 * @subsection music_events Triggering Music Events
 * @code
 * // Collision - brief intensity spike
 * FMGMusicEvent CollisionEvent;
 * CollisionEvent.EventType = TEXT("Collision");
 * CollisionEvent.IntensityModifier = 0.2f;  // +20% intensity
 * CollisionEvent.bTriggerStinger = true;
 * CollisionEvent.StingerID = TEXT("Impact_Heavy");
 * Music->TriggerMusicEvent(CollisionEvent);
 *
 * // Nitro boost - sustained intensity
 * FMGMusicEvent NitroEvent;
 * NitroEvent.EventType = TEXT("NitroStart");
 * NitroEvent.IntensityModifier = 0.3f;
 * Music->TriggerMusicEvent(NitroEvent);
 * @endcode
 *
 * @subsection playback Playback Control
 * @code
 * // Manual track control
 * Music->PlayTrack(TEXT("Track_NightRider"));
 * Music->PlayNext();
 * Music->PlayPrevious();
 * Music->Pause();
 * Music->Resume();
 * Music->Stop();
 *
 * // Check playback state
 * if (Music->IsPlaying())
 * {
 *     FMGMusicTrack Current = Music->GetCurrentTrack();
 *     UE_LOG(LogTemp, Log, TEXT("Now playing: %s by %s"),
 *         *Current.DisplayName.ToString(),
 *         *Current.Artist.ToString());
 * }
 * @endcode
 *
 * @subsection volume Volume Control
 * @code
 * // Master volume
 * Music->SetMusicVolume(0.8f);  // 80%
 *
 * // Layer-specific volume (for mixing)
 * Music->SetLayerVolume(EMGMusicLayer::Bass, 1.2f);  // Boost bass
 * Music->SetLayerVolume(EMGMusicLayer::Vocals, 0.5f);  // Quieter vocals
 *
 * // Smooth transitions
 * Music->FadeToVolume(0.5f, 2.0f);  // Fade to 50% over 2 seconds
 *
 * // Duck music (temporarily lower for voice/sfx)
 * Music->DuckMusic(0.5f, 3.0f);  // Duck 50% for 3 seconds
 * @endcode
 *
 * @subsection playlists Playlist Management
 * @code
 * // Set active playlist
 * Music->SetPlaylist(TEXT("Playlist_Racing"));
 *
 * // Configure playback mode
 * Music->SetShuffle(true);   // Randomize order
 * Music->SetRepeat(true);    // Loop playlist
 *
 * // Get available playlists
 * TArray<FMGPlaylist> AllPlaylists = Music->GetAllPlaylists();
 *
 * // Current playlist info
 * FMGPlaylist Current = Music->GetCurrentPlaylist();
 * @endcode
 *
 * @subsection library Track Library
 * @code
 * // Browse all tracks
 * TArray<FMGMusicTrack> AllTracks = Music->GetAllTracks();
 *
 * // Filter by genre
 * TArray<FMGMusicTrack> ElectronicTracks = Music->GetTracksByGenre(TEXT("Electronic"));
 *
 * // Favorites
 * Music->ToggleFavorite(TEXT("Track_NightRider"));
 * TArray<FMGMusicTrack> Favorites = Music->GetFavorites();
 * @endcode
 *
 * @subsection beat_sync Beat Synchronization
 * @code
 * // Check if we're on a beat (for visual sync)
 * if (Music->IsOnBeat(0.05f))  // 50ms tolerance
 * {
 *     PulseUIElement();
 * }
 *
 * // Get timing info
 * float TimeToNextBeat = Music->GetTimeToNextBeat();
 * float CurrentBPM = Music->GetCurrentBPM();
 *
 * // Subscribe to beat events
 * Music->OnBeat.AddDynamic(this, &UMyClass::HandleBeat);
 * @endcode
 *
 * @section events Events/Delegates
 * - OnMusicStateChanged: Game state changed (Menu -> Racing)
 * - OnTrackChanged: New track started playing
 * - OnIntensityChanged: Racing intensity level changed
 * - OnBeat: Fired on each musical beat (for sync)
 *
 * @section intensity_mixing Intensity Mixing
 * The intensity value (0.0-1.0) affects music mix:
 * - 0.0-0.3: Low energy - emphasis on Base, ambient Synths
 * - 0.3-0.6: Medium energy - add Melody, moderate Percussion
 * - 0.6-0.8: High energy - full Percussion, prominent Bass
 * - 0.8-1.0: Maximum intensity - all layers at full, add Vocals
 *
 * The mixing happens via UpdateIntensityMixing() which adjusts layer volumes
 * based on the current intensity value smoothed over time.
 *
 * @section crossfade Crossfading
 * When changing tracks, CrossfadeTo() smoothly transitions:
 * 1. Current track fades out over duration
 * 2. New track fades in simultaneously
 * 3. Beat-aligned for seamless mixing (when BPM matches)
 *
 * @see MGVehicleSFXComponent.h For vehicle-specific sound effects
 * @see MGScreenEffectSubsystem.h For beat-synced visual effects
 */

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

// Copyright Midnight Grind. All Rights Reserved.


#pragma once
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

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundBase.h"
#include "Music/MGMusicSubsystem.h"
#include "MGMusicManager.generated.h"

class UAudioComponent;

/**
 * Music playback state.
 * 
 * Defines the current game context which determines music selection
 * and intensity behavior. The music manager transitions between states
 * with appropriate crossfades.
 */
UENUM(BlueprintType)
enum class EMGMusicState : uint8
{
	/** No music playing */
	Silent,
	/** Main menu theme */
	MainMenu,
	/** Garage/customization ambient */
	Garage,
	/** Pre-race lobby waiting */
	Lobby,
	/** Race countdown building tension */
	Countdown,
	/** Low-intensity racing (cruising) */
	RacingLow,
	/** Medium-intensity racing (competitive) */
	RacingMedium,
	/** High-intensity racing (close battle) */
	RacingHigh,
	/** Final lap heightened tension */
	FinalLap,
	/** Race victory celebration */
	Victory,
	/** Race defeat/loss */
	Defeat,
	/** Post-race results screen */
	Results,
	/** Cutscene/cinematic music */
	Cutscene
};

/**
 * Music layer type for multi-layer track mixing.
 * 
 * Tracks can have multiple layers that are mixed based on
 * intensity level, allowing dynamic soundtrack adaptation.
 */
UENUM(BlueprintType)
enum class EMGMusicLayer : uint8
{
	/** Foundation rhythm/pad layer */
	Base,
	/** Lead instruments/melody */
	Melody,
	/** Electronic/synth elements */
	Synths,
	/** Low-end bass groove */
	Bass,
	/** Drums and rhythm */
	Percussion,
	/** Voice/choir elements */
	Vocals,
	/** One-shot impact sounds */
	Stinger
};

// FMGMusicTrack - MOVED TO Music/MGMusicSubsystem.h (included above)

// FMGPlaylist - MOVED TO Music/MGMusicSubsystem.h (included above)

/**
 * Music stinger configuration.
 * 
 * Short musical phrase that plays during significant gameplay events
 * (race start, final lap, victory, etc.). Stingers add emotional
 * punctuation to key moments.
 */
USTRUCT(BlueprintType)
struct FMGMusicStinger
{
	GENERATED_BODY()

	/** Unique identifier for this stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stinger")
	FName StingerID;

	/** Sound asset to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stinger")
	USoundBase* Sound = nullptr;

	/** Volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stinger", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Volume = 1.0f;

	/** Whether to duck background music when playing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stinger")
	bool bDuckMusic = true;

	/** Amount to duck music (0-1 where 1 = full duck) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stinger", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bDuckMusic"))
	float DuckAmount = 0.5f;

	/** Duration of music duck in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stinger", meta = (ClampMin = "0.0", EditCondition = "bDuckMusic"))
	float DuckDuration = 1.0f;
};

/**
 * Gameplay event that affects music.
 * 
 * Triggered by game systems to dynamically adjust music intensity
 * or trigger stingers in response to gameplay moments.
 */
USTRUCT(BlueprintType)
struct FMGMusicEvent
{
	GENERATED_BODY()

	/** Event type identifier (e.g., "Collision", "NitroStart") */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FName EventType;

	/** Temporary intensity adjustment (-1 to +1) */
	UPROPERTY(BlueprintReadWrite, Category = "Event", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float IntensityModifier = 0.0f;

	/** Whether to play a stinger sound */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	bool bTriggerStinger = false;

	/** Stinger ID to play if bTriggerStinger is true */
	UPROPERTY(BlueprintReadWrite, Category = "Event", meta = (EditCondition = "bTriggerStinger"))
	FName StingerID;
};

/** Delegate broadcast when music state changes (e.g., Menu -> Racing) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicStateChanged, EMGMusicState, NewState);

/** Delegate broadcast when a new track starts playing */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackChanged, const FMGMusicTrack&, NewTrack);

/** Delegate broadcast when racing intensity level changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntensityChanged, float, NewIntensity);

/** Delegate broadcast on each musical beat (for visual sync) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeat);

/**
 * Dynamic Music Manager Subsystem.
 * 
 * Central management for all background music in Midnight Grind.
 * Handles adaptive soundtrack that responds to gameplay intensity,
 * playlist management, beat tracking for synchronized visual effects,
 * and smooth transitions between game states.
 * 
 * Access via: GetGameInstance()->GetSubsystem<UMGMusicManager>()
 * 
 * @see EMGMusicState for game context states
 * @see FMGMusicTrack for track configuration
 * @see FMGMusicEvent for gameplay-triggered music changes
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMusicManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ==========================================
	// STATE MANAGEMENT
	// ==========================================

	/** Set the current music state (triggers appropriate music transition) */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void SetMusicState(EMGMusicState NewState);

	/** Get the current music state */
	UFUNCTION(BlueprintPure, Category = "Music")
	EMGMusicState GetMusicState() const { return CurrentState; }

	/** Set racing intensity (0-1) to control adaptive music mix */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void SetRaceIntensity(float Intensity);

	/** Get current racing intensity level */
	UFUNCTION(BlueprintPure, Category = "Music")
	float GetRaceIntensity() const { return CurrentIntensity; }

	/** Trigger a music event (intensity spike, stinger, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void TriggerMusicEvent(const FMGMusicEvent& Event);

	// ==========================================
	// PLAYBACK CONTROL
	// ==========================================

	/** Play a specific track by ID */
	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void PlayTrack(FName TrackID);

	/** Skip to next track in playlist */
	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void PlayNext();

	/** Go back to previous track in playlist */
	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void PlayPrevious();

	/** Pause music playback */
	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void Pause();

	/** Resume paused playback */
	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void Resume();

	/** Stop music playback completely */
	UFUNCTION(BlueprintCallable, Category = "Music|Playback")
	void Stop();

	/** Check if music is currently playing */
	UFUNCTION(BlueprintPure, Category = "Music|Playback")
	bool IsPlaying() const { return bIsPlaying; }

	/** Get the currently playing track info */
	UFUNCTION(BlueprintPure, Category = "Music|Playback")
	FMGMusicTrack GetCurrentTrack() const { return CurrentTrack; }

	// ==========================================
	// VOLUME CONTROL
	// ==========================================

	/** Set master music volume (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Music|Volume")
	void SetMusicVolume(float Volume);

	/** Get current master music volume */
	UFUNCTION(BlueprintPure, Category = "Music|Volume")
	float GetMusicVolume() const { return MusicVolume; }

	/** Set volume for a specific music layer */
	UFUNCTION(BlueprintCallable, Category = "Music|Volume")
	void SetLayerVolume(EMGMusicLayer Layer, float Volume);

	/** Smoothly fade to a target volume over duration */
	UFUNCTION(BlueprintCallable, Category = "Music|Volume")
	void FadeToVolume(float TargetVolume, float Duration);

	/** Temporarily reduce music volume (for voice/SFX priority) */
	UFUNCTION(BlueprintCallable, Category = "Music|Volume")
	void DuckMusic(float DuckAmount, float Duration);

	// ==========================================
	// PLAYLIST MANAGEMENT
	// ==========================================

	/** Set active playlist by ID */
	UFUNCTION(BlueprintCallable, Category = "Music|Playlist")
	void SetPlaylist(FName PlaylistID);

	/** Get current playlist configuration */
	UFUNCTION(BlueprintPure, Category = "Music|Playlist")
	FMGPlaylist GetCurrentPlaylist() const { return CurrentPlaylist; }

	/** Get all available playlists */
	UFUNCTION(BlueprintPure, Category = "Music|Playlist")
	TArray<FMGPlaylist> GetAllPlaylists() const;

	/** Enable/disable shuffle mode for current playlist */
	UFUNCTION(BlueprintCallable, Category = "Music|Playlist")
	void SetShuffle(bool bEnabled);

	/** Enable/disable repeat mode for current playlist */
	UFUNCTION(BlueprintCallable, Category = "Music|Playlist")
	void SetRepeat(bool bEnabled);

	// ==========================================
	// TRACK LIBRARY
	// ==========================================

	/** Get all tracks in the library */
	UFUNCTION(BlueprintPure, Category = "Music|Library")
	TArray<FMGMusicTrack> GetAllTracks() const;

	/** Get tracks filtered by genre */
	UFUNCTION(BlueprintPure, Category = "Music|Library")
	TArray<FMGMusicTrack> GetTracksByGenre(FName Genre) const;

	/** Toggle favorite status for a track */
	UFUNCTION(BlueprintCallable, Category = "Music|Library")
	void ToggleFavorite(FName TrackID);

	/** Get all favorited tracks */
	UFUNCTION(BlueprintPure, Category = "Music|Library")
	TArray<FMGMusicTrack> GetFavorites() const;

	// ==========================================
	// BEAT SYNCHRONIZATION
	// ==========================================

	/** Get time until next beat in seconds */
	UFUNCTION(BlueprintPure, Category = "Music|Beat")
	float GetTimeToNextBeat() const;

	/** Get current track BPM */
	UFUNCTION(BlueprintPure, Category = "Music|Beat")
	float GetCurrentBPM() const;

	/** Check if we're currently on a beat (within tolerance) */
	UFUNCTION(BlueprintPure, Category = "Music|Beat")
	bool IsOnBeat(float Tolerance = 0.1f) const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Broadcast when music state changes */
	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnMusicStateChanged OnMusicStateChanged;

	/** Broadcast when track changes */
	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnTrackChanged OnTrackChanged;

	/** Broadcast when intensity changes */
	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnIntensityChanged OnIntensityChanged;

	/** Broadcast on each musical beat */
	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnBeat OnBeat;

protected:
	void OnTick();
	void UpdateIntensityMixing();
	void CrossfadeTo(FName TrackID, float Duration = 1.0f);
	FName GetTrackForState(EMGMusicState State) const;
	void InitializeDefaultTracks();
	void InitializeDefaultPlaylists();
	void UpdateBeatTracking(float MGDeltaTime);

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

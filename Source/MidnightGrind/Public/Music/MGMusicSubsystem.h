// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGMusicSubsystem.h
 * @brief Music and Radio Station Subsystem for Midnight Grind
 *
 * This file defines the music management system that handles background music,
 * radio stations, playlists, and dynamic music features. The system provides
 * an immersive audio experience similar to GTA-style radio with genre-based
 * stations and context-aware music selection.
 *
 * @section music_features Key Features
 *
 * - **Radio Stations**: Genre-based stations with curated track lists
 * - **Music Library**: Central repository of all tracks with metadata
 * - **Playlists**: User-created and system playlists for custom listening
 * - **Context-Aware Music**: Automatically adapts to game situations (menu, racing, victory)
 * - **Beat Detection**: Provides BPM data for syncing visual effects to music
 * - **Crossfading**: Smooth transitions between tracks
 *
 * @section music_architecture Architecture
 *
 * The music subsystem is a GameInstanceSubsystem that persists across level loads.
 * It works independently but coordinates with UMGAudioSubsystem for volume control.
 *
 * @section music_usage Basic Usage
 *
 * @code
 * // Get the music subsystem
 * UMGMusicSubsystem* MusicSys = GetGameInstance()->GetSubsystem<UMGMusicSubsystem>();
 *
 * // Switch to a radio station
 * MusicSys->SetStation(FName("StationSynthwave"));
 *
 * // Control playback
 * MusicSys->Play();
 * MusicSys->NextTrack();
 *
 * // React to game events
 * MusicSys->SetMusicContext(EMGMusicContext::Racing);
 * @endcode
 *
 * @see UMGAudioSubsystem
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMusicSubsystem.generated.h"

// Forward declarations
class UAudioComponent;

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Music genre classification for tracks and stations
 *
 * Used to categorize tracks and radio stations. Players can filter
 * by genre when browsing music or selecting stations.
 */
UENUM(BlueprintType)
enum class EMGMusicGenre : uint8
{
	Electronic		UMETA(DisplayName = "Electronic"),   ///< EDM, house, techno
	HipHop			UMETA(DisplayName = "Hip-Hop"),      ///< Hip-hop, rap, trap
	Rock			UMETA(DisplayName = "Rock"),         ///< Rock, alternative, metal
	Synthwave		UMETA(DisplayName = "Synthwave"),    ///< Retro synth, outrun, vaporwave
	DrumAndBass		UMETA(DisplayName = "Drum & Bass"),  ///< DnB, jungle, breakbeat
	LoFi			UMETA(DisplayName = "Lo-Fi"),        ///< Lo-fi hip hop, chillhop
	Classical		UMETA(DisplayName = "Classical"),    ///< Orchestral, classical crossover
	Mixed			UMETA(DisplayName = "Mixed")         ///< Multi-genre stations
};

/**
 * @brief Game contexts that influence music selection
 *
 * The music system can automatically switch stations or playlists
 * based on the current game context. Each context can have a preferred
 * station configured in the music settings.
 */
UENUM(BlueprintType)
enum class EMGMusicContext : uint8
{
	Menu			UMETA(DisplayName = "Menu"),         ///< Main menu and settings screens
	Garage			UMETA(DisplayName = "Garage"),       ///< Vehicle customization
	PreRace			UMETA(DisplayName = "Pre-Race"),     ///< Race lobby, countdown
	Racing			UMETA(DisplayName = "Racing"),       ///< Active race gameplay
	Victory			UMETA(DisplayName = "Victory"),      ///< Post-race win celebration
	Defeat			UMETA(DisplayName = "Defeat"),       ///< Post-race loss
	PhotoMode		UMETA(DisplayName = "Photo Mode"),   ///< Photo mode (typically quieter music)
	Editor			UMETA(DisplayName = "Editor")        ///< Track/livery editor
};

// ============================================================================
// DATA STRUCTURES - TRACKS
// ============================================================================

/**
 * @brief Represents a single music track with full metadata
 *
 * Contains all information about a track including display metadata,
 * audio asset reference, and player-specific data like favorites.
 */
USTRUCT(BlueprintType)
struct FMGMusicTrack
{
	GENERATED_BODY()

	/// Unique identifier for this track (e.g., "Track_NightDrive_01")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName TrackID;

	/// Display title shown to players
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Title;

	/// Artist/band name
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Artist;

	/// Album name (optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Album;

	/// Genre classification
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGMusicGenre Genre;

	/// Track duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration = 0.0f;

	/// Beats per minute (used for beat sync features)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BPM = 120.0f;

	/// The actual audio asset to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* Audio = nullptr;

	/// Album artwork for UI display
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* AlbumArt = nullptr;

	/// Whether the player has unlocked this track
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsUnlocked = true;

	/// Whether the player has marked this as a favorite
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsFavorite = false;
};

// ============================================================================
// DATA STRUCTURES - RADIO STATIONS
// ============================================================================

/**
 * @brief Represents an in-game radio station
 *
 * Radio stations are curated collections of tracks with a specific theme
 * or genre. Players can tune between stations during gameplay.
 */
USTRUCT(BlueprintType)
struct FMGRadioStation
{
	GENERATED_BODY()

	/// Unique identifier (e.g., "Station_MidnightFM")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName StationID;

	/// Display name shown in UI (e.g., "Midnight FM")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText StationName;

	/// Short description of the station's style
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;

	/// Primary genre for this station
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGMusicGenre Genre;

	/// List of track IDs that belong to this station
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> TrackIDs;

	/// Station logo for UI display
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* StationLogo = nullptr;

	/// Theme color for UI styling
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor StationColor = FLinearColor::White;

	/// Whether this station is available to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsUnlocked = true;
};

// ============================================================================
// DATA STRUCTURES - PLAYLISTS
// ============================================================================

/**
 * @brief User-created or system-generated playlist
 *
 * Playlists allow players to create custom collections of their favorite
 * tracks. The system can also generate automatic playlists (e.g., "Recently Played").
 */
USTRUCT(BlueprintType)
struct FMGPlaylist
{
	GENERATED_BODY()

	/// Unique identifier for this playlist
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString PlaylistID;

	/// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText PlaylistName;

	/// Ordered list of track IDs in this playlist
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> TrackIDs;

	/// True if player created this playlist; false for system playlists
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsUserCreated = false;

	/// When the playlist was created
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FDateTime CreatedDate;
};

// ============================================================================
// DATA STRUCTURES - SETTINGS
// ============================================================================

/**
 * @brief Music playback settings and preferences
 *
 * Contains all user preferences for music playback behavior.
 * Saved with player preferences.
 */
USTRUCT(BlueprintType)
struct FMGMusicSettings
{
	GENERATED_BODY()

	/// Music volume level (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MusicVolume = 0.8f;

	/// Enable context-aware music switching
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDynamicMusic = true;

	/// Shuffle tracks instead of playing in order
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShuffle = true;

	/// Repeat current track/playlist when finished
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRepeat = false;

	/// Enable crossfading between tracks
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCrossfade = true;

	/// Duration of crossfade in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float CrossfadeDuration = 3.0f;

	/// Show "Now Playing" UI notification on track change
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowNowPlaying = true;

	/// Preferred station for each game context
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EMGMusicContext, FName> ContextStations;
};

// ============================================================================
// DELEGATES
// ============================================================================

/** @brief Broadcast when the current track changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackChanged, const FMGMusicTrack&, NewTrack);

/** @brief Broadcast when the radio station changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStationChanged, const FMGRadioStation&, NewStation);

/** @brief Broadcast when the music context changes (e.g., entering a race) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicContextChanged, EMGMusicContext, NewContext);

/** @brief Broadcast on each music beat (for syncing visual effects) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMusicBeat, float, BPM, int32, BeatNumber);

// ============================================================================
// MUSIC SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Music and radio station management subsystem
 *
 * Handles all music playback, radio stations, playlists, and context-aware
 * music features. Provides a rich music experience with beat detection for
 * syncing game effects to the music.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMusicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initialize the music subsystem
	 * Loads music library, initializes radio stations, and restores saved settings.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief Clean up when subsystem is destroyed
	 * Stops playback and saves current state.
	 */
	virtual void Deinitialize() override;

	// ==========================================
	// EVENT DELEGATES
	// ==========================================
	/** @name Events
	 *  Delegates that broadcast music state changes. Bind to these in UI widgets
	 *  or gameplay systems that need to react to music changes.
	 */
	///@{

	/// Fires when a new track starts playing
	UPROPERTY(BlueprintAssignable) FOnTrackChanged OnTrackChanged;

	/// Fires when the radio station is switched
	UPROPERTY(BlueprintAssignable) FOnStationChanged OnStationChanged;

	/// Fires when the game context changes (affects which music plays)
	UPROPERTY(BlueprintAssignable) FOnMusicContextChanged OnMusicContextChanged;

	/// Fires on each beat of the music (for visual sync effects)
	UPROPERTY(BlueprintAssignable) FOnMusicBeat OnMusicBeat;

	///@}

	// ==========================================
	// PLAYBACK CONTROL
	// ==========================================
	/** @name Playback Control
	 *  Basic media player controls for music playback.
	 */
	///@{

	/** @brief Start or resume music playback */
	UFUNCTION(BlueprintCallable) void Play();

	/** @brief Pause music playback */
	UFUNCTION(BlueprintCallable) void Pause();

	/** @brief Stop playback and reset to beginning */
	UFUNCTION(BlueprintCallable) void Stop();

	/** @brief Skip to the next track in the queue */
	UFUNCTION(BlueprintCallable) void NextTrack();

	/** @brief Go back to the previous track */
	UFUNCTION(BlueprintCallable) void PreviousTrack();

	/**
	 * @brief Seek to a specific position in the current track
	 * @param PositionSeconds Position in seconds from track start
	 */
	UFUNCTION(BlueprintCallable) void Seek(float PositionSeconds);

	/** @brief Check if music is currently playing */
	UFUNCTION(BlueprintPure) bool IsPlaying() const { return bIsPlaying; }

	/** @brief Get current playback position in seconds */
	UFUNCTION(BlueprintPure) float GetPlaybackPosition() const;

	/** @brief Get the currently playing track */
	UFUNCTION(BlueprintPure) FMGMusicTrack GetCurrentTrack() const;

	///@}

	// ==========================================
	// STATION CONTROL
	// ==========================================
	/** @name Station Control
	 *  Functions for switching between radio stations.
	 */
	///@{

	/**
	 * @brief Switch to a specific radio station
	 * @param StationID The ID of the station to switch to
	 */
	UFUNCTION(BlueprintCallable) void SetStation(FName StationID);

	/** @brief Cycle to the next radio station */
	UFUNCTION(BlueprintCallable) void NextStation();

	/** @brief Cycle to the previous radio station */
	UFUNCTION(BlueprintCallable) void PreviousStation();

	/** @brief Get the current radio station */
	UFUNCTION(BlueprintPure) FMGRadioStation GetCurrentStation() const;

	/** @brief Get all available radio stations */
	UFUNCTION(BlueprintPure) TArray<FMGRadioStation> GetAllStations() const { return RadioStations; }

	/** @brief Get only stations the player has unlocked */
	UFUNCTION(BlueprintPure) TArray<FMGRadioStation> GetUnlockedStations() const;

	///@}

	// ==========================================
	// TRACK CONTROL
	// ==========================================
	/** @name Track Control
	 *  Functions for direct track selection and browsing.
	 */
	///@{

	/**
	 * @brief Play a specific track by ID
	 * @param TrackID The ID of the track to play
	 */
	UFUNCTION(BlueprintCallable) void PlayTrack(FName TrackID);

	/** @brief Get the complete music library */
	UFUNCTION(BlueprintPure) TArray<FMGMusicTrack> GetAllTracks() const { return MusicTracks; }

	/**
	 * @brief Filter tracks by genre
	 * @param Genre The genre to filter by
	 */
	UFUNCTION(BlueprintPure) TArray<FMGMusicTrack> GetTracksByGenre(EMGMusicGenre Genre) const;

	/** @brief Get all tracks marked as favorites */
	UFUNCTION(BlueprintPure) TArray<FMGMusicTrack> GetFavoriteTracks() const;

	/**
	 * @brief Toggle the favorite status of a track
	 * @param TrackID The track to toggle
	 */
	UFUNCTION(BlueprintCallable) void ToggleFavorite(FName TrackID);

	///@}

	// ==========================================
	// PLAYLISTS
	// ==========================================
	/** @name Playlists
	 *  Functions for creating and managing user playlists.
	 */
	///@{

	/**
	 * @brief Create a new empty playlist
	 * @param Name Display name for the playlist
	 * @return The ID of the created playlist
	 */
	UFUNCTION(BlueprintCallable) FString CreatePlaylist(FText Name);

	/**
	 * @brief Add a track to a playlist
	 * @param PlaylistID The playlist to add to
	 * @param TrackID The track to add
	 */
	UFUNCTION(BlueprintCallable) void AddToPlaylist(const FString& PlaylistID, FName TrackID);

	/**
	 * @brief Remove a track from a playlist
	 * @param PlaylistID The playlist to modify
	 * @param TrackID The track to remove
	 */
	UFUNCTION(BlueprintCallable) void RemoveFromPlaylist(const FString& PlaylistID, FName TrackID);

	/**
	 * @brief Delete an entire playlist
	 * @param PlaylistID The playlist to delete
	 */
	UFUNCTION(BlueprintCallable) void DeletePlaylist(const FString& PlaylistID);

	/**
	 * @brief Start playing a playlist
	 * @param PlaylistID The playlist to play
	 */
	UFUNCTION(BlueprintCallable) void PlayPlaylist(const FString& PlaylistID);

	/** @brief Get all playlists (user and system) */
	UFUNCTION(BlueprintPure) TArray<FMGPlaylist> GetPlaylists() const { return Playlists; }

	///@}

	// ==========================================
	// CONTEXT / DYNAMIC MUSIC
	// ==========================================
	/** @name Context / Dynamic Music
	 *  Functions for context-aware music that adapts to gameplay.
	 */
	///@{

	/**
	 * @brief Set the current music context (triggers automatic station/playlist selection)
	 * @param Context The new game context
	 */
	UFUNCTION(BlueprintCallable) void SetMusicContext(EMGMusicContext Context);

	/** @brief Get the current music context */
	UFUNCTION(BlueprintPure) EMGMusicContext GetCurrentContext() const { return CurrentContext; }

	/**
	 * @brief Assign a station to play for a specific context
	 * @param Context The game context
	 * @param StationID The station to play in that context
	 */
	UFUNCTION(BlueprintCallable) void SetContextStation(EMGMusicContext Context, FName StationID);

	///@}

	// ==========================================
	// SETTINGS
	// ==========================================
	/** @name Settings
	 *  Music playback preferences and configuration.
	 */
	///@{

	/**
	 * @brief Apply a complete settings configuration
	 * @param Settings The settings to apply
	 */
	UFUNCTION(BlueprintCallable) void SetMusicSettings(const FMGMusicSettings& Settings);

	/** @brief Get current music settings */
	UFUNCTION(BlueprintPure) FMGMusicSettings GetMusicSettings() const { return MusicSettings; }

	/**
	 * @brief Set the music volume
	 * @param Volume Volume from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable) void SetVolume(float Volume);

	/**
	 * @brief Enable or disable shuffle mode
	 * @param bEnabled True to shuffle tracks
	 */
	UFUNCTION(BlueprintCallable) void SetShuffle(bool bEnabled);

	/**
	 * @brief Enable or disable repeat mode
	 * @param bEnabled True to repeat tracks/playlist
	 */
	UFUNCTION(BlueprintCallable) void SetRepeat(bool bEnabled);

	///@}

	// ==========================================
	// BEAT DETECTION
	// ==========================================
	/** @name Beat Detection
	 *  Functions for syncing game effects to music beats.
	 *  Use these to create visual effects that pulse with the music.
	 */
	///@{

	/** @brief Get BPM of the current track */
	UFUNCTION(BlueprintPure) float GetCurrentBPM() const;

	/** @brief Get time in seconds since the last beat */
	UFUNCTION(BlueprintPure) float GetTimeSinceLastBeat() const;

	/**
	 * @brief Get progress through current beat (0.0 to 1.0)
	 * Useful for smooth animations that follow the beat.
	 */
	UFUNCTION(BlueprintPure) float GetBeatProgress() const;

	///@}

protected:
	// ==========================================
	// INTERNAL STATE - LIBRARY DATA
	// ==========================================

	/// Complete library of all available music tracks
	UPROPERTY() TArray<FMGMusicTrack> MusicTracks;

	/// All radio stations
	UPROPERTY() TArray<FMGRadioStation> RadioStations;

	/// User and system playlists
	UPROPERTY() TArray<FMGPlaylist> Playlists;

	/// Current music settings
	UPROPERTY() FMGMusicSettings MusicSettings;

	// ==========================================
	// INTERNAL STATE - PLAYBACK
	// ==========================================

	/// ID of the currently selected station
	UPROPERTY() FName CurrentStationID;

	/// ID of the currently playing track
	UPROPERTY() FName CurrentTrackID;

	/// Index into the current queue
	UPROPERTY() int32 CurrentTrackIndex = 0;

	/// Current playback queue (may be shuffled)
	UPROPERTY() TArray<FName> CurrentQueue;

	/// Is music currently playing?
	UPROPERTY() bool bIsPlaying = false;

	/// Current position in the track (seconds)
	UPROPERTY() float PlaybackPosition = 0.0f;

	/// Current game context for dynamic music
	UPROPERTY() EMGMusicContext CurrentContext = EMGMusicContext::Menu;

	/// The audio component playing the current track
	UPROPERTY() UAudioComponent* MusicAudioComponent = nullptr;

	/// Timestamp of the last beat for beat detection
	UPROPERTY() float LastBeatTime = 0.0f;

	/// Counter for beat events
	UPROPERTY() int32 BeatCounter = 0;

	/// Timer handle for beat tick callbacks
	FTimerHandle BeatTimerHandle;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** @brief Load the music library from data assets */
	void InitializeMusicLibrary();

	/** @brief Create default radio stations */
	void InitializeRadioStations();

	/** @brief Build the playback queue from current station/playlist */
	void BuildQueue();

	/** @brief Randomize the current queue order */
	void ShuffleQueue();

	/**
	 * @brief Smoothly transition to a new track
	 * @param TrackID The track to crossfade to
	 */
	void CrossfadeToTrack(FName TrackID);

	/** @brief Called when the current track finishes playing */
	void OnTrackFinished();

	/** @brief Called on timer to fire beat events */
	void BeatTick();

	/**
	 * @brief Find a track by ID
	 * @param TrackID The track to find
	 * @return Pointer to the track, or nullptr if not found
	 */
	FMGMusicTrack* FindTrack(FName TrackID);

	/**
	 * @brief Find a station by ID
	 * @param StationID The station to find
	 * @return Pointer to the station, or nullptr if not found
	 */
	FMGRadioStation* FindStation(FName StationID);
};

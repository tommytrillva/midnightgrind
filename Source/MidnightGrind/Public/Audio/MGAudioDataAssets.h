// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGAudioDataAssets.h
 * @brief Audio configuration data assets for Midnight Grind racing game.
 *
 * This file contains data asset definitions for configuring audio systems:
 * - Engine audio presets (RPM-based sound layering)
 * - Vehicle SFX presets (tires, collisions, wind noise)
 * - Music track configuration (layered/dynamic music)
 * - Music playlists (track collections with shuffle)
 * - Stinger collections (event-triggered audio cues)
 * - Master audio configuration (combines all audio systems)
 *
 * @section usage_audio Usage
 * Create audio data assets in the Unreal Editor:
 * Right-click in Content Browser > Miscellaneous > Data Asset > [Asset Type]
 *
 * The audio system uses a layered approach:
 * 1. UMGEngineAudioPresetData: One per engine type (I4, V6, V8, etc.)
 * 2. UMGVehicleSFXPresetData: One per vehicle category
 * 3. UMGMusicTrackData: One per music track
 * 4. UMGMusicPlaylistData: Groups tracks for game modes
 * 5. UMGStingerCollectionData: Event sounds for race moments
 * 6. UMGAudioConfigData: Master config combining everything
 *
 * @section dynamic_audio Dynamic Music System
 * Music tracks support multiple layers that blend based on game state:
 * - Base layer: Always playing
 * - Action layer: Fades in during intense racing
 * - Ambient layer: For menus and calm moments
 *
 * @see UMGEngineAudioComponent
 * @see UMGVehicleSFXComponent
 * @see UMGMusicManager
 */

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Audio/MGEngineAudioComponent.h"
#include "Audio/MGVehicleSFXComponent.h"
#include "Audio/MGMusicManager.h"
#include "MGAudioDataAssets.generated.h"

// ============================================================================
// ENGINE AUDIO
// ============================================================================

/**
 * Engine Audio Preset Data Asset
 *
 * Configures engine sounds for a specific vehicle/engine type.
 * The audio system layers multiple sounds based on RPM and load
 * to create realistic, dynamic engine audio.
 *
 * @section create_engine Creating Engine Presets
 * Create in Editor: Right-click > Miscellaneous > Data Asset > MGEngineAudioPresetData
 *
 * Each engine type (I4 Turbo, V8 NA, V10, etc.) should have its own preset
 * with characteristic sounds that reflect the engine's personality.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGEngineAudioPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ----------------------------------------
	// Identification
	// ----------------------------------------

	/// Internal preset name for lookup (e.g., "V8_Muscle", "I4_Turbo")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FName PresetName;

	/// Localized display name for audio settings UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FText DisplayName;

	/// Description of the engine sound character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FText Description;

	/// Engine configuration tag for categorization (e.g., "I4", "V6", "V8", "Flat6", "Rotary")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FName EngineType;

	// ----------------------------------------
	// Audio Configuration
	// ----------------------------------------

	/// The actual engine audio configuration (sounds, RPM ranges, crossfades)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FMGEngineAudioPreset Preset;

	/**
	 * Returns the engine audio preset struct for applying to components.
	 * @return Engine audio configuration data
	 */
	UFUNCTION(BlueprintPure, Category = "Engine Audio")
	FMGEngineAudioPreset GetPreset() const { return Preset; }
};

// ============================================================================
// VEHICLE SFX
// ============================================================================

/**
 * Vehicle SFX Preset Data Asset
 *
 * Configures non-engine vehicle sound effects including tire sounds,
 * collision impacts, wind noise, and mechanical sounds.
 *
 * Surface-specific tire sounds are configured in SurfaceConfigs array,
 * allowing different sounds for asphalt, gravel, grass, etc.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleSFXPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ----------------------------------------
	// Identification
	// ----------------------------------------

	/// Internal preset name for lookup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	FName PresetName;

	// ----------------------------------------
	// Surface Sounds
	// ----------------------------------------

	/// Per-surface tire and rolling sound configurations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	TArray<FMGSurfaceSoundConfig> SurfaceConfigs;

	// ----------------------------------------
	// Collision Sounds
	// ----------------------------------------

	/// Impact and collision sound configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	FMGCollisionSoundConfig CollisionConfig;

	// ----------------------------------------
	// Environmental Sounds
	// ----------------------------------------

	/// Speed-dependent wind/air rush sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	USoundBase* WindNoiseSound = nullptr;

	/// Suspension bump/thump for rough terrain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	USoundBase* SuspensionSound = nullptr;

	/// High-friction brake squeal sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	USoundBase* BrakeSquealSound = nullptr;

	/**
	 * Applies this preset configuration to a vehicle SFX component.
	 * Call this when spawning a vehicle to set up its sound effects.
	 * @param Component The vehicle SFX component to configure
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void ApplyToComponent(UMGVehicleSFXComponent* Component) const;
};

// ============================================================================
// MUSIC TRACKS
// ============================================================================

/**
 * Music Track Data Asset
 *
 * Configures a single music track with support for layered/dynamic audio.
 * Tracks can have multiple layers that blend based on the current music state
 * (menu, racing, intense action, victory, etc.).
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGMusicTrackData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ----------------------------------------
	// Track Configuration
	// ----------------------------------------

	/// Core track configuration (sound files, layers, BPM, loop points)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FMGMusicTrack Track;

	// ----------------------------------------
	// Metadata / Tags
	// ----------------------------------------

	/// Genre tag for filtering (e.g., "Electronic", "HipHop", "Rock", "DrumAndBass")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName Genre;

	/// Mood/energy tag for smart selection (e.g., "Aggressive", "Chill", "Hype", "Dark")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName Mood;

	/// Music states where this track works well (Racing, Menu, Victory, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TArray<EMGMusicState> SuitableStates;

	/**
	 * Returns the music track configuration struct.
	 * @return Track configuration data
	 */
	UFUNCTION(BlueprintPure, Category = "Music")
	FMGMusicTrack GetTrack() const { return Track; }

	/**
	 * Checks if this track is appropriate for a given music state.
	 * Used by the music manager to filter available tracks.
	 * @param State The music state to check
	 * @return True if track works well with the given state
	 */
	UFUNCTION(BlueprintPure, Category = "Music")
	bool IsSuitableForState(EMGMusicState State) const
	{
		return SuitableStates.Contains(State);
	}
};

// ============================================================================
// MUSIC PLAYLISTS
// ============================================================================

/**
 * Music Playlist Data Asset
 *
 * Groups multiple music tracks into a playlist for specific game modes
 * or contexts (main gameplay, menus, garage, etc.).
 * Supports shuffle mode for varied playback.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGMusicPlaylistData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ----------------------------------------
	// Identification
	// ----------------------------------------

	/// Internal playlist identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName PlaylistName;

	/// Localized display name for music settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText DisplayName;

	// ----------------------------------------
	// Tracks
	// ----------------------------------------

	/// Ordered list of tracks in this playlist
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TArray<UMGMusicTrackData*> Tracks;

	/// If true, tracks play in random order by default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bShuffleByDefault = true;

	/**
	 * Gets the names of all tracks in this playlist.
	 * @return Array of track name identifiers
	 */
	UFUNCTION(BlueprintPure, Category = "Music")
	TArray<FName> GetTrackNames() const;

	/**
	 * Registers all tracks in this playlist with the music manager.
	 * Call once during game initialization to make tracks available.
	 * @param MusicManager The music manager to register tracks with
	 */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void RegisterWithMusicManager(UMGMusicManager* MusicManager) const;
};

// ============================================================================
// STINGER COLLECTIONS
// ============================================================================

/**
 * Stinger Collection Data Asset
 *
 * Configures short musical phrases (stingers) that play during
 * significant race events. Stingers add emotional punctuation
 * to gameplay moments like race start, final lap, and victory.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGStingerCollectionData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ----------------------------------------
	// Identification
	// ----------------------------------------

	/// Internal collection name for lookup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName CollectionName;

	// ----------------------------------------
	// Race Event Stingers
	// ----------------------------------------

	/// Plays during race countdown (3, 2, 1...)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger CountdownStinger;

	/// Plays at "GO!" when race begins
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger RaceStartStinger;

	/// Plays when entering the final lap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger FinalLapStinger;

	/// Plays when player overtakes another racer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger PositionGainedStinger;

	/// Plays when player is overtaken
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger PositionLostStinger;

	/// Plays on race victory (1st place finish)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger VictoryStinger;

	/// Plays on race completion (non-victory)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger DefeatStinger;

	/// Plays when player sets a new track or personal record
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger NewRecordStinger;

	// ----------------------------------------
	// Custom Stingers
	// ----------------------------------------

	/// Additional stingers for game-specific events
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Custom")
	TArray<FMGMusicStinger> CustomStingers;

	/**
	 * Registers all stingers in this collection with the music manager.
	 * Call once during game initialization to enable stinger playback.
	 * @param MusicManager The music manager to register stingers with
	 */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void RegisterWithMusicManager(UMGMusicManager* MusicManager) const;
};

// ============================================================================
// MASTER AUDIO CONFIGURATION
// ============================================================================

/**
 * Complete Audio Config Data Asset
 *
 * Master configuration asset that combines all audio settings and
 * references to audio sub-assets. Use this as the single entry point
 * for initializing the game's audio systems.
 *
 * @section audio_init Initialization
 * Load this asset at game startup and use it to:
 * 1. Apply DefaultSettings to audio subsystem
 * 2. Register all playlists with music manager
 * 3. Register all stingers with music manager
 * 4. Map vehicle classes to engine presets
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGAudioConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ----------------------------------------
	// Identification
	// ----------------------------------------

	/// Configuration name for identification (e.g., "DefaultAudioConfig")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	FName ConfigName;

	// ----------------------------------------
	// Global Settings
	// ----------------------------------------

	/// Default volume levels and audio quality settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	FMGAudioSettings DefaultSettings;

	// ----------------------------------------
	// Engine Audio
	// ----------------------------------------

	/// Maps vehicle class names to their engine audio presets
	/// Key: Vehicle class name (e.g., "Muscle", "JDM", "Supercar")
	/// Value: Engine audio preset to use for that class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	TMap<FName, UMGEngineAudioPresetData*> EnginePresetsByClass;

	// ----------------------------------------
	// Vehicle SFX
	// ----------------------------------------

	/// Default SFX preset used when no specific preset is assigned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	UMGVehicleSFXPresetData* DefaultVehicleSFX = nullptr;

	// ----------------------------------------
	// Music
	// ----------------------------------------

	/// Primary gameplay music playlist (used during races)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	UMGMusicPlaylistData* MainPlaylist = nullptr;

	/// Menu/UI music playlist (used in menus and garage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	UMGMusicPlaylistData* MenuPlaylist = nullptr;

	/// Collection of event stingers for race moments
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	UMGStingerCollectionData* Stingers = nullptr;

	/**
	 * Gets the appropriate engine audio preset for a vehicle class.
	 * Falls back to a default if no specific preset is mapped.
	 * @param VehicleClass The vehicle class name to look up
	 * @return Engine audio preset for the class, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Audio Config")
	UMGEngineAudioPresetData* GetEnginePresetForClass(FName VehicleClass) const;
};

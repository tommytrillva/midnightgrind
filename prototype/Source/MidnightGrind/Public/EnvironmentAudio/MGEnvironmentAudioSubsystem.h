// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGEnvironmentAudioSubsystem.h
 * @brief Environment and Ambient Audio Subsystem for Midnight Grind
 *
 * This file defines the environment audio system responsible for creating immersive
 * ambient soundscapes that react to the player's location and game conditions. The
 * system manages multiple layers of ambient audio that blend together based on
 * environment type, time of day, weather, and player speed.
 *
 * @section env_audio_features Key Features
 *
 * - **Environment Zones**: Define areas with distinct ambient soundscapes (urban, tunnel, waterfront, etc.)
 * - **Multi-Layer Audio**: Stack multiple ambient layers (base, traffic, pedestrian, nature, weather)
 * - **Time of Day**: Soundscapes adapt to different times (dawn, morning, night, etc.)
 * - **Weather Integration**: Rain, wind, and thunder sounds that blend with the environment
 * - **Speed-Based Audio**: Some layers change based on player vehicle speed (e.g., wind noise)
 * - **One-Shot Sounds**: Random environmental sounds (car horns, sirens, birds, etc.)
 *
 * @section env_audio_architecture Architecture
 *
 * This is a WorldSubsystem, meaning one instance exists per world/level. It continuously
 * monitors the listener (camera/player) position and crossfades between soundscapes as
 * the player moves through different zones.
 *
 * The audio is organized in layers:
 * - **Base Layer**: Core ambient sound for the environment type
 * - **Traffic Layer**: Vehicle and road noise (varies by location)
 * - **Pedestrian Layer**: Crowd murmur, footsteps (urban areas)
 * - **Nature Layer**: Birds, insects, wind in trees (parks, suburbs)
 * - **Weather Layer**: Rain, wind, thunder (when weather is active)
 * - **Special Layer**: Location-specific sounds (construction, factory noise)
 *
 * @section env_audio_concepts Core Concepts
 *
 * - **Soundscape**: A collection of audio layers that define an environment's sound
 * - **Environment Zone**: A spatial volume with an associated soundscape
 * - **One-Shot**: A short sound that plays randomly at intervals (horn honk, bird call)
 * - **Transition**: The crossfade between soundscapes when entering a new zone
 *
 * @section env_audio_usage Basic Usage
 *
 * @code
 * // Get the subsystem
 * UMGEnvironmentAudioSubsystem* EnvAudio = GetWorld()->GetSubsystem<UMGEnvironmentAudioSubsystem>();
 *
 * // Register environment zones (usually done by level actors)
 * EnvAudio->RegisterEnvironmentZone(DowntownZone);
 *
 * // Update listener position each frame
 * EnvAudio->UpdateListenerLocation(CameraLocation);
 *
 * // React to game state
 * EnvAudio->SetTimeOfDay(EMGTimeOfDayAudio::Night);
 * EnvAudio->SetRainIntensity(0.7f);
 * @endcode
 *
 * @see UMGAudioSubsystem
 * @see UMGDynamicMixSubsystem
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGEnvironmentAudioSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Types of environment areas in the game world
 *
 * Each environment type has distinct ambient audio characteristics.
 * The system uses this to select appropriate soundscapes and one-shot sounds.
 */
UENUM(BlueprintType)
enum class EMGEnvironmentType : uint8
{
	Urban,          ///< General city area with mixed sounds (traffic, people, buildings)
	Downtown,       ///< Dense urban core with tall buildings, heavy traffic noise
	Industrial,     ///< Factory/warehouse areas with machinery, trucks, metal sounds
	Residential,    ///< Quiet neighborhoods with occasional cars, dogs, lawn mowers
	Highway,        ///< High-speed roads with constant traffic whoosh, minimal pedestrian
	Tunnel,         ///< Enclosed tunnel with heavy reverb, muffled outside sounds
	Bridge,         ///< Open bridge with wind, structural sounds, water below
	Waterfront,     ///< Harbor/beach areas with water, seagulls, boats
	Park,           ///< Green spaces with birds, wind in trees, distant city
	Suburbs,        ///< Outskirt areas mixing residential and light commercial
	Commercial,     ///< Shopping districts with music, people, store sounds
	Underground     ///< Subterranean areas (parking, subway) with echo, muffled
};

/**
 * @brief Categories of ambient sound layers
 *
 * Soundscapes are built from multiple layers that can be independently
 * controlled. This allows for dynamic mixing based on game conditions.
 */
UENUM(BlueprintType)
enum class EMGAmbientLayerType : uint8
{
	Base,           ///< Core ambient sound that is always present in a zone
	Traffic,        ///< Vehicle sounds - engines, horns, passing cars
	Pedestrian,     ///< Human activity - footsteps, voices, crowd murmur
	Nature,         ///< Natural sounds - birds, insects, wind, water
	Industrial,     ///< Mechanical sounds - machinery, vents, construction
	Weather,        ///< Weather-related - rain, wind, thunder (controlled by weather system)
	TimeOfDay,      ///< Time-specific sounds - crickets at night, morning birds
	Special         ///< Location-specific unique sounds - specific to certain areas
};

/**
 * @brief Time of day periods that affect ambient audio
 *
 * Different times of day have distinct audio characteristics.
 * Night has crickets and fewer cars; morning has birds and commuter traffic.
 */
UENUM(BlueprintType)
enum class EMGTimeOfDayAudio : uint8
{
	Dawn,           ///< Early morning (5-7 AM) - birds waking, quiet traffic
	Morning,        ///< Morning (7-11 AM) - commuter traffic, construction starting
	Afternoon,      ///< Afternoon (11 AM-5 PM) - peak activity, full ambient
	Evening,        ///< Evening (5-8 PM) - rush hour, dinner sounds
	Dusk,           ///< Twilight (8-10 PM) - transition sounds, evening insects
	Night,          ///< Night (10 PM-2 AM) - quieter traffic, nightlife areas active
	LateNight       ///< Late night (2-5 AM) - minimal activity, distant sounds
};

// ============================================================================
// DATA STRUCTURES - SOUND LAYERS
// ============================================================================

/**
 * @brief A single layer of ambient sound within a soundscape
 *
 * Layers are combined to create complete soundscapes. Each layer can have
 * its own volume, spatialization settings, and behavior modifiers.
 *
 * @section layer_example Example
 * A Downtown soundscape might have:
 * - Base layer: City hum
 * - Traffic layer: Car engines, horns
 * - Pedestrian layer: Crowd murmur
 * - Special layer: Construction sounds
 */
USTRUCT(BlueprintType)
struct FMGAmbientSoundLayer
{
	GENERATED_BODY()

	/// Unique identifier for this layer within the soundscape
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LayerID;

	/// Category of this layer (affects how it's controlled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAmbientLayerType LayerType = EMGAmbientLayerType::Base;

	/// The audio asset to play (soft reference for async loading)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	/// Base volume multiplier (0.0 to 1.0+)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.0f;

	/// Pitch multiplier (1.0 = normal pitch)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch = 1.0f;

	/// Time in seconds to fade this layer in when entering a zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeInTime = 2.0f;

	/// Time in seconds to fade this layer out when leaving a zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeOutTime = 2.0f;

	/// Whether this sound loops continuously (most ambient sounds do)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = true;

	/// If true, sound is positioned in 3D space; if false, it's non-directional
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpatialized = false;

	/// Distance at which the sound is at full volume (for spatialized sounds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 0.0f;

	/// Distance at which the sound is inaudible (for spatialized sounds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 10000.0f;

	/// If true, volume changes based on player vehicle speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectedBySpeed = false;

	/// How much speed affects volume (used when bAffectedBySpeed is true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedVolumeMultiplier = 1.0f;
};

// ============================================================================
// DATA STRUCTURES - SOUNDSCAPES
// ============================================================================

/**
 * @brief A complete ambient soundscape composed of multiple layers
 *
 * Soundscapes define the complete audio character of an environment type.
 * They can be assigned to zones or triggered manually.
 */
USTRUCT(BlueprintType)
struct FMGEnvironmentSoundscape
{
	GENERATED_BODY()

	/// Unique identifier (e.g., "Soundscape_Downtown_Day")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SoundscapeID;

	/// Environment type this soundscape is designed for
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEnvironmentType EnvironmentType = EMGEnvironmentType::Urban;

	/// All the audio layers that make up this soundscape
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGAmbientSoundLayer> Layers;

	/// Master volume for the entire soundscape
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseVolume = 1.0f;

	/// Default time to transition to/from this soundscape
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionTime = 3.0f;

	/// Higher priority soundscapes override lower ones (0 = lowest)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

// ============================================================================
// DATA STRUCTURES - ZONES
// ============================================================================

/**
 * @brief A spatial region in the world with an associated soundscape
 *
 * Environment zones define where different ambient audio plays. They can be
 * box-shaped or spherical, and have a blend radius for smooth transitions.
 */
USTRUCT(BlueprintType)
struct FMGEnvironmentZone
{
	GENERATED_BODY()

	/// Unique identifier for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	/// The soundscape that plays when inside this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEnvironmentSoundscape Soundscape;

	/// Center point of the zone in world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	/// Half-extents of the zone box (if using box shape)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Extent = FVector(1000.0f);

	/// Distance over which audio blends when entering/exiting the zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendRadius = 200.0f;

	/// If true, uses box shape; if false, uses sphere shape
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseBoxShape = true;

	/// Radius of the zone sphere (if using sphere shape)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SphereRadius = 500.0f;
};

// ============================================================================
// DATA STRUCTURES - ONE-SHOTS
// ============================================================================

/**
 * @brief A random environmental sound that plays at intervals
 *
 * One-shots add life to environments through occasional sounds like car horns,
 * sirens, birds, dogs barking, etc. They play at random intervals at random
 * nearby locations.
 *
 * @section oneshot_example Examples
 * - Car horn honking (Urban, Downtown - Day/Evening)
 * - Bird chirping (Park, Residential - Morning/Afternoon)
 * - Police siren in distance (Urban, Downtown - Night)
 * - Dog barking (Residential, Suburbs - Any time)
 */
USTRUCT(BlueprintType)
struct FMGOneShot
{
	GENERATED_BODY()

	/// Unique identifier for this one-shot type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OneShotID;

	/// The sound to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	/// Minimum seconds between plays of this sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinInterval = 5.0f;

	/// Maximum seconds between plays of this sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxInterval = 30.0f;

	/// Minimum distance from listener to spawn the sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 50.0f;

	/// Maximum distance from listener to spawn the sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 500.0f;

	/// Minimum random volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeMin = 0.5f;

	/// Maximum random volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeMax = 1.0f;

	/// Minimum random pitch variation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMin = 0.9f;

	/// Maximum random pitch variation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMax = 1.1f;

	/// Environment types where this sound can play (empty = all)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGEnvironmentType> ValidEnvironments;

	/// Times of day when this sound can play (empty = all)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGTimeOfDayAudio> ValidTimes;
};

// ============================================================================
// DATA STRUCTURES - STATE
// ============================================================================

/**
 * @brief Current runtime state of the environment audio system
 *
 * Tracks all the parameters that affect environment audio playback,
 * including location, time, weather, and player state.
 */
USTRUCT(BlueprintType)
struct FMGEnvironmentAudioState
{
	GENERATED_BODY()

	/// ID of the zone the listener is currently in (NAME_None if outside all zones)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentZoneID = NAME_None;

	/// Current environment type (derived from current zone or default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEnvironmentType CurrentEnvironment = EMGEnvironmentType::Urban;

	/// Current time of day for audio selection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeOfDayAudio TimeOfDay = EMGTimeOfDayAudio::Afternoon;

	/// Current player vehicle speed (used for speed-affected layers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	/// Wind intensity (0.0 to 1.0) - affects wind sound layers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindIntensity = 0.0f;

	/// Whether it is currently raining
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRaining = false;

	/// Rain intensity (0.0 to 1.0) - affects rain sound volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RainIntensity = 0.0f;

	/// Whether the listener is inside a vehicle (affects muffling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInsideVehicle = true;

	/// Master volume multiplier for all environment audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MasterVolume = 1.0f;
};

// ============================================================================
// DELEGATES
// ============================================================================

/** @brief Broadcast when the listener enters a different environment zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentZoneChanged, FName, OldZone, FName, NewZone);

/** @brief Broadcast when the time of day changes (affects ambient audio selection) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, EMGTimeOfDayAudio, OldTime, EMGTimeOfDayAudio, NewTime);

/** @brief Broadcast when a one-shot sound is played */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOneShotPlayed, FName, OneShotID);

/** @brief Broadcast when rain intensity changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherAudioChanged, float, RainIntensity);

// ============================================================================
// ENVIRONMENT AUDIO SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Environment and ambient audio management subsystem
 *
 * Manages ambient soundscapes, environment zones, one-shot sounds, and
 * weather audio. Creates immersive audio environments that react to
 * player location, time of day, and weather conditions.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGEnvironmentAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initialize the environment audio subsystem
	 * Sets up default soundscapes and starts the environment tick timer.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief Clean up when subsystem is destroyed
	 * Stops all ambient sounds and releases resources.
	 */
	virtual void Deinitialize() override;

	/**
	 * @brief Determine if this subsystem should be created for the given world
	 * Only creates for game worlds.
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// ZONE MANAGEMENT
	// ==========================================
	/** @name Zone Management
	 *  Functions for registering and managing environment zones.
	 */
	///@{

	/**
	 * @brief Register an environment zone (usually called by zone actors)
	 * @param Zone The zone configuration to register
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Zone")
	void RegisterEnvironmentZone(const FMGEnvironmentZone& Zone);

	/**
	 * @brief Remove an environment zone
	 * @param ZoneID The ID of the zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Zone")
	void UnregisterEnvironmentZone(FName ZoneID);

	/**
	 * @brief Get a specific zone by ID
	 * @param ZoneID The zone to find
	 */
	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|Zone")
	FMGEnvironmentZone GetEnvironmentZone(FName ZoneID) const;

	/** @brief Get all registered environment zones */
	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|Zone")
	TArray<FMGEnvironmentZone> GetAllZones() const;

	///@}

	// ==========================================
	// SOUNDSCAPE MANAGEMENT
	// ==========================================
	/** @name Soundscape Management
	 *  Functions for controlling ambient soundscapes.
	 */
	///@{

	/**
	 * @brief Register a reusable soundscape configuration
	 * @param Soundscape The soundscape to register
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Soundscape")
	void RegisterSoundscape(const FMGEnvironmentSoundscape& Soundscape);

	/**
	 * @brief Manually transition to a specific soundscape
	 * @param SoundscapeID The soundscape to transition to
	 * @param TransitionTime Seconds to crossfade
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Soundscape")
	void TransitionToSoundscape(FName SoundscapeID, float TransitionTime = 3.0f);

	/** @brief Get the currently playing soundscape */
	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|Soundscape")
	FMGEnvironmentSoundscape GetCurrentSoundscape() const { return CurrentSoundscape; }

	/**
	 * @brief Set the volume of a specific layer within the current soundscape
	 * @param LayerID The layer to adjust
	 * @param Volume New volume (0.0 to 1.0+)
	 * @param FadeTime Seconds to fade to new volume
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Soundscape")
	void SetLayerVolume(FName LayerID, float Volume, float FadeTime = 1.0f);

	/**
	 * @brief Mute or unmute a specific layer
	 * @param LayerID The layer to mute/unmute
	 * @param bMute True to mute, false to unmute
	 * @param FadeTime Seconds to fade
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Soundscape")
	void MuteLayer(FName LayerID, bool bMute, float FadeTime = 1.0f);

	///@}

	// ==========================================
	// ONE-SHOT SOUNDS
	// ==========================================
	/** @name One-Shot Sounds
	 *  Functions for random environmental sounds.
	 */
	///@{

	/**
	 * @brief Register a one-shot sound type
	 * @param OneShot The one-shot configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|OneShot")
	void RegisterOneShot(const FMGOneShot& OneShot);

	/**
	 * @brief Play a specific one-shot at a location
	 * @param OneShotID The one-shot type to play
	 * @param Location World position to play at
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|OneShot")
	void PlayOneShot(FName OneShotID, FVector Location);

	/**
	 * @brief Play a random appropriate one-shot for the current environment
	 * @param Environment The environment type to select sounds for
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|OneShot")
	void PlayRandomOneShot(EMGEnvironmentType Environment);

	/**
	 * @brief Enable or disable automatic one-shot playback
	 * @param bEnabled True to enable random one-shots
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|OneShot")
	void SetOneShotEnabled(bool bEnabled);

	///@}

	// ==========================================
	// STATE UPDATES
	// ==========================================
	/** @name State Updates
	 *  Functions for updating environment audio based on game state.
	 */
	///@{

	/**
	 * @brief Update the listener (camera) position
	 * This determines which zone's audio plays.
	 * @param Location World position of the listener
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|State")
	void UpdateListenerLocation(FVector Location);

	/**
	 * @brief Set the player vehicle speed (affects speed-based layers)
	 * @param Speed Current vehicle speed
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|State")
	void SetPlayerSpeed(float Speed);

	/**
	 * @brief Set the current time of day (affects sound selection)
	 * @param Time New time of day
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|State")
	void SetTimeOfDay(EMGTimeOfDayAudio Time);

	/**
	 * @brief Set whether the listener is inside a vehicle
	 * Inside vehicles, outside sounds are slightly muffled.
	 * @param bInside True if inside a vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|State")
	void SetInsideVehicle(bool bInside);

	/** @brief Get the current audio state */
	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|State")
	FMGEnvironmentAudioState GetAudioState() const { return AudioState; }

	///@}

	// ==========================================
	// WEATHER INTEGRATION
	// ==========================================
	/** @name Weather Integration
	 *  Functions for weather-related audio.
	 */
	///@{

	/**
	 * @brief Set rain intensity (triggers rain audio)
	 * @param Intensity 0.0 (no rain) to 1.0 (heavy rain)
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Weather")
	void SetRainIntensity(float Intensity);

	/**
	 * @brief Set wind intensity
	 * @param Intensity 0.0 (calm) to 1.0 (strong wind)
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Weather")
	void SetWindIntensity(float Intensity);

	/**
	 * @brief Trigger a thunder sound at a specific distance
	 * @param Distance How far away the lightning strike is
	 * @param Intensity Volume multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Weather")
	void TriggerThunder(float Distance, float Intensity = 1.0f);

	///@}

	// ==========================================
	// WIND AUDIO
	// ==========================================
	/** @name Wind Audio
	 *  Functions for wind noise based on speed and direction.
	 */
	///@{

	/**
	 * @brief Update wind audio based on speed and direction
	 * @param Speed Current wind/vehicle speed
	 * @param Direction Wind direction vector
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Wind")
	void UpdateWindAudio(float Speed, FVector Direction);

	///@}

	// ==========================================
	// VOLUME CONTROL
	// ==========================================
	/** @name Volume Control
	 *  Master volume controls for all environment audio.
	 */
	///@{

	/**
	 * @brief Set the master volume for all environment audio
	 * @param Volume Volume from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Volume")
	void SetMasterVolume(float Volume);

	/** @brief Get current master volume */
	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|Volume")
	float GetMasterVolume() const { return AudioState.MasterVolume; }

	/**
	 * @brief Fade out all environment audio
	 * @param FadeTime Duration of fade in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Volume")
	void FadeOutAll(float FadeTime = 2.0f);

	/**
	 * @brief Fade in all environment audio
	 * @param FadeTime Duration of fade in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Volume")
	void FadeInAll(float FadeTime = 2.0f);

	///@}

	// ==========================================
	// EVENT DELEGATES
	// ==========================================
	/** @name Events
	 *  Delegates for reacting to environment audio state changes.
	 */
	///@{

	/// Fires when the listener enters a new environment zone
	UPROPERTY(BlueprintAssignable, Category = "EnvironmentAudio|Events")
	FOnEnvironmentZoneChanged OnEnvironmentZoneChanged;

	/// Fires when the time of day changes
	UPROPERTY(BlueprintAssignable, Category = "EnvironmentAudio|Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	/// Fires when a one-shot sound is played
	UPROPERTY(BlueprintAssignable, Category = "EnvironmentAudio|Events")
	FOnOneShotPlayed OnOneShotPlayed;

	/// Fires when rain/weather audio changes
	UPROPERTY(BlueprintAssignable, Category = "EnvironmentAudio|Events")
	FOnWeatherAudioChanged OnWeatherAudioChanged;

	///@}

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** @brief Called periodically to update environment audio */
	void OnEnvironmentTick();

	/** @brief Handle transitions between zones */
	void UpdateZoneTransitions();

	/** @brief Smoothly interpolate layer volumes */
	void UpdateLayerVolumes();

	/** @brief Handle random one-shot sound playback */
	void ProcessOneShots();

	/** @brief Update weather-related audio (rain, wind) */
	void UpdateWeatherAudio();

	/** @brief Update audio layers affected by player speed */
	void UpdateSpeedBasedAudio();

	/**
	 * @brief Find which zone contains a given location
	 * @param Location World position to check
	 * @return The zone at that location, or invalid zone if none
	 */
	FMGEnvironmentZone FindZoneAtLocation(FVector Location) const;

	/** @brief Set up default soundscapes for all environment types */
	void InitializeDefaultSoundscapes();

	/**
	 * @brief Play a one-shot sound at a specific location
	 * @param OneShot The one-shot configuration
	 * @param Location World position to play at
	 */
	void PlayOneShotAtLocation(const FMGOneShot& OneShot, FVector Location);

	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/// All registered environment zones (keyed by ZoneID)
	UPROPERTY()
	TMap<FName, FMGEnvironmentZone> EnvironmentZones;

	/// Registered soundscape configurations
	UPROPERTY()
	TMap<FName, FMGEnvironmentSoundscape> Soundscapes;

	/// Registered one-shot sound configurations
	UPROPERTY()
	TArray<FMGOneShot> OneShots;

	/// Currently playing soundscape
	UPROPERTY()
	FMGEnvironmentSoundscape CurrentSoundscape;

	/// Current runtime state
	UPROPERTY()
	FMGEnvironmentAudioState AudioState;

	/// Current listener world position
	UPROPERTY()
	FVector ListenerLocation = FVector::ZeroVector;

	/// Whether automatic one-shot playback is enabled
	UPROPERTY()
	bool bOneShotsEnabled = true;

	/// Timer tracking for next one-shot
	UPROPERTY()
	float OneShotTimer = 0.0f;

	/// Time until the next random one-shot plays
	UPROPERTY()
	float NextOneShotTime = 5.0f;

	/// Current volume for each layer (being interpolated)
	UPROPERTY()
	TMap<FName, float> LayerVolumes;

	/// Target volumes for layer interpolation
	UPROPERTY()
	TMap<FName, float> TargetLayerVolumes;

	/// Timer handle for environment tick
	FTimerHandle EnvironmentTickHandle;
};

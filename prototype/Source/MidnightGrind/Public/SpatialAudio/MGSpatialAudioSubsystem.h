// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGSpatialAudioSubsystem.h
 * @brief Spatial Audio and Acoustics Subsystem for Midnight Grind
 *
 * This file defines the spatial audio system that handles 3D sound positioning,
 * acoustic environments, occlusion, reflections, and Doppler effects. The system
 * creates realistic audio that responds to the physical environment.
 *
 * @section spatial_features Key Features
 *
 * - **Acoustic Zones**: Define areas with unique reverb, echo, and filtering (tunnels, garages, etc.)
 * - **Sound Occlusion**: Sounds are muffled when obstacles block the path to the listener
 * - **Audio Reflections**: Simulated early reflections for enhanced spatial awareness
 * - **Doppler Effect**: Pitch shifting based on relative velocity (passing cars sound realistic)
 * - **Distance Attenuation**: Sounds naturally fade with distance using customizable curves
 * - **Environment Presets**: Quick application of acoustic settings for common environments
 *
 * @section spatial_architecture Architecture
 *
 * This is a WorldSubsystem that processes spatial audio for all registered sound sources.
 * It performs ray-casting for occlusion, calculates Doppler shifts, and manages acoustic
 * zone transitions.
 *
 * The system uses a priority-based approach to manage CPU usage:
 * - Player-related sounds get full processing
 * - High-priority sounds get occlusion and Doppler
 * - Lower priority sounds may skip expensive calculations
 *
 * @section spatial_concepts Core Concepts
 *
 * - **Acoustic Zone**: A spatial volume with specific reverb/echo/filtering settings
 * - **Sound Source**: A registered 3D sound emitter with position and velocity
 * - **Listener**: The audio receiver (usually the camera/player position)
 * - **Occlusion**: Volume reduction when objects block the sound path
 * - **Doppler Effect**: Pitch change based on relative movement (approaching = higher pitch)
 * - **Attenuation**: How quickly sound fades with distance
 *
 * @section spatial_environments Acoustic Environments
 *
 * | Environment | Description                                          |
 * |-------------|------------------------------------------------------|
 * | Outdoor     | Open air, minimal reverb, natural sound              |
 * | Street      | Urban canyon effect, moderate reflections            |
 * | Tunnel      | Long reverb tail, echo, low-pass filtering           |
 * | Underpass   | Short reverb, slight echo                            |
 * | Garage      | Enclosed space, medium reverb, boxy sound            |
 * | Parking     | Large enclosed space, long reverb                    |
 * | Highway     | Open but with traffic noise characteristics          |
 * | Forest      | Natural absorption, bird-friendly acoustics          |
 * | Urban       | Dense reflections from buildings                     |
 * | Industrial  | Metallic reflections, machinery characteristics      |
 * | Downtown    | Tall building reflections, urban canyon              |
 * | Waterfront  | Open with water reflections, seagull-friendly        |
 *
 * @section spatial_usage Basic Usage
 *
 * @code
 * // Get the subsystem
 * UMGSpatialAudioSubsystem* SpatialSys = GetWorld()->GetSubsystem<UMGSpatialAudioSubsystem>();
 *
 * // Register acoustic zones (usually done by level actors)
 * SpatialSys->RegisterAcousticZone(TunnelZone);
 *
 * // Register a sound source (e.g., another vehicle)
 * FMGSpatialSoundSource CarSource;
 * CarSource.SourceID = FName("AI_Car_01");
 * CarSource.bDopplerEnabled = true;
 * SpatialSys->RegisterSoundSource(CarSource);
 *
 * // Update positions each frame
 * SpatialSys->UpdateListener(CameraLocation, CameraRotation, CameraVelocity);
 * SpatialSys->UpdateSoundSource(FName("AI_Car_01"), CarLocation, CarVelocity);
 *
 * // Query occlusion for custom audio processing
 * float Occlusion = SpatialSys->GetOcclusionForSource(FName("AI_Car_01"));
 * @endcode
 *
 * @see UMGAudioSubsystem
 * @see UMGEngineAudioSubsystem
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGSpatialAudioSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Types of acoustic environments with distinct reverb/echo characteristics
 *
 * Each environment type has preset acoustic properties that affect how sounds
 * are processed (reverb, echo, filtering).
 */
UENUM(BlueprintType)
enum class EMGAcousticEnvironment : uint8
{
	Outdoor,        ///< Open outdoor space - minimal reverb, natural sound
	Street,         ///< City street - moderate reflections from buildings
	Tunnel,         ///< Enclosed tunnel - heavy reverb, low-pass filtering, echo
	Underpass,      ///< Short underpass - brief reverb, slight echo
	Garage,         ///< Enclosed garage - medium reverb, boxy sound
	Parking,        ///< Parking structure - long reverb, concrete reflections
	Highway,        ///< Open highway - minimal reverb, wind noise emphasis
	Forest,         ///< Forest/nature - absorption, natural dampening
	Urban,          ///< Dense urban - many reflections, urban canyon effect
	Industrial,     ///< Industrial area - metallic reflections, machinery resonance
	Downtown,       ///< Downtown core - tall building reflections
	Waterfront      ///< Near water - open with reflective water surface
};

/**
 * @brief Priority levels for sound sources
 *
 * Higher priority sounds get more CPU resources for spatial processing
 * (occlusion, reflections, Doppler). Lower priority sounds may be culled
 * when the system is under load.
 */
UENUM(BlueprintType)
enum class EMGSoundPriority : uint8
{
	Background,     ///< Distant/ambient sounds - may be culled first
	Low,            ///< Less important sounds - simplified processing
	Normal,         ///< Default priority - standard processing
	High,           ///< Important sounds - full processing
	Critical,       ///< Must-hear sounds - never culled
	Player          ///< Player-related sounds - highest priority
};

/**
 * @brief Occlusion calculation modes for sound sources
 *
 * Determines how the system calculates whether obstacles block the
 * sound path between source and listener.
 */
UENUM(BlueprintType)
enum class EMGOcclusionType : uint8
{
	None,           ///< No occlusion - sound always plays at full volume
	Partial,        ///< Fixed partial occlusion (cheaper than dynamic)
	Full,           ///< Fixed full occlusion (sound blocked)
	Dynamic         ///< Ray-cast based occlusion (most realistic, most expensive)
};

// ============================================================================
// DATA STRUCTURES - ACOUSTIC ZONES
// ============================================================================

/**
 * @brief A spatial region with specific acoustic properties
 *
 * Acoustic zones define areas where sound behaves differently due to
 * the physical environment (tunnel reverb, building reflections, etc.).
 */
USTRUCT(BlueprintType)
struct FMGAcousticZone
{
	GENERATED_BODY()

	/// Unique identifier for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	/// Environment type that determines default acoustic settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAcousticEnvironment Environment = EMGAcousticEnvironment::Outdoor;

	/// Center point of the zone in world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	/// Half-extents of the zone box
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Extent = FVector(500.0f);

	/// Reverb decay time in seconds (longer = more reverb)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbDecay = 1.0f;

	/// Amount of reverb mixed into the sound (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbWetLevel = 0.3f;

	/// Low-pass filter cutoff in Hz (lower = more muffled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowPassFrequency = 20000.0f;

	/// Echo delay time in seconds (for distinct echoes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EchoDelay = 0.0f;

	/// Base occlusion factor for sounds outside this zone (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OcclusionFactor = 0.0f;

	/// Distance over which to blend when entering/exiting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendRadius = 100.0f;

	/// Higher priority zones override lower ones when overlapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

// ============================================================================
// DATA STRUCTURES - SOUND SOURCES
// ============================================================================

/**
 * @brief A 3D sound source that can be tracked by the spatial audio system
 *
 * Represents any sound-emitting object in the world (vehicles, sirens,
 * explosions, etc.). The system processes these sources for occlusion,
 * Doppler, and distance attenuation.
 */
USTRUCT(BlueprintType)
struct FMGSpatialSoundSource
{
	GENERATED_BODY()

	/// Unique identifier for this source
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SourceID;

	/// Current world position of the source
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Current velocity (used for Doppler calculation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	/// Base volume multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.0f;

	/// Distance at which sound is at full volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 100.0f;

	/// Distance at which sound is inaudible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 5000.0f;

	/// Processing priority for this source
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSoundPriority Priority = EMGSoundPriority::Normal;

	/// How occlusion is calculated for this source
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGOcclusionType OcclusionType = EMGOcclusionType::Dynamic;

	/// Current calculated occlusion value (0.0 = clear, 1.0 = fully blocked)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentOcclusion = 0.0f;

	/// Whether Doppler effect is applied to this source
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDopplerEnabled = true;

	/// Doppler effect intensity multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DopplerFactor = 1.0f;

	/// Whether this sound is positioned in 3D space
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpatialized = true;

	/// Blend between 2D (0.0) and 3D (1.0) spatialization
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpatialBlend = 1.0f;

	/// Whether this source is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;
};

// ============================================================================
// DATA STRUCTURES - LISTENER
// ============================================================================

/**
 * @brief Current state of the audio listener (player/camera)
 *
 * The listener is the "ears" of the player. Its position, rotation, and
 * velocity affect how all sounds are perceived.
 */
USTRUCT(BlueprintType)
struct FMGListenerState
{
	GENERATED_BODY()

	/// Current world position of the listener
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Current velocity of the listener (for Doppler)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	/// Current orientation of the listener
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/// ID of the acoustic zone the listener is in
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentZoneID = NAME_None;

	/// Current acoustic environment type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAcousticEnvironment CurrentEnvironment = EMGAcousticEnvironment::Outdoor;

	/// Whether the listener is inside a vehicle (affects exterior sound filtering)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInsideVehicle = false;

	/// Current speed of the listener (affects wind noise, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;
};

// ============================================================================
// DATA STRUCTURES - REFLECTIONS
// ============================================================================

/**
 * @brief Information about a single audio reflection
 *
 * Reflections are calculated by ray-casting from sound sources to
 * nearby surfaces. Each reflection adds a delayed, attenuated copy
 * of the sound to simulate real acoustic behavior.
 */
USTRUCT(BlueprintType)
struct FMGAudioReflection
{
	GENERATED_BODY()

	/// World position where the sound reflects off a surface
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ReflectionPoint = FVector::ZeroVector;

	/// Surface normal at the reflection point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Normal = FVector::UpVector;

	/// Total distance traveled (source -> reflection -> listener)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	/// Volume of the reflection relative to direct sound (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 0.0f;

	/// Time delay of the reflection in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Delay = 0.0f;
};

// ============================================================================
// DATA STRUCTURES - SETTINGS
// ============================================================================

/**
 * @brief Global settings for the spatial audio system
 *
 * Controls quality vs. performance tradeoffs and feature toggles.
 */
USTRUCT(BlueprintType)
struct FMGSpatialAudioSettings
{
	GENERATED_BODY()

	/// Enable occlusion ray-casting (CPU intensive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOcclusionEnabled = true;

	/// Enable reflection calculation (CPU intensive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReflectionsEnabled = true;

	/// Enable Doppler pitch shifting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDopplerEnabled = true;

	/// Multiplier for Doppler effect intensity (1.0 = realistic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DopplerScale = 1.0f;

	/// Speed of sound in Unreal units per second (34300 = realistic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedOfSound = 34300.0f;

	/// Maximum reflections to calculate per source
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxReflections = 4;

	/// Seconds between occlusion updates (lower = more responsive, more CPU)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OcclusionUpdateRate = 0.1f;

	/// Maximum sound sources to process at once
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxActiveSources = 32;

	/// Global scale for distance attenuation (higher = faster falloff)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttenuationScale = 1.0f;
};

// ============================================================================
// DELEGATES
// ============================================================================

/** @brief Broadcast when the listener enters a different acoustic zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAcousticZoneChanged, FName, OldZoneID, FName, NewZoneID);

/** @brief Broadcast when the acoustic environment type changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentChanged, EMGAcousticEnvironment, OldEnv, EMGAcousticEnvironment, NewEnv);

/** @brief Broadcast when a sound source's occlusion changes significantly */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSoundOccluded, FName, SourceID, float, OcclusionAmount);

// ============================================================================
// SPATIAL AUDIO SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Spatial audio processing subsystem
 *
 * Handles 3D audio positioning, acoustic zones, occlusion, reflections,
 * and Doppler effects. Creates realistic spatial audio that responds to
 * the physical environment.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSpatialAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initialize the spatial audio subsystem
	 * Sets up default environment presets and starts the spatial tick timer.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief Clean up when subsystem is destroyed
	 */
	virtual void Deinitialize() override;

	/**
	 * @brief Determine if this subsystem should be created
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// ZONE MANAGEMENT
	// ==========================================
	/** @name Zone Management
	 *  Functions for managing acoustic zones.
	 */
	///@{

	/**
	 * @brief Register an acoustic zone
	 * @param Zone The zone configuration to register
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Zone")
	void RegisterAcousticZone(const FMGAcousticZone& Zone);

	/**
	 * @brief Remove an acoustic zone
	 * @param ZoneID The ID of the zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Zone")
	void UnregisterAcousticZone(FName ZoneID);

	/**
	 * @brief Update an existing acoustic zone's settings
	 * @param Zone The updated zone configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Zone")
	void UpdateAcousticZone(const FMGAcousticZone& Zone);

	/**
	 * @brief Get a specific zone by ID
	 * @param ZoneID The zone to find
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Zone")
	FMGAcousticZone GetAcousticZone(FName ZoneID) const;

	/** @brief Get all registered acoustic zones */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Zone")
	TArray<FMGAcousticZone> GetAllAcousticZones() const;

	/**
	 * @brief Find the zone at a specific world location
	 * @param Location World position to check
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Zone")
	FMGAcousticZone GetZoneAtLocation(FVector Location) const;

	///@}

	// ==========================================
	// SOUND SOURCE MANAGEMENT
	// ==========================================
	/** @name Sound Source Management
	 *  Functions for managing 3D sound sources.
	 */
	///@{

	/**
	 * @brief Register a new sound source
	 * @param Source The source configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void RegisterSoundSource(const FMGSpatialSoundSource& Source);

	/**
	 * @brief Remove a sound source
	 * @param SourceID The ID of the source to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void UnregisterSoundSource(FName SourceID);

	/**
	 * @brief Update a source's position and velocity
	 * @param SourceID The source to update
	 * @param Location New world position
	 * @param Velocity New velocity vector
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void UpdateSoundSource(FName SourceID, FVector Location, FVector Velocity);

	/**
	 * @brief Set a source's volume
	 * @param SourceID The source to update
	 * @param Volume New volume (0.0 to 1.0+)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void SetSourceVolume(FName SourceID, float Volume);

	/**
	 * @brief Enable or disable a source
	 * @param SourceID The source to update
	 * @param bActive True to enable, false to disable
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void SetSourceActive(FName SourceID, bool bActive);

	/**
	 * @brief Get a source's current configuration
	 * @param SourceID The source to query
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Source")
	FMGSpatialSoundSource GetSoundSource(FName SourceID) const;

	/** @brief Get IDs of all currently active sources */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Source")
	TArray<FName> GetActiveSoundSources() const;

	///@}

	// ==========================================
	// LISTENER
	// ==========================================
	/** @name Listener
	 *  Functions for managing the audio listener (player/camera).
	 */
	///@{

	/**
	 * @brief Update the listener's position, rotation, and velocity
	 * Call this each frame to keep spatial audio accurate.
	 * @param Location World position
	 * @param Rotation Orientation
	 * @param Velocity Movement velocity
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Listener")
	void UpdateListener(FVector Location, FRotator Rotation, FVector Velocity);

	/**
	 * @brief Set whether the listener is inside a vehicle
	 * Affects how exterior sounds are filtered.
	 * @param bInside True if inside a vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Listener")
	void SetListenerInsideVehicle(bool bInside);

	/** @brief Get the current listener state */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Listener")
	FMGListenerState GetListenerState() const { return ListenerState; }

	/** @brief Get the ID of the zone the listener is in */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Listener")
	FName GetCurrentAcousticZone() const { return ListenerState.CurrentZoneID; }

	/** @brief Get the current acoustic environment type */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Listener")
	EMGAcousticEnvironment GetCurrentEnvironment() const { return ListenerState.CurrentEnvironment; }

	///@}

	// ==========================================
	// OCCLUSION
	// ==========================================
	/** @name Occlusion
	 *  Functions for controlling sound occlusion (blocking by obstacles).
	 */
	///@{

	/**
	 * @brief Enable or disable occlusion processing
	 * @param bEnabled True to enable occlusion
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Occlusion")
	void SetOcclusionEnabled(bool bEnabled);

	/** @brief Check if occlusion is enabled */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Occlusion")
	bool IsOcclusionEnabled() const { return Settings.bOcclusionEnabled; }

	/**
	 * @brief Get the current occlusion value for a source
	 * @param SourceID The source to query
	 * @return Occlusion from 0.0 (clear) to 1.0 (fully blocked)
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Occlusion")
	float GetOcclusionForSource(FName SourceID) const;

	/** @brief Force an immediate occlusion update for all sources */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Occlusion")
	void ForceOcclusionUpdate();

	///@}

	// ==========================================
	// DOPPLER
	// ==========================================
	/** @name Doppler
	 *  Functions for controlling the Doppler effect (pitch shift from movement).
	 */
	///@{

	/**
	 * @brief Enable or disable Doppler effect
	 * @param bEnabled True to enable Doppler
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Doppler")
	void SetDopplerEnabled(bool bEnabled);

	/** @brief Check if Doppler is enabled */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Doppler")
	bool IsDopplerEnabled() const { return Settings.bDopplerEnabled; }

	/**
	 * @brief Set the Doppler effect intensity
	 * @param Scale Multiplier (1.0 = realistic, higher = exaggerated)
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Doppler")
	void SetDopplerScale(float Scale);

	/**
	 * @brief Calculate the Doppler pitch for a source
	 * @param SourceID The source to calculate for
	 * @return Pitch multiplier (1.0 = no shift, <1.0 = lower, >1.0 = higher)
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Doppler")
	float CalculateDopplerPitch(FName SourceID) const;

	///@}

	// ==========================================
	// REFLECTIONS
	// ==========================================
	/** @name Reflections
	 *  Functions for controlling audio reflections (early echoes from surfaces).
	 */
	///@{

	/**
	 * @brief Enable or disable reflection calculation
	 * @param bEnabled True to enable reflections
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Reflections")
	void SetReflectionsEnabled(bool bEnabled);

	/** @brief Check if reflections are enabled */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Reflections")
	bool AreReflectionsEnabled() const { return Settings.bReflectionsEnabled; }

	/**
	 * @brief Get calculated reflections for a source
	 * @param SourceID The source to query
	 * @return Array of reflection data
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Reflections")
	TArray<FMGAudioReflection> GetReflectionsForSource(FName SourceID) const;

	///@}

	// ==========================================
	// ENVIRONMENT PRESETS
	// ==========================================
	/** @name Environment Presets
	 *  Functions for applying predefined acoustic settings.
	 */
	///@{

	/**
	 * @brief Apply a predefined environment's acoustic settings
	 * @param Environment The environment preset to apply
	 * @param TransitionTime Seconds to transition to new settings
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Environment")
	void ApplyEnvironmentPreset(EMGAcousticEnvironment Environment, float TransitionTime = 0.5f);

	/**
	 * @brief Get the acoustic zone settings for an environment type
	 * @param Environment The environment to query
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Environment")
	FMGAcousticZone GetEnvironmentPreset(EMGAcousticEnvironment Environment) const;

	///@}

	// ==========================================
	// DISTANCE ATTENUATION
	// ==========================================
	/** @name Distance Attenuation
	 *  Functions for controlling how sound fades with distance.
	 */
	///@{

	/**
	 * @brief Calculate the distance attenuation for a source
	 * @param SourceID The source to calculate for
	 * @return Attenuation multiplier (1.0 = full volume, 0.0 = inaudible)
	 */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Attenuation")
	float CalculateAttenuation(FName SourceID) const;

	/**
	 * @brief Set the global attenuation scale
	 * @param Scale Higher values = faster distance falloff
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Attenuation")
	void SetAttenuationScale(float Scale);

	///@}

	// ==========================================
	// SETTINGS
	// ==========================================
	/** @name Settings
	 *  Functions for configuring the spatial audio system.
	 */
	///@{

	/** @brief Get current spatial audio settings */
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Settings")
	FMGSpatialAudioSettings GetSettings() const { return Settings; }

	/**
	 * @brief Update all spatial audio settings
	 * @param NewSettings The new settings to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Settings")
	void UpdateSettings(const FMGSpatialAudioSettings& NewSettings);

	///@}

	// ==========================================
	// EVENT DELEGATES
	// ==========================================
	/** @name Events
	 *  Delegates for reacting to spatial audio changes.
	 */
	///@{

	/// Fires when the listener enters a different acoustic zone
	UPROPERTY(BlueprintAssignable, Category = "SpatialAudio|Events")
	FOnAcousticZoneChanged OnAcousticZoneChanged;

	/// Fires when the acoustic environment type changes
	UPROPERTY(BlueprintAssignable, Category = "SpatialAudio|Events")
	FOnEnvironmentChanged OnEnvironmentChanged;

	/// Fires when a sound source becomes occluded or unoccluded
	UPROPERTY(BlueprintAssignable, Category = "SpatialAudio|Events")
	FOnSoundOccluded OnSoundOccluded;

	///@}

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** @brief Called periodically to update spatial audio */
	void OnSpatialTick();

	/** @brief Update occlusion for all sources */
	void UpdateOcclusion();

	/** @brief Handle transitions between acoustic zones */
	void UpdateZoneTransitions();

	/**
	 * @brief Apply the acoustic settings from a zone
	 * @param Zone The zone whose settings to apply
	 */
	void ApplyZoneEffects(const FMGAcousticZone& Zone);

	/**
	 * @brief Calculate reflections for a source
	 * @param Source The source to calculate reflections for
	 */
	void CalculateReflections(FMGSpatialSoundSource& Source);

	/**
	 * @brief Calculate occlusion for a single source using ray-casting
	 * @param Source The source to calculate for
	 * @return Occlusion value from 0.0 to 1.0
	 */
	float CalculateOcclusionForSource(const FMGSpatialSoundSource& Source) const;

	/** @brief Sort sources by priority for processing order */
	void SortSourcesByPriority();

	/** @brief Set up default acoustic settings for each environment type */
	void InitializeEnvironmentPresets();

	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/// Registered acoustic zones (keyed by ZoneID)
	UPROPERTY()
	TMap<FName, FMGAcousticZone> AcousticZones;

	/// Registered sound sources (keyed by SourceID)
	UPROPERTY()
	TMap<FName, FMGSpatialSoundSource> SoundSources;

	/// Calculated reflections for each source
	UPROPERTY()
	TMap<FName, TArray<FMGAudioReflection>> SourceReflections;

	/// Current listener state
	UPROPERTY()
	FMGListenerState ListenerState;

	/// Current spatial audio settings
	UPROPERTY()
	FMGSpatialAudioSettings Settings;

	/// Preset acoustic settings for each environment type
	UPROPERTY()
	TMap<EMGAcousticEnvironment, FMGAcousticZone> EnvironmentPresets;

	/// Timer for throttling occlusion updates
	UPROPERTY()
	float OcclusionUpdateTimer = 0.0f;

	/// Timer handle for spatial tick
	FTimerHandle SpatialTickHandle;
};

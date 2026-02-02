// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGCrowdAudioSubsystem.h
 * @brief Crowd and Spectator Audio Subsystem for Midnight Grind
 *
 * This file defines the crowd audio system that creates dynamic spectator
 * ambiance during races. The system simulates crowds of spectators that react
 * to race events in real-time, creating an immersive atmosphere.
 *
 * @section crowd_features Key Features
 *
 * - **Dynamic Reactions**: Crowds react to overtakes, crashes, near-misses, and more
 * - **Mood System**: Overall crowd mood affects ambient volume and reaction intensity
 * - **Spatial Zones**: Different areas (grandstands, roadside) have unique crowd characteristics
 * - **Event Propagation**: Crowd waves spread outward from exciting events
 * - **Race Integration**: Automatic reactions to race milestones (final lap, photo finish)
 *
 * @section crowd_architecture Architecture
 *
 * This is a WorldSubsystem that manages crowd audio per-level. Crowd zones are
 * registered by level designers and the system handles audio playback and reactions.
 *
 * @section crowd_concepts Core Concepts
 *
 * - **Crowd Zone**: A spatial area with spectators (e.g., grandstand, roadside section)
 * - **Mood**: The overall emotional state of the crowd (calm, excited, cheering, etc.)
 * - **Excitement Level**: A 0-1 value representing how excited the crowd is
 * - **Crowd Wave**: A propagating audio effect that spreads from event locations
 *
 * @section crowd_usage Basic Usage
 *
 * @code
 * // Get the subsystem
 * UMGCrowdAudioSubsystem* CrowdSys = GetWorld()->GetSubsystem<UMGCrowdAudioSubsystem>();
 *
 * // Register crowd zones (usually done by level actors)
 * CrowdSys->RegisterCrowdZone(GrandstandZone);
 *
 * // Trigger events from race logic
 * CrowdSys->TriggerCrowdEvent(EMGCrowdEventType::Overtake, OvertakeLocation);
 *
 * // Integrate with race state
 * CrowdSys->OnFinalLapStarted();
 * @endcode
 *
 * @see UMGAudioSubsystem
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundBase.h"
#include "MGCrowdAudioSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Overall emotional mood of the crowd
 *
 * The mood affects the ambient crowd sounds and how intensely the crowd
 * reacts to events. Mood transitions are smoothed to avoid jarring audio changes.
 */
UENUM(BlueprintType)
enum class EMGCrowdMood : uint8
{
	Calm,           ///< Relaxed crowd, quiet ambient murmur (pre-race, caution periods)
	Excited,        ///< Elevated energy, louder ambient (race in progress)
	Cheering,       ///< Active cheering (responding to positive event)
	Gasping,        ///< Collective gasp/shock (near-miss, dangerous moment)
	Booing,         ///< Crowd disapproval (unfair play, controversial moment)
	Celebrating,    ///< Victory celebration (race finish, podium)
	Tense,          ///< Anticipation/tension (close battle, final corners)
	Disappointed    ///< Letdown (favorite driver out, anticlimactic finish)
};

/**
 * @brief Types of events that trigger crowd reactions
 *
 * The race system should call TriggerCrowdEvent() with the appropriate
 * event type when these situations occur.
 */
UENUM(BlueprintType)
enum class EMGCrowdEventType : uint8
{
	RaceStart,      ///< Race green flag/start
	RaceFinish,     ///< Checkered flag for race winner
	Overtake,       ///< One car passes another
	NearMiss,       ///< Very close call between vehicles
	Crash,          ///< Vehicle crash or collision
	DriftCombo,     ///< Extended drift combo (style points)
	LeadChange,     ///< New race leader
	FinalLap,       ///< Final lap begins
	PhotoFinish,    ///< Extremely close finish
	Podium,         ///< Podium celebration
	PlayerWin,      ///< Player won the race
	PlayerCrash,    ///< Player vehicle crashed
	BigJump,        ///< Vehicle catches big air
	NitroActivation,///< Nitro boost activated
	PoliceEscape    ///< Successfully evaded police
};

/**
 * @brief Types of crowd zones in the world
 *
 * Different zone types have different ambient characteristics and
 * reaction intensities.
 */
UENUM(BlueprintType)
enum class EMGCrowdZoneType : uint8
{
	StartFinish,    ///< Start/finish line grandstands (largest, loudest crowds)
	Grandstand,     ///< General grandstand seating areas
	Roadside,       ///< Spectators along the road (smaller, scattered)
	Overpass,       ///< Crowds on bridges/overpasses watching from above
	Spectator,      ///< General spectator area
	VIP,            ///< VIP/premium viewing areas (smaller, more refined reactions)
	PitLane         ///< Pit lane crew and observers
};

// ============================================================================
// DATA STRUCTURES - ZONES
// ============================================================================

/**
 * @brief Defines a spatial area where crowd audio plays
 *
 * Crowd zones are placed by level designers to define where spectators are located.
 * Each zone has its own ambient loop and can have customized sounds for reactions.
 */
USTRUCT(BlueprintType)
struct FMGCrowdZone
{
	GENERATED_BODY()

	/// Unique identifier for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	/// Center point of the crowd zone in world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Radius of the zone's influence
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 500.0f;

	/// Type of crowd zone (affects behavior and sounds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrowdZoneType ZoneType = EMGCrowdZoneType::Roadside;

	/// Number of simulated spectators (affects volume and intensity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrowdDensity = 100;

	/// Volume multiplier for this zone's sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseVolume = 1.0f;

	/// How strongly this zone reacts to events (1.0 = normal, 2.0 = extra loud)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExcitementMultiplier = 1.0f;

	/// Ambient crowd loop sound for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> AmbientLoop;

	/// Array of cheer sounds to randomly select from
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<USoundBase>> CheerSounds;

	/// Array of gasp sounds for dramatic moments
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<USoundBase>> GaspSounds;

	/// Array of boo sounds for negative reactions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<USoundBase>> BooSounds;
};

// ============================================================================
// DATA STRUCTURES - REACTIONS
// ============================================================================

/**
 * @brief Configuration for how crowds react to specific events
 *
 * Maps event types to reaction behaviors including sounds, mood changes,
 * and timing parameters.
 */
USTRUCT(BlueprintType)
struct FMGCrowdReaction
{
	GENERATED_BODY()

	/// The event that triggers this reaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrowdEventType TriggerEvent = EMGCrowdEventType::Overtake;

	/// What mood the crowd transitions to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrowdMood ResultingMood = EMGCrowdMood::Cheering;

	/// Intensity of the reaction (affects volume and duration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 1.0f;

	/// How long the reaction lasts in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	/// Minimum time between reactions of this type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 1.0f;

	/// Sound to play for this reaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ReactionSound;

	/// If true, all zones react; if false, only nearby zones
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsAllZones = false;

	/// Maximum distance for nearby zone reactions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 2000.0f;
};

// ============================================================================
// DATA STRUCTURES - STATE
// ============================================================================

/**
 * @brief Current runtime state of the crowd system
 *
 * Represents the global crowd state, including mood and excitement levels.
 */
USTRUCT(BlueprintType)
struct FMGCrowdState
{
	GENERATED_BODY()

	/// Current emotional mood of the crowd
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrowdMood CurrentMood = EMGCrowdMood::Calm;

	/// Excitement level from 0.0 (calm) to 1.0 (peak excitement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExcitementLevel = 0.0f;

	/// Tension level from 0.0 (relaxed) to 1.0 (on edge)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TensionLevel = 0.0f;

	/// Current crowd volume (interpolated toward TargetVolume)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentVolume = 0.5f;

	/// Target volume the crowd is transitioning toward
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetVolume = 0.5f;

	/// ID of the zone the listener is currently in (or nearest to)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActiveZoneID = NAME_None;

	/// Seconds since the last crowd reaction (for cooldown tracking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeSinceLastReaction = 0.0f;
};

/**
 * @brief Settings for crowd wave propagation effect
 *
 * Crowd waves are audio effects that spread outward from event locations,
 * simulating the wave of reaction spreading through the crowd.
 */
USTRUCT(BlueprintType)
struct FMGCrowdWaveSettings
{
	GENERATED_BODY()

	/// Enable crowd wave effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/// Speed at which the wave propagates (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveSpeed = 500.0f;

	/// How quickly the wave fades as it propagates (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveDecay = 0.1f;

	/// Minimum event intensity required to trigger a wave
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinIntensityForWave = 0.5f;
};

// ============================================================================
// DELEGATES
// ============================================================================

/** @brief Broadcast when the overall crowd mood changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdMoodChanged, EMGCrowdMood, OldMood, EMGCrowdMood, NewMood);

/** @brief Broadcast when the crowd reacts to an event */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdReaction, EMGCrowdEventType, Event, float, Intensity);

/** @brief Broadcast when the player enters a crowd zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrowdZoneEntered, const FMGCrowdZone&, Zone);

/** @brief Broadcast when the player exits a crowd zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrowdZoneExited, const FMGCrowdZone&, Zone);

/** @brief Broadcast when crowd excitement level changes significantly */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExcitementChanged, float, NewExcitement);

// ============================================================================
// CROWD AUDIO SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Crowd and spectator audio management subsystem
 *
 * Manages all crowd-related audio including ambient crowd sounds,
 * dynamic reactions to race events, and spatial crowd zones.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCrowdAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initialize the crowd audio subsystem
	 * Sets up default reactions and starts the crowd tick timer.
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
	 *  Functions for registering and managing crowd zones.
	 */
	///@{

	/**
	 * @brief Register a crowd zone (usually called by zone actors on BeginPlay)
	 * @param Zone The zone configuration to register
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Zone")
	void RegisterCrowdZone(const FMGCrowdZone& Zone);

	/**
	 * @brief Remove a crowd zone
	 * @param ZoneID The ID of the zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Zone")
	void UnregisterCrowdZone(FName ZoneID);

	/** @brief Get all registered crowd zones */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Zone")
	TArray<FMGCrowdZone> GetAllCrowdZones() const;

	/**
	 * @brief Get a specific zone by ID
	 * @param ZoneID The zone to find
	 */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Zone")
	FMGCrowdZone GetCrowdZone(FName ZoneID) const;

	/**
	 * @brief Find the zone closest to a world location
	 * @param Location World position to search from
	 */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Zone")
	FMGCrowdZone GetNearestCrowdZone(FVector Location) const;

	/**
	 * @brief Change the crowd density of a zone at runtime
	 * @param ZoneID The zone to modify
	 * @param Density New crowd density value
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Zone")
	void SetZoneDensity(FName ZoneID, int32 Density);

	///@}

	// ==========================================
	// EVENT TRIGGERS
	// ==========================================
	/** @name Event Triggers
	 *  Functions for triggering crowd reactions to race events.
	 *  Call these from race logic when events occur.
	 */
	///@{

	/**
	 * @brief Trigger a crowd reaction at a specific location
	 * Nearby zones will react based on distance.
	 * @param Event The type of event that occurred
	 * @param EventLocation World position where the event happened
	 * @param Intensity How intense the reaction should be (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Events")
	void TriggerCrowdEvent(EMGCrowdEventType Event, FVector EventLocation, float Intensity = 1.0f);

	/**
	 * @brief Trigger a reaction in all zones simultaneously
	 * Use for major events like race start/finish.
	 * @param Event The type of event
	 * @param Intensity Reaction intensity
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Events")
	void TriggerGlobalCrowdEvent(EMGCrowdEventType Event, float Intensity = 1.0f);

	/**
	 * @brief Trigger a reaction in a specific zone only
	 * @param ZoneID The zone to trigger
	 * @param Event The type of event
	 * @param Intensity Reaction intensity
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Events")
	void TriggerZoneCrowdEvent(FName ZoneID, EMGCrowdEventType Event, float Intensity = 1.0f);

	///@}

	// ==========================================
	// MOOD CONTROL
	// ==========================================
	/** @name Mood Control
	 *  Functions for managing overall crowd mood and energy.
	 */
	///@{

	/**
	 * @brief Set the overall crowd mood (affects all zones)
	 * @param Mood The new mood
	 * @param TransitionTime Seconds to transition to new mood
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Mood")
	void SetGlobalMood(EMGCrowdMood Mood, float TransitionTime = 1.0f);

	/** @brief Get the current crowd mood */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Mood")
	EMGCrowdMood GetCurrentMood() const { return CrowdState.CurrentMood; }

	/**
	 * @brief Set the crowd excitement level directly
	 * @param Level Excitement from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Mood")
	void SetExcitementLevel(float Level);

	/** @brief Get current excitement level */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Mood")
	float GetExcitementLevel() const { return CrowdState.ExcitementLevel; }

	/**
	 * @brief Set the crowd tension level
	 * @param Level Tension from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Mood")
	void SetTensionLevel(float Level);

	/** @brief Get current tension level */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Mood")
	float GetTensionLevel() const { return CrowdState.TensionLevel; }

	///@}

	// ==========================================
	// RACE STATE INTEGRATION
	// ==========================================
	/** @name Race State Integration
	 *  Convenience functions for common race events.
	 *  These handle triggering appropriate crowd reactions automatically.
	 */
	///@{

	/** @brief Call when the race starts */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnRaceStarted();

	/**
	 * @brief Call when the race finishes
	 * @param bPlayerWon True if the player won
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnRaceFinished(bool bPlayerWon);

	/** @brief Call when the final lap begins */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnFinalLapStarted();

	/**
	 * @brief Call when the race leader changes
	 * @param NewLeaderID ID of the new leader
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnLeadChange(const FString& NewLeaderID);

	/**
	 * @brief Call when an overtake occurs
	 * @param OvertakerID ID of the passing vehicle
	 * @param OvertakenID ID of the passed vehicle
	 * @param Location Where the overtake happened
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnOvertake(const FString& OvertakerID, const FString& OvertakenID, FVector Location);

	/**
	 * @brief Call when a crash occurs
	 * @param PlayerID ID of the crashed vehicle
	 * @param Location Crash location
	 * @param Severity How severe the crash was (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnCrash(const FString& PlayerID, FVector Location, float Severity);

	///@}

	// ==========================================
	// LISTENER
	// ==========================================
	/** @name Listener
	 *  Functions for tracking the audio listener position.
	 */
	///@{

	/**
	 * @brief Update the listener position (for determining active zones)
	 * @param Location Current listener world position
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Listener")
	void UpdateListenerLocation(FVector Location);

	/** @brief Get the current crowd state */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Listener")
	FMGCrowdState GetCrowdState() const { return CrowdState; }

	///@}

	// ==========================================
	// CONFIGURATION
	// ==========================================
	/** @name Configuration
	 *  Functions for configuring crowd behavior.
	 */
	///@{

	/**
	 * @brief Register a custom reaction configuration
	 * @param Reaction The reaction to register
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Config")
	void RegisterReaction(const FMGCrowdReaction& Reaction);

	/**
	 * @brief Configure crowd wave behavior
	 * @param Settings New wave settings
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Config")
	void SetWaveSettings(const FMGCrowdWaveSettings& Settings);

	/** @brief Get current wave settings */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Config")
	FMGCrowdWaveSettings GetWaveSettings() const { return WaveSettings; }

	///@}

	// ==========================================
	// VOLUME CONTROL
	// ==========================================
	/** @name Volume Control
	 *  Functions for controlling crowd audio volume.
	 */
	///@{

	/**
	 * @brief Set the master crowd volume
	 * @param Volume Volume from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Volume")
	void SetCrowdVolume(float Volume);

	/** @brief Get current crowd volume */
	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Volume")
	float GetCrowdVolume() const { return MasterCrowdVolume; }

	/**
	 * @brief Fade out all crowd audio
	 * @param FadeTime Duration of fade in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Volume")
	void FadeOutCrowd(float FadeTime = 1.0f);

	/**
	 * @brief Fade in all crowd audio
	 * @param FadeTime Duration of fade in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Volume")
	void FadeInCrowd(float FadeTime = 1.0f);

	///@}

	// ==========================================
	// EVENT DELEGATES
	// ==========================================
	/** @name Events
	 *  Delegates for reacting to crowd state changes.
	 */
	///@{

	/// Fires when the overall crowd mood changes
	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnCrowdMoodChanged OnCrowdMoodChanged;

	/// Fires when the crowd reacts to an event
	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnCrowdReaction OnCrowdReaction;

	/// Fires when the player enters a crowd zone
	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnCrowdZoneEntered OnCrowdZoneEntered;

	/// Fires when the player exits a crowd zone
	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnCrowdZoneExited OnCrowdZoneExited;

	/// Fires when excitement level changes significantly
	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnExcitementChanged OnExcitementChanged;

	///@}

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** @brief Called periodically to update crowd audio */
	void OnCrowdTick();

	/** @brief Update audio for all zones based on listener position */
	void UpdateZoneAudio();

	/** @brief Update crowd mood based on recent events */
	void UpdateMood();

	/** @brief Gradually reduce excitement over time */
	void DecayExcitement(float MGDeltaTime);

	/** @brief Process a crowd wave effect spreading from an origin */
	void ProcessCrowdWave(FVector Origin, float Intensity);

	/** @brief Play the reaction sound for an event */
	void PlayReactionSound(const FMGCrowdReaction& Reaction, FVector Location);

	/** @brief Find the reaction configuration for an event type */
	FMGCrowdReaction GetReactionForEvent(EMGCrowdEventType Event) const;

	/** @brief Set up default reaction configurations */
	void InitializeDefaultReactions();

	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/// All registered crowd zones
	UPROPERTY()
	TArray<FMGCrowdZone> CrowdZones;

	/// Configured reaction behaviors
	UPROPERTY()
	TArray<FMGCrowdReaction> Reactions;

	/// Current runtime crowd state
	UPROPERTY()
	FMGCrowdState CrowdState;

	/// Crowd wave settings
	UPROPERTY()
	FMGCrowdWaveSettings WaveSettings;

	/// Current listener position
	UPROPERTY()
	FVector ListenerLocation = FVector::ZeroVector;

	/// Master volume for all crowd sounds
	UPROPERTY()
	float MasterCrowdVolume = 1.0f;

	/// Rate at which excitement naturally decays (per second)
	UPROPERTY()
	float ExcitementDecayRate = 0.1f;

	/// Rate at which tension naturally decays (per second)
	UPROPERTY()
	float TensionDecayRate = 0.05f;

	/// Cooldown tracking for each event type to prevent spam
	UPROPERTY()
	TMap<EMGCrowdEventType, float> EventCooldowns;

	/// Timer handle for crowd tick
	FTimerHandle CrowdTickHandle;
};

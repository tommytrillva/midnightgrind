// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGDynamicMixSubsystem.h
 * @brief Dynamic Audio Mixing Subsystem for Midnight Grind
 *
 * This file defines the central audio mixing system that controls the overall audio
 * balance and processing for the entire game. It manages audio buses, mix snapshots,
 * ducking rules, and dynamic intensity-based mixing that responds to gameplay.
 *
 * @section mix_features Key Features
 *
 * - **Audio Buses**: Separate volume control for different audio categories (Music, SFX, Engine, etc.)
 * - **Mix Snapshots**: Predefined mixing states that can be transitioned between smoothly
 * - **Dynamic Intensity**: Automatic mix adjustments based on gameplay intensity (speed, position, etc.)
 * - **Ducking System**: Automatic volume reduction of background audio when important sounds play
 * - **Audio Zones**: Spatial areas with custom mix settings (e.g., muffled audio in tunnels)
 * - **Slow Motion Audio**: Special processing for bullet-time/slow-motion effects
 * - **State Machine**: Audio states that correspond to game states (Menu, Racing, Victory, etc.)
 *
 * @section mix_architecture Architecture
 *
 * This is a WorldSubsystem that acts as the master mixer for all game audio. It sits
 * above other audio subsystems and controls their final output levels and processing.
 *
 * Audio signal flow:
 * 1. Individual sound sources play through their respective buses
 * 2. Buses have individual volume, filtering, and effects
 * 3. Ducking rules automatically adjust bus volumes when triggered
 * 4. Snapshots define complete mix states that can be blended between
 * 5. Master volume and global effects are applied last
 *
 * @section mix_concepts Core Concepts
 *
 * - **Audio Bus**: A channel that groups similar sounds for collective processing (e.g., all music)
 * - **Snapshot**: A saved state of all bus settings that can be recalled instantly
 * - **Ducking**: Automatically lowering one sound when another plays (e.g., music ducks for voice)
 * - **Intensity**: A calculated value (0-1) representing how "intense" the current gameplay is
 * - **Audio State**: High-level game state that determines the base mix (Menu, Racing, etc.)
 *
 * @section mix_buses Audio Bus Types
 *
 * | Bus Type    | Description                                    |
 * |-------------|------------------------------------------------|
 * | Master      | Final output, affects all audio                |
 * | Music       | Background music and radio                     |
 * | Ambience    | Environmental/ambient sounds                   |
 * | SFX         | General sound effects                          |
 * | Vehicle     | Tire squeals, collision sounds                 |
 * | Engine      | Engine audio (separate for fine control)       |
 * | UI          | Menu and interface sounds                      |
 * | Voice       | Voice lines and announcer                      |
 * | Crowd       | Spectator and crowd audio                      |
 * | Weather     | Rain, wind, thunder                            |
 * | Police      | Police sirens and radio chatter                |
 * | Cinematics  | Cutscene audio (highest priority)              |
 *
 * @section mix_usage Basic Usage
 *
 * @code
 * // Get the subsystem
 * UMGDynamicMixSubsystem* MixSys = GetWorld()->GetSubsystem<UMGDynamicMixSubsystem>();
 *
 * // Set the audio state based on game state
 * MixSys->SetAudioState(EMGAudioState::Racing, 1.0f);
 *
 * // Control individual buses
 * MixSys->SetBusVolume(EMGAudioBusType::Music, 0.5f);
 *
 * // Transition to a predefined snapshot
 * MixSys->TransitionToSnapshot(FName("IntenseRacing"), 2.0f);
 *
 * // Update intensity from race logic
 * FMGAudioIntensityParams Params;
 * Params.CurrentSpeed = Vehicle->GetSpeed();
 * Params.RacePosition = RaceManager->GetPosition();
 * MixSys->UpdateIntensityParams(Params);
 * @endcode
 *
 * @see UMGAudioSubsystem
 * @see UMGMusicSubsystem
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGDynamicMixSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Audio bus categories for separate volume control
 *
 * Each bus groups related sounds together for collective mixing.
 * Buses can be individually controlled, muted, and processed.
 */
UENUM(BlueprintType)
enum class EMGAudioBusType : uint8
{
	Master,         ///< Final output bus - affects all audio
	Music,          ///< Background music and radio stations
	Ambience,       ///< Environmental and ambient sounds
	SFX,            ///< General sound effects (collisions, pickups, etc.)
	Vehicle,        ///< Vehicle-related sounds (tires, suspension, etc.)
	Engine,         ///< Engine audio (separate for fine-tuned control)
	UI,             ///< User interface sounds (button clicks, notifications)
	Voice,          ///< Voice lines, announcer, dialogue
	Crowd,          ///< Spectator and crowd audio
	Weather,        ///< Rain, wind, thunder sounds
	Police,         ///< Police sirens, radio chatter, pursuit audio
	Cinematics      ///< Cutscene audio (highest priority)
};

/**
 * @brief High-level audio states corresponding to game situations
 *
 * Each state has associated mix settings that automatically apply.
 * Transitioning between states triggers smooth audio crossfades.
 */
UENUM(BlueprintType)
enum class EMGAudioState : uint8
{
	Idle,           ///< Default state - balanced mix, no special processing
	Cruising,       ///< Free roam, relaxed driving - music prominent
	Racing,         ///< Active race - engine/SFX boosted, crowd active
	Intense,        ///< High-intensity moments - enhanced bass, compressed dynamics
	PoliceChase,    ///< Police pursuit - sirens prominent, tense music
	PhotoMode,      ///< Photo mode - ambient only, very quiet
	Cutscene,       ///< Cutscene playing - cinematics bus priority, duck others
	Menu,           ///< In menus - UI sounds prominent, gameplay audio muted
	Loading,        ///< Loading screen - ambient music, muted gameplay
	Victory,        ///< Post-race win - celebration audio, crowd cheering
	Defeat          ///< Post-race loss - somber audio treatment
};

/**
 * @brief Priority levels for mix snapshots
 *
 * Higher priority snapshots override lower ones when multiple
 * are active. Used for temporary overrides (e.g., cutscene over racing).
 */
UENUM(BlueprintType)
enum class EMGAudioPriority : uint8
{
	Low,            ///< Background priority - easily overridden
	Normal,         ///< Default priority for most gameplay situations
	High,           ///< Important events (race start, finish)
	Critical,       ///< Cannot be interrupted (cutscenes, announcements)
	Override        ///< Absolute priority - used for debugging/testing
};

// ============================================================================
// DATA STRUCTURES - BUS SETTINGS
// ============================================================================

/**
 * @brief Complete settings for a single audio bus
 *
 * Defines volume, filtering, and effects for a bus. These settings
 * can be saved in snapshots for quick recall.
 */
USTRUCT(BlueprintType)
struct FMGAudioBusSettings
{
	GENERATED_BODY()

	/// Which bus these settings apply to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAudioBusType BusType = EMGAudioBusType::Master;

	/// Volume level (0.0 = silent, 1.0 = normal, 2.0+ = boosted)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.0f;

	/// Pitch multiplier (1.0 = normal, <1.0 = lower, >1.0 = higher)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch = 1.0f;

	/// Low-pass filter cutoff frequency in Hz (20000 = no filtering)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowPassFrequency = 20000.0f;

	/// High-pass filter cutoff frequency in Hz (20 = no filtering)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighPassFrequency = 20.0f;

	/// Amount sent to reverb effect (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbSend = 0.0f;

	/// Amount sent to delay effect (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelaySend = 0.0f;

	/// Whether this bus is completely muted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMuted = false;

	/// If true, only this bus plays (for debugging/testing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSolo = false;
};

// ============================================================================
// DATA STRUCTURES - SNAPSHOTS
// ============================================================================

/**
 * @brief A saved state of all bus settings for quick recall
 *
 * Snapshots allow you to save and restore complete mix configurations.
 * Use them for different game states (racing, menu, cutscene, etc.).
 *
 * @section snapshot_example Example Usage
 * - "RacingMix": Engine loud, music moderate, crowd active
 * - "MenuMix": Music prominent, gameplay audio muted
 * - "CutsceneMix": Cinematics priority, everything else ducked
 */
USTRUCT(BlueprintType)
struct FMGAudioMixSnapshot
{
	GENERATED_BODY()

	/// Unique identifier for this snapshot (e.g., "Snapshot_Racing")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SnapshotName;

	/// Settings for each bus in this snapshot
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGAudioBusType, FMGAudioBusSettings> BusSettings;

	/// Default time to transition to this snapshot
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionTime = 1.0f;

	/// Priority level (higher priority snapshots override lower)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAudioPriority Priority = EMGAudioPriority::Normal;
};

// ============================================================================
// DATA STRUCTURES - INTENSITY
// ============================================================================

/**
 * @brief Parameters used to calculate gameplay intensity
 *
 * The system uses these values to determine how "intense" the current
 * gameplay is, which affects the dynamic mix (e.g., bass boost when close
 * to other racers, enhanced audio during nitro).
 *
 * Update these values from your race/game manager each frame.
 */
USTRUCT(BlueprintType)
struct FMGAudioIntensityParams
{
	GENERATED_BODY()

	/// Current vehicle speed (used for speed-based intensity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	/// Maximum possible speed (for normalization)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 200.0f;

	/// Current race position (1 = first place)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacePosition = 1;

	/// Total number of racers in the race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRacers = 8;

	/// Time gap to the race leader (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToLeader = 0.0f;

	/// Time gap to the car directly ahead (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToAhead = 0.0f;

	/// Current police heat level (0 = none, higher = more intense)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PoliceHeatLevel = 0;

	/// Whether nitro boost is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInNitro = false;

	/// Whether a near-miss just occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNearMiss = false;

	/// Whether currently drifting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrifting = false;

	/// Progress through current lap (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapProgress = 0.0f;

	/// Whether this is the final lap
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFinalLap = false;
};

// ============================================================================
// DATA STRUCTURES - DUCKING
// ============================================================================

/**
 * @brief A rule for automatic volume ducking between buses
 *
 * Ducking automatically lowers one sound when another plays. Common uses:
 * - Duck music when voice plays (so dialogue is clear)
 * - Duck ambient when engine is loud (focus on vehicle)
 * - Duck everything when cinematics play
 *
 * @section ducking_example Example
 * When voice plays (SourceBus), music (TargetBus) volume is reduced by
 * DuckAmount over AttackTime seconds, then recovers over ReleaseTime.
 */
USTRUCT(BlueprintType)
struct FMGAudioDuckingRule
{
	GENERATED_BODY()

	/// The bus that triggers ducking when it plays
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAudioBusType SourceBus = EMGAudioBusType::Voice;

	/// The bus that gets ducked (volume reduced)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAudioBusType TargetBus = EMGAudioBusType::Music;

	/// How much to reduce target volume (0.0 = full duck, 1.0 = no duck)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DuckAmount = 0.5f;

	/// Time to reach full duck when source starts (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackTime = 0.1f;

	/// Time to return to normal when source stops (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReleaseTime = 0.5f;

	/// Minimum source level to trigger ducking
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Threshold = 0.1f;

	/// Whether this rule is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;
};

// ============================================================================
// DATA STRUCTURES - EFFECTS
// ============================================================================

/**
 * @brief A preset configuration for audio effects
 *
 * Defines reverb, delay, and other processing settings that can be
 * applied to the master output or individual buses.
 */
USTRUCT(BlueprintType)
struct FMGAudioEffectPreset
{
	GENERATED_BODY()

	/// Unique name for this preset
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetName;

	/// Reverb decay time in seconds (how long reverb tail lasts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbDecayTime = 1.5f;

	/// Reverb wet level (how much reverb is mixed in, 0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbWetLevel = 0.3f;

	/// Reverb dry level (original sound level, 0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbDryLevel = 0.7f;

	/// Delay time in seconds (echo timing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayTime = 0.0f;

	/// Delay feedback (how many repeats, 0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayFeedback = 0.0f;

	/// Chorus effect depth (0.0 = none)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChorusDepth = 0.0f;

	/// Distortion amount (0.0 = clean, 1.0 = heavy distortion)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistortionAmount = 0.0f;
};

// ============================================================================
// DATA STRUCTURES - ZONES
// ============================================================================

/**
 * @brief A spatial area with custom mix and effects settings
 *
 * Audio zones allow location-based mixing changes. For example, entering
 * a tunnel might apply reverb and low-pass filtering to simulate the
 * enclosed acoustic space.
 */
USTRUCT(BlueprintType)
struct FMGAudioZone
{
	GENERATED_BODY()

	/// Unique identifier for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	/// Center point of the zone in world space
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	/// Radius of the zone's influence
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 1000.0f;

	/// Mix snapshot to apply when inside this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGAudioMixSnapshot ZoneSnapshot;

	/// Audio effects preset for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGAudioEffectPreset ZoneEffects;

	/// Distance over which to blend when entering/exiting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendDistance = 200.0f;
};

// ============================================================================
// DELEGATES
// ============================================================================

/** @brief Broadcast when the high-level audio state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioStateChanged, EMGAudioState, OldState, EMGAudioState, NewState);

/** @brief Broadcast when a snapshot transition begins */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSnapshotTransitionStarted, FName, FromSnapshot, FName, ToSnapshot);

/** @brief Broadcast when a snapshot transition completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSnapshotTransitionComplete, FName, SnapshotName);

/** @brief Broadcast when calculated intensity value changes significantly */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntensityChanged, float, NewIntensity);

/** @brief Broadcast when the listener enters an audio zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioZoneEntered, const FMGAudioZone&, Zone);

/** @brief Broadcast when the listener exits an audio zone */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioZoneExited, const FMGAudioZone&, Zone);

// ============================================================================
// DYNAMIC MIX SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Central audio mixing and processing subsystem
 *
 * Controls the overall audio mix, including bus volumes, snapshots,
 * ducking, intensity-based mixing, and audio zones. This is the master
 * controller for the game's audio balance.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDynamicMixSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initialize the dynamic mix subsystem
	 * Sets up default bus settings, snapshots, and ducking rules.
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
	// STATE MANAGEMENT
	// ==========================================
	/** @name State Management
	 *  Functions for controlling the high-level audio state.
	 *  States correspond to game situations and have associated mix settings.
	 */
	///@{

	/**
	 * @brief Set the current audio state
	 * @param NewState The new audio state
	 * @param TransitionTime Seconds to transition to the new state
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|State")
	void SetAudioState(EMGAudioState NewState, float TransitionTime = 1.0f);

	/** @brief Get the current audio state */
	UFUNCTION(BlueprintPure, Category = "Audio|State")
	EMGAudioState GetCurrentAudioState() const { return CurrentState; }

	/**
	 * @brief Push a temporary state onto the state stack
	 * Use this for temporary overrides (e.g., cutscene during race).
	 * @param State The state to push
	 * @param TransitionTime Seconds to transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|State")
	void PushAudioState(EMGAudioState State, float TransitionTime = 1.0f);

	/**
	 * @brief Pop the top state from the stack, returning to previous state
	 * @param TransitionTime Seconds to transition back
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|State")
	void PopAudioState(float TransitionTime = 1.0f);

	///@}

	// ==========================================
	// BUS CONTROL
	// ==========================================
	/** @name Bus Control
	 *  Functions for controlling individual audio buses.
	 */
	///@{

	/**
	 * @brief Set the volume of a specific bus
	 * @param Bus The bus to adjust
	 * @param Volume New volume (0.0 to 1.0+)
	 * @param FadeTime Seconds to fade to new volume
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void SetBusVolume(EMGAudioBusType Bus, float Volume, float FadeTime = 0.0f);

	/**
	 * @brief Get the current volume of a bus
	 * @param Bus The bus to query
	 */
	UFUNCTION(BlueprintPure, Category = "Audio|Bus")
	float GetBusVolume(EMGAudioBusType Bus) const;

	/**
	 * @brief Mute or unmute a specific bus
	 * @param Bus The bus to mute/unmute
	 * @param bMuted True to mute
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void SetBusMuted(EMGAudioBusType Bus, bool bMuted);

	/**
	 * @brief Check if a bus is muted
	 * @param Bus The bus to check
	 */
	UFUNCTION(BlueprintPure, Category = "Audio|Bus")
	bool IsBusMuted(EMGAudioBusType Bus) const;

	/**
	 * @brief Apply complete settings to a bus
	 * @param Bus The bus to configure
	 * @param Settings The settings to apply
	 * @param TransitionTime Seconds to transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void SetBusSettings(EMGAudioBusType Bus, const FMGAudioBusSettings& Settings, float TransitionTime = 0.0f);

	/**
	 * @brief Get the current settings for a bus
	 * @param Bus The bus to query
	 */
	UFUNCTION(BlueprintPure, Category = "Audio|Bus")
	FMGAudioBusSettings GetBusSettings(EMGAudioBusType Bus) const;

	/**
	 * @brief Apply a low-pass filter to a bus (muffles high frequencies)
	 * @param Bus The bus to filter
	 * @param Frequency Cutoff frequency in Hz (lower = more muffled)
	 * @param TransitionTime Seconds to apply filter
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void ApplyLowPassFilter(EMGAudioBusType Bus, float Frequency, float TransitionTime = 0.0f);

	/**
	 * @brief Remove the low-pass filter from a bus
	 * @param Bus The bus to clear
	 * @param TransitionTime Seconds to remove filter
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void ClearLowPassFilter(EMGAudioBusType Bus, float TransitionTime = 0.0f);

	///@}

	// ==========================================
	// SNAPSHOTS
	// ==========================================
	/** @name Snapshots
	 *  Functions for managing and transitioning between mix snapshots.
	 */
	///@{

	/**
	 * @brief Transition to a named snapshot
	 * @param SnapshotName The snapshot to transition to
	 * @param TransitionTime Seconds to crossfade
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Snapshot")
	void TransitionToSnapshot(FName SnapshotName, float TransitionTime = 1.0f);

	/**
	 * @brief Register a snapshot for later use
	 * @param Snapshot The snapshot configuration to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Snapshot")
	void RegisterSnapshot(const FMGAudioMixSnapshot& Snapshot);

	/** @brief Get the name of the currently active snapshot */
	UFUNCTION(BlueprintPure, Category = "Audio|Snapshot")
	FName GetCurrentSnapshotName() const { return CurrentSnapshotName; }

	/** @brief Check if a snapshot transition is in progress */
	UFUNCTION(BlueprintPure, Category = "Audio|Snapshot")
	bool IsTransitioning() const { return bIsTransitioning; }

	/** @brief Get the progress of current transition (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Audio|Snapshot")
	float GetTransitionProgress() const { return TransitionProgress; }

	///@}

	// ==========================================
	// INTENSITY
	// ==========================================
	/** @name Intensity
	 *  Functions for dynamic intensity-based mixing.
	 *  Intensity affects the overall "energy" of the audio mix.
	 */
	///@{

	/**
	 * @brief Update intensity parameters from gameplay
	 * Call this each frame with current game state.
	 * @param Params Current gameplay parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Intensity")
	void UpdateIntensityParams(const FMGAudioIntensityParams& Params);

	/** @brief Get the calculated intensity value (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Audio|Intensity")
	float GetCurrentIntensity() const { return CurrentIntensity; }

	/**
	 * @brief Manually override the intensity value
	 * @param Intensity The override value (0.0 to 1.0)
	 * @param Duration How long the override lasts (0 = indefinite)
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Intensity")
	void SetIntensityOverride(float Intensity, float Duration = 0.0f);

	/** @brief Clear the intensity override and return to calculated value */
	UFUNCTION(BlueprintCallable, Category = "Audio|Intensity")
	void ClearIntensityOverride();

	///@}

	// ==========================================
	// DUCKING
	// ==========================================
	/** @name Ducking
	 *  Functions for automatic volume ducking between buses.
	 */
	///@{

	/**
	 * @brief Add a ducking rule
	 * @param Rule The ducking rule configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Ducking")
	void AddDuckingRule(const FMGAudioDuckingRule& Rule);

	/**
	 * @brief Remove a ducking rule
	 * @param SourceBus The source bus of the rule to remove
	 * @param TargetBus The target bus of the rule to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Ducking")
	void RemoveDuckingRule(EMGAudioBusType SourceBus, EMGAudioBusType TargetBus);

	/**
	 * @brief Enable or disable all ducking
	 * @param bEnabled True to enable ducking
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Ducking")
	void SetDuckingEnabled(bool bEnabled);

	/** @brief Check if ducking is enabled */
	UFUNCTION(BlueprintPure, Category = "Audio|Ducking")
	bool IsDuckingEnabled() const { return bDuckingEnabled; }

	///@}

	// ==========================================
	// EFFECTS
	// ==========================================
	/** @name Effects
	 *  Functions for controlling global audio effects.
	 */
	///@{

	/**
	 * @brief Apply an effects preset
	 * @param Preset The preset to apply
	 * @param TransitionTime Seconds to transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
	void ApplyEffectPreset(const FMGAudioEffectPreset& Preset, float TransitionTime = 0.5f);

	/**
	 * @brief Set reverb parameters directly
	 * @param DecayTime Reverb decay time in seconds
	 * @param WetLevel How much reverb to mix in (0.0 to 1.0)
	 * @param TransitionTime Seconds to transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
	void SetReverbSettings(float DecayTime, float WetLevel, float TransitionTime = 0.5f);

	/**
	 * @brief Set the global pitch multiplier (affects all audio)
	 * @param Pitch Pitch multiplier (1.0 = normal)
	 * @param TransitionTime Seconds to transition
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
	void SetGlobalPitch(float Pitch, float TransitionTime = 0.0f);

	/** @brief Get the current global pitch */
	UFUNCTION(BlueprintPure, Category = "Audio|Effects")
	float GetGlobalPitch() const { return GlobalPitch; }

	///@}

	// ==========================================
	// AUDIO ZONES
	// ==========================================
	/** @name Audio Zones
	 *  Functions for location-based mix changes.
	 */
	///@{

	/**
	 * @brief Register an audio zone
	 * @param Zone The zone configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
	void RegisterAudioZone(const FMGAudioZone& Zone);

	/**
	 * @brief Remove an audio zone
	 * @param ZoneID The ID of the zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
	void UnregisterAudioZone(FName ZoneID);

	/**
	 * @brief Update the listener position for zone detection
	 * @param Position World position of the listener
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
	void UpdateListenerPosition(FVector Position);

	/** @brief Get the zone the listener is currently in */
	UFUNCTION(BlueprintPure, Category = "Audio|Zone")
	FMGAudioZone GetCurrentAudioZone() const;

	///@}

	// ==========================================
	// SLOW MOTION
	// ==========================================
	/** @name Slow Motion
	 *  Functions for slow-motion/bullet-time audio effects.
	 */
	///@{

	/**
	 * @brief Apply slow-motion audio effect
	 * Lowers pitch and applies filtering based on time scale.
	 * @param TimeScale Time dilation value (0.5 = half speed)
	 * @param TransitionTime Seconds to apply effect
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|SlowMo")
	void SetSlowMotionAudio(float TimeScale, float TransitionTime = 0.1f);

	/**
	 * @brief Reset audio from slow-motion to normal
	 * @param TransitionTime Seconds to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|SlowMo")
	void ResetSlowMotionAudio(float TransitionTime = 0.1f);

	///@}

	// ==========================================
	// MASTER CONTROL
	// ==========================================
	/** @name Master Control
	 *  Top-level audio controls.
	 */
	///@{

	/**
	 * @brief Set the master volume (affects all audio)
	 * @param Volume Volume from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Master")
	void SetMasterVolume(float Volume);

	/** @brief Get the current master volume */
	UFUNCTION(BlueprintPure, Category = "Audio|Master")
	float GetMasterVolume() const { return MasterVolume; }

	/**
	 * @brief Mute or unmute all audio
	 * @param bMute True to mute all audio
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Master")
	void MuteAll(bool bMute);

	/**
	 * @brief Pause or unpause all audio
	 * @param bPause True to pause all audio
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Master")
	void PauseAllAudio(bool bPause);

	///@}

	// ==========================================
	// EVENT DELEGATES
	// ==========================================
	/** @name Events
	 *  Delegates for reacting to mix system changes.
	 */
	///@{

	/// Fires when the audio state changes
	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnAudioStateChanged OnAudioStateChanged;

	/// Fires when a snapshot transition begins
	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnSnapshotTransitionStarted OnSnapshotTransitionStarted;

	/// Fires when a snapshot transition completes
	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnSnapshotTransitionComplete OnSnapshotTransitionComplete;

	/// Fires when the intensity value changes significantly
	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnIntensityChanged OnIntensityChanged;

	/// Fires when entering an audio zone
	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnAudioZoneEntered OnAudioZoneEntered;

	/// Fires when exiting an audio zone
	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnAudioZoneExited OnAudioZoneExited;

	///@}

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** @brief Called periodically to update mix state */
	void OnMixTick();

	/** @brief Update snapshot transition interpolation */
	void UpdateTransition(float DeltaTime);

	/** @brief Recalculate intensity from current parameters */
	void UpdateIntensity();

	/** @brief Process ducking rules */
	void UpdateDucking();

	/** @brief Check audio zones and apply zone-based mix */
	void UpdateAudioZones();

	/** @brief Apply settings to a specific bus */
	void ApplyBusSettings(EMGAudioBusType Bus, const FMGAudioBusSettings& Settings);

	/**
	 * @brief Calculate intensity value from parameters
	 * @param Params The parameters to calculate from
	 * @return Intensity value from 0.0 to 1.0
	 */
	float CalculateIntensity(const FMGAudioIntensityParams& Params) const;

	/** @brief Set up default snapshots for common states */
	void InitializeDefaultSnapshots();

	/** @brief Set up default settings for all buses */
	void InitializeDefaultBusSettings();

	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/// Current high-level audio state
	UPROPERTY()
	EMGAudioState CurrentState = EMGAudioState::Idle;

	/// Stack of states for push/pop functionality
	UPROPERTY()
	TArray<EMGAudioState> StateStack;

	/// Current settings for each bus
	UPROPERTY()
	TMap<EMGAudioBusType, FMGAudioBusSettings> BusSettingsMap;

	/// Registered snapshots (keyed by name)
	UPROPERTY()
	TMap<FName, FMGAudioMixSnapshot> Snapshots;

	/// Name of the currently active snapshot
	UPROPERTY()
	FName CurrentSnapshotName;

	/// The snapshot currently being applied
	UPROPERTY()
	FMGAudioMixSnapshot CurrentSnapshot;

	/// The snapshot being transitioned to
	UPROPERTY()
	FMGAudioMixSnapshot TargetSnapshot;

	/// Whether a transition is in progress
	UPROPERTY()
	bool bIsTransitioning = false;

	/// Progress through current transition (0.0 to 1.0)
	UPROPERTY()
	float TransitionProgress = 0.0f;

	/// Duration of current transition
	UPROPERTY()
	float TransitionDuration = 1.0f;

	/// Current calculated intensity value
	UPROPERTY()
	float CurrentIntensity = 0.0f;

	/// Latest intensity parameters from gameplay
	UPROPERTY()
	FMGAudioIntensityParams IntensityParams;

	/// Whether intensity override is active
	UPROPERTY()
	bool bIntensityOverride = false;

	/// Manual intensity override value
	UPROPERTY()
	float IntensityOverrideValue = 0.0f;

	/// All ducking rules
	UPROPERTY()
	TArray<FMGAudioDuckingRule> DuckingRules;

	/// Whether ducking is globally enabled
	UPROPERTY()
	bool bDuckingEnabled = true;

	/// Registered audio zones
	UPROPERTY()
	TArray<FMGAudioZone> AudioZones;

	/// ID of the zone the listener is currently in
	UPROPERTY()
	FName CurrentZoneID = NAME_None;

	/// Current listener position for zone detection
	UPROPERTY()
	FVector ListenerPosition = FVector::ZeroVector;

	/// Master volume multiplier
	UPROPERTY()
	float MasterVolume = 1.0f;

	/// Global pitch multiplier
	UPROPERTY()
	float GlobalPitch = 1.0f;

	/// Whether all audio is muted
	UPROPERTY()
	bool bAllMuted = false;

	/// Whether all audio is paused
	UPROPERTY()
	bool bAllPaused = false;

	/// Timer handle for mix tick
	FTimerHandle MixTickHandle;
};

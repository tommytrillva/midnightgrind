// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGEngineAudioSubsystem.h
 * @brief Vehicle Engine Audio Subsystem for Midnight Grind
 *
 * This file defines the engine audio system responsible for creating realistic and
 * immersive vehicle engine sounds. The system supports multiple vehicles simultaneously
 * and provides detailed control over engine sound characteristics.
 *
 * @section engine_audio_features Key Features
 *
 * - **Multi-Layer Engine Sounds**: Blends multiple sound layers based on RPM for realistic engine audio
 * - **Forced Induction**: Turbo spool, blowoff, and supercharger whine sounds
 * - **Dynamic Events**: Backfires, rev limiter hits, gear shifts
 * - **Multiple Vehicles**: Supports player and AI vehicles with LOD-based optimization
 * - **Interior/Exterior Modes**: Different audio treatment based on camera position
 * - **Exhaust Upgrades**: Different exhaust types affect sound character
 *
 * @section engine_audio_architecture Architecture
 *
 * This is a WorldSubsystem, meaning one instance exists per world/level. It manages
 * audio for all vehicles in the world, prioritizing the player vehicle and nearby
 * vehicles based on audibility.
 *
 * The engine sound is built from multiple layers:
 * - **On-Throttle Layers**: Active when accelerating
 * - **Off-Throttle Layers**: Active when coasting/decelerating (engine braking sounds)
 * - **Special Sounds**: Startup, shutdown, backfire, turbo, rev limiter
 *
 * @section engine_audio_usage Basic Usage
 *
 * @code
 * // Get the subsystem
 * UMGEngineAudioSubsystem* EngineSys = GetWorld()->GetSubsystem<UMGEngineAudioSubsystem>();
 *
 * // Register the player's vehicle with an audio profile
 * EngineSys->RegisterVehicle(VehicleID, V8Profile, true);
 *
 * // Update engine state each frame
 * FMGEngineAudioState State;
 * State.CurrentRPM = Vehicle->GetEngineRPM();
 * State.ThrottleInput = Vehicle->GetThrottle();
 * EngineSys->UpdateEngineState(VehicleID, State);
 * @endcode
 *
 * @see UMGAudioSubsystem
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGEngineAudioSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS - ENGINE CONFIGURATION
// ============================================================================

// EMGEngineType - defined in Core/MGSharedTypes.h

/**
 * @brief Exhaust system types affecting sound character
 *
 * Different exhaust configurations change the tone, volume, and character
 * of the engine sound. Players can upgrade exhaust as part of vehicle customization.
 */
UENUM(BlueprintType)
enum class EMGExhaustType : uint8
{
	Stock,          ///< Factory exhaust (quieter, more muffled)
	Sport,          ///< Sport exhaust (slightly louder, more aggressive)
	Performance,    ///< Performance exhaust (loud, raw sound)
	Racing,         ///< Full race exhaust (very loud, crackles and pops)
	Straight,       ///< Straight pipe (no muffler, maximum volume)
	Catback,        ///< Cat-back system (balanced performance sound)
	Headers,        ///< Headers + exhaust (aggressive, metallic tone)
	Custom          ///< Custom/modified (configurable)
};

/**
 * @brief Current engine operating state
 *
 * Represents what the engine is currently doing. Different states trigger
 * different audio behaviors (e.g., Backfire plays pop sounds, Shifting
 * briefly cuts audio).
 */
UENUM(BlueprintType)
enum class EMGEngineState : uint8
{
	Off,            ///< Engine is not running
	Starting,       ///< Engine is cranking/starting up
	Idle,           ///< Engine running at idle RPM, no throttle
	Revving,        ///< Rev-matching or blipping throttle in neutral
	OnThrottle,     ///< Accelerating (throttle applied)
	OffThrottle,    ///< Decelerating/coasting (engine braking)
	Redline,        ///< At or near redline RPM
	Backfire,       ///< Backfire/pop event occurring
	Shifting,       ///< Mid-gear-change (brief audio gap)
	Stalling        ///< Engine stalling out
};

// ============================================================================
// DATA STRUCTURES - SOUND LAYERS
// ============================================================================

/**
 * @brief Single layer of engine sound for a specific RPM range
 *
 * Engine audio is built from multiple overlapping layers, each covering a
 * portion of the RPM range. Layers crossfade based on current RPM to create
 * smooth transitions.
 *
 * @section layer_example Example Configuration
 * A typical V8 might have:
 * - Layer 1: 0-2000 RPM (idle rumble)
 * - Layer 2: 1500-4000 RPM (low-mid growl)
 * - Layer 3: 3500-6000 RPM (mid-high roar)
 * - Layer 4: 5500-8000 RPM (high-end scream)
 */
USTRUCT(BlueprintType)
struct FMGEngineSoundLayer
{
	GENERATED_BODY()

	/// Sound asset for this layer (should be a looping engine sample)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	/// RPM at which this layer starts fading in
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinRPM = 0.0f;

	/// RPM at which this layer starts fading out
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRPM = 8000.0f;

	/// Base volume multiplier for this layer
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeMultiplier = 1.0f;

	/// Base pitch multiplier (applied before RPM-based pitch)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMultiplier = 1.0f;

	/// Minimum pitch value (at MinRPM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinPitch = 0.5f;

	/// Maximum pitch value (at MaxRPM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPitch = 2.0f;

	/// RPM range over which layers crossfade (overlap region)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrossfadeWidth = 500.0f;

	/// Whether this layer loops continuously
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = true;
};

// ============================================================================
// DATA STRUCTURES - ENGINE PROFILES
// ============================================================================

/**
 * @brief Complete audio profile for a specific engine configuration
 *
 * Defines all the sounds and parameters needed to simulate a particular
 * engine type. Profiles can be shared between vehicles with similar engines.
 */
USTRUCT(BlueprintType)
struct FMGEngineAudioProfile
{
	GENERATED_BODY()

	/// Unique identifier for this profile (e.g., "Profile_V8_American")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ProfileID;

	/// Base engine configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEngineType EngineType = EMGEngineType::V8;

	/// Current exhaust configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGExhaustType ExhaustType = EMGExhaustType::Stock;

	// ===== RPM LIMITS =====

	/// Engine idle RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdleRPM = 800.0f;

	/// Redline RPM (where limiter light comes on)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RedlineRPM = 7000.0f;

	/// Rev limiter RPM (hard limit, triggers limiter sound)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RevLimiterRPM = 7200.0f;

	// ===== SOUND LAYERS =====

	/// Layers played when throttle is applied (accelerating)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEngineSoundLayer> OnThrottleLayers;

	/// Layers played when coasting (engine braking sound)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEngineSoundLayer> OffThrottleLayers;

	// ===== ONE-SHOT SOUNDS =====

	/// Sound played when starting the engine
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> StartupSound;

	/// Sound played when turning off the engine
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ShutdownSound;

	/// Backfire/exhaust pop sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> BackfireSound;

	/// Sound when hitting rev limiter
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> RevLimiterSound;

	// ===== FORCED INDUCTION SOUNDS =====

	/// Turbo spooling up sound (plays while boosting)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> TurboSpoolSound;

	/// Turbo blowoff valve sound (when lifting throttle under boost)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> TurboBlowoffSound;

	/// Supercharger whine sound (constant under load)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SuperchargerWhineSound;

	/// Gear shift sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> GearShiftSound;

	/// Turbo blowoff valve sound (alias for TurboBlowoffSound for code compatibility)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> BlowoffSound;

	// ===== BEHAVIOR PARAMETERS =====

	/// Time in seconds for turbo to spool up
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurboLag = 0.3f;

	/// Probability of backfire on throttle lift (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BackfireChance = 0.3f;

	/// Intensity of exhaust pops and crackles (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExhaustPop = 0.5f;

	/// Low-frequency rumble intensity (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rumble = 0.5f;

	/// Bass punch on throttle application (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BassPunch = 0.5f;
};

// ============================================================================
// DATA STRUCTURES - RUNTIME STATE
// ============================================================================

/**
 * @brief Current state of a vehicle's engine (updated each frame)
 *
 * This struct is passed to the audio system each frame to update the
 * engine sound. It contains all the real-time data needed to calculate
 * the correct sound output.
 */
USTRUCT(BlueprintType)
struct FMGEngineAudioState
{
	GENERATED_BODY()

	/// Current engine RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentRPM = 0.0f;

	/// Target RPM (for smooth interpolation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetRPM = 0.0f;

	/// Current throttle input (0.0 = off, 1.0 = full throttle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleInput = 0.0f;

	/// Engine load percentage (affects sound intensity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Load = 0.0f;

	/// Current gear (0 = neutral, 1-8 = gears)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentGear = 0;

	/// Vehicle speed (affects wind noise, Doppler)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	/// Current turbo boost pressure (for turbo sounds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurboBoost = 0.0f;

	/// True if currently in a gear change
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsShifting = false;

	/// True if bouncing off rev limiter
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRevLimited = false;

	/// True if a backfire is occurring
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBackfiring = false;

	/// Current engine state (for state machine logic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEngineState State = EMGEngineState::Off;
};

/**
 * @brief Transmission audio configuration
 *
 * Settings for gear change sounds and behavior.
 */
USTRUCT(BlueprintType)
struct FMGTransmissionAudioSettings
{
	GENERATED_BODY()

	/// Sound played on successful gear change
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> GearChangeSound;

	/// Sound played on missed/ground gear (manual transmission)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> GearGrindSound;

	/// Time in seconds for gear change (audio gap duration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShiftTime = 0.15f;

	/// How much RPM drops during shift
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RPMDropOnShift = 2000.0f;

	/// True if sequential gearbox (different shift sound)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSequential = false;
};

/**
 * @brief Complete audio instance for a single vehicle
 *
 * Combines the static profile with runtime state for a specific vehicle.
 * The subsystem maintains one of these for each registered vehicle.
 */
USTRUCT(BlueprintType)
struct FMGVehicleAudioInstance
{
	GENERATED_BODY()

	/// Unique identifier for this vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Audio profile for this vehicle's engine
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEngineAudioProfile Profile;

	/// Current runtime state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEngineAudioState State;

	/// True if this is the player's vehicle (gets audio priority)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlayerVehicle = false;

	/// Current world position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Distance from the audio listener
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToListener = 0.0f;

	/// Calculated audibility (0.0 to 1.0) based on distance and priority
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Audibility = 1.0f;
};

// ============================================================================
// DELEGATES
// ============================================================================

/** @brief Broadcast when a vehicle's engine state changes (e.g., Off to Idle) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEngineStateChanged, FName, VehicleID, EMGEngineState, NewState);

/** @brief Broadcast when a vehicle changes gear */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGearChanged, FName, VehicleID, int32, NewGear);

/** @brief Broadcast when a vehicle backfires */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBackfire, FName, VehicleID);

/** @brief Broadcast when a vehicle hits the rev limiter */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRevLimiterHit, FName, VehicleID);

/** @brief Broadcast when turbo blowoff occurs */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurboBlowoff, FName, VehicleID);

// ============================================================================
// ENGINE AUDIO SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Vehicle engine audio management subsystem
 *
 * Manages engine sounds for all vehicles in the world. Handles multi-layer
 * engine audio, forced induction sounds, and dynamic events like backfires.
 *
 * The subsystem uses a priority system to limit CPU usage:
 * - Player vehicle always gets full audio processing
 * - Nearby vehicles get simplified audio
 * - Distant vehicles may be culled entirely
 */
UCLASS()
class MIDNIGHTGRIND_API UMGEngineAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Initialize the engine audio subsystem
	 * Sets up default profiles and starts the audio tick timer.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * @brief Clean up when subsystem is destroyed
	 * Stops all engine sounds and releases resources.
	 */
	virtual void Deinitialize() override;

	/**
	 * @brief Determine if this subsystem should be created for the given world
	 * Only creates for game worlds (not editor preview, etc.).
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// VEHICLE REGISTRATION
	// ==========================================
	/** @name Vehicle Registration
	 *  Functions for adding and removing vehicles from the audio system.
	 */
	///@{

	/**
	 * @brief Register a vehicle with the audio system
	 * @param VehicleID Unique identifier for this vehicle
	 * @param Profile The engine audio profile to use
	 * @param bIsPlayer True if this is the player's vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Vehicle")
	void RegisterVehicle(FName VehicleID, const FMGEngineAudioProfile& Profile, bool bIsPlayer = false);

	/**
	 * @brief Remove a vehicle from the audio system
	 * @param VehicleID The vehicle to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Vehicle")
	void UnregisterVehicle(FName VehicleID);

	/**
	 * @brief Set which vehicle is the player's (for audio priority)
	 * @param VehicleID The player's vehicle ID
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Vehicle")
	void SetPlayerVehicle(FName VehicleID);

	/** @brief Get the current player vehicle ID */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Vehicle")
	FName GetPlayerVehicleID() const { return PlayerVehicleID; }

	///@}

	// ==========================================
	// ENGINE STATE UPDATES
	// ==========================================
	/** @name Engine State Updates
	 *  Functions for updating engine state each frame.
	 *  Call these from your vehicle class to keep audio in sync.
	 */
	///@{

	/**
	 * @brief Update the complete engine state for a vehicle
	 * @param VehicleID The vehicle to update
	 * @param State The new engine state
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void UpdateEngineState(FName VehicleID, const FMGEngineAudioState& State);

	/**
	 * @brief Set just the RPM (convenience function)
	 * @param VehicleID The vehicle to update
	 * @param RPM New RPM value
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetRPM(FName VehicleID, float RPM);

	/**
	 * @brief Set throttle input
	 * @param VehicleID The vehicle to update
	 * @param ThrottleInput Throttle from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetThrottle(FName VehicleID, float ThrottleInput);

	/**
	 * @brief Set current gear
	 * @param VehicleID The vehicle to update
	 * @param Gear Current gear number
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetGear(FName VehicleID, int32 Gear);

	/**
	 * @brief Set turbo boost pressure
	 * @param VehicleID The vehicle to update
	 * @param Boost Boost pressure value
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetTurboBoost(FName VehicleID, float Boost);

	/**
	 * @brief Update vehicle world location (for 3D audio)
	 * @param VehicleID The vehicle to update
	 * @param Location World position
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetVehicleLocation(FName VehicleID, FVector Location);

	///@}

	// ==========================================
	// ENGINE ACTIONS
	// ==========================================
	/** @name Engine Actions
	 *  Trigger specific engine audio events.
	 */
	///@{

	/**
	 * @brief Start the engine (plays startup sound)
	 * @param VehicleID The vehicle to start
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void StartEngine(FName VehicleID);

	/**
	 * @brief Stop the engine (plays shutdown sound)
	 * @param VehicleID The vehicle to stop
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void StopEngine(FName VehicleID);

	/**
	 * @brief Trigger a rev (throttle blip in neutral)
	 * @param VehicleID The vehicle
	 * @param Intensity How aggressive the rev is (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void Rev(FName VehicleID, float Intensity = 1.0f);

	/**
	 * @brief Manually trigger a backfire sound
	 * @param VehicleID The vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void TriggerBackfire(FName VehicleID);

	/**
	 * @brief Trigger turbo blowoff sound
	 * @param VehicleID The vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void TriggerTurboBlowoff(FName VehicleID);

	/**
	 * @brief Trigger gear shift audio
	 * @param VehicleID The vehicle
	 * @param FromGear Previous gear
	 * @param ToGear New gear
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void TriggerGearShift(FName VehicleID, int32 FromGear, int32 ToGear);

	///@}

	// ==========================================
	// QUERIES
	// ==========================================
	/** @name Queries
	 *  Functions for querying engine audio state.
	 */
	///@{

	/** @brief Get the current engine state for a vehicle */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	FMGEngineAudioState GetEngineState(FName VehicleID) const;

	/** @brief Get the engine profile for a vehicle */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	FMGEngineAudioProfile GetEngineProfile(FName VehicleID) const;

	/** @brief Get list of all registered vehicle IDs */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	TArray<FName> GetActiveVehicles() const;

	/** @brief Get current RPM for a vehicle */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	float GetCurrentRPM(FName VehicleID) const;

	/** @brief Check if a vehicle's engine is currently running */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	bool IsEngineRunning(FName VehicleID) const;

	///@}

	// ==========================================
	// PROFILE MANAGEMENT
	// ==========================================
	/** @name Profile Management
	 *  Functions for managing engine audio profiles.
	 */
	///@{

	/**
	 * @brief Register a reusable engine audio profile
	 * @param Profile The profile to register
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Profile")
	void RegisterProfile(const FMGEngineAudioProfile& Profile);

	/**
	 * @brief Get a registered profile by ID
	 * @param ProfileID The profile to retrieve
	 */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Profile")
	FMGEngineAudioProfile GetProfile(FName ProfileID) const;

	/**
	 * @brief Apply an exhaust upgrade to a vehicle (changes sound character)
	 * @param VehicleID The vehicle to modify
	 * @param ExhaustType The new exhaust type
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Profile")
	void ApplyExhaustUpgrade(FName VehicleID, EMGExhaustType ExhaustType);

	/**
	 * @brief Add or modify turbo on a vehicle
	 * @param VehicleID The vehicle to modify
	 * @param bTurbo True to enable turbo, false to disable
	 * @param BoostPressure Maximum boost pressure
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Profile")
	void ApplyTurboUpgrade(FName VehicleID, bool bTurbo, float BoostPressure = 1.0f);

	///@}

	// ==========================================
	// LISTENER
	// ==========================================
	/** @name Listener
	 *  Functions for audio listener (camera) position.
	 */
	///@{

	/**
	 * @brief Set the audio listener location (usually camera position)
	 * @param Location World position of the listener
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Listener")
	void SetListenerLocation(FVector Location);

	/**
	 * @brief Set interior/exterior mode (affects sound filtering)
	 * @param bInterior True when camera is inside the vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Listener")
	void SetInteriorMode(bool bInterior);

	/** @brief Check if currently in interior mode */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Listener")
	bool IsInteriorMode() const { return bInteriorMode; }

	///@}

	// ==========================================
	// SETTINGS
	// ==========================================
	/** @name Settings
	 *  Global engine audio settings.
	 */
	///@{

	/**
	 * @brief Set maximum number of vehicles with active audio
	 * @param MaxVehicles Number of vehicles (helps with performance)
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Settings")
	void SetMaxAudibleVehicles(int32 MaxVehicles);

	/**
	 * @brief Set the master engine audio volume
	 * @param Volume Volume from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Settings")
	void SetEngineVolume(float Volume);

	/** @brief Get current engine volume */
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Settings")
	float GetEngineVolume() const { return EngineVolume; }

	///@}

	// ==========================================
	// EVENT DELEGATES
	// ==========================================
	/** @name Events
	 *  Delegates for reacting to engine audio events in other systems.
	 */
	///@{

	/// Fires when a vehicle's engine state changes
	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnEngineStateChanged OnEngineStateChanged;

	/// Fires when a vehicle changes gear
	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnGearChanged OnGearChanged;

	/// Fires when a vehicle backfires
	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnBackfire OnBackfire;

	/// Fires when a vehicle hits the rev limiter
	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnRevLimiterHit OnRevLimiterHit;

	/// Fires when turbo blowoff occurs
	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnTurboBlowoff OnTurboBlowoff;

	///@}

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** @brief Called each frame to update all vehicle audio */
	void OnEngineTick();

	/** @brief Update audio for a single vehicle */
	void UpdateVehicleAudio(FMGVehicleAudioInstance& Instance);

	/** @brief Calculate audibility for all vehicles based on distance */
	void CalculateAudibility();

	/** @brief Smoothly interpolate RPM to avoid audio pops */
	void ProcessRPMInterpolation(FMGVehicleAudioInstance& Instance, float MGDeltaTime);

	/** @brief Check if backfire should occur and trigger it */
	void ProcessBackfireChance(FMGVehicleAudioInstance& Instance);

	/** @brief Play a specific engine sound layer */
	void PlayEngineLayer(FMGVehicleAudioInstance& Instance, const FMGEngineSoundLayer& Layer, float Volume);

	/** @brief Calculate pitch from RPM for a layer */
	float CalculatePitchFromRPM(float RPM, const FMGEngineSoundLayer& Layer) const;

	/** @brief Calculate volume from RPM for a layer (crossfade) */
	float CalculateLayerVolume(float RPM, const FMGEngineSoundLayer& Layer) const;

	/** @brief Create default engine profiles for common engine types */
	void InitializeDefaultProfiles();

	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/// All currently active vehicles and their audio state
	UPROPERTY()
	TMap<FName, FMGVehicleAudioInstance> ActiveVehicles;

	/// Registered reusable engine profiles
	UPROPERTY()
	TMap<FName, FMGEngineAudioProfile> RegisteredProfiles;

	/// ID of the current player vehicle
	UPROPERTY()
	FName PlayerVehicleID;

	/// Current listener (camera) position
	UPROPERTY()
	FVector ListenerLocation = FVector::ZeroVector;

	/// True if camera is inside the vehicle
	UPROPERTY()
	bool bInteriorMode = false;

	/// Maximum vehicles with active audio (performance limit)
	UPROPERTY()
	int32 MaxAudibleVehicles = 8;

	/// Master volume for all engine sounds
	UPROPERTY()
	float EngineVolume = 1.0f;

	/// Maximum distance at which vehicle audio is heard
	UPROPERTY()
	float MaxAudibleDistance = 5000.0f;

	/// Speed of RPM interpolation (higher = more responsive)
	UPROPERTY()
	float RPMInterpolationSpeed = 5.0f;

	/// Timer handle for the audio tick
	FTimerHandle EngineTickHandle;
};

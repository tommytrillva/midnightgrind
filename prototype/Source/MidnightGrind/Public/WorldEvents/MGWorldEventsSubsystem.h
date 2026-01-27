// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGWorldEventsSubsystem.h
 * @brief Dynamic World Events and Encounters Subsystem
 *
 * This subsystem manages all dynamic world encounters and emergent gameplay
 * events that occur during free roam in Midnight Grind.
 *
 * ## Overview
 * The World Events Subsystem creates a living, reactive open world by:
 * - Spawning random encounters near the player
 * - Managing impromptu street races and challenges
 * - Controlling police chase mechanics
 * - Organizing street meets and car gatherings
 * - Triggering special time-based events
 *
 * ## Architecture
 * This is a Game Instance Subsystem, persisting across all game sessions.
 * It coordinates with:
 * - MGTimeOfDaySubsystem (time-based event availability)
 * - MGAmbientLifeSubsystem (NPC vehicles for events)
 * - Player progression system (rewards and reputation)
 *
 * ## Key Concepts
 * - **World Event**: A discrete encounter with location, duration, and rewards
 * - **Street Meet**: A gathering of NPCs and vehicles at a location
 * - **Police Encounter**: An active chase with escalating heat levels
 * - **Event State**: Lifecycle stages from Pending to Completed/Failed
 *
 * ## Event Flow
 * 1. Events spawn based on probability and player location
 * 2. Player enters event proximity radius
 * 3. Event becomes "Active" and shows on HUD
 * 4. Player can "Join" to engage with the event
 * 5. Event completes with rewards or expires/fails
 *
 * ## Usage Example
 * @code
 * // Get the world events subsystem
 * UMGWorldEventsSubsystem* Events = GetGameInstance()->GetSubsystem<UMGWorldEventsSubsystem>();
 *
 * // Check for nearby events
 * TArray<FMGWorldEvent> NearbyEvents = Events->GetNearbyEvents(PlayerLocation, 500.0f);
 *
 * // Join a street race event
 * if (Events->JoinEvent(NearbyEvents[0].EventID))
 * {
 *     // Event joined successfully, start race logic
 * }
 * @endcode
 *
 * @see EMGWorldEventType for all event categories
 * @see FMGWorldEvent for event data structure
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGWorldEventsSubsystem.generated.h"

// ============================================================================
// WORLD EVENT TYPE ENUMERATION
// ============================================================================

/**
 * @brief Categories of dynamic world events
 *
 * Each event type has distinct gameplay mechanics, rewards, and risk levels.
 * Events spawn based on time of day, player reputation, and random chance.
 */
UENUM(BlueprintType)
enum class EMGWorldEventType : uint8
{
	StreetRace,          ///< An NPC racer challenges you to an impromptu race
	StreetMeet,          ///< A gathering of car enthusiasts with potential races
	PoliceChase,         ///< Police have spotted you and initiated pursuit
	RivalAppearance,     ///< Your story rival appears for a confrontation
	HiddenRace,          ///< A secret underground race is starting nearby
	TimeAttack,          ///< Beat-the-clock checkpoint challenge
	DrivebyChallenge,    ///< Score-based challenge (drift, near-miss, speed)
	SpecialVehicle,      ///< A rare or exotic car has been spotted
	MechanicShop,        ///< Pop-up tuning shop with special parts
	Underground          ///< Secret underground meet with exclusive access
};

// ============================================================================
// WORLD EVENT STATE ENUMERATION
// ============================================================================

/**
 * @brief Lifecycle states for world events
 *
 * Events progress through these states from spawn to completion.
 * State changes trigger delegate broadcasts for UI updates.
 */
UENUM(BlueprintType)
enum class EMGWorldEventState : uint8
{
	Pending,        ///< Event spawned but player not yet nearby
	Active,         ///< Player is within range, event available to join
	PlayerEngaged,  ///< Player has joined and is participating
	Completed,      ///< Event finished successfully, rewards granted
	Failed,         ///< Player failed the event objective
	Expired         ///< Time ran out before player engaged
};

// ============================================================================
// WORLD EVENT STRUCTURE
// ============================================================================

/**
 * @brief Complete data for a single world event instance
 *
 * Contains all information needed to manage, display, and resolve
 * a dynamic world event.
 */
USTRUCT(BlueprintType)
struct FMGWorldEvent
{
	GENERATED_BODY()

	/// Unique identifier for this event instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventID;

	/// Category of event determining gameplay mechanics
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWorldEventType Type = EMGWorldEventType::StreetRace;

	/// Current lifecycle state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWorldEventState State = EMGWorldEventState::Pending;

	/// World location of the event center point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Localized name shown on HUD/map
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Localized description of the event
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Radius for detecting player proximity (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RadiusMeters = 100.0f;

	/// Total event duration before expiration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 300.0f;

	/// Countdown until event expires (seconds remaining)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 300.0f;

	/// Cash reward for successful completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CashReward = 0;

	/// Reputation points for successful completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ReputationReward = 0;

	/// IDs of NPCs involved in this event
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> InvolvedNPCs;

	/// Associated track ID for race events
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RelatedTrackID;

	/// If true, player must be within radius to participate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresProximity = true;
};

// ============================================================================
// STREET MEET STRUCTURE
// ============================================================================

/**
 * @brief Data for a street meet gathering event
 *
 * Street meets are social events where NPC car enthusiasts gather.
 * They can spawn impromptu races and offer networking opportunities.
 */
USTRUCT(BlueprintType)
struct FMGStreetMeet
{
	GENERATED_BODY()

	/// Unique identifier for this meet
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MeetID;

	/// World location of the gathering
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Current number of NPC attendees
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentAttendees = 0;

	/// Maximum capacity for this meet
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAttendees = 20;

	/// Vehicle categories featured at this meet (e.g., "JDM", "Muscle", "Exotic")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> FeaturedVehicleTypes;

	/// True if races will start soon from this location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRacesStartingSoon = false;

	/// Countdown until the meet disperses (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilDispersal = 1800.0f; // 30 minutes
};

// ============================================================================
// POLICE ENCOUNTER STRUCTURE
// ============================================================================

/**
 * @brief Data for an active police chase
 *
 * Tracks all aspects of an ongoing police pursuit including
 * heat level, pursuing units, and escape progress.
 */
USTRUCT(BlueprintType)
struct FMGPoliceEncounter
{
	GENERATED_BODY()

	/// Unique identifier for this encounter
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EncounterID;

	/// Current heat/wanted level (1-5, higher = more aggressive response)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeatLevel = 1;

	/// Number of police vehicles actively pursuing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PursuitUnits = 1;

	/// Progress toward escaping (0-1, reaches 1 = escaped)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeProgress = 0.0f;

	/// True if police helicopter is tracking player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHelicopterDeployed = false;

	/// True if police have set up roadblocks ahead
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRoadblocksActive = false;

	/// Total time spent in this pursuit (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeInPursuit = 0.0f;
};

// ============================================================================
// EVENT SPAWN SETTINGS STRUCTURE
// ============================================================================

/**
 * @brief Configuration for world event spawning behavior
 *
 * Controls spawn rates, distances, and probability weights
 * for different event types.
 */
USTRUCT(BlueprintType)
struct FMGWorldEventSpawnSettings
{
	GENERATED_BODY()

	/// Minimum distance from player for event spawns (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpawnDistance = 200.0f;

	/// Maximum distance from player for event spawns (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpawnDistance = 1000.0f;

	/// Cooldown between event spawn attempts (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EventSpawnCooldown = 60.0f;

	/// Maximum number of active events at once
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxConcurrentEvents = 3;

	/// Base probability for police events (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PoliceSpawnChance = 0.1f;

	/// Base probability for rival appearance (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RivalSpawnChance = 0.15f;

	/// Base probability for street race events (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreetRaceChance = 0.3f;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/// Broadcast when a new world event spawns
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnWorldEventSpawned, const FMGWorldEvent&, Event);

/// Broadcast when an event's state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnWorldEventStateChanged, const FMGWorldEvent&, Event, EMGWorldEventState, NewState);

/// Broadcast when a street meet is discovered
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnStreetMeetFound, const FMGStreetMeet&, Meet);

/// Broadcast when a police chase begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPoliceEncounterStarted, const FMGPoliceEncounter&, Encounter);

/// Broadcast when player successfully escapes police
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPoliceEscaped);

// ============================================================================
// WORLD EVENTS SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem managing dynamic world encounters
 *
 * This Game Instance Subsystem creates and manages all emergent
 * gameplay events during free roam gameplay.
 *
 * ## Features
 * - Automatic event spawning based on player activity
 * - Multiple event types with unique mechanics
 * - Street meet social gatherings
 * - Full police chase system with escalation
 * - Event lifecycle management
 * - Reward distribution on completion
 *
 * ## Spawning Logic
 * Events spawn based on:
 * - Time elapsed since last event
 * - Player location and movement
 * - Time of day (some events are time-restricted)
 * - Player reputation (affects event difficulty/rewards)
 * - Current number of active events
 *
 * ## Performance Notes
 * - Events are updated on a timer, not every frame
 * - Expired events are cleaned up automatically
 * - Distance checks use squared distances for efficiency
 */
UCLASS()
class MIDNIGHTGRIND_API UMGWorldEventsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENT QUERIES
	// ==========================================
	/** @name Event Queries
	 *  Functions for finding and inspecting world events.
	 */
	///@{

	/**
	 * @brief Returns all currently active events
	 * @return Array of all events not yet completed/expired
	 */
	UFUNCTION(BlueprintPure, Category = "WorldEvents")
	TArray<FMGWorldEvent> GetActiveEvents() const;

	/**
	 * @brief Finds events within a radius of a location
	 * @param Location Center point for search
	 * @param Radius Search radius in meters
	 * @return Array of events within the specified area
	 */
	UFUNCTION(BlueprintPure, Category = "WorldEvents")
	TArray<FMGWorldEvent> GetNearbyEvents(FVector Location, float Radius) const;

	/**
	 * @brief Retrieves a specific event by ID
	 * @param EventID The unique event identifier
	 * @return The event data (check EventID for validity)
	 */
	UFUNCTION(BlueprintPure, Category = "WorldEvents")
	FMGWorldEvent GetEvent(const FString& EventID) const;

	/**
	 * @brief Checks if any event of a type is currently active
	 * @param Type The event type to check for
	 * @return True if at least one event of this type is active
	 */
	UFUNCTION(BlueprintPure, Category = "WorldEvents")
	bool HasActiveEventOfType(EMGWorldEventType Type) const;

	///@}

	// ==========================================
	// EVENT INTERACTION
	// ==========================================
	/** @name Event Interaction
	 *  Functions for player interaction with events.
	 */
	///@{

	/**
	 * @brief Attempts to join/engage with an event
	 * @param EventID The event to join
	 * @return True if successfully joined, false if unable
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents")
	bool JoinEvent(const FString& EventID);

	/**
	 * @brief Leaves an event the player is currently engaged with
	 * @param EventID The event to leave
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents")
	void LeaveEvent(const FString& EventID);

	/**
	 * @brief Marks an event as completed
	 * @param EventID The event that finished
	 * @param bSuccess True if player succeeded, false if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents")
	void CompleteEvent(const FString& EventID, bool bSuccess);

	///@}

	// ==========================================
	// EVENT SPAWNING
	// ==========================================
	/** @name Event Spawning
	 *  Functions for manually spawning events (debug/scripted).
	 */
	///@{

	/**
	 * @brief Spawns a specific event type at a location
	 * @param Type The type of event to create
	 * @param Location World position for the event
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Spawn")
	void SpawnEvent(EMGWorldEventType Type, FVector Location);

	/**
	 * @brief Forces an event to spawn near the player
	 * @param Type The type of event to create
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Spawn")
	void ForceSpawnNearPlayer(EMGWorldEventType Type);

	/**
	 * @brief Updates spawn behavior settings
	 * @param Settings New spawn configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Spawn")
	void SetSpawnSettings(const FMGWorldEventSpawnSettings& Settings);

	/// Returns current spawn settings
	UFUNCTION(BlueprintPure, Category = "WorldEvents|Spawn")
	FMGWorldEventSpawnSettings GetSpawnSettings() const { return SpawnSettings; }

	///@}

	// ==========================================
	// STREET MEETS
	// ==========================================
	/** @name Street Meets
	 *  Functions for managing car meet gatherings.
	 */
	///@{

	/// Returns all active street meets
	UFUNCTION(BlueprintPure, Category = "WorldEvents|Meets")
	TArray<FMGStreetMeet> GetActiveStreetMeets() const { return ActiveStreetMeets; }

	/**
	 * @brief Player joins a street meet gathering
	 * @param MeetID The meet to join
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Meets")
	void JoinStreetMeet(const FString& MeetID);

	/**
	 * @brief Player leaves a street meet
	 * @param MeetID The meet to leave
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Meets")
	void LeaveStreetMeet(const FString& MeetID);

	///@}

	// ==========================================
	// POLICE SYSTEM
	// ==========================================
	/** @name Police System
	 *  Functions for managing police encounters and chases.
	 */
	///@{

	/// Returns true if player is currently being chased by police
	UFUNCTION(BlueprintPure, Category = "WorldEvents|Police")
	bool IsInPoliceChase() const { return CurrentPoliceEncounter.PursuitUnits > 0; }

	/// Returns current police encounter data
	UFUNCTION(BlueprintPure, Category = "WorldEvents|Police")
	FMGPoliceEncounter GetPoliceEncounter() const { return CurrentPoliceEncounter; }

	/**
	 * @brief Initiates a police chase
	 * @param InitialHeat Starting heat level (1-5)
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Police")
	void TriggerPoliceChase(int32 InitialHeat = 1);

	/**
	 * @brief Increases the current heat level
	 * @param Amount Heat points to add (clamped to max of 5)
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Police")
	void IncreaseHeat(int32 Amount);

	/**
	 * @brief Updates escape progress during a chase
	 * @param Progress Current escape progress (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Police")
	void UpdateEscapeProgress(float Progress);

	///@}

	// ==========================================
	// EVENTS
	// ==========================================
	/** @name Event Delegates
	 *  Subscribe to respond to world event changes.
	 */
	///@{

	/// Fires when a new event spawns
	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnWorldEventSpawned OnWorldEventSpawned;

	/// Fires when an event's state changes
	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnWorldEventStateChanged OnWorldEventStateChanged;

	/// Fires when a street meet is discovered
	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnStreetMeetFound OnStreetMeetFound;

	/// Fires when a police chase starts
	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnPoliceEncounterStarted OnPoliceEncounterStarted;

	/// Fires when player escapes police
	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnPoliceEscaped OnPoliceEscaped;

	///@}

protected:
	// ==========================================
	// INTERNAL UPDATE METHODS
	// ==========================================
	/** @name Internal Methods
	 *  Private implementation for event management.
	 */
	///@{

	/**
	 * @brief Main update loop for all active events
	 * @param DeltaTime Time since last update
	 */
	void UpdateEvents(float DeltaTime);

	/**
	 * @brief Updates police chase state and escalation
	 * @param DeltaTime Time since last update
	 */
	void UpdatePoliceChase(float DeltaTime);

	/**
	 * @brief Attempts to spawn a random event near player
	 */
	void TrySpawnRandomEvent();

	/**
	 * @brief Removes expired/completed events from active list
	 */
	void CleanupExpiredEvents();

	/**
	 * @brief Creates event data for a new event
	 * @param Type Event type to create
	 * @param Location Spawn location
	 * @return Configured event instance
	 */
	FMGWorldEvent GenerateRandomEvent(EMGWorldEventType Type, FVector Location);

	///@}

private:
	// ==========================================
	// STATE VARIABLES
	// ==========================================

	/// All currently tracked events
	UPROPERTY()
	TArray<FMGWorldEvent> ActiveEvents;

	/// All active street meet gatherings
	UPROPERTY()
	TArray<FMGStreetMeet> ActiveStreetMeets;

	/// Current police chase state (check PursuitUnits > 0 for active)
	FMGPoliceEncounter CurrentPoliceEncounter;

	/// Configuration for event spawning
	FMGWorldEventSpawnSettings SpawnSettings;

	/// Timer for periodic event updates
	FTimerHandle EventUpdateHandle;

	/// Cooldown tracker for spawn attempts
	float TimeSinceLastSpawn = 0.0f;

	/// Cached player position for distance calculations
	FVector LastPlayerLocation = FVector::ZeroVector;
};

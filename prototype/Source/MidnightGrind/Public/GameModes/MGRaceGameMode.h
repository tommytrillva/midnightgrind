// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGRaceGameMode.h
 * =============================================================================
 *
 * PURPOSE:
 * This is the core game mode class that manages all race functionality in
 * Midnight Grind. Think of it as the "race director" - it controls everything
 * from the pre-race countdown to calculating final results and rewards.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. GAME MODE: In Unreal Engine, a Game Mode defines the rules of the game.
 *    It controls what happens when players join, how scoring works, and when
 *    the game ends. This class extends AGameModeBase to add racing-specific
 *    functionality.
 *
 * 2. RACE STATE MACHINE: The race progresses through defined states (PreRace ->
 *    Countdown -> Racing -> Finished). This pattern is called a "state machine"
 *    and makes it easy to know exactly what phase the race is in.
 *
 * 3. DELEGATES/EVENTS: The FOn* declarations (like FOnRaceStarted) are
 *    "delegates" - they allow other parts of the code (like UI) to be notified
 *    when something happens without the GameMode needing to know about them.
 *
 * 4. USTRUCT: Data containers (FMGRaceConfig, FMGRacerData) are defined as
 *    USTRUCT to make them usable in Blueprints and automatically handle
 *    memory management.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 *
 *    [MGRaceGameMode]  <-- You are here
 *           |
 *           +-- Uses [MGRaceTypeHandler] to implement race-type-specific logic
 *           +-- Sends results to [MGRaceFlowManager] for rewards/progression
 *           +-- Broadcasts events to UI (HUD, results screen)
 *           +-- Manages [AMGVehiclePawn] vehicles and [AMGCheckpoint] track data
 *
 * USAGE EXAMPLE:
 *
 *    // In Blueprint or C++, configure and start a race:
 *    FMGRaceConfig Config;
 *    Config.RaceType = EMGRaceType::Circuit;
 *    Config.LapCount = 3;
 *    Config.MaxRacers = 8;
 *
 *    AMGRaceGameMode* GameMode = GetWorld()->GetAuthGameMode<AMGRaceGameMode>();
 *    GameMode->SetRaceConfig(Config);
 *    GameMode->StartCountdown();
 *
 *    // Listen for race completion:
 *    GameMode->OnRaceFinished.AddDynamic(this, &UMyClass::HandleRaceFinished);
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MGRaceGameMode.generated.h"

// Forward declarations - these tell the compiler these classes exist without
// including their full headers (improves compile times)
class AMGVehiclePawn;
class AMGCheckpoint;
class AMGRacingHUD;
class UMGRacingHUD;

/**
 * EMGRaceState - Enumeration of all possible race states
 *
 * The race progresses through these states in a predictable order:
 * PreRace -> Countdown -> Racing -> Finished (or Aborted)
 *
 * The Paused state can occur during Racing and returns to Racing when resumed.
 *
 * WHY THIS MATTERS:
 * - UI elements show/hide based on state (countdown overlay, results screen)
 * - Vehicle controls may be locked during certain states (PreRace, Countdown)
 * - Timing only runs during the Racing state
 */
UENUM(BlueprintType)
enum class EMGRaceState : uint8
{
	/** Pre-race lobby/setup - Players choosing vehicles, waiting for others */
	PreRace			UMETA(DisplayName = "Pre-Race"),

	/** Countdown before race start - "3... 2... 1... GO!" */
	Countdown		UMETA(DisplayName = "Countdown"),

	/** Race in progress - Vehicles are racing, timers running */
	Racing			UMETA(DisplayName = "Racing"),

	/** Race paused - Typically from pause menu, freezes race timer */
	Paused			UMETA(DisplayName = "Paused"),

	/** Race finished - All racers done or time expired, showing results */
	Finished		UMETA(DisplayName = "Finished"),

	/** Race aborted/canceled - Race ended without completion (player quit, etc.) */
	Aborted			UMETA(DisplayName = "Aborted")
};

/**
 * EMGRaceType - The different types of races available in Midnight Grind
 *
 * Each race type has fundamentally different rules, win conditions, and scoring.
 * The game mode uses this to delegate to the appropriate MGRaceTypeHandler.
 *
 * IMPORTANT: When adding a new race type, you must also create a corresponding
 * UMGRaceTypeHandler subclass to implement its specific logic.
 */
UENUM(BlueprintType)
enum class EMGRaceType : uint8
{
	/** Standard circuit race - Complete X laps around a closed track.
	    First to finish all laps wins. Most common race type. */
	Circuit			UMETA(DisplayName = "Circuit"),

	/** Sprint race - Point A to B, no laps.
	    First to reach the finish line wins. */
	Sprint			UMETA(DisplayName = "Sprint"),

	/** Drift competition - Score-based, not position-based.
	    Highest drift score at the end wins. */
	Drift			UMETA(DisplayName = "Drift"),

	/** Time trial - Solo race against the clock.
	    Beat target times, compete against ghost replays. */
	TimeTrial		UMETA(DisplayName = "Time Trial"),

	/** Drag race - Quarter-mile straight-line acceleration.
	    Features Christmas tree start and reaction time scoring. */
	Drag			UMETA(DisplayName = "Drag"),

	/** Pink slip - Winner takes loser's car permanently!
	    High-stakes race with vehicle wagering. */
	PinkSlip		UMETA(DisplayName = "Pink Slip")
};

/**
 * FMGRaceConfig - Configuration settings for a race
 *
 * This struct holds all the settings needed to set up a race. It's used:
 * - When starting a race from the menu
 * - When loading race presets from data assets
 * - When saving/loading race configurations
 *
 * TIP: For designers, these values can be tweaked in Blueprint-based
 * race configurations using UMGRaceConfiguration data assets.
 */
USTRUCT(BlueprintType)
struct FMGRaceConfig
{
	GENERATED_BODY()

	/** What type of race this is (Circuit, Sprint, Drift, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	EMGRaceType RaceType = EMGRaceType::Circuit;

	/**
	 * Number of laps for circuit races.
	 * Ignored for other race types like Sprint or Drag.
	 * Range: 1-99 laps
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "1", ClampMax = "99"))
	int32 LapCount = 3;

	/**
	 * Maximum time allowed for the race in seconds.
	 * 0 = no time limit (race continues until all finish or give up)
	 * Useful for: preventing indefinitely long races, adding urgency
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float TimeLimit = 0.0f;

	/**
	 * Maximum number of vehicles in the race (including player).
	 * Range: 1-16 racers
	 * Note: More racers = more CPU load for AI calculations
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "1", ClampMax = "16"))
	int32 MaxRacers = 8;

	/**
	 * AI difficulty level from 0.0 (easiest) to 1.0 (hardest).
	 * This affects AI racing line accuracy, reaction times, and aggression.
	 * 0.0 = Beginner-friendly AI that makes mistakes
	 * 0.5 = Balanced challenge for average players
	 * 1.0 = Expert AI that rarely makes errors
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AIDifficulty = 0.5f;

	/**
	 * Whether vehicles can physically collide with each other.
	 * true = Realistic collisions (can bump, spin out)
	 * false = Vehicles pass through each other (ghost mode)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bAllowCollisions = true;

	/**
	 * Is this a pink slip (title) race where winner takes loser's car?
	 * IMPORTANT: Pink slip races have additional verification requirements
	 * and use the UMGPinkSlipHandler for transfer logic.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bPinkSlipRace = false;

	/**
	 * Internal name of the track/level being raced on.
	 * Used for loading track data, checkpoints, and spawn points.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName TrackName;

	/**
	 * Time of day setting for lighting/atmosphere.
	 * 0.0 = Midnight (dark, neon-lit streets - the signature "Midnight Grind" vibe)
	 * 0.5 = Dawn/Dusk
	 * 1.0 = Noon (bright daylight)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TimeOfDay = 0.0f; // Default: Night (MIDNIGHT GRIND!)

	/**
	 * Weather intensity setting.
	 * 0.0 = Clear weather
	 * 0.5 = Light rain
	 * 1.0 = Heavy rain/storm
	 * Note: Weather affects vehicle handling (reduced grip in rain)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Weather = 0.0f;
};

/**
 * FMGRacerData - Runtime data tracked for each participant during a race
 *
 * Every vehicle in the race (player or AI) has a corresponding FMGRacerData
 * that tracks their progress, timing, and status. This data is used for:
 * - HUD display (lap counter, position, times)
 * - Position calculation (who's ahead)
 * - Results screen after race ends
 * - Replay system
 *
 * NOTE: This is runtime data, not saved between races. For persistent stats,
 * see the player progression system (UMGPlayerProgression).
 */
USTRUCT(BlueprintType)
struct FMGRacerData
{
	GENERATED_BODY()

	/**
	 * Index in the Racers array (0 = first registered, usually the player).
	 * Used to reference this racer in function calls.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 RacerIndex = 0;

	/**
	 * Weak reference to the actual vehicle pawn in the world.
	 * TWeakObjectPtr = safe reference that becomes null if vehicle is destroyed.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	TWeakObjectPtr<AMGVehiclePawn> Vehicle;

	/**
	 * Current lap number (1 = first lap, 2 = second lap, etc.)
	 * Increments when crossing the start/finish line in the correct direction.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 CurrentLap = 0;

	/**
	 * Index of the last checkpoint passed.
	 * Used to: validate lap completion, calculate progress, prevent shortcuts.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 CurrentCheckpoint = 0;

	/**
	 * Current race position (1 = first place, 2 = second, etc.)
	 * Updated regularly during the race based on progress.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	int32 Position = 0;

	/**
	 * Total distance traveled along the racing line in cm.
	 * Used for accurate position calculation when multiple racers
	 * are on the same lap/checkpoint.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float TotalDistance = 0.0f;

	/** Current lap time in seconds (resets each lap) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float CurrentLapTime = 0.0f;

	/** Best (fastest) lap time achieved in this race */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float BestLapTime = 0.0f;

	/** Total elapsed time since race start */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float TotalTime = 0.0f;

	/** Array of completed lap times (index 0 = first lap time, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	TArray<float> LapTimes;

	/** Has this racer crossed the finish line for the final time? */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	bool bFinished = false;

	/** Time when racer finished (relative to race start) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float FinishTime = 0.0f;

	/** Did Not Finish - set if racer quits, disconnects, or times out */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	bool bDNF = false;

	/** Is this an AI-controlled racer? (false = human player) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	bool bIsAI = false;

	/** Drift score - only used in Drift race type */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	float DriftScore = 0.0f;

	/** Display name shown in UI (player name or AI driver name) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer")
	FText DisplayName;
};

/**
 * FMGRaceResults - Complete results after a race ends
 *
 * This struct contains everything needed for the results screen and
 * for calculating rewards. It's created when the race enters the
 * Finished state and passed to the RaceFlowManager for processing.
 */
USTRUCT(BlueprintType)
struct FMGRaceResults
{
	GENERATED_BODY()

	/** Copy of the race configuration that was used */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FMGRaceConfig Config;

	/** All racers' data, sorted by finishing position (index 0 = winner) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<FMGRacerData> RacerResults;

	/** Fastest single lap time achieved by any racer */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float BestLapTime = 0.0f;

	/** Index of racer who achieved best lap (-1 if none) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 BestLapRacerIndex = -1;

	/** Total duration of the race in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float TotalRaceTime = 0.0f;

	/** Credits (in-game currency) earned by the player */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int64 CreditsEarned = 0;

	/** Reputation points earned (affects unlocks and matchmaking) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 ReputationEarned = 0;

	/** Did the player finish in 1st place? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bPlayerWon = false;
};

// =============================================================================
// DELEGATE DECLARATIONS
// =============================================================================
// These are "events" that other classes can subscribe to. When the GameMode
// calls OnRaceStarted.Broadcast(), all subscribed listeners are notified.
//
// HOW TO USE (in Blueprint or C++):
// GameMode->OnRaceStarted.AddDynamic(this, &UMyClass::HandleRaceStarted);
// =============================================================================

/** Fired when race state changes (e.g., PreRace -> Countdown -> Racing) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceStateChanged, EMGRaceState, NewState);

/** Fired each second during countdown ("3... 2... 1...") */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32, SecondsRemaining);

/** Fired when countdown reaches zero and race begins ("GO!") */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);

/** Fired when any racer completes a lap */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLapCompleted, int32, RacerIndex, float, LapTime);

/** Fired when a racer crosses the finish line for the last time */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRacerFinished, int32, RacerIndex, int32, Position);

/** Fired when entire race ends (all finished or time expired) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFinished, const FMGRaceResults&, Results);

/** Fired when any racer's position changes (for UI updates) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPositionChanged, int32, RacerIndex, int32, NewPosition);

/**
 * AMGRaceGameMode - The main race game mode class
 *
 * This class orchestrates the entire race from setup to results.
 *
 * LIFECYCLE:
 * 1. Level loads with this GameMode set as the Game Mode Override
 * 2. InitGame() - Parse map options, initialize state
 * 3. StartPlay() - World is ready, set up checkpoints
 * 4. HandleStartingNewPlayer_Implementation() - Player joins, register vehicle
 * 5. SetRaceConfig() - Configure race settings
 * 6. StartCountdown() - Begin 3-2-1 countdown
 * 7. Tick() - Called every frame to update race state
 * 8. EndRace() - Race complete, calculate results
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGRaceGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMGRaceGameMode();

	// =========================================================================
	// UNREAL ENGINE OVERRIDES
	// =========================================================================
	// These functions are called automatically by the engine at specific times.
	// We override them to add our race-specific initialization and updates.
	// =========================================================================

	/**
	 * Called before StartPlay when the map is loading.
	 * @param MapName - Name of the map being loaded
	 * @param Options - URL options passed to the map (e.g., "?RaceType=Circuit")
	 * @param ErrorMessage - Output parameter for any error messages
	 */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Called when the game begins play. Set up checkpoints and initial state here. */
	virtual void StartPlay() override;

	/**
	 * Called every frame. Updates countdown, race timing, and positions.
	 * @param DeltaTime - Time since last frame in seconds
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Called when a new player connects.
	 * We use this to register their vehicle as a racer.
	 */
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// ==========================================
	// RACE CONFIGURATION
	// ==========================================

	/**
	 * Set the race configuration. Call this before StartCountdown().
	 * @param Config - The race settings to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void SetRaceConfig(const FMGRaceConfig& Config);

	/** Get the current race configuration */
	UFUNCTION(BlueprintPure, Category = "Race")
	FMGRaceConfig GetRaceConfig() const { return RaceConfig; }

	/** Get what state the race is currently in */
	UFUNCTION(BlueprintPure, Category = "Race")
	EMGRaceState GetRaceState() const { return CurrentRaceState; }

	// ==========================================
	// RACE CONTROL
	// ==========================================
	// These functions control the race flow.
	// Typically called by UI (start button) or game logic.
	// ==========================================

	/**
	 * Start the race countdown (3... 2... 1... GO!)
	 * Vehicles will be frozen during countdown.
	 */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void StartCountdown();

	/** Cancel countdown and return to pre-race state */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void AbortCountdown();

	/** Pause the race (freezes race timer, typically from pause menu) */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void PauseRace();

	/** Resume from paused state */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void ResumeRace();

	/** End the race immediately (force finish, or when all racers done) */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void EndRace();

	/** Restart the race (reset all progress, return to pre-race) */
	UFUNCTION(BlueprintCallable, Category = "Race|Control")
	void RestartRace();

	// ==========================================
	// RACER MANAGEMENT
	// ==========================================
	// Functions for adding/removing vehicles from the race
	// and querying their data.
	// ==========================================

	/**
	 * Register a vehicle as a race participant.
	 * @param Vehicle - The vehicle pawn to register
	 * @param bIsAI - True if AI-controlled, false if player
	 * @param DisplayName - Name to show in UI
	 * @return Assigned racer index (use this for future queries)
	 */
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	int32 RegisterRacer(AMGVehiclePawn* Vehicle, bool bIsAI = false, FText DisplayName = FText());

	/** Remove a racer from the race (e.g., if they disconnect) */
	UFUNCTION(BlueprintCallable, Category = "Race|Racers")
	void UnregisterRacer(int32 RacerIndex);

	/** Get all tracked data for a specific racer */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	FMGRacerData GetRacerData(int32 RacerIndex) const;

	/** Get array of all racers in the race */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	TArray<FMGRacerData> GetAllRacers() const { return Racers; }

	/** Get number of registered racers */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	int32 GetRacerCount() const { return Racers.Num(); }

	/** Get the racer index of the human player (usually 0) */
	UFUNCTION(BlueprintPure, Category = "Race|Racers")
	int32 GetPlayerRacerIndex() const { return PlayerRacerIndex; }

	// ==========================================
	// CHECKPOINT/LAP TRACKING
	// ==========================================
	// Checkpoints are invisible triggers placed around the track.
	// They ensure racers follow the correct route and count laps.
	// ==========================================

	/**
	 * Register a checkpoint with the race system.
	 * Called automatically by AMGCheckpoint actors in the level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Race|Checkpoints")
	void RegisterCheckpoint(AMGCheckpoint* Checkpoint, int32 CheckpointIndex);

	/**
	 * Called when a vehicle passes through a checkpoint.
	 * Validates correct order and updates racer progress.
	 */
	UFUNCTION(BlueprintCallable, Category = "Race|Checkpoints")
	void OnCheckpointPassed(AMGVehiclePawn* Vehicle, int32 CheckpointIndex);

	/** Get total number of checkpoints on the track */
	UFUNCTION(BlueprintPure, Category = "Race|Checkpoints")
	int32 GetCheckpointCount() const { return Checkpoints.Num(); }

	// ==========================================
	// TIMING & POSITIONS
	// ==========================================

	/** Get elapsed race time in seconds (only counts during Racing state) */
	UFUNCTION(BlueprintPure, Category = "Race|Timing")
	float GetRaceTime() const { return RaceTime; }

	/** Get countdown seconds remaining (3, 2, 1, or 0) */
	UFUNCTION(BlueprintPure, Category = "Race|Timing")
	int32 GetCountdownSeconds() const { return CountdownSeconds; }

	/** Get final race results (only valid after race ends) */
	UFUNCTION(BlueprintPure, Category = "Race|Results")
	FMGRaceResults GetRaceResults() const { return RaceResults; }

	// ==========================================
	// EVENTS (DELEGATES)
	// ==========================================
	// Subscribe to these to be notified when race events occur.
	// Example: OnRaceStarted.AddDynamic(this, &UMyClass::HandleStart);
	// ==========================================

	/** Broadcast when race state changes */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceStateChanged OnRaceStateChanged;

	/** Broadcast each countdown tick (3, 2, 1) */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnCountdownTick OnCountdownTick;

	/** Broadcast when race starts (GO!) */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceStarted OnRaceStarted;

	/** Broadcast when any racer completes a lap */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnLapCompleted OnLapCompleted;

	/** Broadcast when a racer finishes the race */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRacerFinished OnRacerFinished;

	/** Broadcast when entire race ends */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceFinished OnRaceFinished;

	/** Broadcast when any racer's position changes */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnPositionChanged OnPositionChanged;

protected:
	// ==========================================
	// CONFIGURATION (Designer-Tweakable)
	// ==========================================

	/** Current race configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Configuration")
	FMGRaceConfig RaceConfig;

	/** How many seconds to count down before GO (typically 3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Configuration")
	int32 CountdownDuration = 3;

	/** How often to recalculate positions (in seconds). Lower = more accurate but more CPU */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Configuration")
	float PositionUpdateRate = 0.1f;

	// ==========================================
	// RUNTIME STATE
	// ==========================================

	/** Current state of the race (PreRace, Countdown, Racing, etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	EMGRaceState CurrentRaceState = EMGRaceState::PreRace;

	/** All registered racers and their data */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	TArray<FMGRacerData> Racers;

	/** All checkpoint actors in the level (ordered by CheckpointIndex) */
	UPROPERTY()
	TArray<TWeakObjectPtr<AMGCheckpoint>> Checkpoints;

	/** Elapsed race time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	float RaceTime = 0.0f;

	/** Current countdown value (3, 2, 1, 0) */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	int32 CountdownSeconds = 0;

	/** Final race results (populated when race ends) */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	FMGRaceResults RaceResults;

	/** Racer index of the human player */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	int32 PlayerRacerIndex = 0;

	/** How many racers have finished so far */
	UPROPERTY(BlueprintReadOnly, Category = "Race|State")
	int32 FinishedCount = 0;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================
	// These are private implementation details.
	// They're not exposed to Blueprint as they handle
	// complex internal logic.
	// ==========================================

	/** Transition to a new race state and broadcast event */
	void SetRaceState(EMGRaceState NewState);

	/** Called each tick during countdown to update timer */
	void UpdateCountdown(float DeltaTime);

	/** Called each tick during racing to update lap times */
	void UpdateRaceTiming(float DeltaTime);

	/** Recalculate all racer positions based on progress */
	void UpdatePositions();

	/** Check if race should end (all finished, time up, etc.) */
	void CheckRaceComplete();

	/** Build the final FMGRaceResults struct */
	void CalculateResults();

	/** Calculate credits reward based on position and race settings */
	int64 CalculateCreditsEarned(int32 Position, bool bWon) const;

	/** Find which racer index a vehicle belongs to */
	int32 GetRacerIndexForVehicle(AMGVehiclePawn* Vehicle) const;

	/** Lock/unlock all vehicle controls (used during countdown) */
	void FreezeAllVehicles(bool bFreeze);

	/** Tell all player controllers that race has started */
	void NotifyPlayersRaceStarted();

	/** Tell all player controllers that race has ended */
	void NotifyPlayersRaceEnded();

	/** Send current race data to the HUD subsystem for display */
	void UpdateHUDSubsystem();

	/** Notify RaceFlowManager to process rewards and progression */
	void NotifyRaceFlowManager();

private:
	// ==========================================
	// INTERNAL TIMERS
	// ==========================================

	/** Accumulator for countdown timing (tracks fractional seconds) */
	float CountdownAccumulator = 0.0f;

	/** Accumulator for position update rate limiting */
	float PositionUpdateAccumulator = 0.0f;
};

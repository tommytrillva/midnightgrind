/**
 * @file MGRaceFlowSubsystem.h
 * @brief Race Flow Subsystem - The central orchestrator for the complete race lifecycle
 *
 * This subsystem serves as the primary coordinator between all race-related systems in
 * Midnight Grind. It manages the entire race flow from the moment a player selects a race
 * in the garage through track loading, race execution, results display, and reward
 * distribution.
 *
 * ## Key Responsibilities
 * - Receiving and validating race setup requests from the garage/menu UI
 * - Coordinating track level loading and unloading
 * - Spawning player and AI vehicles at starting positions
 * - Connecting to the RaceGameMode for active race management
 * - Processing race results and calculating rewards
 * - Managing the return flow back to the garage
 *
 * ## Race Flow States
 * The subsystem operates as a state machine progressing through:
 * Idle -> Setup -> Loading -> PreRace -> Countdown -> Racing -> Cooldown -> Results -> ProcessingRewards -> Returning
 *
 * ## Usage Example
 * @code
 * UMGRaceFlowSubsystem* RaceFlow = GetGameInstance()->GetSubsystem<UMGRaceFlowSubsystem>();
 * FMGRaceSetupRequest Request;
 * Request.TrackID = FName("Downtown_Circuit");
 * Request.PlayerVehicleID = FName("Nissan_GTR");
 * Request.LapCount = 3;
 * RaceFlow->StartRace(Request);
 * @endcode
 *
 * @note This is a GameInstanceSubsystem, meaning it persists across level transitions
 * @see UMGRaceDirectorSubsystem for AI pacing and drama control
 * @see UMGRaceModeSubsystem for race type and scoring logic
 *
 * Copyright Midnight Grind. All Rights Reserved.
 * Stage 51: Race Flow Subsystem - MVP Game Flow Orchestration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRaceFlowSubsystem.generated.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class UMGGameStateSubsystem;
class UMGVehicleManagerSubsystem;
class UMGRaceDirectorSubsystem;
class UMGEconomySubsystem;
class UMGGarageSubsystem;

// ============================================================================
// RACE SETUP REQUEST
// ============================================================================

/**
 * @struct FMGRaceSetupRequest
 * @brief Complete configuration for initiating a race from the garage/menu
 *
 * This struct contains all parameters needed to set up and start a race.
 * It is typically populated by the garage UI when the player selects race options.
 *
 * The request includes:
 * - Track and race type selection
 * - Player vehicle choice
 * - AI opponent configuration
 * - Environmental conditions (time, weather)
 * - Pink slip (vehicle wagering) settings
 * - Reward multipliers
 */
USTRUCT(BlueprintType)
struct FMGRaceSetupRequest
{
	GENERATED_BODY()

	// ---- Track Selection ----

	/** Track ID to race on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName TrackID;

	/** Race type (Circuit, Sprint, Drift, Drag, TimeTrial, PinkSlip) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName RaceType = FName("Circuit");

	/** Number of laps (for circuit races) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 LapCount = 3;

	// ---- AI Configuration ----

	/** Number of AI opponents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 AICount = 7;

	/** AI difficulty (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float AIDifficulty = 0.5f;

	// ---- Player Vehicle ----

	/** Player's selected vehicle ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName PlayerVehicleID;

	// ---- Environmental Conditions ----

	/** Time of day (0 = midnight, 0.5 = noon, 1 = midnight) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float TimeOfDay = 0.0f;  // Midnight by default

	/** Weather condition (0 = clear, 1 = heavy rain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float Weather = 0.0f;

	// ---- Pink Slip Settings ----

	/** Is this a pink slip race? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsPinkSlip = false;

	/** Vehicle being wagered (pink slip) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName PinkSlipVehicleID;

	// ---- Multiplayer Settings ----

	/** Is ranked multiplayer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsRanked = false;

	/** Session ID for multiplayer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString SessionID;

	// ---- Reward Configuration ----

	/** Base cash reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int64 BaseCashReward = 5000;

	/** Base reputation reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 BaseRepReward = 100;
};

// ============================================================================
// RACE FLOW RESULT
// ============================================================================

/**
 * @struct FMGRaceFlowResult
 * @brief Complete race results returned to the garage/menu after race completion
 *
 * Contains all outcome data from a completed race including:
 * - Player finishing position and completion status
 * - Timing data (total time, lap times, best lap)
 * - Rewards earned (cash, reputation, XP)
 * - Special achievements (track records, personal bests)
 * - Pink slip outcomes (vehicles won/lost)
 *
 * This data is used by the results screen and progression systems.
 */
USTRUCT(BlueprintType)
struct FMGRaceFlowResult
{
	GENERATED_BODY()

	// ---- Completion Status ----

	/** Did the player finish? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bPlayerFinished = false;

	/** Player's final position (1-based) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 PlayerPosition = 0;

	/** Total racers */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 TotalRacers = 0;

	// ---- Timing Data ----

	/** Player's total time */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float PlayerTotalTime = 0.0f;

	/** Player's best lap */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float PlayerBestLap = 0.0f;

	/** All lap times */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<float> PlayerLapTimes;

	// ---- Win/Loss Status ----

	/** Did player win? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bPlayerWon = false;

	// ---- Rewards ----

	/** Cash earned */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int64 CashEarned = 0;

	/** Reputation earned */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 ReputationEarned = 0;

	/** XP earned */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 XPEarned = 0;

	/** Drift score (drift races) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int64 DriftScore = 0;

	// ---- Pink Slip Outcomes ----

	/** Pink slip won vehicle (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FName PinkSlipWonVehicleID;

	/** Pink slip lost vehicle (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FName PinkSlipLostVehicleID;

	// ---- Records and Achievements ----

	/** Track record beaten? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bNewTrackRecord = false;

	/** Personal best beaten? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bNewPersonalBest = false;

	/** Race was completed (not aborted) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bRaceCompleted = false;

	// ---- Leaderboard Data ----

	/** All racer results for leaderboard display */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<FName> FinishOrder;
};

// ============================================================================
// AI RACER SETUP
// ============================================================================

/**
 * @struct FMGAIRacerSetup
 * @brief Configuration for a single AI opponent in the race
 *
 * Defines the vehicle, personality, and behavior traits for an AI racer.
 * The Race Flow Subsystem generates these based on difficulty settings
 * or accepts manually configured opponents for story/rival races.
 */
USTRUCT(BlueprintType)
struct FMGAIRacerSetup
{
	GENERATED_BODY()

	/// The vehicle definition ID this AI will drive
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Display name shown in race UI (e.g., "Street King", "Shadow")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	/// Skill level from 0.0 (novice) to 1.0 (expert) - affects racing line adherence and speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillLevel = 0.5f;

	/// Aggression from 0.0 (passive) to 1.0 (aggressive) - affects blocking and overtaking behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Aggression = 0.5f;

	/// Is this a story/career rival with special behaviors and narrative significance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRival = false;
};

// ============================================================================
// RACE FLOW STATE ENUMERATION
// ============================================================================

/**
 * @enum EMGRaceFlowState
 * @brief State machine states representing the complete race lifecycle
 *
 * The Race Flow Subsystem progresses through these states sequentially.
 * Each state has associated setup and teardown logic, and transitions
 * are triggered by various game events or timers.
 *
 * ## State Transition Diagram
 * @verbatim
 * Idle -> Setup -> Loading -> PreRace -> Countdown -> Racing
 *                                                        |
 *   Error <-----------------------------------------+    v
 *                                                   Cooldown
 *                                                       |
 *                                                       v
 *                                                    Results
 *                                                       |
 *                                                       v
 *                                              ProcessingRewards
 *                                                       |
 *                                                       v
 *                                                   Returning -> Idle
 * @endverbatim
 */
UENUM(BlueprintType)
enum class EMGRaceFlowState : uint8
{
	/** No race active - waiting for race request from garage */
	Idle,
	/** Setting up race - validating request and preparing data */
	Setup,
	/** Loading track - streaming in the race level */
	Loading,
	/** Pre-race grid/intro - vehicles on grid, camera flyover */
	PreRace,
	/** Countdown active - 3, 2, 1, GO! sequence */
	Countdown,
	/** Race in progress - active racing */
	Racing,
	/** Race finished, cooldown - brief pause after finish */
	Cooldown,
	/** Showing results - results screen displayed to player */
	Results,
	/** Processing rewards - applying cash/rep/XP to player profile */
	ProcessingRewards,
	/** Returning to garage - unloading race level, loading garage */
	Returning,
	/** Error state - something went wrong, requires recovery */
	Error
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/// Broadcast when the race flow state machine transitions to a new state
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFlowStateChanged, EMGRaceFlowState, NewState);

/// Broadcast when race setup phase completes (success or failure)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceSetupComplete, bool, bSuccess);

/// Broadcast periodically during track loading with progress (0.0 to 1.0)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceLoadProgress, float, Progress);

/// Broadcast when the countdown ends and race officially starts
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);

/// Broadcast when the race ends (player finishes or race concludes)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFinished, const FMGRaceFlowResult&, Result);

/// Broadcast when rewards have been calculated and applied to player profile
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardsProcessed, const FMGRaceFlowResult&, Result);

/// Broadcast when an error occurs during any phase of the race flow
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceError, const FString&, ErrorMessage);

// ============================================================================
// RACE FLOW SUBSYSTEM CLASS
// ============================================================================

/**
 * @class UMGRaceFlowSubsystem
 * @brief Central orchestrator for the complete race lifecycle in Midnight Grind
 *
 * ## Overview
 * This subsystem coordinates all race-related systems from garage selection through
 * post-race rewards. It acts as the primary interface for UI systems to initiate
 * and monitor races.
 *
 * ## Architecture
 * - Operates as a state machine (see EMGRaceFlowState)
 * - Persists across level loads as a GameInstanceSubsystem
 * - Communicates with specialized subsystems for specific functionality:
 *   - UMGRaceDirectorSubsystem: AI pacing and rubber-banding
 *   - UMGRaceModeSubsystem: Race type logic and scoring
 *   - UMGEconomySubsystem: Reward calculations
 *   - UMGGarageSubsystem: Vehicle data
 *
 * ## For New Developers
 * 1. Call StartRace() with a configured FMGRaceSetupRequest to begin a race
 * 2. Subscribe to OnFlowStateChanged to track race progress
 * 3. Subscribe to OnRaceFinished to receive results
 * 4. Call ContinueToGarage() when player is done viewing results
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMGRaceFlowSubsystem();

	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ==========================================
	// EVENTS
	// Bind to these delegates to receive race flow notifications
	// ==========================================

	/** Called when flow state changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceFlowStateChanged OnFlowStateChanged;

	/** Called when race setup completes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceSetupComplete OnRaceSetupComplete;

	/** Called during race loading */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceLoadProgress OnRaceLoadProgress;

	/** Called when race actually starts (after countdown) */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarted OnRaceStarted;

	/** Called when race finishes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceFinished OnRaceFinished;

	/** Called when rewards have been processed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRewardsProcessed OnRewardsProcessed;

	/** Called on error */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceError OnRaceError;

	// ==========================================
	// RACE FLOW CONTROL
	// Primary API for initiating and controlling races
	// ==========================================

	/**
	 * Start race from garage
	 * Main entry point for starting any race
	 * @param Request Fully configured race setup request
	 * @return True if race setup began successfully, false if preconditions not met
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	bool StartRace(const FMGRaceSetupRequest& Request);

	/**
	 * Start a quick race with default settings
	 * Convenience for testing/quick play
	 * @param TrackID ID of the track to race on
	 * @param VehicleID ID of the player's vehicle
	 * @return True if race setup began successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	bool StartQuickRace(FName TrackID, FName VehicleID);

	/**
	 * Abort current race and return to garage
	 * Can be called at any point during the race flow
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	void AbortRace();

	/**
	 * Restart current race with same settings
	 * Reloads track and resets all race state
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	void RestartRace();

	/**
	 * Continue to garage after viewing results
	 * Called by results screen when player presses continue
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	void ContinueToGarage();

	/**
	 * Continue to next race (quick rematch)
	 * Restarts with same settings for rapid iteration
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	void ContinueToNextRace();

	// ==========================================
	// STATE QUERIES
	// Check current race flow status
	// ==========================================

	/** Get current flow state */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	EMGRaceFlowState GetFlowState() const { return CurrentState; }

	/** Is race active? */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	bool IsRaceActive() const;

	/** Is loading? */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	bool IsLoading() const { return CurrentState == EMGRaceFlowState::Loading; }

	/** Can start new race? */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	bool CanStartRace() const;

	/** Get current race setup */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	FMGRaceSetupRequest GetCurrentSetup() const { return CurrentSetup; }

	/** Get last race result */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	FMGRaceFlowResult GetLastResult() const { return LastResult; }

	/** Get loading progress */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	float GetLoadingProgress() const { return LoadingProgress; }

	// ==========================================
	// TRACK DATA
	// Query available tracks and their status
	// ==========================================

	/** Get available tracks */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|Tracks")
	TArray<FName> GetAvailableTracks() const;

	/** Get track display name */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|Tracks")
	FText GetTrackDisplayName(FName TrackID) const;

	/** Get track preview image */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|Tracks")
	UTexture2D* GetTrackPreview(FName TrackID) const;

	/** Is track unlocked? */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|Tracks")
	bool IsTrackUnlocked(FName TrackID) const;

	// ==========================================
	// AI SETUP
	// Configure AI opponents for the race
	// ==========================================

	/**
	 * Generate AI opponents for race
	 * @param Count Number of AI racers to generate
	 * @param Difficulty Difficulty level (0.0 to 1.0)
	 * @param PlayerVehicleClass Player's vehicle class for balanced matchmaking
	 * @return Array of configured AI racer setups
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow|AI")
	TArray<FMGAIRacerSetup> GenerateAIOpponents(int32 Count, float Difficulty, FName PlayerVehicleClass);

	/** Set AI opponents manually */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow|AI")
	void SetAIOpponents(const TArray<FMGAIRacerSetup>& Opponents);

	// ==========================================
	// QUICK RACE PRESETS
	// Convenience methods for common race configurations
	// ==========================================

	/** Get a quick race setup for testing */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetTestRaceSetup();

	/** Get sprint race preset */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetSprintRacePreset(FName TrackID, FName VehicleID);

	/** Get circuit race preset */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetCircuitRacePreset(FName TrackID, FName VehicleID, int32 Laps = 3);

	/** Get drift race preset */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetDriftRacePreset(FName TrackID, FName VehicleID);

	/** Get drag race preset */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetDragRacePreset(FName VehicleID);

protected:
	// ==========================================
	// INTERNAL STATE
	// Runtime state tracked by the subsystem
	// ==========================================

	/** Current flow state */
	EMGRaceFlowState CurrentState = EMGRaceFlowState::Idle;

	/** Current race setup */
	FMGRaceSetupRequest CurrentSetup;

	/** AI opponents for current race */
	TArray<FMGAIRacerSetup> CurrentAIOpponents;

	/** Last race result */
	FMGRaceFlowResult LastResult;

	/** Loading progress */
	float LoadingProgress = 0.0f;

	/** Error message if in error state */
	FString ErrorMessage;

	// ==========================================
	// CACHED SUBSYSTEM REFERENCES
	// Weak references to other subsystems for coordination
	// ==========================================

	UPROPERTY()
	TWeakObjectPtr<UMGGameStateSubsystem> GameStateSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGRaceDirectorSubsystem> RaceDirectorSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGEconomySubsystem> EconomySubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGGarageSubsystem> GarageSubsystem;

	/** Cached race game mode (only valid during race) */
	UPROPERTY()
	TWeakObjectPtr<class AMGRaceGameMode> CachedRaceGameMode;

	// ==========================================
	// INTERNAL FLOW METHODS
	// State machine execution logic
	// ==========================================

	/** Set flow state */
	void SetFlowState(EMGRaceFlowState NewState);

	/** Setup phase */
	void ExecuteSetup();

	/** Loading phase */
	void ExecuteLoading();

	/** Pre-race phase */
	void ExecutePreRace();

	/** Start countdown */
	void ExecuteCountdown();

	/** Race phase */
	void ExecuteRacing();

	/** Cooldown phase */
	void ExecuteCooldown();

	/** Results phase */
	void ExecuteResults();

	/** Process rewards */
	void ExecuteRewardProcessing();

	/** Return to garage */
	void ExecuteReturn();

	/** Handle error */
	void HandleError(const FString& Error);

	// ==========================================
	// CALLBACKS
	// Event handlers for external systems
	// ==========================================

	/** Called when level loading completes */
	void OnLevelLoaded();

	/** Called when race game mode signals race start */
	void OnRaceGameModeStart();

	/** Called when race game mode signals race end */
	void OnRaceGameModeEnd();

	/** Calculate rewards based on result */
	void CalculateRewards(FMGRaceFlowResult& Result);

	/** Apply rewards to player progression */
	void ApplyRewards(const FMGRaceFlowResult& Result);

	// ==========================================
	// HELPER METHODS
	// Utility functions for race flow operations
	// ==========================================

	/** Validate race setup */
	bool ValidateSetup(const FMGRaceSetupRequest& Request, FString& OutError);

	/** Get track level name from track ID */
	FName GetTrackLevelName(FName TrackID) const;

	/** Cache subsystem references */
	void CacheSubsystems();

	/** Bind to race game mode events */
	void BindRaceGameModeEvents();

	/** Unbind from race game mode events */
	void UnbindRaceGameModeEvents();

	/** Convert setup to game mode config */
	struct FMGRaceConfig ConvertSetupToConfig(const FMGRaceSetupRequest& Setup) const;

	/** Handle race game mode's race started event */
	UFUNCTION()
	void HandleRaceStarted();

	/** Handle race game mode's race finished event */
	UFUNCTION()
	void HandleRaceFinished(const struct FMGRaceResults& Results);
};

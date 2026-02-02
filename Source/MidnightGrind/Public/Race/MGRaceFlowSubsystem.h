// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGRaceFlowSubsystem.h
 * @brief Race Flow Subsystem - Central orchestrator for the complete race lifecycle
 *
 * This subsystem manages the entire race flow from garage selection to post-race rewards.
 * It coordinates between multiple subsystems to provide a seamless racing experience.
 *
 * ============================================================================
 * RACE FLOW STATES
 * ============================================================================
 *
 * The race flow progresses through these states:
 * @verbatim
 *   Idle -> Setup -> Loading -> PreRace -> Countdown -> Racing -> Cooldown -> Results -> ProcessingRewards -> Returning -> Idle
 * @endverbatim
 *
 * ============================================================================
 * INTEGRATION
 * ============================================================================
 *
 * Coordinates with:
 * - UMGRaceDirectorSubsystem: AI pacing and rubber-banding
 * - UMGRaceModeSubsystem: Race type logic and scoring
 * - UMGEconomySubsystem: Reward calculations
 * - UMGGarageSubsystem: Vehicle data
 * - UMGGameStateSubsystem: Global game state transitions
 *
 * @see UMGRaceStarter for simplified race initiation API
 * @see UMGRaceDirectorSubsystem for AI behavior control
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameModes/MGRaceFlowManager.h"
#include "MGRaceFlowSubsystem.generated.h"

// Forward declarations
class UMGGameStateSubsystem;
class UMGRaceDirectorSubsystem;
class UMGEconomySubsystem;
class UMGGarageSubsystem;
class AMGRaceGameMode;

// ============================================================================
// RACE SETUP REQUEST STRUCTURE
// ============================================================================

/**
 * @struct FMGRaceSetupRequest
 * @brief Configuration request for initiating a race
 *
 * Contains all parameters needed to configure and start a race.
 * Passed to UMGRaceFlowSubsystem::StartRace().
 */
USTRUCT(BlueprintType)
struct FMGRaceSetupRequest
{
	GENERATED_BODY()

	/// Track identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FName TrackID;

	/// Player's vehicle identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FName PlayerVehicleID;

	/// Race type name (Circuit, Sprint, Drift, Drag, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName RaceType;

	/// Number of laps for circuit races
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 LapCount = 3;

	/// Number of AI opponents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	int32 AICount = 7;

	/// AI difficulty level (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AIDifficulty = 0.5f;

	/// Time of day (0.0 = midnight, 0.5 = noon)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	float TimeOfDay = 0.0f;

	/// Weather intensity (0.0 = clear, 1.0 = storm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	float Weather = 0.0f;

	/// Base cash reward for 1st place
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 BaseCashReward = 5000;

	/// Base reputation reward
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BaseRepReward = 100;

	/// Is this a pink slip (vehicle wager) race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	bool bIsPinkSlip = false;

	/// Vehicle ID being wagered in pink slip race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	FName PinkSlipVehicleID;
};

// ============================================================================
// AI RACER SETUP STRUCTURE
// ============================================================================

/**
 * @struct FMGAIRacerSetup
 * @brief Configuration for a single AI opponent
 */
USTRUCT(BlueprintType)
struct FMGAIRacerSetup
{
	GENERATED_BODY()

	/// AI racer display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FString DisplayName;

	/// Vehicle ID for this AI racer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FName VehicleID;

	/// Skill level (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SkillLevel = 0.5f;

	/// Aggression level (0.0 = passive, 1.0 = very aggressive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Aggression = 0.5f;

	/// Is this a story/career rival
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsRival = false;
};

// ============================================================================
// RACE FLOW RESULT STRUCTURE
// ============================================================================

/**
 * @struct FMGRaceFlowResult
 * @brief Complete results from a finished race flow
 */
USTRUCT(BlueprintType)
struct FMGRaceFlowResult
{
	GENERATED_BODY()

	/// Was the race completed (not aborted)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bRaceCompleted = false;

	/// Did the player finish the race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPlayerFinished = false;

	/// Player's finishing position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 PlayerPosition = 0;

	/// Total number of racers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 TotalRacers = 0;

	/// Player's total race time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float PlayerTotalTime = 0.0f;

	/// Did the player win (1st place)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPlayerWon = false;

	/// Finish order (array of racer IDs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> FinishOrder;

	/// Cash earned from this race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 CashEarned = 0;

	/// Reputation earned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 ReputationEarned = 0;

	/// XP earned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 XPEarned = 0;

	/// Vehicle won via pink slip (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName PinkSlipWonVehicleID;

	/// Vehicle lost via pink slip (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName PinkSlipLostVehicleID;
};

// EMGRaceFlowState - REMOVED (duplicate)
// Canonical definition in: GameModes/MGRaceFlowManager.h


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

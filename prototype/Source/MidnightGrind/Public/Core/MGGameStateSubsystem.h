// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGGameStateSubsystem.h
 * @brief Game State Subsystem - Controls overall game flow and state transitions
 *
 * This subsystem manages the high-level game state machine for Midnight Grind.
 * It controls transitions between major game states (menus, garage, racing, etc.)
 * and handles level loading coordination.
 *
 * @section Overview
 * The Game State Subsystem is a GameInstanceSubsystem, meaning it persists across
 * level transitions and exists for the entire lifetime of the game session. Use this
 * subsystem when you need to:
 * - Navigate between game screens (main menu, garage, lobby browser, etc.)
 * - Check what state the game is currently in
 * - Listen for state changes to update UI or other systems
 * - Manage level loading with proper state coordination
 *
 * @section Usage
 * Access this subsystem through the GameInstance:
 * @code
 * UMGGameStateSubsystem* StateSubsystem = GameInstance->GetSubsystem<UMGGameStateSubsystem>();
 * if (StateSubsystem->GetCurrentState() == EMGGameState::MainMenu)
 * {
 *     StateSubsystem->GoToGarage();
 * }
 * @endcode
 *
 * @section StateFlow State Flow
 * Common state transitions:
 * - Boot -> MainMenu (initial startup)
 * - MainMenu -> Garage, LobbyBrowser, Settings, Leaderboards
 * - Garage -> LobbyBrowser, MainMenu
 * - LobbyBrowser -> InLobby
 * - InLobby -> Loading -> PreRace -> Racing -> PostRace
 * - Racing -> PhotoMode, Replay (can return to Racing)
 *
 * @see UMGGameInstance For the parent game instance
 * @see EMGGameState For the list of all game states
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGGameStateSubsystem.generated.h"

class UWorld;

// ============================================================================
// ENUMS - Game State Definitions
// ============================================================================

/**
 * @brief Top-level game state enumeration
 *
 * Defines all possible high-level states the game can be in.
 * These states determine what UI is shown, what systems are active,
 * and what transitions are valid.
 *
 * @note States follow a logical flow but allow some flexibility.
 *       Use CanTransitionTo() to check if a transition is valid.
 */
UENUM(BlueprintType)
enum class EMGGameState : uint8
{
	/** Initial boot/splash screen - shown during startup initialization */
	Boot,
	/** Main menu - primary navigation hub after boot */
	MainMenu,
	/** Garage/customization - vehicle selection and tuning */
	Garage,
	/** Lobby browser - searching for multiplayer sessions */
	LobbyBrowser,
	/** In lobby - waiting room before race starts */
	InLobby,
	/** Loading - transitioning to race level */
	Loading,
	/** Pre-race - grid lineup before countdown */
	PreRace,
	/** Racing - active gameplay */
	Racing,
	/** Post-race - results screen after race ends */
	PostRace,
	/** Replay - watching recorded race footage */
	Replay,
	/** Photo mode - in-game photography (can enter from Racing or Replay) */
	PhotoMode,
	/** Leaderboards - viewing rankings and records */
	Leaderboards,
	/** Settings - game configuration menu */
	Settings
};

// ============================================================================
// STRUCTS - State Transition Data
// ============================================================================

/**
 * @brief Request structure for initiating a state transition
 *
 * Use this struct when you need fine-grained control over state transitions,
 * such as specifying a level to load or passing context data. For simple
 * transitions, use the GoToState() or convenience functions like GoToGarage().
 *
 * @see RequestStateTransition() To execute a transition with this struct
 */
USTRUCT(BlueprintType)
struct FMGStateTransition
{
	GENERATED_BODY()

	/** The state to transition to */
	UPROPERTY(BlueprintReadWrite)
	EMGGameState TargetState = EMGGameState::MainMenu;

	/** Level to load during this transition (empty = no level change) */
	UPROPERTY(BlueprintReadWrite)
	FName LevelName;

	/** Key-value pairs to pass through the transition (e.g., TrackID, SessionID) */
	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FString> ContextData;

	/** If true, bypass validation checks (use with caution - for error recovery) */
	UPROPERTY(BlueprintReadWrite)
	bool bForce = false;
};

/**
 * @brief Context information provided when a state change occurs
 *
 * This struct is passed to OnGameStateChanged event listeners,
 * providing details about what changed and any associated data.
 * Useful for UI transitions, analytics, and state-dependent logic.
 */
USTRUCT(BlueprintType)
struct FMGStateChangeContext
{
	GENERATED_BODY()

	/** The state we transitioned FROM */
	UPROPERTY(BlueprintReadOnly)
	EMGGameState PreviousState = EMGGameState::Boot;

	/** The state we transitioned TO (current state) */
	UPROPERTY(BlueprintReadOnly)
	EMGGameState NewState = EMGGameState::MainMenu;

	/** Data passed through the transition (TrackID, SessionID, etc.) */
	UPROPERTY(BlueprintReadOnly)
	TMap<FName, FString> ContextData;

	/** How long we spent in the previous state (seconds) - useful for analytics */
	UPROPERTY(BlueprintReadOnly)
	float TimeInPreviousState = 0.0f;
};

// ============================================================================
// DELEGATES - Event Signatures
// ============================================================================

/// Broadcast when game state successfully changes - provides full context
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, const FMGStateChangeContext&, Context);

/// Broadcast when a transition is requested (before validation) - allows preview/veto
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateTransitionRequested, EMGGameState, FromState, EMGGameState, ToState);

/// Broadcast when a transition fails validation - provides reason for UI feedback
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateTransitionBlocked, const FString&, Reason);

/// Broadcast when async level loading begins - show loading screen
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingStarted);

/// Broadcast when level loading completes - hide loading screen
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingCompleted);

// ============================================================================
// CLASS - UMGGameStateSubsystem
// ============================================================================

/**
 * @brief Game State Subsystem - Controls overall game flow and state transitions
 *
 * This subsystem implements a state machine that governs the high-level flow
 * of the game. It handles transitions between menus, garage, racing, and
 * coordinates level loading with proper state management.
 *
 * @section Features
 * - State machine with validation (prevents invalid transitions)
 * - Async level loading with progress tracking
 * - Context data persistence across transitions
 * - Event hooks for UI and other systems to respond to changes
 *
 * @note This subsystem is LOCAL only - it does not replicate. For multiplayer
 *       race state, see AMGGameState which handles replicated race phases.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGGameStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Called when the subsystem is created - sets up initial state
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called when the subsystem is destroyed - cleanup
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// Bind to these delegates to respond to state changes.
	// Use AddDynamic() in C++ or bind in Blueprints.
	// ==========================================

	/**
	 * @brief Fired when game state successfully changes
	 * Use this to update UI, start/stop music, enable/disable systems, etc.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameStateChanged OnGameStateChanged;

	/**
	 * @brief Fired when a transition is requested, BEFORE validation
	 * Can be used to prepare for a transition or log analytics
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStateTransitionRequested OnStateTransitionRequested;

	/**
	 * @brief Fired when a transition fails validation
	 * Use this to show error messages to the player
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStateTransitionBlocked OnStateTransitionBlocked;

	/**
	 * @brief Fired when async level loading begins
	 * Show your loading screen when this fires
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadingStarted OnLoadingStarted;

	/**
	 * @brief Fired when level loading completes
	 * Hide your loading screen when this fires
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadingCompleted OnLoadingCompleted;

	// ==========================================
	// STATE MANAGEMENT
	// Core functions for querying and changing game state.
	// These are the primary API for controlling game flow.
	// ==========================================

	/**
	 * @brief Get the current game state
	 * @return The active game state
	 */
	UFUNCTION(BlueprintPure, Category = "GameState")
	EMGGameState GetCurrentState() const { return CurrentState; }

	/**
	 * @brief Get the previous game state (before current)
	 * @return The state we transitioned from
	 * Useful for "back" navigation or conditional logic
	 */
	UFUNCTION(BlueprintPure, Category = "GameState")
	EMGGameState GetPreviousState() const { return PreviousState; }

	/**
	 * @brief Request a state transition with full control
	 * @param Transition The transition request containing target state, level, and context
	 * @return True if transition was started, false if blocked
	 * Use this when you need to pass context data or specify a level to load
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState")
	bool RequestStateTransition(const FMGStateTransition& Transition);

	/**
	 * @brief Simple transition to a state (no context data)
	 * @param NewState The target state
	 * @return True if transition was started, false if blocked
	 * Use this for simple navigation without extra data
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState")
	bool GoToState(EMGGameState NewState);

	/**
	 * @brief Check if transitioning to a state is allowed
	 * @param TargetState The state to check
	 * @return True if the transition would be valid
	 * Use this to enable/disable UI buttons
	 */
	UFUNCTION(BlueprintPure, Category = "GameState")
	bool CanTransitionTo(EMGGameState TargetState) const;

	/**
	 * @brief Get all states we can currently transition to
	 * @return Array of valid target states
	 * Useful for dynamically building navigation menus
	 */
	UFUNCTION(BlueprintPure, Category = "GameState")
	TArray<EMGGameState> GetValidTransitions() const;

	/**
	 * @brief Get how long we've been in the current state
	 * @return Time in seconds since entering current state
	 */
	UFUNCTION(BlueprintPure, Category = "GameState")
	float GetTimeInCurrentState() const;

	/**
	 * @brief Get localized display name for a state
	 * @param State The state to get the name for
	 * @return Human-readable name (e.g., "Main Menu", "Racing")
	 */
	UFUNCTION(BlueprintPure, Category = "GameState")
	FText GetStateDisplayName(EMGGameState State) const;

	// ==========================================
	// COMMON TRANSITIONS
	// Convenience functions for frequent navigation patterns.
	// These handle common state transitions with appropriate defaults.
	// ==========================================

	/**
	 * @brief Navigate to main menu
	 * Loads the main menu level and transitions to MainMenu state
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void GoToMainMenu();

	/**
	 * @brief Navigate to garage
	 * Loads the garage level for vehicle customization
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void GoToGarage();

	/**
	 * @brief Navigate to multiplayer lobby browser
	 * Shows available sessions to join
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void GoToLobbyBrowser();

	/**
	 * @brief Join a specific lobby
	 * @param SessionID The unique ID of the session to join
	 * Called after selecting a session from the browser
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void EnterLobby(const FString& SessionID);

	/**
	 * @brief Start loading a race track
	 * @param TrackID The ID of the track to load
	 * Initiates async level loading and shows loading screen
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void StartRaceLoading(FName TrackID);

	/**
	 * @brief Transition to pre-race state
	 * Called after loading completes - shows starting grid
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void BeginPreRace();

	/**
	 * @brief Start the race
	 * Called when countdown finishes - enables player control
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void StartRacing();

	/**
	 * @brief End the race and show results
	 * Called when race concludes - displays final standings
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void EndRace();

	/**
	 * @brief Enter replay viewing mode
	 * Can be entered from Racing or PostRace states
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void EnterReplayMode();

	/**
	 * @brief Exit replay mode
	 * Returns to previous state (Racing or PostRace)
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void ExitReplayMode();

	/**
	 * @brief Enter photo mode
	 * Pauses game and enables camera controls
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void EnterPhotoMode();

	/**
	 * @brief Exit photo mode
	 * Returns to racing or replay
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void ExitPhotoMode();

	// ==========================================
	// LOADING
	// Query loading state for UI (progress bars, status text).
	// ==========================================

	/**
	 * @brief Check if currently loading a level
	 * @return True during async level load operations
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|Loading")
	bool IsLoading() const { return bIsLoading; }

	/**
	 * @brief Get current loading progress
	 * @return Value from 0.0 (started) to 1.0 (complete)
	 * Bind this to a progress bar widget
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|Loading")
	float GetLoadingProgress() const { return LoadingProgress; }

	/**
	 * @brief Get current loading status description
	 * @return Localized text like "Loading track..." or "Initializing physics..."
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|Loading")
	FText GetLoadingStatusText() const { return LoadingStatusText; }

	// ==========================================
	// CONTEXT DATA
	// Key-value storage for passing data between states.
	// Persists across transitions until explicitly cleared.
	// ==========================================

	/**
	 * @brief Store context data for the current/upcoming transition
	 * @param Key The identifier for this data (e.g., "TrackID", "SessionID")
	 * @param Value The string value to store
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Context")
	void SetContextData(FName Key, const FString& Value);

	/**
	 * @brief Retrieve stored context data
	 * @param Key The identifier to look up
	 * @return The stored value, or empty string if not found
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|Context")
	FString GetContextData(FName Key) const;

	/**
	 * @brief Clear all stored context data
	 * Typically called when returning to main menu
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState|Context")
	void ClearContextData();

	/**
	 * @brief Convenience getter for current track
	 * @return The TrackID from context data, or NAME_None if not set
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|Context")
	FName GetCurrentTrackID() const;

	/**
	 * @brief Convenience getter for current multiplayer session
	 * @return The SessionID from context data, or empty if not in session
	 */
	UFUNCTION(BlueprintPure, Category = "GameState|Context")
	FString GetCurrentSessionID() const;

protected:
	// ==========================================
	// STATE
	// Internal state tracking - do not modify directly.
	// Use public API functions to change state.
	// ==========================================

	/// The currently active game state
	EMGGameState CurrentState = EMGGameState::Boot;

	/// The state we transitioned from (for back navigation)
	EMGGameState PreviousState = EMGGameState::Boot;

	/// World time when we entered the current state (for duration tracking)
	float StateEnterTime = 0.0f;

	/// True while async level loading is in progress
	bool bIsLoading = false;

	/// Current loading progress (0.0 to 1.0)
	float LoadingProgress = 0.0f;

	/// Human-readable loading status for UI
	FText LoadingStatusText;

	/// Transition to execute after loading completes
	FMGStateTransition PendingTransition;

	/// Key-value context data that persists across transitions
	TMap<FName, FString> ContextData;

	/// Target state after current level load finishes
	EMGGameState PostLoadState = EMGGameState::MainMenu;

	// ==========================================
	// CONFIGURATION
	// Level names and other config - set in derived Blueprint or code.
	// ==========================================

	/// The level to load for the main menu (can be set in subclass)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FName MainMenuLevel = FName("MainMenu");

	/// The level to load for the garage/customization screen
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FName GarageLevel = FName("Garage");

	// ==========================================
	// INTERNAL
	// Private implementation functions - not part of public API.
	// ==========================================

	/// Actually perform the state transition (after validation)
	void ExecuteTransition(const FMGStateTransition& Transition);

	/// Check if transition is allowed, returns false with reason if not
	bool ValidateTransition(EMGGameState FromState, EMGGameState ToState, FString& OutReason) const;

	/// Get list of states that can be transitioned to from a given state
	TArray<EMGGameState> GetValidTransitionsForState(EMGGameState State) const;

	/// Called when async level load completes successfully
	void OnLevelLoaded();

	/// Update loading UI state (called during async load)
	void UpdateLoadingProgress(float Progress, const FText& StatusText);

	/// Handle entering a new state (setup, events)
	void EnterState(EMGGameState NewState, const TMap<FName, FString>& TransitionData);

	/// Handle exiting a state (cleanup)
	void ExitState(EMGGameState OldState);

	/// Start async level loading with callback
	void LoadLevelAsync(FName LevelName, EMGGameState TargetState);

	/// Callback when UE async load operation completes
	UFUNCTION()
	void OnAsyncLoadComplete();
};

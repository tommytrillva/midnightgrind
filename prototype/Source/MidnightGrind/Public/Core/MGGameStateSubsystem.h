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

/**
 * State transition request
 */
USTRUCT(BlueprintType)
struct FMGStateTransition
{
	GENERATED_BODY()

	/** Target state */
	UPROPERTY(BlueprintReadWrite)
	EMGGameState TargetState = EMGGameState::MainMenu;

	/** Level to load (if applicable) */
	UPROPERTY(BlueprintReadWrite)
	FName LevelName;

	/** Additional context data */
	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FString> ContextData;

	/** Force transition (skip validation) */
	UPROPERTY(BlueprintReadWrite)
	bool bForce = false;
};

/**
 * State change context
 */
USTRUCT(BlueprintType)
struct FMGStateChangeContext
{
	GENERATED_BODY()

	/** Previous state */
	UPROPERTY(BlueprintReadOnly)
	EMGGameState PreviousState = EMGGameState::Boot;

	/** New state */
	UPROPERTY(BlueprintReadOnly)
	EMGGameState NewState = EMGGameState::MainMenu;

	/** Transition data */
	UPROPERTY(BlueprintReadOnly)
	TMap<FName, FString> ContextData;

	/** Time in previous state */
	UPROPERTY(BlueprintReadOnly)
	float TimeInPreviousState = 0.0f;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, const FMGStateChangeContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateTransitionRequested, EMGGameState, FromState, EMGGameState, ToState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateTransitionBlocked, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingCompleted);

/**
 * Game State Subsystem
 * Controls overall game flow and state transitions
 *
 * Features:
 * - State machine for game flow
 * - Level loading management
 * - State validation
 * - Transition hooks
 */
UCLASS()
class MIDNIGHTGRIND_API UMGGameStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when game state changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameStateChanged OnGameStateChanged;

	/** Called when transition is requested (before validation) */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStateTransitionRequested OnStateTransitionRequested;

	/** Called when transition is blocked */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStateTransitionBlocked OnStateTransitionBlocked;

	/** Called when level loading starts */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadingStarted OnLoadingStarted;

	/** Called when level loading completes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadingCompleted OnLoadingCompleted;

	// ==========================================
	// STATE MANAGEMENT
	// ==========================================

	/** Get current game state */
	UFUNCTION(BlueprintPure, Category = "GameState")
	EMGGameState GetCurrentState() const { return CurrentState; }

	/** Get previous game state */
	UFUNCTION(BlueprintPure, Category = "GameState")
	EMGGameState GetPreviousState() const { return PreviousState; }

	/** Request state transition */
	UFUNCTION(BlueprintCallable, Category = "GameState")
	bool RequestStateTransition(const FMGStateTransition& Transition);

	/** Quick transition to state */
	UFUNCTION(BlueprintCallable, Category = "GameState")
	bool GoToState(EMGGameState NewState);

	/** Can transition to state */
	UFUNCTION(BlueprintPure, Category = "GameState")
	bool CanTransitionTo(EMGGameState TargetState) const;

	/** Get valid transitions from current state */
	UFUNCTION(BlueprintPure, Category = "GameState")
	TArray<EMGGameState> GetValidTransitions() const;

	/** Get time in current state */
	UFUNCTION(BlueprintPure, Category = "GameState")
	float GetTimeInCurrentState() const;

	/** Get state display name */
	UFUNCTION(BlueprintPure, Category = "GameState")
	FText GetStateDisplayName(EMGGameState State) const;

	// ==========================================
	// COMMON TRANSITIONS
	// ==========================================

	/** Go to main menu */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void GoToMainMenu();

	/** Go to garage */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void GoToGarage();

	/** Go to lobby browser */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void GoToLobbyBrowser();

	/** Enter lobby */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void EnterLobby(const FString& SessionID);

	/** Start race loading */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void StartRaceLoading(FName TrackID);

	/** Begin pre-race countdown */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void BeginPreRace();

	/** Start racing */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void StartRacing();

	/** End race and show results */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void EndRace();

	/** Enter replay mode */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void EnterReplayMode();

	/** Exit replay mode */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void ExitReplayMode();

	/** Enter photo mode */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void EnterPhotoMode();

	/** Exit photo mode */
	UFUNCTION(BlueprintCallable, Category = "GameState|Navigation")
	void ExitPhotoMode();

	// ==========================================
	// LOADING
	// ==========================================

	/** Is currently loading */
	UFUNCTION(BlueprintPure, Category = "GameState|Loading")
	bool IsLoading() const { return bIsLoading; }

	/** Get loading progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "GameState|Loading")
	float GetLoadingProgress() const { return LoadingProgress; }

	/** Get loading status text */
	UFUNCTION(BlueprintPure, Category = "GameState|Loading")
	FText GetLoadingStatusText() const { return LoadingStatusText; }

	// ==========================================
	// CONTEXT DATA
	// ==========================================

	/** Set context data for transitions */
	UFUNCTION(BlueprintCallable, Category = "GameState|Context")
	void SetContextData(FName Key, const FString& Value);

	/** Get context data */
	UFUNCTION(BlueprintPure, Category = "GameState|Context")
	FString GetContextData(FName Key) const;

	/** Clear context data */
	UFUNCTION(BlueprintCallable, Category = "GameState|Context")
	void ClearContextData();

	/** Get current track ID (from context) */
	UFUNCTION(BlueprintPure, Category = "GameState|Context")
	FName GetCurrentTrackID() const;

	/** Get current session ID (from context) */
	UFUNCTION(BlueprintPure, Category = "GameState|Context")
	FString GetCurrentSessionID() const;

protected:
	// ==========================================
	// STATE
	// ==========================================

	/** Current game state */
	EMGGameState CurrentState = EMGGameState::Boot;

	/** Previous game state */
	EMGGameState PreviousState = EMGGameState::Boot;

	/** Time entered current state */
	float StateEnterTime = 0.0f;

	/** Is loading a level */
	bool bIsLoading = false;

	/** Loading progress */
	float LoadingProgress = 0.0f;

	/** Loading status text */
	FText LoadingStatusText;

	/** Pending transition (after load) */
	FMGStateTransition PendingTransition;

	/** Context data */
	TMap<FName, FString> ContextData;

	/** State after loading completes */
	EMGGameState PostLoadState = EMGGameState::MainMenu;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Main menu level */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FName MainMenuLevel = FName("MainMenu");

	/** Garage level */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FName GarageLevel = FName("Garage");

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Execute state transition */
	void ExecuteTransition(const FMGStateTransition& Transition);

	/** Validate transition */
	bool ValidateTransition(EMGGameState FromState, EMGGameState ToState, FString& OutReason) const;

	/** Get valid transitions for state */
	TArray<EMGGameState> GetValidTransitionsForState(EMGGameState State) const;

	/** Handle level loaded */
	void OnLevelLoaded();

	/** Update loading progress */
	void UpdateLoadingProgress(float Progress, const FText& StatusText);

	/** Enter state */
	void EnterState(EMGGameState NewState, const TMap<FName, FString>& TransitionData);

	/** Exit state */
	void ExitState(EMGGameState OldState);

	/** Load level async */
	void LoadLevelAsync(FName LevelName, EMGGameState TargetState);

	/** Handle async load complete */
	UFUNCTION()
	void OnAsyncLoadComplete();
};

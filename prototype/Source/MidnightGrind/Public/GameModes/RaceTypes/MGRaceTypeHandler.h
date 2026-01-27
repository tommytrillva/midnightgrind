/**
 * @file MGRaceTypeHandler.h
 * @brief Base Race Type Handler - Abstract foundation for all race mode implementations
 *
 * This file defines the core architecture for Midnight Grind's race type system.
 * The UMGRaceTypeHandler class serves as the abstract base that all specific race
 * modes (Circuit, Sprint, Drift, Drag, etc.) inherit from.
 *
 * @section overview Overview
 * Race Type Handlers encapsulate the unique rules, scoring systems, and win conditions
 * for each race mode. They are created and managed by the Race Game Mode, and they
 * receive callbacks for race events (start, tick, checkpoint, lap completion, etc.).
 *
 * @section architecture Architecture
 * - UMGRaceTypeHandler: Abstract base class defining the handler interface
 * - UMGRaceTypeFactory: Factory for creating handlers by race type enum
 * - Concrete handlers: UMGCircuitRaceHandler, UMGSprintRaceHandler, etc.
 *
 * @section responsibilities Handler Responsibilities
 * - Define win/completion conditions for the race type
 * - Implement scoring and ranking logic
 * - Provide race-specific UI data (lap counter, score display, etc.)
 * - Calculate rewards (credits, XP, reputation)
 * - Broadcast race events to the UI and other systems
 *
 * @section usage Usage Example
 * @code
 * // Create a handler for a circuit race
 * UMGRaceTypeHandler* Handler = UMGRaceTypeFactory::CreateRaceTypeHandler(this, EMGRaceType::Circuit);
 * Handler->Initialize(GameMode);
 * Handler->Activate();
 * @endcode
 *
 * @see UMGCircuitRaceHandler, UMGSprintRaceHandler, UMGDriftRaceHandler
 * @see UMGDragRaceHandler, UMGTimeTrialHandler, UMGTougeHandler, UMGHighwayBattleHandler
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameModes/MGRaceGameMode.h"
#include "MGRaceTypeHandler.generated.h"

/// Forward declarations
class AMGRaceGameMode;
class AMGVehiclePawn;
class AMGCheckpoint;

// ============================================================================
// ENUMS AND STRUCTS
// ============================================================================

/**
 * @brief Result of checking whether a racer has completed the race
 *
 * This enum is returned by CheckCompletionCondition() to indicate the current
 * state of a racer's progress through the race.
 */
UENUM(BlueprintType)
enum class EMGRaceCompletionResult : uint8
{
	/** Race still in progress - racer has not met any completion condition */
	InProgress,
	/** Racer has finished successfully - crossed finish line or met win condition */
	Finished,
	/** Racer did not finish (DNF) - timed out, gave up, or failed to complete */
	DNF,
	/** Racer disqualified - violated rules (e.g., red light in drag race, excessive cutting) */
	Disqualified
};

/**
 * @brief Score update event data for score-based race modes
 *
 * This struct is used to broadcast score changes in modes like Drift racing
 * where players accumulate points. It contains all information needed for
 * the UI to display score popups and update totals.
 */
USTRUCT(BlueprintType)
struct FMGScoreUpdate
{
	GENERATED_BODY()

	/// Index of the racer who earned the score (matches GameMode's racer array)
	UPROPERTY(BlueprintReadOnly)
	int32 RacerIndex = -1;

	/// Points earned in this update (can be negative for penalties)
	UPROPERTY(BlueprintReadOnly)
	float ScoreDelta = 0.0f;

	/// Racer's new total score after this update
	UPROPERTY(BlueprintReadOnly)
	float TotalScore = 0.0f;

	/// Human-readable reason for the score (e.g., "Drift Combo x5", "Clean Sector")
	UPROPERTY(BlueprintReadOnly)
	FText ScoreReason;

	/// Multiplier applied to this score (for displaying combo bonuses)
	UPROPERTY(BlueprintReadOnly)
	float Multiplier = 1.0f;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/**
 * @brief Broadcast when a racer's score changes (drift/time trial modes)
 * @param ScoreUpdate Contains all score change information
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated, const FMGScoreUpdate&, ScoreUpdate);

/**
 * @brief Broadcast when a racer completes a sector (all race types)
 * @param RacerIndex Index of the racer who completed the sector
 * @param SectorIndex Which sector was completed (0-indexed)
 * @param SectorTime Time taken to complete the sector in seconds
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSectorComplete, int32, RacerIndex, int32, SectorIndex, float, SectorTime);

// ============================================================================
// MAIN HANDLER CLASS
// ============================================================================

/**
 * @brief Abstract base class for all race type handlers
 *
 * Each race type (Circuit, Sprint, Drift, Drag, Time Trial, Touge, Highway Battle)
 * extends this class to provide its specific logic for win conditions, scoring,
 * and race-specific mechanics.
 *
 * @section lifecycle Lifecycle
 * 1. Created via UMGRaceTypeFactory::CreateRaceTypeHandler()
 * 2. Initialize() called with owning game mode
 * 3. Activate() called when race type becomes active
 * 4. Race flow methods called during race (OnRaceStarted, OnRaceTick, etc.)
 * 5. Deactivate() called when race ends or mode changes
 * 6. Reset() can be called to reuse the handler for a new race
 *
 * @section subclassing Subclassing Guidelines
 * - Override GetRaceType() and GetDisplayName() (pure virtual)
 * - Override CheckCompletionCondition() for custom win logic
 * - Override CalculatePositions() for custom ranking
 * - Override OnRaceTick() for per-frame logic (drift detection, gap tracking, etc.)
 * - Set ShouldShowLapCounter/Position/Score based on race type needs
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGRaceTypeHandler : public UObject
{
	GENERATED_BODY()

public:
	UMGRaceTypeHandler();

	// ==========================================
	// INITIALIZATION
	// These methods handle handler setup and state management
	// ==========================================

	/**
	 * @brief Initialize the handler with the owning game mode
	 * @param InGameMode The race game mode that owns this handler
	 * @note Called once when handler is created, before activation
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type")
	virtual void Initialize(AMGRaceGameMode* InGameMode);

	/**
	 * @brief Called when this race type becomes the active mode
	 * @note Use this to set up race-type-specific state and subscribe to events
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type")
	virtual void Activate();

	/**
	 * @brief Called when this race type is no longer active
	 * @note Use this to clean up state and unsubscribe from events
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type")
	virtual void Deactivate();

	/**
	 * @brief Reset handler state for a new race
	 * @note Called between races to clear scores, positions, and other per-race data
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type")
	virtual void Reset();

	// ==========================================
	// RACE FLOW
	// These methods are called by the game mode at key race moments
	// ==========================================

	/**
	 * @brief Called when the pre-race countdown begins (3, 2, 1...)
	 * @note Racers should be in position but not yet allowed to move
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnCountdownStarted();

	/**
	 * @brief Called when the race officially starts (GO!)
	 * @note This is when timing begins and racers can move
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRaceStarted();

	/**
	 * @brief Called every frame during active racing
	 * @param DeltaTime Time since last tick in seconds
	 * @note Use for continuous tracking (drift detection, gap measurement, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRaceTick(float DeltaTime);

	/**
	 * @brief Called when the race is paused
	 * @note Pause any timers or continuous tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRacePaused();

	/**
	 * @brief Called when the race resumes from pause
	 * @note Resume any paused timers or tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRaceResumed();

	/**
	 * @brief Called when the race ends (all racers finished or time expired)
	 * @note Finalize scores and prepare results data
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRaceEnded();

	// ==========================================
	// CHECKPOINT/PROGRESS HANDLING
	// These methods track racer progress through the course
	// ==========================================

	/**
	 * @brief Called when any racer passes through a checkpoint
	 * @param RacerIndex Index of the racer who passed the checkpoint
	 * @param CheckpointIndex Index of the checkpoint that was passed
	 * @note Checkpoints are used for position calculation and sector timing
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Progress")
	virtual void OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex);

	/**
	 * @brief Called when any racer completes a lap
	 * @param RacerIndex Index of the racer who completed the lap
	 * @param LapTime Time taken to complete the lap in seconds
	 * @note Only relevant for lap-based race types (Circuit, Drift with laps)
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Progress")
	virtual void OnLapCompleted(int32 RacerIndex, float LapTime);

	/**
	 * @brief Check if a racer has met the completion conditions
	 * @param RacerIndex Index of the racer to check
	 * @return The racer's current completion status
	 * @note Called frequently to detect when racers finish
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Progress")
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex);

	// ==========================================
	// SCORING & RANKING
	// These methods handle position calculation and score tracking
	// ==========================================

	/**
	 * @brief Calculate current positions for all racers
	 * @param OutPositions Array to fill with position assignments (index = racer, value = position)
	 * @note Position 1 is first place. Called frequently for HUD updates.
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Scoring")
	virtual void CalculatePositions(TArray<int32>& OutPositions);

	/**
	 * @brief Get the current score for a racer
	 * @param RacerIndex Index of the racer
	 * @return Current score (only meaningful for score-based modes)
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Scoring")
	virtual float GetRacerScore(int32 RacerIndex) const;

	/**
	 * @brief Get the target score to win (if applicable)
	 * @return Target score, or 0 if not score-based
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Scoring")
	virtual float GetTargetScore() const { return 0.0f; }

	/**
	 * @brief Check if this race type uses score-based competition
	 * @return True for Drift races, false for position-based races
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Scoring")
	virtual bool IsScoreBased() const { return false; }

	// ==========================================
	// RACE TYPE INFO
	// These methods provide metadata about the race type for UI
	// ==========================================

	/**
	 * @brief Get the enum value for this race type
	 * @return The EMGRaceType this handler manages
	 * @note Pure virtual - must be implemented by subclasses
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual EMGRaceType GetRaceType() const PURE_VIRTUAL(UMGRaceTypeHandler::GetRaceType, return EMGRaceType::Circuit;);

	/**
	 * @brief Get the localized display name for this race type
	 * @return Display name (e.g., "Circuit Race", "Drift Battle")
	 * @note Pure virtual - must be implemented by subclasses
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual FText GetDisplayName() const PURE_VIRTUAL(UMGRaceTypeHandler::GetDisplayName, return FText::GetEmpty(););

	/**
	 * @brief Get a localized description of this race type
	 * @return Description explaining the rules and objectives
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual FText GetDescription() const { return FText::GetEmpty(); }

	/**
	 * @brief Get the icon texture for this race type
	 * @return Icon for menus and HUD, or nullptr if not set
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual UTexture2D* GetIcon() const { return nullptr; }

	/**
	 * @brief Should the HUD show a lap counter?
	 * @return True for Circuit and lap-based Drift, false for Sprint/Drag
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual bool ShouldShowLapCounter() const { return true; }

	/**
	 * @brief Should the HUD show race position (1st, 2nd, etc.)?
	 * @return True for most modes, false for Time Trial (solo)
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual bool ShouldShowPosition() const { return true; }

	/**
	 * @brief Should the HUD show a score display?
	 * @return True for Drift mode, false for position-based modes
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual bool ShouldShowScore() const { return false; }

	/**
	 * @brief Get the format string for progress display
	 * @return Format string (e.g., "Lap {0}/{1}" or "{0}m to go")
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual FText GetProgressFormat() const;

	// ==========================================
	// CREDITS & REWARDS
	// These methods calculate post-race rewards
	// ==========================================

	/**
	 * @brief Calculate credits earned for finishing position
	 * @param Position Finishing position (1 = first place)
	 * @param TotalRacers Total number of racers in the race
	 * @return Credits to award the player
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Rewards")
	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const;

	/**
	 * @brief Calculate experience points earned for finishing position
	 * @param Position Finishing position (1 = first place)
	 * @param TotalRacers Total number of racers in the race
	 * @return XP to award the player
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Rewards")
	virtual int32 CalculateXPForPosition(int32 Position, int32 TotalRacers) const;

	/**
	 * @brief Calculate reputation earned from the race
	 * @param Position Finishing position
	 * @param bWon Whether the player won the race
	 * @return Reputation points to award
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Rewards")
	virtual int32 CalculateReputationEarned(int32 Position, bool bWon) const;

	// ==========================================
	// EVENTS
	// Delegates for broadcasting race events to UI and other systems
	// ==========================================

	/** Broadcast when a racer's score changes (drift/time trial modes) */
	UPROPERTY(BlueprintAssignable, Category = "Race Type|Events")
	FOnScoreUpdated OnScoreUpdated;

	/** Broadcast when any racer completes a track sector */
	UPROPERTY(BlueprintAssignable, Category = "Race Type|Events")
	FOnSectorComplete OnSectorComplete;

protected:
	// ==========================================
	// PROTECTED MEMBERS
	// State and helper methods for subclasses
	// ==========================================

	/** Weak reference to the owning game mode (prevents circular references) */
	UPROPERTY()
	TWeakObjectPtr<AMGRaceGameMode> GameMode;

	/** Whether this handler is currently active and processing race events */
	UPROPERTY()
	bool bIsActive = false;

	/** Score storage for score-based modes (key = racer index, value = score) */
	UPROPERTY()
	TMap<int32, float> RacerScores;

	/**
	 * @brief Safely get the owning game mode
	 * @return Game mode pointer, or nullptr if invalid
	 */
	AMGRaceGameMode* GetGameMode() const;

	/**
	 * @brief Helper to broadcast a score update event
	 * @param RacerIndex Racer who earned the score
	 * @param Delta Points earned in this update
	 * @param Total New total score
	 * @param Reason Display text for the score source
	 * @param Multiplier Optional multiplier for display (default 1.0)
	 */
	void BroadcastScoreUpdate(int32 RacerIndex, float Delta, float Total, const FText& Reason, float Multiplier = 1.0f);
};

// ============================================================================
// FACTORY CLASS
// ============================================================================

/**
 * @brief Factory for creating race type handlers
 *
 * This utility class provides static methods to create the appropriate handler
 * for a given race type enum value. It encapsulates the mapping between
 * EMGRaceType values and their corresponding handler classes.
 *
 * @section usage Usage Example
 * @code
 * // Create handler for drift race
 * UMGRaceTypeHandler* Handler = UMGRaceTypeFactory::CreateRaceTypeHandler(
 *     WorldContext, EMGRaceType::Drift);
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceTypeFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Create a new handler instance for the specified race type
	 * @param WorldContextObject Object providing world context (for spawning)
	 * @param RaceType The type of race to create a handler for
	 * @return Newly created handler instance, or nullptr if type is invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Type", meta = (WorldContext = "WorldContextObject"))
	static UMGRaceTypeHandler* CreateRaceTypeHandler(UObject* WorldContextObject, EMGRaceType RaceType);

	/**
	 * @brief Get the handler class for a race type without instantiating
	 * @param RaceType The race type to look up
	 * @return The UClass of the handler for this race type
	 */
	UFUNCTION(BlueprintPure, Category = "Race Type")
	static TSubclassOf<UMGRaceTypeHandler> GetHandlerClassForType(EMGRaceType RaceType);
};

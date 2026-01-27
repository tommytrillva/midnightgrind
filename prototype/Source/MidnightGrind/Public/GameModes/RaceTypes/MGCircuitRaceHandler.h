/**
 * @file MGCircuitRaceHandler.h
 * @brief Circuit Race Handler - Traditional lap-based closed-track racing
 *
 * Circuit racing is the most common race type in Midnight Grind. Racers compete
 * on a closed loop track, completing multiple laps with the goal of being the
 * first to cross the finish line after the final lap.
 *
 * @section overview Overview
 * Circuit races are the bread and butter of street racing games. The track forms
 * a complete loop, and racers must complete a set number of laps. Position is
 * determined by progress through the course (lap number + checkpoint progress).
 *
 * @section win_condition Win Condition
 * First racer to complete all laps wins. Other racers are ranked by their
 * finishing order or current progress when the race ends.
 *
 * @section scoring Scoring & Rewards
 * - Position-based credits (1st place gets most)
 * - Bonus for setting fastest lap
 * - XP scaled by finishing position
 * - Clean race bonus (no collisions)
 *
 * @section ui_elements UI Elements
 * - Lap counter (Lap X/Y)
 * - Position indicator (1st, 2nd, etc.)
 * - Best lap time display
 * - Gap to leader/next racer
 *
 * @section example Example Configuration
 * @code
 * UMGCircuitRaceHandler* Handler = Cast<UMGCircuitRaceHandler>(
 *     UMGRaceTypeFactory::CreateRaceTypeHandler(this, EMGRaceType::Circuit));
 * // Default is 3 laps, configured via race configuration
 * Handler->Initialize(GameMode);
 * @endcode
 *
 * @see UMGRaceTypeHandler Base class
 * @see UMGSprintRaceHandler For point-to-point alternative
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGCircuitRaceHandler.generated.h"

/**
 * @brief Handler for circuit (lap-based) racing
 *
 * Implements the rules and logic for traditional closed-track racing where
 * competitors complete multiple laps around a circuit. This is position-based
 * racing where the first to complete all laps wins.
 *
 * @section features Key Features
 * - Lap counting and validation
 * - Best lap tracking (individual and overall)
 * - Position calculation based on lap + checkpoint progress
 * - Final lap detection for UI/audio cues
 * - Finish order tracking for late finishers
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGCircuitRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGCircuitRaceHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE OVERRIDES
	// Implementation of base class virtual methods
	// ==========================================

	//~ Begin UMGRaceTypeHandler Interface

	/** Initialize handler and set up lap tracking structures */
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;

	/** Clear all lap times and finish order data */
	virtual void Reset() override;

	/** Begin timing the first lap */
	virtual void OnRaceStarted() override;

	/** Update any per-frame calculations (minimal for circuit) */
	virtual void OnRaceTick(float DeltaTime) override;

	/** Track sector times and validate checkpoint order */
	virtual void OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex) override;

	/** Record lap time and check for best lap / race completion */
	virtual void OnLapCompleted(int32 RacerIndex, float LapTime) override;

	/** Check if racer has completed all laps */
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;

	/** Calculate positions based on lap count and checkpoint progress */
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	/// Returns EMGRaceType::Circuit
	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::Circuit; }

	/** Get localized display name "Circuit Race" */
	virtual FText GetDisplayName() const override;

	/** Get description of circuit racing rules */
	virtual FText GetDescription() const override;

	/// Circuit races show lap counter
	virtual bool ShouldShowLapCounter() const override { return true; }

	/// Circuit races show position
	virtual bool ShouldShowPosition() const override { return true; }

	/// Circuit races are not score-based
	virtual bool ShouldShowScore() const override { return false; }

	/** Returns "Lap {0}/{1}" format string */
	virtual FText GetProgressFormat() const override;

	/** Calculate credits with bonuses for winning and fast laps */
	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// CIRCUIT-SPECIFIC METHODS
	// Public API for circuit race information
	// ==========================================

	/**
	 * @brief Get the total number of laps for this race
	 * @return Total laps to complete (e.g., 3)
	 */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	int32 GetTotalLaps() const { return TotalLaps; }

	/**
	 * @brief Get the current lap number for a specific racer
	 * @param RacerIndex Index of the racer to query
	 * @return Current lap (1-based, so lap 1 means first lap in progress)
	 */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	int32 GetRacerCurrentLap(int32 RacerIndex) const;

	/**
	 * @brief Get the fastest lap time set in this race
	 * @return Best lap time in seconds, or 0 if no laps completed yet
	 */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	float GetBestLapTime() const { return BestLapTime; }

	/**
	 * @brief Get the index of the racer who set the best lap
	 * @return Racer index, or -1 if no laps completed yet
	 */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	int32 GetBestLapRacerIndex() const { return BestLapRacerIndex; }

	/**
	 * @brief Get all recorded lap times for a racer
	 * @param RacerIndex Index of the racer to query
	 * @return Array of lap times in seconds (index 0 = lap 1)
	 */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	TArray<float> GetRacerLapTimes(int32 RacerIndex) const;

	/**
	 * @brief Check if a racer is on their final lap
	 * @param RacerIndex Index of the racer to check
	 * @return True if on final lap (for "FINAL LAP" UI/audio)
	 */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	bool IsOnFinalLap(int32 RacerIndex) const;

protected:
	// ==========================================
	// RACE CONFIGURATION
	// Settings that define the race parameters
	// ==========================================

	/** Total number of laps in this race (typically 3-5) */
	UPROPERTY()
	int32 TotalLaps = 3;

	// ==========================================
	// BEST LAP TRACKING
	// Track the fastest lap for leaderboard and bonuses
	// ==========================================

	/** Fastest lap time recorded in this race (seconds) */
	UPROPERTY()
	float BestLapTime = 0.0f;

	/** Index of the racer who set the best lap (-1 if none) */
	UPROPERTY()
	int32 BestLapRacerIndex = -1;

	// ==========================================
	// PER-RACER STATE
	// Individual tracking for each competitor
	// ==========================================

	/**
	 * Lap times for each racer
	 * Key: Racer index
	 * Value: Array of lap times (index 0 = lap 1 time)
	 */
	UPROPERTY()
	TMap<int32, TArray<float>> RacerLapTimes;

	/**
	 * Set of racers who have crossed the start/finish line
	 * Used to properly count the first lap (must cross start before lap 1 counts)
	 */
	UPROPERTY()
	TSet<int32> HasCrossedStart;

	// ==========================================
	// FINISH ORDER TRACKING
	// Track order of racers crossing finish line
	// ==========================================

	/** Number of racers who have finished (for position assignment) */
	int32 FinishOrder = 0;

	/**
	 * Array of racer indices in the order they finished
	 * Index 0 = 1st place finisher, etc.
	 */
	UPROPERTY()
	TArray<int32> FinishPositions;
};

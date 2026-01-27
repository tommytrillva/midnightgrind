/**
 * @file MGSprintRaceHandler.h
 * @brief Sprint Race Handler - Point-to-point street racing
 *
 * Sprint races are linear, point-to-point races from a start line to a finish
 * line. Unlike circuit races, there are no laps - just a single run through
 * the course. This is common for street racing scenarios where the route goes
 * through different areas of the city.
 *
 * @section overview Overview
 * Sprint races capture the essence of illegal street racing - a high-stakes
 * blast from point A to point B. The course may wind through city streets,
 * highways, and backroads, with sectors dividing the route for split timing.
 *
 * @section win_condition Win Condition
 * First racer to cross the finish line wins. Unlike circuit races, there's
 * no concept of laps - just pure start-to-finish racing.
 *
 * @section scoring Scoring & Rewards
 * - Position-based credits (similar to circuit)
 * - Sector time bonuses (best sector = extra credits)
 * - Time-based bonuses for beating target time
 *
 * @section ui_elements UI Elements
 * - Distance remaining (instead of lap counter)
 * - Progress percentage bar
 * - Sector split times with deltas
 * - Position indicator
 *
 * @section tracks Track Design
 * Sprint tracks use checkpoints both for validation and sector timing:
 * - Checkpoints ensure racers follow the route
 * - Each checkpoint marks a sector boundary
 * - Final checkpoint is the finish line
 *
 * @see UMGRaceTypeHandler Base class
 * @see UMGCircuitRaceHandler For lap-based alternative
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGSprintRaceHandler.generated.h"

/**
 * @brief Handler for sprint (point-to-point) racing
 *
 * Implements rules for linear start-to-finish racing without laps.
 * Position is tracked by progress along the course, measured by
 * checkpoint passage and interpolated distance.
 *
 * @section features Key Features
 * - Distance-based progress tracking
 * - Sector timing with split comparisons
 * - Best sector tracking (per sector)
 * - Progress percentage for UI display
 * - Single-run format (no laps)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGSprintRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGSprintRaceHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE OVERRIDES
	// Implementation of base class virtual methods
	// ==========================================

	//~ Begin UMGRaceTypeHandler Interface

	/** Initialize with track distance and sector information */
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;

	/** Clear all progress and sector time data */
	virtual void Reset() override;

	/** Begin race timing */
	virtual void OnRaceStarted() override;

	/** Update distance tracking for all racers */
	virtual void OnRaceTick(float DeltaTime) override;

	/** Record sector time and update progress */
	virtual void OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex) override;

	/** Check if racer has reached the finish line */
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;

	/** Calculate positions based on distance traveled */
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	/// Returns EMGRaceType::Sprint
	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::Sprint; }

	/** Get localized display name "Sprint Race" */
	virtual FText GetDisplayName() const override;

	/** Get description of sprint racing rules */
	virtual FText GetDescription() const override;

	/// Sprint races do NOT show lap counter (no laps)
	virtual bool ShouldShowLapCounter() const override { return false; }

	/// Sprint races show position
	virtual bool ShouldShowPosition() const override { return true; }

	/// Sprint races are not score-based
	virtual bool ShouldShowScore() const override { return false; }

	/** Returns "{0}m to go" format string */
	virtual FText GetProgressFormat() const override;

	/** Calculate credits with sector bonuses */
	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// SPRINT-SPECIFIC METHODS - DISTANCE
	// Public API for distance and progress queries
	// ==========================================

	/**
	 * @brief Get the total track distance
	 * @return Total distance in centimeters (Unreal units)
	 */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetTotalDistance() const { return TotalDistance; }

	/**
	 * @brief Get the remaining distance for a racer
	 * @param RacerIndex Index of the racer to query
	 * @return Distance remaining to finish in centimeters
	 */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetDistanceRemaining(int32 RacerIndex) const;

	/**
	 * @brief Get progress as a percentage
	 * @param RacerIndex Index of the racer to query
	 * @return Progress from 0.0 (start) to 100.0 (finish)
	 */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetProgressPercentage(int32 RacerIndex) const;

	// ==========================================
	// SPRINT-SPECIFIC METHODS - SECTORS
	// Public API for sector timing information
	// ==========================================

	/**
	 * @brief Get the current sector a racer is in
	 * @param RacerIndex Index of the racer to query
	 * @return Current sector index (0-based)
	 */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	int32 GetCurrentSector(int32 RacerIndex) const;

	/**
	 * @brief Get the total number of sectors in the race
	 * @return Number of sectors (equals number of checkpoints)
	 */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	int32 GetTotalSectors() const { return TotalSectors; }

	/**
	 * @brief Get a racer's time for a specific sector
	 * @param RacerIndex Index of the racer
	 * @param SectorIndex Index of the sector (0-based)
	 * @return Sector time in seconds, or 0 if not yet completed
	 */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetSectorTime(int32 RacerIndex, int32 SectorIndex) const;

	/**
	 * @brief Get the best time recorded for a sector
	 * @param SectorIndex Index of the sector (0-based)
	 * @return Best sector time in seconds across all racers
	 */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetBestSectorTime(int32 SectorIndex) const;

protected:
	// ==========================================
	// TRACK CONFIGURATION
	// Settings derived from track data
	// ==========================================

	/**
	 * Total track distance in centimeters
	 * Calculated from checkpoint positions during initialization
	 */
	UPROPERTY()
	float TotalDistance = 0.0f;

	/**
	 * Total number of sectors (checkpoints)
	 * Each checkpoint boundary defines a sector
	 */
	UPROPERTY()
	int32 TotalSectors = 0;

	// ==========================================
	// PER-RACER PROGRESS TRACKING
	// Individual state for each competitor
	// ==========================================

	/**
	 * Distance traveled by each racer
	 * Key: Racer index
	 * Value: Distance in centimeters from start
	 */
	UPROPERTY()
	TMap<int32, float> RacerProgress;

	/**
	 * Sector times for each racer
	 * Key: Racer index
	 * Value: Array of sector times (index = sector number)
	 */
	UPROPERTY()
	TMap<int32, TArray<float>> RacerSectorTimes;

	// ==========================================
	// BEST SECTOR TRACKING
	// Track records for each sector
	// ==========================================

	/** Best time for each sector (index = sector number) */
	UPROPERTY()
	TArray<float> BestSectorTimes;

	/** Racer index who set each best sector time */
	UPROPERTY()
	TArray<int32> BestSectorRacers;

	// ==========================================
	// FINISH TRACKING
	// Track completion order
	// ==========================================

	/** Number of racers who have finished (for position assignment) */
	int32 FinishOrder = 0;

	/**
	 * Index of the final checkpoint (finish line)
	 * Passing this checkpoint completes the race
	 */
	UPROPERTY()
	int32 FinishCheckpointIndex = 0;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/**
	 * @brief Update a racer's progress based on checkpoint passage
	 * @param RacerIndex Racer who passed the checkpoint
	 * @param CheckpointIndex Checkpoint that was passed
	 */
	void UpdateRacerProgress(int32 RacerIndex, int32 CheckpointIndex);
};

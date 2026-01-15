// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGCircuitRaceHandler.generated.h"

/**
 * Circuit Race Handler
 * Standard lap-based racing around a closed track
 *
 * Win Condition: First to complete all laps
 * Scoring: Position-based + lap time bonuses
 * Features:
 * - Lap counting and timing
 * - Best lap tracking
 * - Position-based rewards
 * - Optional time limit
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGCircuitRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGCircuitRaceHandler();

	//~ Begin UMGRaceTypeHandler Interface
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;
	virtual void Reset() override;
	virtual void OnRaceStarted() override;
	virtual void OnRaceTick(float DeltaTime) override;
	virtual void OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex) override;
	virtual void OnLapCompleted(int32 RacerIndex, float LapTime) override;
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::Circuit; }
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual bool ShouldShowLapCounter() const override { return true; }
	virtual bool ShouldShowPosition() const override { return true; }
	virtual bool ShouldShowScore() const override { return false; }
	virtual FText GetProgressFormat() const override;

	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// CIRCUIT-SPECIFIC
	// ==========================================

	/** Get total laps for the race */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	int32 GetTotalLaps() const { return TotalLaps; }

	/** Get current lap for racer */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	int32 GetRacerCurrentLap(int32 RacerIndex) const;

	/** Get best lap time overall */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	float GetBestLapTime() const { return BestLapTime; }

	/** Get who set the best lap */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	int32 GetBestLapRacerIndex() const { return BestLapRacerIndex; }

	/** Get lap times for a racer */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	TArray<float> GetRacerLapTimes(int32 RacerIndex) const;

	/** Is the final lap? */
	UFUNCTION(BlueprintPure, Category = "Circuit Race")
	bool IsOnFinalLap(int32 RacerIndex) const;

protected:
	/** Total laps in race */
	UPROPERTY()
	int32 TotalLaps = 3;

	/** Best lap time in race */
	UPROPERTY()
	float BestLapTime = 0.0f;

	/** Racer who set best lap */
	UPROPERTY()
	int32 BestLapRacerIndex = -1;

	/** Per-racer lap times */
	UPROPERTY()
	TMap<int32, TArray<float>> RacerLapTimes;

	/** Track if racer has crossed start line (to count first lap correctly) */
	UPROPERTY()
	TSet<int32> HasCrossedStart;

	/** Number of finished racers (for position calculation) */
	int32 FinishOrder = 0;

	/** Finish positions (in order of finish) */
	UPROPERTY()
	TArray<int32> FinishPositions;
};

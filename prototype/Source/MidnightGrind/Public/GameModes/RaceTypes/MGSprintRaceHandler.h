// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGSprintRaceHandler.generated.h"

/**
 * Sprint Race Handler
 * Point-to-point racing from start to finish line
 *
 * Win Condition: First to reach the finish line
 * Scoring: Position-based + time bonuses
 * Features:
 * - Single run from point A to B
 * - Sector timing with split comparisons
 * - Distance-to-finish tracking
 * - No lap counter, shows distance remaining
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGSprintRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGSprintRaceHandler();

	//~ Begin UMGRaceTypeHandler Interface
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;
	virtual void Reset() override;
	virtual void OnRaceStarted() override;
	virtual void OnRaceTick(float DeltaTime) override;
	virtual void OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex) override;
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::Sprint; }
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual bool ShouldShowLapCounter() const override { return false; }
	virtual bool ShouldShowPosition() const override { return true; }
	virtual bool ShouldShowScore() const override { return false; }
	virtual FText GetProgressFormat() const override;

	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// SPRINT-SPECIFIC
	// ==========================================

	/** Get total track distance */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetTotalDistance() const { return TotalDistance; }

	/** Get distance remaining for racer */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetDistanceRemaining(int32 RacerIndex) const;

	/** Get progress percentage (0-100) for racer */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetProgressPercentage(int32 RacerIndex) const;

	/** Get current sector for racer */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	int32 GetCurrentSector(int32 RacerIndex) const;

	/** Get total sectors */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	int32 GetTotalSectors() const { return TotalSectors; }

	/** Get sector time for racer */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetSectorTime(int32 RacerIndex, int32 SectorIndex) const;

	/** Get best sector time overall */
	UFUNCTION(BlueprintPure, Category = "Sprint Race")
	float GetBestSectorTime(int32 SectorIndex) const;

protected:
	/** Total track distance in cm */
	UPROPERTY()
	float TotalDistance = 0.0f;

	/** Total number of sectors (checkpoints) */
	UPROPERTY()
	int32 TotalSectors = 0;

	/** Per-racer current progress (distance traveled) */
	UPROPERTY()
	TMap<int32, float> RacerProgress;

	/** Per-racer sector times [RacerIndex][SectorIndex] */
	UPROPERTY()
	TMap<int32, TArray<float>> RacerSectorTimes;

	/** Best sector times */
	UPROPERTY()
	TArray<float> BestSectorTimes;

	/** Who set each best sector */
	UPROPERTY()
	TArray<int32> BestSectorRacers;

	/** Finish order tracking */
	int32 FinishOrder = 0;

	/** Index of finish checkpoint */
	UPROPERTY()
	int32 FinishCheckpointIndex = 0;

	/** Update racer progress based on checkpoint */
	void UpdateRacerProgress(int32 RacerIndex, int32 CheckpointIndex);
};

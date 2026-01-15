// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameModes/MGRaceGameMode.h"
#include "MGRaceTypeHandler.generated.h"

class AMGRaceGameMode;
class AMGVehiclePawn;
class AMGCheckpoint;

/**
 * Race completion condition result
 */
UENUM(BlueprintType)
enum class EMGRaceCompletionResult : uint8
{
	/** Race still in progress */
	InProgress,
	/** Racer has finished successfully */
	Finished,
	/** Racer did not finish (DNF) */
	DNF,
	/** Racer disqualified */
	Disqualified
};

/**
 * Scoring update for race types that use scores
 */
USTRUCT(BlueprintType)
struct FMGScoreUpdate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 RacerIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	float ScoreDelta = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float TotalScore = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	FText ScoreReason;

	UPROPERTY(BlueprintReadOnly)
	float Multiplier = 1.0f;
};

/** Delegate for score updates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated, const FMGScoreUpdate&, ScoreUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSectorComplete, int32, RacerIndex, int32, SectorIndex, float, SectorTime);

/**
 * Base class for race type handlers
 * Each race type (Circuit, Sprint, Drift, etc.) extends this to provide specific logic
 *
 * Handlers are responsible for:
 * - Defining win conditions
 * - Scoring/ranking logic
 * - Race-specific rules and mechanics
 * - UI data for the race type
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGRaceTypeHandler : public UObject
{
	GENERATED_BODY()

public:
	UMGRaceTypeHandler();

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Initialize the handler with the owning game mode */
	UFUNCTION(BlueprintCallable, Category = "Race Type")
	virtual void Initialize(AMGRaceGameMode* InGameMode);

	/** Called when race type is activated */
	UFUNCTION(BlueprintCallable, Category = "Race Type")
	virtual void Activate();

	/** Called when race type is deactivated */
	UFUNCTION(BlueprintCallable, Category = "Race Type")
	virtual void Deactivate();

	/** Reset handler state for new race */
	UFUNCTION(BlueprintCallable, Category = "Race Type")
	virtual void Reset();

	// ==========================================
	// RACE FLOW
	// ==========================================

	/** Called when race countdown begins */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnCountdownStarted();

	/** Called when race starts (GO!) */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRaceStarted();

	/** Called each tick during racing */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRaceTick(float DeltaTime);

	/** Called when race is paused */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRacePaused();

	/** Called when race is resumed */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRaceResumed();

	/** Called when race ends */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Flow")
	virtual void OnRaceEnded();

	// ==========================================
	// CHECKPOINT/PROGRESS HANDLING
	// ==========================================

	/** Called when any racer passes a checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Progress")
	virtual void OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex);

	/** Called when any racer completes a lap */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Progress")
	virtual void OnLapCompleted(int32 RacerIndex, float LapTime);

	/** Check if a racer has met completion conditions */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Progress")
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex);

	// ==========================================
	// SCORING & RANKING
	// ==========================================

	/** Calculate position/ranking for all racers */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Scoring")
	virtual void CalculatePositions(TArray<int32>& OutPositions);

	/** Get score for a racer (drift/time trial modes) */
	UFUNCTION(BlueprintPure, Category = "Race Type|Scoring")
	virtual float GetRacerScore(int32 RacerIndex) const;

	/** Get total score target (if applicable) */
	UFUNCTION(BlueprintPure, Category = "Race Type|Scoring")
	virtual float GetTargetScore() const { return 0.0f; }

	/** Is this a score-based race type? */
	UFUNCTION(BlueprintPure, Category = "Race Type|Scoring")
	virtual bool IsScoreBased() const { return false; }

	// ==========================================
	// RACE TYPE INFO
	// ==========================================

	/** Get the race type this handler manages */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual EMGRaceType GetRaceType() const PURE_VIRTUAL(UMGRaceTypeHandler::GetRaceType, return EMGRaceType::Circuit;);

	/** Get display name for the race type */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual FText GetDisplayName() const PURE_VIRTUAL(UMGRaceTypeHandler::GetDisplayName, return FText::GetEmpty(););

	/** Get description of the race type */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual FText GetDescription() const { return FText::GetEmpty(); }

	/** Get icon for the race type */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual UTexture2D* GetIcon() const { return nullptr; }

	/** Should show lap counter? */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual bool ShouldShowLapCounter() const { return true; }

	/** Should show position? */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual bool ShouldShowPosition() const { return true; }

	/** Should show score? */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual bool ShouldShowScore() const { return false; }

	/** Get progress format string (e.g., "Lap {0}/{1}" or "{0}m to go") */
	UFUNCTION(BlueprintPure, Category = "Race Type|Info")
	virtual FText GetProgressFormat() const;

	// ==========================================
	// CREDITS & REWARDS
	// ==========================================

	/** Calculate credits earned for position */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Rewards")
	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const;

	/** Calculate XP earned for position */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Rewards")
	virtual int32 CalculateXPForPosition(int32 Position, int32 TotalRacers) const;

	/** Calculate reputation earned */
	UFUNCTION(BlueprintCallable, Category = "Race Type|Rewards")
	virtual int32 CalculateReputationEarned(int32 Position, bool bWon) const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when score changes (drift/time trial) */
	UPROPERTY(BlueprintAssignable, Category = "Race Type|Events")
	FOnScoreUpdated OnScoreUpdated;

	/** Called when a sector is completed (all race types) */
	UPROPERTY(BlueprintAssignable, Category = "Race Type|Events")
	FOnSectorComplete OnSectorComplete;

protected:
	/** Owning game mode */
	UPROPERTY()
	TWeakObjectPtr<AMGRaceGameMode> GameMode;

	/** Is handler active? */
	UPROPERTY()
	bool bIsActive = false;

	/** Per-racer scores (for score-based modes) */
	UPROPERTY()
	TMap<int32, float> RacerScores;

	/** Get game mode (with null check) */
	AMGRaceGameMode* GetGameMode() const;

	/** Broadcast score update */
	void BroadcastScoreUpdate(int32 RacerIndex, float Delta, float Total, const FText& Reason, float Multiplier = 1.0f);
};

/**
 * Factory for creating race type handlers
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceTypeFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Create a handler for the specified race type */
	UFUNCTION(BlueprintCallable, Category = "Race Type", meta = (WorldContext = "WorldContextObject"))
	static UMGRaceTypeHandler* CreateRaceTypeHandler(UObject* WorldContextObject, EMGRaceType RaceType);

	/** Get handler class for race type */
	UFUNCTION(BlueprintPure, Category = "Race Type")
	static TSubclassOf<UMGRaceTypeHandler> GetHandlerClassForType(EMGRaceType RaceType);
};

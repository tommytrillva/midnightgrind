// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGDriftRaceHandler.generated.h"

class AMGVehiclePawn;

/**
 * Drift state for tracking active drifts
 */
UENUM(BlueprintType)
enum class EMGDriftState : uint8
{
	/** Not drifting */
	None,
	/** Building drift (just started) */
	Building,
	/** Sustained drift */
	Sustained,
	/** Drift ending (grace period) */
	Ending
};

/**
 * Drift grade based on angle and speed
 */
UENUM(BlueprintType)
enum class EMGDriftGrade : uint8
{
	/** Below threshold - no points */
	None		UMETA(DisplayName = "None"),
	/** Basic drift 15-30 degrees */
	D			UMETA(DisplayName = "D"),
	/** Moderate drift 30-45 degrees */
	C			UMETA(DisplayName = "C"),
	/** Good drift 45-60 degrees */
	B			UMETA(DisplayName = "B"),
	/** Great drift 60-75 degrees */
	A			UMETA(DisplayName = "A"),
	/** Maximum drift 75+ degrees */
	S			UMETA(DisplayName = "S"),
	/** Perfect angle + high speed */
	SS			UMETA(DisplayName = "SS")
};

/**
 * Active drift tracking data
 */
USTRUCT(BlueprintType)
struct FMGActiveDrift
{
	GENERATED_BODY()

	/** Current drift state */
	UPROPERTY(BlueprintReadOnly)
	EMGDriftState State = EMGDriftState::None;

	/** Current drift grade */
	UPROPERTY(BlueprintReadOnly)
	EMGDriftGrade Grade = EMGDriftGrade::None;

	/** Current drift angle in degrees */
	UPROPERTY(BlueprintReadOnly)
	float DriftAngle = 0.0f;

	/** Current speed during drift */
	UPROPERTY(BlueprintReadOnly)
	float Speed = 0.0f;

	/** Duration of current drift */
	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.0f;

	/** Points accumulated in this drift */
	UPROPERTY(BlueprintReadOnly)
	float Points = 0.0f;

	/** Current multiplier from chain */
	UPROPERTY(BlueprintReadOnly)
	float Multiplier = 1.0f;

	/** Time since last drift (for chain) */
	UPROPERTY()
	float TimeSinceLastDrift = 0.0f;

	/** Chain count */
	UPROPERTY(BlueprintReadOnly)
	int32 ChainCount = 0;

	/** Is this a tandem drift (near another drifter)? */
	UPROPERTY(BlueprintReadOnly)
	bool bIsTandem = false;

	/** Reset drift state */
	void Reset()
	{
		State = EMGDriftState::None;
		Grade = EMGDriftGrade::None;
		DriftAngle = 0.0f;
		Speed = 0.0f;
		Duration = 0.0f;
		Points = 0.0f;
		// Multiplier and chain persist for chaining
	}
};

/**
 * Completed drift data for scoring
 */
USTRUCT(BlueprintType)
struct FMGCompletedDrift
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 RacerIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	float TotalPoints = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	EMGDriftGrade PeakGrade = EMGDriftGrade::None;

	UPROPERTY(BlueprintReadOnly)
	float Multiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 ChainCount = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bWasTandem = false;
};

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftStarted, int32, RacerIndex, EMGDriftGrade, Grade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftGradeChanged, int32, RacerIndex, EMGDriftGrade, NewGrade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftEnded, int32, RacerIndex, const FMGCompletedDrift&, DriftData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChainIncreased, int32, RacerIndex, int32, NewChainCount);

/**
 * Drift Race Handler
 * Score-based drifting competition
 *
 * Win Condition: Highest drift score wins
 * Scoring: Points for drift angle, duration, speed, chains, tandems
 * Features:
 * - Real-time drift detection and grading
 * - Chain multiplier system
 * - Tandem drift bonuses
 * - Drift zones for bonus points
 * - Can be lap-based or target-score based
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGDriftRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGDriftRaceHandler();

	//~ Begin UMGRaceTypeHandler Interface
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;
	virtual void Reset() override;
	virtual void OnRaceStarted() override;
	virtual void OnRaceTick(float DeltaTime) override;
	virtual void OnLapCompleted(int32 RacerIndex, float LapTime) override;
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::Drift; }
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual bool ShouldShowLapCounter() const override { return bIsLapBased; }
	virtual bool ShouldShowPosition() const override { return true; }
	virtual bool ShouldShowScore() const override { return true; }
	virtual bool IsScoreBased() const override { return true; }
	virtual float GetRacerScore(int32 RacerIndex) const override;
	virtual float GetTargetScore() const override { return TargetScore; }
	virtual FText GetProgressFormat() const override;

	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// DRIFT DETECTION
	// ==========================================

	/** Process drift state for a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Drift Race")
	void ProcessVehicleDrift(int32 RacerIndex, AMGVehiclePawn* Vehicle, float DeltaTime);

	/** Get current drift state for racer */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	FMGActiveDrift GetActiveDrift(int32 RacerIndex) const;

	/** Is racer currently drifting? */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	bool IsDrifting(int32 RacerIndex) const;

	// ==========================================
	// SCORING
	// ==========================================

	/** Get total score for racer */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	float GetTotalScore(int32 RacerIndex) const;

	/** Get current multiplier for racer */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	float GetCurrentMultiplier(int32 RacerIndex) const;

	/** Get current chain count for racer */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	int32 GetChainCount(int32 RacerIndex) const;

	/** Get best single drift score for racer */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	float GetBestDriftScore(int32 RacerIndex) const;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set target score (for score-based mode) */
	UFUNCTION(BlueprintCallable, Category = "Drift Race|Config")
	void SetTargetScore(float Score) { TargetScore = Score; bIsLapBased = false; }

	/** Set lap-based mode */
	UFUNCTION(BlueprintCallable, Category = "Drift Race|Config")
	void SetLapBased(int32 Laps) { TotalLaps = Laps; bIsLapBased = true; }

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Drift Race|Events")
	FOnDriftStarted OnDriftStarted;

	UPROPERTY(BlueprintAssignable, Category = "Drift Race|Events")
	FOnDriftGradeChanged OnDriftGradeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Drift Race|Events")
	FOnDriftEnded OnDriftEnded;

	UPROPERTY(BlueprintAssignable, Category = "Drift Race|Events")
	FOnChainIncreased OnChainIncreased;

protected:
	// ==========================================
	// DRIFT DETECTION CONFIG
	// ==========================================

	/** Minimum angle to count as drifting (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float MinDriftAngle = 15.0f;

	/** Minimum speed to count as drifting (cm/s) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float MinDriftSpeed = 1000.0f;

	/** Grace period when drift ends before score is finalized */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float DriftEndGracePeriod = 0.5f;

	/** Time window to chain drifts */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float ChainWindowSeconds = 2.0f;

	/** Distance for tandem drift detection */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float TandemDistance = 1500.0f;

	// ==========================================
	// SCORING CONFIG
	// ==========================================

	/** Base points per second of drifting */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float BasePointsPerSecond = 100.0f;

	/** Angle multiplier (points *= 1 + angle/90) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float AngleScoreMultiplier = 1.0f;

	/** Speed multiplier (points *= 1 + speed/maxSpeed) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float SpeedScoreMultiplier = 0.5f;

	/** Chain multiplier increment per chain */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float ChainMultiplierIncrement = 0.25f;

	/** Maximum chain multiplier */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float MaxChainMultiplier = 5.0f;

	/** Tandem bonus multiplier */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float TandemBonusMultiplier = 1.5f;

	// ==========================================
	// STATE
	// ==========================================

	/** Per-racer active drift tracking */
	UPROPERTY()
	TMap<int32, FMGActiveDrift> ActiveDrifts;

	/** Per-racer total scores */
	UPROPERTY()
	TMap<int32, float> TotalScores;

	/** Per-racer best single drift */
	UPROPERTY()
	TMap<int32, float> BestDriftScores;

	/** Is lap-based or target score based? */
	UPROPERTY()
	bool bIsLapBased = true;

	/** Total laps (if lap-based) */
	UPROPERTY()
	int32 TotalLaps = 3;

	/** Target score (if score-based) */
	UPROPERTY()
	float TargetScore = 50000.0f;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Calculate drift grade from angle and speed */
	EMGDriftGrade CalculateDriftGrade(float Angle, float Speed) const;

	/** Calculate points for current drift state */
	float CalculateDriftPoints(const FMGActiveDrift& Drift, float DeltaTime) const;

	/** Check for nearby drifters (tandem) */
	bool CheckTandemDrift(int32 RacerIndex) const;

	/** Finalize a completed drift */
	void FinalizeDrift(int32 RacerIndex);

	/** Get drift angle from vehicle */
	float GetVehicleDriftAngle(AMGVehiclePawn* Vehicle) const;
};

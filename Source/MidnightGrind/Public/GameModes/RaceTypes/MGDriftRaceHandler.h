#pragma once


/**
 * @file MGDriftRaceHandler.h
 * @brief Drift Race Handler - Score-based drifting competition
 *
 * Drift racing focuses on style over speed. Racers earn points by executing
 * controlled drifts, with scoring based on drift angle, speed, duration, and
 * chaining multiple drifts together. This mode captures the essence of
 * Japanese drift culture and touge mountain racing.
 *
 * @section overview Overview
 * Unlike traditional racing where position matters, drift mode is all about
 * accumulating the highest score. The handler monitors each vehicle's slip
 * angle and speed to detect drifts, grade their quality, and award points.
 *
 * @section win_condition Win Conditions
 * Two modes are supported:
 * - **Lap-based**: Complete X laps, highest score wins
 * - **Target score**: First to reach target score wins
 *
 * @section scoring Scoring System
 * Points are calculated based on multiple factors:
 * - **Drift Angle**: Higher angles = more points (15-90 degrees)
 * - **Speed**: Faster drifts score more
 * - **Duration**: Sustained drifts accumulate continuously
 * - **Chains**: Linking drifts multiplies score (up to 5x)
 * - **Tandem**: Drifting near another car adds 1.5x bonus
 *
 * @section grades Drift Grades
 * Drifts are graded D through SS based on angle and speed:
 * - D: Basic drift (15-30 degrees)
 * - C: Moderate drift (30-45 degrees)
 * - B: Good drift (45-60 degrees)
 * - A: Great drift (60-75 degrees)
 * - S: Maximum angle (75+ degrees)
 * - SS: Perfect angle + high speed
 *
 * @section ui_elements UI Elements
 * - Current score with recent point popup
 * - Drift grade indicator (D/C/B/A/S/SS)
 * - Chain multiplier display
 * - Angle meter
 * - Position by score
 *
 * @see UMGRaceTypeHandler Base class
 * @see FMGActiveDrift Active drift tracking data
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

// Copyright Midnight Grind. All Rights Reserved.

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "Drift/MGDriftSubsystem.h"  // For FMGActiveDrift, EMGDriftGrade
#include "MGDriftRaceHandler.generated.h"

/// Forward declaration for vehicle access
class AMGVehiclePawn;

// ============================================================================
// DRIFT STATE ENUMS
// ============================================================================

/**
 * @brief Current state of an active drift
 *
 * Drifts go through a lifecycle: None -> Building -> Sustained -> Ending -> None
 * The Ending state provides a grace period to chain into another drift.
 */
UENUM(BlueprintType)
enum class EMGDriftState : uint8
{
	/** Vehicle is not drifting (driving normally) */
	None,
	/** Drift just started, building up (first 0.2 seconds) */
	Building,
	/** Drift is sustained and scoring points */
	Sustained,
	/** Drift is ending but still in grace period for chaining */
	Ending
};

// EMGDriftGrade - REMOVED (duplicate)
// Canonical definition in: Drift/MGDriftSubsystem.h

// FMGActiveDrift - REMOVED (duplicate)
// Canonical definition in: Drift/MGDriftSubsystem.h
// Include added at top of file

// ============================================================================
// DRIFT DATA STRUCTS
// ============================================================================

/**
 * @brief Data for a completed drift (used for scoring and history)
 *
 * When a drift ends (grace period expires), this struct captures
 * the final statistics for scoring and display.
 */
USTRUCT(BlueprintType)
struct FMGCompletedDrift
{
	GENERATED_BODY()

	/// Index of the racer who performed the drift
	UPROPERTY(BlueprintReadOnly)
	int32 RacerIndex = -1;

	/// Total points earned from this drift (after multipliers)
	UPROPERTY(BlueprintReadOnly)
	float TotalPoints = 0.0f;

	/// Total duration of the drift in seconds
	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.0f;

	/// Highest grade achieved during the drift
	UPROPERTY(BlueprintReadOnly)
	EMGDriftGrade PeakGrade = EMGDriftGrade::None;

	/// Multiplier that was applied (from chain)
	UPROPERTY(BlueprintReadOnly)
	float Multiplier = 1.0f;

	/// Chain count at time of completion
	UPROPERTY(BlueprintReadOnly)
	int32 ChainCount = 0;

	/// Whether this was a tandem drift
	UPROPERTY(BlueprintReadOnly)
	bool bWasTandem = false;
};

// ============================================================================
// DRIFT EVENT DELEGATES
// ============================================================================

/** Broadcast when a racer begins a new drift */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftStarted, int32, RacerIndex, EMGDriftGrade, Grade);

/** Broadcast when drift grade changes (e.g., C -> B) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftGradeChanged, int32, RacerIndex, EMGDriftGrade, NewGrade);

/** Broadcast when a drift ends with final scoring data */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftEnded, int32, RacerIndex, const FMGCompletedDrift&, DriftData);

/** Broadcast when chain count increases */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChainIncreased, int32, RacerIndex, int32, NewChainCount);

// ============================================================================
// DRIFT RACE HANDLER CLASS
// ============================================================================

/**
 * @brief Handler for drift (score-based) racing
 *
 * Monitors vehicle physics to detect drifts and awards points based on
 * angle, speed, duration, and combo chains. Supports both lap-based and
 * target-score victory conditions.
 *
 * @section detection Drift Detection
 * A drift is detected when:
 * - Slip angle exceeds MinDriftAngle (default 15 degrees)
 * - Speed exceeds MinDriftSpeed (default 1000 cm/s)
 *
 * @section chaining Drift Chaining
 * Consecutive drifts within ChainWindowSeconds (default 2s) increase
 * the multiplier by ChainMultiplierIncrement (default 0.25) up to
 * MaxChainMultiplier (default 5.0x).
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGDriftRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGDriftRaceHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE OVERRIDES
	// ==========================================

	//~ Begin UMGRaceTypeHandler Interface

	/** Set up drift tracking for all racers */
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;

	/** Clear all scores and drift states */
	virtual void Reset() override;

	/** Begin accepting drift scoring */
	virtual void OnRaceStarted() override;

	/** Process drift detection for all vehicles */
	virtual void OnRaceTick(float MGDeltaTime) override;

	/** Check lap completion for lap-based mode */
	virtual void OnLapCompleted(int32 RacerIndex, float LapTime) override;

	/** Check for win condition (laps or target score) */
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;

	/** Rank racers by score (highest first) */
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	/// Returns EMGRaceType::Drift
	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::Drift; }

	/** Get localized display name "Drift Battle" */
	virtual FText GetDisplayName() const override;

	/** Get description of drift racing rules */
	virtual FText GetDescription() const override;

	/// Show lap counter only in lap-based mode
	virtual bool ShouldShowLapCounter() const override { return bIsLapBased; }

	/// Drift races show position (by score)
	virtual bool ShouldShowPosition() const override { return true; }

	/// Drift races always show score
	virtual bool ShouldShowScore() const override { return true; }

	/// Drift is a score-based mode
	virtual bool IsScoreBased() const override { return true; }

	/** Get racer's current total score */
	virtual float GetRacerScore(int32 RacerIndex) const override;

	/** Get target score (if in score-based mode) */
	virtual float GetTargetScore() const override { return TargetScore; }

	/** Returns score format string */
	virtual FText GetProgressFormat() const override;

	/** Calculate credits based on score ranking */
	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// DRIFT DETECTION
	// Methods for processing vehicle drift state
	// ==========================================

	/**
	 * @brief Process drift state for a vehicle (called each tick)
	 * @param RacerIndex Index of the racer
	 * @param Vehicle The vehicle pawn to analyze
	 * @param DeltaTime Frame time for calculations
	 * @note This is the core drift detection logic
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift Race")
	void ProcessVehicleDrift(int32 RacerIndex, AMGVehiclePawn* Vehicle, float MGDeltaTime);

	/**
	 * @brief Get the current drift state for a racer
	 * @param RacerIndex Index of the racer
	 * @return Active drift data (check State for whether drifting)
	 */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	FMGActiveDrift GetActiveDrift(int32 RacerIndex) const;

	/**
	 * @brief Check if a racer is currently drifting
	 * @param RacerIndex Index of the racer
	 * @return True if in Building, Sustained, or Ending state
	 */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	bool IsDrifting(int32 RacerIndex) const;

	// ==========================================
	// SCORING
	// Methods for querying score information
	// ==========================================

	/**
	 * @brief Get the total score for a racer
	 * @param RacerIndex Index of the racer
	 * @return Total accumulated points
	 */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	float GetTotalScore(int32 RacerIndex) const;

	/**
	 * @brief Get the current chain multiplier for a racer
	 * @param RacerIndex Index of the racer
	 * @return Current multiplier (1.0 to MaxChainMultiplier)
	 */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	float GetCurrentMultiplier(int32 RacerIndex) const;

	/**
	 * @brief Get the current chain count for a racer
	 * @param RacerIndex Index of the racer
	 * @return Number of drifts in current chain
	 */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	int32 GetChainCount(int32 RacerIndex) const;

	/**
	 * @brief Get the highest single drift score for a racer
	 * @param RacerIndex Index of the racer
	 * @return Best individual drift score
	 */
	UFUNCTION(BlueprintPure, Category = "Drift Race")
	float GetBestDriftScore(int32 RacerIndex) const;

	// ==========================================
	// CONFIGURATION
	// Methods to set up the race mode
	// ==========================================

	/**
	 * @brief Set the race to target score mode
	 * @param Score Target score to win
	 * @note First racer to reach this score wins
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift Race|Config")
	void SetTargetScore(float Score) { TargetScore = Score; bIsLapBased = false; }

	/**
	 * @brief Set the race to lap-based mode
	 * @param Laps Number of laps to complete
	 * @note Highest score after all laps wins
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift Race|Config")
	void SetLapBased(int32 Laps) { TotalLaps = Laps; bIsLapBased = true; }

	// ==========================================
	// EVENTS
	// Delegates for drift events
	// ==========================================

	/** Broadcast when any racer starts a drift */
	UPROPERTY(BlueprintAssignable, Category = "Drift Race|Events")
	FOnDriftStarted OnDriftStarted;

	/** Broadcast when a drift grade changes */
	UPROPERTY(BlueprintAssignable, Category = "Drift Race|Events")
	FOnDriftGradeChanged OnDriftGradeChanged;

	/** Broadcast when a drift ends with final score */
	UPROPERTY(BlueprintAssignable, Category = "Drift Race|Events")
	FOnDriftEnded OnDriftEnded;

	/** Broadcast when chain multiplier increases */
	UPROPERTY(BlueprintAssignable, Category = "Drift Race|Events")
	FOnChainIncreased OnChainIncreased;

protected:
	// ==========================================
	// DRIFT DETECTION CONFIGURATION
	// Thresholds and timing for drift detection
	// ==========================================

	/** Minimum slip angle to register as a drift (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float MinDriftAngle = 15.0f;

	/** Minimum speed to register as a drift (cm/s, ~36 km/h) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float MinDriftSpeed = 1000.0f;

	/** Grace period after drift ends before finalizing score (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float DriftEndGracePeriod = 0.5f;

	/** Time window to link drifts into a chain (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float ChainWindowSeconds = 2.0f;

	/** Distance to detect tandem drifting (cm, ~15 meters) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Detection")
	float TandemDistance = 1500.0f;

	// ==========================================
	// SCORING CONFIGURATION
	// Point calculation parameters
	// ==========================================

	/** Base points awarded per second of drifting */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float BasePointsPerSecond = 100.0f;

	/**
	 * Multiplier for drift angle contribution
	 * Formula: points *= (1 + angle/90 * AngleScoreMultiplier)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float AngleScoreMultiplier = 1.0f;

	/**
	 * Multiplier for speed contribution
	 * Formula: points *= (1 + speed/maxSpeed * SpeedScoreMultiplier)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float SpeedScoreMultiplier = 0.5f;

	/** How much the multiplier increases per chained drift */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float ChainMultiplierIncrement = 0.25f;

	/** Maximum chain multiplier (caps at this value) */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float MaxChainMultiplier = 5.0f;

	/** Bonus multiplier when tandem drifting */
	UPROPERTY(EditDefaultsOnly, Category = "Drift|Scoring")
	float TandemBonusMultiplier = 1.5f;

	// ==========================================
	// RUNTIME STATE
	// Per-racer tracking data
	// ==========================================

	/** Active drift state for each racer (key = racer index) */
	UPROPERTY()
	TMap<int32, FMGActiveDrift> ActiveDrifts;

	/** Total accumulated score for each racer */
	UPROPERTY()
	TMap<int32, float> TotalScores;

	/** Best single drift score for each racer */
	UPROPERTY()
	TMap<int32, float> BestDriftScores;

	// ==========================================
	// MODE CONFIGURATION
	// Victory condition settings
	// ==========================================

	/** True for lap-based, false for target score mode */
	UPROPERTY()
	bool bIsLapBased = true;

	/** Number of laps (if lap-based mode) */
	UPROPERTY()
	int32 TotalLaps = 3;

	/** Score needed to win (if target score mode) */
	UPROPERTY()
	float TargetScore = 50000.0f;

	// ==========================================
	// INTERNAL METHODS
	// Helper functions for drift processing
	// ==========================================

	/**
	 * @brief Calculate the drift grade from current angle and speed
	 * @param Angle Current slip angle in degrees
	 * @param Speed Current speed in cm/s
	 * @return Appropriate drift grade
	 */
	EMGDriftGrade CalculateDriftGrade(float Angle, float Speed) const;

	/**
	 * @brief Calculate points earned this frame
	 * @param Drift Current drift state
	 * @param DeltaTime Frame time
	 * @return Points to add
	 */
	float CalculateDriftPoints(const FMGActiveDrift& Drift, float MGDeltaTime) const;

	/**
	 * @brief Check if another racer is drifting nearby
	 * @param RacerIndex Racer to check around
	 * @return True if tandem drift is active
	 */
	bool CheckTandemDrift(int32 RacerIndex) const;

	/**
	 * @brief Finalize a drift and add points to total
	 * @param RacerIndex Racer whose drift ended
	 */
	void FinalizeDrift(int32 RacerIndex);

	/**
	 * @brief Extract the slip angle from a vehicle
	 * @param Vehicle Vehicle to analyze
	 * @return Slip angle in degrees
	 */
	float GetVehicleDriftAngle(AMGVehiclePawn* Vehicle) const;
};

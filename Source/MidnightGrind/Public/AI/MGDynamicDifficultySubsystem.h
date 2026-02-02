// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGDynamicDifficultySubsystem.h
 * @brief Dynamic Difficulty Scaling for Single-Player Progression
 *
 * @section overview Overview
 * This subsystem provides adaptive difficulty scaling across the 150-200h
 * single-player campaign. It tracks player performance metrics and subtly
 * adjusts AI competitiveness to maintain challenge without frustration.
 *
 * Key Design Principles:
 * - RESPONSIVE to frustration (quick help if struggling)
 * - CONSERVATIVE on increases (slow to make races harder)
 * - TRANSPARENT (changes feel natural, not artificial)
 * - FAIR (no physics cheats, only decision quality adjustments)
 *
 * @section concepts Key Concepts
 *
 * @subsection metrics Performance Metrics
 * Tracks multiple dimensions of player skill:
 * - **Race Results:** Win rate, finish positions, consecutive outcomes
 * - **Driving Skill:** Cornering speed, braking precision, line accuracy
 * - **Engagement:** Restart rate, playtime, race completion
 *
 * @subsection adjustment Adjustment Algorithm
 * Difficulty adjustments are applied as multipliers to AI profiles:
 * - Struggling players: -5% to -10% AI speed/skill
 * - Dominant players: +3% to +5% AI speed/skill
 * - Adjustments applied gradually over multiple races
 * - Reset on tier changes (new progression phase)
 *
 * @section usage Usage Examples
 *
 * @subsection record_result Recording Race Results
 * @code
 * UMGDynamicDifficultySubsystem* DifficultySystem = 
 *     GetGameInstance()->GetSubsystem<UMGDynamicDifficultySubsystem>();
 *
 * // After race completion
 * DifficultySystem->RecordRaceResult(
 *     PlayerPosition,      // 1-8
 *     TotalRacers,         // 8
 *     PlayerFinishTime,    // 123.45f
 *     WinnerFinishTime,    // 120.00f
 *     bPlayerRestarted     // false
 * );
 * @endcode
 *
 * @subsection apply_difficulty Applying Difficulty
 * @code
 * // Get current difficulty adjustment
 * float Adjustment = DifficultySystem->GetCurrentDifficultyAdjustment();
 *
 * // Apply to AI profile before spawning
 * UMGAIDriverProfile* Profile = LoadDriverProfile();
 * DifficultySystem->ApplyDifficultyToProfile(Profile, Adjustment);
 *
 * // Spawn AI with adjusted profile
 * SpawnAIOpponent(Profile, SpawnTransform);
 * @endcode
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AI/MG_AI_DriverProfile.h"
#include "AI/MG_AI_TierSystem.h"
#include "MGDynamicDifficultySubsystem.generated.h"

/**
 * Player performance metrics tracking
 * Aggregated data used to determine difficulty adjustments
 */
USTRUCT(BlueprintType)
struct FMGPlayerPerformanceMetrics
{
	GENERATED_BODY()

	// ==========================================
	// RACE PERFORMANCE
	// ==========================================

	/** Rolling average finish position (1-8, lower = better) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Racing")
	float AverageFinishPosition = 4.0f;

	/** Total races completed */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Racing")
	int32 TotalRacesCompleted = 0;

	/** Total wins */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Racing")
	int32 TotalWins = 0;

	/** Total losses (finish position > 3) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Racing")
	int32 TotalLosses = 0;

	/** Win rate (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance|Racing")
	float WinRate = 0.0f;

	/** Consecutive wins streak */
	UPROPERTY(BlueprintReadOnly, Category = "Performance|Racing")
	int32 ConsecutiveWins = 0;

	/** Consecutive losses streak */
	UPROPERTY(BlueprintReadOnly, Category = "Performance|Racing")
	int32 ConsecutiveLosses = 0;

	/** Podium finish rate (top 3, 0-1) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Racing")
	float PodiumRate = 0.0f;

	// ==========================================
	// SKILL INDICATORS
	// ==========================================

	/** Average cornering speed as % of optimal (0-1.2) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Skill")
	float AverageCorneringSpeed = 0.0f;

	/** Braking precision (how close to optimal braking points, 0-1) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Skill")
	float BrakingPrecision = 0.0f;

	/** Line accuracy (average deviation from racing line, lower = better) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Skill")
	float LineAccuracy = 0.0f;

	/** Overtake success rate (0-1) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Skill")
	float OvertakeSuccessRate = 0.0f;

	/** Average crash count per race */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Skill")
	float AverageCrashesPerRace = 0.0f;

	// ==========================================
	// ENGAGEMENT INDICATORS
	// ==========================================

	/** Total playtime in hours */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Engagement")
	float PlaytimeHours = 0.0f;

	/** Number of restarts in last 10 races (frustration indicator) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance|Engagement")
	int32 RestartsInLast10Races = 0;

	/** Race completion rate (finished races / started races, 0-1) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Engagement")
	float RaceCompletionRate = 1.0f;

	/** Average time between races (seconds, lower = more engaged) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Engagement")
	float AverageTimeBetweenRaces = 0.0f;

	// ==========================================
	// PROGRESSION
	// ==========================================

	/** Current campaign progress (0-1) */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Progression")
	float CampaignProgress = 0.0f;

	/** Current player level */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Progression")
	int32 PlayerLevel = 1;

	/** Current car tier owned */
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Performance|Progression")
	int32 CurrentCarTier = 1;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Calculate overall skill rating (0-100) */
	FORCEINLINE int32 GetSkillRating() const
	{
		float Rating = 0.0f;
		Rating += (1.0f - (AverageFinishPosition / 8.0f)) * 30.0f;  // 30% weight on positions
		Rating += WinRate * 25.0f;                                   // 25% weight on win rate
		Rating += AverageCorneringSpeed * 20.0f;                     // 20% weight on cornering
		Rating += BrakingPrecision * 15.0f;                          // 15% weight on braking
		Rating += (1.0f - (AverageCrashesPerRace / 5.0f)) * 10.0f;  // 10% weight on crashes
		return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
	}

	/** Check if player is struggling (needs difficulty reduction) */
	FORCEINLINE bool IsStruggling() const
	{
		return (ConsecutiveLosses >= 3) ||
		       (WinRate < 0.2f && TotalRacesCompleted >= 10) ||
		       (RestartsInLast10Races >= 3);
	}

	/** Check if player is dominating (needs difficulty increase) */
	FORCEINLINE bool IsDominating() const
	{
		return (ConsecutiveWins >= 5) ||
		       (WinRate > 0.7f && TotalRacesCompleted >= 10) ||
		       (PodiumRate > 0.85f && TotalRacesCompleted >= 15);
	}
};

/**
 * Difficulty adjustment configuration
 * Defines how aggressively the system responds to performance
 */
USTRUCT(BlueprintType)
struct FMGDifficultyAdjustmentConfig
{
	GENERATED_BODY()

	/** How quickly to reduce difficulty when player struggles (0.01-0.15) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adjustment", meta = (ClampMin = "0.01", ClampMax = "0.15"))
	float StruggleReductionRate = 0.08f;

	/** How quickly to increase difficulty when player dominates (0.01-0.1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adjustment", meta = (ClampMin = "0.01", ClampMax = "0.1"))
	float DominanceIncreaseRate = 0.03f;

	/** Minimum difficulty adjustment (-0.2 to 0.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adjustment", meta = (ClampMin = "-0.2", ClampMax = "0.0"))
	float MinAdjustment = -0.15f;

	/** Maximum difficulty adjustment (0.0 to +0.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adjustment", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float MaxAdjustment = 0.15f;

	/** Races to fully transition to new difficulty (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adjustment", meta = (ClampMin = "1", ClampMax = "10"))
	int32 TransitionRaces = 3;

	/** Enable aggressive help for very frustrated players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adjustment")
	bool bEnableAggressiveHelp = true;

	/** Restart count threshold for aggressive help */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adjustment", meta = (ClampMin = "2", ClampMax = "10"))
	int32 AggressiveHelpThreshold = 5;

	/** Aggressive help difficulty reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adjustment", meta = (ClampMin = "-0.3", ClampMax = "0.0"))
	float AggressiveHelpReduction = -0.20f;
};

/**
 * Difficulty adjustment event
 * Logged for analytics and debugging
 */
USTRUCT(BlueprintType)
struct FMGDifficultyAdjustmentEvent
{
	GENERATED_BODY()

	/** Timestamp of adjustment */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Reason for adjustment */
	UPROPERTY(BlueprintReadOnly)
	FString Reason;

	/** Old difficulty adjustment value */
	UPROPERTY(BlueprintReadOnly)
	float OldAdjustment = 0.0f;

	/** New difficulty adjustment value */
	UPROPERTY(BlueprintReadOnly)
	float NewAdjustment = 0.0f;

	/** Player metrics at time of adjustment */
	UPROPERTY(BlueprintReadOnly)
	FMGPlayerPerformanceMetrics MetricsSnapshot;
};

/** Delegate for difficulty change events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDifficultyAdjusted, float, OldAdjustment, float, NewAdjustment);

/**
 * Dynamic Difficulty Subsystem
 * Tracks player performance and adjusts AI difficulty accordingly
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDynamicDifficultySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// INITIALIZATION
	// ==========================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// METRICS TRACKING
	// ==========================================

	/**
	 * Record race result for difficulty tracking
	 * Call this after every completed race
	 * @param PlayerPosition Final race position (1 = first)
	 * @param TotalRacers Total racers in race
	 * @param PlayerFinishTime Player's finish time
	 * @param WinnerFinishTime Winner's finish time
	 * @param bPlayerRestarted Did player restart this race
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	void RecordRaceResult(
		int32 PlayerPosition,
		int32 TotalRacers,
		float PlayerFinishTime,
		float WinnerFinishTime,
		bool bPlayerRestarted
	);

	/**
	 * Record skill-specific performance metrics
	 * Call this periodically during races
	 * @param CorneringSpeed Current cornering speed as % of optimal
	 * @param BrakingScore Braking precision score (0-1)
	 * @param LineDeviation Current deviation from racing line
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	void RecordSkillMetrics(
		float CorneringSpeed,
		float BrakingScore,
		float LineDeviation
	);

	/**
	 * Record overtake attempt result
	 * @param bSuccess Did overtake succeed
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	void RecordOvertakeAttempt(bool bSuccess);

	/**
	 * Record crash event
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	void RecordCrash();

	/**
	 * Get current performance metrics
	 * @return Current metrics struct
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Difficulty")
	FMGPlayerPerformanceMetrics GetPerformanceMetrics() const { return CurrentMetrics; }

	// ==========================================
	// DIFFICULTY ADJUSTMENT
	// ==========================================

	/**
	 * Calculate current difficulty adjustment
	 * Returns multiplier to apply to AI profiles (0.85-1.15)
	 * @return Difficulty adjustment multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	float GetCurrentDifficultyAdjustment() const { return CurrentAdjustment; }

	/**
	 * Calculate difficulty adjustment for specific tier
	 * Tier affects how aggressive adjustments are
	 * @param Tier AI tier
	 * @return Difficulty adjustment for tier
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	float GetDifficultyAdjustmentForTier(EMGAITier Tier) const;

	/**
	 * Apply difficulty adjustment to AI profile
	 * Modifies profile in-place
	 * @param Profile Profile to adjust
	 * @param Adjustment Difficulty multiplier (0.85-1.15)
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	void ApplyDifficultyToProfile(UMGAIDriverProfile* Profile, float Adjustment);

	/**
	 * Force recalculate difficulty adjustment
	 * Normally updates automatically, but can be forced
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	void RecalculateDifficulty();

	/**
	 * Reset difficulty adjustment to neutral
	 * Use when changing tiers or starting new campaign phase
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	void ResetDifficultyAdjustment();

	/**
	 * Set difficulty adjustment configuration
	 * @param NewConfig New configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	void SetAdjustmentConfig(const FMGDifficultyAdjustmentConfig& NewConfig);

	// ==========================================
	// QUERIES
	// ==========================================

	/**
	 * Check if player is currently struggling
	 * @return True if struggling (may need help)
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Difficulty")
	bool IsPlayerStruggling() const { return CurrentMetrics.IsStruggling(); }

	/**
	 * Check if player is currently dominating
	 * @return True if dominating (may need challenge)
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Difficulty")
	bool IsPlayerDominating() const { return CurrentMetrics.IsDominating(); }

	/**
	 * Get player skill rating (0-100)
	 * @return Overall skill rating
	 */
	UFUNCTION(BlueprintPure, Category = "Dynamic Difficulty")
	int32 GetPlayerSkillRating() const { return CurrentMetrics.GetSkillRating(); }

	/**
	 * Get adjustment history for debugging
	 * @return Array of recent adjustments
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Difficulty")
	TArray<FMGDifficultyAdjustmentEvent> GetAdjustmentHistory() const { return AdjustmentHistory; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when difficulty is adjusted */
	UPROPERTY(BlueprintAssignable, Category = "Dynamic Difficulty|Events")
	FOnDifficultyAdjusted OnDifficultyAdjusted;

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Current performance metrics */
	UPROPERTY(SaveGame)
	FMGPlayerPerformanceMetrics CurrentMetrics;

	/** Current difficulty adjustment multiplier */
	UPROPERTY(SaveGame)
	float CurrentAdjustment = 1.0f;

	/** Target difficulty adjustment (transitions gradually) */
	UPROPERTY(SaveGame)
	float TargetAdjustment = 1.0f;

	/** Races since last tier change (resets adjustment) */
	UPROPERTY(SaveGame)
	int32 RacesSinceLastTierChange = 0;

	/** Adjustment configuration */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	FMGDifficultyAdjustmentConfig Config;

	/** Recent adjustment history (for debugging/analytics) */
	UPROPERTY()
	TArray<FMGDifficultyAdjustmentEvent> AdjustmentHistory;

	/** Maximum history entries to keep */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	int32 MaxHistoryEntries = 50;

	// Metrics rolling window tracking
	TArray<int32> RecentFinishPositions;
	TArray<bool> RecentRestarts;
	int32 OvertakeAttempts = 0;
	int32 OvertakeSuccesses = 0;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Update rolling average metrics */
	void UpdateRollingAverages();

	/** Calculate target adjustment based on metrics */
	float CalculateTargetAdjustment() const;

	/** Transition current adjustment toward target */
	void TransitionAdjustment();

	/** Log adjustment event */
	void LogAdjustmentEvent(const FString& Reason, float OldValue, float NewValue);

	/** Apply adjustment to skill parameters */
	void AdjustSkillParameters(FMGAISkillParams& Skill, float Adjustment) const;

	/** Apply adjustment to speed parameters */
	void AdjustSpeedParameters(FMGAISpeedParams& Speed, float Adjustment) const;

	/** Apply adjustment to aggression parameters */
	void AdjustAggressionParameters(FMGAIAggressionParams& Aggression, float Adjustment) const;
};

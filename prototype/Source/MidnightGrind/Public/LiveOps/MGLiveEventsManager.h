// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLiveEventsManager.generated.h"

class UMGTransactionPipeline;
class UMGProgressionSubsystem;

/**
 * Challenge type
 */
UENUM(BlueprintType)
enum class EMGChallengeType : uint8
{
	/** Complete X races */
	RaceCount,
	/** Win X races */
	WinCount,
	/** Finish in top 3 */
	PodiumCount,
	/** Accumulate drift score */
	DriftScore,
	/** Accumulate distance */
	Distance,
	/** Beat specific time on track */
	LapTime,
	/** Use specific vehicle class */
	VehicleClass,
	/** Use specific vehicle */
	SpecificVehicle,
	/** Race on specific track */
	SpecificTrack,
	/** Win without damage */
	FlawlessWin,
	/** Overtake X opponents */
	Overtakes,
	/** Earn X credits */
	EarnCredits,
	/** Win by X seconds */
	DominatingWin,
	/** Win from last place */
	CombackWin,
	/** Perfect start (no wheel spin) */
	PerfectStart,
	/** Complete race without NOS */
	NoNOS,
	/** Beat a rival */
	BeatRival,
	/** Pink slip victory */
	PinkSlipWin
};

/**
 * Challenge reset period
 */
UENUM(BlueprintType)
enum class EMGChallengeReset : uint8
{
	/** Resets daily at midnight */
	Daily,
	/** Resets weekly on Monday */
	Weekly,
	/** Limited time event */
	Event,
	/** Never resets (one-time) */
	Permanent
};

/**
 * Challenge difficulty
 */
UENUM(BlueprintType)
enum class EMGChallengeDifficulty : uint8
{
	Easy,
	Medium,
	Hard,
	Extreme
};

/**
 * Challenge reward
 */
USTRUCT(BlueprintType)
struct FMGChallengeReward
{
	GENERATED_BODY()

	/** Credits reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Credits = 0;

	/** XP reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XP = 0;

	/** Reputation reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Reputation = 0;

	/** Item unlock (if any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockItemID;

	/** Item type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockItemType;
};

/**
 * Challenge definition
 */
USTRUCT(BlueprintType)
struct FMGChallenge
{
	GENERATED_BODY()

	/** Unique challenge ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Challenge type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeType Type = EMGChallengeType::RaceCount;

	/** Reset period */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeReset ResetPeriod = EMGChallengeReset::Daily;

	/** Difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeDifficulty Difficulty = EMGChallengeDifficulty::Easy;

	/** Target value to complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetValue = 1;

	/** Current progress */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentProgress = 0;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bCompleted = false;

	/** Is reward claimed */
	UPROPERTY(BlueprintReadOnly)
	bool bRewardClaimed = false;

	/** Rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGChallengeReward Reward;

	/** Optional: Specific track required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrackID;

	/** Optional: Specific vehicle required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicleID;

	/** Optional: Vehicle class required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicleClass;

	/** Optional: Target time (for lap time challenges) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetTime = 0.0f;

	/** Expiration time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime ExpirationTime;

	/** Icon/Category for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Category;

	/** Get progress percentage */
	float GetProgressPercent() const
	{
		return TargetValue > 0 ? FMath::Clamp(static_cast<float>(CurrentProgress) / TargetValue, 0.0f, 1.0f) : 0.0f;
	}

	/** Get progress text */
	FText GetProgressText() const
	{
		return FText::Format(NSLOCTEXT("MG", "ChallengeProgress", "{0} / {1}"),
			FText::AsNumber(CurrentProgress), FText::AsNumber(TargetValue));
	}
};

/**
 * Community goal
 */
USTRUCT(BlueprintType)
struct FMGCommunityGoal
{
	GENERATED_BODY()

	/** Goal ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GoalID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Target value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TargetValue = 0;

	/** Current community progress */
	UPROPERTY(BlueprintReadOnly)
	int64 CurrentProgress = 0;

	/** Player's contribution */
	UPROPERTY(BlueprintReadOnly)
	int64 PlayerContribution = 0;

	/** Tiers of rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> RewardTiers; // Percentages: 25%, 50%, 75%, 100%

	/** Reward per tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeReward> TierRewards;

	/** Current tier reached */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentTier = 0;

	/** Expiration */
	UPROPERTY(BlueprintReadOnly)
	FDateTime ExpirationTime;

	/** Is active */
	UPROPERTY(BlueprintReadOnly)
	bool bActive = true;
};

/**
 * Live event
 */
USTRUCT(BlueprintType)
struct FMGLiveEvent
{
	GENERATED_BODY()

	/** Event ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Event type (Weekend Showdown, Holiday Special, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventType;

	/** Start time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime StartTime;

	/** End time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime EndTime;

	/** Challenges specific to this event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallenge> EventChallenges;

	/** Community goal (if any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGCommunityGoal CommunityGoal;

	/** Special modifiers active during event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ActiveModifiers;

	/** XP multiplier during event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float XPMultiplier = 1.0f;

	/** Credits multiplier during event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CreditsMultiplier = 1.0f;

	/** Featured vehicles (bonus rewards) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> FeaturedVehicles;

	/** Featured tracks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> FeaturedTracks;

	/** Is currently active */
	bool IsActive() const
	{
		FDateTime Now = FDateTime::UtcNow();
		return Now >= StartTime && Now <= EndTime;
	}

	/** Time remaining */
	FTimespan GetTimeRemaining() const
	{
		return EndTime - FDateTime::UtcNow();
	}
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeProgressUpdated, const FMGChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeCompleted, const FMGChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeRewardClaimed, const FMGChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCommunityGoalUpdated, const FMGCommunityGoal&, Goal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunityGoalTierReached, const FMGCommunityGoal&, Goal, int32, Tier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLiveEventStarted, const FMGLiveEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLiveEventEnded, const FMGLiveEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDailyChallengesRefreshed);

/**
 * Live Events Manager
 * Handles daily/weekly challenges, community goals, and live events
 *
 * Features:
 * - Daily and weekly challenge rotation
 * - Progress tracking across races
 * - Community-wide goals
 * - Limited-time events with special rewards
 * - XP/Credit multipliers during events
 * - Automatic reset at appropriate intervals
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLiveEventsManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// CHALLENGE MANAGEMENT
	// ==========================================

	/**
	 * Get all active daily challenges
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGChallenge> GetDailyChallenges() const;

	/**
	 * Get all active weekly challenges
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGChallenge> GetWeeklyChallenges() const;

	/**
	 * Get challenge by ID
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	FMGChallenge GetChallenge(FName ChallengeID) const;

	/**
	 * Update challenge progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void UpdateChallengeProgress(FName ChallengeID, int32 ProgressDelta);

	/**
	 * Process race results for challenge progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void ProcessRaceForChallenges(const struct FMGRaceResults& Results);

	/**
	 * Claim challenge reward
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	bool ClaimChallengeReward(FName ChallengeID);

	/**
	 * Get time until daily reset
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	FTimespan GetTimeUntilDailyReset() const;

	/**
	 * Get time until weekly reset
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	FTimespan GetTimeUntilWeeklyReset() const;

	// ==========================================
	// COMMUNITY GOALS
	// ==========================================

	/**
	 * Get active community goals
	 */
	UFUNCTION(BlueprintPure, Category = "Community")
	TArray<FMGCommunityGoal> GetActiveCommunityGoals() const;

	/**
	 * Contribute to community goal
	 */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void ContributeToCommunityGoal(FName GoalID, int64 Amount);

	/**
	 * Get player's contribution to goal
	 */
	UFUNCTION(BlueprintPure, Category = "Community")
	int64 GetPlayerContribution(FName GoalID) const;

	// ==========================================
	// LIVE EVENTS
	// ==========================================

	/**
	 * Get active live events
	 */
	UFUNCTION(BlueprintPure, Category = "Events")
	TArray<FMGLiveEvent> GetActiveLiveEvents() const;

	/**
	 * Get upcoming events
	 */
	UFUNCTION(BlueprintPure, Category = "Events")
	TArray<FMGLiveEvent> GetUpcomingEvents() const;

	/**
	 * Get current event multipliers
	 */
	UFUNCTION(BlueprintPure, Category = "Events")
	void GetEventMultipliers(float& OutXPMultiplier, float& OutCreditsMultiplier) const;

	/**
	 * Is vehicle featured in current event
	 */
	UFUNCTION(BlueprintPure, Category = "Events")
	bool IsVehicleFeatured(FName VehicleID) const;

	/**
	 * Is track featured in current event
	 */
	UFUNCTION(BlueprintPure, Category = "Events")
	bool IsTrackFeatured(FName TrackID) const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Challenge progress updated */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengeProgressUpdated OnChallengeProgressUpdated;

	/** Challenge completed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengeCompleted OnChallengeCompleted;

	/** Challenge reward claimed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengeRewardClaimed OnChallengeRewardClaimed;

	/** Community goal updated */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCommunityGoalUpdated OnCommunityGoalUpdated;

	/** Community goal tier reached */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCommunityGoalTierReached OnCommunityGoalTierReached;

	/** Live event started */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLiveEventStarted OnLiveEventStarted;

	/** Live event ended */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLiveEventEnded OnLiveEventEnded;

	/** Daily challenges refreshed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDailyChallengesRefreshed OnDailyChallengesRefreshed;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Generate daily challenges */
	void GenerateDailyChallenges();

	/** Generate weekly challenges */
	void GenerateWeeklyChallenges();

	/** Check for resets */
	void CheckForResets();

	/** Create challenge from template */
	FMGChallenge CreateChallenge(EMGChallengeType Type, EMGChallengeReset Reset,
		EMGChallengeDifficulty Difficulty, int32 Target);

	/** Award challenge reward */
	void AwardReward(const FMGChallengeReward& Reward);

	/** Update event status */
	void UpdateEventStatus();

	/** Timer handle for periodic checks */
	FTimerHandle ResetCheckTimer;

private:
	/** Active daily challenges */
	UPROPERTY()
	TArray<FMGChallenge> DailyChallenges;

	/** Active weekly challenges */
	UPROPERTY()
	TArray<FMGChallenge> WeeklyChallenges;

	/** Active community goals */
	UPROPERTY()
	TArray<FMGCommunityGoal> CommunityGoals;

	/** Active live events */
	UPROPERTY()
	TArray<FMGLiveEvent> LiveEvents;

	/** Last daily reset time */
	FDateTime LastDailyReset;

	/** Last weekly reset time */
	FDateTime LastWeeklyReset;

	/** Transaction pipeline reference */
	TWeakObjectPtr<UMGTransactionPipeline> TransactionPipeline;

	/** Number of daily challenges to generate */
	static constexpr int32 NumDailyChallenges = 3;

	/** Number of weekly challenges */
	static constexpr int32 NumWeeklyChallenges = 5;
};

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAchievementSubsystem.generated.h"

class UTexture2D;

/**
 * Achievement type
 */
UENUM(BlueprintType)
enum class EMGAchievementType : uint8
{
	/** One-time achievement */
	Standard,
	/** Multi-tier achievement (Bronze, Silver, Gold) */
	Tiered,
	/** Hidden until unlocked */
	Secret,
	/** Cumulative progress */
	Cumulative
};

/**
 * Achievement rarity
 */
UENUM(BlueprintType)
enum class EMGAchievementRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary
};

/**
 * Challenge type
 */
UENUM(BlueprintType)
enum class EMGChallengeType : uint8
{
	/** Daily challenge */
	Daily,
	/** Weekly challenge */
	Weekly,
	/** Special event */
	Event,
	/** Career milestone */
	Career
};

/**
 * Achievement definition
 */
USTRUCT(BlueprintType)
struct FMGAchievementDef
{
	GENERATED_BODY()

	/** Unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AchievementID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;

	/** Achievement type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAchievementType Type = EMGAchievementType::Standard;

	/** Rarity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAchievementRarity Rarity = EMGAchievementRarity::Common;

	/** Required progress (for cumulative/tiered) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredProgress = 1;

	/** Tier thresholds (for tiered achievements) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> TierThresholds;

	/** Cash reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CashReward = 0;

	/** Reputation reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationReward = 0;

	/** Unlock item (vehicle, customization, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockItem;

	/** Is hidden until unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSecret = false;

	/** Stat to track (for auto-tracking) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackedStat;
};

/**
 * Achievement progress
 */
USTRUCT(BlueprintType)
struct FMGAchievementProgress
{
	GENERATED_BODY()

	/** Achievement ID */
	UPROPERTY(BlueprintReadOnly)
	FName AchievementID;

	/** Current progress */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentProgress = 0;

	/** Is unlocked */
	UPROPERTY(BlueprintReadOnly)
	bool bIsUnlocked = false;

	/** Current tier (for tiered achievements) */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentTier = 0;

	/** Unlock timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime UnlockTime;
};

/**
 * Challenge definition
 */
USTRUCT(BlueprintType)
struct FMGChallengeDef
{
	GENERATED_BODY()

	/** Unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Challenge type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeType Type = EMGChallengeType::Daily;

	/** Required progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredProgress = 1;

	/** Cash reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CashReward = 500;

	/** Reputation reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationReward = 50;

	/** Expiration time (relative) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimespan Duration;

	/** Required track (if specific) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrack;

	/** Required vehicle class (if specific) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicleClass;

	/** Tracked stat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackedStat;
};

/**
 * Challenge progress
 */
USTRUCT(BlueprintType)
struct FMGChallengeProgress
{
	GENERATED_BODY()

	/** Challenge definition */
	UPROPERTY(BlueprintReadOnly)
	FMGChallengeDef Challenge;

	/** Current progress */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentProgress = 0;

	/** Start time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime StartTime;

	/** Expiration time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime ExpirationTime;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;

	/** Is claimed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsClaimed = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementUnlocked, FName, AchievementID, const FMGAchievementDef&, Achievement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAchievementProgress, FName, AchievementID, int32, CurrentProgress, int32, RequiredProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeCompleted, const FMGChallengeProgress&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChallengesRefreshed);

/**
 * Achievement Subsystem
 * Tracks achievements and challenges
 *
 * Features:
 * - Achievement tracking and unlocking
 * - Daily/weekly challenges
 * - Stat-based auto-tracking
 * - Reward distribution
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementProgress OnAchievementProgress;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengeCompleted OnChallengeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengesRefreshed OnChallengesRefreshed;

	// ==========================================
	// ACHIEVEMENTS
	// ==========================================

	/** Get all achievement definitions */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDef> GetAllAchievements() const { return Achievements; }

	/** Get achievement by ID */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	FMGAchievementDef GetAchievement(FName AchievementID) const;

	/** Get achievement progress */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	FMGAchievementProgress GetAchievementProgress(FName AchievementID) const;

	/** Get all achievement progress */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementProgress> GetAllAchievementProgress() const;

	/** Get unlocked achievements */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDef> GetUnlockedAchievements() const;

	/** Get locked achievements */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDef> GetLockedAchievements() const;

	/** Update achievement progress */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void UpdateAchievementProgress(FName AchievementID, int32 Progress);

	/** Increment achievement progress */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void IncrementAchievement(FName AchievementID, int32 Amount = 1);

	/** Unlock achievement directly */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void UnlockAchievement(FName AchievementID);

	/** Is achievement unlocked */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	bool IsAchievementUnlocked(FName AchievementID) const;

	/** Get total achievement count */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	int32 GetTotalAchievementCount() const { return Achievements.Num(); }

	/** Get unlocked achievement count */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	int32 GetUnlockedAchievementCount() const;

	// ==========================================
	// CHALLENGES
	// ==========================================

	/** Get active challenges */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGChallengeProgress> GetActiveChallenges() const { return ActiveChallenges; }

	/** Get daily challenges */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGChallengeProgress> GetDailyChallenges() const;

	/** Get weekly challenges */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGChallengeProgress> GetWeeklyChallenges() const;

	/** Update challenge progress */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void UpdateChallengeProgress(FName ChallengeID, int32 Progress);

	/** Increment challenge progress */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void IncrementChallenge(FName ChallengeID, int32 Amount = 1);

	/** Claim challenge reward */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	bool ClaimChallengeReward(FName ChallengeID);

	/** Refresh challenges (daily reset) */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void RefreshChallenges();

	// ==========================================
	// STATS
	// ==========================================

	/** Report stat update (auto-updates related achievements/challenges) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ReportStat(FName StatID, int32 Value);

	/** Increment stat */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void IncrementStat(FName StatID, int32 Amount = 1);

	/** Get stat value */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetStatValue(FName StatID) const;

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Achievement definitions */
	UPROPERTY()
	TArray<FMGAchievementDef> Achievements;

	/** Achievement progress */
	UPROPERTY()
	TMap<FName, FMGAchievementProgress> AchievementProgress;

	/** Available challenge pool */
	UPROPERTY()
	TArray<FMGChallengeDef> ChallengePool;

	/** Active challenges */
	UPROPERTY()
	TArray<FMGChallengeProgress> ActiveChallenges;

	/** Player stats */
	UPROPERTY()
	TMap<FName, int32> PlayerStats;

	/** Achievement-to-stat mapping */
	UPROPERTY()
	TMap<FName, TArray<FName>> StatToAchievementMap;

	/** Challenge-to-stat mapping */
	UPROPERTY()
	TMap<FName, TArray<FName>> StatToChallengeMap;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Load achievement definitions */
	void LoadAchievementDefinitions();

	/** Load challenge pool */
	void LoadChallengePool();

	/** Generate daily challenges */
	void GenerateDailyChallenges();

	/** Generate weekly challenges */
	void GenerateWeeklyChallenges();

	/** Check for expired challenges */
	void CheckChallengeExpiration();

	/** Grant achievement rewards */
	void GrantAchievementReward(const FMGAchievementDef& Achievement);

	/** Grant challenge rewards */
	void GrantChallengeReward(const FMGChallengeProgress& Challenge);

	/** Build stat mappings */
	void BuildStatMappings();
};

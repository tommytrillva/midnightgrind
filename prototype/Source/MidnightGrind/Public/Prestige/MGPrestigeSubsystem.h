// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * =============================================================================
 * MGPrestigeSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Prestige system for Midnight Grind. Prestige is an
 * "endgame" progression system that lets max-level players reset their progress
 * in exchange for permanent bonuses and exclusive rewards.
 *
 * The concept comes from Call of Duty's Prestige mode: once you reach max level,
 * you can "prestige" to reset your level to 1, but keep some rewards and unlock
 * exclusive content that only prestiged players can access.
 *
 * WHY PRESTIGE?
 * -------------
 * - Extends gameplay: Players have something to work toward after hitting max level
 * - Shows dedication: Prestige ranks display player commitment
 * - Rewards loyalty: Exclusive items only for those who prestige
 * - Creates variety: Replaying progression with bonuses feels fresh
 *
 * KEY CONCEPTS:
 * -------------
 * 1. PRESTIGE RANKS (EMGPrestigeRank): How many times player has reset.
 *    None -> Prestige I -> II -> III -> ... -> X
 *    -> Master -> Grand Master -> Legend
 *
 *    Higher ranks = more prestige, better multipliers, exclusive rewards.
 *
 * 2. PRESTIGE CATEGORIES (EMGPrestigeCategory): Different progression tracks.
 *    - Overall: Your main account prestige
 *    - Racing: Racing-specific progression
 *    - Drifting: Drift-specific progression
 *    - Tuning: Vehicle customization progression
 *    - Collection: Collector progression
 *
 *    You can prestige in different categories independently!
 *
 * 3. RESET TYPES (EMGPrestigeResetType): What gets reset when you prestige.
 *    - Soft Reset: Only level resets, keep most unlocks
 *    - Hard Reset: Level and some unlocks reset
 *    - Seasonal Reset: End-of-season prestige with special rules
 *    - Full Reset: Everything resets (rarely used)
 *
 * 4. PRESTIGE TOKENS: Special currency earned from prestiging.
 *    Used to buy exclusive items from the Prestige Token Shop.
 *    Higher prestige ranks give more tokens.
 *
 * 5. PERMANENT UNLOCKS: Some rewards survive prestige resets.
 *    Even after resetting, you keep these exclusive items.
 *    This is the main incentive - cool stuff that never goes away!
 *
 * 6. MULTIPLIERS: Prestige grants permanent bonuses.
 *    Prestige I might give 1.1x XP multiplier.
 *    Prestige X might give 2.0x XP multiplier.
 *    Makes re-leveling faster and more rewarding.
 *
 * 7. PRESTIGE MILESTONES: Special achievements for prestige players.
 *    "Reach Prestige V", "Earn 1 million XP total across all prestiges"
 *
 * ARCHITECTURE:
 * -------------
 * UGameInstanceSubsystem - Singleton for entire game session.
 *
 * Key Data Structures:
 * - FMGPlayerPrestige: Player's current prestige state
 * - FMGCategoryPrestige: Prestige data for a specific category
 * - FMGPrestigeRankDefinition: How each rank works (requirements, bonuses)
 * - FMGPrestigeReward: Items unlocked through prestige
 * - FMGPrestigeResetResult: What happens when you prestige
 * - FMGPrestigeMilestone: Special prestige-specific achievements
 * - FMGPrestigeLeaderboardEntry: Ranking among prestige players
 * - FMGPrestigeTokenShopItem: Items purchasable with prestige tokens
 * - FMGPrestigePlayerStats: Lifetime prestige statistics
 *
 * THE PRESTIGE DECISION:
 * ----------------------
 * When player reaches max level (100), they can choose to prestige:
 *
 * BEFORE PRESTIGE:
 * - Level 100
 * - Has unlocked many items through normal play
 * - Can't progress further
 *
 * AFTER PRESTIGE:
 * - Level 1 again
 * - Loses some unlocks (depends on reset type)
 * - GAINS: Prestige rank, prestige tokens, permanent unlocks, XP multiplier
 * - Can now work toward next prestige rank
 *
 * TYPICAL WORKFLOW:
 * -----------------
 * 1. Player reaches max level -> CanPrestige() returns true
 * 2. UI shows "Prestige Available!" with preview: GetPrestigePreview()
 * 3. Player reviews what they'll gain/lose
 * 4. Player confirms -> PerformPrestige()
 * 5. System returns FMGPrestigeResetResult with all changes
 * 6. Player starts fresh at level 1 with bonuses
 * 7. XP now earns faster due to multiplier: AddExperience()
 * 8. Player can buy exclusive items: PurchaseShopItem()
 *
 * DELEGATES (Events):
 * -------------------
 * - OnPrestigeRankUp: Player advanced to a new prestige rank
 * - OnPrestigeLevelUp: Player gained a level (post-prestige)
 * - OnPrestigeReset: Prestige was performed
 * - OnPrestigeRewardUnlocked: New reward became available
 * - OnPrestigeTokensChanged: Token balance changed
 * - OnPrestigeMilestoneAchieved: Hit a prestige-specific milestone
 * - OnPrestigeEligible: Player CAN now prestige (reached max level)
 * - OnCategoryPrestigeUp: Prestiged in a specific category
 * - OnPrestigeExperienceGained: XP was earned (with multiplier)
 *
 * RELATIONSHIP TO OTHER SYSTEMS:
 * ------------------------------
 * - Level/XP System: Prestige resets level, applies multipliers
 * - Milestone System: Prestige unlocks special milestones
 * - Inventory/Unlocks: Some items persist, some don't
 * - Leaderboards: Prestige has its own leaderboard
 * - Daily Login: Login streaks may survive prestige (design choice)
 *
 * Prestige is the "long-term commitment" layer - it's for players who
 * love the game and want to demonstrate their dedication while
 * earning exclusive rewards unavailable to casual players.
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPrestigeSubsystem.generated.h"

/**
 * Prestige rank
 */
UENUM(BlueprintType)
enum class EMGPrestigeRank : uint8
{
	None				UMETA(DisplayName = "None"),
	Prestige1			UMETA(DisplayName = "Prestige I"),
	Prestige2			UMETA(DisplayName = "Prestige II"),
	Prestige3			UMETA(DisplayName = "Prestige III"),
	Prestige4			UMETA(DisplayName = "Prestige IV"),
	Prestige5			UMETA(DisplayName = "Prestige V"),
	Prestige6			UMETA(DisplayName = "Prestige VI"),
	Prestige7			UMETA(DisplayName = "Prestige VII"),
	Prestige8			UMETA(DisplayName = "Prestige VIII"),
	Prestige9			UMETA(DisplayName = "Prestige IX"),
	Prestige10			UMETA(DisplayName = "Prestige X"),
	PrestigeMaster		UMETA(DisplayName = "Prestige Master"),
	PrestigeGrandMaster	UMETA(DisplayName = "Grand Master"),
	PrestigeLegend		UMETA(DisplayName = "Legend")
};

/**
 * Prestige category
 */
UENUM(BlueprintType)
enum class EMGPrestigeCategory : uint8
{
	Overall				UMETA(DisplayName = "Overall"),
	Racing				UMETA(DisplayName = "Racing"),
	Drifting			UMETA(DisplayName = "Drifting"),
	Tuning				UMETA(DisplayName = "Tuning"),
	Collection			UMETA(DisplayName = "Collection"),
	Social				UMETA(DisplayName = "Social"),
	Combat				UMETA(DisplayName = "Combat"),
	Exploration			UMETA(DisplayName = "Exploration"),
	Ranked				UMETA(DisplayName = "Ranked")
};

/**
 * Reset type
 */
UENUM(BlueprintType)
enum class EMGPrestigeResetType : uint8
{
	Soft				UMETA(DisplayName = "Soft Reset"),
	Hard				UMETA(DisplayName = "Hard Reset"),
	Seasonal			UMETA(DisplayName = "Seasonal Reset"),
	Full				UMETA(DisplayName = "Full Reset")
};

/**
 * Unlock type
 */
UENUM(BlueprintType)
enum class EMGPrestigeUnlockType : uint8
{
	Cosmetic			UMETA(DisplayName = "Cosmetic"),
	Vehicle				UMETA(DisplayName = "Vehicle"),
	Part				UMETA(DisplayName = "Part"),
	Badge				UMETA(DisplayName = "Badge"),
	Title				UMETA(DisplayName = "Title"),
	Emote				UMETA(DisplayName = "Emote"),
	Effect				UMETA(DisplayName = "Effect"),
	Bonus				UMETA(DisplayName = "Bonus")
};

/**
 * Player prestige data
 */
USTRUCT(BlueprintType)
struct FMGPlayerPrestige
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank CurrentRank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxLevel = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ExperienceToNextLevel = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalExperienceEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesPrestiged = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PrestigeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastPrestigeDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstPrestigeDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> UnlockedRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PermanentUnlocks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PrestigeTokens = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEligibleForPrestige = false;
};

/**
 * Category prestige data
 */
USTRUCT(BlueprintType)
struct FMGCategoryPrestige
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeCategory Category = EMGPrestigeCategory::Overall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank CurrentRank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ExperienceToNextLevel = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesPrestiged = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> CategoryUnlocks;
};

/**
 * Prestige rank definition
 */
USTRUCT(BlueprintType)
struct FMGPrestigeRankDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank Rank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredTimesPrestiged = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExperienceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrencyMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReputationMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusStartingLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PrestigeTokenReward = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RankColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> RankIconAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> RankBadgeAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> RankEffectAsset;
};

/**
 * Prestige reward
 */
USTRUCT(BlueprintType)
struct FMGPrestigeReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RewardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeUnlockType Type = EMGPrestigeUnlockType::Cosmetic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank RequiredRank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PrestigeTokenCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnlockableId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPermanent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsExclusive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> RewardAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> PreviewAsset;
};

/**
 * Prestige reset result
 */
USTRUCT(BlueprintType)
struct FMGPrestigeResetResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank OldRank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank NewRank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OldLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NewLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PrestigeTokensEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> NewUnlocks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NewMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ResetTime;
};

/**
 * Prestige milestone
 */
USTRUCT(BlueprintType)
struct FMGPrestigeMilestone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MilestoneId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank RequiredRank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredTimesPrestiged = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 RequiredTotalExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RewardIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAchieved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AchievedDate;
};

/**
 * Prestige leaderboard entry
 */
USTRUCT(BlueprintType)
struct FMGPrestigeLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank Rank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesPrestiged = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LeaderboardPosition = 0;
};

/**
 * Prestige token shop item
 */
USTRUCT(BlueprintType)
struct FMGPrestigeTokenShopItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TokenCost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank RequiredRank = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnlockableId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLimited = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stock = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PurchaseLimit = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> ItemAsset;
};

/**
 * Prestige player stats
 */
USTRUCT(BlueprintType)
struct FMGPrestigePlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTimesPrestiged = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalExperienceAllTime = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestLevelReached = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPrestigeRank HighestRankAchieved = EMGPrestigeRank::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTokensEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTokensSpent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MilestonesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardsUnlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestPrestige = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPrestigeCategory, int32> CategoryPrestigeCounts;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPrestigeRankUp, const FString&, PlayerId, EMGPrestigeRank, OldRank, EMGPrestigeRank, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPrestigeLevelUp, const FString&, PlayerId, int32, OldLevel, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPrestigeReset, const FString&, PlayerId, const FMGPrestigeResetResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPrestigeRewardUnlocked, const FString&, PlayerId, const FString&, RewardId, bool, bPermanent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPrestigeTokensChanged, const FString&, PlayerId, int32, OldTokens, int32, NewTokens);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPrestigeMilestoneAchieved, const FString&, PlayerId, const FString&, MilestoneId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPrestigeEligible, const FString&, PlayerId, EMGPrestigeRank, NextRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnCategoryPrestigeUp, const FString&, PlayerId, EMGPrestigeCategory, Category, EMGPrestigeRank, OldRank, EMGPrestigeRank, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPrestigeExperienceGained, const FString&, PlayerId, int64, Amount, int64, NewTotal, EMGPrestigeCategory, Category);

/**
 * Prestige Subsystem
 * Manages prestige ranks, resets, and permanent unlocks
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPrestigeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnPrestigeRankUp OnPrestigeRankUp;

	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnPrestigeLevelUp OnPrestigeLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnPrestigeReset OnPrestigeReset;

	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnPrestigeRewardUnlocked OnPrestigeRewardUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnPrestigeTokensChanged OnPrestigeTokensChanged;

	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnPrestigeMilestoneAchieved OnPrestigeMilestoneAchieved;

	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnPrestigeEligible OnPrestigeEligible;

	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnCategoryPrestigeUp OnCategoryPrestigeUp;

	UPROPERTY(BlueprintAssignable, Category = "Prestige|Events")
	FOnPrestigeExperienceGained OnPrestigeExperienceGained;

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Prestige|Registration")
	void RegisterPrestigeRank(const FMGPrestigeRankDefinition& Definition);

	UFUNCTION(BlueprintCallable, Category = "Prestige|Registration")
	void RegisterPrestigeReward(const FMGPrestigeReward& Reward);

	UFUNCTION(BlueprintCallable, Category = "Prestige|Registration")
	void RegisterPrestigeMilestone(const FMGPrestigeMilestone& Milestone);

	UFUNCTION(BlueprintCallable, Category = "Prestige|Registration")
	void RegisterTokenShopItem(const FMGPrestigeTokenShopItem& Item);

	// Experience
	UFUNCTION(BlueprintCallable, Category = "Prestige|Experience")
	void AddExperience(const FString& PlayerId, int64 Amount, EMGPrestigeCategory Category = EMGPrestigeCategory::Overall);

	UFUNCTION(BlueprintCallable, Category = "Prestige|Experience")
	void AddCategoryExperience(const FString& PlayerId, EMGPrestigeCategory Category, int64 Amount);

	UFUNCTION(BlueprintPure, Category = "Prestige|Experience")
	int64 GetExperienceToNextLevel(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Experience")
	float GetLevelProgress(const FString& PlayerId) const;

	// Prestige Actions
	UFUNCTION(BlueprintCallable, Category = "Prestige|Actions")
	FMGPrestigeResetResult PerformPrestige(const FString& PlayerId, EMGPrestigeResetType ResetType = EMGPrestigeResetType::Soft);

	UFUNCTION(BlueprintPure, Category = "Prestige|Actions")
	bool CanPrestige(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Actions")
	EMGPrestigeRank GetNextPrestigeRank(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Actions")
	TArray<FString> GetPrestigePreview(const FString& PlayerId) const;

	// Player Data
	UFUNCTION(BlueprintPure, Category = "Prestige|Data")
	FMGPlayerPrestige GetPlayerPrestige(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Data")
	FMGCategoryPrestige GetCategoryPrestige(const FString& PlayerId, EMGPrestigeCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Data")
	EMGPrestigeRank GetPlayerRank(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Data")
	int32 GetPlayerLevel(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Data")
	int32 GetTimesPrestiged(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Data")
	float GetPrestigeMultiplier(const FString& PlayerId) const;

	// Tokens
	UFUNCTION(BlueprintCallable, Category = "Prestige|Tokens")
	void AddPrestigeTokens(const FString& PlayerId, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Prestige|Tokens")
	bool SpendPrestigeTokens(const FString& PlayerId, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Prestige|Tokens")
	int32 GetPrestigeTokens(const FString& PlayerId) const;

	// Token Shop
	UFUNCTION(BlueprintPure, Category = "Prestige|Shop")
	TArray<FMGPrestigeTokenShopItem> GetAvailableShopItems(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Prestige|Shop")
	bool PurchaseShopItem(const FString& PlayerId, const FString& ItemId);

	UFUNCTION(BlueprintPure, Category = "Prestige|Shop")
	bool CanPurchaseShopItem(const FString& PlayerId, const FString& ItemId) const;

	// Rewards
	UFUNCTION(BlueprintPure, Category = "Prestige|Rewards")
	TArray<FMGPrestigeReward> GetAvailableRewards(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Rewards")
	TArray<FMGPrestigeReward> GetUnlockedRewards(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Prestige|Rewards")
	bool UnlockReward(const FString& PlayerId, const FString& RewardId);

	UFUNCTION(BlueprintPure, Category = "Prestige|Rewards")
	bool IsRewardUnlocked(const FString& PlayerId, const FString& RewardId) const;

	// Milestones
	UFUNCTION(BlueprintPure, Category = "Prestige|Milestones")
	TArray<FMGPrestigeMilestone> GetAllMilestones() const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Milestones")
	TArray<FMGPrestigeMilestone> GetAchievedMilestones(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Milestones")
	float GetMilestoneProgress(const FString& PlayerId, const FString& MilestoneId) const;

	// Definitions
	UFUNCTION(BlueprintPure, Category = "Prestige|Definitions")
	FMGPrestigeRankDefinition GetRankDefinition(EMGPrestigeRank Rank) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Definitions")
	TArray<FMGPrestigeRankDefinition> GetAllRankDefinitions() const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Prestige|Stats")
	FMGPrestigePlayerStats GetPlayerStats(const FString& PlayerId) const;

	// Leaderboards
	UFUNCTION(BlueprintPure, Category = "Prestige|Leaderboard")
	TArray<FMGPrestigeLeaderboardEntry> GetPrestigeLeaderboard(int32 Count = 100) const;

	UFUNCTION(BlueprintPure, Category = "Prestige|Leaderboard")
	int32 GetPlayerLeaderboardPosition(const FString& PlayerId) const;

	// Persistence
	UFUNCTION(BlueprintCallable, Category = "Prestige|Persistence")
	void SavePrestigeData();

	UFUNCTION(BlueprintCallable, Category = "Prestige|Persistence")
	void LoadPrestigeData();

protected:
	void CheckLevelUp(const FString& PlayerId);
	void CheckMilestones(const FString& PlayerId);
	void CheckPrestigeEligibility(const FString& PlayerId);
	int64 CalculateExperienceForLevel(int32 Level) const;
	float CalculatePrestigeMultiplier(EMGPrestigeRank Rank, int32 TimesPrestiged) const;
	void ApplyPrestigeReset(const FString& PlayerId, EMGPrestigeResetType ResetType);
	void UpdatePlayerStats(const FString& PlayerId);
	void UpdateLeaderboard();

private:
	UPROPERTY()
	TMap<EMGPrestigeRank, FMGPrestigeRankDefinition> RankDefinitions;

	UPROPERTY()
	TMap<FString, FMGPrestigeReward> Rewards;

	UPROPERTY()
	TMap<FString, FMGPrestigeMilestone> Milestones;

	UPROPERTY()
	TMap<FString, FMGPrestigeTokenShopItem> TokenShopItems;

	UPROPERTY()
	TMap<FString, FMGPlayerPrestige> PlayerPrestigeData;

	UPROPERTY()
	TMap<FString, TMap<EMGPrestigeCategory, FMGCategoryPrestige>> CategoryPrestigeData;

	UPROPERTY()
	TMap<FString, FMGPrestigePlayerStats> PlayerStats;

	UPROPERTY()
	TMap<FString, TSet<FString>> AchievedMilestones;

	UPROPERTY()
	TMap<FString, TMap<FString, int32>> ShopPurchaseCounts;

	UPROPERTY()
	TArray<FMGPrestigeLeaderboardEntry> Leaderboard;

	float BaseExperiencePerLevel = 1000.0f;
	float ExperienceScalingFactor = 1.15f;
};

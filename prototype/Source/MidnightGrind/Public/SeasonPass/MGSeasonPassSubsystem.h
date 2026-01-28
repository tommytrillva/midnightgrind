// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGSeasonPassSubsystem.h
 * Season Pass Subsystem - Fair, Player-Friendly Progression System
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file implements Midnight Grind's season pass system with a focus on
 * FAIR and PLAYER-FRIENDLY design. Unlike predatory implementations, this
 * system ensures meaningful progression without exploitative monetization.
 *
 * DESIGN PHILOSOPHY (READ THIS - IT'S IMPORTANT):
 * -----------------------------------------------
 * This season pass was designed to be ethical and fun. Key principles:
 *
 * 1. FREE TRACK IS MEANINGFUL
 *    - Free players get substantial rewards, not just scraps
 *    - GrindCash (earnable currency), customization items, XP boosts
 *    - You can fully enjoy the game without paying
 *
 * 2. PREMIUM IS COSMETIC ONLY
 *    - Premium track offers visual items: liveries, decals, wheels, etc.
 *    - NO gameplay advantages from premium
 *    - NO pay-to-win mechanics
 *
 * 3. XP FROM ALL ACTIVITIES
 *    - Every race, every mode, everything you do earns XP
 *    - Not locked to specific "battle pass challenges"
 *    - Play how you want, progress naturally
 *
 * 4. REASONABLE PROGRESSION
 *    - Completable with normal play (2-3 hours/week)
 *    - No need to grind 8 hours daily
 *    - Clear XP requirements, no hidden walls
 *
 * 5. NO FOMO (Fear Of Missing Out)
 *    - Seasons are long (8-12 weeks)
 *    - Catch-up mechanics for late starters
 *    - Previous season items become earnable later
 *    - If you miss something, it's not gone forever
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. SEASON INFO (FMGSeasonInfo)
 *    Contains all metadata about the current season:
 *    - SeasonID/Number: Unique identifier (e.g., "S3", Season 3)
 *    - SeasonName/Theme: Display text ("Neon Nights", "Underground")
 *    - StartDate/EndDate: When the season runs
 *    - MaxTier: Typically 100 tiers
 *    - Tiers: Array of all tier definitions with rewards
 *
 * 2. SEASON TIERS (FMGSeasonTier)
 *    Each tier (1-100) has:
 *    - TierNumber: Which tier this is
 *    - XPRequired: How much XP to reach this tier
 *    - FreeReward: What free players get
 *    - PremiumReward: What premium players also get
 *    - bIsMilestone: Is this a special tier (10, 25, 50, 100)?
 *
 * 3. REWARD TYPES (EMGSeasonRewardType)
 *    What players can earn:
 *    - GrindCash: Primary earnable currency
 *    - NeonCredits: Premium currency (also earnable slowly)
 *    - Cosmetics: Livery, Decal, Wheels, Neon, Horn, Trail
 *    - Identity: Emote, Avatar, Banner, Title
 *    - Boosts: XPBoost, CurrencyBoost (temporary multipliers)
 *
 * 4. CHALLENGES (FMGSeasonChallenge)
 *    Optional objectives that grant bonus XP:
 *    - Daily challenges: Small, quick tasks (bonus XP)
 *    - Weekly challenges: Larger goals (more XP)
 *    - NOT required to complete the pass
 *    - Just accelerators for players who want them
 *
 * 5. CATCH-UP MECHANICS
 *    For players who start late or take breaks:
 *    - GetCatchUpXPBonus(): Returns bonus XP multiplier
 *    - IsEligibleForCatchUp(): Checks if player is behind average
 *    - Automatically applies bonus to help them catch up
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *     +---------------------+
 *     | Race/Activity       |
 *     | Completion          |
 *     +---------------------+
 *              |
 *              | XP Earned
 *              v
 *     +---------------------+
 *     | UMGSeasonPassSubsystem | <-- This file
 *     | AddXP(amount, source) |
 *     +---------------------+
 *              |
 *       +------+------+
 *       |             |
 *       v             v
 *  [Tier Up?]    [Challenge
 *       |         Complete?]
 *       v             |
 *  OnSeasonTierReached|
 *       |             v
 *       v        OnChallengeCompleted
 *  [Claim Rewards]
 *       |
 *       v
 *  OnSeasonRewardClaimed
 *
 * XP SOURCES:
 * -----------
 * XP can come from many sources (tracked via AddXP's Source parameter):
 * - "Race": Completing any race
 * - "Win": Winning a race
 * - "Objective": Completing race objectives
 * - "Challenge": Completing challenges
 * - "Daily": Daily login bonus
 * - "Event": Participating in live events
 *
 * TYPICAL USAGE FLOW:
 * -------------------
 * 1. Initialize(): Loads season data and player progress
 * 2. Player completes race -> AddXP(1000, "Race")
 * 3. If XP crosses tier threshold -> CheckTierUp() triggers
 * 4. OnSeasonTierReached broadcasts with new tier number
 * 5. UI shows tier-up animation and available rewards
 * 6. Player claims via ClaimTierReward(tier, bPremium)
 * 7. GrantReward() gives the actual items/currency
 * 8. Progress saved via SaveProgress()
 *
 * PREMIUM PASS:
 * -------------
 * - Purchased once per season (1000 Neon Credits)
 * - Unlocks premium reward track
 * - Can buy mid-season and retroactively claim past rewards
 * - Does NOT give gameplay advantages
 *
 * @see UMGSeasonSubsystem - Older season implementation (being replaced)
 * @see UMGBattlePassSubsystem - Similar system with different naming
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSeasonPassSubsystem.generated.h"

/**
 * Fair Season Pass System
 * - FREE track with meaningful rewards (not just scraps)
 * - Premium track is cosmetic bonuses only
 * - XP earned through ALL activities (not just specific challenges)
 * - Reasonable progression - completable with normal play
 * - NO FOMO - seasons are long, catch-up mechanics exist
 * - Previous season items become earnable later
 */

UENUM(BlueprintType)
enum class EMGSeasonRewardType : uint8
{
	GrindCash,
	NeonCredits,
	Livery,
	Decal,
	Wheels,
	Neon,
	Horn,
	Trail,
	Emote,
	Avatar,
	Banner,
	Title,
	XPBoost,
	CurrencyBoost
};

USTRUCT(BlueprintType)
struct FMGSeasonReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSeasonRewardType Type = EMGSeasonRewardType::GrindCash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrencyAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostDurationHours = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct FMGSeasonTier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TierNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 XPRequired = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGSeasonReward FreeReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGSeasonReward PremiumReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMilestone = false;
};

USTRUCT(BlueprintType)
struct FMGSeasonChallenge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 XPReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWeekly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;
};

USTRUCT(BlueprintType)
struct FMGSeasonInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonTheme;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTier = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGSeasonTier> Tiers;
};

USTRUCT(BlueprintType)
struct FMGSeasonProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentXP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalXPEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPremiumPass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ClaimedFreeTiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ClaimedPremiumTiers;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnSeasonXPGained, int64, Amount, int64, TotalXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSeasonTierReached, int32, Tier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnSeasonRewardClaimed, int32, Tier, bool, bPremium);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnChallengeCompleted, const FMGSeasonChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPremiumPassPurchased);

UCLASS()
class MIDNIGHTGRIND_API UMGSeasonPassSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Season Info
	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	FMGSeasonInfo GetCurrentSeason() const { return CurrentSeason; }

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	FMGSeasonProgress GetProgress() const { return Progress; }

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	FTimespan GetTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	float GetSeasonProgressPercent() const;

	// XP & Progression
	UFUNCTION(BlueprintCallable, Category = "SeasonPass")
	void AddXP(int64 Amount, const FString& Source);

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	int64 GetXPForTier(int32 Tier) const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	int64 GetXPToNextTier() const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	float GetTierProgressPercent() const;

	// Rewards
	UFUNCTION(BlueprintCallable, Category = "SeasonPass")
	bool ClaimTierReward(int32 Tier, bool bPremium);

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	bool CanClaimReward(int32 Tier, bool bPremium) const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	TArray<int32> GetUnclaimedTiers(bool bPremium) const;

	UFUNCTION(BlueprintCallable, Category = "SeasonPass")
	void ClaimAllAvailableRewards();

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	FMGSeasonTier GetTierInfo(int32 Tier) const;

	// Premium Pass
	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	bool HasPremiumPass() const { return Progress.bHasPremiumPass; }

	UFUNCTION(BlueprintCallable, Category = "SeasonPass")
	bool PurchasePremiumPass();

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	int64 GetPremiumPassPrice() const { return 1000; } // 1000 Neon Credits

	// Challenges
	UFUNCTION(BlueprintPure, Category = "SeasonPass|Challenges")
	TArray<FMGSeasonChallenge> GetDailyChallenges() const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass|Challenges")
	TArray<FMGSeasonChallenge> GetWeeklyChallenges() const;

	UFUNCTION(BlueprintCallable, Category = "SeasonPass|Challenges")
	void UpdateChallengeProgress(FName ChallengeID, int32 Progress);

	// Catch-up Mechanics (anti-FOMO)
	UFUNCTION(BlueprintPure, Category = "SeasonPass|CatchUp")
	int64 GetCatchUpXPBonus() const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass|CatchUp")
	bool IsEligibleForCatchUp() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnSeasonXPGained OnSeasonXPGained;

	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnSeasonTierReached OnSeasonTierReached;

	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnSeasonRewardClaimed OnSeasonRewardClaimed;

	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnChallengeCompleted OnChallengeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnPremiumPassPurchased OnPremiumPassPurchased;

protected:
	void LoadSeasonData();
	void SaveProgress();
	void InitializeCurrentSeason();
	void GenerateChallenges();
	void CheckTierUp();
	void GrantReward(const FMGSeasonReward& Reward);

private:
	UPROPERTY()
	FMGSeasonInfo CurrentSeason;

	UPROPERTY()
	FMGSeasonProgress Progress;

	UPROPERTY()
	TArray<FMGSeasonChallenge> DailyChallenges;

	UPROPERTY()
	TArray<FMGSeasonChallenge> WeeklyChallenges;
};

// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGBattlePassSubsystem.h
 * Battle Pass Subsystem - Seasonal Progression with Free and Premium Tracks
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file implements the Battle Pass system for Midnight Grind, providing
 * seasonal progression with tiered rewards. Players earn XP through gameplay
 * to unlock rewards on both free and premium tracks. This is the most feature-
 * complete season pass implementation in the codebase.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. WHAT IS A BATTLE PASS?
 *    A battle pass is a seasonal progression system popularized by games like
 *    Fortnite. Players earn XP through gameplay to "level up" through tiers,
 *    each tier unlocking rewards. There are usually two tracks:
 *    - Free Track: Available to all players
 *    - Premium Track: Requires one-time purchase for the season
 *
 * 2. SEASONS (FMGBattlePassSeason)
 *    A season is a themed content period (typically 8-12 weeks) with:
 *    - Unique identity (name, theme, banner image)
 *    - MaxTier: Standard tiers (usually 100)
 *    - BonusTiers: Extra tiers beyond 100 for dedicated players
 *    - DailyChallenges/WeeklyChallenges: Bonus XP objectives
 *    - Pricing info for premium pass, bundles, tier skips
 *
 * 3. TIERS (FMGBattlePassTierInfo)
 *    Each tier in the pass has:
 *    - TierNumber: 1 through MaxTier (+BonusTiers)
 *    - XPRequired: How much XP to unlock this tier
 *    - CumulativeXP: Total XP needed from tier 1
 *    - TierType: Standard, Milestone (every 10), Featured, Ultimate (tier 100)
 *    - FreeReward/PremiumReward: Items for each track
 *
 * 4. REWARD TRACKS (EMGBattlePassTrack)
 *    - Free: Everyone can claim these rewards
 *    - Premium: Requires purchased premium pass
 *
 * 5. TIER TYPES (EMGBattlePassTier)
 *    - Standard: Normal tiers with regular rewards
 *    - Milestone: Every 10 tiers, better rewards
 *    - Featured: Highlighted rewards (usually cosmetics)
 *    - Ultimate: The final tier (100), best reward
 *
 * 6. REWARD TYPES (EMGRewardType)
 *    The many types of items players can earn:
 *    - Currency: GrindCash or premium currency
 *    - Vehicle: Unlockable cars
 *    - Customization: BodyKit, Vinyl, Decal, Wheels, Spoiler, Interior
 *    - Effects: Underglow, NeonKit, ExhaustEffect, TireSmoke, NitroTrail
 *    - Sound: HornSound, EngineSound
 *    - Identity: PlayerCard, ProfileBanner, Avatar, Title
 *    - Expression: Emote, VictoryPose, LoadingScreen
 *    - Boosters: XPBoost, LootBox
 *
 * 7. CHALLENGES (FMGBattlePassChallenge)
 *    Optional objectives that grant bonus XP:
 *    - Daily: Small tasks refreshing every 24 hours
 *    - Weekly: Larger goals for each week of the season
 *    - Track progress via TargetValue/CurrentValue
 *    - Must be claimed after completion to get XP
 *
 * 8. BUNDLES (FMGBattlePassBundle)
 *    Premium purchase options:
 *    - Standard premium pass
 *    - Bundles with bonus tiers and rewards
 *    - XP boost multipliers for faster progression
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *     +-------------------+     +-------------------+
 *     | Race System       |     | Challenge System  |
 *     | (completes races) |     | (objectives)      |
 *     +-------------------+     +-------------------+
 *              |                         |
 *              v                         v
 *         AddXP(amt, "Race")      UpdateChallengeProgress()
 *              |                         |
 *              +------------+------------+
 *                           |
 *                           v
 *              +------------------------+
 *              | UMGBattlePassSubsystem |  <-- This file
 *              +------------------------+
 *                           |
 *         +-----------------+-----------------+
 *         |                 |                 |
 *         v                 v                 v
 *    [Tier Up?]      [Challenge      [Reward
 *         |           Complete?]      Claimable?]
 *         v                 |                 |
 *    OnTierUp               v                 v
 *         |           OnChallengeComplete   ClaimReward()
 *         v                                   |
 *    [Show                                    v
 *     rewards]                          OnRewardClaimed
 *
 * XP CALCULATION:
 * ---------------
 * XP is earned from multiple sources:
 * - Racing: CalculateRaceXP() based on position, time, racers
 * - Challenges: Fixed XP per challenge
 * - Events: Bonus XP from live events
 * - Multipliers: GetXPMultiplier() from premium bundles
 *
 * TYPICAL USAGE FLOW:
 * -------------------
 * 1. Initialize(): Sets up season data, loads player progress
 * 2. Player completes race -> AddXP(xp, "Race") called
 * 3. CheckTierProgression() sees if new tier reached
 * 4. If tier up -> OnTierUp broadcasts, UI shows animation
 * 5. Player views battle pass -> GetProgress(), GetAllTiers()
 * 6. Player claims reward -> ClaimReward(tier, track)
 * 7. OnRewardClaimed broadcasts, item added to inventory
 *
 * CHALLENGE FLOW:
 * ---------------
 * 1. GenerateDailyChallenges() creates daily objectives
 * 2. Player races -> UpdateChallengesByType("Race", 1)
 * 3. Challenge progress updates -> OnChallengeProgress broadcasts
 * 4. When complete -> OnChallengeComplete broadcasts
 * 5. Player claims -> ClaimChallengeReward(id) adds XP
 * 6. Daily challenges refresh at midnight UTC
 *
 * PREMIUM FEATURES:
 * -----------------
 * - PurchasePremium(): Buy standard premium pass
 * - PurchaseBundle(): Buy premium + bonus tiers + rewards
 * - PurchaseTiers(): Skip tiers with premium currency
 * - HasPremium(): Check if player has premium access
 *
 * DELEGATES (Events to listen to):
 * --------------------------------
 * - OnTierUp: New tier reached, show rewards
 * - OnXPGained: XP earned, update progress bar
 * - OnRewardClaimed: Reward collected, play animation
 * - OnPremiumPurchased: Premium unlocked, refresh UI
 * - OnChallengeComplete: Challenge done, show claim button
 * - OnChallengeProgress: Progress updated, refresh UI
 * - OnSeasonStarted/Ended: Season lifecycle events
 * - OnChallengesRefreshed: New daily challenges available
 *
 * IMPLEMENTATION NOTES:
 * ---------------------
 * - This is a GameInstanceSubsystem (persists across levels)
 * - Progress is saved locally and synced to server
 * - Uses FTimerHandle for daily challenge refresh
 * - Rewards use TSoftObjectPtr for lazy loading (memory efficient)
 *
 * @see UMGSeasonPassSubsystem - Simpler implementation with same concepts
 * @see UMGSeasonSubsystem - Older implementation (being deprecated)
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBattlePassSubsystem.generated.h"

// Forward declarations
class UMGBattlePassSubsystem;

/**
 * EMGBattlePassTrack - Battle pass reward track type
 */
UENUM(BlueprintType)
enum class EMGBattlePassTrack : uint8
{
    Free        UMETA(DisplayName = "Free Track"),
    Premium     UMETA(DisplayName = "Premium Track")
};

/**
 * EMGBattlePassTier - Special tier types
 */
UENUM(BlueprintType)
enum class EMGBattlePassTier : uint8
{
    Standard    UMETA(DisplayName = "Standard"),
    Milestone   UMETA(DisplayName = "Milestone"),
    Featured    UMETA(DisplayName = "Featured"),
    Ultimate    UMETA(DisplayName = "Ultimate")
};

/**
 * EMGRewardType - Types of battle pass rewards
 */
UENUM(BlueprintType)
enum class EMGRewardType : uint8
{
    Currency        UMETA(DisplayName = "Currency"),
    Vehicle         UMETA(DisplayName = "Vehicle"),
    BodyKit         UMETA(DisplayName = "Body Kit"),
    Vinyl           UMETA(DisplayName = "Vinyl"),
    Decal           UMETA(DisplayName = "Decal"),
    Wheels          UMETA(DisplayName = "Wheels"),
    Spoiler         UMETA(DisplayName = "Spoiler"),
    Interior        UMETA(DisplayName = "Interior"),
    Underglow       UMETA(DisplayName = "Underglow"),
    NeonKit         UMETA(DisplayName = "Neon Kit"),
    HornSound       UMETA(DisplayName = "Horn Sound"),
    EngineSound     UMETA(DisplayName = "Engine Sound"),
    ExhaustEffect   UMETA(DisplayName = "Exhaust Effect"),
    TireSmoke       UMETA(DisplayName = "Tire Smoke"),
    NitroTrail      UMETA(DisplayName = "Nitro Trail"),
    PlayerCard      UMETA(DisplayName = "Player Card"),
    ProfileBanner   UMETA(DisplayName = "Profile Banner"),
    Avatar          UMETA(DisplayName = "Avatar"),
    Title           UMETA(DisplayName = "Title"),
    Emote           UMETA(DisplayName = "Emote"),
    VictoryPose     UMETA(DisplayName = "Victory Pose"),
    LoadingScreen   UMETA(DisplayName = "Loading Screen"),
    XPBoost         UMETA(DisplayName = "XP Boost"),
    LootBox         UMETA(DisplayName = "Loot Box")
};

/**
 * FMGBattlePassReward - A single battle pass reward
 */
USTRUCT(BlueprintType)
struct FMGBattlePassReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RewardId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGBattlePassTrack Track;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Tier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RarityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> IconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UObject> RewardAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsClaimed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsNew;

    FMGBattlePassReward()
        : RewardId(NAME_None)
        , RewardType(EMGRewardType::Currency)
        , Track(EMGBattlePassTrack::Free)
        , Tier(1)
        , Quantity(1)
        , RarityLevel(FName("Common"))
        , bIsClaimed(false)
        , bIsNew(true)
    {}
};

/**
 * FMGBattlePassTierInfo - Information about a battle pass tier
 */
USTRUCT(BlueprintType)
struct FMGBattlePassTierInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TierNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 XPRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CumulativeXP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGBattlePassTier TierType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGBattlePassReward FreeReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGBattlePassReward PremiumReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFreeRewardAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPremiumRewardAvailable;

    FMGBattlePassTierInfo()
        : TierNumber(1)
        , XPRequired(1000)
        , CumulativeXP(0)
        , TierType(EMGBattlePassTier::Standard)
        , bFreeRewardAvailable(true)
        , bPremiumRewardAvailable(true)
    {}
};

/**
 * FMGBattlePassChallenge - A challenge that awards battle pass XP
 */
USTRUCT(BlueprintType)
struct FMGBattlePassChallenge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ChallengeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 XPReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ChallengeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsWeekly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WeekNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsClaimed;

    FMGBattlePassChallenge()
        : ChallengeId(NAME_None)
        , TargetValue(1.0f)
        , CurrentValue(0.0f)
        , XPReward(500)
        , ChallengeType(FName("Race"))
        , bIsWeekly(false)
        , WeekNumber(1)
        , bIsComplete(false)
        , bIsClaimed(false)
    {}

    float GetProgress() const
    {
        return TargetValue > 0.0f ? FMath::Clamp(CurrentValue / TargetValue, 0.0f, 1.0f) : 0.0f;
    }
};

/**
 * FMGBattlePassSeason - A complete battle pass season
 */
USTRUCT(BlueprintType)
struct FMGBattlePassSeason
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SeasonId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SeasonName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SeasonTheme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SeasonNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BonusTiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGBattlePassTierInfo> Tiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGBattlePassChallenge> DailyChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGBattlePassChallenge> WeeklyChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PremiumPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BundlePrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TierSkipPrice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> SeasonBanner;

    FMGBattlePassSeason()
        : SeasonNumber(1)
        , MaxTier(100)
        , BonusTiers(20)
        , PremiumPrice(950)
        , BundlePrice(2500)
        , TierSkipPrice(150)
    {}

    FTimespan GetTimeRemaining() const
    {
        FDateTime Now = FDateTime::Now();
        return EndDate > Now ? EndDate - Now : FTimespan::Zero();
    }

    bool IsActive() const
    {
        FDateTime Now = FDateTime::Now();
        return Now >= StartDate && Now <= EndDate;
    }
};

/**
 * FMGBattlePassProgress - Player's progress in battle pass
 */
USTRUCT(BlueprintType)
struct FMGBattlePassProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SeasonId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentXP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalXPEarned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasPremium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> ClaimedFreeTiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> ClaimedPremiumTiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TiersPurchased;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime PremiumPurchaseDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastPlayDate;

    FMGBattlePassProgress()
        : CurrentTier(1)
        , CurrentXP(0)
        , TotalXPEarned(0)
        , bHasPremium(false)
        , TiersPurchased(0)
    {}

    float GetTierProgress(int32 XPForCurrentTier) const
    {
        return XPForCurrentTier > 0 ? static_cast<float>(CurrentXP) / XPForCurrentTier : 0.0f;
    }
};

/**
 * FMGBattlePassBundle - Premium bundle option
 */
USTRUCT(BlueprintType)
struct FMGBattlePassBundle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BundleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Price;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BonusTiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGBattlePassReward> BonusRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float XPBoostMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> BundleImage;

    FMGBattlePassBundle()
        : BundleId(NAME_None)
        , Price(2500)
        , BonusTiers(25)
        , XPBoostMultiplier(1.0f)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnBattlePassTierUp, int32, NewTier, const TArray<FMGBattlePassReward>&, AvailableRewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnBattlePassXPGained, int32, XPGained, int32, TotalXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnBattlePassRewardClaimed, const FMGBattlePassReward&, Reward, EMGBattlePassTrack, Track);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnBattlePassPremiumPurchased);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBattlePassChallengeComplete, const FMGBattlePassChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMGOnBattlePassChallengeProgress, FName, ChallengeId, float, Progress, float, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBattlePassSeasonStarted, const FMGBattlePassSeason&, Season);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBattlePassSeasonEnded, const FMGBattlePassSeason&, Season);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnBattlePassChallengesRefreshed);

/**
 * UMGBattlePassSubsystem
 *
 * Manages the battle pass system for Midnight Grind.
 * Features include:
 * - Seasonal progression tracks
 * - Free and premium reward tiers
 * - Daily and weekly challenges
 * - XP progression system
 * - Tier purchasing
 * - Bonus tiers beyond max level
 */
UCLASS()
class MIDNIGHTGRIND_API UMGBattlePassSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGBattlePassSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ===== Season Access =====

    UFUNCTION(BlueprintPure, Category = "BattlePass|Season")
    FMGBattlePassSeason GetCurrentSeason() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Season")
    bool IsSeasonActive() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Season")
    FTimespan GetSeasonTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Season")
    int32 GetCurrentWeekNumber() const;

    // ===== Progress =====

    UFUNCTION(BlueprintPure, Category = "BattlePass|Progress")
    FMGBattlePassProgress GetProgress() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Progress")
    int32 GetCurrentTier() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Progress")
    int32 GetCurrentXP() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Progress")
    int32 GetXPForNextTier() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Progress")
    float GetTierProgress() const;

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Progress")
    void AddXP(int32 Amount, FName Source);

    // ===== Tiers =====

    UFUNCTION(BlueprintPure, Category = "BattlePass|Tiers")
    FMGBattlePassTierInfo GetTierInfo(int32 TierNumber) const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Tiers")
    TArray<FMGBattlePassTierInfo> GetAllTiers() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Tiers")
    TArray<FMGBattlePassTierInfo> GetTiersInRange(int32 StartTier, int32 EndTier) const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Tiers")
    int32 GetMaxTier() const;

    // ===== Rewards =====

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Rewards")
    bool ClaimReward(int32 TierNumber, EMGBattlePassTrack Track);

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Rewards")
    TArray<FMGBattlePassReward> ClaimAllAvailableRewards();

    UFUNCTION(BlueprintPure, Category = "BattlePass|Rewards")
    TArray<FMGBattlePassReward> GetUnclaimedRewards() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Rewards")
    bool HasUnclaimedRewards() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Rewards")
    bool IsRewardClaimable(int32 TierNumber, EMGBattlePassTrack Track) const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Rewards")
    TArray<FMGBattlePassReward> GetFeaturedRewards() const;

    // ===== Challenges =====

    UFUNCTION(BlueprintPure, Category = "BattlePass|Challenges")
    TArray<FMGBattlePassChallenge> GetDailyChallenges() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Challenges")
    TArray<FMGBattlePassChallenge> GetWeeklyChallenges() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Challenges")
    TArray<FMGBattlePassChallenge> GetActiveChallenges() const;

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Challenges")
    void UpdateChallengeProgress(FName ChallengeId, float Progress);

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Challenges")
    void UpdateChallengesByType(FName ChallengeType, float Progress);

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Challenges")
    bool ClaimChallengeReward(FName ChallengeId);

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Challenges")
    void RefreshDailyChallenges();

    // ===== Premium =====

    UFUNCTION(BlueprintPure, Category = "BattlePass|Premium")
    bool HasPremium() const;

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Premium")
    bool PurchasePremium();

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Premium")
    bool PurchaseBundle(FName BundleId);

    UFUNCTION(BlueprintCallable, Category = "BattlePass|Premium")
    bool PurchaseTiers(int32 TierCount);

    UFUNCTION(BlueprintPure, Category = "BattlePass|Premium")
    TArray<FMGBattlePassBundle> GetAvailableBundles() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Premium")
    int32 GetPremiumPrice() const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|Premium")
    int32 GetTierSkipPrice() const;

    // ===== XP Calculation =====

    UFUNCTION(BlueprintPure, Category = "BattlePass|XP")
    int32 CalculateRaceXP(int32 Position, int32 TotalRacers, float RaceTime) const;

    UFUNCTION(BlueprintPure, Category = "BattlePass|XP")
    float GetXPMultiplier() const;

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassTierUp OnTierUp;

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassXPGained OnXPGained;

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassRewardClaimed OnRewardClaimed;

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassPremiumPurchased OnPremiumPurchased;

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassChallengeComplete OnChallengeComplete;

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassChallengeProgress OnChallengeProgress;

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassSeasonStarted OnSeasonStarted;

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassSeasonEnded OnSeasonEnded;

    UPROPERTY(BlueprintAssignable, Category = "BattlePass|Events")
    FMGOnBattlePassChallengesRefreshed OnChallengesRefreshed;

protected:
    void InitializeSampleSeason();
    void GenerateDailyChallenges();
    void GenerateWeeklyChallenges(int32 WeekNumber);
    void CheckTierProgression();
    int32 CalculateXPForTier(int32 TierNumber) const;

private:
    UPROPERTY()
    FMGBattlePassSeason CurrentSeason;

    UPROPERTY()
    FMGBattlePassProgress PlayerProgress;

    UPROPERTY()
    TArray<FMGBattlePassBundle> AvailableBundles;

    UPROPERTY()
    float XPMultiplier;

    UPROPERTY()
    FDateTime LastDailyChallengeRefresh;

    FTimerHandle DailyChallengeTimerHandle;
};

// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "EventCalendar/MGEventCalendarSubsystem.h"
/**
 * =============================================================================
 * MGDailyLoginSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the comprehensive Daily Login system for Midnight Grind.
 * While MGDailyRewardsSubsystem handles basic daily rewards, THIS subsystem
 * provides a more feature-rich engagement system including:
 * - Daily login tracking and rewards
 * - Daily CHALLENGES (tasks to complete for bonus rewards)
 * - Weekly bonuses (special effects like Double XP)
 * - Return player bonuses (welcome back rewards for players who were away)
 * - Tiered reward systems based on login consistency
 *
 * Think of this as the "engagement hub" - it's designed to give players
 * multiple reasons to log in daily beyond just collecting a reward.
 *
 * KEY CONCEPTS:
 * -------------
 * 1. DAILY CHALLENGES: Small tasks players can complete each day for rewards.
 *    Examples: "Win 3 races", "Drift for 1000 points", "Use nitro 50 times"
 *    These refresh every 24 hours and keep gameplay interesting.
 *
 * 2. REWARD TIERS (EMGDailyRewardTier): Players progress through tiers based
 *    on their login consistency. Higher tiers = better base rewards.
 *    Bronze -> Silver -> Gold -> Platinum -> Diamond -> Champion
 *    This rewards long-term dedicated players.
 *
 * 3. WEEKLY BONUSES: Time-limited effects that enhance gameplay.
 *    Examples: Double XP, Double Currency, Rare Drop Boost
 *    These create "special" feeling days that players look forward to.
 *
 * 4. RETURN PLAYER BONUS: If a player hasn't logged in for several days,
 *    they get a "Welcome Back" bonus to re-engage them. This helps prevent
 *    churn (players leaving permanently).
 *
 * 5. MONTHLY CALENDAR: A visual calendar showing what rewards are available
 *    each day of the month. Reaching the end grants completion bonuses.
 *
 * 6. CHALLENGE TYPES (EMGDailyChallengeType): Various gameplay objectives:
 *    - WinRaces: Win a certain number of races
 *    - DriftScore: Accumulate drift points
 *    - NearMisses: Narrowly avoid obstacles
 *    - Takedowns: Knock out opponents
 *    - And many more...
 *
 * ARCHITECTURE:
 * -------------
 * UGameInstanceSubsystem - Singleton that persists for the entire game session.
 *
 * Key Data Structures:
 * - FMGDailyReward: A single reward item (currency, parts, cosmetics, etc.)
 * - FMGDailyChallenge: A challenge with objectives, progress, and rewards
 * - FMGLoginStreak: Player's streak data including tier and history
 * - FMGWeeklyBonus: Active bonus effects with duration tracking
 * - FMGCalendarDay: One day in the monthly calendar
 * - FMGMonthlyCalendar: The full month's reward layout
 * - FMGReturnPlayerBonus: Special bonuses for returning players
 * - FMGDailyLoginState: Complete state snapshot of the entire system
 *
 * TYPICAL WORKFLOW:
 * -----------------
 * 1. Player logs in -> ProcessLogin() called
 * 2. System checks: Is this a new day? First login today?
 * 3. If returning player after absence -> Grant return bonus
 * 4. Update streak, check for tier upgrades
 * 5. Generate new daily challenges if needed
 * 6. UI displays available rewards and challenges
 * 7. Player claims daily reward -> ClaimDailyReward()
 * 8. During gameplay, challenge progress updates via UpdateChallengeProgress()
 * 9. When challenge complete -> ClaimChallengeReward()
 *
 * DELEGATES (Events):
 * -------------------
 * - OnDailyLoginClaimed: Daily reward was claimed
 * - OnStreakUpdated: Streak changed (could be increase or reset)
 * - OnStreakLost: Player lost their streak (missed a day)
 * - OnChallengeProgressUpdated: Challenge made progress
 * - OnChallengeCompleted: A challenge was finished
 * - OnWeeklyBonusActivated/Expired: Bonus effects started/ended
 * - OnTierUpgrade: Player reached a new reward tier
 *
 * WHY TWO DAILY SYSTEMS?
 * ----------------------
 * MGDailyRewardsSubsystem: Simple, lightweight daily rewards
 * MGDailyLoginSubsystem: Full-featured engagement with challenges and tiers
 *
 * Games often have both - one for quick basic rewards, one for deeper engagement.
 * They can work together or be used independently based on game design needs.
 *
 * =============================================================================
 */

// MidnightGrind - Arcade Street Racing Game
// Daily Login Subsystem - Login rewards, streaks, and daily challenges

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "DailyRewards/MGDailyRewardsSubsystem.h" // FMGDailyReward, EMGStreakMilestone (canonical)
#include "MGDailyLoginSubsystem.generated.h"

// Forward declarations
class UMGDailyLoginSubsystem;

/**
 * EMGDailyRewardTier - Tiers for daily rewards based on streak
 */
UENUM(BlueprintType)
enum class EMGDailyRewardTier : uint8
{
    Bronze          UMETA(DisplayName = "Bronze"),
    Silver          UMETA(DisplayName = "Silver"),
    Gold            UMETA(DisplayName = "Gold"),
    Platinum        UMETA(DisplayName = "Platinum"),
    Diamond         UMETA(DisplayName = "Diamond"),
    Champion        UMETA(DisplayName = "Champion")
};

/**
 * EMGDailyRewardType - Types of rewards from daily login
 */
UENUM(BlueprintType)
enum class EMGDailyRewardType : uint8
{
    Currency        UMETA(DisplayName = "Currency"),
    PremiumCurrency UMETA(DisplayName = "Premium Currency"),
    Experience      UMETA(DisplayName = "Experience"),
    Part            UMETA(DisplayName = "Part"),
    Cosmetic        UMETA(DisplayName = "Cosmetic"),
    LootBox         UMETA(DisplayName = "Loot Box"),
    BoostToken      UMETA(DisplayName = "Boost Token"),
    RepairKit       UMETA(DisplayName = "Repair Kit"),
    Nitro           UMETA(DisplayName = "Nitro"),
    VehicleRental   UMETA(DisplayName = "Vehicle Rental"),
    ExclusiveItem   UMETA(DisplayName = "Exclusive Item"),
    MysteryBox      UMETA(DisplayName = "Mystery Box")
};

/**
 * EMGDailyChallengeType - Types of daily challenges
 */
UENUM(BlueprintType)
enum class EMGDailyChallengeType : uint8
{
    WinRaces        UMETA(DisplayName = "Win Races"),
    CompleteRaces   UMETA(DisplayName = "Complete Races"),
    DriftScore      UMETA(DisplayName = "Drift Score"),
    AirtimeSeconds  UMETA(DisplayName = "Airtime Seconds"),
    NearMisses      UMETA(DisplayName = "Near Misses"),
    Takedowns       UMETA(DisplayName = "Takedowns"),
    PerfectLaps     UMETA(DisplayName = "Perfect Laps"),
    TopSpeedReach   UMETA(DisplayName = "Top Speed Reach"),
    NitroUsage      UMETA(DisplayName = "Nitro Usage"),
    DistanceDriven  UMETA(DisplayName = "Distance Driven"),
    ChainScore      UMETA(DisplayName = "Chain Score"),
    Overtakes       UMETA(DisplayName = "Overtakes"),
    FirstPlaces     UMETA(DisplayName = "First Places"),
    UseSpecificCar  UMETA(DisplayName = "Use Specific Car"),
    RaceOnTrack     UMETA(DisplayName = "Race On Track")
};

// EMGChallengeDifficulty - REMOVED (duplicate)
// Canonical definition in: Challenges/MGChallengeSubsystem.h

/**
 * EMGWeeklyBonusType - Types of weekly bonuses
 */
UENUM(BlueprintType)
enum class EMGWeeklyBonusType : uint8
{
    DoubleXP        UMETA(DisplayName = "Double XP"),
    DoubleCurrency  UMETA(DisplayName = "Double Currency"),
    BonusNitro      UMETA(DisplayName = "Bonus Nitro"),
    FreeParts       UMETA(DisplayName = "Free Parts"),
    DiscountShop    UMETA(DisplayName = "Discount Shop"),
    ExtraRewards    UMETA(DisplayName = "Extra Rewards"),
    RareDropBoost   UMETA(DisplayName = "Rare Drop Boost")
};

// FMGDailyReward - REMOVED (duplicate)
// Canonical definition in: DailyRewards/MGDailyRewardsSubsystem.h

/**
 * FMGDailyChallenge - A daily challenge with objectives
 */
USTRUCT(BlueprintType)
struct FMGDailyChallenge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ChallengeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGDailyChallengeType ChallengeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGChallengeDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGDailyReward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsClaimed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredVehicleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredTrackId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BonusXP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BonusCurrency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> ChallengeIcon;

    FMGDailyChallenge()
        : ChallengeId(NAME_None)
        , ChallengeType(EMGDailyChallengeType::WinRaces)
        , Difficulty(EMGChallengeDifficulty::Easy)
        , TargetValue(1.0f)
        , CurrentValue(0.0f)
        , bIsComplete(false)
        , bIsClaimed(false)
        , RequiredVehicleId(NAME_None)
        , RequiredTrackId(NAME_None)
        , BonusXP(100)
        , BonusCurrency(500)
    {}

    float GetProgressPercent() const
    {
        return TargetValue > 0.0f ? FMath::Clamp(CurrentValue / TargetValue, 0.0f, 1.0f) : 0.0f;
    }

    int32 GetDifficultyMultiplier() const
    {
        switch (Difficulty)
        {
            case EMGChallengeDifficulty::Easy: return 1;
            case EMGChallengeDifficulty::Medium: return 2;
            case EMGChallengeDifficulty::Hard: return 3;
            case EMGChallengeDifficulty::Expert: return 5;
            case EMGChallengeDifficulty::Insane: return 10;
            default: return 1;
        }
    }
};

/**
 * FMGLoginStreak - Tracks player's login streak data
 */
USTRUCT(BlueprintType)
struct FMGLoginStreak
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStreak;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LongestStreak;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalLoginDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastLoginDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime FirstLoginDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StreakStartDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bClaimedToday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WeeklyLoginCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MonthlyLoginCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGDailyRewardTier CurrentTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DaysUntilNextTier;

    FMGLoginStreak()
        : CurrentStreak(0)
        , LongestStreak(0)
        , TotalLoginDays(0)
        , bClaimedToday(false)
        , WeeklyLoginCount(0)
        , MonthlyLoginCount(0)
        , CurrentTier(EMGDailyRewardTier::Bronze)
        , DaysUntilNextTier(7)
    {}

    bool IsStreakActive() const
    {
        FDateTime Now = FDateTime::Now();
        FDateTime Yesterday = Now - FTimespan::FromDays(1);
        return LastLoginDate.GetDay() == Yesterday.GetDay() ||
               LastLoginDate.GetDay() == Now.GetDay();
    }
};

/**
 * FMGWeeklyBonus - Active weekly bonus effects
 */
USTRUCT(BlueprintType)
struct FMGWeeklyBonus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName BonusId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGWeeklyBonusType BonusType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Multiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> BonusIcon;

    FMGWeeklyBonus()
        : BonusId(NAME_None)
        , BonusType(EMGWeeklyBonusType::DoubleXP)
        , Multiplier(2.0f)
        , bIsActive(false)
    {}

    FTimespan GetTimeRemaining() const
    {
        FDateTime Now = FDateTime::Now();
        return EndDate > Now ? EndDate - Now : FTimespan::Zero();
    }

    bool IsExpired() const
    {
        return FDateTime::Now() > EndDate;
    }
};

// MOVED TO EventCalendar/MGEventCalendarSubsystem.h
// FMGCalendarDay is now defined in the canonical event calendar subsystem header.
// Include "EventCalendar/MGEventCalendarSubsystem.h" to use FMGCalendarDay.
// NOTE: The DailyLogin version had different fields (DayNumber, Rewards, bIsClaimed, etc.)
// Consider renaming to FMGLoginCalendarDay if distinct functionality is needed.
/*
USTRUCT(BlueprintType)
struct FMGCalendarDay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DayNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Date;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGDailyReward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsClaimed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsToday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMilestoneDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsWeekendBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SpecialEventName;

    FMGCalendarDay()
        : DayNumber(1)
        , bIsClaimed(false)
        , bIsToday(false)
        , bIsMilestoneDay(false)
        , bIsWeekendBonus(false)
    {}

    bool HasSpecialEvent() const
    {
        return !SpecialEventName.IsEmpty();
    }
};
*/

/**
 * FMGMonthlyCalendar - A full month's login calendar
 */
USTRUCT(BlueprintType)
struct FMGMonthlyCalendar
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Month;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Year;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText MonthName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGCalendarDay> Days;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGDailyReward> MonthlyCompletionRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DaysClaimed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMonthComplete;

    FMGMonthlyCalendar()
        : Month(1)
        , Year(2024)
        , DaysClaimed(0)
        , TotalDays(30)
        , bMonthComplete(false)
    {}

    float GetCompletionPercent() const
    {
        return TotalDays > 0 ? static_cast<float>(DaysClaimed) / TotalDays : 0.0f;
    }
};

/**
 * FMGReturnPlayerBonus - Bonus for players returning after absence
 */
USTRUCT(BlueprintType)
struct FMGReturnPlayerBonus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DaysAbsent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGDailyReward> WelcomeBackRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float XPMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrencyMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BonusDurationHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime BonusExpiresAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FMGReturnPlayerBonus()
        : DaysAbsent(0)
        , XPMultiplier(1.5f)
        , CurrencyMultiplier(1.5f)
        , BonusDurationHours(24)
        , bIsActive(false)
    {}

    bool IsBonusExpired() const
    {
        return FDateTime::Now() > BonusExpiresAt;
    }
};

/**
 * FMGDailyLoginState - Complete state of daily login system
 */
USTRUCT(BlueprintType)
struct FMGDailyLoginState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGLoginStreak Streak;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGMonthlyCalendar Calendar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGDailyChallenge> DailyChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGWeeklyBonus> ActiveBonuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGReturnPlayerBonus ReturnBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGDailyReward> PendingRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastRefreshTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime NextRefreshTime;

    FMGDailyLoginState()
    {}

    FTimespan GetTimeUntilRefresh() const
    {
        FDateTime Now = FDateTime::Now();
        return NextRefreshTime > Now ? NextRefreshTime - Now : FTimespan::Zero();
    }
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDailyLoginClaimed, const TArray<FMGDailyReward>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnStreakUpdated, int32, NewStreak, EMGDailyRewardTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnStreakLost, int32, PreviousStreak);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnChallengeProgressUpdated, FName, ChallengeId, float, NewProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnChallengeCompleted, FName, ChallengeId, const TArray<FMGDailyReward>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnWeeklyBonusActivated, const FMGWeeklyBonus&, Bonus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnWeeklyBonusExpired, FName, BonusId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnDailyRefresh);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnReturnPlayerBonusGranted, const FMGReturnPlayerBonus&, Bonus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMonthlyCalendarComplete, const TArray<FMGDailyReward>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTierUpgrade, EMGDailyRewardTier, NewTier);

/**
 * UMGDailyLoginSubsystem
 *
 * Manages the daily login rewards and engagement systems for Midnight Grind.
 * Features include:
 * - Daily login reward calendar
 * - Login streak tracking and bonuses
 * - Daily challenges with objectives
 * - Weekly bonus events
 * - Return player bonuses
 * - Tier-based reward scaling
 * - Monthly completion rewards
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDailyLoginSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGDailyLoginSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Tick functionality
    void TickDailyLoginSystem(float MGDeltaTime);

    // ===== Login Management =====

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Login")
    void ProcessLogin();

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Login")
    bool ClaimDailyReward(TArray<FMGDailyReward>& OutRewards);

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Login")
    bool CanClaimDailyReward() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Login")
    TArray<FMGDailyReward> GetTodaysRewards() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Login")
    FTimespan GetTimeUntilNextClaim() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Login")
    bool IsFirstLoginToday() const;

    // ===== Streak Management =====

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Streak")
    FMGLoginStreak GetLoginStreak() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Streak")
    int32 GetCurrentStreak() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Streak")
    int32 GetLongestStreak() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Streak")
    EMGDailyRewardTier GetCurrentTier() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Streak")
    int32 GetDaysUntilNextTier() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Streak")
    float GetTierProgress() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Streak")
    TArray<FMGDailyReward> GetStreakMilestoneRewards(int32 StreakDay) const;

    // ===== Calendar =====

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Calendar")
    FMGMonthlyCalendar GetCurrentCalendar() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Calendar")
    FMGCalendarDay GetCalendarDay(int32 DayNumber) const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Calendar")
    TArray<FMGCalendarDay> GetUpcomingDays(int32 Count) const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Calendar")
    float GetMonthlyCompletionProgress() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Calendar")
    bool IsMonthComplete() const;

    // ===== Daily Challenges =====

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Challenges")
    TArray<FMGDailyChallenge> GetDailyChallenges() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Challenges")
    FMGDailyChallenge GetChallenge(FName ChallengeId) const;

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Challenges")
    void UpdateChallengeProgress(FName ChallengeId, float NewValue);

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Challenges")
    void IncrementChallengeProgress(FName ChallengeId, float Amount);

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Challenges")
    void UpdateChallengesByType(EMGDailyChallengeType ChallengeType, float Value);

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Challenges")
    bool ClaimChallengeReward(FName ChallengeId, TArray<FMGDailyReward>& OutRewards);

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Challenges")
    bool ClaimAllChallengeRewards(TArray<FMGDailyReward>& OutRewards);

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Challenges")
    int32 GetCompletedChallengeCount() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Challenges")
    int32 GetUnclaimedChallengeCount() const;

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Challenges")
    void RerollChallenge(FName ChallengeId);

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Challenges")
    int32 GetRemainingRerolls() const;

    // ===== Weekly Bonuses =====

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Weekly")
    TArray<FMGWeeklyBonus> GetActiveBonuses() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Weekly")
    bool HasActiveBonus(EMGWeeklyBonusType BonusType) const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Weekly")
    float GetBonusMultiplier(EMGWeeklyBonusType BonusType) const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Weekly")
    FTimespan GetBonusTimeRemaining(EMGWeeklyBonusType BonusType) const;

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Weekly")
    void ActivateWeeklyBonus(const FMGWeeklyBonus& Bonus);

    // ===== Return Player =====

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Return")
    bool IsReturnPlayer() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Return")
    FMGReturnPlayerBonus GetReturnBonus() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|Return")
    bool HasActiveReturnBonus() const;

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Return")
    bool ClaimReturnBonus(TArray<FMGDailyReward>& OutRewards);

    // ===== State =====

    UFUNCTION(BlueprintPure, Category = "DailyLogin|State")
    FMGDailyLoginState GetLoginState() const;

    UFUNCTION(BlueprintPure, Category = "DailyLogin|State")
    FTimespan GetTimeUntilDailyRefresh() const;

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|State")
    void ForceRefresh();

    // ===== Persistence =====

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Persistence")
    void SaveLoginData();

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Persistence")
    void LoadLoginData();

    // ===== Debug =====

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Debug")
    void DebugSetStreak(int32 NewStreak);

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Debug")
    void DebugAdvanceDay();

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Debug")
    void DebugResetAll();

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Debug")
    void DebugGrantReturnBonus(int32 DaysAbsent);

    UFUNCTION(BlueprintCallable, Category = "DailyLogin|Debug")
    void DebugCompleteAllChallenges();

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnDailyLoginClaimed OnDailyLoginClaimed;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnStreakUpdated OnStreakUpdated;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnStreakLost OnStreakLost;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnChallengeProgressUpdated OnChallengeProgressUpdated;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnChallengeCompleted OnChallengeCompleted;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnWeeklyBonusActivated OnWeeklyBonusActivated;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnWeeklyBonusExpired OnWeeklyBonusExpired;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnDailyRefresh OnDailyRefresh;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnReturnPlayerBonusGranted OnReturnPlayerBonusGranted;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnMonthlyCalendarComplete OnMonthlyCalendarComplete;

    UPROPERTY(BlueprintAssignable, Category = "DailyLogin|Events")
    FMGOnTierUpgrade OnTierUpgrade;

protected:
    // Internal helpers
    void RefreshDailyChallenges();
    void GenerateMonthlyCalendar();
    void UpdateStreakStatus();
    void ProcessReturnPlayer();
    void UpdateTier();
    void CheckBonusExpiration();
    void CalculateNextRefreshTime();
    FMGDailyChallenge GenerateRandomChallenge(EMGChallengeDifficulty Difficulty);
    TArray<FMGDailyReward> GenerateDayRewards(int32 DayNumber);
    EMGDailyRewardTier CalculateTierForStreak(int32 Streak) const;
    int32 GetTierThreshold(EMGDailyRewardTier Tier) const;

private:
    // Login state
    UPROPERTY()
    FMGDailyLoginState LoginState;

    // Configuration
    UPROPERTY()
    int32 MaxDailyChallenges;

    UPROPERTY()
    int32 MaxDailyRerolls;

    UPROPERTY()
    int32 RemainingRerolls;

    UPROPERTY()
    int32 ReturnPlayerThresholdDays;

    // Tick timer handle
    FTimerHandle TickTimerHandle;

    // Daily refresh tracking
    FDateTime LastCheckDate;

    // Dirty flag for persistence
    bool bDataDirty;

    // Configuration values
    static constexpr int32 STREAK_MILESTONE_DAYS[] = { 7, 14, 30, 60, 90, 180, 365 };
    static constexpr int32 NUM_STREAK_MILESTONES = 7;
};

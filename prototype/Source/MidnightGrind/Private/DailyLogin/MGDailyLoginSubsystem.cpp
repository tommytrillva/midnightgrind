// MidnightGrind - Arcade Street Racing Game
// Daily Login Subsystem Implementation

#include "DailyLogin/MGDailyLoginSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UMGDailyLoginSubsystem::UMGDailyLoginSubsystem()
    : MaxDailyChallenges(3)
    , MaxDailyRerolls(1)
    , RemainingRerolls(1)
    , ReturnPlayerThresholdDays(7)
    , bDataDirty(false)
{
}

void UMGDailyLoginSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Load saved data
    LoadLoginData();

    // Calculate next refresh time
    CalculateNextRefreshTime();

    // Generate calendar if needed
    FDateTime Now = FDateTime::Now();
    if (LoginState.Calendar.Month != Now.GetMonth() || LoginState.Calendar.Year != Now.GetYear())
    {
        GenerateMonthlyCalendar();
    }

    // Start tick timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            [this]() { TickDailyLoginSystem(1.0f); },
            1.0f,
            true
        );
    }

    // Process login on initialization
    ProcessLogin();

    UE_LOG(LogTemp, Log, TEXT("MGDailyLoginSubsystem initialized. Current streak: %d"), LoginState.Streak.CurrentStreak);
}

void UMGDailyLoginSubsystem::Deinitialize()
{
    // Save data on shutdown
    if (bDataDirty)
    {
        SaveLoginData();
    }

    // Clear tick timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }

    Super::Deinitialize();
}

void UMGDailyLoginSubsystem::TickDailyLoginSystem(float DeltaTime)
{
    FDateTime Now = FDateTime::Now();

    // Check for daily refresh
    if (Now >= LoginState.NextRefreshTime)
    {
        // Refresh challenges
        RefreshDailyChallenges();

        // Reset rerolls
        RemainingRerolls = MaxDailyRerolls;

        // Reset claim status
        LoginState.Streak.bClaimedToday = false;

        // Calculate next refresh time
        CalculateNextRefreshTime();

        // Update calendar
        if (LoginState.Calendar.Month != Now.GetMonth())
        {
            GenerateMonthlyCalendar();
        }

        OnDailyRefresh.Broadcast();
        bDataDirty = true;
    }

    // Check for expired bonuses
    CheckBonusExpiration();

    // Check for expired return bonus
    if (LoginState.ReturnBonus.bIsActive && LoginState.ReturnBonus.IsBonusExpired())
    {
        LoginState.ReturnBonus.bIsActive = false;
        bDataDirty = true;
    }
}

// ===== Login Management =====

void UMGDailyLoginSubsystem::ProcessLogin()
{
    FDateTime Now = FDateTime::Now();
    FDateTime LastLogin = LoginState.Streak.LastLoginDate;

    // Check if this is a new day
    bool bIsNewDay = LastLogin.GetDay() != Now.GetDay() ||
                     LastLogin.GetMonth() != Now.GetMonth() ||
                     LastLogin.GetYear() != Now.GetYear();

    if (!bIsNewDay)
    {
        return; // Already logged in today
    }

    // Calculate days since last login
    int32 DaysSinceLastLogin = 0;
    if (LastLogin != FDateTime())
    {
        FTimespan TimeSinceLogin = Now - LastLogin;
        DaysSinceLastLogin = FMath::FloorToInt(TimeSinceLogin.GetTotalDays());
    }

    // Check for return player bonus
    if (DaysSinceLastLogin >= ReturnPlayerThresholdDays)
    {
        ProcessReturnPlayer();
    }

    // Update streak
    UpdateStreakStatus();

    // Update login time
    LoginState.Streak.LastLoginDate = Now;
    if (LoginState.Streak.FirstLoginDate == FDateTime())
    {
        LoginState.Streak.FirstLoginDate = Now;
    }

    // Update weekly/monthly counts
    LoginState.Streak.WeeklyLoginCount++;
    LoginState.Streak.MonthlyLoginCount++;
    LoginState.Streak.TotalLoginDays++;

    bDataDirty = true;
}

bool UMGDailyLoginSubsystem::ClaimDailyReward(TArray<FMGDailyReward>& OutRewards)
{
    if (!CanClaimDailyReward())
    {
        return false;
    }

    // Get today's rewards
    OutRewards = GetTodaysRewards();

    // Mark as claimed
    LoginState.Streak.bClaimedToday = true;

    // Update calendar day
    FDateTime Now = FDateTime::Now();
    int32 TodayIndex = Now.GetDay() - 1;
    if (LoginState.Calendar.Days.IsValidIndex(TodayIndex))
    {
        LoginState.Calendar.Days[TodayIndex].bIsClaimed = true;
        LoginState.Calendar.DaysClaimed++;

        // Check for monthly completion
        if (LoginState.Calendar.DaysClaimed >= LoginState.Calendar.TotalDays && !LoginState.Calendar.bMonthComplete)
        {
            LoginState.Calendar.bMonthComplete = true;
            OnMonthlyCalendarComplete.Broadcast(LoginState.Calendar.MonthlyCompletionRewards);
            OutRewards.Append(LoginState.Calendar.MonthlyCompletionRewards);
        }
    }

    // Add pending rewards
    LoginState.PendingRewards.Append(OutRewards);

    OnDailyLoginClaimed.Broadcast(OutRewards);
    bDataDirty = true;

    return true;
}

bool UMGDailyLoginSubsystem::CanClaimDailyReward() const
{
    return !LoginState.Streak.bClaimedToday;
}

TArray<FMGDailyReward> UMGDailyLoginSubsystem::GetTodaysRewards() const
{
    int32 CurrentStreak = LoginState.Streak.CurrentStreak;
    int32 DayInCycle = ((CurrentStreak - 1) % 7) + 1; // 1-7 cycle

    TArray<FMGDailyReward> Rewards;

    // Base daily reward
    FMGDailyReward BaseReward;
    BaseReward.RewardId = FName(*FString::Printf(TEXT("Daily_Day%d"), DayInCycle));
    BaseReward.RewardType = EMGDailyRewardType::Currency;
    BaseReward.DisplayName = FText::FromString(TEXT("Daily Login Bonus"));
    BaseReward.Quantity = 500 + (DayInCycle * 100);
    BaseReward.DayNumber = DayInCycle;
    Rewards.Add(BaseReward);

    // Streak bonus (every 7th day)
    if (CurrentStreak > 0 && CurrentStreak % 7 == 0)
    {
        FMGDailyReward StreakReward;
        StreakReward.RewardId = FName(*FString::Printf(TEXT("Streak_%d"), CurrentStreak));
        StreakReward.RewardType = EMGDailyRewardType::LootBox;
        StreakReward.DisplayName = FText::FromString(TEXT("Weekly Streak Bonus"));
        StreakReward.Quantity = 1;
        StreakReward.bIsWeeklyReward = true;
        Rewards.Add(StreakReward);
    }

    // Tier bonus
    EMGDailyRewardTier CurrentTier = LoginState.Streak.CurrentTier;
    if (CurrentTier >= EMGDailyRewardTier::Silver)
    {
        FMGDailyReward TierReward;
        TierReward.RewardId = FName("TierBonus");
        TierReward.RewardType = EMGDailyRewardType::Experience;
        TierReward.DisplayName = FText::FromString(TEXT("Tier Bonus XP"));
        TierReward.Quantity = 100 * (static_cast<int32>(CurrentTier) + 1);
        TierReward.bIsBonusReward = true;
        TierReward.RequiredTier = CurrentTier;
        Rewards.Add(TierReward);
    }

    // Weekend bonus
    FDateTime Now = FDateTime::Now();
    int32 DayOfWeek = static_cast<int32>(Now.GetDayOfWeek());
    if (DayOfWeek == 0 || DayOfWeek == 6) // Sunday or Saturday
    {
        FMGDailyReward WeekendReward;
        WeekendReward.RewardId = FName("WeekendBonus");
        WeekendReward.RewardType = EMGDailyRewardType::BoostToken;
        WeekendReward.DisplayName = FText::FromString(TEXT("Weekend Bonus"));
        WeekendReward.Quantity = 2;
        WeekendReward.bIsBonusReward = true;
        Rewards.Add(WeekendReward);
    }

    return Rewards;
}

FTimespan UMGDailyLoginSubsystem::GetTimeUntilNextClaim() const
{
    if (CanClaimDailyReward())
    {
        return FTimespan::Zero();
    }
    return LoginState.GetTimeUntilRefresh();
}

bool UMGDailyLoginSubsystem::IsFirstLoginToday() const
{
    FDateTime Now = FDateTime::Now();
    FDateTime LastLogin = LoginState.Streak.LastLoginDate;

    return LastLogin.GetDay() != Now.GetDay() ||
           LastLogin.GetMonth() != Now.GetMonth() ||
           LastLogin.GetYear() != Now.GetYear();
}

// ===== Streak Management =====

FMGLoginStreak UMGDailyLoginSubsystem::GetLoginStreak() const
{
    return LoginState.Streak;
}

int32 UMGDailyLoginSubsystem::GetCurrentStreak() const
{
    return LoginState.Streak.CurrentStreak;
}

int32 UMGDailyLoginSubsystem::GetLongestStreak() const
{
    return LoginState.Streak.LongestStreak;
}

EMGDailyRewardTier UMGDailyLoginSubsystem::GetCurrentTier() const
{
    return LoginState.Streak.CurrentTier;
}

int32 UMGDailyLoginSubsystem::GetDaysUntilNextTier() const
{
    return LoginState.Streak.DaysUntilNextTier;
}

float UMGDailyLoginSubsystem::GetTierProgress() const
{
    int32 CurrentThreshold = GetTierThreshold(LoginState.Streak.CurrentTier);
    EMGDailyRewardTier NextTier = static_cast<EMGDailyRewardTier>(
        FMath::Min(static_cast<int32>(LoginState.Streak.CurrentTier) + 1,
                   static_cast<int32>(EMGDailyRewardTier::Champion))
    );
    int32 NextThreshold = GetTierThreshold(NextTier);

    if (NextThreshold == CurrentThreshold) return 1.0f;

    int32 Progress = LoginState.Streak.CurrentStreak - CurrentThreshold;
    int32 Required = NextThreshold - CurrentThreshold;

    return static_cast<float>(Progress) / Required;
}

TArray<FMGDailyReward> UMGDailyLoginSubsystem::GetStreakMilestoneRewards(int32 StreakDay) const
{
    TArray<FMGDailyReward> Rewards;

    // Check if this is a milestone day
    bool bIsMilestone = false;
    for (int32 i = 0; i < NUM_STREAK_MILESTONES; i++)
    {
        if (STREAK_MILESTONE_DAYS[i] == StreakDay)
        {
            bIsMilestone = true;
            break;
        }
    }

    if (!bIsMilestone) return Rewards;

    // Generate milestone rewards based on streak day
    FMGDailyReward MilestoneReward;
    MilestoneReward.RewardId = FName(*FString::Printf(TEXT("Milestone_%d"), StreakDay));
    MilestoneReward.bIsBonusReward = true;
    MilestoneReward.DayNumber = StreakDay;

    if (StreakDay == 7)
    {
        MilestoneReward.RewardType = EMGDailyRewardType::LootBox;
        MilestoneReward.DisplayName = FText::FromString(TEXT("Week Streak Bonus"));
        MilestoneReward.Quantity = 1;
    }
    else if (StreakDay == 14)
    {
        MilestoneReward.RewardType = EMGDailyRewardType::PremiumCurrency;
        MilestoneReward.DisplayName = FText::FromString(TEXT("2 Week Streak Bonus"));
        MilestoneReward.Quantity = 50;
    }
    else if (StreakDay == 30)
    {
        MilestoneReward.RewardType = EMGDailyRewardType::ExclusiveItem;
        MilestoneReward.DisplayName = FText::FromString(TEXT("Monthly Streak Exclusive"));
        MilestoneReward.Quantity = 1;
        MilestoneReward.UnlockId = FName("Cosmetic_30DayStreak");
    }
    else if (StreakDay == 60)
    {
        MilestoneReward.RewardType = EMGDailyRewardType::Part;
        MilestoneReward.DisplayName = FText::FromString(TEXT("2 Month Streak Part"));
        MilestoneReward.Quantity = 1;
        MilestoneReward.UnlockId = FName("Part_Legendary_Random");
    }
    else if (StreakDay == 90)
    {
        MilestoneReward.RewardType = EMGDailyRewardType::VehicleRental;
        MilestoneReward.DisplayName = FText::FromString(TEXT("3 Month Streak Vehicle"));
        MilestoneReward.Quantity = 7; // 7 day rental
        MilestoneReward.UnlockId = FName("Vehicle_Premium_Random");
    }
    else if (StreakDay == 180)
    {
        MilestoneReward.RewardType = EMGDailyRewardType::ExclusiveItem;
        MilestoneReward.DisplayName = FText::FromString(TEXT("6 Month Dedication Award"));
        MilestoneReward.Quantity = 1;
        MilestoneReward.UnlockId = FName("Title_Dedicated");
    }
    else if (StreakDay == 365)
    {
        MilestoneReward.RewardType = EMGDailyRewardType::ExclusiveItem;
        MilestoneReward.DisplayName = FText::FromString(TEXT("Year One Champion"));
        MilestoneReward.Quantity = 1;
        MilestoneReward.UnlockId = FName("Vehicle_YearOne_Exclusive");
    }

    Rewards.Add(MilestoneReward);
    return Rewards;
}

// ===== Calendar =====

FMGMonthlyCalendar UMGDailyLoginSubsystem::GetCurrentCalendar() const
{
    return LoginState.Calendar;
}

FMGCalendarDay UMGDailyLoginSubsystem::GetCalendarDay(int32 DayNumber) const
{
    int32 Index = DayNumber - 1;
    if (LoginState.Calendar.Days.IsValidIndex(Index))
    {
        return LoginState.Calendar.Days[Index];
    }
    return FMGCalendarDay();
}

TArray<FMGCalendarDay> UMGDailyLoginSubsystem::GetUpcomingDays(int32 Count) const
{
    TArray<FMGCalendarDay> Result;
    FDateTime Now = FDateTime::Now();
    int32 Today = Now.GetDay();

    for (int32 i = 0; i < Count && (Today + i) <= LoginState.Calendar.TotalDays; i++)
    {
        int32 Index = Today - 1 + i;
        if (LoginState.Calendar.Days.IsValidIndex(Index))
        {
            Result.Add(LoginState.Calendar.Days[Index]);
        }
    }

    return Result;
}

float UMGDailyLoginSubsystem::GetMonthlyCompletionProgress() const
{
    return LoginState.Calendar.GetCompletionPercent();
}

bool UMGDailyLoginSubsystem::IsMonthComplete() const
{
    return LoginState.Calendar.bMonthComplete;
}

// ===== Daily Challenges =====

TArray<FMGDailyChallenge> UMGDailyLoginSubsystem::GetDailyChallenges() const
{
    return LoginState.DailyChallenges;
}

FMGDailyChallenge UMGDailyLoginSubsystem::GetChallenge(FName ChallengeId) const
{
    for (const FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        if (Challenge.ChallengeId == ChallengeId)
        {
            return Challenge;
        }
    }
    return FMGDailyChallenge();
}

void UMGDailyLoginSubsystem::UpdateChallengeProgress(FName ChallengeId, float NewValue)
{
    for (FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        if (Challenge.ChallengeId == ChallengeId)
        {
            if (Challenge.bIsComplete)
            {
                return; // Already complete
            }

            Challenge.CurrentValue = NewValue;

            // Check completion
            if (Challenge.CurrentValue >= Challenge.TargetValue)
            {
                Challenge.bIsComplete = true;
                OnChallengeCompleted.Broadcast(ChallengeId, Challenge.Rewards);
            }
            else
            {
                OnChallengeProgressUpdated.Broadcast(ChallengeId, Challenge.GetProgressPercent());
            }

            bDataDirty = true;
            return;
        }
    }
}

void UMGDailyLoginSubsystem::IncrementChallengeProgress(FName ChallengeId, float Amount)
{
    for (FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        if (Challenge.ChallengeId == ChallengeId)
        {
            UpdateChallengeProgress(ChallengeId, Challenge.CurrentValue + Amount);
            return;
        }
    }
}

void UMGDailyLoginSubsystem::UpdateChallengesByType(EMGDailyChallengeType ChallengeType, float Value)
{
    for (FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        if (Challenge.ChallengeType == ChallengeType && !Challenge.bIsComplete)
        {
            UpdateChallengeProgress(Challenge.ChallengeId, Challenge.CurrentValue + Value);
        }
    }
}

bool UMGDailyLoginSubsystem::ClaimChallengeReward(FName ChallengeId, TArray<FMGDailyReward>& OutRewards)
{
    for (FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        if (Challenge.ChallengeId == ChallengeId)
        {
            if (!Challenge.bIsComplete || Challenge.bIsClaimed)
            {
                return false;
            }

            Challenge.bIsClaimed = true;
            OutRewards = Challenge.Rewards;

            // Add bonus currency and XP
            if (Challenge.BonusCurrency > 0)
            {
                FMGDailyReward CurrencyReward;
                CurrencyReward.RewardType = EMGDailyRewardType::Currency;
                CurrencyReward.Quantity = Challenge.BonusCurrency * Challenge.GetDifficultyMultiplier();
                OutRewards.Add(CurrencyReward);
            }

            if (Challenge.BonusXP > 0)
            {
                FMGDailyReward XPReward;
                XPReward.RewardType = EMGDailyRewardType::Experience;
                XPReward.Quantity = Challenge.BonusXP * Challenge.GetDifficultyMultiplier();
                OutRewards.Add(XPReward);
            }

            LoginState.PendingRewards.Append(OutRewards);
            bDataDirty = true;
            return true;
        }
    }
    return false;
}

bool UMGDailyLoginSubsystem::ClaimAllChallengeRewards(TArray<FMGDailyReward>& OutRewards)
{
    OutRewards.Empty();
    bool bClaimedAny = false;

    for (FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        if (Challenge.bIsComplete && !Challenge.bIsClaimed)
        {
            TArray<FMGDailyReward> ChallengeRewards;
            if (ClaimChallengeReward(Challenge.ChallengeId, ChallengeRewards))
            {
                OutRewards.Append(ChallengeRewards);
                bClaimedAny = true;
            }
        }
    }

    return bClaimedAny;
}

int32 UMGDailyLoginSubsystem::GetCompletedChallengeCount() const
{
    int32 Count = 0;
    for (const FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        if (Challenge.bIsComplete)
        {
            Count++;
        }
    }
    return Count;
}

int32 UMGDailyLoginSubsystem::GetUnclaimedChallengeCount() const
{
    int32 Count = 0;
    for (const FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        if (Challenge.bIsComplete && !Challenge.bIsClaimed)
        {
            Count++;
        }
    }
    return Count;
}

void UMGDailyLoginSubsystem::RerollChallenge(FName ChallengeId)
{
    if (RemainingRerolls <= 0)
    {
        return;
    }

    for (int32 i = 0; i < LoginState.DailyChallenges.Num(); i++)
    {
        if (LoginState.DailyChallenges[i].ChallengeId == ChallengeId)
        {
            if (LoginState.DailyChallenges[i].bIsComplete)
            {
                return; // Can't reroll completed challenges
            }

            // Generate new challenge of same difficulty
            EMGChallengeDifficulty Difficulty = LoginState.DailyChallenges[i].Difficulty;
            LoginState.DailyChallenges[i] = GenerateRandomChallenge(Difficulty);
            RemainingRerolls--;
            bDataDirty = true;
            return;
        }
    }
}

int32 UMGDailyLoginSubsystem::GetRemainingRerolls() const
{
    return RemainingRerolls;
}

// ===== Weekly Bonuses =====

TArray<FMGWeeklyBonus> UMGDailyLoginSubsystem::GetActiveBonuses() const
{
    TArray<FMGWeeklyBonus> ActiveBonuses;
    for (const FMGWeeklyBonus& Bonus : LoginState.ActiveBonuses)
    {
        if (Bonus.bIsActive && !Bonus.IsExpired())
        {
            ActiveBonuses.Add(Bonus);
        }
    }
    return ActiveBonuses;
}

bool UMGDailyLoginSubsystem::HasActiveBonus(EMGWeeklyBonusType BonusType) const
{
    for (const FMGWeeklyBonus& Bonus : LoginState.ActiveBonuses)
    {
        if (Bonus.BonusType == BonusType && Bonus.bIsActive && !Bonus.IsExpired())
        {
            return true;
        }
    }
    return false;
}

float UMGDailyLoginSubsystem::GetBonusMultiplier(EMGWeeklyBonusType BonusType) const
{
    for (const FMGWeeklyBonus& Bonus : LoginState.ActiveBonuses)
    {
        if (Bonus.BonusType == BonusType && Bonus.bIsActive && !Bonus.IsExpired())
        {
            return Bonus.Multiplier;
        }
    }
    return 1.0f;
}

FTimespan UMGDailyLoginSubsystem::GetBonusTimeRemaining(EMGWeeklyBonusType BonusType) const
{
    for (const FMGWeeklyBonus& Bonus : LoginState.ActiveBonuses)
    {
        if (Bonus.BonusType == BonusType && Bonus.bIsActive)
        {
            return Bonus.GetTimeRemaining();
        }
    }
    return FTimespan::Zero();
}

void UMGDailyLoginSubsystem::ActivateWeeklyBonus(const FMGWeeklyBonus& Bonus)
{
    FMGWeeklyBonus NewBonus = Bonus;
    NewBonus.bIsActive = true;
    NewBonus.StartDate = FDateTime::Now();

    // Remove any existing bonus of same type
    for (int32 i = LoginState.ActiveBonuses.Num() - 1; i >= 0; i--)
    {
        if (LoginState.ActiveBonuses[i].BonusType == Bonus.BonusType)
        {
            LoginState.ActiveBonuses.RemoveAt(i);
        }
    }

    LoginState.ActiveBonuses.Add(NewBonus);
    OnWeeklyBonusActivated.Broadcast(NewBonus);
    bDataDirty = true;
}

// ===== Return Player =====

bool UMGDailyLoginSubsystem::IsReturnPlayer() const
{
    if (LoginState.Streak.LastLoginDate == FDateTime())
    {
        return false;
    }

    FTimespan TimeSinceLogin = FDateTime::Now() - LoginState.Streak.LastLoginDate;
    return TimeSinceLogin.GetTotalDays() >= ReturnPlayerThresholdDays;
}

FMGReturnPlayerBonus UMGDailyLoginSubsystem::GetReturnBonus() const
{
    return LoginState.ReturnBonus;
}

bool UMGDailyLoginSubsystem::HasActiveReturnBonus() const
{
    return LoginState.ReturnBonus.bIsActive && !LoginState.ReturnBonus.IsBonusExpired();
}

bool UMGDailyLoginSubsystem::ClaimReturnBonus(TArray<FMGDailyReward>& OutRewards)
{
    if (!LoginState.ReturnBonus.bIsActive)
    {
        return false;
    }

    OutRewards = LoginState.ReturnBonus.WelcomeBackRewards;
    LoginState.PendingRewards.Append(OutRewards);

    // Don't deactivate - the multiplier bonuses remain active
    bDataDirty = true;
    return true;
}

// ===== State =====

FMGDailyLoginState UMGDailyLoginSubsystem::GetLoginState() const
{
    return LoginState;
}

FTimespan UMGDailyLoginSubsystem::GetTimeUntilDailyRefresh() const
{
    return LoginState.GetTimeUntilRefresh();
}

void UMGDailyLoginSubsystem::ForceRefresh()
{
    RefreshDailyChallenges();
    RemainingRerolls = MaxDailyRerolls;
    LoginState.Streak.bClaimedToday = false;
    CalculateNextRefreshTime();
    OnDailyRefresh.Broadcast();
    bDataDirty = true;
}

// ===== Persistence =====

void UMGDailyLoginSubsystem::SaveLoginData()
{
    // In a real implementation, this would serialize to save game
    UE_LOG(LogTemp, Log, TEXT("MGDailyLoginSubsystem: Saving login data..."));
    bDataDirty = false;
}

void UMGDailyLoginSubsystem::LoadLoginData()
{
    // In a real implementation, this would deserialize from save game
    UE_LOG(LogTemp, Log, TEXT("MGDailyLoginSubsystem: Loading login data..."));

    // Initialize default challenges if empty
    if (LoginState.DailyChallenges.Num() == 0)
    {
        RefreshDailyChallenges();
    }
}

// ===== Debug =====

void UMGDailyLoginSubsystem::DebugSetStreak(int32 NewStreak)
{
    LoginState.Streak.CurrentStreak = NewStreak;
    if (NewStreak > LoginState.Streak.LongestStreak)
    {
        LoginState.Streak.LongestStreak = NewStreak;
    }
    UpdateTier();
    OnStreakUpdated.Broadcast(NewStreak, LoginState.Streak.CurrentTier);
    bDataDirty = true;
}

void UMGDailyLoginSubsystem::DebugAdvanceDay()
{
    // Simulate advancing to next day
    LoginState.Streak.bClaimedToday = false;
    LoginState.Streak.CurrentStreak++;
    if (LoginState.Streak.CurrentStreak > LoginState.Streak.LongestStreak)
    {
        LoginState.Streak.LongestStreak = LoginState.Streak.CurrentStreak;
    }
    UpdateTier();
    RefreshDailyChallenges();
    RemainingRerolls = MaxDailyRerolls;
    OnDailyRefresh.Broadcast();
    bDataDirty = true;
}

void UMGDailyLoginSubsystem::DebugResetAll()
{
    LoginState = FMGDailyLoginState();
    RemainingRerolls = MaxDailyRerolls;
    GenerateMonthlyCalendar();
    RefreshDailyChallenges();
    CalculateNextRefreshTime();
    bDataDirty = true;

    UE_LOG(LogTemp, Log, TEXT("MGDailyLoginSubsystem: All data reset"));
}

void UMGDailyLoginSubsystem::DebugGrantReturnBonus(int32 DaysAbsent)
{
    LoginState.ReturnBonus.DaysAbsent = DaysAbsent;
    LoginState.ReturnBonus.bIsActive = true;
    LoginState.ReturnBonus.BonusExpiresAt = FDateTime::Now() + FTimespan::FromHours(LoginState.ReturnBonus.BonusDurationHours);

    // Calculate bonus multipliers based on absence
    LoginState.ReturnBonus.XPMultiplier = 1.0f + (DaysAbsent * 0.1f);
    LoginState.ReturnBonus.CurrencyMultiplier = 1.0f + (DaysAbsent * 0.05f);

    // Generate welcome back rewards
    LoginState.ReturnBonus.WelcomeBackRewards.Empty();

    FMGDailyReward CurrencyReward;
    CurrencyReward.RewardType = EMGDailyRewardType::Currency;
    CurrencyReward.DisplayName = FText::FromString(TEXT("Welcome Back Currency"));
    CurrencyReward.Quantity = 1000 + (DaysAbsent * 100);
    LoginState.ReturnBonus.WelcomeBackRewards.Add(CurrencyReward);

    FMGDailyReward BoostReward;
    BoostReward.RewardType = EMGDailyRewardType::BoostToken;
    BoostReward.DisplayName = FText::FromString(TEXT("Welcome Back Boosts"));
    BoostReward.Quantity = FMath::Min(DaysAbsent / 7, 5);
    if (BoostReward.Quantity > 0)
    {
        LoginState.ReturnBonus.WelcomeBackRewards.Add(BoostReward);
    }

    OnReturnPlayerBonusGranted.Broadcast(LoginState.ReturnBonus);
    bDataDirty = true;
}

void UMGDailyLoginSubsystem::DebugCompleteAllChallenges()
{
    for (FMGDailyChallenge& Challenge : LoginState.DailyChallenges)
    {
        Challenge.CurrentValue = Challenge.TargetValue;
        Challenge.bIsComplete = true;
        OnChallengeCompleted.Broadcast(Challenge.ChallengeId, Challenge.Rewards);
    }
    bDataDirty = true;
}

// ===== Internal Helpers =====

void UMGDailyLoginSubsystem::RefreshDailyChallenges()
{
    LoginState.DailyChallenges.Empty();

    // Generate 3 challenges of varying difficulty
    LoginState.DailyChallenges.Add(GenerateRandomChallenge(EMGChallengeDifficulty::Easy));
    LoginState.DailyChallenges.Add(GenerateRandomChallenge(EMGChallengeDifficulty::Medium));
    LoginState.DailyChallenges.Add(GenerateRandomChallenge(EMGChallengeDifficulty::Hard));

    bDataDirty = true;
}

void UMGDailyLoginSubsystem::GenerateMonthlyCalendar()
{
    FDateTime Now = FDateTime::Now();

    LoginState.Calendar.Month = Now.GetMonth();
    LoginState.Calendar.Year = Now.GetYear();
    LoginState.Calendar.DaysClaimed = 0;
    LoginState.Calendar.bMonthComplete = false;

    // Get month name
    static const TCHAR* MonthNames[] = {
        TEXT("January"), TEXT("February"), TEXT("March"), TEXT("April"),
        TEXT("May"), TEXT("June"), TEXT("July"), TEXT("August"),
        TEXT("September"), TEXT("October"), TEXT("November"), TEXT("December")
    };
    LoginState.Calendar.MonthName = FText::FromString(MonthNames[Now.GetMonth() - 1]);

    // Calculate days in month
    int32 DaysInMonth = FDateTime::DaysInMonth(Now.GetYear(), Now.GetMonth());
    LoginState.Calendar.TotalDays = DaysInMonth;
    LoginState.Calendar.Days.Empty();

    // Generate calendar days
    for (int32 Day = 1; Day <= DaysInMonth; Day++)
    {
        FMGCalendarDay CalendarDay;
        CalendarDay.DayNumber = Day;
        CalendarDay.Date = FDateTime(Now.GetYear(), Now.GetMonth(), Day);
        CalendarDay.bIsClaimed = false;
        CalendarDay.bIsToday = (Day == Now.GetDay());

        // Check for milestone days (every 7th day)
        CalendarDay.bIsMilestoneDay = (Day % 7 == 0);

        // Check for weekends
        int32 DayOfWeek = static_cast<int32>(CalendarDay.Date.GetDayOfWeek());
        CalendarDay.bIsWeekendBonus = (DayOfWeek == 0 || DayOfWeek == 6);

        // Generate rewards for this day
        CalendarDay.Rewards = GenerateDayRewards(Day);

        LoginState.Calendar.Days.Add(CalendarDay);
    }

    // Generate monthly completion rewards
    LoginState.Calendar.MonthlyCompletionRewards.Empty();

    FMGDailyReward MonthlyReward;
    MonthlyReward.RewardId = FName("MonthlyCompletion");
    MonthlyReward.RewardType = EMGDailyRewardType::MysteryBox;
    MonthlyReward.DisplayName = FText::FromString(TEXT("Monthly Completion Reward"));
    MonthlyReward.Quantity = 1;
    MonthlyReward.bIsMonthlyReward = true;
    LoginState.Calendar.MonthlyCompletionRewards.Add(MonthlyReward);

    FMGDailyReward BonusCurrency;
    BonusCurrency.RewardId = FName("MonthlyBonusCurrency");
    BonusCurrency.RewardType = EMGDailyRewardType::PremiumCurrency;
    BonusCurrency.DisplayName = FText::FromString(TEXT("Monthly Bonus"));
    BonusCurrency.Quantity = 100;
    BonusCurrency.bIsMonthlyReward = true;
    LoginState.Calendar.MonthlyCompletionRewards.Add(BonusCurrency);

    bDataDirty = true;
}

void UMGDailyLoginSubsystem::UpdateStreakStatus()
{
    FDateTime Now = FDateTime::Now();
    FDateTime LastLogin = LoginState.Streak.LastLoginDate;

    if (LastLogin == FDateTime())
    {
        // First ever login
        LoginState.Streak.CurrentStreak = 1;
        LoginState.Streak.StreakStartDate = Now;
    }
    else
    {
        // Calculate days between last login and now
        FTimespan TimeSinceLogin = Now - LastLogin;
        int32 DaysSinceLogin = FMath::FloorToInt(TimeSinceLogin.GetTotalDays());

        if (DaysSinceLogin <= 1)
        {
            // Streak continues
            if (DaysSinceLogin == 1)
            {
                LoginState.Streak.CurrentStreak++;
            }
            // If same day (DaysSinceLogin == 0), don't increment
        }
        else
        {
            // Streak broken
            int32 PreviousStreak = LoginState.Streak.CurrentStreak;
            OnStreakLost.Broadcast(PreviousStreak);

            LoginState.Streak.CurrentStreak = 1;
            LoginState.Streak.StreakStartDate = Now;
        }
    }

    // Update longest streak
    if (LoginState.Streak.CurrentStreak > LoginState.Streak.LongestStreak)
    {
        LoginState.Streak.LongestStreak = LoginState.Streak.CurrentStreak;
    }

    // Update tier
    EMGDailyRewardTier OldTier = LoginState.Streak.CurrentTier;
    UpdateTier();

    if (LoginState.Streak.CurrentTier != OldTier)
    {
        OnTierUpgrade.Broadcast(LoginState.Streak.CurrentTier);
    }

    OnStreakUpdated.Broadcast(LoginState.Streak.CurrentStreak, LoginState.Streak.CurrentTier);
}

void UMGDailyLoginSubsystem::ProcessReturnPlayer()
{
    FDateTime Now = FDateTime::Now();
    FTimespan TimeSinceLogin = Now - LoginState.Streak.LastLoginDate;
    int32 DaysAbsent = FMath::FloorToInt(TimeSinceLogin.GetTotalDays());

    LoginState.ReturnBonus.DaysAbsent = DaysAbsent;
    LoginState.ReturnBonus.bIsActive = true;
    LoginState.ReturnBonus.BonusExpiresAt = Now + FTimespan::FromHours(LoginState.ReturnBonus.BonusDurationHours);

    // Scale bonuses based on absence length
    LoginState.ReturnBonus.XPMultiplier = FMath::Clamp(1.0f + (DaysAbsent * 0.05f), 1.0f, 2.0f);
    LoginState.ReturnBonus.CurrencyMultiplier = FMath::Clamp(1.0f + (DaysAbsent * 0.05f), 1.0f, 2.0f);

    // Generate welcome back rewards
    LoginState.ReturnBonus.WelcomeBackRewards.Empty();

    FMGDailyReward WelcomeBack;
    WelcomeBack.RewardId = FName("WelcomeBack");
    WelcomeBack.RewardType = EMGDailyRewardType::Currency;
    WelcomeBack.DisplayName = FText::FromString(TEXT("Welcome Back!"));
    WelcomeBack.Quantity = 500 * FMath::Min(DaysAbsent / 7 + 1, 4);
    LoginState.ReturnBonus.WelcomeBackRewards.Add(WelcomeBack);

    if (DaysAbsent >= 14)
    {
        FMGDailyReward BonusBox;
        BonusBox.RewardId = FName("ReturnLootBox");
        BonusBox.RewardType = EMGDailyRewardType::LootBox;
        BonusBox.DisplayName = FText::FromString(TEXT("Return Player Bonus"));
        BonusBox.Quantity = 1;
        LoginState.ReturnBonus.WelcomeBackRewards.Add(BonusBox);
    }

    OnReturnPlayerBonusGranted.Broadcast(LoginState.ReturnBonus);
}

void UMGDailyLoginSubsystem::UpdateTier()
{
    int32 CurrentStreak = LoginState.Streak.CurrentStreak;
    EMGDailyRewardTier NewTier = CalculateTierForStreak(CurrentStreak);

    LoginState.Streak.CurrentTier = NewTier;

    // Calculate days until next tier
    EMGDailyRewardTier NextTier = static_cast<EMGDailyRewardTier>(
        FMath::Min(static_cast<int32>(NewTier) + 1, static_cast<int32>(EMGDailyRewardTier::Champion))
    );
    int32 NextThreshold = GetTierThreshold(NextTier);
    LoginState.Streak.DaysUntilNextTier = FMath::Max(0, NextThreshold - CurrentStreak);
}

void UMGDailyLoginSubsystem::CheckBonusExpiration()
{
    for (int32 i = LoginState.ActiveBonuses.Num() - 1; i >= 0; i--)
    {
        if (LoginState.ActiveBonuses[i].IsExpired())
        {
            FName ExpiredBonusId = LoginState.ActiveBonuses[i].BonusId;
            LoginState.ActiveBonuses.RemoveAt(i);
            OnWeeklyBonusExpired.Broadcast(ExpiredBonusId);
            bDataDirty = true;
        }
    }
}

void UMGDailyLoginSubsystem::CalculateNextRefreshTime()
{
    FDateTime Now = FDateTime::Now();

    // Next refresh is at midnight UTC
    FDateTime Tomorrow(Now.GetYear(), Now.GetMonth(), Now.GetDay());
    Tomorrow += FTimespan::FromDays(1);

    LoginState.NextRefreshTime = Tomorrow;
    LoginState.LastRefreshTime = Now;
}

FMGDailyChallenge UMGDailyLoginSubsystem::GenerateRandomChallenge(EMGChallengeDifficulty Difficulty)
{
    FMGDailyChallenge Challenge;

    // Random challenge type
    int32 TypeIndex = FMath::RandRange(0, static_cast<int32>(EMGDailyChallengeType::Overtakes));
    Challenge.ChallengeType = static_cast<EMGDailyChallengeType>(TypeIndex);
    Challenge.Difficulty = Difficulty;

    // Generate unique ID
    Challenge.ChallengeId = FName(*FString::Printf(TEXT("Daily_%d_%d"),
        static_cast<int32>(Challenge.ChallengeType), FMath::RandRange(1000, 9999)));

    // Set target value based on type and difficulty
    float DifficultyMultiplier = static_cast<float>(Challenge.GetDifficultyMultiplier());

    switch (Challenge.ChallengeType)
    {
        case EMGDailyChallengeType::WinRaces:
            Challenge.DisplayName = FText::FromString(TEXT("Race Winner"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "WinRaces", "Win {0} races"),
                FText::AsNumber(static_cast<int32>(1 * DifficultyMultiplier)));
            Challenge.TargetValue = 1 * DifficultyMultiplier;
            break;

        case EMGDailyChallengeType::CompleteRaces:
            Challenge.DisplayName = FText::FromString(TEXT("Racing Enthusiast"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "CompleteRaces", "Complete {0} races"),
                FText::AsNumber(static_cast<int32>(3 * DifficultyMultiplier)));
            Challenge.TargetValue = 3 * DifficultyMultiplier;
            break;

        case EMGDailyChallengeType::DriftScore:
            Challenge.DisplayName = FText::FromString(TEXT("Drift Master"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "DriftScore", "Score {0} drift points"),
                FText::AsNumber(static_cast<int32>(5000 * DifficultyMultiplier)));
            Challenge.TargetValue = 5000 * DifficultyMultiplier;
            break;

        case EMGDailyChallengeType::AirtimeSeconds:
            Challenge.DisplayName = FText::FromString(TEXT("High Flyer"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "Airtime", "Get {0} seconds of airtime"),
                FText::AsNumber(static_cast<int32>(10 * DifficultyMultiplier)));
            Challenge.TargetValue = 10 * DifficultyMultiplier;
            break;

        case EMGDailyChallengeType::NearMisses:
            Challenge.DisplayName = FText::FromString(TEXT("Risk Taker"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "NearMisses", "Perform {0} near misses"),
                FText::AsNumber(static_cast<int32>(10 * DifficultyMultiplier)));
            Challenge.TargetValue = 10 * DifficultyMultiplier;
            break;

        case EMGDailyChallengeType::Takedowns:
            Challenge.DisplayName = FText::FromString(TEXT("Aggressive Driver"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "Takedowns", "Perform {0} takedowns"),
                FText::AsNumber(static_cast<int32>(2 * DifficultyMultiplier)));
            Challenge.TargetValue = 2 * DifficultyMultiplier;
            break;

        case EMGDailyChallengeType::TopSpeedReach:
            Challenge.DisplayName = FText::FromString(TEXT("Speed Demon"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "TopSpeed", "Reach {0} mph"),
                FText::AsNumber(static_cast<int32>(120 + (20 * DifficultyMultiplier))));
            Challenge.TargetValue = 120 + (20 * DifficultyMultiplier);
            break;

        case EMGDailyChallengeType::DistanceDriven:
            Challenge.DisplayName = FText::FromString(TEXT("Road Warrior"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "Distance", "Drive {0} miles"),
                FText::AsNumber(static_cast<int32>(10 * DifficultyMultiplier)));
            Challenge.TargetValue = 10 * DifficultyMultiplier;
            break;

        case EMGDailyChallengeType::Overtakes:
            Challenge.DisplayName = FText::FromString(TEXT("Overtake King"));
            Challenge.Description = FText::Format(
                NSLOCTEXT("Daily", "Overtakes", "Overtake {0} opponents"),
                FText::AsNumber(static_cast<int32>(5 * DifficultyMultiplier)));
            Challenge.TargetValue = 5 * DifficultyMultiplier;
            break;

        default:
            Challenge.DisplayName = FText::FromString(TEXT("Daily Challenge"));
            Challenge.Description = FText::FromString(TEXT("Complete the objective"));
            Challenge.TargetValue = 1 * DifficultyMultiplier;
            break;
    }

    // Set bonus rewards based on difficulty
    Challenge.BonusXP = 100 * Challenge.GetDifficultyMultiplier();
    Challenge.BonusCurrency = 250 * Challenge.GetDifficultyMultiplier();

    return Challenge;
}

TArray<FMGDailyReward> UMGDailyLoginSubsystem::GenerateDayRewards(int32 DayNumber)
{
    TArray<FMGDailyReward> Rewards;

    FMGDailyReward BaseReward;
    BaseReward.RewardId = FName(*FString::Printf(TEXT("Calendar_Day%d"), DayNumber));
    BaseReward.DayNumber = DayNumber;

    // Vary reward types based on day
    if (DayNumber % 7 == 0)
    {
        // Weekly milestone - better reward
        BaseReward.RewardType = EMGDailyRewardType::LootBox;
        BaseReward.DisplayName = FText::FromString(TEXT("Weekly Bonus"));
        BaseReward.Quantity = 1;
    }
    else if (DayNumber % 5 == 0)
    {
        BaseReward.RewardType = EMGDailyRewardType::BoostToken;
        BaseReward.DisplayName = FText::FromString(TEXT("Boost Token"));
        BaseReward.Quantity = 2;
    }
    else if (DayNumber % 3 == 0)
    {
        BaseReward.RewardType = EMGDailyRewardType::Experience;
        BaseReward.DisplayName = FText::FromString(TEXT("Experience Bonus"));
        BaseReward.Quantity = 500;
    }
    else
    {
        BaseReward.RewardType = EMGDailyRewardType::Currency;
        BaseReward.DisplayName = FText::FromString(TEXT("Daily Currency"));
        BaseReward.Quantity = 200 + (DayNumber * 10);
    }

    Rewards.Add(BaseReward);
    return Rewards;
}

EMGDailyRewardTier UMGDailyLoginSubsystem::CalculateTierForStreak(int32 Streak) const
{
    if (Streak >= 90) return EMGDailyRewardTier::Champion;
    if (Streak >= 60) return EMGDailyRewardTier::Diamond;
    if (Streak >= 30) return EMGDailyRewardTier::Platinum;
    if (Streak >= 14) return EMGDailyRewardTier::Gold;
    if (Streak >= 7) return EMGDailyRewardTier::Silver;
    return EMGDailyRewardTier::Bronze;
}

int32 UMGDailyLoginSubsystem::GetTierThreshold(EMGDailyRewardTier Tier) const
{
    switch (Tier)
    {
        case EMGDailyRewardTier::Bronze: return 0;
        case EMGDailyRewardTier::Silver: return 7;
        case EMGDailyRewardTier::Gold: return 14;
        case EMGDailyRewardTier::Platinum: return 30;
        case EMGDailyRewardTier::Diamond: return 60;
        case EMGDailyRewardTier::Champion: return 90;
        default: return 0;
    }
}

// Copyright Midnight Grind. All Rights Reserved.

// MidnightGrind - Arcade Street Racing Game
// Battle Pass Subsystem - Implementation

#include "BattlePass/MGBattlePassSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMGBattlePassSubsystem::UMGBattlePassSubsystem()
    : XPMultiplier(1.0f)
{
}

void UMGBattlePassSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize sample season
    InitializeSampleSeason();

    // Set up daily challenge refresh timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGBattlePassSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            DailyChallengeTimerHandle,
            [WeakThis]()
            {
                if (!WeakThis.IsValid())
                {
                    return;
                }
                FDateTime Now = FDateTime::Now();
                if (Now.GetDay() != WeakThis->LastDailyChallengeRefresh.GetDay())
                {
                    WeakThis->RefreshDailyChallenges();
                }
            },
            60.0f,
            true
        );
    }
}

void UMGBattlePassSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DailyChallengeTimerHandle);
    }

    Super::Deinitialize();
}

// ===== Season Access =====

FMGBattlePassSeason UMGBattlePassSubsystem::GetCurrentSeason() const
{
    return CurrentSeason;
}

bool UMGBattlePassSubsystem::IsSeasonActive() const
{
    return CurrentSeason.IsActive();
}

FTimespan UMGBattlePassSubsystem::GetSeasonTimeRemaining() const
{
    return CurrentSeason.GetTimeRemaining();
}

int32 UMGBattlePassSubsystem::GetCurrentWeekNumber() const
{
    if (!IsSeasonActive())
    {
        return 0;
    }

    FDateTime Now = FDateTime::Now();
    FTimespan Elapsed = Now - CurrentSeason.StartDate;
    return FMath::Clamp(static_cast<int32>(Elapsed.GetTotalDays() / 7) + 1, 1, 12);
}

// ===== Progress =====

FMGBattlePassProgress UMGBattlePassSubsystem::GetProgress() const
{
    return PlayerProgress;
}

int32 UMGBattlePassSubsystem::GetCurrentTier() const
{
    return PlayerProgress.CurrentTier;
}

int32 UMGBattlePassSubsystem::GetCurrentXP() const
{
    return PlayerProgress.CurrentXP;
}

int32 UMGBattlePassSubsystem::GetXPForNextTier() const
{
    return CalculateXPForTier(PlayerProgress.CurrentTier);
}

float UMGBattlePassSubsystem::GetTierProgress() const
{
    int32 XPRequired = GetXPForNextTier();
    return XPRequired > 0 ? static_cast<float>(PlayerProgress.CurrentXP) / XPRequired : 0.0f;
}

void UMGBattlePassSubsystem::AddXP(int32 Amount, FName Source)
{
    if (Amount <= 0 || !IsSeasonActive())
    {
        return;
    }

    int32 ModifiedAmount = FMath::RoundToInt(Amount * XPMultiplier);
    PlayerProgress.CurrentXP += ModifiedAmount;
    PlayerProgress.TotalXPEarned += ModifiedAmount;

    OnXPGained.Broadcast(ModifiedAmount, PlayerProgress.TotalXPEarned);

    CheckTierProgression();
}

// ===== Tiers =====

FMGBattlePassTierInfo UMGBattlePassSubsystem::GetTierInfo(int32 TierNumber) const
{
    if (TierNumber >= 1 && TierNumber <= CurrentSeason.Tiers.Num())
    {
        return CurrentSeason.Tiers[TierNumber - 1];
    }
    return FMGBattlePassTierInfo();
}

TArray<FMGBattlePassTierInfo> UMGBattlePassSubsystem::GetAllTiers() const
{
    return CurrentSeason.Tiers;
}

TArray<FMGBattlePassTierInfo> UMGBattlePassSubsystem::GetTiersInRange(int32 StartTier, int32 EndTier) const
{
    TArray<FMGBattlePassTierInfo> RangeTiers;

    int32 Start = FMath::Max(1, StartTier);
    int32 End = FMath::Min(CurrentSeason.Tiers.Num(), EndTier);

    for (int32 i = Start; i <= End; i++)
    {
        RangeTiers.Add(CurrentSeason.Tiers[i - 1]);
    }

    return RangeTiers;
}

int32 UMGBattlePassSubsystem::GetMaxTier() const
{
    return CurrentSeason.MaxTier + CurrentSeason.BonusTiers;
}

// ===== Rewards =====

bool UMGBattlePassSubsystem::ClaimReward(int32 TierNumber, EMGBattlePassTrack Track)
{
    if (!IsRewardClaimable(TierNumber, Track))
    {
        return false;
    }

    FMGBattlePassReward Reward;

    if (TierNumber >= 1 && TierNumber <= CurrentSeason.Tiers.Num())
    {
        FMGBattlePassTierInfo& TierInfo = CurrentSeason.Tiers[TierNumber - 1];

        if (Track == EMGBattlePassTrack::Free)
        {
            Reward = TierInfo.FreeReward;
            TierInfo.FreeReward.bIsClaimed = true;
            PlayerProgress.ClaimedFreeTiers.AddUnique(TierNumber);
        }
        else
        {
            Reward = TierInfo.PremiumReward;
            TierInfo.PremiumReward.bIsClaimed = true;
            PlayerProgress.ClaimedPremiumTiers.AddUnique(TierNumber);
        }
    }

    OnRewardClaimed.Broadcast(Reward, Track);
    return true;
}

TArray<FMGBattlePassReward> UMGBattlePassSubsystem::ClaimAllAvailableRewards()
{
    TArray<FMGBattlePassReward> ClaimedRewards;

    for (int32 i = 1; i <= PlayerProgress.CurrentTier; i++)
    {
        if (IsRewardClaimable(i, EMGBattlePassTrack::Free))
        {
            FMGBattlePassTierInfo TierInfo = GetTierInfo(i);
            ClaimReward(i, EMGBattlePassTrack::Free);
            ClaimedRewards.Add(TierInfo.FreeReward);
        }

        if (PlayerProgress.bHasPremium && IsRewardClaimable(i, EMGBattlePassTrack::Premium))
        {
            FMGBattlePassTierInfo TierInfo = GetTierInfo(i);
            ClaimReward(i, EMGBattlePassTrack::Premium);
            ClaimedRewards.Add(TierInfo.PremiumReward);
        }
    }

    return ClaimedRewards;
}

TArray<FMGBattlePassReward> UMGBattlePassSubsystem::GetUnclaimedRewards() const
{
    TArray<FMGBattlePassReward> Unclaimed;

    for (int32 i = 1; i <= PlayerProgress.CurrentTier; i++)
    {
        FMGBattlePassTierInfo TierInfo = GetTierInfo(i);

        if (!PlayerProgress.ClaimedFreeTiers.Contains(i) && TierInfo.bFreeRewardAvailable)
        {
            Unclaimed.Add(TierInfo.FreeReward);
        }

        if (PlayerProgress.bHasPremium && !PlayerProgress.ClaimedPremiumTiers.Contains(i) && TierInfo.bPremiumRewardAvailable)
        {
            Unclaimed.Add(TierInfo.PremiumReward);
        }
    }

    return Unclaimed;
}

bool UMGBattlePassSubsystem::HasUnclaimedRewards() const
{
    return GetUnclaimedRewards().Num() > 0;
}

bool UMGBattlePassSubsystem::IsRewardClaimable(int32 TierNumber, EMGBattlePassTrack Track) const
{
    if (TierNumber < 1 || TierNumber > PlayerProgress.CurrentTier)
    {
        return false;
    }

    if (Track == EMGBattlePassTrack::Premium && !PlayerProgress.bHasPremium)
    {
        return false;
    }

    if (Track == EMGBattlePassTrack::Free)
    {
        return !PlayerProgress.ClaimedFreeTiers.Contains(TierNumber);
    }
    else
    {
        return !PlayerProgress.ClaimedPremiumTiers.Contains(TierNumber);
    }
}

TArray<FMGBattlePassReward> UMGBattlePassSubsystem::GetFeaturedRewards() const
{
    TArray<FMGBattlePassReward> Featured;

    for (const FMGBattlePassTierInfo& TierInfo : CurrentSeason.Tiers)
    {
        if (TierInfo.TierType == EMGBattlePassTier::Featured || TierInfo.TierType == EMGBattlePassTier::Ultimate)
        {
            if (TierInfo.bFreeRewardAvailable)
            {
                Featured.Add(TierInfo.FreeReward);
            }
            if (TierInfo.bPremiumRewardAvailable)
            {
                Featured.Add(TierInfo.PremiumReward);
            }
        }
    }

    return Featured;
}

// ===== Challenges =====

TArray<FMGBattlePassChallenge> UMGBattlePassSubsystem::GetDailyChallenges() const
{
    return CurrentSeason.DailyChallenges;
}

TArray<FMGBattlePassChallenge> UMGBattlePassSubsystem::GetWeeklyChallenges() const
{
    int32 CurrentWeek = GetCurrentWeekNumber();
    TArray<FMGBattlePassChallenge> WeekChallenges;

    for (const FMGBattlePassChallenge& Challenge : CurrentSeason.WeeklyChallenges)
    {
        if (Challenge.WeekNumber <= CurrentWeek)
        {
            WeekChallenges.Add(Challenge);
        }
    }

    return WeekChallenges;
}

TArray<FMGBattlePassChallenge> UMGBattlePassSubsystem::GetActiveChallenges() const
{
    TArray<FMGBattlePassChallenge> Active;

    for (const FMGBattlePassChallenge& Challenge : CurrentSeason.DailyChallenges)
    {
        if (!Challenge.bIsComplete)
        {
            Active.Add(Challenge);
        }
    }

    TArray<FMGBattlePassChallenge> WeeklyChallenges = GetWeeklyChallenges();
    for (const FMGBattlePassChallenge& Challenge : WeeklyChallenges)
    {
        if (!Challenge.bIsComplete)
        {
            Active.Add(Challenge);
        }
    }

    return Active;
}

void UMGBattlePassSubsystem::UpdateChallengeProgress(FName ChallengeId, float Progress)
{
    // Check daily challenges
    for (FMGBattlePassChallenge& Challenge : CurrentSeason.DailyChallenges)
    {
        if (Challenge.ChallengeId == ChallengeId && !Challenge.bIsComplete)
        {
            Challenge.CurrentValue = FMath::Min(Progress, Challenge.TargetValue);
            OnChallengeProgress.Broadcast(ChallengeId, Challenge.CurrentValue, Challenge.TargetValue);

            if (Challenge.CurrentValue >= Challenge.TargetValue)
            {
                Challenge.bIsComplete = true;
                OnChallengeComplete.Broadcast(Challenge);
            }
            return;
        }
    }

    // Check weekly challenges
    for (FMGBattlePassChallenge& Challenge : CurrentSeason.WeeklyChallenges)
    {
        if (Challenge.ChallengeId == ChallengeId && !Challenge.bIsComplete)
        {
            Challenge.CurrentValue = FMath::Min(Progress, Challenge.TargetValue);
            OnChallengeProgress.Broadcast(ChallengeId, Challenge.CurrentValue, Challenge.TargetValue);

            if (Challenge.CurrentValue >= Challenge.TargetValue)
            {
                Challenge.bIsComplete = true;
                OnChallengeComplete.Broadcast(Challenge);
            }
            return;
        }
    }
}

void UMGBattlePassSubsystem::UpdateChallengesByType(FName ChallengeType, float Progress)
{
    // Update daily challenges of this type
    for (FMGBattlePassChallenge& Challenge : CurrentSeason.DailyChallenges)
    {
        if (Challenge.ChallengeType == ChallengeType && !Challenge.bIsComplete)
        {
            Challenge.CurrentValue += Progress;
            Challenge.CurrentValue = FMath::Min(Challenge.CurrentValue, Challenge.TargetValue);
            OnChallengeProgress.Broadcast(Challenge.ChallengeId, Challenge.CurrentValue, Challenge.TargetValue);

            if (Challenge.CurrentValue >= Challenge.TargetValue)
            {
                Challenge.bIsComplete = true;
                OnChallengeComplete.Broadcast(Challenge);
            }
        }
    }

    // Update weekly challenges of this type
    for (FMGBattlePassChallenge& Challenge : CurrentSeason.WeeklyChallenges)
    {
        if (Challenge.ChallengeType == ChallengeType && !Challenge.bIsComplete && Challenge.WeekNumber <= GetCurrentWeekNumber())
        {
            Challenge.CurrentValue += Progress;
            Challenge.CurrentValue = FMath::Min(Challenge.CurrentValue, Challenge.TargetValue);
            OnChallengeProgress.Broadcast(Challenge.ChallengeId, Challenge.CurrentValue, Challenge.TargetValue);

            if (Challenge.CurrentValue >= Challenge.TargetValue)
            {
                Challenge.bIsComplete = true;
                OnChallengeComplete.Broadcast(Challenge);
            }
        }
    }
}

bool UMGBattlePassSubsystem::ClaimChallengeReward(FName ChallengeId)
{
    // Check daily challenges
    for (FMGBattlePassChallenge& Challenge : CurrentSeason.DailyChallenges)
    {
        if (Challenge.ChallengeId == ChallengeId)
        {
            if (!Challenge.bIsComplete || Challenge.bIsClaimed)
            {
                return false;
            }
            Challenge.bIsClaimed = true;
            AddXP(Challenge.XPReward, FName("Challenge"));
            return true;
        }
    }

    // Check weekly challenges
    for (FMGBattlePassChallenge& Challenge : CurrentSeason.WeeklyChallenges)
    {
        if (Challenge.ChallengeId == ChallengeId)
        {
            if (!Challenge.bIsComplete || Challenge.bIsClaimed)
            {
                return false;
            }
            Challenge.bIsClaimed = true;
            AddXP(Challenge.XPReward, FName("Challenge"));
            return true;
        }
    }

    return false;
}

void UMGBattlePassSubsystem::RefreshDailyChallenges()
{
    GenerateDailyChallenges();
    LastDailyChallengeRefresh = FDateTime::Now();
    OnChallengesRefreshed.Broadcast();
}

// ===== Premium =====

bool UMGBattlePassSubsystem::HasPremium() const
{
    return PlayerProgress.bHasPremium;
}

bool UMGBattlePassSubsystem::PurchasePremium()
{
    if (PlayerProgress.bHasPremium)
    {
        return false;
    }

    // In production, this would verify purchase through store
    PlayerProgress.bHasPremium = true;
    PlayerProgress.PremiumPurchaseDate = FDateTime::Now();

    OnPremiumPurchased.Broadcast();
    return true;
}

bool UMGBattlePassSubsystem::PurchaseBundle(FName BundleId)
{
    for (const FMGBattlePassBundle& Bundle : AvailableBundles)
    {
        if (Bundle.BundleId == BundleId)
        {
            // In production, this would verify purchase through store
            PlayerProgress.bHasPremium = true;
            PlayerProgress.PremiumPurchaseDate = FDateTime::Now();

            // Apply bonus tiers
            for (int32 i = 0; i < Bundle.BonusTiers; i++)
            {
                AddXP(CalculateXPForTier(PlayerProgress.CurrentTier), FName("BundleBonus"));
            }

            // Apply XP boost
            if (Bundle.XPBoostMultiplier > 1.0f)
            {
                XPMultiplier = Bundle.XPBoostMultiplier;
            }

            OnPremiumPurchased.Broadcast();
            return true;
        }
    }

    return false;
}

bool UMGBattlePassSubsystem::PurchaseTiers(int32 TierCount)
{
    if (TierCount <= 0)
    {
        return false;
    }

    // In production, this would verify purchase through store
    for (int32 i = 0; i < TierCount; i++)
    {
        AddXP(CalculateXPForTier(PlayerProgress.CurrentTier), FName("TierPurchase"));
    }

    PlayerProgress.TiersPurchased += TierCount;
    return true;
}

TArray<FMGBattlePassBundle> UMGBattlePassSubsystem::GetAvailableBundles() const
{
    return AvailableBundles;
}

int32 UMGBattlePassSubsystem::GetPremiumPrice() const
{
    return CurrentSeason.PremiumPrice;
}

int32 UMGBattlePassSubsystem::GetTierSkipPrice() const
{
    return CurrentSeason.TierSkipPrice;
}

// ===== XP Calculation =====

int32 UMGBattlePassSubsystem::CalculateRaceXP(int32 Position, int32 TotalRacers, float RaceTime) const
{
    int32 BaseXP = 100;

    // Position bonus
    if (Position == 1)
    {
        BaseXP += 200;
    }
    else if (Position == 2)
    {
        BaseXP += 150;
    }
    else if (Position == 3)
    {
        BaseXP += 100;
    }
    else if (Position <= TotalRacers / 2)
    {
        BaseXP += 50;
    }

    // Completion bonus
    BaseXP += 50;

    // Time bonus (longer races = more XP)
    BaseXP += FMath::Min(100, FMath::RoundToInt(RaceTime / 60.0f) * 10);

    return FMath::RoundToInt(BaseXP * XPMultiplier);
}

float UMGBattlePassSubsystem::GetXPMultiplier() const
{
    return XPMultiplier;
}

// ===== Protected =====

void UMGBattlePassSubsystem::InitializeSampleSeason()
{
    FDateTime Now = FDateTime::Now();

    CurrentSeason.SeasonId = TEXT("season_01_neon_nights");
    CurrentSeason.SeasonName = FText::FromString(TEXT("Season 1: Neon Nights"));
    CurrentSeason.SeasonTheme = FText::FromString(TEXT("Neon Nights"));
    CurrentSeason.Description = FText::FromString(TEXT("Light up the streets in the first season of Midnight Grind! Unlock exclusive neon-themed rewards."));
    CurrentSeason.SeasonNumber = 1;
    CurrentSeason.StartDate = Now - FTimespan::FromDays(30);
    CurrentSeason.EndDate = Now + FTimespan::FromDays(60);
    CurrentSeason.MaxTier = 100;
    CurrentSeason.BonusTiers = 20;
    CurrentSeason.PremiumPrice = 950;
    CurrentSeason.BundlePrice = 2500;
    CurrentSeason.TierSkipPrice = 150;

    // Generate tiers
    int32 TotalTiers = CurrentSeason.MaxTier + CurrentSeason.BonusTiers;
    int32 CumulativeXP = 0;

    TArray<FName> RewardTypes = {
        FName("Currency"), FName("Decal"), FName("Vinyl"), FName("Wheels"),
        FName("Spoiler"), FName("BodyKit"), FName("Underglow"), FName("NeonKit"),
        FName("HornSound"), FName("ExhaustEffect"), FName("TireSmoke"), FName("NitroTrail"),
        FName("PlayerCard"), FName("ProfileBanner"), FName("Avatar"), FName("Title"),
        FName("Emote"), FName("VictoryPose"), FName("LoadingScreen"), FName("XPBoost")
    };

    for (int32 i = 1; i <= TotalTiers; i++)
    {
        FMGBattlePassTierInfo TierInfo;
        TierInfo.TierNumber = i;
        TierInfo.XPRequired = CalculateXPForTier(i);
        CumulativeXP += TierInfo.XPRequired;
        TierInfo.CumulativeXP = CumulativeXP;

        // Determine tier type
        if (i == 1 || i == 100)
        {
            TierInfo.TierType = EMGBattlePassTier::Ultimate;
        }
        else if (i % 25 == 0)
        {
            TierInfo.TierType = EMGBattlePassTier::Featured;
        }
        else if (i % 10 == 0)
        {
            TierInfo.TierType = EMGBattlePassTier::Milestone;
        }
        else
        {
            TierInfo.TierType = EMGBattlePassTier::Standard;
        }

        // Generate free reward
        TierInfo.FreeReward.RewardId = FName(*FString::Printf(TEXT("free_tier_%d"), i));
        TierInfo.FreeReward.Tier = i;
        TierInfo.FreeReward.Track = EMGBattlePassTrack::Free;

        if (i % 5 == 0)
        {
            TierInfo.FreeReward.RewardType = EMGRewardType::Currency;
            TierInfo.FreeReward.DisplayName = FText::FromString(FString::Printf(TEXT("%d Neon Credits"), 500 + (i * 10)));
            TierInfo.FreeReward.Quantity = 500 + (i * 10);
            TierInfo.FreeReward.RarityLevel = FName("Common");
        }
        else
        {
            int32 TypeIndex = i % RewardTypes.Num();
            TierInfo.FreeReward.RewardType = static_cast<EMGRewardType>(TypeIndex % 24);
            TierInfo.FreeReward.DisplayName = FText::FromString(FString::Printf(TEXT("Neon Item %d"), i));
            TierInfo.FreeReward.Quantity = 1;
            TierInfo.FreeReward.RarityLevel = (i % 10 == 0) ? FName("Rare") : FName("Common");
        }

        // Generate premium reward
        TierInfo.PremiumReward.RewardId = FName(*FString::Printf(TEXT("premium_tier_%d"), i));
        TierInfo.PremiumReward.Tier = i;
        TierInfo.PremiumReward.Track = EMGBattlePassTrack::Premium;

        if (TierInfo.TierType == EMGBattlePassTier::Ultimate)
        {
            if (i == 1)
            {
                TierInfo.PremiumReward.RewardType = EMGRewardType::Vehicle;
                TierInfo.PremiumReward.DisplayName = FText::FromString(TEXT("Neon Striker - Season Pass Exclusive"));
                TierInfo.PremiumReward.RarityLevel = FName("Legendary");
            }
            else
            {
                TierInfo.PremiumReward.RewardType = EMGRewardType::Vehicle;
                TierInfo.PremiumReward.DisplayName = FText::FromString(TEXT("Neon Phantom - Ultimate Reward"));
                TierInfo.PremiumReward.RarityLevel = FName("Legendary");
            }
        }
        else if (TierInfo.TierType == EMGBattlePassTier::Featured)
        {
            TierInfo.PremiumReward.RewardType = EMGRewardType::BodyKit;
            TierInfo.PremiumReward.DisplayName = FText::FromString(FString::Printf(TEXT("Neon Featured Kit %d"), i / 25));
            TierInfo.PremiumReward.RarityLevel = FName("Epic");
        }
        else if (TierInfo.TierType == EMGBattlePassTier::Milestone)
        {
            TierInfo.PremiumReward.RewardType = EMGRewardType::Vinyl;
            TierInfo.PremiumReward.DisplayName = FText::FromString(FString::Printf(TEXT("Neon Milestone Vinyl %d"), i / 10));
            TierInfo.PremiumReward.RarityLevel = FName("Rare");
        }
        else
        {
            int32 TypeIndex = (i * 3) % RewardTypes.Num();
            TierInfo.PremiumReward.RewardType = static_cast<EMGRewardType>(TypeIndex % 24);
            TierInfo.PremiumReward.DisplayName = FText::FromString(FString::Printf(TEXT("Premium Neon Item %d"), i));
            TierInfo.PremiumReward.RarityLevel = (i % 7 == 0) ? FName("Epic") : FName("Rare");
        }
        TierInfo.PremiumReward.Quantity = 1;

        CurrentSeason.Tiers.Add(TierInfo);
    }

    // Generate challenges
    GenerateDailyChallenges();
    for (int32 Week = 1; Week <= 12; Week++)
    {
        GenerateWeeklyChallenges(Week);
    }

    // Initialize bundles
    FMGBattlePassBundle StandardBundle;
    StandardBundle.BundleId = FName("bundle_standard");
    StandardBundle.DisplayName = FText::FromString(TEXT("Battle Pass"));
    StandardBundle.Description = FText::FromString(TEXT("Unlock the premium track and all 100+ exclusive rewards!"));
    StandardBundle.Price = 950;
    StandardBundle.BonusTiers = 0;
    StandardBundle.XPBoostMultiplier = 1.0f;
    AvailableBundles.Add(StandardBundle);

    FMGBattlePassBundle PremiumBundle;
    PremiumBundle.BundleId = FName("bundle_premium");
    PremiumBundle.DisplayName = FText::FromString(TEXT("Battle Pass + 25 Tiers"));
    PremiumBundle.Description = FText::FromString(TEXT("Get the Battle Pass plus 25 tier skips to jumpstart your progress!"));
    PremiumBundle.Price = 2500;
    PremiumBundle.BonusTiers = 25;
    PremiumBundle.XPBoostMultiplier = 1.0f;
    AvailableBundles.Add(PremiumBundle);

    FMGBattlePassBundle UltimateBundle;
    UltimateBundle.BundleId = FName("bundle_ultimate");
    UltimateBundle.DisplayName = FText::FromString(TEXT("Ultimate Battle Pass"));
    UltimateBundle.Description = FText::FromString(TEXT("Battle Pass + 50 Tiers + XP Boost + Exclusive Animated Decal!"));
    UltimateBundle.Price = 4500;
    UltimateBundle.BonusTiers = 50;
    UltimateBundle.XPBoostMultiplier = 1.2f;
    AvailableBundles.Add(UltimateBundle);

    // Initialize player progress
    PlayerProgress.SeasonId = CurrentSeason.SeasonId;
    PlayerProgress.CurrentTier = 1;
    PlayerProgress.CurrentXP = 0;
    PlayerProgress.TotalXPEarned = 0;
    PlayerProgress.bHasPremium = false;
    PlayerProgress.LastPlayDate = FDateTime::Now();

    LastDailyChallengeRefresh = FDateTime::Now();
}

void UMGBattlePassSubsystem::GenerateDailyChallenges()
{
    CurrentSeason.DailyChallenges.Empty();

    // Generate 3 daily challenges
    TArray<TPair<FText, FName>> DailyChallengeTypes = {
        {FText::FromString(TEXT("Complete 3 Races")), FName("RaceComplete")},
        {FText::FromString(TEXT("Win 1 Race")), FName("RaceWin")},
        {FText::FromString(TEXT("Drift for 5,000m Total")), FName("DriftDistance")},
        {FText::FromString(TEXT("Reach 200 MPH")), FName("TopSpeed")},
        {FText::FromString(TEXT("Use Nitro 20 Times")), FName("NitroUse")},
        {FText::FromString(TEXT("Complete a Perfect Start")), FName("PerfectStart")},
        {FText::FromString(TEXT("Finish in Top 3")), FName("TopThree")},
        {FText::FromString(TEXT("Complete 5 Laps Without Crashing")), FName("CleanLaps")}
    };

    for (int32 i = 0; i < 3; i++)
    {
        int32 Index = FMath::RandRange(0, DailyChallengeTypes.Num() - 1);

        FMGBattlePassChallenge Challenge;
        Challenge.ChallengeId = FName(*FString::Printf(TEXT("daily_%d_%d"), FDateTime::Now().GetDayOfYear(), i));
        Challenge.Title = DailyChallengeTypes[Index].Key;
        Challenge.Description = DailyChallengeTypes[Index].Key;
        Challenge.ChallengeType = DailyChallengeTypes[Index].Value;
        Challenge.bIsWeekly = false;

        // Set target based on challenge type
        if (Challenge.ChallengeType == FName("RaceComplete"))
        {
            Challenge.TargetValue = 3.0f;
            Challenge.XPReward = 300;
        }
        else if (Challenge.ChallengeType == FName("RaceWin"))
        {
            Challenge.TargetValue = 1.0f;
            Challenge.XPReward = 500;
        }
        else if (Challenge.ChallengeType == FName("DriftDistance"))
        {
            Challenge.TargetValue = 5000.0f;
            Challenge.XPReward = 400;
        }
        else if (Challenge.ChallengeType == FName("TopSpeed"))
        {
            Challenge.TargetValue = 200.0f;
            Challenge.XPReward = 350;
        }
        else if (Challenge.ChallengeType == FName("NitroUse"))
        {
            Challenge.TargetValue = 20.0f;
            Challenge.XPReward = 250;
        }
        else
        {
            Challenge.TargetValue = 1.0f;
            Challenge.XPReward = 300;
        }

        CurrentSeason.DailyChallenges.Add(Challenge);
    }
}

void UMGBattlePassSubsystem::GenerateWeeklyChallenges(int32 WeekNumber)
{
    TArray<TPair<FText, FName>> WeeklyChallengeTypes = {
        {FText::FromString(TEXT("Complete 25 Races")), FName("RaceComplete")},
        {FText::FromString(TEXT("Win 10 Races")), FName("RaceWin")},
        {FText::FromString(TEXT("Drift for 100,000m Total")), FName("DriftDistance")},
        {FText::FromString(TEXT("Earn 50,000 Reputation")), FName("ReputationEarn")},
        {FText::FromString(TEXT("Complete 5 Time Trials")), FName("TimeTrial")},
        {FText::FromString(TEXT("Race on 10 Different Tracks")), FName("TrackVariety")},
        {FText::FromString(TEXT("Use 3 Different Vehicles")), FName("VehicleVariety")},
        {FText::FromString(TEXT("Complete 15 Online Races")), FName("OnlineRace")}
    };

    for (int32 i = 0; i < 4; i++)
    {
        int32 Index = (WeekNumber * 4 + i) % WeeklyChallengeTypes.Num();

        FMGBattlePassChallenge Challenge;
        Challenge.ChallengeId = FName(*FString::Printf(TEXT("weekly_w%d_%d"), WeekNumber, i));
        Challenge.Title = WeeklyChallengeTypes[Index].Key;
        Challenge.Description = FText::FromString(FString::Printf(TEXT("Week %d Challenge"), WeekNumber));
        Challenge.ChallengeType = WeeklyChallengeTypes[Index].Value;
        Challenge.bIsWeekly = true;
        Challenge.WeekNumber = WeekNumber;

        // Set target based on challenge type
        if (Challenge.ChallengeType == FName("RaceComplete"))
        {
            Challenge.TargetValue = 25.0f;
            Challenge.XPReward = 2500;
        }
        else if (Challenge.ChallengeType == FName("RaceWin"))
        {
            Challenge.TargetValue = 10.0f;
            Challenge.XPReward = 3500;
        }
        else if (Challenge.ChallengeType == FName("DriftDistance"))
        {
            Challenge.TargetValue = 100000.0f;
            Challenge.XPReward = 3000;
        }
        else if (Challenge.ChallengeType == FName("ReputationEarn"))
        {
            Challenge.TargetValue = 50000.0f;
            Challenge.XPReward = 2500;
        }
        else
        {
            Challenge.TargetValue = 10.0f;
            Challenge.XPReward = 2000;
        }

        CurrentSeason.WeeklyChallenges.Add(Challenge);
    }
}

void UMGBattlePassSubsystem::CheckTierProgression()
{
    int32 OldTier = PlayerProgress.CurrentTier;

    while (PlayerProgress.CurrentXP >= GetXPForNextTier() && PlayerProgress.CurrentTier < GetMaxTier())
    {
        PlayerProgress.CurrentXP -= GetXPForNextTier();
        PlayerProgress.CurrentTier++;

        // Get available rewards for new tier
        TArray<FMGBattlePassReward> AvailableRewards;
        FMGBattlePassTierInfo TierInfo = GetTierInfo(PlayerProgress.CurrentTier);
        AvailableRewards.Add(TierInfo.FreeReward);
        if (PlayerProgress.bHasPremium)
        {
            AvailableRewards.Add(TierInfo.PremiumReward);
        }

        OnTierUp.Broadcast(PlayerProgress.CurrentTier, AvailableRewards);
    }
}

int32 UMGBattlePassSubsystem::CalculateXPForTier(int32 TierNumber) const
{
    // Base XP increases slightly per tier
    int32 BaseXP = 1000;

    if (TierNumber <= 10)
    {
        return BaseXP;
    }
    else if (TierNumber <= 50)
    {
        return BaseXP + ((TierNumber - 10) * 25);
    }
    else if (TierNumber <= 100)
    {
        return BaseXP + 1000 + ((TierNumber - 50) * 50);
    }
    else
    {
        // Bonus tiers require more XP
        return BaseXP + 3500 + ((TierNumber - 100) * 100);
    }
}

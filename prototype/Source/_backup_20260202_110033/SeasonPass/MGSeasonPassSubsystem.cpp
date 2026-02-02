// Copyright Midnight Grind. All Rights Reserved.

#include "SeasonPass/MGSeasonPassSubsystem.h"
#include "Currency/MGCurrencySubsystem.h"
#include "Store/MGStoreSubsystem.h"

void UMGSeasonPassSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeCurrentSeason();
	LoadSeasonData();
	GenerateChallenges();
}

void UMGSeasonPassSubsystem::Deinitialize()
{
	SaveProgress();
	Super::Deinitialize();
}

FTimespan UMGSeasonPassSubsystem::GetTimeRemaining() const
{
	FDateTime Now = FDateTime::UtcNow();
	if (Now > CurrentSeason.EndDate)
		return FTimespan::Zero();
	return CurrentSeason.EndDate - Now;
}

float UMGSeasonPassSubsystem::GetSeasonProgressPercent() const
{
	FDateTime Now = FDateTime::UtcNow();
	FTimespan TotalDuration = CurrentSeason.EndDate - CurrentSeason.StartDate;
	FTimespan Elapsed = Now - CurrentSeason.StartDate;

	if (TotalDuration.GetTotalSeconds() <= 0)
		return 100.0f;

	return FMath::Clamp(100.0f * (Elapsed.GetTotalSeconds() / TotalDuration.GetTotalSeconds()), 0.0f, 100.0f);
}

void UMGSeasonPassSubsystem::AddXP(int64 Amount, const FString& Source)
{
	if (Amount <= 0) return;

	// Apply catch-up bonus if eligible
	float CatchUpMultiplier = 1.0f;
	if (IsEligibleForCatchUp())
	{
		CatchUpMultiplier = 1.5f; // 50% bonus XP when behind
	}

	int64 FinalXP = FMath::RoundToInt64(Amount * CatchUpMultiplier);

	Progress.CurrentXP += FinalXP;
	Progress.TotalXPEarned += FinalXP;

	OnSeasonXPGained.Broadcast(FinalXP, Progress.TotalXPEarned);
	CheckTierUp();
	SaveProgress();
}

int64 UMGSeasonPassSubsystem::GetXPForTier(int32 Tier) const
{
	if (Tier <= 0 || Tier > CurrentSeason.MaxTier)
		return 0;

	// Fair XP curve - not exponential grind
	// ~1000 XP per tier, slight increase at milestones
	int64 BaseXP = 1000;
	int64 TierBonus = (Tier / 10) * 100; // Extra 100 XP every 10 tiers
	return BaseXP + TierBonus;
}

int64 UMGSeasonPassSubsystem::GetXPToNextTier() const
{
	if (Progress.CurrentTier >= CurrentSeason.MaxTier)
		return 0;

	int64 Required = GetXPForTier(Progress.CurrentTier + 1);
	return FMath::Max(0LL, Required - Progress.CurrentXP);
}

float UMGSeasonPassSubsystem::GetTierProgressPercent() const
{
	if (Progress.CurrentTier >= CurrentSeason.MaxTier)
		return 100.0f;

	int64 Required = GetXPForTier(Progress.CurrentTier + 1);
	if (Required <= 0)
		return 100.0f;

	return FMath::Clamp(100.0f * ((float)Progress.CurrentXP / (float)Required), 0.0f, 100.0f);
}

bool UMGSeasonPassSubsystem::ClaimTierReward(int32 Tier, bool bPremium)
{
	if (!CanClaimReward(Tier, bPremium))
		return false;

	FMGSeasonTier TierInfo = GetTierInfo(Tier);
	const FMGSeasonReward& Reward = bPremium ? TierInfo.PremiumReward : TierInfo.FreeReward;

	GrantReward(Reward);

	if (bPremium)
		Progress.ClaimedPremiumTiers.Add(Tier);
	else
		Progress.ClaimedFreeTiers.Add(Tier);

	OnSeasonRewardClaimed.Broadcast(Tier, bPremium);
	SaveProgress();

	return true;
}

bool UMGSeasonPassSubsystem::CanClaimReward(int32 Tier, bool bPremium) const
{
	// Must have reached the tier
	if (Progress.CurrentTier < Tier)
		return false;

	// Must have premium pass for premium rewards
	if (bPremium && !Progress.bHasPremiumPass)
		return false;

	// Check if already claimed
	if (bPremium)
		return !Progress.ClaimedPremiumTiers.Contains(Tier);
	else
		return !Progress.ClaimedFreeTiers.Contains(Tier);
}

TArray<int32> UMGSeasonPassSubsystem::GetUnclaimedTiers(bool bPremium) const
{
	TArray<int32> Unclaimed;
	const TArray<int32>& ClaimedList = bPremium ? Progress.ClaimedPremiumTiers : Progress.ClaimedFreeTiers;

	for (int32 Tier = 1; Tier <= Progress.CurrentTier; Tier++)
	{
		if (!ClaimedList.Contains(Tier))
		{
			if (!bPremium || Progress.bHasPremiumPass)
			{
				Unclaimed.Add(Tier);
			}
		}
	}
	return Unclaimed;
}

void UMGSeasonPassSubsystem::ClaimAllAvailableRewards()
{
	TArray<int32> UnclaimedFree = GetUnclaimedTiers(false);
	for (int32 Tier : UnclaimedFree)
	{
		ClaimTierReward(Tier, false);
	}

	if (Progress.bHasPremiumPass)
	{
		TArray<int32> UnclaimedPremium = GetUnclaimedTiers(true);
		for (int32 Tier : UnclaimedPremium)
		{
			ClaimTierReward(Tier, true);
		}
	}
}

FMGSeasonTier UMGSeasonPassSubsystem::GetTierInfo(int32 Tier) const
{
	if (Tier > 0 && Tier <= CurrentSeason.Tiers.Num())
	{
		return CurrentSeason.Tiers[Tier - 1];
	}
	return FMGSeasonTier();
}

bool UMGSeasonPassSubsystem::PurchasePremiumPass()
{
	if (Progress.bHasPremiumPass)
		return false;

	UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>();
	if (!Currency)
		return false;

	int64 Price = GetPremiumPassPrice();
	if (!Currency->CanAfford(EMGCurrencyType::NeonCredits, Price))
		return false;

	Currency->SpendCurrency(EMGCurrencyType::NeonCredits, Price, TEXT("Premium Season Pass"));
	Progress.bHasPremiumPass = true;

	OnPremiumPassPurchased.Broadcast();
	SaveProgress();

	return true;
}

TArray<FMGSeasonChallenge> UMGSeasonPassSubsystem::GetDailyChallenges() const
{
	return DailyChallenges;
}

TArray<FMGSeasonChallenge> UMGSeasonPassSubsystem::GetWeeklyChallenges() const
{
	return WeeklyChallenges;
}

void UMGSeasonPassSubsystem::UpdateChallengeProgress(FName ChallengeID, int32 ProgressDelta)
{
	auto UpdateChallenge = [&](TArray<FMGSeasonChallenge>& Challenges)
	{
		for (FMGSeasonChallenge& Challenge : Challenges)
		{
			if (Challenge.ChallengeID == ChallengeID && !Challenge.bCompleted)
			{
				Challenge.CurrentProgress = FMath::Min(Challenge.CurrentProgress + ProgressDelta, Challenge.TargetProgress);

				if (Challenge.CurrentProgress >= Challenge.TargetProgress)
				{
					Challenge.bCompleted = true;
					AddXP(Challenge.XPReward, TEXT("Challenge completion"));
					OnChallengeCompleted.Broadcast(Challenge);
				}
				return true;
			}
		}
		return false;
	};

	if (!UpdateChallenge(DailyChallenges))
		UpdateChallenge(WeeklyChallenges);
}

int64 UMGSeasonPassSubsystem::GetCatchUpXPBonus() const
{
	if (!IsEligibleForCatchUp())
		return 0;

	// Calculate how much XP player should have at this point
	float SeasonProgress = GetSeasonProgressPercent() / 100.0f;
	int32 ExpectedTier = FMath::RoundToInt(CurrentSeason.MaxTier * SeasonProgress);

	if (Progress.CurrentTier >= ExpectedTier)
		return 0;

	// 50% bonus XP for being behind
	return 50;
}

bool UMGSeasonPassSubsystem::IsEligibleForCatchUp() const
{
	float SeasonProgress = GetSeasonProgressPercent() / 100.0f;
	float PlayerProgress = (float)Progress.CurrentTier / (float)CurrentSeason.MaxTier;

	// Eligible if player is more than 10 tiers behind expected
	return (SeasonProgress - PlayerProgress) > 0.1f;
}

void UMGSeasonPassSubsystem::LoadSeasonData()
{
	// Would load from cloud save
}

void UMGSeasonPassSubsystem::SaveProgress()
{
	// Would save to cloud save
}

void UMGSeasonPassSubsystem::InitializeCurrentSeason()
{
	CurrentSeason.SeasonID = FName(TEXT("Season_01"));
	CurrentSeason.SeasonName = FText::FromString(TEXT("Neon Nights"));
	CurrentSeason.SeasonTheme = FText::FromString(TEXT("The streets come alive after dark"));
	CurrentSeason.SeasonNumber = 1;
	CurrentSeason.StartDate = FDateTime::UtcNow();
	CurrentSeason.EndDate = FDateTime::UtcNow() + FTimespan::FromDays(90); // 3 month seasons
	CurrentSeason.MaxTier = 100;

	// Generate tiers with fair, meaningful rewards
	for (int32 i = 1; i <= 100; i++)
	{
		FMGSeasonTier Tier;
		Tier.TierNumber = i;
		Tier.XPRequired = GetXPForTier(i);
		Tier.bIsMilestone = (i % 10 == 0);

		// Free track - meaningful rewards, not just scraps
		if (i % 5 == 0)
		{
			// Every 5 tiers: GrindCash
			Tier.FreeReward.Type = EMGSeasonRewardType::GrindCash;
			Tier.FreeReward.CurrencyAmount = 1000 + (i * 50);
			Tier.FreeReward.DisplayName = FText::FromString(FString::Printf(TEXT("%lld Grind Cash"), Tier.FreeReward.CurrencyAmount));
		}
		else if (i % 10 == 0)
		{
			// Every 10 tiers: Free Neon Credits!
			Tier.FreeReward.Type = EMGSeasonRewardType::NeonCredits;
			Tier.FreeReward.CurrencyAmount = 50;
			Tier.FreeReward.DisplayName = FText::FromString(TEXT("50 Neon Credits"));
		}
		else if (i % 25 == 0)
		{
			// Every 25 tiers: Free cosmetic
			Tier.FreeReward.Type = EMGSeasonRewardType::Livery;
			Tier.FreeReward.ItemID = FName(*FString::Printf(TEXT("Season1_Livery_%d"), i / 25));
			Tier.FreeReward.DisplayName = FText::FromString(TEXT("Season Livery"));
		}
		else
		{
			// Regular tiers: Smaller cash rewards
			Tier.FreeReward.Type = EMGSeasonRewardType::GrindCash;
			Tier.FreeReward.CurrencyAmount = 250 + (i * 10);
			Tier.FreeReward.DisplayName = FText::FromString(FString::Printf(TEXT("%lld Grind Cash"), Tier.FreeReward.CurrencyAmount));
		}

		// Premium track - extra cosmetics (not gameplay advantages)
		if (Tier.bIsMilestone)
		{
			// Milestone tiers: Exclusive cosmetics
			Tier.PremiumReward.Type = (i % 20 == 0) ? EMGSeasonRewardType::Livery : EMGSeasonRewardType::Decal;
			Tier.PremiumReward.ItemID = FName(*FString::Printf(TEXT("Season1_Premium_%d"), i));
			Tier.PremiumReward.DisplayName = FText::FromString(TEXT("Premium Cosmetic"));
		}
		else
		{
			// Non-milestone: XP/Currency boosts
			Tier.PremiumReward.Type = EMGSeasonRewardType::XPBoost;
			Tier.PremiumReward.BoostMultiplier = 1.25f;
			Tier.PremiumReward.BoostDurationHours = 2.0f;
			Tier.PremiumReward.DisplayName = FText::FromString(TEXT("2hr XP Boost"));
		}

		CurrentSeason.Tiers.Add(Tier);
	}

	Progress.CurrentTier = 1;
	Progress.CurrentXP = 0;
}

void UMGSeasonPassSubsystem::GenerateChallenges()
{
	DailyChallenges.Empty();
	WeeklyChallenges.Empty();

	// Daily challenges - simple, achievable in 1-2 races
	FMGSeasonChallenge Daily1;
	Daily1.ChallengeID = FName(TEXT("Daily_Races"));
	Daily1.Description = FText::FromString(TEXT("Complete 3 races"));
	Daily1.XPReward = 500;
	Daily1.TargetProgress = 3;
	Daily1.bIsWeekly = false;
	Daily1.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(24);
	DailyChallenges.Add(Daily1);

	FMGSeasonChallenge Daily2;
	Daily2.ChallengeID = FName(TEXT("Daily_Clean"));
	Daily2.Description = FText::FromString(TEXT("Finish a clean race"));
	Daily2.XPReward = 300;
	Daily2.TargetProgress = 1;
	Daily2.bIsWeekly = false;
	Daily2.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(24);
	DailyChallenges.Add(Daily2);

	FMGSeasonChallenge Daily3;
	Daily3.ChallengeID = FName(TEXT("Daily_Podium"));
	Daily3.Description = FText::FromString(TEXT("Finish on the podium"));
	Daily3.XPReward = 400;
	Daily3.TargetProgress = 1;
	Daily3.bIsWeekly = false;
	Daily3.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(24);
	DailyChallenges.Add(Daily3);

	// Weekly challenges - more substantial but achievable
	FMGSeasonChallenge Weekly1;
	Weekly1.ChallengeID = FName(TEXT("Weekly_Wins"));
	Weekly1.Description = FText::FromString(TEXT("Win 5 races"));
	Weekly1.XPReward = 2000;
	Weekly1.TargetProgress = 5;
	Weekly1.bIsWeekly = true;
	Weekly1.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromDays(7);
	WeeklyChallenges.Add(Weekly1);

	FMGSeasonChallenge Weekly2;
	Weekly2.ChallengeID = FName(TEXT("Weekly_Distance"));
	Weekly2.Description = FText::FromString(TEXT("Drive 100 miles"));
	Weekly2.XPReward = 1500;
	Weekly2.TargetProgress = 100;
	Weekly2.bIsWeekly = true;
	Weekly2.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromDays(7);
	WeeklyChallenges.Add(Weekly2);

	FMGSeasonChallenge Weekly3;
	Weekly3.ChallengeID = FName(TEXT("Weekly_Crew"));
	Weekly3.Description = FText::FromString(TEXT("Race with crew members 3 times"));
	Weekly3.XPReward = 1000;
	Weekly3.TargetProgress = 3;
	Weekly3.bIsWeekly = true;
	Weekly3.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromDays(7);
	WeeklyChallenges.Add(Weekly3);
}

void UMGSeasonPassSubsystem::CheckTierUp()
{
	while (Progress.CurrentTier < CurrentSeason.MaxTier)
	{
		int64 RequiredXP = GetXPForTier(Progress.CurrentTier + 1);
		if (Progress.CurrentXP >= RequiredXP)
		{
			Progress.CurrentXP -= RequiredXP;
			Progress.CurrentTier++;
			OnSeasonTierReached.Broadcast(Progress.CurrentTier);
		}
		else
		{
			break;
		}
	}
}

void UMGSeasonPassSubsystem::GrantReward(const FMGSeasonReward& Reward)
{
	UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>();
	UMGStoreSubsystem* Store = GetGameInstance()->GetSubsystem<UMGStoreSubsystem>();

	switch (Reward.Type)
	{
	case EMGSeasonRewardType::GrindCash:
		if (Currency)
			Currency->EarnCurrency(EMGCurrencyType::GrindCash, Reward.CurrencyAmount, EMGEarnSource::SeasonMilestone, TEXT("Season Pass"));
		break;

	case EMGSeasonRewardType::NeonCredits:
		if (Currency)
			Currency->EarnCurrency(EMGCurrencyType::NeonCredits, Reward.CurrencyAmount, EMGEarnSource::SeasonMilestone, TEXT("Season Pass"));
		break;

	case EMGSeasonRewardType::Livery:
	case EMGSeasonRewardType::Decal:
	case EMGSeasonRewardType::Wheels:
	case EMGSeasonRewardType::Neon:
	case EMGSeasonRewardType::Horn:
	case EMGSeasonRewardType::Trail:
	case EMGSeasonRewardType::Emote:
	case EMGSeasonRewardType::Avatar:
	case EMGSeasonRewardType::Banner:
	case EMGSeasonRewardType::Title:
		if (Store)
			Store->UnlockItem(Reward.ItemID, TEXT("SeasonPass"), TEXT(""));
		break;

	case EMGSeasonRewardType::XPBoost:
	case EMGSeasonRewardType::CurrencyBoost:
		if (Currency)
		{
			FMGEarningMultiplier Boost;
			Boost.MultiplierID = FName(*FString::Printf(TEXT("SeasonBoost_%s"), *FGuid::NewGuid().ToString()));
			Boost.Multiplier = Reward.BoostMultiplier;
			Boost.AffectedCurrency = (Reward.Type == EMGSeasonRewardType::XPBoost) ? EMGCurrencyType::SeasonPoints : EMGCurrencyType::GrindCash;
			Boost.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(Reward.BoostDurationHours);
			Currency->AddMultiplier(Boost);
		}
		break;
	}
}

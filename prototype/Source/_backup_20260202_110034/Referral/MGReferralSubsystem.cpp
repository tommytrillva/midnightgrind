// Copyright Midnight Grind. All Rights Reserved.

#include "Referral/MGReferralSubsystem.h"
#include "HAL/PlatformApplicationMisc.h"

void UMGReferralSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeMilestones();

	// Generate initial referral code
	MyReferralCode = GenerateReferralCode();
}

void UMGReferralSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// Referral Codes
FMGReferralCode UMGReferralSubsystem::GenerateReferralCode()
{
	FMGReferralCode Code;
	Code.Code = GenerateCodeString();
	Code.OwnerPlayerID = TEXT("LocalPlayer");
	Code.CreatedTime = FDateTime::Now();
	Code.ExpiryTime = FDateTime::Now() + FTimespan::FromDays(365);
	Code.MaxUses = 0; // Unlimited
	Code.CurrentUses = 0;
	Code.bIsActive = true;

	MyReferralCode = Code;
	OnReferralCodeGenerated.Broadcast(Code);
	return Code;
}

FMGReferralCode UMGReferralSubsystem::GenerateCustomCode(const FString& CustomCode, int32 MaxUses)
{
	FMGReferralCode Code;
	Code.Code = CustomCode.ToUpper();
	Code.OwnerPlayerID = TEXT("LocalPlayer");
	Code.CreatedTime = FDateTime::Now();
	Code.ExpiryTime = FDateTime::Now() + FTimespan::FromDays(30);
	Code.MaxUses = MaxUses;
	Code.CurrentUses = 0;
	Code.bIsActive = true;

	OnReferralCodeGenerated.Broadcast(Code);
	return Code;
}

FString UMGReferralSubsystem::GetReferralLink() const
{
	return BaseReferralURL + MyReferralCode.Code;
}

void UMGReferralSubsystem::CopyReferralCodeToClipboard()
{
	FPlatformApplicationMisc::ClipboardCopy(*MyReferralCode.Code);
}

void UMGReferralSubsystem::ShareReferralCode()
{
	// Would trigger platform share dialog
	FString ShareText = FString::Printf(
		TEXT("Join me in Midnight Grind! Use my referral code: %s\n%s"),
		*MyReferralCode.Code,
		*GetReferralLink()
	);

	// Platform-specific sharing would go here
}

// Applying Codes
bool UMGReferralSubsystem::ApplyReferralCode(const FString& Code)
{
	if (bHasAppliedCode)
	{
		OnReferralCodeInvalid.Broadcast(Code, TEXT("Already applied a referral code"));
		return false;
	}

	if (!ValidateCode(Code))
	{
		return false;
	}

	// Create applied code record
	AppliedCode.Code = Code.ToUpper();
	AppliedCode.CreatedTime = FDateTime::Now();
	bHasAppliedCode = true;

	// Grant welcome rewards to referred player
	FMGReferralReward WelcomeReward;
	WelcomeReward.RewardID = FName("WELCOME_BONUS");
	WelcomeReward.Type = EMGReferralRewardType::Currency;
	WelcomeReward.DisplayName = FText::FromString(TEXT("Welcome Bonus"));
	WelcomeReward.Description = FText::FromString(TEXT("Bonus cash for joining via referral"));
	WelcomeReward.Amount = 10000;
	WelcomeReward.bForReferred = true;
	WelcomeReward.bForReferrer = false;

	PendingRewards.Add(WelcomeReward);
	OnReferralRewardAvailable.Broadcast(WelcomeReward);

	OnReferralCodeApplied.Broadcast(AppliedCode);
	return true;
}

bool UMGReferralSubsystem::ValidateCode(const FString& Code)
{
	FString UpperCode = Code.ToUpper();

	if (UpperCode.Len() < 4 || UpperCode.Len() > 16)
	{
		OnReferralCodeInvalid.Broadcast(Code, TEXT("Invalid code format"));
		return false;
	}

	// Check if it's our own code
	if (UpperCode == MyReferralCode.Code)
	{
		OnReferralCodeInvalid.Broadcast(Code, TEXT("Cannot use your own referral code"));
		return false;
	}

	// Would validate against server
	return true;
}

// Referral Tracking
TArray<FMGReferredPlayer> UMGReferralSubsystem::GetReferredPlayersByStatus(EMGReferralStatus Status) const
{
	TArray<FMGReferredPlayer> Result;

	for (const FMGReferredPlayer& Player : ReferredPlayers)
	{
		if (Player.Status == Status)
		{
			Result.Add(Player);
		}
	}

	return Result;
}

float UMGReferralSubsystem::GetTierProgress() const
{
	int32 CurrentCount = Stats.CompletedReferrals;

	int32 TierMin = 0;
	int32 TierMax = 0;

	switch (Stats.CurrentTier)
	{
	case EMGReferralTier::Bronze:
		TierMin = 0; TierMax = 3;
		break;
	case EMGReferralTier::Silver:
		TierMin = 3; TierMax = 6;
		break;
	case EMGReferralTier::Gold:
		TierMin = 6; TierMax = 11;
		break;
	case EMGReferralTier::Platinum:
		TierMin = 11; TierMax = 26;
		break;
	case EMGReferralTier::Diamond:
		TierMin = 26; TierMax = 51;
		break;
	case EMGReferralTier::Ambassador:
		return 1.0f;
	}

	if (TierMax == TierMin)
	{
		return 1.0f;
	}

	return static_cast<float>(CurrentCount - TierMin) / (TierMax - TierMin);
}

// Rewards
bool UMGReferralSubsystem::ClaimReward(FName RewardID)
{
	int32 Index = PendingRewards.IndexOfByPredicate([RewardID](const FMGReferralReward& R)
	{
		return R.RewardID == RewardID;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	FMGReferralReward Reward = PendingRewards[Index];
	PendingRewards.RemoveAt(Index);

	GrantReward(Reward);
	OnReferralRewardClaimed.Broadcast(Reward);

	return true;
}

void UMGReferralSubsystem::ClaimAllRewards()
{
	TArray<FMGReferralReward> RewardsToClaim = PendingRewards;
	PendingRewards.Empty();

	for (const FMGReferralReward& Reward : RewardsToClaim)
	{
		GrantReward(Reward);
		OnReferralRewardClaimed.Broadcast(Reward);
	}
}

// Milestones
FMGReferralMilestone UMGReferralSubsystem::GetNextMilestone() const
{
	for (const FMGReferralMilestone& Milestone : Milestones)
	{
		if (Stats.CompletedReferrals < Milestone.RequiredReferrals)
		{
			return Milestone;
		}
	}

	return FMGReferralMilestone();
}

int32 UMGReferralSubsystem::GetReferralsToNextMilestone() const
{
	FMGReferralMilestone Next = GetNextMilestone();
	if (Next.RequiredReferrals == 0)
	{
		return 0;
	}

	return Next.RequiredReferrals - Stats.CompletedReferrals;
}

bool UMGReferralSubsystem::ClaimMilestoneReward(int32 MilestoneIndex)
{
	if (!Milestones.IsValidIndex(MilestoneIndex))
	{
		return false;
	}

	FMGReferralMilestone& Milestone = Milestones[MilestoneIndex];

	if (Milestone.bIsClaimed)
	{
		return false;
	}

	if (Stats.CompletedReferrals < Milestone.RequiredReferrals)
	{
		return false;
	}

	Milestone.bIsClaimed = true;

	for (const FMGReferralReward& Reward : Milestone.Rewards)
	{
		GrantReward(Reward);
		OnReferralRewardClaimed.Broadcast(Reward);
	}

	return true;
}

// Progress Reporting
void UMGReferralSubsystem::ReportTutorialComplete()
{
	if (!bHasAppliedCode)
	{
		return;
	}

	// Would notify referrer's server about progress
}

void UMGReferralSubsystem::ReportLevelReached(int32 Level)
{
	if (!bHasAppliedCode)
	{
		return;
	}

	// Would notify referrer's server about progress
}

void UMGReferralSubsystem::ReportFirstWin()
{
	if (!bHasAppliedCode)
	{
		return;
	}

	// Would notify referrer's server about progress
}

void UMGReferralSubsystem::ReportPurchase()
{
	if (!bHasAppliedCode)
	{
		return;
	}

	// Would notify referrer's server about progress
}

// Social
void UMGReferralSubsystem::InviteFriend(const FString& FriendID)
{
	if (!InvitedFriends.Contains(FriendID))
	{
		InvitedFriends.Add(FriendID);
	}

	// Would send invite through online subsystem
}

void UMGReferralSubsystem::InviteFriendByEmail(const FString& Email)
{
	// Would trigger email invite
}

// Internal
void UMGReferralSubsystem::InitializeMilestones()
{
	Milestones.Empty();

	// 1 Referral
	{
		FMGReferralMilestone M;
		M.RequiredReferrals = 1;
		M.MilestoneName = FText::FromString(TEXT("First Friend"));

		FMGReferralReward R;
		R.RewardID = FName("MILESTONE_1");
		R.Type = EMGReferralRewardType::Currency;
		R.DisplayName = FText::FromString(TEXT("First Friend Bonus"));
		R.Amount = 5000;
		M.Rewards.Add(R);

		Milestones.Add(M);
	}

	// 3 Referrals
	{
		FMGReferralMilestone M;
		M.RequiredReferrals = 3;
		M.MilestoneName = FText::FromString(TEXT("Growing Crew"));

		FMGReferralReward R;
		R.RewardID = FName("MILESTONE_3");
		R.Type = EMGReferralRewardType::XPBoost;
		R.DisplayName = FText::FromString(TEXT("24h XP Boost"));
		R.Amount = 24;
		M.Rewards.Add(R);

		Milestones.Add(M);
	}

	// 5 Referrals
	{
		FMGReferralMilestone M;
		M.RequiredReferrals = 5;
		M.MilestoneName = FText::FromString(TEXT("Squad Goals"));

		FMGReferralReward R;
		R.RewardID = FName("MILESTONE_5");
		R.Type = EMGReferralRewardType::Cosmetic;
		R.DisplayName = FText::FromString(TEXT("Exclusive Decal Pack"));
		R.ItemID = FName("DECAL_REFERRAL_01");
		M.Rewards.Add(R);

		Milestones.Add(M);
	}

	// 10 Referrals
	{
		FMGReferralMilestone M;
		M.RequiredReferrals = 10;
		M.MilestoneName = FText::FromString(TEXT("Crew Leader"));

		FMGReferralReward R;
		R.RewardID = FName("MILESTONE_10");
		R.Type = EMGReferralRewardType::UniqueLivery;
		R.DisplayName = FText::FromString(TEXT("Referral Champion Livery"));
		R.ItemID = FName("LIVERY_REFERRAL_CHAMP");
		M.Rewards.Add(R);

		Milestones.Add(M);
	}

	// 25 Referrals
	{
		FMGReferralMilestone M;
		M.RequiredReferrals = 25;
		M.MilestoneName = FText::FromString(TEXT("Community Builder"));

		FMGReferralReward R;
		R.RewardID = FName("MILESTONE_25");
		R.Type = EMGReferralRewardType::Vehicle;
		R.DisplayName = FText::FromString(TEXT("Exclusive Referral Vehicle"));
		R.ItemID = FName("VEH_REFERRAL_EXCLUSIVE");
		M.Rewards.Add(R);

		Milestones.Add(M);
	}

	// 50 Referrals
	{
		FMGReferralMilestone M;
		M.RequiredReferrals = 50;
		M.MilestoneName = FText::FromString(TEXT("Ambassador"));

		FMGReferralReward R;
		R.RewardID = FName("MILESTONE_50");
		R.Type = EMGReferralRewardType::UniqueTitle;
		R.DisplayName = FText::FromString(TEXT("Ambassador Title"));
		R.ItemID = FName("TITLE_AMBASSADOR");
		M.Rewards.Add(R);

		FMGReferralReward R2;
		R2.RewardID = FName("MILESTONE_50_BONUS");
		R2.Type = EMGReferralRewardType::Currency;
		R2.DisplayName = FText::FromString(TEXT("Ambassador Bonus"));
		R2.Amount = 100000;
		M.Rewards.Add(R2);

		Milestones.Add(M);
	}
}

void UMGReferralSubsystem::UpdateStats()
{
	Stats.TotalReferrals = ReferredPlayers.Num();
	Stats.PendingReferrals = 0;
	Stats.CompletedReferrals = 0;
	Stats.ExpiredReferrals = 0;

	for (const FMGReferredPlayer& Player : ReferredPlayers)
	{
		switch (Player.Status)
		{
		case EMGReferralStatus::Pending:
		case EMGReferralStatus::Registered:
		case EMGReferralStatus::FirstLogin:
		case EMGReferralStatus::TutorialComplete:
		case EMGReferralStatus::ReachedLevel5:
			Stats.PendingReferrals++;
			break;

		case EMGReferralStatus::ReachedLevel10:
		case EMGReferralStatus::FirstWin:
		case EMGReferralStatus::PurchasedPremium:
		case EMGReferralStatus::Claimed:
			Stats.CompletedReferrals++;
			break;

		case EMGReferralStatus::Expired:
			Stats.ExpiredReferrals++;
			break;
		}
	}

	UpdateTier();
	CheckMilestones();
}

void UMGReferralSubsystem::UpdateTier()
{
	EMGReferralTier OldTier = Stats.CurrentTier;
	Stats.CurrentTier = CalculateTier(Stats.CompletedReferrals);

	if (OldTier != Stats.CurrentTier)
	{
		OnTierChanged.Broadcast(OldTier, Stats.CurrentTier);
	}
}

void UMGReferralSubsystem::CheckMilestones()
{
	for (int32 i = 0; i < Milestones.Num(); i++)
	{
		const FMGReferralMilestone& Milestone = Milestones[i];

		if (!Milestone.bIsClaimed && Stats.CompletedReferrals >= Milestone.RequiredReferrals)
		{
			OnMilestoneReached.Broadcast(Milestone);
		}
	}
}

void UMGReferralSubsystem::GrantReward(const FMGReferralReward& Reward)
{
	// Would grant through economy/inventory subsystem
	switch (Reward.Type)
	{
	case EMGReferralRewardType::Currency:
		Stats.TotalCurrencyEarned += Reward.Amount;
		break;

	case EMGReferralRewardType::Vehicle:
	case EMGReferralRewardType::Part:
	case EMGReferralRewardType::Cosmetic:
	case EMGReferralRewardType::UniqueTitle:
	case EMGReferralRewardType::UniqueLivery:
	case EMGReferralRewardType::ExclusiveDecal:
		Stats.TotalItemsEarned++;
		break;

	default:
		break;
	}
}

FString UMGReferralSubsystem::GenerateCodeString() const
{
	const FString Chars = TEXT("ABCDEFGHJKLMNPQRSTUVWXYZ23456789");
	FString Code;

	for (int32 i = 0; i < 8; i++)
	{
		int32 Index = FMath::RandRange(0, Chars.Len() - 1);
		Code.AppendChar(Chars[Index]);
	}

	return Code;
}

EMGReferralTier UMGReferralSubsystem::CalculateTier(int32 ReferralCount) const
{
	if (ReferralCount >= 50) return EMGReferralTier::Ambassador;
	if (ReferralCount >= 26) return EMGReferralTier::Diamond;
	if (ReferralCount >= 11) return EMGReferralTier::Platinum;
	if (ReferralCount >= 6) return EMGReferralTier::Gold;
	if (ReferralCount >= 3) return EMGReferralTier::Silver;
	return EMGReferralTier::Bronze;
}

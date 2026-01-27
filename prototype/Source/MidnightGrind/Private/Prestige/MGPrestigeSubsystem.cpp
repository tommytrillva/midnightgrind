// Copyright Epic Games, Inc. All Rights Reserved.

#include "Prestige/MGPrestigeSubsystem.h"

void UMGPrestigeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Register default prestige ranks
	{
		FMGPrestigeRankDefinition Prestige1;
		Prestige1.Rank = EMGPrestigeRank::Prestige1;
		Prestige1.DisplayName = FText::FromString(TEXT("Prestige I"));
		Prestige1.Description = FText::FromString(TEXT("First step into prestige"));
		Prestige1.RequiredLevel = 100;
		Prestige1.RequiredTimesPrestiged = 0;
		Prestige1.ExperienceMultiplier = 1.1f;
		Prestige1.CurrencyMultiplier = 1.05f;
		Prestige1.ReputationMultiplier = 1.05f;
		Prestige1.BonusStartingLevel = 1;
		Prestige1.PrestigeTokenReward = 1;
		Prestige1.RankColor = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
		RegisterPrestigeRank(Prestige1);
	}
	{
		FMGPrestigeRankDefinition Prestige2;
		Prestige2.Rank = EMGPrestigeRank::Prestige2;
		Prestige2.DisplayName = FText::FromString(TEXT("Prestige II"));
		Prestige2.Description = FText::FromString(TEXT("Proven dedication"));
		Prestige2.RequiredLevel = 100;
		Prestige2.RequiredTimesPrestiged = 1;
		Prestige2.ExperienceMultiplier = 1.2f;
		Prestige2.CurrencyMultiplier = 1.1f;
		Prestige2.ReputationMultiplier = 1.1f;
		Prestige2.BonusStartingLevel = 5;
		Prestige2.PrestigeTokenReward = 2;
		Prestige2.RankColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);
		RegisterPrestigeRank(Prestige2);
	}
	{
		FMGPrestigeRankDefinition Prestige3;
		Prestige3.Rank = EMGPrestigeRank::Prestige3;
		Prestige3.DisplayName = FText::FromString(TEXT("Prestige III"));
		Prestige3.Description = FText::FromString(TEXT("Rising through the ranks"));
		Prestige3.RequiredLevel = 100;
		Prestige3.RequiredTimesPrestiged = 2;
		Prestige3.ExperienceMultiplier = 1.3f;
		Prestige3.CurrencyMultiplier = 1.15f;
		Prestige3.ReputationMultiplier = 1.15f;
		Prestige3.BonusStartingLevel = 10;
		Prestige3.PrestigeTokenReward = 2;
		Prestige3.RankColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
		RegisterPrestigeRank(Prestige3);
	}
	{
		FMGPrestigeRankDefinition Prestige5;
		Prestige5.Rank = EMGPrestigeRank::Prestige5;
		Prestige5.DisplayName = FText::FromString(TEXT("Prestige V"));
		Prestige5.Description = FText::FromString(TEXT("Halfway to mastery"));
		Prestige5.RequiredLevel = 100;
		Prestige5.RequiredTimesPrestiged = 4;
		Prestige5.ExperienceMultiplier = 1.5f;
		Prestige5.CurrencyMultiplier = 1.25f;
		Prestige5.ReputationMultiplier = 1.25f;
		Prestige5.BonusStartingLevel = 15;
		Prestige5.PrestigeTokenReward = 3;
		Prestige5.RankColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
		RegisterPrestigeRank(Prestige5);
	}
	{
		FMGPrestigeRankDefinition Prestige10;
		Prestige10.Rank = EMGPrestigeRank::Prestige10;
		Prestige10.DisplayName = FText::FromString(TEXT("Prestige X"));
		Prestige10.Description = FText::FromString(TEXT("Peak dedication"));
		Prestige10.RequiredLevel = 100;
		Prestige10.RequiredTimesPrestiged = 9;
		Prestige10.ExperienceMultiplier = 2.0f;
		Prestige10.CurrencyMultiplier = 1.5f;
		Prestige10.ReputationMultiplier = 1.5f;
		Prestige10.BonusStartingLevel = 25;
		Prestige10.PrestigeTokenReward = 5;
		Prestige10.RankColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
		RegisterPrestigeRank(Prestige10);
	}
	{
		FMGPrestigeRankDefinition PrestigeMaster;
		PrestigeMaster.Rank = EMGPrestigeRank::PrestigeMaster;
		PrestigeMaster.DisplayName = FText::FromString(TEXT("Prestige Master"));
		PrestigeMaster.Description = FText::FromString(TEXT("True mastery achieved"));
		PrestigeMaster.RequiredLevel = 100;
		PrestigeMaster.RequiredTimesPrestiged = 10;
		PrestigeMaster.ExperienceMultiplier = 2.5f;
		PrestigeMaster.CurrencyMultiplier = 1.75f;
		PrestigeMaster.ReputationMultiplier = 1.75f;
		PrestigeMaster.BonusStartingLevel = 30;
		PrestigeMaster.PrestigeTokenReward = 10;
		PrestigeMaster.RankColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
		RegisterPrestigeRank(PrestigeMaster);
	}
	{
		FMGPrestigeRankDefinition GrandMaster;
		GrandMaster.Rank = EMGPrestigeRank::PrestigeGrandMaster;
		GrandMaster.DisplayName = FText::FromString(TEXT("Grand Master"));
		GrandMaster.Description = FText::FromString(TEXT("Elite status"));
		GrandMaster.RequiredLevel = 100;
		GrandMaster.RequiredTimesPrestiged = 15;
		GrandMaster.ExperienceMultiplier = 3.0f;
		GrandMaster.CurrencyMultiplier = 2.0f;
		GrandMaster.ReputationMultiplier = 2.0f;
		GrandMaster.BonusStartingLevel = 40;
		GrandMaster.PrestigeTokenReward = 15;
		GrandMaster.RankColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
		RegisterPrestigeRank(GrandMaster);
	}
	{
		FMGPrestigeRankDefinition Legend;
		Legend.Rank = EMGPrestigeRank::PrestigeLegend;
		Legend.DisplayName = FText::FromString(TEXT("Legend"));
		Legend.Description = FText::FromString(TEXT("Living legend status"));
		Legend.RequiredLevel = 100;
		Legend.RequiredTimesPrestiged = 20;
		Legend.ExperienceMultiplier = 4.0f;
		Legend.CurrencyMultiplier = 2.5f;
		Legend.ReputationMultiplier = 2.5f;
		Legend.BonusStartingLevel = 50;
		Legend.PrestigeTokenReward = 25;
		Legend.RankColor = FLinearColor(1.0f, 0.9f, 0.1f, 1.0f);
		RegisterPrestigeRank(Legend);
	}

	// Register milestones
	{
		FMGPrestigeMilestone FirstPrestige;
		FirstPrestige.MilestoneId = TEXT("FIRST_PRESTIGE");
		FirstPrestige.DisplayName = FText::FromString(TEXT("First Steps"));
		FirstPrestige.Description = FText::FromString(TEXT("Prestige for the first time"));
		FirstPrestige.RequiredTimesPrestiged = 1;
		Milestones.Add(FirstPrestige.MilestoneId, FirstPrestige);
	}
	{
		FMGPrestigeMilestone MasterMilestone;
		MasterMilestone.MilestoneId = TEXT("PRESTIGE_MASTER");
		MasterMilestone.DisplayName = FText::FromString(TEXT("Prestige Master"));
		MasterMilestone.Description = FText::FromString(TEXT("Reach Prestige Master rank"));
		MasterMilestone.RequiredRank = EMGPrestigeRank::PrestigeMaster;
		Milestones.Add(MasterMilestone.MilestoneId, MasterMilestone);
	}
	{
		FMGPrestigeMilestone LegendMilestone;
		LegendMilestone.MilestoneId = TEXT("LEGEND_STATUS");
		LegendMilestone.DisplayName = FText::FromString(TEXT("Living Legend"));
		LegendMilestone.Description = FText::FromString(TEXT("Achieve Legend status"));
		LegendMilestone.RequiredRank = EMGPrestigeRank::PrestigeLegend;
		Milestones.Add(LegendMilestone.MilestoneId, LegendMilestone);
	}
	{
		FMGPrestigeMilestone ExpMilestone;
		ExpMilestone.MilestoneId = TEXT("MILLION_XP");
		ExpMilestone.DisplayName = FText::FromString(TEXT("Experience Hunter"));
		ExpMilestone.Description = FText::FromString(TEXT("Earn 1,000,000 total experience"));
		ExpMilestone.RequiredTotalExperience = 1000000;
		Milestones.Add(ExpMilestone.MilestoneId, ExpMilestone);
	}

	// Load saved data
	LoadPrestigeData();
}

void UMGPrestigeSubsystem::Deinitialize()
{
	SavePrestigeData();

	RankDefinitions.Empty();
	Rewards.Empty();
	Milestones.Empty();
	TokenShopItems.Empty();
	PlayerPrestigeData.Empty();
	CategoryPrestigeData.Empty();
	PlayerStats.Empty();
	AchievedMilestones.Empty();
	ShopPurchaseCounts.Empty();
	Leaderboard.Empty();

	Super::Deinitialize();
}

// Registration
void UMGPrestigeSubsystem::RegisterPrestigeRank(const FMGPrestigeRankDefinition& Definition)
{
	RankDefinitions.Add(Definition.Rank, Definition);
}

void UMGPrestigeSubsystem::RegisterPrestigeReward(const FMGPrestigeReward& Reward)
{
	if (!Reward.RewardId.IsEmpty())
	{
		Rewards.Add(Reward.RewardId, Reward);
	}
}

void UMGPrestigeSubsystem::RegisterPrestigeMilestone(const FMGPrestigeMilestone& Milestone)
{
	if (!Milestone.MilestoneId.IsEmpty())
	{
		Milestones.Add(Milestone.MilestoneId, Milestone);
	}
}

void UMGPrestigeSubsystem::RegisterTokenShopItem(const FMGPrestigeTokenShopItem& Item)
{
	if (!Item.ItemId.IsEmpty())
	{
		TokenShopItems.Add(Item.ItemId, Item);
	}
}

// Experience
void UMGPrestigeSubsystem::AddExperience(const FString& PlayerId, int64 Amount, EMGPrestigeCategory Category)
{
	FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		FMGPlayerPrestige NewPrestige;
		NewPrestige.PlayerId = PlayerId;
		NewPrestige.ExperienceToNextLevel = CalculateExperienceForLevel(2);
		Prestige = &PlayerPrestigeData.Add(PlayerId, NewPrestige);
	}

	// Apply prestige multiplier
	int64 ModifiedAmount = FMath::RoundToInt64(Amount * Prestige->PrestigeMultiplier);

	Prestige->CurrentExperience += ModifiedAmount;
	Prestige->TotalExperienceEarned += ModifiedAmount;

	OnPrestigeExperienceGained.Broadcast(PlayerId, ModifiedAmount, Prestige->CurrentExperience, Category);

	// Check for level up
	CheckLevelUp(PlayerId);

	// Also add to category
	if (Category != EMGPrestigeCategory::Overall)
	{
		AddCategoryExperience(PlayerId, Category, ModifiedAmount);
	}

	// Check milestones
	CheckMilestones(PlayerId);

	// Update stats
	UpdatePlayerStats(PlayerId);

	// Update leaderboard
	UpdateLeaderboard();
}

void UMGPrestigeSubsystem::AddCategoryExperience(const FString& PlayerId, EMGPrestigeCategory Category, int64 Amount)
{
	TMap<EMGPrestigeCategory, FMGCategoryPrestige>* PlayerCategories = CategoryPrestigeData.Find(PlayerId);
	if (!PlayerCategories)
	{
		TMap<EMGPrestigeCategory, FMGCategoryPrestige> NewMap;
		PlayerCategories = &CategoryPrestigeData.Add(PlayerId, NewMap);
	}

	FMGCategoryPrestige* CatPrestige = PlayerCategories->Find(Category);
	if (!CatPrestige)
	{
		FMGCategoryPrestige NewCat;
		NewCat.PlayerId = PlayerId;
		NewCat.Category = Category;
		CatPrestige = &PlayerCategories->Add(Category, NewCat);
	}

	CatPrestige->CurrentExperience += Amount;

	// Check for category level up
	while (CatPrestige->CurrentExperience >= CatPrestige->ExperienceToNextLevel)
	{
		CatPrestige->CurrentExperience -= CatPrestige->ExperienceToNextLevel;
		CatPrestige->CurrentLevel++;
		CatPrestige->ExperienceToNextLevel = CalculateExperienceForLevel(CatPrestige->CurrentLevel + 1) / 2; // Categories level faster

		// Check for category prestige
		if (CatPrestige->CurrentLevel >= 50)
		{
			EMGPrestigeRank OldRank = CatPrestige->CurrentRank;
			CatPrestige->TimesPrestiged++;
			CatPrestige->CurrentLevel = 1;

			// Increment rank if possible
			if (CatPrestige->CurrentRank < EMGPrestigeRank::PrestigeLegend)
			{
				CatPrestige->CurrentRank = static_cast<EMGPrestigeRank>(static_cast<uint8>(CatPrestige->CurrentRank) + 1);
			}

			OnCategoryPrestigeUp.Broadcast(PlayerId, Category, OldRank, CatPrestige->CurrentRank);
		}
	}
}

int64 UMGPrestigeSubsystem::GetExperienceToNextLevel(const FString& PlayerId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		return Prestige->ExperienceToNextLevel - Prestige->CurrentExperience;
	}
	return CalculateExperienceForLevel(2);
}

float UMGPrestigeSubsystem::GetLevelProgress(const FString& PlayerId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		if (Prestige->ExperienceToNextLevel > 0)
		{
			return static_cast<float>(Prestige->CurrentExperience) / static_cast<float>(Prestige->ExperienceToNextLevel);
		}
	}
	return 0.0f;
}

// Prestige Actions
FMGPrestigeResetResult UMGPrestigeSubsystem::PerformPrestige(const FString& PlayerId, EMGPrestigeResetType ResetType)
{
	FMGPrestigeResetResult Result;
	Result.PlayerId = PlayerId;
	Result.ResetTime = FDateTime::Now();

	if (!CanPrestige(PlayerId))
	{
		return Result;
	}

	FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return Result;
	}

	Result.OldRank = Prestige->CurrentRank;
	Result.OldLevel = Prestige->CurrentLevel;

	// Determine new rank
	EMGPrestigeRank NewRank = GetNextPrestigeRank(PlayerId);
	const FMGPrestigeRankDefinition* RankDef = RankDefinitions.Find(NewRank);

	if (!RankDef)
	{
		return Result;
	}

	// Award prestige tokens
	int32 TokensEarned = RankDef->PrestigeTokenReward;
	AddPrestigeTokens(PlayerId, TokensEarned);
	Result.PrestigeTokensEarned = TokensEarned;

	// Perform reset
	ApplyPrestigeReset(PlayerId, ResetType);

	// Update prestige data
	Prestige->CurrentRank = NewRank;
	Prestige->TimesPrestiged++;
	Prestige->LastPrestigeDate = FDateTime::Now();

	if (Prestige->TimesPrestiged == 1)
	{
		Prestige->FirstPrestigeDate = FDateTime::Now();
	}

	// Calculate new multiplier
	Prestige->PrestigeMultiplier = CalculatePrestigeMultiplier(NewRank, Prestige->TimesPrestiged);
	Result.NewMultiplier = Prestige->PrestigeMultiplier;

	// Set starting level based on rank
	Prestige->CurrentLevel = RankDef->BonusStartingLevel;
	Prestige->CurrentExperience = 0;
	Prestige->ExperienceToNextLevel = CalculateExperienceForLevel(Prestige->CurrentLevel + 1);
	Prestige->bEligibleForPrestige = false;

	Result.NewRank = NewRank;
	Result.NewLevel = Prestige->CurrentLevel;

	// Unlock rank rewards
	for (const auto& RewardPair : Rewards)
	{
		if (RewardPair.Value.RequiredRank == NewRank)
		{
			if (UnlockReward(PlayerId, RewardPair.Key))
			{
				Result.NewUnlocks.Add(RewardPair.Key);
			}
		}
	}

	OnPrestigeReset.Broadcast(PlayerId, Result);
	OnPrestigeRankUp.Broadcast(PlayerId, Result.OldRank, Result.NewRank);

	// Update stats and leaderboard
	UpdatePlayerStats(PlayerId);
	UpdateLeaderboard();

	// Check milestones
	CheckMilestones(PlayerId);

	return Result;
}

bool UMGPrestigeSubsystem::CanPrestige(const FString& PlayerId) const
{
	const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return false;
	}

	// Must be at max level
	if (Prestige->CurrentLevel < Prestige->MaxLevel)
	{
		return false;
	}

	// Check if there's a next rank available
	EMGPrestigeRank NextRank = GetNextPrestigeRank(PlayerId);
	if (NextRank == EMGPrestigeRank::None)
	{
		return false;
	}

	// Check requirements for next rank
	const FMGPrestigeRankDefinition* RankDef = RankDefinitions.Find(NextRank);
	if (!RankDef)
	{
		return false;
	}

	if (Prestige->TimesPrestiged < RankDef->RequiredTimesPrestiged)
	{
		return false;
	}

	return true;
}

EMGPrestigeRank UMGPrestigeSubsystem::GetNextPrestigeRank(const FString& PlayerId) const
{
	const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	EMGPrestigeRank CurrentRank = Prestige ? Prestige->CurrentRank : EMGPrestigeRank::None;

	if (CurrentRank == EMGPrestigeRank::PrestigeLegend)
	{
		return EMGPrestigeRank::None; // Already at max
	}

	return static_cast<EMGPrestigeRank>(static_cast<uint8>(CurrentRank) + 1);
}

TArray<FString> UMGPrestigeSubsystem::GetPrestigePreview(const FString& PlayerId) const
{
	TArray<FString> Preview;

	EMGPrestigeRank NextRank = GetNextPrestigeRank(PlayerId);
	const FMGPrestigeRankDefinition* RankDef = RankDefinitions.Find(NextRank);

	if (RankDef)
	{
		Preview.Add(FString::Printf(TEXT("New Rank: %s"), *RankDef->DisplayName.ToString()));
		Preview.Add(FString::Printf(TEXT("Experience Multiplier: %.1fx"), RankDef->ExperienceMultiplier));
		Preview.Add(FString::Printf(TEXT("Currency Multiplier: %.1fx"), RankDef->CurrencyMultiplier));
		Preview.Add(FString::Printf(TEXT("Prestige Tokens: +%d"), RankDef->PrestigeTokenReward));
		Preview.Add(FString::Printf(TEXT("Bonus Starting Level: %d"), RankDef->BonusStartingLevel));
	}

	// List rewards that will be unlocked
	for (const auto& RewardPair : Rewards)
	{
		if (RewardPair.Value.RequiredRank == NextRank)
		{
			Preview.Add(FString::Printf(TEXT("Unlock: %s"), *RewardPair.Value.DisplayName.ToString()));
		}
	}

	return Preview;
}

// Player Data
FMGPlayerPrestige UMGPrestigeSubsystem::GetPlayerPrestige(const FString& PlayerId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		return *Prestige;
	}
	return FMGPlayerPrestige();
}

FMGCategoryPrestige UMGPrestigeSubsystem::GetCategoryPrestige(const FString& PlayerId, EMGPrestigeCategory Category) const
{
	if (const TMap<EMGPrestigeCategory, FMGCategoryPrestige>* Categories = CategoryPrestigeData.Find(PlayerId))
	{
		if (const FMGCategoryPrestige* Cat = Categories->Find(Category))
		{
			return *Cat;
		}
	}
	return FMGCategoryPrestige();
}

EMGPrestigeRank UMGPrestigeSubsystem::GetPlayerRank(const FString& PlayerId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		return Prestige->CurrentRank;
	}
	return EMGPrestigeRank::None;
}

int32 UMGPrestigeSubsystem::GetPlayerLevel(const FString& PlayerId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		return Prestige->CurrentLevel;
	}
	return 1;
}

int32 UMGPrestigeSubsystem::GetTimesPrestiged(const FString& PlayerId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		return Prestige->TimesPrestiged;
	}
	return 0;
}

float UMGPrestigeSubsystem::GetPrestigeMultiplier(const FString& PlayerId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		return Prestige->PrestigeMultiplier;
	}
	return 1.0f;
}

// Tokens
void UMGPrestigeSubsystem::AddPrestigeTokens(const FString& PlayerId, int32 Amount)
{
	FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		FMGPlayerPrestige NewPrestige;
		NewPrestige.PlayerId = PlayerId;
		Prestige = &PlayerPrestigeData.Add(PlayerId, NewPrestige);
	}

	int32 OldTokens = Prestige->PrestigeTokens;
	Prestige->PrestigeTokens += Amount;

	// Update stats
	FMGPrestigePlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats)
	{
		Stats->TotalTokensEarned += Amount;
	}

	OnPrestigeTokensChanged.Broadcast(PlayerId, OldTokens, Prestige->PrestigeTokens);
}

bool UMGPrestigeSubsystem::SpendPrestigeTokens(const FString& PlayerId, int32 Amount)
{
	FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige || Prestige->PrestigeTokens < Amount)
	{
		return false;
	}

	int32 OldTokens = Prestige->PrestigeTokens;
	Prestige->PrestigeTokens -= Amount;

	// Update stats
	FMGPrestigePlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats)
	{
		Stats->TotalTokensSpent += Amount;
	}

	OnPrestigeTokensChanged.Broadcast(PlayerId, OldTokens, Prestige->PrestigeTokens);
	return true;
}

int32 UMGPrestigeSubsystem::GetPrestigeTokens(const FString& PlayerId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		return Prestige->PrestigeTokens;
	}
	return 0;
}

// Token Shop
TArray<FMGPrestigeTokenShopItem> UMGPrestigeSubsystem::GetAvailableShopItems(const FString& PlayerId) const
{
	TArray<FMGPrestigeTokenShopItem> Available;
	EMGPrestigeRank PlayerRank = GetPlayerRank(PlayerId);

	for (const auto& Pair : TokenShopItems)
	{
		if (PlayerRank >= Pair.Value.RequiredRank)
		{
			// Check stock
			if (Pair.Value.bIsLimited && Pair.Value.Stock == 0)
			{
				continue;
			}

			// Check purchase limit
			if (Pair.Value.PurchaseLimit > 0)
			{
				if (const TMap<FString, int32>* Counts = ShopPurchaseCounts.Find(PlayerId))
				{
					if (const int32* Count = Counts->Find(Pair.Key))
					{
						if (*Count >= Pair.Value.PurchaseLimit)
						{
							continue;
						}
					}
				}
			}

			Available.Add(Pair.Value);
		}
	}

	return Available;
}

bool UMGPrestigeSubsystem::PurchaseShopItem(const FString& PlayerId, const FString& ItemId)
{
	if (!CanPurchaseShopItem(PlayerId, ItemId))
	{
		return false;
	}

	FMGPrestigeTokenShopItem* Item = TokenShopItems.Find(ItemId);
	if (!Item)
	{
		return false;
	}

	// Spend tokens
	if (!SpendPrestigeTokens(PlayerId, Item->TokenCost))
	{
		return false;
	}

	// Reduce stock if limited
	if (Item->bIsLimited && Item->Stock > 0)
	{
		Item->Stock--;
	}

	// Track purchase count
	TMap<FString, int32>* Counts = ShopPurchaseCounts.Find(PlayerId);
	if (!Counts)
	{
		TMap<FString, int32> NewCounts;
		Counts = &ShopPurchaseCounts.Add(PlayerId, NewCounts);
	}
	int32& PurchaseCount = Counts->FindOrAdd(ItemId);
	PurchaseCount++;

	// Grant unlockable
	if (!Item->UnlockableId.IsEmpty())
	{
		FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
		if (Prestige)
		{
			Prestige->UnlockedRewards.AddUnique(Item->UnlockableId);
			OnPrestigeRewardUnlocked.Broadcast(PlayerId, Item->UnlockableId, true);
		}
	}

	return true;
}

bool UMGPrestigeSubsystem::CanPurchaseShopItem(const FString& PlayerId, const FString& ItemId) const
{
	const FMGPrestigeTokenShopItem* Item = TokenShopItems.Find(ItemId);
	if (!Item)
	{
		return false;
	}

	// Check rank requirement
	if (GetPlayerRank(PlayerId) < Item->RequiredRank)
	{
		return false;
	}

	// Check tokens
	if (GetPrestigeTokens(PlayerId) < Item->TokenCost)
	{
		return false;
	}

	// Check stock
	if (Item->bIsLimited && Item->Stock == 0)
	{
		return false;
	}

	// Check purchase limit
	if (Item->PurchaseLimit > 0)
	{
		if (const TMap<FString, int32>* Counts = ShopPurchaseCounts.Find(PlayerId))
		{
			if (const int32* Count = Counts->Find(ItemId))
			{
				if (*Count >= Item->PurchaseLimit)
				{
					return false;
				}
			}
		}
	}

	return true;
}

// Rewards
TArray<FMGPrestigeReward> UMGPrestigeSubsystem::GetAvailableRewards(const FString& PlayerId) const
{
	TArray<FMGPrestigeReward> Available;
	const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return Available;
	}

	for (const auto& Pair : Rewards)
	{
		if (Prestige->CurrentRank >= Pair.Value.RequiredRank &&
			Prestige->CurrentLevel >= Pair.Value.RequiredLevel &&
			!Prestige->UnlockedRewards.Contains(Pair.Key))
		{
			Available.Add(Pair.Value);
		}
	}

	return Available;
}

TArray<FMGPrestigeReward> UMGPrestigeSubsystem::GetUnlockedRewards(const FString& PlayerId) const
{
	TArray<FMGPrestigeReward> Unlocked;
	const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return Unlocked;
	}

	for (const FString& RewardId : Prestige->UnlockedRewards)
	{
		if (const FMGPrestigeReward* Reward = Rewards.Find(RewardId))
		{
			Unlocked.Add(*Reward);
		}
	}

	return Unlocked;
}

bool UMGPrestigeSubsystem::UnlockReward(const FString& PlayerId, const FString& RewardId)
{
	FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return false;
	}

	const FMGPrestigeReward* Reward = Rewards.Find(RewardId);
	if (!Reward)
	{
		return false;
	}

	// Check requirements
	if (Prestige->CurrentRank < Reward->RequiredRank)
	{
		return false;
	}
	if (Prestige->CurrentLevel < Reward->RequiredLevel)
	{
		return false;
	}

	// Check token cost
	if (Reward->PrestigeTokenCost > 0)
	{
		if (!SpendPrestigeTokens(PlayerId, Reward->PrestigeTokenCost))
		{
			return false;
		}
	}

	// Unlock
	Prestige->UnlockedRewards.AddUnique(RewardId);

	if (Reward->bIsPermanent)
	{
		Prestige->PermanentUnlocks.AddUnique(RewardId);
	}

	// Update stats
	FMGPrestigePlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats)
	{
		Stats->RewardsUnlocked++;
	}

	OnPrestigeRewardUnlocked.Broadcast(PlayerId, RewardId, Reward->bIsPermanent);
	return true;
}

bool UMGPrestigeSubsystem::IsRewardUnlocked(const FString& PlayerId, const FString& RewardId) const
{
	if (const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId))
	{
		return Prestige->UnlockedRewards.Contains(RewardId);
	}
	return false;
}

// Milestones
TArray<FMGPrestigeMilestone> UMGPrestigeSubsystem::GetAllMilestones() const
{
	TArray<FMGPrestigeMilestone> Result;
	Milestones.GenerateValueArray(Result);
	return Result;
}

TArray<FMGPrestigeMilestone> UMGPrestigeSubsystem::GetAchievedMilestones(const FString& PlayerId) const
{
	TArray<FMGPrestigeMilestone> Result;

	const TSet<FString>* Achieved = AchievedMilestones.Find(PlayerId);
	if (!Achieved)
	{
		return Result;
	}

	for (const FString& MilestoneId : *Achieved)
	{
		if (const FMGPrestigeMilestone* Milestone = Milestones.Find(MilestoneId))
		{
			Result.Add(*Milestone);
		}
	}

	return Result;
}

float UMGPrestigeSubsystem::GetMilestoneProgress(const FString& PlayerId, const FString& MilestoneId) const
{
	const FMGPrestigeMilestone* Milestone = Milestones.Find(MilestoneId);
	if (!Milestone)
	{
		return 0.0f;
	}

	const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return 0.0f;
	}

	// Check different requirements
	if (Milestone->RequiredTimesPrestiged > 0)
	{
		return FMath::Clamp(static_cast<float>(Prestige->TimesPrestiged) / Milestone->RequiredTimesPrestiged, 0.0f, 1.0f);
	}

	if (Milestone->RequiredTotalExperience > 0)
	{
		return FMath::Clamp(static_cast<float>(Prestige->TotalExperienceEarned) / Milestone->RequiredTotalExperience, 0.0f, 1.0f);
	}

	if (Milestone->RequiredRank != EMGPrestigeRank::None)
	{
		if (Prestige->CurrentRank >= Milestone->RequiredRank)
		{
			return 1.0f;
		}
		return static_cast<float>(static_cast<uint8>(Prestige->CurrentRank)) / static_cast<float>(static_cast<uint8>(Milestone->RequiredRank));
	}

	return 0.0f;
}

// Definitions
FMGPrestigeRankDefinition UMGPrestigeSubsystem::GetRankDefinition(EMGPrestigeRank Rank) const
{
	if (const FMGPrestigeRankDefinition* Def = RankDefinitions.Find(Rank))
	{
		return *Def;
	}
	return FMGPrestigeRankDefinition();
}

TArray<FMGPrestigeRankDefinition> UMGPrestigeSubsystem::GetAllRankDefinitions() const
{
	TArray<FMGPrestigeRankDefinition> Result;
	RankDefinitions.GenerateValueArray(Result);
	return Result;
}

// Stats
FMGPrestigePlayerStats UMGPrestigeSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGPrestigePlayerStats* Stats = PlayerStats.Find(PlayerId))
	{
		return *Stats;
	}
	return FMGPrestigePlayerStats();
}

// Leaderboards
TArray<FMGPrestigeLeaderboardEntry> UMGPrestigeSubsystem::GetPrestigeLeaderboard(int32 Count) const
{
	TArray<FMGPrestigeLeaderboardEntry> Result;

	int32 NumEntries = FMath::Min(Count, Leaderboard.Num());
	for (int32 i = 0; i < NumEntries; i++)
	{
		Result.Add(Leaderboard[i]);
	}

	return Result;
}

int32 UMGPrestigeSubsystem::GetPlayerLeaderboardPosition(const FString& PlayerId) const
{
	for (int32 i = 0; i < Leaderboard.Num(); i++)
	{
		if (Leaderboard[i].PlayerId == PlayerId)
		{
			return i + 1;
		}
	}
	return 0;
}

// Protected
void UMGPrestigeSubsystem::CheckLevelUp(const FString& PlayerId)
{
	FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return;
	}

	while (Prestige->CurrentExperience >= Prestige->ExperienceToNextLevel &&
		   Prestige->CurrentLevel < Prestige->MaxLevel)
	{
		int32 OldLevel = Prestige->CurrentLevel;

		Prestige->CurrentExperience -= Prestige->ExperienceToNextLevel;
		Prestige->CurrentLevel++;
		Prestige->ExperienceToNextLevel = CalculateExperienceForLevel(Prestige->CurrentLevel + 1);

		OnPrestigeLevelUp.Broadcast(PlayerId, OldLevel, Prestige->CurrentLevel);
	}

	// Check for prestige eligibility
	CheckPrestigeEligibility(PlayerId);
}

void UMGPrestigeSubsystem::CheckMilestones(const FString& PlayerId)
{
	const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return;
	}

	TSet<FString>* Achieved = AchievedMilestones.Find(PlayerId);
	if (!Achieved)
	{
		TSet<FString> NewSet;
		Achieved = &AchievedMilestones.Add(PlayerId, NewSet);
	}

	for (auto& Pair : Milestones)
	{
		if (Achieved->Contains(Pair.Key))
		{
			continue;
		}

		bool bAchieved = false;

		// Check rank requirement
		if (Pair.Value.RequiredRank != EMGPrestigeRank::None)
		{
			bAchieved = Prestige->CurrentRank >= Pair.Value.RequiredRank;
		}
		// Check prestige count
		else if (Pair.Value.RequiredTimesPrestiged > 0)
		{
			bAchieved = Prestige->TimesPrestiged >= Pair.Value.RequiredTimesPrestiged;
		}
		// Check experience
		else if (Pair.Value.RequiredTotalExperience > 0)
		{
			bAchieved = Prestige->TotalExperienceEarned >= Pair.Value.RequiredTotalExperience;
		}

		if (bAchieved)
		{
			Achieved->Add(Pair.Key);
			Pair.Value.bAchieved = true;
			Pair.Value.AchievedDate = FDateTime::Now();

			// Update stats
			FMGPrestigePlayerStats* Stats = PlayerStats.Find(PlayerId);
			if (Stats)
			{
				Stats->MilestonesCompleted++;
			}

			OnPrestigeMilestoneAchieved.Broadcast(PlayerId, Pair.Key);
		}
	}
}

void UMGPrestigeSubsystem::CheckPrestigeEligibility(const FString& PlayerId)
{
	FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return;
	}

	bool bWasEligible = Prestige->bEligibleForPrestige;
	Prestige->bEligibleForPrestige = CanPrestige(PlayerId);

	if (!bWasEligible && Prestige->bEligibleForPrestige)
	{
		OnPrestigeEligible.Broadcast(PlayerId, GetNextPrestigeRank(PlayerId));
	}
}

int64 UMGPrestigeSubsystem::CalculateExperienceForLevel(int32 Level) const
{
	// Exponential scaling
	return FMath::RoundToInt64(BaseExperiencePerLevel * FMath::Pow(ExperienceScalingFactor, Level - 1));
}

float UMGPrestigeSubsystem::CalculatePrestigeMultiplier(EMGPrestigeRank Rank, int32 TimesPrestiged) const
{
	float BaseMultiplier = 1.0f;

	const FMGPrestigeRankDefinition* RankDef = RankDefinitions.Find(Rank);
	if (RankDef)
	{
		BaseMultiplier = RankDef->ExperienceMultiplier;
	}

	// Additional bonus for multiple prestiges
	float PrestigeBonus = TimesPrestiged * 0.05f;

	return BaseMultiplier + PrestigeBonus;
}

void UMGPrestigeSubsystem::ApplyPrestigeReset(const FString& PlayerId, EMGPrestigeResetType ResetType)
{
	FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return;
	}

	switch (ResetType)
	{
		case EMGPrestigeResetType::Soft:
			// Only reset level, keep most unlocks
			break;

		case EMGPrestigeResetType::Hard:
			// Reset level and non-permanent unlocks
			Prestige->UnlockedRewards = Prestige->PermanentUnlocks;
			break;

		case EMGPrestigeResetType::Seasonal:
			// Similar to soft but may include seasonal bonuses
			break;

		case EMGPrestigeResetType::Full:
			// Complete reset (keep permanent unlocks only)
			Prestige->UnlockedRewards = Prestige->PermanentUnlocks;
			break;
	}
}

void UMGPrestigeSubsystem::UpdatePlayerStats(const FString& PlayerId)
{
	const FMGPlayerPrestige* Prestige = PlayerPrestigeData.Find(PlayerId);
	if (!Prestige)
	{
		return;
	}

	FMGPrestigePlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGPrestigePlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}

	Stats->TotalTimesPrestiged = Prestige->TimesPrestiged;
	Stats->TotalExperienceAllTime = Prestige->TotalExperienceEarned;

	if (Prestige->CurrentLevel > Stats->HighestLevelReached)
	{
		Stats->HighestLevelReached = Prestige->CurrentLevel;
	}

	if (Prestige->CurrentRank > Stats->HighestRankAchieved)
	{
		Stats->HighestRankAchieved = Prestige->CurrentRank;
	}
}

void UMGPrestigeSubsystem::UpdateLeaderboard()
{
	Leaderboard.Empty();

	for (const auto& Pair : PlayerPrestigeData)
	{
		FMGPrestigeLeaderboardEntry Entry;
		Entry.PlayerId = Pair.Key;
		Entry.Rank = Pair.Value.CurrentRank;
		Entry.Level = Pair.Value.CurrentLevel;
		Entry.TimesPrestiged = Pair.Value.TimesPrestiged;
		Entry.TotalExperience = Pair.Value.TotalExperienceEarned;
		Leaderboard.Add(Entry);
	}

	// Sort by rank, then prestige count, then experience
	Leaderboard.Sort([](const FMGPrestigeLeaderboardEntry& A, const FMGPrestigeLeaderboardEntry& B)
	{
		if (A.Rank != B.Rank)
		{
			return A.Rank > B.Rank;
		}
		if (A.TimesPrestiged != B.TimesPrestiged)
		{
			return A.TimesPrestiged > B.TimesPrestiged;
		}
		return A.TotalExperience > B.TotalExperience;
	});

	// Update positions
	for (int32 i = 0; i < Leaderboard.Num(); i++)
	{
		Leaderboard[i].LeaderboardPosition = i + 1;
	}
}

// Persistence
void UMGPrestigeSubsystem::SavePrestigeData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Prestige");
	IFileManager::Get().MakeDirectory(*SaveDir, true);
	FString FilePath = SaveDir / TEXT("prestige_data.dat");

	FBufferArchive SaveArchive;

	// Version for future compatibility
	int32 Version = 1;
	SaveArchive << Version;

	// Save player prestige data
	int32 NumPlayers = PlayerPrestigeData.Num();
	SaveArchive << NumPlayers;

	for (const auto& PrestigePair : PlayerPrestigeData)
	{
		FString PlayerId = PrestigePair.Key;
		SaveArchive << PlayerId;

		const FMGPlayerPrestige& Prestige = PrestigePair.Value;
		int32 RankInt = static_cast<int32>(Prestige.CurrentRank);
		int32 Level = Prestige.CurrentLevel;
		int64 Experience = Prestige.CurrentExperience;
		int64 TotalExp = Prestige.TotalExperienceEarned;
		int32 Times = Prestige.TimesPrestiged;
		float Multiplier = Prestige.PrestigeMultiplier;
		int64 LastPrestigeTicks = Prestige.LastPrestigeDate.GetTicks();
		int64 FirstPrestigeTicks = Prestige.FirstPrestigeDate.GetTicks();

		SaveArchive << RankInt;
		SaveArchive << Level;
		SaveArchive << Experience;
		SaveArchive << TotalExp;
		SaveArchive << Times;
		SaveArchive << Multiplier;
		SaveArchive << LastPrestigeTicks;
		SaveArchive << FirstPrestigeTicks;

		// Save unlocked rewards
		int32 NumUnlocks = Prestige.UnlockedRewards.Num();
		SaveArchive << NumUnlocks;
		for (const FString& RewardId : Prestige.UnlockedRewards)
		{
			FString Id = RewardId;
			SaveArchive << Id;
		}
	}

	// Save player stats
	int32 NumStats = PlayerStats.Num();
	SaveArchive << NumStats;

	for (const auto& StatPair : PlayerStats)
	{
		FString PlayerId = StatPair.Key;
		SaveArchive << PlayerId;

		const FMGPrestigePlayerStats& Stats = StatPair.Value;
		int32 TotalTimes = Stats.TotalTimesPrestiged;
		int64 TotalExpAll = Stats.TotalExperienceAllTime;
		int32 HighestLevel = Stats.HighestLevelReached;
		int32 HighestRankInt = static_cast<int32>(Stats.HighestRankAchieved);
		int32 TokensEarned = Stats.TotalTokensEarned;
		int32 TokensSpent = Stats.TotalTokensSpent;
		int32 Milestones = Stats.MilestonesCompleted;
		int32 Rewards = Stats.RewardsUnlocked;
		float FastestPrestige = Stats.FastestPrestige;

		SaveArchive << TotalTimes;
		SaveArchive << TotalExpAll;
		SaveArchive << HighestLevel;
		SaveArchive << HighestRankInt;
		SaveArchive << TokensEarned;
		SaveArchive << TokensSpent;
		SaveArchive << Milestones;
		SaveArchive << Rewards;
		SaveArchive << FastestPrestige;

		// Save category prestige counts
		int32 NumCats = Stats.CategoryPrestigeCounts.Num();
		SaveArchive << NumCats;
		for (const auto& CatPair : Stats.CategoryPrestigeCounts)
		{
			int32 CatInt = static_cast<int32>(CatPair.Key);
			int32 Count = CatPair.Value;
			SaveArchive << CatInt;
			SaveArchive << Count;
		}
	}

	// Save achieved milestones per player
	int32 NumAchieved = AchievedMilestones.Num();
	SaveArchive << NumAchieved;

	for (const auto& AchievedPair : AchievedMilestones)
	{
		FString PlayerId = AchievedPair.Key;
		SaveArchive << PlayerId;

		int32 NumMilestones = AchievedPair.Value.Num();
		SaveArchive << NumMilestones;

		for (const FString& MilestoneId : AchievedPair.Value)
		{
			FString Id = MilestoneId;
			SaveArchive << Id;
		}
	}

	// Write to file
	if (SaveArchive.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
	}

	UE_LOG(LogTemp, Log, TEXT("MGPrestigeSubsystem: Saved prestige data for %d players"), NumPlayers);
}

void UMGPrestigeSubsystem::LoadPrestigeData()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Prestige") / TEXT("prestige_data.dat");

	TArray<uint8> LoadData;
	if (!FFileHelper::LoadFileToArray(LoadData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("MGPrestigeSubsystem: No saved prestige data found"));
		return;
	}

	FMemoryReader LoadArchive(LoadData, true);

	int32 Version;
	LoadArchive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGPrestigeSubsystem: Unknown save version %d"), Version);
		return;
	}

	// Load player prestige data
	int32 NumPlayers;
	LoadArchive << NumPlayers;

	for (int32 i = 0; i < NumPlayers; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		FMGPlayerPrestige Prestige;
		Prestige.PlayerId = PlayerId;

		int32 RankInt;
		int64 LastPrestigeTicks;
		int64 FirstPrestigeTicks;

		LoadArchive << RankInt;
		LoadArchive << Prestige.CurrentLevel;
		LoadArchive << Prestige.CurrentExperience;
		LoadArchive << Prestige.TotalExperienceEarned;
		LoadArchive << Prestige.TimesPrestiged;
		LoadArchive << Prestige.PrestigeMultiplier;
		LoadArchive << LastPrestigeTicks;
		LoadArchive << FirstPrestigeTicks;

		Prestige.CurrentRank = static_cast<EMGPrestigeRank>(RankInt);
		Prestige.LastPrestigeDate = FDateTime(LastPrestigeTicks);
		Prestige.FirstPrestigeDate = FDateTime(FirstPrestigeTicks);

		// Load unlocked rewards
		int32 NumUnlocks;
		LoadArchive << NumUnlocks;
		for (int32 j = 0; j < NumUnlocks; ++j)
		{
			FString RewardId;
			LoadArchive << RewardId;
			Prestige.UnlockedRewards.Add(RewardId);
		}

		PlayerPrestigeData.Add(PlayerId, Prestige);
	}

	// Load player stats
	int32 NumStats;
	LoadArchive << NumStats;

	for (int32 i = 0; i < NumStats; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		FMGPrestigePlayerStats Stats;
		Stats.PlayerId = PlayerId;

		int32 HighestRankInt;

		LoadArchive << Stats.TotalTimesPrestiged;
		LoadArchive << Stats.TotalExperienceAllTime;
		LoadArchive << Stats.HighestLevelReached;
		LoadArchive << HighestRankInt;
		LoadArchive << Stats.TotalTokensEarned;
		LoadArchive << Stats.TotalTokensSpent;
		LoadArchive << Stats.MilestonesCompleted;
		LoadArchive << Stats.RewardsUnlocked;
		LoadArchive << Stats.FastestPrestige;

		Stats.HighestRankAchieved = static_cast<EMGPrestigeRank>(HighestRankInt);

		// Load category prestige counts
		int32 NumCats;
		LoadArchive << NumCats;
		for (int32 j = 0; j < NumCats; ++j)
		{
			int32 CatInt;
			int32 Count;
			LoadArchive << CatInt;
			LoadArchive << Count;
			Stats.CategoryPrestigeCounts.Add(static_cast<EMGPrestigeCategory>(CatInt), Count);
		}

		PlayerStats.Add(PlayerId, Stats);
	}

	// Load achieved milestones per player
	int32 NumAchieved;
	LoadArchive << NumAchieved;

	for (int32 i = 0; i < NumAchieved; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		int32 NumMilestones;
		LoadArchive << NumMilestones;

		TSet<FString> MilestoneSet;
		for (int32 j = 0; j < NumMilestones; ++j)
		{
			FString MilestoneId;
			LoadArchive << MilestoneId;
			MilestoneSet.Add(MilestoneId);
		}

		AchievedMilestones.Add(PlayerId, MilestoneSet);
	}

	UE_LOG(LogTemp, Log, TEXT("MGPrestigeSubsystem: Loaded prestige data for %d players"), NumPlayers);
}

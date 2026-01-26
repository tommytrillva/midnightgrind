// Copyright Midnight Grind. All Rights Reserved.

#include "Achievements/MGAchievementSubsystem.h"

void UMGAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize badge slots
	EquippedBadgeSlots.SetNum(MaxBadgeSlots);

	InitializeAchievements();
	InitializeMilestones();
	LoadProgress();
}

void UMGAchievementSubsystem::Deinitialize()
{
	SaveProgress();
	Super::Deinitialize();
}

// ==========================================
// STAT REPORTING
// ==========================================

void UMGAchievementSubsystem::ReportStatIncrement(EMGAchievementStatType StatType, int32 Amount)
{
	int32& CurrentValue = StatValues.FindOrAdd(StatType);
	CurrentValue += Amount;

	CheckAchievementUnlocks(StatType);
	CheckMilestoneProgress(StatType);
}

void UMGAchievementSubsystem::ReportStatValue(EMGAchievementStatType StatType, int32 Value)
{
	int32& CurrentValue = StatValues.FindOrAdd(StatType);
	if (Value > CurrentValue)
	{
		CurrentValue = Value;
		CheckAchievementUnlocks(StatType);
		CheckMilestoneProgress(StatType);
	}
}

void UMGAchievementSubsystem::ReportCustomProgress(FName AchievementID, int32 Progress)
{
	FMGAchievementProgress& AchProgress = AchievementProgressMap.FindOrAdd(AchievementID);
	AchProgress.AchievementID = AchievementID;
	AchProgress.CurrentProgress = FMath::Max(AchProgress.CurrentProgress, Progress);

	// Check if achievement should unlock
	for (const FMGAchievementDefinition& Achievement : AchievementDefinitions)
	{
		if (Achievement.AchievementID == AchievementID && !AchProgress.bIsUnlocked)
		{
			if (Achievement.bIsProgressive)
			{
				// Check tier unlocks
				for (int32 i = AchProgress.CurrentTier; i < Achievement.Tiers.Num(); i++)
				{
					if (AchProgress.CurrentProgress >= Achievement.Tiers[i].RequiredProgress)
					{
						UnlockAchievement(AchievementID, i + 1);
					}
					else
					{
						break;
					}
				}
			}
			else if (AchProgress.CurrentProgress >= Achievement.TargetProgress)
			{
				UnlockAchievement(AchievementID);
			}

			float ProgressPercent = (float)AchProgress.CurrentProgress / (float)Achievement.TargetProgress;
			OnAchievementProgress.Broadcast(AchievementID, ProgressPercent);
			break;
		}
	}
}

void UMGAchievementSubsystem::ReportRaceCompletion(int32 Position, bool bIsOnline, bool bIsPerfect, FName TrackID, FName VehicleID)
{
	// Report basic stats
	ReportStatIncrement(EMGAchievementStatType::RacesCompleted, 1);

	if (Position == 1)
	{
		ReportStatIncrement(EMGAchievementStatType::RacesWon, 1);
		ReportStatIncrement(EMGAchievementStatType::FirstPlaceFinishes, 1);
		ReportStatIncrement(EMGAchievementStatType::ConsecutiveWins, 1);
	}
	else
	{
		StatValues.FindOrAdd(EMGAchievementStatType::ConsecutiveWins) = 0;
	}

	if (Position <= 3)
	{
		ReportStatIncrement(EMGAchievementStatType::PodiumFinishes, 1);
		ReportStatIncrement(EMGAchievementStatType::ConsecutivePodiums, 1);
	}
	else
	{
		StatValues.FindOrAdd(EMGAchievementStatType::ConsecutivePodiums) = 0;
	}

	if (bIsPerfect)
	{
		ReportStatIncrement(EMGAchievementStatType::PerfectRaces, 1);
	}

	if (bIsOnline)
	{
		ReportStatIncrement(EMGAchievementStatType::OnlineRacesCompleted, 1);
		if (Position == 1)
		{
			ReportStatIncrement(EMGAchievementStatType::OnlineRacesWon, 1);
		}
	}

	// Check track-specific achievements
	for (const FMGAchievementDefinition& Achievement : AchievementDefinitions)
	{
		if (!Achievement.RequiredTrack.IsNone() && Achievement.RequiredTrack == TrackID)
		{
			ReportCustomProgress(Achievement.AchievementID,
				AchievementProgressMap.FindOrAdd(Achievement.AchievementID).CurrentProgress + 1);
		}
		if (!Achievement.RequiredVehicle.IsNone() && Achievement.RequiredVehicle == VehicleID)
		{
			ReportCustomProgress(Achievement.AchievementID,
				AchievementProgressMap.FindOrAdd(Achievement.AchievementID).CurrentProgress + 1);
		}
	}
}

// ==========================================
// ACHIEVEMENT QUERIES
// ==========================================

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetAllAchievements() const
{
	return AchievementDefinitions;
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetAchievementsByCategory(EMGAchievementCategory Category) const
{
	TArray<FMGAchievementDefinition> Result;
	for (const FMGAchievementDefinition& Achievement : AchievementDefinitions)
	{
		if (Achievement.Category == Category)
		{
			Result.Add(Achievement);
		}
	}
	return Result;
}

bool UMGAchievementSubsystem::GetAchievement(FName AchievementID, FMGAchievementDefinition& OutAchievement) const
{
	for (const FMGAchievementDefinition& Achievement : AchievementDefinitions)
	{
		if (Achievement.AchievementID == AchievementID)
		{
			OutAchievement = Achievement;
			return true;
		}
	}
	return false;
}

FMGAchievementProgress UMGAchievementSubsystem::GetAchievementProgress(FName AchievementID) const
{
	if (const FMGAchievementProgress* Progress = AchievementProgressMap.Find(AchievementID))
	{
		return *Progress;
	}
	return FMGAchievementProgress();
}

bool UMGAchievementSubsystem::IsAchievementUnlocked(FName AchievementID) const
{
	if (const FMGAchievementProgress* Progress = AchievementProgressMap.Find(AchievementID))
	{
		return Progress->bIsUnlocked;
	}
	return false;
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetUnlockedAchievements() const
{
	TArray<FMGAchievementDefinition> Result;
	for (const FMGAchievementDefinition& Achievement : AchievementDefinitions)
	{
		if (IsAchievementUnlocked(Achievement.AchievementID))
		{
			Result.Add(Achievement);
		}
	}
	return Result;
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetLockedAchievements() const
{
	TArray<FMGAchievementDefinition> Result;
	for (const FMGAchievementDefinition& Achievement : AchievementDefinitions)
	{
		if (!IsAchievementUnlocked(Achievement.AchievementID))
		{
			Result.Add(Achievement);
		}
	}
	return Result;
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetRecentlyUnlocked(int32 Count) const
{
	TArray<FMGAchievementDefinition> Unlocked = GetUnlockedAchievements();

	// Sort by unlock time (most recent first)
	Unlocked.Sort([this](const FMGAchievementDefinition& A, const FMGAchievementDefinition& B) {
		const FMGAchievementProgress* ProgressA = AchievementProgressMap.Find(A.AchievementID);
		const FMGAchievementProgress* ProgressB = AchievementProgressMap.Find(B.AchievementID);
		if (ProgressA && ProgressB)
		{
			return ProgressA->UnlockTime > ProgressB->UnlockTime;
		}
		return false;
	});

	if (Unlocked.Num() > Count)
	{
		Unlocked.SetNum(Count);
	}

	return Unlocked;
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetNearestToCompletion(int32 Count) const
{
	TArray<FMGAchievementDefinition> Locked = GetLockedAchievements();

	// Sort by completion percentage (highest first)
	Locked.Sort([this](const FMGAchievementDefinition& A, const FMGAchievementDefinition& B) {
		FMGAchievementProgress ProgressA = GetAchievementProgress(A.AchievementID);
		FMGAchievementProgress ProgressB = GetAchievementProgress(B.AchievementID);

		float PercentA = (float)ProgressA.CurrentProgress / (float)FMath::Max(1, A.TargetProgress);
		float PercentB = (float)ProgressB.CurrentProgress / (float)FMath::Max(1, B.TargetProgress);

		return PercentA > PercentB;
	});

	if (Locked.Num() > Count)
	{
		Locked.SetNum(Count);
	}

	return Locked;
}

// ==========================================
// BADGES
// ==========================================

TArray<FMGBadge> UMGAchievementSubsystem::GetUnlockedBadges() const
{
	return UnlockedBadges;
}

TArray<FMGBadge> UMGAchievementSubsystem::GetEquippedBadges() const
{
	TArray<FMGBadge> Equipped;
	for (const FName& BadgeID : EquippedBadgeSlots)
	{
		if (!BadgeID.IsNone())
		{
			for (const FMGBadge& Badge : UnlockedBadges)
			{
				if (Badge.BadgeID == BadgeID)
				{
					Equipped.Add(Badge);
					break;
				}
			}
		}
	}
	return Equipped;
}

bool UMGAchievementSubsystem::EquipBadge(FName BadgeID, int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxBadgeSlots)
	{
		return false;
	}

	// Check if badge is unlocked
	bool bHasBadge = false;
	for (const FMGBadge& Badge : UnlockedBadges)
	{
		if (Badge.BadgeID == BadgeID)
		{
			bHasBadge = true;
			break;
		}
	}

	if (!bHasBadge)
	{
		return false;
	}

	// Remove from any other slot
	for (FName& SlotBadge : EquippedBadgeSlots)
	{
		if (SlotBadge == BadgeID)
		{
			SlotBadge = NAME_None;
		}
	}

	EquippedBadgeSlots[SlotIndex] = BadgeID;
	return true;
}

void UMGAchievementSubsystem::UnequipBadge(int32 SlotIndex)
{
	if (SlotIndex >= 0 && SlotIndex < MaxBadgeSlots)
	{
		EquippedBadgeSlots[SlotIndex] = NAME_None;
	}
}

// ==========================================
// TITLES
// ==========================================

TArray<FMGPlayerTitle> UMGAchievementSubsystem::GetUnlockedTitles() const
{
	return UnlockedTitles;
}

FMGPlayerTitle UMGAchievementSubsystem::GetEquippedTitle() const
{
	for (const FMGPlayerTitle& Title : UnlockedTitles)
	{
		if (Title.TitleID == EquippedTitleID)
		{
			return Title;
		}
	}
	return FMGPlayerTitle();
}

bool UMGAchievementSubsystem::EquipTitle(FName TitleID)
{
	for (const FMGPlayerTitle& Title : UnlockedTitles)
	{
		if (Title.TitleID == TitleID)
		{
			EquippedTitleID = TitleID;
			return true;
		}
	}
	return false;
}

// ==========================================
// MILESTONES
// ==========================================

float UMGAchievementSubsystem::GetMilestoneProgress(FName MilestoneID) const
{
	for (const FMGMilestone& Milestone : Milestones)
	{
		if (Milestone.MilestoneID == MilestoneID)
		{
			if (Milestone.CurrentThresholdIndex >= Milestone.Thresholds.Num())
			{
				return 1.0f; // Completed
			}

			int32 CurrentValue = GetStatValue(Milestone.StatType);
			int32 PrevThreshold = Milestone.CurrentThresholdIndex > 0 ?
				Milestone.Thresholds[Milestone.CurrentThresholdIndex - 1] : 0;
			int32 NextThreshold = Milestone.Thresholds[Milestone.CurrentThresholdIndex];

			return (float)(CurrentValue - PrevThreshold) / (float)(NextThreshold - PrevThreshold);
		}
	}
	return 0.0f;
}

// ==========================================
// STATS
// ==========================================

FMGAchievementStats UMGAchievementSubsystem::GetAchievementStats() const
{
	FMGAchievementStats Stats;
	Stats.TotalAchievements = AchievementDefinitions.Num();

	for (const FMGAchievementDefinition& Achievement : AchievementDefinitions)
	{
		Stats.TotalGamerScore += Achievement.Reward.PlatformGamerScore;

		if (IsAchievementUnlocked(Achievement.AchievementID))
		{
			Stats.UnlockedAchievements++;
			Stats.EarnedGamerScore += Achievement.Reward.PlatformGamerScore;

			const FMGAchievementProgress* Progress = AchievementProgressMap.Find(Achievement.AchievementID);
			if (Progress && Progress->UnlockTime > Stats.LastAchievementUnlock)
			{
				Stats.LastAchievementUnlock = Progress->UnlockTime;
			}

			// Track rarest
			if (Stats.RarestAchievement.IsNone() ||
				Achievement.Rarity > GetAchievement(Stats.RarestAchievement, const_cast<FMGAchievementDefinition&>(Achievement)) )
			{
				Stats.RarestAchievement = Achievement.AchievementID;
			}
		}
	}

	Stats.CompletionPercentage = Stats.TotalAchievements > 0 ?
		(float)Stats.UnlockedAchievements / (float)Stats.TotalAchievements * 100.0f : 0.0f;

	Stats.BadgesUnlocked = UnlockedBadges.Num();
	Stats.TitlesUnlocked = UnlockedTitles.Num();

	return Stats;
}

int32 UMGAchievementSubsystem::GetStatValue(EMGAchievementStatType StatType) const
{
	if (const int32* Value = StatValues.Find(StatType))
	{
		return *Value;
	}
	return 0;
}

// ==========================================
// REWARDS
// ==========================================

bool UMGAchievementSubsystem::ClaimAchievementReward(FName AchievementID)
{
	FMGAchievementProgress* Progress = AchievementProgressMap.Find(AchievementID);
	if (!Progress || !Progress->bIsUnlocked || Progress->bRewardClaimed)
	{
		return false;
	}

	FMGAchievementDefinition Achievement;
	if (GetAchievement(AchievementID, Achievement))
	{
		ApplyRewards(Achievement.Reward);
		Progress->bRewardClaimed = true;
		return true;
	}

	return false;
}

bool UMGAchievementSubsystem::HasUnclaimedRewards() const
{
	return GetUnclaimedRewardCount() > 0;
}

int32 UMGAchievementSubsystem::GetUnclaimedRewardCount() const
{
	int32 Count = 0;
	for (const auto& Pair : AchievementProgressMap)
	{
		if (Pair.Value.bIsUnlocked && !Pair.Value.bRewardClaimed)
		{
			Count++;
		}
	}
	return Count;
}

// ==========================================
// PLATFORM INTEGRATION
// ==========================================

void UMGAchievementSubsystem::SyncWithPlatform()
{
	// Would sync with Steam/Xbox/PlayStation achievement systems
	for (const auto& Pair : AchievementProgressMap)
	{
		if (Pair.Value.bIsUnlocked)
		{
			ReportToPlatform(Pair.Key);
		}
	}
}

bool UMGAchievementSubsystem::IsPlatformAchievementUnlocked(FName PlatformAchievementID) const
{
	// Would check platform SDK
	return false;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGAchievementSubsystem::InitializeAchievements()
{
	// Racing Achievements
	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_FIRST_WIN"));
		Achievement.DisplayName = FText::FromString(TEXT("First Blood"));
		Achievement.Description = FText::FromString(TEXT("Win your first race"));
		Achievement.Category = EMGAchievementCategory::Racing;
		Achievement.Rarity = EMGAchievementRarity::Common;
		Achievement.StatType = EMGAchievementStatType::RacesWon;
		Achievement.TargetProgress = 1;
		Achievement.Reward.XPReward = 500;
		Achievement.Reward.CashReward = 1000;
		Achievement.Reward.PlatformGamerScore = 10;
		AchievementDefinitions.Add(Achievement);
	}

	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_RACE_MASTER"));
		Achievement.DisplayName = FText::FromString(TEXT("Race Master"));
		Achievement.Description = FText::FromString(TEXT("Win 100 races"));
		Achievement.Category = EMGAchievementCategory::Racing;
		Achievement.Rarity = EMGAchievementRarity::Epic;
		Achievement.StatType = EMGAchievementStatType::RacesWon;
		Achievement.TargetProgress = 100;
		Achievement.bIsProgressive = true;

		// Tiers
		FMGAchievementTier Tier1; Tier1.TierLevel = 1; Tier1.TierName = FText::FromString(TEXT("Bronze")); Tier1.RequiredProgress = 10; Tier1.Reward.XPReward = 1000;
		FMGAchievementTier Tier2; Tier2.TierLevel = 2; Tier2.TierName = FText::FromString(TEXT("Silver")); Tier2.RequiredProgress = 25; Tier2.Reward.XPReward = 2500;
		FMGAchievementTier Tier3; Tier3.TierLevel = 3; Tier3.TierName = FText::FromString(TEXT("Gold")); Tier3.RequiredProgress = 50; Tier3.Reward.XPReward = 5000;
		FMGAchievementTier Tier4; Tier4.TierLevel = 4; Tier4.TierName = FText::FromString(TEXT("Platinum")); Tier4.RequiredProgress = 100; Tier4.Reward.XPReward = 10000; Tier4.Reward.TitleUnlock = FName(TEXT("TITLE_RACE_MASTER"));
		Achievement.Tiers.Add(Tier1);
		Achievement.Tiers.Add(Tier2);
		Achievement.Tiers.Add(Tier3);
		Achievement.Tiers.Add(Tier4);

		Achievement.Reward.PlatformGamerScore = 50;
		AchievementDefinitions.Add(Achievement);
	}

	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_PERFECT_RACE"));
		Achievement.DisplayName = FText::FromString(TEXT("Flawless Victory"));
		Achievement.Description = FText::FromString(TEXT("Complete a race without hitting any walls"));
		Achievement.Category = EMGAchievementCategory::Mastery;
		Achievement.Rarity = EMGAchievementRarity::Rare;
		Achievement.StatType = EMGAchievementStatType::PerfectRaces;
		Achievement.TargetProgress = 1;
		Achievement.Reward.XPReward = 2000;
		Achievement.Reward.BadgeUnlock = FName(TEXT("BADGE_PERFECT"));
		Achievement.Reward.PlatformGamerScore = 25;
		AchievementDefinitions.Add(Achievement);
	}

	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_COMEBACK_KID"));
		Achievement.DisplayName = FText::FromString(TEXT("Comeback Kid"));
		Achievement.Description = FText::FromString(TEXT("Win a race after being in last place"));
		Achievement.Category = EMGAchievementCategory::Challenge;
		Achievement.Rarity = EMGAchievementRarity::Rare;
		Achievement.StatType = EMGAchievementStatType::ComebackWins;
		Achievement.TargetProgress = 1;
		Achievement.Reward.XPReward = 3000;
		Achievement.Reward.TitleUnlock = FName(TEXT("TITLE_COMEBACK_KID"));
		Achievement.Reward.PlatformGamerScore = 30;
		AchievementDefinitions.Add(Achievement);
	}

	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_WIN_STREAK"));
		Achievement.DisplayName = FText::FromString(TEXT("Unstoppable"));
		Achievement.Description = FText::FromString(TEXT("Win 10 races in a row"));
		Achievement.Category = EMGAchievementCategory::Challenge;
		Achievement.Rarity = EMGAchievementRarity::Legendary;
		Achievement.StatType = EMGAchievementStatType::ConsecutiveWins;
		Achievement.TargetProgress = 10;
		Achievement.Reward.XPReward = 10000;
		Achievement.Reward.CashReward = 50000;
		Achievement.Reward.TitleUnlock = FName(TEXT("TITLE_UNSTOPPABLE"));
		Achievement.Reward.PlatformGamerScore = 75;
		AchievementDefinitions.Add(Achievement);
	}

	// Social Achievements
	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_CREW_UP"));
		Achievement.DisplayName = FText::FromString(TEXT("Crew Up"));
		Achievement.Description = FText::FromString(TEXT("Join or create a crew"));
		Achievement.Category = EMGAchievementCategory::Social;
		Achievement.Rarity = EMGAchievementRarity::Common;
		Achievement.StatType = EMGAchievementStatType::CrewsJoined;
		Achievement.TargetProgress = 1;
		Achievement.Reward.XPReward = 500;
		Achievement.Reward.PlatformGamerScore = 10;
		AchievementDefinitions.Add(Achievement);
	}

	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_PHOTO_PRO"));
		Achievement.DisplayName = FText::FromString(TEXT("Photographer"));
		Achievement.Description = FText::FromString(TEXT("Take 50 photos in Photo Mode"));
		Achievement.Category = EMGAchievementCategory::Social;
		Achievement.Rarity = EMGAchievementRarity::Uncommon;
		Achievement.StatType = EMGAchievementStatType::PhotosTaken;
		Achievement.TargetProgress = 50;
		Achievement.Reward.XPReward = 2000;
		Achievement.Reward.BadgeUnlock = FName(TEXT("BADGE_PHOTOGRAPHER"));
		Achievement.Reward.PlatformGamerScore = 20;
		AchievementDefinitions.Add(Achievement);
	}

	// Collection Achievements
	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_COLLECTOR"));
		Achievement.DisplayName = FText::FromString(TEXT("Car Collector"));
		Achievement.Description = FText::FromString(TEXT("Own 20 vehicles"));
		Achievement.Category = EMGAchievementCategory::Collection;
		Achievement.Rarity = EMGAchievementRarity::Rare;
		Achievement.StatType = EMGAchievementStatType::VehiclesOwned;
		Achievement.TargetProgress = 20;
		Achievement.Reward.XPReward = 5000;
		Achievement.Reward.CashReward = 25000;
		Achievement.Reward.PlatformGamerScore = 40;
		AchievementDefinitions.Add(Achievement);
	}

	// Multiplayer Achievements
	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_ONLINE_WARRIOR"));
		Achievement.DisplayName = FText::FromString(TEXT("Online Warrior"));
		Achievement.Description = FText::FromString(TEXT("Win 50 online races"));
		Achievement.Category = EMGAchievementCategory::Racing;
		Achievement.Rarity = EMGAchievementRarity::Epic;
		Achievement.StatType = EMGAchievementStatType::OnlineRacesWon;
		Achievement.TargetProgress = 50;
		Achievement.Reward.XPReward = 7500;
		Achievement.Reward.TitleUnlock = FName(TEXT("TITLE_ONLINE_WARRIOR"));
		Achievement.Reward.PlatformGamerScore = 50;
		AchievementDefinitions.Add(Achievement);
	}

	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_TOURNAMENT_CHAMP"));
		Achievement.DisplayName = FText::FromString(TEXT("Tournament Champion"));
		Achievement.Description = FText::FromString(TEXT("Win a tournament"));
		Achievement.Category = EMGAchievementCategory::Challenge;
		Achievement.Rarity = EMGAchievementRarity::Legendary;
		Achievement.StatType = EMGAchievementStatType::TournamentWins;
		Achievement.TargetProgress = 1;
		Achievement.Reward.XPReward = 15000;
		Achievement.Reward.CashReward = 100000;
		Achievement.Reward.TitleUnlock = FName(TEXT("TITLE_CHAMPION"));
		Achievement.Reward.BadgeUnlock = FName(TEXT("BADGE_CHAMPION"));
		Achievement.Reward.PlatformGamerScore = 100;
		AchievementDefinitions.Add(Achievement);
	}

	// Secret Achievements
	{
		FMGAchievementDefinition Achievement;
		Achievement.AchievementID = FName(TEXT("ACH_SECRET_NIGHT"));
		Achievement.DisplayName = FText::FromString(TEXT("Midnight Legend"));
		Achievement.Description = FText::FromString(TEXT("Win a race at exactly midnight"));
		Achievement.HiddenDescription = FText::FromString(TEXT("???"));
		Achievement.Category = EMGAchievementCategory::Secret;
		Achievement.Rarity = EMGAchievementRarity::Legendary;
		Achievement.StatType = EMGAchievementStatType::Custom;
		Achievement.TargetProgress = 1;
		Achievement.bIsSecret = true;
		Achievement.Reward.XPReward = 5000;
		Achievement.Reward.TitleUnlock = FName(TEXT("TITLE_MIDNIGHT_LEGEND"));
		Achievement.Reward.PlatformGamerScore = 50;
		AchievementDefinitions.Add(Achievement);
	}

	// Initialize titles and badges
	{
		FMGPlayerTitle Title;
		Title.TitleID = FName(TEXT("TITLE_NEWCOMER"));
		Title.DisplayText = FText::FromString(TEXT("Newcomer"));
		Title.TitleColor = FLinearColor::White;
		Title.Rarity = EMGAchievementRarity::Common;
		UnlockedTitles.Add(Title); // Default title
		EquippedTitleID = Title.TitleID;
	}
}

void UMGAchievementSubsystem::InitializeMilestones()
{
	{
		FMGMilestone Milestone;
		Milestone.MilestoneID = FName(TEXT("MILE_DISTANCE"));
		Milestone.DisplayName = FText::FromString(TEXT("Distance Traveled"));
		Milestone.Description = FText::FromString(TEXT("Total distance driven"));
		Milestone.StatType = EMGAchievementStatType::TotalDistance;
		Milestone.Thresholds = { 100, 500, 1000, 5000, 10000, 50000, 100000 };

		for (int32 i = 0; i < Milestone.Thresholds.Num(); i++)
		{
			FMGAchievementReward Reward;
			Reward.CashReward = Milestone.Thresholds[i] * 10;
			Reward.XPReward = Milestone.Thresholds[i] * 5;
			Milestone.ThresholdRewards.Add(Reward);
		}

		Milestones.Add(Milestone);
	}

	{
		FMGMilestone Milestone;
		Milestone.MilestoneID = FName(TEXT("MILE_DRIFT"));
		Milestone.DisplayName = FText::FromString(TEXT("Drift King"));
		Milestone.Description = FText::FromString(TEXT("Total drift distance"));
		Milestone.StatType = EMGAchievementStatType::TotalDriftDistance;
		Milestone.Thresholds = { 1000, 5000, 10000, 50000, 100000, 500000, 1000000 };

		for (int32 i = 0; i < Milestone.Thresholds.Num(); i++)
		{
			FMGAchievementReward Reward;
			Reward.CashReward = Milestone.Thresholds[i] / 10;
			Reward.XPReward = Milestone.Thresholds[i] / 20;
			Milestone.ThresholdRewards.Add(Reward);
		}

		Milestones.Add(Milestone);
	}

	{
		FMGMilestone Milestone;
		Milestone.MilestoneID = FName(TEXT("MILE_PLAYTIME"));
		Milestone.DisplayName = FText::FromString(TEXT("Dedicated Racer"));
		Milestone.Description = FText::FromString(TEXT("Total time played (hours)"));
		Milestone.StatType = EMGAchievementStatType::TotalPlayTime;
		Milestone.Thresholds = { 1, 10, 25, 50, 100, 250, 500 };

		for (int32 i = 0; i < Milestone.Thresholds.Num(); i++)
		{
			FMGAchievementReward Reward;
			Reward.XPReward = Milestone.Thresholds[i] * 1000;
			Milestone.ThresholdRewards.Add(Reward);
		}

		Milestones.Add(Milestone);
	}
}

void UMGAchievementSubsystem::CheckAchievementUnlocks(EMGAchievementStatType StatType)
{
	int32 CurrentValue = GetStatValue(StatType);

	for (const FMGAchievementDefinition& Achievement : AchievementDefinitions)
	{
		if (Achievement.StatType != StatType)
		{
			continue;
		}

		FMGAchievementProgress& Progress = AchievementProgressMap.FindOrAdd(Achievement.AchievementID);
		Progress.AchievementID = Achievement.AchievementID;
		Progress.CurrentProgress = CurrentValue;

		if (Progress.bIsUnlocked)
		{
			continue;
		}

		// Check prerequisites
		bool bPrerequisitesMet = true;
		for (const FName& PrereqID : Achievement.PrerequisiteAchievements)
		{
			if (!IsAchievementUnlocked(PrereqID))
			{
				bPrerequisitesMet = false;
				break;
			}
		}

		if (!bPrerequisitesMet)
		{
			continue;
		}

		if (Achievement.bIsProgressive)
		{
			for (int32 i = Progress.CurrentTier; i < Achievement.Tiers.Num(); i++)
			{
				if (CurrentValue >= Achievement.Tiers[i].RequiredProgress)
				{
					UnlockAchievement(Achievement.AchievementID, i + 1);
				}
				else
				{
					break;
				}
			}
		}
		else if (CurrentValue >= Achievement.TargetProgress)
		{
			UnlockAchievement(Achievement.AchievementID);
		}

		float ProgressPercent = (float)CurrentValue / (float)Achievement.TargetProgress;
		OnAchievementProgress.Broadcast(Achievement.AchievementID, FMath::Min(1.0f, ProgressPercent));
	}
}

void UMGAchievementSubsystem::UnlockAchievement(FName AchievementID, int32 Tier)
{
	FMGAchievementProgress& Progress = AchievementProgressMap.FindOrAdd(AchievementID);
	Progress.AchievementID = AchievementID;
	Progress.UnlockTime = FDateTime::Now();

	FMGAchievementDefinition Achievement;
	if (!GetAchievement(AchievementID, Achievement))
	{
		return;
	}

	if (Achievement.bIsProgressive && Tier > 0)
	{
		Progress.CurrentTier = Tier;
		if (Tier >= Achievement.Tiers.Num())
		{
			Progress.bIsUnlocked = true;
		}

		// Apply tier reward
		if (Tier <= Achievement.Tiers.Num())
		{
			ApplyRewards(Achievement.Tiers[Tier - 1].Reward);
		}
	}
	else
	{
		Progress.bIsUnlocked = true;
	}

	// Report to platform
	ReportToPlatform(AchievementID);

	OnAchievementUnlocked.Broadcast(Achievement, Tier);

	// Unlock any associated badge/title
	if (!Achievement.Reward.BadgeUnlock.IsNone())
	{
		UnlockBadge(Achievement.Reward.BadgeUnlock);
	}
	if (!Achievement.Reward.TitleUnlock.IsNone())
	{
		UnlockTitle(Achievement.Reward.TitleUnlock);
	}
}

void UMGAchievementSubsystem::UnlockBadge(FName BadgeID)
{
	// Check if already unlocked
	for (const FMGBadge& Badge : UnlockedBadges)
	{
		if (Badge.BadgeID == BadgeID)
		{
			return;
		}
	}

	FMGBadge NewBadge;
	NewBadge.BadgeID = BadgeID;
	NewBadge.DisplayName = FText::FromString(BadgeID.ToString());
	NewBadge.UnlockTime = FDateTime::Now();

	UnlockedBadges.Add(NewBadge);
	OnBadgeUnlocked.Broadcast(NewBadge);
}

void UMGAchievementSubsystem::UnlockTitle(FName TitleID)
{
	// Check if already unlocked
	for (const FMGPlayerTitle& Title : UnlockedTitles)
	{
		if (Title.TitleID == TitleID)
		{
			return;
		}
	}

	FMGPlayerTitle NewTitle;
	NewTitle.TitleID = TitleID;
	NewTitle.DisplayText = FText::FromString(TitleID.ToString().Replace(TEXT("TITLE_"), TEXT("")).Replace(TEXT("_"), TEXT(" ")));
	NewTitle.UnlockTime = FDateTime::Now();

	UnlockedTitles.Add(NewTitle);
	OnTitleUnlocked.Broadcast(NewTitle);
}

void UMGAchievementSubsystem::CheckMilestoneProgress(EMGAchievementStatType StatType)
{
	int32 CurrentValue = GetStatValue(StatType);

	for (FMGMilestone& Milestone : Milestones)
	{
		if (Milestone.StatType != StatType)
		{
			continue;
		}

		while (Milestone.CurrentThresholdIndex < Milestone.Thresholds.Num() &&
			   CurrentValue >= Milestone.Thresholds[Milestone.CurrentThresholdIndex])
		{
			// Apply reward
			if (Milestone.CurrentThresholdIndex < Milestone.ThresholdRewards.Num())
			{
				ApplyRewards(Milestone.ThresholdRewards[Milestone.CurrentThresholdIndex]);
			}

			OnMilestoneReached.Broadcast(Milestone, Milestone.CurrentThresholdIndex);
			Milestone.CurrentThresholdIndex++;
		}
	}
}

void UMGAchievementSubsystem::ReportToPlatform(FName AchievementID)
{
	FMGAchievementDefinition Achievement;
	if (GetAchievement(AchievementID, Achievement) && !Achievement.PlatformAchievementID.IsNone())
	{
		// Would call platform SDK to unlock achievement
		// Steam: SteamUserStats()->SetAchievement()
		// Xbox: XGameUiShowAchievementUnlocked()
		// PlayStation: sceNpTrophyUnlockTrophy()
	}
}

void UMGAchievementSubsystem::ApplyRewards(const FMGAchievementReward& Reward)
{
	// Would apply rewards through economy system
	// For now, just log
	if (Reward.CashReward > 0 || Reward.XPReward > 0)
	{
		// Apply cash, XP, reputation
	}
}

void UMGAchievementSubsystem::LoadProgress()
{
	// Would load from save system
}

void UMGAchievementSubsystem::SaveProgress()
{
	// Would save to save system
}

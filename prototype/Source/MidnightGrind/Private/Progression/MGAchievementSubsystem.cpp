// Copyright Midnight Grind. All Rights Reserved.

#include "Progression/MGAchievementSubsystem.h"

void UMGAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadAchievementDefinitions();
	LoadChallengePool();
	BuildStatMappings();

	// Generate initial challenges
	GenerateDailyChallenges();
	GenerateWeeklyChallenges();
}

void UMGAchievementSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// ACHIEVEMENTS
// ==========================================

FMGAchievementDef UMGAchievementSubsystem::GetAchievement(FName AchievementID) const
{
	for (const FMGAchievementDef& Achievement : Achievements)
	{
		if (Achievement.AchievementID == AchievementID)
		{
			return Achievement;
		}
	}
	return FMGAchievementDef();
}

FMGAchievementProgress UMGAchievementSubsystem::GetAchievementProgress(FName AchievementID) const
{
	if (const FMGAchievementProgress* Progress = AchievementProgress.Find(AchievementID))
	{
		return *Progress;
	}

	FMGAchievementProgress DefaultProgress;
	DefaultProgress.AchievementID = AchievementID;
	return DefaultProgress;
}

TArray<FMGAchievementProgress> UMGAchievementSubsystem::GetAllAchievementProgress() const
{
	TArray<FMGAchievementProgress> Result;
	AchievementProgress.GenerateValueArray(Result);
	return Result;
}

TArray<FMGAchievementDef> UMGAchievementSubsystem::GetUnlockedAchievements() const
{
	TArray<FMGAchievementDef> Result;

	for (const FMGAchievementDef& Achievement : Achievements)
	{
		if (IsAchievementUnlocked(Achievement.AchievementID))
		{
			Result.Add(Achievement);
		}
	}

	return Result;
}

TArray<FMGAchievementDef> UMGAchievementSubsystem::GetLockedAchievements() const
{
	TArray<FMGAchievementDef> Result;

	for (const FMGAchievementDef& Achievement : Achievements)
	{
		if (!IsAchievementUnlocked(Achievement.AchievementID))
		{
			// Don't include secret achievements
			if (!Achievement.bIsSecret)
			{
				Result.Add(Achievement);
			}
		}
	}

	return Result;
}

void UMGAchievementSubsystem::UpdateAchievementProgress(FName AchievementID, int32 Progress)
{
	FMGAchievementDef Achievement = GetAchievement(AchievementID);
	if (Achievement.AchievementID.IsNone())
	{
		return;
	}

	FMGAchievementProgress& AchProgress = AchievementProgress.FindOrAdd(AchievementID);
	AchProgress.AchievementID = AchievementID;

	if (AchProgress.bIsUnlocked)
	{
		return; // Already unlocked
	}

	int32 OldProgress = AchProgress.CurrentProgress;
	AchProgress.CurrentProgress = Progress;

	// Check for tier upgrades
	if (Achievement.Type == EMGAchievementType::Tiered)
	{
		for (int32 i = 0; i < Achievement.TierThresholds.Num(); ++i)
		{
			if (Progress >= Achievement.TierThresholds[i] && AchProgress.CurrentTier <= i)
			{
				AchProgress.CurrentTier = i + 1;
			}
		}
	}

	// Broadcast progress
	if (OldProgress != Progress)
	{
		OnAchievementProgress.Broadcast(AchievementID, Progress, Achievement.RequiredProgress);
	}

	// Check for completion
	if (Progress >= Achievement.RequiredProgress)
	{
		AchProgress.bIsUnlocked = true;
		AchProgress.UnlockTime = FDateTime::Now();

		GrantAchievementReward(Achievement);
		OnAchievementUnlocked.Broadcast(AchievementID, Achievement);
	}
}

void UMGAchievementSubsystem::IncrementAchievement(FName AchievementID, int32 Amount)
{
	FMGAchievementProgress Progress = GetAchievementProgress(AchievementID);
	UpdateAchievementProgress(AchievementID, Progress.CurrentProgress + Amount);
}

void UMGAchievementSubsystem::UnlockAchievement(FName AchievementID)
{
	FMGAchievementDef Achievement = GetAchievement(AchievementID);
	if (Achievement.AchievementID.IsNone())
	{
		return;
	}

	UpdateAchievementProgress(AchievementID, Achievement.RequiredProgress);
}

bool UMGAchievementSubsystem::IsAchievementUnlocked(FName AchievementID) const
{
	if (const FMGAchievementProgress* Progress = AchievementProgress.Find(AchievementID))
	{
		return Progress->bIsUnlocked;
	}
	return false;
}

int32 UMGAchievementSubsystem::GetUnlockedAchievementCount() const
{
	int32 Count = 0;
	for (const auto& Pair : AchievementProgress)
	{
		if (Pair.Value.bIsUnlocked)
		{
			Count++;
		}
	}
	return Count;
}

// ==========================================
// CHALLENGES
// ==========================================

TArray<FMGChallengeProgress> UMGAchievementSubsystem::GetDailyChallenges() const
{
	TArray<FMGChallengeProgress> Result;

	for (const FMGChallengeProgress& Challenge : ActiveChallenges)
	{
		if (Challenge.Challenge.Type == EMGChallengeType::Daily)
		{
			Result.Add(Challenge);
		}
	}

	return Result;
}

TArray<FMGChallengeProgress> UMGAchievementSubsystem::GetWeeklyChallenges() const
{
	TArray<FMGChallengeProgress> Result;

	for (const FMGChallengeProgress& Challenge : ActiveChallenges)
	{
		if (Challenge.Challenge.Type == EMGChallengeType::Weekly)
		{
			Result.Add(Challenge);
		}
	}

	return Result;
}

void UMGAchievementSubsystem::UpdateChallengeProgress(FName ChallengeID, int32 Progress)
{
	for (FMGChallengeProgress& Challenge : ActiveChallenges)
	{
		if (Challenge.Challenge.ChallengeID == ChallengeID)
		{
			if (Challenge.bIsCompleted)
			{
				return; // Already completed
			}

			Challenge.CurrentProgress = Progress;

			if (Progress >= Challenge.Challenge.RequiredProgress)
			{
				Challenge.bIsCompleted = true;
				OnChallengeCompleted.Broadcast(Challenge);
			}

			break;
		}
	}
}

void UMGAchievementSubsystem::IncrementChallenge(FName ChallengeID, int32 Amount)
{
	for (FMGChallengeProgress& Challenge : ActiveChallenges)
	{
		if (Challenge.Challenge.ChallengeID == ChallengeID)
		{
			UpdateChallengeProgress(ChallengeID, Challenge.CurrentProgress + Amount);
			break;
		}
	}
}

bool UMGAchievementSubsystem::ClaimChallengeReward(FName ChallengeID)
{
	for (FMGChallengeProgress& Challenge : ActiveChallenges)
	{
		if (Challenge.Challenge.ChallengeID == ChallengeID)
		{
			if (Challenge.bIsCompleted && !Challenge.bIsClaimed)
			{
				Challenge.bIsClaimed = true;
				GrantChallengeReward(Challenge);
				return true;
			}
			break;
		}
	}

	return false;
}

void UMGAchievementSubsystem::RefreshChallenges()
{
	CheckChallengeExpiration();

	// Check if we need new dailies
	bool bNeedDailies = GetDailyChallenges().Num() == 0;
	if (bNeedDailies)
	{
		GenerateDailyChallenges();
	}

	// Check if we need new weeklies
	bool bNeedWeeklies = GetWeeklyChallenges().Num() == 0;
	if (bNeedWeeklies)
	{
		GenerateWeeklyChallenges();
	}

	OnChallengesRefreshed.Broadcast();
}

// ==========================================
// STATS
// ==========================================

void UMGAchievementSubsystem::ReportStat(FName StatID, int32 Value)
{
	PlayerStats.Add(StatID, Value);

	// Update achievements tracking this stat
	if (const TArray<FName>* AchievementIDs = StatToAchievementMap.Find(StatID))
	{
		for (const FName& AchID : *AchievementIDs)
		{
			UpdateAchievementProgress(AchID, Value);
		}
	}

	// Update challenges tracking this stat
	if (const TArray<FName>* ChallengeIDs = StatToChallengeMap.Find(StatID))
	{
		for (const FName& ChalID : *ChallengeIDs)
		{
			UpdateChallengeProgress(ChalID, Value);
		}
	}
}

void UMGAchievementSubsystem::IncrementStat(FName StatID, int32 Amount)
{
	int32 CurrentValue = GetStatValue(StatID);
	ReportStat(StatID, CurrentValue + Amount);
}

int32 UMGAchievementSubsystem::GetStatValue(FName StatID) const
{
	if (const int32* Value = PlayerStats.Find(StatID))
	{
		return *Value;
	}
	return 0;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGAchievementSubsystem::LoadAchievementDefinitions()
{
	// Define built-in achievements
	// In production, these would come from data assets

	// Racing achievements
	{
		FMGAchievementDef Ach;
		Ach.AchievementID = FName("FirstWin");
		Ach.Name = FText::FromString("First Victory");
		Ach.Description = FText::FromString("Win your first race");
		Ach.Type = EMGAchievementType::Standard;
		Ach.Rarity = EMGAchievementRarity::Common;
		Ach.RequiredProgress = 1;
		Ach.CashReward = 1000;
		Ach.ReputationReward = 100;
		Ach.TrackedStat = FName("TotalWins");
		Achievements.Add(Ach);
	}

	{
		FMGAchievementDef Ach;
		Ach.AchievementID = FName("RacingVeteran");
		Ach.Name = FText::FromString("Racing Veteran");
		Ach.Description = FText::FromString("Win 100 races");
		Ach.Type = EMGAchievementType::Tiered;
		Ach.Rarity = EMGAchievementRarity::Rare;
		Ach.RequiredProgress = 100;
		Ach.TierThresholds = { 10, 50, 100 };
		Ach.CashReward = 10000;
		Ach.ReputationReward = 1000;
		Ach.TrackedStat = FName("TotalWins");
		Achievements.Add(Ach);
	}

	{
		FMGAchievementDef Ach;
		Ach.AchievementID = FName("DriftKing");
		Ach.Name = FText::FromString("Drift King");
		Ach.Description = FText::FromString("Accumulate 100,000 drift score");
		Ach.Type = EMGAchievementType::Cumulative;
		Ach.Rarity = EMGAchievementRarity::Epic;
		Ach.RequiredProgress = 100000;
		Ach.CashReward = 5000;
		Ach.ReputationReward = 500;
		Ach.TrackedStat = FName("TotalDriftScore");
		Achievements.Add(Ach);
	}

	{
		FMGAchievementDef Ach;
		Ach.AchievementID = FName("PerfectLap");
		Ach.Name = FText::FromString("Perfect Lap");
		Ach.Description = FText::FromString("Complete a lap without hitting any walls");
		Ach.Type = EMGAchievementType::Standard;
		Ach.Rarity = EMGAchievementRarity::Uncommon;
		Ach.RequiredProgress = 1;
		Ach.CashReward = 2000;
		Ach.ReputationReward = 200;
		Ach.TrackedStat = FName("CleanLaps");
		Achievements.Add(Ach);
	}

	{
		FMGAchievementDef Ach;
		Ach.AchievementID = FName("Collector");
		Ach.Name = FText::FromString("Collector");
		Ach.Description = FText::FromString("Own 10 different vehicles");
		Ach.Type = EMGAchievementType::Tiered;
		Ach.Rarity = EMGAchievementRarity::Rare;
		Ach.RequiredProgress = 10;
		Ach.TierThresholds = { 3, 5, 10 };
		Ach.CashReward = 5000;
		Ach.ReputationReward = 500;
		Ach.TrackedStat = FName("VehiclesOwned");
		Achievements.Add(Ach);
	}

	{
		FMGAchievementDef Ach;
		Ach.AchievementID = FName("NightOwl");
		Ach.Name = FText::FromString("Night Owl");
		Ach.Description = FText::FromString("Win 10 races at night");
		Ach.Type = EMGAchievementType::Cumulative;
		Ach.Rarity = EMGAchievementRarity::Uncommon;
		Ach.RequiredProgress = 10;
		Ach.CashReward = 3000;
		Ach.ReputationReward = 300;
		Ach.TrackedStat = FName("NightWins");
		Achievements.Add(Ach);
	}
}

void UMGAchievementSubsystem::LoadChallengePool()
{
	// Daily challenges
	{
		FMGChallengeDef Challenge;
		Challenge.ChallengeID = FName("Daily_Win3");
		Challenge.Name = FText::FromString("Triple Threat");
		Challenge.Description = FText::FromString("Win 3 races");
		Challenge.Type = EMGChallengeType::Daily;
		Challenge.RequiredProgress = 3;
		Challenge.CashReward = 1500;
		Challenge.ReputationReward = 150;
		Challenge.TrackedStat = FName("TotalWins");
		ChallengePool.Add(Challenge);
	}

	{
		FMGChallengeDef Challenge;
		Challenge.ChallengeID = FName("Daily_Drift5000");
		Challenge.Name = FText::FromString("Sideways");
		Challenge.Description = FText::FromString("Score 5000 drift points");
		Challenge.Type = EMGChallengeType::Daily;
		Challenge.RequiredProgress = 5000;
		Challenge.CashReward = 1000;
		Challenge.ReputationReward = 100;
		Challenge.TrackedStat = FName("TotalDriftScore");
		ChallengePool.Add(Challenge);
	}

	{
		FMGChallengeDef Challenge;
		Challenge.ChallengeID = FName("Daily_NOS10");
		Challenge.Name = FText::FromString("Nitro Boost");
		Challenge.Description = FText::FromString("Use NOS 10 times");
		Challenge.Type = EMGChallengeType::Daily;
		Challenge.RequiredProgress = 10;
		Challenge.CashReward = 800;
		Challenge.ReputationReward = 80;
		Challenge.TrackedStat = FName("NOSUsed");
		ChallengePool.Add(Challenge);
	}

	// Weekly challenges
	{
		FMGChallengeDef Challenge;
		Challenge.ChallengeID = FName("Weekly_Win15");
		Challenge.Name = FText::FromString("Weekly Champion");
		Challenge.Description = FText::FromString("Win 15 races this week");
		Challenge.Type = EMGChallengeType::Weekly;
		Challenge.RequiredProgress = 15;
		Challenge.CashReward = 5000;
		Challenge.ReputationReward = 500;
		Challenge.TrackedStat = FName("TotalWins");
		ChallengePool.Add(Challenge);
	}

	{
		FMGChallengeDef Challenge;
		Challenge.ChallengeID = FName("Weekly_Race50");
		Challenge.Name = FText::FromString("Road Warrior");
		Challenge.Description = FText::FromString("Complete 50 races");
		Challenge.Type = EMGChallengeType::Weekly;
		Challenge.RequiredProgress = 50;
		Challenge.CashReward = 3000;
		Challenge.ReputationReward = 300;
		Challenge.TrackedStat = FName("TotalRaces");
		ChallengePool.Add(Challenge);
	}
}

void UMGAchievementSubsystem::GenerateDailyChallenges()
{
	// Remove expired daily challenges
	ActiveChallenges.RemoveAll([](const FMGChallengeProgress& C)
	{
		return C.Challenge.Type == EMGChallengeType::Daily;
	});

	// Pick 3 random daily challenges
	TArray<FMGChallengeDef> DailyChallenges;
	for (const FMGChallengeDef& Challenge : ChallengePool)
	{
		if (Challenge.Type == EMGChallengeType::Daily)
		{
			DailyChallenges.Add(Challenge);
		}
	}

	int32 NumToAdd = FMath::Min(3, DailyChallenges.Num());
	for (int32 i = 0; i < NumToAdd; ++i)
	{
		int32 RandomIndex = FMath::RandRange(0, DailyChallenges.Num() - 1);

		FMGChallengeProgress Progress;
		Progress.Challenge = DailyChallenges[RandomIndex];
		Progress.StartTime = FDateTime::Now();
		Progress.ExpirationTime = Progress.StartTime + FTimespan::FromHours(24);

		ActiveChallenges.Add(Progress);
		DailyChallenges.RemoveAt(RandomIndex);
	}
}

void UMGAchievementSubsystem::GenerateWeeklyChallenges()
{
	// Remove expired weekly challenges
	ActiveChallenges.RemoveAll([](const FMGChallengeProgress& C)
	{
		return C.Challenge.Type == EMGChallengeType::Weekly;
	});

	// Pick 2 random weekly challenges
	TArray<FMGChallengeDef> WeeklyChallenges;
	for (const FMGChallengeDef& Challenge : ChallengePool)
	{
		if (Challenge.Type == EMGChallengeType::Weekly)
		{
			WeeklyChallenges.Add(Challenge);
		}
	}

	int32 NumToAdd = FMath::Min(2, WeeklyChallenges.Num());
	for (int32 i = 0; i < NumToAdd; ++i)
	{
		int32 RandomIndex = FMath::RandRange(0, WeeklyChallenges.Num() - 1);

		FMGChallengeProgress Progress;
		Progress.Challenge = WeeklyChallenges[RandomIndex];
		Progress.StartTime = FDateTime::Now();
		Progress.ExpirationTime = Progress.StartTime + FTimespan::FromDays(7);

		ActiveChallenges.Add(Progress);
		WeeklyChallenges.RemoveAt(RandomIndex);
	}
}

void UMGAchievementSubsystem::CheckChallengeExpiration()
{
	FDateTime Now = FDateTime::Now();

	ActiveChallenges.RemoveAll([&Now](const FMGChallengeProgress& C)
	{
		return Now >= C.ExpirationTime;
	});
}

void UMGAchievementSubsystem::GrantAchievementReward(const FMGAchievementDef& Achievement)
{
	// Would integrate with economy system
	// For now, just broadcast the unlock
}

void UMGAchievementSubsystem::GrantChallengeReward(const FMGChallengeProgress& Challenge)
{
	// Would integrate with economy system
}

void UMGAchievementSubsystem::BuildStatMappings()
{
	// Build achievement mappings
	for (const FMGAchievementDef& Achievement : Achievements)
	{
		if (!Achievement.TrackedStat.IsNone())
		{
			StatToAchievementMap.FindOrAdd(Achievement.TrackedStat).Add(Achievement.AchievementID);
		}
	}

	// Challenge mappings are built dynamically as challenges are generated
}

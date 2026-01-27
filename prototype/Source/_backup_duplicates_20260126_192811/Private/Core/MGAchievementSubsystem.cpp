// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGAchievementSubsystem.h"
#include "Engine/DataTable.h"

void UMGAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize all stats to 0
	for (int32 i = 0; i < static_cast<int32>(EMGAchievementStatType::Custom); ++i)
	{
		Stats.Add(static_cast<EMGAchievementStatType>(i), 0);
	}
}

void UMGAchievementSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// ACHIEVEMENT REGISTRATION
// ==========================================

void UMGAchievementSubsystem::RegisterAchievement(const FMGAchievementDefinition& Definition)
{
	RegisteredAchievements.Add(Definition.AchievementID, Definition);

	// Build stat mapping for quick lookup
	if (Definition.StatType != EMGAchievementStatType::Custom)
	{
		StatToAchievementMap.Add(Definition.StatType, Definition.AchievementID);
	}
	else if (!Definition.CustomStatID.IsNone())
	{
		CustomStatToAchievementMap.Add(Definition.CustomStatID, Definition.AchievementID);
	}

	// Initialize progress if not exists
	if (!AchievementProgress.Contains(Definition.AchievementID))
	{
		FMGAchievementProgress NewProgress;
		NewProgress.AchievementID = Definition.AchievementID;
		NewProgress.CurrentValue = 0;
		NewProgress.bUnlocked = false;
		AchievementProgress.Add(Definition.AchievementID, NewProgress);
	}
}

void UMGAchievementSubsystem::RegisterAchievementsFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return;
	}

	TArray<FMGAchievementDefinition*> Rows;
	DataTable->GetAllRows<FMGAchievementDefinition>(TEXT(""), Rows);

	for (FMGAchievementDefinition* Row : Rows)
	{
		if (Row)
		{
			RegisterAchievement(*Row);
		}
	}
}

bool UMGAchievementSubsystem::IsAchievementRegistered(FName AchievementID) const
{
	return RegisteredAchievements.Contains(AchievementID);
}

FMGAchievementDefinition UMGAchievementSubsystem::GetAchievementDefinition(FName AchievementID) const
{
	const FMGAchievementDefinition* Definition = RegisteredAchievements.Find(AchievementID);
	return Definition ? *Definition : FMGAchievementDefinition();
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetAllAchievements() const
{
	TArray<FMGAchievementDefinition> Result;
	RegisteredAchievements.GenerateValueArray(Result);
	return Result;
}

// ==========================================
// STAT TRACKING
// ==========================================

void UMGAchievementSubsystem::IncrementStat(EMGAchievementStatType StatType, int32 Amount)
{
	int32 OldValue = Stats.FindOrAdd(StatType);
	int32 NewValue = OldValue + Amount;
	Stats[StatType] = NewValue;

	// Broadcast stat change
	FMGStatChangeEvent Event;
	Event.StatType = StatType;
	Event.OldValue = OldValue;
	Event.NewValue = NewValue;
	Event.Delta = Amount;
	OnStatChanged.Broadcast(Event);

	// Check achievements
	CheckAchievementsForStat(StatType);
}

void UMGAchievementSubsystem::SetStat(EMGAchievementStatType StatType, int32 Value)
{
	int32 OldValue = Stats.FindOrAdd(StatType);
	Stats[StatType] = Value;

	if (OldValue != Value)
	{
		FMGStatChangeEvent Event;
		Event.StatType = StatType;
		Event.OldValue = OldValue;
		Event.NewValue = Value;
		Event.Delta = Value - OldValue;
		OnStatChanged.Broadcast(Event);

		CheckAchievementsForStat(StatType);
	}
}

void UMGAchievementSubsystem::SetStatMax(EMGAchievementStatType StatType, int32 Value)
{
	int32 CurrentValue = Stats.FindOrAdd(StatType);
	if (Value > CurrentValue)
	{
		SetStat(StatType, Value);
	}
}

int32 UMGAchievementSubsystem::GetStat(EMGAchievementStatType StatType) const
{
	const int32* Value = Stats.Find(StatType);
	return Value ? *Value : 0;
}

void UMGAchievementSubsystem::IncrementCustomStat(FName StatID, int32 Amount)
{
	int32 OldValue = CustomStats.FindOrAdd(StatID);
	int32 NewValue = OldValue + Amount;
	CustomStats[StatID] = NewValue;

	FMGStatChangeEvent Event;
	Event.StatType = EMGAchievementStatType::Custom;
	Event.CustomStatID = StatID;
	Event.OldValue = OldValue;
	Event.NewValue = NewValue;
	Event.Delta = Amount;
	OnStatChanged.Broadcast(Event);

	CheckAchievementForCustomStat(StatID);
}

void UMGAchievementSubsystem::SetCustomStat(FName StatID, int32 Value)
{
	int32 OldValue = CustomStats.FindOrAdd(StatID);
	CustomStats[StatID] = Value;

	if (OldValue != Value)
	{
		FMGStatChangeEvent Event;
		Event.StatType = EMGAchievementStatType::Custom;
		Event.CustomStatID = StatID;
		Event.OldValue = OldValue;
		Event.NewValue = Value;
		Event.Delta = Value - OldValue;
		OnStatChanged.Broadcast(Event);

		CheckAchievementForCustomStat(StatID);
	}
}

int32 UMGAchievementSubsystem::GetCustomStat(FName StatID) const
{
	const int32* Value = CustomStats.Find(StatID);
	return Value ? *Value : 0;
}

// ==========================================
// ACHIEVEMENT PROGRESS
// ==========================================

FMGAchievementProgress UMGAchievementSubsystem::GetAchievementProgress(FName AchievementID) const
{
	const FMGAchievementProgress* Progress = AchievementProgress.Find(AchievementID);
	return Progress ? *Progress : FMGAchievementProgress();
}

float UMGAchievementSubsystem::GetAchievementProgressPercent(FName AchievementID) const
{
	const FMGAchievementDefinition* Definition = RegisteredAchievements.Find(AchievementID);
	const FMGAchievementProgress* Progress = AchievementProgress.Find(AchievementID);

	if (!Definition || !Progress || Definition->TargetValue <= 0)
	{
		return 0.0f;
	}

	if (Progress->bUnlocked)
	{
		return 1.0f;
	}

	return FMath::Clamp(static_cast<float>(Progress->CurrentValue) / Definition->TargetValue, 0.0f, 1.0f);
}

bool UMGAchievementSubsystem::IsAchievementUnlocked(FName AchievementID) const
{
	const FMGAchievementProgress* Progress = AchievementProgress.Find(AchievementID);
	return Progress && Progress->bUnlocked;
}

int32 UMGAchievementSubsystem::GetCurrentTier(FName AchievementID) const
{
	const FMGAchievementProgress* Progress = AchievementProgress.Find(AchievementID);
	return Progress ? Progress->CurrentTier : 0;
}

void UMGAchievementSubsystem::ForceUnlockAchievement(FName AchievementID)
{
	const FMGAchievementDefinition* Definition = RegisteredAchievements.Find(AchievementID);
	if (!Definition)
	{
		return;
	}

	FMGAchievementProgress& Progress = AchievementProgress.FindOrAdd(AchievementID);
	Progress.AchievementID = AchievementID;
	Progress.CurrentValue = Definition->TargetValue;
	Progress.bUnlocked = true;
	Progress.UnlockTime = FDateTime::Now();

	NotifyAchievementUnlocked(AchievementID);
}

void UMGAchievementSubsystem::ResetAchievementProgress(FName AchievementID)
{
	FMGAchievementProgress& Progress = AchievementProgress.FindOrAdd(AchievementID);
	Progress.CurrentValue = 0;
	Progress.CurrentTier = 0;
	Progress.bUnlocked = false;
	Progress.bRewardsClaimed = false;
}

void UMGAchievementSubsystem::ResetAllProgress()
{
	for (auto& Pair : AchievementProgress)
	{
		Pair.Value.CurrentValue = 0;
		Pair.Value.CurrentTier = 0;
		Pair.Value.bUnlocked = false;
		Pair.Value.bRewardsClaimed = false;
	}

	// Reset stats
	for (auto& Pair : Stats)
	{
		Pair.Value = 0;
	}
	CustomStats.Empty();

	RecentUnlocks.Empty();
}

// ==========================================
// REWARDS
// ==========================================

bool UMGAchievementSubsystem::ClaimRewards(FName AchievementID)
{
	FMGAchievementProgress* Progress = AchievementProgress.Find(AchievementID);
	if (!Progress || !Progress->bUnlocked || Progress->bRewardsClaimed)
	{
		return false;
	}

	Progress->bRewardsClaimed = true;

	// Actual reward granting would be done by the caller (Save subsystem, etc.)

	OnRewardsClaimed.Broadcast(AchievementID);
	return true;
}

bool UMGAchievementSubsystem::AreRewardsClaimed(FName AchievementID) const
{
	const FMGAchievementProgress* Progress = AchievementProgress.Find(AchievementID);
	return Progress && Progress->bRewardsClaimed;
}

TArray<FName> UMGAchievementSubsystem::GetUnclaimedRewards() const
{
	TArray<FName> Result;

	for (const auto& Pair : AchievementProgress)
	{
		if (Pair.Value.bUnlocked && !Pair.Value.bRewardsClaimed)
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

void UMGAchievementSubsystem::ClaimAllRewards()
{
	for (auto& Pair : AchievementProgress)
	{
		if (Pair.Value.bUnlocked && !Pair.Value.bRewardsClaimed)
		{
			ClaimRewards(Pair.Key);
		}
	}
}

// ==========================================
// QUERIES
// ==========================================

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetAchievementsByCategory(EMGAchievementCategory Category) const
{
	TArray<FMGAchievementDefinition> Result;

	for (const auto& Pair : RegisteredAchievements)
	{
		if (Pair.Value.Category == Category)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetUnlockedAchievements() const
{
	TArray<FMGAchievementDefinition> Result;

	for (const auto& Pair : AchievementProgress)
	{
		if (Pair.Value.bUnlocked)
		{
			const FMGAchievementDefinition* Definition = RegisteredAchievements.Find(Pair.Key);
			if (Definition)
			{
				Result.Add(*Definition);
			}
		}
	}

	return Result;
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetLockedAchievements() const
{
	TArray<FMGAchievementDefinition> Result;

	for (const auto& Pair : RegisteredAchievements)
	{
		const FMGAchievementProgress* Progress = AchievementProgress.Find(Pair.Key);
		if (!Progress || !Progress->bUnlocked)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGAchievementDefinition> UMGAchievementSubsystem::GetInProgressAchievements() const
{
	TArray<FMGAchievementDefinition> Result;

	for (const auto& Pair : AchievementProgress)
	{
		if (!Pair.Value.bUnlocked && Pair.Value.CurrentValue > 0)
		{
			const FMGAchievementDefinition* Definition = RegisteredAchievements.Find(Pair.Key);
			if (Definition)
			{
				Result.Add(*Definition);
			}
		}
	}

	return Result;
}

int32 UMGAchievementSubsystem::GetTotalAchievementCount() const
{
	return RegisteredAchievements.Num();
}

int32 UMGAchievementSubsystem::GetUnlockedAchievementCount() const
{
	int32 Count = 0;
	for (const auto& Pair : AchievementProgress)
	{
		if (Pair.Value.bUnlocked)
		{
			Count++;
		}
	}
	return Count;
}

float UMGAchievementSubsystem::GetOverallCompletionPercent() const
{
	int32 Total = RegisteredAchievements.Num();
	if (Total == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(GetUnlockedAchievementCount()) / Total;
}

int32 UMGAchievementSubsystem::GetTotalPointsEarned() const
{
	int32 TotalPoints = 0;

	for (const auto& Pair : AchievementProgress)
	{
		if (Pair.Value.bUnlocked)
		{
			const FMGAchievementDefinition* Definition = RegisteredAchievements.Find(Pair.Key);
			if (Definition)
			{
				TotalPoints += GetPointsForRarity(Definition->Rarity);
			}
		}
	}

	return TotalPoints;
}

// ==========================================
// RECENT ACHIEVEMENTS
// ==========================================

TArray<FMGAchievementNotification> UMGAchievementSubsystem::GetRecentUnlocks(int32 Count) const
{
	TArray<FMGAchievementNotification> Result;

	int32 NumToReturn = FMath::Min(Count, RecentUnlocks.Num());
	for (int32 i = 0; i < NumToReturn; ++i)
	{
		Result.Add(RecentUnlocks[i]);
	}

	return Result;
}

void UMGAchievementSubsystem::ClearRecentUnlocks()
{
	RecentUnlocks.Empty();
}

// ==========================================
// SAVE/LOAD
// ==========================================

TArray<FMGAchievementProgress> UMGAchievementSubsystem::GetAllProgress() const
{
	TArray<FMGAchievementProgress> Result;
	AchievementProgress.GenerateValueArray(Result);
	return Result;
}

void UMGAchievementSubsystem::LoadProgress(const TArray<FMGAchievementProgress>& ProgressData)
{
	for (const FMGAchievementProgress& Progress : ProgressData)
	{
		AchievementProgress.Add(Progress.AchievementID, Progress);
	}
}

void UMGAchievementSubsystem::LoadStats(const TMap<EMGAchievementStatType, int32>& StatsData)
{
	for (const auto& Pair : StatsData)
	{
		Stats.Add(Pair.Key, Pair.Value);
	}
}

// ==========================================
// INTERNAL FUNCTIONS
// ==========================================

void UMGAchievementSubsystem::CheckAchievementsForStat(EMGAchievementStatType StatType)
{
	TArray<FName> AchievementIDs;
	StatToAchievementMap.MultiFind(StatType, AchievementIDs);

	for (FName AchievementID : AchievementIDs)
	{
		TryUnlockAchievement(AchievementID);
	}
}

void UMGAchievementSubsystem::CheckAchievementForCustomStat(FName StatID)
{
	TArray<FName> AchievementIDs;
	CustomStatToAchievementMap.MultiFind(StatID, AchievementIDs);

	for (FName AchievementID : AchievementIDs)
	{
		TryUnlockAchievement(AchievementID);
	}
}

void UMGAchievementSubsystem::TryUnlockAchievement(FName AchievementID)
{
	const FMGAchievementDefinition* Definition = RegisteredAchievements.Find(AchievementID);
	if (!Definition)
	{
		return;
	}

	FMGAchievementProgress& Progress = AchievementProgress.FindOrAdd(AchievementID);
	Progress.AchievementID = AchievementID;

	// Already fully unlocked
	if (Progress.bUnlocked && !Definition->bHasTiers)
	{
		return;
	}

	// Check prerequisites
	if (!CheckPrerequisites(*Definition))
	{
		return;
	}

	// Get current stat value
	int32 StatValue = 0;
	if (Definition->StatType != EMGAchievementStatType::Custom)
	{
		StatValue = GetStat(Definition->StatType);
	}
	else
	{
		StatValue = GetCustomStat(Definition->CustomStatID);
	}

	Progress.CurrentValue = StatValue;

	// Broadcast progress update
	float ProgressPercent = static_cast<float>(StatValue) / FMath::Max(1, Definition->TargetValue);
	OnAchievementProgress.Broadcast(AchievementID, ProgressPercent);

	// Handle tiered achievements
	if (Definition->bHasTiers && Definition->TierThresholds.Num() > 0)
	{
		int32 NewTier = 0;
		for (int32 i = 0; i < Definition->TierThresholds.Num(); ++i)
		{
			if (StatValue >= Definition->TierThresholds[i])
			{
				NewTier = i + 1;
			}
		}

		if (NewTier > Progress.CurrentTier)
		{
			Progress.CurrentTier = NewTier;
			OnTierUnlocked.Broadcast(AchievementID, NewTier);
			NotifyAchievementUnlocked(AchievementID, NewTier);

			// Fully unlocked if reached max tier
			if (NewTier >= Definition->TierThresholds.Num())
			{
				Progress.bUnlocked = true;
				Progress.UnlockTime = FDateTime::Now();
			}
		}
	}
	else
	{
		// Non-tiered achievement
		if (StatValue >= Definition->TargetValue && !Progress.bUnlocked)
		{
			Progress.bUnlocked = true;
			Progress.UnlockTime = FDateTime::Now();
			NotifyAchievementUnlocked(AchievementID);
		}
	}
}

bool UMGAchievementSubsystem::CheckPrerequisites(const FMGAchievementDefinition& Definition) const
{
	// Check required achievements
	for (FName RequiredID : Definition.RequiredAchievements)
	{
		if (!IsAchievementUnlocked(RequiredID))
		{
			return false;
		}
	}

	// Could also check level requirement here
	// if (PlayerLevel < Definition.RequiredLevel) return false;

	return true;
}

void UMGAchievementSubsystem::NotifyAchievementUnlocked(FName AchievementID, int32 Tier)
{
	const FMGAchievementDefinition* Definition = RegisteredAchievements.Find(AchievementID);
	if (!Definition)
	{
		return;
	}

	FMGAchievementNotification Notification;
	Notification.Achievement = *Definition;
	Notification.TierUnlocked = Tier;
	Notification.bIsNewUnlock = true;
	Notification.UnlockTime = FDateTime::Now();

	// Add to recent unlocks
	RecentUnlocks.Insert(Notification, 0);
	if (RecentUnlocks.Num() > MaxRecentUnlocks)
	{
		RecentUnlocks.SetNum(MaxRecentUnlocks);
	}

	// Broadcast event
	OnAchievementUnlocked.Broadcast(Notification);
}

int32 UMGAchievementSubsystem::GetPointsForRarity(EMGAchievementRarity Rarity) const
{
	switch (Rarity)
	{
		case EMGAchievementRarity::Common: return 10;
		case EMGAchievementRarity::Uncommon: return 25;
		case EMGAchievementRarity::Rare: return 50;
		case EMGAchievementRarity::Epic: return 100;
		case EMGAchievementRarity::Legendary: return 200;
		default: return 10;
	}
}

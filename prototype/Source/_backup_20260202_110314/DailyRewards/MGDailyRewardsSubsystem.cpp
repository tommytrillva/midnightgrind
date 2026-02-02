// Copyright Midnight Grind. All Rights Reserved.

#include "DailyRewards/MGDailyRewardsSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/Paths.h"

void UMGDailyRewardsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ServerTime = FDateTime::UtcNow();
	ResetHourUTC = 0;

	InitializeDefaultCalendar();
	LoadLoginData();

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGDailyRewardsSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			ResetCheckHandle,
			[WeakThis]() { if (WeakThis.IsValid() && WeakThis->IsNewDay()) WeakThis->ProcessLogin(); },
			60.0f,
			true
		);
	}
}

void UMGDailyRewardsSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResetCheckHandle);
	}

	SaveLoginData();
	Super::Deinitialize();
}

bool UMGDailyRewardsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGDailyRewardsSubsystem::ProcessLogin()
{
	FDateTime Now = FDateTime::UtcNow();

	PlayerLoginData.TotalLogins++;

	if (PlayerLoginData.LastLoginDate.GetTicks() == 0)
	{
		PlayerLoginData.CurrentStreak = 1;
		PlayerLoginData.LongestStreak = 1;
		PlayerLoginData.CurrentCalendarDay = 1;
	}
	else
	{
		UpdateStreak();
	}

	PlayerLoginData.LastLoginDate = Now;
	PlayerLoginData.bHasClaimedToday = false;

	CheckAndResetCalendar();

	for (const FMGSpecialLoginEvent& Event : SpecialEvents)
	{
		if (Now >= Event.StartDate && Now <= Event.EndDate)
		{
			OnSpecialEventStarted.Broadcast(Event);
		}
	}

	SaveLoginData();
}

bool UMGDailyRewardsSubsystem::CanClaimDailyReward() const
{
	return !PlayerLoginData.bHasClaimedToday;
}

FMGRewardClaimResult UMGDailyRewardsSubsystem::ClaimDailyReward()
{
	FMGRewardClaimResult Result;

	if (PlayerLoginData.bHasClaimedToday)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FText::FromString(TEXT("Daily reward already claimed"));
		return Result;
	}

	int32 CurrentDay = PlayerLoginData.CurrentCalendarDay;
	if (CurrentDay < 1) CurrentDay = 1;
	if (CurrentDay > ActiveCalendar.CycleDays) CurrentDay = ((CurrentDay - 1) % ActiveCalendar.CycleDays) + 1;

	Result.ClaimedReward = GetRewardForDay(CurrentDay);

	float Multiplier = GetCurrentRewardMultiplier();
	if (Multiplier > 1.0f)
	{
		Result.ClaimedReward.Quantity = FMath::RoundToInt(Result.ClaimedReward.Quantity * Multiplier);
	}

	ApplyReward(Result.ClaimedReward);

	PlayerLoginData.ClaimedDays.Add(CurrentDay);
	PlayerLoginData.bHasClaimedToday = true;
	PlayerLoginData.CurrentCalendarDay++;
	PlayerLoginData.LastClaimDate = FDateTime::UtcNow();

	Result.NewStreak = PlayerLoginData.CurrentStreak;
	Result.bSuccess = true;

	for (const FMGStreakBonus& Bonus : ActiveCalendar.StreakBonuses)
	{
		if (PlayerLoginData.CurrentStreak >= Bonus.StreakDays &&
			!PlayerLoginData.ClaimedMilestones.Contains(Bonus.Milestone))
		{
			Result.bMilestoneReached = true;
			Result.MilestoneReached = Bonus.Milestone;
			Result.BonusRewards = Bonus.BonusRewards;

			for (const FMGDailyReward& BonusReward : Bonus.BonusRewards)
			{
				ApplyReward(BonusReward);
			}

			PlayerLoginData.ClaimedMilestones.Add(Bonus.Milestone);
			OnMilestoneReached.Broadcast(Bonus.Milestone, Bonus.BonusRewards);
			break;
		}
	}

	SaveLoginData();
	OnDailyRewardClaimed.Broadcast(Result);

	return Result;
}

void UMGDailyRewardsSubsystem::SetActiveCalendar(FName CalendarID)
{
	if (FMGLoginCalendar* Calendar = Calendars.Find(CalendarID))
	{
		ActiveCalendar = *Calendar;
	}
}

void UMGDailyRewardsSubsystem::RegisterCalendar(const FMGLoginCalendar& Calendar)
{
	Calendars.Add(Calendar.CalendarID, Calendar);
}

TArray<FMGLoginCalendar> UMGDailyRewardsSubsystem::GetAllCalendars() const
{
	TArray<FMGLoginCalendar> Result;
	Calendars.GenerateValueArray(Result);
	return Result;
}

FMGDailyReward UMGDailyRewardsSubsystem::GetRewardForDay(int32 Day) const
{
	int32 AdjustedDay = ((Day - 1) % ActiveCalendar.CycleDays) + 1;

	for (const FMGDailyReward& Reward : ActiveCalendar.DailyRewards)
	{
		if (Reward.DayNumber == AdjustedDay)
		{
			return Reward;
		}
	}

	return GenerateRewardForDay(AdjustedDay);
}

TArray<FMGDailyReward> UMGDailyRewardsSubsystem::GetUpcomingRewards(int32 DaysAhead) const
{
	TArray<FMGDailyReward> Rewards;

	int32 StartDay = PlayerLoginData.CurrentCalendarDay;
	for (int32 i = 0; i < DaysAhead; ++i)
	{
		Rewards.Add(GetRewardForDay(StartDay + i));
	}

	return Rewards;
}

int32 UMGDailyRewardsSubsystem::GetDaysUntilNextMilestone() const
{
	for (const FMGStreakBonus& Bonus : ActiveCalendar.StreakBonuses)
	{
		if (!PlayerLoginData.ClaimedMilestones.Contains(Bonus.Milestone))
		{
			return FMath::Max(0, Bonus.StreakDays - PlayerLoginData.CurrentStreak);
		}
	}
	return 0;
}

FMGStreakBonus UMGDailyRewardsSubsystem::GetCurrentStreakBonus() const
{
	FMGStreakBonus CurrentBonus;

	for (const FMGStreakBonus& Bonus : ActiveCalendar.StreakBonuses)
	{
		if (PlayerLoginData.CurrentStreak >= Bonus.StreakDays)
		{
			CurrentBonus = Bonus;
		}
	}

	return CurrentBonus;
}

FMGStreakBonus UMGDailyRewardsSubsystem::GetNextStreakMilestone() const
{
	for (const FMGStreakBonus& Bonus : ActiveCalendar.StreakBonuses)
	{
		if (PlayerLoginData.CurrentStreak < Bonus.StreakDays)
		{
			return Bonus;
		}
	}

	return FMGStreakBonus();
}

bool UMGDailyRewardsSubsystem::HasClaimedMilestone(EMGStreakMilestone Milestone) const
{
	return PlayerLoginData.ClaimedMilestones.Contains(Milestone);
}

void UMGDailyRewardsSubsystem::RecoverStreak(int32 RecoveryDays)
{
	int32 MissedDays = PlayerLoginData.MissedDays;
	int32 DaysToRecover = FMath::Min(RecoveryDays, MissedDays);

	PlayerLoginData.CurrentStreak += DaysToRecover;
	PlayerLoginData.MissedDays -= DaysToRecover;

	if (PlayerLoginData.CurrentStreak > PlayerLoginData.LongestStreak)
	{
		PlayerLoginData.LongestStreak = PlayerLoginData.CurrentStreak;
	}

	OnStreakUpdated.Broadcast(PlayerLoginData.CurrentStreak, false);
	SaveLoginData();
}

int32 UMGDailyRewardsSubsystem::GetStreakRecoveryCost(int32 MissedDays) const
{
	return MissedDays * 100;
}

void UMGDailyRewardsSubsystem::RegisterSpecialEvent(const FMGSpecialLoginEvent& Event)
{
	SpecialEvents.Add(Event);
}

TArray<FMGSpecialLoginEvent> UMGDailyRewardsSubsystem::GetActiveSpecialEvents() const
{
	TArray<FMGSpecialLoginEvent> ActiveEvents;
	FDateTime Now = FDateTime::UtcNow();

	for (const FMGSpecialLoginEvent& Event : SpecialEvents)
	{
		if (Now >= Event.StartDate && Now <= Event.EndDate)
		{
			ActiveEvents.Add(Event);
		}
	}

	return ActiveEvents;
}

bool UMGDailyRewardsSubsystem::IsSpecialEventActive() const
{
	return GetActiveSpecialEvents().Num() > 0;
}

float UMGDailyRewardsSubsystem::GetCurrentRewardMultiplier() const
{
	float Multiplier = 1.0f;

	TArray<FMGSpecialLoginEvent> ActiveEvents = GetActiveSpecialEvents();
	for (const FMGSpecialLoginEvent& Event : ActiveEvents)
	{
		Multiplier = FMath::Max(Multiplier, Event.RewardMultiplier);
	}

	FMGStreakBonus CurrentBonus = GetCurrentStreakBonus();
	Multiplier *= CurrentBonus.RewardMultiplier;

	return Multiplier;
}

FTimespan UMGDailyRewardsSubsystem::GetTimeUntilReset() const
{
	FDateTime NextReset = GetNextResetTime();
	FDateTime Now = FDateTime::UtcNow();

	return NextReset - Now;
}

FDateTime UMGDailyRewardsSubsystem::GetNextResetTime() const
{
	FDateTime Now = FDateTime::UtcNow();
	FDateTime Today = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), ResetHourUTC, 0, 0);

	if (Now >= Today)
	{
		return Today + FTimespan::FromDays(1);
	}

	return Today;
}

bool UMGDailyRewardsSubsystem::IsNewDay() const
{
	if (PlayerLoginData.LastLoginDate.GetTicks() == 0)
	{
		return true;
	}

	return !IsSameDay(PlayerLoginData.LastLoginDate, FDateTime::UtcNow());
}

void UMGDailyRewardsSubsystem::SaveLoginData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("DailyRewards");
	IFileManager::Get().MakeDirectory(*SaveDir, true);
	FString FilePath = SaveDir / TEXT("LoginData.sav");

	FBufferArchive Archive;

	int32 Version = 1;
	Archive << Version;

	// Save login timestamps
	int64 LastLoginUnix = PlayerLoginData.LastLoginDate.ToUnixTimestamp();
	int64 LastClaimUnix = PlayerLoginData.LastClaimDate.ToUnixTimestamp();
	Archive << LastLoginUnix;
	Archive << LastClaimUnix;

	// Save streak data
	Archive << PlayerLoginData.CurrentStreak;
	Archive << PlayerLoginData.LongestStreak;
	Archive << PlayerLoginData.TotalLogins;
	Archive << PlayerLoginData.CurrentCalendarDay;
	Archive << PlayerLoginData.MissedDays;
	Archive << PlayerLoginData.bHasClaimedToday;

	// Save claimed days
	int32 ClaimedDayCount = PlayerLoginData.ClaimedDays.Num();
	Archive << ClaimedDayCount;
	for (int32 Day : PlayerLoginData.ClaimedDays)
	{
		Archive << Day;
	}

	// Save claimed milestones
	int32 MilestoneCount = PlayerLoginData.ClaimedMilestones.Num();
	Archive << MilestoneCount;
	for (EMGStreakMilestone Milestone : PlayerLoginData.ClaimedMilestones)
	{
		int32 MilestoneInt = static_cast<int32>(Milestone);
		Archive << MilestoneInt;
	}

	// Save active calendar ID
	FName CalendarID = ActiveCalendar.CalendarID;
	Archive << CalendarID;

	if (FFileHelper::SaveArrayToFile(Archive, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Daily rewards data saved - Streak: %d, Logins: %d"),
			PlayerLoginData.CurrentStreak, PlayerLoginData.TotalLogins);
	}
}

void UMGDailyRewardsSubsystem::LoadLoginData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("DailyRewards");
	FString FilePath = SaveDir / TEXT("LoginData.sav");

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		// No save found, start fresh
		PlayerLoginData = FMGPlayerLoginData();
		return;
	}

	FMemoryReader Archive(FileData, true);

	int32 Version = 0;
	Archive << Version;

	if (Version >= 1)
	{
		// Load login timestamps
		int64 LastLoginUnix;
		int64 LastClaimUnix;
		Archive << LastLoginUnix;
		Archive << LastClaimUnix;
		PlayerLoginData.LastLoginDate = FDateTime::FromUnixTimestamp(LastLoginUnix);
		PlayerLoginData.LastClaimDate = FDateTime::FromUnixTimestamp(LastClaimUnix);

		// Load streak data
		Archive << PlayerLoginData.CurrentStreak;
		Archive << PlayerLoginData.LongestStreak;
		Archive << PlayerLoginData.TotalLogins;
		Archive << PlayerLoginData.CurrentCalendarDay;
		Archive << PlayerLoginData.MissedDays;
		Archive << PlayerLoginData.bHasClaimedToday;

		// Load claimed days
		int32 ClaimedDayCount;
		Archive << ClaimedDayCount;
		PlayerLoginData.ClaimedDays.Empty();
		for (int32 i = 0; i < ClaimedDayCount; i++)
		{
			int32 Day;
			Archive << Day;
			PlayerLoginData.ClaimedDays.Add(Day);
		}

		// Load claimed milestones
		int32 MilestoneCount;
		Archive << MilestoneCount;
		PlayerLoginData.ClaimedMilestones.Empty();
		for (int32 i = 0; i < MilestoneCount; i++)
		{
			int32 MilestoneInt;
			Archive << MilestoneInt;
			PlayerLoginData.ClaimedMilestones.Add(static_cast<EMGStreakMilestone>(MilestoneInt));
		}

		// Load active calendar ID and set it
		FName CalendarID;
		Archive << CalendarID;
		if (!CalendarID.IsNone() && Calendars.Contains(CalendarID))
		{
			ActiveCalendar = Calendars[CalendarID];
		}

		UE_LOG(LogTemp, Log, TEXT("Daily rewards data loaded - Streak: %d, Longest: %d, Logins: %d"),
			PlayerLoginData.CurrentStreak, PlayerLoginData.LongestStreak, PlayerLoginData.TotalLogins);
	}
}

void UMGDailyRewardsSubsystem::UpdateStreak()
{
	FDateTime Now = FDateTime::UtcNow();
	FDateTime LastLogin = PlayerLoginData.LastLoginDate;

	bool bPreventStreakLoss = false;
	for (const FMGSpecialLoginEvent& Event : GetActiveSpecialEvents())
	{
		if (Event.bPreventStreakLoss)
		{
			bPreventStreakLoss = true;
			break;
		}
	}

	if (IsConsecutiveDay(LastLogin, Now))
	{
		PlayerLoginData.CurrentStreak++;

		for (const FMGSpecialLoginEvent& Event : GetActiveSpecialEvents())
		{
			if (Event.bDoubleStreak)
			{
				PlayerLoginData.CurrentStreak++;
				break;
			}
		}

		if (PlayerLoginData.CurrentStreak > PlayerLoginData.LongestStreak)
		{
			PlayerLoginData.LongestStreak = PlayerLoginData.CurrentStreak;
		}

		OnStreakUpdated.Broadcast(PlayerLoginData.CurrentStreak, false);
	}
	else if (IsSameDay(LastLogin, Now))
	{
		// Same day login, streak unchanged
	}
	else
	{
		if (!bPreventStreakLoss)
		{
			FTimespan TimeMissed = Now - LastLogin;
			PlayerLoginData.MissedDays = FMath::FloorToInt(TimeMissed.GetTotalDays()) - 1;
			PlayerLoginData.CurrentStreak = 1;
			OnStreakUpdated.Broadcast(1, true);
		}
	}
}

void UMGDailyRewardsSubsystem::CheckMilestones()
{
	for (const FMGStreakBonus& Bonus : ActiveCalendar.StreakBonuses)
	{
		if (PlayerLoginData.CurrentStreak >= Bonus.StreakDays &&
			!PlayerLoginData.ClaimedMilestones.Contains(Bonus.Milestone))
		{
			OnMilestoneReached.Broadcast(Bonus.Milestone, Bonus.BonusRewards);
		}
	}
}

void UMGDailyRewardsSubsystem::ApplyReward(const FMGDailyReward& Reward)
{
	switch (Reward.RewardType)
	{
	case EMGRewardType::Currency:
		// Add currency through economy subsystem
		break;
	case EMGRewardType::PremiumCurrency:
		// Add premium currency
		break;
	case EMGRewardType::Vehicle:
		// Unlock vehicle
		break;
	case EMGRewardType::Part:
		// Add part to inventory
		break;
	case EMGRewardType::Cosmetic:
		// Add cosmetic item
		break;
	case EMGRewardType::ExperienceBoost:
		// Apply XP boost
		break;
	case EMGRewardType::CurrencyBoost:
		// Apply currency boost
		break;
	case EMGRewardType::LootBox:
		// Add loot box to inventory
		break;
	case EMGRewardType::CustomizationItem:
		// Add customization item
		break;
	case EMGRewardType::Decal:
		// Add decal
		break;
	case EMGRewardType::WheelSet:
		// Add wheel set
		break;
	case EMGRewardType::NeonKit:
		// Add neon kit
		break;
	}
}

void UMGDailyRewardsSubsystem::InitializeDefaultCalendar()
{
	FMGLoginCalendar DefaultCalendar;
	DefaultCalendar.CalendarID = FName(TEXT("Default"));
	DefaultCalendar.CalendarName = FText::FromString(TEXT("Daily Login Rewards"));
	DefaultCalendar.CycleDays = 28;
	DefaultCalendar.bResetOnCycleComplete = true;
	DefaultCalendar.bIsActive = true;

	for (int32 Day = 1; Day <= 28; ++Day)
	{
		DefaultCalendar.DailyRewards.Add(GenerateRewardForDay(Day));
	}

	// Week 1 milestone
	FMGStreakBonus Week1;
	Week1.StreakDays = 7;
	Week1.Milestone = EMGStreakMilestone::Week1;
	Week1.RewardMultiplier = 1.5f;
	Week1.MilestoneTitle = FText::FromString(TEXT("Week 1 Complete!"));

	FMGDailyReward Week1Bonus;
	Week1Bonus.RewardType = EMGRewardType::Currency;
	Week1Bonus.Quantity = 5000;
	Week1Bonus.Rarity = EMGRewardRarity::Rare;
	Week1.BonusRewards.Add(Week1Bonus);
	DefaultCalendar.StreakBonuses.Add(Week1);

	// Week 2 milestone
	FMGStreakBonus Week2;
	Week2.StreakDays = 14;
	Week2.Milestone = EMGStreakMilestone::Week2;
	Week2.RewardMultiplier = 1.75f;
	Week2.MilestoneTitle = FText::FromString(TEXT("2 Weeks Strong!"));

	FMGDailyReward Week2Bonus;
	Week2Bonus.RewardType = EMGRewardType::LootBox;
	Week2Bonus.Quantity = 1;
	Week2Bonus.Rarity = EMGRewardRarity::Epic;
	Week2.BonusRewards.Add(Week2Bonus);
	DefaultCalendar.StreakBonuses.Add(Week2);

	// Week 3 milestone
	FMGStreakBonus Week3;
	Week3.StreakDays = 21;
	Week3.Milestone = EMGStreakMilestone::Week3;
	Week3.RewardMultiplier = 2.0f;
	Week3.MilestoneTitle = FText::FromString(TEXT("3 Week Champion!"));

	FMGDailyReward Week3Bonus;
	Week3Bonus.RewardType = EMGRewardType::NeonKit;
	Week3Bonus.Quantity = 1;
	Week3Bonus.Rarity = EMGRewardRarity::Epic;
	Week3.BonusRewards.Add(Week3Bonus);
	DefaultCalendar.StreakBonuses.Add(Week3);

	// Month 1 milestone
	FMGStreakBonus Month1;
	Month1.StreakDays = 28;
	Month1.Milestone = EMGStreakMilestone::Month1;
	Month1.RewardMultiplier = 2.5f;
	Month1.MilestoneTitle = FText::FromString(TEXT("Month of Dedication!"));

	FMGDailyReward Month1Bonus;
	Month1Bonus.RewardType = EMGRewardType::Vehicle;
	Month1Bonus.Quantity = 1;
	Month1Bonus.Rarity = EMGRewardRarity::Legendary;
	Month1.BonusRewards.Add(Month1Bonus);
	DefaultCalendar.StreakBonuses.Add(Month1);

	Calendars.Add(DefaultCalendar.CalendarID, DefaultCalendar);
	ActiveCalendar = DefaultCalendar;
}

FMGDailyReward UMGDailyRewardsSubsystem::GenerateRewardForDay(int32 Day) const
{
	FMGDailyReward Reward;
	Reward.DayNumber = Day;

	// Day 7, 14, 21, 28 are milestone days with better rewards
	if (Day % 7 == 0)
	{
		Reward.RewardType = EMGRewardType::LootBox;
		Reward.Quantity = 1;
		Reward.Rarity = (Day == 28) ? EMGRewardRarity::Legendary : EMGRewardRarity::Rare;
		Reward.bIsBonusReward = true;
		Reward.DisplayName = FText::FromString(FString::Printf(TEXT("Day %d Loot Box"), Day));
	}
	else if (Day % 5 == 0)
	{
		Reward.RewardType = EMGRewardType::Part;
		Reward.Quantity = 1;
		Reward.Rarity = EMGRewardRarity::Uncommon;
		Reward.DisplayName = FText::FromString(FString::Printf(TEXT("Day %d Part"), Day));
	}
	else if (Day % 3 == 0)
	{
		Reward.RewardType = EMGRewardType::PremiumCurrency;
		Reward.Quantity = 50 + (Day * 5);
		Reward.Rarity = EMGRewardRarity::Uncommon;
		Reward.DisplayName = FText::FromString(TEXT("Premium Credits"));
	}
	else
	{
		Reward.RewardType = EMGRewardType::Currency;
		Reward.Quantity = 500 + (Day * 100);
		Reward.Rarity = EMGRewardRarity::Common;
		Reward.DisplayName = FText::FromString(TEXT("Credits"));
	}

	return Reward;
}

bool UMGDailyRewardsSubsystem::IsSameDay(const FDateTime& Date1, const FDateTime& Date2) const
{
	return Date1.GetYear() == Date2.GetYear() &&
		   Date1.GetMonth() == Date2.GetMonth() &&
		   Date1.GetDay() == Date2.GetDay();
}

bool UMGDailyRewardsSubsystem::IsConsecutiveDay(const FDateTime& LastDate, const FDateTime& CurrentDate) const
{
	FDateTime LastDay = FDateTime(LastDate.GetYear(), LastDate.GetMonth(), LastDate.GetDay(), 0, 0, 0);
	FDateTime CurrentDay = FDateTime(CurrentDate.GetYear(), CurrentDate.GetMonth(), CurrentDate.GetDay(), 0, 0, 0);

	FTimespan Diff = CurrentDay - LastDay;
	return Diff.GetTotalDays() >= 1.0 && Diff.GetTotalDays() < 2.0;
}

void UMGDailyRewardsSubsystem::CheckAndResetCalendar()
{
	if (!ActiveCalendar.bResetOnCycleComplete)
	{
		return;
	}

	if (PlayerLoginData.CurrentCalendarDay > ActiveCalendar.CycleDays)
	{
		PlayerLoginData.CurrentCalendarDay = 1;
		PlayerLoginData.ClaimedDays.Empty();
		PlayerLoginData.ClaimedMilestones.Empty();

		OnLoginCalendarReset.Broadcast(ActiveCalendar.CalendarID);
	}
}

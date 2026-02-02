// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGChallengeSubsystem.cpp
 * @brief Implementation of the Challenge Subsystem for daily/weekly/monthly challenges,
 *        progress tracking, and reward systems.
 */

#include "Challenges/MGChallengeSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGChallengeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MaxActiveChallenges = 10;
	DailyRerollsRemaining = 3;

	InitializeDefaultChallenges();
	LoadChallengeProgress();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ChallengeTickHandle,
			this,
			&UMGChallengeSubsystem::OnChallengeTick,
			1.0f,
			true
		);
	}
}

void UMGChallengeSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChallengeTickHandle);
	}

	SaveChallengeProgress();
	Super::Deinitialize();
}

bool UMGChallengeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetChallengesByType(EMGChallengeType Type) const
{
	TArray<FMGChallenge> Result;
	for (const auto& Pair : AllChallenges)
	{
		if (Pair.Value.ChallengeType == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetChallengesByCategory(EMGChallengeCategory Category) const
{
	TArray<FMGChallenge> Result;
	for (const auto& Pair : AllChallenges)
	{
		if (Pair.Value.Category == Category)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetActiveChallenges() const
{
	TArray<FMGChallenge> Result;
	for (const FName& ID : ActiveChallengeIDs)
	{
		if (const FMGChallenge* Challenge = AllChallenges.Find(ID))
		{
			Result.Add(*Challenge);
		}
	}
	return Result;
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetCompletedChallenges() const
{
	TArray<FMGChallenge> Result;
	for (const auto& Pair : AllChallenges)
	{
		if (Pair.Value.State == EMGChallengeState::Completed ||
			Pair.Value.State == EMGChallengeState::Claimed)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetAvailableChallenges() const
{
	TArray<FMGChallenge> Result;
	for (const auto& Pair : AllChallenges)
	{
		if (Pair.Value.State == EMGChallengeState::Available && ArePrerequisitesMet(Pair.Value))
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FMGChallenge UMGChallengeSubsystem::GetChallenge(FName ChallengeID) const
{
	if (const FMGChallenge* Challenge = AllChallenges.Find(ChallengeID))
	{
		return *Challenge;
	}
	return FMGChallenge();
}

bool UMGChallengeSubsystem::IsChallengeAvailable(FName ChallengeID) const
{
	if (const FMGChallenge* Challenge = AllChallenges.Find(ChallengeID))
	{
		return Challenge->State == EMGChallengeState::Available && ArePrerequisitesMet(*Challenge);
	}
	return false;
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetDailyChallenges() const
{
	return GetChallengesByType(EMGChallengeType::Daily);
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetWeeklyChallenges() const
{
	return GetChallengesByType(EMGChallengeType::Weekly);
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetMonthlyChallenges() const
{
	return GetChallengesByType(EMGChallengeType::Monthly);
}

FTimespan UMGChallengeSubsystem::GetTimeUntilDailyReset() const
{
	FDateTime Now = FDateTime::UtcNow();
	FDateTime NextReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0);
	NextReset += FTimespan::FromDays(1);
	return NextReset - Now;
}

FTimespan UMGChallengeSubsystem::GetTimeUntilWeeklyReset() const
{
	FDateTime Now = FDateTime::UtcNow();
	int32 DaysUntilMonday = (8 - static_cast<int32>(Now.GetDayOfWeek())) % 7;
	if (DaysUntilMonday == 0) DaysUntilMonday = 7;
	FDateTime NextReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0);
	NextReset += FTimespan::FromDays(DaysUntilMonday);
	return NextReset - Now;
}

void UMGChallengeSubsystem::UpdateChallengeProgress(FName ChallengeID, FName ObjectiveID, int32 Progress)
{
	FMGChallenge* Challenge = AllChallenges.Find(ChallengeID);
	if (!Challenge || Challenge->State != EMGChallengeState::Active)
	{
		return;
	}

	FMGChallengeProgress& ProgressData = ChallengeProgress.FindOrAdd(ChallengeID);
	ProgressData.ChallengeID = ChallengeID;
	ProgressData.ObjectiveProgress.Add(ObjectiveID, Progress);

	for (FMGChallengeObjective& Objective : Challenge->Objectives)
	{
		if (Objective.ObjectiveID == ObjectiveID)
		{
			Objective.CurrentValue = Progress;
			Objective.bCompleted = Objective.CurrentValue >= Objective.TargetValue;
			break;
		}
	}

	float ProgressPercent = GetChallengeProgressPercent(ChallengeID);
	OnChallengeProgressUpdated.Broadcast(ChallengeID, ProgressPercent);

	CheckForCompletedChallenges();
}

void UMGChallengeSubsystem::AddChallengeProgress(FName ChallengeID, FName ObjectiveID, int32 Amount)
{
	FMGChallenge* Challenge = AllChallenges.Find(ChallengeID);
	if (!Challenge)
	{
		return;
	}

	for (FMGChallengeObjective& Objective : Challenge->Objectives)
	{
		if (Objective.ObjectiveID == ObjectiveID)
		{
			UpdateChallengeProgress(ChallengeID, ObjectiveID, Objective.CurrentValue + Amount);
			break;
		}
	}
}

void UMGChallengeSubsystem::TrackStat(FName StatName, int32 Value)
{
	TrackedStats.Add(StatName, Value);

	// Update all challenges that track this stat
	for (auto& Pair : AllChallenges)
	{
		FMGChallenge& Challenge = Pair.Value;
		if (Challenge.State != EMGChallengeState::Active)
		{
			continue;
		}

		for (FMGChallengeObjective& Objective : Challenge.Objectives)
		{
			if (Objective.StatToTrack == StatName)
			{
				AddChallengeProgress(Challenge.ChallengeID, Objective.ObjectiveID, Value);
			}
		}
	}
}

float UMGChallengeSubsystem::GetChallengeProgressPercent(FName ChallengeID) const
{
	const FMGChallenge* Challenge = AllChallenges.Find(ChallengeID);
	if (!Challenge || Challenge->Objectives.Num() == 0)
	{
		return 0.0f;
	}

	float TotalProgress = 0.0f;
	int32 RequiredObjectives = 0;

	for (const FMGChallengeObjective& Objective : Challenge->Objectives)
	{
		if (!Objective.bIsOptional)
		{
			TotalProgress += FMath::Clamp(
				static_cast<float>(Objective.CurrentValue) / static_cast<float>(Objective.TargetValue),
				0.0f, 1.0f
			);
			RequiredObjectives++;
		}
	}

	return (RequiredObjectives > 0) ? (TotalProgress / RequiredObjectives) * 100.0f : 0.0f;
}

FMGChallengeProgress UMGChallengeSubsystem::GetChallengeProgress(FName ChallengeID) const
{
	if (const FMGChallengeProgress* Progress = ChallengeProgress.Find(ChallengeID))
	{
		return *Progress;
	}
	return FMGChallengeProgress();
}

bool UMGChallengeSubsystem::ActivateChallenge(FName ChallengeID)
{
	if (ActiveChallengeIDs.Num() >= MaxActiveChallenges)
	{
		return false;
	}

	FMGChallenge* Challenge = AllChallenges.Find(ChallengeID);
	if (!Challenge || !IsChallengeAvailable(ChallengeID))
	{
		return false;
	}

	Challenge->State = EMGChallengeState::Active;
	ActiveChallengeIDs.AddUnique(ChallengeID);

	FMGChallengeProgress& Progress = ChallengeProgress.FindOrAdd(ChallengeID);
	Progress.ChallengeID = ChallengeID;
	Progress.StartedAt = FDateTime::UtcNow();

	return true;
}

void UMGChallengeSubsystem::DeactivateChallenge(FName ChallengeID)
{
	FMGChallenge* Challenge = AllChallenges.Find(ChallengeID);
	if (Challenge && Challenge->State == EMGChallengeState::Active)
	{
		Challenge->State = EMGChallengeState::Available;
		ActiveChallengeIDs.Remove(ChallengeID);
	}
}

int32 UMGChallengeSubsystem::GetActiveChallengeCount() const
{
	return ActiveChallengeIDs.Num();
}

bool UMGChallengeSubsystem::ClaimChallengeRewards(FName ChallengeID)
{
	FMGChallenge* Challenge = AllChallenges.Find(ChallengeID);
	if (!Challenge || Challenge->State != EMGChallengeState::Completed)
	{
		return false;
	}

	FMGChallengeProgress* Progress = ChallengeProgress.Find(ChallengeID);
	if (Progress && Progress->bRewardsClaimed)
	{
		return false;
	}

	// Grant rewards
	for (const FMGChallengeReward& Reward : Challenge->Rewards)
	{
		GrantReward(Reward);
	}

	Challenge->State = EMGChallengeState::Claimed;
	Challenge->TimesCompleted++;

	if (Progress)
	{
		Progress->bRewardsClaimed = true;
		Progress->CompletedAt = FDateTime::UtcNow();
	}

	ActiveChallengeIDs.Remove(ChallengeID);
	OnChallengeRewardsClaimed.Broadcast(Challenge->Rewards);

	// Check for challenge set completion
	for (auto& SetPair : ChallengeSets)
	{
		FMGChallengeSet& Set = SetPair.Value;
		for (const FMGChallenge& SetChallenge : Set.Challenges)
		{
			if (SetChallenge.ChallengeID == ChallengeID)
			{
				Set.CurrentCompletions++;
				if (Set.CurrentCompletions >= Set.RequiredCompletions)
				{
					GrantReward(Set.CompletionReward);
					OnChallengeSetCompleted.Broadcast(Set);
				}
				break;
			}
		}
	}

	return true;
}

void UMGChallengeSubsystem::ClaimAllAvailableRewards()
{
	TArray<FName> CompletedIDs;
	for (const auto& Pair : AllChallenges)
	{
		if (Pair.Value.State == EMGChallengeState::Completed)
		{
			CompletedIDs.Add(Pair.Key);
		}
	}

	for (const FName& ID : CompletedIDs)
	{
		ClaimChallengeRewards(ID);
	}
}

TArray<FMGChallenge> UMGChallengeSubsystem::GetUnclaimedCompletedChallenges() const
{
	TArray<FMGChallenge> Result;
	for (const auto& Pair : AllChallenges)
	{
		if (Pair.Value.State == EMGChallengeState::Completed)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

bool UMGChallengeSubsystem::HasUnclaimedRewards() const
{
	return GetUnclaimedCompletedChallenges().Num() > 0;
}

TArray<FMGChallengeSet> UMGChallengeSubsystem::GetChallengeSets() const
{
	TArray<FMGChallengeSet> Result;
	ChallengeSets.GenerateValueArray(Result);
	return Result;
}

FMGChallengeSet UMGChallengeSubsystem::GetChallengeSet(FName SetID) const
{
	if (const FMGChallengeSet* Set = ChallengeSets.Find(SetID))
	{
		return *Set;
	}
	return FMGChallengeSet();
}

float UMGChallengeSubsystem::GetChallengeSetProgress(FName SetID) const
{
	if (const FMGChallengeSet* Set = ChallengeSets.Find(SetID))
	{
		if (Set->RequiredCompletions > 0)
		{
			return (static_cast<float>(Set->CurrentCompletions) / static_cast<float>(Set->RequiredCompletions)) * 100.0f;
		}
	}
	return 0.0f;
}

TArray<FMGCommunityChallenge> UMGChallengeSubsystem::GetCommunityChallenges() const
{
	return CommunityChallenges;
}

void UMGChallengeSubsystem::ContributeToCommunityChallenge(FName ChallengeID, int64 Amount)
{
	for (FMGCommunityChallenge& Community : CommunityChallenges)
	{
		if (Community.Challenge.ChallengeID == ChallengeID)
		{
			Community.PlayerContribution += Amount;
			Community.CommunityProgress += Amount;

			float ProgressPercent = (Community.CommunityTarget > 0) ?
				(static_cast<float>(Community.CommunityProgress) / static_cast<float>(Community.CommunityTarget)) * 100.0f : 0.0f;

			OnCommunityProgressUpdated.Broadcast(ChallengeID, ProgressPercent);
			break;
		}
	}
}

float UMGChallengeSubsystem::GetCommunityProgressPercent(FName ChallengeID) const
{
	for (const FMGCommunityChallenge& Community : CommunityChallenges)
	{
		if (Community.Challenge.ChallengeID == ChallengeID && Community.CommunityTarget > 0)
		{
			return (static_cast<float>(Community.CommunityProgress) / static_cast<float>(Community.CommunityTarget)) * 100.0f;
		}
	}
	return 0.0f;
}

void UMGChallengeSubsystem::RefreshChallenges()
{
	FDateTime Now = FDateTime::UtcNow();

	// Check for daily reset
	FDateTime TodayReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0);
	if (LastDailyReset < TodayReset)
	{
		GenerateDailyChallenges();
		LastDailyReset = TodayReset;
		DailyRerollsRemaining = 3;
		OnNewChallengesAvailable.Broadcast(EMGChallengeType::Daily);
	}

	// Check for weekly reset
	int32 DayOfWeek = static_cast<int32>(Now.GetDayOfWeek());
	FDateTime WeekStart = TodayReset - FTimespan::FromDays(DayOfWeek == 0 ? 6 : DayOfWeek - 1);
	if (LastWeeklyReset < WeekStart)
	{
		GenerateWeeklyChallenges();
		LastWeeklyReset = WeekStart;
		OnNewChallengesAvailable.Broadcast(EMGChallengeType::Weekly);
	}
}

void UMGChallengeSubsystem::RerollDailyChallenge(FName ChallengeID)
{
	if (DailyRerollsRemaining <= 0)
	{
		return;
	}

	FMGChallenge* Challenge = AllChallenges.Find(ChallengeID);
	if (!Challenge || Challenge->ChallengeType != EMGChallengeType::Daily)
	{
		return;
	}

	// Generate a new daily challenge to replace this one
	// For now, just reset it
	Challenge->State = EMGChallengeState::Available;
	for (FMGChallengeObjective& Obj : Challenge->Objectives)
	{
		Obj.CurrentValue = 0;
		Obj.bCompleted = false;
	}

	DailyRerollsRemaining--;
}

void UMGChallengeSubsystem::SaveChallengeProgress()
{
	// Save to player save game
}

void UMGChallengeSubsystem::LoadChallengeProgress()
{
	// Load from player save game
}

void UMGChallengeSubsystem::OnChallengeTick()
{
	CheckForExpiredChallenges();
	RefreshChallenges();
}

void UMGChallengeSubsystem::CheckForExpiredChallenges()
{
	FDateTime Now = FDateTime::UtcNow();

	for (auto& Pair : AllChallenges)
	{
		FMGChallenge& Challenge = Pair.Value;

		if (Challenge.State == EMGChallengeState::Active ||
			Challenge.State == EMGChallengeState::Available)
		{
			if (Challenge.EndTime.GetTicks() > 0 && Now > Challenge.EndTime)
			{
				Challenge.State = EMGChallengeState::Expired;
				ActiveChallengeIDs.Remove(Challenge.ChallengeID);
				OnChallengeExpired.Broadcast(Challenge.ChallengeID);
			}
		}
	}
}

void UMGChallengeSubsystem::CheckForCompletedChallenges()
{
	for (auto& Pair : AllChallenges)
	{
		FMGChallenge& Challenge = Pair.Value;

		if (Challenge.State != EMGChallengeState::Active)
		{
			continue;
		}

		bool bAllComplete = true;
		for (const FMGChallengeObjective& Obj : Challenge.Objectives)
		{
			if (!Obj.bIsOptional && !Obj.bCompleted)
			{
				bAllComplete = false;
				break;
			}
		}

		if (bAllComplete)
		{
			Challenge.State = EMGChallengeState::Completed;
			OnChallengeCompleted.Broadcast(Challenge);
		}
	}
}

void UMGChallengeSubsystem::GenerateDailyChallenges()
{
	// Clear old daily challenges
	TArray<FName> ToRemove;
	for (auto& Pair : AllChallenges)
	{
		if (Pair.Value.ChallengeType == EMGChallengeType::Daily &&
			Pair.Value.State != EMGChallengeState::Claimed)
		{
			ToRemove.Add(Pair.Key);
		}
	}
	for (const FName& ID : ToRemove)
	{
		AllChallenges.Remove(ID);
		ActiveChallengeIDs.Remove(ID);
	}

	// Generate new daily challenges
	FDateTime Now = FDateTime::UtcNow();
	FDateTime EndOfDay = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 23, 59, 59);

	auto CreateDaily = [this, &Now, &EndOfDay](const FString& ID, const FString& Title, const FString& Desc,
		EMGChallengeCategory Cat, const FString& ObjID, const FString& ObjDesc, int32 Target, int32 XP, int32 Cash)
	{
		FMGChallenge Challenge;
		Challenge.ChallengeID = FName(*ID);
		Challenge.Title = FText::FromString(Title);
		Challenge.Description = FText::FromString(Desc);
		Challenge.ChallengeType = EMGChallengeType::Daily;
		Challenge.Category = Cat;
		Challenge.State = EMGChallengeState::Available;
		Challenge.Difficulty = EMGChallengeDifficulty::Easy;
		Challenge.StartTime = Now;
		Challenge.EndTime = EndOfDay;

		FMGChallengeObjective Obj;
		Obj.ObjectiveID = FName(*ObjID);
		Obj.Description = FText::FromString(ObjDesc);
		Obj.TargetValue = Target;
		Challenge.Objectives.Add(Obj);

		FMGChallengeReward Reward;
		Reward.ExperienceAmount = XP;
		Reward.CurrencyAmount = Cash;
		Challenge.Rewards.Add(Reward);

		AllChallenges.Add(Challenge.ChallengeID, Challenge);
	};

	CreateDaily(TEXT("Daily_Race3"), TEXT("Daily Racer"), TEXT("Complete 3 races today"),
		EMGChallengeCategory::Racing, TEXT("RacesCompleted"), TEXT("Complete races"), 3, 500, 1000);

	CreateDaily(TEXT("Daily_Drift5000"), TEXT("Drift Master"), TEXT("Accumulate 5000 drift points"),
		EMGChallengeCategory::Drifting, TEXT("DriftPoints"), TEXT("Earn drift points"), 5000, 400, 800);

	CreateDaily(TEXT("Daily_Win1"), TEXT("Victory Lap"), TEXT("Win a race"),
		EMGChallengeCategory::Racing, TEXT("Wins"), TEXT("Win races"), 1, 600, 1500);
}

void UMGChallengeSubsystem::GenerateWeeklyChallenges()
{
	// Similar to daily but with longer duration and bigger rewards
	FDateTime Now = FDateTime::UtcNow();
	int32 DaysUntilSunday = (7 - static_cast<int32>(Now.GetDayOfWeek())) % 7;
	FDateTime EndOfWeek = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 23, 59, 59);
	EndOfWeek += FTimespan::FromDays(DaysUntilSunday);

	FMGChallenge WeeklyRacing;
	WeeklyRacing.ChallengeID = FName(TEXT("Weekly_Race20"));
	WeeklyRacing.Title = FText::FromString(TEXT("Weekly Warrior"));
	WeeklyRacing.Description = FText::FromString(TEXT("Complete 20 races this week"));
	WeeklyRacing.ChallengeType = EMGChallengeType::Weekly;
	WeeklyRacing.Category = EMGChallengeCategory::Racing;
	WeeklyRacing.State = EMGChallengeState::Available;
	WeeklyRacing.Difficulty = EMGChallengeDifficulty::Medium;
	WeeklyRacing.StartTime = Now;
	WeeklyRacing.EndTime = EndOfWeek;

	FMGChallengeObjective WeeklyObj;
	WeeklyObj.ObjectiveID = FName(TEXT("RacesCompleted"));
	WeeklyObj.Description = FText::FromString(TEXT("Complete races"));
	WeeklyObj.TargetValue = 20;
	WeeklyRacing.Objectives.Add(WeeklyObj);

	FMGChallengeReward WeeklyReward;
	WeeklyReward.ExperienceAmount = 2500;
	WeeklyReward.CurrencyAmount = 10000;
	WeeklyReward.SeasonXPAmount = 500;
	WeeklyRacing.Rewards.Add(WeeklyReward);

	AllChallenges.Add(WeeklyRacing.ChallengeID, WeeklyRacing);
}

void UMGChallengeSubsystem::InitializeDefaultChallenges()
{
	// Permanent achievement-style challenges
	FMGChallenge FirstWin;
	FirstWin.ChallengeID = FName(TEXT("Achievement_FirstWin"));
	FirstWin.Title = FText::FromString(TEXT("First Victory"));
	FirstWin.Description = FText::FromString(TEXT("Win your first race"));
	FirstWin.ChallengeType = EMGChallengeType::Achievement;
	FirstWin.Category = EMGChallengeCategory::Racing;
	FirstWin.State = EMGChallengeState::Available;
	FirstWin.Difficulty = EMGChallengeDifficulty::Easy;

	FMGChallengeObjective FirstWinObj;
	FirstWinObj.ObjectiveID = FName(TEXT("Wins"));
	FirstWinObj.Description = FText::FromString(TEXT("Win a race"));
	FirstWinObj.TargetValue = 1;
	FirstWin.Objectives.Add(FirstWinObj);

	FMGChallengeReward FirstWinReward;
	FirstWinReward.ExperienceAmount = 1000;
	FirstWinReward.CurrencyAmount = 5000;
	FirstWin.Rewards.Add(FirstWinReward);

	AllChallenges.Add(FirstWin.ChallengeID, FirstWin);

	// Generate initial daily and weekly challenges
	GenerateDailyChallenges();
	GenerateWeeklyChallenges();
}

void UMGChallengeSubsystem::GrantReward(const FMGChallengeReward& Reward)
{
	// Grant rewards through appropriate subsystems
	// Currency, XP, items, etc.
}

void UMGChallengeSubsystem::UpdateChallengeState(FMGChallenge& Challenge)
{
	if (Challenge.State == EMGChallengeState::Locked)
	{
		if (ArePrerequisitesMet(Challenge))
		{
			Challenge.State = EMGChallengeState::Available;
		}
	}
}

bool UMGChallengeSubsystem::ArePrerequisitesMet(const FMGChallenge& Challenge) const
{
	for (const FName& PrereqID : Challenge.PrerequisiteChallenges)
	{
		if (const FMGChallenge* Prereq = AllChallenges.Find(PrereqID))
		{
			if (Prereq->State != EMGChallengeState::Completed &&
				Prereq->State != EMGChallengeState::Claimed)
			{
				return false;
			}
		}
	}
	return true;
}

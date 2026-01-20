// Copyright Midnight Grind. All Rights Reserved.

#include "Career/MGCareerSubsystem.h"
#include "Currency/MGCurrencySubsystem.h"

void UMGCareerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadCareerData();
	InitializeObjectives();
}

void UMGCareerSubsystem::Deinitialize()
{
	SaveCareerData();
	Super::Deinitialize();
}

float UMGCareerSubsystem::GetChapterProgressPercent() const
{
	if (Progress.ChapterProgressRequired <= 0)
		return 100.0f;

	return FMath::Clamp(100.0f * ((float)Progress.ChapterProgress / (float)Progress.ChapterProgressRequired), 0.0f, 100.0f);
}

FText UMGCareerSubsystem::GetChapterName(EMGCareerChapter Chapter) const
{
	switch (Chapter)
	{
	case EMGCareerChapter::Newcomer: return FText::FromString(TEXT("Chapter 1: The Newcomer"));
	case EMGCareerChapter::Rising: return FText::FromString(TEXT("Chapter 2: Rising Star"));
	case EMGCareerChapter::Contender: return FText::FromString(TEXT("Chapter 3: The Contender"));
	case EMGCareerChapter::Champion: return FText::FromString(TEXT("Chapter 4: Champion"));
	case EMGCareerChapter::Legend: return FText::FromString(TEXT("Chapter 5: Legend"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

TArray<FMGCareerObjective> UMGCareerSubsystem::GetCurrentObjectives() const
{
	TArray<FMGCareerObjective> Current;
	for (const FMGCareerObjective& Obj : Objectives)
	{
		if (Obj.Chapter == Progress.CurrentChapter && !Obj.bCompleted)
			Current.Add(Obj);
	}
	return Current;
}

TArray<FMGCareerObjective> UMGCareerSubsystem::GetMainObjectives() const
{
	TArray<FMGCareerObjective> Main;
	for (const FMGCareerObjective& Obj : Objectives)
	{
		if (Obj.bIsMainObjective && Obj.Chapter == Progress.CurrentChapter)
			Main.Add(Obj);
	}
	return Main;
}

void UMGCareerSubsystem::UpdateObjectiveProgress(FName ObjectiveID, int32 ProgressDelta)
{
	for (FMGCareerObjective& Obj : Objectives)
	{
		if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
		{
			Obj.CurrentProgress = FMath::Min(Obj.CurrentProgress + ProgressDelta, Obj.TargetProgress);

			if (Obj.CurrentProgress >= Obj.TargetProgress)
			{
				Obj.bCompleted = true;
				GrantObjectiveReward(Obj);
				OnObjectiveCompleted.Broadcast(Obj);

				Progress.ChapterProgress += Obj.bIsMainObjective ? 25 : 10;
				OnCareerProgressUpdated.Broadcast(Progress.ChapterProgress, Progress.ChapterProgressRequired);
				CheckChapterAdvancement();
			}

			SaveCareerData();
			break;
		}
	}
}

void UMGCareerSubsystem::OnRaceCompleted(int32 Position, int32 TotalRacers, bool bWasCleanRace, const TArray<FString>& DefeatedRivals)
{
	Progress.Stats.TotalRaces++;

	if (Position == 1)
	{
		Progress.Stats.Wins++;
		Progress.Stats.CurrentWinStreak++;
		Progress.Stats.HighestWinStreak = FMath::Max(Progress.Stats.HighestWinStreak, Progress.Stats.CurrentWinStreak);
		UpdateObjectiveProgress(FName(TEXT("Obj_Win")), 1);
	}
	else
	{
		Progress.Stats.CurrentWinStreak = 0;
	}

	if (Position <= 3)
	{
		Progress.Stats.Podiums++;
		UpdateObjectiveProgress(FName(TEXT("Obj_Podium")), 1);
	}

	if (bWasCleanRace)
	{
		Progress.Stats.CleanRaces++;
		UpdateObjectiveProgress(FName(TEXT("Obj_CleanRace")), 1);
	}

	for (const FString& RivalID : DefeatedRivals)
	{
		Progress.Stats.RivalsDefeated++;
		UpdateObjectiveProgress(FName(TEXT("Obj_DefeatRival")), 1);
	}

	// Chapter progress for any race
	Progress.ChapterProgress += 2;
	OnCareerProgressUpdated.Broadcast(Progress.ChapterProgress, Progress.ChapterProgressRequired);

	CheckMilestones();
	CheckChapterAdvancement();
	SaveCareerData();
}

void UMGCareerSubsystem::OnTournamentWon(FName TournamentID)
{
	Progress.Stats.TournamentsWon++;
	UpdateObjectiveProgress(FName(TEXT("Obj_WinTournament")), 1);
	CompleteMilestone(EMGCareerMilestone::WonTournament);
}

void UMGCareerSubsystem::OnCrewJoined(FName CrewID)
{
	CompleteMilestone(EMGCareerMilestone::JoinedCrew);
	UpdateObjectiveProgress(FName(TEXT("Obj_JoinCrew")), 1);
}

void UMGCareerSubsystem::AddDistance(float DistanceKM)
{
	Progress.Stats.TotalDistanceKM += DistanceKM;
	UpdateObjectiveProgress(FName(TEXT("Obj_DriveDistance")), FMath::RoundToInt(DistanceKM));
}

void UMGCareerSubsystem::AddRaceTime(float TimeHours)
{
	Progress.Stats.TotalRaceTimeHours += TimeHours;
}

bool UMGCareerSubsystem::HasCompletedMilestone(EMGCareerMilestone Milestone) const
{
	return Progress.CompletedMilestones.Contains(Milestone);
}

TArray<EMGCareerMilestone> UMGCareerSubsystem::GetPendingMilestones() const
{
	TArray<EMGCareerMilestone> Pending;

	if (!HasCompletedMilestone(EMGCareerMilestone::FirstRace) && Progress.Stats.TotalRaces > 0)
		Pending.Add(EMGCareerMilestone::FirstRace);
	if (!HasCompletedMilestone(EMGCareerMilestone::FirstWin) && Progress.Stats.Wins > 0)
		Pending.Add(EMGCareerMilestone::FirstWin);
	if (!HasCompletedMilestone(EMGCareerMilestone::FirstPodium) && Progress.Stats.Podiums > 0)
		Pending.Add(EMGCareerMilestone::FirstPodium);

	return Pending;
}

void UMGCareerSubsystem::LoadCareerData()
{
	// Would load from cloud save
	Progress.ChapterProgressRequired = 100;
}

void UMGCareerSubsystem::SaveCareerData()
{
	// Would save to cloud save
}

void UMGCareerSubsystem::InitializeObjectives()
{
	Objectives.Empty();

	// Chapter 1: Newcomer
	FMGCareerObjective FirstWin;
	FirstWin.ObjectiveID = FName(TEXT("Obj_FirstWin_Ch1"));
	FirstWin.Title = FText::FromString(TEXT("Prove Yourself"));
	FirstWin.Description = FText::FromString(TEXT("Win your first race against real competition"));
	FirstWin.Chapter = EMGCareerChapter::Newcomer;
	FirstWin.TargetProgress = 1;
	FirstWin.bIsMainObjective = true;
	FirstWin.GrindCashReward = 2000;
	FirstWin.ReputationReward = 100;
	Objectives.Add(FirstWin);

	FMGCareerObjective CompeteRaces;
	CompeteRaces.ObjectiveID = FName(TEXT("Obj_Races_Ch1"));
	CompeteRaces.Title = FText::FromString(TEXT("Hit the Streets"));
	CompeteRaces.Description = FText::FromString(TEXT("Complete 10 races"));
	CompeteRaces.Chapter = EMGCareerChapter::Newcomer;
	CompeteRaces.TargetProgress = 10;
	CompeteRaces.GrindCashReward = 1000;
	CompeteRaces.ReputationReward = 50;
	Objectives.Add(CompeteRaces);

	FMGCareerObjective JoinCrew;
	JoinCrew.ObjectiveID = FName(TEXT("Obj_JoinCrew"));
	JoinCrew.Title = FText::FromString(TEXT("Find Your Crew"));
	JoinCrew.Description = FText::FromString(TEXT("Join a crew to race with"));
	JoinCrew.Chapter = EMGCareerChapter::Newcomer;
	JoinCrew.TargetProgress = 1;
	JoinCrew.bIsMainObjective = true;
	JoinCrew.GrindCashReward = 1500;
	JoinCrew.ReputationReward = 100;
	Objectives.Add(JoinCrew);

	// Chapter 2: Rising
	FMGCareerObjective DefeatRival;
	DefeatRival.ObjectiveID = FName(TEXT("Obj_DefeatRival_Ch2"));
	DefeatRival.Title = FText::FromString(TEXT("Rival Showdown"));
	DefeatRival.Description = FText::FromString(TEXT("Defeat your first rival in a head-to-head race"));
	DefeatRival.Chapter = EMGCareerChapter::Rising;
	DefeatRival.TargetProgress = 1;
	DefeatRival.bIsMainObjective = true;
	DefeatRival.GrindCashReward = 3000;
	DefeatRival.ReputationReward = 200;
	Objectives.Add(DefeatRival);

	FMGCareerObjective WinStreak;
	WinStreak.ObjectiveID = FName(TEXT("Obj_WinStreak_Ch2"));
	WinStreak.Title = FText::FromString(TEXT("On Fire"));
	WinStreak.Description = FText::FromString(TEXT("Win 3 races in a row"));
	WinStreak.Chapter = EMGCareerChapter::Rising;
	WinStreak.TargetProgress = 3;
	WinStreak.GrindCashReward = 2500;
	WinStreak.ReputationReward = 150;
	Objectives.Add(WinStreak);

	// Chapter 3: Contender
	FMGCareerObjective Tournament;
	Tournament.ObjectiveID = FName(TEXT("Obj_Tournament_Ch3"));
	Tournament.Title = FText::FromString(TEXT("Tournament Champion"));
	Tournament.Description = FText::FromString(TEXT("Win a tournament"));
	Tournament.Chapter = EMGCareerChapter::Contender;
	Tournament.TargetProgress = 1;
	Tournament.bIsMainObjective = true;
	Tournament.GrindCashReward = 5000;
	Tournament.ReputationReward = 500;
	Objectives.Add(Tournament);

	// Chapter 4: Champion
	FMGCareerObjective DominateLeaderboard;
	DominateLeaderboard.ObjectiveID = FName(TEXT("Obj_Leaderboard_Ch4"));
	DominateLeaderboard.Title = FText::FromString(TEXT("Top of the World"));
	DominateLeaderboard.Description = FText::FromString(TEXT("Reach top 100 in any leaderboard"));
	DominateLeaderboard.Chapter = EMGCareerChapter::Champion;
	DominateLeaderboard.TargetProgress = 1;
	DominateLeaderboard.bIsMainObjective = true;
	DominateLeaderboard.GrindCashReward = 10000;
	DominateLeaderboard.ReputationReward = 1000;
	Objectives.Add(DominateLeaderboard);

	// Chapter 5: Legend
	FMGCareerObjective LegendStatus;
	LegendStatus.ObjectiveID = FName(TEXT("Obj_Legend_Ch5"));
	LegendStatus.Title = FText::FromString(TEXT("Living Legend"));
	LegendStatus.Description = FText::FromString(TEXT("Complete all chapter objectives"));
	LegendStatus.Chapter = EMGCareerChapter::Legend;
	LegendStatus.TargetProgress = 1;
	LegendStatus.bIsMainObjective = true;
	LegendStatus.GrindCashReward = 25000;
	LegendStatus.ReputationReward = 5000;
	Objectives.Add(LegendStatus);
}

void UMGCareerSubsystem::CheckChapterAdvancement()
{
	if (Progress.ChapterProgress >= Progress.ChapterProgressRequired)
	{
		AdvanceChapter();
	}
}

void UMGCareerSubsystem::CheckMilestones()
{
	TArray<EMGCareerMilestone> Pending = GetPendingMilestones();
	for (EMGCareerMilestone Milestone : Pending)
	{
		CompleteMilestone(Milestone);
	}
}

void UMGCareerSubsystem::CompleteMilestone(EMGCareerMilestone Milestone)
{
	if (HasCompletedMilestone(Milestone))
		return;

	Progress.CompletedMilestones.Add(Milestone);
	OnMilestoneReached.Broadcast(Milestone);

	// Award reputation for milestones
	if (UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>())
	{
		Currency->EarnCurrency(EMGCurrencyType::LegacyMarks, 50, EMGEarnSource::StoryMilestone, TEXT("Career milestone"));
	}
}

void UMGCareerSubsystem::AdvanceChapter()
{
	if (Progress.CurrentChapter == EMGCareerChapter::Legend)
		return;

	Progress.CurrentChapter = (EMGCareerChapter)((int32)Progress.CurrentChapter + 1);
	Progress.ChapterProgress = 0;
	Progress.ChapterProgressRequired = 100 + ((int32)Progress.CurrentChapter * 25); // Increases each chapter

	OnChapterAdvanced.Broadcast(Progress.CurrentChapter);

	// Award chapter completion bonus
	if (UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>())
	{
		int64 ChapterBonus = 5000 * (int32)Progress.CurrentChapter;
		Currency->EarnCurrency(EMGCurrencyType::GrindCash, ChapterBonus, EMGEarnSource::StoryMilestone, TEXT("Chapter completion"));
	}

	SaveCareerData();
}

void UMGCareerSubsystem::GrantObjectiveReward(const FMGCareerObjective& Objective)
{
	if (UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>())
	{
		if (Objective.GrindCashReward > 0)
			Currency->EarnCurrency(EMGCurrencyType::GrindCash, Objective.GrindCashReward, EMGEarnSource::StoryMilestone, TEXT("Objective reward"));

		if (Objective.ReputationReward > 0)
			Progress.TotalReputation += Objective.ReputationReward;
	}
}

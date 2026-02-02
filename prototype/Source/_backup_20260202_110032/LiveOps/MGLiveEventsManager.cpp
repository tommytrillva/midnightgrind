// Copyright Midnight Grind. All Rights Reserved.

#include "LiveOps/MGLiveEventsManager.h"
#include "GameModes/MGRaceGameMode.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGLiveEventsManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize reset times
	FDateTime Now = FDateTime::UtcNow();

	// Find last midnight
	LastDailyReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0);

	// Find last Monday
	int32 DayOfWeek = static_cast<int32>(Now.GetDayOfWeek());
	int32 DaysToMonday = (DayOfWeek == 0) ? 6 : DayOfWeek - 1; // Sunday = 0
	LastWeeklyReset = LastDailyReset - FTimespan::FromDays(DaysToMonday);

	// Generate initial challenges
	GenerateDailyChallenges();
	GenerateWeeklyChallenges();

	// Set up periodic reset check (every minute)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ResetCheckTimer,
			this,
			&UMGLiveEventsManager::CheckForResets,
			60.0f, // Every minute
			true   // Looping
		);
	}
}

void UMGLiveEventsManager::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResetCheckTimer);
	}

	Super::Deinitialize();
}

// ==========================================
// CHALLENGE MANAGEMENT
// ==========================================

TArray<FMGChallenge> UMGLiveEventsManager::GetDailyChallenges() const
{
	return DailyChallenges;
}

TArray<FMGChallenge> UMGLiveEventsManager::GetWeeklyChallenges() const
{
	return WeeklyChallenges;
}

FMGChallenge UMGLiveEventsManager::GetChallenge(FName ChallengeID) const
{
	// Check dailies
	for (const FMGChallenge& Challenge : DailyChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID)
		{
			return Challenge;
		}
	}

	// Check weeklies
	for (const FMGChallenge& Challenge : WeeklyChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID)
		{
			return Challenge;
		}
	}

	return FMGChallenge();
}

void UMGLiveEventsManager::UpdateChallengeProgress(FName ChallengeID, int32 ProgressDelta)
{
	// Update in dailies
	for (FMGChallenge& Challenge : DailyChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID && !Challenge.bCompleted)
		{
			Challenge.CurrentProgress = FMath::Min(Challenge.CurrentProgress + ProgressDelta, Challenge.TargetValue);

			OnChallengeProgressUpdated.Broadcast(Challenge);

			if (Challenge.CurrentProgress >= Challenge.TargetValue)
			{
				Challenge.bCompleted = true;
				OnChallengeCompleted.Broadcast(Challenge);
			}

			return;
		}
	}

	// Update in weeklies
	for (FMGChallenge& Challenge : WeeklyChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID && !Challenge.bCompleted)
		{
			Challenge.CurrentProgress = FMath::Min(Challenge.CurrentProgress + ProgressDelta, Challenge.TargetValue);

			OnChallengeProgressUpdated.Broadcast(Challenge);

			if (Challenge.CurrentProgress >= Challenge.TargetValue)
			{
				Challenge.bCompleted = true;
				OnChallengeCompleted.Broadcast(Challenge);
			}

			return;
		}
	}
}

void UMGLiveEventsManager::ProcessRaceForChallenges(const FMGRaceResults& Results)
{
	// Find player result
	int32 PlayerPosition = -1;
	float PlayerDriftScore = 0.0f;
	float PlayerLapTime = 0.0f;
	bool bHadDamage = false;
	bool bUsedNOS = true; // Assume used unless proven otherwise
	int32 StartPosition = 8; // Assume started last

	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		if (!Racer.bIsAI)
		{
			PlayerPosition = Racer.Position;
			PlayerDriftScore = Racer.DriftScore;
			PlayerLapTime = Racer.BestLapTime;
			break;
		}
	}

	if (PlayerPosition < 0)
	{
		return;
	}

	// Process each challenge type
	auto ProcessChallenge = [&](FMGChallenge& Challenge)
	{
		if (Challenge.bCompleted)
		{
			return;
		}

		int32 Progress = 0;

		switch (Challenge.Type)
		{
		case EMGChallengeType::RaceCount:
			Progress = 1;
			break;

		case EMGChallengeType::WinCount:
			if (PlayerPosition == 1)
			{
				Progress = 1;
			}
			break;

		case EMGChallengeType::PodiumCount:
			if (PlayerPosition <= 3)
			{
				Progress = 1;
			}
			break;

		case EMGChallengeType::DriftScore:
			Progress = static_cast<int32>(PlayerDriftScore);
			break;

		case EMGChallengeType::LapTime:
			if (Challenge.RequiredTrackID.IsNone() || Challenge.RequiredTrackID == Results.Config.TrackID)
			{
				if (PlayerLapTime > 0.0f && PlayerLapTime <= Challenge.TargetTime)
				{
					Progress = 1;
				}
			}
			break;

		case EMGChallengeType::SpecificTrack:
			if (Challenge.RequiredTrackID == Results.Config.TrackID)
			{
				Progress = 1;
			}
			break;

		case EMGChallengeType::FlawlessWin:
			if (PlayerPosition == 1 && !bHadDamage)
			{
				Progress = 1;
			}
			break;

		case EMGChallengeType::DominatingWin:
			if (PlayerPosition == 1)
			{
				// Check time gap
				for (const FMGRacerData& Racer : Results.RacerResults)
				{
					if (Racer.Position == 2)
					{
						float Gap = Racer.TotalTime - Results.RacerResults[0].TotalTime;
						if (Gap >= 5.0f) // 5 second lead
						{
							Progress = 1;
						}
						break;
					}
				}
			}
			break;

		case EMGChallengeType::CombackWin:
			if (PlayerPosition == 1 && StartPosition >= 6)
			{
				Progress = 1;
			}
			break;

		case EMGChallengeType::NoNOS:
			if (!bUsedNOS)
			{
				Progress = 1;
			}
			break;

		case EMGChallengeType::PinkSlipWin:
			if (Results.Config.bPinkSlipRace && PlayerPosition == 1)
			{
				Progress = 1;
			}
			break;

		default:
			break;
		}

		if (Progress > 0)
		{
			Challenge.CurrentProgress = FMath::Min(Challenge.CurrentProgress + Progress, Challenge.TargetValue);
			OnChallengeProgressUpdated.Broadcast(Challenge);

			if (Challenge.CurrentProgress >= Challenge.TargetValue)
			{
				Challenge.bCompleted = true;
				OnChallengeCompleted.Broadcast(Challenge);
			}
		}
	};

	// Process all challenges
	for (FMGChallenge& Challenge : DailyChallenges)
	{
		ProcessChallenge(Challenge);
	}

	for (FMGChallenge& Challenge : WeeklyChallenges)
	{
		ProcessChallenge(Challenge);
	}
}

bool UMGLiveEventsManager::ClaimChallengeReward(FName ChallengeID)
{
	// Find and claim from dailies
	for (FMGChallenge& Challenge : DailyChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID)
		{
			if (Challenge.bCompleted && !Challenge.bRewardClaimed)
			{
				Challenge.bRewardClaimed = true;
				AwardReward(Challenge.Reward);
				OnChallengeRewardClaimed.Broadcast(Challenge);
				return true;
			}
			return false;
		}
	}

	// Find and claim from weeklies
	for (FMGChallenge& Challenge : WeeklyChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID)
		{
			if (Challenge.bCompleted && !Challenge.bRewardClaimed)
			{
				Challenge.bRewardClaimed = true;
				AwardReward(Challenge.Reward);
				OnChallengeRewardClaimed.Broadcast(Challenge);
				return true;
			}
			return false;
		}
	}

	return false;
}

FTimespan UMGLiveEventsManager::GetTimeUntilDailyReset() const
{
	FDateTime Now = FDateTime::UtcNow();
	FDateTime NextReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0) + FTimespan::FromDays(1);
	return NextReset - Now;
}

FTimespan UMGLiveEventsManager::GetTimeUntilWeeklyReset() const
{
	FDateTime Now = FDateTime::UtcNow();
	int32 DayOfWeek = static_cast<int32>(Now.GetDayOfWeek());
	int32 DaysToMonday = (DayOfWeek == 0) ? 1 : 8 - DayOfWeek;
	FDateTime NextMonday = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0) + FTimespan::FromDays(DaysToMonday);
	return NextMonday - Now;
}

// ==========================================
// COMMUNITY GOALS
// ==========================================

TArray<FMGCommunityGoal> UMGLiveEventsManager::GetActiveCommunityGoals() const
{
	TArray<FMGCommunityGoal> Active;

	for (const FMGCommunityGoal& Goal : CommunityGoals)
	{
		if (Goal.bActive)
		{
			Active.Add(Goal);
		}
	}

	return Active;
}

void UMGLiveEventsManager::ContributeToCommunityGoal(FName GoalID, int64 Amount)
{
	for (FMGCommunityGoal& Goal : CommunityGoals)
	{
		if (Goal.GoalID == GoalID && Goal.bActive)
		{
			Goal.PlayerContribution += Amount;
			Goal.CurrentProgress += Amount;

			// Check for tier progression
			float ProgressPercent = static_cast<float>(Goal.CurrentProgress) / Goal.TargetValue;

			for (int32 i = Goal.CurrentTier; i < Goal.RewardTiers.Num(); ++i)
			{
				if (ProgressPercent >= Goal.RewardTiers[i])
				{
					Goal.CurrentTier = i + 1;
					OnCommunityGoalTierReached.Broadcast(Goal, Goal.CurrentTier);

					// Award tier reward
					if (Goal.TierRewards.IsValidIndex(i))
					{
						AwardReward(Goal.TierRewards[i]);
					}
				}
			}

			OnCommunityGoalUpdated.Broadcast(Goal);
			return;
		}
	}
}

int64 UMGLiveEventsManager::GetPlayerContribution(FName GoalID) const
{
	for (const FMGCommunityGoal& Goal : CommunityGoals)
	{
		if (Goal.GoalID == GoalID)
		{
			return Goal.PlayerContribution;
		}
	}
	return 0;
}

// ==========================================
// LIVE EVENTS
// ==========================================

TArray<FMGLiveEvent> UMGLiveEventsManager::GetActiveLiveEvents() const
{
	TArray<FMGLiveEvent> Active;

	for (const FMGLiveEvent& Event : LiveEvents)
	{
		if (Event.IsActive())
		{
			Active.Add(Event);
		}
	}

	return Active;
}

TArray<FMGLiveEvent> UMGLiveEventsManager::GetUpcomingEvents() const
{
	TArray<FMGLiveEvent> Upcoming;
	FDateTime Now = FDateTime::UtcNow();

	for (const FMGLiveEvent& Event : LiveEvents)
	{
		if (Event.StartTime > Now)
		{
			Upcoming.Add(Event);
		}
	}

	// Sort by start time
	Upcoming.Sort([](const FMGLiveEvent& A, const FMGLiveEvent& B)
	{
		return A.StartTime < B.StartTime;
	});

	return Upcoming;
}

void UMGLiveEventsManager::GetEventMultipliers(float& OutXPMultiplier, float& OutCreditsMultiplier) const
{
	OutXPMultiplier = 1.0f;
	OutCreditsMultiplier = 1.0f;

	for (const FMGLiveEvent& Event : LiveEvents)
	{
		if (Event.IsActive())
		{
			OutXPMultiplier *= Event.XPMultiplier;
			OutCreditsMultiplier *= Event.CreditsMultiplier;
		}
	}
}

bool UMGLiveEventsManager::IsVehicleFeatured(FName VehicleID) const
{
	for (const FMGLiveEvent& Event : LiveEvents)
	{
		if (Event.IsActive() && Event.FeaturedVehicles.Contains(VehicleID))
		{
			return true;
		}
	}
	return false;
}

bool UMGLiveEventsManager::IsTrackFeatured(FName TrackID) const
{
	for (const FMGLiveEvent& Event : LiveEvents)
	{
		if (Event.IsActive() && Event.FeaturedTracks.Contains(TrackID))
		{
			return true;
		}
	}
	return false;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGLiveEventsManager::GenerateDailyChallenges()
{
	DailyChallenges.Empty();

	// Create a mix of challenge types
	static const TArray<EMGChallengeType> EasyTypes = {
		EMGChallengeType::RaceCount,
		EMGChallengeType::PodiumCount,
		EMGChallengeType::DriftScore
	};

	static const TArray<EMGChallengeType> MediumTypes = {
		EMGChallengeType::WinCount,
		EMGChallengeType::PodiumCount,
		EMGChallengeType::DriftScore,
		EMGChallengeType::Overtakes
	};

	static const TArray<EMGChallengeType> HardTypes = {
		EMGChallengeType::WinCount,
		EMGChallengeType::FlawlessWin,
		EMGChallengeType::DominatingWin
	};

	// Generate 3 daily challenges: 1 easy, 1 medium, 1 hard
	DailyChallenges.Add(CreateChallenge(
		EasyTypes[FMath::RandRange(0, EasyTypes.Num() - 1)],
		EMGChallengeReset::Daily,
		EMGChallengeDifficulty::Easy,
		0 // Will be set by CreateChallenge
	));

	DailyChallenges.Add(CreateChallenge(
		MediumTypes[FMath::RandRange(0, MediumTypes.Num() - 1)],
		EMGChallengeReset::Daily,
		EMGChallengeDifficulty::Medium,
		0
	));

	DailyChallenges.Add(CreateChallenge(
		HardTypes[FMath::RandRange(0, HardTypes.Num() - 1)],
		EMGChallengeReset::Daily,
		EMGChallengeDifficulty::Hard,
		0
	));

	OnDailyChallengesRefreshed.Broadcast();
}

void UMGLiveEventsManager::GenerateWeeklyChallenges()
{
	WeeklyChallenges.Empty();

	// Weekly challenges are bigger goals
	WeeklyChallenges.Add(CreateChallenge(
		EMGChallengeType::RaceCount,
		EMGChallengeReset::Weekly,
		EMGChallengeDifficulty::Medium,
		25 // Complete 25 races
	));

	WeeklyChallenges.Add(CreateChallenge(
		EMGChallengeType::WinCount,
		EMGChallengeReset::Weekly,
		EMGChallengeDifficulty::Medium,
		10 // Win 10 races
	));

	WeeklyChallenges.Add(CreateChallenge(
		EMGChallengeType::DriftScore,
		EMGChallengeReset::Weekly,
		EMGChallengeDifficulty::Hard,
		50000 // 50k drift score
	));

	WeeklyChallenges.Add(CreateChallenge(
		EMGChallengeType::EarnCredits,
		EMGChallengeReset::Weekly,
		EMGChallengeDifficulty::Medium,
		100000 // Earn 100k credits
	));

	WeeklyChallenges.Add(CreateChallenge(
		EMGChallengeType::FlawlessWin,
		EMGChallengeReset::Weekly,
		EMGChallengeDifficulty::Extreme,
		3 // 3 flawless wins
	));
}

void UMGLiveEventsManager::CheckForResets()
{
	FDateTime Now = FDateTime::UtcNow();

	// Check daily reset
	FDateTime TodayMidnight(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0);
	if (TodayMidnight > LastDailyReset)
	{
		LastDailyReset = TodayMidnight;
		GenerateDailyChallenges();
	}

	// Check weekly reset (Monday)
	if (Now.GetDayOfWeek() == EDayOfWeek::Monday)
	{
		FDateTime ThisMonday(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0);
		if (ThisMonday > LastWeeklyReset)
		{
			LastWeeklyReset = ThisMonday;
			GenerateWeeklyChallenges();
		}
	}

	// Update event status
	UpdateEventStatus();
}

FMGChallenge UMGLiveEventsManager::CreateChallenge(EMGChallengeType Type, EMGChallengeReset Reset,
	EMGChallengeDifficulty Difficulty, int32 TargetOverride)
{
	FMGChallenge Challenge;

	// Generate unique ID
	Challenge.ChallengeID = FName(*FString::Printf(TEXT("Challenge_%d_%d"),
		static_cast<int32>(Type), FMath::Rand()));

	Challenge.Type = Type;
	Challenge.ResetPeriod = Reset;
	Challenge.Difficulty = Difficulty;
	Challenge.Category = FName(TEXT("Racing"));

	// Set expiration
	if (Reset == EMGChallengeReset::Daily)
	{
		Challenge.ExpirationTime = FDateTime::UtcNow() + GetTimeUntilDailyReset();
	}
	else if (Reset == EMGChallengeReset::Weekly)
	{
		Challenge.ExpirationTime = FDateTime::UtcNow() + GetTimeUntilWeeklyReset();
	}

	// Set target and rewards based on type and difficulty
	int32 Target = TargetOverride;
	int64 CreditReward = 0;
	int32 XPReward = 0;

	switch (Type)
	{
	case EMGChallengeType::RaceCount:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_RaceCount", "Road Warrior");
		if (Target == 0)
		{
			Target = (Difficulty == EMGChallengeDifficulty::Easy) ? 3 :
				(Difficulty == EMGChallengeDifficulty::Medium) ? 5 : 10;
		}
		Challenge.Description = FText::Format(
			NSLOCTEXT("MG", "Challenge_RaceCountDesc", "Complete {0} races"),
			FText::AsNumber(Target));
		CreditReward = Target * 500;
		XPReward = Target * 50;
		break;

	case EMGChallengeType::WinCount:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_WinCount", "Victory Lane");
		if (Target == 0)
		{
			Target = (Difficulty == EMGChallengeDifficulty::Easy) ? 1 :
				(Difficulty == EMGChallengeDifficulty::Medium) ? 3 : 5;
		}
		Challenge.Description = FText::Format(
			NSLOCTEXT("MG", "Challenge_WinCountDesc", "Win {0} races"),
			FText::AsNumber(Target));
		CreditReward = Target * 1500;
		XPReward = Target * 100;
		break;

	case EMGChallengeType::PodiumCount:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_Podium", "Podium Finish");
		if (Target == 0)
		{
			Target = (Difficulty == EMGChallengeDifficulty::Easy) ? 2 :
				(Difficulty == EMGChallengeDifficulty::Medium) ? 5 : 8;
		}
		Challenge.Description = FText::Format(
			NSLOCTEXT("MG", "Challenge_PodiumDesc", "Finish in top 3 in {0} races"),
			FText::AsNumber(Target));
		CreditReward = Target * 750;
		XPReward = Target * 60;
		break;

	case EMGChallengeType::DriftScore:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_Drift", "Drift King");
		if (Target == 0)
		{
			Target = (Difficulty == EMGChallengeDifficulty::Easy) ? 5000 :
				(Difficulty == EMGChallengeDifficulty::Medium) ? 15000 : 30000;
		}
		Challenge.Description = FText::Format(
			NSLOCTEXT("MG", "Challenge_DriftDesc", "Accumulate {0} drift score"),
			FText::AsNumber(Target));
		CreditReward = Target / 5;
		XPReward = Target / 50;
		break;

	case EMGChallengeType::FlawlessWin:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_Flawless", "Untouchable");
		Challenge.Description = NSLOCTEXT("MG", "Challenge_FlawlessDesc", "Win a race without taking damage");
		Target = 1;
		CreditReward = 3000;
		XPReward = 200;
		break;

	case EMGChallengeType::DominatingWin:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_Dominate", "Total Domination");
		Challenge.Description = NSLOCTEXT("MG", "Challenge_DominateDesc", "Win by 5+ seconds");
		Target = 1;
		CreditReward = 2500;
		XPReward = 150;
		break;

	case EMGChallengeType::CombackWin:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_Comeback", "Against All Odds");
		Challenge.Description = NSLOCTEXT("MG", "Challenge_ComebackDesc", "Win after starting 6th or worse");
		Target = 1;
		CreditReward = 4000;
		XPReward = 250;
		break;

	case EMGChallengeType::EarnCredits:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_Earn", "Money Maker");
		if (Target == 0)
		{
			Target = 25000;
		}
		Challenge.Description = FText::Format(
			NSLOCTEXT("MG", "Challenge_EarnDesc", "Earn {0} credits from races"),
			FText::AsNumber(Target));
		CreditReward = Target / 4;
		XPReward = Target / 250;
		break;

	default:
		Challenge.DisplayName = NSLOCTEXT("MG", "Challenge_Generic", "Challenge");
		Challenge.Description = NSLOCTEXT("MG", "Challenge_GenericDesc", "Complete this challenge");
		Target = 1;
		CreditReward = 1000;
		XPReward = 50;
		break;
	}

	Challenge.TargetValue = Target;

	// Apply difficulty multiplier to rewards
	float DifficultyMult = 1.0f;
	switch (Difficulty)
	{
	case EMGChallengeDifficulty::Medium:
		DifficultyMult = 1.5f;
		break;
	case EMGChallengeDifficulty::Hard:
		DifficultyMult = 2.5f;
		break;
	case EMGChallengeDifficulty::Extreme:
		DifficultyMult = 4.0f;
		break;
	default:
		break;
	}

	Challenge.Reward.Credits = static_cast<int64>(CreditReward * DifficultyMult);
	Challenge.Reward.XP = static_cast<int32>(XPReward * DifficultyMult);
	Challenge.Reward.Reputation = static_cast<int32>(XPReward * DifficultyMult * 0.5f);

	return Challenge;
}

void UMGLiveEventsManager::AwardReward(const FMGChallengeReward& Reward)
{
	if (TransactionPipeline.IsValid())
	{
		// TransactionPipeline->AwardChallengeRewards(NAME_None, Reward.Credits, {});
	}

	// Award XP through progression subsystem
	// Award reputation through career subsystem
	// Unlock items if any
}

void UMGLiveEventsManager::UpdateEventStatus()
{
	FDateTime Now = FDateTime::UtcNow();

	for (FMGLiveEvent& Event : LiveEvents)
	{
		bool bWasActive = Event.IsActive();

		// Check if just started
		if (!bWasActive && Now >= Event.StartTime && Now <= Event.EndTime)
		{
			OnLiveEventStarted.Broadcast(Event);
		}
		// Check if just ended
		else if (bWasActive && Now > Event.EndTime)
		{
			OnLiveEventEnded.Broadcast(Event);
		}
	}
}

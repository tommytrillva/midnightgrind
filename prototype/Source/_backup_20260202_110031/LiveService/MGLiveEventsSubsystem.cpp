// Copyright Midnight Grind. All Rights Reserved.

#include "LiveService/MGLiveEventsSubsystem.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/Paths.h"

void UMGLiveEventsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load saved progress
	LoadProgress();

	// Generate daily challenges if needed
	GenerateDailyChallenges();

	// Create mock events for testing
	CreateMockEvents();

	// Update statuses
	UpdateEventStatuses();
}

void UMGLiveEventsSubsystem::Deinitialize()
{
	SaveProgress();
	Super::Deinitialize();
}

void UMGLiveEventsSubsystem::Tick(float DeltaTime)
{
	// Periodic status update
	static float UpdateTimer = 0.0f;
	UpdateTimer += DeltaTime;

	if (UpdateTimer >= 60.0f) // Every minute
	{
		UpdateTimer = 0.0f;
		UpdateEventStatuses();
		CheckExpiredEvents();
	}
}

// ==========================================
// LIVE EVENTS
// ==========================================

TArray<FMGLiveEvent> UMGLiveEventsSubsystem::GetActiveEvents() const
{
	TArray<FMGLiveEvent> ActiveEvents;

	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.Status == EMGEventStatus::Active || Event.Status == EMGEventStatus::EndingSoon)
		{
			ActiveEvents.Add(Event);
		}
	}

	return ActiveEvents;
}

TArray<FMGLiveEvent> UMGLiveEventsSubsystem::GetUpcomingEvents() const
{
	TArray<FMGLiveEvent> UpcomingEvents;

	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.Status == EMGEventStatus::Upcoming)
		{
			UpcomingEvents.Add(Event);
		}
	}

	// Sort by start time
	UpcomingEvents.Sort([](const FMGLiveEvent& A, const FMGLiveEvent& B)
	{
		return A.StartTime < B.StartTime;
	});

	return UpcomingEvents;
}

FMGLiveEvent UMGLiveEventsSubsystem::GetEventByID(FName EventID) const
{
	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.EventID == EventID)
		{
			return Event;
		}
	}

	return FMGLiveEvent();
}

bool UMGLiveEventsSubsystem::IsEventActive(FName EventID) const
{
	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.EventID == EventID)
		{
			return Event.Status == EMGEventStatus::Active || Event.Status == EMGEventStatus::EndingSoon;
		}
	}

	return false;
}

FTimespan UMGLiveEventsSubsystem::GetEventTimeRemaining(FName EventID) const
{
	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.EventID == EventID)
		{
			FDateTime Now = FDateTime::UtcNow();
			if (Event.EndTime > Now)
			{
				return Event.EndTime - Now;
			}
		}
	}

	return FTimespan::Zero();
}

void UMGLiveEventsSubsystem::RefreshEvents()
{
	// In production, this would fetch from server
	UpdateEventStatuses();
}

// ==========================================
// CHALLENGES
// ==========================================

FMGEventChallenge UMGLiveEventsSubsystem::GetChallengeProgress(FName EventID, FName ChallengeID) const
{
	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.EventID == EventID)
		{
			for (const FMGEventChallenge& Challenge : Event.Challenges)
			{
				if (Challenge.ChallengeID == ChallengeID)
				{
					return Challenge;
				}
			}
		}
	}

	return FMGEventChallenge();
}

void UMGLiveEventsSubsystem::ReportChallengeProgress(EMGChallengeType Type, int32 Value, FName TrackID, FName VehicleID)
{
	// Update all active event challenges
	for (FMGLiveEvent& Event : AllEvents)
	{
		if (Event.Status != EMGEventStatus::Active && Event.Status != EMGEventStatus::EndingSoon)
		{
			continue;
		}

		for (FMGEventChallenge& Challenge : Event.Challenges)
		{
			if (Challenge.bIsCompleted)
			{
				continue;
			}

			for (FMGChallengeObjective& Objective : Challenge.Objectives)
			{
				if (Objective.bIsCompleted)
				{
					continue;
				}

				UpdateObjectiveProgress(Objective, Type, Value, TrackID, VehicleID);
			}

			CheckChallengeCompletion(Challenge);

			if (Challenge.bIsCompleted && !Challenge.bIsClaimed)
			{
				OnChallengeCompleted.Broadcast(Challenge);
			}
			else
			{
				OnChallengeProgress.Broadcast(Challenge.ChallengeID, Challenge);
			}
		}

		// Update community goals
		for (FMGCommunityGoal& Goal : Event.CommunityGoals)
		{
			if (Goal.bIsCompleted)
			{
				continue;
			}

			if (Goal.Type == Type)
			{
				Goal.PlayerContribution += Value;
				Goal.CommunityProgress += Value; // In production, this comes from server

				CheckCommunityGoalTiers(Goal, Event.EventID);

				OnCommunityGoalProgress.Broadcast(Goal.GoalID, Goal);
			}
		}

		Event.bHasParticipated = true;
	}

	// Update daily challenges
	for (FMGEventChallenge& Challenge : DailyChallenges.Challenges)
	{
		if (Challenge.bIsCompleted)
		{
			continue;
		}

		for (FMGChallengeObjective& Objective : Challenge.Objectives)
		{
			if (!Objective.bIsCompleted)
			{
				UpdateObjectiveProgress(Objective, Type, Value, TrackID, VehicleID);
			}
		}

		CheckChallengeCompletion(Challenge);

		if (Challenge.bIsCompleted)
		{
			OnChallengeCompleted.Broadcast(Challenge);
		}
	}

	// Check if all daily challenges completed
	bool bAllCompleted = true;
	for (const FMGEventChallenge& Challenge : DailyChallenges.Challenges)
	{
		if (!Challenge.bIsCompleted)
		{
			bAllCompleted = false;
			break;
		}
	}
	DailyChallenges.bAllCompleted = bAllCompleted;

	SaveProgress();
}

bool UMGLiveEventsSubsystem::ClaimChallengeReward(FName EventID, FName ChallengeID)
{
	for (FMGLiveEvent& Event : AllEvents)
	{
		if (Event.EventID == EventID)
		{
			for (FMGEventChallenge& Challenge : Event.Challenges)
			{
				if (Challenge.ChallengeID == ChallengeID && Challenge.bIsCompleted && !Challenge.bIsClaimed)
				{
					Challenge.bIsClaimed = true;
					Event.TotalXPEarned += Challenge.XPReward;

					// Grant rewards would happen here
					SaveProgress();
					return true;
				}
			}
		}
	}

	// Check daily challenges
	for (FMGEventChallenge& Challenge : DailyChallenges.Challenges)
	{
		if (Challenge.ChallengeID == ChallengeID && Challenge.bIsCompleted && !Challenge.bIsClaimed)
		{
			Challenge.bIsClaimed = true;
			SaveProgress();
			return true;
		}
	}

	return false;
}

TArray<FMGEventChallenge> UMGLiveEventsSubsystem::GetUnclaimedChallenges() const
{
	TArray<FMGEventChallenge> Unclaimed;

	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.Status == EMGEventStatus::Active || Event.Status == EMGEventStatus::EndingSoon)
		{
			for (const FMGEventChallenge& Challenge : Event.Challenges)
			{
				if (Challenge.bIsCompleted && !Challenge.bIsClaimed)
				{
					Unclaimed.Add(Challenge);
				}
			}
		}
	}

	for (const FMGEventChallenge& Challenge : DailyChallenges.Challenges)
	{
		if (Challenge.bIsCompleted && !Challenge.bIsClaimed)
		{
			Unclaimed.Add(Challenge);
		}
	}

	return Unclaimed;
}

// ==========================================
// DAILY CHALLENGES
// ==========================================

FMGDailyChallenges UMGLiveEventsSubsystem::GetDailyChallenges() const
{
	return DailyChallenges;
}

bool UMGLiveEventsSubsystem::ClaimDailyBonusReward()
{
	if (DailyChallenges.bAllCompleted && !DailyChallenges.bBonusClaimed)
	{
		DailyChallenges.bBonusClaimed = true;
		LastDailyCompletion = FDateTime::UtcNow();

		// Update streak
		FDateTime Today = FDateTime::UtcNow();
		FDateTime Yesterday = Today - FTimespan::FromDays(1);

		if (LastDailyCompletion.GetDay() == Yesterday.GetDay())
		{
			DailyStreak++;
		}
		else if (LastDailyCompletion.GetDay() != Today.GetDay())
		{
			DailyStreak = 1;
		}

		SaveProgress();
		return true;
	}

	return false;
}

// ==========================================
// COMMUNITY GOALS
// ==========================================

FMGCommunityGoal UMGLiveEventsSubsystem::GetCommunityGoalProgress(FName EventID, FName GoalID) const
{
	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.EventID == EventID)
		{
			for (const FMGCommunityGoal& Goal : Event.CommunityGoals)
			{
				if (Goal.GoalID == GoalID)
				{
					return Goal;
				}
			}
		}
	}

	return FMGCommunityGoal();
}

void UMGLiveEventsSubsystem::ContributeToCommunityGoal(FName EventID, FName GoalID, int32 Contribution)
{
	for (FMGLiveEvent& Event : AllEvents)
	{
		if (Event.EventID == EventID)
		{
			for (FMGCommunityGoal& Goal : Event.CommunityGoals)
			{
				if (Goal.GoalID == GoalID && !Goal.bIsCompleted)
				{
					Goal.PlayerContribution += Contribution;
					Goal.CommunityProgress += Contribution;

					CheckCommunityGoalTiers(Goal, EventID);

					OnCommunityGoalProgress.Broadcast(GoalID, Goal);
					SaveProgress();
					return;
				}
			}
		}
	}
}

float UMGLiveEventsSubsystem::GetCommunityGoalPercentage(FName EventID, FName GoalID) const
{
	FMGCommunityGoal Goal = GetCommunityGoalProgress(EventID, GoalID);

	if (Goal.CommunityTarget > 0)
	{
		return FMath::Clamp(static_cast<float>(Goal.CommunityProgress) / static_cast<float>(Goal.CommunityTarget), 0.0f, 1.0f);
	}

	return 0.0f;
}

// ==========================================
// PLAYLISTS
// ==========================================

TArray<FMGFeaturedPlaylist> UMGLiveEventsSubsystem::GetFeaturedPlaylists() const
{
	TArray<FMGFeaturedPlaylist> Featured;

	for (const FMGFeaturedPlaylist& Playlist : FeaturedPlaylists)
	{
		if (Playlist.bIsFeatured)
		{
			Featured.Add(Playlist);
		}
	}

	// Also get from active events
	for (const FMGLiveEvent& Event : AllEvents)
	{
		if (Event.Status == EMGEventStatus::Active || Event.Status == EMGEventStatus::EndingSoon)
		{
			for (const FMGFeaturedPlaylist& Playlist : Event.FeaturedPlaylists)
			{
				Featured.Add(Playlist);
			}
		}
	}

	return Featured;
}

FMGFeaturedPlaylist UMGLiveEventsSubsystem::GetPlaylistByID(FName PlaylistID) const
{
	for (const FMGFeaturedPlaylist& Playlist : FeaturedPlaylists)
	{
		if (Playlist.PlaylistID == PlaylistID)
		{
			return Playlist;
		}
	}

	for (const FMGLiveEvent& Event : AllEvents)
	{
		for (const FMGFeaturedPlaylist& Playlist : Event.FeaturedPlaylists)
		{
			if (Playlist.PlaylistID == PlaylistID)
			{
				return Playlist;
			}
		}
	}

	return FMGFeaturedPlaylist();
}

void UMGLiveEventsSubsystem::GetPlaylistMultipliers(FName PlaylistID, float& OutXPMultiplier, float& OutCashMultiplier) const
{
	FMGFeaturedPlaylist Playlist = GetPlaylistByID(PlaylistID);
	OutXPMultiplier = Playlist.XPMultiplier;
	OutCashMultiplier = Playlist.CashMultiplier;
}

// ==========================================
// UTILITY
// ==========================================

FText UMGLiveEventsSubsystem::GetEventTypeDisplayName(EMGEventType Type)
{
	switch (Type)
	{
	case EMGEventType::Weekend:
		return NSLOCTEXT("LiveEvents", "Weekend", "Weekend Event");
	case EMGEventType::Weekly:
		return NSLOCTEXT("LiveEvents", "Weekly", "Weekly Challenge");
	case EMGEventType::Daily:
		return NSLOCTEXT("LiveEvents", "Daily", "Daily Challenge");
	case EMGEventType::LimitedTime:
		return NSLOCTEXT("LiveEvents", "LimitedTime", "Limited Time");
	case EMGEventType::CommunityGoal:
		return NSLOCTEXT("LiveEvents", "CommunityGoal", "Community Goal");
	case EMGEventType::Holiday:
		return NSLOCTEXT("LiveEvents", "Holiday", "Holiday Event");
	case EMGEventType::Collaboration:
		return NSLOCTEXT("LiveEvents", "Collaboration", "Collaboration");
	case EMGEventType::Flash:
		return NSLOCTEXT("LiveEvents", "Flash", "Flash Event");
	default:
		return FText::GetEmpty();
	}
}

FText UMGLiveEventsSubsystem::GetEventStatusDisplayName(EMGEventStatus Status)
{
	switch (Status)
	{
	case EMGEventStatus::Upcoming:
		return NSLOCTEXT("LiveEvents", "Upcoming", "Coming Soon");
	case EMGEventStatus::Active:
		return NSLOCTEXT("LiveEvents", "Active", "Active");
	case EMGEventStatus::EndingSoon:
		return NSLOCTEXT("LiveEvents", "EndingSoon", "Ending Soon");
	case EMGEventStatus::Completed:
		return NSLOCTEXT("LiveEvents", "Completed", "Completed");
	case EMGEventStatus::Expired:
		return NSLOCTEXT("LiveEvents", "Expired", "Expired");
	default:
		return FText::GetEmpty();
	}
}

FText UMGLiveEventsSubsystem::GetChallengeTypeDisplayName(EMGChallengeType Type)
{
	switch (Type)
	{
	case EMGChallengeType::WinRaces:
		return NSLOCTEXT("LiveEvents", "WinRaces", "Win Races");
	case EMGChallengeType::CompleteRaces:
		return NSLOCTEXT("LiveEvents", "CompleteRaces", "Complete Races");
	case EMGChallengeType::AchievePosition:
		return NSLOCTEXT("LiveEvents", "AchievePosition", "Finish Position");
	case EMGChallengeType::BeatLapTime:
		return NSLOCTEXT("LiveEvents", "BeatLapTime", "Beat Lap Time");
	case EMGChallengeType::DriveDistance:
		return NSLOCTEXT("LiveEvents", "DriveDistance", "Drive Distance");
	case EMGChallengeType::ReachTopSpeed:
		return NSLOCTEXT("LiveEvents", "ReachTopSpeed", "Reach Top Speed");
	case EMGChallengeType::DriftDistance:
		return NSLOCTEXT("LiveEvents", "DriftDistance", "Drift Distance");
	case EMGChallengeType::NearMisses:
		return NSLOCTEXT("LiveEvents", "NearMisses", "Near Misses");
	case EMGChallengeType::Overtakes:
		return NSLOCTEXT("LiveEvents", "Overtakes", "Overtakes");
	case EMGChallengeType::UseVehicle:
		return NSLOCTEXT("LiveEvents", "UseVehicle", "Use Vehicle");
	case EMGChallengeType::RaceOnTrack:
		return NSLOCTEXT("LiveEvents", "RaceOnTrack", "Race on Track");
	case EMGChallengeType::WinStreak:
		return NSLOCTEXT("LiveEvents", "WinStreak", "Win Streak");
	case EMGChallengeType::PerfectLaps:
		return NSLOCTEXT("LiveEvents", "PerfectLaps", "Perfect Laps");
	case EMGChallengeType::EarnCurrency:
		return NSLOCTEXT("LiveEvents", "EarnCurrency", "Earn Currency");
	case EMGChallengeType::CommunityTotal:
		return NSLOCTEXT("LiveEvents", "CommunityTotal", "Community Goal");
	default:
		return FText::GetEmpty();
	}
}

FText UMGLiveEventsSubsystem::FormatTimeRemaining(FTimespan TimeRemaining)
{
	if (TimeRemaining.GetTotalDays() >= 1.0)
	{
		int32 Days = FMath::FloorToInt(TimeRemaining.GetTotalDays());
		int32 Hours = TimeRemaining.GetHours();
		return FText::Format(NSLOCTEXT("LiveEvents", "DaysHours", "{0}d {1}h"), FText::AsNumber(Days), FText::AsNumber(Hours));
	}
	else if (TimeRemaining.GetTotalHours() >= 1.0)
	{
		int32 Hours = FMath::FloorToInt(TimeRemaining.GetTotalHours());
		int32 Minutes = TimeRemaining.GetMinutes();
		return FText::Format(NSLOCTEXT("LiveEvents", "HoursMinutes", "{0}h {1}m"), FText::AsNumber(Hours), FText::AsNumber(Minutes));
	}
	else
	{
		int32 Minutes = FMath::FloorToInt(TimeRemaining.GetTotalMinutes());
		int32 Seconds = TimeRemaining.GetSeconds();
		return FText::Format(NSLOCTEXT("LiveEvents", "MinutesSeconds", "{0}m {1}s"), FText::AsNumber(Minutes), FText::AsNumber(Seconds));
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGLiveEventsSubsystem::UpdateEventStatuses()
{
	FDateTime Now = FDateTime::UtcNow();

	for (FMGLiveEvent& Event : AllEvents)
	{
		EMGEventStatus OldStatus = Event.Status;

		if (Now < Event.StartTime)
		{
			Event.Status = EMGEventStatus::Upcoming;
		}
		else if (Now >= Event.StartTime && Now < Event.EndTime)
		{
			// Check if ending soon (less than 24 hours)
			FTimespan Remaining = Event.EndTime - Now;
			if (Remaining.GetTotalHours() < 24.0)
			{
				Event.Status = EMGEventStatus::EndingSoon;
			}
			else
			{
				Event.Status = EMGEventStatus::Active;
			}
		}
		else
		{
			if (Event.bHasParticipated)
			{
				Event.Status = EMGEventStatus::Completed;
			}
			else
			{
				Event.Status = EMGEventStatus::Expired;
			}
		}

		// Broadcast status changes
		if (OldStatus != Event.Status)
		{
			if (Event.Status == EMGEventStatus::Active)
			{
				OnEventStarted.Broadcast(Event);
			}
			else if (Event.Status == EMGEventStatus::Completed || Event.Status == EMGEventStatus::Expired)
			{
				OnEventEnded.Broadcast(Event);
			}
		}
	}
}

void UMGLiveEventsSubsystem::CheckExpiredEvents()
{
	// Remove very old events
	FDateTime Cutoff = FDateTime::UtcNow() - FTimespan::FromDays(7);

	AllEvents.RemoveAll([Cutoff](const FMGLiveEvent& Event)
	{
		return Event.EndTime < Cutoff;
	});
}

void UMGLiveEventsSubsystem::GenerateDailyChallenges()
{
	FDateTime Today = FDateTime::UtcNow();

	// Check if already generated today
	if (DailyChallenges.Date.GetDay() == Today.GetDay() &&
		DailyChallenges.Date.GetMonth() == Today.GetMonth() &&
		DailyChallenges.Date.GetYear() == Today.GetYear())
	{
		return;
	}

	DailyChallenges = FMGDailyChallenges();
	DailyChallenges.Date = Today;

	// Generate 3 daily challenges
	TArray<EMGChallengeType> PossibleTypes = {
		EMGChallengeType::WinRaces,
		EMGChallengeType::CompleteRaces,
		EMGChallengeType::DriftDistance,
		EMGChallengeType::Overtakes,
		EMGChallengeType::NearMisses
	};

	for (int32 i = 0; i < 3; i++)
	{
		FMGEventChallenge Challenge;
		Challenge.ChallengeID = *FString::Printf(TEXT("Daily_%d"), i);

		// Pick random type
		int32 TypeIndex = FMath::RandRange(0, PossibleTypes.Num() - 1);
		EMGChallengeType Type = PossibleTypes[TypeIndex];
		PossibleTypes.RemoveAt(TypeIndex);

		FMGChallengeObjective Objective;
		Objective.ObjectiveID = TEXT("Main");
		Objective.Type = Type;

		switch (Type)
		{
		case EMGChallengeType::WinRaces:
			Objective.TargetValue = FMath::RandRange(1, 3);
			Objective.Description = FText::Format(NSLOCTEXT("Daily", "WinRaces", "Win {0} races"), FText::AsNumber(Objective.TargetValue));
			Challenge.DisplayName = NSLOCTEXT("Daily", "WinChallenge", "Winner");
			Challenge.XPReward = 500 * Objective.TargetValue;
			break;

		case EMGChallengeType::CompleteRaces:
			Objective.TargetValue = FMath::RandRange(3, 5);
			Objective.Description = FText::Format(NSLOCTEXT("Daily", "CompleteRaces", "Complete {0} races"), FText::AsNumber(Objective.TargetValue));
			Challenge.DisplayName = NSLOCTEXT("Daily", "RacerChallenge", "Racer");
			Challenge.XPReward = 200 * Objective.TargetValue;
			break;

		case EMGChallengeType::DriftDistance:
			Objective.TargetValue = FMath::RandRange(5000, 10000);
			Objective.Description = FText::Format(NSLOCTEXT("Daily", "DriftDistance", "Drift {0}m total"), FText::AsNumber(Objective.TargetValue));
			Challenge.DisplayName = NSLOCTEXT("Daily", "DrifterChallenge", "Drifter");
			Challenge.XPReward = 750;
			break;

		case EMGChallengeType::Overtakes:
			Objective.TargetValue = FMath::RandRange(10, 20);
			Objective.Description = FText::Format(NSLOCTEXT("Daily", "Overtakes", "Perform {0} overtakes"), FText::AsNumber(Objective.TargetValue));
			Challenge.DisplayName = NSLOCTEXT("Daily", "OvertakerChallenge", "Overtaker");
			Challenge.XPReward = 600;
			break;

		case EMGChallengeType::NearMisses:
			Objective.TargetValue = FMath::RandRange(15, 30);
			Objective.Description = FText::Format(NSLOCTEXT("Daily", "NearMisses", "Perform {0} near misses"), FText::AsNumber(Objective.TargetValue));
			Challenge.DisplayName = NSLOCTEXT("Daily", "DaredevilChallenge", "Daredevil");
			Challenge.XPReward = 400;
			break;

		default:
			break;
		}

		Challenge.Objectives.Add(Objective);
		Challenge.Difficulty = i + 1;
		Challenge.SortOrder = i;

		DailyChallenges.Challenges.Add(Challenge);
	}

	// Bonus reward for completing all
	DailyChallenges.BonusReward.RewardID = TEXT("DailyBonus");
	DailyChallenges.BonusReward.RewardType = TEXT("Currency");
	DailyChallenges.BonusReward.DisplayName = NSLOCTEXT("Daily", "BonusCash", "Bonus Cash");
	DailyChallenges.BonusReward.Quantity = 5000;

	OnDailyChallengesRefreshed.Broadcast();
}

void UMGLiveEventsSubsystem::CheckChallengeCompletion(FMGEventChallenge& Challenge)
{
	bool bAllComplete = true;

	for (FMGChallengeObjective& Objective : Challenge.Objectives)
	{
		if (Objective.CurrentValue >= Objective.TargetValue)
		{
			Objective.bIsCompleted = true;
		}
		else
		{
			bAllComplete = false;
		}
	}

	Challenge.bIsCompleted = bAllComplete;
}

void UMGLiveEventsSubsystem::CheckCommunityGoalTiers(FMGCommunityGoal& Goal, FName EventID)
{
	int32 OldTier = Goal.CurrentTier;

	for (int32 i = Goal.TierThresholds.Num() - 1; i >= 0; i--)
	{
		if (Goal.CommunityProgress >= Goal.TierThresholds[i])
		{
			Goal.CurrentTier = i + 1;
			break;
		}
	}

	if (Goal.CommunityProgress >= Goal.CommunityTarget)
	{
		Goal.bIsCompleted = true;
	}

	if (Goal.CurrentTier > OldTier)
	{
		OnCommunityGoalTierReached.Broadcast(Goal.GoalID, Goal.CurrentTier);
	}
}

void UMGLiveEventsSubsystem::UpdateObjectiveProgress(FMGChallengeObjective& Objective, EMGChallengeType Type, int32 Value, FName TrackID, FName VehicleID)
{
	if (Objective.Type != Type)
	{
		return;
	}

	// Check requirements
	if (!Objective.RequiredTrack.IsNone() && Objective.RequiredTrack != TrackID)
	{
		return;
	}

	if (!Objective.RequiredVehicle.IsNone() && Objective.RequiredVehicle != VehicleID)
	{
		return;
	}

	Objective.CurrentValue += Value;

	if (Objective.CurrentValue >= Objective.TargetValue)
	{
		Objective.bIsCompleted = true;
	}
}

void UMGLiveEventsSubsystem::CreateMockEvents()
{
	FDateTime Now = FDateTime::UtcNow();

	// Weekly Event
	{
		FMGLiveEvent WeeklyEvent;
		WeeklyEvent.EventID = TEXT("WeeklyChallenge_001");
		WeeklyEvent.Type = EMGEventType::Weekly;
		WeeklyEvent.DisplayName = NSLOCTEXT("Events", "SpeedDemon", "Speed Demon Week");
		WeeklyEvent.Description = NSLOCTEXT("Events", "SpeedDemonDesc", "Push your limits and reach new top speeds!");
		WeeklyEvent.ThemeColor = FLinearColor(1.0f, 0.3f, 0.0f);
		WeeklyEvent.StartTime = Now - FTimespan::FromDays(2);
		WeeklyEvent.EndTime = Now + FTimespan::FromDays(5);

		// Add challenges
		FMGEventChallenge SpeedChallenge;
		SpeedChallenge.ChallengeID = TEXT("TopSpeed");
		SpeedChallenge.DisplayName = NSLOCTEXT("Events", "TopSpeedChallenge", "Speed King");
		SpeedChallenge.XPReward = 2000;

		FMGChallengeObjective SpeedObj;
		SpeedObj.ObjectiveID = TEXT("Speed");
		SpeedObj.Type = EMGChallengeType::ReachTopSpeed;
		SpeedObj.TargetValue = 300;
		SpeedObj.Description = NSLOCTEXT("Events", "Reach300", "Reach 300 km/h");
		SpeedChallenge.Objectives.Add(SpeedObj);

		WeeklyEvent.Challenges.Add(SpeedChallenge);

		// Community goal
		FMGCommunityGoal DistanceGoal;
		DistanceGoal.GoalID = TEXT("CommunityDistance");
		DistanceGoal.DisplayName = NSLOCTEXT("Events", "CommunityDistance", "Community Distance");
		DistanceGoal.Description = NSLOCTEXT("Events", "CommunityDistanceDesc", "Together, drive 1 million kilometers!");
		DistanceGoal.Type = EMGChallengeType::DriveDistance;
		DistanceGoal.CommunityTarget = 1000000000; // 1 million km in meters
		DistanceGoal.TierThresholds = { 250000000, 500000000, 750000000, 1000000000 };
		DistanceGoal.CommunityProgress = 456789000; // Mock progress

		WeeklyEvent.CommunityGoals.Add(DistanceGoal);

		AllEvents.Add(WeeklyEvent);
	}

	// Weekend Event
	{
		FMGLiveEvent WeekendEvent;
		WeekendEvent.EventID = TEXT("Weekend_DoubleXP");
		WeekendEvent.Type = EMGEventType::Weekend;
		WeekendEvent.DisplayName = NSLOCTEXT("Events", "DoubleXP", "Double XP Weekend");
		WeekendEvent.Description = NSLOCTEXT("Events", "DoubleXPDesc", "Earn double XP on all races this weekend!");
		WeekendEvent.ThemeColor = FLinearColor(0.3f, 0.8f, 1.0f);
		WeekendEvent.StartTime = Now - FTimespan::FromHours(12);
		WeekendEvent.EndTime = Now + FTimespan::FromHours(36);

		// Featured playlist with multiplier
		FMGFeaturedPlaylist DoubleXPPlaylist;
		DoubleXPPlaylist.PlaylistID = TEXT("DoubleXP_Playlist");
		DoubleXPPlaylist.DisplayName = NSLOCTEXT("Events", "DoubleXPPlaylist", "Double XP Races");
		DoubleXPPlaylist.XPMultiplier = 2.0f;
		DoubleXPPlaylist.CashMultiplier = 1.5f;
		DoubleXPPlaylist.bIsFeatured = true;

		WeekendEvent.FeaturedPlaylists.Add(DoubleXPPlaylist);

		AllEvents.Add(WeekendEvent);
	}
}

void UMGLiveEventsSubsystem::SaveProgress()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("LiveEvents");
	IFileManager::Get().MakeDirectory(*SaveDir, true);
	FString FilePath = SaveDir / TEXT("LiveEventsProgress.sav");

	FBufferArchive Archive;

	int32 Version = 1;
	Archive << Version;

	// Save daily streak
	Archive << DailyStreak;
	int64 LastCompletionUnix = LastDailyCompletion.ToUnixTimestamp();
	Archive << LastCompletionUnix;

	// Save daily challenge completion status
	int32 ChallengeCount = DailyChallenges.Challenges.Num();
	Archive << ChallengeCount;
	for (const FMGDailyChallenge& Challenge : DailyChallenges.Challenges)
	{
		FName ChallengeID = Challenge.ChallengeID;
		int32 Progress = Challenge.CurrentProgress;
		bool bCompleted = Challenge.bCompleted;
		bool bRewardClaimed = Challenge.bRewardClaimed;

		Archive << ChallengeID;
		Archive << Progress;
		Archive << bCompleted;
		Archive << bRewardClaimed;
	}

	// Save event participation progress
	int32 EventCount = AllEvents.Num();
	Archive << EventCount;
	for (const FMGLiveEvent& Event : AllEvents)
	{
		FName EventID = Event.EventID;
		int32 Progress = Event.PlayerProgress;
		bool bParticipating = Event.bIsParticipating;
		bool bCompleted = Event.bCompleted;

		Archive << EventID;
		Archive << Progress;
		Archive << bParticipating;
		Archive << bCompleted;
	}

	if (FFileHelper::SaveArrayToFile(Archive, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Live events progress saved - Streak: %d, Events: %d"),
			DailyStreak, EventCount);
	}
}

void UMGLiveEventsSubsystem::LoadProgress()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("LiveEvents");
	FString FilePath = SaveDir / TEXT("LiveEventsProgress.sav");

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return;
	}

	FMemoryReader Archive(FileData, true);

	int32 Version = 0;
	Archive << Version;

	if (Version >= 1)
	{
		// Load daily streak
		Archive << DailyStreak;
		int64 LastCompletionUnix;
		Archive << LastCompletionUnix;
		LastDailyCompletion = FDateTime::FromUnixTimestamp(LastCompletionUnix);

		// Load daily challenge progress
		int32 ChallengeCount;
		Archive << ChallengeCount;
		TMap<FName, TPair<TPair<int32, bool>, bool>> ChallengeData;
		for (int32 i = 0; i < ChallengeCount; i++)
		{
			FName ChallengeID;
			int32 Progress;
			bool bCompleted;
			bool bRewardClaimed;

			Archive << ChallengeID;
			Archive << Progress;
			Archive << bCompleted;
			Archive << bRewardClaimed;

			ChallengeData.Add(ChallengeID, TPair<TPair<int32, bool>, bool>(TPair<int32, bool>(Progress, bCompleted), bRewardClaimed));
		}

		// Apply to daily challenges
		for (FMGDailyChallenge& Challenge : DailyChallenges.Challenges)
		{
			if (const auto* Data = ChallengeData.Find(Challenge.ChallengeID))
			{
				Challenge.CurrentProgress = Data->Key.Key;
				Challenge.bCompleted = Data->Key.Value;
				Challenge.bRewardClaimed = Data->Value;
			}
		}

		// Load event progress
		int32 EventCount;
		Archive << EventCount;
		TMap<FName, FVector4> EventData;
		for (int32 i = 0; i < EventCount; i++)
		{
			FName EventID;
			int32 Progress;
			bool bParticipating;
			bool bCompleted;

			Archive << EventID;
			Archive << Progress;
			Archive << bParticipating;
			Archive << bCompleted;

			EventData.Add(EventID, FVector4(Progress, bParticipating ? 1.0f : 0.0f, bCompleted ? 1.0f : 0.0f, 0.0f));
		}

		// Apply to events
		for (FMGLiveEvent& Event : AllEvents)
		{
			if (const FVector4* Data = EventData.Find(Event.EventID))
			{
				Event.PlayerProgress = static_cast<int32>(Data->X);
				Event.bIsParticipating = Data->Y > 0.5f;
				Event.bCompleted = Data->Z > 0.5f;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Live events progress loaded - Streak: %d, Challenges: %d, Events: %d"),
			DailyStreak, ChallengeCount, EventCount);
	}
}

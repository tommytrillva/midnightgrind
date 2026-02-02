// Copyright Midnight Grind. All Rights Reserved.

#include "Seasons/MGSeasonSubsystem.h"
#include "Misc/DateTime.h"

void UMGSeasonSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSeasonData();
	LoadEventsData();
	LoadProgress();

	// Check initial state
	CheckEventTimers();
	CheckChallengeResets();
}

void UMGSeasonSubsystem::Deinitialize()
{
	SaveProgress();
	Super::Deinitialize();
}

void UMGSeasonSubsystem::Tick(float DeltaTime)
{
	EventCheckAccumulator += DeltaTime;
	if (EventCheckAccumulator >= EventCheckInterval)
	{
		EventCheckAccumulator = 0.0f;
		CheckEventTimers();
		CheckChallengeResets();
	}
}

// ==========================================
// SEASON
// ==========================================

FTimespan UMGSeasonSubsystem::GetSeasonTimeRemaining() const
{
	FDateTime Now = FDateTime::Now();
	if (CurrentSeason.EndDate > Now)
	{
		return CurrentSeason.EndDate - Now;
	}
	return FTimespan::Zero();
}

TArray<FMGSeasonReward> UMGSeasonSubsystem::GetRewardsForTier(int32 Tier) const
{
	TArray<FMGSeasonReward> TierRewards;
	for (const FMGSeasonReward& Reward : CurrentSeason.Rewards)
	{
		if (Reward.Tier == Tier)
		{
			TierRewards.Add(Reward);
		}
	}
	return TierRewards;
}

TArray<FMGSeasonReward> UMGSeasonSubsystem::GetAvailableRewards() const
{
	TArray<FMGSeasonReward> Available;
	for (const FMGSeasonReward& Reward : CurrentSeason.Rewards)
	{
		if (Reward.Tier <= SeasonProgress.CurrentTier && !Reward.bIsClaimed)
		{
			if (!Reward.bIsPremium || SeasonProgress.bHasPremiumPass)
			{
				Available.Add(Reward);
			}
		}
	}
	return Available;
}

bool UMGSeasonSubsystem::ClaimTierReward(int32 Tier, bool bPremium)
{
	if (Tier > SeasonProgress.CurrentTier)
	{
		return false;
	}

	if (bPremium && !SeasonProgress.bHasPremiumPass)
	{
		return false;
	}

	TArray<int32>& ClaimedArray = bPremium ? SeasonProgress.ClaimedPremiumTiers : SeasonProgress.ClaimedTiers;
	if (ClaimedArray.Contains(Tier))
	{
		return false;
	}

	ClaimedArray.Add(Tier);

	// Mark rewards as claimed
	for (FMGSeasonReward& Reward : CurrentSeason.Rewards)
	{
		if (Reward.Tier == Tier && Reward.bIsPremium == bPremium)
		{
			Reward.bIsClaimed = true;
			// Grant reward would happen here
		}
	}

	SaveProgress();
	return true;
}

void UMGSeasonSubsystem::ClaimAllRewards()
{
	for (int32 Tier = 1; Tier <= SeasonProgress.CurrentTier; Tier++)
	{
		ClaimTierReward(Tier, false);
		if (SeasonProgress.bHasPremiumPass)
		{
			ClaimTierReward(Tier, true);
		}
	}
}

void UMGSeasonSubsystem::AddSeasonXP(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	int32 OldTier = SeasonProgress.CurrentTier;
	SeasonProgress.CurrentXP += Amount;
	SeasonProgress.TotalXP += Amount;

	// Check for tier ups
	while (SeasonProgress.CurrentXP >= CurrentSeason.XPPerTier && SeasonProgress.CurrentTier < CurrentSeason.MaxTier)
	{
		SeasonProgress.CurrentXP -= CurrentSeason.XPPerTier;
		SeasonProgress.CurrentTier++;
	}

	// Cap XP at max tier
	if (SeasonProgress.CurrentTier >= CurrentSeason.MaxTier)
	{
		SeasonProgress.CurrentTier = CurrentSeason.MaxTier;
		SeasonProgress.CurrentXP = 0;
	}

	OnSeasonXPGained.Broadcast(Amount, SeasonProgress.TotalXP);

	if (SeasonProgress.CurrentTier > OldTier)
	{
		ProcessTierUp(OldTier, SeasonProgress.CurrentTier);
	}

	SaveProgress();
}

int32 UMGSeasonSubsystem::GetXPForNextTier() const
{
	if (SeasonProgress.CurrentTier >= CurrentSeason.MaxTier)
	{
		return 0;
	}
	return CurrentSeason.XPPerTier - SeasonProgress.CurrentXP;
}

float UMGSeasonSubsystem::GetTierProgress() const
{
	if (SeasonProgress.CurrentTier >= CurrentSeason.MaxTier)
	{
		return 1.0f;
	}
	return static_cast<float>(SeasonProgress.CurrentXP) / static_cast<float>(CurrentSeason.XPPerTier);
}

void UMGSeasonSubsystem::PurchasePremiumPass()
{
	if (SeasonProgress.bHasPremiumPass)
	{
		return;
	}

	SeasonProgress.bHasPremiumPass = true;
	SaveProgress();
}

// ==========================================
// EVENTS
// ==========================================

TArray<FMGEventData> UMGSeasonSubsystem::GetActiveEvents() const
{
	FDateTime Now = FDateTime::Now();
	TArray<FMGEventData> Active;

	for (const FMGEventData& Event : ActiveEvents)
	{
		if (Event.StartTime <= Now && Event.EndTime > Now)
		{
			Active.Add(Event);
		}
	}

	return Active;
}

FMGEventData UMGSeasonSubsystem::GetFeaturedEvent() const
{
	for (const FMGEventData& Event : ActiveEvents)
	{
		if (Event.bIsFeatured)
		{
			return Event;
		}
	}
	return FMGEventData();
}

FMGEventData UMGSeasonSubsystem::GetEvent(FName EventID) const
{
	for (const FMGEventData& Event : ActiveEvents)
	{
		if (Event.EventID == EventID)
		{
			return Event;
		}
	}
	return FMGEventData();
}

TArray<FMGEventData> UMGSeasonSubsystem::GetUpcomingEvents() const
{
	return UpcomingEvents;
}

TArray<FMGEventData> UMGSeasonSubsystem::GetCompletedEvents() const
{
	return CompletedEvents;
}

void UMGSeasonSubsystem::JoinEvent(FName EventID)
{
	for (FMGEventData& Event : ActiveEvents)
	{
		if (Event.EventID == EventID && !Event.bIsParticipating)
		{
			Event.bIsParticipating = true;
			Event.TotalParticipants++;
			SaveProgress();
			return;
		}
	}
}

FTimespan UMGSeasonSubsystem::GetEventTimeRemaining(FName EventID) const
{
	FDateTime Now = FDateTime::Now();
	for (const FMGEventData& Event : ActiveEvents)
	{
		if (Event.EventID == EventID && Event.EndTime > Now)
		{
			return Event.EndTime - Now;
		}
	}
	return FTimespan::Zero();
}

void UMGSeasonSubsystem::UpdateEventProgress(FName StatID, int32 Value, FName TrackID, FName VehicleClass)
{
	for (FMGEventData& Event : ActiveEvents)
	{
		if (!Event.bIsParticipating || Event.bIsCompleted)
		{
			continue;
		}

		for (FMGEventObjective& Objective : Event.Objectives)
		{
			if (Objective.bIsCompleted)
			{
				continue;
			}

			// Check if this stat matches objective
			if (Objective.TrackedStat != StatID)
			{
				continue;
			}

			// Check track requirement
			if (!Objective.RequiredTrack.IsNone() && Objective.RequiredTrack != TrackID)
			{
				continue;
			}

			// Check vehicle class requirement
			if (!Objective.RequiredVehicleClass.IsNone() && Objective.RequiredVehicleClass != VehicleClass)
			{
				continue;
			}

			// Update progress
			Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + Value, Objective.TargetValue);
			if (Objective.CurrentProgress >= Objective.TargetValue)
			{
				Objective.bIsCompleted = true;
			}

			OnEventObjectiveProgress.Broadcast(Event, Objective);
		}

		CheckEventCompletion(Event);
	}

	// Also update daily/weekly challenges
	for (FMGEventObjective& Objective : DailyChallenges)
	{
		if (!Objective.bIsCompleted && Objective.TrackedStat == StatID)
		{
			Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + Value, Objective.TargetValue);
			if (Objective.CurrentProgress >= Objective.TargetValue)
			{
				Objective.bIsCompleted = true;
				AddSeasonXP(100); // Daily challenge bonus
			}
		}
	}

	for (FMGEventObjective& Objective : WeeklyChallenges)
	{
		if (!Objective.bIsCompleted && Objective.TrackedStat == StatID)
		{
			Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + Value, Objective.TargetValue);
			if (Objective.CurrentProgress >= Objective.TargetValue)
			{
				Objective.bIsCompleted = true;
				AddSeasonXP(500); // Weekly challenge bonus
			}
		}
	}

	SaveProgress();
}

// ==========================================
// CHALLENGES
// ==========================================

FTimespan UMGSeasonSubsystem::GetDailyResetTime() const
{
	FDateTime Now = FDateTime::Now();
	FDateTime NextReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0) + FTimespan::FromDays(1);
	return NextReset - Now;
}

FTimespan UMGSeasonSubsystem::GetWeeklyResetTime() const
{
	FDateTime Now = FDateTime::Now();
	int32 DaysUntilMonday = (8 - static_cast<int32>(Now.GetDayOfWeek())) % 7;
	if (DaysUntilMonday == 0) DaysUntilMonday = 7;
	FDateTime NextReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0) + FTimespan::FromDays(DaysUntilMonday);
	return NextReset - Now;
}

void UMGSeasonSubsystem::RefreshDailyChallenges()
{
	GenerateDailyChallenges();
	SaveProgress();
}

// ==========================================
// UTILITY
// ==========================================

FText UMGSeasonSubsystem::FormatTimeRemaining(FTimespan Time)
{
	if (Time.GetTotalDays() >= 1.0)
	{
		return FText::FromString(FString::Printf(TEXT("%dd %dh"), static_cast<int32>(Time.GetTotalDays()), Time.GetHours() % 24));
	}
	else if (Time.GetTotalHours() >= 1.0)
	{
		return FText::FromString(FString::Printf(TEXT("%dh %dm"), static_cast<int32>(Time.GetTotalHours()), Time.GetMinutes() % 60));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("%dm %ds"), Time.GetMinutes(), Time.GetSeconds() % 60));
	}
}

FText UMGSeasonSubsystem::GetRewardTypeDisplayName(EMGRewardType Type)
{
	switch (Type)
	{
		case EMGRewardType::Cash: return FText::FromString(TEXT("Cash"));
		case EMGRewardType::Reputation: return FText::FromString(TEXT("Reputation"));
		case EMGRewardType::SeasonXP: return FText::FromString(TEXT("Season XP"));
		case EMGRewardType::Vehicle: return FText::FromString(TEXT("Vehicle"));
		case EMGRewardType::Customization: return FText::FromString(TEXT("Customization"));
		case EMGRewardType::Cosmetic: return FText::FromString(TEXT("Cosmetic"));
		case EMGRewardType::Title: return FText::FromString(TEXT("Title"));
		case EMGRewardType::Wrap: return FText::FromString(TEXT("Wrap"));
		default: return FText::FromString(TEXT("Reward"));
	}
}

FText UMGSeasonSubsystem::GetEventTypeDisplayName(EMGEventType Type)
{
	switch (Type)
	{
		case EMGEventType::Weekly: return FText::FromString(TEXT("Weekly Challenge"));
		case EMGEventType::Weekend: return FText::FromString(TEXT("Weekend Special"));
		case EMGEventType::TimeTrial: return FText::FromString(TEXT("Time Trial"));
		case EMGEventType::Community: return FText::FromString(TEXT("Community Event"));
		case EMGEventType::Holiday: return FText::FromString(TEXT("Holiday Event"));
		case EMGEventType::LimitedTime: return FText::FromString(TEXT("Limited Time"));
		case EMGEventType::CrewBattle: return FText::FromString(TEXT("Crew Battle"));
		case EMGEventType::Championship: return FText::FromString(TEXT("Championship"));
		default: return FText::FromString(TEXT("Event"));
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGSeasonSubsystem::LoadSeasonData()
{
	// Would load from server
	GenerateMockSeason();
}

void UMGSeasonSubsystem::LoadEventsData()
{
	// Would load from server
	GenerateMockEvents();
}

void UMGSeasonSubsystem::LoadProgress()
{
	// Would load from save file
	SeasonProgress.SeasonID = CurrentSeason.SeasonID;
	SeasonProgress.CurrentTier = 1;
	SeasonProgress.CurrentXP = 0;
	SeasonProgress.TotalXP = 0;

	// Generate initial challenges
	GenerateDailyChallenges();
	GenerateWeeklyChallenges();
}

void UMGSeasonSubsystem::SaveProgress()
{
	// Would save to persistent storage
}

void UMGSeasonSubsystem::CheckEventTimers()
{
	FDateTime Now = FDateTime::Now();

	// Check for newly started events
	for (int32 i = UpcomingEvents.Num() - 1; i >= 0; i--)
	{
		if (UpcomingEvents[i].StartTime <= Now)
		{
			FMGEventData StartedEvent = UpcomingEvents[i];
			UpcomingEvents.RemoveAt(i);
			ActiveEvents.Add(StartedEvent);
			OnEventStarted.Broadcast(StartedEvent);
		}
	}

	// Check for ended events
	for (int32 i = ActiveEvents.Num() - 1; i >= 0; i--)
	{
		if (ActiveEvents[i].EndTime <= Now)
		{
			FMGEventData EndedEvent = ActiveEvents[i];
			ActiveEvents.RemoveAt(i);
			CompletedEvents.Add(EndedEvent);
			OnEventEnded.Broadcast(EndedEvent);
		}
	}
}

void UMGSeasonSubsystem::CheckChallengeResets()
{
	FDateTime Now = FDateTime::Now();

	// Check daily reset
	FDateTime TodayReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0);
	if (LastDailyReset < TodayReset)
	{
		GenerateDailyChallenges();
		LastDailyReset = TodayReset;
		SaveProgress();
	}

	// Check weekly reset (Monday)
	int32 DaysSinceMonday = (static_cast<int32>(Now.GetDayOfWeek()) - 1 + 7) % 7;
	FDateTime ThisWeekReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0) - FTimespan::FromDays(DaysSinceMonday);
	if (LastWeeklyReset < ThisWeekReset)
	{
		GenerateWeeklyChallenges();
		LastWeeklyReset = ThisWeekReset;
		SaveProgress();
	}
}

void UMGSeasonSubsystem::GenerateDailyChallenges()
{
	DailyChallenges.Empty();

	// Complete 3 races
	{
		FMGEventObjective Obj;
		Obj.ObjectiveID = FName("Daily_Races");
		Obj.Description = FText::FromString(TEXT("Complete 3 races"));
		Obj.TargetValue = 3;
		Obj.TrackedStat = FName("RacesCompleted");
		DailyChallenges.Add(Obj);
	}

	// Win 1 race
	{
		FMGEventObjective Obj;
		Obj.ObjectiveID = FName("Daily_Win");
		Obj.Description = FText::FromString(TEXT("Win a race"));
		Obj.TargetValue = 1;
		Obj.TrackedStat = FName("Wins");
		DailyChallenges.Add(Obj);
	}

	// Drift 5000 meters
	{
		FMGEventObjective Obj;
		Obj.ObjectiveID = FName("Daily_Drift");
		Obj.Description = FText::FromString(TEXT("Drift for 5000 meters"));
		Obj.TargetValue = 5000;
		Obj.TrackedStat = FName("DriftDistance");
		DailyChallenges.Add(Obj);
	}
}

void UMGSeasonSubsystem::GenerateWeeklyChallenges()
{
	WeeklyChallenges.Empty();

	// Win 10 races
	{
		FMGEventObjective Obj;
		Obj.ObjectiveID = FName("Weekly_Wins");
		Obj.Description = FText::FromString(TEXT("Win 10 races"));
		Obj.TargetValue = 10;
		Obj.TrackedStat = FName("Wins");
		WeeklyChallenges.Add(Obj);
	}

	// Complete 25 races
	{
		FMGEventObjective Obj;
		Obj.ObjectiveID = FName("Weekly_Races");
		Obj.Description = FText::FromString(TEXT("Complete 25 races"));
		Obj.TargetValue = 25;
		Obj.TrackedStat = FName("RacesCompleted");
		WeeklyChallenges.Add(Obj);
	}

	// Set 5 personal bests
	{
		FMGEventObjective Obj;
		Obj.ObjectiveID = FName("Weekly_PBs");
		Obj.Description = FText::FromString(TEXT("Set 5 personal bests"));
		Obj.TargetValue = 5;
		Obj.TrackedStat = FName("PersonalBests");
		WeeklyChallenges.Add(Obj);
	}

	// Use NOS 50 times
	{
		FMGEventObjective Obj;
		Obj.ObjectiveID = FName("Weekly_NOS");
		Obj.Description = FText::FromString(TEXT("Use NOS 50 times"));
		Obj.TargetValue = 50;
		Obj.TrackedStat = FName("NOSUsed");
		WeeklyChallenges.Add(Obj);
	}
}

void UMGSeasonSubsystem::ProcessTierUp(int32 OldTier, int32 NewTier)
{
	TArray<FMGSeasonReward> UnlockedRewards;

	for (int32 Tier = OldTier + 1; Tier <= NewTier; Tier++)
	{
		TArray<FMGSeasonReward> TierRewards = GetRewardsForTier(Tier);
		for (const FMGSeasonReward& Reward : TierRewards)
		{
			if (!Reward.bIsPremium || SeasonProgress.bHasPremiumPass)
			{
				UnlockedRewards.Add(Reward);
			}
		}
	}

	OnSeasonTierUp.Broadcast(NewTier, UnlockedRewards);
}

void UMGSeasonSubsystem::CheckEventCompletion(FMGEventData& Event)
{
	if (Event.bIsCompleted)
	{
		return;
	}

	bool bAllComplete = true;
	for (const FMGEventObjective& Obj : Event.Objectives)
	{
		if (!Obj.bIsCompleted)
		{
			bAllComplete = false;
			break;
		}
	}

	if (bAllComplete)
	{
		Event.bIsCompleted = true;
		OnEventCompleted.Broadcast(Event);

		// Grant rewards
		for (const FMGSeasonReward& Reward : Event.Rewards)
		{
			// Would grant reward through progression system
		}
	}
}

void UMGSeasonSubsystem::GenerateMockSeason()
{
	CurrentSeason.SeasonID = FName("Season_1");
	CurrentSeason.SeasonNumber = 1;
	CurrentSeason.SeasonName = FText::FromString(TEXT("Midnight Rising"));
	CurrentSeason.SeasonTheme = FText::FromString(TEXT("The streets come alive after dark"));
	CurrentSeason.StartDate = FDateTime::Now() - FTimespan::FromDays(30);
	CurrentSeason.EndDate = FDateTime::Now() + FTimespan::FromDays(60);
	CurrentSeason.MaxTier = 100;
	CurrentSeason.XPPerTier = 1000;
	CurrentSeason.FeaturedVehicle = FName("NightRider_Turbo");
	CurrentSeason.FeaturedTrack = FName("Downtown_Circuit");

	// Generate rewards for each tier
	for (int32 Tier = 1; Tier <= 100; Tier++)
	{
		// Free track reward
		{
			FMGSeasonReward Reward;
			Reward.Tier = Tier;
			Reward.bIsPremium = false;

			if (Tier % 10 == 0)
			{
				// Major reward every 10 tiers
				Reward.Type = (Tier == 100) ? EMGRewardType::Vehicle : EMGRewardType::Customization;
				Reward.Quantity = 1;
				Reward.DisplayName = FText::FromString(FString::Printf(TEXT("Tier %d Exclusive"), Tier));
			}
			else if (Tier % 5 == 0)
			{
				// Medium reward every 5 tiers
				Reward.Type = EMGRewardType::Reputation;
				Reward.Quantity = 500;
				Reward.DisplayName = FText::FromString(TEXT("500 Rep"));
			}
			else
			{
				// Small reward
				Reward.Type = EMGRewardType::Cash;
				Reward.Quantity = 1000;
				Reward.DisplayName = FText::FromString(TEXT("$1000"));
			}

			CurrentSeason.Rewards.Add(Reward);
		}

		// Premium track reward
		{
			FMGSeasonReward Reward;
			Reward.Tier = Tier;
			Reward.bIsPremium = true;

			if (Tier % 10 == 0)
			{
				Reward.Type = EMGRewardType::Wrap;
				Reward.DisplayName = FText::FromString(FString::Printf(TEXT("Premium Wrap Tier %d"), Tier));
			}
			else if (Tier % 5 == 0)
			{
				Reward.Type = EMGRewardType::Cosmetic;
				Reward.DisplayName = FText::FromString(TEXT("Premium Emblem"));
			}
			else
			{
				Reward.Type = EMGRewardType::SeasonXP;
				Reward.Quantity = 200;
				Reward.DisplayName = FText::FromString(TEXT("+200 Season XP"));
			}

			CurrentSeason.Rewards.Add(Reward);
		}
	}
}

void UMGSeasonSubsystem::GenerateMockEvents()
{
	FDateTime Now = FDateTime::Now();

	// Weekly Challenge
	{
		FMGEventData Event;
		Event.EventID = FName("Weekly_SpeedDemon");
		Event.Type = EMGEventType::Weekly;
		Event.EventName = FText::FromString(TEXT("Speed Demon"));
		Event.Description = FText::FromString(TEXT("Push your limits! Complete high-speed challenges."));
		Event.StartTime = Now - FTimespan::FromDays(2);
		Event.EndTime = Now + FTimespan::FromDays(5);
		Event.bIsFeatured = true;
		Event.TotalParticipants = 15847;

		FMGEventObjective Obj1;
		Obj1.ObjectiveID = FName("Speed_1");
		Obj1.Description = FText::FromString(TEXT("Reach 200 km/h in 5 races"));
		Obj1.TargetValue = 5;
		Obj1.TrackedStat = FName("HighSpeedRaces");
		Event.Objectives.Add(Obj1);

		FMGEventObjective Obj2;
		Obj2.ObjectiveID = FName("Speed_2");
		Obj2.Description = FText::FromString(TEXT("Win a race with top speed bonus"));
		Obj2.TargetValue = 1;
		Obj2.TrackedStat = FName("TopSpeedWins");
		Event.Objectives.Add(Obj2);

		FMGSeasonReward Reward;
		Reward.Type = EMGRewardType::Cash;
		Reward.Quantity = 10000;
		Reward.DisplayName = FText::FromString(TEXT("$10,000"));
		Event.Rewards.Add(Reward);

		FMGSeasonReward Reward2;
		Reward2.Type = EMGRewardType::SeasonXP;
		Reward2.Quantity = 2000;
		Reward2.DisplayName = FText::FromString(TEXT("2000 Season XP"));
		Event.Rewards.Add(Reward2);

		ActiveEvents.Add(Event);
	}

	// Time Trial Event
	{
		FMGEventData Event;
		Event.EventID = FName("TimeTrial_Downtown");
		Event.Type = EMGEventType::TimeTrial;
		Event.EventName = FText::FromString(TEXT("Downtown Time Trial"));
		Event.Description = FText::FromString(TEXT("Set your best time on Downtown Circuit!"));
		Event.StartTime = Now - FTimespan::FromDays(1);
		Event.EndTime = Now + FTimespan::FromDays(3);
		Event.TotalParticipants = 8234;

		FMGEventObjective Obj;
		Obj.ObjectiveID = FName("TT_1");
		Obj.Description = FText::FromString(TEXT("Complete Downtown Circuit under 1:30"));
		Obj.TargetValue = 1;
		Obj.TrackedStat = FName("FastLap");
		Obj.RequiredTrack = FName("Downtown_Circuit");
		Event.Objectives.Add(Obj);

		FMGSeasonReward Reward;
		Reward.Type = EMGRewardType::Reputation;
		Reward.Quantity = 2500;
		Reward.DisplayName = FText::FromString(TEXT("2500 Rep"));
		Event.Rewards.Add(Reward);

		ActiveEvents.Add(Event);
	}

	// Upcoming Weekend Event
	{
		FMGEventData Event;
		Event.EventID = FName("Weekend_DriftFest");
		Event.Type = EMGEventType::Weekend;
		Event.EventName = FText::FromString(TEXT("Drift Fest"));
		Event.Description = FText::FromString(TEXT("Double drift XP all weekend!"));
		Event.StartTime = Now + FTimespan::FromDays(3);
		Event.EndTime = Now + FTimespan::FromDays(5);

		UpcomingEvents.Add(Event);
	}
}

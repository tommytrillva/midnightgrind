// MidnightGrind - Arcade Street Racing Game
// Live Event Subsystem - Implementation

#include "LiveEvent/MGLiveEventSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMGLiveEventSubsystem::UMGLiveEventSubsystem()
{
}

void UMGLiveEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize sample events for demonstration
    InitializeSampleEvents();

    // Start tick timer for event management
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            [this]()
            {
                TickEvents(1.0f);
            },
            1.0f,
            true
        );
    }
}

void UMGLiveEventSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }

    Super::Deinitialize();
}

void UMGLiveEventSubsystem::TickEvents(float DeltaTime)
{
    CheckEventTransitions();
}

// ===== Event Access =====

TArray<FMGLiveEvent> UMGLiveEventSubsystem::GetActiveEvents() const
{
    TArray<FMGLiveEvent> ActiveEvents;

    for (const auto& Pair : Events)
    {
        if (Pair.Value.Status == EMGEventStatus::Active || Pair.Value.Status == EMGEventStatus::Ending)
        {
            ActiveEvents.Add(Pair.Value);
        }
    }

    // Sort by end time
    ActiveEvents.Sort([](const FMGLiveEvent& A, const FMGLiveEvent& B)
    {
        return A.EndTime < B.EndTime;
    });

    return ActiveEvents;
}

TArray<FMGLiveEvent> UMGLiveEventSubsystem::GetUpcomingEvents() const
{
    TArray<FMGLiveEvent> UpcomingEvents;

    for (const auto& Pair : Events)
    {
        if (Pair.Value.Status == EMGEventStatus::Upcoming)
        {
            UpcomingEvents.Add(Pair.Value);
        }
    }

    // Sort by start time
    UpcomingEvents.Sort([](const FMGLiveEvent& A, const FMGLiveEvent& B)
    {
        return A.StartTime < B.StartTime;
    });

    return UpcomingEvents;
}

FMGLiveEvent UMGLiveEventSubsystem::GetEvent(const FString& EventId) const
{
    if (const FMGLiveEvent* Event = Events.Find(EventId))
    {
        return *Event;
    }
    return FMGLiveEvent();
}

FMGLiveEvent UMGLiveEventSubsystem::GetFeaturedEvent() const
{
    for (const auto& Pair : Events)
    {
        if (Pair.Value.bIsFeatured && Pair.Value.Status == EMGEventStatus::Active)
        {
            return Pair.Value;
        }
    }

    // Return first active event if no featured
    TArray<FMGLiveEvent> ActiveEvents = GetActiveEvents();
    if (ActiveEvents.Num() > 0)
    {
        return ActiveEvents[0];
    }

    return FMGLiveEvent();
}

FMGEventSchedule UMGLiveEventSubsystem::GetEventSchedule() const
{
    FMGEventSchedule Schedule;
    Schedule.ActiveEvents = GetActiveEvents();
    Schedule.UpcomingEvents = GetUpcomingEvents();
    Schedule.LastRefreshed = FDateTime::Now();

    for (const auto& Pair : Events)
    {
        if (Pair.Value.Status == EMGEventStatus::Completed)
        {
            Schedule.RecentlyEnded.Add(Pair.Value);
        }
    }

    return Schedule;
}

void UMGLiveEventSubsystem::RefreshEventSchedule()
{
    // In production, this would fetch from server
    CheckEventTransitions();
    OnEventScheduleRefreshed.Broadcast();
}

// ===== Participation =====

bool UMGLiveEventSubsystem::JoinEvent(const FString& EventId)
{
    if (!CanJoinEvent(EventId))
    {
        return false;
    }

    FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event)
    {
        return false;
    }

    Event->bHasJoined = true;
    JoinedEvents.AddUnique(EventId);

    OnEventJoined.Broadcast(EventId);
    return true;
}

bool UMGLiveEventSubsystem::HasJoinedEvent(const FString& EventId) const
{
    return JoinedEvents.Contains(EventId);
}

bool UMGLiveEventSubsystem::CanJoinEvent(const FString& EventId) const
{
    const FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event)
    {
        return false;
    }

    // Already joined
    if (Event->bHasJoined)
    {
        return false;
    }

    // Event must be active or upcoming
    if (Event->Status != EMGEventStatus::Active && Event->Status != EMGEventStatus::Upcoming)
    {
        return false;
    }

    return true;
}

TArray<FString> UMGLiveEventSubsystem::GetJoinedEventIds() const
{
    return JoinedEvents;
}

// ===== Progress =====

void UMGLiveEventSubsystem::UpdateObjectiveProgress(const FString& EventId, FName ObjectiveId, float Progress)
{
    FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event || !Event->bHasJoined)
    {
        return;
    }

    for (FMGEventObjective& Objective : Event->Objectives)
    {
        if (Objective.ObjectiveId == ObjectiveId)
        {
            float OldProgress = Objective.CurrentValue;
            Objective.CurrentValue = FMath::Min(Progress, Objective.TargetValue);

            OnObjectiveProgress.Broadcast(EventId, ObjectiveId, Objective.GetProgress());

            // Check completion
            if (!Objective.bIsComplete && Objective.CurrentValue >= Objective.TargetValue)
            {
                Objective.bIsComplete = true;
                Objective.CompletionCount++;
                AddEventScore(EventId, Objective.PointsAwarded);
                OnObjectiveCompleted.Broadcast(EventId, ObjectiveId);

                // Reset for repeatable objectives
                if (Objective.bIsRepeatable && Objective.CompletionCount < Objective.MaxCompletions)
                {
                    Objective.CurrentValue = 0.0f;
                    Objective.bIsComplete = false;
                }
            }
            break;
        }
    }
}

void UMGLiveEventSubsystem::AddEventScore(const FString& EventId, int32 Score)
{
    FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event || !Event->bHasJoined)
    {
        return;
    }

    EMGEventTier OldTier = Event->PlayerTier;
    Event->PlayerScore += Score;

    UpdateTierProgress(EventId);

    // Check tier advancement
    if (Event->PlayerTier != OldTier)
    {
        TArray<FMGEventReward> TierRewards;
        for (const FMGEventReward& Reward : Event->Rewards)
        {
            if (Reward.RequiredTier == Event->PlayerTier && !Reward.bIsClaimed)
            {
                TierRewards.Add(Reward);
            }
        }
        OnTierReached.Broadcast(EventId, Event->PlayerTier, TierRewards);
    }
}

int32 UMGLiveEventSubsystem::GetPlayerScore(const FString& EventId) const
{
    const FMGLiveEvent* Event = Events.Find(EventId);
    return Event ? Event->PlayerScore : 0;
}

int32 UMGLiveEventSubsystem::GetPlayerRank(const FString& EventId) const
{
    const FMGLiveEvent* Event = Events.Find(EventId);
    return Event ? Event->PlayerRank : 0;
}

EMGEventTier UMGLiveEventSubsystem::GetPlayerTier(const FString& EventId) const
{
    const FMGLiveEvent* Event = Events.Find(EventId);
    return Event ? Event->PlayerTier : EMGEventTier::Participation;
}

float UMGLiveEventSubsystem::GetEventProgress(const FString& EventId) const
{
    const FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event || Event->Objectives.Num() == 0)
    {
        return 0.0f;
    }

    float TotalProgress = 0.0f;
    for (const FMGEventObjective& Objective : Event->Objectives)
    {
        TotalProgress += Objective.GetProgress();
    }
    return TotalProgress / Event->Objectives.Num();
}

// ===== Community =====

void UMGLiveEventSubsystem::ContributeToCommunityGoal(const FString& EventId, float Contribution)
{
    FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event || Event->EventType != EMGEventType::CommunityGoal)
    {
        return;
    }

    int32 OldMilestone = Event->CommunityProgress.CurrentMilestone;
    Event->CommunityProgress.TotalProgress += Contribution;
    Event->CommunityProgress.LastUpdated = FDateTime::Now();

    // Check milestone progress
    for (int32 i = OldMilestone; i < Event->CommunityProgress.MilestoneThresholds.Num(); i++)
    {
        if (Event->CommunityProgress.TotalProgress >= Event->CommunityProgress.MilestoneThresholds[i])
        {
            Event->CommunityProgress.CurrentMilestone = i + 1;
            OnCommunityMilestone.Broadcast(EventId, i + 1);
        }
    }
}

FMGCommunityProgress UMGLiveEventSubsystem::GetCommunityProgress(const FString& EventId) const
{
    const FMGLiveEvent* Event = Events.Find(EventId);
    return Event ? Event->CommunityProgress : FMGCommunityProgress();
}

// ===== Leaderboard =====

void UMGLiveEventSubsystem::FetchEventLeaderboard(const FString& EventId, int32 Count, int32 Offset)
{
    // In production, this would fetch from server
    // For demo, generate sample leaderboard
    TArray<FMGEventLeaderboardEntry> Leaderboard;

    for (int32 i = Offset; i < Offset + Count; i++)
    {
        FMGEventLeaderboardEntry Entry;
        Entry.Rank = i + 1;
        Entry.PlayerId = FString::Printf(TEXT("PLAYER_%d"), i + 1);
        Entry.PlayerName = FString::Printf(TEXT("Racer_%d"), FMath::RandRange(1000, 9999));
        Entry.Score = FMath::Max(0, 100000 - (i * 250) + FMath::RandRange(-50, 50));
        Entry.AchievedTier = CalculateTierFromScore(Entry.Score);
        Entry.Platform = i % 3 == 0 ? TEXT("PC") : (i % 3 == 1 ? TEXT("PlayStation") : TEXT("Xbox"));
        Leaderboard.Add(Entry);
    }

    EventLeaderboards.Add(EventId, Leaderboard);
}

TArray<FMGEventLeaderboardEntry> UMGLiveEventSubsystem::GetEventLeaderboard(const FString& EventId) const
{
    const TArray<FMGEventLeaderboardEntry>* Leaderboard = EventLeaderboards.Find(EventId);
    return Leaderboard ? *Leaderboard : TArray<FMGEventLeaderboardEntry>();
}

// ===== Rewards =====

bool UMGLiveEventSubsystem::ClaimReward(const FString& EventId, FName RewardId)
{
    FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event)
    {
        return false;
    }

    for (FMGEventReward& Reward : Event->Rewards)
    {
        if (Reward.RewardId == RewardId)
        {
            if (Reward.bIsClaimed)
            {
                return false;
            }

            // Check tier requirement
            if (static_cast<uint8>(Event->PlayerTier) < static_cast<uint8>(Reward.RequiredTier))
            {
                return false;
            }

            // Check points requirement
            if (Event->PlayerScore < Reward.RequiredPoints)
            {
                return false;
            }

            Reward.bIsClaimed = true;
            OnRewardClaimed.Broadcast(EventId, Reward);
            return true;
        }
    }

    return false;
}

TArray<FMGEventReward> UMGLiveEventSubsystem::ClaimAllRewards(const FString& EventId)
{
    TArray<FMGEventReward> ClaimedRewards;
    TArray<FMGEventReward> Eligible = GetEligibleRewards(EventId);

    for (const FMGEventReward& Reward : Eligible)
    {
        if (ClaimReward(EventId, Reward.RewardId))
        {
            ClaimedRewards.Add(Reward);
        }
    }

    return ClaimedRewards;
}

TArray<FMGEventReward> UMGLiveEventSubsystem::GetUnclaimedRewards(const FString& EventId) const
{
    TArray<FMGEventReward> Unclaimed;
    const FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event)
    {
        return Unclaimed;
    }

    for (const FMGEventReward& Reward : Event->Rewards)
    {
        if (!Reward.bIsClaimed)
        {
            Unclaimed.Add(Reward);
        }
    }

    return Unclaimed;
}

TArray<FMGEventReward> UMGLiveEventSubsystem::GetEligibleRewards(const FString& EventId) const
{
    TArray<FMGEventReward> Eligible;
    const FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event)
    {
        return Eligible;
    }

    for (const FMGEventReward& Reward : Event->Rewards)
    {
        if (!Reward.bIsClaimed &&
            static_cast<uint8>(Event->PlayerTier) >= static_cast<uint8>(Reward.RequiredTier) &&
            Event->PlayerScore >= Reward.RequiredPoints)
        {
            Eligible.Add(Reward);
        }
    }

    return Eligible;
}

// ===== Protected =====

void UMGLiveEventSubsystem::CheckEventTransitions()
{
    FDateTime Now = FDateTime::Now();

    for (auto& Pair : Events)
    {
        FMGLiveEvent& Event = Pair.Value;
        EMGEventStatus OldStatus = Event.Status;

        // Upcoming -> Active
        if (Event.Status == EMGEventStatus::Upcoming && Now >= Event.StartTime)
        {
            Event.Status = EMGEventStatus::Active;
        }
        // Active -> Ending (last 10% of duration)
        else if (Event.Status == EMGEventStatus::Active)
        {
            FTimespan TotalDuration = Event.EndTime - Event.StartTime;
            FTimespan Remaining = Event.EndTime - Now;
            if (Remaining.GetTotalSeconds() < TotalDuration.GetTotalSeconds() * 0.1)
            {
                Event.Status = EMGEventStatus::Ending;
            }
        }
        // Ending/Active -> Completed
        if ((Event.Status == EMGEventStatus::Active || Event.Status == EMGEventStatus::Ending) && Now >= Event.EndTime)
        {
            Event.Status = EMGEventStatus::Completed;
        }

        // Broadcast state changes
        if (OldStatus != Event.Status)
        {
            if (Event.Status == EMGEventStatus::Active && OldStatus == EMGEventStatus::Upcoming)
            {
                OnEventStarted.Broadcast(Event);
            }
            else if (Event.Status == EMGEventStatus::Completed)
            {
                OnEventEnded.Broadcast(Event);
            }
        }
    }
}

void UMGLiveEventSubsystem::UpdateTierProgress(const FString& EventId)
{
    FMGLiveEvent* Event = Events.Find(EventId);
    if (!Event)
    {
        return;
    }

    Event->PlayerTier = CalculateTierFromScore(Event->PlayerScore);
}

EMGEventTier UMGLiveEventSubsystem::CalculateTierFromScore(int32 Score) const
{
    if (Score >= GetTierThreshold(EMGEventTier::Champion))
    {
        return EMGEventTier::Champion;
    }
    if (Score >= GetTierThreshold(EMGEventTier::Diamond))
    {
        return EMGEventTier::Diamond;
    }
    if (Score >= GetTierThreshold(EMGEventTier::Platinum))
    {
        return EMGEventTier::Platinum;
    }
    if (Score >= GetTierThreshold(EMGEventTier::Gold))
    {
        return EMGEventTier::Gold;
    }
    if (Score >= GetTierThreshold(EMGEventTier::Silver))
    {
        return EMGEventTier::Silver;
    }
    if (Score >= GetTierThreshold(EMGEventTier::Bronze))
    {
        return EMGEventTier::Bronze;
    }
    return EMGEventTier::Participation;
}

int32 UMGLiveEventSubsystem::GetTierThreshold(EMGEventTier Tier) const
{
    switch (Tier)
    {
    case EMGEventTier::Participation:
        return 0;
    case EMGEventTier::Bronze:
        return 1000;
    case EMGEventTier::Silver:
        return 5000;
    case EMGEventTier::Gold:
        return 15000;
    case EMGEventTier::Platinum:
        return 35000;
    case EMGEventTier::Diamond:
        return 60000;
    case EMGEventTier::Champion:
        return 100000;
    default:
        return 0;
    }
}

void UMGLiveEventSubsystem::InitializeSampleEvents()
{
    FDateTime Now = FDateTime::Now();

    // Weekend Drift Challenge - Active
    {
        FMGLiveEvent DriftEvent;
        DriftEvent.EventId = TEXT("event_drift_weekend_001");
        DriftEvent.EventName = FText::FromString(TEXT("Weekend Drift Challenge"));
        DriftEvent.Description = FText::FromString(TEXT("Show off your drifting skills! Score points by performing sick drifts on Downtown Circuit."));
        DriftEvent.EventType = EMGEventType::DriftChallenge;
        DriftEvent.Status = EMGEventStatus::Active;
        DriftEvent.StartTime = Now - FTimespan::FromDays(1);
        DriftEvent.EndTime = Now + FTimespan::FromDays(2);
        DriftEvent.bIsFeatured = true;
        DriftEvent.MinLevel = 5;
        DriftEvent.RequiredTrack = FName("Downtown_Circuit");

        FMGEventObjective DriftObj1;
        DriftObj1.ObjectiveId = FName("drift_distance");
        DriftObj1.Description = FText::FromString(TEXT("Total drift distance: 50,000m"));
        DriftObj1.TargetValue = 50000.0f;
        DriftObj1.PointsAwarded = 500;
        DriftEvent.Objectives.Add(DriftObj1);

        FMGEventObjective DriftObj2;
        DriftObj2.ObjectiveId = FName("perfect_drifts");
        DriftObj2.Description = FText::FromString(TEXT("Perfect drifts: 100"));
        DriftObj2.TargetValue = 100.0f;
        DriftObj2.PointsAwarded = 750;
        DriftEvent.Objectives.Add(DriftObj2);

        FMGEventObjective DriftObj3;
        DriftObj3.ObjectiveId = FName("drift_chains");
        DriftObj3.Description = FText::FromString(TEXT("Chain 10+ drifts: 25 times"));
        DriftObj3.TargetValue = 25.0f;
        DriftObj3.PointsAwarded = 1000;
        DriftEvent.Objectives.Add(DriftObj3);

        FMGEventReward BronzeReward;
        BronzeReward.RewardId = FName("drift_bronze");
        BronzeReward.DisplayName = FText::FromString(TEXT("Drift Rookie Decal"));
        BronzeReward.RequiredTier = EMGEventTier::Bronze;
        BronzeReward.RequiredPoints = 1000;
        BronzeReward.UnlockType = FName("Decal");
        BronzeReward.Quantity = 1;
        DriftEvent.Rewards.Add(BronzeReward);

        FMGEventReward GoldReward;
        GoldReward.RewardId = FName("drift_gold");
        GoldReward.DisplayName = FText::FromString(TEXT("Midnight Drift King Vinyl"));
        GoldReward.RequiredTier = EMGEventTier::Gold;
        GoldReward.RequiredPoints = 15000;
        GoldReward.UnlockType = FName("Vinyl");
        GoldReward.Quantity = 1;
        GoldReward.bIsExclusive = true;
        DriftEvent.Rewards.Add(GoldReward);

        FMGEventReward ChampionReward;
        ChampionReward.RewardId = FName("drift_champion");
        ChampionReward.DisplayName = FText::FromString(TEXT("Legendary Drift Spoiler"));
        ChampionReward.RequiredTier = EMGEventTier::Champion;
        ChampionReward.RequiredPoints = 100000;
        ChampionReward.UnlockType = FName("Spoiler");
        ChampionReward.Quantity = 1;
        ChampionReward.bIsExclusive = true;
        DriftEvent.Rewards.Add(ChampionReward);

        Events.Add(DriftEvent.EventId, DriftEvent);
    }

    // Community Speed Goal - Active
    {
        FMGLiveEvent CommunityEvent;
        CommunityEvent.EventId = TEXT("event_community_speed_001");
        CommunityEvent.EventName = FText::FromString(TEXT("Community Speed Rush"));
        CommunityEvent.Description = FText::FromString(TEXT("Together we go faster! Help the community reach a combined 10 million miles driven."));
        CommunityEvent.EventType = EMGEventType::CommunityGoal;
        CommunityEvent.Status = EMGEventStatus::Active;
        CommunityEvent.StartTime = Now - FTimespan::FromDays(3);
        CommunityEvent.EndTime = Now + FTimespan::FromDays(4);
        CommunityEvent.MinLevel = 1;

        CommunityEvent.CommunityProgress.GoalTarget = 10000000.0f;
        CommunityEvent.CommunityProgress.TotalProgress = 4500000.0f;
        CommunityEvent.CommunityProgress.ParticipantCount = 15234;
        CommunityEvent.CommunityProgress.MilestoneThresholds = { 1000000.0f, 2500000.0f, 5000000.0f, 7500000.0f, 10000000.0f };
        CommunityEvent.CommunityProgress.CurrentMilestone = 2;

        FMGEventObjective DriveObj;
        DriveObj.ObjectiveId = FName("miles_contributed");
        DriveObj.Description = FText::FromString(TEXT("Contribute miles to community goal"));
        DriveObj.TargetValue = 100.0f;
        DriveObj.PointsAwarded = 50;
        DriveObj.bIsRepeatable = true;
        DriveObj.MaxCompletions = 100;
        CommunityEvent.Objectives.Add(DriveObj);

        FMGEventReward ParticipationReward;
        ParticipationReward.RewardId = FName("community_participation");
        ParticipationReward.DisplayName = FText::FromString(TEXT("Speed Rush Participant Badge"));
        ParticipationReward.RequiredTier = EMGEventTier::Participation;
        ParticipationReward.UnlockType = FName("Badge");
        ParticipationReward.Quantity = 1;
        CommunityEvent.Rewards.Add(ParticipationReward);

        FMGEventReward MilestoneReward;
        MilestoneReward.RewardId = FName("community_milestone_5");
        MilestoneReward.DisplayName = FText::FromString(TEXT("Speed Rush Champion Body Kit"));
        MilestoneReward.Description = FText::FromString(TEXT("Awarded when community reaches 10 million miles"));
        MilestoneReward.RequiredTier = EMGEventTier::Bronze;
        MilestoneReward.UnlockType = FName("BodyKit");
        MilestoneReward.Quantity = 1;
        MilestoneReward.bIsExclusive = true;
        CommunityEvent.Rewards.Add(MilestoneReward);

        Events.Add(CommunityEvent.EventId, CommunityEvent);
    }

    // Time Attack Tournament - Upcoming
    {
        FMGLiveEvent TournamentEvent;
        TournamentEvent.EventId = TEXT("event_tournament_timeattack_001");
        TournamentEvent.EventName = FText::FromString(TEXT("Tokyo Express Time Attack"));
        TournamentEvent.Description = FText::FromString(TEXT("Race against the clock on Tokyo Express. Top 100 racers win exclusive rewards!"));
        TournamentEvent.EventType = EMGEventType::Tournament;
        TournamentEvent.Status = EMGEventStatus::Upcoming;
        TournamentEvent.StartTime = Now + FTimespan::FromDays(3);
        TournamentEvent.EndTime = Now + FTimespan::FromDays(10);
        TournamentEvent.MinLevel = 10;
        TournamentEvent.RequiredTrack = FName("Tokyo_Express");

        FMGEventObjective TimeObj1;
        TimeObj1.ObjectiveId = FName("best_lap");
        TimeObj1.Description = FText::FromString(TEXT("Set your best lap time"));
        TimeObj1.TargetValue = 1.0f;
        TimeObj1.PointsAwarded = 1000;
        TournamentEvent.Objectives.Add(TimeObj1);

        FMGEventObjective TimeObj2;
        TimeObj2.ObjectiveId = FName("total_laps");
        TimeObj2.Description = FText::FromString(TEXT("Complete 50 laps"));
        TimeObj2.TargetValue = 50.0f;
        TimeObj2.PointsAwarded = 500;
        TournamentEvent.Objectives.Add(TimeObj2);

        FMGEventReward Top100Reward;
        Top100Reward.RewardId = FName("tournament_top100");
        Top100Reward.DisplayName = FText::FromString(TEXT("Tokyo Express Champion Wheels"));
        Top100Reward.RequiredTier = EMGEventTier::Champion;
        Top100Reward.RequiredPoints = 50000;
        Top100Reward.UnlockType = FName("Wheels");
        Top100Reward.Quantity = 1;
        Top100Reward.bIsExclusive = true;
        TournamentEvent.Rewards.Add(Top100Reward);

        Events.Add(TournamentEvent.EventId, TournamentEvent);
    }

    // Holiday Event - Active
    {
        FMGLiveEvent HolidayEvent;
        HolidayEvent.EventId = TEXT("event_holiday_neon_001");
        HolidayEvent.EventName = FText::FromString(TEXT("Neon Nights Festival"));
        HolidayEvent.Description = FText::FromString(TEXT("The city comes alive with neon lights! Complete special challenges to unlock limited festival gear."));
        HolidayEvent.EventType = EMGEventType::HolidayEvent;
        HolidayEvent.Status = EMGEventStatus::Active;
        HolidayEvent.StartTime = Now - FTimespan::FromDays(5);
        HolidayEvent.EndTime = Now + FTimespan::FromDays(9);
        HolidayEvent.MinLevel = 1;

        FMGEventObjective FestObj1;
        FestObj1.ObjectiveId = FName("neon_races");
        FestObj1.Description = FText::FromString(TEXT("Complete 20 night races"));
        FestObj1.TargetValue = 20.0f;
        FestObj1.PointsAwarded = 300;
        HolidayEvent.Objectives.Add(FestObj1);

        FMGEventObjective FestObj2;
        FestObj2.ObjectiveId = FName("neon_photos");
        FestObj2.Description = FText::FromString(TEXT("Take 10 photos with neon effects"));
        FestObj2.TargetValue = 10.0f;
        FestObj2.PointsAwarded = 200;
        HolidayEvent.Objectives.Add(FestObj2);

        FMGEventObjective FestObj3;
        FestObj3.ObjectiveId = FName("neon_crew");
        FestObj3.Description = FText::FromString(TEXT("Race with crew members 15 times"));
        FestObj3.TargetValue = 15.0f;
        FestObj3.PointsAwarded = 400;
        HolidayEvent.Objectives.Add(FestObj3);

        FMGEventReward NeonReward1;
        NeonReward1.RewardId = FName("neon_underglow");
        NeonReward1.DisplayName = FText::FromString(TEXT("Festival Neon Underglow"));
        NeonReward1.RequiredTier = EMGEventTier::Silver;
        NeonReward1.RequiredPoints = 5000;
        NeonReward1.UnlockType = FName("Underglow");
        NeonReward1.Quantity = 1;
        NeonReward1.bIsExclusive = true;
        HolidayEvent.Rewards.Add(NeonReward1);

        FMGEventReward NeonReward2;
        NeonReward2.RewardId = FName("neon_suit");
        NeonReward2.DisplayName = FText::FromString(TEXT("Neon Rider Suit"));
        NeonReward2.RequiredTier = EMGEventTier::Platinum;
        NeonReward2.RequiredPoints = 35000;
        NeonReward2.UnlockType = FName("Outfit");
        NeonReward2.Quantity = 1;
        NeonReward2.bIsExclusive = true;
        HolidayEvent.Rewards.Add(NeonReward2);

        Events.Add(HolidayEvent.EventId, HolidayEvent);
    }

    // Brand Collaboration - Ending Soon
    {
        FMGLiveEvent BrandEvent;
        BrandEvent.EventId = TEXT("event_brand_turbo_001");
        BrandEvent.EventName = FText::FromString(TEXT("TURBO Energy Challenge"));
        BrandEvent.Description = FText::FromString(TEXT("Sponsored by TURBO Energy Drinks! Race hard, stay energized, win exclusive TURBO gear."));
        BrandEvent.EventType = EMGEventType::BrandCollaboration;
        BrandEvent.Status = EMGEventStatus::Ending;
        BrandEvent.StartTime = Now - FTimespan::FromDays(12);
        BrandEvent.EndTime = Now + FTimespan::FromHours(18);
        BrandEvent.MinLevel = 3;

        FMGEventObjective BrandObj1;
        BrandObj1.ObjectiveId = FName("turbo_wins");
        BrandObj1.Description = FText::FromString(TEXT("Win 10 races"));
        BrandObj1.TargetValue = 10.0f;
        BrandObj1.PointsAwarded = 500;
        BrandEvent.Objectives.Add(BrandObj1);

        FMGEventObjective BrandObj2;
        BrandObj2.ObjectiveId = FName("turbo_speed");
        BrandObj2.Description = FText::FromString(TEXT("Reach 200 MPH 25 times"));
        BrandObj2.TargetValue = 25.0f;
        BrandObj2.PointsAwarded = 400;
        BrandEvent.Objectives.Add(BrandObj2);

        FMGEventReward TurboReward;
        TurboReward.RewardId = FName("turbo_livery");
        TurboReward.DisplayName = FText::FromString(TEXT("TURBO Energy Livery"));
        TurboReward.RequiredTier = EMGEventTier::Gold;
        TurboReward.RequiredPoints = 15000;
        TurboReward.UnlockType = FName("Livery");
        TurboReward.Quantity = 1;
        TurboReward.bIsExclusive = true;
        BrandEvent.Rewards.Add(TurboReward);

        Events.Add(BrandEvent.EventId, BrandEvent);
    }
}

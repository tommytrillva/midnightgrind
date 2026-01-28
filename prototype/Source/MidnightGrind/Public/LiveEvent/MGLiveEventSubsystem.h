// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGLiveEventSubsystem.h
 * @brief Live Events System - Time-limited special events with challenges, rewards, and leaderboards
 * @author Midnight Grind Team
 * @version 1.0
 *
 * @section overview Overview
 * ============================================================================
 * MGLiveEventSubsystem.h
 * Midnight Grind - Live Event and Time-Limited Content System
 * ============================================================================
 *
 * This subsystem manages time-limited special events that appear in-game with
 * unique challenges, rewards, and leaderboards. Think of it like Fortnite's
 * live events or Destiny's seasonal activities.
 *
 * @section difference Difference from Event Calendar
 * - EventCalendar: Schedules WHEN things happen (like a TV guide)
 * - LiveEvents: Manages the CONTENT and PROGRESS of active events
 *
 * In practice, EventCalendar might schedule a "Double XP Weekend" (simple bonus),
 * while LiveEvents handles a "Drift King Challenge" with objectives, tiers,
 * leaderboards, and exclusive rewards.
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection eventtypes 1. Event Types (EMGEventType)
 * Different flavors of live events:
 * - RacingChallenge: Win races, earn points
 * - DriftChallenge: Accumulate drift score
 * - TimeAttack: Beat target times on tracks
 * - CommunityGoal: Everyone contributes to shared progress
 * - Tournament: Bracketed competitive event
 * - HolidayEvent: Seasonal themed content (Halloween, etc.)
 * - BrandCollaboration: Sponsored content (real car brands)
 * - CreatorEvent: Content creator partnership events
 *
 * @subsection objectives 2. Objectives (FMGEventObjective)
 * Tasks players complete for event points:
 * - "Win 5 races" (TargetValue: 5)
 * - "Accumulate 1,000,000 drift score" (TargetValue: 1000000)
 * - "Beat the target time on Midnight Circuit" (TargetValue: 1, time-based)
 *
 * Objectives can be repeatable (bIsRepeatable) for farming points.
 * UpdateObjectiveProgress() tracks completion.
 *
 * @subsection tiers 3. Tier System (EMGEventTier)
 * Points earned unlock reward tiers:
 * Participation -> Bronze -> Silver -> Gold -> Platinum -> Diamond -> Champion
 *
 * Higher tiers require more points but give better rewards.
 * OnTierReached fires when player advances to a new tier.
 *
 * @subsection rewards 4. Rewards (FMGEventReward)
 * What players earn from events:
 * - Currency (in-game money)
 * - Premium Currency (real-money equivalent)
 * - Exclusive Items (vehicles, cosmetics) - often tied to high tiers
 * - bIsExclusive: Only available during this event
 *
 * Players must manually claim rewards via ClaimReward().
 *
 * @subsection community 5. Community Goals (FMGCommunityProgress)
 * Server-wide collaborative challenges:
 * - "Community: Drift 1 billion total meters"
 * - Everyone's progress combines toward the goal
 * - MilestoneThresholds unlock community-wide bonuses
 * - OnCommunityMilestone fires when thresholds are reached
 *
 * @subsection leaderboards 6. Leaderboards (FMGEventLeaderboardEntry)
 * Per-event rankings:
 * - FetchEventLeaderboard() requests data from server
 * - GetEventLeaderboard() returns cached results
 * - Shows rank, player name, score, achieved tier
 *
 * @section usage Usage Examples
 *
 * @code
 * // Get the subsystem
 * UMGLiveEventSubsystem* LiveEvents = GetGameInstance()->GetSubsystem<UMGLiveEventSubsystem>();
 *
 * // For Event UI - Display available events
 * TArray<FMGLiveEvent> ActiveEvents = LiveEvents->GetActiveEvents();
 * FMGLiveEvent Featured = LiveEvents->GetFeaturedEvent();
 * TArray<FMGLiveEvent> Upcoming = LiveEvents->GetUpcomingEvents();
 *
 * // Join an event and track progress
 * LiveEvents->JoinEvent("drift_challenge_2024");
 * float Progress = LiveEvents->GetEventProgress("drift_challenge_2024");
 *
 * // After player completes a drift, update the objective
 * LiveEvents->UpdateObjectiveProgress("drift_challenge_2024", FName("DriftScore"), 50000.0f);
 *
 * // Check and claim rewards
 * TArray<FMGEventReward> Unclaimed = LiveEvents->GetUnclaimedRewards("drift_challenge_2024");
 * LiveEvents->ClaimAllRewards("drift_challenge_2024");
 *
 * // Get player's current tier
 * EMGEventTier CurrentTier = LiveEvents->GetPlayerTier("drift_challenge_2024");
 *
 * // Contribute to community goals
 * LiveEvents->ContributeToCommunityGoal("community_drift", 1000.0f);
 *
 * // Listen for events
 * LiveEvents->OnTierReached.AddDynamic(this, &UMyClass::HandleTierReached);
 * LiveEvents->OnEventEnded.AddDynamic(this, &UMyClass::HandleEventEnded);
 * @endcode
 *
 * @section lifecycle Event Lifecycle
 * 1. Event appears in GetUpcomingEvents() with future StartTime
 * 2. At StartTime, moves to GetActiveEvents(), OnEventStarted fires
 * 3. Players JoinEvent() to participate
 * 4. Players complete objectives, earn points, climb tiers
 * 5. At EndTime, OnEventEnded fires
 * 6. Players can still claim unclaimed rewards for a grace period
 *
 * @section delegates Available Delegates
 * - OnEventStarted: Event becomes active
 * - OnEventEnded: Event ends
 * - OnEventJoined: Player joins an event
 * - OnObjectiveProgress: Objective progress updated
 * - OnObjectiveCompleted: Objective fully completed
 * - OnTierReached: Player advances to new tier
 * - OnCommunityMilestone: Community goal reaches threshold
 * - OnRewardClaimed: Player claims a reward
 * - OnEventScheduleRefreshed: Event list updated from server
 *
 * @see UMGEventCalendarSubsystem For scheduling when events occur
 * @see UMGRewardsSubsystem For general reward distribution
 * ============================================================================
 */

// MidnightGrind - Arcade Street Racing Game
// Live Event Subsystem - Live in-game events, special challenges, time-limited content

/**
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file implements the Live Events system - time-limited special events
 * that appear in-game with unique challenges, rewards, and leaderboards.
 * Think of it like Fortnite's live events or Destiny's seasonal activities.
 *
 * DIFFERENCE FROM EVENT CALENDAR:
 * - EventCalendar: Schedules WHEN things happen (like a TV guide)
 * - LiveEvents: Manages the CONTENT and PROGRESS of active events
 *
 * In practice, EventCalendar might schedule a "Double XP Weekend" (simple bonus),
 * while LiveEvents handles a "Drift King Challenge" with objectives, tiers,
 * leaderboards, and exclusive rewards.
 *
 * KEY CONCEPTS:
 *
 * 1. EVENT TYPES (EMGEventType)
 *    Different flavors of live events:
 *    - RacingChallenge: Win races, earn points
 *    - DriftChallenge: Accumulate drift score
 *    - TimeAttack: Beat target times on tracks
 *    - CommunityGoal: Everyone contributes to shared progress
 *    - Tournament: Bracketed competitive event
 *    - HolidayEvent: Seasonal themed content (Halloween, etc.)
 *    - BrandCollaboration: Sponsored content (real car brands)
 *    - CreatorEvent: Content creator partnership events
 *
 * 2. OBJECTIVES (FMGEventObjective)
 *    Tasks players complete for event points:
 *    - "Win 5 races" (TargetValue: 5)
 *    - "Accumulate 1,000,000 drift score" (TargetValue: 1000000)
 *    - "Beat the target time on Midnight Circuit" (TargetValue: 1, time-based)
 *
 *    Objectives can be repeatable (bIsRepeatable) for farming points.
 *    UpdateObjectiveProgress() tracks completion.
 *
 * 3. TIER SYSTEM (EMGEventTier)
 *    Points earned unlock reward tiers:
 *    Participation -> Bronze -> Silver -> Gold -> Platinum -> Diamond -> Champion
 *
 *    Higher tiers require more points but give better rewards.
 *    OnTierReached fires when player advances to a new tier.
 *
 * 4. REWARDS (FMGEventReward)
 *    What players earn from events:
 *    - Currency (in-game money)
 *    - Premium Currency (real-money equivalent)
 *    - Exclusive Items (vehicles, cosmetics) - often tied to high tiers
 *    - bIsExclusive: Only available during this event
 *
 *    Players must manually claim rewards via ClaimReward().
 *
 * 5. COMMUNITY GOALS (FMGCommunityProgress)
 *    Server-wide collaborative challenges:
 *    - "Community: Drift 1 billion total meters"
 *    - Everyone's progress combines toward the goal
 *    - MilestoneThresholds unlock community-wide bonuses
 *    - OnCommunityMilestone fires when thresholds are reached
 *
 * 6. LEADERBOARDS (FMGEventLeaderboardEntry)
 *    Per-event rankings:
 *    - FetchEventLeaderboard() requests data from server
 *    - GetEventLeaderboard() returns cached results
 *    - Shows rank, player name, score, achieved tier
 *
 * COMMON USE CASES:
 *
 * For Event UI:
 * - GetActiveEvents() for the events hub
 * - GetFeaturedEvent() for the main menu spotlight
 * - GetUpcomingEvents() for the "coming soon" section
 * - GetEventProgress() for progress bars
 * - GetUnclaimedRewards() for the rewards screen
 *
 * For Gameplay Integration:
 * - JoinEvent() when player starts participating
 * - UpdateObjectiveProgress() after relevant actions (race win, drift, etc.)
 * - AddEventScore() for direct point additions
 * - ContributeToCommunityGoal() for community events
 *
 * For Progression:
 * - GetPlayerTier() to show current tier badge
 * - GetEligibleRewards() to show what can be claimed
 * - ClaimAllRewards() when player visits reward screen
 *
 * EVENT LIFECYCLE:
 * 1. Event appears in GetUpcomingEvents() with future StartTime
 * 2. At StartTime, moves to GetActiveEvents(), OnEventStarted fires
 * 3. Players JoinEvent() to participate
 * 4. Players complete objectives, earn points, climb tiers
 * 5. At EndTime, OnEventEnded fires
 * 6. Players can still claim unclaimed rewards for a grace period
 *
 * DELEGATES:
 * - OnEventStarted: Event becomes active
 * - OnEventEnded: Event ends
 * - OnEventJoined: Player joins an event
 * - OnObjectiveProgress: Objective progress updated
 * - OnObjectiveCompleted: Objective fully completed
 * - OnTierReached: Player advances to new tier
 * - OnCommunityMilestone: Community goal reaches threshold
 * - OnRewardClaimed: Player claims a reward
 * - OnEventScheduleRefreshed: Event list updated from server
 *
 * ============================================================================
 */

// MidnightGrind - Arcade Street Racing Game
// Live Event Subsystem - Live in-game events, special challenges, time-limited content

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLiveEventSubsystem.generated.h"

// Forward declarations
class UMGLiveEventSubsystem;

/**
 * EMGEventType - Types of live events
 */
UENUM(BlueprintType)
enum class EMGEventType : uint8
{
    RacingChallenge     UMETA(DisplayName = "Racing Challenge"),
    DriftChallenge      UMETA(DisplayName = "Drift Challenge"),
    TimeAttack          UMETA(DisplayName = "Time Attack"),
    CommunityGoal       UMETA(DisplayName = "Community Goal"),
    Tournament          UMETA(DisplayName = "Tournament"),
    SpecialRace         UMETA(DisplayName = "Special Race"),
    HolidayEvent        UMETA(DisplayName = "Holiday Event"),
    BrandCollaboration  UMETA(DisplayName = "Brand Collaboration"),
    CreatorEvent        UMETA(DisplayName = "Creator Event"),
    MilestoneEvent      UMETA(DisplayName = "Milestone Event")
};

/**
 * EMGEventStatus - Status of a live event
 */
UENUM(BlueprintType)
enum class EMGEventStatus : uint8
{
    Upcoming        UMETA(DisplayName = "Upcoming"),
    Active          UMETA(DisplayName = "Active"),
    Ending          UMETA(DisplayName = "Ending Soon"),
    Completed       UMETA(DisplayName = "Completed"),
    Cancelled       UMETA(DisplayName = "Cancelled")
};

/**
 * EMGEventTier - Participation tiers for events
 */
UENUM(BlueprintType)
enum class EMGEventTier : uint8
{
    Participation   UMETA(DisplayName = "Participation"),
    Bronze          UMETA(DisplayName = "Bronze"),
    Silver          UMETA(DisplayName = "Silver"),
    Gold            UMETA(DisplayName = "Gold"),
    Platinum        UMETA(DisplayName = "Platinum"),
    Diamond         UMETA(DisplayName = "Diamond"),
    Champion        UMETA(DisplayName = "Champion")
};

/**
 * FMGEventReward - Reward for event participation
 */
USTRUCT(BlueprintType)
struct FMGEventReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RewardId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGEventTier RequiredTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName UnlockType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> IconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsExclusive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsClaimed;

    FMGEventReward()
        : RewardId(NAME_None)
        , RequiredTier(EMGEventTier::Participation)
        , RequiredPoints(0)
        , UnlockType(FName("Currency"))
        , Quantity(100)
        , bIsExclusive(false)
        , bIsClaimed(false)
    {}
};

/**
 * FMGEventObjective - An objective within an event
 */
USTRUCT(BlueprintType)
struct FMGEventObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ObjectiveId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PointsAwarded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxCompletions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CompletionCount;

    FMGEventObjective()
        : ObjectiveId(NAME_None)
        , TargetValue(1.0f)
        , CurrentValue(0.0f)
        , PointsAwarded(100)
        , bIsComplete(false)
        , bIsRepeatable(false)
        , MaxCompletions(1)
        , CompletionCount(0)
    {}

    float GetProgress() const
    {
        return TargetValue > 0.0f ? FMath::Clamp(CurrentValue / TargetValue, 0.0f, 1.0f) : 0.0f;
    }
};

/**
 * FMGCommunityProgress - Community-wide progress tracking
 */
USTRUCT(BlueprintType)
struct FMGCommunityProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GoalTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ParticipantCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> MilestoneThresholds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastUpdated;

    FMGCommunityProgress()
        : TotalProgress(0.0f)
        , GoalTarget(1000000.0f)
        , ParticipantCount(0)
        , CurrentMilestone(0)
    {}

    float GetProgressPercent() const
    {
        return GoalTarget > 0.0f ? TotalProgress / GoalTarget : 0.0f;
    }
};

/**
 * FMGEventLeaderboardEntry - Leaderboard entry for event
 */
USTRUCT(BlueprintType)
struct FMGEventLeaderboardEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Rank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Score;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGEventTier AchievedTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Platform;

    FMGEventLeaderboardEntry()
        : Rank(0)
        , Score(0)
        , AchievedTier(EMGEventTier::Participation)
    {}
};

/**
 * FMGLiveEvent - Complete live event data
 */
USTRUCT(BlueprintType)
struct FMGLiveEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGEventStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGEventObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGEventReward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGCommunityProgress CommunityProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PlayerScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PlayerRank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGEventTier PlayerTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> BannerTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredVehicle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFeatured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasJoined;

    FMGLiveEvent()
        : EventType(EMGEventType::RacingChallenge)
        , Status(EMGEventStatus::Upcoming)
        , PlayerScore(0)
        , PlayerRank(0)
        , PlayerTier(EMGEventTier::Participation)
        , RequiredTrack(NAME_None)
        , RequiredVehicle(NAME_None)
        , MinLevel(1)
        , bIsFeatured(false)
        , bHasJoined(false)
    {}

    FTimespan GetTimeRemaining() const
    {
        FDateTime Now = FDateTime::Now();
        return EndTime > Now ? EndTime - Now : FTimespan::Zero();
    }

    FTimespan GetTimeUntilStart() const
    {
        FDateTime Now = FDateTime::Now();
        return StartTime > Now ? StartTime - Now : FTimespan::Zero();
    }

    bool IsActive() const
    {
        FDateTime Now = FDateTime::Now();
        return Now >= StartTime && Now <= EndTime;
    }
};

/**
 * FMGEventSchedule - Scheduled events
 */
USTRUCT(BlueprintType)
struct FMGEventSchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGLiveEvent> UpcomingEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGLiveEvent> ActiveEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGLiveEvent> RecentlyEnded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastRefreshed;

    FMGEventSchedule()
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnEventStarted, const FMGLiveEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnEventEnded, const FMGLiveEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnEventJoined, const FString&, EventId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMGOnObjectiveProgress, const FString&, EventId, FName, ObjectiveId, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnObjectiveCompleted, const FString&, EventId, FName, ObjectiveId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMGOnTierReached, const FString&, EventId, EMGEventTier, NewTier, const TArray<FMGEventReward>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCommunityMilestone, const FString&, EventId, int32, MilestoneIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRewardClaimed, const FString&, EventId, const FMGEventReward&, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnEventScheduleRefreshed);

/**
 * UMGLiveEventSubsystem
 *
 * Manages live events for Midnight Grind.
 * Features include:
 * - Time-limited events
 * - Community goals
 * - Event objectives
 * - Tier-based rewards
 * - Event leaderboards
 * - Scheduled events
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLiveEventSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGLiveEventSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void TickEvents(float DeltaTime);

    // ===== Event Access =====

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Access")
    TArray<FMGLiveEvent> GetActiveEvents() const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Access")
    TArray<FMGLiveEvent> GetUpcomingEvents() const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Access")
    FMGLiveEvent GetEvent(const FString& EventId) const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Access")
    FMGLiveEvent GetFeaturedEvent() const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Access")
    FMGEventSchedule GetEventSchedule() const;

    UFUNCTION(BlueprintCallable, Category = "LiveEvent|Access")
    void RefreshEventSchedule();

    // ===== Participation =====

    UFUNCTION(BlueprintCallable, Category = "LiveEvent|Participation")
    bool JoinEvent(const FString& EventId);

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Participation")
    bool HasJoinedEvent(const FString& EventId) const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Participation")
    bool CanJoinEvent(const FString& EventId) const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Participation")
    TArray<FString> GetJoinedEventIds() const;

    // ===== Progress =====

    UFUNCTION(BlueprintCallable, Category = "LiveEvent|Progress")
    void UpdateObjectiveProgress(const FString& EventId, FName ObjectiveId, float Progress);

    UFUNCTION(BlueprintCallable, Category = "LiveEvent|Progress")
    void AddEventScore(const FString& EventId, int32 Score);

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Progress")
    int32 GetPlayerScore(const FString& EventId) const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Progress")
    int32 GetPlayerRank(const FString& EventId) const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Progress")
    EMGEventTier GetPlayerTier(const FString& EventId) const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Progress")
    float GetEventProgress(const FString& EventId) const;

    // ===== Community =====

    UFUNCTION(BlueprintCallable, Category = "LiveEvent|Community")
    void ContributeToCommunityGoal(const FString& EventId, float Contribution);

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Community")
    FMGCommunityProgress GetCommunityProgress(const FString& EventId) const;

    // ===== Leaderboard =====

    UFUNCTION(BlueprintCallable, Category = "LiveEvent|Leaderboard")
    void FetchEventLeaderboard(const FString& EventId, int32 Count, int32 Offset);

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Leaderboard")
    TArray<FMGEventLeaderboardEntry> GetEventLeaderboard(const FString& EventId) const;

    // ===== Rewards =====

    UFUNCTION(BlueprintCallable, Category = "LiveEvent|Rewards")
    bool ClaimReward(const FString& EventId, FName RewardId);

    UFUNCTION(BlueprintCallable, Category = "LiveEvent|Rewards")
    TArray<FMGEventReward> ClaimAllRewards(const FString& EventId);

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Rewards")
    TArray<FMGEventReward> GetUnclaimedRewards(const FString& EventId) const;

    UFUNCTION(BlueprintPure, Category = "LiveEvent|Rewards")
    TArray<FMGEventReward> GetEligibleRewards(const FString& EventId) const;

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnEventStarted OnEventStarted;

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnEventEnded OnEventEnded;

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnEventJoined OnEventJoined;

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnObjectiveProgress OnObjectiveProgress;

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnTierReached OnTierReached;

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnCommunityMilestone OnCommunityMilestone;

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnRewardClaimed OnRewardClaimed;

    UPROPERTY(BlueprintAssignable, Category = "LiveEvent|Events")
    FMGOnEventScheduleRefreshed OnEventScheduleRefreshed;

protected:
    void CheckEventTransitions();
    void UpdateTierProgress(const FString& EventId);
    EMGEventTier CalculateTierFromScore(int32 Score) const;
    int32 GetTierThreshold(EMGEventTier Tier) const;
    void InitializeSampleEvents();

private:
    UPROPERTY()
    TMap<FString, FMGLiveEvent> Events;

    UPROPERTY()
    TArray<FString> JoinedEvents;

    UPROPERTY()
    TMap<FString, TArray<FMGEventLeaderboardEntry>> EventLeaderboards;

    FTimerHandle TickTimerHandle;
};

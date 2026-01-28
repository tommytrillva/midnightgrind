// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGSeasonSubsystem.h
 * Season Subsystem - Manages Seasonal Content and Live Events
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the Season Subsystem, which manages the game's seasonal
 * content cycle. Seasons are multi-week periods (typically 8-12 weeks) with
 * themed content, progression rewards, and time-limited events. Think of it
 * like Fortnite's seasons - each one brings new content and a fresh start
 * for the progression system.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. SEASONS (FMGSeasonData)
 *    A season is a themed content period with:
 *    - Unique name and theme (e.g., "Season 3: Neon Nights")
 *    - Start and end dates (typically 2-3 months)
 *    - 100 progression tiers with rewards
 *    - Featured vehicles and tracks that get bonus XP
 *    - XP requirements that scale per tier
 *
 * 2. SEASON PROGRESS (FMGSeasonProgress)
 *    Tracks the player's advancement through the season:
 *    - CurrentTier: Which tier (1-100) the player has reached
 *    - CurrentXP: XP earned toward the next tier
 *    - TotalXP: All XP earned this season
 *    - bHasPremiumPass: Whether player bought the premium upgrade
 *    - ClaimedTiers: Which rewards have been collected
 *
 * 3. SEASON REWARDS (FMGSeasonReward)
 *    Items earned by reaching tiers in the season pass:
 *    - Cash: In-game currency for buying cars/parts
 *    - Reputation: Increases your street cred level
 *    - SeasonXP: Bonus XP toward future tiers
 *    - Vehicle/Customization: Unlockable content
 *    - Cosmetic: Avatars, emblems, titles
 *    - Premium rewards (bIsPremium=true) require the premium pass
 *
 * 4. EVENT TYPES (EMGEventType)
 *    Different categories of time-limited events:
 *    - Weekly: Core engagement, resets every Monday
 *    - Weekend: Friday-Sunday specials with bonus rewards
 *    - TimeTrial: Beat-the-clock challenges on specific tracks
 *    - Community: Server-wide collaborative goals
 *    - Holiday: Themed events (Halloween, Christmas, etc.)
 *    - Championship: Multi-race competitive tournaments
 *    - CrewBattle: Team-based competitive events
 *
 * 5. EVENTS (FMGEventData)
 *    Time-limited gameplay experiences with:
 *    - Specific objectives to complete (FMGEventObjective)
 *    - Rewards for completion
 *    - Required player level to participate
 *    - Participation tracking
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *     +------------------+     +----------------------+
 *     | UMGSeasonSubsystem| <-> | UMGLiveEventsManager |
 *     +------------------+     +----------------------+
 *              |                         |
 *              v                         v
 *     +------------------+     +------------------+
 *     | Season Pass      |     | Challenges       |
 *     | (Tier Rewards)   |     | (Objectives)     |
 *     +------------------+     +------------------+
 *              |                         |
 *              +------------+------------+
 *                           |
 *                           v
 *              +------------------------+
 *              | Race/Gameplay Results  |
 *              | UpdateEventProgress()  |
 *              +------------------------+
 *                           |
 *                           v
 *              +------------------------+
 *              | UI Widgets             |
 *              | (MGSeasonWidgets.h)    |
 *              +------------------------+
 *
 * TYPICAL USAGE FLOW:
 * -------------------
 * 1. At startup, LoadSeasonData() and LoadEventsData() fetch current season info
 * 2. Player earns XP through races -> AddSeasonXP() updates progress
 * 3. When XP threshold is reached -> OnSeasonTierUp fires, unlocking rewards
 * 4. Player views Season Pass UI -> GetAvailableRewards() shows claimable items
 * 5. Player clicks "Claim" -> ClaimTierReward() grants the item
 * 6. Events are checked periodically via CheckEventTimers()
 * 7. After races, UpdateEventProgress() tracks objective completion
 *
 * DELEGATES (Events you can listen to):
 * ------------------------------------
 * - OnSeasonChanged: New season started
 * - OnSeasonTierUp: Player reached a new tier
 * - OnSeasonXPGained: XP was earned (update progress bar)
 * - OnEventStarted/Ended: Event lifecycle changes
 * - OnEventObjectiveProgress: Objective progress updated
 * - OnEventCompleted: All objectives in an event finished
 *
 * TIMING AND RESETS:
 * ------------------
 * - Daily challenges reset at midnight UTC (GetDailyResetTime())
 * - Weekly challenges reset Monday midnight UTC (GetWeeklyResetTime())
 * - Events have their own start/end times checked every 60 seconds
 * - Seasons typically last 8-12 weeks
 *
 * @see MGSeasonWidgets.h - UI widgets for displaying season content
 * @see UMGLiveEventsManager - Handles challenges and community goals
 * @see UMGProgressionSubsystem - Player level progression (separate from season)
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSeasonSubsystem.generated.h"

/**
 * Event type
 */
UENUM(BlueprintType)
enum class EMGEventType : uint8
{
	/** Weekly challenge */
	Weekly,
	/** Weekend special */
	Weekend,
	/** Time trial */
	TimeTrial,
	/** Community event */
	Community,
	/** Holiday special */
	Holiday,
	/** Limited time */
	LimitedTime,
	/** Crew battle */
	CrewBattle,
	/** Championship */
	Championship
};

/**
 * Reward type
 */
UENUM(BlueprintType)
enum class EMGRewardType : uint8
{
	/** Cash */
	Cash,
	/** Reputation */
	Reputation,
	/** Season XP */
	SeasonXP,
	/** Vehicle unlock */
	Vehicle,
	/** Customization item */
	Customization,
	/** Emblem/avatar */
	Cosmetic,
	/** Title */
	Title,
	/** Exclusive wrap */
	Wrap
};

/**
 * Season tier reward
 */
USTRUCT(BlueprintType)
struct FMGSeasonReward
{
	GENERATED_BODY()

	/** Tier required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Tier = 1;

	/** Reward type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRewardType Type = EMGRewardType::Cash;

	/** Reward ID (for items) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	/** Reward quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;

	/** Is premium reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPremium = false;

	/** Is claimed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsClaimed = false;
};

/**
 * Season data
 */
USTRUCT(BlueprintType)
struct FMGSeasonData
{
	GENERATED_BODY()

	/** Season ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	/** Season number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonNumber = 1;

	/** Season name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonName;

	/** Season theme */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonTheme;

	/** Start date */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	/** End date */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	/** Max tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTier = 100;

	/** XP per tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XPPerTier = 1000;

	/** Rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGSeasonReward> Rewards;

	/** Featured vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FeaturedVehicle;

	/** Featured track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FeaturedTrack;
};

/**
 * Event objective
 */
USTRUCT(BlueprintType)
struct FMGEventObjective
{
	GENERATED_BODY()

	/** Objective ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Target value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetValue = 1;

	/** Current progress */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentProgress = 0;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;

	/** Stat to track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackedStat;

	/** Required track (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrack;

	/** Required vehicle class (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicleClass;
};

/**
 * Live event data
 */
USTRUCT(BlueprintType)
struct FMGEventData
{
	GENERATED_BODY()

	/** Event ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	/** Event type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEventType Type = EMGEventType::Weekly;

	/** Event name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EventName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Start time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	/** End time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	/** Objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventObjective> Objectives;

	/** Rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGSeasonReward> Rewards;

	/** Required level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/** Is featured */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;

	/** Total participants */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalParticipants = 0;

	/** Player participation status */
	UPROPERTY(BlueprintReadOnly)
	bool bIsParticipating = false;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;
};

/**
 * Player season progress
 */
USTRUCT(BlueprintType)
struct FMGSeasonProgress
{
	GENERATED_BODY()

	/** Season ID */
	UPROPERTY(BlueprintReadOnly)
	FName SeasonID;

	/** Current tier */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentTier = 1;

	/** Current XP in tier */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentXP = 0;

	/** Total XP earned */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalXP = 0;

	/** Has premium pass */
	UPROPERTY(BlueprintReadOnly)
	bool bHasPremiumPass = false;

	/** Claimed reward tiers */
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ClaimedTiers;

	/** Claimed premium tiers */
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> ClaimedPremiumTiers;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeasonChanged, const FMGSeasonData&, NewSeason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSeasonTierUp, int32, NewTier, const TArray<FMGSeasonReward>&, UnlockedRewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSeasonXPGained, int32, XPGained, int32, TotalXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventStarted, const FMGEventData&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventEnded, const FMGEventData&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEventObjectiveProgress, const FMGEventData&, Event, const FMGEventObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventCompleted, const FMGEventData&, Event);

/**
 * Season Subsystem
 * Manages seasons and live events
 *
 * Features:
 * - Season pass progression
 * - Live events
 * - Time-limited challenges
 * - Rewards system
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSeasonSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSeasonChanged OnSeasonChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSeasonTierUp OnSeasonTierUp;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSeasonXPGained OnSeasonXPGained;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventStarted OnEventStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventEnded OnEventEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventObjectiveProgress OnEventObjectiveProgress;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventCompleted OnEventCompleted;

	// ==========================================
	// SEASON
	// ==========================================

	/** Get current season */
	UFUNCTION(BlueprintPure, Category = "Season")
	FMGSeasonData GetCurrentSeason() const { return CurrentSeason; }

	/** Get season progress */
	UFUNCTION(BlueprintPure, Category = "Season")
	FMGSeasonProgress GetSeasonProgress() const { return SeasonProgress; }

	/** Get time remaining in season */
	UFUNCTION(BlueprintPure, Category = "Season")
	FTimespan GetSeasonTimeRemaining() const;

	/** Get rewards for tier */
	UFUNCTION(BlueprintPure, Category = "Season")
	TArray<FMGSeasonReward> GetRewardsForTier(int32 Tier) const;

	/** Get all available rewards up to current tier */
	UFUNCTION(BlueprintPure, Category = "Season")
	TArray<FMGSeasonReward> GetAvailableRewards() const;

	/** Claim tier reward */
	UFUNCTION(BlueprintCallable, Category = "Season")
	bool ClaimTierReward(int32 Tier, bool bPremium);

	/** Claim all available rewards */
	UFUNCTION(BlueprintCallable, Category = "Season")
	void ClaimAllRewards();

	/** Add season XP */
	UFUNCTION(BlueprintCallable, Category = "Season")
	void AddSeasonXP(int32 Amount);

	/** Get XP needed for next tier */
	UFUNCTION(BlueprintPure, Category = "Season")
	int32 GetXPForNextTier() const;

	/** Get tier progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Season")
	float GetTierProgress() const;

	/** Has premium pass */
	UFUNCTION(BlueprintPure, Category = "Season")
	bool HasPremiumPass() const { return SeasonProgress.bHasPremiumPass; }

	/** Purchase premium pass */
	UFUNCTION(BlueprintCallable, Category = "Season")
	void PurchasePremiumPass();

	// ==========================================
	// EVENTS
	// ==========================================

	/** Get active events */
	UFUNCTION(BlueprintPure, Category = "Events")
	TArray<FMGEventData> GetActiveEvents() const;

	/** Get featured event */
	UFUNCTION(BlueprintPure, Category = "Events")
	FMGEventData GetFeaturedEvent() const;

	/** Get event by ID */
	UFUNCTION(BlueprintPure, Category = "Events")
	FMGEventData GetEvent(FName EventID) const;

	/** Get upcoming events */
	UFUNCTION(BlueprintPure, Category = "Events")
	TArray<FMGEventData> GetUpcomingEvents() const;

	/** Get completed events */
	UFUNCTION(BlueprintPure, Category = "Events")
	TArray<FMGEventData> GetCompletedEvents() const;

	/** Join event */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void JoinEvent(FName EventID);

	/** Get event time remaining */
	UFUNCTION(BlueprintPure, Category = "Events")
	FTimespan GetEventTimeRemaining(FName EventID) const;

	/** Update event objective progress */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void UpdateEventProgress(FName StatID, int32 Value, FName TrackID = NAME_None, FName VehicleClass = NAME_None);

	// ==========================================
	// CHALLENGES
	// ==========================================

	/** Get daily challenges */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGEventObjective> GetDailyChallenges() const { return DailyChallenges; }

	/** Get weekly challenges */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGEventObjective> GetWeeklyChallenges() const { return WeeklyChallenges; }

	/** Get time until daily reset */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	FTimespan GetDailyResetTime() const;

	/** Get time until weekly reset */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	FTimespan GetWeeklyResetTime() const;

	/** Refresh daily challenges */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void RefreshDailyChallenges();

	// ==========================================
	// UTILITY
	// ==========================================

	/** Format time remaining */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText FormatTimeRemaining(FTimespan Time);

	/** Get reward type display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetRewardTypeDisplayName(EMGRewardType Type);

	/** Get event type display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetEventTypeDisplayName(EMGEventType Type);

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Current season */
	UPROPERTY()
	FMGSeasonData CurrentSeason;

	/** Season progress */
	UPROPERTY()
	FMGSeasonProgress SeasonProgress;

	/** Active events */
	UPROPERTY()
	TArray<FMGEventData> ActiveEvents;

	/** Upcoming events */
	UPROPERTY()
	TArray<FMGEventData> UpcomingEvents;

	/** Completed events (history) */
	UPROPERTY()
	TArray<FMGEventData> CompletedEvents;

	/** Daily challenges */
	UPROPERTY()
	TArray<FMGEventObjective> DailyChallenges;

	/** Weekly challenges */
	UPROPERTY()
	TArray<FMGEventObjective> WeeklyChallenges;

	/** Last daily reset */
	FDateTime LastDailyReset;

	/** Last weekly reset */
	FDateTime LastWeeklyReset;

	/** Event check interval */
	float EventCheckInterval = 60.0f;

	/** Event check accumulator */
	float EventCheckAccumulator = 0.0f;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Load season data */
	void LoadSeasonData();

	/** Load events data */
	void LoadEventsData();

	/** Load progress */
	void LoadProgress();

	/** Save progress */
	void SaveProgress();

	/** Check event timers */
	void CheckEventTimers();

	/** Check challenge resets */
	void CheckChallengeResets();

	/** Generate daily challenges */
	void GenerateDailyChallenges();

	/** Generate weekly challenges */
	void GenerateWeeklyChallenges();

	/** Process tier up */
	void ProcessTierUp(int32 OldTier, int32 NewTier);

	/** Check event completion */
	void CheckEventCompletion(FMGEventData& Event);

	/** Generate mock season */
	void GenerateMockSeason();

	/** Generate mock events */
	void GenerateMockEvents();
};

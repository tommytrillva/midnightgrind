// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGDailyRewardsSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Daily Rewards system for the Midnight Grind racing game.
 * The Daily Rewards system is a common player engagement feature in modern games
 * that incentivizes players to log in every day by offering escalating rewards.
 *
 * Think of it like a digital "punch card" - the more consecutive days you play,
 * the better rewards you receive. This system helps with player retention by
 * creating a habit of daily play.
 *
 * KEY CONCEPTS:
 * -------------
 * 1. LOGIN STREAK: The number of consecutive days a player has logged in.
 *    If a player misses a day, their streak may reset (depending on settings).
 *    Example: Log in Mon, Tue, Wed = 3-day streak. Miss Thu = streak might reset.
 *
 * 2. DAILY REWARD: A gift the player receives for logging in on a specific day.
 *    Day 1 might give 100 coins, Day 7 might give a rare vehicle part.
 *
 * 3. STREAK MILESTONE: Special bonus rewards given at significant streak lengths.
 *    Example: 7-day milestone (Week1), 30-day milestone (Month1), etc.
 *
 * 4. LOGIN CALENDAR: A monthly or cyclic schedule of what rewards are available
 *    on each day. Think of it like an advent calendar for the game.
 *
 * 5. SPECIAL EVENTS: Limited-time periods with enhanced rewards or multipliers.
 *    Example: "Holiday Event" with double rewards for 2 weeks.
 *
 * ARCHITECTURE:
 * -------------
 * This is a UGameInstanceSubsystem, meaning:
 * - It's automatically created when the game starts
 * - There's only ONE instance for the entire game session
 * - It persists across level changes (unlike Actor-based systems)
 * - Access it via: GetGameInstance()->GetSubsystem<UMGDailyRewardsSubsystem>()
 *
 * The system uses several data structures (USTRUCT):
 * - FMGDailyReward: Describes a single reward item
 * - FMGStreakBonus: Defines milestone bonuses
 * - FMGLoginCalendar: The full reward schedule
 * - FMGPlayerLoginData: Tracks the player's login history
 * - FMGRewardClaimResult: The outcome when claiming a reward
 *
 * TYPICAL WORKFLOW:
 * -----------------
 * 1. Player launches game -> ProcessLogin() is called
 * 2. System checks if it's a new day -> Updates streak accordingly
 * 3. UI shows "Claim Reward" button -> Player clicks it
 * 4. ClaimDailyReward() is called -> Returns FMGRewardClaimResult
 * 5. Game grants the items (currency, parts, etc.) to the player
 *
 * DELEGATES (Events):
 * -------------------
 * The system broadcasts events that other parts of the game can listen to:
 * - OnDailyRewardClaimed: Fired when player claims their daily reward
 * - OnStreakUpdated: Fired when the streak count changes
 * - OnMilestoneReached: Fired when player hits a major milestone (Week1, Month1, etc.)
 *
 * RELATED SYSTEMS:
 * ----------------
 * - MGDailyLoginSubsystem: More detailed daily login tracking with challenges
 * - MGStreakSubsystem: Handles in-game performance streaks (win streaks, etc.)
 * - MGMilestoneSubsystem: General achievement and progression tracking
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDailyRewardsSubsystem.generated.h"

// EMGRewardType - REMOVED (duplicate)
// Canonical definition in: BattlePass/MGBattlePassSubsystem.h

UENUM(BlueprintType)
enum class EMGRewardRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary,
	Mythic
};

UENUM(BlueprintType)
enum class EMGStreakMilestone : uint8
{
	None,
	Week1,
	Week2,
	Week3,
	Month1,
	Month2,
	Month3,
	Year1
};

USTRUCT(BlueprintType)
struct FMGDailyReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DayNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRewardType RewardType = EMGRewardType::Currency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRewardRarity Rarity = EMGRewardRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBonusReward = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGStreakBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StreakDays = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreakMilestone Milestone = EMGStreakMilestone::Week1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDailyReward> BonusRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RewardMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MilestoneTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MilestoneDescription;
};

USTRUCT(BlueprintType)
struct FMGLoginCalendar
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CalendarID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CalendarName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDailyReward> DailyRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGStreakBonus> StreakBonuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CycleDays = 28;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bResetOnCycleComplete = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FMGPlayerLoginData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastLoginDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastClaimDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLogins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCalendarDay = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ClaimedDays;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGStreakMilestone> ClaimedMilestones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MissedDays = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasClaimedToday = false;
};

USTRUCT(BlueprintType)
struct FMGRewardClaimResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDailyReward ClaimedReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDailyReward> BonusRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NewStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStreakBroken = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMilestoneReached = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreakMilestone MilestoneReached = EMGStreakMilestone::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ErrorMessage;
};

USTRUCT(BlueprintType)
struct FMGSpecialLoginEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDailyReward> EventRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RewardMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDoubleStreak = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPreventStreakLoss = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDailyRewardClaimed, const FMGRewardClaimResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStreakUpdated, int32, NewStreak, bool, bBroken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMilestoneReached, EMGStreakMilestone, Milestone, const TArray<FMGDailyReward>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginCalendarReset, FName, CalendarID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpecialEventStarted, const FMGSpecialLoginEvent&, Event);

UCLASS()
class MIDNIGHTGRIND_API UMGDailyRewardsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Login Management
	UFUNCTION(BlueprintCallable, Category = "DailyRewards|Login")
	void ProcessLogin();

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Login")
	bool CanClaimDailyReward() const;

	UFUNCTION(BlueprintCallable, Category = "DailyRewards|Login")
	FMGRewardClaimResult ClaimDailyReward();

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Login")
	FMGPlayerLoginData GetPlayerLoginData() const { return PlayerLoginData; }

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Login")
	int32 GetCurrentStreak() const { return PlayerLoginData.CurrentStreak; }

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Login")
	int32 GetLongestStreak() const { return PlayerLoginData.LongestStreak; }

	// Calendar Management
	UFUNCTION(BlueprintCallable, Category = "DailyRewards|Calendar")
	void SetActiveCalendar(FName CalendarID);

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Calendar")
	FMGLoginCalendar GetActiveCalendar() const { return ActiveCalendar; }

	UFUNCTION(BlueprintCallable, Category = "DailyRewards|Calendar")
	void RegisterCalendar(const FMGLoginCalendar& Calendar);

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Calendar")
	TArray<FMGLoginCalendar> GetAllCalendars() const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Calendar")
	FMGDailyReward GetRewardForDay(int32 Day) const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Calendar")
	TArray<FMGDailyReward> GetUpcomingRewards(int32 DaysAhead = 7) const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Calendar")
	int32 GetDaysUntilNextMilestone() const;

	// Streak System
	UFUNCTION(BlueprintPure, Category = "DailyRewards|Streak")
	FMGStreakBonus GetCurrentStreakBonus() const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Streak")
	FMGStreakBonus GetNextStreakMilestone() const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Streak")
	bool HasClaimedMilestone(EMGStreakMilestone Milestone) const;

	UFUNCTION(BlueprintCallable, Category = "DailyRewards|Streak")
	void RecoverStreak(int32 RecoveryDays = 1);

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Streak")
	int32 GetStreakRecoveryCost(int32 MissedDays) const;

	// Special Events
	UFUNCTION(BlueprintCallable, Category = "DailyRewards|Events")
	void RegisterSpecialEvent(const FMGSpecialLoginEvent& Event);

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Events")
	TArray<FMGSpecialLoginEvent> GetActiveSpecialEvents() const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Events")
	bool IsSpecialEventActive() const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Events")
	float GetCurrentRewardMultiplier() const;

	// Time Utilities
	UFUNCTION(BlueprintPure, Category = "DailyRewards|Time")
	FTimespan GetTimeUntilReset() const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Time")
	FDateTime GetNextResetTime() const;

	UFUNCTION(BlueprintPure, Category = "DailyRewards|Time")
	bool IsNewDay() const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "DailyRewards|Save")
	void SaveLoginData();

	UFUNCTION(BlueprintCallable, Category = "DailyRewards|Save")
	void LoadLoginData();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "DailyRewards|Events")
	FOnDailyRewardClaimed OnDailyRewardClaimed;

	UPROPERTY(BlueprintAssignable, Category = "DailyRewards|Events")
	FOnStreakUpdated OnStreakUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DailyRewards|Events")
	FOnMilestoneReached OnMilestoneReached;

	UPROPERTY(BlueprintAssignable, Category = "DailyRewards|Events")
	FOnLoginCalendarReset OnLoginCalendarReset;

	UPROPERTY(BlueprintAssignable, Category = "DailyRewards|Events")
	FOnSpecialEventStarted OnSpecialEventStarted;

protected:
	void UpdateStreak();
	void CheckMilestones();
	void ApplyReward(const FMGDailyReward& Reward);
	void InitializeDefaultCalendar();
	FMGDailyReward GenerateRewardForDay(int32 Day) const;
	bool IsSameDay(const FDateTime& Date1, const FDateTime& Date2) const;
	bool IsConsecutiveDay(const FDateTime& LastDate, const FDateTime& CurrentDate) const;
	void CheckAndResetCalendar();

	UPROPERTY()
	FMGPlayerLoginData PlayerLoginData;

	UPROPERTY()
	FMGLoginCalendar ActiveCalendar;

	UPROPERTY()
	TMap<FName, FMGLoginCalendar> Calendars;

	UPROPERTY()
	TArray<FMGSpecialLoginEvent> SpecialEvents;

	UPROPERTY()
	FDateTime ServerTime;

	UPROPERTY()
	int32 ResetHourUTC = 0;

	FTimerHandle ResetCheckHandle;
};

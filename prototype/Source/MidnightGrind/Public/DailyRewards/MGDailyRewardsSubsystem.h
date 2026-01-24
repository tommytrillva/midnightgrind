// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDailyRewardsSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGRewardType : uint8
{
	Currency,
	PremiumCurrency,
	Vehicle,
	Part,
	Cosmetic,
	ExperienceBoost,
	CurrencyBoost,
	LootBox,
	CustomizationItem,
	Decal,
	WheelSet,
	NeonKit
};

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

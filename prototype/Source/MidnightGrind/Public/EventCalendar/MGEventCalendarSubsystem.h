// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGEventCalendarSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGEventType : uint8
{
	DoubleXP,
	DoubleCredits,
	SpecialRace,
	LimitedTimeMode,
	CommunityChallenge,
	SeasonLaunch,
	SeasonEnd,
	Holiday,
	Tournament,
	ContentDrop,
	FlashSale,
	BonusRewards,
	Showcase,
	MaintenanceWindow
};

UENUM(BlueprintType)
enum class EMGEventState : uint8
{
	Scheduled,
	Upcoming,
	Active,
	Ending,
	Completed,
	Cancelled
};

UENUM(BlueprintType)
enum class EMGEventPriority : uint8
{
	Low,
	Normal,
	High,
	Critical
};

UENUM(BlueprintType)
enum class EMGPlaylistRotation : uint8
{
	Daily,
	Weekly,
	Biweekly,
	Monthly,
	Seasonal,
	Permanent
};

USTRUCT(BlueprintType)
struct FMGEventReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RewardName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PremiumCurrency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExperienceMultiplier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyMultiplier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ExclusiveItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> RewardIcon;
};

USTRUCT(BlueprintType)
struct FMGScheduledEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEventType EventType = EMGEventType::DoubleXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEventState State = EMGEventState::Scheduled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEventPriority Priority = EMGEventPriority::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EventDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventReward> Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredGameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRecurring = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimespan RecurrenceInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> EventBanner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> EventIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor EventColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNotifyOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tags;
};

USTRUCT(BlueprintType)
struct FMGPlaylistEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlaylistID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlaylistName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlaylistDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> TrackPool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlaylistRotation Rotation = EMGPlaylistRotation::Daily;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AvailableFrom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AvailableUntil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> PlaylistIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusXPPercent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusCurrencyPercent = 0;
};

USTRUCT(BlueprintType)
struct FMGSeasonInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonTheme;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxLevel = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> SeasonBanner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> SeasonIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SeasonColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMGCalendarDay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Date;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGScheduledEvent> Events;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPlaylistEntry> FeaturedPlaylists;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHoliday = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText HolidayName;
};

USTRUCT(BlueprintType)
struct FMGEventReminder
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimespan ReminderBefore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNotified = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventStarted, const FMGScheduledEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventEnding, const FMGScheduledEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventCompleted, const FMGScheduledEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaylistRotated, const TArray<FMGPlaylistEntry>&, NewPlaylists);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeasonChanged, const FMGSeasonInfo&, NewSeason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEventReminder, const FMGScheduledEvent&, Event, FTimespan, TimeUntilStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCalendarRefreshed);

UCLASS()
class MIDNIGHTGRIND_API UMGEventCalendarSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Events
	UFUNCTION(BlueprintPure, Category = "Calendar|Events")
	TArray<FMGScheduledEvent> GetActiveEvents() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Events")
	TArray<FMGScheduledEvent> GetUpcomingEvents(int32 MaxEvents = 10) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Events")
	TArray<FMGScheduledEvent> GetEventsByType(EMGEventType Type) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Events")
	FMGScheduledEvent GetEvent(FName EventID) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Events")
	TArray<FMGScheduledEvent> GetFeaturedEvents() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Events")
	bool IsEventActive(FName EventID) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Events")
	FTimespan GetTimeUntilEvent(FName EventID) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Events")
	FTimespan GetEventTimeRemaining(FName EventID) const;

	// Playlists
	UFUNCTION(BlueprintPure, Category = "Calendar|Playlists")
	TArray<FMGPlaylistEntry> GetActivePlaylists() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Playlists")
	TArray<FMGPlaylistEntry> GetFeaturedPlaylists() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Playlists")
	FMGPlaylistEntry GetPlaylist(FName PlaylistID) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Playlists")
	FTimespan GetTimeUntilPlaylistRotation() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Playlists")
	TArray<FMGPlaylistEntry> GetPlaylistsByRotation(EMGPlaylistRotation Rotation) const;

	// Calendar View
	UFUNCTION(BlueprintPure, Category = "Calendar|View")
	TArray<FMGCalendarDay> GetCalendarWeek(FDateTime StartDate) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|View")
	TArray<FMGCalendarDay> GetCalendarMonth(int32 Year, int32 Month) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|View")
	FMGCalendarDay GetCalendarDay(FDateTime Date) const;

	UFUNCTION(BlueprintPure, Category = "Calendar|View")
	FMGCalendarDay GetToday() const;

	// Season
	UFUNCTION(BlueprintPure, Category = "Calendar|Season")
	FMGSeasonInfo GetCurrentSeason() const { return CurrentSeason; }

	UFUNCTION(BlueprintPure, Category = "Calendar|Season")
	FTimespan GetSeasonTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Season")
	float GetSeasonProgress() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Season")
	int32 GetSeasonDaysRemaining() const;

	// Bonuses
	UFUNCTION(BlueprintPure, Category = "Calendar|Bonuses")
	int32 GetActiveXPMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Bonuses")
	int32 GetActiveCurrencyMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Bonuses")
	TArray<FMGScheduledEvent> GetActiveBonusEvents() const;

	// Reminders
	UFUNCTION(BlueprintCallable, Category = "Calendar|Reminders")
	void SetEventReminder(FName EventID, FTimespan ReminderBefore);

	UFUNCTION(BlueprintCallable, Category = "Calendar|Reminders")
	void RemoveEventReminder(FName EventID);

	UFUNCTION(BlueprintPure, Category = "Calendar|Reminders")
	TArray<FMGEventReminder> GetEventReminders() const;

	UFUNCTION(BlueprintPure, Category = "Calendar|Reminders")
	bool HasReminder(FName EventID) const;

	// Refresh
	UFUNCTION(BlueprintCallable, Category = "Calendar|Refresh")
	void RefreshCalendar();

	UFUNCTION(BlueprintCallable, Category = "Calendar|Refresh")
	void ForceRefreshFromServer();

	UFUNCTION(BlueprintPure, Category = "Calendar|Refresh")
	FDateTime GetLastRefreshTime() const { return LastRefreshTime; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Calendar|Events")
	FOnEventStarted OnEventStarted;

	UPROPERTY(BlueprintAssignable, Category = "Calendar|Events")
	FOnEventEnding OnEventEnding;

	UPROPERTY(BlueprintAssignable, Category = "Calendar|Events")
	FOnEventCompleted OnEventCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Calendar|Events")
	FOnPlaylistRotated OnPlaylistRotated;

	UPROPERTY(BlueprintAssignable, Category = "Calendar|Events")
	FOnSeasonChanged OnSeasonChanged;

	UPROPERTY(BlueprintAssignable, Category = "Calendar|Events")
	FOnEventReminder OnEventReminder;

	UPROPERTY(BlueprintAssignable, Category = "Calendar|Events")
	FOnCalendarRefreshed OnCalendarRefreshed;

protected:
	void OnCalendarTick();
	void CheckEventStates();
	void CheckReminders();
	void CheckPlaylistRotation();
	void UpdateEventState(FMGScheduledEvent& Event);
	void InitializeDefaultEvents();
	void InitializeDefaultPlaylists();
	void InitializeDefaultSeason();
	void ProcessRecurringEvents();

	UPROPERTY()
	TMap<FName, FMGScheduledEvent> AllEvents;

	UPROPERTY()
	TArray<FMGPlaylistEntry> AllPlaylists;

	UPROPERTY()
	FMGSeasonInfo CurrentSeason;

	UPROPERTY()
	TArray<FMGEventReminder> Reminders;

	UPROPERTY()
	FDateTime LastRefreshTime;

	UPROPERTY()
	FDateTime LastPlaylistRotation;

	UPROPERTY()
	TSet<FName> NotifiedEventStarts;

	UPROPERTY()
	TSet<FName> NotifiedEventEndings;

	FTimerHandle CalendarTickHandle;
};

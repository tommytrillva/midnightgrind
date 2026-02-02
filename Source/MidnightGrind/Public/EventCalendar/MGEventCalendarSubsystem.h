// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * ============================================================================
 * MGEventCalendarSubsystem.h
 * ============================================================================
 *
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file implements the Event Calendar system - a centralized scheduler
 * for all time-based game events, playlists, and seasonal content. Think of
 * it like a TV guide for the game that knows what's happening and when.
 *
 * KEY CONCEPTS:
 *
 * 1. SCHEDULED EVENTS (FMGScheduledEvent)
 *    Events are time-limited activities with start/end times:
 *    - DoubleXP/DoubleCredits: Bonus reward weekends
 *    - SpecialRace: Limited-time race events
 *    - CommunityChallenge: Goals the whole player base works toward
 *    - SeasonLaunch/SeasonEnd: Season milestones
 *    - Tournament: Competitive events
 *    - ContentDrop: New content releases
 *    - FlashSale: Limited-time shop discounts
 *
 * 2. EVENT LIFECYCLE (EMGEventState)
 *    Events progress through states automatically:
 *    Scheduled -> Upcoming -> Active -> Ending -> Completed
 *
 *    The subsystem's timer checks states and fires delegates when
 *    events transition (OnEventStarted, OnEventEnding, OnEventCompleted).
 *
 * 3. PLAYLIST ROTATION (FMGPlaylistEntry)
 *    Playlists are curated game mode selections that rotate on a schedule:
 *    - Daily: Changes every 24 hours
 *    - Weekly: Featured playlist of the week
 *    - Seasonal: Tied to the current season theme
 *    - Permanent: Always available
 *
 *    Example: "Drift Weekend" playlist with bonus XP, available Fri-Sun
 *
 * 4. SEASONS (FMGSeasonInfo)
 *    Seasons are multi-week content periods with themes:
 *    - Has a name and theme (e.g., "Season 3: Underground")
 *    - Defines the battle pass level cap
 *    - Affects available playlists and events
 *    - GetSeasonTimeRemaining() shows countdown to end
 *
 * 5. REMINDERS (FMGEventReminder)
 *    Players can set reminders for upcoming events:
 *    - SetEventReminder("EVENT_ID", FTimespan::FromHours(1))
 *    - OnEventReminder delegate fires at the reminder time
 *    - UI can show push notification or in-game alert
 *
 * 6. BONUS MULTIPLIERS
 *    Multiple events can stack bonuses:
 *    - GetActiveXPMultiplier() returns combined XP bonus
 *    - GetActiveCurrencyMultiplier() returns combined credit bonus
 *    - These should be applied to race rewards
 *
 * COMMON USE CASES:
 *
 * For UI Developers:
 * - GetActiveEvents() for the "What's Happening Now" screen
 * - GetUpcomingEvents() for the calendar view
 * - GetFeaturedPlaylists() for the main menu playlist selector
 * - GetCurrentSeason() for season pass screen header
 * - GetCalendarMonth() for a monthly calendar widget
 *
 * For Backend/Live Ops:
 * - ForceRefreshFromServer() to pull latest event data
 * - Events can be added server-side and pushed to clients
 * - Recurring events (bIsRecurring) auto-create future instances
 *
 * For Reward Systems:
 * - Check GetActiveXPMultiplier() before awarding XP
 * - Check GetActiveCurrencyMultiplier() before awarding credits
 * - Event rewards (FMGEventReward) grant items/currency
 *
 * DATA STRUCTURES:
 * - FMGScheduledEvent: Complete event definition with times and rewards
 * - FMGPlaylistEntry: A rotatable playlist with game modes and tracks
 * - FMGSeasonInfo: Season metadata and timing
 * - FMGCalendarDay: All events and playlists for one day
 * - FMGEventReward: Currency, XP multipliers, or exclusive items
 * - FMGEventReminder: Player-set notification for an event
 *
 * TIMER ARCHITECTURE:
 * The subsystem uses CalendarTickHandle to periodically:
 * - Check if events need state transitions
 * - Fire reminder notifications
 * - Rotate playlists at scheduled times
 * - Update the "ending soon" warning state
 *
 * ============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "LiveService/MGLiveEventsSubsystem.h"  // FMGEventReward, FMGPlaylistEntry (canonical)
#include "SeasonPass/MGSeasonPassSubsystem.h"   // FMGSeasonInfo (canonical)
#include "MGEventCalendarSubsystem.generated.h"

// MOVED TO MGSharedTypes.h
// UENUM(BlueprintType)
// enum class EMGEventType : uint8
// {
// 	DoubleXP,
// 	DoubleCredits,
// 	SpecialRace,
// 	LimitedTimeMode,
// 	CommunityChallenge,
// 	SeasonLaunch,
// 	SeasonEnd,
// 	Holiday,
// 	Tournament,
// 	ContentDrop,
// 	FlashSale,
// 	BonusRewards,
// 	Showcase,
// 	MaintenanceWindow
// };

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

// FMGEventReward - REMOVED (duplicate)
// Canonical definition in: LiveService/MGLiveEventsSubsystem.h

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

// FMGPlaylistEntry - REMOVED (duplicate)
// Canonical definition in: LiveService/MGLiveEventsSubsystem.h

// FMGSeasonInfo - REMOVED (duplicate)
// Canonical definition in: SeasonPass/MGSeasonPassSubsystem.h

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

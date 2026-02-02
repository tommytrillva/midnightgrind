// Copyright Midnight Grind. All Rights Reserved.

#include "EventCalendar/MGEventCalendarSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGEventCalendarSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultSeason();
	InitializeDefaultEvents();
	InitializeDefaultPlaylists();

	LastRefreshTime = FDateTime::UtcNow();
	LastPlaylistRotation = FDateTime::UtcNow();

	// Start calendar tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CalendarTickHandle,
			this,
			&UMGEventCalendarSubsystem::OnCalendarTick,
			10.0f,
			true
		);
	}
}

void UMGEventCalendarSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CalendarTickHandle);
	}

	Super::Deinitialize();
}

bool UMGEventCalendarSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

TArray<FMGScheduledEvent> UMGEventCalendarSubsystem::GetActiveEvents() const
{
	TArray<FMGScheduledEvent> Result;
	FDateTime Now = FDateTime::UtcNow();

	for (const auto& Pair : AllEvents)
	{
		if (Pair.Value.State == EMGEventState::Active ||
			(Now >= Pair.Value.StartTime && Now <= Pair.Value.EndTime))
		{
			Result.Add(Pair.Value);
		}
	}

	// Sort by priority
	Result.Sort([](const FMGScheduledEvent& A, const FMGScheduledEvent& B)
	{
		return (int32)A.Priority > (int32)B.Priority;
	});

	return Result;
}

TArray<FMGScheduledEvent> UMGEventCalendarSubsystem::GetUpcomingEvents(int32 MaxEvents) const
{
	TArray<FMGScheduledEvent> Result;
	FDateTime Now = FDateTime::UtcNow();

	for (const auto& Pair : AllEvents)
	{
		if (Pair.Value.StartTime > Now)
		{
			Result.Add(Pair.Value);
		}
	}

	// Sort by start time
	Result.Sort([](const FMGScheduledEvent& A, const FMGScheduledEvent& B)
	{
		return A.StartTime < B.StartTime;
	});

	// Limit results
	if (Result.Num() > MaxEvents)
	{
		Result.SetNum(MaxEvents);
	}

	return Result;
}

TArray<FMGScheduledEvent> UMGEventCalendarSubsystem::GetEventsByType(EMGEventType Type) const
{
	TArray<FMGScheduledEvent> Result;
	for (const auto& Pair : AllEvents)
	{
		if (Pair.Value.EventType == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FMGScheduledEvent UMGEventCalendarSubsystem::GetEvent(FName EventID) const
{
	const FMGScheduledEvent* Event = AllEvents.Find(EventID);
	return Event ? *Event : FMGScheduledEvent();
}

TArray<FMGScheduledEvent> UMGEventCalendarSubsystem::GetFeaturedEvents() const
{
	TArray<FMGScheduledEvent> Result;
	for (const auto& Pair : AllEvents)
	{
		if (Pair.Value.bIsFeatured && Pair.Value.State == EMGEventState::Active)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

bool UMGEventCalendarSubsystem::IsEventActive(FName EventID) const
{
	const FMGScheduledEvent* Event = AllEvents.Find(EventID);
	if (!Event)
	{
		return false;
	}

	FDateTime Now = FDateTime::UtcNow();
	return Now >= Event->StartTime && Now <= Event->EndTime;
}

FTimespan UMGEventCalendarSubsystem::GetTimeUntilEvent(FName EventID) const
{
	const FMGScheduledEvent* Event = AllEvents.Find(EventID);
	if (!Event)
	{
		return FTimespan::Zero();
	}

	FDateTime Now = FDateTime::UtcNow();
	if (Now >= Event->StartTime)
	{
		return FTimespan::Zero();
	}

	return Event->StartTime - Now;
}

FTimespan UMGEventCalendarSubsystem::GetEventTimeRemaining(FName EventID) const
{
	const FMGScheduledEvent* Event = AllEvents.Find(EventID);
	if (!Event)
	{
		return FTimespan::Zero();
	}

	FDateTime Now = FDateTime::UtcNow();
	if (Now > Event->EndTime)
	{
		return FTimespan::Zero();
	}

	if (Now < Event->StartTime)
	{
		return Event->EndTime - Event->StartTime;
	}

	return Event->EndTime - Now;
}

TArray<FMGPlaylistEntry> UMGEventCalendarSubsystem::GetActivePlaylists() const
{
	TArray<FMGPlaylistEntry> Result;
	FDateTime Now = FDateTime::UtcNow();

	for (const FMGPlaylistEntry& Playlist : AllPlaylists)
	{
		if (Playlist.bIsActive || (Now >= Playlist.AvailableFrom && Now <= Playlist.AvailableUntil))
		{
			Result.Add(Playlist);
		}
	}

	// Sort by sort order
	Result.Sort([](const FMGPlaylistEntry& A, const FMGPlaylistEntry& B)
	{
		return A.SortOrder < B.SortOrder;
	});

	return Result;
}

TArray<FMGPlaylistEntry> UMGEventCalendarSubsystem::GetFeaturedPlaylists() const
{
	TArray<FMGPlaylistEntry> Result;
	for (const FMGPlaylistEntry& Playlist : AllPlaylists)
	{
		if (Playlist.bIsFeatured && Playlist.bIsActive)
		{
			Result.Add(Playlist);
		}
	}
	return Result;
}

FMGPlaylistEntry UMGEventCalendarSubsystem::GetPlaylist(FName PlaylistID) const
{
	for (const FMGPlaylistEntry& Playlist : AllPlaylists)
	{
		if (Playlist.PlaylistID == PlaylistID)
		{
			return Playlist;
		}
	}
	return FMGPlaylistEntry();
}

FTimespan UMGEventCalendarSubsystem::GetTimeUntilPlaylistRotation() const
{
	// Calculate time until next daily reset (4 AM UTC)
	FDateTime Now = FDateTime::UtcNow();
	FDateTime NextReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 4, 0, 0);

	if (Now >= NextReset)
	{
		NextReset += FTimespan::FromDays(1);
	}

	return NextReset - Now;
}

TArray<FMGPlaylistEntry> UMGEventCalendarSubsystem::GetPlaylistsByRotation(EMGPlaylistRotation Rotation) const
{
	TArray<FMGPlaylistEntry> Result;
	for (const FMGPlaylistEntry& Playlist : AllPlaylists)
	{
		if (Playlist.Rotation == Rotation)
		{
			Result.Add(Playlist);
		}
	}
	return Result;
}

TArray<FMGCalendarDay> UMGEventCalendarSubsystem::GetCalendarWeek(FDateTime StartDate) const
{
	TArray<FMGCalendarDay> Result;

	for (int32 i = 0; i < 7; i++)
	{
		FDateTime Date = StartDate + FTimespan::FromDays(i);
		Result.Add(GetCalendarDay(Date));
	}

	return Result;
}

TArray<FMGCalendarDay> UMGEventCalendarSubsystem::GetCalendarMonth(int32 Year, int32 Month) const
{
	TArray<FMGCalendarDay> Result;

	FDateTime FirstDay = FDateTime(Year, Month, 1, 0, 0, 0);
	int32 DaysInMonth = FDateTime::DaysInMonth(Year, Month);

	for (int32 Day = 1; Day <= DaysInMonth; Day++)
	{
		FDateTime Date = FDateTime(Year, Month, Day, 0, 0, 0);
		Result.Add(GetCalendarDay(Date));
	}

	return Result;
}

FMGCalendarDay UMGEventCalendarSubsystem::GetCalendarDay(FDateTime Date) const
{
	FMGCalendarDay Day;
	Day.Date = Date;

	FDateTime DayStart = FDateTime(Date.GetYear(), Date.GetMonth(), Date.GetDay(), 0, 0, 0);
	FDateTime DayEnd = DayStart + FTimespan::FromDays(1);

	// Find events that overlap with this day
	for (const auto& Pair : AllEvents)
	{
		if (Pair.Value.StartTime < DayEnd && Pair.Value.EndTime > DayStart)
		{
			Day.Events.Add(Pair.Value);
		}
	}

	// Find active playlists for this day
	for (const FMGPlaylistEntry& Playlist : AllPlaylists)
	{
		if (Playlist.AvailableFrom < DayEnd && Playlist.AvailableUntil > DayStart)
		{
			if (Playlist.bIsFeatured)
			{
				Day.FeaturedPlaylists.Add(Playlist);
			}
		}
	}

	return Day;
}

FMGCalendarDay UMGEventCalendarSubsystem::GetToday() const
{
	return GetCalendarDay(FDateTime::UtcNow());
}

FTimespan UMGEventCalendarSubsystem::GetSeasonTimeRemaining() const
{
	FDateTime Now = FDateTime::UtcNow();
	if (Now > CurrentSeason.EndTime)
	{
		return FTimespan::Zero();
	}
	return CurrentSeason.EndTime - Now;
}

float UMGEventCalendarSubsystem::GetSeasonProgress() const
{
	FDateTime Now = FDateTime::UtcNow();

	if (Now < CurrentSeason.StartTime)
	{
		return 0.0f;
	}

	if (Now > CurrentSeason.EndTime)
	{
		return 1.0f;
	}

	double TotalDuration = (CurrentSeason.EndTime - CurrentSeason.StartTime).GetTotalSeconds();
	double Elapsed = (Now - CurrentSeason.StartTime).GetTotalSeconds();

	return (float)(Elapsed / TotalDuration);
}

int32 UMGEventCalendarSubsystem::GetSeasonDaysRemaining() const
{
	return (int32)GetSeasonTimeRemaining().GetTotalDays();
}

int32 UMGEventCalendarSubsystem::GetActiveXPMultiplier() const
{
	int32 Multiplier = 1;

	TArray<FMGScheduledEvent> ActiveEvents = GetActiveEvents();
	for (const FMGScheduledEvent& Event : ActiveEvents)
	{
		if (Event.EventType == EMGEventType::DoubleXP)
		{
			for (const FMGEventReward& Reward : Event.Rewards)
			{
				if (Reward.ExperienceMultiplier > Multiplier)
				{
					Multiplier = Reward.ExperienceMultiplier;
				}
			}
		}
	}

	// Also check playlists
	TArray<FMGPlaylistEntry> ActivePlaylists = GetActivePlaylists();
	for (const FMGPlaylistEntry& Playlist : ActivePlaylists)
	{
		int32 PlaylistMultiplier = 1 + (Playlist.BonusXPPercent / 100);
		if (PlaylistMultiplier > Multiplier)
		{
			Multiplier = PlaylistMultiplier;
		}
	}

	return Multiplier;
}

int32 UMGEventCalendarSubsystem::GetActiveCurrencyMultiplier() const
{
	int32 Multiplier = 1;

	TArray<FMGScheduledEvent> ActiveEvents = GetActiveEvents();
	for (const FMGScheduledEvent& Event : ActiveEvents)
	{
		if (Event.EventType == EMGEventType::DoubleCredits)
		{
			for (const FMGEventReward& Reward : Event.Rewards)
			{
				if (Reward.CurrencyMultiplier > Multiplier)
				{
					Multiplier = Reward.CurrencyMultiplier;
				}
			}
		}
	}

	// Also check playlists
	TArray<FMGPlaylistEntry> ActivePlaylists = GetActivePlaylists();
	for (const FMGPlaylistEntry& Playlist : ActivePlaylists)
	{
		int32 PlaylistMultiplier = 1 + (Playlist.BonusCurrencyPercent / 100);
		if (PlaylistMultiplier > Multiplier)
		{
			Multiplier = PlaylistMultiplier;
		}
	}

	return Multiplier;
}

TArray<FMGScheduledEvent> UMGEventCalendarSubsystem::GetActiveBonusEvents() const
{
	TArray<FMGScheduledEvent> Result;
	TArray<FMGScheduledEvent> ActiveEvents = GetActiveEvents();

	for (const FMGScheduledEvent& Event : ActiveEvents)
	{
		if (Event.EventType == EMGEventType::DoubleXP ||
			Event.EventType == EMGEventType::DoubleCredits ||
			Event.EventType == EMGEventType::BonusRewards)
		{
			Result.Add(Event);
		}
	}

	return Result;
}

void UMGEventCalendarSubsystem::SetEventReminder(FName EventID, FTimespan ReminderBefore)
{
	// Remove existing reminder if any
	RemoveEventReminder(EventID);

	FMGEventReminder Reminder;
	Reminder.EventID = EventID;
	Reminder.ReminderBefore = ReminderBefore;
	Reminder.bNotified = false;

	Reminders.Add(Reminder);
}

void UMGEventCalendarSubsystem::RemoveEventReminder(FName EventID)
{
	for (int32 i = Reminders.Num() - 1; i >= 0; i--)
	{
		if (Reminders[i].EventID == EventID)
		{
			Reminders.RemoveAt(i);
		}
	}
}

TArray<FMGEventReminder> UMGEventCalendarSubsystem::GetEventReminders() const
{
	return Reminders;
}

bool UMGEventCalendarSubsystem::HasReminder(FName EventID) const
{
	for (const FMGEventReminder& Reminder : Reminders)
	{
		if (Reminder.EventID == EventID)
		{
			return true;
		}
	}
	return false;
}

void UMGEventCalendarSubsystem::RefreshCalendar()
{
	CheckEventStates();
	CheckPlaylistRotation();
	ProcessRecurringEvents();

	LastRefreshTime = FDateTime::UtcNow();
	OnCalendarRefreshed.Broadcast();
}

void UMGEventCalendarSubsystem::ForceRefreshFromServer()
{
	// This would fetch calendar data from server
	RefreshCalendar();
}

void UMGEventCalendarSubsystem::OnCalendarTick()
{
	CheckEventStates();
	CheckReminders();
	CheckPlaylistRotation();
}

void UMGEventCalendarSubsystem::CheckEventStates()
{
	FDateTime Now = FDateTime::UtcNow();
	FTimespan EndingThreshold = FTimespan::FromHours(1);

	for (auto& Pair : AllEvents)
	{
		FMGScheduledEvent& Event = Pair.Value;
		EMGEventState PreviousState = Event.State;

		// Update state based on time
		if (Now < Event.StartTime)
		{
			FTimespan TimeUntil = Event.StartTime - Now;
			if (TimeUntil < FTimespan::FromDays(1))
			{
				Event.State = EMGEventState::Upcoming;
			}
			else
			{
				Event.State = EMGEventState::Scheduled;
			}
		}
		else if (Now > Event.EndTime)
		{
			Event.State = EMGEventState::Completed;
		}
		else
		{
			FTimespan TimeRemaining = Event.EndTime - Now;
			if (TimeRemaining < EndingThreshold)
			{
				Event.State = EMGEventState::Ending;
			}
			else
			{
				Event.State = EMGEventState::Active;
			}
		}

		// Broadcast state changes
		if (PreviousState != Event.State)
		{
			if (Event.State == EMGEventState::Active && !NotifiedEventStarts.Contains(Event.EventID))
			{
				NotifiedEventStarts.Add(Event.EventID);
				OnEventStarted.Broadcast(Event);
			}
			else if (Event.State == EMGEventState::Ending && !NotifiedEventEndings.Contains(Event.EventID))
			{
				NotifiedEventEndings.Add(Event.EventID);
				OnEventEnding.Broadcast(Event);
			}
			else if (Event.State == EMGEventState::Completed)
			{
				OnEventCompleted.Broadcast(Event);
			}
		}
	}
}

void UMGEventCalendarSubsystem::CheckReminders()
{
	FDateTime Now = FDateTime::UtcNow();

	for (FMGEventReminder& Reminder : Reminders)
	{
		if (Reminder.bNotified)
		{
			continue;
		}

		const FMGScheduledEvent* Event = AllEvents.Find(Reminder.EventID);
		if (!Event)
		{
			continue;
		}

		FDateTime ReminderTime = Event->StartTime - Reminder.ReminderBefore;
		if (Now >= ReminderTime && Now < Event->StartTime)
		{
			Reminder.bNotified = true;
			OnEventReminder.Broadcast(*Event, Event->StartTime - Now);
		}
	}
}

void UMGEventCalendarSubsystem::CheckPlaylistRotation()
{
	FDateTime Now = FDateTime::UtcNow();

	// Check if daily rotation is due
	FDateTime NextDailyReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 4, 0, 0);
	if (Now.GetHour() >= 4)
	{
		// Already past today's reset, check if we need to rotate
		if (LastPlaylistRotation < NextDailyReset)
		{
			// Rotate daily playlists
			for (FMGPlaylistEntry& Playlist : AllPlaylists)
			{
				if (Playlist.Rotation == EMGPlaylistRotation::Daily)
				{
					// Update availability
					Playlist.AvailableFrom = NextDailyReset;
					Playlist.AvailableUntil = NextDailyReset + FTimespan::FromDays(1);
				}
			}

			LastPlaylistRotation = NextDailyReset;
			OnPlaylistRotated.Broadcast(GetActivePlaylists());
		}
	}
}

void UMGEventCalendarSubsystem::UpdateEventState(FMGScheduledEvent& Event)
{
	FDateTime Now = FDateTime::UtcNow();

	if (Now < Event.StartTime)
	{
		Event.State = EMGEventState::Scheduled;
	}
	else if (Now > Event.EndTime)
	{
		Event.State = EMGEventState::Completed;
	}
	else
	{
		Event.State = EMGEventState::Active;
	}
}

void UMGEventCalendarSubsystem::InitializeDefaultEvents()
{
	FDateTime Now = FDateTime::UtcNow();

	// Weekend Double XP
	FMGScheduledEvent DoubleXP;
	DoubleXP.EventID = FName("WeekendDoubleXP");
	DoubleXP.EventType = EMGEventType::DoubleXP;
	DoubleXP.EventName = FText::FromString("Weekend Double XP");
	DoubleXP.EventDescription = FText::FromString("Earn double experience in all races this weekend!");
	DoubleXP.StartTime = Now + FTimespan::FromDays(1);
	DoubleXP.EndTime = Now + FTimespan::FromDays(3);
	DoubleXP.bIsRecurring = true;
	DoubleXP.RecurrenceInterval = FTimespan::FromDays(7);
	DoubleXP.Priority = EMGEventPriority::High;
	DoubleXP.bIsFeatured = true;
	DoubleXP.bNotifyOnStart = true;

	FMGEventReward XPReward;
	XPReward.ExperienceMultiplier = 2;
	DoubleXP.Rewards.Add(XPReward);

	AllEvents.Add(DoubleXP.EventID, DoubleXP);

	// Midweek Credits Boost
	FMGScheduledEvent CreditsBoost;
	CreditsBoost.EventID = FName("MidweekCredits");
	CreditsBoost.EventType = EMGEventType::DoubleCredits;
	CreditsBoost.EventName = FText::FromString("Midweek Money");
	CreditsBoost.EventDescription = FText::FromString("50% bonus credits on all races!");
	CreditsBoost.StartTime = Now + FTimespan::FromDays(2);
	CreditsBoost.EndTime = Now + FTimespan::FromDays(3);
	CreditsBoost.Priority = EMGEventPriority::Normal;

	FMGEventReward CurrencyReward;
	CurrencyReward.CurrencyMultiplier = 150;
	CreditsBoost.Rewards.Add(CurrencyReward);

	AllEvents.Add(CreditsBoost.EventID, CreditsBoost);
}

void UMGEventCalendarSubsystem::InitializeDefaultPlaylists()
{
	FDateTime Now = FDateTime::UtcNow();

	// Quick Race
	FMGPlaylistEntry QuickRace;
	QuickRace.PlaylistID = FName("QuickRace");
	QuickRace.PlaylistName = FText::FromString("Quick Race");
	QuickRace.PlaylistDescription = FText::FromString("Jump into a race instantly");
	QuickRace.GameModeID = FName("CircuitRace");
	QuickRace.Rotation = EMGPlaylistRotation::Permanent;
	QuickRace.bIsActive = true;
	QuickRace.bIsFeatured = true;
	QuickRace.SortOrder = 0;
	AllPlaylists.Add(QuickRace);

	// Featured Daily
	FMGPlaylistEntry FeaturedDaily;
	FeaturedDaily.PlaylistID = FName("FeaturedDaily");
	FeaturedDaily.PlaylistName = FText::FromString("Daily Featured");
	FeaturedDaily.PlaylistDescription = FText::FromString("Today's featured track rotation");
	FeaturedDaily.GameModeID = FName("CircuitRace");
	FeaturedDaily.Rotation = EMGPlaylistRotation::Daily;
	FeaturedDaily.AvailableFrom = Now;
	FeaturedDaily.AvailableUntil = Now + FTimespan::FromDays(1);
	FeaturedDaily.bIsActive = true;
	FeaturedDaily.bIsFeatured = true;
	FeaturedDaily.SortOrder = 1;
	FeaturedDaily.BonusXPPercent = 25;
	AllPlaylists.Add(FeaturedDaily);

	// Weekly Sprint Series
	FMGPlaylistEntry WeeklySprint;
	WeeklySprint.PlaylistID = FName("WeeklySprint");
	WeeklySprint.PlaylistName = FText::FromString("Sprint Series");
	WeeklySprint.PlaylistDescription = FText::FromString("This week's sprint racing playlist");
	WeeklySprint.GameModeID = FName("SprintRace");
	WeeklySprint.Rotation = EMGPlaylistRotation::Weekly;
	WeeklySprint.AvailableFrom = Now;
	WeeklySprint.AvailableUntil = Now + FTimespan::FromDays(7);
	WeeklySprint.bIsActive = true;
	WeeklySprint.SortOrder = 2;
	WeeklySprint.BonusCurrencyPercent = 10;
	AllPlaylists.Add(WeeklySprint);

	// Drift Zone
	FMGPlaylistEntry DriftZone;
	DriftZone.PlaylistID = FName("DriftZone");
	DriftZone.PlaylistName = FText::FromString("Drift Zone");
	DriftZone.PlaylistDescription = FText::FromString("Master the art of drifting");
	DriftZone.GameModeID = FName("DriftRace");
	DriftZone.Rotation = EMGPlaylistRotation::Permanent;
	DriftZone.bIsActive = true;
	DriftZone.SortOrder = 3;
	AllPlaylists.Add(DriftZone);
}

void UMGEventCalendarSubsystem::InitializeDefaultSeason()
{
	FDateTime Now = FDateTime::UtcNow();

	CurrentSeason.SeasonID = FName("Season1");
	CurrentSeason.SeasonName = FText::FromString("Season 1: Midnight Launch");
	CurrentSeason.SeasonTheme = FText::FromString("The Beginning");
	CurrentSeason.SeasonNumber = 1;
	CurrentSeason.StartTime = FDateTime(Now.GetYear(), Now.GetMonth(), 1, 0, 0, 0);
	CurrentSeason.EndTime = CurrentSeason.StartTime + FTimespan::FromDays(90);
	CurrentSeason.MaxLevel = 100;
	CurrentSeason.SeasonColor = FLinearColor(0.5f, 0.0f, 1.0f, 1.0f);
}

void UMGEventCalendarSubsystem::ProcessRecurringEvents()
{
	FDateTime Now = FDateTime::UtcNow();

	for (auto& Pair : AllEvents)
	{
		FMGScheduledEvent& Event = Pair.Value;

		if (!Event.bIsRecurring)
		{
			continue;
		}

		// If event has completed and is recurring, schedule next occurrence
		if (Event.State == EMGEventState::Completed && Event.RecurrenceInterval.GetTotalSeconds() > 0)
		{
			FTimespan Duration = Event.EndTime - Event.StartTime;
			Event.StartTime = Event.EndTime + Event.RecurrenceInterval - Duration;
			Event.EndTime = Event.StartTime + Duration;
			Event.State = EMGEventState::Scheduled;

			// Clear notification tracking
			NotifiedEventStarts.Remove(Event.EventID);
			NotifiedEventEndings.Remove(Event.EventID);
		}
	}
}

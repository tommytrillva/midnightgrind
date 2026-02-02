// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGNarrativeSubsystem.cpp
 * @brief Implementation of the Live World Storytelling and Dynamic Narrative System
 */

#include "Narrative/MGNarrativeSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGNarrativeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadNarrativeData();
	InitializeStoryThreads();
}

void UMGNarrativeSubsystem::Deinitialize()
{
	SaveNarrativeData();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DialogueTimerHandle);
	}
	Super::Deinitialize();
}

void UMGNarrativeSubsystem::TriggerEvent(const FMGNarrativeEvent& Event)
{
	FMGNarrativeEvent NewEvent = Event;
	NewEvent.Timestamp = FDateTime::UtcNow();

	EventHistory.Add(NewEvent);

	// Keep history reasonable
	while (EventHistory.Num() > MaxEventHistory)
	{
		EventHistory.RemoveAt(0);
	}

	OnNarrativeEventTriggered.Broadcast(NewEvent);

	// Check if event advances any story threads
	for (FMGStoryThread& Thread : StoryThreads)
	{
		if (!Thread.bCompleted)
		{
			// Events can advance threads based on type
			if (Event.Type == EMGNarrativeEventType::RivalShowdown)
			{
				if (Thread.ThreadID == FName(TEXT("Thread_RivalStory")))
				{
					AdvanceThread(Thread.ThreadID, 10);
				}
			}
		}
	}

	SaveNarrativeData();
}

TArray<FMGNarrativeEvent> UMGNarrativeSubsystem::GetRecentEvents(int32 Count) const
{
	TArray<FMGNarrativeEvent> Recent;
	int32 StartIndex = FMath::Max(0, EventHistory.Num() - Count);

	for (int32 i = EventHistory.Num() - 1; i >= StartIndex; i--)
	{
		Recent.Add(EventHistory[i]);
	}
	return Recent;
}

TArray<FMGNarrativeEvent> UMGNarrativeSubsystem::GetEventsByType(EMGNarrativeEventType Type) const
{
	TArray<FMGNarrativeEvent> Result;
	for (const FMGNarrativeEvent& Event : EventHistory)
	{
		if (Event.Type == Type)
			Result.Add(Event);
	}
	return Result;
}

void UMGNarrativeSubsystem::TriggerContextualDialogue(const FMGNarrativeContext& Context)
{
	FMGDialogueLine Line = GenerateContextualLine(Context);
	if (!Line.DialogueText.IsEmpty())
	{
		QueueDialogue(Line);
	}
}

void UMGNarrativeSubsystem::PlayDialogue(const FMGDialogueLine& Line)
{
	bDialoguePlaying = true;
	OnDialogueTriggered.Broadcast(Line);

	// Set timer to mark dialogue as finished
	if (UWorld* World = GetWorld())
	{
		float Duration = Line.Duration > 0.0f ? Line.Duration : 3.0f;
		World->GetTimerManager().SetTimer(
			DialogueTimerHandle,
			this,
			&UMGNarrativeSubsystem::ProcessDialogueQueue,
			Duration,
			false
		);
	}
}

void UMGNarrativeSubsystem::QueueDialogue(const FMGDialogueLine& Line)
{
	DialogueQueue.Add(Line);

	if (!bDialoguePlaying)
	{
		ProcessDialogueQueue();
	}
}

TArray<FMGStoryThread> UMGNarrativeSubsystem::GetActiveThreads() const
{
	TArray<FMGStoryThread> Active;
	for (const FMGStoryThread& Thread : StoryThreads)
	{
		if (!Thread.bCompleted)
			Active.Add(Thread);
	}
	return Active;
}

void UMGNarrativeSubsystem::AdvanceThread(FName ThreadID, int32 Progress)
{
	FMGStoryThread* Thread = FindThread(ThreadID);
	if (!Thread || Thread->bCompleted)
		return;

	Thread->Progress = FMath::Min(Thread->Progress + Progress, Thread->MaxProgress);
	OnStoryThreadAdvanced.Broadcast(*Thread);

	if (Thread->Progress >= Thread->MaxProgress)
	{
		Thread->bCompleted = true;
		OnStoryThreadCompleted.Broadcast(*Thread);
	}

	SaveNarrativeData();
}

FMGStoryThread UMGNarrativeSubsystem::GetThread(FName ThreadID) const
{
	const FMGStoryThread* Thread = StoryThreads.FindByPredicate(
		[ThreadID](const FMGStoryThread& T) { return T.ThreadID == ThreadID; });
	return Thread ? *Thread : FMGStoryThread();
}

void UMGNarrativeSubsystem::OnRaceStart(const TArray<FString>& RacerIDs, FName TrackID)
{
	FMGDialogueLine Line;
	Line.Source = EMGDialogueSource::RadioHost;
	Line.SpeakerName = FText::FromString(TEXT("DJ Midnight"));
	Line.DialogueText = FText::FromString(TEXT("Alright racers, engines hot and ready. Let's see what you've got tonight."));
	Line.Duration = 4.0f;
	QueueDialogue(Line);
}

void UMGNarrativeSubsystem::OnPositionChange(int32 OldPosition, int32 NewPosition)
{
	if (NewPosition < OldPosition && NewPosition <= 3)
	{
		FMGDialogueLine Line;
		Line.Source = EMGDialogueSource::RadioHost;
		Line.SpeakerName = FText::FromString(TEXT("DJ Midnight"));

		if (NewPosition == 1)
			Line.DialogueText = FText::FromString(TEXT("Taking the lead! Show them what you're made of!"));
		else
			Line.DialogueText = FText::FromString(TEXT("Moving up! Keep pushing!"));

		Line.Duration = 2.5f;
		QueueDialogue(Line);
	}
}

void UMGNarrativeSubsystem::OnRivalPassed(const FString& RivalID)
{
	FMGDialogueLine Line;
	Line.Source = EMGDialogueSource::RadioHost;
	Line.SpeakerName = FText::FromString(TEXT("DJ Midnight"));
	Line.DialogueText = FText::FromString(TEXT("Oh! You just passed your rival! That's gonna sting."));
	Line.Duration = 3.0f;
	QueueDialogue(Line);

	// Create narrative event
	FMGNarrativeEvent Event;
	Event.EventID = FName(*FString::Printf(TEXT("RivalPass_%s"), *FGuid::NewGuid().ToString()));
	Event.Type = EMGNarrativeEventType::RivalShowdown;
	Event.Title = FText::FromString(TEXT("Rival Passed"));
	Event.Description = FText::FromString(TEXT("You passed your rival during a race"));
	Event.InvolvedPlayers.Add(RivalID);
	TriggerEvent(Event);
}

void UMGNarrativeSubsystem::OnRaceFinish(int32 FinalPosition, bool bWasCloseRace)
{
	FMGDialogueLine Line;
	Line.Source = EMGDialogueSource::RadioHost;
	Line.SpeakerName = FText::FromString(TEXT("DJ Midnight"));

	if (FinalPosition == 1)
	{
		if (bWasCloseRace)
			Line.DialogueText = FText::FromString(TEXT("Photo finish victory! That was incredible!"));
		else
			Line.DialogueText = FText::FromString(TEXT("Dominant performance. You owned that race."));
	}
	else if (FinalPosition <= 3)
	{
		Line.DialogueText = FText::FromString(TEXT("Solid podium finish. Keep grinding."));
	}
	else
	{
		Line.DialogueText = FText::FromString(TEXT("Not your night, but there's always the next race."));
	}

	Line.Duration = 3.5f;
	QueueDialogue(Line);
}

void UMGNarrativeSubsystem::OnCommunityEventStarted(FName EventID, const FText& EventName)
{
	FMGDialogueLine Line;
	Line.Source = EMGDialogueSource::RadioHost;
	Line.SpeakerName = FText::FromString(TEXT("DJ Midnight"));
	Line.DialogueText = FText::Format(FText::FromString(TEXT("Big news, racers! {0} just kicked off. Get in on the action!")), EventName);
	Line.Duration = 4.0f;
	QueueDialogue(Line);

	FMGNarrativeEvent Event;
	Event.EventID = EventID;
	Event.Type = EMGNarrativeEventType::CommunityEvent;
	Event.Title = EventName;
	Event.Description = FText::FromString(TEXT("A community event has started"));
	TriggerEvent(Event);
}

void UMGNarrativeSubsystem::OnCrewAchievement(FName CrewID, const FText& Achievement)
{
	FMGNarrativeEvent Event;
	Event.EventID = FName(*FString::Printf(TEXT("CrewAchieve_%s"), *FGuid::NewGuid().ToString()));
	Event.Type = EMGNarrativeEventType::CrewMoment;
	Event.Title = Achievement;
	Event.RelatedCrew = CrewID;
	TriggerEvent(Event);
}

void UMGNarrativeSubsystem::LoadNarrativeData()
{
	// Would load from cloud save
}

void UMGNarrativeSubsystem::SaveNarrativeData()
{
	// Would save to cloud save
}

void UMGNarrativeSubsystem::InitializeStoryThreads()
{
	// Rival story thread
	FMGStoryThread RivalThread;
	RivalThread.ThreadID = FName(TEXT("Thread_RivalStory"));
	RivalThread.ThreadName = FText::FromString(TEXT("The Rivalry"));
	RivalThread.CurrentState = FText::FromString(TEXT("Your rivalries are just beginning..."));
	RivalThread.MaxProgress = 100;
	StoryThreads.Add(RivalThread);

	// Crew story thread
	FMGStoryThread CrewThread;
	CrewThread.ThreadID = FName(TEXT("Thread_CrewStory"));
	CrewThread.ThreadName = FText::FromString(TEXT("Crew Chronicles"));
	CrewThread.CurrentState = FText::FromString(TEXT("Find your crew and rise together"));
	CrewThread.MaxProgress = 100;
	StoryThreads.Add(CrewThread);

	// Legend story thread
	FMGStoryThread LegendThread;
	LegendThread.ThreadID = FName(TEXT("Thread_LegendStory"));
	LegendThread.ThreadName = FText::FromString(TEXT("Path to Legend"));
	LegendThread.CurrentState = FText::FromString(TEXT("Every legend starts somewhere..."));
	LegendThread.MaxProgress = 200;
	StoryThreads.Add(LegendThread);
}

void UMGNarrativeSubsystem::ProcessDialogueQueue()
{
	bDialoguePlaying = false;

	if (DialogueQueue.Num() > 0)
	{
		FMGDialogueLine NextLine = DialogueQueue[0];
		DialogueQueue.RemoveAt(0);
		PlayDialogue(NextLine);
	}
}

FMGDialogueLine UMGNarrativeSubsystem::GenerateContextualLine(const FMGNarrativeContext& Context)
{
	FMGDialogueLine Line;
	Line.Source = EMGDialogueSource::RadioHost;
	Line.SpeakerName = FText::FromString(TEXT("DJ Midnight"));
	Line.Duration = 3.0f;

	if (Context.bNemesisInRace)
	{
		Line.DialogueText = FText::FromString(TEXT("Your nemesis is in this race. Time to settle the score."));
	}
	else if (Context.bRivalInRace)
	{
		Line.DialogueText = FText::FromString(TEXT("Familiar faces out there. Don't let them get ahead."));
	}
	else if (Context.bOnWinStreak)
	{
		Line.DialogueText = FText::FromString(TEXT("You're on fire! Keep that streak alive!"));
	}
	else if (Context.bJustWon)
	{
		Line.DialogueText = FText::FromString(TEXT("Winner winner! The streets remember this."));
	}
	else if (Context.bJustLost)
	{
		Line.DialogueText = FText::FromString(TEXT("Shake it off. Get back out there and prove yourself."));
	}
	else
	{
		Line.DialogueText = FText::GetEmpty();
	}

	return Line;
}

FMGStoryThread* UMGNarrativeSubsystem::FindThread(FName ThreadID)
{
	return StoryThreads.FindByPredicate(
		[ThreadID](const FMGStoryThread& T) { return T.ThreadID == ThreadID; });
}

// Copyright Midnight Grind. All Rights Reserved.

#include "StreamerMode/MGStreamerModeSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGStreamerModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSettings();
}

void UMGStreamerModeSubsystem::Deinitialize()
{
	SaveSettings();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollTimerHandle);
	}
	Super::Deinitialize();
}

void UMGStreamerModeSubsystem::EnableStreamerMode(const FMGStreamerSettings& Settings)
{
	StreamerSettings = Settings;
	StreamerSettings.bStreamerModeEnabled = true;
	StreamStartTime = FDateTime::UtcNow();
	StreamStats = FMGStreamStats();
	SaveSettings();
}

void UMGStreamerModeSubsystem::DisableStreamerMode()
{
	StreamerSettings.bStreamerModeEnabled = false;
	DisconnectFromPlatform();
	SaveSettings();
}

void UMGStreamerModeSubsystem::UpdateStreamerSettings(const FMGStreamerSettings& Settings)
{
	bool bWasEnabled = StreamerSettings.bStreamerModeEnabled;
	StreamerSettings = Settings;
	StreamerSettings.bStreamerModeEnabled = bWasEnabled;
	SaveSettings();
}

void UMGStreamerModeSubsystem::ConnectToPlatform(EMGStreamPlatform Platform, const FString& AuthToken)
{
	// Would establish OAuth connection to platform
	ConnectedPlatform = Platform;
	OnStreamConnected.Broadcast(Platform);
}

void UMGStreamerModeSubsystem::DisconnectFromPlatform()
{
	if (ConnectedPlatform != EMGStreamPlatform::None)
	{
		ConnectedPlatform = EMGStreamPlatform::None;
		OnStreamDisconnected.Broadcast();
	}
}

void UMGStreamerModeSubsystem::ProcessViewerCommand(const FString& ViewerName, const FString& Command, bool bIsSubscriber)
{
	if (!StreamerSettings.bStreamerModeEnabled || !StreamerSettings.bAllowViewerInteractions)
		return;

	if (StreamerSettings.bSubOnlyInteractions && !bIsSubscriber)
		return;

	ProcessChatCommand(ViewerName, Command, bIsSubscriber);
}

void UMGStreamerModeSubsystem::TriggerInteraction(const FMGViewerInteraction& Interaction)
{
	if (!CanTriggerInteraction())
		return;

	LastInteractionTime = FDateTime::UtcNow();
	StreamStats.TotalInteractions++;

	OnViewerInteraction.Broadcast(Interaction);

	// Handle interaction types
	switch (Interaction.Type)
	{
	case EMGViewerInteractionType::BoostPlayer:
		// Would trigger temporary speed boost
		break;
	case EMGViewerInteractionType::SlowPlayer:
		// Would trigger temporary slowdown
		break;
	case EMGViewerInteractionType::SpawnObstacle:
		// Would spawn obstacle on track
		break;
	case EMGViewerInteractionType::TriggerEvent:
		// Would trigger random event
		break;
	default:
		break;
	}
}

bool UMGStreamerModeSubsystem::CanTriggerInteraction() const
{
	if (!StreamerSettings.bStreamerModeEnabled || !StreamerSettings.bAllowViewerInteractions)
		return false;

	FTimespan TimeSinceLastInteraction = FDateTime::UtcNow() - LastInteractionTime;
	return TimeSinceLastInteraction.GetTotalSeconds() >= StreamerSettings.InteractionCooldownSeconds;
}

float UMGStreamerModeSubsystem::GetInteractionCooldownRemaining() const
{
	FTimespan TimeSinceLastInteraction = FDateTime::UtcNow() - LastInteractionTime;
	float Remaining = StreamerSettings.InteractionCooldownSeconds - TimeSinceLastInteraction.GetTotalSeconds();
	return FMath::Max(0.0f, Remaining);
}

void UMGStreamerModeSubsystem::StartPoll(const FText& Question, const TArray<FText>& Options, int32 DurationSeconds)
{
	if (ActivePoll.bIsActive)
		EndPollEarly();

	ActivePoll = FMGViewerPoll();
	ActivePoll.PollID = FGuid::NewGuid().ToString();
	ActivePoll.Question = Question;
	ActivePoll.Options = Options;
	ActivePoll.Votes.SetNum(Options.Num());
	ActivePoll.DurationSeconds = DurationSeconds;
	ActivePoll.TimeRemaining = DurationSeconds;
	ActivePoll.bIsActive = true;

	VotedViewers.Empty();

	OnPollStarted.Broadcast(ActivePoll);

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGStreamerModeSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			PollTimerHandle,
			[WeakThis]() { if (WeakThis.IsValid()) WeakThis->UpdatePoll(1.0f); },
			1.0f,
			true
		);
	}
}

void UMGStreamerModeSubsystem::RegisterVote(const FString& ViewerName, int32 OptionIndex)
{
	if (!ActivePoll.bIsActive)
		return;

	if (OptionIndex < 0 || OptionIndex >= ActivePoll.Options.Num())
		return;

	// Prevent double voting
	if (VotedViewers.Contains(ViewerName))
		return;

	VotedViewers.Add(ViewerName);
	ActivePoll.Votes[OptionIndex]++;
	StreamStats.TotalVotes++;
}

void UMGStreamerModeSubsystem::EndPollEarly()
{
	if (ActivePoll.bIsActive)
	{
		FinishPoll();
	}
}

void UMGStreamerModeSubsystem::StartTrackVote(const TArray<FName>& TrackOptions)
{
	TArray<FText> Options;
	for (const FName& TrackID : TrackOptions)
	{
		Options.Add(FText::FromName(TrackID));
	}
	StartPoll(FText::FromString(TEXT("Chat picks the next track!")), Options, 45);
}

void UMGStreamerModeSubsystem::StartWeatherVote()
{
	TArray<FText> Options;
	Options.Add(FText::FromString(TEXT("Clear Night")));
	Options.Add(FText::FromString(TEXT("Rainy")));
	Options.Add(FText::FromString(TEXT("Foggy")));
	Options.Add(FText::FromString(TEXT("Stormy")));
	StartPoll(FText::FromString(TEXT("Chat picks the weather!")), Options, 30);
}

void UMGStreamerModeSubsystem::AllowChaosMode(int32 DurationSeconds)
{
	bChaosModeActive = true;

	// Would enable rapid viewer interactions

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGStreamerModeSubsystem> WeakThis(this);
		FTimerHandle ChaosTimer;
		World->GetTimerManager().SetTimer(
			ChaosTimer,
			[WeakThis]() { if (WeakThis.IsValid()) WeakThis->bChaosModeActive = false; },
			DurationSeconds,
			false
		);
	}
}

void UMGStreamerModeSubsystem::UpdateViewerCount(int32 Count)
{
	StreamStats.CurrentViewers = Count;
	StreamStats.StreamDuration = FDateTime::UtcNow() - StreamStartTime;
}

void UMGStreamerModeSubsystem::UpdatePoll(float DeltaTime)
{
	if (!ActivePoll.bIsActive)
		return;

	ActivePoll.TimeRemaining -= DeltaTime;

	if (ActivePoll.TimeRemaining <= 0.0f)
	{
		FinishPoll();
	}
}

void UMGStreamerModeSubsystem::FinishPoll()
{
	if (!ActivePoll.bIsActive)
		return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollTimerHandle);
	}

	// Find winner
	int32 MaxVotes = 0;
	int32 WinnerIndex = 0;
	for (int32 i = 0; i < ActivePoll.Votes.Num(); i++)
	{
		if (ActivePoll.Votes[i] > MaxVotes)
		{
			MaxVotes = ActivePoll.Votes[i];
			WinnerIndex = i;
		}
	}

	ActivePoll.WinningOptionIndex = WinnerIndex;
	ActivePoll.bIsActive = false;

	OnPollEnded.Broadcast(ActivePoll, WinnerIndex);
}

void UMGStreamerModeSubsystem::ProcessChatCommand(const FString& ViewerName, const FString& Command, bool bIsSub)
{
	FString LowerCommand = Command.ToLower();

	// Poll voting
	if (ActivePoll.bIsActive)
	{
		if (LowerCommand == TEXT("1") || LowerCommand == TEXT("!1"))
			RegisterVote(ViewerName, 0);
		else if (LowerCommand == TEXT("2") || LowerCommand == TEXT("!2"))
			RegisterVote(ViewerName, 1);
		else if (LowerCommand == TEXT("3") || LowerCommand == TEXT("!3"))
			RegisterVote(ViewerName, 2);
		else if (LowerCommand == TEXT("4") || LowerCommand == TEXT("!4"))
			RegisterVote(ViewerName, 3);
	}

	// Interactions (sub-only if configured)
	if (bIsSub || !StreamerSettings.bSubOnlyInteractions)
	{
		if (LowerCommand == TEXT("!boost") && CanTriggerInteraction())
		{
			FMGViewerInteraction Interaction;
			Interaction.Type = EMGViewerInteractionType::BoostPlayer;
			Interaction.ViewerName = ViewerName;
			Interaction.bIsSubscriber = bIsSub;
			Interaction.Timestamp = FDateTime::UtcNow();
			TriggerInteraction(Interaction);
		}
		else if (LowerCommand == TEXT("!slow") && CanTriggerInteraction())
		{
			FMGViewerInteraction Interaction;
			Interaction.Type = EMGViewerInteractionType::SlowPlayer;
			Interaction.ViewerName = ViewerName;
			Interaction.bIsSubscriber = bIsSub;
			Interaction.Timestamp = FDateTime::UtcNow();
			TriggerInteraction(Interaction);
		}
		else if (LowerCommand == TEXT("!chaos") && bIsSub && CanTriggerInteraction())
		{
			FMGViewerInteraction Interaction;
			Interaction.Type = EMGViewerInteractionType::TriggerEvent;
			Interaction.ViewerName = ViewerName;
			Interaction.bIsSubscriber = bIsSub;
			Interaction.Timestamp = FDateTime::UtcNow();
			TriggerInteraction(Interaction);
		}
	}
}

void UMGStreamerModeSubsystem::LoadSettings()
{
	// Would load from local storage
}

void UMGStreamerModeSubsystem::SaveSettings()
{
	// Would save to local storage
}

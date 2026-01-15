// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGGameStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"

void UMGGameStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentState = EMGGameState::Boot;
	StateEnterTime = 0.0f;
}

void UMGGameStateSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// STATE MANAGEMENT
// ==========================================

bool UMGGameStateSubsystem::RequestStateTransition(const FMGStateTransition& Transition)
{
	OnStateTransitionRequested.Broadcast(CurrentState, Transition.TargetState);

	// Validate transition
	FString BlockReason;
	if (!Transition.bForce && !ValidateTransition(CurrentState, Transition.TargetState, BlockReason))
	{
		OnStateTransitionBlocked.Broadcast(BlockReason);
		return false;
	}

	ExecuteTransition(Transition);
	return true;
}

bool UMGGameStateSubsystem::GoToState(EMGGameState NewState)
{
	FMGStateTransition Transition;
	Transition.TargetState = NewState;
	return RequestStateTransition(Transition);
}

bool UMGGameStateSubsystem::CanTransitionTo(EMGGameState TargetState) const
{
	FString Reason;
	return ValidateTransition(CurrentState, TargetState, Reason);
}

TArray<EMGGameState> UMGGameStateSubsystem::GetValidTransitions() const
{
	return GetValidTransitionsForState(CurrentState);
}

float UMGGameStateSubsystem::GetTimeInCurrentState() const
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		return World->GetTimeSeconds() - StateEnterTime;
	}
	return 0.0f;
}

FText UMGGameStateSubsystem::GetStateDisplayName(EMGGameState State) const
{
	switch (State)
	{
		case EMGGameState::Boot: return FText::FromString("Loading");
		case EMGGameState::MainMenu: return FText::FromString("Main Menu");
		case EMGGameState::Garage: return FText::FromString("Garage");
		case EMGGameState::LobbyBrowser: return FText::FromString("Find Race");
		case EMGGameState::InLobby: return FText::FromString("Lobby");
		case EMGGameState::Loading: return FText::FromString("Loading");
		case EMGGameState::PreRace: return FText::FromString("Starting");
		case EMGGameState::Racing: return FText::FromString("Racing");
		case EMGGameState::PostRace: return FText::FromString("Results");
		case EMGGameState::Replay: return FText::FromString("Replay");
		case EMGGameState::PhotoMode: return FText::FromString("Photo Mode");
		case EMGGameState::Leaderboards: return FText::FromString("Leaderboards");
		case EMGGameState::Settings: return FText::FromString("Settings");
		default: return FText::FromString("Unknown");
	}
}

// ==========================================
// COMMON TRANSITIONS
// ==========================================

void UMGGameStateSubsystem::GoToMainMenu()
{
	FMGStateTransition Transition;
	Transition.TargetState = EMGGameState::MainMenu;
	Transition.LevelName = MainMenuLevel;
	RequestStateTransition(Transition);
}

void UMGGameStateSubsystem::GoToGarage()
{
	FMGStateTransition Transition;
	Transition.TargetState = EMGGameState::Garage;
	Transition.LevelName = GarageLevel;
	RequestStateTransition(Transition);
}

void UMGGameStateSubsystem::GoToLobbyBrowser()
{
	GoToState(EMGGameState::LobbyBrowser);
}

void UMGGameStateSubsystem::EnterLobby(const FString& SessionID)
{
	FMGStateTransition Transition;
	Transition.TargetState = EMGGameState::InLobby;
	Transition.ContextData.Add(FName("SessionID"), SessionID);
	RequestStateTransition(Transition);
}

void UMGGameStateSubsystem::StartRaceLoading(FName TrackID)
{
	FMGStateTransition Transition;
	Transition.TargetState = EMGGameState::Loading;
	Transition.LevelName = TrackID;
	Transition.ContextData.Add(FName("TrackID"), TrackID.ToString());
	RequestStateTransition(Transition);
}

void UMGGameStateSubsystem::BeginPreRace()
{
	GoToState(EMGGameState::PreRace);
}

void UMGGameStateSubsystem::StartRacing()
{
	GoToState(EMGGameState::Racing);
}

void UMGGameStateSubsystem::EndRace()
{
	GoToState(EMGGameState::PostRace);
}

void UMGGameStateSubsystem::EnterReplayMode()
{
	GoToState(EMGGameState::Replay);
}

void UMGGameStateSubsystem::ExitReplayMode()
{
	// Return to post-race or main menu
	if (PreviousState == EMGGameState::PostRace)
	{
		GoToState(EMGGameState::PostRace);
	}
	else
	{
		GoToMainMenu();
	}
}

void UMGGameStateSubsystem::EnterPhotoMode()
{
	GoToState(EMGGameState::PhotoMode);
}

void UMGGameStateSubsystem::ExitPhotoMode()
{
	// Return to previous state (usually Racing or Replay)
	GoToState(PreviousState);
}

// ==========================================
// CONTEXT DATA
// ==========================================

void UMGGameStateSubsystem::SetContextData(FName Key, const FString& Value)
{
	ContextData.Add(Key, Value);
}

FString UMGGameStateSubsystem::GetContextData(FName Key) const
{
	if (const FString* Value = ContextData.Find(Key))
	{
		return *Value;
	}
	return FString();
}

void UMGGameStateSubsystem::ClearContextData()
{
	ContextData.Empty();
}

FName UMGGameStateSubsystem::GetCurrentTrackID() const
{
	FString TrackStr = GetContextData(FName("TrackID"));
	return TrackStr.IsEmpty() ? NAME_None : FName(*TrackStr);
}

FString UMGGameStateSubsystem::GetCurrentSessionID() const
{
	return GetContextData(FName("SessionID"));
}

// ==========================================
// INTERNAL
// ==========================================

void UMGGameStateSubsystem::ExecuteTransition(const FMGStateTransition& Transition)
{
	// Store context data
	for (const auto& Pair : Transition.ContextData)
	{
		ContextData.Add(Pair.Key, Pair.Value);
	}

	// Check if we need to load a level
	if (!Transition.LevelName.IsNone())
	{
		PendingTransition = Transition;
		LoadLevelAsync(Transition.LevelName, Transition.TargetState);
	}
	else
	{
		// Direct transition
		EnterState(Transition.TargetState, Transition.ContextData);
	}
}

bool UMGGameStateSubsystem::ValidateTransition(EMGGameState FromState, EMGGameState ToState, FString& OutReason) const
{
	// Get valid transitions
	TArray<EMGGameState> ValidTransitions = GetValidTransitionsForState(FromState);

	if (!ValidTransitions.Contains(ToState))
	{
		OutReason = FString::Printf(TEXT("Cannot transition from %s to %s"),
			*GetStateDisplayName(FromState).ToString(),
			*GetStateDisplayName(ToState).ToString());
		return false;
	}

	// Additional validation based on state
	switch (ToState)
	{
		case EMGGameState::Racing:
			// Must have a track loaded
			if (GetCurrentTrackID().IsNone())
			{
				OutReason = TEXT("No track loaded");
				return false;
			}
			break;

		case EMGGameState::InLobby:
			// Must have session ID
			if (GetCurrentSessionID().IsEmpty())
			{
				OutReason = TEXT("No session specified");
				return false;
			}
			break;

		default:
			break;
	}

	return true;
}

TArray<EMGGameState> UMGGameStateSubsystem::GetValidTransitionsForState(EMGGameState State) const
{
	TArray<EMGGameState> Valid;

	switch (State)
	{
		case EMGGameState::Boot:
			Valid.Add(EMGGameState::MainMenu);
			break;

		case EMGGameState::MainMenu:
			Valid.Add(EMGGameState::Garage);
			Valid.Add(EMGGameState::LobbyBrowser);
			Valid.Add(EMGGameState::Settings);
			Valid.Add(EMGGameState::Leaderboards);
			Valid.Add(EMGGameState::Loading); // Quick play
			break;

		case EMGGameState::Garage:
			Valid.Add(EMGGameState::MainMenu);
			Valid.Add(EMGGameState::Settings);
			break;

		case EMGGameState::LobbyBrowser:
			Valid.Add(EMGGameState::MainMenu);
			Valid.Add(EMGGameState::InLobby);
			break;

		case EMGGameState::InLobby:
			Valid.Add(EMGGameState::MainMenu);
			Valid.Add(EMGGameState::LobbyBrowser);
			Valid.Add(EMGGameState::Loading);
			break;

		case EMGGameState::Loading:
			Valid.Add(EMGGameState::PreRace);
			Valid.Add(EMGGameState::MainMenu); // Cancel
			break;

		case EMGGameState::PreRace:
			Valid.Add(EMGGameState::Racing);
			Valid.Add(EMGGameState::MainMenu); // Disconnect
			break;

		case EMGGameState::Racing:
			Valid.Add(EMGGameState::PostRace);
			Valid.Add(EMGGameState::PhotoMode);
			Valid.Add(EMGGameState::MainMenu); // Quit race
			break;

		case EMGGameState::PostRace:
			Valid.Add(EMGGameState::MainMenu);
			Valid.Add(EMGGameState::InLobby); // Rematch
			Valid.Add(EMGGameState::Loading); // Restart
			Valid.Add(EMGGameState::Replay);
			Valid.Add(EMGGameState::Leaderboards);
			break;

		case EMGGameState::Replay:
			Valid.Add(EMGGameState::PostRace);
			Valid.Add(EMGGameState::MainMenu);
			Valid.Add(EMGGameState::PhotoMode);
			break;

		case EMGGameState::PhotoMode:
			Valid.Add(EMGGameState::Racing);
			Valid.Add(EMGGameState::Replay);
			Valid.Add(EMGGameState::PostRace);
			break;

		case EMGGameState::Leaderboards:
			Valid.Add(EMGGameState::MainMenu);
			Valid.Add(EMGGameState::PostRace);
			break;

		case EMGGameState::Settings:
			Valid.Add(EMGGameState::MainMenu);
			Valid.Add(EMGGameState::Garage);
			break;

		default:
			break;
	}

	return Valid;
}

void UMGGameStateSubsystem::OnLevelLoaded()
{
	bIsLoading = false;
	LoadingProgress = 1.0f;

	OnLoadingCompleted.Broadcast();

	// Complete the pending transition
	EnterState(PendingTransition.TargetState, PendingTransition.ContextData);
	PendingTransition = FMGStateTransition();
}

void UMGGameStateSubsystem::UpdateLoadingProgress(float Progress, const FText& StatusText)
{
	LoadingProgress = Progress;
	LoadingStatusText = StatusText;
}

void UMGGameStateSubsystem::EnterState(EMGGameState NewState, const TMap<FName, FString>& TransitionData)
{
	// Exit current state
	ExitState(CurrentState);

	// Update state
	PreviousState = CurrentState;
	CurrentState = NewState;

	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		StateEnterTime = World->GetTimeSeconds();
	}

	// Broadcast change
	FMGStateChangeContext Context;
	Context.PreviousState = PreviousState;
	Context.NewState = NewState;
	Context.ContextData = TransitionData;
	Context.TimeInPreviousState = GetTimeInCurrentState();

	OnGameStateChanged.Broadcast(Context);
}

void UMGGameStateSubsystem::ExitState(EMGGameState OldState)
{
	// Cleanup based on state being exited
	switch (OldState)
	{
		case EMGGameState::Racing:
			// Could pause/cleanup race systems
			break;

		case EMGGameState::PhotoMode:
			// Re-enable HUD, etc.
			break;

		default:
			break;
	}
}

void UMGGameStateSubsystem::LoadLevelAsync(FName LevelName, EMGGameState TargetState)
{
	bIsLoading = true;
	LoadingProgress = 0.0f;
	LoadingStatusText = FText::FromString(TEXT("Loading..."));
	PostLoadState = TargetState;

	// Enter loading state
	EnterState(EMGGameState::Loading, TMap<FName, FString>());

	OnLoadingStarted.Broadcast();

	// Start async level load
	UGameplayStatics::OpenLevel(GetGameInstance(), LevelName);

	// In a full implementation, we'd use OpenLevelBySoftObjectPtr with async loading
	// and track progress. For now, simulate immediate completion.
	OnLevelLoaded();
}

void UMGGameStateSubsystem::OnAsyncLoadComplete()
{
	OnLevelLoaded();
}

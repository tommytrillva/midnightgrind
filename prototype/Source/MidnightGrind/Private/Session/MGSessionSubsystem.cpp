// Copyright Midnight Grind. All Rights Reserved.

#include "Session/MGSessionSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Generate local player ID (would come from platform)
	LocalPlayerID = FGuid::NewGuid().ToString();
}

void UMGSessionSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchmakingTimerHandle);
	}
	Super::Deinitialize();
}

void UMGSessionSubsystem::CreateSession(const FMGSessionInfo& Settings)
{
	CurrentSession = Settings;
	CurrentSession.SessionID = FGuid::NewGuid().ToString();
	CurrentSession.HostPlayerID = LocalPlayerID;
	CurrentSession.State = EMGSessionState::InLobby;
	CurrentSession.CurrentPlayers = 1;
	bIsHost = true;

	// Add self to lobby
	FMGLobbyPlayer LocalPlayer;
	LocalPlayer.PlayerID = LocalPlayerID;
	LocalPlayer.DisplayName = FText::FromString(TEXT("Player"));
	LocalPlayer.bIsHost = true;
	LocalPlayer.bIsReady = false;
	LobbyPlayers.Add(LocalPlayer);

	SetSessionState(EMGSessionState::InLobby);
}

void UMGSessionSubsystem::JoinSession(const FString& SessionID)
{
	// Would connect to actual session
	for (const FMGSessionInfo& Session : AvailableSessions)
	{
		if (Session.SessionID == SessionID)
		{
			CurrentSession = Session;
			CurrentSession.State = EMGSessionState::InLobby;
			bIsHost = false;

			// Add self to lobby
			FMGLobbyPlayer LocalPlayer;
			LocalPlayer.PlayerID = LocalPlayerID;
			LocalPlayer.DisplayName = FText::FromString(TEXT("Player"));
			LocalPlayer.bIsHost = false;
			LocalPlayer.bIsReady = false;
			LobbyPlayers.Add(LocalPlayer);

			SetSessionState(EMGSessionState::InLobby);
			OnPlayerJoined.Broadcast(LocalPlayer);
			return;
		}
	}
}

void UMGSessionSubsystem::LeaveSession()
{
	if (bIsHost)
	{
		// Would notify all players and close session
	}

	LobbyPlayers.Empty();
	CurrentSession = FMGSessionInfo();
	bIsHost = false;
	SetSessionState(EMGSessionState::None);
}

void UMGSessionSubsystem::StartMatchmaking(const FMGMatchmakingSettings& Settings)
{
	CurrentMatchmakingSettings = Settings;
	MatchmakingTime = 0.0f;
	PlayersInQueue = FMath::RandRange(50, 200);

	SetMatchmakingState(EMGMatchmakingState::Searching);

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGSessionSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			MatchmakingTimerHandle,
			[WeakThis]() { if (WeakThis.IsValid()) WeakThis->UpdateMatchmaking(1.0f); },
			1.0f,
			true
		);
	}
}

void UMGSessionSubsystem::CancelMatchmaking()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchmakingTimerHandle);
	}

	SetMatchmakingState(EMGMatchmakingState::Cancelled);
	SetMatchmakingState(EMGMatchmakingState::Idle);
}

void UMGSessionSubsystem::QuickPlay(FName PlaylistID)
{
	FMGMatchmakingSettings Settings;
	Settings.PlaylistID = PlaylistID;
	Settings.bRankedOnly = false;
	Settings.MaxPingMS = 150;
	Settings.bAllowCrossplay = true;

	StartMatchmaking(Settings);
}

void UMGSessionSubsystem::QuickPlayRanked()
{
	FMGMatchmakingSettings Settings;
	Settings.bRankedOnly = true;
	Settings.MaxPingMS = 100;
	Settings.SkillRange = 100; // Tighter skill matching for ranked

	StartMatchmaking(Settings);
}

void UMGSessionSubsystem::SetReady(bool bReady)
{
	for (FMGLobbyPlayer& Player : LobbyPlayers)
	{
		if (Player.PlayerID == LocalPlayerID)
		{
			Player.bIsReady = bReady;
			break;
		}
	}

	if (AreAllPlayersReady())
	{
		OnAllPlayersReady.Broadcast();
	}
}

void UMGSessionSubsystem::SetVehicle(FName VehicleID, int32 PI)
{
	for (FMGLobbyPlayer& Player : LobbyPlayers)
	{
		if (Player.PlayerID == LocalPlayerID)
		{
			Player.SelectedVehicle = VehicleID;
			Player.VehiclePI = PI;
			break;
		}
	}
}

void UMGSessionSubsystem::SetTeam(int32 TeamIndex)
{
	for (FMGLobbyPlayer& Player : LobbyPlayers)
	{
		if (Player.PlayerID == LocalPlayerID)
		{
			Player.TeamIndex = TeamIndex;
			break;
		}
	}
}

bool UMGSessionSubsystem::AreAllPlayersReady() const
{
	if (LobbyPlayers.Num() == 0)
		return false;

	for (const FMGLobbyPlayer& Player : LobbyPlayers)
	{
		if (!Player.bIsReady && !Player.bIsHost)
			return false;
	}
	return true;
}

int32 UMGSessionSubsystem::GetReadyPlayerCount() const
{
	int32 Count = 0;
	for (const FMGLobbyPlayer& Player : LobbyPlayers)
	{
		if (Player.bIsReady || Player.bIsHost)
			Count++;
	}
	return Count;
}

void UMGSessionSubsystem::SetTrack(FName TrackID)
{
	if (!bIsHost)
		return;

	CurrentSession.CurrentTrackID = TrackID;
}

void UMGSessionSubsystem::SetGameMode(FName ModeID)
{
	if (!bIsHost)
		return;

	CurrentSession.GameModeID = ModeID;
}

void UMGSessionSubsystem::SetPrivacy(EMGLobbyPrivacy Privacy)
{
	if (!bIsHost)
		return;

	CurrentSession.Privacy = Privacy;
}

void UMGSessionSubsystem::KickPlayer(const FString& PlayerID)
{
	if (!bIsHost)
		return;

	LobbyPlayers.RemoveAll([&PlayerID](const FMGLobbyPlayer& Player)
	{
		return Player.PlayerID == PlayerID;
	});

	CurrentSession.CurrentPlayers = LobbyPlayers.Num();
	OnPlayerLeft.Broadcast(PlayerID);
}

void UMGSessionSubsystem::StartRace()
{
	if (!bIsHost)
		return;

	SetSessionState(EMGSessionState::Starting);

	// Would countdown then start
	SetSessionState(EMGSessionState::InProgress);
}

void UMGSessionSubsystem::RandomizeTrack()
{
	if (!bIsHost)
		return;

	// Would pick random track from available tracks
	// For now, just set a placeholder
	CurrentSession.CurrentTrackID = FName(TEXT("Track_Downtown"));
}

void UMGSessionSubsystem::CreateParty()
{
	CurrentParty = FMGPartyInfo();
	CurrentParty.PartyID = FGuid::NewGuid().ToString();

	FMGPartyMember Leader;
	Leader.PlayerID = LocalPlayerID;
	Leader.DisplayName = FText::FromString(TEXT("Player"));
	Leader.bIsLeader = true;
	CurrentParty.Members.Add(Leader);

	OnPartyUpdated.Broadcast(CurrentParty);
}

void UMGSessionSubsystem::JoinParty(const FString& PartyID)
{
	// Would connect to party
	CurrentParty.PartyID = PartyID;

	FMGPartyMember Member;
	Member.PlayerID = LocalPlayerID;
	Member.DisplayName = FText::FromString(TEXT("Player"));
	Member.bIsLeader = false;
	CurrentParty.Members.Add(Member);

	OnPartyUpdated.Broadcast(CurrentParty);
}

void UMGSessionSubsystem::LeaveParty()
{
	CurrentParty = FMGPartyInfo();
	OnPartyUpdated.Broadcast(CurrentParty);
}

void UMGSessionSubsystem::InviteToParty(const FString& PlayerID)
{
	// Would send party invite
}

bool UMGSessionSubsystem::IsPartyLeader() const
{
	for (const FMGPartyMember& Member : CurrentParty.Members)
	{
		if (Member.PlayerID == LocalPlayerID)
		{
			return Member.bIsLeader;
		}
	}
	return false;
}

void UMGSessionSubsystem::InvitePlayer(const FString& PlayerID)
{
	// Would send session invite
}

void UMGSessionSubsystem::InviteFriends()
{
	// Would open platform friend invite UI
}

void UMGSessionSubsystem::InviteCrew()
{
	// Would send invite to all crew members
}

void UMGSessionSubsystem::AcceptInvite(const FString& InviteID)
{
	// Would join the session/party from the invite
}

void UMGSessionSubsystem::DeclineInvite(const FString& InviteID)
{
	// Would decline and remove invite
}

void UMGSessionSubsystem::RefreshSessionList()
{
	AvailableSessions.Empty();

	// Simulate available sessions
	for (int32 i = 0; i < FMath::RandRange(3, 10); i++)
	{
		FMGSessionInfo Session;
		Session.SessionID = FGuid::NewGuid().ToString();
		Session.HostDisplayName = FText::FromString(FString::Printf(TEXT("Host_%d"), i));
		Session.State = EMGSessionState::InLobby;
		Session.Privacy = EMGLobbyPrivacy::Public;
		Session.CurrentPlayers = FMath::RandRange(1, 7);
		Session.MaxPlayers = 8;
		Session.Ping = FMath::RandRange(20, 150);
		Session.bIsFull = Session.CurrentPlayers >= Session.MaxPlayers;
		Session.AverageSkillRating = FMath::RandRange(800, 1500);

		AvailableSessions.Add(Session);
	}
}

void UMGSessionSubsystem::FilterSessions(FName GameModeFilter, bool bHideFullSessions, int32 MaxPing)
{
	TArray<FMGSessionInfo> Filtered;

	for (const FMGSessionInfo& Session : AvailableSessions)
	{
		if (bHideFullSessions && Session.bIsFull)
			continue;

		if (MaxPing > 0 && Session.Ping > MaxPing)
			continue;

		if (!GameModeFilter.IsNone() && Session.GameModeID != GameModeFilter)
			continue;

		Filtered.Add(Session);
	}

	AvailableSessions = Filtered;
}

void UMGSessionSubsystem::UpdateMatchmaking(float DeltaTime)
{
	MatchmakingTime += DeltaTime;
	PlayersInQueue = FMath::Max(1, PlayersInQueue + FMath::RandRange(-5, 10));

	// Simulate finding a match after some time
	if (MatchmakingTime >= FMath::RandRange(5.0f, 15.0f))
	{
		SimulateMatchFound();
	}

	// Timeout check
	if (MatchmakingTime >= CurrentMatchmakingSettings.SearchTimeout)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(MatchmakingTimerHandle);
		}
		SetMatchmakingState(EMGMatchmakingState::Failed);
	}
}

void UMGSessionSubsystem::SetSessionState(EMGSessionState NewState)
{
	if (CurrentSession.State != NewState)
	{
		CurrentSession.State = NewState;
		OnSessionStateChanged.Broadcast(NewState);
	}
}

void UMGSessionSubsystem::SetMatchmakingState(EMGMatchmakingState NewState)
{
	if (MatchmakingState != NewState)
	{
		MatchmakingState = NewState;
		OnMatchmakingStateChanged.Broadcast(NewState);
	}
}

void UMGSessionSubsystem::SimulateMatchFound()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchmakingTimerHandle);
	}

	SetMatchmakingState(EMGMatchmakingState::Found);

	// Create session info for found match
	FMGSessionInfo FoundSession;
	FoundSession.SessionID = FGuid::NewGuid().ToString();
	FoundSession.State = EMGSessionState::InLobby;
	FoundSession.CurrentPlayers = FMath::RandRange(4, 7);
	FoundSession.MaxPlayers = 8;
	FoundSession.Ping = FMath::RandRange(20, CurrentMatchmakingSettings.MaxPingMS);
	FoundSession.bIsRanked = CurrentMatchmakingSettings.bRankedOnly;

	OnSessionFound.Broadcast(FoundSession);

	// Auto-join the found session
	SetMatchmakingState(EMGMatchmakingState::Joining);
	CurrentSession = FoundSession;

	// Simulate other players in lobby
	LobbyPlayers.Empty();

	FMGLobbyPlayer LocalPlayer;
	LocalPlayer.PlayerID = LocalPlayerID;
	LocalPlayer.DisplayName = FText::FromString(TEXT("You"));
	LocalPlayer.bIsHost = false;
	LocalPlayer.bIsReady = false;
	LobbyPlayers.Add(LocalPlayer);

	for (int32 i = 0; i < FoundSession.CurrentPlayers - 1; i++)
	{
		FMGLobbyPlayer OtherPlayer;
		OtherPlayer.PlayerID = FGuid::NewGuid().ToString();
		OtherPlayer.DisplayName = FText::FromString(FString::Printf(TEXT("Racer_%d"), i + 1));
		OtherPlayer.SkillRating = FMath::RandRange(800, 1500);
		OtherPlayer.bIsHost = (i == 0);
		OtherPlayer.bIsReady = FMath::RandBool();
		OtherPlayer.Ping = FMath::RandRange(20, 100);
		LobbyPlayers.Add(OtherPlayer);
	}

	SetSessionState(EMGSessionState::InLobby);
	SetMatchmakingState(EMGMatchmakingState::Idle);
}

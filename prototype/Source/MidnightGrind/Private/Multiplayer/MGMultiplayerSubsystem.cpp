// Copyright Midnight Grind. All Rights Reserved.

#include "Multiplayer/MGMultiplayerSubsystem.h"
#include "Online/MGOnlineProfileSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UMGMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize local player with defaults
	LocalPlayer.PlayerID = FGuid::NewGuid().ToString();
	LocalPlayer.DisplayName = TEXT("Player");
	LocalPlayer.Level = 1;
}

void UMGMultiplayerSubsystem::Deinitialize()
{
	// Leave any active session
	if (bInSession)
	{
		LeaveSession();
	}

	// Cancel matchmaking
	if (bIsMatchmaking)
	{
		CancelMatchmaking();
	}

	// Disconnect
	if (ConnectionState != EMGConnectionState::Disconnected)
	{
		Disconnect();
	}

	Super::Deinitialize();
}

// ==========================================
// CONNECTION
// ==========================================

void UMGMultiplayerSubsystem::Connect()
{
	if (ConnectionState != EMGConnectionState::Disconnected)
	{
		return;
	}

	SetConnectionState(EMGConnectionState::Connecting);

	// Would establish connection to game services
	// For now, simulate immediate connection

	// Get player info from online profile
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGOnlineProfileSubsystem* OnlineProfile = GI->GetSubsystem<UMGOnlineProfileSubsystem>())
		{
			// LocalPlayer.DisplayName would come from online profile
			// LocalPlayer.Level, Reputation, etc.
		}
	}

	SetConnectionState(EMGConnectionState::Connected);
}

void UMGMultiplayerSubsystem::Disconnect()
{
	if (bInSession)
	{
		LeaveSession();
	}

	if (bIsMatchmaking)
	{
		CancelMatchmaking();
	}

	SetConnectionState(EMGConnectionState::Disconnected);
}

// ==========================================
// MATCHMAKING
// ==========================================

void UMGMultiplayerSubsystem::StartQuickMatch(FName PreferredTrack)
{
	if (!IsConnected() || bIsMatchmaking || bInSession)
	{
		return;
	}

	bIsMatchmaking = true;

	// Would start matchmaking with game services
	// For now, create a session after a simulated search

	OnMatchmakingProgress.Broadcast(1, 2); // Finding players

	// Simulate finding a match
	FMGLobbySettings Settings;
	Settings.TrackID = PreferredTrack.IsNone() ? FName("Track_City01") : PreferredTrack;
	Settings.MatchType = EMGMatchType::QuickMatch;
	Settings.LapCount = 3;
	Settings.MaxPlayers = 8;

	// Would call CreateSession or JoinSession based on matchmaking result
}

void UMGMultiplayerSubsystem::StartRankedMatch()
{
	if (!IsConnected() || bIsMatchmaking || bInSession)
	{
		return;
	}

	bIsMatchmaking = true;

	// Ranked matchmaking with skill-based matching
	OnMatchmakingProgress.Broadcast(1, 2);
}

void UMGMultiplayerSubsystem::CancelMatchmaking()
{
	if (!bIsMatchmaking)
	{
		return;
	}

	bIsMatchmaking = false;

	// Would notify matchmaking service of cancellation
}

// ==========================================
// SESSION
// ==========================================

void UMGMultiplayerSubsystem::CreateSession(const FMGLobbySettings& Settings)
{
	if (!IsConnected() || bInSession)
	{
		return;
	}

	// Cancel any active matchmaking
	if (bIsMatchmaking)
	{
		CancelMatchmaking();
	}

	// Initialize session
	CurrentSession.SessionID = FGuid::NewGuid().ToString();
	CurrentSession.HostPlayerID = LocalPlayer.PlayerID;
	CurrentSession.TrackID = Settings.TrackID;
	CurrentSession.MatchType = Settings.MatchType;
	CurrentSession.LapCount = Settings.LapCount;
	CurrentSession.MaxPlayers = Settings.MaxPlayers;
	CurrentSession.CurrentPlayers = 1;
	CurrentSession.bIsJoinable = !Settings.bIsPrivate;

	LobbySettings = Settings;
	LobbyPlayers.Empty();

	// Add local player as host
	LocalPlayer.bIsHost = true;
	LocalPlayer.bIsReady = false;
	LobbyPlayers.Add(LocalPlayer);

	bInSession = true;

	SetConnectionState(EMGConnectionState::InLobby);
	OnSessionJoined.Broadcast(CurrentSession);
	OnPlayerJoined.Broadcast(LocalPlayer);
}

void UMGMultiplayerSubsystem::JoinSession(const FString& SessionID)
{
	if (!IsConnected() || bInSession)
	{
		return;
	}

	if (bIsMatchmaking)
	{
		CancelMatchmaking();
	}

	// Would request to join session from server
	// Server validates and returns session info

	// For now, simulate joining
	CurrentSession.SessionID = SessionID;
	CurrentSession.CurrentPlayers = 2;

	LocalPlayer.bIsHost = false;
	LocalPlayer.bIsReady = false;
	LobbyPlayers.Add(LocalPlayer);

	bInSession = true;

	SetConnectionState(EMGConnectionState::InLobby);
	OnSessionJoined.Broadcast(CurrentSession);
	OnPlayerJoined.Broadcast(LocalPlayer);
}

void UMGMultiplayerSubsystem::JoinByInviteCode(const FString& InviteCode)
{
	FString SessionID = DecodeInviteCode(InviteCode);
	if (!SessionID.IsEmpty())
	{
		JoinSession(SessionID);
	}
}

void UMGMultiplayerSubsystem::LeaveSession()
{
	if (!bInSession)
	{
		return;
	}

	// Would notify server of leaving

	bInSession = false;
	CurrentSession = FMGSessionInfo();
	LobbyPlayers.Empty();
	LobbySettings = FMGLobbySettings();
	LocalPlayer.bIsHost = false;
	LocalPlayer.bIsReady = false;

	SetConnectionState(EMGConnectionState::Connected);
	OnSessionLeft.Broadcast();
}

FString UMGMultiplayerSubsystem::GetInviteCode() const
{
	if (!bInSession)
	{
		return FString();
	}

	return GenerateInviteCode(CurrentSession.SessionID);
}

void UMGMultiplayerSubsystem::SearchSessions(FName TrackFilter)
{
	if (!IsConnected())
	{
		return;
	}

	SessionSearchResults.Empty();

	// Would query server for available sessions
	// For now, return empty results
}

// ==========================================
// LOBBY
// ==========================================

void UMGMultiplayerSubsystem::SetReady(bool bReady)
{
	if (!bInSession)
	{
		return;
	}

	LocalPlayer.bIsReady = bReady;

	// Update in player list
	for (FMGNetPlayer& Player : LobbyPlayers)
	{
		if (Player.PlayerID == LocalPlayer.PlayerID)
		{
			Player.bIsReady = bReady;
			break;
		}
	}

	// Would notify server of ready state
	OnPlayerReady.Broadcast(LocalPlayer.PlayerID);
}

void UMGMultiplayerSubsystem::SetSelectedVehicle(FName VehicleID)
{
	if (!bInSession)
	{
		return;
	}

	LocalPlayer.VehicleID = VehicleID;

	// Update in player list
	for (FMGNetPlayer& Player : LobbyPlayers)
	{
		if (Player.PlayerID == LocalPlayer.PlayerID)
		{
			Player.VehicleID = VehicleID;
			break;
		}
	}

	// Would notify server of vehicle selection
}

void UMGMultiplayerSubsystem::UpdateLobbySettings(const FMGLobbySettings& NewSettings)
{
	if (!bInSession || !IsHost())
	{
		return;
	}

	LobbySettings = NewSettings;
	CurrentSession.TrackID = NewSettings.TrackID;
	CurrentSession.LapCount = NewSettings.LapCount;
	CurrentSession.MaxPlayers = NewSettings.MaxPlayers;
	CurrentSession.bIsJoinable = !NewSettings.bIsPrivate;

	// Would notify server and other players
	OnLobbySettingsChanged.Broadcast(LobbySettings);
}

void UMGMultiplayerSubsystem::KickPlayer(const FString& PlayerID)
{
	if (!bInSession || !IsHost())
	{
		return;
	}

	if (PlayerID == LocalPlayer.PlayerID)
	{
		return; // Can't kick self
	}

	// Would notify server to kick player
	// Server would then notify kicked player and others

	LobbyPlayers.RemoveAll([&PlayerID](const FMGNetPlayer& Player)
	{
		return Player.PlayerID == PlayerID;
	});

	OnPlayerLeft.Broadcast(PlayerID);
}

void UMGMultiplayerSubsystem::StartRace()
{
	if (!bInSession || !IsHost())
	{
		return;
	}

	if (!CanStartRace())
	{
		return;
	}

	// Broadcast starting
	OnRaceStarting.Broadcast();

	// Start countdown
	CountdownTime = LobbySettings.CountdownTime;

	// Would tell server to start race countdown
	// Server syncs countdown across all clients

	// Transition to race state
	SetConnectionState(EMGConnectionState::InRace);

	// After countdown, broadcast race started
	OnRaceStarted.Broadcast();
}

bool UMGMultiplayerSubsystem::CanStartRace() const
{
	if (!bInSession || LobbyPlayers.Num() < 1)
	{
		return false;
	}

	// Check all players ready (or just host for single player testing)
	if (LobbyPlayers.Num() == 1)
	{
		return true;
	}

	for (const FMGNetPlayer& Player : LobbyPlayers)
	{
		if (!Player.bIsReady)
		{
			return false;
		}
	}

	return true;
}

// ==========================================
// RACE
// ==========================================

void UMGMultiplayerSubsystem::ReportRaceFinish(float FinalTime, float BestLapTime)
{
	if (ConnectionState != EMGConnectionState::InRace)
	{
		return;
	}

	// Would send finish to server
	// Server validates and determines final positions

	FMGRaceResult Result;
	Result.PlayerID = LocalPlayer.PlayerID;
	Result.PlayerName = LocalPlayer.DisplayName;
	Result.TotalTime = FinalTime;
	Result.BestLapTime = BestLapTime;
	Result.VehicleID = LocalPlayer.VehicleID;

	// Server would calculate position, rewards, etc.
}

void UMGMultiplayerSubsystem::ReportLapTime(int32 LapNumber, float LapTime)
{
	if (ConnectionState != EMGConnectionState::InRace)
	{
		return;
	}

	// Would send lap time to server for tracking/validation
}

// ==========================================
// INTERNAL
// ==========================================

void UMGMultiplayerSubsystem::SetConnectionState(EMGConnectionState NewState)
{
	if (ConnectionState != NewState)
	{
		ConnectionState = NewState;
		OnConnectionStateChanged.Broadcast(NewState);
	}
}

void UMGMultiplayerSubsystem::OnMatchmakingUpdate(int32 PlayersFound, int32 PlayersNeeded)
{
	OnMatchmakingProgress.Broadcast(PlayersFound, PlayersNeeded);
}

void UMGMultiplayerSubsystem::OnSessionFound(const FMGSessionInfo& Session)
{
	// Called when matchmaking finds a session
	bIsMatchmaking = false;

	if (Session.HostPlayerID == LocalPlayer.PlayerID)
	{
		// We're hosting
		FMGLobbySettings Settings;
		Settings.TrackID = Session.TrackID;
		Settings.LapCount = Session.LapCount;
		Settings.MaxPlayers = Session.MaxPlayers;
		Settings.MatchType = Session.MatchType;
		CreateSession(Settings);
	}
	else
	{
		// Join existing session
		JoinSession(Session.SessionID);
	}
}

void UMGMultiplayerSubsystem::SyncServerTime()
{
	// Would ping server to calculate time offset
	// Used for race timer synchronization
}

FString UMGMultiplayerSubsystem::GenerateInviteCode(const FString& SessionID) const
{
	// Simple base62 encoding of session ID
	// In practice, this would be a shorter, more user-friendly code

	// For now, just use first 8 characters of session ID
	if (SessionID.Len() >= 8)
	{
		return SessionID.Left(8).ToUpper();
	}

	return SessionID.ToUpper();
}

FString UMGMultiplayerSubsystem::DecodeInviteCode(const FString& Code) const
{
	// Would decode back to session ID
	// For now, search for session starting with this code

	for (const FMGSessionInfo& Session : SessionSearchResults)
	{
		if (Session.SessionID.StartsWith(Code, ESearchCase::IgnoreCase))
		{
			return Session.SessionID;
		}
	}

	return FString();
}

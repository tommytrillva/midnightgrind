// Copyright Midnight Grind. All Rights Reserved.

#include "Online/MGSessionManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UMGSessionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Generate local player ID (would come from platform services)
	LocalPlayerID = FGuid::NewGuid().ToString();

	// Set up tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&UMGSessionManager::OnTick,
			0.1f, // 10 Hz
			true
		);
	}
}

void UMGSessionManager::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	Super::Deinitialize();
}

// ==========================================
// SESSION CREATION
// ==========================================

bool UMGSessionManager::CreateSession(EMGSessionType Type, FName TrackID, int32 LapCount, FName VehicleClass, int32 MaxPlayers)
{
	if (CurrentState != EMGSessionState::Idle)
	{
		OnSessionError.Broadcast(NSLOCTEXT("MG", "AlreadyInSession", "Already in a session"));
		return false;
	}

	SetSessionState(EMGSessionState::Creating);

	// Create session info
	CurrentSession = FMGSessionInfo();
	CurrentSession.SessionID = GenerateSessionID();
	CurrentSession.Type = Type;
	CurrentSession.HostID = LocalPlayerID;
	CurrentSession.TrackID = TrackID;
	CurrentSession.RaceType = FName(TEXT("Circuit")); // Default
	CurrentSession.LapCount = LapCount;
	CurrentSession.VehicleClass = VehicleClass;
	CurrentSession.MaxPlayers = MaxPlayers;
	CurrentSession.bRanked = (Type == EMGSessionType::Ranked);
	CurrentSession.Region = TEXT("AUTO");

	// Add local player as host
	FMGSessionPlayer LocalPlayer = CreateLocalPlayerData();
	LocalPlayer.bIsHost = true;
	CurrentSession.Players.Add(LocalPlayer);

	// In real implementation, would create online session here
	// For now, simulate success

	SetSessionState(EMGSessionState::InLobby);
	OnSessionCreated.Broadcast(CurrentSession);

	return true;
}

bool UMGSessionManager::CreatePrivateLobby(int32 MaxPlayers)
{
	return CreateSession(EMGSessionType::Private, NAME_None, 3, NAME_None, MaxPlayers);
}

// ==========================================
// MATCHMAKING
// ==========================================

bool UMGSessionManager::StartMatchmaking(const FMGMatchmakingPrefs& Preferences)
{
	if (CurrentState != EMGSessionState::Idle)
	{
		OnSessionError.Broadcast(NSLOCTEXT("MG", "AlreadyInSession", "Already in a session"));
		return false;
	}

	CurrentMatchmakingPrefs = Preferences;
	MatchmakingTime = 0.0f;

	SetSessionState(EMGSessionState::Searching);

	// In real implementation, would start async matchmaking

	return true;
}

void UMGSessionManager::CancelMatchmaking()
{
	if (CurrentState == EMGSessionState::Searching)
	{
		SetSessionState(EMGSessionState::Idle);
	}
}

void UMGSessionManager::SearchSessions(const FMGMatchmakingPrefs& Filters)
{
	// In real implementation, would query backend for sessions
	// For now, return empty results

	TArray<FMGSessionSearchResult> Results;
	OnSearchResults.Broadcast(Results);
}

bool UMGSessionManager::JoinSession(const FString& SessionID)
{
	if (CurrentState != EMGSessionState::Idle && CurrentState != EMGSessionState::Searching)
	{
		OnSessionError.Broadcast(NSLOCTEXT("MG", "CannotJoin", "Cannot join session in current state"));
		return false;
	}

	SetSessionState(EMGSessionState::Joining);

	// In real implementation, would join online session
	// For now, simulate joining

	// Create mock session
	CurrentSession = FMGSessionInfo();
	CurrentSession.SessionID = SessionID;
	CurrentSession.Type = EMGSessionType::QuickMatch;
	CurrentSession.MaxPlayers = 8;
	CurrentSession.TrackID = FName(TEXT("Downtown"));
	CurrentSession.LapCount = 3;

	// Add local player
	FMGSessionPlayer LocalPlayer = CreateLocalPlayerData();
	CurrentSession.Players.Add(LocalPlayer);

	SetSessionState(EMGSessionState::InLobby);
	OnSessionJoined.Broadcast(CurrentSession);

	return true;
}

bool UMGSessionManager::JoinSessionFromResult(const FMGSessionSearchResult& Result)
{
	return JoinSession(Result.SessionInfo.SessionID);
}

// ==========================================
// LOBBY MANAGEMENT
// ==========================================

void UMGSessionManager::SetReady(bool bReady)
{
	if (CurrentState != EMGSessionState::InLobby)
	{
		return;
	}

	for (FMGSessionPlayer& Player : CurrentSession.Players)
	{
		if (Player.PlayerID == LocalPlayerID)
		{
			Player.bReady = bReady;
			OnPlayerReady.Broadcast(Player);
			OnSessionUpdated.Broadcast(CurrentSession);
			break;
		}
	}
}

void UMGSessionManager::SetSelectedVehicle(FName VehicleID, int32 PerformanceIndex)
{
	if (CurrentState != EMGSessionState::InLobby)
	{
		return;
	}

	for (FMGSessionPlayer& Player : CurrentSession.Players)
	{
		if (Player.PlayerID == LocalPlayerID)
		{
			Player.SelectedVehicle = VehicleID;
			Player.VehiclePI = PerformanceIndex;
			OnSessionUpdated.Broadcast(CurrentSession);
			break;
		}
	}
}

void UMGSessionManager::LeaveSession()
{
	if (!IsInSession())
	{
		return;
	}

	SetSessionState(EMGSessionState::Disconnecting);

	// Notify other players
	FMGSessionPlayer LocalPlayer = GetLocalPlayer();
	OnPlayerLeft.Broadcast(LocalPlayer);

	// Clear session data
	CurrentSession = FMGSessionInfo();
	bCountdownActive = false;
	CountdownRemaining = 0.0f;

	SetSessionState(EMGSessionState::Idle);
}

bool UMGSessionManager::KickPlayer(const FString& PlayerID)
{
	if (!IsHost() || PlayerID == LocalPlayerID)
	{
		return false;
	}

	for (int32 i = 0; i < CurrentSession.Players.Num(); ++i)
	{
		if (CurrentSession.Players[i].PlayerID == PlayerID)
		{
			FMGSessionPlayer KickedPlayer = CurrentSession.Players[i];
			CurrentSession.Players.RemoveAt(i);
			OnPlayerLeft.Broadcast(KickedPlayer);
			OnSessionUpdated.Broadcast(CurrentSession);
			return true;
		}
	}

	return false;
}

bool UMGSessionManager::ChangeTrack(FName NewTrackID)
{
	if (!IsHost())
	{
		return false;
	}

	CurrentSession.TrackID = NewTrackID;

	// Reset ready states when track changes
	for (FMGSessionPlayer& Player : CurrentSession.Players)
	{
		if (!Player.bIsHost)
		{
			Player.bReady = false;
		}
	}

	OnSessionUpdated.Broadcast(CurrentSession);
	return true;
}

bool UMGSessionManager::ChangeLapCount(int32 NewLapCount)
{
	if (!IsHost())
	{
		return false;
	}

	CurrentSession.LapCount = FMath::Clamp(NewLapCount, 1, 99);
	OnSessionUpdated.Broadcast(CurrentSession);
	return true;
}

bool UMGSessionManager::StartCountdown()
{
	if (!IsHost() || !CanStartRace())
	{
		return false;
	}

	bCountdownActive = true;
	CountdownRemaining = DefaultCountdownDuration;
	CurrentSession.bCountdownActive = true;
	CurrentSession.CountdownRemaining = CountdownRemaining;

	OnCountdownStarted.Broadcast(DefaultCountdownDuration);
	OnSessionUpdated.Broadcast(CurrentSession);

	return true;
}

void UMGSessionManager::CancelCountdown()
{
	if (!IsHost() || !bCountdownActive)
	{
		return;
	}

	bCountdownActive = false;
	CountdownRemaining = 0.0f;
	CurrentSession.bCountdownActive = false;
	CurrentSession.CountdownRemaining = 0.0f;

	OnCountdownCancelled.Broadcast();
	OnSessionUpdated.Broadcast(CurrentSession);
}

void UMGSessionManager::InvitePlayer(const FString& PlayerID)
{
	// In real implementation, would send platform invite
}

// ==========================================
// STATE QUERIES
// ==========================================

bool UMGSessionManager::IsInSession() const
{
	return CurrentState == EMGSessionState::InLobby ||
		CurrentState == EMGSessionState::Loading ||
		CurrentState == EMGSessionState::InRace ||
		CurrentState == EMGSessionState::PostRace;
}

bool UMGSessionManager::IsHost() const
{
	return CurrentSession.HostID == LocalPlayerID;
}

bool UMGSessionManager::IsLocalPlayerReady() const
{
	for (const FMGSessionPlayer& Player : CurrentSession.Players)
	{
		if (Player.PlayerID == LocalPlayerID)
		{
			return Player.bReady;
		}
	}
	return false;
}

bool UMGSessionManager::AreAllPlayersReady() const
{
	if (CurrentSession.Players.Num() < 2)
	{
		return false; // Need at least 2 players
	}

	for (const FMGSessionPlayer& Player : CurrentSession.Players)
	{
		if (!Player.bReady && !Player.bIsHost) // Host doesn't need to be ready
		{
			return false;
		}
	}

	return true;
}

FMGSessionPlayer UMGSessionManager::GetLocalPlayer() const
{
	for (const FMGSessionPlayer& Player : CurrentSession.Players)
	{
		if (Player.PlayerID == LocalPlayerID)
		{
			return Player;
		}
	}
	return FMGSessionPlayer();
}

bool UMGSessionManager::CanStartRace() const
{
	if (!IsHost())
	{
		return false;
	}

	if (CurrentSession.Players.Num() < 1)
	{
		return false;
	}

	if (CurrentSession.TrackID.IsNone())
	{
		return false;
	}

	// Check vehicle class restrictions
	if (!CurrentSession.VehicleClass.IsNone())
	{
		for (const FMGSessionPlayer& Player : CurrentSession.Players)
		{
			// Would verify vehicle class here
		}
	}

	return AreAllPlayersReady() || CurrentSession.Players.Num() == 1;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGSessionManager::SetSessionState(EMGSessionState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnSessionStateChanged.Broadcast(NewState);
	}
}

void UMGSessionManager::TickMatchmaking(float DeltaTime)
{
	MatchmakingTime += DeltaTime;

	// Expand search after timeout
	if (MatchmakingTime > MatchmakingExpandTime)
	{
		// Would expand matchmaking criteria here
	}

	// Simulate finding a match after some time
	if (MatchmakingTime > 3.0f) // 3 second mock delay
	{
		// Create mock session
		CurrentSession = FMGSessionInfo();
		CurrentSession.SessionID = GenerateSessionID();
		CurrentSession.Type = CurrentMatchmakingPrefs.SessionType;
		CurrentSession.TrackID = CurrentMatchmakingPrefs.PreferredTrack.IsNone() ?
			FName(TEXT("Downtown")) : CurrentMatchmakingPrefs.PreferredTrack;
		CurrentSession.LapCount = CurrentMatchmakingPrefs.PreferredLaps > 0 ?
			CurrentMatchmakingPrefs.PreferredLaps : 3;
		CurrentSession.VehicleClass = CurrentMatchmakingPrefs.VehicleClass;
		CurrentSession.MaxPlayers = 8;
		CurrentSession.bRanked = (CurrentMatchmakingPrefs.SessionType == EMGSessionType::Ranked);

		// Add local player
		FMGSessionPlayer LocalPlayer = CreateLocalPlayerData();
		CurrentSession.Players.Add(LocalPlayer);

		// Add some mock opponents
		for (int32 i = 0; i < FMath::RandRange(2, 5); ++i)
		{
			FMGSessionPlayer MockPlayer;
			MockPlayer.PlayerID = FGuid::NewGuid().ToString();
			MockPlayer.DisplayName = FText::FromString(FString::Printf(TEXT("Racer_%d"), i + 1));
			MockPlayer.Level = FMath::RandRange(1, 50);
			MockPlayer.Rating = FMath::RandRange(800, 1500);
			MockPlayer.SelectedVehicle = FName(TEXT("GenericRacer"));
			MockPlayer.VehiclePI = FMath::RandRange(400, 600);
			MockPlayer.bReady = true;
			MockPlayer.Ping = FMath::RandRange(20, 80);
			CurrentSession.Players.Add(MockPlayer);
		}

		SetSessionState(EMGSessionState::InLobby);
		OnSessionJoined.Broadcast(CurrentSession);
	}
}

void UMGSessionManager::TickCountdown(float DeltaTime)
{
	CountdownRemaining -= DeltaTime;
	CurrentSession.CountdownRemaining = CountdownRemaining;

	if (CountdownRemaining <= 0.0f)
	{
		bCountdownActive = false;
		CurrentSession.bCountdownActive = false;

		SetSessionState(EMGSessionState::Loading);
		OnRaceStarting.Broadcast();

		// Would trigger level load here
	}
}

FString UMGSessionManager::GenerateSessionID() const
{
	return FString::Printf(TEXT("MG-%s"), *FGuid::NewGuid().ToString().Left(8).ToUpper());
}

FMGSessionPlayer UMGSessionManager::CreateLocalPlayerData() const
{
	FMGSessionPlayer Player;
	Player.PlayerID = LocalPlayerID;
	Player.DisplayName = FText::FromString(TEXT("Player")); // Would get from save
	Player.Level = 1; // Would get from progression
	Player.Rating = 1000; // Default rating
	Player.bReady = false;
	Player.bIsLocal = true;
	Player.Ping = 0;

	return Player;
}

int32 UMGSessionManager::FindBestSession(const TArray<FMGSessionSearchResult>& Results) const
{
	if (Results.Num() == 0)
	{
		return -1;
	}

	int32 BestIndex = 0;
	int32 BestScore = 0;

	for (int32 i = 0; i < Results.Num(); ++i)
	{
		const FMGSessionSearchResult& Result = Results[i];

		int32 Score = Result.MatchQuality;

		// Bonus for having a rival
		if (Result.bHasRival)
		{
			Score += 20;
		}

		// Penalty for high ping
		Score -= Result.Ping / 10;

		// Prefer sessions that aren't almost full
		float FillRatio = static_cast<float>(Result.SessionInfo.GetPlayerCount()) / Result.SessionInfo.MaxPlayers;
		if (FillRatio < 0.75f)
		{
			Score += 10;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestIndex = i;
		}
	}

	return BestIndex;
}

void UMGSessionManager::HandleHostMigration()
{
	// Select new host (lowest ping player)
	if (CurrentSession.Players.Num() == 0)
	{
		LeaveSession();
		return;
	}

	FMGSessionPlayer* NewHost = nullptr;
	int32 LowestPing = INT32_MAX;

	for (FMGSessionPlayer& Player : CurrentSession.Players)
	{
		if (Player.Ping < LowestPing)
		{
			LowestPing = Player.Ping;
			NewHost = &Player;
		}
	}

	if (NewHost)
	{
		NewHost->bIsHost = true;
		CurrentSession.HostID = NewHost->PlayerID;
		OnSessionUpdated.Broadcast(CurrentSession);
	}
}

void UMGSessionManager::OnTick()
{
	const float DeltaTime = 0.1f;

	switch (CurrentState)
	{
	case EMGSessionState::Searching:
		TickMatchmaking(DeltaTime);
		break;

	case EMGSessionState::InLobby:
		if (bCountdownActive)
		{
			TickCountdown(DeltaTime);
		}
		break;

	default:
		break;
	}
}

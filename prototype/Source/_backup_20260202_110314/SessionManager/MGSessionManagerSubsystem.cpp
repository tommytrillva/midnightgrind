// Copyright Midnight Grind. All Rights Reserved.

#include "SessionManager/MGSessionManagerSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGSessionManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Start session tick for housekeeping
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SessionTickHandle,
			this,
			&UMGSessionManagerSubsystem::OnSessionTick,
			1.0f,
			true
		);
	}
}

void UMGSessionManagerSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SessionTickHandle);
		World->GetTimerManager().ClearTimer(JoinTimeoutHandle);
		World->GetTimerManager().ClearTimer(SearchTimeoutHandle);
	}

	if (IsInSession())
	{
		LeaveSession();
	}

	Super::Deinitialize();
}

// Session Creation
bool UMGSessionManagerSubsystem::CreateSession(const FMGSessionSettings& Settings)
{
	if (IsInSession())
	{
		return false;
	}

	SetSessionState(EMGSessionState::Creating);
	SimulateSessionCreation(Settings);
	return true;
}

bool UMGSessionManagerSubsystem::UpdateSession(const FMGSessionSettings& Settings)
{
	if (!IsSessionHost())
	{
		return false;
	}

	CurrentSession.SessionName = Settings.SessionName;
	CurrentSession.MaxPlayers = Settings.MaxPlayers;
	CurrentSession.bPrivate = Settings.bPrivate;
	CurrentSession.bJoinInProgress = Settings.bAllowJoinInProgress;
	CurrentSession.MapName = Settings.MapName;
	CurrentSession.GameMode = Settings.GameMode;
	CurrentSession.CustomData.Empty();
	for (const auto& Pair : Settings.CustomSettings)
	{
		CurrentSession.CustomData.Add(Pair.Key, Pair.Value);
	}

	OnSessionUpdated.Broadcast(CurrentSession);
	return true;
}

void UMGSessionManagerSubsystem::DestroySession()
{
	if (!IsSessionHost())
	{
		return;
	}

	// Notify all players
	for (const FMGSessionPlayer& Player : SessionPlayers)
	{
		if (!Player.bIsLocal)
		{
			OnPlayerLeft.Broadcast(Player.PlayerID, EMGDisconnectReason::HostClosed);
		}
	}

	SessionPlayers.Empty();
	CurrentSession = FMGSessionInfo();

	SetSessionState(EMGSessionState::None);
	OnSessionEnded.Broadcast(EMGDisconnectReason::HostClosed);
}

bool UMGSessionManagerSubsystem::IsSessionHost() const
{
	if (!IsInSession())
	{
		return false;
	}

	FMGSessionPlayer LocalPlayer = GetLocalPlayer();
	return LocalPlayer.bIsHost;
}

// Session Joining
bool UMGSessionManagerSubsystem::JoinSession(const FString& SessionID, const FString& Password)
{
	if (IsInSession())
	{
		return false;
	}

	// Find session in search results
	FMGSessionInfo* FoundSession = SearchResults.FindByPredicate([&SessionID](const FMGSessionInfo& S)
	{
		return S.SessionID == SessionID;
	});

	if (FoundSession)
	{
		return JoinSessionByInfo(*FoundSession, Password);
	}

	// Try to join by ID directly
	SetSessionState(EMGSessionState::Joining);

	CurrentConnectionAttempt.SessionID = SessionID;
	CurrentConnectionAttempt.AttemptNumber = 1;
	CurrentConnectionAttempt.MaxAttempts = 3;
	CurrentConnectionAttempt.AttemptStartTime = 0.0f;
	CurrentConnectionAttempt.TimeoutSeconds = 30.0f;

	OnConnectionAttempt.Broadcast(1, 3);

	// Start timeout timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			JoinTimeoutHandle,
			this,
			&UMGSessionManagerSubsystem::OnJoinTimeout,
			CurrentConnectionAttempt.TimeoutSeconds,
			false
		);
	}

	// Simulate successful join after brief delay
	// In real implementation, would connect to server
	if (UWorld* World = GetWorld())
	{
		FTimerHandle TempHandle;
		TWeakObjectPtr<UMGSessionManagerSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			TempHandle,
			[WeakThis, SessionID]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}
				if (WeakThis->CurrentState == EMGSessionState::Joining)
				{
					// Create simulated session
					WeakThis->CurrentSession.SessionID = SessionID;
					WeakThis->CurrentSession.SessionName = TEXT("Joined Session");
					WeakThis->CurrentSession.Type = EMGSessionType::OnlinePublic;
					WeakThis->CurrentSession.State = EMGSessionState::InLobby;
					WeakThis->CurrentSession.HostPlayerID = TEXT("RemoteHost");
					WeakThis->CurrentSession.MaxPlayers = 8;
					WeakThis->CurrentSession.CreatedTime = FDateTime::Now();

					WeakThis->AddLocalPlayerToSession();
					WeakThis->SetSessionState(EMGSessionState::InLobby);
					WeakThis->OnSessionJoined.Broadcast(WeakThis->CurrentSession);
				}
			},
			2.0f,
			false
		);
	}

	return true;
}

bool UMGSessionManagerSubsystem::JoinSessionByInfo(const FMGSessionInfo& SessionInfo, const FString& Password)
{
	if (IsInSession())
	{
		return false;
	}

	// Check if session is full
	if (SessionInfo.CurrentPlayers >= SessionInfo.MaxPlayers)
	{
		OnSessionJoinFailed.Broadcast(SessionInfo.SessionID, TEXT("Session is full"));
		return false;
	}

	// Check password
	if (SessionInfo.bPrivate && Password.IsEmpty())
	{
		OnSessionJoinFailed.Broadcast(SessionInfo.SessionID, TEXT("Password required"));
		return false;
	}

	return JoinSession(SessionInfo.SessionID, Password);
}

void UMGSessionManagerSubsystem::CancelJoin()
{
	if (CurrentState != EMGSessionState::Joining)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(JoinTimeoutHandle);
	}

	CurrentConnectionAttempt = FMGConnectionAttempt();
	SetSessionState(EMGSessionState::None);
}

void UMGSessionManagerSubsystem::LeaveSession()
{
	if (!IsInSession())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(JoinTimeoutHandle);
	}

	FMGSessionPlayer LocalPlayer = GetLocalPlayer();

	if (LocalPlayer.bIsHost)
	{
		DestroySession();
	}
	else
	{
		OnPlayerLeft.Broadcast(LocalPlayer.PlayerID, EMGDisconnectReason::PlayerQuit);
		SessionPlayers.Empty();
		CurrentSession = FMGSessionInfo();
		SetSessionState(EMGSessionState::None);
		OnSessionEnded.Broadcast(EMGDisconnectReason::PlayerQuit);
	}
}

// Session State
bool UMGSessionManagerSubsystem::IsInSession() const
{
	return CurrentState == EMGSessionState::InLobby ||
		CurrentState == EMGSessionState::InProgress ||
		CurrentState == EMGSessionState::Starting ||
		CurrentState == EMGSessionState::Ending ||
		CurrentState == EMGSessionState::PostGame;
}

FMGSessionPlayer UMGSessionManagerSubsystem::GetLocalPlayer() const
{
	const FMGSessionPlayer* Local = SessionPlayers.FindByPredicate([](const FMGSessionPlayer& P)
	{
		return P.bIsLocal;
	});

	return Local ? *Local : FMGSessionPlayer();
}

FMGSessionPlayer UMGSessionManagerSubsystem::GetPlayer(const FString& PlayerID) const
{
	const FMGSessionPlayer* Found = SessionPlayers.FindByPredicate([&PlayerID](const FMGSessionPlayer& P)
	{
		return P.PlayerID == PlayerID;
	});

	return Found ? *Found : FMGSessionPlayer();
}

// Session Management
bool UMGSessionManagerSubsystem::KickPlayer(const FString& PlayerID, const FString& Reason)
{
	if (!IsSessionHost())
	{
		return false;
	}

	int32 Index = SessionPlayers.IndexOfByPredicate([&PlayerID](const FMGSessionPlayer& P)
	{
		return P.PlayerID == PlayerID && !P.bIsLocal;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	SessionPlayers.RemoveAt(Index);
	CurrentSession.CurrentPlayers = SessionPlayers.Num();

	OnPlayerLeft.Broadcast(PlayerID, EMGDisconnectReason::Kicked);
	OnSessionUpdated.Broadcast(CurrentSession);

	return true;
}

bool UMGSessionManagerSubsystem::BanPlayer(const FString& PlayerID, const FString& Reason)
{
	if (!KickPlayer(PlayerID, Reason))
	{
		return false;
	}

	BannedPlayerIDs.AddUnique(PlayerID);
	return true;
}

bool UMGSessionManagerSubsystem::SetSessionLocked(bool bLocked)
{
	if (!IsSessionHost())
	{
		return false;
	}

	CurrentSession.bJoinInProgress = !bLocked;
	OnSessionUpdated.Broadcast(CurrentSession);
	return true;
}

bool UMGSessionManagerSubsystem::TransferHost(const FString& NewHostPlayerID)
{
	if (!IsSessionHost())
	{
		return false;
	}

	FMGSessionPlayer* NewHost = SessionPlayers.FindByPredicate([&NewHostPlayerID](const FMGSessionPlayer& P)
	{
		return P.PlayerID == NewHostPlayerID;
	});

	if (!NewHost)
	{
		return false;
	}

	// Update host flags
	for (FMGSessionPlayer& P : SessionPlayers)
	{
		P.bIsHost = (P.PlayerID == NewHostPlayerID);
	}

	CurrentSession.HostPlayerID = NewHostPlayerID;
	CurrentSession.HostDisplayName = NewHost->DisplayName;

	OnNewHostSelected.Broadcast(NewHostPlayerID);
	OnSessionUpdated.Broadcast(CurrentSession);

	return true;
}

bool UMGSessionManagerSubsystem::StartSession()
{
	if (!IsSessionHost() || CurrentState != EMGSessionState::InLobby)
	{
		return false;
	}

	if (!AreAllPlayersReady())
	{
		return false;
	}

	SetSessionState(EMGSessionState::Starting);

	// Would trigger map travel here
	SetSessionState(EMGSessionState::InProgress);

	return true;
}

bool UMGSessionManagerSubsystem::EndSession()
{
	if (!IsSessionHost() || CurrentState != EMGSessionState::InProgress)
	{
		return false;
	}

	SetSessionState(EMGSessionState::Ending);
	SetSessionState(EMGSessionState::PostGame);

	return true;
}

// Player Data
void UMGSessionManagerSubsystem::SetLocalPlayerReady(bool bReady)
{
	FMGSessionPlayer* Local = SessionPlayers.FindByPredicate([](const FMGSessionPlayer& P)
	{
		return P.bIsLocal;
	});

	if (Local)
	{
		Local->bIsReady = bReady;
		OnPlayerDataChanged.Broadcast(Local->PlayerID, *Local);
	}
}

void UMGSessionManagerSubsystem::SetLocalPlayerTeam(int32 TeamIndex)
{
	FMGSessionPlayer* Local = SessionPlayers.FindByPredicate([](const FMGSessionPlayer& P)
	{
		return P.bIsLocal;
	});

	if (Local)
	{
		Local->TeamIndex = TeamIndex;
		OnPlayerDataChanged.Broadcast(Local->PlayerID, *Local);
	}
}

void UMGSessionManagerSubsystem::SetLocalPlayerSpectator(bool bSpectator)
{
	FMGSessionPlayer* Local = SessionPlayers.FindByPredicate([](const FMGSessionPlayer& P)
	{
		return P.bIsLocal;
	});

	if (Local)
	{
		Local->bIsSpectator = bSpectator;
		OnPlayerDataChanged.Broadcast(Local->PlayerID, *Local);
	}
}

void UMGSessionManagerSubsystem::SetLocalPlayerData(FName Key, const FString& Value)
{
	FMGSessionPlayer* Local = SessionPlayers.FindByPredicate([](const FMGSessionPlayer& P)
	{
		return P.bIsLocal;
	});

	if (Local)
	{
		Local->PlayerData.Add(Key, Value);
		OnPlayerDataChanged.Broadcast(Local->PlayerID, *Local);
	}
}

FString UMGSessionManagerSubsystem::GetLocalPlayerData(FName Key) const
{
	FMGSessionPlayer Local = GetLocalPlayer();
	const FString* Value = Local.PlayerData.Find(Key);
	return Value ? *Value : TEXT("");
}

bool UMGSessionManagerSubsystem::AreAllPlayersReady() const
{
	for (const FMGSessionPlayer& P : SessionPlayers)
	{
		if (!P.bIsReady && !P.bIsHost && !P.bIsSpectator)
		{
			return false;
		}
	}
	return SessionPlayers.Num() > 0;
}

// Session Search
void UMGSessionManagerSubsystem::SearchSessions(const FMGSessionSearchFilters& Filters)
{
	if (bSearching)
	{
		return;
	}

	bSearching = true;
	SearchResults.Empty();

	SimulateSessionSearch();
}

void UMGSessionManagerSubsystem::CancelSearch()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SearchTimeoutHandle);
	}
	bSearching = false;
}

void UMGSessionManagerSubsystem::RefreshSession(const FString& SessionID)
{
	// Would query specific session for updated info
}

// Invites
bool UMGSessionManagerSubsystem::SendInvite(const FString& RecipientPlayerID)
{
	if (!IsInSession())
	{
		OnInviteSent.Broadcast(RecipientPlayerID, false);
		return false;
	}

	// Would send through online subsystem
	OnInviteSent.Broadcast(RecipientPlayerID, true);
	return true;
}

bool UMGSessionManagerSubsystem::AcceptInvite(const FMGSessionInvite& Invite)
{
	// Remove from pending
	PendingInvites.RemoveAll([&Invite](const FMGSessionInvite& I)
	{
		return I.InviteID == Invite.InviteID;
	});

	return JoinSessionByInfo(Invite.SessionInfo, TEXT(""));
}

void UMGSessionManagerSubsystem::DeclineInvite(const FMGSessionInvite& Invite)
{
	PendingInvites.RemoveAll([&Invite](const FMGSessionInvite& I)
	{
		return I.InviteID == Invite.InviteID;
	});
}

void UMGSessionManagerSubsystem::ClearExpiredInvites()
{
	FDateTime Now = FDateTime::Now();
	PendingInvites.RemoveAll([&Now](const FMGSessionInvite& I)
	{
		return I.ExpiryTime < Now;
	});
}

// Host Migration
bool UMGSessionManagerSubsystem::IsHostMigrationInProgress() const
{
	return MigrationState != EMGMigrationState::None &&
		MigrationState != EMGMigrationState::MigrationComplete &&
		MigrationState != EMGMigrationState::MigrationFailed;
}

void UMGSessionManagerSubsystem::RequestHostMigration()
{
	if (!IsInSession() || IsSessionHost())
	{
		return;
	}

	SimulateHostMigration();
}

// Network Quality
int32 UMGSessionManagerSubsystem::GetAverageSessionPing() const
{
	if (SessionPlayers.Num() == 0)
	{
		return 0;
	}

	int32 TotalPing = 0;
	int32 RemoteCount = 0;

	for (const FMGSessionPlayer& P : SessionPlayers)
	{
		if (!P.bIsLocal)
		{
			TotalPing += P.Ping;
			RemoteCount++;
		}
	}

	return RemoteCount > 0 ? TotalPing / RemoteCount : 0;
}

void UMGSessionManagerSubsystem::UpdateNetworkStats()
{
	// Would update ping and packet loss metrics
	PacketLossPercent = FMath::RandRange(0.0f, 2.0f);

	// Update player pings
	for (FMGSessionPlayer& P : SessionPlayers)
	{
		if (!P.bIsLocal)
		{
			P.Ping = FMath::RandRange(20, 150);
		}
	}
}

// Internal
void UMGSessionManagerSubsystem::SetSessionState(EMGSessionState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		CurrentSession.State = NewState;
		OnSessionStateChanged.Broadcast(NewState);
	}
}

void UMGSessionManagerSubsystem::SetMigrationState(EMGMigrationState NewState)
{
	if (MigrationState != NewState)
	{
		MigrationState = NewState;
		OnHostMigrationStateChanged.Broadcast(NewState);
	}
}

void UMGSessionManagerSubsystem::OnSessionTick()
{
	// Clear expired invites
	ClearExpiredInvites();

	// Update network stats periodically
	if (IsInSession())
	{
		UpdateNetworkStats();
	}
}

void UMGSessionManagerSubsystem::OnJoinTimeout()
{
	if (CurrentState != EMGSessionState::Joining)
	{
		return;
	}

	CurrentConnectionAttempt.AttemptNumber++;

	if (CurrentConnectionAttempt.AttemptNumber <= CurrentConnectionAttempt.MaxAttempts)
	{
		RetryConnection();
	}
	else
	{
		OnSessionJoinFailed.Broadcast(CurrentConnectionAttempt.SessionID, TEXT("Connection timed out"));
		SetSessionState(EMGSessionState::None);
		CurrentConnectionAttempt = FMGConnectionAttempt();
	}
}

void UMGSessionManagerSubsystem::RetryConnection()
{
	OnConnectionAttempt.Broadcast(
		CurrentConnectionAttempt.AttemptNumber,
		CurrentConnectionAttempt.MaxAttempts
	);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			JoinTimeoutHandle,
			this,
			&UMGSessionManagerSubsystem::OnJoinTimeout,
			CurrentConnectionAttempt.TimeoutSeconds,
			false
		);
	}
}

void UMGSessionManagerSubsystem::SimulateSessionCreation(const FMGSessionSettings& Settings)
{
	CurrentSession.SessionID = FGuid::NewGuid().ToString();
	CurrentSession.SessionName = Settings.SessionName;
	CurrentSession.Type = Settings.Type;
	CurrentSession.State = EMGSessionState::InLobby;
	CurrentSession.HostPlayerID = TEXT("LocalPlayer");
	CurrentSession.HostDisplayName = TEXT("Player");
	CurrentSession.MaxPlayers = Settings.MaxPlayers;
	CurrentSession.bPrivate = Settings.bPrivate;
	CurrentSession.bJoinInProgress = Settings.bAllowJoinInProgress;
	CurrentSession.MapName = Settings.MapName;
	CurrentSession.GameMode = Settings.GameMode;
	CurrentSession.Region = Settings.PreferredRegion;
	CurrentSession.GameVersion = TEXT("1.0.0");
	CurrentSession.CreatedTime = FDateTime::Now();

	for (const auto& Pair : Settings.CustomSettings)
	{
		CurrentSession.CustomData.Add(Pair.Key, Pair.Value);
	}

	AddLocalPlayerToSession();

	SetSessionState(EMGSessionState::InLobby);
	OnSessionCreated.Broadcast(CurrentSession);
}

void UMGSessionManagerSubsystem::SimulateSessionSearch()
{
	// Generate some fake search results
	for (int32 i = 0; i < 10; i++)
	{
		FMGSessionInfo Session;
		Session.SessionID = FGuid::NewGuid().ToString();
		Session.SessionName = FString::Printf(TEXT("Race Room %d"), i + 1);
		Session.Type = EMGSessionType::OnlinePublic;
		Session.State = EMGSessionState::InLobby;
		Session.HostPlayerID = FString::Printf(TEXT("Host_%d"), i);
		Session.HostDisplayName = FString::Printf(TEXT("Racer%d"), FMath::RandRange(100, 999));
		Session.CurrentPlayers = FMath::RandRange(1, 7);
		Session.MaxPlayers = 8;
		Session.bPrivate = (i % 4 == 0);
		Session.bJoinInProgress = true;
		Session.MapName = TEXT("Downtown");
		Session.Ping = FMath::RandRange(20, 150);
		Session.Region = TEXT("NA-East");
		Session.GameVersion = TEXT("1.0.0");
		Session.CreatedTime = FDateTime::Now() - FTimespan::FromMinutes(FMath::RandRange(1, 30));
		SearchResults.Add(Session);
	}

	bSearching = false;
	OnSessionSearchComplete.Broadcast(SearchResults);
}

void UMGSessionManagerSubsystem::SimulateHostMigration()
{
	SetMigrationState(EMGMigrationState::HostMigrationStarted);

	// Find best candidate for new host
	FMGSessionPlayer* BestCandidate = nullptr;
	int32 LowestPing = INT_MAX;

	for (FMGSessionPlayer& P : SessionPlayers)
	{
		if (!P.bIsLocal && P.Ping < LowestPing)
		{
			LowestPing = P.Ping;
			BestCandidate = &P;
		}
	}

	SetMigrationState(EMGMigrationState::WaitingForNewHost);

	if (BestCandidate)
	{
		SetMigrationState(EMGMigrationState::NewHostSelected);
		OnNewHostSelected.Broadcast(BestCandidate->PlayerID);

		// Update host flags
		for (FMGSessionPlayer& P : SessionPlayers)
		{
			P.bIsHost = (P.PlayerID == BestCandidate->PlayerID);
		}

		CurrentSession.HostPlayerID = BestCandidate->PlayerID;
		CurrentSession.HostDisplayName = BestCandidate->DisplayName;

		SetMigrationState(EMGMigrationState::MigrationComplete);
		OnSessionUpdated.Broadcast(CurrentSession);
	}
	else
	{
		SetMigrationState(EMGMigrationState::MigrationFailed);
		OnSessionEnded.Broadcast(EMGDisconnectReason::HostClosed);
	}
}

FMGSessionPlayer UMGSessionManagerSubsystem::CreateLocalPlayer() const
{
	FMGSessionPlayer Player;
	Player.PlayerID = TEXT("LocalPlayer");
	Player.DisplayName = TEXT("Player");
	Player.bIsHost = false;
	Player.bIsLocal = true;
	Player.Ping = 0;
	Player.TeamIndex = 0;
	Player.bIsReady = false;
	Player.bIsSpectator = false;
	Player.Platform = TEXT("PC");
	Player.JoinTime = FDateTime::Now();
	return Player;
}

void UMGSessionManagerSubsystem::AddLocalPlayerToSession()
{
	FMGSessionPlayer LocalPlayer = CreateLocalPlayer();
	LocalPlayer.bIsHost = (CurrentSession.HostPlayerID == TEXT("LocalPlayer"));
	SessionPlayers.Add(LocalPlayer);
	CurrentSession.CurrentPlayers = SessionPlayers.Num();

	OnPlayerJoined.Broadcast(LocalPlayer);
}

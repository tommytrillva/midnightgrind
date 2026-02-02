// Copyright Midnight Grind. All Rights Reserved.

#include "Matchmaking/MGMatchmakingSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGMatchmakingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default skill rating
	LocalPlayerSkill.MMR = 1000;
	LocalPlayerSkill.Tier = EMGSkillTier::Bronze;
	LocalPlayerSkill.Division = 1;

	// Initialize region pings with defaults
	RegionPings.Add(EMGMatchmakingRegion::NorthAmerica, 50);
	RegionPings.Add(EMGMatchmakingRegion::SouthAmerica, 120);
	RegionPings.Add(EMGMatchmakingRegion::Europe, 80);
	RegionPings.Add(EMGMatchmakingRegion::Asia, 150);
	RegionPings.Add(EMGMatchmakingRegion::Oceania, 200);
	RegionPings.Add(EMGMatchmakingRegion::MiddleEast, 130);
	RegionPings.Add(EMGMatchmakingRegion::Africa, 180);
}

void UMGMatchmakingSubsystem::Deinitialize()
{
	CancelMatchmaking();
	LeaveLobby();
	Super::Deinitialize();
}

// Matchmaking
bool UMGMatchmakingSubsystem::StartMatchmaking(const FMGMatchmakingPreferences& Preferences)
{
	if (MatchmakingState != EMGMatchmakingState::Idle)
	{
		return false;
	}

	// Create matchmaking ticket
	CurrentTicket.TicketID = FGuid::NewGuid().ToString();
	CurrentTicket.PlayerID = TEXT("LocalPlayer"); // Would get from online subsystem
	CurrentTicket.Preferences = Preferences;
	CurrentTicket.SkillRating = LocalPlayerSkill;
	CurrentTicket.CreatedTime = FDateTime::Now();
	CurrentTicket.SearchTimeSeconds = 0.0f;
	CurrentTicket.SearchExpansionLevel = 0;
	CurrentTicket.CurrentPing = GetRegionPing(Preferences.PreferredRegion);

	SetMatchmakingState(EMGMatchmakingState::SearchingForMatch);

	// Start matchmaking tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MatchmakingTimerHandle,
			this,
			&UMGMatchmakingSubsystem::OnMatchmakingTick,
			1.0f,
			true
		);
	}

	return true;
}

void UMGMatchmakingSubsystem::CancelMatchmaking()
{
	if (MatchmakingState != EMGMatchmakingState::SearchingForMatch)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchmakingTimerHandle);
	}

	SetMatchmakingState(EMGMatchmakingState::Cancelled);
	SetMatchmakingState(EMGMatchmakingState::Idle);
}

float UMGMatchmakingSubsystem::GetMatchmakingTime() const
{
	if (MatchmakingState != EMGMatchmakingState::SearchingForMatch)
	{
		return 0.0f;
	}

	return CurrentTicket.SearchTimeSeconds;
}

// Lobby Management
bool UMGMatchmakingSubsystem::CreateLobby(const FMGLobbySettings& Settings, EMGMatchType MatchType)
{
	if (bInLobby)
	{
		return false;
	}

	CurrentLobby.LobbyID = FGuid::NewGuid().ToString();
	CurrentLobby.SessionID = FGuid::NewGuid().ToString();
	CurrentLobby.HostPlayerID = TEXT("LocalPlayer");
	CurrentLobby.MatchType = MatchType;
	CurrentLobby.Settings = Settings;
	CurrentLobby.Region = GetBestRegion();
	CurrentLobby.CreatedTime = FDateTime::Now();
	CurrentLobby.bMatchStarting = false;
	CurrentLobby.CountdownRemaining = 0.0f;

	// Add local player as host
	FMGLobbyPlayer LocalPlayer = CreateLocalPlayer();
	LocalPlayer.bIsHost = true;
	CurrentLobby.Players.Add(LocalPlayer);
	CurrentLobby.AverageSkillMMR = LocalPlayer.SkillRating.MMR;

	bInLobby = true;
	SetMatchmakingState(EMGMatchmakingState::InLobby);

	OnLobbyUpdated.Broadcast(CurrentLobby);
	return true;
}

bool UMGMatchmakingSubsystem::JoinLobby(const FString& LobbyID, const FString& Password)
{
	if (bInLobby)
	{
		return false;
	}

	// Find lobby in available lobbies
	FMGMatchLobby* FoundLobby = AvailableLobbies.FindByPredicate([&LobbyID](const FMGMatchLobby& L)
	{
		return L.LobbyID == LobbyID;
	});

	if (!FoundLobby)
	{
		OnMatchmakingFailed.Broadcast(TEXT("Lobby not found"));
		return false;
	}

	// Check password
	if (FoundLobby->Settings.bPrivate && FoundLobby->Settings.Password != Password)
	{
		OnMatchmakingFailed.Broadcast(TEXT("Incorrect password"));
		return false;
	}

	// Check if lobby is full
	if (FoundLobby->Players.Num() >= FoundLobby->Settings.MaxPlayers)
	{
		OnMatchmakingFailed.Broadcast(TEXT("Lobby is full"));
		return false;
	}

	SetMatchmakingState(EMGMatchmakingState::JoiningLobby);

	CurrentLobby = *FoundLobby;

	// Add local player
	FMGLobbyPlayer LocalPlayer = CreateLocalPlayer();
	CurrentLobby.Players.Add(LocalPlayer);

	// Recalculate average MMR
	int32 TotalMMR = 0;
	for (const FMGLobbyPlayer& P : CurrentLobby.Players)
	{
		TotalMMR += P.SkillRating.MMR;
	}
	CurrentLobby.AverageSkillMMR = TotalMMR / CurrentLobby.Players.Num();

	bInLobby = true;
	SetMatchmakingState(EMGMatchmakingState::InLobby);

	OnPlayerJoinedLobby.Broadcast(CurrentLobby.LobbyID, LocalPlayer);
	OnLobbyUpdated.Broadcast(CurrentLobby);

	return true;
}

bool UMGMatchmakingSubsystem::JoinLobbyByCode(const FString& JoinCode)
{
	// Find lobby by join code (simplified - would query backend)
	for (const FMGMatchLobby& Lobby : AvailableLobbies)
	{
		// Generate expected code and compare
		FString LobbyCode = Lobby.LobbyID.Left(6).ToUpper();
		if (LobbyCode == JoinCode.ToUpper())
		{
			return JoinLobby(Lobby.LobbyID, TEXT(""));
		}
	}

	OnMatchmakingFailed.Broadcast(TEXT("Invalid lobby code"));
	return false;
}

void UMGMatchmakingSubsystem::LeaveLobby()
{
	if (!bInLobby)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	FString LeftLobbyID = CurrentLobby.LobbyID;
	FString LocalPlayerID = TEXT("LocalPlayer");

	OnPlayerLeftLobby.Broadcast(LeftLobbyID, LocalPlayerID);

	CurrentLobby = FMGMatchLobby();
	bInLobby = false;

	SetMatchmakingState(EMGMatchmakingState::Idle);
}

FString UMGMatchmakingSubsystem::GenerateLobbyJoinCode() const
{
	if (!bInLobby)
	{
		return TEXT("");
	}

	return CurrentLobby.LobbyID.Left(6).ToUpper();
}

// Lobby Host Functions
bool UMGMatchmakingSubsystem::UpdateLobbySettings(const FMGLobbySettings& NewSettings)
{
	if (!bInLobby || !IsLocalPlayerHost())
	{
		return false;
	}

	CurrentLobby.Settings = NewSettings;
	OnLobbyUpdated.Broadcast(CurrentLobby);
	return true;
}

bool UMGMatchmakingSubsystem::KickPlayer(const FString& PlayerID)
{
	if (!bInLobby || !IsLocalPlayerHost())
	{
		return false;
	}

	int32 PlayerIndex = CurrentLobby.Players.IndexOfByPredicate([&PlayerID](const FMGLobbyPlayer& P)
	{
		return P.PlayerID == PlayerID && !P.bIsHost;
	});

	if (PlayerIndex == INDEX_NONE)
	{
		return false;
	}

	CurrentLobby.Players.RemoveAt(PlayerIndex);
	OnPlayerLeftLobby.Broadcast(CurrentLobby.LobbyID, PlayerID);
	OnLobbyUpdated.Broadcast(CurrentLobby);

	return true;
}

bool UMGMatchmakingSubsystem::BanPlayer(const FString& PlayerID)
{
	// Ban list would be stored persistently
	return KickPlayer(PlayerID);
}

bool UMGMatchmakingSubsystem::TransferHost(const FString& NewHostPlayerID)
{
	if (!bInLobby || !IsLocalPlayerHost())
	{
		return false;
	}

	FMGLobbyPlayer* NewHost = CurrentLobby.Players.FindByPredicate([&NewHostPlayerID](const FMGLobbyPlayer& P)
	{
		return P.PlayerID == NewHostPlayerID;
	});

	if (!NewHost)
	{
		return false;
	}

	// Remove host from current host
	for (FMGLobbyPlayer& P : CurrentLobby.Players)
	{
		P.bIsHost = (P.PlayerID == NewHostPlayerID);
	}

	CurrentLobby.HostPlayerID = NewHostPlayerID;
	OnLobbyUpdated.Broadcast(CurrentLobby);

	return true;
}

bool UMGMatchmakingSubsystem::StartLobbyCountdown()
{
	if (!bInLobby || !IsLocalPlayerHost())
	{
		return false;
	}

	if (!AreAllPlayersReady())
	{
		return false;
	}

	CurrentLobby.bMatchStarting = true;
	CurrentLobby.CountdownRemaining = CurrentLobby.Settings.CountdownTime;

	OnLobbyCountdownStarted.Broadcast(CurrentLobby.Settings.CountdownTime);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&UMGMatchmakingSubsystem::OnCountdownTick,
			1.0f,
			true
		);
	}

	return true;
}

void UMGMatchmakingSubsystem::CancelLobbyCountdown()
{
	if (!bInLobby || !IsLocalPlayerHost())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	CurrentLobby.bMatchStarting = false;
	CurrentLobby.CountdownRemaining = 0.0f;

	OnLobbyUpdated.Broadcast(CurrentLobby);
}

// Player Functions
void UMGMatchmakingSubsystem::SetPlayerReady(bool bReady)
{
	if (!bInLobby)
	{
		return;
	}

	FMGLobbyPlayer* LocalPlayer = CurrentLobby.Players.FindByPredicate([](const FMGLobbyPlayer& P)
	{
		return P.PlayerID == TEXT("LocalPlayer");
	});

	if (LocalPlayer)
	{
		EMGLobbyPlayerState NewState = bReady ? EMGLobbyPlayerState::Ready : EMGLobbyPlayerState::NotReady;
		LocalPlayer->State = NewState;
		OnPlayerStateChanged.Broadcast(LocalPlayer->PlayerID, NewState);
		OnLobbyUpdated.Broadcast(CurrentLobby);
	}
}

void UMGMatchmakingSubsystem::SetSelectedVehicle(FName VehicleID)
{
	if (!bInLobby)
	{
		return;
	}

	FMGLobbyPlayer* LocalPlayer = CurrentLobby.Players.FindByPredicate([](const FMGLobbyPlayer& P)
	{
		return P.PlayerID == TEXT("LocalPlayer");
	});

	if (LocalPlayer)
	{
		LocalPlayer->SelectedVehicle = VehicleID;
		OnLobbyUpdated.Broadcast(CurrentLobby);
	}
}

void UMGMatchmakingSubsystem::SetTeam(int32 TeamIndex)
{
	if (!bInLobby)
	{
		return;
	}

	FMGLobbyPlayer* LocalPlayer = CurrentLobby.Players.FindByPredicate([](const FMGLobbyPlayer& P)
	{
		return P.PlayerID == TEXT("LocalPlayer");
	});

	if (LocalPlayer)
	{
		LocalPlayer->TeamIndex = TeamIndex;
		OnLobbyUpdated.Broadcast(CurrentLobby);
	}
}

bool UMGMatchmakingSubsystem::IsLocalPlayerHost() const
{
	if (!bInLobby)
	{
		return false;
	}

	const FMGLobbyPlayer* LocalPlayer = CurrentLobby.Players.FindByPredicate([](const FMGLobbyPlayer& P)
	{
		return P.PlayerID == TEXT("LocalPlayer");
	});

	return LocalPlayer && LocalPlayer->bIsHost;
}

bool UMGMatchmakingSubsystem::IsLocalPlayerReady() const
{
	if (!bInLobby)
	{
		return false;
	}

	const FMGLobbyPlayer* LocalPlayer = CurrentLobby.Players.FindByPredicate([](const FMGLobbyPlayer& P)
	{
		return P.PlayerID == TEXT("LocalPlayer");
	});

	return LocalPlayer && LocalPlayer->State == EMGLobbyPlayerState::Ready;
}

bool UMGMatchmakingSubsystem::AreAllPlayersReady() const
{
	if (!bInLobby || CurrentLobby.Players.Num() < CurrentLobby.Settings.MaxPlayers / 2)
	{
		return false;
	}

	for (const FMGLobbyPlayer& P : CurrentLobby.Players)
	{
		if (P.State != EMGLobbyPlayerState::Ready && !P.bIsHost)
		{
			return false;
		}
	}

	return true;
}

int32 UMGMatchmakingSubsystem::GetReadyPlayerCount() const
{
	int32 Count = 0;
	for (const FMGLobbyPlayer& P : CurrentLobby.Players)
	{
		if (P.State == EMGLobbyPlayerState::Ready || P.bIsHost)
		{
			Count++;
		}
	}
	return Count;
}

// Skill Rating
void UMGMatchmakingSubsystem::UpdateSkillRatingFromMatch(const FMGMatchResult& MatchResult)
{
	FMGPlayerSkillRating OldRating = LocalPlayerSkill;

	// Find local player position
	int32 Position = MatchResult.FinalStandings.IndexOfByKey(TEXT("LocalPlayer"));
	if (Position == INDEX_NONE)
	{
		return;
	}

	Position++; // 1-indexed

	// Calculate MMR change
	int32 MMRChange = CalculateMMRChange(
		LocalPlayerSkill.MMR,
		MatchResult.MMRChanges.Contains(TEXT("LocalPlayer")) ? MatchResult.MMRChanges[TEXT("LocalPlayer")] : 0,
		Position,
		MatchResult.FinalStandings.Num()
	);

	LocalPlayerSkill.MMR = FMath::Max(0, LocalPlayerSkill.MMR + MMRChange);
	LocalPlayerSkill.TotalRankedRaces++;
	LocalPlayerSkill.LastRankedRace = FDateTime::Now();

	// Update win/loss tracking
	if (Position == 1)
	{
		LocalPlayerSkill.SeasonWins++;
		LocalPlayerSkill.WinStreak++;
		LocalPlayerSkill.LossStreak = 0;
	}
	else if (Position > MatchResult.FinalStandings.Num() / 2)
	{
		LocalPlayerSkill.SeasonLosses++;
		LocalPlayerSkill.LossStreak++;
		LocalPlayerSkill.WinStreak = 0;
	}

	// Update win rate
	int32 TotalGames = LocalPlayerSkill.SeasonWins + LocalPlayerSkill.SeasonLosses;
	if (TotalGames > 0)
	{
		LocalPlayerSkill.WinRate = static_cast<float>(LocalPlayerSkill.SeasonWins) / TotalGames;
	}

	// Update tier
	LocalPlayerSkill.Tier = CalculateTierFromMMR(LocalPlayerSkill.MMR);

	// Add to match history
	MatchHistory.Insert(MatchResult, 0);
	if (MatchHistory.Num() > 50)
	{
		MatchHistory.SetNum(50);
	}

	OnSkillRatingUpdated.Broadcast(OldRating, LocalPlayerSkill);
}

EMGSkillTier UMGMatchmakingSubsystem::CalculateTierFromMMR(int32 MMR) const
{
	if (MMR >= 2500) return EMGSkillTier::Legend;
	if (MMR >= 2000) return EMGSkillTier::Champion;
	if (MMR >= 1600) return EMGSkillTier::Diamond;
	if (MMR >= 1300) return EMGSkillTier::Platinum;
	if (MMR >= 1000) return EMGSkillTier::Gold;
	if (MMR >= 700) return EMGSkillTier::Silver;
	return EMGSkillTier::Bronze;
}

int32 UMGMatchmakingSubsystem::CalculateMMRChange(int32 CurrentMMR, int32 OpponentAvgMMR, int32 Position, int32 TotalPlayers) const
{
	// ELO-style calculation
	float ExpectedScore = 1.0f / (1.0f + FMath::Pow(10.0f, (OpponentAvgMMR - CurrentMMR) / 400.0f));
	float ActualScore = 1.0f - (static_cast<float>(Position - 1) / (TotalPlayers - 1));

	int32 KFactor = 32;
	if (CurrentMMR >= 2000) KFactor = 16;
	else if (CurrentMMR >= 1500) KFactor = 24;

	return FMath::RoundToInt(KFactor * (ActualScore - ExpectedScore));
}

// Server Browser
void UMGMatchmakingSubsystem::RefreshServerList()
{
	AvailableServers.Empty();

	// Simulated server list - would query backend
	for (int32 i = 0; i < 10; i++)
	{
		FMGServerInfo Server;
		Server.ServerID = FString::Printf(TEXT("Server_%d"), i);
		Server.ServerAddress = FString::Printf(TEXT("192.168.1.%d"), 100 + i);
		Server.Port = 7777;
		Server.Region = static_cast<EMGMatchmakingRegion>((i % 6) + 1);
		Server.CurrentPlayers = FMath::RandRange(1, 8);
		Server.MaxPlayers = 8;
		Server.Ping = FMath::RandRange(20, 200);
		Server.bDedicated = true;
		Server.bAvailable = true;
		AvailableServers.Add(Server);
	}
}

void UMGMatchmakingSubsystem::RefreshLobbyList(EMGMatchType TypeFilter)
{
	AvailableLobbies.Empty();

	// Simulated lobby list - would query backend
	for (int32 i = 0; i < 5; i++)
	{
		FMGMatchLobby Lobby;
		Lobby.LobbyID = FGuid::NewGuid().ToString();
		Lobby.SessionID = FGuid::NewGuid().ToString();
		Lobby.HostPlayerID = FString::Printf(TEXT("Host_%d"), i);
		Lobby.MatchType = TypeFilter;
		Lobby.Settings.MaxPlayers = 8;
		Lobby.Settings.bPrivate = (i % 3 == 0);
		Lobby.Region = GetBestRegion();
		Lobby.CreatedTime = FDateTime::Now();
		Lobby.AverageSkillMMR = 1000 + FMath::RandRange(-200, 200);

		// Add some fake players
		int32 PlayerCount = FMath::RandRange(1, 6);
		for (int32 j = 0; j < PlayerCount; j++)
		{
			FMGLobbyPlayer Player;
			Player.PlayerID = FString::Printf(TEXT("Player_%d_%d"), i, j);
			Player.DisplayName = FString::Printf(TEXT("Racer%d"), FMath::RandRange(1, 999));
			Player.State = (j == 0) ? EMGLobbyPlayerState::Ready : EMGLobbyPlayerState::NotReady;
			Player.bIsHost = (j == 0);
			Player.Ping = FMath::RandRange(20, 150);
			Lobby.Players.Add(Player);
		}

		AvailableLobbies.Add(Lobby);
	}
}

// Region
EMGMatchmakingRegion UMGMatchmakingSubsystem::GetBestRegion() const
{
	EMGMatchmakingRegion BestRegion = EMGMatchmakingRegion::NorthAmerica;
	int32 BestPing = INT_MAX;

	for (const auto& Pair : RegionPings)
	{
		if (Pair.Value < BestPing)
		{
			BestPing = Pair.Value;
			BestRegion = Pair.Key;
		}
	}

	return BestRegion;
}

void UMGMatchmakingSubsystem::PingAllRegions()
{
	// Would actually ping servers - simulated here
	RegionPings[EMGMatchmakingRegion::NorthAmerica] = FMath::RandRange(30, 80);
	RegionPings[EMGMatchmakingRegion::SouthAmerica] = FMath::RandRange(80, 150);
	RegionPings[EMGMatchmakingRegion::Europe] = FMath::RandRange(50, 120);
	RegionPings[EMGMatchmakingRegion::Asia] = FMath::RandRange(100, 200);
	RegionPings[EMGMatchmakingRegion::Oceania] = FMath::RandRange(150, 250);
	RegionPings[EMGMatchmakingRegion::MiddleEast] = FMath::RandRange(100, 180);
	RegionPings[EMGMatchmakingRegion::Africa] = FMath::RandRange(120, 220);
}

int32 UMGMatchmakingSubsystem::GetRegionPing(EMGMatchmakingRegion Region) const
{
	if (Region == EMGMatchmakingRegion::Automatic)
	{
		return GetRegionPing(GetBestRegion());
	}

	const int32* Ping = RegionPings.Find(Region);
	return Ping ? *Ping : 999;
}

// Match History
TArray<FMGMatchResult> UMGMatchmakingSubsystem::GetRecentMatches(int32 Count) const
{
	TArray<FMGMatchResult> Result;
	int32 NumToReturn = FMath::Min(Count, MatchHistory.Num());
	for (int32 i = 0; i < NumToReturn; i++)
	{
		Result.Add(MatchHistory[i]);
	}
	return Result;
}

// Internal
void UMGMatchmakingSubsystem::SetMatchmakingState(EMGMatchmakingState NewState)
{
	if (MatchmakingState != NewState)
	{
		MatchmakingState = NewState;
		OnMatchmakingStateChanged.Broadcast(NewState);
	}
}

void UMGMatchmakingSubsystem::OnMatchmakingTick()
{
	CurrentTicket.SearchTimeSeconds += 1.0f;
	OnMatchmakingProgressUpdated.Broadcast(CurrentTicket.SearchTimeSeconds);

	// Check for timeout
	if (CurrentTicket.SearchTimeSeconds >= MaxSearchTime)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(MatchmakingTimerHandle);
		}
		SetMatchmakingState(EMGMatchmakingState::Failed);
		OnMatchmakingFailed.Broadcast(TEXT("Matchmaking timed out"));
		SetMatchmakingState(EMGMatchmakingState::Idle);
		return;
	}

	// Expand search criteria over time
	if (FMath::Fmod(CurrentTicket.SearchTimeSeconds, SearchExpansionInterval) < 1.0f &&
		CurrentTicket.SearchTimeSeconds > 1.0f)
	{
		ExpandSearchCriteria();
	}

	// Simulate match found after some time
	if (CurrentTicket.SearchTimeSeconds >= 3.0f + FMath::RandRange(0.0f, 5.0f))
	{
		SimulateMatchFound();
	}
}

void UMGMatchmakingSubsystem::ExpandSearchCriteria()
{
	CurrentTicket.SearchExpansionLevel++;

	// Expand MMR range, ping threshold, etc.
}

void UMGMatchmakingSubsystem::OnCountdownTick()
{
	CurrentLobby.CountdownRemaining -= 1.0f;

	if (CurrentLobby.CountdownRemaining <= 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CountdownTimerHandle);
		}

		SetMatchmakingState(EMGMatchmakingState::StartingMatch);
		OnMatchStarting.Broadcast();

		// Update all player states
		for (FMGLobbyPlayer& P : CurrentLobby.Players)
		{
			P.State = EMGLobbyPlayerState::Loading;
		}

		OnLobbyUpdated.Broadcast(CurrentLobby);

		// Would trigger actual match start here
		SetMatchmakingState(EMGMatchmakingState::InMatch);
	}
	else
	{
		OnLobbyUpdated.Broadcast(CurrentLobby);
	}
}

void UMGMatchmakingSubsystem::SimulateMatchFound()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchmakingTimerHandle);
	}

	SetMatchmakingState(EMGMatchmakingState::MatchFound);

	// Create a lobby with other players
	CurrentLobby.LobbyID = FGuid::NewGuid().ToString();
	CurrentLobby.SessionID = FGuid::NewGuid().ToString();
	CurrentLobby.HostPlayerID = TEXT("Host_MM");
	CurrentLobby.MatchType = CurrentTicket.Preferences.MatchType;
	CurrentLobby.Settings.RaceMode = CurrentTicket.Preferences.PreferredRaceMode;
	CurrentLobby.Settings.TrackID = CurrentTicket.Preferences.PreferredTrack;
	CurrentLobby.Settings.MaxPlayers = CurrentTicket.Preferences.MaxPlayers;
	CurrentLobby.Region = CurrentTicket.Preferences.PreferredRegion;
	CurrentLobby.CreatedTime = FDateTime::Now();

	// Add local player
	FMGLobbyPlayer LocalPlayer = CreateLocalPlayer();
	CurrentLobby.Players.Add(LocalPlayer);

	// Add matched players
	int32 OtherPlayers = FMath::RandRange(CurrentTicket.Preferences.MinPlayers - 1, CurrentTicket.Preferences.MaxPlayers - 1);
	for (int32 i = 0; i < OtherPlayers; i++)
	{
		FMGLobbyPlayer MatchedPlayer;
		MatchedPlayer.PlayerID = FString::Printf(TEXT("Matched_%d"), i);
		MatchedPlayer.DisplayName = FString::Printf(TEXT("Racer%d"), FMath::RandRange(100, 9999));
		MatchedPlayer.State = EMGLobbyPlayerState::Ready;
		MatchedPlayer.SkillRating.MMR = LocalPlayerSkill.MMR + FMath::RandRange(-200, 200);
		MatchedPlayer.SkillRating.Tier = CalculateTierFromMMR(MatchedPlayer.SkillRating.MMR);
		MatchedPlayer.Ping = FMath::RandRange(20, CurrentTicket.Preferences.MaxPingThreshold);
		MatchedPlayer.bIsHost = (i == 0);
		MatchedPlayer.JoinedTime = FDateTime::Now();
		CurrentLobby.Players.Add(MatchedPlayer);
	}

	// Calculate average MMR
	int32 TotalMMR = 0;
	for (const FMGLobbyPlayer& P : CurrentLobby.Players)
	{
		TotalMMR += P.SkillRating.MMR;
	}
	CurrentLobby.AverageSkillMMR = TotalMMR / CurrentLobby.Players.Num();

	bInLobby = true;
	OnMatchFound.Broadcast(CurrentLobby);
	SetMatchmakingState(EMGMatchmakingState::InLobby);
}

FMGLobbyPlayer UMGMatchmakingSubsystem::CreateLocalPlayer() const
{
	FMGLobbyPlayer Player;
	Player.PlayerID = TEXT("LocalPlayer");
	Player.DisplayName = TEXT("Player"); // Would get from profile
	Player.State = EMGLobbyPlayerState::NotReady;
	Player.SkillRating = LocalPlayerSkill;
	Player.SelectedVehicle = NAME_None;
	Player.TeamIndex = 0;
	Player.Ping = GetRegionPing(EMGMatchmakingRegion::Automatic);
	Player.bIsHost = false;
	Player.bIsCrossPlayPlayer = false;
	Player.Platform = TEXT("PC");
	Player.JoinedTime = FDateTime::Now();
	return Player;
}

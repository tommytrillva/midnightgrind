// Copyright Midnight Grind. All Rights Reserved.

#include "Multiplayer/MGLobbyWidget.h"
#include "Multiplayer/MGMultiplayerSubsystem.h"
#include "Kismet/GameplayStatics.h"

// ==========================================
// UMGPlayerSlotWidget
// ==========================================

void UMGPlayerSlotWidget::UpdatePlayerData_Implementation(const FMGNetPlayer& PlayerData)
{
	CurrentPlayerData = PlayerData;
	bIsEmpty = false;
}

void UMGPlayerSlotWidget::SetEmpty_Implementation()
{
	CurrentPlayerData = FMGNetPlayer();
	bIsEmpty = true;
}

void UMGPlayerSlotWidget::SetLocalPlayer_Implementation(bool bIsLocal)
{
	bIsLocalPlayer = bIsLocal;
}

// ==========================================
// UMGLobbyWidget
// ==========================================

void UMGLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get multiplayer subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		MultiplayerSubsystem = GI->GetSubsystem<UMGMultiplayerSubsystem>();
	}

	BindEvents();
	InitializeLobby();
}

void UMGLobbyWidget::NativeDestruct()
{
	UnbindEvents();
	Super::NativeDestruct();
}

void UMGLobbyWidget::InitializeLobby()
{
	if (!MultiplayerSubsystem)
	{
		return;
	}

	UpdatePlayerList();
	UpdateLobbySettings();
}

void UMGLobbyWidget::UpdatePlayerList()
{
	if (!MultiplayerSubsystem)
	{
		return;
	}

	TArray<FMGNetPlayer> Players = MultiplayerSubsystem->GetLobbyPlayers();
	FMGNetPlayer LocalPlayer = MultiplayerSubsystem->GetLocalPlayer();

	// Update existing slots or create new ones
	for (int32 i = 0; i < MaxDisplaySlots; ++i)
	{
		UMGPlayerSlotWidget* Slot = nullptr;

		if (i < PlayerSlotWidgets.Num())
		{
			Slot = PlayerSlotWidgets[i];
		}
		else if (PlayerSlotClass)
		{
			Slot = CreateWidget<UMGPlayerSlotWidget>(this, PlayerSlotClass);
			PlayerSlotWidgets.Add(Slot);
		}

		if (!Slot)
		{
			continue;
		}

		if (i < Players.Num())
		{
			const FMGNetPlayer& Player = Players[i];
			Slot->UpdatePlayerData(Player);
			Slot->SetLocalPlayer(Player.PlayerID == LocalPlayer.PlayerID);
		}
		else
		{
			Slot->SetEmpty();
			Slot->SetLocalPlayer(false);
		}
	}
}

void UMGLobbyWidget::UpdateLobbySettings()
{
	if (!MultiplayerSubsystem)
	{
		return;
	}

	CurrentSettings = MultiplayerSubsystem->GetLobbySettings();
}

void UMGLobbyWidget::SetReady(bool bReady)
{
	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->SetReady(bReady);
	}
}

void UMGLobbyWidget::SelectVehicle(FName VehicleID)
{
	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->SetSelectedVehicle(VehicleID);
	}
}

void UMGLobbyWidget::LeaveLobby()
{
	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->LeaveSession();
	}
}

void UMGLobbyWidget::StartRace()
{
	if (MultiplayerSubsystem && MultiplayerSubsystem->IsHost())
	{
		MultiplayerSubsystem->StartRace();
	}
}

// ==========================================
// HOST CONTROLS
// ==========================================

void UMGLobbyWidget::ChangeTrack(FName TrackID)
{
	if (!MultiplayerSubsystem || !MultiplayerSubsystem->IsHost())
	{
		return;
	}

	FMGLobbySettings NewSettings = CurrentSettings;
	NewSettings.TrackID = TrackID;
	MultiplayerSubsystem->UpdateLobbySettings(NewSettings);
}

void UMGLobbyWidget::ChangeLapCount(int32 Laps)
{
	if (!MultiplayerSubsystem || !MultiplayerSubsystem->IsHost())
	{
		return;
	}

	FMGLobbySettings NewSettings = CurrentSettings;
	NewSettings.LapCount = FMath::Clamp(Laps, 1, 20);
	MultiplayerSubsystem->UpdateLobbySettings(NewSettings);
}

void UMGLobbyWidget::KickPlayer(const FString& PlayerID)
{
	if (MultiplayerSubsystem && MultiplayerSubsystem->IsHost())
	{
		MultiplayerSubsystem->KickPlayer(PlayerID);
	}
}

// ==========================================
// QUERY
// ==========================================

FName UMGLobbyWidget::GetSelectedTrack() const
{
	return CurrentSettings.TrackID;
}

bool UMGLobbyWidget::CanStartRace() const
{
	return MultiplayerSubsystem ? MultiplayerSubsystem->CanStartRace() : false;
}

bool UMGLobbyWidget::IsHost() const
{
	return MultiplayerSubsystem ? MultiplayerSubsystem->IsHost() : false;
}

FString UMGLobbyWidget::GetInviteCode() const
{
	return MultiplayerSubsystem ? MultiplayerSubsystem->GetInviteCode() : FString();
}

// ==========================================
// INTERNAL
// ==========================================

void UMGLobbyWidget::BindEvents()
{
	if (!MultiplayerSubsystem)
	{
		return;
	}

	MultiplayerSubsystem->OnPlayerJoined.AddDynamic(this, &UMGLobbyWidget::HandlePlayerJoined);
	MultiplayerSubsystem->OnPlayerLeft.AddDynamic(this, &UMGLobbyWidget::HandlePlayerLeft);
	MultiplayerSubsystem->OnPlayerReady.AddDynamic(this, &UMGLobbyWidget::HandlePlayerReady);
	MultiplayerSubsystem->OnLobbySettingsChanged.AddDynamic(this, &UMGLobbyWidget::HandleSettingsChanged);
	MultiplayerSubsystem->OnRaceStarting.AddDynamic(this, &UMGLobbyWidget::HandleRaceStarting);
}

void UMGLobbyWidget::UnbindEvents()
{
	if (!MultiplayerSubsystem)
	{
		return;
	}

	MultiplayerSubsystem->OnPlayerJoined.RemoveDynamic(this, &UMGLobbyWidget::HandlePlayerJoined);
	MultiplayerSubsystem->OnPlayerLeft.RemoveDynamic(this, &UMGLobbyWidget::HandlePlayerLeft);
	MultiplayerSubsystem->OnPlayerReady.RemoveDynamic(this, &UMGLobbyWidget::HandlePlayerReady);
	MultiplayerSubsystem->OnLobbySettingsChanged.RemoveDynamic(this, &UMGLobbyWidget::HandleSettingsChanged);
	MultiplayerSubsystem->OnRaceStarting.RemoveDynamic(this, &UMGLobbyWidget::HandleRaceStarting);
}

void UMGLobbyWidget::HandlePlayerJoined(const FMGNetPlayer& Player)
{
	UpdatePlayerList();
	OnPlayerJoinedLobby(Player);
}

void UMGLobbyWidget::HandlePlayerLeft(const FString& PlayerID)
{
	UpdatePlayerList();
	OnPlayerLeftLobby(PlayerID);
}

void UMGLobbyWidget::HandlePlayerReady(const FString& PlayerID)
{
	UpdatePlayerList();

	// Find if player is ready
	if (MultiplayerSubsystem)
	{
		TArray<FMGNetPlayer> Players = MultiplayerSubsystem->GetLobbyPlayers();
		for (const FMGNetPlayer& Player : Players)
		{
			if (Player.PlayerID == PlayerID)
			{
				OnPlayerReadyChanged(PlayerID, Player.bIsReady);
				break;
			}
		}
	}
}

void UMGLobbyWidget::HandleSettingsChanged(const FMGLobbySettings& Settings)
{
	CurrentSettings = Settings;
	UpdateLobbySettings();
	OnSettingsChanged(Settings);
}

void UMGLobbyWidget::HandleRaceStarting()
{
	OnRaceStarting();
}

// ==========================================
// UMGSessionBrowserWidget
// ==========================================

void UMGSessionBrowserWidget::RefreshSessions()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGMultiplayerSubsystem* MP = GI->GetSubsystem<UMGMultiplayerSubsystem>())
		{
			MP->SearchSessions(CurrentTrackFilter);

			// Would be async - for now just get results
			TArray<FMGSessionInfo> Sessions = MP->GetSessionSearchResults();
			OnSessionsUpdated(Sessions);
		}
	}
}

void UMGSessionBrowserWidget::SetTrackFilter(FName TrackID)
{
	CurrentTrackFilter = TrackID;
	RefreshSessions();
}

void UMGSessionBrowserWidget::JoinSelectedSession()
{
	if (!SelectedSession.SessionID.IsEmpty())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UMGMultiplayerSubsystem* MP = GI->GetSubsystem<UMGMultiplayerSubsystem>())
			{
				MP->JoinSession(SelectedSession.SessionID);
			}
		}
	}
}

void UMGSessionBrowserWidget::JoinByCode(const FString& InviteCode)
{
	if (!InviteCode.IsEmpty())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UMGMultiplayerSubsystem* MP = GI->GetSubsystem<UMGMultiplayerSubsystem>())
			{
				MP->JoinByInviteCode(InviteCode);
			}
		}
	}
}

// ==========================================
// UMGMatchmakingWidget
// ==========================================

void UMGMatchmakingWidget::StartQuickMatch()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGMultiplayerSubsystem* MP = GI->GetSubsystem<UMGMultiplayerSubsystem>())
		{
			MP->StartQuickMatch();
			bIsMatchmaking = true;
			MatchmakingStartTime = GetWorld()->GetTimeSeconds();
		}
	}
}

void UMGMatchmakingWidget::StartRankedMatch()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGMultiplayerSubsystem* MP = GI->GetSubsystem<UMGMultiplayerSubsystem>())
		{
			MP->StartRankedMatch();
			bIsMatchmaking = true;
			MatchmakingStartTime = GetWorld()->GetTimeSeconds();
		}
	}
}

void UMGMatchmakingWidget::CancelMatchmaking()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGMultiplayerSubsystem* MP = GI->GetSubsystem<UMGMultiplayerSubsystem>())
		{
			MP->CancelMatchmaking();
			bIsMatchmaking = false;
			OnMatchmakingCancelled();
		}
	}
}

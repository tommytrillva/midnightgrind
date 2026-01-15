// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Multiplayer/MGMultiplayerSubsystem.h"
#include "MGLobbyWidget.generated.h"

class UMGMultiplayerSubsystem;
class UVerticalBox;
class UButton;
class UTextBlock;
class UImage;
class UComboBoxString;

/**
 * Player slot widget for lobby display
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPlayerSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Update with player data */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PlayerSlot")
	void UpdatePlayerData(const FMGNetPlayer& PlayerData);

	/** Set empty slot */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PlayerSlot")
	void SetEmpty();

	/** Set as local player */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PlayerSlot")
	void SetLocalPlayer(bool bIsLocal);

protected:
	/** Player data */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerSlot")
	FMGNetPlayer CurrentPlayerData;

	/** Is this the local player */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerSlot")
	bool bIsLocalPlayer = false;

	/** Is slot empty */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerSlot")
	bool bIsEmpty = true;
};

/**
 * Lobby Widget
 * Main lobby interface
 *
 * Features:
 * - Player list display
 * - Vehicle selection
 * - Track voting
 * - Ready state management
 * - Chat (future)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ==========================================
	// INTERFACE
	// ==========================================

	/** Initialize lobby widget */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void InitializeLobby();

	/** Update player list */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdatePlayerList();

	/** Update lobby settings display */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdateLobbySettings();

	/** Set ready state */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bReady);

	/** Select vehicle */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SelectVehicle(FName VehicleID);

	/** Leave lobby */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void LeaveLobby();

	/** Start race (host only) */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartRace();

	// ==========================================
	// HOST CONTROLS
	// ==========================================

	/** Change track selection */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Host")
	void ChangeTrack(FName TrackID);

	/** Change lap count */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Host")
	void ChangeLapCount(int32 Laps);

	/** Kick player */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Host")
	void KickPlayer(const FString& PlayerID);

	// ==========================================
	// QUERY
	// ==========================================

	/** Get current track selection */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	FName GetSelectedTrack() const;

	/** Can start race */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool CanStartRace() const;

	/** Is local player host */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsHost() const;

	/** Get invite code */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	FString GetInviteCode() const;

protected:
	// ==========================================
	// BLUEPRINT EVENTS
	// ==========================================

	/** Called when player joins */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnPlayerJoinedLobby(const FMGNetPlayer& Player);

	/** Called when player leaves */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnPlayerLeftLobby(const FString& PlayerID);

	/** Called when player ready state changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnPlayerReadyChanged(const FString& PlayerID, bool bReady);

	/** Called when settings change */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnSettingsChanged(const FMGLobbySettings& Settings);

	/** Called when race is starting */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnRaceStarting();

	/** Called when countdown updates */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnCountdownUpdate(float TimeRemaining);

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Player slot widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGPlayerSlotWidget> PlayerSlotClass;

	/** Maximum slots to show */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	int32 MaxDisplaySlots = 8;

	// ==========================================
	// STATE
	// ==========================================

	/** Reference to multiplayer subsystem */
	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	UMGMultiplayerSubsystem* MultiplayerSubsystem;

	/** Player slot widgets */
	UPROPERTY()
	TArray<UMGPlayerSlotWidget*> PlayerSlotWidgets;

	/** Current lobby settings */
	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	FMGLobbySettings CurrentSettings;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Bind to multiplayer events */
	void BindEvents();

	/** Unbind from events */
	void UnbindEvents();

	/** Handle player joined */
	UFUNCTION()
	void HandlePlayerJoined(const FMGNetPlayer& Player);

	/** Handle player left */
	UFUNCTION()
	void HandlePlayerLeft(const FString& PlayerID);

	/** Handle player ready */
	UFUNCTION()
	void HandlePlayerReady(const FString& PlayerID);

	/** Handle settings changed */
	UFUNCTION()
	void HandleSettingsChanged(const FMGLobbySettings& Settings);

	/** Handle race starting */
	UFUNCTION()
	void HandleRaceStarting();
};

/**
 * Session Browser Widget
 * For finding and joining sessions
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSessionBrowserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Refresh session list */
	UFUNCTION(BlueprintCallable, Category = "SessionBrowser")
	void RefreshSessions();

	/** Filter by track */
	UFUNCTION(BlueprintCallable, Category = "SessionBrowser")
	void SetTrackFilter(FName TrackID);

	/** Join selected session */
	UFUNCTION(BlueprintCallable, Category = "SessionBrowser")
	void JoinSelectedSession();

	/** Join by invite code */
	UFUNCTION(BlueprintCallable, Category = "SessionBrowser")
	void JoinByCode(const FString& InviteCode);

protected:
	/** Called when sessions are updated */
	UFUNCTION(BlueprintImplementableEvent, Category = "SessionBrowser")
	void OnSessionsUpdated(const TArray<FMGSessionInfo>& Sessions);

	/** Called when join fails */
	UFUNCTION(BlueprintImplementableEvent, Category = "SessionBrowser")
	void OnJoinFailed(const FString& Reason);

	/** Selected session */
	UPROPERTY(BlueprintReadOnly, Category = "SessionBrowser")
	FMGSessionInfo SelectedSession;

	/** Track filter */
	UPROPERTY(BlueprintReadOnly, Category = "SessionBrowser")
	FName CurrentTrackFilter;
};

/**
 * Matchmaking Widget
 * Quick match/ranked queue display
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGMatchmakingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Start quick match */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void StartQuickMatch();

	/** Start ranked match */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void StartRankedMatch();

	/** Cancel matchmaking */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void CancelMatchmaking();

protected:
	/** Called when matchmaking progress updates */
	UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
	void OnMatchmakingProgress(int32 PlayersFound, int32 PlayersNeeded, float TimeElapsed);

	/** Called when match found */
	UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
	void OnMatchFound();

	/** Called when matchmaking cancelled */
	UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
	void OnMatchmakingCancelled();

	/** Is matchmaking */
	UPROPERTY(BlueprintReadOnly, Category = "Matchmaking")
	bool bIsMatchmaking = false;

	/** Matchmaking start time */
	float MatchmakingStartTime = 0.0f;
};

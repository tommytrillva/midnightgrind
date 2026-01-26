// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSessionManagerSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGSessionState : uint8
{
	None,
	Creating,
	Joining,
	InLobby,
	Starting,
	InProgress,
	Ending,
	PostGame,
	Disconnected,
	Error
};

UENUM(BlueprintType)
enum class EMGSessionType : uint8
{
	Singleplayer,
	LocalMultiplayer,
	OnlinePrivate,
	OnlinePublic,
	Ranked,
	Tournament,
	FreeroamPublic,
	FreeroamPrivate
};

UENUM(BlueprintType)
enum class EMGDisconnectReason : uint8
{
	Unknown,
	PlayerQuit,
	HostClosed,
	Kicked,
	Banned,
	Timeout,
	NetworkError,
	ServerShutdown,
	SessionFull,
	VersionMismatch,
	AuthenticationFailed,
	MaintenanceMode
};

UENUM(BlueprintType)
enum class EMGMigrationState : uint8
{
	None,
	HostMigrationStarted,
	WaitingForNewHost,
	NewHostSelected,
	MigrationComplete,
	MigrationFailed
};

USTRUCT(BlueprintType)
struct FMGSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionType Type = EMGSessionType::Singleplayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionState State = EMGSessionState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJoinInProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameMode = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Region;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GameVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomData;
};

USTRUCT(BlueprintType)
struct FMGSessionPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHost = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSpectator = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Platform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime JoinTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> PlayerData;
};

USTRUCT(BlueprintType)
struct FMGSessionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionType Type = EMGSessionType::OnlinePrivate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSpectators = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowJoinInProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowInvites = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldAdvertise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseDedicatedServer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableHostMigration = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCrossPlayEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PreferredRegion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameMode = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomSettings;
};

USTRUCT(BlueprintType)
struct FMGSessionSearchFilters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionType TypeFilter = EMGSessionType::OnlinePublic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeFilter = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapFilter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideFullSessions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideInProgressSessions = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPing = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RegionFilter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCrossPlayEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxResults = 50;
};

USTRUCT(BlueprintType)
struct FMGSessionInvite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InviteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGSessionInfo SessionInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SentTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiryTime;
};

USTRUCT(BlueprintType)
struct FMGConnectionAttempt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AttemptNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAttempts = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttemptStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeoutSeconds = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LastError;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStateChanged, EMGSessionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, const FMGSessionInfo&, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, const FMGSessionInfo&, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionJoinFailed, const FString&, SessionID, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionEnded, EMGDisconnectReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionUpdated, const FMGSessionInfo&, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, const FMGSessionPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeft, const FString&, PlayerID, EMGDisconnectReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDataChanged, const FString&, PlayerID, const FMGSessionPlayer&, PlayerData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete, const TArray<FMGSessionInfo>&, Sessions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteReceived, const FMGSessionInvite&, Invite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInviteSent, const FString&, RecipientID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostMigrationStateChanged, EMGMigrationState, MigrationState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewHostSelected, const FString&, NewHostPlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConnectionAttempt, int32, AttemptNumber, int32, MaxAttempts);

UCLASS()
class MIDNIGHTGRIND_API UMGSessionManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Session Creation
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	bool CreateSession(const FMGSessionSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	bool UpdateSession(const FMGSessionSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void DestroySession();

	UFUNCTION(BlueprintPure, Category = "Session|Host")
	bool IsSessionHost() const;

	// Session Joining
	UFUNCTION(BlueprintCallable, Category = "Session|Join")
	bool JoinSession(const FString& SessionID, const FString& Password = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Session|Join")
	bool JoinSessionByInfo(const FMGSessionInfo& SessionInfo, const FString& Password = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Session|Join")
	void CancelJoin();

	UFUNCTION(BlueprintCallable, Category = "Session|Join")
	void LeaveSession();

	// Session State
	UFUNCTION(BlueprintPure, Category = "Session|State")
	bool IsInSession() const;

	UFUNCTION(BlueprintPure, Category = "Session|State")
	EMGSessionState GetSessionState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Session|State")
	FMGSessionInfo GetCurrentSession() const { return CurrentSession; }

	UFUNCTION(BlueprintPure, Category = "Session|State")
	TArray<FMGSessionPlayer> GetSessionPlayers() const { return SessionPlayers; }

	UFUNCTION(BlueprintPure, Category = "Session|State")
	FMGSessionPlayer GetLocalPlayer() const;

	UFUNCTION(BlueprintPure, Category = "Session|State")
	FMGSessionPlayer GetPlayer(const FString& PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Session|State")
	int32 GetPlayerCount() const { return SessionPlayers.Num(); }

	// Session Management (Host only)
	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool KickPlayer(const FString& PlayerID, const FString& Reason = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool BanPlayer(const FString& PlayerID, const FString& Reason = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool SetSessionLocked(bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool TransferHost(const FString& NewHostPlayerID);

	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool StartSession();

	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool EndSession();

	// Player Data
	UFUNCTION(BlueprintCallable, Category = "Session|Player")
	void SetLocalPlayerReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Session|Player")
	void SetLocalPlayerTeam(int32 TeamIndex);

	UFUNCTION(BlueprintCallable, Category = "Session|Player")
	void SetLocalPlayerSpectator(bool bSpectator);

	UFUNCTION(BlueprintCallable, Category = "Session|Player")
	void SetLocalPlayerData(FName Key, const FString& Value);

	UFUNCTION(BlueprintPure, Category = "Session|Player")
	FString GetLocalPlayerData(FName Key) const;

	UFUNCTION(BlueprintPure, Category = "Session|Player")
	bool AreAllPlayersReady() const;

	// Session Search
	UFUNCTION(BlueprintCallable, Category = "Session|Search")
	void SearchSessions(const FMGSessionSearchFilters& Filters);

	UFUNCTION(BlueprintCallable, Category = "Session|Search")
	void CancelSearch();

	UFUNCTION(BlueprintPure, Category = "Session|Search")
	bool IsSearching() const { return bSearching; }

	UFUNCTION(BlueprintPure, Category = "Session|Search")
	TArray<FMGSessionInfo> GetSearchResults() const { return SearchResults; }

	UFUNCTION(BlueprintCallable, Category = "Session|Search")
	void RefreshSession(const FString& SessionID);

	// Invites
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	bool SendInvite(const FString& RecipientPlayerID);

	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	bool AcceptInvite(const FMGSessionInvite& Invite);

	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void DeclineInvite(const FMGSessionInvite& Invite);

	UFUNCTION(BlueprintPure, Category = "Session|Invite")
	TArray<FMGSessionInvite> GetPendingInvites() const { return PendingInvites; }

	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void ClearExpiredInvites();

	// Host Migration
	UFUNCTION(BlueprintPure, Category = "Session|Migration")
	EMGMigrationState GetMigrationState() const { return MigrationState; }

	UFUNCTION(BlueprintPure, Category = "Session|Migration")
	bool IsHostMigrationInProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Session|Migration")
	void RequestHostMigration();

	// Network Quality
	UFUNCTION(BlueprintPure, Category = "Session|Network")
	int32 GetAverageSessionPing() const;

	UFUNCTION(BlueprintPure, Category = "Session|Network")
	float GetPacketLoss() const { return PacketLossPercent; }

	UFUNCTION(BlueprintCallable, Category = "Session|Network")
	void UpdateNetworkStats();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionStateChanged OnSessionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionJoined OnSessionJoined;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionJoinFailed OnSessionJoinFailed;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionEnded OnSessionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionUpdated OnSessionUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnPlayerJoined OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnPlayerLeft OnPlayerLeft;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnPlayerDataChanged OnPlayerDataChanged;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionSearchComplete OnSessionSearchComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionSearchFailed OnSessionSearchFailed;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnInviteReceived OnInviteReceived;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnInviteSent OnInviteSent;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnHostMigrationStateChanged OnHostMigrationStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnNewHostSelected OnNewHostSelected;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnConnectionAttempt OnConnectionAttempt;

protected:
	void SetSessionState(EMGSessionState NewState);
	void SetMigrationState(EMGMigrationState NewState);
	void OnSessionTick();
	void OnJoinTimeout();
	void RetryConnection();
	void SimulateSessionCreation(const FMGSessionSettings& Settings);
	void SimulateSessionSearch();
	void SimulateHostMigration();
	FMGSessionPlayer CreateLocalPlayer() const;
	void AddLocalPlayerToSession();

	UPROPERTY()
	EMGSessionState CurrentState = EMGSessionState::None;

	UPROPERTY()
	FMGSessionInfo CurrentSession;

	UPROPERTY()
	TArray<FMGSessionPlayer> SessionPlayers;

	UPROPERTY()
	TArray<FMGSessionInfo> SearchResults;

	UPROPERTY()
	TArray<FMGSessionInvite> PendingInvites;

	UPROPERTY()
	EMGMigrationState MigrationState = EMGMigrationState::None;

	UPROPERTY()
	FMGConnectionAttempt CurrentConnectionAttempt;

	UPROPERTY()
	bool bSearching = false;

	UPROPERTY()
	float PacketLossPercent = 0.0f;

	UPROPERTY()
	TArray<FString> BannedPlayerIDs;

	FTimerHandle SessionTickHandle;
	FTimerHandle JoinTimeoutHandle;
	FTimerHandle SearchTimeoutHandle;
};

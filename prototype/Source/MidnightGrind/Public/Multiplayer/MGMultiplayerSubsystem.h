// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMultiplayerSubsystem.generated.h"

class UMGOnlineProfileSubsystem;

/**
 * Connection state
 */
UENUM(BlueprintType)
enum class EMGConnectionState : uint8
{
	Disconnected,
	Connecting,
	Connected,
	InLobby,
	InRace,
	Reconnecting
};

/**
 * Match type
 */
UENUM(BlueprintType)
enum class EMGMatchType : uint8
{
	/** Quick match - auto matchmaking */
	QuickMatch,
	/** Private lobby */
	Private,
	/** Ranked competitive */
	Ranked,
	/** Custom rules */
	Custom
};

/**
 * Race result for a single player
 */
USTRUCT(BlueprintType)
struct FMGRaceResult
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Player display name */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	/** Final position (1-based) */
	UPROPERTY(BlueprintReadOnly)
	int32 Position = 0;

	/** Total race time */
	UPROPERTY(BlueprintReadOnly)
	float TotalTime = 0.0f;

	/** Best lap time */
	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	/** Did not finish */
	UPROPERTY(BlueprintReadOnly)
	bool bDNF = false;

	/** Disconnected during race */
	UPROPERTY(BlueprintReadOnly)
	bool bDisconnected = false;

	/** Vehicle used */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/** Reputation earned */
	UPROPERTY(BlueprintReadOnly)
	int32 ReputationEarned = 0;

	/** Cash earned */
	UPROPERTY(BlueprintReadOnly)
	int32 CashEarned = 0;
};

/**
 * Session info
 */
USTRUCT(BlueprintType)
struct FMGSessionInfo
{
	GENERATED_BODY()

	/** Session unique ID */
	UPROPERTY(BlueprintReadOnly)
	FString SessionID;

	/** Host player ID */
	UPROPERTY(BlueprintReadOnly)
	FString HostPlayerID;

	/** Track ID */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Match type */
	UPROPERTY(BlueprintReadOnly)
	EMGMatchType MatchType = EMGMatchType::QuickMatch;

	/** Number of laps */
	UPROPERTY(BlueprintReadOnly)
	int32 LapCount = 3;

	/** Max players */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 8;

	/** Current player count */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers = 0;

	/** Is session joinable */
	UPROPERTY(BlueprintReadOnly)
	bool bIsJoinable = true;

	/** Session region */
	UPROPERTY(BlueprintReadOnly)
	FString Region;

	/** Average ping to session */
	UPROPERTY(BlueprintReadOnly)
	int32 Ping = 0;
};

/**
 * Player in lobby/race
 */
USTRUCT(BlueprintType)
struct FMGNetPlayer
{
	GENERATED_BODY()

	/** Player unique ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/** Selected vehicle */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/** Player level */
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/** Reputation */
	UPROPERTY(BlueprintReadOnly)
	int32 Reputation = 0;

	/** Is ready */
	UPROPERTY(BlueprintReadOnly)
	bool bIsReady = false;

	/** Is host */
	UPROPERTY(BlueprintReadOnly)
	bool bIsHost = false;

	/** Ping to host */
	UPROPERTY(BlueprintReadOnly)
	int32 Ping = 0;

	/** Network actor (only valid during race) */
	UPROPERTY(BlueprintReadOnly)
	AActor* PlayerActor = nullptr;
};

/**
 * Lobby settings
 */
USTRUCT(BlueprintType)
struct FMGLobbySettings
{
	GENERATED_BODY()

	/** Track to race on */
	UPROPERTY(BlueprintReadWrite)
	FName TrackID;

	/** Number of laps */
	UPROPERTY(BlueprintReadWrite)
	int32 LapCount = 3;

	/** Max players */
	UPROPERTY(BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/** Match type */
	UPROPERTY(BlueprintReadWrite)
	EMGMatchType MatchType = EMGMatchType::QuickMatch;

	/** Is private (invite only) */
	UPROPERTY(BlueprintReadWrite)
	bool bIsPrivate = false;

	/** Allow spectators */
	UPROPERTY(BlueprintReadWrite)
	bool bAllowSpectators = true;

	/** Auto-start when all ready */
	UPROPERTY(BlueprintReadWrite)
	bool bAutoStart = true;

	/** Countdown time before race starts */
	UPROPERTY(BlueprintReadWrite)
	float CountdownTime = 5.0f;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionStateChanged, EMGConnectionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, const FMGSessionInfo&, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, const FMGNetPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReady, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbySettingsChanged, const FMGLobbySettings&, Settings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceEnded, const TArray<FMGRaceResult>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchmakingProgress, int32, PlayersFound, int32, PlayersNeeded);

/**
 * Multiplayer Subsystem
 * Central management for online racing
 *
 * Features:
 * - Session management (create, join, leave)
 * - Matchmaking integration
 * - Lobby management
 * - Race synchronization
 * - Results tracking
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnConnectionStateChanged OnConnectionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSessionJoined OnSessionJoined;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSessionLeft OnSessionLeft;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerJoined OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerLeft OnPlayerLeft;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerReady OnPlayerReady;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLobbySettingsChanged OnLobbySettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarting OnRaceStarting;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarted OnRaceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceEnded OnRaceEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchmakingProgress OnMatchmakingProgress;

	// ==========================================
	// CONNECTION
	// ==========================================

	/** Connect to game services */
	UFUNCTION(BlueprintCallable, Category = "Connection")
	void Connect();

	/** Disconnect from services */
	UFUNCTION(BlueprintCallable, Category = "Connection")
	void Disconnect();

	/** Get connection state */
	UFUNCTION(BlueprintPure, Category = "Connection")
	EMGConnectionState GetConnectionState() const { return ConnectionState; }

	/** Is connected to services */
	UFUNCTION(BlueprintPure, Category = "Connection")
	bool IsConnected() const { return ConnectionState >= EMGConnectionState::Connected; }

	/** Get local player info */
	UFUNCTION(BlueprintPure, Category = "Connection")
	FMGNetPlayer GetLocalPlayer() const { return LocalPlayer; }

	// ==========================================
	// MATCHMAKING
	// ==========================================

	/** Start quick match matchmaking */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void StartQuickMatch(FName PreferredTrack = NAME_None);

	/** Start ranked matchmaking */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void StartRankedMatch();

	/** Cancel matchmaking */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void CancelMatchmaking();

	/** Is currently matchmaking */
	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	bool IsMatchmaking() const { return bIsMatchmaking; }

	// ==========================================
	// SESSION
	// ==========================================

	/** Create a new session */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(const FMGLobbySettings& Settings);

	/** Join session by ID */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSession(const FString& SessionID);

	/** Join session by invite code */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinByInviteCode(const FString& InviteCode);

	/** Leave current session */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void LeaveSession();

	/** Get current session info */
	UFUNCTION(BlueprintPure, Category = "Session")
	FMGSessionInfo GetCurrentSession() const { return CurrentSession; }

	/** Is in a session */
	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsInSession() const { return bInSession; }

	/** Get invite code for current session */
	UFUNCTION(BlueprintPure, Category = "Session")
	FString GetInviteCode() const;

	/** Search for available sessions */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SearchSessions(FName TrackFilter = NAME_None);

	/** Get search results */
	UFUNCTION(BlueprintPure, Category = "Session")
	TArray<FMGSessionInfo> GetSessionSearchResults() const { return SessionSearchResults; }

	// ==========================================
	// LOBBY
	// ==========================================

	/** Get all players in lobby */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	TArray<FMGNetPlayer> GetLobbyPlayers() const { return LobbyPlayers; }

	/** Get lobby settings */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	FMGLobbySettings GetLobbySettings() const { return LobbySettings; }

	/** Set ready state */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bReady);

	/** Is local player ready */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsLocalPlayerReady() const { return LocalPlayer.bIsReady; }

	/** Set vehicle selection */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetSelectedVehicle(FName VehicleID);

	/** Update lobby settings (host only) */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdateLobbySettings(const FMGLobbySettings& NewSettings);

	/** Kick player (host only) */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void KickPlayer(const FString& PlayerID);

	/** Start the race (host only) */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartRace();

	/** Is local player the host */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsHost() const { return LocalPlayer.bIsHost; }

	/** Can the race start (all ready, enough players) */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool CanStartRace() const;

	// ==========================================
	// RACE
	// ==========================================

	/** Get race elapsed time (synced) */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetRaceTime() const { return RaceTime; }

	/** Get race countdown time */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetCountdownTime() const { return CountdownTime; }

	/** Is race in progress */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool IsRaceInProgress() const { return ConnectionState == EMGConnectionState::InRace; }

	/** Report race finish (called by local player when crossing finish) */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void ReportRaceFinish(float FinalTime, float BestLapTime);

	/** Report lap time */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void ReportLapTime(int32 LapNumber, float LapTime);

	/** Get race results (after race ends) */
	UFUNCTION(BlueprintPure, Category = "Race")
	TArray<FMGRaceResult> GetRaceResults() const { return RaceResults; }

	// ==========================================
	// NETWORK
	// ==========================================

	/** Get current ping to host */
	UFUNCTION(BlueprintPure, Category = "Network")
	int32 GetPing() const { return CurrentPing; }

	/** Get server time offset */
	UFUNCTION(BlueprintPure, Category = "Network")
	float GetServerTimeOffset() const { return ServerTimeOffset; }

protected:
	// ==========================================
	// STATE
	// ==========================================

	/** Current connection state */
	EMGConnectionState ConnectionState = EMGConnectionState::Disconnected;

	/** Local player info */
	FMGNetPlayer LocalPlayer;

	/** Is in a session */
	bool bInSession = false;

	/** Current session info */
	FMGSessionInfo CurrentSession;

	/** Players in current lobby */
	TArray<FMGNetPlayer> LobbyPlayers;

	/** Lobby settings */
	FMGLobbySettings LobbySettings;

	/** Is matchmaking active */
	bool bIsMatchmaking = false;

	/** Session search results */
	TArray<FMGSessionInfo> SessionSearchResults;

	/** Race time (synced) */
	float RaceTime = 0.0f;

	/** Countdown time */
	float CountdownTime = 0.0f;

	/** Race results */
	TArray<FMGRaceResult> RaceResults;

	/** Current ping */
	int32 CurrentPing = 0;

	/** Server time offset for sync */
	float ServerTimeOffset = 0.0f;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Set connection state */
	void SetConnectionState(EMGConnectionState NewState);

	/** Handle matchmaking update */
	void OnMatchmakingUpdate(int32 PlayersFound, int32 PlayersNeeded);

	/** Handle session found */
	void OnSessionFound(const FMGSessionInfo& Session);

	/** Sync time with server */
	void SyncServerTime();

	/** Generate invite code from session ID */
	FString GenerateInviteCode(const FString& SessionID) const;

	/** Decode invite code to session ID */
	FString DecodeInviteCode(const FString& Code) const;
};

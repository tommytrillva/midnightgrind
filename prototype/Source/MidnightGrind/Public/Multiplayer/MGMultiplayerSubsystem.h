// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGMultiplayerSubsystem.h
 * @brief Low-Level Multiplayer Network Management for Midnight Grind
 *
 * This subsystem handles the core networking aspects of multiplayer racing,
 * including connection management, session control, lobby coordination, and
 * race synchronization. It operates at a lower level than MGSessionSubsystem,
 * dealing directly with network state and player replication.
 *
 * ## Responsibilities
 * - **Connection Management**: Establishing and maintaining connections to game servers
 * - **Session Control**: Creating, joining, and leaving game sessions
 * - **Lobby Coordination**: Managing pre-race player states and settings
 * - **Race Synchronization**: Keeping race state consistent across all clients
 * - **Results Tracking**: Collecting and distributing race finish data
 *
 * ## Network Architecture
 * Midnight Grind uses a client-server model where one player hosts (or a dedicated
 * server runs). This subsystem abstracts the networking layer, providing a clean
 * interface for game systems to interact with multiplayer functionality.
 *
 * ## State Machine
 * The connection progresses through defined states:
 * Disconnected -> Connecting -> Connected -> InLobby -> InRace
 *
 * With Reconnecting as a recovery state for dropped connections during races.
 *
 * ## Integration
 * This subsystem works alongside:
 * - UMGSessionSubsystem: Higher-level session/party management
 * - UMGMatchmakingSubsystem: Player matching and queue management
 * - UMGOnlineProfileSubsystem: Player identity and progression
 *
 * @see UMGSessionSubsystem For the primary multiplayer interface
 * @see UMGMatchmakingSubsystem For detailed matchmaking configuration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMultiplayerSubsystem.generated.h"

class UMGOnlineProfileSubsystem;

// ============================================================================
// Connection and Match Type Enumerations
// ============================================================================

/**
 * @brief Represents the current network connection state.
 *
 * Tracks the lifecycle of a multiplayer connection from initial
 * connect through active gameplay.
 */
UENUM(BlueprintType)
enum class EMGConnectionState : uint8
{
	/// Not connected to any game server
	Disconnected,
	/// Attempting to establish connection
	Connecting,
	/// Connected to server, not yet in a lobby
	Connected,
	/// In a pre-race lobby waiting for race to start
	InLobby,
	/// Actively participating in a race
	InRace,
	/// Connection lost during race, attempting recovery
	Reconnecting
};

/**
 * @brief Types of multiplayer matches available.
 *
 * Different match types have different rules for matchmaking,
 * rewards, and session visibility.
 */
UENUM(BlueprintType)
enum class EMGMatchType : uint8
{
	/** Quick match - auto matchmaking for casual play */
	QuickMatch,
	/** Private lobby - invite only, no matchmaking */
	Private,
	/** Ranked competitive - skill-based matching, affects rating */
	Ranked,
	/** Custom rules - host defines special settings */
	Custom
};

// ============================================================================
// Race Result Structures
// ============================================================================

/**
 * @brief Complete race result data for a single player.
 *
 * Contains all information about a player's performance in a completed race,
 * used for leaderboards, rewards calculation, and replay data.
 */
USTRUCT(BlueprintType)
struct FMGRaceResult
{
	GENERATED_BODY()

	/// Unique platform identifier for the player
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/// Human-readable player name for display
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	/// Final finishing position (1 = first place, 1-based indexing)
	UPROPERTY(BlueprintReadOnly)
	int32 Position = 0;

	/// Total time to complete all laps (in seconds)
	UPROPERTY(BlueprintReadOnly)
	float TotalTime = 0.0f;

	/// Fastest single lap time achieved (in seconds)
	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	/// True if player did not finish (retired, timeout, etc.)
	UPROPERTY(BlueprintReadOnly)
	bool bDNF = false;

	/// True if player's connection was lost during the race
	UPROPERTY(BlueprintReadOnly)
	bool bDisconnected = false;

	/// ID of the vehicle used in this race
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/// Reputation points earned from this race
	UPROPERTY(BlueprintReadOnly)
	int32 ReputationEarned = 0;

	/// In-game currency earned from this race
	UPROPERTY(BlueprintReadOnly)
	int32 CashEarned = 0;
};

// ============================================================================
// Session and Player Structures
// ============================================================================

/**
 * @brief Core session information for network operations.
 *
 * This is the network-level view of a session, containing connection
 * details and capacity information needed for joining and display.
 */
USTRUCT(BlueprintType)
struct FMGSessionInfo
{
	GENERATED_BODY()

	/// Unique identifier for network operations
	UPROPERTY(BlueprintReadOnly)
	FString SessionID;

	/// Platform ID of the player hosting this session
	UPROPERTY(BlueprintReadOnly)
	FString HostPlayerID;

	/// ID of the track selected for racing
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/// Type of match (affects rewards and matchmaking)
	UPROPERTY(BlueprintReadOnly)
	EMGMatchType MatchType = EMGMatchType::QuickMatch;

	/// Number of laps for the race
	UPROPERTY(BlueprintReadOnly)
	int32 LapCount = 3;

	/// Maximum players the session can hold
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 8;

	/// Current number of connected players
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers = 0;

	/// Whether new players can still join
	UPROPERTY(BlueprintReadOnly)
	bool bIsJoinable = true;

	/// Geographic region of the server (e.g., "US-East", "EU-West")
	UPROPERTY(BlueprintReadOnly)
	FString Region;

	/// Network latency to this session in milliseconds
	UPROPERTY(BlueprintReadOnly)
	int32 Ping = 0;
};

/**
 * @brief Network player data for lobby and race.
 *
 * Contains all replicated information about a player needed
 * for lobby display and in-race identification.
 */
USTRUCT(BlueprintType)
struct FMGNetPlayer
{
	GENERATED_BODY()

	/// Unique platform identifier
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/// Human-readable display name
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/// Currently selected vehicle
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/// Player's progression level
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/// Player's accumulated reputation
	UPROPERTY(BlueprintReadOnly)
	int32 Reputation = 0;

	/// Whether player has marked ready in lobby
	UPROPERTY(BlueprintReadOnly)
	bool bIsReady = false;

	/// Whether this player is hosting the session
	UPROPERTY(BlueprintReadOnly)
	bool bIsHost = false;

	/// Network latency to host in milliseconds
	UPROPERTY(BlueprintReadOnly)
	int32 Ping = 0;

	/// Pointer to the player's vehicle actor (valid only during race)
	UPROPERTY(BlueprintReadOnly)
	AActor* PlayerActor = nullptr;
};

/**
 * @brief Configurable settings for a lobby session.
 *
 * Host can modify these settings while in lobby to customize
 * the race experience for all players.
 */
USTRUCT(BlueprintType)
struct FMGLobbySettings
{
	GENERATED_BODY()

	/// ID of the track to race on
	UPROPERTY(BlueprintReadWrite)
	FName TrackID;

	/// Number of laps to complete
	UPROPERTY(BlueprintReadWrite)
	int32 LapCount = 3;

	/// Maximum players allowed
	UPROPERTY(BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/// Type of match (affects visibility and rewards)
	UPROPERTY(BlueprintReadWrite)
	EMGMatchType MatchType = EMGMatchType::QuickMatch;

	/// If true, only invited players can join
	UPROPERTY(BlueprintReadWrite)
	bool bIsPrivate = false;

	/// Allow non-racing spectators in the session
	UPROPERTY(BlueprintReadWrite)
	bool bAllowSpectators = true;

	/// Automatically start race when all players ready
	UPROPERTY(BlueprintReadWrite)
	bool bAutoStart = true;

	/// Seconds of countdown before race begins
	UPROPERTY(BlueprintReadWrite)
	float CountdownTime = 5.0f;
};

// ============================================================================
// Event Delegates
// These provide reactive updates for UI and game systems.
// ============================================================================

/** @brief Fired when the network connection state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionStateChanged, EMGConnectionState, NewState);

/** @brief Fired when successfully joining a session. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, const FMGSessionInfo&, SessionInfo);

/** @brief Fired when leaving or disconnecting from a session. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionLeft);

/** @brief Fired when a new player joins the current session. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, const FMGNetPlayer&, Player);

/** @brief Fired when a player leaves or disconnects from the session. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, const FString&, PlayerID);

/** @brief Fired when a player changes their ready state. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReady, const FString&, PlayerID);

/** @brief Fired when the host changes lobby settings. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbySettingsChanged, const FMGLobbySettings&, Settings);

/** @brief Fired when the countdown to race start begins. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarting);

/** @brief Fired when the race officially starts (green light). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);

/** @brief Fired when the race ends and results are available. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceEnded, const TArray<FMGRaceResult>&, Results);

/** @brief Fired during matchmaking to report search progress. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchmakingProgress, int32, PlayersFound, int32, PlayersNeeded);

// ============================================================================
// Multiplayer Subsystem Class
// ============================================================================

/**
 * @brief Core networking subsystem for Midnight Grind multiplayer.
 *
 * Manages the low-level network operations for multiplayer racing including
 * connection lifecycle, session management, lobby coordination, and race
 * state synchronization.
 *
 * ## Features
 * - Session management (create, join, leave)
 * - Matchmaking integration
 * - Lobby management
 * - Race synchronization
 * - Results tracking
 *
 * ## Usage Example
 * @code
 * // Get the subsystem
 * UMGMultiplayerSubsystem* MP = GetGameInstance()->GetSubsystem<UMGMultiplayerSubsystem>();
 *
 * // Connect and create a session
 * MP->OnConnectionStateChanged.AddDynamic(this, &UMyWidget::HandleConnectionChanged);
 * MP->Connect();
 *
 * // In the callback, create session when connected
 * if (NewState == EMGConnectionState::Connected)
 * {
 *     FMGLobbySettings Settings;
 *     Settings.TrackID = "Track_Downtown";
 *     Settings.LapCount = 3;
 *     MP->CreateSession(Settings);
 * }
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// EVENT DELEGATES
	// Subscribe to these to react to network state changes in your UI/game code.
	// ========================================================================

	/** @brief Broadcast when connection state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnConnectionStateChanged OnConnectionStateChanged;

	/** @brief Broadcast when successfully joining a session. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSessionJoined OnSessionJoined;

	/** @brief Broadcast when leaving a session. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSessionLeft OnSessionLeft;

	/** @brief Broadcast when a player joins the session. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerJoined OnPlayerJoined;

	/** @brief Broadcast when a player leaves the session. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerLeft OnPlayerLeft;

	/** @brief Broadcast when a player's ready state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerReady OnPlayerReady;

	/** @brief Broadcast when lobby settings are modified. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLobbySettingsChanged OnLobbySettingsChanged;

	/** @brief Broadcast when race countdown begins. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarting OnRaceStarting;

	/** @brief Broadcast when race starts (green light). */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarted OnRaceStarted;

	/** @brief Broadcast when race ends with results. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceEnded OnRaceEnded;

	/** @brief Broadcast during matchmaking with progress updates. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchmakingProgress OnMatchmakingProgress;

	// ========================================================================
	// CONNECTION
	// Functions for managing the network connection to game services.
	// Must be connected before creating or joining sessions.
	// ========================================================================

	/**
	 * @brief Initiates connection to the game services.
	 * @note Listen for OnConnectionStateChanged to know when connected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Connection")
	void Connect();

	/**
	 * @brief Disconnects from all game services and sessions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Connection")
	void Disconnect();

	/**
	 * @brief Gets the current connection state.
	 * @return Current state of the network connection
	 */
	UFUNCTION(BlueprintPure, Category = "Connection")
	EMGConnectionState GetConnectionState() const { return ConnectionState; }

	/**
	 * @brief Checks if connected to game services.
	 * @return True if connected (in any state >= Connected)
	 */
	UFUNCTION(BlueprintPure, Category = "Connection")
	bool IsConnected() const { return ConnectionState >= EMGConnectionState::Connected; }

	/**
	 * @brief Gets information about the local player.
	 * @return Network player data for the local player
	 */
	UFUNCTION(BlueprintPure, Category = "Connection")
	FMGNetPlayer GetLocalPlayer() const { return LocalPlayer; }

	// ========================================================================
	// MATCHMAKING
	// Automated opponent finding. These functions work with the matchmaking
	// service to find appropriate opponents based on skill and preferences.
	// ========================================================================

	/**
	 * @brief Starts quick match matchmaking with an optional track preference.
	 * @param PreferredTrack Optional track ID to prefer (NAME_None for any)
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void StartQuickMatch(FName PreferredTrack = NAME_None);

	/**
	 * @brief Starts ranked matchmaking for competitive play.
	 * @note Uses stricter skill matching and affects player rating.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void StartRankedMatch();

	/**
	 * @brief Cancels an active matchmaking search.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void CancelMatchmaking();

	/**
	 * @brief Checks if currently searching for a match.
	 * @return True if matchmaking is in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	bool IsMatchmaking() const { return bIsMatchmaking; }

	// ========================================================================
	// SESSION
	// Session lifecycle management. Sessions are the game server instances
	// where races take place.
	// ========================================================================

	/**
	 * @brief Creates a new session with the specified settings.
	 * @param Settings Configuration for the new session
	 * @note Local player becomes the host.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(const FMGLobbySettings& Settings);

	/**
	 * @brief Joins an existing session by its ID.
	 * @param SessionID Unique identifier of the session
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSession(const FString& SessionID);

	/**
	 * @brief Joins a session using a short invite code.
	 * @param InviteCode Short alphanumeric code (e.g., "ABC123")
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinByInviteCode(const FString& InviteCode);

	/**
	 * @brief Leaves the current session.
	 * @note If host, session may be migrated or closed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void LeaveSession();

	/**
	 * @brief Gets information about the current session.
	 * @return Session info, or empty struct if not in session
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FMGSessionInfo GetCurrentSession() const { return CurrentSession; }

	/**
	 * @brief Checks if currently in a session.
	 * @return True if in a session (lobby or race)
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsInSession() const { return bInSession; }

	/**
	 * @brief Gets the invite code for the current session.
	 * @return Short alphanumeric code for sharing
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FString GetInviteCode() const;

	/**
	 * @brief Searches for available sessions matching filters.
	 * @param TrackFilter Optional track to filter by (NAME_None for all)
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void SearchSessions(FName TrackFilter = NAME_None);

	/**
	 * @brief Gets results from the last session search.
	 * @return Array of discovered sessions
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	TArray<FMGSessionInfo> GetSessionSearchResults() const { return SessionSearchResults; }

	// ========================================================================
	// LOBBY
	// Pre-race lobby management. Players gather here to configure settings,
	// select vehicles, and ready up before the race starts.
	// ========================================================================

	/**
	 * @brief Gets all players currently in the lobby.
	 * @return Array of player data
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	TArray<FMGNetPlayer> GetLobbyPlayers() const { return LobbyPlayers; }

	/**
	 * @brief Gets the current lobby settings.
	 * @return Lobby configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	FMGLobbySettings GetLobbySettings() const { return LobbySettings; }

	/**
	 * @brief Sets the local player's ready state.
	 * @param bReady True to mark ready, false to unready
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bReady);

	/**
	 * @brief Checks if the local player is ready.
	 * @return True if marked as ready
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsLocalPlayerReady() const { return LocalPlayer.bIsReady; }

	/**
	 * @brief Sets the local player's vehicle selection.
	 * @param VehicleID ID of the vehicle to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetSelectedVehicle(FName VehicleID);

	/**
	 * @brief Updates lobby settings (host only).
	 * @param NewSettings New configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdateLobbySettings(const FMGLobbySettings& NewSettings);

	/**
	 * @brief Removes a player from the session (host only).
	 * @param PlayerID ID of the player to kick
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void KickPlayer(const FString& PlayerID);

	/**
	 * @brief Starts the race countdown (host only).
	 * @note Requires all players to be ready (or use force start).
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartRace();

	/**
	 * @brief Checks if the local player is the session host.
	 * @return True if hosting
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsHost() const { return LocalPlayer.bIsHost; }

	/**
	 * @brief Checks if the race can start (enough players, all ready).
	 * @return True if race start conditions are met
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool CanStartRace() const;

	// ========================================================================
	// RACE
	// In-race state and reporting. These functions handle race timing,
	// position tracking, and result submission.
	// ========================================================================

	/**
	 * @brief Gets the synchronized race elapsed time.
	 * @return Seconds since race start
	 */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetRaceTime() const { return RaceTime; }

	/**
	 * @brief Gets the current countdown time remaining.
	 * @return Seconds remaining in countdown (0 when racing)
	 */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetCountdownTime() const { return CountdownTime; }

	/**
	 * @brief Checks if a race is currently in progress.
	 * @return True if actively racing
	 */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool IsRaceInProgress() const { return ConnectionState == EMGConnectionState::InRace; }

	/**
	 * @brief Reports that the local player has finished the race.
	 * @param FinalTime Total race time in seconds
	 * @param BestLapTime Fastest lap time achieved
	 */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void ReportRaceFinish(float FinalTime, float BestLapTime);

	/**
	 * @brief Reports a completed lap time.
	 * @param LapNumber Which lap was completed (1-based)
	 * @param LapTime Time for that lap in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void ReportLapTime(int32 LapNumber, float LapTime);

	/**
	 * @brief Gets the race results after the race ends.
	 * @return Array of results for all participants
	 */
	UFUNCTION(BlueprintPure, Category = "Race")
	TArray<FMGRaceResult> GetRaceResults() const { return RaceResults; }

	// ========================================================================
	// NETWORK
	// Low-level network diagnostics and timing information.
	// ========================================================================

	/**
	 * @brief Gets the current network latency to the host.
	 * @return Ping in milliseconds
	 */
	UFUNCTION(BlueprintPure, Category = "Network")
	int32 GetPing() const { return CurrentPing; }

	/**
	 * @brief Gets the time offset from the server for synchronization.
	 * @return Offset in seconds to add to local time for server time
	 */
	UFUNCTION(BlueprintPure, Category = "Network")
	float GetServerTimeOffset() const { return ServerTimeOffset; }

protected:
	// ========================================================================
	// INTERNAL STATE
	// Protected members accessible to derived classes.
	// ========================================================================

	/// Current network connection state
	EMGConnectionState ConnectionState = EMGConnectionState::Disconnected;

	/// Local player network data
	FMGNetPlayer LocalPlayer;

	/// Whether currently in a session
	bool bInSession = false;

	/// Current session information
	FMGSessionInfo CurrentSession;

	/// All players in the current lobby
	TArray<FMGNetPlayer> LobbyPlayers;

	/// Current lobby configuration
	FMGLobbySettings LobbySettings;

	/// Whether matchmaking is active
	bool bIsMatchmaking = false;

	/// Results from session search
	TArray<FMGSessionInfo> SessionSearchResults;

	/// Synchronized race elapsed time
	float RaceTime = 0.0f;

	/// Countdown time remaining before race start
	float CountdownTime = 0.0f;

	/// Race results after completion
	TArray<FMGRaceResult> RaceResults;

	/// Current network latency
	int32 CurrentPing = 0;

	/// Time offset for server synchronization
	float ServerTimeOffset = 0.0f;

	// ========================================================================
	// INTERNAL FUNCTIONS
	// Implementation details for network operations.
	// ========================================================================

	/**
	 * @brief Updates the connection state and broadcasts the change.
	 * @param NewState The new connection state
	 */
	void SetConnectionState(EMGConnectionState NewState);

	/**
	 * @brief Handles matchmaking progress updates from the backend.
	 * @param PlayersFound Current number of matched players
	 * @param PlayersNeeded Total players needed to start
	 */
	void OnMatchmakingUpdate(int32 PlayersFound, int32 PlayersNeeded);

	/**
	 * @brief Handles notification that a suitable session was found.
	 * @param Session The discovered session
	 */
	void OnSessionFound(const FMGSessionInfo& Session);

	/**
	 * @brief Synchronizes local time with the server.
	 */
	void SyncServerTime();

	/**
	 * @brief Generates a short invite code from a session ID.
	 * @param SessionID The full session identifier
	 * @return Short alphanumeric code
	 */
	FString GenerateInviteCode(const FString& SessionID) const;

	/**
	 * @brief Decodes an invite code back to a session ID.
	 * @param Code The short invite code
	 * @return Full session identifier
	 */
	FString DecodeInviteCode(const FString& Code) const;
};

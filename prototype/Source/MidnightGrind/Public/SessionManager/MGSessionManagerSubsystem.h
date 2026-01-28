// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGSessionManagerSubsystem.h - Low-Level Session Lifecycle Management
 * ============================================================================
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 *
 * WHAT IS THIS FILE?
 * ------------------
 * This subsystem handles the technical details of multiplayer sessions:
 * creating game servers, connecting players, handling disconnections,
 * and managing host migration. It's the "plumbing" that makes multiplayer work.
 *
 * While MGSessionSubsystem provides a friendly API for UI developers,
 * this subsystem deals with the nitty-gritty of network connections.
 *
 * WHEN WOULD YOU USE THIS?
 * ------------------------
 * - Implementing custom server browser features
 * - Handling network errors and reconnection
 * - Building admin/moderation tools (kick, ban, etc.)
 * - Debugging connection issues
 * - Implementing host migration (when host disconnects)
 *
 * KEY CONCEPTS EXPLAINED:
 * -----------------------
 *
 * 1. SESSION STATES (EMGSessionState)
 *    A session moves through states like a state machine:
 *
 *    None -> Creating -> InLobby -> Starting -> InProgress -> Ending -> PostGame
 *      ^                                                                   |
 *      +-------------------------------------------------------------------+
 *
 *    - None: No active session
 *    - Creating: Server is being set up
 *    - Joining: Client is connecting to a server
 *    - InLobby: Players are in the pre-game waiting room
 *    - Starting: Countdown to race start
 *    - InProgress: Race is happening
 *    - Ending: Race finished, calculating results
 *    - PostGame: Showing results, before returning to lobby
 *    - Disconnected: Connection was lost
 *    - Error: Something went wrong
 *
 * 2. SESSION TYPES (EMGSessionType)
 *    Different kinds of multiplayer experiences:
 *
 *    - Singleplayer: Solo play (no network)
 *    - LocalMultiplayer: Split-screen, same console
 *    - OnlinePrivate: Invite-only online game
 *    - OnlinePublic: Anyone can join via server browser
 *    - Ranked: Competitive mode affecting skill ratings
 *    - Tournament: Special bracket-based events
 *    - FreeroamPublic/Private: Open-world driving (no races)
 *
 * 3. HOST MIGRATION
 *    What happens when the host disconnects:
 *
 *    - In peer-to-peer games, one player is the "host"
 *    - If host disconnects, the session would normally end
 *    - Host migration picks a new host automatically
 *    - EMGMigrationState tracks this process
 *
 *    Flow:
 *    HostMigrationStarted -> WaitingForNewHost -> NewHostSelected -> MigrationComplete
 *
 * 4. CONNECTION ATTEMPTS (FMGConnectionAttempt)
 *    Handles retrying failed connections:
 *
 *    - Tracks how many times we've tried to connect
 *    - Has a timeout to prevent waiting forever
 *    - Records error messages for debugging
 *    - Default: 3 attempts, 30 second timeout each
 *
 * 5. DISCONNECT REASONS (EMGDisconnectReason)
 *    Why a player left - important for UI feedback:
 *
 *    - PlayerQuit: They chose to leave
 *    - HostClosed: Host ended the session
 *    - Kicked: Removed by host
 *    - Banned: Removed and blocked from rejoining
 *    - Timeout: Connection timed out
 *    - NetworkError: Internet problems
 *    - etc.
 *
 * HOW THIS FITS IN THE ARCHITECTURE:
 * ----------------------------------
 *
 *   [MGSessionSubsystem] - Friendly API for UI
 *            |
 *            v
 *   [MGMatchmakingSubsystem] - Finding opponents
 *            |
 *            v
 *   [MGSessionManagerSubsystem] <-- THIS FILE: Network session management
 *            |
 *            v
 *   [Unreal Online Subsystem] - Platform-specific networking (Steam, Xbox, etc.)
 *            |
 *            v
 *   [Actual Network Layer] - Sockets, packets, etc.
 *
 * KEY STRUCTURES:
 * ---------------
 *
 * FMGSessionInfo - Complete info about a session (for server browser)
 *   - SessionID, HostName, CurrentPlayers, MaxPlayers, Ping, Region, etc.
 *
 * FMGSessionPlayer - Info about a player in the session
 *   - PlayerID, DisplayName, IsHost, IsReady, Ping, Team, etc.
 *
 * FMGSessionSettings - Configuration when creating a session
 *   - SessionName, MaxPlayers, Password, CrossPlay, etc.
 *
 * FMGSessionSearchFilters - Filters for server browser
 *   - GameMode, MaxPing, HideFull, Region, etc.
 *
 * FMGSessionInvite - An invite from another player
 *   - SenderName, SessionInfo, ExpiryTime, etc.
 *
 * IMPORTANT DELEGATES (Events):
 * -----------------------------
 *
 * OnSessionStateChanged - Called whenever session state changes
 *   Use for: Updating UI, showing/hiding screens
 *
 * OnPlayerJoined / OnPlayerLeft - Player roster changes
 *   Use for: Updating lobby player list
 *
 * OnSessionJoinFailed - Connection failed
 *   Use for: Showing error message, offering retry
 *
 * OnHostMigrationStateChanged - Host migration progress
 *   Use for: Showing "Finding new host..." message
 *
 * OnInviteReceived - Someone invited you to their game
 *   Use for: Showing invite notification popup
 *
 * EXAMPLE USAGE:
 * --------------
 *
 * // Create a session
 * FMGSessionSettings Settings;
 * Settings.SessionName = TEXT("My Awesome Race");
 * Settings.MaxPlayers = 8;
 * Settings.bPrivate = true;
 * GetGameInstance()->GetSubsystem<UMGSessionManagerSubsystem>()->CreateSession(Settings);
 *
 * // Listen for players joining
 * SessionManager->OnPlayerJoined.AddDynamic(this, &UMyWidget::OnPlayerJoined);
 *
 * // Kick a player (host only)
 * SessionManager->KickPlayer(PlayerID, TEXT("Breaking rules"));
 *
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSessionManagerSubsystem.generated.h"

/**
 * @brief Represents the current lifecycle state of a multiplayer session.
 *
 * IMPORTANT: Sessions are like state machines - they progress through these
 * states in a predictable order. Understanding this flow is crucial for
 * implementing multiplayer UI and game logic.
 *
 * State Transition Diagram:
 *
 *   [None] --create--> [Creating] --success--> [InLobby]
 *     |                    |                       |
 *     |                    v (fail)                v (start race)
 *     |                 [Error]              [Starting]
 *     |                                           |
 *     |                                           v
 *   [None] <--leave-- [PostGame] <-- [Ending] <-- [InProgress]
 *     ^                                           |
 *     +------- [Disconnected] <----(network fail)-+
 */
UENUM(BlueprintType)
enum class EMGSessionState : uint8
{
	/// No active session - player is in menus or single player mode
	None,
	/// Session is being created on the server (show loading spinner)
	Creating,
	/// Client is connecting to an existing session (show "Connecting...")
	Joining,
	/// Players are in the pre-race lobby (can change settings, vehicles)
	InLobby,
	/// Race countdown has begun, loading the track
	Starting,
	/// Race is actively happening
	InProgress,
	/// Race finished, calculating and syncing results
	Ending,
	/// Showing post-race results screen
	PostGame,
	/// Connection was lost unexpectedly (show reconnection UI)
	Disconnected,
	/// Something went wrong (show error message to user)
	Error
};

/**
 * @brief Defines the type/mode of a multiplayer session.
 *
 * Each type has different rules for:
 * - How players can join (public, invite-only, etc.)
 * - Whether skill ratings are affected
 * - Matchmaking behavior
 * - Server browser visibility
 */
UENUM(BlueprintType)
enum class EMGSessionType : uint8
{
	/// Solo play - no networking involved
	Singleplayer,
	/// Split-screen on same console/PC - shared screen, no internet needed
	LocalMultiplayer,
	/// Online game that requires invite or direct join link
	OnlinePrivate,
	/// Online game visible in server browser, anyone can join
	OnlinePublic,
	/// Competitive mode that affects your skill rating (MMR)
	Ranked,
	/// Special bracket-based competition with prizes/rewards
	Tournament,
	/// Open-world driving with strangers (no structured races)
	FreeroamPublic,
	/// Open-world driving with friends only
	FreeroamPrivate
};

/**
 * @brief Explains why a player left or was disconnected from a session.
 *
 * IMPORTANT for UI: Show different messages to the user based on reason:
 * - PlayerQuit: "You have left the session"
 * - Kicked: "You were removed by the host"
 * - NetworkError: "Connection lost. Check your internet."
 * - VersionMismatch: "Please update your game"
 */
UENUM(BlueprintType)
enum class EMGDisconnectReason : uint8
{
	/// Reason could not be determined
	Unknown,
	/// Player voluntarily left the session
	PlayerQuit,
	/// Session host ended the session for everyone
	HostClosed,
	/// Player was removed by the host
	Kicked,
	/// Player was banned and cannot rejoin this session
	Banned,
	/// Connection timed out (no response for too long)
	Timeout,
	/// General network failure (internet issues)
	NetworkError,
	/// Dedicated server is shutting down
	ServerShutdown,
	/// Session reached max players while joining
	SessionFull,
	/// Player's game version doesn't match server version
	VersionMismatch,
	/// Failed to verify player identity with online services
	AuthenticationFailed,
	/// Game servers are under maintenance
	MaintenanceMode
};

/**
 * @brief Tracks the progress of host migration when the original host disconnects.
 *
 * HOST MIGRATION EXPLAINED:
 * In peer-to-peer games, one player acts as the "host" (server).
 * If that player disconnects, the game would normally end for everyone.
 * Host migration solves this by automatically selecting a new host.
 *
 * Migration Flow:
 * 1. Host disconnects unexpectedly
 * 2. HostMigrationStarted - System detects host is gone
 * 3. WaitingForNewHost - Finding best candidate (lowest ping, etc.)
 * 4. NewHostSelected - Candidate chosen, setting up
 * 5. MigrationComplete - New host is active, game continues!
 *    OR MigrationFailed - Could not recover, session ends
 *
 * UI TIP: Show a "Finding new host..." message during migration
 */
UENUM(BlueprintType)
enum class EMGMigrationState : uint8
{
	/// No migration in progress
	None,
	/// Host disconnection detected, beginning migration
	HostMigrationStarted,
	/// Looking for the best player to become new host
	WaitingForNewHost,
	/// New host has been selected, transferring authority
	NewHostSelected,
	/// Migration successful! Game continues normally
	MigrationComplete,
	/// Migration failed - session will end
	MigrationFailed
};

/**
 * @brief Complete information about a multiplayer session.
 *
 * This struct is used in two main contexts:
 * 1. SERVER BROWSER: Displaying available sessions to join
 * 2. CURRENT SESSION: Tracking the session you're currently in
 *
 * Think of it as a "business card" for a game session - all the info
 * someone would need to decide if they want to join.
 */
USTRUCT(BlueprintType)
struct FMGSessionInfo
{
	GENERATED_BODY()

	/// Unique identifier for this session (used to join it)
	/// Format: Usually a GUID like "550e8400-e29b-41d4-a716-446655440000"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	/// Human-readable name set by the host (e.g., "Pro Racers Only!")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionName;

	/// What kind of session this is (public, ranked, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionType Type = EMGSessionType::Singleplayer;

	/// Current lifecycle state (lobby, in-race, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionState State = EMGSessionState::None;

	/// Platform ID of the host player (for friend checks, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostPlayerID;

	/// Display name of the host (shown in server browser)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostDisplayName;

	/// How many players are currently in the session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayers = 0;

	/// Maximum players allowed (0 means use default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/// If true, session won't appear in public server browser
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrivate = false;

	/// If true, players can join even after race has started
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJoinInProgress = true;

	/// Name of the current map/track
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapName;

	/// Current game mode (Circuit, Sprint, Drift, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameMode = NAME_None;

	/// Network latency to this session in milliseconds (lower is better)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	/// Geographic region code (e.g., "US-East", "EU-West")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Region;

	/// Game version (for compatibility checks)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GameVersion;

	/// When this session was created
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	/// Flexible key-value storage for custom session data
	/// Example: "LapCount" -> "5", "NightMode" -> "true"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomData;
};

/**
 * @brief Information about a player currently in a session.
 *
 * Used to display the player roster in the lobby UI, show player cards,
 * and make gameplay decisions (teams, ready state, etc.).
 *
 * UI TIPS:
 * - Show a crown icon for bIsHost
 * - Show ping bars based on Ping value (green <50, yellow <100, red >100)
 * - Gray out players who aren't ready yet
 * - Show platform icon based on Platform field
 */
USTRUCT(BlueprintType)
struct FMGSessionPlayer
{
	GENERATED_BODY()

	/// Unique platform identifier (used for bans, invites, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	/// Human-readable name (shown in UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	/// True if this player is the session host (has kick/ban powers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHost = false;

	/// True if this represents the local player (you!)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocal = false;

	/// Network latency in milliseconds (only meaningful for remote players)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	/// Team assignment for team modes (0-indexed, or -1 for FFA)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamIndex = 0;

	/// True if player has clicked "Ready" in the lobby
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = false;

	/// True if player is watching but not racing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSpectator = false;

	/// Platform identifier (e.g., "Steam", "Xbox", "PlayStation", "Epic")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Platform;

	/// When this player joined the session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime JoinTime;

	/// Flexible storage for player-specific data
	/// Example: "SelectedVehicle" -> "Nissan_GTR", "Livery" -> "Racing_01"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> PlayerData;
};

/**
 * @brief Configuration options when creating a new session.
 *
 * The host fills out these settings before creating a session.
 * Many of these can be changed later with UpdateSession().
 *
 * COMMON PRESETS:
 *
 * Private Friends Race:
 *   Type = OnlinePrivate, bPrivate = true, bAllowInvites = true
 *
 * Public Server Browser Game:
 *   Type = OnlinePublic, bShouldAdvertise = true
 *
 * Ranked Competitive:
 *   Type = Ranked, bEnableHostMigration = true, bUseDedicatedServer = true
 */
USTRUCT(BlueprintType)
struct FMGSessionSettings
{
	GENERATED_BODY()

	/// Display name for the session (shown in server browser)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionName;

	/// Type of session (affects visibility, ranking, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionType Type = EMGSessionType::OnlinePrivate;

	/// Maximum racing participants (not including spectators)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/// Maximum spectators allowed (0 to disable spectating)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSpectators = 4;

	/// If true, session won't appear in public server browser
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrivate = false;

	/// Password required to join (empty string = no password)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;

	/// Can players join after the race has started?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowJoinInProgress = true;

	/// Can players send invites to friends?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowInvites = true;

	/// Should this session be listed in the public server browser?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldAdvertise = true;

	/// Use a dedicated server instead of player hosting?
	/// Dedicated = better performance, but costs money to run
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseDedicatedServer = false;

	/// If host disconnects, try to migrate to a new host?
	/// Should usually be true for player-hosted games
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableHostMigration = true;

	/// Allow players from different platforms (Xbox, PlayStation, PC)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCrossPlayEnabled = true;

	/// Preferred server region (e.g., "US-East", "EU-West")
	/// Empty string = automatic selection based on ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PreferredRegion;

	/// Starting map/track name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapName;

	/// Game mode identifier (Circuit, Sprint, Drift, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameMode = NAME_None;

	/// Flexible storage for custom game rules
	/// Example: "LapCount" -> "5", "Collisions" -> "Off"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomSettings;
};

/**
 * @brief Filters for the server browser search.
 *
 * Players use these filters to narrow down the list of available sessions.
 * All filters are AND conditions (must match ALL specified criteria).
 *
 * UI TIP: Provide sensible defaults, then let players customize:
 * - Default: HideFull=true, MaxPing=200, CrossPlay=user's preference
 */
USTRUCT(BlueprintType)
struct FMGSessionSearchFilters
{
	GENERATED_BODY()

	/// Only show sessions of this type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionType TypeFilter = EMGSessionType::OnlinePublic;

	/// Only show sessions with this game mode (NAME_None = any mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeFilter = NAME_None;

	/// Only show sessions on this map (empty = any map)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapFilter;

	/// Hide sessions that are already at max capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideFullSessions = true;

	/// Hide sessions where the race has already started
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideInProgressSessions = false;

	/// Only show sessions with ping below this threshold (ms)
	/// Higher values = more results but potentially laggy games
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPing = 200;

	/// Only show sessions in this region (empty = any region)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RegionFilter;

	/// Include sessions with players from other platforms?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCrossPlayEnabled = true;

	/// Maximum number of sessions to return (for performance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxResults = 50;
};

/**
 * @brief Represents an invitation from another player to join their session.
 *
 * When you receive an invite, show a notification popup with:
 * - Who invited you (SenderDisplayName)
 * - What kind of game (SessionInfo.GameMode, SessionInfo.MapName)
 * - Accept/Decline buttons
 *
 * IMPORTANT: Invites expire! Check ExpiryTime before displaying old invites.
 */
USTRUCT(BlueprintType)
struct FMGSessionInvite
{
	GENERATED_BODY()

	/// Unique identifier for this invite (for accept/decline calls)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InviteID;

	/// ID of the session being invited to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	/// Who sent this invite (their player ID)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderPlayerID;

	/// Human-readable name of who sent this (show in notification)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderDisplayName;

	/// Full info about the session (for displaying details)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGSessionInfo SessionInfo;

	/// When this invite was sent
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SentTime;

	/// When this invite expires (don't show expired invites!)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiryTime;
};

/**
 * @brief Tracks the state of a connection attempt with automatic retry logic.
 *
 * Network connections can fail for many reasons (temporary issues, etc.).
 * Instead of immediately failing, we retry a few times before giving up.
 *
 * UI TIP: Show "Connecting... (Attempt 2 of 3)" during retries
 */
USTRUCT(BlueprintType)
struct FMGConnectionAttempt
{
	GENERATED_BODY()

	/// Session we're trying to connect to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	/// Current attempt number (1-indexed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AttemptNumber = 0;

	/// Maximum attempts before giving up
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAttempts = 3;

	/// When this attempt started (for timeout calculation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttemptStartTime = 0.0f;

	/// How long to wait before timing out each attempt
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeoutSeconds = 30.0f;

	/// Error message from the last failed attempt (for debugging/display)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LastError;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================
//
// WHAT ARE DELEGATES?
// Delegates are Unreal's implementation of the "Observer" pattern.
// They let other code "subscribe" to events and get notified when they happen.
//
// HOW TO USE DELEGATES IN YOUR CODE:
//
// 1. Get the subsystem:
//    UMGSessionManagerSubsystem* SessionMgr = GetGameInstance()->GetSubsystem<UMGSessionManagerSubsystem>();
//
// 2. Subscribe to an event:
//    SessionMgr->OnPlayerJoined.AddDynamic(this, &UMyClass::HandlePlayerJoined);
//
// 3. Implement the handler (must match the delegate signature):
//    void UMyClass::HandlePlayerJoined(const FMGSessionPlayer& Player)
//    {
//        // Update your player list UI
//    }
//
// 4. Unsubscribe when done (usually in destructor or EndPlay):
//    SessionMgr->OnPlayerJoined.RemoveDynamic(this, &UMyClass::HandlePlayerJoined);
//
// ============================================================================

/** @brief Fires when the session state changes (lobby, in-game, etc.) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStateChanged, EMGSessionState, NewState);

/** @brief Fires when a new session is successfully created (you are the host) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, const FMGSessionInfo&, SessionInfo);

/** @brief Fires when successfully joined someone else's session */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, const FMGSessionInfo&, SessionInfo);

/** @brief Fires when joining a session fails (show error to user) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionJoinFailed, const FString&, SessionID, const FString&, ErrorMessage);

/** @brief Fires when the session ends or you disconnect */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionEnded, EMGDisconnectReason, Reason);

/** @brief Fires when session info changes (map, settings, etc.) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionUpdated, const FMGSessionInfo&, SessionInfo);

/** @brief Fires when a new player joins (update lobby UI) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, const FMGSessionPlayer&, Player);

/** @brief Fires when a player leaves (update lobby UI, show reason) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeft, const FString&, PlayerID, EMGDisconnectReason, Reason);

/** @brief Fires when a player's data changes (ready state, vehicle, etc.) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDataChanged, const FString&, PlayerID, const FMGSessionPlayer&, PlayerData);

/** @brief Fires when server browser search completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchComplete, const TArray<FMGSessionInfo>&, Sessions);

/** @brief Fires when server browser search fails */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchFailed, const FString&, ErrorMessage);

/** @brief Fires when you receive an invite from another player */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteReceived, const FMGSessionInvite&, Invite);

/** @brief Fires when your invite to another player succeeds or fails */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInviteSent, const FString&, RecipientID, bool, bSuccess);

/** @brief Fires when host migration state changes (show "Finding new host...") */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostMigrationStateChanged, EMGMigrationState, MigrationState);

/** @brief Fires when a new host is selected after migration */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewHostSelected, const FString&, NewHostPlayerID);

/** @brief Fires during connection retries (show "Attempt 2 of 3...") */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConnectionAttempt, int32, AttemptNumber, int32, MaxAttempts);

/**
 * @brief Low-level session management subsystem for multiplayer networking.
 *
 * This subsystem handles the technical details of multiplayer sessions:
 * creating servers, managing connections, handling disconnections,
 * and implementing host migration.
 *
 * RELATIONSHIP TO OTHER SUBSYSTEMS:
 * - MGSessionSubsystem: High-level, simplified API for UI developers
 * - MGMatchmakingSubsystem: Finding opponents automatically
 * - MGSessionManagerSubsystem (THIS): Low-level network session management
 *
 * WHO SHOULD USE THIS:
 * - Systems programmers implementing network features
 * - Developers building server browser or admin tools
 * - Anyone needing fine-grained control over sessions
 *
 * For most gameplay UI, prefer MGSessionSubsystem instead.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSessionManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================================================
	// LIFECYCLE
	// Called automatically by Unreal Engine - don't call these manually!
	// ========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// SESSION CREATION (Host-side functions)
	// Use these when creating a new session that others will join.
	// ========================================================================
	/**
	 * @brief Creates a new multiplayer session with the given settings.
	 * @param Settings Configuration for the new session
	 * @return True if creation started (check OnSessionCreated for completion)
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	bool CreateSession(const FMGSessionSettings& Settings);

	/**
	 * @brief Updates the current session's settings (host only).
	 * @param Settings New settings to apply
	 * @return True if update was accepted
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	bool UpdateSession(const FMGSessionSettings& Settings);

	/**
	 * @brief Destroys the current session, disconnecting all players.
	 * Host only! This ends the game for everyone.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void DestroySession();

	/**
	 * @brief Checks if the local player is the session host.
	 * @return True if we created this session (have admin powers)
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Host")
	bool IsSessionHost() const;

	// ========================================================================
	// SESSION JOINING (Client-side functions)
	// Use these when joining a session created by someone else.
	// ========================================================================
	/**
	 * @brief Joins a session by its unique ID.
	 * @param SessionID The session to join (from server browser or invite)
	 * @param Password Password if required (empty string if none)
	 * @return True if join started (check OnSessionJoined/OnSessionJoinFailed)
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Join")
	bool JoinSession(const FString& SessionID, const FString& Password = TEXT(""));

	/**
	 * @brief Joins a session using full session info (from server browser).
	 * @param SessionInfo Complete session info from GetSearchResults()
	 * @param Password Password if required
	 * @return True if join started
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Join")
	bool JoinSessionByInfo(const FMGSessionInfo& SessionInfo, const FString& Password = TEXT(""));

	/**
	 * @brief Cancels an in-progress join attempt.
	 * Safe to call even if not currently joining.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Join")
	void CancelJoin();

	/**
	 * @brief Leaves the current session and returns to menu state.
	 * If you're the host, this may trigger host migration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Join")
	void LeaveSession();

	// ========================================================================
	// SESSION STATE (Query functions)
	// Read-only functions to check current session status.
	// ========================================================================
	/** @brief Checks if we're currently in any session (lobby or game). */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	bool IsInSession() const;

	/** @brief Gets the current session lifecycle state. */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	EMGSessionState GetSessionState() const { return CurrentState; }

	/** @brief Gets complete info about the current session. */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	FMGSessionInfo GetCurrentSession() const { return CurrentSession; }

	/** @brief Gets all players in the current session (for lobby UI). */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	TArray<FMGSessionPlayer> GetSessionPlayers() const { return SessionPlayers; }

	/** @brief Gets info about the local player. */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	FMGSessionPlayer GetLocalPlayer() const;

	/** @brief Gets info about a specific player by ID. */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	FMGSessionPlayer GetPlayer(const FString& PlayerID) const;

	/** @brief Gets the total number of players in the session. */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	int32 GetPlayerCount() const { return SessionPlayers.Num(); }

	// ========================================================================
	// SESSION MANAGEMENT (Host-only admin functions)
	// These require IsSessionHost() == true.
	// ========================================================================
	/**
	 * @brief Removes a player from the session (host only).
	 * @param PlayerID Player to kick
	 * @param Reason Optional reason to show the kicked player
	 * @return True if kick succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool KickPlayer(const FString& PlayerID, const FString& Reason = TEXT(""));

	/**
	 * @brief Removes a player and prevents them from rejoining (host only).
	 * @param PlayerID Player to ban
	 * @param Reason Optional reason to show
	 * @return True if ban succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool BanPlayer(const FString& PlayerID, const FString& Reason = TEXT(""));

	/**
	 * @brief Locks/unlocks the session to prevent new joins (host only).
	 * @param bLocked True to lock (no new players), false to unlock
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool SetSessionLocked(bool bLocked);

	/**
	 * @brief Transfers host privileges to another player (host only).
	 * @param NewHostPlayerID Player to become the new host
	 * @return True if transfer succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool TransferHost(const FString& NewHostPlayerID);

	/**
	 * @brief Starts the game/race (host only, from lobby state).
	 * @return True if start succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool StartSession();

	/**
	 * @brief Ends the current game and returns to lobby (host only).
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Manage")
	bool EndSession();

	// ========================================================================
	// PLAYER DATA (Local player actions)
	// Things the local player can do/set about themselves.
	// ========================================================================
	/** @brief Sets whether the local player is ready to start. */
	UFUNCTION(BlueprintCallable, Category = "Session|Player")
	void SetLocalPlayerReady(bool bReady);

	/** @brief Sets the local player's team (for team-based modes). */
	UFUNCTION(BlueprintCallable, Category = "Session|Player")
	void SetLocalPlayerTeam(int32 TeamIndex);

	/** @brief Sets whether the local player is spectating vs racing. */
	UFUNCTION(BlueprintCallable, Category = "Session|Player")
	void SetLocalPlayerSpectator(bool bSpectator);

	/**
	 * @brief Sets custom data for the local player.
	 * @param Key Data key (e.g., "SelectedVehicle", "Livery")
	 * @param Value Data value
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Player")
	void SetLocalPlayerData(FName Key, const FString& Value);

	/**
	 * @brief Gets custom data for the local player.
	 * @param Key Data key to retrieve
	 * @return Value, or empty string if not set
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Player")
	FString GetLocalPlayerData(FName Key) const;

	/** @brief Checks if all players have marked themselves ready. */
	UFUNCTION(BlueprintPure, Category = "Session|Player")
	bool AreAllPlayersReady() const;

	// ========================================================================
	// SESSION SEARCH (Server Browser)
	// Functions for discovering joinable sessions.
	// ========================================================================
	/**
	 * @brief Starts searching for sessions matching the filters.
	 * Results arrive via OnSessionSearchComplete delegate.
	 * @param Filters Criteria for filtering sessions
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Search")
	void SearchSessions(const FMGSessionSearchFilters& Filters);

	/** @brief Cancels an in-progress search. */
	UFUNCTION(BlueprintCallable, Category = "Session|Search")
	void CancelSearch();

	/** @brief Checks if a search is currently in progress. */
	UFUNCTION(BlueprintPure, Category = "Session|Search")
	bool IsSearching() const { return bSearching; }

	/** @brief Gets the results from the most recent search. */
	UFUNCTION(BlueprintPure, Category = "Session|Search")
	TArray<FMGSessionInfo> GetSearchResults() const { return SearchResults; }

	/**
	 * @brief Refreshes info for a specific session (update ping, player count).
	 * @param SessionID Session to refresh
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Search")
	void RefreshSession(const FString& SessionID);

	// ========================================================================
	// INVITES
	// Sending and receiving game invitations.
	// ========================================================================
	/**
	 * @brief Sends an invite to another player to join your session.
	 * @param RecipientPlayerID Player to invite (must be in session first)
	 * @return True if invite was sent (check OnInviteSent for delivery status)
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	bool SendInvite(const FString& RecipientPlayerID);

	/**
	 * @brief Accepts a received invite and joins the session.
	 * @param Invite The invite to accept
	 * @return True if join started
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	bool AcceptInvite(const FMGSessionInvite& Invite);

	/**
	 * @brief Declines a received invite (no notification sent to sender).
	 * @param Invite The invite to decline
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void DeclineInvite(const FMGSessionInvite& Invite);

	/** @brief Gets all pending (unhandled) invites. */
	UFUNCTION(BlueprintPure, Category = "Session|Invite")
	TArray<FMGSessionInvite> GetPendingInvites() const { return PendingInvites; }

	/** @brief Removes invites that have passed their ExpiryTime. */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void ClearExpiredInvites();

	// ========================================================================
	// HOST MIGRATION
	// Handling when the original host disconnects.
	// ========================================================================
	/** @brief Gets the current host migration state. */
	UFUNCTION(BlueprintPure, Category = "Session|Migration")
	EMGMigrationState GetMigrationState() const { return MigrationState; }

	/** @brief Checks if host migration is currently happening. */
	UFUNCTION(BlueprintPure, Category = "Session|Migration")
	bool IsHostMigrationInProgress() const;

	/**
	 * @brief Manually triggers host migration (for testing/admin use).
	 * Usually this happens automatically when host disconnects.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Migration")
	void RequestHostMigration();

	// ========================================================================
	// NETWORK QUALITY
	// Monitoring connection quality for debugging and UI.
	// ========================================================================
	/** @brief Gets the average ping across all players in the session. */
	UFUNCTION(BlueprintPure, Category = "Session|Network")
	int32 GetAverageSessionPing() const;

	/**
	 * @brief Gets the current packet loss percentage (0.0 = perfect, 100.0 = complete loss).
	 * Values above 5% typically cause noticeable lag.
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Network")
	float GetPacketLoss() const { return PacketLossPercent; }

	/** @brief Updates network statistics (call periodically for fresh data). */
	UFUNCTION(BlueprintCallable, Category = "Session|Network")
	void UpdateNetworkStats();

	// ========================================================================
	// EVENT DELEGATES
	// Subscribe to these to react to session changes in your UI/game code.
	// See the delegate declarations above for usage examples.
	// ========================================================================
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
	// ========================================================================
	// INTERNAL IMPLEMENTATION
	// These functions handle the actual work - don't call from outside.
	// ========================================================================

	/** @brief Updates session state and fires the delegate. */
	void SetSessionState(EMGSessionState NewState);

	/** @brief Updates migration state and fires the delegate. */
	void SetMigrationState(EMGMigrationState NewState);

	/** @brief Called periodically to update session state. */
	void OnSessionTick();

	/** @brief Called when a join attempt times out. */
	void OnJoinTimeout();

	/** @brief Attempts to reconnect after a failed connection. */
	void RetryConnection();

	/** @brief Test helper: simulates successful session creation. */
	void SimulateSessionCreation(const FMGSessionSettings& Settings);

	/** @brief Test helper: simulates search results. */
	void SimulateSessionSearch();

	/** @brief Test helper: simulates host migration. */
	void SimulateHostMigration();

	/** @brief Creates an FMGSessionPlayer representing the local player. */
	FMGSessionPlayer CreateLocalPlayer() const;

	/** @brief Adds the local player to the SessionPlayers array. */
	void AddLocalPlayerToSession();

	// ========================================================================
	// INTERNAL STATE
	// Private data used by the subsystem - not accessible from outside.
	// ========================================================================

	/// Current lifecycle state of the session
	UPROPERTY()
	EMGSessionState CurrentState = EMGSessionState::None;

	/// Info about the current session
	UPROPERTY()
	FMGSessionInfo CurrentSession;

	/// All players in the current session
	UPROPERTY()
	TArray<FMGSessionPlayer> SessionPlayers;

	/// Results from the most recent session search
	UPROPERTY()
	TArray<FMGSessionInfo> SearchResults;

	/// Invites received but not yet accepted/declined
	UPROPERTY()
	TArray<FMGSessionInvite> PendingInvites;

	/// Current host migration state
	UPROPERTY()
	EMGMigrationState MigrationState = EMGMigrationState::None;

	/// Tracks retry attempts when connecting
	UPROPERTY()
	FMGConnectionAttempt CurrentConnectionAttempt;

	/// True if a session search is in progress
	UPROPERTY()
	bool bSearching = false;

	/// Current packet loss percentage (for connection quality display)
	UPROPERTY()
	float PacketLossPercent = 0.0f;

	/// Players banned from this session (if we're host)
	UPROPERTY()
	TArray<FString> BannedPlayerIDs;

	/// Timer for periodic session state updates
	FTimerHandle SessionTickHandle;

	/// Timer for join attempt timeouts
	FTimerHandle JoinTimeoutHandle;

	/// Timer for search timeouts
	FTimerHandle SearchTimeoutHandle;
};

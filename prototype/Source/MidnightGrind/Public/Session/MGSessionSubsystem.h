// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGSessionSubsystem.h - Core Session Management for Multiplayer
 * ============================================================================
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 *
 * WHAT IS THIS FILE?
 * ------------------
 * This is the main entry point for all multiplayer features in Midnight Grind.
 * It provides a simplified, high-level interface for creating/joining games,
 * finding opponents, and playing with friends. Think of it as the "front door"
 * to all online functionality.
 *
 * If you're building multiplayer UI (menus, lobbies, etc.), this is likely
 * the subsystem you'll interact with most often.
 *
 * KEY CONCEPTS EXPLAINED:
 * -----------------------
 *
 * 1. SESSION vs LOBBY vs MATCH
 *    - SESSION: The entire multiplayer game instance (server + all players)
 *    - LOBBY: The pre-game waiting room where players gather
 *    - MATCH: The actual race/gameplay portion
 *    - Flow: Create Session -> Enter Lobby -> Start Match -> Return to Lobby
 *
 * 2. HOST vs CLIENT
 *    - HOST: The player who created the session (has admin powers)
 *    - CLIENT: Players who joined the session
 *    - Host can kick players, change settings, start races
 *    - In dedicated server games, no player is the "host"
 *
 * 3. PARTY SYSTEM
 *    - Parties are groups of friends who want to play together
 *    - Party members stick together through matchmaking
 *    - Example: 3 friends form a party, then queue together for races
 *    - The party persists even after individual races end
 *
 * 4. MATCHMAKING vs SERVER BROWSER
 *    - MATCHMAKING: Automatic - system finds suitable opponents for you
 *    - SERVER BROWSER: Manual - you pick a specific game to join
 *    - This subsystem supports both approaches
 *
 * 5. PRIVACY LEVELS (EMGLobbyPrivacy)
 *    - Public: Anyone can join (shows in server browser)
 *    - FriendsOnly: Only your platform friends can join
 *    - InviteOnly: Requires an explicit invite
 *    - Closed: No one can join (race in progress)
 *
 * HOW THIS FITS IN THE ARCHITECTURE:
 * ----------------------------------
 *
 *   [UI Layer - Menus, HUD]
 *            |
 *            | (calls CreateSession, StartMatchmaking, etc.)
 *            v
 *   [MGSessionSubsystem] <-- THIS FILE: Simplified API for multiplayer
 *            |
 *            | (delegates to specialized subsystems)
 *            v
 *   +-------------------+-------------------+
 *   |                   |                   |
 *   v                   v                   v
 * [MGMatchmaking]   [MGParty]      [MGMultiplayer]
 * (find opponents)  (friend groups) (network layer)
 *
 * WHY USE THIS INSTEAD OF MGMatchmakingSubsystem?
 * -----------------------------------------------
 * - This subsystem is SIMPLER - fewer options, easier to use
 * - MGMatchmakingSubsystem offers FINE-GRAINED control
 * - Use MGSessionSubsystem for: Quick menus, basic flows
 * - Use MGMatchmakingSubsystem for: Advanced features, custom modes
 *
 * COMMON TASKS:
 * -------------
 *
 * Quick Play (find any match):
 *   GetGameInstance()->GetSubsystem<UMGSessionSubsystem>()->QuickPlay("Circuit");
 *
 * Create a private lobby:
 *   FMGSessionInfo Settings;
 *   Settings.Privacy = EMGLobbyPrivacy::InviteOnly;
 *   SessionSubsystem->CreateSession(Settings);
 *
 * Invite a friend:
 *   SessionSubsystem->InvitePlayer(FriendPlayerID);
 *
 * Listen for match found:
 *   SessionSubsystem->OnSessionFound.AddDynamic(this, &UMyWidget::HandleMatchFound);
 *
 * ============================================================================
 *
 * @file MGSessionSubsystem.h
 * @brief Core Session Management Subsystem for Midnight Grind Multiplayer
 *
 * This subsystem serves as the primary entry point for all online multiplayer functionality
 * in Midnight Grind. It coordinates session creation, matchmaking, lobby management, and
 * party systems to provide a seamless multiplayer experience.
 *
 * ## Architecture Overview
 * The Session Subsystem acts as a facade that unifies several multiplayer concepts:
 * - **Sessions**: The actual game server instance where races take place
 * - **Lobbies**: Pre-race waiting areas where players gather and configure settings
 * - **Matchmaking**: Automated system for finding appropriate opponents
 * - **Parties**: Persistent player groups that queue together
 *
 * ## Usage Flow
 * A typical multiplayer session follows this pattern:
 * 1. Player creates or joins a party (optional but recommended for group play)
 * 2. Player initiates matchmaking or creates/joins a session directly
 * 3. In the lobby, players select vehicles and ready up
 * 4. Host starts the race when all players are ready
 * 5. After the race, players return to lobby or leave session
 *
 * ## Integration Notes
 * - This subsystem is a GameInstanceSubsystem, persisting across level transitions
 * - All functions are Blueprint-callable for easy UI integration
 * - Events (delegates) provide reactive updates for UI binding
 *
 * @see UMGMultiplayerSubsystem For lower-level network connection management
 * @see UMGMatchmakingSubsystem For detailed matchmaking configuration
 * @see UMGPartySubsystem For advanced party features
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSessionSubsystem.generated.h"

/**
 * Session System - Online Multiplayer Management
 * - Session creation and discovery
 * - Matchmaking with skill-based matching
 * - Lobby management
 * - Friend invites and party system
 * - Quick join and playlist queuing
 */

// ============================================================================
// Session State Enumerations
// ============================================================================

/**
 * @brief Represents the current lifecycle state of a multiplayer session.
 *
 * Sessions progress through these states in a mostly linear fashion,
 * though certain transitions (like returning to lobby after a race) are valid.
 */
UENUM(BlueprintType)
enum class EMGSessionState : uint8
{
	/// No active session - player is in menus or single player
	None,
	/// Session is being created on the server
	Creating,
	/// Players are in the pre-race lobby, configuring settings
	InLobby,
	/// Countdown has begun, transitioning to race
	Starting,
	/// Race is actively in progress
	InProgress,
	/// Race has ended, calculating results
	Finishing,
	/// Showing results screen before returning to lobby
	PostRace
};

/**
 * @brief Represents the current state of the matchmaking search process.
 *
 * Used to update UI and prevent conflicting operations during matchmaking.
 */
UENUM(BlueprintType)
enum class EMGMatchmakingState : uint8
{
	/// Not searching - ready to start matchmaking
	Idle,
	/// Actively searching for opponents
	Searching,
	/// Match found, waiting for confirmation
	Found,
	/// Joining the discovered session
	Joining,
	/// Matchmaking failed (timeout, network error, etc.)
	Failed,
	/// User cancelled the search
	Cancelled
};

/**
 * @brief Controls who can join a multiplayer lobby.
 *
 * The host can change privacy at any time while in lobby,
 * allowing flexible control over session access.
 */
UENUM(BlueprintType)
enum class EMGLobbyPrivacy : uint8
{
	Public,          /// Anyone can join via server browser or matchmaking
	FriendsOnly,     /// Only platform friends can join directly
	InviteOnly,      /// Requires an explicit invite from a lobby member
	Closed           /// No new players can join (for mid-race lockout)
};

// ============================================================================
// Session Data Structures
// ============================================================================

/**
 * @brief Complete information about a multiplayer session.
 *
 * This structure contains all the data needed to display session info
 * in a server browser or to make matchmaking decisions.
 */
USTRUCT(BlueprintType)
struct FMGSessionInfo
{
	GENERATED_BODY()

	/// Unique identifier for this session (used for joining)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	/// Platform ID of the session host
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostPlayerID;

	/// Human-readable name of the host player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText HostDisplayName;

	/// Current lifecycle state of the session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionState State = EMGSessionState::None;

	/// Who can join this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLobbyPrivacy Privacy = EMGLobbyPrivacy::Public;

	/// ID of the currently selected track (NAME_None if not yet selected)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentTrackID;

	/// ID of the game mode (e.g., "Circuit", "Sprint", "Drift")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeID;

	/// Number of players currently in the session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayers = 0;

	/// Maximum players allowed in this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/// Average MMR of players in the session (for skill-based display)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AverageSkillRating = 1000;

	/// Geographic region of the session server
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Region;

	/// Latency to the session in milliseconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	/// Whether this is a ranked competitive session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanked = false;

	/// Whether the session has reached maximum capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFull = false;
};

/**
 * @brief Information about a player in the lobby.
 *
 * Contains everything needed to display player cards in the lobby UI
 * and to make gameplay decisions (team assignment, vehicle restrictions, etc.).
 */
USTRUCT(BlueprintType)
struct FMGLobbyPlayer
{
	GENERATED_BODY()

	/// Unique platform identifier for this player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	/// Human-readable display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Player's matchmaking rating (1000 = average)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillRating = 1000;

	/// ID of the vehicle the player has selected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SelectedVehicle;

	/// Performance Index of the selected vehicle (for class restrictions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VehiclePI = 0;

	/// Whether the player has marked themselves as ready to race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = false;

	/// Whether this player is the session host
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHost = false;

	/// Whether this player is on the local player's friends list
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFriend = false;

	/// Whether this player is in the same crew as local player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrewMember = false;

	/// Network latency in milliseconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	/// Team index for team-based modes (-1 = no team/FFA)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamIndex = -1;
};

/**
 * @brief Configuration options for automated matchmaking.
 *
 * These settings control how the matchmaking system searches for opponents,
 * balancing wait time against match quality.
 */
USTRUCT(BlueprintType)
struct FMGMatchmakingSettings
{
	GENERATED_BODY()

	/// Which playlist/queue to search in (e.g., "QuickRace", "Ranked", "Drift")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlaylistID;

	/// Only match with ranked sessions (affects rewards and ratings)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRankedOnly = false;

	/// Maximum acceptable ping in milliseconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPingMS = 150;

	/// Allow matching with players on different platforms
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCrossplay = true;

	/// Initial skill range for opponent search (+/- this value from player MMR)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillRange = 200;

	/// Maximum time to search before failing (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SearchTimeout = 120.0f;

	/// Gradually widen search criteria if no match found
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExpandSearchOverTime = true;

	/// Ordered list of preferred server regions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PreferredRegions;
};

// ============================================================================
// Party Data Structures
// ============================================================================

/**
 * @brief Information about a player in a party.
 *
 * Parties are persistent groups that queue together. Party members
 * stay together through matchmaking and multiple races.
 */
USTRUCT(BlueprintType)
struct FMGPartyMember
{
	GENERATED_BODY()

	/// Unique platform identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	/// Human-readable display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Whether this member is the party leader (controls queue decisions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLeader = false;

	/// Whether this member is currently in a game session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInGame = false;
};

/**
 * @brief Complete information about a player party.
 *
 * Parties provide a way for friends to group up before matchmaking,
 * ensuring they end up on the same team or in the same session.
 */
USTRUCT(BlueprintType)
struct FMGPartyInfo
{
	GENERATED_BODY()

	/// Unique identifier for this party
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PartyID;

	/// All members currently in the party
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPartyMember> Members;

	/// Maximum number of members allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSize = 4;

	/// Whether the party is open for friends to join without invite
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPublic = true;
};

// ============================================================================
// Event Delegates
// ============================================================================

/** @brief Fired when the session state changes (lobby, race, etc.) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSessionStateChanged, EMGSessionState, NewState);

/** @brief Fired when matchmaking state changes (searching, found, etc.) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMatchmakingStateChanged, EMGMatchmakingState, NewState);

/** @brief Fired when a session is discovered during browsing or matchmaking */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSessionFound, const FMGSessionInfo&, Session);

/** @brief Fired when a new player joins the current lobby */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerJoined, const FMGLobbyPlayer&, Player);

/** @brief Fired when a player leaves the current lobby */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerLeft, const FString&, PlayerID);

/** @brief Fired when all players in the lobby have marked themselves ready */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnAllPlayersReady);

/** @brief Fired when party membership or settings change */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPartyUpdated, const FMGPartyInfo&, Party);

// ============================================================================
// Session Subsystem Class
// ============================================================================

/**
 * @brief Main subsystem for managing multiplayer sessions in Midnight Grind.
 *
 * This GameInstanceSubsystem provides a unified interface for all multiplayer
 * functionality. It handles session lifecycle, matchmaking, lobby management,
 * and party coordination.
 *
 * ## Lifecycle
 * The subsystem is automatically created when the game instance starts and
 * persists until the game is closed. It survives level transitions, maintaining
 * connection state throughout.
 *
 * ## Thread Safety
 * All public methods should be called from the game thread. Network callbacks
 * are marshalled to the game thread before invoking delegates.
 *
 * ## Error Handling
 * Most operations that can fail will broadcast appropriate state changes via
 * delegates rather than returning error codes. Subscribe to OnMatchmakingStateChanged
 * and OnSessionStateChanged to handle failures.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Subsystem Lifecycle
	// ========================================================================

	/** @brief Called when the subsystem is created. Sets up network listeners. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** @brief Called when the subsystem is destroyed. Cleans up active sessions. */
	virtual void Deinitialize() override;

	// ========================================================================
	// Session Management
	// These functions control the lifecycle of multiplayer sessions.
	// A session represents a single game server where races take place.
	// ========================================================================

	/**
	 * @brief Creates a new multiplayer session with the specified settings.
	 * @param Settings Initial configuration for the session (track, mode, privacy, etc.)
	 * @note The local player becomes the host of the new session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(const FMGSessionInfo& Settings);

	/**
	 * @brief Joins an existing session by its unique identifier.
	 * @param SessionID The unique ID of the session to join
	 * @note Use GetAvailableSessions() or matchmaking to discover session IDs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSession(const FString& SessionID);

	/**
	 * @brief Leaves the current session and returns to the main menu state.
	 * @note If the local player is the host, the session may be migrated or closed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	void LeaveSession();

	/**
	 * @brief Gets complete information about the current session.
	 * @return Session info struct, or empty struct if not in a session
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	FMGSessionInfo GetCurrentSession() const { return CurrentSession; }

	/**
	 * @brief Gets the current session lifecycle state.
	 * @return Current state (None if not in a session)
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	EMGSessionState GetSessionState() const { return CurrentSession.State; }

	/**
	 * @brief Checks if the local player is currently in any session.
	 * @return True if in a session (lobby or race), false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsInSession() const { return CurrentSession.State != EMGSessionState::None; }

	/**
	 * @brief Checks if the local player is the host of the current session.
	 * @return True if host, false if client or not in session
	 */
	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsHost() const { return bIsHost; }

	// ========================================================================
	// Matchmaking
	// Automated opponent finding based on skill, region, and preferences.
	// Matchmaking searches for or creates sessions automatically.
	// ========================================================================

	/**
	 * @brief Begins searching for a match with the specified settings.
	 * @param Settings Configuration for the search (playlist, skill range, ping limits)
	 * @note Subscribe to OnMatchmakingStateChanged to track progress.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Matchmaking")
	void StartMatchmaking(const FMGMatchmakingSettings& Settings);

	/**
	 * @brief Cancels an in-progress matchmaking search.
	 * @note Safe to call even if not currently matchmaking.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Matchmaking")
	void CancelMatchmaking();

	/**
	 * @brief Gets the current matchmaking state.
	 * @return Current state of the matchmaking process
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Matchmaking")
	EMGMatchmakingState GetMatchmakingState() const { return MatchmakingState; }

	/**
	 * @brief Gets how long the current matchmaking search has been active.
	 * @return Time in seconds since matchmaking started
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Matchmaking")
	float GetMatchmakingTime() const { return MatchmakingTime; }

	/**
	 * @brief Gets the estimated number of players searching in the same queue.
	 * @return Approximate player count (may be delayed/estimated)
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Matchmaking")
	int32 GetPlayersInQueue() const { return PlayersInQueue; }

	// ========================================================================
	// Quick Play
	// Simplified entry points for common matchmaking scenarios.
	// These wrap StartMatchmaking with preset configurations.
	// ========================================================================

	/**
	 * @brief Starts a quick match search in the specified playlist.
	 * @param PlaylistID The playlist to search (e.g., "Circuit", "Sprint")
	 * @note Uses default matchmaking settings for the playlist.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|QuickPlay")
	void QuickPlay(FName PlaylistID);

	/**
	 * @brief Starts searching for a ranked competitive match.
	 * @note Uses stricter skill-based matching for competitive play.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|QuickPlay")
	void QuickPlayRanked();

	// ========================================================================
	// Lobby Management
	// Functions for interacting with the pre-race lobby.
	// These control vehicle selection, ready state, and team assignment.
	// ========================================================================

	/**
	 * @brief Gets all players currently in the lobby.
	 * @return Array of player info for everyone in the lobby
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Lobby")
	TArray<FMGLobbyPlayer> GetLobbyPlayers() const { return LobbyPlayers; }

	/**
	 * @brief Sets the local player's ready state.
	 * @param bReady True to mark as ready, false to unready
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Lobby")
	void SetReady(bool bReady);

	/**
	 * @brief Sets the local player's vehicle selection.
	 * @param VehicleID ID of the vehicle to use
	 * @param PI Performance Index of the vehicle (for class restrictions)
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Lobby")
	void SetVehicle(FName VehicleID, int32 PI);

	/**
	 * @brief Sets the local player's team for team-based modes.
	 * @param TeamIndex Index of the team to join (0-based, or -1 for auto-assign)
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Lobby")
	void SetTeam(int32 TeamIndex);

	/**
	 * @brief Checks if all players in the lobby are ready.
	 * @return True if everyone is ready, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Lobby")
	bool AreAllPlayersReady() const;

	/**
	 * @brief Gets the number of players who have marked themselves ready.
	 * @return Count of ready players
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Lobby")
	int32 GetReadyPlayerCount() const;

	// ========================================================================
	// Host Controls
	// Functions only available to the session host.
	// These control session-wide settings and match flow.
	// ========================================================================

	/**
	 * @brief Sets the track for the next race (host only).
	 * @param TrackID ID of the track to race on
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void SetTrack(FName TrackID);

	/**
	 * @brief Sets the game mode for the session (host only).
	 * @param ModeID ID of the game mode (e.g., "Circuit", "Sprint", "Drift")
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void SetGameMode(FName ModeID);

	/**
	 * @brief Sets the lobby privacy level (host only).
	 * @param Privacy New privacy setting
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void SetPrivacy(EMGLobbyPrivacy Privacy);

	/**
	 * @brief Removes a player from the session (host only).
	 * @param PlayerID ID of the player to kick
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void KickPlayer(const FString& PlayerID);

	/**
	 * @brief Initiates the race start countdown (host only).
	 * @note All players should be ready before calling this.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void StartRace();

	/**
	 * @brief Randomly selects a track from the playlist (host only).
	 * @note Useful for variety or when players cannot agree on a track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void RandomizeTrack();

	// ========================================================================
	// Party System
	// Parties are persistent player groups that queue together.
	// Party members stay together through multiple races.
	// ========================================================================

	/**
	 * @brief Creates a new party with the local player as leader.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Party")
	void CreateParty();

	/**
	 * @brief Joins an existing party by its ID.
	 * @param PartyID Unique identifier of the party to join
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Party")
	void JoinParty(const FString& PartyID);

	/**
	 * @brief Leaves the current party.
	 * @note If you are the leader, leadership transfers to another member.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Party")
	void LeaveParty();

	/**
	 * @brief Sends a party invite to another player.
	 * @param PlayerID ID of the player to invite
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Party")
	void InviteToParty(const FString& PlayerID);

	/**
	 * @brief Gets information about the current party.
	 * @return Party info struct, or empty struct if not in a party
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Party")
	FMGPartyInfo GetPartyInfo() const { return CurrentParty; }

	/**
	 * @brief Checks if the local player is in a party.
	 * @return True if in a party, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Party")
	bool IsInParty() const { return !CurrentParty.PartyID.IsEmpty(); }

	/**
	 * @brief Checks if the local player is the party leader.
	 * @return True if leader, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Party")
	bool IsPartyLeader() const;

	// ========================================================================
	// Invites
	// Functions for sending and managing game invites.
	// Invites allow players to join sessions or parties.
	// ========================================================================

	/**
	 * @brief Invites a specific player to the current session.
	 * @param PlayerID ID of the player to invite
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void InvitePlayer(const FString& PlayerID);

	/**
	 * @brief Opens the platform friends picker to send invites.
	 * @note Uses platform-specific UI (Steam, Xbox, PSN, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void InviteFriends();

	/**
	 * @brief Invites all online members of the player's crew.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void InviteCrew();

	/**
	 * @brief Accepts a pending invite.
	 * @param InviteID ID of the invite to accept
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void AcceptInvite(const FString& InviteID);

	/**
	 * @brief Declines a pending invite.
	 * @param InviteID ID of the invite to decline
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void DeclineInvite(const FString& InviteID);

	// ========================================================================
	// Server Browser
	// Functions for discovering and filtering available sessions.
	// Alternative to matchmaking for players who want specific sessions.
	// ========================================================================

	/**
	 * @brief Refreshes the list of available sessions.
	 * @note Results are available via GetAvailableSessions() after refresh completes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Browser")
	void RefreshSessionList();

	/**
	 * @brief Gets the current list of available sessions.
	 * @return Array of session info for joinable sessions
	 */
	UFUNCTION(BlueprintPure, Category = "Session|Browser")
	TArray<FMGSessionInfo> GetAvailableSessions() const { return AvailableSessions; }

	/**
	 * @brief Filters the session list based on criteria.
	 * @param GameModeFilter Only show sessions with this mode (NAME_None for all)
	 * @param bHideFullSessions Exclude sessions at max capacity
	 * @param MaxPing Maximum acceptable ping in milliseconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Session|Browser")
	void FilterSessions(FName GameModeFilter, bool bHideFullSessions, int32 MaxPing);

	// ========================================================================
	// Events
	// Delegates for reacting to session, matchmaking, and party changes.
	// Subscribe to these in UI widgets for reactive updates.
	// ========================================================================

	/** @brief Broadcast when the session state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnSessionStateChanged OnSessionStateChanged;

	/** @brief Broadcast when matchmaking state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnMatchmakingStateChanged OnMatchmakingStateChanged;

	/** @brief Broadcast when a session is discovered. */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnSessionFound OnSessionFound;

	/** @brief Broadcast when a player joins the lobby. */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnPlayerJoined OnPlayerJoined;

	/** @brief Broadcast when a player leaves the lobby. */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnPlayerLeft OnPlayerLeft;

	/** @brief Broadcast when all players are ready. */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnAllPlayersReady OnAllPlayersReady;

	/** @brief Broadcast when party information changes. */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnPartyUpdated OnPartyUpdated;

protected:
	// ========================================================================
	// Internal Implementation
	// These functions handle the actual network communication and state updates.
	// ========================================================================

	/** @brief Called periodically to update matchmaking search. */
	void UpdateMatchmaking(float DeltaTime);

	/** @brief Sets the session state and broadcasts the change. */
	void SetSessionState(EMGSessionState NewState);

	/** @brief Sets the matchmaking state and broadcasts the change. */
	void SetMatchmakingState(EMGMatchmakingState NewState);

	/** @brief Simulates finding a match (for development/testing). */
	void SimulateMatchFound();

private:
	// ========================================================================
	// Private State
	// ========================================================================

	/// Current session information
	FMGSessionInfo CurrentSession;

	/// Current party information
	FMGPartyInfo CurrentParty;

	/// Active matchmaking configuration
	FMGMatchmakingSettings CurrentMatchmakingSettings;

	/// Players in the current lobby
	TArray<FMGLobbyPlayer> LobbyPlayers;

	/// Sessions discovered in server browser
	TArray<FMGSessionInfo> AvailableSessions;

	/// Current matchmaking search state
	EMGMatchmakingState MatchmakingState = EMGMatchmakingState::Idle;

	/// Timer handle for matchmaking updates
	FTimerHandle MatchmakingTimerHandle;

	/// Time spent in current matchmaking search
	float MatchmakingTime = 0.0f;

	/// Estimated players in the matchmaking queue
	int32 PlayersInQueue = 0;

	/// Whether local player is the session host
	bool bIsHost = false;

	/// Local player's unique identifier
	FString LocalPlayerID;
};

// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGMatchmakingSubsystem.h - Skill-Based Matchmaking and Lobby Management
 * ============================================================================
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 *
 * WHAT IS THIS FILE?
 * ------------------
 * This header file defines the matchmaking system for Midnight Grind, a racing game.
 * Matchmaking is the automated process of finding suitable opponents for players
 * to race against online. Think of it like a dating app, but for finding racing
 * opponents with similar skill levels.
 *
 * KEY CONCEPTS EXPLAINED:
 * -----------------------
 *
 * 1. SUBSYSTEM (UGameInstanceSubsystem)
 *    - In Unreal Engine, a "subsystem" is a singleton object that provides
 *      game-wide services. Think of it as a global manager class.
 *    - GameInstanceSubsystem means this object persists across level loads,
 *      so your matchmaking state is preserved when changing maps.
 *    - You access it via: GetGameInstance()->GetSubsystem<UMGMatchmakingSubsystem>()
 *
 * 2. MMR (Matchmaking Rating)
 *    - A hidden number representing player skill (like ELO in chess)
 *    - Players start at 1000 (average)
 *    - Win against better players = gain more MMR
 *    - Lose against worse players = lose more MMR
 *    - The system uses this to match players of similar skill
 *
 * 3. LOBBY
 *    - A virtual "waiting room" where players gather before a race
 *    - Players can see each other, select vehicles, mark themselves ready
 *    - Host controls when the race starts
 *
 * 4. DELEGATES (Events)
 *    - Unreal's way of implementing the Observer pattern
 *    - Other code can "subscribe" to these events to react to changes
 *    - Example: UI subscribes to OnMatchFound to show "Match Found!" popup
 *
 * 5. BLUEPRINTTYPE / BLUEPRINTCALLABLE
 *    - Macros that expose C++ code to Unreal's visual scripting (Blueprints)
 *    - Allows designers to use these systems without writing C++
 *
 * HOW THIS FITS IN THE ARCHITECTURE:
 * ----------------------------------
 *
 *   [Player wants to race online]
 *            |
 *            v
 *   [MGSessionSubsystem] <-- High-level session management (simplified API)
 *            |
 *            v
 *   [MGMatchmakingSubsystem] <-- THIS FILE: Finds opponents, manages lobbies
 *            |
 *            v
 *   [MGMultiplayerSubsystem] <-- Low-level network connections
 *            |
 *            v
 *   [Game Server / P2P Connection]
 *
 * TYPICAL USAGE FLOW:
 * -------------------
 * 1. Player clicks "Find Match" in UI
 * 2. UI calls StartMatchmaking() with player preferences
 * 3. System searches for players with similar MMR and acceptable ping
 * 4. OnMatchFound delegate fires when opponents are found
 * 5. Players enter lobby, select vehicles, mark ready
 * 6. Host clicks "Start Race" -> OnMatchStarting fires
 * 7. Race begins, OnMatchEnded fires when race completes
 * 8. MMR updates based on finishing position
 *
 * ============================================================================
 *
 * @file MGMatchmakingSubsystem.h
 * @brief Skill-Based Matchmaking and Lobby Management for Midnight Grind
 *
 * This subsystem provides comprehensive matchmaking functionality for finding
 * fair and competitive races. It handles the entire flow from queue entry
 * through lobby formation to match start.
 *
 * ## Matchmaking Philosophy
 * The system prioritizes match quality over speed while respecting player time.
 * Key principles:
 * - **Skill Balance**: Uses MMR (Matchmaking Rating) to create fair races
 * - **Latency Awareness**: Prefers nearby servers for responsive gameplay
 * - **Flexible Preferences**: Players can customize what they're looking for
 * - **Progressive Search**: Gradually widens criteria if no match is found
 *
 * ## MMR System
 * Players have a hidden skill rating (MMR) that adjusts based on race results:
 * - Starting MMR: 1000 (average)
 * - Winning against higher MMR = larger gain
 * - Losing against lower MMR = larger loss
 * - Placement in races (1st-8th) affects magnitude
 *
 * ## Visible Ranks
 * MMR maps to visible tiers for player progression:
 * - Bronze (0-799): Learning the ropes
 * - Silver (800-1199): Competent racers
 * - Gold (1200-1599): Skilled competitors
 * - Platinum (1600-1999): Expert drivers
 * - Diamond (2000-2399): Elite racers
 * - Champion (2400-2799): Top tier
 * - Legend (2800+): The best of the best
 *
 * ## Queue Types
 * - **Quick Race**: Casual unranked play with relaxed matching
 * - **Ranked**: Competitive mode affecting skill ratings
 * - **Private**: Direct lobby creation for friends
 * - **Tournament**: Special event queues with brackets
 * - **Crew**: Team-based matchmaking for crew battles
 *
 * @see UMGSessionSubsystem For the higher-level session interface
 * @see UMGMultiplayerSubsystem For network connection management
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMatchmakingSubsystem.generated.h"

// Forward declarations
class UMGMultiplayerSubsystem;

// ============================================================================
// Matchmaking State Enumerations
// ============================================================================

/**
 * @brief Current state of the matchmaking process.
 *
 * Tracks progress from initial search through match start,
 * including error states for UI feedback.
 */
UENUM(BlueprintType)
enum class EMGMatchmakingState : uint8
{
	/// Not searching - ready to queue
	Idle,
	/// Actively looking for opponents
	SearchingForMatch,
	/// Suitable match found, confirming
	MatchFound,
	/// Connecting to the lobby
	JoiningLobby,
	/// In lobby waiting for race start
	InLobby,
	/// Countdown in progress
	StartingMatch,
	/// Race actively in progress
	InMatch,
	/// User cancelled the search
	Cancelled,
	/// Search failed (timeout, network error, etc.)
	Failed
};

/**
 * @brief Types of multiplayer matches available for matchmaking.
 *
 * Each type has different rules for matching, rewards, and visibility.
 */
UENUM(BlueprintType)
enum class EMGMatchType : uint8
{
	/// Standard unranked race with relaxed skill matching
	QuickRace,
	/// Competitive mode affecting skill ratings
	Ranked,
	/// Invite-only lobby for friends
	Private,
	/// Special event bracket-style competition
	Tournament,
	/// Custom rules defined by the host
	Custom,
	/// Team-based crew vs crew racing
	Crew,
	/// Open world public server
	FreeroamPublic
};

/**
 * @brief Geographic regions for server selection.
 *
 * Matchmaking prefers servers in or near the player's region
 * to minimize latency.
 */
UENUM(BlueprintType)
enum class EMGMatchmakingRegion : uint8
{
	/// Let the system choose based on ping
	Automatic,
	/// US East, US West, US Central
	NorthAmerica,
	/// Brazil, Argentina
	SouthAmerica,
	/// UK, Germany, France, etc.
	Europe,
	/// Japan, Korea, Singapore, etc.
	Asia,
	/// Australia, New Zealand
	Oceania,
	/// UAE, Saudi Arabia
	MiddleEast,
	/// South Africa
	Africa
};

/**
 * @brief Visible skill tier for player profiles and matchmaking display.
 *
 * These tiers provide progression milestones and are derived from
 * the hidden MMR value.
 */
UENUM(BlueprintType)
enum class EMGSkillTier : uint8
{
	/// 0-799 MMR: New or learning players
	Bronze,
	/// 800-1199 MMR: Average skill level
	Silver,
	/// 1200-1599 MMR: Above average
	Gold,
	/// 1600-1999 MMR: Highly skilled
	Platinum,
	/// 2000-2399 MMR: Expert level
	Diamond,
	/// 2400-2799 MMR: Top 1%
	Champion,
	/// 2800+ MMR: Elite players
	Legend
};

/**
 * @brief Current state of a player within a lobby.
 *
 * Tracks readiness and loading progress for all lobby members.
 */
UENUM(BlueprintType)
enum class EMGLobbyPlayerState : uint8
{
	/// Player has not clicked ready
	NotReady,
	/// Player has confirmed ready
	Ready,
	/// Player is loading the race level
	Loading,
	/// Player is in the active race
	InGame,
	/// Player is watching as spectator
	Spectating,
	/// Player's connection was lost
	Disconnected
};

// ============================================================================
// Matchmaking Configuration Structures
// ============================================================================

/**
 * @brief Player preferences for matchmaking searches.
 *
 * These settings control how the matchmaking system finds opponents,
 * balancing match quality against queue time.
 */
USTRUCT(BlueprintType)
struct FMGMatchmakingPreferences
{
	GENERATED_BODY()

	/// Type of match to search for
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchType MatchType = EMGMatchType::QuickRace;

	/// Preferred server region (Automatic uses ping-based selection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchmakingRegion PreferredRegion = EMGMatchmakingRegion::Automatic;

	/// Preferred race mode (NAME_None accepts any)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PreferredRaceMode = NAME_None;

	/// Preferred track (NAME_None accepts any)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PreferredTrack = NAME_None;

	/// Minimum players required to start a match
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPlayers = 2;

	/// Maximum players allowed in the match
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/// Allow matching with players on other platforms
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCrossPlay = true;

	/// Allow joining matches already in progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowBackfill = true;

	/// Use skill-based matching (if false, matches anyone)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkillBasedMatchmaking = true;

	/// Maximum acceptable ping in milliseconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPingThreshold = 150;

	/// Restrict to specific vehicle classes (empty = all allowed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> VehicleClassRestrictions;

	/// Additional key-value settings for custom modes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomSettings;
};

// ============================================================================
// Skill Rating Structures
// ============================================================================

/**
 * @brief Complete skill rating data for a player.
 *
 * Contains both the hidden MMR and the visible tier/rank information
 * used for matchmaking and progression display.
 */
USTRUCT(BlueprintType)
struct FMGPlayerSkillRating
{
	GENERATED_BODY()

	/// Hidden Matchmaking Rating (1000 = average)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MMR = 1000;

	/// Current visible skill tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSkillTier Tier = EMGSkillTier::Bronze;

	/// Division within the tier (1-4, lower is better)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Division = 1;

	/// Points toward next division promotion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RankPoints = 0;

	/// Current consecutive wins
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WinStreak = 0;

	/// Current consecutive losses
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LossStreak = 0;

	/// Overall win percentage (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.5f;

	/// Total ranked races completed all-time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRankedRaces = 0;

	/// Wins in the current season
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonWins = 0;

	/// Losses in the current season
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonLosses = 0;

	/// Timestamp of the most recent ranked race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRankedRace;
};

/**
 * @brief Active matchmaking search ticket.
 *
 * Represents a player's position in the matchmaking queue,
 * tracking search progress and any criteria expansions.
 */
USTRUCT(BlueprintType)
struct FMGMatchmakingTicket
{
	GENERATED_BODY()

	/// Unique identifier for this search ticket
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TicketID;

	/// Player who created this ticket
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	/// Search preferences for this ticket
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGMatchmakingPreferences Preferences;

	/// Player's skill rating at time of queue
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerSkillRating SkillRating;

	/// When this ticket was created
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	/// How long the search has been active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SearchTimeSeconds = 0.0f;

	/// How many times search criteria has expanded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SearchExpansionLevel = 0;

	/// Current ping at time of search
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPing = 0;
};

// ============================================================================
// Lobby Data Structures
// ============================================================================

/**
 * @brief Information about a player in a lobby.
 *
 * Contains all data needed to display player cards and
 * make game-start decisions.
 */
USTRUCT(BlueprintType)
struct FMGLobbyPlayer
{
	GENERATED_BODY()

	/// Unique platform identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	/// Human-readable display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	/// Current ready/loading state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLobbyPlayerState State = EMGLobbyPlayerState::NotReady;

	/// Player's skill rating
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerSkillRating SkillRating;

	/// Selected vehicle for the race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SelectedVehicle = NAME_None;

	/// Team assignment for team modes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamIndex = 0;

	/// Network latency in milliseconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	/// Whether this player is the session host
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHost = false;

	/// Whether this player is from another platform
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrossPlayPlayer = false;

	/// Platform identifier (e.g., "Steam", "Xbox", "PlayStation")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Platform;

	/// When this player joined the lobby
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime JoinedTime;
};

/**
 * @brief Configurable settings for a lobby.
 *
 * Host can modify these settings before the race starts
 * to customize the experience.
 */
USTRUCT(BlueprintType)
struct FMGLobbySettings
{
	GENERATED_BODY()

	/// Race mode (e.g., "Circuit", "Sprint", "Drift")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceMode = NAME_None;

	/// Track to race on
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID = NAME_None;

	/// Number of laps for circuit races
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 3;

	/// Maximum players allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/// Invite-only mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrivate = false;

	/// Password for private lobbies
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;

	/// Allow non-racing spectators
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowSpectators = true;

	/// Maximum spectator count
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSpectators = 10;

	/// Enable vehicle collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCollisionsEnabled = true;

	/// Enable rubber-banding for trailing players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCatchupEnabled = false;

	/// Seconds of countdown before race start
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CountdownTime = 5.0f;

	/// Restrict to specific vehicle classes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AllowedVehicleClasses;

	/// Custom rule settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomRules;
};

/**
 * @brief Complete lobby data including all players and settings.
 *
 * Represents the full state of a match lobby for synchronization
 * and display purposes.
 */
USTRUCT(BlueprintType)
struct FMGMatchLobby
{
	GENERATED_BODY()

	/// Unique lobby identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LobbyID;

	/// Associated session ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	/// Host player ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostPlayerID;

	/// Type of match
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchType MatchType = EMGMatchType::QuickRace;

	/// Lobby configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLobbySettings Settings;

	/// All racing participants
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLobbyPlayer> Players;

	/// Non-racing spectators
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLobbyPlayer> Spectators;

	/// Server region
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchmakingRegion Region = EMGMatchmakingRegion::Automatic;

	/// When the lobby was created
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	/// Whether countdown is in progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMatchStarting = false;

	/// Seconds remaining in countdown
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CountdownRemaining = 0.0f;

	/// Average skill of lobby players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AverageSkillMMR = 0;
};

// ============================================================================
// Match Result Structures
// ============================================================================

/**
 * @brief Complete results from a finished match.
 *
 * Contains final standings, rating changes, and timing data
 * for post-race display and progression.
 */
USTRUCT(BlueprintType)
struct FMGMatchResult
{
	GENERATED_BODY()

	/// Unique match identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MatchID;

	/// Type of match that was played
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchType MatchType = EMGMatchType::QuickRace;

	/// Track that was raced
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID = NAME_None;

	/// Race mode that was used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceMode = NAME_None;

	/// Player IDs in finishing order (1st place first)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> FinalStandings;

	/// MMR change for each player (can be positive or negative)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> MMRChanges;

	/// Rank point change for each player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> RankPointChanges;

	/// When the race started
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	/// When the race ended
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	/// Total race duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRaceTime = 0.0f;
};

// ============================================================================
// Server Information
// ============================================================================

/**
 * @brief Information about a game server for browser display.
 *
 * Used by the server browser to show available dedicated servers
 * or player-hosted sessions.
 */
USTRUCT(BlueprintType)
struct FMGServerInfo
{
	GENERATED_BODY()

	/// Unique server identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ServerID;

	/// Network address (IP or hostname)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ServerAddress;

	/// Port number for connections
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Port = 7777;

	/// Geographic region
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchmakingRegion Region = EMGMatchmakingRegion::Automatic;

	/// Current player count
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayers = 0;

	/// Maximum player capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/// Latency in milliseconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	/// Whether this is a dedicated (not player-hosted) server
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDedicated = true;

	/// Whether the server is accepting connections
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAvailable = true;
};

// ============================================================================
// Event Delegates
// ============================================================================

/** @brief Fired when matchmaking state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingStateChanged, EMGMatchmakingState, NewState);

/** @brief Fired when a suitable match is found. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFound, const FMGMatchLobby&, Lobby);

/** @brief Fired when matchmaking fails. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingFailed, const FString&, ErrorMessage);

/** @brief Fired periodically with search time updates. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingProgressUpdated, float, SearchTimeSeconds);

/** @brief Fired when lobby data is updated. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyUpdated, const FMGMatchLobby&, Lobby);

/** @brief Fired when a player joins the lobby. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerJoinedLobby, const FString&, LobbyID, const FMGLobbyPlayer&, Player);

/** @brief Fired when a player leaves the lobby. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeftLobby, const FString&, LobbyID, const FString&, PlayerID);

/** @brief Fired when a player's state changes (ready, loading, etc.). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateChanged, const FString&, PlayerID, EMGLobbyPlayerState, NewState);

/** @brief Fired when the race countdown begins. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyCountdownStarted, float, CountdownTime);

/** @brief Fired when the race is about to begin. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchStarting);

/** @brief Fired when a race ends with full results. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchEnded, const FMGMatchResult&, Result);

/** @brief Fired when skill rating changes after a match. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillRatingUpdated, const FMGPlayerSkillRating&, OldRating, const FMGPlayerSkillRating&, NewRating);

/** @brief Fired when kicked from a lobby by the host. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKickedFromLobby);

// ============================================================================
// Matchmaking Subsystem Class
// ============================================================================

/**
 * @brief Comprehensive matchmaking and lobby management subsystem.
 *
 * Handles skill-based matchmaking, lobby lifecycle, and race coordination
 * for Midnight Grind multiplayer.
 *
 * ## Key Features
 * - Skill-based matchmaking with MMR system
 * - Multiple match types (Quick, Ranked, Private, Tournament)
 * - Cross-platform play support
 * - Lobby management with host controls
 * - Server browser for direct connections
 * - Regional server selection
 *
 * ## Usage Flow
 * 1. Set matchmaking preferences
 * 2. Call StartMatchmaking()
 * 3. Listen for OnMatchFound or OnMatchmakingFailed
 * 4. In lobby, set ready state with SetPlayerReady()
 * 5. Host starts countdown with StartLobbyCountdown()
 * 6. After race, receive OnMatchEnded with results
 *
 * @see UMGSessionSubsystem For simplified session management
 * @see UMGPartySubsystem For party/group features
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================================================
	// Matchmaking
	// Core matchmaking functionality for finding opponents.
	// ========================================================================

	/**
	 * @brief Starts searching for a match with the given preferences.
	 * @param Preferences Configuration for the search
	 * @return True if search started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool StartMatchmaking(const FMGMatchmakingPreferences& Preferences);

	/**
	 * @brief Cancels the current matchmaking search.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void CancelMatchmaking();

	/**
	 * @brief Checks if currently searching for a match.
	 * @return True if matchmaking is active
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool IsMatchmaking() const { return MatchmakingState == EMGMatchmakingState::SearchingForMatch; }

	/**
	 * @brief Gets the current matchmaking state.
	 * @return Current state of the matchmaking process
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	EMGMatchmakingState GetMatchmakingState() const { return MatchmakingState; }

	/**
	 * @brief Gets how long the current search has been active.
	 * @return Search time in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	float GetMatchmakingTime() const;

	/**
	 * @brief Gets the current matchmaking ticket.
	 * @return Active ticket, or empty if not matchmaking
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	FMGMatchmakingTicket GetCurrentTicket() const { return CurrentTicket; }

	// ========================================================================
	// Lobby Management
	// Creating, joining, and managing game lobbies.
	// ========================================================================

	/**
	 * @brief Creates a new lobby with the specified settings.
	 * @param Settings Configuration for the lobby
	 * @param MatchType Type of match for this lobby
	 * @return True if lobby creation started
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool CreateLobby(const FMGLobbySettings& Settings, EMGMatchType MatchType);

	/**
	 * @brief Joins an existing lobby by ID.
	 * @param LobbyID Unique identifier of the lobby
	 * @param Password Optional password for private lobbies
	 * @return True if join was initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool JoinLobby(const FString& LobbyID, const FString& Password = TEXT(""));

	/**
	 * @brief Joins a lobby using a short join code.
	 * @param JoinCode Short alphanumeric code (e.g., "RACE123")
	 * @return True if join was initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool JoinLobbyByCode(const FString& JoinCode);

	/**
	 * @brief Leaves the current lobby.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	void LeaveLobby();

	/**
	 * @brief Checks if currently in a lobby.
	 * @return True if in a lobby
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Lobby")
	bool IsInLobby() const { return bInLobby; }

	/**
	 * @brief Gets the current lobby data.
	 * @return Lobby info, or empty if not in lobby
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Lobby")
	FMGMatchLobby GetCurrentLobby() const { return CurrentLobby; }

	/**
	 * @brief Generates a short join code for the current lobby.
	 * @return Shareable code string
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Lobby")
	FString GenerateLobbyJoinCode() const;

	// ========================================================================
	// Lobby Host Functions
	// These functions are only available to the lobby host.
	// ========================================================================

	/**
	 * @brief Updates lobby settings (host only).
	 * @param NewSettings New configuration to apply
	 * @return True if update was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool UpdateLobbySettings(const FMGLobbySettings& NewSettings);

	/**
	 * @brief Kicks a player from the lobby (host only).
	 * @param PlayerID ID of the player to remove
	 * @return True if kick was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool KickPlayer(const FString& PlayerID);

	/**
	 * @brief Bans a player from rejoining (host only).
	 * @param PlayerID ID of the player to ban
	 * @return True if ban was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool BanPlayer(const FString& PlayerID);

	/**
	 * @brief Transfers host privileges to another player (host only).
	 * @param NewHostPlayerID ID of the new host
	 * @return True if transfer was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool TransferHost(const FString& NewHostPlayerID);

	/**
	 * @brief Starts the pre-race countdown (host only).
	 * @return True if countdown started
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool StartLobbyCountdown();

	/**
	 * @brief Cancels an active countdown (host only).
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	void CancelLobbyCountdown();

	// ========================================================================
	// Player Functions
	// Actions available to all lobby players.
	// ========================================================================

	/**
	 * @brief Sets the local player's ready state.
	 * @param bReady True to mark ready
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Player")
	void SetPlayerReady(bool bReady);

	/**
	 * @brief Sets the local player's vehicle selection.
	 * @param VehicleID ID of the vehicle to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Player")
	void SetSelectedVehicle(FName VehicleID);

	/**
	 * @brief Sets the local player's team.
	 * @param TeamIndex Team index to join
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Player")
	void SetTeam(int32 TeamIndex);

	/**
	 * @brief Checks if the local player is the host.
	 * @return True if hosting
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Player")
	bool IsLocalPlayerHost() const;

	/**
	 * @brief Checks if the local player is ready.
	 * @return True if marked ready
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Player")
	bool IsLocalPlayerReady() const;

	/**
	 * @brief Checks if all players are ready.
	 * @return True if everyone is ready
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Player")
	bool AreAllPlayersReady() const;

	/**
	 * @brief Gets the count of ready players.
	 * @return Number of ready players
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Player")
	int32 GetReadyPlayerCount() const;

	// ========================================================================
	// Skill Rating
	// MMR and rank management functions.
	// ========================================================================

	/**
	 * @brief Gets the local player's skill rating.
	 * @return Current skill rating data
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Skill")
	FMGPlayerSkillRating GetLocalPlayerSkillRating() const { return LocalPlayerSkill; }

	/**
	 * @brief Updates skill rating based on match results.
	 * @param MatchResult The completed match data
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Skill")
	void UpdateSkillRatingFromMatch(const FMGMatchResult& MatchResult);

	/**
	 * @brief Calculates the visible tier from an MMR value.
	 * @param MMR The matchmaking rating
	 * @return Corresponding skill tier
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Skill")
	EMGSkillTier CalculateTierFromMMR(int32 MMR) const;

	/**
	 * @brief Calculates MMR change for a race result.
	 * @param CurrentMMR Player's current MMR
	 * @param OpponentAvgMMR Average MMR of opponents
	 * @param Position Finishing position (1-based)
	 * @param TotalPlayers Total racers in the match
	 * @return MMR change (positive or negative)
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Skill")
	int32 CalculateMMRChange(int32 CurrentMMR, int32 OpponentAvgMMR, int32 Position, int32 TotalPlayers) const;

	// ========================================================================
	// Server Browser
	// Direct server/lobby discovery without matchmaking.
	// ========================================================================

	/**
	 * @brief Refreshes the list of available servers.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Browser")
	void RefreshServerList();

	/**
	 * @brief Refreshes the list of available lobbies.
	 * @param TypeFilter Only show lobbies of this match type
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Browser")
	void RefreshLobbyList(EMGMatchType TypeFilter);

	/**
	 * @brief Gets the list of available servers.
	 * @return Array of server info
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Browser")
	TArray<FMGServerInfo> GetAvailableServers() const { return AvailableServers; }

	/**
	 * @brief Gets the list of available lobbies.
	 * @return Array of lobby info
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Browser")
	TArray<FMGMatchLobby> GetAvailableLobbies() const { return AvailableLobbies; }

	// ========================================================================
	// Region
	// Server region selection and ping testing.
	// ========================================================================

	/**
	 * @brief Gets the best region based on ping.
	 * @return Region with lowest latency
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Region")
	EMGMatchmakingRegion GetBestRegion() const;

	/**
	 * @brief Pings all regions to find the best one.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Region")
	void PingAllRegions();

	/**
	 * @brief Gets the ping to a specific region.
	 * @param Region The region to check
	 * @return Ping in milliseconds
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Region")
	int32 GetRegionPing(EMGMatchmakingRegion Region) const;

	// ========================================================================
	// Match History
	// Past match data for stats and replays.
	// ========================================================================

	/**
	 * @brief Gets recent match results.
	 * @param Count Maximum number of matches to return
	 * @return Array of match results, newest first
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking|History")
	TArray<FMGMatchResult> GetRecentMatches(int32 Count = 10) const;

	// ========================================================================
	// Event Delegates
	// Subscribe to these for reactive updates.
	// ========================================================================

	/** @brief Broadcast when matchmaking state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchmakingStateChanged OnMatchmakingStateChanged;

	/** @brief Broadcast when a match is found. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchFound OnMatchFound;

	/** @brief Broadcast when matchmaking fails. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchmakingFailed OnMatchmakingFailed;

	/** @brief Broadcast with search time updates. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchmakingProgressUpdated OnMatchmakingProgressUpdated;

	/** @brief Broadcast when lobby data changes. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnLobbyUpdated OnLobbyUpdated;

	/** @brief Broadcast when a player joins. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnPlayerJoinedLobby OnPlayerJoinedLobby;

	/** @brief Broadcast when a player leaves. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnPlayerLeftLobby OnPlayerLeftLobby;

	/** @brief Broadcast when player state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnPlayerStateChanged OnPlayerStateChanged;

	/** @brief Broadcast when countdown starts. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnLobbyCountdownStarted OnLobbyCountdownStarted;

	/** @brief Broadcast when match is starting. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchStarting OnMatchStarting;

	/** @brief Broadcast when match ends. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchEnded OnMatchEnded;

	/** @brief Broadcast when skill rating updates. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnSkillRatingUpdated OnSkillRatingUpdated;

	/** @brief Broadcast when kicked from lobby. */
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnKickedFromLobby OnKickedFromLobby;

protected:
	// ========================================================================
	// Internal Functions
	// ========================================================================

	/** @brief Updates matchmaking state and broadcasts change. */
	void SetMatchmakingState(EMGMatchmakingState NewState);

	/** @brief Called periodically during matchmaking search. */
	void OnMatchmakingTick();

	/** @brief Widens search criteria after timeout. */
	void ExpandSearchCriteria();

	/** @brief Called periodically during countdown. */
	void OnCountdownTick();

	/** @brief Simulates finding a match (for testing). */
	void SimulateMatchFound();

	/** @brief Creates local player lobby data. */
	FMGLobbyPlayer CreateLocalPlayer() const;

	// ========================================================================
	// Internal State
	// ========================================================================

	/// Current matchmaking state
	UPROPERTY()
	EMGMatchmakingState MatchmakingState = EMGMatchmakingState::Idle;

	/// Active matchmaking ticket
	UPROPERTY()
	FMGMatchmakingTicket CurrentTicket;

	/// Local player's skill rating
	UPROPERTY()
	FMGPlayerSkillRating LocalPlayerSkill;

	/// Current lobby data
	UPROPERTY()
	FMGMatchLobby CurrentLobby;

	/// Whether currently in a lobby
	UPROPERTY()
	bool bInLobby = false;

	/// Available dedicated servers
	UPROPERTY()
	TArray<FMGServerInfo> AvailableServers;

	/// Available player lobbies
	UPROPERTY()
	TArray<FMGMatchLobby> AvailableLobbies;

	/// Recent match history
	UPROPERTY()
	TArray<FMGMatchResult> MatchHistory;

	/// Cached ping values per region
	UPROPERTY()
	TMap<EMGMatchmakingRegion, int32> RegionPings;

	/// Timer for matchmaking updates
	FTimerHandle MatchmakingTimerHandle;

	/// Timer for countdown updates
	FTimerHandle CountdownTimerHandle;

	/// Seconds between search criteria expansions
	float SearchExpansionInterval = 10.0f;

	/// Maximum search time before failure
	float MaxSearchTime = 120.0f;
};

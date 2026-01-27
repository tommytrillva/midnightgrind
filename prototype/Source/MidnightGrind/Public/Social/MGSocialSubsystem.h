// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGSocialSubsystem.h
 * @brief Core social features subsystem for Midnight Grind
 *
 * This subsystem manages all social interactions between players, including:
 * - Friends list management (adding, removing, blocking players)
 * - Crew/club membership and management
 * - Game invites and join-in-progress functionality
 * - Recent players tracking from completed races
 * - Player presence and status updates
 *
 * The Social Subsystem serves as the central hub for player-to-player interactions,
 * enabling features like racing with friends, forming crews for competitive play,
 * and maintaining social connections within the game.
 *
 * @section usage_sec Basic Usage
 * Access this subsystem from any game code via:
 * @code
 * UMGSocialSubsystem* SocialSub = GameInstance->GetSubsystem<UMGSocialSubsystem>();
 * if (SocialSub)
 * {
 *     TArray<FMGFriendData> OnlineFriends = SocialSub->GetOnlineFriends();
 * }
 * @endcode
 *
 * @section events_sec Event-Driven Architecture
 * Subscribe to delegates like OnFriendListUpdated and OnGameInviteReceived to
 * respond to social events in your UI or gameplay code.
 *
 * @see UMGLeaderboardSubsystem For competitive rankings integration
 * @see UMGCrewSubsystem For advanced crew functionality (use Crew/ version)
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSocialSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS - Player Status & Request Types
// ============================================================================

/**
 * @brief Represents a friend's current activity status in the game
 *
 * Used to display presence information in friends lists and determine
 * whether a player can be joined or invited to activities.
 */
UENUM(BlueprintType)
enum class EMGFriendStatus : uint8
{
	Offline,        ///< Player is not connected to the game
	Online,         ///< Player is online, browsing menus
	InGarage,       ///< Player is customizing vehicles in their garage
	InLobby,        ///< Player is in a race lobby, may be joinable
	Racing,         ///< Player is actively in a race
	WatchingReplay, ///< Player is viewing a replay
	Away            ///< Player is idle/AFK
};

/**
 * @brief Status of a friend request between two players
 *
 * Tracks the lifecycle of friend requests from initial send through
 * acceptance, rejection, or blocking.
 */
UENUM(BlueprintType)
enum class EMGRequestStatus : uint8
{
	Pending,  ///< Request sent, awaiting response from recipient
	Accepted, ///< Request accepted, players are now friends
	Declined, ///< Request declined by recipient
	Blocked   ///< Recipient blocked the sender
};

/**
 * @brief Hierarchical rank within a crew determining permissions
 *
 * Higher ranks have more permissions for crew management. Used in the
 * simplified crew system; see Crew/MGCrewSubsystem.h for extended ranks.
 */
UENUM(BlueprintType)
enum class EMGCrewRank : uint8
{
	Member,  ///< Regular member - can participate but no management rights
	Officer, ///< Officer - can invite new members and kick lower ranks
	Leader   ///< Leader - full control including disbanding and promotions
};

// ============================================================================
// STRUCTURES - Friend & Player Data
// ============================================================================

/**
 * @brief Complete data for a single friend in the friends list
 *
 * Contains all information needed to display a friend in the UI and
 * determine available social actions (join, invite, etc.).
 */
USTRUCT(BlueprintType)
struct FMGFriendData
{
	GENERATED_BODY()

	/// Unique identifier for this player (persistent across sessions)
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/// Player's chosen display name shown in UI
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/// Current activity status (online, racing, etc.)
	UPROPERTY(BlueprintReadOnly)
	EMGFriendStatus Status = EMGFriendStatus::Offline;

	/// Custom status message set by the player
	UPROPERTY(BlueprintReadOnly)
	FString StatusText;

	/// Track ID if player is currently racing (empty otherwise)
	UPROPERTY(BlueprintReadOnly)
	FName CurrentTrack;

	/// Session ID for join-in-progress functionality
	UPROPERTY(BlueprintReadOnly)
	FString SessionID;

	/// Name of the crew this player belongs to (empty if none)
	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	/// Player's account level (1 = new player)
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/// Accumulated reputation points from races
	UPROPERTY(BlueprintReadOnly)
	int32 Reputation = 0;

	/// Lifetime race wins
	UPROPERTY(BlueprintReadOnly)
	int32 TotalWins = 0;

	/// Timestamp of last online activity (for "last seen" display)
	UPROPERTY(BlueprintReadOnly)
	FDateTime LastOnline;

	/// Reference to player's avatar/profile picture asset
	UPROPERTY(BlueprintReadOnly)
	FName AvatarID;

	/// True if player has been marked as a favorite for quick access
	UPROPERTY(BlueprintReadOnly)
	bool bIsFavorite = false;

	/// True if the player's current session is joinable
	UPROPERTY(BlueprintReadOnly)
	bool bCanJoin = false;
};

/**
 * @brief Data for a pending friend request (incoming or outgoing)
 *
 * Represents a friend request that has been sent but not yet fully resolved.
 * Used to populate friend request lists and notifications.
 */
USTRUCT(BlueprintType)
struct FMGFriendRequest
{
	GENERATED_BODY()

	/// Unique identifier for this specific request
	UPROPERTY(BlueprintReadOnly)
	FString RequestID;

	/// Player ID of the person who sent the request
	UPROPERTY(BlueprintReadOnly)
	FString SenderID;

	/// Display name of the sender
	UPROPERTY(BlueprintReadOnly)
	FString SenderName;

	/// Sender's account level (helps recipient gauge experience)
	UPROPERTY(BlueprintReadOnly)
	int32 SenderLevel = 1;

	/// When the request was sent
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/// Current status of this request
	UPROPERTY(BlueprintReadOnly)
	EMGRequestStatus Status = EMGRequestStatus::Pending;

	/// True = someone sent this to us; False = we sent this to someone
	UPROPERTY(BlueprintReadOnly)
	bool bIsIncoming = true;
};

// ============================================================================
// STRUCTURES - Crew Data
// ============================================================================

/**
 * @brief Information about a single member within a crew
 *
 * Tracks each crew member's rank, contributions, and activity within the crew.
 */
USTRUCT(BlueprintType)
struct FMGCrewMember
{
	GENERATED_BODY()

	/// Unique player identifier
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/// Player's display name
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/// Member's rank within the crew hierarchy
	UPROPERTY(BlueprintReadOnly)
	EMGCrewRank Rank = EMGCrewRank::Member;

	/// When this player joined the crew
	UPROPERTY(BlueprintReadOnly)
	FDateTime JoinDate;

	/// XP contributed to crew this week (resets weekly)
	UPROPERTY(BlueprintReadOnly)
	int32 WeeklyContribution = 0;

	/// Lifetime XP contributed to this crew
	UPROPERTY(BlueprintReadOnly)
	int32 TotalContribution = 0;

	/// Current online/activity status
	UPROPERTY(BlueprintReadOnly)
	EMGFriendStatus Status = EMGFriendStatus::Offline;

	/// Player's account level
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;
};

/**
 * @brief Complete data structure for a crew/club
 *
 * Contains all information about a crew including identity, progression,
 * statistics, and member roster. Used for crew display and management UIs.
 */
USTRUCT(BlueprintType)
struct FMGCrewData
{
	GENERATED_BODY()

	/// Unique identifier for this crew
	UPROPERTY(BlueprintReadOnly)
	FString CrewID;

	/// Full display name of the crew
	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	/// Short tag shown next to player names (e.g., "[TAG]")
	UPROPERTY(BlueprintReadOnly)
	FString CrewTag;

	/// Crew description/bio set by leadership
	UPROPERTY(BlueprintReadOnly)
	FString Description;

	/// Reference to crew emblem/logo asset
	UPROPERTY(BlueprintReadOnly)
	FName EmblemID;

	/// Primary crew color used for UI and liveries
	UPROPERTY(BlueprintReadOnly)
	FLinearColor CrewColor = FLinearColor::White;

	/// Current crew level (unlocks perks and capacity)
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/// Current XP progress toward next level
	UPROPERTY(BlueprintReadOnly)
	int32 CrewXP = 0;

	/// XP threshold required to reach next level
	UPROPERTY(BlueprintReadOnly)
	int32 NextLevelXP = 1000;

	/// Current number of members in the crew
	UPROPERTY(BlueprintReadOnly)
	int32 MemberCount = 0;

	/// Maximum allowed members (increases with crew level)
	UPROPERTY(BlueprintReadOnly)
	int32 MaxMembers = 50;

	/// Combined race wins from all crew members
	UPROPERTY(BlueprintReadOnly)
	int32 TotalWins = 0;

	/// Crew's position on the weekly leaderboard
	UPROPERTY(BlueprintReadOnly)
	int32 WeeklyRank = 0;

	/// True if crew is accepting new member applications
	UPROPERTY(BlueprintReadOnly)
	bool bIsRecruiting = true;

	/// When the crew was founded
	UPROPERTY(BlueprintReadOnly)
	FDateTime CreatedDate;

	/// Full roster of crew members
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGCrewMember> Members;
};

/**
 * @brief An invitation to join a crew
 *
 * Represents a pending crew invitation sent by an officer or leader.
 */
USTRUCT(BlueprintType)
struct FMGCrewInvite
{
	GENERATED_BODY()

	/// Unique identifier for this invite
	UPROPERTY(BlueprintReadOnly)
	FString InviteID;

	/// ID of the crew extending the invitation
	UPROPERTY(BlueprintReadOnly)
	FString CrewID;

	/// Display name of the inviting crew
	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	/// Name of the player who sent the invite
	UPROPERTY(BlueprintReadOnly)
	FString InviterName;

	/// When the invitation was sent
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;
};

// ============================================================================
// STRUCTURES - Recent Players
// ============================================================================

/**
 * @brief Record of a player encountered in a recent race
 *
 * Tracks players you've raced against for easy friend adding or reporting.
 * Automatically populated after each race completes.
 */
USTRUCT(BlueprintType)
struct FMGRecentPlayer
{
	GENERATED_BODY()

	/// Unique player identifier
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/// Player's display name
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/// Timestamp of the most recent race together
	UPROPERTY(BlueprintReadOnly)
	FDateTime LastRaced;

	/// Track where the race occurred
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/// Their finishing position in that race
	UPROPERTY(BlueprintReadOnly)
	int32 TheirPosition = 0;

	/// Your finishing position in that race
	UPROPERTY(BlueprintReadOnly)
	int32 OurPosition = 0;

	/// True if this player is already on your friends list
	UPROPERTY(BlueprintReadOnly)
	bool bIsFriend = false;

	/// True if you have blocked this player
	UPROPERTY(BlueprintReadOnly)
	bool bIsBlocked = false;
};

// ============================================================================
// DELEGATES - Event Callbacks
// ============================================================================

/// Broadcast when the friends list is refreshed or modified
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendListUpdated, const TArray<FMGFriendData>&, Friends);

/// Broadcast when a specific friend's status changes (online, racing, etc.)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendStatusChanged, const FMGFriendData&, Friend);

/// Broadcast when a new friend request is received
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendRequestReceived, const FMGFriendRequest&, Request);

/// Broadcast when crew data is updated (level up, member changes, etc.)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewDataUpdated, const FMGCrewData&, Crew);

/// Broadcast when an invitation to join a crew is received
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewInviteReceived, const FMGCrewInvite&, Invite);

/// Broadcast when a friend invites you to join their game session
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameInviteReceived, const FMGFriendData&, FromFriend, const FString&, SessionID);

// ============================================================================
// SOCIAL SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Central subsystem for all social features in Midnight Grind
 *
 * UMGSocialSubsystem is a GameInstanceSubsystem that persists for the lifetime
 * of the game instance. It manages:
 * - Friends list with presence tracking
 * - Basic crew membership and management
 * - Game invites and join-in-progress
 * - Recent players from completed races
 *
 * @note For advanced crew features, use UMGCrewSubsystem from Crew/MGCrewSubsystem.h
 *
 * @section subsystem_access Accessing the Subsystem
 * @code
 * UGameInstance* GI = GetWorld()->GetGameInstance();
 * UMGSocialSubsystem* Social = GI->GetSubsystem<UMGSocialSubsystem>();
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSocialSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initialize the subsystem when the game instance is created */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Clean up when the game instance is destroyed */
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS - Subscribe to receive social notifications
	// ==========================================

	/// Fires when the friends list is refreshed (call RefreshFriendsList() to trigger)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendListUpdated OnFriendListUpdated;

	/// Fires when any friend's presence status changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendStatusChanged OnFriendStatusChanged;

	/// Fires when someone sends you a friend request
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendRequestReceived OnFriendRequestReceived;

	/// Fires when your crew's data changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewDataUpdated OnCrewDataUpdated;

	/// Fires when you receive a crew invitation
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewInviteReceived OnCrewInviteReceived;

	/// Fires when a friend invites you to their game session
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameInviteReceived OnGameInviteReceived;

	// ==========================================
	// FRIENDS
	// ==========================================

	/** Get friends list */
	UFUNCTION(BlueprintPure, Category = "Social|Friends")
	TArray<FMGFriendData> GetFriendsList() const { return Friends; }

	/** Get online friends */
	UFUNCTION(BlueprintPure, Category = "Social|Friends")
	TArray<FMGFriendData> GetOnlineFriends() const;

	/** Get friend by ID */
	UFUNCTION(BlueprintPure, Category = "Social|Friends")
	FMGFriendData GetFriend(const FString& PlayerID) const;

	/** Is player a friend */
	UFUNCTION(BlueprintPure, Category = "Social|Friends")
	bool IsFriend(const FString& PlayerID) const;

	/** Send friend request */
	UFUNCTION(BlueprintCallable, Category = "Social|Friends")
	void SendFriendRequest(const FString& PlayerID);

	/** Accept friend request */
	UFUNCTION(BlueprintCallable, Category = "Social|Friends")
	void AcceptFriendRequest(const FString& RequestID);

	/** Decline friend request */
	UFUNCTION(BlueprintCallable, Category = "Social|Friends")
	void DeclineFriendRequest(const FString& RequestID);

	/** Remove friend */
	UFUNCTION(BlueprintCallable, Category = "Social|Friends")
	void RemoveFriend(const FString& PlayerID);

	/** Block player */
	UFUNCTION(BlueprintCallable, Category = "Social|Friends")
	void BlockPlayer(const FString& PlayerID);

	/** Unblock player */
	UFUNCTION(BlueprintCallable, Category = "Social|Friends")
	void UnblockPlayer(const FString& PlayerID);

	/** Set friend as favorite */
	UFUNCTION(BlueprintCallable, Category = "Social|Friends")
	void SetFriendFavorite(const FString& PlayerID, bool bFavorite);

	/** Get pending friend requests */
	UFUNCTION(BlueprintPure, Category = "Social|Friends")
	TArray<FMGFriendRequest> GetPendingRequests() const;

	/** Get blocked players */
	UFUNCTION(BlueprintPure, Category = "Social|Friends")
	TArray<FString> GetBlockedPlayers() const { return BlockedPlayers; }

	/** Refresh friends list */
	UFUNCTION(BlueprintCallable, Category = "Social|Friends")
	void RefreshFriendsList();

	// ==========================================
	// CREW
	// ==========================================

	/** Is in a crew */
	UFUNCTION(BlueprintPure, Category = "Social|Crew")
	bool IsInCrew() const { return !CurrentCrew.CrewID.IsEmpty(); }

	/** Get current crew */
	UFUNCTION(BlueprintPure, Category = "Social|Crew")
	FMGCrewData GetCurrentCrew() const { return CurrentCrew; }

	/** Get player's crew rank */
	UFUNCTION(BlueprintPure, Category = "Social|Crew")
	EMGCrewRank GetCrewRank() const { return PlayerCrewRank; }

	/** Create crew */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void CreateCrew(const FString& CrewName, const FString& CrewTag, const FString& Description);

	/** Leave crew */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void LeaveCrew();

	/** Invite to crew */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void InviteToCrew(const FString& PlayerID);

	/** Accept crew invite */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void AcceptCrewInvite(const FString& InviteID);

	/** Decline crew invite */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void DeclineCrewInvite(const FString& InviteID);

	/** Kick member (requires Officer+) */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void KickCrewMember(const FString& PlayerID);

	/** Promote member (requires Leader) */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void PromoteCrewMember(const FString& PlayerID, EMGCrewRank NewRank);

	/** Update crew info (requires Leader) */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void UpdateCrewInfo(const FString& Description, bool bRecruiting);

	/** Set crew color (requires Leader) */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	void SetCrewColor(FLinearColor Color);

	/** Search crews */
	UFUNCTION(BlueprintCallable, Category = "Social|Crew")
	TArray<FMGCrewData> SearchCrews(const FString& SearchTerm) const;

	/** Get pending crew invites */
	UFUNCTION(BlueprintPure, Category = "Social|Crew")
	TArray<FMGCrewInvite> GetPendingCrewInvites() const { return PendingCrewInvites; }

	// ==========================================
	// GAME INVITES
	// ==========================================

	/** Send game invite to friend */
	UFUNCTION(BlueprintCallable, Category = "Social|Invite")
	void SendGameInvite(const FString& PlayerID, const FString& SessionID);

	/** Accept game invite */
	UFUNCTION(BlueprintCallable, Category = "Social|Invite")
	void AcceptGameInvite(const FString& SessionID);

	/** Decline game invite */
	UFUNCTION(BlueprintCallable, Category = "Social|Invite")
	void DeclineGameInvite(const FString& SessionID);

	/** Join friend's session */
	UFUNCTION(BlueprintCallable, Category = "Social|Invite")
	void JoinFriend(const FString& PlayerID);

	// ==========================================
	// RECENT PLAYERS
	// ==========================================

	/** Get recent players */
	UFUNCTION(BlueprintPure, Category = "Social|Recent")
	TArray<FMGRecentPlayer> GetRecentPlayers() const { return RecentPlayers; }

	/** Add recent player */
	UFUNCTION(BlueprintCallable, Category = "Social|Recent")
	void AddRecentPlayer(const FString& PlayerID, const FString& DisplayName, FName TrackID, int32 TheirPosition, int32 OurPosition);

	/** Clear recent players */
	UFUNCTION(BlueprintCallable, Category = "Social|Recent")
	void ClearRecentPlayers();

	// ==========================================
	// STATUS
	// ==========================================

	/** Set own status */
	UFUNCTION(BlueprintCallable, Category = "Social|Status")
	void SetStatus(EMGFriendStatus Status, const FString& StatusText = TEXT(""));

	/** Get status display text */
	UFUNCTION(BlueprintPure, Category = "Social|Status")
	static FText GetStatusDisplayText(EMGFriendStatus Status);

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Friends list */
	UPROPERTY()
	TArray<FMGFriendData> Friends;

	/** Pending friend requests */
	UPROPERTY()
	TArray<FMGFriendRequest> PendingFriendRequests;

	/** Blocked players */
	UPROPERTY()
	TArray<FString> BlockedPlayers;

	/** Current crew */
	UPROPERTY()
	FMGCrewData CurrentCrew;

	/** Player's rank in crew */
	EMGCrewRank PlayerCrewRank = EMGCrewRank::Member;

	/** Pending crew invites */
	UPROPERTY()
	TArray<FMGCrewInvite> PendingCrewInvites;

	/** Recent players */
	UPROPERTY()
	TArray<FMGRecentPlayer> RecentPlayers;

	/** Own status */
	EMGFriendStatus OwnStatus = EMGFriendStatus::Online;

	/** Max recent players to track */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	int32 MaxRecentPlayers = 50;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Load social data */
	void LoadSocialData();

	/** Save social data */
	void SaveSocialData();

	/** Generate mock friends data */
	void GenerateMockFriends();

	/** Generate mock crew data */
	void GenerateMockCrew();
};

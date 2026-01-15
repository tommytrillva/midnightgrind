// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSocialSubsystem.generated.h"

/**
 * Friend status
 */
UENUM(BlueprintType)
enum class EMGFriendStatus : uint8
{
	/** Offline */
	Offline,
	/** Online in menus */
	Online,
	/** In garage */
	InGarage,
	/** In lobby */
	InLobby,
	/** Racing */
	Racing,
	/** Watching replay */
	WatchingReplay,
	/** Away */
	Away
};

/**
 * Friend request status
 */
UENUM(BlueprintType)
enum class EMGRequestStatus : uint8
{
	/** Pending */
	Pending,
	/** Accepted */
	Accepted,
	/** Declined */
	Declined,
	/** Blocked */
	Blocked
};

/**
 * Crew rank
 */
UENUM(BlueprintType)
enum class EMGCrewRank : uint8
{
	/** Regular member */
	Member,
	/** Officer with invite permissions */
	Officer,
	/** Leader with full control */
	Leader
};

/**
 * Friend data
 */
USTRUCT(BlueprintType)
struct FMGFriendData
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/** Current status */
	UPROPERTY(BlueprintReadOnly)
	EMGFriendStatus Status = EMGFriendStatus::Offline;

	/** Status text */
	UPROPERTY(BlueprintReadOnly)
	FString StatusText;

	/** Current track (if racing) */
	UPROPERTY(BlueprintReadOnly)
	FName CurrentTrack;

	/** Session ID (if in lobby/racing) */
	UPROPERTY(BlueprintReadOnly)
	FString SessionID;

	/** Crew name */
	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	/** Profile level */
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/** Reputation */
	UPROPERTY(BlueprintReadOnly)
	int32 Reputation = 0;

	/** Total wins */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalWins = 0;

	/** Last online time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime LastOnline;

	/** Avatar ID */
	UPROPERTY(BlueprintReadOnly)
	FName AvatarID;

	/** Is favorite friend */
	UPROPERTY(BlueprintReadOnly)
	bool bIsFavorite = false;

	/** Can join their session */
	UPROPERTY(BlueprintReadOnly)
	bool bCanJoin = false;
};

/**
 * Friend request
 */
USTRUCT(BlueprintType)
struct FMGFriendRequest
{
	GENERATED_BODY()

	/** Request ID */
	UPROPERTY(BlueprintReadOnly)
	FString RequestID;

	/** Sender player ID */
	UPROPERTY(BlueprintReadOnly)
	FString SenderID;

	/** Sender display name */
	UPROPERTY(BlueprintReadOnly)
	FString SenderName;

	/** Sender level */
	UPROPERTY(BlueprintReadOnly)
	int32 SenderLevel = 1;

	/** Request timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Status */
	UPROPERTY(BlueprintReadOnly)
	EMGRequestStatus Status = EMGRequestStatus::Pending;

	/** Is incoming (vs outgoing) */
	UPROPERTY(BlueprintReadOnly)
	bool bIsIncoming = true;
};

/**
 * Crew member data
 */
USTRUCT(BlueprintType)
struct FMGCrewMember
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/** Crew rank */
	UPROPERTY(BlueprintReadOnly)
	EMGCrewRank Rank = EMGCrewRank::Member;

	/** Join date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime JoinDate;

	/** Contribution to crew XP */
	UPROPERTY(BlueprintReadOnly)
	int32 WeeklyContribution = 0;

	/** Total contribution */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalContribution = 0;

	/** Online status */
	UPROPERTY(BlueprintReadOnly)
	EMGFriendStatus Status = EMGFriendStatus::Offline;

	/** Player level */
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;
};

/**
 * Crew data
 */
USTRUCT(BlueprintType)
struct FMGCrewData
{
	GENERATED_BODY()

	/** Crew ID */
	UPROPERTY(BlueprintReadOnly)
	FString CrewID;

	/** Crew name */
	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	/** Crew tag (short) */
	UPROPERTY(BlueprintReadOnly)
	FString CrewTag;

	/** Crew description */
	UPROPERTY(BlueprintReadOnly)
	FString Description;

	/** Crew emblem ID */
	UPROPERTY(BlueprintReadOnly)
	FName EmblemID;

	/** Crew color */
	UPROPERTY(BlueprintReadOnly)
	FLinearColor CrewColor = FLinearColor::White;

	/** Crew level */
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/** Crew XP */
	UPROPERTY(BlueprintReadOnly)
	int32 CrewXP = 0;

	/** XP needed for next level */
	UPROPERTY(BlueprintReadOnly)
	int32 NextLevelXP = 1000;

	/** Member count */
	UPROPERTY(BlueprintReadOnly)
	int32 MemberCount = 0;

	/** Max members */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxMembers = 50;

	/** Total wins */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalWins = 0;

	/** Weekly ranking */
	UPROPERTY(BlueprintReadOnly)
	int32 WeeklyRank = 0;

	/** Is recruiting */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRecruiting = true;

	/** Created date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime CreatedDate;

	/** Members */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGCrewMember> Members;
};

/**
 * Crew invite
 */
USTRUCT(BlueprintType)
struct FMGCrewInvite
{
	GENERATED_BODY()

	/** Invite ID */
	UPROPERTY(BlueprintReadOnly)
	FString InviteID;

	/** Crew ID */
	UPROPERTY(BlueprintReadOnly)
	FString CrewID;

	/** Crew name */
	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	/** Inviter name */
	UPROPERTY(BlueprintReadOnly)
	FString InviterName;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;
};

/**
 * Recent player
 */
USTRUCT(BlueprintType)
struct FMGRecentPlayer
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/** When we last raced */
	UPROPERTY(BlueprintReadOnly)
	FDateTime LastRaced;

	/** Track raced on */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Their finish position */
	UPROPERTY(BlueprintReadOnly)
	int32 TheirPosition = 0;

	/** Our finish position */
	UPROPERTY(BlueprintReadOnly)
	int32 OurPosition = 0;

	/** Is already friend */
	UPROPERTY(BlueprintReadOnly)
	bool bIsFriend = false;

	/** Is blocked */
	UPROPERTY(BlueprintReadOnly)
	bool bIsBlocked = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendListUpdated, const TArray<FMGFriendData>&, Friends);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendStatusChanged, const FMGFriendData&, Friend);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFriendRequestReceived, const FMGFriendRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewDataUpdated, const FMGCrewData&, Crew);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewInviteReceived, const FMGCrewInvite&, Invite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameInviteReceived, const FMGFriendData&, FromFriend, const FString&, SessionID);

/**
 * Social Subsystem
 * Manages friends, crews, and social features
 *
 * Features:
 * - Friends list management
 * - Crew system
 * - Game invites
 * - Recent players
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSocialSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendListUpdated OnFriendListUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendStatusChanged OnFriendStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFriendRequestReceived OnFriendRequestReceived;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewDataUpdated OnCrewDataUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewInviteReceived OnCrewInviteReceived;

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

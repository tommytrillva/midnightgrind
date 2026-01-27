// Copyright Midnight Grind. All Rights Reserved.

// MidnightGrind - Arcade Street Racing Game
// Party Invite Subsystem - Party invitations, session joining, social matchmaking

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPartyInviteSubsystem.generated.h"

// Forward declarations
class UMGPartyInviteSubsystem;

/**
 * EMGInviteStatus - Status of an invitation
 */
UENUM(BlueprintType)
enum class EMGInviteStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending"),
    Accepted        UMETA(DisplayName = "Accepted"),
    Declined        UMETA(DisplayName = "Declined"),
    Expired         UMETA(DisplayName = "Expired"),
    Cancelled       UMETA(DisplayName = "Cancelled"),
    Error           UMETA(DisplayName = "Error")
};

/**
 * EMGPartyState - State of the party
 */
UENUM(BlueprintType)
enum class EMGPartyState : uint8
{
    None            UMETA(DisplayName = "None"),
    Forming         UMETA(DisplayName = "Forming"),
    Ready           UMETA(DisplayName = "Ready"),
    InMatchmaking   UMETA(DisplayName = "In Matchmaking"),
    InSession       UMETA(DisplayName = "In Session"),
    Disbanded       UMETA(DisplayName = "Disbanded")
};

/**
 * EMGPartyRole - Roles within a party
 */
UENUM(BlueprintType)
enum class EMGPartyRole : uint8
{
    Member          UMETA(DisplayName = "Member"),
    Leader          UMETA(DisplayName = "Leader"),
    Moderator       UMETA(DisplayName = "Moderator")
};

/**
 * EMGJoinability - Party joinability settings
 */
UENUM(BlueprintType)
enum class EMGJoinability : uint8
{
    Open            UMETA(DisplayName = "Open"),
    FriendsOnly     UMETA(DisplayName = "Friends Only"),
    InviteOnly      UMETA(DisplayName = "Invite Only"),
    Closed          UMETA(DisplayName = "Closed")
};

/**
 * EMGInviteSource - Source of an invitation
 */
UENUM(BlueprintType)
enum class EMGInviteSource : uint8
{
    Direct          UMETA(DisplayName = "Direct Invite"),
    Platform        UMETA(DisplayName = "Platform Invite"),
    GameMenu        UMETA(DisplayName = "Game Menu"),
    RecentPlayer    UMETA(DisplayName = "Recent Player"),
    JoinInProgress  UMETA(DisplayName = "Join In Progress"),
    Link            UMETA(DisplayName = "Invite Link")
};

/**
 * FMGPartyMember - A member of a party
 */
USTRUCT(BlueprintType)
struct FMGPartyMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AvatarUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPartyRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInGame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSpeaking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SelectedVehicle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlatformType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime JoinedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Latency;

    FMGPartyMember()
        : Role(EMGPartyRole::Member)
        , bIsReady(false)
        , bIsInGame(false)
        , bIsSpeaking(false)
        , SelectedVehicle(NAME_None)
        , Level(1)
        , SkillRating(1000)
        , Latency(0.0f)
    {}
};

/**
 * FMGPartyInvitation - An invitation to join a party
 */
USTRUCT(BlueprintType)
struct FMGPartyInvitation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InviteId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PartyId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SenderId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SenderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ReceiverId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInviteStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInviteSource Source;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime SentAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ExpiresAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PartySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PartyMaxSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentActivity;

    FMGPartyInvitation()
        : Status(EMGInviteStatus::Pending)
        , Source(EMGInviteSource::Direct)
        , PartySize(1)
        , PartyMaxSize(4)
        , CurrentActivity(NAME_None)
    {}

    bool IsExpired() const
    {
        return FDateTime::Now() > ExpiresAt;
    }

    bool CanAccept() const
    {
        return Status == EMGInviteStatus::Pending && !IsExpired() && PartySize < PartyMaxSize;
    }
};

/**
 * FMGPartyData - Complete party information
 */
USTRUCT(BlueprintType)
struct FMGPartyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PartyId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LeaderId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPartyState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGJoinability Joinability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGPartyMember> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SelectedGameMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SelectedTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SessionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowCrossPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> PartySettings;

    FMGPartyData()
        : State(EMGPartyState::None)
        , Joinability(EMGJoinability::FriendsOnly)
        , MaxSize(4)
        , CurrentActivity(NAME_None)
        , SelectedGameMode(NAME_None)
        , SelectedTrack(NAME_None)
        , bAllowCrossPlay(true)
    {}

    int32 GetMemberCount() const { return Members.Num(); }
    bool IsFull() const { return Members.Num() >= MaxSize; }
    bool IsLeader(const FString& PlayerId) const { return LeaderId == PlayerId; }
};

/**
 * FMGJoinRequest - Request to join a party or session
 */
USTRUCT(BlueprintType)
struct FMGJoinRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequesterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequesterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetPartyId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInviteStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime RequestedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequesterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequesterSkillRating;

    FMGJoinRequest()
        : Status(EMGInviteStatus::Pending)
        , RequesterLevel(1)
        , RequesterSkillRating(1000)
    {}
};

/**
 * FMGRecentPlayer - A recently played with player
 */
USTRUCT(BlueprintType)
struct FMGRecentPlayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AvatarUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastPlayedWith;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName LastGameMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesPlayedWith;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFriend;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOnline;

    FMGRecentPlayer()
        : TimesPlayedWith(1)
        , bIsFriend(false)
        , bIsBlocked(false)
        , bIsOnline(false)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPartyCreated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPartyDisbanded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPartyJoined, const FMGPartyData&, Party);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPartyLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMemberJoined, const FMGPartyMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMemberLeft, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMemberUpdated, const FMGPartyMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnLeaderChanged, const FString&, NewLeaderId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnInviteReceived, const FMGPartyInvitation&, Invitation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnInviteStatusChanged, const FString&, InviteId, EMGInviteStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnJoinRequestReceived, const FMGJoinRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPartyStateChanged, EMGPartyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPartyError, const FString&, ErrorMessage);

/**
 * UMGPartyInviteSubsystem
 *
 * Manages party system and invitations for Midnight Grind.
 * Features include:
 * - Party creation and management
 * - Sending and receiving invitations
 * - Join-in-progress support
 * - Recent players tracking
 * - Party settings and joinability
 * - Cross-platform party support
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPartyInviteSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGPartyInviteSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ===== Party Management =====

    UFUNCTION(BlueprintCallable, Category = "Party|Management")
    bool CreateParty(int32 MaxSize = 4);

    UFUNCTION(BlueprintCallable, Category = "Party|Management")
    void DisbandParty();

    UFUNCTION(BlueprintCallable, Category = "Party|Management")
    void LeaveParty();

    UFUNCTION(BlueprintPure, Category = "Party|Management")
    bool IsInParty() const;

    UFUNCTION(BlueprintPure, Category = "Party|Management")
    bool IsPartyLeader() const;

    UFUNCTION(BlueprintPure, Category = "Party|Management")
    FMGPartyData GetCurrentParty() const;

    UFUNCTION(BlueprintPure, Category = "Party|Management")
    TArray<FMGPartyMember> GetPartyMembers() const;

    UFUNCTION(BlueprintPure, Category = "Party|Management")
    FMGPartyMember GetPartyMember(const FString& PlayerId) const;

    UFUNCTION(BlueprintPure, Category = "Party|Management")
    int32 GetPartySize() const;

    UFUNCTION(BlueprintPure, Category = "Party|Management")
    bool IsPartyFull() const;

    // ===== Party Settings =====

    UFUNCTION(BlueprintCallable, Category = "Party|Settings")
    void SetJoinability(EMGJoinability Joinability);

    UFUNCTION(BlueprintPure, Category = "Party|Settings")
    EMGJoinability GetJoinability() const;

    UFUNCTION(BlueprintCallable, Category = "Party|Settings")
    void SetMaxPartySize(int32 MaxSize);

    UFUNCTION(BlueprintCallable, Category = "Party|Settings")
    void SetPartySetting(const FString& Key, const FString& Value);

    UFUNCTION(BlueprintPure, Category = "Party|Settings")
    FString GetPartySetting(const FString& Key) const;

    UFUNCTION(BlueprintCallable, Category = "Party|Settings")
    void SetSelectedGameMode(FName GameMode);

    UFUNCTION(BlueprintCallable, Category = "Party|Settings")
    void SetSelectedTrack(FName TrackId);

    UFUNCTION(BlueprintCallable, Category = "Party|Settings")
    void SetCrossPlayEnabled(bool bEnabled);

    // ===== Member Management =====

    UFUNCTION(BlueprintCallable, Category = "Party|Members")
    void KickMember(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Party|Members")
    void PromoteToLeader(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Party|Members")
    void SetMemberRole(const FString& PlayerId, EMGPartyRole Role);

    UFUNCTION(BlueprintCallable, Category = "Party|Members")
    void SetLocalReady(bool bReady);

    UFUNCTION(BlueprintPure, Category = "Party|Members")
    bool IsLocalReady() const;

    UFUNCTION(BlueprintPure, Category = "Party|Members")
    bool AreAllMembersReady() const;

    UFUNCTION(BlueprintCallable, Category = "Party|Members")
    void SetLocalVehicle(FName VehicleId);

    // ===== Invitations =====

    UFUNCTION(BlueprintCallable, Category = "Party|Invites")
    bool SendInvite(const FString& PlayerId, FText Message = FText());

    UFUNCTION(BlueprintCallable, Category = "Party|Invites")
    bool SendInviteToFriends(const TArray<FString>& PlayerIds);

    UFUNCTION(BlueprintCallable, Category = "Party|Invites")
    void AcceptInvite(const FString& InviteId);

    UFUNCTION(BlueprintCallable, Category = "Party|Invites")
    void DeclineInvite(const FString& InviteId);

    UFUNCTION(BlueprintCallable, Category = "Party|Invites")
    void CancelInvite(const FString& InviteId);

    UFUNCTION(BlueprintPure, Category = "Party|Invites")
    TArray<FMGPartyInvitation> GetPendingInvites() const;

    UFUNCTION(BlueprintPure, Category = "Party|Invites")
    TArray<FMGPartyInvitation> GetSentInvites() const;

    UFUNCTION(BlueprintPure, Category = "Party|Invites")
    int32 GetPendingInviteCount() const;

    UFUNCTION(BlueprintCallable, Category = "Party|Invites")
    void ClearExpiredInvites();

    // ===== Join Requests =====

    UFUNCTION(BlueprintCallable, Category = "Party|JoinRequests")
    bool RequestToJoin(const FString& PartyId, FText Message = FText());

    UFUNCTION(BlueprintCallable, Category = "Party|JoinRequests")
    void ApproveJoinRequest(const FString& RequestId);

    UFUNCTION(BlueprintCallable, Category = "Party|JoinRequests")
    void DenyJoinRequest(const FString& RequestId);

    UFUNCTION(BlueprintPure, Category = "Party|JoinRequests")
    TArray<FMGJoinRequest> GetPendingJoinRequests() const;

    // ===== Join In Progress =====

    UFUNCTION(BlueprintCallable, Category = "Party|Join")
    void JoinParty(const FString& PartyId);

    UFUNCTION(BlueprintCallable, Category = "Party|Join")
    void JoinFriend(const FString& FriendId);

    UFUNCTION(BlueprintCallable, Category = "Party|Join")
    void JoinFromInviteLink(const FString& InviteLink);

    UFUNCTION(BlueprintPure, Category = "Party|Join")
    bool CanJoinParty(const FString& PartyId) const;

    UFUNCTION(BlueprintCallable, Category = "Party|Join")
    FString GenerateInviteLink();

    // ===== Recent Players =====

    UFUNCTION(BlueprintCallable, Category = "Party|RecentPlayers")
    void AddRecentPlayer(const FMGRecentPlayer& Player);

    UFUNCTION(BlueprintPure, Category = "Party|RecentPlayers")
    TArray<FMGRecentPlayer> GetRecentPlayers(int32 MaxCount = 50) const;

    UFUNCTION(BlueprintCallable, Category = "Party|RecentPlayers")
    void ClearRecentPlayers();

    // ===== Platform Integration =====

    UFUNCTION(BlueprintCallable, Category = "Party|Platform")
    void ShowPlatformInviteUI();

    UFUNCTION(BlueprintCallable, Category = "Party|Platform")
    void RegisterForPlatformInvites();

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnPartyCreated OnPartyCreated;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnPartyDisbanded OnPartyDisbanded;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnPartyJoined OnPartyJoined;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnPartyLeft OnPartyLeft;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnMemberJoined OnMemberJoined;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnMemberLeft OnMemberLeft;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnMemberUpdated OnMemberUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnLeaderChanged OnLeaderChanged;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnInviteReceived OnInviteReceived;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnInviteStatusChanged OnInviteStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnJoinRequestReceived OnJoinRequestReceived;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnPartyStateChanged OnPartyStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Party|Events")
    FMGOnPartyError OnPartyError;

protected:
    void UpdatePartyState(EMGPartyState NewState);
    FString GenerateUniqueId();
    void ProcessPlatformInvite(const FString& InviteData);

private:
    // Current party
    UPROPERTY()
    FMGPartyData CurrentParty;

    // Local player ID
    UPROPERTY()
    FString LocalPlayerId;

    // Received invitations
    UPROPERTY()
    TArray<FMGPartyInvitation> ReceivedInvites;

    // Sent invitations
    UPROPERTY()
    TArray<FMGPartyInvitation> SentInvites;

    // Join requests (when leader)
    UPROPERTY()
    TArray<FMGJoinRequest> JoinRequests;

    // Recent players
    UPROPERTY()
    TArray<FMGRecentPlayer> RecentPlayers;

    // Local ready state
    bool bLocalReady;

    // Max recent players to track
    static constexpr int32 MaxRecentPlayers = 100;
};

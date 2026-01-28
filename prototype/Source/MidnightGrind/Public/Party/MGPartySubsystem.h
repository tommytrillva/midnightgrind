// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPartySubsystem.h
 * @brief Multiplayer Party System for Group Play and Matchmaking
 *
 * @section overview Overview
 * This subsystem manages multiplayer parties - groups of players who want to
 * play together. It handles party creation, invites, voice chat, ready states,
 * and matchmaking queue integration. Think of it like Discord's party system
 * or any modern multiplayer game's squad/fireteam feature.
 *
 * @section key_concepts Key Concepts for Beginners
 *
 * 1. WHAT IS A PARTY?
 *    A party is a group of players (1-8) who:
 *    - Can chat via voice together
 *    - Queue for matches together
 *    - Stay together between races
 *    - Share game mode preferences
 *
 *    Example: You and 3 friends form a party to race as a team
 *
 * 2. PARTY LIFECYCLE:
 *    @code
 *    CreateParty() --> State: Forming
 *          |
 *          v
 *    InvitePlayer() / AcceptInvite()
 *          |
 *          v
 *    All Members Ready --> State: Ready
 *          |
 *          v
 *    StartQueue() --> State: InQueue
 *          |
 *          v
 *    Match Found --> State: InMatch
 *          |
 *          v
 *    Match Ends --> State: PostMatch --> State: Ready (loop)
 *          |
 *          v
 *    DisbandParty() or LeaveParty() --> State: None
 *    @endcode
 *
 * 3. PARTY ROLES (EMGPartyRole):
 *    - Member: Regular participant, can ready up and vote
 *    - Moderator: Can kick members, approve join requests
 *    - Leader: Full control - change settings, start queue, disband
 *
 * 4. PARTY PRIVACY (EMGPartyPrivacy):
 *    - Open: Anyone can join directly
 *    - FriendsOnly: Only friends can join/see the party
 *    - InviteOnly: Must be invited by a member
 *    - Closed: No new members allowed
 *
 * 5. INVITE SYSTEM:
 *    Party invites flow:
 *    @code
 *    Leader calls InvitePlayer("Player2")
 *          |
 *          v
 *    Server sends FMGPartyInvite to Player2
 *          |
 *          v
 *    Player2's OnPartyInviteReceived fires
 *          |
 *          v
 *    Player2 calls AcceptInvite() or DeclineInvite()
 *          |
 *          v
 *    OnPartyMemberJoined fires for everyone
 *    @endcode
 *
 * 6. READY SYSTEM:
 *    Before queuing, all members must be ready:
 *    - SetReady(true) marks you as ready
 *    - AreAllMembersReady() checks if queue can start
 *    - OnPartyReady fires when everyone is ready
 *    - Leader can then call StartQueue()
 *
 * 7. VOICE CHAT INTEGRATION:
 *    Party includes voice communication:
 *    - JoinVoiceChannel() connects to party voice
 *    - SetMuted(true) mutes your microphone
 *    - SetDeafened(true) mutes all incoming audio
 *    - SetMemberVolume() adjusts individual member volume
 *    - GetSpeakingMembers() shows who's currently talking
 *
 * 8. PARTY SETTINGS (FMGPartySettings):
 *    Configurable by the party leader:
 *    - Privacy: Who can join
 *    - MaxSize: Maximum party members (1-8)
 *    - bAutoReady: Auto-ready when joining
 *    - bRequireReadyToQueue: Must all be ready to queue
 *    - PreferredGameMode: What mode to queue for
 *    - PreferredRegion: Server region preference
 *    - bEnableVoiceChat: Voice chat on/off
 *    - bPushToTalk: PTT vs open mic
 *
 * 9. PARTY MEMBER DATA (FMGPartyMember):
 *    Information about each party member:
 *    - PlayerID, DisplayName: Identity
 *    - Role: Member/Moderator/Leader
 *    - bIsReady: Ready state
 *    - SelectedVehicleID: Their chosen car
 *    - RankTier, RankPoints: Competitive rank
 *    - VoiceState: Voice chat status
 *    - bIsSpeaking: Currently talking
 *
 * 10. JOIN REQUESTS:
 *     For non-open parties, players can request to join:
 *     - RequestToJoin() sends a request
 *     - OnJoinRequestReceived fires for leader/mods
 *     - ApproveJoinRequest() or DenyJoinRequest()
 *
 * @section usage_examples Code Examples
 *
 * @code
 * // Get the party subsystem
 * UMGPartySubsystem* Party = GetGameInstance()->GetSubsystem<UMGPartySubsystem>();
 *
 * // Set up local player info (call after login)
 * Party->SetLocalPlayerInfo(
 *     FName("Player_12345"),
 *     TEXT("SpeedDemon"),
 *     42  // Level
 * );
 *
 * // Create a new party
 * FMGPartySettings Settings;
 * Settings.Privacy = EMGPartyPrivacy::FriendsOnly;
 * Settings.MaxSize = 4;
 * Settings.bEnableVoiceChat = true;
 * Settings.PreferredGameMode = FName("CircuitRace");
 *
 * FGuid PartyID = Party->CreateParty(Settings);
 *
 * // Invite a friend
 * FGuid InviteID = Party->InvitePlayer(
 *     FName("Friend_67890"),
 *     FText::FromString(TEXT("Join my party for some races!"))
 * );
 *
 * // Check pending invites you've received
 * TArray<FMGPartyInvite> Invites = Party->GetPendingInvites();
 * for (const FMGPartyInvite& Invite : Invites)
 * {
 *     ShowInviteNotification(Invite.SenderName, Invite.PartyMemberCount);
 * }
 *
 * // Accept an invite
 * Party->AcceptInvite(Invites[0].InviteID);
 *
 * // Join voice chat
 * Party->JoinVoiceChannel();
 *
 * // Set ready state
 * Party->SetSelectedVehicle(FName("Nissan_Skyline_R34"));
 * Party->SetReady(true);
 *
 * // Check if everyone is ready
 * if (Party->IsPartyLeader() && Party->AreAllMembersReady())
 * {
 *     // Start matchmaking
 *     Party->StartQueue(FName("CircuitRace"));
 * }
 *
 * // Get party members for UI
 * TArray<FMGPartyMember> Members = Party->GetPartyMembers();
 * for (const FMGPartyMember& Member : Members)
 * {
 *     UpdatePartySlotUI(Member.DisplayName, Member.bIsReady, Member.Role);
 * }
 *
 * // Listen for party events
 * Party->OnPartyInviteReceived.AddDynamic(this, &MyClass::HandleInviteReceived);
 * Party->OnPartyMemberJoined.AddDynamic(this, &MyClass::HandleMemberJoined);
 * Party->OnPartyMemberLeft.AddDynamic(this, &MyClass::HandleMemberLeft);
 * Party->OnPartyStateChanged.AddDynamic(this, &MyClass::HandleStateChanged);
 * Party->OnPartyReady.AddDynamic(this, &MyClass::HandlePartyReady);
 * Party->OnVoiceStateChanged.AddDynamic(this, &MyClass::HandleVoiceChanged);
 *
 * // Leave the party
 * Party->LeaveParty();
 *
 * // Or if you're the leader, disband it
 * Party->DisbandParty();
 * @endcode
 *
 * @section events_reference Events Reference
 * - OnPartyCreated: You created a new party
 * - OnPartyDisbanded: Party was disbanded
 * - OnPartyJoined: You joined a party
 * - OnPartyLeft: You left a party
 * - OnPartyMemberJoined: Someone joined your party
 * - OnPartyMemberLeft: Someone left your party
 * - OnPartyMemberUpdated: Member data changed (ready state, vehicle, etc.)
 * - OnPartyLeaderChanged: New party leader assigned
 * - OnPartyInviteReceived: You received a party invite
 * - OnPartyInviteResponse: Someone responded to your invite
 * - OnJoinRequestReceived: Someone wants to join (leaders/mods)
 * - OnPartyStateChanged: Party state machine transition
 * - OnVoiceStateChanged: Voice chat status changed
 * - OnPartyReady: All members are ready
 *
 * @see UMGMatchmakingSubsystem - Handles the actual matchmaking queue
 * @see UMGFriendsSubsystem - Friend list for inviting
 * @see UMGVoiceChatSubsystem - Voice chat implementation
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPartySubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGPartyState : uint8
{
	None,
	Forming,
	Ready,
	InQueue,
	InMatch,
	PostMatch
};

UENUM(BlueprintType)
enum class EMGPartyRole : uint8
{
	Member,
	Moderator,
	Leader
};

UENUM(BlueprintType)
enum class EMGPartyPrivacy : uint8
{
	Open,
	FriendsOnly,
	InviteOnly,
	Closed
};

UENUM(BlueprintType)
enum class EMGPartyInviteState : uint8
{
	Pending,
	Accepted,
	Declined,
	Expired,
	Cancelled
};

UENUM(BlueprintType)
enum class EMGVoiceState : uint8
{
	Disconnected,
	Connecting,
	Connected,
	Muted,
	Deafened
};

USTRUCT(BlueprintType)
struct FMGPartyMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPartyRole Role = EMGPartyRole::Member;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SelectedVehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentActivity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RankPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RankTier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVoiceState VoiceState = EMGVoiceState::Disconnected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VoiceVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSpeaking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime JoinedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Avatar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TeamColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMGPartyInvite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InviteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PartyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SenderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RecipientID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPartyInviteState State = EMGPartyInviteState::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SentAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CustomMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PartyMemberCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PartyMaxSize = 4;
};

USTRUCT(BlueprintType)
struct FMGPartySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPartyPrivacy Privacy = EMGPartyPrivacy::FriendsOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSize = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowSpectators = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireReadyToQueue = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PreferredGameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PreferredRegion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableVoiceChat = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPushToTalk = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InviteExpirationMinutes = 5.0f;
};

USTRUCT(BlueprintType)
struct FMGParty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PartyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPartyState State = EMGPartyState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPartyMember> Members;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LeaderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPartySettings Settings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentMatchID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VoiceChannelID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PartyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WinsThisSession = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacesThisSession = 0;
};

USTRUCT(BlueprintType)
struct FMGJoinRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RequestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RequestedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Avatar;
};

USTRUCT(BlueprintType)
struct FMGPartyActivity
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActivityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ActivityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ActivityDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPlayers = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSupportsRanked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ActivityIcon;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyCreated, const FMGParty&, Party);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPartyDisbanded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyJoined, const FMGParty&, Party);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPartyLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyMemberJoined, const FMGPartyMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyMemberLeft, FName, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyMemberUpdated, const FMGPartyMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyLeaderChanged, FName, NewLeaderID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyInviteReceived, const FMGPartyInvite&, Invite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartyInviteResponse, FGuid, InviteID, bool, bAccepted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinRequestReceived, const FMGJoinRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyStateChanged, EMGPartyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceStateChanged, FName, PlayerID, EMGVoiceState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPartyReady);

UCLASS()
class MIDNIGHTGRIND_API UMGPartySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Party Creation
	UFUNCTION(BlueprintCallable, Category = "Party|Create")
	FGuid CreateParty(const FMGPartySettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Party|Create")
	bool DisbandParty();

	UFUNCTION(BlueprintPure, Category = "Party|Create")
	bool IsInParty() const;

	UFUNCTION(BlueprintPure, Category = "Party|Create")
	bool IsPartyLeader() const;

	UFUNCTION(BlueprintPure, Category = "Party|Create")
	FMGParty GetCurrentParty() const { return CurrentParty; }

	// Invites
	UFUNCTION(BlueprintCallable, Category = "Party|Invite")
	FGuid InvitePlayer(FName PlayerID, const FText& CustomMessage = FText::GetEmpty());

	UFUNCTION(BlueprintCallable, Category = "Party|Invite")
	bool CancelInvite(FGuid InviteID);

	UFUNCTION(BlueprintCallable, Category = "Party|Invite")
	bool AcceptInvite(FGuid InviteID);

	UFUNCTION(BlueprintCallable, Category = "Party|Invite")
	bool DeclineInvite(FGuid InviteID);

	UFUNCTION(BlueprintPure, Category = "Party|Invite")
	TArray<FMGPartyInvite> GetPendingInvites() const;

	UFUNCTION(BlueprintPure, Category = "Party|Invite")
	TArray<FMGPartyInvite> GetSentInvites() const;

	// Joining
	UFUNCTION(BlueprintCallable, Category = "Party|Join")
	bool JoinParty(FGuid PartyID);

	UFUNCTION(BlueprintCallable, Category = "Party|Join")
	bool RequestToJoin(FGuid PartyID, const FText& Message = FText::GetEmpty());

	UFUNCTION(BlueprintCallable, Category = "Party|Join")
	bool ApproveJoinRequest(FGuid RequestID);

	UFUNCTION(BlueprintCallable, Category = "Party|Join")
	bool DenyJoinRequest(FGuid RequestID);

	UFUNCTION(BlueprintCallable, Category = "Party|Join")
	bool LeaveParty();

	UFUNCTION(BlueprintPure, Category = "Party|Join")
	TArray<FMGJoinRequest> GetPendingJoinRequests() const;

	// Member Management
	UFUNCTION(BlueprintCallable, Category = "Party|Members")
	bool KickMember(FName PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Party|Members")
	bool PromoteToLeader(FName PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Party|Members")
	bool SetMemberRole(FName PlayerID, EMGPartyRole NewRole);

	UFUNCTION(BlueprintPure, Category = "Party|Members")
	TArray<FMGPartyMember> GetPartyMembers() const;

	UFUNCTION(BlueprintPure, Category = "Party|Members")
	FMGPartyMember GetMember(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Party|Members")
	FMGPartyMember GetLocalMember() const;

	UFUNCTION(BlueprintPure, Category = "Party|Members")
	int32 GetMemberCount() const;

	UFUNCTION(BlueprintPure, Category = "Party|Members")
	bool IsMemberReady(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Party|Members")
	bool AreAllMembersReady() const;

	// Ready State
	UFUNCTION(BlueprintCallable, Category = "Party|Ready")
	void SetReady(bool bReady);

	UFUNCTION(BlueprintPure, Category = "Party|Ready")
	bool IsReady() const;

	UFUNCTION(BlueprintCallable, Category = "Party|Ready")
	void ToggleReady();

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Party|Settings")
	bool UpdatePartySettings(const FMGPartySettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Party|Settings")
	FMGPartySettings GetPartySettings() const;

	UFUNCTION(BlueprintCallable, Category = "Party|Settings")
	bool SetPrivacy(EMGPartyPrivacy Privacy);

	UFUNCTION(BlueprintCallable, Category = "Party|Settings")
	bool SetMaxSize(int32 MaxSize);

	UFUNCTION(BlueprintCallable, Category = "Party|Settings")
	bool SetPreferredGameMode(FName GameModeID);

	// Voice Chat
	UFUNCTION(BlueprintCallable, Category = "Party|Voice")
	bool JoinVoiceChannel();

	UFUNCTION(BlueprintCallable, Category = "Party|Voice")
	void LeaveVoiceChannel();

	UFUNCTION(BlueprintCallable, Category = "Party|Voice")
	void SetMuted(bool bMuted);

	UFUNCTION(BlueprintCallable, Category = "Party|Voice")
	void SetDeafened(bool bDeafened);

	UFUNCTION(BlueprintCallable, Category = "Party|Voice")
	void SetMemberVolume(FName PlayerID, float Volume);

	UFUNCTION(BlueprintPure, Category = "Party|Voice")
	EMGVoiceState GetVoiceState() const;

	UFUNCTION(BlueprintPure, Category = "Party|Voice")
	bool IsInVoiceChannel() const;

	UFUNCTION(BlueprintPure, Category = "Party|Voice")
	TArray<FName> GetSpeakingMembers() const;

	// Queue/Match
	UFUNCTION(BlueprintCallable, Category = "Party|Queue")
	bool StartQueue(FName GameModeID);

	UFUNCTION(BlueprintCallable, Category = "Party|Queue")
	bool CancelQueue();

	UFUNCTION(BlueprintPure, Category = "Party|Queue")
	bool IsInQueue() const;

	UFUNCTION(BlueprintPure, Category = "Party|Queue")
	float GetQueueTime() const;

	// Local Player
	UFUNCTION(BlueprintCallable, Category = "Party|Player")
	void SetLocalPlayerInfo(FName PlayerID, const FString& DisplayName, int32 Level);

	UFUNCTION(BlueprintCallable, Category = "Party|Player")
	void SetSelectedVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Party|Player")
	void UpdateActivity(const FString& Activity);

	// Activities
	UFUNCTION(BlueprintPure, Category = "Party|Activities")
	TArray<FMGPartyActivity> GetAvailableActivities() const;

	UFUNCTION(BlueprintCallable, Category = "Party|Activities")
	bool SetPartyActivity(FName ActivityType);

	// Network
	UFUNCTION(BlueprintCallable, Category = "Party|Network")
	void ReceivePartyUpdate(const FMGParty& PartyData);

	UFUNCTION(BlueprintCallable, Category = "Party|Network")
	void ReceiveInvite(const FMGPartyInvite& Invite);

	UFUNCTION(BlueprintCallable, Category = "Party|Network")
	void ReceiveJoinRequest(const FMGJoinRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Party|Network")
	void ReceiveMemberUpdate(const FMGPartyMember& Member);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyCreated OnPartyCreated;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyDisbanded OnPartyDisbanded;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyJoined OnPartyJoined;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyLeft OnPartyLeft;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyMemberJoined OnPartyMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyMemberLeft OnPartyMemberLeft;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyMemberUpdated OnPartyMemberUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyLeaderChanged OnPartyLeaderChanged;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyInviteReceived OnPartyInviteReceived;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyInviteResponse OnPartyInviteResponse;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnJoinRequestReceived OnJoinRequestReceived;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyStateChanged OnPartyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnVoiceStateChanged OnVoiceStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Party|Events")
	FOnPartyReady OnPartyReady;

protected:
	void OnPartyTick();
	void CheckExpiredInvites();
	void UpdateMemberPresence();
	void BroadcastPartyUpdate();
	FMGPartyMember CreateLocalMember() const;
	void SetPartyState(EMGPartyState NewState);
	int32 FindMemberIndex(FName PlayerID) const;

	UPROPERTY()
	FMGParty CurrentParty;

	UPROPERTY()
	TArray<FMGPartyInvite> PendingInvites;

	UPROPERTY()
	TArray<FMGPartyInvite> SentInvites;

	UPROPERTY()
	TArray<FMGJoinRequest> JoinRequests;

	UPROPERTY()
	TArray<FMGPartyActivity> AvailableActivities;

	UPROPERTY()
	FName LocalPlayerID;

	UPROPERTY()
	FString LocalPlayerName;

	UPROPERTY()
	int32 LocalPlayerLevel = 1;

	UPROPERTY()
	FName LocalSelectedVehicle;

	UPROPERTY()
	FString LocalActivity;

	UPROPERTY()
	EMGVoiceState LocalVoiceState = EMGVoiceState::Disconnected;

	UPROPERTY()
	bool bLocalReady = false;

	UPROPERTY()
	float QueueStartTime = 0.0f;

	FTimerHandle PartyTickHandle;
};

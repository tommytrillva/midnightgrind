// Copyright Midnight Grind. All Rights Reserved.

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

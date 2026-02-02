// Copyright Midnight Grind. All Rights Reserved.

// MidnightGrind - Arcade Street Racing Game
// Party Invite Subsystem Implementation

#include "PartyInvite/MGPartyInviteSubsystem.h"
#include "Misc/Guid.h"

UMGPartyInviteSubsystem::UMGPartyInviteSubsystem()
    : bLocalReady(false)
{
}

void UMGPartyInviteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Generate a local player ID (in real implementation, get from platform)
    LocalPlayerId = GenerateUniqueId();

    // Register for platform invites
    RegisterForPlatformInvites();

    UE_LOG(LogTemp, Log, TEXT("MGPartyInviteSubsystem initialized"));
}

void UMGPartyInviteSubsystem::Deinitialize()
{
    // Leave party on shutdown
    if (IsInParty())
    {
        LeaveParty();
    }

    Super::Deinitialize();
}

// ===== Party Management =====

bool UMGPartyInviteSubsystem::CreateParty(int32 MaxSize)
{
    if (IsInParty())
    {
        OnPartyError.Broadcast(TEXT("Already in a party"));
        return false;
    }

    CurrentParty = FMGPartyData();
    CurrentParty.PartyId = GenerateUniqueId();
    CurrentParty.LeaderId = LocalPlayerId;
    CurrentParty.State = EMGPartyState::Forming;
    CurrentParty.MaxSize = FMath::Clamp(MaxSize, 2, 8);
    CurrentParty.CreatedAt = FDateTime::Now();

    // Add local player as first member
    FMGPartyMember LocalMember;
    LocalMember.PlayerId = LocalPlayerId;
    LocalMember.DisplayName = TEXT("Local Player");
    LocalMember.Role = EMGPartyRole::Leader;
    LocalMember.JoinedAt = FDateTime::Now();
    CurrentParty.Members.Add(LocalMember);

    OnPartyCreated.Broadcast();
    UpdatePartyState(EMGPartyState::Ready);

    UE_LOG(LogTemp, Log, TEXT("Party created: %s"), *CurrentParty.PartyId);
    return true;
}

void UMGPartyInviteSubsystem::DisbandParty()
{
    if (!IsInParty() || !IsPartyLeader())
    {
        return;
    }

    // Clear invites
    SentInvites.Empty();
    JoinRequests.Empty();

    // Clear party
    CurrentParty = FMGPartyData();
    bLocalReady = false;

    OnPartyDisbanded.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Party disbanded"));
}

void UMGPartyInviteSubsystem::LeaveParty()
{
    if (!IsInParty())
    {
        return;
    }

    FString PartyId = CurrentParty.PartyId;
    bool bWasLeader = IsPartyLeader();

    // Remove self from party
    CurrentParty.Members.RemoveAll([this](const FMGPartyMember& Member) {
        return Member.PlayerId == LocalPlayerId;
    });

    // If leader and members remain, promote next member
    if (bWasLeader && CurrentParty.Members.Num() > 0)
    {
        CurrentParty.LeaderId = CurrentParty.Members[0].PlayerId;
        CurrentParty.Members[0].Role = EMGPartyRole::Leader;
        OnLeaderChanged.Broadcast(CurrentParty.LeaderId);
    }

    // Clear local party data
    CurrentParty = FMGPartyData();
    bLocalReady = false;
    SentInvites.Empty();

    OnPartyLeft.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Left party: %s"), *PartyId);
}

bool UMGPartyInviteSubsystem::IsInParty() const
{
    return !CurrentParty.PartyId.IsEmpty();
}

bool UMGPartyInviteSubsystem::IsPartyLeader() const
{
    return IsInParty() && CurrentParty.LeaderId == LocalPlayerId;
}

FMGPartyData UMGPartyInviteSubsystem::GetCurrentParty() const
{
    return CurrentParty;
}

TArray<FMGPartyMember> UMGPartyInviteSubsystem::GetPartyMembers() const
{
    return CurrentParty.Members;
}

FMGPartyMember UMGPartyInviteSubsystem::GetPartyMember(const FString& PlayerId) const
{
    for (const FMGPartyMember& Member : CurrentParty.Members)
    {
        if (Member.PlayerId == PlayerId)
        {
            return Member;
        }
    }
    return FMGPartyMember();
}

int32 UMGPartyInviteSubsystem::GetPartySize() const
{
    return CurrentParty.Members.Num();
}

bool UMGPartyInviteSubsystem::IsPartyFull() const
{
    return CurrentParty.IsFull();
}

// ===== Party Settings =====

void UMGPartyInviteSubsystem::SetJoinability(EMGJoinability Joinability)
{
    if (!IsPartyLeader()) return;
    CurrentParty.Joinability = Joinability;
}

EMGJoinability UMGPartyInviteSubsystem::GetJoinability() const
{
    return CurrentParty.Joinability;
}

void UMGPartyInviteSubsystem::SetMaxPartySize(int32 MaxSize)
{
    if (!IsPartyLeader()) return;
    CurrentParty.MaxSize = FMath::Clamp(MaxSize, CurrentParty.Members.Num(), 8);
}

void UMGPartyInviteSubsystem::SetPartySetting(const FString& Key, const FString& Value)
{
    if (!IsPartyLeader()) return;
    CurrentParty.PartySettings.Add(Key, Value);
}

FString UMGPartyInviteSubsystem::GetPartySetting(const FString& Key) const
{
    const FString* Value = CurrentParty.PartySettings.Find(Key);
    return Value ? *Value : FString();
}

void UMGPartyInviteSubsystem::SetSelectedGameMode(FName GameMode)
{
    if (!IsPartyLeader()) return;
    CurrentParty.SelectedGameMode = GameMode;
}

void UMGPartyInviteSubsystem::SetSelectedTrack(FName TrackId)
{
    if (!IsPartyLeader()) return;
    CurrentParty.SelectedTrack = TrackId;
}

void UMGPartyInviteSubsystem::SetCrossPlayEnabled(bool bEnabled)
{
    if (!IsPartyLeader()) return;
    CurrentParty.bAllowCrossPlay = bEnabled;
}

// ===== Member Management =====

void UMGPartyInviteSubsystem::KickMember(const FString& PlayerId)
{
    if (!IsPartyLeader() || PlayerId == LocalPlayerId)
    {
        return;
    }

    int32 RemovedIndex = CurrentParty.Members.IndexOfByPredicate([&PlayerId](const FMGPartyMember& Member) {
        return Member.PlayerId == PlayerId;
    });

    if (RemovedIndex != INDEX_NONE)
    {
        CurrentParty.Members.RemoveAt(RemovedIndex);
        OnMemberLeft.Broadcast(PlayerId);
        UE_LOG(LogTemp, Log, TEXT("Kicked member: %s"), *PlayerId);
    }
}

void UMGPartyInviteSubsystem::PromoteToLeader(const FString& PlayerId)
{
    if (!IsPartyLeader() || PlayerId == LocalPlayerId)
    {
        return;
    }

    for (FMGPartyMember& Member : CurrentParty.Members)
    {
        if (Member.PlayerId == LocalPlayerId)
        {
            Member.Role = EMGPartyRole::Member;
        }
        else if (Member.PlayerId == PlayerId)
        {
            Member.Role = EMGPartyRole::Leader;
        }
    }

    CurrentParty.LeaderId = PlayerId;
    OnLeaderChanged.Broadcast(PlayerId);
    UE_LOG(LogTemp, Log, TEXT("Promoted to leader: %s"), *PlayerId);
}

void UMGPartyInviteSubsystem::SetMemberRole(const FString& PlayerId, EMGPartyRole Role)
{
    if (!IsPartyLeader()) return;

    for (FMGPartyMember& Member : CurrentParty.Members)
    {
        if (Member.PlayerId == PlayerId)
        {
            Member.Role = Role;
            OnMemberUpdated.Broadcast(Member);
            break;
        }
    }
}

void UMGPartyInviteSubsystem::SetLocalReady(bool bReady)
{
    bLocalReady = bReady;

    for (FMGPartyMember& Member : CurrentParty.Members)
    {
        if (Member.PlayerId == LocalPlayerId)
        {
            Member.bIsReady = bReady;
            OnMemberUpdated.Broadcast(Member);
            break;
        }
    }
}

bool UMGPartyInviteSubsystem::IsLocalReady() const
{
    return bLocalReady;
}

bool UMGPartyInviteSubsystem::AreAllMembersReady() const
{
    for (const FMGPartyMember& Member : CurrentParty.Members)
    {
        if (!Member.bIsReady)
        {
            return false;
        }
    }
    return CurrentParty.Members.Num() > 0;
}

void UMGPartyInviteSubsystem::SetLocalVehicle(FName VehicleId)
{
    for (FMGPartyMember& Member : CurrentParty.Members)
    {
        if (Member.PlayerId == LocalPlayerId)
        {
            Member.SelectedVehicle = VehicleId;
            OnMemberUpdated.Broadcast(Member);
            break;
        }
    }
}

// ===== Invitations =====

bool UMGPartyInviteSubsystem::SendInvite(const FString& PlayerId, FText Message)
{
    if (!IsInParty())
    {
        OnPartyError.Broadcast(TEXT("Not in a party"));
        return false;
    }

    if (IsPartyFull())
    {
        OnPartyError.Broadcast(TEXT("Party is full"));
        return false;
    }

    FMGPartyInvitation Invite;
    Invite.InviteId = GenerateUniqueId();
    Invite.PartyId = CurrentParty.PartyId;
    Invite.SenderId = LocalPlayerId;
    Invite.SenderName = TEXT("Local Player");
    Invite.ReceiverId = PlayerId;
    Invite.Status = EMGInviteStatus::Pending;
    Invite.Source = EMGInviteSource::Direct;
    Invite.Message = Message;
    Invite.SentAt = FDateTime::Now();
    Invite.ExpiresAt = FDateTime::Now() + FTimespan::FromMinutes(5);
    Invite.PartySize = CurrentParty.Members.Num();
    Invite.PartyMaxSize = CurrentParty.MaxSize;
    Invite.CurrentActivity = CurrentParty.CurrentActivity;

    SentInvites.Add(Invite);

    UE_LOG(LogTemp, Log, TEXT("Invite sent to: %s"), *PlayerId);
    return true;
}

bool UMGPartyInviteSubsystem::SendInviteToFriends(const TArray<FString>& PlayerIds)
{
    bool bAllSent = true;
    for (const FString& PlayerId : PlayerIds)
    {
        if (!SendInvite(PlayerId))
        {
            bAllSent = false;
        }
    }
    return bAllSent;
}

void UMGPartyInviteSubsystem::AcceptInvite(const FString& InviteId)
{
    FMGPartyInvitation* Invite = ReceivedInvites.FindByPredicate([&InviteId](const FMGPartyInvitation& Inv) {
        return Inv.InviteId == InviteId;
    });

    if (!Invite || !Invite->CanAccept())
    {
        OnPartyError.Broadcast(TEXT("Cannot accept invite"));
        return;
    }

    // Leave current party if in one
    if (IsInParty())
    {
        LeaveParty();
    }

    Invite->Status = EMGInviteStatus::Accepted;
    OnInviteStatusChanged.Broadcast(InviteId, EMGInviteStatus::Accepted);

    // Join the party
    JoinParty(Invite->PartyId);

    UE_LOG(LogTemp, Log, TEXT("Accepted invite: %s"), *InviteId);
}

void UMGPartyInviteSubsystem::DeclineInvite(const FString& InviteId)
{
    for (FMGPartyInvitation& Invite : ReceivedInvites)
    {
        if (Invite.InviteId == InviteId && Invite.Status == EMGInviteStatus::Pending)
        {
            Invite.Status = EMGInviteStatus::Declined;
            OnInviteStatusChanged.Broadcast(InviteId, EMGInviteStatus::Declined);
            UE_LOG(LogTemp, Log, TEXT("Declined invite: %s"), *InviteId);
            break;
        }
    }
}

void UMGPartyInviteSubsystem::CancelInvite(const FString& InviteId)
{
    for (FMGPartyInvitation& Invite : SentInvites)
    {
        if (Invite.InviteId == InviteId && Invite.Status == EMGInviteStatus::Pending)
        {
            Invite.Status = EMGInviteStatus::Cancelled;
            OnInviteStatusChanged.Broadcast(InviteId, EMGInviteStatus::Cancelled);
            UE_LOG(LogTemp, Log, TEXT("Cancelled invite: %s"), *InviteId);
            break;
        }
    }
}

TArray<FMGPartyInvitation> UMGPartyInviteSubsystem::GetPendingInvites() const
{
    TArray<FMGPartyInvitation> Result;
    for (const FMGPartyInvitation& Invite : ReceivedInvites)
    {
        if (Invite.Status == EMGInviteStatus::Pending && !Invite.IsExpired())
        {
            Result.Add(Invite);
        }
    }
    return Result;
}

TArray<FMGPartyInvitation> UMGPartyInviteSubsystem::GetSentInvites() const
{
    TArray<FMGPartyInvitation> Result;
    for (const FMGPartyInvitation& Invite : SentInvites)
    {
        if (Invite.Status == EMGInviteStatus::Pending && !Invite.IsExpired())
        {
            Result.Add(Invite);
        }
    }
    return Result;
}

int32 UMGPartyInviteSubsystem::GetPendingInviteCount() const
{
    return GetPendingInvites().Num();
}

void UMGPartyInviteSubsystem::ClearExpiredInvites()
{
    ReceivedInvites.RemoveAll([](const FMGPartyInvitation& Invite) {
        return Invite.IsExpired() || Invite.Status != EMGInviteStatus::Pending;
    });

    SentInvites.RemoveAll([](const FMGPartyInvitation& Invite) {
        return Invite.IsExpired() || Invite.Status != EMGInviteStatus::Pending;
    });
}

// ===== Join Requests =====

bool UMGPartyInviteSubsystem::RequestToJoin(const FString& PartyId, FText Message)
{
    FMGJoinRequest Request;
    Request.RequestId = GenerateUniqueId();
    Request.RequesterId = LocalPlayerId;
    Request.RequesterName = TEXT("Local Player");
    Request.TargetPartyId = PartyId;
    Request.Status = EMGInviteStatus::Pending;
    Request.Message = Message;
    Request.RequestedAt = FDateTime::Now();

    // In a real implementation, send to party host
    UE_LOG(LogTemp, Log, TEXT("Join request sent to party: %s"), *PartyId);
    return true;
}

void UMGPartyInviteSubsystem::ApproveJoinRequest(const FString& RequestId)
{
    if (!IsPartyLeader()) return;

    for (FMGJoinRequest& Request : JoinRequests)
    {
        if (Request.RequestId == RequestId && Request.Status == EMGInviteStatus::Pending)
        {
            Request.Status = EMGInviteStatus::Accepted;

            // Add member to party
            FMGPartyMember NewMember;
            NewMember.PlayerId = Request.RequesterId;
            NewMember.DisplayName = Request.RequesterName;
            NewMember.Role = EMGPartyRole::Member;
            NewMember.JoinedAt = FDateTime::Now();
            CurrentParty.Members.Add(NewMember);

            OnMemberJoined.Broadcast(NewMember);
            UE_LOG(LogTemp, Log, TEXT("Join request approved: %s"), *RequestId);
            break;
        }
    }
}

void UMGPartyInviteSubsystem::DenyJoinRequest(const FString& RequestId)
{
    if (!IsPartyLeader()) return;

    for (FMGJoinRequest& Request : JoinRequests)
    {
        if (Request.RequestId == RequestId && Request.Status == EMGInviteStatus::Pending)
        {
            Request.Status = EMGInviteStatus::Declined;
            UE_LOG(LogTemp, Log, TEXT("Join request denied: %s"), *RequestId);
            break;
        }
    }
}

TArray<FMGJoinRequest> UMGPartyInviteSubsystem::GetPendingJoinRequests() const
{
    TArray<FMGJoinRequest> Result;
    for (const FMGJoinRequest& Request : JoinRequests)
    {
        if (Request.Status == EMGInviteStatus::Pending)
        {
            Result.Add(Request);
        }
    }
    return Result;
}

// ===== Join In Progress =====

void UMGPartyInviteSubsystem::JoinParty(const FString& PartyId)
{
    // In a real implementation, connect to party service
    // Simulating join

    if (IsInParty())
    {
        LeaveParty();
    }

    CurrentParty.PartyId = PartyId;
    CurrentParty.State = EMGPartyState::Ready;

    // Add self as member
    FMGPartyMember LocalMember;
    LocalMember.PlayerId = LocalPlayerId;
    LocalMember.DisplayName = TEXT("Local Player");
    LocalMember.Role = EMGPartyRole::Member;
    LocalMember.JoinedAt = FDateTime::Now();
    CurrentParty.Members.Add(LocalMember);

    OnPartyJoined.Broadcast(CurrentParty);
    UE_LOG(LogTemp, Log, TEXT("Joined party: %s"), *PartyId);
}

void UMGPartyInviteSubsystem::JoinFriend(const FString& FriendId)
{
    // In a real implementation, query friend's party and join
    UE_LOG(LogTemp, Log, TEXT("Joining friend: %s"), *FriendId);
}

void UMGPartyInviteSubsystem::JoinFromInviteLink(const FString& InviteLink)
{
    // Parse invite link and join
    ProcessPlatformInvite(InviteLink);
}

bool UMGPartyInviteSubsystem::CanJoinParty(const FString& PartyId) const
{
    // In a real implementation, query party status
    return true;
}

FString UMGPartyInviteSubsystem::GenerateInviteLink()
{
    if (!IsInParty()) return FString();

    // Generate shareable link
    return FString::Printf(TEXT("midnightgrind://party/%s"), *CurrentParty.PartyId);
}

// ===== Recent Players =====

void UMGPartyInviteSubsystem::AddRecentPlayer(const FMGRecentPlayer& Player)
{
    // Check if already in list
    for (FMGRecentPlayer& Existing : RecentPlayers)
    {
        if (Existing.PlayerId == Player.PlayerId)
        {
            Existing.LastPlayedWith = FDateTime::Now();
            Existing.TimesPlayedWith++;
            Existing.LastGameMode = Player.LastGameMode;
            return;
        }
    }

    // Add new
    RecentPlayers.Insert(Player, 0);

    // Trim to max
    if (RecentPlayers.Num() > MaxRecentPlayers)
    {
        RecentPlayers.SetNum(MaxRecentPlayers);
    }
}

TArray<FMGRecentPlayer> UMGPartyInviteSubsystem::GetRecentPlayers(int32 MaxCount) const
{
    int32 Count = FMath::Min(MaxCount, RecentPlayers.Num());
    TArray<FMGRecentPlayer> Result;
    for (int32 i = 0; i < Count; i++)
    {
        Result.Add(RecentPlayers[i]);
    }
    return Result;
}

void UMGPartyInviteSubsystem::ClearRecentPlayers()
{
    RecentPlayers.Empty();
}

// ===== Platform Integration =====

void UMGPartyInviteSubsystem::ShowPlatformInviteUI()
{
    // In a real implementation, show platform-specific invite UI
    UE_LOG(LogTemp, Log, TEXT("Showing platform invite UI"));
}

void UMGPartyInviteSubsystem::RegisterForPlatformInvites()
{
    // In a real implementation, register callbacks for platform invites
    UE_LOG(LogTemp, Log, TEXT("Registered for platform invites"));
}

// ===== Internal Helpers =====

void UMGPartyInviteSubsystem::UpdatePartyState(EMGPartyState NewState)
{
    if (CurrentParty.State != NewState)
    {
        CurrentParty.State = NewState;
        OnPartyStateChanged.Broadcast(NewState);
    }
}

FString UMGPartyInviteSubsystem::GenerateUniqueId()
{
    return FGuid::NewGuid().ToString();
}

void UMGPartyInviteSubsystem::ProcessPlatformInvite(const FString& InviteData)
{
    // Parse platform-specific invite data
    // Extract party ID and join
    UE_LOG(LogTemp, Log, TEXT("Processing platform invite: %s"), *InviteData);
}

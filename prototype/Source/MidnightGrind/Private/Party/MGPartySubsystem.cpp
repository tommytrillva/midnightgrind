// Copyright Midnight Grind. All Rights Reserved.

#include "Party/MGPartySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default activities
	FMGPartyActivity RaceActivity;
	RaceActivity.ActivityType = FName("Race");
	RaceActivity.ActivityName = FText::FromString("Race");
	RaceActivity.ActivityDescription = FText::FromString("Compete in online races");
	RaceActivity.MinPlayers = 1;
	RaceActivity.MaxPlayers = 8;
	RaceActivity.bSupportsRanked = true;
	AvailableActivities.Add(RaceActivity);

	FMGPartyActivity FreeroamActivity;
	FreeroamActivity.ActivityType = FName("Freeroam");
	FreeroamActivity.ActivityName = FText::FromString("Free Roam");
	FreeroamActivity.ActivityDescription = FText::FromString("Cruise the city with friends");
	FreeroamActivity.MinPlayers = 1;
	FreeroamActivity.MaxPlayers = 16;
	FreeroamActivity.bSupportsRanked = false;
	AvailableActivities.Add(FreeroamActivity);

	FMGPartyActivity DriftActivity;
	DriftActivity.ActivityType = FName("Drift");
	DriftActivity.ActivityName = FText::FromString("Drift Session");
	DriftActivity.ActivityDescription = FText::FromString("Show off your drift skills");
	DriftActivity.MinPlayers = 1;
	DriftActivity.MaxPlayers = 4;
	DriftActivity.bSupportsRanked = true;
	AvailableActivities.Add(DriftActivity);

	// Start party tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PartyTickHandle,
			this,
			&UMGPartySubsystem::OnPartyTick,
			1.0f,
			true
		);
	}
}

void UMGPartySubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PartyTickHandle);
	}

	// Leave party on shutdown
	if (IsInParty())
	{
		LeaveParty();
	}

	Super::Deinitialize();
}

bool UMGPartySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

FGuid UMGPartySubsystem::CreateParty(const FMGPartySettings& Settings)
{
	if (IsInParty())
	{
		return FGuid();
	}

	CurrentParty.PartyID = FGuid::NewGuid();
	CurrentParty.State = EMGPartyState::Forming;
	CurrentParty.Settings = Settings;
	CurrentParty.LeaderID = LocalPlayerID;
	CurrentParty.CreatedAt = FDateTime::UtcNow();
	CurrentParty.VoiceChannelID = FName(*FGuid::NewGuid().ToString());

	// Add local player as first member
	FMGPartyMember LocalMember = CreateLocalMember();
	LocalMember.Role = EMGPartyRole::Leader;
	CurrentParty.Members.Add(LocalMember);

	SetPartyState(EMGPartyState::Forming);

	OnPartyCreated.Broadcast(CurrentParty);

	return CurrentParty.PartyID;
}

bool UMGPartySubsystem::DisbandParty()
{
	if (!IsInParty())
	{
		return false;
	}

	if (!IsPartyLeader())
	{
		return false;
	}

	// Cancel any pending invites
	for (const FMGPartyInvite& Invite : SentInvites)
	{
		OnPartyInviteResponse.Broadcast(Invite.InviteID, false);
	}
	SentInvites.Empty();

	// Clear party
	CurrentParty = FMGParty();

	OnPartyDisbanded.Broadcast();

	return true;
}

bool UMGPartySubsystem::IsInParty() const
{
	return CurrentParty.PartyID.IsValid();
}

bool UMGPartySubsystem::IsPartyLeader() const
{
	return IsInParty() && CurrentParty.LeaderID == LocalPlayerID;
}

FGuid UMGPartySubsystem::InvitePlayer(FName PlayerID, const FText& CustomMessage)
{
	if (!IsInParty())
	{
		return FGuid();
	}

	if (!IsPartyLeader() && GetLocalMember().Role != EMGPartyRole::Moderator)
	{
		return FGuid();
	}

	// Check if party is full
	if (CurrentParty.Members.Num() >= CurrentParty.Settings.MaxSize)
	{
		return FGuid();
	}

	// Check if player is already in party
	if (FindMemberIndex(PlayerID) != INDEX_NONE)
	{
		return FGuid();
	}

	// Check for existing invite
	for (const FMGPartyInvite& Existing : SentInvites)
	{
		if (Existing.RecipientID == PlayerID && Existing.State == EMGPartyInviteState::Pending)
		{
			return FGuid();
		}
	}

	FMGPartyInvite Invite;
	Invite.InviteID = FGuid::NewGuid();
	Invite.PartyID = CurrentParty.PartyID;
	Invite.SenderID = LocalPlayerID;
	Invite.SenderName = LocalPlayerName;
	Invite.RecipientID = PlayerID;
	Invite.State = EMGPartyInviteState::Pending;
	Invite.SentAt = FDateTime::UtcNow();
	Invite.ExpiresAt = Invite.SentAt + FTimespan::FromMinutes(CurrentParty.Settings.InviteExpirationMinutes);
	Invite.CustomMessage = CustomMessage;
	Invite.PartyMemberCount = CurrentParty.Members.Num();
	Invite.PartyMaxSize = CurrentParty.Settings.MaxSize;

	SentInvites.Add(Invite);

	return Invite.InviteID;
}

bool UMGPartySubsystem::CancelInvite(FGuid InviteID)
{
	for (int32 i = SentInvites.Num() - 1; i >= 0; i--)
	{
		if (SentInvites[i].InviteID == InviteID)
		{
			SentInvites[i].State = EMGPartyInviteState::Cancelled;
			OnPartyInviteResponse.Broadcast(InviteID, false);
			SentInvites.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UMGPartySubsystem::AcceptInvite(FGuid InviteID)
{
	for (int32 i = PendingInvites.Num() - 1; i >= 0; i--)
	{
		if (PendingInvites[i].InviteID == InviteID)
		{
			FMGPartyInvite& Invite = PendingInvites[i];

			if (Invite.State != EMGPartyInviteState::Pending)
			{
				return false;
			}

			if (FDateTime::UtcNow() > Invite.ExpiresAt)
			{
				Invite.State = EMGPartyInviteState::Expired;
				return false;
			}

			Invite.State = EMGPartyInviteState::Accepted;

			// Join the party
			bool bJoined = JoinParty(Invite.PartyID);

			PendingInvites.RemoveAt(i);

			return bJoined;
		}
	}
	return false;
}

bool UMGPartySubsystem::DeclineInvite(FGuid InviteID)
{
	for (int32 i = PendingInvites.Num() - 1; i >= 0; i--)
	{
		if (PendingInvites[i].InviteID == InviteID)
		{
			PendingInvites[i].State = EMGPartyInviteState::Declined;
			PendingInvites.RemoveAt(i);
			return true;
		}
	}
	return false;
}

TArray<FMGPartyInvite> UMGPartySubsystem::GetPendingInvites() const
{
	TArray<FMGPartyInvite> Result;
	for (const FMGPartyInvite& Invite : PendingInvites)
	{
		if (Invite.State == EMGPartyInviteState::Pending)
		{
			Result.Add(Invite);
		}
	}
	return Result;
}

TArray<FMGPartyInvite> UMGPartySubsystem::GetSentInvites() const
{
	TArray<FMGPartyInvite> Result;
	for (const FMGPartyInvite& Invite : SentInvites)
	{
		if (Invite.State == EMGPartyInviteState::Pending)
		{
			Result.Add(Invite);
		}
	}
	return Result;
}

bool UMGPartySubsystem::JoinParty(FGuid PartyID)
{
	if (IsInParty())
	{
		LeaveParty();
	}

	// This would normally send a network request to join
	// For now, simulate success
	CurrentParty.PartyID = PartyID;

	FMGPartyMember LocalMember = CreateLocalMember();
	CurrentParty.Members.Add(LocalMember);

	SetPartyState(EMGPartyState::Forming);

	OnPartyJoined.Broadcast(CurrentParty);

	return true;
}

bool UMGPartySubsystem::RequestToJoin(FGuid PartyID, const FText& Message)
{
	// This would send a join request to the party leader
	// For now, just return true
	return true;
}

bool UMGPartySubsystem::ApproveJoinRequest(FGuid RequestID)
{
	if (!IsPartyLeader() && GetLocalMember().Role != EMGPartyRole::Moderator)
	{
		return false;
	}

	for (int32 i = JoinRequests.Num() - 1; i >= 0; i--)
	{
		if (JoinRequests[i].RequestID == RequestID)
		{
			// Would normally send approval to the player
			JoinRequests.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UMGPartySubsystem::DenyJoinRequest(FGuid RequestID)
{
	if (!IsPartyLeader() && GetLocalMember().Role != EMGPartyRole::Moderator)
	{
		return false;
	}

	for (int32 i = JoinRequests.Num() - 1; i >= 0; i--)
	{
		if (JoinRequests[i].RequestID == RequestID)
		{
			JoinRequests.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UMGPartySubsystem::LeaveParty()
{
	if (!IsInParty())
	{
		return false;
	}

	// If leader and others remain, promote someone
	if (IsPartyLeader() && CurrentParty.Members.Num() > 1)
	{
		for (const FMGPartyMember& Member : CurrentParty.Members)
		{
			if (Member.PlayerID != LocalPlayerID)
			{
				PromoteToLeader(Member.PlayerID);
				break;
			}
		}
	}

	// Leave voice channel
	LeaveVoiceChannel();

	// Clear local state
	CurrentParty = FMGParty();
	bLocalReady = false;

	OnPartyLeft.Broadcast();

	return true;
}

TArray<FMGJoinRequest> UMGPartySubsystem::GetPendingJoinRequests() const
{
	return JoinRequests;
}

bool UMGPartySubsystem::KickMember(FName PlayerID)
{
	if (!IsPartyLeader())
	{
		return false;
	}

	if (PlayerID == LocalPlayerID)
	{
		return false;
	}

	int32 Index = FindMemberIndex(PlayerID);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	CurrentParty.Members.RemoveAt(Index);

	OnPartyMemberLeft.Broadcast(PlayerID);
	BroadcastPartyUpdate();

	return true;
}

bool UMGPartySubsystem::PromoteToLeader(FName PlayerID)
{
	if (!IsPartyLeader())
	{
		return false;
	}

	int32 Index = FindMemberIndex(PlayerID);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	// Demote current leader
	int32 LocalIndex = FindMemberIndex(LocalPlayerID);
	if (LocalIndex != INDEX_NONE)
	{
		CurrentParty.Members[LocalIndex].Role = EMGPartyRole::Moderator;
	}

	// Promote new leader
	CurrentParty.Members[Index].Role = EMGPartyRole::Leader;
	CurrentParty.LeaderID = PlayerID;

	OnPartyLeaderChanged.Broadcast(PlayerID);
	BroadcastPartyUpdate();

	return true;
}

bool UMGPartySubsystem::SetMemberRole(FName PlayerID, EMGPartyRole NewRole)
{
	if (!IsPartyLeader())
	{
		return false;
	}

	if (NewRole == EMGPartyRole::Leader)
	{
		return PromoteToLeader(PlayerID);
	}

	int32 Index = FindMemberIndex(PlayerID);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	CurrentParty.Members[Index].Role = NewRole;

	OnPartyMemberUpdated.Broadcast(CurrentParty.Members[Index]);
	BroadcastPartyUpdate();

	return true;
}

TArray<FMGPartyMember> UMGPartySubsystem::GetPartyMembers() const
{
	return CurrentParty.Members;
}

FMGPartyMember UMGPartySubsystem::GetMember(FName PlayerID) const
{
	int32 Index = FindMemberIndex(PlayerID);
	if (Index != INDEX_NONE)
	{
		return CurrentParty.Members[Index];
	}
	return FMGPartyMember();
}

FMGPartyMember UMGPartySubsystem::GetLocalMember() const
{
	return GetMember(LocalPlayerID);
}

int32 UMGPartySubsystem::GetMemberCount() const
{
	return CurrentParty.Members.Num();
}

bool UMGPartySubsystem::IsMemberReady(FName PlayerID) const
{
	int32 Index = FindMemberIndex(PlayerID);
	if (Index != INDEX_NONE)
	{
		return CurrentParty.Members[Index].bIsReady;
	}
	return false;
}

bool UMGPartySubsystem::AreAllMembersReady() const
{
	if (CurrentParty.Members.Num() == 0)
	{
		return false;
	}

	for (const FMGPartyMember& Member : CurrentParty.Members)
	{
		if (!Member.bIsReady)
		{
			return false;
		}
	}
	return true;
}

void UMGPartySubsystem::SetReady(bool bReady)
{
	bLocalReady = bReady;

	int32 Index = FindMemberIndex(LocalPlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].bIsReady = bReady;
		OnPartyMemberUpdated.Broadcast(CurrentParty.Members[Index]);
	}

	if (AreAllMembersReady())
	{
		SetPartyState(EMGPartyState::Ready);
		OnPartyReady.Broadcast();
	}

	BroadcastPartyUpdate();
}

bool UMGPartySubsystem::IsReady() const
{
	return bLocalReady;
}

void UMGPartySubsystem::ToggleReady()
{
	SetReady(!bLocalReady);
}

bool UMGPartySubsystem::UpdatePartySettings(const FMGPartySettings& NewSettings)
{
	if (!IsPartyLeader())
	{
		return false;
	}

	// Validate max size doesn't kick members
	if (NewSettings.MaxSize < CurrentParty.Members.Num())
	{
		return false;
	}

	CurrentParty.Settings = NewSettings;
	BroadcastPartyUpdate();

	return true;
}

FMGPartySettings UMGPartySubsystem::GetPartySettings() const
{
	return CurrentParty.Settings;
}

bool UMGPartySubsystem::SetPrivacy(EMGPartyPrivacy Privacy)
{
	if (!IsPartyLeader())
	{
		return false;
	}

	CurrentParty.Settings.Privacy = Privacy;
	BroadcastPartyUpdate();

	return true;
}

bool UMGPartySubsystem::SetMaxSize(int32 MaxSize)
{
	if (!IsPartyLeader())
	{
		return false;
	}

	if (MaxSize < CurrentParty.Members.Num() || MaxSize > 16)
	{
		return false;
	}

	CurrentParty.Settings.MaxSize = MaxSize;
	BroadcastPartyUpdate();

	return true;
}

bool UMGPartySubsystem::SetPreferredGameMode(FName GameModeID)
{
	if (!IsPartyLeader())
	{
		return false;
	}

	CurrentParty.Settings.PreferredGameMode = GameModeID;
	BroadcastPartyUpdate();

	return true;
}

bool UMGPartySubsystem::JoinVoiceChannel()
{
	if (!IsInParty())
	{
		return false;
	}

	if (!CurrentParty.Settings.bEnableVoiceChat)
	{
		return false;
	}

	LocalVoiceState = EMGVoiceState::Connecting;
	OnVoiceStateChanged.Broadcast(LocalPlayerID, LocalVoiceState);

	// Simulate connection
	LocalVoiceState = EMGVoiceState::Connected;

	int32 Index = FindMemberIndex(LocalPlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].VoiceState = EMGVoiceState::Connected;
	}

	OnVoiceStateChanged.Broadcast(LocalPlayerID, LocalVoiceState);

	return true;
}

void UMGPartySubsystem::LeaveVoiceChannel()
{
	LocalVoiceState = EMGVoiceState::Disconnected;

	int32 Index = FindMemberIndex(LocalPlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].VoiceState = EMGVoiceState::Disconnected;
	}

	OnVoiceStateChanged.Broadcast(LocalPlayerID, LocalVoiceState);
}

void UMGPartySubsystem::SetMuted(bool bMuted)
{
	if (LocalVoiceState == EMGVoiceState::Disconnected)
	{
		return;
	}

	LocalVoiceState = bMuted ? EMGVoiceState::Muted : EMGVoiceState::Connected;

	int32 Index = FindMemberIndex(LocalPlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].VoiceState = LocalVoiceState;
	}

	OnVoiceStateChanged.Broadcast(LocalPlayerID, LocalVoiceState);
}

void UMGPartySubsystem::SetDeafened(bool bDeafened)
{
	if (LocalVoiceState == EMGVoiceState::Disconnected)
	{
		return;
	}

	LocalVoiceState = bDeafened ? EMGVoiceState::Deafened : EMGVoiceState::Connected;

	int32 Index = FindMemberIndex(LocalPlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].VoiceState = LocalVoiceState;
	}

	OnVoiceStateChanged.Broadcast(LocalPlayerID, LocalVoiceState);
}

void UMGPartySubsystem::SetMemberVolume(FName PlayerID, float Volume)
{
	int32 Index = FindMemberIndex(PlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].VoiceVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
	}
}

EMGVoiceState UMGPartySubsystem::GetVoiceState() const
{
	return LocalVoiceState;
}

bool UMGPartySubsystem::IsInVoiceChannel() const
{
	return LocalVoiceState != EMGVoiceState::Disconnected;
}

TArray<FName> UMGPartySubsystem::GetSpeakingMembers() const
{
	TArray<FName> Result;
	for (const FMGPartyMember& Member : CurrentParty.Members)
	{
		if (Member.bIsSpeaking)
		{
			Result.Add(Member.PlayerID);
		}
	}
	return Result;
}

bool UMGPartySubsystem::StartQueue(FName GameModeID)
{
	if (!IsInParty())
	{
		return false;
	}

	if (!IsPartyLeader())
	{
		return false;
	}

	if (CurrentParty.Settings.bRequireReadyToQueue && !AreAllMembersReady())
	{
		return false;
	}

	CurrentParty.Settings.PreferredGameMode = GameModeID;
	SetPartyState(EMGPartyState::InQueue);

	if (UWorld* World = GetWorld())
	{
		QueueStartTime = World->GetTimeSeconds();
	}

	return true;
}

bool UMGPartySubsystem::CancelQueue()
{
	if (!IsInQueue())
	{
		return false;
	}

	if (!IsPartyLeader())
	{
		return false;
	}

	SetPartyState(EMGPartyState::Ready);
	QueueStartTime = 0.0f;

	return true;
}

bool UMGPartySubsystem::IsInQueue() const
{
	return CurrentParty.State == EMGPartyState::InQueue;
}

float UMGPartySubsystem::GetQueueTime() const
{
	if (!IsInQueue() || QueueStartTime <= 0.0f)
	{
		return 0.0f;
	}

	if (UWorld* World = GetWorld())
	{
		return World->GetTimeSeconds() - QueueStartTime;
	}
	return 0.0f;
}

void UMGPartySubsystem::SetLocalPlayerInfo(FName PlayerID, const FString& DisplayName, int32 Level)
{
	LocalPlayerID = PlayerID;
	LocalPlayerName = DisplayName;
	LocalPlayerLevel = Level;

	// Update in party if present
	int32 Index = FindMemberIndex(PlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].DisplayName = DisplayName;
		CurrentParty.Members[Index].PlayerLevel = Level;
		OnPartyMemberUpdated.Broadcast(CurrentParty.Members[Index]);
	}
}

void UMGPartySubsystem::SetSelectedVehicle(FName VehicleID)
{
	LocalSelectedVehicle = VehicleID;

	int32 Index = FindMemberIndex(LocalPlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].SelectedVehicleID = VehicleID;
		OnPartyMemberUpdated.Broadcast(CurrentParty.Members[Index]);
		BroadcastPartyUpdate();
	}
}

void UMGPartySubsystem::UpdateActivity(const FString& Activity)
{
	LocalActivity = Activity;

	int32 Index = FindMemberIndex(LocalPlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index].CurrentActivity = Activity;
		OnPartyMemberUpdated.Broadcast(CurrentParty.Members[Index]);
	}
}

TArray<FMGPartyActivity> UMGPartySubsystem::GetAvailableActivities() const
{
	return AvailableActivities;
}

bool UMGPartySubsystem::SetPartyActivity(FName ActivityType)
{
	if (!IsPartyLeader())
	{
		return false;
	}

	for (const FMGPartyActivity& Activity : AvailableActivities)
	{
		if (Activity.ActivityType == ActivityType)
		{
			CurrentParty.Settings.PreferredGameMode = ActivityType;
			BroadcastPartyUpdate();
			return true;
		}
	}
	return false;
}

void UMGPartySubsystem::ReceivePartyUpdate(const FMGParty& PartyData)
{
	if (!IsInParty())
	{
		return;
	}

	if (PartyData.PartyID != CurrentParty.PartyID)
	{
		return;
	}

	// Update party data but preserve local state
	EMGPartyState PreviousState = CurrentParty.State;
	CurrentParty = PartyData;

	if (PreviousState != CurrentParty.State)
	{
		OnPartyStateChanged.Broadcast(CurrentParty.State);
	}
}

void UMGPartySubsystem::ReceiveInvite(const FMGPartyInvite& Invite)
{
	if (Invite.RecipientID != LocalPlayerID)
	{
		return;
	}

	PendingInvites.Add(Invite);
	OnPartyInviteReceived.Broadcast(Invite);
}

void UMGPartySubsystem::ReceiveJoinRequest(const FMGJoinRequest& Request)
{
	if (!IsPartyLeader() && GetLocalMember().Role != EMGPartyRole::Moderator)
	{
		return;
	}

	JoinRequests.Add(Request);
	OnJoinRequestReceived.Broadcast(Request);
}

void UMGPartySubsystem::ReceiveMemberUpdate(const FMGPartyMember& Member)
{
	int32 Index = FindMemberIndex(Member.PlayerID);
	if (Index != INDEX_NONE)
	{
		CurrentParty.Members[Index] = Member;
		OnPartyMemberUpdated.Broadcast(Member);
	}
	else
	{
		// New member joined
		CurrentParty.Members.Add(Member);
		OnPartyMemberJoined.Broadcast(Member);
	}
}

void UMGPartySubsystem::OnPartyTick()
{
	CheckExpiredInvites();
	UpdateMemberPresence();
}

void UMGPartySubsystem::CheckExpiredInvites()
{
	FDateTime Now = FDateTime::UtcNow();

	// Check pending invites
	for (int32 i = PendingInvites.Num() - 1; i >= 0; i--)
	{
		if (PendingInvites[i].State == EMGPartyInviteState::Pending && Now > PendingInvites[i].ExpiresAt)
		{
			PendingInvites[i].State = EMGPartyInviteState::Expired;
			PendingInvites.RemoveAt(i);
		}
	}

	// Check sent invites
	for (int32 i = SentInvites.Num() - 1; i >= 0; i--)
	{
		if (SentInvites[i].State == EMGPartyInviteState::Pending && Now > SentInvites[i].ExpiresAt)
		{
			SentInvites[i].State = EMGPartyInviteState::Expired;
			OnPartyInviteResponse.Broadcast(SentInvites[i].InviteID, false);
			SentInvites.RemoveAt(i);
		}
	}
}

void UMGPartySubsystem::UpdateMemberPresence()
{
	// This would check network status of party members
	// For now, do nothing
}

void UMGPartySubsystem::BroadcastPartyUpdate()
{
	// This would send party state to all members via network
	// For now, do nothing
}

FMGPartyMember UMGPartySubsystem::CreateLocalMember() const
{
	FMGPartyMember Member;
	Member.PlayerID = LocalPlayerID;
	Member.DisplayName = LocalPlayerName;
	Member.Role = EMGPartyRole::Member;
	Member.bIsReady = bLocalReady;
	Member.bIsOnline = true;
	Member.SelectedVehicleID = LocalSelectedVehicle;
	Member.CurrentActivity = LocalActivity;
	Member.PlayerLevel = LocalPlayerLevel;
	Member.VoiceState = LocalVoiceState;
	Member.JoinedAt = FDateTime::UtcNow();
	return Member;
}

void UMGPartySubsystem::SetPartyState(EMGPartyState NewState)
{
	if (CurrentParty.State != NewState)
	{
		CurrentParty.State = NewState;
		OnPartyStateChanged.Broadcast(NewState);
	}
}

int32 UMGPartySubsystem::FindMemberIndex(FName PlayerID) const
{
	for (int32 i = 0; i < CurrentParty.Members.Num(); i++)
	{
		if (CurrentParty.Members[i].PlayerID == PlayerID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

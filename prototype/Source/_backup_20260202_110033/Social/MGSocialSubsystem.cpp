// Copyright Midnight Grind. All Rights Reserved.

#include "Social/MGSocialSubsystem.h"
#include "Misc/DateTime.h"

void UMGSocialSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSocialData();
	GenerateMockFriends();
	GenerateMockCrew();
}

void UMGSocialSubsystem::Deinitialize()
{
	SaveSocialData();
	Super::Deinitialize();
}

// ==========================================
// FRIENDS
// ==========================================

TArray<FMGFriendData> UMGSocialSubsystem::GetOnlineFriends() const
{
	TArray<FMGFriendData> Online;
	for (const FMGFriendData& Friend : Friends)
	{
		if (Friend.Status != EMGFriendStatus::Offline)
		{
			Online.Add(Friend);
		}
	}
	return Online;
}

FMGFriendData UMGSocialSubsystem::GetFriend(const FString& PlayerID) const
{
	for (const FMGFriendData& Friend : Friends)
	{
		if (Friend.PlayerID == PlayerID)
		{
			return Friend;
		}
	}
	return FMGFriendData();
}

bool UMGSocialSubsystem::IsFriend(const FString& PlayerID) const
{
	for (const FMGFriendData& Friend : Friends)
	{
		if (Friend.PlayerID == PlayerID)
		{
			return true;
		}
	}
	return false;
}

void UMGSocialSubsystem::SendFriendRequest(const FString& PlayerID)
{
	// Check if already friends
	if (IsFriend(PlayerID))
	{
		return;
	}

	// Check if blocked
	if (BlockedPlayers.Contains(PlayerID))
	{
		return;
	}

	// Create outgoing request
	FMGFriendRequest Request;
	Request.RequestID = FGuid::NewGuid().ToString();
	Request.SenderID = TEXT("LocalPlayer"); // Would be actual player ID
	Request.SenderName = TEXT("You");
	Request.Timestamp = FDateTime::Now();
	Request.Status = EMGRequestStatus::Pending;
	Request.bIsIncoming = false;

	PendingFriendRequests.Add(Request);
	SaveSocialData();
}

void UMGSocialSubsystem::AcceptFriendRequest(const FString& RequestID)
{
	for (int32 i = 0; i < PendingFriendRequests.Num(); i++)
	{
		if (PendingFriendRequests[i].RequestID == RequestID && PendingFriendRequests[i].bIsIncoming)
		{
			FMGFriendRequest& Request = PendingFriendRequests[i];
			Request.Status = EMGRequestStatus::Accepted;

			// Add as friend
			FMGFriendData NewFriend;
			NewFriend.PlayerID = Request.SenderID;
			NewFriend.DisplayName = Request.SenderName;
			NewFriend.Status = EMGFriendStatus::Online;
			NewFriend.Level = Request.SenderLevel;
			NewFriend.LastOnline = FDateTime::Now();
			Friends.Add(NewFriend);

			PendingFriendRequests.RemoveAt(i);
			SaveSocialData();

			OnFriendListUpdated.Broadcast(Friends);
			return;
		}
	}
}

void UMGSocialSubsystem::DeclineFriendRequest(const FString& RequestID)
{
	for (int32 i = 0; i < PendingFriendRequests.Num(); i++)
	{
		if (PendingFriendRequests[i].RequestID == RequestID)
		{
			PendingFriendRequests[i].Status = EMGRequestStatus::Declined;
			PendingFriendRequests.RemoveAt(i);
			SaveSocialData();
			return;
		}
	}
}

void UMGSocialSubsystem::RemoveFriend(const FString& PlayerID)
{
	for (int32 i = 0; i < Friends.Num(); i++)
	{
		if (Friends[i].PlayerID == PlayerID)
		{
			Friends.RemoveAt(i);
			SaveSocialData();
			OnFriendListUpdated.Broadcast(Friends);
			return;
		}
	}
}

void UMGSocialSubsystem::BlockPlayer(const FString& PlayerID)
{
	if (!BlockedPlayers.Contains(PlayerID))
	{
		BlockedPlayers.Add(PlayerID);
		RemoveFriend(PlayerID);
		SaveSocialData();
	}
}

void UMGSocialSubsystem::UnblockPlayer(const FString& PlayerID)
{
	BlockedPlayers.Remove(PlayerID);
	SaveSocialData();
}

void UMGSocialSubsystem::SetFriendFavorite(const FString& PlayerID, bool bFavorite)
{
	for (FMGFriendData& Friend : Friends)
	{
		if (Friend.PlayerID == PlayerID)
		{
			Friend.bIsFavorite = bFavorite;
			SaveSocialData();
			OnFriendStatusChanged.Broadcast(Friend);
			return;
		}
	}
}

TArray<FMGFriendRequest> UMGSocialSubsystem::GetPendingRequests() const
{
	TArray<FMGFriendRequest> Pending;
	for (const FMGFriendRequest& Request : PendingFriendRequests)
	{
		if (Request.Status == EMGRequestStatus::Pending)
		{
			Pending.Add(Request);
		}
	}
	return Pending;
}

void UMGSocialSubsystem::RefreshFriendsList()
{
	// Would make network request to refresh friend data
	// For now, just broadcast current list
	OnFriendListUpdated.Broadcast(Friends);
}

// ==========================================
// CREW
// ==========================================

void UMGSocialSubsystem::CreateCrew(const FString& CrewName, const FString& CrewTag, const FString& Description)
{
	if (IsInCrew())
	{
		return;
	}

	CurrentCrew.CrewID = FGuid::NewGuid().ToString();
	CurrentCrew.CrewName = CrewName;
	CurrentCrew.CrewTag = CrewTag;
	CurrentCrew.Description = Description;
	CurrentCrew.Level = 1;
	CurrentCrew.CrewXP = 0;
	CurrentCrew.NextLevelXP = 1000;
	CurrentCrew.MemberCount = 1;
	CurrentCrew.MaxMembers = 50;
	CurrentCrew.bIsRecruiting = true;
	CurrentCrew.CreatedDate = FDateTime::Now();
	CurrentCrew.CrewColor = FLinearColor(1.0f, 0.2f, 0.2f);

	// Add self as leader
	FMGCrewMember SelfMember;
	SelfMember.PlayerID = TEXT("LocalPlayer");
	SelfMember.DisplayName = TEXT("You");
	SelfMember.Rank = EMGCrewRank::Leader;
	SelfMember.JoinDate = FDateTime::Now();
	SelfMember.Status = OwnStatus;
	CurrentCrew.Members.Add(SelfMember);

	PlayerCrewRank = EMGCrewRank::Leader;
	SaveSocialData();

	OnCrewDataUpdated.Broadcast(CurrentCrew);
}

void UMGSocialSubsystem::LeaveCrew()
{
	if (!IsInCrew())
	{
		return;
	}

	// If leader, transfer leadership or disband
	if (PlayerCrewRank == EMGCrewRank::Leader && CurrentCrew.Members.Num() > 1)
	{
		// Would transfer to next highest rank
	}

	CurrentCrew = FMGCrewData();
	PlayerCrewRank = EMGCrewRank::Member;
	SaveSocialData();

	OnCrewDataUpdated.Broadcast(CurrentCrew);
}

void UMGSocialSubsystem::InviteToCrew(const FString& PlayerID)
{
	if (!IsInCrew())
	{
		return;
	}

	// Check permissions
	if (PlayerCrewRank == EMGCrewRank::Member)
	{
		return;
	}

	// Would send invite through network
}

void UMGSocialSubsystem::AcceptCrewInvite(const FString& InviteID)
{
	for (int32 i = 0; i < PendingCrewInvites.Num(); i++)
	{
		if (PendingCrewInvites[i].InviteID == InviteID)
		{
			// Leave current crew if any
			if (IsInCrew())
			{
				LeaveCrew();
			}

			// Would join the crew through network request
			// For now, just remove invite
			PendingCrewInvites.RemoveAt(i);
			SaveSocialData();
			return;
		}
	}
}

void UMGSocialSubsystem::DeclineCrewInvite(const FString& InviteID)
{
	for (int32 i = 0; i < PendingCrewInvites.Num(); i++)
	{
		if (PendingCrewInvites[i].InviteID == InviteID)
		{
			PendingCrewInvites.RemoveAt(i);
			SaveSocialData();
			return;
		}
	}
}

void UMGSocialSubsystem::KickCrewMember(const FString& PlayerID)
{
	if (!IsInCrew() || PlayerCrewRank == EMGCrewRank::Member)
	{
		return;
	}

	for (int32 i = 0; i < CurrentCrew.Members.Num(); i++)
	{
		if (CurrentCrew.Members[i].PlayerID == PlayerID)
		{
			// Can't kick leader, can't kick higher rank
			if (CurrentCrew.Members[i].Rank == EMGCrewRank::Leader)
			{
				return;
			}
			if (PlayerCrewRank == EMGCrewRank::Officer && CurrentCrew.Members[i].Rank == EMGCrewRank::Officer)
			{
				return;
			}

			CurrentCrew.Members.RemoveAt(i);
			CurrentCrew.MemberCount--;
			SaveSocialData();
			OnCrewDataUpdated.Broadcast(CurrentCrew);
			return;
		}
	}
}

void UMGSocialSubsystem::PromoteCrewMember(const FString& PlayerID, EMGCrewRank NewRank)
{
	if (!IsInCrew() || PlayerCrewRank != EMGCrewRank::Leader)
	{
		return;
	}

	for (FMGCrewMember& Member : CurrentCrew.Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			// Can't promote to leader through this
			if (NewRank == EMGCrewRank::Leader)
			{
				return;
			}

			Member.Rank = NewRank;
			SaveSocialData();
			OnCrewDataUpdated.Broadcast(CurrentCrew);
			return;
		}
	}
}

void UMGSocialSubsystem::UpdateCrewInfo(const FString& Description, bool bRecruiting)
{
	if (!IsInCrew() || PlayerCrewRank != EMGCrewRank::Leader)
	{
		return;
	}

	CurrentCrew.Description = Description;
	CurrentCrew.bIsRecruiting = bRecruiting;
	SaveSocialData();
	OnCrewDataUpdated.Broadcast(CurrentCrew);
}

void UMGSocialSubsystem::SetCrewColor(FLinearColor Color)
{
	if (!IsInCrew() || PlayerCrewRank != EMGCrewRank::Leader)
	{
		return;
	}

	CurrentCrew.CrewColor = Color;
	SaveSocialData();
	OnCrewDataUpdated.Broadcast(CurrentCrew);
}

TArray<FMGCrewData> UMGSocialSubsystem::SearchCrews(const FString& SearchTerm) const
{
	// Would query server
	return TArray<FMGCrewData>();
}

// ==========================================
// GAME INVITES
// ==========================================

void UMGSocialSubsystem::SendGameInvite(const FString& PlayerID, const FString& SessionID)
{
	if (!IsFriend(PlayerID))
	{
		return;
	}

	// Would send through network
}

void UMGSocialSubsystem::AcceptGameInvite(const FString& SessionID)
{
	// Would join session
}

void UMGSocialSubsystem::DeclineGameInvite(const FString& SessionID)
{
	// Would decline invite
}

void UMGSocialSubsystem::JoinFriend(const FString& PlayerID)
{
	FMGFriendData Friend = GetFriend(PlayerID);
	if (Friend.bCanJoin && !Friend.SessionID.IsEmpty())
	{
		AcceptGameInvite(Friend.SessionID);
	}
}

// ==========================================
// RECENT PLAYERS
// ==========================================

void UMGSocialSubsystem::AddRecentPlayer(const FString& PlayerID, const FString& DisplayName, FName TrackID, int32 TheirPosition, int32 OurPosition)
{
	// Check if already in recent
	for (int32 i = 0; i < RecentPlayers.Num(); i++)
	{
		if (RecentPlayers[i].PlayerID == PlayerID)
		{
			// Update existing entry
			RecentPlayers[i].LastRaced = FDateTime::Now();
			RecentPlayers[i].TrackID = TrackID;
			RecentPlayers[i].TheirPosition = TheirPosition;
			RecentPlayers[i].OurPosition = OurPosition;
			RecentPlayers[i].bIsFriend = IsFriend(PlayerID);
			RecentPlayers[i].bIsBlocked = BlockedPlayers.Contains(PlayerID);

			// Move to front
			FMGRecentPlayer Temp = RecentPlayers[i];
			RecentPlayers.RemoveAt(i);
			RecentPlayers.Insert(Temp, 0);
			SaveSocialData();
			return;
		}
	}

	// Add new entry
	FMGRecentPlayer NewRecent;
	NewRecent.PlayerID = PlayerID;
	NewRecent.DisplayName = DisplayName;
	NewRecent.LastRaced = FDateTime::Now();
	NewRecent.TrackID = TrackID;
	NewRecent.TheirPosition = TheirPosition;
	NewRecent.OurPosition = OurPosition;
	NewRecent.bIsFriend = IsFriend(PlayerID);
	NewRecent.bIsBlocked = BlockedPlayers.Contains(PlayerID);

	RecentPlayers.Insert(NewRecent, 0);

	// Trim if too many
	while (RecentPlayers.Num() > MaxRecentPlayers)
	{
		RecentPlayers.RemoveAt(RecentPlayers.Num() - 1);
	}

	SaveSocialData();
}

void UMGSocialSubsystem::ClearRecentPlayers()
{
	RecentPlayers.Empty();
	SaveSocialData();
}

// ==========================================
// STATUS
// ==========================================

void UMGSocialSubsystem::SetStatus(EMGFriendStatus Status, const FString& StatusText)
{
	OwnStatus = Status;
	// Would broadcast to friends through network
}

FText UMGSocialSubsystem::GetStatusDisplayText(EMGFriendStatus Status)
{
	switch (Status)
	{
		case EMGFriendStatus::Offline: return FText::FromString(TEXT("Offline"));
		case EMGFriendStatus::Online: return FText::FromString(TEXT("Online"));
		case EMGFriendStatus::InGarage: return FText::FromString(TEXT("In Garage"));
		case EMGFriendStatus::InLobby: return FText::FromString(TEXT("In Lobby"));
		case EMGFriendStatus::Racing: return FText::FromString(TEXT("Racing"));
		case EMGFriendStatus::WatchingReplay: return FText::FromString(TEXT("Watching Replay"));
		case EMGFriendStatus::Away: return FText::FromString(TEXT("Away"));
		default: return FText::FromString(TEXT("Unknown"));
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGSocialSubsystem::LoadSocialData()
{
	// Would load from save file
}

void UMGSocialSubsystem::SaveSocialData()
{
	// Would save to persistent storage
}

void UMGSocialSubsystem::GenerateMockFriends()
{
	const TArray<TPair<FString, EMGFriendStatus>> MockFriends = {
		{TEXT("NightRider_X"), EMGFriendStatus::Racing},
		{TEXT("DriftKing99"), EMGFriendStatus::Online},
		{TEXT("SpeedDemon"), EMGFriendStatus::InLobby},
		{TEXT("MidnightRacer"), EMGFriendStatus::Offline},
		{TEXT("NeonPhantom"), EMGFriendStatus::InGarage},
		{TEXT("TurboTony"), EMGFriendStatus::Online},
		{TEXT("StreetLegend"), EMGFriendStatus::Offline},
		{TEXT("GhostRunner"), EMGFriendStatus::Racing}
	};

	for (int32 i = 0; i < MockFriends.Num(); i++)
	{
		FMGFriendData Friend;
		Friend.PlayerID = FString::Printf(TEXT("player_%d"), i);
		Friend.DisplayName = MockFriends[i].Key;
		Friend.Status = MockFriends[i].Value;
		Friend.Level = FMath::RandRange(1, 50);
		Friend.Reputation = FMath::RandRange(100, 50000);
		Friend.TotalWins = FMath::RandRange(0, 200);
		Friend.LastOnline = FDateTime::Now() - FTimespan::FromHours(FMath::RandRange(0, 168));
		Friend.bIsFavorite = i < 2;
		Friend.bCanJoin = (Friend.Status == EMGFriendStatus::InLobby || Friend.Status == EMGFriendStatus::Racing);

		if (Friend.Status == EMGFriendStatus::Racing)
		{
			Friend.CurrentTrack = FName(*FString::Printf(TEXT("Track_%d"), FMath::RandRange(1, 5)));
			Friend.SessionID = FGuid::NewGuid().ToString();
			Friend.StatusText = TEXT("Racing on Downtown Circuit");
		}
		else if (Friend.Status == EMGFriendStatus::InLobby)
		{
			Friend.SessionID = FGuid::NewGuid().ToString();
			Friend.StatusText = TEXT("In Lobby - 3/8 players");
		}

		// Some have crews
		if (FMath::RandBool())
		{
			Friend.CrewName = TEXT("Midnight Runners");
		}

		Friends.Add(Friend);
	}
}

void UMGSocialSubsystem::GenerateMockCrew()
{
	// Create a mock crew
	CurrentCrew.CrewID = FGuid::NewGuid().ToString();
	CurrentCrew.CrewName = TEXT("Midnight Runners");
	CurrentCrew.CrewTag = TEXT("MR");
	CurrentCrew.Description = TEXT("Late night street racing crew. Race hard, race fast.");
	CurrentCrew.Level = 15;
	CurrentCrew.CrewXP = 12500;
	CurrentCrew.NextLevelXP = 15000;
	CurrentCrew.MemberCount = 24;
	CurrentCrew.MaxMembers = 50;
	CurrentCrew.TotalWins = 1847;
	CurrentCrew.WeeklyRank = 47;
	CurrentCrew.bIsRecruiting = true;
	CurrentCrew.CreatedDate = FDateTime::Now() - FTimespan::FromDays(180);
	CurrentCrew.CrewColor = FLinearColor(0.8f, 0.2f, 1.0f);

	PlayerCrewRank = EMGCrewRank::Officer;

	// Add some mock members
	const TArray<FString> MemberNames = {
		TEXT("CrewLeader"), TEXT("You"), TEXT("NightRider_X"), TEXT("DriftKing99"),
		TEXT("SpeedDemon"), TEXT("NeonPhantom"), TEXT("TurboTony")
	};

	for (int32 i = 0; i < MemberNames.Num(); i++)
	{
		FMGCrewMember Member;
		Member.PlayerID = FString::Printf(TEXT("crew_member_%d"), i);
		Member.DisplayName = MemberNames[i];
		Member.Rank = (i == 0) ? EMGCrewRank::Leader : (i < 3) ? EMGCrewRank::Officer : EMGCrewRank::Member;
		Member.JoinDate = FDateTime::Now() - FTimespan::FromDays(FMath::RandRange(1, 180));
		Member.WeeklyContribution = FMath::RandRange(0, 5000);
		Member.TotalContribution = FMath::RandRange(1000, 50000);
		Member.Status = (i < 4) ? EMGFriendStatus::Online : EMGFriendStatus::Offline;
		Member.Level = FMath::RandRange(10, 50);

		CurrentCrew.Members.Add(Member);
	}
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Social/MGCrewSubsystem.h"

void UMGCrewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializePerks();
	CreateMockData();
}

void UMGCrewSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// CREW STATE
// ==========================================

bool UMGCrewSubsystem::CanPerformAction(EMGCrewRole RequiredRole) const
{
	return static_cast<int32>(MyRole) >= static_cast<int32>(RequiredRole);
}

// ==========================================
// CREW MANAGEMENT
// ==========================================

bool UMGCrewSubsystem::CreateCrew(FText Name, FString Tag, FText Description)
{
	if (bIsInCrew)
	{
		return false;
	}

	if (!IsValidCrewTag(Tag))
	{
		return false;
	}

	// Create crew
	CurrentCrew = FMGCrewData();
	CurrentCrew.CrewID = FGuid::NewGuid().ToString();
	CurrentCrew.Name = Name;
	CurrentCrew.Tag = Tag.ToUpper();
	CurrentCrew.Description = Description;
	CurrentCrew.CreationDate = FDateTime::UtcNow();
	CurrentCrew.Level = 1;
	CurrentCrew.XP = 0;
	CurrentCrew.XPToNextLevel = GetXPForCrewLevel(2);
	CurrentCrew.MemberCount = 1;
	CurrentCrew.MaxMembers = 50;

	// Add self as leader
	FMGCrewMember SelfMember;
	SelfMember.PlayerID = TEXT("LocalPlayer");
	SelfMember.DisplayName = NSLOCTEXT("Crew", "You", "You");
	SelfMember.Role = EMGCrewRole::Leader;
	SelfMember.JoinDate = FDateTime::UtcNow();
	SelfMember.bIsOnline = true;
	CurrentCrew.Members.Add(SelfMember);

	MyRole = EMGCrewRole::Leader;
	bIsInCrew = true;

	UpdatePerkStatus();

	AddActivityToFeed(EMGCrewActivityType::MemberJoined, SelfMember.DisplayName,
		NSLOCTEXT("Crew", "CreatedCrew", "created the crew"));

	OnCrewJoined.Broadcast(CurrentCrew);

	return true;
}

bool UMGCrewSubsystem::LeaveCrew()
{
	if (!bIsInCrew)
	{
		return false;
	}

	// Leaders must transfer leadership first
	if (MyRole == EMGCrewRole::Leader && CurrentCrew.MemberCount > 1)
	{
		return false;
	}

	// If leader and only member, disband
	if (MyRole == EMGCrewRole::Leader && CurrentCrew.MemberCount == 1)
	{
		return DisbandCrew();
	}

	bIsInCrew = false;
	CurrentCrew = FMGCrewData();
	MyRole = EMGCrewRole::Member;

	OnCrewLeft.Broadcast();

	return true;
}

bool UMGCrewSubsystem::DisbandCrew()
{
	if (!bIsInCrew || MyRole != EMGCrewRole::Leader)
	{
		return false;
	}

	bIsInCrew = false;
	CurrentCrew = FMGCrewData();
	MyRole = EMGCrewRole::Member;

	OnCrewLeft.Broadcast();

	return true;
}

bool UMGCrewSubsystem::UpdateCrewSettings(const FMGCrewData& NewSettings)
{
	if (!bIsInCrew || !CanPerformAction(EMGCrewRole::CoLeader))
	{
		return false;
	}

	// Update allowed settings
	CurrentCrew.Name = NewSettings.Name;
	CurrentCrew.Description = NewSettings.Description;
	CurrentCrew.Motto = NewSettings.Motto;
	CurrentCrew.JoinType = NewSettings.JoinType;
	CurrentCrew.MinimumLevel = NewSettings.MinimumLevel;
	CurrentCrew.PrimaryColor = NewSettings.PrimaryColor;
	CurrentCrew.SecondaryColor = NewSettings.SecondaryColor;

	if (IsValidCrewTag(NewSettings.Tag))
	{
		CurrentCrew.Tag = NewSettings.Tag.ToUpper();
	}

	OnCrewUpdated.Broadcast(CurrentCrew);

	return true;
}

// ==========================================
// MEMBERSHIP
// ==========================================

bool UMGCrewSubsystem::RequestToJoinCrew(const FString& CrewID, FText Message)
{
	if (bIsInCrew)
	{
		return false;
	}

	// In production, this would send to server
	return true;
}

bool UMGCrewSubsystem::AcceptJoinRequest(const FString& RequestID)
{
	if (!bIsInCrew || !CanPerformAction(EMGCrewRole::Officer))
	{
		return false;
	}

	int32 Index = PendingJoinRequests.IndexOfByPredicate([RequestID](const FMGCrewJoinRequest& R)
	{
		return R.RequestID == RequestID;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	FMGCrewJoinRequest Request = PendingJoinRequests[Index];
	PendingJoinRequests.RemoveAt(Index);

	// Add new member
	FMGCrewMember NewMember;
	NewMember.PlayerID = Request.PlayerID;
	NewMember.DisplayName = Request.PlayerName;
	NewMember.Role = EMGCrewRole::Member;
	NewMember.JoinDate = FDateTime::UtcNow();
	NewMember.bIsOnline = true;

	CurrentCrew.Members.Add(NewMember);
	CurrentCrew.MemberCount++;

	AddActivityToFeed(EMGCrewActivityType::MemberJoined, Request.PlayerName,
		NSLOCTEXT("Crew", "JoinedCrew", "joined the crew"));

	OnCrewMemberJoined.Broadcast(NewMember);
	OnCrewUpdated.Broadcast(CurrentCrew);

	return true;
}

bool UMGCrewSubsystem::DeclineJoinRequest(const FString& RequestID)
{
	if (!bIsInCrew || !CanPerformAction(EMGCrewRole::Officer))
	{
		return false;
	}

	PendingJoinRequests.RemoveAll([RequestID](const FMGCrewJoinRequest& R)
	{
		return R.RequestID == RequestID;
	});

	return true;
}

bool UMGCrewSubsystem::InvitePlayer(const FString& PlayerID)
{
	if (!bIsInCrew || !CanPerformAction(EMGCrewRole::Officer))
	{
		return false;
	}

	if (CurrentCrew.MemberCount >= CurrentCrew.MaxMembers)
	{
		return false;
	}

	// In production, this would send invite to player
	return true;
}

bool UMGCrewSubsystem::AcceptCrewInvite(const FString& InviteID)
{
	if (bIsInCrew)
	{
		return false;
	}

	int32 Index = PendingInvites.IndexOfByPredicate([InviteID](const FMGCrewInvite& I)
	{
		return I.InviteID == InviteID;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	FMGCrewInvite Invite = PendingInvites[Index];
	PendingInvites.RemoveAt(Index);

	// In production, this would fetch crew data from server and join
	bIsInCrew = true;
	MyRole = EMGCrewRole::Member;

	return true;
}

bool UMGCrewSubsystem::DeclineCrewInvite(const FString& InviteID)
{
	PendingInvites.RemoveAll([InviteID](const FMGCrewInvite& I)
	{
		return I.InviteID == InviteID;
	});

	return true;
}

bool UMGCrewSubsystem::KickMember(const FString& PlayerID)
{
	if (!bIsInCrew || !CanPerformAction(EMGCrewRole::Officer))
	{
		return false;
	}

	int32 Index = CurrentCrew.Members.IndexOfByPredicate([PlayerID](const FMGCrewMember& M)
	{
		return M.PlayerID == PlayerID;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	FMGCrewMember Member = CurrentCrew.Members[Index];

	// Can't kick someone of equal or higher rank
	if (static_cast<int32>(Member.Role) >= static_cast<int32>(MyRole))
	{
		return false;
	}

	CurrentCrew.Members.RemoveAt(Index);
	CurrentCrew.MemberCount--;

	AddActivityToFeed(EMGCrewActivityType::MemberLeft, Member.DisplayName,
		NSLOCTEXT("Crew", "WasKicked", "was kicked from the crew"));

	OnCrewMemberLeft.Broadcast(Member);
	OnCrewUpdated.Broadcast(CurrentCrew);

	return true;
}

bool UMGCrewSubsystem::PromoteMember(const FString& PlayerID)
{
	if (!bIsInCrew || !CanPerformAction(EMGCrewRole::CoLeader))
	{
		return false;
	}

	FMGCrewMember* Member = CurrentCrew.Members.FindByPredicate([PlayerID](const FMGCrewMember& M)
	{
		return M.PlayerID == PlayerID;
	});

	if (!Member)
	{
		return false;
	}

	// Can't promote beyond your own rank - 1
	EMGCrewRole MaxPromotion = static_cast<EMGCrewRole>(static_cast<int32>(MyRole) - 1);

	if (Member->Role >= MaxPromotion)
	{
		return false;
	}

	EMGCrewRole OldRole = Member->Role;
	Member->Role = static_cast<EMGCrewRole>(static_cast<int32>(Member->Role) + 1);

	AddActivityToFeed(EMGCrewActivityType::MemberPromoted, Member->DisplayName,
		FText::Format(NSLOCTEXT("Crew", "Promoted", "was promoted to {0}"), GetRoleDisplayName(Member->Role)));

	OnCrewUpdated.Broadcast(CurrentCrew);

	return true;
}

bool UMGCrewSubsystem::DemoteMember(const FString& PlayerID)
{
	if (!bIsInCrew || !CanPerformAction(EMGCrewRole::CoLeader))
	{
		return false;
	}

	FMGCrewMember* Member = CurrentCrew.Members.FindByPredicate([PlayerID](const FMGCrewMember& M)
	{
		return M.PlayerID == PlayerID;
	});

	if (!Member || Member->Role == EMGCrewRole::Member)
	{
		return false;
	}

	// Can't demote someone of equal or higher rank
	if (static_cast<int32>(Member->Role) >= static_cast<int32>(MyRole))
	{
		return false;
	}

	Member->Role = static_cast<EMGCrewRole>(static_cast<int32>(Member->Role) - 1);

	AddActivityToFeed(EMGCrewActivityType::MemberDemoted, Member->DisplayName,
		FText::Format(NSLOCTEXT("Crew", "Demoted", "was demoted to {0}"), GetRoleDisplayName(Member->Role)));

	OnCrewUpdated.Broadcast(CurrentCrew);

	return true;
}

bool UMGCrewSubsystem::TransferLeadership(const FString& PlayerID)
{
	if (!bIsInCrew || MyRole != EMGCrewRole::Leader)
	{
		return false;
	}

	FMGCrewMember* NewLeader = CurrentCrew.Members.FindByPredicate([PlayerID](const FMGCrewMember& M)
	{
		return M.PlayerID == PlayerID;
	});

	if (!NewLeader)
	{
		return false;
	}

	// Find self and demote
	FMGCrewMember* Self = CurrentCrew.Members.FindByPredicate([](const FMGCrewMember& M)
	{
		return M.PlayerID == TEXT("LocalPlayer");
	});

	if (Self)
	{
		Self->Role = EMGCrewRole::CoLeader;
	}

	NewLeader->Role = EMGCrewRole::Leader;
	MyRole = EMGCrewRole::CoLeader;

	AddActivityToFeed(EMGCrewActivityType::MemberPromoted, NewLeader->DisplayName,
		NSLOCTEXT("Crew", "BecameLeader", "became the crew leader"));

	OnCrewUpdated.Broadcast(CurrentCrew);

	return true;
}

// ==========================================
// SEARCH
// ==========================================

TArray<FMGCrewSearchResult> UMGCrewSubsystem::SearchCrews(const FString& Query, int32 MaxResults)
{
	TArray<FMGCrewSearchResult> Results;

	// Mock search results
	for (int32 i = 0; i < FMath::Min(MaxResults, 5); i++)
	{
		FMGCrewSearchResult Result;
		Result.CrewID = FString::Printf(TEXT("SearchCrew_%d"), i);
		Result.Name = FText::Format(NSLOCTEXT("Crew", "SearchResult", "{0} Racing Team"), FText::FromString(Query));
		Result.Tag = TEXT("SRT");
		Result.Level = FMath::RandRange(1, 50);
		Result.MemberCount = FMath::RandRange(5, 45);
		Result.MaxMembers = 50;
		Result.JoinType = EMGCrewJoinType::RequestOnly;
		Result.WeeklyRank = FMath::RandRange(1, 1000);

		Results.Add(Result);
	}

	return Results;
}

TArray<FMGCrewSearchResult> UMGCrewSubsystem::GetRecommendedCrews()
{
	TArray<FMGCrewSearchResult> Results;

	// Mock recommended crews
	TArray<FString> CrewNames = { TEXT("Speed Demons"), TEXT("Night Riders"), TEXT("Urban Legends"), TEXT("Drift Kings"), TEXT("Street Elite") };

	for (int32 i = 0; i < CrewNames.Num(); i++)
	{
		FMGCrewSearchResult Result;
		Result.CrewID = FString::Printf(TEXT("RecCrew_%d"), i);
		Result.Name = FText::FromString(CrewNames[i]);
		Result.Tag = CrewNames[i].Left(3).ToUpper();
		Result.Level = FMath::RandRange(10, 30);
		Result.MemberCount = FMath::RandRange(20, 45);
		Result.MaxMembers = 50;
		Result.JoinType = EMGCrewJoinType::Open;
		Result.WeeklyRank = FMath::RandRange(50, 500);

		Results.Add(Result);
	}

	return Results;
}

TArray<FMGCrewSearchResult> UMGCrewSubsystem::GetTopCrews(int32 Count)
{
	TArray<FMGCrewSearchResult> Results;

	for (int32 i = 0; i < Count; i++)
	{
		FMGCrewSearchResult Result;
		Result.CrewID = FString::Printf(TEXT("TopCrew_%d"), i);
		Result.Name = FText::Format(NSLOCTEXT("Crew", "TopCrew", "Top Crew #{0}"), FText::AsNumber(i + 1));
		Result.Tag = FString::Printf(TEXT("T%02d"), i + 1);
		Result.Level = 50 - i;
		Result.MemberCount = 50;
		Result.MaxMembers = 50;
		Result.JoinType = EMGCrewJoinType::InviteOnly;
		Result.WeeklyRank = i + 1;

		Results.Add(Result);
	}

	return Results;
}

// ==========================================
// LIVERIES
// ==========================================

bool UMGCrewSubsystem::ShareLivery(const FMGSharedLivery& Livery)
{
	if (!bIsInCrew)
	{
		return false;
	}

	FMGSharedLivery NewLivery = Livery;
	NewLivery.LiveryID = FGuid::NewGuid().ToString();
	NewLivery.CreatorName = NSLOCTEXT("Crew", "You", "You");
	NewLivery.UploadDate = FDateTime::UtcNow();

	CurrentCrew.SharedLiveries.Add(NewLivery);

	AddActivityToFeed(EMGCrewActivityType::LiveryShared, NewLivery.CreatorName,
		FText::Format(NSLOCTEXT("Crew", "SharedLivery", "shared a livery: {0}"), NewLivery.DisplayName));

	OnCrewUpdated.Broadcast(CurrentCrew);

	return true;
}

bool UMGCrewSubsystem::DownloadLivery(const FString& LiveryID)
{
	FMGSharedLivery* Livery = CurrentCrew.SharedLiveries.FindByPredicate([LiveryID](const FMGSharedLivery& L)
	{
		return L.LiveryID == LiveryID;
	});

	if (Livery)
	{
		Livery->Downloads++;
		return true;
	}

	return false;
}

void UMGCrewSubsystem::LikeLivery(const FString& LiveryID)
{
	FMGSharedLivery* Livery = CurrentCrew.SharedLiveries.FindByPredicate([LiveryID](const FMGSharedLivery& L)
	{
		return L.LiveryID == LiveryID;
	});

	if (Livery)
	{
		Livery->Likes++;
	}
}

bool UMGCrewSubsystem::DeleteSharedLivery(const FString& LiveryID)
{
	if (!bIsInCrew)
	{
		return false;
	}

	// Find livery
	int32 Index = CurrentCrew.SharedLiveries.IndexOfByPredicate([LiveryID](const FMGSharedLivery& L)
	{
		return L.LiveryID == LiveryID;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	// Check permissions (owner or officer+)
	// For now, allow if officer+
	if (!CanPerformAction(EMGCrewRole::Officer))
	{
		return false;
	}

	CurrentCrew.SharedLiveries.RemoveAt(Index);
	OnCrewUpdated.Broadcast(CurrentCrew);

	return true;
}

// ==========================================
// CREW VS CREW
// ==========================================

bool UMGCrewSubsystem::StartCrewBattle(const FString& OpponentCrewID)
{
	if (!bIsInCrew || !CanPerformAction(EMGCrewRole::Officer))
	{
		return false;
	}

	if (CurrentCrew.ActiveBattle.bIsActive)
	{
		return false;
	}

	CurrentCrew.ActiveBattle.BattleID = FGuid::NewGuid().ToString();
	CurrentCrew.ActiveBattle.OpponentCrewID = OpponentCrewID;
	CurrentCrew.ActiveBattle.OpponentCrewName = NSLOCTEXT("Crew", "OpponentCrew", "Opponent Crew");
	CurrentCrew.ActiveBattle.OurScore = 0;
	CurrentCrew.ActiveBattle.TheirScore = 0;
	CurrentCrew.ActiveBattle.StartTime = FDateTime::UtcNow();
	CurrentCrew.ActiveBattle.EndTime = FDateTime::UtcNow() + FTimespan::FromHours(24);
	CurrentCrew.ActiveBattle.bIsActive = true;

	OnCrewBattleStarted.Broadcast(CurrentCrew.ActiveBattle);

	return true;
}

FMGCrewBattle UMGCrewSubsystem::GetCurrentBattle() const
{
	return CurrentCrew.ActiveBattle;
}

void UMGCrewSubsystem::ReportBattleScore(int32 ScoreEarned)
{
	if (!bIsInCrew || !CurrentCrew.ActiveBattle.bIsActive)
	{
		return;
	}

	CurrentCrew.ActiveBattle.OurScore += ScoreEarned;

	// Check if battle ended
	FDateTime Now = FDateTime::UtcNow();
	if (Now >= CurrentCrew.ActiveBattle.EndTime)
	{
		CurrentCrew.ActiveBattle.bIsActive = false;
		CurrentCrew.ActiveBattle.bDidWin = CurrentCrew.ActiveBattle.OurScore > CurrentCrew.ActiveBattle.TheirScore;

		if (CurrentCrew.ActiveBattle.bDidWin)
		{
			CurrentCrew.TotalWins++;
			CurrentCrew.CrewTokens += 100;

			AddActivityToFeed(EMGCrewActivityType::CrewBattleWin, FText::GetEmpty(),
				FText::Format(NSLOCTEXT("Crew", "BattleWon", "Won crew battle against {0}!"), CurrentCrew.ActiveBattle.OpponentCrewName));
		}

		OnCrewBattleEnded.Broadcast(CurrentCrew.ActiveBattle, CurrentCrew.ActiveBattle.bDidWin);
	}

	OnCrewUpdated.Broadcast(CurrentCrew);
}

// ==========================================
// CONTRIBUTION
// ==========================================

void UMGCrewSubsystem::ContributeXP(int32 Amount)
{
	if (!bIsInCrew)
	{
		return;
	}

	CurrentCrew.XP += Amount;

	// Update member contribution
	FMGCrewMember* Self = CurrentCrew.Members.FindByPredicate([](const FMGCrewMember& M)
	{
		return M.PlayerID == TEXT("LocalPlayer");
	});

	if (Self)
	{
		Self->XPContributed += Amount;
	}

	CheckCrewLevelUp();
	OnCrewUpdated.Broadcast(CurrentCrew);
}

void UMGCrewSubsystem::ContributeToChallenge(FName ChallengeID, int32 Amount)
{
	if (!bIsInCrew)
	{
		return;
	}

	FMGCrewChallenge* Challenge = CurrentCrew.ActiveChallenges.FindByPredicate([ChallengeID](const FMGCrewChallenge& C)
	{
		return C.ChallengeID == ChallengeID;
	});

	if (Challenge && !Challenge->bIsCompleted)
	{
		Challenge->CurrentValue += Amount;

		if (Challenge->CurrentValue >= Challenge->TargetValue)
		{
			Challenge->bIsCompleted = true;
			CurrentCrew.XP += Challenge->XPReward;
			CurrentCrew.CrewTokens += Challenge->TokenReward;

			AddActivityToFeed(EMGCrewActivityType::ChallengeCompleted, FText::GetEmpty(),
				FText::Format(NSLOCTEXT("Crew", "ChallengeComplete", "Completed challenge: {0}"), Challenge->DisplayName));

			CheckCrewLevelUp();
		}

		OnCrewUpdated.Broadcast(CurrentCrew);
	}
}

// ==========================================
// PERKS
// ==========================================

float UMGCrewSubsystem::GetPerkValue(FName PerkID) const
{
	if (!bIsInCrew)
	{
		return 0.0f;
	}

	const FMGCrewPerk* Perk = CurrentCrew.Perks.FindByPredicate([PerkID](const FMGCrewPerk& P)
	{
		return P.PerkID == PerkID && P.bIsUnlocked;
	});

	return Perk ? Perk->Value : 0.0f;
}

bool UMGCrewSubsystem::IsPerkUnlocked(FName PerkID) const
{
	if (!bIsInCrew)
	{
		return false;
	}

	const FMGCrewPerk* Perk = CurrentCrew.Perks.FindByPredicate([PerkID](const FMGCrewPerk& P)
	{
		return P.PerkID == PerkID;
	});

	return Perk && Perk->bIsUnlocked;
}

// ==========================================
// UTILITY
// ==========================================

FText UMGCrewSubsystem::GetRoleDisplayName(EMGCrewRole Role)
{
	switch (Role)
	{
	case EMGCrewRole::Member:
		return NSLOCTEXT("Crew", "Member", "Member");
	case EMGCrewRole::Veteran:
		return NSLOCTEXT("Crew", "Veteran", "Veteran");
	case EMGCrewRole::Officer:
		return NSLOCTEXT("Crew", "Officer", "Officer");
	case EMGCrewRole::CoLeader:
		return NSLOCTEXT("Crew", "CoLeader", "Co-Leader");
	case EMGCrewRole::Leader:
		return NSLOCTEXT("Crew", "Leader", "Leader");
	default:
		return FText::GetEmpty();
	}
}

int64 UMGCrewSubsystem::GetXPForCrewLevel(int32 Level)
{
	// Exponential growth
	return static_cast<int64>(1000.0 * FMath::Pow(1.5, Level - 1));
}

bool UMGCrewSubsystem::IsValidCrewTag(const FString& Tag)
{
	if (Tag.Len() < 2 || Tag.Len() > 4)
	{
		return false;
	}

	// Alphanumeric only
	for (TCHAR Char : Tag)
	{
		if (!FChar::IsAlnum(Char))
		{
			return false;
		}
	}

	return true;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGCrewSubsystem::InitializePerks()
{
	AllPerks.Empty();

	// XP Boost
	{
		FMGCrewPerk Perk;
		Perk.PerkID = TEXT("XPBoost");
		Perk.DisplayName = NSLOCTEXT("Perks", "XPBoost", "XP Boost");
		Perk.Description = NSLOCTEXT("Perks", "XPBoostDesc", "Earn 5% bonus XP from races");
		Perk.RequiredLevel = 5;
		Perk.Value = 5.0f;
		Perk.bIsPercentage = true;
		AllPerks.Add(Perk);
	}

	// Cash Boost
	{
		FMGCrewPerk Perk;
		Perk.PerkID = TEXT("CashBoost");
		Perk.DisplayName = NSLOCTEXT("Perks", "CashBoost", "Cash Boost");
		Perk.Description = NSLOCTEXT("Perks", "CashBoostDesc", "Earn 5% bonus cash from races");
		Perk.RequiredLevel = 10;
		Perk.Value = 5.0f;
		Perk.bIsPercentage = true;
		AllPerks.Add(Perk);
	}

	// Rep Boost
	{
		FMGCrewPerk Perk;
		Perk.PerkID = TEXT("RepBoost");
		Perk.DisplayName = NSLOCTEXT("Perks", "RepBoost", "Reputation Boost");
		Perk.Description = NSLOCTEXT("Perks", "RepBoostDesc", "Earn 10% bonus reputation");
		Perk.RequiredLevel = 15;
		Perk.Value = 10.0f;
		Perk.bIsPercentage = true;
		AllPerks.Add(Perk);
	}

	// Livery Slots
	{
		FMGCrewPerk Perk;
		Perk.PerkID = TEXT("LiverySlots");
		Perk.DisplayName = NSLOCTEXT("Perks", "LiverySlots", "Extra Livery Slots");
		Perk.Description = NSLOCTEXT("Perks", "LiverySlotsDesc", "+10 shared livery slots");
		Perk.RequiredLevel = 20;
		Perk.Value = 10.0f;
		Perk.bIsPercentage = false;
		AllPerks.Add(Perk);
	}

	// Member Capacity
	{
		FMGCrewPerk Perk;
		Perk.PerkID = TEXT("MemberCapacity");
		Perk.DisplayName = NSLOCTEXT("Perks", "MemberCapacity", "Increased Capacity");
		Perk.Description = NSLOCTEXT("Perks", "MemberCapacityDesc", "+25 maximum members");
		Perk.RequiredLevel = 30;
		Perk.Value = 25.0f;
		Perk.bIsPercentage = false;
		AllPerks.Add(Perk);
	}

	// XP Boost II
	{
		FMGCrewPerk Perk;
		Perk.PerkID = TEXT("XPBoost2");
		Perk.DisplayName = NSLOCTEXT("Perks", "XPBoost2", "XP Boost II");
		Perk.Description = NSLOCTEXT("Perks", "XPBoost2Desc", "Additional 5% bonus XP (10% total)");
		Perk.RequiredLevel = 40;
		Perk.Value = 5.0f;
		Perk.bIsPercentage = true;
		AllPerks.Add(Perk);
	}
}

void UMGCrewSubsystem::UpdatePerkStatus()
{
	CurrentCrew.Perks = AllPerks;

	for (FMGCrewPerk& Perk : CurrentCrew.Perks)
	{
		Perk.bIsUnlocked = CurrentCrew.Level >= Perk.RequiredLevel;
	}

	// Update max members based on perk
	CurrentCrew.MaxMembers = 50;
	if (IsPerkUnlocked(TEXT("MemberCapacity")))
	{
		CurrentCrew.MaxMembers += static_cast<int32>(GetPerkValue(TEXT("MemberCapacity")));
	}
}

void UMGCrewSubsystem::CheckCrewLevelUp()
{
	while (CurrentCrew.XP >= CurrentCrew.XPToNextLevel)
	{
		CurrentCrew.XP -= CurrentCrew.XPToNextLevel;
		CurrentCrew.Level++;
		CurrentCrew.XPToNextLevel = GetXPForCrewLevel(CurrentCrew.Level + 1);

		UpdatePerkStatus();

		AddActivityToFeed(EMGCrewActivityType::CrewLevelUp, FText::GetEmpty(),
			FText::Format(NSLOCTEXT("Crew", "LevelUp", "Crew reached level {0}!"), FText::AsNumber(CurrentCrew.Level)));

		OnCrewLevelUp.Broadcast(CurrentCrew.Level);
	}
}

void UMGCrewSubsystem::AddActivityToFeed(EMGCrewActivityType Type, FText PlayerName, FText Description, int32 Value)
{
	FMGCrewActivity Activity;
	Activity.Type = Type;
	Activity.PlayerName = PlayerName;
	Activity.Description = Description;
	Activity.Timestamp = FDateTime::UtcNow();
	Activity.Value = Value;

	CurrentCrew.ActivityFeed.Insert(Activity, 0);

	// Limit feed size
	if (CurrentCrew.ActivityFeed.Num() > 50)
	{
		CurrentCrew.ActivityFeed.SetNum(50);
	}
}

void UMGCrewSubsystem::CreateMockData()
{
	// Mock pending invites
	{
		FMGCrewInvite Invite;
		Invite.InviteID = TEXT("Invite_001");
		Invite.CrewID = TEXT("MockCrew_001");
		Invite.CrewName = NSLOCTEXT("Crew", "MockCrewName", "Midnight Riders");
		Invite.InviterName = NSLOCTEXT("Crew", "MockInviter", "SpeedDemon99");
		Invite.InviteDate = FDateTime::UtcNow() - FTimespan::FromHours(2);
		Invite.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromDays(7);

		PendingInvites.Add(Invite);
	}
}

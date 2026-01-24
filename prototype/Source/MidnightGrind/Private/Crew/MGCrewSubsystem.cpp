// Copyright Midnight Grind. All Rights Reserved.

#include "Crew/MGCrewSubsystem.h"

void UMGCrewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MaxActivityFeedSize = 200;
	InitializePerks();
	LoadCrewData();
}

void UMGCrewSubsystem::Deinitialize()
{
	SaveCrewData();
	Super::Deinitialize();
}

bool UMGCrewSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

bool UMGCrewSubsystem::CreateCrew(const FString& Name, const FString& Tag, const FString& Description)
{
	if (IsInCrew())
	{
		return false;
	}

	if (Name.Len() < 3 || Name.Len() > 32)
	{
		return false;
	}

	if (Tag.Len() < 2 || Tag.Len() > 5)
	{
		return false;
	}

	CurrentCrew.CrewID = FGuid::NewGuid();
	CurrentCrew.CrewName = Name;
	CurrentCrew.CrewTag = Tag;
	CurrentCrew.Description = Description;
	CurrentCrew.CreatedDate = FDateTime::UtcNow();
	CurrentCrew.Level = 1;
	CurrentCrew.ExperiencePoints = 0;
	CurrentCrew.ExperienceToNextLevel = CalculateExperienceForLevel(2);
	CurrentCrew.MaxMembers = 50;
	CurrentCrew.CurrentMembers = 1;
	CurrentCrew.Privacy = EMGCrewPrivacy::ApprovalRequired;

	// Add creator as leader
	FMGCrewMember LeaderMember;
	LeaderMember.PlayerID = LocalPlayerID;
	LeaderMember.Rank = EMGCrewRank::Leader;
	LeaderMember.JoinedDate = FDateTime::UtcNow();
	LeaderMember.LastActiveDate = FDateTime::UtcNow();
	LeaderMember.bIsOnline = true;
	Members.Add(LeaderMember);

	RecordActivity(EMGCrewActivityType::MemberJoined, FText::FromString(TEXT("Crew created!")), 100);

	OnCrewJoined.Broadcast(CurrentCrew);

	return true;
}

bool UMGCrewSubsystem::DisbandCrew()
{
	if (!IsInCrew() || !IsCrewLeader())
	{
		return false;
	}

	CurrentCrew = FMGCrewInfo();
	Members.Empty();
	PendingApplications.Empty();
	ActivityFeed.Empty();
	ActiveChallenges.Empty();

	OnCrewLeft.Broadcast();

	return true;
}

bool UMGCrewSubsystem::LeaveCrew()
{
	if (!IsInCrew())
	{
		return false;
	}

	if (IsCrewLeader() && Members.Num() > 1)
	{
		// Must transfer leadership first
		return false;
	}

	RecordActivity(EMGCrewActivityType::MemberLeft,
		FText::FromString(FString::Printf(TEXT("%s left the crew"), *GetLocalMember().PlayerName)), 0);

	OnCrewMemberLeft.Broadcast(LocalPlayerID);

	CurrentCrew = FMGCrewInfo();
	Members.Empty();

	OnCrewLeft.Broadcast();

	return true;
}

bool UMGCrewSubsystem::IsCrewLeader() const
{
	FMGCrewMember LocalMember = GetLocalMember();
	return LocalMember.Rank == EMGCrewRank::Leader;
}

bool UMGCrewSubsystem::IsCrewOfficer() const
{
	FMGCrewMember LocalMember = GetLocalMember();
	return LocalMember.Rank >= EMGCrewRank::Officer;
}

TArray<FMGCrewMember> UMGCrewSubsystem::GetOnlineMembers() const
{
	TArray<FMGCrewMember> OnlineMembers;
	for (const FMGCrewMember& Member : Members)
	{
		if (Member.bIsOnline)
		{
			OnlineMembers.Add(Member);
		}
	}
	return OnlineMembers;
}

FMGCrewMember UMGCrewSubsystem::GetMember(FName PlayerID) const
{
	for (const FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			return Member;
		}
	}
	return FMGCrewMember();
}

FMGCrewMember UMGCrewSubsystem::GetLocalMember() const
{
	return GetMember(LocalPlayerID);
}

bool UMGCrewSubsystem::KickMember(FName PlayerID)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	if (PlayerID == LocalPlayerID)
	{
		return false;
	}

	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (Members[i].PlayerID == PlayerID)
		{
			FMGCrewMember LocalMember = GetLocalMember();
			if (Members[i].Rank >= LocalMember.Rank)
			{
				return false;
			}

			FString KickedName = Members[i].PlayerName;
			Members.RemoveAt(i);
			CurrentCrew.CurrentMembers--;

			RecordActivity(EMGCrewActivityType::MemberLeft,
				FText::FromString(FString::Printf(TEXT("%s was removed from the crew"), *KickedName)), 0);

			OnCrewMemberLeft.Broadcast(PlayerID);
			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::PromoteMember(FName PlayerID, EMGCrewRank NewRank)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	FMGCrewMember LocalMember = GetLocalMember();
	if (NewRank >= LocalMember.Rank)
	{
		return false;
	}

	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			if (NewRank <= Member.Rank)
			{
				return false;
			}

			Member.Rank = NewRank;

			RecordActivity(EMGCrewActivityType::Promotion,
				FText::FromString(FString::Printf(TEXT("%s was promoted"), *Member.PlayerName)), 50);

			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::DemoteMember(FName PlayerID, EMGCrewRank NewRank)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			FMGCrewMember LocalMember = GetLocalMember();
			if (Member.Rank >= LocalMember.Rank)
			{
				return false;
			}

			if (NewRank >= Member.Rank)
			{
				return false;
			}

			Member.Rank = NewRank;
			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::TransferLeadership(FName NewLeaderID)
{
	if (!IsCrewLeader())
	{
		return false;
	}

	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == NewLeaderID)
		{
			Member.Rank = EMGCrewRank::Leader;
		}
		else if (Member.PlayerID == LocalPlayerID)
		{
			Member.Rank = EMGCrewRank::Captain;
		}
	}

	return true;
}

bool UMGCrewSubsystem::SendInvite(FName PlayerID)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	if (CurrentCrew.CurrentMembers >= CurrentCrew.MaxMembers)
	{
		return false;
	}

	// Create invite (would be sent through network)
	FMGCrewInvite Invite;
	Invite.InviteID = FGuid::NewGuid();
	Invite.CrewID = CurrentCrew.CrewID;
	Invite.CrewName = CurrentCrew.CrewName;
	Invite.InviterID = LocalPlayerID;
	Invite.InviterName = GetLocalMember().PlayerName;
	Invite.SentDate = FDateTime::UtcNow();
	Invite.ExpiresDate = FDateTime::UtcNow() + FTimespan::FromDays(7);

	return true;
}

bool UMGCrewSubsystem::AcceptInvite(FGuid InviteID)
{
	for (int32 i = 0; i < PendingInvites.Num(); ++i)
	{
		if (PendingInvites[i].InviteID == InviteID)
		{
			// Join the crew
			// This would require network communication to actually join
			PendingInvites.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UMGCrewSubsystem::DeclineInvite(FGuid InviteID)
{
	for (int32 i = 0; i < PendingInvites.Num(); ++i)
	{
		if (PendingInvites[i].InviteID == InviteID)
		{
			PendingInvites.RemoveAt(i);
			return true;
		}
	}
	return false;
}

bool UMGCrewSubsystem::ApplyToCrew(FGuid CrewID, const FString& Message)
{
	if (IsInCrew())
	{
		return false;
	}

	// Would send application through network
	return true;
}

bool UMGCrewSubsystem::AcceptApplication(FGuid ApplicationID)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	for (int32 i = 0; i < PendingApplications.Num(); ++i)
	{
		if (PendingApplications[i].ApplicationID == ApplicationID)
		{
			if (CurrentCrew.CurrentMembers >= CurrentCrew.MaxMembers)
			{
				return false;
			}

			FMGCrewMember NewMember;
			NewMember.PlayerID = PendingApplications[i].ApplicantID;
			NewMember.PlayerName = PendingApplications[i].ApplicantName;
			NewMember.Rank = EMGCrewRank::Member;
			NewMember.JoinedDate = FDateTime::UtcNow();
			NewMember.LastActiveDate = FDateTime::UtcNow();
			NewMember.PlayerLevel = PendingApplications[i].ApplicantLevel;

			Members.Add(NewMember);
			CurrentCrew.CurrentMembers++;

			RecordActivity(EMGCrewActivityType::MemberJoined,
				FText::FromString(FString::Printf(TEXT("%s joined the crew"), *NewMember.PlayerName)), 25);

			OnCrewMemberJoined.Broadcast(NewMember);

			PendingApplications.RemoveAt(i);
			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::RejectApplication(FGuid ApplicationID)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	for (int32 i = 0; i < PendingApplications.Num(); ++i)
	{
		if (PendingApplications[i].ApplicationID == ApplicationID)
		{
			PendingApplications.RemoveAt(i);
			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::UpdateCrewInfo(const FString& Description, const FString& Motto)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	CurrentCrew.Description = Description;
	CurrentCrew.Motto = Motto;
	return true;
}

bool UMGCrewSubsystem::SetCrewPrivacy(EMGCrewPrivacy Privacy)
{
	if (!IsCrewLeader())
	{
		return false;
	}

	CurrentCrew.Privacy = Privacy;
	return true;
}

bool UMGCrewSubsystem::SetCrewColors(FLinearColor Primary, FLinearColor Secondary)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	CurrentCrew.PrimaryColor = Primary;
	CurrentCrew.SecondaryColor = Secondary;
	return true;
}

bool UMGCrewSubsystem::SetCrewEmblem(TSoftObjectPtr<UTexture2D> Emblem)
{
	if (!IsCrewOfficer())
	{
		return false;
	}

	CurrentCrew.Emblem = Emblem;
	return true;
}

void UMGCrewSubsystem::AddCrewExperience(int64 Amount, const FString& Source)
{
	if (!IsInCrew())
	{
		return;
	}

	CurrentCrew.ExperiencePoints += Amount;
	CheckLevelUp();
}

float UMGCrewSubsystem::GetCrewLevelProgress() const
{
	if (CurrentCrew.ExperienceToNextLevel <= 0)
	{
		return 0.0f;
	}

	return (static_cast<float>(CurrentCrew.ExperiencePoints) /
		static_cast<float>(CurrentCrew.ExperienceToNextLevel)) * 100.0f;
}

TArray<FMGCrewPerk> UMGCrewSubsystem::GetUnlockedPerks() const
{
	TArray<FMGCrewPerk> Unlocked;
	for (const FMGCrewPerk& Perk : CrewPerks)
	{
		if (Perk.bIsUnlocked)
		{
			Unlocked.Add(Perk);
		}
	}
	return Unlocked;
}

bool UMGCrewSubsystem::HasPerk(FName PerkID) const
{
	for (const FMGCrewPerk& Perk : CrewPerks)
	{
		if (Perk.PerkID == PerkID && Perk.bIsUnlocked)
		{
			return true;
		}
	}
	return false;
}

bool UMGCrewSubsystem::DonateToTreasury(int64 Amount)
{
	if (!IsInCrew() || Amount <= 0)
	{
		return false;
	}

	CurrentCrew.Treasury += Amount;

	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == LocalPlayerID)
		{
			Member.CurrencyDonated += Amount;
			Member.ContributionPoints += Amount / 10;
			break;
		}
	}

	RecordActivity(EMGCrewActivityType::Donation,
		FText::FromString(FString::Printf(TEXT("%s donated %lld to the treasury"),
			*GetLocalMember().PlayerName, Amount)), Amount / 10);

	AddCrewExperience(Amount / 100, TEXT("Donation"));

	return true;
}

bool UMGCrewSubsystem::WithdrawFromTreasury(int64 Amount)
{
	if (!IsCrewLeader())
	{
		return false;
	}

	if (Amount > CurrentCrew.Treasury)
	{
		return false;
	}

	CurrentCrew.Treasury -= Amount;
	return true;
}

void UMGCrewSubsystem::ContributeToChallenge(FName ChallengeID, int64 Amount)
{
	for (FMGCrewChallenge& Challenge : ActiveChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID && !Challenge.bCompleted)
		{
			Challenge.CurrentValue += Amount;

			if (Challenge.CurrentValue >= Challenge.TargetValue)
			{
				Challenge.bCompleted = true;
				AddCrewExperience(Challenge.ExperienceReward, TEXT("Challenge"));
				CurrentCrew.Treasury += Challenge.CurrencyReward;

				OnCrewChallengeCompleted.Broadcast(Challenge);
			}
			break;
		}
	}
}

TArray<FMGCrewActivity> UMGCrewSubsystem::GetActivityFeed(int32 MaxEntries) const
{
	if (MaxEntries >= ActivityFeed.Num())
	{
		return ActivityFeed;
	}

	TArray<FMGCrewActivity> Result;
	int32 StartIndex = ActivityFeed.Num() - MaxEntries;
	for (int32 i = StartIndex; i < ActivityFeed.Num(); ++i)
	{
		Result.Add(ActivityFeed[i]);
	}
	return Result;
}

void UMGCrewSubsystem::RecordActivity(EMGCrewActivityType Type, const FText& Description, int64 Points)
{
	FMGCrewActivity Activity;
	Activity.Type = Type;
	Activity.PlayerID = LocalPlayerID;
	Activity.PlayerName = GetLocalMember().PlayerName;
	Activity.Description = Description;
	Activity.Timestamp = FDateTime::UtcNow();
	Activity.PointsEarned = Points;

	ActivityFeed.Add(Activity);

	if (ActivityFeed.Num() > MaxActivityFeedSize)
	{
		ActivityFeed.RemoveAt(0);
	}

	OnCrewActivityAdded.Broadcast(Activity);
}

TArray<FMGCrewInfo> UMGCrewSubsystem::SearchCrews(const FString& Query, int32 MaxResults)
{
	// Would search through server database
	return TArray<FMGCrewInfo>();
}

TArray<FMGCrewInfo> UMGCrewSubsystem::GetRecommendedCrews() const
{
	// Would get recommendations from server
	return TArray<FMGCrewInfo>();
}

TArray<FMGCrewInfo> UMGCrewSubsystem::GetTopCrews(int32 Count) const
{
	// Would get from server leaderboard
	return TArray<FMGCrewInfo>();
}

void UMGCrewSubsystem::InitializePerks()
{
	auto AddPerk = [this](FName ID, const FString& Name, const FString& Desc, int32 Level, float Bonus)
	{
		FMGCrewPerk Perk;
		Perk.PerkID = ID;
		Perk.PerkName = FText::FromString(Name);
		Perk.Description = FText::FromString(Desc);
		Perk.RequiredLevel = Level;
		Perk.BonusValue = Bonus;
		Perk.bIsUnlocked = false;
		CrewPerks.Add(Perk);
	};

	AddPerk(FName(TEXT("XPBoost1")), TEXT("Crew Spirit I"), TEXT("+5% XP bonus for crew members"), 2, 0.05f);
	AddPerk(FName(TEXT("CashBoost1")), TEXT("Crew Funds I"), TEXT("+5% cash bonus for crew members"), 3, 0.05f);
	AddPerk(FName(TEXT("MemberSlots1")), TEXT("Expanded Roster I"), TEXT("+10 max crew members"), 5, 10.0f);
	AddPerk(FName(TEXT("XPBoost2")), TEXT("Crew Spirit II"), TEXT("+10% XP bonus for crew members"), 8, 0.10f);
	AddPerk(FName(TEXT("CashBoost2")), TEXT("Crew Funds II"), TEXT("+10% cash bonus for crew members"), 10, 0.10f);
	AddPerk(FName(TEXT("MemberSlots2")), TEXT("Expanded Roster II"), TEXT("+25 max crew members"), 15, 25.0f);
	AddPerk(FName(TEXT("NitroBoost")), TEXT("Crew Nitro"), TEXT("+5% nitro capacity for crew members"), 12, 0.05f);
	AddPerk(FName(TEXT("XPBoost3")), TEXT("Crew Spirit III"), TEXT("+15% XP bonus for crew members"), 20, 0.15f);
	AddPerk(FName(TEXT("Garage")), TEXT("Crew Garage"), TEXT("Unlock crew garage with shared vehicles"), 25, 1.0f);
	AddPerk(FName(TEXT("MemberSlots3")), TEXT("Expanded Roster III"), TEXT("+50 max crew members"), 30, 50.0f);
}

void UMGCrewSubsystem::CheckLevelUp()
{
	while (CurrentCrew.ExperiencePoints >= CurrentCrew.ExperienceToNextLevel)
	{
		CurrentCrew.ExperiencePoints -= CurrentCrew.ExperienceToNextLevel;
		CurrentCrew.Level++;
		CurrentCrew.ExperienceToNextLevel = CalculateExperienceForLevel(CurrentCrew.Level + 1);

		// Unlock perks
		TArray<FMGCrewPerk> NewlyUnlocked;
		for (FMGCrewPerk& Perk : CrewPerks)
		{
			if (!Perk.bIsUnlocked && Perk.RequiredLevel <= CurrentCrew.Level)
			{
				Perk.bIsUnlocked = true;
				NewlyUnlocked.Add(Perk);

				// Apply perk bonuses
				if (Perk.PerkID.ToString().Contains(TEXT("MemberSlots")))
				{
					CurrentCrew.MaxMembers += static_cast<int32>(Perk.BonusValue);
				}
			}
		}

		RecordActivity(EMGCrewActivityType::LevelUp,
			FText::FromString(FString::Printf(TEXT("Crew reached level %d!"), CurrentCrew.Level)), 100);

		OnCrewLevelUp.Broadcast(CurrentCrew.Level, NewlyUnlocked);
	}
}

void UMGCrewSubsystem::CheckChallenges()
{
	for (FMGCrewChallenge& Challenge : ActiveChallenges)
	{
		if (!Challenge.bCompleted && Challenge.CurrentValue >= Challenge.TargetValue)
		{
			Challenge.bCompleted = true;
			OnCrewChallengeCompleted.Broadcast(Challenge);
		}
	}
}

int64 UMGCrewSubsystem::CalculateExperienceForLevel(int32 Level) const
{
	return static_cast<int64>(1000 * FMath::Pow(1.5f, Level - 1));
}

void UMGCrewSubsystem::SaveCrewData()
{
	// Save to player save game
}

void UMGCrewSubsystem::LoadCrewData()
{
	// Load from player save game
}

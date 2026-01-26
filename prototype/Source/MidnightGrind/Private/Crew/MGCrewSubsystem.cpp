// Copyright Midnight Grind. All Rights Reserved.

#include "Crew/MGCrewSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================================
// USubsystem Interface
// ============================================================================

void UMGCrewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MaxActivityFeedSize = 200;
	MaxChatHistorySize = 500;
	BaseGarageCapacity = 5;

	InitializePerks();
	InitializeTerritories();
	LoadCrewData();

	// Start periodic tick for maintenance
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CrewTickHandle,
			this,
			&UMGCrewSubsystem::OnCrewTick,
			30.0f,  // Every 30 seconds
			true
		);
	}
}

void UMGCrewSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CrewTickHandle);
	}

	SaveCrewData();
	Super::Deinitialize();
}

bool UMGCrewSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Crew Management
// ============================================================================

bool UMGCrewSubsystem::CreateCrew(const FString& Name, const FString& Tag, const FString& Description)
{
	// Cannot create if already in a crew
	if (IsInCrew())
	{
		return false;
	}

	// Validate name length (3-32 characters)
	if (Name.Len() < 3 || Name.Len() > 32)
	{
		return false;
	}

	// Validate tag length (2-5 characters)
	if (Tag.Len() < 2 || Tag.Len() > 5)
	{
		return false;
	}

	// Initialize new crew
	CurrentCrew.CrewID = FGuid::NewGuid();
	CurrentCrew.CrewName = Name;
	CurrentCrew.CrewTag = Tag.ToUpper();
	CurrentCrew.Description = Description;
	CurrentCrew.CreatedDate = FDateTime::UtcNow();
	CurrentCrew.Level = 1;
	CurrentCrew.ExperiencePoints = 0;
	CurrentCrew.ExperienceToNextLevel = CalculateExperienceForLevel(2);
	CurrentCrew.MaxMembers = 50;
	CurrentCrew.CurrentMembers = 1;
	CurrentCrew.Privacy = EMGCrewPrivacy::ApprovalRequired;
	CurrentCrew.BattleRating = 1500;

	// Add creator as leader
	FMGCrewMember LeaderMember;
	LeaderMember.PlayerID = LocalPlayerID;
	LeaderMember.PlayerName = LocalPlayerName;
	LeaderMember.Rank = EMGCrewRank::Leader;
	LeaderMember.JoinedDate = FDateTime::UtcNow();
	LeaderMember.LastActiveDate = FDateTime::UtcNow();
	LeaderMember.bIsOnline = true;
	Members.Add(LeaderMember);

	// Record activity and add welcome message
	RecordActivity(EMGCrewActivityType::MemberJoined,
		FText::FromString(FString::Printf(TEXT("%s created the crew!"), *LocalPlayerName)), 100);

	AddSystemChatMessage(FString::Printf(TEXT("Welcome to %s! Your crew has been created."), *Name));

	OnCrewJoined.Broadcast(CurrentCrew);
	SaveCrewData();

	return true;
}

bool UMGCrewSubsystem::DisbandCrew()
{
	if (!IsInCrew() || !HasPermission(EMGCrewPermission::DisbandCrew))
	{
		return false;
	}

	// Clear all crew data
	CurrentCrew = FMGCrewInfo();
	Members.Empty();
	PendingApplications.Empty();
	ActivityFeed.Empty();
	ActiveChallenges.Empty();
	ChatHistory.Empty();
	GarageVehicles.Empty();

	OnCrewLeft.Broadcast();
	SaveCrewData();

	return true;
}

bool UMGCrewSubsystem::LeaveCrew()
{
	if (!IsInCrew())
	{
		return false;
	}

	// Leaders must transfer leadership first if there are other members
	if (IsCrewLeader() && Members.Num() > 1)
	{
		return false;
	}

	// Record departure
	RecordActivity(EMGCrewActivityType::MemberLeft,
		FText::FromString(FString::Printf(TEXT("%s left the crew"), *GetLocalMember().PlayerName)), 0);

	OnCrewMemberLeft.Broadcast(LocalPlayerID);

	// If last member, disband the crew
	if (Members.Num() <= 1)
	{
		return DisbandCrew();
	}

	// Clear local crew data
	CurrentCrew = FMGCrewInfo();
	Members.Empty();
	ChatHistory.Empty();

	OnCrewLeft.Broadcast();
	SaveCrewData();

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

EMGCrewRank UMGCrewSubsystem::GetLocalPlayerRank() const
{
	return GetLocalMember().Rank;
}

// ============================================================================
// Permissions
// ============================================================================

bool UMGCrewSubsystem::HasPermission(EMGCrewPermission Permission) const
{
	if (!IsInCrew())
	{
		return false;
	}

	return RankHasPermission(GetLocalPlayerRank(), Permission);
}

bool UMGCrewSubsystem::RankHasPermission(EMGCrewRank Rank, EMGCrewPermission Permission)
{
	// Define permission matrix based on rank
	switch (Permission)
	{
		case EMGCrewPermission::ViewCrew:
		case EMGCrewPermission::ParticipateEvents:
			// All ranks can view and participate
			return true;

		case EMGCrewPermission::UseChat:
			// Prospect and above can chat
			return Rank >= EMGCrewRank::Prospect;

		case EMGCrewPermission::AccessGarage:
			// Members and above can access garage
			return Rank >= EMGCrewRank::Member;

		case EMGCrewPermission::BorrowVehicles:
			// Members and above can borrow
			return Rank >= EMGCrewRank::Member;

		case EMGCrewPermission::InviteMembers:
			// Officers and above can invite
			return Rank >= EMGCrewRank::Officer;

		case EMGCrewPermission::AcceptApplications:
			// Officers and above can accept applications
			return Rank >= EMGCrewRank::Officer;

		case EMGCrewPermission::KickMembers:
			// Lieutenants and above can kick
			return Rank >= EMGCrewRank::Lieutenant;

		case EMGCrewPermission::PromoteMembers:
			// Lieutenants and above can promote (up to one below their rank)
			return Rank >= EMGCrewRank::Lieutenant;

		case EMGCrewPermission::DemoteMembers:
			// Lieutenants and above can demote
			return Rank >= EMGCrewRank::Lieutenant;

		case EMGCrewPermission::ManageGarage:
			// Officers and above can manage garage
			return Rank >= EMGCrewRank::Officer;

		case EMGCrewPermission::EditCrewInfo:
			// Captains and above can edit crew info
			return Rank >= EMGCrewRank::Captain;

		case EMGCrewPermission::StartCrewBattles:
			// Officers and above can start battles
			return Rank >= EMGCrewRank::Officer;

		case EMGCrewPermission::ManageChallenges:
			// Captains and above can manage challenges
			return Rank >= EMGCrewRank::Captain;

		case EMGCrewPermission::ManageTreasury:
			// Captains and above can manage treasury
			return Rank >= EMGCrewRank::Captain;

		case EMGCrewPermission::ClaimTerritory:
			// Officers and above can claim territory
			return Rank >= EMGCrewRank::Officer;

		case EMGCrewPermission::ChangeSettings:
			// Leaders only can change settings
			return Rank >= EMGCrewRank::Leader;

		case EMGCrewPermission::TransferLeadership:
			// Leaders only
			return Rank >= EMGCrewRank::Leader;

		case EMGCrewPermission::DisbandCrew:
			// Leaders only
			return Rank >= EMGCrewRank::Leader;

		default:
			return false;
	}
}

TArray<EMGCrewPermission> UMGCrewSubsystem::GetPermissionsForRank(EMGCrewRank Rank)
{
	TArray<EMGCrewPermission> Permissions;

	// Check each permission
	for (int32 i = 0; i < static_cast<int32>(EMGCrewPermission::DisbandCrew) + 1; ++i)
	{
		EMGCrewPermission Permission = static_cast<EMGCrewPermission>(i);
		if (RankHasPermission(Rank, Permission))
		{
			Permissions.Add(Permission);
		}
	}

	return Permissions;
}

FText UMGCrewSubsystem::GetRankDisplayName(EMGCrewRank Rank)
{
	switch (Rank)
	{
		case EMGCrewRank::Prospect:
			return NSLOCTEXT("Crew", "RankProspect", "Prospect");
		case EMGCrewRank::Member:
			return NSLOCTEXT("Crew", "RankMember", "Member");
		case EMGCrewRank::Veteran:
			return NSLOCTEXT("Crew", "RankVeteran", "Veteran");
		case EMGCrewRank::Officer:
			return NSLOCTEXT("Crew", "RankOfficer", "Officer");
		case EMGCrewRank::Lieutenant:
			return NSLOCTEXT("Crew", "RankLieutenant", "Lieutenant");
		case EMGCrewRank::Captain:
			return NSLOCTEXT("Crew", "RankCaptain", "Captain");
		case EMGCrewRank::Leader:
			return NSLOCTEXT("Crew", "RankLeader", "Leader");
		default:
			return FText::GetEmpty();
	}
}

// ============================================================================
// Member Management
// ============================================================================

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

TArray<FMGCrewMember> UMGCrewSubsystem::GetMembersByRank(EMGCrewRank Rank) const
{
	TArray<FMGCrewMember> FilteredMembers;
	for (const FMGCrewMember& Member : Members)
	{
		if (Member.Rank == Rank)
		{
			FilteredMembers.Add(Member);
		}
	}
	return FilteredMembers;
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

bool UMGCrewSubsystem::KickMember(FName PlayerID, const FString& Reason)
{
	if (!HasPermission(EMGCrewPermission::KickMembers))
	{
		return false;
	}

	// Cannot kick self
	if (PlayerID == LocalPlayerID)
	{
		return false;
	}

	for (int32 i = 0; i < Members.Num(); ++i)
	{
		if (Members[i].PlayerID == PlayerID)
		{
			FMGCrewMember LocalMember = GetLocalMember();

			// Cannot kick someone of equal or higher rank
			if (Members[i].Rank >= LocalMember.Rank)
			{
				return false;
			}

			FString KickedName = Members[i].PlayerName;
			Members.RemoveAt(i);
			CurrentCrew.CurrentMembers--;

			FString Description = FString::Printf(TEXT("%s was removed from the crew"), *KickedName);
			if (!Reason.IsEmpty())
			{
				Description += FString::Printf(TEXT(" (%s)"), *Reason);
			}

			RecordActivity(EMGCrewActivityType::MemberKicked,
				FText::FromString(Description), 0);

			AddSystemChatMessage(Description);

			OnCrewMemberLeft.Broadcast(PlayerID);
			SaveCrewData();
			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::PromoteMember(FName PlayerID, EMGCrewRank NewRank)
{
	if (!HasPermission(EMGCrewPermission::PromoteMembers))
	{
		return false;
	}

	FMGCrewMember LocalMember = GetLocalMember();

	// Cannot promote to or above own rank
	if (NewRank >= LocalMember.Rank)
	{
		return false;
	}

	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			// Must be a promotion (higher rank)
			if (NewRank <= Member.Rank)
			{
				return false;
			}

			EMGCrewRank OldRank = Member.Rank;
			Member.Rank = NewRank;

			RecordActivity(EMGCrewActivityType::Promotion,
				FText::FromString(FString::Printf(TEXT("%s was promoted to %s"),
					*Member.PlayerName, *GetRankDisplayName(NewRank).ToString())), 50);

			AddSystemChatMessage(FString::Printf(TEXT("%s has been promoted to %s!"),
				*Member.PlayerName, *GetRankDisplayName(NewRank).ToString()));

			OnCrewMemberRankChanged.Broadcast(PlayerID, NewRank);
			SaveCrewData();
			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::DemoteMember(FName PlayerID, EMGCrewRank NewRank)
{
	if (!HasPermission(EMGCrewPermission::DemoteMembers))
	{
		return false;
	}

	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			FMGCrewMember LocalMember = GetLocalMember();

			// Cannot demote someone of equal or higher rank
			if (Member.Rank >= LocalMember.Rank)
			{
				return false;
			}

			// Must be a demotion (lower rank)
			if (NewRank >= Member.Rank)
			{
				return false;
			}

			Member.Rank = NewRank;

			RecordActivity(EMGCrewActivityType::Demotion,
				FText::FromString(FString::Printf(TEXT("%s was demoted to %s"),
					*Member.PlayerName, *GetRankDisplayName(NewRank).ToString())), 0);

			OnCrewMemberRankChanged.Broadcast(PlayerID, NewRank);
			SaveCrewData();
			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::TransferLeadership(FName NewLeaderID)
{
	if (!HasPermission(EMGCrewPermission::TransferLeadership))
	{
		return false;
	}

	FMGCrewMember* NewLeader = nullptr;
	FMGCrewMember* CurrentLeader = nullptr;

	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == NewLeaderID)
		{
			NewLeader = &Member;
		}
		else if (Member.PlayerID == LocalPlayerID)
		{
			CurrentLeader = &Member;
		}
	}

	if (!NewLeader || !CurrentLeader)
	{
		return false;
	}

	// Transfer leadership
	NewLeader->Rank = EMGCrewRank::Leader;
	CurrentLeader->Rank = EMGCrewRank::Captain;

	RecordActivity(EMGCrewActivityType::Promotion,
		FText::FromString(FString::Printf(TEXT("%s is now the crew leader!"), *NewLeader->PlayerName)), 100);

	AddSystemChatMessage(FString::Printf(TEXT("%s has transferred leadership to %s."),
		*CurrentLeader->PlayerName, *NewLeader->PlayerName));

	OnCrewMemberRankChanged.Broadcast(NewLeaderID, EMGCrewRank::Leader);
	OnCrewMemberRankChanged.Broadcast(LocalPlayerID, EMGCrewRank::Captain);
	SaveCrewData();

	return true;
}

void UMGCrewSubsystem::SetMemberNote(FName PlayerID, const FString& Note)
{
	if (!HasPermission(EMGCrewPermission::KickMembers))
	{
		return;
	}

	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == PlayerID)
		{
			Member.OfficerNote = Note;
			SaveCrewData();
			return;
		}
	}
}

// ============================================================================
// Invites & Applications
// ============================================================================

bool UMGCrewSubsystem::SendInvite(FName PlayerID, const FString& Message)
{
	if (!HasPermission(EMGCrewPermission::InviteMembers))
	{
		return false;
	}

	if (CurrentCrew.CurrentMembers >= CurrentCrew.MaxMembers)
	{
		return false;
	}

	// Create and send invite (would be sent through network in production)
	FMGCrewInvite Invite;
	Invite.InviteID = FGuid::NewGuid();
	Invite.CrewID = CurrentCrew.CrewID;
	Invite.CrewName = CurrentCrew.CrewName;
	Invite.InviterID = LocalPlayerID;
	Invite.InviterName = GetLocalMember().PlayerName;
	Invite.SentDate = FDateTime::UtcNow();
	Invite.ExpiresDate = FDateTime::UtcNow() + FTimespan::FromDays(7);
	Invite.Message = Message;

	// In production, this would send through online services
	return true;
}

bool UMGCrewSubsystem::AcceptInvite(FGuid InviteID)
{
	if (IsInCrew())
	{
		return false;
	}

	for (int32 i = 0; i < PendingInvites.Num(); ++i)
	{
		if (PendingInvites[i].InviteID == InviteID)
		{
			// In production, this would communicate with server to join crew
			PendingInvites.RemoveAt(i);
			SaveCrewData();
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
			SaveCrewData();
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

	// In production, this would send application to server
	return true;
}

bool UMGCrewSubsystem::AcceptApplication(FGuid ApplicationID)
{
	if (!HasPermission(EMGCrewPermission::AcceptApplications))
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

			// Add new member
			FMGCrewMember NewMember;
			NewMember.PlayerID = PendingApplications[i].ApplicantID;
			NewMember.PlayerName = PendingApplications[i].ApplicantName;
			NewMember.Rank = EMGCrewRank::Prospect;  // New members start as prospects
			NewMember.JoinedDate = FDateTime::UtcNow();
			NewMember.LastActiveDate = FDateTime::UtcNow();
			NewMember.PlayerLevel = PendingApplications[i].ApplicantLevel;

			Members.Add(NewMember);
			CurrentCrew.CurrentMembers++;

			RecordActivity(EMGCrewActivityType::MemberJoined,
				FText::FromString(FString::Printf(TEXT("%s joined the crew!"), *NewMember.PlayerName)), 25);

			AddSystemChatMessage(FString::Printf(TEXT("Welcome %s to the crew!"), *NewMember.PlayerName));

			OnCrewMemberJoined.Broadcast(NewMember);

			PendingApplications.RemoveAt(i);
			SaveCrewData();
			return true;
		}
	}

	return false;
}

bool UMGCrewSubsystem::RejectApplication(FGuid ApplicationID)
{
	if (!HasPermission(EMGCrewPermission::AcceptApplications))
	{
		return false;
	}

	for (int32 i = 0; i < PendingApplications.Num(); ++i)
	{
		if (PendingApplications[i].ApplicationID == ApplicationID)
		{
			PendingApplications.RemoveAt(i);
			SaveCrewData();
			return true;
		}
	}

	return false;
}

// ============================================================================
// Crew Settings
// ============================================================================

bool UMGCrewSubsystem::UpdateCrewInfo(const FString& Description, const FString& Motto)
{
	if (!HasPermission(EMGCrewPermission::EditCrewInfo))
	{
		return false;
	}

	CurrentCrew.Description = Description;
	CurrentCrew.Motto = Motto;

	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
	return true;
}

bool UMGCrewSubsystem::SetCrewPrivacy(EMGCrewPrivacy Privacy)
{
	if (!HasPermission(EMGCrewPermission::ChangeSettings))
	{
		return false;
	}

	CurrentCrew.Privacy = Privacy;
	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
	return true;
}

bool UMGCrewSubsystem::SetMinimumJoinLevel(int32 MinLevel)
{
	if (!HasPermission(EMGCrewPermission::ChangeSettings))
	{
		return false;
	}

	CurrentCrew.MinimumJoinLevel = FMath::Max(1, MinLevel);
	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
	return true;
}

bool UMGCrewSubsystem::SetCrewColors(FLinearColor Primary, FLinearColor Secondary)
{
	if (!HasPermission(EMGCrewPermission::EditCrewInfo))
	{
		return false;
	}

	CurrentCrew.PrimaryColor = Primary;
	CurrentCrew.SecondaryColor = Secondary;
	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
	return true;
}

bool UMGCrewSubsystem::SetCrewEmblem(TSoftObjectPtr<UTexture2D> Emblem)
{
	if (!HasPermission(EMGCrewPermission::EditCrewInfo))
	{
		return false;
	}

	CurrentCrew.Emblem = Emblem;
	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
	return true;
}

// ============================================================================
// Progression
// ============================================================================

void UMGCrewSubsystem::AddCrewExperience(int64 Amount, const FString& Source)
{
	if (!IsInCrew())
	{
		return;
	}

	CurrentCrew.ExperiencePoints += Amount;

	// Update local member's contribution
	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == LocalPlayerID)
		{
			Member.ContributionPoints += Amount;
			Member.WeeklyContribution += Amount;
			break;
		}
	}

	CheckLevelUp();
	OnCrewUpdated.Broadcast(CurrentCrew);
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

float UMGCrewSubsystem::GetPerkValue(FName PerkID) const
{
	for (const FMGCrewPerk& Perk : CrewPerks)
	{
		if (Perk.PerkID == PerkID && Perk.bIsUnlocked)
		{
			return Perk.BonusValue;
		}
	}
	return 0.0f;
}

// ============================================================================
// Treasury
// ============================================================================

bool UMGCrewSubsystem::DonateToTreasury(int64 Amount)
{
	if (!IsInCrew() || Amount <= 0)
	{
		return false;
	}

	CurrentCrew.Treasury += Amount;

	// Update member donation stats
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
		FText::FromString(FString::Printf(TEXT("%s donated $%lld to the treasury"),
			*GetLocalMember().PlayerName, Amount)), Amount / 10);

	// Add XP for donations
	AddCrewExperience(Amount / 100, TEXT("Donation"));

	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
	return true;
}

bool UMGCrewSubsystem::WithdrawFromTreasury(int64 Amount, const FString& Purpose)
{
	if (!HasPermission(EMGCrewPermission::ManageTreasury))
	{
		return false;
	}

	if (Amount > CurrentCrew.Treasury)
	{
		return false;
	}

	CurrentCrew.Treasury -= Amount;

	FString Message = FString::Printf(TEXT("%s withdrew $%lld from treasury"),
		*GetLocalMember().PlayerName, Amount);
	if (!Purpose.IsEmpty())
	{
		Message += FString::Printf(TEXT(" for: %s"), *Purpose);
	}

	AddSystemChatMessage(Message);
	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
	return true;
}

// ============================================================================
// Challenges
// ============================================================================

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

				RecordActivity(EMGCrewActivityType::CrewChallenge,
					FText::FromString(FString::Printf(TEXT("Crew completed: %s"), *Challenge.Title.ToString())),
					Challenge.ExperienceReward);

				AddSystemChatMessage(FString::Printf(TEXT("Challenge Complete: %s! Rewards available."),
					*Challenge.Title.ToString()));

				OnCrewChallengeCompleted.Broadcast(Challenge);
			}
			break;
		}
	}
}

bool UMGCrewSubsystem::ClaimChallengeRewards(FName ChallengeID)
{
	for (FMGCrewChallenge& Challenge : ActiveChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID && Challenge.bCompleted && !Challenge.bRewardsClaimed)
		{
			Challenge.bRewardsClaimed = true;
			AddCrewExperience(Challenge.ExperienceReward, TEXT("Challenge Reward"));
			CurrentCrew.Treasury += Challenge.CurrencyReward;

			OnCrewUpdated.Broadcast(CurrentCrew);
			SaveCrewData();
			return true;
		}
	}
	return false;
}

// ============================================================================
// Activity Feed
// ============================================================================

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

	// Trim to max size
	while (ActivityFeed.Num() > MaxActivityFeedSize)
	{
		ActivityFeed.RemoveAt(0);
	}

	OnCrewActivityAdded.Broadcast(Activity);
}

// ============================================================================
// Chat System
// ============================================================================

bool UMGCrewSubsystem::SendChatMessage(const FString& Content)
{
	if (!IsInCrew() || !HasPermission(EMGCrewPermission::UseChat))
	{
		return false;
	}

	if (Content.IsEmpty() || Content.Len() > 500)
	{
		return false;
	}

	FMGCrewChatMessage Message;
	Message.MessageID = FGuid::NewGuid();
	Message.SenderID = LocalPlayerID;
	Message.SenderName = GetLocalMember().PlayerName;
	Message.SenderRank = GetLocalMember().Rank;
	Message.MessageType = EMGCrewChatType::Text;
	Message.Content = Content;
	Message.Timestamp = FDateTime::UtcNow();
	Message.bIsRead = true;  // Own messages are read

	ChatHistory.Add(Message);

	// Trim to max size
	while (ChatHistory.Num() > MaxChatHistorySize)
	{
		ChatHistory.RemoveAt(0);
	}

	OnCrewChatMessageReceived.Broadcast(Message);

	// In production, would send through network
	return true;
}

bool UMGCrewSubsystem::SendBroadcast(const FString& Content)
{
	if (!IsInCrew() || !HasPermission(EMGCrewPermission::EditCrewInfo))
	{
		return false;
	}

	FMGCrewChatMessage Message;
	Message.MessageID = FGuid::NewGuid();
	Message.SenderID = LocalPlayerID;
	Message.SenderName = GetLocalMember().PlayerName;
	Message.SenderRank = GetLocalMember().Rank;
	Message.MessageType = EMGCrewChatType::Broadcast;
	Message.Content = Content;
	Message.Timestamp = FDateTime::UtcNow();
	Message.bIsRead = true;

	ChatHistory.Add(Message);

	while (ChatHistory.Num() > MaxChatHistorySize)
	{
		ChatHistory.RemoveAt(0);
	}

	OnCrewChatMessageReceived.Broadcast(Message);
	return true;
}

TArray<FMGCrewChatMessage> UMGCrewSubsystem::GetChatHistory(int32 MaxMessages) const
{
	if (MaxMessages >= ChatHistory.Num())
	{
		return ChatHistory;
	}

	TArray<FMGCrewChatMessage> Result;
	int32 StartIndex = ChatHistory.Num() - MaxMessages;
	for (int32 i = StartIndex; i < ChatHistory.Num(); ++i)
	{
		Result.Add(ChatHistory[i]);
	}
	return Result;
}

int32 UMGCrewSubsystem::GetUnreadMessageCount() const
{
	int32 Count = 0;
	for (const FMGCrewChatMessage& Message : ChatHistory)
	{
		if (!Message.bIsRead)
		{
			Count++;
		}
	}
	return Count;
}

void UMGCrewSubsystem::MarkAllMessagesRead()
{
	for (FMGCrewChatMessage& Message : ChatHistory)
	{
		Message.bIsRead = true;
	}
}

void UMGCrewSubsystem::ReceiveChatMessage(const FMGCrewChatMessage& Message)
{
	ChatHistory.Add(Message);

	while (ChatHistory.Num() > MaxChatHistorySize)
	{
		ChatHistory.RemoveAt(0);
	}

	OnCrewChatMessageReceived.Broadcast(Message);
}

// ============================================================================
// Shared Garage
// ============================================================================

TArray<FMGCrewGarageVehicle> UMGCrewSubsystem::GetAvailableVehicles() const
{
	TArray<FMGCrewGarageVehicle> Available;
	EMGCrewRank LocalRank = GetLocalPlayerRank();

	for (const FMGCrewGarageVehicle& Vehicle : GarageVehicles)
	{
		if (!Vehicle.bIsBorrowed && LocalRank >= Vehicle.MinimumBorrowRank)
		{
			Available.Add(Vehicle);
		}
	}
	return Available;
}

bool UMGCrewSubsystem::DonateVehicleToGarage(FName VehicleID, const FString& VehicleName,
	int32 PerformanceIndex, const FString& VehicleClass, int64 EstimatedValue)
{
	if (!IsInCrew() || !HasPermission(EMGCrewPermission::AccessGarage))
	{
		return false;
	}

	if (GarageVehicles.Num() >= GetGarageCapacity())
	{
		return false;
	}

	FMGCrewGarageVehicle NewVehicle;
	NewVehicle.VehicleInstanceID = FGuid::NewGuid();
	NewVehicle.VehicleID = VehicleID;
	NewVehicle.VehicleName = VehicleName;
	NewVehicle.DonorPlayerID = LocalPlayerID;
	NewVehicle.DonorName = GetLocalMember().PlayerName;
	NewVehicle.AddedDate = FDateTime::UtcNow();
	NewVehicle.PerformanceIndex = PerformanceIndex;
	NewVehicle.VehicleClass = VehicleClass;
	NewVehicle.EstimatedValue = EstimatedValue;
	NewVehicle.bIsBorrowed = false;
	NewVehicle.MinimumBorrowRank = EMGCrewRank::Member;

	GarageVehicles.Add(NewVehicle);

	RecordActivity(EMGCrewActivityType::GarageVehicleAdded,
		FText::FromString(FString::Printf(TEXT("%s donated a %s to the crew garage"),
			*GetLocalMember().PlayerName, *VehicleName)), 50);

	AddSystemChatMessage(FString::Printf(TEXT("%s donated their %s to the crew garage!"),
		*GetLocalMember().PlayerName, *VehicleName));

	// Give contribution points based on value
	int64 ContributionPoints = EstimatedValue / 100;
	AddCrewExperience(ContributionPoints, TEXT("Vehicle Donation"));

	OnCrewGarageVehicleAdded.Broadcast(NewVehicle);
	SaveCrewData();
	return true;
}

bool UMGCrewSubsystem::BorrowVehicle(FGuid VehicleInstanceID)
{
	if (!HasPermission(EMGCrewPermission::BorrowVehicles))
	{
		return false;
	}

	for (FMGCrewGarageVehicle& Vehicle : GarageVehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			if (Vehicle.bIsBorrowed)
			{
				return false;
			}

			if (GetLocalPlayerRank() < Vehicle.MinimumBorrowRank)
			{
				return false;
			}

			Vehicle.bIsBorrowed = true;
			Vehicle.BorrowerID = LocalPlayerID;
			Vehicle.BorrowerName = GetLocalMember().PlayerName;

			OnCrewGarageVehicleBorrowed.Broadcast(Vehicle, LocalPlayerID);
			SaveCrewData();
			return true;
		}
	}
	return false;
}

bool UMGCrewSubsystem::ReturnVehicle(FGuid VehicleInstanceID)
{
	for (FMGCrewGarageVehicle& Vehicle : GarageVehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			if (Vehicle.BorrowerID != LocalPlayerID)
			{
				return false;
			}

			Vehicle.bIsBorrowed = false;
			Vehicle.BorrowerID = NAME_None;
			Vehicle.BorrowerName.Empty();

			SaveCrewData();
			return true;
		}
	}
	return false;
}

bool UMGCrewSubsystem::RemoveVehicleFromGarage(FGuid VehicleInstanceID)
{
	if (!HasPermission(EMGCrewPermission::ManageGarage))
	{
		return false;
	}

	for (int32 i = 0; i < GarageVehicles.Num(); ++i)
	{
		if (GarageVehicles[i].VehicleInstanceID == VehicleInstanceID)
		{
			GarageVehicles.RemoveAt(i);
			SaveCrewData();
			return true;
		}
	}
	return false;
}

bool UMGCrewSubsystem::SetVehicleBorrowRank(FGuid VehicleInstanceID, EMGCrewRank MinRank)
{
	if (!HasPermission(EMGCrewPermission::ManageGarage))
	{
		return false;
	}

	for (FMGCrewGarageVehicle& Vehicle : GarageVehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			Vehicle.MinimumBorrowRank = MinRank;
			SaveCrewData();
			return true;
		}
	}
	return false;
}

int32 UMGCrewSubsystem::GetGarageCapacity() const
{
	int32 Capacity = BaseGarageCapacity;

	// Add bonus from Garage perk
	if (HasPerk(FName(TEXT("Garage"))))
	{
		Capacity += 5;
	}

	// Add bonus from level (1 slot per 5 levels)
	Capacity += CurrentCrew.Level / 5;

	return Capacity;
}

// ============================================================================
// Territory System
// ============================================================================

TArray<FMGCrewTerritory> UMGCrewSubsystem::GetOwnedTerritories() const
{
	TArray<FMGCrewTerritory> Owned;

	for (const auto& Pair : Territories)
	{
		if (Pair.Value.OwningCrewID == CurrentCrew.CrewID)
		{
			Owned.Add(Pair.Value);
		}
	}

	return Owned;
}

FMGCrewTerritory UMGCrewSubsystem::GetTerritory(FName TerritoryID) const
{
	if (const FMGCrewTerritory* Territory = Territories.Find(TerritoryID))
	{
		return *Territory;
	}
	return FMGCrewTerritory();
}

bool UMGCrewSubsystem::ClaimTerritory(FName TerritoryID)
{
	if (!HasPermission(EMGCrewPermission::ClaimTerritory))
	{
		return false;
	}

	FMGCrewTerritory* Territory = Territories.Find(TerritoryID);
	if (!Territory)
	{
		return false;
	}

	// Can only claim unclaimed territories directly
	if (Territory->Status != EMGTerritoryStatus::Unclaimed)
	{
		return false;
	}

	Territory->OwningCrewID = CurrentCrew.CrewID;
	Territory->OwningCrewName = CurrentCrew.CrewName;
	Territory->Status = EMGTerritoryStatus::Owned;
	Territory->CapturedDate = FDateTime::UtcNow();

	CurrentCrew.TerritoriesOwned++;

	RecordActivity(EMGCrewActivityType::TerritoryWon,
		FText::FromString(FString::Printf(TEXT("Crew claimed %s!"),
			*Territory->TerritoryName.ToString())), 200);

	AddSystemChatMessage(FString::Printf(TEXT("We have claimed %s! +%.0f%% %s bonus."),
		*Territory->TerritoryName.ToString(), Territory->BonusValue * 100.0f,
		*Territory->BonusType.ToString()));

	OnCrewTerritoryChanged.Broadcast(*Territory);
	SaveCrewData();
	return true;
}

bool UMGCrewSubsystem::ChallengeForTerritory(FName TerritoryID)
{
	if (!HasPermission(EMGCrewPermission::ClaimTerritory))
	{
		return false;
	}

	FMGCrewTerritory* Territory = Territories.Find(TerritoryID);
	if (!Territory)
	{
		return false;
	}

	// Can only challenge rival-owned territories
	if (Territory->Status != EMGTerritoryStatus::Rival)
	{
		return false;
	}

	Territory->bIsContested = true;
	Territory->ChallengingCrewID = CurrentCrew.CrewID;
	Territory->Status = EMGTerritoryStatus::Contested;

	AddSystemChatMessage(FString::Printf(TEXT("We are challenging %s for %s!"),
		*Territory->OwningCrewName, *Territory->TerritoryName.ToString()));

	OnCrewTerritoryChanged.Broadcast(*Territory);
	SaveCrewData();
	return true;
}

float UMGCrewSubsystem::GetTerritoryBonus(FName BonusType) const
{
	float TotalBonus = 0.0f;

	for (const auto& Pair : Territories)
	{
		if (Pair.Value.OwningCrewID == CurrentCrew.CrewID && Pair.Value.BonusType == BonusType)
		{
			TotalBonus += Pair.Value.BonusValue;
		}
	}

	return TotalBonus;
}

// ============================================================================
// Search & Discovery
// ============================================================================

TArray<FMGCrewInfo> UMGCrewSubsystem::SearchCrews(const FString& Query, int32 MaxResults)
{
	// In production, this would query the server
	return TArray<FMGCrewInfo>();
}

TArray<FMGCrewInfo> UMGCrewSubsystem::GetRecommendedCrews() const
{
	// In production, this would get recommendations from server
	return TArray<FMGCrewInfo>();
}

TArray<FMGCrewInfo> UMGCrewSubsystem::GetTopCrews(int32 Count) const
{
	// In production, this would query server leaderboard
	return TArray<FMGCrewInfo>();
}

// ============================================================================
// Protected Implementation
// ============================================================================

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

	// XP and Cash perks
	AddPerk(FName(TEXT("XPBoost1")), TEXT("Crew Spirit I"), TEXT("+5% XP bonus for crew members"), 2, 0.05f);
	AddPerk(FName(TEXT("CashBoost1")), TEXT("Crew Funds I"), TEXT("+5% cash bonus for crew members"), 3, 0.05f);
	AddPerk(FName(TEXT("RepBoost1")), TEXT("Reputation I"), TEXT("+5% reputation bonus for crew members"), 4, 0.05f);

	// Capacity perks
	AddPerk(FName(TEXT("MemberSlots1")), TEXT("Expanded Roster I"), TEXT("+10 max crew members"), 5, 10.0f);
	AddPerk(FName(TEXT("MemberSlots2")), TEXT("Expanded Roster II"), TEXT("+25 max crew members"), 15, 25.0f);
	AddPerk(FName(TEXT("MemberSlots3")), TEXT("Expanded Roster III"), TEXT("+50 max crew members"), 30, 50.0f);

	// Tier 2 perks
	AddPerk(FName(TEXT("XPBoost2")), TEXT("Crew Spirit II"), TEXT("+10% XP bonus for crew members"), 8, 0.10f);
	AddPerk(FName(TEXT("CashBoost2")), TEXT("Crew Funds II"), TEXT("+10% cash bonus for crew members"), 10, 0.10f);
	AddPerk(FName(TEXT("NitroBoost")), TEXT("Crew Nitro"), TEXT("+5% nitro capacity for crew members"), 12, 0.05f);

	// Special perks
	AddPerk(FName(TEXT("Garage")), TEXT("Crew Garage"), TEXT("Unlock crew garage with shared vehicles"), 20, 1.0f);
	AddPerk(FName(TEXT("Territory")), TEXT("Territory Control"), TEXT("Unlock ability to claim territories"), 25, 1.0f);
	AddPerk(FName(TEXT("XPBoost3")), TEXT("Crew Spirit III"), TEXT("+15% XP bonus for crew members"), 35, 0.15f);

	// High level perks
	AddPerk(FName(TEXT("EliteGarage")), TEXT("Elite Garage"), TEXT("+10 crew garage slots"), 40, 10.0f);
	AddPerk(FName(TEXT("MasterBoost")), TEXT("Master Crew"), TEXT("+20% all bonuses"), 50, 0.20f);
}

void UMGCrewSubsystem::InitializeTerritories()
{
	// Initialize territory data for each district
	auto AddTerritory = [this](FName ID, const FString& Name, FName District, FName Bonus, float Value)
	{
		FMGCrewTerritory Territory;
		Territory.TerritoryID = ID;
		Territory.TerritoryName = FText::FromString(Name);
		Territory.DistrictID = District;
		Territory.Status = EMGTerritoryStatus::Unclaimed;
		Territory.BonusType = Bonus;
		Territory.BonusValue = Value;
		Territories.Add(ID, Territory);
	};

	// Downtown territories
	AddTerritory(FName(TEXT("DT_Central")), TEXT("Central Plaza"), FName(TEXT("Downtown")), FName(TEXT("Cash")), 0.05f);
	AddTerritory(FName(TEXT("DT_Financial")), TEXT("Financial District"), FName(TEXT("Downtown")), FName(TEXT("Cash")), 0.08f);
	AddTerritory(FName(TEXT("DT_NightLife")), TEXT("Nightlife Strip"), FName(TEXT("Downtown")), FName(TEXT("Rep")), 0.05f);

	// Industrial territories
	AddTerritory(FName(TEXT("IND_Docks")), TEXT("The Docks"), FName(TEXT("Industrial")), FName(TEXT("XP")), 0.05f);
	AddTerritory(FName(TEXT("IND_Warehouse")), TEXT("Warehouse Row"), FName(TEXT("Industrial")), FName(TEXT("XP")), 0.07f);
	AddTerritory(FName(TEXT("IND_Rail")), TEXT("Rail Yard"), FName(TEXT("Industrial")), FName(TEXT("Cash")), 0.06f);

	// Highway territories
	AddTerritory(FName(TEXT("HWY_East")), TEXT("Eastern Highway"), FName(TEXT("Highway")), FName(TEXT("XP")), 0.08f);
	AddTerritory(FName(TEXT("HWY_West")), TEXT("Western Expressway"), FName(TEXT("Highway")), FName(TEXT("Rep")), 0.07f);

	// Hills/Canyon territories
	AddTerritory(FName(TEXT("HILL_Peak")), TEXT("Overlook Peak"), FName(TEXT("Hills")), FName(TEXT("Rep")), 0.10f);
	AddTerritory(FName(TEXT("HILL_Pass")), TEXT("Mountain Pass"), FName(TEXT("Hills")), FName(TEXT("XP")), 0.10f);
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

		AddSystemChatMessage(FString::Printf(TEXT("Congratulations! Our crew is now Level %d!"), CurrentCrew.Level));

		if (NewlyUnlocked.Num() > 0)
		{
			for (const FMGCrewPerk& Perk : NewlyUnlocked)
			{
				AddSystemChatMessage(FString::Printf(TEXT("New perk unlocked: %s"), *Perk.PerkName.ToString()));
			}
		}

		OnCrewLevelUp.Broadcast(CurrentCrew.Level, NewlyUnlocked);
	}
}

void UMGCrewSubsystem::CheckChallenges()
{
	FDateTime Now = FDateTime::UtcNow();

	for (FMGCrewChallenge& Challenge : ActiveChallenges)
	{
		// Check if challenge expired
		if (!Challenge.bCompleted && Now > Challenge.EndTime)
		{
			// Challenge failed - could add penalty or notification
		}
		// Check if challenge completed
		else if (!Challenge.bCompleted && Challenge.CurrentValue >= Challenge.TargetValue)
		{
			Challenge.bCompleted = true;
			OnCrewChallengeCompleted.Broadcast(Challenge);
		}
	}
}

int64 UMGCrewSubsystem::CalculateExperienceForLevel(int32 Level) const
{
	// Exponential growth: base 1000, 1.5x multiplier per level
	return static_cast<int64>(1000 * FMath::Pow(1.5f, Level - 1));
}

void UMGCrewSubsystem::SaveCrewData()
{
	// In production, this would save to USaveGame or online services
}

void UMGCrewSubsystem::LoadCrewData()
{
	// In production, this would load from USaveGame or online services
}

void UMGCrewSubsystem::OnCrewTick()
{
	// Periodic maintenance tasks
	CleanupExpiredData();
	CheckChallenges();

	// Update member online status (in production, would query online services)
}

void UMGCrewSubsystem::CleanupExpiredData()
{
	FDateTime Now = FDateTime::UtcNow();

	// Remove expired invites
	for (int32 i = PendingInvites.Num() - 1; i >= 0; --i)
	{
		if (PendingInvites[i].ExpiresDate < Now)
		{
			PendingInvites.RemoveAt(i);
		}
	}
}

void UMGCrewSubsystem::AddSystemChatMessage(const FString& Content)
{
	FMGCrewChatMessage Message;
	Message.MessageID = FGuid::NewGuid();
	Message.SenderID = NAME_None;
	Message.SenderName = TEXT("System");
	Message.SenderRank = EMGCrewRank::Leader;
	Message.MessageType = EMGCrewChatType::System;
	Message.Content = Content;
	Message.Timestamp = FDateTime::UtcNow();
	Message.bIsRead = false;

	ChatHistory.Add(Message);

	while (ChatHistory.Num() > MaxChatHistorySize)
	{
		ChatHistory.RemoveAt(0);
	}

	OnCrewChatMessageReceived.Broadcast(Message);
}

// ============================================================================
// Crew Battles Integration
// ============================================================================

void UMGCrewSubsystem::GetCrewBattleInfo(FName& OutCrewID, FString& OutCrewName, FString& OutCrewTag, int32& OutRating) const
{
	if (IsInCrew())
	{
		OutCrewID = FName(*CurrentCrew.CrewID.ToString());
		OutCrewName = CurrentCrew.CrewName;
		OutCrewTag = CurrentCrew.CrewTag;
		OutRating = CurrentCrew.BattleRating;
	}
	else
	{
		OutCrewID = NAME_None;
		OutCrewName = TEXT("");
		OutCrewTag = TEXT("");
		OutRating = 0;
	}
}

void UMGCrewSubsystem::UpdateBattleRating(int32 NewRating, bool bWon)
{
	if (!IsInCrew())
	{
		return;
	}

	int32 OldRating = CurrentCrew.BattleRating;
	CurrentCrew.BattleRating = NewRating;

	// Award XP based on battle result
	int64 XPAwarded = bWon ? 500 : 100;
	AddCrewExperience(XPAwarded, bWon ? TEXT("Battle Victory") : TEXT("Battle Participation"));

	// Award treasury bonus for wins
	if (bWon)
	{
		int64 TreasuryBonus = 1000 + (FMath::Abs(NewRating - OldRating) * 10);
		CurrentCrew.Treasury += TreasuryBonus;
	}

	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
}

bool UMGCrewSubsystem::CanParticipateInBattles() const
{
	if (!IsInCrew())
	{
		return false;
	}

	// Require minimum level 5 to participate in battles
	if (CurrentCrew.Level < 5)
	{
		return false;
	}

	// Require minimum 3 members
	if (CurrentCrew.CurrentMembers < 3)
	{
		return false;
	}

	// Require at least 2 online members
	int32 OnlineCount = 0;
	for (const FMGCrewMember& Member : Members)
	{
		if (Member.bIsOnline)
		{
			OnlineCount++;
		}
	}

	return OnlineCount >= 2;
}

TArray<FName> UMGCrewSubsystem::GetBattleRoster(int32 MaxPlayers) const
{
	TArray<FName> Roster;

	if (!IsInCrew())
	{
		return Roster;
	}

	// Prioritize online members, then sort by contribution
	TArray<FMGCrewMember> SortedMembers = Members;
	SortedMembers.Sort([](const FMGCrewMember& A, const FMGCrewMember& B)
	{
		// Online members first
		if (A.bIsOnline != B.bIsOnline)
		{
			return A.bIsOnline > B.bIsOnline;
		}
		// Then by contribution points
		return A.ContributionPoints > B.ContributionPoints;
	});

	for (int32 i = 0; i < FMath::Min(SortedMembers.Num(), MaxPlayers); ++i)
	{
		if (SortedMembers[i].bIsOnline)
		{
			Roster.Add(SortedMembers[i].PlayerID);
		}
	}

	return Roster;
}

void UMGCrewSubsystem::RecordBattleResult(bool bWon, const FString& OpponentName, int32 RatingChange)
{
	if (!IsInCrew())
	{
		return;
	}

	// Update stats
	CurrentCrew.TotalRaces++;
	if (bWon)
	{
		CurrentCrew.TotalWins++;
	}

	// Record activity
	EMGCrewActivityType ActivityType = bWon ? EMGCrewActivityType::CrewBattleWon : EMGCrewActivityType::CrewBattleLost;
	FString ResultText = bWon ? TEXT("won") : TEXT("lost");
	FString RatingText = RatingChange > 0 ? FString::Printf(TEXT("+%d"), RatingChange) : FString::Printf(TEXT("%d"), RatingChange);

	RecordActivity(ActivityType,
		FText::FromString(FString::Printf(TEXT("Crew %s battle against %s (%s rating)"), *ResultText, *OpponentName, *RatingText)),
		bWon ? 100 : 25);

	// System announcement
	AddSystemChatMessage(FString::Printf(TEXT("Battle %s! %s vs %s (Rating: %s)"),
		bWon ? TEXT("Victory") : TEXT("Defeat"),
		*CurrentCrew.CrewName, *OpponentName, *RatingText));

	// Update stats for participating members
	for (FMGCrewMember& Member : Members)
	{
		if (Member.bIsOnline)
		{
			Member.RacesForCrew++;
			if (bWon)
			{
				Member.WinsForCrew++;
			}
		}
	}

	OnCrewUpdated.Broadcast(CurrentCrew);
	SaveCrewData();
}

// ============================================================================
// Reputation Sharing System
// ============================================================================

int32 UMGCrewSubsystem::ContributeReputation(int32 Amount, FName Category)
{
	if (!IsInCrew() || Amount <= 0)
	{
		return 0;
	}

	// Calculate contribution based on member rank
	const float ContributionRate = GetReputationContributionRate(GetLocalPlayerRank());
	const int32 Contribution = FMath::RoundToInt(Amount * ContributionRate);

	if (Contribution <= 0)
	{
		return 0;
	}

	// Add to crew pool
	CrewReputationPool += Contribution;

	// Track weekly contribution
	if (WeeklyReputationContributions.Contains(LocalPlayerID))
	{
		WeeklyReputationContributions[LocalPlayerID] += Contribution;
	}
	else
	{
		WeeklyReputationContributions.Add(LocalPlayerID, Contribution);
	}

	// Update member contribution points
	for (FMGCrewMember& Member : Members)
	{
		if (Member.PlayerID == LocalPlayerID)
		{
			Member.ContributionPoints += Contribution;
			Member.WeeklyContribution += Contribution;
			break;
		}
	}

	// Check for tier change
	FName NewTier = GetCrewReputationTier();
	if (NewTier != CurrentReputationTier)
	{
		FName OldTier = CurrentReputationTier;
		CurrentReputationTier = NewTier;
		OnCrewReputationTierChanged.Broadcast(OldTier, NewTier);

		// Add activity
		RecordActivity(EMGCrewActivityType::LevelUp,
			FText::FromString(FString::Printf(TEXT("Crew reached %s reputation tier!"), *NewTier.ToString())),
			500);
	}

	OnCrewReputationContributed.Broadcast(LocalPlayerID, Contribution, CrewReputationPool);
	SaveCrewData();

	return Contribution;
}

float UMGCrewSubsystem::GetReputationBonusMultiplier() const
{
	if (!IsInCrew())
	{
		return 1.0f;
	}

	float Bonus = 1.0f;

	// Base bonus from crew level (0.5% per level)
	Bonus += CurrentCrew.Level * 0.005f;

	// Bonus from reputation tier
	if (CurrentReputationTier == TEXT("Silver"))
	{
		Bonus += 0.05f;
	}
	else if (CurrentReputationTier == TEXT("Gold"))
	{
		Bonus += 0.10f;
	}
	else if (CurrentReputationTier == TEXT("Platinum"))
	{
		Bonus += 0.15f;
	}
	else if (CurrentReputationTier == TEXT("Diamond"))
	{
		Bonus += 0.20f;
	}

	// Bonus from territory control
	Bonus += GetTerritoryBonus(TEXT("Rep")) * 0.01f;

	// Bonus from active perks
	Bonus += GetPerkValue(TEXT("Reputation_I")) * 0.01f;
	Bonus += GetPerkValue(TEXT("Reputation_II")) * 0.01f;
	Bonus += GetPerkValue(TEXT("Reputation_III")) * 0.01f;

	return Bonus;
}

FName UMGCrewSubsystem::GetCrewReputationTier() const
{
	if (CrewReputationPool >= 1000000)
	{
		return TEXT("Diamond");
	}
	else if (CrewReputationPool >= 200000)
	{
		return TEXT("Platinum");
	}
	else if (CrewReputationPool >= 50000)
	{
		return TEXT("Gold");
	}
	else if (CrewReputationPool >= 10000)
	{
		return TEXT("Silver");
	}
	return TEXT("Bronze");
}

float UMGCrewSubsystem::GetReputationContributionRate(EMGCrewRank Rank)
{
	// Higher ranks contribute more to the crew pool
	switch (Rank)
	{
		case EMGCrewRank::Prospect:   return 0.05f; // 5%
		case EMGCrewRank::Member:     return 0.08f; // 8%
		case EMGCrewRank::Veteran:    return 0.10f; // 10%
		case EMGCrewRank::Officer:    return 0.12f; // 12%
		case EMGCrewRank::Lieutenant: return 0.14f; // 14%
		case EMGCrewRank::Captain:    return 0.16f; // 16%
		case EMGCrewRank::Leader:     return 0.18f; // 18%
		default:                      return 0.10f;
	}
}

int64 UMGCrewSubsystem::DistributeWeeklyReputationRewards()
{
	if (!IsInCrew() || Members.Num() == 0)
	{
		return 0;
	}

	// Calculate total weekly contributions
	int64 TotalContributions = 0;
	for (const auto& Pair : WeeklyReputationContributions)
	{
		TotalContributions += Pair.Value;
	}

	if (TotalContributions <= 0)
	{
		return 0;
	}

	// Bonus pool is 20% of weekly contributions
	const int64 BonusPool = FMath::RoundToInt64(TotalContributions * 0.20f);

	int64 TotalDistributed = 0;
	int32 MembersRewarded = 0;

	// Distribute based on contribution share
	for (FMGCrewMember& Member : Members)
	{
		if (WeeklyReputationContributions.Contains(Member.PlayerID))
		{
			const int32 MemberContribution = WeeklyReputationContributions[Member.PlayerID];
			const float Share = static_cast<float>(MemberContribution) / static_cast<float>(TotalContributions);
			const int64 Reward = FMath::RoundToInt64(BonusPool * Share);

			if (Reward > 0)
			{
				// In production, this would add to the member's actual reputation
				// For now, we track it as contribution points
				Member.ContributionPoints += Reward;
				TotalDistributed += Reward;
				MembersRewarded++;
			}
		}
	}

	// Record activity
	RecordActivity(EMGCrewActivityType::Donation,
		FText::FromString(FString::Printf(TEXT("Weekly reputation rewards distributed: %lld rep to %d members"),
			TotalDistributed, MembersRewarded)),
		0);

	// Reset weekly tracking
	WeeklyReputationContributions.Empty();
	for (FMGCrewMember& Member : Members)
	{
		Member.WeeklyContribution = 0;
	}

	LastReputationDistribution = FDateTime::UtcNow();
	OnCrewReputationDistributed.Broadcast(TotalDistributed, MembersRewarded);
	SaveCrewData();

	return TotalDistributed;
}

int32 UMGCrewSubsystem::GetMemberWeeklyReputationShare(FName MemberID) const
{
	if (!IsInCrew())
	{
		return 0;
	}

	// Calculate total weekly contributions
	int64 TotalContributions = 0;
	for (const auto& Pair : WeeklyReputationContributions)
	{
		TotalContributions += Pair.Value;
	}

	if (TotalContributions <= 0)
	{
		return 0;
	}

	// Get member's contribution
	if (!WeeklyReputationContributions.Contains(MemberID))
	{
		return 0;
	}

	const int32 MemberContribution = WeeklyReputationContributions[MemberID];
	const float Share = static_cast<float>(MemberContribution) / static_cast<float>(TotalContributions);

	// Estimate reward (20% of total as bonus pool)
	const int64 BonusPool = FMath::RoundToInt64(TotalContributions * 0.20f);
	return FMath::RoundToInt(BonusPool * Share);
}

TArray<FMGCrewMember> UMGCrewSubsystem::GetReputationLeaderboard(int32 Count) const
{
	TArray<FMGCrewMember> Leaderboard = Members;

	// Sort by weekly contribution descending
	Leaderboard.Sort([](const FMGCrewMember& A, const FMGCrewMember& B)
	{
		return A.WeeklyContribution > B.WeeklyContribution;
	});

	// Limit to requested count
	if (Leaderboard.Num() > Count)
	{
		Leaderboard.SetNum(Count);
	}

	return Leaderboard;
}

bool UMGCrewSubsystem::HasReachedReputationMilestone(int64 MilestoneAmount) const
{
	return CrewReputationPool >= MilestoneAmount;
}

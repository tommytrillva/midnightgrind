// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCrewSubsystem.generated.h"

class UTexture2D;

/**
 * @brief Crew member rank hierarchy
 *
 * Defines the rank structure within a crew, with each rank
 * providing increasing permissions and responsibilities.
 */
UENUM(BlueprintType)
enum class EMGCrewRank : uint8
{
	/** New member, limited permissions */
	Prospect		UMETA(DisplayName = "Prospect"),
	/** Regular member */
	Member			UMETA(DisplayName = "Member"),
	/** Experienced member with seniority */
	Veteran			UMETA(DisplayName = "Veteran"),
	/** Can invite members, manage events */
	Officer			UMETA(DisplayName = "Officer"),
	/** Senior officer, can kick/promote members */
	Lieutenant		UMETA(DisplayName = "Lieutenant"),
	/** Second-in-command, full permissions except disband */
	Captain			UMETA(DisplayName = "Captain"),
	/** Crew founder/leader, full permissions */
	Leader			UMETA(DisplayName = "Leader")
};

/**
 * @brief Crew privacy/recruitment settings
 */
UENUM(BlueprintType)
enum class EMGCrewPrivacy : uint8
{
	/** Anyone can join instantly */
	Open			UMETA(DisplayName = "Open"),
	/** Applications require approval */
	ApprovalRequired UMETA(DisplayName = "Approval Required"),
	/** Members can only join via invite */
	InviteOnly		UMETA(DisplayName = "Invite Only"),
	/** No new members allowed */
	Closed			UMETA(DisplayName = "Closed")
};

/**
 * @brief Types of crew activity for the activity feed
 */
UENUM(BlueprintType)
enum class EMGCrewActivityType : uint8
{
	RaceCompleted,
	RaceWon,
	MemberJoined,
	MemberLeft,
	MemberKicked,
	Promotion,
	Demotion,
	CrewChallenge,
	LevelUp,
	Achievement,
	Donation,
	TerritoryWon,
	TerritoryLost,
	CrewBattleWon,
	CrewBattleLost,
	GarageVehicleAdded,
	ChatMessage
};

/**
 * @brief Crew permissions that can be granted to ranks
 */
UENUM(BlueprintType)
enum class EMGCrewPermission : uint8
{
	/** Can view crew info */
	ViewCrew			UMETA(DisplayName = "View Crew"),
	/** Can participate in crew events */
	ParticipateEvents	UMETA(DisplayName = "Participate in Events"),
	/** Can use crew chat */
	UseChat				UMETA(DisplayName = "Use Chat"),
	/** Can access shared garage */
	AccessGarage		UMETA(DisplayName = "Access Garage"),
	/** Can borrow crew vehicles */
	BorrowVehicles		UMETA(DisplayName = "Borrow Vehicles"),
	/** Can invite new members */
	InviteMembers		UMETA(DisplayName = "Invite Members"),
	/** Can accept applications */
	AcceptApplications	UMETA(DisplayName = "Accept Applications"),
	/** Can kick lower-ranked members */
	KickMembers			UMETA(DisplayName = "Kick Members"),
	/** Can promote members to lower ranks */
	PromoteMembers		UMETA(DisplayName = "Promote Members"),
	/** Can demote lower-ranked members */
	DemoteMembers		UMETA(DisplayName = "Demote Members"),
	/** Can add vehicles to crew garage */
	ManageGarage		UMETA(DisplayName = "Manage Garage"),
	/** Can edit crew info (description, motto) */
	EditCrewInfo		UMETA(DisplayName = "Edit Crew Info"),
	/** Can initiate crew battles */
	StartCrewBattles	UMETA(DisplayName = "Start Crew Battles"),
	/** Can manage crew challenges */
	ManageChallenges	UMETA(DisplayName = "Manage Challenges"),
	/** Can withdraw from treasury */
	ManageTreasury		UMETA(DisplayName = "Manage Treasury"),
	/** Can claim territories */
	ClaimTerritory		UMETA(DisplayName = "Claim Territory"),
	/** Can change crew settings */
	ChangeSettings		UMETA(DisplayName = "Change Settings"),
	/** Can transfer leadership */
	TransferLeadership	UMETA(DisplayName = "Transfer Leadership"),
	/** Can disband crew */
	DisbandCrew			UMETA(DisplayName = "Disband Crew")
};

/**
 * @brief Territory status
 */
UENUM(BlueprintType)
enum class EMGTerritoryStatus : uint8
{
	/** No crew owns this territory */
	Unclaimed		UMETA(DisplayName = "Unclaimed"),
	/** Owned by the local crew */
	Owned			UMETA(DisplayName = "Owned"),
	/** Owned by another crew */
	Rival			UMETA(DisplayName = "Rival"),
	/** Currently being contested */
	Contested		UMETA(DisplayName = "Contested")
};

/**
 * @brief Chat message type
 */
UENUM(BlueprintType)
enum class EMGCrewChatType : uint8
{
	/** Regular text message */
	Text			UMETA(DisplayName = "Text"),
	/** System announcement */
	System			UMETA(DisplayName = "System"),
	/** Join/leave notification */
	Notification	UMETA(DisplayName = "Notification"),
	/** Officer broadcast */
	Broadcast		UMETA(DisplayName = "Broadcast")
};

/**
 * @brief Crew member data structure
 */
USTRUCT(BlueprintType)
struct FMGCrewMember
{
	GENERATED_BODY()

	/** Unique player identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Member")
	FName PlayerID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Member")
	FString PlayerName;

	/** Current rank within the crew */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Member")
	EMGCrewRank Rank = EMGCrewRank::Member;

	/** When the member joined the crew */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Member")
	FDateTime JoinedDate;

	/** Last time this member was active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Member")
	FDateTime LastActiveDate;

	/** Total contribution points earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 ContributionPoints = 0;

	/** Weekly contribution points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 WeeklyContribution = 0;

	/** Races completed for the crew */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 RacesForCrew = 0;

	/** Race wins for the crew */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 WinsForCrew = 0;

	/** Total currency donated to treasury */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 CurrencyDonated = 0;

	/** Player's overall level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Member")
	int32 PlayerLevel = 1;

	/** Whether the member is currently online */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool bIsOnline = false;

	/** Current activity description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	FString CurrentActivity;

	/** Custom note about this member (visible to officers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Member")
	FString OfficerNote;
};

/**
 * @brief Crew information data structure
 */
USTRUCT(BlueprintType)
struct FMGCrewInfo
{
	GENERATED_BODY()

	/** Unique crew identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FGuid CrewID;

	/** Crew display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CrewName;

	/** Short crew tag (2-5 characters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CrewTag;

	/** Crew description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Description;

	/** Crew motto */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString Motto;

	/** Recruitment privacy setting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	EMGCrewPrivacy Privacy = EMGCrewPrivacy::ApprovalRequired;

	/** Minimum level required to join */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 MinimumJoinLevel = 1;

	/** When the crew was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FDateTime CreatedDate;

	/** Current crew level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 Level = 1;

	/** Current experience points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int64 ExperiencePoints = 0;

	/** Experience required for next level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int64 ExperienceToNextLevel = 1000;

	/** Maximum number of members allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capacity")
	int32 MaxMembers = 50;

	/** Current member count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capacity")
	int32 CurrentMembers = 0;

	/** Total race wins for the crew */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 TotalWins = 0;

	/** Total races participated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int64 TotalRaces = 0;

	/** Crew treasury balance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int64 Treasury = 0;

	/** Global ranking position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	int32 GlobalRank = 0;

	/** Regional ranking position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	int32 RegionalRank = 0;

	/** Crew battle rating (ELO-based) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
	int32 BattleRating = 1500;

	/** Region identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString Region;

	/** Crew emblem texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TSoftObjectPtr<UTexture2D> Emblem;

	/** Primary crew color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor PrimaryColor = FLinearColor::Blue;

	/** Secondary crew color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor SecondaryColor = FLinearColor::White;

	/** Home territory ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FName HomeTerritoryID;

	/** Number of territories owned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	int32 TerritoriesOwned = 0;
};

/**
 * @brief Activity feed entry
 */
USTRUCT(BlueprintType)
struct FMGCrewActivity
{
	GENERATED_BODY()

	/** Type of activity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
	EMGCrewActivityType Type = EMGCrewActivityType::RaceCompleted;

	/** Player who triggered the activity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
	FName PlayerID;

	/** Player display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
	FString PlayerName;

	/** Activity description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
	FText Description;

	/** When the activity occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
	FDateTime Timestamp;

	/** Points earned from this activity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
	int64 PointsEarned = 0;

	/** Additional context data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
	FString ContextData;
};

/**
 * @brief Crew challenge/objective
 */
USTRUCT(BlueprintType)
struct FMGCrewChallenge
{
	GENERATED_BODY()

	/** Unique challenge identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FName ChallengeID;

	/** Challenge title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FText Title;

	/** Challenge description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FText Description;

	/** Target value to complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	int64 TargetValue = 0;

	/** Current progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	int64 CurrentValue = 0;

	/** When the challenge started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FDateTime StartTime;

	/** When the challenge expires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FDateTime EndTime;

	/** Experience reward for completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 ExperienceReward = 0;

	/** Currency reward for completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 CurrencyReward = 0;

	/** Whether the challenge is complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	bool bCompleted = false;

	/** Whether rewards have been claimed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	bool bRewardsClaimed = false;
};

/**
 * @brief Crew perk/bonus
 */
USTRUCT(BlueprintType)
struct FMGCrewPerk
{
	GENERATED_BODY()

	/** Unique perk identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk")
	FName PerkID;

	/** Perk display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk")
	FText PerkName;

	/** Perk description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk")
	FText Description;

	/** Crew level required to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk")
	int32 RequiredLevel = 1;

	/** Whether the perk is unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk")
	bool bIsUnlocked = false;

	/** Perk bonus value (percentage or flat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk")
	float BonusValue = 0.0f;

	/** Perk icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * @brief Crew invite structure
 */
USTRUCT(BlueprintType)
struct FMGCrewInvite
{
	GENERATED_BODY()

	/** Unique invite identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invite")
	FGuid InviteID;

	/** Crew being invited to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invite")
	FGuid CrewID;

	/** Name of the crew */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invite")
	FString CrewName;

	/** Who sent the invite */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invite")
	FName InviterID;

	/** Inviter display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invite")
	FString InviterName;

	/** When the invite was sent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invite")
	FDateTime SentDate;

	/** When the invite expires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invite")
	FDateTime ExpiresDate;

	/** Optional message from inviter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Invite")
	FString Message;
};

/**
 * @brief Application to join a crew
 */
USTRUCT(BlueprintType)
struct FMGCrewApplication
{
	GENERATED_BODY()

	/** Unique application identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Application")
	FGuid ApplicationID;

	/** Applicant player ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Application")
	FName ApplicantID;

	/** Applicant display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Application")
	FString ApplicantName;

	/** Applicant's level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Application")
	int32 ApplicantLevel = 1;

	/** Applicant's reputation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Application")
	int32 ApplicantReputation = 0;

	/** Application message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Application")
	FString Message;

	/** When the application was submitted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Application")
	FDateTime AppliedDate;
};

/**
 * @brief Chat message in crew chat
 */
USTRUCT(BlueprintType)
struct FMGCrewChatMessage
{
	GENERATED_BODY()

	/** Unique message identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FGuid MessageID;

	/** Sender player ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FName SenderID;

	/** Sender display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString SenderName;

	/** Sender's rank at time of message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	EMGCrewRank SenderRank = EMGCrewRank::Member;

	/** Message type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	EMGCrewChatType MessageType = EMGCrewChatType::Text;

	/** Message content */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Content;

	/** When the message was sent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FDateTime Timestamp;

	/** Whether the message has been read */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	bool bIsRead = false;
};

/**
 * @brief Vehicle in the shared crew garage
 */
USTRUCT(BlueprintType)
struct FMGCrewGarageVehicle
{
	GENERATED_BODY()

	/** Unique vehicle instance ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FGuid VehicleInstanceID;

	/** Vehicle definition ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FName VehicleID;

	/** Vehicle display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FString VehicleName;

	/** Who donated this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FName DonorPlayerID;

	/** Donor display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FString DonorName;

	/** When the vehicle was added */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FDateTime AddedDate;

	/** Who is currently borrowing this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FName BorrowerID;

	/** Borrower display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FString BorrowerName;

	/** Vehicle performance index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 PerformanceIndex = 0;

	/** Vehicle class (D, C, B, A, S) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FString VehicleClass;

	/** Estimated value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int64 EstimatedValue = 0;

	/** Whether the vehicle is currently borrowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bIsBorrowed = false;

	/** Minimum rank required to borrow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	EMGCrewRank MinimumBorrowRank = EMGCrewRank::Member;
};

/**
 * @brief Territory data
 */
USTRUCT(BlueprintType)
struct FMGCrewTerritory
{
	GENERATED_BODY()

	/** Territory identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FName TerritoryID;

	/** Territory display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FText TerritoryName;

	/** District this territory belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FName DistrictID;

	/** Current ownership status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	EMGTerritoryStatus Status = EMGTerritoryStatus::Unclaimed;

	/** Crew that owns this territory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FGuid OwningCrewID;

	/** Name of owning crew */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FString OwningCrewName;

	/** When the territory was captured */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FDateTime CapturedDate;

	/** Bonus type provided (e.g., "XP", "Cash", "Rep") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FName BonusType;

	/** Bonus value (percentage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	float BonusValue = 0.0f;

	/** Whether territory is currently contested */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	bool bIsContested = false;

	/** Crew challenging for this territory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
	FGuid ChallengingCrewID;
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewJoined, const FMGCrewInfo&, Crew);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCrewLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewUpdated, const FMGCrewInfo&, Crew);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewMemberJoined, const FMGCrewMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewMemberLeft, FName, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewMemberRankChanged, FName, PlayerID, EMGCrewRank, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewLevelUp, int32, NewLevel, const TArray<FMGCrewPerk>&, UnlockedPerks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewChallengeCompleted, const FMGCrewChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewInviteReceived, const FMGCrewInvite&, Invite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewApplicationReceived, const FMGCrewApplication&, Application);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewActivityAdded, const FMGCrewActivity&, Activity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewChatMessageReceived, const FMGCrewChatMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewTerritoryChanged, const FMGCrewTerritory&, Territory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewGarageVehicleAdded, const FMGCrewGarageVehicle&, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewGarageVehicleBorrowed, const FMGCrewGarageVehicle&, Vehicle, FName, BorrowerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCrewReputationContributed, FName, MemberID, int32, Amount, int64, NewPoolTotal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewReputationTierChanged, FName, OldTier, FName, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewReputationDistributed, int64, TotalDistributed, int32, MemberCount);

/**
 * @class UMGCrewSubsystem
 * @brief Core subsystem for managing crew/club functionality
 *
 * The Crew System is central to the "Living Car Culture" design pillar.
 * It provides:
 * - Crew creation, management, and progression
 * - Member management with rank-based permissions
 * - Invite and application systems
 * - Shared crew garage for vehicles
 * - Territory control system
 * - Crew chat and communication
 * - Crew challenges and perks
 * - Integration with Crew Battles
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCrewSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface

	// ========================================================================
	// Crew Management
	// ========================================================================

	/**
	 * @brief Create a new crew
	 * @param Name Crew display name (3-32 characters)
	 * @param Tag Short crew tag (2-5 characters)
	 * @param Description Crew description
	 * @return True if crew was created successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Management")
	bool CreateCrew(const FString& Name, const FString& Tag, const FString& Description);

	/**
	 * @brief Disband the current crew (Leader only)
	 * @return True if crew was disbanded
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Management")
	bool DisbandCrew();

	/**
	 * @brief Leave the current crew
	 * @return True if successfully left
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Management")
	bool LeaveCrew();

	/** @brief Check if the local player is in a crew */
	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	bool IsInCrew() const { return CurrentCrew.CrewID.IsValid(); }

	/** @brief Get current crew information */
	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	FMGCrewInfo GetCurrentCrew() const { return CurrentCrew; }

	/** @brief Check if local player is the crew leader */
	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	bool IsCrewLeader() const;

	/** @brief Check if local player is an officer or higher */
	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	bool IsCrewOfficer() const;

	/** @brief Get the local player's rank in the crew */
	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	EMGCrewRank GetLocalPlayerRank() const;

	// ========================================================================
	// Permissions
	// ========================================================================

	/**
	 * @brief Check if the local player has a specific permission
	 * @param Permission The permission to check
	 * @return True if the player has the permission
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Permissions")
	bool HasPermission(EMGCrewPermission Permission) const;

	/**
	 * @brief Check if a specific rank has a permission
	 * @param Rank The rank to check
	 * @param Permission The permission to check
	 * @return True if the rank has the permission
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Permissions")
	static bool RankHasPermission(EMGCrewRank Rank, EMGCrewPermission Permission);

	/**
	 * @brief Get all permissions for a rank
	 * @param Rank The rank to get permissions for
	 * @return Array of permissions the rank has
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Permissions")
	static TArray<EMGCrewPermission> GetPermissionsForRank(EMGCrewRank Rank);

	/**
	 * @brief Get display name for a rank
	 * @param Rank The rank
	 * @return Localized display name
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Permissions")
	static FText GetRankDisplayName(EMGCrewRank Rank);

	// ========================================================================
	// Member Management
	// ========================================================================

	/** @brief Get all crew members */
	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	TArray<FMGCrewMember> GetMembers() const { return Members; }

	/** @brief Get only online crew members */
	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	TArray<FMGCrewMember> GetOnlineMembers() const;

	/** @brief Get members by rank */
	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	TArray<FMGCrewMember> GetMembersByRank(EMGCrewRank Rank) const;

	/**
	 * @brief Get a specific member by ID
	 * @param PlayerID Player to look up
	 * @return Member data (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	FMGCrewMember GetMember(FName PlayerID) const;

	/** @brief Get the local player's member data */
	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	FMGCrewMember GetLocalMember() const;

	/**
	 * @brief Kick a member from the crew
	 * @param PlayerID Member to kick
	 * @param Reason Optional reason for kicking
	 * @return True if member was kicked
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	bool KickMember(FName PlayerID, const FString& Reason = TEXT(""));

	/**
	 * @brief Promote a member to a higher rank
	 * @param PlayerID Member to promote
	 * @param NewRank Target rank
	 * @return True if member was promoted
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	bool PromoteMember(FName PlayerID, EMGCrewRank NewRank);

	/**
	 * @brief Demote a member to a lower rank
	 * @param PlayerID Member to demote
	 * @param NewRank Target rank
	 * @return True if member was demoted
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	bool DemoteMember(FName PlayerID, EMGCrewRank NewRank);

	/**
	 * @brief Transfer crew leadership to another member
	 * @param NewLeaderID Player to become the new leader
	 * @return True if leadership was transferred
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	bool TransferLeadership(FName NewLeaderID);

	/**
	 * @brief Set an officer note for a member
	 * @param PlayerID Member to set note for
	 * @param Note Note content
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	void SetMemberNote(FName PlayerID, const FString& Note);

	// ========================================================================
	// Invites & Applications
	// ========================================================================

	/**
	 * @brief Send an invite to a player
	 * @param PlayerID Player to invite
	 * @param Message Optional message with the invite
	 * @return True if invite was sent
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Invites")
	bool SendInvite(FName PlayerID, const FString& Message = TEXT(""));

	/**
	 * @brief Accept a crew invite
	 * @param InviteID Invite to accept
	 * @return True if invite was accepted
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Invites")
	bool AcceptInvite(FGuid InviteID);

	/**
	 * @brief Decline a crew invite
	 * @param InviteID Invite to decline
	 * @return True if invite was declined
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Invites")
	bool DeclineInvite(FGuid InviteID);

	/** @brief Get all pending invites for the local player */
	UFUNCTION(BlueprintPure, Category = "Crew|Invites")
	TArray<FMGCrewInvite> GetPendingInvites() const { return PendingInvites; }

	/**
	 * @brief Apply to join a crew
	 * @param CrewID Crew to apply to
	 * @param Message Application message
	 * @return True if application was sent
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Applications")
	bool ApplyToCrew(FGuid CrewID, const FString& Message);

	/**
	 * @brief Accept a join application
	 * @param ApplicationID Application to accept
	 * @return True if application was accepted
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Applications")
	bool AcceptApplication(FGuid ApplicationID);

	/**
	 * @brief Reject a join application
	 * @param ApplicationID Application to reject
	 * @return True if application was rejected
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Applications")
	bool RejectApplication(FGuid ApplicationID);

	/** @brief Get all pending applications to the crew */
	UFUNCTION(BlueprintPure, Category = "Crew|Applications")
	TArray<FMGCrewApplication> GetPendingApplications() const { return PendingApplications; }

	// ========================================================================
	// Crew Settings
	// ========================================================================

	/**
	 * @brief Update crew description and motto
	 * @param Description New description
	 * @param Motto New motto
	 * @return True if updated
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool UpdateCrewInfo(const FString& Description, const FString& Motto);

	/**
	 * @brief Set crew privacy level
	 * @param Privacy New privacy setting
	 * @return True if updated
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool SetCrewPrivacy(EMGCrewPrivacy Privacy);

	/**
	 * @brief Set minimum level to join
	 * @param MinLevel Minimum player level required
	 * @return True if updated
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool SetMinimumJoinLevel(int32 MinLevel);

	/**
	 * @brief Set crew colors
	 * @param Primary Primary color
	 * @param Secondary Secondary color
	 * @return True if updated
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool SetCrewColors(FLinearColor Primary, FLinearColor Secondary);

	/**
	 * @brief Set crew emblem
	 * @param Emblem Emblem texture
	 * @return True if updated
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool SetCrewEmblem(TSoftObjectPtr<UTexture2D> Emblem);

	// ========================================================================
	// Progression
	// ========================================================================

	/**
	 * @brief Add experience to the crew
	 * @param Amount XP amount
	 * @param Source Description of XP source
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Progression")
	void AddCrewExperience(int64 Amount, const FString& Source);

	/** @brief Get crew level progress as percentage (0-100) */
	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	float GetCrewLevelProgress() const;

	/** @brief Get all crew perks */
	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	TArray<FMGCrewPerk> GetCrewPerks() const { return CrewPerks; }

	/** @brief Get only unlocked perks */
	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	TArray<FMGCrewPerk> GetUnlockedPerks() const;

	/**
	 * @brief Check if a perk is unlocked
	 * @param PerkID Perk to check
	 * @return True if unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	bool HasPerk(FName PerkID) const;

	/**
	 * @brief Get the value of a perk (0 if not unlocked)
	 * @param PerkID Perk to get
	 * @return Perk bonus value
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	float GetPerkValue(FName PerkID) const;

	// ========================================================================
	// Treasury
	// ========================================================================

	/**
	 * @brief Donate currency to crew treasury
	 * @param Amount Amount to donate
	 * @return True if donation was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Treasury")
	bool DonateToTreasury(int64 Amount);

	/**
	 * @brief Withdraw from treasury (Leader/Captain only)
	 * @param Amount Amount to withdraw
	 * @param Purpose Reason for withdrawal
	 * @return True if withdrawal was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Treasury")
	bool WithdrawFromTreasury(int64 Amount, const FString& Purpose = TEXT(""));

	/** @brief Get current treasury balance */
	UFUNCTION(BlueprintPure, Category = "Crew|Treasury")
	int64 GetTreasuryBalance() const { return CurrentCrew.Treasury; }

	// ========================================================================
	// Challenges
	// ========================================================================

	/** @brief Get all active crew challenges */
	UFUNCTION(BlueprintPure, Category = "Crew|Challenges")
	TArray<FMGCrewChallenge> GetActiveChallenges() const { return ActiveChallenges; }

	/**
	 * @brief Contribute progress to a challenge
	 * @param ChallengeID Challenge to contribute to
	 * @param Amount Progress amount
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Challenges")
	void ContributeToChallenge(FName ChallengeID, int64 Amount);

	/**
	 * @brief Claim rewards for a completed challenge
	 * @param ChallengeID Challenge to claim
	 * @return True if rewards were claimed
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Challenges")
	bool ClaimChallengeRewards(FName ChallengeID);

	// ========================================================================
	// Activity Feed
	// ========================================================================

	/**
	 * @brief Get recent activity feed
	 * @param MaxEntries Maximum entries to return
	 * @return Array of activity entries
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Activity")
	TArray<FMGCrewActivity> GetActivityFeed(int32 MaxEntries = 50) const;

	/**
	 * @brief Record an activity to the feed
	 * @param Type Activity type
	 * @param Description Activity description
	 * @param Points Points earned
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Activity")
	void RecordActivity(EMGCrewActivityType Type, const FText& Description, int64 Points = 0);

	// ========================================================================
	// Chat System
	// ========================================================================

	/**
	 * @brief Send a chat message to the crew
	 * @param Content Message content
	 * @return True if message was sent
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Chat")
	bool SendChatMessage(const FString& Content);

	/**
	 * @brief Send a broadcast message (Officers+ only)
	 * @param Content Broadcast content
	 * @return True if broadcast was sent
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Chat")
	bool SendBroadcast(const FString& Content);

	/**
	 * @brief Get recent chat messages
	 * @param MaxMessages Maximum messages to return
	 * @return Array of chat messages
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Chat")
	TArray<FMGCrewChatMessage> GetChatHistory(int32 MaxMessages = 100) const;

	/** @brief Get count of unread messages */
	UFUNCTION(BlueprintPure, Category = "Crew|Chat")
	int32 GetUnreadMessageCount() const;

	/** @brief Mark all messages as read */
	UFUNCTION(BlueprintCallable, Category = "Crew|Chat")
	void MarkAllMessagesRead();

	/**
	 * @brief Receive a chat message (network callback)
	 * @param Message The received message
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Chat")
	void ReceiveChatMessage(const FMGCrewChatMessage& Message);

	// ========================================================================
	// Shared Garage
	// ========================================================================

	/** @brief Get all vehicles in the crew garage */
	UFUNCTION(BlueprintPure, Category = "Crew|Garage")
	TArray<FMGCrewGarageVehicle> GetGarageVehicles() const { return GarageVehicles; }

	/** @brief Get available (not borrowed) vehicles */
	UFUNCTION(BlueprintPure, Category = "Crew|Garage")
	TArray<FMGCrewGarageVehicle> GetAvailableVehicles() const;

	/**
	 * @brief Donate a vehicle to the crew garage
	 * @param VehicleID Vehicle to donate
	 * @param VehicleName Display name
	 * @param PerformanceIndex PI rating
	 * @param VehicleClass Class letter
	 * @param EstimatedValue Value in currency
	 * @return True if vehicle was donated
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Garage")
	bool DonateVehicleToGarage(FName VehicleID, const FString& VehicleName, int32 PerformanceIndex,
							   const FString& VehicleClass, int64 EstimatedValue);

	/**
	 * @brief Borrow a vehicle from the garage
	 * @param VehicleInstanceID Vehicle to borrow
	 * @return True if vehicle was borrowed
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Garage")
	bool BorrowVehicle(FGuid VehicleInstanceID);

	/**
	 * @brief Return a borrowed vehicle
	 * @param VehicleInstanceID Vehicle to return
	 * @return True if vehicle was returned
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Garage")
	bool ReturnVehicle(FGuid VehicleInstanceID);

	/**
	 * @brief Remove a vehicle from the garage (Officers+ only)
	 * @param VehicleInstanceID Vehicle to remove
	 * @return True if vehicle was removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Garage")
	bool RemoveVehicleFromGarage(FGuid VehicleInstanceID);

	/**
	 * @brief Set minimum rank to borrow a vehicle
	 * @param VehicleInstanceID Vehicle to modify
	 * @param MinRank Minimum rank required
	 * @return True if setting was changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Garage")
	bool SetVehicleBorrowRank(FGuid VehicleInstanceID, EMGCrewRank MinRank);

	/** @brief Get maximum garage capacity */
	UFUNCTION(BlueprintPure, Category = "Crew|Garage")
	int32 GetGarageCapacity() const;

	// ========================================================================
	// Territory System
	// ========================================================================

	/** @brief Get all territories owned by the crew */
	UFUNCTION(BlueprintPure, Category = "Crew|Territory")
	TArray<FMGCrewTerritory> GetOwnedTerritories() const;

	/**
	 * @brief Get territory information
	 * @param TerritoryID Territory to look up
	 * @return Territory data
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Territory")
	FMGCrewTerritory GetTerritory(FName TerritoryID) const;

	/**
	 * @brief Claim a territory (requires race victory in territory)
	 * @param TerritoryID Territory to claim
	 * @return True if territory was claimed
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Territory")
	bool ClaimTerritory(FName TerritoryID);

	/**
	 * @brief Challenge another crew for their territory
	 * @param TerritoryID Territory to challenge for
	 * @return True if challenge was initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Territory")
	bool ChallengeForTerritory(FName TerritoryID);

	/**
	 * @brief Get total bonus value from territories
	 * @param BonusType Type of bonus (e.g., "XP", "Cash")
	 * @return Total bonus percentage
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Territory")
	float GetTerritoryBonus(FName BonusType) const;

	// ========================================================================
	// Search & Discovery
	// ========================================================================

	/**
	 * @brief Search for crews by name
	 * @param Query Search query
	 * @param MaxResults Maximum results
	 * @return Array of matching crews
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Search")
	TArray<FMGCrewInfo> SearchCrews(const FString& Query, int32 MaxResults = 20);

	/** @brief Get recommended crews for the player */
	UFUNCTION(BlueprintPure, Category = "Crew|Search")
	TArray<FMGCrewInfo> GetRecommendedCrews() const;

	// ========================================================================
	// Leaderboards
	// ========================================================================

	/**
	 * @brief Get top crews by ranking
	 * @param Count Number of crews to return
	 * @return Array of top crews
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Leaderboard")
	TArray<FMGCrewInfo> GetTopCrews(int32 Count = 100) const;

	/** @brief Get current crew's global rank */
	UFUNCTION(BlueprintPure, Category = "Crew|Leaderboard")
	int32 GetCrewRank() const { return CurrentCrew.GlobalRank; }

	// ========================================================================
	// Crew Battles Integration
	// ========================================================================

	/**
	 * @brief Get crew info for battles subsystem
	 * @return Current crew ID, name, tag, and rating for battles
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Battles")
	void GetCrewBattleInfo(FName& OutCrewID, FString& OutCrewName, FString& OutCrewTag, int32& OutRating) const;

	/**
	 * @brief Update crew battle rating after a battle
	 * @param NewRating New ELO rating
	 * @param bWon Whether the crew won the battle
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Battles")
	void UpdateBattleRating(int32 NewRating, bool bWon);

	/**
	 * @brief Check if crew can participate in battles
	 * @return True if crew meets requirements
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Battles")
	bool CanParticipateInBattles() const;

	/**
	 * @brief Get eligible roster for crew battles
	 * @param MaxPlayers Maximum roster size
	 * @return Array of eligible member IDs
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Battles")
	TArray<FName> GetBattleRoster(int32 MaxPlayers = 4) const;

	/**
	 * @brief Record a crew battle result
	 * @param bWon Whether the battle was won
	 * @param OpponentName Name of opponent crew
	 * @param RatingChange Change in battle rating
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Battles")
	void RecordBattleResult(bool bWon, const FString& OpponentName, int32 RatingChange);

	// ========================================================================
	// Reputation Sharing System
	// ========================================================================

	/**
	 * @brief Contribute reputation to the crew pool
	 *
	 * When members earn reputation, a portion goes to the crew rep pool.
	 * This pool provides bonuses to all members based on crew level and perks.
	 *
	 * @param Amount Reputation amount earned by member
	 * @param Category Category of reputation (Racing, Technical, etc.)
	 * @return Amount actually contributed to crew pool
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Reputation")
	int32 ContributeReputation(int32 Amount, FName Category = NAME_None);

	/**
	 * @brief Get the current crew reputation pool total
	 * @return Total accumulated crew reputation
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Reputation")
	int64 GetCrewReputationPool() const { return CrewReputationPool; }

	/**
	 * @brief Get reputation bonus multiplier for members
	 *
	 * Based on crew level, perks, and territory bonuses, members
	 * receive a multiplier on reputation earned while in the crew.
	 *
	 * @return Reputation multiplier (1.0 = no bonus, 1.15 = 15% bonus)
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Reputation")
	float GetReputationBonusMultiplier() const;

	/**
	 * @brief Get crew reputation tier based on pool size
	 *
	 * Crew reputation tiers unlock additional benefits:
	 * - Bronze (0-10k): Base bonuses
	 * - Silver (10k-50k): +5% rep bonus, priority matchmaking
	 * - Gold (50k-200k): +10% rep bonus, exclusive events access
	 * - Platinum (200k-1M): +15% rep bonus, custom crew challenges
	 * - Diamond (1M+): +20% rep bonus, legendary status
	 *
	 * @return Crew reputation tier name
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Reputation")
	FName GetCrewReputationTier() const;

	/**
	 * @brief Get contribution percentage for crew rep pool
	 *
	 * The percentage of earned rep that goes to the crew pool.
	 * Higher ranks contribute more. Default is 10%.
	 *
	 * @param Rank Member's rank
	 * @return Percentage of rep contributed (0.0 - 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Reputation")
	static float GetReputationContributionRate(EMGCrewRank Rank);

	/**
	 * @brief Distribute weekly reputation rewards to members
	 *
	 * Called at weekly reset. Distributes bonus reputation to members
	 * based on their contribution to the crew that week.
	 *
	 * @return Total reputation distributed
	 */
	UFUNCTION(BlueprintCallable, Category = "Crew|Reputation")
	int64 DistributeWeeklyReputationRewards();

	/**
	 * @brief Get member's share of weekly rep distribution
	 * @param MemberID Member to check
	 * @return Estimated reputation bonus for next distribution
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Reputation")
	int32 GetMemberWeeklyReputationShare(FName MemberID) const;

	/**
	 * @brief Get reputation leaderboard within the crew
	 * @param Count Max members to return
	 * @return Array of members sorted by weekly contribution
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Reputation")
	TArray<FMGCrewMember> GetReputationLeaderboard(int32 Count = 10) const;

	/**
	 * @brief Check if crew has reached a reputation milestone
	 * @param MilestoneAmount The milestone threshold to check
	 * @return True if crew has reached or exceeded the milestone
	 */
	UFUNCTION(BlueprintPure, Category = "Crew|Reputation")
	bool HasReachedReputationMilestone(int64 MilestoneAmount) const;

	// ========================================================================
	// Delegates
	// ========================================================================

	/** Fired when the local player joins a crew */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewJoined OnCrewJoined;

	/** Fired when the local player leaves a crew */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewLeft OnCrewLeft;

	/** Fired when crew data is updated */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewUpdated OnCrewUpdated;

	/** Fired when a member joins the crew */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewMemberJoined OnCrewMemberJoined;

	/** Fired when a member leaves the crew */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewMemberLeft OnCrewMemberLeft;

	/** Fired when a member's rank changes */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewMemberRankChanged OnCrewMemberRankChanged;

	/** Fired when the crew levels up */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewLevelUp OnCrewLevelUp;

	/** Fired when a crew challenge is completed */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewChallengeCompleted OnCrewChallengeCompleted;

	/** Fired when an invite is received */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewInviteReceived OnCrewInviteReceived;

	/** Fired when an application is received */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewApplicationReceived OnCrewApplicationReceived;

	/** Fired when activity is added to the feed */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewActivityAdded OnCrewActivityAdded;

	/** Fired when a chat message is received */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewChatMessageReceived OnCrewChatMessageReceived;

	/** Fired when territory ownership changes */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewTerritoryChanged OnCrewTerritoryChanged;

	/** Fired when a vehicle is added to garage */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewGarageVehicleAdded OnCrewGarageVehicleAdded;

	/** Fired when a vehicle is borrowed from garage */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewGarageVehicleBorrowed OnCrewGarageVehicleBorrowed;

	/** Fired when reputation is contributed to crew pool */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewReputationContributed OnCrewReputationContributed;

	/** Fired when crew reputation tier changes */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewReputationTierChanged OnCrewReputationTierChanged;

	/** Fired when weekly reputation is distributed */
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewReputationDistributed OnCrewReputationDistributed;

protected:
	/** Initialize crew perks data */
	void InitializePerks();

	/** Initialize territory data */
	void InitializeTerritories();

	/** Check and process crew level up */
	void CheckLevelUp();

	/** Check challenge completion */
	void CheckChallenges();

	/** Calculate XP required for a level */
	int64 CalculateExperienceForLevel(int32 Level) const;

	/** Save crew data to persistent storage */
	void SaveCrewData();

	/** Load crew data from persistent storage */
	void LoadCrewData();

	/** Tick function for periodic updates */
	void OnCrewTick();

	/** Clean up expired invites and applications */
	void CleanupExpiredData();

	/** Add a system message to chat */
	void AddSystemChatMessage(const FString& Content);

	/** Current crew information */
	UPROPERTY()
	FMGCrewInfo CurrentCrew;

	/** Crew member list */
	UPROPERTY()
	TArray<FMGCrewMember> Members;

	/** Pending invites for the local player */
	UPROPERTY()
	TArray<FMGCrewInvite> PendingInvites;

	/** Pending applications to the crew */
	UPROPERTY()
	TArray<FMGCrewApplication> PendingApplications;

	/** Active crew challenges */
	UPROPERTY()
	TArray<FMGCrewChallenge> ActiveChallenges;

	/** Activity feed */
	UPROPERTY()
	TArray<FMGCrewActivity> ActivityFeed;

	/** Crew perks */
	UPROPERTY()
	TArray<FMGCrewPerk> CrewPerks;

	/** Chat message history */
	UPROPERTY()
	TArray<FMGCrewChatMessage> ChatHistory;

	/** Shared garage vehicles */
	UPROPERTY()
	TArray<FMGCrewGarageVehicle> GarageVehicles;

	/** Territory data */
	UPROPERTY()
	TMap<FName, FMGCrewTerritory> Territories;

	/** Local player ID */
	UPROPERTY()
	FName LocalPlayerID;

	/** Local player name */
	UPROPERTY()
	FString LocalPlayerName;

	/** Maximum activity feed size */
	UPROPERTY()
	int32 MaxActivityFeedSize = 200;

	/** Maximum chat history size */
	UPROPERTY()
	int32 MaxChatHistorySize = 500;

	/** Base garage capacity */
	UPROPERTY()
	int32 BaseGarageCapacity = 5;

	/** Accumulated crew reputation pool */
	UPROPERTY()
	int64 CrewReputationPool = 0;

	/** Current reputation tier */
	UPROPERTY()
	FName CurrentReputationTier = TEXT("Bronze");

	/** Weekly reputation contributions by member (reset on distribution) */
	UPROPERTY()
	TMap<FName, int32> WeeklyReputationContributions;

	/** Last weekly distribution timestamp */
	UPROPERTY()
	FDateTime LastReputationDistribution;

	/** Timer handle for crew tick */
	FTimerHandle CrewTickHandle;
};

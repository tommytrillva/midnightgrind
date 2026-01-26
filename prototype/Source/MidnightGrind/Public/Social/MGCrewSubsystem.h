// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCrewSubsystem.generated.h"

class UTexture2D;

/**
 * Crew member role
 */
UENUM(BlueprintType)
enum class EMGCrewRole : uint8
{
	/** Regular member */
	Member,
	/** Veteran member */
	Veteran,
	/** Officer - can invite/kick */
	Officer,
	/** Co-Leader - full permissions except disband */
	CoLeader,
	/** Leader - full permissions */
	Leader
};

/**
 * Crew join type
 */
UENUM(BlueprintType)
enum class EMGCrewJoinType : uint8
{
	/** Anyone can join */
	Open,
	/** Request to join */
	RequestOnly,
	/** Invite only */
	InviteOnly,
	/** Closed - no new members */
	Closed
};

/**
 * Crew activity type
 */
UENUM(BlueprintType)
enum class EMGCrewActivityType : uint8
{
	/** Member joined */
	MemberJoined,
	/** Member left */
	MemberLeft,
	/** Member promoted */
	MemberPromoted,
	/** Member demoted */
	MemberDemoted,
	/** Crew leveled up */
	CrewLevelUp,
	/** Challenge completed */
	ChallengeCompleted,
	/** Crew vs Crew win */
	CrewBattleWin,
	/** New record set */
	NewRecord,
	/** Livery shared */
	LiveryShared
};

/**
 * Crew member data
 */
USTRUCT(BlueprintType)
struct FMGCrewMember
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	/** Avatar */
	UPROPERTY(BlueprintReadOnly)
	UTexture2D* Avatar = nullptr;

	/** Role */
	UPROPERTY(BlueprintReadOnly)
	EMGCrewRole Role = EMGCrewRole::Member;

	/** Join date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime JoinDate;

	/** XP contributed to crew */
	UPROPERTY(BlueprintReadOnly)
	int64 XPContributed = 0;

	/** Races completed for crew */
	UPROPERTY(BlueprintReadOnly)
	int32 RacesCompleted = 0;

	/** Wins for crew */
	UPROPERTY(BlueprintReadOnly)
	int32 Wins = 0;

	/** Is online */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOnline = false;

	/** Last online */
	UPROPERTY(BlueprintReadOnly)
	FDateTime LastOnline;

	/** Rep earned this week */
	UPROPERTY(BlueprintReadOnly)
	int32 WeeklyRep = 0;
};

/**
 * Crew activity entry
 */
USTRUCT(BlueprintType)
struct FMGCrewActivity
{
	GENERATED_BODY()

	/** Activity type */
	UPROPERTY(BlueprintReadOnly)
	EMGCrewActivityType Type = EMGCrewActivityType::MemberJoined;

	/** Player involved */
	UPROPERTY(BlueprintReadOnly)
	FText PlayerName;

	/** Activity description */
	UPROPERTY(BlueprintReadOnly)
	FText Description;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Associated value */
	UPROPERTY(BlueprintReadOnly)
	int32 Value = 0;
};

/**
 * Crew challenge
 */
USTRUCT(BlueprintType)
struct FMGCrewChallenge
{
	GENERATED_BODY()

	/** Challenge ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Target value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetValue = 0;

	/** Current progress */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentValue = 0;

	/** XP reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XPReward = 0;

	/** Token reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TokenReward = 0;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;

	/** Expires at */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;
};

/**
 * Crew perk
 */
USTRUCT(BlueprintType)
struct FMGCrewPerk
{
	GENERATED_BODY()

	/** Perk ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PerkID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Required crew level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/** Perk value (percentage or flat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;

	/** Is percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPercentage = true;

	/** Is unlocked */
	UPROPERTY(BlueprintReadOnly)
	bool bIsUnlocked = false;
};

/**
 * Shared livery
 */
USTRUCT(BlueprintType)
struct FMGSharedLivery
{
	GENERATED_BODY()

	/** Livery ID */
	UPROPERTY(BlueprintReadOnly)
	FString LiveryID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Creator name */
	UPROPERTY(BlueprintReadOnly)
	FText CreatorName;

	/** Vehicle ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/** Preview image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PreviewImage = nullptr;

	/** Livery data (serialized) */
	UPROPERTY()
	TArray<uint8> LiveryData;

	/** Upload date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime UploadDate;

	/** Downloads */
	UPROPERTY(BlueprintReadOnly)
	int32 Downloads = 0;

	/** Likes */
	UPROPERTY(BlueprintReadOnly)
	int32 Likes = 0;

	/** Is featured */
	UPROPERTY(BlueprintReadOnly)
	bool bIsFeatured = false;
};

/**
 * Crew vs Crew battle
 */
USTRUCT(BlueprintType)
struct FMGCrewBattle
{
	GENERATED_BODY()

	/** Battle ID */
	UPROPERTY(BlueprintReadOnly)
	FString BattleID;

	/** Opponent crew ID */
	UPROPERTY(BlueprintReadOnly)
	FString OpponentCrewID;

	/** Opponent crew name */
	UPROPERTY(BlueprintReadOnly)
	FText OpponentCrewName;

	/** Opponent crew emblem */
	UPROPERTY(BlueprintReadOnly)
	UTexture2D* OpponentEmblem = nullptr;

	/** Our score */
	UPROPERTY(BlueprintReadOnly)
	int32 OurScore = 0;

	/** Their score */
	UPROPERTY(BlueprintReadOnly)
	int32 TheirScore = 0;

	/** Start time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime StartTime;

	/** End time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime EndTime;

	/** Is active */
	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false;

	/** Did we win */
	UPROPERTY(BlueprintReadOnly)
	bool bDidWin = false;
};

/**
 * Crew data
 */
USTRUCT(BlueprintType)
struct FMGCrewData
{
	GENERATED_BODY()

	/** Crew ID */
	UPROPERTY(BlueprintReadOnly)
	FString CrewID;

	/** Crew name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	/** Crew tag (3-4 chars) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Tag;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Motto */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Motto;

	/** Emblem */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Emblem = nullptr;

	/** Primary color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	/** Secondary color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::Black;

	/** Join type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewJoinType JoinType = EMGCrewJoinType::RequestOnly;

	/** Minimum level to join */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinimumLevel = 1;

	/** Crew level */
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/** Crew XP */
	UPROPERTY(BlueprintReadOnly)
	int64 XP = 0;

	/** XP needed for next level */
	UPROPERTY(BlueprintReadOnly)
	int64 XPToNextLevel = 1000;

	/** Creation date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime CreationDate;

	/** Member count */
	UPROPERTY(BlueprintReadOnly)
	int32 MemberCount = 0;

	/** Max members */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxMembers = 50;

	/** Total wins */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalWins = 0;

	/** Crew tokens */
	UPROPERTY(BlueprintReadOnly)
	int32 CrewTokens = 0;

	/** Weekly rank */
	UPROPERTY(BlueprintReadOnly)
	int32 WeeklyRank = 0;

	/** All-time rank */
	UPROPERTY(BlueprintReadOnly)
	int32 AllTimeRank = 0;

	/** Members */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGCrewMember> Members;

	/** Activity feed */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGCrewActivity> ActivityFeed;

	/** Active challenges */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGCrewChallenge> ActiveChallenges;

	/** Unlocked perks */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGCrewPerk> Perks;

	/** Shared liveries */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGSharedLivery> SharedLiveries;

	/** Active battle */
	UPROPERTY(BlueprintReadOnly)
	FMGCrewBattle ActiveBattle;
};

/**
 * Crew invite
 */
USTRUCT(BlueprintType)
struct FMGCrewInvite
{
	GENERATED_BODY()

	/** Invite ID */
	UPROPERTY(BlueprintReadOnly)
	FString InviteID;

	/** Crew ID */
	UPROPERTY(BlueprintReadOnly)
	FString CrewID;

	/** Crew name */
	UPROPERTY(BlueprintReadOnly)
	FText CrewName;

	/** Crew emblem */
	UPROPERTY(BlueprintReadOnly)
	UTexture2D* CrewEmblem = nullptr;

	/** Inviter name */
	UPROPERTY(BlueprintReadOnly)
	FText InviterName;

	/** Invite date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime InviteDate;

	/** Expires at */
	UPROPERTY(BlueprintReadOnly)
	FDateTime ExpiresAt;
};

/**
 * Crew join request
 */
USTRUCT(BlueprintType)
struct FMGCrewJoinRequest
{
	GENERATED_BODY()

	/** Request ID */
	UPROPERTY(BlueprintReadOnly)
	FString RequestID;

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Player name */
	UPROPERTY(BlueprintReadOnly)
	FText PlayerName;

	/** Player level */
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerLevel = 1;

	/** Player avatar */
	UPROPERTY(BlueprintReadOnly)
	UTexture2D* PlayerAvatar = nullptr;

	/** Request message */
	UPROPERTY(BlueprintReadOnly)
	FText Message;

	/** Request date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime RequestDate;
};

/**
 * Crew search result
 */
USTRUCT(BlueprintType)
struct FMGCrewSearchResult
{
	GENERATED_BODY()

	/** Crew ID */
	UPROPERTY(BlueprintReadOnly)
	FString CrewID;

	/** Crew name */
	UPROPERTY(BlueprintReadOnly)
	FText Name;

	/** Crew tag */
	UPROPERTY(BlueprintReadOnly)
	FString Tag;

	/** Emblem */
	UPROPERTY(BlueprintReadOnly)
	UTexture2D* Emblem = nullptr;

	/** Level */
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/** Member count */
	UPROPERTY(BlueprintReadOnly)
	int32 MemberCount = 0;

	/** Max members */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxMembers = 50;

	/** Join type */
	UPROPERTY(BlueprintReadOnly)
	EMGCrewJoinType JoinType = EMGCrewJoinType::RequestOnly;

	/** Weekly rank */
	UPROPERTY(BlueprintReadOnly)
	int32 WeeklyRank = 0;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewJoined, const FMGCrewData&, Crew);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCrewLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewUpdated, const FMGCrewData&, Crew);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewMemberJoined, const FMGCrewMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewMemberLeft, const FMGCrewMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewInviteReceived, const FMGCrewInvite&, Invite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewBattleStarted, const FMGCrewBattle&, Battle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewBattleEnded, const FMGCrewBattle&, Battle, bool, bWon);

/**
 * Crew Subsystem
 * Manages crew/club functionality
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCrewSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewJoined OnCrewJoined;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewLeft OnCrewLeft;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewUpdated OnCrewUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewMemberJoined OnCrewMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewMemberLeft OnCrewMemberLeft;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewLevelUp OnCrewLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewInviteReceived OnCrewInviteReceived;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewBattleStarted OnCrewBattleStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCrewBattleEnded OnCrewBattleEnded;

	// ==========================================
	// CREW STATE
	// ==========================================

	/** Is in a crew */
	UFUNCTION(BlueprintPure, Category = "Crew")
	bool IsInCrew() const { return bIsInCrew; }

	/** Get current crew */
	UFUNCTION(BlueprintPure, Category = "Crew")
	FMGCrewData GetCurrentCrew() const { return CurrentCrew; }

	/** Get my role in crew */
	UFUNCTION(BlueprintPure, Category = "Crew")
	EMGCrewRole GetMyRole() const { return MyRole; }

	/** Can perform action based on role */
	UFUNCTION(BlueprintPure, Category = "Crew")
	bool CanPerformAction(EMGCrewRole RequiredRole) const;

	// ==========================================
	// CREW MANAGEMENT
	// ==========================================

	/** Create new crew */
	UFUNCTION(BlueprintCallable, Category = "Crew")
	bool CreateCrew(FText Name, FString Tag, FText Description);

	/** Leave current crew */
	UFUNCTION(BlueprintCallable, Category = "Crew")
	bool LeaveCrew();

	/** Disband crew (leader only) */
	UFUNCTION(BlueprintCallable, Category = "Crew")
	bool DisbandCrew();

	/** Update crew settings */
	UFUNCTION(BlueprintCallable, Category = "Crew")
	bool UpdateCrewSettings(const FMGCrewData& NewSettings);

	// ==========================================
	// MEMBERSHIP
	// ==========================================

	/** Request to join crew */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool RequestToJoinCrew(const FString& CrewID, FText Message);

	/** Accept join request (officer+) */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool AcceptJoinRequest(const FString& RequestID);

	/** Decline join request (officer+) */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool DeclineJoinRequest(const FString& RequestID);

	/** Invite player to crew (officer+) */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool InvitePlayer(const FString& PlayerID);

	/** Accept crew invite */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool AcceptCrewInvite(const FString& InviteID);

	/** Decline crew invite */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool DeclineCrewInvite(const FString& InviteID);

	/** Kick member (officer+) */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool KickMember(const FString& PlayerID);

	/** Promote member (co-leader+) */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool PromoteMember(const FString& PlayerID);

	/** Demote member (co-leader+) */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool DemoteMember(const FString& PlayerID);

	/** Transfer leadership (leader only) */
	UFUNCTION(BlueprintCallable, Category = "Membership")
	bool TransferLeadership(const FString& PlayerID);

	/** Get pending join requests */
	UFUNCTION(BlueprintPure, Category = "Membership")
	TArray<FMGCrewJoinRequest> GetPendingJoinRequests() const { return PendingJoinRequests; }

	/** Get pending invites */
	UFUNCTION(BlueprintPure, Category = "Membership")
	TArray<FMGCrewInvite> GetPendingInvites() const { return PendingInvites; }

	// ==========================================
	// SEARCH
	// ==========================================

	/** Search for crews */
	UFUNCTION(BlueprintCallable, Category = "Search")
	TArray<FMGCrewSearchResult> SearchCrews(const FString& Query, int32 MaxResults = 20);

	/** Get recommended crews */
	UFUNCTION(BlueprintCallable, Category = "Search")
	TArray<FMGCrewSearchResult> GetRecommendedCrews();

	/** Get top crews */
	UFUNCTION(BlueprintCallable, Category = "Search")
	TArray<FMGCrewSearchResult> GetTopCrews(int32 Count = 10);

	// ==========================================
	// LIVERIES
	// ==========================================

	/** Share livery with crew */
	UFUNCTION(BlueprintCallable, Category = "Liveries")
	bool ShareLivery(const FMGSharedLivery& Livery);

	/** Download crew livery */
	UFUNCTION(BlueprintCallable, Category = "Liveries")
	bool DownloadLivery(const FString& LiveryID);

	/** Like livery */
	UFUNCTION(BlueprintCallable, Category = "Liveries")
	void LikeLivery(const FString& LiveryID);

	/** Delete shared livery (owner or officer+) */
	UFUNCTION(BlueprintCallable, Category = "Liveries")
	bool DeleteSharedLivery(const FString& LiveryID);

	// ==========================================
	// CREW VS CREW
	// ==========================================

	/** Start crew battle */
	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool StartCrewBattle(const FString& OpponentCrewID);

	/** Get current battle */
	UFUNCTION(BlueprintPure, Category = "Battle")
	FMGCrewBattle GetCurrentBattle() const;

	/** Report battle score */
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ReportBattleScore(int32 ScoreEarned);

	// ==========================================
	// CONTRIBUTION
	// ==========================================

	/** Contribute XP to crew */
	UFUNCTION(BlueprintCallable, Category = "Contribution")
	void ContributeXP(int32 Amount);

	/** Contribute to challenge */
	UFUNCTION(BlueprintCallable, Category = "Contribution")
	void ContributeToChallenge(FName ChallengeID, int32 Amount);

	// ==========================================
	// PERKS
	// ==========================================

	/** Get crew perk value */
	UFUNCTION(BlueprintPure, Category = "Perks")
	float GetPerkValue(FName PerkID) const;

	/** Is perk unlocked */
	UFUNCTION(BlueprintPure, Category = "Perks")
	bool IsPerkUnlocked(FName PerkID) const;

	/** Get all available perks */
	UFUNCTION(BlueprintPure, Category = "Perks")
	TArray<FMGCrewPerk> GetAllPerks() const { return AllPerks; }

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get role display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetRoleDisplayName(EMGCrewRole Role);

	/** Get XP needed for crew level */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static int64 GetXPForCrewLevel(int32 Level);

	/** Validate crew tag */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static bool IsValidCrewTag(const FString& Tag);

protected:
	/** Is in a crew */
	bool bIsInCrew = false;

	/** Current crew data */
	UPROPERTY()
	FMGCrewData CurrentCrew;

	/** My role */
	EMGCrewRole MyRole = EMGCrewRole::Member;

	/** Pending join requests */
	UPROPERTY()
	TArray<FMGCrewJoinRequest> PendingJoinRequests;

	/** Pending invites */
	UPROPERTY()
	TArray<FMGCrewInvite> PendingInvites;

	/** All available perks */
	UPROPERTY()
	TArray<FMGCrewPerk> AllPerks;

	/** Initialize perks */
	void InitializePerks();

	/** Update perk status */
	void UpdatePerkStatus();

	/** Check crew level up */
	void CheckCrewLevelUp();

	/** Add activity to feed */
	void AddActivityToFeed(EMGCrewActivityType Type, FText PlayerName, FText Description, int32 Value = 0);

	/** Create mock data */
	void CreateMockData();
};

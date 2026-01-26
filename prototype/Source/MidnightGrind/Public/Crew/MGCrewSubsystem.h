// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCrewSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGCrewRank : uint8
{
	Member,
	Veteran,
	Officer,
	Lieutenant,
	Captain,
	Leader
};

UENUM(BlueprintType)
enum class EMGCrewPrivacy : uint8
{
	Open,
	ApprovalRequired,
	InviteOnly,
	Closed
};

UENUM(BlueprintType)
enum class EMGCrewActivityType : uint8
{
	RaceCompleted,
	RaceWon,
	MemberJoined,
	MemberLeft,
	Promotion,
	CrewChallenge,
	LevelUp,
	Achievement,
	Donation
};

USTRUCT(BlueprintType)
struct FMGCrewMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewRank Rank = EMGCrewRank::Member;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime JoinedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastActiveDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ContributionPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacesForCrew = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WinsForCrew = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrencyDonated = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnline = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentActivity;
};

USTRUCT(BlueprintType)
struct FMGCrewInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CrewName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CrewTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Motto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewPrivacy Privacy = EMGCrewPrivacy::ApprovalRequired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ExperiencePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ExperienceToNextLevel = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxMembers = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentMembers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Treasury = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GlobalRank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RegionalRank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Region;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Emblem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMGCrewActivity
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewActivityType Type = EMGCrewActivityType::RaceCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 PointsEarned = 0;
};

USTRUCT(BlueprintType)
struct FMGCrewChallenge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TargetValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ExperienceReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrencyReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct FMGCrewPerk
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PerkID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PerkName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusValue = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGCrewInvite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InviteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CrewName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName InviterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InviterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SentDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresDate;
};

USTRUCT(BlueprintType)
struct FMGCrewApplication
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ApplicationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ApplicantID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ApplicantName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ApplicantLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AppliedDate;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewJoined, const FMGCrewInfo&, Crew);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCrewLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewMemberJoined, const FMGCrewMember&, Member);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewMemberLeft, FName, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewLevelUp, int32, NewLevel, const TArray<FMGCrewPerk>&, UnlockedPerks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewChallengeCompleted, const FMGCrewChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewInviteReceived, const FMGCrewInvite&, Invite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewApplicationReceived, const FMGCrewApplication&, Application);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewActivityAdded, const FMGCrewActivity&, Activity);

UCLASS()
class MIDNIGHTGRIND_API UMGCrewSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Crew Management
	UFUNCTION(BlueprintCallable, Category = "Crew|Management")
	bool CreateCrew(const FString& Name, const FString& Tag, const FString& Description);

	UFUNCTION(BlueprintCallable, Category = "Crew|Management")
	bool DisbandCrew();

	UFUNCTION(BlueprintCallable, Category = "Crew|Management")
	bool LeaveCrew();

	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	bool IsInCrew() const { return CurrentCrew.CrewID.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	FMGCrewInfo GetCurrentCrew() const { return CurrentCrew; }

	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	bool IsCrewLeader() const;

	UFUNCTION(BlueprintPure, Category = "Crew|Management")
	bool IsCrewOfficer() const;

	// Member Management
	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	TArray<FMGCrewMember> GetMembers() const { return Members; }

	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	TArray<FMGCrewMember> GetOnlineMembers() const;

	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	FMGCrewMember GetMember(FName PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Crew|Members")
	FMGCrewMember GetLocalMember() const;

	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	bool KickMember(FName PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	bool PromoteMember(FName PlayerID, EMGCrewRank NewRank);

	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	bool DemoteMember(FName PlayerID, EMGCrewRank NewRank);

	UFUNCTION(BlueprintCallable, Category = "Crew|Members")
	bool TransferLeadership(FName NewLeaderID);

	// Invites & Applications
	UFUNCTION(BlueprintCallable, Category = "Crew|Invites")
	bool SendInvite(FName PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Crew|Invites")
	bool AcceptInvite(FGuid InviteID);

	UFUNCTION(BlueprintCallable, Category = "Crew|Invites")
	bool DeclineInvite(FGuid InviteID);

	UFUNCTION(BlueprintPure, Category = "Crew|Invites")
	TArray<FMGCrewInvite> GetPendingInvites() const { return PendingInvites; }

	UFUNCTION(BlueprintCallable, Category = "Crew|Applications")
	bool ApplyToCrew(FGuid CrewID, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Crew|Applications")
	bool AcceptApplication(FGuid ApplicationID);

	UFUNCTION(BlueprintCallable, Category = "Crew|Applications")
	bool RejectApplication(FGuid ApplicationID);

	UFUNCTION(BlueprintPure, Category = "Crew|Applications")
	TArray<FMGCrewApplication> GetPendingApplications() const { return PendingApplications; }

	// Crew Settings
	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool UpdateCrewInfo(const FString& Description, const FString& Motto);

	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool SetCrewPrivacy(EMGCrewPrivacy Privacy);

	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool SetCrewColors(FLinearColor Primary, FLinearColor Secondary);

	UFUNCTION(BlueprintCallable, Category = "Crew|Settings")
	bool SetCrewEmblem(TSoftObjectPtr<UTexture2D> Emblem);

	// Progression
	UFUNCTION(BlueprintCallable, Category = "Crew|Progression")
	void AddCrewExperience(int64 Amount, const FString& Source);

	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	float GetCrewLevelProgress() const;

	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	TArray<FMGCrewPerk> GetCrewPerks() const { return CrewPerks; }

	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	TArray<FMGCrewPerk> GetUnlockedPerks() const;

	UFUNCTION(BlueprintPure, Category = "Crew|Progression")
	bool HasPerk(FName PerkID) const;

	// Treasury
	UFUNCTION(BlueprintCallable, Category = "Crew|Treasury")
	bool DonateToTreasury(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Crew|Treasury")
	bool WithdrawFromTreasury(int64 Amount);

	UFUNCTION(BlueprintPure, Category = "Crew|Treasury")
	int64 GetTreasuryBalance() const { return CurrentCrew.Treasury; }

	// Challenges
	UFUNCTION(BlueprintPure, Category = "Crew|Challenges")
	TArray<FMGCrewChallenge> GetActiveChallenges() const { return ActiveChallenges; }

	UFUNCTION(BlueprintCallable, Category = "Crew|Challenges")
	void ContributeToChallenge(FName ChallengeID, int64 Amount);

	// Activity Feed
	UFUNCTION(BlueprintPure, Category = "Crew|Activity")
	TArray<FMGCrewActivity> GetActivityFeed(int32 MaxEntries = 50) const;

	UFUNCTION(BlueprintCallable, Category = "Crew|Activity")
	void RecordActivity(EMGCrewActivityType Type, const FText& Description, int64 Points = 0);

	// Search
	UFUNCTION(BlueprintCallable, Category = "Crew|Search")
	TArray<FMGCrewInfo> SearchCrews(const FString& Query, int32 MaxResults = 20);

	UFUNCTION(BlueprintPure, Category = "Crew|Search")
	TArray<FMGCrewInfo> GetRecommendedCrews() const;

	// Leaderboards
	UFUNCTION(BlueprintPure, Category = "Crew|Leaderboard")
	TArray<FMGCrewInfo> GetTopCrews(int32 Count = 100) const;

	UFUNCTION(BlueprintPure, Category = "Crew|Leaderboard")
	int32 GetCrewRank() const { return CurrentCrew.GlobalRank; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewJoined OnCrewJoined;

	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewLeft OnCrewLeft;

	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewMemberJoined OnCrewMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewMemberLeft OnCrewMemberLeft;

	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewLevelUp OnCrewLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewChallengeCompleted OnCrewChallengeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewInviteReceived OnCrewInviteReceived;

	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewApplicationReceived OnCrewApplicationReceived;

	UPROPERTY(BlueprintAssignable, Category = "Crew|Events")
	FOnCrewActivityAdded OnCrewActivityAdded;

protected:
	void InitializePerks();
	void CheckLevelUp();
	void CheckChallenges();
	int64 CalculateExperienceForLevel(int32 Level) const;
	void SaveCrewData();
	void LoadCrewData();

	UPROPERTY()
	FMGCrewInfo CurrentCrew;

	UPROPERTY()
	TArray<FMGCrewMember> Members;

	UPROPERTY()
	TArray<FMGCrewInvite> PendingInvites;

	UPROPERTY()
	TArray<FMGCrewApplication> PendingApplications;

	UPROPERTY()
	TArray<FMGCrewChallenge> ActiveChallenges;

	UPROPERTY()
	TArray<FMGCrewActivity> ActivityFeed;

	UPROPERTY()
	TArray<FMGCrewPerk> CrewPerks;

	UPROPERTY()
	FName LocalPlayerID;

	UPROPERTY()
	int32 MaxActivityFeedSize = 200;
};

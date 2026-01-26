// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBountySubsystem.generated.h"

/**
 * Bounty type
 */
UENUM(BlueprintType)
enum class EMGBountyType : uint8
{
	None				UMETA(DisplayName = "None"),
	RaceWin				UMETA(DisplayName = "Race Win"),
	Takedown			UMETA(DisplayName = "Takedown"),
	PolicePursuit		UMETA(DisplayName = "Police Pursuit"),
	StreetLegend		UMETA(DisplayName = "Street Legend"),
	BossChallenge		UMETA(DisplayName = "Boss Challenge"),
	RivalRevenge		UMETA(DisplayName = "Rival Revenge"),
	CommunityTarget		UMETA(DisplayName = "Community Target"),
	Weekly				UMETA(DisplayName = "Weekly Bounty"),
	Special				UMETA(DisplayName = "Special Event")
};

/**
 * Bounty status
 */
UENUM(BlueprintType)
enum class EMGBountyStatus : uint8
{
	Available			UMETA(DisplayName = "Available"),
	Accepted			UMETA(DisplayName = "Accepted"),
	InProgress			UMETA(DisplayName = "In Progress"),
	Completed			UMETA(DisplayName = "Completed"),
	Failed				UMETA(DisplayName = "Failed"),
	Expired				UMETA(DisplayName = "Expired"),
	Abandoned			UMETA(DisplayName = "Abandoned")
};

/**
 * Bounty difficulty
 */
UENUM(BlueprintType)
enum class EMGBountyDifficulty : uint8
{
	Easy				UMETA(DisplayName = "Easy"),
	Medium				UMETA(DisplayName = "Medium"),
	Hard				UMETA(DisplayName = "Hard"),
	Expert				UMETA(DisplayName = "Expert"),
	Legendary			UMETA(DisplayName = "Legendary"),
	Impossible			UMETA(DisplayName = "Impossible")
};

/**
 * Bounty target type
 */
UENUM(BlueprintType)
enum class EMGBountyTargetType : uint8
{
	AIDriver			UMETA(DisplayName = "AI Driver"),
	Player				UMETA(DisplayName = "Player"),
	BossRacer			UMETA(DisplayName = "Boss Racer"),
	LegendaryRacer		UMETA(DisplayName = "Legendary Racer"),
	PoliceChief			UMETA(DisplayName = "Police Chief"),
	CrewLeader			UMETA(DisplayName = "Crew Leader"),
	Champion			UMETA(DisplayName = "Champion")
};

/**
 * Bounty definition
 */
USTRUCT(BlueprintType)
struct FMGBountyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BountyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FlavorText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBountyType Type = EMGBountyType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBountyDifficulty Difficulty = EMGBountyDifficulty::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBountyTargetType TargetType = EMGBountyTargetType::AIDriver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TargetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardCurrency = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardExperience = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardReputation = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpecialRewardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLimit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExpirationHours = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredPlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RepeatCooldownHours = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RequiredObjectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> OptionalObjectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusMultiplierPerOptional = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> TargetPortraitAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> BountyPosterAsset;
};

/**
 * Active bounty
 */
USTRUCT(BlueprintType)
struct FMGActiveBounty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BountyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBountyStatus Status = EMGBountyStatus::Available;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AcceptedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpirationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, bool> ObjectiveProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletedObjectives = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalObjectives = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OptionalObjectivesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentRewardMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AttemptCount = 0;
};

/**
 * Bounty completion result
 */
USTRUCT(BlueprintType)
struct FMGBountyCompletionResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BountyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExperienceEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpecialRewardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CompletionTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ObjectivesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OptionalObjectivesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPerfectCompletion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFirstCompletion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Bounty target
 */
USTRUCT(BlueprintType)
struct FMGBountyTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Biography;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBountyTargetType Type = EMGBountyTargetType::AIDriver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillLevel = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HomeDistrict;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AssociatedBountyIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountyValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesCaptured = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> PortraitAsset;
};

/**
 * Player bounty board
 */
USTRUCT(BlueprintType)
struct FMGPlayerBountyBoard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AvailableBountyIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ActiveBountyIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxActiveBounties = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRefreshTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RefreshesRemaining = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RefreshCost = 1000;
};

/**
 * Bounty player stats
 */
USTRUCT(BlueprintType)
struct FMGBountyPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountiesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountiesFailed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountiesAbandoned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectCompletions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalCurrencyEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalExperienceEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalReputationEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestBountyTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGBountyType, int32> CompletionsByType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGBountyDifficulty, int32> CompletionsByDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> TargetCaptureCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MostCapturedTargetId;
};

/**
 * Community bounty
 */
USTRUCT(BlueprintType)
struct FMGCommunityBounty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CommunityBountyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BountyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalContributors = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCompletions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetCompletions = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusRewardTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> MilestoneThresholds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBountyAccepted, const FString&, PlayerId, const FString&, BountyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBountyCompleted, const FString&, PlayerId, const FMGBountyCompletionResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBountyFailed, const FString&, PlayerId, const FString&, BountyId, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBountyAbandoned, const FString&, PlayerId, const FString&, BountyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBountyObjectiveCompleted, const FString&, PlayerId, const FString&, BountyId, const FString&, ObjectiveId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBountyProgress, const FString&, PlayerId, const FString&, BountyId, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBountyBoardRefreshed, const FString&, PlayerId, int32, NewBountyCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBountyExpiring, const FString&, PlayerId, const FString&, BountyId, float, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCommunityBountyProgress, const FString&, CommunityBountyId, int32, CurrentCompletions, int32, TargetCompletions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunityBountyMilestone, const FString&, CommunityBountyId, int32, MilestoneReached);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBountyStreakUpdated, const FString&, PlayerId, int32, NewStreak, bool, bNewRecord);

/**
 * Bounty Subsystem
 * Manages bounty hunts, targets, and rewards
 */
UCLASS()
class MIDNIGHTGRIND_API UMGBountySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyAccepted OnBountyAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyCompleted OnBountyCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyFailed OnBountyFailed;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyAbandoned OnBountyAbandoned;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyObjectiveCompleted OnBountyObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyProgress OnBountyProgress;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyBoardRefreshed OnBountyBoardRefreshed;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyExpiring OnBountyExpiring;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnCommunityBountyProgress OnCommunityBountyProgress;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnCommunityBountyMilestone OnCommunityBountyMilestone;

	UPROPERTY(BlueprintAssignable, Category = "Bounty|Events")
	FOnBountyStreakUpdated OnBountyStreakUpdated;

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Bounty|Registration")
	void RegisterBounty(const FMGBountyDefinition& Bounty);

	UFUNCTION(BlueprintCallable, Category = "Bounty|Registration")
	void UnregisterBounty(const FString& BountyId);

	UFUNCTION(BlueprintCallable, Category = "Bounty|Registration")
	void RegisterTarget(const FMGBountyTarget& Target);

	// Bounty Actions
	UFUNCTION(BlueprintCallable, Category = "Bounty|Actions")
	bool AcceptBounty(const FString& PlayerId, const FString& BountyId);

	UFUNCTION(BlueprintCallable, Category = "Bounty|Actions")
	bool AbandonBounty(const FString& PlayerId, const FString& BountyId);

	UFUNCTION(BlueprintCallable, Category = "Bounty|Actions")
	FMGBountyCompletionResult CompleteBounty(const FString& PlayerId, const FString& BountyId);

	UFUNCTION(BlueprintCallable, Category = "Bounty|Actions")
	void FailBounty(const FString& PlayerId, const FString& BountyId, const FString& Reason);

	// Objective Tracking
	UFUNCTION(BlueprintCallable, Category = "Bounty|Objectives")
	void CompleteObjective(const FString& PlayerId, const FString& BountyId, const FString& ObjectiveId);

	UFUNCTION(BlueprintPure, Category = "Bounty|Objectives")
	bool IsObjectiveComplete(const FString& PlayerId, const FString& BountyId, const FString& ObjectiveId) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Objectives")
	float GetBountyProgress(const FString& PlayerId, const FString& BountyId) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Objectives")
	bool AreAllRequiredObjectivesComplete(const FString& PlayerId, const FString& BountyId) const;

	// Bounty Board
	UFUNCTION(BlueprintPure, Category = "Bounty|Board")
	FMGPlayerBountyBoard GetBountyBoard(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Bounty|Board")
	bool RefreshBountyBoard(const FString& PlayerId, bool bForce = false);

	UFUNCTION(BlueprintPure, Category = "Bounty|Board")
	TArray<FMGBountyDefinition> GetAvailableBounties(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Board")
	TArray<FMGActiveBounty> GetActiveBounties(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Board")
	bool CanAcceptBounty(const FString& PlayerId, const FString& BountyId) const;

	// Queries
	UFUNCTION(BlueprintPure, Category = "Bounty|Query")
	FMGBountyDefinition GetBountyDefinition(const FString& BountyId) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Query")
	FMGActiveBounty GetActiveBounty(const FString& PlayerId, const FString& BountyId) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Query")
	FMGBountyTarget GetTarget(const FString& TargetId) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Query")
	TArray<FMGBountyDefinition> GetBountiesForTarget(const FString& TargetId) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Query")
	TArray<FMGBountyDefinition> GetBountiesByType(EMGBountyType Type) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Query")
	TArray<FMGBountyDefinition> GetBountiesByDifficulty(EMGBountyDifficulty Difficulty) const;

	// Community Bounties
	UFUNCTION(BlueprintCallable, Category = "Bounty|Community")
	void RegisterCommunityBounty(const FMGCommunityBounty& CommunityBounty);

	UFUNCTION(BlueprintPure, Category = "Bounty|Community")
	TArray<FMGCommunityBounty> GetActiveCommunityBounties() const;

	UFUNCTION(BlueprintCallable, Category = "Bounty|Community")
	void ContributeToCommunityBounty(const FString& CommunityBountyId);

	UFUNCTION(BlueprintPure, Category = "Bounty|Community")
	FMGCommunityBounty GetCommunityBounty(const FString& CommunityBountyId) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Bounty|Stats")
	FMGBountyPlayerStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Bounty|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Rewards
	UFUNCTION(BlueprintPure, Category = "Bounty|Rewards")
	int32 CalculateRewardCurrency(const FString& BountyId, float Multiplier) const;

	UFUNCTION(BlueprintPure, Category = "Bounty|Rewards")
	int32 CalculateRewardExperience(const FString& BountyId, float Multiplier) const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "Bounty|Update")
	void UpdateBountySystem(float DeltaTime);

	// Persistence
	UFUNCTION(BlueprintCallable, Category = "Bounty|Persistence")
	void SaveBountyData();

	UFUNCTION(BlueprintCallable, Category = "Bounty|Persistence")
	void LoadBountyData();

protected:
	void TickBounties(float DeltaTime);
	void CheckExpirations();
	void UpdateBountyBoards();
	void GenerateBountiesForPlayer(const FString& PlayerId);
	TArray<FString> SelectRandomBounties(const FString& PlayerId, int32 Count);
	void UpdatePlayerStats(const FString& PlayerId, const FMGBountyCompletionResult& Result);
	void UpdateStreak(const FString& PlayerId, bool bSuccess);
	FString GenerateInstanceId() const;
	float GetDifficultyMultiplier(EMGBountyDifficulty Difficulty) const;

private:
	UPROPERTY()
	TMap<FString, FMGBountyDefinition> BountyDefinitions;

	UPROPERTY()
	TMap<FString, FMGBountyTarget> Targets;

	UPROPERTY()
	TMap<FString, TMap<FString, FMGActiveBounty>> PlayerActiveBounties;

	UPROPERTY()
	TMap<FString, FMGPlayerBountyBoard> PlayerBountyBoards;

	UPROPERTY()
	TMap<FString, FMGBountyPlayerStats> PlayerStats;

	UPROPERTY()
	TMap<FString, TSet<FString>> CompletedBounties;

	UPROPERTY()
	TMap<FString, TMap<FString, FDateTime>> BountyCooldowns;

	UPROPERTY()
	TMap<FString, FMGCommunityBounty> CommunityBounties;

	UPROPERTY()
	int32 InstanceCounter = 0;

	FTimerHandle BountyTickTimer;
};

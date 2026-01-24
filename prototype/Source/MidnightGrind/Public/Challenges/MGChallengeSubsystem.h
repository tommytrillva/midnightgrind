// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGChallengeSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGChallengeType : uint8
{
	Daily,
	Weekly,
	Monthly,
	Seasonal,
	Community,
	Special,
	Achievement,
	Tutorial
};

UENUM(BlueprintType)
enum class EMGChallengeCategory : uint8
{
	Racing,
	Drifting,
	Collection,
	Exploration,
	Social,
	Customization,
	Mastery,
	Speed,
	Endurance,
	Skill
};

UENUM(BlueprintType)
enum class EMGChallengeState : uint8
{
	Locked,
	Available,
	Active,
	Completed,
	Failed,
	Expired,
	Claimed
};

UENUM(BlueprintType)
enum class EMGChallengeDifficulty : uint8
{
	Easy,
	Medium,
	Hard,
	Expert,
	Legendary
};

USTRUCT(BlueprintType)
struct FMGChallengeReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RewardName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PremiumCurrencyAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExperienceAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonXPAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> RewardIcon;
};

USTRUCT(BlueprintType)
struct FMGChallengeObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StatToTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusRewardAmount = 0;
};

USTRUCT(BlueprintType)
struct FMGChallenge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeType ChallengeType = EMGChallengeType::Daily;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeCategory Category = EMGChallengeCategory::Racing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeState State = EMGChallengeState::Available;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeDifficulty Difficulty = EMGChallengeDifficulty::Easy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeReward> Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCompletions = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> PrerequisiteChallenges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ChallengeIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicle;
};

USTRUCT(BlueprintType)
struct FMGChallengeSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallenge> Challenges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGChallengeReward CompletionReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredCompletions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCompletions = 0;
};

USTRUCT(BlueprintType)
struct FMGCommunityChallenge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGChallenge Challenge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CommunityTarget = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CommunityProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalContributors = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 PlayerContribution = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeReward> MilestoneRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int64> MilestoneThresholds;
};

USTRUCT(BlueprintType)
struct FMGChallengeProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> ObjectiveProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRewardsClaimed = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChallengeProgressUpdated, FName, ChallengeID, float, ProgressPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeCompleted, const FMGChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeRewardsClaimed, const TArray<FMGChallengeReward>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewChallengesAvailable, EMGChallengeType, ChallengeType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeExpired, FName, ChallengeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunityProgressUpdated, FName, ChallengeID, float, CommunityPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeSetCompleted, const FMGChallengeSet&, Set);

UCLASS()
class MIDNIGHTGRIND_API UMGChallengeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Challenge Retrieval
	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetChallengesByType(EMGChallengeType Type) const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetChallengesByCategory(EMGChallengeCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetActiveChallenges() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetCompletedChallenges() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	TArray<FMGChallenge> GetAvailableChallenges() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	FMGChallenge GetChallenge(FName ChallengeID) const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Query")
	bool IsChallengeAvailable(FName ChallengeID) const;

	// Daily/Weekly/Monthly
	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	TArray<FMGChallenge> GetDailyChallenges() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	TArray<FMGChallenge> GetWeeklyChallenges() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	TArray<FMGChallenge> GetMonthlyChallenges() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	FTimespan GetTimeUntilDailyReset() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Timed")
	FTimespan GetTimeUntilWeeklyReset() const;

	// Progress Tracking
	UFUNCTION(BlueprintCallable, Category = "Challenges|Progress")
	void UpdateChallengeProgress(FName ChallengeID, FName ObjectiveID, int32 Progress);

	UFUNCTION(BlueprintCallable, Category = "Challenges|Progress")
	void AddChallengeProgress(FName ChallengeID, FName ObjectiveID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Challenges|Progress")
	void TrackStat(FName StatName, int32 Value);

	UFUNCTION(BlueprintPure, Category = "Challenges|Progress")
	float GetChallengeProgressPercent(FName ChallengeID) const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Progress")
	FMGChallengeProgress GetChallengeProgress(FName ChallengeID) const;

	// Activation
	UFUNCTION(BlueprintCallable, Category = "Challenges|Activation")
	bool ActivateChallenge(FName ChallengeID);

	UFUNCTION(BlueprintCallable, Category = "Challenges|Activation")
	void DeactivateChallenge(FName ChallengeID);

	UFUNCTION(BlueprintPure, Category = "Challenges|Activation")
	int32 GetActiveChallengeCount() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Activation")
	int32 GetMaxActiveChallenges() const { return MaxActiveChallenges; }

	// Rewards
	UFUNCTION(BlueprintCallable, Category = "Challenges|Rewards")
	bool ClaimChallengeRewards(FName ChallengeID);

	UFUNCTION(BlueprintCallable, Category = "Challenges|Rewards")
	void ClaimAllAvailableRewards();

	UFUNCTION(BlueprintPure, Category = "Challenges|Rewards")
	TArray<FMGChallenge> GetUnclaimedCompletedChallenges() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Rewards")
	bool HasUnclaimedRewards() const;

	// Challenge Sets
	UFUNCTION(BlueprintPure, Category = "Challenges|Sets")
	TArray<FMGChallengeSet> GetChallengeSets() const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Sets")
	FMGChallengeSet GetChallengeSet(FName SetID) const;

	UFUNCTION(BlueprintPure, Category = "Challenges|Sets")
	float GetChallengeSetProgress(FName SetID) const;

	// Community Challenges
	UFUNCTION(BlueprintPure, Category = "Challenges|Community")
	TArray<FMGCommunityChallenge> GetCommunityChallenges() const;

	UFUNCTION(BlueprintCallable, Category = "Challenges|Community")
	void ContributeToCommunityChallenge(FName ChallengeID, int64 Amount);

	UFUNCTION(BlueprintPure, Category = "Challenges|Community")
	float GetCommunityProgressPercent(FName ChallengeID) const;

	// Management
	UFUNCTION(BlueprintCallable, Category = "Challenges|Management")
	void RefreshChallenges();

	UFUNCTION(BlueprintCallable, Category = "Challenges|Management")
	void RerollDailyChallenge(FName ChallengeID);

	UFUNCTION(BlueprintPure, Category = "Challenges|Management")
	int32 GetRerollsRemaining() const { return DailyRerollsRemaining; }

	UFUNCTION(BlueprintCallable, Category = "Challenges|Management")
	void SaveChallengeProgress();

	UFUNCTION(BlueprintCallable, Category = "Challenges|Management")
	void LoadChallengeProgress();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeProgressUpdated OnChallengeProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeCompleted OnChallengeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeRewardsClaimed OnChallengeRewardsClaimed;

	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnNewChallengesAvailable OnNewChallengesAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeExpired OnChallengeExpired;

	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnCommunityProgressUpdated OnCommunityProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Challenges|Events")
	FOnChallengeSetCompleted OnChallengeSetCompleted;

protected:
	void OnChallengeTick();
	void CheckForExpiredChallenges();
	void CheckForCompletedChallenges();
	void GenerateDailyChallenges();
	void GenerateWeeklyChallenges();
	void InitializeDefaultChallenges();
	void GrantReward(const FMGChallengeReward& Reward);
	void UpdateChallengeState(FMGChallenge& Challenge);
	bool ArePrerequisitesMet(const FMGChallenge& Challenge) const;

	UPROPERTY()
	TMap<FName, FMGChallenge> AllChallenges;

	UPROPERTY()
	TMap<FName, FMGChallengeProgress> ChallengeProgress;

	UPROPERTY()
	TArray<FName> ActiveChallengeIDs;

	UPROPERTY()
	TMap<FName, FMGChallengeSet> ChallengeSets;

	UPROPERTY()
	TArray<FMGCommunityChallenge> CommunityChallenges;

	UPROPERTY()
	TMap<FName, int32> TrackedStats;

	UPROPERTY()
	int32 MaxActiveChallenges = 10;

	UPROPERTY()
	int32 DailyRerollsRemaining = 3;

	UPROPERTY()
	FDateTime LastDailyReset;

	UPROPERTY()
	FDateTime LastWeeklyReset;

	FTimerHandle ChallengeTickHandle;
};

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGReferralSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGReferralStatus : uint8
{
	Pending,
	Registered,
	FirstLogin,
	TutorialComplete,
	ReachedLevel5,
	ReachedLevel10,
	FirstWin,
	PurchasedPremium,
	Claimed,
	Expired
};

UENUM(BlueprintType)
enum class EMGReferralRewardType : uint8
{
	Currency,
	Vehicle,
	Part,
	Cosmetic,
	XPBoost,
	CurrencyBoost,
	PremiumTime,
	UniqueTitle,
	UniqueLivery,
	ExclusiveDecal
};

UENUM(BlueprintType)
enum class EMGReferralTier : uint8
{
	Bronze,      // 1-2 referrals
	Silver,      // 3-5 referrals
	Gold,        // 6-10 referrals
	Platinum,    // 11-25 referrals
	Diamond,     // 26-50 referrals
	Ambassador   // 50+ referrals
};

USTRUCT(BlueprintType)
struct FMGReferralReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReferralRewardType Type = EMGReferralRewardType::Currency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForReferrer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForReferred = true;
};

USTRUCT(BlueprintType)
struct FMGReferralMilestone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MilestoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReferralReward> Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsClaimed = false;
};

USTRUCT(BlueprintType)
struct FMGReferralCode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Code;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OwnerPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiryTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxUses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentUses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReferralReward> BonusRewards;
};

USTRUCT(BlueprintType)
struct FMGReferredPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReferralStatus Status = EMGReferralStatus::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReferredTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastProgressTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasWon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPurchased = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReferralReward> PendingRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReferralReward> ClaimedRewards;
};

USTRUCT(BlueprintType)
struct FMGReferralStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletedReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PendingReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExpiredReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReferralTier CurrentTier = EMGReferralTier::Bronze;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReferralsToNextTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCurrencyEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalItemsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstReferralTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastReferralTime;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralCodeGenerated, const FMGReferralCode&, Code);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralCodeApplied, const FMGReferralCode&, Code);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReferralCodeInvalid, const FString&, Code, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewReferral, const FMGReferredPlayer&, ReferredPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReferralProgressUpdated, const FString&, PlayerID, EMGReferralStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralComplete, const FMGReferredPlayer&, ReferredPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralRewardAvailable, const FMGReferralReward&, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralRewardClaimed, const FMGReferralReward&, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMilestoneReached, const FMGReferralMilestone&, Milestone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTierChanged, EMGReferralTier, OldTier, EMGReferralTier, NewTier);

UCLASS()
class MIDNIGHTGRIND_API UMGReferralSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Referral Codes
	UFUNCTION(BlueprintCallable, Category = "Referral|Code")
	FMGReferralCode GenerateReferralCode();

	UFUNCTION(BlueprintCallable, Category = "Referral|Code")
	FMGReferralCode GenerateCustomCode(const FString& CustomCode, int32 MaxUses = 0);

	UFUNCTION(BlueprintPure, Category = "Referral|Code")
	FMGReferralCode GetMyReferralCode() const { return MyReferralCode; }

	UFUNCTION(BlueprintPure, Category = "Referral|Code")
	FString GetReferralLink() const;

	UFUNCTION(BlueprintCallable, Category = "Referral|Code")
	void CopyReferralCodeToClipboard();

	UFUNCTION(BlueprintCallable, Category = "Referral|Code")
	void ShareReferralCode();

	// Applying Codes
	UFUNCTION(BlueprintCallable, Category = "Referral|Apply")
	bool ApplyReferralCode(const FString& Code);

	UFUNCTION(BlueprintPure, Category = "Referral|Apply")
	bool HasAppliedReferralCode() const { return bHasAppliedCode; }

	UFUNCTION(BlueprintPure, Category = "Referral|Apply")
	FMGReferralCode GetAppliedReferralCode() const { return AppliedCode; }

	UFUNCTION(BlueprintCallable, Category = "Referral|Apply")
	bool ValidateCode(const FString& Code);

	// Referral Tracking
	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	TArray<FMGReferredPlayer> GetReferredPlayers() const { return ReferredPlayers; }

	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	TArray<FMGReferredPlayer> GetReferredPlayersByStatus(EMGReferralStatus Status) const;

	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	FMGReferralStats GetReferralStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	EMGReferralTier GetCurrentTier() const { return Stats.CurrentTier; }

	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	float GetTierProgress() const;

	// Rewards
	UFUNCTION(BlueprintPure, Category = "Referral|Rewards")
	TArray<FMGReferralReward> GetPendingRewards() const { return PendingRewards; }

	UFUNCTION(BlueprintPure, Category = "Referral|Rewards")
	int32 GetPendingRewardCount() const { return PendingRewards.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Referral|Rewards")
	bool ClaimReward(FName RewardID);

	UFUNCTION(BlueprintCallable, Category = "Referral|Rewards")
	void ClaimAllRewards();

	// Milestones
	UFUNCTION(BlueprintPure, Category = "Referral|Milestones")
	TArray<FMGReferralMilestone> GetMilestones() const { return Milestones; }

	UFUNCTION(BlueprintPure, Category = "Referral|Milestones")
	FMGReferralMilestone GetNextMilestone() const;

	UFUNCTION(BlueprintPure, Category = "Referral|Milestones")
	int32 GetReferralsToNextMilestone() const;

	UFUNCTION(BlueprintCallable, Category = "Referral|Milestones")
	bool ClaimMilestoneReward(int32 MilestoneIndex);

	// Progress Reporting (for referred player)
	UFUNCTION(BlueprintCallable, Category = "Referral|Progress")
	void ReportTutorialComplete();

	UFUNCTION(BlueprintCallable, Category = "Referral|Progress")
	void ReportLevelReached(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "Referral|Progress")
	void ReportFirstWin();

	UFUNCTION(BlueprintCallable, Category = "Referral|Progress")
	void ReportPurchase();

	// Social
	UFUNCTION(BlueprintCallable, Category = "Referral|Social")
	void InviteFriend(const FString& FriendID);

	UFUNCTION(BlueprintCallable, Category = "Referral|Social")
	void InviteFriendByEmail(const FString& Email);

	UFUNCTION(BlueprintPure, Category = "Referral|Social")
	TArray<FString> GetInvitedFriends() const { return InvitedFriends; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralCodeGenerated OnReferralCodeGenerated;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralCodeApplied OnReferralCodeApplied;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralCodeInvalid OnReferralCodeInvalid;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnNewReferral OnNewReferral;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralProgressUpdated OnReferralProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralComplete OnReferralComplete;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralRewardAvailable OnReferralRewardAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralRewardClaimed OnReferralRewardClaimed;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnMilestoneReached OnMilestoneReached;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnTierChanged OnTierChanged;

protected:
	void InitializeMilestones();
	void UpdateStats();
	void UpdateTier();
	void CheckMilestones();
	void GrantReward(const FMGReferralReward& Reward);
	FString GenerateCodeString() const;
	EMGReferralTier CalculateTier(int32 ReferralCount) const;

	UPROPERTY()
	FMGReferralCode MyReferralCode;

	UPROPERTY()
	FMGReferralCode AppliedCode;

	UPROPERTY()
	bool bHasAppliedCode = false;

	UPROPERTY()
	TArray<FMGReferredPlayer> ReferredPlayers;

	UPROPERTY()
	FMGReferralStats Stats;

	UPROPERTY()
	TArray<FMGReferralReward> PendingRewards;

	UPROPERTY()
	TArray<FMGReferralMilestone> Milestones;

	UPROPERTY()
	TArray<FString> InvitedFriends;

	FString BaseReferralURL = TEXT("https://midnightgrind.com/invite/");
};

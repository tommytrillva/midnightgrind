// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSeasonPassSubsystem.generated.h"

/**
 * Fair Season Pass System
 * - FREE track with meaningful rewards (not just scraps)
 * - Premium track is cosmetic bonuses only
 * - XP earned through ALL activities (not just specific challenges)
 * - Reasonable progression - completable with normal play
 * - NO FOMO - seasons are long, catch-up mechanics exist
 * - Previous season items become earnable later
 */

UENUM(BlueprintType)
enum class EMGSeasonRewardType : uint8
{
	GrindCash,
	NeonCredits,
	Livery,
	Decal,
	Wheels,
	Neon,
	Horn,
	Trail,
	Emote,
	Avatar,
	Banner,
	Title,
	XPBoost,
	CurrencyBoost
};

USTRUCT(BlueprintType)
struct FMGSeasonReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSeasonRewardType Type = EMGSeasonRewardType::GrindCash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrencyAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostDurationHours = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct FMGSeasonTier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TierNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 XPRequired = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGSeasonReward FreeReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGSeasonReward PremiumReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMilestone = false;
};

USTRUCT(BlueprintType)
struct FMGSeasonChallenge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 XPReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWeekly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;
};

USTRUCT(BlueprintType)
struct FMGSeasonInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonTheme;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTier = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGSeasonTier> Tiers;
};

USTRUCT(BlueprintType)
struct FMGSeasonProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentXP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalXPEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPremiumPass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ClaimedFreeTiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ClaimedPremiumTiers;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnSeasonXPGained, int64, Amount, int64, TotalXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSeasonTierReached, int32, Tier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnSeasonRewardClaimed, int32, Tier, bool, bPremium);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnChallengeCompleted, const FMGSeasonChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPremiumPassPurchased);

UCLASS()
class MIDNIGHTGRIND_API UMGSeasonPassSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Season Info
	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	FMGSeasonInfo GetCurrentSeason() const { return CurrentSeason; }

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	FMGSeasonProgress GetProgress() const { return Progress; }

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	FTimespan GetTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	float GetSeasonProgressPercent() const;

	// XP & Progression
	UFUNCTION(BlueprintCallable, Category = "SeasonPass")
	void AddXP(int64 Amount, const FString& Source);

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	int64 GetXPForTier(int32 Tier) const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	int64 GetXPToNextTier() const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	float GetTierProgressPercent() const;

	// Rewards
	UFUNCTION(BlueprintCallable, Category = "SeasonPass")
	bool ClaimTierReward(int32 Tier, bool bPremium);

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	bool CanClaimReward(int32 Tier, bool bPremium) const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	TArray<int32> GetUnclaimedTiers(bool bPremium) const;

	UFUNCTION(BlueprintCallable, Category = "SeasonPass")
	void ClaimAllAvailableRewards();

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	FMGSeasonTier GetTierInfo(int32 Tier) const;

	// Premium Pass
	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	bool HasPremiumPass() const { return Progress.bHasPremiumPass; }

	UFUNCTION(BlueprintCallable, Category = "SeasonPass")
	bool PurchasePremiumPass();

	UFUNCTION(BlueprintPure, Category = "SeasonPass")
	int64 GetPremiumPassPrice() const { return 1000; } // 1000 Neon Credits

	// Challenges
	UFUNCTION(BlueprintPure, Category = "SeasonPass|Challenges")
	TArray<FMGSeasonChallenge> GetDailyChallenges() const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass|Challenges")
	TArray<FMGSeasonChallenge> GetWeeklyChallenges() const;

	UFUNCTION(BlueprintCallable, Category = "SeasonPass|Challenges")
	void UpdateChallengeProgress(FName ChallengeID, int32 Progress);

	// Catch-up Mechanics (anti-FOMO)
	UFUNCTION(BlueprintPure, Category = "SeasonPass|CatchUp")
	int64 GetCatchUpXPBonus() const;

	UFUNCTION(BlueprintPure, Category = "SeasonPass|CatchUp")
	bool IsEligibleForCatchUp() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnSeasonXPGained OnSeasonXPGained;

	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnSeasonTierReached OnSeasonTierReached;

	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnSeasonRewardClaimed OnSeasonRewardClaimed;

	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnChallengeCompleted OnChallengeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "SeasonPass|Events")
	FMGOnPremiumPassPurchased OnPremiumPassPurchased;

protected:
	void LoadSeasonData();
	void SaveProgress();
	void InitializeCurrentSeason();
	void GenerateChallenges();
	void CheckTierUp();
	void GrantReward(const FMGSeasonReward& Reward);

private:
	UPROPERTY()
	FMGSeasonInfo CurrentSeason;

	UPROPERTY()
	FMGSeasonProgress Progress;

	UPROPERTY()
	TArray<FMGSeasonChallenge> DailyChallenges;

	UPROPERTY()
	TArray<FMGSeasonChallenge> WeeklyChallenges;
};

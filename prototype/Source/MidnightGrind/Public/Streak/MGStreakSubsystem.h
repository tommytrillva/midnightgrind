// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStreakSubsystem.generated.h"

/**
 * Streak type
 */
UENUM(BlueprintType)
enum class EMGStreakType : uint8
{
	None				UMETA(DisplayName = "None"),
	Win					UMETA(DisplayName = "Win Streak"),
	Podium				UMETA(DisplayName = "Podium Streak"),
	Perfect				UMETA(DisplayName = "Perfect Streak"),
	NearMiss			UMETA(DisplayName = "Near Miss Streak"),
	Drift				UMETA(DisplayName = "Drift Streak"),
	Nitro				UMETA(DisplayName = "Nitro Streak"),
	Takedown			UMETA(DisplayName = "Takedown Streak"),
	Clean				UMETA(DisplayName = "Clean Race Streak"),
	Daily				UMETA(DisplayName = "Daily Streak"),
	Weekly				UMETA(DisplayName = "Weekly Streak"),
	EventWin			UMETA(DisplayName = "Event Win Streak"),
	Ranked				UMETA(DisplayName = "Ranked Win Streak"),
	Undefeated			UMETA(DisplayName = "Undefeated Streak")
};

/**
 * Streak tier
 */
UENUM(BlueprintType)
enum class EMGStreakTier : uint8
{
	None				UMETA(DisplayName = "None"),
	Bronze				UMETA(DisplayName = "Bronze"),
	Silver				UMETA(DisplayName = "Silver"),
	Gold				UMETA(DisplayName = "Gold"),
	Platinum			UMETA(DisplayName = "Platinum"),
	Diamond				UMETA(DisplayName = "Diamond"),
	Champion			UMETA(DisplayName = "Champion"),
	Legend				UMETA(DisplayName = "Legend")
};

/**
 * Streak status
 */
UENUM(BlueprintType)
enum class EMGStreakStatus : uint8
{
	Inactive			UMETA(DisplayName = "Inactive"),
	Active				UMETA(DisplayName = "Active"),
	AtRisk				UMETA(DisplayName = "At Risk"),
	Frozen				UMETA(DisplayName = "Frozen"),
	Broken				UMETA(DisplayName = "Broken"),
	Completed			UMETA(DisplayName = "Completed")
};

/**
 * Combo type
 */
UENUM(BlueprintType)
enum class EMGComboType : uint8
{
	None				UMETA(DisplayName = "None"),
	DriftCombo			UMETA(DisplayName = "Drift Combo"),
	NearMissCombo		UMETA(DisplayName = "Near Miss Combo"),
	SlipstreamCombo		UMETA(DisplayName = "Slipstream Combo"),
	AirtimeCombo		UMETA(DisplayName = "Airtime Combo"),
	TakedownCombo		UMETA(DisplayName = "Takedown Combo"),
	NitroCombo			UMETA(DisplayName = "Nitro Combo"),
	MixedCombo			UMETA(DisplayName = "Mixed Combo"),
	MegaCombo			UMETA(DisplayName = "Mega Combo")
};

/**
 * Active streak
 */
USTRUCT(BlueprintType)
struct FMGActiveStreak
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StreakId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreakType Type = EMGStreakType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreakStatus Status = EMGStreakStatus::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreakTier CurrentTier = EMGStreakTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NextTierThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPointsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastUpdateTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpirationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasExpiration = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FreezeTokensUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxFreezeTokens = 1;
};

/**
 * Streak definition
 */
USTRUCT(BlueprintType)
struct FMGStreakDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreakType Type = EMGStreakType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzeThreshold = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverThreshold = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldThreshold = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumThreshold = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiamondThreshold = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChampionThreshold = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LegendThreshold = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MultiplierPerCount = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMultiplier = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePointsPerCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasDailyReset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExpirationHours = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FreezeTokensAllowed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> IconAsset;
};

/**
 * Active combo
 */
USTRUCT(BlueprintType)
struct FMGActiveCombo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComboId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGComboType Type = EMGComboType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HitCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxComboTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGComboType> ContributingTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDuration = 0.0f;
};

/**
 * Combo result
 */
USTRUCT(BlueprintType)
struct FMGComboResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResultId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGComboType FinalType = EMGComboType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalHits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGComboType> TypesUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Streak tier reward
 */
USTRUCT(BlueprintType)
struct FMGStreakTierReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreakTier Tier = EMGStreakTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExperienceReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnlockableId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RewardDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> RewardAsset;
};

/**
 * Daily streak data
 */
USTRUCT(BlueprintType)
struct FMGDailyStreakData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentDayStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestDayStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastLoginDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StreakStartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayedToday = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompletedDailyChallenge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FreezeTokensAvailable = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDaysPlayed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDateTime> MissedDays;
};

/**
 * Streak player stats
 */
USTRUCT(BlueprintType)
struct FMGStreakPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGStreakType, int32> BestStreaks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGStreakType, int32> TotalStreakCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGStreakTier, int32> TiersAchieved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCombosCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestComboHits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestComboScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalComboPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MegaCombosAchieved = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestComboDuration = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnStreakUpdated, const FString&, PlayerId, EMGStreakType, Type, int32, NewCount, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStreakTierReached, const FString&, PlayerId, EMGStreakType, Type, EMGStreakTier, Tier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStreakBroken, const FString&, PlayerId, EMGStreakType, Type, int32, FinalCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnComboHit, const FString&, PlayerId, EMGComboType, Type, int32, HitCount, int32, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboEnded, const FString&, PlayerId, const FMGComboResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMegaCombo, const FString&, PlayerId, int32, HitCount, int32, TotalScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDailyStreakUpdated, const FString&, PlayerId, int32, DayCount, bool, bNewRecord);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStreakFrozen, const FString&, PlayerId, EMGStreakType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStreakAtRisk, const FString&, PlayerId, EMGStreakType, Type, float, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewStreakRecord, const FString&, PlayerId, EMGStreakType, Type);

/**
 * Streak Subsystem
 * Manages win streaks, combos, daily streaks, and multipliers
 */
UCLASS()
class MIDNIGHTGRIND_API UMGStreakSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnStreakUpdated OnStreakUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnStreakTierReached OnStreakTierReached;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnStreakBroken OnStreakBroken;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnComboHit OnComboHit;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnComboEnded OnComboEnded;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnMegaCombo OnMegaCombo;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnDailyStreakUpdated OnDailyStreakUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnStreakFrozen OnStreakFrozen;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnStreakAtRisk OnStreakAtRisk;

	UPROPERTY(BlueprintAssignable, Category = "Streak|Events")
	FOnNewStreakRecord OnNewStreakRecord;

	// Streak Management
	UFUNCTION(BlueprintCallable, Category = "Streak|Management")
	void RegisterStreakType(const FMGStreakDefinition& Definition);

	UFUNCTION(BlueprintPure, Category = "Streak|Management")
	FMGStreakDefinition GetStreakDefinition(EMGStreakType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Streak|Management")
	void IncrementStreak(const FString& PlayerId, EMGStreakType Type);

	UFUNCTION(BlueprintCallable, Category = "Streak|Management")
	void BreakStreak(const FString& PlayerId, EMGStreakType Type);

	UFUNCTION(BlueprintCallable, Category = "Streak|Management")
	void ResetStreak(const FString& PlayerId, EMGStreakType Type);

	UFUNCTION(BlueprintCallable, Category = "Streak|Management")
	bool FreezeStreak(const FString& PlayerId, EMGStreakType Type);

	UFUNCTION(BlueprintCallable, Category = "Streak|Management")
	void UnfreezeStreak(const FString& PlayerId, EMGStreakType Type);

	// Streak Queries
	UFUNCTION(BlueprintPure, Category = "Streak|Query")
	FMGActiveStreak GetActiveStreak(const FString& PlayerId, EMGStreakType Type) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Query")
	TArray<FMGActiveStreak> GetAllActiveStreaks(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Query")
	int32 GetCurrentStreakCount(const FString& PlayerId, EMGStreakType Type) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Query")
	int32 GetBestStreakCount(const FString& PlayerId, EMGStreakType Type) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Query")
	float GetStreakMultiplier(const FString& PlayerId, EMGStreakType Type) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Query")
	EMGStreakTier GetStreakTier(const FString& PlayerId, EMGStreakType Type) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Query")
	bool IsStreakActive(const FString& PlayerId, EMGStreakType Type) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Query")
	bool IsStreakAtRisk(const FString& PlayerId, EMGStreakType Type) const;

	// Combo System
	UFUNCTION(BlueprintCallable, Category = "Streak|Combo")
	void StartCombo(const FString& PlayerId, EMGComboType Type);

	UFUNCTION(BlueprintCallable, Category = "Streak|Combo")
	void AddComboHit(const FString& PlayerId, EMGComboType Type, int32 Points);

	UFUNCTION(BlueprintCallable, Category = "Streak|Combo")
	void ExtendComboTimer(const FString& PlayerId, float AdditionalTime);

	UFUNCTION(BlueprintCallable, Category = "Streak|Combo")
	FMGComboResult EndCombo(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Streak|Combo")
	void DropCombo(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Streak|Combo")
	FMGActiveCombo GetActiveCombo(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Combo")
	bool HasActiveCombo(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Combo")
	float GetComboTimeRemaining(const FString& PlayerId) const;

	// Daily Streak
	UFUNCTION(BlueprintCallable, Category = "Streak|Daily")
	void RecordDailyLogin(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Streak|Daily")
	void CompleteDailyChallenge(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Streak|Daily")
	FMGDailyStreakData GetDailyStreakData(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Daily")
	int32 GetDailyStreakCount(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Streak|Daily")
	bool UseDailyFreezeToken(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Streak|Daily")
	int32 GetDailyFreezeTokensAvailable(const FString& PlayerId) const;

	// Tier Rewards
	UFUNCTION(BlueprintCallable, Category = "Streak|Rewards")
	void RegisterTierReward(EMGStreakType Type, const FMGStreakTierReward& Reward);

	UFUNCTION(BlueprintPure, Category = "Streak|Rewards")
	FMGStreakTierReward GetTierReward(EMGStreakType Type, EMGStreakTier Tier) const;

	UFUNCTION(BlueprintCallable, Category = "Streak|Rewards")
	TArray<FMGStreakTierReward> ClaimAvailableRewards(const FString& PlayerId, EMGStreakType Type);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Streak|Stats")
	FMGStreakPlayerStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Streak|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Combined Multiplier
	UFUNCTION(BlueprintPure, Category = "Streak|Multiplier")
	float GetCombinedMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Streak|Multiplier")
	float GetComboMultiplier(const FString& PlayerId) const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "Streak|Update")
	void UpdateStreakSystem(float DeltaTime);

	// Persistence
	UFUNCTION(BlueprintCallable, Category = "Streak|Persistence")
	void SaveStreakData();

	UFUNCTION(BlueprintCallable, Category = "Streak|Persistence")
	void LoadStreakData();

protected:
	void TickStreaks(float DeltaTime);
	void UpdateCombos(float DeltaTime);
	void CheckStreakExpirations();
	void CheckDailyReset(const FString& PlayerId);
	EMGStreakTier CalculateTier(EMGStreakType Type, int32 Count) const;
	float CalculateMultiplier(EMGStreakType Type, int32 Count) const;
	int32 GetTierThreshold(EMGStreakType Type, EMGStreakTier Tier) const;
	void AwardTierRewards(const FString& PlayerId, EMGStreakType Type, EMGStreakTier Tier);
	void UpdatePlayerStats(const FString& PlayerId, EMGStreakType Type, int32 Count);
	FString GenerateStreakId() const;
	FString GenerateComboId() const;
	FString GenerateResultId() const;

private:
	UPROPERTY()
	TMap<EMGStreakType, FMGStreakDefinition> StreakDefinitions;

	UPROPERTY()
	TMap<FString, TMap<EMGStreakType, FMGActiveStreak>> PlayerStreaks;

	UPROPERTY()
	TMap<FString, FMGActiveCombo> ActiveCombos;

	UPROPERTY()
	TMap<FString, FMGDailyStreakData> DailyStreaks;

	UPROPERTY()
	TMap<FString, FMGStreakPlayerStats> PlayerStats;

	UPROPERTY()
	TMap<EMGStreakType, TMap<EMGStreakTier, FMGStreakTierReward>> TierRewards;

	UPROPERTY()
	TMap<FString, TSet<FString>> ClaimedRewards;

	UPROPERTY()
	int32 StreakCounter = 0;

	UPROPERTY()
	int32 ComboCounter = 0;

	UPROPERTY()
	int32 ResultCounter = 0;

	FTimerHandle StreakTickTimer;
};

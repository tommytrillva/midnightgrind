// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBalancingSubsystem.generated.h"

/**
 * Dynamic Game Balancing System
 * - Monitors player performance and adjusts difficulty curves
 * - Balances economy values based on player progression
 * - Tracks meta-game statistics for tuning decisions
 * - Provides tools for live-ops balancing adjustments
 */

UENUM(BlueprintType)
enum class EMGBalanceCategory : uint8
{
	Economy,
	Difficulty,
	Progression,
	Rewards,
	AI,
	Physics,
	Matchmaking
};

UENUM(BlueprintType)
enum class EMGDifficultyTier : uint8
{
	Beginner,
	Casual,
	Normal,
	Competitive,
	Expert,
	Master
};

UENUM(BlueprintType)
enum class EMGBalanceFlag : uint8
{
	None,
	UnderTuned,
	Balanced,
	OverTuned,
	NeedsReview
};

USTRUCT(BlueprintType)
struct FMGBalanceParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ParameterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBalanceCategory Category = EMGBalanceCategory::Economy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RemoteOverride = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseRemoteOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBalanceFlag Flag = EMGBalanceFlag::Balanced;
};

USTRUCT(BlueprintType)
struct FMGDifficultyProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDifficultyTier Tier = EMGDifficultyTier::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AIAggressionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AIRubberBandingStrength = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RewardMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProgressionSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OpponentSkillVariance = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableAssists = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRacingLine = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoTransmission = true;
};

USTRUCT(BlueprintType)
struct FMGEconomyBalance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseRaceEarnings = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinBonusPercent = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PodiumBonusPercent = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VehiclePriceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PartPriceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CosmeticPriceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DailyBonusBaseAmount = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InflationRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetRacesToUnlockVehicle = 15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetHoursToEndgame = 40;
};

USTRUCT(BlueprintType)
struct FMGPlayerMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageRacePosition = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PodiumRate = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DNFRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageRaceTimeSeconds = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SessionLengthMinutes = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConsecutiveLosses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConsecutiveWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillRating = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrustrationIndex = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngagementScore = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGBalanceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> ParameterValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerMetrics PlayerMetrics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Notes;
};

USTRUCT(BlueprintType)
struct FMGAdaptiveDifficultyState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetWinRate = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AdjustmentSpeed = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinModifier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxModifier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacesSinceLastAdjustment = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnBalanceParameterChanged, FName, ParameterID, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDifficultyTierChanged, EMGDifficultyTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAdaptiveDifficultyAdjusted, float, NewModifier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnRemoteConfigReceived);

UCLASS()
class MIDNIGHTGRIND_API UMGBalancingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Parameter Management
	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void SetParameter(FName ParameterID, float Value);

	UFUNCTION(BlueprintPure, Category = "Balancing")
	float GetParameter(FName ParameterID) const;

	UFUNCTION(BlueprintPure, Category = "Balancing")
	FMGBalanceParameter GetParameterInfo(FName ParameterID) const;

	UFUNCTION(BlueprintPure, Category = "Balancing")
	TArray<FMGBalanceParameter> GetParametersByCategory(EMGBalanceCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void ResetParameterToDefault(FName ParameterID);

	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void ResetAllParameters();

	// Difficulty Management
	UFUNCTION(BlueprintCallable, Category = "Balancing|Difficulty")
	void SetDifficultyTier(EMGDifficultyTier Tier);

	UFUNCTION(BlueprintPure, Category = "Balancing|Difficulty")
	EMGDifficultyTier GetDifficultyTier() const { return CurrentDifficultyTier; }

	UFUNCTION(BlueprintPure, Category = "Balancing|Difficulty")
	FMGDifficultyProfile GetDifficultyProfile() const { return CurrentDifficultyProfile; }

	UFUNCTION(BlueprintPure, Category = "Balancing|Difficulty")
	FMGDifficultyProfile GetDifficultyProfileForTier(EMGDifficultyTier Tier) const;

	// Adaptive Difficulty
	UFUNCTION(BlueprintCallable, Category = "Balancing|Adaptive")
	void SetAdaptiveDifficultyEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Balancing|Adaptive")
	bool IsAdaptiveDifficultyEnabled() const { return AdaptiveDifficultyState.bEnabled; }

	UFUNCTION(BlueprintPure, Category = "Balancing|Adaptive")
	FMGAdaptiveDifficultyState GetAdaptiveDifficultyState() const { return AdaptiveDifficultyState; }

	UFUNCTION(BlueprintCallable, Category = "Balancing|Adaptive")
	void RecordRaceResult(int32 Position, int32 TotalRacers, float RaceTimeSeconds);

	UFUNCTION(BlueprintCallable, Category = "Balancing|Adaptive")
	float GetCurrentDifficultyModifier() const { return AdaptiveDifficultyState.CurrentModifier; }

	// Economy Balance
	UFUNCTION(BlueprintPure, Category = "Balancing|Economy")
	FMGEconomyBalance GetEconomyBalance() const { return EconomyBalance; }

	UFUNCTION(BlueprintCallable, Category = "Balancing|Economy")
	void SetEconomyBalance(const FMGEconomyBalance& Balance);

	UFUNCTION(BlueprintPure, Category = "Balancing|Economy")
	float CalculateAdjustedPrice(float BasePrice, EMGBalanceCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Balancing|Economy")
	float CalculateAdjustedReward(float BaseReward) const;

	// Player Metrics
	UFUNCTION(BlueprintPure, Category = "Balancing|Metrics")
	FMGPlayerMetrics GetPlayerMetrics() const { return PlayerMetrics; }

	UFUNCTION(BlueprintCallable, Category = "Balancing|Metrics")
	void UpdatePlayerMetrics(const FMGPlayerMetrics& Metrics);

	UFUNCTION(BlueprintPure, Category = "Balancing|Metrics")
	EMGDifficultyTier RecommendDifficultyTier() const;

	UFUNCTION(BlueprintPure, Category = "Balancing|Metrics")
	bool IsPlayerFrustrated() const;

	UFUNCTION(BlueprintPure, Category = "Balancing|Metrics")
	bool IsPlayerBored() const;

	// Remote Config
	UFUNCTION(BlueprintCallable, Category = "Balancing|Remote")
	void FetchRemoteConfig();

	UFUNCTION(BlueprintCallable, Category = "Balancing|Remote")
	void ApplyRemoteOverrides();

	UFUNCTION(BlueprintPure, Category = "Balancing|Remote")
	bool HasPendingRemoteConfig() const { return bHasPendingRemoteConfig; }

	// Snapshots
	UFUNCTION(BlueprintCallable, Category = "Balancing|Debug")
	void TakeSnapshot(const FString& Notes = TEXT(""));

	UFUNCTION(BlueprintPure, Category = "Balancing|Debug")
	TArray<FMGBalanceSnapshot> GetSnapshots() const { return Snapshots; }

	UFUNCTION(BlueprintCallable, Category = "Balancing|Debug")
	void RestoreSnapshot(int32 Index);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Balancing|Events")
	FMGOnBalanceParameterChanged OnBalanceParameterChanged;

	UPROPERTY(BlueprintAssignable, Category = "Balancing|Events")
	FMGOnDifficultyTierChanged OnDifficultyTierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Balancing|Events")
	FMGOnAdaptiveDifficultyAdjusted OnAdaptiveDifficultyAdjusted;

	UPROPERTY(BlueprintAssignable, Category = "Balancing|Events")
	FMGOnRemoteConfigReceived OnRemoteConfigReceived;

protected:
	void InitializeDefaultParameters();
	void InitializeDifficultyProfiles();
	void LoadBalanceData();
	void SaveBalanceData();
	void UpdateAdaptiveDifficulty();
	void CalculateFrustrationIndex();

private:
	UPROPERTY()
	TMap<FName, FMGBalanceParameter> Parameters;

	UPROPERTY()
	TMap<EMGDifficultyTier, FMGDifficultyProfile> DifficultyProfiles;

	UPROPERTY()
	EMGDifficultyTier CurrentDifficultyTier = EMGDifficultyTier::Normal;

	UPROPERTY()
	FMGDifficultyProfile CurrentDifficultyProfile;

	UPROPERTY()
	FMGAdaptiveDifficultyState AdaptiveDifficultyState;

	UPROPERTY()
	FMGEconomyBalance EconomyBalance;

	UPROPERTY()
	FMGPlayerMetrics PlayerMetrics;

	UPROPERTY()
	TArray<FMGBalanceSnapshot> Snapshots;

	TMap<FName, float> PendingRemoteOverrides;
	bool bHasPendingRemoteConfig = false;
};

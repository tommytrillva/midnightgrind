// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCurrencySubsystem.generated.h"

/**
 * Fair Currency System - Designed to be generous and respect players
 * - Primary currency (Grind Cash) earned through ALL gameplay
 * - Premium currency (Neon Credits) earnable through gameplay AND purchasable
 * - NO gameplay advantages for purchase - cosmetics only
 * - Generous daily/weekly bonuses for active players
 */

UENUM(BlueprintType)
enum class EMGCurrencyType : uint8
{
	GrindCash,      // Primary earnable currency - abundant
	NeonCredits,    // Premium currency - earnable AND purchasable
	CrewTokens,     // Earned through crew activities
	SeasonPoints,   // Earned through season pass progression
	LegacyMarks     // Earned through story/career milestones
};

UENUM(BlueprintType)
enum class EMGEarnSource : uint8
{
	RaceFinish,
	RaceWin,
	PodiumFinish,
	DailyLogin,
	WeeklyChallenge,
	SeasonMilestone,
	AchievementUnlock,
	CrewContribution,
	TournamentPlacement,
	StoryMilestone,
	RivalDefeat,
	FirstTimeBonus,
	LevelUp,
	EventParticipation,
	CommunityGoal,
	Gifted
};

USTRUCT(BlueprintType)
struct FMGCurrencyBalance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 GrindCash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 NeonCredits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CrewTokens = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 SeasonPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 LegacyMarks = 0;
};

USTRUCT(BlueprintType)
struct FMGCurrencyTransaction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCurrencyType CurrencyType = EMGCurrencyType::GrindCash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEarnSource Source = EMGEarnSource::RaceFinish;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourceDetails;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BalanceAfter = 0;
};

USTRUCT(BlueprintType)
struct FMGDailyBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConsecutiveDays = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastClaimDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TodayGrindCash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TodayNeonCredits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClaimedToday = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeeklyBonusProgress = 0;
};

USTRUCT(BlueprintType)
struct FMGEarningMultiplier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MultiplierID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCurrencyType AffectedCurrency = EMGCurrencyType::GrindCash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPermanent = false;
};

USTRUCT(BlueprintType)
struct FMGRaceEarnings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BaseEarnings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 PositionBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CleanRaceBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 RivalBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 FirstWinBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MultiplierBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CrewBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalEarnings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEarningMultiplier> AppliedMultipliers;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMGOnCurrencyChanged, EMGCurrencyType, Type, int64, NewBalance, int64, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTransactionCompleted, const FMGCurrencyTransaction&, Transaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDailyBonusClaimed, const FMGDailyBonus&, Bonus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMultiplierActivated, const FMGEarningMultiplier&, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMultiplierExpired, FName, MultiplierID);

UCLASS()
class MIDNIGHTGRIND_API UMGCurrencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Balance Queries
	UFUNCTION(BlueprintPure, Category = "Currency")
	FMGCurrencyBalance GetBalance() const { return CurrentBalance; }

	UFUNCTION(BlueprintPure, Category = "Currency")
	int64 GetCurrencyAmount(EMGCurrencyType Type) const;

	UFUNCTION(BlueprintPure, Category = "Currency")
	bool CanAfford(EMGCurrencyType Type, int64 Amount) const;

	// Currency Transactions
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool EarnCurrency(EMGCurrencyType Type, int64 Amount, EMGEarnSource Source, const FString& Details = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool SpendCurrency(EMGCurrencyType Type, int64 Amount, const FString& PurchaseDetails);

	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool TransferCurrency(EMGCurrencyType Type, int64 Amount, const FString& RecipientID);

	// Race Earnings (generous by design)
	UFUNCTION(BlueprintCallable, Category = "Currency|Racing")
	FMGRaceEarnings CalculateRaceEarnings(int32 Position, int32 TotalRacers, FName TrackID, bool bCleanRace, bool bDefeatedRival, bool bFirstWinOnTrack);

	UFUNCTION(BlueprintCallable, Category = "Currency|Racing")
	void AwardRaceEarnings(const FMGRaceEarnings& Earnings);

	// Daily/Weekly Bonuses (rewarding consistent play, not punishing absence)
	UFUNCTION(BlueprintCallable, Category = "Currency|Bonus")
	FMGDailyBonus GetDailyBonusStatus() const { return DailyBonus; }

	UFUNCTION(BlueprintCallable, Category = "Currency|Bonus")
	bool ClaimDailyBonus();

	UFUNCTION(BlueprintPure, Category = "Currency|Bonus")
	bool CanClaimDailyBonus() const;

	UFUNCTION(BlueprintPure, Category = "Currency|Bonus")
	int64 GetDailyBonusAmount(int32 ConsecutiveDays) const;

	UFUNCTION(BlueprintPure, Category = "Currency|Bonus")
	int64 GetWeeklyBonusNeonCredits() const;

	// Multipliers (earned through gameplay, never purchased)
	UFUNCTION(BlueprintCallable, Category = "Currency|Multiplier")
	void AddMultiplier(const FMGEarningMultiplier& Multiplier);

	UFUNCTION(BlueprintCallable, Category = "Currency|Multiplier")
	void RemoveMultiplier(FName MultiplierID);

	UFUNCTION(BlueprintPure, Category = "Currency|Multiplier")
	TArray<FMGEarningMultiplier> GetActiveMultipliers() const { return ActiveMultipliers; }

	UFUNCTION(BlueprintPure, Category = "Currency|Multiplier")
	float GetTotalMultiplier(EMGCurrencyType Type) const;

	// Transaction History
	UFUNCTION(BlueprintPure, Category = "Currency|History")
	TArray<FMGCurrencyTransaction> GetRecentTransactions(int32 Count = 20) const;

	UFUNCTION(BlueprintPure, Category = "Currency|History")
	int64 GetTotalEarned(EMGCurrencyType Type) const;

	UFUNCTION(BlueprintPure, Category = "Currency|History")
	int64 GetTotalSpent(EMGCurrencyType Type) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnCurrencyChanged OnCurrencyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnTransactionCompleted OnTransactionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnDailyBonusClaimed OnDailyBonusClaimed;

	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnMultiplierActivated OnMultiplierActivated;

	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnMultiplierExpired OnMultiplierExpired;

protected:
	void LoadCurrencyData();
	void SaveCurrencyData();
	void UpdateMultipliers();
	FString GenerateTransactionID();
	void RecordTransaction(EMGCurrencyType Type, int64 Amount, EMGEarnSource Source, const FString& Details);

private:
	UPROPERTY()
	FMGCurrencyBalance CurrentBalance;

	UPROPERTY()
	FMGDailyBonus DailyBonus;

	UPROPERTY()
	TArray<FMGEarningMultiplier> ActiveMultipliers;

	UPROPERTY()
	TArray<FMGCurrencyTransaction> TransactionHistory;

	int64 TotalGrindCashEarned = 0;
	int64 TotalGrindCashSpent = 0;
	int64 TotalNeonCreditsEarned = 0;
	int64 TotalNeonCreditsSpent = 0;

	FTimerHandle MultiplierUpdateTimer;
};

// Copyright Midnight Grind. All Rights Reserved.
// Task #5: Economy Subsystem - Balanced for car culture grind

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGEconomySubsystem.generated.h"

class UMGVehicleModelData;

/**
 * Transaction types for history tracking
 */
UENUM(BlueprintType)
enum class EMGTransactionType : uint8
{
	RaceWinnings,
	RaceEntryFee,
	VehiclePurchase,
	VehicleSale,
	PartPurchase,
	PartSale,
	PaintJob,
	RepairCost,
	PinkSlipWin,
	PinkSlipLoss,
	BountyReward,
	CrewBonus,
	Wager,
	DailyBonus,
	MilestoneReward,
	TournamentPrize,
	MarketplaceSale,
	MarketplacePurchase,
	Trade,
	Other
};

/**
 * A single transaction record
 */
USTRUCT(BlueprintType)
struct FMGTransaction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid TransactionId;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	EMGTransactionType Type = EMGTransactionType::Other;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int64 Amount = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int64 BalanceAfter = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FText Description;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FName RelatedItemID;

	FMGTransaction()
	{
		TransactionId = FGuid::NewGuid();
		Timestamp = FDateTime::Now();
	}
};

/**
 * Shop item for purchase
 */
USTRUCT(BlueprintType)
struct FMGShopItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int64 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int64 SalePrice = 0; // 0 = not on sale

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	bool bIsAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	bool bRequiresUnlock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FName RequiredUnlockID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 RequiredLevel = 0;

	/** Required REP tier (0=None, 1=Rookie, 2=Known, 3=Respected, 4=Feared, 5=Legend) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 RequiredREPTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Get the effective price (sale or regular) */
	int64 GetEffectivePrice() const
	{
		return (SalePrice > 0 && SalePrice < Price) ? SalePrice : Price;
	}

	bool IsOnSale() const
	{
		return SalePrice > 0 && SalePrice < Price;
	}
};

/**
 * Daily/weekly challenge reward
 */
USTRUCT(BlueprintType)
struct FMGChallengeReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FName ChallengeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FText ChallengeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	int64 CreditReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	int64 XPReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	int32 REPReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	float Progress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	float Target = 1.0f;
};

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreditsChanged, int64, NewBalance, int64, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransactionCompleted, const FMGTransaction&, Transaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPurchaseResult, bool, bSuccess, FText, Message);

/**
 * Game Instance Subsystem for the game economy
 * Handles credits, transactions, purchases, and economic events
 *
 * BALANCE PHILOSOPHY:
 * - "Feel the Grind, Not the Frustration" - Progression should be satisfying
 * - Risk = Reward - Higher stakes races pay proportionally more
 * - Car Culture Authenticity - Parts and vehicles priced realistically
 * - The Build Journey Matters - Upgrading a car should feel meaningful
 *
 * See MGEconomyBalanceConfig.cpp for all balance values
 */
UCLASS()
class MIDNIGHTGRIND_API UMGEconomySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// CREDITS/BALANCE
	// ==========================================

	/** Get current credit balance */
	UFUNCTION(BlueprintPure, Category = "Economy")
	int64 GetCredits() const { return Credits; }

	/** Check if player can afford an amount */
	UFUNCTION(BlueprintPure, Category = "Economy")
	bool CanAfford(int64 Amount) const { return Credits >= Amount; }

	/** Add credits (race winnings, sales, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool AddCredits(int64 Amount, EMGTransactionType Type, const FText& Description, FName RelatedItemID = NAME_None);

	/** Spend credits (purchases, fees, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool SpendCredits(int64 Amount, EMGTransactionType Type, const FText& Description, FName RelatedItemID = NAME_None);

	/** Transfer credits (internal, no transaction log) */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void SetCredits(int64 Amount);

	// ==========================================
	// TRANSACTIONS
	// ==========================================

	/** Get transaction history */
	UFUNCTION(BlueprintPure, Category = "Economy|Transactions")
	TArray<FMGTransaction> GetTransactionHistory() const { return TransactionHistory; }

	/** Get recent transactions */
	UFUNCTION(BlueprintCallable, Category = "Economy|Transactions")
	TArray<FMGTransaction> GetRecentTransactions(int32 Count = 10) const;

	/** Get transactions by type */
	UFUNCTION(BlueprintCallable, Category = "Economy|Transactions")
	TArray<FMGTransaction> GetTransactionsByType(EMGTransactionType Type) const;

	/** Get total earned */
	UFUNCTION(BlueprintPure, Category = "Economy|Transactions")
	int64 GetTotalEarned() const { return TotalEarned; }

	/** Get total spent */
	UFUNCTION(BlueprintPure, Category = "Economy|Transactions")
	int64 GetTotalSpent() const { return TotalSpent; }

	// ==========================================
	// RACE ECONOMY
	// ==========================================

	/** Calculate race winnings based on position and race parameters */
	UFUNCTION(BlueprintPure, Category = "Economy|Racing")
	static int64 CalculateRaceWinnings(int32 Position, int32 TotalRacers, int64 BasePrize, float DifficultyMultiplier = 1.0f);

	/** Calculate entry fee for a race */
	UFUNCTION(BlueprintPure, Category = "Economy|Racing")
	static int64 CalculateEntryFee(int64 BasePrize, float FeePercentage = 0.1f);

	/** Pay entry fee for a race */
	UFUNCTION(BlueprintCallable, Category = "Economy|Racing")
	bool PayEntryFee(int64 Fee, FName RaceID);

	/** Award race winnings */
	UFUNCTION(BlueprintCallable, Category = "Economy|Racing")
	void AwardRaceWinnings(int64 Amount, FName RaceID);

	/** Process pink slip win */
	UFUNCTION(BlueprintCallable, Category = "Economy|Racing")
	void ProcessPinkSlipWin(int64 VehicleValue, FName VehicleID);

	/** Process pink slip loss */
	UFUNCTION(BlueprintCallable, Category = "Economy|Racing")
	void ProcessPinkSlipLoss(int64 VehicleValue, FName VehicleID);

	// ==========================================
	// SHOP/PURCHASES
	// ==========================================

	/** Purchase a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Economy|Shop")
	bool PurchaseVehicle(UMGVehicleModelData* VehicleModel, FText& OutMessage);

	/** Purchase a part */
	UFUNCTION(BlueprintCallable, Category = "Economy|Shop")
	bool PurchasePart(FName PartID, int64 Price, FText& OutMessage);

	/** Sell a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Economy|Shop")
	bool SellVehicle(FName VehicleID, int64 SellPrice, FText& OutMessage);

	/** Sell a part */
	UFUNCTION(BlueprintCallable, Category = "Economy|Shop")
	bool SellPart(FName PartID, int64 SellPrice, FText& OutMessage);

	/** Purchase a shop item */
	UFUNCTION(BlueprintCallable, Category = "Economy|Shop")
	bool PurchaseShopItem(const FMGShopItem& Item, FText& OutMessage);

	/** Get sell value for a vehicle (depreciation applied) */
	UFUNCTION(BlueprintPure, Category = "Economy|Shop")
	static int64 CalculateSellValue(int64 PurchasePrice, float Condition, float DepreciationRate = 0.3f);

	// ==========================================
	// WAGERS
	// ==========================================

	/** Place a wager on a race */
	UFUNCTION(BlueprintCallable, Category = "Economy|Wager")
	bool PlaceWager(int64 Amount, FName RaceID);

	/** Resolve a wager (win or lose) */
	UFUNCTION(BlueprintCallable, Category = "Economy|Wager")
	void ResolveWager(bool bWon, float Odds = 2.0f);

	/** Get current active wager */
	UFUNCTION(BlueprintPure, Category = "Economy|Wager")
	int64 GetActiveWager() const { return ActiveWager; }

	/** Check if there's an active wager */
	UFUNCTION(BlueprintPure, Category = "Economy|Wager")
	bool HasActiveWager() const { return ActiveWager > 0; }

	// ==========================================
	// DAILY BONUSES
	// ==========================================

	/** Claim daily login bonus */
	UFUNCTION(BlueprintCallable, Category = "Economy|Daily")
	bool ClaimDailyLoginBonus(int64& OutBonusAmount);

	/** Get current login streak days */
	UFUNCTION(BlueprintPure, Category = "Economy|Daily")
	int32 GetLoginStreakDays() const { return LoginStreakDays; }

	/** Check if daily bonus is available */
	UFUNCTION(BlueprintPure, Category = "Economy|Daily")
	bool IsDailyBonusAvailable() const;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Format credits as display string */
	UFUNCTION(BlueprintPure, Category = "Economy|Utility")
	static FText FormatCredits(int64 Amount);

	/** Get transaction type display name */
	UFUNCTION(BlueprintPure, Category = "Economy|Utility")
	static FText GetTransactionTypeName(EMGTransactionType Type);

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Economy|Events")
	FOnCreditsChanged OnCreditsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Economy|Events")
	FOnTransactionCompleted OnTransactionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Economy|Events")
	FOnPurchaseResult OnPurchaseResult;

protected:
	/** Record a transaction */
	void RecordTransaction(EMGTransactionType Type, int64 Amount, const FText& Description, FName RelatedItemID = NAME_None);

	// ==========================================
	// DATA
	// ==========================================

	/**
	 * Current credit balance
	 * Starting cash: $7,500 (enough for meaningful first upgrades)
	 * Tutorial completion bonus: $2,500 additional
	 * See MGEconomyBalanceConfig.cpp for balance rationale
	 */
	UPROPERTY(SaveGame)
	int64 Credits = 7500; // Starting credits - balanced for meaningful first upgrades

	/** Total credits earned all-time */
	UPROPERTY(SaveGame)
	int64 TotalEarned = 0;

	/** Total credits spent all-time */
	UPROPERTY(SaveGame)
	int64 TotalSpent = 0;

	/** Transaction history */
	UPROPERTY(SaveGame)
	TArray<FMGTransaction> TransactionHistory;

	/** Maximum transactions to keep in history */
	int32 MaxTransactionHistory = 100;

	/** Active wager amount */
	UPROPERTY(SaveGame)
	int64 ActiveWager = 0;

	/** Active wager race ID */
	UPROPERTY(SaveGame)
	FName ActiveWagerRaceID;

	/** Current login streak days */
	UPROPERTY(SaveGame)
	int32 LoginStreakDays = 0;

	/** Last login date for streak tracking */
	UPROPERTY(SaveGame)
	FDateTime LastLoginDate;
};

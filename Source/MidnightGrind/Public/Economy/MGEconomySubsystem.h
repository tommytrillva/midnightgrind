// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGEconomySubsystem.h
 * @brief Core economy subsystem managing player credits, transactions, and economic balance.
 *
 * The Economy Subsystem is the foundation of Midnight Grind's in-game economy. It manages
 * the player's credit balance, tracks all financial transactions, and provides APIs for
 * race earnings, shop purchases, wagers, and daily bonuses. The system is designed around
 * the philosophy of "Feel the Grind, Not the Frustration" - progression should be
 * satisfying and authentic to car culture.
 *
 * @section econ_concepts Key Concepts
 *
 * **Credits:**
 * Credits are the primary in-game currency. Players start with $7,500 - enough for
 * meaningful first upgrades but not enough to skip the early game grind. Credits are
 * earned through racing, selling items, and completing challenges.
 *
 * **Transaction Types:**
 * All credit changes are logged as transactions with specific types:
 * - RaceWinnings/RaceEntryFee: Money earned or spent on races
 * - VehiclePurchase/VehicleSale: Buying and selling cars
 * - PartPurchase/PartSale: Performance and cosmetic parts
 * - PartInstallLabor: Shop labor costs for installations
 * - PinkSlipWin/PinkSlipLoss: Vehicle transfers from pink slip races
 * - RepairCost: Fixing vehicle damage
 * - Wager: Side bets on races
 * - DailyBonus/MilestoneReward: Progression rewards
 *
 * **Balance Philosophy:**
 * The economy is balanced around several core principles:
 * - Risk = Reward: Higher stakes races pay proportionally more
 * - Car Culture Authenticity: Parts and vehicles priced realistically
 * - The Build Journey Matters: Upgrading a car should feel meaningful
 * - Respect Player Time: Grinding should progress, not plateau
 *
 * **Depreciation:**
 * Vehicles and parts lose value when sold back. The default depreciation rate is 30%,
 * meaning you'll get 70% of the purchase price back. Condition affects this further.
 *
 * @section econ_wagers Wager System
 *
 * Players can place side bets on races for additional risk/reward:
 * - Only one active wager at a time
 * - Wager amount is held until race completes
 * - Winning multiplies the wager by the odds (default 2x)
 * - Losing forfeits the entire wager
 *
 * @section econ_daily Daily Bonuses
 *
 * The daily login bonus encourages regular play:
 * - Bonus amount increases with login streak
 * - Streak resets if you miss a day
 * - Maximum streak bonus caps at a reasonable level
 *
 * @section econ_usage Basic Usage Examples
 *
 * **Checking Balance and Affordability:**
 * @code
 * UMGEconomySubsystem* Economy = GetGameInstance()->GetSubsystem<UMGEconomySubsystem>();
 *
 * // Get current balance
 * int64 Credits = Economy->GetCredits();
 *
 * // Check if player can afford something
 * if (Economy->CanAfford(25000))
 * {
 *     // Player has at least $25,000
 * }
 *
 * // Format for display
 * FText DisplayBalance = UMGEconomySubsystem::FormatCredits(Credits);
 * // Result: "$7,500" or "$1,234,567"
 * @endcode
 *
 * **Processing Race Economy:**
 * @code
 * // Calculate winnings based on position
 * int64 Winnings = UMGEconomySubsystem::CalculateRaceWinnings(
 *     1,      // Position (1st place)
 *     8,      // Total racers
 *     10000,  // Base prize pool
 *     1.5f    // Difficulty multiplier
 * );
 *
 * // Pay entry fee before race
 * int64 EntryFee = UMGEconomySubsystem::CalculateEntryFee(10000, 0.1f);
 * if (Economy->PayEntryFee(EntryFee, RaceID))
 * {
 *     // Entry fee paid, race can begin
 * }
 *
 * // Award winnings after race
 * Economy->AwardRaceWinnings(Winnings, RaceID);
 * @endcode
 *
 * **Handling Pink Slip Races:**
 * @code
 * // When player wins opponent's car
 * int64 VehicleValue = 45000;
 * Economy->ProcessPinkSlipWin(VehicleValue, OpponentVehicleID);
 *
 * // When player loses their car
 * Economy->ProcessPinkSlipLoss(PlayerVehicleValue, PlayerVehicleID);
 * @endcode
 *
 * **Shop Transactions:**
 * @code
 * // Purchase a vehicle
 * FText Message;
 * if (Economy->PurchaseVehicle(VehicleModelData, Message))
 * {
 *     // Vehicle purchased successfully
 * }
 *
 * // Sell a part
 * int64 SellPrice = UMGEconomySubsystem::CalculateSellValue(
 *     OriginalPrice,
 *     0.9f,   // 90% condition
 *     0.3f    // 30% depreciation
 * );
 * Economy->SellPart(PartID, SellPrice, Message);
 * @endcode
 *
 * **Using Wagers:**
 * @code
 * // Place a side bet
 * if (Economy->PlaceWager(5000, RaceID))
 * {
 *     // $5,000 wager placed
 * }
 *
 * // Check active wager
 * if (Economy->HasActiveWager())
 * {
 *     int64 WagerAmount = Economy->GetActiveWager();
 * }
 *
 * // After race - resolve wager
 * Economy->ResolveWager(true, 2.5f);  // Won at 2.5:1 odds
 * // Player receives $12,500 (5000 * 2.5)
 * @endcode
 *
 * **Daily Bonuses:**
 * @code
 * // Check if bonus is available
 * if (Economy->IsDailyBonusAvailable())
 * {
 *     int64 BonusAmount;
 *     if (Economy->ClaimDailyLoginBonus(BonusAmount))
 *     {
 *         // Bonus claimed! Show notification
 *         int32 Streak = Economy->GetLoginStreakDays();
 *     }
 * }
 * @endcode
 *
 * **Listening to Events:**
 * @code
 * // React to credit changes
 * Economy->OnCreditsChanged.AddDynamic(this, &UMyWidget::HandleCreditsChanged);
 *
 * void UMyWidget::HandleCreditsChanged(int64 NewBalance, int64 Delta)
 * {
 *     if (Delta > 0)
 *     {
 *         ShowEarningsAnimation(Delta);
 *     }
 *     else
 *     {
 *         ShowSpendingAnimation(-Delta);
 *     }
 *     UpdateBalanceDisplay(NewBalance);
 * }
 * @endcode
 *
 * @see UMGTransactionPipeline For complex multi-system transactions
 * @see UMGShopSubsystem For shop browsing and item catalogs
 * @see UMGProgressionSubsystem For XP and reputation systems
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "Economy/MGTransactionPipeline.h"
#include "Economy/MGShopSubsystem.h"
#include "Challenges/MGChallengeSubsystem.h"
#include "MGEconomySubsystem.generated.h"

class UMGVehicleModelData;

// MOVED TO MGSharedTypes.h
// /**
//  * Transaction types for history tracking
//  */
// UENUM(BlueprintType)
// enum class EMGTransactionType : uint8
// {
// 	RaceWinnings,
// 	RaceEntryFee,
// 	VehiclePurchase,
// 	VehicleSale,
// 	PartPurchase,
// 	PartSale,
// 	PartInstallLabor,    /**< Shop labor cost for part installation */
// 	PaintJob,
// 	RepairCost,
// 	PinkSlipWin,
// 	PinkSlipLoss,
// 	BountyReward,
// 	CrewBonus,
// 	Wager,
// 	DailyBonus,
// 	MilestoneReward,
// 	TournamentPrize,
// 	MarketplaceSale,
// 	MarketplacePurchase,
// 	Trade,
// 	DynoRental,
// 	ToolPurchase,        /**< Player purchased a mechanic tool */
// 	GarageUpgrade,       /**< Player upgraded their garage (lift, etc.) */
// 	FuelPurchase,        /**< Player purchased fuel for vehicle */
// 	Other
// };

// FMGTransaction - MOVED TO Economy/MGTransactionPipeline.h (included above)

// FMGShopItem - MOVED TO Economy/MGShopSubsystem.h (included above)

// FMGChallengeReward - MOVED TO Challenges/MGChallengeSubsystem.h (included above)

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
	// MARKETPLACE INTEGRATION
	// ==========================================

	/**
	 * Deduct cash for marketplace transactions
	 * @param PlayerID Player identifier (unused in single-player, for future multiplayer)
	 * @param Amount Amount to deduct
	 * @param Description Transaction description
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Marketplace")
	bool DeductCash(FGuid PlayerID, int64 Amount, const FString& Description);

	/**
	 * Add cash from marketplace transactions
	 * @param PlayerID Player identifier (unused in single-player, for future multiplayer)
	 * @param Amount Amount to add
	 * @param Description Transaction description
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy|Marketplace")
	void AddCash(FGuid PlayerID, int64 Amount, const FString& Description);

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

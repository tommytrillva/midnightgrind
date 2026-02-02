// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "Progression/MGPlayerProgression.h"
#include "MGTransactionPipeline.generated.h"

// EMGTransactionType - MOVED TO MGSharedTypes.h
// (Previously in Economy/MGEconomySubsystem.h)

/**
 * Item type for transactions
 */
UENUM(BlueprintType)
enum class EMGTransactionItemType : uint8
{
	Currency,
	Vehicle,
	Part,
	Paint,
	Vinyl,
	Wheel,
	Customization,
	Consumable
};

/**
 * Single transaction item
 */
USTRUCT(BlueprintType)
struct FMGTransactionItem
{
	GENERATED_BODY()

	/** Item type */
	UPROPERTY(BlueprintReadOnly)
	EMGTransactionItemType ItemType = EMGTransactionItemType::Currency;

	/** Item ID */
	UPROPERTY(BlueprintReadOnly)
	FName ItemID;

	/** Quantity (for stackables) */
	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 1;

	/** Custom data (e.g., vehicle config) */
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> Metadata;
};

/**
 * Complete transaction record
 */
USTRUCT(BlueprintType)
struct FMGTransaction
{
	GENERATED_BODY()

	/** Unique transaction ID */
	UPROPERTY(BlueprintReadOnly)
	FGuid TransactionID;

	/** Transaction type */
	UPROPERTY(BlueprintReadOnly)
	EMGTransactionType Type = EMGTransactionType::ShopPurchase;

	/** Items received */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGTransactionItem> ItemsReceived;

	/** Items given (including currency spent) */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGTransactionItem> ItemsGiven;

	/** Credits spent (negative) or earned (positive) */
	UPROPERTY(BlueprintReadOnly)
	int64 CreditsDelta = 0;

	/** Premium currency spent or earned */
	UPROPERTY(BlueprintReadOnly)
	int32 PremiumCurrencyDelta = 0;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Was successful */
	UPROPERTY(BlueprintReadOnly)
	bool bSuccessful = false;

	/** Failure reason (if any) */
	UPROPERTY(BlueprintReadOnly)
	FText FailureReason;

	/** Source context (shop ID, race ID, etc.) */
	UPROPERTY(BlueprintReadOnly)
	FString SourceContext;
};

/**
 * Purchase request
 */
USTRUCT(BlueprintType)
struct FMGPurchaseRequest
{
	GENERATED_BODY()

	/** Shop item ID */
	UPROPERTY(BlueprintReadWrite)
	FName ShopItemID;

	/** Item type being purchased */
	UPROPERTY(BlueprintReadWrite)
	EMGTransactionItemType ItemType = EMGTransactionItemType::Part;

	/** Item ID */
	UPROPERTY(BlueprintReadWrite)
	FName ItemID;

	/** Price in credits */
	UPROPERTY(BlueprintReadWrite)
	int64 Price = 0;

	/** Price in premium currency (if applicable) */
	UPROPERTY(BlueprintReadWrite)
	int32 PremiumPrice = 0;

	/** Quantity */
	UPROPERTY(BlueprintReadWrite)
	int32 Quantity = 1;

	/** For vehicle/part purchases: target vehicle to install on */
	UPROPERTY(BlueprintReadWrite)
	FName TargetVehicleID;

	/** Immediately install (for parts) */
	UPROPERTY(BlueprintReadWrite)
	bool bInstallImmediately = false;
};

/**
 * Transaction result
 */
UENUM(BlueprintType)
enum class EMGTransactionResult : uint8
{
	Success,
	InsufficientFunds,
	ItemNotAvailable,
	InventoryFull,
	AlreadyOwned,
	LevelRestricted,
	InvalidRequest,
	ServerError
};

/**
 * Delegate for transaction events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTransactionComplete, const FMGTransaction&, Transaction, EMGTransactionResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPurchaseComplete, const FMGTransaction&, Transaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemReceived, const FMGTransactionItem&, Item);

/**
 * Transaction Pipeline
 * Handles all item/currency exchanges between systems
 *
 * Features:
 * - Validates purchases before processing
 * - Updates economy (credits)
 * - Updates garage (vehicles, parts)
 * - Updates inventory (consumables, customization)
 * - Maintains transaction history
 * - Supports rollback on failure
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTransactionPipeline : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// PURCHASES
	// ==========================================

	/**
	 * Process a purchase request
	 * @param Request The purchase details
	 * @return Transaction result
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	EMGTransactionResult ProcessPurchase(const FMGPurchaseRequest& Request);

	/**
	 * Quick purchase (item ID + price)
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	EMGTransactionResult QuickPurchase(FName ItemID, EMGTransactionItemType ItemType, int64 Price);

	/**
	 * Check if player can afford purchase
	 */
	UFUNCTION(BlueprintPure, Category = "Transaction")
	bool CanAfford(int64 Credits, int32 PremiumCurrency = 0) const;

	/**
	 * Check if purchase is valid
	 */
	UFUNCTION(BlueprintPure, Category = "Transaction")
	EMGTransactionResult ValidatePurchase(const FMGPurchaseRequest& Request) const;

	// ==========================================
	// SALES
	// ==========================================

	/**
	 * Sell item back to shop
	 * @param ItemType Type of item
	 * @param ItemID Item to sell
	 * @param SaleValue Value to receive
	 * @return Transaction result
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	EMGTransactionResult SellItem(EMGTransactionItemType ItemType, FName ItemID, int64 SaleValue);

	/**
	 * Sell vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	EMGTransactionResult SellVehicle(FName VehicleID, int64 SaleValue);

	/**
	 * Get sale value for item
	 */
	UFUNCTION(BlueprintPure, Category = "Transaction")
	int64 GetSaleValue(EMGTransactionItemType ItemType, FName ItemID) const;

	// ==========================================
	// REWARDS
	// ==========================================

	/**
	 * Award race rewards
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	void AwardRaceRewards(int64 Credits, int32 XP, int32 Reputation, const TArray<FMGTransactionItem>& BonusItems);

	/**
	 * Award challenge completion rewards
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	void AwardChallengeRewards(FName ChallengeID, int64 Credits, const TArray<FMGTransactionItem>& Items);

	/**
	 * Award level up rewards
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	void AwardLevelUpRewards(int32 NewLevel, const TArray<FMGTransactionItem>& Items);

	/**
	 * Award pink slip vehicle (won from opponent)
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	void AwardPinkSlipVehicle(FName VehicleID, FName OpponentID);

	/**
	 * Remove vehicle (lost pink slip)
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	void RemovePinkSlipVehicle(FName VehicleID);

	// ==========================================
	// COSTS
	// ==========================================

	/**
	 * Pay repair costs
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	EMGTransactionResult PayRepairCost(FName VehicleID, int64 Cost);

	/**
	 * Pay customization cost (paint, vinyl, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	EMGTransactionResult PayCustomizationCost(FName VehicleID, int64 Cost, const FString& Description);

	/**
	 * Pay upgrade/install cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Transaction")
	EMGTransactionResult PayUpgradeCost(FName VehicleID, FName PartID, int64 Cost);

	// ==========================================
	// HISTORY
	// ==========================================

	/**
	 * Get transaction history
	 */
	UFUNCTION(BlueprintPure, Category = "Transaction|History")
	TArray<FMGTransaction> GetTransactionHistory(int32 Count = 50) const;

	/**
	 * Get transactions by type
	 */
	UFUNCTION(BlueprintPure, Category = "Transaction|History")
	TArray<FMGTransaction> GetTransactionsByType(EMGTransactionType Type, int32 Count = 50) const;

	/**
	 * Get total spent
	 */
	UFUNCTION(BlueprintPure, Category = "Transaction|History")
	int64 GetTotalSpent() const;

	/**
	 * Get total earned
	 */
	UFUNCTION(BlueprintPure, Category = "Transaction|History")
	int64 GetTotalEarned() const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Transaction completed */
	UPROPERTY(BlueprintAssignable, Category = "Transaction|Events")
	FOnTransactionComplete OnTransactionComplete;

	/** Purchase completed successfully */
	UPROPERTY(BlueprintAssignable, Category = "Transaction|Events")
	FOnPurchaseComplete OnPurchaseComplete;

	/** Item received (for UI notifications) */
	UPROPERTY(BlueprintAssignable, Category = "Transaction|Events")
	FOnItemReceived OnItemReceived;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Execute credit transaction */
	bool DeductCredits(int64 Amount);

	/** Add credits */
	void AddCredits(int64 Amount);

	/** Add item to appropriate system */
	bool AddItemToInventory(const FMGTransactionItem& Item, FName TargetVehicleID = NAME_None);

	/** Remove item from inventory */
	bool RemoveItemFromInventory(const FMGTransactionItem& Item);

	/** Record transaction */
	void RecordTransaction(const FMGTransaction& Transaction);

	/** Generate transaction ID */
	FGuid GenerateTransactionID() const;

	/** Cache subsystem references */
	void CacheSubsystems();

private:
	/** Transaction history */
	TArray<FMGTransaction> TransactionHistory;

	/** Max history entries to keep */
	static constexpr int32 MaxHistoryEntries = 500;

	/** Subsystem references */
	UPROPERTY()
	TWeakObjectPtr<UMGEconomySubsystem> EconomySubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGGarageSubsystem> GarageSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGPlayerProgression> ProgressionSubsystem;
};

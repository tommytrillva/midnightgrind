// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGShopSubsystem.generated.h"

class UTexture2D;

/**
 * Currency type
 */
UENUM(BlueprintType)
enum class EMGCurrencyType : uint8
{
	/** In-game cash */
	Cash,
	/** Premium currency */
	Gold,
	/** Reputation points (cannot purchase, earned only) */
	Reputation,
	/** Season tokens */
	SeasonTokens,
	/** Crew tokens */
	CrewTokens
};

/**
 * Shop category
 */
UENUM(BlueprintType)
enum class EMGShopCategory : uint8
{
	/** Vehicles */
	Vehicles,
	/** Performance parts */
	Performance,
	/** Visual customization */
	Cosmetic,
	/** Wraps and decals */
	WrapsDecals,
	/** Wheels */
	Wheels,
	/** Special/limited items */
	Special,
	/** Premium items */
	Premium,
	/** Bundles */
	Bundles
};

/**
 * Purchase result
 */
UENUM(BlueprintType)
enum class EMGPurchaseResult : uint8
{
	/** Purchase successful */
	Success,
	/** Not enough currency */
	InsufficientFunds,
	/** Item already owned */
	AlreadyOwned,
	/** Level requirement not met */
	LevelRequirementNotMet,
	/** Item not available */
	ItemNotAvailable,
	/** Purchase failed (generic) */
	Failed
};

/**
 * Shop item price
 */
USTRUCT(BlueprintType)
struct FMGItemPrice
{
	GENERATED_BODY()

	/** Currency type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCurrencyType Currency = EMGCurrencyType::Cash;

	/** Amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;

	/** Original price (for sales) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OriginalAmount = 0;

	/** Is on sale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnSale = false;

	/** Sale percentage */
	UPROPERTY(BlueprintReadOnly)
	float SalePercent = 0.0f;
};

/**
 * Shop item
 */
USTRUCT(BlueprintType)
struct FMGShopItem
{
	GENERATED_BODY()

	/** Item ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShopCategory Category = EMGShopCategory::Cosmetic;

	/** Preview image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PreviewImage;

	/** Price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGItemPrice Price;

	/** Alternative price (some items have dual pricing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGItemPrice AlternativePrice;

	/** Has alternative price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasAlternativePrice = false;

	/** Required level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/** Required reputation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredReputation = 0;

	/** Is limited time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLimitedTime = false;

	/** Time remaining (if limited) */
	UPROPERTY(BlueprintReadOnly)
	FTimespan TimeRemaining;

	/** Stock quantity (-1 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StockQuantity = -1;

	/** Is new */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsNew = false;

	/** Is featured */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;

	/** Is owned */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOwned = false;

	/** Can afford */
	UPROPERTY(BlueprintReadOnly)
	bool bCanAfford = false;

	/** Meets requirements */
	UPROPERTY(BlueprintReadOnly)
	bool bMeetsRequirements = false;

	/** Vehicle compatibility (empty = all) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> CompatibleVehicles;

	/** Associated asset ID (vehicle, part, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AssetID;

	/** Rarity tier (0-4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rarity = 0;
};

/**
 * Bundle item
 */
USTRUCT(BlueprintType)
struct FMGBundleItem
{
	GENERATED_BODY()

	/** Bundle ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BundleID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Preview image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PreviewImage;

	/** Items in bundle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ItemIDs;

	/** Bundle price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGItemPrice Price;

	/** Total value of items (for showing savings) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalValue = 0;

	/** Savings percentage */
	UPROPERTY(BlueprintReadOnly)
	float SavingsPercent = 0.0f;

	/** Is limited time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLimitedTime = false;

	/** Expiration time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpirationTime;
};

/**
 * Daily deal
 */
USTRUCT(BlueprintType)
struct FMGDailyDeal
{
	GENERATED_BODY()

	/** Deal item */
	UPROPERTY(BlueprintReadOnly)
	FMGShopItem Item;

	/** Discount percentage */
	UPROPERTY(BlueprintReadOnly)
	float DiscountPercent = 0.0f;

	/** Discounted price */
	UPROPERTY(BlueprintReadOnly)
	FMGItemPrice DiscountedPrice;

	/** Time remaining */
	UPROPERTY(BlueprintReadOnly)
	FTimespan TimeRemaining;

	/** Deal index (for multiple daily deals) */
	UPROPERTY(BlueprintReadOnly)
	int32 DealIndex = 0;
};

/**
 * Transaction record
 */
USTRUCT(BlueprintType)
struct FMGTransaction
{
	GENERATED_BODY()

	/** Transaction ID */
	UPROPERTY(BlueprintReadOnly)
	FString TransactionID;

	/** Item ID */
	UPROPERTY(BlueprintReadOnly)
	FName ItemID;

	/** Item name */
	UPROPERTY(BlueprintReadOnly)
	FText ItemName;

	/** Currency used */
	UPROPERTY(BlueprintReadOnly)
	EMGCurrencyType Currency = EMGCurrencyType::Cash;

	/** Amount paid */
	UPROPERTY(BlueprintReadOnly)
	int32 AmountPaid = 0;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Was refunded */
	UPROPERTY(BlueprintReadOnly)
	bool bRefunded = false;
};

/**
 * Wallet data
 */
USTRUCT(BlueprintType)
struct FMGWallet
{
	GENERATED_BODY()

	/** Cash balance */
	UPROPERTY(BlueprintReadOnly)
	int64 Cash = 0;

	/** Gold balance */
	UPROPERTY(BlueprintReadOnly)
	int32 Gold = 0;

	/** Reputation (display only) */
	UPROPERTY(BlueprintReadOnly)
	int32 Reputation = 0;

	/** Season tokens */
	UPROPERTY(BlueprintReadOnly)
	int32 SeasonTokens = 0;

	/** Crew tokens */
	UPROPERTY(BlueprintReadOnly)
	int32 CrewTokens = 0;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPurchaseComplete, const FMGShopItem&, Item, EMGPurchaseResult, Result, const FMGTransaction&, Transaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyChanged, EMGCurrencyType, Currency, int64, NewBalance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDailyDealsRefreshed, const TArray<FMGDailyDeal>&, Deals);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopRefreshed, EMGShopCategory, Category);

/**
 * Shop Subsystem
 * Manages in-game shop and economy
 *
 * Features:
 * - Multi-currency system
 * - Category-based shop
 * - Daily deals
 * - Bundles
 * - Transaction history
 */
UCLASS()
class MIDNIGHTGRIND_API UMGShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPurchaseComplete OnPurchaseComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCurrencyChanged OnCurrencyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDailyDealsRefreshed OnDailyDealsRefreshed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShopRefreshed OnShopRefreshed;

	// ==========================================
	// WALLET
	// ==========================================

	/** Get wallet */
	UFUNCTION(BlueprintPure, Category = "Shop|Wallet")
	FMGWallet GetWallet() const { return Wallet; }

	/** Get currency balance */
	UFUNCTION(BlueprintPure, Category = "Shop|Wallet")
	int64 GetCurrencyBalance(EMGCurrencyType Currency) const;

	/** Add currency */
	UFUNCTION(BlueprintCallable, Category = "Shop|Wallet")
	void AddCurrency(EMGCurrencyType Currency, int64 Amount);

	/** Remove currency */
	UFUNCTION(BlueprintCallable, Category = "Shop|Wallet")
	bool RemoveCurrency(EMGCurrencyType Currency, int64 Amount);

	/** Can afford price */
	UFUNCTION(BlueprintPure, Category = "Shop|Wallet")
	bool CanAfford(const FMGItemPrice& Price) const;

	/** Format currency for display */
	UFUNCTION(BlueprintPure, Category = "Shop|Wallet")
	static FText FormatCurrency(EMGCurrencyType Currency, int64 Amount);

	// ==========================================
	// SHOP BROWSING
	// ==========================================

	/** Get items by category */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> GetItemsByCategory(EMGShopCategory Category) const;

	/** Get featured items */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> GetFeaturedItems() const;

	/** Get new items */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> GetNewItems() const;

	/** Get item by ID */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	FMGShopItem GetItem(FName ItemID) const;

	/** Search items */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> SearchItems(const FString& SearchTerm) const;

	/** Get items for vehicle */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> GetItemsForVehicle(FName VehicleID, EMGShopCategory Category) const;

	// ==========================================
	// PURCHASING
	// ==========================================

	/** Purchase item */
	UFUNCTION(BlueprintCallable, Category = "Shop|Purchase")
	EMGPurchaseResult PurchaseItem(FName ItemID, bool bUseAlternativePrice = false);

	/** Purchase bundle */
	UFUNCTION(BlueprintCallable, Category = "Shop|Purchase")
	EMGPurchaseResult PurchaseBundle(FName BundleID);

	/** Can purchase item */
	UFUNCTION(BlueprintPure, Category = "Shop|Purchase")
	EMGPurchaseResult CanPurchaseItem(FName ItemID, bool bUseAlternativePrice = false) const;

	/** Get purchase preview (what you get) */
	UFUNCTION(BlueprintPure, Category = "Shop|Purchase")
	FMGShopItem GetPurchasePreview(FName ItemID) const;

	// ==========================================
	// DAILY DEALS
	// ==========================================

	/** Get daily deals */
	UFUNCTION(BlueprintPure, Category = "Shop|Deals")
	TArray<FMGDailyDeal> GetDailyDeals() const { return DailyDeals; }

	/** Get time until deals refresh */
	UFUNCTION(BlueprintPure, Category = "Shop|Deals")
	FTimespan GetTimeUntilDealsRefresh() const;

	/** Refresh deals (admin/debug) */
	UFUNCTION(BlueprintCallable, Category = "Shop|Deals")
	void ForceRefreshDeals();

	// ==========================================
	// BUNDLES
	// ==========================================

	/** Get available bundles */
	UFUNCTION(BlueprintPure, Category = "Shop|Bundles")
	TArray<FMGBundleItem> GetAvailableBundles() const { return AvailableBundles; }

	/** Get bundle by ID */
	UFUNCTION(BlueprintPure, Category = "Shop|Bundles")
	FMGBundleItem GetBundle(FName BundleID) const;

	/** Get bundle contents preview */
	UFUNCTION(BlueprintPure, Category = "Shop|Bundles")
	TArray<FMGShopItem> GetBundleContents(FName BundleID) const;

	// ==========================================
	// OWNERSHIP
	// ==========================================

	/** Is item owned */
	UFUNCTION(BlueprintPure, Category = "Shop|Ownership")
	bool IsItemOwned(FName ItemID) const;

	/** Get owned items */
	UFUNCTION(BlueprintPure, Category = "Shop|Ownership")
	TArray<FName> GetOwnedItems() const { return OwnedItems; }

	/** Get owned items in category */
	UFUNCTION(BlueprintPure, Category = "Shop|Ownership")
	TArray<FMGShopItem> GetOwnedItemsInCategory(EMGShopCategory Category) const;

	// ==========================================
	// TRANSACTION HISTORY
	// ==========================================

	/** Get transaction history */
	UFUNCTION(BlueprintPure, Category = "Shop|History")
	TArray<FMGTransaction> GetTransactionHistory(int32 Count = 50) const;

	/** Get recent purchases */
	UFUNCTION(BlueprintPure, Category = "Shop|History")
	TArray<FMGTransaction> GetRecentPurchases(int32 Count = 10) const;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get rarity name */
	UFUNCTION(BlueprintPure, Category = "Shop|Utility")
	static FText GetRarityName(int32 Rarity);

	/** Get rarity color */
	UFUNCTION(BlueprintPure, Category = "Shop|Utility")
	static FLinearColor GetRarityColor(int32 Rarity);

	/** Get category display name */
	UFUNCTION(BlueprintPure, Category = "Shop|Utility")
	static FText GetCategoryDisplayName(EMGShopCategory Category);

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Player wallet */
	UPROPERTY()
	FMGWallet Wallet;

	/** All shop items */
	UPROPERTY()
	TMap<FName, FMGShopItem> AllItems;

	/** Owned items */
	UPROPERTY()
	TArray<FName> OwnedItems;

	/** Daily deals */
	UPROPERTY()
	TArray<FMGDailyDeal> DailyDeals;

	/** Available bundles */
	UPROPERTY()
	TArray<FMGBundleItem> AvailableBundles;

	/** Transaction history */
	UPROPERTY()
	TArray<FMGTransaction> Transactions;

	/** Last deals refresh */
	FDateTime LastDealsRefresh;

	/** Deals refresh interval */
	float DealsRefreshCheckInterval = 60.0f;
	float DealsRefreshAccumulator = 0.0f;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Load shop data */
	void LoadShopData();

	/** Save shop data */
	void SaveShopData();

	/** Generate daily deals */
	void GenerateDailyDeals();

	/** Check for deals refresh */
	void CheckDealsRefresh();

	/** Update item status (owned, can afford, etc.) */
	void UpdateItemStatus(FMGShopItem& Item) const;

	/** Process purchase */
	FMGTransaction ProcessPurchase(const FMGShopItem& Item, const FMGItemPrice& Price);

	/** Grant item to player */
	void GrantItem(FName ItemID);

	/** Generate mock shop data */
	void GenerateMockShopData();

	/** Calculate sale percent */
	float CalculateSalePercent(int32 Original, int32 Current) const;
};

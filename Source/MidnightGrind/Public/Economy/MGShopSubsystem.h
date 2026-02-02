// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "Economy/MGTransactionPipeline.h"
#include "Store/MGStoreSubsystem.h"
#include "MGShopSubsystem.generated.h"

// EMGCurrencyType - MOVED TO MGSharedTypes.h
// (Previously in Currency/MGCurrencySubsystem.h)

/**
 * @brief Categories for organizing shop items.
 *
 * Items are grouped into categories for easier browsing in the shop UI.
 * Each category may have different filtering and sorting options.
 */
UENUM(BlueprintType)
enum class EMGShopCategory : uint8
{
	/** Vehicles - Complete cars available for purchase */
	Vehicles,
	/** Performance parts - Engine, turbo, suspension upgrades */
	Performance,
	/** Visual customization - Body kits, spoilers, mirrors */
	Cosmetic,
	/** Wraps and decals - Paint schemes and stickers */
	WrapsDecals,
	/** Wheels - Rims and tire combinations */
	Wheels,
	/** Special/limited items - Time-limited or event-exclusive content */
	Special,
	/** Premium items - Gold currency exclusive items */
	Premium,
	/** Bundles - Grouped items sold together at a discount */
	Bundles
};

/**
 * @brief Result codes returned from purchase operations.
 *
 * These codes help the UI display appropriate feedback to the player
 * and can be used to trigger specific recovery actions.
 */
UENUM(BlueprintType)
enum class EMGPurchaseResult : uint8
{
	/** Purchase successful - Item has been added to inventory */
	Success,
	/** Not enough currency - Player needs more of the required currency */
	InsufficientFunds,
	/** Item already owned - Cannot purchase duplicates of this item */
	AlreadyOwned,
	/** Level requirement not met - Player must reach a higher level first */
	LevelRequirementNotMet,
	/** Item not available - Item is out of stock or no longer in shop */
	ItemNotAvailable,
	/** Purchase failed (generic) - Server error or unknown failure */
	Failed
};

// ============================================================================
// DATA STRUCTURES - Pricing
// ============================================================================

/**
 * @brief Represents the price of a shop item with sale information.
 *
 * This struct supports both regular pricing and sale pricing, allowing
 * the UI to show original prices crossed out with discounted prices.
 */
USTRUCT(BlueprintType)
struct FMGItemPrice
{
	GENERATED_BODY()

	/** Currency type required for this price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCurrencyType Currency = EMGCurrencyType::Cash;

	/** Current price amount (discounted if on sale) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;

	/** Original price before discount (used to show savings) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OriginalAmount = 0;

	/** Whether this item is currently on sale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnSale = false;

	/** Calculated sale percentage (0.0 to 1.0) - computed at runtime */
	UPROPERTY(BlueprintReadOnly)
	float SalePercent = 0.0f;
};

// ============================================================================
// DATA STRUCTURES - Shop Items
// ============================================================================

/**
 * @brief Complete data structure for a purchasable shop item.
 *
 * Contains all information needed to display and purchase an item,
 * including metadata, pricing, requirements, and current status.
 */
USTRUCT(BlueprintType)
struct FMGShopItem
{
	GENERATED_BODY()

	/// @name Item Identity
	/// @{

	/** Unique identifier for this item (e.g., "vehicle_skyline_r34") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	/** Localized display name shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Localized description with item details */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Shop category this item belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShopCategory Category = EMGShopCategory::Cosmetic;

	/** Preview image for shop display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PreviewImage;

	/// @}

	/// @name Pricing Information
	/// @{

	/** Primary price for this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGItemPrice Price;

	/** Alternative price (some items have dual pricing, e.g., Cash OR Gold) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGItemPrice AlternativePrice;

	/** Whether this item can be purchased with alternative currency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasAlternativePrice = false;

	/// @}

	/// @name Requirements
	/// @{

	/** Minimum player level required to purchase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/** Minimum reputation required to purchase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredReputation = 0;

	/// @}

	/// @name Availability
	/// @{

	/** Whether this item is only available for a limited time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLimitedTime = false;

	/** Time remaining until item is removed from shop (if limited) */
	UPROPERTY(BlueprintReadOnly)
	FTimespan TimeRemaining;

	/** Available stock quantity (-1 indicates unlimited stock) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StockQuantity = -1;

	/// @}

	/// @name Display Flags
	/// @{

	/** Whether to show "NEW" badge on this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsNew = false;

	/** Whether this item appears in the featured section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;

	/// @}

	/// @name Runtime Status (computed per-player)
	/// @{

	/** Whether the current player already owns this item */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOwned = false;

	/** Whether the current player can afford this item */
	UPROPERTY(BlueprintReadOnly)
	bool bCanAfford = false;

	/** Whether the current player meets all requirements */
	UPROPERTY(BlueprintReadOnly)
	bool bMeetsRequirements = false;

	/// @}

	/// @name Item Data
	/// @{

	/** List of vehicle IDs this item is compatible with (empty = universal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> CompatibleVehicles;

	/** Reference to the actual game asset this item unlocks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AssetID;

	/** Rarity tier: 0=Common, 1=Uncommon, 2=Rare, 3=Epic, 4=Legendary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rarity = 0;

	/// @}
};

/**
 * @brief Represents a bundle of multiple items sold together at a discount.
 *
 * Bundles allow players to purchase multiple related items at once
 * for less than buying them individually.
 */
USTRUCT(BlueprintType)
struct FMGBundleItem
{
	GENERATED_BODY()

	/** Unique identifier for this bundle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BundleID;

	/** Localized display name for the bundle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Localized description of bundle contents and value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Preview image showing bundle contents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PreviewImage;

	/** List of item IDs included in this bundle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ItemIDs;

	/** Bundle price (should be less than sum of individual prices) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGItemPrice Price;

	/** Combined value if items were purchased separately */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalValue = 0;

	/** Calculated savings percentage (0.0 to 1.0) */
	UPROPERTY(BlueprintReadOnly)
	float SavingsPercent = 0.0f;

	/** Whether this bundle is only available for a limited time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLimitedTime = false;

	/** When this bundle will be removed from the shop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpirationTime;
};

// FMGDailyDeal - MOVED TO Store/MGStoreSubsystem.h (included above)

// ============================================================================
// DATA STRUCTURES - Transactions
// ============================================================================

// FMGTransaction - MOVED TO Economy/MGTransactionPipeline.h (included above)

// ============================================================================
// DATA STRUCTURES - Wallet
// ============================================================================

/**
 * @brief Player's current currency balances across all currency types.
 *
 * The wallet provides a snapshot of all currencies for UI display
 * and affordability calculations.
 */
USTRUCT(BlueprintType)
struct FMGWallet
{
	GENERATED_BODY()

	/** In-game cash balance (64-bit for large amounts) */
	UPROPERTY(BlueprintReadOnly)
	int64 Cash = 0;

	/** Premium gold currency balance */
	UPROPERTY(BlueprintReadOnly)
	int32 Gold = 0;

	/** Reputation points (display only, not spendable) */
	UPROPERTY(BlueprintReadOnly)
	int32 Reputation = 0;

	/** Season pass tokens balance */
	UPROPERTY(BlueprintReadOnly)
	int32 SeasonTokens = 0;

	/** Crew activity tokens balance */
	UPROPERTY(BlueprintReadOnly)
	int32 CrewTokens = 0;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/// @brief Fired when a purchase attempt completes (success or failure)
/// @param Item The item that was being purchased
/// @param Result The outcome of the purchase attempt
/// @param Transaction The transaction record (valid only on success)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPurchaseComplete, const FMGShopItem&, Item, EMGPurchaseResult, Result, const FMGTransaction&, Transaction);

/// @brief Fired when any currency balance changes
/// @param Currency Which currency type changed
/// @param NewBalance The new balance after the change
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyChanged, EMGCurrencyType, Currency, int64, NewBalance);

/// @brief Fired when daily deals are refreshed with new items
/// @param Deals The new set of daily deals
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDailyDealsRefreshed, const TArray<FMGDailyDeal>&, Deals);

/// @brief Fired when shop inventory is refreshed for a category
/// @param Category Which category was refreshed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopRefreshed, EMGShopCategory, Category);

// ============================================================================
// SHOP SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Central subsystem for managing the in-game shop and economy.
 *
 * The Shop Subsystem serves as the primary interface between the player
 * and the game's economy. It manages:
 *
 * **Wallet Management:**
 * - Tracks all currency balances
 * - Provides formatted currency display
 * - Validates affordability checks
 *
 * **Shop Browsing:**
 * - Category-based item organization
 * - Search and filter functionality
 * - Featured and new item highlighting
 *
 * **Purchasing:**
 * - Multi-currency support
 * - Requirement validation
 * - Transaction logging
 *
 * **Daily Deals:**
 * - Automatic daily rotation
 * - Configurable discount percentages
 * - Countdown timers
 *
 * @note Changes to wallet balances should go through this subsystem to ensure
 *       proper event firing and save data synchronization.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// @name Lifecycle
	/// @{

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Called each frame to update timers and check for deal refreshes */
	virtual void Tick(float MGDeltaTime);

	/// @}

	// ==========================================
	// EVENTS
	// ==========================================

	/// @name Event Delegates
	/// @brief Subscribe to these events to react to shop state changes.
	/// @{

	/** Broadcast when a purchase completes (success or failure) */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPurchaseComplete OnPurchaseComplete;

	/** Broadcast when any currency balance changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCurrencyChanged OnCurrencyChanged;

	/** Broadcast when daily deals refresh with new items */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDailyDealsRefreshed OnDailyDealsRefreshed;

	/** Broadcast when shop inventory is refreshed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShopRefreshed OnShopRefreshed;

	/// @}

	// ==========================================
	// WALLET
	// ==========================================

	/// @name Wallet Operations
	/// @brief Functions for querying and modifying currency balances.
	/// @{

	/**
	 * @brief Get the complete wallet with all currency balances.
	 * @return Current wallet state with all currency amounts
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Wallet")
	FMGWallet GetWallet() const { return Wallet; }

	/**
	 * @brief Get the balance of a specific currency type.
	 * @param Currency The currency type to query
	 * @return Current balance of the specified currency
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Wallet")
	int64 GetCurrencyBalance(EMGCurrencyType Currency) const;

	/**
	 * @brief Add currency to the player's wallet.
	 * @param Currency The currency type to add
	 * @param Amount The amount to add (must be positive)
	 * @note Fires OnCurrencyChanged event
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Wallet")
	void AddCurrency(EMGCurrencyType Currency, int64 Amount);

	/**
	 * @brief Remove currency from the player's wallet.
	 * @param Currency The currency type to remove
	 * @param Amount The amount to remove (must be positive)
	 * @return True if successful, false if insufficient funds
	 * @note Fires OnCurrencyChanged event on success
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Wallet")
	bool RemoveCurrency(EMGCurrencyType Currency, int64 Amount);

	/**
	 * @brief Check if the player can afford a given price.
	 * @param Price The price to check against
	 * @return True if the player has enough currency
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Wallet")
	bool CanAfford(const FMGItemPrice& Price) const;

	/**
	 * @brief Format a currency amount for display with proper symbols.
	 * @param Currency The currency type
	 * @param Amount The amount to format
	 * @return Formatted string (e.g., "$1,234,567" or "500 Gold")
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Wallet")
	static FText FormatCurrency(EMGCurrencyType Currency, int64 Amount);

	/// @}

	// ==========================================
	// SHOP BROWSING
	// ==========================================

	/// @name Shop Browsing
	/// @brief Functions for discovering and filtering shop items.
	/// @{

	/**
	 * @brief Get all items in a specific category.
	 * @param Category The category to filter by
	 * @return Array of items in the specified category
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> GetItemsByCategory(EMGShopCategory Category) const;

	/**
	 * @brief Get all items marked as featured.
	 * @return Array of featured items across all categories
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> GetFeaturedItems() const;

	/**
	 * @brief Get all items marked as new.
	 * @return Array of recently added items
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> GetNewItems() const;

	/**
	 * @brief Get a specific item by its ID.
	 * @param ItemID The unique identifier of the item
	 * @return The item data, or empty struct if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	FMGShopItem GetItem(FName ItemID) const;

	/**
	 * @brief Search items by name or description.
	 * @param SearchTerm Text to search for (case-insensitive)
	 * @return Array of matching items
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> SearchItems(const FString& SearchTerm) const;

	/**
	 * @brief Get items compatible with a specific vehicle.
	 * @param VehicleID The vehicle to find compatible items for
	 * @param Category Filter to a specific category (optional)
	 * @return Array of compatible items
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Browse")
	TArray<FMGShopItem> GetItemsForVehicle(FName VehicleID, EMGShopCategory Category) const;

	/// @}

	// ==========================================
	// PURCHASING
	// ==========================================

	/// @name Purchase Operations
	/// @brief Functions for buying items from the shop.
	/// @{

	/**
	 * @brief Attempt to purchase an item.
	 * @param ItemID The item to purchase
	 * @param bUseAlternativePrice If true, use the alternative price if available
	 * @return Result code indicating success or failure reason
	 * @note Fires OnPurchaseComplete event
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Purchase")
	EMGPurchaseResult PurchaseItem(FName ItemID, bool bUseAlternativePrice = false);

	/**
	 * @brief Attempt to purchase a bundle.
	 * @param BundleID The bundle to purchase
	 * @return Result code indicating success or failure reason
	 * @note Fires OnPurchaseComplete event for each item in bundle
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Purchase")
	EMGPurchaseResult PurchaseBundle(FName BundleID);

	/**
	 * @brief Check if an item can be purchased without actually buying it.
	 * @param ItemID The item to check
	 * @param bUseAlternativePrice If true, check against alternative price
	 * @return Result code indicating whether purchase would succeed
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Purchase")
	EMGPurchaseResult CanPurchaseItem(FName ItemID, bool bUseAlternativePrice = false) const;

	/**
	 * @brief Get detailed information about what a purchase would grant.
	 * @param ItemID The item to preview
	 * @return Full item data with current player-specific status
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Purchase")
	FMGShopItem GetPurchasePreview(FName ItemID) const;

	/// @}

	// ==========================================
	// DAILY DEALS
	// ==========================================

	/// @name Daily Deals
	/// @brief Functions for the rotating daily deals system.
	/// @{

	/**
	 * @brief Get the current set of daily deals.
	 * @return Array of daily deals with discounted prices
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Deals")
	TArray<FMGDailyDeal> GetDailyDeals() const { return DailyDeals; }

	/**
	 * @brief Get time remaining until deals refresh.
	 * @return Time span until next refresh
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Deals")
	FTimespan GetTimeUntilDealsRefresh() const;

	/**
	 * @brief Force an immediate deals refresh (admin/debug only).
	 * @note This bypasses the normal 24-hour timer
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Deals")
	void ForceRefreshDeals();

	/// @}

	// ==========================================
	// BUNDLES
	// ==========================================

	/// @name Bundle Operations
	/// @brief Functions for browsing and purchasing bundles.
	/// @{

	/**
	 * @brief Get all currently available bundles.
	 * @return Array of bundle offerings
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Bundles")
	TArray<FMGBundleItem> GetAvailableBundles() const { return AvailableBundles; }

	/**
	 * @brief Get a specific bundle by its ID.
	 * @param BundleID The bundle to retrieve
	 * @return Bundle data, or empty struct if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Bundles")
	FMGBundleItem GetBundle(FName BundleID) const;

	/**
	 * @brief Get detailed item data for all items in a bundle.
	 * @param BundleID The bundle to inspect
	 * @return Array of full item data for bundle contents
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Bundles")
	TArray<FMGShopItem> GetBundleContents(FName BundleID) const;

	/// @}

	// ==========================================
	// OWNERSHIP
	// ==========================================

	/// @name Ownership Queries
	/// @brief Functions for checking item ownership status.
	/// @{

	/**
	 * @brief Check if the player owns a specific item.
	 * @param ItemID The item to check
	 * @return True if the player owns this item
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Ownership")
	bool IsItemOwned(FName ItemID) const;

	/**
	 * @brief Get list of all owned item IDs.
	 * @return Array of item IDs the player owns
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Ownership")
	TArray<FName> GetOwnedItems() const { return OwnedItems; }

	/**
	 * @brief Get owned items filtered by category.
	 * @param Category The category to filter by
	 * @return Array of owned items in the specified category
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Ownership")
	TArray<FMGShopItem> GetOwnedItemsInCategory(EMGShopCategory Category) const;

	/// @}

	// ==========================================
	// TRANSACTION HISTORY
	// ==========================================

	/// @name Transaction History
	/// @brief Functions for viewing past purchases.
	/// @{

	/**
	 * @brief Get transaction history.
	 * @param Count Maximum number of transactions to return
	 * @return Array of transactions, newest first
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|History")
	TArray<FMGTransaction> GetTransactionHistory(int32 Count = 50) const;

	/**
	 * @brief Get most recent purchases only.
	 * @param Count Maximum number of purchases to return
	 * @return Array of recent purchase transactions
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|History")
	TArray<FMGTransaction> GetRecentPurchases(int32 Count = 10) const;

	/// @}

	// ==========================================
	// UTILITY
	// ==========================================

	/// @name Utility Functions
	/// @brief Helper functions for display and formatting.
	/// @{

	/**
	 * @brief Get localized name for a rarity tier.
	 * @param Rarity Rarity level (0-4)
	 * @return Localized rarity name (e.g., "Legendary")
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Utility")
	static FText GetRarityName(int32 Rarity);

	/**
	 * @brief Get the display color for a rarity tier.
	 * @param Rarity Rarity level (0-4)
	 * @return Color for UI highlighting (gray to gold gradient)
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Utility")
	static FLinearColor GetRarityColor(int32 Rarity);

	/**
	 * @brief Get localized display name for a category.
	 * @param Category The category to get the name for
	 * @return Localized category name
	 */
	UFUNCTION(BlueprintPure, Category = "Shop|Utility")
	static FText GetCategoryDisplayName(EMGShopCategory Category);

	/// @}

protected:
	// ==========================================
	// DATA
	// ==========================================

	/// @name Data Storage
	/// @brief Internal data managed by the subsystem.
	/// @{

	/** Player's current wallet balances */
	UPROPERTY()
	FMGWallet Wallet;

	/** Master catalog of all shop items, keyed by ItemID */
	UPROPERTY()
	TMap<FName, FMGShopItem> AllItems;

	/** Set of item IDs the player currently owns */
	UPROPERTY()
	TArray<FName> OwnedItems;

	/** Current set of daily deals */
	UPROPERTY()
	TArray<FMGDailyDeal> DailyDeals;

	/** Currently available bundle offerings */
	UPROPERTY()
	TArray<FMGBundleItem> AvailableBundles;

	/** Historical record of all transactions */
	UPROPERTY()
	TArray<FMGTransaction> Transactions;

	/** Timestamp of the last daily deals refresh */
	FDateTime LastDealsRefresh;

	/** How often to check if deals need refreshing (seconds) */
	float DealsRefreshCheckInterval = 60.0f;

	/** Accumulator for refresh check timing */
	float DealsRefreshAccumulator = 0.0f;

	/// @}

	// ==========================================
	// INTERNAL
	// ==========================================

	/// @name Internal Operations
	/// @brief Private implementation functions.
	/// @{

	/** Load shop catalog and player data from save */
	void LoadShopData();

	/** Save player shop data (owned items, transactions) */
	void SaveShopData();

	/** Generate a new set of daily deals with random discounts */
	void GenerateDailyDeals();

	/** Check if it's time to refresh daily deals */
	void CheckDealsRefresh();

	/** Update computed fields on an item (owned, can afford, etc.) */
	void UpdateItemStatus(FMGShopItem& Item) const;

	/** Execute a purchase and create transaction record */
	FMGTransaction ProcessPurchase(const FMGShopItem& Item, const FMGItemPrice& Price);

	/** Add item to player's inventory after successful purchase */
	void GrantItem(FName ItemID);

	/** Create placeholder shop data for development/testing */
	void GenerateMockShopData();

	/** Calculate percentage discount between two prices */
	float CalculateSalePercent(int32 Original, int32 Current) const;

	/// @}
};

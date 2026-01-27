// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGInventorySubsystem.h
 * @brief Comprehensive inventory management system for items and vehicles.
 *
 * This subsystem manages all player-owned items and vehicles including:
 * - General items (cosmetics, consumables, blueprints, crates)
 * - Vehicle garage with detailed per-vehicle statistics
 * - Equipment slots for customizing vehicles
 * - Crate opening with configurable drop tables
 * - Crafting system with recipes
 * - Item selling for currency
 *
 * @section storage Storage Model
 * Items use a dual-key system:
 * - **ItemID (FName)**: The item template/definition (e.g., "spoiler_gt_wing")
 * - **ItemInstanceID (FGuid)**: Unique ID for each owned copy of an item
 *
 * This allows players to own multiple copies of stackable items while
 * maintaining individual state for each equipped/customized item.
 *
 * @section usage Basic Usage Example
 * @code
 * UMGInventorySubsystem* Inv = GetGameInstance()->GetSubsystem<UMGInventorySubsystem>();
 *
 * // Check if player has a specific item
 * if (Inv->HasItem(TEXT("turbo_stage2")))
 * {
 *     // Equip to current vehicle
 *     Inv->EquipItemToVehicle(CurrentVehicleID, ItemID, TEXT("Turbo"));
 * }
 *
 * // Browse garage
 * TArray<FMGVehicleInventoryEntry> AllCars = Inv->GetAllVehicles();
 * @endcode
 *
 * @see UMGShopSubsystem For purchasing new items
 * @see UMGCurrencySubsystem For selling items for currency
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGInventorySubsystem.generated.h"

// ============================================================================
// ENUMERATIONS - Item Classification
// ============================================================================

/**
 * @brief Categories of items that can exist in the inventory.
 *
 * Item types determine how items are displayed, filtered, equipped,
 * and what actions are available for them.
 */
UENUM(BlueprintType)
enum class EMGItemType : uint8
{
	/// @name Vehicles
	/// @{
	Vehicle,        ///< Complete vehicle (stored separately in garage)
	/// @}

	/// @name Performance Parts
	/// @{
	VehiclePart,    ///< Generic performance upgrade
	/// @}

	/// @name Visual Customization
	/// @{
	Cosmetic,       ///< General cosmetic item
	Livery,         ///< Full vehicle wrap/livery
	Decal,          ///< Individual decal/sticker
	Paint,          ///< Paint color/finish
	Wheels,         ///< Wheel/rim set
	Spoiler,        ///< Rear wing/spoiler
	BodyKit,        ///< Body kit/aero package
	Neon,           ///< Underglow lighting
	Interior,       ///< Interior customization
	Horn,           ///< Custom horn sound
	NitroEffect,    ///< Nitrous visual effect
	TrailEffect,    ///< Trail/particle effect
	/// @}

	/// @name Player Customization
	/// @{
	Badge,          ///< Profile badge/emblem
	Banner,         ///< Profile banner
	Avatar,         ///< Profile picture
	Title,          ///< Player title displayed with name
	Emote,          ///< In-game emote/animation
	/// @}

	/// @name Consumables & Special
	/// @{
	Currency,       ///< Currency bundle (from crates)
	Consumable,     ///< Single-use boost or item
	Blueprint,      ///< Crafting blueprint
	Crate,          ///< Unopened loot crate
	Key             ///< Key for opening specific crates
	/// @}
};

/**
 * @brief Rarity tiers that determine item value and visual treatment.
 *
 * Higher rarity items are rarer drops, worth more currency,
 * and have distinct UI colors/effects.
 */
UENUM(BlueprintType)
enum class EMGItemRarity : uint8
{
	Common,     ///< Gray - Most frequent drops
	Uncommon,   ///< Green - Slightly rare
	Rare,       ///< Blue - Notable items
	Epic,       ///< Purple - Desirable items
	Legendary,  ///< Gold - Very rare items
	Mythic,     ///< Red - Extremely rare
	Exclusive   ///< Platinum - Limited/event-only items
};

/**
 * @brief How the player acquired an item.
 *
 * Tracked for analytics and to display acquisition badges
 * (e.g., "Event Exclusive", "Crafted").
 */
UENUM(BlueprintType)
enum class EMGItemSource : uint8
{
	Unknown,    ///< Source not recorded
	Store,      ///< Purchased from shop
	Race,       ///< Earned as race reward
	Challenge,  ///< Challenge completion reward
	Achievement,///< Achievement unlock
	Crate,      ///< Dropped from loot crate
	Trade,      ///< Received via player trade
	Gift,       ///< Gift from developers
	Craft,      ///< Player crafted
	Event,      ///< Limited-time event reward
	SeasonPass, ///< Season pass tier reward
	Referral,   ///< Referral program reward
	Promotion,  ///< Promotional reward
	Legacy      ///< Migrated from previous version
};

/**
 * @brief Sort options for inventory browsing.
 */
UENUM(BlueprintType)
enum class EMGSortMethod : uint8
{
	DateAcquired,       ///< Oldest first
	DateAcquiredDesc,   ///< Newest first (default)
	Name,               ///< Alphabetical A-Z
	NameDesc,           ///< Alphabetical Z-A
	Rarity,             ///< Common to Exclusive
	RarityDesc,         ///< Exclusive to Common
	Type,               ///< Grouped by item type
	Value,              ///< Lowest sell value first
	ValueDesc,          ///< Highest sell value first
	Favorite            ///< Favorites first, then by date
};

// ============================================================================
// DATA STRUCTURES - Inventory Items
// ============================================================================

/**
 * @brief Complete data for a single inventory item instance.
 *
 * Each owned item has its own instance with unique ID, allowing
 * individual state tracking for equipped items and stacks.
 */
USTRUCT(BlueprintType)
struct FMGInventoryItem
{
	GENERATED_BODY()

	/// @name Identity
	/// @{

	/** Unique ID for this specific item instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ItemInstanceID;

	/** Item template/definition ID (e.g., "spoiler_gt_wing") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	/** Localized display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Localized description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Item category for filtering and behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemType ItemType = EMGItemType::Cosmetic;

	/** Rarity tier for visuals and value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemRarity Rarity = EMGItemRarity::Common;

	/** How this item was acquired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemSource Source = EMGItemSource::Unknown;

	/// @}

	/// @name Stack Properties
	/// @{

	/** Current quantity in this stack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	/** Maximum stack size (1 = non-stackable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack = 1;

	/** Whether this item type can stack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStackable = false;

	/// @}

	/// @name Trade & Sell Properties
	/// @{

	/** Whether this item can be traded to other players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTradeable = true;

	/** Whether this item can be sold for currency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSellable = true;

	/** Grind Cash value when sold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SellValue = 0;

	/// @}

	/// @name User Preferences
	/// @{

	/** Player marked as favorite */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFavorite = false;

	/** Item has not been viewed yet (show "NEW" badge) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNew = true;

	/** Currently equipped to a vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEquipped = false;

	/** Locked by player to prevent accidental sale/trade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLocked = false;

	/// @}

	/// @name Dates & Expiration
	/// @{

	/** When the item was added to inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AcquiredAt;

	/** When the item expires (if bExpires is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	/** Whether this item has an expiration date */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExpires = false;

	/// @}

	/// @name Visual Assets
	/// @{

	/** Icon texture for inventory display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	/** 3D mesh for preview rendering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> PreviewMesh;

	/// @}

	/// @name Compatibility
	/// @{

	/** Vehicle this item is compatible with (empty = universal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompatibleVehicle;

	/** Tags for filtering and categorization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tags;

	/** Arbitrary key-value data for extensibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomData;

	/// @}
};

// ============================================================================
// DATA STRUCTURES - Vehicle Inventory
// ============================================================================

/**
 * @brief Complete data for an owned vehicle in the garage.
 *
 * Vehicles are stored separately from items with their own
 * statistics, customization state, and equipment slots.
 */
USTRUCT(BlueprintType)
struct FMGVehicleInventoryEntry
{
	GENERATED_BODY()

	/// @name Identity
	/// @{

	/** Unique ID for this specific vehicle instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleInstanceID;

	/** Vehicle template/definition ID (e.g., "nissan_skyline_r34") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/** Base vehicle display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VehicleName;

	/** Player-assigned custom name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomName;

	/** Vehicle rarity (affects base value and visuals) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemRarity Rarity = EMGItemRarity::Common;

	/** How this vehicle was acquired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemSource Source = EMGItemSource::Unknown;

	/** When the vehicle was added to garage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AcquiredAt;

	/// @}

	/// @name Statistics
	/// @{

	/** Total races completed with this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	/** Total race wins with this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalWins = 0;

	/** Total distance driven in kilometers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 0.0f;

	/** Highest speed achieved in km/h */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeed = 0.0f;

	/** Best lap time in seconds (0 = not set) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	/// @}

	/// @name User Preferences
	/// @{

	/** Player marked as favorite */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFavorite = false;

	/** Whether this vehicle can be traded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTradeable = true;

	/** Locked to prevent accidental sale/trade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLocked = false;

	/// @}

	/// @name Customization State
	/// @{

	/** Map of slot name to equipped item instance ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FGuid> EquippedParts;

	/** Currently applied livery ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EquippedLivery;

	/** Primary body color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	/** Secondary/accent color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::Black;

	/// @}

	/// @name Performance & Classification
	/// @{

	/** Garage thumbnail image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Thumbnail;

	/** Performance Index rating (for matchmaking) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerformanceIndex = 0;

	/** Vehicle class for race restrictions (e.g., "S", "A", "B") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleClass;

	/// @}
};

// ============================================================================
// DATA STRUCTURES - Filters & Sorting
// ============================================================================

/**
 * @brief Filter and sort parameters for inventory queries.
 *
 * Used by UI to display filtered/sorted inventory views.
 */
USTRUCT(BlueprintType)
struct FMGInventoryFilter
{
	GENERATED_BODY()

	/// @name Type Filters
	/// @{

	/** Only include these item types (empty = all types) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGItemType> AllowedTypes;

	/** Only include these rarities (empty = all rarities) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGItemRarity> AllowedRarities;

	/** Items must have ALL of these tags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiredTags;

	/** Only show items compatible with this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompatibleVehicle;

	/// @}

	/// @name Boolean Filters
	/// @{

	/** Only tradeable items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyTradeable = false;

	/** Only sellable items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlySellable = false;

	/** Only favorite items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyFavorites = false;

	/** Only new (unviewed) items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyNew = false;

	/** Exclude currently equipped items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideEquipped = false;

	/// @}

	/// @name Search & Sort
	/// @{

	/** Text search in name/description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SearchText;

	/** How to sort the results */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSortMethod SortMethod = EMGSortMethod::DateAcquiredDesc;

	/// @}
};

// ============================================================================
// DATA STRUCTURES - Crates & Crafting
// ============================================================================

/**
 * @brief Defines the contents and drop rates of a loot crate type.
 *
 * Crates contain random items with configurable rarity weights
 * and pity/guarantee systems.
 */
USTRUCT(BlueprintType)
struct FMGCrateContents
{
	GENERATED_BODY()

	/** Unique identifier for this crate type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrateID;

	/** Display name of the crate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CrateName;

	/** All possible items that can drop from this crate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGInventoryItem> PossibleItems;

	/** Drop weight for each rarity tier (higher = more common) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGItemRarity, float> RarityWeights;

	/** After this many opens without the guaranteed rarity, force a drop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GuaranteedRarityAtCount = 10;

	/** Rarity that is guaranteed after GuaranteedRarityAtCount opens */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemRarity GuaranteedRarity = EMGItemRarity::Rare;
};

/**
 * @brief Recipe for crafting new items from existing items.
 */
USTRUCT(BlueprintType)
struct FMGCraftingRecipe
{
	GENERATED_BODY()

	/** Unique identifier for this recipe */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RecipeID;

	/** Display name of the recipe */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RecipeName;

	/** Required items and quantities (ItemID -> Count) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> RequiredItems;

	/** Grind Cash cost to craft */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyCost = 0;

	/** Item ID that will be created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ResultItemID;

	/** How many of the result item are created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ResultQuantity = 1;

	/** Chance of success (1.0 = always succeeds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SuccessRate = 1.0f;

	/** Whether this recipe has been unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;
};

// ============================================================================
// DATA STRUCTURES - Statistics
// ============================================================================

/**
 * @brief Aggregate statistics about the player's inventory.
 */
USTRUCT(BlueprintType)
struct FMGInventoryStats
{
	GENERATED_BODY()

	/** Total item count (sum of all stack quantities) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalItems = 0;

	/** Number of vehicles in garage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalVehicles = 0;

	/** Count of unique item definitions owned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UniqueItems = 0;

	/** Item count breakdown by type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGItemType, int32> ItemsByType;

	/** Item count breakdown by rarity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGItemRarity, int32> ItemsByRarity;

	/** Combined sell value of all items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalValue = 0;

	/** Number of unopened crates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CratesOwned = 0;

	/** Lifetime crates opened */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CratesOpened = 0;

	/** Lifetime items crafted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemsCrafted = 0;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/// @brief Fired when an item is added to inventory
/// @param Item The added item
/// @param Quantity How many were added
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, const FMGInventoryItem&, Item, int32, Quantity);

/// @brief Fired when an item is removed from inventory
/// @param Item The removed item
/// @param Quantity How many were removed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, const FMGInventoryItem&, Item, int32, Quantity);

/// @brief Fired when a vehicle is added to the garage
/// @param Vehicle The added vehicle
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleAdded, const FMGVehicleInventoryEntry&, Vehicle);

/// @brief Fired when a vehicle is removed from the garage
/// @param VehicleInstanceID ID of the removed vehicle
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleRemoved, FGuid, VehicleInstanceID);

/// @brief Fired when an item is equipped to a vehicle
/// @param VehicleInstanceID The vehicle that received the item
/// @param Item The equipped item
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, FGuid, VehicleInstanceID, const FMGInventoryItem&, Item);

/// @brief Fired when a crate is opened
/// @param CrateID Type of crate opened
/// @param Rewards Items received from the crate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrateOpened, FName, CrateID, const TArray<FMGInventoryItem>&, Rewards);

/// @brief Fired when an item is crafted
/// @param RecipeID The recipe used
/// @param Result The crafted item
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemCrafted, FName, RecipeID, const FMGInventoryItem&, Result);

/// @brief Fired when inventory capacity is reached
/// @param ItemType The type of item that couldn't be added
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryFull, EMGItemType, ItemType);

// ============================================================================
// INVENTORY SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Central subsystem for managing player inventory and vehicle garage.
 *
 * The Inventory Subsystem provides comprehensive item and vehicle management:
 *
 * @section items Item Management
 * - Add/remove items with automatic stacking
 * - Filter and sort with flexible query system
 * - Track acquisition source and dates
 * - Support for tradeable, sellable, and locked items
 *
 * @section vehicles Vehicle Garage
 * - Separate storage for owned vehicles
 * - Per-vehicle statistics (races, wins, distance)
 * - Customization state persistence
 * - Performance index tracking
 *
 * @section equipment Equipment System
 * - Named equipment slots per vehicle
 * - Compatibility checking
 * - Automatic unequip on sell/trade
 *
 * @section crates Crate System
 * - Configurable drop tables
 * - Pity system for guaranteed drops
 * - Opening animations (via events)
 *
 * @section crafting Crafting System
 * - Unlockable recipes
 * - Material consumption
 * - Success rate mechanics
 *
 * @note Item capacity and garage size are configurable to prevent
 *       unlimited hoarding while remaining generous.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// @name Lifecycle
	/// @{

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Determines if subsystem should be created (always true for this subsystem) */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/// @}

	// ==========================================
	// ITEM MANAGEMENT
	// ==========================================

	/// @name Item Management
	/// @brief Core functions for adding, removing, and querying items.
	/// @{

	/**
	 * @brief Add an item to the inventory.
	 * @param Item The item to add
	 * @param Quantity How many to add (for stackable items)
	 * @return True if successfully added
	 * @note Fires OnItemAdded event
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool AddItem(const FMGInventoryItem& Item, int32 Quantity = 1);

	/**
	 * @brief Remove an item by its instance ID.
	 * @param ItemInstanceID Unique ID of the item instance
	 * @param Quantity How many to remove from stack
	 * @return True if successfully removed
	 * @note Fires OnItemRemoved event
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool RemoveItem(FGuid ItemInstanceID, int32 Quantity = 1);

	/**
	 * @brief Remove items by template ID.
	 * @param ItemID The item definition ID
	 * @param Quantity How many to remove
	 * @return True if enough items were removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool RemoveItemByID(FName ItemID, int32 Quantity = 1);

	/**
	 * @brief Check if player has enough of an item.
	 * @param ItemID The item definition ID
	 * @param Quantity Minimum quantity required
	 * @return True if player has at least Quantity of the item
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	bool HasItem(FName ItemID, int32 Quantity = 1) const;

	/**
	 * @brief Get total count of an item type.
	 * @param ItemID The item definition ID
	 * @return Combined quantity across all stacks
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	int32 GetItemCount(FName ItemID) const;

	/**
	 * @brief Get item by its instance ID.
	 * @param ItemInstanceID Unique ID of the item instance
	 * @return Item data, or empty struct if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	FMGInventoryItem GetItem(FGuid ItemInstanceID) const;

	/**
	 * @brief Get all item instances of a specific type.
	 * @param ItemID The item definition ID
	 * @return Array of all matching item instances
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	TArray<FMGInventoryItem> GetItemsByID(FName ItemID) const;

	/**
	 * @brief Get complete item list (unfiltered).
	 * @return All items in inventory
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	TArray<FMGInventoryItem> GetAllItems() const { return Items; }

	/**
	 * @brief Get items matching filter criteria.
	 * @param Filter Filter and sort parameters
	 * @return Filtered and sorted item array
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	TArray<FMGInventoryItem> GetFilteredItems(const FMGInventoryFilter& Filter) const;

	/**
	 * @brief Mark an item as viewed (remove "NEW" badge).
	 * @param ItemInstanceID Unique ID of the item instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void MarkItemViewed(FGuid ItemInstanceID);

	/**
	 * @brief Mark all items as viewed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void MarkAllViewed();

	/**
	 * @brief Toggle favorite status on an item.
	 * @param ItemInstanceID Unique ID of the item instance
	 * @param bFavorite New favorite status
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void SetItemFavorite(FGuid ItemInstanceID, bool bFavorite);

	/**
	 * @brief Toggle lock status on an item.
	 * @param ItemInstanceID Unique ID of the item instance
	 * @param bLocked New lock status (locked items cannot be sold/traded)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void SetItemLocked(FGuid ItemInstanceID, bool bLocked);

	/// @}

	// ==========================================
	// VEHICLE MANAGEMENT
	// ==========================================

	/// @name Vehicle Management
	/// @brief Functions for the vehicle garage.
	/// @{

	/**
	 * @brief Add a vehicle to the garage.
	 * @param Vehicle The vehicle to add
	 * @return True if successfully added
	 * @note Fires OnVehicleAdded event
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	bool AddVehicle(const FMGVehicleInventoryEntry& Vehicle);

	/**
	 * @brief Remove a vehicle from the garage.
	 * @param VehicleInstanceID Unique ID of the vehicle
	 * @return True if successfully removed
	 * @note Fires OnVehicleRemoved event
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	bool RemoveVehicle(FGuid VehicleInstanceID);

	/**
	 * @brief Check if player owns a specific vehicle type.
	 * @param VehicleID The vehicle definition ID
	 * @return True if player owns at least one
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Vehicles")
	bool HasVehicle(FName VehicleID) const;

	/**
	 * @brief Get vehicle by its instance ID.
	 * @param VehicleInstanceID Unique ID of the vehicle
	 * @return Vehicle data, or empty struct if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Vehicles")
	FMGVehicleInventoryEntry GetVehicle(FGuid VehicleInstanceID) const;

	/**
	 * @brief Get all vehicles in the garage.
	 * @return Array of all owned vehicles
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Vehicles")
	TArray<FMGVehicleInventoryEntry> GetAllVehicles() const { return Vehicles; }

	/**
	 * @brief Get vehicles of a specific class.
	 * @param VehicleClass Class identifier (e.g., "S", "A")
	 * @return Vehicles matching the class
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Vehicles")
	TArray<FMGVehicleInventoryEntry> GetVehiclesByClass(FName VehicleClass) const;

	/**
	 * @brief Toggle favorite status on a vehicle.
	 * @param VehicleInstanceID Unique ID of the vehicle
	 * @param bFavorite New favorite status
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	void SetVehicleFavorite(FGuid VehicleInstanceID, bool bFavorite);

	/**
	 * @brief Toggle lock status on a vehicle.
	 * @param VehicleInstanceID Unique ID of the vehicle
	 * @param bLocked New lock status
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	void SetVehicleLocked(FGuid VehicleInstanceID, bool bLocked);

	/**
	 * @brief Set custom display name for a vehicle.
	 * @param VehicleInstanceID Unique ID of the vehicle
	 * @param CustomName Player-chosen name
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	void SetVehicleCustomName(FGuid VehicleInstanceID, const FString& CustomName);

	/**
	 * @brief Update vehicle statistics after a race.
	 * @param VehicleInstanceID Unique ID of the vehicle
	 * @param Distance Distance driven in km
	 * @param TopSpeed Highest speed achieved
	 * @param bWon Whether the race was won
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	void UpdateVehicleStats(FGuid VehicleInstanceID, float Distance, float TopSpeed, bool bWon);

	/// @}

	// ==========================================
	// EQUIPMENT
	// ==========================================

	/// @name Equipment
	/// @brief Functions for equipping items to vehicles.
	/// @{

	/**
	 * @brief Equip an item to a vehicle slot.
	 * @param VehicleInstanceID Vehicle to equip to
	 * @param ItemInstanceID Item to equip
	 * @param SlotName Equipment slot (e.g., "Turbo", "Spoiler")
	 * @return True if successfully equipped
	 * @note Fires OnItemEquipped event
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItemToVehicle(FGuid VehicleInstanceID, FGuid ItemInstanceID, FName SlotName);

	/**
	 * @brief Unequip an item from a vehicle slot.
	 * @param VehicleInstanceID Vehicle to unequip from
	 * @param SlotName Equipment slot to clear
	 * @return True if something was unequipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnequipFromVehicle(FGuid VehicleInstanceID, FName SlotName);

	/**
	 * @brief Get the item equipped in a specific slot.
	 * @param VehicleInstanceID Vehicle to check
	 * @param SlotName Equipment slot to query
	 * @return Equipped item, or empty struct if slot is empty
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Equipment")
	FMGInventoryItem GetEquippedItem(FGuid VehicleInstanceID, FName SlotName) const;

	/**
	 * @brief Get all equipped items on a vehicle.
	 * @param VehicleInstanceID Vehicle to query
	 * @return Map of slot name to equipped item
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Equipment")
	TMap<FName, FMGInventoryItem> GetAllEquippedItems(FGuid VehicleInstanceID) const;

	/**
	 * @brief Get items compatible with a vehicle slot.
	 * @param VehicleInstanceID Vehicle to check compatibility for
	 * @param SlotName Equipment slot type
	 * @return Array of compatible unequipped items
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Equipment")
	TArray<FMGInventoryItem> GetCompatibleItems(FGuid VehicleInstanceID, FName SlotName) const;

	/// @}

	// ==========================================
	// CRATES
	// ==========================================

	/// @name Crates
	/// @brief Functions for the loot crate system.
	/// @{

	/**
	 * @brief Open a crate and receive rewards.
	 * @param CrateID Type of crate to open (must be owned)
	 * @return Array of items received
	 * @note Fires OnCrateOpened event
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Crates")
	TArray<FMGInventoryItem> OpenCrate(FName CrateID);

	/**
	 * @brief Check if player can open a crate type.
	 * @param CrateID Type of crate
	 * @return True if player owns at least one
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Crates")
	bool CanOpenCrate(FName CrateID) const;

	/**
	 * @brief Get the contents/drop table of a crate type.
	 * @param CrateID Type of crate
	 * @return Crate definition with possible drops
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Crates")
	FMGCrateContents GetCrateContents(FName CrateID) const;

	/**
	 * @brief Register a new crate type definition.
	 * @param CrateContents The crate definition to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Crates")
	void RegisterCrateType(const FMGCrateContents& CrateContents);

	/// @}

	// ==========================================
	// CRAFTING
	// ==========================================

	/// @name Crafting
	/// @brief Functions for the item crafting system.
	/// @{

	/**
	 * @brief Craft an item using a recipe.
	 * @param RecipeID Recipe to use
	 * @return True if crafting succeeded
	 * @note Consumes materials and currency on success
	 * @note Fires OnItemCrafted event on success
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Crafting")
	bool CraftItem(FName RecipeID);

	/**
	 * @brief Check if player has materials to craft.
	 * @param RecipeID Recipe to check
	 * @return True if all requirements are met
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Crafting")
	bool CanCraftItem(FName RecipeID) const;

	/**
	 * @brief Get all recipes the player can use.
	 * @return Array of unlocked recipes
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Crafting")
	TArray<FMGCraftingRecipe> GetAvailableRecipes() const;

	/**
	 * @brief Get a specific recipe by ID.
	 * @param RecipeID Recipe to retrieve
	 * @return Recipe data, or empty struct if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Crafting")
	FMGCraftingRecipe GetRecipe(FName RecipeID) const;

	/**
	 * @brief Unlock a crafting recipe for the player.
	 * @param RecipeID Recipe to unlock
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Crafting")
	void UnlockRecipe(FName RecipeID);

	/// @}

	// ==========================================
	// SELLING
	// ==========================================

	/// @name Selling
	/// @brief Functions for selling items for currency.
	/// @{

	/**
	 * @brief Sell an item for Grind Cash.
	 * @param ItemInstanceID Item to sell
	 * @param Quantity How many to sell from stack
	 * @return True if sold successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Selling")
	bool SellItem(FGuid ItemInstanceID, int32 Quantity = 1);

	/**
	 * @brief Sell a vehicle for Grind Cash.
	 * @param VehicleInstanceID Vehicle to sell
	 * @return True if sold successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Selling")
	bool SellVehicle(FGuid VehicleInstanceID);

	/**
	 * @brief Get the sell price of an item.
	 * @param ItemInstanceID Item to price
	 * @param Quantity Quantity to price
	 * @return Total sell value in Grind Cash
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Selling")
	int32 GetSellPrice(FGuid ItemInstanceID, int32 Quantity = 1) const;

	/**
	 * @brief Get the sell price of a vehicle.
	 * @param VehicleInstanceID Vehicle to price
	 * @return Sell value in Grind Cash
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Selling")
	int32 GetVehicleSellPrice(FGuid VehicleInstanceID) const;

	/// @}

	// ==========================================
	// STATS
	// ==========================================

	/// @name Statistics
	/// @brief Functions for inventory statistics and capacity.
	/// @{

	/**
	 * @brief Get aggregate inventory statistics.
	 * @return Current inventory stats
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	FMGInventoryStats GetInventoryStats() const { return Stats; }

	/**
	 * @brief Get maximum inventory capacity.
	 * @return Maximum number of item slots
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	int32 GetInventoryCapacity() const { return MaxInventorySlots; }

	/**
	 * @brief Get number of used inventory slots.
	 * @return Current slot usage
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	int32 GetUsedInventorySlots() const;

	/**
	 * @brief Check if inventory is at capacity.
	 * @return True if no more items can be added
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	bool IsInventoryFull() const;

	/**
	 * @brief Get maximum garage capacity.
	 * @return Maximum number of vehicles
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	int32 GetGarageCapacity() const { return MaxGarageSlots; }

	/**
	 * @brief Get count of items with "NEW" badge.
	 * @return Number of unviewed items
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	int32 GetNewItemCount() const;

	/// @}

	// ==========================================
	// EVENTS
	// ==========================================

	/// @name Event Delegates
	/// @brief Subscribe to react to inventory changes.
	/// @{

	/** Broadcast when an item is added */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemAdded OnItemAdded;

	/** Broadcast when an item is removed */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemRemoved OnItemRemoved;

	/** Broadcast when a vehicle is added to garage */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnVehicleAdded OnVehicleAdded;

	/** Broadcast when a vehicle is removed from garage */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnVehicleRemoved OnVehicleRemoved;

	/** Broadcast when an item is equipped to a vehicle */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemEquipped OnItemEquipped;

	/** Broadcast when a crate is opened */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnCrateOpened OnCrateOpened;

	/** Broadcast when an item is crafted */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemCrafted OnItemCrafted;

	/** Broadcast when inventory reaches capacity */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryFull OnInventoryFull;

	/// @}

protected:
	/// @name Internal Operations
	/// @{

	/** Recalculate inventory statistics */
	void UpdateStats();

	/** Save inventory to persistent storage */
	void SaveInventory();

	/** Load inventory from persistent storage */
	void LoadInventory();

	/** Roll random reward from a crate using weighted odds */
	FMGInventoryItem RollCrateReward(const FMGCrateContents& Crate) const;

	/** Get number of times a specific crate type has been opened */
	int32 GetCrateOpenCount(FName CrateID) const;

	/// @}

	/// @name Data Storage
	/// @{

	/** All owned items */
	UPROPERTY()
	TArray<FMGInventoryItem> Items;

	/** All owned vehicles */
	UPROPERTY()
	TArray<FMGVehicleInventoryEntry> Vehicles;

	/** Registered crate type definitions */
	UPROPERTY()
	TMap<FName, FMGCrateContents> CrateTypes;

	/** Registered crafting recipes */
	UPROPERTY()
	TMap<FName, FMGCraftingRecipe> Recipes;

	/** Tracks how many times each crate type has been opened (for pity) */
	UPROPERTY()
	TMap<FName, int32> CrateOpenCounts;

	/** Cached inventory statistics */
	UPROPERTY()
	FMGInventoryStats Stats;

	/** Maximum item slots (configurable) */
	UPROPERTY()
	int32 MaxInventorySlots = 500;

	/** Maximum vehicle garage slots (configurable) */
	UPROPERTY()
	int32 MaxGarageSlots = 50;

	/// @}
};

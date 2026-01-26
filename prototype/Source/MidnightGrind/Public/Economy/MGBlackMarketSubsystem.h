// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBlackMarketSubsystem.generated.h"

class UTexture2D;

/**
 * Black market access tier
 * Higher tiers require more heat/rep to access
 */
UENUM(BlueprintType)
enum class EMGBlackMarketTier : uint8
{
	/** Basic underground - low risk items */
	Street			UMETA(DisplayName = "Street Level"),
	/** Serious contraband - requires established rep */
	Underground		UMETA(DisplayName = "Underground"),
	/** Elite connections - high heat requirement */
	Shadow			UMETA(DisplayName = "Shadow Network"),
	/** Legendary dealer - max rep, pink slip winners only */
	Phantom			UMETA(DisplayName = "Phantom Circle")
};

/**
 * Part rarity tier
 */
UENUM(BlueprintType)
enum class EMGPartRarity : uint8
{
	/** Standard parts - available everywhere */
	Common			UMETA(DisplayName = "Common"),
	/** Better quality - slightly rarer */
	Uncommon		UMETA(DisplayName = "Uncommon"),
	/** Hard to find - specialty shops only */
	Rare			UMETA(DisplayName = "Rare"),
	/** Exceptional quality - black market only */
	Epic			UMETA(DisplayName = "Epic"),
	/** One of a kind - legendary status */
	Legendary		UMETA(DisplayName = "Legendary")
};

/**
 * Black market dealer personality types
 */
UENUM(BlueprintType)
enum class EMGDealerPersonality : uint8
{
	/** Fair prices, reliable */
	Professional	UMETA(DisplayName = "Professional"),
	/** Cheap but unreliable */
	Shady			UMETA(DisplayName = "Shady"),
	/** Premium prices, best quality */
	Elite			UMETA(DisplayName = "Elite"),
	/** Random prices, chaotic */
	Wildcard		UMETA(DisplayName = "Wildcard")
};

/**
 * Black market dealer
 */
USTRUCT(BlueprintType)
struct FMGBlackMarketDealer
{
	GENERATED_BODY()

	/** Unique dealer ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DealerID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	/** Dealer nickname */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Nickname;

	/** Description/bio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Portrait image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Access tier required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBlackMarketTier RequiredTier = EMGBlackMarketTier::Street;

	/** Personality affects prices and reliability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDealerPersonality Personality = EMGDealerPersonality::Professional;

	/** Specialization (Turbo, Engine, Suspension, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Specialization;

	/** Price multiplier (1.0 = normal, 0.8 = 20% discount, 1.3 = 30% markup) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PriceMultiplier = 1.0f;

	/** Chance of having rare items (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RareItemChance = 0.1f;

	/** Chance of deal going bad (counterfeit parts, police sting) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RiskFactor = 0.05f;

	/** Player's trust level with this dealer (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrustLevel = 0;

	/** Total transactions with this dealer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTransactions = 0;

	/** Location in the world */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MeetLocation = FVector::ZeroVector;

	/** Time window available (24h format, -1 = always) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AvailableHourStart = 22; // 10 PM

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AvailableHourEnd = 4; // 4 AM

	/** Currently available */
	UPROPERTY(BlueprintReadOnly)
	bool bIsAvailable = false;
};

/**
 * Rare part listing
 */
USTRUCT(BlueprintType)
struct FMGRarePart
{
	GENERATED_BODY()

	/** Base part ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BasePartID;

	/** Unique variant ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VariantID;

	/** Display name override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Rarity tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPartRarity Rarity = EMGPartRarity::Rare;

	/** Stat bonus percentage (5% for rare, 10% for epic, 15% for legendary) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StatBonus = 0.05f;

	/** Price multiplier over base part */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PriceMultiplier = 2.0f;

	/** Source of this part */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SourceID; // "BlackMarket", "PinkSlip", "Tournament", "Collector"

	/** Is this a one-of-a-kind item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExclusive = false;

	/** Current stock (-1 = unlimited, specific number = limited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stock = -1;

	/** Description of what makes this part special */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FlavorText;

	/** Visual distinction (gold plating, carbon fiber, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VisualVariant;
};

/**
 * Black market inventory item
 */
USTRUCT(BlueprintType)
struct FMGBlackMarketItem
{
	GENERATED_BODY()

	/** The rare part */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRarePart Part;

	/** Dealer selling this */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DealerID;

	/** Current price (after dealer multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Price = 0;

	/** Time this listing expires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	/** Is this a "hot" item (stolen, traced) - higher risk */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHot = false;

	/** Heat gained if caught with this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeatIfCaught = 50;
};

/**
 * Purchase result for black market
 */
UENUM(BlueprintType)
enum class EMGBlackMarketResult : uint8
{
	/** Purchase successful */
	Success,
	/** Not enough money */
	InsufficientFunds,
	/** Access tier too low */
	AccessDenied,
	/** Dealer not available */
	DealerUnavailable,
	/** Item no longer in stock */
	OutOfStock,
	/** Deal went bad - counterfeit part */
	Counterfeit,
	/** Deal went bad - police sting */
	PoliceSting,
	/** Trust level too low */
	InsufficientTrust
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlackMarketPurchase, const FMGBlackMarketItem&, Item, EMGBlackMarketResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDealerTrustChanged, FName, DealerID, int32, NewTrustLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackMarketTierUnlocked, EMGBlackMarketTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRarePartDiscovered, const FMGRarePart&, Part);

/**
 * @class UMGBlackMarketSubsystem
 * @brief Underground parts market with risk/reward mechanics
 *
 * The Black Market provides access to rare, high-performance parts
 * that aren't available through legitimate channels. Access is gated
 * by heat level, reputation, and dealer trust.
 *
 * Features:
 * - Tiered access based on criminal reputation
 * - Multiple dealers with different specialties and personalities
 * - Risk mechanics (counterfeit parts, police stings)
 * - Trust building with dealers for better prices
 * - Exclusive legendary parts
 * - Time-limited inventory that rotates
 */
UCLASS()
class MIDNIGHTGRIND_API UMGBlackMarketSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ========================================================================
	// Access Control
	// ========================================================================

	/**
	 * Get current black market access tier
	 * @return Current access tier based on heat and rep
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Access")
	EMGBlackMarketTier GetAccessTier() const;

	/**
	 * Check if player can access a specific tier
	 * @param Tier Tier to check
	 * @return True if accessible
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Access")
	bool CanAccessTier(EMGBlackMarketTier Tier) const;

	/**
	 * Get requirements for a tier
	 * @param Tier Tier to check
	 * @param OutMinHeat Minimum heat level required
	 * @param OutMinRep Minimum reputation required
	 * @param OutMinPinkSlipWins Minimum pink slip wins required
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Access")
	void GetTierRequirements(EMGBlackMarketTier Tier, int32& OutMinHeat, int32& OutMinRep, int32& OutMinPinkSlipWins) const;

	// ========================================================================
	// Dealers
	// ========================================================================

	/**
	 * Get all known dealers
	 * @return Array of all dealers (even if not accessible)
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Dealers")
	TArray<FMGBlackMarketDealer> GetAllDealers() const { return Dealers; }

	/**
	 * Get dealers available to player at current tier
	 * @return Array of accessible dealers
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Dealers")
	TArray<FMGBlackMarketDealer> GetAvailableDealers() const;

	/**
	 * Get a specific dealer by ID
	 * @param DealerID Dealer to find
	 * @return Dealer data (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Dealers")
	FMGBlackMarketDealer GetDealer(FName DealerID) const;

	/**
	 * Get trust level with a dealer
	 * @param DealerID Dealer to check
	 * @return Trust level (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Dealers")
	int32 GetDealerTrust(FName DealerID) const;

	/**
	 * Build trust with a dealer through successful transactions
	 * @param DealerID Dealer ID
	 * @param Amount Trust to add
	 */
	UFUNCTION(BlueprintCallable, Category = "BlackMarket|Dealers")
	void AddDealerTrust(FName DealerID, int32 Amount);

	// ========================================================================
	// Inventory
	// ========================================================================

	/**
	 * Get current black market inventory from a dealer
	 * @param DealerID Dealer to query
	 * @return Array of available items
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Inventory")
	TArray<FMGBlackMarketItem> GetDealerInventory(FName DealerID) const;

	/**
	 * Get all rare parts across all dealers
	 * @return All available rare parts
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Inventory")
	TArray<FMGBlackMarketItem> GetAllAvailableItems() const;

	/**
	 * Refresh a dealer's inventory (happens automatically on timer)
	 * @param DealerID Dealer to refresh
	 */
	UFUNCTION(BlueprintCallable, Category = "BlackMarket|Inventory")
	void RefreshDealerInventory(FName DealerID);

	// ========================================================================
	// Purchases
	// ========================================================================

	/**
	 * Attempt to purchase a black market item
	 * @param DealerID Dealer to buy from
	 * @param PartVariantID Part variant to purchase
	 * @return Purchase result
	 */
	UFUNCTION(BlueprintCallable, Category = "BlackMarket|Purchase")
	EMGBlackMarketResult PurchaseItem(FName DealerID, FName PartVariantID);

	/**
	 * Get the risk level for a potential purchase
	 * @param DealerID Dealer
	 * @param PartVariantID Part
	 * @return Risk percentage (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Purchase")
	float GetPurchaseRisk(FName DealerID, FName PartVariantID) const;

	/**
	 * Check if a purchase would trigger heat
	 * @param Item Item to check
	 * @return Heat that would be added if caught
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|Purchase")
	int32 GetPotentialHeat(const FMGBlackMarketItem& Item) const;

	// ========================================================================
	// Rare Parts
	// ========================================================================

	/**
	 * Get all discovered rare parts (even if not owned)
	 * @return Array of discovered rare parts
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|RareParts")
	TArray<FMGRarePart> GetDiscoveredRareParts() const { return DiscoveredRareParts; }

	/**
	 * Get owned rare parts
	 * @return Array of owned rare parts
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|RareParts")
	TArray<FMGRarePart> GetOwnedRareParts() const { return OwnedRareParts; }

	/**
	 * Check if player owns a specific rare part
	 * @param VariantID Part variant to check
	 * @return True if owned
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|RareParts")
	bool OwnsRarePart(FName VariantID) const;

	/**
	 * Get stat bonus for a rare part
	 * @param Rarity Rarity tier
	 * @return Stat bonus multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|RareParts")
	static float GetRarityStatBonus(EMGPartRarity Rarity);

	/**
	 * Get display color for rarity
	 * @param Rarity Rarity tier
	 * @return Display color
	 */
	UFUNCTION(BlueprintPure, Category = "BlackMarket|RareParts")
	static FLinearColor GetRarityColor(EMGPartRarity Rarity);

	// ========================================================================
	// Events
	// ========================================================================

	/** Fired when a black market purchase is made */
	UPROPERTY(BlueprintAssignable, Category = "BlackMarket|Events")
	FOnBlackMarketPurchase OnBlackMarketPurchase;

	/** Fired when dealer trust changes */
	UPROPERTY(BlueprintAssignable, Category = "BlackMarket|Events")
	FOnDealerTrustChanged OnDealerTrustChanged;

	/** Fired when a new tier is unlocked */
	UPROPERTY(BlueprintAssignable, Category = "BlackMarket|Events")
	FOnBlackMarketTierUnlocked OnBlackMarketTierUnlocked;

	/** Fired when a new rare part is discovered */
	UPROPERTY(BlueprintAssignable, Category = "BlackMarket|Events")
	FOnRarePartDiscovered OnRarePartDiscovered;

protected:
	/** Initialize dealers */
	void InitializeDealers();

	/** Initialize rare parts catalog */
	void InitializeRareParts();

	/** Generate random inventory for a dealer */
	void GenerateDealerInventory(FName DealerID);

	/** Roll for risk outcome */
	EMGBlackMarketResult RollRiskOutcome(FName DealerID, const FMGBlackMarketItem& Item);

	/** List of all dealers */
	UPROPERTY()
	TArray<FMGBlackMarketDealer> Dealers;

	/** Current inventory per dealer */
	UPROPERTY()
	TMap<FName, TArray<FMGBlackMarketItem>> DealerInventories;

	/** All possible rare parts */
	UPROPERTY()
	TArray<FMGRarePart> RarePartsCatalog;

	/** Discovered rare parts (shown in collection) */
	UPROPERTY()
	TArray<FMGRarePart> DiscoveredRareParts;

	/** Owned rare parts */
	UPROPERTY()
	TArray<FMGRarePart> OwnedRareParts;

	/** Trust levels per dealer */
	UPROPERTY()
	TMap<FName, int32> DealerTrustLevels;

	/** Last inventory refresh time per dealer */
	UPROPERTY()
	TMap<FName, FDateTime> LastInventoryRefresh;

	/** Current highest unlocked tier */
	UPROPERTY()
	EMGBlackMarketTier HighestUnlockedTier = EMGBlackMarketTier::Street;

	/** Inventory refresh interval (hours) */
	UPROPERTY()
	float InventoryRefreshHours = 6.0f;
};

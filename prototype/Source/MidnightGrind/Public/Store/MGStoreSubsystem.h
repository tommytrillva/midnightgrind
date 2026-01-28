// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGStoreSubsystem.h
 * @brief In-game store system for purchasing cosmetic items with fair monetization
 *
 * @section overview Overview
 * This file defines the Store Subsystem for Midnight Grind - a cosmetics-only
 * in-game store that follows ethical monetization principles. The store allows
 * players to purchase visual customization items using either earned in-game
 * currency (GrindCash) or premium currency (NeonCredits).
 *
 * @section philosophy Design Philosophy: Fair Play
 * Midnight Grind's store follows these core principles:
 * - NO PAY-TO-WIN: All items are purely cosmetic with no gameplay advantage
 * - DUAL CURRENCY: Everything can be earned through play OR purchased
 * - TRANSPARENT PRICING: No hidden costs, bundles show individual values
 * - PLAYER-FRIENDLY: Daily deals offer discounts, not artificial scarcity
 * - COMMUNITY FOCUSED: Gifting system encourages positive interactions
 *
 * @section concepts Key Concepts for Beginners
 *
 * ### 1. Store Categories (EMGStoreCategory)
 * Items are organized by type for easy browsing:
 * - Featured: Highlighted items, often new releases
 * - Vehicles: Visual vehicle variants (same stats, different looks)
 * - Liveries: Paint schemes and wraps for vehicles
 * - Decals/Wheels/Neon/Trails: Vehicle customization parts
 * - Horns/Emotes: Expression items for player personality
 * - Avatars/Banners: Profile customization
 * - CrewCustomization: Items for crew/clan identity
 * - Bundles: Grouped items at a discount
 *
 * ### 2. Item Rarity (EMGItemRarity)
 * Visual indicator of item exclusivity (cosmetic only, no power difference):
 * - Common: Standard items, lowest price
 * - Uncommon: Slightly more unique designs
 * - Rare: More elaborate designs
 * - Epic: High-quality, distinctive items
 * - Legendary: Most elaborate and visually impressive
 *
 * ### 3. Store Items (FMGStoreItem)
 * Each item in the store has:
 * - ItemID: Unique identifier for the item
 * - DisplayName/Description: Player-facing text
 * - GrindCashPrice: Cost in earned currency
 * - NeonCreditPrice: Cost in premium currency
 * - Category and Rarity: Classification
 * - ThumbnailTexture/PreviewMeshID: Visual preview assets
 * - bIsBundle: True if this item contains multiple items
 * - bIsLimitedTime: True if item has availability window
 * - bIsGiftable: True if item can be sent to another player
 *
 * ### 4. Daily Deals (FMGDailyDeal)
 * Rotating discounted items that refresh every 24 hours:
 * - DiscountPercent: How much off the normal price (e.g., 30% off)
 * - DiscountedGrindCash/NeonCredits: Already-calculated sale prices
 * - ExpiresAt: When the deal ends
 * - Items cycle to give all players opportunities
 *
 * ### 5. Purchase Flow (FMGPurchaseResult)
 * When a player buys something:
 * - bSuccess: Whether the purchase completed
 * - FailureReason: Why it failed (insufficient funds, already owned, etc.)
 * - UnlockedItems: List of items now available (multiple for bundles)
 *
 * ### 6. Gifting System (FMGGiftTransaction)
 * Players can buy items for friends:
 * - SenderID/RecipientID: Who sent and receives the gift
 * - PersonalMessage: Optional message from the giver
 * - bClaimed: Whether the recipient has accepted the gift
 * - Promotes positive community interaction
 *
 * ### 7. Owned Items (FMGOwnedItem)
 * Tracking what players have acquired:
 * - AcquiredAt: When the item was obtained
 * - AcquisitionMethod: How it was obtained ("Purchase", "Gift", "SeasonPass", etc.)
 * - GiftedBy: If gifted, who sent it
 *
 * @section currencies Currency Types
 * The store uses two currencies (defined in MGCurrencySubsystem):
 *
 * **GrindCash (Earned Currency)**
 * - Earned by playing: completing races, challenges, daily tasks
 * - Cannot be purchased with real money
 * - Every item has a GrindCash price
 *
 * **NeonCredits (Premium Currency)**
 * - Can be purchased with real money
 * - Also earnable in small amounts through gameplay
 * - Provides convenience, not exclusivity
 * - Same items, just faster acquisition
 *
 * @section workflow Typical Workflow
 * @code
 * // Get the store subsystem
 * UMGStoreSubsystem* Store = GetGameInstance()->GetSubsystem<UMGStoreSubsystem>();
 *
 * // Browse featured items
 * TArray<FMGStoreItem> Featured = Store->GetFeaturedItems();
 *
 * // Check if player can afford an item
 * if (Store->CanAffordItem(ItemID, false))  // false = use GrindCash
 * {
 *     // Purchase with earned currency
 *     FMGPurchaseResult Result = Store->PurchaseWithGrindCash(ItemID);
 *     if (Result.bSuccess)
 *     {
 *         // Item unlocked! Update UI
 *     }
 * }
 *
 * // Check daily deals for discounts
 * TArray<FMGDailyDeal> Deals = Store->GetDailyDeals();
 * FTimespan TimeLeft = Store->GetTimeUntilDailyRefresh();
 *
 * // Send a gift to a friend
 * Store->SendGift(ItemID, FriendID, LOCTEXT("GiftMessage", "Enjoy!"));
 *
 * // Claim a received gift
 * TArray<FMGGiftTransaction> Gifts = Store->GetPendingGifts();
 * Store->ClaimGift(Gifts[0].GiftID);
 * @endcode
 *
 * @section bundles Bundle Value System
 * Bundles are always better value than buying items individually:
 * @code
 * // Check bundle savings
 * int64 TotalValue = Store->GetBundleValue(BundleID);
 * float Savings = Store->GetBundleSavingsPercent(BundleID);
 *
 * // See what items you'd actually get (excludes already owned)
 * TArray<FName> NewItems = Store->GetMissingBundleItems(BundleID);
 * @endcode
 *
 * @section events Delegate Events
 * Subscribe to these to update UI in real-time:
 * - OnPurchaseCompleted: Fires after any purchase attempt
 * - OnGiftReceived: Fires when someone sends you a gift
 * - OnDailyDealsRefreshed: Fires when daily deals rotate (midnight UTC)
 * - OnItemUnlocked: Fires when any item becomes available to the player
 *
 * @section ownership Checking Ownership
 * @code
 * // Check if player owns an item
 * if (Store->OwnsItem(ItemID))
 * {
 *     // Already owned, show "Equipped" option instead of "Buy"
 * }
 *
 * // Get all owned items in a category
 * TArray<FMGOwnedItem> OwnedDecals = Store->GetOwnedItemsByCategory(EMGStoreCategory::Decals);
 * @endcode
 *
 * @section persistence Data Persistence
 * - Store catalog is loaded from data tables via LoadStoreData()
 * - Owned items are saved/loaded via SaveOwnedItems()/LoadOwnedItems()
 * - Daily deal rotation handled automatically by CheckDailyRefresh()
 *
 * @see UMGCurrencySubsystem - Manages currency balances
 * @see EMGCurrencyType - Currency type enumeration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Currency/MGCurrencySubsystem.h"
#include "MGStoreSubsystem.generated.h"

/**
 * Fair Store System - Cosmetics Only, No Pay-to-Win
 * - ALL items purchasable with earnable currency (GrindCash)
 * - Premium currency (NeonCredits) only speeds up cosmetic acquisition
 * - No exclusive premium-only items that affect gameplay
 * - Daily deals offer discounts, not pressure tactics
 * - Gifting system for community generosity
 */

UENUM(BlueprintType)
enum class EMGStoreCategory : uint8
{
	Featured,
	Vehicles,       // Visual variants only - no stat differences
	Liveries,
	Decals,
	Wheels,
	Neon,
	Horns,
	Trails,
	Emotes,
	Avatars,
	Banners,
	CrewCustomization,
	Bundles
};

UENUM(BlueprintType)
enum class EMGItemRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary
};

USTRUCT(BlueprintType)
struct FMGStoreItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStoreCategory Category = EMGStoreCategory::Featured;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemRarity Rarity = EMGItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 GrindCashPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 NeonCreditPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ThumbnailTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PreviewMeshID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBundle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> BundleContents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLimitedTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AvailableUntil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGiftable = true;
};

USTRUCT(BlueprintType)
struct FMGDailyDeal
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGStoreItem Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiscountPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 DiscountedGrindCash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 DiscountedNeonCredits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;
};

USTRUCT(BlueprintType)
struct FMGPurchaseResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FailureReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> UnlockedItems;
};

USTRUCT(BlueprintType)
struct FMGGiftTransaction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GiftID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SenderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RecipientID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PersonalMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SentAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClaimed = false;
};

USTRUCT(BlueprintType)
struct FMGOwnedItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AcquiredAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AcquisitionMethod; // "Purchase", "Gift", "SeasonPass", "Achievement", "Event"

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GiftedBy; // If gifted
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPurchaseCompleted, const FMGPurchaseResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnGiftReceived, const FMGGiftTransaction&, Gift);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnDailyDealsRefreshed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnItemUnlocked, FName, ItemID);

UCLASS()
class MIDNIGHTGRIND_API UMGStoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Store Browsing
	UFUNCTION(BlueprintPure, Category = "Store")
	TArray<FMGStoreItem> GetFeaturedItems() const;

	UFUNCTION(BlueprintPure, Category = "Store")
	TArray<FMGStoreItem> GetItemsByCategory(EMGStoreCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Store")
	FMGStoreItem GetItem(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Store")
	TArray<FMGDailyDeal> GetDailyDeals() const { return CurrentDailyDeals; }

	UFUNCTION(BlueprintPure, Category = "Store")
	FTimespan GetTimeUntilDailyRefresh() const;

	// Purchasing
	UFUNCTION(BlueprintCallable, Category = "Store")
	FMGPurchaseResult PurchaseWithGrindCash(FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "Store")
	FMGPurchaseResult PurchaseWithNeonCredits(FName ItemID);

	UFUNCTION(BlueprintCallable, Category = "Store")
	FMGPurchaseResult PurchaseDailyDeal(int32 DealIndex, bool bUseNeonCredits = false);

	UFUNCTION(BlueprintPure, Category = "Store")
	bool CanAffordItem(FName ItemID, bool bUseNeonCredits = false) const;

	// Ownership
	UFUNCTION(BlueprintPure, Category = "Store")
	bool OwnsItem(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Store")
	TArray<FMGOwnedItem> GetOwnedItems() const { return OwnedItems; }

	UFUNCTION(BlueprintPure, Category = "Store")
	TArray<FMGOwnedItem> GetOwnedItemsByCategory(EMGStoreCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Store")
	void UnlockItem(FName ItemID, const FString& Method, const FString& Source = TEXT(""));

	// Gifting
	UFUNCTION(BlueprintCallable, Category = "Store|Gifting")
	bool SendGift(FName ItemID, const FString& RecipientID, const FText& Message);

	UFUNCTION(BlueprintPure, Category = "Store|Gifting")
	TArray<FMGGiftTransaction> GetPendingGifts() const { return PendingGifts; }

	UFUNCTION(BlueprintCallable, Category = "Store|Gifting")
	bool ClaimGift(const FString& GiftID);

	UFUNCTION(BlueprintPure, Category = "Store|Gifting")
	bool CanGiftItem(FName ItemID) const;

	// Bundle Value
	UFUNCTION(BlueprintPure, Category = "Store|Bundle")
	int64 GetBundleValue(FName BundleID) const;

	UFUNCTION(BlueprintPure, Category = "Store|Bundle")
	float GetBundleSavingsPercent(FName BundleID) const;

	UFUNCTION(BlueprintPure, Category = "Store|Bundle")
	TArray<FName> GetMissingBundleItems(FName BundleID) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Store|Events")
	FMGOnPurchaseCompleted OnPurchaseCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Store|Events")
	FMGOnGiftReceived OnGiftReceived;

	UPROPERTY(BlueprintAssignable, Category = "Store|Events")
	FMGOnDailyDealsRefreshed OnDailyDealsRefreshed;

	UPROPERTY(BlueprintAssignable, Category = "Store|Events")
	FMGOnItemUnlocked OnItemUnlocked;

protected:
	void LoadStoreData();
	void LoadOwnedItems();
	void SaveOwnedItems();
	void InitializeStoreCatalog();
	void RefreshDailyDeals();
	void CheckDailyRefresh();
	FMGPurchaseResult ExecutePurchase(FName ItemID, EMGCurrencyType CurrencyType, int64 Price);

private:
	UPROPERTY()
	TArray<FMGStoreItem> StoreCatalog;

	UPROPERTY()
	TArray<FMGDailyDeal> CurrentDailyDeals;

	UPROPERTY()
	TArray<FMGOwnedItem> OwnedItems;

	UPROPERTY()
	TArray<FMGGiftTransaction> PendingGifts;

	FDateTime LastDailyRefresh;
	FTimerHandle DailyRefreshTimer;
};

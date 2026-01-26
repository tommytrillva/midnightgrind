// Copyright Midnight Grind. All Rights Reserved.

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

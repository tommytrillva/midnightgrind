// Copyright Midnight Grind. All Rights Reserved.

#include "Store/MGStoreSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGStoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeStoreCatalog();
	LoadOwnedItems();
	RefreshDailyDeals();

	// Check for daily refresh every hour
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DailyRefreshTimer,
			this,
			&UMGStoreSubsystem::CheckDailyRefresh,
			3600.0f,
			true
		);
	}
}

void UMGStoreSubsystem::Deinitialize()
{
	SaveOwnedItems();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DailyRefreshTimer);
	}
	Super::Deinitialize();
}

TArray<FMGStoreItem> UMGStoreSubsystem::GetFeaturedItems() const
{
	TArray<FMGStoreItem> Featured;
	for (const FMGStoreItem& Item : StoreCatalog)
	{
		if (Item.Category == EMGStoreCategory::Featured)
			Featured.Add(Item);
	}
	return Featured;
}

TArray<FMGStoreItem> UMGStoreSubsystem::GetItemsByCategory(EMGStoreCategory Category) const
{
	TArray<FMGStoreItem> Items;
	for (const FMGStoreItem& Item : StoreCatalog)
	{
		if (Item.Category == Category)
			Items.Add(Item);
	}
	return Items;
}

FMGStoreItem UMGStoreSubsystem::GetItem(FName ItemID) const
{
	const FMGStoreItem* Found = StoreCatalog.FindByPredicate(
		[ItemID](const FMGStoreItem& Item) { return Item.ItemID == ItemID; });
	return Found ? *Found : FMGStoreItem();
}

FTimespan UMGStoreSubsystem::GetTimeUntilDailyRefresh() const
{
	FDateTime Now = FDateTime::UtcNow();
	FDateTime NextRefresh = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay() + 1, 0, 0, 0);
	return NextRefresh - Now;
}

FMGPurchaseResult UMGStoreSubsystem::PurchaseWithGrindCash(FName ItemID)
{
	FMGStoreItem Item = GetItem(ItemID);
	if (Item.ItemID.IsNone())
	{
		FMGPurchaseResult Result;
		Result.bSuccess = false;
		Result.FailureReason = FText::FromString(TEXT("Item not found"));
		return Result;
	}

	return ExecutePurchase(ItemID, EMGCurrencyType::GrindCash, Item.GrindCashPrice);
}

FMGPurchaseResult UMGStoreSubsystem::PurchaseWithNeonCredits(FName ItemID)
{
	FMGStoreItem Item = GetItem(ItemID);
	if (Item.ItemID.IsNone())
	{
		FMGPurchaseResult Result;
		Result.bSuccess = false;
		Result.FailureReason = FText::FromString(TEXT("Item not found"));
		return Result;
	}

	return ExecutePurchase(ItemID, EMGCurrencyType::NeonCredits, Item.NeonCreditPrice);
}

FMGPurchaseResult UMGStoreSubsystem::PurchaseDailyDeal(int32 DealIndex, bool bUseNeonCredits)
{
	FMGPurchaseResult Result;

	if (DealIndex < 0 || DealIndex >= CurrentDailyDeals.Num())
	{
		Result.bSuccess = false;
		Result.FailureReason = FText::FromString(TEXT("Invalid deal"));
		return Result;
	}

	const FMGDailyDeal& Deal = CurrentDailyDeals[DealIndex];
	int64 Price = bUseNeonCredits ? Deal.DiscountedNeonCredits : Deal.DiscountedGrindCash;
	EMGCurrencyType Currency = bUseNeonCredits ? EMGCurrencyType::NeonCredits : EMGCurrencyType::GrindCash;

	return ExecutePurchase(Deal.Item.ItemID, Currency, Price);
}

bool UMGStoreSubsystem::CanAffordItem(FName ItemID, bool bUseNeonCredits) const
{
	FMGStoreItem Item = GetItem(ItemID);
	if (Item.ItemID.IsNone()) return false;

	UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>();
	if (!Currency) return false;

	if (bUseNeonCredits)
		return Currency->CanAfford(EMGCurrencyType::NeonCredits, Item.NeonCreditPrice);
	else
		return Currency->CanAfford(EMGCurrencyType::GrindCash, Item.GrindCashPrice);
}

bool UMGStoreSubsystem::OwnsItem(FName ItemID) const
{
	return OwnedItems.ContainsByPredicate(
		[ItemID](const FMGOwnedItem& Item) { return Item.ItemID == ItemID; });
}

TArray<FMGOwnedItem> UMGStoreSubsystem::GetOwnedItemsByCategory(EMGStoreCategory Category) const
{
	TArray<FMGOwnedItem> Result;
	for (const FMGOwnedItem& Owned : OwnedItems)
	{
		FMGStoreItem StoreItem = GetItem(Owned.ItemID);
		if (StoreItem.Category == Category)
			Result.Add(Owned);
	}
	return Result;
}

void UMGStoreSubsystem::UnlockItem(FName ItemID, const FString& Method, const FString& Source)
{
	if (OwnsItem(ItemID)) return;

	FMGOwnedItem Owned;
	Owned.ItemID = ItemID;
	Owned.AcquiredAt = FDateTime::UtcNow();
	Owned.AcquisitionMethod = Method;
	Owned.GiftedBy = Source;

	OwnedItems.Add(Owned);
	OnItemUnlocked.Broadcast(ItemID);
	SaveOwnedItems();
}

bool UMGStoreSubsystem::SendGift(FName ItemID, const FString& RecipientID, const FText& Message)
{
	if (!CanGiftItem(ItemID)) return false;

	FMGStoreItem Item = GetItem(ItemID);
	if (Item.ItemID.IsNone()) return false;

	UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>();
	if (!Currency || !Currency->CanAfford(EMGCurrencyType::GrindCash, Item.GrindCashPrice))
		return false;

	// Deduct cost
	Currency->SpendCurrency(EMGCurrencyType::GrindCash, Item.GrindCashPrice,
		FString::Printf(TEXT("Gift: %s to %s"), *ItemID.ToString(), *RecipientID));

	// Create gift transaction (would be sent to server)
	FMGGiftTransaction Gift;
	Gift.GiftID = FGuid::NewGuid().ToString();
	Gift.SenderID = TEXT("LocalPlayer"); // Would use actual player ID
	Gift.RecipientID = RecipientID;
	Gift.ItemID = ItemID;
	Gift.PersonalMessage = Message;
	Gift.SentAt = FDateTime::UtcNow();

	return true;
}

bool UMGStoreSubsystem::ClaimGift(const FString& GiftID)
{
	int32 Index = PendingGifts.IndexOfByPredicate(
		[&GiftID](const FMGGiftTransaction& G) { return G.GiftID == GiftID; });

	if (Index == INDEX_NONE) return false;

	FMGGiftTransaction& Gift = PendingGifts[Index];
	Gift.bClaimed = true;

	UnlockItem(Gift.ItemID, TEXT("Gift"), Gift.SenderID);
	OnGiftReceived.Broadcast(Gift);

	PendingGifts.RemoveAt(Index);
	return true;
}

bool UMGStoreSubsystem::CanGiftItem(FName ItemID) const
{
	FMGStoreItem Item = GetItem(ItemID);
	return !Item.ItemID.IsNone() && Item.bIsGiftable;
}

int64 UMGStoreSubsystem::GetBundleValue(FName BundleID) const
{
	FMGStoreItem Bundle = GetItem(BundleID);
	if (!Bundle.bIsBundle) return 0;

	int64 TotalValue = 0;
	for (const FName& ContentID : Bundle.BundleContents)
	{
		FMGStoreItem Item = GetItem(ContentID);
		TotalValue += Item.GrindCashPrice;
	}
	return TotalValue;
}

float UMGStoreSubsystem::GetBundleSavingsPercent(FName BundleID) const
{
	FMGStoreItem Bundle = GetItem(BundleID);
	if (!Bundle.bIsBundle) return 0.0f;

	int64 TotalValue = GetBundleValue(BundleID);
	if (TotalValue <= 0) return 0.0f;

	return 100.0f * (1.0f - ((float)Bundle.GrindCashPrice / (float)TotalValue));
}

TArray<FName> UMGStoreSubsystem::GetMissingBundleItems(FName BundleID) const
{
	TArray<FName> Missing;
	FMGStoreItem Bundle = GetItem(BundleID);

	if (Bundle.bIsBundle)
	{
		for (const FName& ContentID : Bundle.BundleContents)
		{
			if (!OwnsItem(ContentID))
				Missing.Add(ContentID);
		}
	}
	return Missing;
}

void UMGStoreSubsystem::LoadStoreData()
{
	// Would load from server
}

void UMGStoreSubsystem::LoadOwnedItems()
{
	// Would load from cloud save
}

void UMGStoreSubsystem::SaveOwnedItems()
{
	// Would save to cloud save
}

void UMGStoreSubsystem::InitializeStoreCatalog()
{
	// Sample catalog - cosmetics only, fair pricing
	// Grind Cash prices are achievable through normal play
	// Neon Credit prices are optional shortcuts

	// Liveries
	FMGStoreItem NeonStripes;
	NeonStripes.ItemID = FName(TEXT("Livery_NeonStripes"));
	NeonStripes.DisplayName = FText::FromString(TEXT("Neon Stripes"));
	NeonStripes.Description = FText::FromString(TEXT("Classic racing stripes with neon glow"));
	NeonStripes.Category = EMGStoreCategory::Liveries;
	NeonStripes.Rarity = EMGItemRarity::Common;
	NeonStripes.GrindCashPrice = 2000;  // ~2-3 races worth
	NeonStripes.NeonCreditPrice = 100;
	StoreCatalog.Add(NeonStripes);

	FMGStoreItem MidnightChrome;
	MidnightChrome.ItemID = FName(TEXT("Livery_MidnightChrome"));
	MidnightChrome.DisplayName = FText::FromString(TEXT("Midnight Chrome"));
	MidnightChrome.Description = FText::FromString(TEXT("Reflective chrome finish"));
	MidnightChrome.Category = EMGStoreCategory::Liveries;
	MidnightChrome.Rarity = EMGItemRarity::Rare;
	MidnightChrome.GrindCashPrice = 5000;
	MidnightChrome.NeonCreditPrice = 250;
	StoreCatalog.Add(MidnightChrome);

	// Wheels
	FMGStoreItem DeepDish;
	DeepDish.ItemID = FName(TEXT("Wheels_DeepDish"));
	DeepDish.DisplayName = FText::FromString(TEXT("Deep Dish Rims"));
	DeepDish.Description = FText::FromString(TEXT("Classic deep dish style"));
	DeepDish.Category = EMGStoreCategory::Wheels;
	DeepDish.Rarity = EMGItemRarity::Uncommon;
	DeepDish.GrindCashPrice = 3000;
	DeepDish.NeonCreditPrice = 150;
	StoreCatalog.Add(DeepDish);

	// Neon underglow
	FMGStoreItem PurpleNeon;
	PurpleNeon.ItemID = FName(TEXT("Neon_Purple"));
	PurpleNeon.DisplayName = FText::FromString(TEXT("Purple Underglow"));
	PurpleNeon.Description = FText::FromString(TEXT("Vibrant purple neon kit"));
	PurpleNeon.Category = EMGStoreCategory::Neon;
	PurpleNeon.Rarity = EMGItemRarity::Common;
	PurpleNeon.GrindCashPrice = 1500;
	PurpleNeon.NeonCreditPrice = 75;
	StoreCatalog.Add(PurpleNeon);

	// Bundle example
	FMGStoreItem StarterBundle;
	StarterBundle.ItemID = FName(TEXT("Bundle_StarterPack"));
	StarterBundle.DisplayName = FText::FromString(TEXT("Street Starter Pack"));
	StarterBundle.Description = FText::FromString(TEXT("Everything you need to hit the streets in style"));
	StarterBundle.Category = EMGStoreCategory::Bundles;
	StarterBundle.Rarity = EMGItemRarity::Rare;
	StarterBundle.bIsBundle = true;
	StarterBundle.BundleContents.Add(FName(TEXT("Livery_NeonStripes")));
	StarterBundle.BundleContents.Add(FName(TEXT("Wheels_DeepDish")));
	StarterBundle.BundleContents.Add(FName(TEXT("Neon_Purple")));
	StarterBundle.GrindCashPrice = 5000;  // 23% savings vs individual
	StarterBundle.NeonCreditPrice = 250;
	StoreCatalog.Add(StarterBundle);
}

void UMGStoreSubsystem::RefreshDailyDeals()
{
	CurrentDailyDeals.Empty();

	// Pick 3 random items for daily deals with 20-40% discount
	TArray<FMGStoreItem> EligibleItems;
	for (const FMGStoreItem& Item : StoreCatalog)
	{
		if (!Item.bIsBundle && !Item.bIsLimitedTime)
			EligibleItems.Add(Item);
	}

	for (int32 i = 0; i < FMath::Min(3, EligibleItems.Num()); i++)
	{
		int32 RandomIndex = FMath::RandRange(0, EligibleItems.Num() - 1);
		FMGStoreItem& SelectedItem = EligibleItems[RandomIndex];

		FMGDailyDeal Deal;
		Deal.Item = SelectedItem;
		Deal.DiscountPercent = FMath::RandRange(20.0f, 40.0f);
		Deal.DiscountedGrindCash = FMath::RoundToInt64(SelectedItem.GrindCashPrice * (1.0f - Deal.DiscountPercent / 100.0f));
		Deal.DiscountedNeonCredits = FMath::RoundToInt64(SelectedItem.NeonCreditPrice * (1.0f - Deal.DiscountPercent / 100.0f));
		Deal.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(24);

		CurrentDailyDeals.Add(Deal);
		EligibleItems.RemoveAt(RandomIndex);
	}

	LastDailyRefresh = FDateTime::UtcNow();
	OnDailyDealsRefreshed.Broadcast();
}

void UMGStoreSubsystem::CheckDailyRefresh()
{
	FDateTime Now = FDateTime::UtcNow();
	FTimespan TimeSinceRefresh = Now - LastDailyRefresh;

	if (TimeSinceRefresh.GetTotalHours() >= 24.0)
	{
		RefreshDailyDeals();
	}
}

FMGPurchaseResult UMGStoreSubsystem::ExecutePurchase(FName ItemID, EMGCurrencyType CurrencyType, int64 Price)
{
	FMGPurchaseResult Result;
	Result.ItemID = ItemID;

	// Check if already owned
	if (OwnsItem(ItemID))
	{
		Result.bSuccess = false;
		Result.FailureReason = FText::FromString(TEXT("You already own this item"));
		return Result;
	}

	// Check currency
	UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>();
	if (!Currency)
	{
		Result.bSuccess = false;
		Result.FailureReason = FText::FromString(TEXT("Currency system unavailable"));
		return Result;
	}

	if (!Currency->CanAfford(CurrencyType, Price))
	{
		Result.bSuccess = false;
		Result.FailureReason = FText::FromString(TEXT("Insufficient funds"));
		return Result;
	}

	// Execute purchase
	FMGStoreItem Item = GetItem(ItemID);
	Currency->SpendCurrency(CurrencyType, Price, FString::Printf(TEXT("Purchase: %s"), *Item.DisplayName.ToString()));

	// Unlock item(s)
	if (Item.bIsBundle)
	{
		for (const FName& ContentID : Item.BundleContents)
		{
			if (!OwnsItem(ContentID))
			{
				UnlockItem(ContentID, TEXT("Purchase"), TEXT(""));
				Result.UnlockedItems.Add(ContentID);
			}
		}
	}
	else
	{
		UnlockItem(ItemID, TEXT("Purchase"), TEXT(""));
		Result.UnlockedItems.Add(ItemID);
	}

	Result.bSuccess = true;
	OnPurchaseCompleted.Broadcast(Result);

	return Result;
}

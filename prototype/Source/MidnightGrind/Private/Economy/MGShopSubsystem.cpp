// Copyright Midnight Grind. All Rights Reserved.

#include "Economy/MGShopSubsystem.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"

void UMGShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadShopData();
	GenerateMockShopData();
	GenerateDailyDeals();
}

void UMGShopSubsystem::Deinitialize()
{
	SaveShopData();
	Super::Deinitialize();
}

void UMGShopSubsystem::Tick(float DeltaTime)
{
	DealsRefreshAccumulator += DeltaTime;
	if (DealsRefreshAccumulator >= DealsRefreshCheckInterval)
	{
		DealsRefreshAccumulator = 0.0f;
		CheckDealsRefresh();
	}
}

// ==========================================
// WALLET
// ==========================================

int64 UMGShopSubsystem::GetCurrencyBalance(EMGCurrencyType Currency) const
{
	switch (Currency)
	{
		case EMGCurrencyType::Cash: return Wallet.Cash;
		case EMGCurrencyType::Gold: return Wallet.Gold;
		case EMGCurrencyType::Reputation: return Wallet.Reputation;
		case EMGCurrencyType::SeasonTokens: return Wallet.SeasonTokens;
		case EMGCurrencyType::CrewTokens: return Wallet.CrewTokens;
		default: return 0;
	}
}

void UMGShopSubsystem::AddCurrency(EMGCurrencyType Currency, int64 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	switch (Currency)
	{
		case EMGCurrencyType::Cash:
			Wallet.Cash += Amount;
			break;
		case EMGCurrencyType::Gold:
			Wallet.Gold += static_cast<int32>(Amount);
			break;
		case EMGCurrencyType::Reputation:
			Wallet.Reputation += static_cast<int32>(Amount);
			break;
		case EMGCurrencyType::SeasonTokens:
			Wallet.SeasonTokens += static_cast<int32>(Amount);
			break;
		case EMGCurrencyType::CrewTokens:
			Wallet.CrewTokens += static_cast<int32>(Amount);
			break;
	}

	OnCurrencyChanged.Broadcast(Currency, GetCurrencyBalance(Currency));
	SaveShopData();
}

bool UMGShopSubsystem::RemoveCurrency(EMGCurrencyType Currency, int64 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	int64 CurrentBalance = GetCurrencyBalance(Currency);
	if (CurrentBalance < Amount)
	{
		return false;
	}

	switch (Currency)
	{
		case EMGCurrencyType::Cash:
			Wallet.Cash -= Amount;
			break;
		case EMGCurrencyType::Gold:
			Wallet.Gold -= static_cast<int32>(Amount);
			break;
		case EMGCurrencyType::SeasonTokens:
			Wallet.SeasonTokens -= static_cast<int32>(Amount);
			break;
		case EMGCurrencyType::CrewTokens:
			Wallet.CrewTokens -= static_cast<int32>(Amount);
			break;
		default:
			return false; // Can't remove reputation
	}

	OnCurrencyChanged.Broadcast(Currency, GetCurrencyBalance(Currency));
	SaveShopData();
	return true;
}

bool UMGShopSubsystem::CanAfford(const FMGItemPrice& Price) const
{
	return GetCurrencyBalance(Price.Currency) >= Price.Amount;
}

FText UMGShopSubsystem::FormatCurrency(EMGCurrencyType Currency, int64 Amount)
{
	FString Prefix;
	switch (Currency)
	{
		case EMGCurrencyType::Cash:
			Prefix = TEXT("$");
			break;
		case EMGCurrencyType::Gold:
			// Use gold icon or "G"
			return FText::FromString(FString::Printf(TEXT("%lld G"), Amount));
		case EMGCurrencyType::Reputation:
			return FText::FromString(FString::Printf(TEXT("%lld Rep"), Amount));
		case EMGCurrencyType::SeasonTokens:
			return FText::FromString(FString::Printf(TEXT("%lld ST"), Amount));
		case EMGCurrencyType::CrewTokens:
			return FText::FromString(FString::Printf(TEXT("%lld CT"), Amount));
	}

	// Format with commas
	FString AmountStr = FString::Printf(TEXT("%lld"), Amount);
	int32 InsertPos = AmountStr.Len() - 3;
	while (InsertPos > 0)
	{
		AmountStr.InsertAt(InsertPos, TEXT(","));
		InsertPos -= 3;
	}

	return FText::FromString(Prefix + AmountStr);
}

// ==========================================
// SHOP BROWSING
// ==========================================

TArray<FMGShopItem> UMGShopSubsystem::GetItemsByCategory(EMGShopCategory Category) const
{
	TArray<FMGShopItem> Items;

	for (const auto& Pair : AllItems)
	{
		if (Pair.Value.Category == Category)
		{
			FMGShopItem Item = Pair.Value;
			const_cast<UMGShopSubsystem*>(this)->UpdateItemStatus(Item);
			Items.Add(Item);
		}
	}

	// Sort by featured, then new, then price
	Items.Sort([](const FMGShopItem& A, const FMGShopItem& B)
	{
		if (A.bIsFeatured != B.bIsFeatured) return A.bIsFeatured;
		if (A.bIsNew != B.bIsNew) return A.bIsNew;
		return A.Price.Amount < B.Price.Amount;
	});

	return Items;
}

TArray<FMGShopItem> UMGShopSubsystem::GetFeaturedItems() const
{
	TArray<FMGShopItem> Featured;

	for (const auto& Pair : AllItems)
	{
		if (Pair.Value.bIsFeatured)
		{
			FMGShopItem Item = Pair.Value;
			const_cast<UMGShopSubsystem*>(this)->UpdateItemStatus(Item);
			Featured.Add(Item);
		}
	}

	return Featured;
}

TArray<FMGShopItem> UMGShopSubsystem::GetNewItems() const
{
	TArray<FMGShopItem> NewItems;

	for (const auto& Pair : AllItems)
	{
		if (Pair.Value.bIsNew)
		{
			FMGShopItem Item = Pair.Value;
			const_cast<UMGShopSubsystem*>(this)->UpdateItemStatus(Item);
			NewItems.Add(Item);
		}
	}

	return NewItems;
}

FMGShopItem UMGShopSubsystem::GetItem(FName ItemID) const
{
	if (const FMGShopItem* Item = AllItems.Find(ItemID))
	{
		FMGShopItem ItemCopy = *Item;
		const_cast<UMGShopSubsystem*>(this)->UpdateItemStatus(ItemCopy);
		return ItemCopy;
	}
	return FMGShopItem();
}

TArray<FMGShopItem> UMGShopSubsystem::SearchItems(const FString& SearchTerm) const
{
	TArray<FMGShopItem> Results;

	for (const auto& Pair : AllItems)
	{
		if (Pair.Value.DisplayName.ToString().Contains(SearchTerm, ESearchCase::IgnoreCase))
		{
			FMGShopItem Item = Pair.Value;
			const_cast<UMGShopSubsystem*>(this)->UpdateItemStatus(Item);
			Results.Add(Item);
		}
	}

	return Results;
}

TArray<FMGShopItem> UMGShopSubsystem::GetItemsForVehicle(FName VehicleID, EMGShopCategory Category) const
{
	TArray<FMGShopItem> Items;

	for (const auto& Pair : AllItems)
	{
		const FMGShopItem& Item = Pair.Value;

		if (Item.Category != Category)
		{
			continue;
		}

		// Check compatibility
		if (Item.CompatibleVehicles.Num() > 0 && !Item.CompatibleVehicles.Contains(VehicleID))
		{
			continue;
		}

		FMGShopItem ItemCopy = Item;
		const_cast<UMGShopSubsystem*>(this)->UpdateItemStatus(ItemCopy);
		Items.Add(ItemCopy);
	}

	return Items;
}

// ==========================================
// PURCHASING
// ==========================================

EMGPurchaseResult UMGShopSubsystem::PurchaseItem(FName ItemID, bool bUseAlternativePrice)
{
	EMGPurchaseResult CanBuy = CanPurchaseItem(ItemID, bUseAlternativePrice);
	if (CanBuy != EMGPurchaseResult::Success)
	{
		FMGShopItem Item = GetItem(ItemID);
		FMGTransaction EmptyTransaction;
		OnPurchaseComplete.Broadcast(Item, CanBuy, EmptyTransaction);
		return CanBuy;
	}

	FMGShopItem* Item = AllItems.Find(ItemID);
	if (!Item)
	{
		return EMGPurchaseResult::ItemNotAvailable;
	}

	// Get price
	FMGItemPrice Price = bUseAlternativePrice && Item->bHasAlternativePrice
		? Item->AlternativePrice
		: Item->Price;

	// Process transaction
	FMGTransaction Transaction = ProcessPurchase(*Item, Price);

	// Grant item
	GrantItem(ItemID);

	// Update stock
	if (Item->StockQuantity > 0)
	{
		Item->StockQuantity--;
	}

	OnPurchaseComplete.Broadcast(*Item, EMGPurchaseResult::Success, Transaction);
	SaveShopData();

	return EMGPurchaseResult::Success;
}

EMGPurchaseResult UMGShopSubsystem::PurchaseBundle(FName BundleID)
{
	const FMGBundleItem* Bundle = nullptr;
	for (const FMGBundleItem& B : AvailableBundles)
	{
		if (B.BundleID == BundleID)
		{
			Bundle = &B;
			break;
		}
	}

	if (!Bundle)
	{
		return EMGPurchaseResult::ItemNotAvailable;
	}

	// Check affordability
	if (!CanAfford(Bundle->Price))
	{
		return EMGPurchaseResult::InsufficientFunds;
	}

	// Remove currency
	RemoveCurrency(Bundle->Price.Currency, Bundle->Price.Amount);

	// Grant all items in bundle
	for (const FName& ItemID : Bundle->ItemIDs)
	{
		GrantItem(ItemID);
	}

	// Create transaction record
	FMGTransaction Transaction;
	Transaction.TransactionID = FGuid::NewGuid().ToString();
	Transaction.ItemID = BundleID;
	Transaction.ItemName = Bundle->DisplayName;
	Transaction.Currency = Bundle->Price.Currency;
	Transaction.AmountPaid = Bundle->Price.Amount;
	Transaction.Timestamp = FDateTime::Now();
	Transactions.Insert(Transaction, 0);

	SaveShopData();
	return EMGPurchaseResult::Success;
}

EMGPurchaseResult UMGShopSubsystem::CanPurchaseItem(FName ItemID, bool bUseAlternativePrice) const
{
	const FMGShopItem* Item = AllItems.Find(ItemID);
	if (!Item)
	{
		return EMGPurchaseResult::ItemNotAvailable;
	}

	// Check ownership
	if (OwnedItems.Contains(ItemID))
	{
		return EMGPurchaseResult::AlreadyOwned;
	}

	// Check stock
	if (Item->StockQuantity == 0)
	{
		return EMGPurchaseResult::ItemNotAvailable;
	}

	// Check level requirement
	// Would check player level here
	// if (PlayerLevel < Item->RequiredLevel) return EMGPurchaseResult::LevelRequirementNotMet;

	// Check affordability
	FMGItemPrice Price = bUseAlternativePrice && Item->bHasAlternativePrice
		? Item->AlternativePrice
		: Item->Price;

	if (!CanAfford(Price))
	{
		return EMGPurchaseResult::InsufficientFunds;
	}

	return EMGPurchaseResult::Success;
}

FMGShopItem UMGShopSubsystem::GetPurchasePreview(FName ItemID) const
{
	return GetItem(ItemID);
}

// ==========================================
// DAILY DEALS
// ==========================================

FTimespan UMGShopSubsystem::GetTimeUntilDealsRefresh() const
{
	FDateTime Now = FDateTime::Now();
	FDateTime NextRefresh = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0) + FTimespan::FromDays(1);
	return NextRefresh - Now;
}

void UMGShopSubsystem::ForceRefreshDeals()
{
	GenerateDailyDeals();
}

// ==========================================
// BUNDLES
// ==========================================

FMGBundleItem UMGShopSubsystem::GetBundle(FName BundleID) const
{
	for (const FMGBundleItem& Bundle : AvailableBundles)
	{
		if (Bundle.BundleID == BundleID)
		{
			return Bundle;
		}
	}
	return FMGBundleItem();
}

TArray<FMGShopItem> UMGShopSubsystem::GetBundleContents(FName BundleID) const
{
	TArray<FMGShopItem> Contents;

	FMGBundleItem Bundle = GetBundle(BundleID);
	for (const FName& ItemID : Bundle.ItemIDs)
	{
		FMGShopItem Item = GetItem(ItemID);
		if (!Item.ItemID.IsNone())
		{
			Contents.Add(Item);
		}
	}

	return Contents;
}

// ==========================================
// OWNERSHIP
// ==========================================

bool UMGShopSubsystem::IsItemOwned(FName ItemID) const
{
	return OwnedItems.Contains(ItemID);
}

TArray<FMGShopItem> UMGShopSubsystem::GetOwnedItemsInCategory(EMGShopCategory Category) const
{
	TArray<FMGShopItem> Owned;

	for (const FName& ItemID : OwnedItems)
	{
		if (const FMGShopItem* Item = AllItems.Find(ItemID))
		{
			if (Item->Category == Category)
			{
				FMGShopItem ItemCopy = *Item;
				ItemCopy.bIsOwned = true;
				Owned.Add(ItemCopy);
			}
		}
	}

	return Owned;
}

// ==========================================
// TRANSACTION HISTORY
// ==========================================

TArray<FMGTransaction> UMGShopSubsystem::GetTransactionHistory(int32 Count) const
{
	TArray<FMGTransaction> History;
	int32 Limit = FMath::Min(Count, Transactions.Num());

	for (int32 i = 0; i < Limit; i++)
	{
		History.Add(Transactions[i]);
	}

	return History;
}

TArray<FMGTransaction> UMGShopSubsystem::GetRecentPurchases(int32 Count) const
{
	return GetTransactionHistory(Count);
}

// ==========================================
// UTILITY
// ==========================================

FText UMGShopSubsystem::GetRarityName(int32 Rarity)
{
	switch (Rarity)
	{
		case 0: return FText::FromString(TEXT("Common"));
		case 1: return FText::FromString(TEXT("Uncommon"));
		case 2: return FText::FromString(TEXT("Rare"));
		case 3: return FText::FromString(TEXT("Epic"));
		case 4: return FText::FromString(TEXT("Legendary"));
		default: return FText::FromString(TEXT("Common"));
	}
}

FLinearColor UMGShopSubsystem::GetRarityColor(int32 Rarity)
{
	switch (Rarity)
	{
		case 0: return FLinearColor(0.7f, 0.7f, 0.7f); // Gray
		case 1: return FLinearColor(0.3f, 0.8f, 0.3f); // Green
		case 2: return FLinearColor(0.3f, 0.5f, 1.0f); // Blue
		case 3: return FLinearColor(0.8f, 0.3f, 1.0f); // Purple
		case 4: return FLinearColor(1.0f, 0.8f, 0.0f); // Gold
		default: return FLinearColor::White;
	}
}

FText UMGShopSubsystem::GetCategoryDisplayName(EMGShopCategory Category)
{
	switch (Category)
	{
		case EMGShopCategory::Vehicles: return FText::FromString(TEXT("Vehicles"));
		case EMGShopCategory::Performance: return FText::FromString(TEXT("Performance"));
		case EMGShopCategory::Cosmetic: return FText::FromString(TEXT("Cosmetics"));
		case EMGShopCategory::WrapsDecals: return FText::FromString(TEXT("Wraps & Decals"));
		case EMGShopCategory::Wheels: return FText::FromString(TEXT("Wheels"));
		case EMGShopCategory::Special: return FText::FromString(TEXT("Special"));
		case EMGShopCategory::Premium: return FText::FromString(TEXT("Premium"));
		case EMGShopCategory::Bundles: return FText::FromString(TEXT("Bundles"));
		default: return FText::FromString(TEXT("Shop"));
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGShopSubsystem::LoadShopData()
{
	// Would load from save file
	// Initialize with starting money
	Wallet.Cash = 25000;
	Wallet.Gold = 100;
	Wallet.Reputation = 0;
}

void UMGShopSubsystem::SaveShopData()
{
	// Would save to persistent storage
}

void UMGShopSubsystem::GenerateDailyDeals()
{
	DailyDeals.Empty();

	// Get random items for deals
	TArray<FName> ItemIDs;
	AllItems.GetKeys(ItemIDs);

	// Shuffle
	for (int32 i = ItemIDs.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		ItemIDs.Swap(i, j);
	}

	// Create 3 daily deals
	int32 DealCount = FMath::Min(3, ItemIDs.Num());
	for (int32 i = 0; i < DealCount; i++)
	{
		FMGShopItem* Item = AllItems.Find(ItemIDs[i]);
		if (!Item || OwnedItems.Contains(ItemIDs[i]))
		{
			continue;
		}

		FMGDailyDeal Deal;
		Deal.Item = *Item;
		Deal.DealIndex = i;

		// Random discount 20-50%
		Deal.DiscountPercent = FMath::RandRange(20.0f, 50.0f);
		Deal.DiscountedPrice = Item->Price;
		Deal.DiscountedPrice.Amount = FMath::RoundToInt(Item->Price.Amount * (1.0f - Deal.DiscountPercent / 100.0f));
		Deal.DiscountedPrice.OriginalAmount = Item->Price.Amount;
		Deal.DiscountedPrice.bOnSale = true;
		Deal.DiscountedPrice.SalePercent = Deal.DiscountPercent;

		Deal.TimeRemaining = GetTimeUntilDealsRefresh();

		DailyDeals.Add(Deal);
	}

	LastDealsRefresh = FDateTime::Now();
	OnDailyDealsRefreshed.Broadcast(DailyDeals);
}

void UMGShopSubsystem::CheckDealsRefresh()
{
	FDateTime Now = FDateTime::Now();
	FDateTime TodayReset = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), 0, 0, 0);

	if (LastDealsRefresh < TodayReset)
	{
		GenerateDailyDeals();
	}
	else
	{
		// Update time remaining
		for (FMGDailyDeal& Deal : DailyDeals)
		{
			Deal.TimeRemaining = GetTimeUntilDealsRefresh();
		}
	}
}

void UMGShopSubsystem::UpdateItemStatus(FMGShopItem& Item) const
{
	Item.bIsOwned = OwnedItems.Contains(Item.ItemID);
	Item.bCanAfford = CanAfford(Item.Price);
	Item.bMeetsRequirements = true; // Would check level/rep requirements

	// Update sale percent
	if (Item.Price.bOnSale && Item.Price.OriginalAmount > 0)
	{
		Item.Price.SalePercent = CalculateSalePercent(Item.Price.OriginalAmount, Item.Price.Amount);
	}
}

FMGTransaction UMGShopSubsystem::ProcessPurchase(const FMGShopItem& Item, const FMGItemPrice& Price)
{
	// Remove currency
	RemoveCurrency(Price.Currency, Price.Amount);

	// Create transaction record
	FMGTransaction Transaction;
	Transaction.TransactionID = FGuid::NewGuid().ToString();
	Transaction.ItemID = Item.ItemID;
	Transaction.ItemName = Item.DisplayName;
	Transaction.Currency = Price.Currency;
	Transaction.AmountPaid = Price.Amount;
	Transaction.Timestamp = FDateTime::Now();

	// Add to history
	Transactions.Insert(Transaction, 0);

	// Limit history size
	while (Transactions.Num() > 100)
	{
		Transactions.RemoveAt(Transactions.Num() - 1);
	}

	return Transaction;
}

void UMGShopSubsystem::GrantItem(FName ItemID)
{
	if (!OwnedItems.Contains(ItemID))
	{
		OwnedItems.Add(ItemID);

		// Would also grant to appropriate subsystem (customization, vehicles, etc.)
	}
}

void UMGShopSubsystem::GenerateMockShopData()
{
	// Vehicles
	const TArray<TPair<FString, int32>> Vehicles = {
		{TEXT("Midnight Runner"), 50000},
		{TEXT("Street Phantom"), 75000},
		{TEXT("Neon Drift"), 100000},
		{TEXT("Urban Legend"), 150000},
		{TEXT("Ghost Rider"), 250000},
	};

	for (int32 i = 0; i < Vehicles.Num(); i++)
	{
		FMGShopItem Item;
		Item.ItemID = FName(*FString::Printf(TEXT("Vehicle_%d"), i));
		Item.DisplayName = FText::FromString(Vehicles[i].Key);
		Item.Description = FText::FromString(FString::Printf(TEXT("A powerful street racer built for the midnight scene.")));
		Item.Category = EMGShopCategory::Vehicles;
		Item.Price.Currency = EMGCurrencyType::Cash;
		Item.Price.Amount = Vehicles[i].Value;
		Item.RequiredLevel = (i + 1) * 5;
		Item.Rarity = FMath::Min(i, 4);
		Item.bIsFeatured = i == 2;
		Item.bIsNew = i >= 3;
		Item.AssetID = Item.ItemID;

		AllItems.Add(Item.ItemID, Item);
	}

	// Wraps
	const TArray<FString> Wraps = {
		TEXT("Carbon Fiber"),
		TEXT("Racing Stripes"),
		TEXT("Urban Camo"),
		TEXT("Neon Grid"),
		TEXT("Midnight Stars"),
		TEXT("Dragon Scale"),
	};

	for (int32 i = 0; i < Wraps.Num(); i++)
	{
		FMGShopItem Item;
		Item.ItemID = FName(*FString::Printf(TEXT("Wrap_%d"), i));
		Item.DisplayName = FText::FromString(Wraps[i]);
		Item.Description = FText::FromString(TEXT("A custom vehicle wrap."));
		Item.Category = EMGShopCategory::WrapsDecals;
		Item.Price.Currency = EMGCurrencyType::Cash;
		Item.Price.Amount = 5000 + (i * 2500);
		Item.Rarity = FMath::Min(i / 2, 3);

		AllItems.Add(Item.ItemID, Item);
	}

	// Wheels
	const TArray<FString> Wheels = {
		TEXT("Street Spokes"),
		TEXT("Chrome Deep Dish"),
		TEXT("Matte Black"),
		TEXT("Gold Mesh"),
		TEXT("Carbon Forged"),
	};

	for (int32 i = 0; i < Wheels.Num(); i++)
	{
		FMGShopItem Item;
		Item.ItemID = FName(*FString::Printf(TEXT("Wheels_%d"), i));
		Item.DisplayName = FText::FromString(Wheels[i]);
		Item.Description = FText::FromString(TEXT("Custom wheels for any vehicle."));
		Item.Category = EMGShopCategory::Wheels;
		Item.Price.Currency = EMGCurrencyType::Cash;
		Item.Price.Amount = 3000 + (i * 2000);
		Item.Rarity = FMath::Min(i, 3);

		AllItems.Add(Item.ItemID, Item);
	}

	// Performance Parts
	const TArray<TPair<FString, int32>> Parts = {
		{TEXT("Stage 1 Turbo"), 15000},
		{TEXT("Stage 2 Turbo"), 35000},
		{TEXT("Race Exhaust"), 8000},
		{TEXT("Sport Suspension"), 12000},
		{TEXT("Racing Brakes"), 10000},
		{TEXT("NOS Kit"), 20000},
	};

	for (int32 i = 0; i < Parts.Num(); i++)
	{
		FMGShopItem Item;
		Item.ItemID = FName(*FString::Printf(TEXT("Part_%d"), i));
		Item.DisplayName = FText::FromString(Parts[i].Key);
		Item.Description = FText::FromString(TEXT("Performance upgrade for your vehicle."));
		Item.Category = EMGShopCategory::Performance;
		Item.Price.Currency = EMGCurrencyType::Cash;
		Item.Price.Amount = Parts[i].Value;
		Item.Rarity = i < 2 ? i : FMath::Min(i / 2, 2);

		AllItems.Add(Item.ItemID, Item);
	}

	// Bundles
	FMGBundleItem StarterBundle;
	StarterBundle.BundleID = FName("Bundle_Starter");
	StarterBundle.DisplayName = FText::FromString(TEXT("Starter Pack"));
	StarterBundle.Description = FText::FromString(TEXT("Everything you need to hit the streets!"));
	StarterBundle.ItemIDs = {FName("Vehicle_0"), FName("Wheels_0"), FName("Wrap_0")};
	StarterBundle.Price.Currency = EMGCurrencyType::Cash;
	StarterBundle.Price.Amount = 45000;
	StarterBundle.TotalValue = 58000;
	StarterBundle.SavingsPercent = 22.0f;
	AvailableBundles.Add(StarterBundle);
}

float UMGShopSubsystem::CalculateSalePercent(int32 Original, int32 Current) const
{
	if (Original <= 0)
	{
		return 0.0f;
	}
	return (1.0f - (static_cast<float>(Current) / static_cast<float>(Original))) * 100.0f;
}

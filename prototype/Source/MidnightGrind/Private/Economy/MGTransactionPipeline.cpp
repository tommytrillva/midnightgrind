// Copyright Midnight Grind. All Rights Reserved.

#include "Economy/MGTransactionPipeline.h"
#include "Kismet/GameplayStatics.h"

void UMGTransactionPipeline::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CacheSubsystems();
}

void UMGTransactionPipeline::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// PURCHASES
// ==========================================

EMGTransactionResult UMGTransactionPipeline::ProcessPurchase(const FMGPurchaseRequest& Request)
{
	// Validate first
	EMGTransactionResult ValidationResult = ValidatePurchase(Request);
	if (ValidationResult != EMGTransactionResult::Success)
	{
		// Record failed transaction
		FMGTransaction FailedTransaction;
		FailedTransaction.TransactionID = GenerateTransactionID();
		FailedTransaction.Type = EMGTransactionType::ShopPurchase;
		FailedTransaction.Timestamp = FDateTime::UtcNow();
		FailedTransaction.bSuccessful = false;

		switch (ValidationResult)
		{
		case EMGTransactionResult::InsufficientFunds:
			FailedTransaction.FailureReason = NSLOCTEXT("MG", "InsufficientFunds", "Insufficient funds");
			break;
		case EMGTransactionResult::AlreadyOwned:
			FailedTransaction.FailureReason = NSLOCTEXT("MG", "AlreadyOwned", "Item already owned");
			break;
		case EMGTransactionResult::LevelRestricted:
			FailedTransaction.FailureReason = NSLOCTEXT("MG", "LevelRestricted", "Level requirement not met");
			break;
		default:
			FailedTransaction.FailureReason = NSLOCTEXT("MG", "PurchaseFailed", "Purchase failed");
			break;
		}

		OnTransactionComplete.Broadcast(FailedTransaction, ValidationResult);
		return ValidationResult;
	}

	// Deduct payment
	bool bPaymentSuccess = DeductCredits(Request.Price);
	if (!bPaymentSuccess)
	{
		return EMGTransactionResult::InsufficientFunds;
	}

	// Add item to inventory
	FMGTransactionItem ReceivedItem;
	ReceivedItem.ItemType = Request.ItemType;
	ReceivedItem.ItemID = Request.ItemID;
	ReceivedItem.Quantity = Request.Quantity;

	bool bItemAdded = AddItemToInventory(ReceivedItem, Request.TargetVehicleID);
	if (!bItemAdded)
	{
		// Rollback payment
		AddCredits(Request.Price);
		return EMGTransactionResult::InventoryFull;
	}

	// Install if requested
	if (Request.bInstallImmediately && !Request.TargetVehicleID.IsNone())
	{
		if (GarageSubsystem.IsValid())
		{
			// GarageSubsystem->InstallPart(Request.TargetVehicleID, Request.ItemID);
		}
	}

	// Record successful transaction
	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::ShopPurchase;
	Transaction.CreditsDelta = -Request.Price;
	Transaction.PremiumCurrencyDelta = -Request.PremiumPrice;
	Transaction.ItemsReceived.Add(ReceivedItem);
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;
	Transaction.SourceContext = Request.ShopItemID.ToString();

	RecordTransaction(Transaction);

	// Broadcast events
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);
	OnPurchaseComplete.Broadcast(Transaction);
	OnItemReceived.Broadcast(ReceivedItem);

	return EMGTransactionResult::Success;
}

EMGTransactionResult UMGTransactionPipeline::QuickPurchase(FName ItemID, EMGTransactionItemType ItemType, int64 Price)
{
	FMGPurchaseRequest Request;
	Request.ItemID = ItemID;
	Request.ItemType = ItemType;
	Request.Price = Price;
	Request.Quantity = 1;

	return ProcessPurchase(Request);
}

bool UMGTransactionPipeline::CanAfford(int64 Credits, int32 PremiumCurrency) const
{
	if (EconomySubsystem.IsValid())
	{
		int64 CurrentCredits = 0; // EconomySubsystem->GetCredits();
		int32 CurrentPremium = 0; // EconomySubsystem->GetPremiumCurrency();

		return CurrentCredits >= Credits && CurrentPremium >= PremiumCurrency;
	}

	return false;
}

EMGTransactionResult UMGTransactionPipeline::ValidatePurchase(const FMGPurchaseRequest& Request) const
{
	// Check funds
	if (!CanAfford(Request.Price, Request.PremiumPrice))
	{
		return EMGTransactionResult::InsufficientFunds;
	}

	// Check if already owned (for unique items like vehicles)
	if (Request.ItemType == EMGTransactionItemType::Vehicle)
	{
		if (GarageSubsystem.IsValid())
		{
			// if (GarageSubsystem->OwnsVehicle(Request.ItemID))
			// {
			//     return EMGTransactionResult::AlreadyOwned;
			// }
		}
	}

	// Check level restrictions
	if (ProgressionSubsystem.IsValid())
	{
		// int32 PlayerLevel = ProgressionSubsystem->GetPlayerLevel();
		// int32 RequiredLevel = GetItemLevelRequirement(Request.ItemID);
		// if (PlayerLevel < RequiredLevel)
		// {
		//     return EMGTransactionResult::LevelRestricted;
		// }
	}

	return EMGTransactionResult::Success;
}

// ==========================================
// SALES
// ==========================================

EMGTransactionResult UMGTransactionPipeline::SellItem(EMGTransactionItemType ItemType, FName ItemID, int64 SaleValue)
{
	// Remove from inventory
	FMGTransactionItem Item;
	Item.ItemType = ItemType;
	Item.ItemID = ItemID;
	Item.Quantity = 1;

	bool bRemoved = RemoveItemFromInventory(Item);
	if (!bRemoved)
	{
		return EMGTransactionResult::ItemNotAvailable;
	}

	// Add credits
	AddCredits(SaleValue);

	// Record transaction
	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::ShopSale;
	Transaction.CreditsDelta = SaleValue;
	Transaction.ItemsGiven.Add(Item);
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);

	return EMGTransactionResult::Success;
}

EMGTransactionResult UMGTransactionPipeline::SellVehicle(FName VehicleID, int64 SaleValue)
{
	return SellItem(EMGTransactionItemType::Vehicle, VehicleID, SaleValue);
}

int64 UMGTransactionPipeline::GetSaleValue(EMGTransactionItemType ItemType, FName ItemID) const
{
	// Sale value is typically 50-60% of purchase price
	// Would look up base value from data table

	// Placeholder
	return 5000; // Base sale value
}

// ==========================================
// REWARDS
// ==========================================

void UMGTransactionPipeline::AwardRaceRewards(int64 Credits, int32 XP, int32 Reputation, const TArray<FMGTransactionItem>& BonusItems)
{
	// Add credits
	AddCredits(Credits);

	// Add XP
	if (ProgressionSubsystem.IsValid())
	{
		// ProgressionSubsystem->AddXP(XP);
	}

	// Add reputation
	// CareerSubsystem->AddReputation(Reputation);

	// Add bonus items
	for (const FMGTransactionItem& Item : BonusItems)
	{
		AddItemToInventory(Item);
		OnItemReceived.Broadcast(Item);
	}

	// Record transaction
	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::RaceReward;
	Transaction.CreditsDelta = Credits;
	Transaction.ItemsReceived = BonusItems;
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;
	Transaction.Metadata.Add(TEXT("XP"), FString::FromInt(XP));
	Transaction.Metadata.Add(TEXT("Reputation"), FString::FromInt(Reputation));

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);
}

void UMGTransactionPipeline::AwardChallengeRewards(FName ChallengeID, int64 Credits, const TArray<FMGTransactionItem>& Items)
{
	AddCredits(Credits);

	for (const FMGTransactionItem& Item : Items)
	{
		AddItemToInventory(Item);
		OnItemReceived.Broadcast(Item);
	}

	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::ChallengeReward;
	Transaction.CreditsDelta = Credits;
	Transaction.ItemsReceived = Items;
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;
	Transaction.SourceContext = ChallengeID.ToString();

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);
}

void UMGTransactionPipeline::AwardLevelUpRewards(int32 NewLevel, const TArray<FMGTransactionItem>& Items)
{
	for (const FMGTransactionItem& Item : Items)
	{
		AddItemToInventory(Item);
		OnItemReceived.Broadcast(Item);
	}

	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::LevelUpReward;
	Transaction.ItemsReceived = Items;
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;
	Transaction.SourceContext = FString::Printf(TEXT("Level %d"), NewLevel);

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);
}

void UMGTransactionPipeline::AwardPinkSlipVehicle(FName VehicleID, FName OpponentID)
{
	// Add vehicle to garage
	FMGTransactionItem VehicleItem;
	VehicleItem.ItemType = EMGTransactionItemType::Vehicle;
	VehicleItem.ItemID = VehicleID;
	VehicleItem.Quantity = 1;
	VehicleItem.Metadata.Add(TEXT("WonFrom"), OpponentID.ToString());

	AddItemToInventory(VehicleItem);

	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::PinkSlipWin;
	Transaction.ItemsReceived.Add(VehicleItem);
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;
	Transaction.SourceContext = OpponentID.ToString();

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);
	OnItemReceived.Broadcast(VehicleItem);
}

void UMGTransactionPipeline::RemovePinkSlipVehicle(FName VehicleID)
{
	FMGTransactionItem VehicleItem;
	VehicleItem.ItemType = EMGTransactionItemType::Vehicle;
	VehicleItem.ItemID = VehicleID;
	VehicleItem.Quantity = 1;

	RemoveItemFromInventory(VehicleItem);

	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::PinkSlipLoss;
	Transaction.ItemsGiven.Add(VehicleItem);
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);
}

// ==========================================
// COSTS
// ==========================================

EMGTransactionResult UMGTransactionPipeline::PayRepairCost(FName VehicleID, int64 Cost)
{
	if (!CanAfford(Cost))
	{
		return EMGTransactionResult::InsufficientFunds;
	}

	DeductCredits(Cost);

	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::RepairCost;
	Transaction.CreditsDelta = -Cost;
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;
	Transaction.SourceContext = VehicleID.ToString();

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);

	return EMGTransactionResult::Success;
}

EMGTransactionResult UMGTransactionPipeline::PayCustomizationCost(FName VehicleID, int64 Cost, const FString& Description)
{
	if (!CanAfford(Cost))
	{
		return EMGTransactionResult::InsufficientFunds;
	}

	DeductCredits(Cost);

	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::CustomizationCost;
	Transaction.CreditsDelta = -Cost;
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;
	Transaction.SourceContext = FString::Printf(TEXT("%s: %s"), *VehicleID.ToString(), *Description);

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);

	return EMGTransactionResult::Success;
}

EMGTransactionResult UMGTransactionPipeline::PayUpgradeCost(FName VehicleID, FName PartID, int64 Cost)
{
	if (!CanAfford(Cost))
	{
		return EMGTransactionResult::InsufficientFunds;
	}

	DeductCredits(Cost);

	FMGTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.Type = EMGTransactionType::UpgradeCost;
	Transaction.CreditsDelta = -Cost;
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.bSuccessful = true;
	Transaction.SourceContext = FString::Printf(TEXT("%s -> %s"), *VehicleID.ToString(), *PartID.ToString());

	RecordTransaction(Transaction);
	OnTransactionComplete.Broadcast(Transaction, EMGTransactionResult::Success);

	return EMGTransactionResult::Success;
}

// ==========================================
// HISTORY
// ==========================================

TArray<FMGTransaction> UMGTransactionPipeline::GetTransactionHistory(int32 Count) const
{
	int32 StartIndex = FMath::Max(0, TransactionHistory.Num() - Count);
	TArray<FMGTransaction> Result;

	for (int32 i = TransactionHistory.Num() - 1; i >= StartIndex; --i)
	{
		Result.Add(TransactionHistory[i]);
	}

	return Result;
}

TArray<FMGTransaction> UMGTransactionPipeline::GetTransactionsByType(EMGTransactionType Type, int32 Count) const
{
	TArray<FMGTransaction> Result;

	for (int32 i = TransactionHistory.Num() - 1; i >= 0 && Result.Num() < Count; --i)
	{
		if (TransactionHistory[i].Type == Type)
		{
			Result.Add(TransactionHistory[i]);
		}
	}

	return Result;
}

int64 UMGTransactionPipeline::GetTotalSpent() const
{
	int64 Total = 0;

	for (const FMGTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.bSuccessful && Transaction.CreditsDelta < 0)
		{
			Total += FMath::Abs(Transaction.CreditsDelta);
		}
	}

	return Total;
}

int64 UMGTransactionPipeline::GetTotalEarned() const
{
	int64 Total = 0;

	for (const FMGTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.bSuccessful && Transaction.CreditsDelta > 0)
		{
			Total += Transaction.CreditsDelta;
		}
	}

	return Total;
}

// ==========================================
// INTERNAL
// ==========================================

bool UMGTransactionPipeline::DeductCredits(int64 Amount)
{
	if (EconomySubsystem.IsValid())
	{
		// return EconomySubsystem->DeductCredits(Amount);
	}

	// Placeholder - assume success
	return true;
}

void UMGTransactionPipeline::AddCredits(int64 Amount)
{
	if (EconomySubsystem.IsValid())
	{
		// EconomySubsystem->AddCredits(Amount);
	}
}

bool UMGTransactionPipeline::AddItemToInventory(const FMGTransactionItem& Item, FName TargetVehicleID)
{
	switch (Item.ItemType)
	{
	case EMGTransactionItemType::Vehicle:
		if (GarageSubsystem.IsValid())
		{
			// GarageSubsystem->AddVehicle(Item.ItemID);
		}
		break;

	case EMGTransactionItemType::Part:
		if (GarageSubsystem.IsValid())
		{
			// GarageSubsystem->AddPartToInventory(Item.ItemID, Item.Quantity);
			// if (!TargetVehicleID.IsNone())
			// {
			//     GarageSubsystem->AssignPartToVehicle(TargetVehicleID, Item.ItemID);
			// }
		}
		break;

	case EMGTransactionItemType::Paint:
	case EMGTransactionItemType::Vinyl:
	case EMGTransactionItemType::Wheel:
	case EMGTransactionItemType::Customization:
		// Add to customization inventory
		// InventorySubsystem->AddCustomizationItem(Item.ItemID, Item.ItemType);
		break;

	case EMGTransactionItemType::Consumable:
		// Add to consumables
		// InventorySubsystem->AddConsumable(Item.ItemID, Item.Quantity);
		break;

	case EMGTransactionItemType::Currency:
		// Should not happen - handled separately
		break;
	}

	return true;
}

bool UMGTransactionPipeline::RemoveItemFromInventory(const FMGTransactionItem& Item)
{
	switch (Item.ItemType)
	{
	case EMGTransactionItemType::Vehicle:
		if (GarageSubsystem.IsValid())
		{
			// return GarageSubsystem->RemoveVehicle(Item.ItemID);
		}
		break;

	case EMGTransactionItemType::Part:
		if (GarageSubsystem.IsValid())
		{
			// return GarageSubsystem->RemovePartFromInventory(Item.ItemID, Item.Quantity);
		}
		break;

	default:
		// InventorySubsystem->RemoveItem(Item.ItemID, Item.Quantity);
		break;
	}

	return true;
}

void UMGTransactionPipeline::RecordTransaction(const FMGTransaction& Transaction)
{
	TransactionHistory.Add(Transaction);

	// Trim old entries if needed
	while (TransactionHistory.Num() > MaxHistoryEntries)
	{
		TransactionHistory.RemoveAt(0);
	}
}

FGuid UMGTransactionPipeline::GenerateTransactionID() const
{
	return FGuid::NewGuid();
}

void UMGTransactionPipeline::CacheSubsystems()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		// EconomySubsystem = GI->GetSubsystem<UMGEconomySubsystem>();
		// GarageSubsystem = GI->GetSubsystem<UMGGarageSubsystem>();
		// ProgressionSubsystem = GI->GetSubsystem<UMGProgressionSubsystem>();
	}
}

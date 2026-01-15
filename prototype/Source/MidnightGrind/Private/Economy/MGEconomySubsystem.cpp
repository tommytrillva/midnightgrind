// Copyright Midnight Grind. All Rights Reserved.

#include "Economy/MGEconomySubsystem.h"
#include "Vehicle/MGVehicleData.h"

void UMGEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MGEconomySubsystem initialized with %lld starting credits"), Credits);
}

void UMGEconomySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// CREDITS/BALANCE
// ==========================================

bool UMGEconomySubsystem::AddCredits(int64 Amount, EMGTransactionType Type, const FText& Description, FName RelatedItemID)
{
	if (Amount <= 0)
	{
		return false;
	}

	Credits += Amount;
	TotalEarned += Amount;

	RecordTransaction(Type, Amount, Description, RelatedItemID);
	OnCreditsChanged.Broadcast(Credits, Amount);

	return true;
}

bool UMGEconomySubsystem::SpendCredits(int64 Amount, EMGTransactionType Type, const FText& Description, FName RelatedItemID)
{
	if (Amount <= 0 || !CanAfford(Amount))
	{
		return false;
	}

	Credits -= Amount;
	TotalSpent += Amount;

	RecordTransaction(Type, -Amount, Description, RelatedItemID);
	OnCreditsChanged.Broadcast(Credits, -Amount);

	return true;
}

void UMGEconomySubsystem::SetCredits(int64 Amount)
{
	int64 Delta = Amount - Credits;
	Credits = FMath::Max(0LL, Amount);

	if (Delta != 0)
	{
		OnCreditsChanged.Broadcast(Credits, Delta);
	}
}

// ==========================================
// TRANSACTIONS
// ==========================================

TArray<FMGTransaction> UMGEconomySubsystem::GetRecentTransactions(int32 Count) const
{
	TArray<FMGTransaction> Result;
	int32 StartIndex = FMath::Max(0, TransactionHistory.Num() - Count);

	for (int32 i = TransactionHistory.Num() - 1; i >= StartIndex; --i)
	{
		Result.Add(TransactionHistory[i]);
	}

	return Result;
}

TArray<FMGTransaction> UMGEconomySubsystem::GetTransactionsByType(EMGTransactionType Type) const
{
	TArray<FMGTransaction> Result;

	for (const FMGTransaction& Transaction : TransactionHistory)
	{
		if (Transaction.Type == Type)
		{
			Result.Add(Transaction);
		}
	}

	return Result;
}

void UMGEconomySubsystem::RecordTransaction(EMGTransactionType Type, int64 Amount, const FText& Description, FName RelatedItemID)
{
	FMGTransaction Transaction;
	Transaction.Type = Type;
	Transaction.Amount = Amount;
	Transaction.BalanceAfter = Credits;
	Transaction.Description = Description;
	Transaction.RelatedItemID = RelatedItemID;

	TransactionHistory.Add(Transaction);

	// Trim history if too long
	while (TransactionHistory.Num() > MaxTransactionHistory)
	{
		TransactionHistory.RemoveAt(0);
	}

	OnTransactionCompleted.Broadcast(Transaction);
}

// ==========================================
// RACE ECONOMY
// ==========================================

int64 UMGEconomySubsystem::CalculateRaceWinnings(int32 Position, int32 TotalRacers, int64 BasePrize, float DifficultyMultiplier)
{
	if (Position <= 0 || Position > TotalRacers)
	{
		return 0;
	}

	// Prize distribution percentages
	float PrizePercentage = 0.0f;
	switch (Position)
	{
		case 1: PrizePercentage = 1.0f; break;		// 100% of base
		case 2: PrizePercentage = 0.6f; break;		// 60%
		case 3: PrizePercentage = 0.4f; break;		// 40%
		case 4: PrizePercentage = 0.25f; break;		// 25%
		case 5: PrizePercentage = 0.15f; break;		// 15%
		default: PrizePercentage = 0.1f; break;		// 10% for finishing
	}

	// Bonus for more competitors
	float CompetitorBonus = 1.0f + (TotalRacers - 4) * 0.05f; // 5% per racer above 4
	CompetitorBonus = FMath::Clamp(CompetitorBonus, 1.0f, 1.5f);

	int64 Winnings = static_cast<int64>(BasePrize * PrizePercentage * DifficultyMultiplier * CompetitorBonus);

	return FMath::Max(Winnings, 100LL); // Minimum 100 credits for finishing
}

int64 UMGEconomySubsystem::CalculateEntryFee(int64 BasePrize, float FeePercentage)
{
	return static_cast<int64>(BasePrize * FeePercentage);
}

bool UMGEconomySubsystem::PayEntryFee(int64 Fee, FName RaceID)
{
	FText Description = FText::Format(NSLOCTEXT("Economy", "EntryFee", "Race entry fee: {0}"), FText::FromName(RaceID));
	return SpendCredits(Fee, EMGTransactionType::RaceEntryFee, Description, RaceID);
}

void UMGEconomySubsystem::AwardRaceWinnings(int64 Amount, FName RaceID)
{
	FText Description = FText::Format(NSLOCTEXT("Economy", "RaceWinnings", "Race winnings: {0}"), FText::FromName(RaceID));
	AddCredits(Amount, EMGTransactionType::RaceWinnings, Description, RaceID);
}

void UMGEconomySubsystem::ProcessPinkSlipWin(int64 VehicleValue, FName VehicleID)
{
	FText Description = FText::Format(NSLOCTEXT("Economy", "PinkSlipWin", "Pink slip win: {0}"), FText::FromName(VehicleID));
	// Note: The actual vehicle is added through the garage, this just records the transaction
	RecordTransaction(EMGTransactionType::PinkSlipWin, VehicleValue, Description, VehicleID);
}

void UMGEconomySubsystem::ProcessPinkSlipLoss(int64 VehicleValue, FName VehicleID)
{
	FText Description = FText::Format(NSLOCTEXT("Economy", "PinkSlipLoss", "Pink slip loss: {0}"), FText::FromName(VehicleID));
	// Note: The actual vehicle is removed through the garage, this just records the transaction
	RecordTransaction(EMGTransactionType::PinkSlipLoss, -VehicleValue, Description, VehicleID);
}

// ==========================================
// SHOP/PURCHASES
// ==========================================

bool UMGEconomySubsystem::PurchaseVehicle(UMGVehicleModelData* VehicleModel, FText& OutMessage)
{
	if (!VehicleModel)
	{
		OutMessage = NSLOCTEXT("Economy", "InvalidVehicle", "Invalid vehicle");
		OnPurchaseResult.Broadcast(false, OutMessage);
		return false;
	}

	int64 Price = VehicleModel->BasePriceMSRP;

	if (!CanAfford(Price))
	{
		OutMessage = NSLOCTEXT("Economy", "CannotAfford", "Not enough credits");
		OnPurchaseResult.Broadcast(false, OutMessage);
		return false;
	}

	FText Description = FText::Format(NSLOCTEXT("Economy", "VehiclePurchase", "Purchased {0}"), VehicleModel->DisplayName);

	if (SpendCredits(Price, EMGTransactionType::VehiclePurchase, Description, VehicleModel->ModelID))
	{
		OutMessage = FText::Format(NSLOCTEXT("Economy", "PurchaseSuccess", "Successfully purchased {0}"), VehicleModel->DisplayName);
		OnPurchaseResult.Broadcast(true, OutMessage);
		return true;
	}

	OutMessage = NSLOCTEXT("Economy", "PurchaseFailed", "Purchase failed");
	OnPurchaseResult.Broadcast(false, OutMessage);
	return false;
}

bool UMGEconomySubsystem::PurchasePart(FName PartID, int64 Price, FText& OutMessage)
{
	if (!CanAfford(Price))
	{
		OutMessage = NSLOCTEXT("Economy", "CannotAfford", "Not enough credits");
		OnPurchaseResult.Broadcast(false, OutMessage);
		return false;
	}

	FText Description = FText::Format(NSLOCTEXT("Economy", "PartPurchase", "Purchased part: {0}"), FText::FromName(PartID));

	if (SpendCredits(Price, EMGTransactionType::PartPurchase, Description, PartID))
	{
		OutMessage = NSLOCTEXT("Economy", "PartPurchaseSuccess", "Part purchased successfully");
		OnPurchaseResult.Broadcast(true, OutMessage);
		return true;
	}

	OutMessage = NSLOCTEXT("Economy", "PurchaseFailed", "Purchase failed");
	OnPurchaseResult.Broadcast(false, OutMessage);
	return false;
}

bool UMGEconomySubsystem::SellVehicle(FName VehicleID, int64 SellPrice, FText& OutMessage)
{
	FText Description = FText::Format(NSLOCTEXT("Economy", "VehicleSale", "Sold vehicle: {0}"), FText::FromName(VehicleID));

	if (AddCredits(SellPrice, EMGTransactionType::VehicleSale, Description, VehicleID))
	{
		OutMessage = FText::Format(NSLOCTEXT("Economy", "SaleSuccess", "Sold for {0}"), FormatCredits(SellPrice));
		return true;
	}

	OutMessage = NSLOCTEXT("Economy", "SaleFailed", "Sale failed");
	return false;
}

bool UMGEconomySubsystem::SellPart(FName PartID, int64 SellPrice, FText& OutMessage)
{
	FText Description = FText::Format(NSLOCTEXT("Economy", "PartSale", "Sold part: {0}"), FText::FromName(PartID));

	if (AddCredits(SellPrice, EMGTransactionType::PartSale, Description, PartID))
	{
		OutMessage = FText::Format(NSLOCTEXT("Economy", "SaleSuccess", "Sold for {0}"), FormatCredits(SellPrice));
		return true;
	}

	OutMessage = NSLOCTEXT("Economy", "SaleFailed", "Sale failed");
	return false;
}

bool UMGEconomySubsystem::PurchaseShopItem(const FMGShopItem& Item, FText& OutMessage)
{
	int64 Price = Item.GetEffectivePrice();

	if (!Item.bIsAvailable)
	{
		OutMessage = NSLOCTEXT("Economy", "ItemUnavailable", "Item is not available");
		OnPurchaseResult.Broadcast(false, OutMessage);
		return false;
	}

	if (!CanAfford(Price))
	{
		OutMessage = NSLOCTEXT("Economy", "CannotAfford", "Not enough credits");
		OnPurchaseResult.Broadcast(false, OutMessage);
		return false;
	}

	FText Description = FText::Format(NSLOCTEXT("Economy", "ShopPurchase", "Purchased {0}"), Item.DisplayName);

	if (SpendCredits(Price, EMGTransactionType::Other, Description, Item.ItemID))
	{
		OutMessage = FText::Format(NSLOCTEXT("Economy", "PurchaseSuccess", "Successfully purchased {0}"), Item.DisplayName);
		OnPurchaseResult.Broadcast(true, OutMessage);
		return true;
	}

	OutMessage = NSLOCTEXT("Economy", "PurchaseFailed", "Purchase failed");
	OnPurchaseResult.Broadcast(false, OutMessage);
	return false;
}

int64 UMGEconomySubsystem::CalculateSellValue(int64 PurchasePrice, float Condition, float DepreciationRate)
{
	// Base sell value is purchase price minus depreciation
	float BaseValue = PurchasePrice * (1.0f - DepreciationRate);

	// Condition affects final value
	float ConditionFactor = FMath::Lerp(0.5f, 1.0f, Condition / 100.0f);

	return static_cast<int64>(BaseValue * ConditionFactor);
}

// ==========================================
// WAGERS
// ==========================================

bool UMGEconomySubsystem::PlaceWager(int64 Amount, FName RaceID)
{
	if (HasActiveWager())
	{
		return false; // Already have an active wager
	}

	if (!CanAfford(Amount))
	{
		return false;
	}

	FText Description = FText::Format(NSLOCTEXT("Economy", "WagerPlaced", "Wager placed on {0}"), FText::FromName(RaceID));

	if (SpendCredits(Amount, EMGTransactionType::Wager, Description, RaceID))
	{
		ActiveWager = Amount;
		ActiveWagerRaceID = RaceID;
		return true;
	}

	return false;
}

void UMGEconomySubsystem::ResolveWager(bool bWon, float Odds)
{
	if (!HasActiveWager())
	{
		return;
	}

	if (bWon)
	{
		int64 Payout = static_cast<int64>(ActiveWager * Odds);
		FText Description = FText::Format(NSLOCTEXT("Economy", "WagerWon", "Wager won on {0}"), FText::FromName(ActiveWagerRaceID));
		AddCredits(Payout, EMGTransactionType::Wager, Description, ActiveWagerRaceID);
	}
	// If lost, credits were already spent when placing the wager

	ActiveWager = 0;
	ActiveWagerRaceID = NAME_None;
}

// ==========================================
// UTILITY
// ==========================================

FText UMGEconomySubsystem::FormatCredits(int64 Amount)
{
	// Format with thousands separator
	FNumberFormattingOptions Options;
	Options.UseGrouping = true;

	return FText::Format(NSLOCTEXT("Economy", "CreditsFormat", "${0}"),
		FText::AsNumber(Amount, &Options));
}

FText UMGEconomySubsystem::GetTransactionTypeName(EMGTransactionType Type)
{
	switch (Type)
	{
		case EMGTransactionType::RaceWinnings:	return NSLOCTEXT("Transaction", "RaceWinnings", "Race Winnings");
		case EMGTransactionType::RaceEntryFee:	return NSLOCTEXT("Transaction", "RaceEntryFee", "Entry Fee");
		case EMGTransactionType::VehiclePurchase:return NSLOCTEXT("Transaction", "VehiclePurchase", "Vehicle Purchase");
		case EMGTransactionType::VehicleSale:	return NSLOCTEXT("Transaction", "VehicleSale", "Vehicle Sale");
		case EMGTransactionType::PartPurchase:	return NSLOCTEXT("Transaction", "PartPurchase", "Part Purchase");
		case EMGTransactionType::PartSale:		return NSLOCTEXT("Transaction", "PartSale", "Part Sale");
		case EMGTransactionType::PaintJob:		return NSLOCTEXT("Transaction", "PaintJob", "Paint Job");
		case EMGTransactionType::RepairCost:	return NSLOCTEXT("Transaction", "RepairCost", "Repair Cost");
		case EMGTransactionType::PinkSlipWin:	return NSLOCTEXT("Transaction", "PinkSlipWin", "Pink Slip Win");
		case EMGTransactionType::PinkSlipLoss:	return NSLOCTEXT("Transaction", "PinkSlipLoss", "Pink Slip Loss");
		case EMGTransactionType::BountyReward:	return NSLOCTEXT("Transaction", "BountyReward", "Bounty Reward");
		case EMGTransactionType::CrewBonus:		return NSLOCTEXT("Transaction", "CrewBonus", "Crew Bonus");
		case EMGTransactionType::Wager:			return NSLOCTEXT("Transaction", "Wager", "Wager");
		default:								return NSLOCTEXT("Transaction", "Other", "Other");
	}
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Economy/MGBlackMarketSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Police/MGPoliceSubsystem.h"
#include "Reputation/MGReputationSubsystem.h"

// ============================================================================
// SUBSYSTEM LIFECYCLE
// ============================================================================

void UMGBlackMarketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDealers();
	InitializeRareParts();

	// Generate initial inventories for all dealers
	for (const FMGBlackMarketDealer& Dealer : Dealers)
	{
		GenerateDealerInventory(Dealer.DealerID);
	}
}

void UMGBlackMarketSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ============================================================================
// ACCESS CONTROL
// ============================================================================

EMGBlackMarketTier UMGBlackMarketSubsystem::GetAccessTier() const
{
	// Check from highest to lowest tier
	if (CanAccessTier(EMGBlackMarketTier::Phantom))
	{
		return EMGBlackMarketTier::Phantom;
	}
	if (CanAccessTier(EMGBlackMarketTier::Shadow))
	{
		return EMGBlackMarketTier::Shadow;
	}
	if (CanAccessTier(EMGBlackMarketTier::Underground))
	{
		return EMGBlackMarketTier::Underground;
	}
	return EMGBlackMarketTier::Street;
}

bool UMGBlackMarketSubsystem::CanAccessTier(EMGBlackMarketTier Tier) const
{
	int32 MinHeat, MinRep, MinPinkSlips;
	GetTierRequirements(Tier, MinHeat, MinRep, MinPinkSlips);

	// Get current stats from other subsystems
	int32 CurrentHeat = 0;
	int32 CurrentRep = 0;
	int32 PinkSlipWins = 0;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGPoliceSubsystem* Police = GI->GetSubsystem<UMGPoliceSubsystem>())
		{
			// Use highest heat reached, not current heat
			CurrentHeat = static_cast<int32>(Police->GetCriminalRecord().HighestHeatReached);
		}

		if (UMGReputationSubsystem* Rep = GI->GetSubsystem<UMGReputationSubsystem>())
		{
			CurrentRep = Rep->GetTotalReputation();
		}

		// Pink slip wins would come from a stats subsystem
		// For now, check from criminal record or player stats
		if (UMGPoliceSubsystem* Police = GI->GetSubsystem<UMGPoliceSubsystem>())
		{
			PinkSlipWins = Police->GetCriminalRecord().TotalEscapes; // Placeholder
		}
	}

	return CurrentHeat >= MinHeat && CurrentRep >= MinRep && PinkSlipWins >= MinPinkSlips;
}

void UMGBlackMarketSubsystem::GetTierRequirements(EMGBlackMarketTier Tier, int32& OutMinHeat, int32& OutMinRep, int32& OutMinPinkSlips) const
{
	switch (Tier)
	{
		case EMGBlackMarketTier::Street:
			OutMinHeat = 0;
			OutMinRep = 0;
			OutMinPinkSlips = 0;
			break;

		case EMGBlackMarketTier::Underground:
			OutMinHeat = 2; // Must have reached Heat Level 2
			OutMinRep = 500;
			OutMinPinkSlips = 0;
			break;

		case EMGBlackMarketTier::Shadow:
			OutMinHeat = 4; // Must have reached Heat Level 4
			OutMinRep = 2000;
			OutMinPinkSlips = 3;
			break;

		case EMGBlackMarketTier::Phantom:
			OutMinHeat = 5; // Must have reached Max Heat
			OutMinRep = 10000;
			OutMinPinkSlips = 10;
			break;

		default:
			OutMinHeat = 0;
			OutMinRep = 0;
			OutMinPinkSlips = 0;
	}
}

// ============================================================================
// DEALERS
// ============================================================================

TArray<FMGBlackMarketDealer> UMGBlackMarketSubsystem::GetAvailableDealers() const
{
	TArray<FMGBlackMarketDealer> Available;
	EMGBlackMarketTier CurrentTier = GetAccessTier();

	for (const FMGBlackMarketDealer& Dealer : Dealers)
	{
		if (static_cast<int32>(Dealer.RequiredTier) <= static_cast<int32>(CurrentTier))
		{
			Available.Add(Dealer);
		}
	}

	return Available;
}

FMGBlackMarketDealer UMGBlackMarketSubsystem::GetDealer(FName DealerID) const
{
	for (const FMGBlackMarketDealer& Dealer : Dealers)
	{
		if (Dealer.DealerID == DealerID)
		{
			return Dealer;
		}
	}
	return FMGBlackMarketDealer();
}

int32 UMGBlackMarketSubsystem::GetDealerTrust(FName DealerID) const
{
	if (const int32* Trust = DealerTrustLevels.Find(DealerID))
	{
		return *Trust;
	}
	return 0;
}

void UMGBlackMarketSubsystem::AddDealerTrust(FName DealerID, int32 Amount)
{
	int32& Trust = DealerTrustLevels.FindOrAdd(DealerID);
	int32 OldTrust = Trust;
	Trust = FMath::Clamp(Trust + Amount, 0, 100);

	if (Trust != OldTrust)
	{
		OnDealerTrustChanged.Broadcast(DealerID, Trust);
	}
}

// ============================================================================
// INVENTORY
// ============================================================================

TArray<FMGBlackMarketItem> UMGBlackMarketSubsystem::GetDealerInventory(FName DealerID) const
{
	if (const TArray<FMGBlackMarketItem>* Inventory = DealerInventories.Find(DealerID))
	{
		// Filter out expired items
		TArray<FMGBlackMarketItem> ValidItems;
		FDateTime Now = FDateTime::UtcNow();

		for (const FMGBlackMarketItem& Item : *Inventory)
		{
			if (Item.ExpiresAt > Now && (Item.Part.Stock == -1 || Item.Part.Stock > 0))
			{
				ValidItems.Add(Item);
			}
		}
		return ValidItems;
	}
	return TArray<FMGBlackMarketItem>();
}

TArray<FMGBlackMarketItem> UMGBlackMarketSubsystem::GetAllAvailableItems() const
{
	TArray<FMGBlackMarketItem> AllItems;

	for (const FMGBlackMarketDealer& Dealer : GetAvailableDealers())
	{
		AllItems.Append(GetDealerInventory(Dealer.DealerID));
	}

	return AllItems;
}

void UMGBlackMarketSubsystem::RefreshDealerInventory(FName DealerID)
{
	GenerateDealerInventory(DealerID);
	LastInventoryRefresh.Add(DealerID, FDateTime::UtcNow());
}

// ============================================================================
// PURCHASES
// ============================================================================

EMGBlackMarketResult UMGBlackMarketSubsystem::PurchaseItem(FName DealerID, FName PartVariantID)
{
	// Check dealer exists and is accessible
	FMGBlackMarketDealer Dealer = GetDealer(DealerID);
	if (Dealer.DealerID.IsNone())
	{
		return EMGBlackMarketResult::DealerUnavailable;
	}

	EMGBlackMarketTier CurrentTier = GetAccessTier();
	if (static_cast<int32>(Dealer.RequiredTier) > static_cast<int32>(CurrentTier))
	{
		return EMGBlackMarketResult::AccessDenied;
	}

	// Find the item in inventory
	TArray<FMGBlackMarketItem>* Inventory = DealerInventories.Find(DealerID);
	if (!Inventory)
	{
		return EMGBlackMarketResult::OutOfStock;
	}

	FMGBlackMarketItem* ItemToRemove = nullptr;
	for (FMGBlackMarketItem& Item : *Inventory)
	{
		if (Item.Part.VariantID == PartVariantID)
		{
			ItemToRemove = &Item;
			break;
		}
	}

	if (!ItemToRemove)
	{
		return EMGBlackMarketResult::OutOfStock;
	}

	// Check funds
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGEconomySubsystem* Economy = GI->GetSubsystem<UMGEconomySubsystem>())
		{
			if (Economy->GetPlayerBalance() < ItemToRemove->Price)
			{
				return EMGBlackMarketResult::InsufficientFunds;
			}
		}
	}

	// Roll for risk
	EMGBlackMarketResult RiskResult = RollRiskOutcome(DealerID, *ItemToRemove);
	if (RiskResult != EMGBlackMarketResult::Success)
	{
		OnBlackMarketPurchase.Broadcast(*ItemToRemove, RiskResult);
		return RiskResult;
	}

	// Process purchase
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGEconomySubsystem* Economy = GI->GetSubsystem<UMGEconomySubsystem>())
		{
			Economy->DeductMoney(ItemToRemove->Price, EMGTransactionType::PartPurchase,
				FText::FromString(FString::Printf(TEXT("Black Market: %s"), *ItemToRemove->Part.DisplayName.ToString())));
		}
	}

	// Add to owned rare parts
	OwnedRareParts.Add(ItemToRemove->Part);

	// Add to discovered if not already
	bool bAlreadyDiscovered = false;
	for (const FMGRarePart& Discovered : DiscoveredRareParts)
	{
		if (Discovered.VariantID == ItemToRemove->Part.VariantID)
		{
			bAlreadyDiscovered = true;
			break;
		}
	}
	if (!bAlreadyDiscovered)
	{
		DiscoveredRareParts.Add(ItemToRemove->Part);
		OnRarePartDiscovered.Broadcast(ItemToRemove->Part);
	}

	// Update stock if limited
	if (ItemToRemove->Part.Stock > 0)
	{
		ItemToRemove->Part.Stock--;
	}

	// Build trust with dealer
	AddDealerTrust(DealerID, 5);

	// Add heat if item was "hot"
	if (ItemToRemove->bIsHot)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMGPoliceSubsystem* Police = GI->GetSubsystem<UMGPoliceSubsystem>())
			{
				Police->AddHeat(ItemToRemove->HeatIfCaught / 2, EMGViolationType::PropertyDamage);
			}
		}
	}

	OnBlackMarketPurchase.Broadcast(*ItemToRemove, EMGBlackMarketResult::Success);
	return EMGBlackMarketResult::Success;
}

float UMGBlackMarketSubsystem::GetPurchaseRisk(FName DealerID, FName PartVariantID) const
{
	FMGBlackMarketDealer Dealer = GetDealer(DealerID);
	if (Dealer.DealerID.IsNone())
	{
		return 1.0f;
	}

	float BaseRisk = Dealer.RiskFactor;

	// Trust reduces risk
	int32 Trust = GetDealerTrust(DealerID);
	float TrustReduction = Trust * 0.005f; // 0.5% reduction per trust point
	BaseRisk -= TrustReduction;

	// Hot items increase risk
	if (const TArray<FMGBlackMarketItem>* Inventory = DealerInventories.Find(DealerID))
	{
		for (const FMGBlackMarketItem& Item : *Inventory)
		{
			if (Item.Part.VariantID == PartVariantID && Item.bIsHot)
			{
				BaseRisk += 0.15f;
				break;
			}
		}
	}

	return FMath::Clamp(BaseRisk, 0.0f, 0.5f);
}

int32 UMGBlackMarketSubsystem::GetPotentialHeat(const FMGBlackMarketItem& Item) const
{
	return Item.HeatIfCaught;
}

// ============================================================================
// RARE PARTS
// ============================================================================

bool UMGBlackMarketSubsystem::OwnsRarePart(FName VariantID) const
{
	for (const FMGRarePart& Part : OwnedRareParts)
	{
		if (Part.VariantID == VariantID)
		{
			return true;
		}
	}
	return false;
}

float UMGBlackMarketSubsystem::GetRarityStatBonus(EMGPartRarity Rarity)
{
	switch (Rarity)
	{
		case EMGPartRarity::Common:    return 0.0f;
		case EMGPartRarity::Uncommon:  return 0.03f;  // +3%
		case EMGPartRarity::Rare:      return 0.05f;  // +5%
		case EMGPartRarity::Epic:      return 0.10f;  // +10%
		case EMGPartRarity::Legendary: return 0.15f;  // +15%
		default:                       return 0.0f;
	}
}

FLinearColor UMGBlackMarketSubsystem::GetRarityColor(EMGPartRarity Rarity)
{
	switch (Rarity)
	{
		case EMGPartRarity::Common:    return FLinearColor(0.7f, 0.7f, 0.7f);  // Gray
		case EMGPartRarity::Uncommon:  return FLinearColor(0.2f, 0.8f, 0.2f);  // Green
		case EMGPartRarity::Rare:      return FLinearColor(0.2f, 0.4f, 1.0f);  // Blue
		case EMGPartRarity::Epic:      return FLinearColor(0.6f, 0.2f, 0.9f);  // Purple
		case EMGPartRarity::Legendary: return FLinearColor(1.0f, 0.8f, 0.0f);  // Gold
		default:                       return FLinearColor::White;
	}
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UMGBlackMarketSubsystem::InitializeDealers()
{
	// Street Level Dealers
	{
		FMGBlackMarketDealer Dealer;
		Dealer.DealerID = TEXT("dealer_vinnie");
		Dealer.DisplayName = TEXT("Vinnie");
		Dealer.Nickname = TEXT("The Wrench");
		Dealer.Description = FText::FromString(TEXT("Small-time parts dealer. Fair prices, limited selection."));
		Dealer.RequiredTier = EMGBlackMarketTier::Street;
		Dealer.Personality = EMGDealerPersonality::Professional;
		Dealer.Specialization = TEXT("Engine");
		Dealer.PriceMultiplier = 1.1f;
		Dealer.RareItemChance = 0.05f;
		Dealer.RiskFactor = 0.02f;
		Dealers.Add(Dealer);
	}

	{
		FMGBlackMarketDealer Dealer;
		Dealer.DealerID = TEXT("dealer_rico");
		Dealer.DisplayName = TEXT("Rico");
		Dealer.Nickname = TEXT("Fast Hands");
		Dealer.Description = FText::FromString(TEXT("Cheap parts, but quality varies. Sometimes too good to be true."));
		Dealer.RequiredTier = EMGBlackMarketTier::Street;
		Dealer.Personality = EMGDealerPersonality::Shady;
		Dealer.Specialization = TEXT("Suspension");
		Dealer.PriceMultiplier = 0.8f;
		Dealer.RareItemChance = 0.08f;
		Dealer.RiskFactor = 0.12f; // Higher risk
		Dealers.Add(Dealer);
	}

	// Underground Dealers
	{
		FMGBlackMarketDealer Dealer;
		Dealer.DealerID = TEXT("dealer_ghost");
		Dealer.DisplayName = TEXT("Ghost");
		Dealer.Nickname = TEXT("");
		Dealer.Description = FText::FromString(TEXT("No one knows his real name. Premium turbo parts at fair prices."));
		Dealer.RequiredTier = EMGBlackMarketTier::Underground;
		Dealer.Personality = EMGDealerPersonality::Professional;
		Dealer.Specialization = TEXT("Turbo");
		Dealer.PriceMultiplier = 1.0f;
		Dealer.RareItemChance = 0.15f;
		Dealer.RiskFactor = 0.05f;
		Dealer.AvailableHourStart = 23;
		Dealer.AvailableHourEnd = 3;
		Dealers.Add(Dealer);
	}

	{
		FMGBlackMarketDealer Dealer;
		Dealer.DealerID = TEXT("dealer_miko");
		Dealer.DisplayName = TEXT("Miko");
		Dealer.Nickname = TEXT("The Collector");
		Dealer.Description = FText::FromString(TEXT("JDM specialist. If it came from Japan, she can get it."));
		Dealer.RequiredTier = EMGBlackMarketTier::Underground;
		Dealer.Personality = EMGDealerPersonality::Elite;
		Dealer.Specialization = TEXT("JDM");
		Dealer.PriceMultiplier = 1.25f;
		Dealer.RareItemChance = 0.25f;
		Dealer.RiskFactor = 0.03f;
		Dealers.Add(Dealer);
	}

	// Shadow Network Dealers
	{
		FMGBlackMarketDealer Dealer;
		Dealer.DealerID = TEXT("dealer_shadow");
		Dealer.DisplayName = TEXT("???");
		Dealer.Nickname = TEXT("The Shadow");
		Dealer.Description = FText::FromString(TEXT("Connections to racing teams worldwide. Epic and legendary parts only."));
		Dealer.RequiredTier = EMGBlackMarketTier::Shadow;
		Dealer.Personality = EMGDealerPersonality::Elite;
		Dealer.Specialization = TEXT("Race");
		Dealer.PriceMultiplier = 1.5f;
		Dealer.RareItemChance = 0.5f;
		Dealer.RiskFactor = 0.08f;
		Dealer.AvailableHourStart = 0;
		Dealer.AvailableHourEnd = 5;
		Dealers.Add(Dealer);
	}

	// Phantom Circle Dealer
	{
		FMGBlackMarketDealer Dealer;
		Dealer.DealerID = TEXT("dealer_phantom");
		Dealer.DisplayName = TEXT("The Phantom");
		Dealer.Nickname = TEXT("");
		Dealer.Description = FText::FromString(TEXT("Legend says he was a championship engineer. Now he deals in unicorns."));
		Dealer.RequiredTier = EMGBlackMarketTier::Phantom;
		Dealer.Personality = EMGDealerPersonality::Elite;
		Dealer.Specialization = TEXT("Legendary");
		Dealer.PriceMultiplier = 2.0f;
		Dealer.RareItemChance = 0.8f;
		Dealer.RiskFactor = 0.01f;
		Dealer.AvailableHourStart = 3;
		Dealer.AvailableHourEnd = 4; // Only 1 hour window
		Dealers.Add(Dealer);
	}
}

void UMGBlackMarketSubsystem::InitializeRareParts()
{
	// Rare Turbos
	{
		FMGRarePart Part;
		Part.BasePartID = TEXT("turbo_gt30");
		Part.VariantID = TEXT("turbo_gt30_gold");
		Part.DisplayName = FText::FromString(TEXT("Golden GT30 Turbo"));
		Part.Rarity = EMGPartRarity::Rare;
		Part.StatBonus = 0.05f;
		Part.PriceMultiplier = 2.5f;
		Part.SourceID = TEXT("BlackMarket");
		Part.FlavorText = FText::FromString(TEXT("Gold-plated compressor housing. Pure flex."));
		Part.VisualVariant = TEXT("Gold");
		RarePartsCatalog.Add(Part);
	}

	{
		FMGRarePart Part;
		Part.BasePartID = TEXT("turbo_t51r");
		Part.VariantID = TEXT("turbo_t51r_hks_pro");
		Part.DisplayName = FText::FromString(TEXT("HKS T51R Pro Series"));
		Part.Rarity = EMGPartRarity::Epic;
		Part.StatBonus = 0.10f;
		Part.PriceMultiplier = 3.0f;
		Part.SourceID = TEXT("BlackMarket");
		Part.FlavorText = FText::FromString(TEXT("Team HKS development prototype. Never officially released."));
		Part.VisualVariant = TEXT("Carbon");
		RarePartsCatalog.Add(Part);
	}

	{
		FMGRarePart Part;
		Part.BasePartID = TEXT("turbo_gtx5533");
		Part.VariantID = TEXT("turbo_gtx5533_legendary");
		Part.DisplayName = FText::FromString(TEXT("Legendary GTX5533R"));
		Part.Rarity = EMGPartRarity::Legendary;
		Part.StatBonus = 0.15f;
		Part.PriceMultiplier = 5.0f;
		Part.SourceID = TEXT("Phantom");
		Part.bExclusive = true;
		Part.Stock = 1;
		Part.FlavorText = FText::FromString(TEXT("The unicorn. Supposedly only 3 exist in the world."));
		Part.VisualVariant = TEXT("Titanium");
		RarePartsCatalog.Add(Part);
	}

	// Rare Engines
	{
		FMGRarePart Part;
		Part.BasePartID = TEXT("engine_rb26");
		Part.VariantID = TEXT("engine_rb26_nismo_n1");
		Part.DisplayName = FText::FromString(TEXT("NISMO N1 RB26DETT"));
		Part.Rarity = EMGPartRarity::Epic;
		Part.StatBonus = 0.10f;
		Part.PriceMultiplier = 4.0f;
		Part.SourceID = TEXT("BlackMarket");
		Part.FlavorText = FText::FromString(TEXT("Factory N1 block. Hand-balanced internals."));
		Part.VisualVariant = TEXT("NISMO");
		RarePartsCatalog.Add(Part);
	}

	{
		FMGRarePart Part;
		Part.BasePartID = TEXT("engine_2jz");
		Part.VariantID = TEXT("engine_2jz_billet");
		Part.DisplayName = FText::FromString(TEXT("Full Billet 2JZ-GTE"));
		Part.Rarity = EMGPartRarity::Legendary;
		Part.StatBonus = 0.15f;
		Part.PriceMultiplier = 6.0f;
		Part.SourceID = TEXT("Phantom");
		Part.bExclusive = true;
		Part.Stock = 1;
		Part.FlavorText = FText::FromString(TEXT("100% billet aluminum and steel. Built for 2000+ HP."));
		Part.VisualVariant = TEXT("Billet");
		RarePartsCatalog.Add(Part);
	}

	// Rare Suspension
	{
		FMGRarePart Part;
		Part.BasePartID = TEXT("coilovers_bc_racing");
		Part.VariantID = TEXT("coilovers_bc_racing_gold");
		Part.DisplayName = FText::FromString(TEXT("BC Racing ER Series Gold"));
		Part.Rarity = EMGPartRarity::Rare;
		Part.StatBonus = 0.05f;
		Part.PriceMultiplier = 2.0f;
		Part.SourceID = TEXT("BlackMarket");
		Part.FlavorText = FText::FromString(TEXT("Champagne gold finish. Same performance, more style."));
		Part.VisualVariant = TEXT("Gold");
		RarePartsCatalog.Add(Part);
	}

	// Rare Exhaust
	{
		FMGRarePart Part;
		Part.BasePartID = TEXT("exhaust_titanium");
		Part.VariantID = TEXT("exhaust_amuse_titanium_pro");
		Part.DisplayName = FText::FromString(TEXT("Amuse R1 Titan Pro"));
		Part.Rarity = EMGPartRarity::Epic;
		Part.StatBonus = 0.08f;
		Part.PriceMultiplier = 3.5f;
		Part.SourceID = TEXT("BlackMarket");
		Part.FlavorText = FText::FromString(TEXT("Full titanium from manifold to tip. Blue heat tint."));
		Part.VisualVariant = TEXT("BlueTitanium");
		RarePartsCatalog.Add(Part);
	}
}

void UMGBlackMarketSubsystem::GenerateDealerInventory(FName DealerID)
{
	FMGBlackMarketDealer Dealer = GetDealer(DealerID);
	if (Dealer.DealerID.IsNone())
	{
		return;
	}

	TArray<FMGBlackMarketItem>& Inventory = DealerInventories.FindOrAdd(DealerID);
	Inventory.Empty();

	// Determine number of items based on tier
	int32 NumItems = 3;
	switch (Dealer.RequiredTier)
	{
		case EMGBlackMarketTier::Street:      NumItems = 3; break;
		case EMGBlackMarketTier::Underground: NumItems = 4; break;
		case EMGBlackMarketTier::Shadow:      NumItems = 5; break;
		case EMGBlackMarketTier::Phantom:     NumItems = 3; break; // Fewer but better
	}

	// Generate items from catalog
	TArray<FMGRarePart> AvailableParts = RarePartsCatalog;

	// Filter by dealer specialization and rarity
	TArray<FMGRarePart> EligibleParts;
	for (const FMGRarePart& Part : AvailableParts)
	{
		// Check if dealer would carry this rarity
		bool bRarityOK = false;
		switch (Dealer.RequiredTier)
		{
			case EMGBlackMarketTier::Street:
				bRarityOK = (Part.Rarity == EMGPartRarity::Uncommon || Part.Rarity == EMGPartRarity::Rare);
				break;
			case EMGBlackMarketTier::Underground:
				bRarityOK = (Part.Rarity == EMGPartRarity::Rare || Part.Rarity == EMGPartRarity::Epic);
				break;
			case EMGBlackMarketTier::Shadow:
				bRarityOK = (Part.Rarity == EMGPartRarity::Epic || Part.Rarity == EMGPartRarity::Legendary);
				break;
			case EMGBlackMarketTier::Phantom:
				bRarityOK = (Part.Rarity == EMGPartRarity::Legendary);
				break;
		}

		if (bRarityOK && (Part.Stock == -1 || Part.Stock > 0))
		{
			EligibleParts.Add(Part);
		}
	}

	// Randomly select items
	for (int32 i = 0; i < NumItems && EligibleParts.Num() > 0; ++i)
	{
		int32 Index = FMath::RandRange(0, EligibleParts.Num() - 1);
		FMGRarePart& SelectedPart = EligibleParts[Index];

		FMGBlackMarketItem Item;
		Item.Part = SelectedPart;
		Item.DealerID = DealerID;

		// Calculate price based on dealer multiplier
		int64 BasePrice = 5000; // Would come from parts catalog
		switch (SelectedPart.Rarity)
		{
			case EMGPartRarity::Uncommon:  BasePrice = 3000; break;
			case EMGPartRarity::Rare:      BasePrice = 8000; break;
			case EMGPartRarity::Epic:      BasePrice = 25000; break;
			case EMGPartRarity::Legendary: BasePrice = 100000; break;
		}
		Item.Price = FMath::RoundToInt64(BasePrice * SelectedPart.PriceMultiplier * Dealer.PriceMultiplier);

		// Set expiration (6 hours from now)
		Item.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(InventoryRefreshHours);

		// Random chance of being a "hot" item
		Item.bIsHot = FMath::FRand() < 0.15f;
		if (Item.bIsHot)
		{
			Item.Price = FMath::RoundToInt64(Item.Price * 0.7f); // 30% discount for hot items
			Item.HeatIfCaught = 50 + static_cast<int32>(SelectedPart.Rarity) * 25;
		}

		Inventory.Add(Item);

		// Remove if exclusive (can only appear once)
		if (SelectedPart.bExclusive)
		{
			EligibleParts.RemoveAt(Index);
		}
	}
}

EMGBlackMarketResult UMGBlackMarketSubsystem::RollRiskOutcome(FName DealerID, const FMGBlackMarketItem& Item)
{
	float Risk = GetPurchaseRisk(DealerID, Item.Part.VariantID);

	float Roll = FMath::FRand();
	if (Roll < Risk)
	{
		// Bad outcome - determine type
		FMGBlackMarketDealer Dealer = GetDealer(DealerID);

		// Shady dealers more likely to sell counterfeits
		// Others more likely to be police stings
		if (Dealer.Personality == EMGDealerPersonality::Shady && FMath::FRand() < 0.7f)
		{
			// Lose trust with dealer
			AddDealerTrust(DealerID, -20);
			return EMGBlackMarketResult::Counterfeit;
		}
		else
		{
			// Police sting - add significant heat
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UMGPoliceSubsystem* Police = GI->GetSubsystem<UMGPoliceSubsystem>())
				{
					Police->AddHeat(Item.HeatIfCaught, EMGViolationType::StreetRacing);
				}
			}
			return EMGBlackMarketResult::PoliceSting;
		}
	}

	return EMGBlackMarketResult::Success;
}

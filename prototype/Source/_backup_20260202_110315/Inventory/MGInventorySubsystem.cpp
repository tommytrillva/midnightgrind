// Copyright Midnight Grind. All Rights Reserved.

#include "Inventory/MGInventorySubsystem.h"
#include "Kismet/GameplayStatics.h"

void UMGInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadInventory();
	UpdateStats();
}

void UMGInventorySubsystem::Deinitialize()
{
	SaveInventory();
	Super::Deinitialize();
}

bool UMGInventorySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Item Management
// ============================================================================

bool UMGInventorySubsystem::AddItem(const FMGInventoryItem& Item, int32 Quantity)
{
	if (Quantity <= 0)
	{
		return false;
	}

	// Check inventory capacity
	if (IsInventoryFull() && !Item.bStackable)
	{
		OnInventoryFull.Broadcast(Item.ItemType);
		return false;
	}

	// If stackable, try to find existing stack
	if (Item.bStackable)
	{
		for (FMGInventoryItem& ExistingItem : Items)
		{
			if (ExistingItem.ItemID == Item.ItemID && ExistingItem.Quantity < ExistingItem.MaxStack)
			{
				int32 SpaceInStack = ExistingItem.MaxStack - ExistingItem.Quantity;
				int32 AmountToAdd = FMath::Min(Quantity, SpaceInStack);

				ExistingItem.Quantity += AmountToAdd;
				Quantity -= AmountToAdd;

				OnItemAdded.Broadcast(ExistingItem, AmountToAdd);

				if (Quantity <= 0)
				{
					UpdateStats();
					SaveInventory();
					return true;
				}
			}
		}
	}

	// Add as new item(s)
	while (Quantity > 0 && !IsInventoryFull())
	{
		FMGInventoryItem NewItem = Item;
		NewItem.ItemInstanceID = FGuid::NewGuid();
		NewItem.AcquiredAt = FDateTime::UtcNow();
		NewItem.bNew = true;

		if (NewItem.bStackable)
		{
			NewItem.Quantity = FMath::Min(Quantity, NewItem.MaxStack);
			Quantity -= NewItem.Quantity;
		}
		else
		{
			NewItem.Quantity = 1;
			Quantity--;
		}

		Items.Add(NewItem);
		OnItemAdded.Broadcast(NewItem, NewItem.Quantity);
	}

	UpdateStats();
	SaveInventory();

	return Quantity == 0;
}

bool UMGInventorySubsystem::RemoveItem(FGuid ItemInstanceID, int32 Quantity)
{
	if (Quantity <= 0)
	{
		return false;
	}

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].ItemInstanceID == ItemInstanceID)
		{
			if (Items[i].bLocked)
			{
				return false;
			}

			FMGInventoryItem RemovedItem = Items[i];

			if (Items[i].Quantity <= Quantity)
			{
				int32 RemovedQuantity = Items[i].Quantity;
				Items.RemoveAt(i);
				OnItemRemoved.Broadcast(RemovedItem, RemovedQuantity);
			}
			else
			{
				Items[i].Quantity -= Quantity;
				OnItemRemoved.Broadcast(RemovedItem, Quantity);
			}

			UpdateStats();
			SaveInventory();
			return true;
		}
	}

	return false;
}

bool UMGInventorySubsystem::RemoveItemByID(FName ItemID, int32 Quantity)
{
	if (Quantity <= 0 || ItemID.IsNone())
	{
		return false;
	}

	int32 RemainingToRemove = Quantity;

	for (int32 i = Items.Num() - 1; i >= 0 && RemainingToRemove > 0; --i)
	{
		if (Items[i].ItemID == ItemID && !Items[i].bLocked)
		{
			FMGInventoryItem RemovedItem = Items[i];
			int32 AmountToRemove = FMath::Min(RemainingToRemove, Items[i].Quantity);

			if (Items[i].Quantity <= AmountToRemove)
			{
				Items.RemoveAt(i);
			}
			else
			{
				Items[i].Quantity -= AmountToRemove;
			}

			OnItemRemoved.Broadcast(RemovedItem, AmountToRemove);
			RemainingToRemove -= AmountToRemove;
		}
	}

	if (RemainingToRemove < Quantity)
	{
		UpdateStats();
		SaveInventory();
		return RemainingToRemove == 0;
	}

	return false;
}

bool UMGInventorySubsystem::HasItem(FName ItemID, int32 Quantity) const
{
	return GetItemCount(ItemID) >= Quantity;
}

int32 UMGInventorySubsystem::GetItemCount(FName ItemID) const
{
	int32 TotalCount = 0;

	for (const FMGInventoryItem& Item : Items)
	{
		if (Item.ItemID == ItemID)
		{
			TotalCount += Item.Quantity;
		}
	}

	return TotalCount;
}

FMGInventoryItem UMGInventorySubsystem::GetItem(FGuid ItemInstanceID) const
{
	for (const FMGInventoryItem& Item : Items)
	{
		if (Item.ItemInstanceID == ItemInstanceID)
		{
			return Item;
		}
	}

	return FMGInventoryItem();
}

TArray<FMGInventoryItem> UMGInventorySubsystem::GetItemsByID(FName ItemID) const
{
	TArray<FMGInventoryItem> MatchingItems;

	for (const FMGInventoryItem& Item : Items)
	{
		if (Item.ItemID == ItemID)
		{
			MatchingItems.Add(Item);
		}
	}

	return MatchingItems;
}

TArray<FMGInventoryItem> UMGInventorySubsystem::GetFilteredItems(const FMGInventoryFilter& Filter) const
{
	TArray<FMGInventoryItem> FilteredItems;

	for (const FMGInventoryItem& Item : Items)
	{
		// Type filter
		if (Filter.AllowedTypes.Num() > 0 && !Filter.AllowedTypes.Contains(Item.ItemType))
		{
			continue;
		}

		// Rarity filter
		if (Filter.AllowedRarities.Num() > 0 && !Filter.AllowedRarities.Contains(Item.Rarity))
		{
			continue;
		}

		// Tag filter
		bool bHasAllTags = true;
		for (const FName& RequiredTag : Filter.RequiredTags)
		{
			if (!Item.Tags.Contains(RequiredTag))
			{
				bHasAllTags = false;
				break;
			}
		}
		if (!bHasAllTags)
		{
			continue;
		}

		// Vehicle compatibility
		if (!Filter.CompatibleVehicle.IsNone() && !Item.CompatibleVehicle.IsNone() &&
			Item.CompatibleVehicle != Filter.CompatibleVehicle)
		{
			continue;
		}

		// Boolean filters
		if (Filter.bOnlyTradeable && !Item.bTradeable)
		{
			continue;
		}
		if (Filter.bOnlySellable && !Item.bSellable)
		{
			continue;
		}
		if (Filter.bOnlyFavorites && !Item.bFavorite)
		{
			continue;
		}
		if (Filter.bOnlyNew && !Item.bNew)
		{
			continue;
		}
		if (Filter.bHideEquipped && Item.bEquipped)
		{
			continue;
		}

		// Text search
		if (!Filter.SearchText.IsEmpty())
		{
			FString ItemName = Item.DisplayName.ToString().ToLower();
			FString SearchLower = Filter.SearchText.ToLower();
			if (!ItemName.Contains(SearchLower))
			{
				continue;
			}
		}

		FilteredItems.Add(Item);
	}

	// Sort results
	FilteredItems.Sort([&Filter](const FMGInventoryItem& A, const FMGInventoryItem& B)
	{
		switch (Filter.SortMethod)
		{
		case EMGSortMethod::DateAcquired:
			return A.AcquiredAt < B.AcquiredAt;
		case EMGSortMethod::DateAcquiredDesc:
			return A.AcquiredAt > B.AcquiredAt;
		case EMGSortMethod::Name:
			return A.DisplayName.ToString() < B.DisplayName.ToString();
		case EMGSortMethod::NameDesc:
			return A.DisplayName.ToString() > B.DisplayName.ToString();
		case EMGSortMethod::Rarity:
			return static_cast<int32>(A.Rarity) < static_cast<int32>(B.Rarity);
		case EMGSortMethod::RarityDesc:
			return static_cast<int32>(A.Rarity) > static_cast<int32>(B.Rarity);
		case EMGSortMethod::Type:
			return static_cast<int32>(A.ItemType) < static_cast<int32>(B.ItemType);
		case EMGSortMethod::Value:
			return A.SellValue < B.SellValue;
		case EMGSortMethod::ValueDesc:
			return A.SellValue > B.SellValue;
		case EMGSortMethod::Favorite:
			if (A.bFavorite != B.bFavorite)
			{
				return A.bFavorite;
			}
			return A.AcquiredAt > B.AcquiredAt;
		default:
			return A.AcquiredAt > B.AcquiredAt;
		}
	});

	return FilteredItems;
}

void UMGInventorySubsystem::MarkItemViewed(FGuid ItemInstanceID)
{
	for (FMGInventoryItem& Item : Items)
	{
		if (Item.ItemInstanceID == ItemInstanceID)
		{
			Item.bNew = false;
			SaveInventory();
			return;
		}
	}
}

void UMGInventorySubsystem::MarkAllViewed()
{
	for (FMGInventoryItem& Item : Items)
	{
		Item.bNew = false;
	}
	SaveInventory();
}

void UMGInventorySubsystem::SetItemFavorite(FGuid ItemInstanceID, bool bFavorite)
{
	for (FMGInventoryItem& Item : Items)
	{
		if (Item.ItemInstanceID == ItemInstanceID)
		{
			Item.bFavorite = bFavorite;
			SaveInventory();
			return;
		}
	}
}

void UMGInventorySubsystem::SetItemLocked(FGuid ItemInstanceID, bool bLocked)
{
	for (FMGInventoryItem& Item : Items)
	{
		if (Item.ItemInstanceID == ItemInstanceID)
		{
			Item.bLocked = bLocked;
			SaveInventory();
			return;
		}
	}
}

// ============================================================================
// Vehicle Management
// ============================================================================

bool UMGInventorySubsystem::AddVehicle(const FMGVehicleInventoryEntry& Vehicle)
{
	if (Vehicles.Num() >= MaxGarageSlots)
	{
		OnInventoryFull.Broadcast(EMGItemType::Vehicle);
		return false;
	}

	FMGVehicleInventoryEntry NewVehicle = Vehicle;
	if (!NewVehicle.VehicleInstanceID.IsValid())
	{
		NewVehicle.VehicleInstanceID = FGuid::NewGuid();
	}
	NewVehicle.AcquiredAt = FDateTime::UtcNow();

	Vehicles.Add(NewVehicle);
	OnVehicleAdded.Broadcast(NewVehicle);

	UpdateStats();
	SaveInventory();

	return true;
}

bool UMGInventorySubsystem::RemoveVehicle(FGuid VehicleInstanceID)
{
	for (int32 i = 0; i < Vehicles.Num(); ++i)
	{
		if (Vehicles[i].VehicleInstanceID == VehicleInstanceID)
		{
			if (Vehicles[i].bLocked)
			{
				return false;
			}

			// Unequip all items from this vehicle
			for (const auto& EquippedPair : Vehicles[i].EquippedParts)
			{
				// Mark items as not equipped
				for (FMGInventoryItem& Item : Items)
				{
					if (Item.ItemInstanceID == EquippedPair.Value)
					{
						Item.bEquipped = false;
					}
				}
			}

			Vehicles.RemoveAt(i);
			OnVehicleRemoved.Broadcast(VehicleInstanceID);

			UpdateStats();
			SaveInventory();
			return true;
		}
	}

	return false;
}

bool UMGInventorySubsystem::HasVehicle(FName VehicleID) const
{
	for (const FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleID == VehicleID)
		{
			return true;
		}
	}

	return false;
}

FMGVehicleInventoryEntry UMGInventorySubsystem::GetVehicle(FGuid VehicleInstanceID) const
{
	for (const FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			return Vehicle;
		}
	}

	return FMGVehicleInventoryEntry();
}

TArray<FMGVehicleInventoryEntry> UMGInventorySubsystem::GetVehiclesByClass(FName VehicleClass) const
{
	TArray<FMGVehicleInventoryEntry> MatchingVehicles;

	for (const FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleClass == VehicleClass)
		{
			MatchingVehicles.Add(Vehicle);
		}
	}

	return MatchingVehicles;
}

void UMGInventorySubsystem::SetVehicleFavorite(FGuid VehicleInstanceID, bool bFavorite)
{
	for (FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			Vehicle.bFavorite = bFavorite;
			SaveInventory();
			return;
		}
	}
}

void UMGInventorySubsystem::SetVehicleLocked(FGuid VehicleInstanceID, bool bLocked)
{
	for (FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			Vehicle.bLocked = bLocked;
			SaveInventory();
			return;
		}
	}
}

void UMGInventorySubsystem::SetVehicleCustomName(FGuid VehicleInstanceID, const FString& CustomName)
{
	for (FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			Vehicle.CustomName = CustomName;
			SaveInventory();
			return;
		}
	}
}

void UMGInventorySubsystem::UpdateVehicleStats(FGuid VehicleInstanceID, float Distance, float TopSpeed, bool bWon)
{
	for (FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			Vehicle.TotalRaces++;
			Vehicle.TotalDistance += Distance;

			if (bWon)
			{
				Vehicle.TotalWins++;
			}

			if (TopSpeed > Vehicle.TopSpeed)
			{
				Vehicle.TopSpeed = TopSpeed;
			}

			SaveInventory();
			return;
		}
	}
}

// ============================================================================
// Equipment
// ============================================================================

bool UMGInventorySubsystem::EquipItemToVehicle(FGuid VehicleInstanceID, FGuid ItemInstanceID, FName SlotName)
{
	FMGVehicleInventoryEntry* TargetVehicle = nullptr;
	FMGInventoryItem* TargetItem = nullptr;

	for (FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			TargetVehicle = &Vehicle;
			break;
		}
	}

	for (FMGInventoryItem& Item : Items)
	{
		if (Item.ItemInstanceID == ItemInstanceID)
		{
			TargetItem = &Item;
			break;
		}
	}

	if (!TargetVehicle || !TargetItem)
	{
		return false;
	}

	// Unequip current item in slot if any
	if (TargetVehicle->EquippedParts.Contains(SlotName))
	{
		FGuid OldItemID = TargetVehicle->EquippedParts[SlotName];
		for (FMGInventoryItem& Item : Items)
		{
			if (Item.ItemInstanceID == OldItemID)
			{
				Item.bEquipped = false;
				break;
			}
		}
	}

	// Equip new item
	TargetVehicle->EquippedParts.Add(SlotName, ItemInstanceID);
	TargetItem->bEquipped = true;

	OnItemEquipped.Broadcast(VehicleInstanceID, *TargetItem);
	SaveInventory();

	return true;
}

bool UMGInventorySubsystem::UnequipFromVehicle(FGuid VehicleInstanceID, FName SlotName)
{
	for (FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			if (Vehicle.EquippedParts.Contains(SlotName))
			{
				FGuid ItemID = Vehicle.EquippedParts[SlotName];

				// Mark item as unequipped
				for (FMGInventoryItem& Item : Items)
				{
					if (Item.ItemInstanceID == ItemID)
					{
						Item.bEquipped = false;
						break;
					}
				}

				Vehicle.EquippedParts.Remove(SlotName);
				SaveInventory();
				return true;
			}
		}
	}

	return false;
}

FMGInventoryItem UMGInventorySubsystem::GetEquippedItem(FGuid VehicleInstanceID, FName SlotName) const
{
	for (const FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			if (const FGuid* ItemID = Vehicle.EquippedParts.Find(SlotName))
			{
				return GetItem(*ItemID);
			}
		}
	}

	return FMGInventoryItem();
}

TMap<FName, FMGInventoryItem> UMGInventorySubsystem::GetAllEquippedItems(FGuid VehicleInstanceID) const
{
	TMap<FName, FMGInventoryItem> EquippedItems;

	for (const FMGVehicleInventoryEntry& Vehicle : Vehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			for (const auto& Pair : Vehicle.EquippedParts)
			{
				EquippedItems.Add(Pair.Key, GetItem(Pair.Value));
			}
			break;
		}
	}

	return EquippedItems;
}

TArray<FMGInventoryItem> UMGInventorySubsystem::GetCompatibleItems(FGuid VehicleInstanceID, FName SlotName) const
{
	TArray<FMGInventoryItem> CompatibleItems;

	FMGVehicleInventoryEntry Vehicle = GetVehicle(VehicleInstanceID);
	if (!Vehicle.VehicleInstanceID.IsValid())
	{
		return CompatibleItems;
	}

	for (const FMGInventoryItem& Item : Items)
	{
		// Check if item type matches slot
		bool bTypeMatches = false;

		if (SlotName == TEXT("Wheels") && Item.ItemType == EMGItemType::Wheels)
		{
			bTypeMatches = true;
		}
		else if (SlotName == TEXT("Spoiler") && Item.ItemType == EMGItemType::Spoiler)
		{
			bTypeMatches = true;
		}
		else if (SlotName == TEXT("BodyKit") && Item.ItemType == EMGItemType::BodyKit)
		{
			bTypeMatches = true;
		}
		else if (SlotName == TEXT("Neon") && Item.ItemType == EMGItemType::Neon)
		{
			bTypeMatches = true;
		}
		else if (SlotName == TEXT("Interior") && Item.ItemType == EMGItemType::Interior)
		{
			bTypeMatches = true;
		}
		else if (SlotName == TEXT("Horn") && Item.ItemType == EMGItemType::Horn)
		{
			bTypeMatches = true;
		}

		if (!bTypeMatches)
		{
			continue;
		}

		// Check vehicle compatibility
		if (!Item.CompatibleVehicle.IsNone() && Item.CompatibleVehicle != Vehicle.VehicleID)
		{
			continue;
		}

		// Don't include already equipped items (unless equipped to this vehicle in this slot)
		if (Item.bEquipped)
		{
			if (const FGuid* EquippedID = Vehicle.EquippedParts.Find(SlotName))
			{
				if (*EquippedID != Item.ItemInstanceID)
				{
					continue;
				}
			}
			else
			{
				continue;
			}
		}

		CompatibleItems.Add(Item);
	}

	return CompatibleItems;
}

// ============================================================================
// Crates
// ============================================================================

TArray<FMGInventoryItem> UMGInventorySubsystem::OpenCrate(FName CrateID)
{
	TArray<FMGInventoryItem> Rewards;

	if (!CanOpenCrate(CrateID))
	{
		return Rewards;
	}

	// Remove crate from inventory
	RemoveItemByID(CrateID, 1);

	// Get crate contents
	const FMGCrateContents* CrateContents = CrateTypes.Find(CrateID);
	if (!CrateContents || CrateContents->PossibleItems.Num() == 0)
	{
		return Rewards;
	}

	// Roll rewards (typically 1-3 items)
	int32 RewardCount = FMath::RandRange(1, 3);

	for (int32 i = 0; i < RewardCount; ++i)
	{
		FMGInventoryItem Reward = RollCrateReward(*CrateContents);
		Reward.Source = EMGItemSource::Crate;
		Rewards.Add(Reward);
		AddItem(Reward, 1);
	}

	// Update crate open count
	int32& OpenCount = CrateOpenCounts.FindOrAdd(CrateID);
	OpenCount++;

	Stats.CratesOpened++;

	OnCrateOpened.Broadcast(CrateID, Rewards);
	SaveInventory();

	return Rewards;
}

bool UMGInventorySubsystem::CanOpenCrate(FName CrateID) const
{
	// Check if player has the crate
	if (!HasItem(CrateID, 1))
	{
		return false;
	}

	// Check if crate type is registered
	if (!CrateTypes.Contains(CrateID))
	{
		return false;
	}

	return true;
}

FMGCrateContents UMGInventorySubsystem::GetCrateContents(FName CrateID) const
{
	if (const FMGCrateContents* Contents = CrateTypes.Find(CrateID))
	{
		return *Contents;
	}

	return FMGCrateContents();
}

void UMGInventorySubsystem::RegisterCrateType(const FMGCrateContents& CrateContents)
{
	CrateTypes.Add(CrateContents.CrateID, CrateContents);
}

// ============================================================================
// Crafting
// ============================================================================

bool UMGInventorySubsystem::CraftItem(FName RecipeID)
{
	if (!CanCraftItem(RecipeID))
	{
		return false;
	}

	const FMGCraftingRecipe* Recipe = Recipes.Find(RecipeID);
	if (!Recipe)
	{
		return false;
	}

	// Check success rate
	if (Recipe->SuccessRate < 1.0f)
	{
		float Roll = FMath::FRand();
		if (Roll > Recipe->SuccessRate)
		{
			// Failed - still consume materials
			for (const auto& Required : Recipe->RequiredItems)
			{
				RemoveItemByID(Required.Key, Required.Value);
			}
			return false;
		}
	}

	// Consume materials
	for (const auto& Required : Recipe->RequiredItems)
	{
		RemoveItemByID(Required.Key, Required.Value);
	}

	// Create result item
	FMGInventoryItem ResultItem;
	ResultItem.ItemID = Recipe->ResultItemID;
	ResultItem.Source = EMGItemSource::Craft;
	ResultItem.Quantity = Recipe->ResultQuantity;

	AddItem(ResultItem, Recipe->ResultQuantity);

	Stats.ItemsCrafted++;

	OnItemCrafted.Broadcast(RecipeID, ResultItem);
	SaveInventory();

	return true;
}

bool UMGInventorySubsystem::CanCraftItem(FName RecipeID) const
{
	const FMGCraftingRecipe* Recipe = Recipes.Find(RecipeID);
	if (!Recipe || !Recipe->bUnlocked)
	{
		return false;
	}

	// Check required items
	for (const auto& Required : Recipe->RequiredItems)
	{
		if (!HasItem(Required.Key, Required.Value))
		{
			return false;
		}
	}

	return true;
}

TArray<FMGCraftingRecipe> UMGInventorySubsystem::GetAvailableRecipes() const
{
	TArray<FMGCraftingRecipe> AvailableRecipes;

	for (const auto& RecipePair : Recipes)
	{
		if (RecipePair.Value.bUnlocked)
		{
			AvailableRecipes.Add(RecipePair.Value);
		}
	}

	return AvailableRecipes;
}

FMGCraftingRecipe UMGInventorySubsystem::GetRecipe(FName RecipeID) const
{
	if (const FMGCraftingRecipe* Recipe = Recipes.Find(RecipeID))
	{
		return *Recipe;
	}

	return FMGCraftingRecipe();
}

void UMGInventorySubsystem::UnlockRecipe(FName RecipeID)
{
	if (FMGCraftingRecipe* Recipe = Recipes.Find(RecipeID))
	{
		Recipe->bUnlocked = true;
		SaveInventory();
	}
}

// ============================================================================
// Selling
// ============================================================================

bool UMGInventorySubsystem::SellItem(FGuid ItemInstanceID, int32 Quantity)
{
	FMGInventoryItem Item = GetItem(ItemInstanceID);

	if (!Item.ItemInstanceID.IsValid() || !Item.bSellable || Item.bLocked)
	{
		return false;
	}

	int32 SellQuantity = FMath::Min(Quantity, Item.Quantity);
	int32 TotalValue = Item.SellValue * SellQuantity;

	if (RemoveItem(ItemInstanceID, SellQuantity))
	{
		// Award currency (would integrate with currency subsystem)
		return true;
	}

	return false;
}

bool UMGInventorySubsystem::SellVehicle(FGuid VehicleInstanceID)
{
	FMGVehicleInventoryEntry Vehicle = GetVehicle(VehicleInstanceID);

	if (!Vehicle.VehicleInstanceID.IsValid() || !Vehicle.bTradeable || Vehicle.bLocked)
	{
		return false;
	}

	int32 SellPrice = GetVehicleSellPrice(VehicleInstanceID);

	if (RemoveVehicle(VehicleInstanceID))
	{
		// Award currency (would integrate with currency subsystem)
		return true;
	}

	return false;
}

int32 UMGInventorySubsystem::GetSellPrice(FGuid ItemInstanceID, int32 Quantity) const
{
	FMGInventoryItem Item = GetItem(ItemInstanceID);

	if (!Item.ItemInstanceID.IsValid())
	{
		return 0;
	}

	int32 SellQuantity = FMath::Min(Quantity, Item.Quantity);
	return Item.SellValue * SellQuantity;
}

int32 UMGInventorySubsystem::GetVehicleSellPrice(FGuid VehicleInstanceID) const
{
	FMGVehicleInventoryEntry Vehicle = GetVehicle(VehicleInstanceID);

	if (!Vehicle.VehicleInstanceID.IsValid())
	{
		return 0;
	}

	// Base price by rarity
	int32 BasePrice = 1000;
	switch (Vehicle.Rarity)
	{
	case EMGItemRarity::Uncommon:
		BasePrice = 2500;
		break;
	case EMGItemRarity::Rare:
		BasePrice = 5000;
		break;
	case EMGItemRarity::Epic:
		BasePrice = 10000;
		break;
	case EMGItemRarity::Legendary:
		BasePrice = 25000;
		break;
	case EMGItemRarity::Mythic:
		BasePrice = 50000;
		break;
	case EMGItemRarity::Exclusive:
		BasePrice = 100000;
		break;
	default:
		break;
	}

	// Bonus for performance upgrades
	BasePrice += Vehicle.PerformanceIndex * 10;

	return BasePrice;
}

// ============================================================================
// Stats
// ============================================================================

int32 UMGInventorySubsystem::GetUsedInventorySlots() const
{
	return Items.Num();
}

bool UMGInventorySubsystem::IsInventoryFull() const
{
	return Items.Num() >= MaxInventorySlots;
}

int32 UMGInventorySubsystem::GetNewItemCount() const
{
	int32 Count = 0;
	for (const FMGInventoryItem& Item : Items)
	{
		if (Item.bNew)
		{
			Count++;
		}
	}
	return Count;
}

// ============================================================================
// Protected Helpers
// ============================================================================

void UMGInventorySubsystem::UpdateStats()
{
	Stats.TotalItems = 0;
	Stats.UniqueItems = 0;
	Stats.TotalVehicles = Vehicles.Num();
	Stats.TotalValue = 0;
	Stats.CratesOwned = 0;
	Stats.ItemsByType.Empty();
	Stats.ItemsByRarity.Empty();

	TSet<FName> UniqueItemIDs;

	for (const FMGInventoryItem& Item : Items)
	{
		Stats.TotalItems += Item.Quantity;
		Stats.TotalValue += Item.SellValue * Item.Quantity;

		UniqueItemIDs.Add(Item.ItemID);

		int32& TypeCount = Stats.ItemsByType.FindOrAdd(Item.ItemType);
		TypeCount += Item.Quantity;

		int32& RarityCount = Stats.ItemsByRarity.FindOrAdd(Item.Rarity);
		RarityCount += Item.Quantity;

		if (Item.ItemType == EMGItemType::Crate)
		{
			Stats.CratesOwned += Item.Quantity;
		}
	}

	Stats.UniqueItems = UniqueItemIDs.Num();
}

void UMGInventorySubsystem::SaveInventory()
{
	// Persist inventory data
	// Implementation would use USaveGame or cloud save
}

void UMGInventorySubsystem::LoadInventory()
{
	// Load persisted inventory data
	// Implementation would use USaveGame or cloud save
}

FMGInventoryItem UMGInventorySubsystem::RollCrateReward(const FMGCrateContents& Crate) const
{
	if (Crate.PossibleItems.Num() == 0)
	{
		return FMGInventoryItem();
	}

	// Check for guaranteed rarity pity system
	int32 OpenCount = GetCrateOpenCount(Crate.CrateID);
	bool bGuaranteedRarity = (OpenCount > 0 && (OpenCount % Crate.GuaranteedRarityAtCount) == 0);

	// Build weighted item list
	TArray<FMGInventoryItem> EligibleItems;
	float TotalWeight = 0.0f;

	for (const FMGInventoryItem& Item : Crate.PossibleItems)
	{
		if (bGuaranteedRarity && static_cast<int32>(Item.Rarity) < static_cast<int32>(Crate.GuaranteedRarity))
		{
			continue;
		}

		float Weight = 1.0f;
		if (const float* RarityWeight = Crate.RarityWeights.Find(Item.Rarity))
		{
			Weight = *RarityWeight;
		}

		EligibleItems.Add(Item);
		TotalWeight += Weight;
	}

	if (EligibleItems.Num() == 0)
	{
		// Fallback to any item
		if (Crate.PossibleItems.Num() == 0)
		{
			return FMGInventoryItem();
		}
		return Crate.PossibleItems[FMath::RandRange(0, Crate.PossibleItems.Num() - 1)];
	}

	// Roll for item
	float Roll = FMath::FRand() * TotalWeight;
	float AccumulatedWeight = 0.0f;

	for (const FMGInventoryItem& Item : EligibleItems)
	{
		float Weight = 1.0f;
		if (const float* RarityWeight = Crate.RarityWeights.Find(Item.Rarity))
		{
			Weight = *RarityWeight;
		}

		AccumulatedWeight += Weight;
		if (Roll <= AccumulatedWeight)
		{
			return Item;
		}
	}

	return EligibleItems.Last();
}

int32 UMGInventorySubsystem::GetCrateOpenCount(FName CrateID) const
{
	if (const int32* Count = CrateOpenCounts.Find(CrateID))
	{
		return *Count;
	}
	return 0;
}

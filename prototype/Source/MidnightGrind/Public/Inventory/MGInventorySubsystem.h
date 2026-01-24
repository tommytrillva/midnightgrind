// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGInventorySubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGItemType : uint8
{
	Vehicle,
	VehiclePart,
	Cosmetic,
	Livery,
	Decal,
	Paint,
	Wheels,
	Spoiler,
	BodyKit,
	Neon,
	Interior,
	Horn,
	NitroEffect,
	TrailEffect,
	Badge,
	Banner,
	Avatar,
	Title,
	Emote,
	Currency,
	Consumable,
	Blueprint,
	Crate,
	Key
};

UENUM(BlueprintType)
enum class EMGItemRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary,
	Mythic,
	Exclusive
};

UENUM(BlueprintType)
enum class EMGItemSource : uint8
{
	Unknown,
	Store,
	Race,
	Challenge,
	Achievement,
	Crate,
	Trade,
	Gift,
	Craft,
	Event,
	SeasonPass,
	Referral,
	Promotion,
	Legacy
};

UENUM(BlueprintType)
enum class EMGSortMethod : uint8
{
	DateAcquired,
	DateAcquiredDesc,
	Name,
	NameDesc,
	Rarity,
	RarityDesc,
	Type,
	Value,
	ValueDesc,
	Favorite
};

USTRUCT(BlueprintType)
struct FMGInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ItemInstanceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemType ItemType = EMGItemType::Cosmetic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemRarity Rarity = EMGItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemSource Source = EMGItemSource::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStackable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTradeable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSellable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFavorite = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNew = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AcquiredAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExpires = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SellValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> PreviewMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompatibleVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomData;
};

USTRUCT(BlueprintType)
struct FMGVehicleInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleInstanceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VehicleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemRarity Rarity = EMGItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemSource Source = EMGItemSource::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AcquiredAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFavorite = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTradeable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FGuid> EquippedParts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EquippedLivery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerformanceIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleClass;
};

USTRUCT(BlueprintType)
struct FMGInventoryFilter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGItemType> AllowedTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGItemRarity> AllowedRarities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompatibleVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyTradeable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlySellable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyFavorites = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyNew = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SearchText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSortMethod SortMethod = EMGSortMethod::DateAcquiredDesc;
};

USTRUCT(BlueprintType)
struct FMGCrateContents
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrateID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CrateName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGInventoryItem> PossibleItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGItemRarity, float> RarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GuaranteedRarityAtCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGItemRarity GuaranteedRarity = EMGItemRarity::Rare;
};

USTRUCT(BlueprintType)
struct FMGCraftingRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RecipeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RecipeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> RequiredItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ResultItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ResultQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SuccessRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;
};

USTRUCT(BlueprintType)
struct FMGInventoryStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalVehicles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UniqueItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGItemType, int32> ItemsByType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGItemRarity, int32> ItemsByRarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CratesOwned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CratesOpened = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemsCrafted = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, const FMGInventoryItem&, Item, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, const FMGInventoryItem&, Item, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleAdded, const FMGVehicleInventoryEntry&, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleRemoved, FGuid, VehicleInstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, FGuid, VehicleInstanceID, const FMGInventoryItem&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrateOpened, FName, CrateID, const TArray<FMGInventoryItem>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemCrafted, FName, RecipeID, const FMGInventoryItem&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryFull, EMGItemType, ItemType);

UCLASS()
class MIDNIGHTGRIND_API UMGInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Item Management
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool AddItem(const FMGInventoryItem& Item, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool RemoveItem(FGuid ItemInstanceID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool RemoveItemByID(FName ItemID, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	bool HasItem(FName ItemID, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	FMGInventoryItem GetItem(FGuid ItemInstanceID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	TArray<FMGInventoryItem> GetItemsByID(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	TArray<FMGInventoryItem> GetAllItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Items")
	TArray<FMGInventoryItem> GetFilteredItems(const FMGInventoryFilter& Filter) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void MarkItemViewed(FGuid ItemInstanceID);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void MarkAllViewed();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void SetItemFavorite(FGuid ItemInstanceID, bool bFavorite);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void SetItemLocked(FGuid ItemInstanceID, bool bLocked);

	// Vehicle Management
	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	bool AddVehicle(const FMGVehicleInventoryEntry& Vehicle);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	bool RemoveVehicle(FGuid VehicleInstanceID);

	UFUNCTION(BlueprintPure, Category = "Inventory|Vehicles")
	bool HasVehicle(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Vehicles")
	FMGVehicleInventoryEntry GetVehicle(FGuid VehicleInstanceID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Vehicles")
	TArray<FMGVehicleInventoryEntry> GetAllVehicles() const { return Vehicles; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Vehicles")
	TArray<FMGVehicleInventoryEntry> GetVehiclesByClass(FName VehicleClass) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	void SetVehicleFavorite(FGuid VehicleInstanceID, bool bFavorite);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	void SetVehicleLocked(FGuid VehicleInstanceID, bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	void SetVehicleCustomName(FGuid VehicleInstanceID, const FString& CustomName);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Vehicles")
	void UpdateVehicleStats(FGuid VehicleInstanceID, float Distance, float TopSpeed, bool bWon);

	// Equipment
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItemToVehicle(FGuid VehicleInstanceID, FGuid ItemInstanceID, FName SlotName);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnequipFromVehicle(FGuid VehicleInstanceID, FName SlotName);

	UFUNCTION(BlueprintPure, Category = "Inventory|Equipment")
	FMGInventoryItem GetEquippedItem(FGuid VehicleInstanceID, FName SlotName) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Equipment")
	TMap<FName, FMGInventoryItem> GetAllEquippedItems(FGuid VehicleInstanceID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Equipment")
	TArray<FMGInventoryItem> GetCompatibleItems(FGuid VehicleInstanceID, FName SlotName) const;

	// Crates
	UFUNCTION(BlueprintCallable, Category = "Inventory|Crates")
	TArray<FMGInventoryItem> OpenCrate(FName CrateID);

	UFUNCTION(BlueprintPure, Category = "Inventory|Crates")
	bool CanOpenCrate(FName CrateID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Crates")
	FMGCrateContents GetCrateContents(FName CrateID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Crates")
	void RegisterCrateType(const FMGCrateContents& CrateContents);

	// Crafting
	UFUNCTION(BlueprintCallable, Category = "Inventory|Crafting")
	bool CraftItem(FName RecipeID);

	UFUNCTION(BlueprintPure, Category = "Inventory|Crafting")
	bool CanCraftItem(FName RecipeID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Crafting")
	TArray<FMGCraftingRecipe> GetAvailableRecipes() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Crafting")
	FMGCraftingRecipe GetRecipe(FName RecipeID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Crafting")
	void UnlockRecipe(FName RecipeID);

	// Selling
	UFUNCTION(BlueprintCallable, Category = "Inventory|Selling")
	bool SellItem(FGuid ItemInstanceID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Selling")
	bool SellVehicle(FGuid VehicleInstanceID);

	UFUNCTION(BlueprintPure, Category = "Inventory|Selling")
	int32 GetSellPrice(FGuid ItemInstanceID, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Selling")
	int32 GetVehicleSellPrice(FGuid VehicleInstanceID) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	FMGInventoryStats GetInventoryStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	int32 GetInventoryCapacity() const { return MaxInventorySlots; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	int32 GetUsedInventorySlots() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	bool IsInventoryFull() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	int32 GetGarageCapacity() const { return MaxGarageSlots; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Stats")
	int32 GetNewItemCount() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnVehicleAdded OnVehicleAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnVehicleRemoved OnVehicleRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemEquipped OnItemEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnCrateOpened OnCrateOpened;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemCrafted OnItemCrafted;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryFull OnInventoryFull;

protected:
	void UpdateStats();
	void SaveInventory();
	void LoadInventory();
	FMGInventoryItem RollCrateReward(const FMGCrateContents& Crate) const;
	int32 GetCrateOpenCount(FName CrateID) const;

	UPROPERTY()
	TArray<FMGInventoryItem> Items;

	UPROPERTY()
	TArray<FMGVehicleInventoryEntry> Vehicles;

	UPROPERTY()
	TMap<FName, FMGCrateContents> CrateTypes;

	UPROPERTY()
	TMap<FName, FMGCraftingRecipe> Recipes;

	UPROPERTY()
	TMap<FName, int32> CrateOpenCounts;

	UPROPERTY()
	FMGInventoryStats Stats;

	UPROPERTY()
	int32 MaxInventorySlots = 500;

	UPROPERTY()
	int32 MaxGarageSlots = 50;
};

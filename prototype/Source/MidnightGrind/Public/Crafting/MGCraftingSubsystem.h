// MidnightGrind - Arcade Street Racing Game
// Crafting Subsystem - Salvage, crafting, and item upgrading system

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCraftingSubsystem.generated.h"

// Forward declarations
class UMGCraftingSubsystem;

/**
 * EMGMaterialRarity - Rarity of crafting materials
 */
UENUM(BlueprintType)
enum class EMGMaterialRarity : uint8
{
    Common          UMETA(DisplayName = "Common"),
    Uncommon        UMETA(DisplayName = "Uncommon"),
    Rare            UMETA(DisplayName = "Rare"),
    Epic            UMETA(DisplayName = "Epic"),
    Legendary       UMETA(DisplayName = "Legendary")
};

/**
 * EMGMaterialType - Types of crafting materials
 */
UENUM(BlueprintType)
enum class EMGMaterialType : uint8
{
    Metal           UMETA(DisplayName = "Metal"),
    Carbon          UMETA(DisplayName = "Carbon Fiber"),
    Electronics     UMETA(DisplayName = "Electronics"),
    Fabric          UMETA(DisplayName = "Fabric"),
    Paint           UMETA(DisplayName = "Paint"),
    Glass           UMETA(DisplayName = "Glass"),
    Rubber          UMETA(DisplayName = "Rubber"),
    Chrome          UMETA(DisplayName = "Chrome"),
    NeonGas         UMETA(DisplayName = "Neon Gas"),
    PerformanceChip UMETA(DisplayName = "Performance Chip"),
    TurboCore       UMETA(DisplayName = "Turbo Core"),
    NitroCatalyst   UMETA(DisplayName = "Nitro Catalyst"),
    BlueprintScrap  UMETA(DisplayName = "Blueprint Scrap"),
    TokenFragment   UMETA(DisplayName = "Token Fragment")
};

/**
 * EMGCraftingCategory - Categories of craftable items
 */
UENUM(BlueprintType)
enum class EMGCraftingCategory : uint8
{
    BodyParts       UMETA(DisplayName = "Body Parts"),
    PerformanceParts UMETA(DisplayName = "Performance Parts"),
    Cosmetics       UMETA(DisplayName = "Cosmetics"),
    Vinyls          UMETA(DisplayName = "Vinyls"),
    Wheels          UMETA(DisplayName = "Wheels"),
    NeonKits        UMETA(DisplayName = "Neon Kits"),
    Upgrades        UMETA(DisplayName = "Upgrades"),
    Consumables     UMETA(DisplayName = "Consumables"),
    Tokens          UMETA(DisplayName = "Tokens")
};

/**
 * FMGCraftingMaterial - A crafting material
 */
USTRUCT(BlueprintType)
struct FMGCraftingMaterial
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MaterialId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMaterialType MaterialType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMaterialRarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SellValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> IconTexture;

    FMGCraftingMaterial()
        : MaterialId(NAME_None)
        , MaterialType(EMGMaterialType::Metal)
        , Rarity(EMGMaterialRarity::Common)
        , Quantity(0)
        , MaxStack(999)
        , SellValue(10)
    {}
};

/**
 * FMGRecipeIngredient - An ingredient required for crafting
 */
USTRUCT(BlueprintType)
struct FMGRecipeIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MaterialId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 QuantityRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BonusIfIncluded;

    FMGRecipeIngredient()
        : MaterialId(NAME_None)
        , QuantityRequired(1)
        , bIsOptional(false)
        , BonusIfIncluded(0.0f)
    {}
};

/**
 * FMGCraftingRecipe - A recipe for crafting an item
 */
USTRUCT(BlueprintType)
struct FMGCraftingRecipe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RecipeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCraftingCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMaterialRarity OutputRarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName OutputItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OutputQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGRecipeIngredient> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CraftingCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CraftingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TimesCrafted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> IconTexture;

    FMGCraftingRecipe()
        : RecipeId(NAME_None)
        , Category(EMGCraftingCategory::BodyParts)
        , OutputRarity(EMGMaterialRarity::Common)
        , OutputItemId(NAME_None)
        , OutputQuantity(1)
        , CraftingCost(100)
        , CraftingTime(5.0f)
        , RequiredLevel(1)
        , bRequiresBlueprint(false)
        , bIsUnlocked(true)
        , TimesCrafted(0)
    {}
};

/**
 * FMGSalvageResult - Result of salvaging an item
 */
USTRUCT(BlueprintType)
struct FMGSalvageResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SalvagedItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGCraftingMaterial> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrencyGained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 XPGained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBonusDrop;

    FMGSalvageResult()
        : SalvagedItemId(NAME_None)
        , CurrencyGained(0)
        , XPGained(0)
        , bBonusDrop(false)
    {}
};

/**
 * FMGCraftingQueue - Item in the crafting queue
 */
USTRUCT(BlueprintType)
struct FMGCraftingQueue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CraftId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RecipeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsClaimed;

    FMGCraftingQueue()
        : CraftId(TEXT(""))
        , RecipeId(NAME_None)
        , Quantity(1)
        , bIsComplete(false)
        , bIsClaimed(false)
    {}

    float GetProgress() const
    {
        FDateTime Now = FDateTime::Now();
        if (Now >= EndTime) return 1.0f;
        FTimespan Total = EndTime - StartTime;
        FTimespan Elapsed = Now - StartTime;
        return Total.GetTotalSeconds() > 0 ? Elapsed.GetTotalSeconds() / Total.GetTotalSeconds() : 0.0f;
    }

    FTimespan GetTimeRemaining() const
    {
        FDateTime Now = FDateTime::Now();
        return EndTime > Now ? EndTime - Now : FTimespan::Zero();
    }
};

/**
 * FMGUpgradeSlot - An upgrade slot for an item
 */
USTRUCT(BlueprintType)
struct FMGUpgradeSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SlotId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SlotName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentUpgradeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> CompatibleUpgrades;

    FMGUpgradeSlot()
        : SlotId(NAME_None)
        , CurrentUpgradeId(NAME_None)
        , Level(0)
        , MaxLevel(5)
    {}
};

/**
 * FMGCraftingStats - Player crafting statistics
 */
USTRUCT(BlueprintType)
struct FMGCraftingStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalItemsCrafted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalItemsSalvaged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CraftingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CraftingXP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 XPToNextLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RecipesUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RareItemsCrafted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BonusDropsReceived;

    FMGCraftingStats()
        : TotalItemsCrafted(0)
        , TotalItemsSalvaged(0)
        , CraftingLevel(1)
        , CraftingXP(0)
        , XPToNextLevel(1000)
        , RecipesUnlocked(0)
        , RareItemsCrafted(0)
        , BonusDropsReceived(0)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCraftingStarted, const FMGCraftingQueue&, CraftingItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCraftingComplete, const FMGCraftingQueue&, CraftingItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnItemCrafted, FName, ItemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnItemSalvaged, const FMGSalvageResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRecipeUnlocked, const FMGCraftingRecipe&, Recipe);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCraftingLevelUp, int32, NewLevel, TArray<FName>, UnlockedRecipes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnMaterialGained, FName, MaterialId, int32, Quantity);

/**
 * UMGCraftingSubsystem
 *
 * Manages crafting and salvaging for Midnight Grind.
 * Features include:
 * - Material collection and management
 * - Recipe-based crafting system
 * - Item salvaging for materials
 * - Crafting queue with timers
 * - Upgrade system
 * - Crafting level progression
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCraftingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGCraftingSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void TickCrafting(float DeltaTime);

    // ===== Materials =====

    UFUNCTION(BlueprintPure, Category = "Crafting|Materials")
    TArray<FMGCraftingMaterial> GetAllMaterials() const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Materials")
    FMGCraftingMaterial GetMaterial(FName MaterialId) const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Materials")
    int32 GetMaterialQuantity(FName MaterialId) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting|Materials")
    void AddMaterial(FName MaterialId, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Materials")
    bool RemoveMaterial(FName MaterialId, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Materials")
    int32 SellMaterial(FName MaterialId, int32 Quantity);

    // ===== Recipes =====

    UFUNCTION(BlueprintPure, Category = "Crafting|Recipes")
    TArray<FMGCraftingRecipe> GetAllRecipes() const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Recipes")
    TArray<FMGCraftingRecipe> GetRecipesByCategory(EMGCraftingCategory Category) const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Recipes")
    TArray<FMGCraftingRecipe> GetUnlockedRecipes() const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Recipes")
    FMGCraftingRecipe GetRecipe(FName RecipeId) const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Recipes")
    bool CanCraftRecipe(FName RecipeId) const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Recipes")
    bool HasIngredientsForRecipe(FName RecipeId) const;

    UFUNCTION(BlueprintCallable, Category = "Crafting|Recipes")
    bool UnlockRecipe(FName RecipeId);

    // ===== Crafting =====

    UFUNCTION(BlueprintCallable, Category = "Crafting|Craft")
    bool StartCrafting(FName RecipeId, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Craft")
    bool CancelCrafting(const FString& CraftId);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Craft")
    bool ClaimCraftedItem(const FString& CraftId);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Craft")
    bool SpeedUpCrafting(const FString& CraftId, int32 PremiumCurrencyCost);

    UFUNCTION(BlueprintPure, Category = "Crafting|Craft")
    TArray<FMGCraftingQueue> GetCraftingQueue() const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Craft")
    int32 GetQueueSize() const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Craft")
    int32 GetMaxQueueSize() const;

    // ===== Salvaging =====

    UFUNCTION(BlueprintCallable, Category = "Crafting|Salvage")
    FMGSalvageResult SalvageItem(FName ItemId);

    UFUNCTION(BlueprintCallable, Category = "Crafting|Salvage")
    TArray<FMGSalvageResult> SalvageItems(const TArray<FName>& ItemIds);

    UFUNCTION(BlueprintPure, Category = "Crafting|Salvage")
    FMGSalvageResult PreviewSalvage(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Salvage")
    bool CanSalvageItem(FName ItemId) const;

    // ===== Upgrades =====

    UFUNCTION(BlueprintCallable, Category = "Crafting|Upgrade")
    bool UpgradeItem(FName ItemId, FName SlotId);

    UFUNCTION(BlueprintPure, Category = "Crafting|Upgrade")
    TArray<FMGUpgradeSlot> GetUpgradeSlots(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Upgrade")
    int32 GetUpgradeCost(FName ItemId, FName SlotId) const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Upgrade")
    TArray<FMGRecipeIngredient> GetUpgradeMaterials(FName ItemId, FName SlotId) const;

    // ===== Stats =====

    UFUNCTION(BlueprintPure, Category = "Crafting|Stats")
    FMGCraftingStats GetCraftingStats() const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Stats")
    int32 GetCraftingLevel() const;

    UFUNCTION(BlueprintPure, Category = "Crafting|Stats")
    float GetCraftingLevelProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Crafting|Stats")
    void AddCraftingXP(int32 Amount);

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FMGOnCraftingStarted OnCraftingStarted;

    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FMGOnCraftingComplete OnCraftingComplete;

    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FMGOnItemCrafted OnItemCrafted;

    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FMGOnItemSalvaged OnItemSalvaged;

    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FMGOnRecipeUnlocked OnRecipeUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FMGOnCraftingLevelUp OnCraftingLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
    FMGOnMaterialGained OnMaterialGained;

protected:
    void InitializeMaterials();
    void InitializeRecipes();
    void ProcessCraftingQueue();
    void CheckLevelUp();
    int32 CalculateXPForLevel(int32 Level) const;
    float GetBonusDropChance() const;

private:
    UPROPERTY()
    TMap<FName, FMGCraftingMaterial> Materials;

    UPROPERTY()
    TMap<FName, FMGCraftingRecipe> Recipes;

    UPROPERTY()
    TArray<FMGCraftingQueue> CraftingQueue;

    UPROPERTY()
    TMap<FName, TArray<FMGUpgradeSlot>> ItemUpgrades;

    UPROPERTY()
    FMGCraftingStats Stats;

    UPROPERTY()
    int32 MaxCraftingQueueSize;

    FTimerHandle TickTimerHandle;
};

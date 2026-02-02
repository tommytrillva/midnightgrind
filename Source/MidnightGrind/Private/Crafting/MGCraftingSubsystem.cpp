// Copyright Midnight Grind. All Rights Reserved.

// MidnightGrind - Arcade Street Racing Game
// Crafting Subsystem - Implementation

#include "Crafting/MGCraftingSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMGCraftingSubsystem::UMGCraftingSubsystem()
    : MaxCraftingQueueSize(3)
{
}

void UMGCraftingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeMaterials();
    InitializeRecipes();

    Stats.CraftingLevel = 1;
    Stats.CraftingXP = 0;
    Stats.XPToNextLevel = CalculateXPForLevel(1);

    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGCraftingSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->TickCrafting(1.0f);
                }
            },
            1.0f,
            true
        );
    }
}

void UMGCraftingSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }

    Super::Deinitialize();
}

void UMGCraftingSubsystem::TickCrafting(float MGDeltaTime)
{
    ProcessCraftingQueue();
}

// ===== Materials =====

TArray<FMGCraftingMaterial> UMGCraftingSubsystem::GetAllMaterials() const
{
    TArray<FMGCraftingMaterial> MaterialList;
    for (const auto& Pair : Materials)
    {
        MaterialList.Add(Pair.Value);
    }
    return MaterialList;
}

FMGCraftingMaterial UMGCraftingSubsystem::GetMaterial(FName MaterialId) const
{
    if (const FMGCraftingMaterial* Material = Materials.Find(MaterialId))
    {
        return *Material;
    }
    return FMGCraftingMaterial();
}

int32 UMGCraftingSubsystem::GetMaterialQuantity(FName MaterialId) const
{
    if (const FMGCraftingMaterial* Material = Materials.Find(MaterialId))
    {
        return Material->Quantity;
    }
    return 0;
}

void UMGCraftingSubsystem::AddMaterial(FName MaterialId, int32 Quantity)
{
    if (FMGCraftingMaterial* Material = Materials.Find(MaterialId))
    {
        Material->Quantity = FMath::Min(Material->Quantity + Quantity, Material->MaxStack);
        OnMaterialGained.Broadcast(MaterialId, Quantity);
    }
}

bool UMGCraftingSubsystem::RemoveMaterial(FName MaterialId, int32 Quantity)
{
    FMGCraftingMaterial* Material = Materials.Find(MaterialId);
    if (!Material || Material->Quantity < Quantity)
    {
        return false;
    }

    Material->Quantity -= Quantity;
    return true;
}

int32 UMGCraftingSubsystem::SellMaterial(FName MaterialId, int32 Quantity)
{
    FMGCraftingMaterial* Material = Materials.Find(MaterialId);
    if (!Material || Material->Quantity < Quantity)
    {
        return 0;
    }

    int32 TotalValue = Material->SellValue * Quantity;
    Material->Quantity -= Quantity;
    return TotalValue;
}

// ===== Recipes =====

TArray<FMGCraftingRecipe> UMGCraftingSubsystem::GetAllRecipes() const
{
    TArray<FMGCraftingRecipe> RecipeList;
    for (const auto& Pair : Recipes)
    {
        RecipeList.Add(Pair.Value);
    }
    return RecipeList;
}

TArray<FMGCraftingRecipe> UMGCraftingSubsystem::GetRecipesByCategory(EMGCraftingCategory Category) const
{
    TArray<FMGCraftingRecipe> RecipeList;
    for (const auto& Pair : Recipes)
    {
        if (Pair.Value.Category == Category)
        {
            RecipeList.Add(Pair.Value);
        }
    }
    return RecipeList;
}

TArray<FMGCraftingRecipe> UMGCraftingSubsystem::GetUnlockedRecipes() const
{
    TArray<FMGCraftingRecipe> RecipeList;
    for (const auto& Pair : Recipes)
    {
        if (Pair.Value.bIsUnlocked)
        {
            RecipeList.Add(Pair.Value);
        }
    }
    return RecipeList;
}

FMGCraftingRecipe UMGCraftingSubsystem::GetRecipe(FName RecipeId) const
{
    if (const FMGCraftingRecipe* Recipe = Recipes.Find(RecipeId))
    {
        return *Recipe;
    }
    return FMGCraftingRecipe();
}

bool UMGCraftingSubsystem::CanCraftRecipe(FName RecipeId) const
{
    const FMGCraftingRecipe* Recipe = Recipes.Find(RecipeId);
    if (!Recipe || !Recipe->bIsUnlocked)
    {
        return false;
    }

    if (Recipe->RequiredLevel > Stats.CraftingLevel)
    {
        return false;
    }

    return HasIngredientsForRecipe(RecipeId);
}

bool UMGCraftingSubsystem::HasIngredientsForRecipe(FName RecipeId) const
{
    const FMGCraftingRecipe* Recipe = Recipes.Find(RecipeId);
    if (!Recipe)
    {
        return false;
    }

    for (const FMGRecipeIngredient& Ingredient : Recipe->Ingredients)
    {
        if (!Ingredient.bIsOptional && GetMaterialQuantity(Ingredient.MaterialId) < Ingredient.QuantityRequired)
        {
            return false;
        }
    }

    return true;
}

bool UMGCraftingSubsystem::UnlockRecipe(FName RecipeId)
{
    FMGCraftingRecipe* Recipe = Recipes.Find(RecipeId);
    if (!Recipe || Recipe->bIsUnlocked)
    {
        return false;
    }

    Recipe->bIsUnlocked = true;
    Stats.RecipesUnlocked++;

    OnRecipeUnlocked.Broadcast(*Recipe);
    return true;
}

// ===== Crafting =====

bool UMGCraftingSubsystem::StartCrafting(FName RecipeId, int32 Quantity)
{
    if (CraftingQueue.Num() >= MaxCraftingQueueSize)
    {
        return false;
    }

    if (!CanCraftRecipe(RecipeId))
    {
        return false;
    }

    const FMGCraftingRecipe* Recipe = Recipes.Find(RecipeId);
    if (!Recipe)
    {
        return false;
    }

    // Consume ingredients
    for (const FMGRecipeIngredient& Ingredient : Recipe->Ingredients)
    {
        if (!Ingredient.bIsOptional)
        {
            RemoveMaterial(Ingredient.MaterialId, Ingredient.QuantityRequired * Quantity);
        }
    }

    // Create queue item
    FMGCraftingQueue QueueItem;
    QueueItem.CraftId = FGuid::NewGuid().ToString();
    QueueItem.RecipeId = RecipeId;
    QueueItem.ItemName = Recipe->DisplayName;
    QueueItem.StartTime = FDateTime::Now();
    QueueItem.EndTime = FDateTime::Now() + FTimespan::FromSeconds(Recipe->CraftingTime * Quantity);
    QueueItem.Quantity = Quantity;

    CraftingQueue.Add(QueueItem);

    OnCraftingStarted.Broadcast(QueueItem);
    return true;
}

bool UMGCraftingSubsystem::CancelCrafting(const FString& CraftId)
{
    for (int32 i = 0; i < CraftingQueue.Num(); i++)
    {
        if (CraftingQueue[i].CraftId == CraftId && !CraftingQueue[i].bIsComplete)
        {
            // Refund partial materials
            const FMGCraftingRecipe* Recipe = Recipes.Find(CraftingQueue[i].RecipeId);
            if (Recipe)
            {
                float Progress = CraftingQueue[i].GetProgress();
                float RefundPercent = 1.0f - Progress;

                for (const FMGRecipeIngredient& Ingredient : Recipe->Ingredients)
                {
                    int32 RefundAmount = FMath::RoundToInt(Ingredient.QuantityRequired * RefundPercent * CraftingQueue[i].Quantity);
                    if (RefundAmount > 0)
                    {
                        AddMaterial(Ingredient.MaterialId, RefundAmount);
                    }
                }
            }

            CraftingQueue.RemoveAt(i);
            return true;
        }
    }
    return false;
}

bool UMGCraftingSubsystem::ClaimCraftedItem(const FString& CraftId)
{
    for (int32 i = 0; i < CraftingQueue.Num(); i++)
    {
        if (CraftingQueue[i].CraftId == CraftId && CraftingQueue[i].bIsComplete && !CraftingQueue[i].bIsClaimed)
        {
            CraftingQueue[i].bIsClaimed = true;

            const FMGCraftingRecipe* Recipe = Recipes.Find(CraftingQueue[i].RecipeId);
            if (Recipe)
            {
                Stats.TotalItemsCrafted += CraftingQueue[i].Quantity;

                if (Recipe->OutputRarity >= EMGMaterialRarity::Rare)
                {
                    Stats.RareItemsCrafted += CraftingQueue[i].Quantity;
                }

                // Grant XP
                int32 XPGain = 50 * (static_cast<int32>(Recipe->OutputRarity) + 1) * CraftingQueue[i].Quantity;
                AddCraftingXP(XPGain);

                OnItemCrafted.Broadcast(Recipe->OutputItemId);

                // Update recipe stats
                FMGCraftingRecipe* MutableRecipe = Recipes.Find(CraftingQueue[i].RecipeId);
                if (MutableRecipe)
                {
                    MutableRecipe->TimesCrafted += CraftingQueue[i].Quantity;
                }
            }

            CraftingQueue.RemoveAt(i);
            return true;
        }
    }
    return false;
}

bool UMGCraftingSubsystem::SpeedUpCrafting(const FString& CraftId, int32 PremiumCurrencyCost)
{
    for (FMGCraftingQueue& QueueItem : CraftingQueue)
    {
        if (QueueItem.CraftId == CraftId && !QueueItem.bIsComplete)
        {
            QueueItem.EndTime = FDateTime::Now();
            QueueItem.bIsComplete = true;
            OnCraftingComplete.Broadcast(QueueItem);
            return true;
        }
    }
    return false;
}

TArray<FMGCraftingQueue> UMGCraftingSubsystem::GetCraftingQueue() const
{
    return CraftingQueue;
}

int32 UMGCraftingSubsystem::GetQueueSize() const
{
    return CraftingQueue.Num();
}

int32 UMGCraftingSubsystem::GetMaxQueueSize() const
{
    return MaxCraftingQueueSize;
}

// ===== Salvaging =====

FMGSalvageResult UMGCraftingSubsystem::SalvageItem(FName ItemId)
{
    FMGSalvageResult Result;
    Result.SalvagedItemId = ItemId;

    // Generate materials based on item
    bool bBonusDrop = FMath::FRand() < GetBonusDropChance();
    Result.bBonusDrop = bBonusDrop;

    // Sample salvage logic - in production would look up item data
    FMGCraftingMaterial MetalMaterial;
    MetalMaterial.MaterialId = FName("mat_metal_scrap");
    MetalMaterial.Quantity = FMath::RandRange(5, 15);
    if (bBonusDrop)
    {
        MetalMaterial.Quantity = FMath::RoundToInt(MetalMaterial.Quantity * 1.5f);
        Stats.BonusDropsReceived++;
    }
    Result.Materials.Add(MetalMaterial);

    // Add electronics occasionally
    if (FMath::FRand() < 0.3f)
    {
        FMGCraftingMaterial ElectronicsMaterial;
        ElectronicsMaterial.MaterialId = FName("mat_electronics");
        ElectronicsMaterial.Quantity = FMath::RandRange(1, 3);
        Result.Materials.Add(ElectronicsMaterial);
    }

    Result.CurrencyGained = FMath::RandRange(50, 150);
    Result.XPGained = FMath::RandRange(10, 30);

    // Apply results
    for (const FMGCraftingMaterial& Material : Result.Materials)
    {
        AddMaterial(Material.MaterialId, Material.Quantity);
    }

    AddCraftingXP(Result.XPGained);
    Stats.TotalItemsSalvaged++;

    OnItemSalvaged.Broadcast(Result);
    return Result;
}

TArray<FMGSalvageResult> UMGCraftingSubsystem::SalvageItems(const TArray<FName>& ItemIds)
{
    TArray<FMGSalvageResult> Results;
    for (const FName& ItemId : ItemIds)
    {
        Results.Add(SalvageItem(ItemId));
    }
    return Results;
}

FMGSalvageResult UMGCraftingSubsystem::PreviewSalvage(FName ItemId) const
{
    FMGSalvageResult Preview;
    Preview.SalvagedItemId = ItemId;

    // Show estimated materials
    FMGCraftingMaterial MetalMaterial;
    MetalMaterial.MaterialId = FName("mat_metal_scrap");
    MetalMaterial.Quantity = 10;
    Preview.Materials.Add(MetalMaterial);

    Preview.CurrencyGained = 100;
    Preview.XPGained = 20;

    return Preview;
}

bool UMGCraftingSubsystem::CanSalvageItem(FName ItemId) const
{
    // In production, would check item data
    return true;
}

// ===== Upgrades =====

bool UMGCraftingSubsystem::UpgradeItem(FName ItemId, FName SlotId)
{
    TArray<FMGUpgradeSlot>* Slots = ItemUpgrades.Find(ItemId);
    if (!Slots)
    {
        return false;
    }

    for (FMGUpgradeSlot& Slot : *Slots)
    {
        if (Slot.SlotId == SlotId && Slot.Level < Slot.MaxLevel)
        {
            // Check materials
            TArray<FMGRecipeIngredient> RequiredMaterials = GetUpgradeMaterials(ItemId, SlotId);
            for (const FMGRecipeIngredient& Ingredient : RequiredMaterials)
            {
                if (GetMaterialQuantity(Ingredient.MaterialId) < Ingredient.QuantityRequired)
                {
                    return false;
                }
            }

            // Consume materials
            for (const FMGRecipeIngredient& Ingredient : RequiredMaterials)
            {
                RemoveMaterial(Ingredient.MaterialId, Ingredient.QuantityRequired);
            }

            Slot.Level++;
            AddCraftingXP(100 * Slot.Level);
            return true;
        }
    }

    return false;
}

TArray<FMGUpgradeSlot> UMGCraftingSubsystem::GetUpgradeSlots(FName ItemId) const
{
    if (const TArray<FMGUpgradeSlot>* Slots = ItemUpgrades.Find(ItemId))
    {
        return *Slots;
    }
    return TArray<FMGUpgradeSlot>();
}

int32 UMGCraftingSubsystem::GetUpgradeCost(FName ItemId, FName SlotId) const
{
    if (const TArray<FMGUpgradeSlot>* Slots = ItemUpgrades.Find(ItemId))
    {
        for (const FMGUpgradeSlot& Slot : *Slots)
        {
            if (Slot.SlotId == SlotId)
            {
                return 500 * (Slot.Level + 1);
            }
        }
    }
    return 0;
}

TArray<FMGRecipeIngredient> UMGCraftingSubsystem::GetUpgradeMaterials(FName ItemId, FName SlotId) const
{
    TArray<FMGRecipeIngredient> RequiredMaterials;

    if (const TArray<FMGUpgradeSlot>* Slots = ItemUpgrades.Find(ItemId))
    {
        for (const FMGUpgradeSlot& Slot : *Slots)
        {
            if (Slot.SlotId == SlotId)
            {
                FMGRecipeIngredient Ingredient;
                Ingredient.MaterialId = FName("mat_metal_scrap");
                Ingredient.QuantityRequired = 10 * (Slot.Level + 1);
                RequiredMaterials.Add(Ingredient);

                if (Slot.Level >= 2)
                {
                    FMGRecipeIngredient RareIngredient;
                    RareIngredient.MaterialId = FName("mat_turbo_core");
                    RareIngredient.QuantityRequired = Slot.Level;
                    RequiredMaterials.Add(RareIngredient);
                }
                break;
            }
        }
    }

    return RequiredMaterials;
}

// ===== Stats =====

FMGCraftingStats UMGCraftingSubsystem::GetCraftingStats() const
{
    return Stats;
}

int32 UMGCraftingSubsystem::GetCraftingLevel() const
{
    return Stats.CraftingLevel;
}

float UMGCraftingSubsystem::GetCraftingLevelProgress() const
{
    return Stats.XPToNextLevel > 0 ? static_cast<float>(Stats.CraftingXP) / Stats.XPToNextLevel : 0.0f;
}

void UMGCraftingSubsystem::AddCraftingXP(int32 Amount)
{
    Stats.CraftingXP += Amount;
    CheckLevelUp();
}

// ===== Protected =====

void UMGCraftingSubsystem::InitializeMaterials()
{
    // Metal Scrap
    {
        FMGCraftingMaterial Metal;
        Metal.MaterialId = FName("mat_metal_scrap");
        Metal.DisplayName = FText::FromString(TEXT("Metal Scrap"));
        Metal.Description = FText::FromString(TEXT("Basic metal salvaged from vehicles. Used in most crafting recipes."));
        Metal.MaterialType = EMGMaterialType::Metal;
        Metal.Rarity = EMGMaterialRarity::Common;
        Metal.Quantity = 50;
        Metal.SellValue = 5;
        Materials.Add(Metal.MaterialId, Metal);
    }

    // Carbon Fiber
    {
        FMGCraftingMaterial Carbon;
        Carbon.MaterialId = FName("mat_carbon_fiber");
        Carbon.DisplayName = FText::FromString(TEXT("Carbon Fiber"));
        Carbon.Description = FText::FromString(TEXT("Lightweight and strong. Essential for performance parts."));
        Carbon.MaterialType = EMGMaterialType::Carbon;
        Carbon.Rarity = EMGMaterialRarity::Uncommon;
        Carbon.Quantity = 20;
        Carbon.SellValue = 15;
        Materials.Add(Carbon.MaterialId, Carbon);
    }

    // Electronics
    {
        FMGCraftingMaterial Electronics;
        Electronics.MaterialId = FName("mat_electronics");
        Electronics.DisplayName = FText::FromString(TEXT("Electronics"));
        Electronics.Description = FText::FromString(TEXT("Various electronic components for ECUs and sensors."));
        Electronics.MaterialType = EMGMaterialType::Electronics;
        Electronics.Rarity = EMGMaterialRarity::Uncommon;
        Electronics.Quantity = 15;
        Electronics.SellValue = 20;
        Materials.Add(Electronics.MaterialId, Electronics);
    }

    // Turbo Core
    {
        FMGCraftingMaterial TurboCore;
        TurboCore.MaterialId = FName("mat_turbo_core");
        TurboCore.DisplayName = FText::FromString(TEXT("Turbo Core"));
        TurboCore.Description = FText::FromString(TEXT("High-quality turbine assembly for forced induction systems."));
        TurboCore.MaterialType = EMGMaterialType::TurboCore;
        TurboCore.Rarity = EMGMaterialRarity::Rare;
        TurboCore.Quantity = 5;
        TurboCore.SellValue = 100;
        Materials.Add(TurboCore.MaterialId, TurboCore);
    }

    // Neon Gas
    {
        FMGCraftingMaterial NeonGas;
        NeonGas.MaterialId = FName("mat_neon_gas");
        NeonGas.DisplayName = FText::FromString(TEXT("Neon Gas Canister"));
        NeonGas.Description = FText::FromString(TEXT("Pressurized neon gas for underglow kits."));
        NeonGas.MaterialType = EMGMaterialType::NeonGas;
        NeonGas.Rarity = EMGMaterialRarity::Common;
        NeonGas.Quantity = 30;
        NeonGas.SellValue = 10;
        Materials.Add(NeonGas.MaterialId, NeonGas);
    }

    // Performance Chip
    {
        FMGCraftingMaterial Chip;
        Chip.MaterialId = FName("mat_performance_chip");
        Chip.DisplayName = FText::FromString(TEXT("Performance Chip"));
        Chip.Description = FText::FromString(TEXT("Advanced ECU chip for performance tuning."));
        Chip.MaterialType = EMGMaterialType::PerformanceChip;
        Chip.Rarity = EMGMaterialRarity::Epic;
        Chip.Quantity = 3;
        Chip.SellValue = 250;
        Materials.Add(Chip.MaterialId, Chip);
    }
}

void UMGCraftingSubsystem::InitializeRecipes()
{
    // Basic Spoiler
    {
        FMGCraftingRecipe Spoiler;
        Spoiler.RecipeId = FName("recipe_spoiler_basic");
        Spoiler.DisplayName = FText::FromString(TEXT("Basic Rear Spoiler"));
        Spoiler.Description = FText::FromString(TEXT("A functional rear spoiler for improved downforce."));
        Spoiler.Category = EMGCraftingCategory::BodyParts;
        Spoiler.OutputRarity = EMGMaterialRarity::Common;
        Spoiler.OutputItemId = FName("part_spoiler_basic");
        Spoiler.CraftingCost = 100;
        Spoiler.CraftingTime = 30.0f;
        Spoiler.RequiredLevel = 1;
        Spoiler.bIsUnlocked = true;

        FMGRecipeIngredient Metal;
        Metal.MaterialId = FName("mat_metal_scrap");
        Metal.QuantityRequired = 10;
        Spoiler.Ingredients.Add(Metal);

        Recipes.Add(Spoiler.RecipeId, Spoiler);
    }

    // Carbon Fiber Hood
    {
        FMGCraftingRecipe Hood;
        Hood.RecipeId = FName("recipe_hood_carbon");
        Hood.DisplayName = FText::FromString(TEXT("Carbon Fiber Hood"));
        Hood.Description = FText::FromString(TEXT("Lightweight carbon fiber hood with ventilation."));
        Hood.Category = EMGCraftingCategory::BodyParts;
        Hood.OutputRarity = EMGMaterialRarity::Rare;
        Hood.OutputItemId = FName("part_hood_carbon");
        Hood.CraftingCost = 500;
        Hood.CraftingTime = 120.0f;
        Hood.RequiredLevel = 5;
        Hood.bIsUnlocked = true;

        FMGRecipeIngredient Carbon;
        Carbon.MaterialId = FName("mat_carbon_fiber");
        Carbon.QuantityRequired = 15;
        Hood.Ingredients.Add(Carbon);

        FMGRecipeIngredient Metal;
        Metal.MaterialId = FName("mat_metal_scrap");
        Metal.QuantityRequired = 5;
        Hood.Ingredients.Add(Metal);

        Recipes.Add(Hood.RecipeId, Hood);
    }

    // Turbo Kit
    {
        FMGCraftingRecipe Turbo;
        Turbo.RecipeId = FName("recipe_turbo_kit");
        Turbo.DisplayName = FText::FromString(TEXT("Turbo Kit"));
        Turbo.Description = FText::FromString(TEXT("Complete turbo kit for forced induction."));
        Turbo.Category = EMGCraftingCategory::PerformanceParts;
        Turbo.OutputRarity = EMGMaterialRarity::Epic;
        Turbo.OutputItemId = FName("part_turbo_kit");
        Turbo.CraftingCost = 2000;
        Turbo.CraftingTime = 300.0f;
        Turbo.RequiredLevel = 10;
        Turbo.bRequiresBlueprint = true;
        Turbo.bIsUnlocked = false;

        FMGRecipeIngredient TurboCore;
        TurboCore.MaterialId = FName("mat_turbo_core");
        TurboCore.QuantityRequired = 3;
        Turbo.Ingredients.Add(TurboCore);

        FMGRecipeIngredient Metal;
        Metal.MaterialId = FName("mat_metal_scrap");
        Metal.QuantityRequired = 20;
        Turbo.Ingredients.Add(Metal);

        FMGRecipeIngredient Electronics;
        Electronics.MaterialId = FName("mat_electronics");
        Electronics.QuantityRequired = 5;
        Turbo.Ingredients.Add(Electronics);

        Recipes.Add(Turbo.RecipeId, Turbo);
    }

    // Neon Underglow Kit
    {
        FMGCraftingRecipe Neon;
        Neon.RecipeId = FName("recipe_neon_underglow");
        Neon.DisplayName = FText::FromString(TEXT("Neon Underglow Kit"));
        Neon.Description = FText::FromString(TEXT("Multi-color neon underglow system."));
        Neon.Category = EMGCraftingCategory::NeonKits;
        Neon.OutputRarity = EMGMaterialRarity::Uncommon;
        Neon.OutputItemId = FName("cosmetic_neon_underglow");
        Neon.CraftingCost = 300;
        Neon.CraftingTime = 60.0f;
        Neon.RequiredLevel = 3;
        Neon.bIsUnlocked = true;

        FMGRecipeIngredient NeonGas;
        NeonGas.MaterialId = FName("mat_neon_gas");
        NeonGas.QuantityRequired = 10;
        Neon.Ingredients.Add(NeonGas);

        FMGRecipeIngredient Electronics;
        Electronics.MaterialId = FName("mat_electronics");
        Electronics.QuantityRequired = 3;
        Neon.Ingredients.Add(Electronics);

        Recipes.Add(Neon.RecipeId, Neon);
    }
}

void UMGCraftingSubsystem::ProcessCraftingQueue()
{
    FDateTime Now = FDateTime::Now();

    for (FMGCraftingQueue& QueueItem : CraftingQueue)
    {
        if (!QueueItem.bIsComplete && Now >= QueueItem.EndTime)
        {
            QueueItem.bIsComplete = true;
            OnCraftingComplete.Broadcast(QueueItem);
        }
    }
}

void UMGCraftingSubsystem::CheckLevelUp()
{
    while (Stats.CraftingXP >= Stats.XPToNextLevel && Stats.CraftingLevel < 50)
    {
        Stats.CraftingXP -= Stats.XPToNextLevel;
        Stats.CraftingLevel++;
        Stats.XPToNextLevel = CalculateXPForLevel(Stats.CraftingLevel);

        // Unlock recipes at certain levels
        TArray<FName> UnlockedRecipes;
        for (auto& Pair : Recipes)
        {
            if (!Pair.Value.bIsUnlocked && Pair.Value.RequiredLevel <= Stats.CraftingLevel && !Pair.Value.bRequiresBlueprint)
            {
                Pair.Value.bIsUnlocked = true;
                UnlockedRecipes.Add(Pair.Key);
                Stats.RecipesUnlocked++;
            }
        }

        // Increase queue size every 10 levels
        if (Stats.CraftingLevel % 10 == 0)
        {
            MaxCraftingQueueSize++;
        }

        OnCraftingLevelUp.Broadcast(Stats.CraftingLevel, UnlockedRecipes);
    }
}

int32 UMGCraftingSubsystem::CalculateXPForLevel(int32 Level) const
{
    return 1000 + (Level * 500);
}

float UMGCraftingSubsystem::GetBonusDropChance() const
{
    return 0.05f + (Stats.CraftingLevel * 0.01f);
}

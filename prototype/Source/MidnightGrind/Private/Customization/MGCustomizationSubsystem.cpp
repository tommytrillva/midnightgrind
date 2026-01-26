// Copyright Midnight Grind. All Rights Reserved.

#include "Customization/MGCustomizationSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "Components/MeshComponent.h"

void UMGCustomizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializePresetColors();
	LoadCustomizationData();
}

void UMGCustomizationSubsystem::Deinitialize()
{
	SaveCustomizationData();
	Super::Deinitialize();
}

// ==========================================
// CUSTOMIZATION MANAGEMENT
// ==========================================

FMGVehicleCustomization UMGCustomizationSubsystem::GetVehicleCustomization(FName VehicleID) const
{
	if (const FMGVehicleCustomization* Customization = VehicleCustomizations.Find(VehicleID))
	{
		return *Customization;
	}
	return FMGVehicleCustomization();
}

void UMGCustomizationSubsystem::SetVehicleCustomization(FName VehicleID, const FMGVehicleCustomization& Customization)
{
	VehicleCustomizations.Add(VehicleID, Customization);
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Paint);
}

void UMGCustomizationSubsystem::ResetToDefault(FName VehicleID)
{
	FMGVehicleCustomization DefaultCustomization;
	DefaultCustomization.VehicleID = VehicleID;
	VehicleCustomizations.Add(VehicleID, DefaultCustomization);
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Paint);
}

void UMGCustomizationSubsystem::SaveCustomization(FName VehicleID)
{
	SaveCustomizationData();
	OnCustomizationSaved.Broadcast(VehicleID);
}

// ==========================================
// PAINT
// ==========================================

void UMGCustomizationSubsystem::SetPaintConfig(FName VehicleID, const FMGPaintConfig& Paint)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Paint = Paint;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Paint);
}

void UMGCustomizationSubsystem::SetPrimaryColor(FName VehicleID, FLinearColor Color)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Paint.PrimaryColor = Color;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Paint);
}

void UMGCustomizationSubsystem::SetSecondaryColor(FName VehicleID, FLinearColor Color)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Paint.SecondaryColor = Color;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Paint);
}

void UMGCustomizationSubsystem::SetPaintFinish(FName VehicleID, EMGPaintFinish Finish)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Paint.Finish = Finish;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Paint);
}

TArray<FLinearColor> UMGCustomizationSubsystem::GetPresetColors() const
{
	return PresetColors;
}

// ==========================================
// WRAP
// ==========================================

void UMGCustomizationSubsystem::SetWrap(FName VehicleID, const FMGWrapConfig& Wrap)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Wrap = Wrap;
	Customization.bUsingWrap = true;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Wrap);
}

void UMGCustomizationSubsystem::RemoveWrap(FName VehicleID)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.bUsingWrap = false;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Wrap);
}

void UMGCustomizationSubsystem::SetWrapColors(FName VehicleID, FLinearColor Primary, FLinearColor Secondary)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Wrap.PrimaryTint = Primary;
	Customization.Wrap.SecondaryTint = Secondary;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Wrap);
}

// ==========================================
// DECALS
// ==========================================

int32 UMGCustomizationSubsystem::AddDecal(FName VehicleID, const FMGDecalPlacement& Decal)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);

	if (Customization.Decals.Num() >= MaxDecalsPerVehicle)
	{
		return -1;
	}

	int32 Index = Customization.Decals.Add(Decal);
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Decal);
	return Index;
}

void UMGCustomizationSubsystem::RemoveDecal(FName VehicleID, int32 DecalIndex)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);

	if (DecalIndex >= 0 && DecalIndex < Customization.Decals.Num())
	{
		Customization.Decals.RemoveAt(DecalIndex);
		NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Decal);
	}
}

void UMGCustomizationSubsystem::UpdateDecal(FName VehicleID, int32 DecalIndex, const FMGDecalPlacement& Decal)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);

	if (DecalIndex >= 0 && DecalIndex < Customization.Decals.Num())
	{
		Customization.Decals[DecalIndex] = Decal;
		NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Decal);
	}
}

void UMGCustomizationSubsystem::ClearAllDecals(FName VehicleID)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Decals.Empty();
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Decal);
}

// ==========================================
// PARTS
// ==========================================

void UMGCustomizationSubsystem::SetPart(FName VehicleID, EMGCustomizationCategory Category, const FMGPartConfig& Part)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Parts.Add(Category, Part);
	NotifyCustomizationChanged(VehicleID, Category);
}

void UMGCustomizationSubsystem::RemovePart(FName VehicleID, EMGCustomizationCategory Category)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Parts.Remove(Category);
	NotifyCustomizationChanged(VehicleID, Category);
}

TArray<FMGCustomizationItem> UMGCustomizationSubsystem::GetAvailableParts(FName VehicleID, EMGCustomizationCategory Category) const
{
	TArray<FMGCustomizationItem> Available;

	for (const auto& Pair : AllItems)
	{
		const FMGCustomizationItem& Item = Pair.Value;

		if (Item.Category != Category)
		{
			continue;
		}

		// Check vehicle compatibility
		if (Item.CompatibleVehicles.Num() > 0 && !Item.CompatibleVehicles.Contains(VehicleID))
		{
			continue;
		}

		// Check if owned
		FMGCustomizationItem ItemCopy = Item;
		ItemCopy.bIsOwned = OwnedItems.Contains(Item.ItemID);

		Available.Add(ItemCopy);
	}

	return Available;
}

// ==========================================
// WHEELS
// ==========================================

void UMGCustomizationSubsystem::SetWheels(FName VehicleID, const FMGWheelConfig& Wheels)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Wheels = Wheels;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Wheels);
}

void UMGCustomizationSubsystem::SetWheelColor(FName VehicleID, FLinearColor Color, EMGPaintFinish Finish)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Wheels.RimColor = Color;
	Customization.Wheels.RimFinish = Finish;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Wheels);
}

// ==========================================
// LIGHTING
// ==========================================

void UMGCustomizationSubsystem::SetLightingConfig(FName VehicleID, const FMGLightingConfig& Lighting)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Lighting = Lighting;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Underglow);
}

void UMGCustomizationSubsystem::SetUnderglowEnabled(FName VehicleID, bool bEnabled)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Lighting.bUnderglowEnabled = bEnabled;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Underglow);
}

void UMGCustomizationSubsystem::SetUnderglowColor(FName VehicleID, FLinearColor Color)
{
	FMGVehicleCustomization& Customization = GetOrCreateCustomization(VehicleID);
	Customization.Lighting.UnderglowColor = Color;
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Underglow);
}

// ==========================================
// MATERIAL GENERATION
// ==========================================

UMaterialInstanceDynamic* UMGCustomizationSubsystem::CreatePaintMaterial(const FMGPaintConfig& Paint)
{
	if (!BasePaintMaterial)
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* MatInstance = UMaterialInstanceDynamic::Create(BasePaintMaterial, this);
	if (!MatInstance)
	{
		return nullptr;
	}

	// Set parameters
	MatInstance->SetVectorParameterValue(FName("PrimaryColor"), Paint.PrimaryColor);
	MatInstance->SetVectorParameterValue(FName("SecondaryColor"), Paint.SecondaryColor);
	MatInstance->SetScalarParameterValue(FName("Metallic"), Paint.MetallicIntensity);
	MatInstance->SetScalarParameterValue(FName("ClearCoat"), Paint.ClearCoat);
	MatInstance->SetScalarParameterValue(FName("FlakeIntensity"), Paint.FlakeIntensity);
	MatInstance->SetVectorParameterValue(FName("PearlShift"), Paint.PearlShift);

	// Set roughness based on finish
	float Roughness = 0.3f;
	switch (Paint.Finish)
	{
		case EMGPaintFinish::Matte: Roughness = 0.8f; break;
		case EMGPaintFinish::Gloss: Roughness = 0.1f; break;
		case EMGPaintFinish::Metallic: Roughness = 0.2f; break;
		case EMGPaintFinish::Pearl: Roughness = 0.15f; break;
		case EMGPaintFinish::Chrome: Roughness = 0.05f; break;
		case EMGPaintFinish::Brushed: Roughness = 0.4f; break;
		case EMGPaintFinish::Satin: Roughness = 0.5f; break;
		case EMGPaintFinish::Carbon: Roughness = 0.3f; break;
	}
	MatInstance->SetScalarParameterValue(FName("Roughness"), Roughness);

	return MatInstance;
}

UMaterialInstanceDynamic* UMGCustomizationSubsystem::CreateWrapMaterial(const FMGWrapConfig& Wrap, const FMGPaintConfig& BasePaint)
{
	if (!BaseWrapMaterial)
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* MatInstance = UMaterialInstanceDynamic::Create(BaseWrapMaterial, this);
	if (!MatInstance)
	{
		return nullptr;
	}

	// Set wrap texture
	if (Wrap.WrapTexture)
	{
		MatInstance->SetTextureParameterValue(FName("WrapTexture"), Wrap.WrapTexture);
	}

	// Set colors
	MatInstance->SetVectorParameterValue(FName("PrimaryTint"), Wrap.PrimaryTint);
	MatInstance->SetVectorParameterValue(FName("SecondaryTint"), Wrap.SecondaryTint);

	// Set UV transforms
	MatInstance->SetScalarParameterValue(FName("ScaleU"), Wrap.Scale.X);
	MatInstance->SetScalarParameterValue(FName("ScaleV"), Wrap.Scale.Y);
	MatInstance->SetScalarParameterValue(FName("OffsetU"), Wrap.Offset.X);
	MatInstance->SetScalarParameterValue(FName("OffsetV"), Wrap.Offset.Y);
	MatInstance->SetScalarParameterValue(FName("Rotation"), Wrap.Rotation);

	// Roughness
	MatInstance->SetScalarParameterValue(FName("Roughness"), Wrap.bGlossy ? 0.1f : 0.5f);

	return MatInstance;
}

void UMGCustomizationSubsystem::ApplyCustomizationToVehicle(AActor* VehicleActor, const FMGVehicleCustomization& Customization)
{
	if (!VehicleActor)
	{
		return;
	}

	// Create appropriate material
	UMaterialInstanceDynamic* BodyMaterial = nullptr;
	if (Customization.bUsingWrap)
	{
		BodyMaterial = CreateWrapMaterial(Customization.Wrap, Customization.Paint);
	}
	else
	{
		BodyMaterial = CreatePaintMaterial(Customization.Paint);
	}

	// Apply to body mesh components
	TArray<UMeshComponent*> MeshComponents;
	VehicleActor->GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* Mesh : MeshComponents)
	{
		// Would check component tags to identify body panels
		if (Mesh->ComponentHasTag(FName("Body")))
		{
			if (BodyMaterial)
			{
				Mesh->SetMaterial(0, BodyMaterial);
			}
		}
	}

	// Apply wheel customization
	// Would find wheel components and apply wheel mesh/material

	// Apply lighting
	if (Customization.Lighting.bUnderglowEnabled)
	{
		// Would enable underglow light components
	}

	// Apply parts
	for (const auto& PartPair : Customization.Parts)
	{
		// Would swap mesh components for custom parts
	}
}

// ==========================================
// INVENTORY
// ==========================================

TArray<FMGCustomizationItem> UMGCustomizationSubsystem::GetOwnedItems(EMGCustomizationCategory Category) const
{
	TArray<FMGCustomizationItem> Owned;

	for (const FName& ItemID : OwnedItems)
	{
		if (const FMGCustomizationItem* Item = AllItems.Find(ItemID))
		{
			if (Item->Category == Category)
			{
				FMGCustomizationItem ItemCopy = *Item;
				ItemCopy.bIsOwned = true;
				Owned.Add(ItemCopy);
			}
		}
	}

	return Owned;
}

bool UMGCustomizationSubsystem::IsItemOwned(FName ItemID) const
{
	return OwnedItems.Contains(ItemID);
}

void UMGCustomizationSubsystem::UnlockItem(FName ItemID)
{
	if (!OwnedItems.Contains(ItemID))
	{
		OwnedItems.Add(ItemID);

		if (const FMGCustomizationItem* Item = AllItems.Find(ItemID))
		{
			OnItemUnlocked.Broadcast(*Item, Item->Category);
		}

		SaveCustomizationData();
	}
}

TArray<FMGCustomizationItem> UMGCustomizationSubsystem::GetAllItems(EMGCustomizationCategory Category) const
{
	TArray<FMGCustomizationItem> Items;

	for (const auto& Pair : AllItems)
	{
		if (Pair.Value.Category == Category)
		{
			FMGCustomizationItem ItemCopy = Pair.Value;
			ItemCopy.bIsOwned = OwnedItems.Contains(Pair.Key);
			Items.Add(ItemCopy);
		}
	}

	return Items;
}

// ==========================================
// PRESETS
// ==========================================

void UMGCustomizationSubsystem::SavePreset(FName VehicleID, const FString& PresetName)
{
	if (!VehicleCustomizations.Contains(VehicleID))
	{
		return;
	}

	TMap<FString, FMGVehicleCustomization>& VehiclePresets = SavedPresets.FindOrAdd(VehicleID);
	VehiclePresets.Add(PresetName, VehicleCustomizations[VehicleID]);

	SaveCustomizationData();
}

bool UMGCustomizationSubsystem::LoadPreset(FName VehicleID, const FString& PresetName)
{
	const TMap<FString, FMGVehicleCustomization>* VehiclePresets = SavedPresets.Find(VehicleID);
	if (!VehiclePresets)
	{
		return false;
	}

	const FMGVehicleCustomization* Preset = VehiclePresets->Find(PresetName);
	if (!Preset)
	{
		return false;
	}

	VehicleCustomizations.Add(VehicleID, *Preset);
	NotifyCustomizationChanged(VehicleID, EMGCustomizationCategory::Paint);
	return true;
}

TArray<FString> UMGCustomizationSubsystem::GetSavedPresets(FName VehicleID) const
{
	TArray<FString> PresetNames;

	const TMap<FString, FMGVehicleCustomization>* VehiclePresets = SavedPresets.Find(VehicleID);
	if (VehiclePresets)
	{
		VehiclePresets->GetKeys(PresetNames);
	}

	return PresetNames;
}

void UMGCustomizationSubsystem::DeletePreset(FName VehicleID, const FString& PresetName)
{
	TMap<FString, FMGVehicleCustomization>* VehiclePresets = SavedPresets.Find(VehicleID);
	if (VehiclePresets)
	{
		VehiclePresets->Remove(PresetName);
		SaveCustomizationData();
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGCustomizationSubsystem::LoadCustomizationData()
{
	// Would load from save file
}

void UMGCustomizationSubsystem::SaveCustomizationData()
{
	// Would save to persistent storage
}

void UMGCustomizationSubsystem::InitializePresetColors()
{
	PresetColors = {
		// Whites/Grays
		FLinearColor::White,
		FLinearColor(0.9f, 0.9f, 0.9f),
		FLinearColor(0.7f, 0.7f, 0.7f),
		FLinearColor(0.5f, 0.5f, 0.5f),
		FLinearColor(0.3f, 0.3f, 0.3f),
		FLinearColor(0.1f, 0.1f, 0.1f),
		FLinearColor::Black,

		// Reds
		FLinearColor(1.0f, 0.0f, 0.0f),
		FLinearColor(0.8f, 0.0f, 0.0f),
		FLinearColor(0.5f, 0.0f, 0.0f),
		FLinearColor(1.0f, 0.2f, 0.2f),

		// Oranges
		FLinearColor(1.0f, 0.5f, 0.0f),
		FLinearColor(1.0f, 0.3f, 0.0f),

		// Yellows
		FLinearColor(1.0f, 1.0f, 0.0f),
		FLinearColor(1.0f, 0.9f, 0.0f),

		// Greens
		FLinearColor(0.0f, 1.0f, 0.0f),
		FLinearColor(0.0f, 0.8f, 0.0f),
		FLinearColor(0.0f, 0.5f, 0.0f),
		FLinearColor(0.5f, 1.0f, 0.0f),

		// Blues
		FLinearColor(0.0f, 0.0f, 1.0f),
		FLinearColor(0.0f, 0.5f, 1.0f),
		FLinearColor(0.0f, 0.8f, 1.0f),
		FLinearColor(0.2f, 0.2f, 0.8f),

		// Purples
		FLinearColor(0.5f, 0.0f, 1.0f),
		FLinearColor(0.8f, 0.0f, 1.0f),
		FLinearColor(0.5f, 0.0f, 0.5f),

		// Pinks
		FLinearColor(1.0f, 0.0f, 0.5f),
		FLinearColor(1.0f, 0.4f, 0.7f),

		// Special
		FLinearColor(1.0f, 0.84f, 0.0f), // Gold
		FLinearColor(0.75f, 0.75f, 0.75f), // Silver
		FLinearColor(0.72f, 0.45f, 0.2f), // Bronze/Copper
	};
}

FMGVehicleCustomization& UMGCustomizationSubsystem::GetOrCreateCustomization(FName VehicleID)
{
	FMGVehicleCustomization* Existing = VehicleCustomizations.Find(VehicleID);
	if (Existing)
	{
		return *Existing;
	}

	FMGVehicleCustomization NewCustomization;
	NewCustomization.VehicleID = VehicleID;
	VehicleCustomizations.Add(VehicleID, NewCustomization);
	return VehicleCustomizations[VehicleID];
}

void UMGCustomizationSubsystem::NotifyCustomizationChanged(FName VehicleID, EMGCustomizationCategory Category)
{
	OnCustomizationChanged.Broadcast(VehicleID, Category);
}

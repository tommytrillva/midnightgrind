// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCustomizationSubsystem.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTexture2D;
class UStaticMesh;

/**
 * Customization category
 */
UENUM(BlueprintType)
enum class EMGCustomizationCategory : uint8
{
	/** Body paint */
	Paint,
	/** Vinyl wraps */
	Wrap,
	/** Decals/stickers */
	Decal,
	/** Wheels/rims */
	Wheels,
	/** Spoiler */
	Spoiler,
	/** Hood */
	Hood,
	/** Front bumper */
	FrontBumper,
	/** Rear bumper */
	RearBumper,
	/** Side skirts */
	SideSkirts,
	/** Mirrors */
	Mirrors,
	/** Exhaust */
	Exhaust,
	/** Underglow */
	Underglow,
	/** Window tint */
	WindowTint,
	/** Headlights */
	Headlights,
	/** Taillights */
	Taillights,
	/** Interior */
	Interior,
	/** License plate */
	LicensePlate,
	/** Neon */
	Neon
};

/**
 * Paint finish type
 */
UENUM(BlueprintType)
enum class EMGPaintFinish : uint8
{
	/** Matte finish */
	Matte,
	/** Gloss finish */
	Gloss,
	/** Metallic finish */
	Metallic,
	/** Pearlescent finish */
	Pearl,
	/** Chrome finish */
	Chrome,
	/** Brushed metal */
	Brushed,
	/** Satin finish */
	Satin,
	/** Carbon fiber */
	Carbon
};

/**
 * Paint configuration
 */
USTRUCT(BlueprintType)
struct FMGPaintConfig
{
	GENERATED_BODY()

	/** Primary color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor(0.1f, 0.1f, 0.1f);

	/** Secondary color (for two-tone) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor(0.2f, 0.2f, 0.2f);

	/** Paint finish */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPaintFinish Finish = EMGPaintFinish::Gloss;

	/** Metallic intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MetallicIntensity = 0.5f;

	/** Clear coat intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClearCoat = 0.8f;

	/** Flake intensity (for metallic/pearl) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FlakeIntensity = 0.3f;

	/** Pearl shift color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PearlShift = FLinearColor::White;
};

/**
 * Wrap/vinyl configuration
 */
USTRUCT(BlueprintType)
struct FMGWrapConfig
{
	GENERATED_BODY()

	/** Wrap asset ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WrapID;

	/** Wrap texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* WrapTexture;

	/** Primary color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryTint = FLinearColor::White;

	/** Secondary color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryTint = FLinearColor::White;

	/** Wrap scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	/** Wrap offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Offset = FVector2D(0.0f, 0.0f);

	/** Wrap rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rotation = 0.0f;

	/** Is glossy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGlossy = true;
};

/**
 * Decal placement
 */
USTRUCT(BlueprintType)
struct FMGDecalPlacement
{
	GENERATED_BODY()

	/** Decal asset ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DecalID;

	/** Decal texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* DecalTexture;

	/** Placement slot (body panel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	/** Position on surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position = FVector2D(0.5f, 0.5f);

	/** Scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	/** Rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rotation = 0.0f;

	/** Color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Tint = FLinearColor::White;

	/** Flip horizontal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlipH = false;

	/** Flip vertical */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlipV = false;
};

/**
 * Part configuration
 */
USTRUCT(BlueprintType)
struct FMGPartConfig
{
	GENERATED_BODY()

	/** Part asset ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PartID;

	/** Part mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* PartMesh;

	/** Part material override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* MaterialOverride;

	/** Part color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	/** Use body paint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseBodyPaint = false;
};

/**
 * Wheel configuration
 */
USTRUCT(BlueprintType)
struct FMGWheelConfig
{
	GENERATED_BODY()

	/** Wheel/rim asset ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WheelID;

	/** Wheel mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* WheelMesh;

	/** Rim color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RimColor = FLinearColor(0.3f, 0.3f, 0.3f);

	/** Rim finish */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPaintFinish RimFinish = EMGPaintFinish::Chrome;

	/** Tire type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TireType;

	/** Tire color (for colored tires) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TireColor = FLinearColor::Black;

	/** Wheel width multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.5"))
	float WidthMultiplier = 1.0f;

	/** Wheel size multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
	float SizeMultiplier = 1.0f;
};

/**
 * Lighting configuration
 */
USTRUCT(BlueprintType)
struct FMGLightingConfig
{
	GENERATED_BODY()

	/** Underglow enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnderglowEnabled = false;

	/** Underglow color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor UnderglowColor = FLinearColor(0.0f, 0.5f, 1.0f);

	/** Underglow intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float UnderglowIntensity = 2.0f;

	/** Underglow pulse speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float UnderglowPulseSpeed = 0.0f;

	/** Neon enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNeonEnabled = false;

	/** Neon color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor NeonColor = FLinearColor(1.0f, 0.0f, 0.5f);

	/** Headlight tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HeadlightTint = FLinearColor::White;

	/** Taillight tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TaillightTint = FLinearColor(1.0f, 0.0f, 0.0f);

	/** Window tint darkness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindowTint = 0.0f;

	/** Window tint color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor WindowTintColor = FLinearColor::Black;
};

/**
 * Complete vehicle customization
 */
USTRUCT(BlueprintType)
struct FMGVehicleCustomization
{
	GENERATED_BODY()

	/** Vehicle ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/** Paint configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPaintConfig Paint;

	/** Wrap configuration (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWrapConfig Wrap;

	/** Is using wrap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsingWrap = false;

	/** Decal placements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDecalPlacement> Decals;

	/** Wheel configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWheelConfig Wheels;

	/** Body parts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCustomizationCategory, FMGPartConfig> Parts;

	/** Lighting configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLightingConfig Lighting;

	/** License plate text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LicensePlateText;

	/** License plate style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LicensePlateStyle;
};

/**
 * Customization item (for shop/inventory)
 */
USTRUCT(BlueprintType)
struct FMGCustomizationItem
{
	GENERATED_BODY()

	/** Item ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCustomizationCategory Category = EMGCustomizationCategory::Paint;

	/** Preview image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PreviewImage;

	/** Price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Price = 0;

	/** Required level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/** Is owned */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOwned = false;

	/** Is equipped */
	UPROPERTY(BlueprintReadOnly)
	bool bIsEquipped = false;

	/** Is premium/exclusive */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPremium = false;

	/** Compatible vehicles (empty = all) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> CompatibleVehicles;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomizationChanged, FName, VehicleID, EMGCustomizationCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCustomizationSaved, FName, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUnlocked, const FMGCustomizationItem&, Item, EMGCustomizationCategory, Category);

/**
 * Customization Subsystem
 * Manages vehicle visual customization
 *
 * Features:
 * - Paint/wrap application
 * - Decal placement
 * - Part swapping
 * - Material generation
 * - Save/load presets
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCustomizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCustomizationChanged OnCustomizationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCustomizationSaved OnCustomizationSaved;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemUnlocked OnItemUnlocked;

	// ==========================================
	// CUSTOMIZATION MANAGEMENT
	// ==========================================

	/** Get vehicle customization */
	UFUNCTION(BlueprintPure, Category = "Customization")
	FMGVehicleCustomization GetVehicleCustomization(FName VehicleID) const;

	/** Set vehicle customization */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SetVehicleCustomization(FName VehicleID, const FMGVehicleCustomization& Customization);

	/** Reset to default */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void ResetToDefault(FName VehicleID);

	/** Save current customization */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SaveCustomization(FName VehicleID);

	// ==========================================
	// PAINT
	// ==========================================

	/** Set paint config */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetPaintConfig(FName VehicleID, const FMGPaintConfig& Paint);

	/** Set primary color */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetPrimaryColor(FName VehicleID, FLinearColor Color);

	/** Set secondary color */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetSecondaryColor(FName VehicleID, FLinearColor Color);

	/** Set paint finish */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetPaintFinish(FName VehicleID, EMGPaintFinish Finish);

	/** Get preset colors */
	UFUNCTION(BlueprintPure, Category = "Customization|Paint")
	TArray<FLinearColor> GetPresetColors() const;

	// ==========================================
	// WRAP
	// ==========================================

	/** Set wrap */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wrap")
	void SetWrap(FName VehicleID, const FMGWrapConfig& Wrap);

	/** Remove wrap */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wrap")
	void RemoveWrap(FName VehicleID);

	/** Set wrap colors */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wrap")
	void SetWrapColors(FName VehicleID, FLinearColor Primary, FLinearColor Secondary);

	// ==========================================
	// DECALS
	// ==========================================

	/** Add decal */
	UFUNCTION(BlueprintCallable, Category = "Customization|Decal")
	int32 AddDecal(FName VehicleID, const FMGDecalPlacement& Decal);

	/** Remove decal */
	UFUNCTION(BlueprintCallable, Category = "Customization|Decal")
	void RemoveDecal(FName VehicleID, int32 DecalIndex);

	/** Update decal */
	UFUNCTION(BlueprintCallable, Category = "Customization|Decal")
	void UpdateDecal(FName VehicleID, int32 DecalIndex, const FMGDecalPlacement& Decal);

	/** Clear all decals */
	UFUNCTION(BlueprintCallable, Category = "Customization|Decal")
	void ClearAllDecals(FName VehicleID);

	/** Get max decals */
	UFUNCTION(BlueprintPure, Category = "Customization|Decal")
	int32 GetMaxDecals() const { return MaxDecalsPerVehicle; }

	// ==========================================
	// PARTS
	// ==========================================

	/** Set part */
	UFUNCTION(BlueprintCallable, Category = "Customization|Part")
	void SetPart(FName VehicleID, EMGCustomizationCategory Category, const FMGPartConfig& Part);

	/** Remove part (reset to default) */
	UFUNCTION(BlueprintCallable, Category = "Customization|Part")
	void RemovePart(FName VehicleID, EMGCustomizationCategory Category);

	/** Get available parts for category */
	UFUNCTION(BlueprintPure, Category = "Customization|Part")
	TArray<FMGCustomizationItem> GetAvailableParts(FName VehicleID, EMGCustomizationCategory Category) const;

	// ==========================================
	// WHEELS
	// ==========================================

	/** Set wheels */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wheels")
	void SetWheels(FName VehicleID, const FMGWheelConfig& Wheels);

	/** Set wheel color */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wheels")
	void SetWheelColor(FName VehicleID, FLinearColor Color, EMGPaintFinish Finish);

	// ==========================================
	// LIGHTING
	// ==========================================

	/** Set lighting config */
	UFUNCTION(BlueprintCallable, Category = "Customization|Lighting")
	void SetLightingConfig(FName VehicleID, const FMGLightingConfig& Lighting);

	/** Toggle underglow */
	UFUNCTION(BlueprintCallable, Category = "Customization|Lighting")
	void SetUnderglowEnabled(FName VehicleID, bool bEnabled);

	/** Set underglow color */
	UFUNCTION(BlueprintCallable, Category = "Customization|Lighting")
	void SetUnderglowColor(FName VehicleID, FLinearColor Color);

	// ==========================================
	// MATERIAL GENERATION
	// ==========================================

	/** Create paint material */
	UFUNCTION(BlueprintCallable, Category = "Customization|Material")
	UMaterialInstanceDynamic* CreatePaintMaterial(const FMGPaintConfig& Paint);

	/** Create wrap material */
	UFUNCTION(BlueprintCallable, Category = "Customization|Material")
	UMaterialInstanceDynamic* CreateWrapMaterial(const FMGWrapConfig& Wrap, const FMGPaintConfig& BasePaint);

	/** Apply customization to vehicle actor */
	UFUNCTION(BlueprintCallable, Category = "Customization|Material")
	void ApplyCustomizationToVehicle(AActor* VehicleActor, const FMGVehicleCustomization& Customization);

	// ==========================================
	// INVENTORY
	// ==========================================

	/** Get owned items for category */
	UFUNCTION(BlueprintPure, Category = "Customization|Inventory")
	TArray<FMGCustomizationItem> GetOwnedItems(EMGCustomizationCategory Category) const;

	/** Is item owned */
	UFUNCTION(BlueprintPure, Category = "Customization|Inventory")
	bool IsItemOwned(FName ItemID) const;

	/** Unlock item */
	UFUNCTION(BlueprintCallable, Category = "Customization|Inventory")
	void UnlockItem(FName ItemID);

	/** Get all items for category */
	UFUNCTION(BlueprintPure, Category = "Customization|Inventory")
	TArray<FMGCustomizationItem> GetAllItems(EMGCustomizationCategory Category) const;

	// ==========================================
	// PRESETS
	// ==========================================

	/** Save preset */
	UFUNCTION(BlueprintCallable, Category = "Customization|Preset")
	void SavePreset(FName VehicleID, const FString& PresetName);

	/** Load preset */
	UFUNCTION(BlueprintCallable, Category = "Customization|Preset")
	bool LoadPreset(FName VehicleID, const FString& PresetName);

	/** Get saved presets */
	UFUNCTION(BlueprintPure, Category = "Customization|Preset")
	TArray<FString> GetSavedPresets(FName VehicleID) const;

	/** Delete preset */
	UFUNCTION(BlueprintCallable, Category = "Customization|Preset")
	void DeletePreset(FName VehicleID, const FString& PresetName);

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Base paint material */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UMaterialInterface* BasePaintMaterial;

	/** Base wrap material */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UMaterialInterface* BaseWrapMaterial;

	/** Max decals per vehicle */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	int32 MaxDecalsPerVehicle = 20;

	// ==========================================
	// DATA
	// ==========================================

	/** Vehicle customizations */
	UPROPERTY()
	TMap<FName, FMGVehicleCustomization> VehicleCustomizations;

	/** Owned items */
	UPROPERTY()
	TArray<FName> OwnedItems;

	/** All available items */
	UPROPERTY()
	TMap<FName, FMGCustomizationItem> AllItems;

	/** Saved presets */
	TMap<FName, TMap<FString, FMGVehicleCustomization>> SavedPresets;

	/** Preset colors */
	TArray<FLinearColor> PresetColors;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Load customization data */
	void LoadCustomizationData();

	/** Save customization data */
	void SaveCustomizationData();

	/** Initialize preset colors */
	void InitializePresetColors();

	/** Get or create customization */
	FMGVehicleCustomization& GetOrCreateCustomization(FName VehicleID);

	/** Notify customization changed */
	void NotifyCustomizationChanged(FName VehicleID, EMGCustomizationCategory Category);
};

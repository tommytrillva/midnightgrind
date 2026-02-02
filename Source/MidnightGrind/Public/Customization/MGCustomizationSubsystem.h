// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGCustomizationSubsystem.h
 * @brief Vehicle visual customization system for Midnight Grind.
 *
 * This subsystem manages all aspects of vehicle visual customization, allowing
 * players to personalize their cars with paints, wraps, decals, body parts,
 * wheels, and lighting effects.
 *
 * The customization system provides:
 * - **Paint System**: Full color control with multiple finish types (matte, gloss, metallic, pearl, chrome)
 * - **Wrap System**: Vinyl wraps with scaling, rotation, and color tinting
 * - **Decal System**: Individual sticker placement with precise positioning controls
 * - **Body Parts**: Swappable visual components (bumpers, spoilers, hoods, etc.)
 * - **Wheels**: Rim and tire customization with size/width adjustments
 * - **Lighting**: Underglow, neon, headlight/taillight tints, window tint
 * - **Presets**: Save and load customization configurations
 * - **Inventory**: Track owned customization items
 *
 * Key concepts for new developers:
 * - Each vehicle is identified by a unique FName (VehicleID)
 * - Customizations are stored per-vehicle in FMGVehicleCustomization structs
 * - Materials are generated dynamically using UMaterialInstanceDynamic
 * - Changes broadcast events so UI and vehicle actors can update
 *
 * Example usage:
 * @code
 * // Get the subsystem
 * UMGCustomizationSubsystem* CustomSys = GetGameInstance()->GetSubsystem<UMGCustomizationSubsystem>();
 *
 * // Change paint color
 * CustomSys->SetPrimaryColor(VehicleID, FLinearColor::Red);
 *
 * // Apply a wrap
 * FMGWrapConfig Wrap;
 * Wrap.WrapID = "RacingStripes";
 * Wrap.WrapTexture = LoadedTexture;
 * CustomSys->SetWrap(VehicleID, Wrap);
 *
 * // Add a decal
 * FMGDecalPlacement Decal;
 * Decal.DecalID = "SponsorLogo";
 * Decal.Position = FVector2D(0.5f, 0.3f);
 * CustomSys->AddDecal(VehicleID, Decal);
 * @endcode
 *
 * @see FMGVehicleCustomization Complete customization state structure
 * @see FMGPaintConfig Paint configuration options
 * @see EMGCustomizationCategory Categories of customizable parts
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGCustomizationSubsystem.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTexture2D;
class UStaticMesh;

// ============================================================================
// ENUMERATIONS
// ============================================================================

// MOVED TO MGSharedTypes.h
// /**
//  * @brief Categories of visual customization options.
//  *
//  * Each category represents a distinct area of vehicle customization.
//  * Used for organizing UI menus and tracking which parts are equipped.
//  */
// UENUM(BlueprintType)
// enum class EMGCustomizationCategory : uint8
// {
// 	Paint,			///< Body paint color and finish
// 	Wrap,			///< Full vehicle vinyl wraps
// 	Decal,			///< Individual stickers and decals
// 	Wheels,			///< Rims and tires
// 	Spoiler,		///< Rear wing/spoiler
// 	Hood,			///< Hood/bonnet variants (vented, carbon, etc.)
// 	FrontBumper,	///< Front bumper/fascia
// 	RearBumper,		///< Rear bumper/diffuser
// 	SideSkirts,		///< Rocker panel extensions
// 	Mirrors,		///< Side mirror styles
// 	Exhaust,		///< Exhaust tip visuals
// 	Underglow,		///< Underbody lighting
// 	WindowTint,		///< Window tint darkness
// 	Headlights,		///< Headlight style/tint
// 	Taillights,		///< Taillight style/tint
// 	Interior,		///< Interior customization
// 	LicensePlate,	///< License plate text and style
// 	Neon			///< Body neon lighting effects
// };

/**
 * @brief Paint surface finish types.
 *
 * Determines the visual appearance and light reflection behavior
 * of the vehicle's painted surfaces. Each finish creates distinct
 * material properties in the generated UMaterialInstanceDynamic.
 */
UENUM(BlueprintType)
enum class EMGPaintFinish : uint8
{
	Matte,		///< Flat, non-reflective finish with no clear coat
	Gloss,		///< Standard glossy automotive finish
	Metallic,	///< Contains metal flakes that sparkle in light
	Pearl,		///< Pearlescent finish that shifts color at angles
	Chrome,		///< Mirror-like reflective chrome surface
	Brushed,	///< Brushed/satin metal appearance
	Satin,		///< Semi-gloss finish between matte and gloss
	Carbon		///< Carbon fiber weave pattern
};

// ============================================================================
// CONFIGURATION STRUCTURES
// ============================================================================

/**
 * @brief Paint color and finish configuration.
 *
 * Defines all parameters needed to generate a paint material for the vehicle.
 * Supports two-tone paint jobs and various finish types with adjustable
 * intensity values for metallic flakes and clear coat.
 */
USTRUCT(BlueprintType)
struct FMGPaintConfig
{
	GENERATED_BODY()

	/// Main body color (RGB values 0-1 range)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor(0.1f, 0.1f, 0.1f);

	/// Secondary color for two-tone paint jobs (e.g., roof, mirror caps)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor(0.2f, 0.2f, 0.2f);

	/// Surface finish type determining reflection behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPaintFinish Finish = EMGPaintFinish::Gloss;

	/// Metallic flake visibility (0 = none, 1 = maximum sparkle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MetallicIntensity = 0.5f;

	/// Clear coat thickness affecting gloss and depth (0 = no clear, 1 = heavy clear)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClearCoat = 0.8f;

	/// Metal flake density for metallic/pearl finishes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FlakeIntensity = 0.3f;

	/// Color shift hue for pearlescent finishes (the color seen at grazing angles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PearlShift = FLinearColor::White;
};

/**
 * @brief Vinyl wrap/livery configuration.
 *
 * Wraps replace or overlay the base paint with a textured pattern.
 * Supports UV transformations (scale, offset, rotation) and color tinting.
 */
USTRUCT(BlueprintType)
struct FMGWrapConfig
{
	GENERATED_BODY()

	/// Unique identifier for this wrap design
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WrapID;

	/// The texture asset containing the wrap pattern
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* WrapTexture;

	/// Tint applied to the wrap's primary color regions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryTint = FLinearColor::White;

	/// Tint applied to the wrap's secondary color regions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryTint = FLinearColor::White;

	/// UV scale multiplier (larger values = smaller pattern, more repetition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	/// UV offset for positioning the pattern on the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Offset = FVector2D(0.0f, 0.0f);

	/// Pattern rotation in degrees (0-360)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float Rotation = 0.0f;

	/// True for glossy finish, false for matte wrap material
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGlossy = true;
};

/**
 * @brief Individual decal/sticker placement data.
 *
 * Decals are applied on top of paint/wrap as separate texture layers.
 * Each decal can be independently positioned, scaled, rotated, and tinted.
 */
USTRUCT(BlueprintType)
struct FMGDecalPlacement
{
	GENERATED_BODY()

	/// Unique identifier for this decal design
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DecalID;

	/// The texture asset for this decal (should have alpha channel)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* DecalTexture;

	/// Which body panel this decal is attached to (e.g., "Hood", "LeftDoor")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	/// Normalized position on the surface (0,0 = top-left, 1,1 = bottom-right)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position = FVector2D(0.5f, 0.5f);

	/// Size multiplier (1,1 = original size)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	/// Rotation in degrees around the decal center
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float Rotation = 0.0f;

	/// Color multiplier applied to the decal texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Tint = FLinearColor::White;

	/// Mirror the decal horizontally (useful for side-matching logos)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlipH = false;

	/// Mirror the decal vertically
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFlipV = false;
};

/**
 * @brief Visual body part configuration.
 *
 * Represents a swappable body component like bumpers, spoilers, or hoods.
 * Parts can use the vehicle's body paint or have their own color/material.
 */
USTRUCT(BlueprintType)
struct FMGPartConfig
{
	GENERATED_BODY()

	/// Unique identifier for this part variant
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PartID;

	/// 3D mesh asset for this part
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* PartMesh;

	/// Optional custom material (nullptr = use generated material)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* MaterialOverride;

	/// Part-specific color when not using body paint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	/// If true, apply the vehicle's body paint to this part
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseBodyPaint = false;
};

/**
 * @brief Wheel and tire configuration.
 *
 * Controls rim style, size, color, and tire appearance. Width and size
 * multipliers affect both visuals and can influence handling (via other systems).
 */
USTRUCT(BlueprintType)
struct FMGWheelConfig
{
	GENERATED_BODY()

	/// Unique identifier for this wheel/rim design
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WheelID;

	/// 3D mesh asset for the wheel (rim + tire combined or separate)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* WheelMesh;

	/// Rim surface color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RimColor = FLinearColor(0.3f, 0.3f, 0.3f);

	/// Rim surface finish (chrome, matte black, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPaintFinish RimFinish = EMGPaintFinish::Chrome;

	/// Tire compound/style identifier (affects tire texture)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TireType;

	/// Tire sidewall color (for colored tire lettering or custom tires)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TireColor = FLinearColor::Black;

	/// Tire width scale (0.8 = narrow, 1.5 = very wide)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.5"))
	float WidthMultiplier = 1.0f;

	/// Overall wheel diameter scale (0.8 = smaller, 1.2 = larger)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
	float SizeMultiplier = 1.0f;
};

/**
 * @brief Vehicle lighting effects configuration.
 *
 * Controls all dynamic lighting customizations including underglow, neon,
 * headlight/taillight tints, and window tinting.
 */
USTRUCT(BlueprintType)
struct FMGLightingConfig
{
	GENERATED_BODY()

	// --- Underglow Settings ---

	/// Enable/disable underbody lighting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnderglowEnabled = false;

	/// Underglow light color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor UnderglowColor = FLinearColor(0.0f, 0.5f, 1.0f);

	/// Light brightness multiplier (0 = off, 5 = very bright)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float UnderglowIntensity = 2.0f;

	/// Pulsing animation speed (0 = static, higher = faster pulse)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float UnderglowPulseSpeed = 0.0f;

	// --- Neon Settings ---

	/// Enable/disable body neon lighting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNeonEnabled = false;

	/// Neon light color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor NeonColor = FLinearColor(1.0f, 0.0f, 0.5f);

	// --- Lens Tints ---

	/// Headlight lens tint color (white = stock, blue = xenon look, yellow = JDM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HeadlightTint = FLinearColor::White;

	/// Taillight lens tint (red = stock, smoked = darker)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TaillightTint = FLinearColor(1.0f, 0.0f, 0.0f);

	// --- Window Tint ---

	/// Window tint darkness (0 = clear, 1 = limo/blacked out)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindowTint = 0.0f;

	/// Window tint color (black = standard, can be colored for style)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor WindowTintColor = FLinearColor::Black;
};

// ============================================================================
// AGGREGATE STRUCTURES
// ============================================================================

/**
 * @brief Complete vehicle customization state.
 *
 * This structure contains ALL customization data for a single vehicle.
 * It is serialized for save/load and used to fully reconstruct a vehicle's
 * visual appearance.
 */
USTRUCT(BlueprintType)
struct FMGVehicleCustomization
{
	GENERATED_BODY()

	/// Unique identifier linking this customization to a specific vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Paint color and finish settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPaintConfig Paint;

	/// Vinyl wrap configuration (only applied if bUsingWrap is true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWrapConfig Wrap;

	/// Whether a wrap is currently applied over the paint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsingWrap = false;

	/// All decals/stickers placed on the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDecalPlacement> Decals;

	/// Wheel and tire setup
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGWheelConfig Wheels;

	/// Equipped body parts mapped by category
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCustomizationCategory, FMGPartConfig> Parts;

	/// Lighting effects configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLightingConfig Lighting;

	/// Custom license plate text (max 8 characters typically)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LicensePlateText;

	/// License plate visual style (e.g., "California", "JDM", "Euro")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LicensePlateStyle;
};

/**
 * @brief Shop/inventory item representation.
 *
 * Used for displaying customization items in the shop UI, tracking
 * ownership, and managing unlocks. Contains both display info and
 * gameplay requirements.
 */
USTRUCT(BlueprintType)
struct FMGCustomizationItem
{
	GENERATED_BODY()

	/// Unique identifier for this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	/// Localized name shown in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Localized description for tooltips
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Which customization category this item belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCustomizationCategory Category = EMGCustomizationCategory::Paint;

	/// Thumbnail image for shop/inventory display
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PreviewImage;

	/// Purchase price in game currency
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 Price = 0;

	/// Minimum player level required to purchase
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	/// Runtime flag: true if player owns this item (not serialized in asset)
	UPROPERTY(BlueprintReadOnly)
	bool bIsOwned = false;

	/// Runtime flag: true if currently equipped on active vehicle
	UPROPERTY(BlueprintReadOnly)
	bool bIsEquipped = false;

	/// Premium/exclusive item (may require special currency or be limited)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPremium = false;

	/// Vehicle IDs this item can be used on (empty = universal/all vehicles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> CompatibleVehicles;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/// Broadcast when any customization category changes on a vehicle
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomizationChanged, FName, VehicleID, EMGCustomizationCategory, Category);

/// Broadcast when customization is saved to persistent storage
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCustomizationSaved, FName, VehicleID);

/// Broadcast when a new customization item is unlocked/purchased
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUnlocked, const FMGCustomizationItem&, Item, EMGCustomizationCategory, Category);

// ============================================================================
// SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Game instance subsystem managing all vehicle visual customization.
 *
 * UMGCustomizationSubsystem is the central hub for all visual customization
 * operations. It persists across level transitions as a GameInstanceSubsystem
 * and maintains the customization state for all player vehicles.
 *
 * ## Responsibilities
 * - Store and retrieve vehicle customization configurations
 * - Generate dynamic materials for paints and wraps
 * - Apply customizations to vehicle actors at runtime
 * - Manage customization item inventory and unlocks
 * - Save/load customization presets
 *
 * ## Usage Pattern
 * 1. Get subsystem: `GetGameInstance()->GetSubsystem<UMGCustomizationSubsystem>()`
 * 2. Modify customization using category-specific functions (SetPrimaryColor, SetWrap, etc.)
 * 3. Listen to OnCustomizationChanged to update vehicle visuals
 * 4. Call SaveCustomization to persist changes
 *
 * ## Thread Safety
 * All functions should be called from the game thread only.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCustomizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// LIFECYCLE
	// ==========================================

	/** @brief Initialize the subsystem, load saved data, and setup preset colors. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** @brief Cleanup and save any pending customization data. */
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	/** @brief Fired when any customization property changes. Bind to update vehicle visuals. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCustomizationChanged OnCustomizationChanged;

	/** @brief Fired after customization is successfully saved to disk. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCustomizationSaved OnCustomizationSaved;

	/** @brief Fired when a customization item is unlocked/purchased. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnItemUnlocked OnItemUnlocked;

	// ==========================================
	// CUSTOMIZATION MANAGEMENT
	// ==========================================

	/**
	 * @brief Get the complete customization state for a vehicle.
	 * @param VehicleID Unique identifier of the vehicle.
	 * @return Copy of the vehicle's customization data.
	 */
	UFUNCTION(BlueprintPure, Category = "Customization")
	FMGVehicleCustomization GetVehicleCustomization(FName VehicleID) const;

	/**
	 * @brief Set the complete customization state for a vehicle.
	 * @param VehicleID Unique identifier of the vehicle.
	 * @param Customization New customization data to apply.
	 * @note Broadcasts OnCustomizationChanged for all categories.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SetVehicleCustomization(FName VehicleID, const FMGVehicleCustomization& Customization);

	/**
	 * @brief Reset a vehicle to its default/stock appearance.
	 * @param VehicleID Vehicle to reset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void ResetToDefault(FName VehicleID);

	/**
	 * @brief Persist the current customization to save file.
	 * @param VehicleID Vehicle whose customization should be saved.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SaveCustomization(FName VehicleID);

	// ==========================================
	// PAINT
	// ==========================================

	/**
	 * @brief Set the complete paint configuration.
	 * @param VehicleID Target vehicle.
	 * @param Paint New paint configuration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetPaintConfig(FName VehicleID, const FMGPaintConfig& Paint);

	/**
	 * @brief Set only the primary body color.
	 * @param VehicleID Target vehicle.
	 * @param Color New primary color.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetPrimaryColor(FName VehicleID, FLinearColor Color);

	/**
	 * @brief Set only the secondary (accent) color.
	 * @param VehicleID Target vehicle.
	 * @param Color New secondary color.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetSecondaryColor(FName VehicleID, FLinearColor Color);

	/**
	 * @brief Change the paint finish type.
	 * @param VehicleID Target vehicle.
	 * @param Finish New finish type (matte, gloss, metallic, etc.).
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetPaintFinish(FName VehicleID, EMGPaintFinish Finish);

	/**
	 * @brief Get the list of preset colors available in the paint shop.
	 * @return Array of preset color values.
	 */
	UFUNCTION(BlueprintPure, Category = "Customization|Paint")
	TArray<FLinearColor> GetPresetColors() const;

	// ==========================================
	// WRAP
	// ==========================================

	/**
	 * @brief Apply a vinyl wrap to the vehicle.
	 * @param VehicleID Target vehicle.
	 * @param Wrap Wrap configuration to apply.
	 * @note This enables bUsingWrap on the vehicle customization.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wrap")
	void SetWrap(FName VehicleID, const FMGWrapConfig& Wrap);

	/**
	 * @brief Remove any wrap and return to paint-only appearance.
	 * @param VehicleID Target vehicle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wrap")
	void RemoveWrap(FName VehicleID);

	/**
	 * @brief Change wrap tint colors without replacing the wrap.
	 * @param VehicleID Target vehicle.
	 * @param Primary New primary tint color.
	 * @param Secondary New secondary tint color.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wrap")
	void SetWrapColors(FName VehicleID, FLinearColor Primary, FLinearColor Secondary);

	// ==========================================
	// DECALS
	// ==========================================

	/**
	 * @brief Add a new decal to the vehicle.
	 * @param VehicleID Target vehicle.
	 * @param Decal Decal placement data.
	 * @return Index of the newly added decal, or -1 if max decals reached.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Decal")
	int32 AddDecal(FName VehicleID, const FMGDecalPlacement& Decal);

	/**
	 * @brief Remove a specific decal by index.
	 * @param VehicleID Target vehicle.
	 * @param DecalIndex Index of the decal to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Decal")
	void RemoveDecal(FName VehicleID, int32 DecalIndex);

	/**
	 * @brief Update an existing decal's properties.
	 * @param VehicleID Target vehicle.
	 * @param DecalIndex Index of the decal to update.
	 * @param Decal New decal data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Decal")
	void UpdateDecal(FName VehicleID, int32 DecalIndex, const FMGDecalPlacement& Decal);

	/**
	 * @brief Remove all decals from the vehicle.
	 * @param VehicleID Target vehicle.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Decal")
	void ClearAllDecals(FName VehicleID);

	/**
	 * @brief Get the maximum number of decals allowed per vehicle.
	 * @return Maximum decal count.
	 */
	UFUNCTION(BlueprintPure, Category = "Customization|Decal")
	int32 GetMaxDecals() const { return MaxDecalsPerVehicle; }

	// ==========================================
	// PARTS
	// ==========================================

	/**
	 * @brief Equip a body part in a specific category.
	 * @param VehicleID Target vehicle.
	 * @param Category Which part slot to change (Spoiler, Hood, etc.).
	 * @param Part Part configuration to equip.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Part")
	void SetPart(FName VehicleID, EMGCustomizationCategory Category, const FMGPartConfig& Part);

	/**
	 * @brief Remove a part and revert to stock/default.
	 * @param VehicleID Target vehicle.
	 * @param Category Which part slot to reset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Part")
	void RemovePart(FName VehicleID, EMGCustomizationCategory Category);

	/**
	 * @brief Get all parts available for a category on a specific vehicle.
	 * @param VehicleID Vehicle to check compatibility for.
	 * @param Category Part category to query.
	 * @return Array of available parts (includes ownership status).
	 */
	UFUNCTION(BlueprintPure, Category = "Customization|Part")
	TArray<FMGCustomizationItem> GetAvailableParts(FName VehicleID, EMGCustomizationCategory Category) const;

	// ==========================================
	// WHEELS
	// ==========================================

	/**
	 * @brief Set the complete wheel configuration.
	 * @param VehicleID Target vehicle.
	 * @param Wheels Wheel configuration to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wheels")
	void SetWheels(FName VehicleID, const FMGWheelConfig& Wheels);

	/**
	 * @brief Change only the wheel rim color and finish.
	 * @param VehicleID Target vehicle.
	 * @param Color New rim color.
	 * @param Finish New rim finish type.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Wheels")
	void SetWheelColor(FName VehicleID, FLinearColor Color, EMGPaintFinish Finish);

	// ==========================================
	// LIGHTING
	// ==========================================

	/**
	 * @brief Set the complete lighting configuration.
	 * @param VehicleID Target vehicle.
	 * @param Lighting Lighting configuration to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Lighting")
	void SetLightingConfig(FName VehicleID, const FMGLightingConfig& Lighting);

	/**
	 * @brief Toggle underglow on or off.
	 * @param VehicleID Target vehicle.
	 * @param bEnabled True to enable underglow.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Lighting")
	void SetUnderglowEnabled(FName VehicleID, bool bEnabled);

	/**
	 * @brief Change the underglow light color.
	 * @param VehicleID Target vehicle.
	 * @param Color New underglow color.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Lighting")
	void SetUnderglowColor(FName VehicleID, FLinearColor Color);

	// ==========================================
	// MATERIAL GENERATION
	// ==========================================

	/**
	 * @brief Create a dynamic material instance for a paint configuration.
	 * @param Paint Paint settings to bake into the material.
	 * @return New dynamic material instance (caller manages lifetime).
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Material")
	UMaterialInstanceDynamic* CreatePaintMaterial(const FMGPaintConfig& Paint);

	/**
	 * @brief Create a dynamic material instance for a wrap.
	 * @param Wrap Wrap settings to bake into the material.
	 * @param BasePaint Underlying paint (visible where wrap has transparency).
	 * @return New dynamic material instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Material")
	UMaterialInstanceDynamic* CreateWrapMaterial(const FMGWrapConfig& Wrap, const FMGPaintConfig& BasePaint);

	/**
	 * @brief Apply all customization visuals to a vehicle actor.
	 * @param VehicleActor The vehicle actor to update.
	 * @param Customization Customization data to apply.
	 * @note This updates meshes, materials, and lighting components on the actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Material")
	void ApplyCustomizationToVehicle(AActor* VehicleActor, const FMGVehicleCustomization& Customization);

	// ==========================================
	// INVENTORY
	// ==========================================

	/**
	 * @brief Get all owned items in a specific category.
	 * @param Category Category to filter by.
	 * @return Array of owned customization items.
	 */
	UFUNCTION(BlueprintPure, Category = "Customization|Inventory")
	TArray<FMGCustomizationItem> GetOwnedItems(EMGCustomizationCategory Category) const;

	/**
	 * @brief Check if a specific item is owned.
	 * @param ItemID Unique ID of the item to check.
	 * @return True if the item is owned.
	 */
	UFUNCTION(BlueprintPure, Category = "Customization|Inventory")
	bool IsItemOwned(FName ItemID) const;

	/**
	 * @brief Unlock/purchase a customization item.
	 * @param ItemID Item to unlock.
	 * @note Broadcasts OnItemUnlocked on success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Inventory")
	void UnlockItem(FName ItemID);

	/**
	 * @brief Get all items (owned and not owned) in a category.
	 * @param Category Category to filter by.
	 * @return Array of all items with ownership status populated.
	 */
	UFUNCTION(BlueprintPure, Category = "Customization|Inventory")
	TArray<FMGCustomizationItem> GetAllItems(EMGCustomizationCategory Category) const;

	// ==========================================
	// PRESETS
	// ==========================================

	/**
	 * @brief Save the current customization as a named preset.
	 * @param VehicleID Vehicle whose customization to save.
	 * @param PresetName User-defined name for the preset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Preset")
	void SavePreset(FName VehicleID, const FString& PresetName);

	/**
	 * @brief Load and apply a saved preset.
	 * @param VehicleID Vehicle to apply the preset to.
	 * @param PresetName Name of the preset to load.
	 * @return True if preset was found and applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Preset")
	bool LoadPreset(FName VehicleID, const FString& PresetName);

	/**
	 * @brief Get list of saved preset names for a vehicle.
	 * @param VehicleID Vehicle to query presets for.
	 * @return Array of preset names.
	 */
	UFUNCTION(BlueprintPure, Category = "Customization|Preset")
	TArray<FString> GetSavedPresets(FName VehicleID) const;

	/**
	 * @brief Delete a saved preset.
	 * @param VehicleID Vehicle the preset belongs to.
	 * @param PresetName Name of the preset to delete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Customization|Preset")
	void DeletePreset(FName VehicleID, const FString& PresetName);

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** @brief Master paint material used as template for CreatePaintMaterial. */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UMaterialInterface* BasePaintMaterial;

	/** @brief Master wrap material used as template for CreateWrapMaterial. */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UMaterialInterface* BaseWrapMaterial;

	/** @brief Maximum number of decals that can be placed on a single vehicle. */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	int32 MaxDecalsPerVehicle = 20;

	// ==========================================
	// RUNTIME DATA
	// ==========================================

	/** @brief Customization state for all registered vehicles, keyed by VehicleID. */
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	TMap<FName, FMGVehicleCustomization> VehicleCustomizations;

	/** @brief Set of item IDs the player has unlocked/purchased. */
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	TArray<FName> OwnedItems;

	/** @brief Database of all available customization items. */
	UPROPERTY(VisibleAnywhere, Category = "Runtime")
	TMap<FName, FMGCustomizationItem> AllItems;

	/** @brief Saved customization presets, organized by VehicleID then PresetName. */
	TMap<FName, TMap<FString, FMGVehicleCustomization>> SavedPresets;

	/** @brief Quick-access preset colors for the paint shop UI. */
	TArray<FLinearColor> PresetColors;

	// ==========================================
	// INTERNAL HELPERS
	// ==========================================

	/** @brief Load all customization data from save file on startup. */
	void LoadCustomizationData();

	/** @brief Persist all customization data to save file. */
	void SaveCustomizationData();

	/** @brief Populate the PresetColors array with default color palette. */
	void InitializePresetColors();

	/**
	 * @brief Get existing customization or create a default one.
	 * @param VehicleID Vehicle to get/create customization for.
	 * @return Reference to the vehicle's customization (creates if missing).
	 */
	FMGVehicleCustomization& GetOrCreateCustomization(FName VehicleID);

	/**
	 * @brief Broadcast the OnCustomizationChanged event.
	 * @param VehicleID Vehicle that changed.
	 * @param Category Category that was modified.
	 */
	void NotifyCustomizationChanged(FName VehicleID, EMGCustomizationCategory Category);
};

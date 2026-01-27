// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGCustomizationTypes.h
 * =============================================================================
 *
 * PURPOSE:
 * This file defines all the data types (enums and structs) used by the vehicle
 * customization/garage UI system. It contains no logic - only type definitions
 * that describe how customization data is structured and categorized.
 *
 * KEY CONCEPTS:
 *
 * 1. ENUMS (Enumerated Types):
 *    - EMGCustomizationCategory: Categories like Engine, Tires, Paint, etc.
 *    - EMGPartFilter: How to filter parts (All, Owned, Locked, etc.)
 *    - EMGPartSortMode: How to sort part lists (by name, price, tier, etc.)
 *    - EMGUINavigationDirection: Input directions for gamepad/keyboard navigation
 *    - EMGCustomizationMenuState: What screen/state the menu is currently in
 *
 * 2. STRUCTS (Data Containers):
 *    - FMGUIPartData: All display info for a single upgrade part
 *    - FMGStatChange: Before/after stat comparison for a single stat
 *    - FMGPartComparison: Full comparison when previewing a part
 *    - FMGTuningSliderConfig: Settings for a tuning slider (like suspension stiffness)
 *    - FMGPaintColorData: Color and finish settings for paint jobs
 *    - FMGVinylPlacement: Position/scale/rotation of a vinyl decal on the car
 *
 * ARCHITECTURE NOTES:
 * - This is a "types-only" header - it defines data structures but no behavior
 * - Other classes (like UMGCustomizationWidget) use these types
 * - The USTRUCT and UENUM macros make these usable in Blueprints
 * - UPROPERTY macros expose struct members to Unreal's reflection system
 *
 * USAGE EXAMPLE:
 * ```cpp
 * // Creating part data to display in the UI
 * FMGUIPartData PartData;
 * PartData.DisplayName = FText::FromString("Stage 2 Turbo Kit");
 * PartData.Category = EMGCustomizationCategory::ForcedInduction;
 * PartData.Tier = EMGPartTier::Sport;
 * PartData.Price = 15000;
 * PartData.bOwned = false;
 * ```
 *
 * RELATED FILES:
 * - MGCustomizationWidget.h: The main UI widget that uses these types
 * - MGPartListItemWidget.h: Individual part items in the list
 * - MGVehicleData.h: Defines EMGPartTier and EMGPerformanceClass used here
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Vehicle/MGVehicleData.h"
#include "MGCustomizationTypes.generated.h"

/**
 * UI Categories for customization menu
 *
 * These categories organize upgrade parts into logical groups that players
 * can browse. The three main groups are:
 * - Performance: Parts that affect how the car drives (Engine, Tires, etc.)
 * - Visual: Parts that change how the car looks (Paint, Body Kits, etc.)
 * - Tuning: Fine-tuning adjustments to existing parts (gear ratios, alignment, etc.)
 */
UENUM(BlueprintType)
enum class EMGCustomizationCategory : uint8
{
	// Performance Categories
	Engine			UMETA(DisplayName = "Engine"),
	ForcedInduction	UMETA(DisplayName = "Forced Induction"),
	Drivetrain		UMETA(DisplayName = "Drivetrain"),
	Suspension		UMETA(DisplayName = "Suspension"),
	Brakes			UMETA(DisplayName = "Brakes"),
	Wheels			UMETA(DisplayName = "Wheels"),
	Tires			UMETA(DisplayName = "Tires"),
	Aero			UMETA(DisplayName = "Aero"),
	Weight			UMETA(DisplayName = "Weight Reduction"),
	Nitrous			UMETA(DisplayName = "Nitrous"),

	// Visual Categories
	BodyKits		UMETA(DisplayName = "Body Kits"),
	Paint			UMETA(DisplayName = "Paint"),
	Vinyl			UMETA(DisplayName = "Vinyl/Wraps"),
	Interior		UMETA(DisplayName = "Interior"),
	Lights			UMETA(DisplayName = "Lights"),
	Underglow		UMETA(DisplayName = "Underglow"),
	Window			UMETA(DisplayName = "Window Tint"),

	// Tuning Categories
	ECUTuning		UMETA(DisplayName = "ECU Tuning"),
	TransmissionTuning UMETA(DisplayName = "Transmission Tuning"),
	SuspensionTuning UMETA(DisplayName = "Suspension Tuning"),
	AlignmentTuning	UMETA(DisplayName = "Alignment"),
	DifferentialTuning UMETA(DisplayName = "Differential"),

	None			UMETA(Hidden)
};

/**
 * Part filter options for shop/inventory
 */
UENUM(BlueprintType)
enum class EMGPartFilter : uint8
{
	All				UMETA(DisplayName = "All Parts"),
	Owned			UMETA(DisplayName = "Owned"),
	Available		UMETA(DisplayName = "Available to Buy"),
	Locked			UMETA(DisplayName = "Locked"),
	Equipped		UMETA(DisplayName = "Currently Equipped"),

	// By Tier
	TierStock		UMETA(DisplayName = "Stock Parts"),
	TierStreet		UMETA(DisplayName = "Street Parts"),
	TierSport		UMETA(DisplayName = "Sport Parts"),
	TierRace		UMETA(DisplayName = "Race Parts"),
	TierPro			UMETA(DisplayName = "Pro Parts"),
	TierLegendary	UMETA(DisplayName = "Legendary Parts")
};

/**
 * Sort options for part lists
 */
UENUM(BlueprintType)
enum class EMGPartSortMode : uint8
{
	Default			UMETA(DisplayName = "Default"),
	NameAscending	UMETA(DisplayName = "Name (A-Z)"),
	NameDescending	UMETA(DisplayName = "Name (Z-A)"),
	PriceAscending	UMETA(DisplayName = "Price (Low to High)"),
	PriceDescending	UMETA(DisplayName = "Price (High to Low)"),
	TierAscending	UMETA(DisplayName = "Tier (Low to High)"),
	TierDescending	UMETA(DisplayName = "Tier (High to Low)"),
	PowerGain		UMETA(DisplayName = "Power Gain"),
	WeightReduction	UMETA(DisplayName = "Weight Reduction"),
	NewestFirst		UMETA(DisplayName = "Newest First"),
	Rarity			UMETA(DisplayName = "Rarity")
};

/**
 * Navigation direction for UI
 */
UENUM(BlueprintType)
enum class EMGUINavigationDirection : uint8
{
	Up,
	Down,
	Left,
	Right,
	Accept,
	Back,
	LeftShoulder,
	RightShoulder,
	LeftTrigger,
	RightTrigger
};

/**
 * Customization sub-menu state
 */
UENUM(BlueprintType)
enum class EMGCustomizationMenuState : uint8
{
	MainMenu,		// Main garage view
	CategorySelect,	// Selecting a category
	PartSelect,		// Selecting a part within category
	PartDetails,	// Viewing part details/comparison
	TuningAdjust,	// Fine-tuning sliders
	PaintEditor,	// Paint/color customization
	VinylEditor,	// Vinyl/decal placement
	Checkout,		// Purchase confirmation
	InstallProgress	// Install animation/progress
};

/**
 * Represents a single customization part for UI display
 */
USTRUCT(BlueprintType)
struct FMGUIPartData
{
	GENERATED_BODY()

	/** Unique part identifier */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	FGuid PartID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	FText DisplayName;

	/** Manufacturer/brand name */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	FText Manufacturer;

	/** Description */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	FText Description;

	/** Part category */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	EMGCustomizationCategory Category = EMGCustomizationCategory::None;

	/** Part tier */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	EMGPartTier Tier = EMGPartTier::Stock;

	/** Price in credits */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	int64 Price = 0;

	/** Whether player owns this part */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	bool bOwned = false;

	/** Whether part is currently equipped */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	bool bEquipped = false;

	/** Whether part is locked (needs unlock condition) */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	bool bLocked = false;

	/** Unlock requirement description */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	FText UnlockRequirement;

	/** Thumbnail/icon texture */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	TSoftObjectPtr<UTexture2D> ThumbnailTexture;

	/** 3D preview mesh (for parts with visual component) */
	UPROPERTY(BlueprintReadOnly, Category = "Part")
	TSoftObjectPtr<UStaticMesh> PreviewMesh;
};

/**
 * Stat change preview for UI
 */
USTRUCT(BlueprintType)
struct FMGStatChange
{
	GENERATED_BODY()

	/** Stat name for display */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FText StatName;

	/** Current value before change */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentValue = 0.0f;

	/** New value after change */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float NewValue = 0.0f;

	/** Maximum possible value for this stat (for progress bars) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float MaxValue = 100.0f;

	/** Unit suffix (e.g., "HP", "lbs", "mph") */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FText UnitSuffix;

	/** Is higher better? (for color coding) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	bool bHigherIsBetter = true;

	/** Stat display format (0 = integer, 1 = 1 decimal, 2 = 2 decimals) */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 DecimalPlaces = 0;

	/** Get the change amount */
	FORCEINLINE float GetChange() const { return NewValue - CurrentValue; }

	/** Is this a positive change? (accounting for bHigherIsBetter) */
	FORCEINLINE bool IsPositiveChange() const
	{
		return bHigherIsBetter ? (NewValue > CurrentValue) : (NewValue < CurrentValue);
	}

	/** Get change as percentage */
	FORCEINLINE float GetChangePercent() const
	{
		return CurrentValue != 0.0f ? ((NewValue - CurrentValue) / CurrentValue) * 100.0f : 0.0f;
	}
};

/**
 * Full stat comparison for part preview
 */
USTRUCT(BlueprintType)
struct FMGPartComparison
{
	GENERATED_BODY()

	/** The part being compared */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	FMGUIPartData Part;

	/** All stat changes from equipping this part */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	TArray<FMGStatChange> StatChanges;

	/** Current PI before change */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	int32 CurrentPI = 0;

	/** New PI after change */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	int32 NewPI = 0;

	/** Current class before change */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	EMGPerformanceClass CurrentClass = EMGPerformanceClass::D;

	/** New class after change */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	EMGPerformanceClass NewClass = EMGPerformanceClass::D;

	/** Whether installing this part changes performance class */
	FORCEINLINE bool ChangesClass() const { return CurrentClass != NewClass; }

	/** Get PI change */
	FORCEINLINE int32 GetPIChange() const { return NewPI - CurrentPI; }
};

/**
 * Category display info for UI
 */
USTRUCT(BlueprintType)
struct FMGCategoryDisplayInfo
{
	GENERATED_BODY()

	/** Category enum value */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	EMGCustomizationCategory Category = EMGCustomizationCategory::None;

	/** Display name */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	FText DisplayName;

	/** Category icon */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Number of owned parts in this category */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	int32 OwnedPartsCount = 0;

	/** Number of available parts in this category */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	int32 AvailablePartsCount = 0;

	/** Whether this category is locked */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	bool bLocked = false;

	/** Brief description */
	UPROPERTY(BlueprintReadOnly, Category = "Category")
	FText Description;
};

/**
 * Tuning slider configuration
 */
USTRUCT(BlueprintType)
struct FMGTuningSliderConfig
{
	GENERATED_BODY()

	/** Slider identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FName SliderID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FText DisplayName;

	/** Description of what this affects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FText Description;

	/** Left label (e.g., "Soft") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FText LeftLabel;

	/** Right label (e.g., "Stiff") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FText RightLabel;

	/** Minimum value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float MinValue = 0.0f;

	/** Maximum value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float MaxValue = 100.0f;

	/** Current value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float CurrentValue = 50.0f;

	/** Default value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float DefaultValue = 50.0f;

	/** Step size for incremental adjustments */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	float StepSize = 1.0f;

	/** Unit display (e.g., "%", "degrees", "mm") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FText UnitSuffix;

	/** Decimal places for display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	int32 DecimalPlaces = 1;
};

/**
 * Paint color data for UI
 */
USTRUCT(BlueprintType)
struct FMGPaintColorData
{
	GENERATED_BODY()

	/** Color name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FText ColorName;

	/** Primary color value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor Color = FLinearColor::White;

	/** Paint finish type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	EMGPaintFinish Finish = EMGPaintFinish::Gloss;

	/** Price for this color (0 = free/basic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	int32 Price = 0;

	/** Whether this is a premium/special color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	bool bPremium = false;

	/** Whether player owns this color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	bool bOwned = true;

	/** Metallic flake color (for metallic/pearl finishes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor FlakeColor = FLinearColor::White;

	/** Metallic intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	float MetallicIntensity = 0.0f;

	/** Clearcoat intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	float ClearcoatIntensity = 0.5f;
};

/**
 * Vinyl/decal placement data
 */
USTRUCT(BlueprintType)
struct FMGVinylPlacement
{
	GENERATED_BODY()

	/** Vinyl asset ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	FGuid VinylID;

	/** Position on vehicle UV (0-1 range) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	FVector2D Position = FVector2D(0.5f, 0.5f);

	/** Rotation in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	float Rotation = 0.0f;

	/** Scale (1 = 100%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	/** Primary color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	FLinearColor PrimaryColor = FLinearColor::White;

	/** Secondary color (if vinyl has multiple layers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	FLinearColor SecondaryColor = FLinearColor::Black;

	/** Whether to mirror on opposite side */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	bool bMirrored = false;

	/** Layer order (higher = on top) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	int32 LayerOrder = 0;

	/** Opacity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vinyl")
	float Opacity = 1.0f;
};

/**
 * UI Animation state
 */
USTRUCT(BlueprintType)
struct FMGUIAnimationState
{
	GENERATED_BODY()

	/** Is animation currently playing? */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsPlaying = false;

	/** Current animation time (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float CurrentTime = 0.0f;

	/** Animation duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Duration = 0.3f;

	/** Easing function type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TEnumAsByte<EEasingFunc::Type> EasingFunction = EEasingFunc::EaseInOut;
};

/**
 * Garage camera state for UI transitions
 */
USTRUCT(BlueprintType)
struct FMGGarageCameraState
{
	GENERATED_BODY()

	/** Camera location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector Location = FVector::ZeroVector;

	/** Camera rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FRotator Rotation = FRotator::ZeroRotator;

	/** Field of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float FOV = 60.0f;

	/** Focus distance for DOF */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float FocusDistance = 300.0f;

	/** Camera orbit enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bAllowOrbit = true;

	/** Camera zoom enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bAllowZoom = true;
};

/**
 * Input binding for customization UI
 */
USTRUCT(BlueprintType)
struct FMGCustomizationInputBinding
{
	GENERATED_BODY()

	/** Action display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FText ActionName;

	/** Gamepad button/key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FKey GamepadKey;

	/** Keyboard key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FKey KeyboardKey;

	/** Icon texture for gamepad */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TSoftObjectPtr<UTexture2D> GamepadIcon;

	/** Icon texture for keyboard */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TSoftObjectPtr<UTexture2D> KeyboardIcon;
};

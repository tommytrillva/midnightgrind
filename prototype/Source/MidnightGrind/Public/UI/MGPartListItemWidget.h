// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * @file MGPartListItemWidget.h
 * @brief UI widgets for displaying vehicle parts in the customization menu
 *
 * =============================================================================
 * @section Overview
 * This file defines widgets used in the garage/customization UI to display
 * individual upgrade parts, stat comparison bars, and Performance Index (PI)
 * indicators. These are the building blocks of the parts list that players
 * browse when shopping for or selecting upgrades.
 *
 * The visual style follows a PS1/PS2 era aesthetic with bold colors, chunky
 * UI elements, and high readability - inspired by classic racing games like
 * Need for Speed Underground and Midnight Club.
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Part Tiers**: Parts are categorized by quality (Stock, Street, Sport,
 *   Race, Pro, Legendary), each with distinct colors and pricing.
 *
 * - **State Management**: Items track selected, focused (gamepad), and hovered
 *   states to provide appropriate visual feedback.
 *
 * - **Stat Bars**: Visual comparison of how a part changes vehicle stats,
 *   showing current value vs preview value with color-coded indicators.
 *
 * - **Performance Index (PI)**: A single number (100-999) representing overall
 *   vehicle performance, used for class-based matchmaking.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [MGCustomizationWidget]
 *          |
 *          +-- [MGPartListItemWidget] (this file)
 *          |         |
 *          |         +-- Part thumbnail, name, price
 *          |         +-- Tier badge with color coding
 *          |         +-- Owned/locked/equipped status icons
 *          |
 *          +-- [MGStatBarWidget] (this file)
 *          |         |
 *          |         +-- Current value bar
 *          |         +-- Preview value overlay (green/red change)
 *          |
 *          +-- [MGPerformanceIndexWidget] (this file)
 *                    |
 *                    +-- Class letter (D, C, B, A, S, X)
 *                    +-- PI number with change preview
 *
 * =============================================================================
 * @section Usage
 * @code
 * // Creating and configuring a part list item
 * UMGPartListItemWidget* PartItem = CreateWidget<UMGPartListItemWidget>(this, PartItemClass);
 *
 * // Set the part data
 * FMGUIPartData PartData;
 * PartData.DisplayName = FText::FromString("Stage 2 Turbo");
 * PartData.Tier = EMGPartTier::Sport;
 * PartData.Price = 15000;
 * PartItem->SetPartData(PartData);
 *
 * // Handle selection events
 * PartItem->OnPartItemClicked.AddDynamic(this, &UMyWidget::HandlePartClicked);
 *
 * // For stat bars
 * UMGStatBarWidget* StatBar = CreateWidget<UMGStatBarWidget>(this, StatBarClass);
 * FMGStatChange Change;
 * Change.StatName = FText::FromString("Horsepower");
 * Change.CurrentValue = 300.0f;
 * Change.NewValue = 350.0f;
 * StatBar->SetStatData(Change);
 * @endcode
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGCustomizationTypes.h"
#include "MGPartListItemWidget.generated.h"

class UTextBlock;
class UImage;
class UBorder;

// -----------------------------------------------------------------------------
// Delegate Declarations
// -----------------------------------------------------------------------------

/** Broadcast when a part item is clicked/selected */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartItemClicked, const FGuid&, PartID);

/** Broadcast when mouse hovers over a part item (for preview) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartItemHovered, const FGuid&, PartID);

/**
 * Widget representing a single part item in the customization list
 * Designed for PS1/PS2 era aesthetic with chunky elements and clear readability
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPartListItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMGPartListItemWidget(const FObjectInitializer& ObjectInitializer);

	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	//~ End UUserWidget Interface

	/** Set the part data for this item */
	UFUNCTION(BlueprintCallable, Category = "Part Item")
	void SetPartData(const FMGUIPartData& InPartData);

	/** Get the part data */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	const FMGUIPartData& GetPartData() const { return PartData; }

	/** Set whether this item is selected */
	UFUNCTION(BlueprintCallable, Category = "Part Item")
	void SetSelected(bool bInSelected);

	/** Check if selected */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	bool IsSelected() const { return bIsSelected; }

	/** Set focus state (for gamepad navigation) */
	UFUNCTION(BlueprintCallable, Category = "Part Item")
	void SetFocused(bool bInFocused);

	/** Check if focused */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	bool IsFocused() const { return bIsFocused; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when item is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Part Item|Events")
	FOnPartItemClicked OnPartItemClicked;

	/** Called when item is hovered */
	UPROPERTY(BlueprintAssignable, Category = "Part Item|Events")
	FOnPartItemHovered OnPartItemHovered;

protected:
	/** Update visual appearance based on state */
	UFUNCTION(BlueprintImplementableEvent, Category = "Part Item")
	void OnUpdateVisualState();

	/** Called when part data changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Part Item")
	void OnPartDataChanged();

	/** Get the color for the current tier */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	FLinearColor GetTierColor() const;

	/** Get text for tier badge */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	FText GetTierText() const;

	/** Get the background color based on state */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	FLinearColor GetBackgroundColor() const;

	/** Get the border color based on state */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	FLinearColor GetBorderColor() const;

	/** Format price for display */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	FText GetFormattedPrice() const;

	/** Get status icon (owned, locked, new, etc.) */
	UFUNCTION(BlueprintPure, Category = "Part Item")
	TSoftObjectPtr<UTexture2D> GetStatusIcon() const;

	// ==========================================
	// STYLE SETTINGS (PS1/PS2 Aesthetic)
	// ==========================================

	/** Base background color for normal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor NormalBackgroundColor = FLinearColor(0.1f, 0.1f, 0.12f, 0.9f);

	/** Background color when hovered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor HoveredBackgroundColor = FLinearColor(0.15f, 0.15f, 0.18f, 1.0f);

	/** Background color when selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor SelectedBackgroundColor = FLinearColor(0.2f, 0.1f, 0.25f, 1.0f);

	/** Background color when focused (gamepad) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor FocusedBackgroundColor = FLinearColor(0.25f, 0.15f, 0.3f, 1.0f);

	/** Border color for normal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor NormalBorderColor = FLinearColor(0.3f, 0.3f, 0.35f, 1.0f);

	/** Border color when selected/focused */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor SelectedBorderColor = FLinearColor(1.0f, 0.0f, 0.8f, 1.0f); // Hot pink - signature color

	/** Locked item tint color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor LockedTintColor = FLinearColor(0.4f, 0.4f, 0.4f, 0.6f);

	/** Stock tier color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Tiers")
	FLinearColor TierColorStock = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	/** Street tier color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Tiers")
	FLinearColor TierColorStreet = FLinearColor(0.3f, 0.7f, 0.3f, 1.0f);

	/** Sport tier color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Tiers")
	FLinearColor TierColorSport = FLinearColor(0.3f, 0.5f, 1.0f, 1.0f);

	/** Race tier color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Tiers")
	FLinearColor TierColorRace = FLinearColor(0.8f, 0.4f, 0.0f, 1.0f);

	/** Pro tier color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Tiers")
	FLinearColor TierColorPro = FLinearColor(0.7f, 0.2f, 0.9f, 1.0f);

	/** Legendary tier color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style|Tiers")
	FLinearColor TierColorLegendary = FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);

	// ==========================================
	// STATE
	// ==========================================

	UPROPERTY(BlueprintReadOnly, Category = "Part Item|State")
	FMGUIPartData PartData;

	UPROPERTY(BlueprintReadOnly, Category = "Part Item|State")
	bool bIsSelected = false;

	UPROPERTY(BlueprintReadOnly, Category = "Part Item|State")
	bool bIsFocused = false;

	UPROPERTY(BlueprintReadOnly, Category = "Part Item|State")
	bool bIsHovered = false;
};

/**
 * Widget for displaying stat comparison bars
 * Shows current vs preview value with +/- indicator
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGStatBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set the stat data to display */
	UFUNCTION(BlueprintCallable, Category = "Stat Bar")
	void SetStatData(const FMGStatChange& InStatChange);

	/** Get the current stat data */
	UFUNCTION(BlueprintPure, Category = "Stat Bar")
	const FMGStatChange& GetStatData() const { return StatChange; }

	/** Set whether to show the change preview */
	UFUNCTION(BlueprintCallable, Category = "Stat Bar")
	void SetShowPreview(bool bShow);

protected:
	/** Called when stat data changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Stat Bar")
	void OnStatDataChanged();

	/** Get color for the change indicator */
	UFUNCTION(BlueprintPure, Category = "Stat Bar")
	FLinearColor GetChangeColor() const;

	/** Get formatted change text (+15, -5, etc.) */
	UFUNCTION(BlueprintPure, Category = "Stat Bar")
	FText GetChangeText() const;

	/** Get current value as percentage of max (for bar fill) */
	UFUNCTION(BlueprintPure, Category = "Stat Bar")
	float GetCurrentFillPercent() const;

	/** Get preview value as percentage of max */
	UFUNCTION(BlueprintPure, Category = "Stat Bar")
	float GetPreviewFillPercent() const;

	// ==========================================
	// STYLE SETTINGS
	// ==========================================

	/** Bar fill color for current value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor CurrentBarColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);

	/** Bar fill color for positive change preview */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor PositiveChangeColor = FLinearColor(0.2f, 0.9f, 0.3f, 1.0f);

	/** Bar fill color for negative change preview */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor NegativeChangeColor = FLinearColor(0.9f, 0.2f, 0.2f, 1.0f);

	/** Background bar color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor BackgroundBarColor = FLinearColor(0.15f, 0.15f, 0.18f, 1.0f);

	// ==========================================
	// STATE
	// ==========================================

	UPROPERTY(BlueprintReadOnly, Category = "Stat Bar|State")
	FMGStatChange StatChange;

	UPROPERTY(BlueprintReadOnly, Category = "Stat Bar|State")
	bool bShowPreview = true;
};

/**
 * Performance Index display widget
 * Shows class letter with PI number and change preview
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPerformanceIndexWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set current PI value */
	UFUNCTION(BlueprintCallable, Category = "PI Display")
	void SetCurrentPI(int32 InPI);

	/** Set preview PI value */
	UFUNCTION(BlueprintCallable, Category = "PI Display")
	void SetPreviewPI(int32 InPreviewPI);

	/** Clear preview */
	UFUNCTION(BlueprintCallable, Category = "PI Display")
	void ClearPreview();

	/** Get current PI */
	UFUNCTION(BlueprintPure, Category = "PI Display")
	int32 GetCurrentPI() const { return CurrentPI; }

protected:
	/** Called when PI values change */
	UFUNCTION(BlueprintImplementableEvent, Category = "PI Display")
	void OnPIChanged();

	/** Get the class letter for a PI value */
	UFUNCTION(BlueprintPure, Category = "PI Display")
	FText GetClassLetter(int32 PI) const;

	/** Get the class color for a PI value */
	UFUNCTION(BlueprintPure, Category = "PI Display")
	FLinearColor GetClassColor(int32 PI) const;

	/** Get formatted PI text */
	UFUNCTION(BlueprintPure, Category = "PI Display")
	FText GetPIText(int32 PI) const;

	/** Is the preview showing a class change? */
	UFUNCTION(BlueprintPure, Category = "PI Display")
	bool IsClassChanging() const;

	// ==========================================
	// STYLE - Class Colors (PS1/PS2 bold palette)
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor ClassD_Color = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor ClassC_Color = FLinearColor(0.3f, 0.8f, 0.3f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor ClassB_Color = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor ClassA_Color = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor ClassS_Color = FLinearColor(1.0f, 0.0f, 0.6f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor ClassX_Color = FLinearColor(1.0f, 0.9f, 0.0f, 1.0f);

	// ==========================================
	// STATE
	// ==========================================

	UPROPERTY(BlueprintReadOnly, Category = "PI Display|State")
	int32 CurrentPI = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PI Display|State")
	int32 PreviewPI = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PI Display|State")
	bool bHasPreview = false;
};

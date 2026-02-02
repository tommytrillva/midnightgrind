// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * @file MGVehicleSelectWidget.h
 * @brief Carousel-style vehicle selection UI for pre-race setup
 *
 * =============================================================================
 * @section Overview
 * This file defines the vehicle selection widget displayed before races, allowing
 * players to browse their garage and select which vehicle to race with. The widget
 * presents vehicles in a carousel format with 3D previews and detailed stat
 * comparisons.
 *
 * The selection interface supports:
 * - Horizontal carousel navigation with animated transitions
 * - Performance stat bars (speed, acceleration, handling, braking, drift)
 * - Filtering by class tier, ownership status, and vehicle type
 * - Sorting by performance index, name, manufacturer, or price
 * - Class restrictions for ranked/competitive races
 * - Quick access to customization from selection screen
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Performance Index (PI)**: A single number (100-999) representing overall
 *   vehicle capability. Higher PI means better performance. PI determines which
 *   class tier a vehicle belongs to.
 *
 * - **Class Tiers**: Vehicles are grouped into classes (S, A, B, C, D) based on
 *   their PI. Ranked races often restrict which classes can participate to ensure
 *   fair competition.
 *
 * - **Stat Ratings**: Each stat (speed, acceleration, etc.) is normalized to 0-1
 *   for easy comparison. These are derived from the vehicle's actual physics
 *   parameters but simplified for player comprehension.
 *
 * - **Carousel Animation**: The selection uses smooth interpolation when switching
 *   vehicles, creating a polished "spinning through options" feel common in
 *   racing game UIs.
 *
 * - **Ownership States**: Vehicles can be owned (ready to race), available for
 *   purchase, or locked behind progression requirements.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Race Setup Flow]
 *          |
 *          v
 *   [MGVehicleSelectWidget] <-- Vehicle data from garage subsystem
 *          |
 *          +-- FMGVehiclePreviewData (struct per vehicle)
 *          |       |
 *          |       +-- Stats, ownership, unlock requirements
 *          |
 *          +-- Filtering/Sorting Logic
 *          |
 *          +-- Carousel Animation System
 *          |
 *          v
 *   [OnVehicleConfirmed] --> Race starts with selected vehicle
 *
 * =============================================================================
 * @section Usage
 * @code
 * // Create and configure the vehicle select widget
 * UMGVehicleSelectWidget* SelectWidget = CreateWidget<UMGVehicleSelectWidget>(
 *     GetWorld(), VehicleSelectClass);
 *
 * // Get available vehicles from garage system
 * TArray<FMGVehiclePreviewData> Vehicles = GarageSubsystem->GetOwnedVehicles();
 *
 * // Initialize the widget
 * SelectWidget->Initialize(Vehicles);
 *
 * // For ranked races, set class restriction
 * SelectWidget->SetClassRestriction("A");  // Only A-class vehicles
 *
 * // Pre-select player's last used vehicle
 * SelectWidget->SetInitialSelection(LastUsedVehicleID);
 *
 * // Listen for selection events
 * SelectWidget->OnVehicleConfirmed.AddDynamic(this, &AMyController::OnVehicleChosen);
 * SelectWidget->OnCancelled.AddDynamic(this, &AMyController::OnSelectionCancelled);
 *
 * // Display the widget
 * SelectWidget->AddToViewport();
 *
 * // Navigation (typically bound to gamepad/keyboard)
 * SelectWidget->SelectNext();      // Move to next vehicle
 * SelectWidget->SelectPrevious();  // Move to previous vehicle
 * SelectWidget->ConfirmSelection(); // Confirm and start race
 *
 * // Filtering
 * SelectWidget->SetFilter(EMGVehicleFilter::ClassA); // Show only A-class
 * SelectWidget->SetSortMode(EMGVehicleSort::PerformanceIndex); // Sort by PI
 * @endcode
 *
 * =============================================================================
 * @section VisualDesign Visual Design Notes
 *
 * The widget follows the game's Y2K neon aesthetic with:
 * - Cyan/pink neon accent colors for stats and highlights
 * - Class tier colors: S=Hot Pink, A=Orange, B=Yellow, C=Green, D=Light Blue
 * - Bold, chunky stat bars reminiscent of PS1/PS2 racing games
 * - Animated transitions with smooth easing
 *
 * =============================================================================
 * @section UnrealMacros Unreal Engine Macros Explained
 *
 * - USTRUCT(BlueprintType): FMGVehiclePreviewData can be used in Blueprints for
 *   UI binding, making it easy to display vehicle info in UMG widgets.
 *
 * - DECLARE_DYNAMIC_MULTICAST_DELEGATE: Events that can be bound in both C++
 *   and Blueprints. Use AddDynamic() in C++ or "Bind Event" nodes in Blueprint.
 *
 * - UPROPERTY(BlueprintAssignable): These delegates appear in Blueprint event
 *   graphs and can trigger Blueprint logic when the event fires.
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/MG_VHCL_Data.h"
#include "MGVehicleSelectWidget.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
class UCanvasPanel;
class UButton;
class UVerticalBox;
class UHorizontalBox;

/**
 * Vehicle preview data for selection screen
 */
USTRUCT(BlueprintType)
struct FMGVehiclePreviewData
{
	GENERATED_BODY()

	/** Vehicle data asset ID */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	/** Manufacturer */
	UPROPERTY(BlueprintReadOnly)
	FText Manufacturer;

	/** Class tier (S, A, B, C, D) */
	UPROPERTY(BlueprintReadOnly)
	FText ClassTier;

	/** Performance index (100-999) */
	UPROPERTY(BlueprintReadOnly)
	int32 PerformanceIndex = 0;

	/** Is owned by player */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOwned = false;

	/** Is locked (not yet unlocked) */
	UPROPERTY(BlueprintReadOnly)
	bool bIsLocked = false;

	/** Unlock requirement text */
	UPROPERTY(BlueprintReadOnly)
	FText UnlockRequirement;

	/** Purchase price (if not owned) */
	UPROPERTY(BlueprintReadOnly)
	int64 Price = 0;

	/** Base stats normalized 0-1 */
	UPROPERTY(BlueprintReadOnly)
	float SpeedRating = 0.5f;

	UPROPERTY(BlueprintReadOnly)
	float AccelerationRating = 0.5f;

	UPROPERTY(BlueprintReadOnly)
	float HandlingRating = 0.5f;

	UPROPERTY(BlueprintReadOnly)
	float BrakingRating = 0.5f;

	UPROPERTY(BlueprintReadOnly)
	float DriftRating = 0.5f;

	/** Vehicle type tag */
	UPROPERTY(BlueprintReadOnly)
	FText VehicleType;

	/** Drivetrain type */
	UPROPERTY(BlueprintReadOnly)
	FText Drivetrain;

	/** Description/flavor text */
	UPROPERTY(BlueprintReadOnly)
	FText Description;
};

/**
 * Vehicle filter options
 */
UENUM(BlueprintType)
enum class EMGVehicleFilter : uint8
{
	All,
	Owned,
	Available,
	ClassS,
	ClassA,
	ClassB,
	ClassC,
	ClassD,
	JDM,
	European,
	American,
	Tuner,
	Muscle,
	Exotic
};

/**
 * Vehicle sort options
 */
UENUM(BlueprintType)
enum class EMGVehicleSort : uint8
{
	PerformanceIndex,
	Name,
	Manufacturer,
	Price,
	RecentlyUsed
};

/**
 * Delegate for vehicle selection
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleSelected, FName, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleSelectCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleConfirmed, FName, VehicleID);

/**
 * Vehicle Selection Widget
 * Carousel-style vehicle selection for pre-race setup
 *
 * Features:
 * - Horizontal carousel with 3D preview
 * - Stat bars with comparisons
 * - Filtering and sorting
 * - Owned/locked state display
 * - Quick customization access
 * - Class tier filtering for ranked races
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMGVehicleSelectWidget(const FObjectInitializer& ObjectInitializer);

	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	//~ End UUserWidget Interface

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Initialize with available vehicles */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select")
	void Initialize(const TArray<FMGVehiclePreviewData>& AvailableVehicles);

	/** Set class restriction for ranked races */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select")
	void SetClassRestriction(const FString& AllowedClass);

	/** Clear class restriction */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select")
	void ClearClassRestriction();

	/** Set initial selection */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select")
	void SetInitialSelection(FName VehicleID);

	// ==========================================
	// NAVIGATION
	// ==========================================

	/** Select next vehicle */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select|Navigation")
	void SelectNext();

	/** Select previous vehicle */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select|Navigation")
	void SelectPrevious();

	/** Jump to specific index */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select|Navigation")
	void SelectIndex(int32 Index);

	/** Confirm current selection */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select|Navigation")
	void ConfirmSelection();

	/** Cancel and close */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select|Navigation")
	void Cancel();

	// ==========================================
	// FILTERING/SORTING
	// ==========================================

	/** Set filter */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select|Filter")
	void SetFilter(EMGVehicleFilter Filter);

	/** Set sort mode */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select|Filter")
	void SetSortMode(EMGVehicleSort SortMode);

	/** Toggle filter panel visibility */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Select|Filter")
	void ToggleFilterPanel();

	// ==========================================
	// STATE
	// ==========================================

	/** Get currently highlighted vehicle */
	UFUNCTION(BlueprintPure, Category = "Vehicle Select|State")
	FMGVehiclePreviewData GetCurrentVehicle() const;

	/** Get current selection index */
	UFUNCTION(BlueprintPure, Category = "Vehicle Select|State")
	int32 GetCurrentIndex() const { return CurrentIndex; }

	/** Get filtered vehicle count */
	UFUNCTION(BlueprintPure, Category = "Vehicle Select|State")
	int32 GetVehicleCount() const { return FilteredVehicles.Num(); }

	/** Can select current vehicle (owned and meets restrictions) */
	UFUNCTION(BlueprintPure, Category = "Vehicle Select|State")
	bool CanSelectCurrentVehicle() const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when highlighted vehicle changes */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle Select|Events")
	FOnVehicleSelected OnVehicleHighlighted;

	/** Called when selection is confirmed */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle Select|Events")
	FOnVehicleConfirmed OnVehicleConfirmed;

	/** Called when selection is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle Select|Events")
	FOnVehicleSelectCancelled OnCancelled;

protected:
	// ==========================================
	// UI CREATION
	// ==========================================

	/** Create all UI elements */
	virtual void CreateUIElements();

	/** Create vehicle info panel */
	void CreateInfoPanel();

	/** Create stat bars */
	void CreateStatBars();

	/** Create navigation controls */
	void CreateNavigationControls();

	/** Create filter panel */
	void CreateFilterPanel();

	// ==========================================
	// UI UPDATE
	// ==========================================

	/** Update display for current vehicle */
	void UpdateVehicleDisplay();

	/** Update stat bar values */
	void UpdateStatBars(const FMGVehiclePreviewData& Vehicle);

	/** Update navigation indicators */
	void UpdateNavigationIndicators();

	/** Update carousel animation */
	void UpdateCarouselAnimation(float MGDeltaTime);

	// ==========================================
	// FILTERING
	// ==========================================

	/** Apply current filter and sort */
	void ApplyFilterAndSort();

	/** Check if vehicle passes filter */
	bool PassesFilter(const FMGVehiclePreviewData& Vehicle) const;

	/** Sort vehicles */
	void SortVehicles();

	// ==========================================
	// HELPERS
	// ==========================================

	/** Get class tier color */
	FSlateColor GetClassColor(const FText& ClassTier) const;

	/** Format price text */
	FText FormatPrice(int64 Price) const;

	/** Get stat bar color based on value */
	FLinearColor GetStatBarColor(float Value) const;

private:
	// ==========================================
	// DATA
	// ==========================================

	/** All available vehicles */
	TArray<FMGVehiclePreviewData> AllVehicles;

	/** Filtered/sorted vehicles */
	TArray<FMGVehiclePreviewData> FilteredVehicles;

	/** Current selection index */
	int32 CurrentIndex = 0;

	/** Current filter */
	EMGVehicleFilter CurrentFilter = EMGVehicleFilter::All;

	/** Current sort */
	EMGVehicleSort CurrentSort = EMGVehicleSort::PerformanceIndex;

	/** Class restriction (empty = no restriction) */
	FString ClassRestriction;

	/** Carousel animation progress */
	float CarouselAnimProgress = 0.0f;

	/** Target index for animation */
	int32 TargetIndex = 0;

	/** Is animating */
	bool bIsAnimating = false;

	/** Filter panel visible */
	bool bFilterPanelVisible = false;

	// ==========================================
	// UI ELEMENTS
	// ==========================================

	UPROPERTY()
	UCanvasPanel* RootCanvas;

	// Vehicle info
	UPROPERTY()
	UTextBlock* VehicleNameText;

	UPROPERTY()
	UTextBlock* ManufacturerText;

	UPROPERTY()
	UTextBlock* ClassText;

	UPROPERTY()
	UTextBlock* PIText;

	UPROPERTY()
	UTextBlock* DescriptionText;

	UPROPERTY()
	UTextBlock* DrivetrainText;

	UPROPERTY()
	UTextBlock* TypeText;

	UPROPERTY()
	UTextBlock* PriceText;

	UPROPERTY()
	UTextBlock* LockReasonText;

	// Stat bars
	UPROPERTY()
	UProgressBar* SpeedBar;

	UPROPERTY()
	UProgressBar* AccelBar;

	UPROPERTY()
	UProgressBar* HandlingBar;

	UPROPERTY()
	UProgressBar* BrakingBar;

	UPROPERTY()
	UProgressBar* DriftBar;

	UPROPERTY()
	TArray<UTextBlock*> StatLabels;

	UPROPERTY()
	TArray<UTextBlock*> StatValues;

	// Navigation
	UPROPERTY()
	UTextBlock* IndexText;

	UPROPERTY()
	UButton* PrevButton;

	UPROPERTY()
	UButton* NextButton;

	UPROPERTY()
	UButton* ConfirmButton;

	UPROPERTY()
	UButton* CancelButton;

	UPROPERTY()
	UButton* CustomizeButton;

	// Filter panel
	UPROPERTY()
	UCanvasPanel* FilterPanel;

	UPROPERTY()
	UVerticalBox* FilterOptions;

	// ==========================================
	// STYLE
	// ==========================================

	FLinearColor CyanNeon = FLinearColor(0.0f, 1.0f, 0.9f, 1.0f);
	FLinearColor PinkNeon = FLinearColor(1.0f, 0.0f, 0.6f, 1.0f);
	FLinearColor YellowNeon = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	FLinearColor GreenNeon = FLinearColor(0.0f, 1.0f, 0.4f, 1.0f);
	FLinearColor RedNeon = FLinearColor(1.0f, 0.0f, 0.2f, 1.0f);
	FLinearColor GoldColor = FLinearColor(1.0f, 0.843f, 0.0f, 1.0f);

	// Class colors
	FLinearColor ClassSColor = FLinearColor(1.0f, 0.0f, 0.5f, 1.0f);   // Hot pink
	FLinearColor ClassAColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);   // Orange
	FLinearColor ClassBColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);   // Yellow
	FLinearColor ClassCColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);   // Green
	FLinearColor ClassDColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f);   // Light blue
};

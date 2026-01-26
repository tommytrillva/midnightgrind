// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/MGVehicleData.h"
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
	void UpdateCarouselAnimation(float DeltaTime);

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

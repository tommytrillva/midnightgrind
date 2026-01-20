// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGCustomizationTypes.h"
#include "Vehicle/MGVehicleData.h"
#include "MGCustomizationWidget.generated.h"

class UMGGarageSubsystem;
class UMGPartDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategorySelected, EMGCustomizationCategory, Category);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartSelected, const FMGUIPartData&, PartData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartPurchased, const FMGUIPartData&, PartData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartInstalled, const FMGUIPartData&, PartData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuStateChanged, EMGCustomizationMenuState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCustomizationCanceled);

/**
 * Base widget class for the vehicle customization interface
 * Provides core functionality for the garage/customization UI
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGCustomizationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMGCustomizationWidget(const FObjectInitializer& ObjectInitializer);

	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	//~ End UUserWidget Interface

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Initialize the customization UI with a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void InitializeWithVehicle(const FGuid& VehicleID);

	/** Get the currently selected vehicle ID */
	UFUNCTION(BlueprintPure, Category = "Customization")
	FGuid GetSelectedVehicleID() const { return SelectedVehicleID; }

	// ==========================================
	// NAVIGATION
	// ==========================================

	/** Navigate in a direction (gamepad/keyboard navigation) */
	UFUNCTION(BlueprintCallable, Category = "Customization|Navigation")
	virtual void Navigate(EMGUINavigationDirection Direction);

	/** Go back to previous menu/state */
	UFUNCTION(BlueprintCallable, Category = "Customization|Navigation")
	virtual void NavigateBack();

	/** Confirm current selection */
	UFUNCTION(BlueprintCallable, Category = "Customization|Navigation")
	virtual void ConfirmSelection();

	/** Get current menu state */
	UFUNCTION(BlueprintPure, Category = "Customization|Navigation")
	EMGCustomizationMenuState GetCurrentMenuState() const { return CurrentMenuState; }

	/** Set menu state directly */
	UFUNCTION(BlueprintCallable, Category = "Customization|Navigation")
	void SetMenuState(EMGCustomizationMenuState NewState);

	// ==========================================
	// CATEGORY MANAGEMENT
	// ==========================================

	/** Get all available categories */
	UFUNCTION(BlueprintPure, Category = "Customization|Categories")
	TArray<FMGCategoryDisplayInfo> GetAllCategories() const;

	/** Get categories for a specific tab (Performance, Visual, Tuning) */
	UFUNCTION(BlueprintPure, Category = "Customization|Categories")
	TArray<FMGCategoryDisplayInfo> GetCategoriesForTab(int32 TabIndex) const;

	/** Select a category */
	UFUNCTION(BlueprintCallable, Category = "Customization|Categories")
	void SelectCategory(EMGCustomizationCategory Category);

	/** Get currently selected category */
	UFUNCTION(BlueprintPure, Category = "Customization|Categories")
	EMGCustomizationCategory GetSelectedCategory() const { return SelectedCategory; }

	// ==========================================
	// PART MANAGEMENT
	// ==========================================

	/** Get parts for the currently selected category */
	UFUNCTION(BlueprintPure, Category = "Customization|Parts")
	TArray<FMGUIPartData> GetPartsForSelectedCategory() const;

	/** Get parts with filters applied */
	UFUNCTION(BlueprintPure, Category = "Customization|Parts")
	TArray<FMGUIPartData> GetFilteredParts(EMGPartFilter Filter, EMGPartSortMode SortMode) const;

	/** Select a part (shows preview) */
	UFUNCTION(BlueprintCallable, Category = "Customization|Parts")
	void SelectPart(const FGuid& PartID);

	/** Get currently selected part data */
	UFUNCTION(BlueprintPure, Category = "Customization|Parts")
	FMGUIPartData GetSelectedPartData() const { return SelectedPartData; }

	/** Get comparison data for selected part vs currently equipped */
	UFUNCTION(BlueprintPure, Category = "Customization|Parts")
	FMGPartComparison GetPartComparison() const { return CurrentComparison; }

	/** Purchase the selected part */
	UFUNCTION(BlueprintCallable, Category = "Customization|Parts")
	bool PurchasePart();

	/** Install/equip the selected part */
	UFUNCTION(BlueprintCallable, Category = "Customization|Parts")
	bool InstallPart();

	/** Uninstall currently equipped part (revert to stock) */
	UFUNCTION(BlueprintCallable, Category = "Customization|Parts")
	bool UninstallPart(EMGCustomizationCategory Category);

	// ==========================================
	// FILTERING & SORTING
	// ==========================================

	/** Set current part filter */
	UFUNCTION(BlueprintCallable, Category = "Customization|Filter")
	void SetPartFilter(EMGPartFilter NewFilter);

	/** Get current part filter */
	UFUNCTION(BlueprintPure, Category = "Customization|Filter")
	EMGPartFilter GetCurrentFilter() const { return CurrentFilter; }

	/** Set current sort mode */
	UFUNCTION(BlueprintCallable, Category = "Customization|Filter")
	void SetSortMode(EMGPartSortMode NewSortMode);

	/** Get current sort mode */
	UFUNCTION(BlueprintPure, Category = "Customization|Filter")
	EMGPartSortMode GetCurrentSortMode() const { return CurrentSortMode; }

	// ==========================================
	// VEHICLE STATS
	// ==========================================

	/** Get current vehicle stats */
	UFUNCTION(BlueprintPure, Category = "Customization|Stats")
	FMGVehicleStats GetCurrentVehicleStats() const;

	/** Get preview stats (with selected part applied) */
	UFUNCTION(BlueprintPure, Category = "Customization|Stats")
	FMGVehicleStats GetPreviewVehicleStats() const;

	/** Get current PI (Performance Index) */
	UFUNCTION(BlueprintPure, Category = "Customization|Stats")
	int32 GetCurrentPI() const;

	/** Get current performance class */
	UFUNCTION(BlueprintPure, Category = "Customization|Stats")
	EMGPerformanceClass GetCurrentPerformanceClass() const;

	/** Get preview PI (with selected part) */
	UFUNCTION(BlueprintPure, Category = "Customization|Stats")
	int32 GetPreviewPI() const;

	// ==========================================
	// TUNING
	// ==========================================

	/** Get tuning sliders for current category */
	UFUNCTION(BlueprintPure, Category = "Customization|Tuning")
	TArray<FMGTuningSliderConfig> GetTuningSlidersForCategory() const;

	/** Set a tuning slider value */
	UFUNCTION(BlueprintCallable, Category = "Customization|Tuning")
	void SetTuningValue(FName SliderID, float Value);

	/** Reset a tuning slider to default */
	UFUNCTION(BlueprintCallable, Category = "Customization|Tuning")
	void ResetTuningValue(FName SliderID);

	/** Reset all tuning in current category to defaults */
	UFUNCTION(BlueprintCallable, Category = "Customization|Tuning")
	void ResetCategoryTuning();

	// ==========================================
	// PAINT & VISUALS
	// ==========================================

	/** Get available paint colors */
	UFUNCTION(BlueprintPure, Category = "Customization|Paint")
	TArray<FMGPaintColorData> GetAvailablePaintColors() const;

	/** Set paint color for a zone */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void SetPaintColor(int32 ZoneIndex, const FMGPaintColorData& ColorData);

	/** Get current paint for a zone */
	UFUNCTION(BlueprintPure, Category = "Customization|Paint")
	FMGPaintColorData GetPaintForZone(int32 ZoneIndex) const;

	/** Open custom color picker */
	UFUNCTION(BlueprintCallable, Category = "Customization|Paint")
	void OpenColorPicker(int32 ZoneIndex);

	// ==========================================
	// VINYL/DECALS
	// ==========================================

	/** Get all vinyl placements on vehicle */
	UFUNCTION(BlueprintPure, Category = "Customization|Vinyl")
	TArray<FMGVinylPlacement> GetVinylPlacements() const;

	/** Add a vinyl to the vehicle */
	UFUNCTION(BlueprintCallable, Category = "Customization|Vinyl")
	void AddVinyl(const FGuid& VinylAssetID);

	/** Update vinyl placement */
	UFUNCTION(BlueprintCallable, Category = "Customization|Vinyl")
	void UpdateVinylPlacement(int32 VinylIndex, const FMGVinylPlacement& Placement);

	/** Remove a vinyl */
	UFUNCTION(BlueprintCallable, Category = "Customization|Vinyl")
	void RemoveVinyl(int32 VinylIndex);

	/** Enter vinyl edit mode for a specific vinyl */
	UFUNCTION(BlueprintCallable, Category = "Customization|Vinyl")
	void EnterVinylEditMode(int32 VinylIndex);

	/** Exit vinyl edit mode */
	UFUNCTION(BlueprintCallable, Category = "Customization|Vinyl")
	void ExitVinylEditMode(bool bSaveChanges);

	// ==========================================
	// CAMERA CONTROL
	// ==========================================

	/** Set camera to a preset position */
	UFUNCTION(BlueprintCallable, Category = "Customization|Camera")
	void SetCameraPreset(FName PresetName);

	/** Get current camera state */
	UFUNCTION(BlueprintPure, Category = "Customization|Camera")
	FMGGarageCameraState GetCurrentCameraState() const { return CurrentCameraState; }

	/** Rotate camera orbit */
	UFUNCTION(BlueprintCallable, Category = "Customization|Camera")
	void RotateCameraOrbit(float YawDelta, float PitchDelta);

	/** Zoom camera */
	UFUNCTION(BlueprintCallable, Category = "Customization|Camera")
	void ZoomCamera(float ZoomDelta);

	/** Reset camera to default */
	UFUNCTION(BlueprintCallable, Category = "Customization|Camera")
	void ResetCamera();

	// ==========================================
	// PLAYER INFO
	// ==========================================

	/** Get player's current credits */
	UFUNCTION(BlueprintPure, Category = "Customization|Player")
	int64 GetPlayerCredits() const;

	/** Check if player can afford a part */
	UFUNCTION(BlueprintPure, Category = "Customization|Player")
	bool CanAffordPart(const FGuid& PartID) const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when a category is selected */
	UPROPERTY(BlueprintAssignable, Category = "Customization|Events")
	FOnCategorySelected OnCategorySelected;

	/** Called when a part is selected (for preview) */
	UPROPERTY(BlueprintAssignable, Category = "Customization|Events")
	FOnPartSelected OnPartSelected;

	/** Called when a part is purchased */
	UPROPERTY(BlueprintAssignable, Category = "Customization|Events")
	FOnPartPurchased OnPartPurchased;

	/** Called when a part is installed */
	UPROPERTY(BlueprintAssignable, Category = "Customization|Events")
	FOnPartInstalled OnPartInstalled;

	/** Called when menu state changes */
	UPROPERTY(BlueprintAssignable, Category = "Customization|Events")
	FOnMenuStateChanged OnMenuStateChanged;

	/** Called when customization is canceled */
	UPROPERTY(BlueprintAssignable, Category = "Customization|Events")
	FOnCustomizationCanceled OnCustomizationCanceled;

protected:
	// ==========================================
	// BLUEPRINT IMPLEMENTABLE EVENTS
	// ==========================================

	/** Called when vehicle data is loaded and ready */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Events")
	void OnVehicleDataReady();

	/** Called when part list needs refresh */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Events")
	void OnPartListUpdated();

	/** Called when stats preview changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Events")
	void OnStatsPreviewUpdated();

	/** Called when entering a new menu state */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Events")
	void OnEnterMenuState(EMGCustomizationMenuState NewState);

	/** Called when exiting a menu state */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Events")
	void OnExitMenuState(EMGCustomizationMenuState OldState);

	/** Play menu transition animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Animation")
	void PlayMenuTransition(EMGCustomizationMenuState FromState, EMGCustomizationMenuState ToState);

	/** Play part selection animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Animation")
	void PlayPartSelectionAnimation(const FMGUIPartData& PartData);

	/** Play purchase confirmation animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Animation")
	void PlayPurchaseAnimation(const FMGUIPartData& PartData);

	/** Play part install animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Customization|Animation")
	void PlayInstallAnimation(const FMGUIPartData& PartData);

	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/** Currently selected vehicle ID */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	FGuid SelectedVehicleID;

	/** Current menu state */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	EMGCustomizationMenuState CurrentMenuState = EMGCustomizationMenuState::MainMenu;

	/** Previous menu state (for back navigation) */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	EMGCustomizationMenuState PreviousMenuState = EMGCustomizationMenuState::MainMenu;

	/** Currently selected category */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	EMGCustomizationCategory SelectedCategory = EMGCustomizationCategory::Engine;

	/** Currently selected part data */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	FMGUIPartData SelectedPartData;

	/** Current part comparison data */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	FMGPartComparison CurrentComparison;

	/** Current filter mode */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	EMGPartFilter CurrentFilter = EMGPartFilter::All;

	/** Current sort mode */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	EMGPartSortMode CurrentSortMode = EMGPartSortMode::Default;

	/** Current camera state */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	FMGGarageCameraState CurrentCameraState;

	/** Menu state history for navigation */
	UPROPERTY()
	TArray<EMGCustomizationMenuState> MenuStateHistory;

	/** Cached parts list for current category */
	UPROPERTY()
	TArray<FMGUIPartData> CachedPartsList;

	/** Is currently in a transition animation */
	UPROPERTY(BlueprintReadOnly, Category = "Customization|State")
	bool bIsInTransition = false;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Refresh the parts list cache */
	virtual void RefreshPartsList();

	/** Calculate stat comparison for a part */
	virtual FMGPartComparison CalculatePartComparison(const FGuid& PartID) const;

	/** Update the 3D preview in the garage */
	virtual void UpdateVehiclePreview();

	/** Get input bindings for current state */
	virtual TArray<FMGCustomizationInputBinding> GetInputBindingsForState() const;

	/** Handle input based on current menu state */
	virtual void HandleInputForState(EMGUINavigationDirection Direction);

private:
	/** Reference to garage subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMGGarageSubsystem> GarageSubsystem;

	/** Update camera interpolation */
	void UpdateCameraInterpolation(float DeltaTime);

	/** Camera interpolation data */
	FMGGarageCameraState TargetCameraState;
	float CameraInterpAlpha = 0.0f;
	bool bIsCameraInterpolating = false;

	// ==========================================
	// LOCAL STATE FOR TESTING
	// (Will integrate with save system later)
	// ==========================================

	/** Cached player credits for local testing */
	int64 PlayerCreditsCache = 50000;

	/** Track purchased parts locally */
	TSet<FGuid> PurchasedPartIDs;

	/** Track installed parts by category */
	TMap<EMGCustomizationCategory, FGuid> InstalledPartsByCategory;

	/** Cached current vehicle stats */
	mutable FMGVehicleStats CachedCurrentStats;
};

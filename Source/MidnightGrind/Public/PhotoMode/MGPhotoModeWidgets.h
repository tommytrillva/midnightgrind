// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGPhotoModeWidgets.h
 * =============================================================================
 *
 * OVERVIEW:
 * This file defines all the UI widgets for the photo mode feature. Photo mode
 * lets players pause the game, position a camera, apply filters, and capture
 * screenshots - similar to photo modes in games like Forza, Gran Turismo, or
 * Ghost of Tsushima.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. PHOTO MODE UI STRUCTURE:
 *    The UI is organized into "tabs" for different settings:
 *    - Camera Tab: FOV, depth of field, focus distance, roll
 *    - Visual Tab: Filters, exposure, contrast, saturation, effects
 *    - Overlay Tab: Watermarks, frames, date stamps
 *
 * 2. SLIDERS AND VALUES:
 *    Most settings use sliders (USlider) that map to a 0-1 range.
 *    The widget code converts this to the actual setting range.
 *    Example: A 0-1 slider for FOV might map to 30-120 degrees.
 *
 * 3. BLUEPRINT NATIVE EVENTS:
 *    Functions marked BlueprintNativeEvent have C++ implementations that
 *    Blueprints can override. The C++ provides default behavior, but
 *    designers can customize it in the Widget Blueprint.
 *
 * 4. SUBSYSTEM COMMUNICATION:
 *    Widgets communicate with MGPhotoModeSubsystem to:
 *    - Read current settings (to show current values)
 *    - Write new settings (when user adjusts sliders)
 *    - Trigger actions (take photo, exit photo mode)
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Photo Mode Subsystem]
 *           ^
 *           | (reads/writes settings)
 *           |
 *    [Main HUD Widget] (UMGPhotoModeHUDWidget)
 *           |
 *           | contains
 *           v
 *    [Camera Widget] [Visual Widget] [Overlay Widget]
 *           |
 *           | user interaction
 *           v
 *    [Sliders, Buttons, etc.]
 *
 * WORKFLOW FOR DESIGNERS:
 * 1. Create Widget Blueprints inheriting from these classes
 * 2. Design the visual layout with sliders, buttons, and labels
 * 3. Name widgets to match BindWidget variables
 * 4. The C++ handles all the logic - you just design the look
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGPhotoModeSubsystem.h"
#include "MGPhotoModeWidgets.generated.h"

// Forward declarations for common UMG widgets
class USlider;      // A draggable slider for numeric input (0 to 1 range)
class UTextBlock;   // Displays text labels
class UImage;       // Displays images/thumbnails
class UButton;      // Clickable button for actions

/**
 * UMGPhotoCameraWidget - Controls for camera settings in photo mode.
 *
 * Allows adjustment of:
 * - Field of View (FOV) - Zoom level, wide vs telephoto
 * - Focal Distance - Where the camera focuses for depth of field
 * - Aperture - How blurry the background/foreground is (f-stop)
 * - Depth of Field toggle - Enable/disable blur effect
 * - Roll - Tilt the camera for Dutch angles
 * - Camera Mode - Free, Orbit, Track, or Locked
 *
 * PHOTOGRAPHY TERMS FOR NON-PHOTOGRAPHERS:
 * - FOV (Field of View): Lower = zoomed in, Higher = wide angle (fisheye-ish)
 * - Focal Distance: The distance at which objects are perfectly sharp
 * - Aperture (f-stop): Lower number = more blur, Higher = everything sharp
 * - Depth of Field: The "blur the background" effect (called bokeh)
 * - Roll: Tilting the camera sideways (Dutch angle = dramatic tilted shot)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPhotoCameraWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Set camera settings */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraSettings(const FMGPhotoCameraSettings& Settings);

	/** Get current settings */
	UFUNCTION(BlueprintPure, Category = "Camera")
	FMGPhotoCameraSettings GetCameraSettings() const { return CurrentSettings; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGPhotoCameraSettings CurrentSettings;

	UPROPERTY()
	UMGPhotoModeSubsystem* PhotoModeSubsystem;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** FOV changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnFOVChanged(float Value);

	/** Focal distance changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnFocalDistanceChanged(float Value);

	/** Aperture changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnApertureChanged(float Value);

	/** DOF toggled */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnDOFToggled(bool bEnabled);

	/** Roll changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnRollChanged(float Value);

	/** Camera mode changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnCameraModeChanged(int32 ModeIndex);

	/** Reset camera */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnResetCamera();
};

/**
 * UMGPhotoVisualWidget - Controls for visual/post-processing settings.
 *
 * This is where the "Instagram filter" style adjustments happen:
 * - Preset Filters - Quick presets like Vintage, Noir, Neon, VHS
 * - Exposure - Overall brightness
 * - Contrast - Difference between lights and darks
 * - Saturation - Color intensity (0 = black & white)
 * - Temperature - Color warmth (negative = blue/cold, positive = orange/warm)
 * - Vignette - Darkening at the edges of the image
 * - Film Grain - Adds noise for a vintage film look
 * - Bloom - Glowing effect on bright areas
 *
 * FILTER PRESETS:
 * When a preset filter is selected, it sets all the individual values.
 * Selecting "Custom" allows manual adjustment of all parameters.
 * The presets are great for quick looks; custom is for fine-tuning.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPhotoVisualWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Set visual settings */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void SetVisualSettings(const FMGPhotoVisualSettings& Settings);

	/** Get current settings */
	UFUNCTION(BlueprintPure, Category = "Visual")
	FMGPhotoVisualSettings GetVisualSettings() const { return CurrentSettings; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGPhotoVisualSettings CurrentSettings;

	UPROPERTY()
	UMGPhotoModeSubsystem* PhotoModeSubsystem;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Filter changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnFilterChanged(int32 FilterIndex);

	/** Exposure changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnExposureChanged(float Value);

	/** Contrast changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnContrastChanged(float Value);

	/** Saturation changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnSaturationChanged(float Value);

	/** Temperature changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnTemperatureChanged(float Value);

	/** Vignette changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnVignetteChanged(float Value);

	/** Film grain changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnFilmGrainChanged(float Value);

	/** Bloom changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnBloomChanged(float Value);

	/** Reset visual settings */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnResetVisual();
};

/**
 * UMGPhotoOverlayWidget - Controls for image overlays and watermarks.
 *
 * Manages elements that appear ON TOP of the captured image:
 * - Game Logo - The Midnight Grind watermark (can be toggled off)
 * - Frame/Border - Decorative frames around the image
 * - Date Stamp - Shows capture date (like old film cameras)
 * - Vehicle Info - Shows the car name/specs
 *
 * DESIGN NOTE:
 * These overlays are rendered onto the final image, so they become part
 * of the saved file. Users who want "clean" photos can turn everything off.
 * Consider making the logo subtle (corner, semi-transparent) so it doesn't
 * ruin otherwise great shots.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPhotoOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set overlay settings */
	UFUNCTION(BlueprintCallable, Category = "Overlay")
	void SetOverlaySettings(const FMGPhotoOverlaySettings& Settings);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGPhotoOverlaySettings CurrentSettings;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Logo toggled */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnLogoToggled(bool bEnabled);

	/** Frame toggled */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnFrameToggled(bool bEnabled);

	/** Frame style changed */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnFrameStyleChanged(int32 Style);

	/** Date stamp toggled */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnDateStampToggled(bool bEnabled);

	/** Vehicle info toggled */
	UFUNCTION(BlueprintCallable, Category = "Controls")
	void OnVehicleInfoToggled(bool bEnabled);
};

/**
 * UMGPhotoThumbnailWidget - A single photo thumbnail in the gallery.
 *
 * Represents one saved photo with:
 * - Thumbnail preview image
 * - Selection highlight state
 * - Click to select
 * - Option to delete
 *
 * DELEGATE PATTERN:
 * This widget uses delegates (OnSelected, OnDeleteRequested) to communicate
 * with its parent gallery widget. This keeps the thumbnail "dumb" - it just
 * broadcasts events, and the gallery handles the actual logic.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPhotoThumbnailWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhotoSelected, const FMGPhotoInfo&, Photo);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhotoDeleted, const FString&, PhotoID);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhotoSelected OnSelected;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhotoDeleted OnDeleteRequested;

	/** Set photo data */
	UFUNCTION(BlueprintCallable, Category = "Photo")
	void SetPhotoData(const FMGPhotoInfo& Photo);

	/** Set selected state */
	UFUNCTION(BlueprintCallable, Category = "Photo")
	void SetSelected(bool bSelected);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGPhotoInfo PhotoData;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsSelected = false;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Handle click */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void HandleClick();

	/** Handle delete */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void HandleDelete();
};

/**
 * UMGPhotoGalleryWidget - Browse and manage saved photos.
 *
 * Displays a grid of thumbnails for all photos the player has taken.
 * Supports:
 * - Browsing through saved photos
 * - Selecting photos to view full-size
 * - Deleting unwanted photos
 * - Sharing photos (platform-dependent)
 *
 * WIDGET POOLING:
 * The ThumbnailWidgets array maintains a pool of reusable thumbnail widgets.
 * Instead of creating/destroying widgets as photos are added/removed,
 * we reuse existing ones. This improves performance, especially with many photos.
 *
 * FILE SYSTEM NOTE:
 * Photos are stored on disk (in the PhotoSaveDirectory from the subsystem).
 * This widget reads from that directory and displays thumbnails.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPhotoGalleryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Refresh gallery */
	UFUNCTION(BlueprintCallable, Category = "Gallery")
	void RefreshGallery();

	/** Select photo */
	UFUNCTION(BlueprintCallable, Category = "Gallery")
	void SelectPhoto(int32 Index);

	/** Get selected photo */
	UFUNCTION(BlueprintPure, Category = "Gallery")
	FMGPhotoInfo GetSelectedPhoto() const;

	/** Delete selected photo */
	UFUNCTION(BlueprintCallable, Category = "Gallery")
	void DeleteSelectedPhoto();

	/** Share selected photo */
	UFUNCTION(BlueprintCallable, Category = "Gallery")
	void ShareSelectedPhoto();

protected:
	/** Thumbnail widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGPhotoThumbnailWidget> ThumbnailWidgetClass;

	/** Thumbnail widgets */
	UPROPERTY()
	TArray<UMGPhotoThumbnailWidget*> ThumbnailWidgets;

	/** Current photos */
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TArray<FMGPhotoInfo> CurrentPhotos;

	/** Selected index */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 SelectedIndex = -1;

	UPROPERTY()
	UMGPhotoModeSubsystem* PhotoModeSubsystem;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Handle thumbnail selected */
	UFUNCTION()
	void OnThumbnailSelected(const FMGPhotoInfo& Photo);

	/** Create thumbnail widget */
	UMGPhotoThumbnailWidget* CreateThumbnailWidget();
};

/**
 * UMGPhotoModeHUDWidget - The main container widget for all photo mode UI.
 *
 * This is the top-level widget that contains:
 * - Tab navigation (Camera, Visual, Overlay tabs)
 * - Settings panels for each tab
 * - Control hints showing available inputs
 * - Capture feedback (flash effect when taking a photo)
 *
 * TAB SYSTEM:
 * The UI uses tabs to organize settings without overwhelming the screen.
 * Only one tab's content is visible at a time:
 * - Tab 0: Camera settings (CameraWidget)
 * - Tab 1: Visual settings (VisualWidget)
 * - Tab 2: Overlay settings (OverlayWidget)
 *
 * CONTROLS PANEL:
 * The controls panel can be hidden (ToggleControlsPanel) to get an
 * unobstructed view for composing shots. Input hints update based on
 * whether controller or keyboard is being used.
 *
 * CAPTURE WORKFLOW:
 * When TakePhoto() is called:
 * 1. ShowCaptureFeedback() plays a flash/shutter animation
 * 2. The subsystem captures the image
 * 3. OnPhotoCaptured delegate fires
 * 4. UI can show confirmation or allow immediate viewing
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPhotoModeHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Show/hide controls panel */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleControlsPanel();

	/** Switch to camera tab */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowCameraTab();

	/** Switch to visual tab */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowVisualTab();

	/** Switch to overlay tab */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowOverlayTab();

	/** Take photo */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void TakePhoto();

	/** Exit photo mode */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void ExitPhotoMode();

protected:
	/** Current tab */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentTab = 0;

	/** Controls visible */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bControlsVisible = true;

	/** Camera widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UMGPhotoCameraWidget* CameraWidget;

	/** Visual widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UMGPhotoVisualWidget* VisualWidget;

	/** Overlay widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UMGPhotoOverlayWidget* OverlayWidget;

	UPROPERTY()
	UMGPhotoModeSubsystem* PhotoModeSubsystem;

	/** Update tab display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateTabDisplay();

	/** Show capture feedback */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void ShowCaptureFeedback();

	/** Handle photo captured */
	UFUNCTION()
	void OnPhotoCaptured(const FMGPhotoInfo& Photo);

	/** Update input hints */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateInputHints();
};

/**
 * UMGPhotoViewerWidget - Full-screen photo viewer for browsing saved photos.
 *
 * When you want to view a photo at full size (not just thumbnail), this widget
 * takes over the screen and shows:
 * - The full-resolution photo
 * - Navigation controls (next/previous)
 * - Action buttons (delete, share)
 *
 * NAVIGATION:
 * Users can browse through their photos with next/previous controls.
 * The AllPhotos array holds references to all available photos,
 * and CurrentIndex tracks which one is displayed.
 *
 * TEXTURE LOADING:
 * Photos are loaded from disk when viewed. LoadPhotoTexture() handles
 * async loading to prevent hitches. Large 4K images take time to load,
 * so consider showing a loading indicator.
 *
 * SHARING:
 * ShareCurrentPhoto() uses platform APIs (Steam, PlayStation, Xbox, etc.)
 * to share to social media or platform-specific photo features.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPhotoViewerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** View photo */
	UFUNCTION(BlueprintCallable, Category = "Viewer")
	void ViewPhoto(const FMGPhotoInfo& Photo);

	/** Close viewer */
	UFUNCTION(BlueprintCallable, Category = "Viewer")
	void CloseViewer();

	/** Next photo */
	UFUNCTION(BlueprintCallable, Category = "Viewer")
	void NextPhoto();

	/** Previous photo */
	UFUNCTION(BlueprintCallable, Category = "Viewer")
	void PreviousPhoto();

	/** Delete current photo */
	UFUNCTION(BlueprintCallable, Category = "Viewer")
	void DeleteCurrentPhoto();

	/** Share current photo */
	UFUNCTION(BlueprintCallable, Category = "Viewer")
	void ShareCurrentPhoto();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGPhotoInfo CurrentPhoto;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentIndex = 0;

	UPROPERTY()
	TArray<FMGPhotoInfo> AllPhotos;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Load photo texture */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void LoadPhotoTexture(const FString& FilePath);
};

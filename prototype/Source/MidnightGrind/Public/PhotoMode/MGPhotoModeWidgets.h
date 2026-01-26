// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGPhotoModeSubsystem.h"
#include "MGPhotoModeWidgets.generated.h"

class USlider;
class UTextBlock;
class UImage;
class UButton;

/**
 * Photo mode camera controls widget
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
 * Photo mode visual settings widget
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
 * Photo mode overlay settings widget
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
 * Photo gallery thumbnail widget
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
 * Photo gallery widget
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
 * Photo mode main HUD widget
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
 * Photo viewer (full-screen) widget
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

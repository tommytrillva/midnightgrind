// Copyright Midnight Grind. All Rights Reserved.

#include "PhotoMode/MGPhotoModeWidgets.h"
#include "PhotoMode/MGPhotoModeSubsystem.h"
#include "Kismet/GameplayStatics.h"

// ==========================================
// UMGPhotoCameraWidget
// ==========================================

void UMGPhotoCameraWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		PhotoModeSubsystem = World->GetSubsystem<UMGPhotoModeSubsystem>();
		if (PhotoModeSubsystem)
		{
			CurrentSettings = PhotoModeSubsystem->GetCameraSettings();
		}
	}

	UpdateDisplay();
}

void UMGPhotoCameraWidget::SetCameraSettings(const FMGPhotoCameraSettings& Settings)
{
	CurrentSettings = Settings;
	UpdateDisplay();
}

void UMGPhotoCameraWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGPhotoCameraWidget::OnFOVChanged(float Value)
{
	CurrentSettings.FieldOfView = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetFieldOfView(Value);
	}
}

void UMGPhotoCameraWidget::OnFocalDistanceChanged(float Value)
{
	CurrentSettings.FocalDistance = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetFocalDistance(Value);
	}
}

void UMGPhotoCameraWidget::OnApertureChanged(float Value)
{
	CurrentSettings.Aperture = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetAperture(Value);
	}
}

void UMGPhotoCameraWidget::OnDOFToggled(bool bEnabled)
{
	CurrentSettings.bEnableDepthOfField = bEnabled;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetDepthOfFieldEnabled(bEnabled);
	}
}

void UMGPhotoCameraWidget::OnRollChanged(float Value)
{
	CurrentSettings.Roll = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetCameraRoll(Value);
	}
}

void UMGPhotoCameraWidget::OnCameraModeChanged(int32 ModeIndex)
{
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetCameraMode(static_cast<EMGPhotoCamera>(ModeIndex));
	}
}

void UMGPhotoCameraWidget::OnResetCamera()
{
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->ResetCamera();
		CurrentSettings = PhotoModeSubsystem->GetCameraSettings();
		UpdateDisplay();
	}
}

// ==========================================
// UMGPhotoVisualWidget
// ==========================================

void UMGPhotoVisualWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		PhotoModeSubsystem = World->GetSubsystem<UMGPhotoModeSubsystem>();
		if (PhotoModeSubsystem)
		{
			CurrentSettings = PhotoModeSubsystem->GetVisualSettings();
		}
	}

	UpdateDisplay();
}

void UMGPhotoVisualWidget::SetVisualSettings(const FMGPhotoVisualSettings& Settings)
{
	CurrentSettings = Settings;
	UpdateDisplay();
}

void UMGPhotoVisualWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGPhotoVisualWidget::OnFilterChanged(int32 FilterIndex)
{
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetFilter(static_cast<EMGPhotoFilter>(FilterIndex));
		CurrentSettings = PhotoModeSubsystem->GetVisualSettings();
		UpdateDisplay();
	}
}

void UMGPhotoVisualWidget::OnExposureChanged(float Value)
{
	CurrentSettings.Exposure = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetExposure(Value);
	}
}

void UMGPhotoVisualWidget::OnContrastChanged(float Value)
{
	CurrentSettings.Contrast = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetContrast(Value);
	}
}

void UMGPhotoVisualWidget::OnSaturationChanged(float Value)
{
	CurrentSettings.Saturation = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetSaturation(Value);
	}
}

void UMGPhotoVisualWidget::OnTemperatureChanged(float Value)
{
	CurrentSettings.Temperature = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetVisualSettings(CurrentSettings);
	}
}

void UMGPhotoVisualWidget::OnVignetteChanged(float Value)
{
	CurrentSettings.Vignette = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetVisualSettings(CurrentSettings);
	}
}

void UMGPhotoVisualWidget::OnFilmGrainChanged(float Value)
{
	CurrentSettings.FilmGrain = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetVisualSettings(CurrentSettings);
	}
}

void UMGPhotoVisualWidget::OnBloomChanged(float Value)
{
	CurrentSettings.Bloom = Value;
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SetVisualSettings(CurrentSettings);
	}
}

void UMGPhotoVisualWidget::OnResetVisual()
{
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->ResetVisualSettings();
		CurrentSettings = PhotoModeSubsystem->GetVisualSettings();
		UpdateDisplay();
	}
}

// ==========================================
// UMGPhotoOverlayWidget
// ==========================================

void UMGPhotoOverlayWidget::SetOverlaySettings(const FMGPhotoOverlaySettings& Settings)
{
	CurrentSettings = Settings;
	UpdateDisplay();
}

void UMGPhotoOverlayWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGPhotoOverlayWidget::OnLogoToggled(bool bEnabled)
{
	CurrentSettings.bShowLogo = bEnabled;
}

void UMGPhotoOverlayWidget::OnFrameToggled(bool bEnabled)
{
	CurrentSettings.bShowFrame = bEnabled;
}

void UMGPhotoOverlayWidget::OnFrameStyleChanged(int32 Style)
{
	CurrentSettings.FrameStyle = Style;
	CurrentSettings.bShowFrame = true;
}

void UMGPhotoOverlayWidget::OnDateStampToggled(bool bEnabled)
{
	CurrentSettings.bShowDateStamp = bEnabled;
}

void UMGPhotoOverlayWidget::OnVehicleInfoToggled(bool bEnabled)
{
	CurrentSettings.bShowVehicleInfo = bEnabled;
}

// ==========================================
// UMGPhotoThumbnailWidget
// ==========================================

void UMGPhotoThumbnailWidget::SetPhotoData(const FMGPhotoInfo& Photo)
{
	PhotoData = Photo;
	UpdateDisplay();
}

void UMGPhotoThumbnailWidget::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
	UpdateDisplay();
}

void UMGPhotoThumbnailWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation - load thumbnail texture
}

void UMGPhotoThumbnailWidget::HandleClick()
{
	OnSelected.Broadcast(PhotoData);
}

void UMGPhotoThumbnailWidget::HandleDelete()
{
	OnDeleteRequested.Broadcast(PhotoData.PhotoID);
}

// ==========================================
// UMGPhotoGalleryWidget
// ==========================================

void UMGPhotoGalleryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		PhotoModeSubsystem = World->GetSubsystem<UMGPhotoModeSubsystem>();
	}

	RefreshGallery();
}

void UMGPhotoGalleryWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMGPhotoGalleryWidget::RefreshGallery()
{
	if (PhotoModeSubsystem)
	{
		CurrentPhotos = PhotoModeSubsystem->GetAllPhotos();
	}

	UpdateDisplay();
}

void UMGPhotoGalleryWidget::SelectPhoto(int32 Index)
{
	if (Index < 0 || Index >= CurrentPhotos.Num())
	{
		return;
	}

	// Deselect previous
	if (SelectedIndex >= 0 && SelectedIndex < ThumbnailWidgets.Num())
	{
		ThumbnailWidgets[SelectedIndex]->SetSelected(false);
	}

	SelectedIndex = Index;

	// Select new
	if (SelectedIndex >= 0 && SelectedIndex < ThumbnailWidgets.Num())
	{
		ThumbnailWidgets[SelectedIndex]->SetSelected(true);
	}
}

FMGPhotoInfo UMGPhotoGalleryWidget::GetSelectedPhoto() const
{
	if (SelectedIndex >= 0 && SelectedIndex < CurrentPhotos.Num())
	{
		return CurrentPhotos[SelectedIndex];
	}
	return FMGPhotoInfo();
}

void UMGPhotoGalleryWidget::DeleteSelectedPhoto()
{
	if (SelectedIndex >= 0 && SelectedIndex < CurrentPhotos.Num() && PhotoModeSubsystem)
	{
		PhotoModeSubsystem->DeletePhoto(CurrentPhotos[SelectedIndex].PhotoID);
		RefreshGallery();
	}
}

void UMGPhotoGalleryWidget::ShareSelectedPhoto()
{
	if (SelectedIndex >= 0 && SelectedIndex < CurrentPhotos.Num() && PhotoModeSubsystem)
	{
		PhotoModeSubsystem->SharePhoto(CurrentPhotos[SelectedIndex].PhotoID);
	}
}

void UMGPhotoGalleryWidget::UpdateDisplay_Implementation()
{
	// Ensure enough widgets
	while (ThumbnailWidgets.Num() < CurrentPhotos.Num())
	{
		UMGPhotoThumbnailWidget* Widget = CreateThumbnailWidget();
		if (Widget)
		{
			Widget->OnSelected.AddDynamic(this, &UMGPhotoGalleryWidget::OnThumbnailSelected);
			ThumbnailWidgets.Add(Widget);
		}
	}

	// Update visible widgets
	for (int32 i = 0; i < CurrentPhotos.Num(); i++)
	{
		if (i < ThumbnailWidgets.Num() && ThumbnailWidgets[i])
		{
			ThumbnailWidgets[i]->SetPhotoData(CurrentPhotos[i]);
			ThumbnailWidgets[i]->SetVisibility(ESlateVisibility::Visible);
		}
	}

	// Hide unused
	for (int32 i = CurrentPhotos.Num(); i < ThumbnailWidgets.Num(); i++)
	{
		if (ThumbnailWidgets[i])
		{
			ThumbnailWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMGPhotoGalleryWidget::OnThumbnailSelected(const FMGPhotoInfo& Photo)
{
	for (int32 i = 0; i < CurrentPhotos.Num(); i++)
	{
		if (CurrentPhotos[i].PhotoID == Photo.PhotoID)
		{
			SelectPhoto(i);
			break;
		}
	}
}

UMGPhotoThumbnailWidget* UMGPhotoGalleryWidget::CreateThumbnailWidget()
{
	if (!ThumbnailWidgetClass)
	{
		return nullptr;
	}
	return CreateWidget<UMGPhotoThumbnailWidget>(this, ThumbnailWidgetClass);
}

// ==========================================
// UMGPhotoModeHUDWidget
// ==========================================

void UMGPhotoModeHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		PhotoModeSubsystem = World->GetSubsystem<UMGPhotoModeSubsystem>();
		if (PhotoModeSubsystem)
		{
			PhotoModeSubsystem->OnPhotoCaptured.AddDynamic(this, &UMGPhotoModeHUDWidget::OnPhotoCaptured);
		}
	}

	UpdateTabDisplay();
	UpdateInputHints();
}

void UMGPhotoModeHUDWidget::NativeDestruct()
{
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->OnPhotoCaptured.RemoveDynamic(this, &UMGPhotoModeHUDWidget::OnPhotoCaptured);
	}
	Super::NativeDestruct();
}

void UMGPhotoModeHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// Handle input updates
}

void UMGPhotoModeHUDWidget::ToggleControlsPanel()
{
	bControlsVisible = !bControlsVisible;
	UpdateTabDisplay();
}

void UMGPhotoModeHUDWidget::ShowCameraTab()
{
	CurrentTab = 0;
	UpdateTabDisplay();
}

void UMGPhotoModeHUDWidget::ShowVisualTab()
{
	CurrentTab = 1;
	UpdateTabDisplay();
}

void UMGPhotoModeHUDWidget::ShowOverlayTab()
{
	CurrentTab = 2;
	UpdateTabDisplay();
}

void UMGPhotoModeHUDWidget::TakePhoto()
{
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->CapturePhoto();
	}
}

void UMGPhotoModeHUDWidget::ExitPhotoMode()
{
	if (PhotoModeSubsystem)
	{
		PhotoModeSubsystem->ExitPhotoMode();
	}
}

void UMGPhotoModeHUDWidget::UpdateTabDisplay_Implementation()
{
	// Blueprint implementation - show/hide tab content
	if (CameraWidget)
	{
		CameraWidget->SetVisibility(CurrentTab == 0 && bControlsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (VisualWidget)
	{
		VisualWidget->SetVisibility(CurrentTab == 1 && bControlsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (OverlayWidget)
	{
		OverlayWidget->SetVisibility(CurrentTab == 2 && bControlsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMGPhotoModeHUDWidget::ShowCaptureFeedback_Implementation()
{
	// Blueprint implementation - flash screen, play sound
}

void UMGPhotoModeHUDWidget::OnPhotoCaptured(const FMGPhotoInfo& Photo)
{
	ShowCaptureFeedback();
}

void UMGPhotoModeHUDWidget::UpdateInputHints_Implementation()
{
	// Blueprint implementation - update button prompts
}

// ==========================================
// UMGPhotoViewerWidget
// ==========================================

void UMGPhotoViewerWidget::ViewPhoto(const FMGPhotoInfo& Photo)
{
	CurrentPhoto = Photo;

	// Get all photos
	if (UWorld* World = GetWorld())
	{
		if (UMGPhotoModeSubsystem* Subsystem = World->GetSubsystem<UMGPhotoModeSubsystem>())
		{
			AllPhotos = Subsystem->GetAllPhotos();

			// Find current index
			for (int32 i = 0; i < AllPhotos.Num(); i++)
			{
				if (AllPhotos[i].PhotoID == Photo.PhotoID)
				{
					CurrentIndex = i;
					break;
				}
			}
		}
	}

	UpdateDisplay();
	SetVisibility(ESlateVisibility::Visible);
}

void UMGPhotoViewerWidget::CloseViewer()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UMGPhotoViewerWidget::NextPhoto()
{
	if (AllPhotos.Num() == 0)
	{
		return;
	}

	CurrentIndex = (CurrentIndex + 1) % AllPhotos.Num();
	CurrentPhoto = AllPhotos[CurrentIndex];
	UpdateDisplay();
}

void UMGPhotoViewerWidget::PreviousPhoto()
{
	if (AllPhotos.Num() == 0)
	{
		return;
	}

	CurrentIndex = CurrentIndex > 0 ? CurrentIndex - 1 : AllPhotos.Num() - 1;
	CurrentPhoto = AllPhotos[CurrentIndex];
	UpdateDisplay();
}

void UMGPhotoViewerWidget::DeleteCurrentPhoto()
{
	if (UWorld* World = GetWorld())
	{
		if (UMGPhotoModeSubsystem* Subsystem = World->GetSubsystem<UMGPhotoModeSubsystem>())
		{
			Subsystem->DeletePhoto(CurrentPhoto.PhotoID);
			AllPhotos = Subsystem->GetAllPhotos();

			if (AllPhotos.Num() > 0)
			{
				CurrentIndex = FMath::Min(CurrentIndex, AllPhotos.Num() - 1);
				CurrentPhoto = AllPhotos[CurrentIndex];
				UpdateDisplay();
			}
			else
			{
				CloseViewer();
			}
		}
	}
}

void UMGPhotoViewerWidget::ShareCurrentPhoto()
{
	if (UWorld* World = GetWorld())
	{
		if (UMGPhotoModeSubsystem* Subsystem = World->GetSubsystem<UMGPhotoModeSubsystem>())
		{
			Subsystem->SharePhoto(CurrentPhoto.PhotoID);
		}
	}
}

void UMGPhotoViewerWidget::UpdateDisplay_Implementation()
{
	LoadPhotoTexture(CurrentPhoto.FilePath);
}

void UMGPhotoViewerWidget::LoadPhotoTexture_Implementation(const FString& FilePath)
{
	// Blueprint implementation - load texture from file
}

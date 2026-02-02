// Copyright Midnight Grind. All Rights Reserved.

#include "PhotoMode/MGPhotoModeSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

void UMGPhotoModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSavedPhotosList();
}

void UMGPhotoModeSubsystem::Deinitialize()
{
	if (bIsActive)
	{
		ExitPhotoMode();
	}
	Super::Deinitialize();
}

void UMGPhotoModeSubsystem::Tick(float MGDeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		UpdateCamera(DeltaTime);
	}
}

// ==========================================
// PHOTO MODE CONTROL
// ==========================================

void UMGPhotoModeSubsystem::EnterPhotoMode()
{
	if (bIsActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get player controller and pawn
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return;
	}

	APawn* PlayerPawn = PC->GetPawn();
	if (PlayerPawn)
	{
		TargetVehicle = PlayerPawn;
		OriginalCameraLocation = PlayerPawn->GetActorLocation();
		OriginalCameraRotation = PlayerPawn->GetActorRotation();
	}

	bIsActive = true;
	bScenePaused = true;

	// Pause the game
	UGameplayStatics::SetGamePaused(World, true);

	// Setup camera
	SetupPhotoModeCamera();

	// Initialize camera position
	ResetCamera();

	OnPhotoModeEntered.Broadcast();
}

void UMGPhotoModeSubsystem::ExitPhotoMode()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;

	// Cleanup
	CleanupPhotoModeCamera();

	// Unpause
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, false);
	}

	OnPhotoModeExited.Broadcast();
}

void UMGPhotoModeSubsystem::TogglePhotoMode()
{
	if (bIsActive)
	{
		ExitPhotoMode();
	}
	else
	{
		EnterPhotoMode();
	}
}

// ==========================================
// CAMERA CONTROL
// ==========================================

void UMGPhotoModeSubsystem::SetCameraMode(EMGPhotoCamera Mode)
{
	CurrentCameraMode = Mode;
	OnCameraModeChanged.Broadcast(Mode);
}

void UMGPhotoModeSubsystem::MoveCamera(FVector Delta)
{
	if (!bIsActive || CurrentCameraMode != EMGPhotoCamera::Free)
	{
		return;
	}

	// Transform delta to camera space
	UWorld* World = GetWorld();
	float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
	FVector WorldDelta = CameraRotation.RotateVector(Delta) * CameraMoveSpeed * DeltaSeconds;
	FVector NewLocation = CameraLocation + WorldDelta;

	// Clamp distance from vehicle
	if (TargetVehicle)
	{
		FVector ToVehicle = TargetVehicle->GetActorLocation() - NewLocation;
		if (ToVehicle.Size() > MaxCameraDistance)
		{
			NewLocation = TargetVehicle->GetActorLocation() - ToVehicle.GetSafeNormal() * MaxCameraDistance;
		}
	}

	CameraLocation = NewLocation;
}

void UMGPhotoModeSubsystem::RotateCamera(FRotator Delta)
{
	if (!bIsActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
	FRotator ScaledDelta = Delta * CameraRotateSpeed * DeltaSeconds;
	CameraRotation += ScaledDelta;

	// Clamp pitch
	CameraRotation.Pitch = FMath::Clamp(CameraRotation.Pitch, -89.0f, 89.0f);
	CameraRotation.Normalize();
}

void UMGPhotoModeSubsystem::OrbitCamera(float YawDelta, float PitchDelta)
{
	if (!bIsActive || CurrentCameraMode != EMGPhotoCamera::Orbit)
	{
		return;
	}

	UWorld* World = GetWorld();
	float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
	OrbitYaw += YawDelta * OrbitSpeed * DeltaSeconds;
	OrbitPitch += PitchDelta * OrbitSpeed * DeltaSeconds;

	// Clamp pitch
	OrbitPitch = FMath::Clamp(OrbitPitch, -80.0f, 80.0f);
}

void UMGPhotoModeSubsystem::ZoomCamera(float Delta)
{
	if (!bIsActive)
	{
		return;
	}

	if (CurrentCameraMode == EMGPhotoCamera::Orbit)
	{
		CameraSettings.OrbitDistance = FMath::Clamp(
			CameraSettings.OrbitDistance - Delta * 100.0f,
			100.0f,
			5000.0f
		);
	}
	else
	{
		CameraSettings.FieldOfView = FMath::Clamp(
			CameraSettings.FieldOfView - Delta * 5.0f,
			10.0f,
			150.0f
		);
	}
}

void UMGPhotoModeSubsystem::ResetCamera()
{
	if (!TargetVehicle)
	{
		return;
	}

	// Reset to behind vehicle
	OrbitYaw = 180.0f;
	OrbitPitch = 15.0f;
	CameraSettings.OrbitDistance = 500.0f;
	CameraSettings.OrbitHeight = 100.0f;
	CameraSettings.FieldOfView = 90.0f;
	CameraSettings.Roll = 0.0f;

	// Calculate initial position
	FVector VehicleLocation = TargetVehicle->GetActorLocation();
	FRotator OrbitRotation(OrbitPitch, OrbitYaw, 0.0f);
	FVector Offset = OrbitRotation.RotateVector(FVector(-CameraSettings.OrbitDistance, 0.0f, CameraSettings.OrbitHeight));

	CameraLocation = VehicleLocation + Offset;
	CameraRotation = (VehicleLocation - CameraLocation).Rotation();
}

void UMGPhotoModeSubsystem::SetCameraSettings(const FMGPhotoCameraSettings& Settings)
{
	CameraSettings = Settings;
	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::SetFieldOfView(float FOV)
{
	CameraSettings.FieldOfView = FMath::Clamp(FOV, 10.0f, 150.0f);
}

void UMGPhotoModeSubsystem::SetFocalDistance(float Distance)
{
	CameraSettings.FocalDistance = FMath::Max(10.0f, Distance);
	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::SetAperture(float FStop)
{
	CameraSettings.Aperture = FMath::Clamp(FStop, 1.0f, 22.0f);
	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::SetDepthOfFieldEnabled(bool bEnabled)
{
	CameraSettings.bEnableDepthOfField = bEnabled;
	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::SetCameraRoll(float Roll)
{
	CameraSettings.Roll = FMath::Clamp(Roll, -90.0f, 90.0f);
}

// ==========================================
// VISUAL SETTINGS
// ==========================================

void UMGPhotoModeSubsystem::SetFilter(EMGPhotoFilter Filter)
{
	VisualSettings.Filter = Filter;

	if (Filter != EMGPhotoFilter::Custom && Filter != EMGPhotoFilter::None)
	{
		VisualSettings = GetFilterPreset(Filter);
		VisualSettings.Filter = Filter;
	}

	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::SetVisualSettings(const FMGPhotoVisualSettings& Settings)
{
	VisualSettings = Settings;
	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::SetExposure(float Value)
{
	VisualSettings.Exposure = FMath::Clamp(Value, -3.0f, 3.0f);
	VisualSettings.Filter = EMGPhotoFilter::Custom;
	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::SetContrast(float Value)
{
	VisualSettings.Contrast = FMath::Clamp(Value, 0.5f, 1.5f);
	VisualSettings.Filter = EMGPhotoFilter::Custom;
	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::SetSaturation(float Value)
{
	VisualSettings.Saturation = FMath::Clamp(Value, 0.0f, 2.0f);
	VisualSettings.Filter = EMGPhotoFilter::Custom;
	ApplyVisualSettings();
}

void UMGPhotoModeSubsystem::ResetVisualSettings()
{
	VisualSettings = FMGPhotoVisualSettings();
	ApplyVisualSettings();
}

// ==========================================
// OVERLAYS
// ==========================================

void UMGPhotoModeSubsystem::SetOverlaySettings(const FMGPhotoOverlaySettings& Settings)
{
	OverlaySettings = Settings;
}

void UMGPhotoModeSubsystem::ToggleLogo()
{
	OverlaySettings.bShowLogo = !OverlaySettings.bShowLogo;
}

void UMGPhotoModeSubsystem::CycleFrameStyle()
{
	OverlaySettings.FrameStyle = (OverlaySettings.FrameStyle + 1) % 5;
	OverlaySettings.bShowFrame = true;
}

// ==========================================
// SCENE CONTROL
// ==========================================

void UMGPhotoModeSubsystem::SetScenePaused(bool bPaused)
{
	bScenePaused = bPaused;
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, bPaused);
	}
}

void UMGPhotoModeSubsystem::ToggleScenePause()
{
	SetScenePaused(!bScenePaused);
}

void UMGPhotoModeSubsystem::SetVehicleHidden(bool bHidden)
{
	if (TargetVehicle)
	{
		TargetVehicle->SetActorHiddenInGame(bHidden);
	}
}

void UMGPhotoModeSubsystem::SetHUDHidden(bool bHidden)
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (AHUD* HUD = PC->GetHUD())
			{
				HUD->bShowHUD = !bHidden;
			}
		}
	}
}

// ==========================================
// CAPTURE
// ==========================================

void UMGPhotoModeSubsystem::CapturePhoto()
{
	// Use default resolution
	CaptureHighResPhoto(FIntPoint(1920, 1080));
}

void UMGPhotoModeSubsystem::CaptureHighResPhoto(FIntPoint Resolution)
{
	if (!bIsActive)
	{
		return;
	}

	// Create photo info
	FMGPhotoInfo PhotoInfo;
	PhotoInfo.PhotoID = FGuid::NewGuid().ToString();
	PhotoInfo.Timestamp = FDateTime::Now();
	PhotoInfo.Resolution = Resolution;

	if (TargetVehicle)
	{
		// Would get vehicle ID from vehicle component
	}

	// Generate filename
	FString Filename = GeneratePhotoFilename();
	FString FullPath = FPaths::Combine(FPaths::ProjectSavedDir(), PhotoSaveDirectory, Filename);

	// Ensure directory exists
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullPath), true);

	// In production, would capture from scene capture component
	// For now, just create photo info
	PhotoInfo.FilePath = FullPath;
	PhotoInfo.ThumbnailPath = FullPath.Replace(TEXT(".png"), TEXT("_thumb.png"));

	// Save to list
	SavedPhotos.Add(PhotoInfo);
	LastCapturedPhoto = PhotoInfo;

	OnPhotoCaptured.Broadcast(PhotoInfo);
}

// ==========================================
// GALLERY
// ==========================================

void UMGPhotoModeSubsystem::DeletePhoto(const FString& PhotoID)
{
	for (int32 i = 0; i < SavedPhotos.Num(); i++)
	{
		if (SavedPhotos[i].PhotoID == PhotoID)
		{
			// Delete file
			IFileManager::Get().Delete(*SavedPhotos[i].FilePath);
			IFileManager::Get().Delete(*SavedPhotos[i].ThumbnailPath);

			SavedPhotos.RemoveAt(i);
			return;
		}
	}
}

void UMGPhotoModeSubsystem::SharePhoto(const FString& PhotoID)
{
	for (FMGPhotoInfo& Photo : SavedPhotos)
	{
		if (Photo.PhotoID == PhotoID)
		{
			Photo.bIsShared = true;
			// Would upload to server
			return;
		}
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGPhotoModeSubsystem::SetupPhotoModeCamera()
{
	// Would create scene capture component for high-res captures
}

void UMGPhotoModeSubsystem::CleanupPhotoModeCamera()
{
	// Cleanup scene capture
}

void UMGPhotoModeSubsystem::UpdateCamera(float MGDeltaTime)
{
	if (!TargetVehicle)
	{
		return;
	}

	FVector VehicleLocation = TargetVehicle->GetActorLocation();

	switch (CurrentCameraMode)
	{
		case EMGPhotoCamera::Orbit:
		{
			// Calculate orbit position
			FRotator OrbitRotation(OrbitPitch, OrbitYaw, 0.0f);
			FVector Offset = OrbitRotation.RotateVector(FVector(-CameraSettings.OrbitDistance, 0.0f, CameraSettings.OrbitHeight));
			CameraLocation = VehicleLocation + Offset;
			CameraRotation = (VehicleLocation - CameraLocation).Rotation();
			break;
		}

		case EMGPhotoCamera::Track:
		{
			// Look at vehicle
			CameraRotation = (VehicleLocation - CameraLocation).Rotation();
			break;
		}

		case EMGPhotoCamera::Locked:
		{
			// Follow vehicle
			FVector VehicleForward = TargetVehicle->GetActorForwardVector();
			CameraLocation = VehicleLocation - VehicleForward * CameraSettings.OrbitDistance + FVector(0, 0, CameraSettings.OrbitHeight);
			CameraRotation = VehicleForward.Rotation();
			break;
		}

		case EMGPhotoCamera::Free:
		default:
			// Free camera - position controlled by input
			break;
	}

	// Apply roll
	CameraRotation.Roll = CameraSettings.Roll;

	// Update player controller view
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			PC->SetControlRotation(CameraRotation);
			if (APawn* Pawn = PC->GetPawn())
			{
				// In production, would move camera actor
			}
		}
	}
}

void UMGPhotoModeSubsystem::ApplyVisualSettings()
{
	// Would update post-process volume settings
	// Including DOF, color grading, vignette, etc.
}

FMGPhotoVisualSettings UMGPhotoModeSubsystem::GetFilterPreset(EMGPhotoFilter Filter) const
{
	FMGPhotoVisualSettings Settings;

	switch (Filter)
	{
		case EMGPhotoFilter::Vintage:
			Settings.Saturation = 0.7f;
			Settings.Contrast = 1.1f;
			Settings.Temperature = 0.3f;
			Settings.Vignette = 0.4f;
			Settings.FilmGrain = 0.15f;
			Settings.ColorGrade = FLinearColor(1.1f, 1.0f, 0.85f);
			break;

		case EMGPhotoFilter::Dramatic:
			Settings.Contrast = 1.4f;
			Settings.Saturation = 1.1f;
			Settings.Vignette = 0.3f;
			Settings.Bloom = 0.7f;
			break;

		case EMGPhotoFilter::Noir:
			Settings.Saturation = 0.0f;
			Settings.Contrast = 1.3f;
			Settings.Vignette = 0.5f;
			Settings.FilmGrain = 0.1f;
			break;

		case EMGPhotoFilter::Neon:
			Settings.Saturation = 1.5f;
			Settings.Bloom = 1.5f;
			Settings.ChromaticAberration = 0.3f;
			Settings.ColorGrade = FLinearColor(1.0f, 0.8f, 1.2f);
			break;

		case EMGPhotoFilter::VHS:
			Settings.Saturation = 0.8f;
			Settings.Contrast = 1.2f;
			Settings.ChromaticAberration = 0.5f;
			Settings.FilmGrain = 0.3f;
			Settings.Vignette = 0.3f;
			Settings.ColorGrade = FLinearColor(1.1f, 1.0f, 0.9f);
			break;

		case EMGPhotoFilter::Blueprint:
			Settings.Saturation = 0.0f;
			Settings.ColorGrade = FLinearColor(0.2f, 0.4f, 1.0f);
			Settings.Contrast = 1.5f;
			break;

		case EMGPhotoFilter::NightVision:
			Settings.Saturation = 0.0f;
			Settings.ColorGrade = FLinearColor(0.2f, 1.0f, 0.2f);
			Settings.Bloom = 0.8f;
			Settings.FilmGrain = 0.2f;
			break;

		default:
			break;
	}

	return Settings;
}

FString UMGPhotoModeSubsystem::SavePhotoToDisk(UTextureRenderTarget2D* Texture, const FString& Filename)
{
	if (!Texture)
	{
		return FString();
	}

	// Would use FImageUtils to save texture to PNG
	return Filename;
}

void UMGPhotoModeSubsystem::LoadSavedPhotosList()
{
	// Would scan photo directory and load metadata
	SavedPhotos.Empty();
}

FString UMGPhotoModeSubsystem::GeneratePhotoFilename() const
{
	FDateTime Now = FDateTime::Now();
	return FString::Printf(TEXT("MG_%04d%02d%02d_%02d%02d%02d.png"),
		Now.GetYear(), Now.GetMonth(), Now.GetDay(),
		Now.GetHour(), Now.GetMinute(), Now.GetSecond());
}

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGPhotoModeSubsystem.generated.h"

class APlayerController;
class UCameraComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;

/**
 * Photo mode camera mode
 */
UENUM(BlueprintType)
enum class EMGPhotoCamera : uint8
{
	/** Free camera */
	Free,
	/** Orbit around vehicle */
	Orbit,
	/** Track vehicle */
	Track,
	/** Locked to vehicle */
	Locked
};

/**
 * Photo filter preset
 */
UENUM(BlueprintType)
enum class EMGPhotoFilter : uint8
{
	/** No filter */
	None,
	/** Vintage/sepia */
	Vintage,
	/** High contrast */
	Dramatic,
	/** Film noir */
	Noir,
	/** Neon glow */
	Neon,
	/** VHS/Retro */
	VHS,
	/** Blueprint style */
	Blueprint,
	/** Night vision */
	NightVision,
	/** Custom settings */
	Custom
};

/**
 * Photo camera settings
 */
USTRUCT(BlueprintType)
struct FMGPhotoCameraSettings
{
	GENERATED_BODY()

	/** Field of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10.0", ClampMax = "150.0"))
	float FieldOfView = 90.0f;

	/** Focal distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "10.0"))
	float FocalDistance = 1000.0f;

	/** Aperture (f-stop) for DOF */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "22.0"))
	float Aperture = 4.0f;

	/** Enable depth of field */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableDepthOfField = true;

	/** Camera roll */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-90.0", ClampMax = "90.0"))
	float Roll = 0.0f;

	/** Orbit distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float OrbitDistance = 500.0f;

	/** Orbit height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OrbitHeight = 100.0f;
};

/**
 * Photo visual settings
 */
USTRUCT(BlueprintType)
struct FMGPhotoVisualSettings
{
	GENERATED_BODY()

	/** Current filter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPhotoFilter Filter = EMGPhotoFilter::None;

	/** Exposure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-3.0", ClampMax = "3.0"))
	float Exposure = 0.0f;

	/** Contrast */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float Contrast = 1.0f;

	/** Saturation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Saturation = 1.0f;

	/** Temperature */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Temperature = 0.0f;

	/** Tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Tint = 0.0f;

	/** Vignette intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Vignette = 0.0f;

	/** Film grain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FilmGrain = 0.0f;

	/** Chromatic aberration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChromaticAberration = 0.0f;

	/** Bloom intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Bloom = 0.5f;

	/** Color grade color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ColorGrade = FLinearColor::White;
};

/**
 * Photo overlay settings
 */
USTRUCT(BlueprintType)
struct FMGPhotoOverlaySettings
{
	GENERATED_BODY()

	/** Show logo watermark */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowLogo = true;

	/** Logo position (0-1 screen space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D LogoPosition = FVector2D(0.95f, 0.95f);

	/** Logo scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float LogoScale = 0.5f;

	/** Show frame/border */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowFrame = false;

	/** Frame style index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrameStyle = 0;

	/** Show date stamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDateStamp = false;

	/** Show vehicle info */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowVehicleInfo = false;

	/** Show track info */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTrackInfo = false;
};

/**
 * Saved photo info
 */
USTRUCT(BlueprintType)
struct FMGPhotoInfo
{
	GENERATED_BODY()

	/** Photo ID */
	UPROPERTY(BlueprintReadOnly)
	FString PhotoID;

	/** File path */
	UPROPERTY(BlueprintReadOnly)
	FString FilePath;

	/** Thumbnail path */
	UPROPERTY(BlueprintReadOnly)
	FString ThumbnailPath;

	/** Capture timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Track ID */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Vehicle ID */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/** Resolution */
	UPROPERTY(BlueprintReadOnly)
	FIntPoint Resolution;

	/** Is shared */
	UPROPERTY(BlueprintReadOnly)
	bool bIsShared = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhotoModeEntered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhotoModeExited);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhotoCaptured, const FMGPhotoInfo&, PhotoInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraModeChanged, EMGPhotoCamera, NewMode);

/**
 * Photo Mode Subsystem
 * Manages photo capture and editing
 *
 * Features:
 * - Free camera controls
 * - Filters and effects
 * - Depth of field
 * - Photo export and gallery
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPhotoModeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhotoModeEntered OnPhotoModeEntered;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhotoModeExited OnPhotoModeExited;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhotoCaptured OnPhotoCaptured;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCameraModeChanged OnCameraModeChanged;

	// ==========================================
	// PHOTO MODE CONTROL
	// ==========================================

	/** Enter photo mode */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void EnterPhotoMode();

	/** Exit photo mode */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void ExitPhotoMode();

	/** Is photo mode active */
	UFUNCTION(BlueprintPure, Category = "PhotoMode")
	bool IsPhotoModeActive() const { return bIsActive; }

	/** Toggle photo mode */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void TogglePhotoMode();

	// ==========================================
	// CAMERA CONTROL
	// ==========================================

	/** Set camera mode */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetCameraMode(EMGPhotoCamera Mode);

	/** Get camera mode */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Camera")
	EMGPhotoCamera GetCameraMode() const { return CurrentCameraMode; }

	/** Move camera (free mode) */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void MoveCamera(FVector Delta);

	/** Rotate camera */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void RotateCamera(FRotator Delta);

	/** Orbit camera (orbit mode) */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void OrbitCamera(float YawDelta, float PitchDelta);

	/** Zoom camera */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void ZoomCamera(float Delta);

	/** Reset camera */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void ResetCamera();

	/** Get camera settings */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Camera")
	FMGPhotoCameraSettings GetCameraSettings() const { return CameraSettings; }

	/** Set camera settings */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetCameraSettings(const FMGPhotoCameraSettings& Settings);

	/** Set field of view */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetFieldOfView(float FOV);

	/** Set focal distance */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetFocalDistance(float Distance);

	/** Set aperture */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetAperture(float FStop);

	/** Enable/disable DOF */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetDepthOfFieldEnabled(bool bEnabled);

	/** Set camera roll */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetCameraRoll(float Roll);

	// ==========================================
	// VISUAL SETTINGS
	// ==========================================

	/** Set filter */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetFilter(EMGPhotoFilter Filter);

	/** Get current filter */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Visual")
	EMGPhotoFilter GetCurrentFilter() const { return VisualSettings.Filter; }

	/** Get visual settings */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Visual")
	FMGPhotoVisualSettings GetVisualSettings() const { return VisualSettings; }

	/** Set visual settings */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetVisualSettings(const FMGPhotoVisualSettings& Settings);

	/** Set exposure */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetExposure(float Value);

	/** Set contrast */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetContrast(float Value);

	/** Set saturation */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetSaturation(float Value);

	/** Reset visual settings */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void ResetVisualSettings();

	// ==========================================
	// OVERLAYS
	// ==========================================

	/** Get overlay settings */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Overlay")
	FMGPhotoOverlaySettings GetOverlaySettings() const { return OverlaySettings; }

	/** Set overlay settings */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Overlay")
	void SetOverlaySettings(const FMGPhotoOverlaySettings& Settings);

	/** Toggle logo */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Overlay")
	void ToggleLogo();

	/** Cycle frame style */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Overlay")
	void CycleFrameStyle();

	// ==========================================
	// SCENE CONTROL
	// ==========================================

	/** Pause/unpause scene */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Scene")
	void SetScenePaused(bool bPaused);

	/** Is scene paused */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Scene")
	bool IsScenePaused() const { return bScenePaused; }

	/** Toggle scene pause */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Scene")
	void ToggleScenePause();

	/** Hide player vehicle */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Scene")
	void SetVehicleHidden(bool bHidden);

	/** Hide HUD elements */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Scene")
	void SetHUDHidden(bool bHidden);

	// ==========================================
	// CAPTURE
	// ==========================================

	/** Take photo */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Capture")
	void CapturePhoto();

	/** Take high-res photo */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Capture")
	void CaptureHighResPhoto(FIntPoint Resolution = FIntPoint(3840, 2160));

	/** Get last captured photo */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Capture")
	FMGPhotoInfo GetLastCapturedPhoto() const { return LastCapturedPhoto; }

	// ==========================================
	// GALLERY
	// ==========================================

	/** Get all photos */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Gallery")
	TArray<FMGPhotoInfo> GetAllPhotos() const { return SavedPhotos; }

	/** Delete photo */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Gallery")
	void DeletePhoto(const FString& PhotoID);

	/** Share photo */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Gallery")
	void SharePhoto(const FString& PhotoID);

	/** Get photo count */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Gallery")
	int32 GetPhotoCount() const { return SavedPhotos.Num(); }

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Photo save directory */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FString PhotoSaveDirectory = TEXT("Photos");

	/** Camera move speed */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float CameraMoveSpeed = 500.0f;

	/** Camera rotate speed */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float CameraRotateSpeed = 100.0f;

	/** Orbit speed */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float OrbitSpeed = 50.0f;

	/** Max free camera distance from vehicle */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float MaxCameraDistance = 2000.0f;

	/** Filter post-process material */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UMaterialInterface* FilterMaterial;

	// ==========================================
	// STATE
	// ==========================================

	/** Is photo mode active */
	bool bIsActive = false;

	/** Current camera mode */
	EMGPhotoCamera CurrentCameraMode = EMGPhotoCamera::Orbit;

	/** Camera settings */
	FMGPhotoCameraSettings CameraSettings;

	/** Visual settings */
	FMGPhotoVisualSettings VisualSettings;

	/** Overlay settings */
	FMGPhotoOverlaySettings OverlaySettings;

	/** Is scene paused */
	bool bScenePaused = true;

	/** Current camera location */
	FVector CameraLocation;

	/** Current camera rotation */
	FRotator CameraRotation;

	/** Orbit yaw */
	float OrbitYaw = 0.0f;

	/** Orbit pitch */
	float OrbitPitch = 0.0f;

	/** Target vehicle */
	UPROPERTY()
	AActor* TargetVehicle;

	/** Original camera location */
	FVector OriginalCameraLocation;

	/** Original camera rotation */
	FRotator OriginalCameraRotation;

	/** Scene capture component */
	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;

	/** Render target */
	UPROPERTY()
	UTextureRenderTarget2D* RenderTarget;

	/** Filter material instance */
	UPROPERTY()
	UMaterialInstanceDynamic* FilterMaterialInstance;

	/** Saved photos */
	UPROPERTY()
	TArray<FMGPhotoInfo> SavedPhotos;

	/** Last captured photo */
	FMGPhotoInfo LastCapturedPhoto;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Setup photo mode camera */
	void SetupPhotoModeCamera();

	/** Cleanup photo mode camera */
	void CleanupPhotoModeCamera();

	/** Update camera position */
	void UpdateCamera(float DeltaTime);

	/** Apply visual settings to post-process */
	void ApplyVisualSettings();

	/** Get filter settings */
	FMGPhotoVisualSettings GetFilterPreset(EMGPhotoFilter Filter) const;

	/** Save photo to disk */
	FString SavePhotoToDisk(UTextureRenderTarget2D* Texture, const FString& Filename);

	/** Load saved photos list */
	void LoadSavedPhotosList();

	/** Generate photo filename */
	FString GeneratePhotoFilename() const;
};

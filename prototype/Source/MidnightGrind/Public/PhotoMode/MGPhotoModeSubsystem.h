// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPhotoModeSubsystem.h
 * @brief Photo Mode Subsystem for in-game photography and image capture.
 *
 * This subsystem provides a comprehensive photo mode feature that allows players
 * to pause the game, position a free-moving camera, apply visual filters and effects,
 * and capture high-quality screenshots. The system supports multiple camera modes
 * (free, orbit, track, locked), various filter presets, depth of field controls,
 * and customizable overlays.
 *
 * Key Features:
 * - Multiple camera modes for flexible shot composition
 * - Real-time filter and post-processing adjustments
 * - Depth of field with adjustable focal distance and aperture
 * - Logo watermarks, frames, and informational overlays
 * - Photo gallery management with save/delete/share functionality
 * - High-resolution capture support up to 4K
 *
 * Usage:
 * @code
 * UMGPhotoModeSubsystem* PhotoMode = GetWorld()->GetSubsystem<UMGPhotoModeSubsystem>();
 * PhotoMode->EnterPhotoMode();
 * PhotoMode->SetFilter(EMGPhotoFilter::Noir);
 * PhotoMode->CapturePhoto();
 * @endcode
 *
 * @see UWorldSubsystem
 * @see EMGPhotoCamera
 * @see EMGPhotoFilter
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGPhotoModeSubsystem.generated.h"

// Forward declarations
class APlayerController;      ///< Player controller for input handling
class UCameraComponent;       ///< Camera component for view management
class USceneCaptureComponent2D; ///< Scene capture for rendering photos
class UTextureRenderTarget2D; ///< Render target for photo output
class UMaterialInstanceDynamic; ///< Dynamic material for filter effects

// ============================================================================
// ENUMERATIONS - Camera and Filter Types
// ============================================================================

/**
 * @brief Photo mode camera movement modes.
 *
 * Defines how the camera behaves and moves while in photo mode. Each mode
 * offers different creative control for composing shots.
 */
UENUM(BlueprintType)
enum class EMGPhotoCamera : uint8
{
	/** Free camera - Full 6DOF movement, player has complete control over position and rotation */
	Free,
	/** Orbit around vehicle - Camera rotates around the target vehicle at a fixed distance */
	Orbit,
	/** Track vehicle - Camera follows vehicle movement while maintaining relative position */
	Track,
	/** Locked to vehicle - Camera is rigidly attached to the vehicle, moves with it */
	Locked
};

/**
 * @brief Photo filter presets for visual styling.
 *
 * Predefined visual filters that modify color grading, contrast, saturation,
 * and other post-processing effects to achieve specific artistic looks.
 * Use Custom to apply user-defined settings from FMGPhotoVisualSettings.
 */
UENUM(BlueprintType)
enum class EMGPhotoFilter : uint8
{
	/** No filter - Raw captured image without post-processing modifications */
	None,
	/** Vintage/sepia - Warm tones with faded colors, nostalgic film look */
	Vintage,
	/** High contrast - Dramatic shadows and highlights, punchy colors */
	Dramatic,
	/** Film noir - Black and white with high contrast, classic cinema style */
	Noir,
	/** Neon glow - Enhanced bloom with vibrant neon color palette */
	Neon,
	/** VHS/Retro - Scan lines, chromatic aberration, 80s/90s aesthetic */
	VHS,
	/** Blueprint style - Technical wireframe look with blue tint */
	Blueprint,
	/** Night vision - Green tinted with enhanced brightness, military style */
	NightVision,
	/** Custom settings - Uses manually configured FMGPhotoVisualSettings values */
	Custom
};

// ============================================================================
// STRUCTURES - Camera, Visual, and Overlay Configuration
// ============================================================================

/**
 * @brief Camera configuration settings for photo mode.
 *
 * Contains all adjustable camera parameters including field of view,
 * depth of field settings, and orbit mode configuration. These settings
 * directly affect the captured image composition and focus.
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
 * @brief Visual and post-processing settings for photos.
 *
 * Controls all image adjustments including exposure, color correction,
 * and visual effects like vignette and film grain. When Filter is set
 * to Custom, these individual values are applied directly. When using
 * a preset filter, these values are overridden by the preset.
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
 * @brief Overlay and watermark settings for captured photos.
 *
 * Configures optional visual elements that appear on top of the captured
 * image, such as game logos, decorative frames, timestamps, and contextual
 * information about the vehicle and track.
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
 * @brief Metadata for a saved photo.
 *
 * Contains all information about a captured and saved photo, including
 * file paths, capture context (track, vehicle), and sharing status.
 * Used by the gallery system to display and manage saved photos.
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

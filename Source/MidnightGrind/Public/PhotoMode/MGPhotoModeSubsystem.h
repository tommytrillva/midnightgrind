// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGPhotoModeSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * This file defines the Photo Mode Subsystem - the "brain" behind the in-game
 * photography feature that lets players pause the game, compose shots with a
 * free camera, apply visual effects/filters, and capture screenshots.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. WHAT IS A SUBSYSTEM?
 *    - A subsystem is an automatically-created singleton that lives alongside
 *      something else (in this case, the game World)
 *    - UWorldSubsystem exists once per loaded level/world
 *    - It's automatically created when the level loads - no manual setup
 *    - Access via: GetWorld()->GetSubsystem<UMGPhotoModeSubsystem>()
 *
 * 2. WHAT IS PHOTO MODE?
 *    - A feature in many modern games (Forza, Gran Turismo, Spider-Man, etc.)
 *    - Pauses gameplay so you can compose the perfect shot
 *    - Gives you a free camera to position anywhere
 *    - Applies Instagram-like filters and effects
 *    - Saves high-quality screenshots to your device
 *
 * 3. RENDER TARGETS AND SCENE CAPTURE:
 *    - A "Render Target" is like a virtual photograph - it captures what a
 *      camera sees into a texture
 *    - "Scene Capture" is a special component that renders the scene to a
 *      render target (instead of to the screen)
 *    - This lets us apply filters and then save the result as an image file
 *
 * 4. POST-PROCESSING:
 *    - Effects applied AFTER the scene is rendered (hence "post")
 *    - Includes: color correction, blur, vignette, bloom, film grain
 *    - Controlled via Materials (FilterMaterialInstance) with parameters
 *    - Each filter preset just sets different material parameters
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Player presses Photo Mode button]
 *                |
 *                v
 *    [MGPhotoModeSubsystem::EnterPhotoMode()]
 *                |
 *    +-----------+-----------+
 *    |           |           |
 *    v           v           v
 * [Pause Game] [Hide HUD] [Setup Camera]
 *                |
 *                v
 *    [Player adjusts camera, applies filters]
 *                |
 *                v
 *    [MGPhotoModeSubsystem::CapturePhoto()]
 *                |
 *                v
 *    [Scene Capture -> Render Target -> Save to Disk]
 *                |
 *                v
 *    [Photo added to gallery, OnPhotoCaptured event fires]
 *
 * USAGE EXAMPLE:
 * @code
 * // Get the subsystem
 * UMGPhotoModeSubsystem* PhotoMode = GetWorld()->GetSubsystem<UMGPhotoModeSubsystem>();
 *
 * // Enter photo mode
 * PhotoMode->EnterPhotoMode();
 *
 * // Apply a filter
 * PhotoMode->SetFilter(EMGPhotoFilter::Noir);
 *
 * // Adjust depth of field
 * PhotoMode->SetDepthOfFieldEnabled(true);
 * PhotoMode->SetFocalDistance(500.0f);  // Focus at 5 meters
 * PhotoMode->SetAperture(2.8f);         // Shallow DOF for bokeh
 *
 * // Take the photo
 * PhotoMode->CapturePhoto();
 *
 * // Exit photo mode
 * PhotoMode->ExitPhotoMode();
 * @endcode
 *
 * =============================================================================
 * @file MGPhotoModeSubsystem.h
 * @brief Photo Mode Subsystem for in-game photography and image capture.
 *
 * Key Features:
 * - Multiple camera modes for flexible shot composition
 * - Real-time filter and post-processing adjustments
 * - Depth of field with adjustable focal distance and aperture
 * - Logo watermarks, frames, and informational overlays
 * - Photo gallery management with save/delete/share functionality
 * - High-resolution capture support up to 4K
 *
 * @see UWorldSubsystem
 * @see EMGPhotoCamera
 * @see EMGPhotoFilter
 * =============================================================================
 */

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

// ============================================================================
// DELEGATES - Event Notifications
// ============================================================================

/** @brief Broadcast when photo mode is entered. Use to pause gameplay, hide HUD, etc. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhotoModeEntered);

/** @brief Broadcast when photo mode is exited. Use to resume gameplay, restore HUD, etc. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhotoModeExited);

/** @brief Broadcast when a photo is successfully captured. Provides photo metadata. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhotoCaptured, const FMGPhotoInfo&, PhotoInfo);

/** @brief Broadcast when camera mode changes. Use to update UI hints for controls. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraModeChanged, EMGPhotoCamera, NewMode);

// ============================================================================
// PHOTO MODE SUBSYSTEM CLASS
// ============================================================================

/**
 * @class UMGPhotoModeSubsystem
 * @brief World subsystem that manages the in-game photo mode feature.
 *
 * This subsystem provides complete photo mode functionality including:
 * - **Camera Control**: Multiple modes (free, orbit, track, locked) with smooth movement
 * - **Visual Adjustments**: Filters, exposure, color grading, and effects
 * - **Depth of Field**: Adjustable focal distance and aperture for bokeh effects
 * - **Overlays**: Logos, frames, timestamps, and contextual information
 * - **Capture**: Standard and high-resolution photo capture with automatic saving
 * - **Gallery**: Browse, delete, and share saved photos
 *
 * The subsystem automatically handles scene pausing, HUD hiding, and camera
 * transitions when entering/exiting photo mode.
 *
 * @note This is a UWorldSubsystem, meaning one instance exists per game world.
 *       Access via GetWorld()->GetSubsystem<UMGPhotoModeSubsystem>().
 *
 * @see EMGPhotoCamera for camera mode options
 * @see EMGPhotoFilter for available filter presets
 * @see FMGPhotoInfo for saved photo metadata
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPhotoModeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/// @name Lifecycle
	/// @{

	/** Initialize the subsystem, set up render targets and materials. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Clean up resources when subsystem is destroyed. */
	virtual void Deinitialize() override;

	/** Per-frame update for camera movement and effect updates. */
	virtual void Tick(float MGDeltaTime) override;

	/** Always create this subsystem for worlds that support it. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	/// @}

	// ==========================================
	/// @name Events
	/// @brief Delegates for photo mode state changes and captures.
	/// Bind to these events to react to photo mode activities.
	// ==========================================
	/// @{

	/** Broadcast when entering photo mode. Pause gameplay systems here. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhotoModeEntered OnPhotoModeEntered;

	/** Broadcast when exiting photo mode. Resume gameplay systems here. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhotoModeExited OnPhotoModeExited;

	/** Broadcast when a photo is captured. Provides saved photo info. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhotoCaptured OnPhotoCaptured;

	/** Broadcast when camera mode changes (Free, Orbit, etc.). */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCameraModeChanged OnCameraModeChanged;

	/// @}

	// ==========================================
	/// @name Photo Mode Control
	/// @brief Core functions to enter, exit, and query photo mode state.
	// ==========================================
	/// @{

	/**
	 * @brief Enter photo mode and take control of the camera.
	 *
	 * Pauses the scene (if enabled), hides HUD, stores original camera state,
	 * and enables photo mode controls. Broadcasts OnPhotoModeEntered.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void EnterPhotoMode();

	/**
	 * @brief Exit photo mode and restore normal gameplay.
	 *
	 * Restores original camera position, unpauses scene, shows HUD,
	 * and returns input control. Broadcasts OnPhotoModeExited.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void ExitPhotoMode();

	/** @brief Check if photo mode is currently active. */
	UFUNCTION(BlueprintPure, Category = "PhotoMode")
	bool IsPhotoModeActive() const { return bIsActive; }

	/** @brief Toggle photo mode on/off based on current state. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void TogglePhotoMode();

	/// @}

	// ==========================================
	/// @name Camera Control
	/// @brief Functions for camera positioning, movement, and lens settings.
	/// Camera behavior varies based on the active EMGPhotoCamera mode.
	// ==========================================
	/// @{

	/**
	 * @brief Set the camera movement mode.
	 * @param Mode The camera mode to activate (Free, Orbit, Track, or Locked).
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetCameraMode(EMGPhotoCamera Mode);

	/** @brief Get the currently active camera mode. */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Camera")
	EMGPhotoCamera GetCameraMode() const { return CurrentCameraMode; }

	/**
	 * @brief Move camera by delta in world space. Only effective in Free mode.
	 * @param Delta Movement vector in world coordinates.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void MoveCamera(FVector Delta);

	/**
	 * @brief Rotate camera by delta. Works in all camera modes.
	 * @param Delta Rotation delta (pitch, yaw, roll).
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void RotateCamera(FRotator Delta);

	/**
	 * @brief Orbit camera around target. Only effective in Orbit mode.
	 * @param YawDelta Horizontal orbit angle change in degrees.
	 * @param PitchDelta Vertical orbit angle change in degrees.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void OrbitCamera(float YawDelta, float PitchDelta);

	/**
	 * @brief Adjust camera zoom/distance.
	 * @param Delta Positive zooms in, negative zooms out.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void ZoomCamera(float Delta);

	/** @brief Reset camera to default position relative to target vehicle. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void ResetCamera();

	/** @brief Get current camera settings structure. */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Camera")
	FMGPhotoCameraSettings GetCameraSettings() const { return CameraSettings; }

	/** @brief Apply a complete camera settings configuration. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetCameraSettings(const FMGPhotoCameraSettings& Settings);

	/**
	 * @brief Set camera field of view.
	 * @param FOV Field of view in degrees. Lower values zoom in, higher values show more.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetFieldOfView(float FOV);

	/**
	 * @brief Set depth of field focal distance.
	 * @param Distance Distance in centimeters where focus is sharpest.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetFocalDistance(float Distance);

	/**
	 * @brief Set camera aperture for depth of field.
	 * @param FStop Aperture value (f-stop). Lower values = more blur, shallower DOF.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetAperture(float FStop);

	/** @brief Enable or disable depth of field effect. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetDepthOfFieldEnabled(bool bEnabled);

	/**
	 * @brief Set camera roll angle for Dutch angle effects.
	 * @param Roll Roll angle in degrees (-90 to 90).
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Camera")
	void SetCameraRoll(float Roll);

	/// @}

	// ==========================================
	/// @name Visual Settings
	/// @brief Functions for filters, color grading, and post-processing effects.
	/// These settings modify the final captured image appearance.
	// ==========================================
	/// @{

	/**
	 * @brief Apply a filter preset to the view.
	 * @param Filter The filter preset to apply. Use Custom to keep manual settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetFilter(EMGPhotoFilter Filter);

	/** @brief Get the currently active filter preset. */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Visual")
	EMGPhotoFilter GetCurrentFilter() const { return VisualSettings.Filter; }

	/** @brief Get all current visual settings. */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Visual")
	FMGPhotoVisualSettings GetVisualSettings() const { return VisualSettings; }

	/** @brief Apply a complete visual settings configuration. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetVisualSettings(const FMGPhotoVisualSettings& Settings);

	/**
	 * @brief Adjust image exposure (brightness).
	 * @param Value Exposure compensation in EV (-3 to +3). 0 is neutral.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetExposure(float Value);

	/**
	 * @brief Adjust image contrast.
	 * @param Value Contrast multiplier (0.5 to 1.5). 1.0 is neutral.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetContrast(float Value);

	/**
	 * @brief Adjust color saturation.
	 * @param Value Saturation multiplier (0 to 2). 0 = grayscale, 1 = normal, 2 = vivid.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void SetSaturation(float Value);

	/** @brief Reset all visual settings to defaults (no filter, neutral values). */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Visual")
	void ResetVisualSettings();

	/// @}

	// ==========================================
	/// @name Overlays
	/// @brief Functions for watermarks, frames, and informational overlays.
	/// Overlays are rendered on top of the captured image.
	// ==========================================
	/// @{

	/** @brief Get current overlay configuration. */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Overlay")
	FMGPhotoOverlaySettings GetOverlaySettings() const { return OverlaySettings; }

	/** @brief Apply a complete overlay settings configuration. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Overlay")
	void SetOverlaySettings(const FMGPhotoOverlaySettings& Settings);

	/** @brief Toggle the game logo watermark on/off. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Overlay")
	void ToggleLogo();

	/** @brief Cycle through available decorative frame styles. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Overlay")
	void CycleFrameStyle();

	/// @}

	// ==========================================
	/// @name Scene Control
	/// @brief Functions for controlling scene state during photo mode.
	/// Allows pausing action and hiding elements for clean shots.
	// ==========================================
	/// @{

	/**
	 * @brief Pause or unpause the game scene.
	 * @param bPaused True to freeze all gameplay, false to allow motion.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Scene")
	void SetScenePaused(bool bPaused);

	/** @brief Check if the scene is currently paused/frozen. */
	UFUNCTION(BlueprintPure, Category = "PhotoMode|Scene")
	bool IsScenePaused() const { return bScenePaused; }

	/** @brief Toggle scene pause state. Useful for capturing motion. */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Scene")
	void ToggleScenePause();

	/**
	 * @brief Show or hide the player's vehicle in the shot.
	 * @param bHidden True to hide vehicle, false to show.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Scene")
	void SetVehicleHidden(bool bHidden);

	/**
	 * @brief Show or hide HUD elements during photo mode.
	 * @param bHidden True to hide HUD, false to show.
	 */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Scene")
	void SetHUDHidden(bool bHidden);

	/// @}

	// ==========================================
	// CAPTURE
	// ==========================================

	/** Take photo */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Capture")
	void CapturePhoto();

	/** Take high-res photo with default 4K resolution (3840x2160) */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Capture")
	void CaptureHighResPhoto(FIntPoint Resolution);

	/** Take high-res photo at 4K resolution (3840x2160) */
	UFUNCTION(BlueprintCallable, Category = "PhotoMode|Capture")
	void CaptureHighResPhoto4K() { CaptureHighResPhoto(FIntPoint(3840, 2160)); }

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
	void UpdateCamera(float MGDeltaTime);

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

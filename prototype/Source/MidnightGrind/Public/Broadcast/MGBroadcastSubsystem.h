// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGBroadcastSubsystem.h
 * @brief Broadcast output management for esports streaming and content creation.
 *
 * This subsystem manages the video output layer for professional broadcasts and
 * content creation. It handles camera presets, overlay graphics, picture-in-picture,
 * and output configuration for streaming or recording.
 *
 * Key Features:
 * - Pre-configured camera presets for common broadcast shots
 * - Overlay management with clean feed support
 * - Picture-in-picture for replays and secondary views
 * - Hotkey system for quick camera switching
 * - Output resolution and capture controls
 *
 * Typical Workflow:
 * 1. Call StartBroadcast() when going live
 * 2. Use SetCameraPreset() or SetCamera() to control shots
 * 3. Configure overlays with SetOverlay() or SetCleanFeed()
 * 4. Use PiP for replays and secondary content
 * 5. Call EndBroadcast() when finished
 *
 * Usage Example:
 * @code
 *     UMGBroadcastSubsystem* Broadcast = GetGameInstance()->GetSubsystem<UMGBroadcastSubsystem>();
 *     Broadcast->StartBroadcast("TournamentFinals_Match1");
 *     Broadcast->SetCameraPreset(EMGBroadcastCameraPreset::WideStart);
 *     // Race starts...
 *     Broadcast->SetCameraPreset(EMGBroadcastCameraPreset::HelicopterFollow);
 * @endcode
 *
 * @note This is a GameInstanceSubsystem - it persists across level transitions.
 * @see UMGCasterToolsSubsystem for detailed camera and overlay controls
 * @see UMGEsportsSubsystem for tournament management
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBroadcastSubsystem.generated.h"

/**
 * Broadcast System - For esports and content creation
 * - Multiple camera presets for cinematic shots
 * - Overlay management for stats/graphics
 * - Smooth transitions between cameras
 * - Picture-in-picture for replays
 * - Clean feed options (no UI) for production
 */

//=============================================================================
// Enums - Camera Presets
//=============================================================================

/**
 * @brief Pre-configured camera positions and behaviors for broadcast production.
 *
 * These presets provide quick access to common camera setups used in professional
 * race broadcasts. Each preset has optimized settings for its intended purpose.
 */
UENUM(BlueprintType)
enum class EMGBroadcastCameraPreset : uint8
{
	WideStart,        ///< Wide establishing shot of the starting grid
	HelicopterFollow, ///< Aerial camera following the pack from above
	TrackSideA,       ///< First trackside camera position
	TrackSideB,       ///< Second trackside camera position
	OnboardLeader,    ///< In-car view from the race leader
	OnboardBattle,    ///< In-car view of the most intense battle
	FinishLine,       ///< Fixed camera at the finish line
	PodiumCeremony,   ///< Post-race podium celebration view
	Replay,           ///< Camera used during replay playback
	Custom            ///< Custom user-defined camera position
};

//=============================================================================
// Structs - Camera Configuration
//=============================================================================

/**
 * @brief Complete configuration for a broadcast camera.
 *
 * Defines all parameters needed for a camera shot, including position,
 * target, field of view, and visual effects settings.
 */
USTRUCT(BlueprintType)
struct FMGBroadcastCamera
{
	GENERATED_BODY()

	/// Unique identifier for this camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CameraID;

	/// Which preset this camera is based on (Custom if manually configured)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBroadcastCameraPreset Preset = EMGBroadcastCameraPreset::Custom;

	/// World position and rotation of the camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CameraTransform;

	/// Player ID to track (empty for static cameras)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerID;

	/// Camera field of view in degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOV = 60.0f;

	/// Transition time when switching to this camera (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendTime = 1.0f;

	/// Enable depth of field effect (blurs background/foreground)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseDepthOfField = false;

	/// Enable motion blur effect during fast movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMotionBlur = true;
};

//=============================================================================
// Structs - Overlay Configuration
//=============================================================================

/**
 * @brief Configuration for broadcast overlay graphics.
 *
 * Controls which HUD elements are displayed on the broadcast output.
 * Individual elements can be toggled on/off for different broadcast needs.
 */
USTRUCT(BlueprintType)
struct FMGBroadcastOverlay
{
	GENERATED_BODY()

	/// Unique identifier for this overlay configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OverlayID;

	/// Show the race leaderboard/standings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowLeaderboard = true;

	/// Show race progress indicator (lap counter, distance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRaceProgress = true;

	/// Show driver statistics panel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDriverStats = true;

	/// Show track minimap
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowMinimap = true;

	/// Show race timer
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTimer = true;

	/// Show time gaps between racers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGapTimes = true;

	/// Show speedometer (typically for onboard cameras)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeedometer = false;

	/// Overall opacity for overlay elements (0.0 = invisible, 1.0 = solid)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverlayOpacity = 1.0f;
};

//=============================================================================
// Structs - Picture-in-Picture
//=============================================================================

/**
 * @brief Configuration for picture-in-picture overlay.
 *
 * PiP allows showing a secondary camera view or replay in a smaller window
 * overlaid on the main broadcast. Useful for showing replays without
 * completely cutting away from live action.
 */
USTRUCT(BlueprintType)
struct FMGPictureInPicture
{
	GENERATED_BODY()

	/// Whether PiP is currently visible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;

	/// Screen position (0,0 = top-left, 1,1 = bottom-right, normalized)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position = FVector2D(0.7f, 0.1f);

	/// Size as fraction of screen (0.25 = 25% of screen width/height)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Size = FVector2D(0.25f, 0.25f);

	/// Player ID to show in PiP (if not showing replay)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerID;

	/// Show replay content instead of live feed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowReplay = false;
};

//=============================================================================
// Delegates - Event Notifications
//=============================================================================

/// Broadcast when the camera changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBroadcastCameraChanged, const FMGBroadcastCamera&, Camera);

/// Broadcast when overlay settings change
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnOverlayChanged, const FMGBroadcastOverlay&, Overlay);

/// Broadcast when broadcast mode begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnBroadcastStarted);

/// Broadcast when broadcast mode ends
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnBroadcastEnded);

//=============================================================================
// UMGBroadcastSubsystem - Main Subsystem Class
//=============================================================================

/**
 * @brief Manages broadcast output for streaming and content creation.
 *
 * This GameInstanceSubsystem provides the output layer for professional broadcasts,
 * handling camera management, overlay configuration, and picture-in-picture
 * functionality.
 *
 * The broadcast subsystem works in conjunction with the caster tools subsystem,
 * which provides more granular control over camera behavior and data collection.
 * This subsystem focuses on the final output composition.
 *
 * Key responsibilities:
 * - Camera preset management and switching
 * - Overlay visibility and configuration
 * - Picture-in-picture composition
 * - Clean feed output (no UI) for production
 * - Output resolution and capture controls
 *
 * @note Access via: GetGameInstance()->GetSubsystem<UMGBroadcastSubsystem>()
 */
UCLASS()
class MIDNIGHTGRIND_API UMGBroadcastSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Lifecycle
	//-------------------------------------------------------------------------

	/** Initialize the broadcast subsystem and set up camera presets. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	//-------------------------------------------------------------------------
	// Broadcast Mode Control
	//-------------------------------------------------------------------------

	/**
	 * @brief Start broadcast mode for a session.
	 * @param SessionID Unique identifier for this broadcast session (for logging/recording).
	 * @note Fires OnBroadcastStarted when complete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void StartBroadcast(const FString& SessionID);

	/**
	 * @brief End the current broadcast session.
	 * @note Fires OnBroadcastEnded when complete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void EndBroadcast();

	/** @return True if currently in broadcast mode. */
	UFUNCTION(BlueprintPure, Category = "Broadcast")
	bool IsBroadcasting() const { return bIsBroadcasting; }

	//-------------------------------------------------------------------------
	// Camera Control
	//-------------------------------------------------------------------------

	/**
	 * @brief Set the broadcast camera with full configuration.
	 * @param Camera Complete camera configuration to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void SetCamera(const FMGBroadcastCamera& Camera);

	/**
	 * @brief Switch to a predefined camera preset.
	 * @param Preset The camera preset to activate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void SetCameraPreset(EMGBroadcastCameraPreset Preset);

	/**
	 * @brief Cycle through available camera presets.
	 * @note Wraps around from the last preset to the first.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void CycleCamera();

	/** @return The current camera configuration. */
	UFUNCTION(BlueprintPure, Category = "Broadcast|Camera")
	FMGBroadcastCamera GetCurrentCamera() const { return CurrentCamera; }

	/**
	 * @brief Set the player target for following cameras.
	 * @param PlayerID Player to track with the camera.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void SetCameraTarget(const FString& PlayerID);

	/**
	 * @brief Set the transition time for camera switches.
	 * @param Seconds Duration of the blend between cameras.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Camera")
	void SetBlendTime(float Seconds);

	//-------------------------------------------------------------------------
	// Overlay Control
	//-------------------------------------------------------------------------

	/**
	 * @brief Apply a complete overlay configuration.
	 * @param Overlay The overlay settings to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Overlay")
	void SetOverlay(const FMGBroadcastOverlay& Overlay);

	/**
	 * @brief Toggle clean feed mode (no overlay graphics).
	 * @param bClean True for clean feed (no UI), false for normal overlay.
	 * @note Clean feed is used when external graphics systems handle overlays.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Overlay")
	void SetCleanFeed(bool bClean);

	/** @return True if currently outputting a clean feed. */
	UFUNCTION(BlueprintPure, Category = "Broadcast|Overlay")
	bool IsCleanFeed() const { return bCleanFeed; }

	/** @return The current overlay configuration. */
	UFUNCTION(BlueprintPure, Category = "Broadcast|Overlay")
	FMGBroadcastOverlay GetCurrentOverlay() const { return CurrentOverlay; }

	//-------------------------------------------------------------------------
	// Picture-in-Picture
	//-------------------------------------------------------------------------

	/**
	 * @brief Enable picture-in-picture with the specified settings.
	 * @param Settings PiP configuration including position, size, and content.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|PiP")
	void EnablePictureInPicture(const FMGPictureInPicture& Settings);

	/** @brief Disable the picture-in-picture overlay. */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|PiP")
	void DisablePictureInPicture();

	/**
	 * @brief Show a replay in the PiP window for a duration.
	 * @param Duration How long to show the replay (seconds).
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|PiP")
	void ShowReplayInPiP(float Duration);

	/** @return Current PiP configuration. */
	UFUNCTION(BlueprintPure, Category = "Broadcast|PiP")
	FMGPictureInPicture GetPictureInPictureSettings() const { return PiPSettings; }

	//-------------------------------------------------------------------------
	// Hotkeys and Shortcuts
	//-------------------------------------------------------------------------

	/**
	 * @brief Register a camera to a numeric hotkey (1-9).
	 * @param HotkeyIndex The hotkey number (1-9).
	 * @param Camera The camera configuration to assign.
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void RegisterCameraHotkey(int32 HotkeyIndex, const FMGBroadcastCamera& Camera);

	/**
	 * @brief Activate a camera via its registered hotkey.
	 * @param HotkeyIndex The hotkey number (1-9).
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast")
	void TriggerCameraHotkey(int32 HotkeyIndex);

	//-------------------------------------------------------------------------
	// Output Configuration
	//-------------------------------------------------------------------------

	/**
	 * @brief Set the broadcast output resolution.
	 * @param Width Output width in pixels.
	 * @param Height Output height in pixels.
	 * @note Common values: 1920x1080 (1080p), 3840x2160 (4K).
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Output")
	void SetOutputResolution(int32 Width, int32 Height);

	/**
	 * @brief Capture a screenshot of the current broadcast view.
	 * @param Filename Path to save the screenshot (without extension).
	 */
	UFUNCTION(BlueprintCallable, Category = "Broadcast|Output")
	void CaptureScreenshot(const FString& Filename);

	//-------------------------------------------------------------------------
	// Events - Bindable Delegates
	//-------------------------------------------------------------------------

	/// Fired when the broadcast camera changes
	UPROPERTY(BlueprintAssignable, Category = "Broadcast|Events")
	FMGOnBroadcastCameraChanged OnBroadcastCameraChanged;

	/// Fired when overlay settings change
	UPROPERTY(BlueprintAssignable, Category = "Broadcast|Events")
	FMGOnOverlayChanged OnOverlayChanged;

	/// Fired when broadcast mode starts
	UPROPERTY(BlueprintAssignable, Category = "Broadcast|Events")
	FMGOnBroadcastStarted OnBroadcastStarted;

	/// Fired when broadcast mode ends
	UPROPERTY(BlueprintAssignable, Category = "Broadcast|Events")
	FMGOnBroadcastEnded OnBroadcastEnded;

protected:
	//-------------------------------------------------------------------------
	// Internal Methods
	//-------------------------------------------------------------------------

	/** Set up default camera presets based on track data. */
	void InitializeCameraPresets();

	/**
	 * @brief Get the camera configuration for a preset.
	 * @param Preset The preset to retrieve.
	 * @return Camera configuration for the preset.
	 */
	FMGBroadcastCamera GetPresetCamera(EMGBroadcastCameraPreset Preset) const;

private:
	//-------------------------------------------------------------------------
	// Camera State
	//-------------------------------------------------------------------------

	/// Current active camera configuration
	FMGBroadcastCamera CurrentCamera;

	/// Current overlay configuration
	FMGBroadcastOverlay CurrentOverlay;

	/// Current PiP configuration
	FMGPictureInPicture PiPSettings;

	/// Map of hotkey indices to camera configurations
	TMap<int32, FMGBroadcastCamera> CameraHotkeys;

	/// Map of presets to their camera configurations
	TMap<EMGBroadcastCameraPreset, FMGBroadcastCamera> PresetCameras;

	//-------------------------------------------------------------------------
	// Broadcast State
	//-------------------------------------------------------------------------

	/// Current broadcast session identifier
	FString CurrentSessionID;

	/// Whether broadcast mode is active
	bool bIsBroadcasting = false;

	/// Whether outputting clean feed (no overlay)
	bool bCleanFeed = false;

	/// Index for cycling through presets
	int32 CurrentPresetIndex = 0;
};

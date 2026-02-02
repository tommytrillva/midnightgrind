// Copyright Midnight Grind. All Rights Reserved.

#pragma once
/**
 * @file MGReplayDataAssets.h
 * @brief Data Assets for replay system configuration
 *
 * This file contains Data Asset classes that store configuration settings for the
 * replay and ghost racing systems. Data Assets allow designers to create and tweak
 * these settings in the Editor without code changes.
 *
 * DATA ASSETS EXPLAINED (For Entry-Level Developers):
 * -----------------------------------------------
 * Data Assets are Blueprint-editable objects that store configuration data.
 * They're created in the Content Browser and can be referenced by code.
 * This allows designers to:
 * - Tweak ghost visuals without programmer help
 * - Create multiple presets (racing, drift, photo mode)
 * - A/B test different settings easily
 *
 * ASSETS IN THIS FILE:
 * - UMGGhostSettingsAsset: Visual and behavioral settings for ghost racers
 * - UMGReplaySettingsAsset: Recording and playback configuration
 * - UMGReplayCameraAsset: Camera presets for replay viewer
 *
 * USAGE:
 * 1. Create a Data Asset in Content Browser (Right-click > Miscellaneous > Data Asset)
 * 2. Select the appropriate class (e.g., MGGhostSettingsAsset)
 * 3. Configure the settings in the Details panel
 * 4. Reference the asset in your subsystem or Blueprint
 *
 * @see UMGReplaySubsystem - Uses these assets for replay functionality
 * @see AMGGhostRacerActor - Uses ghost settings for visual appearance
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

// EMGGhostType - REMOVED (duplicate)
// Canonical definition in: Core/MGSharedTypes.h

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "Core/MGSharedTypes.h"
#include "MGReplayDataAssets.generated.h"

/**
 * Ghost visual preset
 *
 * Defines the visual appearance of a ghost racer. Different presets
 * can be used for different ghost types (personal best, friend, world record).
 */
USTRUCT(BlueprintType)
struct FMGGhostVisualPreset
{
	GENERATED_BODY()

	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Ghost color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GhostColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** Base transparency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Transparency = 0.5f;

	/** Outline color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OutlineColor = FLinearColor::White;

	/** Outline width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutlineWidth = 2.0f;

	/** Enable glow effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableGlow = true;

	/** Glow intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlowIntensity = 1.0f;

	/** Trail effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* TrailEffect;
};

/**
 * Ghost Settings Data Asset
 *
 * Centralizes all visual and behavioral settings for ghost racers.
 * Create instances of this asset to define different ghost configurations.
 *
 * TYPICAL SETUP:
 * 1. Create one "default" ghost settings asset
 * 2. Reference it in your game's settings/config
 * 3. Optionally create variants for different game modes
 *
 * @see AMGGhostRacerActor - Applies these settings
 * @see UMGReplaySubsystem - Spawns ghosts using these settings
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGGhostSettingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// VISUAL PRESETS
	// ==========================================

	/** Visual preset for personal best ghost (typically green/gold) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presets")
	FMGGhostVisualPreset PersonalBestPreset;

	/** Visual preset for friend ghost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presets")
	FMGGhostVisualPreset FriendPreset;

	/** Visual preset for world record ghost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presets")
	FMGGhostVisualPreset WorldRecordPreset;

	/** Visual preset for developer ghost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presets")
	FMGGhostVisualPreset DeveloperPreset;

	// ==========================================
	// MATERIALS
	// ==========================================

	/** Ghost material (translucent) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
	UMaterialInterface* GhostMaterial;

	/** Ghost outline material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
	UMaterialInterface* OutlineMaterial;

	// ==========================================
	// BEHAVIOR
	// ==========================================

	/** Enable ghost collision for drafting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bEnableDrafting = false;

	/** Ghost fade-in distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	float FadeInDistance = 100.0f;

	/** Ghost fade-out distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	float FadeOutDistance = 500.0f;

	/** Maximum ghosts visible at once */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	int32 MaxVisibleGhosts = 3;

	// ==========================================
	// DELTA DISPLAY
	// ==========================================

	/** Show delta time to ghost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delta")
	bool bShowDelta = true;

	/** Delta color when ahead */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delta")
	FLinearColor DeltaAheadColor = FLinearColor::Green;

	/** Delta color when behind */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delta")
	FLinearColor DeltaBehindColor = FLinearColor::Red;

	/** Delta update frequency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Delta")
	float DeltaUpdateRate = 0.1f;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	/** Get preset for ghost type */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	FMGGhostVisualPreset GetPresetForType(EMGGhostType GhostType) const;
};

/**
 * Replay Settings Data Asset
 *
 * Configures replay recording and playback behavior. This includes
 * frame rates, storage limits, compression, and camera options.
 *
 * PERFORMANCE NOTES:
 * - Higher RecordingFPS = smoother replays but larger files
 * - 30fps is usually sufficient for racing games
 * - Enable compression for network sharing (slight CPU cost)
 *
 * STORAGE NOTES:
 * - Uncompressed 30fps replay: ~1MB per minute
 * - Compressed: ~200KB per minute
 * - Set reasonable limits to prevent disk bloat
 *
 * @see UMGReplaySubsystem - Uses these settings for recording
 * @see UMGReplayRecordingComponent - Per-vehicle recording
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGReplaySettingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// RECORDING
	// ==========================================

	/** Recording frame rate (30 recommended, 60 for high-fidelity) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recording", meta = (ClampMin = "15.0", ClampMax = "60.0"))
	float RecordingFPS = 30.0f;

	/** Maximum recording duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recording")
	float MaxRecordingDuration = 600.0f;

	/** Auto-save personal best replays */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recording")
	bool bAutoSavePersonalBest = true;

	/** Upload personal best to server */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recording")
	bool bUploadPersonalBest = true;

	// ==========================================
	// PLAYBACK
	// ==========================================

	/** Available playback speeds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	TArray<float> PlaybackSpeeds;

	/** Default slow motion speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	float SlowMotionSpeed = 0.25f;

	/** Default fast forward speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	float FastForwardSpeed = 2.0f;

	/** Enable smooth interpolation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	bool bEnableInterpolation = true;

	/** Interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Playback")
	float InterpolationSpeed = 10.0f;

	// ==========================================
	// CAMERA
	// ==========================================

	/** Enable free camera in replay */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bEnableFreeCamera = true;

	/** Enable TV-style camera */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bEnableTVCamera = true;

	/** Free camera movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float FreeCameraSpeed = 1000.0f;

	/** Free camera rotation speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float FreeCameraRotationSpeed = 100.0f;

	// ==========================================
	// STORAGE
	// ==========================================

	/** Maximum saved replays per track */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage")
	int32 MaxSavedReplaysPerTrack = 10;

	/** Total maximum saved replays */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage")
	int32 MaxTotalSavedReplays = 50;

	/** Enable replay compression */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage")
	bool bEnableCompression = true;

	/** Compression level (1-9) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Storage", meta = (ClampMin = "1", ClampMax = "9"))
	int32 CompressionLevel = 6;

	UMGReplaySettingsAsset()
	{
		// Default playback speeds
		PlaybackSpeeds.Add(0.25f);
		PlaybackSpeeds.Add(0.5f);
		PlaybackSpeeds.Add(1.0f);
		PlaybackSpeeds.Add(2.0f);
		PlaybackSpeeds.Add(4.0f);
	}
};

/**
 * Replay camera preset
 */
USTRUCT(BlueprintType)
struct FMGReplayCameraPreset
{
	GENERATED_BODY()

	/** Preset identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Camera offset from vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Offset = FVector(-500.0f, 0.0f, 200.0f);

	/** Look-at offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LookAtOffset = FVector(200.0f, 0.0f, 50.0f);

	/** Field of view */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOV = 90.0f;

	/** Camera lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LagSpeed = 10.0f;

	/** Is this a fixed position camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFixedPosition = false;

	/** Enable motion blur */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableMotionBlur = true;

	/** Motion blur intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MotionBlurIntensity = 0.5f;
};

/**
 * Replay Camera Settings Data Asset
 *
 * Defines camera presets for the replay viewer. Each preset specifies
 * camera position, angle, FOV, and post-processing effects.
 *
 * CAMERA TIPS FOR DESIGNERS:
 * - Chase cam: Classic third-person view, good default
 * - Hood cam: Immersive, shows speed well
 * - Cockpit: Most immersive but may cause motion sickness
 * - Wheel cam: Dramatic low angle, great for replays
 * - TV cam: Fixed trackside cameras, broadcast feel
 *
 * AUTO-CUT (Director Mode):
 * When enabled, the system automatically switches between cameras
 * based on action (corners, overtakes, etc.). Tune MinCutTime and
 * MaxCutTime for desired pacing.
 *
 * @see UMGReplaySubsystem - Uses these presets during playback
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGReplayCameraAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Available camera presets (cycled with camera button) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cameras")
	TArray<FMGReplayCameraPreset> CameraPresets;

	/** Default camera preset ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cameras")
	FName DefaultPresetID;

	/** Enable auto-cut between cameras */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Director")
	bool bEnableAutoCut = true;

	/** Minimum time before auto-cut (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Director")
	float MinCutTime = 3.0f;

	/** Maximum time before auto-cut (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Director")
	float MaxCutTime = 10.0f;

	/** Get camera preset by ID */
	UFUNCTION(BlueprintPure, Category = "Camera")
	FMGReplayCameraPreset GetPreset(FName PresetID) const;

	UMGReplayCameraAsset()
	{
		// Default camera presets
		FMGReplayCameraPreset ChasePreset;
		ChasePreset.PresetID = FName("Chase");
		ChasePreset.DisplayName = FText::FromString("Chase Cam");
		ChasePreset.Offset = FVector(-500.0f, 0.0f, 200.0f);
		CameraPresets.Add(ChasePreset);

		FMGReplayCameraPreset HoodPreset;
		HoodPreset.PresetID = FName("Hood");
		HoodPreset.DisplayName = FText::FromString("Hood Cam");
		HoodPreset.Offset = FVector(100.0f, 0.0f, 80.0f);
		HoodPreset.FOV = 100.0f;
		CameraPresets.Add(HoodPreset);

		FMGReplayCameraPreset CockpitPreset;
		CockpitPreset.PresetID = FName("Cockpit");
		CockpitPreset.DisplayName = FText::FromString("Cockpit Cam");
		CockpitPreset.Offset = FVector(0.0f, 0.0f, 60.0f);
		CockpitPreset.FOV = 90.0f;
		CameraPresets.Add(CockpitPreset);

		FMGReplayCameraPreset WheelPreset;
		WheelPreset.PresetID = FName("Wheel");
		WheelPreset.DisplayName = FText::FromString("Wheel Cam");
		WheelPreset.Offset = FVector(-100.0f, -100.0f, 30.0f);
		WheelPreset.FOV = 80.0f;
		CameraPresets.Add(WheelPreset);

		DefaultPresetID = FName("Chase");
	}
};

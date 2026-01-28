// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGReplayDataAssets.h
 * @brief Data Assets for configuring replay system visuals and behavior
 *
 * @section overview Overview
 * This file contains Unreal Engine Data Assets that store configuration settings
 * for the replay and ghost racing systems. Data Assets are designer-friendly
 * configuration objects that can be created and edited in the Unreal Editor
 * without requiring code changes.
 *
 * The file includes:
 * - UMGGhostSettingsAsset: Visual presets and behavior settings for ghost racers
 * - UMGReplaySettingsAsset: Recording and playback configuration
 * - UMGReplayCameraAsset: Camera presets for replay viewing
 *
 * @section concepts Key Concepts for Beginners
 *
 * **What are Data Assets?**
 * Data Assets are Unreal Engine objects that store configurable data separately
 * from code. They allow designers to tweak values (colors, speeds, limits) in
 * the Editor without programmer involvement. Changes to Data Assets don't require
 * recompilation, making iteration faster.
 *
 * **Ghost Visual Presets**
 * Different ghost types (personal best, friend, world record) use distinct visual
 * styles to help players identify them at a glance. Presets define colors,
 * transparency, glow effects, and particle trails for each ghost type.
 *
 * **Recording Settings**
 * Replay recording captures vehicle state at a fixed frame rate (e.g., 30 FPS).
 * Higher frame rates produce smoother replays but larger file sizes. Settings
 * control recording duration limits, compression, and auto-save behavior.
 *
 * **Camera Presets**
 * Replay viewers can switch between camera angles (chase cam, hood cam, cockpit).
 * Each preset defines camera offset, field of view, motion blur, and lag settings.
 * The "Director" mode can automatically cut between cameras for cinematic replays.
 *
 * @section usage Usage Examples
 *
 * **Creating Ghost Settings in Editor:**
 * 1. Right-click in Content Browser
 * 2. Select Miscellaneous -> Data Asset
 * 3. Choose MGGhostSettingsAsset
 * 4. Configure visual presets for each ghost type
 *
 * **Loading and Using Ghost Settings:**
 * @code
 * // Load the ghost settings asset
 * UMGGhostSettingsAsset* GhostSettings = LoadObject<UMGGhostSettingsAsset>(
 *     nullptr, TEXT("/Game/Data/GhostSettings"));
 *
 * // Get the preset for a world record ghost
 * FMGGhostVisualPreset Preset = GhostSettings->GetPresetForType(EMGGhostType::WorldRecord);
 *
 * // Apply preset to a ghost actor
 * Ghost->SetGhostColor(Preset.GhostColor);
 * Ghost->SetTransparency(Preset.Transparency);
 * @endcode
 *
 * **Configuring Replay Recording:**
 * @code
 * // Access replay settings
 * UMGReplaySettingsAsset* ReplaySettings = GetReplaySettings();
 *
 * // Check recording constraints
 * float MaxDuration = ReplaySettings->MaxRecordingDuration;
 * float TargetFPS = ReplaySettings->RecordingFPS;
 *
 * // Configure compression
 * if (ReplaySettings->bEnableCompression)
 * {
 *     int32 Level = ReplaySettings->CompressionLevel;
 *     // Apply compression with specified level
 * }
 * @endcode
 *
 * **Using Camera Presets:**
 * @code
 * // Load camera settings asset
 * UMGReplayCameraAsset* CameraSettings = GetCameraSettings();
 *
 * // Get the chase camera preset
 * FMGReplayCameraPreset ChasePreset = CameraSettings->GetPreset(FName("Chase"));
 *
 * // Apply to replay camera
 * ReplayCamera->SetRelativeLocation(ChasePreset.Offset);
 * ReplayCamera->FieldOfView = ChasePreset.FOV;
 * @endcode
 *
 * @section bestpractices Best Practices
 * - Create separate Data Assets for different game modes (arcade vs simulation)
 * - Use distinct colors for each ghost type for quick visual identification
 * - Keep recording FPS at 30 for good balance of quality and file size
 * - Test camera presets in-game before finalizing
 * - Document any project-specific preset conventions
 *
 * @see AMGGhostRacerActor
 * @see UMGReplaySubsystem
 * @see EMGGhostType
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGReplayDataAssets.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UMaterialInterface;
class USoundBase;
class UNiagaraSystem;

/**
 * Ghost type for different sources
 */
UENUM(BlueprintType)
enum class EMGGhostType : uint8
{
	/** Player's personal best */
	PersonalBest,
	/** Friend's best time */
	Friend,
	/** World record holder */
	WorldRecord,
	/** Developer ghost */
	Developer,
	/** Custom/downloaded ghost */
	Custom
};

/**
 * Ghost visual preset
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
 * Ghost settings asset
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGGhostSettingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// VISUAL PRESETS
	// ==========================================

	/** Visual preset for personal best ghost */
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
 * Replay settings asset
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGReplaySettingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// RECORDING
	// ==========================================

	/** Recording frame rate */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recording")
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
 * Replay camera settings asset
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGReplayCameraAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Camera presets */
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

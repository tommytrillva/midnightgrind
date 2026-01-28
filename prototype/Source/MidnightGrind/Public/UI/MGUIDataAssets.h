// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * @file MGUIDataAssets.h
 * @brief Data assets for UI theming, HUD layouts, and track minimap configuration
 *
 * =============================================================================
 * @section Overview
 * This file defines data asset classes that allow designers to configure UI
 * appearance without modifying code. These assets store visual themes, HUD
 * element layouts, and track-specific minimap data that can be created and
 * edited in the Unreal Editor.
 *
 * Data assets defined here:
 * - **UMGHUDThemeData**: Complete visual theme including colors, textures, sounds
 * - **UMGHUDLayoutPresetData**: Quick layout presets for HUD element positioning
 * - **UMGTrackMinimapData**: Track-specific minimap textures and bounds
 * - **UMGUIConfigData**: Master configuration referencing themes and presets
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Data Assets vs Blueprints**: Data Assets are pure data containers (no logic),
 *   making them ideal for configuration. They load faster than Blueprints and are
 *   easier for non-programmers to edit.
 *
 * - **Theme System**: The HUD theme system allows complete visual customization
 *   through data. Players could unlock or purchase different HUD themes, or
 *   designers can create era-specific themes (90s arcade, modern minimal, etc.).
 *
 * - **Layout Presets**: Predefined HUD element positions that players can switch
 *   between. Useful for different screen sizes, preferences, or streaming setups.
 *
 * - **Anchor System**: HUD elements use a 9-point anchor system (TopLeft to
 *   BottomRight) plus offset values, making layouts resolution-independent.
 *
 * - **Track Minimap Data**: Each track has associated minimap configuration
 *   including the texture, world-to-minimap coordinate bounds, checkpoint
 *   positions, and optimal racing line for display.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [UMGUIConfigData] (Master Config)
 *          |
 *          +-- AvailableThemes[] --> [UMGHUDThemeData]
 *          |                              |
 *          |                              +-- TachometerStyle
 *          |                              +-- MinimapStyle
 *          |                              +-- NotificationStyle
 *          |                              +-- Colors
 *          |                              +-- Sounds
 *          |
 *          +-- LayoutPresets[] --> [UMGHUDLayoutPresetData]
 *          |                              |
 *          |                              +-- ElementLayouts[]
 *          |
 *          +-- Widget Classes
 *          |
 *          v
 *   [Race HUD Subsystem] applies theme/layout to widgets
 *
 *   [UMGTrackMinimapData] (Per-Track)
 *          |
 *          +-- MinimapTexture
 *          +-- TrackBounds (world space)
 *          +-- CheckpointPositions[]
 *          +-- RacingLinePoints[]
 *          |
 *          v
 *   [UMGMinimapWidget] uses this data for rendering
 *
 * =============================================================================
 * @section Usage
 *
 * @subsection CreatingTheme Creating a HUD Theme
 * @code
 * // In Editor: Content Browser -> Right Click -> Miscellaneous -> Data Asset
 * // Select UMGHUDThemeData, name it "DA_HUDTheme_Retro"
 *
 * // In C++, load and apply a theme:
 * UMGHUDThemeData* RetroTheme = LoadObject<UMGHUDThemeData>(nullptr,
 *     TEXT("/Game/UI/Themes/DA_HUDTheme_Retro"));
 *
 * // Apply to HUD subsystem
 * HUDSubsystem->ApplyTheme(RetroTheme);
 *
 * // Or directly configure widgets:
 * TachWidget->SetStyle(RetroTheme->TachometerStyle);
 * MinimapWidget->SetStyle(RetroTheme->MinimapStyle);
 * @endcode
 *
 * @subsection UsingLayoutPresets Using Layout Presets
 * @code
 * // Get available presets
 * TArray<UMGHUDLayoutPresetData*> Presets = UIConfig->LayoutPresets;
 *
 * // Apply a preset
 * for (const FMGHUDElementLayout& Layout : SelectedPreset->ElementLayouts)
 * {
 *     Widget->SetElementPosition(Layout.ElementName, Layout.Anchor, Layout.Offset);
 *     Widget->SetElementScale(Layout.ElementName, Layout.Scale);
 *     Widget->SetElementVisible(Layout.ElementName, Layout.bVisibleByDefault);
 * }
 * @endcode
 *
 * @subsection TrackMinimapSetup Setting Up Track Minimap Data
 * @code
 * // Create minimap data asset for a track
 * // In Editor: Create UMGTrackMinimapData asset
 *
 * // Configure in Details panel:
 * // - Assign minimap texture (top-down track render)
 * // - Set TrackBoundsMin/Max to match world coordinates
 * // - Add checkpoint positions
 * // - Optionally add racing line points
 *
 * // In code, apply to minimap widget:
 * UMGTrackMinimapData* TrackData = GetMinimapDataForTrack(TrackName);
 * TrackData->ApplyToMinimap(MinimapWidget);
 *
 * // This sets:
 * // - Texture
 * // - World bounds for coordinate conversion
 * // - Checkpoint markers
 * // - Racing line (if enabled)
 * @endcode
 *
 * =============================================================================
 * @section UnrealMacros Unreal Engine Macros Explained
 *
 * - UCLASS(BlueprintType): Allows this class to be used as a variable type in
 *   Blueprints. Combined with UDataAsset base, creates editable data containers.
 *
 * - USTRUCT(BlueprintType): Structs that can be used in Blueprints as variables,
 *   function parameters, and in data tables.
 *
 * - UPROPERTY(EditAnywhere): Property can be edited in any property window
 *   (Details panel in Editor, archetype properties, instance properties).
 *
 * - UDataAsset base class: Provides automatic serialization and Editor support
 *   for creating data-only assets without Blueprint overhead.
 *
 * - TSubclassOf<UWidget>: A type-safe reference to a UClass that must derive
 *   from UWidget. Used to specify which widget class to instantiate at runtime.
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "UI/MGRaceHUDWidget.h"
#include "UI/MGMinimapWidget.h"
#include "UI/MGRaceOverlayWidget.h"
#include "MGUIDataAssets.generated.h"

class UTexture2D;
class USoundBase;
class UUserWidget;

/**
 * HUD element position anchor
 */
UENUM(BlueprintType)
enum class EMGHUDAnchor : uint8
{
	TopLeft,
	TopCenter,
	TopRight,
	MiddleLeft,
	MiddleCenter,
	MiddleRight,
	BottomLeft,
	BottomCenter,
	BottomRight
};

/**
 * HUD element layout configuration
 */
USTRUCT(BlueprintType)
struct FMGHUDElementLayout
{
	GENERATED_BODY()

	/** Element name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ElementName;

	/** Anchor position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHUDAnchor Anchor = EMGHUDAnchor::BottomRight;

	/** Offset from anchor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Offset = FVector2D::ZeroVector;

	/** Element scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.0f;

	/** Is visible by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisibleByDefault = true;
};

/**
 * Speedometer/Tachometer visual style
 */
USTRUCT(BlueprintType)
struct FMGTachometerStyle
{
	GENERATED_BODY()

	/** Style name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StyleName;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Tachometer type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTachStyle TachType = EMGTachStyle::Arc;

	/** Background image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* BackgroundTexture = nullptr;

	/** Needle/indicator image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* NeedleTexture = nullptr;

	/** Tachometer color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TachColor = FLinearColor::White;

	/** Redline color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RedlineColor = FLinearColor::Red;

	/** Speed text color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SpeedTextColor = FLinearColor::White;

	/** Gear text color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GearTextColor = FLinearColor::White;

	/** NOS gauge color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor NOSColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** Show shift indicator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowShiftIndicator = true;

	/** Shift indicator threshold (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShiftIndicatorThreshold = 0.9f;
};

/**
 * Minimap visual style
 */
USTRUCT(BlueprintType)
struct FMGMinimapStyle
{
	GENERATED_BODY()

	/** Style name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StyleName;

	/** Minimap shape (circular, square, rounded) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Shape = FName("Circular");

	/** Border texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* BorderTexture = nullptr;

	/** Player marker texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* PlayerMarkerTexture = nullptr;

	/** Opponent marker texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* OpponentMarkerTexture = nullptr;

	/** Player marker color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PlayerMarkerColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	/** Opponent marker color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor OpponentMarkerColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** Checkpoint marker color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CheckpointColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	/** Default zoom level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultZoom = 2.0f;

	/** Default mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMinimapMode DefaultMode = EMGMinimapMode::RotatingMap;
};

/**
 * Notification visual style
 */
USTRUCT(BlueprintType)
struct FMGNotificationStyle
{
	GENERATED_BODY()

	/** Background texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* BackgroundTexture = nullptr;

	/** Position gain color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PositionGainColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	/** Position loss color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PositionLossColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** Best lap color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BestLapColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);

	/** Near miss color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor NearMissColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	/** Drift score color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DriftScoreColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** Animation in duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimInDuration = 0.2f;

	/** Animation out duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimOutDuration = 0.3f;
};

/**
 * HUD Theme Data Asset
 * Complete visual theme for the racing HUD
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGHUDThemeData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Theme name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FName ThemeName;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme", meta = (MultiLine = true))
	FText Description;

	/** Preview image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	UTexture2D* PreviewImage = nullptr;

	// ==========================================
	// STYLES
	// ==========================================

	/** Tachometer style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Styles")
	FMGTachometerStyle TachometerStyle;

	/** Minimap style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Styles")
	FMGMinimapStyle MinimapStyle;

	/** Notification style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Styles")
	FMGNotificationStyle NotificationStyle;

	// ==========================================
	// LAYOUT
	// ==========================================

	/** Element layouts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Layout")
	TArray<FMGHUDElementLayout> ElementLayouts;

	// ==========================================
	// COLORS
	// ==========================================

	/** Primary accent color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor PrimaryColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** Secondary accent color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor SecondaryColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);

	/** Background color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.7f);

	/** Text color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Colors")
	FLinearColor TextColor = FLinearColor::White;

	// ==========================================
	// SOUNDS
	// ==========================================

	/** Position gain sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Sounds")
	USoundBase* PositionGainSound = nullptr;

	/** Position loss sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Sounds")
	USoundBase* PositionLossSound = nullptr;

	/** Best lap sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Sounds")
	USoundBase* BestLapSound = nullptr;

	/** Final lap sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Sounds")
	USoundBase* FinalLapSound = nullptr;

	/** Countdown tick sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Sounds")
	USoundBase* CountdownTickSound = nullptr;

	/** Countdown go sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Sounds")
	USoundBase* CountdownGoSound = nullptr;

	/** Shift indicator sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme|Sounds")
	USoundBase* ShiftIndicatorSound = nullptr;

	// ==========================================
	// HELPERS
	// ==========================================

	/** Get layout for element */
	UFUNCTION(BlueprintPure, Category = "Theme")
	bool GetElementLayout(FName ElementName, FMGHUDElementLayout& OutLayout) const;
};

/**
 * HUD Layout Preset Data Asset
 * Quick layout presets without full theme
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGHUDLayoutPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FName PresetName;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FText DisplayName;

	/** Element layouts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FMGHUDElementLayout> ElementLayouts;

	/** Default HUD mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	EMGHUDMode DefaultMode = EMGHUDMode::Full;

	/** Global scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float GlobalScale = 1.0f;

	/** Global opacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float GlobalOpacity = 1.0f;
};

/**
 * Track Minimap Data Asset
 * Pre-configured minimap data for a track
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackMinimapData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Track name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FName TrackName;

	/** Minimap texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	UTexture2D* MinimapTexture = nullptr;

	/** Track world bounds min (X, Y) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FVector2D TrackBoundsMin = FVector2D(-10000.0f, -10000.0f);

	/** Track world bounds max (X, Y) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FVector2D TrackBoundsMax = FVector2D(10000.0f, 10000.0f);

	/** Track rotation offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float RotationOffset = 0.0f;

	/** Checkpoint positions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TArray<FVector> CheckpointPositions;

	/** Finish line position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FVector FinishLinePosition = FVector::ZeroVector;

	/** Finish line rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float FinishLineRotation = 0.0f;

	/** Optimal racing line points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TArray<FVector> RacingLinePoints;

	/** Suggested minimap zoom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float SuggestedZoom = 2.0f;

	/** Apply to minimap widget */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void ApplyToMinimap(UMGMinimapWidget* Minimap) const;
};

/**
 * Master UI Config Data Asset
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGUIConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Config name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName ConfigName;

	/** Available HUD themes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<UMGHUDThemeData*> AvailableThemes;

	/** Default theme */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UMGHUDThemeData* DefaultTheme = nullptr;

	/** Available layout presets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<UMGHUDLayoutPresetData*> LayoutPresets;

	/** Widget classes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Widgets")
	TSubclassOf<UMGRaceHUDWidget> RaceHUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Widgets")
	TSubclassOf<UMGMinimapWidget> MinimapWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Widgets")
	TSubclassOf<UMGRaceOverlayWidget> OverlayWidgetClass;

	/** Default settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Defaults")
	bool bDefaultUseMPH = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Defaults")
	float DefaultHUDScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Defaults")
	float DefaultHUDOpacity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Defaults")
	EMGMinimapMode DefaultMinimapMode = EMGMinimapMode::RotatingMap;
};

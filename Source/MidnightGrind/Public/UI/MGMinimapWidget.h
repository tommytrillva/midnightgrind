// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * @file MGMinimapWidget.h
 * @brief Track minimap display showing racer positions and checkpoints
 *
 * =============================================================================
 * @section Overview
 * This file defines the minimap widget that displays a top-down view of the
 * race track with markers for all racers, checkpoints, and waypoints. The
 * minimap helps players navigate the track and track opponent positions.
 *
 * The minimap supports multiple display modes:
 * - Rotating: Map rotates so player always faces "up" (most common in racing)
 * - Fixed: North is always up, player icon rotates (useful for learning tracks)
 * - Full Track: Zoomed out to show the entire track at once
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **World-to-Minimap Coordinates**: The widget converts 3D world positions
 *   to 2D minimap positions using track bounds. This mapping is essential for
 *   correctly placing markers.
 *
 * - **Marker System**: A flexible marker system supports different types
 *   (player, opponents, checkpoints, hazards) with unique icons and colors.
 *
 * - **Smooth Interpolation**: Marker positions and rotations interpolate
 *   smoothly rather than jumping, preventing visual jitter.
 *
 * - **Zoom Levels**: Higher zoom values show more detail around the player
 *   but less of the overall track context.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Race Manager] ---> Positions ---> [MGMinimapWidget]
 *                                            |
 *   [Track Data] ---> Bounds/Texture --------|
 *                                            |
 *                                            v
 *                                      World-to-Minimap Transform
 *                                            |
 *                                            +-- Player Marker (green arrow)
 *                                            |
 *                                            +-- Opponent Markers (red dots)
 *                                            |
 *                                            +-- Checkpoint Markers (yellow)
 *                                            |
 *                                            +-- Optional Racing Line
 *
 * =============================================================================
 * @section Usage
 * @code
 * // Setup minimap with track data
 * MinimapWidget->SetTrackTexture(TrackMinimapTexture);
 * MinimapWidget->SetTrackBounds(FVector2D(-5000, -5000), FVector2D(5000, 5000));
 * MinimapWidget->SetMinimapMode(EMGMinimapMode::RotatingMap);
 * MinimapWidget->SetZoomLevel(2.5f);
 *
 * // Set opponent count (creates markers)
 * MinimapWidget->SetOpponentCount(7);
 *
 * // Update positions each frame
 * MinimapWidget->UpdatePlayerMarker(PlayerLocation, PlayerYaw);
 * for (int32 i = 0; i < Opponents.Num(); i++)
 * {
 *     MinimapWidget->UpdateOpponentMarker(i, OpponentLocation, OpponentYaw, Position);
 * }
 *
 * // Add custom markers
 * FMGMinimapMarker HazardMarker;
 * HazardMarker.Type = EMGMinimapMarkerType::Hazard;
 * HazardMarker.WorldPosition = OilSlickLocation;
 * HazardMarker.Color = FLinearColor::Red;
 * int32 MarkerID = MinimapWidget->AddMarker(HazardMarker);
 * @endcode
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGMinimapWidget.generated.h"

class UImage;
class UCanvasPanel;
class UTexture2D;

// =============================================================================
// Enums and Structs
// =============================================================================

/**
 * Minimap display mode
 * Determines how the map orients relative to player facing direction
 */
UENUM(BlueprintType)
enum class EMGMinimapMode : uint8
{
	/** Rotating map, player always points up */
	RotatingMap,
	/** Fixed map, north always up */
	FixedMap,
	/** Full track overview */
	FullTrack
};

/**
 * Minimap marker type
 */
UENUM(BlueprintType)
enum class EMGMinimapMarkerType : uint8
{
	Player,
	Opponent,
	Checkpoint,
	FinishLine,
	Waypoint,
	Hazard
};

/**
 * Minimap marker data
 */
USTRUCT(BlueprintType)
struct FMGMinimapMarker
{
	GENERATED_BODY()

	/** Marker type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMinimapMarkerType Type = EMGMinimapMarkerType::Opponent;

	/** World position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldPosition = FVector::ZeroVector;

	/** Rotation (yaw) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rotation = 0.0f;

	/** Marker color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	/** Marker scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.0f;

	/** Is marker visible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisible = true;

	/** Optional label (e.g., position number) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Label;

	/** Unique ID for this marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MarkerID = -1;
};

/**
 * Minimap Widget
 * Displays track overview with racer positions
 *
 * Features:
 * - Track texture display
 * - Player and opponent markers
 * - Checkpoint indicators
 * - Zoom control
 * - Rotating or fixed orientation
 * - Racing line preview
 */
UCLASS(Abstract)
class MIDNIGHTGRIND_API UMGMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ==========================================
	// TRACK SETUP
	// ==========================================

	/** Set the track texture */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Setup")
	void SetTrackTexture(UTexture2D* Texture);

	/** Set track world bounds for coordinate conversion */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Setup")
	void SetTrackBounds(FVector2D MinBounds, FVector2D MaxBounds);

	/** Set track rotation offset */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Setup")
	void SetTrackRotationOffset(float RotationDegrees);

	// ==========================================
	// DISPLAY MODE
	// ==========================================

	/** Set minimap display mode */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Display")
	void SetMinimapMode(EMGMinimapMode Mode);

	/** Get current mode */
	UFUNCTION(BlueprintPure, Category = "Minimap|Display")
	EMGMinimapMode GetMinimapMode() const { return CurrentMode; }

	/** Set zoom level (1.0 = full track, higher = zoomed in) */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Display")
	void SetZoomLevel(float Zoom);

	/** Get zoom level */
	UFUNCTION(BlueprintPure, Category = "Minimap|Display")
	float GetZoomLevel() const { return ZoomLevel; }

	/** Set minimap opacity */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Display")
	void SetMinimapOpacity(float Opacity);

	// ==========================================
	// MARKERS
	// ==========================================

	/** Update player marker */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Markers")
	void UpdatePlayerMarker(FVector WorldPosition, float Rotation);

	/** Update opponent marker */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Markers")
	void UpdateOpponentMarker(int32 OpponentIndex, FVector WorldPosition, float Rotation, int32 Position);

	/** Set opponent count (creates/removes markers) */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Markers")
	void SetOpponentCount(int32 Count);

	/** Add custom marker */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Markers")
	int32 AddMarker(const FMGMinimapMarker& Marker);

	/** Update marker by ID */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Markers")
	void UpdateMarker(int32 MarkerID, const FMGMinimapMarker& Marker);

	/** Remove marker by ID */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Markers")
	void RemoveMarker(int32 MarkerID);

	/** Clear all markers */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Markers")
	void ClearMarkers();

	/** Set marker visibility by type */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Markers")
	void SetMarkerTypeVisible(EMGMinimapMarkerType Type, bool bVisible);

	// ==========================================
	// CHECKPOINTS
	// ==========================================

	/** Set checkpoint positions */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Checkpoints")
	void SetCheckpoints(const TArray<FVector>& CheckpointPositions);

	/** Highlight next checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Checkpoints")
	void SetNextCheckpoint(int32 CheckpointIndex);

	/** Set finish line position */
	UFUNCTION(BlueprintCallable, Category = "Minimap|Checkpoints")
	void SetFinishLinePosition(FVector Position, float Rotation);

	// ==========================================
	// RACING LINE
	// ==========================================

	/** Set racing line points */
	UFUNCTION(BlueprintCallable, Category = "Minimap|RacingLine")
	void SetRacingLine(const TArray<FVector>& LinePoints);

	/** Show/hide racing line */
	UFUNCTION(BlueprintCallable, Category = "Minimap|RacingLine")
	void SetRacingLineVisible(bool bVisible);

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Player marker color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	FLinearColor PlayerMarkerColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	/** Opponent marker color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	FLinearColor OpponentMarkerColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** Checkpoint marker color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	FLinearColor CheckpointMarkerColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	/** Next checkpoint highlight color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	FLinearColor NextCheckpointColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);

	/** Racing line color */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	FLinearColor RacingLineColor = FLinearColor(0.0f, 0.5f, 1.0f, 0.5f);

	/** Default zoom level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	float DefaultZoomLevel = 2.0f;

	/** Marker size in pixels */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	float MarkerSize = 12.0f;

	/** Player marker size multiplier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	float PlayerMarkerSizeMultiplier = 1.5f;

	/** Smooth marker movement rate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	float MarkerInterpRate = 15.0f;

	/** Smooth rotation rate */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap|Config")
	float RotationInterpRate = 10.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current display mode */
	EMGMinimapMode CurrentMode = EMGMinimapMode::RotatingMap;

	/** Current zoom level */
	float ZoomLevel = 2.0f;

	/** Track bounds (world space) */
	FVector2D TrackBoundsMin = FVector2D(-10000.0f, -10000.0f);
	FVector2D TrackBoundsMax = FVector2D(10000.0f, 10000.0f);

	/** Track rotation offset */
	float TrackRotationOffset = 0.0f;

	/** Player position and rotation */
	FVector PlayerWorldPosition = FVector::ZeroVector;
	float PlayerRotation = 0.0f;
	FVector2D DisplayedPlayerPosition = FVector2D::ZeroVector;
	float DisplayedPlayerRotation = 0.0f;

	/** All markers */
	UPROPERTY()
	TArray<FMGMinimapMarker> Markers;

	/** Displayed marker positions (smoothed) */
	TMap<int32, FVector2D> DisplayedMarkerPositions;
	TMap<int32, float> DisplayedMarkerRotations;

	/** Next marker ID */
	int32 NextMarkerID = 0;

	/** Checkpoint positions */
	TArray<FVector> CheckpointPositions;

	/** Current next checkpoint index */
	int32 NextCheckpointIndex = 0;

	/** Racing line points */
	TArray<FVector> RacingLinePoints;

	/** Is racing line visible */
	bool bRacingLineVisible = false;

	/** Marker type visibility */
	TMap<EMGMinimapMarkerType, bool> MarkerTypeVisibility;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Convert world position to minimap position */
	UFUNCTION(BlueprintPure, Category = "Minimap|Internal")
	FVector2D WorldToMinimapPosition(FVector WorldPos) const;

	/** Convert minimap position to widget local position */
	UFUNCTION(BlueprintPure, Category = "Minimap|Internal")
	FVector2D MinimapToWidgetPosition(FVector2D MinimapPos) const;

	/** Update marker display positions (smoothing) */
	void UpdateMarkerDisplayPositions(float MGDeltaTime);

	/** Get current map rotation based on mode */
	float GetCurrentMapRotation() const;

	/** Render markers - override in Blueprint */
	UFUNCTION(BlueprintNativeEvent, Category = "Minimap|Internal")
	void RenderMarkers();

	/** Render checkpoints - override in Blueprint */
	UFUNCTION(BlueprintNativeEvent, Category = "Minimap|Internal")
	void RenderCheckpoints();

	/** Render racing line - override in Blueprint */
	UFUNCTION(BlueprintNativeEvent, Category = "Minimap|Internal")
	void RenderRacingLine();
};

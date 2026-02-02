// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGMinimapWidget.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"

void UMGMinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize marker type visibility
	MarkerTypeVisibility.Add(EMGMinimapMarkerType::Player, true);
	MarkerTypeVisibility.Add(EMGMinimapMarkerType::Opponent, true);
	MarkerTypeVisibility.Add(EMGMinimapMarkerType::Checkpoint, true);
	MarkerTypeVisibility.Add(EMGMinimapMarkerType::FinishLine, true);
	MarkerTypeVisibility.Add(EMGMinimapMarkerType::Waypoint, true);
	MarkerTypeVisibility.Add(EMGMinimapMarkerType::Hazard, true);

	ZoomLevel = DefaultZoomLevel;
}

void UMGMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateMarkerDisplayPositions(InDeltaTime);

	// Render all elements
	RenderMarkers();
	RenderCheckpoints();
	if (bRacingLineVisible)
	{
		RenderRacingLine();
	}
}

// ==========================================
// TRACK SETUP
// ==========================================

void UMGMinimapWidget::SetTrackTexture(UTexture2D* Texture)
{
	// Would set the texture on the track image widget
	// Implementation depends on widget structure
}

void UMGMinimapWidget::SetTrackBounds(FVector2D MinBounds, FVector2D MaxBounds)
{
	TrackBoundsMin = MinBounds;
	TrackBoundsMax = MaxBounds;
}

void UMGMinimapWidget::SetTrackRotationOffset(float RotationDegrees)
{
	TrackRotationOffset = RotationDegrees;
}

// ==========================================
// DISPLAY MODE
// ==========================================

void UMGMinimapWidget::SetMinimapMode(EMGMinimapMode Mode)
{
	CurrentMode = Mode;
}

void UMGMinimapWidget::SetZoomLevel(float Zoom)
{
	ZoomLevel = FMath::Clamp(Zoom, 0.5f, 10.0f);
}

void UMGMinimapWidget::SetMinimapOpacity(float Opacity)
{
	SetRenderOpacity(FMath::Clamp(Opacity, 0.0f, 1.0f));
}

// ==========================================
// MARKERS
// ==========================================

void UMGMinimapWidget::UpdatePlayerMarker(FVector WorldPosition, float Rotation)
{
	PlayerWorldPosition = WorldPosition;
	PlayerRotation = Rotation;
}

void UMGMinimapWidget::UpdateOpponentMarker(int32 OpponentIndex, FVector WorldPosition, float Rotation, int32 Position)
{
	// Find existing opponent marker or create new one
	for (FMGMinimapMarker& Marker : Markers)
	{
		if (Marker.Type == EMGMinimapMarkerType::Opponent && Marker.MarkerID == OpponentIndex)
		{
			Marker.WorldPosition = WorldPosition;
			Marker.Rotation = Rotation;
			Marker.Label = FText::AsNumber(Position);
			return;
		}
	}

	// Create new opponent marker
	FMGMinimapMarker NewMarker;
	NewMarker.Type = EMGMinimapMarkerType::Opponent;
	NewMarker.MarkerID = OpponentIndex;
	NewMarker.WorldPosition = WorldPosition;
	NewMarker.Rotation = Rotation;
	NewMarker.Color = OpponentMarkerColor;
	NewMarker.Label = FText::AsNumber(Position);
	Markers.Add(NewMarker);
}

void UMGMinimapWidget::SetOpponentCount(int32 Count)
{
	// Remove excess opponent markers
	Markers.RemoveAll([Count](const FMGMinimapMarker& Marker)
	{
		return Marker.Type == EMGMinimapMarkerType::Opponent && Marker.MarkerID >= Count;
	});
}

int32 UMGMinimapWidget::AddMarker(const FMGMinimapMarker& Marker)
{
	FMGMinimapMarker NewMarker = Marker;
	NewMarker.MarkerID = NextMarkerID++;

	Markers.Add(NewMarker);

	return NewMarker.MarkerID;
}

void UMGMinimapWidget::UpdateMarker(int32 MarkerID, const FMGMinimapMarker& Marker)
{
	for (FMGMinimapMarker& Existing : Markers)
	{
		if (Existing.MarkerID == MarkerID)
		{
			FMGMinimapMarker Updated = Marker;
			Updated.MarkerID = MarkerID;
			Existing = Updated;
			return;
		}
	}
}

void UMGMinimapWidget::RemoveMarker(int32 MarkerID)
{
	Markers.RemoveAll([MarkerID](const FMGMinimapMarker& Marker)
	{
		return Marker.MarkerID == MarkerID;
	});

	DisplayedMarkerPositions.Remove(MarkerID);
	DisplayedMarkerRotations.Remove(MarkerID);
}

void UMGMinimapWidget::ClearMarkers()
{
	Markers.Empty();
	DisplayedMarkerPositions.Empty();
	DisplayedMarkerRotations.Empty();
}

void UMGMinimapWidget::SetMarkerTypeVisible(EMGMinimapMarkerType Type, bool bVisible)
{
	MarkerTypeVisibility.Add(Type, bVisible);
}

// ==========================================
// CHECKPOINTS
// ==========================================

void UMGMinimapWidget::SetCheckpoints(const TArray<FVector>& Positions)
{
	CheckpointPositions = Positions;
}

void UMGMinimapWidget::SetNextCheckpoint(int32 CheckpointIndex)
{
	NextCheckpointIndex = CheckpointIndex;
}

void UMGMinimapWidget::SetFinishLinePosition(FVector Position, float Rotation)
{
	// Add or update finish line marker
	for (FMGMinimapMarker& Marker : Markers)
	{
		if (Marker.Type == EMGMinimapMarkerType::FinishLine)
		{
			Marker.WorldPosition = Position;
			Marker.Rotation = Rotation;
			return;
		}
	}

	// Add new finish line marker
	FMGMinimapMarker FinishMarker;
	FinishMarker.Type = EMGMinimapMarkerType::FinishLine;
	FinishMarker.WorldPosition = Position;
	FinishMarker.Rotation = Rotation;
	FinishMarker.Color = FLinearColor::White;
	FinishMarker.Scale = 1.5f;
	AddMarker(FinishMarker);
}

// ==========================================
// RACING LINE
// ==========================================

void UMGMinimapWidget::SetRacingLine(const TArray<FVector>& LinePoints)
{
	RacingLinePoints = LinePoints;
}

void UMGMinimapWidget::SetRacingLineVisible(bool bVisible)
{
	bRacingLineVisible = bVisible;
}

// ==========================================
// INTERNAL
// ==========================================

FVector2D UMGMinimapWidget::WorldToMinimapPosition(FVector WorldPos) const
{
	// Convert world position to normalized 0-1 range within track bounds
	FVector2D WorldPos2D(WorldPos.X, WorldPos.Y);

	FVector2D TrackSize = TrackBoundsMax - TrackBoundsMin;
	if (TrackSize.X == 0.0f || TrackSize.Y == 0.0f)
	{
		return FVector2D(0.5f, 0.5f);
	}

	FVector2D Normalized = (WorldPos2D - TrackBoundsMin) / TrackSize;

	// Clamp to 0-1
	Normalized.X = FMath::Clamp(Normalized.X, 0.0f, 1.0f);
	Normalized.Y = FMath::Clamp(Normalized.Y, 0.0f, 1.0f);

	return Normalized;
}

FVector2D UMGMinimapWidget::MinimapToWidgetPosition(FVector2D MinimapPos) const
{
	// This depends on widget size and zoom level
	// For rotating map mode, positions are relative to player center

	FVector2D WidgetSize = GetCachedGeometry().GetLocalSize();

	if (CurrentMode == EMGMinimapMode::RotatingMap)
	{
		// Player is always at center, world rotates around them
		FVector2D PlayerMinimapPos = WorldToMinimapPosition(PlayerWorldPosition);
		FVector2D RelativePos = (MinimapPos - PlayerMinimapPos) * ZoomLevel;

		// Rotate around center
		float MapRotation = FMath::DegreesToRadians(-PlayerRotation + TrackRotationOffset);
		float CosRot = FMath::Cos(MapRotation);
		float SinRot = FMath::Sin(MapRotation);

		FVector2D RotatedPos;
		RotatedPos.X = RelativePos.X * CosRot - RelativePos.Y * SinRot;
		RotatedPos.Y = RelativePos.X * SinRot + RelativePos.Y * CosRot;

		return (WidgetSize * 0.5f) + RotatedPos * WidgetSize;
	}
	else if (CurrentMode == EMGMinimapMode::FixedMap)
	{
		// Fixed orientation, zoom centered on player
		FVector2D PlayerMinimapPos = WorldToMinimapPosition(PlayerWorldPosition);
		FVector2D RelativePos = (MinimapPos - PlayerMinimapPos) * ZoomLevel;

		return (WidgetSize * 0.5f) + RelativePos * WidgetSize;
	}
	else // FullTrack
	{
		// No zoom, show full track
		return MinimapPos * WidgetSize;
	}
}

void UMGMinimapWidget::UpdateMarkerDisplayPositions(float DeltaTime)
{
	// Update player position smoothly
	FVector2D TargetPlayerPos = WorldToMinimapPosition(PlayerWorldPosition);
	DisplayedPlayerPosition = FMath::Vector2DInterpTo(DisplayedPlayerPosition, TargetPlayerPos, DeltaTime, MarkerInterpRate);
	DisplayedPlayerRotation = FMath::FInterpTo(DisplayedPlayerRotation, PlayerRotation, DeltaTime, RotationInterpRate);

	// Update all markers
	for (const FMGMinimapMarker& Marker : Markers)
	{
		FVector2D TargetPos = WorldToMinimapPosition(Marker.WorldPosition);

		if (DisplayedMarkerPositions.Contains(Marker.MarkerID))
		{
			FVector2D& CurrentPos = DisplayedMarkerPositions[Marker.MarkerID];
			CurrentPos = FMath::Vector2DInterpTo(CurrentPos, TargetPos, DeltaTime, MarkerInterpRate);
		}
		else
		{
			DisplayedMarkerPositions.Add(Marker.MarkerID, TargetPos);
		}

		if (DisplayedMarkerRotations.Contains(Marker.MarkerID))
		{
			float& CurrentRot = DisplayedMarkerRotations[Marker.MarkerID];
			CurrentRot = FMath::FInterpTo(CurrentRot, Marker.Rotation, DeltaTime, RotationInterpRate);
		}
		else
		{
			DisplayedMarkerRotations.Add(Marker.MarkerID, Marker.Rotation);
		}
	}
}

float UMGMinimapWidget::GetCurrentMapRotation() const
{
	switch (CurrentMode)
	{
	case EMGMinimapMode::RotatingMap:
		return -PlayerRotation + TrackRotationOffset;

	case EMGMinimapMode::FixedMap:
	case EMGMinimapMode::FullTrack:
	default:
		return TrackRotationOffset;
	}
}

void UMGMinimapWidget::RenderMarkers_Implementation()
{
	// Default implementation - override in Blueprint
	// Blueprint would iterate through Markers and draw them
}

void UMGMinimapWidget::RenderCheckpoints_Implementation()
{
	// Default implementation - override in Blueprint
}

void UMGMinimapWidget::RenderRacingLine_Implementation()
{
	// Default implementation - override in Blueprint
}

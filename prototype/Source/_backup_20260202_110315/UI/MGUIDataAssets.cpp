// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGUIDataAssets.h"
#include "UI/MGMinimapWidget.h"

// ==========================================
// HUD THEME DATA
// ==========================================

bool UMGHUDThemeData::GetElementLayout(FName ElementName, FMGHUDElementLayout& OutLayout) const
{
	for (const FMGHUDElementLayout& Layout : ElementLayouts)
	{
		if (Layout.ElementName == ElementName)
		{
			OutLayout = Layout;
			return true;
		}
	}
	return false;
}

// ==========================================
// TRACK MINIMAP DATA
// ==========================================

void UMGTrackMinimapData::ApplyToMinimap(UMGMinimapWidget* Minimap) const
{
	if (!Minimap)
	{
		return;
	}

	// Set track texture
	Minimap->SetTrackTexture(MinimapTexture);

	// Set bounds
	Minimap->SetTrackBounds(TrackBoundsMin, TrackBoundsMax);

	// Set rotation
	Minimap->SetTrackRotationOffset(RotationOffset);

	// Set checkpoints
	Minimap->SetCheckpoints(CheckpointPositions);

	// Set finish line
	Minimap->SetFinishLinePosition(FinishLinePosition, FinishLineRotation);

	// Set racing line
	if (RacingLinePoints.Num() > 0)
	{
		Minimap->SetRacingLine(RacingLinePoints);
	}

	// Set zoom
	Minimap->SetZoomLevel(SuggestedZoom);

	UE_LOG(LogTemp, Log, TEXT("Applied track minimap data '%s' to minimap"), *TrackName.ToString());
}

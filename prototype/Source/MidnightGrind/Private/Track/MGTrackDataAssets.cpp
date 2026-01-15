// Copyright Midnight Grind. All Rights Reserved.

#include "Track/MGTrackDataAssets.h"

// ==========================================
// UMGTrackDataAsset
// ==========================================

FMGGridPosition UMGTrackDataAsset::GetGridPosition(int32 Index) const
{
	if (Index >= 0 && Index < GridPositions.Num())
	{
		return GridPositions[Index];
	}

	// Return default position if invalid index
	FMGGridPosition DefaultPos;
	DefaultPos.GridIndex = Index;
	return DefaultPos;
}

FMGTrackSector UMGTrackDataAsset::GetSector(int32 Index) const
{
	if (Index >= 0 && Index < Sectors.Num())
	{
		return Sectors[Index];
	}

	FMGTrackSector DefaultSector;
	DefaultSector.SectorIndex = Index;
	return DefaultSector;
}

FName UMGTrackDataAsset::GetMedalForLapTime(float LapTime) const
{
	if (LapTime <= GoldLapTime)
	{
		return FName("Gold");
	}
	else if (LapTime <= SilverLapTime)
	{
		return FName("Silver");
	}
	else if (LapTime <= BronzeLapTime)
	{
		return FName("Bronze");
	}

	return FName("None");
}

FVector2D UMGTrackDataAsset::WorldToMinimapUV(FVector WorldPosition) const
{
	// Convert world position to minimap UV coordinates
	FVector2D WorldPos2D(WorldPosition.X, WorldPosition.Y);

	// Get bounds dimensions
	FVector2D BoundsMin = WorldBounds.Min;
	FVector2D BoundsMax = WorldBounds.Max;
	FVector2D BoundsSize = BoundsMax - BoundsMin;

	if (BoundsSize.X <= 0.0f || BoundsSize.Y <= 0.0f)
	{
		return FVector2D(0.5f, 0.5f);
	}

	// Normalize to 0-1 range
	FVector2D UV;
	UV.X = (WorldPos2D.X - BoundsMin.X) / BoundsSize.X;
	UV.Y = (WorldPos2D.Y - BoundsMin.Y) / BoundsSize.Y;

	// Apply rotation if needed
	if (MinimapRotation != 0.0f)
	{
		FVector2D Center(0.5f, 0.5f);
		FVector2D FromCenter = UV - Center;

		float Radians = FMath::DegreesToRadians(MinimapRotation);
		float Cos = FMath::Cos(Radians);
		float Sin = FMath::Sin(Radians);

		FVector2D Rotated;
		Rotated.X = FromCenter.X * Cos - FromCenter.Y * Sin;
		Rotated.Y = FromCenter.X * Sin + FromCenter.Y * Cos;

		UV = Center + Rotated;
	}

	// Clamp to valid range
	UV.X = FMath::Clamp(UV.X, 0.0f, 1.0f);
	UV.Y = FMath::Clamp(UV.Y, 0.0f, 1.0f);

	return UV;
}

FPrimaryAssetId UMGTrackDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("Track"), TrackID);
}

// ==========================================
// UMGTrackCollectionAsset
// ==========================================

FPrimaryAssetId UMGTrackCollectionAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("TrackCollection"), CollectionID);
}

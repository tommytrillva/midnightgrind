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

// ==========================================
// STAGE 49: DOWNTOWN DISTRICT DEFAULT DATA
// ==========================================

namespace MGTrackDefaults
{
	FMGTrackConfig CreateDowntownSprintNeonMile()
	{
		FMGTrackConfig Track;
		Track.TrackID = FName(TEXT("DOWNTOWN_SPRINT_NEON_MILE"));
		Track.DisplayName = FText::FromString(TEXT("Neon Mile"));
		Track.Description = FText::FromString(TEXT("A straight shot through the heart of downtown's neon district."));
		Track.DistrictID = FName(TEXT("DOWNTOWN"));
		Track.RaceType = EMGRaceType::Sprint;
		Track.Difficulty = EMGRaceDifficulty::Easy;
		Track.TotalDistanceMeters = 2400.0f;
		Track.EstimatedLapTime = 65.0f;
		Track.NumCheckpoints = 6;
		Track.BaseRewardCash = 1500;
		Track.BaseXP = 100;
		Track.bAvailableAtStart = true;
		return Track;
	}

	FMGTrackConfig CreateDowntownCircuitCityLoop()
	{
		FMGTrackConfig Track;
		Track.TrackID = FName(TEXT("DOWNTOWN_CIRCUIT_CITY_LOOP"));
		Track.DisplayName = FText::FromString(TEXT("City Loop"));
		Track.Description = FText::FromString(TEXT("A technical circuit weaving through downtown blocks."));
		Track.DistrictID = FName(TEXT("DOWNTOWN"));
		Track.RaceType = EMGRaceType::Circuit;
		Track.Difficulty = EMGRaceDifficulty::Medium;
		Track.TotalDistanceMeters = 3200.0f;
		Track.EstimatedLapTime = 90.0f;
		Track.NumCheckpoints = 8;
		Track.NumLaps = 3;
		Track.bIsCircuit = true;
		Track.BaseRewardCash = 2500;
		Track.BaseXP = 200;
		Track.MinPerformanceIndex = 300;
		Track.bAvailableAtStart = true;
		return Track;
	}

	FMGTrackConfig CreateDowntownDragHarborStrip()
	{
		FMGTrackConfig Track;
		Track.TrackID = FName(TEXT("DOWNTOWN_DRAG_HARBOR_STRIP"));
		Track.DisplayName = FText::FromString(TEXT("Harbor Strip"));
		Track.Description = FText::FromString(TEXT("Quarter mile on the abandoned harbor road. Pure speed."));
		Track.DistrictID = FName(TEXT("DOWNTOWN"));
		Track.RaceType = EMGRaceType::Drag;
		Track.Difficulty = EMGRaceDifficulty::Easy;
		Track.TotalDistanceMeters = 402.0f;
		Track.EstimatedLapTime = 12.0f;
		Track.NumCheckpoints = 2;
		Track.BaseRewardCash = 1000;
		Track.BaseXP = 75;
		Track.bAvailableAtStart = true;
		return Track;
	}

	FMGTrackConfig CreateDowntownDriftParkingGarage()
	{
		FMGTrackConfig Track;
		Track.TrackID = FName(TEXT("DOWNTOWN_DRIFT_PARKING_GARAGE"));
		Track.DisplayName = FText::FromString(TEXT("Parking Garage"));
		Track.Description = FText::FromString(TEXT("Multi-level garage with tight turns. Show your drift skills."));
		Track.DistrictID = FName(TEXT("DOWNTOWN"));
		Track.RaceType = EMGRaceType::Drift;
		Track.Difficulty = EMGRaceDifficulty::Medium;
		Track.TotalDistanceMeters = 1800.0f;
		Track.EstimatedLapTime = 120.0f;
		Track.NumCheckpoints = 12;
		Track.BaseRewardCash = 2000;
		Track.BaseXP = 150;
		Track.MinPerformanceIndex = 200;
		Track.bAvailableAtStart = true;
		return Track;
	}

	FMGTrackConfig CreateDowntownTougeHillsidePass()
	{
		FMGTrackConfig Track;
		Track.TrackID = FName(TEXT("DOWNTOWN_TOUGE_HILLSIDE"));
		Track.DisplayName = FText::FromString(TEXT("Hillside Pass"));
		Track.Description = FText::FromString(TEXT("Winding mountain road overlooking the city. Technical and fast."));
		Track.DistrictID = FName(TEXT("DOWNTOWN"));
		Track.RaceType = EMGRaceType::Touge;
		Track.Difficulty = EMGRaceDifficulty::Hard;
		Track.TotalDistanceMeters = 4500.0f;
		Track.EstimatedLapTime = 150.0f;
		Track.NumCheckpoints = 15;
		Track.BaseRewardCash = 4000;
		Track.BaseXP = 300;
		Track.MinPerformanceIndex = 400;
		Track.RequiredStoryProgress = 5;
		return Track;
	}

	TArray<FMGTrackConfig> GetAllDowntownTracks()
	{
		TArray<FMGTrackConfig> Tracks;
		Tracks.Add(CreateDowntownSprintNeonMile());
		Tracks.Add(CreateDowntownCircuitCityLoop());
		Tracks.Add(CreateDowntownDragHarborStrip());
		Tracks.Add(CreateDowntownDriftParkingGarage());
		Tracks.Add(CreateDowntownTougeHillsidePass());
		return Tracks;
	}

	FMGDistrictData CreateDowntownDistrict()
	{
		FMGDistrictData District;
		District.DistrictID = FName(TEXT("DOWNTOWN"));
		District.DisplayName = FText::FromString(TEXT("Downtown"));
		District.Description = FText::FromString(TEXT("The neon-lit heart of the city. Skyscrapers, endless streets, and where legends are born."));
		District.bStarterDistrict = true;
		District.RequiredPlayerLevel = 1;
		District.AmbientTimeOfDay = 22.0f;
		District.TrafficDensity = 0.6f;
		return District;
	}
}

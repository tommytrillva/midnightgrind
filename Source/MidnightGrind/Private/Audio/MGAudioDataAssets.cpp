// Copyright Midnight Grind. All Rights Reserved.

#include "Audio/MGAudioDataAssets.h"
#include "Audio/MGVehicleSFXComponent.h"
#include "Audio/MGMusicManager.h"

// ==========================================
// UMGVehicleSFXPresetData
// ==========================================

void UMGVehicleSFXPresetData::ApplyToComponent(UMGVehicleSFXComponent* Component) const
{
	if (!Component)
	{
		return;
	}

	// Apply surface configs
	for (const FMGSurfaceSoundConfig& Config : SurfaceConfigs)
	{
		Component->AddSurfaceConfig(Config);
	}

	// Apply collision config
	Component->SetCollisionConfig(CollisionConfig);

	// Note: Wind, suspension, and brake sounds are typically set in the component
	// itself via the data asset reference, rather than copied here
}

// ==========================================
// UMGMusicPlaylistData
// ==========================================

TArray<FName> UMGMusicPlaylistData::GetTrackNames() const
{
	TArray<FName> Names;
	for (UMGMusicTrackData* TrackData : Tracks)
	{
		if (TrackData)
		{
			Names.Add(TrackData->Track.TrackName);
		}
	}
	return Names;
}

void UMGMusicPlaylistData::RegisterWithMusicManager(UMGMusicManager* MusicManager) const
{
	if (!MusicManager)
	{
		return;
	}

	for (UMGMusicTrackData* TrackData : Tracks)
	{
		if (TrackData)
		{
			MusicManager->RegisterTrack(TrackData->Track);
		}
	}

	if (bShuffleByDefault)
	{
		MusicManager->ShufflePlaylist();
	}
}

// ==========================================
// UMGStingerCollectionData
// ==========================================

void UMGStingerCollectionData::RegisterWithMusicManager(UMGMusicManager* MusicManager) const
{
	if (!MusicManager)
	{
		return;
	}

	// Register named stingers
	auto RegisterIfValid = [MusicManager](const FMGMusicStinger& Stinger)
	{
		if (!Stinger.StingerName.IsNone() && Stinger.Sound)
		{
			MusicManager->RegisterStinger(Stinger);
		}
	};

	RegisterIfValid(CountdownStinger);
	RegisterIfValid(RaceStartStinger);
	RegisterIfValid(FinalLapStinger);
	RegisterIfValid(PositionGainedStinger);
	RegisterIfValid(PositionLostStinger);
	RegisterIfValid(VictoryStinger);
	RegisterIfValid(DefeatStinger);
	RegisterIfValid(NewRecordStinger);

	// Register custom stingers
	for (const FMGMusicStinger& Stinger : CustomStingers)
	{
		RegisterIfValid(Stinger);
	}
}

// ==========================================
// UMGAudioConfigData
// ==========================================

UMGEngineAudioPresetData* UMGAudioConfigData::GetEnginePresetForClass(FName VehicleClass) const
{
	UMGEngineAudioPresetData* const* Preset = EnginePresetsByClass.Find(VehicleClass);
	if (Preset && *Preset)
	{
		return *Preset;
	}

	// Fallback to "Default" class if exists
	Preset = EnginePresetsByClass.Find(FName("Default"));
	if (Preset && *Preset)
	{
		return *Preset;
	}

	return nullptr;
}

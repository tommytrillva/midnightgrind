// Copyright Midnight Grind. All Rights Reserved.

#include "Replay/MGReplayDataAssets.h"

// ==========================================
// UMGGhostSettingsAsset
// ==========================================

FMGGhostVisualPreset UMGGhostSettingsAsset::GetPresetForType(EMGGhostType GhostType) const
{
	switch (GhostType)
	{
		case EMGGhostType::PersonalBest:
			return PersonalBestPreset;

		case EMGGhostType::Friend:
			return FriendPreset;

		case EMGGhostType::WorldRecord:
			return WorldRecordPreset;

		case EMGGhostType::Developer:
			return DeveloperPreset;

		case EMGGhostType::Custom:
		default:
			return PersonalBestPreset;
	}
}

// ==========================================
// UMGReplayCameraAsset
// ==========================================

FMGReplayCameraPreset UMGReplayCameraAsset::GetPreset(FName PresetID) const
{
	for (const FMGReplayCameraPreset& Preset : CameraPresets)
	{
		if (Preset.PresetID == PresetID)
		{
			return Preset;
		}
	}

	// Return default or first preset
	if (CameraPresets.Num() > 0)
	{
		// Try to find default
		for (const FMGReplayCameraPreset& Preset : CameraPresets)
		{
			if (Preset.PresetID == DefaultPresetID)
			{
				return Preset;
			}
		}

		return CameraPresets[0];
	}

	return FMGReplayCameraPreset();
}

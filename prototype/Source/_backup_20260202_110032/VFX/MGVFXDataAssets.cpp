// Copyright Midnight Grind. All Rights Reserved.

#include "VFX/MGVFXDataAssets.h"
#include "VFX/MGVFXSubsystem.h"
#include "VFX/MGVehicleVFXComponent.h"
#include "VFX/MGEnvironmentVFXManager.h"
#include "VFX/MGCameraVFXComponent.h"

// ==========================================
// VEHICLE VFX PRESET
// ==========================================

void UMGVehicleVFXPresetData::ApplyToComponent(UMGVehicleVFXComponent* Component) const
{
	if (!Component)
	{
		return;
	}

	// Note: The component uses EditDefaultsOnly properties, so this would need
	// to be applied at spawn time or the component redesigned to accept runtime config.
	// For now, this serves as documentation of the data asset structure.

	// Set exhaust configs
	if (ExhaustConfigs.Num() > 0)
	{
		Component->SetExhaustConfigs(ExhaustConfigs);
	}

	// Set drift trail color
	Component->SetDriftTrailColor(DefaultDriftColor);

	UE_LOG(LogTemp, Log, TEXT("Applied vehicle VFX preset '%s'"), *PresetName.ToString());
}

// ==========================================
// WEATHER VFX PRESET
// ==========================================

void UMGWeatherVFXPresetData::ApplyToEnvironmentManager(AMGEnvironmentVFXManager* Manager) const
{
	if (!Manager)
	{
		return;
	}

	// Apply weather configuration
	Manager->SetWeatherConfig(WeatherConfig);

	UE_LOG(LogTemp, Log, TEXT("Applied weather VFX preset '%s'"), *PresetName.ToString());
}

// ==========================================
// ZONE VFX PRESET
// ==========================================

void UMGZoneVFXPresetData::RegisterWithEnvironmentManager(AMGEnvironmentVFXManager* Manager) const
{
	if (!Manager)
	{
		return;
	}

	// Create zone config
	FMGZoneParticleConfig Config = ParticleConfig;
	Config.Zone = ZoneType;
	Config.DensityMultiplier = ParticleDensityMultiplier;
	Config.ZoneTint = AmbientColor;

	// Use day ambient as default
	if (DayAmbientSystem)
	{
		Config.AmbientParticles = DayAmbientSystem;
	}

	// Register with manager
	Manager->RegisterZoneConfig(Config);

	UE_LOG(LogTemp, Log, TEXT("Registered zone VFX preset '%s' for zone %d"), *PresetName.ToString(), static_cast<int32>(ZoneType));
}

// ==========================================
// CAMERA VFX PRESET
// ==========================================

void UMGCameraVFXPresetData::ApplyToComponent(UMGCameraVFXComponent* Component) const
{
	if (!Component)
	{
		return;
	}

	// Apply speed effect config
	Component->SetSpeedEffectConfig(SpeedEffectConfig);

	// Apply drift config
	Component->SetDriftCameraConfig(DriftCameraConfig);

	// Apply shake multiplier
	Component->SetShakeIntensityMultiplier(ShakeIntensityMultiplier);

	// Apply base FOV
	Component->SetBaseFOV(BaseFOV);

	UE_LOG(LogTemp, Log, TEXT("Applied camera VFX preset '%s'"), *PresetName.ToString());
}

// ==========================================
// EVENT VFX PRESET
// ==========================================

void UMGEventVFXPresetData::RegisterWithVFXSubsystem(UMGVFXSubsystem* Subsystem) const
{
	if (!Subsystem)
	{
		return;
	}

	// Register all event systems
	for (const auto& Pair : EventSystems)
	{
		if (Pair.Value)
		{
			int32 Priority = 0;
			if (const int32* FoundPriority = EventPriorities.Find(Pair.Key))
			{
				Priority = *FoundPriority;
			}

			Subsystem->RegisterEventVFX(Pair.Key, Pair.Value, Priority);
		}
	}

	// Register special events
	if (FinishLineCelebrationSystem)
	{
		Subsystem->RegisterEventVFX(EMGVFXEvent::FinishLine, FinishLineCelebrationSystem, 0);
	}

	if (NearMissRewardSystem)
	{
		Subsystem->RegisterEventVFX(EMGVFXEvent::NearMiss, NearMissRewardSystem, 1);
	}

	UE_LOG(LogTemp, Log, TEXT("Registered event VFX preset '%s' with %d events"), *PresetName.ToString(), EventSystems.Num());
}

// ==========================================
// MASTER VFX CONFIG
// ==========================================

UMGVehicleVFXPresetData* UMGVFXConfigData::GetVehiclePresetForClass(FName VehicleClass) const
{
	if (UMGVehicleVFXPresetData* const* Found = VehiclePresetsByClass.Find(VehicleClass))
	{
		return *Found;
	}

	return DefaultVehiclePreset;
}

UMGWeatherVFXPresetData* UMGVFXConfigData::GetWeatherPreset(EMGWeatherType Weather) const
{
	if (UMGWeatherVFXPresetData* const* Found = WeatherPresets.Find(Weather))
	{
		return *Found;
	}

	return nullptr;
}

UMGZoneVFXPresetData* UMGVFXConfigData::GetZonePreset(EMGEnvironmentZone Zone) const
{
	for (UMGZoneVFXPresetData* Preset : ZonePresets)
	{
		if (Preset && Preset->ZoneType == Zone)
		{
			return Preset;
		}
	}

	return nullptr;
}

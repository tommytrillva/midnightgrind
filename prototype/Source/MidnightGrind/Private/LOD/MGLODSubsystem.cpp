// Copyright Midnight Grind. All Rights Reserved.

#include "LOD/MGLODSubsystem.h"

void UMGLODSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default settings per category
	FMGLODSettings VehicleSettings;
	VehicleSettings.LOD1Distance = 30.0f;
	VehicleSettings.LOD2Distance = 80.0f;
	VehicleSettings.LOD3Distance = 150.0f;
	VehicleSettings.CullDistance = 500.0f;
	CategorySettings.Add(EMGLODCategory::Vehicle, VehicleSettings);

	FMGLODSettings EnvironmentSettings;
	EnvironmentSettings.LOD1Distance = 100.0f;
	EnvironmentSettings.LOD2Distance = 250.0f;
	EnvironmentSettings.LOD3Distance = 500.0f;
	EnvironmentSettings.CullDistance = 1500.0f;
	CategorySettings.Add(EMGLODCategory::Environment, EnvironmentSettings);

	FMGLODSettings PropsSettings;
	PropsSettings.LOD1Distance = 50.0f;
	PropsSettings.LOD2Distance = 100.0f;
	PropsSettings.LOD3Distance = 200.0f;
	PropsSettings.CullDistance = 400.0f;
	CategorySettings.Add(EMGLODCategory::Props, PropsSettings);

	FMGLODSettings EffectsSettings;
	EffectsSettings.LOD1Distance = 30.0f;
	EffectsSettings.LOD2Distance = 60.0f;
	EffectsSettings.CullDistance = 150.0f;
	CategorySettings.Add(EMGLODCategory::Effects, EffectsSettings);
}

void UMGLODSubsystem::SetLODSettings(EMGLODCategory Category, const FMGLODSettings& Settings)
{
	CategorySettings.Add(Category, Settings);
	OnLODSettingsChanged.Broadcast(Category, Settings);
}

FMGLODSettings UMGLODSubsystem::GetLODSettings(EMGLODCategory Category) const
{
	const FMGLODSettings* Found = CategorySettings.Find(Category);
	return Found ? *Found : FMGLODSettings();
}

void UMGLODSubsystem::ApplyQualityPreset(int32 QualityLevel)
{
	float BiasMultiplier = 1.0f;

	switch (QualityLevel)
	{
	case 0: // Low
		BiasMultiplier = 2.0f;
		break;
	case 1: // Medium
		BiasMultiplier = 1.5f;
		break;
	case 2: // High
		BiasMultiplier = 1.0f;
		break;
	case 3: // Ultra
		BiasMultiplier = 0.75f;
		break;
	}

	for (auto& Pair : CategorySettings)
	{
		FMGLODSettings& Settings = Pair.Value;
		Settings.LODBias = (BiasMultiplier - 1.0f) * 100.0f;
		OnLODSettingsChanged.Broadcast(Pair.Key, Settings);
	}
}

EMGLODLevel UMGLODSubsystem::CalculateLOD(EMGLODCategory Category, float Distance) const
{
	FMGLODSettings Settings = GetLODSettings(Category);

	if (Settings.bForceLOD)
		return Settings.ForcedLOD;

	// Apply bias and speed factor
	float EffectiveDistance = Distance * (1.0f + Settings.LODBias / 100.0f);

	if (bSpeedBasedScaling)
	{
		// At high speed, push LOD transitions further out for smoother visuals
		EffectiveDistance /= SpeedFactor;
	}

	EffectiveDistance *= (1.0f + GlobalLODBias / 100.0f);

	if (EffectiveDistance >= Settings.CullDistance)
		return EMGLODLevel::Culled;
	if (EffectiveDistance >= Settings.LOD4Distance)
		return EMGLODLevel::LOD4;
	if (EffectiveDistance >= Settings.LOD3Distance)
		return EMGLODLevel::LOD3;
	if (EffectiveDistance >= Settings.LOD2Distance)
		return EMGLODLevel::LOD2;
	if (EffectiveDistance >= Settings.LOD1Distance)
		return EMGLODLevel::LOD1;

	return EMGLODLevel::LOD0;
}

void UMGLODSubsystem::SetGlobalLODBias(float Bias)
{
	GlobalLODBias = FMath::Clamp(Bias, -50.0f, 100.0f);
}

void UMGLODSubsystem::SetSpeedBasedLODScaling(bool bEnabled)
{
	bSpeedBasedScaling = bEnabled;
}

void UMGLODSubsystem::UpdateSpeedFactor(float CurrentSpeed)
{
	// At 200+ km/h, increase LOD distances by 50%
	const float MaxSpeed = 200.0f * 0.277778f; // km/h to m/s
	SpeedFactor = FMath::Lerp(1.0f, 1.5f, FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f));
}

void UMGLODSubsystem::UpdateStats()
{
	// Would query render stats
	CurrentStats = FMGLODStats();
}

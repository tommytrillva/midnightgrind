// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGPhysicsConstants.h"

FMGPhysicsHandlingSettings FMGPhysicsHandlingSettings::GetDefaultsForMode(EMGPhysicsHandlingMode InMode)
{
	return UMGPhysicsHandlingConfig::GetSettingsForMode(InMode);
}

FMGPhysicsHandlingSettings UMGPhysicsHandlingConfig::GetArcadeSettings()
{
	FMGPhysicsHandlingSettings Settings;
	Settings.Mode = EMGPhysicsHandlingMode::Arcade;

	// Strong assists for accessibility
	Settings.StabilityControl = 0.7f;
	Settings.AntiFlipTorque = 15000.0f;
	Settings.SpeedSensitiveSteeringFactor = 0.8f;

	// Forgiving physics
	Settings.WeightTransferRate = 4.0f;        // Slower, less punishing
	Settings.BaseTireGrip = 1.2f;              // 20% more grip
	Settings.TireTempInfluence = 0.0f;         // No temp effects
	Settings.TurboLagSimulation = 0.0f;        // Instant boost
	Settings.EngineBrakingMultiplier = 0.5f;   // Less aggressive

	// Snappy steering
	Settings.ArcadeSteeringSpeed = 8.0f;
	Settings.ArcadeSteeringReturnSpeed = 12.0f;

	return Settings;
}

FMGPhysicsHandlingSettings UMGPhysicsHandlingConfig::GetBalancedSettings()
{
	FMGPhysicsHandlingSettings Settings;
	Settings.Mode = EMGPhysicsHandlingMode::Balanced;

	// Moderate assists
	Settings.StabilityControl = 0.3f;
	Settings.AntiFlipTorque = 5000.0f;
	Settings.SpeedSensitiveSteeringFactor = 0.5f;

	// Standard physics
	Settings.WeightTransferRate = 8.0f;
	Settings.BaseTireGrip = 1.0f;
	Settings.TireTempInfluence = 0.3f;         // Partial temp effects
	Settings.TurboLagSimulation = 0.5f;        // Partial lag
	Settings.EngineBrakingMultiplier = 1.0f;

	// Standard steering
	Settings.ArcadeSteeringSpeed = 5.0f;
	Settings.ArcadeSteeringReturnSpeed = 8.0f;

	return Settings;
}

FMGPhysicsHandlingSettings UMGPhysicsHandlingConfig::GetSimulationSettings()
{
	FMGPhysicsHandlingSettings Settings;
	Settings.Mode = EMGPhysicsHandlingMode::Simulation;

	// Minimal assists
	Settings.StabilityControl = 0.0f;          // No stability control
	Settings.AntiFlipTorque = 0.0f;            // No anti-flip
	Settings.SpeedSensitiveSteeringFactor = 0.2f; // Minimal reduction

	// Realistic physics
	Settings.WeightTransferRate = 12.0f;       // Fast, realistic
	Settings.BaseTireGrip = 1.0f;              // Standard grip
	Settings.TireTempInfluence = 1.0f;         // Full temp simulation
	Settings.TurboLagSimulation = 1.0f;        // Full lag simulation
	Settings.EngineBrakingMultiplier = 1.2f;   // Realistic engine braking

	// Direct steering (for wheel users)
	Settings.ArcadeSteeringSpeed = 3.0f;       // Slower, more realistic
	Settings.ArcadeSteeringReturnSpeed = 5.0f;

	return Settings;
}

FMGPhysicsHandlingSettings UMGPhysicsHandlingConfig::GetSettingsForMode(EMGPhysicsHandlingMode Mode)
{
	switch (Mode)
	{
	case EMGPhysicsHandlingMode::Arcade:
		return GetArcadeSettings();

	case EMGPhysicsHandlingMode::Simulation:
		return GetSimulationSettings();

	case EMGPhysicsHandlingMode::Balanced:
	default:
		return GetBalancedSettings();
	}
}

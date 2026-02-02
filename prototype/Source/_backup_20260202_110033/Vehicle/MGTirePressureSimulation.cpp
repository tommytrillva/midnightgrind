// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTirePressureSimulation.cpp
 * @brief Implementation of tire pressure simulation for UMGVehicleMovementComponent
 *
 * This file contains all tire pressure-related implementations including:
 * - Pressure state queries
 * - Damage API (punctures, spike strips, blowouts)
 * - Helper methods for pressure calculations
 *
 * The implementation integrates with the existing tire temperature system
 * and provides realistic pressure-grip-wear relationships.
 */

#include "Vehicle/MGVehicleMovementComponent.h"

// ==========================================
// TIRE PRESSURE STATE QUERIES
// ==========================================

FMGTirePressureState UMGVehicleMovementComponent::GetTirePressureState(int32 WheelIndex) const
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return FMGTirePressureState();
	}
	return TirePressures[WheelIndex];
}

float UMGVehicleMovementComponent::GetTirePressurePSI(int32 WheelIndex) const
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return 0.0f;
	}
	return TirePressures[WheelIndex].CurrentPressurePSI;
}

float UMGVehicleMovementComponent::GetOptimalTirePressurePSI(int32 WheelIndex) const
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return TirePressureConfig.OptimalPressure_Street;
	}
	return TirePressures[WheelIndex].OptimalHotPressurePSI;
}

bool UMGVehicleMovementComponent::HasTirePressureWarning() const
{
	for (int32 i = 0; i < 4; ++i)
	{
		if (TirePressures[i].NeedsAttention())
		{
			return true;
		}
	}
	return false;
}

bool UMGVehicleMovementComponent::HasFlatTire() const
{
	for (int32 i = 0; i < 4; ++i)
	{
		if (TirePressures[i].bIsFlat || TirePressures[i].bIsBlownOut)
		{
			return true;
		}
	}
	return false;
}

float UMGVehicleMovementComponent::GetAverageTirePressure() const
{
	float TotalPressure = 0.0f;
	for (int32 i = 0; i < 4; ++i)
	{
		TotalPressure += TirePressures[i].CurrentPressurePSI;
	}
	return TotalPressure / 4.0f;
}

float UMGVehicleMovementComponent::GetTotalRollingResistanceFromPressure() const
{
	float TotalResistance = 0.0f;
	for (int32 i = 0; i < 4; ++i)
	{
		TotalResistance += TirePressures[i].RollingResistanceMultiplier;
	}
	return TotalResistance / 4.0f;
}

// ==========================================
// TIRE PRESSURE DAMAGE API
// ==========================================

void UMGVehicleMovementComponent::ApplyTirePuncture(int32 WheelIndex, EMGPressureLossCause Cause, float Severity)
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return;
	}

	FMGTirePressureState& Pressure = TirePressures[WheelIndex];

	// Determine leak rate based on cause
	float LeakRate = 0.0f;
	switch (Cause)
	{
	case EMGPressureLossCause::NaturalLeak:
		LeakRate = TirePressureConfig.NaturalLeakRatePSIPerHour / 3600.0f; // Convert to per-second
		break;
	case EMGPressureLossCause::SlowLeak:
		LeakRate = TirePressureConfig.SlowLeakRatePSIPerSec;
		break;
	case EMGPressureLossCause::ModerateLeakDamage:
		LeakRate = TirePressureConfig.ModerateLeakRatePSIPerSec;
		break;
	case EMGPressureLossCause::SpikeStripPuncture:
		LeakRate = TirePressureConfig.SpikeStripLeakRatePSIPerSec;
		break;
	case EMGPressureLossCause::ValveStemDamage:
		LeakRate = TirePressureConfig.ValveStemLeakRatePSIPerSec;
		break;
	case EMGPressureLossCause::BeadSeparation:
		LeakRate = TirePressureConfig.BeadSeparationLeakRatePSIPerSec;
		break;
	case EMGPressureLossCause::Blowout:
		// Blowout is instant, not a leak
		CauseTireBlowout(WheelIndex, Cause);
		return;
	default:
		LeakRate = TirePressureConfig.SlowLeakRatePSIPerSec;
		break;
	}

	// Apply severity modifier
	LeakRate *= FMath::Clamp(Severity, 0.0f, 1.0f);

	// Start the leak
	Pressure.StartLeak(Cause, LeakRate);

	// Broadcast warning event
	OnTirePressureWarning.Broadcast(WheelIndex, Pressure.CurrentPressurePSI, Cause);
}

void UMGVehicleMovementComponent::ApplySpikeStripDamage(const TArray<int32>& AffectedWheels)
{
	for (int32 WheelIndex : AffectedWheels)
	{
		if (WheelIndex >= 0 && WheelIndex < 4)
		{
			ApplyTirePuncture(WheelIndex, EMGPressureLossCause::SpikeStripPuncture, 1.0f);
		}
	}
}

void UMGVehicleMovementComponent::CauseTireBlowout(int32 WheelIndex, EMGPressureLossCause Cause)
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return;
	}

	FMGTirePressureState& Pressure = TirePressures[WheelIndex];

	// Apply instant blowout
	Pressure.Blowout(Cause);

	// Broadcast blowout event
	OnTireBlowout.Broadcast(WheelIndex, Cause);
}

void UMGVehicleMovementComponent::SetTireColdPressure(int32 WheelIndex, float PressurePSI)
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return;
	}

	// Clamp to valid range
	float ClampedPressure = FMath::Clamp(PressurePSI, 20.0f, 50.0f);

	TirePressures[WheelIndex].ColdPressurePSI = ClampedPressure;
	TirePressures[WheelIndex].CurrentPressurePSI = ClampedPressure;
	TirePressures[WheelIndex].UpdateCachedEffects();
}

void UMGVehicleMovementComponent::SetAllTiresColdPressure(float FrontPressurePSI, float RearPressurePSI)
{
	// Front tires
	SetTireColdPressure(0, FrontPressurePSI);
	SetTireColdPressure(1, FrontPressurePSI);

	// Rear tires
	SetTireColdPressure(2, RearPressurePSI);
	SetTireColdPressure(3, RearPressurePSI);
}

void UMGVehicleMovementComponent::RepairTire(int32 WheelIndex)
{
	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return;
	}

	const float OptimalPressure = GetOptimalPressureForCompound(CurrentConfiguration.TireCompound);
	TirePressures[WheelIndex].Repair(TirePressureConfig.DefaultColdPressurePSI, OptimalPressure);
}

void UMGVehicleMovementComponent::RepairAllTires()
{
	for (int32 i = 0; i < 4; ++i)
	{
		RepairTire(i);
	}
}

// ==========================================
// TIRE PRESSURE HELPER METHODS
// ==========================================

float UMGVehicleMovementComponent::CalculateHotPressure(float ColdPressure, float TireTemp) const
{
	// Ideal gas law approximation: P_hot = P_cold * (T_hot / T_cold)
	// Simplified to linear approximation for game performance
	float TempDelta = TireTemp - TirePressureConfig.ReferenceAmbientTempC;
	return ColdPressure + (TempDelta * TirePressureConfig.PressurePerDegreeC);
}

float UMGVehicleMovementComponent::GetOptimalPressureForCompound(EMGTireCompound Compound) const
{
	switch (Compound)
	{
	case EMGTireCompound::Street:
		return TirePressureConfig.OptimalPressure_Street;
	case EMGTireCompound::Sport:
		return TirePressureConfig.OptimalPressure_Sport;
	case EMGTireCompound::Track:
		return TirePressureConfig.OptimalPressure_Track;
	case EMGTireCompound::Drift:
		return TirePressureConfig.OptimalPressure_Drift;
	case EMGTireCompound::Rain:
		return TirePressureConfig.OptimalPressure_Rain;
	case EMGTireCompound::OffRoad:
		return TirePressureConfig.OptimalPressure_OffRoad;
	default:
		return TirePressureConfig.OptimalPressure_Street;
	}
}

bool UMGVehicleMovementComponent::CheckAndApplyBlowoutRisk(int32 WheelIndex, float DeltaTime)
{
	if (!TirePressureConfig.bEnableBlowoutSimulation)
	{
		return false;
	}

	if (WheelIndex < 0 || WheelIndex >= 4)
	{
		return false;
	}

	FMGTirePressureState& Pressure = TirePressures[WheelIndex];

	// Already blown out - can't blow out twice
	if (Pressure.bIsBlownOut)
	{
		return false;
	}

	// Check blowout conditions
	const float TireTemp = TireTemperatures[WheelIndex].GetAverageTemp();
	const float PressureRatio = Pressure.CurrentPressurePSI / FMath::Max(Pressure.OptimalHotPressurePSI, 1.0f);

	// Must exceed temperature threshold OR be below pressure threshold
	bool bOverTemp = TireTemp > TirePressureConfig.BlowoutTempThresholdC;
	bool bUnderPressure = PressureRatio < TirePressureConfig.BlowoutPressureRatioThreshold;

	// No risk if neither condition is met
	if (!bOverTemp && !bUnderPressure)
	{
		return false;
	}

	// Calculate base blowout probability per second
	float BlowoutProb = TirePressureConfig.BlowoutBaseProbabilityPerSec;

	// Increase probability with temperature excess
	if (bOverTemp)
	{
		float TempExcess = TireTemp - TirePressureConfig.BlowoutTempThresholdC;
		BlowoutProb *= (1.0f + TempExcess * 0.05f); // 5% increase per degree over threshold
	}

	// Increase probability with low pressure
	if (bUnderPressure)
	{
		// More risk the lower the pressure ratio
		BlowoutProb *= (2.0f - PressureRatio * 2.0f);
	}

	// Increase probability with vehicle speed
	float SpeedMPH = GetSpeedMPH();
	BlowoutProb *= (1.0f + (SpeedMPH / 100.0f) * TirePressureConfig.BlowoutSpeedMultiplier);

	// Combined risk if both conditions present
	if (bOverTemp && bUnderPressure)
	{
		BlowoutProb *= 2.0f; // Double risk when both conditions present
	}

	// Scale probability by delta time for frame-rate independence
	BlowoutProb *= DeltaTime;

	// Clamp probability to reasonable range
	BlowoutProb = FMath::Clamp(BlowoutProb, 0.0f, 0.5f);

	// Random check for blowout
	if (FMath::FRand() < BlowoutProb)
	{
		CauseTireBlowout(WheelIndex, EMGPressureLossCause::Blowout);
		return true;
	}

	return false;
}

void UMGVehicleMovementComponent::ApplyLeak(int32 WheelIndex, EMGPressureLossCause Cause, float Severity)
{
	// Delegate to the main puncture method
	ApplyTirePuncture(WheelIndex, Cause, Severity);
}

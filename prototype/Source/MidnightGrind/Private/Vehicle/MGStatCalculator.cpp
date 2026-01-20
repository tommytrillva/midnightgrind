// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGStatCalculator.h"

float UMGStatCalculator::CalculateHorsepower(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel)
{
	if (!BaseModel)
	{
		return 0.0f;
	}

	float BaseHP = BaseModel->BaseHorsepower;

	// Get combined modifiers from all parts
	FMGPartModifiers Modifiers = GetCombinedModifiers(Engine);

	// Apply power multiplier
	BaseHP *= Modifiers.PowerMultiplier;

	// Apply forced induction
	if (Engine.ForcedInduction.Type != EMGForcedInductionType::None)
	{
		// Boost multiplier: roughly 5-8% per PSI depending on efficiency
		const float BoostMultiplier = 1.0f + (Engine.ForcedInduction.MaxBoostPSI * 0.065f * Engine.ForcedInduction.IntercoolerEfficiency);
		BaseHP *= BoostMultiplier;
	}

	// Apply ECU tune efficiency
	const float TuneMultipliers[] = { 0.90f, 0.95f, 1.00f, 1.02f };
	const int32 TuneIndex = FMath::Clamp(Engine.TuneLevel, 0, 3);
	BaseHP *= TuneMultipliers[TuneIndex];

	// Add nitrous shot (when calculating peak potential)
	if (Engine.Nitrous.bInstalled)
	{
		BaseHP += Engine.Nitrous.ShotSizeHP;
	}

	return BaseHP;
}

float UMGStatCalculator::CalculateTorque(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel)
{
	if (!BaseModel)
	{
		return 0.0f;
	}

	float BaseTQ = BaseModel->BaseTorque;

	FMGPartModifiers Modifiers = GetCombinedModifiers(Engine);
	BaseTQ *= Modifiers.TorqueMultiplier;

	// Forced induction typically adds more torque than HP
	if (Engine.ForcedInduction.Type != EMGForcedInductionType::None)
	{
		const float BoostMultiplier = 1.0f + (Engine.ForcedInduction.MaxBoostPSI * 0.08f * Engine.ForcedInduction.IntercoolerEfficiency);
		BaseTQ *= BoostMultiplier;
	}

	// ECU tune
	const float TuneMultipliers[] = { 0.90f, 0.95f, 1.00f, 1.02f };
	const int32 TuneIndex = FMath::Clamp(Engine.TuneLevel, 0, 3);
	BaseTQ *= TuneMultipliers[TuneIndex];

	return BaseTQ;
}

FMGPowerCurve UMGStatCalculator::CalculatePowerCurve(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel)
{
	FMGPowerCurve Curve;

	if (!BaseModel)
	{
		return Curve;
	}

	const float PeakHP = CalculateHorsepower(Engine, BaseModel);
	const float PeakTQ = CalculateTorque(Engine, BaseModel);
	const int32 Redline = CalculateRedline(Engine, BaseModel);

	// Generate curve points
	// Torque typically peaks before HP
	const int32 TorquePeakRPM = Redline * 0.55f; // ~55% of redline
	const int32 HPPeakRPM = Redline * 0.85f;     // ~85% of redline

	// Generate points every 500 RPM
	for (int32 RPM = 1000; RPM <= Redline; RPM += 500)
	{
		FMGPowerCurvePoint Point;
		Point.RPM = RPM;

		// Simplified torque curve (bell curve around peak)
		const float TorqueProgress = (float)(RPM - 1000) / (float)(Redline - 1000);
		const float TorqueCurveFactor = FMath::Sin(TorqueProgress * PI);
		Point.Torque = PeakTQ * FMath::Lerp(0.6f, 1.0f, TorqueCurveFactor);

		// HP = (TQ * RPM) / 5252
		Point.Horsepower = (Point.Torque * RPM) / 5252.0f;

		// Cap HP at calculated peak
		Point.Horsepower = FMath::Min(Point.Horsepower, PeakHP);

		Curve.CurvePoints.Add(Point);
	}

	// Find actual peaks from curve
	Curve.PeakHP = PeakHP;
	Curve.PeakHPRPM = HPPeakRPM;
	Curve.PeakTorque = PeakTQ;
	Curve.PeakTorqueRPM = TorquePeakRPM;
	Curve.Redline = Redline;

	return Curve;
}

int32 UMGStatCalculator::CalculateRedline(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel)
{
	if (!BaseModel)
	{
		return 6000;
	}

	int32 Redline = BaseModel->BaseRedline;

	// Internal upgrades can increase redline
	FMGPartModifiers Modifiers = GetCombinedModifiers(Engine);
	Redline += Modifiers.RedlineBonus;

	return FMath::Clamp(Redline, 4000, 12000);
}

float UMGStatCalculator::CalculateWeight(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
	if (!BaseModel)
	{
		return 1500.0f;
	}

	float Weight = BaseModel->BaseWeightKG;

	// Get weight deltas from parts
	FMGPartModifiers Modifiers = GetCombinedModifiers(Vehicle.Engine);
	Weight += Modifiers.WeightDelta;

	// Additional weight from forced induction
	if (Vehicle.Engine.ForcedInduction.Type != EMGForcedInductionType::None)
	{
		Weight += 25.0f; // Turbo system weight
	}

	// Nitrous system
	if (Vehicle.Engine.Nitrous.bInstalled)
	{
		Weight += Vehicle.Engine.Nitrous.BottleSizeLbs * 0.453592f; // lbs to kg
	}

	return FMath::Max(Weight, 500.0f);
}

float UMGStatCalculator::CalculateWeightDistribution(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
	if (!BaseModel)
	{
		return 0.55f;
	}

	// Base distribution
	float FrontWeight = BaseModel->BaseWeightDistributionFront;

	// Engine position affects distribution
	// Front engine = more front weight
	// Rear engine = more rear weight

	return FMath::Clamp(FrontWeight, 0.35f, 0.70f);
}

float UMGStatCalculator::CalculateFrontGrip(const FMGVehicleData& Vehicle)
{
	float Grip = GetTireCompoundGrip(Vehicle.WheelsTires.FrontTireCompound);

	// Apply tire width factor (wider = more grip)
	const float WidthFactor = Vehicle.WheelsTires.FrontTireWidth / 245.0f; // Normalize to 245mm
	Grip *= FMath::Lerp(0.9f, 1.1f, FMath::Clamp(WidthFactor, 0.0f, 1.5f));

	// Apply condition
	Grip *= FMath::Lerp(0.5f, 1.0f, Vehicle.WheelsTires.FrontTireCondition / 100.0f);

	// Aero downforce (at speed)
	Grip += Vehicle.Aero.FrontDownforceCoefficient * 0.05f;

	return Grip;
}

float UMGStatCalculator::CalculateRearGrip(const FMGVehicleData& Vehicle)
{
	float Grip = GetTireCompoundGrip(Vehicle.WheelsTires.RearTireCompound);

	const float WidthFactor = Vehicle.WheelsTires.RearTireWidth / 275.0f;
	Grip *= FMath::Lerp(0.9f, 1.1f, FMath::Clamp(WidthFactor, 0.0f, 1.5f));

	Grip *= FMath::Lerp(0.5f, 1.0f, Vehicle.WheelsTires.RearTireCondition / 100.0f);

	Grip += Vehicle.Aero.RearDownforceCoefficient * 0.05f;

	return Grip;
}

float UMGStatCalculator::CalculateHandlingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
	// Base handling from weight and distribution
	const float Weight = CalculateWeight(Vehicle, BaseModel);
	const float Distribution = CalculateWeightDistribution(Vehicle, BaseModel);

	// Lighter is better
	const float WeightScore = FMath::GetMappedRangeValueClamped(FVector2D(1800.0f, 1000.0f), FVector2D(0.0f, 100.0f), Weight);

	// Balanced distribution is better
	const float DistributionScore = 100.0f - FMath::Abs(Distribution - 0.5f) * 200.0f;

	// Grip average
	const float GripScore = ((CalculateFrontGrip(Vehicle) + CalculateRearGrip(Vehicle)) / 2.0f) * 100.0f;

	// Suspension factor (better suspension = better handling)
	// For now, assume mid-range score
	const float SuspensionScore = 70.0f;

	// Weighted average
	const float Handling = (WeightScore * 0.25f) + (DistributionScore * 0.15f) + (GripScore * 0.35f) + (SuspensionScore * 0.25f);

	return FMath::Clamp(Handling, 0.0f, 100.0f);
}

float UMGStatCalculator::CalculateBrakingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
	// Rotor size factor
	const float FrontRotorFactor = Vehicle.Brakes.FrontRotorDiameterMM / 350.0f;
	const float RearRotorFactor = Vehicle.Brakes.RearRotorDiameterMM / 320.0f;

	// Piston count factor
	const float FrontPistonFactor = Vehicle.Brakes.FrontPistonCount / 6.0f;
	const float RearPistonFactor = Vehicle.Brakes.RearPistonCount / 4.0f;

	// Combined brake score
	const float BrakeHardware = ((FrontRotorFactor + RearRotorFactor) / 2.0f * 0.5f) +
	                           ((FrontPistonFactor + RearPistonFactor) / 2.0f * 0.5f);

	// Weight affects braking (lighter = easier to stop)
	const float Weight = CalculateWeight(Vehicle, BaseModel);
	const float WeightFactor = FMath::GetMappedRangeValueClamped(FVector2D(1800.0f, 1000.0f), FVector2D(0.7f, 1.0f), Weight);

	// Tire grip affects braking
	const float GripFactor = (CalculateFrontGrip(Vehicle) + CalculateRearGrip(Vehicle)) / 2.0f;

	const float Rating = BrakeHardware * WeightFactor * GripFactor * 100.0f;

	return FMath::Clamp(Rating, 0.0f, 100.0f);
}

float UMGStatCalculator::EstimateZeroTo60(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain)
{
	// Simple estimation formula
	// Based on power-to-weight ratio and drivetrain
	const float PTW = Stats.PowerToWeightRatio;

	if (PTW <= 0.0f)
	{
		return 99.0f;
	}

	// Base time from power-to-weight
	float Time = 15.0f / PTW; // Rough approximation

	// Drivetrain factor (AWD is fastest off the line)
	switch (Drivetrain.DrivetrainType)
	{
		case EMGDrivetrainType::AWD:
			Time *= 0.90f;
			break;
		case EMGDrivetrainType::RWD:
			Time *= 1.0f;
			break;
		case EMGDrivetrainType::FWD:
			Time *= 1.05f; // Torque steer, wheel spin
			break;
	}

	// Grip factor
	Time *= FMath::Lerp(1.2f, 0.9f, FMath::Clamp(Stats.GripRear, 0.0f, 1.5f));

	return FMath::Clamp(Time, 2.5f, 15.0f);
}

float UMGStatCalculator::EstimateZeroTo100(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain)
{
	const float ZeroTo60 = EstimateZeroTo60(Stats, Drivetrain);

	// 0-100 is typically 2.5-3x the 0-60 time
	return ZeroTo60 * 2.7f;
}

float UMGStatCalculator::EstimateQuarterMile(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain)
{
	// Quarter mile formula approximation
	// ET = 5.825 * (Weight/HP)^0.333
	const float WeightLbs = Stats.WeightKG * 2.20462f;

	if (Stats.Horsepower <= 0.0f)
	{
		return 20.0f;
	}

	float ET = 5.825f * FMath::Pow(WeightLbs / Stats.Horsepower, 0.333f);

	// Drivetrain modifier
	switch (Drivetrain.DrivetrainType)
	{
		case EMGDrivetrainType::AWD:
			ET *= 0.97f;
			break;
		case EMGDrivetrainType::RWD:
			ET *= 1.0f;
			break;
		case EMGDrivetrainType::FWD:
			ET *= 1.03f;
			break;
	}

	return FMath::Clamp(ET, 8.0f, 20.0f);
}

float UMGStatCalculator::EstimateQuarterMileTrap(const FMGVehicleStats& Stats)
{
	// Trap speed formula
	// MPH = (HP / (Weight_lbs / 2000))^0.333 * 225
	const float WeightLbs = Stats.WeightKG * 2.20462f;

	if (WeightLbs <= 0.0f)
	{
		return 0.0f;
	}

	const float TrapMPH = FMath::Pow(Stats.Horsepower / (WeightLbs / 2000.0f), 0.333f) * 225.0f / 10.0f;

	return FMath::Clamp(TrapMPH, 60.0f, 200.0f);
}

float UMGStatCalculator::EstimateTopSpeed(const FMGVehicleStats& Stats, const FMGDrivetrainConfiguration& Drivetrain, const FMGAeroConfiguration& Aero)
{
	// Top speed limited by power vs drag
	// Simplified: TopSpeed = k * (HP / Cd)^0.333

	const float DragCoef = FMath::Max(Aero.DragCoefficient, 0.25f);
	float TopSpeed = FMath::Pow(Stats.Horsepower / DragCoef, 0.333f) * 15.0f;

	// Gear limit - check if final gear can reach this speed
	if (Drivetrain.GearRatios.Num() > 0)
	{
		const float TopGear = Drivetrain.GearRatios.Last();
		const float FinalDrive = Drivetrain.FinalDriveRatio;
		// Higher ratio = lower top speed but better acceleration
		const float GearLimitFactor = 1.0f / (TopGear * FinalDrive) * 3.0f;
		TopSpeed = FMath::Min(TopSpeed, 300.0f * GearLimitFactor);
	}

	return FMath::Clamp(TopSpeed, 80.0f, 280.0f);
}

float UMGStatCalculator::CalculatePerformanceIndex(const FMGVehicleStats& Stats)
{
	// PI Formula combines multiple factors

	// Power component (0-400 points)
	const float PowerScore = FMath::GetMappedRangeValueClamped(
		FVector2D(100.0f, 1000.0f), FVector2D(100.0f, 400.0f), Stats.Horsepower);

	// Weight component (0-200 points, inverted - lighter is better)
	const float WeightScore = FMath::GetMappedRangeValueClamped(
		FVector2D(2000.0f, 800.0f), FVector2D(50.0f, 200.0f), Stats.WeightKG);

	// Handling component (0-200 points)
	const float HandlingScore = Stats.HandlingRating * 2.0f;

	// Braking component (0-100 points)
	const float BrakingScore = Stats.BrakingRating;

	// Grip component (0-100 points)
	const float GripScore = ((Stats.GripFront + Stats.GripRear) / 2.0f) * 100.0f;

	const float PI = PowerScore + WeightScore + HandlingScore + BrakingScore + GripScore;

	return FMath::Clamp(PI, 100.0f, 999.0f);
}

EMGPerformanceClass UMGStatCalculator::GetPerformanceClass(float PerformanceIndex)
{
	if (PerformanceIndex >= 901.0f) return EMGPerformanceClass::X;
	if (PerformanceIndex >= 750.0f) return EMGPerformanceClass::S;
	if (PerformanceIndex >= 600.0f) return EMGPerformanceClass::A;
	if (PerformanceIndex >= 450.0f) return EMGPerformanceClass::B;
	if (PerformanceIndex >= 300.0f) return EMGPerformanceClass::C;
	return EMGPerformanceClass::D;
}

float UMGStatCalculator::CalculateVehicleValue(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
	if (!BaseModel)
	{
		return 0.0f;
	}

	// Base value with depreciation
	float Value = BaseModel->BasePriceMSRP;

	// Add parts value
	Value += CalculatePartsValue(Vehicle);

	// Condition modifier
	float AverageCondition = 100.0f;
	if (Vehicle.PartConditions.Num() > 0)
	{
		float Sum = 0.0f;
		for (const auto& Pair : Vehicle.PartConditions)
		{
			Sum += Pair.Value;
		}
		AverageCondition = Sum / Vehicle.PartConditions.Num();
	}
	Value *= FMath::Lerp(0.5f, 1.0f, AverageCondition / 100.0f);

	// Mileage depreciation
	const float MileageDepreciation = FMath::Min(Vehicle.Mileage / 100000.0f, 0.3f);
	Value *= (1.0f - MileageDepreciation);

	// Accident history penalty
	Value *= FMath::Pow(0.9f, Vehicle.AccidentCount);

	// Race history can add value (proven performer)
	if (Vehicle.RaceHistory.Wins > 10)
	{
		Value *= 1.1f;
	}

	return FMath::Max(Value, 500.0f);
}

float UMGStatCalculator::CalculatePartsValue(const FMGVehicleData& Vehicle)
{
	float PartsValue = 0.0f;

	// Would iterate through all installed parts and sum their values
	// For now, estimate based on upgrade level

	return PartsValue;
}

FMGVehicleStats UMGStatCalculator::CalculateAllStats(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
	FMGVehicleStats Stats;

	if (!BaseModel)
	{
		return Stats;
	}

	// Power
	Stats.Horsepower = CalculateHorsepower(Vehicle.Engine, BaseModel);
	Stats.Torque = CalculateTorque(Vehicle.Engine, BaseModel);
	Stats.Redline = CalculateRedline(Vehicle.Engine, BaseModel);
	Stats.BoostPSI = Vehicle.Engine.ForcedInduction.MaxBoostPSI;

	// Weight
	Stats.WeightKG = CalculateWeight(Vehicle, BaseModel);
	Stats.WeightDistributionFront = CalculateWeightDistribution(Vehicle, BaseModel);
	Stats.PowerToWeightRatio = Stats.Horsepower / (Stats.WeightKG * 2.20462f) * 1000.0f; // HP per 1000 lbs

	// Grip
	Stats.GripFront = CalculateFrontGrip(Vehicle);
	Stats.GripRear = CalculateRearGrip(Vehicle);

	// Ratings
	Stats.HandlingRating = CalculateHandlingRating(Vehicle, BaseModel);
	Stats.BrakingRating = CalculateBrakingRating(Vehicle, BaseModel);

	// Performance predictions
	Stats.ZeroTo60MPH = EstimateZeroTo60(Stats, Vehicle.Drivetrain);
	Stats.ZeroTo100MPH = EstimateZeroTo100(Stats, Vehicle.Drivetrain);
	Stats.QuarterMileTime = EstimateQuarterMile(Stats, Vehicle.Drivetrain);
	Stats.QuarterMileTrapMPH = EstimateQuarterMileTrap(Stats);
	Stats.TopSpeedMPH = EstimateTopSpeed(Stats, Vehicle.Drivetrain, Vehicle.Aero);

	// Classification
	Stats.PerformanceIndex = CalculatePerformanceIndex(Stats);
	Stats.PerformanceClass = GetPerformanceClass(Stats.PerformanceIndex);

	// Value
	Stats.EstimatedValue = CalculateVehicleValue(Vehicle, BaseModel);

	// Reliability (placeholder)
	Stats.ReliabilityRating = 100.0f;

	return Stats;
}

UMGPartData* UMGStatCalculator::GetPartData(FName PartID)
{
	// Part database lookup through asset manager
	// For now returns nullptr - parts can be created as data assets in Content/Parts/
	// Usage: Create UMGPartData assets and reference by PartID

	if (PartID == NAME_None)
	{
		return nullptr;
	}

	// Try to find the asset through asset registry
	// In production, parts would be loaded via FPrimaryAssetId lookup
	// For testing, the vehicle factory provides programmatic part data

	return nullptr;
}

FMGPartModifiers UMGStatCalculator::GetCombinedModifiers(const FMGEngineConfiguration& Engine)
{
	FMGPartModifiers Combined;
	Combined.PowerMultiplier = 1.0f;
	Combined.TorqueMultiplier = 1.0f;
	Combined.WeightDelta = 0.0f;
	Combined.RedlineBonus = 0;

	// Calculate modifiers based on installed upgrade tiers
	// Higher tiers = better performance but possibly less reliable

	// Air filter effects (affects breathing/power)
	switch (Engine.AirFilterTier)
	{
		case EMGPartTier::Street:
			Combined.PowerMultiplier *= 1.02f;
			break;
		case EMGPartTier::Sport:
			Combined.PowerMultiplier *= 1.04f;
			break;
		case EMGPartTier::Race:
			Combined.PowerMultiplier *= 1.06f;
			break;
		case EMGPartTier::Pro:
		case EMGPartTier::Legendary:
			Combined.PowerMultiplier *= 1.08f;
			Combined.RedlineBonus += 200;
			break;
		default:
			break;
	}

	// Exhaust effects (power and weight)
	switch (Engine.ExhaustTier)
	{
		case EMGPartTier::Street:
			Combined.PowerMultiplier *= 1.03f;
			Combined.WeightDelta -= 5.0f;
			break;
		case EMGPartTier::Sport:
			Combined.PowerMultiplier *= 1.06f;
			Combined.WeightDelta -= 10.0f;
			break;
		case EMGPartTier::Race:
			Combined.PowerMultiplier *= 1.10f;
			Combined.TorqueMultiplier *= 1.05f;
			Combined.WeightDelta -= 20.0f;
			break;
		case EMGPartTier::Pro:
		case EMGPartTier::Legendary:
			Combined.PowerMultiplier *= 1.15f;
			Combined.TorqueMultiplier *= 1.08f;
			Combined.WeightDelta -= 30.0f;
			break;
		default:
			break;
	}

	// Camshaft effects (power curve and redline)
	switch (Engine.CamshaftTier)
	{
		case EMGPartTier::Street:
			Combined.PowerMultiplier *= 1.02f;
			Combined.RedlineBonus += 100;
			break;
		case EMGPartTier::Sport:
			Combined.PowerMultiplier *= 1.05f;
			Combined.RedlineBonus += 300;
			break;
		case EMGPartTier::Race:
			Combined.PowerMultiplier *= 1.08f;
			Combined.RedlineBonus += 500;
			break;
		case EMGPartTier::Pro:
		case EMGPartTier::Legendary:
			Combined.PowerMultiplier *= 1.12f;
			Combined.RedlineBonus += 800;
			break;
		default:
			break;
	}

	// Internal upgrades (pistons/rods - supports more power)
	switch (Engine.InternalsTier)
	{
		case EMGPartTier::Sport:
			Combined.PowerMultiplier *= 1.02f;
			Combined.WeightDelta += 2.0f; // Forged is slightly heavier
			break;
		case EMGPartTier::Race:
			Combined.PowerMultiplier *= 1.05f;
			Combined.RedlineBonus += 300;
			break;
		case EMGPartTier::Pro:
		case EMGPartTier::Legendary:
			Combined.PowerMultiplier *= 1.08f;
			Combined.RedlineBonus += 500;
			break;
		default:
			break;
	}

	return Combined;
}

bool UMGStatCalculator::IsPartCompatible(const UMGPartData* Part, FName VehicleModelID, const FMGVehicleData& Vehicle)
{
	if (!Part)
	{
		return false;
	}

	// Check vehicle compatibility
	if (Part->CompatibleVehicles.Num() > 0 && !Part->CompatibleVehicles.Contains(VehicleModelID))
	{
		return false;
	}

	// Check required parts
	for (const FName& RequiredPart : Part->RequiredParts)
	{
		// TODO: Check if required part is installed
	}

	// Check incompatible parts
	for (const FName& IncompatiblePart : Part->IncompatibleParts)
	{
		// TODO: Check if incompatible part is installed
	}

	return true;
}

float UMGStatCalculator::GetTireCompoundGrip(EMGTireCompound Compound)
{
	switch (Compound)
	{
		case EMGTireCompound::Economy:     return 0.70f;
		case EMGTireCompound::AllSeason:   return 0.78f;
		case EMGTireCompound::Sport:       return 0.85f;
		case EMGTireCompound::Performance: return 0.95f;
		case EMGTireCompound::SemiSlick:   return 1.05f;
		case EMGTireCompound::Slick:       return 1.15f;
		case EMGTireCompound::DragRadial:  return 1.10f;
		case EMGTireCompound::Drift:       return 0.80f;
		default:                           return 1.00f;
	}
}

float UMGStatCalculator::GetWetGripModifier(EMGTireCompound Compound)
{
	switch (Compound)
	{
		case EMGTireCompound::Economy:     return 0.65f;
		case EMGTireCompound::AllSeason:   return 0.72f;
		case EMGTireCompound::Sport:       return 0.70f;
		case EMGTireCompound::Performance: return 0.65f;
		case EMGTireCompound::SemiSlick:   return 0.45f;
		case EMGTireCompound::Slick:       return 0.25f; // Dangerous in wet
		case EMGTireCompound::DragRadial:  return 0.40f;
		case EMGTireCompound::Drift:       return 0.55f;
		default:                           return 0.70f;
	}
}

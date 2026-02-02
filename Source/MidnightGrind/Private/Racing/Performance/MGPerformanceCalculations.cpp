// Copyright Midnight Grind. All Rights Reserved.

#include "Racing/Performance/MGPerformanceCalculations.h"
#include "Vehicle/MG_VHCL_Data.h"
#include "Data/MGVehicleModelData.h"

/**
 * Implementation of namespaced performance calculation logic.
 * 
 * This file contains the actual calculation implementations extracted from
 * UMGStatCalculator. The logic is now pure C++ with no UObject dependencies
 * in the calculation paths themselves.
 * 
 * Note: This is a PARTIAL implementation serving as a template.
 * The full implementation should be completed by extracting logic from
 * the existing UMGStatCalculator methods.
 */

namespace MidnightGrind::Racing::Performance
{

// =============================================================================
// FPowerCalculator Implementation
// =============================================================================

float FPowerCalculator::CalculateHorsepower(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel)
{
    if (!BaseModel)
    {
        UE_LOG(LogTemp, Warning, TEXT("CalculateHorsepower: BaseModel is null"));
        return 0.0f;
    }

    // Start with base horsepower from the model
    float HP = BaseModel->BaseHorsepower;

    // Apply multipliers from installed parts
    float PowerMultiplier = GetCombinedPowerMultiplier(Engine);
    HP *= PowerMultiplier;

    // Add boost contribution if forced induction is equipped
    if (Engine.bHasTurbo || Engine.bHasSupercharger)
    {
        HP += GetBoostPowerContribution(Engine);
    }

    // Apply tune level multiplier (street/race/competition tune affects final output)
    HP *= GetTuneMultiplier(Engine.TuneLevel);

    return HP;
}

float FPowerCalculator::CalculateTorque(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel)
{
    if (!BaseModel)
    {
        UE_LOG(LogTemp, Warning, TEXT("CalculateTorque: BaseModel is null"));
        return 0.0f;
    }

    // Start with base torque
    float Torque = BaseModel->BaseTorque;

    // Apply multipliers (torque follows similar pattern to HP but with different coefficients)
    float TorqueMultiplier = GetCombinedTorqueMultiplier(Engine);
    Torque *= TorqueMultiplier;

    // Forced induction adds significant torque
    if (Engine.bHasTurbo || Engine.bHasSupercharger)
    {
        // Boost adds more torque than HP proportionally
        float BoostTorqueMultiplier = 1.0f + (Engine.BoostPressure / 14.7f) * 0.6f;
        Torque *= BoostTorqueMultiplier;
    }

    // Tune level
    Torque *= GetTuneMultiplier(Engine.TuneLevel);

    return Torque;
}

FMGPowerCurve FPowerCalculator::CalculatePowerCurve(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel)
{
    FMGPowerCurve Curve;

    if (!BaseModel)
    {
        return Curve;
    }

    // Calculate key values
    float PeakHP = CalculateHorsepower(Engine, BaseModel);
    float PeakTorque = CalculateTorque(Engine, BaseModel);
    int32 Redline = CalculateRedline(Engine, BaseModel);

    // Set peak values
    Curve.PeakHorsepower = PeakHP;
    Curve.PeakTorque = PeakTorque;
    Curve.PeakHorsepowerRPM = BaseModel->PeakHorsepowerRPM;
    Curve.PeakTorqueRPM = BaseModel->PeakTorqueRPM;
    Curve.RedlineRPM = Redline;

    // Generate curve points (typically 20-30 points from idle to redline)
    const int32 NumPoints = 25;
    const int32 IdleRPM = BaseModel->IdleRPM;
    const int32 RPMRange = Redline - IdleRPM;

    Curve.RPMPoints.Reserve(NumPoints);
    Curve.HorsepowerPoints.Reserve(NumPoints);
    Curve.TorquePoints.Reserve(NumPoints);

    for (int32 i = 0; i < NumPoints; ++i)
    {
        float Alpha = static_cast<float>(i) / static_cast<float>(NumPoints - 1);
        int32 CurrentRPM = IdleRPM + FMath::RoundToInt(Alpha * RPMRange);

        // Simple power curve model (real dyno curves are more complex)
        // Torque peaks early, HP peaks later
        float TorqueAlpha = FMath::Abs((CurrentRPM - Curve.PeakTorqueRPM) / static_cast<float>(RPMRange));
        float HPAlpha = FMath::Abs((CurrentRPM - Curve.PeakHorsepowerRPM) / static_cast<float>(RPMRange));

        // Use bell curves centered on peak RPMs
        float TorqueFactor = FMath::Exp(-TorqueAlpha * TorqueAlpha * 4.0f);
        float HPFactor = FMath::Exp(-HPAlpha * HPAlpha * 4.0f);

        Curve.RPMPoints.Add(CurrentRPM);
        Curve.TorquePoints.Add(PeakTorque * TorqueFactor);
        Curve.HorsepowerPoints.Add(PeakHP * HPFactor);
    }

    return Curve;
}

int32 FPowerCalculator::CalculateRedline(const FMGEngineConfiguration& Engine, const UMGVehicleModelData* BaseModel)
{
    if (!BaseModel)
    {
        return 7000; // Default safe redline
    }

    int32 Redline = BaseModel->RedlineRPM;

    // Upgraded internals increase safe RPM
    // Stock internals: typically 6000-8000 RPM
    // Forged internals: can push 9000+ RPM
    if (Engine.InternalsPartID != NAME_None)
    {
        // TODO: Look up actual part data and get RPM bonus
        // For now, use simplified logic
        Redline += 500; // +500 RPM for upgraded internals
    }

    return Redline;
}

float FPowerCalculator::GetCombinedPowerMultiplier(const FMGEngineConfiguration& Engine)
{
    float Multiplier = 1.0f;

    // Intake adds 5-15% power
    if (Engine.IntakePartID != NAME_None)
    {
        Multiplier *= 1.10f; // +10% average
    }

    // Exhaust adds 5-20% power
    if (Engine.ExhaustPartID != NAME_None)
    {
        Multiplier *= 1.12f; // +12% average
    }

    // Cams add 8-25% power
    if (Engine.CamsPartID != NAME_None)
    {
        Multiplier *= 1.15f; // +15% average
    }

    // Headers add 3-10% power
    if (Engine.HeadersPartID != NAME_None)
    {
        Multiplier *= 1.06f; // +6% average
    }

    // Note: In full implementation, look up actual part data from part database
    // Each part should have its own PowerMultiplier value

    return Multiplier;
}

float FPowerCalculator::GetCombinedTorqueMultiplier(const FMGEngineConfiguration& Engine)
{
    // Similar to power but with different coefficients
    // Intake/exhaust affect torque differently than HP
    float Multiplier = 1.0f;

    if (Engine.IntakePartID != NAME_None)
    {
        Multiplier *= 1.08f; // Slightly less torque gain than HP
    }

    if (Engine.ExhaustPartID != NAME_None)
    {
        Multiplier *= 1.10f;
    }

    if (Engine.CamsPartID != NAME_None)
    {
        Multiplier *= 1.12f; // Cams can actually reduce low-end torque but increase peak
    }

    return Multiplier;
}

float FPowerCalculator::GetBoostPowerContribution(const FMGEngineConfiguration& Engine)
{
    // Simplified boost model
    // Rule of thumb: Each PSI of boost adds ~10-15 HP on average
    // This varies greatly based on engine displacement and tuning
    
    float BoostPSI = Engine.BoostPressure;
    constexpr float HP_PER_PSI = 12.0f;

    return BoostPSI * HP_PER_PSI;
}

float FPowerCalculator::GetTuneMultiplier(EMGTuneLevel TuneLevel)
{
    // Tune level affects timing, fuel delivery, boost control
    switch (TuneLevel)
    {
        case EMGTuneLevel::Stock:
            return 1.0f;
        
        case EMGTuneLevel::Street:
            return 1.08f; // +8% for street tune
        
        case EMGTuneLevel::Sport:
            return 1.15f; // +15% for sport tune
        
        case EMGTuneLevel::Race:
            return 1.25f; // +25% for race tune
        
        case EMGTuneLevel::Competition:
            return 1.35f; // +35% for competition tune (requires race gas)
        
        default:
            return 1.0f;
    }
}

// =============================================================================
// FWeightCalculator Implementation
// =============================================================================

float FWeightCalculator::CalculateTotalWeight(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
    if (!BaseModel)
    {
        return 1500.0f; // Default weight if no model data
    }

    float TotalWeight = BaseModel->BaseWeight;

    // Add/subtract weight from parts
    float PartsDelta = GetPartsWeightDelta(Vehicle);
    TotalWeight += PartsDelta;

    // Clamp to reasonable range (can't be lighter than 50% stock or heavier than 200% stock)
    float MinWeight = BaseModel->BaseWeight * 0.5f;
    float MaxWeight = BaseModel->BaseWeight * 2.0f;
    TotalWeight = FMath::Clamp(TotalWeight, MinWeight, MaxWeight);

    return TotalWeight;
}

float FWeightCalculator::CalculateWeightDistribution(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
    if (!BaseModel)
    {
        return 0.55f; // Default 55/45 split
    }

    // Start with base distribution (depends on engine position and drivetrain)
    float FrontWeightRatio = BaseModel->BaseWeightDistribution;

    // Apply modification shifts
    float Shift = GetWeightDistributionShift(Vehicle, BaseModel);
    FrontWeightRatio += Shift;

    // Clamp to realistic range (35% to 65% front)
    FrontWeightRatio = FMath::Clamp(FrontWeightRatio, 0.35f, 0.65f);

    return FrontWeightRatio;
}

float FWeightCalculator::GetPartsWeightDelta(const FMGVehicleData& Vehicle)
{
    float WeightDelta = 0.0f;

    // TODO: Look up actual part weights from part database
    // This is simplified example logic

    // Carbon fiber body panels (-30 to -100 kg)
    // Lightweight wheels (-5 to -15 kg per wheel)
    // Roll cage (+40 to +80 kg)
    // Sound system (+20 to +60 kg)
    // NOS bottle (+10 kg per bottle)

    // For now, just return 0 (no change from stock)
    // In full implementation, iterate through all installed parts and sum weight deltas

    return WeightDelta;
}

float FWeightCalculator::GetWeightDistributionShift(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
    float Shift = 0.0f;

    // Heavy front-mounted parts shift weight forward
    // - Front mount intercooler: +2% front
    // - Heavy battery: varies by position

    // Weight reductions shift depending on location
    // - Rear seat delete: -2% front (reduces rear weight)
    // - Lightweight exhaust: -1% front (reduces rear weight)

    // TODO: Implement based on actual part locations

    return Shift;
}

// =============================================================================
// FHandlingCalculator Implementation
// =============================================================================

float FHandlingCalculator::CalculateFrontGrip(const FMGVehicleData& Vehicle)
{
    // Base grip from tire compound
    float Grip = GetTireCompoundGrip(Vehicle.Tires.FrontCompound);

    // Width factor (wider = more grip)
    float WidthFactor = Vehicle.Tires.FrontWidth / 225.0f; // Normalized to 225mm
    Grip *= FMath::Lerp(0.9f, 1.1f, FMath::Clamp(WidthFactor, 0.8f, 1.3f) - 0.8f);

    // Suspension contribution
    Grip *= GetSuspensionGripModifier(Vehicle.Suspension);

    // Alignment contribution (camber/toe)
    Grip *= GetAlignmentGripModifier(Vehicle.Suspension.FrontCamber, Vehicle.Suspension.FrontToe);

    return Grip;
}

float FHandlingCalculator::CalculateRearGrip(const FMGVehicleData& Vehicle)
{
    // Similar to front grip calculation
    float Grip = GetTireCompoundGrip(Vehicle.Tires.RearCompound);

    float WidthFactor = Vehicle.Tires.RearWidth / 245.0f; // Normalized to 245mm
    Grip *= FMath::Lerp(0.9f, 1.1f, FMath::Clamp(WidthFactor, 0.8f, 1.3f) - 0.8f);

    Grip *= GetSuspensionGripModifier(Vehicle.Suspension);
    Grip *= GetAlignmentGripModifier(Vehicle.Suspension.RearCamber, Vehicle.Suspension.RearToe);

    return Grip;
}

float FHandlingCalculator::CalculateHandlingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
    if (!BaseModel)
    {
        return 50.0f;
    }

    // Handling rating is composite of multiple factors
    float Rating = 50.0f; // Start at middle

    // Grip balance (ideal is balanced front/rear)
    float FrontGrip = CalculateFrontGrip(Vehicle);
    float RearGrip = CalculateRearGrip(Vehicle);
    float GripBalance = FMath::Abs(FrontGrip - RearGrip);
    float BalanceScore = FMath::Lerp(25.0f, 0.0f, FMath::Clamp(GripBalance / 0.3f, 0.0f, 1.0f));
    Rating += BalanceScore;

    // Weight distribution (50/50 is ideal)
    float WeightDist = FWeightCalculator::CalculateWeightDistribution(Vehicle, BaseModel);
    float DistBalance = FMath::Abs(WeightDist - 0.5f);
    float DistScore = FMath::Lerp(15.0f, 0.0f, DistBalance / 0.15f);
    Rating += DistScore;

    // Power-to-weight ratio (too much power makes handling harder)
    // Ideal is around 200-300 HP per 1000kg
    // More power requires more skill

    // TODO: Complete rating calculation with aerodynamics, suspension stiffness, etc.

    return FMath::Clamp(Rating, 0.0f, 100.0f);
}

float FHandlingCalculator::CalculateBrakingRating(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
    // TODO: Implement braking rating calculation
    // Consider:
    // - Rotor size
    // - Caliper piston count
    // - Pad compound
    // - Vehicle weight
    // - Tire grip

    return 70.0f; // Placeholder
}

float FHandlingCalculator::GetTireCompoundGrip(EMGTireCompound Compound)
{
    // Base grip coefficients for each compound
    switch (Compound)
    {
        case EMGTireCompound::Economy:
            return 0.75f;
        
        case EMGTireCompound::Comfort:
            return 0.85f;
        
        case EMGTireCompound::AllSeason:
            return 0.90f;
        
        case EMGTireCompound::Summer:
            return 1.00f;
        
        case EMGTireCompound::Sport:
            return 1.10f;
        
        case EMGTireCompound::SuperSport:
            return 1.20f;
        
        case EMGTireCompound::Race:
            return 1.30f;
        
        case EMGTireCompound::Slick:
            return 1.40f;
        
        case EMGTireCompound::Drag:
            return 1.50f; // Highest straight-line grip
        
        default:
            return 1.0f;
    }
}

float FHandlingCalculator::GetSuspensionGripModifier(const FMGSuspensionConfiguration& Suspension)
{
    // TODO: Calculate based on suspension stiffness, damping, anti-roll bars
    // Stiffer suspension = better handling but harsher ride
    // Softer suspension = more comfort but more body roll
    
    return 1.0f; // Placeholder (no modification)
}

float FHandlingCalculator::GetAlignmentGripModifier(float Camber, float Toe)
{
    // Negative camber increases cornering grip (up to optimal angle)
    float CamberModifier = 1.0f;
    if (Camber < 0.0f)
    {
        // Optimal camber is around -2.5 degrees
        float CamberDelta = FMath::Abs(Camber + 2.5f);
        CamberModifier = 1.0f + (0.1f - FMath::Min(CamberDelta * 0.02f, 0.1f));
    }

    // Toe affects stability vs turn-in
    // Small amounts are beneficial, excessive toe hurts grip
    float ToeModifier = 1.0f - FMath::Min(FMath::Abs(Toe) * 0.01f, 0.05f);

    return CamberModifier * ToeModifier;
}

// =============================================================================
// FPerformancePredictor Implementation (Stub)
// =============================================================================

float FPerformancePredictor::EstimateZeroTo60(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain)
{
    // TODO: Implement physics-based 0-60 prediction
    // Factors: power-to-weight, drivetrain type, tire grip, gearing
    
    return 5.0f; // Placeholder
}

float FPerformancePredictor::EstimateZeroTo100(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain)
{
    // TODO: Implement 0-100 prediction
    return 12.0f; // Placeholder
}

float FPerformancePredictor::EstimateQuarterMile(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain)
{
    // TODO: Implement quarter mile ET prediction
    return 13.5f; // Placeholder
}

float FPerformancePredictor::EstimateQuarterMileTrap(const FMGVehicleSpecs& Stats)
{
    // TODO: Implement trap speed prediction
    return 105.0f; // Placeholder
}

float FPerformancePredictor::EstimateTopSpeed(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain, const FMGAeroConfiguration& Aero)
{
    // TODO: Implement top speed calculation
    // Top speed = when power equals drag
    return 180.0f; // Placeholder
}

float FPerformancePredictor::GetDrivetrainEfficiency(EMGDrivetrainType DrivetrainType)
{
    // Power loss through drivetrain
    switch (DrivetrainType)
    {
        case EMGDrivetrainType::FWD:
            return 0.92f; // 8% loss
        
        case EMGDrivetrainType::RWD:
        case EMGDrivetrainType::MR:
        case EMGDrivetrainType::RR:
            return 0.85f; // 15% loss (longer driveshaft)
        
        case EMGDrivetrainType::AWD:
        case EMGDrivetrainType::F4WD:
            return 0.80f; // 20% loss (most complex)
        
        default:
            return 0.85f;
    }
}

float FPerformancePredictor::GetLaunchTractionCoefficient(EMGDrivetrainType DrivetrainType, float RearGrip)
{
    // AWD has best launch traction, FWD worst (no weight on front during launch)
    switch (DrivetrainType)
    {
        case EMGDrivetrainType::FWD:
            return RearGrip * 0.7f; // Weight shifts off front wheels
        
        case EMGDrivetrainType::RWD:
        case EMGDrivetrainType::MR:
        case EMGDrivetrainType::RR:
            return RearGrip * 1.0f; // Weight shifts onto drive wheels
        
        case EMGDrivetrainType::AWD:
        case EMGDrivetrainType::F4WD:
            return RearGrip * 1.2f; // All wheels driven, best launch
        
        default:
            return RearGrip;
    }
}

float FPerformancePredictor::EstimateSixtyFootTime(const FMGVehicleSpecs& Stats, const FMGDrivetrainConfiguration& Drivetrain)
{
    // 60-foot time is critical for drag racing
    // Depends heavily on launch technique and traction
    
    // TODO: Implement launch simulation
    return 2.0f; // Placeholder (2.0 seconds is good street car time)
}

// =============================================================================
// Additional Implementations (Stubs)
// =============================================================================

// Note: The following are stubs to maintain compilation.
// Full implementation should be completed following the patterns above.

float FPerformanceIndexCalculator::CalculatePI(const FMGVehicleSpecs& Stats)
{
    // TODO: Implement PI calculation
    return 500.0f;
}

EMGPerformanceClass FPerformanceIndexCalculator::GetPerformanceClass(float PI)
{
    if (PI < 400.0f) return EMGPerformanceClass::D;
    if (PI < 500.0f) return EMGPerformanceClass::C;
    if (PI < 600.0f) return EMGPerformanceClass::B;
    if (PI < 700.0f) return EMGPerformanceClass::A;
    if (PI < 800.0f) return EMGPerformanceClass::S;
    if (PI < 900.0f) return EMGPerformanceClass::SPlus;
    if (PI < 999.0f) return EMGPerformanceClass::Hyper;
    return EMGPerformanceClass::Legend;
}

float FValueCalculator::CalculateVehicleValue(const FMGVehicleData& Vehicle, const UMGVehicleModelData* BaseModel)
{
    // TODO: Implement valuation
    return 25000.0f;
}

float FValueCalculator::CalculatePartsValue(const FMGVehicleData& Vehicle)
{
    // TODO: Implement parts value calculation
    return 5000.0f;
}

float FValueCalculator::GetDepreciationMultiplier(float Mileage, float Age)
{
    // TODO: Implement depreciation curve
    return 0.8f;
}

float FValueCalculator::GetPrestigeBonus(const FMGVehicleData& Vehicle)
{
    // TODO: Calculate prestige from race history
    return 0.0f;
}

float FReliabilityCalculator::CalculateReliability(const FMGVehicleData& Vehicle)
{
    // TODO: Implement reliability rating based on part tiers
    return 85.0f;
}

float FReliabilityCalculator::GetTierReliability(EMGPartTier Tier)
{
    switch (Tier)
    {
        case EMGPartTier::OEM:
            return 1.0f; // 100% reliable
        
        case EMGPartTier::Street:
            return 0.9f;
        
        case EMGPartTier::Sport:
            return 0.75f;
        
        case EMGPartTier::Race:
            return 0.5f;
        
        case EMGPartTier::Pro:
            return 0.3f;
        
        case EMGPartTier::Legendary:
            return 0.1f; // Experimental/prototype
        
        default:
            return 1.0f;
    }
}

} // namespace MidnightGrind::Racing::Performance

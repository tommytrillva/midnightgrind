// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleFactory.h"

FMGVehicleData UMGVehicleFactory::CreateVehicleFromPreset(EMGVehiclePreset Preset)
{
	FMGVehicleData Vehicle;
	Vehicle.DisplayName = GenerateVehicleName(Preset);

	switch (Preset)
	{
	case EMGVehiclePreset::JDM_Entry:
		// Civic Si / Miata style - lightweight, rev-happy
		Vehicle = CreateVehicle(EMGEngineType::I4, 1800, 180.0f, 150.0f, 7200, EMGDrivetrainType::FWD, 1150.0f, EMGForcedInductionType::None, 0.0f);
		Vehicle.DisplayName = TEXT("JDM Tuner Si");
		break;

	case EMGVehiclePreset::JDM_Mid:
		// S15 / RX-7 style - balanced, turbo
		Vehicle = CreateVehicle(EMGEngineType::I4, 2000, 280.0f, 260.0f, 7500, EMGDrivetrainType::RWD, 1350.0f, EMGForcedInductionType::Turbo_Single, 12.0f);
		Vehicle.DisplayName = TEXT("JDM Sports Turbo");
		break;

	case EMGVehiclePreset::JDM_High:
		// Supra / GTR style - big power, AWD option
		Vehicle = CreateVehicle(EMGEngineType::I6, 3000, 400.0f, 380.0f, 7000, EMGDrivetrainType::RWD, 1550.0f, EMGForcedInductionType::Turbo_Twin, 18.0f);
		Vehicle.DisplayName = TEXT("JDM Legend TT");
		break;

	case EMGVehiclePreset::Muscle_Entry:
		// V6 Mustang / Camaro style
		Vehicle = CreateVehicle(EMGEngineType::V6, 3700, 300.0f, 280.0f, 6500, EMGDrivetrainType::RWD, 1600.0f, EMGForcedInductionType::None, 0.0f);
		Vehicle.DisplayName = TEXT("Muscle Coupe V6");
		break;

	case EMGVehiclePreset::Muscle_Mid:
		// GT Mustang / SS Camaro style
		Vehicle = CreateVehicle(EMGEngineType::V8, 5000, 450.0f, 420.0f, 6800, EMGDrivetrainType::RWD, 1750.0f, EMGForcedInductionType::None, 0.0f);
		Vehicle.DisplayName = TEXT("Muscle GT V8");
		break;

	case EMGVehiclePreset::Muscle_High:
		// Hellcat style - supercharged V8
		Vehicle = CreateVehicle(EMGEngineType::V8, 6200, 707.0f, 650.0f, 6500, EMGDrivetrainType::RWD, 1950.0f, EMGForcedInductionType::Supercharger_Roots, 11.6f);
		Vehicle.DisplayName = TEXT("Muscle Super SC");
		break;

	case EMGVehiclePreset::Euro_Entry:
		// Golf GTI style
		Vehicle = CreateVehicle(EMGEngineType::I4, 2000, 230.0f, 250.0f, 6800, EMGDrivetrainType::FWD, 1400.0f, EMGForcedInductionType::Turbo_Single, 8.0f);
		Vehicle.DisplayName = TEXT("Euro Hot Hatch");
		break;

	case EMGVehiclePreset::Euro_Mid:
		// M3 / RS4 style
		Vehicle = CreateVehicle(EMGEngineType::I6, 3000, 425.0f, 400.0f, 7600, EMGDrivetrainType::RWD, 1650.0f, EMGForcedInductionType::Turbo_Twin, 14.0f);
		Vehicle.DisplayName = TEXT("Euro Sports M");
		break;

	case EMGVehiclePreset::Euro_High:
		// AMG GT / M5 style
		Vehicle = CreateVehicle(EMGEngineType::V8, 4000, 577.0f, 553.0f, 7200, EMGDrivetrainType::AWD, 1800.0f, EMGForcedInductionType::Turbo_Twin, 16.0f);
		Vehicle.DisplayName = TEXT("Euro Super GT");
		break;

	case EMGVehiclePreset::Hypercar:
		// McLaren / Lambo style
		Vehicle = CreateVehicle(EMGEngineType::V8, 4000, 710.0f, 568.0f, 8500, EMGDrivetrainType::AWD, 1475.0f, EMGForcedInductionType::Turbo_Twin, 20.0f);
		Vehicle.DisplayName = TEXT("Hypercar X");
		break;

	case EMGVehiclePreset::Custom:
	default:
		// Basic empty vehicle
		Vehicle = CreateVehicle(EMGEngineType::I4, 2000, 200.0f, 180.0f, 7000, EMGDrivetrainType::RWD, 1400.0f, EMGForcedInductionType::None, 0.0f);
		Vehicle.DisplayName = TEXT("Custom Build");
		break;
	}

	// Apply slight random variation for uniqueness
	ApplyRandomVariation(Vehicle);

	// Calculate stats
	RecalculateStats(Vehicle);

	return Vehicle;
}

FMGVehicleData UMGVehicleFactory::CreateRandomVehicle(float MinPI, float MaxPI)
{
	// Pick a random preset
	TArray<EMGVehiclePreset> Presets = {
		EMGVehiclePreset::JDM_Entry, EMGVehiclePreset::JDM_Mid, EMGVehiclePreset::JDM_High,
		EMGVehiclePreset::Muscle_Entry, EMGVehiclePreset::Muscle_Mid, EMGVehiclePreset::Muscle_High,
		EMGVehiclePreset::Euro_Entry, EMGVehiclePreset::Euro_Mid, EMGVehiclePreset::Euro_High
	};

	EMGVehiclePreset RandomPreset = Presets[FMath::RandRange(0, Presets.Num() - 1)];
	FMGVehicleData Vehicle = CreateVehicleFromPreset(RandomPreset);

	// Adjust to fit within PI range
	if (Vehicle.Stats.PerformanceIndex < MinPI)
	{
		Vehicle = UpgradeToTargetPI(Vehicle, (MinPI + MaxPI) * 0.5f);
	}
	else if (Vehicle.Stats.PerformanceIndex > MaxPI)
	{
		// Scale down power
		float Scale = MaxPI / Vehicle.Stats.PerformanceIndex;
		Vehicle.Stats.Horsepower *= Scale;
		Vehicle.Stats.Torque *= Scale;
		RecalculateStats(Vehicle);
	}

	return Vehicle;
}

FMGVehicleData UMGVehicleFactory::CreateVehicleForClass(EMGPerformanceClass TargetClass)
{
	float TargetPI;
	switch (TargetClass)
	{
	case EMGPerformanceClass::D: TargetPI = 200.0f; break;
	case EMGPerformanceClass::C: TargetPI = 375.0f; break;
	case EMGPerformanceClass::B: TargetPI = 525.0f; break;
	case EMGPerformanceClass::A: TargetPI = 675.0f; break;
	case EMGPerformanceClass::S: TargetPI = 825.0f; break;
	case EMGPerformanceClass::X: TargetPI = 950.0f; break;
	default: TargetPI = 400.0f; break;
	}

	return CreateRandomVehicle(TargetPI - 50.0f, TargetPI + 50.0f);
}

FMGVehicleData UMGVehicleFactory::CreateStarterVehicle()
{
	// Classic starter car - reliable, affordable, room to grow
	FMGVehicleData Vehicle = CreateVehicleFromPreset(EMGVehiclePreset::JDM_Entry);
	Vehicle.DisplayName = TEXT("Street Tuner");

	// Make it slightly worn (more character)
	Vehicle.Mileage = FMath::RandRange(50000, 100000);
	Vehicle.WheelsTires.FrontTireCondition = 85.0f;
	Vehicle.WheelsTires.RearTireCondition = 80.0f;

	return Vehicle;
}

FMGVehicleData UMGVehicleFactory::CreateAIOpponent(const FMGVehicleData& PlayerVehicle, float DifficultyScale)
{
	// Create vehicle close to player's PI
	float TargetPI = PlayerVehicle.Stats.PerformanceIndex;

	// Difficulty affects how much better/worse AI cars are
	// DifficultyScale: 0 = AI is worse, 0.5 = equal, 1.0 = AI is better
	float PIVariation = FMath::Lerp(-50.0f, 50.0f, DifficultyScale);
	TargetPI += PIVariation;

	FMGVehicleData AIVehicle = CreateRandomVehicle(TargetPI - 30.0f, TargetPI + 30.0f);

	// Generate a random driver name
	static const TArray<FString> FirstNames = {TEXT("Jake"), TEXT("Marcus"), TEXT("Yuki"), TEXT("Diego"), TEXT("Viktor"), TEXT("Kenji"), TEXT("Carlos"), TEXT("Jin"), TEXT("Alex"), TEXT("Rico")};
	static const TArray<FString> Nicknames = {TEXT("Shadow"), TEXT("Nitro"), TEXT("Drift"), TEXT("Speed"), TEXT("Boost"), TEXT("Rev"), TEXT("Apex"), TEXT("Turbo"), TEXT("Flash"), TEXT("Blaze")};

	FString DriverName = FirstNames[FMath::RandRange(0, FirstNames.Num() - 1)] + TEXT(" '") +
		Nicknames[FMath::RandRange(0, Nicknames.Num() - 1)] + TEXT("'");

	AIVehicle.DisplayName = DriverName;

	return AIVehicle;
}

FMGVehicleData UMGVehicleFactory::UpgradeToTargetPI(const FMGVehicleData& BaseVehicle, float TargetPI)
{
	FMGVehicleData Vehicle = BaseVehicle;

	float CurrentPI = Vehicle.Stats.PerformanceIndex;
	float PIGap = TargetPI - CurrentPI;

	if (PIGap <= 0)
	{
		return Vehicle;
	}

	// Prioritize upgrades: Tires -> Turbo -> Power
	if (PIGap > 0 && Vehicle.WheelsTires.FrontTireCompound < EMGTireCompound::SemiSlick)
	{
		Vehicle = ApplyTireUpgrade(Vehicle, EMGTireCompound::SemiSlick);
		RecalculateStats(Vehicle);
		PIGap = TargetPI - Vehicle.Stats.PerformanceIndex;
	}

	if (PIGap > 50 && Vehicle.Engine.ForcedInduction.Type == EMGForcedInductionType::None)
	{
		Vehicle = ApplyTurboUpgrade(Vehicle, EMGForcedInductionType::Turbo_Single, 10.0f);
		RecalculateStats(Vehicle);
		PIGap = TargetPI - Vehicle.Stats.PerformanceIndex;
	}

	// Fine-tune with power adjustments
	if (PIGap > 0)
	{
		float PowerIncrease = PIGap * 0.5f; // Rough approximation
		Vehicle.Stats.Horsepower += PowerIncrease;
		Vehicle.Stats.Torque += PowerIncrease * 0.8f;
		RecalculateStats(Vehicle);
	}

	return Vehicle;
}

FMGVehicleData UMGVehicleFactory::ApplyTurboUpgrade(const FMGVehicleData& BaseVehicle, EMGForcedInductionType TurboType, float BoostPSI)
{
	FMGVehicleData Vehicle = BaseVehicle;

	Vehicle.Engine.ForcedInduction.Type = TurboType;
	Vehicle.Engine.ForcedInduction.MaxBoostPSI = BoostPSI;
	Vehicle.Engine.ForcedInduction.SpoolTimeSeconds = (TurboType == EMGForcedInductionType::Turbo_Twin) ? 0.5f : 0.8f;
	Vehicle.Engine.ForcedInduction.BoostThresholdRPM = 3500;

	// Power increase: roughly 5-7% per PSI
	float PowerMultiplier = 1.0f + (BoostPSI * 0.06f);
	Vehicle.Stats.Horsepower *= PowerMultiplier;
	Vehicle.Stats.Torque *= PowerMultiplier * 1.1f; // Turbos add more torque

	return Vehicle;
}

FMGVehicleData UMGVehicleFactory::ApplyTireUpgrade(const FMGVehicleData& BaseVehicle, EMGTireCompound Compound)
{
	FMGVehicleData Vehicle = BaseVehicle;

	Vehicle.WheelsTires.FrontTireCompound = Compound;
	Vehicle.WheelsTires.RearTireCompound = Compound;
	Vehicle.WheelsTires.FrontTireCondition = 100.0f;
	Vehicle.WheelsTires.RearTireCondition = 100.0f;

	return Vehicle;
}

void UMGVehicleFactory::RecalculateStats(FMGVehicleData& Vehicle)
{
	// Calculate derived stats
	Vehicle.Stats.PowerToWeightRatio = Vehicle.Stats.Horsepower / (Vehicle.Stats.WeightKG / 1000.0f);

	// Estimate acceleration (simplified physics)
	float PowerToWeight = Vehicle.Stats.PowerToWeightRatio;
	Vehicle.Stats.ZeroTo60MPH = 60.0f / (PowerToWeight * 0.3f); // Rough approximation
	Vehicle.Stats.ZeroTo100MPH = Vehicle.Stats.ZeroTo60MPH * 2.5f;
	Vehicle.Stats.QuarterMileTime = Vehicle.Stats.ZeroTo60MPH * 2.0f + 4.0f;
	Vehicle.Stats.QuarterMileTrapMPH = Vehicle.Stats.Horsepower / 10.0f + 50.0f;

	// Top speed (simplified - affected by power, drag, gearing)
	Vehicle.Stats.TopSpeedMPH = FMath::Sqrt(Vehicle.Stats.Horsepower * 150.0f);

	// Grip ratings (affected by tires, weight distribution, downforce)
	float TireGripFactor = 1.0f;
	switch (Vehicle.WheelsTires.FrontTireCompound)
	{
	case EMGTireCompound::Economy: TireGripFactor = 0.7f; break;
	case EMGTireCompound::AllSeason: TireGripFactor = 0.8f; break;
	case EMGTireCompound::Sport: TireGripFactor = 0.9f; break;
	case EMGTireCompound::Performance: TireGripFactor = 0.95f; break;
	case EMGTireCompound::SemiSlick: TireGripFactor = 1.05f; break;
	case EMGTireCompound::Slick: TireGripFactor = 1.15f; break;
	default: TireGripFactor = 1.0f; break;
	}

	Vehicle.Stats.GripFront = 80.0f * TireGripFactor * Vehicle.Stats.WeightDistributionFront;
	Vehicle.Stats.GripRear = 80.0f * TireGripFactor * (1.0f - Vehicle.Stats.WeightDistributionFront);
	Vehicle.Stats.HandlingRating = (Vehicle.Stats.GripFront + Vehicle.Stats.GripRear) * 0.6f;
	Vehicle.Stats.BrakingRating = (Vehicle.Stats.GripFront + Vehicle.Stats.GripRear) * 0.5f + 20.0f;

	// Calculate Performance Index
	Vehicle.Stats.PerformanceIndex = CalculatePerformanceIndex(Vehicle.Stats);
	Vehicle.Stats.PerformanceClass = GetPerformanceClassFromPI(Vehicle.Stats.PerformanceIndex);
}

float UMGVehicleFactory::CalculatePerformanceIndex(const FMGVehicleSpecs& Stats)
{
	// Performance Index formula (inspired by Forza Horizon)
	// Weighs: Power, Grip, Weight, Braking, Top Speed
	float PowerScore = Stats.Horsepower * 0.5f;
	float GripScore = (Stats.GripFront + Stats.GripRear) * 2.0f;
	float WeightPenalty = Stats.WeightKG * 0.1f;
	float SpeedScore = Stats.TopSpeedMPH * 1.5f;
	float BrakingScore = Stats.BrakingRating * 1.0f;

	float PI = PowerScore + GripScore + SpeedScore + BrakingScore - WeightPenalty;

	return FMath::Clamp(PI, 100.0f, 999.0f);
}

EMGPerformanceClass UMGVehicleFactory::GetPerformanceClassFromPI(float PI)
{
	if (PI >= 901.0f) return EMGPerformanceClass::X;
	if (PI >= 750.0f) return EMGPerformanceClass::S;
	if (PI >= 600.0f) return EMGPerformanceClass::A;
	if (PI >= 450.0f) return EMGPerformanceClass::B;
	if (PI >= 300.0f) return EMGPerformanceClass::C;
	return EMGPerformanceClass::D;
}

FString UMGVehicleFactory::GenerateVehicleName(EMGVehiclePreset Preset)
{
	static const TMap<EMGVehiclePreset, TArray<FString>> PresetNames = {
		{EMGVehiclePreset::JDM_Entry, {TEXT("Street Civic"), TEXT("MX-5 Roadster"), TEXT("Integra Type R"), TEXT("Celica GT")}},
		{EMGVehiclePreset::JDM_Mid, {TEXT("Silvia S15"), TEXT("RX-7 FD"), TEXT("350Z"), TEXT("Evo IX")}},
		{EMGVehiclePreset::JDM_High, {TEXT("Supra MK4"), TEXT("GT-R R34"), TEXT("NSX Type S"), TEXT("LFA")}},
		{EMGVehiclePreset::Muscle_Entry, {TEXT("Mustang V6"), TEXT("Camaro LT"), TEXT("Challenger SXT")}},
		{EMGVehiclePreset::Muscle_Mid, {TEXT("Mustang GT"), TEXT("Camaro SS"), TEXT("Challenger RT")}},
		{EMGVehiclePreset::Muscle_High, {TEXT("Hellcat"), TEXT("GT500"), TEXT("ZL1"), TEXT("Demon")}},
		{EMGVehiclePreset::Euro_Entry, {TEXT("Golf GTI"), TEXT("Focus ST"), TEXT("Civic Type R EU")}},
		{EMGVehiclePreset::Euro_Mid, {TEXT("M3 E46"), TEXT("RS4 B7"), TEXT("C63 AMG")}},
		{EMGVehiclePreset::Euro_High, {TEXT("AMG GT"), TEXT("M5 F90"), TEXT("RS7")}},
		{EMGVehiclePreset::Hypercar, {TEXT("720S"), TEXT("Huracan"), TEXT("488 GTB"), TEXT("Aventador")}}
	};

	if (const TArray<FString>* Names = PresetNames.Find(Preset))
	{
		return (*Names)[FMath::RandRange(0, Names->Num() - 1)];
	}

	return TEXT("Unknown Vehicle");
}

FText UMGVehicleFactory::GetPresetDisplayName(EMGVehiclePreset Preset)
{
	const UEnum* EnumPtr = StaticEnum<EMGVehiclePreset>();
	if (EnumPtr)
	{
		return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Preset));
	}
	return FText::FromString(TEXT("Unknown"));
}

FMGVehicleData UMGVehicleFactory::CreateVehicle(
	EMGEngineType EngineType,
	int32 DisplacementCC,
	float BaseHP,
	float BaseTorque,
	int32 Redline,
	EMGDrivetrainType Drivetrain,
	float WeightKG,
	EMGForcedInductionType FI,
	float BoostPSI)
{
	FMGVehicleData Vehicle;
	Vehicle.VehicleID = FGuid::NewGuid();
	Vehicle.DateAcquired = FDateTime::Now();

	// Engine
	Vehicle.Engine.EngineType = EngineType;
	Vehicle.Engine.DisplacementCC = DisplacementCC;
	Vehicle.Engine.ForcedInduction.Type = FI;
	Vehicle.Engine.ForcedInduction.MaxBoostPSI = BoostPSI;
	if (FI != EMGForcedInductionType::None)
	{
		Vehicle.Engine.ForcedInduction.SpoolTimeSeconds = (FI == EMGForcedInductionType::Turbo_Single) ? 0.8f : 0.5f;
		Vehicle.Engine.ForcedInduction.BoostThresholdRPM = 3500;
	}

	// Drivetrain
	Vehicle.Drivetrain.DrivetrainType = Drivetrain;
	Vehicle.Drivetrain.TransmissionType = EMGTransmissionType::Manual;
	Vehicle.Drivetrain.GearCount = 6;

	// Stats
	Vehicle.Stats.Horsepower = BaseHP;
	Vehicle.Stats.Torque = BaseTorque;
	Vehicle.Stats.Redline = Redline;
	Vehicle.Stats.BoostPSI = BoostPSI;
	Vehicle.Stats.WeightKG = WeightKG;

	// Weight distribution based on drivetrain
	switch (Drivetrain)
	{
	case EMGDrivetrainType::FWD:
		Vehicle.Stats.WeightDistributionFront = 0.62f;
		break;
	case EMGDrivetrainType::RWD:
		Vehicle.Stats.WeightDistributionFront = 0.52f;
		break;
	case EMGDrivetrainType::AWD:
		Vehicle.Stats.WeightDistributionFront = 0.55f;
		break;
	}

	// Default tires
	Vehicle.WheelsTires.FrontTireCompound = EMGTireCompound::Sport;
	Vehicle.WheelsTires.RearTireCompound = EMGTireCompound::Sport;
	Vehicle.WheelsTires.FrontTireCondition = 100.0f;
	Vehicle.WheelsTires.RearTireCondition = 100.0f;

	// Suspension defaults
	Vehicle.Suspension.FrontRideHeightMM = 150.0f;
	Vehicle.Suspension.RearRideHeightMM = 160.0f;

	return Vehicle;
}

void UMGVehicleFactory::ApplyRandomVariation(FMGVehicleData& Vehicle, float VariationAmount)
{
	// Add small random variations to make each vehicle unique
	float Variation = FMath::RandRange(-VariationAmount, VariationAmount);

	Vehicle.Stats.Horsepower *= (1.0f + Variation);
	Vehicle.Stats.Torque *= (1.0f + Variation);
	Vehicle.Stats.WeightKG *= (1.0f + Variation * 0.5f);
}

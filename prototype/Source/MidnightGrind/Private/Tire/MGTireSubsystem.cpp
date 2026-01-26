// Copyright Midnight Grind. All Rights Reserved.

#include "Tire/MGTireSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGTireSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default settings
	Settings.GlobalWearMultiplier = 1.0f;
	Settings.GlobalGripMultiplier = 1.0f;
	Settings.TemperatureSimSpeed = 1.0f;
	Settings.AmbientTemperature = 25.0f;
	Settings.TrackTemperature = 35.0f;
	Settings.bSimulatePressure = true;
	Settings.bSimulateTemperature = true;
	Settings.bSimulateWear = true;
	Settings.bAllowPunctures = true;
	Settings.PunctureChance = 0.0001f;

	// Initialize wear factors
	WearFactors.AccelerationWear = 1.0f;
	WearFactors.BrakingWear = 1.2f;
	WearFactors.CorneringWear = 1.5f;
	WearFactors.SlipWear = 2.0f;
	WearFactors.LockupWear = 3.0f;
	WearFactors.TemperatureWear = 1.0f;
	WearFactors.SurfaceWear = 1.0f;
	WearFactors.LoadWear = 1.0f;

	InitializeDefaultCompounds();
	LoadTireData();

	// Start tire tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TireTickHandle,
			this,
			&UMGTireSubsystem::OnTireTick,
			0.05f,
			true
		);
	}
}

void UMGTireSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TireTickHandle);
	}

	SaveTireData();

	Super::Deinitialize();
}

bool UMGTireSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGTireSubsystem::InitializeDefaultCompounds()
{
	// Ultra Soft
	FMGTireCompoundData UltraSoft;
	UltraSoft.CompoundType = EMGTireCompoundType::UltraSoft;
	UltraSoft.CompoundID = TEXT("UltraSoft");
	UltraSoft.DisplayName = FText::FromString(TEXT("Ultra Soft"));
	UltraSoft.CompoundColor = FLinearColor(1.0f, 0.0f, 0.5f, 1.0f); // Pink
	UltraSoft.BaseGrip = 1.25f;
	UltraSoft.PeakGripTemperature = 85.0f;
	UltraSoft.OptimalTempMin = 75.0f;
	UltraSoft.OptimalTempMax = 95.0f;
	UltraSoft.WearRate = 2.0f;
	UltraSoft.HeatUpRate = 1.5f;
	UltraSoft.CoolDownRate = 0.8f;
	UltraSoft.WetPerformance = 0.3f;
	UltraSoft.ExpectedLaps = 10;
	CompoundDatabase.Add(EMGTireCompoundType::UltraSoft, UltraSoft);

	// Soft
	FMGTireCompoundData Soft;
	Soft.CompoundType = EMGTireCompoundType::Soft;
	Soft.CompoundID = TEXT("Soft");
	Soft.DisplayName = FText::FromString(TEXT("Soft"));
	Soft.CompoundColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
	Soft.BaseGrip = 1.15f;
	Soft.PeakGripTemperature = 90.0f;
	Soft.OptimalTempMin = 80.0f;
	Soft.OptimalTempMax = 100.0f;
	Soft.WearRate = 1.5f;
	Soft.HeatUpRate = 1.3f;
	Soft.CoolDownRate = 0.9f;
	Soft.WetPerformance = 0.4f;
	Soft.ExpectedLaps = 15;
	CompoundDatabase.Add(EMGTireCompoundType::Soft, Soft);

	// Medium
	FMGTireCompoundData Medium;
	Medium.CompoundType = EMGTireCompoundType::Medium;
	Medium.CompoundID = TEXT("Medium");
	Medium.DisplayName = FText::FromString(TEXT("Medium"));
	Medium.CompoundColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
	Medium.BaseGrip = 1.0f;
	Medium.PeakGripTemperature = 95.0f;
	Medium.OptimalTempMin = 85.0f;
	Medium.OptimalTempMax = 105.0f;
	Medium.WearRate = 1.0f;
	Medium.HeatUpRate = 1.0f;
	Medium.CoolDownRate = 1.0f;
	Medium.WetPerformance = 0.5f;
	Medium.ExpectedLaps = 25;
	CompoundDatabase.Add(EMGTireCompoundType::Medium, Medium);

	// Hard
	FMGTireCompoundData Hard;
	Hard.CompoundType = EMGTireCompoundType::Hard;
	Hard.CompoundID = TEXT("Hard");
	Hard.DisplayName = FText::FromString(TEXT("Hard"));
	Hard.CompoundColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
	Hard.BaseGrip = 0.9f;
	Hard.PeakGripTemperature = 100.0f;
	Hard.OptimalTempMin = 90.0f;
	Hard.OptimalTempMax = 115.0f;
	Hard.WearRate = 0.6f;
	Hard.HeatUpRate = 0.7f;
	Hard.CoolDownRate = 1.2f;
	Hard.WetPerformance = 0.45f;
	Hard.ExpectedLaps = 40;
	CompoundDatabase.Add(EMGTireCompoundType::Hard, Hard);

	// Intermediate
	FMGTireCompoundData Intermediate;
	Intermediate.CompoundType = EMGTireCompoundType::Intermediate;
	Intermediate.CompoundID = TEXT("Intermediate");
	Intermediate.DisplayName = FText::FromString(TEXT("Intermediate"));
	Intermediate.CompoundColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
	Intermediate.BaseGrip = 0.85f;
	Intermediate.PeakGripTemperature = 70.0f;
	Intermediate.OptimalTempMin = 50.0f;
	Intermediate.OptimalTempMax = 90.0f;
	Intermediate.WearRate = 1.2f;
	Intermediate.HeatUpRate = 0.8f;
	Intermediate.CoolDownRate = 0.6f;
	Intermediate.WetPerformance = 0.9f;
	Intermediate.ExpectedLaps = 20;
	Intermediate.bAllWeather = true;
	CompoundDatabase.Add(EMGTireCompoundType::Intermediate, Intermediate);

	// Full Wet
	FMGTireCompoundData FullWet;
	FullWet.CompoundType = EMGTireCompoundType::FullWet;
	FullWet.CompoundID = TEXT("FullWet");
	FullWet.DisplayName = FText::FromString(TEXT("Full Wet"));
	FullWet.CompoundColor = FLinearColor(0.0f, 0.0f, 1.0f, 1.0f); // Blue
	FullWet.BaseGrip = 0.7f;
	FullWet.PeakGripTemperature = 60.0f;
	FullWet.OptimalTempMin = 40.0f;
	FullWet.OptimalTempMax = 80.0f;
	FullWet.WearRate = 1.5f;
	FullWet.HeatUpRate = 0.6f;
	FullWet.CoolDownRate = 0.5f;
	FullWet.WetPerformance = 1.0f;
	FullWet.ExpectedLaps = 15;
	FullWet.bAllWeather = true;
	CompoundDatabase.Add(EMGTireCompoundType::FullWet, FullWet);

	// Drift Compound
	FMGTireCompoundData DriftComp;
	DriftComp.CompoundType = EMGTireCompoundType::DriftCompound;
	DriftComp.CompoundID = TEXT("Drift");
	DriftComp.DisplayName = FText::FromString(TEXT("Drift"));
	DriftComp.CompoundColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
	DriftComp.BaseGrip = 0.8f;
	DriftComp.LateralGripMod = 0.7f;
	DriftComp.LongitudinalGripMod = 1.1f;
	DriftComp.PeakGripTemperature = 90.0f;
	DriftComp.OptimalTempMin = 60.0f;
	DriftComp.OptimalTempMax = 120.0f;
	DriftComp.WearRate = 0.8f;
	DriftComp.HeatUpRate = 1.5f;
	DriftComp.ExpectedLaps = 30;
	CompoundDatabase.Add(EMGTireCompoundType::DriftCompound, DriftComp);

	// Slick
	FMGTireCompoundData Slick;
	Slick.CompoundType = EMGTireCompoundType::Slick;
	Slick.CompoundID = TEXT("Slick");
	Slick.DisplayName = FText::FromString(TEXT("Slick"));
	Slick.CompoundColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f); // Black
	Slick.BaseGrip = 1.3f;
	Slick.PeakGripTemperature = 100.0f;
	Slick.OptimalTempMin = 90.0f;
	Slick.OptimalTempMax = 110.0f;
	Slick.WearRate = 1.8f;
	Slick.HeatUpRate = 1.4f;
	Slick.WetPerformance = 0.2f;
	Slick.ExpectedLaps = 12;
	CompoundDatabase.Add(EMGTireCompoundType::Slick, Slick);
}

void UMGTireSubsystem::OnTireTick()
{
	UpdateAllTires(0.05f);
}

void UMGTireSubsystem::UpdateAllTires(float DeltaTime)
{
	for (auto& Pair : VehicleTires)
	{
		FMGVehicleTireSet& TireSet = Pair.Value;
		FName VehicleID = Pair.Key;

		// Update each tire
		TArray<FMGTireState*> Tires = { &TireSet.FrontLeft, &TireSet.FrontRight, &TireSet.RearLeft, &TireSet.RearRight };

		for (FMGTireState* Tire : Tires)
		{
			if (Tire->bFlat) continue;

			FMGTireCompoundData Compound = GetCompoundData(Tire->Compound);

			if (Settings.bSimulateTemperature)
			{
				UpdateTireTemperature(*Tire, Compound, DeltaTime);
			}

			if (Settings.bSimulateWear)
			{
				UpdateTireWear(*Tire, Compound, DeltaTime);
			}

			UpdateTireGrip(*Tire, Compound);

			if (Settings.bAllowPunctures)
			{
				CheckForPuncture(VehicleID, *Tire);
			}
		}

		// Update averages
		TireSet.AverageWear = (TireSet.FrontLeft.WearLevel + TireSet.FrontRight.WearLevel +
							   TireSet.RearLeft.WearLevel + TireSet.RearRight.WearLevel) / 4.0f;
		TireSet.AverageTemperature = (TireSet.FrontLeft.Temperature + TireSet.FrontRight.Temperature +
									  TireSet.RearLeft.Temperature + TireSet.RearRight.Temperature) / 4.0f;
		TireSet.AverageGrip = (TireSet.FrontLeft.CurrentGrip + TireSet.FrontRight.CurrentGrip +
							   TireSet.RearLeft.CurrentGrip + TireSet.RearRight.CurrentGrip) / 4.0f;
	}
}

void UMGTireSubsystem::UpdateTireTemperature(FMGTireState& Tire, const FMGTireCompoundData& Compound, float DeltaTime)
{
	// Heat from slip
	float SlipHeat = (FMath::Abs(Tire.SlipRatio) + FMath::Abs(Tire.SlipAngle)) * 50.0f * Compound.HeatUpRate;

	// Heat from load
	float LoadHeat = (Tire.LoadForce / 10000.0f) * 10.0f;

	// Cooling from ambient
	float Cooling = (Tire.Temperature - Settings.AmbientTemperature) * 0.1f * Compound.CoolDownRate;

	// Apply temperature change
	float TempChange = (SlipHeat + LoadHeat - Cooling) * DeltaTime * Settings.TemperatureSimSpeed;
	Tire.Temperature += TempChange;
	Tire.Temperature = FMath::Clamp(Tire.Temperature, Settings.AmbientTemperature, 200.0f);

	// Update surface and core temps
	Tire.SurfaceTemperature = Tire.Temperature + (SlipHeat * 0.2f);
	Tire.CoreTemperature = FMath::Lerp(Tire.CoreTemperature, Tire.Temperature, DeltaTime * 0.5f);

	// Check for overheating
	if (Tire.Temperature > Compound.OptimalTempMax + 20.0f)
	{
		// Would broadcast warning but need vehicle ID
	}
}

void UMGTireSubsystem::UpdateTireWear(FMGTireState& Tire, const FMGTireCompoundData& Compound, float DeltaTime)
{
	float WearAmount = 0.0f;

	// Base wear
	WearAmount += 0.0001f * Compound.WearRate;

	// Slip wear
	float SlipMagnitude = FMath::Sqrt(Tire.SlipRatio * Tire.SlipRatio + Tire.SlipAngle * Tire.SlipAngle);
	WearAmount += SlipMagnitude * 0.001f * WearFactors.SlipWear;

	// Temperature wear (overheating causes more wear)
	if (Tire.Temperature > Compound.OptimalTempMax)
	{
		float OverTemp = Tire.Temperature - Compound.OptimalTempMax;
		WearAmount += OverTemp * 0.0001f * WearFactors.TemperatureWear;
	}

	// Lockup wear
	if (Tire.bLocked)
	{
		WearAmount += 0.01f * WearFactors.LockupWear;
	}

	// Apply wear
	WearAmount *= Settings.GlobalWearMultiplier * DeltaTime;
	Tire.WearLevel -= WearAmount;
	Tire.WearLevel = FMath::Max(Tire.WearLevel, 0.0f);

	// Update condition
	EMGTireCondition NewCondition = CalculateCondition(Tire.WearLevel, Tire.bFlat);
	if (NewCondition != Tire.Condition)
	{
		Tire.Condition = NewCondition;
	}
}

void UMGTireSubsystem::UpdateTireGrip(FMGTireState& Tire, const FMGTireCompoundData& Compound)
{
	float TempGrip = GetGripFromTemperature(Tire.Temperature, Compound);
	float WearGrip = GetGripFromWear(Tire.WearLevel);

	Tire.CurrentGrip = Compound.BaseGrip * TempGrip * WearGrip * Settings.GlobalGripMultiplier;
	Tire.LateralGrip = Tire.CurrentGrip * Compound.LateralGripMod;
	Tire.LongitudinalGrip = Tire.CurrentGrip * Compound.LongitudinalGripMod;

	if (Tire.bFlat)
	{
		Tire.CurrentGrip *= 0.3f;
		Tire.LateralGrip *= 0.3f;
		Tire.LongitudinalGrip *= 0.3f;
	}
}

void UMGTireSubsystem::CheckForPuncture(FName VehicleID, FMGTireState& Tire)
{
	if (Tire.bFlat) return;

	// Check for random puncture
	float PunctureRoll = FMath::FRand();
	float AdjustedChance = Settings.PunctureChance;

	// Increased chance with low wear
	if (Tire.WearLevel < 0.2f)
	{
		AdjustedChance *= 3.0f;
	}

	// Increased chance with high temperature
	if (Tire.Temperature > 150.0f)
	{
		AdjustedChance *= 2.0f;
	}

	if (PunctureRoll < AdjustedChance)
	{
		Tire.bFlat = true;
		Tire.Condition = EMGTireCondition::Punctured;
		OnTirePunctured.Broadcast(VehicleID, Tire.Position);
	}
}

EMGTireCondition UMGTireSubsystem::CalculateCondition(float WearLevel, bool bFlat) const
{
	if (bFlat)
	{
		return EMGTireCondition::Punctured;
	}

	if (WearLevel > 0.75f)
	{
		return EMGTireCondition::Optimal;
	}
	else if (WearLevel > 0.5f)
	{
		return EMGTireCondition::Good;
	}
	else if (WearLevel > 0.25f)
	{
		return EMGTireCondition::Worn;
	}
	else if (WearLevel > 0.0f)
	{
		return EMGTireCondition::Critical;
	}
	else
	{
		return EMGTireCondition::Blown;
	}
}

void UMGTireSubsystem::RegisterVehicle(FName VehicleID)
{
	if (VehicleTires.Contains(VehicleID))
	{
		return;
	}

	FMGVehicleTireSet NewTireSet;
	NewTireSet.VehicleID = VehicleID;

	// Initialize all four tires with medium compound
	NewTireSet.FrontLeft.Position = EMGTirePosition::FrontLeft;
	NewTireSet.FrontLeft.Compound = EMGTireCompoundType::Medium;
	NewTireSet.FrontLeft.WearLevel = 1.0f;
	NewTireSet.FrontLeft.Temperature = Settings.AmbientTemperature;

	NewTireSet.FrontRight.Position = EMGTirePosition::FrontRight;
	NewTireSet.FrontRight.Compound = EMGTireCompoundType::Medium;
	NewTireSet.FrontRight.WearLevel = 1.0f;
	NewTireSet.FrontRight.Temperature = Settings.AmbientTemperature;

	NewTireSet.RearLeft.Position = EMGTirePosition::RearLeft;
	NewTireSet.RearLeft.Compound = EMGTireCompoundType::Medium;
	NewTireSet.RearLeft.WearLevel = 1.0f;
	NewTireSet.RearLeft.Temperature = Settings.AmbientTemperature;

	NewTireSet.RearRight.Position = EMGTirePosition::RearRight;
	NewTireSet.RearRight.Compound = EMGTireCompoundType::Medium;
	NewTireSet.RearRight.WearLevel = 1.0f;
	NewTireSet.RearRight.Temperature = Settings.AmbientTemperature;

	VehicleTires.Add(VehicleID, NewTireSet);
}

void UMGTireSubsystem::UnregisterVehicle(FName VehicleID)
{
	VehicleTires.Remove(VehicleID);
	VehicleTelemetry.Remove(VehicleID);
}

FMGTireState UMGTireSubsystem::GetTireState(FName VehicleID, EMGTirePosition Position) const
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return FMGTireState();
	}

	const FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	switch (Position)
	{
		case EMGTirePosition::FrontLeft: return TireSet.FrontLeft;
		case EMGTirePosition::FrontRight: return TireSet.FrontRight;
		case EMGTirePosition::RearLeft: return TireSet.RearLeft;
		case EMGTirePosition::RearRight: return TireSet.RearRight;
		default: return FMGTireState();
	}
}

FMGVehicleTireSet UMGTireSubsystem::GetVehicleTires(FName VehicleID) const
{
	if (VehicleTires.Contains(VehicleID))
	{
		return VehicleTires[VehicleID];
	}
	return FMGVehicleTireSet();
}

float UMGTireSubsystem::GetTireWear(FName VehicleID, EMGTirePosition Position) const
{
	return GetTireState(VehicleID, Position).WearLevel;
}

float UMGTireSubsystem::GetTireTemperature(FName VehicleID, EMGTirePosition Position) const
{
	return GetTireState(VehicleID, Position).Temperature;
}

float UMGTireSubsystem::GetTireGrip(FName VehicleID, EMGTirePosition Position) const
{
	return GetTireState(VehicleID, Position).CurrentGrip;
}

float UMGTireSubsystem::GetTirePressure(FName VehicleID, EMGTirePosition Position) const
{
	return GetTireState(VehicleID, Position).Pressure;
}

EMGTireCondition UMGTireSubsystem::GetTireCondition(FName VehicleID, EMGTirePosition Position) const
{
	return GetTireState(VehicleID, Position).Condition;
}

float UMGTireSubsystem::GetAverageWear(FName VehicleID) const
{
	if (VehicleTires.Contains(VehicleID))
	{
		return VehicleTires[VehicleID].AverageWear;
	}
	return 1.0f;
}

float UMGTireSubsystem::GetAverageGrip(FName VehicleID) const
{
	if (VehicleTires.Contains(VehicleID))
	{
		return VehicleTires[VehicleID].AverageGrip;
	}
	return 1.0f;
}

void UMGTireSubsystem::UpdateTireState(FName VehicleID, EMGTirePosition Position, float SlipRatio, float SlipAngle, float Load, float Speed)
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return;
	}

	FMGTireState* Tire = nullptr;
	FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	switch (Position)
	{
		case EMGTirePosition::FrontLeft: Tire = &TireSet.FrontLeft; break;
		case EMGTirePosition::FrontRight: Tire = &TireSet.FrontRight; break;
		case EMGTirePosition::RearLeft: Tire = &TireSet.RearLeft; break;
		case EMGTirePosition::RearRight: Tire = &TireSet.RearRight; break;
	}

	if (Tire)
	{
		Tire->SlipRatio = SlipRatio;
		Tire->SlipAngle = SlipAngle;
		Tire->LoadForce = Load;

		// Check for lockup
		if (SlipRatio < -0.3f)
		{
			Tire->bLocked = true;
		}
		else
		{
			Tire->bLocked = false;
		}

		// Check for wheelspin
		if (SlipRatio > 0.3f)
		{
			Tire->bSpinning = true;
		}
		else
		{
			Tire->bSpinning = false;
		}
	}
}

void UMGTireSubsystem::ApplyWear(FName VehicleID, EMGTirePosition Position, float WearAmount)
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return;
	}

	FMGTireState* Tire = nullptr;
	FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	switch (Position)
	{
		case EMGTirePosition::FrontLeft: Tire = &TireSet.FrontLeft; break;
		case EMGTirePosition::FrontRight: Tire = &TireSet.FrontRight; break;
		case EMGTirePosition::RearLeft: Tire = &TireSet.RearLeft; break;
		case EMGTirePosition::RearRight: Tire = &TireSet.RearRight; break;
	}

	if (Tire)
	{
		float PrevWear = Tire->WearLevel;
		Tire->WearLevel = FMath::Clamp(Tire->WearLevel - WearAmount, 0.0f, 1.0f);
		OnTireWearUpdated.Broadcast(VehicleID, Position, Tire->WearLevel);
	}
}

void UMGTireSubsystem::ApplyHeat(FName VehicleID, EMGTirePosition Position, float HeatAmount)
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return;
	}

	FMGTireState* Tire = nullptr;
	FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	switch (Position)
	{
		case EMGTirePosition::FrontLeft: Tire = &TireSet.FrontLeft; break;
		case EMGTirePosition::FrontRight: Tire = &TireSet.FrontRight; break;
		case EMGTirePosition::RearLeft: Tire = &TireSet.RearLeft; break;
		case EMGTirePosition::RearRight: Tire = &TireSet.RearRight; break;
	}

	if (Tire)
	{
		Tire->Temperature += HeatAmount;
		Tire->Temperature = FMath::Clamp(Tire->Temperature, Settings.AmbientTemperature, 200.0f);
	}
}

void UMGTireSubsystem::CoolTire(FName VehicleID, EMGTirePosition Position, float CoolAmount)
{
	ApplyHeat(VehicleID, Position, -CoolAmount);
}

void UMGTireSubsystem::SetTirePressure(FName VehicleID, EMGTirePosition Position, float Pressure)
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return;
	}

	FMGTireState* Tire = nullptr;
	FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	switch (Position)
	{
		case EMGTirePosition::FrontLeft: Tire = &TireSet.FrontLeft; break;
		case EMGTirePosition::FrontRight: Tire = &TireSet.FrontRight; break;
		case EMGTirePosition::RearLeft: Tire = &TireSet.RearLeft; break;
		case EMGTirePosition::RearRight: Tire = &TireSet.RearRight; break;
	}

	if (Tire)
	{
		Tire->Pressure = FMath::Clamp(Pressure, 1.0f, 3.5f);
	}
}

void UMGTireSubsystem::ReportLockup(FName VehicleID, EMGTirePosition Position)
{
	OnTireLockup.Broadcast(VehicleID, Position);

	if (VehicleTelemetry.Contains(VehicleID))
	{
		VehicleTelemetry[VehicleID].Lockups++;
	}
}

void UMGTireSubsystem::ReportWheelspin(FName VehicleID, EMGTirePosition Position)
{
	OnTireWheelspin.Broadcast(VehicleID, Position);

	if (VehicleTelemetry.Contains(VehicleID))
	{
		VehicleTelemetry[VehicleID].Wheelspin++;
	}
}

void UMGTireSubsystem::ReportSurfaceContact(FName VehicleID, EMGTirePosition Position, EMGTrackSurface Surface)
{
	// Apply surface-specific effects
	if (!VehicleTires.Contains(VehicleID))
	{
		return;
	}

	FMGTireState* Tire = nullptr;
	FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	switch (Position)
	{
		case EMGTirePosition::FrontLeft: Tire = &TireSet.FrontLeft; break;
		case EMGTirePosition::FrontRight: Tire = &TireSet.FrontRight; break;
		case EMGTirePosition::RearLeft: Tire = &TireSet.RearLeft; break;
		case EMGTirePosition::RearRight: Tire = &TireSet.RearRight; break;
	}

	if (!Tire) return;

	// Different surfaces affect tires differently
	switch (Surface)
	{
		case EMGTrackSurface::Gravel:
		case EMGTrackSurface::Dirt:
			ApplyWear(VehicleID, Position, 0.001f * WearFactors.SurfaceWear);
			break;
		case EMGTrackSurface::Grass:
			CoolTire(VehicleID, Position, 2.0f);
			break;
		case EMGTrackSurface::Wet:
		case EMGTrackSurface::Puddle:
			CoolTire(VehicleID, Position, 5.0f);
			break;
		case EMGTrackSurface::Oil:
			// Immediate grip loss handled elsewhere
			break;
		default:
			break;
	}
}

void UMGTireSubsystem::ChangeTires(FName VehicleID, EMGTireCompoundType NewCompound)
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return;
	}

	FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	TireSet.FrontLeft.Compound = NewCompound;
	TireSet.FrontLeft.WearLevel = 1.0f;
	TireSet.FrontLeft.Temperature = Settings.AmbientTemperature;
	TireSet.FrontLeft.Condition = EMGTireCondition::Optimal;
	TireSet.FrontLeft.bFlat = false;
	TireSet.FrontLeft.LapsOnTire = 0.0f;

	TireSet.FrontRight.Compound = NewCompound;
	TireSet.FrontRight.WearLevel = 1.0f;
	TireSet.FrontRight.Temperature = Settings.AmbientTemperature;
	TireSet.FrontRight.Condition = EMGTireCondition::Optimal;
	TireSet.FrontRight.bFlat = false;
	TireSet.FrontRight.LapsOnTire = 0.0f;

	TireSet.RearLeft.Compound = NewCompound;
	TireSet.RearLeft.WearLevel = 1.0f;
	TireSet.RearLeft.Temperature = Settings.AmbientTemperature;
	TireSet.RearLeft.Condition = EMGTireCondition::Optimal;
	TireSet.RearLeft.bFlat = false;
	TireSet.RearLeft.LapsOnTire = 0.0f;

	TireSet.RearRight.Compound = NewCompound;
	TireSet.RearRight.WearLevel = 1.0f;
	TireSet.RearRight.Temperature = Settings.AmbientTemperature;
	TireSet.RearRight.Condition = EMGTireCondition::Optimal;
	TireSet.RearRight.bFlat = false;
	TireSet.RearRight.LapsOnTire = 0.0f;

	TireSet.bMixedCompounds = false;

	OnTiresChanged.Broadcast(VehicleID, NewCompound);
}

void UMGTireSubsystem::ChangeSingleTire(FName VehicleID, EMGTirePosition Position, EMGTireCompoundType NewCompound)
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return;
	}

	FMGTireState* Tire = nullptr;
	FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	switch (Position)
	{
		case EMGTirePosition::FrontLeft: Tire = &TireSet.FrontLeft; break;
		case EMGTirePosition::FrontRight: Tire = &TireSet.FrontRight; break;
		case EMGTirePosition::RearLeft: Tire = &TireSet.RearLeft; break;
		case EMGTirePosition::RearRight: Tire = &TireSet.RearRight; break;
	}

	if (Tire)
	{
		Tire->Compound = NewCompound;
		Tire->WearLevel = 1.0f;
		Tire->Temperature = Settings.AmbientTemperature;
		Tire->Condition = EMGTireCondition::Optimal;
		Tire->bFlat = false;
		Tire->LapsOnTire = 0.0f;
	}

	// Check for mixed compounds
	TireSet.bMixedCompounds = (TireSet.FrontLeft.Compound != TireSet.FrontRight.Compound ||
							   TireSet.RearLeft.Compound != TireSet.RearRight.Compound ||
							   TireSet.FrontLeft.Compound != TireSet.RearLeft.Compound);
}

void UMGTireSubsystem::ChangeFrontTires(FName VehicleID, EMGTireCompoundType NewCompound)
{
	ChangeSingleTire(VehicleID, EMGTirePosition::FrontLeft, NewCompound);
	ChangeSingleTire(VehicleID, EMGTirePosition::FrontRight, NewCompound);
}

void UMGTireSubsystem::ChangeRearTires(FName VehicleID, EMGTireCompoundType NewCompound)
{
	ChangeSingleTire(VehicleID, EMGTirePosition::RearLeft, NewCompound);
	ChangeSingleTire(VehicleID, EMGTirePosition::RearRight, NewCompound);
}

void UMGTireSubsystem::PunctureRepair(FName VehicleID, EMGTirePosition Position)
{
	ChangeSingleTire(VehicleID, Position, GetTireState(VehicleID, Position).Compound);
}

void UMGTireSubsystem::RegisterCompound(const FMGTireCompoundData& CompoundData)
{
	CompoundDatabase.Add(CompoundData.CompoundType, CompoundData);
}

FMGTireCompoundData UMGTireSubsystem::GetCompoundData(EMGTireCompoundType CompoundType) const
{
	if (CompoundDatabase.Contains(CompoundType))
	{
		return CompoundDatabase[CompoundType];
	}
	return FMGTireCompoundData();
}

TArray<FMGTireCompoundData> UMGTireSubsystem::GetAllCompounds() const
{
	TArray<FMGTireCompoundData> Compounds;
	for (const auto& Pair : CompoundDatabase)
	{
		Compounds.Add(Pair.Value);
	}
	return Compounds;
}

EMGTireCompoundType UMGTireSubsystem::GetRecommendedCompound(float TrackTemp, bool bWet) const
{
	if (bWet)
	{
		if (TrackTemp < 15.0f)
		{
			return EMGTireCompoundType::FullWet;
		}
		return EMGTireCompoundType::Intermediate;
	}

	if (TrackTemp < 25.0f)
	{
		return EMGTireCompoundType::Hard;
	}
	else if (TrackTemp < 35.0f)
	{
		return EMGTireCompoundType::Medium;
	}
	else if (TrackTemp < 45.0f)
	{
		return EMGTireCompoundType::Soft;
	}
	else
	{
		return EMGTireCompoundType::UltraSoft;
	}
}

int32 UMGTireSubsystem::GetExpectedTireLaps(EMGTireCompoundType CompoundType) const
{
	FMGTireCompoundData Compound = GetCompoundData(CompoundType);
	return Compound.ExpectedLaps;
}

float UMGTireSubsystem::CalculateGrip(const FMGTireState& TireState, const FMGTireCompoundData& Compound) const
{
	float TempGrip = GetGripFromTemperature(TireState.Temperature, Compound);
	float WearGrip = GetGripFromWear(TireState.WearLevel);

	return Compound.BaseGrip * TempGrip * WearGrip * Settings.GlobalGripMultiplier;
}

float UMGTireSubsystem::GetGripFromTemperature(float Temperature, const FMGTireCompoundData& Compound) const
{
	// Peak grip at optimal temperature
	float OptimalMid = (Compound.OptimalTempMin + Compound.OptimalTempMax) / 2.0f;

	if (Temperature < Compound.OptimalTempMin)
	{
		// Cold tires - reduced grip
		float ColdRatio = Temperature / Compound.OptimalTempMin;
		return 0.7f + (0.3f * ColdRatio);
	}
	else if (Temperature > Compound.OptimalTempMax)
	{
		// Overheating - reduced grip
		float OverTemp = Temperature - Compound.OptimalTempMax;
		float MaxOverTemp = 50.0f;
		float GripLoss = FMath::Min(OverTemp / MaxOverTemp, 0.4f);
		return 1.0f - GripLoss;
	}
	else
	{
		// In optimal window - peak grip
		float DistFromPeak = FMath::Abs(Temperature - Compound.PeakGripTemperature);
		float WindowHalf = (Compound.OptimalTempMax - Compound.OptimalTempMin) / 2.0f;
		float PeakBonus = 1.0f - (DistFromPeak / WindowHalf) * 0.05f;
		return PeakBonus;
	}
}

float UMGTireSubsystem::GetGripFromWear(float WearLevel) const
{
	// Grip drops off sharply when wear is critical
	if (WearLevel > 0.5f)
	{
		return 1.0f;
	}
	else if (WearLevel > 0.25f)
	{
		return 0.85f + (WearLevel - 0.25f) * 0.6f;
	}
	else if (WearLevel > 0.1f)
	{
		return 0.6f + (WearLevel - 0.1f) * 1.67f;
	}
	else
	{
		return 0.3f + WearLevel * 3.0f;
	}
}

float UMGTireSubsystem::GetSurfaceGripMultiplier(EMGTrackSurface Surface, EMGTireCompoundType Compound) const
{
	FMGTireCompoundData CompoundData = GetCompoundData(Compound);

	switch (Surface)
	{
		case EMGTrackSurface::Asphalt:
		case EMGTrackSurface::Concrete:
			return 1.0f;
		case EMGTrackSurface::Gravel:
			return 0.6f;
		case EMGTrackSurface::Dirt:
			return 0.5f;
		case EMGTrackSurface::Grass:
			return 0.4f;
		case EMGTrackSurface::Sand:
			return 0.3f;
		case EMGTrackSurface::Snow:
			return CompoundData.bStudded ? 0.6f : 0.2f;
		case EMGTrackSurface::Ice:
			return CompoundData.bStudded ? 0.4f : 0.1f;
		case EMGTrackSurface::Wet:
			return CompoundData.WetPerformance;
		case EMGTrackSurface::Puddle:
			return CompoundData.WetPerformance * 0.7f;
		case EMGTrackSurface::Oil:
			return 0.15f;
		default:
			return 1.0f;
	}
}

FMGTireTemperatureZone UMGTireSubsystem::GetTireTemperatureZones(FName VehicleID, EMGTirePosition Position) const
{
	FMGTireTemperatureZone Zones;
	FMGTireState TireState = GetTireState(VehicleID, Position);

	// Simplified - distribute temperature across zones
	Zones.MiddleTemp = TireState.Temperature;
	Zones.InnerTemp = TireState.Temperature * 0.95f;
	Zones.OuterTemp = TireState.Temperature * 1.05f;
	Zones.AverageTemp = (Zones.InnerTemp + Zones.MiddleTemp + Zones.OuterTemp) / 3.0f;
	Zones.TempSpread = Zones.OuterTemp - Zones.InnerTemp;

	FMGTireCompoundData Compound = GetCompoundData(TireState.Compound);
	Zones.bOverheating = Zones.AverageTemp > Compound.OptimalTempMax;
	Zones.bUndercooled = Zones.AverageTemp < Compound.OptimalTempMin;

	return Zones;
}

bool UMGTireSubsystem::IsTireOverheating(FName VehicleID, EMGTirePosition Position) const
{
	FMGTireState TireState = GetTireState(VehicleID, Position);
	FMGTireCompoundData Compound = GetCompoundData(TireState.Compound);
	return TireState.Temperature > Compound.OptimalTempMax;
}

bool UMGTireSubsystem::IsTireCold(FName VehicleID, EMGTirePosition Position) const
{
	FMGTireState TireState = GetTireState(VehicleID, Position);
	FMGTireCompoundData Compound = GetCompoundData(TireState.Compound);
	return TireState.Temperature < Compound.OptimalTempMin;
}

bool UMGTireSubsystem::IsTireInOptimalWindow(FName VehicleID, EMGTirePosition Position) const
{
	return !IsTireOverheating(VehicleID, Position) && !IsTireCold(VehicleID, Position);
}

FMGTireTelemetry UMGTireSubsystem::GetTireTelemetry(FName VehicleID) const
{
	if (VehicleTelemetry.Contains(VehicleID))
	{
		return VehicleTelemetry[VehicleID];
	}
	return FMGTireTelemetry();
}

void UMGTireSubsystem::RecordTelemetry(FName VehicleID)
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return;
	}

	if (!VehicleTelemetry.Contains(VehicleID))
	{
		FMGTireTelemetry NewTelemetry;
		NewTelemetry.VehicleID = VehicleID;
		VehicleTelemetry.Add(VehicleID, NewTelemetry);
	}

	FMGTireTelemetry& Telemetry = VehicleTelemetry[VehicleID];
	const FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];

	// Record peak temperatures
	Telemetry.PeakFrontLeftTemp = FMath::Max(Telemetry.PeakFrontLeftTemp, TireSet.FrontLeft.Temperature);
	Telemetry.PeakFrontRightTemp = FMath::Max(Telemetry.PeakFrontRightTemp, TireSet.FrontRight.Temperature);
	Telemetry.PeakRearLeftTemp = FMath::Max(Telemetry.PeakRearLeftTemp, TireSet.RearLeft.Temperature);
	Telemetry.PeakRearRightTemp = FMath::Max(Telemetry.PeakRearRightTemp, TireSet.RearRight.Temperature);

	// Record history (limit to last 100 entries per tire)
	Telemetry.TireHistory.Add(TireSet.FrontLeft);
	if (Telemetry.TireHistory.Num() > 400)
	{
		Telemetry.TireHistory.RemoveAt(0, 4);
	}
}

void UMGTireSubsystem::ClearTelemetry(FName VehicleID)
{
	VehicleTelemetry.Remove(VehicleID);
}

void UMGTireSubsystem::SetTireSettings(const FMGTireSettings& NewSettings)
{
	Settings = NewSettings;
}

void UMGTireSubsystem::SetWearFactors(const FMGTireWearFactors& Factors)
{
	WearFactors = Factors;
}

void UMGTireSubsystem::SetTrackTemperature(float Temperature)
{
	Settings.TrackTemperature = Temperature;
}

void UMGTireSubsystem::SetAmbientTemperature(float Temperature)
{
	Settings.AmbientTemperature = Temperature;
}

int32 UMGTireSubsystem::PredictTireLapsRemaining(FName VehicleID) const
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return 0;
	}

	const FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];
	float MinWear = FMath::Min(
		FMath::Min(TireSet.FrontLeft.WearLevel, TireSet.FrontRight.WearLevel),
		FMath::Min(TireSet.RearLeft.WearLevel, TireSet.RearRight.WearLevel)
	);

	// Estimate based on average wear rate
	FMGTireCompoundData Compound = GetCompoundData(TireSet.FrontLeft.Compound);
	float WearPerLap = 1.0f / Compound.ExpectedLaps;
	float CriticalThreshold = 0.2f;

	float WearToGo = MinWear - CriticalThreshold;
	if (WearToGo <= 0)
	{
		return 0;
	}

	return FMath::FloorToInt(WearToGo / WearPerLap);
}

float UMGTireSubsystem::PredictWearAfterLaps(FName VehicleID, int32 Laps) const
{
	if (!VehicleTires.Contains(VehicleID))
	{
		return 0.0f;
	}

	const FMGVehicleTireSet& TireSet = VehicleTires[VehicleID];
	FMGTireCompoundData Compound = GetCompoundData(TireSet.FrontLeft.Compound);

	float WearPerLap = 1.0f / Compound.ExpectedLaps;
	return FMath::Max(TireSet.AverageWear - (WearPerLap * Laps), 0.0f);
}

bool UMGTireSubsystem::ShouldChangeTires(FName VehicleID, int32 RemainingLaps) const
{
	int32 LapsRemaining = PredictTireLapsRemaining(VehicleID);
	return LapsRemaining < RemainingLaps;
}

void UMGTireSubsystem::SaveTireData()
{
	// Placeholder for save implementation
}

void UMGTireSubsystem::LoadTireData()
{
	// Placeholder for load implementation
}

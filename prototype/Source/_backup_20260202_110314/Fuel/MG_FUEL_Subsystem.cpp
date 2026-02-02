// Copyright Midnight Grind. All Rights Reserved.

#include "Fuel/MG_FUEL_Subsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGFuelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default settings
	Settings.bSimulateFuel = true;
	Settings.GlobalConsumptionMultiplier = 1.0f;
	Settings.bFuelAffectsWeight = true;
	Settings.FuelWeightPerLiter = 0.75f;
	Settings.bShowFuelAlerts = true;
	Settings.LowFuelThreshold = 0.25f;
	Settings.CriticalFuelThreshold = 0.1f;
	Settings.RefuelRate = 10.0f;

	InitializeDefaultFuelTypes();
	InitializeDefaultFuelModes();
	LoadFuelData();

	// Start fuel tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FuelTickHandle,
			this,
			&UMGFuelSubsystem::OnFuelTick,
			0.1f,
			true
		);
	}
}

void UMGFuelSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FuelTickHandle);
	}

	SaveFuelData();

	Super::Deinitialize();
}

bool UMGFuelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGFuelSubsystem::InitializeDefaultFuelTypes()
{
	// Regular
	FMGFuelTypeData Regular;
	Regular.FuelType = EMGFuelType::Regular;
	Regular.FuelID = TEXT("Regular");
	Regular.DisplayName = FText::FromString(TEXT("Regular"));
	Regular.EnergyDensity = 1.0f;
	Regular.PowerMultiplier = 1.0f;
	Regular.EfficiencyMultiplier = 1.0f;
	Regular.CostPerLiter = 1.0f;
	Regular.OctaneRating = 87.0f;
	Regular.FuelColor = FLinearColor::Green;
	FuelTypes.Add(EMGFuelType::Regular, Regular);

	// Premium
	FMGFuelTypeData Premium;
	Premium.FuelType = EMGFuelType::Premium;
	Premium.FuelID = TEXT("Premium");
	Premium.DisplayName = FText::FromString(TEXT("Premium"));
	Premium.EnergyDensity = 1.05f;
	Premium.PowerMultiplier = 1.05f;
	Premium.EfficiencyMultiplier = 1.02f;
	Premium.CostPerLiter = 1.3f;
	Premium.OctaneRating = 93.0f;
	Premium.FuelColor = FLinearColor(0.0f, 0.8f, 0.0f, 1.0f);
	FuelTypes.Add(EMGFuelType::Premium, Premium);

	// Racing
	FMGFuelTypeData Racing;
	Racing.FuelType = EMGFuelType::Racing;
	Racing.FuelID = TEXT("Racing");
	Racing.DisplayName = FText::FromString(TEXT("Racing Fuel"));
	Racing.EnergyDensity = 1.15f;
	Racing.PowerMultiplier = 1.12f;
	Racing.EfficiencyMultiplier = 0.9f;
	Racing.CostPerLiter = 3.0f;
	Racing.OctaneRating = 110.0f;
	Racing.bRequiresSpecialTank = true;
	Racing.FuelColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
	FuelTypes.Add(EMGFuelType::Racing, Racing);

	// E85
	FMGFuelTypeData E85;
	E85.FuelType = EMGFuelType::E85;
	E85.FuelID = TEXT("E85");
	E85.DisplayName = FText::FromString(TEXT("E85 Ethanol"));
	E85.EnergyDensity = 0.75f;
	E85.PowerMultiplier = 1.08f;
	E85.EfficiencyMultiplier = 0.7f;
	E85.CostPerLiter = 0.85f;
	E85.OctaneRating = 105.0f;
	E85.EnvironmentalImpact = 0.6f;
	E85.FuelColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
	FuelTypes.Add(EMGFuelType::E85, E85);

	// Nitromethane
	FMGFuelTypeData Nitro;
	Nitro.FuelType = EMGFuelType::Nitromethane;
	Nitro.FuelID = TEXT("Nitromethane");
	Nitro.DisplayName = FText::FromString(TEXT("Nitromethane"));
	Nitro.EnergyDensity = 0.5f;
	Nitro.PowerMultiplier = 1.5f;
	Nitro.EfficiencyMultiplier = 0.3f;
	Nitro.CostPerLiter = 10.0f;
	Nitro.bRequiresSpecialTank = true;
	Nitro.FuelColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
	FuelTypes.Add(EMGFuelType::Nitromethane, Nitro);
}

void UMGFuelSubsystem::InitializeDefaultFuelModes()
{
	// Standard
	FMGFuelModeSettings Standard;
	Standard.Mode = EMGFuelMode::Standard;
	Standard.ModeName = FText::FromString(TEXT("Standard"));
	Standard.PowerMultiplier = 1.0f;
	Standard.ConsumptionMultiplier = 1.0f;
	Standard.MaxRPMMultiplier = 1.0f;
	Standard.bAllowNitro = true;
	FuelModes.Add(EMGFuelMode::Standard, Standard);

	// Economy
	FMGFuelModeSettings Economy;
	Economy.Mode = EMGFuelMode::Economy;
	Economy.ModeName = FText::FromString(TEXT("Economy"));
	Economy.PowerMultiplier = 0.85f;
	Economy.ConsumptionMultiplier = 0.7f;
	Economy.MaxRPMMultiplier = 0.9f;
	Economy.ThrottleResponseMultiplier = 0.8f;
	Economy.bAllowNitro = false;
	Economy.FuelSaveTarget = 0.2f;
	FuelModes.Add(EMGFuelMode::Economy, Economy);

	// Performance
	FMGFuelModeSettings Performance;
	Performance.Mode = EMGFuelMode::Performance;
	Performance.ModeName = FText::FromString(TEXT("Performance"));
	Performance.PowerMultiplier = 1.1f;
	Performance.ConsumptionMultiplier = 1.3f;
	Performance.MaxRPMMultiplier = 1.05f;
	Performance.ThrottleResponseMultiplier = 1.1f;
	Performance.bAllowNitro = true;
	FuelModes.Add(EMGFuelMode::Performance, Performance);

	// Qualifying
	FMGFuelModeSettings Qualifying;
	Qualifying.Mode = EMGFuelMode::Qualifying;
	Qualifying.ModeName = FText::FromString(TEXT("Qualifying"));
	Qualifying.PowerMultiplier = 1.15f;
	Qualifying.ConsumptionMultiplier = 1.5f;
	Qualifying.MaxRPMMultiplier = 1.1f;
	Qualifying.ThrottleResponseMultiplier = 1.15f;
	Qualifying.bAllowNitro = true;
	FuelModes.Add(EMGFuelMode::Qualifying, Qualifying);

	// Attack
	FMGFuelModeSettings Attack;
	Attack.Mode = EMGFuelMode::Attack;
	Attack.ModeName = FText::FromString(TEXT("Attack"));
	Attack.PowerMultiplier = 1.2f;
	Attack.ConsumptionMultiplier = 1.8f;
	Attack.MaxRPMMultiplier = 1.15f;
	Attack.bAllowNitro = true;
	FuelModes.Add(EMGFuelMode::Attack, Attack);

	// Defend
	FMGFuelModeSettings Defend;
	Defend.Mode = EMGFuelMode::Defend;
	Defend.ModeName = FText::FromString(TEXT("Defend"));
	Defend.PowerMultiplier = 0.95f;
	Defend.ConsumptionMultiplier = 0.85f;
	Defend.MaxRPMMultiplier = 0.95f;
	Defend.bAllowNitro = true;
	FuelModes.Add(EMGFuelMode::Defend, Defend);

	// Limp
	FMGFuelModeSettings Limp;
	Limp.Mode = EMGFuelMode::Limp;
	Limp.ModeName = FText::FromString(TEXT("Limp"));
	Limp.PowerMultiplier = 0.5f;
	Limp.ConsumptionMultiplier = 0.4f;
	Limp.MaxRPMMultiplier = 0.6f;
	Limp.ThrottleResponseMultiplier = 0.5f;
	Limp.bAllowNitro = false;
	Limp.FuelSaveTarget = 0.5f;
	FuelModes.Add(EMGFuelMode::Limp, Limp);
}

void UMGFuelSubsystem::OnFuelTick()
{
	UpdateRefueling(0.1f);
	UpdateFuelStates();
}

void UMGFuelSubsystem::UpdateRefueling(float DeltaTime)
{
	for (auto& Pair : RefuelingVehicles)
	{
		if (!Pair.Value) continue;

		FName VehicleID = Pair.Key;
		if (!VehicleFuelStates.Contains(VehicleID)) continue;

		FMGVehicleFuelState& State = VehicleFuelStates[VehicleID];
		float TargetFuel = RefuelingTargets.Contains(VehicleID) ? RefuelingTargets[VehicleID] : State.TankCapacity;

		// Add fuel at refuel rate
		float FuelToAdd = Settings.RefuelRate * DeltaTime;
		float RemainingToTarget = TargetFuel - State.CurrentFuel;

		if (FuelToAdd >= RemainingToTarget)
		{
			FuelToAdd = RemainingToTarget;
			Pair.Value = false; // Stop refueling
		}

		if (FuelToAdd > 0)
		{
			State.CurrentFuel += FuelToAdd;
			State.CurrentFuel = FMath::Min(State.CurrentFuel, State.TankCapacity);
			State.FuelPercentage = State.CurrentFuel / State.TankCapacity;

			// Update progress
			if (RefuelingProgress.Contains(VehicleID))
			{
				float TotalToAdd = TargetFuel - RefuelingProgress[VehicleID];
				RefuelingProgress[VehicleID] = State.CurrentFuel;
			}

			OnFuelAdded.Broadcast(VehicleID, FuelToAdd);
		}
	}
}

void UMGFuelSubsystem::UpdateFuelStates()
{
	for (auto& Pair : VehicleFuelStates)
	{
		FMGVehicleFuelState& State = Pair.Value;
		EMGFuelState OldState = State.State;

		State.FuelPercentage = (State.TankCapacity > 0) ? State.CurrentFuel / State.TankCapacity : 0.0f;
		State.State = CalculateFuelState(State.FuelPercentage);

		// Update fuel weight
		if (Settings.bFuelAffectsWeight)
		{
			State.FuelWeight = State.CurrentFuel * Settings.FuelWeightPerLiter;
		}

		if (State.State != OldState)
		{
			OnFuelStateChanged.Broadcast(Pair.Key, OldState, State.State);
		}

		CheckFuelAlerts(Pair.Key);
	}
}

void UMGFuelSubsystem::CheckFuelAlerts(FName VehicleID)
{
	if (!Settings.bShowFuelAlerts) return;
	if (!VehicleFuelStates.Contains(VehicleID)) return;

	const FMGVehicleFuelState& State = VehicleFuelStates[VehicleID];

	if (State.FuelPercentage <= Settings.CriticalFuelThreshold)
	{
		OnFuelAlert.Broadcast(VehicleID, EMGFuelAlert::CriticalFuel);
	}
	else if (State.FuelPercentage <= Settings.LowFuelThreshold)
	{
		OnFuelAlert.Broadcast(VehicleID, EMGFuelAlert::LowFuel);
	}
}

EMGFuelState UMGFuelSubsystem::CalculateFuelState(float Percentage) const
{
	if (Percentage <= 0.0f)
	{
		return EMGFuelState::Empty;
	}
	else if (Percentage <= Settings.CriticalFuelThreshold)
	{
		return EMGFuelState::Critical;
	}
	else if (Percentage <= Settings.LowFuelThreshold)
	{
		return EMGFuelState::Low;
	}
	else if (Percentage >= 0.9f)
	{
		return EMGFuelState::Full;
	}
	else
	{
		return EMGFuelState::Adequate;
	}
}

void UMGFuelSubsystem::RegisterVehicle(FName VehicleID, float TankCapacity, EMGFuelType FuelType)
{
	FMGVehicleFuelState NewState;
	NewState.VehicleID = VehicleID;
	NewState.TankCapacity = TankCapacity;
	NewState.CurrentFuel = TankCapacity;
	NewState.FuelPercentage = 1.0f;
	NewState.CurrentFuelType = FuelType;
	NewState.State = EMGFuelState::Full;
	NewState.FuelMode = EMGFuelMode::Standard;
	NewState.FuelWeight = TankCapacity * Settings.FuelWeightPerLiter;

	VehicleFuelStates.Add(VehicleID, NewState);

	FMGFuelConsumptionFactors DefaultFactors;
	VehicleConsumptionFactors.Add(VehicleID, DefaultFactors);

	FMGFuelTelemetry DefaultTelemetry;
	DefaultTelemetry.VehicleID = VehicleID;
	VehicleTelemetry.Add(VehicleID, DefaultTelemetry);
}

void UMGFuelSubsystem::UnregisterVehicle(FName VehicleID)
{
	VehicleFuelStates.Remove(VehicleID);
	VehicleConsumptionFactors.Remove(VehicleID);
	VehicleTelemetry.Remove(VehicleID);
	VehicleStrategies.Remove(VehicleID);
	VehicleLapStartFuel.Remove(VehicleID);
	RefuelingVehicles.Remove(VehicleID);
	RefuelingTargets.Remove(VehicleID);
	RefuelingProgress.Remove(VehicleID);
}

void UMGFuelSubsystem::SetVehicleFuel(FName VehicleID, float FuelAmount)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		FMGVehicleFuelState& State = VehicleFuelStates[VehicleID];
		State.CurrentFuel = FMath::Clamp(FuelAmount, 0.0f, State.TankCapacity);
		State.FuelPercentage = State.CurrentFuel / State.TankCapacity;
	}
}

void UMGFuelSubsystem::SetTankCapacity(FName VehicleID, float Capacity)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		VehicleFuelStates[VehicleID].TankCapacity = Capacity;
	}
}

FMGVehicleFuelState UMGFuelSubsystem::GetFuelState(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID];
	}
	return FMGVehicleFuelState();
}

float UMGFuelSubsystem::GetCurrentFuel(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].CurrentFuel;
	}
	return 0.0f;
}

float UMGFuelSubsystem::GetFuelPercentage(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].FuelPercentage;
	}
	return 0.0f;
}

EMGFuelState UMGFuelSubsystem::GetFuelStatus(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].State;
	}
	return EMGFuelState::Empty;
}

float UMGFuelSubsystem::GetEstimatedRange(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].EstimatedRange;
	}
	return 0.0f;
}

float UMGFuelSubsystem::GetEstimatedLapsRemaining(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].EstimatedLapsRemaining;
	}
	return 0.0f;
}

bool UMGFuelSubsystem::CanFinishRace(FName VehicleID, int32 RemainingLaps) const
{
	float EstimatedLaps = GetEstimatedLapsRemaining(VehicleID);
	return EstimatedLaps >= RemainingLaps;
}

float UMGFuelSubsystem::GetFuelWeight(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].FuelWeight;
	}
	return 0.0f;
}

void UMGFuelSubsystem::UpdateFuelConsumption(FName VehicleID, float Throttle, float Speed, float RPM, int32 Gear, bool bNitroActive, float DeltaTime)
{
	if (!Settings.bSimulateFuel) return;
	if (!VehicleFuelStates.Contains(VehicleID)) return;

	FMGVehicleFuelState& State = VehicleFuelStates[VehicleID];
	FMGFuelConsumptionFactors Factors = GetConsumptionFactors(VehicleID);
	FMGFuelModeSettings ModeSettings = GetFuelModeSettings(State.FuelMode);

	// Calculate base consumption
	float Consumption = Factors.BaseConsumption;

	// Apply throttle factor
	Consumption *= FMath::Lerp(Factors.IdleConsumption, Factors.ThrottleMultiplier, Throttle);

	// Apply speed factor
	float SpeedFactor = 1.0f + (Speed / 200.0f) * (Factors.SpeedMultiplier - 1.0f);
	Consumption *= SpeedFactor;

	// Apply RPM factor
	float RPMFactor = 1.0f + (RPM / 8000.0f) * (Factors.RPMMultiplier - 1.0f);
	Consumption *= RPMFactor;

	// Apply nitro factor
	if (bNitroActive && ModeSettings.bAllowNitro)
	{
		Consumption *= Factors.NitroMultiplier;
	}

	// Apply fuel mode multiplier
	Consumption *= ModeSettings.ConsumptionMultiplier;

	// Apply fuel type efficiency
	FMGFuelTypeData FuelData = GetFuelTypeData(State.CurrentFuelType);
	Consumption /= FuelData.EfficiencyMultiplier;

	// Apply global multiplier
	Consumption *= Settings.GlobalConsumptionMultiplier;

	// Apply fuel save if active
	if (State.bFuelSaveActive)
	{
		Consumption *= (1.0f - State.FuelSavePercentage);
	}

	// Apply time factor
	float FuelUsed = Consumption * DeltaTime;

	// Update state
	State.InstantConsumption = Consumption;
	State.TotalFuelUsed += FuelUsed;
	State.DistanceTraveled += Speed * DeltaTime;

	// Calculate average consumption
	if (State.DistanceTraveled > 0)
	{
		State.AverageConsumption = State.TotalFuelUsed / (State.DistanceTraveled / 1000.0f); // Per km
	}

	// Consume the fuel
	ConsumeFuel(VehicleID, FuelUsed);

	// Update estimated range
	if (State.AverageConsumption > 0)
	{
		State.EstimatedRange = State.CurrentFuel / State.AverageConsumption * 1000.0f;
	}

	// Update telemetry
	if (VehicleTelemetry.Contains(VehicleID))
	{
		VehicleTelemetry[VehicleID].ConsumptionHistory.Add(State.InstantConsumption);
		if (VehicleTelemetry[VehicleID].ConsumptionHistory.Num() > 600)
		{
			VehicleTelemetry[VehicleID].ConsumptionHistory.RemoveAt(0);
		}

		VehicleTelemetry[VehicleID].PeakConsumption = FMath::Max(
			VehicleTelemetry[VehicleID].PeakConsumption,
			State.InstantConsumption
		);
	}
}

void UMGFuelSubsystem::ConsumeFuel(FName VehicleID, float Amount)
{
	if (!VehicleFuelStates.Contains(VehicleID)) return;

	FMGVehicleFuelState& State = VehicleFuelStates[VehicleID];
	float PreviousFuel = State.CurrentFuel;

	State.CurrentFuel -= Amount;
	State.CurrentFuel = FMath::Max(State.CurrentFuel, 0.0f);
	State.FuelPercentage = State.CurrentFuel / State.TankCapacity;

	float ActualConsumed = PreviousFuel - State.CurrentFuel;
	if (ActualConsumed > 0)
	{
		OnFuelConsumed.Broadcast(VehicleID, ActualConsumed);
	}

	if (State.CurrentFuel <= 0.0f && PreviousFuel > 0.0f)
	{
		OnFuelEmpty.Broadcast(VehicleID);
	}
}

float UMGFuelSubsystem::GetInstantConsumption(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].InstantConsumption;
	}
	return 0.0f;
}

float UMGFuelSubsystem::GetAverageConsumption(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].AverageConsumption;
	}
	return 0.0f;
}

float UMGFuelSubsystem::GetConsumptionPerLap(FName VehicleID) const
{
	if (!VehicleTelemetry.Contains(VehicleID)) return 0.0f;

	const FMGFuelTelemetry& Telemetry = VehicleTelemetry[VehicleID];
	if (Telemetry.LapConsumption.Num() == 0) return 0.0f;

	float Total = 0.0f;
	for (float Lap : Telemetry.LapConsumption)
	{
		Total += Lap;
	}
	return Total / Telemetry.LapConsumption.Num();
}

void UMGFuelSubsystem::SetConsumptionFactors(FName VehicleID, const FMGFuelConsumptionFactors& Factors)
{
	VehicleConsumptionFactors.Add(VehicleID, Factors);
}

FMGFuelConsumptionFactors UMGFuelSubsystem::GetConsumptionFactors(FName VehicleID) const
{
	if (VehicleConsumptionFactors.Contains(VehicleID))
	{
		return VehicleConsumptionFactors[VehicleID];
	}
	return FMGFuelConsumptionFactors();
}

void UMGFuelSubsystem::SetFuelMode(FName VehicleID, EMGFuelMode Mode)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		VehicleFuelStates[VehicleID].FuelMode = Mode;
		OnFuelModeChanged.Broadcast(VehicleID, Mode);
	}
}

EMGFuelMode UMGFuelSubsystem::GetFuelMode(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].FuelMode;
	}
	return EMGFuelMode::Standard;
}

void UMGFuelSubsystem::CycleFuelMode(FName VehicleID, bool bForward)
{
	if (!VehicleFuelStates.Contains(VehicleID)) return;

	EMGFuelMode CurrentMode = VehicleFuelStates[VehicleID].FuelMode;
	TArray<EMGFuelMode> Modes = GetAvailableFuelModes();

	if (Modes.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = Modes.Find(CurrentMode);
	if (CurrentIndex == INDEX_NONE) CurrentIndex = 0;

	int32 NewIndex = bForward ?
		(CurrentIndex + 1) % Modes.Num() :
		(CurrentIndex - 1 + Modes.Num()) % Modes.Num();

	SetFuelMode(VehicleID, Modes[NewIndex]);
}

FMGFuelModeSettings UMGFuelSubsystem::GetFuelModeSettings(EMGFuelMode Mode) const
{
	if (FuelModes.Contains(Mode))
	{
		return FuelModes[Mode];
	}
	return FMGFuelModeSettings();
}

void UMGFuelSubsystem::SetFuelModeSettings(EMGFuelMode Mode, const FMGFuelModeSettings& ModeSettings)
{
	FuelModes.Add(Mode, ModeSettings);
}

TArray<EMGFuelMode> UMGFuelSubsystem::GetAvailableFuelModes() const
{
	TArray<EMGFuelMode> Modes;
	for (const auto& Pair : FuelModes)
	{
		Modes.Add(Pair.Key);
	}
	return Modes;
}

void UMGFuelSubsystem::ActivateFuelSave(FName VehicleID, float TargetPercentage)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		VehicleFuelStates[VehicleID].bFuelSaveActive = true;
		VehicleFuelStates[VehicleID].FuelSavePercentage = FMath::Clamp(TargetPercentage, 0.0f, 0.5f);
	}
}

void UMGFuelSubsystem::DeactivateFuelSave(FName VehicleID)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		VehicleFuelStates[VehicleID].bFuelSaveActive = false;
		VehicleFuelStates[VehicleID].FuelSavePercentage = 0.0f;
	}
}

bool UMGFuelSubsystem::IsFuelSaveActive(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].bFuelSaveActive;
	}
	return false;
}

float UMGFuelSubsystem::GetFuelSaveAmount(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].FuelSavePercentage;
	}
	return 0.0f;
}

void UMGFuelSubsystem::AddFuel(FName VehicleID, float Amount)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		FMGVehicleFuelState& State = VehicleFuelStates[VehicleID];
		State.CurrentFuel = FMath::Min(State.CurrentFuel + Amount, State.TankCapacity);
		OnFuelAdded.Broadcast(VehicleID, Amount);
	}
}

void UMGFuelSubsystem::FillTank(FName VehicleID)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		FMGVehicleFuelState& State = VehicleFuelStates[VehicleID];
		float AmountToAdd = State.TankCapacity - State.CurrentFuel;
		State.CurrentFuel = State.TankCapacity;
		OnFuelAdded.Broadcast(VehicleID, AmountToAdd);
	}
}

void UMGFuelSubsystem::StartRefueling(FName VehicleID, float TargetAmount)
{
	if (!VehicleFuelStates.Contains(VehicleID)) return;

	RefuelingVehicles.Add(VehicleID, true);
	RefuelingTargets.Add(VehicleID, FMath::Min(TargetAmount, VehicleFuelStates[VehicleID].TankCapacity));
	RefuelingProgress.Add(VehicleID, VehicleFuelStates[VehicleID].CurrentFuel);
}

void UMGFuelSubsystem::StopRefueling(FName VehicleID)
{
	RefuelingVehicles.Add(VehicleID, false);
}

bool UMGFuelSubsystem::IsRefueling(FName VehicleID) const
{
	return RefuelingVehicles.Contains(VehicleID) && RefuelingVehicles[VehicleID];
}

float UMGFuelSubsystem::GetRefuelingProgress(FName VehicleID) const
{
	if (!RefuelingProgress.Contains(VehicleID) || !RefuelingTargets.Contains(VehicleID)) return 0.0f;

	float Start = RefuelingProgress[VehicleID];
	float Target = RefuelingTargets[VehicleID];
	float Current = GetCurrentFuel(VehicleID);

	float TotalToAdd = Target - Start;
	if (TotalToAdd <= 0) return 1.0f;

	return (Current - Start) / TotalToAdd;
}

float UMGFuelSubsystem::CalculateRefuelTime(float Amount) const
{
	return (Settings.RefuelRate > KINDA_SMALL_NUMBER) ? Amount / Settings.RefuelRate : 0.0f;
}

void UMGFuelSubsystem::RegisterFuelType(const FMGFuelTypeData& FuelData)
{
	FuelTypes.Add(FuelData.FuelType, FuelData);
}

FMGFuelTypeData UMGFuelSubsystem::GetFuelTypeData(EMGFuelType Type) const
{
	if (FuelTypes.Contains(Type))
	{
		return FuelTypes[Type];
	}
	return FMGFuelTypeData();
}

void UMGFuelSubsystem::SetVehicleFuelType(FName VehicleID, EMGFuelType Type)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		VehicleFuelStates[VehicleID].CurrentFuelType = Type;
	}
}

EMGFuelType UMGFuelSubsystem::GetVehicleFuelType(FName VehicleID) const
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		return VehicleFuelStates[VehicleID].CurrentFuelType;
	}
	return EMGFuelType::Regular;
}

TArray<FMGFuelTypeData> UMGFuelSubsystem::GetAllFuelTypes() const
{
	TArray<FMGFuelTypeData> Types;
	for (const auto& Pair : FuelTypes)
	{
		Types.Add(Pair.Value);
	}
	return Types;
}

void UMGFuelSubsystem::SetFuelStrategy(FName VehicleID, const FMGFuelStrategy& Strategy)
{
	VehicleStrategies.Add(VehicleID, Strategy);
}

FMGFuelStrategy UMGFuelSubsystem::GetFuelStrategy(FName VehicleID) const
{
	if (VehicleStrategies.Contains(VehicleID))
	{
		return VehicleStrategies[VehicleID];
	}
	return FMGFuelStrategy();
}

FMGFuelStrategy UMGFuelSubsystem::CalculateOptimalStrategy(FName VehicleID, int32 TotalLaps, float LapLength) const
{
	FMGFuelStrategy Strategy;
	Strategy.StrategyName = TEXT("Optimal");

	float FuelPerLap = GetConsumptionPerLap(VehicleID);
	if (FuelPerLap <= 0) FuelPerLap = 2.5f; // Default estimate

	float TotalFuelNeeded = FuelPerLap * TotalLaps + Strategy.TargetFuelAtFinish;
	float TankCapacity = VehicleFuelStates.Contains(VehicleID) ?
		VehicleFuelStates[VehicleID].TankCapacity : 60.0f;

	// Calculate number of stops needed
	if (TotalFuelNeeded <= TankCapacity)
	{
		// No stops needed
		Strategy.StartingFuel = TotalFuelNeeded;
	}
	else if (TankCapacity > KINDA_SMALL_NUMBER)
	{
		// Need at least one stop
		int32 NumStops = FMath::CeilToInt(TotalFuelNeeded / TankCapacity) - 1;

		// Calculate optimal pit lap
		int32 LapsPerStint = TotalLaps / (NumStops + 1);

		for (int32 i = 0; i < NumStops; i++)
		{
			Strategy.PlannedPitLaps.Add((i + 1) * LapsPerStint);
			Strategy.PlannedFuelLoads.Add(TankCapacity);
		}

		Strategy.StartingFuel = TankCapacity;
	}

	Strategy.EstimatedConsumptionPerLap = FuelPerLap;

	return Strategy;
}

float UMGFuelSubsystem::CalculateRequiredFuel(FName VehicleID, int32 Laps) const
{
	float FuelPerLap = GetConsumptionPerLap(VehicleID);
	if (FuelPerLap <= 0) FuelPerLap = 2.5f;

	return FuelPerLap * Laps + 2.0f; // Plus reserve
}

int32 UMGFuelSubsystem::GetRecommendedPitLap(FName VehicleID, int32 RemainingLaps) const
{
	float EstimatedLaps = GetEstimatedLapsRemaining(VehicleID);

	if (EstimatedLaps >= RemainingLaps)
	{
		return -1; // No pit needed
	}

	// Recommend pitting with 1 lap buffer
	return RemainingLaps - FMath::FloorToInt(EstimatedLaps) + 1;
}

FMGFuelTelemetry UMGFuelSubsystem::GetFuelTelemetry(FName VehicleID) const
{
	if (VehicleTelemetry.Contains(VehicleID))
	{
		return VehicleTelemetry[VehicleID];
	}
	return FMGFuelTelemetry();
}

void UMGFuelSubsystem::RecordLapFuelUsage(FName VehicleID, int32 LapNumber)
{
	if (!VehicleFuelStates.Contains(VehicleID)) return;
	if (!VehicleLapStartFuel.Contains(VehicleID)) return;

	float StartFuel = VehicleLapStartFuel[VehicleID];
	float CurrentFuel = VehicleFuelStates[VehicleID].CurrentFuel;
	float LapUsage = StartFuel - CurrentFuel;

	if (VehicleTelemetry.Contains(VehicleID))
	{
		VehicleTelemetry[VehicleID].LapConsumption.Add(LapUsage);
	}

	OnLapFuelUsage.Broadcast(VehicleID, LapNumber, LapUsage);
}

void UMGFuelSubsystem::ResetTelemetry(FName VehicleID)
{
	if (VehicleTelemetry.Contains(VehicleID))
	{
		VehicleTelemetry[VehicleID] = FMGFuelTelemetry();
		VehicleTelemetry[VehicleID].VehicleID = VehicleID;
	}
}

void UMGFuelSubsystem::OnLapStarted(FName VehicleID, int32 LapNumber)
{
	if (VehicleFuelStates.Contains(VehicleID))
	{
		VehicleLapStartFuel.Add(VehicleID, VehicleFuelStates[VehicleID].CurrentFuel);
	}
}

void UMGFuelSubsystem::OnLapCompleted(FName VehicleID, int32 LapNumber)
{
	RecordLapFuelUsage(VehicleID, LapNumber);

	// Update estimated laps remaining
	if (VehicleFuelStates.Contains(VehicleID) && VehicleTelemetry.Contains(VehicleID))
	{
		FMGVehicleFuelState& State = VehicleFuelStates[VehicleID];
		float FuelPerLap = GetConsumptionPerLap(VehicleID);
		if (FuelPerLap > 0)
		{
			State.EstimatedLapsRemaining = State.CurrentFuel / FuelPerLap;
		}
	}

	// Set lap start fuel for next lap
	OnLapStarted(VehicleID, LapNumber + 1);
}

void UMGFuelSubsystem::SetFuelSettings(const FMGFuelSettings& NewSettings)
{
	Settings = NewSettings;
}

void UMGFuelSubsystem::SaveFuelData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Fuel");
	IFileManager::Get().MakeDirectory(*SaveDir, true);

	FString FilePath = SaveDir / TEXT("fuel_stats.dat");

	FBufferArchive SaveArchive;

	// Save version for future compatibility
	int32 Version = 1;
	SaveArchive << Version;

	// Save telemetry data
	int32 TelemetryCount = VehicleTelemetry.Num();
	SaveArchive << TelemetryCount;

	for (auto& Pair : VehicleTelemetry)
	{
		FString VehicleIDStr = Pair.Key.ToString();
		SaveArchive << VehicleIDStr;

		FMGFuelTelemetry& Telemetry = Pair.Value;
		SaveArchive << Telemetry.TotalFuelConsumed;
		SaveArchive << Telemetry.TotalDistanceCovered;
		SaveArchive << Telemetry.BestEconomy;
		SaveArchive << Telemetry.PeakConsumption;
		SaveArchive << Telemetry.TotalRefuels;
		SaveArchive << Telemetry.TotalFuelAdded;

		// Save lap consumption history (limited)
		int32 LapCount = FMath::Min(Telemetry.LapConsumption.Num(), 100);
		SaveArchive << LapCount;
		for (int32 i = 0; i < LapCount; i++)
		{
			SaveArchive << Telemetry.LapConsumption[i];
		}
	}

	// Save strategies
	int32 StrategyCount = VehicleStrategies.Num();
	SaveArchive << StrategyCount;

	for (auto& Pair : VehicleStrategies)
	{
		FString VehicleIDStr = Pair.Key.ToString();
		SaveArchive << VehicleIDStr;

		FMGFuelStrategy& Strategy = Pair.Value;
		SaveArchive << Strategy.StrategyName;
		SaveArchive << Strategy.StartingFuel;
		SaveArchive << Strategy.TargetFuelAtFinish;
		SaveArchive << Strategy.EstimatedConsumptionPerLap;

		int32 PitLapCount = Strategy.PlannedPitLaps.Num();
		SaveArchive << PitLapCount;
		for (int32 Lap : Strategy.PlannedPitLaps)
		{
			SaveArchive << Lap;
		}

		int32 FuelLoadCount = Strategy.PlannedFuelLoads.Num();
		SaveArchive << FuelLoadCount;
		for (float Load : Strategy.PlannedFuelLoads)
		{
			SaveArchive << Load;
		}
	}

	if (SaveArchive.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
	}

	SaveArchive.FlushCache();
	SaveArchive.Empty();
}

void UMGFuelSubsystem::LoadFuelData()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Fuel") / TEXT("fuel_stats.dat");

	TArray<uint8> LoadData;
	if (!FFileHelper::LoadFileToArray(LoadData, *FilePath))
	{
		return;
	}

	FMemoryReader LoadArchive(LoadData, true);

	int32 Version;
	LoadArchive << Version;

	if (Version != 1)
	{
		return;
	}

	// Load telemetry data
	int32 TelemetryCount;
	LoadArchive << TelemetryCount;

	for (int32 i = 0; i < TelemetryCount; i++)
	{
		FString VehicleIDStr;
		LoadArchive << VehicleIDStr;
		FName VehicleID(*VehicleIDStr);

		FMGFuelTelemetry Telemetry;
		Telemetry.VehicleID = VehicleID;
		LoadArchive << Telemetry.TotalFuelConsumed;
		LoadArchive << Telemetry.TotalDistanceCovered;
		LoadArchive << Telemetry.BestEconomy;
		LoadArchive << Telemetry.PeakConsumption;
		LoadArchive << Telemetry.TotalRefuels;
		LoadArchive << Telemetry.TotalFuelAdded;

		int32 LapCount;
		LoadArchive << LapCount;
		for (int32 l = 0; l < LapCount; l++)
		{
			float LapConsumption;
			LoadArchive << LapConsumption;
			Telemetry.LapConsumption.Add(LapConsumption);
		}

		VehicleTelemetry.Add(VehicleID, Telemetry);
	}

	// Load strategies
	int32 StrategyCount;
	LoadArchive << StrategyCount;

	for (int32 i = 0; i < StrategyCount; i++)
	{
		FString VehicleIDStr;
		LoadArchive << VehicleIDStr;
		FName VehicleID(*VehicleIDStr);

		FMGFuelStrategy Strategy;
		LoadArchive << Strategy.StrategyName;
		LoadArchive << Strategy.StartingFuel;
		LoadArchive << Strategy.TargetFuelAtFinish;
		LoadArchive << Strategy.EstimatedConsumptionPerLap;

		int32 PitLapCount;
		LoadArchive << PitLapCount;
		for (int32 p = 0; p < PitLapCount; p++)
		{
			int32 Lap;
			LoadArchive << Lap;
			Strategy.PlannedPitLaps.Add(Lap);
		}

		int32 FuelLoadCount;
		LoadArchive << FuelLoadCount;
		for (int32 f = 0; f < FuelLoadCount; f++)
		{
			float Load;
			LoadArchive << Load;
			Strategy.PlannedFuelLoads.Add(Load);
		}

		VehicleStrategies.Add(VehicleID, Strategy);
	}

	LoadArchive.FlushCache();
	LoadArchive.Close();
}

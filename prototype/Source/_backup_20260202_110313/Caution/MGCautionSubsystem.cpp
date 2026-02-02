// Copyright Midnight Grind. All Rights Reserved.

#include "Caution/MGCautionSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGCautionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default settings
	Settings.bEnableCautions = true;
	Settings.bAutoDeploySafetyCar = true;
	Settings.bAutoDeployVSC = true;
	Settings.SafetyCarSpeed = 80.0f;
	Settings.VSCSpeedLimit = 60.0f;
	Settings.MinLapsUnderSC = 2;
	Settings.MaxLapsUnderSC = 5;
	Settings.bAllowLappedCarsToUnlap = true;
	Settings.bClosePitOnRedFlag = true;
	Settings.DebrisCleanupTime = 30.0f;
	Settings.AccidentResponseTime = 10.0f;
	Settings.RestartWarningDistance = 500.0f;

	CurrentState = EMGCautionState::Clear;
	bPitLaneOpen = true;
	CautionCounter = 0;

	// Initialize default zones
	ConfigureZones(10, 5000.0f);

	// Start caution tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CautionTickHandle,
			this,
			&UMGCautionSubsystem::OnCautionTick,
			0.1f,
			true
		);
	}
}

void UMGCautionSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CautionTickHandle);
	}

	Super::Deinitialize();
}

bool UMGCautionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGCautionSubsystem::OnCautionTick()
{
	if (!Settings.bEnableCautions) return;

	float DeltaTime = 0.1f;

	if (SafetyCarState.bDeployed)
	{
		UpdateSafetyCar(DeltaTime);
	}

	if (VSCState.bActive)
	{
		UpdateVSC(DeltaTime);
	}

	if (CurrentState == EMGCautionState::RestartPending ||
		CurrentState == EMGCautionState::GreenFlagPending)
	{
		UpdateRestartProcedure(DeltaTime);
	}

	// Update caution duration
	if (IsCautionActive())
	{
		CurrentCaution.Duration += DeltaTime;
	}
}

void UMGCautionSubsystem::UpdateSafetyCar(float DeltaTime)
{
	switch (SafetyCarState.Phase)
	{
		case EMGSafetyCarPhase::Deploying:
			// Safety car is getting on track
			// Would transition to PickingUpLeader when in position
			break;

		case EMGSafetyCarPhase::PickingUpLeader:
			// Waiting for leader to catch up
			if (SafetyCarState.GapToLeader < 50.0f)
			{
				SetSafetyCarPhase(EMGSafetyCarPhase::Leading);
			}
			break;

		case EMGSafetyCarPhase::Leading:
			// Leading the field
			SafetyCarState.LapsLed++;
			break;

		case EMGSafetyCarPhase::InLap:
			// Safety car is leaving
			break;

		case EMGSafetyCarPhase::PitEntry:
			// Safety car entering pits
			SafetyCarState.bDeployed = false;
			SafetyCarState.Phase = EMGSafetyCarPhase::NotDeployed;
			OnSafetyCarIn.Broadcast();
			SetCautionState(EMGCautionState::RestartPending);
			break;

		default:
			break;
	}
}

void UMGCautionSubsystem::UpdateVSC(float DeltaTime)
{
	// Check all vehicles for VSC compliance
	for (auto& Pair : VSCState.VehicleDeltas)
	{
		FName VehicleID = Pair.Key;
		float Delta = Pair.Value;

		bool bCompliant = Delta >= VSCState.MinDelta && Delta <= VSCState.MaxDelta;
		VSCState.VehicleCompliance.Add(VehicleID, bCompliant);

		if (!bCompliant)
		{
			OnVSCDeltaViolation.Broadcast(VehicleID, Delta);
		}
	}
}

void UMGCautionSubsystem::UpdateRestartProcedure(float DeltaTime)
{
	if (RestartProcedure.WarningLapsRemaining > 0)
	{
		// Would decrement on lap completion
	}

	if (RestartProcedure.bGreenFlagReady && CurrentState == EMGCautionState::GreenFlagPending)
	{
		// Waiting for leader to initiate restart
	}
}

void UMGCautionSubsystem::SetCautionState(EMGCautionState NewState)
{
	if (CurrentState != NewState)
	{
		EMGCautionState OldState = CurrentState;
		CurrentState = NewState;
		OnCautionStateChanged.Broadcast(OldState, NewState);
	}
}

void UMGCautionSubsystem::DeployCaution(EMGCautionType Type, EMGCautionReason Reason, const FVector& IncidentLocation)
{
	if (!Settings.bEnableCautions) return;

	// Don't deploy if already under more severe caution
	if (IsCautionActive() && CurrentCaution.Type >= Type)
	{
		return;
	}

	CautionCounter++;

	CurrentCaution = FMGCautionPeriod();
	CurrentCaution.CautionNumber = CautionCounter;
	CurrentCaution.Type = Type;
	CurrentCaution.Reason = Reason;
	CurrentCaution.State = EMGCautionState::CautionDeployed;
	CurrentCaution.IncidentLocation = IncidentLocation;
	CurrentCaution.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	SetCautionState(EMGCautionState::CautionDeployed);

	// Show yellow flags to all vehicles
	for (auto& Pair : VehicleFlags)
	{
		ShowFlag(Pair.Key, Type == EMGCautionType::FullCourseYellow ?
			EMGFlagType::DoubleYellow : EMGFlagType::Yellow);
	}

	OnCautionDeployed.Broadcast(Type, Reason);

	// Auto-deploy safety car for severe incidents
	if (Type == EMGCautionType::SafetyCar && Settings.bAutoDeploySafetyCar)
	{
		DeploySafetyCar(Reason);
	}
	else if (Type == EMGCautionType::VirtualSafetyCar && Settings.bAutoDeployVSC)
	{
		DeployVirtualSafetyCar();
	}
}

void UMGCautionSubsystem::EndCaution()
{
	if (!IsCautionActive()) return;

	// Record the caution period
	RecordCautionPeriod();

	EMGCautionType EndedType = CurrentCaution.Type;

	// Clear caution state
	CurrentCaution = FMGCautionPeriod();
	SetCautionState(EMGCautionState::Clear);

	// Clear safety car if deployed
	if (SafetyCarState.bDeployed)
	{
		SafetyCarState.bDeployed = false;
		SafetyCarState.Phase = EMGSafetyCarPhase::NotDeployed;
	}

	// End VSC if active
	if (VSCState.bActive)
	{
		VSCState.bActive = false;
	}

	// Clear all flags
	for (auto& Pair : VehicleFlags)
	{
		Pair.Value = EMGFlagType::Green;
	}

	// Reopen pit lane
	bPitLaneOpen = true;

	OnCautionEnded.Broadcast(EndedType);
	OnGreenFlag.Broadcast();
}

void UMGCautionSubsystem::EscalateCaution(EMGCautionType NewType)
{
	if (!IsCautionActive()) return;
	if (NewType <= CurrentCaution.Type) return;

	CurrentCaution.Type = NewType;

	if (NewType == EMGCautionType::SafetyCar)
	{
		DeploySafetyCar(CurrentCaution.Reason);
	}
	else if (NewType == EMGCautionType::RedFlag)
	{
		DeployRedFlag(CurrentCaution.Reason);
	}
}

void UMGCautionSubsystem::DeployLocalYellow(int32 ZoneIndex, EMGCautionReason Reason, const FVector& Location)
{
	if (!CautionZones.IsValidIndex(ZoneIndex)) return;

	CautionZones[ZoneIndex].ActiveFlag = EMGFlagType::Yellow;
	CautionZones[ZoneIndex].Reason = Reason;
	CautionZones[ZoneIndex].IncidentLocation = Location;
	CautionZones[ZoneIndex].bNoOvertaking = true;
	CautionZones[ZoneIndex].ActivatedTime = FDateTime::Now();
}

void UMGCautionSubsystem::ClearLocalYellow(int32 ZoneIndex)
{
	if (!CautionZones.IsValidIndex(ZoneIndex)) return;

	CautionZones[ZoneIndex].ActiveFlag = EMGFlagType::None;
	CautionZones[ZoneIndex].Reason = EMGCautionReason::None;
	CautionZones[ZoneIndex].bNoOvertaking = false;
}

void UMGCautionSubsystem::ClearAllLocalYellows()
{
	for (int32 i = 0; i < CautionZones.Num(); i++)
	{
		ClearLocalYellow(i);
	}
}

bool UMGCautionSubsystem::IsCautionActive() const
{
	return CurrentCaution.Type != EMGCautionType::None;
}

EMGCautionType UMGCautionSubsystem::GetActiveCautionType() const
{
	return CurrentCaution.Type;
}

void UMGCautionSubsystem::DeploySafetyCar(EMGCautionReason Reason)
{
	SafetyCarState.bDeployed = true;
	SafetyCarState.Phase = EMGSafetyCarPhase::Deploying;
	SafetyCarState.TargetSpeed = Settings.SafetyCarSpeed;
	SafetyCarState.LapsLed = 0;
	SafetyCarState.bLightsOn = true;
	SafetyCarState.bReadyToWithdraw = false;

	if (CurrentCaution.Type != EMGCautionType::SafetyCar)
	{
		DeployCaution(EMGCautionType::SafetyCar, Reason, SafetyCarState.CurrentPosition);
	}

	OnSafetyCarDeployed.Broadcast(SafetyCarState);
}

void UMGCautionSubsystem::BringSafetyCarIn()
{
	if (!SafetyCarState.bDeployed) return;

	SafetyCarState.Phase = EMGSafetyCarPhase::InLap;
	SafetyCarState.bLightsOn = false;
	SafetyCarState.bReadyToWithdraw = true;

	// Prepare restart
	PrepareRestart();
}

void UMGCautionSubsystem::UpdateSafetyCarPosition(const FVector& Position, float Speed)
{
	SafetyCarState.CurrentPosition = Position;
	SafetyCarState.CurrentSpeed = Speed;
}

void UMGCautionSubsystem::SetSafetyCarPhase(EMGSafetyCarPhase Phase)
{
	SafetyCarState.Phase = Phase;
}

void UMGCautionSubsystem::DeployVirtualSafetyCar()
{
	VSCState.bActive = true;
	VSCState.TargetDelta = 0.0f;
	VSCState.SpeedLimit = Settings.VSCSpeedLimit;
	VSCState.MinDelta = -0.5f;
	VSCState.MaxDelta = 1.0f;
	VSCState.bEndingSoon = false;

	if (CurrentCaution.Type != EMGCautionType::VirtualSafetyCar)
	{
		DeployCaution(EMGCautionType::VirtualSafetyCar, CurrentCaution.Reason, FVector::ZeroVector);
	}
}

void UMGCautionSubsystem::EndVirtualSafetyCar()
{
	VSCState.bEndingSoon = true;

	// Would give drivers warning before ending
	// After warning period, fully end
	VSCState.bActive = false;
	VSCState.VehicleDeltas.Empty();
	VSCState.VehicleCompliance.Empty();
}

void UMGCautionSubsystem::UpdateVSCDelta(FName VehicleID, float Delta)
{
	VSCState.VehicleDeltas.Add(VehicleID, Delta);
}

float UMGCautionSubsystem::GetVSCDelta(FName VehicleID) const
{
	if (VSCState.VehicleDeltas.Contains(VehicleID))
	{
		return VSCState.VehicleDeltas[VehicleID];
	}
	return 0.0f;
}

bool UMGCautionSubsystem::IsVehicleVSCCompliant(FName VehicleID) const
{
	if (VSCState.VehicleCompliance.Contains(VehicleID))
	{
		return VSCState.VehicleCompliance[VehicleID];
	}
	return true;
}

void UMGCautionSubsystem::DeployRedFlag(EMGCautionReason Reason)
{
	CurrentCaution.Type = EMGCautionType::RedFlag;
	CurrentCaution.Reason = Reason;

	// Close pit lane on red flag
	if (Settings.bClosePitOnRedFlag)
	{
		bPitLaneOpen = false;
	}

	// Show red flags to all
	for (auto& Pair : VehicleFlags)
	{
		ShowFlag(Pair.Key, EMGFlagType::Red);
	}

	CautionStats.RedFlags++;

	OnRedFlag.Broadcast();
}

void UMGCautionSubsystem::RestartFromRedFlag()
{
	bPitLaneOpen = true;
	PrepareRestart();
}

bool UMGCautionSubsystem::IsRedFlagActive() const
{
	return CurrentCaution.Type == EMGCautionType::RedFlag;
}

void UMGCautionSubsystem::ShowFlag(FName VehicleID, EMGFlagType Flag)
{
	VehicleFlags.Add(VehicleID, Flag);
	OnFlagDisplayed.Broadcast(VehicleID, Flag);
}

void UMGCautionSubsystem::ClearFlag(FName VehicleID)
{
	VehicleFlags.Add(VehicleID, EMGFlagType::None);
}

EMGFlagType UMGCautionSubsystem::GetVehicleFlag(FName VehicleID) const
{
	if (VehicleFlags.Contains(VehicleID))
	{
		return VehicleFlags[VehicleID];
	}
	return EMGFlagType::None;
}

TArray<FName> UMGCautionSubsystem::GetVehiclesWithFlag(EMGFlagType Flag) const
{
	TArray<FName> Vehicles;
	for (const auto& Pair : VehicleFlags)
	{
		if (Pair.Value == Flag)
		{
			Vehicles.Add(Pair.Key);
		}
	}
	return Vehicles;
}

void UMGCautionSubsystem::ConfigureZones(int32 NumZones, float InTrackLength)
{
	TrackLength = InTrackLength;
	CautionZones.Empty();

	if (NumZones <= 0)
	{
		return;
	}

	float ZoneLength = TrackLength / NumZones;

	for (int32 i = 0; i < NumZones; i++)
	{
		FMGCautionZone Zone;
		Zone.ZoneIndex = i;
		Zone.StartDistance = i * ZoneLength;
		Zone.EndDistance = (i + 1) * ZoneLength;
		Zone.ActiveFlag = EMGFlagType::None;

		CautionZones.Add(Zone);
	}
}

FMGCautionZone UMGCautionSubsystem::GetZoneAtDistance(float Distance) const
{
	int32 Index = GetZoneIndex(Distance);
	if (CautionZones.IsValidIndex(Index))
	{
		return CautionZones[Index];
	}
	return FMGCautionZone();
}

bool UMGCautionSubsystem::IsZoneUnderCaution(int32 ZoneIndex) const
{
	if (CautionZones.IsValidIndex(ZoneIndex))
	{
		return CautionZones[ZoneIndex].ActiveFlag != EMGFlagType::None;
	}
	return false;
}

float UMGCautionSubsystem::GetZoneSpeedLimit(int32 ZoneIndex) const
{
	if (CautionZones.IsValidIndex(ZoneIndex))
	{
		return CautionZones[ZoneIndex].SpeedLimit;
	}
	return 0.0f;
}

int32 UMGCautionSubsystem::GetZoneIndex(float Distance) const
{
	if (TrackLength <= 0 || CautionZones.Num() == 0) return 0;

	// Normalize distance
	while (Distance < 0) Distance += TrackLength;
	while (Distance >= TrackLength) Distance -= TrackLength;

	float ZoneLength = TrackLength / CautionZones.Num();
	return FMath::FloorToInt(Distance / ZoneLength);
}

void UMGCautionSubsystem::PrepareRestart()
{
	SetCautionState(EMGCautionState::RestartPending);

	RestartProcedure = FMGRestartProcedure();
	RestartProcedure.bRollingStart = true;
	RestartProcedure.bDoubleFileRestart = Settings.bUseDoubleFileRestarts;
	RestartProcedure.MinRestartSpeed = 60.0f;
	RestartProcedure.MaxRestartSpeed = 100.0f;
	RestartProcedure.WarningLapsRemaining = 1;

	OnRestartWarning.Broadcast(1, false);
}

void UMGCautionSubsystem::InitiateRestart()
{
	SetCautionState(EMGCautionState::GreenFlagPending);
	RestartProcedure.bGreenFlagReady = true;

	OnRestartWarning.Broadcast(0, true);
}

void UMGCautionSubsystem::SetRestartLeader(FName VehicleID)
{
	RestartProcedure.RestartLeader = VehicleID;
}

bool UMGCautionSubsystem::IsRestartPending() const
{
	return CurrentState == EMGCautionState::RestartPending ||
		   CurrentState == EMGCautionState::GreenFlagPending;
}

void UMGCautionSubsystem::AbortRestart()
{
	SetCautionState(EMGCautionState::CautionDeployed);
	RestartProcedure = FMGRestartProcedure();
}

void UMGCautionSubsystem::SetPitLaneOpen(bool bOpen)
{
	bPitLaneOpen = bOpen;
}

void UMGCautionSubsystem::AllowLappedCarsToUnlap()
{
	if (!Settings.bAllowLappedCarsToUnlap) return;

	// Lapped cars would pass safety car to unlap themselves
	LappedCars.Empty();
}

void UMGCautionSubsystem::ResetStats()
{
	CautionStats = FMGCautionStats();
}

void UMGCautionSubsystem::SetCautionSettings(const FMGCautionSettings& NewSettings)
{
	Settings = NewSettings;
}

void UMGCautionSubsystem::ReportIncident(FName VehicleID, const FVector& Location, EMGCautionReason Reason)
{
	if (!Settings.bEnableCautions) return;

	// Add to involved vehicles
	if (!CurrentCaution.InvolvedVehicles.Contains(VehicleID))
	{
		CurrentCaution.InvolvedVehicles.Add(VehicleID);
	}

	// Determine caution level based on severity
	switch (Reason)
	{
		case EMGCautionReason::Accident:
			DeployCaution(EMGCautionType::SafetyCar, Reason, Location);
			break;
		case EMGCautionReason::VehicleStopped:
			DeployCaution(EMGCautionType::VirtualSafetyCar, Reason, Location);
			break;
		case EMGCautionReason::Debris:
			DeployLocalYellow(GetZoneIndex(Location.X), Reason, Location);
			break;
		default:
			DeployLocalYellow(GetZoneIndex(Location.X), Reason, Location);
			break;
	}
}

void UMGCautionSubsystem::ReportDebris(const FVector& Location)
{
	int32 Zone = GetZoneIndex(Location.X);
	DeployLocalYellow(Zone, EMGCautionReason::Debris, Location);
}

void UMGCautionSubsystem::ReportStoppedVehicle(FName VehicleID, const FVector& Location)
{
	ReportIncident(VehicleID, Location, EMGCautionReason::VehicleStopped);
}

void UMGCautionSubsystem::RecordCautionPeriod()
{
	CurrentCaution.EndTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CurrentCaution.Duration = CurrentCaution.EndTime - CurrentCaution.StartTime;

	CautionStats.TotalCautions++;
	CautionStats.TotalTimeUnderCaution += CurrentCaution.Duration;
	CautionStats.TotalLapsUnderCaution += CurrentCaution.LapsUnderCaution;

	if (CurrentCaution.Type == EMGCautionType::SafetyCar)
	{
		CautionStats.SafetyCarPeriods++;
	}
	else if (CurrentCaution.Type == EMGCautionType::VirtualSafetyCar)
	{
		CautionStats.VSCPeriods++;
	}

	CautionStats.CautionHistory.Add(CurrentCaution);
}

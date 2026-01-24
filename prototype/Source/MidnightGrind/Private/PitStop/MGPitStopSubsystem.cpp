// Copyright Midnight Grind. All Rights Reserved.

#include "PitStop/MGPitStopSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGPitStopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default pit lane config
	PitLaneConfig.SpeedLimit = 60.0f;
	PitLaneConfig.LaneLength = 300.0f;
	PitLaneConfig.bPitLaneOpen = true;
	PitLaneConfig.bHasSpeedLimitEnforcement = true;
	PitLaneConfig.SpeedingPenaltyTime = 5.0f;
	PitLaneConfig.bHasPitLimiter = true;
	PitLaneConfig.bHasTrafficLight = true;

	// Initialize default pit boxes
	for (int32 i = 0; i < 20; i++)
	{
		FMGPitBoxConfig BoxConfig;
		BoxConfig.BoxNumber = i;
		BoxConfig.BoxLocation = FVector(i * 12.0f, 0.0f, 0.0f);
		BoxConfig.StopPosition = BoxConfig.BoxLocation + FVector(0.0f, 3.0f, 0.0f);
		BoxConfig.FuelCapacity = 100.0f;
		BoxConfig.CurrentFuel = 100.0f;
		BoxConfig.EquipmentQuality = 1.0f;

		// Initialize tire inventory
		BoxConfig.TireInventory.Add(EMGTireCompound::UltraSoft, 4);
		BoxConfig.TireInventory.Add(EMGTireCompound::Soft, 4);
		BoxConfig.TireInventory.Add(EMGTireCompound::Medium, 4);
		BoxConfig.TireInventory.Add(EMGTireCompound::Hard, 4);
		BoxConfig.TireInventory.Add(EMGTireCompound::Intermediate, 4);
		BoxConfig.TireInventory.Add(EMGTireCompound::FullWet, 4);

		// Initialize crew
		FMGPitCrewMember JackOp;
		JackOp.Role = EMGPitCrewRole::JackOperator;
		JackOp.SkillLevel = 1.0f;
		JackOp.BaseServiceTime = 1.0f;
		BoxConfig.Crew.Add(JackOp);

		FMGPitCrewMember FuelMan;
		FuelMan.Role = EMGPitCrewRole::FuelMan;
		FuelMan.SkillLevel = 1.0f;
		FuelMan.BaseServiceTime = 0.5f; // Per 10 liters
		BoxConfig.Crew.Add(FuelMan);

		// Tire changers
		for (int32 t = 0; t < 4; t++)
		{
			FMGPitCrewMember TireChanger;
			TireChanger.Role = static_cast<EMGPitCrewRole>(static_cast<uint8>(EMGPitCrewRole::TireChangerFL) + t);
			TireChanger.SkillLevel = 1.0f;
			TireChanger.BaseServiceTime = 2.5f;
			BoxConfig.Crew.Add(TireChanger);
		}

		FMGPitCrewMember BodyRepair;
		BodyRepair.Role = EMGPitCrewRole::BodyRepair;
		BodyRepair.SkillLevel = 1.0f;
		BodyRepair.BaseServiceTime = 5.0f;
		BoxConfig.Crew.Add(BodyRepair);

		FMGPitCrewMember Lollipop;
		Lollipop.Role = EMGPitCrewRole::LollipopMan;
		Lollipop.SkillLevel = 1.0f;
		BoxConfig.Crew.Add(Lollipop);

		PitLaneConfig.PitBoxes.Add(BoxConfig);
	}

	FastestPitStopTime = 0.0f;

	LoadPitStopData();

	// Start pit stop tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PitStopTickHandle,
			this,
			&UMGPitStopSubsystem::OnPitStopTick,
			0.05f,
			true
		);
	}
}

void UMGPitStopSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PitStopTickHandle);
	}

	SavePitStopData();

	Super::Deinitialize();
}

bool UMGPitStopSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGPitStopSubsystem::OnPitStopTick()
{
	UpdateActivePitStops(0.05f);
}

void UMGPitStopSubsystem::UpdateActivePitStops(float DeltaTime)
{
	TArray<FName> CompletedStops;

	for (auto& Pair : ActivePitStops)
	{
		FMGActivePitStop& PitStop = Pair.Value;

		if (PitStop.CurrentState == EMGPitStopState::Servicing)
		{
			PitStop.ElapsedTime += DeltaTime;
			ProcessService(PitStop, DeltaTime);

			// Check if all services completed
			if (PitStop.PendingServices.Num() == 0)
			{
				// Ready for release
				PitStop.bGreenLightGiven = true;
			}
		}
		else if (PitStop.CurrentState == EMGPitStopState::InPitLane ||
				 PitStop.CurrentState == EMGPitStopState::Approaching)
		{
			PitStop.ElapsedTime += DeltaTime;
		}
	}

	for (const FName& VehicleID : CompletedStops)
	{
		ActivePitStops.Remove(VehicleID);
	}
}

void UMGPitStopSubsystem::ProcessService(FMGActivePitStop& PitStop, float DeltaTime)
{
	if (PitStop.PendingServices.Num() == 0)
	{
		return;
	}

	// Get current service
	EMGPitStopService CurrentService = PitStop.PendingServices[0];

	// Calculate service time
	float ServiceTime = CalculateServiceTime(CurrentService, PitStop.AssignedBox);

	// Update progress
	PitStop.CurrentServiceProgress += DeltaTime / ServiceTime;

	if (PitStop.CurrentServiceProgress >= 1.0f)
	{
		// Service completed
		PitStop.CompletedServices.Add(CurrentService);
		PitStop.PendingServices.RemoveAt(0);
		PitStop.CurrentServiceProgress = 0.0f;

		OnPitServiceCompleted.Broadcast(PitStop.VehicleID, CurrentService);

		// Start next service if available
		if (PitStop.PendingServices.Num() > 0)
		{
			EMGPitStopService NextService = PitStop.PendingServices[0];
			float EstTime = CalculateServiceTime(NextService, PitStop.AssignedBox);
			OnPitServiceStarted.Broadcast(PitStop.VehicleID, NextService, EstTime);
		}
	}

	// Update estimated time remaining
	float TimeRemaining = 0.0f;
	for (int32 i = 0; i < PitStop.PendingServices.Num(); i++)
	{
		float SvcTime = CalculateServiceTime(PitStop.PendingServices[i], PitStop.AssignedBox);
		if (i == 0)
		{
			SvcTime *= (1.0f - PitStop.CurrentServiceProgress);
		}
		TimeRemaining += SvcTime;
	}
	PitStop.EstimatedTimeRemaining = TimeRemaining;
}

float UMGPitStopSubsystem::CalculateServiceTime(EMGPitStopService Service, int32 BoxIndex) const
{
	if (!PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		return 5.0f; // Default
	}

	const FMGPitBoxConfig& Box = PitLaneConfig.PitBoxes[BoxIndex];
	float BaseTime = 2.0f;
	float QualityMod = 2.0f - Box.EquipmentQuality;

	switch (Service)
	{
		case EMGPitStopService::Refuel:
			BaseTime = 5.0f; // Depends on fuel amount
			break;
		case EMGPitStopService::TireChange:
			BaseTime = 2.5f;
			break;
		case EMGPitStopService::RepairDamage:
			BaseTime = 8.0f;
			break;
		case EMGPitStopService::AdjustSetup:
			BaseTime = 3.0f;
			break;
		case EMGPitStopService::DriverChange:
			BaseTime = 10.0f;
			break;
		case EMGPitStopService::PenaltyServe:
			BaseTime = 5.0f;
			break;
		case EMGPitStopService::QuickService:
			BaseTime = 4.0f;
			break;
		case EMGPitStopService::FullService:
			BaseTime = 15.0f;
			break;
		default:
			break;
	}

	// Apply crew skill modifier
	float AvgSkill = 1.0f;
	if (Box.Crew.Num() > 0)
	{
		float TotalSkill = 0.0f;
		for (const FMGPitCrewMember& Crew : Box.Crew)
		{
			TotalSkill += Crew.SkillLevel;
		}
		AvgSkill = TotalSkill / Box.Crew.Num();
	}

	return BaseTime * QualityMod / AvgSkill;
}

bool UMGPitStopSubsystem::CheckForCrewError(int32 BoxIndex, EMGPitCrewRole Role) const
{
	if (!PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		return false;
	}

	const FMGPitBoxConfig& Box = PitLaneConfig.PitBoxes[BoxIndex];

	for (const FMGPitCrewMember& Crew : Box.Crew)
	{
		if (Crew.Role == Role)
		{
			float ErrorRoll = FMath::FRand();
			float AdjustedError = Crew.ErrorChance * (1.0f + Crew.Fatigue);
			return ErrorRoll < AdjustedError;
		}
	}

	return false;
}

void UMGPitStopSubsystem::ApplyPenalty(FName VehicleID, EMGPitLaneViolation Violation)
{
	if (!ActivePitStops.Contains(VehicleID))
	{
		return;
	}

	FMGActivePitStop& PitStop = ActivePitStops[VehicleID];

	float PenaltyTime = 0.0f;
	switch (Violation)
	{
		case EMGPitLaneViolation::Speeding:
			PenaltyTime = PitLaneConfig.SpeedingPenaltyTime;
			break;
		case EMGPitLaneViolation::UnsafeRelease:
			PenaltyTime = 10.0f;
			break;
		case EMGPitLaneViolation::CrossingLine:
			PenaltyTime = 5.0f;
			break;
		case EMGPitLaneViolation::EquipmentContact:
			PenaltyTime = 3.0f;
			break;
		case EMGPitLaneViolation::WrongBox:
			PenaltyTime = 5.0f;
			break;
		case EMGPitLaneViolation::IgnoringRedLight:
			PenaltyTime = 10.0f;
			break;
		default:
			break;
	}

	// Add penalty service
	if (PenaltyTime > 0.0f)
	{
		PitStop.PendingServices.Insert(EMGPitStopService::PenaltyServe, 0);
	}

	OnPitLaneViolation.Broadcast(VehicleID, Violation);

	// Update stats
	if (VehicleStats.Contains(VehicleID))
	{
		VehicleStats[VehicleID].TotalTimeLostToPenalties += PenaltyTime;
		if (Violation == EMGPitLaneViolation::Speeding)
		{
			VehicleStats[VehicleID].SpeedingViolations++;
		}
	}
}

FMGPitStopResult UMGPitStopSubsystem::CompletePitStop(const FMGActivePitStop& PitStop)
{
	FMGPitStopResult Result;
	Result.VehicleID = PitStop.VehicleID;
	Result.TotalTime = PitStop.ElapsedTime;
	Result.StationaryTime = PitStop.ElapsedTime; // Simplified
	Result.CompletedServices = PitStop.CompletedServices;
	Result.Timestamp = FDateTime::Now();

	// Count tires changed
	for (EMGPitStopService Service : PitStop.CompletedServices)
	{
		if (Service == EMGPitStopService::TireChange)
		{
			int32 TireCount = 0;
			if (PitStop.Request.bChangeFrontTires) TireCount += 2;
			if (PitStop.Request.bChangeRearTires) TireCount += 2;
			Result.TiresChanged = TireCount;
		}
		else if (Service == EMGPitStopService::Refuel)
		{
			Result.FuelAdded = PitStop.Request.FuelAmount;
		}
		else if (Service == EMGPitStopService::RepairDamage)
		{
			Result.DamageRepaired = 100.0f; // Full repair
		}
	}

	// Update fastest pit stop
	if (FastestPitStopTime == 0.0f || Result.StationaryTime < FastestPitStopTime)
	{
		FastestPitStopTime = Result.StationaryTime;
		FastestPitStopVehicle = PitStop.VehicleID;
	}

	// Update vehicle stats
	if (!VehicleStats.Contains(PitStop.VehicleID))
	{
		FMGPitStopStats NewStats;
		NewStats.VehicleID = PitStop.VehicleID;
		VehicleStats.Add(PitStop.VehicleID, NewStats);
	}

	FMGPitStopStats& Stats = VehicleStats[PitStop.VehicleID];
	Stats.TotalPitStops++;
	if (Stats.FastestPitStop == 0.0f || Result.StationaryTime < Stats.FastestPitStop)
	{
		Stats.FastestPitStop = Result.StationaryTime;
	}
	Stats.AveragePitStop = ((Stats.AveragePitStop * (Stats.TotalPitStops - 1)) + Result.StationaryTime) / Stats.TotalPitStops;
	Stats.PitStopHistory.Add(Result);

	RacePitStopHistory.Add(Result);

	return Result;
}

bool UMGPitStopSubsystem::RequestPitStop(FName VehicleID, const FMGPitStopRequest& Request)
{
	if (!PitLaneConfig.bPitLaneOpen)
	{
		return false;
	}

	PendingRequests.Add(VehicleID, Request);
	OnPitStopRequested.Broadcast(VehicleID, Request);

	return true;
}

void UMGPitStopSubsystem::CancelPitStopRequest(FName VehicleID)
{
	PendingRequests.Remove(VehicleID);
}

void UMGPitStopSubsystem::ModifyPitStopRequest(FName VehicleID, const FMGPitStopRequest& NewRequest)
{
	if (PendingRequests.Contains(VehicleID))
	{
		PendingRequests[VehicleID] = NewRequest;
	}
	else if (ActivePitStops.Contains(VehicleID))
	{
		// Can only modify if not yet servicing
		FMGActivePitStop& PitStop = ActivePitStops[VehicleID];
		if (PitStop.CurrentState != EMGPitStopState::Servicing)
		{
			PitStop.Request = NewRequest;
			PitStop.PendingServices = NewRequest.RequestedServices;
		}
	}
}

bool UMGPitStopSubsystem::HasPendingPitStop(FName VehicleID) const
{
	return PendingRequests.Contains(VehicleID) || ActivePitStops.Contains(VehicleID);
}

FMGPitStopRequest UMGPitStopSubsystem::GetPendingRequest(FName VehicleID) const
{
	if (PendingRequests.Contains(VehicleID))
	{
		return PendingRequests[VehicleID];
	}
	if (ActivePitStops.Contains(VehicleID))
	{
		return ActivePitStops[VehicleID].Request;
	}
	return FMGPitStopRequest();
}

float UMGPitStopSubsystem::EstimatePitStopTime(const FMGPitStopRequest& Request) const
{
	float TotalTime = 0.0f;
	int32 BoxIndex = GetAvailablePitBox();

	// Jack raise time
	TotalTime += 1.0f;

	for (EMGPitStopService Service : Request.RequestedServices)
	{
		TotalTime += CalculateServiceTime(Service, BoxIndex);
	}

	// Jack lower and release
	TotalTime += 1.0f;

	return TotalTime;
}

void UMGPitStopSubsystem::EnterPitLane(FName VehicleID)
{
	if (!PendingRequests.Contains(VehicleID))
	{
		return;
	}

	FMGActivePitStop NewPitStop;
	NewPitStop.VehicleID = VehicleID;
	NewPitStop.Request = PendingRequests[VehicleID];
	NewPitStop.CurrentState = EMGPitStopState::InPitLane;
	NewPitStop.StateStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	NewPitStop.PendingServices = NewPitStop.Request.RequestedServices;
	NewPitStop.AssignedBox = GetAvailablePitBox();

	ActivePitStops.Add(VehicleID, NewPitStop);
	PendingRequests.Remove(VehicleID);

	if (NewPitStop.AssignedBox >= 0 && PitLaneConfig.PitBoxes.IsValidIndex(NewPitStop.AssignedBox))
	{
		PitLaneConfig.PitBoxes[NewPitStop.AssignedBox].bIsOccupied = true;
		PitLaneConfig.PitBoxes[NewPitStop.AssignedBox].AssignedVehicle = VehicleID;
	}

	OnPitStopStateChanged.Broadcast(VehicleID, EMGPitStopState::InPitLane);
}

void UMGPitStopSubsystem::ArriveAtPitBox(FName VehicleID)
{
	if (!ActivePitStops.Contains(VehicleID))
	{
		return;
	}

	FMGActivePitStop& PitStop = ActivePitStops[VehicleID];
	PitStop.CurrentState = EMGPitStopState::Stopping;
	OnPitStopStateChanged.Broadcast(VehicleID, EMGPitStopState::Stopping);
}

void UMGPitStopSubsystem::BeginServicing(FName VehicleID)
{
	if (!ActivePitStops.Contains(VehicleID))
	{
		return;
	}

	FMGActivePitStop& PitStop = ActivePitStops[VehicleID];
	PitStop.CurrentState = EMGPitStopState::Servicing;
	PitStop.bJackRaised = true;
	PitStop.ElapsedTime = 0.0f;

	OnPitStopStateChanged.Broadcast(VehicleID, EMGPitStopState::Servicing);

	// Start first service
	if (PitStop.PendingServices.Num() > 0)
	{
		EMGPitStopService FirstService = PitStop.PendingServices[0];
		float EstTime = CalculateServiceTime(FirstService, PitStop.AssignedBox);
		OnPitServiceStarted.Broadcast(VehicleID, FirstService, EstTime);
	}
}

void UMGPitStopSubsystem::ReleaseFromPitBox(FName VehicleID)
{
	if (!ActivePitStops.Contains(VehicleID))
	{
		return;
	}

	FMGActivePitStop& PitStop = ActivePitStops[VehicleID];

	// Check if safe to release (no traffic)
	// Simplified - always allow
	PitStop.CurrentState = EMGPitStopState::Departing;
	PitStop.bJackRaised = false;
	PitStop.bGreenLightGiven = true;

	// Free up pit box
	if (PitStop.AssignedBox >= 0 && PitLaneConfig.PitBoxes.IsValidIndex(PitStop.AssignedBox))
	{
		PitLaneConfig.PitBoxes[PitStop.AssignedBox].bIsOccupied = false;
		PitLaneConfig.PitBoxes[PitStop.AssignedBox].AssignedVehicle = NAME_None;
	}

	OnPitStopStateChanged.Broadcast(VehicleID, EMGPitStopState::Departing);
}

void UMGPitStopSubsystem::ExitPitLane(FName VehicleID)
{
	if (!ActivePitStops.Contains(VehicleID))
	{
		return;
	}

	FMGActivePitStop& PitStop = ActivePitStops[VehicleID];
	FMGPitStopResult Result = CompletePitStop(PitStop);

	ActivePitStops.Remove(VehicleID);

	OnPitStopCompleted.Broadcast(VehicleID, Result);
}

EMGPitStopState UMGPitStopSubsystem::GetPitStopState(FName VehicleID) const
{
	if (ActivePitStops.Contains(VehicleID))
	{
		return ActivePitStops[VehicleID].CurrentState;
	}
	return EMGPitStopState::Available;
}

FMGActivePitStop UMGPitStopSubsystem::GetActivePitStop(FName VehicleID) const
{
	if (ActivePitStops.Contains(VehicleID))
	{
		return ActivePitStops[VehicleID];
	}
	return FMGActivePitStop();
}

bool UMGPitStopSubsystem::IsVehicleInPitLane(FName VehicleID) const
{
	if (!ActivePitStops.Contains(VehicleID))
	{
		return false;
	}

	EMGPitStopState State = ActivePitStops[VehicleID].CurrentState;
	return State == EMGPitStopState::InPitLane ||
		   State == EMGPitStopState::Stopping ||
		   State == EMGPitStopState::Servicing ||
		   State == EMGPitStopState::Departing;
}

bool UMGPitStopSubsystem::IsVehicleBeingServiced(FName VehicleID) const
{
	if (ActivePitStops.Contains(VehicleID))
	{
		return ActivePitStops[VehicleID].CurrentState == EMGPitStopState::Servicing;
	}
	return false;
}

void UMGPitStopSubsystem::SetPitLaneConfig(const FMGPitLaneConfig& Config)
{
	PitLaneConfig = Config;
}

void UMGPitStopSubsystem::OpenPitLane()
{
	PitLaneConfig.bPitLaneOpen = true;
	OnPitLaneStatusChanged.Broadcast(true);
}

void UMGPitStopSubsystem::ClosePitLane()
{
	PitLaneConfig.bPitLaneOpen = false;
	OnPitLaneStatusChanged.Broadcast(false);
}

void UMGPitStopSubsystem::ReportVehicleSpeed(FName VehicleID, float CurrentSpeed)
{
	if (!IsVehicleInPitLane(VehicleID))
	{
		return;
	}

	if (PitLaneConfig.bHasSpeedLimitEnforcement && CurrentSpeed > PitLaneConfig.SpeedLimit)
	{
		ApplyPenalty(VehicleID, EMGPitLaneViolation::Speeding);
	}
}

int32 UMGPitStopSubsystem::GetAvailablePitBox() const
{
	for (int32 i = 0; i < PitLaneConfig.PitBoxes.Num(); i++)
	{
		if (!PitLaneConfig.PitBoxes[i].bIsOccupied)
		{
			return i;
		}
	}
	return -1;
}

void UMGPitStopSubsystem::AssignPitBox(FName VehicleID, int32 BoxIndex)
{
	if (ActivePitStops.Contains(VehicleID) && PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		ActivePitStops[VehicleID].AssignedBox = BoxIndex;
	}
}

void UMGPitStopSubsystem::ConfigurePitBox(int32 BoxIndex, const FMGPitBoxConfig& Config)
{
	if (PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		PitLaneConfig.PitBoxes[BoxIndex] = Config;
	}
	else if (BoxIndex == PitLaneConfig.PitBoxes.Num())
	{
		PitLaneConfig.PitBoxes.Add(Config);
	}
}

FMGPitBoxConfig UMGPitStopSubsystem::GetPitBoxConfig(int32 BoxIndex) const
{
	if (PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		return PitLaneConfig.PitBoxes[BoxIndex];
	}
	return FMGPitBoxConfig();
}

void UMGPitStopSubsystem::SetCrewMemberSkill(int32 BoxIndex, EMGPitCrewRole Role, float SkillLevel)
{
	if (!PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		return;
	}

	for (FMGPitCrewMember& Crew : PitLaneConfig.PitBoxes[BoxIndex].Crew)
	{
		if (Crew.Role == Role)
		{
			Crew.SkillLevel = FMath::Clamp(SkillLevel, 0.1f, 2.0f);
			break;
		}
	}
}

void UMGPitStopSubsystem::RefillTireInventory(int32 BoxIndex, EMGTireCompound Compound, int32 Amount)
{
	if (PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		if (PitLaneConfig.PitBoxes[BoxIndex].TireInventory.Contains(Compound))
		{
			PitLaneConfig.PitBoxes[BoxIndex].TireInventory[Compound] += Amount;
		}
		else
		{
			PitLaneConfig.PitBoxes[BoxIndex].TireInventory.Add(Compound, Amount);
		}
	}
}

void UMGPitStopSubsystem::RefuelPitBox(int32 BoxIndex, float Amount)
{
	if (PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		PitLaneConfig.PitBoxes[BoxIndex].CurrentFuel = FMath::Min(
			PitLaneConfig.PitBoxes[BoxIndex].CurrentFuel + Amount,
			PitLaneConfig.PitBoxes[BoxIndex].FuelCapacity
		);
	}
}

int32 UMGPitStopSubsystem::GetTireInventory(int32 BoxIndex, EMGTireCompound Compound) const
{
	if (PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		if (PitLaneConfig.PitBoxes[BoxIndex].TireInventory.Contains(Compound))
		{
			return PitLaneConfig.PitBoxes[BoxIndex].TireInventory[Compound];
		}
	}
	return 0;
}

float UMGPitStopSubsystem::GetPitBoxFuel(int32 BoxIndex) const
{
	if (PitLaneConfig.PitBoxes.IsValidIndex(BoxIndex))
	{
		return PitLaneConfig.PitBoxes[BoxIndex].CurrentFuel;
	}
	return 0.0f;
}

void UMGPitStopSubsystem::SetPitStrategy(FName VehicleID, const FMGPitStrategy& Strategy)
{
	VehicleStrategies.Add(VehicleID, Strategy);
	OnPitStrategyUpdated.Broadcast(VehicleID, Strategy);
}

FMGPitStrategy UMGPitStopSubsystem::GetPitStrategy(FName VehicleID) const
{
	if (VehicleStrategies.Contains(VehicleID))
	{
		return VehicleStrategies[VehicleID];
	}
	return FMGPitStrategy();
}

FMGPitStrategy UMGPitStopSubsystem::CalculateOptimalStrategy(FName VehicleID, int32 RemainingLaps, float CurrentFuel, float TireWear) const
{
	FMGPitStrategy Strategy;
	Strategy.StrategyName = TEXT("Optimal");

	// Simple strategy calculation
	float FuelPerLap = 2.0f; // Estimate
	float TireWearPerLap = 0.03f; // Estimate
	float MaxTireWear = 0.8f;
	float MaxFuel = 100.0f;

	// Calculate if we need to stop
	float FuelNeeded = RemainingLaps * FuelPerLap;
	float TireWearAtEnd = TireWear + (RemainingLaps * TireWearPerLap);

	if (CurrentFuel >= FuelNeeded && TireWearAtEnd <= MaxTireWear)
	{
		// Can finish without stopping
		Strategy.PlannedStops = 0;
	}
	else
	{
		// Need at least one stop
		int32 LapsToTireLimit = FMath::FloorToInt((MaxTireWear - TireWear) / TireWearPerLap);
		int32 LapsToFuelEmpty = FMath::FloorToInt(CurrentFuel / FuelPerLap);
		int32 LapsToStop = FMath::Min(LapsToTireLimit, LapsToFuelEmpty);

		Strategy.PlannedStops = 1;
		Strategy.PlannedStopLaps.Add(RemainingLaps - LapsToStop);
		Strategy.PlannedCompounds.Add(EMGTireCompound::Medium);
		Strategy.PlannedFuelLoads.Add(FuelNeeded - (CurrentFuel - (LapsToStop * FuelPerLap)));
	}

	Strategy.TireWearThreshold = 0.2f;
	Strategy.MinLapsOnTire = 10;
	Strategy.FuelReserveTarget = 2.0f;

	return Strategy;
}

int32 UMGPitStopSubsystem::GetRecommendedPitLap(FName VehicleID) const
{
	if (VehicleStrategies.Contains(VehicleID))
	{
		const FMGPitStrategy& Strategy = VehicleStrategies[VehicleID];
		if (Strategy.PlannedStopLaps.Num() > 0)
		{
			return Strategy.PlannedStopLaps[0];
		}
	}
	return -1;
}

EMGTireCompound UMGPitStopSubsystem::GetRecommendedCompound(FName VehicleID) const
{
	if (VehicleStrategies.Contains(VehicleID))
	{
		const FMGPitStrategy& Strategy = VehicleStrategies[VehicleID];
		if (Strategy.PlannedCompounds.Num() > 0)
		{
			return Strategy.PlannedCompounds[0];
		}
	}
	return EMGTireCompound::Medium;
}

void UMGPitStopSubsystem::UpdateStrategyForWeather(FName VehicleID, bool bRaining)
{
	if (!VehicleStrategies.Contains(VehicleID))
	{
		return;
	}

	FMGPitStrategy& Strategy = VehicleStrategies[VehicleID];

	if (bRaining)
	{
		// Switch to wet tires
		for (int32 i = 0; i < Strategy.PlannedCompounds.Num(); i++)
		{
			Strategy.PlannedCompounds[i] = EMGTireCompound::Intermediate;
		}
	}

	OnPitStrategyUpdated.Broadcast(VehicleID, Strategy);
}

void UMGPitStopSubsystem::UpdateStrategyForPosition(FName VehicleID, int32 CurrentPosition, int32 GapToAhead, int32 GapToBehind)
{
	if (!VehicleStrategies.Contains(VehicleID))
	{
		return;
	}

	FMGPitStrategy& Strategy = VehicleStrategies[VehicleID];

	// Simple strategy adjustments based on position
	if (GapToAhead > 0 && GapToAhead < 3000) // Within 3 seconds
	{
		Strategy.bUndercut = true;
	}
	else if (GapToBehind > 0 && GapToBehind < 3000)
	{
		Strategy.bOvercut = true;
	}

	OnPitStrategyUpdated.Broadcast(VehicleID, Strategy);
}

void UMGPitStopSubsystem::RequestQuickFuel(FName VehicleID, float Amount)
{
	FMGPitStopRequest Request;
	Request.VehicleID = VehicleID;
	Request.RequestedServices.Add(EMGPitStopService::Refuel);
	Request.FuelAmount = Amount;
	Request.bChangeFrontTires = false;
	Request.bChangeRearTires = false;

	RequestPitStop(VehicleID, Request);
}

void UMGPitStopSubsystem::RequestQuickTires(FName VehicleID, EMGTireCompound Compound)
{
	FMGPitStopRequest Request;
	Request.VehicleID = VehicleID;
	Request.RequestedServices.Add(EMGPitStopService::TireChange);
	Request.NewTireCompound = Compound;
	Request.bChangeFrontTires = true;
	Request.bChangeRearTires = true;
	Request.FuelAmount = 0.0f;

	RequestPitStop(VehicleID, Request);
}

void UMGPitStopSubsystem::RequestFullService(FName VehicleID)
{
	FMGPitStopRequest Request;
	Request.VehicleID = VehicleID;
	Request.RequestedServices.Add(EMGPitStopService::FullService);
	Request.NewTireCompound = EMGTireCompound::Medium;
	Request.bChangeFrontTires = true;
	Request.bChangeRearTires = true;
	Request.FuelAmount = 100.0f;
	Request.bRepairBodywork = true;

	RequestPitStop(VehicleID, Request);
}

void UMGPitStopSubsystem::RequestMinimalService(FName VehicleID)
{
	FMGPitStopRequest Request;
	Request.VehicleID = VehicleID;
	Request.RequestedServices.Add(EMGPitStopService::QuickService);
	Request.bChangeFrontTires = false;
	Request.bChangeRearTires = false;
	Request.FuelAmount = 20.0f;

	RequestPitStop(VehicleID, Request);
}

FMGPitStopStats UMGPitStopSubsystem::GetPitStopStats(FName VehicleID) const
{
	if (VehicleStats.Contains(VehicleID))
	{
		return VehicleStats[VehicleID];
	}
	return FMGPitStopStats();
}

TArray<FMGPitStopResult> UMGPitStopSubsystem::GetPitStopHistory(FName VehicleID) const
{
	if (VehicleStats.Contains(VehicleID))
	{
		return VehicleStats[VehicleID].PitStopHistory;
	}
	return TArray<FMGPitStopResult>();
}

float UMGPitStopSubsystem::GetFastestPitStop() const
{
	return FastestPitStopTime;
}

FMGPitStopResult UMGPitStopSubsystem::GetLastPitStop(FName VehicleID) const
{
	if (VehicleStats.Contains(VehicleID))
	{
		const TArray<FMGPitStopResult>& History = VehicleStats[VehicleID].PitStopHistory;
		if (History.Num() > 0)
		{
			return History.Last();
		}
	}
	return FMGPitStopResult();
}

void UMGPitStopSubsystem::ResetRaceStats()
{
	RacePitStopHistory.Empty();
	VehicleStats.Empty();
	FastestPitStopTime = 0.0f;
	FastestPitStopVehicle = NAME_None;
}

void UMGPitStopSubsystem::SavePitStopData()
{
	// Placeholder for save implementation
}

void UMGPitStopSubsystem::LoadPitStopData()
{
	// Placeholder for load implementation
}

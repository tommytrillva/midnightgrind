// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleWearSubsystem.h"
#include "Economy/MGEconomySubsystem.h"

void UMGVehicleWearSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get economy subsystem for purchases
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		EconomySubsystem = GI->GetSubsystem<UMGEconomySubsystem>();
	}
}

void UMGVehicleWearSubsystem::Deinitialize()
{
	VehicleWearStates.Empty();
	Super::Deinitialize();
}

// ==========================================
// WEAR TRACKING
// ==========================================

void UMGVehicleWearSubsystem::RegisterVehicle(FGuid VehicleID)
{
	if (!VehicleWearStates.Contains(VehicleID))
	{
		FMGVehicleWearState NewState;
		NewState.VehicleID = VehicleID;
		VehicleWearStates.Add(VehicleID, NewState);
	}
}

void UMGVehicleWearSubsystem::UnregisterVehicle(FGuid VehicleID)
{
	VehicleWearStates.Remove(VehicleID);
}

bool UMGVehicleWearSubsystem::GetWearState(FGuid VehicleID, FMGVehicleWearState& OutState) const
{
	if (const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID))
	{
		OutState = *State;
		return true;
	}
	return false;
}

void UMGVehicleWearSubsystem::UpdateWearFromGameplay(FGuid VehicleID, float DeltaTime, const FVector& Velocity,
	float ThrottleInput, float BrakeInput, float SteeringInput, bool bDrifting, bool bNOSActive)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	float Speed = Velocity.Size() * 0.036f; // cm/s to km/h

	// Calculate approximate wheel slip from inputs
	float WheelSlip = 0.0f;
	if (bDrifting)
	{
		WheelSlip = 0.5f + FMath::Abs(SteeringInput) * 0.3f;
	}
	else if (ThrottleInput > 0.8f && Speed < 50.0f)
	{
		// Wheel spin on acceleration
		WheelSlip = ThrottleInput * 0.3f;
	}

	// Apply tire wear
	ApplyTireWear(VehicleID, WheelSlip, Speed, bDrifting, DeltaTime);

	// Apply engine wear
	// Estimate RPM from speed (simplified)
	float EstimatedRPM = FMath::Clamp(Speed * 50.0f + 1000.0f, 1000.0f, 8000.0f);
	if (ThrottleInput > 0.9f)
	{
		EstimatedRPM *= 1.2f;
	}
	ApplyEngineWear(VehicleID, EstimatedRPM, 8000.0f, ThrottleInput, DeltaTime);

	// Apply brake wear
	if (BrakeInput > 0.1f)
	{
		ApplyBrakeWear(VehicleID, BrakeInput, Speed, DeltaTime);
	}

	// Use nitrous
	if (bNOSActive)
	{
		UseNitrous(VehicleID, 1.0f * DeltaTime); // 1% per second
	}

	// Use fuel (simplified)
	float FuelUseRate = 0.01f + ThrottleInput * 0.02f; // 1-3% per minute base
	if (bNOSActive)
	{
		FuelUseRate *= 2.0f;
	}
	UseFuel(VehicleID, FuelUseRate * DeltaTime / 60.0f);

	// Update mileage
	float DistanceKM = Speed * DeltaTime / 3600.0f;
	State->SessionMileage += static_cast<int32>(DistanceKM * 1000);
	State->TotalMileage += static_cast<int32>(DistanceKM * 1000);

	// Update temperatures
	UpdateEngineState(State->Engine, DeltaTime);
	UpdateBrakeState(State->Brakes, DeltaTime);
}

// ==========================================
// TIRE WEAR
// ==========================================

void UMGVehicleWearSubsystem::ApplyTireWear(FGuid VehicleID, float WheelSlip, float Speed, bool bDrifting, float DeltaTime)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	float WearAmount = BaseTireWearRate * WheelSlip * (Speed / 100.0f) * DeltaTime;

	if (bDrifting)
	{
		WearAmount *= DriftWearMultiplier;
	}

	// Apply to rear tires more if drifting (RWD assumed)
	float FrontWear = WearAmount * 0.3f;
	float RearWear = WearAmount * (bDrifting ? 1.5f : 0.7f);

	// Track previous states for event
	EMGTireWearState PrevFLState = State->Tires.FrontLeft.WearState;
	EMGTireWearState PrevFRState = State->Tires.FrontRight.WearState;
	EMGTireWearState PrevRLState = State->Tires.RearLeft.WearState;
	EMGTireWearState PrevRRState = State->Tires.RearRight.WearState;

	// Apply wear
	State->Tires.FrontLeft.Condition = FMath::Max(0.0f, State->Tires.FrontLeft.Condition - FrontWear);
	State->Tires.FrontRight.Condition = FMath::Max(0.0f, State->Tires.FrontRight.Condition - FrontWear);
	State->Tires.RearLeft.Condition = FMath::Max(0.0f, State->Tires.RearLeft.Condition - RearWear);
	State->Tires.RearRight.Condition = FMath::Max(0.0f, State->Tires.RearRight.Condition - RearWear);

	// Update states
	UpdateTireState(State->Tires.FrontLeft);
	UpdateTireState(State->Tires.FrontRight);
	UpdateTireState(State->Tires.RearLeft);
	UpdateTireState(State->Tires.RearRight);

	// Track distance
	float DistanceKM = Speed * DeltaTime / 3600.0f;
	State->Tires.FrontLeft.TotalDistanceKM += DistanceKM;
	State->Tires.FrontRight.TotalDistanceKM += DistanceKM;
	State->Tires.RearLeft.TotalDistanceKM += DistanceKM;
	State->Tires.RearRight.TotalDistanceKM += DistanceKM;

	if (bDrifting)
	{
		State->Tires.RearLeft.DriftDistanceKM += DistanceKM;
		State->Tires.RearRight.DriftDistanceKM += DistanceKM;
	}

	// Fire events if state changed
	if (State->Tires.RearLeft.WearState != PrevRLState || State->Tires.RearRight.WearState != PrevRRState)
	{
		EMGTireWearState WorstState = FMath::Max(State->Tires.RearLeft.WearState, State->Tires.RearRight.WearState);
		OnTireConditionChanged.Broadcast(VehicleID, WorstState);
	}
}

void UMGVehicleWearSubsystem::ApplyBurnoutWear(FGuid VehicleID, float Duration)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	float WearAmount = BaseTireWearRate * BurnoutWearMultiplier * Duration;

	State->Tires.RearLeft.Condition = FMath::Max(0.0f, State->Tires.RearLeft.Condition - WearAmount);
	State->Tires.RearRight.Condition = FMath::Max(0.0f, State->Tires.RearRight.Condition - WearAmount);
	State->Tires.RearLeft.BurnoutCount++;
	State->Tires.RearRight.BurnoutCount++;

	UpdateTireState(State->Tires.RearLeft);
	UpdateTireState(State->Tires.RearRight);
}

float UMGVehicleWearSubsystem::GetTireGripMultiplier(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return 1.0f;
	}

	// Average of all tires, weighted toward rear for grip
	float FrontGrip = (ConditionToGrip(State->Tires.FrontLeft.Condition) + ConditionToGrip(State->Tires.FrontRight.Condition)) / 2.0f;
	float RearGrip = (ConditionToGrip(State->Tires.RearLeft.Condition) + ConditionToGrip(State->Tires.RearRight.Condition)) / 2.0f;

	return (FrontGrip * 0.4f + RearGrip * 0.6f);
}

float UMGVehicleWearSubsystem::GetIndividualTireGrip(FGuid VehicleID, int32 WheelIndex) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return 1.0f;
	}

	float Condition = 100.0f;
	switch (WheelIndex)
	{
	case 0: Condition = State->Tires.FrontLeft.Condition; break;
	case 1: Condition = State->Tires.FrontRight.Condition; break;
	case 2: Condition = State->Tires.RearLeft.Condition; break;
	case 3: Condition = State->Tires.RearRight.Condition; break;
	}

	return ConditionToGrip(Condition);
}

bool UMGVehicleWearSubsystem::NeedsTireReplacement(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return false;
	}

	return State->Tires.GetWorstCondition() < 25.0f;
}

// ==========================================
// ENGINE WEAR
// ==========================================

void UMGVehicleWearSubsystem::ApplyEngineWear(FGuid VehicleID, float RPM, float MaxRPM, float Throttle, float DeltaTime)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	// Base wear from running
	float WearAmount = BaseEngineWearRate * DeltaTime;

	// Increased wear at high RPM
	float RPMRatio = RPM / MaxRPM;
	if (RPMRatio > 0.9f)
	{
		WearAmount *= 2.0f;
	}

	// Increased wear at full throttle
	if (Throttle > 0.95f)
	{
		WearAmount *= 1.5f;
	}

	// Increased wear when overheating
	if (State->Engine.bOverheating)
	{
		WearAmount *= 5.0f;
		State->Engine.OverheatTime += DeltaTime;
	}

	// Oil degradation
	State->Engine.OilCondition -= WearAmount * 10.0f;
	State->Engine.OilCondition = FMath::Max(0.0f, State->Engine.OilCondition);

	// Poor oil increases engine wear
	if (State->Engine.OilCondition < 30.0f)
	{
		WearAmount *= 2.0f;
	}

	State->Engine.Condition = FMath::Max(0.0f, State->Engine.Condition - WearAmount);

	// Temperature simulation
	float TargetTemp = 85.0f + Throttle * 20.0f + RPMRatio * 15.0f;
	if (State->Engine.CoolantLevel < 50.0f)
	{
		TargetTemp += 30.0f;
	}

	State->Engine.Temperature = FMath::FInterpTo(State->Engine.Temperature, TargetTemp, DeltaTime, 0.1f);

	// Check overheating
	bool bWasOverheating = State->Engine.bOverheating;
	State->Engine.bOverheating = State->Engine.Temperature > EngineOverheatTemp;

	if (State->Engine.bOverheating && !bWasOverheating)
	{
		OnEngineOverheat.Broadcast(VehicleID);
	}
}

void UMGVehicleWearSubsystem::ApplyRedlineDamage(FGuid VehicleID)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	State->Engine.RedlineHits++;
	State->Engine.Condition -= RedlineWearRate;
	State->Engine.Condition = FMath::Max(0.0f, State->Engine.Condition);

	// Extended redline can cause failure
	if (State->Engine.RedlineHits > 100 && State->Engine.Condition < 50.0f)
	{
		OnPartFailure.Broadcast(VehicleID, TEXT("Engine"));
	}
}

bool UMGVehicleWearSubsystem::IsEngineOverheating(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	return State ? State->Engine.bOverheating : false;
}

float UMGVehicleWearSubsystem::GetEnginePowerMultiplier(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return 1.0f;
	}

	float ConditionMult = 0.5f + (State->Engine.Condition / 100.0f) * 0.5f; // 50-100%

	// Overheat penalty
	if (State->Engine.bOverheating)
	{
		ConditionMult *= 0.7f;
	}

	// Low oil penalty
	if (State->Engine.OilCondition < 30.0f)
	{
		ConditionMult *= 0.9f;
	}

	return ConditionMult;
}

// ==========================================
// BRAKE WEAR
// ==========================================

void UMGVehicleWearSubsystem::ApplyBrakeWear(FGuid VehicleID, float BrakeForce, float Speed, float DeltaTime)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	float WearAmount = BaseBrakeWearRate * BrakeForce * (Speed / 100.0f) * DeltaTime;

	State->Brakes.FrontPadCondition -= WearAmount * 0.7f; // Front does more work
	State->Brakes.RearPadCondition -= WearAmount * 0.3f;

	State->Brakes.FrontPadCondition = FMath::Max(0.0f, State->Brakes.FrontPadCondition);
	State->Brakes.RearPadCondition = FMath::Max(0.0f, State->Brakes.RearPadCondition);

	// Rotor wear is slower
	State->Brakes.FrontRotorCondition -= WearAmount * 0.1f;
	State->Brakes.RearRotorCondition -= WearAmount * 0.05f;

	// Temperature from braking
	float HeatGenerated = BrakeForce * Speed * 0.5f;
	State->Brakes.Temperature += HeatGenerated * DeltaTime;

	// Check brake fade
	bool bWasFading = State->Brakes.bBrakeFade;
	State->Brakes.bBrakeFade = State->Brakes.Temperature > BrakeFadeTemp;

	if (State->Brakes.bBrakeFade && !bWasFading)
	{
		OnBrakeFade.Broadcast(VehicleID);
	}
}

bool UMGVehicleWearSubsystem::HasBrakeFade(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	return State ? State->Brakes.bBrakeFade : false;
}

float UMGVehicleWearSubsystem::GetBrakeEffectiveness(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return 1.0f;
	}

	float PadCondition = (State->Brakes.FrontPadCondition + State->Brakes.RearPadCondition) / 2.0f;
	float ConditionMult = 0.3f + (PadCondition / 100.0f) * 0.7f; // 30-100%

	// Brake fade penalty
	if (State->Brakes.bBrakeFade)
	{
		ConditionMult *= 0.5f;
	}

	return ConditionMult;
}

// ==========================================
// CONSUMABLES
// ==========================================

bool UMGVehicleWearSubsystem::UseNitrous(FGuid VehicleID, float Amount)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return false;
	}

	if (State->NitrousRemaining <= 0.0f)
	{
		OnNitrousEmpty.Broadcast(VehicleID);
		return false;
	}

	State->NitrousRemaining = FMath::Max(0.0f, State->NitrousRemaining - Amount);

	if (State->NitrousRemaining <= 0.0f)
	{
		OnNitrousEmpty.Broadcast(VehicleID);
	}

	return true;
}

float UMGVehicleWearSubsystem::GetNitrousRemaining(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	return State ? State->NitrousRemaining : 0.0f;
}

bool UMGVehicleWearSubsystem::RefillNitrous(FGuid OwnerID, FGuid VehicleID)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return false;
	}

	float RefillAmount = 100.0f - State->NitrousRemaining;
	if (RefillAmount <= 0.0f)
	{
		return true; // Already full
	}

	int64 Cost = static_cast<int64>(BaseNitrousRefillCost * (RefillAmount / 100.0f));

	if (EconomySubsystem && !EconomySubsystem->DeductCash(OwnerID, Cost, TEXT("Nitrous Refill")))
	{
		return false;
	}

	State->NitrousRemaining = 100.0f;
	return true;
}

void UMGVehicleWearSubsystem::UseFuel(FGuid VehicleID, float Amount)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (State)
	{
		State->FuelLevel = FMath::Max(0.0f, State->FuelLevel - Amount);
	}
}

float UMGVehicleWearSubsystem::GetFuelRemaining(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	return State ? State->FuelLevel : 0.0f;
}

// ==========================================
// COLLISION DAMAGE
// ==========================================

void UMGVehicleWearSubsystem::ApplyCollisionDamage(FGuid VehicleID, float ImpactForce, const FVector& ImpactPoint)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	// Scale damage based on impact force
	float DamagePercent = FMath::Clamp(ImpactForce / 100000.0f, 0.0f, 50.0f);

	State->BodyCondition = FMath::Max(0.0f, State->BodyCondition - DamagePercent);

	// Heavy impacts can damage engine
	if (ImpactForce > 50000.0f)
	{
		State->Engine.Condition -= DamagePercent * 0.2f;
	}
}

float UMGVehicleWearSubsystem::GetBodyCondition(FGuid VehicleID) const
{
	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	return State ? State->BodyCondition : 100.0f;
}

// ==========================================
// REPAIRS & MAINTENANCE
// ==========================================

FMGRepairEstimate UMGVehicleWearSubsystem::GetRepairEstimate(FGuid VehicleID) const
{
	FMGRepairEstimate Estimate;

	const FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return Estimate;
	}

	// Tires
	if (State->Tires.GetWorstCondition() < 80.0f)
	{
		Estimate.TireReplacementCost = BaseTireCost * 4; // 4 tires
	}

	// Brakes
	if (State->Brakes.FrontPadCondition < 50.0f || State->Brakes.RearPadCondition < 50.0f)
	{
		Estimate.BrakePadCost = BaseBrakePadCost * 4;
	}

	if (State->Brakes.FrontRotorCondition < 70.0f || State->Brakes.RearRotorCondition < 70.0f)
	{
		Estimate.BrakeRotorCost = BaseBrakePadCost * 3; // Rotors more expensive
	}

	// Oil
	if (State->Engine.OilCondition < 50.0f)
	{
		Estimate.OilChangeCost = BaseOilChangeCost;
	}

	// Coolant
	if (State->Engine.CoolantLevel < 80.0f)
	{
		Estimate.CoolantTopOffCost = 50;
	}

	// Engine repair
	if (State->Engine.Condition < 70.0f)
	{
		float DamagePercent = 100.0f - State->Engine.Condition;
		Estimate.EngineRepairCost = static_cast<int64>(DamagePercent * 100);
	}

	// Body repair
	if (State->BodyCondition < 100.0f)
	{
		float DamagePercent = 100.0f - State->BodyCondition;
		Estimate.BodyRepairCost = static_cast<int64>(DamagePercent * BaseBodyRepairCostPerPercent);
	}

	// Nitrous
	if (State->NitrousRemaining < 100.0f)
	{
		float RefillPercent = (100.0f - State->NitrousRemaining) / 100.0f;
		Estimate.NitrousRefillCost = static_cast<int64>(BaseNitrousRefillCost * RefillPercent);
	}

	Estimate.CalculateTotal();
	return Estimate;
}

int64 UMGVehicleWearSubsystem::GetTireReplacementCost(FGuid VehicleID) const
{
	// Cost based on tire compound
	// TODO: Look up actual compound from vehicle data
	return BaseTireCost * 4;
}

bool UMGVehicleWearSubsystem::ReplaceTires(FGuid OwnerID, FGuid VehicleID)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return false;
	}

	int64 Cost = GetTireReplacementCost(VehicleID);

	if (EconomySubsystem && !EconomySubsystem->DeductCash(OwnerID, Cost, TEXT("Tire Replacement")))
	{
		return false;
	}

	// Reset all tires to new
	State->Tires.FrontLeft = FMGTireWearData();
	State->Tires.FrontRight = FMGTireWearData();
	State->Tires.RearLeft = FMGTireWearData();
	State->Tires.RearRight = FMGTireWearData();

	return true;
}

bool UMGVehicleWearSubsystem::PerformOilChange(FGuid OwnerID, FGuid VehicleID)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return false;
	}

	if (EconomySubsystem && !EconomySubsystem->DeductCash(OwnerID, BaseOilChangeCost, TEXT("Oil Change")))
	{
		return false;
	}

	State->Engine.OilLevel = 100.0f;
	State->Engine.OilCondition = 100.0f;

	return true;
}

bool UMGVehicleWearSubsystem::ReplaceBrakePads(FGuid OwnerID, FGuid VehicleID)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return false;
	}

	int64 Cost = BaseBrakePadCost * 4;

	if (EconomySubsystem && !EconomySubsystem->DeductCash(OwnerID, Cost, TEXT("Brake Pad Replacement")))
	{
		return false;
	}

	State->Brakes.FrontPadCondition = 100.0f;
	State->Brakes.RearPadCondition = 100.0f;
	State->Brakes.FluidCondition = 100.0f;

	return true;
}

bool UMGVehicleWearSubsystem::PerformFullRepair(FGuid OwnerID, FGuid VehicleID)
{
	FMGRepairEstimate Estimate = GetRepairEstimate(VehicleID);

	if (EconomySubsystem && !EconomySubsystem->DeductCash(OwnerID, Estimate.TotalCost, TEXT("Full Vehicle Repair")))
	{
		return false;
	}

	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return false;
	}

	// Reset everything
	State->Tires.FrontLeft = FMGTireWearData();
	State->Tires.FrontRight = FMGTireWearData();
	State->Tires.RearLeft = FMGTireWearData();
	State->Tires.RearRight = FMGTireWearData();

	State->Engine.Condition = 100.0f;
	State->Engine.OilLevel = 100.0f;
	State->Engine.OilCondition = 100.0f;
	State->Engine.CoolantLevel = 100.0f;
	State->Engine.Temperature = 85.0f;
	State->Engine.bOverheating = false;
	State->Engine.RedlineHits = 0;
	State->Engine.OverheatTime = 0.0f;

	State->Brakes.FrontPadCondition = 100.0f;
	State->Brakes.RearPadCondition = 100.0f;
	State->Brakes.FrontRotorCondition = 100.0f;
	State->Brakes.RearRotorCondition = 100.0f;
	State->Brakes.FluidCondition = 100.0f;
	State->Brakes.bBrakeFade = false;

	State->BodyCondition = 100.0f;
	State->NitrousRemaining = 100.0f;
	State->FuelLevel = 100.0f;

	return true;
}

bool UMGVehicleWearSubsystem::PerformQuickRepair(FGuid OwnerID, FGuid VehicleID)
{
	FMGVehicleWearState* State = VehicleWearStates.Find(VehicleID);
	if (!State)
	{
		return false;
	}

	float DamagePercent = 100.0f - State->BodyCondition;
	int64 Cost = static_cast<int64>(DamagePercent * BaseBodyRepairCostPerPercent);

	if (EconomySubsystem && !EconomySubsystem->DeductCash(OwnerID, Cost, TEXT("Quick Body Repair")))
	{
		return false;
	}

	State->BodyCondition = 100.0f;

	return true;
}

// ==========================================
// INTERNAL
// ==========================================

EMGTireWearState UMGVehicleWearSubsystem::GetTireWearState(float Condition) const
{
	if (Condition >= 80.0f) return EMGTireWearState::New;
	if (Condition >= 50.0f) return EMGTireWearState::Good;
	if (Condition >= 25.0f) return EMGTireWearState::Worn;
	if (Condition >= 10.0f) return EMGTireWearState::Critical;
	return EMGTireWearState::Destroyed;
}

void UMGVehicleWearSubsystem::UpdateTireState(FMGTireWearData& Tire)
{
	Tire.WearState = GetTireWearState(Tire.Condition);
}

void UMGVehicleWearSubsystem::UpdateEngineState(FMGEngineWearData& Engine, float DeltaTime)
{
	// Cool down when not under load
	if (!Engine.bOverheating)
	{
		Engine.Temperature = FMath::FInterpTo(Engine.Temperature, 85.0f, DeltaTime, 0.05f);
	}
}

void UMGVehicleWearSubsystem::UpdateBrakeState(FMGBrakeWearData& Brakes, float DeltaTime)
{
	// Cool down brakes
	Brakes.Temperature = FMath::FInterpTo(Brakes.Temperature, 50.0f, DeltaTime, 0.1f);

	if (Brakes.Temperature < BrakeFadeTemp * 0.8f)
	{
		Brakes.bBrakeFade = false;
	}
}

float ConditionToGrip(float Condition)
{
	// Grip curve: 100% condition = 1.0 grip, 0% = 0.4 grip
	return 0.4f + (Condition / 100.0f) * 0.6f;
}

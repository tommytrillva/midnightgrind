// Copyright Midnight Grind. All Rights Reserved.

#include "World/MGPoliceSubsystem.h"
#include "Vehicle/MGVehiclePawn.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGPoliceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Default district activity levels
	DistrictActivityLevels.Add(FName(TEXT("Downtown")), 0.8f);
	DistrictActivityLevels.Add(FName(TEXT("Industrial")), 0.4f);
	DistrictActivityLevels.Add(FName(TEXT("Port")), 0.5f);
	DistrictActivityLevels.Add(FName(TEXT("Highway")), 0.6f);
	DistrictActivityLevels.Add(FName(TEXT("Hills")), 0.3f);
	DistrictActivityLevels.Add(FName(TEXT("Suburbs")), 0.5f);

	// Set up tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&UMGPoliceSubsystem::OnTick,
			0.1f, // 10 Hz
			true
		);
	}
}

void UMGPoliceSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	Super::Deinitialize();
}

void UMGPoliceSubsystem::AddHeat(int32 Amount, EMGPoliceEvent Reason)
{
	if (!bPoliceEnabled)
	{
		return;
	}

	EMGHeatLevel OldLevel = CurrentPursuitState.HeatLevel;
	HeatPoints = FMath::Min(HeatPoints + Amount, 1000);

	EMGHeatLevel NewLevel = CalculateHeatLevel(HeatPoints);

	if (NewLevel != OldLevel)
	{
		CurrentPursuitState.HeatLevel = NewLevel;
		OnHeatLevelChanged.Broadcast(OldLevel, NewLevel);

		// Start pursuit if reached pursuit level
		if (NewLevel >= EMGHeatLevel::Pursuit && OldLevel < EMGHeatLevel::Pursuit)
		{
			// Would get player vehicle here
			OnPursuitStarted.Broadcast();
		}

		// Update max pursuing units
		CurrentPursuitState.MaxPursuingUnits = GetMaxUnitsForHeatLevel(NewLevel);
	}

	OnPoliceEvent.Broadcast(Reason);
}

void UMGPoliceSubsystem::RemoveHeat(int32 Amount)
{
	EMGHeatLevel OldLevel = CurrentPursuitState.HeatLevel;
	HeatPoints = FMath::Max(HeatPoints - Amount, 0);

	EMGHeatLevel NewLevel = CalculateHeatLevel(HeatPoints);

	if (NewLevel != OldLevel)
	{
		CurrentPursuitState.HeatLevel = NewLevel;
		OnHeatLevelChanged.Broadcast(OldLevel, NewLevel);

		// End pursuit if dropped below pursuit level
		if (NewLevel < EMGHeatLevel::Pursuit && OldLevel >= EMGHeatLevel::Pursuit)
		{
			EndPursuit(true);
		}
	}
}

void UMGPoliceSubsystem::ClearHeat()
{
	EMGHeatLevel OldLevel = CurrentPursuitState.HeatLevel;
	HeatPoints = 0;
	CurrentPursuitState.HeatLevel = EMGHeatLevel::Clean;
	CurrentPursuitState.bInCooldown = false;
	CurrentPursuitState.CooldownProgress = 0.0f;
	CurrentPursuitState.TotalPursuitTime = 0.0f;

	if (OldLevel != EMGHeatLevel::Clean)
	{
		OnHeatLevelChanged.Broadcast(OldLevel, EMGHeatLevel::Clean);

		if (OldLevel >= EMGHeatLevel::Pursuit)
		{
			EndPursuit(true);
		}
	}

	OnPoliceEvent.Broadcast(EMGPoliceEvent::HeatDecay);
}

void UMGPoliceSubsystem::StartPursuit(AMGVehiclePawn* Target)
{
	if (!bPoliceEnabled || !Target)
	{
		return;
	}

	PursuitTarget = Target;
	CurrentPursuitState.TotalPursuitTime = 0.0f;
	CurrentPursuitState.bInCooldown = false;

	// Spawn initial pursuit units
	int32 InitialUnits = GetMaxUnitsForHeatLevel(CurrentPursuitState.HeatLevel);
	SpawnPursuitUnits(InitialUnits);

	OnPursuitStarted.Broadcast();
}

void UMGPoliceSubsystem::EndPursuit(bool bEscaped)
{
	// Clear pursuing units
	for (AMGPoliceVehicle* Unit : PursuingUnits)
	{
		if (Unit)
		{
			DespawnPatrolUnit(Unit);
		}
	}
	PursuingUnits.Empty();

	// Clear roadblocks
	ClearAllRoadblocks();

	// Reset pursuit state
	CurrentPursuitState.PursuingUnits = 0;
	CurrentPursuitState.ActiveRoadblocks = 0;
	CurrentPursuitState.DeployedSpikeStrips = 0;
	CurrentPursuitState.bInCooldown = false;
	CurrentPursuitState.CooldownProgress = 0.0f;

	PursuitTarget.Reset();

	if (bEscaped)
	{
		OnPoliceEvent.Broadcast(EMGPoliceEvent::EscapedPursuit);
	}

	OnPursuitEnded.Broadcast(bEscaped);
}

void UMGPoliceSubsystem::EnterSafeZone()
{
	CurrentPursuitState.bInSafeZone = true;

	// In safe zone, can hide and lose heat faster
	if (IsInPursuit())
	{
		CurrentPursuitState.bInCooldown = true;
	}

	OnPoliceEvent.Broadcast(EMGPoliceEvent::EnteredSafeZone);
}

void UMGPoliceSubsystem::ExitSafeZone()
{
	CurrentPursuitState.bInSafeZone = false;

	// If still being pursued, reset cooldown
	if (IsInPursuit() && CurrentPursuitState.CooldownProgress < 1.0f)
	{
		CurrentPursuitState.bInCooldown = false;
		CurrentPursuitState.CooldownProgress = 0.0f;
	}
}

FMGBustConsequences UMGPoliceSubsystem::BustPlayer()
{
	FMGBustConsequences Consequences = CalculateBustConsequences();

	// Impound vehicle
	if (PursuitTarget.IsValid())
	{
		FGuid VehicleID; // Would get from vehicle data
		ImpoundedVehicles.Add(VehicleID, FDateTime::UtcNow());
		Consequences.bVehicleImpounded = true;
	}

	// End pursuit
	EndPursuit(false);

	// Clear heat after bust
	ClearHeat();

	OnPlayerBusted.Broadcast(Consequences);
	OnPoliceEvent.Broadcast(EMGPoliceEvent::Busted);

	return Consequences;
}

void UMGPoliceSubsystem::SetDistrictActivity(FName DistrictID, float ActivityLevel)
{
	DistrictActivityLevels.Add(DistrictID, FMath::Clamp(ActivityLevel, 0.0f, 1.0f));
}

float UMGPoliceSubsystem::GetDistrictActivity(FName DistrictID) const
{
	if (const float* Level = DistrictActivityLevels.Find(DistrictID))
	{
		return *Level;
	}
	return 0.5f; // Default
}

AMGPoliceVehicle* UMGPoliceSubsystem::SpawnPatrolUnit(const FMGPoliceSpawnPoint& SpawnPoint)
{
	// In real implementation, would spawn actual police vehicle actor
	// For now, return nullptr as placeholder
	return nullptr;
}

void UMGPoliceSubsystem::DespawnPatrolUnit(AMGPoliceVehicle* Unit)
{
	if (Unit)
	{
		ActivePatrolUnits.Remove(Unit);
		PursuingUnits.Remove(Unit);
		Unit->Destroy();
	}
}

void UMGPoliceSubsystem::RequestRoadblock(FVector Location)
{
	if (CurrentPursuitState.HeatLevel < EMGHeatLevel::Pursuit)
	{
		return;
	}

	// In real implementation, would spawn roadblock actors
	CurrentPursuitState.ActiveRoadblocks++;
}

void UMGPoliceSubsystem::RequestSpikeStrip(FVector Location, FRotator Direction)
{
	if (CurrentPursuitState.HeatLevel < EMGHeatLevel::Manhunt)
	{
		return;
	}

	// In real implementation, would spawn spike strip actor
	CurrentPursuitState.DeployedSpikeStrips++;
}

void UMGPoliceSubsystem::ClearAllRoadblocks()
{
	// In real implementation, would destroy all roadblock actors
	CurrentPursuitState.ActiveRoadblocks = 0;
	CurrentPursuitState.DeployedSpikeStrips = 0;
}

bool UMGPoliceSubsystem::HasImpoundedVehicles() const
{
	return ImpoundedVehicles.Num() > 0;
}

int64 UMGPoliceSubsystem::GetRetrievalCost(FGuid VehicleID) const
{
	// Base cost + storage fee per day
	if (const FDateTime* ImpoundTime = ImpoundedVehicles.Find(VehicleID))
	{
		int32 DaysImpounded = (FDateTime::UtcNow() - *ImpoundTime).GetDays();
		int64 StorageFee = DaysImpounded * 500; // $500 per day
		int64 BaseFee = 2000; // Base retrieval fee
		return BaseFee + StorageFee;
	}
	return 0;
}

bool UMGPoliceSubsystem::RetrieveVehicle(FGuid VehicleID)
{
	return ImpoundedVehicles.Remove(VehicleID) > 0;
}

void UMGPoliceSubsystem::UpdateHeatDecay(float DeltaTime)
{
	if (!IsInPursuit() && HeatPoints > 0)
	{
		// Natural heat decay when not in pursuit
		float DecayAmount = HeatDecayRate * DeltaTime;

		// Faster decay in safe zone
		if (CurrentPursuitState.bInSafeZone)
		{
			DecayAmount *= 3.0f;
		}

		RemoveHeat(FMath::CeilToInt(DecayAmount));
	}
}

void UMGPoliceSubsystem::UpdatePursuit(float DeltaTime)
{
	if (!IsInPursuit())
	{
		return;
	}

	CurrentPursuitState.TotalPursuitTime += DeltaTime;
	CurrentPursuitState.TimeInHeat += DeltaTime;

	// Escalate heat over time
	if (CurrentPursuitState.TimeInHeat > 30.0f && CurrentPursuitState.HeatLevel < EMGHeatLevel::Manhunt)
	{
		AddHeat(50, EMGPoliceEvent::Evading);
		CurrentPursuitState.TimeInHeat = 0.0f;
	}

	// Update pursuing unit count
	CurrentPursuitState.PursuingUnits = PursuingUnits.Num();

	// Calculate distance to nearest unit (simplified)
	float NearestDistance = VehicleDetectionRange;
	if (PursuitTarget.IsValid())
	{
		FVector PlayerLocation = PursuitTarget->GetActorLocation();
		for (AMGPoliceVehicle* Unit : PursuingUnits)
		{
			if (Unit)
			{
				float Distance = FVector::Dist(PlayerLocation, Unit->GetActorLocation());
				NearestDistance = FMath::Min(NearestDistance, Distance);
			}
		}
	}
	CurrentPursuitState.DistanceToNearestUnit = NearestDistance;

	// Request tactical support at high heat
	if (CurrentPursuitState.HeatLevel >= EMGHeatLevel::Pursuit &&
		CurrentPursuitState.ActiveRoadblocks < 2 &&
		CurrentPursuitState.TotalPursuitTime > 20.0f)
	{
		// Would calculate position ahead of player
		RequestRoadblock(FVector::ZeroVector);
	}

	if (CurrentPursuitState.HeatLevel >= EMGHeatLevel::Manhunt &&
		CurrentPursuitState.DeployedSpikeStrips < 3 &&
		CurrentPursuitState.TotalPursuitTime > 40.0f)
	{
		RequestSpikeStrip(FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

void UMGPoliceSubsystem::UpdateCooldown(float DeltaTime)
{
	if (!CurrentPursuitState.bInCooldown)
	{
		return;
	}

	// Progress cooldown
	float ProgressRate = 1.0f / CooldownDuration;

	// Faster cooldown in safe zone
	if (CurrentPursuitState.bInSafeZone)
	{
		ProgressRate *= 2.0f;
	}

	CurrentPursuitState.CooldownProgress += ProgressRate * DeltaTime;
	OnCooldownProgress.Broadcast(CurrentPursuitState.CooldownProgress);

	// Escape successful
	if (CurrentPursuitState.CooldownProgress >= 1.0f)
	{
		EndPursuit(true);
		ClearHeat();
	}
}

EMGHeatLevel UMGPoliceSubsystem::CalculateHeatLevel(int32 Points) const
{
	if (Points >= HeatManhunt)
	{
		return EMGHeatLevel::Manhunt;
	}
	else if (Points >= HeatPursuit)
	{
		return EMGHeatLevel::Pursuit;
	}
	else if (Points >= HeatWanted)
	{
		return EMGHeatLevel::Wanted;
	}
	else if (Points >= HeatNoticed)
	{
		return EMGHeatLevel::Noticed;
	}
	return EMGHeatLevel::Clean;
}

int32 UMGPoliceSubsystem::GetMaxUnitsForHeatLevel(EMGHeatLevel Level) const
{
	switch (Level)
	{
	case EMGHeatLevel::Noticed:
		return 1;
	case EMGHeatLevel::Wanted:
		return 2;
	case EMGHeatLevel::Pursuit:
		return 4;
	case EMGHeatLevel::Manhunt:
		return 8;
	default:
		return 0;
	}
}

void UMGPoliceSubsystem::SpawnPursuitUnits(int32 Count)
{
	// In real implementation, would spawn police vehicles around player
	for (int32 i = 0; i < Count; ++i)
	{
		FMGPoliceSpawnPoint SpawnPoint;
		// Would calculate spawn point relative to player

		AMGPoliceVehicle* Unit = SpawnPatrolUnit(SpawnPoint);
		if (Unit)
		{
			PursuingUnits.Add(Unit);
		}
	}
}

FMGBustConsequences UMGPoliceSubsystem::CalculateBustConsequences() const
{
	FMGBustConsequences Consequences;

	// Calculate based on heat level
	int32 HeatMultiplier = static_cast<int32>(CurrentPursuitState.HeatLevel);

	// Fine: 5-15% of car value (simulated)
	int64 EstimatedCarValue = 50000; // Would get from vehicle data
	float FinePercentage = 0.05f + (HeatMultiplier * 0.025f);
	Consequences.FineAmount = FMath::CeilToInt64(EstimatedCarValue * FinePercentage);

	// REP loss: -200 to -1000
	Consequences.REPLost = 200 + (HeatMultiplier * 200);

	// Retrieval cost
	Consequences.RetrievalCost = Consequences.FineAmount / 2;

	// Days until auction
	Consequences.DaysUntilAuction = 7;

	Consequences.bVehicleImpounded = true;

	return Consequences;
}

void UMGPoliceSubsystem::OnTick()
{
	const float DeltaTime = 0.1f;

	if (!bPoliceEnabled)
	{
		return;
	}

	UpdateHeatDecay(DeltaTime);
	UpdatePursuit(DeltaTime);
	UpdateCooldown(DeltaTime);

	// Check for auctions (impounded vehicles past 7 days)
	FDateTime Now = FDateTime::UtcNow();
	TArray<FGuid> ToAuction;

	for (const auto& Pair : ImpoundedVehicles)
	{
		if ((Now - Pair.Value).GetDays() >= 7)
		{
			ToAuction.Add(Pair.Key);
		}
	}

	for (const FGuid& VehicleID : ToAuction)
	{
		ImpoundedVehicles.Remove(VehicleID);
		// Would notify economy/garage subsystems that vehicle was auctioned
	}
}

// Vehicle detection range constant
constexpr float VehicleDetectionRange = 10000.0f;

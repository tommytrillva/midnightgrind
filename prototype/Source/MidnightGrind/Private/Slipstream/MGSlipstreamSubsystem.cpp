// Copyright Midnight Grind. All Rights Reserved.

#include "Slipstream/MGSlipstreamSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

void UMGSlipstreamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default config
	Config.MaxDraftDistance = 3000.0f;
	Config.MinDraftDistance = 200.0f;
	Config.OptimalDistanceStart = 500.0f;
	Config.OptimalDistanceEnd = 1500.0f;
	Config.DraftConeAngle = 30.0f;
	Config.MaxSpeedBonus = 0.15f;
	Config.NitroChargeRate = 5.0f;
	Config.BuildUpTime = 1.5f;
	Config.FallOffTime = 0.75f;
	Config.MinLeadVehicleSpeed = 50.0f;
	Config.bRequireLineOfSight = true;
	Config.SlingshotBonus = 0.1f;
	Config.SlingshotDuration = 2.0f;

	// Initialize visual config
	VisualConfig.bShowSlipstreamEffect = true;
	VisualConfig.EffectIntensity = 1.0f;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SlipstreamTickHandle,
			this,
			&UMGSlipstreamSubsystem::OnSlipstreamTick,
			0.016f,
			true
		);
	}
}

void UMGSlipstreamSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SlipstreamTickHandle);
	}

	RegisteredVehicles.Empty();
	VehicleStates.Empty();
	VehicleStats.Empty();

	Super::Deinitialize();
}

bool UMGSlipstreamSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGSlipstreamSubsystem::RegisterVehicle(AActor* Vehicle, const FMGVehicleSlipstreamData& Data)
{
	if (!Vehicle)
	{
		return;
	}

	FMGVehicleSlipstreamData VehicleData = Data;
	VehicleData.Vehicle = Vehicle;
	RegisteredVehicles.Add(Vehicle, VehicleData);

	FMGSlipstreamState State;
	VehicleStates.Add(Vehicle, State);

	FMGSlipstreamStats Stats;
	VehicleStats.Add(Vehicle, Stats);
}

void UMGSlipstreamSubsystem::UnregisterVehicle(AActor* Vehicle)
{
	if (!Vehicle)
	{
		return;
	}

	RegisteredVehicles.Remove(Vehicle);
	VehicleStates.Remove(Vehicle);
	VehicleStats.Remove(Vehicle);
}

void UMGSlipstreamSubsystem::UpdateVehicleData(AActor* Vehicle, const FMGVehicleSlipstreamData& Data)
{
	if (FMGVehicleSlipstreamData* ExistingData = RegisteredVehicles.Find(Vehicle))
	{
		*ExistingData = Data;
		ExistingData->Vehicle = Vehicle;
	}
}

bool UMGSlipstreamSubsystem::IsVehicleRegistered(AActor* Vehicle) const
{
	return RegisteredVehicles.Contains(Vehicle);
}

FMGSlipstreamState UMGSlipstreamSubsystem::GetSlipstreamState(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		return *State;
	}
	return FMGSlipstreamState();
}

bool UMGSlipstreamSubsystem::IsInSlipstream(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		return State->bIsInSlipstream;
	}
	return false;
}

float UMGSlipstreamSubsystem::GetCurrentSpeedBonus(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		float Bonus = State->CurrentBonus;

		if (State->bSlingshotActive)
		{
			Bonus += Config.SlingshotBonus;
		}

		return Bonus;
	}
	return 0.0f;
}

EMGSlipstreamStrength UMGSlipstreamSubsystem::GetSlipstreamStrength(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		return State->Strength;
	}
	return EMGSlipstreamStrength::None;
}

EMGDraftingZone UMGSlipstreamSubsystem::GetDraftingZone(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		return State->Zone;
	}
	return EMGDraftingZone::None;
}

AActor* UMGSlipstreamSubsystem::GetLeadVehicle(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		return State->LeadVehicle;
	}
	return nullptr;
}

bool UMGSlipstreamSubsystem::ActivateSlingshot(AActor* Vehicle)
{
	FMGSlipstreamState* State = VehicleStates.Find(Vehicle);
	if (!State || !State->bSlingshotReady || State->bSlingshotActive)
	{
		return false;
	}

	State->bSlingshotActive = true;
	State->bSlingshotReady = false;
	State->SlingshotTimeRemaining = Config.SlingshotDuration;
	State->ChargeLevel = 0.0f;

	// Update stats
	if (FMGSlipstreamStats* Stats = VehicleStats.Find(Vehicle))
	{
		Stats->SlingshotsPerformed++;
	}

	OnSlingshotActivated.Broadcast(Config.SlingshotBonus);

	return true;
}

bool UMGSlipstreamSubsystem::IsSlingshotReady(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		return State->bSlingshotReady;
	}
	return false;
}

bool UMGSlipstreamSubsystem::IsSlingshotActive(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		return State->bSlingshotActive;
	}
	return false;
}

float UMGSlipstreamSubsystem::GetSlingshotChargePercent(AActor* Vehicle) const
{
	if (const FMGSlipstreamState* State = VehicleStates.Find(Vehicle))
	{
		return FMath::Clamp(State->ChargeLevel * 100.0f, 0.0f, 100.0f);
	}
	return 0.0f;
}

void UMGSlipstreamSubsystem::SetConfig(const FMGSlipstreamConfig& NewConfig)
{
	Config = NewConfig;
}

void UMGSlipstreamSubsystem::SetVisualConfig(const FMGSlipstreamVisual& NewVisual)
{
	VisualConfig = NewVisual;
}

FMGSlipstreamStats UMGSlipstreamSubsystem::GetStats(AActor* Vehicle) const
{
	if (const FMGSlipstreamStats* Stats = VehicleStats.Find(Vehicle))
	{
		return *Stats;
	}
	return FMGSlipstreamStats();
}

void UMGSlipstreamSubsystem::ResetStats(AActor* Vehicle)
{
	if (FMGSlipstreamStats* Stats = VehicleStats.Find(Vehicle))
	{
		*Stats = FMGSlipstreamStats();
	}
}

void UMGSlipstreamSubsystem::ResetAllStats()
{
	for (auto& Pair : VehicleStats)
	{
		Pair.Value = FMGSlipstreamStats();
	}
}

TArray<AActor*> UMGSlipstreamSubsystem::GetVehiclesInSlipstream(AActor* LeadVehicle) const
{
	TArray<AActor*> Result;

	for (const auto& Pair : VehicleStates)
	{
		if (Pair.Value.LeadVehicle == LeadVehicle && Pair.Value.bIsInSlipstream)
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

int32 UMGSlipstreamSubsystem::GetDraftingTrainLength(AActor* LeadVehicle) const
{
	return GetVehiclesInSlipstream(LeadVehicle).Num() + 1;
}

AActor* UMGSlipstreamSubsystem::FindBestDraftTarget(AActor* Vehicle) const
{
	const FMGVehicleSlipstreamData* VehicleData = RegisteredVehicles.Find(Vehicle);
	if (!VehicleData)
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	for (const auto& Pair : RegisteredVehicles)
	{
		AActor* Candidate = Pair.Key;
		if (Candidate == Vehicle)
		{
			continue;
		}

		const FMGVehicleSlipstreamData& CandidateData = Pair.Value;

		// Check if candidate is ahead
		FVector ToCandidate = CandidateData.Position - VehicleData->Position;
		float DotProduct = FVector::DotProduct(ToCandidate.GetSafeNormal(), VehicleData->ForwardVector);

		if (DotProduct <= 0.0f)
		{
			continue;
		}

		float Distance = ToCandidate.Size();
		if (Distance > Config.MaxDraftDistance || Distance < Config.MinDraftDistance)
		{
			continue;
		}

		if (!IsInDraftingCone(Vehicle, Candidate))
		{
			continue;
		}

		// Score based on distance and speed
		float DistanceScore = 1.0f - (Distance / Config.MaxDraftDistance);
		float SpeedScore = CandidateData.Speed / 200.0f;
		float Score = DistanceScore * 0.6f + SpeedScore * 0.4f;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void UMGSlipstreamSubsystem::SetDebugDrawEnabled(bool bEnabled)
{
	bDebugDraw = bEnabled;
}

void UMGSlipstreamSubsystem::OnSlipstreamTick()
{
	float DeltaTime = 0.016f;

	for (auto& Pair : VehicleStates)
	{
		AActor* Vehicle = Pair.Key;
		UpdateVehicleSlipstream(Vehicle);

		if (bDebugDraw)
		{
			DrawDebugSlipstream(Vehicle, Pair.Value);
		}
	}
}

void UMGSlipstreamSubsystem::UpdateVehicleSlipstream(AActor* Vehicle)
{
	FMGSlipstreamState* State = VehicleStates.Find(Vehicle);
	FMGVehicleSlipstreamData* VehicleData = RegisteredVehicles.Find(Vehicle);

	if (!State || !VehicleData)
	{
		return;
	}

	float DeltaTime = 0.016f;
	bool bWasInSlipstream = State->bIsInSlipstream;
	EMGSlipstreamStrength OldStrength = State->Strength;

	// Find potential lead vehicle
	AActor* NewLeader = FindLeadVehicle(Vehicle);

	if (NewLeader)
	{
		State->LeadVehicle = NewLeader;
		State->bIsInSlipstream = true;

		FMGVehicleSlipstreamData* LeaderData = RegisteredVehicles.Find(NewLeader);
		if (LeaderData)
		{
			State->DistanceToLeader = FVector::Dist(VehicleData->Position, LeaderData->Position);

			FVector ToLeader = LeaderData->Position - VehicleData->Position;
			State->AngleToLeader = FMath::RadiansToDegrees(
				FMath::Acos(FVector::DotProduct(ToLeader.GetSafeNormal(), VehicleData->ForwardVector))
			);
		}

		ProcessSlipstreamState(Vehicle, *State);

		if (!bWasInSlipstream)
		{
			OnSlipstreamEntered.Broadcast(NewLeader, State->Zone);
		}
	}
	else
	{
		// Fall off slipstream
		if (State->bIsInSlipstream)
		{
			State->ChargeLevel = FMath::Max(0.0f, State->ChargeLevel - DeltaTime / Config.FallOffTime);

			if (State->ChargeLevel <= 0.0f)
			{
				State->bIsInSlipstream = false;
				State->LeadVehicle = nullptr;
				State->Strength = EMGSlipstreamStrength::None;
				State->Zone = EMGDraftingZone::None;
				State->CurrentBonus = 0.0f;

				OnSlipstreamExited.Broadcast();
			}
		}
	}

	// Update slingshot
	UpdateSlingshot(Vehicle, *State);

	// Check strength change
	if (State->Strength != OldStrength)
	{
		OnSlipstreamStrengthChanged.Broadcast(State->Strength);
	}

	// Update stats
	if (State->bIsInSlipstream)
	{
		if (FMGSlipstreamStats* Stats = VehicleStats.Find(Vehicle))
		{
			Stats->TotalTimeInSlipstream += DeltaTime;

			if (State->TimeInSlipstream > Stats->LongestSlipstreamDuration)
			{
				Stats->LongestSlipstreamDuration = State->TimeInSlipstream;
			}

			if (VehicleData->Speed > 0.0f)
			{
				Stats->TotalDistanceInSlipstream += VehicleData->Speed * DeltaTime * 0.01f;
			}
		}
	}
}

void UMGSlipstreamSubsystem::ProcessSlipstreamState(AActor* Vehicle, FMGSlipstreamState& State)
{
	float DeltaTime = 0.016f;

	State.Zone = DetermineZone(State.DistanceToLeader);
	State.TimeInSlipstream += DeltaTime;

	// Build up charge based on zone
	float ChargeRate = 0.0f;
	switch (State.Zone)
	{
	case EMGDraftingZone::Optimal:
		ChargeRate = 1.0f;
		break;
	case EMGDraftingZone::Inner:
		ChargeRate = 0.75f;
		break;
	case EMGDraftingZone::Outer:
		ChargeRate = 0.5f;
		break;
	case EMGDraftingZone::TooClose:
		ChargeRate = 0.25f;
		break;
	default:
		ChargeRate = 0.0f;
		break;
	}

	State.ChargeLevel = FMath::Clamp(State.ChargeLevel + ChargeRate * DeltaTime / Config.BuildUpTime, 0.0f, 1.0f);
	State.Strength = DetermineStrength(State.ChargeLevel);
	State.CurrentBonus = CalculateSlipstreamBonus(State.DistanceToLeader, State.Zone) * State.ChargeLevel;

	// Charge nitro
	if (State.ChargeLevel > 0.5f)
	{
		float NitroCharge = Config.NitroChargeRate * DeltaTime * State.ChargeLevel;

		if (FMGSlipstreamStats* Stats = VehicleStats.Find(Vehicle))
		{
			Stats->NitroChargedFromDrafting += NitroCharge;
		}

		OnDraftingNitroCharged.Broadcast(NitroCharge);
	}

	// Check slingshot ready
	if (State.ChargeLevel >= 1.0f && !State.bSlingshotReady && !State.bSlingshotActive)
	{
		State.bSlingshotReady = true;
		OnSlingshotReady.Broadcast();
	}
}

void UMGSlipstreamSubsystem::UpdateSlingshot(AActor* Vehicle, FMGSlipstreamState& State)
{
	if (!State.bSlingshotActive)
	{
		return;
	}

	float DeltaTime = 0.016f;
	State.SlingshotTimeRemaining -= DeltaTime;

	if (State.SlingshotTimeRemaining <= 0.0f)
	{
		State.bSlingshotActive = false;
		State.SlingshotTimeRemaining = 0.0f;
		OnSlingshotEnded.Broadcast();
	}
}

AActor* UMGSlipstreamSubsystem::FindLeadVehicle(AActor* Vehicle) const
{
	const FMGVehicleSlipstreamData* VehicleData = RegisteredVehicles.Find(Vehicle);
	if (!VehicleData || VehicleData->Speed < 10.0f)
	{
		return nullptr;
	}

	AActor* BestLeader = nullptr;
	float BestDistance = Config.MaxDraftDistance;

	for (const auto& Pair : RegisteredVehicles)
	{
		AActor* Candidate = Pair.Key;
		if (Candidate == Vehicle)
		{
			continue;
		}

		const FMGVehicleSlipstreamData& CandidateData = Pair.Value;

		// Check minimum speed
		if (CandidateData.Speed < Config.MinLeadVehicleSpeed)
		{
			continue;
		}

		// Check if candidate is ahead
		FVector ToCandidate = CandidateData.Position - VehicleData->Position;
		float Distance = ToCandidate.Size();

		if (Distance > Config.MaxDraftDistance || Distance < Config.MinDraftDistance)
		{
			continue;
		}

		// Check drafting cone
		if (!IsInDraftingCone(Vehicle, Candidate))
		{
			continue;
		}

		// Check line of sight
		if (Config.bRequireLineOfSight && !HasLineOfSight(Vehicle, Candidate))
		{
			continue;
		}

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestLeader = Candidate;
		}
	}

	return BestLeader;
}

bool UMGSlipstreamSubsystem::IsInDraftingCone(AActor* Follower, AActor* Leader) const
{
	const FMGVehicleSlipstreamData* FollowerData = RegisteredVehicles.Find(Follower);
	const FMGVehicleSlipstreamData* LeaderData = RegisteredVehicles.Find(Leader);

	if (!FollowerData || !LeaderData)
	{
		return false;
	}

	FVector ToLeader = LeaderData->Position - FollowerData->Position;
	ToLeader.Z = 0.0f;
	ToLeader.Normalize();

	FVector FollowerForward = FollowerData->ForwardVector;
	FollowerForward.Z = 0.0f;
	FollowerForward.Normalize();

	float DotProduct = FVector::DotProduct(ToLeader, FollowerForward);
	float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	return Angle <= Config.DraftConeAngle;
}

bool UMGSlipstreamSubsystem::HasLineOfSight(AActor* Follower, AActor* Leader) const
{
	if (!GetWorld())
	{
		return true;
	}

	const FMGVehicleSlipstreamData* FollowerData = RegisteredVehicles.Find(Follower);
	const FMGVehicleSlipstreamData* LeaderData = RegisteredVehicles.Find(Leader);

	if (!FollowerData || !LeaderData)
	{
		return false;
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Follower);
	QueryParams.AddIgnoredActor(Leader);

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		FollowerData->Position,
		LeaderData->Position,
		ECC_Visibility,
		QueryParams
	);

	return !bHit;
}

float UMGSlipstreamSubsystem::CalculateSlipstreamBonus(float Distance, EMGDraftingZone Zone) const
{
	float BaseBonus = 0.0f;

	switch (Zone)
	{
	case EMGDraftingZone::Optimal:
		BaseBonus = Config.MaxSpeedBonus;
		break;
	case EMGDraftingZone::Inner:
		BaseBonus = Config.MaxSpeedBonus * 0.85f;
		break;
	case EMGDraftingZone::Outer:
		{
			float t = (Distance - Config.OptimalDistanceEnd) / (Config.MaxDraftDistance - Config.OptimalDistanceEnd);
			BaseBonus = Config.MaxSpeedBonus * (1.0f - t) * 0.6f;
		}
		break;
	case EMGDraftingZone::TooClose:
		BaseBonus = Config.MaxSpeedBonus * 0.3f;
		break;
	default:
		BaseBonus = 0.0f;
		break;
	}

	return BaseBonus;
}

EMGDraftingZone UMGSlipstreamSubsystem::DetermineZone(float Distance) const
{
	if (Distance < Config.MinDraftDistance)
	{
		return EMGDraftingZone::TooClose;
	}
	else if (Distance < Config.OptimalDistanceStart)
	{
		return EMGDraftingZone::Inner;
	}
	else if (Distance <= Config.OptimalDistanceEnd)
	{
		return EMGDraftingZone::Optimal;
	}
	else if (Distance <= Config.MaxDraftDistance)
	{
		return EMGDraftingZone::Outer;
	}

	return EMGDraftingZone::None;
}

EMGSlipstreamStrength UMGSlipstreamSubsystem::DetermineStrength(float ChargeLevel) const
{
	if (ChargeLevel >= 1.0f)
	{
		return EMGSlipstreamStrength::Maximum;
	}
	else if (ChargeLevel >= 0.75f)
	{
		return EMGSlipstreamStrength::Strong;
	}
	else if (ChargeLevel >= 0.5f)
	{
		return EMGSlipstreamStrength::Moderate;
	}
	else if (ChargeLevel >= 0.25f)
	{
		return EMGSlipstreamStrength::Weak;
	}

	return EMGSlipstreamStrength::None;
}

void UMGSlipstreamSubsystem::DrawDebugSlipstream(AActor* Vehicle, const FMGSlipstreamState& State)
{
#if ENABLE_DRAW_DEBUG
	if (!GetWorld() || !Vehicle)
	{
		return;
	}

	const FMGVehicleSlipstreamData* VehicleData = RegisteredVehicles.Find(Vehicle);
	if (!VehicleData)
	{
		return;
	}

	FColor Color = FColor::White;
	switch (State.Strength)
	{
	case EMGSlipstreamStrength::Weak:
		Color = FColor::Blue;
		break;
	case EMGSlipstreamStrength::Moderate:
		Color = FColor::Cyan;
		break;
	case EMGSlipstreamStrength::Strong:
		Color = FColor::Green;
		break;
	case EMGSlipstreamStrength::Maximum:
		Color = FColor::Yellow;
		break;
	default:
		Color = FColor::White;
		break;
	}

	if (State.bIsInSlipstream && State.LeadVehicle)
	{
		if (const FMGVehicleSlipstreamData* LeaderData = RegisteredVehicles.Find(State.LeadVehicle))
		{
			DrawDebugLine(
				GetWorld(),
				VehicleData->Position,
				LeaderData->Position,
				Color,
				false,
				-1.0f,
				0,
				3.0f
			);
		}
	}

	// Draw draft cone
	FVector ConeStart = VehicleData->Position;
	FVector ConeEnd = ConeStart + VehicleData->ForwardVector * Config.MaxDraftDistance;
	DrawDebugCone(
		GetWorld(),
		ConeStart,
		VehicleData->ForwardVector,
		Config.MaxDraftDistance,
		FMath::DegreesToRadians(Config.DraftConeAngle),
		FMath::DegreesToRadians(Config.DraftConeAngle),
		12,
		FColor::White,
		false,
		-1.0f
	);
#endif
}

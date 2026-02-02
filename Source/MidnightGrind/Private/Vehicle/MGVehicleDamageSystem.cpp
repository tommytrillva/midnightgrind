// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehicleDamageSystem.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"

UMGVehicleDamageSystem::UMGVehicleDamageSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Default component damage multipliers
	ComponentDamageMultipliers.Add(EMGDamageComponent::Body, 1.0f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::Engine, 0.5f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::Transmission, 0.3f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::Suspension, 0.4f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::Steering, 0.4f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::Brakes, 0.3f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::Wheels, 0.5f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::Aero, 0.6f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::Cooling, 0.4f);
	ComponentDamageMultipliers.Add(EMGDamageComponent::NOS, 0.2f);
}

void UMGVehicleDamageSystem::BeginPlay()
{
	Super::BeginPlay();

	// Cache references
	VehiclePawn = Cast<AMGVehiclePawn>(GetOwner());
	if (VehiclePawn.IsValid())
	{
		MovementComponent = VehiclePawn->GetMGVehicleMovement();
	}

	InitializeComponents();
}

void UMGVehicleDamageSystem::TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAutoRepair(DeltaTime);
	UpdateRepairs(DeltaTime);

	// Scrape detection - if no collision for a while, stop scraping
	TimeSinceLastCollision += DeltaTime;
	if (bIsScraping && TimeSinceLastCollision > ScrapeDetectionWindow)
	{
		bIsScraping = false;
		RecentCollisionCount = 0;
		OnScrapeEnd.Broadcast();
	}
}

// ==========================================
// INITIALIZATION
// ==========================================

void UMGVehicleDamageSystem::InitializeComponents()
{
	// Initialize all component states
	for (int32 i = 0; i < static_cast<int32>(EMGDamageComponent::NOS) + 1; ++i)
	{
		EMGDamageComponent Component = static_cast<EMGDamageComponent>(i);
		FMGComponentDamageState State;
		State.Component = Component;
		State.Health = 100.0f;
		State.bIsBroken = false;
		State.PerformanceMultiplier = 1.0f;
		ComponentStates.Add(Component, State);
	}

	// Initialize visual damage zones
	for (int32 i = 0; i < static_cast<int32>(EMGDamageZone::Bottom) + 1; ++i)
	{
		EMGDamageZone Zone = static_cast<EMGDamageZone>(i);
		VisualDamage.ZoneDeformation.Add(Zone, 0.0f);
		VisualDamage.ZoneScratchDamage.Add(Zone, 0.0f);
	}
}

// ==========================================
// DAMAGE APPLICATION
// ==========================================

void UMGVehicleDamageSystem::ApplyCollisionDamage(const FHitResult& HitResult, float ImpactForce, AActor* OtherActor)
{
	// Track collisions for scrape detection even if damage is below threshold
	RecentCollisionCount++;
	TimeSinceLastCollision = 0.0f;
	LastScrapePoint = HitResult.ImpactPoint;

	// Detect scraping (multiple low-force impacts in quick succession)
	if (RecentCollisionCount >= 3 && !bIsScraping)
	{
		bIsScraping = true;
		float ScrapeIntensity = FMath::Clamp(ImpactForce / MaxImpactForce, 0.1f, 1.0f);
		OnScrapeStart.Broadcast(HitResult.ImpactPoint, ScrapeIntensity);
	}

	if (bIsTotaled || ImpactForce < MinImpactForceForDamage)
	{
		return;
	}

	// Convert hit location to local space
	FVector LocalHitLocation = FVector::ZeroVector;
	if (VehiclePawn.IsValid())
	{
		LocalHitLocation = VehiclePawn->GetActorTransform().InverseTransformPosition(HitResult.ImpactPoint);
	}

	// Determine damage zone
	EMGDamageZone Zone = DetermineZoneFromHit(LocalHitLocation);

	// Calculate base damage from impact force
	float NormalizedForce = FMath::GetMappedRangeValueClamped(
		FVector2D(MinImpactForceForDamage, MaxImpactForce),
		FVector2D(0.0f, 1.0f),
		ImpactForce
	);
	float RawDamage = NormalizedForce * 50.0f; // Max 50 damage per hit

	// Apply resistance
	float FinalDamage = CalculateDamageAfterResistance(RawDamage, Zone);

	// Create damage event
	FMGDamageEvent DamageEvent;
	DamageEvent.ImpactForce = ImpactForce;
	DamageEvent.ImpactLocation = HitResult.ImpactPoint;
	DamageEvent.ImpactNormal = HitResult.ImpactNormal;
	DamageEvent.DamageZone = Zone;
	DamageEvent.OtherActor = OtherActor;
	DamageEvent.bWasVehicleCollision = OtherActor && OtherActor->IsA(AMGVehiclePawn::StaticClass());
	DamageEvent.DamageDealt = FinalDamage;

	// Apply damage
	ApplyZoneDamage(Zone, FinalDamage);

	// Propagate to components
	PropagateToComponents(Zone, FinalDamage);

	// Update visual damage
	if (bEnableVisualDamage)
	{
		UpdateVisualDamage(Zone, FinalDamage);
	}

	OnDamageTaken.Broadcast(DamageEvent);
}

void UMGVehicleDamageSystem::ApplyZoneDamage(EMGDamageZone Zone, float DamageAmount)
{
	if (bIsTotaled)
	{
		return;
	}

	// Apply to body component
	ApplyComponentDamage(EMGDamageComponent::Body, DamageAmount);

	CheckTotaledState();
}

void UMGVehicleDamageSystem::ApplyComponentDamage(EMGDamageComponent Component, float DamageAmount)
{
	if (bIsTotaled)
	{
		return;
	}

	FMGComponentDamageState* State = ComponentStates.Find(Component);
	if (!State)
	{
		return;
	}

	// Apply component-specific damage multiplier
	float* Multiplier = ComponentDamageMultipliers.Find(Component);
	float AdjustedDamage = DamageAmount * (Multiplier ? *Multiplier : 1.0f);

	float OldHealth = State->Health;
	State->Health = FMath::Max(0.0f, State->Health - AdjustedDamage);

	// Update performance
	UpdateComponentPerformance(Component);

	// Check if component broke
	if (OldHealth > 0.0f && State->Health <= 0.0f)
	{
		State->bIsBroken = true;
		OnComponentBroken.Broadcast(Component);

		// Special effects for broken components
		if (Component == EMGDamageComponent::Engine)
		{
			VisualDamage.bIsSmoking = true;
		}
	}

	OnComponentDamaged.Broadcast(Component, State->Health);

	// Apply performance effects
	ApplyPerformanceEffects();
}

void UMGVehicleDamageSystem::ApplyGlobalDamage(float DamagePercent)
{
	float DamageAmount = DamagePercent;

	for (auto& Pair : ComponentStates)
	{
		ApplyComponentDamage(Pair.Key, DamageAmount);
	}
}

// ==========================================
// REPAIR
// ==========================================

void UMGVehicleDamageSystem::StartRepair(EMGDamageComponent Component, float RepairDuration)
{
	FMGComponentDamageState* State = ComponentStates.Find(Component);
	if (!State || State->Health >= 100.0f)
	{
		return;
	}

	State->bIsRepairing = true;
	State->RepairProgress = 0.0f;
}

void UMGVehicleDamageSystem::InstantRepair(EMGDamageComponent Component)
{
	FMGComponentDamageState* State = ComponentStates.Find(Component);
	if (!State)
	{
		return;
	}

	State->Health = 100.0f;
	State->bIsBroken = false;
	State->bIsRepairing = false;
	State->RepairProgress = 0.0f;

	UpdateComponentPerformance(Component);
	ApplyPerformanceEffects();

	// Clear visual effects
	if (Component == EMGDamageComponent::Engine)
	{
		VisualDamage.bIsSmoking = false;
		VisualDamage.bIsOnFire = false;
	}

	OnComponentRepaired.Broadcast(Component);
}

void UMGVehicleDamageSystem::InstantRepairAll()
{
	for (auto& Pair : ComponentStates)
	{
		InstantRepair(Pair.Key);
	}

	// Reset visual damage
	for (auto& Zone : VisualDamage.ZoneDeformation)
	{
		Zone.Value = 0.0f;
	}
	for (auto& Zone : VisualDamage.ZoneScratchDamage)
	{
		Zone.Value = 0.0f;
	}
	VisualDamage.bHeadlightsBroken = false;
	VisualDamage.bTaillightsBroken = false;
	VisualDamage.WindowDamage = 0.0f;

	bIsTotaled = false;

	OnVisualDamageUpdated.Broadcast(VisualDamage);
}

void UMGVehicleDamageSystem::CancelRepair(EMGDamageComponent Component)
{
	FMGComponentDamageState* State = ComponentStates.Find(Component);
	if (State)
	{
		State->bIsRepairing = false;
		State->RepairProgress = 0.0f;
	}
}

int32 UMGVehicleDamageSystem::GetRepairCost(EMGDamageComponent Component) const
{
	const FMGComponentDamageState* State = ComponentStates.Find(Component);
	if (!State)
	{
		return 0;
	}

	float DamagePercent = (100.0f - State->Health) / 100.0f;

	// Base cost per component
	int32 BaseCost = 0;
	switch (Component)
	{
	case EMGDamageComponent::Body:      BaseCost = 500; break;
	case EMGDamageComponent::Engine:    BaseCost = 2000; break;
	case EMGDamageComponent::Transmission: BaseCost = 1500; break;
	case EMGDamageComponent::Suspension: BaseCost = 800; break;
	case EMGDamageComponent::Steering:  BaseCost = 600; break;
	case EMGDamageComponent::Brakes:    BaseCost = 400; break;
	case EMGDamageComponent::Wheels:    BaseCost = 300; break;
	case EMGDamageComponent::Aero:      BaseCost = 700; break;
	case EMGDamageComponent::Cooling:   BaseCost = 500; break;
	case EMGDamageComponent::NOS:       BaseCost = 400; break;
	}

	return FMath::RoundToInt(BaseCost * DamagePercent);
}

int32 UMGVehicleDamageSystem::GetTotalRepairCost() const
{
	int32 Total = 0;
	for (const auto& Pair : ComponentStates)
	{
		Total += GetRepairCost(Pair.Key);
	}
	return Total;
}

// ==========================================
// STATE QUERIES
// ==========================================

float UMGVehicleDamageSystem::GetOverallDamagePercent() const
{
	float TotalHealth = 0.0f;
	for (const auto& Pair : ComponentStates)
	{
		TotalHealth += Pair.Value.Health;
	}

	float AverageHealth = TotalHealth / FMath::Max(1, ComponentStates.Num());
	return 100.0f - AverageHealth;
}

FMGComponentDamageState UMGVehicleDamageSystem::GetComponentState(EMGDamageComponent Component) const
{
	const FMGComponentDamageState* State = ComponentStates.Find(Component);
	if (State)
	{
		return *State;
	}
	return FMGComponentDamageState();
}

TArray<FMGComponentDamageState> UMGVehicleDamageSystem::GetAllComponentStates() const
{
	TArray<FMGComponentDamageState> States;
	for (const auto& Pair : ComponentStates)
	{
		States.Add(Pair.Value);
	}
	return States;
}

bool UMGVehicleDamageSystem::IsComponentBroken(EMGDamageComponent Component) const
{
	const FMGComponentDamageState* State = ComponentStates.Find(Component);
	return State ? State->bIsBroken : false;
}

bool UMGVehicleDamageSystem::IsRepairing() const
{
	for (const auto& Pair : ComponentStates)
	{
		if (Pair.Value.bIsRepairing)
		{
			return true;
		}
	}
	return false;
}

float UMGVehicleDamageSystem::GetComponentPerformance(EMGDamageComponent Component) const
{
	const FMGComponentDamageState* State = ComponentStates.Find(Component);
	return State ? State->PerformanceMultiplier : 1.0f;
}

// ==========================================
// INTERNAL
// ==========================================

EMGDamageZone UMGVehicleDamageSystem::DetermineZoneFromHit(const FVector& LocalHitLocation) const
{
	// Determine zone based on local hit position
	// Assuming vehicle faces forward (+X), with +Y being right

	float X = LocalHitLocation.X;
	float Y = LocalHitLocation.Y;
	float Z = LocalHitLocation.Z;

	// Top/Bottom check
	if (Z > 100.0f)
	{
		return EMGDamageZone::Top;
	}
	if (Z < -50.0f)
	{
		return EMGDamageZone::Bottom;
	}

	// Front/Rear
	bool bIsFront = X > 0;
	bool bIsRear = X < -100.0f;

	// Left/Right
	bool bIsLeft = Y < -50.0f;
	bool bIsRight = Y > 50.0f;

	if (bIsFront)
	{
		if (bIsLeft) return EMGDamageZone::FrontLeft;
		if (bIsRight) return EMGDamageZone::FrontRight;
		return EMGDamageZone::Front;
	}
	else if (bIsRear)
	{
		if (bIsLeft) return EMGDamageZone::RearLeft;
		if (bIsRight) return EMGDamageZone::RearRight;
		return EMGDamageZone::Rear;
	}
	else
	{
		if (bIsLeft) return EMGDamageZone::Left;
		if (bIsRight) return EMGDamageZone::Right;
		return EMGDamageZone::Front; // Default
	}
}

float UMGVehicleDamageSystem::CalculateDamageAfterResistance(float RawDamage, EMGDamageZone Zone) const
{
	float Resistance = BaseDamageResistance;

	// Add zone-specific resistance
	const float* ZoneResistance = ZoneDamageResistance.Find(Zone);
	if (ZoneResistance)
	{
		Resistance += *ZoneResistance;
	}

	// Apply resistance (capped at 90%)
	Resistance = FMath::Clamp(Resistance, 0.0f, 0.9f);
	return RawDamage * (1.0f - Resistance);
}

void UMGVehicleDamageSystem::PropagateToComponents(EMGDamageZone Zone, float Damage)
{
	// Determine which components are affected by each zone
	TArray<EMGDamageComponent> AffectedComponents;

	switch (Zone)
	{
	case EMGDamageZone::Front:
	case EMGDamageZone::FrontLeft:
	case EMGDamageZone::FrontRight:
		AffectedComponents.Add(EMGDamageComponent::Engine);
		AffectedComponents.Add(EMGDamageComponent::Cooling);
		AffectedComponents.Add(EMGDamageComponent::Steering);
		AffectedComponents.Add(EMGDamageComponent::Aero);
		break;

	case EMGDamageZone::Left:
	case EMGDamageZone::Right:
		AffectedComponents.Add(EMGDamageComponent::Suspension);
		AffectedComponents.Add(EMGDamageComponent::Wheels);
		break;

	case EMGDamageZone::Rear:
	case EMGDamageZone::RearLeft:
	case EMGDamageZone::RearRight:
		AffectedComponents.Add(EMGDamageComponent::Transmission);
		AffectedComponents.Add(EMGDamageComponent::NOS);
		AffectedComponents.Add(EMGDamageComponent::Aero);
		break;

	case EMGDamageZone::Top:
		AffectedComponents.Add(EMGDamageComponent::Aero);
		break;

	case EMGDamageZone::Bottom:
		AffectedComponents.Add(EMGDamageComponent::Suspension);
		AffectedComponents.Add(EMGDamageComponent::Brakes);
		break;
	}

	// Apply partial damage to affected components
	for (EMGDamageComponent Component : AffectedComponents)
	{
		ApplyComponentDamage(Component, Damage * 0.5f);
	}
}

void UMGVehicleDamageSystem::UpdateComponentPerformance(EMGDamageComponent Component)
{
	FMGComponentDamageState* State = ComponentStates.Find(Component);
	if (!State)
	{
		return;
	}

	// Performance scales with health
	// 100 health = 1.0 multiplier
	// 50 health = 0.75 multiplier
	// 0 health = 0.5 multiplier (or 0 if broken)

	if (State->bIsBroken)
	{
		State->PerformanceMultiplier = 0.25f; // Severely degraded but not completely non-functional
	}
	else
	{
		State->PerformanceMultiplier = 0.5f + (State->Health / 100.0f) * 0.5f;
	}
}

void UMGVehicleDamageSystem::ApplyPerformanceEffects()
{
	if (!MovementComponent.IsValid())
	{
		return;
	}

	// Engine affects max power and causes misfiring when damaged
	float EngineMult = GetComponentPerformance(EMGDamageComponent::Engine);
	MovementComponent->SetEngineDamageMultiplier(EngineMult);

	// Transmission affects acceleration and gear changes
	float TransMult = GetComponentPerformance(EMGDamageComponent::Transmission);
	MovementComponent->SetTransmissionDamageMultiplier(TransMult);

	// Suspension affects handling and grip
	float SuspMult = GetComponentPerformance(EMGDamageComponent::Suspension);
	MovementComponent->SetSuspensionDamageMultiplier(SuspMult);

	// Steering affects turn response
	float SteerMult = GetComponentPerformance(EMGDamageComponent::Steering);
	MovementComponent->SetSteeringDamageMultiplier(SteerMult);

	// Brakes affect braking power
	float BrakeMult = GetComponentPerformance(EMGDamageComponent::Brakes);
	MovementComponent->SetBrakeDamageMultiplier(BrakeMult);

	// Cooling damage affects engine efficiency (overheating)
	float CoolingMult = GetComponentPerformance(EMGDamageComponent::Cooling);
	if (CoolingMult < 0.5f)
	{
		// Overheating causes additional power loss
		float OverheatPenalty = 1.0f - ((0.5f - CoolingMult) * 0.5f);
		MovementComponent->SetEngineDamageMultiplier(EngineMult * OverheatPenalty);
	}

	// Wheels/tires damage affects grip
	float WheelMult = GetComponentPerformance(EMGDamageComponent::Wheels);
	MovementComponent->SetTireGripMultiplier(WheelMult);

	// Calculate overall max speed from combined damage
	// Severe damage to any critical component limits max speed
	float MinCriticalMult = FMath::Min3(EngineMult, TransMult, WheelMult);
	MovementComponent->SetMaxSpeedMultiplier(FMath::Max(0.5f, MinCriticalMult));
}

void UMGVehicleDamageSystem::UpdateVisualDamage(EMGDamageZone Zone, float Damage)
{
	// Update deformation
	float* Deformation = VisualDamage.ZoneDeformation.Find(Zone);
	if (Deformation)
	{
		*Deformation = FMath::Min(1.0f, *Deformation + (Damage / 100.0f) * DeformationMultiplier);
	}

	// Update scratches
	float* Scratches = VisualDamage.ZoneScratchDamage.Find(Zone);
	if (Scratches)
	{
		*Scratches = FMath::Min(1.0f, *Scratches + (Damage / 50.0f));
	}

	// Check for light damage
	if (Zone == EMGDamageZone::Front || Zone == EMGDamageZone::FrontLeft || Zone == EMGDamageZone::FrontRight)
	{
		if (Damage > 30.0f && FMath::RandRange(0.0f, 1.0f) < 0.5f)
		{
			VisualDamage.bHeadlightsBroken = true;
		}
	}
	if (Zone == EMGDamageZone::Rear || Zone == EMGDamageZone::RearLeft || Zone == EMGDamageZone::RearRight)
	{
		if (Damage > 30.0f && FMath::RandRange(0.0f, 1.0f) < 0.5f)
		{
			VisualDamage.bTaillightsBroken = true;
		}
	}

	// Window damage from high impacts
	if (Damage > 40.0f)
	{
		VisualDamage.WindowDamage = FMath::Min(1.0f, VisualDamage.WindowDamage + 0.2f);
	}

	OnVisualDamageUpdated.Broadcast(VisualDamage);
}

void UMGVehicleDamageSystem::CheckTotaledState()
{
	const FMGComponentDamageState* BodyState = ComponentStates.Find(EMGDamageComponent::Body);
	if (BodyState && BodyState->Health <= TotaledThreshold)
	{
		bIsTotaled = true;
		VisualDamage.bIsSmoking = true;

		// Chance to catch fire when totaled
		if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
		{
			VisualDamage.bIsOnFire = true;
		}

		OnVehicleTotaled.Broadcast();
	}
}

void UMGVehicleDamageSystem::UpdateAutoRepair(float MGDeltaTime)
{
	if (!bAutoRepairWhenStationary || !MovementComponent.IsValid())
	{
		return;
	}

	// Check if stationary
	float Speed = FMath::Abs(MovementComponent->GetForwardSpeed());
	bool bIsStationary = Speed < 1.0f; // Less than 1 cm/s

	if (bIsStationary)
	{
		if (bWasStationary)
		{
			StationaryTime += DeltaTime;
		}
		else
		{
			StationaryTime = 0.0f;
		}

		// Start auto-repair after delay
		if (StationaryTime >= AutoRepairDelay)
		{
			for (auto& Pair : ComponentStates)
			{
				if (Pair.Value.Health < 100.0f && !Pair.Value.bIsBroken)
				{
					Pair.Value.Health = FMath::Min(100.0f, Pair.Value.Health + AutoRepairRate * DeltaTime);
					UpdateComponentPerformance(Pair.Key);
				}
			}
			ApplyPerformanceEffects();
		}
	}
	else
	{
		StationaryTime = 0.0f;
	}

	bWasStationary = bIsStationary;
}

void UMGVehicleDamageSystem::UpdateRepairs(float MGDeltaTime)
{
	for (auto& Pair : ComponentStates)
	{
		if (Pair.Value.bIsRepairing)
		{
			Pair.Value.RepairProgress += DeltaTime * 0.5f; // 2 seconds to repair

			if (Pair.Value.RepairProgress >= 1.0f)
			{
				InstantRepair(Pair.Key);
			}
		}
	}
}

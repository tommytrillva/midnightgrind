// Copyright Midnight Grind. All Rights Reserved.

#include "Damage/MG_DMG_Subsystem.h"
#include "Kismet/GameplayStatics.h"

void UMGDamageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default config
	Config.bVisualDamageEnabled = true;
	Config.bMechanicalDamageEnabled = true;
	Config.DamageMultiplier = 1.0f;
	Config.MinImpactVelocityForDamage = 20.0f;
	Config.TotaledHealthThreshold = 10.0f;
	Config.CriticalHealthThreshold = 25.0f;
	Config.bAllowPartDetachment = true;
	Config.PartDetachmentThreshold = 75.0f;
	Config.bAllowFire = true;
	Config.FireIgnitionThreshold = 50.0f;
	Config.ComponentDamageSpreadFactor = 0.3f;
	Config.bAutoRepairOnRespawn = true;
}

void UMGDamageSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UMGDamageSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Damage Application
// ============================================================================

FMGDamageInstance UMGDamageSubsystem::ApplyDamage(FName VehicleID, const FMGDamageInstance& Damage)
{
	FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID);
	if (!State || State->bIsTotaled)
	{
		FMGDamageInstance BlockedDamage = Damage;
		BlockedDamage.bWasBlocked = true;
		return BlockedDamage;
	}

	FMGDamageInstance AppliedDamage = Damage;
	AppliedDamage.FinalDamage = CalculateFinalDamage(Damage);

	float OldHealth = State->OverallHealth;

	// Apply zone damage
	if (Damage.Zone != EMGDamageZone::None)
	{
		ApplyZoneDamage(VehicleID, Damage.Zone, AppliedDamage.FinalDamage);
	}

	// Spread damage to components
	if (Config.bMechanicalDamageEnabled)
	{
		SpreadComponentDamage(VehicleID, Damage.Zone, AppliedDamage.FinalDamage);
	}

	// Update overall state
	UpdateOverallState(VehicleID);

	// Check for fire ignition
	if (Config.bAllowFire)
	{
		CheckFireIgnition(VehicleID);
	}

	// Check for part detachment
	if (Config.bAllowPartDetachment)
	{
		CheckPartDetachment(VehicleID, Damage.Zone);
	}

	OnDamageReceived.Broadcast(VehicleID, AppliedDamage);

	if (State->OverallHealth != OldHealth)
	{
		OnHealthChanged.Broadcast(VehicleID, OldHealth, State->OverallHealth);
	}

	return AppliedDamage;
}

FMGDamageInstance UMGDamageSubsystem::ApplyCollisionDamage(FName VehicleID, const FVector& ImpactPoint, const FVector& ImpactNormal, float ImpactVelocity, FName InstigatorID)
{
	FMGDamageInstance Damage;

	if (ImpactVelocity < Config.MinImpactVelocityForDamage)
	{
		Damage.bWasBlocked = true;
		return Damage;
	}

	// Calculate impact zone based on impact point
	// Simplified - in production would use vehicle bounds
	Damage.Zone = CalculateImpactZone(ImpactPoint, FVector::ForwardVector, FVector::RightVector);
	Damage.DamageType = DetermineCollisionType(ImpactNormal, FVector::ForwardVector);
	Damage.ImpactPoint = ImpactPoint;
	Damage.ImpactNormal = ImpactNormal;
	Damage.ImpactVelocity = ImpactVelocity;
	Damage.InstigatorID = InstigatorID;

	// Calculate raw damage from impact velocity
	// Higher velocity = more damage, squared relationship
	float VelocityFactor = (ImpactVelocity - Config.MinImpactVelocityForDamage) / 100.0f;
	Damage.RawDamage = VelocityFactor * VelocityFactor * 10.0f;

	// Modify damage based on impact type
	switch (Damage.DamageType)
	{
	case EMGDamageType::FrontalImpact:
		Damage.RawDamage *= 1.2f;
		break;
	case EMGDamageType::TBone:
		Damage.RawDamage *= 1.5f;
		break;
	case EMGDamageType::RearEnd:
		Damage.RawDamage *= 0.8f;
		break;
	case EMGDamageType::SideSwipe:
		Damage.RawDamage *= 0.5f;
		break;
	case EMGDamageType::WallScrape:
		Damage.RawDamage *= 0.3f;
		break;
	default:
		break;
	}

	return ApplyDamage(VehicleID, Damage);
}

void UMGDamageSubsystem::ApplyZoneDamage(FName VehicleID, EMGDamageZone Zone, float DamageAmount)
{
	FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	FMGZoneDamageState& ZoneState = State->ZoneDamage.FindOrAdd(Zone);
	if (ZoneState.Zone == EMGDamageZone::None)
	{
		ZoneState.Zone = Zone;
		ZoneState.MaxDamage = 100.0f;
	}

	EMGDamageSeverity OldSeverity = ZoneState.Severity;

	ZoneState.CurrentDamage = FMath::Min(ZoneState.MaxDamage, ZoneState.CurrentDamage + DamageAmount);

	float DamagePercent = (ZoneState.CurrentDamage / ZoneState.MaxDamage) * 100.0f;
	ZoneState.Severity = CalculateSeverity(DamagePercent);
	ZoneState.bIsFunctional = DamagePercent < 90.0f;

	// Update deformation level
	ZoneState.DeformationLevel = FMath::Clamp(FMath::FloorToInt(DamagePercent / 20.0f), 0, 5);

	if (ZoneState.Severity != OldSeverity)
	{
		OnZoneDamaged.Broadcast(Zone, ZoneState.Severity);
	}
}

void UMGDamageSubsystem::ApplyComponentDamage(FName VehicleID, EMGVehicleComponent Component, float DamageAmount)
{
	FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID);
	if (!State || !Config.bMechanicalDamageEnabled)
	{
		return;
	}

	FMGComponentDamageState& CompState = State->ComponentDamage.FindOrAdd(Component);
	if (CompState.Component == EMGVehicleComponent::None)
	{
		CompState.Component = Component;
		CompState.MaxHealth = 100.0f;
		CompState.Health = 100.0f;
	}

	bool bWasFunctional = CompState.bIsFunctional;

	CompState.Health = FMath::Max(0.0f, CompState.Health - DamageAmount);

	// Calculate efficiency based on remaining health
	float HealthPercent = CompState.Health / CompState.MaxHealth;
	CompState.EfficiencyMultiplier = FMath::Lerp(0.2f, 1.0f, HealthPercent);
	CompState.bIsFunctional = CompState.Health > 10.0f;
	CompState.bIsDisabled = CompState.Health <= 0.0f;

	// Calculate repair cost
	float DamagePercent = 1.0f - HealthPercent;
	CompState.RepairCost = FMath::RoundToInt(DamagePercent * 500.0f);

	OnComponentDamaged.Broadcast(Component, CompState.Health);

	if (bWasFunctional && !CompState.bIsFunctional)
	{
		OnComponentDisabled.Broadcast(Component);
	}
}

void UMGDamageSubsystem::ApplyEnvironmentalDamage(FName VehicleID, EMGDamageType DamageType, float DamageAmount)
{
	FMGDamageInstance Damage;
	Damage.DamageType = DamageType;
	Damage.RawDamage = DamageAmount;

	switch (DamageType)
	{
	case EMGDamageType::SpikeTrap:
		// Spike traps damage tires specifically
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::Tires, DamageAmount * 2.0f);
		break;

	case EMGDamageType::EMP:
		// EMP damages electronics
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::Electronics, DamageAmount);
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::NitroSystem, DamageAmount * 0.5f);
		break;

	default:
		ApplyDamage(VehicleID, Damage);
		break;
	}
}

// ============================================================================
// State Queries
// ============================================================================

FMGVehicleDamageState UMGDamageSubsystem::GetVehicleDamageState(FName VehicleID) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		return *State;
	}

	return FMGVehicleDamageState();
}

float UMGDamageSubsystem::GetVehicleHealth(FName VehicleID) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		return State->OverallHealth;
	}

	return 100.0f;
}

float UMGDamageSubsystem::GetVehicleHealthPercent(FName VehicleID) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		return (State->OverallHealth / State->MaxHealth) * 100.0f;
	}

	return 100.0f;
}

EMGDamageSeverity UMGDamageSubsystem::GetVehicleSeverity(FName VehicleID) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		return State->OverallSeverity;
	}

	return EMGDamageSeverity::None;
}

bool UMGDamageSubsystem::IsVehicleDriveable(FName VehicleID) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		return State->bIsDriveable;
	}

	return true;
}

bool UMGDamageSubsystem::IsVehicleTotaled(FName VehicleID) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		return State->bIsTotaled;
	}

	return false;
}

// ============================================================================
// Zone Queries
// ============================================================================

FMGZoneDamageState UMGDamageSubsystem::GetZoneDamageState(FName VehicleID, EMGDamageZone Zone) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		if (const FMGZoneDamageState* ZoneState = State->ZoneDamage.Find(Zone))
		{
			return *ZoneState;
		}
	}

	return FMGZoneDamageState();
}

float UMGDamageSubsystem::GetZoneDamagePercent(FName VehicleID, EMGDamageZone Zone) const
{
	FMGZoneDamageState ZoneState = GetZoneDamageState(VehicleID, Zone);
	if (ZoneState.MaxDamage > 0)
	{
		return (ZoneState.CurrentDamage / ZoneState.MaxDamage) * 100.0f;
	}

	return 0.0f;
}

EMGDamageZone UMGDamageSubsystem::GetMostDamagedZone(FName VehicleID) const
{
	EMGDamageZone MostDamaged = EMGDamageZone::None;
	float HighestDamage = 0.0f;

	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		for (const auto& ZonePair : State->ZoneDamage)
		{
			if (ZonePair.Value.CurrentDamage > HighestDamage)
			{
				HighestDamage = ZonePair.Value.CurrentDamage;
				MostDamaged = ZonePair.Key;
			}
		}
	}

	return MostDamaged;
}

TArray<FName> UMGDamageSubsystem::GetDetachedParts(FName VehicleID) const
{
	TArray<FName> AllDetached;

	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		for (const auto& ZonePair : State->ZoneDamage)
		{
			AllDetached.Append(ZonePair.Value.DetachedParts);
		}
	}

	return AllDetached;
}

// ============================================================================
// Component Queries
// ============================================================================

FMGComponentDamageState UMGDamageSubsystem::GetComponentDamageState(FName VehicleID, EMGVehicleComponent Component) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		if (const FMGComponentDamageState* CompState = State->ComponentDamage.Find(Component))
		{
			return *CompState;
		}
	}

	return FMGComponentDamageState();
}

float UMGDamageSubsystem::GetComponentEfficiency(FName VehicleID, EMGVehicleComponent Component) const
{
	FMGComponentDamageState CompState = GetComponentDamageState(VehicleID, Component);
	return CompState.EfficiencyMultiplier;
}

bool UMGDamageSubsystem::IsComponentFunctional(FName VehicleID, EMGVehicleComponent Component) const
{
	FMGComponentDamageState CompState = GetComponentDamageState(VehicleID, Component);
	return CompState.bIsFunctional;
}

TArray<EMGVehicleComponent> UMGDamageSubsystem::GetDisabledComponents(FName VehicleID) const
{
	TArray<EMGVehicleComponent> Disabled;

	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		for (const auto& CompPair : State->ComponentDamage)
		{
			if (CompPair.Value.bIsDisabled)
			{
				Disabled.Add(CompPair.Key);
			}
		}
	}

	return Disabled;
}

// ============================================================================
// Performance Impact
// ============================================================================

float UMGDamageSubsystem::GetEnginePerformanceMultiplier(FName VehicleID) const
{
	float EngineEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Engine);
	float RadiatorEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Radiator);

	return EngineEff * FMath::Lerp(1.0f, RadiatorEff, 0.3f);
}

float UMGDamageSubsystem::GetHandlingMultiplier(FName VehicleID) const
{
	float SuspensionEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Suspension);
	float SteeringEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Steering);
	float TiresEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Tires);

	return (SuspensionEff + SteeringEff + TiresEff) / 3.0f;
}

float UMGDamageSubsystem::GetTopSpeedMultiplier(FName VehicleID) const
{
	float EngineEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Engine);
	float TransmissionEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Transmission);

	return EngineEff * TransmissionEff;
}

float UMGDamageSubsystem::GetAccelerationMultiplier(FName VehicleID) const
{
	float EngineEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Engine);
	float TransmissionEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Transmission);
	float TiresEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Tires);

	return (EngineEff * 0.5f + TransmissionEff * 0.3f + TiresEff * 0.2f);
}

float UMGDamageSubsystem::GetBrakingMultiplier(FName VehicleID) const
{
	float BrakesEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Brakes);
	float TiresEff = GetComponentEfficiency(VehicleID, EMGVehicleComponent::Tires);

	return (BrakesEff * 0.7f + TiresEff * 0.3f);
}

float UMGDamageSubsystem::GetNitroEfficiency(FName VehicleID) const
{
	return GetComponentEfficiency(VehicleID, EMGVehicleComponent::NitroSystem);
}

// ============================================================================
// Repair
// ============================================================================

int32 UMGDamageSubsystem::RepairVehicle(FName VehicleID, const FMGRepairOptions& Options)
{
	FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID);
	if (!State)
	{
		return 0;
	}

	int32 TotalCost = 0;
	float HealthRestored = 0.0f;

	// Repair zones
	if (Options.bRepairVisual)
	{
		for (auto& ZonePair : State->ZoneDamage)
		{
			if (!Options.bRepairAllZones && !Options.SpecificZones.Contains(ZonePair.Key))
			{
				continue;
			}

			float RepairAmount = ZonePair.Value.CurrentDamage * (Options.RepairPercentage / 100.0f);
			int32 ZoneCost = CalculateRepairCost(RepairAmount, ZonePair.Value.Severity);

			ZonePair.Value.CurrentDamage -= RepairAmount;
			ZonePair.Value.Severity = CalculateSeverity((ZonePair.Value.CurrentDamage / ZonePair.Value.MaxDamage) * 100.0f);
			ZonePair.Value.bIsFunctional = true;
			ZonePair.Value.DetachedParts.Empty();
			ZonePair.Value.DeformationLevel = FMath::FloorToInt((ZonePair.Value.CurrentDamage / ZonePair.Value.MaxDamage) * 5.0f);

			TotalCost += ZoneCost;
			HealthRestored += RepairAmount;
		}
	}

	// Repair components
	if (Options.bRepairMechanical)
	{
		for (auto& CompPair : State->ComponentDamage)
		{
			if (!Options.bRepairAllComponents && !Options.SpecificComponents.Contains(CompPair.Key))
			{
				continue;
			}

			float DamageAmount = CompPair.Value.MaxHealth - CompPair.Value.Health;
			float RepairAmount = DamageAmount * (Options.RepairPercentage / 100.0f);

			TotalCost += FMath::RoundToInt(CompPair.Value.RepairCost * (Options.RepairPercentage / 100.0f));

			CompPair.Value.Health += RepairAmount;
			CompPair.Value.EfficiencyMultiplier = CompPair.Value.Health / CompPair.Value.MaxHealth;
			CompPair.Value.bIsFunctional = true;
			CompPair.Value.bIsDisabled = false;
			CompPair.Value.RepairCost = CalculateRepairCost(CompPair.Value.MaxHealth - CompPair.Value.Health, EMGDamageSeverity::Light);
		}
	}

	// Update overall state
	State->bOnFire = false;
	State->SmokeLevel = 0.0f;
	State->bLeakingFuel = false;
	State->bLeakingOil = false;

	UpdateOverallState(VehicleID);

	if (HealthRestored > 0)
	{
		OnVehicleRepaired.Broadcast(VehicleID, HealthRestored);
	}

	return TotalCost;
}

int32 UMGDamageSubsystem::RepairVehicleFull(FName VehicleID)
{
	FMGRepairOptions Options;
	Options.bRepairVisual = true;
	Options.bRepairMechanical = true;
	Options.bRepairAllZones = true;
	Options.bRepairAllComponents = true;
	Options.RepairPercentage = 100.0f;

	return RepairVehicle(VehicleID, Options);
}

int32 UMGDamageSubsystem::RepairZone(FName VehicleID, EMGDamageZone Zone)
{
	FMGRepairOptions Options;
	Options.bRepairVisual = true;
	Options.bRepairMechanical = false;
	Options.bRepairAllZones = false;
	Options.SpecificZones.Add(Zone);
	Options.RepairPercentage = 100.0f;

	return RepairVehicle(VehicleID, Options);
}

int32 UMGDamageSubsystem::RepairComponent(FName VehicleID, EMGVehicleComponent Component)
{
	FMGRepairOptions Options;
	Options.bRepairVisual = false;
	Options.bRepairMechanical = true;
	Options.bRepairAllComponents = false;
	Options.SpecificComponents.Add(Component);
	Options.RepairPercentage = 100.0f;

	return RepairVehicle(VehicleID, Options);
}

int32 UMGDamageSubsystem::GetRepairCost(FName VehicleID) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		return State->TotalRepairCost;
	}

	return 0;
}

int32 UMGDamageSubsystem::GetZoneRepairCost(FName VehicleID, EMGDamageZone Zone) const
{
	FMGZoneDamageState ZoneState = GetZoneDamageState(VehicleID, Zone);
	return CalculateRepairCost(ZoneState.CurrentDamage, ZoneState.Severity);
}

int32 UMGDamageSubsystem::GetComponentRepairCost(FName VehicleID, EMGVehicleComponent Component) const
{
	FMGComponentDamageState CompState = GetComponentDamageState(VehicleID, Component);
	return FMath::RoundToInt(CompState.RepairCost);
}

// ============================================================================
// Vehicle Registration
// ============================================================================

void UMGDamageSubsystem::RegisterVehicle(FName VehicleID, float MaxHealth)
{
	if (VehicleDamageStates.Contains(VehicleID))
	{
		return;
	}

	FMGVehicleDamageState NewState;
	InitializeVehicleState(NewState, MaxHealth);
	NewState.VehicleID = VehicleID;

	VehicleDamageStates.Add(VehicleID, NewState);
}

void UMGDamageSubsystem::UnregisterVehicle(FName VehicleID)
{
	VehicleDamageStates.Remove(VehicleID);
	VehicleDeformation.Remove(VehicleID);
}

void UMGDamageSubsystem::ResetVehicleDamage(FName VehicleID)
{
	if (FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		float MaxHealth = State->MaxHealth;
		InitializeVehicleState(*State, MaxHealth);
		State->VehicleID = VehicleID;
	}

	VehicleDeformation.Remove(VehicleID);
}

// ============================================================================
// Deformation
// ============================================================================

FMGDeformationData UMGDamageSubsystem::GetDeformationData(FName VehicleID, EMGDamageZone Zone) const
{
	if (const auto* VehicleDeform = VehicleDeformation.Find(VehicleID))
	{
		if (const FMGDeformationData* DeformData = VehicleDeform->Find(Zone))
		{
			return *DeformData;
		}
	}

	return FMGDeformationData();
}

void UMGDamageSubsystem::AddDeformation(FName VehicleID, EMGDamageZone Zone, const FVector& ImpactPoint, float Depth)
{
	TMap<EMGDamageZone, FMGDeformationData>& VehicleDeform = VehicleDeformation.FindOrAdd(VehicleID);
	FMGDeformationData& DeformData = VehicleDeform.FindOrAdd(Zone);

	DeformData.DeformationCenter = ImpactPoint;
	DeformData.MaxDeformationDepth = FMath::Max(DeformData.MaxDeformationDepth, Depth);
}

// ============================================================================
// Fire
// ============================================================================

void UMGDamageSubsystem::IgniteVehicle(FName VehicleID)
{
	if (FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		if (!State->bOnFire && Config.bAllowFire)
		{
			State->bOnFire = true;
			State->SmokeLevel = 1.0f;
			OnVehicleOnFire.Broadcast(VehicleID);
		}
	}
}

void UMGDamageSubsystem::ExtinguishVehicle(FName VehicleID)
{
	if (FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		State->bOnFire = false;
	}
}

bool UMGDamageSubsystem::IsVehicleOnFire(FName VehicleID) const
{
	if (const FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID))
	{
		return State->bOnFire;
	}

	return false;
}

// ============================================================================
// Configuration
// ============================================================================

void UMGDamageSubsystem::SetDamageConfig(const FMGDamageConfig& NewConfig)
{
	Config = NewConfig;
}

// ============================================================================
// Protected Helpers
// ============================================================================

EMGDamageZone UMGDamageSubsystem::CalculateImpactZone(const FVector& ImpactPoint, const FVector& VehicleForward, const FVector& VehicleRight) const
{
	// Simplified zone calculation based on impact point relative to vehicle center
	float ForwardDot = FVector::DotProduct(ImpactPoint.GetSafeNormal(), VehicleForward);
	float RightDot = FVector::DotProduct(ImpactPoint.GetSafeNormal(), VehicleRight);

	if (ForwardDot > 0.5f)
	{
		// Front
		if (RightDot > 0.3f) return EMGDamageZone::FrontRight;
		if (RightDot < -0.3f) return EMGDamageZone::FrontLeft;
		return EMGDamageZone::FrontCenter;
	}
	else if (ForwardDot < -0.5f)
	{
		// Rear
		if (RightDot > 0.3f) return EMGDamageZone::RearRight;
		if (RightDot < -0.3f) return EMGDamageZone::RearLeft;
		return EMGDamageZone::RearCenter;
	}
	else
	{
		// Side
		return (RightDot > 0) ? EMGDamageZone::SideRight : EMGDamageZone::SideLeft;
	}
}

EMGDamageType UMGDamageSubsystem::DetermineCollisionType(const FVector& ImpactNormal, const FVector& VehicleForward) const
{
	float ForwardDot = FMath::Abs(FVector::DotProduct(ImpactNormal, VehicleForward));
	float RightDot = FMath::Abs(FVector::DotProduct(ImpactNormal, FVector::RightVector));

	if (ForwardDot > 0.7f)
	{
		// Frontal or rear
		return (FVector::DotProduct(ImpactNormal, VehicleForward) > 0) ? EMGDamageType::FrontalImpact : EMGDamageType::RearEnd;
	}
	else if (RightDot > 0.7f)
	{
		return EMGDamageType::TBone;
	}
	else
	{
		return EMGDamageType::SideSwipe;
	}
}

EMGDamageSeverity UMGDamageSubsystem::CalculateSeverity(float DamagePercent) const
{
	if (DamagePercent >= 90.0f) return EMGDamageSeverity::Totaled;
	if (DamagePercent >= 75.0f) return EMGDamageSeverity::Critical;
	if (DamagePercent >= 50.0f) return EMGDamageSeverity::Heavy;
	if (DamagePercent >= 25.0f) return EMGDamageSeverity::Moderate;
	if (DamagePercent >= 10.0f) return EMGDamageSeverity::Light;
	if (DamagePercent > 0.0f) return EMGDamageSeverity::Cosmetic;

	return EMGDamageSeverity::None;
}

float UMGDamageSubsystem::CalculateFinalDamage(const FMGDamageInstance& Damage) const
{
	return Damage.RawDamage * Config.DamageMultiplier;
}

void UMGDamageSubsystem::SpreadComponentDamage(FName VehicleID, EMGDamageZone Zone, float DamageAmount)
{
	float SpreadDamage = DamageAmount * Config.ComponentDamageSpreadFactor;

	// Map zones to most affected components
	switch (Zone)
	{
	case EMGDamageZone::FrontCenter:
	case EMGDamageZone::FrontLeft:
	case EMGDamageZone::FrontRight:
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::Radiator, SpreadDamage);
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::Engine, SpreadDamage * 0.5f);
		break;

	case EMGDamageZone::RearCenter:
	case EMGDamageZone::RearLeft:
	case EMGDamageZone::RearRight:
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::FuelTank, SpreadDamage);
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::Exhaust, SpreadDamage * 0.7f);
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::NitroSystem, SpreadDamage * 0.5f);
		break;

	case EMGDamageZone::SideLeft:
	case EMGDamageZone::SideRight:
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::Suspension, SpreadDamage);
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::Tires, SpreadDamage * 0.7f);
		break;

	case EMGDamageZone::Underbody:
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::Transmission, SpreadDamage);
		ApplyComponentDamage(VehicleID, EMGVehicleComponent::FuelTank, SpreadDamage * 0.5f);
		break;

	default:
		break;
	}
}

void UMGDamageSubsystem::CheckPartDetachment(FName VehicleID, EMGDamageZone Zone)
{
	FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	FMGZoneDamageState* ZoneState = State->ZoneDamage.Find(Zone);
	if (!ZoneState)
	{
		return;
	}

	float DamagePercent = (ZoneState->CurrentDamage / ZoneState->MaxDamage) * 100.0f;

	if (DamagePercent >= Config.PartDetachmentThreshold)
	{
		// Determine which parts can detach based on zone
		TArray<FName> PotentialParts;

		switch (Zone)
		{
		case EMGDamageZone::FrontCenter:
			PotentialParts.Add(TEXT("Hood"));
			PotentialParts.Add(TEXT("Bumper_Front"));
			break;
		case EMGDamageZone::FrontLeft:
		case EMGDamageZone::FrontRight:
			PotentialParts.Add(TEXT("Headlight"));
			PotentialParts.Add(TEXT("Fender"));
			break;
		case EMGDamageZone::RearCenter:
			PotentialParts.Add(TEXT("Trunk"));
			PotentialParts.Add(TEXT("Bumper_Rear"));
			PotentialParts.Add(TEXT("Spoiler"));
			break;
		case EMGDamageZone::SideLeft:
		case EMGDamageZone::SideRight:
			PotentialParts.Add(TEXT("Mirror"));
			PotentialParts.Add(TEXT("Door"));
			break;
		default:
			break;
		}

		// Randomly detach parts based on damage severity
		for (const FName& Part : PotentialParts)
		{
			if (!ZoneState->DetachedParts.Contains(Part))
			{
				float DetachChance = (DamagePercent - Config.PartDetachmentThreshold) / (100.0f - Config.PartDetachmentThreshold);
				if (FMath::FRand() < DetachChance)
				{
					ZoneState->DetachedParts.Add(Part);
					OnPartDetached.Broadcast(VehicleID, Part);
				}
			}
		}
	}
}

void UMGDamageSubsystem::CheckFireIgnition(FName VehicleID)
{
	FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID);
	if (!State || State->bOnFire)
	{
		return;
	}

	// Check engine and fuel tank damage
	FMGComponentDamageState EngineState = GetComponentDamageState(VehicleID, EMGVehicleComponent::Engine);
	FMGComponentDamageState FuelTankState = GetComponentDamageState(VehicleID, EMGVehicleComponent::FuelTank);

	float EngineDamagePercent = 100.0f - ((EngineState.Health / EngineState.MaxHealth) * 100.0f);
	float FuelDamagePercent = 100.0f - ((FuelTankState.Health / FuelTankState.MaxHealth) * 100.0f);

	if (EngineDamagePercent >= Config.FireIgnitionThreshold || FuelDamagePercent >= Config.FireIgnitionThreshold)
	{
		float IgnitionChance = FMath::Max(EngineDamagePercent, FuelDamagePercent) / 100.0f * 0.3f;
		if (FMath::FRand() < IgnitionChance)
		{
			IgniteVehicle(VehicleID);
		}
	}

	// Update smoke and leak status
	State->SmokeLevel = FMath::Max(EngineDamagePercent / 100.0f, State->SmokeLevel);
	State->bLeakingFuel = FuelDamagePercent >= 50.0f;
	State->bLeakingOil = EngineDamagePercent >= 40.0f;
}

void UMGDamageSubsystem::UpdateOverallState(FName VehicleID)
{
	FMGVehicleDamageState* State = VehicleDamageStates.Find(VehicleID);
	if (!State)
	{
		return;
	}

	// Calculate overall health from zone damage
	float TotalDamage = 0.0f;
	float TotalMaxDamage = 0.0f;

	for (const auto& ZonePair : State->ZoneDamage)
	{
		TotalDamage += ZonePair.Value.CurrentDamage;
		TotalMaxDamage += ZonePair.Value.MaxDamage;
	}

	if (TotalMaxDamage > 0)
	{
		float DamagePercent = (TotalDamage / TotalMaxDamage) * 100.0f;
		State->OverallHealth = State->MaxHealth * (1.0f - DamagePercent / 100.0f);
		State->OverallSeverity = CalculateSeverity(DamagePercent);
	}

	// Calculate total repair cost
	State->TotalRepairCost = 0;
	for (const auto& ZonePair : State->ZoneDamage)
	{
		State->TotalRepairCost += CalculateRepairCost(ZonePair.Value.CurrentDamage, ZonePair.Value.Severity);
	}
	for (const auto& CompPair : State->ComponentDamage)
	{
		State->TotalRepairCost += FMath::RoundToInt(CompPair.Value.RepairCost);
	}

	// Check driveability
	bool bEngineWorks = IsComponentFunctional(VehicleID, EMGVehicleComponent::Engine);
	bool bTransWorks = IsComponentFunctional(VehicleID, EMGVehicleComponent::Transmission);
	bool bTiresWork = IsComponentFunctional(VehicleID, EMGVehicleComponent::Tires);

	State->bIsDriveable = bEngineWorks && bTransWorks && bTiresWork && !State->bOnFire;

	// Check if totaled
	float HealthPercent = (State->OverallHealth / State->MaxHealth) * 100.0f;
	bool bWasTotaled = State->bIsTotaled;
	State->bIsTotaled = HealthPercent <= Config.TotaledHealthThreshold;

	if (State->bIsTotaled && !bWasTotaled)
	{
		State->bIsDriveable = false;
		OnVehicleTotaled.Broadcast(VehicleID);
	}
}

int32 UMGDamageSubsystem::CalculateRepairCost(float DamageAmount, EMGDamageSeverity Severity) const
{
	float BaseCost = DamageAmount * 10.0f;

	float SeverityMultiplier = 1.0f;
	switch (Severity)
	{
	case EMGDamageSeverity::Cosmetic:
		SeverityMultiplier = 0.5f;
		break;
	case EMGDamageSeverity::Light:
		SeverityMultiplier = 1.0f;
		break;
	case EMGDamageSeverity::Moderate:
		SeverityMultiplier = 1.5f;
		break;
	case EMGDamageSeverity::Heavy:
		SeverityMultiplier = 2.0f;
		break;
	case EMGDamageSeverity::Critical:
		SeverityMultiplier = 3.0f;
		break;
	case EMGDamageSeverity::Totaled:
		SeverityMultiplier = 5.0f;
		break;
	default:
		break;
	}

	return FMath::RoundToInt(BaseCost * SeverityMultiplier);
}

void UMGDamageSubsystem::InitializeVehicleState(FMGVehicleDamageState& State, float MaxHealth)
{
	State = FMGVehicleDamageState();
	State.MaxHealth = MaxHealth;
	State.OverallHealth = MaxHealth;
	State.bIsDriveable = true;
	State.bIsTotaled = false;

	// Initialize all zones
	TArray<EMGDamageZone> AllZones = {
		EMGDamageZone::FrontLeft, EMGDamageZone::FrontCenter, EMGDamageZone::FrontRight,
		EMGDamageZone::SideLeft, EMGDamageZone::SideRight,
		EMGDamageZone::RearLeft, EMGDamageZone::RearCenter, EMGDamageZone::RearRight,
		EMGDamageZone::Roof, EMGDamageZone::Underbody
	};

	for (EMGDamageZone Zone : AllZones)
	{
		FMGZoneDamageState ZoneState;
		ZoneState.Zone = Zone;
		ZoneState.MaxDamage = 100.0f;
		ZoneState.CurrentDamage = 0.0f;
		ZoneState.bIsFunctional = true;
		State.ZoneDamage.Add(Zone, ZoneState);
	}

	// Initialize all components
	TArray<EMGVehicleComponent> AllComponents = {
		EMGVehicleComponent::Engine, EMGVehicleComponent::Transmission,
		EMGVehicleComponent::Suspension, EMGVehicleComponent::Steering,
		EMGVehicleComponent::Brakes, EMGVehicleComponent::Tires,
		EMGVehicleComponent::Exhaust, EMGVehicleComponent::NitroSystem,
		EMGVehicleComponent::Radiator, EMGVehicleComponent::FuelTank,
		EMGVehicleComponent::Electronics
	};

	for (EMGVehicleComponent Comp : AllComponents)
	{
		FMGComponentDamageState CompState;
		CompState.Component = Comp;
		CompState.MaxHealth = 100.0f;
		CompState.Health = 100.0f;
		CompState.EfficiencyMultiplier = 1.0f;
		CompState.bIsFunctional = true;
		CompState.bIsDisabled = false;
		State.ComponentDamage.Add(Comp, CompState);
	}
}

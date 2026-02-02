// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCollisionSubsystem.cpp
 * @brief Implementation of the Collision Subsystem for vehicle damage,
 *        collision processing, takedowns, and crash effects.
 */

#include "Collision/MGCollisionSubsystem.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Haptics/MGHapticsSubsystem.h"
#include "ScreenEffect/MGScreenEffectSubsystem.h"

void UMGCollisionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CollisionCounter = 0;
	TakedownCounter = 0;

	// Set up default physics config
	PhysicsConfig.MassInfluence = 1.0f;
	PhysicsConfig.SpeedInfluence = 1.5f;
	PhysicsConfig.AngleInfluence = 1.2f;
	PhysicsConfig.RestitutionCoefficient = 0.3f;
	PhysicsConfig.FrictionCoefficient = 0.8f;
	PhysicsConfig.MinSpeedForDamage = 20.0f;
	PhysicsConfig.DamagePerMPH = 0.5f;
	PhysicsConfig.SpinImpulseMultiplier = 1.0f;
	PhysicsConfig.BounceMultiplier = 1.0f;
	PhysicsConfig.SpeedLossPercentMin = 0.1f;
	PhysicsConfig.SpeedLossPercentMax = 0.5f;
	PhysicsConfig.InvincibilityAfterCollision = 0.5f;

	// Set up default scoring config
	ScoringConfig.BasePointsPerTakedown = 500;
	ScoringConfig.RevengeBonus = 250;
	ScoringConfig.DriftTakedownBonus = 300;
	ScoringConfig.AirborneTakedownBonus = 400;
	ScoringConfig.ChainMultiplierPerTakedown = 0.5f;
	ScoringConfig.MaxChainMultiplier = 5.0f;
	ScoringConfig.ChainWindowSeconds = 5.0f;

	// Type bonus points
	ScoringConfig.TypeBonusPoints.Add(EMGCollisionType::Sideswipe, 100);
	ScoringConfig.TypeBonusPoints.Add(EMGCollisionType::TBone, 200);
	ScoringConfig.TypeBonusPoints.Add(EMGCollisionType::HeadOn, 300);
	ScoringConfig.TypeBonusPoints.Add(EMGCollisionType::RearEnd, 150);
	ScoringConfig.TypeBonusPoints.Add(EMGCollisionType::Rollover, 500);

	// Severity multipliers
	ScoringConfig.SeverityMultipliers.Add(EMGCollisionSeverity::Glancing, 0.5f);
	ScoringConfig.SeverityMultipliers.Add(EMGCollisionSeverity::Minor, 1.0f);
	ScoringConfig.SeverityMultipliers.Add(EMGCollisionSeverity::Moderate, 1.5f);
	ScoringConfig.SeverityMultipliers.Add(EMGCollisionSeverity::Major, 2.0f);
	ScoringConfig.SeverityMultipliers.Add(EMGCollisionSeverity::Severe, 3.0f);
	ScoringConfig.SeverityMultipliers.Add(EMGCollisionSeverity::Catastrophic, 5.0f);

	// Register default crash effects
	FMGCrashEffect MinorEffect;
	MinorEffect.MinSeverity = EMGCollisionSeverity::Minor;
	MinorEffect.CameraShakeIntensity = 0.2f;
	MinorEffect.bTriggerRumble = true;
	MinorEffect.RumbleIntensity = 0.3f;
	MinorEffect.RumbleDuration = 0.2f;
	CrashEffects.Add(MinorEffect);

	FMGCrashEffect ModerateEffect;
	ModerateEffect.MinSeverity = EMGCollisionSeverity::Moderate;
	ModerateEffect.CameraShakeIntensity = 0.5f;
	ModerateEffect.bTriggerRumble = true;
	ModerateEffect.RumbleIntensity = 0.6f;
	ModerateEffect.RumbleDuration = 0.3f;
	CrashEffects.Add(ModerateEffect);

	FMGCrashEffect SevereEffect;
	SevereEffect.MinSeverity = EMGCollisionSeverity::Severe;
	SevereEffect.CameraShakeIntensity = 1.0f;
	SevereEffect.SlowMotionDuration = 0.5f;
	SevereEffect.SlowMotionScale = 0.3f;
	SevereEffect.bTriggerRumble = true;
	SevereEffect.RumbleIntensity = 1.0f;
	SevereEffect.RumbleDuration = 0.5f;
	CrashEffects.Add(SevereEffect);

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGCollisionSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			CollisionTickTimer,
			[WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->TickCollision(0.033f);
				}
			},
			0.033f,
			true
		);
	}

	LoadCollisionData();
}

void UMGCollisionSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CollisionTickTimer);
	}

	SaveCollisionData();
	Super::Deinitialize();
}

// ============================================================================
// Vehicle Registration
// ============================================================================

void UMGCollisionSubsystem::RegisterVehicle(const FString& VehicleId, float MaxHealth)
{
	if (VehicleId.IsEmpty())
	{
		return;
	}

	FMGVehicleCollisionState State;
	State.VehicleId = VehicleId;
	State.TotalHealth = MaxHealth;
	State.MaxHealth = MaxHealth;
	State.OverallState = EMGDamageState::Pristine;

	// Initialize damage zones
	TArray<EMGImpactZone> Zones = {
		EMGImpactZone::FrontCenter, EMGImpactZone::FrontLeft, EMGImpactZone::FrontRight,
		EMGImpactZone::SideLeft, EMGImpactZone::SideRight,
		EMGImpactZone::RearCenter, EMGImpactZone::RearLeft, EMGImpactZone::RearRight,
		EMGImpactZone::Roof, EMGImpactZone::Undercarriage
	};

	for (EMGImpactZone Zone : Zones)
	{
		FMGDamageZone DamageZone;
		DamageZone.Zone = Zone;
		DamageZone.CurrentHealth = 100.0f;
		DamageZone.MaxHealth = 100.0f;
		DamageZone.DamageMultiplier = 1.0f;
		DamageZone.State = EMGDamageState::Pristine;
		State.DamageZones.Add(Zone, DamageZone);
	}

	// Initialize tire status (4 tires)
	State.TireStatus = {true, true, true, true};

	VehicleStates.Add(VehicleId, State);
}

void UMGCollisionSubsystem::UnregisterVehicle(const FString& VehicleId)
{
	VehicleStates.Remove(VehicleId);
	InvincibilityTimers.Remove(VehicleId);
}

FMGVehicleCollisionState UMGCollisionSubsystem::GetVehicleState(const FString& VehicleId) const
{
	if (const FMGVehicleCollisionState* Found = VehicleStates.Find(VehicleId))
	{
		return *Found;
	}
	return FMGVehicleCollisionState();
}

bool UMGCollisionSubsystem::IsVehicleRegistered(const FString& VehicleId) const
{
	return VehicleStates.Contains(VehicleId);
}

// ============================================================================
// Collision Processing
// ============================================================================

FMGCollisionEvent UMGCollisionSubsystem::ProcessCollision(const FString& PlayerId, const FString& OtherEntityId, FVector ImpactLocation, FVector ImpactNormal, FVector ImpactVelocity, EMGCollisionType Type)
{
	FMGCollisionEvent Event;

	// Check for invincibility
	if (IsInvincible(PlayerId))
	{
		return Event;
	}

	float ImpactSpeed = ImpactVelocity.Size() / 44.704f; // Convert to MPH

	// Check minimum speed for damage
	if (ImpactSpeed < PhysicsConfig.MinSpeedForDamage)
	{
		return Event;
	}

	// Calculate severity
	float ImpactForce = ImpactVelocity.Size() * PhysicsConfig.MassInfluence;
	EMGCollisionSeverity Severity = CalculateSeverity(ImpactSpeed, ImpactForce);

	// Determine impact zone
	FMGVehicleCollisionState* VehicleState = VehicleStates.Find(PlayerId);
	EMGImpactZone ImpactZone = EMGImpactZone::FrontCenter;
	if (VehicleState)
	{
		// Simple impact zone calculation based on impact normal
		ImpactZone = DetermineImpactZone(ImpactNormal);
	}

	// Calculate damage
	float Damage = ImpactSpeed * PhysicsConfig.DamagePerMPH;

	// Apply damage
	if (VehicleState)
	{
		ApplyDamage(PlayerId, Damage, ImpactZone);
	}

	// Calculate speed loss
	float SpeedLossPercent = FMath::Lerp(
		PhysicsConfig.SpeedLossPercentMin,
		PhysicsConfig.SpeedLossPercentMax,
		static_cast<float>(Severity) / static_cast<float>(EMGCollisionSeverity::Catastrophic)
	);

	// Create event
	Event.CollisionId = GenerateCollisionId();
	Event.PlayerId = PlayerId;
	Event.OtherEntityId = OtherEntityId;
	Event.Type = Type;
	Event.Severity = Severity;
	Event.ImpactZone = ImpactZone;
	Event.ImpactLocation = ImpactLocation;
	Event.ImpactNormal = ImpactNormal;
	Event.ImpactVelocity = ImpactVelocity;
	Event.ImpactSpeed = ImpactSpeed;
	Event.ImpactForce = ImpactForce;
	Event.DamageDealt = Damage;
	Event.SpeedLoss = SpeedLossPercent;
	Event.SpinImpulse = CalculateSpinImpulse(Event);
	Event.Timestamp = FDateTime::Now();

	// Update player stats
	UpdatePlayerStats(PlayerId, Event);

	// Store recent collision
	TArray<FMGCollisionEvent>& Recent = RecentCollisions.FindOrAdd(PlayerId);
	Recent.Add(Event);
	if (Recent.Num() > 50)
	{
		Recent.RemoveAt(0);
	}

	// Check for wreck
	if (VehicleState)
	{
		CheckWreckCondition(PlayerId, Event);
	}

	// Trigger effects
	TriggerCrashEffects(Event);

	// Grant invincibility
	GrantInvincibility(PlayerId, PhysicsConfig.InvincibilityAfterCollision);

	OnCollisionOccurred.Broadcast(PlayerId, Event);

	return Event;
}

void UMGCollisionSubsystem::ProcessVehicleToVehicle(const FString& VehicleA, const FString& VehicleB, FVector ImpactLocation, FVector RelativeVelocity)
{
	FVector ImpactNormal = RelativeVelocity.GetSafeNormal();

	// Process for vehicle A
	EMGCollisionType TypeA = DetectCollisionType(ImpactNormal, FVector::ForwardVector, RelativeVelocity);
	FMGCollisionEvent EventA = ProcessCollision(VehicleA, VehicleB, ImpactLocation, ImpactNormal, RelativeVelocity, TypeA);

	// Process for vehicle B (reversed)
	EMGCollisionType TypeB = DetectCollisionType(-ImpactNormal, FVector::ForwardVector, -RelativeVelocity);
	FMGCollisionEvent EventB = ProcessCollision(VehicleB, VehicleA, ImpactLocation, -ImpactNormal, -RelativeVelocity, TypeB);

	// Check for takedown
	FMGVehicleCollisionState* StateA = VehicleStates.Find(VehicleA);
	FMGVehicleCollisionState* StateB = VehicleStates.Find(VehicleB);

	if (StateA && StateB)
	{
		if (StateB->bIsWrecked && !StateA->bIsWrecked)
		{
			RegisterTakedown(VehicleA, VehicleB, EventA);
		}
		else if (StateA->bIsWrecked && !StateB->bIsWrecked)
		{
			RegisterTakedown(VehicleB, VehicleA, EventB);
		}
	}
}

void UMGCollisionSubsystem::ProcessVehicleToStatic(const FString& VehicleId, FVector ImpactLocation, FVector ImpactNormal, FVector Velocity)
{
	EMGCollisionType Type = EMGCollisionType::VehicleToWall;
	ProcessCollision(VehicleId, TEXT("Static"), ImpactLocation, ImpactNormal, Velocity, Type);
}

// ============================================================================
// Collision Detection
// ============================================================================

EMGCollisionType UMGCollisionSubsystem::DetectCollisionType(FVector ImpactNormal, FVector VehicleForward, FVector RelativeVelocity) const
{
	float ForwardDot = FVector::DotProduct(ImpactNormal, VehicleForward);
	float SideDot = FVector::DotProduct(ImpactNormal, FVector::RightVector);

	// Front/back collision
	if (FMath::Abs(ForwardDot) > 0.7f)
	{
		if (ForwardDot > 0)
		{
			return EMGCollisionType::HeadOn;
		}
		else
		{
			return EMGCollisionType::RearEnd;
		}
	}
	// Side collision
	else if (FMath::Abs(SideDot) > 0.7f)
	{
		// Check for T-bone vs sideswipe
		float RelativeSpeedDot = FVector::DotProduct(RelativeVelocity.GetSafeNormal(), ImpactNormal);
		if (FMath::Abs(RelativeSpeedDot) > 0.8f)
		{
			return EMGCollisionType::TBone;
		}
		else
		{
			return EMGCollisionType::Sideswipe;
		}
	}

	return EMGCollisionType::VehicleToVehicle;
}

EMGCollisionSeverity UMGCollisionSubsystem::CalculateSeverity(float ImpactSpeed, float ImpactForce) const
{
	if (ImpactSpeed >= 120.0f)
	{
		return EMGCollisionSeverity::Catastrophic;
	}
	else if (ImpactSpeed >= 90.0f)
	{
		return EMGCollisionSeverity::Severe;
	}
	else if (ImpactSpeed >= 60.0f)
	{
		return EMGCollisionSeverity::Major;
	}
	else if (ImpactSpeed >= 40.0f)
	{
		return EMGCollisionSeverity::Moderate;
	}
	else if (ImpactSpeed >= 25.0f)
	{
		return EMGCollisionSeverity::Minor;
	}

	return EMGCollisionSeverity::Glancing;
}

EMGImpactZone UMGCollisionSubsystem::DetermineImpactZone(FVector LocalImpactPoint) const
{
	// Simplified zone detection based on normalized impact point
	FVector Norm = LocalImpactPoint.GetSafeNormal();

	if (Norm.X > 0.5f)
	{
		if (Norm.Y > 0.3f) return EMGImpactZone::FrontRight;
		if (Norm.Y < -0.3f) return EMGImpactZone::FrontLeft;
		return EMGImpactZone::FrontCenter;
	}
	else if (Norm.X < -0.5f)
	{
		if (Norm.Y > 0.3f) return EMGImpactZone::RearRight;
		if (Norm.Y < -0.3f) return EMGImpactZone::RearLeft;
		return EMGImpactZone::RearCenter;
	}
	else
	{
		if (Norm.Y > 0) return EMGImpactZone::SideRight;
		return EMGImpactZone::SideLeft;
	}
}

// ============================================================================
// Damage System
// ============================================================================

float UMGCollisionSubsystem::ApplyDamage(const FString& VehicleId, float DamageAmount, EMGImpactZone Zone)
{
	FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return 0.0f;
	}

	// Apply damage to zone
	FMGDamageZone* DamageZone = State->DamageZones.Find(Zone);
	float ActualDamage = DamageAmount;

	if (DamageZone)
	{
		// Apply armor if present
		if (DamageZone->bIsArmored)
		{
			ActualDamage *= (1.0f - DamageZone->ArmorRating);
		}

		ActualDamage *= DamageZone->DamageMultiplier;

		float OldHealth = DamageZone->CurrentHealth;
		DamageZone->CurrentHealth = FMath::Max(0.0f, DamageZone->CurrentHealth - ActualDamage);

		// Update zone state
		float HealthPercent = DamageZone->CurrentHealth / DamageZone->MaxHealth * 100.0f;
		EMGDamageState OldState = DamageZone->State;
		DamageZone->State = CalculateDamageState(HealthPercent);

		if (OldState != DamageZone->State)
		{
			OnDamageStateChanged.Broadcast(VehicleId, OldState, DamageZone->State);
		}
	}

	// Update total health
	State->TotalDamageTaken += ActualDamage;
	UpdateVehicleState(VehicleId);

	OnDamageReceived.Broadcast(VehicleId, ActualDamage, Zone);

	return ActualDamage;
}

void UMGCollisionSubsystem::RepairVehicle(const FString& VehicleId, float RepairAmount)
{
	FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return;
	}

	// Distribute repair across all zones
	float RepairPerZone = RepairAmount / State->DamageZones.Num();

	for (auto& Pair : State->DamageZones)
	{
		FMGDamageZone& Zone = Pair.Value;
		Zone.CurrentHealth = FMath::Min(Zone.MaxHealth, Zone.CurrentHealth + RepairPerZone);
		Zone.State = CalculateDamageState(Zone.CurrentHealth / Zone.MaxHealth * 100.0f);
	}

	State->bIsWrecked = false;
	State->bIsOnFire = false;
	UpdateVehicleState(VehicleId);

	OnVehicleRepaired.Broadcast(VehicleId, RepairAmount);
}

void UMGCollisionSubsystem::RepairZone(const FString& VehicleId, EMGImpactZone Zone, float RepairAmount)
{
	FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return;
	}

	FMGDamageZone* DamageZone = State->DamageZones.Find(Zone);
	if (DamageZone)
	{
		DamageZone->CurrentHealth = FMath::Min(DamageZone->MaxHealth, DamageZone->CurrentHealth + RepairAmount);
		DamageZone->State = CalculateDamageState(DamageZone->CurrentHealth / DamageZone->MaxHealth * 100.0f);
	}

	UpdateVehicleState(VehicleId);
	OnVehicleRepaired.Broadcast(VehicleId, RepairAmount);
}

void UMGCollisionSubsystem::FullRepair(const FString& VehicleId)
{
	FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return;
	}

	for (auto& Pair : State->DamageZones)
	{
		FMGDamageZone& Zone = Pair.Value;
		Zone.CurrentHealth = Zone.MaxHealth;
		Zone.State = EMGDamageState::Pristine;
	}

	State->TotalHealth = State->MaxHealth;
	State->OverallState = EMGDamageState::Pristine;
	State->EngineHealth = 100.0f;
	State->SteeringHealth = 100.0f;
	State->SuspensionHealth = 100.0f;
	State->TransmissionHealth = 100.0f;
	State->bIsWrecked = false;
	State->bIsOnFire = false;
	State->FireDuration = 0.0f;

	for (int32 i = 0; i < State->TireStatus.Num(); i++)
	{
		State->TireStatus[i] = true;
	}

	OnVehicleRepaired.Broadcast(VehicleId, State->MaxHealth);
}

float UMGCollisionSubsystem::GetTotalHealth(const FString& VehicleId) const
{
	if (const FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId))
	{
		return State->TotalHealth;
	}
	return 0.0f;
}

float UMGCollisionSubsystem::GetZoneHealth(const FString& VehicleId, EMGImpactZone Zone) const
{
	if (const FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId))
	{
		if (const FMGDamageZone* DamageZone = State->DamageZones.Find(Zone))
		{
			return DamageZone->CurrentHealth;
		}
	}
	return 0.0f;
}

EMGDamageState UMGCollisionSubsystem::GetDamageState(const FString& VehicleId) const
{
	if (const FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId))
	{
		return State->OverallState;
	}
	return EMGDamageState::Pristine;
}

bool UMGCollisionSubsystem::IsWrecked(const FString& VehicleId) const
{
	if (const FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId))
	{
		return State->bIsWrecked;
	}
	return false;
}

// ============================================================================
// Takedowns
// ============================================================================

FMGTakedownEvent UMGCollisionSubsystem::RegisterTakedown(const FString& AttackerId, const FString& VictimId, const FMGCollisionEvent& Collision)
{
	FMGTakedownEvent Takedown;
	Takedown.TakedownId = GenerateTakedownId();
	Takedown.AttackerId = AttackerId;
	Takedown.VictimId = VictimId;
	Takedown.CollisionType = Collision.Type;
	Takedown.Location = Collision.ImpactLocation;
	Takedown.ImpactSpeed = Collision.ImpactSpeed;
	Takedown.Timestamp = FDateTime::Now();

	// Calculate points
	int32 Points = ScoringConfig.BasePointsPerTakedown;

	// Apply type bonus
	if (const int32* TypeBonus = ScoringConfig.TypeBonusPoints.Find(Collision.Type))
	{
		Points += *TypeBonus;
	}

	// Apply severity multiplier
	if (const float* SeverityMult = ScoringConfig.SeverityMultipliers.Find(Collision.Severity))
	{
		Points = FMath::RoundToInt(Points * *SeverityMult);
	}

	// Check for revenge
	if (IsRevengeTarget(AttackerId, VictimId))
	{
		Points += ScoringConfig.RevengeBonus;
		Takedown.bWasRevenge = true;
		RevengeTargets.Remove(AttackerId);
		
		// Track revenge stat
		FMGCollisionStats& RevengeStats = PlayerStats.FindOrAdd(AttackerId);
		RevengeStats.TotalRevenges++;
		
		OnRevengeComplete.Broadcast(AttackerId, VictimId);
	}

	// Update chain
	int32& Chain = TakedownChains.FindOrAdd(AttackerId);
	Chain++;
	Takedown.ChainCount = Chain;

	float ChainMultiplier = 1.0f + (Chain - 1) * ScoringConfig.ChainMultiplierPerTakedown;
	ChainMultiplier = FMath::Min(ChainMultiplier, ScoringConfig.MaxChainMultiplier);
	Points = FMath::RoundToInt(Points * ChainMultiplier);

	TakedownChainTimers.Add(AttackerId, ScoringConfig.ChainWindowSeconds);

	// Track best chain stat
	FMGCollisionStats& ChainStats = PlayerStats.FindOrAdd(AttackerId);
	if (Chain > ChainStats.BestTakedownChain)
	{
		ChainStats.BestTakedownChain = Chain;
	}

	Takedown.PointsAwarded = Points;

	// Set revenge target for victim
	RevengeTargets.Add(VictimId, AttackerId);

	// Update stats
	FMGCollisionStats& AttackerStats = PlayerStats.FindOrAdd(AttackerId);
	AttackerStats.TakedownsDealt++;
	AttackerStats.AggressivePointsEarned += Points;

	FMGCollisionStats& VictimStats = PlayerStats.FindOrAdd(VictimId);
	VictimStats.TakedownsReceived++;

	OnTakedownDealt.Broadcast(AttackerId, Takedown);
	OnTakedownReceived.Broadcast(VictimId, Takedown);

	if (Chain > 1)
	{
		OnTakedownChain.Broadcast(AttackerId, Chain, ChainMultiplier);
	}

	return Takedown;
}

int32 UMGCollisionSubsystem::GetTakedownChainCount(const FString& PlayerId) const
{
	if (const int32* Chain = TakedownChains.Find(PlayerId))
	{
		return *Chain;
	}
	return 0;
}

float UMGCollisionSubsystem::GetTakedownChainMultiplier(const FString& PlayerId) const
{
	int32 Chain = GetTakedownChainCount(PlayerId);
	float Multiplier = 1.0f + (Chain - 1) * ScoringConfig.ChainMultiplierPerTakedown;
	return FMath::Min(Multiplier, ScoringConfig.MaxChainMultiplier);
}

bool UMGCollisionSubsystem::IsRevengeTarget(const FString& PlayerId, const FString& TargetId) const
{
	if (const FString* Revenge = RevengeTargets.Find(PlayerId))
	{
		return *Revenge == TargetId;
	}
	return false;
}

void UMGCollisionSubsystem::ClearTakedownChain(const FString& PlayerId)
{
	TakedownChains.Remove(PlayerId);
	TakedownChainTimers.Remove(PlayerId);
}

// ============================================================================
// Invincibility
// ============================================================================

void UMGCollisionSubsystem::GrantInvincibility(const FString& VehicleId, float Duration)
{
	InvincibilityTimers.Add(VehicleId, Duration);
	OnInvincibilityStart.Broadcast(VehicleId, Duration);
}

bool UMGCollisionSubsystem::IsInvincible(const FString& VehicleId) const
{
	if (const float* Timer = InvincibilityTimers.Find(VehicleId))
	{
		return *Timer > 0.0f;
	}
	return false;
}

float UMGCollisionSubsystem::GetRemainingInvincibility(const FString& VehicleId) const
{
	if (const float* Timer = InvincibilityTimers.Find(VehicleId))
	{
		return FMath::Max(0.0f, *Timer);
	}
	return 0.0f;
}

// ============================================================================
// Physics Response
// ============================================================================

FVector UMGCollisionSubsystem::CalculateBounceVelocity(FVector InVelocity, FVector ImpactNormal, float Restitution) const
{
	float VelocityDotNormal = FVector::DotProduct(InVelocity, ImpactNormal);
	FVector ReflectedVelocity = InVelocity - 2.0f * VelocityDotNormal * ImpactNormal;
	return ReflectedVelocity * Restitution * PhysicsConfig.BounceMultiplier;
}

FRotator UMGCollisionSubsystem::CalculateSpinImpulse(const FMGCollisionEvent& Collision) const
{
	FRotator Spin = FRotator::ZeroRotator;

	// Calculate spin based on impact zone and speed
	float SpinMagnitude = Collision.ImpactSpeed * PhysicsConfig.SpinImpulseMultiplier;

	switch (Collision.ImpactZone)
	{
		case EMGImpactZone::FrontLeft:
		case EMGImpactZone::RearRight:
			Spin.Yaw = SpinMagnitude;
			break;
		case EMGImpactZone::FrontRight:
		case EMGImpactZone::RearLeft:
			Spin.Yaw = -SpinMagnitude;
			break;
		case EMGImpactZone::SideLeft:
			Spin.Yaw = SpinMagnitude * 0.5f;
			break;
		case EMGImpactZone::SideRight:
			Spin.Yaw = -SpinMagnitude * 0.5f;
			break;
		default:
			break;
	}

	return Spin;
}

float UMGCollisionSubsystem::CalculateSpeedLoss(const FMGCollisionEvent& Collision) const
{
	float BaseLoss = FMath::Lerp(
		PhysicsConfig.SpeedLossPercentMin,
		PhysicsConfig.SpeedLossPercentMax,
		Collision.ImpactSpeed / 150.0f
	);

	// Modify based on collision type
	switch (Collision.Type)
	{
		case EMGCollisionType::HeadOn:
			BaseLoss *= 1.5f;
			break;
		case EMGCollisionType::Sideswipe:
			BaseLoss *= 0.5f;
			break;
		case EMGCollisionType::RearEnd:
			BaseLoss *= 0.75f;
			break;
		default:
			break;
	}

	return FMath::Clamp(BaseLoss, 0.0f, 1.0f);
}

// ============================================================================
// Effects
// ============================================================================

void UMGCollisionSubsystem::RegisterCrashEffect(const FMGCrashEffect& Effect)
{
	// Insert in sorted order by severity
	int32 InsertIndex = 0;
	for (int32 i = 0; i < CrashEffects.Num(); i++)
	{
		if (CrashEffects[i].MinSeverity > Effect.MinSeverity)
		{
			InsertIndex = i;
			break;
		}
		InsertIndex = i + 1;
	}
	CrashEffects.Insert(Effect, InsertIndex);
}

FMGCrashEffect UMGCollisionSubsystem::GetCrashEffect(EMGCollisionSeverity Severity) const
{
	FMGCrashEffect Result;

	for (const FMGCrashEffect& Effect : CrashEffects)
	{
		if (Severity >= Effect.MinSeverity)
		{
			Result = Effect;
		}
	}

	return Result;
}

void UMGCollisionSubsystem::TriggerCrashEffects(const FMGCollisionEvent& Collision)
{
	FMGCrashEffect Effect = GetCrashEffect(Collision.Severity);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 1. Spawn particle effect at impact location
	if (Effect.ParticleEffect.IsValid())
	{
		if (UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(Effect.ParticleEffect.LoadSynchronous()))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				NiagaraSystem,
				Collision.ImpactLocation,
				Collision.ImpactNormal.Rotation(),
				FVector(1.0f),
				true,
				true
			);
		}
	}

	// 2. Play sound effect at impact location
	if (Effect.SoundEffect.IsValid())
	{
		if (USoundBase* Sound = Cast<USoundBase>(Effect.SoundEffect.LoadSynchronous()))
		{
			// Scale volume based on impact force
			float VolumeMultiplier = FMath::Clamp(Collision.ImpactForce / 10000.0f, 0.5f, 1.5f);
			UGameplayStatics::PlaySoundAtLocation(
				World,
				Sound,
				Collision.ImpactLocation,
				VolumeMultiplier
			);
		}
	}

	// 3. Trigger camera shake for local player
	if (Effect.CameraShakeIntensity > 0.0f)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			// Use screen effect subsystem for camera shake
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UMGScreenEffectSubsystem* ScreenEffects = GI->GetSubsystem<UMGScreenEffectSubsystem>())
				{
					ScreenEffects->TriggerImpactShake(Effect.CameraShakeIntensity, Collision.ImpactNormal);
				}
			}
		}
	}

	// 4. Trigger slow motion for dramatic crashes
	if (Effect.SlowMotionDuration > 0.0f && Collision.Severity >= EMGCollisionSeverity::Major)
	{
		UGameplayStatics::SetGlobalTimeDilation(World, Effect.SlowMotionScale);

		// Set timer to restore normal time
		FTimerHandle TimeDilationHandle;
		World->GetTimerManager().SetTimer(
			TimeDilationHandle,
			[World, SlowMoDuration = Effect.SlowMotionDuration]()
			{
				if (World)
				{
					UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
				}
			},
			Effect.SlowMotionDuration * Effect.SlowMotionScale, // Adjusted for time dilation
			false
		);
	}

	// 5. Trigger controller rumble/haptics
	if (Effect.bTriggerRumble)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMGHapticsSubsystem* Haptics = GI->GetSubsystem<UMGHapticsSubsystem>())
			{
				Haptics->PlayCollisionFeedback(Collision.ImpactForce, Collision.ImpactNormal);
			}
		}
	}

	// Broadcast collision event for other systems to react
	OnCollisionEffectsTriggered.Broadcast(Collision, Effect);
}

// ============================================================================
// Stats
// ============================================================================

FMGCollisionStats UMGCollisionSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGCollisionStats* Stats = PlayerStats.Find(PlayerId))
	{
		return *Stats;
	}
	return FMGCollisionStats();
}

void UMGCollisionSubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGCollisionStats Stats;
	Stats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, Stats);
}

TArray<FMGCollisionEvent> UMGCollisionSubsystem::GetRecentCollisions(const FString& PlayerId, int32 MaxCount) const
{
	TArray<FMGCollisionEvent> Result;

	if (const TArray<FMGCollisionEvent>* Recent = RecentCollisions.Find(PlayerId))
	{
		int32 Count = FMath::Min(MaxCount, Recent->Num());
		for (int32 i = Recent->Num() - Count; i < Recent->Num(); i++)
		{
			Result.Add((*Recent)[i]);
		}
	}

	return Result;
}

// ============================================================================
// Configuration
// ============================================================================

void UMGCollisionSubsystem::SetPhysicsConfig(const FMGCollisionPhysicsConfig& Config)
{
	PhysicsConfig = Config;
}

FMGCollisionPhysicsConfig UMGCollisionSubsystem::GetPhysicsConfig() const
{
	return PhysicsConfig;
}

void UMGCollisionSubsystem::SetScoringConfig(const FMGCollisionScoringConfig& Config)
{
	ScoringConfig = Config;
}

FMGCollisionScoringConfig UMGCollisionSubsystem::GetScoringConfig() const
{
	return ScoringConfig;
}

// ============================================================================
// Update
// ============================================================================

void UMGCollisionSubsystem::UpdateCollisionSystem(float MGDeltaTime)
{
	UpdateInvincibility(DeltaTime);
	UpdateTakedownChains(DeltaTime);
	UpdateFireDamage(DeltaTime);
}

// ============================================================================
// Save/Load
// ============================================================================

void UMGCollisionSubsystem::SaveCollisionData()
{
	FString DataDir = FPaths::ProjectSavedDir() / TEXT("Collision");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*DataDir))
	{
		PlatformFile.CreateDirectory(*DataDir);
	}

	FString FilePath = DataDir / TEXT("collision_stats.dat");

	FBufferArchive Archive;

	// Write version
	int32 Version = 1;
	Archive << Version;

	// Write player stats
	int32 PlayerCount = PlayerStats.Num();
	Archive << PlayerCount;

	for (const auto& Pair : PlayerStats)
	{
		FString PlayerId = Pair.Key;
		const FMGCollisionStats& Stats = Pair.Value;

		Archive << PlayerId;
		Archive << Stats.TotalCollisions;
		Archive << Stats.TakedownsDealt;
		Archive << Stats.TakedownsReceived;
		Archive << Stats.TotalDamageDealt;
		Archive << Stats.TotalDamageReceived;
		Archive << Stats.BestTakedownChain;
		Archive << Stats.TotalRevenges;

		// Write collision type counts
		int32 TypeCount = Stats.CollisionsByType.Num();
		Archive << TypeCount;
		for (const auto& TypePair : Stats.CollisionsByType)
		{
			int32 TypeInt = static_cast<int32>(TypePair.Key);
			int32 Count = TypePair.Value;
			Archive << TypeInt;
			Archive << Count;
		}

		// Write severity counts
		int32 SevCount = Stats.CollisionsBySeverity.Num();
		Archive << SevCount;
		for (const auto& SevPair : Stats.CollisionsBySeverity)
		{
			int32 SevInt = static_cast<int32>(SevPair.Key);
			int32 Count = SevPair.Value;
			Archive << SevInt;
			Archive << Count;
		}
	}

	FFileHelper::SaveArrayToFile(Archive, *FilePath);
	Archive.FlushCache();
	Archive.Empty();

	UE_LOG(LogTemp, Log, TEXT("MGCollision: Saved collision stats for %d players"), PlayerCount);
}

void UMGCollisionSubsystem::LoadCollisionData()
{
	FString DataDir = FPaths::ProjectSavedDir() / TEXT("Collision");
	FString FilePath = DataDir / TEXT("collision_stats.dat");

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return;
	}

	FMemoryReader Archive(FileData, true);

	// Read version
	int32 Version;
	Archive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGCollision: Unknown save version %d"), Version);
		return;
	}

	// Read player stats
	int32 PlayerCount;
	Archive << PlayerCount;

	for (int32 i = 0; i < PlayerCount; i++)
	{
		FString PlayerId;
		FMGCollisionStats Stats;

		Archive << PlayerId;
		Archive << Stats.TotalCollisions;
		Archive << Stats.TakedownsDealt;
		Archive << Stats.TakedownsReceived;
		Archive << Stats.TotalDamageDealt;
		Archive << Stats.TotalDamageReceived;
		Archive << Stats.BestTakedownChain;
		Archive << Stats.TotalRevenges;

		// Read collision type counts
		int32 TypeCount;
		Archive << TypeCount;
		for (int32 j = 0; j < TypeCount; j++)
		{
			int32 TypeInt;
			int32 Count;
			Archive << TypeInt;
			Archive << Count;
			Stats.CollisionsByType.Add(static_cast<EMGCollisionType>(TypeInt), Count);
		}

		// Read severity counts
		int32 SevCount;
		Archive << SevCount;
		for (int32 j = 0; j < SevCount; j++)
		{
			int32 SevInt;
			int32 Count;
			Archive << SevInt;
			Archive << Count;
			Stats.CollisionsBySeverity.Add(static_cast<EMGCollisionSeverity>(SevInt), Count);
		}

		Stats.PlayerId = PlayerId;
		PlayerStats.Add(PlayerId, Stats);
	}

	UE_LOG(LogTemp, Log, TEXT("MGCollision: Loaded collision stats for %d players"), PlayerCount);
}

// ============================================================================
// Protected Methods
// ============================================================================

void UMGCollisionSubsystem::TickCollision(float MGDeltaTime)
{
	UpdateCollisionSystem(DeltaTime);
}

void UMGCollisionSubsystem::UpdateInvincibility(float MGDeltaTime)
{
	TArray<FString> Expired;

	for (auto& Pair : InvincibilityTimers)
	{
		Pair.Value -= DeltaTime;
		if (Pair.Value <= 0.0f)
		{
			Expired.Add(Pair.Key);
		}
	}

	for (const FString& VehicleId : Expired)
	{
		InvincibilityTimers.Remove(VehicleId);
		OnInvincibilityEnd.Broadcast(VehicleId);
	}
}

void UMGCollisionSubsystem::UpdateTakedownChains(float MGDeltaTime)
{
	TArray<FString> Expired;

	for (auto& Pair : TakedownChainTimers)
	{
		Pair.Value -= DeltaTime;
		if (Pair.Value <= 0.0f)
		{
			Expired.Add(Pair.Key);
		}
	}

	for (const FString& PlayerId : Expired)
	{
		ClearTakedownChain(PlayerId);
	}
}

void UMGCollisionSubsystem::UpdateFireDamage(float MGDeltaTime)
{
	for (auto& Pair : VehicleStates)
	{
		FMGVehicleCollisionState& State = Pair.Value;

		if (State.bIsOnFire)
		{
			State.FireDuration += DeltaTime;

			// Apply fire damage
			float FireDamage = 5.0f * DeltaTime;
			State.TotalHealth -= FireDamage;
			State.EngineHealth -= FireDamage * 2.0f;

			if (State.TotalHealth <= 0.0f)
			{
				State.bIsWrecked = true;
			}
		}
	}
}

EMGDamageState UMGCollisionSubsystem::CalculateDamageState(float HealthPercent) const
{
	if (HealthPercent >= 100.0f)
	{
		return EMGDamageState::Pristine;
	}
	else if (HealthPercent >= 80.0f)
	{
		return EMGDamageState::Scratched;
	}
	else if (HealthPercent >= 60.0f)
	{
		return EMGDamageState::Dented;
	}
	else if (HealthPercent >= 40.0f)
	{
		return EMGDamageState::Damaged;
	}
	else if (HealthPercent >= 20.0f)
	{
		return EMGDamageState::HeavyDamage;
	}
	else if (HealthPercent > 0.0f)
	{
		return EMGDamageState::Critical;
	}

	return EMGDamageState::Wrecked;
}

void UMGCollisionSubsystem::CheckWreckCondition(const FString& VehicleId, const FMGCollisionEvent& Collision)
{
	FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return;
	}

	// Check total health
	float TotalZoneHealth = 0.0f;
	for (const auto& Pair : State->DamageZones)
	{
		TotalZoneHealth += Pair.Value.CurrentHealth;
	}

	State->TotalHealth = TotalZoneHealth / State->DamageZones.Num();

	if (State->TotalHealth <= 0.0f || State->EngineHealth <= 0.0f)
	{
		State->bIsWrecked = true;
		State->OverallState = EMGDamageState::Wrecked;
		PlayerStats.FindOrAdd(VehicleId).WrecksTotal++;
		OnVehicleWrecked.Broadcast(VehicleId, Collision);
	}
}

void UMGCollisionSubsystem::UpdateVehicleState(const FString& VehicleId)
{
	FMGVehicleCollisionState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return;
	}

	// Calculate average health
	float TotalHealth = 0.0f;
	EMGDamageState WorstState = EMGDamageState::Pristine;

	for (const auto& Pair : State->DamageZones)
	{
		TotalHealth += Pair.Value.CurrentHealth;
		if (Pair.Value.State > WorstState)
		{
			WorstState = Pair.Value.State;
		}
	}

	State->TotalHealth = TotalHealth / State->DamageZones.Num() * State->MaxHealth / 100.0f;

	EMGDamageState OldState = State->OverallState;
	State->OverallState = WorstState;

	if (OldState != State->OverallState)
	{
		OnDamageStateChanged.Broadcast(VehicleId, OldState, State->OverallState);
	}
}

void UMGCollisionSubsystem::UpdatePlayerStats(const FString& PlayerId, const FMGCollisionEvent& Collision)
{
	FMGCollisionStats& Stats = PlayerStats.FindOrAdd(PlayerId);
	Stats.PlayerId = PlayerId;
	Stats.TotalCollisions++;

	int32& TypeCount = Stats.CollisionsByType.FindOrAdd(Collision.Type);
	TypeCount++;

	int32& SeverityCount = Stats.CollisionsBySeverity.FindOrAdd(Collision.Severity);
	SeverityCount++;

	Stats.TotalDamageReceived += Collision.DamageDealt;

	if (Collision.ImpactSpeed > Stats.HighestImpactSpeed)
	{
		Stats.HighestImpactSpeed = Collision.ImpactSpeed;
	}

	if (Collision.ImpactForce > Stats.HighestImpactForce)
	{
		Stats.HighestImpactForce = Collision.ImpactForce;
	}
}

FString UMGCollisionSubsystem::GenerateCollisionId() const
{
	return FString::Printf(TEXT("COL_%d_%lld"), ++const_cast<UMGCollisionSubsystem*>(this)->CollisionCounter,
	                       FDateTime::Now().GetTicks());
}

FString UMGCollisionSubsystem::GenerateTakedownId() const
{
	return FString::Printf(TEXT("TKD_%d_%lld"), ++const_cast<UMGCollisionSubsystem*>(this)->TakedownCounter,
	                       FDateTime::Now().GetTicks());
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Takedown/MGTakedownSubsystem.h"
#include "Save/MGSaveManagerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UMGTakedownSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultPointValues();

	// Initialize default crash camera config
	CrashCameraConfig.Mode = EMGCrashCameraMode::QuickSlowMo;
	CrashCameraConfig.SlowMotionScale = 0.25f;
	CrashCameraConfig.Duration = 2.0f;
	CrashCameraConfig.CameraDistance = 5.0f;
	CrashCameraConfig.OrbitSpeed = 30.0f;
	CrashCameraConfig.bEnableAftertouch = true;
	CrashCameraConfig.AftertouchForce = 500.0f;
	CrashCameraConfig.bFollowDebris = false;
	CrashCameraConfig.ShakeIntensity = 0.5f;

	// Initialize aggression state
	AggressionState.Level = EMGAggressionLevel::None;
	AggressionState.AggressionMeter = 0.0f;
	AggressionState.MaxAggression = 100.0f;
	AggressionState.DecayRate = 5.0f;
	AggressionState.TakedownBonus = 25.0f;
	AggressionState.CollisionBonus = 5.0f;
	AggressionState.RampageDuration = 10.0f;

	// Initialize streak
	CurrentStreak.StreakWindow = 10.0f;

	LoadTakedownData();
}

void UMGTakedownSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AggressionTickTimer);
		World->GetTimerManager().ClearTimer(StreakTickTimer);
		World->GetTimerManager().ClearTimer(CrashCameraTimer);
	}

	SaveTakedownData();
	Super::Deinitialize();
}

void UMGTakedownSubsystem::InitializeDefaultPointValues()
{
	// Ram
	{
		FMGTakedownPoints Ram;
		Ram.TakedownType = EMGTakedownType::Ram;
		Ram.BasePoints = 100;
		Ram.SpeedMultiplier = 1.5f;
		Ram.ForceMultiplier = 1.25f;
		Ram.RevengeBonus = 50;
		Ram.AirborneBonus = 100;
		Ram.TrafficBonus = 75;
		Ram.WallBonus = 50;
		Ram.BoostReward = 10.0f;
		TakedownPointValues.Add(EMGTakedownType::Ram, Ram);
	}

	// Sideswipe
	{
		FMGTakedownPoints Sideswipe;
		Sideswipe.TakedownType = EMGTakedownType::Sideswipe;
		Sideswipe.BasePoints = 75;
		Sideswipe.SpeedMultiplier = 1.25f;
		Sideswipe.ForceMultiplier = 1.0f;
		Sideswipe.RevengeBonus = 50;
		Sideswipe.AirborneBonus = 75;
		Sideswipe.TrafficBonus = 50;
		Sideswipe.WallBonus = 100;
		Sideswipe.BoostReward = 7.5f;
		TakedownPointValues.Add(EMGTakedownType::Sideswipe, Sideswipe);
	}

	// PIT Maneuver
	{
		FMGTakedownPoints PIT;
		PIT.TakedownType = EMGTakedownType::PIT;
		PIT.BasePoints = 150;
		PIT.SpeedMultiplier = 1.75f;
		PIT.ForceMultiplier = 1.0f;
		PIT.RevengeBonus = 100;
		PIT.AirborneBonus = 150;
		PIT.TrafficBonus = 100;
		PIT.WallBonus = 75;
		PIT.BoostReward = 15.0f;
		TakedownPointValues.Add(EMGTakedownType::PIT, PIT);
	}

	// Shunt
	{
		FMGTakedownPoints Shunt;
		Shunt.TakedownType = EMGTakedownType::Shunt;
		Shunt.BasePoints = 125;
		Shunt.SpeedMultiplier = 2.0f;
		Shunt.ForceMultiplier = 1.5f;
		Shunt.RevengeBonus = 75;
		Shunt.AirborneBonus = 100;
		Shunt.TrafficBonus = 125;
		Shunt.WallBonus = 50;
		Shunt.BoostReward = 12.5f;
		TakedownPointValues.Add(EMGTakedownType::Shunt, Shunt);
	}

	// Slam
	{
		FMGTakedownPoints Slam;
		Slam.TakedownType = EMGTakedownType::Slam;
		Slam.BasePoints = 100;
		Slam.SpeedMultiplier = 1.5f;
		Slam.ForceMultiplier = 1.75f;
		Slam.RevengeBonus = 50;
		Slam.AirborneBonus = 125;
		Slam.TrafficBonus = 75;
		Slam.WallBonus = 150;
		Slam.BoostReward = 10.0f;
		TakedownPointValues.Add(EMGTakedownType::Slam, Slam);
	}

	// Grind
	{
		FMGTakedownPoints Grind;
		Grind.TakedownType = EMGTakedownType::Grind;
		Grind.BasePoints = 50;
		Grind.SpeedMultiplier = 1.25f;
		Grind.ForceMultiplier = 0.5f;
		Grind.RevengeBonus = 25;
		Grind.AirborneBonus = 50;
		Grind.TrafficBonus = 25;
		Grind.WallBonus = 75;
		Grind.BoostReward = 5.0f;
		TakedownPointValues.Add(EMGTakedownType::Grind, Grind);
	}

	// Traffic Check
	{
		FMGTakedownPoints TrafficCheck;
		TrafficCheck.TakedownType = EMGTakedownType::TrafficCheck;
		TrafficCheck.BasePoints = 75;
		TrafficCheck.SpeedMultiplier = 1.5f;
		TrafficCheck.ForceMultiplier = 1.0f;
		TrafficCheck.RevengeBonus = 50;
		TrafficCheck.AirborneBonus = 100;
		TrafficCheck.TrafficBonus = 200;
		TrafficCheck.WallBonus = 50;
		TrafficCheck.BoostReward = 7.5f;
		TakedownPointValues.Add(EMGTakedownType::TrafficCheck, TrafficCheck);
	}

	// Wall Grind
	{
		FMGTakedownPoints WallGrind;
		WallGrind.TakedownType = EMGTakedownType::WallGrind;
		WallGrind.BasePoints = 100;
		WallGrind.SpeedMultiplier = 1.25f;
		WallGrind.ForceMultiplier = 1.0f;
		WallGrind.RevengeBonus = 50;
		WallGrind.AirborneBonus = 75;
		WallGrind.TrafficBonus = 75;
		WallGrind.WallBonus = 200;
		WallGrind.BoostReward = 10.0f;
		TakedownPointValues.Add(EMGTakedownType::WallGrind, WallGrind);
	}

	// Air Strike
	{
		FMGTakedownPoints AirStrike;
		AirStrike.TakedownType = EMGTakedownType::AirStrike;
		AirStrike.BasePoints = 200;
		AirStrike.SpeedMultiplier = 2.0f;
		AirStrike.ForceMultiplier = 2.0f;
		AirStrike.RevengeBonus = 100;
		AirStrike.AirborneBonus = 300;
		AirStrike.TrafficBonus = 150;
		AirStrike.WallBonus = 100;
		AirStrike.BoostReward = 20.0f;
		TakedownPointValues.Add(EMGTakedownType::AirStrike, AirStrike);
	}

	// Aftertouch
	{
		FMGTakedownPoints Aftertouch;
		Aftertouch.TakedownType = EMGTakedownType::Aftertouch;
		Aftertouch.BasePoints = 250;
		Aftertouch.SpeedMultiplier = 1.0f;
		Aftertouch.ForceMultiplier = 1.0f;
		Aftertouch.RevengeBonus = 150;
		Aftertouch.AirborneBonus = 200;
		Aftertouch.TrafficBonus = 150;
		Aftertouch.WallBonus = 100;
		Aftertouch.BoostReward = 25.0f;
		TakedownPointValues.Add(EMGTakedownType::Aftertouch, Aftertouch);
	}

	// Revenge
	{
		FMGTakedownPoints Revenge;
		Revenge.TakedownType = EMGTakedownType::Revenge;
		Revenge.BasePoints = 150;
		Revenge.SpeedMultiplier = 1.5f;
		Revenge.ForceMultiplier = 1.25f;
		Revenge.RevengeBonus = 200;
		Revenge.AirborneBonus = 150;
		Revenge.TrafficBonus = 100;
		Revenge.WallBonus = 75;
		Revenge.BoostReward = 15.0f;
		TakedownPointValues.Add(EMGTakedownType::Revenge, Revenge);
	}

	// Psyche-Out
	{
		FMGTakedownPoints Psyche;
		Psyche.TakedownType = EMGTakedownType::Psyche;
		Psyche.BasePoints = 75;
		Psyche.SpeedMultiplier = 1.0f;
		Psyche.ForceMultiplier = 0.5f;
		Psyche.RevengeBonus = 50;
		Psyche.AirborneBonus = 50;
		Psyche.TrafficBonus = 100;
		Psyche.WallBonus = 100;
		Psyche.BoostReward = 7.5f;
		TakedownPointValues.Add(EMGTakedownType::Psyche, Psyche);
	}

	// Signature
	{
		FMGTakedownPoints Signature;
		Signature.TakedownType = EMGTakedownType::Signature;
		Signature.BasePoints = 500;
		Signature.SpeedMultiplier = 2.0f;
		Signature.ForceMultiplier = 2.0f;
		Signature.RevengeBonus = 250;
		Signature.AirborneBonus = 250;
		Signature.TrafficBonus = 250;
		Signature.WallBonus = 250;
		Signature.BoostReward = 50.0f;
		TakedownPointValues.Add(EMGTakedownType::Signature, Signature);
	}
}

bool UMGTakedownSubsystem::ProcessCollision(const FMGTakedownCollision& Collision)
{
	if (!bSessionActive)
	{
		return false;
	}

	if (!IsValidTakedown(Collision))
	{
		// Still count as aggression
		AddAggression(AggressionState.CollisionBonus);
		return false;
	}

	EMGTakedownType Type = DetermineCollisionType(Collision);
	FMGTakedownEvent Event = RegisterTakedown(Type, Collision);

	// Trigger crash camera for significant takedowns
	if (Event.TotalPoints >= 150 || Event.bIsRevenge)
	{
		StartCrashCamera(Event);
	}

	return true;
}

EMGTakedownType UMGTakedownSubsystem::DetermineCollisionType(const FMGTakedownCollision& Collision) const
{
	// Calculate angle of impact
	FVector AttackerDir = Collision.AttackerVelocity.GetSafeNormal();
	FVector VictimDir = Collision.VictimVelocity.GetSafeNormal();
	FVector ImpactDir = Collision.ImpactNormal;

	float DotProduct = FVector::DotProduct(AttackerDir, ImpactDir);
	float AngleOfAttack = FMath::RadiansToDegrees(FMath::Acos(FMath::Abs(DotProduct)));

	// Check for airborne
	if (Collision.bAttackerAirborne)
	{
		return EMGTakedownType::AirStrike;
	}

	// Check for wall grind
	if (Collision.bNearWall)
	{
		return EMGTakedownType::WallGrind;
	}

	// Check for traffic check
	if (Collision.bNearTraffic)
	{
		return EMGTakedownType::TrafficCheck;
	}

	// Determine by angle
	if (AngleOfAttack < 15.0f)
	{
		// Direct rear hit
		return EMGTakedownType::Shunt;
	}
	else if (AngleOfAttack < 45.0f)
	{
		// PIT maneuver angle
		return EMGTakedownType::PIT;
	}
	else if (AngleOfAttack < 75.0f)
	{
		// Sideswipe
		return EMGTakedownType::Sideswipe;
	}
	else if (AngleOfAttack < 105.0f)
	{
		// Side slam
		return EMGTakedownType::Slam;
	}
	else
	{
		// Head-on
		return EMGTakedownType::Ram;
	}
}

bool UMGTakedownSubsystem::IsValidTakedown(const FMGTakedownCollision& Collision) const
{
	float ImpactForce = CalculateImpactForce(Collision);
	return ImpactForce >= MinTakedownImpactForce;
}

float UMGTakedownSubsystem::CalculateImpactForce(const FMGTakedownCollision& Collision) const
{
	FVector RelativeVelocity = Collision.AttackerVelocity - Collision.VictimVelocity;
	float RelativeSpeed = RelativeVelocity.Size();

	// F = m * v (simplified)
	float CombinedMass = Collision.AttackerMass + Collision.VictimMass;
	return RelativeSpeed * CombinedMass * 0.01f; // Scale factor
}

FMGTakedownEvent UMGTakedownSubsystem::RegisterTakedown(EMGTakedownType Type, const FMGTakedownCollision& Collision)
{
	FMGTakedownEvent Event;
	Event.EventId = FGuid::NewGuid().ToString();
	Event.TakedownType = Type;
	Event.TargetType = EMGTakedownTarget::Opponent;
	Event.Result = EMGTakedownResult::Success;
	Event.AttackerId = Collision.AttackerId;
	Event.VictimId = Collision.VictimId;
	Event.ImpactLocation = Collision.ImpactPoint;
	Event.ImpactVelocity = Collision.AttackerVelocity;
	Event.ImpactForce = CalculateImpactForce(Collision);
	Event.SpeedAtImpact = Collision.AttackerVelocity.Size() * 0.036f; // Convert to km/h
	Event.RelativeSpeed = (Collision.AttackerVelocity - Collision.VictimVelocity).Size() * 0.036f;
	Event.Timestamp = FDateTime::Now();
	Event.bIsAirborne = Collision.bAttackerAirborne;
	Event.bInvolvedTraffic = Collision.bNearTraffic;
	Event.bInvolvedWall = Collision.bNearWall;

	// Calculate angle
	FVector AttackerDir = Collision.AttackerVelocity.GetSafeNormal();
	FVector ImpactDir = Collision.ImpactNormal;
	Event.ImpactAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(AttackerDir, ImpactDir)));

	// Check revenge
	if (FMGRevengeTarget* RevengeTarget = RevengeTargets.Find(Collision.VictimId))
	{
		if (RevengeTarget->bIsActiveRevenge)
		{
			Event.bIsRevenge = true;
			Event.TakedownType = EMGTakedownType::Revenge;
			Event.BonusTags.Add(TEXT("REVENGE!"));
			OnRevengeTakedown.Broadcast(Collision.VictimId, GetTakedownPoints(EMGTakedownType::Revenge).RevengeBonus);
			ClearRevengeTarget(Collision.VictimId);
		}
	}

	// Calculate points
	Event.BasePoints = CalculateTakedownPoints(Event);

	// Apply aggression multiplier
	float AggressionMult = GetAggressionMultiplier();
	Event.TotalPoints = FMath::RoundToInt(Event.BasePoints * AggressionMult);

	// Calculate boost reward
	Event.BoostReward = CalculateBoostReward(Event);

	// Update stats
	SessionStats.TotalTakedowns++;
	SessionStats.TotalPoints += Event.TotalPoints;

	int32& TypeCount = SessionStats.TakedownsByType.FindOrAdd(Type);
	TypeCount++;

	int32& VictimCount = SessionStats.TakedownsByVictim.FindOrAdd(Collision.VictimId);
	VictimCount++;

	if (Event.bIsRevenge)
	{
		SessionStats.RevengeTakedowns++;
	}
	if (Event.bIsAirborne)
	{
		SessionStats.AirborneTakedowns++;
	}
	if (Event.bInvolvedTraffic)
	{
		SessionStats.TrafficTakedowns++;
	}
	if (Event.bInvolvedWall)
	{
		SessionStats.WallTakedowns++;
	}

	SessionStats.TotalBoostEarned += Event.BoostReward;

	if (Event.ImpactForce > SessionStats.HighestImpactForce)
	{
		SessionStats.HighestImpactForce = Event.ImpactForce;
	}

	// Add aggression
	AddAggression(AggressionState.TakedownBonus);

	// Extend streak
	ExtendStreak(Event);

	// Store recent takedown
	RecentTakedowns.Insert(Event, 0);
	if (RecentTakedowns.Num() > MaxRecentTakedowns)
	{
		RecentTakedowns.SetNum(MaxRecentTakedowns);
	}

	OnTakedownOccurred.Broadcast(Event, SessionStats.TotalPoints);

	return Event;
}

void UMGTakedownSubsystem::RegisterPlayerWreck(const FString& AttackerId, const FVector& Location)
{
	SessionStats.TotalTimesWrecked++;

	// Track revenge target
	TrackRevengeTarget(AttackerId);

	OnPlayerWrecked.Broadcast(AttackerId, Location);
}

int32 UMGTakedownSubsystem::CalculateTakedownPoints(const FMGTakedownEvent& Event) const
{
	FMGTakedownPoints Points = GetTakedownPoints(Event.TakedownType);

	int32 TotalPoints = Points.BasePoints;

	// Speed multiplier
	if (Event.SpeedAtImpact > 100.0f)
	{
		float SpeedBonus = (Event.SpeedAtImpact - 100.0f) / 100.0f * Points.SpeedMultiplier;
		TotalPoints = FMath::RoundToInt(TotalPoints * (1.0f + SpeedBonus));
	}

	// Force multiplier
	if (Event.ImpactForce > MinTakedownImpactForce)
	{
		float ForceBonus = (Event.ImpactForce - MinTakedownImpactForce) / MinTakedownImpactForce * Points.ForceMultiplier * 0.1f;
		TotalPoints = FMath::RoundToInt(TotalPoints * (1.0f + ForceBonus));
	}

	// Bonus points
	if (Event.bIsRevenge)
	{
		TotalPoints += Points.RevengeBonus;
	}
	if (Event.bIsAirborne)
	{
		TotalPoints += Points.AirborneBonus;
	}
	if (Event.bInvolvedTraffic)
	{
		TotalPoints += Points.TrafficBonus;
	}
	if (Event.bInvolvedWall)
	{
		TotalPoints += Points.WallBonus;
	}

	// Streak bonus (5% per streak)
	TotalPoints = FMath::RoundToInt(TotalPoints * (1.0f + CurrentStreak.CurrentStreak * 0.05f));

	return TotalPoints;
}

float UMGTakedownSubsystem::CalculateBoostReward(const FMGTakedownEvent& Event) const
{
	FMGTakedownPoints Points = GetTakedownPoints(Event.TakedownType);
	float Boost = Points.BoostReward;

	// Bonus boost for special takedowns
	if (Event.bIsRevenge)
	{
		Boost *= 1.5f;
	}
	if (Event.bIsAirborne)
	{
		Boost *= 1.25f;
	}

	// Rampage bonus
	if (IsRampageActive())
	{
		Boost *= 2.0f;
	}

	return Boost;
}

void UMGTakedownSubsystem::SetTakedownPoints(EMGTakedownType Type, const FMGTakedownPoints& Points)
{
	TakedownPointValues.Add(Type, Points);
}

FMGTakedownPoints UMGTakedownSubsystem::GetTakedownPoints(EMGTakedownType Type) const
{
	if (const FMGTakedownPoints* Points = TakedownPointValues.Find(Type))
	{
		return *Points;
	}

	// Return Ram as default
	if (const FMGTakedownPoints* Default = TakedownPointValues.Find(EMGTakedownType::Ram))
	{
		return *Default;
	}

	return FMGTakedownPoints();
}

FMGTakedownStreak UMGTakedownSubsystem::GetCurrentStreak() const
{
	return CurrentStreak;
}

int32 UMGTakedownSubsystem::GetCurrentStreakCount() const
{
	return CurrentStreak.CurrentStreak;
}

float UMGTakedownSubsystem::GetStreakTimeRemaining() const
{
	return CurrentStreak.StreakTimer;
}

void UMGTakedownSubsystem::ExtendStreak(const FMGTakedownEvent& Event)
{
	CurrentStreak.CurrentStreak++;
	CurrentStreak.StreakTimer = CurrentStreak.StreakWindow;
	CurrentStreak.StreakEvents.Add(Event);

	if (CurrentStreak.CurrentStreak > CurrentStreak.BestStreak)
	{
		CurrentStreak.BestStreak = CurrentStreak.CurrentStreak;
	}

	if (CurrentStreak.CurrentStreak > SessionStats.BestStreak)
	{
		SessionStats.BestStreak = CurrentStreak.CurrentStreak;
	}

	// Start streak timer if not running
	if (CurrentStreak.CurrentStreak == 1)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(StreakTickTimer, FTimerDelegate::CreateUObject(this, &UMGTakedownSubsystem::TickStreak, 0.1f), 0.1f, true);
		}
	}

	OnStreakUpdated.Broadcast(CurrentStreak.CurrentStreak, CurrentStreak.StreakTimer);
}

void UMGTakedownSubsystem::EndStreak()
{
	if (CurrentStreak.CurrentStreak > 0)
	{
		OnStreakEnded.Broadcast(CurrentStreak.CurrentStreak);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StreakTickTimer);
	}

	CurrentStreak.CurrentStreak = 0;
	CurrentStreak.StreakTimer = 0.0f;
	CurrentStreak.StreakEvents.Empty();
}

FMGAggressionState UMGTakedownSubsystem::GetAggressionState() const
{
	return AggressionState;
}

EMGAggressionLevel UMGTakedownSubsystem::GetAggressionLevel() const
{
	return AggressionState.Level;
}

float UMGTakedownSubsystem::GetAggressionPercent() const
{
	return (AggressionState.AggressionMeter / AggressionState.MaxAggression) * 100.0f;
}

void UMGTakedownSubsystem::AddAggression(float Amount)
{
	AggressionState.AggressionMeter = FMath::Min(AggressionState.AggressionMeter + Amount, AggressionState.MaxAggression);
	UpdateAggressionLevel();

	// Start aggression timer if not running
	if (AggressionState.Level != EMGAggressionLevel::None)
	{
		if (UWorld* World = GetWorld())
		{
			if (!World->GetTimerManager().IsTimerActive(AggressionTickTimer))
			{
				World->GetTimerManager().SetTimer(AggressionTickTimer, FTimerDelegate::CreateUObject(this, &UMGTakedownSubsystem::TickAggression, 0.1f), 0.1f, true);
			}
		}
	}
}

bool UMGTakedownSubsystem::IsRampageActive() const
{
	return AggressionState.Level == EMGAggressionLevel::Rampage && AggressionState.RampageTimer > 0.0f;
}

float UMGTakedownSubsystem::GetRampageTimeRemaining() const
{
	return AggressionState.RampageTimer;
}

float UMGTakedownSubsystem::GetAggressionMultiplier() const
{
	switch (AggressionState.Level)
	{
		case EMGAggressionLevel::None: return 1.0f;
		case EMGAggressionLevel::Mild: return 1.1f;
		case EMGAggressionLevel::Moderate: return 1.25f;
		case EMGAggressionLevel::Aggressive: return 1.5f;
		case EMGAggressionLevel::Violent: return 1.75f;
		case EMGAggressionLevel::Rampage: return 2.5f;
		default: return 1.0f;
	}
}

void UMGTakedownSubsystem::TrackRevengeTarget(const FString& TargetId)
{
	FMGRevengeTarget* Target = RevengeTargets.Find(TargetId);
	if (!Target)
	{
		FMGRevengeTarget NewTarget;
		NewTarget.TargetId = TargetId;
		RevengeTargets.Add(TargetId, NewTarget);
		Target = RevengeTargets.Find(TargetId);
	}

	Target->TimesWreckedBy++;
	Target->LastWreckedByTime = FDateTime::Now();
	Target->bIsActiveRevenge = true;
	Target->RevengeMultiplier = 1.5f + (Target->TimesWreckedBy - 1) * 0.25f;

	OnRevengeAvailable.Broadcast(TargetId, Target->RevengeMultiplier);
}

bool UMGTakedownSubsystem::HasRevengeTarget(const FString& TargetId) const
{
	if (const FMGRevengeTarget* Target = RevengeTargets.Find(TargetId))
	{
		return Target->bIsActiveRevenge;
	}
	return false;
}

FMGRevengeTarget UMGTakedownSubsystem::GetRevengeTarget(const FString& TargetId) const
{
	if (const FMGRevengeTarget* Target = RevengeTargets.Find(TargetId))
	{
		return *Target;
	}
	return FMGRevengeTarget();
}

TArray<FMGRevengeTarget> UMGTakedownSubsystem::GetActiveRevengeTargets() const
{
	TArray<FMGRevengeTarget> Result;
	for (const auto& TargetPair : RevengeTargets)
	{
		if (TargetPair.Value.bIsActiveRevenge)
		{
			Result.Add(TargetPair.Value);
		}
	}
	return Result;
}

void UMGTakedownSubsystem::ClearRevengeTarget(const FString& TargetId)
{
	if (FMGRevengeTarget* Target = RevengeTargets.Find(TargetId))
	{
		Target->bIsActiveRevenge = false;
		Target->TimesWrecked++;
	}
}

void UMGTakedownSubsystem::StartCrashCamera(const FMGTakedownEvent& Event)
{
	if (bCrashCameraActive)
	{
		return;
	}

	bCrashCameraActive = true;
	CurrentCrashEvent = Event;

	EMGCrashCameraMode SelectedMode = SelectCrashCameraMode(Event);
	CrashCameraConfig.Mode = SelectedMode;

	OnCrashCameraStarted.Broadcast(SelectedMode, CrashCameraConfig.Duration);

	// Set timer to end crash camera
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CrashCameraTimer, this, &UMGTakedownSubsystem::EndCrashCamera, CrashCameraConfig.Duration, false);
	}
}

void UMGTakedownSubsystem::EndCrashCamera()
{
	if (!bCrashCameraActive)
	{
		return;
	}

	bCrashCameraActive = false;
	CurrentCrashEvent = FMGTakedownEvent();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CrashCameraTimer);
	}

	OnCrashCameraEnded.Broadcast();
}

bool UMGTakedownSubsystem::IsCrashCameraActive() const
{
	return bCrashCameraActive;
}

void UMGTakedownSubsystem::SetCrashCameraConfig(const FMGCrashCameraConfig& Config)
{
	CrashCameraConfig = Config;
}

FMGCrashCameraConfig UMGTakedownSubsystem::GetCrashCameraConfig() const
{
	return CrashCameraConfig;
}

void UMGTakedownSubsystem::ApplyAftertouch(FVector Direction)
{
	if (!bCrashCameraActive || !CrashCameraConfig.bEnableAftertouch)
	{
		return;
	}

	// Normalize direction and apply configured force
	Direction.Normalize();
	const float Force = CrashCameraConfig.AftertouchForce;

	// Apply aftertouch to the victim vehicle from the current crash event
	// The actual force application is handled by listeners (e.g., vehicle controller)
	// which can look up the vehicle by ID and apply physics impulse

	if (!CurrentCrashEvent.VictimId.IsEmpty())
	{
		// Broadcast event for vehicle controllers to handle
		OnAftertouchApplied.Broadcast(CurrentCrashEvent.VictimId, Direction, Force);

		// Log for debugging
		UE_LOG(LogTemp, Verbose, TEXT("Aftertouch applied to %s: Direction=(%.2f, %.2f, %.2f), Force=%.0f"),
			*CurrentCrashEvent.VictimId,
			Direction.X, Direction.Y, Direction.Z,
			Force);
	}
}

void UMGTakedownSubsystem::StartSession()
{
	bSessionActive = true;
	SessionStats = FMGTakedownSessionStats();
	RecentTakedowns.Empty();
	EndStreak();

	AggressionState.AggressionMeter = 0.0f;
	AggressionState.Level = EMGAggressionLevel::None;
	AggressionState.RampageTimer = 0.0f;
}

void UMGTakedownSubsystem::EndSession()
{
	bSessionActive = false;
	EndStreak();
	EndCrashCamera();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AggressionTickTimer);
	}

	SaveTakedownData();
}

bool UMGTakedownSubsystem::IsSessionActive() const
{
	return bSessionActive;
}

FMGTakedownSessionStats UMGTakedownSubsystem::GetSessionStats() const
{
	return SessionStats;
}

int32 UMGTakedownSubsystem::GetTotalTakedowns() const
{
	return SessionStats.TotalTakedowns;
}

int32 UMGTakedownSubsystem::GetTotalTimesWrecked() const
{
	return SessionStats.TotalTimesWrecked;
}

int32 UMGTakedownSubsystem::GetBestStreak() const
{
	return SessionStats.BestStreak;
}

TArray<FMGTakedownEvent> UMGTakedownSubsystem::GetRecentTakedowns(int32 Count) const
{
	TArray<FMGTakedownEvent> Result;
	int32 NumToReturn = FMath::Min(Count, RecentTakedowns.Num());
	for (int32 i = 0; i < NumToReturn; ++i)
	{
		Result.Add(RecentTakedowns[i]);
	}
	return Result;
}

FText UMGTakedownSubsystem::GetTakedownDisplayName(EMGTakedownType Type) const
{
	switch (Type)
	{
		case EMGTakedownType::Ram: return FText::FromString(TEXT("RAM!"));
		case EMGTakedownType::Sideswipe: return FText::FromString(TEXT("SIDESWIPE!"));
		case EMGTakedownType::PIT: return FText::FromString(TEXT("PIT MANEUVER!"));
		case EMGTakedownType::Shunt: return FText::FromString(TEXT("SHUNT!"));
		case EMGTakedownType::Slam: return FText::FromString(TEXT("SLAM!"));
		case EMGTakedownType::Grind: return FText::FromString(TEXT("GRIND!"));
		case EMGTakedownType::TrafficCheck: return FText::FromString(TEXT("TRAFFIC CHECK!"));
		case EMGTakedownType::WallGrind: return FText::FromString(TEXT("WALL GRIND!"));
		case EMGTakedownType::AirStrike: return FText::FromString(TEXT("AIR STRIKE!"));
		case EMGTakedownType::Aftertouch: return FText::FromString(TEXT("AFTERTOUCH!"));
		case EMGTakedownType::Revenge: return FText::FromString(TEXT("REVENGE!"));
		case EMGTakedownType::Psyche: return FText::FromString(TEXT("PSYCHE-OUT!"));
		case EMGTakedownType::Signature: return FText::FromString(TEXT("SIGNATURE TAKEDOWN!"));
		default: return FText::FromString(TEXT("TAKEDOWN!"));
	}
}

FLinearColor UMGTakedownSubsystem::GetAggressionColor() const
{
	switch (AggressionState.Level)
	{
		case EMGAggressionLevel::None: return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
		case EMGAggressionLevel::Mild: return FLinearColor(0.5f, 1.0f, 0.5f, 1.0f);
		case EMGAggressionLevel::Moderate: return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
		case EMGAggressionLevel::Aggressive: return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
		case EMGAggressionLevel::Violent: return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
		case EMGAggressionLevel::Rampage: return FLinearColor(1.0f, 0.0f, 0.5f, 1.0f);
		default: return FLinearColor::White;
	}
}

void UMGTakedownSubsystem::SaveTakedownData()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UMGSaveManagerSubsystem* SaveManager = GameInstance->GetSubsystem<UMGSaveManagerSubsystem>();
	if (!SaveManager)
	{
		return;
	}

	UMGSaveGame* SaveGame = SaveManager->GetSaveDataMutable();
	if (!SaveGame)
	{
		return;
	}

	// Save takedown stats
	SaveGame->TakedownData.TotalTakedowns = LifetimeStats.TotalTakedowns;
	SaveGame->TakedownData.PoliceTakedowns = LifetimeStats.PoliceTakedowns;
	SaveGame->TakedownData.RacerTakedowns = LifetimeStats.RacerTakedowns;
	SaveGame->TakedownData.TrafficTakedowns = LifetimeStats.TrafficTakedowns;
	SaveGame->TakedownData.PerfectTakedowns = LifetimeStats.PerfectTakedowns;
	SaveGame->TakedownData.DoubleTakedowns = LifetimeStats.DoubleTakedowns;
	SaveGame->TakedownData.TripleTakedowns = LifetimeStats.TripleTakedowns;
	SaveGame->TakedownData.TotalTakedownScore = LifetimeStats.TotalTakedownScore;
}

void UMGTakedownSubsystem::LoadTakedownData()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UMGSaveManagerSubsystem* SaveManager = GameInstance->GetSubsystem<UMGSaveManagerSubsystem>();
	if (!SaveManager)
	{
		return;
	}

	const UMGSaveGame* SaveGame = SaveManager->GetCurrentSaveData();
	if (!SaveGame)
	{
		return;
	}

	// Load takedown stats
	LifetimeStats.TotalTakedowns = SaveGame->TakedownData.TotalTakedowns;
	LifetimeStats.PoliceTakedowns = SaveGame->TakedownData.PoliceTakedowns;
	LifetimeStats.RacerTakedowns = SaveGame->TakedownData.RacerTakedowns;
	LifetimeStats.TrafficTakedowns = SaveGame->TakedownData.TrafficTakedowns;
	LifetimeStats.PerfectTakedowns = SaveGame->TakedownData.PerfectTakedowns;
	LifetimeStats.DoubleTakedowns = SaveGame->TakedownData.DoubleTakedowns;
	LifetimeStats.TripleTakedowns = SaveGame->TakedownData.TripleTakedowns;
	LifetimeStats.TotalTakedownScore = SaveGame->TakedownData.TotalTakedownScore;
}

void UMGTakedownSubsystem::TickAggression(float DeltaTime)
{
	if (IsRampageActive())
	{
		AggressionState.RampageTimer -= DeltaTime;
		if (AggressionState.RampageTimer <= 0.0f)
		{
			DeactivateRampage();
		}
	}
	else
	{
		// Decay aggression
		AggressionState.AggressionMeter = FMath::Max(0.0f, AggressionState.AggressionMeter - AggressionState.DecayRate * DeltaTime);
		UpdateAggressionLevel();

		if (AggressionState.AggressionMeter <= 0.0f)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(AggressionTickTimer);
			}
		}
	}
}

void UMGTakedownSubsystem::TickStreak(float DeltaTime)
{
	CurrentStreak.StreakTimer -= DeltaTime;

	if (CurrentStreak.StreakTimer <= 0.0f)
	{
		EndStreak();
	}
}

void UMGTakedownSubsystem::UpdateAggressionLevel()
{
	EMGAggressionLevel OldLevel = AggressionState.Level;
	float Percent = GetAggressionPercent();

	if (Percent >= 100.0f)
	{
		ActivateRampage();
	}
	else if (Percent >= 80.0f)
	{
		AggressionState.Level = EMGAggressionLevel::Violent;
	}
	else if (Percent >= 60.0f)
	{
		AggressionState.Level = EMGAggressionLevel::Aggressive;
	}
	else if (Percent >= 40.0f)
	{
		AggressionState.Level = EMGAggressionLevel::Moderate;
	}
	else if (Percent >= 20.0f)
	{
		AggressionState.Level = EMGAggressionLevel::Mild;
	}
	else
	{
		AggressionState.Level = EMGAggressionLevel::None;
	}

	if (OldLevel != AggressionState.Level)
	{
		OnAggressionLevelChanged.Broadcast(OldLevel, AggressionState.Level);
	}
}

void UMGTakedownSubsystem::ActivateRampage()
{
	if (AggressionState.Level == EMGAggressionLevel::Rampage)
	{
		return;
	}

	EMGAggressionLevel OldLevel = AggressionState.Level;
	AggressionState.Level = EMGAggressionLevel::Rampage;
	AggressionState.RampageTimer = AggressionState.RampageDuration;

	OnAggressionLevelChanged.Broadcast(OldLevel, EMGAggressionLevel::Rampage);
	OnRampageActivated.Broadcast(AggressionState.RampageDuration, GetAggressionMultiplier());
}

void UMGTakedownSubsystem::DeactivateRampage()
{
	AggressionState.RampageTimer = 0.0f;
	AggressionState.AggressionMeter = AggressionState.MaxAggression * 0.5f;
	UpdateAggressionLevel();
}

EMGCrashCameraMode UMGTakedownSubsystem::SelectCrashCameraMode(const FMGTakedownEvent& Event) const
{
	// Select based on takedown type and conditions
	if (Event.TakedownType == EMGTakedownType::AirStrike)
	{
		return EMGCrashCameraMode::CinematicChase;
	}

	if (Event.bIsRevenge)
	{
		return EMGCrashCameraMode::ImpactZoom;
	}

	if (Event.ImpactForce > MinTakedownImpactForce * 3.0f)
	{
		return EMGCrashCameraMode::WreckageOrbit;
	}

	if (Event.TakedownType == EMGTakedownType::Aftertouch)
	{
		return EMGCrashCameraMode::Aftertouch;
	}

	return EMGCrashCameraMode::QuickSlowMo;
}

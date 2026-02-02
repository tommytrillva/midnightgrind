// Copyright Epic Games, Inc. All Rights Reserved.

#include "Stunt/MGStuntSubsystem.h"
#include "Save/MGSaveManagerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGStuntSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultConfigs();

	// Default thresholds
	StuntThresholds.MinAirTimeForStunt = 0.5f;
	StuntThresholds.BigAirTime = 2.0f;
	StuntThresholds.MassiveAirTime = 4.0f;
	StuntThresholds.MinHeightForStunt = 1.0f;
	StuntThresholds.BigAirHeight = 5.0f;
	StuntThresholds.MassiveAirHeight = 15.0f;
	StuntThresholds.BarrelRollDegrees = 360.0f;
	StuntThresholds.FlipDegrees = 360.0f;
	StuntThresholds.FlatSpinDegrees = 360.0f;
	StuntThresholds.PerfectLandingAngle = 10.0f;
	StuntThresholds.GoodLandingAngle = 25.0f;
	StuntThresholds.HardLandingAngle = 45.0f;

	// Default combo window
	CurrentCombo.ComboWindow = 5.0f;

	LoadStuntData();
}

void UMGStuntSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboTickTimer);
	}

	SaveStuntData();
	Super::Deinitialize();
}

void UMGStuntSubsystem::InitializeDefaultConfigs()
{
	// Jump
	{
		FMGStuntPointConfig Jump;
		Jump.StuntType = EMGStuntType::Jump;
		Jump.BasePoints = 50;
		Jump.AirTimeMultiplier = 10.0f;
		Jump.HeightMultiplier = 5.0f;
		Jump.DistanceMultiplier = 2.0f;
		Jump.RotationMultiplier = 0.0f;
		Jump.SpeedMultiplier = 0.5f;
		Jump.BoostReward = 5.0f;
		PointConfigs.Add(EMGStuntType::Jump, Jump);
	}

	// Big Air
	{
		FMGStuntPointConfig BigAir;
		BigAir.StuntType = EMGStuntType::BigAir;
		BigAir.BasePoints = 150;
		BigAir.AirTimeMultiplier = 25.0f;
		BigAir.HeightMultiplier = 10.0f;
		BigAir.DistanceMultiplier = 5.0f;
		BigAir.RotationMultiplier = 0.0f;
		BigAir.SpeedMultiplier = 1.0f;
		BigAir.BoostReward = 15.0f;
		PointConfigs.Add(EMGStuntType::BigAir, BigAir);
	}

	// Massive Air
	{
		FMGStuntPointConfig MassiveAir;
		MassiveAir.StuntType = EMGStuntType::MassiveAir;
		MassiveAir.BasePoints = 500;
		MassiveAir.AirTimeMultiplier = 50.0f;
		MassiveAir.HeightMultiplier = 25.0f;
		MassiveAir.DistanceMultiplier = 10.0f;
		MassiveAir.RotationMultiplier = 0.0f;
		MassiveAir.SpeedMultiplier = 2.0f;
		MassiveAir.BoostReward = 50.0f;
		PointConfigs.Add(EMGStuntType::MassiveAir, MassiveAir);
	}

	// Barrel Roll
	{
		FMGStuntPointConfig BarrelRoll;
		BarrelRoll.StuntType = EMGStuntType::BarrelRoll;
		BarrelRoll.BasePoints = 200;
		BarrelRoll.AirTimeMultiplier = 5.0f;
		BarrelRoll.HeightMultiplier = 2.0f;
		BarrelRoll.DistanceMultiplier = 1.0f;
		BarrelRoll.RotationMultiplier = 50.0f;
		BarrelRoll.SpeedMultiplier = 0.5f;
		BarrelRoll.BoostReward = 20.0f;
		PointConfigs.Add(EMGStuntType::BarrelRoll, BarrelRoll);
	}

	// Flip
	{
		FMGStuntPointConfig Flip;
		Flip.StuntType = EMGStuntType::Flip;
		Flip.BasePoints = 300;
		Flip.AirTimeMultiplier = 5.0f;
		Flip.HeightMultiplier = 3.0f;
		Flip.DistanceMultiplier = 1.0f;
		Flip.RotationMultiplier = 75.0f;
		Flip.SpeedMultiplier = 0.5f;
		Flip.BoostReward = 30.0f;
		PointConfigs.Add(EMGStuntType::Flip, Flip);
	}

	// Flat Spin
	{
		FMGStuntPointConfig FlatSpin;
		FlatSpin.StuntType = EMGStuntType::FlatSpin;
		FlatSpin.BasePoints = 175;
		FlatSpin.AirTimeMultiplier = 5.0f;
		FlatSpin.HeightMultiplier = 2.0f;
		FlatSpin.DistanceMultiplier = 1.0f;
		FlatSpin.RotationMultiplier = 35.0f;
		FlatSpin.SpeedMultiplier = 0.5f;
		FlatSpin.BoostReward = 17.5f;
		PointConfigs.Add(EMGStuntType::FlatSpin, FlatSpin);
	}

	// Corkscrew
	{
		FMGStuntPointConfig Corkscrew;
		Corkscrew.StuntType = EMGStuntType::Corkscrew;
		Corkscrew.BasePoints = 400;
		Corkscrew.AirTimeMultiplier = 10.0f;
		Corkscrew.HeightMultiplier = 5.0f;
		Corkscrew.DistanceMultiplier = 2.0f;
		Corkscrew.RotationMultiplier = 100.0f;
		Corkscrew.SpeedMultiplier = 1.0f;
		Corkscrew.BoostReward = 40.0f;
		PointConfigs.Add(EMGStuntType::Corkscrew, Corkscrew);
	}

	// Two Wheels
	{
		FMGStuntPointConfig TwoWheels;
		TwoWheels.StuntType = EMGStuntType::TwoWheels;
		TwoWheels.BasePoints = 25;
		TwoWheels.AirTimeMultiplier = 0.0f;
		TwoWheels.HeightMultiplier = 0.0f;
		TwoWheels.DistanceMultiplier = 5.0f;
		TwoWheels.RotationMultiplier = 0.0f;
		TwoWheels.SpeedMultiplier = 0.25f;
		TwoWheels.BoostReward = 2.5f;
		PointConfigs.Add(EMGStuntType::TwoWheels, TwoWheels);
	}

	// Perfect Landing
	{
		FMGStuntPointConfig PerfectLanding;
		PerfectLanding.StuntType = EMGStuntType::PerfectLanding;
		PerfectLanding.BasePoints = 100;
		PerfectLanding.AirTimeMultiplier = 0.0f;
		PerfectLanding.HeightMultiplier = 0.0f;
		PerfectLanding.DistanceMultiplier = 0.0f;
		PerfectLanding.RotationMultiplier = 0.0f;
		PerfectLanding.SpeedMultiplier = 0.0f;
		PerfectLanding.BoostReward = 10.0f;
		PointConfigs.Add(EMGStuntType::PerfectLanding, PerfectLanding);
	}

	// Signature Stunt
	{
		FMGStuntPointConfig Signature;
		Signature.StuntType = EMGStuntType::Signature;
		Signature.BasePoints = 1000;
		Signature.AirTimeMultiplier = 50.0f;
		Signature.HeightMultiplier = 25.0f;
		Signature.DistanceMultiplier = 20.0f;
		Signature.RotationMultiplier = 100.0f;
		Signature.SpeedMultiplier = 2.0f;
		Signature.BoostReward = 100.0f;
		PointConfigs.Add(EMGStuntType::Signature, Signature);
	}
}

void UMGStuntSubsystem::NotifyLaunch(FVector Position, FVector Velocity, FRotator Rotation, bool bWasDrifting)
{
	if (!bSessionActive)
	{
		return;
	}

	ActiveAirState = FMGActiveAirState();
	ActiveAirState.bIsAirborne = true;
	ActiveAirState.LaunchPosition = Position;
	ActiveAirState.LaunchVelocity = Velocity;
	ActiveAirState.LaunchRotation = Rotation;
	ActiveAirState.CurrentRotation = Rotation;
	ActiveAirState.bWasDrifting = bWasDrifting;
	ActiveAirState.LaunchTime = FDateTime::Now();

	LastReportedRolls = 0;
	LastReportedFlips = 0;
	LastReportedSpins = 0;

	OnStuntStarted.Broadcast(EMGStuntType::Jump, Position);
}

void UMGStuntSubsystem::UpdateAirState(FVector CurrentPosition, FRotator CurrentRotation, float DeltaTime)
{
	if (!ActiveAirState.bIsAirborne)
	{
		return;
	}

	// Update time and height
	ActiveAirState.CurrentAirTime += DeltaTime;
	ActiveAirState.CurrentHeight = CurrentPosition.Z - ActiveAirState.LaunchPosition.Z;

	if (ActiveAirState.CurrentHeight > ActiveAirState.MaxHeight)
	{
		ActiveAirState.MaxHeight = ActiveAirState.CurrentHeight;
	}

	// Calculate rotation deltas
	FRotator DeltaRotation = CurrentRotation - ActiveAirState.CurrentRotation;
	DeltaRotation.Normalize();

	ActiveAirState.AccumulatedPitch += FMath::Abs(DeltaRotation.Pitch);
	ActiveAirState.AccumulatedRoll += FMath::Abs(DeltaRotation.Roll);
	ActiveAirState.AccumulatedYaw += FMath::Abs(DeltaRotation.Yaw);

	ActiveAirState.CurrentRotation = CurrentRotation;

	// Check for rotation milestones
	CheckRotationMilestones();
}

void UMGStuntSubsystem::NotifyLanding(FVector Position, FVector Velocity, FRotator Rotation)
{
	if (!ActiveAirState.bIsAirborne)
	{
		return;
	}

	// Finalize stunt
	FMGStuntEvent Event = FinalizeStunt();
	Event.LandingLocation = Position;
	Event.LandingSpeed = Velocity.Size() * 0.036f;

	// Calculate landing state
	Event.Landing = CalculateLandingState(ActiveAirState.LaunchRotation, Rotation, Velocity);

	// Calculate landing bonus
	int32 LandingBonus = CalculateLandingBonus(Event.Landing, Event.BasePoints);
	Event.BonusPoints += LandingBonus;
	Event.TotalPoints = Event.BasePoints + Event.BonusPoints;

	// Broadcast landing event
	OnLanding.Broadcast(Event.Landing, LandingBonus);

	// Only record stunt if it met minimum thresholds
	if (Event.AirTime >= StuntThresholds.MinAirTimeForStunt || Event.MaxHeight >= StuntThresholds.MinHeightForStunt)
	{
		// Update session stats
		SessionStats.TotalStunts++;
		SessionStats.TotalPoints += Event.TotalPoints;
		SessionStats.TotalAirTime += Event.AirTime;

		if (Event.TotalPoints > SessionStats.BestSingleStunt)
		{
			SessionStats.BestSingleStunt = Event.TotalPoints;
		}
		if (Event.Distance > SessionStats.LongestJump)
		{
			SessionStats.LongestJump = Event.Distance;
		}
		if (Event.MaxHeight > SessionStats.HighestJump)
		{
			SessionStats.HighestJump = Event.MaxHeight;
		}
		if (Event.TotalRotation > SessionStats.MostRotation)
		{
			SessionStats.MostRotation = Event.TotalRotation;
		}

		if (Event.RotationsX > 0)
		{
			SessionStats.TotalBarrelRolls += Event.RotationsX;
		}
		if (Event.RotationsY > 0)
		{
			SessionStats.TotalFlips += Event.RotationsY;
		}

		if (Event.Landing == EMGLandingState::Perfect)
		{
			SessionStats.PerfectLandings++;
		}
		else if (Event.Landing == EMGLandingState::Crash || Event.Landing == EMGLandingState::Rollover)
		{
			SessionStats.CrashLandings++;
		}

		int32& TypeCount = SessionStats.StuntsByType.FindOrAdd(Event.StuntType);
		TypeCount++;

		// Store recent stunt
		RecentStunts.Insert(Event, 0);
		if (RecentStunts.Num() > MaxRecentStunts)
		{
			RecentStunts.SetNum(MaxRecentStunts);
		}

		// Extend combo
		ExtendCombo(Event);

		OnStuntCompleted.Broadcast(Event, SessionStats.TotalPoints);
	}
	else if (Event.Landing == EMGLandingState::Crash || Event.Landing == EMGLandingState::Rollover)
	{
		OnStuntFailed.Broadcast(TEXT("Crash Landing"));
		LoseCombo();
	}

	// Reset air state
	ActiveAirState = FMGActiveAirState();
}

void UMGStuntSubsystem::NotifyNearMissWhileAirborne()
{
	if (ActiveAirState.bIsAirborne)
	{
		ActiveAirState.NearMissCount++;
	}
}

void UMGStuntSubsystem::NotifyOncomingWhileAirborne()
{
	if (ActiveAirState.bIsAirborne)
	{
		ActiveAirState.OncomingCount++;
	}
}

bool UMGStuntSubsystem::IsAirborne() const
{
	return ActiveAirState.bIsAirborne;
}

FMGActiveAirState UMGStuntSubsystem::GetActiveAirState() const
{
	return ActiveAirState;
}

float UMGStuntSubsystem::GetCurrentAirTime() const
{
	return ActiveAirState.CurrentAirTime;
}

float UMGStuntSubsystem::GetCurrentHeight() const
{
	return ActiveAirState.CurrentHeight;
}

void UMGStuntSubsystem::StartTwoWheelDriving(bool bLeftSide, float TiltAngle)
{
	if (!bSessionActive || TwoWheelState.bActive)
	{
		return;
	}

	TwoWheelState = FMGTwoWheelState();
	TwoWheelState.bActive = true;
	TwoWheelState.bIsLeftSide = bLeftSide;
	TwoWheelState.TiltAngle = TiltAngle;

	OnTwoWheelStarted.Broadcast(bLeftSide, TiltAngle);
}

void UMGStuntSubsystem::UpdateTwoWheelDriving(float Distance, float TiltAngle, float DeltaTime)
{
	if (!TwoWheelState.bActive)
	{
		return;
	}

	TwoWheelState.Duration += DeltaTime;
	TwoWheelState.Distance += Distance;
	TwoWheelState.TiltAngle = TiltAngle;

	// Calculate points per second
	FMGStuntPointConfig Config = GetStuntPointConfig(EMGStuntType::TwoWheels);
	int32 PointsThisFrame = FMath::RoundToInt(Config.BasePoints * DeltaTime);
	PointsThisFrame += FMath::RoundToInt(Distance * Config.DistanceMultiplier);
	TwoWheelState.AccumulatedPoints += PointsThisFrame;
}

void UMGStuntSubsystem::EndTwoWheelDriving()
{
	if (!TwoWheelState.bActive)
	{
		return;
	}

	int32 FinalPoints = TwoWheelState.AccumulatedPoints;

	// Create stunt event for combo
	if (TwoWheelState.Duration >= 1.0f)
	{
		FMGStuntEvent Event;
		Event.EventId = FGuid::NewGuid().ToString();
		Event.StuntType = EMGStuntType::TwoWheels;
		Event.AirTime = 0.0f;
		Event.Distance = TwoWheelState.Distance;
		Event.BasePoints = FinalPoints;
		Event.TotalPoints = FinalPoints;
		Event.Timestamp = FDateTime::Now();

		SessionStats.TotalStunts++;
		SessionStats.TotalPoints += FinalPoints;

		ExtendCombo(Event);
	}

	OnTwoWheelEnded.Broadcast(TwoWheelState.Duration, TwoWheelState.Distance, FinalPoints);

	TwoWheelState = FMGTwoWheelState();
}

bool UMGStuntSubsystem::IsTwoWheelDriving() const
{
	return TwoWheelState.bActive;
}

FMGTwoWheelState UMGStuntSubsystem::GetTwoWheelState() const
{
	return TwoWheelState;
}

TArray<EMGStuntType> UMGStuntSubsystem::DetectStuntsFromAirState() const
{
	TArray<EMGStuntType> DetectedStunts;

	if (!ActiveAirState.bIsAirborne)
	{
		return DetectedStunts;
	}

	// Air time based
	if (ActiveAirState.CurrentAirTime >= StuntThresholds.MassiveAirTime)
	{
		DetectedStunts.Add(EMGStuntType::MassiveAir);
	}
	else if (ActiveAirState.CurrentAirTime >= StuntThresholds.BigAirTime)
	{
		DetectedStunts.Add(EMGStuntType::BigAir);
	}
	else if (ActiveAirState.CurrentAirTime >= StuntThresholds.MinAirTimeForStunt)
	{
		DetectedStunts.Add(EMGStuntType::Jump);
	}

	// Rotation based
	int32 Rolls = CountFullRotations(ActiveAirState.AccumulatedRoll);
	int32 Flips = CountFullRotations(ActiveAirState.AccumulatedPitch);
	int32 Spins = CountFullRotations(ActiveAirState.AccumulatedYaw);

	if (Rolls > 0 && Flips > 0)
	{
		DetectedStunts.Add(EMGStuntType::Corkscrew);
	}
	else if (Rolls > 0)
	{
		DetectedStunts.Add(EMGStuntType::BarrelRoll);
	}
	else if (Flips > 0)
	{
		DetectedStunts.Add(EMGStuntType::Flip);
	}
	else if (Spins > 0)
	{
		DetectedStunts.Add(EMGStuntType::FlatSpin);
	}

	// Near miss while airborne
	if (ActiveAirState.NearMissCount > 0)
	{
		DetectedStunts.Add(EMGStuntType::NearMissAir);
	}

	// Oncoming while airborne
	if (ActiveAirState.OncomingCount > 0)
	{
		DetectedStunts.Add(EMGStuntType::OncomingAir);
	}

	// Drift jump
	if (ActiveAirState.bWasDrifting)
	{
		DetectedStunts.Add(EMGStuntType::DriftJump);
	}

	return DetectedStunts;
}

EMGStuntQuality UMGStuntSubsystem::CalculateStuntQuality(const FMGStuntEvent& Event) const
{
	int32 QualityScore = 0;

	// Air time contribution
	if (Event.AirTime >= StuntThresholds.MassiveAirTime)
	{
		QualityScore += 3;
	}
	else if (Event.AirTime >= StuntThresholds.BigAirTime)
	{
		QualityScore += 2;
	}
	else if (Event.AirTime >= 1.0f)
	{
		QualityScore += 1;
	}

	// Height contribution
	if (Event.MaxHeight >= StuntThresholds.MassiveAirHeight)
	{
		QualityScore += 3;
	}
	else if (Event.MaxHeight >= StuntThresholds.BigAirHeight)
	{
		QualityScore += 2;
	}
	else if (Event.MaxHeight >= 2.0f)
	{
		QualityScore += 1;
	}

	// Rotation contribution
	int32 TotalRotations = Event.RotationsX + Event.RotationsY + Event.RotationsZ;
	QualityScore += FMath::Min(3, TotalRotations);

	// Bonuses
	if (Event.bHadNearMiss)
	{
		QualityScore += 1;
	}
	if (Event.bHadOncoming)
	{
		QualityScore += 2;
	}
	if (Event.Landing == EMGLandingState::Perfect)
	{
		QualityScore += 2;
	}

	// Determine quality
	if (QualityScore >= 12)
	{
		return EMGStuntQuality::Legendary;
	}
	if (QualityScore >= 9)
	{
		return EMGStuntQuality::Incredible;
	}
	if (QualityScore >= 6)
	{
		return EMGStuntQuality::Awesome;
	}
	if (QualityScore >= 4)
	{
		return EMGStuntQuality::Great;
	}
	if (QualityScore >= 2)
	{
		return EMGStuntQuality::Good;
	}
	return EMGStuntQuality::Basic;
}

EMGLandingState UMGStuntSubsystem::CalculateLandingState(FRotator LaunchRotation, FRotator LandingRotation, FVector LandingVelocity) const
{
	// Calculate angular difference
	FRotator DeltaRotation = LandingRotation - LaunchRotation;
	DeltaRotation.Normalize();

	float TotalAngleOff = FMath::Abs(DeltaRotation.Pitch) + FMath::Abs(DeltaRotation.Roll);

	// Check for rollover
	if (FMath::Abs(DeltaRotation.Roll) > 90.0f || FMath::Abs(DeltaRotation.Pitch) > 90.0f)
	{
		return EMGLandingState::Rollover;
	}

	// Check landing angle
	if (TotalAngleOff <= StuntThresholds.PerfectLandingAngle)
	{
		return EMGLandingState::Perfect;
	}
	if (TotalAngleOff <= StuntThresholds.GoodLandingAngle)
	{
		return EMGLandingState::Good;
	}
	if (TotalAngleOff <= StuntThresholds.HardLandingAngle)
	{
		return EMGLandingState::Hard;
	}

	return EMGLandingState::Crash;
}

int32 UMGStuntSubsystem::CalculateStuntPoints(const FMGStuntEvent& Event) const
{
	FMGStuntPointConfig Config = GetStuntPointConfig(Event.StuntType);

	float Points = static_cast<float>(Config.BasePoints);

	// Air time bonus
	Points += Event.AirTime * Config.AirTimeMultiplier;

	// Height bonus
	Points += Event.MaxHeight * Config.HeightMultiplier;

	// Distance bonus
	Points += Event.Distance * Config.DistanceMultiplier;

	// Rotation bonus
	Points += Event.TotalRotation * Config.RotationMultiplier;

	// Speed bonus
	Points += Event.LaunchSpeed * Config.SpeedMultiplier;

	return FMath::RoundToInt(Points);
}

int32 UMGStuntSubsystem::CalculateLandingBonus(EMGLandingState Landing, int32 BasePoints) const
{
	switch (Landing)
	{
		case EMGLandingState::Perfect:
			return FMath::RoundToInt(BasePoints * 0.5f);
		case EMGLandingState::Good:
			return FMath::RoundToInt(BasePoints * 0.25f);
		case EMGLandingState::Hard:
			return 0;
		case EMGLandingState::Crash:
		case EMGLandingState::Rollover:
			return -FMath::RoundToInt(BasePoints * 0.5f);
		default:
			return 0;
	}
}

float UMGStuntSubsystem::CalculateBoostReward(const FMGStuntEvent& Event) const
{
	FMGStuntPointConfig Config = GetStuntPointConfig(Event.StuntType);
	float Boost = Config.BoostReward;

	// Quality multiplier
	switch (Event.Quality)
	{
		case EMGStuntQuality::Legendary: Boost *= 3.0f; break;
		case EMGStuntQuality::Incredible: Boost *= 2.5f; break;
		case EMGStuntQuality::Awesome: Boost *= 2.0f; break;
		case EMGStuntQuality::Great: Boost *= 1.5f; break;
		case EMGStuntQuality::Good: Boost *= 1.25f; break;
		default: break;
	}

	// Landing bonus
	if (Event.Landing == EMGLandingState::Perfect)
	{
		Boost *= 1.5f;
	}
	else if (Event.Landing == EMGLandingState::Crash || Event.Landing == EMGLandingState::Rollover)
	{
		Boost = 0.0f;
	}

	return Boost;
}

void UMGStuntSubsystem::SetStuntPointConfig(EMGStuntType StuntType, const FMGStuntPointConfig& Config)
{
	PointConfigs.Add(StuntType, Config);
}

FMGStuntPointConfig UMGStuntSubsystem::GetStuntPointConfig(EMGStuntType StuntType) const
{
	if (const FMGStuntPointConfig* Config = PointConfigs.Find(StuntType))
	{
		return *Config;
	}

	// Return Jump as default
	if (const FMGStuntPointConfig* Default = PointConfigs.Find(EMGStuntType::Jump))
	{
		return *Default;
	}

	return FMGStuntPointConfig();
}

void UMGStuntSubsystem::SetThresholds(const FMGStuntThresholds& Thresholds)
{
	StuntThresholds = Thresholds;
}

FMGStuntThresholds UMGStuntSubsystem::GetThresholds() const
{
	return StuntThresholds;
}

void UMGStuntSubsystem::ExtendCombo(const FMGStuntEvent& Event)
{
	CurrentCombo.ComboCount++;
	CurrentCombo.TotalPoints += Event.TotalPoints;
	CurrentCombo.ComboEvents.Add(Event);
	CurrentCombo.TimeRemaining = CurrentCombo.ComboWindow;

	// Calculate multiplier (increases with variety)
	TSet<EMGStuntType> UniqueTypes;
	for (const FMGStuntEvent& ComboEvent : CurrentCombo.ComboEvents)
	{
		UniqueTypes.Add(ComboEvent.StuntType);
	}
	CurrentCombo.UniqueStuntTypes = UniqueTypes.Num();

	// Base multiplier + variety bonus
	CurrentCombo.Multiplier = 1.0f + (CurrentCombo.ComboCount - 1) * 0.1f;
	CurrentCombo.Multiplier += (CurrentCombo.UniqueStuntTypes - 1) * 0.15f;
	CurrentCombo.Multiplier = FMath::Min(CurrentCombo.Multiplier, 5.0f);

	if (CurrentCombo.ComboCount > SessionStats.BestCombo)
	{
		SessionStats.BestCombo = CurrentCombo.ComboCount;
	}

	// Start combo timer
	if (CurrentCombo.ComboCount == 1)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ComboTickTimer, FTimerDelegate::CreateUObject(this, &UMGStuntSubsystem::TickCombo, 0.1f), 0.1f, true);
		}
	}

	OnComboUpdated.Broadcast(CurrentCombo.ComboCount, CurrentCombo.Multiplier);
}

void UMGStuntSubsystem::BankCombo()
{
	if (CurrentCombo.ComboCount == 0)
	{
		return;
	}

	int32 FinalPoints = FMath::RoundToInt(CurrentCombo.TotalPoints * CurrentCombo.Multiplier);
	OnComboBanked.Broadcast(CurrentCombo.ComboCount, FinalPoints);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboTickTimer);
	}

	CurrentCombo = FMGStuntCombo();
	CurrentCombo.ComboWindow = 5.0f;
}

void UMGStuntSubsystem::LoseCombo()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboTickTimer);
	}

	CurrentCombo = FMGStuntCombo();
	CurrentCombo.ComboWindow = 5.0f;
}

FMGStuntCombo UMGStuntSubsystem::GetCurrentCombo() const
{
	return CurrentCombo;
}

bool UMGStuntSubsystem::IsComboActive() const
{
	return CurrentCombo.ComboCount > 0 && CurrentCombo.TimeRemaining > 0.0f;
}

void UMGStuntSubsystem::RegisterStuntZone(const FMGStuntZone& Zone)
{
	if (!Zone.ZoneId.IsEmpty())
	{
		StuntZones.Add(Zone.ZoneId, Zone);
	}
}

FMGStuntZone UMGStuntSubsystem::GetStuntZone(const FString& ZoneId) const
{
	if (const FMGStuntZone* Zone = StuntZones.Find(ZoneId))
	{
		return *Zone;
	}
	return FMGStuntZone();
}

FMGStuntZone UMGStuntSubsystem::GetNearestStuntZone(FVector Location) const
{
	FMGStuntZone Nearest;
	float NearestDist = FLT_MAX;

	for (const auto& ZonePair : StuntZones)
	{
		float Dist = FVector::Dist(Location, ZonePair.Value.Location);
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			Nearest = ZonePair.Value;
		}
	}

	return Nearest;
}

bool UMGStuntSubsystem::IsInStuntZone(FVector Location, FString& OutZoneId) const
{
	for (const auto& ZonePair : StuntZones)
	{
		float Dist = FVector::Dist(Location, ZonePair.Value.Location);
		if (Dist <= ZonePair.Value.Radius)
		{
			OutZoneId = ZonePair.Key;
			return true;
		}
	}
	return false;
}

void UMGStuntSubsystem::UpdateStuntZoneBestScore(const FString& ZoneId, int32 NewScore)
{
	if (FMGStuntZone* Zone = StuntZones.Find(ZoneId))
	{
		if (NewScore > Zone->BestScore)
		{
			Zone->BestScore = NewScore;
		}
	}
}

void UMGStuntSubsystem::StartSession()
{
	bSessionActive = true;
	SessionStats = FMGStuntSessionStats();
	RecentStunts.Empty();
	LoseCombo();
	ActiveAirState = FMGActiveAirState();
	TwoWheelState = FMGTwoWheelState();
}

void UMGStuntSubsystem::EndSession()
{
	bSessionActive = false;
	BankCombo();
	SaveStuntData();
}

bool UMGStuntSubsystem::IsSessionActive() const
{
	return bSessionActive;
}

FMGStuntSessionStats UMGStuntSubsystem::GetSessionStats() const
{
	return SessionStats;
}

int32 UMGStuntSubsystem::GetTotalStuntPoints() const
{
	return SessionStats.TotalPoints;
}

int32 UMGStuntSubsystem::GetTotalStunts() const
{
	return SessionStats.TotalStunts;
}

TArray<FMGStuntEvent> UMGStuntSubsystem::GetRecentStunts(int32 Count) const
{
	TArray<FMGStuntEvent> Result;
	int32 NumToReturn = FMath::Min(Count, RecentStunts.Num());
	for (int32 i = 0; i < NumToReturn; ++i)
	{
		Result.Add(RecentStunts[i]);
	}
	return Result;
}

FText UMGStuntSubsystem::GetStuntDisplayName(EMGStuntType StuntType) const
{
	switch (StuntType)
	{
		case EMGStuntType::Jump: return FText::FromString(TEXT("JUMP!"));
		case EMGStuntType::BigAir: return FText::FromString(TEXT("BIG AIR!"));
		case EMGStuntType::MassiveAir: return FText::FromString(TEXT("MASSIVE AIR!"));
		case EMGStuntType::BarrelRoll: return FText::FromString(TEXT("BARREL ROLL!"));
		case EMGStuntType::Corkscrew: return FText::FromString(TEXT("CORKSCREW!"));
		case EMGStuntType::Flip: return FText::FromString(TEXT("FLIP!"));
		case EMGStuntType::FlatSpin: return FText::FromString(TEXT("FLAT SPIN!"));
		case EMGStuntType::TwoWheels: return FText::FromString(TEXT("TWO WHEELS!"));
		case EMGStuntType::NearMissAir: return FText::FromString(TEXT("NEAR MISS AIR!"));
		case EMGStuntType::DriftJump: return FText::FromString(TEXT("DRIFT JUMP!"));
		case EMGStuntType::OncomingAir: return FText::FromString(TEXT("ONCOMING AIR!"));
		case EMGStuntType::Hangtime: return FText::FromString(TEXT("HANGTIME!"));
		case EMGStuntType::PerfectLanding: return FText::FromString(TEXT("PERFECT LANDING!"));
		case EMGStuntType::Signature: return FText::FromString(TEXT("SIGNATURE STUNT!"));
		default: return FText::FromString(TEXT("STUNT!"));
	}
}

FText UMGStuntSubsystem::GetQualityDisplayName(EMGStuntQuality Quality) const
{
	switch (Quality)
	{
		case EMGStuntQuality::Basic: return FText::FromString(TEXT(""));
		case EMGStuntQuality::Good: return FText::FromString(TEXT("Good"));
		case EMGStuntQuality::Great: return FText::FromString(TEXT("Great"));
		case EMGStuntQuality::Awesome: return FText::FromString(TEXT("Awesome"));
		case EMGStuntQuality::Incredible: return FText::FromString(TEXT("Incredible"));
		case EMGStuntQuality::Legendary: return FText::FromString(TEXT("LEGENDARY"));
		default: return FText::GetEmpty();
	}
}

FLinearColor UMGStuntSubsystem::GetQualityColor(EMGStuntQuality Quality) const
{
	switch (Quality)
	{
		case EMGStuntQuality::Basic: return FLinearColor::White;
		case EMGStuntQuality::Good: return FLinearColor(0.5f, 1.0f, 0.5f, 1.0f);
		case EMGStuntQuality::Great: return FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);
		case EMGStuntQuality::Awesome: return FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
		case EMGStuntQuality::Incredible: return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
		case EMGStuntQuality::Legendary: return FLinearColor(1.0f, 0.0f, 0.5f, 1.0f);
		default: return FLinearColor::White;
	}
}

void UMGStuntSubsystem::SaveStuntData()
{
	// Save is handled centrally by MGSaveManagerSubsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			SaveManager->QuickSave();
		}
	}
}

void UMGStuntSubsystem::LoadStuntData()
{
	// Load stunt data from central save manager
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			if (const UMGSaveGame* SaveData = SaveManager->GetCurrentSaveData())
			{
				// Restore career stats from saved data
				SessionStats.TotalStunts = SaveData->StuntData.TotalStunts;
				SessionStats.TotalPoints = SaveData->StuntData.TotalStuntScore;
				SessionStats.BestCombo = SaveData->StuntData.StuntComboMax;
				SessionStats.LongestJump = SaveData->StuntData.LongestJump;
				SessionStats.HighestAir = SaveData->StuntData.HighestAirTime;
				UE_LOG(LogTemp, Log, TEXT("StuntSubsystem: Loaded stunt data - TotalStunts: %d, Score: %lld"),
					SaveData->StuntData.TotalStunts, SaveData->StuntData.TotalStuntScore);
			}
		}
	}
}

FMGStuntEvent UMGStuntSubsystem::FinalizeStunt()
{
	FMGStuntEvent Event;
	Event.EventId = FGuid::NewGuid().ToString();
	Event.AirTime = ActiveAirState.CurrentAirTime;
	Event.MaxHeight = ActiveAirState.MaxHeight;
	Event.Distance = FVector::Dist2D(ActiveAirState.LaunchPosition, ActiveAirState.LaunchPosition + ActiveAirState.LaunchVelocity * ActiveAirState.CurrentAirTime);
	Event.LaunchSpeed = ActiveAirState.LaunchVelocity.Size() * 0.036f;
	Event.LaunchLocation = ActiveAirState.LaunchPosition;
	Event.Timestamp = ActiveAirState.LaunchTime;
	Event.bIsDrifting = ActiveAirState.bWasDrifting;
	Event.bHadNearMiss = ActiveAirState.NearMissCount > 0;
	Event.bHadOncoming = ActiveAirState.OncomingCount > 0;

	// Count rotations
	Event.RotationsX = CountFullRotations(ActiveAirState.AccumulatedRoll);
	Event.RotationsY = CountFullRotations(ActiveAirState.AccumulatedPitch);
	Event.RotationsZ = CountFullRotations(ActiveAirState.AccumulatedYaw);
	Event.TotalRotation = ActiveAirState.AccumulatedPitch + ActiveAirState.AccumulatedRoll + ActiveAirState.AccumulatedYaw;

	// Determine stunt type
	TArray<EMGStuntType> DetectedStunts = DetectStuntsFromAirState();

	if (DetectedStunts.Num() > 0)
	{
		// Pick highest value stunt
		if (DetectedStunts.Contains(EMGStuntType::Corkscrew))
		{
			Event.StuntType = EMGStuntType::Corkscrew;
		}
		else if (DetectedStunts.Contains(EMGStuntType::MassiveAir))
		{
			Event.StuntType = EMGStuntType::MassiveAir;
		}
		else if (DetectedStunts.Contains(EMGStuntType::Flip))
		{
			Event.StuntType = EMGStuntType::Flip;
		}
		else if (DetectedStunts.Contains(EMGStuntType::BarrelRoll))
		{
			Event.StuntType = EMGStuntType::BarrelRoll;
		}
		else if (DetectedStunts.Contains(EMGStuntType::BigAir))
		{
			Event.StuntType = EMGStuntType::BigAir;
		}
		else if (DetectedStunts.Contains(EMGStuntType::FlatSpin))
		{
			Event.StuntType = EMGStuntType::FlatSpin;
		}
		else
		{
			Event.StuntType = DetectedStunts[0];
		}

		// Add bonus tags
		for (EMGStuntType Stunt : DetectedStunts)
		{
			if (Stunt != Event.StuntType)
			{
				Event.BonusTags.Add(GetStuntDisplayName(Stunt).ToString());
			}
		}
	}
	else
	{
		Event.StuntType = EMGStuntType::Jump;
	}

	// Calculate points
	Event.BasePoints = CalculateStuntPoints(Event);
	Event.Quality = CalculateStuntQuality(Event);
	Event.BoostReward = CalculateBoostReward(Event);

	return Event;
}

void UMGStuntSubsystem::CheckRotationMilestones()
{
	int32 CurrentRolls = CountFullRotations(ActiveAirState.AccumulatedRoll);
	int32 CurrentFlips = CountFullRotations(ActiveAirState.AccumulatedPitch);
	int32 CurrentSpins = CountFullRotations(ActiveAirState.AccumulatedYaw);

	if (CurrentRolls > LastReportedRolls)
	{
		int32 Points = 200 * CurrentRolls;
		OnRotationMilestone.Broadcast(EMGStuntType::BarrelRoll, CurrentRolls, Points);
		LastReportedRolls = CurrentRolls;
	}

	if (CurrentFlips > LastReportedFlips)
	{
		int32 Points = 300 * CurrentFlips;
		OnRotationMilestone.Broadcast(EMGStuntType::Flip, CurrentFlips, Points);
		LastReportedFlips = CurrentFlips;
	}

	if (CurrentSpins > LastReportedSpins)
	{
		int32 Points = 175 * CurrentSpins;
		OnRotationMilestone.Broadcast(EMGStuntType::FlatSpin, CurrentSpins, Points);
		LastReportedSpins = CurrentSpins;
	}
}

void UMGStuntSubsystem::TickCombo(float DeltaTime)
{
	if (CurrentCombo.ComboCount == 0)
	{
		return;
	}

	CurrentCombo.TimeRemaining -= DeltaTime;

	if (CurrentCombo.TimeRemaining <= 0.0f)
	{
		BankCombo();
	}
}

int32 UMGStuntSubsystem::CountFullRotations(float Degrees) const
{
	return FMath::FloorToInt(FMath::Abs(Degrees) / 360.0f);
}

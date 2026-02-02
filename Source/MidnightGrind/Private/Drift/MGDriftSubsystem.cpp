// Copyright Midnight Grind. All Rights Reserved.

#include "Drift/MGDriftSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UMGDriftSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default config
	Config.MinDriftAngle = 15.0f;
	Config.MinDriftSpeed = 50.0f;
	Config.ChainTimeWindow = 2.0f;
	Config.BasePointsPerSecond = 100.0f;
	Config.AngleMultiplierScale = 0.02f;
	Config.SpeedMultiplierScale = 0.01f;
	Config.WallProximityBonusDistance = 100.0f;
	Config.WallProximityMultiplier = 1.5f;
	Config.TandemBonusDistance = 300.0f;
	Config.TandemMultiplier = 2.0f;
	Config.SSSThreshold = 100000;
	Config.SSThreshold = 50000;
	Config.SThreshold = 25000;
	Config.AThreshold = 10000;
	Config.BThreshold = 5000;
	Config.CThreshold = 2500;
	Config.DThreshold = 1000;

	LoadDriftData();
}

void UMGDriftSubsystem::Deinitialize()
{
	SaveDriftData();
	Super::Deinitialize();
}

bool UMGDriftSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Core Drift Functions
// ============================================================================

void UMGDriftSubsystem::UpdateDriftState(float MGDeltaTime, float SlipAngle, float Speed, const FVector& Position, const FVector& Velocity)
{
	float AbsSlipAngle = FMath::Abs(SlipAngle);

	// Check if we should be drifting
	bool bShouldBeDrifting = AbsSlipAngle >= Config.MinDriftAngle && Speed >= Config.MinDriftSpeed;

	if (bShouldBeDrifting && !ActiveDrift.bIsDrifting)
	{
		StartDrift();
	}
	else if (!bShouldBeDrifting && ActiveDrift.bIsDrifting)
	{
		EndDrift(false);
		return;
	}

	if (!ActiveDrift.bIsDrifting)
	{
		return;
	}

	// Update drift state
	ActiveDrift.CurrentAngle = AbsSlipAngle;
	ActiveDrift.CurrentSpeed = Speed;
	ActiveDrift.DriftDuration += DeltaTime;
	ActiveDrift.DriftDistance += Speed * DeltaTime * 0.01f; // Convert to meters

	// Track max values
	MaxAngleThisDrift = FMath::Max(MaxAngleThisDrift, AbsSlipAngle);
	MaxSpeedThisDrift = FMath::Max(MaxSpeedThisDrift, Speed);

	// Detect drift direction for reverse/counter detection
	ActiveDrift.bIsReverse = (SlipAngle < 0);

	// Calculate points this frame
	CalculatePoints(DeltaTime);

	// Update grade
	UpdateGrade();

	// Update multiplier
	UpdateMultiplier();
}

void UMGDriftSubsystem::StartDrift()
{
	if (ActiveDrift.bIsDrifting)
	{
		return;
	}

	// Check if this extends a chain
	float TimeSinceLastDrift = GetTimeSinceLastDrift();
	bool bExtendingChain = TimeSinceLastDrift > 0 && TimeSinceLastDrift <= Config.ChainTimeWindow;

	ActiveDrift = FMGActiveDrift();
	ActiveDrift.bIsDrifting = true;

	if (bExtendingChain)
	{
		ExtendChain();
	}

	MaxAngleThisDrift = 0.0f;
	MaxSpeedThisDrift = 0.0f;

	OnDriftStarted.Broadcast(ActiveDrift);
}

void UMGDriftSubsystem::EndDrift(bool bFailed)
{
	if (!ActiveDrift.bIsDrifting)
	{
		return;
	}

	FMGDriftResult Result = BuildDriftResult(bFailed);

	// Update session stats
	if (!bFailed)
	{
		SessionStats.TotalPoints += Result.TotalPoints;
		SessionStats.TotalDrifts++;
		SessionStats.TotalDriftDistance += Result.Distance;
		SessionStats.TotalDriftTime += Result.Duration;
		SessionStats.MaxDriftAngle = FMath::Max(SessionStats.MaxDriftAngle, Result.MaxAngle);
		SessionStats.MaxDriftSpeed = FMath::Max(SessionStats.MaxDriftSpeed, Result.MaxSpeed);
		SessionStats.LongestChain = FMath::Max(SessionStats.LongestChain, Result.ChainCount);
		SessionStats.HighestSingleDrift = FMath::Max(SessionStats.HighestSingleDrift, Result.TotalPoints);

		if (Result.bPerfect)
		{
			SessionStats.PerfectDrifts++;
		}

		int32& GradeCount = SessionStats.GradeCounts.FindOrAdd(Result.Grade);
		GradeCount++;

		for (EMGDriftChainBonus Bonus : Result.Bonuses)
		{
			int32& BonusCount = SessionStats.BonusCounts.FindOrAdd(Bonus);
			BonusCount++;
		}

		// Update zone high score if applicable
		if (bInDriftZone && !CurrentZone.ZoneID.IsNone())
		{
			int32& HighScore = ZoneHighScores.FindOrAdd(CurrentZone.ZoneID);
			if (Result.TotalPoints > HighScore)
			{
				bool bNewHighScore = HighScore > 0;
				HighScore = Result.TotalPoints;
				OnDriftZoneEntered.Broadcast(CurrentZone, bNewHighScore);
			}
		}

		OnDriftEnded.Broadcast(Result);
	}
	else
	{
		SessionStats.FailedDrifts++;
		OnDriftFailed.Broadcast(Result);
	}

	// Record end time for chain tracking
	if (const UWorld* World = GetWorld())
	{
		LastDriftEndTime = World->GetTimeSeconds();
	}

	// Reset active drift but keep chain count if not failed
	int32 PreviousChainCount = bFailed ? 0 : ActiveDrift.ChainCount;
	ActiveDrift = FMGActiveDrift();
	ActiveDrift.ChainCount = PreviousChainCount;
}

void UMGDriftSubsystem::ResetSession()
{
	SessionStats = FMGDriftSessionStats();
	ActiveDrift = FMGActiveDrift();
	LastDriftEndTime = 0.0f;
	LastMilestoneReached = 0;
}

float UMGDriftSubsystem::GetTimeSinceLastDrift() const
{
	if (LastDriftEndTime <= 0.0f)
	{
		return -1.0f;
	}

	if (const UWorld* World = GetWorld())
	{
		return World->GetTimeSeconds() - LastDriftEndTime;
	}

	return -1.0f;
}

// ============================================================================
// Scoring
// ============================================================================

EMGDriftGrade UMGDriftSubsystem::CalculateGradeFromPoints(int32 Points) const
{
	if (Points >= Config.SSSThreshold)
	{
		return EMGDriftGrade::SSS;
	}
	if (Points >= Config.SSThreshold)
	{
		return EMGDriftGrade::SS;
	}
	if (Points >= Config.SThreshold)
	{
		return EMGDriftGrade::S;
	}
	if (Points >= Config.AThreshold)
	{
		return EMGDriftGrade::A;
	}
	if (Points >= Config.BThreshold)
	{
		return EMGDriftGrade::B;
	}
	if (Points >= Config.CThreshold)
	{
		return EMGDriftGrade::C;
	}
	if (Points >= Config.DThreshold)
	{
		return EMGDriftGrade::D;
	}

	return EMGDriftGrade::None;
}

// ============================================================================
// Bonuses
// ============================================================================

void UMGDriftSubsystem::TriggerBonus(EMGDriftChainBonus Bonus)
{
	if (!ActiveDrift.bIsDrifting || Bonus == EMGDriftChainBonus::None)
	{
		return;
	}

	if (ActiveDrift.ActiveBonuses.Contains(Bonus))
	{
		return; // Already have this bonus this drift
	}

	ActiveDrift.ActiveBonuses.Add(Bonus);

	int32 BonusPoints = GetBonusPoints(Bonus);
	ActiveDrift.CurrentPoints += BonusPoints;

	OnDriftBonusTriggered.Broadcast(Bonus, BonusPoints);
}

void UMGDriftSubsystem::SetWallProximity(float Distance)
{
	if (!ActiveDrift.bIsDrifting)
	{
		return;
	}

	ActiveDrift.MinWallDistance = FMath::Min(ActiveDrift.MinWallDistance, Distance);

	if (Distance <= Config.WallProximityBonusDistance)
	{
		TriggerBonus(EMGDriftChainBonus::WallTap);
	}
}

void UMGDriftSubsystem::SetTandemPartner(bool bHasPartner, float PartnerDistance)
{
	bHasTandemPartner = bHasPartner;
	TandemPartnerDistance = PartnerDistance;

	if (bHasPartner && PartnerDistance <= Config.TandemBonusDistance && ActiveDrift.bIsDrifting)
	{
		TriggerBonus(EMGDriftChainBonus::Tandem);
	}
}

void UMGDriftSubsystem::RegisterOvertake()
{
	if (ActiveDrift.bIsDrifting)
	{
		TriggerBonus(EMGDriftChainBonus::Overtake);
	}
}

void UMGDriftSubsystem::RegisterCloseCall(float Distance)
{
	if (ActiveDrift.bIsDrifting && Distance < 50.0f)
	{
		TriggerBonus(EMGDriftChainBonus::CloseCall);
	}
}

int32 UMGDriftSubsystem::GetBonusPoints(EMGDriftChainBonus Bonus) const
{
	switch (Bonus)
	{
	case EMGDriftChainBonus::Tandem:
		return 2000;
	case EMGDriftChainBonus::Counter:
		return 1500;
	case EMGDriftChainBonus::Manji:
		return 2500;
	case EMGDriftChainBonus::Feint:
		return 1000;
	case EMGDriftChainBonus::WallTap:
		return 1500;
	case EMGDriftChainBonus::DonutEntry:
		return 500;
	case EMGDriftChainBonus::CloseCall:
		return 1000;
	case EMGDriftChainBonus::Overtake:
		return 1500;
	default:
		return 0;
	}
}

// ============================================================================
// Chain Management
// ============================================================================

bool UMGDriftSubsystem::IsChainActive() const
{
	float TimeSinceLastDrift = GetTimeSinceLastDrift();
	return ActiveDrift.bIsDrifting || (TimeSinceLastDrift >= 0 && TimeSinceLastDrift <= Config.ChainTimeWindow);
}

void UMGDriftSubsystem::ExtendChain()
{
	ActiveDrift.ChainCount++;

	// Increase multiplier with chain
	float ChainBonus = 0.25f * ActiveDrift.ChainCount;
	ActiveDrift.Multiplier = FMath::Min(5.0f, 1.0f + ChainBonus);

	OnDriftChainExtended.Broadcast(ActiveDrift.ChainCount, ActiveDrift.Multiplier);
}

void UMGDriftSubsystem::BreakChain()
{
	ActiveDrift.ChainCount = 0;
	ActiveDrift.Multiplier = 1.0f;
}

// ============================================================================
// Zones
// ============================================================================

void UMGDriftSubsystem::EnterDriftZone(const FMGDriftZone& Zone)
{
	bInDriftZone = true;
	CurrentZone = Zone;

	int32 HighScore = GetZoneHighScore(Zone.ZoneID);
	OnDriftZoneEntered.Broadcast(Zone, false);
}

void UMGDriftSubsystem::ExitDriftZone()
{
	bInDriftZone = false;
	CurrentZone = FMGDriftZone();
}

void UMGDriftSubsystem::RegisterDriftZone(const FMGDriftZone& Zone)
{
	// Check if zone already exists
	for (FMGDriftZone& ExistingZone : RegisteredZones)
	{
		if (ExistingZone.ZoneID == Zone.ZoneID)
		{
			ExistingZone = Zone;
			return;
		}
	}

	RegisteredZones.Add(Zone);
}

TArray<FMGDriftZone> UMGDriftSubsystem::GetAllDriftZones() const
{
	return RegisteredZones;
}

int32 UMGDriftSubsystem::GetZoneHighScore(FName ZoneID) const
{
	if (const int32* Score = ZoneHighScores.Find(ZoneID))
	{
		return *Score;
	}
	return 0;
}

// ============================================================================
// Stats
// ============================================================================

void UMGDriftSubsystem::MergeSessionToCareer()
{
	CareerStats.TotalPoints += SessionStats.TotalPoints;
	CareerStats.TotalDrifts += SessionStats.TotalDrifts;
	CareerStats.TotalDriftDistance += SessionStats.TotalDriftDistance;
	CareerStats.TotalDriftTime += SessionStats.TotalDriftTime;
	CareerStats.MaxDriftAngle = FMath::Max(CareerStats.MaxDriftAngle, SessionStats.MaxDriftAngle);
	CareerStats.MaxDriftSpeed = FMath::Max(CareerStats.MaxDriftSpeed, SessionStats.MaxDriftSpeed);
	CareerStats.LongestChain = FMath::Max(CareerStats.LongestChain, SessionStats.LongestChain);
	CareerStats.HighestSingleDrift = FMath::Max(CareerStats.HighestSingleDrift, SessionStats.HighestSingleDrift);
	CareerStats.PerfectDrifts += SessionStats.PerfectDrifts;
	CareerStats.FailedDrifts += SessionStats.FailedDrifts;

	for (const auto& GradePair : SessionStats.GradeCounts)
	{
		int32& CareerCount = CareerStats.GradeCounts.FindOrAdd(GradePair.Key);
		CareerCount += GradePair.Value;
	}

	for (const auto& BonusPair : SessionStats.BonusCounts)
	{
		int32& CareerCount = CareerStats.BonusCounts.FindOrAdd(BonusPair.Key);
		CareerCount += BonusPair.Value;
	}

	SaveDriftData();
}

// ============================================================================
// Leaderboard
// ============================================================================

TArray<FMGDriftLeaderboardEntry> UMGDriftSubsystem::GetZoneLeaderboard(FName ZoneID, int32 MaxEntries) const
{
	if (const TArray<FMGDriftLeaderboardEntry>* Leaderboard = ZoneLeaderboards.Find(ZoneID))
	{
		if (MaxEntries <= 0 || MaxEntries >= Leaderboard->Num())
		{
			return *Leaderboard;
		}

		TArray<FMGDriftLeaderboardEntry> TopEntries;
		for (int32 i = 0; i < FMath::Min(MaxEntries, Leaderboard->Num()); ++i)
		{
			TopEntries.Add((*Leaderboard)[i]);
		}
		return TopEntries;
	}

	return TArray<FMGDriftLeaderboardEntry>();
}

void UMGDriftSubsystem::SubmitZoneScore(FName ZoneID, int32 Score, FName VehicleID)
{
	TArray<FMGDriftLeaderboardEntry>& Leaderboard = ZoneLeaderboards.FindOrAdd(ZoneID);

	FMGDriftLeaderboardEntry NewEntry;
	NewEntry.Score = Score;
	NewEntry.VehicleID = VehicleID;
	NewEntry.SetAt = FDateTime::UtcNow();
	// PlayerName and PlayerID would be set from player profile

	Leaderboard.Add(NewEntry);

	// Sort by score descending
	Leaderboard.Sort([](const FMGDriftLeaderboardEntry& A, const FMGDriftLeaderboardEntry& B)
	{
		return A.Score > B.Score;
	});

	// Update ranks
	for (int32 i = 0; i < Leaderboard.Num(); ++i)
	{
		Leaderboard[i].Rank = i + 1;
	}

	// Limit leaderboard size
	const int32 MaxLeaderboardSize = 100;
	while (Leaderboard.Num() > MaxLeaderboardSize)
	{
		Leaderboard.RemoveAt(Leaderboard.Num() - 1);
	}

	SaveDriftData();
}

int32 UMGDriftSubsystem::GetZoneLeaderboardPosition(FName ZoneID) const
{
	// Would check local player's position
	return 0;
}

// ============================================================================
// Configuration
// ============================================================================

void UMGDriftSubsystem::SetDriftConfig(const FMGDriftConfig& NewConfig)
{
	Config = NewConfig;
}

// ============================================================================
// Protected Helpers
// ============================================================================

void UMGDriftSubsystem::CalculatePoints(float MGDeltaTime)
{
	// Base points from duration
	float BasePoints = Config.BasePointsPerSecond * DeltaTime;

	// Angle multiplier (higher angle = more points)
	float AngleMultiplier = 1.0f + (ActiveDrift.CurrentAngle - Config.MinDriftAngle) * Config.AngleMultiplierScale;

	// Speed multiplier (higher speed = more points)
	float SpeedMultiplier = 1.0f + (ActiveDrift.CurrentSpeed - Config.MinDriftSpeed) * Config.SpeedMultiplierScale;

	// Zone multiplier
	float ZoneMultiplier = bInDriftZone ? CurrentZone.PointsMultiplier : 1.0f;

	// Tandem multiplier
	float TandemMult = 1.0f;
	if (bHasTandemPartner && TandemPartnerDistance <= Config.TandemBonusDistance)
	{
		TandemMult = Config.TandemMultiplier;
	}

	// Wall proximity multiplier
	float WallMult = 1.0f;
	if (ActiveDrift.MinWallDistance <= Config.WallProximityBonusDistance)
	{
		float ProximityFactor = 1.0f - (ActiveDrift.MinWallDistance / Config.WallProximityBonusDistance);
		WallMult = 1.0f + (Config.WallProximityMultiplier - 1.0f) * ProximityFactor;
	}

	// Calculate final points
	float FramePoints = BasePoints * AngleMultiplier * SpeedMultiplier * ZoneMultiplier * TandemMult * WallMult * ActiveDrift.Multiplier;

	int32 OldScore = ActiveDrift.CurrentPoints;
	ActiveDrift.CurrentPoints += FMath::RoundToInt(FramePoints);

	// Check milestones
	CheckMilestones(OldScore, ActiveDrift.CurrentPoints);
}

void UMGDriftSubsystem::UpdateGrade()
{
	EMGDriftGrade NewGrade = CalculateGradeFromPoints(ActiveDrift.CurrentPoints);

	if (NewGrade != ActiveDrift.CurrentGrade)
	{
		EMGDriftGrade OldGrade = ActiveDrift.CurrentGrade;
		ActiveDrift.CurrentGrade = NewGrade;
		OnDriftGradeChanged.Broadcast(OldGrade, NewGrade);
	}
}

void UMGDriftSubsystem::UpdateMultiplier()
{
	// Base multiplier from chain
	float ChainBonus = 0.25f * ActiveDrift.ChainCount;

	// Duration bonus
	float DurationBonus = FMath::Min(0.5f, ActiveDrift.DriftDuration * 0.1f);

	ActiveDrift.Multiplier = FMath::Min(5.0f, 1.0f + ChainBonus + DurationBonus);
}

void UMGDriftSubsystem::CheckMilestones(int32 OldScore, int32 NewScore)
{
	// Define milestone thresholds
	TArray<int32> Milestones = { 5000, 10000, 25000, 50000, 75000, 100000, 150000, 200000 };

	for (int32 Milestone : Milestones)
	{
		if (OldScore < Milestone && NewScore >= Milestone && Milestone > LastMilestoneReached)
		{
			LastMilestoneReached = Milestone;

			FName MilestoneName;
			if (Milestone >= 100000)
			{
				MilestoneName = TEXT("DriftMaster");
			}
			else if (Milestone >= 50000)
			{
				MilestoneName = TEXT("DriftKing");
			}
			else if (Milestone >= 25000)
			{
				MilestoneName = TEXT("Sideways");
			}
			else
			{
				MilestoneName = TEXT("GettingIt");
			}

			OnDriftScoreMilestone.Broadcast(Milestone, MilestoneName);
		}
	}
}

FMGDriftResult UMGDriftSubsystem::BuildDriftResult(bool bFailed)
{
	FMGDriftResult Result;
	Result.BasePoints = ActiveDrift.CurrentPoints;
	Result.BonusPoints = 0;

	for (EMGDriftChainBonus Bonus : ActiveDrift.ActiveBonuses)
	{
		Result.BonusPoints += GetBonusPoints(Bonus);
	}

	Result.TotalPoints = Result.BasePoints;
	Result.FinalMultiplier = ActiveDrift.Multiplier;
	Result.Duration = ActiveDrift.DriftDuration;
	Result.Distance = ActiveDrift.DriftDistance;
	Result.MaxAngle = MaxAngleThisDrift;
	Result.MaxSpeed = MaxSpeedThisDrift;
	Result.Grade = ActiveDrift.CurrentGrade;
	Result.ChainCount = ActiveDrift.ChainCount;
	Result.Bonuses = ActiveDrift.ActiveBonuses;
	Result.bFailed = bFailed;

	// Check for perfect drift (high angle, high speed, long duration, no failure)
	Result.bPerfect = !bFailed &&
		Result.Grade >= EMGDriftGrade::S &&
		Result.Duration >= 3.0f &&
		Result.MaxAngle >= 45.0f;

	return Result;
}

void UMGDriftSubsystem::SaveDriftData()
{
	// Persist drift data
	// Implementation would use USaveGame or cloud save
}

void UMGDriftSubsystem::LoadDriftData()
{
	// Load persisted drift data
	// Implementation would use USaveGame or cloud save
}

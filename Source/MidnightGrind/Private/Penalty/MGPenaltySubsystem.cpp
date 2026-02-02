// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPenaltySubsystem.cpp
 * @brief Implementation of the Race Penalty and Rules Enforcement Subsystem.
 *
 * This file implements penalty detection, issuance, tracking, and resolution
 * for all racing rule violations including track limits, collisions, false
 * starts, pit lane speeding, and blue flag enforcement.
 *
 * @see UMGPenaltySubsystem
 */

#include "Penalty/MGPenaltySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGPenaltySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default rules
	Rules.bEnforceTrackLimits = true;
	Rules.TrackLimitsWarnings = 3;
	Rules.TrackLimitsTimeAdded = 5.0f;
	Rules.bAutoDeleteLapTimes = true;
	Rules.bEnforceCornerCutting = true;
	Rules.bEnforceCollisions = true;
	Rules.CollisionSpeedThreshold = 30.0f;
	Rules.bEnforceFalseStart = true;
	Rules.FalseStartThreshold = 0.1f;
	Rules.PitSpeedLimit = 60.0f;
	Rules.PitSpeedPenalty = 5.0f;
	Rules.bEnforceBlueFlags = true;
	Rules.BlueFlagIgnoreLimit = 3;
	Rules.bAllowAppeals = true;
	Rules.PenaltyServeLaps = 3;

	// Initialize default settings
	Settings.bEnablePenalties = true;
	Settings.bStrictRules = false;
	Settings.bShowPenaltyNotifications = true;
	Settings.bShowWarnings = true;
	Settings.bAutoServePenalties = false;

	// Start penalty tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PenaltyTickHandle,
			this,
			&UMGPenaltySubsystem::OnPenaltyTick,
			1.0f,
			true
		);
	}
}

void UMGPenaltySubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PenaltyTickHandle);
	}

	Super::Deinitialize();
}

bool UMGPenaltySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGPenaltySubsystem::OnPenaltyTick()
{
	CheckUnservedPenalties();
}

void UMGPenaltySubsystem::CheckUnservedPenalties()
{
	for (auto& Pair : DriverIncidents)
	{
		FName VehicleID = Pair.Key;
		FMGDriverIncidents& Incidents = Pair.Value;

		for (FMGPenalty& Penalty : Incidents.PenaltyHistory)
		{
			if (Penalty.State == EMGPenaltyState::Active)
			{
				// Check if penalty serve window expired
				int32 LapsElapsed = CurrentLap - Penalty.LapIssued;
				if (LapsElapsed > Penalty.LapsToServe && !Settings.bAutoServePenalties)
				{
					// Unserved penalty - escalate
					if (Penalty.Type == EMGPenaltyType::DriveThrough)
					{
						Penalty.Type = EMGPenaltyType::StopAndGo;
						Penalty.TimeValue = 10.0f;
					}
					else if (Penalty.Type == EMGPenaltyType::StopAndGo)
					{
						IssueDisqualification(VehicleID, EMGViolationType::UnservedPenalty);
					}
				}
			}
		}
	}
}

FMGDriverIncidents& UMGPenaltySubsystem::GetOrCreateIncidents(FName VehicleID)
{
	if (!DriverIncidents.Contains(VehicleID))
	{
		FMGDriverIncidents NewIncidents;
		NewIncidents.VehicleID = VehicleID;
		DriverIncidents.Add(VehicleID, NewIncidents);
	}
	return DriverIncidents[VehicleID];
}

FMGPenalty UMGPenaltySubsystem::IssuePenalty(FName VehicleID, EMGPenaltyType Type, EMGViolationType Violation, float TimeValue)
{
	if (!Settings.bEnablePenalties)
	{
		return FMGPenalty();
	}

	FMGPenalty Penalty;
	Penalty.PenaltyID = FGuid::NewGuid();
	Penalty.VehicleID = VehicleID;
	Penalty.Type = Type;
	Penalty.Violation = Violation;
	Penalty.TimeValue = TimeValue;
	Penalty.State = EMGPenaltyState::Announced;
	Penalty.LapIssued = CurrentLap;
	Penalty.LapsToServe = Rules.PenaltyServeLaps;
	Penalty.IssuedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	FMGDriverIncidents& Incidents = GetOrCreateIncidents(VehicleID);
	Incidents.Penalties++;
	Incidents.TotalTimePenalties += TimeValue;
	Incidents.PenaltyHistory.Add(Penalty);

	OnPenaltyIssued.Broadcast(VehicleID, Penalty);

	// Auto-activate penalty
	Penalty.State = EMGPenaltyState::Active;

	return Penalty;
}

void UMGPenaltySubsystem::IssueWarning(FName VehicleID, EMGViolationType Violation)
{
	FMGDriverIncidents& Incidents = GetOrCreateIncidents(VehicleID);
	Incidents.Warnings++;

	OnWarningIssued.Broadcast(VehicleID, Violation);
}

void UMGPenaltySubsystem::IssueDriveThrough(FName VehicleID, EMGViolationType Violation)
{
	FMGPenalty Penalty = IssuePenalty(VehicleID, EMGPenaltyType::DriveThrough, Violation, 0.0f);
	Penalty.Description = FText::FromString(TEXT("Drive-through penalty"));
}

void UMGPenaltySubsystem::IssueStopAndGo(FName VehicleID, EMGViolationType Violation, float Duration)
{
	FMGPenalty Penalty = IssuePenalty(VehicleID, EMGPenaltyType::StopAndGo, Violation, Duration);
	Penalty.Description = FText::FromString(FString::Printf(TEXT("Stop and Go - %.0f seconds"), Duration));
}

void UMGPenaltySubsystem::IssueTimePenalty(FName VehicleID, EMGViolationType Violation, float Seconds)
{
	FMGPenalty Penalty = IssuePenalty(VehicleID, EMGPenaltyType::TimeAdded, Violation, Seconds);
	Penalty.Description = FText::FromString(FString::Printf(TEXT("+%.0f second time penalty"), Seconds));
}

void UMGPenaltySubsystem::IssuePositionPenalty(FName VehicleID, EMGViolationType Violation, int32 Positions)
{
	FMGPenalty Penalty = IssuePenalty(VehicleID, EMGPenaltyType::PositionPenalty, Violation, 0.0f);
	Penalty.PositionValue = Positions;
	Penalty.Description = FText::FromString(FString::Printf(TEXT("Drop %d position(s)"), Positions));
}

void UMGPenaltySubsystem::IssueDisqualification(FName VehicleID, EMGViolationType Violation)
{
	FMGPenalty Penalty = IssuePenalty(VehicleID, EMGPenaltyType::Disqualification, Violation, 0.0f);
	Penalty.Description = FText::FromString(TEXT("Disqualified"));
	Penalty.bAppealable = false;

	OnDisqualification.Broadcast(VehicleID);
}

void UMGPenaltySubsystem::ServePenalty(FName VehicleID, FGuid PenaltyID)
{
	if (!DriverIncidents.Contains(VehicleID)) return;

	FMGDriverIncidents& Incidents = DriverIncidents[VehicleID];

	for (FMGPenalty& Penalty : Incidents.PenaltyHistory)
	{
		if (Penalty.PenaltyID == PenaltyID && Penalty.State == EMGPenaltyState::Active)
		{
			Penalty.State = EMGPenaltyState::Served;
			Penalty.ServedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

			OnPenaltyServed.Broadcast(VehicleID, Penalty);
			break;
		}
	}
}

void UMGPenaltySubsystem::CancelPenalty(FName VehicleID, FGuid PenaltyID)
{
	if (!DriverIncidents.Contains(VehicleID)) return;

	FMGDriverIncidents& Incidents = DriverIncidents[VehicleID];

	for (FMGPenalty& Penalty : Incidents.PenaltyHistory)
	{
		if (Penalty.PenaltyID == PenaltyID)
		{
			Penalty.State = EMGPenaltyState::Cancelled;
			Incidents.TotalTimePenalties -= Penalty.TimeValue;
			Incidents.Penalties--;

			OnPenaltyCancelled.Broadcast(VehicleID, PenaltyID);
			break;
		}
	}
}

void UMGPenaltySubsystem::AppealPenalty(FName VehicleID, FGuid PenaltyID)
{
	if (!Rules.bAllowAppeals) return;
	if (!DriverIncidents.Contains(VehicleID)) return;

	FMGDriverIncidents& Incidents = DriverIncidents[VehicleID];

	for (FMGPenalty& Penalty : Incidents.PenaltyHistory)
	{
		if (Penalty.PenaltyID == PenaltyID && Penalty.bAppealable)
		{
			Penalty.State = EMGPenaltyState::Appealed;
			// Appeal would be reviewed
			break;
		}
	}
}

TArray<FMGPenalty> UMGPenaltySubsystem::GetPendingPenalties(FName VehicleID) const
{
	TArray<FMGPenalty> Pending;

	if (DriverIncidents.Contains(VehicleID))
	{
		for (const FMGPenalty& Penalty : DriverIncidents[VehicleID].PenaltyHistory)
		{
			if (Penalty.State == EMGPenaltyState::Active || Penalty.State == EMGPenaltyState::Announced)
			{
				Pending.Add(Penalty);
			}
		}
	}

	return Pending;
}

TArray<FMGPenalty> UMGPenaltySubsystem::GetAllPenalties(FName VehicleID) const
{
	if (DriverIncidents.Contains(VehicleID))
	{
		return DriverIncidents[VehicleID].PenaltyHistory;
	}
	return TArray<FMGPenalty>();
}

bool UMGPenaltySubsystem::HasPendingPenalty(FName VehicleID) const
{
	return GetPendingPenalties(VehicleID).Num() > 0;
}

FMGPenalty UMGPenaltySubsystem::GetMostSeverePenalty(FName VehicleID) const
{
	TArray<FMGPenalty> Pending = GetPendingPenalties(VehicleID);

	if (Pending.Num() == 0)
	{
		return FMGPenalty();
	}

	FMGPenalty MostSevere = Pending[0];
	for (const FMGPenalty& Penalty : Pending)
	{
		if (Penalty.Type > MostSevere.Type)
		{
			MostSevere = Penalty;
		}
	}

	return MostSevere;
}

void UMGPenaltySubsystem::ReportTrackLimitsViolation(FName VehicleID, int32 CornerNumber, int32 LapNumber, const FVector& Position)
{
	if (!Rules.bEnforceTrackLimits) return;

	FMGDriverIncidents& Incidents = GetOrCreateIncidents(VehicleID);

	FMGTrackLimitsViolation Violation;
	Violation.VehicleID = VehicleID;
	Violation.CornerNumber = CornerNumber;
	Violation.LapNumber = LapNumber;
	Violation.ViolationPosition = Position;
	Violation.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Violation.Severity = EMGTrackLimitsSeverity::Minor;

	Incidents.TrackLimitsViolations++;
	Incidents.TrackLimitsHistory.Add(Violation);

	OnTrackLimitsViolation.Broadcast(VehicleID, Violation);

	ProcessTrackLimits(VehicleID);
}

void UMGPenaltySubsystem::ProcessTrackLimits(FName VehicleID)
{
	FMGDriverIncidents& Incidents = GetOrCreateIncidents(VehicleID);

	int32 Count = Incidents.TrackLimitsViolations;

	if (Count > 0 && Count <= Rules.TrackLimitsWarnings)
	{
		Incidents.TrackLimitsWarnings++;
		IssueWarning(VehicleID, EMGViolationType::TrackLimits);
	}
	else if (Count > Rules.TrackLimitsWarnings)
	{
		// Reset count and issue penalty
		int32 PenaltyNumber = (Count - 1) / Rules.TrackLimitsWarnings;

		if (Count % Rules.TrackLimitsWarnings == 1)
		{
			IssueTimePenalty(VehicleID, EMGViolationType::TrackLimits, Rules.TrackLimitsTimeAdded);
		}
	}
}

void UMGPenaltySubsystem::SetTrackLimitsSeverity(FName VehicleID, EMGTrackLimitsSeverity Severity, float TimeGained)
{
	FMGDriverIncidents& Incidents = GetOrCreateIncidents(VehicleID);

	if (Incidents.TrackLimitsHistory.Num() > 0)
	{
		FMGTrackLimitsViolation& LastViolation = Incidents.TrackLimitsHistory.Last();
		LastViolation.Severity = Severity;
		LastViolation.TimeGained = TimeGained;

		// Delete lap time if significant advantage
		if (Rules.bAutoDeleteLapTimes && TimeGained > 0.5f)
		{
			DeleteLapTime(VehicleID, LastViolation.LapNumber);
			LastViolation.bLapTimeDeleted = true;
		}
	}
}

int32 UMGPenaltySubsystem::GetTrackLimitsCount(FName VehicleID) const
{
	if (DriverIncidents.Contains(VehicleID))
	{
		return DriverIncidents[VehicleID].TrackLimitsViolations;
	}
	return 0;
}

int32 UMGPenaltySubsystem::GetTrackLimitsWarningsRemaining(FName VehicleID) const
{
	int32 Count = GetTrackLimitsCount(VehicleID);
	int32 WarningsUsed = Count % Rules.TrackLimitsWarnings;
	return Rules.TrackLimitsWarnings - WarningsUsed;
}

void UMGPenaltySubsystem::DeleteLapTime(FName VehicleID, int32 LapNumber)
{
	OnLapTimeDeleted.Broadcast(VehicleID, LapNumber);
}

void UMGPenaltySubsystem::ReportCollision(const FMGCollisionData& Collision)
{
	if (!Rules.bEnforceCollisions) return;

	AllCollisions.Add(Collision);

	FMGCollisionData MutableCollision = Collision;
	DetermineFault(MutableCollision);

	// Record for both drivers
	GetOrCreateIncidents(Collision.Vehicle1ID).CollisionHistory.Add(MutableCollision);
	GetOrCreateIncidents(Collision.Vehicle2ID).CollisionHistory.Add(MutableCollision);

	if (!MutableCollision.bRacingIncident && !MutableCollision.AtFaultDriver.IsNone())
	{
		GetOrCreateIncidents(MutableCollision.AtFaultDriver).CollisionsAtFault++;

		// Issue penalty based on severity
		if (MutableCollision.RelativeSpeed > Rules.CollisionSpeedThreshold * 2)
		{
			IssueDriveThrough(MutableCollision.AtFaultDriver, EMGViolationType::Collision);
		}
		else if (MutableCollision.RelativeSpeed > Rules.CollisionSpeedThreshold)
		{
			IssueTimePenalty(MutableCollision.AtFaultDriver, EMGViolationType::Collision, 5.0f);
		}
		else
		{
			IssueWarning(MutableCollision.AtFaultDriver, EMGViolationType::Collision);
		}
	}
	else
	{
		GetOrCreateIncidents(Collision.Vehicle1ID).RacingIncidents++;
		GetOrCreateIncidents(Collision.Vehicle2ID).RacingIncidents++;
	}

	OnCollisionDetected.Broadcast(MutableCollision);
}

void UMGPenaltySubsystem::DetermineFault(FMGCollisionData& Collision)
{
	// Simple fault determination based on overlap
	// Vehicle with more overlap is typically at fault

	if (Collision.OverlapPercentage > 0.7f)
	{
		// Mostly alongside - racing incident
		Collision.bRacingIncident = true;
	}
	else if (Collision.OverlapPercentage < 0.3f)
	{
		// Minimal overlap - rear car at fault
		// Would need more context about relative positions
		Collision.AtFaultDriver = Collision.Vehicle2ID;
	}
	else
	{
		// Mid overlap - need steward review
		Collision.bRacingIncident = !Settings.bStrictRules;
	}
}

TArray<FMGCollisionData> UMGPenaltySubsystem::GetCollisionHistory() const
{
	return AllCollisions;
}

int32 UMGPenaltySubsystem::GetCollisionsAtFault(FName VehicleID) const
{
	if (DriverIncidents.Contains(VehicleID))
	{
		return DriverIncidents[VehicleID].CollisionsAtFault;
	}
	return 0;
}

void UMGPenaltySubsystem::CheckFalseStart(FName VehicleID, float ReactionTime)
{
	if (!Rules.bEnforceFalseStart) return;

	if (ReactionTime < Rules.FalseStartThreshold)
	{
		// Jumped the start
		IssueDriveThrough(VehicleID, EMGViolationType::FalseStart);
	}
}

void UMGPenaltySubsystem::CheckPitSpeeding(FName VehicleID, float Speed)
{
	if (Speed > Rules.PitSpeedLimit)
	{
		IssueTimePenalty(VehicleID, EMGViolationType::PitSpeeding, Rules.PitSpeedPenalty);
	}
}

void UMGPenaltySubsystem::CheckBlueFlagIgnore(FName VehicleID)
{
	if (!Rules.bEnforceBlueFlags) return;

	if (!BlueFlagCounts.Contains(VehicleID))
	{
		BlueFlagCounts.Add(VehicleID, 0);
	}

	BlueFlagCounts[VehicleID]++;

	if (BlueFlagCounts[VehicleID] >= Rules.BlueFlagIgnoreLimit)
	{
		IssueDriveThrough(VehicleID, EMGViolationType::IgnoringFlags);
		BlueFlagCounts[VehicleID] = 0;
	}
}

void UMGPenaltySubsystem::ClearBlueFlagCount(FName VehicleID)
{
	BlueFlagCounts.Add(VehicleID, 0);
}

FMGDriverIncidents UMGPenaltySubsystem::GetDriverIncidents(FName VehicleID) const
{
	if (DriverIncidents.Contains(VehicleID))
	{
		return DriverIncidents[VehicleID];
	}
	return FMGDriverIncidents();
}

void UMGPenaltySubsystem::ResetDriverIncidents(FName VehicleID)
{
	if (DriverIncidents.Contains(VehicleID))
	{
		FMGDriverIncidents& Incidents = DriverIncidents[VehicleID];
		Incidents.TrackLimitsViolations = 0;
		Incidents.TrackLimitsWarnings = 0;
		Incidents.TrackLimitsHistory.Empty();
	}
	BlueFlagCounts.Remove(VehicleID);
}

void UMGPenaltySubsystem::ResetAllIncidents()
{
	DriverIncidents.Empty();
	BlueFlagCounts.Empty();
	AllCollisions.Empty();
}

float UMGPenaltySubsystem::GetTotalTimePenalties(FName VehicleID) const
{
	if (DriverIncidents.Contains(VehicleID))
	{
		return DriverIncidents[VehicleID].TotalTimePenalties;
	}
	return 0.0f;
}

int32 UMGPenaltySubsystem::GetTotalWarnings(FName VehicleID) const
{
	if (DriverIncidents.Contains(VehicleID))
	{
		return DriverIncidents[VehicleID].Warnings;
	}
	return 0;
}

void UMGPenaltySubsystem::SetPenaltyRules(const FMGPenaltyRules& NewRules)
{
	Rules = NewRules;
}

void UMGPenaltySubsystem::SetPenaltySettings(const FMGPenaltySettings& NewSettings)
{
	Settings = NewSettings;
}

void UMGPenaltySubsystem::OnLapCompleted(FName VehicleID, int32 LapNumber)
{
	CurrentLap = LapNumber;

	// Check pending penalties
	if (DriverIncidents.Contains(VehicleID))
	{
		for (FMGPenalty& Penalty : DriverIncidents[VehicleID].PenaltyHistory)
		{
			if (Penalty.State == EMGPenaltyState::Active)
			{
				int32 LapsRemaining = Penalty.LapsToServe - (LapNumber - Penalty.LapIssued);
				if (LapsRemaining <= 0)
				{
					// Penalty service window expired
					if (Settings.bAutoServePenalties && Penalty.Type == EMGPenaltyType::TimeAdded)
					{
						ServePenalty(VehicleID, Penalty.PenaltyID);
					}
				}
			}
		}
	}
}

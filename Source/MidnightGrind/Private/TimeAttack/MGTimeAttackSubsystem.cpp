// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTimeAttackSubsystem.cpp
 * @brief Implementation of the Time Attack racing mode subsystem.
 *
 * Manages time trial sessions, lap timing, ghost replay data, sector splits,
 * delta calculations, trial challenges, and personal best records.
 */

#include "TimeAttack/MGTimeAttackSubsystem.h"

void UMGTimeAttackSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultTrials();
	LoadTimeAttackData();
}

void UMGTimeAttackSubsystem::Deinitialize()
{
	if (IsInSession())
	{
		EndSession();
	}

	SaveTimeAttackData();

	Super::Deinitialize();
}

bool UMGTimeAttackSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

bool UMGTimeAttackSubsystem::StartSession(FName TrackID, FName VehicleID, EMGTimeAttackMode Mode)
{
	if (IsInSession())
	{
		EndSession();
	}

	CurrentSession = FMGTimeAttackSession();
	CurrentSession.SessionID = FGuid::NewGuid();
	CurrentSession.TrackID = TrackID;
	CurrentSession.VehicleID = VehicleID;
	CurrentSession.Mode = Mode;
	CurrentSession.bIsActive = true;
	CurrentSession.StartedAt = FDateTime::UtcNow();

	// Get personal best for this combo
	FMGTimeAttackRecord Record = GetPersonalBest(TrackID, VehicleID);
	CurrentSession.PersonalBest = Record.BestTime;

	// Reset current lap
	CurrentLap = FMGLapTime();
	CurrentLap.LapNumber = 1;
	CurrentLap.bIsValid = true;

	OnSessionStarted.Broadcast();

	return true;
}

void UMGTimeAttackSubsystem::EndSession()
{
	if (!IsInSession())
	{
		return;
	}

	CurrentSession.bIsActive = false;

	// Calculate session time
	FTimespan SessionDuration = FDateTime::UtcNow() - CurrentSession.StartedAt;
	TotalTimeAttackTime += (float)SessionDuration.GetTotalSeconds();

	OnSessionEnded.Broadcast(CurrentSession);

	SaveTimeAttackData();
}

void UMGTimeAttackSubsystem::RestartLap()
{
	if (!IsInSession())
	{
		return;
	}

	CurrentLap = FMGLapTime();
	CurrentLap.LapNumber = CurrentSession.CurrentLap + 1;
	CurrentLap.bIsValid = true;

	CurrentSession.CurrentSector = 0;
	CurrentSession.CurrentLapTime = 0.0f;

	CurrentDelta = FMGDeltaInfo();
}

bool UMGTimeAttackSubsystem::IsInSession() const
{
	return CurrentSession.bIsActive && CurrentSession.SessionID.IsValid();
}

void UMGTimeAttackSubsystem::OnCrossedStartLine()
{
	if (!IsInSession())
	{
		return;
	}

	// If we have a current lap with time, this is a lap completion
	if (CurrentLap.TotalTime > 0.0f)
	{
		ProcessLapCompletion(CurrentLap);
	}

	// Start new lap
	CurrentSession.CurrentLap++;
	CurrentLap = FMGLapTime();
	CurrentLap.LapNumber = CurrentSession.CurrentLap;
	CurrentLap.bIsValid = true;

	CurrentSession.CurrentSector = 0;
	CurrentSession.CurrentLapTime = 0.0f;
}

void UMGTimeAttackSubsystem::OnCrossedSector(int32 SectorIndex)
{
	if (!IsInSession())
	{
		return;
	}

	float SectorTime = CurrentSession.CurrentLapTime;

	// Calculate sector time (current time minus previous sector times)
	float PreviousSectorsTime = 0.0f;
	for (float PrevSector : CurrentLap.SectorTimes)
	{
		PreviousSectorsTime += PrevSector;
	}
	SectorTime -= PreviousSectorsTime;

	CurrentLap.SectorTimes.Add(SectorTime);
	CurrentSession.CurrentSector = SectorIndex + 1;

	OnSectorCompleted.Broadcast(SectorIndex, SectorTime);

	UpdateDelta();
}

void UMGTimeAttackSubsystem::OnCrossedCheckpoint(int32 CheckpointIndex, float Time)
{
	if (!IsInSession())
	{
		return;
	}

	// Checkpoints are used for validation
	// Could store checkpoint times for detailed analysis
}

void UMGTimeAttackSubsystem::UpdateCurrentTime(float Time)
{
	if (!IsInSession())
	{
		return;
	}

	CurrentSession.CurrentLapTime = Time;
	CurrentLap.TotalTime = Time;

	UpdateDelta();
}

void UMGTimeAttackSubsystem::InvalidateLap()
{
	CurrentLap.bIsValid = false;
}

float UMGTimeAttackSubsystem::GetCurrentLapTime() const
{
	return CurrentSession.CurrentLapTime;
}

FMGDeltaInfo UMGTimeAttackSubsystem::GetCurrentDelta() const
{
	return CurrentDelta;
}

FMGTimeAttackRecord UMGTimeAttackSubsystem::GetPersonalBest(FName TrackID, FName VehicleID) const
{
	FName Key = MakeRecordKey(TrackID, VehicleID);
	const FMGTimeAttackRecord* Record = AllRecords.Find(Key);
	return Record ? *Record : FMGTimeAttackRecord();
}

TArray<FMGTimeAttackRecord> UMGTimeAttackSubsystem::GetAllRecords() const
{
	TArray<FMGTimeAttackRecord> Result;
	for (const auto& Pair : AllRecords)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

TArray<FMGTimeAttackRecord> UMGTimeAttackSubsystem::GetRecordsForTrack(FName TrackID) const
{
	TArray<FMGTimeAttackRecord> Result;
	for (const auto& Pair : AllRecords)
	{
		if (Pair.Value.TrackID == TrackID)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

float UMGTimeAttackSubsystem::GetTheoreticalBest(FName TrackID, FName VehicleID) const
{
	FMGTimeAttackRecord Record = GetPersonalBest(TrackID, VehicleID);
	return Record.TheoreticalBest;
}

TArray<float> UMGTimeAttackSubsystem::GetBestSectorTimes(FName TrackID, FName VehicleID) const
{
	FMGTimeAttackRecord Record = GetPersonalBest(TrackID, VehicleID);
	return Record.BestSectorTimes;
}

void UMGTimeAttackSubsystem::LoadGhost(FGuid GhostID)
{
	for (FMGGhostData& Ghost : AllGhosts)
	{
		if (Ghost.GhostID == GhostID)
		{
			Ghost.bIsLoaded = true;
			OnGhostLoaded.Broadcast(Ghost);
			return;
		}
	}
}

void UMGTimeAttackSubsystem::UnloadGhost(FGuid GhostID)
{
	for (FMGGhostData& Ghost : AllGhosts)
	{
		if (Ghost.GhostID == GhostID)
		{
			Ghost.bIsLoaded = false;
			Ghost.bIsSelected = false;
			return;
		}
	}
}

void UMGTimeAttackSubsystem::SelectGhostsForSession(const TArray<FGuid>& GhostIDs)
{
	CurrentSession.ActiveGhosts.Empty();

	for (FMGGhostData& Ghost : AllGhosts)
	{
		Ghost.bIsSelected = GhostIDs.Contains(Ghost.GhostID);
		if (Ghost.bIsSelected)
		{
			CurrentSession.ActiveGhosts.Add(Ghost);
		}
	}
}

TArray<FMGGhostData> UMGTimeAttackSubsystem::GetAvailableGhosts(FName TrackID) const
{
	TArray<FMGGhostData> Result;
	for (const FMGGhostData& Ghost : AllGhosts)
	{
		if (Ghost.TrackID == TrackID)
		{
			Result.Add(Ghost);
		}
	}

	// Sort by lap time
	Result.Sort([](const FMGGhostData& A, const FMGGhostData& B)
	{
		return A.LapTime < B.LapTime;
	});

	return Result;
}

TArray<FMGGhostData> UMGTimeAttackSubsystem::GetPersonalGhosts(FName TrackID) const
{
	TArray<FMGGhostData> Result;
	for (const FMGGhostData& Ghost : AllGhosts)
	{
		if (Ghost.TrackID == TrackID && Ghost.GhostType == EMGGhostType::Personal)
		{
			Result.Add(Ghost);
		}
	}
	return Result;
}

TArray<FMGGhostData> UMGTimeAttackSubsystem::GetFriendGhosts(FName TrackID) const
{
	TArray<FMGGhostData> Result;
	for (const FMGGhostData& Ghost : AllGhosts)
	{
		if (Ghost.TrackID == TrackID && Ghost.GhostType == EMGGhostType::Friend)
		{
			Result.Add(Ghost);
		}
	}
	return Result;
}

TArray<FMGGhostData> UMGTimeAttackSubsystem::GetLeaderboardGhosts(FName TrackID, int32 Count) const
{
	TArray<FMGGhostData> Result;
	for (const FMGGhostData& Ghost : AllGhosts)
	{
		if (Ghost.TrackID == TrackID && Ghost.GhostType == EMGGhostType::Leaderboard)
		{
			Result.Add(Ghost);
		}
	}

	Result.Sort([](const FMGGhostData& A, const FMGGhostData& B)
	{
		return A.LeaderboardRank < B.LeaderboardRank;
	});

	if (Result.Num() > Count)
	{
		Result.SetNum(Count);
	}

	return Result;
}

void UMGTimeAttackSubsystem::SaveGhost(const FMGGhostData& Ghost)
{
	// Check if ghost already exists
	for (int32 i = 0; i < AllGhosts.Num(); i++)
	{
		if (AllGhosts[i].GhostID == Ghost.GhostID)
		{
			AllGhosts[i] = Ghost;
			return;
		}
	}

	AllGhosts.Add(Ghost);
}

void UMGTimeAttackSubsystem::DeleteGhost(FGuid GhostID)
{
	for (int32 i = AllGhosts.Num() - 1; i >= 0; i--)
	{
		if (AllGhosts[i].GhostID == GhostID)
		{
			AllGhosts.RemoveAt(i);
			return;
		}
	}
}

TArray<FMGTrialDefinition> UMGTimeAttackSubsystem::GetAllTrials() const
{
	TArray<FMGTrialDefinition> Result;
	for (const auto& Pair : AllTrials)
	{
		Result.Add(Pair.Value);
	}

	Result.Sort([](const FMGTrialDefinition& A, const FMGTrialDefinition& B)
	{
		return A.SortOrder < B.SortOrder;
	});

	return Result;
}

TArray<FMGTrialDefinition> UMGTimeAttackSubsystem::GetTrialsByType(EMGTrialType Type) const
{
	TArray<FMGTrialDefinition> Result;
	for (const auto& Pair : AllTrials)
	{
		if (Pair.Value.TrialType == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGTrialDefinition> UMGTimeAttackSubsystem::GetTrialsForTrack(FName TrackID) const
{
	TArray<FMGTrialDefinition> Result;
	for (const auto& Pair : AllTrials)
	{
		if (Pair.Value.TrackID == TrackID)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FMGTrialDefinition UMGTimeAttackSubsystem::GetTrial(FName TrialID) const
{
	const FMGTrialDefinition* Trial = AllTrials.Find(TrialID);
	return Trial ? *Trial : FMGTrialDefinition();
}

FMGTrialProgress UMGTimeAttackSubsystem::GetTrialProgress(FName TrialID) const
{
	const FMGTrialProgress* Progress = TrialProgress.Find(TrialID);
	return Progress ? *Progress : FMGTrialProgress();
}

EMGTrialMedal UMGTimeAttackSubsystem::GetMedalForTime(FName TrialID, float Time) const
{
	const FMGTrialDefinition* Trial = AllTrials.Find(TrialID);
	if (!Trial)
	{
		return EMGTrialMedal::None;
	}

	if (Trial->DiamondTime > 0.0f && Time <= Trial->DiamondTime)
	{
		return EMGTrialMedal::Diamond;
	}
	if (Trial->PlatinumTime > 0.0f && Time <= Trial->PlatinumTime)
	{
		return EMGTrialMedal::Platinum;
	}
	if (Trial->GoldTime > 0.0f && Time <= Trial->GoldTime)
	{
		return EMGTrialMedal::Gold;
	}
	if (Trial->SilverTime > 0.0f && Time <= Trial->SilverTime)
	{
		return EMGTrialMedal::Silver;
	}
	if (Trial->BronzeTime > 0.0f && Time <= Trial->BronzeTime)
	{
		return EMGTrialMedal::Bronze;
	}

	return EMGTrialMedal::None;
}

bool UMGTimeAttackSubsystem::StartTrial(FName TrialID)
{
	const FMGTrialDefinition* Trial = AllTrials.Find(TrialID);
	if (!Trial)
	{
		return false;
	}

	ActiveTrialID = TrialID;

	// Start session with trial parameters
	FName VehicleID = Trial->RequiredVehicle.IsNone() ? FName("DefaultVehicle") : Trial->RequiredVehicle;
	return StartSession(Trial->TrackID, VehicleID, EMGTimeAttackMode::FullRace);
}

void UMGTimeAttackSubsystem::EndTrial(float FinalTime)
{
	if (ActiveTrialID.IsNone())
	{
		EndSession();
		return;
	}

	EMGTrialMedal Medal = GetMedalForTime(ActiveTrialID, FinalTime);

	// Update progress
	FMGTrialProgress* Progress = TrialProgress.Find(ActiveTrialID);
	if (!Progress)
	{
		FMGTrialProgress NewProgress;
		NewProgress.TrialID = ActiveTrialID;
		TrialProgress.Add(ActiveTrialID, NewProgress);
		Progress = TrialProgress.Find(ActiveTrialID);
	}

	Progress->Attempts++;
	Progress->LastAttempt = FDateTime::UtcNow();

	if (Progress->BestTime <= 0.0f || FinalTime < Progress->BestTime)
	{
		Progress->BestTime = FinalTime;
	}

	if ((int32)Medal > (int32)Progress->BestMedal)
	{
		Progress->BestMedal = Medal;
		if (Progress->FirstCompleted.GetTicks() == 0)
		{
			Progress->FirstCompleted = FDateTime::UtcNow();
		}
	}

	OnTrialCompleted.Broadcast(ActiveTrialID, Medal);

	ActiveTrialID = FName();
	EndSession();
}

int32 UMGTimeAttackSubsystem::GetTotalMedals(EMGTrialMedal MinMedal) const
{
	int32 Count = 0;
	for (const auto& Pair : TrialProgress)
	{
		if ((int32)Pair.Value.BestMedal >= (int32)MinMedal)
		{
			Count++;
		}
	}
	return Count;
}

float UMGTimeAttackSubsystem::GetTrialCompletionPercent() const
{
	if (AllTrials.Num() == 0)
	{
		return 0.0f;
	}

	int32 Completed = GetTotalMedals(EMGTrialMedal::Bronze);
	return (float)Completed / (float)AllTrials.Num();
}

float UMGTimeAttackSubsystem::GetDeltaToRecord(FName TrackID, FName VehicleID) const
{
	return CurrentDelta.DeltaToPersonalBest;
}

TArray<float> UMGTimeAttackSubsystem::GetSectorDeltas(FName TrackID, FName VehicleID) const
{
	TArray<float> Result;
	TArray<float> BestSectors = GetBestSectorTimes(TrackID, VehicleID);

	for (int32 i = 0; i < CurrentLap.SectorTimes.Num() && i < BestSectors.Num(); i++)
	{
		Result.Add(CurrentLap.SectorTimes[i] - BestSectors[i]);
	}

	return Result;
}

void UMGTimeAttackSubsystem::SaveTimeAttackData()
{
	// This would integrate with save game system
}

void UMGTimeAttackSubsystem::LoadTimeAttackData()
{
	// This would integrate with save game system
}

void UMGTimeAttackSubsystem::UpdateDelta()
{
	CurrentDelta = FMGDeltaInfo();

	// Delta to personal best
	if (CurrentSession.PersonalBest > 0.0f)
	{
		// Simple linear interpolation for delta
		float ExpectedProgress = CurrentSession.CurrentLapTime / CurrentSession.PersonalBest;
		CurrentDelta.DeltaToPersonalBest = CurrentSession.CurrentLapTime - (CurrentSession.PersonalBest * ExpectedProgress);
	}

	// Delta to session best
	if (CurrentSession.SessionBest > 0.0f)
	{
		float ExpectedProgress = CurrentSession.CurrentLapTime / CurrentSession.SessionBest;
		CurrentDelta.DeltaToSessionBest = CurrentSession.CurrentLapTime - (CurrentSession.SessionBest * ExpectedProgress);
	}

	// Delta to theoretical best
	float TheoreticalBest = GetTheoreticalBest(CurrentSession.TrackID, CurrentSession.VehicleID);
	if (TheoreticalBest > 0.0f)
	{
		float ExpectedProgress = CurrentSession.CurrentLapTime / TheoreticalBest;
		CurrentDelta.DeltaToTheoreticalBest = CurrentSession.CurrentLapTime - (TheoreticalBest * ExpectedProgress);
	}

	// Check if improving
	CurrentDelta.bIsImproving = CurrentDelta.DeltaToPersonalBest < 0.0f;

	// Predict final lap time
	if (CurrentSession.PersonalBest > 0.0f && CurrentSession.CurrentLapTime > 0.0f)
	{
		CurrentDelta.PredictedLapTime = CurrentSession.PersonalBest + CurrentDelta.DeltaToPersonalBest;
	}

	OnDeltaUpdated.Broadcast(CurrentDelta);
}

void UMGTimeAttackSubsystem::ProcessLapCompletion(FMGLapTime& Lap)
{
	if (!Lap.bIsValid)
	{
		return;
	}

	TotalLapsCompleted++;

	// Check for session best
	if (CurrentSession.SessionBest <= 0.0f || Lap.TotalTime < CurrentSession.SessionBest)
	{
		CurrentSession.SessionBest = Lap.TotalTime;
	}

	// Check for personal best
	bool bNewPersonalBest = false;
	if (CurrentSession.PersonalBest <= 0.0f || Lap.TotalTime < CurrentSession.PersonalBest)
	{
		CurrentSession.PersonalBest = Lap.TotalTime;
		Lap.bIsBest = true;
		bNewPersonalBest = true;
		PersonalBestsSet++;
	}

	// Add to session history
	CurrentSession.SessionLaps.Add(Lap);

	// Update records
	UpdateRecords(Lap);

	OnLapCompleted.Broadcast(Lap);

	if (bNewPersonalBest)
	{
		OnNewPersonalBest.Broadcast(CurrentSession.TrackID, Lap.TotalTime);

		// Save ghost for new personal best
		FMGGhostData NewGhost;
		NewGhost.GhostID = FGuid::NewGuid();
		NewGhost.TrackID = CurrentSession.TrackID;
		NewGhost.VehicleID = CurrentSession.VehicleID;
		NewGhost.GhostType = EMGGhostType::Personal;
		NewGhost.OwnerID = LocalPlayerID;
		NewGhost.OwnerName = LocalPlayerName;
		NewGhost.LapTime = Lap.TotalTime;
		NewGhost.RecordedAt = FDateTime::UtcNow();
		NewGhost.GhostColor = FLinearColor::Green;
		SaveGhost(NewGhost);
	}
}

void UMGTimeAttackSubsystem::UpdateRecords(const FMGLapTime& Lap)
{
	FName Key = MakeRecordKey(CurrentSession.TrackID, CurrentSession.VehicleID);

	FMGTimeAttackRecord* Record = AllRecords.Find(Key);
	if (!Record)
	{
		FMGTimeAttackRecord NewRecord;
		NewRecord.RecordID = FGuid::NewGuid();
		NewRecord.TrackID = CurrentSession.TrackID;
		NewRecord.VehicleID = CurrentSession.VehicleID;
		NewRecord.Mode = CurrentSession.Mode;
		NewRecord.PlayerID = LocalPlayerID;
		NewRecord.PlayerName = LocalPlayerName;
		AllRecords.Add(Key, NewRecord);
		Record = AllRecords.Find(Key);
	}

	Record->TotalAttempts++;
	Record->LapHistory.Add(Lap);

	// Limit lap history
	const int32 MaxLapHistory = 50;
	if (Record->LapHistory.Num() > MaxLapHistory)
	{
		Record->LapHistory.RemoveAt(0, Record->LapHistory.Num() - MaxLapHistory);
	}

	// Update best time
	if (Record->BestTime <= 0.0f || Lap.TotalTime < Record->BestTime)
	{
		Record->BestTime = Lap.TotalTime;
		Record->SetAt = FDateTime::UtcNow();
	}

	// Update best sector times
	for (int32 i = 0; i < Lap.SectorTimes.Num(); i++)
	{
		if (i >= Record->BestSectorTimes.Num())
		{
			Record->BestSectorTimes.Add(Lap.SectorTimes[i]);
		}
		else if (Lap.SectorTimes[i] < Record->BestSectorTimes[i])
		{
			Record->BestSectorTimes[i] = Lap.SectorTimes[i];
		}
	}

	CalculateTheoreticalBest(*Record);
}

void UMGTimeAttackSubsystem::CalculateTheoreticalBest(FMGTimeAttackRecord& Record)
{
	float Theoretical = 0.0f;
	for (float SectorTime : Record.BestSectorTimes)
	{
		Theoretical += SectorTime;
	}
	Record.TheoreticalBest = Theoretical;
}

void UMGTimeAttackSubsystem::InitializeDefaultTrials()
{
	// Speed Trial 1
	FMGTrialDefinition SpeedTrial1;
	SpeedTrial1.TrialID = FName("SpeedTrial_Downtown_01");
	SpeedTrial1.TrialName = FText::FromString("Downtown Sprint");
	SpeedTrial1.TrialDescription = FText::FromString("Race through downtown as fast as possible");
	SpeedTrial1.TrialType = EMGTrialType::Speed;
	SpeedTrial1.TrackID = FName("Track_Downtown_01");
	SpeedTrial1.LapCount = 1;
	SpeedTrial1.BronzeTime = 120.0f;
	SpeedTrial1.SilverTime = 110.0f;
	SpeedTrial1.GoldTime = 100.0f;
	SpeedTrial1.PlatinumTime = 95.0f;
	SpeedTrial1.DiamondTime = 90.0f;
	SpeedTrial1.SortOrder = 1;
	AllTrials.Add(SpeedTrial1.TrialID, SpeedTrial1);

	// Drift Trial 1
	FMGTrialDefinition DriftTrial1;
	DriftTrial1.TrialID = FName("DriftTrial_Industrial_01");
	DriftTrial1.TrialName = FText::FromString("Industrial Drift");
	DriftTrial1.TrialDescription = FText::FromString("Show off your drift skills in the industrial zone");
	DriftTrial1.TrialType = EMGTrialType::Drift;
	DriftTrial1.TrackID = FName("Track_Industrial_01");
	DriftTrial1.LapCount = 1;
	DriftTrial1.BronzeTime = 90.0f;
	DriftTrial1.SilverTime = 80.0f;
	DriftTrial1.GoldTime = 70.0f;
	DriftTrial1.PlatinumTime = 65.0f;
	DriftTrial1.DiamondTime = 60.0f;
	DriftTrial1.SortOrder = 2;
	AllTrials.Add(DriftTrial1.TrialID, DriftTrial1);

	// Technical Trial 1
	FMGTrialDefinition TechnicalTrial1;
	TechnicalTrial1.TrialID = FName("TechnicalTrial_Mountain_01");
	TechnicalTrial1.TrialName = FText::FromString("Mountain Precision");
	TechnicalTrial1.TrialDescription = FText::FromString("Navigate the winding mountain roads with precision");
	TechnicalTrial1.TrialType = EMGTrialType::Technical;
	TechnicalTrial1.TrackID = FName("Track_Mountain_01");
	TechnicalTrial1.LapCount = 1;
	TechnicalTrial1.BronzeTime = 180.0f;
	TechnicalTrial1.SilverTime = 165.0f;
	TechnicalTrial1.GoldTime = 150.0f;
	TechnicalTrial1.PlatinumTime = 140.0f;
	TechnicalTrial1.DiamondTime = 130.0f;
	TechnicalTrial1.SortOrder = 3;
	AllTrials.Add(TechnicalTrial1.TrialID, TechnicalTrial1);

	// Night Speed Trial
	FMGTrialDefinition NightTrial;
	NightTrial.TrialID = FName("SpeedTrial_Highway_Night");
	NightTrial.TrialName = FText::FromString("Midnight Highway");
	NightTrial.TrialDescription = FText::FromString("Speed through the highway at night");
	NightTrial.TrialType = EMGTrialType::Speed;
	NightTrial.TrackID = FName("Track_Highway_01");
	NightTrial.LapCount = 1;
	NightTrial.bNightConditions = true;
	NightTrial.BronzeTime = 100.0f;
	NightTrial.SilverTime = 90.0f;
	NightTrial.GoldTime = 80.0f;
	NightTrial.PlatinumTime = 75.0f;
	NightTrial.DiamondTime = 70.0f;
	NightTrial.SortOrder = 4;
	NightTrial.RequiredLevel = 10;
	AllTrials.Add(NightTrial.TrialID, NightTrial);
}

FName UMGTimeAttackSubsystem::MakeRecordKey(FName TrackID, FName VehicleID) const
{
	return FName(*FString::Printf(TEXT("%s_%s"), *TrackID.ToString(), *VehicleID.ToString()));
}
